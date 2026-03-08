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

#ifndef GUCEF_MLF_CMEMORYTRACKER_H
#define GUCEF_MLF_CMEMORYTRACKER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifdef GUCEF_MOVE_SEMANTICS_SUPPORTED
  #include <unordered_map>
#else
  #include <map>
#endif

#ifndef GUCEF_MLF_ETYPES_H
#include "gucefMLF_ETypes.h"
#define GUCEF_MLF_ETYPES_H
#endif /* GUCEF_MLF_ETYPES_H ? */

#ifndef GUCEF_MLF_SMEMORYTRACKERCONFIG_H
#include "gucefMLF_SMemoryTrackerConfig.h"
#define GUCEF_MLF_SMEMORYTRACKERCONFIG_H
#endif /* GUCEF_MLF_SMEMORYTRACKERCONFIG_H ? */

#ifndef GUCEF_MLF_CALLOCATIONRECORD_H
#include "gucefMLF_CAllocationRecord.h"
#define GUCEF_MLF_CALLOCATIONRECORD_H
#endif /* GUCEF_MLF_CALLOCATIONRECORD_H ? */

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_MT_CREADWRITELOCK_H
#include "gucefMT_CReadWriteLock.h"
#define GUCEF_MT_CREADWRITELOCK_H
#endif /* GUCEF_MT_CREADWRITELOCK_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MLF {

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 * Thread-local owner context node for SetOwner/dealloc source tracking.
 * Stored in a per-thread LIFO stack — no global write-lock contention.
 */
struct SOwnerContextNode
{
    const char*        file;
    int                line;
    const char*        typeName;
    SOwnerContextNode* prev;    /* previous (older) node in the per-thread stack */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 * Core singleton memory tracker.
 *
 * Responsibilities:
 *  - Registry of all live allocations (address → CAllocationRecord*)
 *  - Dealloc ring for use-after-free diagnostics
 *  - Free-pool of recycled CAllocationRecord nodes
 *  - Thread-local owner context stack (SetOwner)
 *  - Thread-local re-entrancy guard
 *  - Break-on-allocation / break-on-deallocation hooks
 *  - Windows SEH vectored exception handler
 *  - Statistics (totalAllocations, peakAllocation, …)
 *
 * Locking:
 *  - m_dataLock (CReadWriteLock) guards the registry + dealloc ring
 *  - m_poolMutex (CMutex) guards the free-pool
 *  - Owner context stack is per-thread (no lock needed)
 */
class GUCEF_HIDDEN CMemoryTracker
{
    public:

    /* ------------------------------------------------------------------ */
    /* Singleton lifecycle                                                  */
    /* ------------------------------------------------------------------ */
    static CMemoryTracker* Instance( void );
    static void            Deinstance( void );
    static bool            IsConstructed( void );
    static bool            IsDestructed( void );

    /* ------------------------------------------------------------------ */
    /* Configuration                                                        */
    /* ------------------------------------------------------------------ */
    void ApplyConfig( const SMemoryTrackerConfig& cfg );
    const SMemoryTrackerConfig& GetConfig( void ) const;

    /* ------------------------------------------------------------------ */
    /* Core alloc/dealloc tracking                                          */
    /* ------------------------------------------------------------------ */
    void* TrackAllocation( const char* file    ,
                           int         line    ,
                           size_t      size    ,
                           char        type    ,
                           void*       address ,
                           const char* typeName );

    void  TrackDeallocation( void*       address  ,
                             char        type     ,
                             const char* typeName );

    void  TrackDeallocationEx( const char* file    ,
                               int         line    ,
                               void*       address ,
                               char        type    ,
                               const char* typeName );

    /* ------------------------------------------------------------------ */
    /* Owner context (source file/line for delete/free)                     */
    /* ------------------------------------------------------------------ */
    int  PushOwnerContext( const char* file, int line, const char* typeName );
    void PopOwnerContext( void );

    /* ------------------------------------------------------------------ */
    /* Break hooks                                                          */
    /* ------------------------------------------------------------------ */
    void SetBreakOnAllocationCount( int count );
    void SetBreakOnDeallocation( void* address );
    void SetBreakOnReallocation( void* address );

    /* ------------------------------------------------------------------ */
    /* Validation                                                           */
    /* ------------------------------------------------------------------ */
    void ValidateKnownAllocPtr( const void* address, const char* file, int line );
    void ValidateKnownAllocBlock( const void* address, UInt32 blocksize, const char* file, int line );
    void ValidateChunk( const void* address, const void* chunk, UInt32 blocksize, const char* file, int line );
    void ValidateAccessibility( const void* address, UInt32 blocksize, const char* file, int line );
    void ValidatePendingDestructor( const char* file, int line, const void* address, size_t size, const char* typeName );
    void ValidateFinishedDestructor( const char* file, int line, const void* address, size_t size, const char* typeName );
    bool ValidateAllLiveAllocations( void );

    /* ------------------------------------------------------------------ */
    /* Registry queries (used by CReporter)                                 */
    /* ------------------------------------------------------------------ */
#ifdef GUCEF_MOVE_SEMANTICS_SUPPORTED
    typedef std::unordered_map< UIntPtr, CAllocationRecord* > TRegistry;
#else
    typedef std::map< UIntPtr, CAllocationRecord* > TRegistry;
#endif

    typedef std::map< UInt64, CAllocationRecord* > TUInt64ToRecordMap;

    /* Acquire reader lock; caller must hold it for the duration of access  */
    MT::CReadWriteLock& GetDataLock( void );

    /* Raw registry access — call only while holding at least a read lock  */
    const TRegistry& GetRegistry( void ) const;

    /* Dealloc ring — raw access under data lock                           */
    CAllocationRecord* GetDeallocRingHead( void ) const;

    /* ------------------------------------------------------------------ */
    /* Statistics (read under data lock)                                    */
    /* ------------------------------------------------------------------ */
    Int32  GetTotalMemoryAllocations( void )    const;
    UInt32 GetTotalMemoryAllocated( void )      const;
    UInt32 GetTotalMemoryUsed( void )           const;
    UInt32 GetPeakMemoryAllocation( void )      const;
    UInt32 GetPeakTotalNumAllocations( void )   const;
    UInt32 GetOverheadMemoryCost( void )        const;
    UInt32 GetPeakOverheadMemoryCost( void )    const;
    UInt32 GetTotalOverheadMemoryCost( void )   const;
    UInt32 GetAllocatedMemory( void )           const;
    UInt32 GetNumBoundsViolations( void )       const;
    UInt32 GetNumAllocations( void )            const;
    UInt32 GetNumSubAllocations( void )         const;

    /* ------------------------------------------------------------------ */
    /* Nearest-node queries (for exception reports, used by CReporter)      */
    /* ------------------------------------------------------------------ */
    void FindNearestCurrentRecords( const void* address, UInt32 maxCount, TUInt64ToRecordMap& out );
    void FindNearestDeallocatedRecords( const void* address, UInt32 maxCount, TUInt64ToRecordMap& out );
    bool ValidateAddressAccessibility( const void*         address                ,
                                       UInt32              blockSize              ,
                                       CAllocationRecord** locatedCurrentRecord   ,
                                       CAllocationRecord** locatedDeallocatedRecord,
                                       bool&               wouldHaveFitInDealloced );

    /* Log helper — writes to the configured log file */
    void Log( const char* fmt, ... );

    private:

    CMemoryTracker( void );
    ~CMemoryTracker( void );
    CMemoryTracker( const CMemoryTracker& );
    CMemoryTracker& operator=( const CMemoryTracker& );

    void Initialize( void );
    void Release( void );

    /* ------------------------------------------------------------------ */
    /* Internal helpers                                                     */
    /* ------------------------------------------------------------------ */
    void InsertRecord( CAllocationRecord* record );
    CAllocationRecord* GetRecord( const void* address );
    CAllocationRecord* RemoveRecord( void* address );

    CAllocationRecord* GetSubRecord( const void* address );
    CAllocationRecord* RemoveSubRecord( void* address );

    bool ValidateRecord( CAllocationRecord* record );

    CAllocationRecord* AllocateFromPool( void );
    void ReturnToPool( CAllocationRecord* record );

    void PushDeallocRing( CAllocationRecord* record );

    /* ------------------------------------------------------------------ */
    /* Thread-local owner context                                           */
    /* ------------------------------------------------------------------ */
    SOwnerContextNode* GetTopOwnerContext( void );
    void               FreeOwnerContext( SOwnerContextNode* node );

    /* ------------------------------------------------------------------ */
    /* Members                                                              */
    /* ------------------------------------------------------------------ */
    SMemoryTrackerConfig    m_config;
    bool                    m_initialized;
    bool                    m_shutdownCalled;
    char                    m_logFileBuf[ 2048 ]; /* writable copy of log file path */

    TRegistry               m_registry;            /* address → active record */
    MT::CReadWriteLock      m_dataLock;             /* guards registry + dealloc ring */

    CAllocationRecord*      m_deallocRingHead;      /* most-recently freed (newest) */
    CAllocationRecord*      m_deallocRingTail;      /* least-recently freed (oldest) */
    UInt32                  m_deallocRingCount;

    CAllocationRecord*      m_freePool;             /* recycled nodes (singly-linked via hashNext) */
    MT::CMutex              m_poolMutex;

    /* Statistics */
    Int32   m_totalMemoryAllocations;
    UInt32  m_totalMemoryAllocated;
    UInt32  m_totalMemoryUsed;
    UInt32  m_peakMemoryAllocation;
    UInt32  m_peakTotalNumAllocations;
    UInt32  m_overheadMemoryCost;
    UInt32  m_peakOverheadMemoryCost;
    UInt32  m_totalOverheadMemoryCost;
    UInt32  m_allocatedMemory;
    UInt32  m_numBoundsViolations;
    UInt32  m_numAllocations;
    UInt32  m_numSubAllocations;

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
  #if ( _WIN32_WINNT >= 0x0500 )
    void* m_vectoredExceptionHandler;
  #endif
#endif

    static MT::CMutex    g_instanceLock;
    static bool          g_isConstructed;
    static bool          g_isDestructed;
    static CMemoryTracker* g_instance;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MLF */
}; /* namespace GUCEF */

/*--------------------------------------------------------------------------*/

#endif /* GUCEF_MLF_CMEMORYTRACKER_H ? */
