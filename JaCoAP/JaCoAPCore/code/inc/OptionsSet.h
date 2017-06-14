#pragma once

#include "Option.h"
#include "ErrorCodes.h"
#include "ByteArray.h"
#include "BlockOption.h"
#include "OptionsSet.h"

namespace JaCoAP
{
const uint32_t OptionBitmaskIfMatch     = 0x00000001;
const uint32_t OptionBitmaskUriHost     = 0x00000002;
const uint32_t OptionBitmaskEtag        = 0x00000004;
const uint32_t OptionBitmaskIfNoneMatch = 0x00000008;

const uint32_t OptionBitmaskUriPort       = 0x00000010;
const uint32_t OptionBitmaskLocationPath  = 0x00000020;
const uint32_t OptionBitmaskUriPath       = 0x00000040;
const uint32_t OptionBitmaskContentFormat = 0x00000080;

const uint32_t OptionBitmaskMaxAge        = 0x00000100;
const uint32_t OptionBitmaskUriQuery      = 0x00000200;
const uint32_t OptionBitmaskAccept        = 0x00000400;
const uint32_t OptionBitmaskLocationQuery = 0x00000800;

const uint32_t OptionBitmaskProxyUri    = 0x00001000;
const uint32_t OptionBitmaskProxyScheme = 0x00002000;

const uint32_t OptionBitmaskSize1 = 0x00004000;
const uint32_t OptionBitmaskSize2 = 0x00008000;

const uint32_t OptionBitmaskBlock1  = 0x00010000;
const uint32_t OptionBitmaskBlock2  = 0x00020000;
const uint32_t OptionBitmaskObserve = 0x00040000;

class OptionsSet
{
    OptionList _optionList;
    uint32_t   _status = 0;

public:
    OptionsSet();
    ~OptionsSet();

    template<typename T>

    ErrCode AddOption(u16 optionNo, T val)
    {
        uint8_t tempByte[sizeof(T)] {};

        for( u8 i = 0; i < sizeof(T); i++ )
        {
            tempByte[i] = (val >> ( ( (sizeof(T) - 1) - i ) * 8 ) ) & 0xFF;
        }

        return ( AddOption(optionNo,
               sizeof(u32),
               &tempByte[0]) );
    }

    ErrCode AddOption(u16 optionNo, u16 val);
    ErrCode AddOption(u16 optionNo, u32 val);
    ErrCode AddOption(u16 optionNo, u64 val);
    ErrCode AddOption(u16 optionNo, u16 val_len, pu8 value);

    u32   Get4ByteOption(OptionType optionNo) const;
    Option* GetOption(OptionType optionNo) const;

    bool HasSize1() const
    {
        return ( IsBitSet(_status,
               OptionBitmaskSize1) );
    }

    bool HasSize2() const
    {
        return ( IsBitSet(_status,
               OptionBitmaskSize2) );
    }

    bool HasBlock1() const
    {
        return ( IsBitSet(_status,
               OptionBitmaskBlock1) );
    }

    bool HasBlock2() const
    {
        return ( IsBitSet(_status,
               OptionBitmaskBlock2) );
    }

    bool HasObserve() const
    {
        return ( IsBitSet(_status,
               OptionBitmaskObserve) );
    }

    bool HasContentFormat() const {return ( IsBitSet(_status, OptionBitmaskContentFormat) );}

    u32 GetSize1() const;
    u32 GetSize2() const;

    void GetBlock1(BlockOption &blockOpt) const;
    void GetBlock2(BlockOption &blockOpt) const;

    void SetBlock1(const BlockOption &block1);
    void SetBlock2(const BlockOption &block2);
    void SetSize1(u32 size1);
    void SetSize2(u32 size2);

    u16 GetContentFormat() const;

    bool HasOpt(u16 optionNo) const;

    u16 GetTotalSizeInBytes() const;

    ErrCode FreeOptions();

    u16 GetNoOfOptions() const {return ( _optionList.Size() );}

    u16 CheckOptions();

    OptionList* OptList(){return ( &_optionList);}

    void GetUriPathString(ByteArray &uriPath) const;

    void Copy(OptionsSet &src);

    void Print() const;
    void Init();
    void Release();
};
}