#include <Block2Info.h>
#include <BlockOption.h>
#include <CoapMgr.h>
#include <CoapStack.h>
#include <IpSocketAddr.h>
#include <MemMgr.h>
#include <Msg.h>
#include <Transaction.h>
#include <TransStore.h>
#include <Utils.h>
#include "JaCoapIF.h"

FILE_NAME("Transaction.cpp");

namespace JaCoAP
{
Transaction::Transaction( TransOrigin origin, Msg *msg )
{
    Origin( origin );
    _curRequest = msg;
}

Transaction::Transaction ()
{
}

Transaction::~Transaction ()
{
    auto deleteReq = Req() != CurReq();
    auto deleteRes = Res() != CurRes();

    if( CurReq() != nullptr )
    {
        mdelete CurReq();
    }

    if( CurRes() != nullptr )
    {
        mdelete CurRes();
    }

    if( deleteReq && ( Req() != nullptr ) )
    {
        mdelete Req();
    }

    if( deleteRes && ( Res() != nullptr ) )
    {
        mdelete Res();
    }

    _request     = nullptr;
    _response    = nullptr;
    _curRequest  = nullptr;
    _curResponse = nullptr;

    _epHndl = 0xFF;

    _retransCnt = 0;
    _timeStamp  = -1;
    _timeOut    = -1;

    _data = 0;

    SetBlock1( nullptr );
    SetResBlockTransferStatus( nullptr );
    SetReqBlockTransferStatus( nullptr );
}

void Transaction::Reset()
{
}

Transaction::Transaction( const Transaction &other ) :
    _request{ other._request },
    _response{ other._response },
    _curRequest{ other._curRequest },
    _curResponse{ other._curResponse },
    _timeStamp{ other._timeStamp },
    _timeOut{ other._timeOut },
    _epHndl{ other._epHndl },
    _retransCnt{ other._retransCnt },
    _data{ other._data },
    _block1{ other._block1 },
    _reqBlockTransferStatus{ other._reqBlockTransferStatus },
    _resBlockTransferStatus{ other._resBlockTransferStatus }
{
}

Transaction::Transaction( Transaction &&other ) noexcept : _request{ other._request }
    , _response{ other._response }
    , _curRequest{ other._curRequest }
    , _curResponse{ other._curResponse }
    , _timeStamp{ other._timeStamp }
    , _timeOut{ other._timeOut }
    , _epHndl{ other._epHndl }
    , _retransCnt{ other._retransCnt }
    , _data{ other._data }
    , _block1{ other._block1 }
    , _reqBlockTransferStatus{ other._reqBlockTransferStatus }
    , _resBlockTransferStatus{ other._resBlockTransferStatus }
{
}

Transaction & Transaction::operator = ( Transaction other )
{
    using std::swap;
    swap( *this, other );
    return ( *this );
}

TransOrigin Transaction::Origin() const
{
    if( IsBitSet( _data, u16( TRANS_FIELD_ORIGIN_BITMASK ) ) )
    {
        return ( TransOrigin::REMOTE );
    }

    return ( TransOrigin::LOCAL );
}

void Transaction::Origin( TransOrigin origin )
{
    origin == TransOrigin::LOCAL ? ClearBit( _data, TRANS_FIELD_ORIGIN_BITMASK ) : SetBit( _data, TRANS_FIELD_ORIGIN_BITMASK );
}

void Transaction::Complete( bool completeValue )
{
    completeValue ? SetBit( _data, TRANS_FIELD_COMPLETE_BITMASK ) : ClearBit( _data, TRANS_FIELD_COMPLETE_BITMASK );

    if( completeValue )
    {
        CoapMgr::GetTransStore()->CompleteTrans( this );
    }
}

void Transaction::SetBlock1( BlockOption *block1 )
{
    if( _block1 != nullptr )
    {
        mdelete _block1;
    }

    _block1 = block1;
}

void Transaction::SetReqBlockTransferStatus( Block2Info *reqBlock2Info )
{
    if( _reqBlockTransferStatus != nullptr )
    {
        mdelete _reqBlockTransferStatus;
    }

    _reqBlockTransferStatus = reqBlock2Info;
}

void Transaction::SetResBlockTransferStatus( Block2Info *resBlock2Info )
{
    if( _resBlockTransferStatus != nullptr )
    {
        mdelete _resBlockTransferStatus;
    }

    _resBlockTransferStatus = resBlock2Info;
}

void Transaction::SendAccept()
{
    /*Sending the response is only for transaction created for remote request*/
    if( ( Origin() == TransOrigin::REMOTE ) && ( _request->IsConfirmable() && !_request->IsAcknowledged() ) )
    {
        _request->SetAcknowledged( true );

        auto emptyAckMsg = Utils::getEmptyMsg( MsgType::ACK, _request );
        CoapMgr::GetCoapStack()->sendEmptyMessage( this, emptyAckMsg );
        mdelete emptyAckMsg;
    }
}

void Transaction::SendReject()
{
    /*Sending the reject is only for transaction created for remote request*/
    if( Origin() == TransOrigin::REMOTE )
    {
        _request->SetRejected( true );

        auto emptyRstMsg = Utils::getEmptyMsg( MsgType::RST, _request );
        CoapMgr::GetCoapStack()->sendEmptyMessage( this, emptyRstMsg );
        mdelete emptyRstMsg;
    }
}

void Transaction::SendResponse( Msg *response )
{
    response->DstSocketAddr( *_request->SrcSocketAddr() );
    response->EpHndl( _request->EpHndl() );

    _response = response;

    CoapMgr::GetCoapStack()->sendResponse( this, response );
}

void Transaction::MarkTime() const
{
}

void * Transaction::operator new ( size_t size )
{
#ifdef ENABLE_MEM_DEBUG
	printf("Alloc Type[%-20s] size[%-6d] ", "Transaction", (u32)size);
#endif
    return ( static_cast<void *>( MemMgr::AllocTrans() ) );
}

void Transaction::operator delete ( void *ptr )
{
#ifdef ENABLE_MEM_DEBUG
	printf("Free  Type[%-20s] ", "Transaction");
#endif
    MemMgr::FreeTrans( ( static_cast<Transaction *>( ptr ) ) );
}
}
