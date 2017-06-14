#pragma once

#include <ja_types.h>
#include <cstddef>
#include <cstdint>
#include <memory>
#include "BaseObject.h"

namespace JaCoAP
{
using ByteArrayBufPtr = std::unique_ptr<uint8_t, void ( *)(uint8_t *)>;

class ByteArray
{
    uint8_t  *_byteArrayPtr = nullptr;
    uint16_t _length        = 0;
    uint16_t _size          = 0;
    bool     _freeBuffer    = true;

public:
    ByteArray();
    ByteArray(uint16_t size);
    ByteArray(uint8_t *buf, uint16_t bufLen, bool freeBuf = false);
    ~ByteArray();

    ByteArray(ByteArray &&other) noexcept;
    ByteArray & operator = (ByteArray &&other) noexcept;

    uint8_t* GetArray() const
    {
        return (_byteArrayPtr);
    }

    void SetArray(uint8_t *buf, uint16_t bufLen, bool freeBuf = true);

    void Assign(uint8_t *buf, uint16_t bufLen);

    uint16_t GetLen() const;
    ByteArray* SetLen(uint16_t len);

    void CopyStr(u8 *str);
    void CopyNBytes(pu8 srcBuf, uint16_t len);
    void CopyTo(ByteArray *dst, uint16_t from, uint16_t len) const;

    uint32_t GetHashValue();

    bool IsEmpty() const;

    bool operator == (const ByteArray &other) const;
    u8 operator   [] (uint16_t index) const;

    DEFINE_MEMORY_OPERATORS(ByteArray);
};
}
