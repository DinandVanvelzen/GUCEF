/*
 *  gucefDRN_TestApp: GUCEF test application for the DRN module
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

/*
 *  An address sanitizer brings its own malloc and does not get to set itself up when the allocation functions
 *  are replaced here, so with a sanitizer allocations are not counted
 */
#if defined( __SANITIZE_ADDRESS__ )
  #define DRNTEST_ADDRESS_SANITIZER
#elif defined( __has_feature )
  #if __has_feature( address_sanitizer )
    #define DRNTEST_ADDRESS_SANITIZER
  #endif
#endif

#if defined( __linux__ ) && defined( __GLIBC__ ) && !defined( DRNTEST_ADDRESS_SANITIZER )
  #include <pthread.h>
  #include <execinfo.h>
  #define DRNTEST_COUNT_ALLOCATIONS
#endif

#include "TestDRNAllocationCounter.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifdef DRNTEST_COUNT_ALLOCATIONS

static volatile bool g_countAllocations = false;
static pthread_t g_countingThread;
static volatile GUCEF::UInt32 g_allocationCount = 0;

static const int MaxTracedAllocations = 4;
static const int MaxTraceDepth = 24;
static void* g_traces[ MaxTracedAllocations ][ MaxTraceDepth ];
static int g_traceDepths[ MaxTracedAllocations ];
static volatile bool g_tracing = false;

/*-------------------------------------------------------------------------//
//                                                                         //
//      C LIBRARY INTERPOSERS                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

extern "C" void* __libc_malloc( size_t size );
extern "C" void* __libc_calloc( size_t count, size_t size );
extern "C" void* __libc_realloc( void* ptr, size_t size );

static inline void
CountAllocation( void )
{
    if ( !g_countAllocations || g_tracing || 0 == pthread_equal( pthread_self(), g_countingThread ) )
        return;
    GUCEF::UInt32 index = g_allocationCount++;
    if ( index < (GUCEF::UInt32) MaxTracedAllocations )
    {
        g_tracing = true;
        g_traceDepths[ index ] = backtrace( g_traces[ index ], MaxTraceDepth );
        g_tracing = false;
    }
}

extern "C" void*
malloc( size_t size )
{
    CountAllocation();
    return __libc_malloc( size );
}

extern "C" void*
calloc( size_t count, size_t size )
{
    CountAllocation();
    return __libc_calloc( count, size );
}

extern "C" void*
realloc( void* ptr, size_t size )
{
    CountAllocation();
    return __libc_realloc( ptr, size );
}

#endif /* DRNTEST_COUNT_ALLOCATIONS ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace DRNTEST {

/*-------------------------------------------------------------------------*/

bool
IsAllocationCountingAvailable( void )
{
    #ifdef DRNTEST_COUNT_ALLOCATIONS
    return true;
    #else
    return false;
    #endif
}

/*-------------------------------------------------------------------------*/

const char*
GetAllocationCountingUnavailableReason( void )
{
    #if defined( DRNTEST_COUNT_ALLOCATIONS )
    return "";
    #elif defined( DRNTEST_ADDRESS_SANITIZER )
    return "Allocations are not counted in an address sanitizer build, the sanitizer owns malloc";
    #else
    return "Allocations are only counted where the C library allows malloc to be replaced (glibc)";
    #endif
}

/*-------------------------------------------------------------------------*/

void
StartAllocationCounting( void )
{
    #ifdef DRNTEST_COUNT_ALLOCATIONS
    // the first call of backtrace() loads what it needs, that is not to be counted
    void* warmUp[ 2 ];
    backtrace( warmUp, 2 );
    for ( int i=0; i<MaxTracedAllocations; ++i )
        g_traceDepths[ i ] = 0;
    g_countingThread = pthread_self();
    g_allocationCount = 0;
    g_countAllocations = true;
    #endif
}

/*-------------------------------------------------------------------------*/

GUCEF::UInt32
StopAllocationCounting( void )
{
    #ifdef DRNTEST_COUNT_ALLOCATIONS
    g_countAllocations = false;
    return g_allocationCount;
    #else
    return 0;
    #endif
}

/*-------------------------------------------------------------------------*/

GUCEF::CORE::CString
GetCountedAllocationStacks( void )
{
    GUCEF::CORE::CString stacks;
    #ifdef DRNTEST_COUNT_ALLOCATIONS
    GUCEF::UInt32 traced = g_allocationCount < (GUCEF::UInt32) MaxTracedAllocations ? g_allocationCount : (GUCEF::UInt32) MaxTracedAllocations;
    for ( GUCEF::UInt32 i=0; i<traced; ++i )
    {
        char** symbols = backtrace_symbols( g_traces[ i ], g_traceDepths[ i ] );
        if ( GUCEF_NULL == symbols )
            continue;
        stacks += "allocation " + GUCEF::CORE::UInt32ToString( i+1 ) + ":";
        for ( int n=0; n<g_traceDepths[ i ]; ++n )
            stacks += GUCEF::CORE::CString( " | " ) + symbols[ n ];
        stacks += "\n";
        free( symbols );
    }
    #endif
    return stacks;
}

/*-------------------------------------------------------------------------*/

}; /* namespace DRNTEST */

/*-------------------------------------------------------------------------*/
