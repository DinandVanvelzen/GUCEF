/*
 *  gucefWEB_TestApp: GUCEF test application for the WEB module
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

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_CORE_CTFACTORY_H
#include "CTFactory.h"
#define GUCEF_CORE_CTFACTORY_H
#endif /* GUCEF_CORE_CTFACTORY_H ? */

#include "TestFakeDevToolsBrowser.h"

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static UInt16 g_fakeDevToolsPort = 0;

/*-------------------------------------------------------------------------*/

static UInt64
NowInMs( void )
{
    return (UInt64) ( MT::PrecisionTickCount() / ( MT::PrecisionTimerResolution() / 1000.0 ) );
}

/*-------------------------------------------------------------------------*/

/** The discovery pages of the fake browser: the browser's own web socket, and no pages (the recorder asks over the socket) */
class CFakeDevToolsHttpHandler : public WEB::CIHttpServerRequestHandler
{
    public:

    virtual bool OnRequest( const WEB::CHttpRequestData& request, WEB::CHttpResponseData& response ) GUCEF_VIRTUAL_OVERRIDE
    {
        CORE::CString body;
        if ( request.requestUri == "/json/version" )
        {
            body = "{\"Browser\": \"Chrome/140.0.0.0\", \"Protocol-Version\": \"1.3\", \"User-Agent\": \"Mozilla/5.0 Fake\","
                   " \"webSocketDebuggerUrl\": \"ws://127.0.0.1:" + CORE::UInt32ToString( g_fakeDevToolsPort ) + "/devtools/browser/B1\"}";
        }
        else
        if ( request.requestUri == "/json/list" )
        {
            body = "[]";
        }
        else
        {
            response.statusCode = 404;
            return true;
        }
        response.statusCode = 200;
        response.contentType = "application/json; charset=UTF-8";
        response.content.CopyFrom( body.ByteSize() - 1, body.C_String() );
        return true;
    }

    virtual bool OnRead( const WEB::CHttpRequestData& request, WEB::CHttpResponseData& response ) GUCEF_VIRTUAL_OVERRIDE { return OnRequest( request, response ); }
    virtual bool OnReadMetaData( const WEB::CHttpRequestData&, WEB::CHttpResponseData& ) GUCEF_VIRTUAL_OVERRIDE { return false; }
    virtual bool OnUpdate( const WEB::CHttpRequestData&, bool, WEB::CHttpResponseData& ) GUCEF_VIRTUAL_OVERRIDE { return false; }
    virtual bool OnCreate( const WEB::CHttpRequestData&, WEB::CHttpResponseData& ) GUCEF_VIRTUAL_OVERRIDE { return false; }
    virtual bool OnDelete( const WEB::CHttpRequestData&, WEB::CHttpResponseData& ) GUCEF_VIRTUAL_OVERRIDE { return false; }
    virtual bool SetRouterController( WEB::CIHTTPServerRouterController* ) GUCEF_VIRTUAL_OVERRIDE { return false; }
    virtual WEB::CIHTTPServerRouterController* GetRouterController( void ) const GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NULL; }
};

typedef CORE::CTFactory< WEB::CIHttpServerRequestHandler, CFakeDevToolsHttpHandler, MT::CMutex > TFakeDevToolsHttpHandlerFactory;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CFakeDevToolsBrowser::CFakeDevToolsBrowser( void )
    : COM::CIJsonRpcRequestHandler()
    , commands()
    , commandParams()
    , m_handlerFactory( GUCEF_NULL )
    , m_httpSettings()
    , m_httpServer( GUCEF_NULL )
    , m_webSocketServer( GUCEF_NULL )
    , m_rpcServer( GUCEF_NULL )
    , m_connection( GUCEF_NULL )
    , m_targets()
    , m_bodies()
    , m_postData()
    , m_results()
    , m_errors()
    , m_queued()
    , m_commandEvents()
    , m_pendingEvents()
    , m_tabUrls()
    , m_pulseSink( GUCEF_NULL )
    , m_attached()
    , m_emulateProbe( false )
    , m_probeCount( 0 )
{
    m_pulseSink = GUCEF_NEW CPulseSink( *this );
}

/*-------------------------------------------------------------------------*/

CFakeDevToolsBrowser::CPulseSink::CPulseSink( CFakeDevToolsBrowser& owner )
    : CORE::CObserver()
    , m_owner( owner )
{
    TEventCallback callback( this, &CPulseSink::OnPulse );
    SubscribeTo( CORE::CCoreGlobal::Instance()->GetPulseGenerator().GetPointerAlways(), CORE::CPulseGenerator::PulseEvent, callback );
}

/*-------------------------------------------------------------------------*/

CFakeDevToolsBrowser::CPulseSink::~CPulseSink()
{
    UnsubscribeAllFromObserver();
}

/*-------------------------------------------------------------------------*/

void
CFakeDevToolsBrowser::CPulseSink::OnPulse( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{
    m_owner.SendPendingEvents();
}

/*-------------------------------------------------------------------------*/

CFakeDevToolsBrowser::~CFakeDevToolsBrowser()
{
    GUCEF_DELETE m_pulseSink;
    GUCEF_DELETE m_rpcServer;
    GUCEF_DELETE m_webSocketServer;
    if ( GUCEF_NULL != m_httpServer )
        m_httpServer->Close();
    GUCEF_DELETE m_httpServer;
    GUCEF_DELETE m_handlerFactory;
}

/*-------------------------------------------------------------------------*/

bool
CFakeDevToolsBrowser::Start( void )
{
    m_handlerFactory = GUCEF_NEW TFakeDevToolsHttpHandlerFactory();
    m_httpSettings.allowWebSocketUpgrades = true;
    m_httpSettings.processRequestsAsync = false;
    m_httpSettings.keepAliveConnections = false;
    m_httpServer = GUCEF_NEW WEB::CHTTPServer( m_handlerFactory, &m_httpSettings );
    if ( !m_httpServer->ListenOnPort( 0 ) )
        return false;
    g_fakeDevToolsPort = m_httpServer->GetBoundPort();
    m_webSocketServer = GUCEF_NEW WEB::CWebSocketServer( *m_httpServer );
    m_rpcServer = GUCEF_NEW WEB::CJsonRpcServer( *m_webSocketServer );
    m_rpcServer->SetDialect( COM::JSONRPC_DIALECT_LENIENT );
    static const char* const methods[] = { "Target.setDiscoverTargets", "Target.getTargets", "Target.attachToTarget", "Target.setAutoAttach",
                                           "Network.enable", "Network.setCacheDisabled", "Network.getResponseBody", "Network.getRequestPostData",
                                           "Page.enable", "Page.addScriptToEvaluateOnNewDocument", "Runtime.addBinding", "Runtime.evaluate",
                                           "Runtime.enable", "Runtime.callFunctionOn", "Runtime.releaseObjectGroup",
                                           "Fetch.enable", "Fetch.disable", "Fetch.continueRequest", "Fetch.failRequest",
                                           "Browser.setDownloadBehavior", "Browser.cancelDownload",
                                           "DOM.getDocument", "DOM.querySelector", "DOM.querySelectorAll", "DOM.describeNode",
                                           "DOM.pushNodesByBackendIdsToFrontend", "DOM.getContentQuads", "DOM.scrollIntoViewIfNeeded",
                                           "DOM.focus", "DOM.resolveNode", "Accessibility.queryAXTree", "Input.dispatchMouseEvent",
                                           "Input.insertText", "Page.navigate", "Page.getFrameTree", "Page.createIsolatedWorld",
                                           "Page.getNavigationHistory", GUCEF_NULL };
    for ( UInt32 i=0; GUCEF_NULL != methods[ i ]; ++i )
        m_rpcServer->RegisterMethod( methods[ i ], this );
    return true;
}

/*-------------------------------------------------------------------------*/

UInt16
CFakeDevToolsBrowser::GetPort( void ) const
{
    return g_fakeDevToolsPort;
}

/*-------------------------------------------------------------------------*/

void
CFakeDevToolsBrowser::AddTarget( const CORE::CString& targetId, const CORE::CString& type, const CORE::CString& url )
{
    STarget target;
    target.id = targetId;
    target.type = type;
    target.url = url;
    m_targets.push_back( target );
}

/*-------------------------------------------------------------------------*/

void
CFakeDevToolsBrowser::SetResponseBody( const CORE::CString& requestId, const CORE::CString& body )
{
    m_bodies[ requestId ] = body;
}

/*-------------------------------------------------------------------------*/

void
CFakeDevToolsBrowser::SetRequestPostData( const CORE::CString& requestId, const CORE::CString& postData )
{
    m_postData[ requestId ] = postData;
}

/*-------------------------------------------------------------------------*/

bool
CFakeDevToolsBrowser::HasConnection( void ) const
{
    return GUCEF_NULL != m_connection;
}

/*-------------------------------------------------------------------------*/

void
CFakeDevToolsBrowser::SetCommandResult( const CORE::CString& key, const CORE::CDataNode& result )
{
    m_results[ key ] = result;
}

/*-------------------------------------------------------------------------*/

void
CFakeDevToolsBrowser::SetCommandError( const CORE::CString& key, const CORE::CString& message )
{
    m_errors[ key ] = message;
}

/*-------------------------------------------------------------------------*/

void
CFakeDevToolsBrowser::QueueCommandResult( const CORE::CString& key, const CORE::CDataNode& result )
{
    SScriptedAnswer answer;
    answer.isError = false;
    answer.result = result;
    m_queued[ key ].push_back( answer );
}

/*-------------------------------------------------------------------------*/

void
CFakeDevToolsBrowser::QueueCommandError( const CORE::CString& key, const CORE::CString& message )
{
    SScriptedAnswer answer;
    answer.isError = true;
    answer.message = message;
    m_queued[ key ].push_back( answer );
}

/*-------------------------------------------------------------------------*/

void
CFakeDevToolsBrowser::AddCommandEvent( const CORE::CString& key, const CORE::CString& method, const CORE::CDataNode& params, bool afterAnswer, UInt32 delayInMs )
{
    SScriptedEvent event;
    event.method = method;
    event.params = params;
    event.afterAnswer = afterAnswer;
    event.delayInMs = delayInMs;
    event.dueInMs = 0;
    m_commandEvents[ key ].push_back( event );
}

/*-------------------------------------------------------------------------*/

void
CFakeDevToolsBrowser::ClearCommandScript( const CORE::CString& key )
{
    m_results.erase( key );
    m_errors.erase( key );
    m_queued.erase( key );
    m_commandEvents.erase( key );
}

/*-------------------------------------------------------------------------*/

void
CFakeDevToolsBrowser::EmitScripted( const SScriptedEvent& event )
{
    // the address of the tab follows the commits of its main frame
    if ( event.method == "Page.frameNavigated" )
    {
        const CORE::CDataNode* frame = event.params.FindChild( "frame" );
        if ( GUCEF_NULL != frame && frame->GetAttributeValue( "parentId" ).AsString().IsNULLOrEmpty() )
            m_tabUrls[ event.sessionId ] = frame->GetAttributeValue( "url" ).AsString();
    }
    else
    if ( event.method == "Page.navigatedWithinDocument" )
        m_tabUrls[ event.sessionId ] = event.params.GetAttributeValue( "url" ).AsString();
    Emit( event.sessionId, event.method, event.params );
}

/*-------------------------------------------------------------------------*/

void
CFakeDevToolsBrowser::SendPendingEvents( void )
{
    if ( m_pendingEvents.empty() || GUCEF_NULL == m_connection )
        return;
    const UInt64 now = NowInMs();
    while ( !m_pendingEvents.empty() )
    {
        // the first that is due, in the order of their due times
        size_t first = 0;
        for ( size_t i=1; i<m_pendingEvents.size(); ++i )
            if ( m_pendingEvents[ i ].dueInMs < m_pendingEvents[ first ].dueInMs )
                first = i;
        if ( m_pendingEvents[ first ].dueInMs > now )
            return;
        const SScriptedEvent event = m_pendingEvents[ first ];
        m_pendingEvents.erase( m_pendingEvents.begin() + first );
        EmitScripted( event );
    }
}

/*-------------------------------------------------------------------------*/

void
CFakeDevToolsBrowser::SetEmulateProbeInterception( bool emulate )
{
    m_emulateProbe = emulate;
}

/*-------------------------------------------------------------------------*/

bool
CFakeDevToolsBrowser::GetLastCommandParams( const CORE::CString& method, const CORE::CString& sessionId, CORE::CDataNode& params ) const
{
    for ( size_t i=commands.size(); i>0; --i )
    {
        const CORE::CString& entry = commands[ i-1 ];
        if ( sessionId == "*" ? entry.StartsWith( method + "@" ) : entry == method + "@" + sessionId )
        {
            if ( i-1 < commandParams.size() )
                params = commandParams[ i-1 ];
            return true;
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

UInt32
CFakeDevToolsBrowser::CountCommands( const CORE::CString& method, const CORE::CString& sessionId ) const
{
    UInt32 count = 0;
    for ( size_t i=0; i<commands.size(); ++i )
        if ( sessionId == "*" ? commands[ i ].StartsWith( method + "@" ) : commands[ i ] == method + "@" + sessionId )
            ++count;
    return count;
}

/*-------------------------------------------------------------------------*/

void
CFakeDevToolsBrowser::ForgetConnection( void )
{
    m_connection = GUCEF_NULL;
    m_pendingEvents.clear();
    commands.clear();
    commandParams.clear();
    m_attached.clear();
}

/*-------------------------------------------------------------------------*/

bool
CFakeDevToolsBrowser::WasCommandSent( const CORE::CString& method, const CORE::CString& sessionId ) const
{
    const CORE::CString entry = method + "@" + sessionId;
    for ( size_t i=0; i<commands.size(); ++i )
        if ( commands[ i ] == entry )
            return true;
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CFakeDevToolsBrowser::WasAttachedTo( const CORE::CString& targetId ) const
{
    return m_attached.find( targetId ) != m_attached.end();
}

/*-------------------------------------------------------------------------*/

bool
CFakeDevToolsBrowser::Emit( const CORE::CString& sessionId, const CORE::CString& method, const CORE::CDataNode& params )
{
    if ( GUCEF_NULL == m_connection )
        return false;
    if ( !sessionId.IsNULLOrEmpty() )
        m_connection->SetOutgoingCallField( "sessionId", CORE::CVariant( sessionId ) );
    const bool sent = m_connection->Notify( method, &params );
    m_connection->SetOutgoingCallField( CORE::CString::Empty, CORE::CVariant() );
    return sent;
}

/*-------------------------------------------------------------------------*/

bool
CFakeDevToolsBrowser::EmitRequest( const CORE::CString& sessionId, const CORE::CString& requestId, const CORE::CString& method,
                                   const CORE::CString& url, const CORE::CString& type, const CORE::CString& frameId,
                                   const TStringMap& headers, const CORE::CString& postData, bool hasPostData, Float64 timestamp )
{
    CORE::CDataNode params( GUCEF_DATATYPE_OBJECT );
    params.SetAttribute( "requestId", CORE::CVariant( requestId ) );
    params.SetAttribute( "loaderId", CORE::CVariant( CORE::CString( "L1" ) ) );
    params.SetAttribute( "documentURL", CORE::CVariant( url ) );
    params.SetAttribute( "timestamp", CORE::CVariant( timestamp ) );
    params.SetAttribute( "wallTime", CORE::CVariant( (Float64) 1790000000.0 ) );
    params.SetAttribute( "type", CORE::CVariant( type ) );
    params.SetAttribute( "frameId", CORE::CVariant( frameId ) );
    CORE::CDataNode* request = params.AddChild( "request", GUCEF_DATATYPE_OBJECT );
    request->SetAttribute( "url", CORE::CVariant( url ) );
    request->SetAttribute( "method", CORE::CVariant( method ) );
    if ( !postData.IsNULLOrEmpty() )
        request->SetAttribute( "postData", CORE::CVariant( postData ) );
    if ( hasPostData )
        request->SetAttribute( "hasPostData", true );
    CORE::CDataNode* headerNode = request->AddChild( "headers", GUCEF_DATATYPE_OBJECT );
    for ( TStringMap::const_iterator i = headers.begin(); i != headers.end(); ++i )
        headerNode->SetAttribute( i->first, CORE::CVariant( i->second ) );
    CORE::CDataNode* initiator = params.AddChild( "initiator", GUCEF_DATATYPE_OBJECT );
    initiator->SetAttribute( "type", CORE::CVariant( CORE::CString( "script" ) ) );
    return Emit( sessionId, "Network.requestWillBeSent", params );
}

/*-------------------------------------------------------------------------*/

bool
CFakeDevToolsBrowser::EmitResponse( const CORE::CString& sessionId, const CORE::CString& requestId, Int32 status,
                                    const CORE::CString& mimeType, const TStringMap& headers, const CORE::CString& type )
{
    CORE::CDataNode params( GUCEF_DATATYPE_OBJECT );
    params.SetAttribute( "requestId", CORE::CVariant( requestId ) );
    params.SetAttribute( "type", CORE::CVariant( type ) );
    params.SetAttribute( "timestamp", CORE::CVariant( (Float64) 1.0 ) );
    CORE::CDataNode* response = params.AddChild( "response", GUCEF_DATATYPE_OBJECT );
    response->SetAttribute( "status", CORE::CVariant( status ) );
    response->SetAttribute( "mimeType", CORE::CVariant( mimeType ) );
    CORE::CDataNode* headerNode = response->AddChild( "headers", GUCEF_DATATYPE_OBJECT );
    for ( TStringMap::const_iterator i = headers.begin(); i != headers.end(); ++i )
        headerNode->SetAttribute( i->first, CORE::CVariant( i->second ) );
    return Emit( sessionId, "Network.responseReceived", params );
}

/*-------------------------------------------------------------------------*/

bool
CFakeDevToolsBrowser::EmitFinished( const CORE::CString& sessionId, const CORE::CString& requestId, UInt32 size )
{
    CORE::CDataNode params( GUCEF_DATATYPE_OBJECT );
    params.SetAttribute( "requestId", CORE::CVariant( requestId ) );
    params.SetAttribute( "timestamp", CORE::CVariant( (Float64) 2.0 ) );
    params.SetAttribute( "encodedDataLength", CORE::CVariant( size ) );
    return Emit( sessionId, "Network.loadingFinished", params );
}

/*-------------------------------------------------------------------------*/

COM::EJsonRpcRequestResult
CFakeDevToolsBrowser::OnJsonRpcRequest( COM::CJsonRpcEndpoint& endpoint     ,
                                        const COM::SJsonRpcRequest& request ,
                                        COM::SJsonRpcResponse& response     )
{
    m_connection = &endpoint;
    const CORE::CString& method = request.method;
    const CORE::CString sessionId = request.envelope.GetAttributeValue( "sessionId" ).AsString();
    commands.push_back( method + "@" + sessionId );
    commandParams.push_back( request.params );

    response.result = CORE::CDataNode( GUCEF_DATATYPE_OBJECT );
    CORE::CDataNode& result = response.result;

    // scripted answers first: by a telling parameter, then by the method
    static const char* const keyParams[] = { "backendNodeId", "nodeId", "role", "selector", "accessibleName", GUCEF_NULL };
    CORE::CString::StringVector keys;
    for ( UInt32 k=0; GUCEF_NULL != keyParams[ k ]; ++k )
    {
        const CORE::CString value = request.params.GetAttributeValue( keyParams[ k ] ).AsString( CORE::CString::Empty, false );
        if ( !value.IsNULLOrEmpty() )
            keys.push_back( method + "#" + value );
    }
    keys.push_back( method );

    // the events of the command: those before the answer now, the others on the pulse after it
    for ( size_t k=0; k<keys.size(); ++k )
    {
        GUCEF::map< CORE::CString, TScriptedEventVector >::const_iterator scripted = m_commandEvents.find( keys[ k ] );
        if ( scripted == m_commandEvents.end() )
            continue;
        for ( size_t e=0; e<scripted->second.size(); ++e )
        {
            SScriptedEvent event = scripted->second[ e ];
            event.sessionId = sessionId;
            event.dueInMs = NowInMs() + event.delayInMs;
            if ( event.afterAnswer )
                m_pendingEvents.push_back( event );
            else
                EmitScripted( event );
        }
        break;
    }

    for ( size_t k=0; k<keys.size(); ++k )
    {
        GUCEF::map< CORE::CString, TScriptedAnswerVector >::iterator queued = m_queued.find( keys[ k ] );
        if ( queued == m_queued.end() || queued->second.empty() )
            continue;
        const SScriptedAnswer answer = queued->second.front();
        queued->second.erase( queued->second.begin() );
        if ( answer.isError )
        {
            response.isError = true;
            response.error.code = -32000;
            response.error.message = answer.message;
        }
        else
            response.result = answer.result;
        return COM::JSONRPC_REQUEST_HANDLED;
    }

    for ( size_t k=0; k<keys.size(); ++k )
    {
        TStringMap::const_iterator error = m_errors.find( keys[ k ] );
        if ( error != m_errors.end() )
        {
            response.isError = true;
            response.error.code = -32000;
            response.error.message = error->second;
            return COM::JSONRPC_REQUEST_HANDLED;
        }
        GUCEF::map< CORE::CString, CORE::CDataNode >::const_iterator scripted = m_results.find( keys[ k ] );
        if ( scripted != m_results.end() )
        {
            response.result = scripted->second;
            return COM::JSONRPC_REQUEST_HANDLED;
        }
    }

    if ( method == "Target.getTargets" )
    {
        CORE::CDataNode* infos = result.AddChild( "targetInfos", GUCEF_DATATYPE_ARRAY );
        for ( size_t i=0; i<m_targets.size(); ++i )
        {
            CORE::CDataNode* info = infos->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT );
            info->SetAttribute( "targetId", CORE::CVariant( m_targets[ i ].id ) );
            info->SetAttribute( "type", CORE::CVariant( m_targets[ i ].type ) );
            info->SetAttribute( "title", CORE::CVariant( m_targets[ i ].id ) );
            info->SetAttribute( "url", CORE::CVariant( m_targets[ i ].url ) );
            info->SetAttribute( "attached", false );
        }
        return COM::JSONRPC_REQUEST_HANDLED;
    }
    if ( method == "Target.attachToTarget" )
    {
        const CORE::CString targetId = request.params.GetAttributeValue( "targetId" ).AsString();
        m_attached.insert( targetId );
        result.SetAttribute( "sessionId", CORE::CVariant( CORE::CString( "S-" ) + targetId ) );
        return COM::JSONRPC_REQUEST_HANDLED;
    }
    if ( method == "Network.getResponseBody" )
    {
        TStringMap::const_iterator i = m_bodies.find( request.params.GetAttributeValue( "requestId" ).AsString() );
        if ( i == m_bodies.end() )
        {
            response.isError = true;
            response.error.code = -32000;
            response.error.message = "No resource with given identifier found";
            return COM::JSONRPC_REQUEST_HANDLED;
        }
        result.SetAttribute( "body", CORE::CVariant( i->second ) );
        result.SetAttribute( "base64Encoded", false );
        return COM::JSONRPC_REQUEST_HANDLED;
    }
    if ( method == "Network.getRequestPostData" )
    {
        TStringMap::const_iterator i = m_postData.find( request.params.GetAttributeValue( "requestId" ).AsString() );
        if ( i == m_postData.end() )
        {
            response.isError = true;
            response.error.code = -32000;
            response.error.message = "No post data available for the request";
            return COM::JSONRPC_REQUEST_HANDLED;
        }
        result.SetAttribute( "postData", CORE::CVariant( i->second ) );
        return COM::JSONRPC_REQUEST_HANDLED;
    }
    if ( method == "Page.getNavigationHistory" )
    {
        CORE::CString url = "about:blank";
        TStringMap::const_iterator tab = m_tabUrls.find( sessionId );
        if ( tab != m_tabUrls.end() )
            url = tab->second;
        else
            for ( size_t i=0; i<m_targets.size(); ++i )
                if ( "S-" + m_targets[ i ].id == sessionId )
                    url = m_targets[ i ].url;
        result.SetAttribute( "currentIndex", (Int32) 0 );
        CORE::CDataNode* entry = result.AddChild( "entries", GUCEF_DATATYPE_ARRAY )->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT );
        entry->SetAttribute( "id", (Int32) 1 );
        entry->SetAttribute( "url", CORE::CVariant( url ) );
        return COM::JSONRPC_REQUEST_HANDLED;
    }
    if ( method == "Page.addScriptToEvaluateOnNewDocument" )
    {
        result.SetAttribute( "identifier", CORE::CVariant( CORE::CString( "1" ) ) );
        return COM::JSONRPC_REQUEST_HANDLED;
    }
    if ( method == "Runtime.evaluate" )
    {
        const CORE::CString expression = request.params.GetAttributeValue( "expression" ).AsString();
        const Int32 probe = expression.HasSubstr( "fetch('https://write-guard-probe.invalid/", true );
        CORE::CDataNode* evaluated = result.AddChild( "result", GUCEF_DATATYPE_OBJECT );
        if ( m_emulateProbe && probe >= 0 )
        {
            // the browser pauses the page's request on the browser target before the page's promise settles
            CORE::CString url = expression.CutChars( (UInt32) probe + 7, true );
            url = url.SubstrToIndex( (UInt32) url.HasChar( '\'', true ), true );
            ++m_probeCount;
            CORE::CDataNode paused( GUCEF_DATATYPE_OBJECT );
            paused.SetAttribute( "requestId", CORE::CVariant( "PROBE" + CORE::UInt32ToString( m_probeCount ) ) );
            CORE::CDataNode* pausedRequest = paused.AddChild( "request", GUCEF_DATATYPE_OBJECT );
            pausedRequest->SetAttribute( "url", CORE::CVariant( url ) );
            pausedRequest->SetAttribute( "method", CORE::CVariant( CORE::CString( "POST" ) ) );
            paused.SetAttribute( "frameId", CORE::CVariant( sessionId.CutChars( 2, true ) ) );
            paused.SetAttribute( "resourceType", CORE::CVariant( CORE::CString( "Fetch" ) ) );
            Emit( CORE::CString::Empty, "Fetch.requestPaused", paused );
            evaluated->SetAttribute( "type", CORE::CVariant( CORE::CString( "string" ) ) );
            evaluated->SetAttribute( "value", CORE::CVariant( CORE::CString( "refused" ) ) );
            return COM::JSONRPC_REQUEST_HANDLED;
        }
        evaluated->SetAttribute( "type", CORE::CVariant( CORE::CString( "undefined" ) ) );
        return COM::JSONRPC_REQUEST_HANDLED;
    }
    // the commands that only switch something on
    return COM::JSONRPC_REQUEST_HANDLED;
}
