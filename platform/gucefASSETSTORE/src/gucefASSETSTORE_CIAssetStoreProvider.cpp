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
#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */
#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */
#ifndef GUCEF_CORE_CTASKMANAGER_H
#include "gucefCORE_CTaskManager.h"
#define GUCEF_CORE_CTASKMANAGER_H
#endif /* GUCEF_CORE_CTASKMANAGER_H ? */
#ifndef GUCEF_CORE_CTASK_H
#include "gucefCORE_CTask.h"
#define GUCEF_CORE_CTASK_H
#endif /* GUCEF_CORE_CTASK_H ? */
#ifndef GUCEF_CORE_CURI_H
#include "gucefCORE_CUri.h"
#define GUCEF_CORE_CURI_H
#endif /* GUCEF_CORE_CURI_H ? */
#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */
#ifndef GUCEF_CORE_CLONEABLES_H
#include "cloneables.h"
#define GUCEF_CORE_CLONEABLES_H
#endif /* GUCEF_CORE_CLONEABLES_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETSTOREGLOBAL_H
#include "gucefASSETSTORE_CAssetStoreGlobal.h"
#define GUCEF_ASSETSTORE_CASSETSTOREGLOBAL_H
#endif /* GUCEF_ASSETSTORE_CASSETSTOREGLOBAL_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETSTOREPROVIDEROPERATIONTASKCONSUMER_H
#include "gucefASSETSTORE_CAssetStoreProviderOperationTaskConsumer.h"
#define GUCEF_ASSETSTORE_CASSETSTOREPROVIDEROPERATIONTASKCONSUMER_H
#endif /* GUCEF_ASSETSTORE_CASSETSTOREPROVIDEROPERATIONTASKCONSUMER_H ? */
#include "gucefASSETSTORE_CIAssetStoreProvider.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CEvent CIAssetStoreProvider::SessionStateChangedEvent = "GUCEF::ASSETSTORE::CIAssetStoreProvider::SessionStateChangedEvent";
const CORE::CString CIAssetStoreProvider::DefaultThreadPoolName = "AssetStore";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CIAssetStoreProvider::RegisterEvents( void )
{GUCEF_TRACE;

    SessionStateChangedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CIAssetStoreProvider::CIAssetStoreProvider( void )
    : CORE::CTSGNotifier()
    , m_pendingOperations( 0 )
    , m_threadPoolName( DefaultThreadPoolName )
    , m_lock()
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CIAssetStoreProvider::~CIAssetStoreProvider()
{GUCEF_TRACE;

    if ( m_pendingOperations.Load() > 0 )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "AssetStoreProvider: Destroyed with async operations pending, the provider must call WaitForAsyncOperations()" );
        WaitForAsyncOperations();
    }
    SignalUpcomingDestruction();
}

/*-------------------------------------------------------------------------*/

CORE::CString
CIAssetStoreProvider::GetIconName( void ) const
{GUCEF_TRACE;

    return CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CIAssetStoreProvider::GetHomePageUrl( void ) const
{GUCEF_TRACE;

    return CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CIAssetStoreProvider::GetSiteName( void ) const
{GUCEF_TRACE;

    const CORE::CString homePage = GetHomePageUrl();
    if ( homePage.IsNULLOrEmpty() )
        return CORE::CString::Empty;

    // a home page given without its scheme is a host as well
    CORE::CUri uri( homePage );
    CORE::CString host = uri.GetAuthorityHost();
    if ( host.IsNULLOrEmpty() )
        host = CORE::CUri( "https://" + homePage ).GetAuthorityHost();
    host = host.Lowercase();
    if ( 0 == host.HasSubstr( "www.", true ) && host.Length() > 4 )
        host = host.CutChars( 4, true );
    return host;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CIAssetStoreProvider::GetAuthProviderName( void ) const
{GUCEF_TRACE;

    return CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

TAssetStoreSignInKind
CIAssetStoreProvider::GetSignInKind( void ) const
{GUCEF_TRACE;

    return GetAuthProviderName().IsNULLOrEmpty() ? ASSETSTORE_SIGNIN_NONE : ASSETSTORE_SIGNIN_AUTH_PROVIDER;
}

/*-------------------------------------------------------------------------*/

AUTH::CWebSessionSignIn*
CIAssetStoreProvider::GetWebSessionSignIn( void )
{GUCEF_TRACE;

    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

CIAssetStoreInteractiveSignIn*
CIAssetStoreProvider::GetInteractiveSignIn( void )
{GUCEF_TRACE;

    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

bool
CIAssetStoreProvider::ConfigureSignIn( const CAssetStoreSignInContext& context, CORE::CString& problem )
{GUCEF_TRACE;

    return true;
}

/*-------------------------------------------------------------------------*/

bool
CIAssetStoreProvider::SignOut( void )
{GUCEF_TRACE;

    return true;
}

/*-------------------------------------------------------------------------*/

bool
CIAssetStoreProvider::RenewSession( void )
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CIAssetStoreProvider::ImportWebSession( const AUTH::CWebSession& session, CORE::CString& message )
{GUCEF_TRACE;

    message = "The store " + GetProviderId() + " does not sign in with the session of a web browser";
    return ASSETSTORE_STATUS_UNSUPPORTED;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CIAssetStoreProvider::RenewPlan( CAssetStoreDownloadPlan& plan )
{GUCEF_TRACE;

    CAssetStoreDownloadPlan renewed;
    const TAssetStoreStatus status = PlanDownload( plan.itemId, plan.formatId, plan.fileIds, renewed );
    if ( ASSETSTORE_STATUS_OK == status )
        plan = renewed;
    return status;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CIAssetStoreProvider::GetPlanProblem( const CORE::CString& itemId, const CORE::CString& formatId ) const
{GUCEF_TRACE;

    return CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

UInt64
CIAssetStoreProvider::GetMaxDownloadBytes( void ) const
{GUCEF_TRACE;

    return 0;
}

/*-------------------------------------------------------------------------*/

void
CIAssetStoreProvider::AllowLargeDownload( const CORE::CString& itemId )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CIAssetMetadataNormalizer*
CIAssetStoreProvider::GetMetadataNormalizer( void )
{GUCEF_TRACE;

    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

CIAssetStoreTransferSource*
CIAssetStoreProvider::GetTransferSource( void )
{GUCEF_TRACE;

    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

bool
CIAssetStoreProvider::AuthorizeTransferRequest( const CAssetStoreTransfer& transfer, WEB::CHttpRequestData& request )
{GUCEF_TRACE;

    return true;
}

/*-------------------------------------------------------------------------*/

bool
CIAssetStoreProvider::IsExpiredUrlResponse( const CAssetStoreTransfer& transfer, const WEB::CHttpResponseData& response )
{GUCEF_TRACE;

    return 403 == response.statusCode || 410 == response.statusCode;
}

/*-------------------------------------------------------------------------*/

CIAssetStoreProbe*
CIAssetStoreProvider::GetProbe( void )
{GUCEF_TRACE;

    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CIAssetStoreProvider::RunProbeAsync( void )
{GUCEF_TRACE;

    CAssetStoreProviderOperationData* data = GUCEF_NEW CAssetStoreProviderOperationData();
    data->operation = CAssetStoreProviderOperationData::OPERATION_PROBE;
    return QueueOperation( data );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CIAssetStoreProvider::QueryLibraryAsync( const CAssetStoreQuery& query )
{GUCEF_TRACE;

    CAssetStoreProviderOperationData* data = GUCEF_NEW CAssetStoreProviderOperationData();
    data->operation = CAssetStoreProviderOperationData::OPERATION_QUERY_LIBRARY;
    data->query = query;
    return QueueOperation( data );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CIAssetStoreProvider::GetItemAsync( const CORE::CString& itemId )
{GUCEF_TRACE;

    CAssetStoreProviderOperationData* data = GUCEF_NEW CAssetStoreProviderOperationData();
    data->operation = CAssetStoreProviderOperationData::OPERATION_GET_ITEM;
    data->itemId = itemId;
    return QueueOperation( data );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CIAssetStoreProvider::PlanDownloadAsync( const CORE::CString& itemId               ,
                                         const CORE::CString& formatId             ,
                                         const CORE::CString::StringVector& fileIds )
{GUCEF_TRACE;

    CAssetStoreProviderOperationData* data = GUCEF_NEW CAssetStoreProviderOperationData();
    data->operation = CAssetStoreProviderOperationData::OPERATION_PLAN_DOWNLOAD;
    data->itemId = itemId;
    data->formatId = formatId;
    data->fileIds = fileIds;
    return QueueOperation( data );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CIAssetStoreProvider::QueueOperation( CORE::CICloneable* operationData )
{GUCEF_TRACE;

    CAssetStoreProviderOperationData* data = static_cast< CAssetStoreProviderOperationData* >( operationData );

    // the global registers the task type
    if ( GUCEF_NULL == CAssetStoreGlobal::Instance() )
    {
        GUCEF_DELETE data;
        return CORE::CFutureResult( CORE::TASKSTATUS_QUEUEING_FAILED );
    }

    CORE::ThreadPoolPtr threadPool = CORE::CCoreGlobal::Instance()->GetTaskManager().GetOrCreateThreadPool( GetThreadPoolName() );
    if ( threadPool.IsNULL() )
    {
        GUCEF_DELETE data;
        return CORE::CFutureResult( CORE::TASKSTATUS_QUEUEING_FAILED );
    }

    CORE::CFutureResultSource source;
    data->provider = this;
    data->source = source;

    ++m_pendingOperations;
    CORE::CFutureResult queued = threadPool->QueueTask( CAssetStoreProviderOperationTaskConsumer::TaskType, data, GUCEF_NULL, true );
    if ( queued.HasNoFuture() )
    {
        --m_pendingOperations;
        GUCEF_DELETE data;
        source.Fail( CORE::TASKSTATUS_QUEUEING_FAILED, "The operation could not be queued on the thread pool" );
    }
    return source.GetFuture();
}

/*-------------------------------------------------------------------------*/

void
CIAssetStoreProvider::OperationEnded( void )
{GUCEF_TRACE;

    --m_pendingOperations;
}

/*-------------------------------------------------------------------------*/

void
CIAssetStoreProvider::SetThreadPoolName( const CORE::CString& threadPoolName )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_threadPoolName = threadPoolName.IsNULLOrEmpty() ? DefaultThreadPoolName : threadPoolName;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CIAssetStoreProvider::GetThreadPoolName( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_threadPoolName;
}

/*-------------------------------------------------------------------------*/

bool
CIAssetStoreProvider::WaitForAsyncOperations( UInt32 timeoutInMs )
{GUCEF_TRACE;

    UInt32 waited = 0;
    while ( m_pendingOperations.Load() > 0 )
    {
        if ( waited >= timeoutInMs )
            return false;
        MT::ThreadDelay( 1 );
        ++waited;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

UInt32
CIAssetStoreProvider::GetPendingAsyncOperationCount( void ) const
{GUCEF_TRACE;

    return m_pendingOperations.Load();
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CIAssetStoreProvider::GetStatusOfFuture( const CORE::CFutureResult& future )
{GUCEF_TRACE;

    if ( !future.IsReady() )
        return ASSETSTORE_STATUS_FAILED;
    CORE::CTaskPtr task = future.GetResult( 0 );
    if ( task.IsNULL() )
        return ASSETSTORE_STATUS_FAILED;
    const CORE::TTaskStatus taskStatus = task->GetTaskStatus();
    if ( CORE::TASKSTATUS_FINISHED == taskStatus )
        return ASSETSTORE_STATUS_OK;
    if ( CORE::TASKSTATUS_CANCELLED == taskStatus )
        return ASSETSTORE_STATUS_CANCELLED;
    const CORE::CString info = task->GetTaskStatusExtraInfo();
    return AssetStoreStatusFromString( info.SubstrToChar( ':', true ) );
}

/*-------------------------------------------------------------------------*/

void
CIAssetStoreProvider::NotifySessionStateChanged( void )
{GUCEF_TRACE;

    CORE::TCloneableString eventData( GetProviderId() );
    NotifyObserversFromThread( SessionStateChangedEvent, &eventData );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CIAssetStoreProvider::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::ASSETSTORE::CIAssetStoreProvider";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
