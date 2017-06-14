#pragma once

#include "ja_types.h"

namespace JaCoAP
{
class MsgIdProvider
{
public:
    virtual ~MsgIdProvider()   = default;
    virtual u16 GetNextMsgId() = 0;
};
}