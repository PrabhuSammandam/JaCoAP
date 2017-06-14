#include <stdio.h>
#include "ja_types.h"
#include "JaCoapIF.h"

FILE_NAME( "JaCoapIF.cpp" )

namespace JaCoAP
{
void * JaCoapIF::MemAlloc( size_t size )
{
    void *mem = ::operator new ( size );

    //dbg_info( " size[%u]",(u32) size );
    return ( mem );
}

void JaCoapIF::MemFree( void *memPtr )
{
    //dbg_info( "Released mem[%p]", memPtr );
    ::operator delete ( memPtr );
}

void * JaCoapIF::MemAlloc( u8 *fileName, u32 lineNo, size_t size )
{
    void *mem = ::operator new ( size );

    printf( "Alloc Type[%-20s] size[%-6d] P[%p] F[%-20s] L[%-6u]\n","unsigned char", (u32) size, mem,  fileName, lineNo );
    return ( mem );
}

void JaCoapIF::MemFree( u8 *fileName, u32 lineNo, void *memPtr )
{
	printf( "Free  Type[%-20s] P[%p] F[%-20s] L[%-6u]\n","unsigned char", memPtr, fileName, lineNo );
    ::operator delete ( memPtr );
}
}
