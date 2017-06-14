#pragma once

#include <ByteArray.h>
#include <DataChannel.h>
#include <IpSocketAddr.h>
#include <ja_types.h>
#include <winsock2.h>
#include "ErrorCodes.h"

constexpr auto DEFAULT_BUFLEN = 1024;
constexpr auto MAX_IP_SOCKET_ADDR = 3;

struct IpSocketAddrEntry
{
	JaCoAP::IpSocketAddr ipSocketAddr;
	SOCKET socket;
	bool used;
};

class ServerDataChannel: public JaCoAP::DataChannel
{
private:
	IpSocketAddrEntry _ipSocketList[MAX_IP_SOCKET_ADDR];
	u8 _socketCount = 0;
	JaCoAP::ByteArray _byteArray { DEFAULT_BUFLEN };

public:
	ServerDataChannel();

	JaCoAP::ErrCode Init();

	JaCoAP::ErrCode AddEp(JaCoAP::IpSocketAddr const& socketAddr) override;

	JaCoAP::ErrCode RecvPacket(JaCoAP::Packet& packet, u32 timeoutMs) override;
	JaCoAP::ErrCode SendPacket(JaCoAP::Packet& packet, u32 timeoutMs) override;

private:
	IpSocketAddrEntry* FindEntry(JaCoAP::IpSocketAddr& socketAddr);
};
