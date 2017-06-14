#include <cstring>
#include "JaCoapConfig.h"
#include "MsgCodec.h"
#include "JaCoapIF.h"

FILE_NAME("MsgCodec.cpp");

namespace JaCoAP
{
MsgCodec::MsgCodec ()
{
}

void MsgCodec::ParseOnly( ByteArray &byteArray, MsgHdr &msg )
{
    msg.Error( ErrCode::OK );

    if( ( byteArray.GetArray() == nullptr ) || byteArray.IsEmpty() )
    {
        msg.Error( ErrCode::INVALID_ARG );
        return;
    }

    if( byteArray.GetLen() < 4 )
    {
        msg.Error( ErrCode::MSG_FORMAT_ERROR );
        return;
    }

    if( ( byteArray[0] >> 6 ) != 0x01 )
    {
        msg.Error( ErrCode::INVALID_VERSION );
        return;
    }

    msg.Type( static_cast<MsgType>( ( byteArray[0] >> 4 ) & 0x03 ) );
    u8 tknLen = byteArray[0] & 0x0F;

    if( tknLen > 8 )
    {
        msg.Error( ErrCode::MSG_FORMAT_ERROR );
        return;
    }

    msg.Code( static_cast<MsgCode>( byteArray[1] ) );

    /* Section 4.1: An Empty message has the Code field set to 0.00.
     * The Token Length field MUST be set to 0 and bytes of data
     * MUST NOT be present after the Message ID field. If there
     * are any bytes, they MUST be processed as a message format error.*/
    if( msg.Code() == MsgCode::EMPTY )
    {
        if( ( byteArray.GetLen() != 4 ) || ( tknLen != 0 ) )
        {
            msg.Error( ErrCode::MSG_FORMAT_ERROR );
            return;
        }
    }

    u8 msgCodeClass = u8( msg.Code() ) >> 5;

    if( ( msgCodeClass == 1 ) || ( msgCodeClass == 6 ) || ( msgCodeClass == 7 ) )
    {
        msg.Error( ErrCode::MSG_FORMAT_ERROR );
        return;
    }

    msg.Id( static_cast<u16>( ( byteArray[2] << 8 ) | byteArray[3] ) );

    u16 currByteIdx = 4;

    if( byteArray.GetLen() == currByteIdx )
    {
        return;
    }

    currByteIdx += tknLen;

    if( byteArray.GetLen() == currByteIdx )
    {
        return;
    }

    u8 *payloadStart{ nullptr };

    ErrCode status = ParseOnlyOptions( &( byteArray.GetArray() )[currByteIdx],
            ( byteArray.GetLen() - currByteIdx ), &payloadStart );

    if( status != ErrCode::OK )
    {
        msg.Error( status );
        return;
    }

    msg.PayloadStart( nullptr );
    msg.PayloadLen( 0 );

    if( payloadStart != nullptr )
    {
        u16 payloadLen = u16(
            byteArray.GetLen() - ( payloadStart - ( byteArray.GetArray() ) ) );

        if( payloadLen > JA_COAP_CONFIG_MAX_PAYLOAD_SIZE )
        {
            msg.Error( ErrCode::PAYLOAD_TOO_LARGE );
            return;
        }

        msg.PayloadStart( payloadStart );
        msg.PayloadLen( payloadLen );
    }
}

ErrCode MsgCodec::Decode( ByteArray &byteArray, Msg &msg )
{
    msg.Reset();
    msg.Type( static_cast<MsgType>( ( byteArray[0] >> 4 ) & 0x03 ) );
    msg.TknLen( ( byteArray[0] & 0x0F ) );
    msg.Code( static_cast<MsgCode>( byteArray[1] ) );
    msg.Id( static_cast<u16>( ( byteArray[2] << 8 ) | byteArray[3] ) );

    u16 currByteIdx = 4;

    if( byteArray.GetLen() == currByteIdx )
    {
        return ( ErrCode::OK );
    }

    if( msg.TknLen() > 0 )
    {
        msg.Tkn( &( byteArray.GetArray() )[currByteIdx], msg.TknLen() );
    }

    currByteIdx += msg.TknLen();

    if( byteArray.GetLen() == currByteIdx )
    {
        return ( ErrCode::OK );
    }

    u8 *payloadStart{ nullptr };

    auto status = ParseOptions( msg, &( byteArray.GetArray() )[currByteIdx],
            ( byteArray.GetLen() - currByteIdx ), &payloadStart );

    if( status != ErrCode::OK )
    {
        return ( status );
    }

    if( payloadStart != nullptr )
    {
        u16 payloadLen = u16(
            byteArray.GetLen() - ( payloadStart - ( byteArray.GetArray() ) ) );

        if( payloadLen > JA_COAP_CONFIG_MAX_PAYLOAD_SIZE )
        {
            status = ErrCode::PAYLOAD_TOO_LARGE;
            goto exit_label;
        }

        if( payloadLen > 0 )
        {
            auto newPay = mnew ByteArray( payloadLen );
            ::memcpy( newPay->GetArray(), payloadStart, payloadLen );
            newPay->SetLen( payloadLen );
            msg.Payload( newPay );
        }
    }

exit_label:

    if( status != ErrCode::OK )
    {
        msg.GetOptionSet()->FreeOptions();
    }

    return ( ErrCode::OK );
}

ErrCode MsgCodec::Encode( Msg &msg, ByteArray &byteArray )
{
    u8 *outBytes = byteArray.GetArray();

    outBytes[0]  = 0x40;
    outBytes[0] |= ( u8( msg.Type() ) & 0x03 ) << 4;
    outBytes[0] |= ( ( msg.TknLen() ) & 0x0F );

    outBytes[1] = u8( msg.Code() );
    outBytes[2] = ( msg.Id() >> 8 ) & 0xFF;
    outBytes[3] = msg.Id() & 0xFF;

    u16 curByteIdx = 4;

    for( auto i = 0; i < msg.TknLen(); i++ )
    {
        outBytes[curByteIdx] = u8( msg.Tkn() >> i * 8 & 0xFF );
        curByteIdx++;
    }

    curByteIdx += msg.TknLen();

    if( msg.GetOptionSet()->GetNoOfOptions() > 0 )
    {
        uint16_t optionLen = 0;
        EncodeOptions( msg, &outBytes[curByteIdx], &optionLen );
        curByteIdx += optionLen;
    }

    auto plBuf = msg.Payload();

    if( ( plBuf != nullptr ) && ( plBuf->GetLen() > 0 ) )
    {
        outBytes[curByteIdx++] = 0xFF;

        ::memcpy( &outBytes[curByteIdx], plBuf->GetArray(), plBuf->GetLen() );

        curByteIdx += plBuf->GetLen();
    }

    byteArray.SetLen( curByteIdx );

    return ( ErrCode::OK );
}

ErrCode MsgCodec::EncodeOptions( Msg &msg, uint8_t *outBuff, uint16_t *bufLen )
{
    u16  curByteIdx   = 0;  // Current "Write" Position while packing the options array to the byte array
    u16  lastOptionNo = 0;

    *bufLen = 0;

    auto it = msg.GetOptionSet()->OptList()->begin();

    auto curOption = *it;

    do
    {
        u16 curDelta = curOption->GetNo() - lastOptionNo;
        lastOptionNo = curOption->GetNo();

        u16 curOptionLen      = curOption->GetLen();
        u16 curOptionStartIdx = curByteIdx;
        curByteIdx++;
        outBuff[curOptionStartIdx] = 0;

        // Delta Bytes
        if( curDelta < 13 )
        {
            outBuff[curOptionStartIdx] |= u8( curDelta ) << 4;
        }
        else if( curDelta < 269 )
        {
            outBuff[curOptionStartIdx] |= u8( 13 ) << 4;
            outBuff[curByteIdx]         = u8( curDelta ) - 13;
            curByteIdx++;
        }
        else
        {
            outBuff[curOptionStartIdx] |= u8( 14 ) << 4;

            outBuff[curByteIdx] = u8( ( curDelta - 269 ) >> 8 );
            curByteIdx++;
            outBuff[curByteIdx] = u8( ( curDelta - 269 ) & 0xff );
            curByteIdx++;
        }

        // Length Bytes
        if( curOptionLen < 13 )
        {
            outBuff[curOptionStartIdx] |= u8( curOptionLen );
        }
        else if( curOptionLen < 269 )
        {
            outBuff[curOptionStartIdx] |= u8( 13 );
            outBuff[curByteIdx]         = u8( curOptionLen ) - 13;
            curByteIdx++;
        }
        else
        {
            outBuff[curOptionStartIdx] |= u8( 14 );

            outBuff[curByteIdx] = u8( ( curOptionLen - 269 ) >> 8 );
            curByteIdx++;
            outBuff[curByteIdx] = u8( ( curOptionLen - 269 ) & 0xff );
            curByteIdx++;
        }

        // Option Values
        auto optValue = curOption->GetVal();

        for( auto t = 0; t < curOptionLen; t++ )
        {
            outBuff[curByteIdx] = optValue[t];
            curByteIdx++;
        }

        ++it;

        if( it == msg.GetOptionSet()->OptList()->end() )
        {
            break;
        }

        curOption = *it;
    } while( true );

    *bufLen = curByteIdx;

    return ( ErrCode::OK );
}

ErrCode MsgCodec::ParseOptions( Msg &msg, u8 *inByteArray, u16 optionLen, u8 **payloadStart )
{
    ErrCode status        = ErrCode::OK;
    u16     curByteIndex  = 0;
    u16     lastOptionNum = 0;

    *payloadStart = nullptr;

    if( inByteArray[0] == 0xFF )
    {
        status = ErrCode::MSG_FORMAT_ERROR;
        goto success;
    }

    while( curByteIndex < optionLen )
    {
        // check for payload marker, if it is there then mark the start of payload and return
        if( inByteArray[curByteIndex] == 0xFF )
        {
            // at least one byte payload must follow to the payload marker
            if( ( optionLen - curByteIndex ) < 2 )
            {
                status = ErrCode::MSG_FORMAT_ERROR;
                goto success;
            }
            else
            {
                curByteIndex++;                 // skip payload marker
                *payloadStart = &inByteArray[curByteIndex];
                status        = ErrCode::OK;
                goto success;
            }
        }
        else
        {
            u16 option_delta  = inByteArray[curByteIndex] >> 4;            // initial delta in upper 4 bits
            u16 option_length = inByteArray[curByteIndex] & 0x0F;             // initial length in lower 4 bits
            curByteIndex++;             // skip the initial delta & length byte

            if( option_delta == 13 )
            {
                // An 8-bit unsigned integer follows the initial byte and indicates the Option Delta minus 13.
                option_delta = inByteArray[curByteIndex] + 13;
                curByteIndex++;                 // skip 1 byte big delta
            }
            else if( option_delta == 14 )
            {
                // A 16-bit unsigned integer in network byte order follows the initial byte and indicates the Option Delta minus 269.
                option_delta = ( ( u16( inByteArray[curByteIndex] ) << 8 )
                    | u16( inByteArray[curByteIndex] ) ) + 269;
                curByteIndex += 2;
            }
            else if( option_delta == 15 )
            {
                // Reserved for the Payload Marker.
                status = ErrCode::MSG_FORMAT_ERROR;
                goto success;
            }

            if( option_length == 13 )
            {
                // An 8-bit unsigned integer precedes the Option Value and indicates the Option Length minus 13.
                option_length = inByteArray[curByteIndex] + 13;
                curByteIndex++;                 // skip 1 byte big delta
            }
            else if( option_length == 14 )
            {
                // A 16-bit unsigned integer in network byte order precedes the Option Value and indicates the Option Length minus 269.
                option_length = ( ( u16( inByteArray[curByteIndex] ) << 8 )
                    | u16( inByteArray[curByteIndex] ) ) + 269;
                curByteIndex += 2;
            }
            else if( option_length == 15 )
            {
                // Reserved for future use.
                status = ErrCode::MSG_FORMAT_ERROR;
                goto success;
            }

            // if current option length is greater than max option size or
            // current option length is greater than remaining payload
            if( ( option_length > JA_COAP_CONFIG_MAX_OPTION_SIZE )
              || ( ( optionLen - curByteIndex ) < option_length ) )
            {
                status = ErrCode::MSG_FORMAT_ERROR;
                goto success;
            }

            lastOptionNum += option_delta;             // encode option delta

            status = msg.GetOptionSet()->AddOption( lastOptionNum, option_length,
                    &inByteArray[curByteIndex] );

            if( status != ErrCode::OK )
            {
                goto success;
            }

            curByteIndex += option_length;
        }
    }

success:

    if( status != ErrCode::OK )
    {
        msg.GetOptionSet()->FreeOptions();
    }

    return ( status );
}

ErrCode MsgCodec::ParseOnlyOptions( u8 *inByteArray, u16 optionLen, u8 **payloadStart )
{
    u16 curByteIndex  = 0;
    u16 lastOptionNum = 0;

    *payloadStart = nullptr;

    if( inByteArray[0] == 0xFF )
    {
        return ( ErrCode::MSG_FORMAT_ERROR );
    }

    while( curByteIndex < optionLen )
    {
        // check for payload marker, if it is there then mark the start of payload and return
        if( inByteArray[curByteIndex] == 0xFF )
        {
            // at least one byte payload must follow to the payload marker
            if( ( optionLen - curByteIndex ) < 2 )
            {
                return ( ErrCode::MSG_FORMAT_ERROR );
            }
            else
            {
                curByteIndex++;                 // skip payload marker
                *payloadStart = &inByteArray[curByteIndex];
                return ( ErrCode::OK );
            }
        }
        else
        {
            u16 option_delta  = inByteArray[curByteIndex] >> 4;            // initial delta in upper 4 bits
            u16 option_length = inByteArray[curByteIndex] & 0x0F;             // initial length in lower 4 bits
            curByteIndex++;             // skip the initial delta & length byte

            if( option_delta == 13 )
            {
                // An 8-bit unsigned integer follows the initial byte and indicates the Option Delta minus 13.
                option_delta = inByteArray[curByteIndex] + 13;
                curByteIndex++;                 // skip 1 byte big delta
            }
            else if( option_delta == 14 )
            {
                // A 16-bit unsigned integer in network byte order follows the initial byte and indicates the Option Delta minus 269.
                option_delta = ( ( u16( inByteArray[curByteIndex] ) << 8 )
                    | u16( inByteArray[curByteIndex] ) ) + 269;
                curByteIndex += 2;
            }
            else if( option_delta == 15 )
            {
                // Reserved for the Payload Marker.
                return ( ErrCode::MSG_FORMAT_ERROR );
            }

            if( option_length == 13 )
            {
                // An 8-bit unsigned integer precedes the Option Value and indicates the Option Length minus 13.
                option_length = inByteArray[curByteIndex] + 13;
                curByteIndex++;                 // skip 1 byte big delta
            }
            else if( option_length == 14 )
            {
                // A 16-bit unsigned integer in network byte order precedes the Option Value and indicates the Option Length minus 269.
                option_length = ( ( u16( inByteArray[curByteIndex] ) << 8 )
                    | u16( inByteArray[curByteIndex] ) ) + 269;
                curByteIndex += 2;
            }
            else if( option_length == 15 )
            {
                // Reserved for future use.
                return ( ErrCode::MSG_FORMAT_ERROR );
            }

            // if current option length is greater than max option size or
            // current option length is greater than remaining payload
            if( ( option_length > JA_COAP_CONFIG_MAX_OPTION_SIZE )
              || ( ( optionLen - curByteIndex ) < option_length ) )
            {
                return ( ErrCode::MSG_FORMAT_ERROR );
            }

            lastOptionNum += option_delta;             // encode option delta
            curByteIndex  += option_length;
        }
    }

    return ( ErrCode::OK );
}
}
