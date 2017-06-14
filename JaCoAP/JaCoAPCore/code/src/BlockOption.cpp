#include <BlockOption.h>
#include <MemMgr.h>
#include <cstdint>
#include "JaCoapIF.h"

FILE_NAME("BlockOption.cpp");

namespace JaCoAP
{
BlockOption::BlockOption ()
{
}

BlockOption::BlockOption( u8 szx, bool m, u32 num )
{
    setSZX( szx );
    setMore( m );
    setNum( num );
}

u8 BlockOption::getSZX() const
{
    return ( _data & 0x00000007 );
}

void BlockOption::setSZX( u8 szx )
{
    _data |= ( szx & 0x07 );
}

u16 BlockOption::getSize() const
{
    return ( 1 << ( 4 + getSZX() ) );
}

void BlockOption::setSize( u16 size )
{
    setSZX( sizeToSZX( size ) );
}

bool BlockOption::hasMore() const
{
    return ( ( _data >> 3 ) & 0x01 );
}

void BlockOption::setMore( bool setValue )
{
    setValue ? _data |= 0x08 : _data &= ( ~0x08 );
}

u32 BlockOption::getNum() const
{
    return ( ( _data >> 4 ) & 0x0FFFFF );
}

void BlockOption::setNum( u32 num )
{
    _data &= 0xFF00000F;
    _data |= ( ( num & 0x0FFFFF ) << 4 );
}

u8 BlockOption::getLen() const
{
    return ( ( _data >> 24 ) & 0xFF );
}

void BlockOption::setLen( u8 len )
{
    _data &= 0x00FFFFFF;
    _data |= ( len << 24 );
}

void BlockOption::decode( uint8_t *buf, uint8_t bufLen )
{
    setLen( bufLen );

    if( bufLen == 0 )
    {
        setSZX( 0 );
        setMore( false );
        setNum( 0 );
    }
    else
    {
        u8  end = buf[bufLen - 1];
        setSZX( end & 0x07 );
        setMore( ( ( end >> 3 ) & 0x01 ) == 0x01 );

        u32 tempNum = ( end & 0xFF ) >> 4;

        for( int i = 1; i < bufLen; i++ )
        {
            tempNum += ( ( buf[bufLen - i - 1] & 0xff ) << ( i * 8 - 4 ) );
        }

        setNum( tempNum );
    }
}

void BlockOption::encode( uint8_t *buf, uint8_t &bufLen ) const
{
    u8  end = getSZX() | ( hasMore() ? 1 << 3 : 0 );
    u32 num = getNum();

    if( ( num == 0 ) && !hasMore() && ( getSZX() == 0 ) )
    {
        bufLen = 0;
    }
    else if( num < ( 1 << 4 ) )
    {
        bufLen = 1;
        buf[0] = ( num << 4 ) | end;
    }
    else if( num < ( 1 << 12 ) )
    {
        bufLen = 2;
        buf[0] = num >> 4;
        buf[1] = ( num << 4 ) | end;
    }
    else
    {
        bufLen = 3;
        buf[0] = num >> 12;
        buf[1] = num >> 4;
        buf[2] = ( num << 4 ) | end;
    }
}

u8 BlockOption::sizeToSZX( u16 size )
{
    if( size <= 16 )
    {
        return ( 0 );
    }

    if( ( size >= 32 ) && ( size < 64 ) )
    {
        return ( 1 );
    }

    if( ( size >= 64 ) && ( size < 128 ) )
    {
        return ( 2 );
    }

    if( ( size >= 128 ) && ( size < 256 ) )
    {
        return ( 3 );
    }

    if( ( size >= 256 ) && ( size < 512 ) )
    {
        return ( 4 );
    }

    if( ( size >= 512 ) && ( size < 1024 ) )
    {
        return ( 5 );
    }

    return ( 6 );
}

u16 BlockOption::szxToSize( u8 szx )
{
    if( szx <= 0 )
    {
        return ( 16 );
    }

    if( szx >= 6 )
    {
        return ( 1024 );
    }

    return ( 1 << ( szx + 4 ) );
}

}
