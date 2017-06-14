#pragma once

#include "ErrorCodes.h"
#include "Msg.h"
#include "Packet.h"

namespace JaCoAP
{
class Inbox
{
protected:
    ~Inbox( )										= default;
public:
    virtual ErrCode receivePacket(Packet* pkt)		= 0;

    virtual ErrCode receiveRequest(Request* req)	= 0;

    virtual ErrCode receiveResponse(Response* res)	= 0;

    virtual ErrCode receiveEmptyMsg(Msg* emptyMsg)	= 0;
};
}
