#pragma once

#include <TransStore.h>
#include <Msg.h>

namespace JaCoAP
{
class TransProcessor
{
    TransStore _transStore;

public:
    TransProcessor ();

    void sendRequest( Transaction *trans, Request *request );
    void sendResponse( Transaction *trans, Response *response );
    void sendEmptyMsg( Transaction *trans, Msg *emptyMsg ) const;

    Transaction* receiveRequest( Request *request );
    Transaction* receiveResponse( Response *response );
    Transaction* receiveEmptyMsg( Msg *emptyMsg );
};
}