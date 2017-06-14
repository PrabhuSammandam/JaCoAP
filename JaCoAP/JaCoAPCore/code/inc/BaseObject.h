/*
 * BaseObject.h
 *
 *  Created on: Jun 12, 2017
 *      Author: psammand
 */

#pragma once

#include <stdio.h>
#include <typeinfo>
#include "JaCoapIF.h"

#ifdef ENABLE_MEM_DEBUG
#define ALLOC_MEM_DEBUG_PRINT(__alloc_x__,__alloc_size__) printf("Alloc Type[%-20s] size[%-6d] ", #__alloc_x__, (u32)__alloc_size__);
#define DEALLOC_MEM_DEBUG_PRINT(__dealloc_x__) printf("Free  Type[%-20s] ", #__dealloc_x__);
#else
#define ALLOC_MEM_DEBUG_PRINT(__alloc_x__,__alloc_size__)
#define DEALLOC_MEM_DEBUG_PRINT(__dealloc_x__)
#endif

#define DECLARE_NEW_OPERATOR void *operator   new ( size_t size );
#define DECLARE_DELETE_OPERATOR void operator delete ( void *ptr );

#define DECLARE_MEMORY_OPERATORS DECLARE_NEW_OPERATOR DECLARE_DELETE_OPERATOR

#define DEFINE_NEW_OPERATOR(__x__) void * operator   new ( size_t size ) { 	ALLOC_MEM_DEBUG_PRINT(__x__,size) return ( JaCoapIF::MemAlloc( size ) ); }
#define DEFINE_DELETE_OPERATOR(__x__) void operator delete ( void *ptr ) {  DEALLOC_MEM_DEBUG_PRINT(__x__) JaCoapIF::MemFree( ptr ); }

#define DEFINE_MEMORY_OPERATORS(__x__) DEFINE_NEW_OPERATOR(__x__) DEFINE_DELETE_OPERATOR(__x__)
