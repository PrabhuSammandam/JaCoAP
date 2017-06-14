#pragma once

#include <BlockTransferLayer.h>
#include <ReliabilityLayer.h>

namespace JaCoAP
{
class MsgDeliverer;
}

namespace JaCoAP
{
class CoapStack
{
	ReliabilityLayer _reliabilityLayer;
	BlockTransferLayer _blockTransferLayer;
public:
	CoapStack();

	void init();

	void sendRequest(Msg* request);
	void sendResponse(Transaction* trans, Msg* response);
	void sendEmptyMessage(Transaction* trans, Msg* message);
	void receiveRequest(Transaction* trans, Msg* request);
	void receiveResponse(Transaction* trans, Msg* response);
	void receiveEmptyMessage(Transaction* trans, Msg* message);
	void setMsgDeliverer(MsgDeliverer* deliverer);
	bool hasDeliverer() const;
};
}
