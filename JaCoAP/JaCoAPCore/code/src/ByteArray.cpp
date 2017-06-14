#include <ByteArray.h>
#include <JaCoapIF.h>
#include <string.h>
#include <Utils.h>
#include <algorithm>

FILE_NAME("ByteArray.cpp")

namespace JaCoAP
{
auto ByteArrayDeleter = [] ( uint8_t *buf )
    {
        JaCoapIF::MemFree( buf );
    };
ByteArray::ByteArray ()
{
}

ByteArray::ByteArray( uint16_t size ) : _size{ size }
{
    if( _size > 0 )
    {
//        _byteArrayPtr = static_cast<uint8_t *>( JaCoapIF::MemAlloc( _size ) );
    	_byteArrayPtr =  static_cast<uint8_t *>(mnew_g(_size));//( JaCoapIF::MemAlloc( _size ) );
    }
}

ByteArray::ByteArray( uint8_t *buf, uint16_t bufLen, bool freeBuf )
{
    SetArray( buf, bufLen, freeBuf );
}

ByteArray::~ByteArray ()
{
    if( ( _freeBuffer == true ) && ( _byteArrayPtr != nullptr ) )
    {
        mdelete_g( _byteArrayPtr);
        _byteArrayPtr = nullptr;
    }

    _length = 0;
    _size   = 0;
}

ByteArray::ByteArray( ByteArray &&other ) noexcept : _byteArrayPtr
        { nullptr }
{
    // You must do this to pass to move assignment
    *this = std::move( other );    // <- Important
}

ByteArray & ByteArray::operator = ( ByteArray &&other ) noexcept
{
    // Avoid self assignment
    if( &other == this )
    {
        return ( *this );
    }

    // Get moving
    std::swap( _byteArrayPtr, other._byteArrayPtr );     // Moves ptrs
    this->_length     = other._length;
    this->_size       = other._size;
    this->_freeBuffer = other._freeBuffer;

    return ( *this );
}

u8 ByteArray::operator [] ( uint16_t index ) const
{
    return ( _byteArrayPtr[index] );
}

uint16_t ByteArray::GetLen() const
{
    return ( _length );
}

ByteArray * ByteArray::SetLen( uint16_t len )
{
    _length = len;
    return ( this );
}

void ByteArray::CopyStr( u8 *str )
{
    if( ( str == nullptr ) || ( _length >= _size ) )
    {
        return;
    }

    uint16_t idx = 0;

    while( str[idx] != '\0' && _length < _size )
    {
        _byteArrayPtr[_length] = str[idx];
        _length++;
        idx++;
    }
}

void ByteArray::CopyNBytes( pu8 srcBuf, uint16_t len )
{
    if( ( srcBuf == nullptr ) || ( _length >= _size ) )
    {
        return;
    }

    for( auto i = 0; i < len && _length < _size; i++ )
    {
        _byteArrayPtr[_length] = srcBuf[i];
        _length++;
    }
}

void ByteArray::CopyTo( ByteArray *dst, uint16_t from, uint16_t len ) const
{
    if( ( dst == nullptr ) || ( from >= _length ) || ( len <= 0 ) || ( dst->GetArray() == nullptr ) )
    {
        return;
    }

    auto     dstBuf = dst->GetArray();
    uint16_t i      = 0;

    for(; ( i < len ) && ( ( i + from ) < _length ) && ( i < dst->_size ); i++ )
    {
        dstBuf[i] = _byteArrayPtr[i + from];
    }

    dst->SetLen( i );
    // SetArray((uint8_t*)0,0, false);
}

void ByteArray::SetArray( uint8_t *buf, uint16_t bufLen, bool freeBuf )
{
    if( _freeBuffer && _byteArrayPtr != nullptr )
    {
		mdelete_g( _byteArrayPtr);
    }

    _byteArrayPtr = buf;
    _length       = bufLen;
    _freeBuffer   = freeBuf;
    _size         = _length;
}

bool ByteArray::IsEmpty() const
{
    return ( _length == 0 );
}

bool ByteArray::operator == ( const ByteArray &other ) const
{
    if( _length != other._length )
    {
        return ( false );
    }

    return ( ::memcmp( _byteArrayPtr, other._byteArrayPtr, _length ) == 0 );
}

void ByteArray::Assign( uint8_t *buf, uint16_t bufLen )
{
}

uint32_t ByteArray::GetHashValue()
{
    if( _byteArrayPtr != nullptr )
    {
        return ( Utils::Hash( _byteArrayPtr, _length ) );
    }

    return ( 0 );
}
}
