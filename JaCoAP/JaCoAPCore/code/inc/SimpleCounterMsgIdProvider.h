#pragma once

#include "MsgIdProvider.h"

namespace JaCoAP
{
class SimpleCounterMsgIdProvider : public MsgIdProvider
{
    u16 _msgCounter;

public:
    SimpleCounterMsgIdProvider ();

    u16 GetNextMsgId() override;
};
}