#include "Deduplicator.h"

namespace JaCoAP
{
DeduplicatorHashList::DeduplicatorHashList () :
    _list()
{
    Reset();
    _count = 0;
}

void DeduplicatorHashList::Reset()
{
    for( auto i = 0; i < _size; i++ )
    {
        _list[i].used    = false;
        _list[i].trans   = nullptr;
        _list[i].hashKey = 0;
    }
}

Transaction * DeduplicatorHashList::PutIfAbsent( u32 hashKey, Transaction *trans )
{
    Transaction *tempTrans = Find( hashKey );

    if( tempTrans != nullptr )
    {
        return ( tempTrans );
    }

    CreateEntry( hashKey, trans );

    return ( nullptr );
}

u16 DeduplicatorHashList::CreateEntry( u32 hashKey, Transaction *trans )
{
    for( auto i = 0; i < _size; i++ )
    {
        if( _list[i].used == false )
        {
            _list[i].trans   = trans;
            _list[i].hashKey = hashKey;
            _list[i].used    = true;
            _count++;

            return ( i );
        }
    }

    return ( -1 );
}

Transaction * DeduplicatorHashList::Find( u32 hashKey ) const
{
    for( auto i = 0; i < _size; i++ )
    {
        if( _list[i].hashKey == hashKey )
        {
            return ( _list[i].trans );
        }
    }

    return ( nullptr );
}

Transaction * DeduplicatorHashList::Remove( u32 hashKey )
{
    Transaction *tempTrans = nullptr;

    for( auto i = 0; i < _size; i++ )
    {
        if( _list[i].hashKey == hashKey )
        {
            tempTrans        = _list[i].trans;
            _list[i].used    = false;
            _list[i].trans   = nullptr;
            _list[i].hashKey = 0;
            _count--;
        }
    }

    return ( tempTrans );
}

Deduplicator::Deduplicator () :
    _hashList()
{
}

bool Deduplicator::IsEmpty() const
{
    return ( true );  // _hashList.IsEmpty( );
}

Transaction * Deduplicator::Find( u32 keyMid ) const
{
    return ( nullptr );  // _hashList.Find( keyMid );
}

Transaction * Deduplicator::FindOrAdd( u32 keyMid, Transaction *trans )
{
    return ( nullptr );  // _hashList.PutIfAbsent( keyMid, trans );
}
}