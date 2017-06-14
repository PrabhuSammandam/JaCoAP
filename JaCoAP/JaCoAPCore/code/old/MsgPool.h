#ifndef __MSG_POOL_H__
#define __MSG_POOL_H__

#include "ja_types.h"
#include "JaCoapConfig.h"
#include "Msg.h"

namespace JaCoAP {

struct MsgPoolEntry {
	Msg msg;
	u8 used;
};

class MsgPool {
	MsgPoolEntry _msgList[JA_COAP_CONFIG_MAX_MSGS];

public:
	MsgPool();

	Msg* GetMsg();
	void PutMsg(Msg* msg);
};

}

#endif /*__MSG_POOL_H__*/
