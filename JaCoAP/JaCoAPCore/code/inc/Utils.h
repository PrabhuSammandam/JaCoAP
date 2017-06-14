#pragma once

#include <ByteArray.h>
#include <ja_types.h>
#include <Msg.h>
#include <cstdint>

namespace JaCoAP
{
constexpr uint32_t HASH_INIT_VALUE = 0x811c9dc5;

#define PRINT_MSG(__msg_to_print__) Utils::printMsg(__msg_to_print__)

class Utils
{
public:
    static uint32_t Hash( pvoid buf, uint32_t bufLen, uint32_t hashVal );
    static uint32_t Hash( pvoid buf, uint32_t bufLen );

    static uint32_t HashByUri( ByteArray &uri, IpSocketAddr &ipSktAddr );
    static uint32_t HashByUri( Msg *msg, IpSocketAddr &ipSktAddr );

    static Msg* getEmptyMsg( MsgType msgType, Msg *src );
    static Msg* getEmptyMsg( MsgType msgType, u16 id, IpSocketAddr const &socketAddr, u8 epHndl );

    static void printMsg(Msg& msg);
};
}
