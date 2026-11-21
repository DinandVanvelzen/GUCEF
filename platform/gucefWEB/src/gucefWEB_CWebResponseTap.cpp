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

#ifndef GUCEF_CORE_CGLOBPATTERN_H
#include "gucefCORE_CGlobPattern.h"
#define GUCEF_CORE_CGLOBPATTERN_H
#endif /* GUCEF_CORE_CGLOBPATTERN_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_WEB_CNAVIGATIONRULE_H
#include "gucefWEB_CNavigationRule.h"
#define GUCEF_WEB_CNAVIGATIONRULE_H
#endif /* GUCEF_WEB_CNAVIGATIONRULE_H ? */

#include "gucefWEB_CWebResponseTap.h"

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

const CORE::CEvent CWebResponseTap::ResponseDeliveredEvent = "GUCEF::WEB::CWebResponseTap::ResponseDeliveredEvent";
const CORE::CEvent CWebResponseTap::WaitEndedEvent = "GUCEF::WEB::CWebResponseTap::WaitEndedEvent";
const UInt32 CWebResponseTap::MaxBodyBytesCeiling = 64 * 1024 * 1024;

/** Requests in flight that are kept; a page that never finishes its requests cannot make this grow */
static const size_t MaxInFlight = 1024;

/*-------------------------------------------------------------------------*/

static CORE::CString
GetText( const CORE::CDataNode& node, const char* name )
{
    return node.GetAttributeValue( name ).AsString( CORE::CString::Empty, false );
}

/*-------------------------------------------------------------------------*/

static CORE::CString
MakeKey( const CORE::CString& sessionId, const CORE::CString& requestId )
{
    return sessionId + "|" + requestId;
}

/*-------------------------------------------------------------------------*/

CWebResponseTap::STap::STap( void )
    : id( 0 )
    , method()
    , hostPattern()
    , pathPattern()
    , sink( GUCEF_NULL )
    , maxBodyBytes( 0 )
    , delivered( 0 )
    , tooLarge( 0 )
    , deliveredSeen( 0 )
    , tooLargeSeen( 0 )
    , waitState( WAIT_NONE )
    , waitDeadline( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CWebResponseTap::SPending::SPending( void )
    : sessionId()
    , requestId()
    , method()
    , host()
    , pathTemplate()
    , status( 0 )
    , mimeType()
    , sequence( 0 )
    , encodedDataLength( 0 )
    , tapIds()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CWebResponseTap::RegisterEvents( void )
{GUCEF_TRACE;

    ResponseDeliveredEvent.Initialize();
    WaitEndedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CWebResponseTap::CWebResponseTap( CChromeDevToolsClient& browserClient )
    : CORE::CObservingNotifier()
    , m_client( browserClient )
    , m_neverRecord()
    , m_pageHostFilter()
    , m_taps()
    , m_nextTapId( 1 )
    , m_nextSequence( 1 )
    , m_started( false )
    , m_inFlight()
    , m_ready()
    , m_targetsToAttach()
    , m_attachedTargets()
    , m_sessions()
    , m_sessionsToSetup()
    , m_sessionTargets()
    , m_body()
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CWebResponseTap::~CWebResponseTap()
{GUCEF_TRACE;

    Stop();
    if ( m_body.GetBufferSize() > 0 )
        memset( m_body.GetBufferPtr(), 0, m_body.GetBufferSize() );
}

/*-------------------------------------------------------------------------*/

UInt64
CWebResponseTap::NowInMs( void ) const
{GUCEF_TRACE;

    return (UInt64) ( MT::PrecisionTickCount() / ( MT::PrecisionTimerResolution() / 1000.0 ) );
}

/*-------------------------------------------------------------------------*/

bool
CWebResponseTap::IsMatch( const CORE::CString& methodPattern, const CORE::CString& hostPattern, const CORE::CString& pathTemplatePattern,
                          const CORE::CString& method, const CUrlPathTemplate& parsed )
{GUCEF_TRACE;

    if ( !methodPattern.IsNULLOrEmpty() && methodPattern != "*" && methodPattern.Uppercase() != method.Uppercase() )
        return false;

    // the host name, or host:port when the pattern names a port
    const CORE::CString hostAndPort = parsed.host.Lowercase();
    CORE::CString hostName = hostAndPort;
    if ( hostPattern.HasChar( ':' ) < 0 )
    {
        const Int32 colon = hostAndPort.HasChar( ':', false );
        const Int32 bracket = hostAndPort.HasChar( ']', false );
        if ( colon >= 0 && colon > bracket )
            hostName = hostAndPort.SubstrToIndex( (UInt32) colon, true );
    }
    if ( !CORE::CGlobPattern::IsMatch( hostPattern, hostName, false ) )
        return false;
    return CORE::CGlobPattern::IsMatch( CNavigationRule::EscapeBraces( pathTemplatePattern ), parsed.pathTemplate, true );
}

/*-------------------------------------------------------------------------*/

UInt32
CWebResponseTap::AddTap( const CORE::CString& method              ,
                         const CORE::CString& hostPattern         ,
                         const CORE::CString& pathTemplatePattern ,
                         CIWebResponseSink* sink                  ,
                         UInt32 maxBodyBytes                      )
{GUCEF_TRACE;

    if ( GUCEF_NULL == sink || hostPattern.IsNULLOrEmpty() || pathTemplatePattern.IsNULLOrEmpty() || 0 == maxBodyBytes ||
         maxBodyBytes > MaxBodyBytesCeiling || !CORE::CGlobPattern( hostPattern ).IsValid() || !CORE::CGlobPattern( CNavigationRule::EscapeBraces( pathTemplatePattern ), true ).IsValid() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "WebResponseTap: A tap needs a sink, a host pattern, a path template pattern and a size limit of 1 byte to 64 MiB" );
        return 0;
    }
    STap tap;
    tap.id = m_nextTapId++;
    tap.method = method.Uppercase();
    tap.hostPattern = hostPattern.Lowercase();
    tap.pathPattern = pathTemplatePattern;
    tap.sink = sink;
    tap.maxBodyBytes = maxBodyBytes;
    m_taps[ tap.id ] = tap;
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "WebResponseTap: Tap " << tap.id << " on " << ( tap.method.IsNULLOrEmpty() ? CORE::CString( "*" ) : tap.method ) <<
               " " << tap.hostPattern << tap.pathPattern );
    return tap.id;
}

/*-------------------------------------------------------------------------*/

bool
CWebResponseTap::RemoveTap( UInt32 tapId )
{GUCEF_TRACE;

    return m_taps.erase( tapId ) > 0;
}

/*-------------------------------------------------------------------------*/

UInt32
CWebResponseTap::GetTapCount( void ) const
{GUCEF_TRACE;

    return (UInt32) m_taps.size();
}

/*-------------------------------------------------------------------------*/

void
CWebResponseTap::SetPageHostFilter( const CORE::CString::StringVector& terms )
{GUCEF_TRACE;

    m_pageHostFilter = terms;
}

/*-------------------------------------------------------------------------*/

void
CWebResponseTap::SetNeverRecordRules( const CSiteDiscoverySettings& settings )
{GUCEF_TRACE;

    CSiteDiscoverySettings rules;
    rules.neverRecordHosts = settings.neverRecordHosts;
    rules.neverRecordPathPatterns = settings.neverRecordPathPatterns;
    m_neverRecord.SetSettings( rules );
}

/*-------------------------------------------------------------------------*/

bool
CWebResponseTap::PagePassesFilter( const CORE::CString& url ) const
{GUCEF_TRACE;

    CUrlPathTemplate parsed;
    return CUrlPathTemplate::Parse( url, parsed ) && CSiteDiscoveryRedactor::MatchesHostFilter( m_pageHostFilter, parsed.host );
}

/*-------------------------------------------------------------------------*/

bool
CWebResponseTap::Start( void )
{GUCEF_TRACE;

    if ( m_started )
        return true;
    if ( !m_client.IsConnected() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "WebResponseTap: The tap needs a connection to the browser" );
        return false;
    }
    TEventCallback callback( this, &CWebResponseTap::OnProtocolEvent );
    SubscribeTo( &m_client, CChromeDevToolsClient::ProtocolEvent, callback );
    m_started = true;

    CORE::CDataNode params( GUCEF_DATATYPE_OBJECT );
    params.SetAttribute( "discover", true );
    CORE::CDataNode result;
    TChromeTargetVector targets;
    if ( !m_client.SendCommandOnSessionAndWait( CORE::CString::Empty, "Target.setDiscoverTargets", &params, result ) ||
         !m_client.TargetGetTargets( targets ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "WebResponseTap: The browser would not list its tabs" );
        Stop();
        return false;
    }
    for ( size_t i=0; i<targets.size(); ++i )
        if ( targets[ i ].type == "page" && PagePassesFilter( targets[ i ].url ) )
            m_targetsToAttach.push_back( targets[ i ].id );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CWebResponseTap::Stop( void )
{GUCEF_TRACE;

    if ( m_started )
        UnsubscribeFrom( &m_client );
    m_started = false;
    m_inFlight.clear();
    m_ready.clear();
    m_targetsToAttach.clear();
    m_attachedTargets.clear();
    m_sessions.clear();
    m_sessionsToSetup.clear();
    m_sessionTargets.clear();
}

/*-------------------------------------------------------------------------*/

bool
CWebResponseTap::IsStarted( void ) const
{GUCEF_TRACE;

    return m_started;
}

/*-------------------------------------------------------------------------*/

void
CWebResponseTap::OnProtocolEvent( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( GUCEF_NULL == eventData )
        return;
    const SChromeProtocolEvent& event = static_cast< TChromeProtocolEventData* >( eventData )->GetData();
    if ( !event.hasParams )
        return;
    const CORE::CString& method = event.method;
    if ( 0 == strncmp( method.C_String(), "Target.", 7 ) )
    {
        HandleTarget( event );
        return;
    }
    // only the sessions of this tap: those of a recorder or a page driver are theirs
    if ( event.sessionId.IsNULLOrEmpty() || m_sessions.find( event.sessionId ) == m_sessions.end() )
        return;
    if ( method == "Network.requestWillBeSent" )
        HandleRequestWillBeSent( event );
    else
    if ( method == "Network.responseReceived" )
        HandleResponseReceived( event );
    else
    if ( method == "Network.loadingFinished" )
        HandleLoadingFinished( event );
    else
    if ( method == "Network.loadingFailed" )
        m_inFlight.erase( MakeKey( event.sessionId, GetText( event.params, "requestId" ) ) );
}

/*-------------------------------------------------------------------------*/

void
CWebResponseTap::HandleTarget( const SChromeProtocolEvent& event )
{GUCEF_TRACE;

    const CORE::CString& method = event.method;
    if ( method == "Target.attachedToTarget" )
    {
        // a frame or worker of a tab this tap watches
        if ( event.sessionId.IsNULLOrEmpty() || m_sessions.find( event.sessionId ) == m_sessions.end() )
            return;
        const CORE::CString sessionId = GetText( event.params, "sessionId" );
        const CORE::CDataNode* info = event.params.FindChild( "targetInfo" );
        if ( !sessionId.IsNULLOrEmpty() && m_sessions.find( sessionId ) == m_sessions.end() )
            m_sessionsToSetup[ sessionId ] = GUCEF_NULL != info ? GetText( *info, "type" ) : CORE::CString::Empty;
        return;
    }
    if ( method == "Target.detachedFromTarget" )
    {
        const CORE::CString sessionId = GetText( event.params, "sessionId" );
        m_sessions.erase( sessionId );
        m_sessionsToSetup.erase( sessionId );
        TStringMap::iterator target = m_sessionTargets.find( sessionId );
        if ( target != m_sessionTargets.end() )
        {
            m_attachedTargets.erase( target->second );
            m_sessionTargets.erase( target );
        }
        return;
    }
    if ( ( method == "Target.targetCreated" || method == "Target.targetInfoChanged" ) && event.sessionId.IsNULLOrEmpty() )
    {
        const CORE::CDataNode* info = event.params.FindChild( "targetInfo" );
        if ( GUCEF_NULL == info || GetText( *info, "type" ) != "page" || !PagePassesFilter( GetText( *info, "url" ) ) )
            return;
        const CORE::CString targetId = GetText( *info, "targetId" );
        if ( m_attachedTargets.find( targetId ) == m_attachedTargets.end() )
            m_targetsToAttach.push_back( targetId );
    }
}

/*-------------------------------------------------------------------------*/

void
CWebResponseTap::HandleRequestWillBeSent( const SChromeProtocolEvent& event )
{GUCEF_TRACE;

    const CORE::CString key = MakeKey( event.sessionId, GetText( event.params, "requestId" ) );
    m_inFlight.erase( key );        // a redirect reuses the id: the earlier leg had no body to give

    const CORE::CDataNode* request = event.params.FindChild( "request" );
    if ( GUCEF_NULL == request || m_taps.empty() )
        return;
    CUrlPathTemplate parsed;
    if ( !CUrlPathTemplate::Parse( GetText( *request, "url" ), parsed ) || m_neverRecord.IsNeverRecord( parsed ) )
        return;     // sign-in and payment traffic is never matched

    SPending pending;
    pending.method = GetText( *request, "method" ).Uppercase();
    for ( TTapMap::const_iterator i = m_taps.begin(); i != m_taps.end(); ++i )
        if ( IsMatch( i->second.method, i->second.hostPattern, i->second.pathPattern, pending.method, parsed ) )
            pending.tapIds.push_back( i->first );
    if ( pending.tapIds.empty() )
        return;

    pending.sessionId = event.sessionId;
    pending.requestId = GetText( event.params, "requestId" );
    pending.host = parsed.host.Lowercase();
    pending.pathTemplate = parsed.pathTemplate;
    pending.sequence = m_nextSequence++;
    if ( m_inFlight.size() >= MaxInFlight )
        m_inFlight.clear();
    m_inFlight[ key ] = pending;
}

/*-------------------------------------------------------------------------*/

void
CWebResponseTap::HandleResponseReceived( const SChromeProtocolEvent& event )
{GUCEF_TRACE;

    TPendingMap::iterator i = m_inFlight.find( MakeKey( event.sessionId, GetText( event.params, "requestId" ) ) );
    const CORE::CDataNode* response = event.params.FindChild( "response" );
    if ( i == m_inFlight.end() || GUCEF_NULL == response )
        return;
    i->second.status = (Int32) response->GetAttributeValue( "status" ).AsInt64( 0 );
    i->second.mimeType = GetText( *response, "mimeType" );
}

/*-------------------------------------------------------------------------*/

void
CWebResponseTap::HandleLoadingFinished( const SChromeProtocolEvent& event )
{GUCEF_TRACE;

    TPendingMap::iterator i = m_inFlight.find( MakeKey( event.sessionId, GetText( event.params, "requestId" ) ) );
    if ( i == m_inFlight.end() )
        return;
    i->second.encodedDataLength = (UInt64) event.params.GetAttributeValue( "encodedDataLength" ).AsFloat64( 0.0 );
    m_ready.push_back( i->second );
    m_inFlight.erase( i );
}

/*-------------------------------------------------------------------------*/

bool
CWebResponseTap::SetupSession( const CORE::CString& sessionId )
{GUCEF_TRACE;

    if ( !m_sessions.insert( sessionId ).second )
        return true;

    // the browser keeps responses until they are asked for, up to these sizes
    CORE::CDataNode enable( GUCEF_DATATYPE_OBJECT );
    enable.SetAttribute( "maxTotalBufferSize", (UInt32) ( 128 * 1024 * 1024 ) );
    enable.SetAttribute( "maxResourceBufferSize", MaxBodyBytesCeiling );
    CORE::CDataNode result;
    bool ok = m_client.SendCommandOnSessionAndWait( sessionId, "Network.enable", &enable, result, GUCEF_NULL, 15000 );

    // frames (also of other sites) and workers of the tab get a session of their own
    CORE::CDataNode autoAttach( GUCEF_DATATYPE_OBJECT );
    autoAttach.SetAttribute( "autoAttach", true );
    autoAttach.SetAttribute( "waitForDebuggerOnStart", false );
    autoAttach.SetAttribute( "flatten", true );
    ok = m_client.SendCommandOnSessionAndWait( sessionId, "Target.setAutoAttach", &autoAttach, result, GUCEF_NULL, 15000 ) && ok;
    if ( !ok )
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "WebResponseTap: Not everything could be set up to watch session " << sessionId );
    return ok;
}

/*-------------------------------------------------------------------------*/

void
CWebResponseTap::EndWait( STap& tap, EWaitState state )
{GUCEF_TRACE;

    tap.waitState = state;
    tap.deliveredSeen = tap.delivered;
    tap.tooLargeSeen = tap.tooLarge;
    CORE::TCloneableUInt32 eventData( tap.id );
    NotifyObservers( WaitEndedEvent, &eventData );
}

/*-------------------------------------------------------------------------*/

void
CWebResponseTap::Deliver( SPending& pending )
{GUCEF_TRACE;

    // the largest size any of its taps takes; a response larger than that is not fetched at all
    UInt32 largest = 0;
    for ( size_t i=0; i<pending.tapIds.size(); ++i )
    {
        TTapMap::const_iterator tap = m_taps.find( pending.tapIds[ i ] );
        if ( tap != m_taps.end() && tap->second.maxBodyBytes > largest )
            largest = tap->second.maxBodyBytes;
    }
    if ( 0 == largest )
        return;     // its taps were removed

    bool fetched = false;
    if ( pending.encodedDataLength <= largest )
    {
        CORE::CDataNode params( GUCEF_DATATYPE_OBJECT );
        params.SetAttribute( "requestId", CORE::CVariant( pending.requestId ) );
        CORE::CDataNode result;
        if ( m_client.SendCommandOnSessionAndWait( pending.sessionId, "Network.getResponseBody", &params, result, GUCEF_NULL, 15000 ) )
        {
            const CORE::CString body = GetText( result, "body" );
            m_body.Clear( true );
            if ( result.GetAttributeValue( "base64Encoded" ).AsBool( false ) )
                m_body.CopyAndDecodeBase64From( body, 0 );
            else
            if ( !body.IsNULLOrEmpty() )
                m_body.CopyFrom( body.ByteSize() - 1, body.C_String() );
            result.Clear();
            fetched = true;
        }
        else
            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "WebResponseTap: The browser no longer has the body of " << pending.method << " " << pending.host << pending.pathTemplate );
    }

    for ( size_t i=0; i<pending.tapIds.size(); ++i )
    {
        TTapMap::iterator tap = m_taps.find( pending.tapIds[ i ] );
        if ( tap == m_taps.end() || ( !fetched && pending.encodedDataLength <= tap->second.maxBodyBytes ) )
            continue;
        if ( !fetched || m_body.GetDataSize() > tap->second.maxBodyBytes )
        {
            ++tap->second.tooLarge;
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "WebResponseTap: A response of " << pending.method << " " << pending.host << pending.pathTemplate <<
                       " is larger than tap " << tap->first << " takes, it is not delivered" );
            continue;
        }
        CObservedResponseBody response( m_body );
        response.method = pending.method;
        response.host = pending.host;
        response.pathTemplate = pending.pathTemplate;
        response.status = pending.status;
        response.mimeType = pending.mimeType;
        response.requestSeq = pending.sequence;
        response.tapId = tap->first;
        tap->second.sink->OnResponseBody( response );
        ++tap->second.delivered;
        CORE::TCloneableUInt32 eventData( tap->first );
        NotifyObservers( ResponseDeliveredEvent, &eventData );
    }

    // the account data held here is gone once it was handed over
    if ( m_body.GetBufferSize() > 0 )
        memset( m_body.GetBufferPtr(), 0, m_body.GetBufferSize() );
    m_body.Clear( true );
}

/*-------------------------------------------------------------------------*/

UInt32
CWebResponseTap::Process( void )
{GUCEF_TRACE;

    UInt32 done = 0;
    if ( m_started )
    {
        // the queues are taken first: commands pump the event loop, more may be announced meanwhile
        CORE::CString::StringVector targets;
        targets.swap( m_targetsToAttach );
        for ( size_t i=0; i<targets.size(); ++i )
        {
            if ( !m_attachedTargets.insert( targets[ i ] ).second )
                continue;
            CORE::CString sessionId;
            CORE::CDataNode params( GUCEF_DATATYPE_OBJECT );
            params.SetAttribute( "targetId", CORE::CVariant( targets[ i ] ) );
            params.SetAttribute( "flatten", true );
            CORE::CDataNode result;
            if ( m_client.SendCommandOnSessionAndWait( CORE::CString::Empty, "Target.attachToTarget", &params, result, GUCEF_NULL, 15000 ) &&
                 !( sessionId = GetText( result, "sessionId" ) ).IsNULLOrEmpty() )
            {
                m_sessionTargets[ sessionId ] = targets[ i ];
                SetupSession( sessionId );
                ++done;
            }
            else
            {
                m_attachedTargets.erase( targets[ i ] );
                GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "WebResponseTap: Unable to attach to tab " << targets[ i ] );
            }
        }

        TStringMap sessions;
        sessions.swap( m_sessionsToSetup );
        for ( TStringMap::const_iterator i = sessions.begin(); i != sessions.end(); ++i )
        {
            SetupSession( i->first );
            ++done;
        }

        TPendingVector ready;
        ready.swap( m_ready );
        for ( size_t i=0; i<ready.size(); ++i )
        {
            Deliver( ready[ i ] );
            ++done;
        }
    }

    // the waits
    const UInt64 now = NowInMs();
    for ( TTapMap::iterator i = m_taps.begin(); i != m_taps.end(); ++i )
    {
        STap& tap = i->second;
        if ( WAIT_PENDING != tap.waitState )
            continue;
        if ( tap.delivered > tap.deliveredSeen )
            EndWait( tap, WAIT_RECEIVED );
        else
        if ( tap.tooLarge > tap.tooLargeSeen )
            EndWait( tap, WAIT_TOO_LARGE );
        else
        if ( now >= tap.waitDeadline )
            EndWait( tap, WAIT_TIMED_OUT );
    }
    return done;
}

/*-------------------------------------------------------------------------*/

bool
CWebResponseTap::WaitForResponse( UInt32 tapId, UInt32 timeoutInMs )
{GUCEF_TRACE;

    TTapMap::iterator i = m_taps.find( tapId );
    if ( i == m_taps.end() )
        return false;
    i->second.waitState = WAIT_PENDING;
    i->second.waitDeadline = NowInMs() + timeoutInMs;
    return true;
}

/*-------------------------------------------------------------------------*/

CWebResponseTap::EWaitState
CWebResponseTap::GetWaitState( UInt32 tapId ) const
{GUCEF_TRACE;

    TTapMap::const_iterator i = m_taps.find( tapId );
    return i == m_taps.end() ? WAIT_NONE : i->second.waitState;
}

/*-------------------------------------------------------------------------*/

UInt32
CWebResponseTap::GetDeliveredCount( UInt32 tapId ) const
{GUCEF_TRACE;

    TTapMap::const_iterator i = m_taps.find( tapId );
    return i == m_taps.end() ? 0 : i->second.delivered;
}

/*-------------------------------------------------------------------------*/

UInt32
CWebResponseTap::GetSessionCount( void ) const
{GUCEF_TRACE;

    return (UInt32) m_sessions.size();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
