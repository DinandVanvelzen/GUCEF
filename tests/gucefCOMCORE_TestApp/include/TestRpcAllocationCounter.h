/*
 *  gucefCOMCORE_TestApp: GUCEF test application for the COMCORE module
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

/*
 *  Counts the heap allocations of the calling thread between Start and Stop, for the allocation targets of the RPC
 *  design (15). Header only, as the other test headers of the RPC tests, so that gucefCOM_TestApp uses it too: exactly
 *  one source file of a test application defines GUCEF_RPCTEST_ALLOCATION_COUNTER_IMPLEMENTATION before it includes
 *  this header, which defines the counting replacements of malloc, calloc and realloc there.
 *
 *  Counting replaces the allocation functions of the C library, so it is only available with glibc and without the
 *  address or thread sanitizer, which bring their own allocator.
 */

#ifndef GUCEF_COMCORE_TESTAPP_TESTRPCALLOCATIONCOUNTER_H
#define GUCEF_COMCORE_TESTAPP_TESTRPCALLOCATIONCOUNTER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <stdlib.h>

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#if defined( __SANITIZE_ADDRESS__ ) || defined( __SANITIZE_THREAD__ )
  #define GUCEF_RPCTEST_SANITIZER_BUILD
#elif defined( __has_feature )
  #if __has_feature( address_sanitizer ) || __has_feature( thread_sanitizer )
    #define GUCEF_RPCTEST_SANITIZER_BUILD
  #endif
#endif

#if defined( __linux__ ) && defined( __GLIBC__ ) && !defined( GUCEF_RPCTEST_SANITIZER_BUILD )
  #include <string.h>
  #include <pthread.h>
  #include <execinfo.h>
  #include <dlfcn.h>
  #include <cxxabi.h>
  #define GUCEF_RPCTEST_COUNT_ALLOCATIONS
#endif

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace RPCTEST {

bool IsAllocationCountingAvailable( void );
const char* GetAllocationCountingUnavailableReason( void );

/**
 *  Counts the allocations of the calling thread from now on. With a module (an address of code in it, such as a static
 *  member function) only the allocations that module makes are counted: those whose first caller outside the C and
 *  C++ libraries, gucefMT, gucefCORE and the test application is code of that module. For a part of a send path whose
 *  callees log their traces in Debug builds (the sockets do), so that the count is the part's own.
 */
void StartAllocationCounting( const void* moduleCode = GUCEF_NULL );

/** Ends the counting: the number of allocations of the counting thread since the start */
GUCEF::UInt32 StopAllocationCounting( void );

/** The call stacks of the first allocations counted, one frame per line, for the log of a failing test */
GUCEF::CORE::CString GetCountedAllocationStacks( void );

}; /* namespace RPCTEST */

/*-------------------------------------------------------------------------*/

#ifdef GUCEF_RPCTEST_ALLOCATION_COUNTER_IMPLEMENTATION

#ifdef GUCEF_RPCTEST_COUNT_ALLOCATIONS

static volatile bool g_rpcTestCountAllocations = false;
static pthread_t g_rpcTestCountingThread;
static volatile GUCEF::UInt32 g_rpcTestAllocationCount = 0;
static const int g_rpcTestMaxTraces = 4;
static const int g_rpcTestMaxTraceDepth = 24;
static void* g_rpcTestTraces[ g_rpcTestMaxTraces ][ g_rpcTestMaxTraceDepth ];
static int g_rpcTestTraceDepths[ g_rpcTestMaxTraces ];
static volatile bool g_rpcTestTracing = false;
static const void* g_rpcTestModuleBase = GUCEF_NULL;
static const void* g_rpcTestSkippedBases[ 5 ];

static const void*
RpcTestModuleBaseOf( const void* address )
{
    Dl_info info;
    if ( GUCEF_NULL == address || 0 == dladdr( address, &info ) )
        return GUCEF_NULL;
    return info.dli_fbase;
}

/** With a module to attribute to: whether the allocation now in progress is that module's */
static bool
RpcTestIsModulesAllocation( void )
{
    void* frames[ 32 ];
    int depth = backtrace( frames, 32 );
    for ( int f=0; f<depth; ++f )
    {
        const void* base = RpcTestModuleBaseOf( frames[ f ] );
        bool skipped = GUCEF_NULL == base;
        for ( int s=0; s<5 && !skipped; ++s )
            skipped = base == g_rpcTestSkippedBases[ s ];
        if ( !skipped )
            return base == g_rpcTestModuleBase;
    }
    return false;
}

extern "C" void* __libc_malloc( size_t size );
extern "C" void* __libc_calloc( size_t count, size_t size );
extern "C" void* __libc_realloc( void* ptr, size_t size );

static inline void
RpcTestCountAllocation( void )
{
    if ( !g_rpcTestCountAllocations || g_rpcTestTracing || 0 == pthread_equal( pthread_self(), g_rpcTestCountingThread ) )
        return;
    if ( GUCEF_NULL != g_rpcTestModuleBase )
    {
        g_rpcTestTracing = true;
        bool modules = RpcTestIsModulesAllocation();
        g_rpcTestTracing = false;
        if ( !modules )
            return;
    }
    GUCEF::UInt32 index = g_rpcTestAllocationCount++;
    if ( index < (GUCEF::UInt32) g_rpcTestMaxTraces )
    {
        g_rpcTestTracing = true;
        g_rpcTestTraceDepths[ index ] = backtrace( g_rpcTestTraces[ index ], g_rpcTestMaxTraceDepth );
        g_rpcTestTracing = false;
    }
}

extern "C" void*
malloc( size_t size )
{
    RpcTestCountAllocation();
    return __libc_malloc( size );
}

extern "C" void*
calloc( size_t count, size_t size )
{
    RpcTestCountAllocation();
    return __libc_calloc( count, size );
}

extern "C" void*
realloc( void* ptr, size_t size )
{
    RpcTestCountAllocation();
    return __libc_realloc( ptr, size );
}

#endif /* GUCEF_RPCTEST_COUNT_ALLOCATIONS ? */

namespace RPCTEST {

bool
IsAllocationCountingAvailable( void )
{
    #ifdef GUCEF_RPCTEST_COUNT_ALLOCATIONS
    return true;
    #else
    return false;
    #endif
}

const char*
GetAllocationCountingUnavailableReason( void )
{
    #if defined( GUCEF_RPCTEST_COUNT_ALLOCATIONS )
    return "";
    #elif defined( GUCEF_RPCTEST_SANITIZER_BUILD )
    return "Allocations are not counted in a sanitizer build, the sanitizer owns malloc";
    #else
    return "Allocations are only counted where the C library allows malloc to be replaced (glibc)";
    #endif
}

void
StartAllocationCounting( const void* moduleCode )
{
    #ifdef GUCEF_RPCTEST_COUNT_ALLOCATIONS
    g_rpcTestModuleBase = RpcTestModuleBaseOf( moduleCode );
    g_rpcTestSkippedBases[ 0 ] = RpcTestModuleBaseOf( (const void*) &memcpy );
    g_rpcTestSkippedBases[ 1 ] = RpcTestModuleBaseOf( (const void*) &__cxa_begin_catch );
    g_rpcTestSkippedBases[ 2 ] = RpcTestModuleBaseOf( (const void*) &GUCEF::CORE::CCoreGlobal::Instance );
    g_rpcTestSkippedBases[ 3 ] = RpcTestModuleBaseOf( (const void*) &GUCEF::MT::PrecisionTickCount );
    g_rpcTestSkippedBases[ 4 ] = RpcTestModuleBaseOf( (const void*) &RpcTestModuleBaseOf );
    // the first call of backtrace() loads what it needs, that is not to be counted
    void* warmUp[ 2 ];
    backtrace( warmUp, 2 );
    for ( int i=0; i<g_rpcTestMaxTraces; ++i )
        g_rpcTestTraceDepths[ i ] = 0;
    g_rpcTestCountingThread = pthread_self();
    g_rpcTestAllocationCount = 0;
    g_rpcTestCountAllocations = true;
    #endif
}

GUCEF::UInt32
StopAllocationCounting( void )
{
    #ifdef GUCEF_RPCTEST_COUNT_ALLOCATIONS
    g_rpcTestCountAllocations = false;
    return g_rpcTestAllocationCount;
    #else
    return 0;
    #endif
}

GUCEF::CORE::CString
GetCountedAllocationStacks( void )
{
    GUCEF::CORE::CString text;
    #ifdef GUCEF_RPCTEST_COUNT_ALLOCATIONS
    for ( int i=0; i<g_rpcTestMaxTraces; ++i )
    {
        if ( 0 == g_rpcTestTraceDepths[ i ] )
            continue;
        char** symbols = backtrace_symbols( g_rpcTestTraces[ i ], g_rpcTestTraceDepths[ i ] );
        text += "allocation " + GUCEF::CORE::ToString( (GUCEF::UInt32) i + 1 ) + ":\n";
        for ( int f=0; f<g_rpcTestTraceDepths[ i ] && GUCEF_NULL != symbols; ++f )
            text += GUCEF::CORE::CString( "    " ) + symbols[ f ] + "\n";
        free( symbols );
    }
    #endif
    return text;
}

}; /* namespace RPCTEST */

#endif /* GUCEF_RPCTEST_ALLOCATION_COUNTER_IMPLEMENTATION ? */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMCORE_TESTAPP_TESTRPCALLOCATIONCOUNTER_H ? */
