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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */
#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#include "gucefASSETSTORE_CAssetReprocessGuard.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CAssetReprocessGuard::CAssetReprocessGuard( void )
    : m_lock()
    , m_settings()
    , m_budget()
    , m_factsKnown( false )
    , m_logicalCores( 1 )
    , m_memoryLimitBytes( 0 )
    , m_memoryLimitSource()
    , m_starts()
    , m_running()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetReprocessGuard::~CAssetReprocessGuard()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CAssetReprocessGuard::ComputeBudget( void )
{GUCEF_TRACE;

    if ( !m_factsKnown )
    {
        m_logicalCores = CAssetReprocessBudget::DetectLogicalCores();
        m_memoryLimitBytes = CAssetReprocessBudget::DetectMemoryLimitBytes( m_memoryLimitSource );
        m_factsKnown = true;
    }
    m_budget.Compute( m_settings, m_logicalCores, m_memoryLimitBytes, m_memoryLimitSource );
}

/*-------------------------------------------------------------------------*/

void
CAssetReprocessGuard::Configure( const CAssetReprocessSettings& settings )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_settings = settings;
    ComputeBudget();
}

/*-------------------------------------------------------------------------*/

void
CAssetReprocessGuard::SetMachineFacts( UInt32 logicalCores, UInt64 memoryLimitBytes )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_logicalCores = logicalCores > 0 ? logicalCores : 1;
    m_memoryLimitBytes = memoryLimitBytes;
    m_memoryLimitSource = "given";
    m_factsKnown = true;
    ComputeBudget();
}

/*-------------------------------------------------------------------------*/

CAssetReprocessSettings
CAssetReprocessGuard::GetSettings( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_settings;
}

/*-------------------------------------------------------------------------*/

CAssetReprocessBudget
CAssetReprocessGuard::GetBudget( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_budget;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetReprocessGuard::DescribeWait( Int64 waitMs )
{GUCEF_TRACE;

    const Int64 minutes = ( waitMs + 59999 ) / 60000;
    return "in " + CORE::ToString( (UInt64) ( minutes > 0 ? minutes : 1 ) ) + " minute(s)";
}

/*-------------------------------------------------------------------------*/

void
CAssetReprocessGuard::Trim( Int64 nowMs )
{GUCEF_TRACE;

    const Int64 dayMs = 24 * 3600 * 1000LL;
    TStartVector kept;
    for ( size_t i=0; i<m_starts.size(); ++i )
        if ( nowMs - m_starts[ i ].atMs < dayMs )
            kept.push_back( m_starts[ i ] );
    m_starts = kept;
}

/*-------------------------------------------------------------------------*/

bool
CAssetReprocessGuard::Check( const CORE::CString& packId, bool machine, bool owner, UInt32 queued, Int64 nowMs, TAssetStoreStatus& status, CORE::CString& reason ) const
{GUCEF_TRACE;

    if ( !m_settings.enabled )
    {
        status = ASSETSTORE_STATUS_UNSUPPORTED;
        reason = "the reprocessing of packs is switched off in the settings of the service";
        return false;
    }
    if ( m_settings.maxQueuedJobs > 0 && queued >= m_settings.maxQueuedJobs )
    {
        status = ASSETSTORE_STATUS_ALREADY_EXISTS;
        reason = "busy: " + CORE::ToString( queued ) + " reprocessing jobs wait or run (maxQueuedJobs " + CORE::ToString( m_settings.maxQueuedJobs ) + "); try again when one ended";
        return false;
    }
    const bool limited = machine || m_settings.limitOwner;
    if ( !limited )
        return true;
    const Int64 hourMs = 3600 * 1000LL;
    UInt32 ofPack = 0;
    UInt32 ofMachines = 0;
    Int64 oldestOfPack = nowMs;
    Int64 oldestOfMachines = nowMs;
    for ( size_t i=0; i<m_starts.size(); ++i )
    {
        const SStart& start = m_starts[ i ];
        if ( start.redownload || nowMs - start.atMs >= hourMs )
            continue;
        if ( start.packId == packId && ( start.machine || m_settings.limitOwner ) )
        {
            ++ofPack;
            if ( start.atMs < oldestOfPack )
                oldestOfPack = start.atMs;
        }
        if ( start.machine )
        {
            ++ofMachines;
            if ( start.atMs < oldestOfMachines )
                oldestOfMachines = start.atMs;
        }
    }
    if ( m_settings.maxJobsPerPackPerHour > 0 && ofPack >= m_settings.maxJobsPerPackPerHour )
    {
        status = ASSETSTORE_STATUS_RATE_LIMITED;
        reason = "rate limited: " + CORE::ToString( ofPack ) + " reprocessing jobs of pack " + packId + " in the last hour (maxJobsPerPackPerHour " +
                 CORE::ToString( m_settings.maxJobsPerPackPerHour ) + "); the next one is allowed " + DescribeWait( oldestOfPack + hourMs - nowMs );
        return false;
    }
    if ( machine && m_settings.maxMachineJobsPerHour > 0 && ofMachines >= m_settings.maxMachineJobsPerHour )
    {
        status = ASSETSTORE_STATUS_RATE_LIMITED;
        reason = "rate limited: " + CORE::ToString( ofMachines ) + " reprocessing jobs of machine callers in the last hour (maxMachineJobsPerHour " +
                 CORE::ToString( m_settings.maxMachineJobsPerHour ) + "); the next one is allowed " + DescribeWait( oldestOfMachines + hourMs - nowMs );
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAssetReprocessGuard::Admit( const CORE::CString& packId, bool machine, bool owner, UInt32 queued, Int64 nowMs, TAssetStoreStatus& status, CORE::CString& reason )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    Trim( nowMs );
    status = ASSETSTORE_STATUS_OK;
    if ( !Check( packId, machine, owner, queued, nowMs, status, reason ) )
        return false;
    SStart start;
    start.packId = packId;
    start.atMs = nowMs;
    start.machine = machine;
    start.redownload = false;
    m_starts.push_back( start );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAssetReprocessGuard::WouldAdmit( const CORE::CString& packId, bool machine, bool owner, UInt32 queued, Int64 nowMs, CORE::CString& reason ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    TAssetStoreStatus status = ASSETSTORE_STATUS_OK;
    return Check( packId, machine, owner, queued, nowMs, status, reason );
}

/*-------------------------------------------------------------------------*/

bool
CAssetReprocessGuard::AdmitRedownload( bool machine, Int64 nowMs, CORE::CString& reason )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    Trim( nowMs );
    if ( machine && m_settings.maxMachineRedownloadsPerDay > 0 )
    {
        UInt32 count = 0;
        Int64 oldest = nowMs;
        for ( size_t i=0; i<m_starts.size(); ++i )
        {
            if ( m_starts[ i ].redownload && m_starts[ i ].machine )
            {
                ++count;
                if ( m_starts[ i ].atMs < oldest )
                    oldest = m_starts[ i ].atMs;
            }
        }
        if ( count >= m_settings.maxMachineRedownloadsPerDay )
        {
            reason = "rate limited: " + CORE::ToString( count ) + " downloads again by machine callers in the last day (maxMachineRedownloadsPerDay " +
                     CORE::ToString( m_settings.maxMachineRedownloadsPerDay ) + "); the next one is allowed " + DescribeWait( oldest + 24 * 3600 * 1000LL - nowMs );
            return false;
        }
    }
    SStart start;
    start.atMs = nowMs;
    start.machine = machine;
    start.redownload = true;
    m_starts.push_back( start );
    return true;
}

/*-------------------------------------------------------------------------*/

CAssetReprocessGuard::EStartOutcome
CAssetReprocessGuard::TryStart( UInt64 memoryMB, const CORE::CString& locationId, UInt32& slot, UInt32& threads, CORE::CString& reason )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    slot = 0;
    threads = 0;
    reason.Clear();
    const UInt32 running = (UInt32) m_running.size();
    if ( running >= m_budget.concurrentJobs )
    {
        reason = "waiting for slot: " + CORE::ToString( running ) + " reprocessing job(s) run, " + CORE::ToString( m_budget.concurrentJobs ) +
                 " at a time (maxConcurrentJobs " + ( m_budget.jobsAutomatic ? CORE::CString( "automatic" ) : CORE::ToString( m_settings.maxConcurrentJobs ) ) + ")";
        return START_WAIT_SLOT;
    }
    UInt64 admitted = 0;
    for ( size_t r=0; r<m_running.size(); ++r )
        admitted += m_running[ r ].memoryMB;
    if ( m_budget.memoryBudgetMB > 0 && memoryMB > 0 && running > 0 && admitted + memoryMB > m_budget.memoryBudgetMB )
    {
        if ( memoryMB > m_budget.memoryBudgetMB )
            reason = "waiting for memory: the job is estimated at " + CORE::ToString( memoryMB ) + " MB, more than the memory budget of " +
                     CORE::ToString( m_budget.memoryBudgetMB ) + " MB: it runs alone once the " + CORE::ToString( running ) + " running job(s) ended";
        else
            reason = "waiting for memory: the job is estimated at " + CORE::ToString( memoryMB ) + " MB, the running job(s) hold " + CORE::ToString( admitted ) +
                     " MB of the memory budget of " + CORE::ToString( m_budget.memoryBudgetMB ) + " MB";
        return START_WAIT_MEMORY;
    }
    if ( m_settings.maxConcurrentWritesPerLocation > 0 && !locationId.IsNULLOrEmpty() )
    {
        UInt32 writers = 0;
        for ( size_t r=0; r<m_running.size(); ++r )
            if ( m_running[ r ].locationId == locationId )
                ++writers;
        if ( writers >= m_settings.maxConcurrentWritesPerLocation )
        {
            reason = "waiting for location: " + CORE::ToString( writers ) + " running job(s) write to the storage location " + locationId +
                     " (maxConcurrentWritesPerLocation " + CORE::ToString( m_settings.maxConcurrentWritesPerLocation ) + ")";
            return START_WAIT_LOCATION;
        }
    }
    // the first free slot number: the slot names the thread pool of the job, so the pools are reused
    UInt32 free = 1;
    bool taken = true;
    while ( taken )
    {
        taken = false;
        for ( size_t r=0; r<m_running.size() && !taken; ++r )
            taken = m_running[ r ].slot == free;
        if ( taken )
            ++free;
    }
    SRunning entry;
    entry.slot = free;
    entry.memoryMB = memoryMB;
    entry.threads = m_budget.threadsPerJob;
    entry.locationId = locationId;
    m_running.push_back( entry );
    slot = entry.slot;
    threads = entry.threads;
    return START_OK;
}

/*-------------------------------------------------------------------------*/

void
CAssetReprocessGuard::Finish( UInt32 slot )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    for ( size_t r=0; r<m_running.size(); ++r )
        if ( m_running[ r ].slot == slot )
        {
            m_running.erase( m_running.begin() + r );
            return;
        }
}

/*-------------------------------------------------------------------------*/

UInt32
CAssetReprocessGuard::GetRunning( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return (UInt32) m_running.size();
}

/*-------------------------------------------------------------------------*/

UInt64
CAssetReprocessGuard::GetAdmittedMemoryMB( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    UInt64 admitted = 0;
    for ( size_t r=0; r<m_running.size(); ++r )
        admitted += m_running[ r ].memoryMB;
    return admitted;
}

/*-------------------------------------------------------------------------*/

UInt32
CAssetReprocessGuard::GetBusyThreads( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    UInt32 threads = 0;
    for ( size_t r=0; r<m_running.size(); ++r )
        threads += m_running[ r ].threads;
    return threads;
}

/*-------------------------------------------------------------------------*/

void
CAssetReprocessGuard::Describe( CORE::CDataNode& node ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_budget.Describe( node );
    UInt64 admitted = 0;
    UInt32 threads = 0;
    for ( size_t r=0; r<m_running.size(); ++r )
    {
        admitted += m_running[ r ].memoryMB;
        threads += m_running[ r ].threads;
    }
    node.SetAttribute( "runningJobs", (UInt32) m_running.size() );
    node.SetAttribute( "busyThreads", threads );
    node.SetAttribute( "admittedMemoryMB", admitted );
    node.SetAttribute( "maxConcurrentWritesPerLocation", m_settings.maxConcurrentWritesPerLocation );
}

/*-------------------------------------------------------------------------*/

const char*
CAssetReprocessGuard::WaitText( EStartOutcome outcome )
{GUCEF_TRACE;

    switch ( outcome )
    {
        case START_WAIT_SLOT: return "waiting for slot";
        case START_WAIT_MEMORY: return "waiting for memory";
        case START_WAIT_LOCATION: return "waiting for location";
        default: return "running";
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
