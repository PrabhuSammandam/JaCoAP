#include <BaseIStackLayer.h>
#include <ByteArray.h>
#include <CoapMgr.h>
#include <CoapStack.h>
#include <ErrorCodes.h>
#include <ja_types.h>
#include <Msg.h>
#include <OptionsSet.h>
#include <Outbox.h>
#include <Transaction.h>
#include <MsgDeliverer.h>
#include "JaCoapIF.h"

FILE_NAME("ServerMsgDeliverer.cpp");
namespace JaCoAP
{
class StackTopLayer : public BaseStackLayer
{
    MsgDeliverer    *_msgDeliverer = nullptr;

public:

    ErrCode sendRequest( Request *request )
    {
        auto newTrans = mnew Transaction( TransOrigin::LOCAL, request );

        if( newTrans == nullptr )
        {
            return ( ErrCode::OUT_OF_MEMORY );
        }

        return ( sendRequest( newTrans, request ) );
    }

    ErrCode sendRequest( Transaction *trans, Request *request ) override
    {
        trans->Req( request );
        return ( BaseStackLayer::sendRequest( trans, request ) );
    }

    ErrCode sendResponse( Transaction *trans, Response *response ) override
    {
        trans->Res( response );
        return ( BaseStackLayer::sendResponse( trans, response ) );
    }

    ErrCode receiveRequest( Transaction *trans, Response *request ) override
    {
        if( trans->Req() == nullptr )
        {
            trans->Req( request );
        }

        // printf(" StackTopLayer::ReceiveRequest => \n");

        if( _msgDeliverer != nullptr )
        {
            _msgDeliverer->deliverRequest( trans );
        }

        return ( ErrCode::OK );
    }

    ErrCode receiveResponse( Transaction *trans, Response *response ) override
    {
        if( !response->GetOptionSet()->HasObserve() )
        {
            trans->Complete( true );
        }

        if( _msgDeliverer != nullptr )
        {
            _msgDeliverer->deliverResponse( trans, response );
        }

        return ( ErrCode::OK );
    }

    ErrCode receiveEmptyMsg( Transaction *trans, Msg *emptyMsg ) override
    {
        // When empty messages reach the top of the CoAP stack we can ignore them
        return ( ErrCode::OK );
    }

    MsgDeliverer* getMsgDeliverer() const
    {
        return ( _msgDeliverer );
    }

    void setMsgDeliverer( MsgDeliverer *deliverer = nullptr )
    {
        _msgDeliverer = deliverer;
    }
};

class StackBotLayer : public BaseStackLayer
{
    Outbox    *_outbox = nullptr;

public:

    ErrCode sendRequest( Transaction *trans, Msg *request ) override
    {
        return ( _outbox->sendRequest( trans, request ) );
    }

    ErrCode sendResponse( Transaction *trans, Msg *response ) override
    {
        return ( _outbox->sendResponse( trans, response ) );
    }

    ErrCode sendEmptyMsg( Transaction *trans, Msg *emptyMsg ) override
    {
        return ( _outbox->sendEmptyMsg( trans, emptyMsg ) );
    }

    void SetOutbox( Outbox *outbox )
    {
        _outbox = outbox;
    }
};

StackBotLayer _botLayer{};
StackTopLayer _topLayer{};
CoapStack::CoapStack ()
{
}

void CoapStack::init()
{
    _topLayer.SetBottomLayer( &_blockTransferLayer );

    _blockTransferLayer.SetTopLayer( &_topLayer );
    _blockTransferLayer.SetBottomLayer( &_reliabilityLayer );

    _reliabilityLayer.SetTopLayer( &_blockTransferLayer );
    _reliabilityLayer.SetBottomLayer( &_botLayer );

    _botLayer.SetTopLayer( &_reliabilityLayer );
    _botLayer.SetOutbox( CoapMgr::Inst()->GetOutbox() );
}

void CoapStack::sendRequest( Msg *request )
{
    _topLayer.sendRequest( request );
}

void CoapStack::sendResponse( Transaction *trans, Msg *response )
{
    _topLayer.sendResponse( trans, response );
}

void CoapStack::sendEmptyMessage( Transaction *trans, Msg *message )
{
    _topLayer.sendEmptyMsg( trans, message );
}

void CoapStack::receiveRequest( Transaction *trans, Msg *request )
{
    _botLayer.receiveRequest( trans, request );
}

void CoapStack::receiveResponse( Transaction *trans, Msg *response )
{
    _botLayer.receiveResponse( trans, response );
}

void CoapStack::receiveEmptyMessage( Transaction *trans, Msg *message )
{
    _botLayer.receiveEmptyMsg( trans, message );
}

void CoapStack::setMsgDeliverer( MsgDeliverer *deliverer )
{
    _topLayer.setMsgDeliverer( deliverer );
}

bool CoapStack::hasDeliverer() const
{
    return ( _topLayer.getMsgDeliverer() != nullptr );
}
}
