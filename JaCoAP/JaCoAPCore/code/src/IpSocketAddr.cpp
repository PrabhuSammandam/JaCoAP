#include <IpSocketAddr.h>
#include <stdio.h>
#include <Utils.h>

namespace JaCoAP
{
IpSocketAddr::IpSocketAddr () : _ipAddr{}
{
}

IpSocketAddr::IpSocketAddr( const IpAddress &ipAddr, u16 port ) : _ipAddr{ ipAddr }, _port{ port }
{
}

IpSocketAddr::IpSocketAddr( const u32 ipAddr, u16 port ) : _ipAddr{ ipAddr }, _port{ port }
{
}

IpSocketAddr::IpSocketAddr( const u8 *ipAddr, u16 port ) : _ipAddr{ const_cast<u8 *>( ipAddr ) }, _port{ port }
{
}

IpSocketAddr::~IpSocketAddr ()
{
    _ipAddr.~IpAddress ();
    _port = 0;
}

bool IpSocketAddr::operator == ( const IpSocketAddr &other )
{
    return ( ( _port == other._port ) && ( _ipAddr == other._ipAddr ) );
}

void IpSocketAddr::Print()
{
    printf( "IP:" );
    _ipAddr.Print();
    printf( "Port:%u\n", _port );
}

u32 IpSocketAddr::GetHashValue()
{
    auto hashVal = Utils::Hash( _ipAddr.AsBytes(), sizeof( IpAddress ) );

    hashVal = Utils::Hash( &_port, sizeof( u16 ), hashVal );

    return ( hashVal );
}
}