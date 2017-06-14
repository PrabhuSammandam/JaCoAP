#pragma once

#include <ErrorCodes.h>

namespace JaCoAP
{
class DataChannel;
class IpSocketAddr;
}

namespace JaCoAP
{
class CoapServer
{
public:
   CoapServer(DataChannel *dataChannel);

   ~CoapServer();

   ErrCode Start() const;

   ErrCode AddEp(IpSocketAddr const &ipSocketAddr) const;

   void Loop() const;
};
}
