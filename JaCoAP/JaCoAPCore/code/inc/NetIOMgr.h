#pragma once

#include <ErrorCodes.h>
#include <ja_types.h>
#include <JaCoapConfig.h>
#include <Msg.h>
#include <Packet.h>

namespace JaCoAP
{
class DataChannel;
}

namespace JaCoAP
{
class NetIO
{
    DataChannel    *_dataChannel = nullptr;
    Packet         _outPacket;

public:
    NetIO ();

    void Init() const;

    ErrCode AddEp( IpSocketAddr const &ipSocketAddr ) const;

    void SetDataChannel( DataChannel *dataChannel ) { _dataChannel = dataChannel; }

    DataChannel* GetDataChannel() const { return ( _dataChannel ); }

    ErrCode SendMsg( Msg *msg );

    ErrCode SendBadOptionRes( IpSocketAddr const &remoteEp, Msg &req );
    ErrCode SendRstMsg( IpSocketAddr const &remoteEp, u16 msgId );
    ErrCode SendAckMsg( IpSocketAddr const &remoteEp, u16 msgId );

private:
    ErrCode SendEmptyMsg( MsgType msgType, IpSocketAddr const &remoteEp, u16 msgId );
};
}