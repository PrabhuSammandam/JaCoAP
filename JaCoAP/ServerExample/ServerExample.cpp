#include "ServerDataChannel.h"
#include "CoapServer.h"
#include "JaCoapIF.h"

using namespace JaCoAP;

FILE_NAME("ServerExample.cpp")

int main()
{
	dbg_info("Starting");
//    auto serverDataChannel = new ServerDataChannel();
    auto serverDataChannel = mnew ServerDataChannel();

    CoapServer coapServer(serverDataChannel);

    IpSocketAddr ipSockAddr(u32(0), 5683);

    coapServer.AddEp(ipSockAddr);

    coapServer.Start();

    coapServer.Loop();

    return 0;
}

