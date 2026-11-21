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

#ifndef GUCEF_CORE_CGLOBPATTERN_H
#include "gucefCORE_CGlobPattern.h"
#define GUCEF_CORE_CGLOBPATTERN_H
#endif /* GUCEF_CORE_CGLOBPATTERN_H ? */

#ifndef GUCEF_WEB_CURLPATHTEMPLATE_H
#include "gucefWEB_CUrlPathTemplate.h"
#define GUCEF_WEB_CURLPATHTEMPLATE_H
#endif /* GUCEF_WEB_CURLPATHTEMPLATE_H ? */

#include "gucefWEB_CNavigationGuard.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace WEB {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CEvent CNavigationGuard::NavigationRefusedEvent = "GUCEF::WEB::CNavigationGuard::NavigationRefusedEvent";

/** The refusals that are kept for GetRecentRefusals() */
static const size_t MaxRecentRefusals = 64;

/*-------------------------------------------------------------------------*/

void
CNavigationGuard::RegisterEvents( void )
{GUCEF_TRACE;

    NavigationRefusedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CNavigationGuard::CNavigationGuard( void )
    : CORE::CObservingNotifier()
    , m_settings()
    , m_allow()
    , m_deny()
    , m_client( GUCEF_NULL )
    , m_pageTargets()
    , m_reportedCommits()
    , m_refusedCount( 0 )
    , m_allowedCount( 0 )
    , m_recent()
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CNavigationGuard::~CNavigationGuard()
{GUCEF_TRACE;

    Stop();
}

/*-------------------------------------------------------------------------*/

bool
CNavigationGuard::SetSettings( const CNavigationGuardSettings& settings )
{GUCEF_TRACE;

    TNavigationRuleVector allow;
    TNavigationRuleVector deny;
    for ( size_t i=0; i<settings.allow.size(); ++i )
    {
        CNavigationRule rule;
        if ( !CNavigationRule::Parse( settings.allow[ i ], rule ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "NavigationGuard: The allow rule \"" << settings.allow[ i ] << "\" is not a host or host/path rule" );
            return false;
        }
        allow.push_back( rule );
    }
    for ( size_t i=0; i<settings.deny.size(); ++i )
    {
        CNavigationRule rule;
        if ( !CNavigationRule::Parse( settings.deny[ i ], rule ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "NavigationGuard: The deny rule \"" << settings.deny[ i ] << "\" is not a host or host/path rule" );
            return false;
        }
        deny.push_back( rule );
    }
    m_settings = settings;
    m_allow.swap( allow );
    m_deny.swap( deny );
    return true;
}

/*-------------------------------------------------------------------------*/

const CNavigationGuardSettings&
CNavigationGuard::GetSettings( void ) const
{GUCEF_TRACE;

    return m_settings;
}

/*-------------------------------------------------------------------------*/

bool
CNavigationGuard::IsNeverRecord( const CORE::CString& hostName, const CORE::CString& hostAndPort, const CORE::CString& path ) const
{GUCEF_TRACE;

    for ( size_t i=0; i<m_settings.neverRecordHosts.size(); ++i )
    {
        const CORE::CString& pattern = m_settings.neverRecordHosts[ i ];
        if ( CORE::CGlobPattern::IsMatch( pattern, hostName, false ) || CORE::CGlobPattern::IsMatch( pattern, hostAndPort, false ) )
            return true;
    }
    const CORE::CString lowerPath = path.Lowercase();
    for ( size_t i=0; i<m_settings.neverRecordPathPatterns.size(); ++i )
    {
        const CORE::CString part = m_settings.neverRecordPathPatterns[ i ].Lowercase();
        if ( !part.IsNULLOrEmpty() && lowerPath.HasSubstr( part, true ) >= 0 )
            return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

static CORE::CString
SchemeOf( const CORE::CString& url )
{
    const Int32 colon = url.HasChar( ':', true );
    return colon > 0 ? url.SubstrToIndex( (UInt32) colon, true ).Lowercase() : CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

static CORE::CString
HostNameOf( const CORE::CString& hostAndPort )
{
    // [::1]:8080, example.com:8080, example.com
    if ( !hostAndPort.IsNULLOrEmpty() && hostAndPort.C_String()[ 0 ] == '[' )
    {
        const Int32 close = hostAndPort.HasChar( ']', true );
        return close > 0 ? hostAndPort.SubstrToIndex( (UInt32) close + 1, true ) : hostAndPort;
    }
    const Int32 colon = hostAndPort.HasChar( ':', false );
    return colon >= 0 ? hostAndPort.SubstrToIndex( (UInt32) colon, true ) : hostAndPort;
}

/*-------------------------------------------------------------------------*/

bool
CNavigationGuard::IsAllowed( const CORE::CString& url, bool isMainFrame, CORE::CString* reason ) const
{GUCEF_TRACE;

    CORE::CString why;
    bool allowed = false;
    const CORE::CString scheme = SchemeOf( url );
    if ( !isMainFrame && !m_settings.guardSubframes )
        allowed = true;
    else
    if ( scheme == "about" )
    {
        const CORE::CString lower = url.Lowercase();
        allowed = lower == "about:blank" || lower.StartsWith( "about:blank#" ) || lower.StartsWith( "about:blank?" ) || lower == "about:srcdoc";
        if ( !allowed )
            why = CNavigationRefusal::ReasonScheme;
    }
    else
    if ( scheme != "http" && scheme != "https" )
        why = scheme.IsNULLOrEmpty() ? CNavigationRefusal::ReasonMalformed : CNavigationRefusal::ReasonScheme;
    else
    {
        CUrlPathTemplate parsed;
        if ( !CUrlPathTemplate::Parse( url, parsed ) || parsed.host.IsNULLOrEmpty() )
            why = CNavigationRefusal::ReasonMalformed;
        else
        {
            const CORE::CString hostAndPort = parsed.host.Lowercase();
            const CORE::CString hostName = HostNameOf( hostAndPort );
            CORE::CString rawPath = parsed.path;
            const Int32 fragment = rawPath.HasChar( '#', true );
            if ( fragment >= 0 )
                rawPath = rawPath.SubstrToIndex( (UInt32) fragment, true );
            bool ambiguous = false;
            const CORE::CString path = CNavigationRule::NormalisePath( rawPath, ambiguous );

            // a deny rule also sees the path with encoded slashes and backslashes taken as slashes
            CORE::CString loosePath = path;
            if ( ambiguous )
            {
                CORE::CString slashed = rawPath.ReplaceSubstr( "%2F", "/" ).ReplaceSubstr( "%2f", "/" ).ReplaceSubstr( "%5C", "/" ).ReplaceSubstr( "%5c", "/" ).ReplaceChar( '\\', '/' );
                bool stillAmbiguous = false;
                loosePath = CNavigationRule::NormalisePath( slashed, stillAmbiguous );
            }
            bool denied = false;
            for ( size_t i=0; i<m_deny.size() && !denied; ++i )
                denied = m_deny[ i ].Matches( hostName, hostAndPort, path, false ) || m_deny[ i ].Matches( hostName, hostAndPort, loosePath, false );

            bool named = false;
            for ( size_t i=0; i<m_allow.size() && !named && !ambiguous; ++i )
                named = m_allow[ i ].Matches( hostName, hostAndPort, path, true );

            if ( denied )
                why = CNavigationRefusal::ReasonDenied;
            else
            if ( !m_allow.empty() && !named )
                why = CNavigationRefusal::ReasonNotAllowed;
            else
            if ( !named && m_settings.refuseNeverRecordUnlessAllowed && ( IsNeverRecord( hostName, hostAndPort, path ) || IsNeverRecord( hostName, hostAndPort, loosePath ) ) )
                why = CNavigationRefusal::ReasonNeverRecord;
            else
                allowed = true;
        }
    }
    if ( GUCEF_NULL != reason )
        *reason = why;
    return allowed;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CNavigationGuard::MakeUrlTemplate( const CORE::CString& url, CORE::CString* host )
{GUCEF_TRACE;

    const CORE::CString scheme = SchemeOf( url );
    CUrlPathTemplate parsed;
    if ( ( scheme == "http" || scheme == "https" ) && CUrlPathTemplate::Parse( url, parsed ) )
    {
        if ( GUCEF_NULL != host )
            *host = parsed.host;
        return parsed.scheme + "://" + parsed.host + parsed.pathTemplate;
    }
    if ( GUCEF_NULL != host )
        host->Clear();
    return scheme.IsNULLOrEmpty() ? CORE::CString( "(not a URL)" ) : scheme + ":";
}

/*-------------------------------------------------------------------------*/

void
CNavigationGuard::Report( const CORE::CString& url, const CORE::CString& frameId, bool isMainFrame, const CORE::CString& reason, const CORE::CString& source, bool blocked )
{GUCEF_TRACE;

    CNavigationRefusal refusal;
    refusal.urlTemplate = MakeUrlTemplate( url, &refusal.host );
    refusal.frameId = frameId;
    refusal.isMainFrame = isMainFrame;
    refusal.reason = reason;
    refusal.source = source;
    refusal.blocked = blocked;

    ++m_refusedCount;
    if ( m_recent.size() >= MaxRecentRefusals )
        m_recent.erase( m_recent.begin() );
    m_recent.push_back( refusal );

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "NavigationGuard: " << ( blocked ? "Refused " : "Reported " ) << ( isMainFrame ? "a tab's" : "a frame's" ) <<
               " navigation to " << refusal.urlTemplate << " (" << reason << ", " << source << ")" );

    TNavigationRefusalEventData eventData( refusal );
    NotifyObservers( NavigationRefusedEvent, &eventData );
}

/*-------------------------------------------------------------------------*/

bool
CNavigationGuard::CheckNavigate( const CORE::CString& url )
{GUCEF_TRACE;

    CORE::CString reason;
    if ( IsAllowed( url, true, &reason ) )
    {
        ++m_allowedCount;
        return true;
    }
    Report( url, CORE::CString::Empty, true, reason, CNavigationRefusal::SourceNavigate, true );
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CNavigationGuard::Start( CChromeDevToolsClient& browserClient )
{GUCEF_TRACE;

    if ( IsStarted() )
        return true;
    if ( !browserClient.IsConnected() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "NavigationGuard: The guard needs a connection to the browser" );
        return false;
    }
    m_client = &browserClient;
    TEventCallback callback( this, &CNavigationGuard::OnProtocolEvent );
    SubscribeTo( m_client, CChromeDevToolsClient::ProtocolEvent, callback );
    TEventCallback disconnected( this, &CNavigationGuard::OnClientDisconnected );
    SubscribeTo( m_client, CChromeDevToolsClient::DisconnectedEvent, disconnected );

    // the tabs, whose ids are those of their main frames
    TChromeTargetVector targets;
    if ( m_client->TargetGetTargets( targets ) )
        for ( size_t i=0; i<targets.size(); ++i )
            if ( targets[ i ].type == "page" )
                m_pageTargets.insert( targets[ i ].id );

    CORE::CDataNode result;
    CORE::CDataNode discover( GUCEF_DATATYPE_OBJECT );
    discover.SetAttribute( "discover", true );
    bool ok = m_client->SendCommandOnSessionAndWait( CORE::CString::Empty, "Target.setDiscoverTargets", &discover, result );

    // every document request of every tab and frame waits for our answer
    CORE::CDataNode fetch( GUCEF_DATATYPE_OBJECT );
    CORE::CDataNode* patterns = fetch.AddChild( "patterns", GUCEF_DATATYPE_ARRAY );
    CORE::CDataNode* pattern = patterns->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT );
    pattern->SetAttribute( "urlPattern", CORE::CVariant( CORE::CString( "*" ) ) );
    pattern->SetAttribute( "resourceType", CORE::CVariant( CORE::CString( "Document" ) ) );
    pattern->SetAttribute( "requestStage", CORE::CVariant( CORE::CString( "Request" ) ) );
    ok = m_client->SendCommandOnSessionAndWait( CORE::CString::Empty, "Fetch.enable", &fetch, result ) && ok;
    if ( !ok )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "NavigationGuard: The browser would not let the guard intercept navigations" );
        Stop();
        return false;
    }
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "NavigationGuard: Guarding navigations with " << m_allow.size() << " allow and " << m_deny.size() << " deny rules" );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CNavigationGuard::Stop( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_client )
        return;
    UnsubscribeFrom( m_client );
    if ( m_client->IsConnected() )
    {
        UInt64 commandId = 0;
        m_client->SendCommandOnSession( CORE::CString::Empty, "Fetch.disable", GUCEF_NULL, commandId );
    }
    m_client = GUCEF_NULL;
    m_pageTargets.clear();
    m_reportedCommits.clear();
}

/*-------------------------------------------------------------------------*/

bool
CNavigationGuard::IsStarted( void ) const
{GUCEF_TRACE;

    return GUCEF_NULL != m_client;
}

/*-------------------------------------------------------------------------*/

void
CNavigationGuard::OnClientDisconnected( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_client )
    {
        UnsubscribeFrom( m_client );
        m_client = GUCEF_NULL;
        m_pageTargets.clear();
        m_reportedCommits.clear();
    }
}

/*-------------------------------------------------------------------------*/

void
CNavigationGuard::OnProtocolEvent( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( GUCEF_NULL == eventData || GUCEF_NULL == m_client )
        return;
    const SChromeProtocolEvent& event = static_cast< TChromeProtocolEventData* >( eventData )->GetData();
    if ( !event.hasParams || !event.sessionId.IsNULLOrEmpty() )
        return;     // ours are the events of the browser target
    if ( event.method == "Fetch.requestPaused" )
        HandleRequestPaused( event );
    else
    if ( event.method == "Target.targetCreated" || event.method == "Target.targetInfoChanged" || event.method == "Target.targetDestroyed" )
        HandleTargetInfo( event );
}

/*-------------------------------------------------------------------------*/

void
CNavigationGuard::HandleRequestPaused( const SChromeProtocolEvent& event )
{GUCEF_TRACE;

    const CORE::CString requestId = event.params.GetAttributeValue( "requestId" ).AsString( CORE::CString::Empty, false );
    if ( requestId.IsNULLOrEmpty() )
        return;
    const CORE::CDataNode* request = event.params.FindChild( "request" );
    const CORE::CString url = GUCEF_NULL != request ? request->GetAttributeValue( "url" ).AsString( CORE::CString::Empty, false ) : CORE::CString::Empty;
    const CORE::CString frameId = event.params.GetAttributeValue( "frameId" ).AsString( CORE::CString::Empty, false );
    const bool redirected = !event.params.GetAttributeValue( "redirectedRequestId" ).AsString( CORE::CString::Empty, false ).IsNULLOrEmpty();
    const bool isMainFrame = m_pageTargets.find( frameId ) != m_pageTargets.end();

    // the request waits in the browser: it is answered right away, also when the answer cannot be sent the browser has to hear of it
    CORE::CString reason;
    const bool allowed = IsAllowed( url, isMainFrame, &reason );
    CORE::CDataNode params( GUCEF_DATATYPE_OBJECT );
    params.SetAttribute( "requestId", CORE::CVariant( requestId ) );
    UInt64 commandId = 0;
    if ( allowed )
    {
        ++m_allowedCount;
        m_client->SendCommandOnSession( CORE::CString::Empty, "Fetch.continueRequest", &params, commandId );
        return;
    }
    // aborted, not blocked: the page stays where it was instead of showing an error page
    params.SetAttribute( "errorReason", CORE::CVariant( CORE::CString( "Aborted" ) ) );
    m_client->SendCommandOnSession( CORE::CString::Empty, "Fetch.failRequest", &params, commandId );
    Report( url, frameId, isMainFrame, reason, redirected ? CNavigationRefusal::SourceRedirect : CNavigationRefusal::SourcePage, true );
}

/*-------------------------------------------------------------------------*/

void
CNavigationGuard::HandleTargetInfo( const SChromeProtocolEvent& event )
{GUCEF_TRACE;

    if ( event.method == "Target.targetDestroyed" )
    {
        const CORE::CString targetId = event.params.GetAttributeValue( "targetId" ).AsString( CORE::CString::Empty, false );
        m_pageTargets.erase( targetId );
        m_reportedCommits.erase( targetId );
        return;
    }
    const CORE::CDataNode* info = event.params.FindChild( "targetInfo" );
    if ( GUCEF_NULL == info || info->GetAttributeValue( "type" ).AsString( CORE::CString::Empty, false ) != "page" )
        return;
    const CORE::CString targetId = info->GetAttributeValue( "targetId" ).AsString( CORE::CString::Empty, false );
    const CORE::CString url = info->GetAttributeValue( "url" ).AsString( CORE::CString::Empty, false );
    m_pageTargets.insert( targetId );
    if ( event.method != "Target.targetInfoChanged" || url.IsNULLOrEmpty() || SchemeOf( url ) == "chrome-error" )
        return;

    // an address the tab shows without a request having passed: a navigation within the document
    CORE::CString reason;
    if ( IsAllowed( url, true, &reason ) )
    {
        m_reportedCommits.erase( targetId );
        return;
    }
    const CORE::CString urlTemplate = MakeUrlTemplate( url );
    TStringMap::iterator reported = m_reportedCommits.find( targetId );
    if ( reported != m_reportedCommits.end() && reported->second == urlTemplate )
        return;
    m_reportedCommits[ targetId ] = urlTemplate;
    Report( url, targetId, true, reason, CNavigationRefusal::SourceCommitted, false );
}

/*-------------------------------------------------------------------------*/

UInt32
CNavigationGuard::GetRefusedCount( void ) const
{GUCEF_TRACE;

    return m_refusedCount;
}

/*-------------------------------------------------------------------------*/

UInt32
CNavigationGuard::GetAllowedCount( void ) const
{GUCEF_TRACE;

    return m_allowedCount;
}

/*-------------------------------------------------------------------------*/

const TNavigationRefusalVector&
CNavigationGuard::GetRecentRefusals( void ) const
{GUCEF_TRACE;

    return m_recent;
}

/*-------------------------------------------------------------------------*/

void
CNavigationGuard::ClearRecentRefusals( void )
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
