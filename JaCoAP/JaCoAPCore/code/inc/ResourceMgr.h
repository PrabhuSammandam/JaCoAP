#pragma once

#include <ByteArray.h>
#include <ErrorCodes.h>
#include <ja_types.h>
#include <Msg.h>
#include <cstdint>

namespace JaCoAP
{
class Transaction;
class Resource;
}

namespace JaCoAP
{
const uint16_t MAX_RESOURCES = 10;

class ResourceMgr
{
private:
	static ResourceMgr _resMgr;
	Resource* _resourceList[MAX_RESOURCES];
	u16 _resourceCount = 0;

protected:
	ResourceMgr();

public:
	static ResourceMgr& Inst()
	{
		return _resMgr;
	}

	ErrCode AddResource(Resource* resource);
	Resource* FindResource(Request* req);

	uint16_t GetResourceDiscoveryPayloadSize();
	void GetResourceDiscoveryPayload(ByteArray* resourceDiscoverData);
};

}

