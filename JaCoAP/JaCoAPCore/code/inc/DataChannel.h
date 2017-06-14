#ifndef __DATA_CHANNEL_H__
#define __DATA_CHANNEL_H__

#include "ja_types.h"
#include "Packet.h"
#include "ErrorCodes.h"

namespace JaCoAP {

class DataChannel {
public:
	virtual ~DataChannel( ) = default;

	virtual ErrCode AddEp( IpSocketAddr const&  socketAddr ) = 0;

	virtual ErrCode RecvPacket( Packet& packet, u32 timeoutMs ) = 0;
	virtual ErrCode SendPacket( Packet& packet, u32 timeoutMs ) = 0;
};

}

#endif/*__DATA_CHANNEL_H__*/
