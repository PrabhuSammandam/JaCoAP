#pragma once

#include "Outbox.h"
#include "DataChannel.h"
#include "TransProcessor.h"

namespace JaCoAP
{
class OutboxImpl: public Outbox
{
private:
	DataChannel * _dataChannel = nullptr;
	TransProcessor * _transProcessor = nullptr;

public:
	ErrCode sendRequest(Transaction * trans, Request * request) override;
	ErrCode sendResponse(Transaction * trans, Response * response) override;
	ErrCode sendEmptyMsg(Transaction * trans, Msg * emptyMsg) override;

	void SetTransProcessor(TransProcessor * transProcessor);
};
}
