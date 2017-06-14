#include <Msg.h>
#include <SimpleCounterMsgIdProvider.h>
#include <SimpleTokenProvider.h>
#include <Transaction.h>
#include <TransStore.h>
#include "JaCoapIF.h"

FILE_NAME( "TransStore.cpp" );

namespace JaCoAP
{
SimpleCounterMsgIdProvider _simpleCounterMsgIdProvider{};
SimpleTokenProvider _simpleTokenProvider{};
TransStore::TransStore () : _transHashList()
{
    SetMsgIdProv( &_simpleCounterMsgIdProvider );
    SetTokenProv( &_simpleTokenProvider );
}

u16 TransStore::AssignMsgId( Msg *msg ) const
{
    if( !msg->HasMsgId() )
    {
        msg->Id( _msgIdProvider->GetNextMsgId() );
    }

    return ( msg->Id() );
}

bool TransStore::RegisterRequest( Transaction *trans )
{
    if( trans == nullptr )
    {
        dbg_error( "Trans NULL" );
        return ( false );
    }

    if( trans->CurReq() == nullptr )
    {
        dbg_error( "Request NULL" );
        return ( false );
    }

    u16 mid = RegisterByMid( trans, trans->CurReq() );

    if( mid == 0xFFFF )
    {
        return ( false );
    }

    RegisterByToken( trans );

    return ( true );
}

bool TransStore::RegisterRequestByToken( Transaction *trans )
{
    if( trans == nullptr )
    {
        dbg_error( "Trans NULL" );
        return ( false );
    }

    if( trans->CurReq() == nullptr )
    {
        dbg_error( "Request NULL" );
        return ( false );
    }

    RegisterByToken( trans );

    return ( true );
}

bool TransStore::RegisterResponse( Transaction *trans )
{
    if( trans == nullptr )
    {
        dbg_error( "Trans NULL" );
        return ( false );
    }

    if( trans->CurRes() == nullptr )
    {
        dbg_error( "Response NULL" );
        return ( false );
    }

    return ( RegisterByMid( trans, trans->CurRes() ) != -1 );
}

bool TransStore::RegisterByURI( u32 uriKey, Transaction *trans )
{
    if( _transHashList.PutIfAbsent( uriKey, HashKeyType::URI, trans ) != nullptr )
    {
        return ( true );
    }

    return ( false );
}

void TransStore::CompleteTrans( Transaction *trans )
{
    if( trans->IsOriginLocal() )
    {
        auto curReq = trans->CurReq();

        if( curReq != nullptr )
        {
            auto keyMid = curReq->GetHashByDstEpAndMid();
            auto keyTkn = curReq->GetHashByDstEpAndTkn();

            this->RemoveByTokenKey( keyTkn );
            this->RemoveByMidKey( keyMid );           // in case an empty ACK was lost
        }
    }
    else
    {
        auto curRes = trans->CurRes();

        if( ( curRes != nullptr ) && !curRes->IsAck() )
        {
            // this means that we have sent the response in a separate CON/NON message
            // (not piggy-backed in ACK). The response therefore has a different MID
            // than the original request

            // first remove the entry for the (separate) response's MID
            auto keyMid = curRes->GetHashByDstEpAndMid();
            this->RemoveByMidKey( keyMid );
        }
    }
}

Transaction * TransStore::RemoveByTokenKey( u32 tokenKey )
{
    return ( _transHashList.Remove( tokenKey, HashKeyType::TOKEN ) );
}

Transaction * TransStore::RemoveByMidKey( u32 midKey )
{
    return ( _transHashList.Remove( midKey, HashKeyType::MID ) );
}

Transaction * TransStore::RemoveByUriKey( u32 uriKey )
{
    return ( _transHashList.Remove( uriKey, HashKeyType::URI ) );
}

Transaction * TransStore::GetByTokenKey( u32 tokenKey ) const
{
    return ( _transHashList.FindEntry( tokenKey, HashKeyType::TOKEN ) );
}

Transaction * TransStore::GetByMidKey( u32 midKey ) const
{
    return ( _transHashList.FindEntry( midKey, HashKeyType::MID ) );
}

Transaction * TransStore::GetByUriKey( u32 uriKey ) const
{
    return ( _transHashList.FindEntry( uriKey, HashKeyType::URI ) );
}

Transaction * TransStore::FindOrAdd( u32 midKey, Transaction *trans )
{
    return ( _deduplicator.FindOrAdd( midKey, trans ) );
}

u16 TransStore::RegisterByMid( Transaction *trans, Msg *msg )
{
    u16 msgId  = msg->Id();
    u32 keyMid = msg->GetHashByDstEpAndMid();

    if( !msg->HasMsgId() )
    {
        msgId = AssignMsgId( msg );
    }

    auto prevTrans = _transHashList.PutIfAbsent( keyMid, HashKeyType::MID, trans );

    if( prevTrans != nullptr )
    {
    	/* check if trans is there with same MID*/
        if( prevTrans != trans )
        {
            dbg_error( "MsgId[%u] already in use, cannot register trans", msgId );
            return ( -1 );
        }
        else if( trans->RetransCnt() == 0 )
        {
            dbg_error( "message with already registered ID [%d] is not a re-transmission, cannot register exchange", msgId );
            return ( -1 );
        }
    }

    return ( msgId );
}

void TransStore::RegisterByToken( Transaction *trans )
{
}

TransStoreHashList::TransStoreHashList ()
{
}

Transaction * TransStoreHashList::PutIfAbsent( u32 hashKey, HashKeyType type, Transaction *trans )
{
    Transaction *tempTrans = FindEntry( hashKey, type );

    if( tempTrans != nullptr )
    {
        return ( tempTrans );
    }

    CreateEntry( hashKey, type, trans );

    return ( nullptr );
}

u16 TransStoreHashList::CreateEntry( u32 hashKey, HashKeyType type, Transaction *trans )
{
    for( auto i = 0; i < JA_COAP_CONFIG_TRANS_HASH_LIST_MAX_SIZE; i++ )
    {
        if( _list[i].used == false )
        {
            _list[i].trans     = trans;
            _list[i].type      = u8( type );
            _list[i].hashValue = hashKey;
            _list[i].used      = true;
            _count++;

            return ( i );
        }
    }

    return ( -1 );
}

Transaction * TransStoreHashList::FindEntry( u32 hashKey, HashKeyType type ) const
{
    for( auto i = 0; i < JA_COAP_CONFIG_TRANS_HASH_LIST_MAX_SIZE; i++ )
    {
        if( ( _list[i].hashValue == hashKey ) && ( _list[i].type == u8( type ) ) )
        {
            return ( _list[i].trans );
        }
    }

    return ( nullptr );
}

Transaction * TransStoreHashList::Remove( u32 hashKey, HashKeyType type )
{
    Transaction *tempTrans = nullptr;

    for( auto i = 0; i < JA_COAP_CONFIG_TRANS_HASH_LIST_MAX_SIZE; i++ )
    {
        if( ( _list[i].hashValue == hashKey ) && ( _list[i].type == u8( type ) ) )
        {
            tempTrans          = _list[i].trans;
            _list[i].used      = false;
            _list[i].trans     = nullptr;
            _list[i].hashValue = 0;
            _list[i].type      = u8( HashKeyType::NONE );
            _count--;
            break;
        }
    }

    return ( tempTrans );
}
}
