/*
 *  AncestryDotComBackupLib: logic of the Ancestry.com tree backup tool
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

#ifndef ANCESTRYBACKUP_CENDPOINTCATALOG_H
#include "AncestryBackup_CEndpointCatalog.h"
#define ANCESTRYBACKUP_CENDPOINTCATALOG_H
#endif /* ANCESTRYBACKUP_CENDPOINTCATALOG_H ? */

#ifndef ANCESTRYBACKUP_CURLTEMPLATE_H
#include "AncestryBackup_CUrlTemplate.h"
#define ANCESTRYBACKUP_CURLTEMPLATE_H
#endif /* ANCESTRYBACKUP_CURLTEMPLATE_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CDATETIME_H
#include "gucefCORE_CDateTime.h"
#define GUCEF_CORE_CDATETIME_H
#endif /* GUCEF_CORE_CDATETIME_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_CFILEACCESS_H
#include "CFileAccess.h"
#define GUCEF_CORE_CFILEACCESS_H
#endif /* GUCEF_CORE_CFILEACCESS_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CREDACTOR_H
#include "gucefCORE_CRedactor.h"
#define GUCEF_CORE_CREDACTOR_H
#endif /* GUCEF_CORE_CREDACTOR_H ? */

#include <vector>

#include "AncestryBackup_CBrowserSession.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ANCESTRYBACKUP {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

static UInt64
NowInMs( void )
{
    return CORE::CDateTime::NowUTCDateTime().ToUnixEpochBasedTicksInMillisecs();
}

/*-------------------------------------------------------------------------*/

CBrowserSession::CBrowserSession( const CSettings& settings, const CDataLayout& layout, const std::string& siteTreeId )
    : CORE::CObserver()
    , m_settings( settings )
    , m_layout( layout )
    , m_policy()
    , m_budget()
    , m_browser()
    , m_client()
    , m_sessionId()
    , m_frameId()
    , m_contextHeader()
    , m_csrfToken()
    , m_preferredTabPart()
    , m_needsPostToken( false )
    , m_open( false )
{GUCEF_TRACE;

    if ( !m_policy.SetTreeId( siteTreeId ) )
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_CRITICAL, "AncestryBackup: \"" + CORE::CString( siteTreeId.c_str() ) + "\" is not the number Ancestry uses for a tree, nothing about a tree will be requested" );
    m_policy.AddDefaultAllowed();
    m_policy.LoadAllowedFile( layout.GetEndpointsConfigPath() );
    m_budget.Configure( settings.requestDelayInMs, settings.requestJitterPercent, settings.maxRequestsPerRun, settings.maxRequestsPerDay );
    m_budget.SetStateFile( CORE::CombinePath( layout.GetStateDir(), "request-budget.txt" ), NowInMs() );
}

/*-------------------------------------------------------------------------*/

CBrowserSession::~CBrowserSession()
{GUCEF_TRACE;

    Close();
}

/*-------------------------------------------------------------------------*/

CRequestPolicy&
CBrowserSession::GetPolicy( void )
{GUCEF_TRACE;

    return m_policy;
}

/*-------------------------------------------------------------------------*/

CRequestBudget&
CBrowserSession::GetBudget( void )
{GUCEF_TRACE;

    return m_budget;
}

/*-------------------------------------------------------------------------*/

bool
CBrowserSession::IsOpen( void ) const
{GUCEF_TRACE;

    return m_open;
}

/*-------------------------------------------------------------------------*/

bool
CBrowserSession::IsAttached( void ) const
{GUCEF_TRACE;

    return m_browser.IsAttached();
}

/*-------------------------------------------------------------------------*/

void
CBrowserSession::SetPreferredTabUrlPart( const std::string& part )
{GUCEF_TRACE;

    m_preferredTabPart = part;
}

/*-------------------------------------------------------------------------*/

void
CBrowserSession::SetNeedsPostToken( bool needed )
{GUCEF_TRACE;

    m_needsPostToken = needed;
}

/*-------------------------------------------------------------------------*/

const std::string&
CBrowserSession::GetContextHeader( void ) const
{GUCEF_TRACE;

    return m_contextHeader;
}

/*-------------------------------------------------------------------------*/

void
CBrowserSession::OnNotify( CORE::CNotifier* notifier, const CORE::CEvent& eventid, CORE::CICloneable* eventdata )
{GUCEF_TRACE;

    if ( eventid != WEB::CChromeDevToolsClient::ProtocolEvent || GUCEF_NULL == eventdata )
        return;
    const WEB::SChromeProtocolEvent& event = static_cast< WEB::TChromeProtocolEventData* >( eventdata )->GetData();
    if ( event.method != "Network.requestWillBeSent" || !event.hasParams || event.sessionId != m_sessionId )
        return;

    // what the site's own scripts send along with their requests to its servers
    const CORE::CDataNode* request = event.params.Find( "request" );
    if ( GUCEF_NULL == request )
        return;
    const CORE::CString requestUrl = request->GetAttributeValue( "url" ).AsString();
    if ( requestUrl.HasSubstr( "ancestry.com/" ) < 0 )
        return;
    const CORE::CDataNode* headers = request->Find( "headers" );
    if ( GUCEF_NULL == headers )
        return;
    for ( UInt32 i=0; i<headers->GetAttCount(); ++i )
    {
        const CORE::CDataNode::TKeyValuePair* att = headers->GetAttribute( i );
        if ( GUCEF_NULL != att )
        {
            std::string name = att->first.STL_String();
            for ( size_t c=0; c<name.size(); ++c )
                name[ c ] = (char) tolower( name[ c ] );
            if ( name == "ancestry-context-ube" )
                m_contextHeader = att->second.AsString().STL_String();
            else
            if ( name == "x-csrf-token" && m_csrfToken.empty() )
                m_csrfToken = att->second.AsString().STL_String();
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CBrowserSession::Pause( UInt32 delayInMs )
{GUCEF_TRACE;

    const UInt64 start = MT::PrecisionTickCount();
    const Float64 ticksPerMs = MT::PrecisionTimerResolution() / 1000.0;
    do
    {
        CORE::CCoreGlobal::Instance()->GetPulseGenerator()->ForceDirectPulse();
        MT::ThreadDelay( delayInMs > 20 ? 10 : 1 );
    }
    while ( ( MT::PrecisionTickCount() - start ) / ticksPerMs < delayInMs );
}

/*-------------------------------------------------------------------------*/

bool
CBrowserSession::Open( void )
{GUCEF_TRACE;

    std::string haltReason;
    if ( CHaltState::IsHalted( m_layout.GetStateDir(), haltReason ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_CRITICAL, "AncestryBackup: The tool was stopped earlier because " + CORE::CString( haltReason.c_str() ) + ". Look at " + CHaltState::GetPath( m_layout.GetStateDir() ) +
                                                  " and delete that file when you are sure it is fine to go on" );
        return false;
    }

    m_browser.SetExecutablePath( m_settings.chromePath );
    m_browser.SetDebugPort( m_settings.debugPort );
    m_browser.SetProfileDir( m_layout.GetBrowserProfileDir() );
    m_browser.SetHeadless( m_settings.headless );
    m_browser.SetAttachToRunning( m_settings.attachToRunning );
    std::vector< CORE::CString > dataDirs;
    for ( size_t i=0; i<m_settings.chromeUserDataDirs.size(); ++i )
        dataDirs.push_back( m_settings.chromeUserDataDirs[ i ].c_str() );
    m_browser.SetUserDataDirs( dataDirs );
    for ( size_t i=0; i<m_settings.chromeArgs.size(); ++i )
        m_browser.AddArgument( m_settings.chromeArgs[ i ].c_str() );

    if ( !m_browser.EnsureRunning( m_settings.startUrl ) || !m_browser.ConnectToBrowser( m_client ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_CRITICAL, "AncestryBackup: Unable to get a browser to work with" );
        return false;
    }

    // a tab of the site: the requests are made from inside it, so that they are the site's own
    CORE::CDataNode targets;
    CORE::CString targetId;
    bool preferred = false;
    if ( m_client.SendCommandAndWait( "Target.getTargets", GUCEF_NULL, targets ) )
    {
        const CORE::CDataNode* infos = targets.Find( "targetInfos" );
        if ( GUCEF_NULL != infos )
            for ( CORE::CDataNode::const_iterator i = infos->ConstBegin(); i != infos->ConstEnd(); ++i )
            {
                if ( (*i)->GetAttributeValue( "type" ).AsString() != "page" )
                    continue;
                const CORE::CString tabUrl = (*i)->GetAttributeValue( "url" ).AsString();
                if ( !m_preferredTabPart.empty() && tabUrl.HasSubstr( m_preferredTabPart.c_str() ) >= 0 )
                {
                    targetId = (*i)->GetAttributeValue( "targetId" ).AsString();
                    preferred = true;
                }
                else
                if ( !preferred && tabUrl.HasSubstr( m_settings.cookieDomain ) >= 0 )
                    targetId = (*i)->GetAttributeValue( "targetId" ).AsString();
            }
    }
    if ( targetId.IsNULLOrEmpty() && !m_client.TargetCreate( m_settings.startUrl, targetId ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_CRITICAL, "AncestryBackup: The browser has no tab of the site to work in" );
        return false;
    }
    if ( !m_client.AttachToTarget( targetId, m_sessionId ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_CRITICAL, "AncestryBackup: Unable to attach to the tab of the site" );
        return false;
    }
    m_client.SetSessionId( m_sessionId );

    // wait for the page to be loaded, a tab that is not on the site yet cannot make requests to it
    bool loaded = false;
    for ( UInt32 i=0; i<300 && !loaded; ++i )
    {
        CORE::CDataNode state;
        if ( m_client.RuntimeEvaluate( "document.readyState + ' ' + location.host", false, state ) )
        {
            const std::string text = state.GetValue().AsString().STL_String();
            loaded = text.compare( 0, 8, "complete" ) == 0 && text.find( m_settings.cookieDomain.STL_String() ) != std::string::npos;
        }
        if ( !loaded )
            Pause( 100 );
    }
    if ( !loaded )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_CRITICAL, "AncestryBackup: The tab of the site did not finish loading" );
        return false;
    }

    CORE::CDataNode frames;
    if ( m_client.SendCommandAndWait( "Page.getFrameTree", GUCEF_NULL, frames ) )
    {
        const CORE::CDataNode* tree = frames.Find( "frameTree" );
        const CORE::CDataNode* frame = GUCEF_NULL != tree ? tree->Find( "frame" ) : GUCEF_NULL;
        if ( GUCEF_NULL != frame )
            m_frameId = frame->GetAttributeValue( "id" ).AsString();
    }
    // let the tab show what it sends. A tab that has been quiet is loaded again once, like a person pressing F5
    SubscribeTo( &m_client );
    m_client.NetworkEnable();
    for ( UInt32 i=0; i<30 && m_contextHeader.empty(); ++i )
        Pause( 100 );
    if ( m_contextHeader.empty() )
    {
        CORE::CDataNode ignored;
        m_client.SendCommandAndWait( "Page.reload", GUCEF_NULL, ignored );
        for ( UInt32 i=0; i<300 && m_contextHeader.empty(); ++i )
            Pause( 100 );
    }
    if ( m_contextHeader.empty() )
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "AncestryBackup: The tab did not show the headers the site sends, requests may be refused" );

    // The token the site's own POST requests carry is added by the scripts of its pages. It is seen in the traffic of the tab: the page is
    // loaded again once if it has not said anything, which makes it do its requests.
    if ( m_needsPostToken && m_csrfToken.empty() )
    {
        CORE::CDataNode ignored;
        m_client.SendCommandAndWait( "Page.reload", GUCEF_NULL, ignored );
        for ( UInt32 i=0; i<300 && m_csrfToken.empty(); ++i )
            Pause( 100 );
        if ( !m_csrfToken.empty() )
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AncestryBackup: Saw the token for POST requests in the traffic of the page" );
    }

    // or found in the page: where the site keeps it for its scripts
    if ( m_csrfToken.empty() )
    {
        CORE::CDataNode found;
        const char* const probe =
            "(function() { var out = {};"
            " document.cookie.split(';').forEach(function(c) { var p = c.split('='); var n = (p[0] || '').trim(); if (/csrf|xsrf/i.test(n)) out['cookie:' + n] = p.slice(1).join('='); });"
            " document.querySelectorAll('meta').forEach(function(m) { var n = m.getAttribute('name') || ''; if (/csrf|xsrf/i.test(n)) out['meta:' + n] = m.getAttribute('content'); });"
            " Object.keys(window).forEach(function(k) { if (/csrf|xsrf/i.test(k)) { try { var v = window[k]; if (typeof v === 'string') out['window:' + k] = v; } catch (e) {} } });"
            " return out; })()";
        if ( m_client.RuntimeEvaluate( probe, false, found ) )
        {
            for ( UInt32 i=0; i<found.GetAttCount() && m_csrfToken.empty(); ++i )
            {
                const CORE::CDataNode::TKeyValuePair* att = found.GetAttribute( i );
                if ( GUCEF_NULL != att && !att->second.AsString().IsNULLOrEmpty() )
                {
                    m_csrfToken = att->second.AsString().STL_String();
                    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AncestryBackup: Found the token for POST requests in the page (" + att->first + ")" );
                }
            }
        }
    }

    m_open = true;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CBrowserSession::Close( void )
{GUCEF_TRACE;

    if ( m_open )
    {
        m_client.SetSessionId( CORE::CString::Empty );
        m_client.Disconnect();
    }
    m_open = false;
}

/*-------------------------------------------------------------------------*/

bool
CBrowserSession::Admit( const std::string& method, const std::string& url, const std::string& body, SFetchResult& result )
{GUCEF_TRACE;

    result = SFetchResult();

    std::string haltReason;
    if ( CHaltState::IsHalted( m_layout.GetStateDir(), haltReason ) )
    {
        result.error = "the tool was stopped earlier because " + haltReason;
        result.halted = true;
        return false;
    }
    if ( !m_open )
    {
        result.error = "there is no browser session";
        return false;
    }

    const SRequestVerdict verdict = m_policy.Check( method, url, body );
    if ( !verdict.allowed )
    {
        result.error = "refused: " + verdict.reason;
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "AncestryBackup: Refused to request " << CORE::CRedactor::ForUrl().RedactUrl( CORE::CString( url.c_str() ) ) << ": " << CORE::CString( verdict.reason.c_str() ) );
        return false;
    }

    std::string why;
    if ( !m_budget.CanRequest( NowInMs(), why ) )
    {
        result.error = "no more requests: " + why;
        return false;
    }

    // the pause between requests, which differs every time
    const UInt32 pause = m_budget.GetDelayBeforeNextInMs( NowInMs() );
    if ( pause > 0 )
        Pause( pause );
    m_budget.RecordRequest( NowInMs() );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CBrowserSession::Judge( const std::string& url, SFetchResult& result, const std::string& mitigation, const std::string& bodyStart, bool expectJson )
{GUCEF_TRACE;

    std::string reason;
    if ( CSiteResponseCheck::IsTrouble( result.status, result.contentType, mitigation, bodyStart, expectJson, reason ) )
    {
        result.ok = false;
        result.halted = true;
        result.error = reason;
        CHaltState::Halt( m_layout.GetStateDir(), reason + " (while requesting " + CUrlTemplate::PercentDecode( CORE::CString( url.c_str() ) ).STL_String() + ")" );
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_CRITICAL, "AncestryBackup: STOPPING. " + CORE::CString( reason.c_str() ) + ". Nothing more will be requested until you have looked at " + CHaltState::GetPath( m_layout.GetStateDir() ) +
                                                  " and deleted it" );
        return;
    }
    result.ok = result.status >= 200 && result.status < 300;
    if ( !result.ok )
        result.error = "the site answered " + std::to_string( (long long) result.status );
}

/*-------------------------------------------------------------------------*/

bool
CBrowserSession::GetText( const std::string& url, bool expectJson, SFetchResult& result )
{GUCEF_TRACE;

    if ( !Admit( "GET", url, std::string(), result ) )
        return false;

    const std::string contextHeaderPart = m_contextHeader.empty() ? std::string() : ", 'ancestry-context-ube': \"" + CUrlTemplate::EscapeJson( CORE::CString( m_contextHeader.c_str() ) ).STL_String() + "\"";
    const std::string script =
        "(async function() { try { const r = await fetch(" + std::string( "\"" ) + CUrlTemplate::EscapeJson( CORE::CString( url.c_str() ) ).STL_String() + "\"" +
        ", { method: 'GET', credentials: 'include', redirect: 'follow', headers: { 'Accept': 'application/json, text/plain, */*'" + contextHeaderPart + " } });"
        " const t = await r.text();"
        " return { status: r.status, ct: r.headers.get('content-type') || '', cm: r.headers.get('cf-mitigated') || '', body: t }; }"
        " catch (e) { return { status: 0, ct: '', cm: '', body: '', error: String(e) }; } })()";

    CORE::CDataNode value;
    CORE::CString exception;
    if ( !m_client.RuntimeEvaluate( script.c_str(), true, value, &exception ) )
    {
        result.error = "the browser could not make the request: " + exception.STL_String();
        return false;
    }

    result.status = (Int32) value.GetAttributeValue( "status" ).AsInt64( 0 );
    result.contentType = value.GetAttributeValue( "ct" ).AsString().STL_String();
    result.body = value.GetAttributeValue( "body" ).AsString().STL_String();
    const std::string mitigation = value.GetAttributeValue( "cm" ).AsString().STL_String();
    if ( result.status == 0 )
    {
        result.error = "no answer: " + value.GetAttributeValue( "error" ).AsString().STL_String();
        return false;
    }
    Judge( url, result, mitigation, result.body, expectJson );
    if ( !result.ok && !result.halted && !result.body.empty() )
        result.error += ": " + result.body.substr( 0, 300 );       // what the site says about it, an error page is short
    return result.ok;
}

/*-------------------------------------------------------------------------*/

bool
CBrowserSession::PostJson( const std::string& url, const std::string& body, SFetchResult& result )
{GUCEF_TRACE;

    if ( !Admit( "POST", url, body, result ) )
        return false;

    const std::string contextHeaderPart = m_contextHeader.empty() ? std::string() : ", 'ancestry-context-ube': \"" + CUrlTemplate::EscapeJson( CORE::CString( m_contextHeader.c_str() ) ).STL_String() + "\"";
    const std::string csrfHeaderPart = m_csrfToken.empty() ? std::string() : ", 'x-csrf-token': \"" + CUrlTemplate::EscapeJson( CORE::CString( m_csrfToken.c_str() ) ).STL_String() + "\"";
    const std::string script =
        "(async function() { try { const r = await fetch(\"" + CUrlTemplate::EscapeJson( CORE::CString( url.c_str() ) ).STL_String() + "\"" +
        ", { method: 'POST', credentials: 'include', redirect: 'manual', headers: { 'Accept': 'application/json, text/plain, */*', 'Content-Type': 'application/json'" + contextHeaderPart + csrfHeaderPart + " }, body: \"" +
        CUrlTemplate::EscapeJson( CORE::CString( body.c_str() ) ).STL_String() + "\" });"
        " const t = await r.text();"
        " return { status: r.status, ct: r.headers.get('content-type') || '', cm: r.headers.get('cf-mitigated') || '', body: t }; }"
        " catch (e) { return { status: 0, ct: '', cm: '', body: '', error: String(e) }; } })()";

    CORE::CDataNode value;
    CORE::CString exception;
    if ( !m_client.RuntimeEvaluate( script.c_str(), true, value, &exception ) )
    {
        result.error = "the browser could not make the request: " + exception.STL_String();
        return false;
    }
    result.status = (Int32) value.GetAttributeValue( "status" ).AsInt64( 0 );
    result.contentType = value.GetAttributeValue( "ct" ).AsString().STL_String();
    result.body = value.GetAttributeValue( "body" ).AsString().STL_String();
    if ( result.status == 0 )
    {
        result.error = "no answer: " + value.GetAttributeValue( "error" ).AsString().STL_String();
        return false;
    }
    Judge( url, result, value.GetAttributeValue( "cm" ).AsString().STL_String(), result.body, true );
    if ( !result.ok && !result.halted && !result.body.empty() )
        result.error += ": " + result.body.substr( 0, 300 );
    return result.ok;
}

/*-------------------------------------------------------------------------*/

bool
CBrowserSession::GetToFile( const std::string& url, const CORE::CString& path, SFetchResult& result )
{GUCEF_TRACE;

    if ( !Admit( "GET", url, std::string(), result ) )
        return false;
    if ( m_frameId.IsNULLOrEmpty() )
    {
        result.error = "the tab has no frame to load from";
        return false;
    }

    // the browser loads the resource itself, with its own cookies, and hands it over in pieces
    CORE::CDataNode params( GUCEF_DATATYPE_OBJECT );
    params.SetAttribute( "frameId", m_frameId );
    params.SetAttribute( "url", CORE::CString( url.c_str() ) );
    CORE::CDataNode* options = params.AddChild( "options", GUCEF_DATATYPE_OBJECT );
    options->SetAttribute( "disableCache", true );
    options->SetAttribute( "includeCredentials", true );

    CORE::CDataNode loaded;
    if ( !m_client.SendCommandAndWait( "Network.loadNetworkResource", &params, loaded, GUCEF_NULL, 120000 ) )
    {
        result.error = "the browser could not load the resource";
        return false;
    }
    const CORE::CDataNode* resource = loaded.Find( "resource" );
    if ( GUCEF_NULL == resource )
    {
        result.error = "the browser gave no resource";
        return false;
    }
    result.status = (Int32) resource->GetAttributeValue( "httpStatusCode" ).AsInt64( 0 );
    std::string mitigation;
    const CORE::CDataNode* headers = resource->Find( "headers" );
    if ( GUCEF_NULL != headers )
    {
        result.contentType = headers->GetAttributeValue( "content-type" ).AsString().STL_String();
        if ( result.contentType.empty() )
            result.contentType = headers->GetAttributeValue( "Content-Type" ).AsString().STL_String();
        mitigation = headers->GetAttributeValue( "cf-mitigated" ).AsString().STL_String();
    }
    const CORE::CString handle = resource->GetAttributeValue( "stream" ).AsString();

    Judge( url, result, mitigation, std::string(), false );
    if ( !result.ok || handle.IsNULLOrEmpty() )
    {
        if ( !handle.IsNULLOrEmpty() )
        {
            CORE::CDataNode closeParams( GUCEF_DATATYPE_OBJECT ), ignored;
            closeParams.SetAttribute( "handle", handle );
            m_client.SendCommandAndWait( "IO.close", &closeParams, ignored );
        }
        if ( result.error.empty() )
            result.error = "the browser could not load the resource, it gave no data";
        return false;
    }

    // to a file of its own first, in place only when everything has arrived
    const CORE::CString temp = path + ".part";
    CORE::CreateDirs( CORE::StripFilename( path ) );
    bool ok = true;
    {
        CORE::CFileAccess out( temp, "wb" );
        ok = out.Opened();
        bool eof = false;
        while ( ok && !eof )
        {
            CORE::CDataNode readParams( GUCEF_DATATYPE_OBJECT ), piece;
            readParams.SetAttribute( "handle", handle );
            readParams.SetAttribute( "size", (UInt32) ( 1024 * 1024 ) );
            if ( !m_client.SendCommandAndWait( "IO.read", &readParams, piece, GUCEF_NULL, 120000 ) )
            {
                result.error = "reading the data from the browser failed";
                ok = false;
                break;
            }
            const CORE::CString data = piece.GetAttributeValue( "data" ).AsString();
            eof = piece.GetAttributeValue( "eof" ).AsBool( false );
            if ( piece.GetAttributeValue( "base64Encoded" ).AsBool( false ) )
            {
                std::vector< char > bytes( data.Length() + 16 );
                UInt32 used = 0;
                if ( !data.IsNULLOrEmpty() )
                {
                    if ( !CORE::Base64Decode( data, &bytes[ 0 ], (UInt32) bytes.size(), used ) )
                    {
                        result.error = "the data from the browser is not valid";
                        ok = false;
                        break;
                    }
                    ok = out.Write( &bytes[ 0 ], 1, used ) == used;
                    result.bytes += used;
                }
            }
            else
            if ( !data.IsNULLOrEmpty() )
            {
                const std::string text = data.STL_String();
                ok = out.Write( text.data(), 1, (UInt32) text.size() ) == text.size();
                result.bytes += text.size();
            }
        }
        out.Flush();
        out.Close();
    }
    CORE::CDataNode closeParams( GUCEF_DATATYPE_OBJECT ), ignored;
    closeParams.SetAttribute( "handle", handle );
    m_client.SendCommandAndWait( "IO.close", &closeParams, ignored );

    ok = ok && CORE::MoveFile( temp, path, true );
    if ( !ok )
    {
        CORE::DeleteFile( temp );
        result.ok = false;
        if ( result.error.empty() )
            result.error = "the data could not be written to " + path.STL_String();
    }
    return result.ok;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ANCESTRYBACKUP */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
