/*
 *  gucefAUTH: GUCEF module providing player and service authentication through external identity providers
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

#ifndef GUCEF_CORE_CTFACTORY_H
#include "CTFactory.h"
#define GUCEF_CORE_CTFACTORY_H
#endif /* GUCEF_CORE_CTFACTORY_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_WEB_CIHTTPSERVERREQUESTHANDLER_H
#include "gucefWEB_CIHttpServerRequestHandler.h"
#define GUCEF_WEB_CIHTTPSERVERREQUESTHANDLER_H
#endif /* GUCEF_WEB_CIHTTPSERVERREQUESTHANDLER_H ? */

#ifndef GUCEF_AUTH_CAUTHCRYPTO_H
#include "gucefAUTH_CAuthCrypto.h"
#define GUCEF_AUTH_CAUTHCRYPTO_H
#endif /* GUCEF_AUTH_CAUTHCRYPTO_H ? */

#ifndef GUCEF_AUTH_CAUTHURL_H
#include "gucefAUTH_CAuthUrl.h"
#define GUCEF_AUTH_CAUTHURL_H
#endif /* GUCEF_AUTH_CAUTHURL_H ? */

#ifndef GUCEF_AUTH_CLOCK_H
#include "gucefAUTH_Clock.h"
#define GUCEF_AUTH_CLOCK_H
#endif /* GUCEF_AUTH_CLOCK_H ? */

#ifndef GUCEF_AUTH_STRINGUTILS_H
#include "gucefAUTH_StringUtils.h"
#define GUCEF_AUTH_STRINGUTILS_H
#endif /* GUCEF_AUTH_STRINGUTILS_H ? */

#include "gucefAUTH_CLoopbackRedirectListener.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUTH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The HTTP request handler of the listener: everything goes to the listener, the other operations of the interface are not used */
class CLoopbackRequestHandler : public WEB::CIHttpServerRequestHandler
{
    public:

    CLoopbackRequestHandler( void ) : WEB::CIHttpServerRequestHandler(), m_listener( GUCEF_NULL ) {}
    virtual ~CLoopbackRequestHandler() {}

    void SetListener( CLoopbackRedirectListener* listener ) { m_listener = listener; }

    virtual bool OnRequest( const WEB::CHttpRequestData& request, WEB::CHttpResponseData& response ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( GUCEF_NULL == m_listener )
        {
            response.statusCode = 404;
            return true;
        }
        m_listener->HandleRequest( request, response );
        return true;
    }

    virtual bool OnRead( const WEB::CHttpRequestData& request, WEB::CHttpResponseData& response ) GUCEF_VIRTUAL_OVERRIDE { return OnRequest( request, response ); }
    virtual bool OnReadMetaData( const WEB::CHttpRequestData&, WEB::CHttpResponseData& ) GUCEF_VIRTUAL_OVERRIDE { return false; }
    virtual bool OnUpdate( const WEB::CHttpRequestData&, bool, WEB::CHttpResponseData& ) GUCEF_VIRTUAL_OVERRIDE { return false; }
    virtual bool OnCreate( const WEB::CHttpRequestData&, WEB::CHttpResponseData& ) GUCEF_VIRTUAL_OVERRIDE { return false; }
    virtual bool OnDelete( const WEB::CHttpRequestData&, WEB::CHttpResponseData& ) GUCEF_VIRTUAL_OVERRIDE { return false; }
    virtual bool SetRouterController( WEB::CIHTTPServerRouterController* ) GUCEF_VIRTUAL_OVERRIDE { return false; }
    virtual WEB::CIHTTPServerRouterController* GetRouterController( void ) const GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NULL; }

    private:

    CLoopbackRedirectListener* m_listener;
};

/*-------------------------------------------------------------------------*/

/** Makes the handler of the HTTP server of one listener and links it to that listener */
class CLoopbackHandlerFactory : public CORE::CTFactory< WEB::CIHttpServerRequestHandler, CLoopbackRequestHandler, MT::CMutex >
{
    public:

    typedef CORE::CTFactory< WEB::CIHttpServerRequestHandler, CLoopbackRequestHandler, MT::CMutex > TBase;

    explicit CLoopbackHandlerFactory( CLoopbackRedirectListener* listener )
        : TBase(), m_listener( listener ) {}

    virtual ~CLoopbackHandlerFactory() {}

    virtual TProductPtr Create( void ) GUCEF_VIRTUAL_OVERRIDE
    {
        TProductPtr product = TBase::Create();
        CLoopbackRequestHandler* handler = static_cast< CLoopbackRequestHandler* >( product.GetPointerAlways() );
        if ( GUCEF_NULL != handler )
            handler->SetListener( m_listener );
        return product;
    }

    private:

    CLoopbackHandlerFactory( const CLoopbackHandlerFactory& src );              /**< not implemented */
    CLoopbackHandlerFactory& operator=( const CLoopbackHandlerFactory& src );   /**< not implemented */

    private:

    CLoopbackRedirectListener* m_listener;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const UInt32 CLoopbackRedirectListener::CloseDelayAfterCallbackInMs;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CLoopbackRedirectListener::CLoopbackRedirectListener( void )
    : m_server( GUCEF_NULL )
    , m_factory( GUCEF_NULL )
    , m_state( STATE_IDLE )
    , m_port( 0 )
    , m_path()
    , m_stateParameter()
    , m_expectedState()
    , m_deadlineMs( 0 )
    , m_receivedAtMs( 0 )
    , m_serverClosed( true )
    , m_parameters()
    , m_rejectedCount( 0 )
    , m_requestCount( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CLoopbackRedirectListener::~CLoopbackRedirectListener()
{GUCEF_TRACE;

    Close();
    GUCEF_DELETE m_server;
    m_server = GUCEF_NULL;
    GUCEF_DELETE m_factory;
    m_factory = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

bool
CLoopbackRedirectListener::Start( UInt16 port                       ,
                                  const CORE::CString& path         ,
                                  const CSecretString& expectedState,
                                  UInt32 timeoutInMs                ,
                                  const CORE::CString& stateParameter )
{GUCEF_TRACE;

    if ( STATE_IDLE != m_state || path.IsNULLOrEmpty() || path[ 0 ] != '/' || expectedState.IsEmpty() || stateParameter.IsNULLOrEmpty() )
        return false;
    // The path is used in a redirect uri and compared as it is: only plain characters
    if ( !CAuthUrl::IsAcceptableEndpoint( "https://127.0.0.1" + path, false ) || path.HasChar( '?' ) >= 0 )
        return false;

    m_path = path;
    m_stateParameter = stateParameter;
    m_expectedState = expectedState;

    m_factory = GUCEF_NEW CLoopbackHandlerFactory( this );
    WEB::CHttpServerSettings settings;
    settings.bindAddress = "127.0.0.1";
    settings.keepAliveConnections = false;
    settings.processRequestsAsync = false;
    settings.allowWebSocketUpgrades = false;
    settings.maxClientConnections = 8;
    m_server = GUCEF_NEW WEB::CHTTPServer( m_factory, &settings );

    if ( !m_server->ListenOnPort( port ) )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "LoopbackRedirectListener: Unable to listen on the loopback address" );
        GUCEF_DELETE m_server;
        m_server = GUCEF_NULL;
        GUCEF_DELETE m_factory;
        m_factory = GUCEF_NULL;
        m_expectedState.Clear();
        return false;
    }

    m_serverClosed = false;
    m_port = m_server->GetBoundPort();
    if ( 0 == m_port )
        m_port = port;
    m_deadlineMs = 0 == timeoutInMs ? 0 : GetMonotonicTimeInMs() + timeoutInMs;
    m_state = STATE_LISTENING;
    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "LoopbackRedirectListener: Listening on the loopback address, port " << m_port );
    return true;
}

/*-------------------------------------------------------------------------*/

CLoopbackRedirectListener::EState
CLoopbackRedirectListener::Poll( void )
{GUCEF_TRACE;

    UInt64 now = GetMonotonicTimeInMs();
    if ( STATE_LISTENING == m_state && 0 != m_deadlineMs && now >= m_deadlineMs )
    {
        m_state = STATE_TIMED_OUT;
        m_expectedState.Clear();
    }
    if ( !m_serverClosed && STATE_RECEIVED == m_state && now >= m_receivedAtMs + CloseDelayAfterCallbackInMs )
    {
        m_server->Close();
        m_serverClosed = true;
    }
    if ( !m_serverClosed && STATE_TIMED_OUT == m_state )
    {
        m_server->Close();
        m_serverClosed = true;
    }
    return m_state;
}

/*-------------------------------------------------------------------------*/

CLoopbackRedirectListener::EState
CLoopbackRedirectListener::GetState( void ) const
{GUCEF_TRACE;

    return m_state;
}

/*-------------------------------------------------------------------------*/

UInt16
CLoopbackRedirectListener::GetPort( void ) const
{GUCEF_TRACE;

    return m_port;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CLoopbackRedirectListener::GetOrigin( void ) const
{GUCEF_TRACE;

    return "http://127.0.0.1:" + CORE::ToString( m_port );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CLoopbackRedirectListener::GetRedirectUri( void ) const
{GUCEF_TRACE;

    return GetOrigin() + m_path;
}

/*-------------------------------------------------------------------------*/

bool
CLoopbackRedirectListener::TakeCallback( CORE::CString::StringMap& parameters )
{GUCEF_TRACE;

    parameters.clear();
    if ( STATE_RECEIVED != m_state || m_parameters.empty() )
        return false;
    parameters = m_parameters;
    m_parameters.clear();
    return true;
}

/*-------------------------------------------------------------------------*/

void
CLoopbackRedirectListener::Close( void )
{GUCEF_TRACE;

    if ( !m_serverClosed && GUCEF_NULL != m_server )
    {
        m_server->Close();
        m_serverClosed = true;
    }
    if ( STATE_LISTENING == m_state )
        m_state = STATE_CLOSED;
    m_expectedState.Clear();
    m_parameters.clear();
}

/*-------------------------------------------------------------------------*/

UInt32
CLoopbackRedirectListener::GetRejectedCount( void ) const
{GUCEF_TRACE;

    return m_rejectedCount;
}

/*-------------------------------------------------------------------------*/

UInt32
CLoopbackRedirectListener::GetRequestCount( void ) const
{GUCEF_TRACE;

    return m_requestCount;
}

/*-------------------------------------------------------------------------*/

void
CLoopbackRedirectListener::FillPage( WEB::CHttpResponseData& response, int statusCode, const CORE::CString& message ) const
{GUCEF_TRACE;

    // Plain page, no scripts and nothing from elsewhere. The message is fixed text, never part of the request
    CORE::CString page = "<!doctype html>\n<html lang=\"en\"><head><meta charset=\"utf-8\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">"
                         "<title>Sign in</title><style>body{font-family:sans-serif;margin:3em auto;max-width:30em;padding:0 1em;color:#222;background:#fff}</style></head>"
                         "<body><p>" + message + "</p></body></html>\n";
    response.statusCode = statusCode;
    response.contentType = "text/html; charset=utf-8";
    response.content.Clear( true );
    response.content.CopyFrom( GetByteLength( page ), page.C_String() );
    response.cacheability = "no-store";
    response.keepConnectionsAlive = false;
    response.otherHeaders[ "Referrer-Policy" ] = "no-referrer";
    response.otherHeaders[ "X-Content-Type-Options" ] = "nosniff";
    response.otherHeaders[ "X-Frame-Options" ] = "DENY";
    response.otherHeaders[ "Pragma" ] = "no-cache";
    response.otherHeaders[ "Content-Security-Policy" ] = "default-src 'none'; style-src 'unsafe-inline'; frame-ancestors 'none'; base-uri 'none'; form-action 'none'";
}

/*-------------------------------------------------------------------------*/

void
CLoopbackRedirectListener::Reject( WEB::CHttpResponseData& response, int statusCode, const char* text )
{GUCEF_TRACE;

    ++m_rejectedCount;
    FillPage( response, statusCode, text );
    if ( 405 == statusCode )
    {
        response.allowedMethods.clear();
        response.allowedMethods.push_back( "GET" );
    }
}

/*-------------------------------------------------------------------------*/

void
CLoopbackRedirectListener::HandleRequest( const WEB::CHttpRequestData& request, WEB::CHttpResponseData& response )
{GUCEF_TRACE;

    ++m_requestCount;

    // Once the callback is in, or the listener is done, the page does not exist any more
    if ( STATE_LISTENING != m_state )
    {
        Reject( response, 404, "Not found." );
        return;
    }

    // The Host: only the loopback address with the port of the listener. Anything else is a name that was pointed at this machine
    CORE::CString host = request.requestHost.Lowercase();
    CORE::CString portText = ":" + CORE::ToString( m_port );
    if ( host != "127.0.0.1" + portText && host != "localhost" + portText )
    {
        Reject( response, 400, "Bad request." );
        return;
    }

    if ( request.requestType != "GET" )
    {
        Reject( response, 405, "Method not allowed." );
        return;
    }

    if ( request.requestUri != m_path )
    {
        Reject( response, 404, "Not found." );
        return;
    }

    CORE::CString::StringMap parameters;
    bool duplicate = false;
    if ( !CAuthUrl::ParseQuery( request.requestUriParams, parameters, duplicate ) || duplicate )
    {
        Reject( response, 400, "Bad request." );
        return;
    }

    // The state, in constant time
    CORE::CString::StringMap::const_iterator stateParam = parameters.find( m_stateParameter );
    const CORE::CString& expected = m_expectedState.Reveal();
    UInt32 expectedSize = GetByteLength( expected );
    bool stateOk = false;
    if ( stateParam != parameters.end() && GetByteLength( (*stateParam).second ) == expectedSize )
        stateOk = CAuthCrypto::ConstantTimeEquals( (*stateParam).second.C_String(), expected.C_String(), expectedSize );
    if ( !stateOk )
    {
        Reject( response, 400, "This link is not valid." );
        return;
    }

    bool hasCode = parameters.find( "code" ) != parameters.end();
    bool hasError = parameters.find( "error" ) != parameters.end();
    bool hasOpenId = parameters.find( "openid.mode" ) != parameters.end();
    if ( !hasCode && !hasError && !hasOpenId )
    {
        Reject( response, 400, "This link is not valid." );
        return;
    }

    // The one callback
    m_parameters = parameters;
    m_state = STATE_RECEIVED;
    m_receivedAtMs = GetMonotonicTimeInMs();
    m_expectedState.Clear();
    if ( hasError && !hasCode )
        FillPage( response, 200, "The sign in was not completed. You can return to the application." );
    else
        FillPage( response, 200, "You can return to the application." );
    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "LoopbackRedirectListener: The callback arrived" );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUTH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
