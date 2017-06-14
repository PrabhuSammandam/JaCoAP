#pragma once
#include "MsgDeliverer.h"
#include "Resource.h"
//#include "vector"
//#include "SimpleAllocator.h"

namespace JaCoAP {
class ServerMsgDeliverer : public MsgDeliverer {
//	std::vector<Resource*, JaCoAP::SimpleAllocator<Resource*>> _resources;
public:

	ServerMsgDeliverer();
	~ServerMsgDeliverer() override;

	void deliverRequest(Transaction* trans) override;
	void deliverResponse(Transaction* trans, Msg* response) override;
};
}
