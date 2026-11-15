/*
 *  gucefGEOMETRY_TestApp: GUCEF test application for the Geometry module
 *
 *  Copyright (C) 1998 - 2026.  Dinand Vanvelzen
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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <stdlib.h>
#include <errno.h>
#if defined( __linux__ )
#include <pthread.h>
#endif

#include "TestGeometryAllocationCounter.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONFIGURATION                                                      //
//                                                                         //
//-------------------------------------------------------------------------*/

/* A sanitizer that replaces the allocator itself cannot be combined with replacing malloc here */
#if defined( __SANITIZE_ADDRESS__ ) || defined( __SANITIZE_THREAD__ )
  #define GUCEF_GEOMETRY_TEST_ALLOCATOR_REPLACED_BY_SANITIZER
#elif defined( __has_feature )
  #if __has_feature( address_sanitizer ) || __has_feature( thread_sanitizer ) || __has_feature( memory_sanitizer ) || __has_feature( realtime_sanitizer )
    #define GUCEF_GEOMETRY_TEST_ALLOCATOR_REPLACED_BY_SANITIZER
  #endif
#endif

#if defined( __linux__ ) && defined( __GLIBC__ ) && !defined( GUCEF_GEOMETRY_TEST_ALLOCATOR_REPLACED_BY_SANITIZER )
  #define GUCEF_GEOMETRY_TEST_ALLOCATION_COUNTER
#endif

/*-------------------------------------------------------------------------//
//                                                                         //
//      ALLOCATION COUNTING                                                //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifdef GUCEF_GEOMETRY_TEST_ALLOCATION_COUNTER

/* The allocation functions of the C library under their internal names */
extern "C" {
void* __libc_malloc( size_t size );
void __libc_free( void* memory );
void* __libc_calloc( size_t count, size_t size );
void* __libc_realloc( void* memory, size_t size );
void* __libc_memalign( size_t alignment, size_t size );
}

/* Constant initialised, so they are valid before any constructor runs: allocations happen during start-up */
static volatile bool g_countAllocations = false;
static volatile GUCEF::UInt32 g_allocationCount = 0;
static pthread_t g_countingThread;

static void
CountAllocation( void )
{
    if ( !g_countAllocations )
        return;
    if ( 0 == pthread_equal( pthread_self(), g_countingThread ) )
        return;
    ++g_allocationCount;
}

extern "C" {

__attribute__(( visibility( "default" ) )) void* malloc( size_t size ) { CountAllocation(); return __libc_malloc( size ); }
__attribute__(( visibility( "default" ) )) void free( void* memory ) { __libc_free( memory ); }
__attribute__(( visibility( "default" ) )) void* calloc( size_t count, size_t size ) { CountAllocation(); return __libc_calloc( count, size ); }
__attribute__(( visibility( "default" ) )) void* realloc( void* memory, size_t size ) { CountAllocation(); return __libc_realloc( memory, size ); }
__attribute__(( visibility( "default" ) )) void* memalign( size_t alignment, size_t size ) { CountAllocation(); return __libc_memalign( alignment, size ); }
__attribute__(( visibility( "default" ) )) void* aligned_alloc( size_t alignment, size_t size ) { CountAllocation(); return __libc_memalign( alignment, size ); }

__attribute__(( visibility( "default" ) )) int
posix_memalign( void** memory, size_t alignment, size_t size )
{
    CountAllocation();
    void* result = __libc_memalign( alignment, size );
    if ( GUCEF_NULL == result )
        return ENOMEM;
    *memory = result;
    return 0;
}

}

#endif /* GUCEF_GEOMETRY_TEST_ALLOCATION_COUNTER ? */

/*-------------------------------------------------------------------------*/

namespace GeometryAllocationCounter {

bool
IsAvailable( void )
{
    #ifdef GUCEF_GEOMETRY_TEST_ALLOCATION_COUNTER
    return true;
    #else
    return false;
    #endif
}

/*-------------------------------------------------------------------------*/

void
StartCounting( void )
{
    #ifdef GUCEF_GEOMETRY_TEST_ALLOCATION_COUNTER
    g_countingThread = pthread_self();
    g_allocationCount = 0;
    g_countAllocations = true;
    #endif
}

/*-------------------------------------------------------------------------*/

GUCEF::UInt32
StopCounting( void )
{
    #ifdef GUCEF_GEOMETRY_TEST_ALLOCATION_COUNTER
    g_countAllocations = false;
    return g_allocationCount;
    #else
    return 0;
    #endif
}

/*-------------------------------------------------------------------------*/

void
AllocateProbe( void )
{
    // stored through a volatile pointer, so the compiler cannot remove the pair
    void* volatile probe = malloc( 16 );
    free( probe );
}

}; /* namespace GeometryAllocationCounter */

/*-------------------------------------------------------------------------*/
