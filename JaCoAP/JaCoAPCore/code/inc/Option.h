#pragma once

#include <cstddef>
#include "ja_types.h"
#include <cstdint>

namespace JaCoAP
{
#define OPTION_ERROR_CRITICAL_MASK 0x0100
#define OPTION_ERROR_CRITICAL_UNKNOWN 0x0101
#define OPTION_ERROR_CRITICAL_REPEAT_MORE 0x0102
#define OPTION_ERROR_CRITICAL_LEN_RANGE_OUT 0x0104
#define OPTION_ERROR_UNKNOWN 0x0001
#define OPTION_ERROR_REPEAT_MORE 0x0002
#define OPTION_ERROR_LEN_RANGE_OUT 0x0004

enum class ContentFormatId
{
    NONE              = 0xFFFF,
    TEXT__PLAIN       = 0,
    APP__LINK_FORMAT  = 40,
    APP__XML          = 41,
    APP__OCTET_STREAM = 42,
    APP__EXI          = 47,
    APP__JSON         = 50
};

enum class OptionType
{
    IF_MATCH       = 1,
    URI_HOST       = 3,
    ETAG           = 4,
    IF_NONE_MATCH  = 5,
    OBSERVE        = 6,
    URI_PORT       = 7,
    LOCATION_PATH  = 8,
    URI_PATH       = 11,
    CONTENT_FORMAT = 12,
    MAX_AGE        = 14,
    URI_QUERY      = 15,
    ACCEPT         = 17,
    LOCATION_QUERY = 20,
    BLOCK_2        = 23,
    BLOCK_1        = 27,
    SIZE2          = 28,
    PROXY_URI      = 35,
    PROXU_SCHEME   = 39,
    SIZE1          = 60
};

class Option
{
    u16       _no    = 0;
    u16       _len   = 0;
    pu8       _val   = nullptr;
    Option    *_next = nullptr;

    void * operator new ( size_t len, u16 buf_len );
    void operator   delete ( void *mem, u16 buf_len );

public:
    Option () {}

    ~Option ();

    Option( u16 no, u16 len, pu8 val );
    Option( Option const &opt );

    u16 GetNo() const { return ( _no ); }

    u16 GetLen() const { return ( _len ); }

    pu8 GetVal() const { return ( _val ); }

    void SetNo( u16 no ) { _no = no; }

    void SetLen( u16 len ) { _len = len; }

    void SetVal( pu8 val ) { _val = val; }

    Option* GetNext() const { return ( _next ); }

    void SetNext( Option *opt ) { _next = opt; }

    void Print() const;

public:
    static void  Destroy( Option *opt );
    static Option* Allocate( u16 no, u16 len, pu8 val );
    static Option* Allocate();
};

class OptionListIterator
{
public:
    Option    *_node;
    OptionListIterator () { _node = nullptr; }
    explicit OptionListIterator( Option *node ) :
        _node( node )
    {
    }

    ~OptionListIterator () { _node = nullptr; }

    bool operator != ( const OptionListIterator &other ) const { return ( _node != other._node ); }

    bool operator == ( const OptionListIterator &other ) const { return ( _node == other._node ); }

    void operator ++ () { _node = _node->GetNext(); }

    void operator ++ ( int ) { _node = _node->GetNext(); }

    Option * operator * () const { return ( _node ); }
};

class OptionList
{
    Option    *_first = nullptr;
    Option    *_last;

public:
    OptionList ();
    ~OptionList ();

    bool IsEmpty() const { return ( _first == nullptr && _last == nullptr ); }

    void InsertFront( Option *option );
    void InsertEnd( Option *option );
    void Insert( Option *option );
    void FreeList();
    void Print() const;
    u16  Size() const;

    /*iterator*/
    OptionListIterator begin() const { return ( OptionListIterator( _first ) ); }

    OptionListIterator end() const { return ( OptionListIterator( nullptr ) ); }
};
}