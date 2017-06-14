#include <IpAddress.h>
#include <stdio.h>

namespace JaCoAP
{
IpAddress::IpAddress ()
{
}

IpAddress::IpAddress( u8 *addr )
{
    _addr[0] = addr[0];
    _addr[1] = addr[1];
    _addr[2] = addr[2];
    _addr[3] = addr[3];
}

IpAddress::IpAddress( u32 addr )
{
    _addr[0] = addr >> 24 & 0xFF;
    _addr[1] = addr >> 16 & 0xFF;
    _addr[2] = addr >> 8 & 0xFF;
    _addr[3] = addr & 0xFF;
}

IpAddress::IpAddress( u8 byte1, u8 byte2, u8 byte3, u8 byte4 )
{
    _addr[0] = byte1;
    _addr[1] = byte2;
    _addr[2] = byte3;
    _addr[3] = byte4;
}

IpAddress::~IpAddress ()
{
}

u8 IpAddress::operator [] ( u8 index ) const
{
    return ( _addr[index] );
}

u32 IpAddress::AsU32()
{
    return ( static_cast<u32>( _addr[0] << 24 | _addr[1] << 16 | _addr[2] << 8 | _addr[3] ) );
}

bool IpAddress::operator == ( const IpAddress &other )
{
    for( auto i = 0; i < 4; i++ )
    {
        if( _addr[i] != other._addr[i] )
        {
            return ( false );
        }
    }

    return ( true );
}

IpAddress & IpAddress::operator = ( const u8 *address )
{
    IpAddress( const_cast<u8 *>( address ) );
    return ( *this );
}

IpAddress & IpAddress::operator = ( u32 address )
{
    IpAddress( u32( address ) );
    return ( *this );
}

void IpAddress::Print()
{
    printf( "Dec %u.%u.%u.%u Hex %x.%x.%x.%x\n", _addr[0], _addr[1], _addr[2], _addr[3], _addr[0], _addr[1], _addr[2], _addr[3] );
}
}