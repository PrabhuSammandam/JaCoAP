#pragma once

#include "TokenProvider.h"

namespace JaCoAP
{
#define TOKEN_LENGTH 4

class SimpleTokenProvider : public TokenProvider
{
public:
    SimpleTokenProvider ();

    u64  AllocToken( Msg *msg ) override;
    void FreeToken( u64 token ) override;
    bool IsTokenInUse( u64 token ) override;
};
}