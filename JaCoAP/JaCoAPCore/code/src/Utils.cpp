#include <IpSocketAddr.h>
#include <OptionsSet.h>
#include <Utils.h>
#include <cstdint>
#include "JaCoapIF.h"

FILE_NAME("Utils.cpp");

namespace JaCoAP
{

constexpr uint16_t UTIL_TEMP_BUF_SIZE = 512;

ByteArray _tempBuf { UTIL_TEMP_BUF_SIZE }; // used for parsing URI path from option set

#define FNV_32_PRIME ((Fnv32_t)0x01000193)

uint32_t Utils::Hash(pvoid buf, uint32_t bufLen, uint32_t hval)
{
	auto bp = static_cast<unsigned char *>(buf); /* start of buffer */
	auto be = bp + bufLen; /* beyond end of buffer */

	/*FNV-1 hash each octet in the buffer */
	while (bp < be)
	{
		/* xor the bottom with the current octet */
		hval ^= uint32_t(*bp++);

		/* multiply by the 32 bit FNV magic prime mod 2^32 */
#if defined(NO_FNV_GCC_OPTIMIZATION)
		hval *= FNV_32_PRIME;
#else
		hval += (hval << 1) + (hval << 4) + (hval << 7) + (hval << 8) + (hval << 24);
#endif
	}

	/* return our new hash value */
	return hval;
}

uint32_t Utils::Hash(pvoid buf, uint32_t bufLen)
{
	return Hash(buf, bufLen, HASH_INIT_VALUE);
}

uint32_t Utils::HashByUri(ByteArray& uri, IpSocketAddr& ipSktAddr)
{
	uint32_t ipHashVal = ipSktAddr.GetHashValue();
	uint32_t hashVal = Hash(uri.GetArray(), uri.GetLen(), ipHashVal);

#if 0
	printf( "Utils::HashByUri=> uri[" );
	for (auto i = 0; i < uri.GetLen(); i++)
	{
		printf( "%c", (*uri)[i] );
	}

	printf("] Ip Hash[%u], Final Hash[%u]\n", ipHashVal, hashVal);
#endif
	return hashVal;
}

uint32_t Utils::HashByUri(Msg* msg, IpSocketAddr& ipSktAddr)
{
	_tempBuf.SetLen(0);
	msg->GetOptionSet()->GetUriPathString(_tempBuf);

	return HashByUri(_tempBuf, ipSktAddr);
}

Msg* Utils::getEmptyMsg(MsgType msgType, Msg* src)
{
	return getEmptyMsg(msgType, src->Id(), *src->SrcSocketAddr(), src->EpHndl());
}

Msg* Utils::getEmptyMsg(MsgType msgType, u16 id, IpSocketAddr const& socketAddr, u8 epHndl)
{
	auto _emptyMsg = mnew Msg();

	_emptyMsg->Type(msgType);
	_emptyMsg->Id(id);
	_emptyMsg->DstSocketAddr(socketAddr);
	_emptyMsg->EpHndl(epHndl);

	return _emptyMsg;
}

void Utils::printMsg(Msg& msg) {
}

}

