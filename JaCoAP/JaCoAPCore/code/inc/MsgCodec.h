#pragma once

#include "ja_types.h"
#include "Msg.h"
#include "ByteArray.h"
#include "ErrorCodes.h"

namespace JaCoAP
{
class MsgCodec
{
public:
    MsgCodec();

    static void    ParseOnly(ByteArray &byteArray, MsgHdr &msg);
    static ErrCode Decode(ByteArray &byteArray, Msg &msg);
    static ErrCode Encode(Msg &msg, ByteArray &byteArray);

private:
    static ErrCode EncodeOptions(Msg &msg, uint8_t *buffer, uint16_t *bufLen);
    static ErrCode ParseOptions(Msg &msg, u8 *inBuffer, u16 optionLen, u8 **payloadStart);
    static ErrCode ParseOnlyOptions(u8 *inBuffer, u16 optionLen, u8 **payloadStart);
};
}