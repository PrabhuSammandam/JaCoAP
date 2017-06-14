#include <Block2Info.h>
#include <MemMgr.h>
#include <string.h>
#include "JaCoapIF.h"

FILE_NAME("Block2Info.cpp");

namespace JaCoAP
{
Block2Info::~Block2Info ()
{
    if( _dataBuf != nullptr )
    {
        mdelete _dataBuf;
        _dataBuf = nullptr;
    }
}

void Block2Info::SetComplete( bool setValue )
{
    setValue ? SetBit( _data, BLOCK_TRANSFER_STATUS_BITMASK_COMPLETE ) : ClearBit( _data, BLOCK_TRANSFER_STATUS_BITMASK_COMPLETE );
}

void Block2Info::SetRandomAccess( bool setValue )
{
    setValue ? SetBit( _data, BLOCK_TRANSFER_STATUS_BITMASK_RANDOM_ACCESS ) : ClearBit( _data, BLOCK_TRANSFER_STATUS_BITMASK_RANDOM_ACCESS );
}

void Block2Info::SetDataBufSize( u16 dataBufSize )
{
    if( _dataBuf != nullptr )
    {
        mdelete _dataBuf;
    }

    _dataBufSize = dataBufSize;
    _dataBuf     = ( _dataBufSize > 0 ) ? mnew ByteArray{ _dataBufSize } : nullptr;
}

void Block2Info::SetSZX( u8 szx )
{
    _data &= ~( BLOCK_TRANSFER_STATUS_BITMASK_SZX );
    _data |= ( szx & BLOCK_TRANSFER_STATUS_BITMASK_SZX );
}

bool Block2Info::AddData( ByteArray &dataArray )
{
    auto result = false;

    if( _dataBuf && ( dataArray.GetLen() > 0 ) )
    {
        if( ( _dataBufSize - _dataBuf->GetLen() >= dataArray.GetLen() ) )
        {
            ::memcpy( &_dataBuf->GetArray()[_dataBuf->GetLen()], dataArray.GetArray(), dataArray.GetLen() );
            _dataBuf->SetLen( _dataBuf->GetLen() + dataArray.GetLen() );
            result = true;
        }
    }

    return ( result );
}

}
