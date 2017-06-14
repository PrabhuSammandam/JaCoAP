#pragma once

#include <BaseIStackLayer.h>
#include <ErrorCodes.h>

namespace JaCoAP {

class ReliabilityLayer : public BaseStackLayer {
public:
	ErrCode sendRequest(Transaction* trans, Request* request) override;

	ErrCode sendResponse(Transaction* trans, Response* response) override;

	ErrCode receiveRequest(Transaction* trans, Request* request) override;

	ErrCode receiveResponse(Transaction* trans, Response* response) override;

	ErrCode receiveEmptyMsg(Transaction* trans, Msg* emptyMsg) override;

};

}
