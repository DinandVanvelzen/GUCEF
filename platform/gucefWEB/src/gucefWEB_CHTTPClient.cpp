/*
 *  gucefWEB: GUCEF module providing Web application functionality 
 *  for standardized protocols
 *
 *  Copyright (C) 1998 - 2020.  Dinand Vanvelzen
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
#include <stdio.h>

#ifndef GUCEF_COMCORE_CCOM_H
#include "CCom.h"
#define GUCEF_COMCORE_CCOM_H
#endif /* GUCEF_COMCORE_CCOM_H ? */

#ifndef GUCEF_COMCORE_CCOMCOREGLOBAL_H
#include "gucefCOMCORE_CComCoreGlobal.h"
#define GUCEF_COMCORE_CCOMCOREGLOBAL_H
#endif /* GUCEF_COMCORE_CCOMCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CTASKMANAGER_H
#include "gucefCORE_CTaskManager.h"
#define GUCEF_CORE_CTASKMANAGER_H
#endif /* GUCEF_CORE_CTASKMANAGER_H ? */

#ifndef GUCEF_CORE_CTHREADPOOL_H
#include "gucefCORE_CThreadPool.h"
#define GUCEF_CORE_CTHREADPOOL_H
#endif /* GUCEF_CORE_CTHREADPOOL_H ? */

#ifndef GUCEF_CORE_CTASK_H
#include "gucefCORE_CTask.h"
#define GUCEF_CORE_CTASK_H
#endif /* GUCEF_CORE_CTASK_H ? */

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "gucefWEB_CHttpClientRequestRunner.h"
#include "gucefWEB_CHTTPClient.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace WEB {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CEvent CHTTPClient::ConnectingEvent = "GUCEF::COM::CHTTPClient::ConnectingEvent";
const CORE::CEvent CHTTPClient::ConnectedEvent = "GUCEF::COM::CHTTPClient::ConnectedEvent";
const CORE::CEvent CHTTPClient::DisconnectedEvent = "GUCEF::COM::CHTTPClient::DisconnectedEvent";
const CORE::CEvent CHTTPClient::ConnectionErrorEvent = "GUCEF::COM::CHTTPClient::ConnectionErrorEvent";
const CORE::CEvent CHTTPClient::HTTPErrorEvent = "GUCEF::COM::CHTTPClient::HTTPErrorEvent";
const CORE::CEvent CHTTPClient::HTTPRedirectEvent = "GUCEF::COM::CHTTPClient::HTTPRedirectEvent";
const CORE::CEvent CHTTPClient::HTTPContentEvent = "GUCEF::COM::CHTTPClient::HTTPContentEvent";
const CORE::CEvent CHTTPClient::HTTPDataRecievedEvent = "GUCEF::COM::CHTTPClient::HTTPDataRecievedEvent";
const CORE::CEvent CHTTPClient::HTTPDataSentEvent = "GUCEF::COM::CHTTPClient::HTTPDataSentEvent";
const CORE::CEvent CHTTPClient::HTTPTransferFinishedEvent = "GUCEF::COM::CHTTPClient::HTTPTransferFinishedEvent";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CHTTPClient::CHTTPClient( void )
    : CORE::CTSGNotifier()
    , m_backends( GUCEF_NEW CHttpClientBackendSelection() )
    , m_progressNotifier( GUCEF_NEW CHttpClientProgressNotifier() )
    , m_threadPool()
    , m_threadPoolName()
    , m_cookieJar()
    , m_defaultOptions()
    , m_proxyHost()
    , m_proxyPort( 80 )
    , m_classicRequest( GUCEF_NULL )
    , m_classicRequestId( 0 )
    , m_classicRequestActive( false )
    , m_classicConnected( false )
    , m_classicErrorReported( false )
    , m_bytesReceived( 0 )
    , m_bytesTransmitted( 0 )
    , m_settingsLock()
{GUCEF_TRACE;

    RegisterEvents();

    // Progress of requests of the classic API arrives via the shared progress notifier, on our own thread
    m_progressNotifier->Subscribe( &AsObserver() );
}

/*-------------------------------------------------------------------------*/

CHTTPClient::CHTTPClient( CORE::PulseGeneratorPtr& pulseGenerator )
    : CORE::CTSGNotifier( pulseGenerator )
    , m_backends( GUCEF_NEW CHttpClientBackendSelection() )
    , m_progressNotifier( GUCEF_NEW CHttpClientProgressNotifier() )
    , m_threadPool()
    , m_threadPoolName()
    , m_cookieJar()
    , m_defaultOptions()
    , m_proxyHost()
    , m_proxyPort( 80 )
    , m_classicRequest( GUCEF_NULL )
    , m_classicRequestId( 0 )
    , m_classicRequestActive( false )
    , m_classicConnected( false )
    , m_classicErrorReported( false )
    , m_bytesReceived( 0 )
    , m_bytesTransmitted( 0 )
    , m_settingsLock()
{GUCEF_TRACE;

    RegisterEvents();

    // Progress of requests of the classic API arrives via the shared progress notifier, on our own thread
    m_progressNotifier->Subscribe( &AsObserver() );
}

/*-------------------------------------------------------------------------*/

CHTTPClient::~CHTTPClient()
{GUCEF_TRACE;

    SignalUpcomingDestruction();
    m_progressNotifier->Unsubscribe( &AsObserver() );
    if ( m_classicRequestActive && GUCEF_NULL != m_classicRequest )
        m_classicRequest->RequestCancellation();
    GUCEF_DELETE m_classicRequest;
    m_classicRequest = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

CHttpClientRequestOptions
CHTTPClient::PrepareOptions( const CHttpClientRequestOptions& options ) const
{GUCEF_TRACE;

    CHttpClientRequestOptions prepared( options );
    MT::CScopeMutex lock( m_settingsLock );
    if ( prepared.cookieJar.IsNULL() )
        prepared.cookieJar = m_cookieJar;
    return prepared;
}

/*-------------------------------------------------------------------------*/

bool
CHTTPClient::Execute( const CHttpRequestData& request          ,
                      const CHttpClientRequestOptions& options ,
                      CHttpResponseData& response              ,
                      CHttpClientTransferInfo* transferInfo    )
{GUCEF_TRACE;

    CHttpClientRequestOptions preparedOptions = PrepareOptions( options );
    CHttpClientTransferInfo localTransferInfo;
    CHttpClientTransferInfo& info = GUCEF_NULL != transferInfo ? *transferInfo : localTransferInfo;
    return CHttpClientRequestRunner::Run( m_backends, request, preparedOptions, response, info, GUCEF_NULL );
}

/*-------------------------------------------------------------------------*/

bool
CHTTPClient::Execute( const CHttpRequestData& request ,
                      CHttpResponseData& response     )
{GUCEF_TRACE;

    return Execute( request, GetDefaultOptions(), response, GUCEF_NULL );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CHTTPClient::QueueRequest( const CHttpRequestData& request          ,
                           const CHttpClientRequestOptions& options ,
                           bool reportProgress                      ,
                           UInt64 progressRequestId                 )
{GUCEF_TRACE;

    CORE::ThreadPoolPtr threadPool = GetThreadPool();
    if ( threadPool.IsNULL() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "HTTPClient: No thread pool is available to carry out the request" );
        return CORE::CFutureResult( CORE::TTaskStatus::TASKSTATUS_QUEUEING_FAILED );
    }

    // The task data is handed over to the task, it holds everything the request needs for its lifetime
    CHttpClientRequestTaskData* taskData = GUCEF_NEW CHttpClientRequestTaskData();
    taskData->request = request;
    taskData->options = PrepareOptions( options );
    taskData->backends = m_backends;
    taskData->reportProgress = reportProgress;
    taskData->progressRequestId = progressRequestId;
    if ( reportProgress )
        taskData->progressNotifier = m_progressNotifier;

    return threadPool->QueueTask( CHttpClientRequestTaskConsumer::TaskType, taskData, GUCEF_NULL, true );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CHTTPClient::ExecuteAsync( const CHttpRequestData& request          ,
                           const CHttpClientRequestOptions& options )
{GUCEF_TRACE;

    return QueueRequest( request, options, false, 0 );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CHTTPClient::ExecuteAsync( const CHttpRequestData& request )
{GUCEF_TRACE;

    return QueueRequest( request, GetDefaultOptions(), false, 0 );
}

/*-------------------------------------------------------------------------*/

CHttpClientRequestTaskData*
CHTTPClient::GetRequestTaskData( CORE::CTaskPtr task )
{GUCEF_TRACE;

    if ( task.IsNULL() || CHttpClientRequestTaskConsumer::TaskType != task->GetTaskType() )
        return GUCEF_NULL;
    return static_cast< CHttpClientRequestTaskData* >( task->GetTaskData() );
}

/*-------------------------------------------------------------------------*/

void
CHTTPClient::SetThreadPool( CORE::ThreadPoolPtr threadPool )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_settingsLock );
    m_threadPool = threadPool;
    m_threadPoolName.Clear();
}

/*-------------------------------------------------------------------------*/

void
CHTTPClient::SetThreadPoolName( const CString& threadPoolName )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_settingsLock );
    m_threadPoolName = threadPoolName;
    m_threadPool = CORE::ThreadPoolPtr();
}

/*-------------------------------------------------------------------------*/

CORE::ThreadPoolPtr
CHTTPClient::GetThreadPool( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_settingsLock );
    if ( !m_threadPool.IsNULL() )
        return m_threadPool;
    CORE::CTaskManager& taskManager = CORE::CCoreGlobal::Instance()->GetTaskManager();
    if ( !m_threadPoolName.IsNULLOrEmpty() )
        return taskManager.GetOrCreateThreadPool( m_threadPoolName );
    return taskManager.GetThreadPool();
}

/*-------------------------------------------------------------------------*/

void
CHTTPClient::SetPreferredBackends( const CString::StringVector& backendNames )
{GUCEF_TRACE;

    m_backends->SetPreferredBackends( backendNames );
}

/*-------------------------------------------------------------------------*/

CString::StringVector
CHTTPClient::GetPreferredBackends( void ) const
{GUCEF_TRACE;

    return m_backends->GetPreferredBackends();
}

/*-------------------------------------------------------------------------*/

CString
CHTTPClient::GetBackendNameForScheme( const CString& scheme ) const
{GUCEF_TRACE;

    CHttpClientBackendSelectionPtr backends = m_backends;
    CIHttpClientBackendPtr backend = backends->GetBackendForScheme( scheme.Lowercase() );
    return backend.IsNULL() ? CString::Empty : backend->GetBackendName();
}

/*-------------------------------------------------------------------------*/

bool
CHTTPClient::IsSchemeSupported( const CString& scheme )
{GUCEF_TRACE;

    CHttpClientBackendSelection backends;
    return backends.IsSchemeSupported( scheme );
}

/*-------------------------------------------------------------------------*/

CString
CHTTPClient::DescribeUnsupportedScheme( const CString& scheme )
{GUCEF_TRACE;

    return CHttpClientBackendSelection::DescribeUnsupportedScheme( scheme.Lowercase() );
}

/*-------------------------------------------------------------------------*/

CString
CHTTPClient::DescribeAvailableBackends( void )
{GUCEF_TRACE;

    CHttpClientBackendSelection backends;
    return backends.DescribeAvailableBackends();
}

/*-------------------------------------------------------------------------*/

void
CHTTPClient::SetCookieJar( CHttpCookieJarPtr cookieJar )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_settingsLock );
    m_cookieJar = cookieJar;
}

/*-------------------------------------------------------------------------*/

CHttpCookieJarPtr
CHTTPClient::GetCookieJar( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_settingsLock );
    return m_cookieJar;
}

/*-------------------------------------------------------------------------*/

void
CHTTPClient::SetDefaultOptions( const CHttpClientRequestOptions& options )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_settingsLock );
    m_defaultOptions = options;
}

/*-------------------------------------------------------------------------*/

const CHttpClientRequestOptions&
CHTTPClient::GetDefaultOptions( void ) const
{GUCEF_TRACE;

    return m_defaultOptions;
}

/*-------------------------------------------------------------------------*/

CString
CHTTPClient::BuildUrl( const CORE::CString& host, UInt16 port, const CORE::CString& path, const CORE::CValueList* valuelist )
{GUCEF_TRACE;

    CString url = "http://" + host;
    if ( 0 != port && 80 != port )
        url += ":" + CORE::ToString( port );

    CString normalizedPath = path.ReplaceChar( '\\', '/' );
    if ( normalizedPath.IsNULLOrEmpty() || '/' != normalizedPath[ 0 ] )
        url += "/";
    url += normalizedPath;

    if ( GUCEF_NULL != valuelist && valuelist->GetKeyCount() > 0 )
    {
        url += normalizedPath.HasChar( '?' ) >= 0 ? "&" : "?";
        for ( UInt32 i=0; i<valuelist->GetKeyCount(); ++i )
        {
            if ( i > 0 )
                url += "&";
            url += valuelist->GetPair( i );
        }
    }
    return url;
}

/*-------------------------------------------------------------------------*/

bool
CHTTPClient::StartClassicRequest( const CHttpRequestData& request, UInt32 byteOffset )
{GUCEF_TRACE;

    // The classic API carries out one request at a time
    if ( m_classicRequestActive && GUCEF_NULL != m_classicRequest )
        m_classicRequest->RequestCancellation();

    m_classicRequestActive = false;
    m_classicConnected = false;
    m_classicErrorReported = false;

    CHttpClientRequestOptions options = GetDefaultOptions();
    if ( 0 != byteOffset )
    {
        options.hasRange = true;
        options.rangeStart = byteOffset;
        options.rangeLength = -1;
    }

    // A proxy set on the client takes precedence over a system wide proxy
    CORE::CString proxyHost = m_proxyHost;
    UInt16 proxyPort = m_proxyPort;
    if ( proxyHost.IsNULLOrEmpty() )
    {
        bool systemWideProxyActive = false;
        COMCORE::CComCoreGlobal::Instance()->GetCom().GetSystemWideProxyServer( "HTTP", proxyHost, proxyPort, systemWideProxyActive );
        if ( !systemWideProxyActive )
            proxyHost.Clear();
    }
    if ( !proxyHost.IsNULLOrEmpty() )
    {
        options.proxyHost = proxyHost;
        options.proxyPort = proxyPort;
    }

    ++m_classicRequestId;
    GUCEF_DELETE m_classicRequest;
    m_classicRequest = GUCEF_NEW CORE::CFutureResult( QueueRequest( request, options, true, m_classicRequestId ) );
    m_classicRequestActive = m_classicRequest->HasAFuture();
    return m_classicRequestActive;
}

/*-------------------------------------------------------------------------*/

void
CHTTPClient::Close( void )
{GUCEF_TRACE;

    if ( m_classicRequestActive && GUCEF_NULL != m_classicRequest )
    {
        m_classicRequest->RequestCancellation();
        m_classicRequestActive = false;
    }
    m_classicConnected = false;
}

/*-------------------------------------------------------------------------*/

bool
CHTTPClient::Post( const CORE::CString& host                                     ,
                   UInt16 port                                                   ,
                   const CORE::CString& path                                     ,
                   const CORE::CValueList* valuelistAsContent /* = GUCEF_NULL */ ,
                   const CORE::CString& contentType                              )
{GUCEF_TRACE;

    CORE::CDynamicBuffer payload;
    if ( GUCEF_NULL != valuelistAsContent )
    {
        CORE::CString kvContent = valuelistAsContent->GetAllPairs( "=", "&", false );
        payload.CopyFrom( kvContent.ByteSize()-1, kvContent.C_String() );
    }
    return Post( host, port, path, contentType, payload );
}

/*-------------------------------------------------------------------------*/

bool
CHTTPClient::Post( const CORE::CString& urlstring      ,
                   const CORE::CString& contentType    ,
                   const CORE::CDynamicBuffer& payload )
{GUCEF_TRACE;

    CHttpRequestData request;
    if ( !request.SetUrl( urlstring.ReplaceChar( '\\', '/' ) ) )
        return false;
    request.requestType = "POST";
    request.contentRepresentation = contentType;
    request.content.CopyFrom( payload.GetDataSize(), payload.GetConstBufferPtr() );
    return StartClassicRequest( request, 0 );
}

/*-------------------------------------------------------------------------*/

bool
CHTTPClient::Post( const CORE::CString& host           ,
                   UInt16 port                         ,
                   const CORE::CString& path           ,
                   const CORE::CString& contentType    ,
                   const CORE::CDynamicBuffer& payload )
{GUCEF_TRACE;

    return Post( BuildUrl( host, port, path, GUCEF_NULL ), contentType, payload );
}

/*-------------------------------------------------------------------------*/

bool
CHTTPClient::Post( const CORE::CString& urlstring                                ,
                   const CORE::CValueList* valuelistAsContent /* = GUCEF_NULL */ ,
                   const CORE::CString& contentType                              )
{GUCEF_TRACE;

    CORE::CDynamicBuffer payload;
    if ( GUCEF_NULL != valuelistAsContent )
    {
        CORE::CString kvContent = valuelistAsContent->GetAllPairs( "=", "&", false );
        payload.CopyFrom( kvContent.ByteSize()-1, kvContent.C_String() );
    }
    return Post( urlstring, contentType, payload );
}

/*-------------------------------------------------------------------------*/

bool
CHTTPClient::Get( const CORE::CString& host                      ,
                  UInt16 port                                    ,
                  const CORE::CString& path                      ,
                  const UInt32 byteoffset /* = 0 */              ,
                  const CORE::CValueList* valuelist /* = NULL */ )
{GUCEF_TRACE;

    return Get( BuildUrl( host, port, path, valuelist ), byteoffset, GUCEF_NULL );
}

/*-------------------------------------------------------------------------*/

bool
CHTTPClient::Get( const CORE::CString& urlstring                 ,
                  const UInt32 byteoffset /* = 0 */              ,
                  const CORE::CValueList* valuelist /* = NULL */ )
{GUCEF_TRACE;

    CString url = urlstring.ReplaceChar( '\\', '/' );
    if ( GUCEF_NULL != valuelist && valuelist->GetKeyCount() > 0 )
    {
        url += url.HasChar( '?' ) >= 0 ? "&" : "?";
        for ( UInt32 i=0; i<valuelist->GetKeyCount(); ++i )
        {
            if ( i > 0 )
                url += "&";
            url += valuelist->GetPair( i );
        }
    }

    CHttpRequestData request;
    if ( !request.SetUrl( url ) )
        return false;
    request.requestType = "GET";
    return StartClassicRequest( request, byteoffset );
}

/*-------------------------------------------------------------------------*/

UInt32
CHTTPClient::GetBytesReceived( bool resetCounter )
{GUCEF_TRACE;

    UInt32 bytes = m_bytesReceived;
    if ( resetCounter )
        m_bytesReceived = 0;
    return bytes;
}

/*-------------------------------------------------------------------------*/

UInt32
CHTTPClient::GetBytesTransmitted( bool resetCounter )
{GUCEF_TRACE;

    UInt32 bytes = m_bytesTransmitted;
    if ( resetCounter )
        m_bytesTransmitted = 0;
    return bytes;
}

/*-------------------------------------------------------------------------*/

bool
CHTTPClient::IsConnected( void ) const
{GUCEF_TRACE;

    return m_classicConnected;
}

/*-------------------------------------------------------------------------*/

bool
CHTTPClient::SetHTTPProxy( const CORE::CString& proxyHost ,
                           const UInt16 port /* = 80 */   )
{GUCEF_TRACE;

    if ( !IsConnected() )
    {
        m_proxyHost = proxyHost;
        m_proxyPort = port;
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

void
CHTTPClient::GetHTTPProxy( CORE::CString& proxyHost ,
                           UInt16& port             )
{GUCEF_TRACE;

    proxyHost = m_proxyHost;
    port = m_proxyPort;
}

/*-------------------------------------------------------------------------*/

void
CHTTPClient::RegisterEvents( void )
{GUCEF_TRACE;

    ConnectingEvent.Initialize();
    ConnectedEvent.Initialize();
    DisconnectedEvent.Initialize();
    ConnectionErrorEvent.Initialize();
    HTTPErrorEvent.Initialize();
    HTTPRedirectEvent.Initialize();
    HTTPContentEvent.Initialize();
    HTTPDataRecievedEvent.Initialize();
    HTTPDataSentEvent.Initialize();
    HTTPTransferFinishedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

void
CHTTPClient::OnPumpedNotify( CORE::CNotifier* notifier                 ,
                             const CORE::CEvent& eventid               ,
                             CORE::CICloneable* eventdata /* = NULL */ )
{GUCEF_TRACE;

    // Only the progress of the current request of the classic API is of interest, any other is ignored
    if ( !m_classicRequestActive || GUCEF_NULL == eventdata )
        return;
    if ( eventid != CHttpClientRequestTaskConsumer::ConnectingEvent && eventid != CHttpClientRequestTaskConsumer::ConnectedEvent &&
         eventid != CHttpClientRequestTaskConsumer::ResponseHeadersEvent && eventid != CHttpClientRequestTaskConsumer::BodyDataEvent &&
         eventid != CHttpClientRequestTaskConsumer::DataSentEvent && eventid != CHttpClientRequestTaskConsumer::RedirectEvent &&
         eventid != CHttpClientRequestTaskConsumer::RequestFinishedEvent )
        return;

    CHttpClientRequestProgressData* progress = static_cast< CHttpClientRequestProgressData* >( eventdata );
    if ( progress->requestId != m_classicRequestId )
        return;

    // Note: when NotifyObservers() returns false this object was destroyed and must not be touched anymore
    if ( eventid == CHttpClientRequestTaskConsumer::ConnectingEvent )
    {
        NotifyObservers( ConnectingEvent );
    }
    else
    if ( eventid == CHttpClientRequestTaskConsumer::ConnectedEvent )
    {
        m_classicConnected = true;
        NotifyObservers( ConnectedEvent );
    }
    else
    if ( eventid == CHttpClientRequestTaskConsumer::ResponseHeadersEvent )
    {
        if ( progress->statusCode >= 400 )
        {
            // As before an error status is reported instead of the content
            m_classicErrorReported = true;
            THTTPErrorEventData eventData( (THTTPCODE) progress->statusCode );
            NotifyObservers( HTTPErrorEvent, &eventData );
        }
        else
        {
            struct SHTTPContentEventData contentData;
            contentData.contentSize = (UInt32) progress->contentLength;
            contentData.resumeSupported = progress->resumeSupported;
            contentData.HTTPcode = (THTTPCODE) progress->statusCode;
            THTTPContentEventData eventData( contentData );
            NotifyObservers( HTTPContentEvent, &eventData );
        }
    }
    else
    if ( eventid == CHttpClientRequestTaskConsumer::BodyDataEvent )
    {
        m_bytesReceived += progress->data.GetDataSize();
        if ( !m_classicErrorReported )
        {
            THTTPDataRecievedEventData eventData( &progress->data );
            NotifyObservers( HTTPDataRecievedEvent, &eventData );
        }
    }
    else
    if ( eventid == CHttpClientRequestTaskConsumer::DataSentEvent )
    {
        // The classic event carried the data sent, only the amount is known now
        m_bytesTransmitted += progress->byteCount;
        CORE::CDynamicBuffer noData;
        THTTPDataSendEventData eventData( &noData );
        NotifyObservers( HTTPDataSentEvent, &eventData );
    }
    else
    if ( eventid == CHttpClientRequestTaskConsumer::RedirectEvent )
    {
        THTTPRedirectEventData eventData( progress->url );
        NotifyObservers( HTTPRedirectEvent, &eventData );
    }
    else
    if ( eventid == CHttpClientRequestTaskConsumer::RequestFinishedEvent )
    {
        m_classicRequestActive = false;
        m_classicConnected = false;

        // Same order as before: the connection is closed first, then the outcome is reported.
        // Observers may start the next request from these events, as such nothing is touched afterwards
        CHttpClientTransferInfo info( progress->transferInfo );
        UInt64 requestId = m_classicRequestId;
        if ( !info.success && !info.wasCancelled )
        {
            TConnectionErrorEventData eventData( info.errorCode );
            if ( !NotifyObservers( ConnectionErrorEvent, &eventData ) ) return;
        }
        if ( requestId != m_classicRequestId ) return;
        if ( !NotifyObservers( DisconnectedEvent ) ) return;
        if ( requestId != m_classicRequestId ) return;
        if ( info.success && !m_classicErrorReported )
            NotifyObservers( HTTPTransferFinishedEvent );
    }
}

/*-------------------------------------------------------------------------*/

const CString&
CHTTPClient::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static CString typeName = "GUCEF::WEB::CHTTPClient";
    return typeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
