#include <ByteArray.h>
#include <CoapMgr.h>
#include <CoapServer.h>
#include <CoapStack.h>
#include <DataChannel.h>
#include <Inbox.h>
#include <IpAddress.h>
#include <IpSocketAddr.h>
#include <ja_types.h>
#include <NetIOMgr.h>
#include <Packet.h>
#include <ServerMsgDeliverer.h>

namespace JaCoAP
{
ServerMsgDeliverer _serverMsgDeliverer{};
CoapServer::CoapServer( DataChannel *dataChannel )
{
    CoapMgr::GetNetIO()->SetDataChannel( dataChannel );
}

CoapServer::~CoapServer ()
{
}

ErrCode CoapServer::Start() const
{
    CoapMgr::Init();
    CoapMgr::GetCoapStack()->setMsgDeliverer( &_serverMsgDeliverer );

    return ( ErrCode::OK );
}

ErrCode CoapServer::AddEp( IpSocketAddr const &ipSocketAddr ) const
{
    return ( CoapMgr::GetNetIO()->AddEp( ipSocketAddr ) );
}

void CoapServer::Loop() const
{
    auto dataChannel = CoapMgr::Inst()->GetNetIO()->GetDataChannel();

    if( dataChannel == nullptr )
    {
        return;
    }

    Packet packet{};
    u32 timeout = -1;

    while( true )
    {
        auto status = static_cast<ErrCode>( dataChannel->RecvPacket( packet, timeout ) );

        if( ( status == ErrCode::OK ) && ( packet.GetByteArray()->GetLen() < 4 ) )
        {
            status = ErrCode::DGRAM_TOO_SMALL;
        }

        if( ( status == ErrCode::OK ) && ( ( packet.GetByteArray()->GetArray()[0] >> 6 & 0x03 ) != 0x01 ) )
        {
            status = ErrCode::INVALID_VERSION;
        }

        if( status == ErrCode::OK )
        {
            auto remoteEp = packet.GetRemoteSocketAddr();
            auto remoteIp = remoteEp->IpAddr();
            // printf("Packet Received from %u.%u.%u.%u Port %u\n", (*remoteIp)[0], (*remoteIp)[1], (*remoteIp)[2], (*remoteIp)[3], remoteEp->Port());

            CoapMgr::GetInbox()->receivePacket( &packet );
            // CoapMgr::GetTransMgr()->CleanupTrans();
        }
    }
}
}