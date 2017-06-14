#include <CoapMgr.h>
#include <MemMgr.h>
#include <stdio.h>
#include "JaCoapConfig.h"
#include "Utils.h"
#include "OptionsSet.h"
#include "JaCoapIF.h"

FILE_NAME("Msg.cpp");

namespace JaCoAP
{
Msg::Msg ()
{
    Init();
}

Msg::~Msg ()
{
    Reset();
}

void Msg::Init()
{
    _type         = MsgType::NONE;
    _code         = MsgCode::EMPTY;
    _tknLen       = 0;
    _id           = -1;
    _tkn          = 0;
    _epHndl       = 0xFF;
    _status       = 0;
    _payloadBlock = nullptr;

    SetLastRes( true );

    _optSet.Init();
}

MsgType Msg::Type() const
{
    return ( _type );
}

void Msg::Type( MsgType type )
{
    _type = type;
}

MsgCode Msg::Code() const
{
    return ( _code );
}

void Msg::Code( MsgCode code )
{
    _code = code;
}

u16 Msg::Id() const
{
    return ( _id );
}

void Msg::Id( u16 id )
{
    _id = id;
}

void Msg::TknLen( u8 tknLen )
{
    _tknLen = tknLen;
}

u8 Msg::TknLen() const
{
    return ( _tknLen );
}

u64 Msg::Tkn() const
{
    return ( _tkn );
}

void Msg::Tkn( u64 token )
{
    _tkn = token;
}

void Msg::Tkn( u8 *buff, u8 len )
{
    for( auto i = 0; i < len; i++ )
    {
        _tkn |= u64( buff[i] ) << i * 8;
    }
}

ByteArray * Msg::Payload() const
{
    return ( _payloadBlock );
}

Msg * Msg::Payload( ByteArray *plBuf )
{
    _payloadBlock = plBuf;
    return ( this );
}

u32 Msg::GetPayloadSize() const
{
    return ( ( _payloadBlock != nullptr ) ? _payloadBlock->GetLen() : 0 );
}

void Msg::Reset()
{
    if( _payloadBlock != nullptr )
    {
        mdelete _payloadBlock;
        _payloadBlock = nullptr;
    }

    _optSet.Release();

    Init();
}

void Msg::Print() const
{
    printf( "Message:\n" );
    printf( "\tType           :%d\n", u8( _type ) );
    printf( "\tCode           :%d\n", u8( _code ) );
    printf( "\tMsgId          :%x\n", _id );
    printf( "\tTokenLen       :%u\n", _tknLen );
    printf( "\tToken          :%I64u\n", _tkn );
    printf( "\tOptionCount    :%u\n", _optSet.GetNoOfOptions() );

    if( _payloadBlock != nullptr )
    {
        printf( "\tPayloadLength  :%u\n", _payloadBlock->GetLen() );
    }

    printf( "\n" );
}

bool Msg::IsPing() const
{
    return ( _code == MsgCode::EMPTY && _type == MsgType::CON );
}

bool Msg::IsRequest() const
{
    return ( _code > MsgCode::EMPTY && _code <= MsgCode::DELETE );
}

bool Msg::IsResponse() const
{
    return ( _code >= MsgCode::CREATED_201 );
}

bool Msg::IsEmpty() const
{
    return ( _code == MsgCode::EMPTY );
}

void Msg::SetDuplicate( bool setValue )
{
    setValue ? SetBit( _status, MsgBitmaskDuplicate ) : ClearBit( _status, MsgBitmaskDuplicate );
}

void Msg::SetTimeout( bool setValue )
{
    setValue ? SetBit( _status, MsgBitmaskTimeout ) : ClearBit( _status, MsgBitmaskTimeout );
}

void Msg::SetAcknowledged( bool setValue )
{
    setValue ?
    SetBit( _status, MsgBitmaskAcknowledged ) : ClearBit( _status, MsgBitmaskAcknowledged );
}

void Msg::SetCancelled( bool setValue )
{
    setValue ? SetBit( _status, MsgBitmaskCancelled ) : ClearBit( _status, MsgBitmaskCancelled );
}

void Msg::SetRejected( bool setValue )
{
    setValue ? SetBit( _status, MsgBitmaskRejected ) : ClearBit( _status, MsgBitmaskRejected );
}

void Msg::SetLastRes( bool setValue )
{
    setValue ?
    SetBit( _status, MsgBitmaskLastResponse ) : ClearBit( _status, MsgBitmaskLastResponse );
}

bool Msg::HasBlockOption() const
{
    return ( _optSet.HasBlock1() || _optSet.HasBlock2() );
}

u32 Msg::GetHashBySrcEpAndMid()
{
    u32 hashVal = _srcSocketAddr.GetHashValue();

    hashVal = Utils::Hash( &_id, sizeof( u16 ), hashVal );

    return ( hashVal );
}

u32 Msg::GetHashBySrcEpAndTkn()
{
    u32 hashVal = _srcSocketAddr.GetHashValue();

    hashVal = Utils::Hash( &_tkn, sizeof( u16 ), hashVal );

    return ( hashVal );
}

u32 Msg::GetHashByDstEpAndMid()
{
    u32 hashVal = _dstSocketAddr.GetHashValue();

    hashVal = Utils::Hash( &_id, sizeof( u16 ), hashVal );

    return ( hashVal );
}

u32 Msg::GetHashByDstEpAndTkn()
{
    u32 hashVal = _dstSocketAddr.GetHashValue();

    hashVal = Utils::Hash( &_tkn, sizeof( u16 ), hashVal );

    return ( hashVal );
}

u32 Msg::GetHashBySrcEpAndUri()
{
    return ( Utils::HashByUri( this, _srcSocketAddr ) );
}

u32 Msg::GetHashByDstEpAndUri()
{
    return ( Utils::HashByUri( this, _dstSocketAddr ) );
}

Msg * Msg::UpdateForResFromReq( MsgCode msgCode, Msg *req )
{
    _code                = msgCode;
    this->_dstSocketAddr = req->_srcSocketAddr;
    this->_epHndl        = req->_epHndl;
    return ( this );
}

Msg * Msg::UpdateForResFromRes( MsgCode msgCode, Msg *res )
{
    _code                = msgCode;
    this->_dstSocketAddr = res->_dstSocketAddr;
    this->_epHndl        = res->_epHndl;
    return ( this );
}
}

void * JaCoAP::Msg::operator new ( size_t size )
{
#ifdef ENABLE_MEM_DEBUG
	printf("Alloc Type[%-20s] size[%-6d] ", "Msg", (u32)size);
#endif
    return ( static_cast<Msg *>( MemMgr::AllocMsg() ) );
}

void JaCoAP::Msg::operator delete ( void *ptr )
{
#ifdef ENABLE_MEM_DEBUG
	printf("Free  Type[%-20s] ", "Msg");
#endif
    MemMgr::FreeMsg( ( static_cast<Msg *>( ptr ) ) );
}
