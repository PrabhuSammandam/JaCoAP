#pragma once
#include "Transaction.h"

namespace JaCoAP
{
class MsgDeliverer
{
public:
    virtual ~MsgDeliverer() = default;

    virtual void deliverRequest(Transaction *trans) = 0;

    virtual void deliverResponse(Transaction *trans, Msg *response) = 0;
};
}