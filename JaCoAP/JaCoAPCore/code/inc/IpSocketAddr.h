#pragma once

#include <IpAddress.h>
#include <ja_types.h>

namespace JaCoAP
{

class IpSocketAddr
{
	IpAddress _ipAddr;
	u16 _port = 0;

public:
	IpSocketAddr();

	IpSocketAddr(const IpAddress& ipAddr, u16 port);
	IpSocketAddr(const u32 ipAddr, u16 port);
	IpSocketAddr(const u8* ipAddr, u16 port);
	~IpSocketAddr();

	u16 Port() const
	{
		return _port;
	}
	void Port(u16 port)
	{
		_port = port;
	}

	IpAddress* IpAddr()
	{
		return &_ipAddr;
	}
	void IpAddr(const IpAddress& ipAddr)
	{
		_ipAddr = ipAddr;
	}

	bool operator ==(const IpSocketAddr& other);

	void Print();

	u32 GetHashValue();
};

}
