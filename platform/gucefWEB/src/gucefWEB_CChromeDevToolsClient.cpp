/*
 *  gucefWEB: GUCEF module providing WEB application functionality
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
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

#ifndef GUCEF_CORE_CREDACTOR_H
#include "gucefCORE_CRedactor.h"
#define GUCEF_CORE_CREDACTOR_H
#endif /* GUCEF_CORE_CREDACTOR_H ? */

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

static void
PumpPulses( UInt32 milliseconds )
{
    CORE::CCoreGlobal::Instance()->GetPulseGenerator()->ForceDirectPulse();
    MT::ThreadDelay( milliseconds );
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
    SubscribeTo( &m_client.m_rpcClient, CJsonRpcEndpoint::NotificationEvent, rpcCallback );
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

    m_client.OnRpcNotification( static_cast< TJsonRpcNotificationEventData* >( eventData )->GetData() );
}

/*-------------------------------------------------------------------------*/

CChromeDevToolsClient::CChromeDevToolsClient( void )
    : CORE::CNotifier()
    , m_httpClient()
    , m_webSocketClient()
    , m_rpcClient()
    , m_eventSink( *this )
    , m_discoveryPhase( DISCOVERY_IDLE )
    , m_discoveryHost()
    , m_discoveryPort( 9222 )
    , m_discoveryBody()
    , m_discoveryResult()
    , m_discoveryDone( false )
    , m_discoverySucceeded( false )
    , m_waitingForDiscovery( false )
{GUCEF_TRACE;

    RegisterEvents();

    // CDP leaves the version field out of its messages
    m_rpcClient.SetDialect( JSONRPC_DIALECT_LENIENT );
    m_rpcClient.AttachWebSocketClient( m_webSocketClient );

    m_eventSink.RegisterEventHandlers();
}

/*-------------------------------------------------------------------------*/

CChromeDevToolsClient::~CChromeDevToolsClient()
{GUCEF_TRACE;

    m_rpcClient.DetachWebSocketClient();
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
        if ( !CJsonRpcEndpoint::ParseJsonText( CORE::CString( m_discoveryBody ), document ) )
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
CChromeDevToolsClient::OnRpcNotification( const SJsonRpcNotification& notification )
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

    if ( sessionId.IsNULLOrEmpty() )
        m_rpcClient.SetOutgoingCallField( CORE::CString::Empty, CORE::CVariant() );
    else
        m_rpcClient.SetOutgoingCallField( "sessionId", CORE::CVariant( sessionId ) );
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
                                           SJsonRpcError* error          ,
                                           UInt32 timeoutInMs            )
{GUCEF_TRACE;

    SJsonRpcResponse response;
    if ( !m_rpcClient.CallAndWait( method, params, response, timeoutInMs ) )
    {
        if ( GUCEF_NULL != error )
        {
            error->code = JSONRPC_ERROR_TRANSPORT_CLOSED;
            error->message = "The command could not be sent, is there a connection?";
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
    return SendCommandAndWait( "Network.enable", GUCEF_NULL, result );
}

/*-------------------------------------------------------------------------*/

bool
CChromeDevToolsClient::GetAllCookies( std::vector< SChromeCookie >& cookies )
{GUCEF_TRACE;

    cookies.clear();
    CORE::CDataNode result;
    if ( !SendCommandAndWait( "Network.getAllCookies", GUCEF_NULL, result ) )
        return false;

    const CORE::CDataNode* list = result.Find( "cookies" );
    if ( GUCEF_NULL == list )
        return true;

    for ( CORE::CDataNode::const_iterator i = list->ConstBegin(); i != list->ConstEnd(); ++i )
    {
        const CORE::CDataNode& node = *(*i);
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
        cookies.push_back( cookie );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CChromeDevToolsClient::GetResponseBody( const CORE::CString& requestId, CORE::CString& body, bool& isBase64Encoded )
{GUCEF_TRACE;

    CORE::CDataNode params( GUCEF_DATATYPE_OBJECT );
    params.SetAttribute( "requestId", requestId );
    CORE::CDataNode result;
    if ( !SendCommandAndWait( "Network.getResponseBody", &params, result ) )
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
    if ( !SendCommandAndWait( "Runtime.evaluate", &params, response ) )
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
    if ( !SendCommandAndWait( "Page.navigate", &params, result ) )
        return false;

    // a navigation can fail without the command failing, the result then says why
    if ( !GetText( result, "errorText" ).IsNULLOrEmpty() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ChromeDevToolsClient: Navigation to " << CORE::CRedactor::ForUrl().RedactUrl( url ) << " failed: " << GetText( result, "errorText" ) );
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
    if ( !SendCommandAndWait( "Target.createTarget", &params, result ) )
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
    if ( !SendCommandAndWait( "Target.attachToTarget", &params, result ) )
        return false;
    sessionId = GetText( result, "sessionId" );
    return !sessionId.IsNULLOrEmpty();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
