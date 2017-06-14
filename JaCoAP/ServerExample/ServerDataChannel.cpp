#include "ServerDataChannel.h"

#include <stdio.h>
#include <winsock2.h>
#include "Packet.h"
#include "Debug.h"

#include <IpSocketAddr.h>

#pragma comment(lib, "Ws2_32.lib")

using namespace JaCoAP;

WSADATA wsa {};

ServerDataChannel::ServerDataChannel()
{
	for (auto i = 0; i < MAX_IP_SOCKET_ADDR; i++)
	{
		_ipSocketList[i].used = false;
	}

	Init();
}

ErrCode ServerDataChannel::Init()
{
	dbg("\nInitialising Winsock...");

	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		printf("Failed. Error Code : %d", WSAGetLastError());
		return ErrCode::INVALID_ARG;
	}

	printf("Initialized.\n");

	return ErrCode::OK;
}

ErrCode ServerDataChannel::AddEp(IpSocketAddr const& socketAddr)
{
	IpSocketAddrEntry* ipSocketEntry = nullptr;
	struct sockaddr_in server {};

	for (auto i = 0; i < MAX_IP_SOCKET_ADDR; i++)
	{
		if (_ipSocketList[i].used == false)
		{
			ipSocketEntry = &_ipSocketList[i];
			break;
		}
	}

	if (ipSocketEntry == nullptr)
	{
		return ErrCode::INVALID_ARG;
	}

	if ((ipSocketEntry->socket = socket( AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET)
	{
		printf("Could not create socket : %d", WSAGetLastError());

		return ErrCode::INVALID_ARG;
	}

	printf("Socket created.\n");

	//Prepare the sockaddr_in structure
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(socketAddr.Port());

	//Bind
	if (bind(ipSocketEntry->socket, (struct sockaddr*) &server, sizeof(server)) == SOCKET_ERROR)
	{
		closesocket(ipSocketEntry->socket);
		printf("Bind failed with error code : %d", WSAGetLastError());
		return ErrCode::INVALID_ARG;
	}
	ipSocketEntry->used = true;
	_socketCount++;

	return ErrCode::OK;
}

ErrCode ServerDataChannel::RecvPacket(Packet& packet, u32 timeoutMs)
{
	fd_set readfds {};
	int maxFd { 0 };
	SOCKADDR_IN remoteSocketAddr {};
	int remoteSocketAddrLen = sizeof(SOCKADDR_IN);
	struct timeval timeOutStruct {};
	struct timeval*timeoutValue {};

	if (_socketCount == 0)
	{
		return ErrCode::INVALID_ARG;
	}

	FD_ZERO(&readfds);

	for (auto i = 0; i < MAX_IP_SOCKET_ADDR; i++)
	{
		if (_ipSocketList[i].used == true)
		{
			FD_SET(_ipSocketList[i].socket, &readfds);
			if (_ipSocketList[i].socket > maxFd)
			{
				maxFd = _ipSocketList[i].socket;
			}
		}
	}

	if (timeoutMs != 0xFFFFFFFF)
	{
		timeOutStruct.tv_usec = timeoutMs * 1000;
		timeoutValue = &timeOutStruct;
	}
	else
	{
		timeoutValue = nullptr;
	}

	int selectReturn = select(maxFd + 1, &readfds, NULL, NULL, timeoutValue);

	if ((selectReturn == SOCKET_ERROR) || (selectReturn == 0))
	{
		return ErrCode::INVALID_ARG;
	}

	for (auto i = 0; i < MAX_IP_SOCKET_ADDR; i++)
	{
		if (FD_ISSET(_ipSocketList[i].socket, &readfds))
		{
			auto receivedBytesLen = recvfrom(_ipSocketList[i].socket, (char*) (_byteArray.GetArray()), DEFAULT_BUFLEN, 0,
			                                 (SOCKADDR*) &remoteSocketAddr, &remoteSocketAddrLen);

			if (receivedBytesLen == SOCKET_ERROR)
			{
				return ErrCode::INVALID_ARG;
			}

			_byteArray.SetLen(receivedBytesLen);

			IpSocketAddr remoteEndpoint(ntohl(remoteSocketAddr.sin_addr.S_un.S_addr), ntohs(remoteSocketAddr.sin_port));

			packet.SetRemoteSocketAddr(remoteEndpoint);
			packet.SetByteArray(&_byteArray);
			packet.SetEpHndl(i);

			return ErrCode::OK;
		}
	}

	return ErrCode::OK;

#if 0

	if ( timeoutMs != 0xFFFFFFFF )
	{
		if ( setsockopt( _socket, SOL_SOCKET, SO_RCVTIMEO, (const char*) &timeoutMs, sizeof( timeoutMs ) ) )
		{
			status = 3;
		}
	}

	if ( (status == 0) && (receivedBytesLen = recvfrom( _socket,
							(char*) (*_byteArray),
							DEFAULT_BUFLEN,
							0,
							(SOCKADDR*) &remoteSocketAddr, &remoteSocketAddrLen )) == SOCKET_ERROR )
	{

		int error = WSAGetLastError( );

		if ( WSAETIMEDOUT == error )
		{
			//printf( "socket recv timeout\n");
			status = 2;
		}
		else
		{
			status = 1;
			printf( "socket receive error happened, error code %d\n", error );
		}
	}

	if ( status == 0 )
	{
		_byteArray.GetLen( receivedBytesLen );

		IpSocketAddr remoteEndpoint( ntohl( remoteSocketAddr.sin_addr.S_un.S_addr ), ntohs( remoteSocketAddr.sin_port ) );

		packet.SetRemoteEndPoint( remoteEndpoint );
		packet.SetByteArray( _byteArray );
	}
	return status;
#endif
}

ErrCode ServerDataChannel::SendPacket(Packet& packet, u32 timeoutMs)
{
	SOCKADDR_IN sockRemoteAddr {};
	int sockRemoteAddrLen = sizeof(SOCKADDR_IN);
	auto remoteEp = packet.GetRemoteSocketAddr();
	auto byteArray = packet.GetByteArray();

	if (packet.GetEpHndl() == 0xFF)
	{
		return ErrCode::INVALID_ARG;
	}

	auto socket = _ipSocketList[packet.GetEpHndl()].socket;

	sockRemoteAddr.sin_family = AF_INET;
	sockRemoteAddr.sin_port = htons(remoteEp->Port());
	sockRemoteAddr.sin_addr.S_un.S_addr = htonl(remoteEp->IpAddr()->AsU32());

	if (sendto(socket, reinterpret_cast<const char*>(byteArray->GetArray()), byteArray->GetLen(), 0, reinterpret_cast<SOCKADDR*>(&sockRemoteAddr),
	           sockRemoteAddrLen) == SOCKET_ERROR)
	{
		int error = WSAGetLastError();

		printf("socket receive error happened, error code %d\n", error);

		return ErrCode::INVALID_ARG;
	}

	return ErrCode::OK;
}

IpSocketAddrEntry * ServerDataChannel::FindEntry(IpSocketAddr & socketAddr)
{
	IpSocketAddrEntry* ipSocketEntry = nullptr;

	for (auto i = 0; i < MAX_IP_SOCKET_ADDR; i++)
	{
		if (_ipSocketList[i].ipSocketAddr == socketAddr)
		{
			ipSocketEntry = &_ipSocketList[i];
		}
	}

	return ipSocketEntry;
}
