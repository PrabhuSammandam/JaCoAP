#pragma once

#include <cstddef>
#include "ja_types.h"
#include "Msg.h"
#include "OptionsSet.h"
#include "IpSocketAddr.h"
#include <cstdint>

namespace JaCoAP
{
enum class MsgType
{
    CON = 0,
    NON,
    ACK,
    RST,
    NONE
};

enum class MsgCode
{
    EMPTY                          = 0,
    GET                            = 1,
    POST                           = 2,
    PUT                            = 3,
    DELETE                         = 4,
    CREATED_201                    = 65,
    DELETED_202                    = 66,
    VALID_203                      = 67,
    CHANGED_204                    = 68,
    CONTENT_205                    = 69,
    CONTINUE_231                   = 95,
    BAD_REQUEST_400                = 128,
    UNAUTHORIZED_401               = 129,
    BAD_OPTION_402                 = 130,
    FORBIDDEN_403                  = 131,
    NOT_FOUND_404                  = 132,
    METHOD_NOT_ALLOWED_405         = 133,
    NOT_ACCEPTABLE_406             = 134,
    REQUEST_ENTITY_INCOMPLETE_408  = 136,
    PRECONDITION_FAILED_412        = 140,
    REQUEST_ENTITY_TOO_LARGE_413   = 141,
    UNSUPPORTED_CONTENT_FORMAT_415 = 143,
    INTERNAL_SERVER_ERROR_500      = 160,
    NOT_IMPLEMENTED_501            = 161,
    BAD_GATEWAY_502                = 162,
    SERVICE_UNAVAILABLE_503        = 163,
    GATEWAY_TIMEOUT_504            = 164,
    PROXYING_NOT_SUPPORTED_505     = 165
};

class MsgHdr
{
private:
    MsgType    _type          = MsgType::RST;
    MsgCode    _code          = MsgCode::EMPTY;
    u16        _id            = -1;
    ErrCode    _errCode       = ErrCode::OK;
    u8         *_payloadStart = nullptr;
    u16        _payloadLen    = 0;
    u8         _epHndl        = 0xFF;

public:
    MsgHdr () {}

    MsgType Type() const { return ( _type ); }
    void    Type( MsgType type ) { _type = type; }

    MsgCode Code() const { return ( _code ); }
    void    Code( MsgCode code ) { _code = code; }

    u16  Id() const { return ( _id ); }
    void Id( u16 id ) { _id = id; }

    ErrCode Error() const { return ( _errCode ); }
    void    Error( ErrCode errCode ) { _errCode = errCode; }

    u16  PayloadLen() const { return ( _payloadLen ); }
    void PayloadLen( u16 len ) { _payloadLen = len; }

    u8   * PayloadStart() const { return ( _payloadStart ); }
    void PayloadStart( u8 *payloadStart ) { _payloadStart = payloadStart; }

    bool HasPayload() const { return ( _payloadStart != nullptr ); }
    bool IsConfirmable() const { return ( _type == MsgType::CON ); }
    bool HasMsgId() const { return ( _id != -1 ); }

    u8   EpHndl() const { return ( _epHndl ); }
    void EpHndl( u8 epHndl ) { _epHndl = epHndl; }
};

const uint32_t MsgBitmaskLastResponse = 0x04000000;
const uint32_t MsgBitmaskDuplicate    = 0x08000000;
const uint32_t MsgBitmaskAcknowledged = 0x10000000;
const uint32_t MsgBitmaskCancelled    = 0x20000000;
const uint32_t MsgBitmaskRejected     = 0x40000000;
const uint32_t MsgBitmaskTimeout      = 0x80000000;

class Msg
{
public:
    Msg ();
    ~Msg ();

private:
    MsgType    _type;
    MsgCode    _code;
    u64        _tkn;
    u16        _id;
    u8         _tknLen;
    u8         _epHndl = 0xFF;

    OptionsSet      _optSet;
    ByteArray       *_payloadBlock = nullptr;
    IpSocketAddr    _srcSocketAddr;
    IpSocketAddr    _dstSocketAddr;
    u32             _status = 0;

public:
    void Init();

    MsgType Type() const;
    void    Type( MsgType type );

    MsgCode Code() const;
    void    Code( MsgCode code );

    u16  Id() const;
    void Id( u16 id );

    void TknLen( u8 tknLen );
    u8   TknLen() const;

    u64  Tkn() const;
    void Tkn( u64 token );
    void Tkn( u8 *buff, u8 len );

    OptionsSet* GetOptionSet() { return ( &_optSet ); }

    ByteArray* Payload() const;
    Msg      * Payload( ByteArray *plBuf );
    u32      GetPayloadSize() const;

    void Reset();
    void Print() const;

    IpSocketAddr* SrcSocketAddr() { return ( &_srcSocketAddr ); }
    void        SrcSocketAddr( const IpSocketAddr &srcAddr ) { _srcSocketAddr = srcAddr; }

    IpSocketAddr* DstSocketAddr() { return ( &_dstSocketAddr ); }
    void        DstSocketAddr( const IpSocketAddr &dstAddr ) { _dstSocketAddr = dstAddr; }

    u8   EpHndl() const { return ( _epHndl ); }
    void EpHndl( u8 epHndl ) { _epHndl = epHndl; }

    bool IsPing() const;
    bool IsRequest() const;
    bool IsResponse() const;
    bool IsEmpty() const;

    bool IsConfirmable() const { return ( _type == MsgType::CON ); }
    bool HasMsgId() const { return ( _id != 0xFFFF ); }
    bool IsNonConfirmable() const { return ( _type == MsgType::NON ); }
    bool IsAck() const { return ( _type == MsgType::ACK ); }
    bool IsRst() const { return ( _type == MsgType::RST ); }

    bool IsDuplicate() const { return ( IsBitSet( _status, MsgBitmaskDuplicate ) ); }
    void SetDuplicate( bool setValue );

    bool IsTimeout() const { return ( IsBitSet( _status, MsgBitmaskTimeout ) ); }
    void SetTimeout( bool setValue );

    bool IsAcknowledged() const { return ( IsBitSet( _status, MsgBitmaskAcknowledged ) ); }
    void SetAcknowledged( bool setValue );

    bool IsCancelled() const { return ( IsBitSet( _status, MsgBitmaskCancelled ) ); }
    void SetCancelled( bool setValue );

    bool IsRejected() const { return ( IsBitSet( _status, MsgBitmaskRejected ) ); }
    void SetRejected( bool setValue );

    bool IsLastRes() const { return ( IsBitSet( _status, MsgBitmaskLastResponse ) ); }
    void SetLastRes( bool setValue );

    bool HasBlockOption() const;

    u32 GetHashBySrcEpAndMid();
    u32 GetHashBySrcEpAndTkn();

    u32 GetHashByDstEpAndMid();
    u32 GetHashByDstEpAndTkn();

    u32 GetHashBySrcEpAndUri();
    u32 GetHashByDstEpAndUri();

    Msg* UpdateForResFromReq( MsgCode msgCode, Msg *req );
    Msg* UpdateForResFromRes( MsgCode msgCode, Msg *res );

    void * operator new ( size_t size );
    void operator   delete ( void *ptr );
};

typedef Msg Request;
typedef Msg Response;
}