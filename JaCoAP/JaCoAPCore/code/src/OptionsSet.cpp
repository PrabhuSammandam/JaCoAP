#include <string.h>
#include "OptionsSet.h"

namespace JaCoAP
{
static u16       nonRepeatableOption[] = { u16( OptionType::URI_HOST ),
                                           u16( OptionType::IF_NONE_MATCH ),
                                           u16( OptionType::OBSERVE ),
                                           u16( OptionType::URI_PORT ),
                                           u16( OptionType::CONTENT_FORMAT ),
                                           u16( OptionType::MAX_AGE ),
                                           u16( OptionType::ACCEPT ),
                                           u16( OptionType::PROXY_URI ),
                                           u16( OptionType::PROXU_SCHEME ),
                                           u16( OptionType::SIZE1 ),
                                           u16( OptionType::SIZE2 ),
                                           u16( OptionType::BLOCK_1 ),
                                           u16( OptionType::BLOCK_2 ) };

static ByteArray _8byteEncodeArray( 8 );
static ByteArray _8byteDecodeArray{};

ErrCode OptionsSet::AddOption( u16 optionNo, u16 val )
{
    u8 temp_byte[2] {};

    temp_byte[0] = ( val >> 8 ) & 0xFF;
    temp_byte[1] = val & 0xFF;

    return ( AddOption( optionNo, sizeof( u16 ), &temp_byte[0] ) );
}

ErrCode OptionsSet::AddOption( u16 optionNo, u32 val )
{
    u8 temp_byte[4] {};

    for( u8 i = 0; i < 4; i++ )
    {
        temp_byte[i] = ( val >> ( ( 3 - i ) * 8 ) ) & 0xFF;
    }

    return ( AddOption( optionNo, sizeof( u32 ), &temp_byte[0] ) );
}

ErrCode OptionsSet::AddOption( u16 optionNo, u64 val )
{
    u8 temp_byte[8] {};

    for( u8 i = 0; i < 8; i++ )
    {
        temp_byte[i] = ( val >> ( ( 7 - i ) * 8 ) ) & 0xFF;
    }

    return ( AddOption( optionNo, sizeof( u64 ), &temp_byte[0] ) );
}

ErrCode OptionsSet::AddOption( u16 optionNo, u16 val_len, pu8 value )
{
    auto opt = Option::Allocate( optionNo, val_len, value );

    if( opt != nullptr )
    {
        _optionList.Insert( opt );

        return ( ErrCode::OK );
    }

    return ( ErrCode::OUT_OF_MEMORY );
}

u32 OptionsSet::Get4ByteOption( OptionType optionNo ) const
{
    u32  value = 0;
    auto opt   = GetOption( optionNo );

    if( opt != nullptr )
    {
        for( auto i = 0; i < opt->GetLen(); i++ )
        {
            value |= ( opt->GetVal()[i] & 0xFF ) << ( i * 8 );
        }
    }

    return ( value );
}

Option * OptionsSet::GetOption( OptionType optionNo ) const
{
    for( auto it = _optionList.begin(); it != _optionList.end(); ++it )
    {
        auto opt = *it;

        if( opt->GetNo() == u16( optionNo ) )
        {
            return ( opt );
        }
    }

    return ( nullptr );
}

u32 OptionsSet::GetSize1() const
{
    return ( Get4ByteOption( OptionType::SIZE1 ) );
}

u32 OptionsSet::GetSize2() const
{
    return ( Get4ByteOption( OptionType::SIZE2 ) );
}

void OptionsSet::GetBlock1( BlockOption &blockOpt ) const
{
    auto opt = GetOption( OptionType::BLOCK_1 );

    blockOpt.decode( opt->GetVal(), opt->GetLen() );
}

void OptionsSet::GetBlock2( BlockOption &blockOpt ) const
{
    auto opt = GetOption( OptionType::BLOCK_2 );

    blockOpt.decode( opt->GetVal(), opt->GetLen() );
}

void OptionsSet::SetBlock1( const BlockOption &block1 )
{
    uint8_t len = 0;

    block1.encode( _8byteEncodeArray.GetArray(), len );
    _8byteEncodeArray.SetLen( len );
    AddOption( u16( OptionType::BLOCK_1 ), _8byteEncodeArray.GetLen(), _8byteEncodeArray.GetArray() );
    SetBit( _status, OptionBitmaskBlock1 );
}

void OptionsSet::SetBlock2( const BlockOption &block2 )
{
    uint8_t len = 0;

    block2.encode( _8byteEncodeArray.GetArray(), len );
    _8byteEncodeArray.SetLen( len );
    AddOption( u16( OptionType::BLOCK_2 ), _8byteEncodeArray.GetLen(), _8byteEncodeArray.GetArray() );
    SetBit( _status, OptionBitmaskBlock2 );
}

void OptionsSet::SetSize1( u32 size1 )
{
    AddOption( u16( OptionType::SIZE1 ), size1 );
    SetBit( _status, OptionBitmaskSize1 );
}

void OptionsSet::SetSize2( u32 size2 )
{
    AddOption( u16( OptionType::SIZE2 ), size2 );
    SetBit( _status, OptionBitmaskSize2 );
}

u16 OptionsSet::GetContentFormat() const
{
    if( HasContentFormat() )
    {
        return ( static_cast<u16>( Get4ByteOption( OptionType::CONTENT_FORMAT ) ) );
    }

    return ( 0xFFFF );
}

ErrCode OptionsSet::FreeOptions()
{
    _optionList.FreeList();

    return ( ErrCode::OK );
}

u16 OptionsSet::GetTotalSizeInBytes() const
{
    return ( 0 );
}

bool OptionsSet::HasOpt( u16 optionNo ) const
{
    if( _optionList.IsEmpty() )
    {
        return ( false );
    }

    auto it = _optionList.begin();

    for(; it != _optionList.end(); ++it )
    {
        if( ( *it )->GetNo() == optionNo )
        {
            return ( true );
        }
    }

    return ( false );
}

static u8 FindOptionInRepeatArray( u16 optNo )
{
    for( uint16_t i = 0; i < sizeof( nonRepeatableOption ) / sizeof( u16 ); i++ )
    {
        if( optNo == nonRepeatableOption[i] )
        {
            return ( i );
        }
    }

    return ( 0xFF );
}

static void AddOptionToRepeatArray( u16 optNo )
{
    for( uint16_t i = 0; i < sizeof( nonRepeatableOption ) / sizeof( u16 ); i++ )
    {
        if( 0 == nonRepeatableOption[i] )
        {
            nonRepeatableOption[i] = optNo;
            break;
        }
    }
}

static u16 CheckOptionForRepeat( u16 optNo )
{
    if( FindOptionInRepeatArray( optNo ) == 0xFF )
    {
        AddOptionToRepeatArray( optNo );
    }
    else
    {
        if( ( optNo & 0x01 ) == 0x01 )
        {
            return ( OPTION_ERROR_CRITICAL_REPEAT_MORE );
        }

        return ( OPTION_ERROR_REPEAT_MORE );
    }

    return ( 0 );
}

static u16 CheckOptionForLen( Option *option, u16 min, u16 max )
{
    u16 optLen = option->GetLen();

    if( ( optLen < min ) || ( optLen > max ) )
    {
        if( ( option->GetNo() & 0x01 ) == 0x01 )
        {
            return ( OPTION_ERROR_CRITICAL_LEN_RANGE_OUT );
        }

        return ( OPTION_ERROR_LEN_RANGE_OUT );
    }

    return ( 0 );
}

u16 OptionsSet::CheckOptions()
{
    u8 nonRepOptCount = sizeof( nonRepeatableOption ) / sizeof( u16 );

    for( auto i = 0; i < nonRepOptCount; i++ )
    {
        nonRepeatableOption[i] = 0;
    }

    u16 errMask = 0;

    for( auto it = _optionList.begin(); ( it != _optionList.end() ); ++it )
    {
        auto currOption   = *it;
        auto currOptionNo = currOption->GetNo();

        switch( OptionType( currOptionNo ) )
        {
        case OptionType::IF_MATCH:
        {
            SetBit( _status, OptionBitmaskIfMatch );
            errMask |= CheckOptionForLen( currOption, 0, 8 );
        }
        break;

        case OptionType::URI_HOST:
        {
            SetBit( _status, OptionBitmaskUriHost );
            errMask |= CheckOptionForLen( currOption, 1, 255 );
            errMask |= CheckOptionForRepeat( currOptionNo );
        }
        break;

        case OptionType::ETAG:
        {
            SetBit( _status, OptionBitmaskEtag );
            errMask |= CheckOptionForLen( currOption, 1, 8 );
        }
        break;
        case OptionType::IF_NONE_MATCH:
        {
            SetBit( _status, OptionBitmaskIfNoneMatch );
            errMask |= CheckOptionForLen( currOption, 0, 0 );
            errMask |= CheckOptionForRepeat( currOptionNo );
        }
        break;
        case OptionType::OBSERVE:
        {
            SetBit( _status, OptionBitmaskObserve );
            errMask |= CheckOptionForLen( currOption, 0, 3 );
            errMask |= CheckOptionForRepeat( currOptionNo );
        }
        break;
        case OptionType::URI_PORT:
        {
            SetBit( _status, OptionBitmaskUriPort );
            errMask |= CheckOptionForLen( currOption, 0, 2 );
            errMask |= CheckOptionForRepeat( currOptionNo );
        }
        break;
        case OptionType::LOCATION_PATH:
        {
            SetBit( _status, OptionBitmaskLocationPath );
            errMask |= CheckOptionForLen( currOption, 0, 255 );
        }
        break;
        case OptionType::URI_PATH:
        {
            SetBit( _status, OptionBitmaskUriPath );
            errMask |= CheckOptionForLen( currOption, 0, 255 );
        }
        break;
        case OptionType::CONTENT_FORMAT:
        {
            SetBit( _status, OptionBitmaskContentFormat );
            errMask |= CheckOptionForLen( currOption, 0, 2 );
            errMask |= CheckOptionForRepeat( currOptionNo );
        }
        break;
        case OptionType::MAX_AGE:
        {
            SetBit( _status, OptionBitmaskMaxAge );
            errMask |= CheckOptionForLen( currOption, 0, 4 );
            errMask |= CheckOptionForRepeat( currOptionNo );
        }
        break;
        case OptionType::URI_QUERY:
        {
            SetBit( _status, OptionBitmaskUriQuery );
            errMask |= CheckOptionForLen( currOption, 0, 255 );
        }
        break;
        case OptionType::ACCEPT:
        {
            SetBit( _status, OptionBitmaskAccept );
            errMask |= CheckOptionForLen( currOption, 0, 2 );
            errMask |= CheckOptionForRepeat( currOptionNo );
        }
        break;
        case OptionType::LOCATION_QUERY:
        {
            SetBit( _status, OptionBitmaskLocationQuery );
            errMask |= CheckOptionForLen( currOption, 0, 255 );
        }
        break;
        case OptionType::PROXY_URI:
        {
            SetBit( _status, OptionBitmaskProxyUri );
            errMask |= CheckOptionForLen( currOption, 1, 1034 );
            errMask |= CheckOptionForRepeat( currOptionNo );
        }
        break;
        case OptionType::PROXU_SCHEME:
        {
            SetBit( _status, OptionBitmaskProxyScheme );
            errMask |= CheckOptionForLen( currOption, 1, 255 );
            errMask |= CheckOptionForRepeat( currOptionNo );
        }
        break;
        case OptionType::SIZE1:
        {
            SetBit( _status, OptionBitmaskSize1 );
            errMask |= CheckOptionForLen( currOption, 0, 4 );
            errMask |= CheckOptionForRepeat( currOptionNo );
        }
        break;

        case OptionType::SIZE2:
        {
            SetBit( _status, OptionBitmaskSize2 );
            errMask |= CheckOptionForLen( currOption, 0, 4 );
            errMask |= CheckOptionForRepeat( currOptionNo );
        }
        break;
        case OptionType::BLOCK_1:
        {
            SetBit( _status, OptionBitmaskBlock1 );
            errMask |= CheckOptionForLen( currOption, 0, 3 );
            errMask |= CheckOptionForRepeat( currOptionNo );
        }
        break;
        case OptionType::BLOCK_2:
        {
            SetBit( _status, OptionBitmaskBlock2 );
            errMask |= CheckOptionForLen( currOption, 0, 3 );
            errMask |= CheckOptionForRepeat( currOptionNo );
        }
        break;

        default:
        {
            if( ( currOption->GetNo() & 0x01 ) == 0x01 )
            {
                errMask |= OPTION_ERROR_CRITICAL_UNKNOWN;
            }
            else
            {
                errMask |= OPTION_ERROR_UNKNOWN;
            }
        }
        break;
        }
    }

    return ( errMask );
}

void OptionsSet::GetUriPathString( ByteArray &fullUri ) const
{
    u16  currBytesIdx = 0;
    auto byteArray    = fullUri.GetArray();

    if( _optionList.Size() == 0 )
    {
        byteArray[0] = '/';
        fullUri.SetLen( 1 );
        return;
    }

    for( auto it = _optionList.begin(); it != _optionList.end(); ++it )
    {
        auto opt = *it;

        if( ( opt->GetNo() == u16( OptionType::URI_PATH ) ) && ( opt->GetLen() > 0 ) )
        {
            byteArray[currBytesIdx++] = '/';
            memcpy( &byteArray[currBytesIdx], opt->GetVal(), opt->GetLen() );

            currBytesIdx += opt->GetLen();
        }
    }

    byteArray[currBytesIdx] = '\0';

    fullUri.SetLen( currBytesIdx );
}

void OptionsSet::Copy( OptionsSet &src )
{
    if( src.GetNoOfOptions() == 0 )
    {
        return;
    }

    for( auto it = _optionList.begin(); it != _optionList.end(); ++it )
    {
        auto opt = *it;
        AddOption( opt->GetNo(), opt->GetLen(), opt->GetVal() );
    }

    this->_status = src._status;
}

void OptionsSet::Print() const
{
    _optionList.Print();
}

void OptionsSet::Init()
{
    _status = 0;
}

void OptionsSet::Release()
{
    _optionList.FreeList();
    Init();
}

OptionsSet::OptionsSet () :
    _optionList()
{
}

OptionsSet::~OptionsSet ()
{
    Release();
}
}