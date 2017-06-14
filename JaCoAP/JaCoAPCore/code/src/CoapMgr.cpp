#include <CoapMgr.h>
#include <CoapStack.h>
#include <MsgCodec.h>
#include <NetIOMgr.h>
#include <TransProcessor.h>
#include <TransStore.h>

#include "InboxImpl.h"
#include "OutboxImpl.h"

namespace JaCoAP
{
CoapMgr CoapMgr::_coapMgr{};
CoapStack _coapStack{};
TransProcessor _transProcessor{};
MsgCodec _msgCodec{};
TransStore _transStore{};
InboxImpl _inbox{};
OutboxImpl _outbox{};
NetIO _netIO{};

CoapMgr * CoapMgr::Inst()
{
    return ( &_coapMgr );
}

ErrCode CoapMgr::Init()
{
    _coapStack.init();
    _netIO.Init();
    _outbox.SetTransProcessor( &_transProcessor );
    _inbox.setTransProcessor( &_transProcessor );
    _inbox.setCoapStack( &_coapStack );

    return ( ErrCode() );
}

CoapStack * CoapMgr::GetCoapStack()
{
    return ( &_coapStack );
}

TransProcessor * CoapMgr::GetTransProcessor()
{
    return ( &_transProcessor );
}

MsgCodec * CoapMgr::GetMsgCodec()
{
    return ( &_msgCodec );
}

DataChannel * CoapMgr::GetDataChannel() const
{
    return ( GetNetIO()->GetDataChannel() );
}

TransStore * CoapMgr::GetTransStore()
{
    return ( &_transStore );
}

Inbox * CoapMgr::GetInbox()
{
    return ( &_inbox );
}

Outbox * CoapMgr::GetOutbox()
{
    return ( &_outbox );
}

NetIO * CoapMgr::GetNetIO()
{
    return ( &_netIO );
}

CoapMgr::CoapMgr ()
{
}
}