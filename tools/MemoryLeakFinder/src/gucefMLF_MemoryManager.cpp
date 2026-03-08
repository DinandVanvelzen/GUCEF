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
 *  Original MemoryManager.cpp concept by Peter Dalton (3/23/01).
 *  Rewritten as thin C API wrapper by Dinand Vanvelzen, 2024.
 *  All logic moved into CMemoryTracker / CReporter / CCallStackTracker / CLockTracer.
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

#ifndef GUCEF_MLF_MEMORYMANAGER_H
#include "gucefMLF_MemoryManager.h"
#define GUCEF_MLF_MEMORYMANAGER_H
#endif /* GUCEF_MLF_MEMORYMANAGER_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_MLF_SMEMORYTRACKERCONFIG_H
#include "gucefMLF_SMemoryTrackerConfig.h"
#define GUCEF_MLF_SMEMORYTRACKERCONFIG_H
#endif /* GUCEF_MLF_SMEMORYTRACKERCONFIG_H ? */

#ifndef GUCEF_MLF_CMEMORYTRACKER_H
#include "gucefMLF_CMemoryTracker.h"
#define GUCEF_MLF_CMEMORYTRACKER_H
#endif /* GUCEF_MLF_CMEMORYTRACKER_H ? */

#ifndef GUCEF_MLF_CREPORTER_H
#include "gucefMLF_CReporter.h"
#define GUCEF_MLF_CREPORTER_H
#endif /* GUCEF_MLF_CREPORTER_H ? */

#ifndef GUCEF_MLF_CCALLSTACKTRACKER_H
#include "gucefMLF_CCallStackTracker.h"
#define GUCEF_MLF_CCALLSTACKTRACKER_H
#endif /* GUCEF_MLF_CCALLSTACKTRACKER_H ? */

#ifndef GUCEF_MLF_CLOCKTRACER_H
#include "gucefMLF_CLockTracer.h"
#define GUCEF_MLF_CLOCKTRACER_H
#endif /* GUCEF_MLF_CLOCKTRACER_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MLF {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 * MEMMAN_Initialize():
 *  Initialize the memory tracking subsystem.  Safe to call multiple times.
 *  Initializes callstack tracker, lock tracer, and memory tracker in order.
 *
 *  Return Type : __int32 -> 1 on success, 0 on failure.
 */
__int32
MEMMAN_Initialize( void )
{
    MT::PrecisionTimerInit();

    /* Lazy-construct CCallStackTracker first (used by CMemoryTracker) */
    CCallStackTracker* callstackTracker = CCallStackTracker::Instance();
    if ( GUCEF_NULL == callstackTracker )
        return 0;

    /* Lazy-construct CLockTracer */
    CLockTracer* lockTracer = CLockTracer::Instance();
    if ( GUCEF_NULL == lockTracer )
        return 0;

    /* Lazy-construct CMemoryTracker with defaults if not yet constructed */
    CMemoryTracker* tracker = CMemoryTracker::Instance();
    if ( GUCEF_NULL == tracker )
        return 0;

    /* Apply default config if this is the first init */
    SMemoryTrackerConfig cfg;
    SMemoryTrackerConfig_SetDefaults( cfg );
    tracker->ApplyConfig( cfg );

    return 1;
}

/*-------------------------------------------------------------------------*/

/**
 * MEMMAN_Shutdown():
 *  Dump the final report and tear down all subsystems in reverse-init order.
 *
 *  Return Type : __int32 -> 1 always.
 */
__int32
MEMMAN_Shutdown( void )
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

    /* Tear down in reverse init order */
    if ( dbg ) { fprintf( dbg, "SHUTDOWN step 4: before CLockTracer::Deinstance\n" ); fflush( dbg ); }
    CLockTracer::Deinstance();
    if ( dbg ) { fprintf( dbg, "SHUTDOWN step 5: after CLockTracer::Deinstance\n" ); fflush( dbg ); }

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
 * MEMMAN_AllocateMemory():
 *  Track a new or placement-new or realloc allocation.
 *  Falls back to raw malloc if the tracker is not available.
 */
void*
MEMMAN_AllocateMemory( const char* file     ,
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
 * MEMMAN_DeAllocateMemory():
 *  Track a deallocation.  Pops the owner context pushed by the preceding MEMMAN_SetOwner call.
 */
void
MEMMAN_DeAllocateMemory( void*       address  ,
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
 * MEMMAN_DeAllocateMemoryEx():
 *  Combined SetOwner + DeAllocateMemory for sites that know file/line at dealloc time.
 */
void
MEMMAN_DeAllocateMemoryEx( const char* file     ,
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
 * MEMMAN_SetOwner():
 *  Push file/line context for the imminent deallocation.
 *  Must be paired with a subsequent MEMMAN_DeAllocateMemory call on the same thread.
 */
__int32
MEMMAN_SetOwner( const char* file     ,
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
 * MEMMAN_DumpLogReport():
 *  Write the statistics summary to the log file.
 */
void
MEMMAN_DumpLogReport( void )
{
    if ( !CMemoryTracker::IsConstructed() )
        return;

    CReporter reporter;
    reporter.DumpLogReport();
}

/*-------------------------------------------------------------------------*/

/**
 * MEMMAN_DumpMemoryAllocations():
 *  Write all currently live allocations to the log file.
 */
void
MEMMAN_DumpMemoryAllocations( void )
{
    if ( !CMemoryTracker::IsConstructed() )
        return;

    CReporter reporter;
    reporter.DumpMemoryAllocations();
}

/*-------------------------------------------------------------------------*/

/**
 * MEMMAN_SetLogFile():
 *  Change the log file path from the default "GUCEF_memlog.txt".
 */
void
MEMMAN_SetLogFile( const char* file )
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
 * MEMMAN_SetExhaustiveTesting():
 *  Enable or disable exhaustive testing (large padding + log every operation).
 */
void
MEMMAN_SetExhaustiveTesting( UInt32 test )
{
    if ( test > 0 )
    {
        MEMMAN_SetPaddingSize( 1024 );
        MEMMAN_SetLogAlways( 1 );

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
        MEMMAN_SetPaddingSize( 4 );
        MEMMAN_SetLogAlways( 0 );

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
 * MEMMAN_SetLogAlways():
 *  Log every allocation and deallocation to the log file.
 */
void
MEMMAN_SetLogAlways( UInt32 log )
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
 * MEMMAN_SetPaddingSize():
 *  Set the size (in longs) of sentinel padding on each side of every allocation.
 */
void
MEMMAN_SetPaddingSize( UInt32 size )
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
 * MEMMAN_CleanLogFile():
 *  Delete the existing log file on the next write.
 */
void
MEMMAN_CleanLogFile( UInt32 clean )
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
 * MEMMAN_BreakOnAllocation():
 *  Trigger a debugger breakpoint on the N-th allocation.
 */
void
MEMMAN_BreakOnAllocation( int alloccount )
{
    CMemoryTracker* tracker = CMemoryTracker::Instance();
    if ( GUCEF_NULL == tracker )
        return;

    tracker->SetBreakOnAllocationCount( alloccount );
}

/*-------------------------------------------------------------------------*/

/**
 * MEMMAN_BreakOnDeallocation():
 *  Trigger a debugger breakpoint when the specified address is deallocated.
 */
void
MEMMAN_BreakOnDeallocation( void* address )
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
 * MEMMAN_BreakOnReallocation():
 *  Trigger a debugger breakpoint when the specified address is reallocated.
 */
void
MEMMAN_BreakOnReallocation( void* address )
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
MEMMAN_ValidateKnownAllocPtr( const void* address ,
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
MEMMAN_ValidateKnownAllocBlock( const void* address  ,
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
MEMMAN_ValidateChunk( const void* address  ,
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
MEMMAN_ValidateAccessibility( const void* address  ,
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
MEMMAN_ValidatePendingDestructor( const char* file    ,
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
MEMMAN_ValidateFinishedDestructor( const char* file    ,
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

#ifdef MEMCHECK_OLEAPI

/**
 * MEMMAN_SysAllocString():
 *  Memory-tracking replacement for OLE SysAllocString().
 *  Allocates [4-byte length prefix][wchar_t data] and returns pointer to the wchar_t data.
 */
wchar_t*
MEMMAN_SysAllocString( const char*    file ,
                       int            line ,
                       const wchar_t* str  )
{
    if ( GUCEF_NULL == str )
        return GUCEF_NULL;

    int i = 0;
    while ( str[ i ] != 0 )
        ++i;

    char* buffer = (char*) MEMMAN_AllocateMemory( file, line, 4 + (i * sizeof(wchar_t)), MM_OLE_ALLOC, GUCEF_NULL, GUCEF_NULL );
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
 * MEMMAN_SysAllocStringByteLen():
 *  Memory-tracking replacement for OLE SysAllocStringByteLen().
 */
wchar_t*
MEMMAN_SysAllocStringByteLen( const char*  file       ,
                              int          line       ,
                              const char*  str        ,
                              unsigned int bufferSize )
{
    char* buffer = (char*) MEMMAN_AllocateMemory( file, line, (size_t)4 + bufferSize, MM_OLE_ALLOC, GUCEF_NULL, GUCEF_NULL );
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
 * MEMMAN_SysAllocStringLen():
 *  Memory-tracking replacement for OLE SysAllocStringLen().
 */
wchar_t*
MEMMAN_SysAllocStringLen( const char*    file        ,
                          int            line        ,
                          const wchar_t* str         ,
                          unsigned int   charsToCopy )
{
    int bufferSize = (charsToCopy + 1) * 2;
    char* buffer = (char*) MEMMAN_AllocateMemory( file, line, 4 + bufferSize, MM_OLE_ALLOC, GUCEF_NULL, GUCEF_NULL );
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
 * MEMMAN_SysFreeString():
 *  Memory-tracking replacement for OLE SysFreeString().
 */
void
MEMMAN_SysFreeString( const char* file      ,
                      int         line      ,
                      wchar_t*    bstrString )
{
    if ( GUCEF_NULL == bstrString )
        return;

    char* buffer = ((char*) bstrString) - 4;
    MEMMAN_DeAllocateMemoryEx( file, line, buffer, MM_OLE_FREE, GUCEF_NULL );
}

/*-------------------------------------------------------------------------*/

/**
 * MEMMAN_SysReAllocString():
 *  Memory-tracking replacement for OLE SysReAllocString().
 */
void
MEMMAN_SysReAllocString( const char*    file  ,
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
    buffer = (char*) MEMMAN_AllocateMemory( file, line, 4 + pszByteSize, MM_OLE_ALLOC, buffer, GUCEF_NULL );
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
 * MEMMAN_SysReAllocStringLen():
 *  Memory-tracking replacement for OLE SysReAllocStringLen().
 */
void
MEMMAN_SysReAllocStringLen( const char*    file  ,
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
    buffer = (char*) MEMMAN_AllocateMemory( file, line, 4 + copyByteSize + (unsigned int) sizeof(wchar_t), MM_OLE_ALLOC, buffer, GUCEF_NULL );
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

}; /* namespace MLF */
}; /* namespace GUCEF */

/*--------------------------------------------------------------------------*/
