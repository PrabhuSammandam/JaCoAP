#pragma once

#include <ErrorCodes.h>

namespace JaCoAP
{
class CoapStack;
class DataChannel;
class Inbox;
class MsgCodec;
class NetIO;
class Outbox;
class TransProcessor;
class TransStore;
}

namespace JaCoAP
{
class CoapMgr {
public:
   static CoapMgr* Inst();

public:
   static ErrCode Init();

   static CoapStack* GetCoapStack();
   static TransProcessor* GetTransProcessor();
   static MsgCodec* GetMsgCodec();
   DataChannel* GetDataChannel() const;
   static TransStore* GetTransStore();
   static Inbox* GetInbox();
   static Outbox* GetOutbox();
   static NetIO* GetNetIO();

protected:
   CoapMgr();
   static CoapMgr _coapMgr;
};
}
