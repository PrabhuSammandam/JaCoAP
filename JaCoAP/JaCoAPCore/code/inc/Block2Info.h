#pragma once

#include <ByteArray.h>
#include <ja_types.h>
#include <Option.h>
#include <cstddef>

namespace JaCoAP
{
class Msg;
}

namespace JaCoAP
{
constexpr auto BLOCK_TRANSFER_STATUS_BITMASK_SZX = 0x07;
constexpr auto BLOCK_TRANSFER_STATUS_BITMASK_RANDOM_ACCESS = 0x08;
constexpr auto BLOCK_TRANSFER_STATUS_BITMASK_COMPLETE = 0x10;

class Block2Info
{
Msg * _firstMsg = nullptr;
ByteArray * _dataBuf = nullptr;
u32 _curNum = 0;
u32 _obsSeqNum = -1;
u16 _contentFormat = u16(ContentFormatId::TEXT__PLAIN);
u16 _dataBufSize = 0;
u8 _data = 0;

public:
Block2Info()
{
}

Block2Info(u16 dataBufSize, u16 contentFormat) :
	_contentFormat { contentFormat }
{
	SetDataBufSize(dataBufSize);
}

Block2Info(u8 szx, u32 num, u16 contentFormat) :
	_curNum { num }, _contentFormat { contentFormat }
{
	SetSZX(szx);
}

~Block2Info();

bool IsComplete() const
{
	return IsBitSet(_data, BLOCK_TRANSFER_STATUS_BITMASK_COMPLETE);
}

void SetComplete(bool setValue);

bool IsRandomAccess() const
{
	return IsBitSet(_data, BLOCK_TRANSFER_STATUS_BITMASK_RANDOM_ACCESS);
}

void SetRandomAccess(bool setValue);

const u16& GetDataBufSize() const
{
	return _dataBufSize;
}

void SetDataBufSize(u16 dataBufSize);

u8 GetSZX() const
{
	return _data & 0x07;
}

void SetSZX(u8 szx);

u32 GetCurNum() const
{
	return _curNum;
}

void SetCurNum(u32 curNum)
{
	_curNum = curNum;
}

u32 GetObsSeqNum() const
{
	return _obsSeqNum;
}

void SetObsSeqNum(u32 obsSeqNum)
{
	_obsSeqNum = obsSeqNum;
}

u16 GetContentFormat() const
{
	return _contentFormat;
}

void SetContentFormat(u16 contentFormat)
{
	_contentFormat = contentFormat;
}

Msg * GetFirstMsg() const
{
	return _firstMsg;
}

void SetFirstMsg(Msg * firstMsg)
{
	_firstMsg = firstMsg;
}

bool AddData(ByteArray & dataArray);

ByteArray* GetData() const
{
	return _dataBuf;
}

void ClearData()
{
	_dataBuf = nullptr;
}

DEFINE_MEMORY_OPERATORS(Block2Info);
};
}
