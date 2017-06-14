#include <Block2Info.h>
#include <BlockOption.h>
#include <BlockTransferLayer.h>
#include <ByteArray.h>
#include <ErrorCodes.h>
#include <IpSocketAddr.h>
#include <ja_types.h>
#include <Msg.h>
#include <OptionsSet.h>
#include <Transaction.h>
#include <Utils.h>
#include <algorithm>
#include <cstdio>
#include "JaCoapIF.h"

FILE_NAME( "BlockTransferLayer.cpp" )

namespace JaCoAP
{
BlockTransferLayer::BlockTransferLayer ()
{
}

ErrCode BlockTransferLayer::sendRequest( Transaction *trans, Request *request )
{
    auto optSet = request->GetOptionSet();

    if( optSet->HasBlock2() )
    {
        // This is the case if the user has explicitly added a block option for random access.
        // Note: We do not regard it as random access when the block num is 0.
        // This is because the user might just want to do early block
        // size negotiation but actually wants to receive all blocks.
        BlockOption block2        {};

        optSet->GetBlock2( block2 );

        if( block2.getNum() > 0 )
        {
            auto blkTfrStatus = mnew Block2Info( BlockOption::szxToSize( block2.getSZX() ), optSet->GetContentFormat() );

            blkTfrStatus->SetSZX( block2.getSZX() );
            blkTfrStatus->SetCurNum( block2.getNum() );
            blkTfrStatus->SetRandomAccess( true );
            trans->SetResBlockTransferStatus( blkTfrStatus );
        }
    }
    else if( isBlockTransferRequired( request ) )
    {
        // This must be a large POST or PUT request
        auto blkTfrStatus = getReqBlock2Info( trans, request );
        auto block        = getNextReqBlock( trans->Req(), blkTfrStatus );

        // indicate overall body size to peer
        block->GetOptionSet()->SetSize1( request->GetPayloadSize() );
        trans->SetReqBlockTransferStatus( blkTfrStatus );
        trans->CurReq( block );

        return ( BaseStackLayer::sendRequest( trans, block ) );
    }
    else
    {
        trans->CurReq( request );
    }

    return ( BaseStackLayer::sendRequest( trans, request ) );
}

ErrCode BlockTransferLayer::sendResponse( Transaction *trans, Response *response )
{
    auto block1 = trans->GetBlock1();

    if( block1 != nullptr )
    {
        trans->SetBlock1( nullptr );
    }

    if( isBlockTransferRequired( trans, response ) )
    {
        auto block2Info    = getResBlock2Info( trans, response );
        auto firstBlockRes = getNextResBlock( response, block2Info );

        // indicate overall body size to peer
        firstBlockRes->GetOptionSet()->SetSize2( response->GetPayloadSize() );

        if( block1 != nullptr )
        {
            firstBlockRes->GetOptionSet()->SetBlock1( *block1 );
        }

        if( block2Info->IsComplete() )
        {
            trans->SetResBlockTransferStatus( nullptr );
        }

        trans->CurRes( firstBlockRes );

        return ( BaseStackLayer::sendResponse( trans, firstBlockRes ) );
    }

    // this is normal response without block transfer
    if( block1 != nullptr )
    {
        response->GetOptionSet()->SetBlock1( *block1 );
    }

    trans->CurRes( response );

    return ( BaseStackLayer::sendResponse( trans, response ) );
}

void BlockTransferLayer::startEarlyBlock2Negotiation( Transaction *trans, Msg *request ) const
{
    // Call this method when a request has completely arrived (might have been sent in one piece without blockwise).
    if( request->GetOptionSet()->HasBlock2() )
    {
        BlockOption block2
        {};

        request->GetOptionSet()->GetBlock2( block2 );

        auto blkTfrStatus = mnew Block2Info( block2.getSZX(), block2.getNum(), request->GetOptionSet()->GetContentFormat() );

        trans->SetResBlockTransferStatus( blkTfrStatus );
    }
}

ErrCode BlockTransferLayer::receiveRequest( Transaction *trans, Request *request )
{
    auto optSet = request->GetOptionSet();

    if( optSet->HasBlock1() )
    {
        if( _maxResBodySize > 0 )
        {
            handleInboundBlockUpload( trans, request );
        }
        else
        {
            // "transparent block wise handling is disabled, delivering request to application layer"
            return ( BaseStackLayer::receiveRequest( trans, request ) );
        }
    }
    else if( ( trans->CurRes() != nullptr ) && optSet->HasBlock2() )
    {
        // The response has already been generated and the client just wants its next block
        BlockOption nextReqBlock2Opt{};

        optSet->GetBlock2( nextReqBlock2Opt );

        auto parentResponse = trans->Res();
        auto resBlock2Info  = getResBlock2Info( trans, parentResponse );

        resBlock2Info->SetCurNum( nextReqBlock2Opt.getNum() );   // set the client requested block number
        resBlock2Info->SetSZX( nextReqBlock2Opt.getSZX() );   // set the client requested block size

        auto nextResBlock = getNextResBlock( parentResponse, resBlock2Info );

        // indicate overall body size to peer
        nextResBlock->GetOptionSet()->SetSize2( parentResponse->GetPayloadSize() );

        if( resBlock2Info->IsComplete() )
        {
            // clean up block wise status
            trans->SetResBlockTransferStatus( nullptr );
        }

        // delete the previous send response block
        if( trans->CurRes() != nullptr )
        {
            delete trans->CurRes();
        }

        // send next res block
        trans->CurRes( nextResBlock );

        return ( BaseStackLayer::sendResponse( trans, nextResBlock ) );
    }
    else
    {
        startEarlyBlock2Negotiation( trans, request );
        trans->Req( request );

        return ( BaseStackLayer::receiveRequest( trans, request ) );
    }

    return ( ErrCode::OK );
}

ErrCode BlockTransferLayer::receiveResponse( Transaction *trans, Response *response )
{
    auto stat   = ErrCode::OK;
    auto optSet = response->GetOptionSet();

    if( trans->Req()->IsCancelled() && !response->IsAck() )
    {
        // do not continue fetching blocks if canceled, do reject (in particular for Block+Observe)
        // reject block wise transfer for canceled Exchange
        auto emptRstMsg = Utils::getEmptyMsg( MsgType::RST, response );
        stat = BaseStackLayer::sendEmptyMsg( trans, emptRstMsg );
        delete emptRstMsg;

        // Matcher sets exchange as complete when RST is sent
    }
    else if( !( optSet->HasBlock1() || optSet->HasBlock2() ) )
    {
        // This is a normal response, no special treatment necessary
        trans->Res( response );

        stat = BaseStackLayer::receiveResponse( trans, response );
    }
    else
    {
        if( optSet->HasBlock1() )
        {
            stat = handleBlock1Res( trans, response );
        }

        if( optSet->HasBlock2() )
        {
            stat = handleBlock2Res( trans, response );
        }
    }

    return ( stat );
}

Block2Info * BlockTransferLayer::getReqBlock2Info( Transaction *trans, Msg *request ) const
{
    auto block1Info = trans->GetReqBlockTransferStatus();
    auto optSet     = request->GetOptionSet();

    if( block1Info == nullptr )
    {
        u16 blkTfrBufSize
        {};

        if( trans->IsOriginLocal() )
        {
            // we are sending a large body out in a POST/GET to a peer we only need to buffer one block each
            blkTfrBufSize = _prefBlockSize;
        }
        else
        {
            // we are receiving a large body in a POST/GET from a peer
            // we need to be prepared to buffer up to MAX_RESOURCE_BODY_SIZE bytes
            blkTfrBufSize =
                ( optSet->HasBlock1() && optSet->HasSize1() ) ?
                optSet->GetSize1() : _maxResBodySize;
        }

        block1Info = mnew Block2Info{ blkTfrBufSize, optSet->GetContentFormat() };

        block1Info->SetFirstMsg( request );
        block1Info->SetSZX( BlockOption::sizeToSZX( _prefBlockSize ) );
        trans->SetReqBlockTransferStatus( block1Info );
    }

    return ( block1Info );
}

/**
 * \brief Get the response block2 info. This block2 info is created by two ways.
 * 1. Client send the block2 option with required block size.
 * 2. Server will start sending response by creating new block2 info.
 * \param trans
 * \param response
 * \return
 */
Block2Info * BlockTransferLayer::getResBlock2Info( Transaction *trans, Msg *response ) const
{
    auto resBlock2Info = trans->GetResBlockTransferStatus();
    auto optSet        = response->GetOptionSet();

    if( resBlock2Info == nullptr )
    {
        // we didn't received the block2 option in request.So we are starting the block transfer.
        u16 blockBufSize = 0;

        if( trans->IsOriginLocal() )
        {
            // this is for CLIENT going to receive response.
            // we are receiving a large body in response to a request originating locally
            // we need to be prepared to buffer up to MAX_RESOURCE_BODY_SIZE bytes
            blockBufSize =
                ( optSet->HasBlock2() && optSet->HasSize2() ) ?
                optSet->GetSize2() : _maxResBodySize;
        }
        else
        {
            // we are sending out a large body in response to a request from a peer
            // we do not need to buffer and assemble anything
            blockBufSize = 0;     // this is SERVER sending response to request
        }

        resBlock2Info = mnew Block2Info{ blockBufSize, optSet->GetContentFormat() };

        resBlock2Info->SetFirstMsg( response );

        // we are starting the block transfer, so set our preferred size.
        resBlock2Info->SetSZX( BlockOption::sizeToSZX( _prefBlockSize ) );
        trans->SetResBlockTransferStatus( resBlock2Info );
    }
    else
    {
        if( BlockOption::szxToSize( resBlock2Info->GetSZX() ) > _prefBlockSize )
        {
            resBlock2Info->SetSZX( BlockOption::sizeToSZX( _prefBlockSize ) );
        }
    }

    return ( resBlock2Info );
}

constexpr auto MSG_ENTITY_TOO_LARGE          = "Body too large";
constexpr auto MSG_UNEXPECTED_CONTENT_FORMAT = "Unexpected Content-Format";
constexpr auto ERROR_MSG_WRONG_BLOCK_NUMBER  = "Wrong Block Number";

ErrCode BlockTransferLayer::sendErrorRes( Transaction *trans, Msg *request, BlockOption block1, pu8 payloadStr )
{
    auto errorResponse = mnew Msg();
    auto plBuf = mnew ByteArray( 128 );

    plBuf->CopyStr( payloadStr );
    errorResponse->UpdateForResFromReq( MsgCode::REQUEST_ENTITY_INCOMPLETE_408, request )->Payload(
        plBuf );
    errorResponse->GetOptionSet()->SetBlock1(
        BlockOption( block1.getSZX(), block1.hasMore(), block1.getNum() ) );
    trans->CurRes( errorResponse );

    return ( BaseStackLayer::sendResponse( trans, errorResponse ) );
}

ErrCode BlockTransferLayer::handleInboundBlockUpload( Transaction *trans, Msg *request )
{
    // printf("BlockTransferLayer::HandleInboundBlockUpload=> Got request\n");

    /* If server is getting more than the its configured resource body size, then send req too large */
    if( request->GetOptionSet()->HasSize1()
      && ( request->GetOptionSet()->GetSize1() > _maxResBodySize ) )
    {
        auto errorRes = mnew Msg();
        auto plBuf = mnew ByteArray( 128 );

        plBuf->CopyStr( pu8( MSG_ENTITY_TOO_LARGE ) );
        errorRes->UpdateForResFromReq( MsgCode::REQUEST_ENTITY_TOO_LARGE_413, request )->Payload(
            plBuf )->GetOptionSet()->SetSize1( _maxResBodySize );
        trans->CurRes( errorRes );

        return ( BaseStackLayer::sendResponse( trans, errorRes ) );
    }

    BlockOption block1{};

    auto reqBlock2Info = getReqBlock2Info( trans, request );

    request->GetOptionSet()->GetBlock1( block1 );

    if( ( block1.getNum() == 0 ) && ( reqBlock2Info->GetCurNum() > 0 ) )
    {
        // Block1 num is 0, the client has restarted the block wise transfer. Reset status.
        // reset the block wise transfer
        // reset current status
        trans->SetReqBlockTransferStatus( nullptr );

        // and create new status for fresher notification
        reqBlock2Info = getReqBlock2Info( trans, request );
    }

    if( block1.getNum() == reqBlock2Info->GetCurNum() )
    {
        if( reqBlock2Info->GetContentFormat() == request->GetOptionSet()->GetContentFormat() )
        {
            reqBlock2Info->AddData( *request->Payload() );
            reqBlock2Info->SetCurNum( reqBlock2Info->GetCurNum() + 1 );

            if( block1.hasMore() )
            {
                // "There are more blocks to come. Acknowledge this block."
                auto piggyResponse = new Msg();

                piggyResponse->UpdateForResFromReq( MsgCode::CONTINUE_231, request )->SetLastRes( false );
                piggyResponse->GetOptionSet()->SetBlock1( BlockOption( block1.getSZX(), true, block1.getNum() ) );
                trans->CurRes( piggyResponse );

                return ( BaseStackLayer::sendResponse( trans, piggyResponse ) );
            }
            else
            {
                auto newBlock1 = mnew BlockOption();

                request->GetOptionSet()->GetBlock1( *newBlock1 );
                trans->SetBlock1( newBlock1 );
                startEarlyBlock2Negotiation( trans, request );

                auto parentRequest = reqBlock2Info->GetFirstMsg();
                auto assembledMsg  = new Msg();

                assembledMsg->Code( request->Code() );
                assembledMsg->SrcSocketAddr( *parentRequest->SrcSocketAddr() );
                assembledMsg->EpHndl( parentRequest->EpHndl() );
                assembledMsg->Type( parentRequest->Type() );
                assembledMsg->Id( parentRequest->Id() );
                assembledMsg->Tkn( parentRequest->Tkn() );
                assembledMsg->GetOptionSet()->Copy( *parentRequest->GetOptionSet() );
                assembledMsg->Payload( reqBlock2Info->GetData() );
                reqBlock2Info->ClearData();
                trans->Req( assembledMsg );

                return ( BaseStackLayer::receiveRequest( trans, assembledMsg ) );
            }
        }
        else
        {
            // Send UnExcepted Content-Format msg
            return ( sendErrorRes( trans, request, block1, pu8( MSG_ENTITY_TOO_LARGE ) ) );
        }
    }
    else
    {
        // Send Wrong Block No msg
        return ( sendErrorRes( trans, request, block1, pu8( ERROR_MSG_WRONG_BLOCK_NUMBER ) ) );
    }
}

void BlockTransferLayer::SendNextBlock( Transaction *trans, Msg *msg, BlockOption *block1, Block2Info *block2Info )
{
    // Send next block
    int currentSize
    { 1 << ( 4 + block2Info->GetSZX() ) };

    // Define new size of the block depending of preferred size block
    int newSize
    {}, newSzx
    {};

    if( block1->getSize() < currentSize )
    {
        newSize = block1->getSize();
        newSzx  = block1->getSZX();
    }
    else
    {
        newSize = currentSize;
        newSzx  = block2Info->GetSZX();
    }

    int nextNum = block2Info->GetCurNum() + currentSize / newSize;

    block2Info->SetCurNum( nextNum );
    block2Info->SetSZX( newSzx );

    auto nextBlock = getNextReqBlock( trans->Req(), block2Info );

    // indicate overall body size to peer
    nextBlock->GetOptionSet()->SetSize1( trans->Req()->GetPayloadSize() );

    // we use the same token to ease traceability
    nextBlock->Tkn( msg->Tkn() );
    trans->CurReq( nextBlock );
    BaseStackLayer::sendRequest( trans, nextBlock );

    // do not deliver response
}

ErrCode BlockTransferLayer::handleBlock1Res( Transaction *trans, Msg *msg )
{
    auto block2Info = trans->GetResBlockTransferStatus();

    if( block2Info == nullptr )
    {
    }
    else if( !block2Info->IsComplete() )
    {
        BlockOption block1
        {};

        msg->GetOptionSet()->GetBlock1( block1 );

        if( block1.hasMore() )
        {
            // server wants us to send the remaining blocks before returning its response
            SendNextBlock( trans, msg, &block1, block2Info );
        }
        else
        {
            // this means that the response already contains the server's final
            // response to the request. However, the server is still expecting us
            // to continue to send the remaining blocks as specified in
            // https://tools.ietf.org/html/rfc7959#section-2.3
            // the current implementation does not allow us to forward the response
            // to the application layer, though, because it would "complete"
            // the exchange and thus remove the blockwise status necessary
            // to keep track of this POST/PUT request
            // we therefore go on sending all pending blocks and then return the
            // response received for the last block
            SendNextBlock( trans, msg, &block1, block2Info );
        }
    }
    else if( !msg->GetOptionSet()->HasBlock2() )
    {
        // All request block have been acknowledged and we receive a piggy-backed
        // response that needs no blockwise transfer. Thus, deliver it.
        return ( BaseStackLayer::sendResponse( trans, msg ) );
    }

    return ( ErrCode::OK );
}

ErrCode BlockTransferLayer::handleBlock2Res( Transaction *trans, Msg *response )
{
    auto optSet = response->GetOptionSet();

    if( optSet->HasSize2() && ( optSet->GetSize2() > _maxResBodySize ) )
    {
        // requested resource body exceeds max buffer size
        trans->Req()->SetCancelled( true );

        return ( ErrCode::OK );
    }

    BlockOption block2{};

    auto block2Info = getResBlock2Info( trans, response );

    optSet->GetBlock2( block2 );

    // a new notification might arrive during a block-wise transfer
    if( optSet->HasObserve() && ( block2.getNum() == 0 ) && ( block2Info->GetCurNum() != 0 ) )
    {
        // TODO
        return ( ErrCode::OK );
    }

    // check token to avoid mixed blockwise transfers (possible with observe)
    if( ( block2.getNum() == block2Info->GetCurNum() )
      && ( ( block2.getNum() == 0 ) || ( response->Tkn() == trans->CurReq()->Tkn() ) ) )
    {
        // got the expected block
        if( !block2Info->AddData( *response->Payload() ) )
        {
            // requested resource body exceeds max buffer size
            trans->Req()->SetCancelled( true );

            return ( ErrCode::OK );
        }

        if( optSet->HasObserve() )
        {
        }

        if( block2Info->IsRandomAccess() )
        {
            // The client has requested this specific block and we deliver it
            trans->Res( response );

            return ( BaseStackLayer::receiveResponse( trans, response ) );
        }

        if( block2.hasMore() )
        {
            auto request = trans->Req();

            u32  num = block2.getNum() + 1;
            u8   szx = block2.getSZX();
            bool m
            { false };

            auto newBlock = mnew Msg();

            newBlock->Code( request->Code() );
            newBlock->Type( request->Type() );
            newBlock->DstSocketAddr( *request->DstSocketAddr() );
            newBlock->EpHndl( request->EpHndl() );

            if( !optSet->HasObserve() )
            {
                newBlock->Tkn( response->Tkn() );
            }

            newBlock->GetOptionSet()->Copy( *request->GetOptionSet() );

            // newBlock->GetOptionSet()->removeObserve();
            newBlock->GetOptionSet()->SetBlock2( BlockOption( szx, m, num ) );
            block2Info->SetCurNum( num );
            trans->CurReq( newBlock );

            return ( BaseStackLayer::sendRequest( trans, newBlock ) );
        }

        auto firstResponse = block2Info->GetFirstMsg();
        auto finalResponse = new Msg();

        finalResponse->Code( response->Code() );
        finalResponse->SrcSocketAddr( *firstResponse->SrcSocketAddr() );
        finalResponse->Type( firstResponse->Type() );
        finalResponse->Tkn( firstResponse->Tkn() );
        finalResponse->Id( firstResponse->Id() );
        finalResponse->GetOptionSet()->Copy( *firstResponse->GetOptionSet() );
        finalResponse->Payload( block2Info->GetData() );
        block2Info->ClearData();
        trans->Res( finalResponse );

        return ( BaseStackLayer::receiveResponse( trans, finalResponse ) );
    }

    // ERROR, wrong block number (server error)
    if( response->IsConfirmable() )
    {
        auto emptyRstMsg = Utils::getEmptyMsg( MsgType::RST, response );

        return ( BaseStackLayer::sendEmptyMsg( trans, emptyRstMsg ) );
        delete emptyRstMsg;
    }

    return ( ErrCode::OK );
}

bool BlockTransferLayer::isBlockTransferRequired( Msg *request ) const
{
    return ( ( ( request->Code() == MsgCode::PUT ) || ( request->Code() == MsgCode::POST ) )
           && ( request->GetPayloadSize() > _maxPayloadSize ) );
}

bool BlockTransferLayer::isBlockTransferRequired( Transaction *trans, Msg *response ) const
{
    return ( ( response->GetPayloadSize() > _maxPayloadSize )
           || ( trans->GetResBlockTransferStatus() != nullptr ) );
}

Msg * BlockTransferLayer::getNextResBlock( Msg *parentResponse, Block2Info *block2TransferStatus )
{
    Msg *nextBlockRes = parentResponse;

    if( !parentResponse->GetOptionSet()->HasObserve() )
    {
        nextBlockRes = mnew Msg{};

        nextBlockRes->UpdateForResFromRes( parentResponse->Code(), parentResponse );
        nextBlockRes->GetOptionSet()->Copy( *parentResponse->GetOptionSet() );
    }

    auto        szx            = block2TransferStatus->GetSZX();
    auto        blockNum       = block2TransferStatus->GetCurNum();
    auto        payloadsize    = parentResponse->GetPayloadSize();
    u16         currentSize    = 1 << ( 4 + szx );
    auto        from           = blockNum * currentSize;
    auto        isLastResponse = true;
    auto        isComplete     = true;

    BlockOption block2Option( szx, false, blockNum );

    dbg_info( "Sending payload of size[%u]", currentSize );

    if( ( payloadsize > 0 ) && ( from < payloadsize ) )
    {
        auto to     = std::min( ( blockNum + 1 ) * currentSize, payloadsize );
        auto length = to - from;
        auto m      = ( to < payloadsize );

        block2Option.setMore( m );
        nextBlockRes->GetOptionSet()->SetBlock2( block2Option );
        dbg_info( "Sending data szx[%u], m[%u], num[%u]", szx, m, blockNum );

        auto blockPayload = mnew ByteArray( length );
        parentResponse->Payload()->CopyTo( blockPayload, from, length );
        nextBlockRes->Payload( blockPayload );
        isLastResponse = !m && !parentResponse->GetOptionSet()->HasObserve();
        isComplete     = !m;
    }

    nextBlockRes->GetOptionSet()->SetBlock2( block2Option );
    nextBlockRes->SetLastRes( isLastResponse );
    block2TransferStatus->SetComplete( isComplete );

    return ( nextBlockRes );
}

Msg * BlockTransferLayer::getNextReqBlock( Msg *request, Block2Info *block2Info )
{
    auto num = block2Info->GetCurNum();
    auto szx = block2Info->GetSZX();
    auto newReq = mnew Msg();

    newReq->Type( request->Type() );
    newReq->DstSocketAddr( *request->DstSocketAddr() );
    newReq->GetOptionSet()->Copy( *request->GetOptionSet() );

    int  currentSize = 1 << ( 4 + szx );
    int  from        = num * currentSize;

    auto to = std::min( ( num + 1 ) * currentSize, request->GetPayloadSize() );
    auto length = to - from;
    auto blockPayload = mnew ByteArray( length );

    request->Payload()->CopyTo( blockPayload, from, length );
    newReq->Payload( blockPayload );

    auto m = ( to < request->GetPayloadSize() );

    newReq->GetOptionSet()->SetBlock1( BlockOption( szx, m, num ) );
    block2Info->SetComplete( !m );

    return ( newReq );
}
}
