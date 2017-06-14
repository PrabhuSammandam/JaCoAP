#ifndef __JA_TYPES_H__
#define __JA_TYPES_H__

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

typedef unsigned char* pu8;
typedef unsigned short* pu16;
typedef unsigned int* pu32;
typedef unsigned long long* pu64;

typedef char i8;
typedef short i16;
typedef int i32;

typedef char* pi8;
typedef short* pi16;
typedef int* pi32;

typedef void* pvoid;

template<typename T1, typename T2>
auto IsBitSet(T1&& VALUE,
                               T2&& BITMASK) -> decltype(((VALUE & BITMASK) == BITMASK))
{
	return (((VALUE & BITMASK) == BITMASK));
}

template<typename T1, typename T2>
auto SetBit(T1&& VALUE,
                             T2&& BITMASK) -> decltype((VALUE |= BITMASK))
{
	return ((VALUE |= BITMASK));
}

template<typename T1, typename T2>
auto ClearBit(T1&& VALUE,
                               T2&& BITMASK) -> decltype((VALUE &= (~BITMASK)))
{
	return ((VALUE &= (~BITMASK)));
}

#endif /*__JA_TYPES_H__*/
