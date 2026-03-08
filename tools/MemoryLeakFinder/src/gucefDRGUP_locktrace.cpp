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
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <assert.h>
#include <stdio.h>
#include <map>

#ifndef GUCEF_DYNNEWOFF_H
#include "gucef_dynnewoff.h"
#define GUCEF_DYNNEWOFF_H
#endif /* GUCEF_DYNNEWOFF_H ? */

#undef GUCEF_USE_CALLSTACK_TRACING
#undef GUCEF_USE_PLATFORM_CALLSTACK_TRACING
#undef GUCEF_USE_PLATFORM_LOCK_TRACER

#include "gucefDRGUP_locktrace.h"
#include "gucefDRGUP_CLockTracer.h"
#include "gucefDRGUP_callstack.h"

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_MT_CREADWRITELOCK_H
#include "gucefMT_CReadWriteLock.h"
#define GUCEF_MT_CREADWRITELOCK_H
#endif /* GUCEF_MT_CREADWRITELOCK_H ? */

#ifndef GUCEF_MT_CSCOPERWLOCK_H
#include "gucefMT_CScopeRwLock.h"
#define GUCEF_MT_CSCOPERWLOCK_H
#endif /* GUCEF_MT_CSCOPERWLOCK_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifdef GUCEF_MSWIN_BUILD
  #define EOL "\r\n"
#else
  #define EOL "\n"
#endif

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRGUP {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

bool        CLockTracer::g_shutdownRequested = false;
MT::CMutex  CLockTracer::g_instanceLock;
CLockTracer* CLockTracer::g_instance = GUCEF_NULL;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION: CLockTracer                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

/* static */
CLockTracer*
CLockTracer::Instance( void )
{
    if ( GUCEF_NULL == g_instance )
    {
        MT::CScopeMutex lock( g_instanceLock );
        if ( GUCEF_NULL == g_instance )
        {
            g_shutdownRequested = false;
            g_instance = new CLockTracer();
        }
    }
    return g_instance;
}

/*-------------------------------------------------------------------------*/

/* static */
void
CLockTracer::Deinstance( void )
{
    FILE* dbg = fopen( "memman_shutdown_debug.txt", "a" );
    if ( dbg ) { fprintf( dbg, "  LockTracer::Deinstance enter, g_instance=%p\n", (void*)g_instance ); fflush( dbg ); }

    MT::CScopeMutex lock( g_instanceLock );
    if ( dbg ) { fprintf( dbg, "  LockTracer::Deinstance: got g_instanceLock\n" ); fflush( dbg ); }

    if ( GUCEF_NULL != g_instance )
    {
        g_shutdownRequested = true;
        if ( dbg ) { fprintf( dbg, "  LockTracer::Deinstance: shutdown requested, threadPtr=%p\n", (void*)g_instance->m_snapshotThread ); fflush( dbg ); }
        /* Wait for the snapshot thread to exit before destroying the object.
           The thread delay is 10 seconds so a 15-second timeout is sufficient. */
        if ( GUCEF_NULL != g_instance->m_snapshotThread )
        {
            UInt32 waitResult = MT::ThreadWait( g_instance->m_snapshotThread, 15000 );
            if ( dbg ) { fprintf( dbg, "  LockTracer::Deinstance: ThreadWait returned %u\n", waitResult ); fflush( dbg ); }
        }
        if ( dbg ) { fprintf( dbg, "  LockTracer::Deinstance: about to delete g_instance\n" ); fflush( dbg ); }
        delete g_instance;
        if ( dbg ) { fprintf( dbg, "  LockTracer::Deinstance: delete done\n" ); fflush( dbg ); }
        g_instance = GUCEF_NULL;
    }

    if ( dbg ) { fprintf( dbg, "  LockTracer::Deinstance exit\n" ); fclose( dbg ); }
}

/*-------------------------------------------------------------------------*/

CLockTracer::CLockTracer( void )
    : m_inventory()
    , m_datalock( true )
    , m_snapshotThread( GUCEF_NULL )
{
    m_snapshotThread = MT::ThreadDataReserve();
    MT::ThreadCreate( m_snapshotThread, (MT::TThreadFunc) SnapshotThreadMain, this );
}

/*-------------------------------------------------------------------------*/

CLockTracer::~CLockTracer( void )
{
    FILE* dbg = fopen( "memman_shutdown_debug.txt", "a" );
    if ( dbg ) { fprintf( dbg, "  ~CLockTracer enter, inventorySize=%d\n", (int)m_inventory.size() ); fflush( dbg ); }
    {
        MT::CScopeWriterLock writeLock( m_datalock );
        if ( dbg ) { fprintf( dbg, "  ~CLockTracer: got write lock\n" ); fflush( dbg ); }
        m_inventory.clear();
        if ( dbg ) { fprintf( dbg, "  ~CLockTracer: inventory cleared\n" ); fflush( dbg ); }
    }
    if ( dbg ) { fprintf( dbg, "  ~CLockTracer: about to ThreadDataCleanup\n" ); fflush( dbg ); }
    MT::ThreadDataCleanup( m_snapshotThread );
    if ( dbg ) { fprintf( dbg, "  ~CLockTracer: ThreadDataCleanup done\n" ); fflush( dbg ); }
    m_snapshotThread = GUCEF_NULL;
    if ( dbg ) { fprintf( dbg, "  ~CLockTracer exit\n" ); fclose( dbg ); }
}

/*-------------------------------------------------------------------------*/

/* static */
Int32 GUCEF_CALLSPEC_STD_PREFIX
CLockTracer::SnapshotThreadMain( void* thisVoidObject ) GUCEF_CALLSPEC_STD_SUFFIX
{
    CLockTracer* thisObj = static_cast< CLockTracer* >( thisVoidObject );
    while ( !g_shutdownRequested )
    {
        /* Sleep in 100ms slices so we react to shutdown within 100ms */
        for ( UInt32 i = 0; i < 100 && !g_shutdownRequested; ++i )
        {
            MT::PrecisionDelay( 100 );
        }
        if ( !g_shutdownRequested )
        {
            thisObj->PrintAllLockStacks();
        }
    }
    return 0;
}

/*-------------------------------------------------------------------------*/

CLockTracer::LockTraceInfo*
CLockTracer::GetLockTraceInfo( MT::CScopeReaderLock& readLock, void* lockId )
{
    TLockIdToLockTraceInfoMap::iterator i = m_inventory.find( lockId );
    if ( i != m_inventory.end() )
        return &(*i).second;

    /* First time — escalate to write lock */
    MT::CScopeWriterLock writeLock( readLock );
    LockTraceInfo& newLockTrace = m_inventory[ lockId ];
    writeLock.TransitionToReader( readLock );
    return &newLockTrace;
}

/*-------------------------------------------------------------------------*/

void
CLockTracer::RegisterExclusiveLockCreation( void* lockId )
{
    MT::CScopeReaderLock readLock( m_datalock );

    UInt32 callerThreadId = MT::GetCurrentTaskID();
    LockTraceInfo* lockTrace = GetLockTraceInfo( readLock, lockId );

    assert( GUCEF_NULL != lockTrace );
    assert( GUCEF_NULL == lockTrace->m_callstackAtLockCreation );
    assert( 0 == lockTrace->m_threadIdAtLockCreation );
    assert( 0 == lockTrace->m_lockReentrancyDepth );
    assert( !lockTrace->m_isLocked );

    DRGUP_FreeCallstackCopy( lockTrace->m_callstackAtLockCreation );
    DRGUP_GetCallstackCopyForCurrentThread( &lockTrace->m_callstackAtLockCreation, 0 );
    lockTrace->m_threadIdAtLockCreation = callerThreadId;
    lockTrace->m_isLocked = false;
    lockTrace->m_isExclusivelyLocked = false;
}

/*-------------------------------------------------------------------------*/

void
CLockTracer::RegisterExclusiveLockObtained( void* lockId )
{
    MT::CScopeReaderLock readLock( m_datalock );

    UInt32 callerThreadId = MT::GetCurrentTaskID();
    LockTraceInfo* lockTrace = GetLockTraceInfo( readLock, lockId );
    assert( GUCEF_NULL != lockTrace );

    if ( lockTrace->m_isLocked )
    {
        if ( callerThreadId == lockTrace->m_lastCallerThreadIdAtLockObtainment )
        {
            ++lockTrace->m_lockReentrancyDepth;
        }
        else
        {
            /* Unexpected: exclusive lock obtained by multiple threads simultaneously */
            MT::CScopeWriterLock writeLock( readLock );
            DRGUP_PrintCallstack();
            PrintLockStacks( lockId, lockTrace, stdout );
            GUCEF_UNREACHABLE;
        }
    }

    DRGUP_FreeCallstackCopy( lockTrace->m_callstackAtLockObtainment );
    DRGUP_GetCallstackCopyForCurrentThread( &lockTrace->m_callstackAtLockObtainment, 0 );
    lockTrace->m_lastCallerThreadIdAtLockObtainment = callerThreadId;
    lockTrace->m_isLocked = true;
    lockTrace->m_isExclusivelyLocked = true;
}

/*-------------------------------------------------------------------------*/

void
CLockTracer::RegisterExclusiveLockAbandonment( void* lockId )
{
    MT::CScopeReaderLock readLock( m_datalock );

    UInt32 callerThreadId = MT::GetCurrentTaskID();
    LockTraceInfo* lockTrace = GetLockTraceInfo( readLock, lockId );
    assert( GUCEF_NULL != lockTrace );

    if ( lockTrace->m_isLocked )
    {
        if ( callerThreadId != lockTrace->m_lastCallerThreadIdAtLockObtainment )
        {
            ++lockTrace->m_abandonmentCounter;
            DRGUP_FreeCallstackCopy( lockTrace->m_lastAbandonedCallstackAtLockObtainment );
            lockTrace->m_lastAbandonedCallstackAtLockObtainment = lockTrace->m_callstackAtLockObtainment;
            lockTrace->m_lastAbandonedCallerThreadIdAtLockObtainment = lockTrace->m_lastCallerThreadIdAtLockObtainment;
            lockTrace->m_callstackAtLockObtainment = GUCEF_NULL;
            lockTrace->m_lockReentrancyDepth = 0;
        }
        else
        {
            /* Cannot abandon a lock we currently hold */
            MT::CScopeWriterLock writeLock( readLock );
            DRGUP_PrintCallstack();
            PrintLockStacks( lockId, lockTrace, stdout );
            GUCEF_UNREACHABLE;
        }
    }

    DRGUP_FreeCallstackCopy( lockTrace->m_callstackAtLockObtainment );
    DRGUP_GetCallstackCopyForCurrentThread( &lockTrace->m_callstackAtLockObtainment, 0 );
    lockTrace->m_lastCallerThreadIdAtLockObtainment = callerThreadId;
    lockTrace->m_isLocked = true;
    lockTrace->m_isExclusivelyLocked = true;
}

/*-------------------------------------------------------------------------*/

void
CLockTracer::RegisterExclusiveLockReleased( void* lockId )
{
    MT::CScopeReaderLock readLock( m_datalock );

    UInt32 callerThreadId = MT::GetCurrentTaskID();
    LockTraceInfo* lockTrace = GetLockTraceInfo( readLock, lockId );
    assert( GUCEF_NULL != lockTrace );

    if ( !lockTrace->m_isLocked || !lockTrace->m_isExclusivelyLocked )
    {
        /* Surplus release — lock not currently held */
        ++lockTrace->m_surplusLockReleases;
        DRGUP_FreeCallstackCopy( lockTrace->m_callstackAtLastSurplusLockRelease );
        DRGUP_GetCallstackCopyForCurrentThread( &lockTrace->m_callstackAtLastSurplusLockRelease, 0 );

        MT::CScopeWriterLock writeLock( readLock );
        DRGUP_PrintCallstack();
        PrintLockStacks( lockId, lockTrace, stdout );
        GUCEF_UNREACHABLE;
    }

    if ( callerThreadId == lockTrace->m_lastCallerThreadIdAtLockObtainment )
    {
        lockTrace->m_lastCallerThreadIdAtLockRelease = callerThreadId;

        if ( lockTrace->m_lockReentrancyDepth > 0 )
            --lockTrace->m_lockReentrancyDepth;
        else
        {
            lockTrace->m_isLocked = false;
            lockTrace->m_isExclusivelyLocked = false;
        }

        DRGUP_FreeCallstackCopy( lockTrace->m_callstackAtLockRelease );
        DRGUP_GetCallstackCopyForCurrentThread( &lockTrace->m_callstackAtLockRelease, 0 );
    }
    else
    {
        /* Thread releasing the lock is not the thread that obtained it */
        MT::CScopeWriterLock writeLock( readLock );
        DRGUP_PrintCallstack();
        PrintLockStacks( lockId, lockTrace, stdout );
        GUCEF_UNREACHABLE;
    }
}

/*-------------------------------------------------------------------------*/

void
CLockTracer::RegisterExclusiveLockDestruction( void* lockId )
{
    MT::CScopeReaderLock readLock( m_datalock );

    UInt32 callerThreadId = MT::GetCurrentTaskID();
    LockTraceInfo* lockTrace = GetLockTraceInfo( readLock, lockId );
    assert( GUCEF_NULL != lockTrace );

    if ( lockTrace->m_isLocked && callerThreadId != lockTrace->m_lastCallerThreadIdAtLockObtainment )
    {
        /* Lock still held by another thread while we destroy it */
        MT::CScopeWriterLock writeLock( readLock );
        DRGUP_PrintCallstack();
        PrintLockStacks( lockId, lockTrace, stdout );
        GUCEF_UNREACHABLE;
    }

    MT::CScopeWriterLock writeLock( readLock );
    m_inventory.erase( lockId );
}

/*-------------------------------------------------------------------------*/

void
CLockTracer::PrintLockStacks( void* lockId )
{
    MT::CScopeReaderLock readLock( m_datalock );
    LockTraceInfo* lockTrace = GetLockTraceInfo( readLock, lockId );
    PrintLockStacks( lockId, lockTrace, stdout );
}

/*-------------------------------------------------------------------------*/

void
CLockTracer::PrintLockStacks( void* lockId, LockTraceInfo* lockTrace, FILE* dest )
{
    if ( GUCEF_NULL == lockTrace || GUCEF_NULL == dest )
        return;

    fprintf( dest, "------------------------------%s", EOL );
    fprintf( dest, "Trace for lock %p:%s%s", lockId, EOL, EOL );
    fprintf( dest, "   threadIdAtLockCreation: %d%s", lockTrace->m_threadIdAtLockCreation, EOL );
    fprintf( dest, "   isExclusivelyLocked: %d%s", lockTrace->m_isExclusivelyLocked ? 1 : 0, EOL );
    fprintf( dest, "   isLocked: %d%s", lockTrace->m_isLocked ? 1 : 0, EOL );
    fprintf( dest, "   lastCallerThreadIdAtLockObtainment: %d%s", lockTrace->m_lastCallerThreadIdAtLockObtainment, EOL );
    fprintf( dest, "   lastCallerThreadIdAtLockRelease: %d%s", lockTrace->m_lastCallerThreadIdAtLockRelease, EOL );
    fprintf( dest, "   lockReentrancyDepth: %d%s", lockTrace->m_lockReentrancyDepth, EOL );
    fprintf( dest, "   surplusLockReleases: %d%s", lockTrace->m_surplusLockReleases, EOL );
    fprintf( dest, "   abandonmentCounter: %d%s", lockTrace->m_abandonmentCounter, EOL );
    fprintf( dest, "   lastAbandonedCallerThreadIdAtLockObtainment: %d%s", lockTrace->m_lastAbandonedCallerThreadIdAtLockObtainment, EOL );
    fprintf( dest, "------------------------------%s%s", EOL, EOL );
    fprintf( dest, "Stack at lock creation: %s%s", EOL, EOL );
    DRGUP_PrintCallstackCopyTo( lockTrace->m_callstackAtLockCreation, dest );
    fprintf( dest, "------------------------------%s%s", EOL, EOL );
    fprintf( dest, "Stack at lock obtainment: %s%s", EOL, EOL );
    fprintf( dest, "------------------------%s%s", EOL, EOL );
    DRGUP_PrintCallstackCopyTo( lockTrace->m_callstackAtLockObtainment, dest );
    fprintf( dest, "Stack at lock release: %s%s", EOL, EOL );
    fprintf( dest, "------------------------%s%s", EOL, EOL );
    DRGUP_PrintCallstackCopyTo( lockTrace->m_callstackAtLockRelease, dest );
    fprintf( dest, "------------------------------%s%s", EOL, EOL );
    if ( lockTrace->m_abandonmentCounter > 0 )
    {
        fprintf( dest, "Stack at lock obtainment from last lock abandonment: %s%s", EOL, EOL );
        fprintf( dest, "------------------------%s%s", EOL, EOL );
        DRGUP_PrintCallstackCopyTo( lockTrace->m_lastAbandonedCallstackAtLockObtainment, dest );
    }
    if ( lockTrace->m_surplusLockReleases > 0 )
    {
        fprintf( dest, "Stack at last surplus lock release call: %s%s", EOL, EOL );
        fprintf( dest, "------------------------%s%s", EOL, EOL );
        DRGUP_PrintCallstackCopyTo( lockTrace->m_callstackAtLastSurplusLockRelease, dest );
    }
}

/*-------------------------------------------------------------------------*/

void
CLockTracer::PrintAllLockStacks( void )
{
    long long int tickCount = (long long int) MT::PrecisionTickCount();
    char snapshotFilename[ 128 ];
    sprintf( snapshotFilename, "LockInventorySnapshot_%lld.txt", tickCount );

    FILE* outFile = fopen( snapshotFilename, "w" );
    if ( GUCEF_NULL != outFile )
    {
        MT::CScopeWriterLock writeLock( m_datalock );
        TLockIdToLockTraceInfoMap::iterator i = m_inventory.begin();
        while ( i != m_inventory.end() )
        {
            PrintLockStacks( (*i).first, &(*i).second, outFile );
            ++i;
        }
        fclose( outFile );
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION: CLockTracer::LockTraceInfo                         //
//                                                                         //
//-------------------------------------------------------------------------*/

CLockTracer::LockTraceInfo::LockTraceInfo( void )
    : m_callstackAtLockCreation( GUCEF_NULL )
    , m_callstackAtLockObtainment( GUCEF_NULL )
    , m_callstackAtLockRelease( GUCEF_NULL )
    , m_threadIdAtLockCreation( 0 )
    , m_lastCallerThreadIdAtLockObtainment( 0 )
    , m_lastCallerThreadIdAtLockRelease( 0 )
    , m_lockReentrancyDepth( 0 )
    , m_abandonmentCounter( 0 )
    , m_lastAbandonedCallstackAtLockObtainment( GUCEF_NULL )
    , m_lastAbandonedCallerThreadIdAtLockObtainment( 0 )
    , m_isLocked( false )
    , m_isExclusivelyLocked( false )
    , m_surplusLockReleases( 0 )
    , m_callstackAtLastSurplusLockRelease( GUCEF_NULL )
{
}

/*-------------------------------------------------------------------------*/

CLockTracer::LockTraceInfo::~LockTraceInfo( void )
{
    DRGUP_FreeCallstackCopy( m_callstackAtLockCreation );
    m_callstackAtLockCreation = GUCEF_NULL;
    DRGUP_FreeCallstackCopy( m_callstackAtLockObtainment );
    m_callstackAtLockObtainment = GUCEF_NULL;
    DRGUP_FreeCallstackCopy( m_callstackAtLockRelease );
    m_callstackAtLockRelease = GUCEF_NULL;
    DRGUP_FreeCallstackCopy( m_lastAbandonedCallstackAtLockObtainment );
    m_lastAbandonedCallstackAtLockObtainment = GUCEF_NULL;
    DRGUP_FreeCallstackCopy( m_callstackAtLastSurplusLockRelease );
    m_callstackAtLastSurplusLockRelease = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

void
CLockTracer::LockProtectsRange( void* lockId, const void* address, size_t size )
{
    if ( GUCEF_NULL == lockId || GUCEF_NULL == address || 0 == size )
        return;

    MT::CScopeWriterLock writeLock( m_datalock );

    SLockRangeAssoc& assoc = m_lockRanges[ lockId ];
    assoc.rangeAddress = address;
    assoc.rangeSize    = size;
}

/*-------------------------------------------------------------------------*/

void
CLockTracer::LockUnprotectsRange( void* lockId )
{
    if ( GUCEF_NULL == lockId )
        return;

    MT::CScopeWriterLock writeLock( m_datalock );

    m_lockRanges.erase( lockId );
}

/*-------------------------------------------------------------------------*/

bool
CLockTracer::IsRangeProtectedByUnheldLock( const void* address, size_t size, void** outLockId )
{
    if ( GUCEF_NULL == address || 0 == size )
        return false;

    MT::CScopeReaderLock readLock( m_datalock );

    if ( m_lockRanges.empty() )
        return false;

    const char* checkStart = static_cast< const char* >( address );
    const char* checkEnd   = checkStart + size;

    for ( TLockRangeMap::const_iterator it = m_lockRanges.begin(); it != m_lockRanges.end(); ++it )
    {
        void*       lockId     = it->first;
        const char* rangeStart = static_cast< const char* >( it->second.rangeAddress );
        const char* rangeEnd   = rangeStart + it->second.rangeSize;

        /* Check for range overlap */
        if ( checkStart < rangeEnd && checkEnd > rangeStart )
        {
            /* Overlapping range found — check whether the associated lock is unheld */
            TLockIdToLockTraceInfoMap::const_iterator li = m_inventory.find( lockId );
            if ( li != m_inventory.end() && !li->second.m_isLocked )
            {
                if ( GUCEF_NULL != outLockId )
                    *outLockId = lockId;
                return true;
            }
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

void
CLockTracer::GetAggregateStats( SLockAggregateStats& stats )
{
    MT::CScopeReaderLock readLock( m_datalock );

    stats.totalLockInstances   = (UInt64) m_inventory.size();
    stats.totalAbandonments    = 0;
    stats.totalSurplusReleases = 0;
    stats.currentlyLockedCount = 0;

    TLockIdToLockTraceInfoMap::const_iterator i = m_inventory.begin();
    while ( i != m_inventory.end() )
    {
        const LockTraceInfo& info = i->second;
        stats.totalAbandonments    += info.m_abandonmentCounter;
        stats.totalSurplusReleases += info.m_surplusLockReleases;
        if ( info.m_isLocked )
            ++stats.currentlyLockedCount;
        ++i;
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      C API — 1-line delegations to CLockTracer::Instance()              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
DRGUP_ExclusiveLockCreated( void* lockId )
{
    CLockTracer::Instance()->RegisterExclusiveLockCreation( lockId );
}

/*-------------------------------------------------------------------------*/

void
DRGUP_ExclusiveLockObtained( void* lockId )
{
    CLockTracer::Instance()->RegisterExclusiveLockObtained( lockId );
}

/*-------------------------------------------------------------------------*/

void
DRGUP_ExclusiveLockReleased( void* lockId )
{
    CLockTracer::Instance()->RegisterExclusiveLockReleased( lockId );
}

/*-------------------------------------------------------------------------*/

void
DRGUP_ExclusiveLockAbandoned( void* lockId )
{
    CLockTracer::Instance()->RegisterExclusiveLockAbandonment( lockId );
}

/*-------------------------------------------------------------------------*/

void
DRGUP_ExclusiveLockDestroy( void* lockId )
{
    CLockTracer::Instance()->RegisterExclusiveLockDestruction( lockId );
}

/*-------------------------------------------------------------------------*/

void
DRGUP_LockTraceInit( void )
{
    CLockTracer::Instance();
}

/*-------------------------------------------------------------------------*/

void
DRGUP_LockTraceShutdown( void )
{
    CLockTracer::Deinstance();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRGUP */
}; /* namespace GUCEF */

/*--------------------------------------------------------------------------*/
