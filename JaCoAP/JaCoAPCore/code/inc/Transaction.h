#pragma once

#include <ja_types.h>
#include <cstddef>

namespace JaCoAP
{
class Block2Info;
class BlockOption;
class Msg;
}

namespace JaCoAP
{
constexpr auto TRANS_FIELD_ORIGIN_BITMASK   = 0x0100;
constexpr auto TRANS_FIELD_COMPLETE_BITMASK = 0x0200;

enum class TransOrigin
{
    LOCAL  = 0,
    REMOTE = 1
};

class Transaction
{
private:
    Msg            *_request                = nullptr;
    Msg            *_response               = nullptr;
    Msg            *_curRequest             = nullptr;
    Msg            *_curResponse            = nullptr;
    u32            _timeStamp               = -1;
    u32            _timeOut                 = -1;
    u8             _epHndl                  = 0xFF;
    u8             _retransCnt              = 0;
    u16            _data                    = 0;
    BlockOption    *_block1                 = nullptr;
    Block2Info     *_reqBlockTransferStatus = nullptr;
    Block2Info     *_resBlockTransferStatus = nullptr;

public:
    Transaction ();

    Transaction( TransOrigin origin, Msg *msg );

    ~Transaction ();

    void Reset();

    Transaction( const Transaction &other );

    Transaction( Transaction &&other ) noexcept;
    Transaction & operator = ( Transaction other );

    TransOrigin Origin() const;
    void        Origin( TransOrigin origin );

    bool IsOriginLocal() const { return ( Origin() == TransOrigin::LOCAL ); }

    bool IsOriginRemote() const { return ( Origin() == TransOrigin::REMOTE ); }

    Msg* Req() const { return ( _request ); }

    void Req( Msg *msg ) { _request = msg; }

    Msg* Res() const { return ( _response ); }

    void Res( Msg *msg ) { _response = msg; }

    Msg* CurReq() const { return ( _curRequest ); }

    void CurReq( Msg *req ) { _curRequest = req; }

    Msg* CurRes() const { return ( _curResponse ); }

    void CurRes( Msg *res ) { _curResponse = res; }

    u8 RetransCnt() const { return ( _retransCnt ); }

    void RetransCnt( u8 count ) { _retransCnt = count; }

    void Timestamp( u32 timeStamp ) { _timeStamp = timeStamp; }

    u32 Timestamp() const { return ( _timeStamp ); }

    bool IsComplete() const { return ( IsBitSet( _data, TRANS_FIELD_COMPLETE_BITMASK ) ); }
    void Complete( bool completeValue );

    BlockOption* GetBlock1() const { return ( _block1 ); }
    void       SetBlock1( BlockOption *block1 );

    Block2Info* GetReqBlockTransferStatus() const { return ( _reqBlockTransferStatus ); }
    void      SetReqBlockTransferStatus( Block2Info *reqBlockTransferStatus );

    Block2Info* GetResBlockTransferStatus() const { return ( _resBlockTransferStatus ); }
    void      SetResBlockTransferStatus( Block2Info *resBlockTransferStatus );

    void SendAccept();
    void SendReject();
    void SendResponse( Msg *response );

    void MarkTime() const;

    void * operator new ( size_t size );
    void operator   delete ( void *ptr );
};
}