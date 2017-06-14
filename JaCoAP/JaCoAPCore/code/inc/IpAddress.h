#pragma once

#include <ja_types.h>

namespace JaCoAP
{
class IpAddress
{
    u8 _addr[4] = {0};

public:
    IpAddress();
    explicit IpAddress(u8 *addr);
    explicit IpAddress(u32 addr);
    IpAddress(u8 byte1, u8 byte2, u8 byte3, u8 byte4);
    ~IpAddress();

    bool operator == (const IpAddress &other);

    u8 operator [] (u8 index) const;

    IpAddress & operator = (const u8 *address);
    IpAddress & operator = (u32 address);

    u8* AsBytes(){return ( &_addr[0]);}

    u32 AsU32();

    void Print();
};
}