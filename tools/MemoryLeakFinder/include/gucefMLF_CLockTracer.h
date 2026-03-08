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

#ifndef GUCEF_MLF_CLOCKTRACER_H
#define GUCEF_MLF_CLOCKTRACER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <map>
#include <stdio.h>
#include <stdlib.h>   /* size_t */

#ifndef GUCEF_MLF_ETYPES_H
#include "gucefMLF_ETypes.h"
#define GUCEF_MLF_ETYPES_H
#endif /* GUCEF_MLF_ETYPES_H ? */

#ifndef GUCEF_CALLSTACK_H
#include "gucefMLF_callstack.h"
#define GUCEF_CALLSTACK_H
#endif /* GUCEF_CALLSTACK_H ? */

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_MT_CREADWRITELOCK_H
#include "gucefMT_CReadWriteLock.h"
#define GUCEF_MT_CREADWRITELOCK_H
#endif /* GUCEF_MT_CREADWRITELOCK_H ? */

#ifndef GUCEF_MT_CSCOPERWLOCK_H
#include "gucefMT_CScopeRwLock.h"
#define GUCEF_MT_CSCOPERWLOCK_H
#endif /* GUCEF_MT_CSCOPERWLOCK_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MLF {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 * Encapsulates the lock trace subsystem.
 * Replaces the anonymous LockInventory class from locktrace.cpp.
 *
 * Tracks exclusive locks through their lifecycle: creation → obtain → release →
 * destroy, and records callstacks at each transition for deadlock diagnostics.
 *
 * An optional periodic snapshot thread writes inventory snapshots to disk at
 * fixed intervals (controlled by the enableSnapshotThread flag in Initialize).
 */
class GUCEF_HIDDEN CLockTracer
{
    public:

    static CLockTracer* Instance( void );
    static void         Deinstance( void );

    /* ------------------------------------------------------------------ */
    /* Aggregate statistics (for surfacing in DumpLogReport)               */
    /* ------------------------------------------------------------------ */

    struct SLockAggregateStats
    {
        UInt64 totalLockInstances;    /* total distinct lock IDs ever registered */
        UInt64 totalAbandonments;     /* sum of abandonmentCounter across all locks */
        UInt64 totalSurplusReleases;  /* sum of surplusLockReleases across all locks */
        UInt64 currentlyLockedCount;  /* locks still in a locked state at query time */
    };

    void GetAggregateStats( SLockAggregateStats& stats );

    /* ------------------------------------------------------------------ */
    /* Lock range protection (Gap 5B)                                      */
    /*                                                                     */
    /* Associate a lock with the memory range it is intended to protect.   */
    /* At deallocation time, if the allocation overlaps a declared range   */
    /* and the associated lock is not currently held, a warning is logged. */
    /* This is contract validation, not race detection.                    */
    /* ------------------------------------------------------------------ */

    struct SLockRangeAssoc
    {
        const void* rangeAddress;   /* start of protected range */
        size_t      rangeSize;      /* length of protected range in bytes */
    };

    typedef std::map< void*, SLockRangeAssoc > TLockRangeMap;

    /** Declare that lockId is intended to protect [address, address+size). */
    void LockProtectsRange( void* lockId, const void* address, size_t size );

    /** Remove any range association for lockId. */
    void LockUnprotectsRange( void* lockId );

    /**
     * Check whether the given address range overlaps any declared protected range
     * whose associated lock is currently unheld.
     * @param outLockId  receives the lock ID of the first matching unheld lock (if any)
     * @return true if an overlap with an unheld protecting lock was found
     */
    bool IsRangeProtectedByUnheldLock( const void* address, size_t size, void** outLockId );

    /* ------------------------------------------------------------------ */

    void RegisterExclusiveLockCreation( void* lockId );
    void RegisterExclusiveLockObtained( void* lockId );
    void RegisterExclusiveLockReleased( void* lockId );
    void RegisterExclusiveLockAbandonment( void* lockId );
    void RegisterExclusiveLockDestruction( void* lockId );

    void PrintLockStacks( void* lockId );
    void PrintAllLockStacks( void );

    private:

    CLockTracer( void );
    ~CLockTracer( void );
    CLockTracer( const CLockTracer& );
    CLockTracer& operator=( const CLockTracer& );

    /* ------------------------------------------------------------------ */
    /* Per-lock trace record                                                */
    /* ------------------------------------------------------------------ */
    class GUCEF_HIDDEN LockTraceInfo
    {
        public:

        TCallStack* m_callstackAtLockCreation;
        TCallStack* m_callstackAtLockObtainment;
        TCallStack* m_callstackAtLockRelease;
        UInt32      m_threadIdAtLockCreation;
        UInt32      m_lastCallerThreadIdAtLockObtainment;
        UInt32      m_lastCallerThreadIdAtLockRelease;
        Int32       m_lockReentrancyDepth;
        UInt32      m_abandonmentCounter;
        TCallStack* m_lastAbandonedCallstackAtLockObtainment;
        UInt32      m_lastAbandonedCallerThreadIdAtLockObtainment;
        bool        m_isLocked;
        bool        m_isExclusivelyLocked;
        UInt32      m_surplusLockReleases;
        TCallStack* m_callstackAtLastSurplusLockRelease;

        LockTraceInfo( void );
        ~LockTraceInfo( void );
    };

    typedef std::map< void*, LockTraceInfo > TLockIdToLockTraceInfoMap;

    LockTraceInfo* GetLockTraceInfo( MT::CScopeReaderLock& readLock, void* lockId );
    void PrintLockStacks( void* lockId, LockTraceInfo* lockTrace, FILE* dest );

    static Int32 GUCEF_CALLSPEC_STD_PREFIX SnapshotThreadMain( void* thisObject ) GUCEF_CALLSPEC_STD_SUFFIX;

    TLockIdToLockTraceInfoMap   m_inventory;
    TLockRangeMap               m_lockRanges;
    MT::CReadWriteLock          m_datalock;
    struct MT::SThreadData*     m_snapshotThread;

    static bool         g_shutdownRequested;
    static MT::CMutex   g_instanceLock;
    static CLockTracer* g_instance;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MLF */
}; /* namespace GUCEF */

/*--------------------------------------------------------------------------*/

#endif /* GUCEF_MLF_CLOCKTRACER_H ? */
