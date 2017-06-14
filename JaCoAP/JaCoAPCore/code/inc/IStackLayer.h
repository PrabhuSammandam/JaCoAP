#pragma once

#include <ErrorCodes.h>

namespace JaCoAP
{
class Msg;
class Transaction;
}

namespace JaCoAP
{
class IStackLayer
{
public:
    virtual ~IStackLayer ()                                              = default;
    virtual ErrCode sendRequest( Transaction *trans, Msg *request )      = 0;
    virtual ErrCode sendResponse( Transaction *trans, Msg *response )    = 0;
    virtual ErrCode sendEmptyMsg( Transaction *trans, Msg *emptyMsg )    = 0;
    virtual ErrCode receiveRequest( Transaction *trans, Msg *request )   = 0;
    virtual ErrCode receiveResponse( Transaction *trans, Msg *response ) = 0;
    virtual ErrCode receiveEmptyMsg( Transaction *trans, Msg *emptyMsg ) = 0;
    virtual void    SetBottomLayer( IStackLayer *nextLayer )             = 0;
    virtual void    SetTopLayer( IStackLayer *prevLayer )                = 0;
};
}