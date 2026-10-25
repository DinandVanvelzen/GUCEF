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

#ifndef ANCESTRYBACKUP_CURLTEMPLATE_H
#include "AncestryBackup_CUrlTemplate.h"
#define ANCESTRYBACKUP_CURLTEMPLATE_H
#endif /* ANCESTRYBACKUP_CURLTEMPLATE_H ? */

#ifndef ANCESTRYBACKUP_CJSONSHAPE_H
#include "AncestryBackup_CJsonShape.h"
#define ANCESTRYBACKUP_CJSONSHAPE_H
#endif /* ANCESTRYBACKUP_CJSONSHAPE_H ? */

#ifndef GUCEF_WEB_CJSONRPCENDPOINT_H
#include "gucefWEB_CJsonRpcEndpoint.h"
#define GUCEF_WEB_CJSONRPCENDPOINT_H
#endif /* GUCEF_WEB_CJSONRPCENDPOINT_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CREDACTOR_H
#include "gucefCORE_CRedactor.h"
#define GUCEF_CORE_CREDACTOR_H
#endif /* GUCEF_CORE_CREDACTOR_H ? */

#include <algorithm>
#include <cstdio>

#include "AncestryBackup_CChromeNetworkRecorder.h"

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

static std::string
Lower( const std::string& text )
{
    std::string lower = text;
    for ( size_t i=0; i<lower.size(); ++i )
        lower[ i ] = (char) tolower( lower[ i ] );
    return lower;
}

/*-------------------------------------------------------------------------*/

static std::string
GetText( const CORE::CDataNode& node, const char* name )
{
    return node.GetAttributeValue( name ).AsString().STL_String().c_str();
}

/*-------------------------------------------------------------------------*/

CChromeNetworkRecorder::CChromeNetworkRecorder( WEB::CChromeDevToolsClient& client ,
                                                const CDataLayout& layout          ,
                                                CEndpointCatalog& catalog          ,
                                                CDiscoveryJournal& journal         )
    : CORE::CObserver()
    , m_client( client )
    , m_layout( layout )
    , m_catalog( catalog )
    , m_journal( journal )
    , m_hostFilter()
    , m_maxBodyBytes( 32ULL * 1024ULL * 1024ULL )
    , m_started( false )
    , m_inFlight()
    , m_finished()
    , m_completedWithoutBody()
    , m_targetsToAttach()
    , m_sessionsToSetup()
    , m_sessions()
    , m_attachedTargets()
    , m_newEndpoints()
    , m_recordedCount( 0 )
    , m_savedBodyCount( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CChromeNetworkRecorder::~CChromeNetworkRecorder()
{GUCEF_TRACE;

    Stop();
}

/*-------------------------------------------------------------------------*/

void
CChromeNetworkRecorder::SetHostFilter( const std::vector< std::string >& terms )
{GUCEF_TRACE;

    m_hostFilter.clear();
    for ( size_t i=0; i<terms.size(); ++i )
        if ( !terms[ i ].empty() )
            m_hostFilter.push_back( Lower( terms[ i ] ) );
}

/*-------------------------------------------------------------------------*/

void
CChromeNetworkRecorder::SetMaxBodyBytes( UInt64 maxBodyBytes )
{GUCEF_TRACE;

    m_maxBodyBytes = maxBodyBytes;
}

/*-------------------------------------------------------------------------*/

bool
CChromeNetworkRecorder::MatchesFilter( const std::vector< std::string >& terms, const std::string& host )
{GUCEF_TRACE;

    if ( terms.empty() )
        return true;
    const std::string lowerHost = Lower( host );
    for ( size_t i=0; i<terms.size(); ++i )
        if ( lowerHost.find( Lower( terms[ i ] ) ) != std::string::npos )
            return true;
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CChromeNetworkRecorder::IsStarted( void ) const
{GUCEF_TRACE;

    return m_started;
}

/*-------------------------------------------------------------------------*/

bool
CChromeNetworkRecorder::Start( void )
{GUCEF_TRACE;

    if ( m_started )
        return true;
    if ( !m_client.IsConnected() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "AncestryBackup: The recorder needs a connection to the browser" );
        return false;
    }

    SubscribeTo( &m_client );
    m_started = true;

    // be told about tabs that get opened
    CORE::CDataNode params( GUCEF_DATATYPE_OBJECT );
    params.SetAttribute( "discover", true );
    CORE::CDataNode result;
    if ( !m_client.SendCommandAndWait( "Target.setDiscoverTargets", &params, result ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "AncestryBackup: The browser would not tell about new tabs" );
        Stop();
        return false;
    }

    // and watch the ones that are open right now
    CORE::CDataNode targets;
    if ( !m_client.SendCommandAndWait( "Target.getTargets", GUCEF_NULL, targets ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "AncestryBackup: The browser would not list its tabs" );
        Stop();
        return false;
    }
    const CORE::CDataNode* infos = targets.Find( "targetInfos" );
    if ( GUCEF_NULL != infos )
    {
        for ( CORE::CDataNode::const_iterator i = infos->ConstBegin(); i != infos->ConstEnd(); ++i )
        {
            if ( GetText( *(*i), "type" ) == "page" && UrlPassesFilter( GetText( *(*i), "url" ) ) )
                m_targetsToAttach.push_back( GetText( *(*i), "targetId" ) );
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
CChromeNetworkRecorder::Stop( void )
{GUCEF_TRACE;

    if ( m_started )
        UnsubscribeFrom( m_client );
    m_started = false;
}

/*-------------------------------------------------------------------------*/

std::string
CChromeNetworkRecorder::MakeKey( const std::string& sessionId, const std::string& requestId )
{GUCEF_TRACE;

    return sessionId + "|" + requestId;
}

/*-------------------------------------------------------------------------*/

void
CChromeNetworkRecorder::OnNotify( CORE::CNotifier* notifier   ,
                                  const CORE::CEvent& eventid ,
                                  CORE::CICloneable* eventdata )
{GUCEF_TRACE;

    if ( eventid == WEB::CChromeDevToolsClient::ProtocolEvent && GUCEF_NULL != eventdata )
        HandleProtocolEvent( static_cast< WEB::TChromeProtocolEventData* >( eventdata )->GetData() );
}

/*-------------------------------------------------------------------------*/

void
CChromeNetworkRecorder::HandleProtocolEvent( const WEB::SChromeProtocolEvent& event )
{GUCEF_TRACE;

    if ( !event.hasParams )
        return;

    const std::string method = event.method.STL_String().c_str();
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
        HandleLoadingFailed( event );
    else
    if ( method == "Target.attachedToTarget" )
        HandleAttachedToTarget( event );
    else
    if ( method == "Target.targetCreated" )
        HandleTargetCreated( event );
    else
    if ( method == "Target.targetInfoChanged" )
        HandleTargetInfoChanged( event );
}

/*-------------------------------------------------------------------------*/

void
CChromeNetworkRecorder::HandleRequestWillBeSent( const WEB::SChromeProtocolEvent& event )
{GUCEF_TRACE;

    const std::string sessionId = event.sessionId.STL_String().c_str();
    const std::string requestId = GetText( event.params, "requestId" );
    const std::string key = MakeKey( sessionId, requestId );

    // a redirect reuses the request id: the previous leg of the request ended with the redirect response
    TInFlightMap::iterator previous = m_inFlight.find( key );
    if ( previous != m_inFlight.end() )
    {
        const CORE::CDataNode* redirect = event.params.Find( "redirectResponse" );
        if ( GUCEF_NULL != redirect )
        {
            previous->second.observation.status = (Int32) redirect->GetAttributeValue( "status" ).AsInt64( 0 );
            previous->second.observation.mimeType = GetText( *redirect, "mimeType" );
        }
        m_completedWithoutBody.push_back( previous->second );
        m_inFlight.erase( previous );
    }

    const CORE::CDataNode* request = event.params.Find( "request" );
    if ( GUCEF_NULL == request )
        return;

    SPendingFinish record;
    record.sessionId = sessionId;
    record.requestId = requestId;
    SObservedRequest& observation = record.observation;
    observation.url = GetText( *request, "url" );
    observation.method = GetText( *request, "method" );
    observation.resourceType = GetText( event.params, "type" );
    observation.timestamp = event.params.GetAttributeValue( "timestamp" ).AsFloat64( 0.0 );
    observation.postData = GetText( *request, "postData" );
    if ( m_catalog.IsExcluded( observation.url ) && !observation.postData.empty() )
        observation.postData = "<not recorded: sensitive>";      // a password is sent in one of these

    SUrlTemplate parsed;
    if ( !CUrlTemplate::Parse( CORE::CString( observation.url.c_str() ), parsed ) || !MatchesFilter( m_hostFilter, parsed.host.STL_String().c_str() ) )
        return;

    const CORE::CDataNode* headers = request->Find( "headers" );
    if ( GUCEF_NULL != headers )
    {
        for ( UInt32 i=0; i<headers->GetAttCount(); ++i )
        {
            const CORE::CDataNode::TKeyValuePair* att = headers->GetAttribute( i );
            if ( GUCEF_NULL == att )
                continue;
            const std::string name = Lower( att->first.STL_String().c_str() );
            observation.requestHeaders[ name ] = CEndpointCatalog::IsSecretHeader( name ) ? std::string( "<not recorded>" ) : std::string( att->second.AsString().STL_String().c_str() );
        }
    }
    m_inFlight[ key ] = record;
}

/*-------------------------------------------------------------------------*/

void
CChromeNetworkRecorder::HandleResponseReceived( const WEB::SChromeProtocolEvent& event )
{GUCEF_TRACE;

    TInFlightMap::iterator i = m_inFlight.find( MakeKey( event.sessionId.STL_String().c_str(), GetText( event.params, "requestId" ) ) );
    if ( i == m_inFlight.end() )
        return;

    const CORE::CDataNode* response = event.params.Find( "response" );
    if ( GUCEF_NULL != response )
    {
        i->second.observation.status = (Int32) response->GetAttributeValue( "status" ).AsInt64( 0 );
        i->second.observation.mimeType = GetText( *response, "mimeType" );
    }
    const std::string type = GetText( event.params, "type" );
    if ( !type.empty() )
        i->second.observation.resourceType = type;
}

/*-------------------------------------------------------------------------*/

void
CChromeNetworkRecorder::HandleLoadingFinished( const WEB::SChromeProtocolEvent& event )
{GUCEF_TRACE;

    TInFlightMap::iterator i = m_inFlight.find( MakeKey( event.sessionId.STL_String().c_str(), GetText( event.params, "requestId" ) ) );
    if ( i == m_inFlight.end() )
        return;

    i->second.observation.encodedDataLength = (UInt64) event.params.GetAttributeValue( "encodedDataLength" ).AsFloat64( 0.0 );
    m_finished.push_back( i->second );
    m_inFlight.erase( i );
}

/*-------------------------------------------------------------------------*/

void
CChromeNetworkRecorder::HandleLoadingFailed( const WEB::SChromeProtocolEvent& event )
{GUCEF_TRACE;

    TInFlightMap::iterator i = m_inFlight.find( MakeKey( event.sessionId.STL_String().c_str(), GetText( event.params, "requestId" ) ) );
    if ( i == m_inFlight.end() )
        return;

    i->second.observation.failed = true;
    i->second.observation.errorText = GetText( event.params, "errorText" );
    m_completedWithoutBody.push_back( i->second );
    m_inFlight.erase( i );
}

/*-------------------------------------------------------------------------*/

void
CChromeNetworkRecorder::HandleAttachedToTarget( const WEB::SChromeProtocolEvent& event )
{GUCEF_TRACE;

    // A target we attached to ourselves announces itself as well, and so does one that someone else attached to:
    // watching those twice would show every request twice. Only the frames and workers that come with the
    // tabs we watch are new.
    const std::string sessionId = GetText( event.params, "sessionId" );
    const CORE::CDataNode* info = event.params.Find( "targetInfo" );
    const std::string targetId = GUCEF_NULL != info ? GetText( *info, "targetId" ) : std::string();
    if ( sessionId.empty() || m_sessions.find( sessionId ) != m_sessions.end() )
        return;
    if ( !targetId.empty() && !m_attachedTargets.insert( targetId ).second )
        return;
    m_sessionsToSetup.push_back( sessionId );
}

/*-------------------------------------------------------------------------*/

bool
CChromeNetworkRecorder::UrlPassesFilter( const std::string& url ) const
{GUCEF_TRACE;

    // a tab is only watched when it shows the site: the other tabs of the user are not ours to look at
    SUrlTemplate parsed;
    return CUrlTemplate::Parse( CORE::CString( url.c_str() ), parsed ) && MatchesFilter( m_hostFilter, parsed.host.STL_String() );
}

/*-------------------------------------------------------------------------*/

void
CChromeNetworkRecorder::HandleTargetCreated( const WEB::SChromeProtocolEvent& event )
{GUCEF_TRACE;

    const CORE::CDataNode* info = event.params.Find( "targetInfo" );
    if ( GUCEF_NULL != info && GetText( *info, "type" ) == "page" && UrlPassesFilter( GetText( *info, "url" ) ) )
        m_targetsToAttach.push_back( GetText( *info, "targetId" ) );
}

/*-------------------------------------------------------------------------*/

void
CChromeNetworkRecorder::HandleTargetInfoChanged( const WEB::SChromeProtocolEvent& event )
{GUCEF_TRACE;

    // a tab that was somewhere else and went to the site
    const CORE::CDataNode* info = event.params.Find( "targetInfo" );
    if ( GUCEF_NULL == info || GetText( *info, "type" ) != "page" || !UrlPassesFilter( GetText( *info, "url" ) ) )
        return;
    const std::string targetId = GetText( *info, "targetId" );
    if ( m_attachedTargets.find( targetId ) == m_attachedTargets.end() )
        m_targetsToAttach.push_back( targetId );
}

/*-------------------------------------------------------------------------*/

bool
CChromeNetworkRecorder::SendOnSession( const std::string& sessionId, const CORE::CString& method, const CORE::CDataNode* params )
{GUCEF_TRACE;

    m_client.SetSessionId( sessionId.c_str() );
    CORE::CDataNode result;
    const bool ok = m_client.SendCommandAndWait( method, params, result, GUCEF_NULL, 15000 );
    m_client.SetSessionId( CORE::CString::Empty );
    return ok;
}

/*-------------------------------------------------------------------------*/

bool
CChromeNetworkRecorder::SetupSession( const std::string& sessionId )
{GUCEF_TRACE;

    if ( !m_sessions.insert( sessionId ).second )
        return true;

    // room for the browser to keep responses around until we ask for them
    CORE::CDataNode enable( GUCEF_DATATYPE_OBJECT );
    enable.SetAttribute( "maxTotalBufferSize", (UInt32) ( 200 * 1024 * 1024 ) );
    enable.SetAttribute( "maxResourceBufferSize", (UInt32) ( 50 * 1024 * 1024 ) );
    bool ok = SendOnSession( sessionId, "Network.enable", &enable );

    // a response that comes from the cache is a response we may not see being requested
    CORE::CDataNode cache( GUCEF_DATATYPE_OBJECT );
    cache.SetAttribute( "cacheDisabled", true );
    ok = SendOnSession( sessionId, "Network.setCacheDisabled", &cache ) && ok;

    // frames and workers of the page get their own session
    CORE::CDataNode autoAttach( GUCEF_DATATYPE_OBJECT );
    autoAttach.SetAttribute( "autoAttach", true );
    autoAttach.SetAttribute( "waitForDebuggerOnStart", false );
    autoAttach.SetAttribute( "flatten", true );
    ok = SendOnSession( sessionId, "Target.setAutoAttach", &autoAttach ) && ok;

    if ( !ok )
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "AncestryBackup: Not everything could be set up to watch the network of session " + CORE::CString( sessionId.c_str() ) );
    return ok;
}

/*-------------------------------------------------------------------------*/

bool
CChromeNetworkRecorder::ShouldSaveBody( const SObservedRequest& observation ) const
{GUCEF_TRACE;

    if ( observation.status < 200 || observation.status > 299 || observation.encodedDataLength > m_maxBodyBytes )
        return false;
    if ( m_catalog.IsExcluded( observation.url ) )
        return false;
    if ( CEndpointCatalog::Classify( observation ) != CEndpointCatalog::KIND_DATA )
        return false;
    const std::string mime = Lower( observation.mimeType );
    return mime.find( "json" ) != std::string::npos || mime.find( "xml" ) != std::string::npos || mime.find( "text/plain" ) != std::string::npos;
}

/*-------------------------------------------------------------------------*/

void
CChromeNetworkRecorder::Complete( SPendingFinish& finished, bool fetchBody )
{GUCEF_TRACE;

    SObservedRequest& observation = finished.observation;
    observation.sequence = m_journal.GetNextSequence();

    SJsonShape shape;
    bool haveShape = false;

    if ( fetchBody && ShouldSaveBody( observation ) )
    {
        CORE::CString body;
        bool isBase64 = false;
        m_client.SetSessionId( finished.sessionId.c_str() );
        const bool gotBody = m_client.GetResponseBody( finished.requestId.c_str(), body, isBase64 );
        m_client.SetSessionId( CORE::CString::Empty );

        if ( gotBody && !isBase64 )
        {
            const std::string mime = Lower( observation.mimeType );
            const char* ext = mime.find( "json" ) != std::string::npos ? "json" : ( mime.find( "xml" ) != std::string::npos ? "xml" : "txt" );
            char name[ 64 ];
            snprintf( name, sizeof( name ), "bodies/%06u.%s", (unsigned int) observation.sequence, ext );
            if ( CORE::WriteStringAsTextFile( CORE::CombinePath( m_layout.GetDiscoveryDir(), name ), body ) )
            {
                observation.bodyFile = name;
                ++m_savedBodyCount;

                CORE::CDataNode document;
                if ( m_catalog.NeedsShape( observation ) && WEB::CJsonRpcEndpoint::ParseJsonText( body, document ) )
                {
                    CJsonShape::Describe( document, shape );
                    haveShape = true;
                }
            }
        }
        else
            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "AncestryBackup: The browser no longer has the body of " << CORE::CRedactor::ForUrl().RedactUrl( CORE::CString( observation.url.c_str() ) ) );
    }

    ++m_recordedCount;
    m_journal.Append( observation );
    if ( m_catalog.Add( observation, haveShape ? &shape : GUCEF_NULL ) )
    {
        SUrlTemplate parsed;
        CUrlTemplate::Parse( CORE::CString( observation.url.c_str() ), parsed );
        m_newEndpoints.push_back( observation.method + " " + parsed.GetKey().STL_String().c_str() + " (" + observation.resourceType + ")" );
    }
}

/*-------------------------------------------------------------------------*/

UInt32
CChromeNetworkRecorder::Process( void )
{GUCEF_TRACE;

    UInt32 done = 0;

    // new tabs. The queue is taken first: commands pump the event loop and more tabs may be announced meanwhile
    std::vector< std::string > targets;
    targets.swap( m_targetsToAttach );
    for ( size_t i=0; i<targets.size(); ++i )
    {
        if ( !m_attachedTargets.insert( targets[ i ] ).second )
            continue;
        CORE::CString sessionId;
        if ( m_client.AttachToTarget( targets[ i ].c_str(), sessionId ) )
        {
            SetupSession( sessionId.STL_String().c_str() );
            ++done;
        }
        else
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "AncestryBackup: Unable to attach to tab " + CORE::CString( targets[ i ].c_str() ) );
    }

    std::vector< std::string > sessions;
    sessions.swap( m_sessionsToSetup );
    for ( size_t i=0; i<sessions.size(); ++i )
    {
        SetupSession( sessions[ i ] );
        ++done;
    }

    std::vector< SPendingFinish > withoutBody;
    withoutBody.swap( m_completedWithoutBody );
    for ( size_t i=0; i<withoutBody.size(); ++i )
    {
        Complete( withoutBody[ i ], false );
        ++done;
    }

    std::vector< SPendingFinish > finished;
    finished.swap( m_finished );
    for ( size_t i=0; i<finished.size(); ++i )
    {
        Complete( finished[ i ], true );
        ++done;
    }
    return done;
}

/*-------------------------------------------------------------------------*/

bool
CChromeNetworkRecorder::PopNewEndpoint( std::string& description )
{GUCEF_TRACE;

    if ( m_newEndpoints.empty() )
        return false;
    description = m_newEndpoints.front();
    m_newEndpoints.erase( m_newEndpoints.begin() );
    return true;
}

/*-------------------------------------------------------------------------*/

UInt32
CChromeNetworkRecorder::GetRecordedRequestCount( void ) const
{GUCEF_TRACE;

    return m_recordedCount;
}

/*-------------------------------------------------------------------------*/

UInt32
CChromeNetworkRecorder::GetSavedBodyCount( void ) const
{GUCEF_TRACE;

    return m_savedBodyCount;
}

/*-------------------------------------------------------------------------*/

UInt32
CChromeNetworkRecorder::GetSessionCount( void ) const
{GUCEF_TRACE;

    return (UInt32) m_sessions.size();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ANCESTRYBACKUP */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
