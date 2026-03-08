/*
 *  gucef common header: provides header based platform wide facilities
 *
 *  Copyright (C) 1998 - 2020.  Dinand Vanvelzen
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#ifndef GUCEF_MEMORYMANAGERLOADER_INLINE_H
#define GUCEF_MEMORYMANAGERLOADER_INLINE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MEMORYMANAGERLOADER_LOAD_H
#include "gucef_MemoryManagerLoader_load.h"
#define GUCEF_MEMORYMANAGERLOADER_LOAD_H
#endif /* GUCEF_MEMORYMANAGERLOADER_LOAD_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

#if defined( GUCEF_USE_MEMORY_LEAK_CHECKER ) && defined( GUCEF_USE_PLATFORM_MEMORY_LEAK_CHECKER )

/*-------------------------------------------------------------------------*/

inline
void
MEMMAN_SetExhaustiveTesting( unsigned __int32 test )
{
    if ( 0 != MEMMAN_LazyLoadMemoryManager() )
        fp_MEMMAN_SetExhaustiveTesting( test );
}

/*-------------------------------------------------------------------------*/

inline
void
MEMMAN_SetPaddingSize( unsigned __int32 size )
{
    if ( 0 != MEMMAN_LazyLoadMemoryManager() )
        fp_MEMMAN_SetPaddingSize( size );
}

/*-------------------------------------------------------------------------*/

inline
MEMMAN_Int32
MEMMAN_SetOwner( const char* file, int line, const char* typeName )
{
    if ( 0 != MEMMAN_LazyLoadMemoryManager() )
        return fp_MEMMAN_SetOwner( file, line, typeName );
    return 0;
}

/*-------------------------------------------------------------------------*/

inline
void*
MEMMAN_malloc( const char* file, int line, size_t size )
{
    return ( 0 == MEMMAN_LazyLoadMemoryManager() ?
             malloc( size ) :
             fp_MEMMAN_AllocateMemory( file, line, size, MM_MALLOC, GUCEF_NULL, GUCEF_NULL ) );
}

/*-------------------------------------------------------------------------*/

inline
void*
MEMMAN_placement_new( const char* file, int line, size_t size, void* address, const char* typeName )
{
    if ( 0 != MEMMAN_LazyLoadMemoryManager() )
        fp_MEMMAN_AllocateMemory( file, line, size, MM_PLACEMENT_NEW, address, typeName );
    return address;
}

/*-------------------------------------------------------------------------*/

inline
void*
MEMMAN_calloc( const char* file, int line, size_t num, size_t size )
{
    return ( 0 == MEMMAN_LazyLoadMemoryManager() ?
             calloc( num, size ) :
             fp_MEMMAN_AllocateMemory( file, line, size * num, MM_CALLOC, NULL, GUCEF_NULL ) );
}

/*-------------------------------------------------------------------------*/

inline
void*
MEMMAN_realloc( const char* file, int line, void* ptr, size_t size )
{
    if ( 0 == MEMMAN_LazyLoadMemoryManager() )
        return realloc( ptr, size );
    return ptr ?
        fp_MEMMAN_AllocateMemory( file, line, size, MM_REALLOC, ptr, GUCEF_NULL ) :
        fp_MEMMAN_AllocateMemory( file, line, size, MM_MALLOC, GUCEF_NULL, GUCEF_NULL );
}

/*-------------------------------------------------------------------------*/

inline
void
MEMMAN_free( const char* file, int line, void* ptr )
{
    if ( 0 == MEMMAN_LazyLoadMemoryManager() )
        free( ptr );
    else
        fp_MEMMAN_DeAllocateMemoryEx( file, line, ptr, MM_FREE, GUCEF_NULL );
}

/*-------------------------------------------------------------------------*/

inline
void
MEMMAN_ValidateAccessibility( const void* address, unsigned __int32 blocksize, const char* file, int line )
{
    if ( 0 != MEMMAN_LazyLoadMemoryManager() )
        fp_MEMMAN_ValidateAccessibility( address, blocksize, file, line );
}

/*-------------------------------------------------------------------------*/

inline
void
MEMMAN_ValidatePendingDestructor( const char* file, int line, const void* address, size_t blocksize, const char* typeName )
{
    if ( 0 != MEMMAN_LazyLoadMemoryManager() )
        fp_MEMMAN_ValidatePendingDestructor( file, line, address, blocksize, typeName );
}

/*-------------------------------------------------------------------------*/

inline
void
MEMMAN_ValidateFinishedDestructor( const char* file, int line, const void* address, size_t blocksize, const char* typeName )
{
    if ( 0 != MEMMAN_LazyLoadMemoryManager() )
        fp_MEMMAN_ValidateFinishedDestructor( file, line, address, blocksize, typeName );
}

/*-------------------------------------------------------------------------*/

#endif /* defined( GUCEF_USE_MEMORY_LEAK_CHECKER ) && defined( GUCEF_USE_PLATFORM_MEMORY_LEAK_CHECKER ) ? */
#if ( defined( GUCEF_USE_MEMORY_LEAK_CHECKER ) && defined( GUCEF_USE_PLATFORM_MEMORY_LEAK_CHECKER ) && defined( GUCEF_PLATFORM_MEMORY_LEAK_CHECKER_INCLUDES_OLEAPI ) )

/*-------------------------------------------------------------------------*/

inline
wchar_t*
MEMMAN_SysAllocString( const char* file, int line, wchar_t* wcharStr )
{
    return ( 0 == MEMMAN_LazyLoadMemoryManager() ?
             SysAllocString( wcharStr ) :
             fp_MEMMAN_SysAllocString( file, line, wcharStr ) );
}

/*-------------------------------------------------------------------------*/

inline
wchar_t*
MEMMAN_SysAllocStringByteLen( const char* file, int line, const char* str, unsigned int bufferSize )
{
    return ( 0 == MEMMAN_LazyLoadMemoryManager() ?
             SysAllocStringByteLen( str, bufferSize ) :
             fp_MEMMAN_SysAllocStringByteLen( file, line, str, bufferSize ) );
}

/*-------------------------------------------------------------------------*/

inline
wchar_t*
MEMMAN_SysAllocStringLen( const char* file, int line, const wchar_t* str, unsigned int charsToCopy )
{
    return ( 0 == MEMMAN_LazyLoadMemoryManager() ?
             SysAllocStringLen( str, charsToCopy ) :
             fp_MEMMAN_SysAllocStringLen( file, line, str, charsToCopy ) );
}

/*-------------------------------------------------------------------------*/

inline
void
MEMMAN_SysFreeString( const char* file, int line, wchar_t* bstrString )
{
    if ( 0 == MEMMAN_LazyLoadMemoryManager() )
        SysFreeString( bstrString );
    else
        fp_MEMMAN_SysFreeString( file, line, bstrString );
}

/*-------------------------------------------------------------------------*/

inline
int
MEMMAN_SysReAllocString( const char* file, int line, wchar_t** pbstr, const wchar_t* psz )
{
    return ( 0 == MEMMAN_LazyLoadMemoryManager() ?
             SysReAllocString( pbstr, psz ) :
             fp_MEMMAN_SysReAllocString( file, line, pbstr, psz ) );
}

/*-------------------------------------------------------------------------*/

inline
int
MEMMAN_SysReAllocStringLen( const char* file, int line, wchar_t** pbstr, const wchar_t* psz, unsigned int len )
{
    return ( 0 == MEMMAN_LazyLoadMemoryManager() ?
             SysReAllocStringLen( pbstr, psz, len ) :
             fp_MEMMAN_SysReAllocStringLen( file, line, pbstr, psz, len ) );
}

/*-------------------------------------------------------------------------*/

#endif /* ( defined( GUCEF_USE_MEMORY_LEAK_CHECKER ) && defined( GUCEF_USE_PLATFORM_MEMORY_LEAK_CHECKER ) && defined( GUCEF_PLATFORM_MEMORY_LEAK_CHECKER_INCLUDES_OLEAPI ) ) ? */
#if defined( GUCEF_USE_CALLSTACK_TRACING ) && defined( GUCEF_USE_PLATFORM_CALLSTACK_TRACING )

/*-------------------------------------------------------------------------*/

inline
void
MEMMAN_CallstackScopeBegin( const char* file, int line )
{
    if ( 1 == MEMMAN_LazyLoadMemoryManager() )
        fp_MEMMAN_CallstackScopeBegin( file, line );
}

/*-------------------------------------------------------------------------*/

inline
void
MEMMAN_CallstackScopeEnd( void )
{
    if ( 1 == MEMMAN_LazyLoadMemoryManager() )
        fp_MEMMAN_CallstackScopeEnd();
}

/*-------------------------------------------------------------------------*/

#endif /* defined( GUCEF_USE_CALLSTACK_TRACING ) && defined( GUCEF_USE_PLATFORM_CALLSTACK_TRACING ) ? */
#if defined( GUCEF_USE_PLATFORM_LOCK_TRACER )

/*-------------------------------------------------------------------------*/

inline
void
MEMMAN_ExclusiveLockCreated( void* lockId )
{
    if ( 1 == MEMMAN_LazyLoadMemoryManager() )
        fp_MEMMAN_ExclusiveLockCreated( lockId );
}

/*-------------------------------------------------------------------------*/

inline
void
MEMMAN_ExclusiveLockObtained( void* lockId )
{
    if ( 1 == MEMMAN_LazyLoadMemoryManager() )
        fp_MEMMAN_ExclusiveLockObtained( lockId );
}

/*-------------------------------------------------------------------------*/

inline
void
MEMMAN_ExclusiveLockReleased( void* lockId )
{
    if ( 1 == MEMMAN_LazyLoadMemoryManager() )
        fp_MEMMAN_ExclusiveLockReleased( lockId );
}

/*-------------------------------------------------------------------------*/

inline
void
MEMMAN_ExclusiveLockAbandoned( void* lockId )
{
    if ( 1 == MEMMAN_LazyLoadMemoryManager() )
        fp_MEMMAN_ExclusiveLockAbandoned( lockId );
}

/*-------------------------------------------------------------------------*/

inline
void
MEMMAN_ExclusiveLockDestroy( void* lockId )
{
    if ( 1 == MEMMAN_LazyLoadMemoryManager() )
        fp_MEMMAN_ExclusiveLockDestroy( lockId );
}

/*-------------------------------------------------------------------------*/

#endif /* defined( GUCEF_USE_PLATFORM_LOCK_TRACER ) ? */
#if defined( GUCEF_USE_MEMORY_LEAK_CHECKER ) && defined( GUCEF_USE_PLATFORM_MEMORY_LEAK_CHECKER )

#undef GUCEF_DYNNEWOFF_H  /* allow the undef header to be included again later */

#endif /* defined( GUCEF_USE_MEMORY_LEAK_CHECKER ) && defined( GUCEF_USE_PLATFORM_MEMORY_LEAK_CHECKER ) ? */

/*--------------------------------------------------------------------------*/

#endif /* GUCEF_MEMORYMANAGERLOADER_INLINE_H ? */
