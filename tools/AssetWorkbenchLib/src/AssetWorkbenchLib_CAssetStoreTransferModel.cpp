/*
 *  AssetWorkbenchLib: the asset sources of the workbench: online stores, the primary store and the transfers
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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef ASSETWORKBENCHLIB_CASSETSTORETRANSFERMODEL_H
#include "AssetWorkbenchLib_CAssetStoreTransferModel.h"
#define ASSETWORKBENCHLIB_CASSETSTORETRANSFERMODEL_H
#endif /* ASSETWORKBENCHLIB_CASSETSTORETRANSFERMODEL_H ? */

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

const CORE::CString CAssetStoreTransferModel::IngestIdPrefix = "ingest:";
const CORE::CString CAssetStoreTransferModel::ConversionIdPrefix = "convert:";

namespace {

bool
ToJobAction( GUI::CITransferQueueModel::ETransferAction action, ASSETSTORE::TAssetJobAction& jobAction )
{
    switch ( action )
    {
        case GUI::CITransferQueueModel::TRANSFERACTION_PAUSE:  jobAction = ASSETSTORE::ASSETJOB_ACTION_PAUSE; return true;
        case GUI::CITransferQueueModel::TRANSFERACTION_RESUME: jobAction = ASSETSTORE::ASSETJOB_ACTION_RESUME; return true;
        case GUI::CITransferQueueModel::TRANSFERACTION_CANCEL: jobAction = ASSETSTORE::ASSETJOB_ACTION_CANCEL; return true;
        case GUI::CITransferQueueModel::TRANSFERACTION_RETRY:  jobAction = ASSETSTORE::ASSETJOB_ACTION_RETRY; return true;
        case GUI::CITransferQueueModel::TRANSFERACTION_REMOVE: jobAction = ASSETSTORE::ASSETJOB_ACTION_REMOVE; return true;
        default: return false;
    }
}

CORE::CString
KindLabel( const CORE::CString& kind )
{
    if ( "download" == kind )
        return "Download";
    if ( "ingest" == kind )
        return "Ingest";
    if ( "conversion" == kind )
        return "Conversion";
    if ( kind.IsNULLOrEmpty() )
        return kind;
    return kind.SubstrToIndex( 1, true ).Uppercase() + kind.CutChars( 1, true );
}

}

/*-------------------------------------------------------------------------*/

CAssetStoreTransferModel::CAssetStoreTransferModel( ASSETSTORE::CIAssetStoreService& service )
    : GUI::CITransferQueueModel()
    , m_service( service )
    , m_jobs()
    , m_jobsFuture( CORE::TASKSTATUS_UNDEFINED )
    , m_readAgain( false )
    , m_local( GUCEF_NULL )
{GUCEF_TRACE;

    TEventCallback changed( this, &CAssetStoreTransferModel::OnJobChanged );
    SubscribeTo( &m_service, ASSETSTORE::CIAssetStoreService::JobChangedEvent, changed );
    TEventCallback removed( this, &CAssetStoreTransferModel::OnJobRemoved );
    SubscribeTo( &m_service, ASSETSTORE::CIAssetStoreService::JobRemovedEvent, removed );
    if ( !m_service.IsInProcess() )
        RequestJobs();
}

/*-------------------------------------------------------------------------*/

CAssetStoreTransferModel::~CAssetStoreTransferModel()
{GUCEF_TRACE;

    UnsubscribeAllFromObserver();
}

/*-------------------------------------------------------------------------*/

void
CAssetStoreTransferModel::SetLocalJobs( CWorkbenchJobRegistry* local )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_local )
        UnsubscribeFrom( m_local, CWorkbenchJobRegistry::JobsChangedEvent );
    m_local = local;
    if ( GUCEF_NULL != m_local )
    {
        TEventCallback changed( this, &CAssetStoreTransferModel::OnLocalJobsChanged );
        SubscribeTo( m_local, CWorkbenchJobRegistry::JobsChangedEvent, changed );
    }
    NotifyObservers( EntriesChangedEvent );
}

/*-------------------------------------------------------------------------*/

CWorkbenchJobRegistry*
CAssetStoreTransferModel::GetLocalJobs( void ) const
{GUCEF_TRACE;

    return m_local;
}

/*-------------------------------------------------------------------------*/

void
CAssetStoreTransferModel::GetJobs( ASSETSTORE::TAssetJobEntryVector& jobs ) const
{GUCEF_TRACE;

    jobs.clear();
    if ( m_service.IsInProcess() )
        ReadJobs();
    jobs = m_jobs;
    if ( GUCEF_NULL != m_local )
        m_local->GetJobs( jobs );
}

/*-------------------------------------------------------------------------*/

bool
CAssetStoreTransferModel::GetJob( const CORE::CString& jobId, ASSETSTORE::CAssetJobEntry& job ) const
{GUCEF_TRACE;

    if ( CWorkbenchJobRegistry::IsLocalId( jobId ) )
        return GUCEF_NULL != m_local && m_local->GetJob( jobId, job );
    if ( m_service.IsInProcess() )
        return ASSETSTORE::ASSETSTORE_STATUS_OK == m_service.GetJob( jobId, job );
    for ( size_t i=0; i<m_jobs.size(); ++i )
    {
        if ( m_jobs[ i ].jobId == jobId )
        {
            job = m_jobs[ i ];
            return true;
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

CWorkbenchJobSummary
CAssetStoreTransferModel::GetSummary( void ) const
{GUCEF_TRACE;

    ASSETSTORE::TAssetJobEntryVector jobs;
    GetJobs( jobs );
    return CWorkbenchJobSummary::Count( jobs );
}

/*-------------------------------------------------------------------------*/

bool
CAssetStoreTransferModel::IsJobActionAvailable( const CORE::CString& jobId, ASSETSTORE::TAssetJobAction action ) const
{GUCEF_TRACE;

    ASSETSTORE::CAssetJobEntry job;
    return GetJob( jobId, job ) && job.IsActionAvailable( action );
}

/*-------------------------------------------------------------------------*/

bool
CAssetStoreTransferModel::ControlJob( const CORE::CString& jobId, ASSETSTORE::TAssetJobAction action )
{GUCEF_TRACE;

    if ( CWorkbenchJobRegistry::IsLocalId( jobId ) )
    {
        if ( GUCEF_NULL == m_local )
            return false;
        const UInt32 number = CWorkbenchJobRegistry::NumberOfId( jobId );
        if ( ASSETSTORE::ASSETJOB_ACTION_CANCEL == action )
            return m_local->RequestCancel( number );
        if ( ASSETSTORE::ASSETJOB_ACTION_REMOVE == action )
            return m_local->Remove( jobId );
        return false;
    }
    if ( !m_service.IsInProcess() )
    {
        // sent: the state follows from the events of the service
        return m_service.ControlJobAsync( CORE::CString::Empty, jobId, action ).HasAFuture();
    }
    ASSETSTORE::CAssetServiceResult result;
    return ASSETSTORE::ASSETSTORE_STATUS_OK == m_service.ControlJob( CORE::CString::Empty, jobId, action, result );
}

/*-------------------------------------------------------------------------*/

CAssetStoreTransferModel::ETransferState
CAssetStoreTransferModel::ToTransferState( ASSETSTORE::TAssetJobState state )
{GUCEF_TRACE;

    switch ( state )
    {
        case ASSETSTORE::ASSETJOB_STATE_QUEUED:    return TRANSFERSTATE_QUEUED;
        case ASSETSTORE::ASSETJOB_STATE_RUNNING:   return TRANSFERSTATE_RUNNING;
        case ASSETSTORE::ASSETJOB_STATE_PAUSED:    return TRANSFERSTATE_PAUSED;
        case ASSETSTORE::ASSETJOB_STATE_FINISHING: return TRANSFERSTATE_FINISHING;
        case ASSETSTORE::ASSETJOB_STATE_DONE:      return TRANSFERSTATE_DONE;
        case ASSETSTORE::ASSETJOB_STATE_FAILED:    return TRANSFERSTATE_FAILED;
        default:                                   return TRANSFERSTATE_CANCELLED;
    }
}

/*-------------------------------------------------------------------------*/

void
CAssetStoreTransferModel::ToEntry( const ASSETSTORE::CAssetJobEntry& job, TEntry& entry )
{GUCEF_TRACE;

    entry = TEntry();
    entry.id = job.jobId;
    entry.name = job.name;
    entry.kind = KindLabel( job.kind );
    entry.detail = job.detail;
    entry.totalBytes = job.totalBytes;
    entry.doneBytes = job.doneBytes;
    entry.fraction = job.fraction;
    entry.state = ToTransferState( job.state );
    entry.stateText = job.stateText;
    entry.message = job.message;
    entry.folder = job.folder;
    if ( TRANSFERSTATE_RUNNING == entry.state )
    {
        entry.bytesPerSecond = job.bytesPerSecond;
        entry.secondsLeft = job.secondsLeft;
    }
}

/*-------------------------------------------------------------------------*/

void
CAssetStoreTransferModel::ReadJobs( void ) const
{GUCEF_TRACE;

    m_jobs.clear();
    CORE::CString token;
    do
    {
        ASSETSTORE::CAssetJobPage page;
        if ( ASSETSTORE::ASSETSTORE_STATUS_OK != m_service.ListJobs( token, ASSETSTORE::CIAssetStoreService::MaxPageEntries, page ) )
            return;
        m_jobs.insert( m_jobs.end(), page.entries.begin(), page.entries.end() );
        token = page.nextPageToken;
    }
    while ( !token.IsNULLOrEmpty() );
}

/*-------------------------------------------------------------------------*/

void
CAssetStoreTransferModel::RequestJobs( void )
{GUCEF_TRACE;

    // one read at a time: a change while one runs reads once more when it is answered
    if ( m_jobsFuture.HasAFuture() && !m_jobsFuture.IsReady() )
    {
        m_readAgain = true;
        return;
    }
    m_readAgain = false;
    m_jobsFuture = m_service.ListJobsAsync( CORE::CString::Empty, ASSETSTORE::CIAssetStoreService::MaxPageEntries );
}

/*-------------------------------------------------------------------------*/

void
CAssetStoreTransferModel::Update( void )
{GUCEF_TRACE;

    if ( m_service.IsInProcess() || !m_jobsFuture.IsReady() )
        return;
    ASSETSTORE::CAssetJobPage page;
    if ( ASSETSTORE::CIAssetStoreService::TryGetFutureResult( m_jobsFuture, page ) )
    {
        m_jobs = page.entries;
        NotifyObservers( EntriesChangedEvent );
    }
    m_jobsFuture = CORE::CFutureResult( CORE::TASKSTATUS_UNDEFINED );
    if ( m_readAgain )
        RequestJobs();
}

/*-------------------------------------------------------------------------*/

void
CAssetStoreTransferModel::GetEntries( TEntryVector& entries ) const
{GUCEF_TRACE;

    entries.clear();
    if ( m_service.IsInProcess() )
        ReadJobs();
    for ( size_t i=0; i<m_jobs.size(); ++i )
    {
        TEntry entry;
        ToEntry( m_jobs[ i ], entry );
        entries.push_back( entry );
    }
}

/*-------------------------------------------------------------------------*/

bool
CAssetStoreTransferModel::IsActionAvailable( const TEntry& entry, ETransferAction action ) const
{GUCEF_TRACE;

    ASSETSTORE::TAssetJobAction jobAction = ASSETSTORE::ASSETJOB_ACTION_REMOVE;
    if ( !ToJobAction( action, jobAction ) )
        return GUI::CITransferQueueModel::IsActionAvailable( entry, action );
    for ( size_t i=0; i<m_jobs.size(); ++i )
        if ( m_jobs[ i ].jobId == entry.id )
            return m_jobs[ i ].IsActionAvailable( jobAction );
    ASSETSTORE::CAssetJobEntry job;
    if ( m_service.IsInProcess() && ASSETSTORE::ASSETSTORE_STATUS_OK == m_service.GetJob( entry.id, job ) )
        return job.IsActionAvailable( jobAction );
    return GUI::CITransferQueueModel::IsActionAvailable( entry, action );
}

/*-------------------------------------------------------------------------*/

bool
CAssetStoreTransferModel::PerformAction( const CORE::CString& entryId, ETransferAction action )
{GUCEF_TRACE;

    ASSETSTORE::TAssetJobAction jobAction = ASSETSTORE::ASSETJOB_ACTION_REMOVE;
    if ( !ToJobAction( action, jobAction ) )
        return false;
    if ( !m_service.IsInProcess() )
    {
        // sent: the state follows from the events of the service
        return m_service.ControlJobAsync( CORE::CString::Empty, entryId, jobAction ).HasAFuture();
    }
    ASSETSTORE::CAssetServiceResult result;
    return ASSETSTORE::ASSETSTORE_STATUS_OK == m_service.ControlJob( CORE::CString::Empty, entryId, jobAction, result );
}

/*-------------------------------------------------------------------------*/

UInt32
CAssetStoreTransferModel::CountJobs( const CORE::CString& kind ) const
{GUCEF_TRACE;

    if ( m_service.IsInProcess() )
        ReadJobs();
    UInt32 count = 0;
    for ( size_t i=0; i<m_jobs.size(); ++i )
        if ( m_jobs[ i ].kind == kind )
            ++count;
    return count;
}

/*-------------------------------------------------------------------------*/

UInt32
CAssetStoreTransferModel::GetIngestCount( void ) const
{GUCEF_TRACE;

    return CountJobs( "ingest" );
}

/*-------------------------------------------------------------------------*/

UInt32
CAssetStoreTransferModel::GetConversionCount( void ) const
{GUCEF_TRACE;

    return CountJobs( "conversion" );
}

/*-------------------------------------------------------------------------*/

bool
CAssetStoreTransferModel::StartConversion( const CORE::CString& packId )
{GUCEF_TRACE;

    if ( packId.IsNULLOrEmpty() )
        return false;
    ASSETSTORE::CAssetConversionJobRequest request;
    request.packId = packId;
    if ( !m_service.IsInProcess() )
        return m_service.RequestConversionAsync( CORE::CString::Empty, request ).HasAFuture();
    ASSETSTORE::CAssetServiceResult result;
    return ASSETSTORE::ASSETSTORE_STATUS_OK == m_service.RequestConversion( CORE::CString::Empty, request, result );
}

/*-------------------------------------------------------------------------*/

bool
CAssetStoreTransferModel::StartPendingConversion( void )
{GUCEF_TRACE;

    ASSETSTORE::CAssetConversionJobRequest request;
    request.allPending = true;
    if ( !m_service.IsInProcess() )
        return m_service.RequestConversionAsync( CORE::CString::Empty, request ).HasAFuture();
    ASSETSTORE::CAssetServiceResult result;
    return ASSETSTORE::ASSETSTORE_STATUS_OK == m_service.RequestConversion( CORE::CString::Empty, request, result );
}

/*-------------------------------------------------------------------------*/

void
CAssetStoreTransferModel::OnJobChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( !m_service.IsInProcess() )
    {
        // the mirror takes the job at once, the list in its order follows from the read
        const ASSETSTORE::CAssetJobEntry* job = CORE::CICloneable::CloneableAs< ASSETSTORE::CAssetJobEntry >( eventData );
        if ( GUCEF_NULL != job )
        {
            bool found = false;
            for ( size_t i=0; i<m_jobs.size(); ++i )
            {
                if ( m_jobs[ i ].jobId == job->jobId )
                {
                    m_jobs[ i ] = *job;
                    found = true;
                }
            }
            if ( !found )
                RequestJobs();
        }
    }
    NotifyObservers( EntriesChangedEvent );
}

/*-------------------------------------------------------------------------*/

void
CAssetStoreTransferModel::OnLocalJobsChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    NotifyObservers( EntriesChangedEvent );
}

/*-------------------------------------------------------------------------*/

void
CAssetStoreTransferModel::OnJobRemoved( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    const ASSETSTORE::CAssetJobEntry* job = CORE::CICloneable::CloneableAs< ASSETSTORE::CAssetJobEntry >( eventData );
    if ( GUCEF_NULL != job && !m_service.IsInProcess() )
    {
        for ( size_t i=0; i<m_jobs.size(); ++i )
        {
            if ( m_jobs[ i ].jobId == job->jobId )
            {
                m_jobs.erase( m_jobs.begin() + i );
                break;
            }
        }
    }
    NotifyObservers( EntriesChangedEvent );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CAssetStoreTransferModel::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::ASSETWORKBENCH::CAssetStoreTransferModel";
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
