/*
 * Exchange.h
 *
 *  Created on: May 21, 2017
 *      Author: psammand
 */

#pragma once

#include <Msg.h>
#include <Option.h>
#include <Transaction.h>
#include <cstddef>
#include <cstdint>
#include "BaseObject.h"

namespace JaCoAP
{
class Resource;
} /* namespace JaCoAP */

namespace JaCoAP
{
class Exchange
{
    Transaction *_trans    = nullptr;
    Resource    *_resource = nullptr;

public:
    Exchange();
    Exchange(Transaction *trans, Resource *resource);
    ~Exchange();

    // Move
    Exchange(Exchange &&move) noexcept;
    Exchange & operator = (Exchange &&move) noexcept;

    IpSocketAddr* GetSrcSocketAddr(){return ( _trans->Req()->SrcSocketAddr() );}

    MsgCode GetRequestCode(){return ( _trans->Req()->Code() );}

    OptionsSet* GetOptionSet(){return ( _trans->Req()->GetOptionSet() );}

    ByteArray* GetPayload(){return ( _trans->Req()->Payload() );}

    void Accept(){_trans->SendAccept();}

    void Reject(){_trans->SendReject();}

    void Respond(MsgCode code);
    void Respond(uint8_t *payload);
    void Respond(ByteArray *payload);
    void Respond(MsgCode code, ByteArray *payload, ContentFormatId contentFormat = ContentFormatId::NONE);
    void Respond(MsgCode code, uint8_t *payload, ContentFormatId contentFormat = ContentFormatId::TEXT__PLAIN);
    void Respond(Response *response);

    DEFINE_MEMORY_OPERATORS(Exchange);

private:
    // Disabling the copy constructor and copy assignment
    Exchange(const Exchange &other);
    Exchange & operator = (const Exchange &other);
};
}
