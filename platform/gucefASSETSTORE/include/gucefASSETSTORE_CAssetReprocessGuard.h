/*
 *  gucefASSETSTORE: GUCEF module for online asset stores and the primary asset store
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

#ifndef GUCEF_ASSETSTORE_CASSETREPROCESSGUARD_H
#define GUCEF_ASSETSTORE_CASSETREPROCESSGUARD_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */
#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */
#ifndef GUCEF_ASSETSTORE_ETYPES_H
#include "gucefASSETSTORE_ETypes.h"
#define GUCEF_ASSETSTORE_ETYPES_H
#endif /* GUCEF_ASSETSTORE_ETYPES_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETREPROCESSSETTINGS_H
#include "gucefASSETSTORE_CAssetReprocessSettings.h"
#define GUCEF_ASSETSTORE_CASSETREPROCESSSETTINGS_H
#endif /* GUCEF_ASSETSTORE_CASSETREPROCESSSETTINGS_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETREPROCESSBUDGET_H
#include "gucefASSETSTORE_CAssetReprocessBudget.h"
#define GUCEF_ASSETSTORE_CASSETREPROCESSBUDGET_H
#endif /* GUCEF_ASSETSTORE_CASSETREPROCESSBUDGET_H ? */
#ifndef GUCEF_ASSETSTORE_MACROS_H
#include "gucefASSETSTORE_macros.h"
#define GUCEF_ASSETSTORE_MACROS_H
#endif /* GUCEF_ASSETSTORE_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The admission of reprocessing jobs (CAssetReprocessRequest) by the limits of CAssetReprocessSettings: Admit() before a job is
 *  queued (busy when too many wait, the hourly limits of machine callers per pack and together), TryStart() and Finish() around
 *  the run, AdmitRedownload() before a download again by a machine (the daily limit). The ledger of the starts is kept in memory.
 *
 *  TryStart() admits a job to run when a slot of the jobs at a time is free (CAssetReprocessBudget::concurrentJobs), when its
 *  estimated memory fits what the running jobs leave of the memory budget (a job estimated at more than the whole budget runs
 *  alone: only when no other job runs, and no other job starts beside it), and when fewer than maxConcurrentWritesPerLocation
 *  running jobs write to its storage location. The job gets a slot number (1 for the first free slot) and its share of the
 *  converter threads. Configure() replaces the settings at any time: the jobs that run keep the share they were given, the
 *  next TryStart() uses the new values. Thread safe.
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CAssetReprocessGuard
{
    public:

    /** What TryStart() decided */
    enum EStartOutcome
    {
        START_OK = 0            ,   /**< the job runs: it holds a slot until Finish() */
        START_WAIT_SLOT         ,   /**< all slots of the jobs at a time are taken */
        START_WAIT_MEMORY       ,   /**< its estimated memory does not fit the budget now */
        START_WAIT_LOCATION         /**< maxConcurrentWritesPerLocation jobs write to its storage location */
    };

    CAssetReprocessGuard( void );
    ~CAssetReprocessGuard();

    /** Replaces the settings; the budget is computed again with the facts of the machine (detected once, or given) */
    void Configure( const CAssetReprocessSettings& settings );
    CAssetReprocessSettings GetSettings( void ) const;

    /** Gives the facts of the machine in the place of the detected ones (the tests give fake ones) and computes the budget again */
    void SetMachineFacts( UInt32 logicalCores, UInt64 memoryLimitBytes );

    /** The jobs at a time, the threads per job and the memory budget in effect */
    CAssetReprocessBudget GetBudget( void ) const;

    /**
     *  Whether a job of the pack may be queued now; on true the start is counted. 'queued' is the number of reprocessing jobs that
     *  wait or run. On false 'status' is ASSETSTORE_STATUS_RATE_LIMITED (an hourly limit) or ASSETSTORE_STATUS_ALREADY_EXISTS (busy)
     *  and 'reason' says which limit and when the next start is allowed.
     */
    bool Admit( const CORE::CString& packId, bool machine, bool owner, UInt32 queued, Int64 nowMs, TAssetStoreStatus& status, CORE::CString& reason );

    /** Whether a machine may download an original again now; on true it is counted */
    bool AdmitRedownload( bool machine, Int64 nowMs, CORE::CString& reason );

    /** What Admit() would say, without counting: for the plan */
    bool WouldAdmit( const CORE::CString& packId, bool machine, bool owner, UInt32 queued, Int64 nowMs, CORE::CString& reason ) const;

    /**
     *  Admits a job to run now with its estimated memory (MiB, 0 when it converts nothing) and storage location: START_OK with its
     *  slot and its threads, else why it waits (in 'reason' in words)
     */
    EStartOutcome TryStart( UInt64 memoryMB, const CORE::CString& locationId, UInt32& slot, UInt32& threads, CORE::CString& reason );

    /** The job of the slot ended: its slot, memory and location are free */
    void Finish( UInt32 slot );

    UInt32 GetRunning( void ) const;
    UInt64 GetAdmittedMemoryMB( void ) const;
    UInt32 GetBusyThreads( void ) const;

    /** The budget and what the running jobs use of it, as attributes of 'node' */
    void Describe( CORE::CDataNode& node ) const;

    /** "waiting for slot", "waiting for memory", "waiting for location" */
    static const char* WaitText( EStartOutcome outcome );

    private:

    struct SStart
    {
        CORE::CString packId;
        Int64 atMs;
        bool machine;
        bool redownload;
    };
    typedef GUCEF::vector< SStart > TStartVector;

    struct SRunning
    {
        UInt32 slot;
        UInt64 memoryMB;
        UInt32 threads;
        CORE::CString locationId;
    };
    typedef GUCEF::vector< SRunning > TRunningVector;

    bool Check( const CORE::CString& packId, bool machine, bool owner, UInt32 queued, Int64 nowMs, TAssetStoreStatus& status, CORE::CString& reason ) const;
    void Trim( Int64 nowMs );
    void ComputeBudget( void );
    static CORE::CString DescribeWait( Int64 waitMs );

    mutable MT::CMutex m_lock;
    CAssetReprocessSettings m_settings;
    CAssetReprocessBudget m_budget;
    bool m_factsKnown;
    UInt32 m_logicalCores;
    UInt64 m_memoryLimitBytes;
    CORE::CString m_memoryLimitSource;
    TStartVector m_starts;
    TRunningVector m_running;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETSTORE_CASSETREPROCESSGUARD_H ? */
