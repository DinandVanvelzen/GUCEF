/*
 *  AssetWorkbenchLib: views and controllers of the asset workbench
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

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_CDATETIME_H
#include "gucefCORE_CDateTime.h"
#define GUCEF_CORE_CDATETIME_H
#endif /* GUCEF_CORE_CDATETIME_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef ASSETWORKBENCHLIB_CWORKBENCHJOBREGISTRY_H
#include "AssetWorkbenchLib_CWorkbenchJobRegistry.h"
#define ASSETWORKBENCHLIB_CWORKBENCHJOBREGISTRY_H
#endif /* ASSETWORKBENCHLIB_CWORKBENCHJOBREGISTRY_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETWORKBENCH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CEvent CWorkbenchJobRegistry::JobsChangedEvent = "GUCEF::ASSETWORKBENCH::CWorkbenchJobRegistry::JobsChangedEvent";
const CORE::CString CWorkbenchJobRegistry::IdPrefix = "local:";

namespace {

UInt64
PrecisionClock( void* userData )
{
    return MT::PrecisionTickCount() * 1000 / MT::PrecisionTimerResolution();
}

}

/*-------------------------------------------------------------------------*/

void
CWorkbenchJobRegistry::RegisterEvents( void )
{GUCEF_TRACE;

    JobsChangedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CWorkbenchJobRegistry::SJob::SJob( void )
    : number( 0 )
    , activityId( 0 )
    , kind()
    , subKind()
    , name()
    , scope()
    , note()
    , resultText()
    , cancellable( false )
    , cancelRequested( false )
    , ended( false )
    , visible( false )
    , announcedLong( false )
    , state( ASSETSTORE::ASSETJOB_STATE_RUNNING )
    , done( 0 )
    , total( 0 )
    , startedMs( 0 )
    , endedMs( 0 )
    , startedWallMs( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CWorkbenchJobRegistry::CWorkbenchJobRegistry( void )
    : CORE::CObservingNotifier()
    , m_jobs()
    , m_nextNumber( 1 )
    , m_visibleAfterMs( (UInt32) DefaultVisibleAfterMs )
    , m_clock( &PrecisionClock )
    , m_clockData( GUCEF_NULL )
    , m_activity( GUCEF_NULL )
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CWorkbenchJobRegistry::~CWorkbenchJobRegistry()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CWorkbenchJobRegistry::SetClock( TClockFunction clock, void* userData )
{GUCEF_TRACE;

    m_clock = GUCEF_NULL != clock ? clock : &PrecisionClock;
    m_clockData = GUCEF_NULL != clock ? userData : GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchJobRegistry::SetVisibleAfterMs( UInt32 visibleAfterMs )
{GUCEF_TRACE;

    m_visibleAfterMs = visibleAfterMs;
}

/*-------------------------------------------------------------------------*/

UInt32
CWorkbenchJobRegistry::GetVisibleAfterMs( void ) const
{GUCEF_TRACE;

    return m_visibleAfterMs;
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchJobRegistry::SetActivityService( GUI::CActivityService* activity )
{GUCEF_TRACE;

    m_activity = activity;
}

/*-------------------------------------------------------------------------*/

UInt64
CWorkbenchJobRegistry::Now( void ) const
{GUCEF_TRACE;

    return m_clock( m_clockData );
}

/*-------------------------------------------------------------------------*/

CWorkbenchJobRegistry::SJob*
CWorkbenchJobRegistry::Find( UInt32 number )
{GUCEF_TRACE;

    for ( size_t i=0; i<m_jobs.size(); ++i )
        if ( m_jobs[ i ].number == number )
            return &m_jobs[ i ];
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

const CWorkbenchJobRegistry::SJob*
CWorkbenchJobRegistry::Find( UInt32 number ) const
{GUCEF_TRACE;

    for ( size_t i=0; i<m_jobs.size(); ++i )
        if ( m_jobs[ i ].number == number )
            return &m_jobs[ i ];
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

UInt32
CWorkbenchJobRegistry::Begin( const CORE::CString& kind, const CORE::CString& subKind, const CORE::CString& name, const CORE::CString& scope,
                              bool cancellable, UInt32 expectedDurationInMs )
{GUCEF_TRACE;

    SJob job;
    job.number = m_nextNumber++;
    if ( 0 == m_nextNumber )
        m_nextNumber = 1;
    job.kind = kind;
    job.subKind = subKind;
    job.name = name;
    job.scope = scope;
    job.cancellable = cancellable;
    job.announcedLong = expectedDurationInMs > m_visibleAfterMs;
    job.startedMs = Now();
    job.startedWallMs = (Int64) CORE::CDateTime::NowUTCDateTime().ToUnixEpochBasedTicksInMillisecs();
    m_jobs.push_back( job );
    if ( job.announcedLong )
        NotifyObservers( JobsChangedEvent );
    return job.number;
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchJobRegistry::LinkActivity( UInt32 number, UInt32 activityId )
{GUCEF_TRACE;

    SJob* job = Find( number );
    if ( GUCEF_NULL != job )
        job->activityId = activityId;
}

/*-------------------------------------------------------------------------*/

UInt32
CWorkbenchJobRegistry::FindByActivity( UInt32 activityId ) const
{GUCEF_TRACE;

    if ( 0 == activityId )
        return 0;
    for ( size_t i=0; i<m_jobs.size(); ++i )
        if ( m_jobs[ i ].activityId == activityId && IsVisible( m_jobs[ i ] ) )
            return m_jobs[ i ].number;
    return 0;
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchJobRegistry::SetProgress( UInt32 number, UInt32 done, UInt32 total )
{GUCEF_TRACE;

    SJob* job = Find( number );
    if ( GUCEF_NULL == job || job->ended )
        return;
    job->done = done;
    job->total = total;
    if ( IsVisible( *job ) )
        NotifyObservers( JobsChangedEvent );
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchJobRegistry::SetNote( UInt32 number, const CORE::CString& note )
{GUCEF_TRACE;

    SJob* job = Find( number );
    if ( GUCEF_NULL != job )
        job->note = note;
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchJobRegistry::IsVisible( const SJob& job ) const
{GUCEF_TRACE;

    if ( job.announcedLong )
        return true;
    const UInt64 until = job.ended ? job.endedMs : Now();
    return until >= job.startedMs && until - job.startedMs >= m_visibleAfterMs;
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchJobRegistry::End( UInt32 number, ASSETSTORE::TAssetJobState state, const CORE::CString& resultText )
{GUCEF_TRACE;

    SJob* job = Find( number );
    if ( GUCEF_NULL == job || job->ended )
        return;
    job->ended = true;
    job->endedMs = Now();
    job->state = ASSETSTORE::AssetJobStateIsFinal( state ) ? state : ASSETSTORE::ASSETJOB_STATE_DONE;
    job->resultText = resultText;
    if ( !IsVisible( *job ) )
    {
        // it was quick: it leaves no trace
        for ( size_t i=0; i<m_jobs.size(); ++i )
            if ( m_jobs[ i ].number == number )
            {
                m_jobs.erase( m_jobs.begin() + i );
                break;
            }
        return;
    }
    TrimEnded();
    NotifyObservers( JobsChangedEvent );
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchJobRegistry::TrimEnded( void )
{GUCEF_TRACE;

    UInt32 ended = 0;
    for ( size_t i=0; i<m_jobs.size(); ++i )
        if ( m_jobs[ i ].ended )
            ++ended;
    for ( size_t i=0; i<m_jobs.size() && ended > (UInt32) MaxEndedJobs; )
    {
        if ( m_jobs[ i ].ended )
        {
            m_jobs.erase( m_jobs.begin() + i );
            --ended;
        }
        else
            ++i;
    }
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchJobRegistry::RequestCancel( UInt32 number )
{GUCEF_TRACE;

    SJob* job = Find( number );
    if ( GUCEF_NULL == job || job->ended || !job->cancellable )
        return false;
    job->cancelRequested = true;
    if ( GUCEF_NULL != m_activity && 0 != job->activityId )
        m_activity->RequestCancel( job->activityId );
    NotifyObservers( JobsChangedEvent );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchJobRegistry::IsCancelRequested( UInt32 number ) const
{GUCEF_TRACE;

    const SJob* job = Find( number );
    return GUCEF_NULL != job && job->cancelRequested;
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchJobRegistry::MakeEntry( const SJob& job, ASSETSTORE::CAssetJobEntry& entry ) const
{GUCEF_TRACE;

    entry.Clear();
    entry.jobId = IdPrefix + CORE::UInt32ToString( job.number );
    entry.kind = job.kind;
    entry.subKind = job.subKind;
    entry.name = job.name;
    entry.scope = job.scope;
    entry.detail = job.note;
    entry.state = job.ended ? job.state : ASSETSTORE::ASSETJOB_STATE_RUNNING;
    entry.resultText = job.resultText;
    entry.itemsDone = job.done;
    entry.itemsTotal = job.total;
    entry.fraction = ASSETSTORE::ASSETJOB_STATE_DONE == entry.state ? 1.0f : ( job.total > 0 ? (Float32) job.done / (Float32) job.total : -1.0f );
    if ( !job.ended && GUCEF_NULL != m_activity && 0 != job.activityId )
    {
        GUI::CActivityInfo info;
        if ( m_activity->GetOperation( job.activityId, info ) && info.determinate )
            entry.fraction = info.progress;
    }
    if ( job.cancelRequested && !job.ended )
        entry.stateText = "cancelling";
    entry.createdAtMs = job.startedWallMs;
    entry.updatedAtMs = job.startedWallMs + (Int64) ( ( job.ended ? job.endedMs : Now() ) - job.startedMs );
    entry.message = ASSETSTORE::ASSETJOB_STATE_FAILED == entry.state ? job.resultText : CORE::CString::Empty;
    entry.SetActionAvailable( ASSETSTORE::ASSETJOB_ACTION_CANCEL, !job.ended && job.cancellable && !job.cancelRequested );
    entry.SetActionAvailable( ASSETSTORE::ASSETJOB_ACTION_REMOVE, job.ended );
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchJobRegistry::GetJobs( ASSETSTORE::TAssetJobEntryVector& jobs ) const
{GUCEF_TRACE;

    for ( size_t i=0; i<m_jobs.size(); ++i )
    {
        if ( !IsVisible( m_jobs[ i ] ) )
            continue;
        ASSETSTORE::CAssetJobEntry entry;
        MakeEntry( m_jobs[ i ], entry );
        jobs.push_back( entry );
    }
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchJobRegistry::GetJob( const CORE::CString& jobId, ASSETSTORE::CAssetJobEntry& job ) const
{GUCEF_TRACE;

    const UInt32 number = NumberOfId( jobId );
    const SJob* found = 0 == number ? GUCEF_NULL : Find( number );
    if ( GUCEF_NULL == found || !IsVisible( *found ) )
        return false;
    MakeEntry( *found, job );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchJobRegistry::Remove( const CORE::CString& jobId )
{GUCEF_TRACE;

    const UInt32 number = NumberOfId( jobId );
    for ( size_t i=0; i<m_jobs.size() && 0 != number; ++i )
    {
        if ( m_jobs[ i ].number == number && m_jobs[ i ].ended )
        {
            m_jobs.erase( m_jobs.begin() + i );
            NotifyObservers( JobsChangedEvent );
            return true;
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

UInt32
CWorkbenchJobRegistry::NumberOfId( const CORE::CString& jobId )
{GUCEF_TRACE;

    if ( !IsLocalId( jobId ) )
        return 0;
    return CORE::StringToUInt32( jobId.CutChars( IdPrefix.Length(), true ), 0 );
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchJobRegistry::IsLocalId( const CORE::CString& jobId )
{GUCEF_TRACE;

    return 0 == jobId.HasSubstr( IdPrefix );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CWorkbenchJobRegistry::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::ASSETWORKBENCH::CWorkbenchJobRegistry";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETWORKBENCH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
