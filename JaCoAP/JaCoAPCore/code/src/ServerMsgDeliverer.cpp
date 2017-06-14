// #include <cstdio>
#include <ServerMsgDeliverer.h>
#include <ResourceMgr.h>
#include "JaCoapIF.h"

FILE_NAME("ServerMsgDeliverer.cpp");

namespace JaCoAP
{
ServerMsgDeliverer::ServerMsgDeliverer ()
{
}

ServerMsgDeliverer::~ServerMsgDeliverer ()
{
}

void ServerMsgDeliverer::deliverRequest( Transaction *trans )
{
    auto resource = ResourceMgr::Inst().FindResource( trans->Req() );

    if( resource != nullptr )
    {
        resource->HandleRequest( trans );
    }
    else
    {
        // printf("Didn't find the resource\n");
        auto response = mnew Response();
        response->Code( MsgCode::NOT_FOUND_404 );

        trans->SendResponse( response );
    }
}

void ServerMsgDeliverer::deliverResponse( Transaction *trans, Msg *response )
{
}
}
