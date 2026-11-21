/*
 *  gucefWEB: GUCEF module providing WEB application functionality
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
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

#ifndef GUCEF_WEB_CCHROMEDEVTOOLSCLIENT_H
#include "gucefWEB_CChromeDevToolsClient.h"
#define GUCEF_WEB_CCHROMEDEVTOOLSCLIENT_H
#endif /* GUCEF_WEB_CCHROMEDEVTOOLSCLIENT_H ? */

#ifndef GUCEF_WEB_CHTTPLOGREDACTION_H
#include "gucefWEB_CHttpLogRedaction.h"
#define GUCEF_WEB_CHTTPLOGREDACTION_H
#endif /* GUCEF_WEB_CHTTPLOGREDACTION_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CLONEABLES_H
#include "cloneables.h"
#define GUCEF_CORE_CLONEABLES_H
#endif /* GUCEF_CORE_CLONEABLES_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace WEB {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CEvent CChromeDevToolsClient::DiscoveryCompletedEvent = "GUCEF::WEB::CChromeDevToolsClient::DiscoveryCompletedEvent";
const CORE::CEvent CChromeDevToolsClient::DiscoveryFailedEvent = "GUCEF::WEB::CChromeDevToolsClient::DiscoveryFailedEvent";
const CORE::CEvent CChromeDevToolsClient::ConnectedEvent = "GUCEF::WEB::CChromeDevToolsClient::ConnectedEvent";
const CORE::CEvent CChromeDevToolsClient::DisconnectedEvent = "GUCEF::WEB::CChromeDevToolsClient::DisconnectedEvent";
const CORE::CEvent CChromeDevToolsClient::ProtocolEvent = "GUCEF::WEB::CChromeDevToolsClient::ProtocolEvent";

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static CORE::CString
GetText( const CORE::CDataNode& node, const CORE::CString& name )
{
    return node.GetAttributeValue( name ).AsString();
}

static UInt64
NowInMs( void )
{
    return (UInt64) ( MT::PrecisionTickCount() / ( MT::PrecisionTimerResolution() / 1000.0 ) );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CChromeDevToolsClient::RegisterEvents( void )
{GUCEF_TRACE;

    DiscoveryCompletedEvent.Initialize();
    DiscoveryFailedEvent.Initialize();
    ConnectedEvent.Initialize();
    DisconnectedEvent.Initialize();
    ProtocolEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

void
CChromeDevToolsClient::CEventSink::RegisterEventHandlers( void )
{GUCEF_TRACE;

    TEventCallback httpCallback( this, &CEventSink::OnHttpNotification );
    SubscribeTo( &m_client.m_httpClient, CHTTPClient::HTTPDataRecievedEvent    , httpCallback );
    SubscribeTo( &m_client.m_httpClient, CHTTPClient::HTTPErrorEvent           , httpCallback );
    SubscribeTo( &m_client.m_httpClient, CHTTPClient::ConnectionErrorEvent     , httpCallback );
    SubscribeTo( &m_client.m_httpClient, CHTTPClient::HTTPTransferFinishedEvent, httpCallback );

    TEventCallback webSocketCallback( this, &CEventSink::OnWebSocketNotification );
    SubscribeTo( &m_client.m_webSocketClient, CWebSocketClient::ConnectedEvent, webSocketCallback );
    SubscribeTo( &m_client.m_webSocketClient, CWebSocketClient::ClosedEvent   , webSocketCallback );

    TEventCallback rpcCallback( this, &CEventSink::OnRpcNotificationEvent );
    SubscribeTo( &m_client.m_rpcClient, COM::CJsonRpcEndpoint::NotificationEvent, rpcCallback );
}

/*-------------------------------------------------------------------------*/

void
CChromeDevToolsClient::CEventSink::OnHttpNotification( CORE::CNotifier* notifier    ,
                                                       const CORE::CEvent& eventId  ,
                                                       CORE::CICloneable* eventData )
{GUCEF_TRACE;

    m_client.OnHttpEvent( eventId, eventData );
}

/*-------------------------------------------------------------------------*/

void
CChromeDevToolsClient::CEventSink::OnWebSocketNotification( CORE::CNotifier* notifier    ,
                                                            const CORE::CEvent& eventId  ,
                                                            CORE::CICloneable* eventData )
{GUCEF_TRACE;

    m_client.OnWebSocketEvent( eventId, eventData );
}

/*-------------------------------------------------------------------------*/

void
CChromeDevToolsClient::CEventSink::OnRpcNotificationEvent( CORE::CNotifier* notifier    ,
                                                           const CORE::CEvent& eventId  ,
                                                           CORE::CICloneable* eventData )
{GUCEF_TRACE;

    m_client.OnRpcNotification( static_cast< COM::TJsonRpcNotificationEventData* >( eventData )->GetData() );
}

/*-------------------------------------------------------------------------*/

CChromeDevToolsClient::CChromeDevToolsClient( void )
    : CORE::CNotifier()
    , m_pulseGenerator( CORE::CCoreGlobal::Instance()->GetPulseGenerator() )
    , m_httpClient( m_pulseGenerator )
    , m_webSocketClient( m_pulseGenerator )
    , m_rpcClient( m_pulseGenerator )
    , m_eventSink( *this )
    , m_discoveryPhase( DISCOVERY_IDLE )
    , m_discoveryHost()
    , m_discoveryPort( 9222 )
    , m_discoveryBody()
    , m_discoveryResult()
    , m_discoveryDone( false )
    , m_discoverySucceeded( false )
    , m_waitingForDiscovery( false )
    , m_commandTimeoutInMs( 30000 )
    , m_sessionId()
{GUCEF_TRACE;

    RegisterEvents();

    // CDP leaves the version field out of its messages
    m_rpcClient.SetDialect( COM::JSONRPC_DIALECT_LENIENT );
    m_rpcClient.AttachWebSocketClient( m_webSocketClient );

    m_eventSink.RegisterEventHandlers();
}

/*-------------------------------------------------------------------------*/

CChromeDevToolsClient::CChromeDevToolsClient( const CORE::PulseGeneratorPtr& pulseGenerator )
    : CORE::CNotifier()
    , m_pulseGenerator( pulseGenerator )
    , m_httpClient( m_pulseGenerator )
    , m_webSocketClient( m_pulseGenerator )
    , m_rpcClient( m_pulseGenerator )
    , m_eventSink( *this )
    , m_discoveryPhase( DISCOVERY_IDLE )
    , m_discoveryHost()
    , m_discoveryPort( 9222 )
    , m_discoveryBody()
    , m_discoveryResult()
    , m_discoveryDone( false )
    , m_discoverySucceeded( false )
    , m_waitingForDiscovery( false )
    , m_commandTimeoutInMs( 30000 )
    , m_sessionId()
{GUCEF_TRACE;

    RegisterEvents();

    // CDP leaves the version field out of its messages
    m_rpcClient.SetDialect( COM::JSONRPC_DIALECT_LENIENT );
    m_rpcClient.AttachWebSocketClient( m_webSocketClient );

    m_eventSink.RegisterEventHandlers();
}

/*-------------------------------------------------------------------------*/

CChromeDevToolsClient::~CChromeDevToolsClient()
{GUCEF_TRACE;

    m_rpcClient.DetachWebSocketClient();
}

/*-------------------------------------------------------------------------*/

CORE::PulseGeneratorPtr
CChromeDevToolsClient::GetPulseGenerator( void ) const
{GUCEF_TRACE;

    return m_pulseGenerator;
}

/*-------------------------------------------------------------------------*/

bool
CChromeDevToolsClient::IsCallingThreadDriving( const char* operation ) const
{GUCEF_TRACE;

    // A blocking member waits on the calling thread by pumping the pulse generator, which only its own thread may do
    TThreadId callingThreadId = MT::GetCurrentThreadID();
    TThreadId drivingThreadId = m_pulseGenerator.IsNULL() ? 0 : m_pulseGenerator->GetPulseDriverThreadId();
    if ( callingThreadId == drivingThreadId )
        return true;

    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ChromeDevToolsClient: " << operation << "() refused on thread " << callingThreadId <<
        ": the pulse generator of the client is driven by thread " << drivingThreadId << " (0 if not known). Wait on that thread or use the events" );
    return false;
}

/*-------------------------------------------------------------------------*/

void
CChromeDevToolsClient::PumpPulses( UInt32 milliseconds )
{GUCEF_TRACE;

    m_pulseGenerator->ForceDirectPulse();
    MT::ThreadDelay( milliseconds );
}

/*-------------------------------------------------------------------------*/

CJsonRpcClient&
CChromeDevToolsClient::GetRpcClient( void )
{GUCEF_TRACE;

    return m_rpcClient;
}

/*-------------------------------------------------------------------------*/

bool
CChromeDevToolsClient::RequestNextDiscoveryDocument( void )
{GUCEF_TRACE;

    m_discoveryBody.clear();
    m_httpClient.Close();
    const CORE::CString path = DISCOVERY_VERSION == m_discoveryPhase ? "/json/version" : "/json/list";
    return m_httpClient.Get( m_discoveryHost, m_discoveryPort, path );
}

/*-------------------------------------------------------------------------*/

bool
CChromeDevToolsClient::DiscoverTargets( const CORE::CString& host, UInt16 port )
{GUCEF_TRACE;

    if ( DISCOVERY_IDLE != m_discoveryPhase )
        return false;

    m_discoveryHost = host;
    m_discoveryPort = port;
    m_discoveryResult = SChromeDiscoveryResult();
    m_discoveryDone = false;
    m_discoverySucceeded = false;
    m_discoveryPhase = DISCOVERY_VERSION;
    if ( !RequestNextDiscoveryDocument() )
    {
        m_discoveryPhase = DISCOVERY_IDLE;
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CChromeDevToolsClient::DiscoverTargetsAndWait( const CORE::CString& host, UInt16 port, SChromeDiscoveryResult& result, UInt32 timeoutInMs )
{GUCEF_TRACE;

    if ( !IsCallingThreadDriving( "DiscoverTargetsAndWait" ) )
        return false;

    m_waitingForDiscovery = true;
    if ( !DiscoverTargets( host, port ) )
    {
        m_waitingForDiscovery = false;
        return false;
    }

    UInt64 deadline = NowInMs() + timeoutInMs;
    while ( !m_discoveryDone && NowInMs() < deadline )
        PumpPulses( 1 );
    m_waitingForDiscovery = false;

    if ( !m_discoveryDone )
    {
        FinishDiscovery( false, "Timed out waiting for the browser to answer" );
    }
    result = m_discoveryResult;
    return m_discoverySucceeded;
}

/*-------------------------------------------------------------------------*/

void
CChromeDevToolsClient::FinishDiscovery( bool success, const CORE::CString& failureReason )
{GUCEF_TRACE;

    m_discoveryPhase = DISCOVERY_IDLE;
    m_discoveryDone = true;
    m_discoverySucceeded = success;
    m_httpClient.Close();

    if ( success )
    {
        TChromeDiscoveryCompletedEventData eData( m_discoveryResult );
        NotifyObservers( DiscoveryCompletedEvent, &eData );
    }
    else
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ChromeDevToolsClient: Discovery failed: " + failureReason );
        CORE::TCloneableString eData( failureReason );
        NotifyObservers( DiscoveryFailedEvent, &eData );
    }
}

/*-------------------------------------------------------------------------*/

void
CChromeDevToolsClient::OnHttpEvent( const CORE::CEvent& eventid, CORE::CICloneable* eventdata )
{GUCEF_TRACE;

    if ( DISCOVERY_IDLE == m_discoveryPhase )
        return;

    if ( eventid == CHTTPClient::HTTPDataRecievedEvent )
    {
        CHTTPClient::THTTPDataRecievedEventData* eData = static_cast< CHTTPClient::THTTPDataRecievedEventData* >( eventdata );
        m_discoveryBody.append( static_cast< const char* >( eData->GetData().GetConstBufferPtr() ), eData->GetData().GetDataSize() );
    }
    else
    if ( eventid == CHTTPClient::HTTPErrorEvent )
    {
        CHTTPClient::THTTPErrorEventData* eData = static_cast< CHTTPClient::THTTPErrorEventData* >( eventdata );
        FinishDiscovery( false, "The browser answered with HTTP code " + CORE::Int32ToString( (Int32) eData->GetData() ) );
    }
    else
    if ( eventid == CHTTPClient::ConnectionErrorEvent )
    {
        FinishDiscovery( false, "Unable to connect to " + m_discoveryHost + ":" + CORE::UInt32ToString( m_discoveryPort ) );
    }
    else
    if ( eventid == CHTTPClient::HTTPTransferFinishedEvent )
    {
        CORE::CDataNode document;
        if ( !COM::CJsonRpcEndpoint::ParseJsonText( CORE::CString( m_discoveryBody ), document ) )
        {
            FinishDiscovery( false, "The browser sent a document which is not valid JSON" );
            return;
        }

        if ( DISCOVERY_VERSION == m_discoveryPhase )
        {
            m_discoveryResult.browser.browser = GetText( document, "Browser" );
            m_discoveryResult.browser.protocolVersion = GetText( document, "Protocol-Version" );
            m_discoveryResult.browser.userAgent = GetText( document, "User-Agent" );
            m_discoveryResult.browser.webSocketDebuggerUrl = GetText( document, "webSocketDebuggerUrl" );

            m_discoveryPhase = DISCOVERY_TARGETS;
            if ( !RequestNextDiscoveryDocument() )
                FinishDiscovery( false, "Unable to request the list of targets" );
        }
        else
        {
            for ( CORE::CDataNode::const_iterator i = document.ConstBegin(); i != document.ConstEnd(); ++i )
            {
                SChromeTargetInfo target;
                target.id = GetText( *(*i), "id" );
                target.type = GetText( *(*i), "type" );
                target.title = GetText( *(*i), "title" );
                target.url = GetText( *(*i), "url" );
                target.webSocketDebuggerUrl = GetText( *(*i), "webSocketDebuggerUrl" );
                target.devtoolsFrontendUrl = GetText( *(*i), "devtoolsFrontendUrl" );
                m_discoveryResult.targets.push_back( target );
            }
            FinishDiscovery( true, CORE::CString() );
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CChromeDevToolsClient::OnWebSocketEvent( const CORE::CEvent& eventid, CORE::CICloneable* eventdata )
{GUCEF_TRACE;

    if ( eventid == CWebSocketClient::ConnectedEvent )
        NotifyObservers( ConnectedEvent );
    else
    if ( eventid == CWebSocketClient::ClosedEvent )
        NotifyObservers( DisconnectedEvent );
}

/*-------------------------------------------------------------------------*/

void
CChromeDevToolsClient::OnRpcNotification( const COM::SJsonRpcNotification& notification )
{GUCEF_TRACE;

    TChromeProtocolEventData eData;
    eData.GetData().method = notification.method;
    eData.GetData().params = notification.params;
    eData.GetData().hasParams = notification.hasParams;
    eData.GetData().sessionId = notification.envelope.GetAttributeValue( "sessionId" ).AsString();
    NotifyObservers( ProtocolEvent, &eData );
}

/*-------------------------------------------------------------------------*/

bool
CChromeDevToolsClient::Connect( const CORE::CString& webSocketDebuggerUrl )
{GUCEF_TRACE;

    return m_webSocketClient.Connect( webSocketDebuggerUrl );
}

/*-------------------------------------------------------------------------*/

bool
CChromeDevToolsClient::ConnectAndWait( const CORE::CString& webSocketDebuggerUrl, UInt32 timeoutInMs )
{GUCEF_TRACE;

    if ( !IsCallingThreadDriving( "ConnectAndWait" ) )
        return false;
    if ( !Connect( webSocketDebuggerUrl ) )
        return false;

    UInt64 deadline = NowInMs() + timeoutInMs;
    while ( CWebSocketClient::STATE_OPEN != m_webSocketClient.GetState() && CWebSocketClient::STATE_DISCONNECTED != m_webSocketClient.GetState() && NowInMs() < deadline )
        PumpPulses( 1 );
    return m_webSocketClient.IsConnected();
}

/*-------------------------------------------------------------------------*/

void
CChromeDevToolsClient::Disconnect( void )
{GUCEF_TRACE;

    m_webSocketClient.Disconnect();
}

/*-------------------------------------------------------------------------*/

bool
CChromeDevToolsClient::IsConnected( void ) const
{GUCEF_TRACE;

    return m_webSocketClient.IsConnected();
}

/*-------------------------------------------------------------------------*/

void
CChromeDevToolsClient::SetSessionId( const CORE::CString& sessionId )
{GUCEF_TRACE;

    m_sessionId = sessionId;
    if ( sessionId.IsNULLOrEmpty() )
        m_rpcClient.SetOutgoingCallField( CORE::CString::Empty, CORE::CVariant() );
    else
        m_rpcClient.SetOutgoingCallField( "sessionId", CORE::CVariant( sessionId ) );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CChromeDevToolsClient::GetSessionId( void ) const
{GUCEF_TRACE;

    return m_sessionId;
}

/*-------------------------------------------------------------------------*/

bool
CChromeDevToolsClient::SendCommandOnSession( const CORE::CString& sessionId ,
                                             const CORE::CString& method    ,
                                             const CORE::CDataNode* params  ,
                                             UInt64& commandId              )
{GUCEF_TRACE;

    // the session in use is put back, this may run inside an event that arrived while someone else waits for an answer
    const CORE::CString previous = m_sessionId;
    SetSessionId( sessionId );
    const bool sent = SendCommand( method, params, commandId );
    SetSessionId( previous );
    return sent;
}

/*-------------------------------------------------------------------------*/

bool
CChromeDevToolsClient::SendCommandOnSessionAndWait( const CORE::CString& sessionId ,
                                                    const CORE::CString& method    ,
                                                    const CORE::CDataNode* params  ,
                                                    CORE::CDataNode& result        ,
                                                    COM::SJsonRpcError* error      ,
                                                    UInt32 timeoutInMs             )
{GUCEF_TRACE;

    const CORE::CString previous = m_sessionId;
    SetSessionId( sessionId );
    const bool ok = SendCommandAndWait( method, params, result, error, timeoutInMs );
    SetSessionId( previous );
    return ok;
}

/*-------------------------------------------------------------------------*/

bool
CChromeDevToolsClient::SendCommand( const CORE::CString& method, const CORE::CDataNode* params, UInt64& commandId )
{GUCEF_TRACE;

    return m_rpcClient.Call( method, params, commandId );
}

/*-------------------------------------------------------------------------*/

bool
CChromeDevToolsClient::SendCommandAndWait( const CORE::CString& method   ,
                                           const CORE::CDataNode* params ,
                                           CORE::CDataNode& result       ,
                                           COM::SJsonRpcError* error          ,
                                           UInt32 timeoutInMs            )
{GUCEF_TRACE;

    COM::SJsonRpcResponse response;
    if ( !m_rpcClient.CallAndWait( method, params, response, timeoutInMs ) )
    {
        if ( GUCEF_NULL != error )
        {
            error->code = COM::JSONRPC_ERROR_TRANSPORT_CLOSED;
            error->message = "The command could not be sent: there is no connection, or the wait was refused on this thread";
        }
        return false;
    }
    if ( response.isError )
    {
        if ( GUCEF_NULL != error )
            *error = response.error;
        return false;
    }
    result = response.result;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CChromeDevToolsClient::NetworkEnable( void )
{GUCEF_TRACE;

    CORE::CDataNode result;
    return SendCommandAndWait( "Network.enable", GUCEF_NULL, result, GUCEF_NULL, m_commandTimeoutInMs );
}

/*-------------------------------------------------------------------------*/

bool
CChromeDevToolsClient::GetAllCookies( std::vector< SChromeCookie >& cookies )
{GUCEF_TRACE;

    cookies.clear();
    CORE::CDataNode result;
    if ( !SendCommandAndWait( "Network.getAllCookies", GUCEF_NULL, result, GUCEF_NULL, m_commandTimeoutInMs ) )
        return false;

    const CORE::CDataNode* list = result.Find( "cookies" );
    if ( GUCEF_NULL == list )
        return true;

    for ( CORE::CDataNode::const_iterator i = list->ConstBegin(); i != list->ConstEnd(); ++i )
        cookies.push_back( ParseCookie( *(*i) ) );
    return true;
}

/*-------------------------------------------------------------------------*/

SChromeCookie
CChromeDevToolsClient::ParseCookie( const CORE::CDataNode& node )
{GUCEF_TRACE;

    SChromeCookie cookie;
    cookie.name = GetText( node, "name" );
    cookie.value = GetText( node, "value" );
    cookie.domain = GetText( node, "domain" );
    cookie.path = GetText( node, "path" );
    cookie.expires = node.GetAttributeValue( "expires" ).AsFloat64( -1.0 );
    cookie.httpOnly = node.GetAttributeValue( "httpOnly" ).AsBool( false );
    cookie.secure = node.GetAttributeValue( "secure" ).AsBool( false );
    cookie.session = node.GetAttributeValue( "session" ).AsBool( cookie.expires < 0.0 );
    cookie.sameSite = GetText( node, "sameSite" );
    return cookie;
}

/*-------------------------------------------------------------------------*/

void
CChromeDevToolsClient::ParseCookies( const CORE::CDataNode& result, TChromeCookieVector& cookies )
{GUCEF_TRACE;

    cookies.clear();
    const CORE::CDataNode* list = result.Find( "cookies" );
    if ( GUCEF_NULL == list )
        return;
    for ( CORE::CDataNode::const_iterator i = list->ConstBegin(); i != list->ConstEnd(); ++i )
        cookies.push_back( ParseCookie( *(*i) ) );
}

/*-------------------------------------------------------------------------*/

bool
CChromeDevToolsClient::GetResponseBody( const CORE::CString& requestId, CORE::CString& body, bool& isBase64Encoded )
{GUCEF_TRACE;

    CORE::CDataNode params( GUCEF_DATATYPE_OBJECT );
    params.SetAttribute( "requestId", requestId );
    CORE::CDataNode result;
    if ( !SendCommandAndWait( "Network.getResponseBody", &params, result, GUCEF_NULL, m_commandTimeoutInMs ) )
        return false;

    body = GetText( result, "body" );
    isBase64Encoded = result.GetAttributeValue( "base64Encoded" ).AsBool( false );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CChromeDevToolsClient::RuntimeEvaluate( const CORE::CString& expression ,
                                        bool awaitPromise               ,
                                        CORE::CDataNode& result         ,
                                        CORE::CString* exceptionText    )
{GUCEF_TRACE;

    CORE::CDataNode params( GUCEF_DATATYPE_OBJECT );
    params.SetAttribute( "expression", expression );
    params.SetAttribute( "returnByValue", true );
    params.SetAttribute( "awaitPromise", awaitPromise );

    CORE::CDataNode response;
    if ( !SendCommandAndWait( "Runtime.evaluate", &params, response, GUCEF_NULL, m_commandTimeoutInMs ) )
        return false;

    // A script that throws is not a protocol failure: the exception is described next to the result
    const CORE::CDataNode* exception = response.Find( "exceptionDetails" );
    if ( GUCEF_NULL != exception )
    {
        if ( GUCEF_NULL != exceptionText )
        {
            const CORE::CDataNode* thrown = exception->Find( "exception" );
            *exceptionText = GUCEF_NULL != thrown && !GetText( *thrown, "description" ).IsNULLOrEmpty() ? GetText( *thrown, "description" ) : GetText( *exception, "text" );
        }
        return false;
    }

    // result.value holds the value of the expression, there is none for undefined
    result = CORE::CDataNode( GUCEF_DATATYPE_NIL );
    const CORE::CDataNode* evaluated = response.Find( "result" );
    if ( GUCEF_NULL != evaluated )
    {
        const CORE::CDataNode::TKeyValuePair* scalar = evaluated->GetAttribute( "value" );
        if ( GUCEF_NULL != scalar )
        {
            result = CORE::CDataNode();
            result.SetValue( (*scalar).second );
            result.SetNodeType( (*scalar).second.GetTypeId() );
        }
        else
        if ( GUCEF_NULL != evaluated->Find( "value" ) )
        {
            result = *evaluated->Find( "value" );
            result.SetName( CORE::CString::Empty );
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CChromeDevToolsClient::PageNavigate( const CORE::CString& url, CORE::CString* frameId )
{GUCEF_TRACE;

    CORE::CDataNode params( GUCEF_DATATYPE_OBJECT );
    params.SetAttribute( "url", url );
    CORE::CDataNode result;
    if ( !SendCommandAndWait( "Page.navigate", &params, result, GUCEF_NULL, m_commandTimeoutInMs ) )
        return false;

    // a navigation can fail without the command failing, the result then says why
    if ( !GetText( result, "errorText" ).IsNULLOrEmpty() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ChromeDevToolsClient: Navigation to " << CHttpLogRedaction::RedactUrl( url ) << " failed: " << GetText( result, "errorText" ) );
        return false;
    }
    if ( GUCEF_NULL != frameId )
        *frameId = GetText( result, "frameId" );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CChromeDevToolsClient::TargetCreate( const CORE::CString& url, CORE::CString& targetId )
{GUCEF_TRACE;

    CORE::CDataNode params( GUCEF_DATATYPE_OBJECT );
    params.SetAttribute( "url", url );
    CORE::CDataNode result;
    if ( !SendCommandAndWait( "Target.createTarget", &params, result, GUCEF_NULL, m_commandTimeoutInMs ) )
        return false;
    targetId = GetText( result, "targetId" );
    return !targetId.IsNULLOrEmpty();
}

/*-------------------------------------------------------------------------*/

bool
CChromeDevToolsClient::AttachToTarget( const CORE::CString& targetId, CORE::CString& sessionId )
{GUCEF_TRACE;

    CORE::CDataNode params( GUCEF_DATATYPE_OBJECT );
    params.SetAttribute( "targetId", targetId );
    params.SetAttribute( "flatten", true );
    CORE::CDataNode result;
    if ( !SendCommandAndWait( "Target.attachToTarget", &params, result, GUCEF_NULL, m_commandTimeoutInMs ) )
        return false;
    sessionId = GetText( result, "sessionId" );
    return !sessionId.IsNULLOrEmpty();
}

/*-------------------------------------------------------------------------*/

bool
CChromeDevToolsClient::TargetGetTargets( TChromeTargetVector& targets )
{GUCEF_TRACE;

    targets.clear();
    CORE::CDataNode result;
    if ( !SendCommandAndWait( "Target.getTargets", GUCEF_NULL, result, GUCEF_NULL, m_commandTimeoutInMs ) )
        return false;

    const CORE::CDataNode* list = result.Find( "targetInfos" );
    if ( GUCEF_NULL == list )
        return true;
    for ( CORE::CDataNode::const_iterator i = list->ConstBegin(); i != list->ConstEnd(); ++i )
    {
        SChromeTargetInfo target;
        target.id = GetText( *(*i), "targetId" );
        target.type = GetText( *(*i), "type" );
        target.title = GetText( *(*i), "title" );
        target.url = GetText( *(*i), "url" );
        targets.push_back( target );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CChromeDevToolsClient::GetCookies( const CORE::CString::StringVector& urls, TChromeCookieVector& cookies )
{GUCEF_TRACE;

    cookies.clear();
    CORE::CDataNode params( GUCEF_DATATYPE_OBJECT );
    if ( !urls.empty() )
        params.SetValuesOfChildByName( "urls", urls );
    CORE::CDataNode result;
    if ( !SendCommandAndWait( "Network.getCookies", urls.empty() ? GUCEF_NULL : &params, result, GUCEF_NULL, m_commandTimeoutInMs ) )
        return false;
    ParseCookies( result, cookies );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CChromeDevToolsClient::StorageGetCookies( TChromeCookieVector& cookies )
{GUCEF_TRACE;

    cookies.clear();
    CORE::CDataNode result;
    if ( !SendCommandAndWait( "Storage.getCookies", GUCEF_NULL, result, GUCEF_NULL, m_commandTimeoutInMs ) )
        return false;
    ParseCookies( result, cookies );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CChromeDevToolsClient::StorageClearCookies( void )
{GUCEF_TRACE;

    CORE::CDataNode result;
    return SendCommandAndWait( "Storage.clearCookies", GUCEF_NULL, result, GUCEF_NULL, m_commandTimeoutInMs );
}

/*-------------------------------------------------------------------------*/

bool
CChromeDevToolsClient::InputInsertText( const CORE::CString& text )
{GUCEF_TRACE;

    CORE::CDataNode params( GUCEF_DATATYPE_OBJECT );
    params.SetAttribute( "text", text );
    CORE::CDataNode result;
    return SendCommandAndWait( "Input.insertText", &params, result, GUCEF_NULL, m_commandTimeoutInMs );
}

/*-------------------------------------------------------------------------*/

bool
CChromeDevToolsClient::BrowserClose( void )
{GUCEF_TRACE;

    CORE::CDataNode result;
    if ( SendCommandAndWait( "Browser.close", GUCEF_NULL, result, GUCEF_NULL, m_commandTimeoutInMs ) )
        return true;

    // the browser may end the connection before its answer arrives
    return !IsConnected();
}

/*-------------------------------------------------------------------------*/

void
CChromeDevToolsClient::SetCommandTimeout( UInt32 timeoutInMs )
{GUCEF_TRACE;

    m_commandTimeoutInMs = timeoutInMs > 0 ? timeoutInMs : 1;
}

/*-------------------------------------------------------------------------*/

UInt32
CChromeDevToolsClient::GetCommandTimeout( void ) const
{GUCEF_TRACE;

    return m_commandTimeoutInMs;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
