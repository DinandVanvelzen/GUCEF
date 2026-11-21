/*
 *  AssetWorkbenchLib: the remote transfers of the workbench with their feedback
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

#ifndef GUCEF_CORE_CLONEABLES_H
#include "cloneables.h"
#define GUCEF_CORE_CLONEABLES_H
#endif /* GUCEF_CORE_CLONEABLES_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "AssetWorkbenchLib_CRemoteFetchJobs.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETWORKBENCH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CEvent CRemoteFetchJobs::JobEndedEvent = "GUCEF::ASSETWORKBENCH::CRemoteFetchJobs::JobEndedEvent";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CRemoteFetchJobs::SEnded::SEnded( void )
    : id( 0 ), kind(), name(), openPath(), status( ASSETSTORE::ASSETSTORE_STATUS_OK ), cancelled( false ), message(), bytes( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CRemoteFetchJobs::SJob::SJob( void )
    : id( 0 ), transfer(), future( CORE::TASKSTATUS_UNDEFINED ), kind(), name(), openPath(), activityId( 0 ), jobNumber( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CRemoteFetchJobs::RegisterEvents( void )
{GUCEF_TRACE;

    JobEndedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CRemoteFetchJobs::CRemoteFetchJobs( GUI::CActivityService* activity, CWorkbenchJobRegistry* registry )
    : CORE::CObservingNotifier()
    , m_activity( activity )
    , m_registry( registry )
    , m_system( GUCEF_NULL )
    , m_jobs()
    , m_nextId( 1 )
    , m_lastEnded()
{GUCEF_TRACE;

    RegisterEvents();
    if ( GUCEF_NULL != m_activity )
    {
        TEventCallback callback( this, &CRemoteFetchJobs::OnCancelRequested );
        SubscribeTo( m_activity, GUI::CActivityService::CancelRequestedEvent, callback );
    }
}

/*-------------------------------------------------------------------------*/

CRemoteFetchJobs::~CRemoteFetchJobs()
{GUCEF_TRACE;

    CancelAll();
    // the transfers end at their next look at the cancel flag; the file system waits for them when it closes
    for ( size_t i=0; i<m_jobs.size(); ++i )
    {
        if ( GUCEF_NULL != m_activity && 0 != m_jobs[ i ].activityId )
            m_activity->End( m_jobs[ i ].activityId );
        if ( GUCEF_NULL != m_registry && 0 != m_jobs[ i ].jobNumber )
            m_registry->End( m_jobs[ i ].jobNumber, ASSETSTORE::ASSETJOB_STATE_CANCELLED, "Cancelled" );
    }
    m_jobs.clear();
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CRemoteFetchJobs::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::ASSETWORKBENCH::CRemoteFetchJobs";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

void
CRemoteFetchJobs::SetFileSystem( ASSETSTORE::CRemoteAssetShares* system )
{GUCEF_TRACE;

    if ( system != m_system )
        CancelAll();
    m_system = system;
}

/*-------------------------------------------------------------------------*/

UInt32
CRemoteFetchJobs::Start( const ASSETSTORE::TRemoteFileTransferPtr& transfer, const CORE::CString& kind, const CORE::CString& label, const CORE::CString& openPath, UInt32 expectedDurationInMs )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_system || transfer.IsNULL() )
        return 0;
    SJob job;
    job.id = m_nextId++;
    job.transfer = transfer;
    job.kind = kind;
    job.name = label;
    job.openPath = openPath;
    if ( GUCEF_NULL != m_activity )
        job.activityId = m_activity->Begin( label, expectedDurationInMs, true, true );
    if ( GUCEF_NULL != m_registry )
    {
        job.jobNumber = m_registry->Begin( kind, "remote", label, label, true, expectedDurationInMs );
        if ( 0 != job.activityId )
            m_registry->LinkActivity( job.jobNumber, job.activityId );
    }
    job.future = m_system->StartTransfer( transfer );
    if ( !job.future.HasAFuture() )
    {
        if ( GUCEF_NULL != m_activity && 0 != job.activityId )
            m_activity->End( job.activityId );
        if ( GUCEF_NULL != m_registry && 0 != job.jobNumber )
            m_registry->End( job.jobNumber, ASSETSTORE::ASSETJOB_STATE_FAILED, "The transfer could not be started" );
        return 0;
    }
    m_jobs.push_back( job );
    return job.id;
}

/*-------------------------------------------------------------------------*/

void
CRemoteFetchJobs::Update( void )
{GUCEF_TRACE;

    for ( size_t i=0; i<m_jobs.size(); )
    {
        SJob& job = m_jobs[ i ];
        const UInt64 total = job.transfer->GetBytesTotal();
        const UInt64 done = job.transfer->GetBytesDone();
        const Float32 fraction = total > 0 ? (Float32) ( (double) done / (double) total ) : 0.0f;
        if ( !job.transfer->IsEnded() )
        {
            if ( GUCEF_NULL != m_activity && 0 != job.activityId )
                m_activity->SetProgress( job.activityId, fraction > 1.0f ? 1.0f : fraction );
            if ( GUCEF_NULL != m_registry && 0 != job.jobNumber )
            {
                m_registry->SetProgress( job.jobNumber, (UInt32) ( done >> 10 ), (UInt32) ( total >> 10 ) );
                m_registry->SetNote( job.jobNumber, job.transfer->GetCurrentFile() );
            }
            ++i;
            continue;
        }
        SEnded ended;
        ended.id = job.id;
        ended.kind = job.kind;
        ended.name = job.name;
        ended.openPath = job.openPath;
        ended.status = job.transfer->GetStatus();
        ended.cancelled = ASSETSTORE::ASSETSTORE_STATUS_CANCELLED == ended.status;
        ended.message = job.transfer->GetMessage();
        ended.bytes = done;
        if ( GUCEF_NULL != m_activity && 0 != job.activityId )
            m_activity->End( job.activityId );
        if ( GUCEF_NULL != m_registry && 0 != job.jobNumber )
        {
            m_registry->SetProgress( job.jobNumber, (UInt32) ( done >> 10 ), (UInt32) ( total >> 10 ) );
            m_registry->End( job.jobNumber, ASSETSTORE::ASSETSTORE_STATUS_OK == ended.status ? ASSETSTORE::ASSETJOB_STATE_DONE :
                             ( ended.cancelled ? ASSETSTORE::ASSETJOB_STATE_CANCELLED : ASSETSTORE::ASSETJOB_STATE_FAILED ), ended.message );
        }
        m_jobs.erase( m_jobs.begin() + i );
        m_lastEnded = ended;
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "CRemoteFetchJobs: " << ended.kind << " \"" << ended.name << "\" ended: " << ended.message );
        TEndedEventData data( ended );
        NotifyObservers( JobEndedEvent, &data );
    }
}

/*-------------------------------------------------------------------------*/

bool
CRemoteFetchJobs::Cancel( UInt32 id )
{GUCEF_TRACE;

    for ( size_t i=0; i<m_jobs.size(); ++i )
    {
        if ( m_jobs[ i ].id == id )
        {
            m_jobs[ i ].transfer->Cancel();
            return true;
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

void
CRemoteFetchJobs::CancelAll( void )
{GUCEF_TRACE;

    for ( size_t i=0; i<m_jobs.size(); ++i )
        m_jobs[ i ].transfer->Cancel();
}

/*-------------------------------------------------------------------------*/

UInt32
CRemoteFetchJobs::GetActiveCount( void ) const
{
    return (UInt32) m_jobs.size();
}

/*-------------------------------------------------------------------------*/

Int32
CRemoteFetchJobs::GetProgress( UInt32 id ) const
{GUCEF_TRACE;

    for ( size_t i=0; i<m_jobs.size(); ++i )
    {
        if ( m_jobs[ i ].id == id )
        {
            const UInt64 total = m_jobs[ i ].transfer->GetBytesTotal();
            return total > 0 ? (Int32) ( ( m_jobs[ i ].transfer->GetBytesDone() * 1000 ) / total ) : 0;
        }
    }
    return -1;
}

/*-------------------------------------------------------------------------*/

ASSETSTORE::TRemoteFileTransferPtr
CRemoteFetchJobs::GetTransfer( UInt32 id ) const
{GUCEF_TRACE;

    for ( size_t i=0; i<m_jobs.size(); ++i )
        if ( m_jobs[ i ].id == id )
            return m_jobs[ i ].transfer;
    return ASSETSTORE::TRemoteFileTransferPtr();
}

/*-------------------------------------------------------------------------*/

const CRemoteFetchJobs::SEnded&
CRemoteFetchJobs::GetLastEnded( void ) const
{
    return m_lastEnded;
}

/*-------------------------------------------------------------------------*/

void
CRemoteFetchJobs::OnCancelRequested( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    const CORE::TCloneableUInt32* id = static_cast< const CORE::TCloneableUInt32* >( eventData );
    if ( GUCEF_NULL == id )
        return;
    for ( size_t i=0; i<m_jobs.size(); ++i )
        if ( m_jobs[ i ].activityId == id->GetData() )
            m_jobs[ i ].transfer->Cancel();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETWORKBENCH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
