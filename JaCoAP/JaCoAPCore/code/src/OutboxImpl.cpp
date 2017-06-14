#include "OutboxImpl.h"

#include <CoapMgr.h>
#include <ErrorCodes.h>
#include <NetIOMgr.h>

namespace JaCoAP
{
ErrCode OutboxImpl::sendRequest( Transaction *trans, Request *request )
{
    _transProcessor->sendRequest( trans, request );
    CoapMgr::Inst()->GetNetIO()->SendMsg( request );

    return ( ErrCode::OK );
}

ErrCode OutboxImpl::sendResponse( Transaction *trans, Response *response )
{
    _transProcessor->sendResponse( trans, response );
    CoapMgr::Inst()->GetNetIO()->SendMsg( response );

	if (trans->IsComplete())
	{

	}

    return ( ErrCode::OK );
}

ErrCode OutboxImpl::sendEmptyMsg( Transaction *trans, Msg *emptyMsg )
{
    _transProcessor->sendEmptyMsg( trans, emptyMsg );
    CoapMgr::Inst()->GetNetIO()->SendMsg( emptyMsg );

    return ( ErrCode::OK );
}

void OutboxImpl::SetTransProcessor( TransProcessor *transProcessor )
{
    _transProcessor = transProcessor;
}
}