/*
 *  gucefWEB: GUCEF module providing Web application functionality
 *  for standardized protocols
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

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

#ifndef GUCEF_CORE_CIMONOTONICCLOCK_H
#include "gucefCORE_CIMonotonicClock.h"
#define GUCEF_CORE_CIMONOTONICCLOCK_H
#endif /* GUCEF_CORE_CIMONOTONICCLOCK_H ? */

#ifndef GUCEF_WEB_CHTTPLOGREDACTION_H
#include "gucefWEB_CHttpLogRedaction.h"
#define GUCEF_WEB_CHTTPLOGREDACTION_H
#endif /* GUCEF_WEB_CHTTPLOGREDACTION_H ? */

#ifndef GUCEF_CORE_CTFACTORY_H
#include "CTFactory.h"
#define GUCEF_CORE_CTFACTORY_H
#endif /* GUCEF_CORE_CTFACTORY_H ? */

#ifndef GUCEF_WEB_CIHTTPSERVERREQUESTHANDLER_H
#include "gucefWEB_CIHttpServerRequestHandler.h"
#define GUCEF_WEB_CIHTTPSERVERREQUESTHANDLER_H
#endif /* GUCEF_WEB_CIHTTPSERVERREQUESTHANDLER_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_COM_CMCPSERVER_H
#include "gucefCOM_CMcpServer.h"
#define GUCEF_COM_CMCPSERVER_H
#endif /* GUCEF_COM_CMCPSERVER_H ? */

#ifndef GUCEF_COM_CMCPTCPTRANSPORT_H
#include "gucefCOM_CMcpTcpTransport.h"
#define GUCEF_COM_CMCPTCPTRANSPORT_H
#endif /* GUCEF_COM_CMCPTCPTRANSPORT_H ? */

#ifndef GUCEF_WEB_CWEBGLOBAL_H
#include "gucefWEB_CWebGlobal.h"
#define GUCEF_WEB_CWEBGLOBAL_H
#endif /* GUCEF_WEB_CWEBGLOBAL_H ? */

#ifndef GUCEF_WEB_CMCPHTTPEXCHANGECHANNEL_H
#include "gucefWEB_CMcpHttpExchangeChannel.h"
#define GUCEF_WEB_CMCPHTTPEXCHANGECHANNEL_H
#endif /* GUCEF_WEB_CMCPHTTPEXCHANGECHANNEL_H ? */

#include "gucefWEB_CMcpStreamableHttpTransport.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace WEB {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/**
 *  The request handler of the HTTP server of the transport: the server has no routes besides the MCP endpoint, which
 *  is taken over, so every other path is not there (404). The default handler answers a server without routes with
 *  500 ("improperly configured"), which says the server is broken where it is not
 */
class CMcpNotFoundRequestHandler : public CIHttpServerRequestHandler
{
    public:

    CMcpNotFoundRequestHandler( void ) : CIHttpServerRequestHandler() {}
    virtual ~CMcpNotFoundRequestHandler() {}

    virtual bool OnRequest( const CHttpRequestData& request, CHttpResponseData& response ) GUCEF_VIRTUAL_OVERRIDE { return NotFound( response ); }
    virtual bool OnRead( const CHttpRequestData& request, CHttpResponseData& response ) GUCEF_VIRTUAL_OVERRIDE { return NotFound( response ); }
    virtual bool OnReadMetaData( const CHttpRequestData& request, CHttpResponseData& response ) GUCEF_VIRTUAL_OVERRIDE { return NotFound( response ); }
    virtual bool OnUpdate( const CHttpRequestData& request, bool isDeltaUpdateOnly, CHttpResponseData& response ) GUCEF_VIRTUAL_OVERRIDE { return NotFound( response ); }
    virtual bool OnCreate( const CHttpRequestData& request, CHttpResponseData& response ) GUCEF_VIRTUAL_OVERRIDE { return NotFound( response ); }
    virtual bool OnDelete( const CHttpRequestData& request, CHttpResponseData& response ) GUCEF_VIRTUAL_OVERRIDE { return NotFound( response ); }
    virtual bool SetRouterController( CIHTTPServerRouterController* routerController ) GUCEF_VIRTUAL_OVERRIDE { return false; }
    virtual CIHTTPServerRouterController* GetRouterController( void ) const GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NULL; }

    private:

    static bool NotFound( CHttpResponseData& response )
    {
        response.statusCode = 404;
        return true;
    }
};

typedef CORE::CTFactory< CIHttpServerRequestHandler, CMcpNotFoundRequestHandler, MT::CMutex > TMcpNotFoundRequestHandlerFactory;
TMcpNotFoundRequestHandlerFactory g_mcpNotFoundRequestHandlerFactory;

}; /* anonymous namespace */

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** Splits the text of a Host field (RFC 9110 7.2): host, IPv6 in brackets, and the port when it is given */
static bool
SplitHostField( const CORE::CString& field, CORE::CString& host, bool& hasPort, UInt32& port )
{GUCEF_TRACE;

    hasPort = false;
    port = 0;
    if ( field.IsNULLOrEmpty() )
        return false;
    CORE::CString portText;
    if ( '[' == field[ 0 ] )
    {
        const Int32 close = field.HasChar( ']' );
        if ( close < 0 )
            return false;
        host = field.SubstrToIndex( (UInt32) close + 1, true );
        CORE::CString rest = field.CutChars( (UInt32) close + 1, true );
        if ( !rest.IsNULLOrEmpty() )
        {
            if ( ':' != rest[ 0 ] )
                return false;
            portText = rest.CutChars( 1, true );
            hasPort = true;
        }
    }
    else
    {
        const Int32 colon = field.HasChar( ':' );
        if ( colon >= 0 )
        {
            if ( field.HasChar( ':', (UInt32) colon + 1, true ) >= 0 )
                return false;       // an IPv6 address must be in brackets
            host = field.SubstrToIndex( (UInt32) colon, true );
            portText = field.CutChars( (UInt32) colon + 1, true );
            hasPort = true;
        }
        else
        {
            host = field;
        }
    }
    if ( host.IsNULLOrEmpty() )
        return false;
    if ( hasPort )
    {
        if ( portText.IsNULLOrEmpty() || portText.Length() > 5 )
            return false;
        for ( UInt32 i=0; i<portText.Length(); ++i )
            if ( portText[ i ] < '0' || portText[ i ] > '9' )
                return false;
        port = CORE::StringToUInt32( portText );
        if ( port > 65535 )
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

/** Splits an origin, scheme "://" host [ ":" port ], into the scheme and the authority */
static bool
SplitOrigin( const CORE::CString& origin, CORE::CString& scheme, CORE::CString& authority )
{GUCEF_TRACE;

    const Int32 separator = origin.HasSubstr( "://" );
    if ( separator <= 0 )
        return false;
    scheme = origin.SubstrToIndex( (UInt32) separator, true );
    authority = origin.CutChars( (UInt32) separator + 3, true );
    if ( authority.IsNULLOrEmpty() || authority.HasChar( '/' ) >= 0 || authority.HasChar( '?' ) >= 0 || authority.HasChar( '#' ) >= 0 )
        return false;
    return true;
}

/*-------------------------------------------------------------------------*/

/** The media type of a Content-Type value, without its parameters and in lower case */
static CORE::CString
MediaTypeOf( const CORE::CString& contentType )
{GUCEF_TRACE;

    return contentType.SubstrToChar( ';', true ).Trim( true ).Trim( false ).Lowercase();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CMcpStreamableHttpTransport::CMcpStreamableHttpTransport( void )
    : COM::CIMcpTransport()
    , CIHttpRequestTakeoverHandler()
    , m_server( GUCEF_NULL )
    , m_httpServer( GUCEF_NULL )
    , m_pulseGenerator()
    , m_settings()
    , m_authenticator()
    , m_authenticatorGiven( false )
    , m_authenticatorConfig()
    , m_channels()
    , m_doneChannels()
    , m_active( false )
    , m_keepAliveMs( 15000 )
    , m_nextKeepAliveInMs( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMcpStreamableHttpTransport::~CMcpStreamableHttpTransport()
{GUCEF_TRACE;

    SignalUpcomingObserverDestruction();
    if ( m_active )
        Release();
    UnsubscribeAllFromObserver();
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CMcpStreamableHttpTransport::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::WEB::CMcpStreamableHttpTransport";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CMcpStreamableHttpTransport::GetTransportName( void ) const
{GUCEF_TRACE;

    return StreamableHttpTransportName;
}

/*-------------------------------------------------------------------------*/

void
CMcpStreamableHttpTransport::SetSettings( const CMcpHttpTransportSettings& settings )
{GUCEF_TRACE;

    m_settings = settings;
}

/*-------------------------------------------------------------------------*/

const CMcpHttpTransportSettings&
CMcpStreamableHttpTransport::GetSettings( void ) const
{GUCEF_TRACE;

    return m_settings;
}

/*-------------------------------------------------------------------------*/

void
CMcpStreamableHttpTransport::SetAuthenticator( const CIMcpHttpAuthenticatorPtr& authenticator )
{GUCEF_TRACE;

    m_authenticator = authenticator;
    m_authenticatorGiven = !authenticator.IsNULL();
}

/*-------------------------------------------------------------------------*/

CIMcpHttpAuthenticatorPtr
CMcpStreamableHttpTransport::GetAuthenticator( void ) const
{GUCEF_TRACE;

    return m_authenticator;
}

/*-------------------------------------------------------------------------*/

UInt16
CMcpStreamableHttpTransport::GetBoundPort( void ) const
{GUCEF_TRACE;

    return m_active && GUCEF_NULL != m_httpServer ? m_httpServer->GetBoundPort() : 0;
}

/*-------------------------------------------------------------------------*/

CHTTPServer*
CMcpStreamableHttpTransport::GetHttpServer( void ) const
{GUCEF_TRACE;

    return m_active ? m_httpServer : GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

UInt32
CMcpStreamableHttpTransport::GetOpenExchangeCount( void ) const
{GUCEF_TRACE;

    return (UInt32) m_channels.size();
}

/*-------------------------------------------------------------------------*/

COM::CMcpServer*
CMcpStreamableHttpTransport::GetServer( void ) const
{GUCEF_TRACE;

    return m_server;
}

/*-------------------------------------------------------------------------*/

bool
CMcpStreamableHttpTransport::IsActive( void ) const
{GUCEF_TRACE;

    return m_active;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CMcpStreamableHttpTransport::GetEndpointDescription( void ) const
{GUCEF_TRACE;

    return "http://" + m_settings.bindAddress + ":" + CORE::UInt32ToString( m_active ? (UInt32) GetBoundPort() : (UInt32) m_settings.port ) + m_settings.path;
}

/*-------------------------------------------------------------------------*/

void
CMcpStreamableHttpTransport::RaiseError( const CORE::CString& problem )
{GUCEF_TRACE;

    const CORE::CString text = "McpStreamableHttpTransport: " + problem;
    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, text );
    CORE::TCloneableString eData( text );
    NotifyObservers( TransportErrorEvent, &eData );
}

/*-------------------------------------------------------------------------*/

bool
CMcpStreamableHttpTransport::Start( COM::CMcpServer& server )
{GUCEF_TRACE;

    if ( m_active )
        return true;

    if ( !COM::CMcpTcpTransport::IsLoopbackAddress( m_settings.bindAddress ) )
    {
        if ( !m_settings.allowNonLoopback )
        {
            RaiseError( "The bind address \"" + m_settings.bindAddress + "\" is not a loopback address, which needs allowNonLoopback. There is no TLS server" );
            return false;
        }
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "McpStreamableHttpTransport: Binding to \"" << m_settings.bindAddress << "\", which is not a loopback address, without TLS" );
    }
    if ( m_settings.path.IsNULLOrEmpty() || '/' != m_settings.path[ 0 ] )
    {
        RaiseError( "The path of the MCP endpoint must start with /" );
        return false;
    }

    if ( !m_authenticatorGiven )
    {
        m_authenticator = CWebGlobal::Instance()->GetMcpHttpAuthenticatorFactory().Create( m_settings.authenticator );
        if ( m_authenticator.IsNULL() )
        {
            RaiseError( "There is no authenticator named \"" + m_settings.authenticator + "\"" );
            return false;
        }
        m_authenticator->LoadConfig( m_authenticatorConfig );
    }
    CORE::CString problem;
    if ( !m_authenticator->Prepare( problem ) )
    {
        RaiseError( problem );
        return false;
    }

    m_server = &server;
    m_pulseGenerator = server.GetPulseGenerator();
    const COM::CMcpServerSettings& serverSettings = server.GetSettings();
    const UInt32 bodyLimit = m_settings.maxBodyBytes > 0 ? m_settings.maxBodyBytes : serverSettings.maxMessageBytes;
    m_keepAliveMs = m_settings.sseKeepAliveMs > 0 ? m_settings.sseKeepAliveMs : serverSettings.sseKeepAliveMs;

    CHttpServerSettings httpSettings;
    httpSettings.bindAddress = m_settings.bindAddress;
    httpSettings.maxClientConnections = m_settings.maxConnections > 0 ? m_settings.maxConnections : 32;
    httpSettings.keepAliveConnections = true;
    httpSettings.processRequestsAsync = false;     // the MCP server does its own pool dispatch
    httpSettings.allowWebSocketUpgrades = false;
    httpSettings.maxBodyBytes = bodyLimit;
    httpSettings.headerTimeoutMs = m_settings.headerTimeoutMs;
    httpSettings.bodyTimeoutMs = m_settings.bodyTimeoutMs;
    httpSettings.idleTimeoutMs = m_settings.idleTimeoutMs;

    // the HTTP server runs on the protocol thread of the MCP server: sockets, dispatch and sends never race
    m_httpServer = GUCEF_NEW CHTTPServer( m_pulseGenerator, &g_mcpNotFoundRequestHandlerFactory, &httpSettings );
    if ( !m_httpServer->RegisterRequestTakeoverHandler( m_settings.path, this, bodyLimit ) )
    {
        RaiseError( "Cannot take over the path " + m_settings.path );
        Release();
        return false;
    }
    if ( !m_httpServer->ListenOnPort( m_settings.port ) )
    {
        RaiseError( "Cannot listen on " + m_settings.bindAddress + ":" + CORE::UInt32ToString( m_settings.port ) );
        Release();
        return false;
    }

    typedef CORE::CTEventHandlerFunctor< CMcpStreamableHttpTransport > TEventCallback;
    TEventCallback pulseCallback( this, &CMcpStreamableHttpTransport::OnPulse );
    SubscribeTo( m_pulseGenerator.GetPointerAlways(), CORE::CPulseGenerator::PulseEvent, pulseCallback );
    m_nextKeepAliveInMs = CORE::CSystemMonotonicClock::Instance().GetMonotonicTimeInMs() + m_keepAliveMs;

    m_active = true;
    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "McpStreamableHttpTransport: Serving " << GetEndpointDescription() << " with the authenticator \"" << m_authenticator->GetAuthenticatorName() << "\"" );
    CORE::TCloneableString started( GetEndpointDescription() );
    NotifyObservers( StartedEvent, &started );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CMcpStreamableHttpTransport::Stop( bool graceful )
{GUCEF_TRACE;

    if ( m_active || GUCEF_NULL != m_httpServer )
        Release();
}

/*-------------------------------------------------------------------------*/

void
CMcpStreamableHttpTransport::Release( void )
{GUCEF_TRACE;

    if ( !m_pulseGenerator.IsNULL() )
        UnsubscribeFrom( m_pulseGenerator.GetPointerAlways() );

    // the POSTs still open close at the server: their clients get the end of their stream, or 503
    TChannelMap open = m_channels;
    m_channels.clear();
    for ( TChannelMap::iterator i = open.begin(); i != open.end(); ++i )
    {
        if ( GUCEF_NULL != m_server )
            m_server->CloseChannel( (*i).second );
        static_cast< CMcpHttpExchangeChannel* >( (*i).second.GetPointerAlways() )->DropExchange();
    }
    CloseDoneChannels();

    if ( GUCEF_NULL != m_httpServer )
    {
        m_httpServer->UnregisterRequestTakeoverHandler( m_settings.path );
        m_httpServer->Close();
        GUCEF_DELETE m_httpServer;
        m_httpServer = GUCEF_NULL;
    }
    const bool wasActive = m_active;
    m_active = false;
    m_server = GUCEF_NULL;
    if ( wasActive )
        NotifyObservers( StoppedEvent );
}

/*-------------------------------------------------------------------------*/

void
CMcpStreamableHttpTransport::SetJsonRpcErrorBody( CHttpResponseData& response, Int32 code, const CORE::CString& message )
{GUCEF_TRACE;

    // fixed texts only: nothing of the request goes into it
    CORE::CString body = "{\"jsonrpc\":\"2.0\",\"id\":null,\"error\":{\"code\":" + CORE::Int32ToString( code ) + ",\"message\":\"" + message + "\"}}";
    response.contentType = "application/json";
    response.content.CopyFrom( body.ByteSize() - 1, body.C_String() );
}

/*-------------------------------------------------------------------------*/

bool
CMcpStreamableHttpTransport::IsHostAllowed( const CORE::CString& hostField, const CORE::CString::StringVector& allowedHosts, UInt16 boundPort )
{GUCEF_TRACE;

    CORE::CString host;
    bool hasPort = false;
    UInt32 port = 0;
    if ( !SplitHostField( hostField, host, hasPort, port ) )
        return false;
    // a Host without a port names the default port of http
    if ( ( hasPort && port != boundPort ) || ( !hasPort && 80 != boundPort ) )
        return false;

    if ( allowedHosts.empty() )
        return host.Equals( "localhost", false ) || host.Equals( "127.0.0.1", false ) || host.Equals( "[::1]", false );
    for ( size_t i=0; i<allowedHosts.size(); ++i )
    {
        if ( host.Equals( allowedHosts[ i ], false ) )
            return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CMcpStreamableHttpTransport::IsOriginAllowed( const CORE::CString& originField, const CORE::CString::StringVector& allowedOrigins )
{GUCEF_TRACE;

    CORE::CString scheme;
    CORE::CString authority;
    if ( !SplitOrigin( originField, scheme, authority ) )
        return false;   // "null" among them: an opaque origin is never allowed
    for ( size_t i=0; i<allowedOrigins.size(); ++i )
    {
        CORE::CString allowedScheme;
        CORE::CString allowedAuthority;
        if ( SplitOrigin( allowedOrigins[ i ], allowedScheme, allowedAuthority ) &&
             CHttpRequestData::IsSameOrigin( scheme, authority, allowedScheme, allowedAuthority ) )
            return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

void
CMcpStreamableHttpTransport::AuditRefusal( int status, const CORE::CString& why, Int32 errorCode, const COM::CMcpPrincipal* principal )
{GUCEF_TRACE;

    // the reason holds no text of the client: it is written by the transport
    if ( GUCEF_NULL == m_server )
        return;
    COM::CMcpPrincipal unknown( StreamableHttpTransportName, "anonymous", false );
    m_server->AuditTransportRefusal( GUCEF_NULL != principal ? *principal : unknown, "HTTP " + CORE::ToString( status ) + ": " + why, errorCode );
}

/*-------------------------------------------------------------------------*/

void
CMcpStreamableHttpTransport::AuditAuthenticationRefusal( const COM::CMcpPrincipal& principal, const CHttpResponseData& challenge )
{GUCEF_TRACE;

    CORE::CString challengeText;
    CHttpResponseData::TStringMap::const_iterator field = challenge.otherHeaders.find( "WWW-Authenticate" );
    if ( field != challenge.otherHeaders.end() )
        challengeText = (*field).second;
    const char* why = "no credentials";
    if ( challengeText.HasSubstr( "invalid_token" ) >= 0 )
        why = "the credentials are not valid";
    else
    if ( challengeText.HasSubstr( "invalid_request" ) >= 0 )
        why = "the request is malformed for authentication";
    COM::CMcpPrincipal refused( principal.transport.IsNULLOrEmpty() ? StreamableHttpTransportName : principal.transport, principal.subject, false );
    AuditRefusal( challenge.statusCode, why, 0, &refused );
}

/*-------------------------------------------------------------------------*/

CIHttpRequestTakeoverHandler::EHeaderDecision
CMcpStreamableHttpTransport::OnHttpRequestHeaders( CHTTPServer& server                 ,
                                                   const CHttpRequestData& request     ,
                                                   const CHttpServerExchange& exchange ,
                                                   CHttpResponseData& response         )
{GUCEF_TRACE;

    if ( !m_active || GUCEF_NULL == m_server )
    {
        response.statusCode = 503;
        return TAKEOVER_REJECT;
    }

    // 1. Host: a page that rebinds its own DNS name to 127.0.0.1 still names its own host here
    if ( request.requestHost.IsNULLOrEmpty() )
    {
        response.statusCode = 400;
        return TAKEOVER_REJECT;
    }
    if ( !IsHostAllowed( request.requestHost, m_settings.allowedHosts, GetBoundPort() ) )
    {
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "McpStreamableHttpTransport: A request for the host \"" << CHttpLogRedaction::RedactText( request.requestHost ) << "\" was refused" );
        response.statusCode = 403;
        SetJsonRpcErrorBody( response, COM::MCP_ERROR_GUCEF_ACCESS_DENIED, "Forbidden: the Host is not allowed" );
        AuditRefusal( 403, "the Host is not allowed", COM::MCP_ERROR_GUCEF_ACCESS_DENIED );
        return TAKEOVER_REJECT;
    }

    // 2. Origin, when there is one: browsers send it, other clients do not
    CHttpRequestData::TStringMap::const_iterator origin = request.otherHeaders.find( "origin" );
    if ( origin != request.otherHeaders.end() && !IsOriginAllowed( (*origin).second, m_settings.allowedOrigins ) )
    {
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "McpStreamableHttpTransport: A request from the origin \"" << CHttpLogRedaction::RedactText( (*origin).second ) << "\" was refused" );
        response.statusCode = 403;
        SetJsonRpcErrorBody( response, COM::MCP_ERROR_GUCEF_ACCESS_DENIED, "Forbidden: the Origin is not allowed" );
        AuditRefusal( 403, "the Origin is not allowed", COM::MCP_ERROR_GUCEF_ACCESS_DENIED );
        return TAKEOVER_REJECT;
    }

    // 3. only POST: GET (the stream of earlier revisions) and DELETE (their session end) are not offered
    if ( "POST" != request.requestType )
    {
        response.statusCode = 405;
        response.allowedMethods.push_back( "POST" );
        return TAKEOVER_REJECT;
    }

    // 4. who asks
    COM::CMcpPrincipal principal( StreamableHttpTransportName, "anonymous", false );
    if ( !m_authenticator->Authenticate( request, principal, response ) )
    {
        if ( response.statusCode < 400 )
            response.statusCode = 401;
        AuditAuthenticationRefusal( principal, response );
        return TAKEOVER_REJECT;
    }

    // 5. what is sent
    if ( "application/json" != MediaTypeOf( request.contentRepresentation ) )
    {
        response.statusCode = 415;
        response.acceptedTypes.push_back( "application/json" );
        return TAKEOVER_REJECT;
    }
    if ( request.otherHeaders.end() == request.otherHeaders.find( "content-length" ) )
    {
        response.statusCode = 411;
        return TAKEOVER_REJECT;
    }

    // 6. what can be answered with
    bool json = false;
    bool sse = false;
    if ( request.resourceRepresentations.empty() )
    {
        json = true;
        sse = true;
    }
    for ( size_t i=0; i<request.resourceRepresentations.size(); ++i )
    {
        const CORE::CString range = request.resourceRepresentations[ i ].Lowercase();
        if ( "*/*" == range || "application/json" == range || "application/*" == range )
            json = true;
        if ( "*/*" == range || "text/event-stream" == range || "text/*" == range )
            sse = true;
    }
    if ( !json && !sse )
    {
        response.statusCode = 406;
        return TAKEOVER_REJECT;
    }
    return TAKEOVER_CONTINUE;
}

/*-------------------------------------------------------------------------*/

void
CMcpStreamableHttpTransport::OnHttpRequest( CHTTPServer& server             ,
                                            const CHttpRequestData& request ,
                                            CHttpServerExchange& exchange   )
{GUCEF_TRACE;

    // authenticated again rather than remembered between the callbacks: a request whose body never came leaves nothing
    COM::CMcpPrincipal principal( StreamableHttpTransportName, "anonymous", false );
    CHttpResponseData challenge;
    if ( GUCEF_NULL == m_server || !m_authenticator->Authenticate( request, principal, challenge ) )
    {
        if ( challenge.statusCode < 400 )
            challenge.statusCode = 401;
        AuditAuthenticationRefusal( principal, challenge );
        exchange.SendResponse( challenge );
        return;
    }

    CMcpHttpExchangeChannel* channel = GUCEF_NEW CMcpHttpExchangeChannel( principal, *this, exchange.GetSharedPtr(), request );
    COM::CMcpChannelPtr channelPtr( channel );
    if ( !m_server->OpenChannel( channelPtr ) )
    {
        channel->DropExchange();
        CHttpResponseData busy;
        busy.statusCode = 503;
        exchange.SendResponse( busy );
        return;
    }
    m_channels[ exchange.GetExchangeId() ] = channelPtr;

    const UInt32 size = request.content.GetDataSize();
    const CORE::CString body = size > 0 ? CORE::CString( static_cast< const char* >( request.content.GetConstBufferPtr() ), size ) : CORE::CString::Empty;
    channel->ProcessIncomingMessage( body );
    channel->FinishIncoming();
}

/*-------------------------------------------------------------------------*/

void
CMcpStreamableHttpTransport::OnHttpExchangeClosed( CHTTPServer& server           ,
                                                   CHttpServerExchange& exchange )
{GUCEF_TRACE;

    // the client went away: for a modern request that is the cancellation, a legacy one runs on and its answer is dropped
    TChannelMap::iterator found = m_channels.find( exchange.GetExchangeId() );
    if ( found == m_channels.end() )
        return;
    COM::CMcpChannelPtr channel = (*found).second;
    m_channels.erase( found );
    static_cast< CMcpHttpExchangeChannel* >( channel.GetPointerAlways() )->DropExchange();
    if ( GUCEF_NULL != m_server )
        m_server->CloseChannel( channel );
}

/*-------------------------------------------------------------------------*/

void
CMcpStreamableHttpTransport::OnChannelDone( CMcpHttpExchangeChannel& channel )
{GUCEF_TRACE;

    // the answer went out from inside the server: the channel is closed at the server on the next pulse, not from
    // within the call that sent the answer
    TChannelMap::iterator found = m_channels.find( channel.GetExchangeId() );
    if ( found == m_channels.end() )
        return;
    m_doneChannels.push_back( (*found).second );
    m_channels.erase( found );
    // the next periodic pulse of the HTTP server's socket closes it; no immediate pulse, which without a driver
    // would pulse from inside the very send that is to be left alone
}

/*-------------------------------------------------------------------------*/

void
CMcpStreamableHttpTransport::CloseDoneChannels( void )
{GUCEF_TRACE;

    if ( m_doneChannels.empty() )
        return;
    TChannelVector done;
    done.swap( m_doneChannels );
    for ( size_t i=0; i<done.size(); ++i )
    {
        static_cast< CMcpHttpExchangeChannel* >( done[ i ].GetPointerAlways() )->DropExchange();
        if ( GUCEF_NULL != m_server )
            m_server->CloseChannel( done[ i ] );
    }
}

/*-------------------------------------------------------------------------*/

void
CMcpStreamableHttpTransport::OnPulse( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( !m_active )
        return;
    CloseDoneChannels();

    if ( m_keepAliveMs > 0 && !m_channels.empty() )
    {
        const UInt64 now = CORE::CSystemMonotonicClock::Instance().GetMonotonicTimeInMs();
        if ( now >= m_nextKeepAliveInMs )
        {
            m_nextKeepAliveInMs = now + m_keepAliveMs;
            TChannelMap open = m_channels;
            for ( TChannelMap::iterator i = open.begin(); i != open.end(); ++i )
                static_cast< CMcpHttpExchangeChannel* >( (*i).second.GetPointerAlways() )->SendKeepAlive();
        }
    }
}

/*-------------------------------------------------------------------------*/

bool
CMcpStreamableHttpTransport::SaveConfig( CORE::CDataNode& cfg ) const
{GUCEF_TRACE;

    cfg.SetAttribute( "type", CORE::CVariant( StreamableHttpTransportName ) );
    m_settings.SaveConfig( cfg );
    if ( !m_authenticator.IsNULL() )
        m_authenticator->SaveConfig( cfg );
    else
    {
        // the keys of an authenticator that was not made yet stay as they were configured
        for ( CORE::CDataNode::TAttributeMap::const_iterator i = m_authenticatorConfig.AttributeBegin(); i != m_authenticatorConfig.AttributeEnd(); ++i )
        {
            if ( GUCEF_NULL == cfg.GetAttribute( (*i).first ) )
                cfg.SetAttribute( (*i).first, (*i).second );
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMcpStreamableHttpTransport::LoadConfig( const CORE::CDataNode& cfg )
{GUCEF_TRACE;

    if ( !m_settings.LoadConfig( cfg ) )
        return false;
    m_authenticatorConfig = cfg;
    if ( !m_authenticatorGiven && !m_authenticator.IsNULL() )
        m_authenticator.Unlink();      // made again, from this configuration, at the next start
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
