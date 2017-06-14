#pragma once

#include "ja_types.h"
#include "Msg.h"

namespace JaCoAP
{
class TokenProvider
{
public:
    virtual ~TokenProvider ()              = default;
    virtual u64  AllocToken( Msg *msg )    = 0;
    virtual void FreeToken( u64 token )    = 0;
    virtual bool IsTokenInUse( u64 token ) = 0;
};
}