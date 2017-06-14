#pragma once

#include "ja_types.h"
#include "IpSocketAddr.h"
#include "ByteArray.h"

namespace JaCoAP
{
class Packet
{
    IpSocketAddr _remoteSocketAddr;
    u8           _epHndl     = 0xFF;
    ByteArray    *_byteArray = nullptr;

public:
    Packet ();

    Packet* SetByteArray( ByteArray *byteArray ) { _byteArray = byteArray; return ( this ); }
    ByteArray* GetByteArray() const { return ( _byteArray ); }
    Packet* SetRemoteSocketAddr( const IpSocketAddr &endpoint ) { _remoteSocketAddr = endpoint; return ( this ); }

    IpSocketAddr* GetRemoteSocketAddr() { return ( &_remoteSocketAddr ); }

    u8 GetEpHndl() const { return ( _epHndl ); }

    Packet* SetEpHndl( u8 epHndl ) { _epHndl = epHndl; return ( this ); }

    u8* getByteArrayPtr() { return ( ( _byteArray != nullptr ) ? _byteArray->GetArray() : nullptr ); }
};
}