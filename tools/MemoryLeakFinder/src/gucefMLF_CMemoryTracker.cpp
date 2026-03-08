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

#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <time.h>

/* Turn off tracking macros for this file */
#ifndef GUCEF_DYNNEWOFF_H
#include "gucef_dynnewoff.h"
#define GUCEF_DYNNEWOFF_H
#endif /* GUCEF_DYNNEWOFF_H ? */

#undef GUCEF_USE_CALLSTACK_TRACING
#undef GUCEF_USE_CALLSTACK_PLATFORM_TRACING

#include "gucefMLF_CMemoryTracker.h"
#include "gucefMLF_callstack.h"

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

#ifndef GUCEF_MT_CSCOPERWLOCK_H
#include "gucefMT_CScopeRwLock.h"
#define GUCEF_MT_CSCOPERWLOCK_H
#endif /* GUCEF_MT_CSCOPERWLOCK_H ? */

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
  #define PSAPI_VERSION 2
  #include <eh.h>
  #include <Psapi.h>
  #include <debugapi.h>
  #include <sstream>
  #include <string>
#elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )
  #include <signal.h>
  #include <time.h>
  #include <sys/mman.h>   /* mmap / munmap / mprotect — for guard page mode */
  #include <unistd.h>     /* sysconf / _SC_PAGESIZE */
#endif

#ifndef GUCEF_MLF_CLOCKTRACER_H
#include "gucefMLF_CLockTracer.h"
#define GUCEF_MLF_CLOCKTRACER_H
#endif /* GUCEF_MLF_CLOCKTRACER_H ? */

#ifndef GUCEF_MLF_CCALLSITESTATS_H
#include "gucefMLF_CCallsiteStats.h"
#define GUCEF_MLF_CCALLSITESTATS_H
#endif /* GUCEF_MLF_CCALLSITESTATS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MLF {

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
  #define GUCEF_SETBREAKPOINT { DebugBreak(); }
#elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )
  #define GUCEF_SETBREAKPOINT { raise(SIGTRAP); }
#else
  #define GUCEF_SETBREAKPOINT
#endif

/* Platform-specific line deletion */
#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
  #define GUCEF_UNLINK _unlink
#else
  #include <unistd.h>
  #define GUCEF_UNLINK unlink
#endif

static const char* const s_allocationTypes[] =
{
    "Unknown", "new", "new(addr)", "new[]",
    "malloc", "calloc", "realloc",
    "delete", "delete[]", "free",
    "OLESysAlloc", "OLESysFree"
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      THREAD-LOCAL OWNER CONTEXT                                         //
//                                                                         //
//      We use a simple TLS approach via a thread-local pointer to the     //
//      top-of-stack SOwnerContextNode.  On MSVC we use __declspec,        //
//      on GCC/Clang we use __thread.                                      //
//                                                                         //
//-------------------------------------------------------------------------*/

#if defined( _MSC_VER )
  static __declspec(thread) SOwnerContextNode* tl_ownerStack = NULL;
  static __declspec(thread) bool               tl_inTracker  = false;
#elif defined( __GNUC__ )
  static __thread SOwnerContextNode* tl_ownerStack = NULL;
  static __thread bool               tl_inTracker  = false;
#else
  /* Fallback: no TLS — use global (not ideal, but better than crashing) */
  static SOwnerContextNode* tl_ownerStack = NULL;
  static bool               tl_inTracker  = false;
#endif

/*-------------------------------------------------------------------------//
//                                                                         //
//      WINDOWS VECTORED EXCEPTION HANDLER                                 //
//                                                                         //
//-------------------------------------------------------------------------*/

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
#if ( _WIN32_WINNT >= 0x0500 )

static LONG WINAPI
Win32VectoredExceptionHandler( struct _EXCEPTION_POINTERS* ExceptionInfo )
{
    switch ( ExceptionInfo->ExceptionRecord->ExceptionCode )
    {
        case EXCEPTION_ACCESS_VIOLATION:
        case EXCEPTION_IN_PAGE_ERROR:
        {
            if ( CMemoryTracker::IsConstructed() && !CMemoryTracker::IsDestructed() )
            {
                CMemoryTracker* tracker = CMemoryTracker::Instance();

                if ( ExceptionInfo->ExceptionRecord->NumberParameters > 1 )
                {
                    ULONG_PTR faultAddr = ExceptionInfo->ExceptionRecord->ExceptionInformation[ 1 ];
                    tracker->Log( "MEMMAN: Vectored exception at fault address 0x%p", (void*)faultAddr );
                    tracker->ValidateAllLiveAllocations();

                    /* Find nearest nodes for context */
                    CMemoryTracker::TUInt64ToRecordMap nearestCurrent, nearestDealloc;
                    tracker->FindNearestCurrentRecords( (const void*)faultAddr, 10, nearestCurrent );
                    tracker->FindNearestDeallocatedRecords( (const void*)faultAddr, 10, nearestDealloc );
                }
            }
            return EXCEPTION_CONTINUE_SEARCH;
        }
        default:
            return EXCEPTION_CONTINUE_SEARCH;
    }
}

#endif
#endif

/*-------------------------------------------------------------------------//
//                                                                         //
//      STATIC MEMBER INITIALIZATION                                       //
//                                                                         //
//-------------------------------------------------------------------------*/

MT::CMutex     CMemoryTracker::g_instanceLock;
bool           CMemoryTracker::g_isConstructed = false;
bool           CMemoryTracker::g_isDestructed  = false;
CMemoryTracker* CMemoryTracker::g_instance     = GUCEF_NULL;

/*-------------------------------------------------------------------------*/

/**
 * Return the platform high-resolution monotonic time in microseconds.
 * Used to populate allocationTimestampUs on each allocation record.
 */
static UInt64
GetCurrentTimestampUs( void )
{
#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    LARGE_INTEGER freq, now;
    if ( QueryPerformanceFrequency( &freq ) && QueryPerformanceCounter( &now ) && freq.QuadPart > 0 )
        return (UInt64)( now.QuadPart * 1000000LL / freq.QuadPart );
    return 0;
#elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )
    struct timespec ts;
    if ( 0 == clock_gettime( CLOCK_MONOTONIC, &ts ) )
        return (UInt64)ts.tv_sec * 1000000ULL + (UInt64)ts.tv_nsec / 1000ULL;
    return 0;
#else
    return 0;
#endif
}

/*-------------------------------------------------------------------------*/

/**
 * Map an allocation size to a histogram bucket index.
 * Buckets: [0]=1-16B, [1]=17-64B, [2]=65-256B, [3]=257-1023B,
 *          [4]=1KB-4095B, [5]=4KB-65535B, [6]=64KB-1MB-1B, [7]=>=1MB
 */
static UInt32
SizeToBucket( size_t size )
{
    if ( size <= 16 )       return 0;
    if ( size <= 64 )       return 1;
    if ( size <= 256 )      return 2;
    if ( size <= 1023 )     return 3;
    if ( size <= 4095 )     return 4;
    if ( size <= 65535 )    return 5;
    if ( size < 1048576 )   return 6;
    return 7;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION: CMemoryTracker singleton                           //
//                                                                         //
//-------------------------------------------------------------------------*/

/* static */
CMemoryTracker*
CMemoryTracker::Instance( void )
{
    if ( GUCEF_NULL == g_instance )
    {
        MT::CScopeMutex lock( g_instanceLock );
        if ( GUCEF_NULL == g_instance )
        {
            g_isDestructed = false; /* allow re-init after shutdown */
            g_instance = new CMemoryTracker();
        }
    }
    return g_instance;
}

/*-------------------------------------------------------------------------*/

/* static */
void
CMemoryTracker::Deinstance( void )
{
    MT::CScopeMutex lock( g_instanceLock );
    if ( GUCEF_NULL != g_instance )
    {
        g_isDestructed = true;
        delete g_instance;
        g_instance = GUCEF_NULL;
    }
}

/*-------------------------------------------------------------------------*/

/* static */
bool
CMemoryTracker::IsConstructed( void )
{
    return g_isConstructed;
}

/*-------------------------------------------------------------------------*/

/* static */
bool
CMemoryTracker::IsDestructed( void )
{
    return g_isDestructed;
}

/*-------------------------------------------------------------------------*/

CMemoryTracker::CMemoryTracker( void )
    : m_config()
    , m_initialized( false )
    , m_shutdownCalled( false )
    , m_logFileBuf()
    , m_registry()
    , m_dataLock( true )
    , m_deallocRingHead( GUCEF_NULL )
    , m_deallocRingTail( GUCEF_NULL )
    , m_deallocRingCount( 0 )
    , m_freePool( GUCEF_NULL )
    , m_poolMutex()
    , m_totalMemoryAllocations( 0 )
    , m_totalMemoryAllocated( 0 )
    , m_totalMemoryUsed( 0 )
    , m_peakMemoryAllocation( 0 )
    , m_peakTotalNumAllocations( 0 )
    , m_overheadMemoryCost( 0 )
    , m_peakOverheadMemoryCost( 0 )
    , m_totalOverheadMemoryCost( 0 )
    , m_allocatedMemory( 0 )
    , m_numBoundsViolations( 0 )
    , m_numAllocations( 0 )
    , m_numSubAllocations( 0 )
    , m_numMismatchedDeallocs( 0 )
    , m_callsiteMap()
    , m_timestampBaseUs( 0 )
#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
  #if ( _WIN32_WINNT >= 0x0500 )
    , m_vectoredExceptionHandler( GUCEF_NULL )
  #endif
#endif
{
    g_isConstructed = true;
    SMemoryTrackerConfig_SetDefaults( m_config );
    strncpy( m_logFileBuf, m_config.logFilePath, sizeof(m_logFileBuf) - 1 );
    m_logFileBuf[ sizeof(m_logFileBuf) - 1 ] = '\0';
    m_config.logFilePath = m_logFileBuf;

    /* Initialize histogram buckets to zero */
    for ( UInt32 b = 0; b < HISTOGRAM_BUCKET_COUNT; ++b )
    {
        m_histogramCounts[ b ] = 0;
        m_histogramBytes[ b ]  = 0;
    }

    Initialize();
}

/*-------------------------------------------------------------------------*/

CMemoryTracker::~CMemoryTracker( void )
{
    if ( !m_shutdownCalled )
        Release();
    g_isConstructed = false;
}

/*-------------------------------------------------------------------------*/

void
CMemoryTracker::Initialize( void )
{
#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
  #if ( _WIN32_WINNT >= 0x0500 )
    m_vectoredExceptionHandler = ::AddVectoredExceptionHandler( 1, Win32VectoredExceptionHandler );
  #endif

    /* Capture timestamp base for per-allocation timestamps */
    LARGE_INTEGER freq, now;
    if ( QueryPerformanceFrequency( &freq ) && QueryPerformanceCounter( &now ) && freq.QuadPart > 0 )
        m_timestampBaseUs = (UInt64)( now.QuadPart * 1000000LL / freq.QuadPart );
    else
        m_timestampBaseUs = 0;

#elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )

    struct timespec ts;
    if ( 0 == clock_gettime( CLOCK_MONOTONIC, &ts ) )
        m_timestampBaseUs = (UInt64)ts.tv_sec * 1000000ULL + (UInt64)ts.tv_nsec / 1000ULL;
    else
        m_timestampBaseUs = 0;

#else
    m_timestampBaseUs = 0;
#endif

    m_initialized = true;
}

/*-------------------------------------------------------------------------*/

void
CMemoryTracker::Release( void )
{
    /* Clean up all live records */
    {
        MT::CScopeWriterLock writeLock( m_dataLock );
        TRegistry::iterator i = m_registry.begin();
        while ( i != m_registry.end() )
        {
            CAllocationRecord* rec = i->second;
            if ( GUCEF_NULL != rec && GUCEF_NULL != rec->actualAddress )
            {
                ::free( rec->actualAddress );
                rec->actualAddress = GUCEF_NULL;
            }
            CAllocationRecord::Release( rec );
            ++i;
        }
        m_registry.clear();

        /* Clean dealloc ring */
        CAllocationRecord* node = m_deallocRingHead;
        while ( GUCEF_NULL != node )
        {
            CAllocationRecord* next = node->hashNext;
            CAllocationRecord::Release( node );
            node = next;
        }
        m_deallocRingHead = GUCEF_NULL;
        m_deallocRingTail = GUCEF_NULL;
        m_deallocRingCount = 0;
    }

    /* Clean free pool */
    {
        MT::CScopeMutex poolLock( m_poolMutex );
        CAllocationRecord* node = m_freePool;
        while ( GUCEF_NULL != node )
        {
            CAllocationRecord* next = node->hashNext;
            CAllocationRecord::Release( node );
            node = next;
        }
        m_freePool = GUCEF_NULL;
    }

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
  #if ( _WIN32_WINNT >= 0x0500 )
    if ( GUCEF_NULL != m_vectoredExceptionHandler )
    {
        ::RemoveVectoredExceptionHandler( (PVOID) m_vectoredExceptionHandler );
        m_vectoredExceptionHandler = GUCEF_NULL;
    }
  #endif
#endif

    m_initialized = false;
    m_shutdownCalled = true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      Configuration                                                      //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CMemoryTracker::ApplyConfig( const SMemoryTrackerConfig& cfg )
{
    MT::CScopeWriterLock writeLock( m_dataLock );
    UInt32 paddingSize            = cfg.paddingSize > 0 ? cfg.paddingSize : 4;
    m_config.paddingSize          = paddingSize;
    m_config.logAlways            = cfg.logAlways;
    m_config.cleanLogFileOnFirstRun = cfg.cleanLogFileOnFirstRun;
    m_config.exhaustiveTesting    = cfg.exhaustiveTesting;
    m_config.breakOnAllocationCount = cfg.breakOnAllocationCount;
    m_config.deallocRingCapacity        = cfg.deallocRingCapacity;
    m_config.enableCallstackCapture     = cfg.enableCallstackCapture;
    m_config.enableRawCallstackCapture  = cfg.enableRawCallstackCapture;
    m_config.maxRawCallstackDepth       = cfg.maxRawCallstackDepth;
    m_config.deallocMismatchResponse    = cfg.deallocMismatchResponse;
    m_config.enableCallsiteProfiling    = cfg.enableCallsiteProfiling;
    m_config.useGuardPages              = cfg.useGuardPages;
    /* When guard pages are active, cap the dealloc ring to avoid address space exhaustion.
     * Each entry consumes at least one page (4 KB) of address space in 32-bit processes. */
    #ifndef GUCEF_MLF_ASAN_ACTIVE
    if ( m_config.useGuardPages && m_config.deallocRingCapacity > 1000 )
        m_config.deallocRingCapacity = 1000;
    #endif
    if ( GUCEF_NULL != cfg.logFilePath )
    {
        strncpy( m_logFileBuf, cfg.logFilePath, sizeof(m_logFileBuf) - 1 );
        m_logFileBuf[ sizeof(m_logFileBuf) - 1 ] = '\0';
    }
}

/*-------------------------------------------------------------------------*/

const SMemoryTrackerConfig&
CMemoryTracker::GetConfig( void ) const
{
    return m_config;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      Internal helpers                                                   //
//                                                                         //
//-------------------------------------------------------------------------*/

CAllocationRecord*
CMemoryTracker::AllocateFromPool( void )
{
    MT::CScopeMutex poolLock( m_poolMutex );
    if ( GUCEF_NULL != m_freePool )
    {
        CAllocationRecord* rec = m_freePool;
        m_freePool = rec->hashNext;
        rec->hashNext = GUCEF_NULL;
        rec->hashPrev = GUCEF_NULL;
        return rec;
    }
    return CAllocationRecord::Allocate();
}

/*-------------------------------------------------------------------------*/

void
CMemoryTracker::ReturnToPool( CAllocationRecord* record )
{
    if ( GUCEF_NULL == record )
        return;

    /* Clear dynamic fields before pooling */
    if ( GUCEF_NULL != record->sourceFile )
    {
        ::free( record->sourceFile );
        record->sourceFile = GUCEF_NULL;
    }
    if ( GUCEF_NULL != record->allocCallstack )
    {
        MEMMAN_FreeCallstackCopy( record->allocCallstack );
        record->allocCallstack = GUCEF_NULL;
    }
    if ( GUCEF_NULL != record->deallocCallstack )
    {
        MEMMAN_FreeCallstackCopy( record->deallocCallstack );
        record->deallocCallstack = GUCEF_NULL;
    }
    if ( GUCEF_NULL != record->allocRawCallstack )
    {
        MEMMAN_FreeRawCallstack( record->allocRawCallstack );
        record->allocRawCallstack = GUCEF_NULL;
    }
    if ( GUCEF_NULL != record->deallocRawCallstack )
    {
        MEMMAN_FreeRawCallstack( record->deallocRawCallstack );
        record->deallocRawCallstack = GUCEF_NULL;
    }

    ::memset( record, 0, sizeof(CAllocationRecord) );

    MT::CScopeMutex poolLock( m_poolMutex );
    record->hashNext = m_freePool;
    m_freePool = record;
}

/*-------------------------------------------------------------------------*/

void
CMemoryTracker::InsertRecord( CAllocationRecord* record )
{
    /* Caller must hold write lock */
    UIntPtr key = (UIntPtr) record->reportedAddress;
    m_registry[ key ] = record;

    /* Statistics */
    ++m_numAllocations;
    m_allocatedMemory += (UInt32) record->reportedSize;
    if ( m_allocatedMemory > m_peakMemoryAllocation )
        m_peakMemoryAllocation = m_allocatedMemory;
    if ( m_numAllocations > m_peakTotalNumAllocations )
        m_peakTotalNumAllocations = m_numAllocations;
    m_totalMemoryAllocated += (UInt32) record->reportedSize;
    ++m_totalMemoryAllocations;

    /* Size histogram */
    UInt32 bucket = SizeToBucket( record->reportedSize );
    ++m_histogramCounts[ bucket ];
    m_histogramBytes[ bucket ] += (UInt64) record->reportedSize;

    /* Per-callsite aggregation (opt-in) */
    if ( m_config.enableCallsiteProfiling && GUCEF_NULL != record->sourceFile )
    {
        TCallsiteKey key2 = { record->sourceFile, record->sourceLine };
        CCallsiteStats& cs = m_callsiteMap[ key2 ];
        if ( cs.sourceFile == GUCEF_NULL )
        {
            cs.sourceFile = record->sourceFile;
            cs.sourceLine = record->sourceLine;
        }
        ++cs.allocCount;
        cs.totalBytesAllocated += (UInt64) record->reportedSize;
        cs.currentLiveBytes    += (UInt64) record->reportedSize;
        if ( cs.currentLiveBytes > cs.peakLiveBytes )
            cs.peakLiveBytes = cs.currentLiveBytes;
        UInt64 liveCount = cs.allocCount - cs.freeCount;
        if ( liveCount > cs.peakLiveCount )
            cs.peakLiveCount = liveCount;
    }
}

/*-------------------------------------------------------------------------*/

CAllocationRecord*
CMemoryTracker::GetRecord( const void* address )
{
    /* Caller must hold at least read lock */
    TRegistry::iterator i = m_registry.find( (UIntPtr) address );
    if ( i != m_registry.end() )
        return i->second;
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

CAllocationRecord*
CMemoryTracker::RemoveRecord( void* address )
{
    /* Caller must hold write lock */
    TRegistry::iterator i = m_registry.find( (UIntPtr) address );
    if ( i == m_registry.end() )
        return GUCEF_NULL;

    CAllocationRecord* rec = i->second;
    m_registry.erase( i );

    --m_numAllocations;
    m_allocatedMemory -= (UInt32) rec->reportedSize;

    /* Per-callsite aggregation (opt-in) */
    if ( m_config.enableCallsiteProfiling && GUCEF_NULL != rec->sourceFile )
    {
        TCallsiteKey cskey = { rec->sourceFile, rec->sourceLine };
        TCallsiteMap::iterator cit = m_callsiteMap.find( cskey );
        if ( cit != m_callsiteMap.end() )
        {
            CCallsiteStats& cs = cit->second;
            ++cs.freeCount;
            if ( cs.currentLiveBytes >= (UInt64) rec->reportedSize )
                cs.currentLiveBytes -= (UInt64) rec->reportedSize;
            else
                cs.currentLiveBytes = 0;
        }
    }

    return rec;
}

/*-------------------------------------------------------------------------*/

CAllocationRecord*
CMemoryTracker::GetSubRecord( const void* address )
{
    /* Caller must hold at least read lock */
    /* Sub-records (placement-new children) are stored in the registry under
     * their own address. We distinguish them by parentRecord != NULL. */
    TRegistry::iterator i = m_registry.find( (UIntPtr) address );
    if ( i != m_registry.end() && GUCEF_NULL != i->second->parentRecord )
        return i->second;
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

CAllocationRecord*
CMemoryTracker::RemoveSubRecord( void* address )
{
    TRegistry::iterator i = m_registry.find( (UIntPtr) address );
    if ( i == m_registry.end() || GUCEF_NULL == i->second->parentRecord )
        return GUCEF_NULL;

    CAllocationRecord* rec = i->second;
    m_registry.erase( i );
    --m_numSubAllocations;
    return rec;
}

/*-------------------------------------------------------------------------*/

bool
CMemoryTracker::ValidateRecord( CAllocationRecord* record )
{
    if ( GUCEF_NULL == record )
        return true;

    UInt32 violations = 0;
    bool ok = record->ValidateSentinels( violations );
    if ( !ok )
    {
        ++m_numBoundsViolations;
        Log( "Bounds Violation Detected: %s(%05d) %s(0x%p)",
             GUCEF_NULL != record->sourceFile ? record->sourceFile : "?",
             record->sourceLine,
             s_allocationTypes[ (unsigned char) record->allocationType ],
             record->reportedAddress );
    }
    return ok;
}

/*-------------------------------------------------------------------------*/

void
CMemoryTracker::PushDeallocRing( CAllocationRecord* record )
{
    /* Caller must hold write lock */
    /* Free pool management when ring is full */
    if ( m_deallocRingCount >= m_config.deallocRingCapacity )
    {
        /* Evict tail (oldest) */
        CAllocationRecord* evicted = m_deallocRingTail;
        if ( GUCEF_NULL != evicted )
        {
            m_deallocRingTail = evicted->hashPrev;
            if ( GUCEF_NULL != m_deallocRingTail )
                m_deallocRingTail->hashNext = GUCEF_NULL;
            else
                m_deallocRingHead = GUCEF_NULL;

            evicted->hashNext = GUCEF_NULL;
            evicted->hashPrev = GUCEF_NULL;
            ReturnToPool( evicted );
            --m_deallocRingCount;
        }
    }

    /* Push to head */
    record->hashPrev = GUCEF_NULL;
    record->hashNext = m_deallocRingHead;
    if ( GUCEF_NULL != m_deallocRingHead )
        m_deallocRingHead->hashPrev = record;
    else
        m_deallocRingTail = record;
    m_deallocRingHead = record;
    ++m_deallocRingCount;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      Core tracking                                                      //
//                                                                         //
//-------------------------------------------------------------------------*/

static size_t
GetSystemPageSize( void )
{
#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    SYSTEM_INFO si;
    ::GetSystemInfo( &si );
    return (size_t) si.dwPageSize;
#elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )
    long sz = ::sysconf( _SC_PAGESIZE );
    return sz > 0 ? (size_t) sz : 4096;
#else
    return 4096;
#endif
}

/*-------------------------------------------------------------------------*/

void*
CMemoryTracker::TrackAllocation( const char* file    ,
                                 int         line    ,
                                 size_t      size    ,
                                 char        type    ,
                                 void*       address ,
                                 const char* typeName )
{
    if ( !m_initialized || g_isDestructed )
        return ::malloc( size );

    /* Re-entrancy guard */
    if ( tl_inTracker )
        return ::malloc( size );
    tl_inTracker = true;

    if ( size == 0 )
        size = 1; /* ANSI: allocation of 0 must return a valid pointer */

    /* Determine actual type — detect placement new */
    char allocType = type;
    if ( allocType == 1 /* MM_NEW */ && GUCEF_NULL != address )
        allocType = 2; /* MM_PLACEMENT_NEW */

    void* result = GUCEF_NULL;

    MT::CScopeWriterLock writeLock( m_dataLock );

    /* Break-on-allocation hook */
    if ( m_config.breakOnAllocationCount > 0 &&
         m_totalMemoryAllocations == m_config.breakOnAllocationCount )
    {
        GUCEF_SETBREAKPOINT;
    }

    /* Exhaustive validation */
    if ( m_config.exhaustiveTesting )
        ValidateAllLiveAllocations();

    /* ---- Placement new ---- */
    if ( allocType == 2 /* MM_PLACEMENT_NEW */ && GUCEF_NULL != address )
    {
        CAllocationRecord* parentRec = GUCEF_NULL;
        CAllocationRecord* deallocRec = GUCEF_NULL;
        bool wouldFit = false;
        if ( ValidateAddressAccessibility( address, (UInt32)size, &parentRec, &deallocRec, wouldFit ) &&
             GUCEF_NULL != parentRec )
        {
            /* Create a sub-record hanging off the parent */
            CAllocationRecord* subRec = AllocateFromPool();
            if ( GUCEF_NULL != subRec )
            {
                subRec->reportedAddress = address;
                subRec->reportedSize    = size;
                subRec->actualAddress   = GUCEF_NULL; /* no backing allocation */
                subRec->actualSize      = 0;
                subRec->allocationType  = allocType;
                subRec->SetSourceFile( file );
                subRec->sourceLine      = (UInt32) line;
                subRec->parentRecord    = parentRec;

                /* Link into parent's child list */
                subRec->placementSiblingNext = parentRec->placementChildren;
                parentRec->placementChildren = subRec;
                parentRec->hadPlacementChildren = true;

                /* Also insert into registry for O(1) lookup */
                m_registry[ (UIntPtr) address ] = subRec;
                ++m_numSubAllocations;

                if ( m_config.enableCallstackCapture )
                    MEMMAN_GetCallstackCopyForCurrentThread( &subRec->allocCallstack, 1 );

                if ( m_config.logAlways )
                    Log( "MEMMAN: PlacementNew %s(%05d) %s(0x%p) size=%zu",
                         GUCEF_NULL != subRec->sourceFile ? subRec->sourceFile : "?",
                         line, s_allocationTypes[ (unsigned char) allocType ], address, size );

                result = address;
            }
        }
        else
        {
            Log( "MEMMAN: Placement new at unallocated address 0x%p size=%zu @ %s(%d)",
                 address, size, GUCEF_NULL != file ? file : "?", line );
        }
        tl_inTracker = false;
        return result;
    }

    /* ---- Realloc ---- */
    size_t originalReportedSize = 0;
    CAllocationRecord* existingRec = GUCEF_NULL;
    if ( allocType == 6 /* MM_REALLOC */ && GUCEF_NULL != address )
    {
        existingRec = RemoveRecord( address );
        if ( GUCEF_NULL == existingRec )
        {
            Log( "MEMMAN: realloc of unknown address 0x%p @ %s(%d)", address, GUCEF_NULL != file ? file : "?", line );
            tl_inTracker = false;
            return GUCEF_NULL;
        }
        originalReportedSize = existingRec->reportedSize;
        size_t desiredActual = size + (size_t) existingRec->paddingSize * sizeof(long) * 2;
        void* newActual = ::realloc( existingRec->actualAddress, desiredActual );
        if ( GUCEF_NULL == newActual )
        {
            Log( "MEMMAN: realloc failed. New size %zu old size %zu", desiredActual, existingRec->actualSize );
            /* Put it back */
            InsertRecord( existingRec );
            tl_inTracker = false;
            return GUCEF_NULL;
        }
        existingRec->actualAddress   = newActual;
        existingRec->actualSize      = desiredActual;
        existingRec->reportedAddress = (char*) newActual + (size_t) existingRec->paddingSize * sizeof(long);
        existingRec->reportedSize    = size;
        existingRec->InitializeReallocSentinels( ALLOC_BODY_SENTINEL, originalReportedSize );
        existingRec->SetSourceFile( file );
        existingRec->sourceLine = (UInt32) line;
        existingRec->allocationType = allocType;

        if ( m_config.enableCallstackCapture )
        {
            if ( GUCEF_NULL != existingRec->allocCallstack )
            {
                MEMMAN_FreeCallstackCopy( existingRec->allocCallstack );
                existingRec->allocCallstack = GUCEF_NULL;
            }
            MEMMAN_GetCallstackCopyForCurrentThread( &existingRec->allocCallstack, 1 );
        }

        InsertRecord( existingRec );
        result = existingRec->reportedAddress;
        tl_inTracker = false;
        return result;
    }

    /* ---- Normal alloc (new / new[] / malloc / calloc) ---- */
    CAllocationRecord* rec = AllocateFromPool();
    if ( GUCEF_NULL == rec )
    {
        Log( "MEMMAN: Failed to allocate tracking record (out of memory)" );
        tl_inTracker = false;
        return GUCEF_NULL;
    }

    UInt32 overheadBytes = (UInt32) m_config.paddingSize * sizeof(long) * 2 + (UInt32) sizeof(CAllocationRecord);
    m_overheadMemoryCost += overheadBytes;
    m_totalOverheadMemoryCost += overheadBytes;
    if ( m_overheadMemoryCost > m_peakOverheadMemoryCost )
        m_peakOverheadMemoryCost = m_overheadMemoryCost;

    rec->reportedSize   = size;
    rec->allocationType = allocType;
    rec->SetSourceFile( file );
    rec->sourceLine     = (UInt32) line;
    rec->breakOptions   = 0;

    /* --- Guard page allocation (opt-in, disabled under ASan) --- */
    #if !defined( GUCEF_MLF_ASAN_ACTIVE )
    if ( m_config.useGuardPages )
    {
        size_t pageSize    = GetSystemPageSize();
        size_t frontPad    = (size_t) m_config.paddingSize * sizeof(long);
        size_t minCommit   = frontPad + size;
        /* Round up so user data ends exactly at a page boundary */
        size_t pageAligned = ( (minCommit + pageSize - 1) / pageSize ) * pageSize;
        size_t guardTotal  = pageAligned + pageSize; /* +1 NOACCESS guard page */

        void* region = GUCEF_NULL;
        #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
        /* Reserve full region (all NOACCESS), then commit only the data pages */
        region = ::VirtualAlloc( GUCEF_NULL, guardTotal, MEM_RESERVE, PAGE_NOACCESS );
        if ( GUCEF_NULL != region )
        {
            if ( GUCEF_NULL == ::VirtualAlloc( region, pageAligned, MEM_COMMIT, PAGE_READWRITE ) )
            {
                ::VirtualFree( region, 0, MEM_RELEASE );
                region = GUCEF_NULL;
            }
        }
        #elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )
        region = ::mmap( GUCEF_NULL, guardTotal, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0 );
        if ( MAP_FAILED == region )
            region = GUCEF_NULL;
        else if ( 0 != ::mprotect( (char*) region + pageAligned, pageSize, PROT_NONE ) )
        {
            ::munmap( region, guardTotal );
            region = GUCEF_NULL;
        }
        #endif

        if ( GUCEF_NULL != region )
        {
            /* User data ends at the page boundary — overflow hits the guard page immediately */
            rec->actualAddress       = region;
            rec->actualSize          = pageAligned;
            rec->reportedAddress     = (char*) region + ( pageAligned - size );
            rec->paddingSize         = 0;            /* guard page replaces back sentinel */
            rec->guardPageRegionSize = guardTotal;

            /* Write front-of-block sentinel pattern for pre-buffer detection in dumps */
            long* front      = (long*) region;
            size_t frontLongs = ( pageAligned - size ) / sizeof(long);
            for ( size_t fi = 0; fi < frontLongs; ++fi )
                front[ fi ] = ALLOC_PADDING_SENTINEL;
        }
        else
        {
            Log( "MEMMAN: guard page allocation failed for size=%zu, falling back to malloc", size );
            /* Fall through to normal malloc below */
        }
    }
    #endif /* GUCEF_MLF_ASAN_ACTIVE */

    /* --- Normal malloc path (used when guard pages are disabled or fell back) --- */
    if ( GUCEF_NULL == rec->actualAddress )
    {
        rec->actualSize      = size + (size_t) m_config.paddingSize * sizeof(long) * 2;
        rec->actualAddress   = ::malloc( rec->actualSize );
        rec->reportedAddress = (char*) rec->actualAddress + (size_t) m_config.paddingSize * sizeof(long);
        rec->paddingSize     = (UInt16) m_config.paddingSize;
    }

    if ( GUCEF_NULL == rec->actualAddress )
    {
        Log( "MEMMAN: allocation failed for size=%zu — out of memory", size );
        ReturnToPool( rec );
        tl_inTracker = false;
        return GUCEF_NULL;
    }

    /* Capture timestamp */
    rec->allocationTimestampUs = GetCurrentTimestampUs() - m_timestampBaseUs;

    if ( m_config.enableCallstackCapture )
        MEMMAN_GetCallstackCopyForCurrentThread( &rec->allocCallstack, 1 );

    if ( m_config.enableRawCallstackCapture )
        MEMMAN_CaptureRawCallstack( &rec->allocRawCallstack, m_config.maxRawCallstackDepth );

    /* Initialize sentinels */
#if defined( GUCEF_MLF_ASAN_ACTIVE ) || defined( GUCEF_MLF_MSAN_ACTIVE )
    /* ASan/MSan owns memory poisoning — do not fill the body */
    (void) ALLOC_BODY_SENTINEL;
#else
    if ( allocType == 5 /* MM_CALLOC */ )
        rec->InitializeSentinels( 0x00000000 );
    else
        rec->InitializeSentinels( ALLOC_BODY_SENTINEL );
#endif

    InsertRecord( rec );

    if ( m_config.logAlways )
        Log( "MEMMAN: Alloc %s(%05d) %s(0x%p) size=%zu",
             GUCEF_NULL != rec->sourceFile ? rec->sourceFile : "?",
             line, s_allocationTypes[ (unsigned char) allocType ], rec->reportedAddress, size );

    result = rec->reportedAddress;
    tl_inTracker = false;
    return result;
}

/*-------------------------------------------------------------------------*/

void
CMemoryTracker::TrackDeallocation( void*       address  ,
                                   char        type     ,
                                   const char* typeName )
{
    if ( g_isDestructed )
        return;
    if ( !m_initialized )
    {
        if ( GUCEF_NULL != address )
            ::free( address );
        return;
    }

    /* Re-entrancy guard */
    if ( tl_inTracker )
    {
        ::free( address );
        return;
    }
    tl_inTracker = true;

    /* Pop the per-thread owner context */
    SOwnerContextNode* ownerCtx = tl_ownerStack;
    if ( GUCEF_NULL != ownerCtx )
        tl_ownerStack = ownerCtx->prev;

    MT::CScopeWriterLock writeLock( m_dataLock );

    if ( m_config.exhaustiveTesting )
        ValidateAllLiveAllocations();

    /* Check if it's a sub-record (placement new) first */
    CAllocationRecord* subRec = GetSubRecord( address );
    if ( GUCEF_NULL != subRec )
    {
        /* Remove placement-new child */
        RemoveSubRecord( address );

        /* Unlink from parent's child list */
        CAllocationRecord* parent = subRec->parentRecord;
        if ( GUCEF_NULL != parent )
        {
            CAllocationRecord** childPtr = &parent->placementChildren;
            while ( GUCEF_NULL != *childPtr )
            {
                if ( *childPtr == subRec )
                {
                    *childPtr = subRec->placementSiblingNext;
                    break;
                }
                childPtr = &(*childPtr)->placementSiblingNext;
            }
        }

        if ( m_config.logAlways && GUCEF_NULL != ownerCtx )
            Log( "MEMMAN: PlacementDealloc %s(%05d) (0x%p) size=%zu",
                 ownerCtx->file, ownerCtx->line, address, subRec->reportedSize );

        CAllocationRecord::Release( subRec );
        if ( GUCEF_NULL != ownerCtx )
            ::free( ownerCtx );
        tl_inTracker = false;
        return;
    }

    /* Normal deallocation */
    CAllocationRecord* rec = RemoveRecord( address );
    if ( GUCEF_NULL == rec )
    {
        if ( GUCEF_NULL != ownerCtx )
            Log( "MEMMAN: Dealloc of unknown address 0x%p @ %s(%d)", address, ownerCtx->file, ownerCtx->line );
        ::free( address );
        if ( GUCEF_NULL != ownerCtx )
            ::free( ownerCtx );
        tl_inTracker = false;
        return;
    }

    if ( m_config.logAlways )
    {
        if ( GUCEF_NULL != ownerCtx )
            Log( "MEMMAN: Dealloc %s(%05d) %s(0x%p) size=%zu",
                 ownerCtx->file, ownerCtx->line,
                 s_allocationTypes[ (unsigned char) type ], address, rec->reportedSize );
        else
            Log( "MEMMAN: Dealloc %s(0x%p) size=%zu",
                 s_allocationTypes[ (unsigned char) type ], address, rec->reportedSize );
    }

    ValidateRecord( rec );

    /* Check alloc/dealloc type mismatch */
    if ( 0 == rec->suppressMismatchCheck && rec->IsDeallocationTypeMismatch( type ) )
    {
        ++m_numMismatchedDeallocs;
        Log( "MEMMAN: Alloc/dealloc type mismatch: allocated as %s, freed as %s @ 0x%p",
             s_allocationTypes[ (unsigned char) rec->allocationType ],
             s_allocationTypes[ (unsigned char) type ], address );
        switch ( m_config.deallocMismatchResponse )
        {
            case MISMATCH_BREAK:
                GUCEF_SETBREAKPOINT;
                break;
            case MISMATCH_ABORT:
                GUCEF_SETBREAKPOINT;
                ::abort();
                break;
            case MISMATCH_LOG:
            default:
                break;
        }
    }

    /* Break-on-dealloc hook */
    if ( (rec->breakOptions & BREAK_OPTION_ON_DEALLOC) != 0 )
        GUCEF_SETBREAKPOINT;

    /* Lock range protection check — warn if deallocating a range declared protected by an unheld lock */
    #ifndef GUCEF_MLF_TSAN_ACTIVE
    {
        CLockTracer* lockTracer = CLockTracer::Instance();
        if ( GUCEF_NULL != lockTracer )
        {
            void* protectingLock = GUCEF_NULL;
            if ( lockTracer->IsRangeProtectedByUnheldLock( address, rec->reportedSize, &protectingLock ) )
            {
                Log( "MEMMAN: WARNING: Deallocating 0x%p (size=%zu) which is declared protected by lock 0x%p, but that lock is not currently held",
                     address, rec->reportedSize, protectingLock );
            }
        }
    }
    #endif /* GUCEF_MLF_TSAN_ACTIVE */

    /* Poison freed memory with recognizable pattern before freeing.
     * Skip for guard-page allocations — the memory is about to be released
     * back to the OS and poisoning serves no diagnostic purpose there. */
#if !defined( GUCEF_MLF_ASAN_ACTIVE ) && !defined( GUCEF_MLF_MSAN_ACTIVE )
    if ( GUCEF_NULL != rec->actualAddress && rec->actualSize > 0 && 0 == rec->guardPageRegionSize )
    {
        long* p     = (long*) rec->actualAddress;
        size_t longs = rec->actualSize / sizeof(long);
        for ( size_t pi = 0; pi < longs; ++pi )
            p[ pi ] = ALLOC_FREED_BODY_SENTINEL;
        /* Remainder bytes */
        char* cp = (char*)( p + longs );
        size_t rem = rec->actualSize - longs * sizeof(long);
        for ( size_t pi = 0; pi < rem; ++pi )
            cp[ pi ] = (char) 0xFE;
    }
#endif

    /* Free the backing memory */
    #if !defined( GUCEF_MLF_ASAN_ACTIVE )
    if ( rec->guardPageRegionSize > 0 )
    {
        /* Guard-page-backed: release the entire reserved region (data + guard page) */
        #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
        ::VirtualFree( rec->actualAddress, 0, MEM_RELEASE );
        #elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )
        ::munmap( rec->actualAddress, rec->guardPageRegionSize );
        #endif
    }
    else
    #endif /* GUCEF_MLF_ASAN_ACTIVE */
    {
        ::free( rec->actualAddress );
    }
    rec->actualAddress = GUCEF_NULL;

    /* Capture dealloc callstack */
    if ( m_config.enableCallstackCapture )
    {
        if ( GUCEF_NULL != rec->deallocCallstack )
        {
            MEMMAN_FreeCallstackCopy( rec->deallocCallstack );
            rec->deallocCallstack = GUCEF_NULL;
        }
        MEMMAN_GetCallstackCopyForCurrentThread( &rec->deallocCallstack, 1 );
    }
    if ( m_config.enableRawCallstackCapture )
    {
        if ( GUCEF_NULL != rec->deallocRawCallstack )
        {
            MEMMAN_FreeRawCallstack( rec->deallocRawCallstack );
            rec->deallocRawCallstack = GUCEF_NULL;
        }
        MEMMAN_CaptureRawCallstack( &rec->deallocRawCallstack, m_config.maxRawCallstackDepth );
    }

    /* Overhead accounting */
    UInt32 overheadBytes = (UInt32) rec->paddingSize * sizeof(long) * 2;
    if ( m_overheadMemoryCost >= overheadBytes )
        m_overheadMemoryCost -= overheadBytes;

    /* Push to dealloc ring for use-after-free diagnostics */
    PushDeallocRing( rec );

    if ( GUCEF_NULL != ownerCtx )
        ::free( ownerCtx );

    tl_inTracker = false;
}

/*-------------------------------------------------------------------------*/

void
CMemoryTracker::TrackDeallocationEx( const char* file    ,
                                     int         line    ,
                                     void*       address ,
                                     char        type    ,
                                     const char* typeName )
{
    if ( g_isDestructed )
        return;
    if ( !m_initialized )
    {
        if ( GUCEF_NULL != address )
            ::free( address );
        return;
    }
    PushOwnerContext( file, line, typeName );
    TrackDeallocation( address, type, typeName );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      Owner context (per-thread stack)                                   //
//                                                                         //
//-------------------------------------------------------------------------*/

int
CMemoryTracker::PushOwnerContext( const char* file, int line, const char* typeName )
{
    SOwnerContextNode* node = (SOwnerContextNode*) ::malloc( sizeof(SOwnerContextNode) );
    if ( GUCEF_NULL == node )
        return 0;
    node->file     = file;
    node->line     = line;
    node->typeName = typeName;
    node->prev     = tl_ownerStack;
    tl_ownerStack  = node;
    return 1;
}

/*-------------------------------------------------------------------------*/

void
CMemoryTracker::PopOwnerContext( void )
{
    SOwnerContextNode* node = tl_ownerStack;
    if ( GUCEF_NULL != node )
    {
        tl_ownerStack = node->prev;
        ::free( node );
    }
}

/*-------------------------------------------------------------------------*/

SOwnerContextNode*
CMemoryTracker::GetTopOwnerContext( void )
{
    return tl_ownerStack;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      Break hooks                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CMemoryTracker::SetBreakOnAllocationCount( int count )
{
    MT::CScopeWriterLock writeLock( m_dataLock );
    m_config.breakOnAllocationCount = count;
}

/*-------------------------------------------------------------------------*/

void
CMemoryTracker::SetBreakOnDeallocation( void* address )
{
    MT::CScopeWriterLock writeLock( m_dataLock );
    CAllocationRecord* rec = GetRecord( address );
    if ( GUCEF_NULL != rec )
        rec->breakOptions |= BREAK_OPTION_ON_DEALLOC;
}

/*-------------------------------------------------------------------------*/

void
CMemoryTracker::SetBreakOnReallocation( void* address )
{
    MT::CScopeWriterLock writeLock( m_dataLock );
    CAllocationRecord* rec = GetRecord( address );
    if ( GUCEF_NULL != rec )
        rec->breakOptions |= BREAK_OPTION_ON_REALLOC;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      Validation                                                         //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CMemoryTracker::ValidateKnownAllocPtr( const void* address, const char* file, int line )
{
    MT::CScopeReaderLock readLock( m_dataLock );
    if ( !m_initialized || GUCEF_NULL == address )
        return;

    CAllocationRecord* rec = GetRecord( address );
    if ( GUCEF_NULL != rec )
    {
        if ( !ValidateRecord( rec ) )
        {
            Log( "MEMMAN: Block validation failed at 0x%p @ %s(%d)", address, file, line );
            GUCEF_SETBREAKPOINT;
        }
        return;
    }
    Log( "MEMMAN: ValidateKnownAllocPtr: unknown address 0x%p @ %s(%d)", address, file, line );
}

/*-------------------------------------------------------------------------*/

void
CMemoryTracker::ValidateKnownAllocBlock( const void* address, UInt32 blocksize, const char* file, int line )
{
    MT::CScopeReaderLock readLock( m_dataLock );
    if ( !m_initialized || GUCEF_NULL == address )
        return;

    CAllocationRecord* rec = GetRecord( address );
    if ( GUCEF_NULL != rec )
    {
        if ( ValidateRecord( rec ) )
        {
            if ( rec->reportedSize != (size_t) blocksize )
                Log( "MEMMAN: ValidateKnownAllocBlock: size mismatch at 0x%p: expected %u actual %zu @ %s(%d)",
                     address, blocksize, rec->reportedSize, file, line );
        }
        return;
    }
    Log( "MEMMAN: ValidateKnownAllocBlock: unknown address 0x%p @ %s(%d)", address, file, line );
}

/*-------------------------------------------------------------------------*/

void
CMemoryTracker::ValidateChunk( const void* address, const void* chunk, UInt32 blocksize, const char* file, int line )
{
    ValidateKnownAllocBlock( address, blocksize, file, line );
}

/*-------------------------------------------------------------------------*/

void
CMemoryTracker::ValidateAccessibility( const void* address, UInt32 blocksize, const char* file, int line )
{
    MT::CScopeReaderLock readLock( m_dataLock );
    if ( !m_initialized || GUCEF_NULL == address )
        return;

    CAllocationRecord* currentRec = GUCEF_NULL;
    CAllocationRecord* deallocRec = GUCEF_NULL;
    bool wouldFit = false;
    if ( !ValidateAddressAccessibility( address, blocksize, &currentRec, &deallocRec, wouldFit ) )
    {
        Log( "MEMMAN: ValidateAccessibility failed for 0x%p size=%u @ %s(%d)", address, blocksize, file, line );
        if ( wouldFit && GUCEF_NULL != deallocRec )
            Log( "MEMMAN:   note: address belongs to a recently freed block" );
    }
}

/*-------------------------------------------------------------------------*/

void
CMemoryTracker::ValidatePendingDestructor( const char* file, int line, const void* address, size_t size, const char* typeName )
{
    ValidateKnownAllocPtr( address, file, line );
}

/*-------------------------------------------------------------------------*/

void
CMemoryTracker::ValidateFinishedDestructor( const char* file, int line, const void* address, size_t size, const char* typeName )
{
    /* After destruction the memory should no longer be in our registry */
}

/*-------------------------------------------------------------------------*/

bool
CMemoryTracker::ValidateAllLiveAllocations( void )
{
    /* Caller must hold at least read lock, or this is called internally under write lock */
    bool allOk = true;
    TRegistry::iterator i = m_registry.begin();
    while ( i != m_registry.end() )
    {
        CAllocationRecord* rec = i->second;
        if ( GUCEF_NULL != rec && GUCEF_NULL == rec->parentRecord )
        {
            if ( !ValidateRecord( rec ) )
                allOk = false;
        }
        ++i;
    }
    return allOk;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      Registry / data-lock accessors                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

MT::CReadWriteLock&
CMemoryTracker::GetDataLock( void )
{
    return m_dataLock;
}

/*-------------------------------------------------------------------------*/

const CMemoryTracker::TRegistry&
CMemoryTracker::GetRegistry( void ) const
{
    return m_registry;
}

/*-------------------------------------------------------------------------*/

CAllocationRecord*
CMemoryTracker::GetDeallocRingHead( void ) const
{
    return m_deallocRingHead;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      Statistics accessors                                               //
//                                                                         //
//-------------------------------------------------------------------------*/

Int32  CMemoryTracker::GetTotalMemoryAllocations( void ) const { return m_totalMemoryAllocations; }
UInt32 CMemoryTracker::GetTotalMemoryAllocated( void )   const { return m_totalMemoryAllocated; }
UInt32 CMemoryTracker::GetTotalMemoryUsed( void )        const { return m_totalMemoryUsed; }
UInt32 CMemoryTracker::GetPeakMemoryAllocation( void )   const { return m_peakMemoryAllocation; }
UInt32 CMemoryTracker::GetPeakTotalNumAllocations( void )const { return m_peakTotalNumAllocations; }
UInt32 CMemoryTracker::GetOverheadMemoryCost( void )     const { return m_overheadMemoryCost; }
UInt32 CMemoryTracker::GetPeakOverheadMemoryCost( void ) const { return m_peakOverheadMemoryCost; }
UInt32 CMemoryTracker::GetTotalOverheadMemoryCost( void )const { return m_totalOverheadMemoryCost; }
UInt32 CMemoryTracker::GetAllocatedMemory( void )        const { return m_allocatedMemory; }
UInt32 CMemoryTracker::GetNumBoundsViolations( void )    const { return m_numBoundsViolations; }
UInt32 CMemoryTracker::GetNumAllocations( void )         const { return m_numAllocations; }
UInt32 CMemoryTracker::GetNumSubAllocations( void )      const { return m_numSubAllocations; }
UInt32 CMemoryTracker::GetNumMismatchedDeallocs( void )  const { return m_numMismatchedDeallocs; }

/*-------------------------------------------------------------------------*/

const CMemoryTracker::TCallsiteMap&
CMemoryTracker::GetCallsiteMap( void ) const
{
    return m_callsiteMap;
}

/*-------------------------------------------------------------------------*/

void
CMemoryTracker::GetSizeHistogram( UInt64 outCounts[ HISTOGRAM_BUCKET_COUNT ],
                                  UInt64 outBytes [ HISTOGRAM_BUCKET_COUNT ] ) const
{
    for ( UInt32 b = 0; b < HISTOGRAM_BUCKET_COUNT; ++b )
    {
        outCounts[ b ] = m_histogramCounts[ b ];
        outBytes[ b ]  = m_histogramBytes[ b ];
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      Nearest-node queries                                               //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CMemoryTracker::FindNearestCurrentRecords( const void* address, UInt32 maxCount, TUInt64ToRecordMap& out )
{
    const Int8* testAddr = (const Int8*) address;

    TRegistry::iterator i = m_registry.begin();
    while ( i != m_registry.end() )
    {
        CAllocationRecord* rec = i->second;
        if ( GUCEF_NULL != rec )
        {
            const Int8* repAddr = (const Int8*) rec->reportedAddress;
            UInt64 dist;
            if ( testAddr > repAddr )
                dist = (UInt64)( testAddr - repAddr );
            else
                dist = (UInt64)( repAddr - testAddr );

            if ( out.size() < (size_t) maxCount )
            {
                out[ dist ] = rec;
            }
            else
            {
                TUInt64ToRecordMap::reverse_iterator last = out.rbegin();
                if ( (*last).first > dist )
                {
                    TUInt64ToRecordMap::iterator lastFwd = out.end();
                    --lastFwd;
                    out.erase( lastFwd );
                    out[ dist ] = rec;
                }
            }
        }
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

void
CMemoryTracker::FindNearestDeallocatedRecords( const void* address, UInt32 maxCount, TUInt64ToRecordMap& out )
{
    const Int8* testAddr = (const Int8*) address;
    CAllocationRecord* node = m_deallocRingHead;
    while ( GUCEF_NULL != node )
    {
        const Int8* repAddr = (const Int8*) node->reportedAddress;
        UInt64 dist;
        if ( testAddr > repAddr )
            dist = (UInt64)( testAddr - repAddr );
        else
            dist = (UInt64)( repAddr - testAddr );

        if ( out.size() < (size_t) maxCount )
        {
            out[ dist ] = node;
        }
        else
        {
            TUInt64ToRecordMap::reverse_iterator last = out.rbegin();
            if ( (*last).first > dist )
            {
                TUInt64ToRecordMap::iterator lastFwd = out.end();
                --lastFwd;
                out.erase( lastFwd );
                out[ dist ] = node;
            }
        }
        node = node->hashNext;
    }
}

/*-------------------------------------------------------------------------*/

bool
CMemoryTracker::ValidateAddressAccessibility( const void*         address                ,
                                              UInt32              blockSize              ,
                                              CAllocationRecord** locatedCurrentRecord   ,
                                              CAllocationRecord** locatedDeallocRecord   ,
                                              bool&               wouldHaveFitInDealloced )
{
    *locatedCurrentRecord = GUCEF_NULL;
    *locatedDeallocRecord = GUCEF_NULL;
    wouldHaveFitInDealloced = false;

    TUInt64ToRecordMap nearest;
    FindNearestCurrentRecords( address, 1, nearest );
    if ( !nearest.empty() )
    {
        CAllocationRecord* rec = nearest.begin()->second;
        *locatedCurrentRecord = rec;
        const Int8* repAddr = (const Int8*) rec->reportedAddress;
        const Int8* accAddr = (const Int8*) address;
        UInt64 offset = ( accAddr > repAddr ) ? (UInt64)(accAddr - repAddr) : (UInt64)(repAddr - accAddr);
        if ( rec->reportedSize > offset && rec->reportedSize - offset >= (size_t) blockSize )
            return true;
    }

    nearest.clear();
    FindNearestDeallocatedRecords( address, 1, nearest );
    if ( !nearest.empty() )
    {
        CAllocationRecord* rec = nearest.begin()->second;
        *locatedDeallocRecord = rec;
        const Int8* repAddr = (const Int8*) rec->reportedAddress;
        const Int8* accAddr = (const Int8*) address;
        UInt64 offset = ( accAddr > repAddr ) ? (UInt64)(accAddr - repAddr) : (UInt64)(repAddr - accAddr);
        if ( rec->reportedSize > offset && rec->reportedSize - offset >= (size_t) blockSize )
            wouldHaveFitInDealloced = true;
    }

    return false;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      Logging                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CMemoryTracker::Log( const char* fmt, ... )
{
    if ( m_config.cleanLogFileOnFirstRun )
    {
        GUCEF_UNLINK( m_logFileBuf );
        /* We allow this to clear once; subsequent writes use "ab" */
        m_config.cleanLogFileOnFirstRun = false;
    }

    static char buffer[ 2048 ];
    va_list args;
    va_start( args, fmt );
    vsnprintf( buffer, sizeof(buffer) - 1, fmt, args );
    va_end( args );
    buffer[ sizeof(buffer) - 1 ] = '\0';

    FILE* fp = fopen( m_logFileBuf, "ab" );
    if ( GUCEF_NULL != fp )
    {
        fprintf( fp, "%s\r\n", buffer );
        fclose( fp );
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MLF */
}; /* namespace GUCEF */

/*--------------------------------------------------------------------------*/
