#pragma once

#include "ErrorCodes.h"
#include "Transaction.h"
#include "Msg.h"

namespace   JaCoAP {

class   Outbox {
public:
	virtual ~Outbox( ) = default;

	virtual ErrCode sendRequest( Transaction* trans, Request* request ) = 0;

	virtual ErrCode sendResponse( Transaction* trans, Response* response ) = 0;

	virtual ErrCode sendEmptyMsg( Transaction* trans, Msg* emptyMsg ) = 0;
};

}
