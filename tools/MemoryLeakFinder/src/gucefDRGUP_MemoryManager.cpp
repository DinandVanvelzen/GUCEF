/*
 *  MemoryLeakFinder: GUCEF module providing memory diagnostics
 *  Copyright (C) 2002 - 2024.  Dinand Vanvelzen
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
 *
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

/* MUST be first — prevents our own new/delete macros from firing in this file */
#include "gucef_dynnewoff.h"

#undef GUCEF_USE_CALLSTACK_TRACING
#undef GUCEF_USE_CALLSTACK_PLATFORM_TRACING

#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

#ifndef GUCEF_DRGUP_MEMORYMANAGER_H
#include "gucefDRGUP_MemoryManager.h"
#define GUCEF_DRGUP_MEMORYMANAGER_H
#endif /* GUCEF_DRGUP_MEMORYMANAGER_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_DRGUP_SMEMORYTRACKERCONFIG_H
#include "gucefDRGUP_SMemoryTrackerConfig.h"
#define GUCEF_DRGUP_SMEMORYTRACKERCONFIG_H
#endif /* GUCEF_DRGUP_SMEMORYTRACKERCONFIG_H ? */

#ifndef GUCEF_DRGUP_CMEMORYTRACKER_H
#include "gucefDRGUP_CMemoryTracker.h"
#define GUCEF_DRGUP_CMEMORYTRACKER_H
#endif /* GUCEF_DRGUP_CMEMORYTRACKER_H ? */

#ifndef GUCEF_DRGUP_CREPORTER_H
#include "gucefDRGUP_CReporter.h"
#define GUCEF_DRGUP_CREPORTER_H
#endif /* GUCEF_DRGUP_CREPORTER_H ? */

#ifndef GUCEF_DRGUP_CCALLSTACKTRACKER_H
#include "gucefDRGUP_CCallStackTracker.h"
#define GUCEF_DRGUP_CCALLSTACKTRACKER_H
#endif /* GUCEF_DRGUP_CCALLSTACKTRACKER_H ? */

#ifndef GUCEF_DRGUP_CLOCKTRACER_H
#include "gucefDRGUP_CLockTracer.h"
#define GUCEF_DRGUP_CLOCKTRACER_H
#endif /* GUCEF_DRGUP_CLOCKTRACER_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRGUP {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/* Forward declarations for CRT hook functions defined later in this file */
#if defined( GUCEF_DRGUP_HOOK_CRT_HEAP ) && defined( _MSC_VER ) && !defined( GUCEF_DRGUP_ASAN_ACTIVE )
static void InstallCrtHook( void );
static void RemoveCrtHook( void );
#endif

/*-------------------------------------------------------------------------*/

/**
 * DRGUP_Initialize():
 *  Initialize the memory tracking subsystem.  Safe to call multiple times.
 *  Initializes callstack tracker, lock tracer, and memory tracker in order.
 *
 *  Return Type : __int32 -> 1 on success, 0 on failure.
 */
__int32
DRGUP_Initialize( void )
{
    MT::PrecisionTimerInit();

    /* Lazy-construct CCallStackTracker first (used by CMemoryTracker) */
    CCallStackTracker* callstackTracker = CCallStackTracker::Instance();
    if ( GUCEF_NULL == callstackTracker )
        return 0;

    /* Lazy-construct CLockTracer — disabled when TSan is active to avoid false positives */
    #ifndef GUCEF_DRGUP_TSAN_ACTIVE
    CLockTracer* lockTracer = CLockTracer::Instance();
    if ( GUCEF_NULL == lockTracer )
        return 0;
    #endif /* GUCEF_DRGUP_TSAN_ACTIVE */

    /* Lazy-construct CMemoryTracker with defaults if not yet constructed */
    CMemoryTracker* tracker = CMemoryTracker::Instance();
    if ( GUCEF_NULL == tracker )
        return 0;

    /* Apply default config if this is the first init */
    SMemoryTrackerConfig cfg;
    SMemoryTrackerConfig_SetDefaults( cfg );
    tracker->ApplyConfig( cfg );

    /* Install CRT heap hook if opted in */
    #if defined( GUCEF_DRGUP_HOOK_CRT_HEAP ) && defined( _MSC_VER ) && !defined( GUCEF_DRGUP_ASAN_ACTIVE )
    InstallCrtHook();
    #endif

    return 1;
}

/*-------------------------------------------------------------------------*/

/**
 * DRGUP_Shutdown():
 *  Dump the final report and tear down all subsystems in reverse-init order.
 *
 *  Return Type : __int32 -> 1 always.
 */
__int32
DRGUP_Shutdown( void )
{
    /* --- diagnostic: log each step to a file for crash location analysis --- */
    FILE* dbg = fopen( "memman_shutdown_debug.txt", "a" );
    if ( dbg ) { fprintf( dbg, "SHUTDOWN ENTER\n" ); fflush( dbg ); }

    if ( CMemoryTracker::IsDestructed() )
    {
        if ( dbg ) { fprintf( dbg, "SHUTDOWN already destructed\n" ); fclose( dbg ); }
        return 1;
    }

    if ( dbg ) { fprintf( dbg, "SHUTDOWN step 1: IsConstructed=%d\n", (int)CMemoryTracker::IsConstructed() ); fflush( dbg ); }

    if ( CMemoryTracker::IsConstructed() )
    {
        /* Dump the final report while the tracker is still alive */
        CReporter reporter;
        if ( dbg ) { fprintf( dbg, "SHUTDOWN step 2: before DumpLogReport\n" ); fflush( dbg ); }
        reporter.DumpLogReport();
        if ( dbg ) { fprintf( dbg, "SHUTDOWN step 3: after DumpLogReport\n" ); fflush( dbg ); }
    }

    /* Remove CRT heap hook before teardown */
    #if defined( GUCEF_DRGUP_HOOK_CRT_HEAP ) && defined( _MSC_VER ) && !defined( GUCEF_DRGUP_ASAN_ACTIVE )
    RemoveCrtHook();
    #endif

    /* Tear down in reverse init order */
    #ifndef GUCEF_DRGUP_TSAN_ACTIVE
    if ( dbg ) { fprintf( dbg, "SHUTDOWN step 4: before CLockTracer::Deinstance\n" ); fflush( dbg ); }
    CLockTracer::Deinstance();
    if ( dbg ) { fprintf( dbg, "SHUTDOWN step 5: after CLockTracer::Deinstance\n" ); fflush( dbg ); }
    #endif /* GUCEF_DRGUP_TSAN_ACTIVE */

    if ( dbg ) { fprintf( dbg, "SHUTDOWN step 6: before CCallStackTracker::Deinstance\n" ); fflush( dbg ); }
    CCallStackTracker::Deinstance();
    if ( dbg ) { fprintf( dbg, "SHUTDOWN step 7: after CCallStackTracker::Deinstance\n" ); fflush( dbg ); }

    if ( dbg ) { fprintf( dbg, "SHUTDOWN step 8: before CMemoryTracker::Deinstance\n" ); fflush( dbg ); }
    CMemoryTracker::Deinstance();
    if ( dbg ) { fprintf( dbg, "SHUTDOWN step 9: after CMemoryTracker::Deinstance\n" ); fflush( dbg ); }

    if ( dbg ) { fprintf( dbg, "SHUTDOWN COMPLETE\n" ); fclose( dbg ); }
    return 1;
}

/*-------------------------------------------------------------------------*/

/**
 * DRGUP_AllocateMemory():
 *  Track a new or placement-new or realloc allocation.
 *  Falls back to raw malloc if the tracker is not available.
 */
void*
DRGUP_AllocateMemory( const char* file     ,
                       int         line     ,
                       size_t      size     ,
                       char        allocType,
                       void*       address  ,
                       const char* typeName )
{
    if ( CMemoryTracker::IsDestructed() )
        return ::malloc( size == 0 ? 1 : size );

    CMemoryTracker* tracker = CMemoryTracker::Instance();
    if ( GUCEF_NULL == tracker )
        return ::malloc( size == 0 ? 1 : size );

    return tracker->TrackAllocation( file, line, size, allocType, address, typeName );
}

/*-------------------------------------------------------------------------*/

/**
 * DRGUP_DeAllocateMemory():
 *  Track a deallocation.  Pops the owner context pushed by the preceding DRGUP_SetOwner call.
 */
void
DRGUP_DeAllocateMemory( void*       address  ,
                         char        allocType,
                         const char* typeName )
{
    if ( CMemoryTracker::IsDestructed() )
        return; /* in teardown; OS will reclaim */

    CMemoryTracker* tracker = CMemoryTracker::Instance();
    if ( GUCEF_NULL == tracker )
    {
        ::free( address );
        return;
    }

    tracker->TrackDeallocation( address, allocType, typeName );
}

/*-------------------------------------------------------------------------*/

/**
 * DRGUP_DeAllocateMemoryEx():
 *  Combined SetOwner + DeAllocateMemory for sites that know file/line at dealloc time.
 */
void
DRGUP_DeAllocateMemoryEx( const char* file     ,
                           int         line     ,
                           void*       address  ,
                           char        allocType,
                           const char* typeName )
{
    if ( CMemoryTracker::IsDestructed() )
        return;

    CMemoryTracker* tracker = CMemoryTracker::Instance();
    if ( GUCEF_NULL == tracker )
    {
        ::free( address );
        return;
    }

    tracker->TrackDeallocationEx( file, line, address, allocType, typeName );
}

/*-------------------------------------------------------------------------*/

/**
 * DRGUP_SetOwner():
 *  Push file/line context for the imminent deallocation.
 *  Must be paired with a subsequent DRGUP_DeAllocateMemory call on the same thread.
 */
__int32
DRGUP_SetOwner( const char* file     ,
                 int         line     ,
                 const char* typeName )
{
    if ( CMemoryTracker::IsDestructed() )
        return 0;

    CMemoryTracker* tracker = CMemoryTracker::Instance();
    if ( GUCEF_NULL == tracker )
        return 0;

    return (tracker->PushOwnerContext( file, line, typeName ) >= 0) ? 1 : 0;
}

/*-------------------------------------------------------------------------*/

/**
 * DRGUP_DumpLogReport():
 *  Write the statistics summary to the log file.
 */
void
DRGUP_DumpLogReport( void )
{
    if ( !CMemoryTracker::IsConstructed() )
        return;

    CReporter reporter;
    reporter.DumpLogReport();
}

/*-------------------------------------------------------------------------*/

/**
 * DRGUP_DumpMemoryAllocations():
 *  Write all currently live allocations to the log file.
 */
void
DRGUP_DumpMemoryAllocations( void )
{
    if ( !CMemoryTracker::IsConstructed() )
        return;

    CReporter reporter;
    reporter.DumpMemoryAllocations();
}

/*-------------------------------------------------------------------------*/

void
DRGUP_SuppressMismatchCheck( void* address )
{
    if ( GUCEF_NULL == address )
        return;

    CMemoryTracker* tracker = CMemoryTracker::Instance();
    if ( GUCEF_NULL == tracker )
        return;

    /* Acquire write lock to safely access the registry */
    MT::CScopeWriterLock writeLock( tracker->GetDataLock() );
    /* GetRegistry() returns const ref, but the CAllocationRecord* values are mutable */
    const CMemoryTracker::TRegistry& reg = tracker->GetRegistry();
    CMemoryTracker::TRegistry::const_iterator i = reg.find( (UIntPtr) address );
    if ( i != reg.end() && GUCEF_NULL != i->second )
    {
        /* The pointer in the registry points to a mutable record */
        i->second->suppressMismatchCheck = 1;
    }
}

/*-------------------------------------------------------------------------*/

void
DRGUP_LockProtectsRange( void* lockId, const void* address, size_t size )
{
    #ifndef GUCEF_DRGUP_TSAN_ACTIVE
    CLockTracer* lockTracer = CLockTracer::Instance();
    if ( GUCEF_NULL != lockTracer )
        lockTracer->LockProtectsRange( lockId, address, size );
    #endif /* GUCEF_DRGUP_TSAN_ACTIVE */
}

/*-------------------------------------------------------------------------*/

void
DRGUP_LockUnprotectsRange( void* lockId )
{
    #ifndef GUCEF_DRGUP_TSAN_ACTIVE
    CLockTracer* lockTracer = CLockTracer::Instance();
    if ( GUCEF_NULL != lockTracer )
        lockTracer->LockUnprotectsRange( lockId );
    #endif /* GUCEF_DRGUP_TSAN_ACTIVE */
}

/*-------------------------------------------------------------------------*/

void
DRGUP_DumpCallsiteReport( UInt32 topN )
{
    if ( !CMemoryTracker::IsConstructed() )
        return;

    CReporter reporter;
    reporter.DumpCallsiteReport( topN );
}

/*-------------------------------------------------------------------------*/

void
DRGUP_DumpSizeHistogram( void )
{
    if ( !CMemoryTracker::IsConstructed() )
        return;

    CReporter reporter;
    reporter.DumpSizeHistogram();
}

/*-------------------------------------------------------------------------*/

void
DRGUP_DumpTimeline( const char* path )
{
    if ( !CMemoryTracker::IsConstructed() || GUCEF_NULL == path )
        return;

    CReporter reporter;
    reporter.DumpTimeline( path );
}

/*-------------------------------------------------------------------------*/

void
DRGUP_DumpMassifFormat( const char* path )
{
    if ( !CMemoryTracker::IsConstructed() || GUCEF_NULL == path )
        return;

    CReporter reporter;
    reporter.DumpMassifFormat( path );
}

/*-------------------------------------------------------------------------*/

/**
 * DRGUP_SetLogFile():
 *  Change the log file path from the default "GUCEF_memlog.txt".
 */
void
DRGUP_SetLogFile( const char* file )
{
    CMemoryTracker* tracker = CMemoryTracker::Instance();
    if ( GUCEF_NULL == tracker || GUCEF_NULL == file )
        return;

    SMemoryTrackerConfig cfg = tracker->GetConfig();
    cfg.logFilePath = file;
    tracker->ApplyConfig( cfg );
}

/*-------------------------------------------------------------------------*/

/**
 * DRGUP_SetExhaustiveTesting():
 *  Enable or disable exhaustive testing (large padding + log every operation).
 */
void
DRGUP_SetExhaustiveTesting( UInt32 test )
{
    if ( test > 0 )
    {
        DRGUP_SetPaddingSize( 1024 );
        DRGUP_SetLogAlways( 1 );

        CMemoryTracker* tracker = CMemoryTracker::Instance();
        if ( GUCEF_NULL != tracker )
        {
            SMemoryTrackerConfig cfg = tracker->GetConfig();
            cfg.exhaustiveTesting = true;
            tracker->ApplyConfig( cfg );
        }
    }
    else
    {
        DRGUP_SetPaddingSize( 4 );
        DRGUP_SetLogAlways( 0 );

        CMemoryTracker* tracker = CMemoryTracker::Instance();
        if ( GUCEF_NULL != tracker )
        {
            SMemoryTrackerConfig cfg = tracker->GetConfig();
            cfg.exhaustiveTesting = false;
            tracker->ApplyConfig( cfg );
        }
    }
}

/*-------------------------------------------------------------------------*/

/**
 * DRGUP_SetLogAlways():
 *  Log every allocation and deallocation to the log file.
 */
void
DRGUP_SetLogAlways( UInt32 log )
{
    CMemoryTracker* tracker = CMemoryTracker::Instance();
    if ( GUCEF_NULL == tracker )
        return;

    SMemoryTrackerConfig cfg = tracker->GetConfig();
    cfg.logAlways = ( log > 0 );
    tracker->ApplyConfig( cfg );
}

/*-------------------------------------------------------------------------*/

/**
 * DRGUP_SetPaddingSize():
 *  Set the size (in longs) of sentinel padding on each side of every allocation.
 */
void
DRGUP_SetPaddingSize( UInt32 size )
{
    if ( 0 == size )
        return;

    CMemoryTracker* tracker = CMemoryTracker::Instance();
    if ( GUCEF_NULL == tracker )
        return;

    SMemoryTrackerConfig cfg = tracker->GetConfig();
    cfg.paddingSize = size;
    tracker->ApplyConfig( cfg );
}

/*-------------------------------------------------------------------------*/

/**
 * DRGUP_CleanLogFile():
 *  Delete the existing log file on the next write.
 */
void
DRGUP_CleanLogFile( UInt32 clean )
{
    CMemoryTracker* tracker = CMemoryTracker::Instance();
    if ( GUCEF_NULL == tracker )
        return;

    SMemoryTrackerConfig cfg = tracker->GetConfig();
    cfg.cleanLogFileOnFirstRun = ( clean > 0 );
    tracker->ApplyConfig( cfg );
}

/*-------------------------------------------------------------------------*/

/**
 * DRGUP_BreakOnAllocation():
 *  Trigger a debugger breakpoint on the N-th allocation.
 */
void
DRGUP_BreakOnAllocation( int alloccount )
{
    CMemoryTracker* tracker = CMemoryTracker::Instance();
    if ( GUCEF_NULL == tracker )
        return;

    tracker->SetBreakOnAllocationCount( alloccount );
}

/*-------------------------------------------------------------------------*/

/**
 * DRGUP_BreakOnDeallocation():
 *  Trigger a debugger breakpoint when the specified address is deallocated.
 */
void
DRGUP_BreakOnDeallocation( void* address )
{
    if ( GUCEF_NULL == address )
        return;

    CMemoryTracker* tracker = CMemoryTracker::Instance();
    if ( GUCEF_NULL == tracker )
        return;

    tracker->SetBreakOnDeallocation( address );
}

/*-------------------------------------------------------------------------*/

/**
 * DRGUP_BreakOnReallocation():
 *  Trigger a debugger breakpoint when the specified address is reallocated.
 */
void
DRGUP_BreakOnReallocation( void* address )
{
    if ( GUCEF_NULL == address )
        return;

    CMemoryTracker* tracker = CMemoryTracker::Instance();
    if ( GUCEF_NULL == tracker )
        return;

    tracker->SetBreakOnReallocation( address );
}

/*-------------------------------------------------------------------------*/

void
DRGUP_ValidateKnownAllocPtr( const void* address ,
                              const char* file    ,
                              int         line    )
{
    if ( GUCEF_NULL == address )
        return;

    CMemoryTracker* tracker = CMemoryTracker::Instance();
    if ( GUCEF_NULL == tracker )
        return;

    tracker->ValidateKnownAllocPtr( address, file, line );
}

/*-------------------------------------------------------------------------*/

void
DRGUP_ValidateKnownAllocBlock( const void* address  ,
                                UInt32      blocksize ,
                                const char* file     ,
                                int         line     )
{
    if ( GUCEF_NULL == address )
        return;

    CMemoryTracker* tracker = CMemoryTracker::Instance();
    if ( GUCEF_NULL == tracker )
        return;

    tracker->ValidateKnownAllocBlock( address, blocksize, file, line );
}

/*-------------------------------------------------------------------------*/

void
DRGUP_ValidateChunk( const void* address  ,
                      const void* chunk    ,
                      UInt32      blocksize,
                      const char* file     ,
                      int         line     )
{
    if ( GUCEF_NULL == address )
        return;

    CMemoryTracker* tracker = CMemoryTracker::Instance();
    if ( GUCEF_NULL == tracker )
        return;

    tracker->ValidateChunk( address, chunk, blocksize, file, line );
}

/*-------------------------------------------------------------------------*/

void
DRGUP_ValidateAccessibility( const void* address  ,
                              UInt32      blocksize,
                              const char* file     ,
                              int         line     )
{
    if ( GUCEF_NULL == address )
        return;

    CMemoryTracker* tracker = CMemoryTracker::Instance();
    if ( GUCEF_NULL == tracker )
        return;

    tracker->ValidateAccessibility( address, blocksize, file, line );
}

/*-------------------------------------------------------------------------*/

void
DRGUP_ValidatePendingDestructor( const char* file    ,
                                  int         line    ,
                                  const void* address ,
                                  size_t      size    ,
                                  const char* typeName )
{
    if ( GUCEF_NULL == address )
        return;

    CMemoryTracker* tracker = CMemoryTracker::Instance();
    if ( GUCEF_NULL == tracker )
        return;

    tracker->ValidatePendingDestructor( file, line, address, size, typeName );
}

/*-------------------------------------------------------------------------*/

void
DRGUP_ValidateFinishedDestructor( const char* file    ,
                                   int         line    ,
                                   const void* address ,
                                   size_t      size    ,
                                   const char* typeName )
{
    if ( GUCEF_NULL == address )
        return;

    CMemoryTracker* tracker = CMemoryTracker::Instance();
    if ( GUCEF_NULL == tracker )
        return;

    tracker->ValidateFinishedDestructor( file, line, address, size, typeName );
}

/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL NEW / DELETE REPLACEMENT (opt-in via GUCEF_DRGUP_GLOBAL_NEW_OVERRIDE)
//                                                                         //
//  When GUCEF_DRGUP_GLOBAL_NEW_OVERRIDE is defined at DLL build time, all    //
//  C++ operator-new and operator-delete calls in the process that resolve   //
//  through the normal ODR mechanism are routed through the tracker.         //
//                                                                         //
//  WARNING: Do NOT define this if any other library in the link unit also   //
//  overrides operator new/delete.  Linker will report a duplicate symbol.   //
//-------------------------------------------------------------------------*/

#ifdef GUCEF_DRGUP_GLOBAL_NEW_OVERRIDE

#pragma message( "GUCEF MemoryLeakFinder: GUCEF_DRGUP_GLOBAL_NEW_OVERRIDE is active — " \
                 "global operator new/delete are intercepted. " \
                 "Ensure no other library in this link unit also overrides them." )

} /* close namespace DRGUP */
} /* close namespace GUCEF */

void* operator new( size_t size )
{
    return GUCEF::DRGUP::DRGUP_AllocateMemory( "<global new>", 0, size, MM_NEW, GUCEF_NULL, GUCEF_NULL );
}

void* operator new[]( size_t size )
{
    return GUCEF::DRGUP::DRGUP_AllocateMemory( "<global new[]>", 0, size, MM_NEW_ARRAY, GUCEF_NULL, GUCEF_NULL );
}

void operator delete( void* p ) GUCEF_NOEXCEPT
{
    if ( GUCEF_NULL != p )
        GUCEF::DRGUP::DRGUP_DeAllocateMemory( p, MM_DELETE, GUCEF_NULL );
}

void operator delete[]( void* p ) GUCEF_NOEXCEPT
{
    if ( GUCEF_NULL != p )
        GUCEF::DRGUP::DRGUP_DeAllocateMemory( p, MM_DELETE_ARRAY, GUCEF_NULL );
}

/* Sized delete (C++14) */
#if __cplusplus >= 201402L
void operator delete( void* p, size_t ) GUCEF_NOEXCEPT
{
    if ( GUCEF_NULL != p )
        GUCEF::DRGUP::DRGUP_DeAllocateMemory( p, MM_DELETE, GUCEF_NULL );
}

void operator delete[]( void* p, size_t ) GUCEF_NOEXCEPT
{
    if ( GUCEF_NULL != p )
        GUCEF::DRGUP::DRGUP_DeAllocateMemory( p, MM_DELETE_ARRAY, GUCEF_NULL );
}
#endif /* C++14 */

namespace GUCEF {
namespace DRGUP {

#endif /* GUCEF_DRGUP_GLOBAL_NEW_OVERRIDE */

/*-------------------------------------------------------------------------//
//                                                                         //
//      CRT HEAP HOOK (opt-in via GUCEF_DRGUP_HOOK_CRT_HEAP, MSVC only)      //
//                                                                         //
//  When defined, _CrtSetAllocHook intercepts all CRT malloc/calloc/       //
//  realloc/free calls within the same CRT instance.  File/line info is    //
//  not available from the CRT hook, but size and type are.                //
//-------------------------------------------------------------------------*/

#if defined( GUCEF_DRGUP_HOOK_CRT_HEAP ) && defined( _MSC_VER ) && !defined( GUCEF_DRGUP_ASAN_ACTIVE )

#include <crtdbg.h>

static bool g_crtHookActive = false;

static int __cdecl
GucefCrtAllocHook( int      allocType ,
                   void*    userData  ,
                   size_t   size      ,
                   int      blockType ,
                   long     requestNumber,
                   const unsigned char* filename,
                   int      lineNumber )
{
    /* Ignore internal CRT allocations */
    if ( blockType == _CRT_BLOCK )
        return TRUE;

    /* Only intercept when tracker is alive and re-entrancy guard allows */
    if ( !CMemoryTracker::IsConstructed() || CMemoryTracker::IsDestructed() )
        return TRUE;

    CMemoryTracker* tracker = CMemoryTracker::Instance();
    if ( GUCEF_NULL == tracker )
        return TRUE;

    const char* file = GUCEF_NULL != filename ? (const char*) filename : "<CRT>";

    switch ( allocType )
    {
        case _HOOK_ALLOC:
            /* CRT hook is informational only — actual allocation done by CRT */
            break;
        case _HOOK_REALLOC:
            break;
        case _HOOK_FREE:
            break;
        default:
            break;
    }
    return TRUE; /* Allow CRT to proceed with the actual operation */
}

static void
InstallCrtHook( void )
{
    if ( !g_crtHookActive )
    {
        _CrtSetAllocHook( GucefCrtAllocHook );
        g_crtHookActive = true;
    }
}

static void
RemoveCrtHook( void )
{
    if ( g_crtHookActive )
    {
        _CrtSetAllocHook( GUCEF_NULL );
        g_crtHookActive = false;
    }
}

#endif /* GUCEF_DRGUP_HOOK_CRT_HEAP */

#ifdef MEMCHECK_OLEAPI

/**
 * DRGUP_SysAllocString():
 *  Memory-tracking replacement for OLE SysAllocString().
 *  Allocates [4-byte length prefix][wchar_t data] and returns pointer to the wchar_t data.
 */
wchar_t*
DRGUP_SysAllocString( const char*    file ,
                       int            line ,
                       const wchar_t* str  )
{
    if ( GUCEF_NULL == str )
        return GUCEF_NULL;

    int i = 0;
    while ( str[ i ] != 0 )
        ++i;

    char* buffer = (char*) DRGUP_AllocateMemory( file, line, 4 + (i * sizeof(wchar_t)), MM_OLE_ALLOC, GUCEF_NULL, GUCEF_NULL );
    if ( GUCEF_NULL != buffer )
    {
        unsigned int* bufferPrefix = (unsigned int*) buffer;
        *bufferPrefix = (unsigned int) i;
        buffer += 4;
        return (wchar_t*) buffer;
    }
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

/**
 * DRGUP_SysAllocStringByteLen():
 *  Memory-tracking replacement for OLE SysAllocStringByteLen().
 */
wchar_t*
DRGUP_SysAllocStringByteLen( const char*  file       ,
                              int          line       ,
                              const char*  str        ,
                              unsigned int bufferSize )
{
    char* buffer = (char*) DRGUP_AllocateMemory( file, line, (size_t)4 + bufferSize, MM_OLE_ALLOC, GUCEF_NULL, GUCEF_NULL );
    if ( GUCEF_NULL != buffer )
    {
        unsigned int* bufferPrefix = (unsigned int*) buffer;
        *bufferPrefix = bufferSize;
        buffer += 4;

        if ( GUCEF_NULL != str && 0 < bufferSize )
        {
            size_t strLength = strlen( str );
            if ( strLength > bufferSize )
                strLength = (size_t) bufferSize - 1;
            memcpy( buffer, str, strLength );
            buffer[ strLength ] = 0;
        }
        return (wchar_t*) buffer;
    }
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

/**
 * DRGUP_SysAllocStringLen():
 *  Memory-tracking replacement for OLE SysAllocStringLen().
 */
wchar_t*
DRGUP_SysAllocStringLen( const char*    file        ,
                          int            line        ,
                          const wchar_t* str         ,
                          unsigned int   charsToCopy )
{
    int bufferSize = (charsToCopy + 1) * 2;
    char* buffer = (char*) DRGUP_AllocateMemory( file, line, 4 + bufferSize, MM_OLE_ALLOC, GUCEF_NULL, GUCEF_NULL );
    if ( GUCEF_NULL != buffer )
    {
        unsigned int* bufferPrefix = (unsigned int*) buffer;
        *bufferPrefix = (unsigned int) bufferSize;
        buffer += 4;

        if ( GUCEF_NULL != str && 0 < bufferSize )
        {
            int strLength = 0;
            while ( str[ strLength ] != 0 )
                ++strLength;

            int strByteLength = strLength * 2;
            if ( strByteLength > bufferSize )
                strByteLength = bufferSize - 2;

            wchar_t* wbuffer = (wchar_t*) buffer;
            memcpy( wbuffer, str, (size_t) strByteLength );
            wbuffer[ charsToCopy ] = 0;
        }
        return (wchar_t*) buffer;
    }
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

/**
 * DRGUP_SysFreeString():
 *  Memory-tracking replacement for OLE SysFreeString().
 */
void
DRGUP_SysFreeString( const char* file      ,
                      int         line      ,
                      wchar_t*    bstrString )
{
    if ( GUCEF_NULL == bstrString )
        return;

    char* buffer = ((char*) bstrString) - 4;
    DRGUP_DeAllocateMemoryEx( file, line, buffer, MM_OLE_FREE, GUCEF_NULL );
}

/*-------------------------------------------------------------------------*/

/**
 * DRGUP_SysReAllocString():
 *  Memory-tracking replacement for OLE SysReAllocString().
 */
void
DRGUP_SysReAllocString( const char*    file  ,
                         int            line  ,
                         wchar_t**      pbstr ,
                         const wchar_t* psz   )
{
    if ( GUCEF_NULL == pbstr || GUCEF_NULL == psz )
        return;

    /* Count characters in psz — psz is a plain wchar_t*, NOT a BSTR with a prefix */
    int i = 0;
    while ( psz[ i ] != 0 )
        ++i;
    unsigned int pszByteSize = (unsigned int) ( i * (int) sizeof(wchar_t) );

    char* buffer = ((char*)(*pbstr)) - 4;
    buffer = (char*) DRGUP_AllocateMemory( file, line, 4 + pszByteSize, MM_OLE_ALLOC, buffer, GUCEF_NULL );
    if ( GUCEF_NULL != buffer )
    {
        *( (unsigned int*) buffer ) = (unsigned int) i;
        buffer += 4;
        memcpy( buffer, psz, pszByteSize );
        *pbstr = (wchar_t*) buffer;
    }
    else
    {
        *pbstr = GUCEF_NULL;
    }
}

/*-------------------------------------------------------------------------*/

/**
 * DRGUP_SysReAllocStringLen():
 *  Memory-tracking replacement for OLE SysReAllocStringLen().
 */
void
DRGUP_SysReAllocStringLen( const char*    file  ,
                            int            line  ,
                            wchar_t**      pbstr ,
                            const wchar_t* psz   ,
                            unsigned int   len   )
{
    if ( GUCEF_NULL == pbstr )
        return;

    /* len is the number of wchar_t characters to copy from psz (not byte count) */
    unsigned int copyByteSize = len * (unsigned int) sizeof(wchar_t);

    char* buffer = ((char*)(*pbstr)) - 4;
    buffer = (char*) DRGUP_AllocateMemory( file, line, 4 + copyByteSize + (unsigned int) sizeof(wchar_t), MM_OLE_ALLOC, buffer, GUCEF_NULL );
    if ( GUCEF_NULL != buffer )
    {
        *( (unsigned int*) buffer ) = len;
        buffer += 4;
        *pbstr = (wchar_t*) buffer;

        if ( GUCEF_NULL != psz && 0 < len )
        {
            memcpy( buffer, psz, copyByteSize );
        }
        /* Always null-terminate */
        (*pbstr)[ len ] = 0;
    }
    else
    {
        *pbstr = GUCEF_NULL;
    }
}

#endif /* MEMCHECK_OLEAPI ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRGUP */
}; /* namespace GUCEF */

/*--------------------------------------------------------------------------*/
