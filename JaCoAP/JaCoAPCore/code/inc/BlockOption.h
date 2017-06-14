#pragma once

#include <ja_types.h>
#include <cstddef>
#include <cstdint>
#include "BaseObject.h"

namespace JaCoAP
{
class BlockOption
{
    u32    _data = 0;

public:
    BlockOption ();
    BlockOption( u8 szx, bool m, u32 num );

    u8   getSZX() const;
    void setSZX( u8 szx );

    u16  getSize() const;
    void setSize( u16 size );

    bool hasMore() const;
    void setMore( bool setValue );

    u32  getNum() const;
    void setNum( u32 num );

    u8   getLen() const;
    void setLen( u8 len );

    void decode( uint8_t *buf, uint8_t bufLen );
    void encode( uint8_t *buf, uint8_t &bufLen ) const;

    static u8  sizeToSZX( u16 size );
    static u16 szxToSize( u8 szx );

    DEFINE_MEMORY_OPERATORS(BlockOption);
};
}
