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

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CDATETIME_H
#include "gucefCORE_CDateTime.h"
#define GUCEF_CORE_CDATETIME_H
#endif /* GUCEF_CORE_CDATETIME_H ? */

#ifndef GUCEF_CORE_CREDACTOR_H
#include "gucefCORE_CRedactor.h"
#define GUCEF_CORE_CREDACTOR_H
#endif /* GUCEF_CORE_CREDACTOR_H ? */

#ifndef GUCEF_CORE_CJSONLOGGINGFORMATTER_H
#include "gucefCORE_CJsonLoggingFormatter.h"
#define GUCEF_CORE_CJSONLOGGINGFORMATTER_H
#endif /* GUCEF_CORE_CJSONLOGGINGFORMATTER_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_WEB_CHTTPSERVER_H
#include "gucefWEB_CHTTPServer.h"
#define GUCEF_WEB_CHTTPSERVER_H
#endif /* GUCEF_WEB_CHTTPSERVER_H ? */

#ifndef GUCEF_WEB_CHTTPSERVEREXCHANGE_H
#include "gucefWEB_CHttpServerExchange.h"
#define GUCEF_WEB_CHTTPSERVEREXCHANGE_H
#endif /* GUCEF_WEB_CHTTPSERVEREXCHANGE_H ? */

#include "gucefWEB_CServiceHealthHttpEndpoints.h"

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

const CORE::CString CServiceHealthHttpEndpoints::DefaultLivenessPath = "/health/live";
const CORE::CString CServiceHealthHttpEndpoints::DefaultReadinessPath = "/health/ready";
const CORE::CString CServiceHealthHttpEndpoints::DefaultStartupPath = "/health/started";
const CORE::CString CServiceHealthHttpEndpoints::DefaultInfoPath = "/info";

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static CORE::CString
JsonText( const CORE::CString& text )
{GUCEF_TRACE;

    return "\"" + CORE::CJsonLoggingFormatter::EscapeJson( text ) + "\"";
}

/*-------------------------------------------------------------------------*/

static CORE::CString
UtcMsToIso8601( UInt64 utcMs )
{GUCEF_TRACE;

    CORE::CDateTime dateTime;
    dateTime.FromUnixEpochBasedTicksInMillisecs( utcMs );
    return dateTime.ToIso8601DateTimeString( true, true );
}

/*-------------------------------------------------------------------------*/

/** Whether the text is a dotted IPv4 address of the loopback network 127.0.0.0/8 */
static bool
IsLoopbackIPv4Address( const CORE::CString& address )
{GUCEF_TRACE;

    CORE::CString::StringVector parts = address.ParseElements( '.', true );
    if ( 4 != parts.size() )
        return false;
    for ( size_t i=0; i<parts.size(); ++i )
    {
        const CORE::CString& part = parts[ i ];
        if ( part.IsNULLOrEmpty() || part.Length() > 3 )
            return false;
        for ( UInt32 c=0; c<part.Length(); ++c )
            if ( part[ c ] < '0' || part[ c ] > '9' )
                return false;
        if ( CORE::StringToUInt32( part ) > 255 )
            return false;
    }
    return parts[ 0 ] == "127";
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CServiceHealthHttpEndpoints::CServiceHealthHttpEndpoints( void )
    : CIHttpRequestTakeoverHandler()
    , m_lock()
    , m_registry( &CORE::CCoreGlobal::Instance()->GetServiceHealthRegistry() )
    , m_livenessPath( DefaultLivenessPath )
    , m_readinessPath( DefaultReadinessPath )
    , m_startupPath( DefaultStartupPath )
    , m_infoPath( DefaultInfoPath )
    , m_checkDetails( CHECKDETAILS_AUTOMATIC )
    , m_server( GUCEF_NULL )
    , m_registeredPaths()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CServiceHealthHttpEndpoints::CServiceHealthHttpEndpoints( CORE::CServiceHealthRegistry& registry )
    : CIHttpRequestTakeoverHandler()
    , m_lock()
    , m_registry( &registry )
    , m_livenessPath( DefaultLivenessPath )
    , m_readinessPath( DefaultReadinessPath )
    , m_startupPath( DefaultStartupPath )
    , m_infoPath( DefaultInfoPath )
    , m_checkDetails( CHECKDETAILS_AUTOMATIC )
    , m_server( GUCEF_NULL )
    , m_registeredPaths()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CServiceHealthHttpEndpoints::~CServiceHealthHttpEndpoints()
{GUCEF_TRACE;

    Unregister();
}

/*-------------------------------------------------------------------------*/

void
CServiceHealthHttpEndpoints::SetPaths( const CORE::CString& livenessPath  ,
                                       const CORE::CString& readinessPath ,
                                       const CORE::CString& startupPath   ,
                                       const CORE::CString& infoPath      )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_livenessPath = livenessPath;
    m_readinessPath = readinessPath;
    m_startupPath = startupPath;
    m_infoPath = infoPath;
}

/*-------------------------------------------------------------------------*/

void
CServiceHealthHttpEndpoints::SetCheckDetails( ECheckDetails checkDetails )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_checkDetails = checkDetails;
}

/*-------------------------------------------------------------------------*/

CServiceHealthHttpEndpoints::ECheckDetails
CServiceHealthHttpEndpoints::GetCheckDetails( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_checkDetails;
}

/*-------------------------------------------------------------------------*/

bool
CServiceHealthHttpEndpoints::IncludesCheckDetails( ECheckDetails checkDetails, const CORE::CString& bindAddress )
{GUCEF_TRACE;

    switch ( checkDetails )
    {
        case CHECKDETAILS_INCLUDED: return true;
        case CHECKDETAILS_OMITTED: return false;
        default: return IsLoopbackIPv4Address( bindAddress.Trim( true ).Trim( false ) );
    }
}

/*-------------------------------------------------------------------------*/

bool
CServiceHealthHttpEndpoints::RegisterWith( CHTTPServer& server )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    if ( GUCEF_NULL != m_server )
        return false;

    const CORE::CString* paths[ 4 ] = { &m_livenessPath, &m_readinessPath, &m_startupPath, &m_infoPath };
    m_server = &server;
    for ( UInt32 i=0; i<4; ++i )
    {
        if ( paths[ i ]->IsNULLOrEmpty() )
            continue;
        // a health request has no body worth reading
        if ( !server.RegisterRequestTakeoverHandler( *paths[ i ], this, 1024 ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ServiceHealthHttpEndpoints: The path " << *paths[ i ] << " is taken on the HTTP server, the health endpoints are not served" );
            UnregisterWhileLocked();
            return false;
        }
        m_registeredPaths.push_back( *paths[ i ] );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
CServiceHealthHttpEndpoints::UnregisterWhileLocked( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_server )
    {
        for ( size_t i=0; i<m_registeredPaths.size(); ++i )
            m_server->UnregisterRequestTakeoverHandler( m_registeredPaths[ i ] );
    }
    m_registeredPaths.clear();
    m_server = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

void
CServiceHealthHttpEndpoints::Unregister( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    UnregisterWhileLocked();
}

/*-------------------------------------------------------------------------*/

bool
CServiceHealthHttpEndpoints::IsRegistered( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return GUCEF_NULL != m_server;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CServiceHealthHttpEndpoints::BuildHealthBody( const CORE::CServiceHealthRegistry::SHealthReport& report )
{GUCEF_TRACE;

    return BuildHealthBody( report, true );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CServiceHealthHttpEndpoints::BuildHealthBody( const CORE::CServiceHealthRegistry::SHealthReport& report ,
                                              bool includeCheckDetails                                )
{GUCEF_TRACE;

    CORE::CString body = "{\"status\":" + JsonText( CORE::ServiceHealthStateToString( report.state ) );
    if ( report.isStale )
        body += ",\"stale\":true";
    if ( !includeCheckDetails )
    {
        body += "}";
        return body;
    }
    body += ",\"checks\":[";
    for ( size_t i=0; i<report.checks.size(); ++i )
    {
        const CORE::CServiceHealthRegistry::SCheckOutcome& outcome = report.checks[ i ];
        if ( i > 0 )
            body += ',';
        body += "{\"name\":" + JsonText( outcome.name ) +
                ",\"status\":" + JsonText( CORE::ServiceHealthStateToString( outcome.state ) ) +
                ",\"detail\":" + JsonText( CORE::CRedactor::ForLogLine().RedactText( outcome.detail ) ) +
                ",\"since\":" + JsonText( UtcMsToIso8601( outcome.sinceUtcMs ) ) + "}";
    }
    body += "]}";
    return body;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CServiceHealthHttpEndpoints::BuildInfoBody( const CORE::CValueList& info )
{GUCEF_TRACE;

    CORE::CString body = "{";
    for ( UInt32 i=0; i<info.GetKeyCount(); ++i )
    {
        if ( i > 0 )
            body += ',';
        body += JsonText( info.GetKey( i ).AsString() ) + ":" + JsonText( info.GetValue( i ).AsString() );
    }
    body += "}";
    return body;
}

/*-------------------------------------------------------------------------*/

CIHttpRequestTakeoverHandler::EHeaderDecision
CServiceHealthHttpEndpoints::OnHttpRequestHeaders( CHTTPServer& server                 ,
                                                   const CHttpRequestData& request     ,
                                                   const CHttpServerExchange& exchange ,
                                                   CHttpResponseData& response         )
{GUCEF_TRACE;

    if ( "GET" == request.requestType || "HEAD" == request.requestType )
        return TAKEOVER_CONTINUE;
    response.statusCode = 405;
    response.allowedMethods.push_back( "GET" );
    response.allowedMethods.push_back( "HEAD" );
    return TAKEOVER_REJECT;
}

/*-------------------------------------------------------------------------*/

void
CServiceHealthHttpEndpoints::OnHttpRequest( CHTTPServer& server             ,
                                            const CHttpRequestData& request ,
                                            CHttpServerExchange& exchange   )
{GUCEF_TRACE;

    CORE::CString path;
    CORE::CServiceHealthRegistry* registry = GUCEF_NULL;
    ECheckDetails checkDetails = CHECKDETAILS_AUTOMATIC;
    bool isLiveness = false, isReadiness = false, isStartup = false, isInfo = false;
    {
        MT::CScopeMutex lock( m_lock );
        registry = m_registry;
        checkDetails = m_checkDetails;
        isLiveness = request.requestUri == m_livenessPath;
        isReadiness = request.requestUri == m_readinessPath;
        isStartup = request.requestUri == m_startupPath;
        isInfo = request.requestUri == m_infoPath;
    }

    CHttpResponseData response;
    response.contentType = "application/json";
    response.cacheability = "no-store";
    CORE::CString body;
    if ( isInfo )
    {
        response.statusCode = 200;
        body = BuildInfoBody( registry->GetInfo() );
    }
    else
    if ( isLiveness || isReadiness || isStartup )
    {
        CORE::CServiceHealthRegistry::SHealthReport report;
        if ( isLiveness )
            registry->GetLiveness( report );
        else
        if ( isReadiness )
            registry->GetReadiness( report );
        else
            registry->GetStartup( report );
        response.statusCode = report.Passes() ? 200 : 503;

        // the server's address of the moment: it may listen again elsewhere after the endpoints were registered
        bool includeCheckDetails = CHECKDETAILS_INCLUDED == checkDetails;
        if ( CHECKDETAILS_AUTOMATIC == checkDetails )
        {
            CHttpServerSettings settings;
            server.GetSettings( settings );
            includeCheckDetails = IncludesCheckDetails( checkDetails, settings.bindAddress );
        }
        body = BuildHealthBody( report, includeCheckDetails );
    }
    else
    {
        response.statusCode = 404;
    }

    // HEAD: the status and the headers, no body
    if ( "HEAD" != request.requestType && !body.IsNULLOrEmpty() )
        response.content.CopyFrom( body.ByteSize() - 1, body.C_String() );
    exchange.SendResponse( response );
}

/*-------------------------------------------------------------------------*/

void
CServiceHealthHttpEndpoints::OnHttpExchangeClosed( CHTTPServer& server           ,
                                                   CHttpServerExchange& exchange )
{GUCEF_TRACE;

    // every exchange is answered within OnHttpRequest(): nothing is kept
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
