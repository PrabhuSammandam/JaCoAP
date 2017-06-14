#include <Exchange.h>
#include <Msg.h>
#include <Resource.h>
#include <string.h>
#include <Transaction.h>
#include <Utils.h>
#include <algorithm>
#include "JaCoapIF.h"

FILE_NAME("Resource.cpp");

namespace JaCoAP
{
Resource::Resource( uint8_t *name, bool isVisible ) :
    _visible{ isVisible }
{
    auto nameLen = 0;

    if( name != nullptr )
    {
        nameLen = uint16_t( strlen( (const char *) name ) );
    }

    _name = mnew ByteArray( name, nameLen );

    if( nameLen > 0 )
    {
        _hashValue = _name->GetHashValue();
    }
}

Resource::Resource( ByteArray *name, bool isVisible ) :
    _visible{ isVisible }
{
    *_name     = std::move( *name );
    _hashValue = _name->GetHashValue();
}

Resource::~Resource ()
{
    mdelete _name;
}

void Resource::HandleRequest( Transaction *trans )
{
    auto code     = trans->Req()->Code();
    auto exchange = mnew Exchange( trans, this );

    if( code == MsgCode::GET )
    {
        HandleGet( exchange );
    }
    else if( code == MsgCode::POST )
    {
        HandlePost( exchange );
    }
    else if( code == MsgCode::DELETE )
    {
        HandleDelete( exchange );
    }
    else if( code == MsgCode::PUT )
    {
        HandlePut( exchange );
    }

    mdelete exchange;
}

void Resource::HandleGet( Exchange *exchange )
{
    exchange->Respond( MsgCode::METHOD_NOT_ALLOWED_405 );
}

void Resource::HandlePost( Exchange *exchange )
{
    exchange->Respond( MsgCode::METHOD_NOT_ALLOWED_405 );
}

void Resource::HandlePut( Exchange *exchange )
{
    exchange->Respond( MsgCode::METHOD_NOT_ALLOWED_405 );
}

void Resource::HandleDelete( Exchange *exchange )
{
    exchange->Respond( MsgCode::METHOD_NOT_ALLOWED_405 );
}
}
