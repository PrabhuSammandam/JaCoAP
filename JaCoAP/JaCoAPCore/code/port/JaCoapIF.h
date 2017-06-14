#pragma once

#include <cstdlib>
#include <utility>

namespace JaCoAP
{
#define ENABLE_MEM_DEBUG

#define FILE_NAME( _x_ ) static const char *const _file_name = _x_;

#define dbg_info( format, ... ) printf( "%s=>%s=>%u=>" format "\n", _file_name, __FUNCTION__, __LINE__, ## __VA_ARGS__ )
#define dbg_warn( format, ... ) printf( "%s=>%s=>%u=>" format "\n", _file_name, __FUNCTION__, __LINE__, ## __VA_ARGS__ )
#define dbg_error( format, ... ) printf( "%s=>%s=>%u=>" format "\n", _file_name, __FUNCTION__, __LINE__, ## __VA_ARGS__ )

class MemStamp
{
public:
    char const *const    _fileName;
    int const            _lineNum;

public:
    MemStamp( char const *filename, int lineNo ) : _fileName( filename ), _lineNum( lineNo ) {}
    ~MemStamp () {}
};

class JaCoapIF
{
public:
    static void * MemAlloc( size_t size );
    static void MemFree( void *mem );

    static void * MemAlloc( u8 *fileName, u32 lineNo, size_t size );
    static void MemFree( u8 *fileName, u32 lineNo, void *memPtr );
};

template<class T> inline T * operator + ( const MemStamp &memStamp, T *p )
{
#ifdef ENABLE_MEM_DEBUG
    printf( "P[%p] F[%-20s] L[%-6u]\n", p, memStamp._fileName, memStamp._lineNum);
#endif
    return ( p );
}

template<class T> inline void operator - ( const MemStamp &memStamp, T *p )
{
    delete p;
#ifdef ENABLE_MEM_DEBUG
    printf( "P[%p] F[%-20s] L[%-6u]\n", p, memStamp._fileName, memStamp._lineNum );
#endif
}

}

#ifdef ENABLE_MEM_DEBUG
#define mnew JaCoAP::MemStamp( _file_name, __LINE__ ) + new
#define mdelete JaCoAP::MemStamp( _file_name, __LINE__ ) -

#define mnew_g(__mnew_g_size__) JaCoapIF::MemAlloc((u8*)_file_name, __LINE__, __mnew_g_size__ )
#define mdelete_g(__mnew_g_ptr__) JaCoapIF::MemFree((u8*)_file_name, __LINE__,__mnew_g_ptr__)
#else
#define mnew new
#define mdelete delete

#define mnew_g(__mnew_g_size__) JaCoapIF::MemAlloc(__mnew_g_size__)
#define mdelete_g(__mnew_g_ptr__) JaCoapIF::MemFree(__mnew_g_ptr__)
#endif
