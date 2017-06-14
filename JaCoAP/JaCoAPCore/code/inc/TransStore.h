#pragma once

#include <Deduplicator.h>
#include <ja_types.h>
#include <JaCoapConfig.h>

namespace JaCoAP
{
class MsgIdProvider;
class TokenProvider;
}

namespace JaCoAP
{
enum class HashKeyType
{
    NONE  = 0,
    MID   = 1,
    TOKEN = 2,
    URI   = 3
};

struct TransStoreEntry
{
    Transaction    *trans;
    u32            hashValue;
    u8             used;
    u8             type;
    TransStoreEntry () : trans{ nullptr }, hashValue{ 0 }, used{ false }, type{ u8( HashKeyType::NONE ) }
    {
    }
};

class TransStoreHashList
{
    TransStoreEntry    _list[JA_COAP_CONFIG_TRANS_HASH_LIST_MAX_SIZE];
    u8                 _count = 0;

public:
    TransStoreHashList ();

    u16        CreateEntry( u32 hashKey, HashKeyType type, Transaction *trans );
    Transaction* Remove( u32 hashKey, HashKeyType type );
    Transaction* FindEntry( u32 hashKey, HashKeyType type ) const;
    Transaction* PutIfAbsent( u32 hashKey, HashKeyType type, Transaction *trans );

    u8 Count() const { return ( _count ); }

    bool IsEmpty() const { return ( _count == 0 ); }
};

class TransStore
{
    TransStoreHashList    _transHashList;
    MsgIdProvider         *_msgIdProvider = nullptr;
    TokenProvider         *_tokenProvider = nullptr;
    Deduplicator          _deduplicator;

public:
    TransStore ();

    bool RegisterRequest( Transaction *trans );
    bool RegisterRequestByToken( Transaction *trans );
    bool RegisterResponse( Transaction *trans );
    bool RegisterByURI( u32 uriKey, Transaction *trans );

    Transaction* RemoveByTokenKey( u32 tokenKey );
    Transaction* RemoveByMidKey( u32 midKey );
    Transaction* RemoveByUriKey( u32 uriKey );

    Transaction* GetByTokenKey( u32 tokenKey ) const;
    Transaction* GetByMidKey( u32 midKey ) const;
    Transaction* GetByUriKey( u32 uriKey ) const;

    void CompleteTrans( Transaction *trans );
    u16  AssignMsgId( Msg *msg ) const;

    Transaction* FindOrAdd( u32 midKey, Transaction *trans );

    Transaction* Find( u32 midKey ) const { return ( _deduplicator.Find( midKey ) ); }

    bool IsEmpty() const { return ( _transHashList.IsEmpty() && _deduplicator.IsEmpty() ); }

    MsgIdProvider* GetMsgIdProv() const { return ( _msgIdProvider ); }

    void SetMsgIdProv( MsgIdProvider *msgIdProvider ) { _msgIdProvider = msgIdProvider; }

    TokenProvider* GetTokenProv() const { return ( _tokenProvider ); }

    void SetTokenProv( TokenProvider *tokenProvider ) { _tokenProvider = tokenProvider; }

private:
    u16  RegisterByMid( Transaction *trans, Msg *msg );
    void RegisterByToken( Transaction *trans );
};
}