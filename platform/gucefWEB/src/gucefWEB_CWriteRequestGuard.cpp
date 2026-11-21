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

#include <string.h>

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CRANDOM_H
#include "gucefCORE_CRandom.h"
#define GUCEF_CORE_CRANDOM_H
#endif /* GUCEF_CORE_CRANDOM_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_WEB_CURLPATHTEMPLATE_H
#include "gucefWEB_CUrlPathTemplate.h"
#define GUCEF_WEB_CURLPATHTEMPLATE_H
#endif /* GUCEF_WEB_CURLPATHTEMPLATE_H ? */

#ifndef GUCEF_WEB_CCHROMEDEVTOOLSCLIENT_H
#include "gucefWEB_CChromeDevToolsClient.h"
#define GUCEF_WEB_CCHROMEDEVTOOLSCLIENT_H
#endif /* GUCEF_WEB_CCHROMEDEVTOOLSCLIENT_H ? */

#ifndef GUCEF_WEB_CNAVIGATIONGUARD_H
#include "gucefWEB_CNavigationGuard.h"
#define GUCEF_WEB_CNAVIGATIONGUARD_H
#endif /* GUCEF_WEB_CNAVIGATIONGUARD_H ? */

#include "gucefWEB_CWriteRequestGuard.h"

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

/** The refusals that are kept for GetRecentRefusals() */
static const size_t MaxRecentRefusals = 64;

/*-------------------------------------------------------------------------*/

static UInt32
ByteLength( const CORE::CString& text )
{
    return text.IsNULLOrEmpty() ? 0 : text.ByteSize() - 1;
}

/*-------------------------------------------------------------------------*/

/** The parts of an http(s) address as the guard compares them; false for anything else and for an address with user info */
static bool
SplitUrl( const CORE::CString& url, CORE::CString& scheme, CORE::CString& host, CORE::CString& path, bool& hasQuery, CORE::CString& query )
{
    hasQuery = false;
    query.Clear();
    const char* u = url.C_String();
    const UInt32 len = ByteLength( url );
    const char* sep = GUCEF_NULL != u ? strstr( u, "://" ) : GUCEF_NULL;
    if ( GUCEF_NULL == sep || sep == u )
        return false;
    const UInt32 schemeEnd = (UInt32) ( sep - u );
    scheme = CORE::CString( u, schemeEnd ).Lowercase();
    if ( scheme != "http" && scheme != "https" )
        return false;
    const UInt32 hostStart = schemeEnd + 3;
    UInt32 hostEnd = hostStart;
    while ( hostEnd < len && u[ hostEnd ] != '/' && u[ hostEnd ] != '?' && u[ hostEnd ] != '#' )
    {
        if ( u[ hostEnd ] == '@' || u[ hostEnd ] == '\\' )
            return false;
        ++hostEnd;
    }
    if ( hostEnd == hostStart )
        return false;
    host = CORE::CString( u + hostStart, hostEnd - hostStart ).Lowercase();
    UInt32 pathEnd = hostEnd;
    while ( pathEnd < len && u[ pathEnd ] != '?' && u[ pathEnd ] != '#' )
        ++pathEnd;
    path = pathEnd > hostEnd ? CORE::CString( u + hostEnd, pathEnd - hostEnd ) : CORE::CString( "/" );
    if ( pathEnd < len && u[ pathEnd ] == '?' )
    {
        UInt32 queryEnd = pathEnd + 1;
        while ( queryEnd < len && u[ queryEnd ] != '#' )
            ++queryEnd;
        query = CORE::CString( u + pathEnd + 1, queryEnd - pathEnd - 1 );
        hasQuery = !query.IsNULLOrEmpty();
    }
    return true;
}

/*-------------------------------------------------------------------------*/

static CORE::CString
HostNameOf( const CORE::CString& hostAndPort )
{
    if ( !hostAndPort.IsNULLOrEmpty() && hostAndPort.C_String()[ 0 ] == '[' )
        return hostAndPort;
    const Int32 colon = hostAndPort.HasChar( ':', true );
    return colon >= 0 ? hostAndPort.SubstrToIndex( (UInt32) colon, true ) : hostAndPort;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CEvent CWriteRequestGuard::WriteRequestRefusedEvent = "GUCEF::WEB::CWriteRequestGuard::WriteRequestRefusedEvent";
const CORE::CString CWriteRequestGuard::ProbeHost = "write-guard-probe.invalid";

/*-------------------------------------------------------------------------*/

void
CWriteRequestGuard::RegisterEvents( void )
{GUCEF_TRACE;

    WriteRequestRefusedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CWriteRequestGuard::CWriteRequestGuard( void )
    : CORE::CNotifier()
    , m_settings()
    , m_allow()
    , m_host( GUCEF_NULL )
    , m_client( GUCEF_NULL )
    , m_enforcing( false )
    , m_probePath()
    , m_probeSeen( false )
    , m_refusedCount( 0 )
    , m_allowedWriteCount( 0 )
    , m_passedReadCount( 0 )
    , m_recent()
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CWriteRequestGuard::~CWriteRequestGuard()
{GUCEF_TRACE;

    // the navigation guard must not ask a guard that is gone
    if ( GUCEF_NULL != m_host )
        m_host->ReleaseWriteRequestGuard( this );
    m_host = GUCEF_NULL;
    m_client = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

bool
CWriteRequestGuard::SetSettings( const CWriteRequestGuardSettings& settings )
{GUCEF_TRACE;

    TWriteRequestRuleVector allow;
    for ( size_t i=0; i<settings.allow.size(); ++i )
    {
        CWriteRequestRule rule;
        CORE::CString problem;
        if ( !CWriteRequestRule::Parse( settings.allow[ i ], rule, &problem ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "WriteRequestGuard: The allow rule \"" << settings.allow[ i ] << "\" is not a write request rule: " << problem );
            return false;
        }
        allow.push_back( rule );
    }
    m_settings = settings;
    m_allow.swap( allow );
    return true;
}

/*-------------------------------------------------------------------------*/

const CWriteRequestGuardSettings&
CWriteRequestGuard::GetSettings( void ) const
{GUCEF_TRACE;

    return m_settings;
}

/*-------------------------------------------------------------------------*/

bool
CWriteRequestGuard::IsSafeMethod( const CORE::CString& method )
{GUCEF_TRACE;

    return method == "GET" || method == "HEAD" || method == "OPTIONS";
}

/*-------------------------------------------------------------------------*/

bool
CWriteRequestGuard::IsAllowed( const CORE::CString& method, const CORE::CString& url, CORE::CString* reason ) const
{GUCEF_TRACE;

    CORE::CString why;
    bool allowed = false;
    CORE::CString scheme, host, path, query;
    bool hasQuery = false;
    if ( IsSafeMethod( method ) )
        allowed = true;
    else
    if ( !SplitUrl( url, scheme, host, path, hasQuery, query ) )
        why = CWriteRequestRefusal::ReasonMalformed;
    else
    {
        CORE::CString::StringVector segments;
        const bool strict = CWriteRequestRule::SplitStrictPath( path, segments, false );
        bool anyMethod = false, anyHost = false, schemeMiss = false, anyPath = false;
        for ( size_t i=0; i<m_allow.size() && !allowed; ++i )
        {
            const CWriteRequestRule& rule = m_allow[ i ];
            if ( !rule.MatchesMethod( method ) )
                continue;
            anyMethod = true;
            if ( !rule.MatchesHost( host ) )
                continue;
            if ( !rule.MatchesScheme( scheme ) )
            {
                schemeMiss = true;
                continue;
            }
            anyHost = true;
            if ( !strict || !rule.MatchesPath( segments ) )
                continue;
            anyPath = true;
            if ( rule.MatchesQuery( hasQuery, query ) )
                allowed = true;
        }
        if ( !allowed )
        {
            if ( !anyMethod )
                why = CWriteRequestRefusal::ReasonMethodNotAllowed;
            else
            if ( !anyHost )
                why = schemeMiss ? CWriteRequestRefusal::ReasonSchemeNotAllowed : CWriteRequestRefusal::ReasonHostNotAllowed;
            else
            if ( !strict )
                why = CWriteRequestRefusal::ReasonAmbiguousPath;
            else
            if ( !anyPath )
                why = CWriteRequestRefusal::ReasonPathNotAllowed;
            else
                why = CWriteRequestRefusal::ReasonQueryNotAllowed;
        }
    }
    if ( GUCEF_NULL != reason )
        *reason = why;
    return allowed;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CWriteRequestGuard::MakeRedactedUrl( const CORE::CString& url, CORE::CString* host, CORE::CString* pathTemplate )
{GUCEF_TRACE;

    CORE::CString scheme, hostPart, path, query;
    bool hasQuery = false;
    if ( !SplitUrl( url, scheme, hostPart, path, hasQuery, query ) )
    {
        if ( GUCEF_NULL != host )
            host->Clear();
        if ( GUCEF_NULL != pathTemplate )
            pathTemplate->Clear();
        const Int32 colon = url.HasChar( ':', true );
        return colon > 0 && colon < 16 ? url.SubstrToIndex( (UInt32) colon + 1, true ).Lowercase() : CORE::CString( "(not an http(s) address)" );
    }
    const CORE::CString templ = CUrlPathTemplate::TemplatePath( path );
    if ( GUCEF_NULL != host )
        *host = hostPart;
    if ( GUCEF_NULL != pathTemplate )
        *pathTemplate = templ;
    CORE::CString result = scheme + "://" + hostPart + templ;
    if ( hasQuery )
    {
        CORE::CString::StringVector items = query.ParseElements( '&', false );
        for ( size_t i=0; i<items.size(); ++i )
        {
            const Int32 eq = items[ i ].HasChar( '=', true );
            result += ( i == 0 ? "?" : "&" );
            result += ( eq >= 0 ? items[ i ].SubstrToIndex( (UInt32) eq, true ) : items[ i ] ) + "={redacted}";
        }
    }
    return result;
}

/*-------------------------------------------------------------------------*/

void
CWriteRequestGuard::Report( const CORE::CString& method, const CORE::CString& url, const CORE::CString& resourceType,
                            const CORE::CString& frameId, bool isMainFrame, bool redirected, const CORE::CString& reason )
{GUCEF_TRACE;

    CWriteRequestRefusal refusal;
    refusal.method = method;
    refusal.url = MakeRedactedUrl( url, &refusal.host, &refusal.pathTemplate );
    refusal.resourceType = resourceType;
    refusal.frameId = frameId;
    refusal.isMainFrame = isMainFrame;
    refusal.redirected = redirected;
    refusal.reason = reason;

    ++m_refusedCount;
    if ( m_recent.size() >= MaxRecentRefusals )
        m_recent.erase( m_recent.begin() );
    m_recent.push_back( refusal );

    GUCEF_LOG( CORE::LOGLEVEL_IMPORTANT, "WriteRequestGuard: Refused " << method << " " << refusal.url << " (" << resourceType << ", " << reason <<
               ( redirected ? ", a redirect" : "" ) << ( isMainFrame ? ", a tab" : "" ) << ")" );

    TWriteRequestRefusalEventData eventData( refusal );
    NotifyObservers( WriteRequestRefusedEvent, &eventData );
}

/*-------------------------------------------------------------------------*/

bool
CWriteRequestGuard::CheckPausedRequest( const CORE::CString& method, const CORE::CString& url, const CORE::CString& resourceType,
                                        const CORE::CString& frameId, bool isMainFrame, bool redirected )
{GUCEF_TRACE;

    // the probe of VerifyEnforcing(), and anything else sent to its host, never passes
    CORE::CString scheme, host, path, query;
    bool hasQuery = false;
    if ( SplitUrl( url, scheme, host, path, hasQuery, query ) && HostNameOf( host ) == ProbeHost )
    {
        if ( !m_probePath.IsNULLOrEmpty() && path == m_probePath )
        {
            m_probeSeen = true;
            return false;
        }
        Report( method, url, resourceType, frameId, isMainFrame, redirected, CWriteRequestRefusal::ReasonHostNotAllowed );
        return false;
    }

    CORE::CString reason;
    if ( IsAllowed( method, url, &reason ) )
    {
        if ( IsSafeMethod( method ) )
            ++m_passedReadCount;
        else
            ++m_allowedWriteCount;
        return true;
    }
    Report( method, url, resourceType, frameId, isMainFrame, redirected, reason );
    return false;
}

/*-------------------------------------------------------------------------*/

void
CWriteRequestGuard::SetHost( CNavigationGuard* host, CChromeDevToolsClient* client, bool enforcing )
{GUCEF_TRACE;

    m_host = host;
    m_client = GUCEF_NULL != host ? client : GUCEF_NULL;
    m_enforcing = GUCEF_NULL != m_client && enforcing;
}

/*-------------------------------------------------------------------------*/

bool
CWriteRequestGuard::IsEnforcing( void ) const
{GUCEF_TRACE;

    return m_enforcing && GUCEF_NULL != m_host && GUCEF_NULL != m_client && m_client->IsConnected() && m_host->IsStarted();
}

/*-------------------------------------------------------------------------*/

bool
CWriteRequestGuard::VerifyEnforcing( const CORE::CString& pageSessionId, UInt32 timeoutInMs )
{GUCEF_TRACE;

    if ( !IsEnforcing() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "WriteRequestGuard: The guard is not in force: give it to a started navigation guard first" );
        return false;
    }
    if ( pageSessionId.IsNULLOrEmpty() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "WriteRequestGuard: The check needs the session of the page" );
        return false;
    }

    // a path of our own making (hex only), so that only this probe counts. A no-cors request must follow redirects (the
    // fetch standard refuses it before sending otherwise), which does not matter: the request never gets that far
    UInt8 nonce[ 12 ];
    if ( !CORE::CRandom::GetRandomBytes( nonce, (UInt32) sizeof( nonce ) ) )
    {
        const UInt64 ticks = MT::PrecisionTickCount();
        memset( nonce, 0, sizeof( nonce ) );
        memcpy( nonce, &ticks, sizeof( ticks ) );
    }
    static const char* const hex = "0123456789abcdef";
    CORE::CString path = "/";
    for ( UInt32 i=0; i<sizeof( nonce ); ++i )
    {
        path += hex[ nonce[ i ] >> 4 ];
        path += hex[ nonce[ i ] & 15 ];
    }
    m_probePath = path;
    m_probeSeen = false;

    const CORE::CString expression =
        "(function(){try{return fetch('https://" + ProbeHost + path + "',{method:'POST',body:'probe',mode:'no-cors',credentials:'omit',"
        "cache:'no-store'}).then(function(){return 'sent';},function(){return 'refused';});}catch(e){return 'error';}})()";
    CORE::CDataNode params( GUCEF_DATATYPE_OBJECT );
    params.SetAttribute( "expression", CORE::CVariant( expression ) );
    params.SetAttribute( "awaitPromise", true );
    params.SetAttribute( "returnByValue", true );
    CORE::CDataNode result;
    COM::SJsonRpcError error;
    const bool answered = m_client->SendCommandOnSessionAndWait( pageSessionId, "Runtime.evaluate", &params, result, &error, timeoutInMs );
    CORE::CString outcome;
    const CORE::CDataNode* value = result.FindChild( "result" );
    if ( GUCEF_NULL != value )
        outcome = value->GetAttributeValue( "value" ).AsString( CORE::CString::Empty, false );

    const bool passed = m_probeSeen && outcome != "sent" && IsEnforcing();
    m_probePath.Clear();
    m_probeSeen = false;
    if ( passed )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "WriteRequestGuard: The guard is in force for the page: its probe request was failed before it left the browser" );
    }
    else
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "WriteRequestGuard: The guard could not be shown to be in force for the page (" << ( answered ? "the probe request was not seen" : "the page did not answer" ) <<
                         ", page outcome \"" << outcome << "\")" );
    }
    return passed;
}

/*-------------------------------------------------------------------------*/

UInt32
CWriteRequestGuard::GetRefusedCount( void ) const
{GUCEF_TRACE;

    return m_refusedCount;
}

/*-------------------------------------------------------------------------*/

UInt32
CWriteRequestGuard::GetAllowedWriteCount( void ) const
{GUCEF_TRACE;

    return m_allowedWriteCount;
}

/*-------------------------------------------------------------------------*/

UInt32
CWriteRequestGuard::GetPassedReadCount( void ) const
{GUCEF_TRACE;

    return m_passedReadCount;
}

/*-------------------------------------------------------------------------*/

const TWriteRequestRefusalVector&
CWriteRequestGuard::GetRecentRefusals( void ) const
{GUCEF_TRACE;

    return m_recent;
}

/*-------------------------------------------------------------------------*/

void
CWriteRequestGuard::ClearRecentRefusals( void )
{GUCEF_TRACE;

    m_recent.clear();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
