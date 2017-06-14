#include <ByteArray.h>
#include <DataChannel.h>
#include <IpSocketAddr.h>
#include <MsgCodec.h>
#include <NetIOMgr.h>

namespace JaCoAP
{
ByteArray _gs_outgoingByteArray( JA_COAP_CONFIG_MAX_OUT_PACKET_SIZE );
NetIO::NetIO ()
{
    _outPacket.SetByteArray( &_gs_outgoingByteArray );
}

void NetIO::Init() const
{
}

ErrCode NetIO::AddEp( IpSocketAddr const &ipSocketAddr ) const
{
    if( _dataChannel == nullptr )
    {
        return ( ErrCode::INVALID_ARG );
    }

    auto status = _dataChannel->AddEp( ipSocketAddr );

    if( status != ErrCode::OK )
    {
        return ( status );
    }

    return ( ErrCode::OK );
}

ErrCode NetIO::SendMsg( Msg *msg )
{
    if( ( _dataChannel == nullptr ) || ( msg == nullptr ) )
    {
        return ( ErrCode::INVALID_ARG );
    }

    auto status = MsgCodec::Encode( *msg, *_outPacket.GetByteArray() );

    if( status != ErrCode::OK )
    {
        return ( status );
    }

    _outPacket.SetRemoteSocketAddr( *msg->DstSocketAddr() )->SetEpHndl( msg->EpHndl() );

    _dataChannel->SendPacket( _outPacket, -1 );

    return ( ErrCode::OK );
}

ErrCode NetIO::SendBadOptionRes( IpSocketAddr const &remoteEp
                               , Msg &req )
{
    auto tknLen   = req.TknLen();
    auto msgBytes = _outPacket.getByteArrayPtr();

    msgBytes[0]  = 0b01100000;
    msgBytes[0] |= ( tknLen & 0x0F );
    msgBytes[1]  = u8( MsgCode::BAD_OPTION_402 );
    msgBytes[2]  = ( req.Id() >> 8 ) & 0xFF;
    msgBytes[3]  = req.Id() & 0xFF;

    u16 curByteIdx = 4;

    for( auto i = 0; i < tknLen; i++ )
    {
        msgBytes[curByteIdx] = u8( req.Tkn() >> i * 8 & 0xFF );
        curByteIdx++;
    }

    _outPacket.GetByteArray()->SetLen( curByteIdx );
    _outPacket.SetRemoteSocketAddr( remoteEp );

    _dataChannel->SendPacket( _outPacket, -1 ); // -1 is infinite timeout
    return ( ErrCode::OK );
}

ErrCode NetIO::SendRstMsg( IpSocketAddr const &remoteEp, u16 msgId )
{
    return ( SendEmptyMsg( MsgType::RST
                         , remoteEp
                         , msgId ) );
}

ErrCode NetIO::SendAckMsg( IpSocketAddr const &remoteEp, u16 msgId )
{
    return ( SendEmptyMsg( MsgType::ACK, remoteEp, msgId ) );
}

ErrCode NetIO::SendEmptyMsg( MsgType msgType, IpSocketAddr const &remoteEp, u16 msgId )
{
    auto msgBytes = _outPacket.getByteArrayPtr();

    msgBytes[0] = ( msgType == MsgType::ACK ) ? 0x60 : 0x70;
    msgBytes[1] = 0; // Token Length
    msgBytes[2] = ( msgId >> 8 ) & 0xFF;
    msgBytes[3] = msgId & 0xFF;

    _outPacket.GetByteArray()->SetLen( 4 );
    _outPacket.SetRemoteSocketAddr( remoteEp );

    _dataChannel->SendPacket( _outPacket, -1 ); // -1 is infinite timeout

    return ( ErrCode::OK );
}
}