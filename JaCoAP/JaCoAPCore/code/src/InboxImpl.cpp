#include "InboxImpl.h"

#include <ByteArray.h>
#include <CoapMgr.h>
#include <CoapStack.h>
#include <IpSocketAddr.h>
#include <Msg.h>
#include <MsgCodec.h>
#include <OptionsSet.h>
#include <Packet.h>
#include <stdio.h>
#include <Transaction.h>
#include <TransProcessor.h>
#include <Utils.h>
#include "JaCoapIF.h"

FILE_NAME("InboxImpl.cpp");

namespace JaCoAP
{
/* it is static because to reduce the stack size */
MsgHdr _staticMsgHdr{};
InboxImpl::~InboxImpl ()
{
}

ErrCode InboxImpl::receivePacket( Packet *pkt )
{
    _staticMsgHdr.EpHndl( pkt->GetEpHndl() );

    CoapMgr::GetMsgCodec()->ParseOnly( *pkt->GetByteArray(), _staticMsgHdr );

    if( _staticMsgHdr.Error() == ErrCode::MSG_FORMAT_ERROR )
    {
        /* reject the ill formed message only it is CON & has message id */
        if( _staticMsgHdr.IsConfirmable() && _staticMsgHdr.HasMsgId() )
        {
            reject( pkt->GetRemoteSocketAddr(), &_staticMsgHdr );
        }
    }
    else
    {
        auto newMsg = mnew Msg();

        if( newMsg == nullptr )
        {
            return ( ErrCode::OUT_OF_MEMORY );
        }

        auto status = CoapMgr::GetMsgCodec()->Decode( *pkt->GetByteArray(), *newMsg );

        if( status != ErrCode::OK )
        {
            mdelete newMsg;
            return ( status );
        }

        newMsg->SrcSocketAddr( *pkt->GetRemoteSocketAddr() );
        newMsg->EpHndl( pkt->GetEpHndl() );

        auto optError = newMsg->GetOptionSet()->CheckOptions();

        if( optError != 0 )
        {
            reject( newMsg );
            mdelete newMsg;

            return ( status );
        }

        if( newMsg->IsRequest() )
        {
            receiveRequest( newMsg );
        }
        else if( newMsg->IsResponse() )
        {
            receiveResponse( newMsg );
        }
        else if( newMsg->IsEmpty() )
        {
            receiveEmptyMsg( newMsg );
            mdelete newMsg;
        }
    }

    return ( ErrCode::OK );
}

ErrCode InboxImpl::receiveRequest( Request *request )
{
    auto trans = _transProcessor->receiveRequest( request );

    if( trans != nullptr )
    {
        _coapStack->receiveRequest( trans, request );

        if(trans->IsComplete()){
        	mdelete trans;
        }
    }

    return ( ErrCode::OK );
}

ErrCode InboxImpl::receiveResponse( Response *response )
{
    auto trans = _transProcessor->receiveResponse( response );

    if( trans != nullptr )
    {
        _coapStack->receiveResponse( trans, response );
    }
    else
    {
        /* if there is no transaction, then it means there is no previous request happened. So reject this message. */
        reject( response );
    }

    return ( ErrCode::OK );
}

ErrCode InboxImpl::receiveEmptyMsg( Msg *emptyMsg )
{
    /* For both client and server, if received empty CON message, then send RST message. */
    if( emptyMsg->IsConfirmable() || emptyMsg->IsNonConfirmable() )
    {
        dbg_info("responding to PING message");
        reject( emptyMsg );
    }
    else
    {
        /*
         * the message might be ACK or RST so inform transaction processor to remove the
         * entry from transaction store.
         */
        auto trans = _transProcessor->receiveEmptyMsg( emptyMsg );

        /* pass the empty message for the stack */
        if( trans != nullptr )
        {
            _coapStack->receiveEmptyMessage( trans, emptyMsg );

            /* since this is the end of the request-response, the transaction need to released and the
             * resources hold by transaction also need to be released. */
            trans->Complete( true );
            mdelete trans;
        }
    }

    return ( ErrCode::OK );
}

void InboxImpl::reject( Msg *msg ) const
{
    auto emptyRstMsg = Utils::getEmptyMsg( MsgType::RST, msg );

    _coapStack->sendEmptyMessage( nullptr, emptyRstMsg );

    mdelete emptyRstMsg;
}

void InboxImpl::reject( IpSocketAddr *dstSocketAddr, MsgHdr *msgHdr ) const
{
    auto emptyRstMsg = Utils::getEmptyMsg( msgHdr->Type(), msgHdr->Id(), *dstSocketAddr, msgHdr->EpHndl() );

    _coapStack->sendEmptyMessage( nullptr, emptyRstMsg );

    mdelete emptyRstMsg;
}
}
