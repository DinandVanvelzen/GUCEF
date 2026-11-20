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

#include <stdio.h>
#include <string.h>
#include <algorithm>

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

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_COM_CJSONRPCENDPOINT_H
#include "gucefCOM_CJsonRpcEndpoint.h"
#define GUCEF_COM_CJSONRPCENDPOINT_H
#endif /* GUCEF_COM_CJSONRPCENDPOINT_H ? */

#ifndef GUCEF_WEB_CURLPATHTEMPLATE_H
#include "gucefWEB_CUrlPathTemplate.h"
#define GUCEF_WEB_CURLPATHTEMPLATE_H
#endif /* GUCEF_WEB_CURLPATHTEMPLATE_H ? */

#ifndef GUCEF_WEB_CSITEENDPOINTCATALOG_H
#include "gucefWEB_CSiteEndpointCatalog.h"
#define GUCEF_WEB_CSITEENDPOINTCATALOG_H
#endif /* GUCEF_WEB_CSITEENDPOINTCATALOG_H ? */

#include "gucefWEB_CChromeNetworkRecorder.h"

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

static CORE::CString
GetText( const CORE::CDataNode& node, const char* name )
{
    return node.GetAttributeValue( name ).AsString( CORE::CString::Empty, false );
}

/*-------------------------------------------------------------------------*/

static bool
Contains( const CORE::CString& text, const char* part )
{
    return !text.IsNULLOrEmpty() && GUCEF_NULL != strstr( text.C_String(), part );
}

/*-------------------------------------------------------------------------*/

/** The content type without its parameters, lower case */
static CORE::CString
BareContentType( const CORE::CString& value )
{
    const Int32 semicolon = value.HasChar( ';' );
    const CORE::CString type = semicolon >= 0 ? value.SubstrToIndex( (UInt32) semicolon, true ) : value;
    return type.Trim( true ).Trim( false ).Lowercase();
}

/*-------------------------------------------------------------------------*/

/** A header of a header object, by its name in any case */
static CORE::CString
FindHeader( const CORE::CDataNode* headers, const char* lowerCaseName )
{
    if ( GUCEF_NULL == headers )
        return CORE::CString::Empty;
    for ( UInt32 i=0; i<headers->GetAttCount(); ++i )
    {
        const CORE::CDataNode::TKeyValuePair* att = headers->GetAttribute( i );
        if ( GUCEF_NULL != att && att->first.Lowercase() == lowerCaseName )
            return att->second.AsString();
    }
    return CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

/**
 *  The passive click and submit listener. It runs in an isolated world (ClickListenerWorldName) where page scripts
 *  cannot reach it, follows the composed path of the event so clicks inside shadow roots count, and reports the role and
 *  the label of the control: aria-label, else the visible text (80 characters at most), else title. Never the value of
 *  an input, never anything of a password field, never the text of a table cell. A control inside a table cell is
 *  reported with its own label, like any other, unless the settings withhold it (reportTableCellControlNames).
 */
static const char* const ClickListenerScript =
    "(function(){if(window.__gucefSiteDiscoveryListener)return;window.__gucefSiteDiscoveryListener=true;"
    "var B='__gucefSiteDiscoveryAction';"
    "var C='button,a[href],summary,select,option,[role=button],[role=link],[role=tab],[role=menuitem],[role=menuitemcheckbox],"
    "[role=menuitemradio],[role=option],[role=checkbox],[role=switch],[role=radio],input[type=submit],input[type=button],"
    "input[type=reset],input[type=image],input[type=checkbox],input[type=radio]';"
    "var T='td,th,[role=cell],[role=gridcell],[role=rowheader],[role=columnheader]';"
    "function up(n){return n.parentElement||(n.parentNode&&n.parentNode.host)||null;}"
    "function find(n,s){for(;n;n=up(n)){if(n.nodeType===1&&n.matches&&n.matches(s))return n;}return null;}"
    "function clean(v){return String(v||'').replace(/\\s+/g,' ').trim().slice(0,80);}"
    "function roleOf(el,tag,type){var r=el.getAttribute('role');if(r)return r.split(' ')[0];"
    "if(tag==='a')return 'link';if(tag==='select')return 'select';if(tag==='option')return 'option';if(tag==='form')return 'form';"
    "if(tag==='input'){if(type==='checkbox'||type==='radio')return type;if(/^(submit|button|reset|image)$/.test(type))return 'button';return 'textbox';}"
    "if(tag==='textarea')return 'textbox';if(tag==='button'||tag==='summary')return 'button';return '';}"
    "function report(e,kind){try{var p=e.composedPath?e.composedPath():[];var t=p.length?p[0]:e.target;"
    "if(t&&t.nodeType!==1)t=up(t);if(kind==='submit'&&e.submitter)t=e.submitter;"
    "var el=find(t,C)||t;if(!el||!el.getAttribute)return;"
    "var tag=(el.tagName||'').toLowerCase();var type=(el.getAttribute('type')||'').toLowerCase();"
    "var role=roleOf(el,tag,type);var inTable=!!find(el,T);var label='';"
    "if(tag==='input'&&type==='password'){label='';}"
    "else if(tag==='form'||tag==='select'||tag==='textarea'||(tag==='input'&&!/^(submit|button|reset|image)$/.test(type))){label=el.getAttribute('aria-label')||el.getAttribute('title')||'';}"
    "else{label=el.getAttribute('aria-label')||(tag==='input'?el.getAttribute('value'):(el.innerText||el.textContent))||el.getAttribute('title')||'';}"
    "if(typeof window[B]==='function')window[B](JSON.stringify({kind:kind,tag:tag,role:role,label:clean(label),inTable:inTable}));}catch(x){}}"
    "document.addEventListener('click',function(e){report(e,'click');},true);"
    "document.addEventListener('submit',function(e){report(e,'submit');},true);})();";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

/** Fetching a response to look for a new shape stops after this many tries per allowed shape of an endpoint */
static const UInt32 ShapeProbesPerShape = 4;

/** The longest label kept of a clicked control, in bytes */
static const UInt32 MaxLabelBytes = 120;

/*-------------------------------------------------------------------------*/

/** A label as the listener reported it, with control characters as spaces and at most MaxLabelBytes, cut at a character boundary */
static CORE::CString
CleanLabel( const CORE::CString& label )
{
    const char* t = label.IsNULLOrEmpty() ? "" : label.C_String();
    UInt32 len = label.IsNULLOrEmpty() ? 0 : label.ByteSize() - 1;
    if ( len > MaxLabelBytes )
    {
        len = MaxLabelBytes;
        while ( len > 0 && ( ( (unsigned char) t[ len ] ) & 0xC0 ) == 0x80 )
            --len;
    }
    GUCEF::vector< char > buffer( t, t + len );
    for ( UInt32 i=0; i<len; ++i )
        if ( ( (unsigned char) buffer[ i ] ) < 0x20 || buffer[ i ] == 0x7F )
            buffer[ i ] = ' ';
    if ( 0 == len )
        return CORE::CString::Empty;
    return CORE::CString( &buffer[ 0 ], len ).Trim( true ).Trim( false );
}

/*-------------------------------------------------------------------------*/

const CORE::CString CChromeNetworkRecorder::ActionBindingName = "__gucefSiteDiscoveryAction";
const CORE::CString CChromeNetworkRecorder::ClickListenerWorldName = "__gucefSiteDiscovery";

/*-------------------------------------------------------------------------*/

CChromeNetworkRecorder::CChromeNetworkRecorder( CChromeDevToolsClient& client )
    : CORE::CObserver()
    , m_client( client )
    , m_ownRedactor()
    , m_redactor( &m_ownRedactor )
    , m_bodyDirectory()
    , m_sinks()
    , m_tracker()
    , m_started( false )
    , m_nextSequence( 1 )
    , m_startTicks( MT::PrecisionTickCount() )
    , m_inFlight()
    , m_finished()
    , m_completedWithoutBody()
    , m_extraInfo()
    , m_shapeProbes()
    , m_targetsToAttach()
    , m_sessionsToSetup()
    , m_sessions()
    , m_attachedTargets()
    , m_sessionTargets()
    , m_sessionPages()
    , m_sessionParents()
    , m_recordedCount( 0 )
    , m_savedBodyCount( 0 )
    , m_bodyDirectoryReady( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CChromeNetworkRecorder::~CChromeNetworkRecorder()
{GUCEF_TRACE;

    Stop();
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CChromeNetworkRecorder::GetClickListenerScript( void )
{GUCEF_TRACE;

    static const CORE::CString script = ClickListenerScript;
    return script;
}

/*-------------------------------------------------------------------------*/

void
CChromeNetworkRecorder::SetRedactor( CSiteDiscoveryRedactor* redactor )
{GUCEF_TRACE;

    m_redactor = GUCEF_NULL != redactor ? redactor : &m_ownRedactor;
}

/*-------------------------------------------------------------------------*/

CSiteDiscoveryRedactor&
CChromeNetworkRecorder::GetRedactor( void ) const
{GUCEF_TRACE;

    return *m_redactor;
}

/*-------------------------------------------------------------------------*/

void
CChromeNetworkRecorder::SetBodyDirectory( const CORE::CString& directory )
{GUCEF_TRACE;

    m_bodyDirectory = directory;
    m_bodyDirectoryReady = false;
}

/*-------------------------------------------------------------------------*/

void
CChromeNetworkRecorder::AddSink( CIWebObservationSink* sink )
{GUCEF_TRACE;

    if ( GUCEF_NULL == sink )
        return;
    for ( size_t i=0; i<m_sinks.size(); ++i )
        if ( m_sinks[ i ] == sink )
            return;
    m_sinks.push_back( sink );
}

/*-------------------------------------------------------------------------*/

void
CChromeNetworkRecorder::RemoveSink( CIWebObservationSink* sink )
{GUCEF_TRACE;

    for ( TSinkVector::iterator i = m_sinks.begin(); i != m_sinks.end(); ++i )
    {
        if ( *i == sink )
        {
            m_sinks.erase( i );
            return;
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CChromeNetworkRecorder::SetNextSequence( UInt32 sequence )
{GUCEF_TRACE;

    m_nextSequence = sequence > 0 ? sequence : 1;
}

/*-------------------------------------------------------------------------*/

UInt32
CChromeNetworkRecorder::GetNextSequence( void ) const
{GUCEF_TRACE;

    return m_nextSequence;
}

/*-------------------------------------------------------------------------*/

bool
CChromeNetworkRecorder::IsStarted( void ) const
{GUCEF_TRACE;

    return m_started;
}

/*-------------------------------------------------------------------------*/

UInt64
CChromeNetworkRecorder::NowInMs( void ) const
{GUCEF_TRACE;

    const Float64 ticksPerMs = MT::PrecisionTimerResolution() / 1000.0;
    return (UInt64) ( ( MT::PrecisionTickCount() - m_startTicks ) / ticksPerMs );
}

/*-------------------------------------------------------------------------*/

bool
CChromeNetworkRecorder::Start( void )
{GUCEF_TRACE;

    if ( m_started )
        return true;
    if ( !m_client.IsConnected() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ChromeNetworkRecorder: The recorder needs a connection to the browser" );
        return false;
    }

    m_tracker.Configure( m_redactor->GetSettings() );
    TEventCallback callback( this, &CChromeNetworkRecorder::OnProtocolEvent );
    SubscribeTo( &m_client, CChromeDevToolsClient::ProtocolEvent, callback );
    m_started = true;

    // be told about tabs that get opened
    CORE::CDataNode params( GUCEF_DATATYPE_OBJECT );
    params.SetAttribute( "discover", true );
    CORE::CDataNode result;
    if ( !m_client.SendCommandAndWait( "Target.setDiscoverTargets", &params, result ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ChromeNetworkRecorder: The browser would not tell about new tabs" );
        Stop();
        return false;
    }

    // and watch the ones that are open right now
    TChromeTargetVector targets;
    if ( !m_client.TargetGetTargets( targets ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ChromeNetworkRecorder: The browser would not list its tabs" );
        Stop();
        return false;
    }
    for ( size_t i=0; i<targets.size(); ++i )
        if ( targets[ i ].type == "page" && UrlPassesFilter( targets[ i ].url ) )
            m_targetsToAttach.push_back( targets[ i ].id );
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

CORE::CString
CChromeNetworkRecorder::MakeKey( const CORE::CString& sessionId, const CORE::CString& requestId )
{GUCEF_TRACE;

    return sessionId + "|" + requestId;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CChromeNetworkRecorder::PageOf( const CORE::CString& url )
{GUCEF_TRACE;

    CUrlPathTemplate parsed;
    if ( !CUrlPathTemplate::Parse( url, parsed ) )
        return CORE::CString::Empty;
    return parsed.scheme + "://" + parsed.host + parsed.pathTemplate;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CChromeNetworkRecorder::TemplateFileName( const CORE::CString& fileName )
{GUCEF_TRACE;

    const char* t = fileName.C_String();
    const UInt32 len = fileName.IsNULLOrEmpty() ? 0 : fileName.ByteSize() - 1;
    CORE::CString result;
    UInt32 i = 0;
    while ( i < len )
    {
        if ( t[ i ] >= '0' && t[ i ] <= '9' )
        {
            while ( i < len && t[ i ] >= '0' && t[ i ] <= '9' )
                ++i;
            result += "{n}";
        }
        else
        {
            result += t[ i ];
            ++i;
        }
    }
    return result;
}

/*-------------------------------------------------------------------------*/

void
CChromeNetworkRecorder::ParseContentDisposition( const CORE::CString& headerValue, CORE::CString& dispositionType, CORE::CString& fileNameTemplate )
{GUCEF_TRACE;

    dispositionType.Clear();
    fileNameTemplate.Clear();
    CORE::CString::StringVector parts = headerValue.ParseElements( ';', false );
    for ( size_t i=0; i<parts.size(); ++i )
    {
        const CORE::CString part = parts[ i ].Trim( true ).Trim( false );
        if ( 0 == i )
        {
            dispositionType = part.Lowercase();
            continue;
        }
        const Int32 eq = part.HasChar( '=' );
        if ( eq <= 0 )
            continue;
        const CORE::CString name = part.SubstrToIndex( (UInt32) eq, true ).Trim( false ).Lowercase();
        CORE::CString value = part.CutChars( (UInt32) eq + 1, true ).Trim( true );
        if ( name == "filename*" )
        {
            // charset'language'percent-encoded
            const Int32 quote = value.HasChar( '\'', false );
            if ( quote >= 0 )
                value = CUrlPathTemplate::PercentDecode( value.CutChars( (UInt32) quote + 1, true ) );
        }
        else
        if ( name != "filename" )
            continue;
        value = value.ReplaceSubstr( "\"", "" );
        // only the name, never a path
        const Int32 slash = value.HasChar( '/', false );
        if ( slash >= 0 )
            value = value.CutChars( (UInt32) slash + 1, true );
        const Int32 backslash = value.HasChar( '\\', false );
        if ( backslash >= 0 )
            value = value.CutChars( (UInt32) backslash + 1, true );
        if ( !value.IsNULLOrEmpty() )
            fileNameTemplate = TemplateFileName( value );
    }
}

/*-------------------------------------------------------------------------*/

void
CChromeNetworkRecorder::OnProtocolEvent( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( GUCEF_NULL == eventData )
        return;
    const SChromeProtocolEvent& event = static_cast< TChromeProtocolEventData* >( eventData )->GetData();
    if ( !event.hasParams )
        return;

    const CORE::CString& method = event.method;

    // network, page and runtime events only of the sessions set up here: another user of the client (a response tap,
    // a page driver) may have sessions of its own with the network events on, which would record requests twice
    if ( !event.sessionId.IsNULLOrEmpty() && m_sessions.find( event.sessionId ) == m_sessions.end() &&
         ( 0 == strncmp( method.C_String(), "Network.", 8 ) || 0 == strncmp( method.C_String(), "Page.", 5 ) ||
           0 == strncmp( method.C_String(), "Runtime.", 8 ) ) )
        return;

    if ( method == "Network.requestWillBeSent" )
        HandleRequestWillBeSent( event );
    else
    if ( method == "Network.responseReceived" )
        HandleResponseReceived( event );
    else
    if ( method == "Network.requestWillBeSentExtraInfo" )
        HandleRequestWillBeSentExtraInfo( event );
    else
    if ( method == "Network.responseReceivedExtraInfo" )
        HandleResponseReceivedExtraInfo( event );
    else
    if ( method == "Network.loadingFinished" )
        HandleLoadingFinished( event );
    else
    if ( method == "Network.loadingFailed" )
        HandleLoadingFailed( event );
    else
    if ( method == "Page.downloadWillBegin" || method == "Browser.downloadWillBegin" )
        HandleDownloadWillBegin( event );
    else
    if ( method == "Page.navigatedWithinDocument" )
        HandleNavigatedWithinDocument( event );
    else
    if ( method == "Runtime.bindingCalled" )
        HandleBindingCalled( event );
    else
    if ( method == "Target.attachedToTarget" )
        HandleAttachedToTarget( event );
    else
    if ( method == "Target.detachedFromTarget" )
        HandleDetachedFromTarget( event );
    else
    if ( method == "Target.targetCreated" )
        HandleTargetCreated( event );
    else
    if ( method == "Target.targetInfoChanged" )
        HandleTargetInfoChanged( event );
}

/*-------------------------------------------------------------------------*/

bool
CChromeNetworkRecorder::UrlPassesFilter( const CORE::CString& url ) const
{GUCEF_TRACE;

    // a tab is only watched when it shows the site: the other tabs of the user are not ours to look at
    CUrlPathTemplate parsed;
    return CUrlPathTemplate::Parse( url, parsed ) && CSiteDiscoveryRedactor::MatchesHostFilter( m_redactor->GetSettings().hostFilter, parsed.host );
}

/*-------------------------------------------------------------------------*/

void
CChromeNetworkRecorder::EmitAction( const CObservedUserAction& action )
{GUCEF_TRACE;

    for ( size_t i=0; i<m_sinks.size(); ++i )
        m_sinks[ i ]->OnUserActionObserved( action );
}

/*-------------------------------------------------------------------------*/

void
CChromeNetworkRecorder::StartAction( const CORE::CString& kind, const CORE::CString& label, const CORE::CString& page )
{GUCEF_TRACE;

    EmitAction( m_tracker.StartAction( NowInMs(), kind, label, page ) );
}

/*-------------------------------------------------------------------------*/

void
CChromeNetworkRecorder::DescribeRequestBody( CObservedWebRequest& observation ) const
{GUCEF_TRACE;

    observation.requestBodyShape.Clear();
    if ( observation.postData.IsNULLOrEmpty() )
        return;

    const CSiteDiscoverySettings& settings = m_redactor->GetSettings();
    const CORE::CString& type = observation.requestContentType;
    const char first = observation.postData.C_String()[ 0 ];
    if ( Contains( type, "json" ) || ( type.IsNULLOrEmpty() && ( first == '{' || first == '[' ) ) )
    {
        CORE::CDataNode document;
        if ( COM::CJsonRpcEndpoint::ParseJsonText( observation.postData, document ) )
        {
            CWebBodyShape::Describe( document, observation.requestBodyShape, settings.maxShapeDepth, settings.maxShapeLines,
                                     settings.recordExampleValues, &m_redactor->GetNameRedactor() );
            return;
        }
    }
    if ( Contains( type, "multipart/form-data" ) )
    {
        CWebBodyShape::DescribeMultipart( observation.postData, observation.requestBodyShape, settings.maxShapeLines, &m_redactor->GetNameRedactor() );
        observation.requestBodyShape.lines.insert( observation.requestBodyShape.lines.begin(),
                                                   "(body): multipart/form-data, " + CORE::UInt32ToString( observation.postData.ByteSize() - 1 ) + " bytes" );
        return;
    }
    if ( Contains( type, "x-www-form-urlencoded" ) )
    {
        CWebBodyShape::DescribeFormUrlEncoded( observation.postData, observation.requestBodyShape, settings.maxShapeLines,
                                               settings.recordExampleValues, &m_redactor->GetNameRedactor() );
        return;
    }
    observation.requestBodyShape.lines.push_back( "(body): " + ( type.IsNULLOrEmpty() ? CORE::CString( "unknown content type" ) : type ) +
                                                  ", " + CORE::UInt32ToString( observation.postData.ByteSize() - 1 ) + " bytes" );
}

/*-------------------------------------------------------------------------*/

void
CChromeNetworkRecorder::HandleRequestWillBeSent( const SChromeProtocolEvent& event )
{GUCEF_TRACE;

    const CORE::CString requestId = GetText( event.params, "requestId" );
    const CORE::CString key = MakeKey( event.sessionId, requestId );

    // a redirect reuses the request id: the previous leg of the request ended with the redirect response
    TPendingMap::iterator previous = m_inFlight.find( key );
    CORE::CString::StringVector chainHosts;
    if ( previous != m_inFlight.end() )
    {
        const CORE::CDataNode* redirect = event.params.FindChild( "redirectResponse" );
        CObservedWebRequest& left = previous->second.observation;
        if ( GUCEF_NULL != redirect )
        {
            left.status = (Int32) redirect->GetAttributeValue( "status" ).AsInt64( 0 );
            left.mimeType = GetText( *redirect, "mimeType" );
            if ( !left.neverRecorded )
            {
                // where it went: host and path template, never the query; the cookies it set by name
                const CORE::CDataNode* redirectHeaders = redirect->FindChild( "headers" );
                const CORE::CString location = FindHeader( redirectHeaders, "location" );
                CUrlPathTemplate from;
                CUrlPathTemplate::Parse( left.url, from );
                CORE::CString target = location;
                if ( !location.IsNULLOrEmpty() && location.C_String()[ 0 ] == '/' && !from.host.IsNULLOrEmpty() )
                    target = from.scheme + "://" + from.host + location;
                CUrlPathTemplate to;
                if ( CUrlPathTemplate::Parse( target, to ) )
                    left.redirectTarget = m_redactor->IsNeverRecord( to ) ? to.host : to.host + to.pathTemplate;
                const CORE::CString redirectCookies = FindHeader( redirectHeaders, "set-cookie" );
                if ( !redirectCookies.IsNULLOrEmpty() )
                {
                    m_redactor->RegisterSetCookie( redirectCookies );
                    NoteCookiesSet( left, redirectCookies );
                }
            }
        }
        chainHosts = left.redirectedFromHosts;
        CUrlPathTemplate leftParsed;
        if ( CUrlPathTemplate::Parse( left.url, leftParsed ) )
            chainHosts.push_back( leftParsed.host );
        m_completedWithoutBody.push_back( previous->second );
        m_inFlight.erase( previous );
    }

    const CORE::CDataNode* request = event.params.FindChild( "request" );
    if ( GUCEF_NULL == request )
        return;

    const CORE::CString url = GetText( *request, "url" );

    // what is in an address is public: a value seen there is not looked for as a secret
    m_redactor->NotePublicUrl( url );

    CUrlPathTemplate parsed;
    if ( !CUrlPathTemplate::Parse( url, parsed ) || !m_redactor->IsInScope( parsed ) )
        return;

    const CSiteDiscoverySettings& settings = m_redactor->GetSettings();
    const bool neverRecorded = m_redactor->IsNeverRecord( parsed );
    const CORE::CDataNode* headers = request->FindChild( "headers" );
    const CORE::CString contentType = BareContentType( FindHeader( headers, "content-type" ) );
    const CORE::CString postData = GetText( *request, "postData" );

    // whatever is secret in it is known from here on, also of requests that are not recorded
    m_redactor->RegisterQuerySecrets( url );
    m_redactor->RegisterBodySecrets( contentType, postData );

    SPending pending;
    pending.sessionId = event.sessionId;
    pending.requestId = requestId;
    CObservedWebRequest& observation = pending.observation;
    observation.url = url;
    observation.method = GetText( *request, "method" );
    observation.resourceType = GetText( event.params, "type" );
    observation.timestamp = event.params.GetAttributeValue( "timestamp" ).AsFloat64( 0.0 );
    observation.neverRecorded = neverRecorded;

    if ( GUCEF_NULL != headers )
    {
        for ( UInt32 i=0; i<headers->GetAttCount(); ++i )
        {
            const CORE::CDataNode::TKeyValuePair* att = headers->GetAttribute( i );
            if ( GUCEF_NULL == att )
                continue;
            const CORE::CString name = att->first.Lowercase();
            const CORE::CString value = m_redactor->CaptureHeaderValue( name, att->second.AsString() );
            if ( !neverRecorded )
            {
                observation.requestHeaders[ name ] = value;
                if ( name == "cookie" )
                {
                    CORE::CString::StringVector names;
                    CSiteDiscoveryRedactor::ParseCookieNames( att->second.AsString(), names );
                    for ( size_t n=0; n<names.size(); ++n )
                        observation.cookiesSent.push_back( m_redactor->RedactCookieName( names[ n ] ) );
                }
            }
        }
    }

    if ( neverRecorded )
    {
        if ( !settings.noteNeverRecorded )
            return;
    }
    else
    {
        observation.requestContentType = contentType;
        observation.postData = postData;
        pending.needsPostData = postData.IsNULLOrEmpty() && request->GetAttributeValue( "hasPostData" ).AsBool( false );
        DescribeRequestBody( observation );
    }

    // a document of a tab's main frame is a navigation: the frame id of the main frame is the id of the tab
    const CORE::CString frameId = GetText( event.params, "frameId" );
    TStringMap::const_iterator target = m_sessionTargets.find( event.sessionId );
    if ( observation.resourceType == "Document" && target != m_sessionTargets.end() && target->second == frameId )
    {
        m_sessionPages[ event.sessionId ] = PageOf( url );
        StartAction( CObservedUserAction::KindNavigation, CORE::CString::Empty, m_sessionPages[ event.sessionId ] );
    }

    bool startedPause = false;
    observation.actionId = m_tracker.AssignRequest( NowInMs(), startedPause );
    if ( startedPause )
    {
        CObservedUserAction action = m_tracker.GetLastAction();
        TStringMap::const_iterator page = m_sessionPages.find( event.sessionId );
        if ( page != m_sessionPages.end() )
            action.page = page->second;
        EmitAction( action );
    }

    observation.redirectedFromHosts = chainHosts;
    TExtraInfoMap::iterator extra = m_extraInfo.find( key );
    if ( extra != m_extraInfo.end() )
    {
        ApplyExtraInfo( observation, extra->second );
        m_extraInfo.erase( extra );
    }
    m_inFlight[ key ] = pending;
}

/*-------------------------------------------------------------------------*/

void
CChromeNetworkRecorder::HandleResponseReceived( const SChromeProtocolEvent& event )
{GUCEF_TRACE;

    const CORE::CDataNode* response = event.params.FindChild( "response" );
    const CORE::CDataNode* headers = GUCEF_NULL != response ? response->FindChild( "headers" ) : GUCEF_NULL;
    const CORE::CString setCookie = FindHeader( headers, "set-cookie" );
    if ( !setCookie.IsNULLOrEmpty() )
        m_redactor->RegisterSetCookie( setCookie );

    TPendingMap::iterator i = m_inFlight.find( MakeKey( event.sessionId, GetText( event.params, "requestId" ) ) );
    if ( i == m_inFlight.end() )
        return;

    CObservedWebRequest& observation = i->second.observation;
    if ( !setCookie.IsNULLOrEmpty() && !observation.neverRecorded )
        NoteCookiesSet( observation, setCookie );
    if ( GUCEF_NULL != response )
    {
        observation.status = (Int32) response->GetAttributeValue( "status" ).AsInt64( 0 );
        observation.mimeType = GetText( *response, "mimeType" );
        if ( !observation.neverRecorded )
        {
            const CORE::CString disposition = FindHeader( headers, "content-disposition" );
            if ( !disposition.IsNULLOrEmpty() )
            {
                CORE::CString type;
                CORE::CString fileName;
                ParseContentDisposition( disposition, type, fileName );
                observation.contentDisposition = type;
                if ( !fileName.IsNULLOrEmpty() )
                    observation.downloadFileName = fileName;
                if ( type == "attachment" )
                    observation.isDownload = true;
            }
        }
    }
    const CORE::CString type = GetText( event.params, "type" );
    if ( !type.IsNULLOrEmpty() )
        observation.resourceType = type;
}

/*-------------------------------------------------------------------------*/

CChromeNetworkRecorder::SPending*
CChromeNetworkRecorder::FindPending( const CORE::CString& key )
{GUCEF_TRACE;

    TPendingMap::iterator i = m_inFlight.find( key );
    if ( i != m_inFlight.end() )
        return &i->second;
    for ( size_t n=0; n<m_finished.size(); ++n )
        if ( MakeKey( m_finished[ n ].sessionId, m_finished[ n ].requestId ) == key )
            return &m_finished[ n ];
    for ( size_t n=0; n<m_completedWithoutBody.size(); ++n )
        if ( MakeKey( m_completedWithoutBody[ n ].sessionId, m_completedWithoutBody[ n ].requestId ) == key )
            return &m_completedWithoutBody[ n ];
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

static void
AddNames( CORE::CString::StringVector& list, const CORE::CString::StringVector& names )
{
    for ( size_t i=0; i<names.size(); ++i )
        if ( std::find( list.begin(), list.end(), names[ i ] ) == list.end() )
            list.push_back( names[ i ] );
}

/*-------------------------------------------------------------------------*/

void
CChromeNetworkRecorder::ApplyExtraInfo( CObservedWebRequest& observation, const SExtraInfo& extra ) const
{GUCEF_TRACE;

    if ( observation.neverRecorded )
        return;
    AddNames( observation.cookiesSent, extra.cookiesSent );
    AddNames( observation.cookiesSet, extra.cookiesSet );
    for ( TStringMap::const_iterator h = extra.headerNames.begin(); h != extra.headerNames.end(); ++h )
        if ( observation.requestHeaders.find( h->first ) == observation.requestHeaders.end() )
            observation.requestHeaders[ h->first ] = h->second;
}

/*-------------------------------------------------------------------------*/

void
CChromeNetworkRecorder::NoteCookiesSet( CObservedWebRequest& observation, const CORE::CString& setCookieHeader ) const
{GUCEF_TRACE;

    CORE::CString::StringVector names;
    CSiteDiscoveryRedactor::ParseSetCookieNames( setCookieHeader, names );
    CORE::CString::StringVector redacted;
    for ( size_t n=0; n<names.size(); ++n )
        redacted.push_back( m_redactor->RedactCookieName( names[ n ] ) );
    AddNames( observation.cookiesSet, redacted );
}

/*-------------------------------------------------------------------------*/

void
CChromeNetworkRecorder::HandleRequestWillBeSentExtraInfo( const SChromeProtocolEvent& event )
{GUCEF_TRACE;

    // the headers the browser really sent, among them the cookies: values are secret and registered, names are kept
    const CORE::CString key = MakeKey( event.sessionId, GetText( event.params, "requestId" ) );
    const CORE::CDataNode* headers = event.params.FindChild( "headers" );
    if ( GUCEF_NULL == headers )
        return;
    SExtraInfo extra;
    for ( UInt32 i=0; i<headers->GetAttCount(); ++i )
    {
        const CORE::CDataNode::TKeyValuePair* att = headers->GetAttribute( i );
        if ( GUCEF_NULL == att )
            continue;
        const CORE::CString name = att->first.Lowercase();
        if ( name.IsNULLOrEmpty() || name.C_String()[ 0 ] == ':' )
            continue;       // pseudo headers of HTTP/2 repeat what the URL says
        extra.headerNames[ name ] = m_redactor->CaptureHeaderValue( name, att->second.AsString() );
        if ( name == "cookie" )
        {
            CORE::CString::StringVector names;
            CSiteDiscoveryRedactor::ParseCookieNames( att->second.AsString(), names );
            for ( size_t n=0; n<names.size(); ++n )
                extra.cookiesSent.push_back( m_redactor->RedactCookieName( names[ n ] ) );
        }
    }

    SPending* pending = FindPending( key );
    if ( GUCEF_NULL != pending )
    {
        ApplyExtraInfo( pending->observation, extra );
        return;
    }
    if ( m_extraInfo.size() >= 512 )
        m_extraInfo.clear();
    SExtraInfo& stored = m_extraInfo[ key ];
    AddNames( stored.cookiesSent, extra.cookiesSent );
    for ( TStringMap::const_iterator h = extra.headerNames.begin(); h != extra.headerNames.end(); ++h )
        stored.headerNames[ h->first ] = h->second;
}

/*-------------------------------------------------------------------------*/

void
CChromeNetworkRecorder::HandleResponseReceivedExtraInfo( const SChromeProtocolEvent& event )
{GUCEF_TRACE;

    const CORE::CString key = MakeKey( event.sessionId, GetText( event.params, "requestId" ) );
    const CORE::CDataNode* headers = event.params.FindChild( "headers" );
    const CORE::CString setCookie = FindHeader( headers, "set-cookie" );
    if ( setCookie.IsNULLOrEmpty() )
        return;
    m_redactor->RegisterSetCookie( setCookie );

    CORE::CString::StringVector names;
    CSiteDiscoveryRedactor::ParseSetCookieNames( setCookie, names );
    SExtraInfo extra;
    for ( size_t n=0; n<names.size(); ++n )
        extra.cookiesSet.push_back( m_redactor->RedactCookieName( names[ n ] ) );

    SPending* pending = FindPending( key );
    if ( GUCEF_NULL != pending )
    {
        ApplyExtraInfo( pending->observation, extra );
        return;
    }
    if ( m_extraInfo.size() >= 512 )
        m_extraInfo.clear();
    AddNames( m_extraInfo[ key ].cookiesSet, extra.cookiesSet );
}

/*-------------------------------------------------------------------------*/

void
CChromeNetworkRecorder::HandleDownloadWillBegin( const SChromeProtocolEvent& event )
{GUCEF_TRACE;

    // the browser saves the response as a file: mark the request of that address that is the latest
    const CORE::CString url = GetText( event.params, "url" );
    const CORE::CString fileName = TemplateFileName( GetText( event.params, "suggestedFilename" ) );
    SPending* found = GUCEF_NULL;
    for ( TPendingMap::iterator i = m_inFlight.begin(); i != m_inFlight.end(); ++i )
        if ( i->second.observation.url == url && ( GUCEF_NULL == found || i->second.observation.timestamp >= found->observation.timestamp ) )
            found = &i->second;
    for ( size_t i=0; GUCEF_NULL == found && i<m_finished.size(); ++i )
        if ( m_finished[ m_finished.size() - 1 - i ].observation.url == url )
            found = &m_finished[ m_finished.size() - 1 - i ];
    if ( GUCEF_NULL == found || found->observation.neverRecorded )
        return;
    found->observation.isDownload = true;
    if ( found->observation.contentDisposition.IsNULLOrEmpty() )
        found->observation.contentDisposition = "attachment";
    if ( found->observation.downloadFileName.IsNULLOrEmpty() )
        found->observation.downloadFileName = fileName;
}

/*-------------------------------------------------------------------------*/

void
CChromeNetworkRecorder::HandleLoadingFinished( const SChromeProtocolEvent& event )
{GUCEF_TRACE;

    TPendingMap::iterator i = m_inFlight.find( MakeKey( event.sessionId, GetText( event.params, "requestId" ) ) );
    if ( i == m_inFlight.end() )
        return;

    i->second.observation.encodedDataLength = (UInt64) event.params.GetAttributeValue( "encodedDataLength" ).AsFloat64( 0.0 );
    m_finished.push_back( i->second );
    m_inFlight.erase( i );
}

/*-------------------------------------------------------------------------*/

void
CChromeNetworkRecorder::HandleLoadingFailed( const SChromeProtocolEvent& event )
{GUCEF_TRACE;

    TPendingMap::iterator i = m_inFlight.find( MakeKey( event.sessionId, GetText( event.params, "requestId" ) ) );
    if ( i == m_inFlight.end() )
        return;

    i->second.observation.failed = true;
    i->second.observation.errorText = GetText( event.params, "errorText" );
    m_completedWithoutBody.push_back( i->second );
    m_inFlight.erase( i );
}

/*-------------------------------------------------------------------------*/

void
CChromeNetworkRecorder::HandleNavigatedWithinDocument( const SChromeProtocolEvent& event )
{GUCEF_TRACE;

    TStringMap::const_iterator target = m_sessionTargets.find( event.sessionId );
    if ( target == m_sessionTargets.end() || target->second != GetText( event.params, "frameId" ) )
        return;
    const CORE::CString url = GetText( event.params, "url" );
    if ( !UrlPassesFilter( url ) )
        return;
    m_sessionPages[ event.sessionId ] = PageOf( url );
    StartAction( CObservedUserAction::KindInPageNavigation, CORE::CString::Empty, m_sessionPages[ event.sessionId ] );
}

/*-------------------------------------------------------------------------*/

void
CChromeNetworkRecorder::HandleBindingCalled( const SChromeProtocolEvent& event )
{GUCEF_TRACE;

    if ( GetText( event.params, "name" ) != ActionBindingName || !m_redactor->GetSettings().captureClicks )
        return;
    CORE::CDataNode payload;
    if ( !COM::CJsonRpcEndpoint::ParseJsonText( GetText( event.params, "payload" ), payload ) )
        return;
    const CORE::CString kind = GetText( payload, "kind" ) == "submit" ? CObservedUserAction::KindSubmit : CObservedUserAction::KindClick;
    const bool inTable = payload.GetAttributeValue( "inTable" ).AsBool( false );
    const bool withhold = inTable && !m_redactor->GetSettings().reportTableCellControlNames;
    // the label as the listener gave it: the value rules apply when a report is made (CSiteEndpointCatalog)
    CORE::CString label = withhold ? CORE::CString::Empty : CleanLabel( GetText( payload, "label" ) );
    if ( label.IsNULLOrEmpty() )
    {
        // only the kind of control: its role, else its tag, both checked to be plain words
        CORE::CString what = GetText( payload, "role" ).Lowercase();
        if ( what.IsNULLOrEmpty() )
            what = GetText( payload, "tag" ).Lowercase();
        const UInt32 whatBytes = what.IsNULLOrEmpty() ? 0 : what.ByteSize() - 1;
        bool plain = whatBytes > 0 && whatBytes <= 24;
        for ( UInt32 i=0; plain && i<whatBytes; ++i )
        {
            const char c = what.C_String()[ i ];
            plain = ( c >= 'a' && c <= 'z' ) || ( c >= '0' && c <= '9' ) || c == '-';
        }
        if ( plain )
            label = "<" + what + ( inTable ? CORE::CString( " in table" ) : CORE::CString::Empty ) + ">";
    }
    StartAction( kind, label, FindPageOfSession( event.sessionId ) );
}

/*-------------------------------------------------------------------------*/

void
CChromeNetworkRecorder::HandleAttachedToTarget( const SChromeProtocolEvent& event )
{GUCEF_TRACE;

    // A target we attached to ourselves announces itself as well, and so does one that someone else attached to:
    // watching those twice would show every request twice. Only the frames and workers that come with the
    // tabs we watch are new.
    const CORE::CString sessionId = GetText( event.params, "sessionId" );
    const CORE::CDataNode* info = event.params.FindChild( "targetInfo" );
    const CORE::CString targetId = GUCEF_NULL != info ? GetText( *info, "targetId" ) : CORE::CString::Empty;
    const CORE::CString type = GUCEF_NULL != info ? GetText( *info, "type" ) : CORE::CString::Empty;
    if ( sessionId.IsNULLOrEmpty() || m_sessions.find( sessionId ) != m_sessions.end() )
        return;
    if ( !targetId.IsNULLOrEmpty() && !m_attachedTargets.insert( targetId ).second )
        return;
    if ( !targetId.IsNULLOrEmpty() && type == "page" )
        m_sessionTargets[ sessionId ] = targetId;       // the main frame of a tab has the id of the tab
    if ( !event.sessionId.IsNULLOrEmpty() )
        m_sessionParents[ sessionId ] = event.sessionId;  // a frame or worker of a tab we watch: its actions are on the tab's page
    m_sessionsToSetup[ sessionId ] = type;
}

/*-------------------------------------------------------------------------*/

void
CChromeNetworkRecorder::HandleDetachedFromTarget( const SChromeProtocolEvent& event )
{GUCEF_TRACE;

    const CORE::CString sessionId = GetText( event.params, "sessionId" );
    m_sessions.erase( sessionId );
    m_sessionPages.erase( sessionId );
    m_sessionParents.erase( sessionId );
    TStringMap::iterator target = m_sessionTargets.find( sessionId );
    if ( target != m_sessionTargets.end() )
    {
        m_attachedTargets.erase( target->second );
        m_sessionTargets.erase( target );
    }
}

/*-------------------------------------------------------------------------*/

void
CChromeNetworkRecorder::HandleTargetCreated( const SChromeProtocolEvent& event )
{GUCEF_TRACE;

    const CORE::CDataNode* info = event.params.FindChild( "targetInfo" );
    if ( GUCEF_NULL != info && GetText( *info, "type" ) == "page" && UrlPassesFilter( GetText( *info, "url" ) ) )
        m_targetsToAttach.push_back( GetText( *info, "targetId" ) );
}

/*-------------------------------------------------------------------------*/

void
CChromeNetworkRecorder::HandleTargetInfoChanged( const SChromeProtocolEvent& event )
{GUCEF_TRACE;

    // a tab that was somewhere else and went to the site
    const CORE::CDataNode* info = event.params.FindChild( "targetInfo" );
    if ( GUCEF_NULL == info || GetText( *info, "type" ) != "page" || !UrlPassesFilter( GetText( *info, "url" ) ) )
        return;
    const CORE::CString targetId = GetText( *info, "targetId" );
    if ( m_attachedTargets.find( targetId ) == m_attachedTargets.end() )
        m_targetsToAttach.push_back( targetId );
}

/*-------------------------------------------------------------------------*/

bool
CChromeNetworkRecorder::SendOnSession( const CORE::CString& sessionId, const CORE::CString& method, const CORE::CDataNode* params, CORE::CDataNode* result )
{GUCEF_TRACE;

    m_client.SetSessionId( sessionId );
    CORE::CDataNode localResult;
    const bool ok = m_client.SendCommandAndWait( method, params, GUCEF_NULL != result ? *result : localResult, GUCEF_NULL, 15000 );
    m_client.SetSessionId( CORE::CString::Empty );
    return ok;
}

/*-------------------------------------------------------------------------*/

bool
CChromeNetworkRecorder::SetupSession( const CORE::CString& sessionId, bool isPage )
{GUCEF_TRACE;

    if ( !m_sessions.insert( sessionId ).second )
        return true;

    const CSiteDiscoverySettings& settings = m_redactor->GetSettings();

    // room for the browser to keep responses around until we ask for them
    CORE::CDataNode enable( GUCEF_DATATYPE_OBJECT );
    enable.SetAttribute( "maxTotalBufferSize", (UInt32) ( 200 * 1024 * 1024 ) );
    enable.SetAttribute( "maxResourceBufferSize", (UInt32) ( 50 * 1024 * 1024 ) );
    bool ok = SendOnSession( sessionId, "Network.enable", &enable );

    if ( settings.disableBrowserCache )
    {
        // a response that comes from the cache is a response we may not see being requested
        CORE::CDataNode cache( GUCEF_DATATYPE_OBJECT );
        cache.SetAttribute( "cacheDisabled", true );
        ok = SendOnSession( sessionId, "Network.setCacheDisabled", &cache ) && ok;
    }

    // frames and workers of the page get their own session
    CORE::CDataNode autoAttach( GUCEF_DATATYPE_OBJECT );
    autoAttach.SetAttribute( "autoAttach", true );
    autoAttach.SetAttribute( "waitForDebuggerOnStart", false );
    autoAttach.SetAttribute( "flatten", true );
    ok = SendOnSession( sessionId, "Target.setAutoAttach", &autoAttach ) && ok;

    if ( isPage )
    {
        // navigations within a single page application and downloads; a worker has no pages, that is no failure
        SendOnSession( sessionId, "Page.enable", GUCEF_NULL );

        if ( settings.captureClicks && !InstallClickListener( sessionId ) )
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "ChromeNetworkRecorder: The click listener could not be installed in session " << sessionId );
    }

    if ( !ok )
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "ChromeNetworkRecorder: Not everything could be set up to watch the network of session " << sessionId );
    return ok;
}

/*-------------------------------------------------------------------------*/

bool
CChromeNetworkRecorder::InstallClickListener( const CORE::CString& sessionId )
{GUCEF_TRACE;

    // Without Runtime.enable a binding only reaches the execution contexts that exist right now: the document the
    // user navigates to next would have the listener but not the binding, and no click would ever be reported
    bool ok = SendOnSession( sessionId, "Runtime.enable", GUCEF_NULL );

    // an isolated world of our own: page scripts can neither see nor replace the binding and the listener
    CORE::CDataNode binding( GUCEF_DATATYPE_OBJECT );
    binding.SetAttribute( "name", CORE::CVariant( ActionBindingName ) );
    binding.SetAttribute( "executionContextName", CORE::CVariant( ClickListenerWorldName ) );
    ok = SendOnSession( sessionId, "Runtime.addBinding", &binding ) && ok;

    // in every later document, and right now in the documents the target already shows
    CORE::CDataNode script( GUCEF_DATATYPE_OBJECT );
    script.SetAttribute( "source", CORE::CVariant( GetClickListenerScript() ) );
    script.SetAttribute( "worldName", CORE::CVariant( ClickListenerWorldName ) );
    script.SetAttribute( "runImmediately", true );
    if ( SendOnSession( sessionId, "Page.addScriptToEvaluateOnNewDocument", &script ) )
        return ok;

    // a browser that does not know runImmediately: the main world, as before
    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "ChromeNetworkRecorder: The browser refused the isolated listener, installing it in the page's own world" );
    CORE::CDataNode mainBinding( GUCEF_DATATYPE_OBJECT );
    mainBinding.SetAttribute( "name", CORE::CVariant( ActionBindingName ) );
    ok = SendOnSession( sessionId, "Runtime.addBinding", &mainBinding ) && ok;
    CORE::CDataNode mainScript( GUCEF_DATATYPE_OBJECT );
    mainScript.SetAttribute( "source", CORE::CVariant( GetClickListenerScript() ) );
    ok = SendOnSession( sessionId, "Page.addScriptToEvaluateOnNewDocument", &mainScript ) && ok;
    CORE::CDataNode evaluate( GUCEF_DATATYPE_OBJECT );
    evaluate.SetAttribute( "expression", CORE::CVariant( GetClickListenerScript() ) );
    return SendOnSession( sessionId, "Runtime.evaluate", &evaluate ) && ok;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CChromeNetworkRecorder::FindPageOfSession( const CORE::CString& sessionId ) const
{GUCEF_TRACE;

    // a frame's session leads up to the session of its tab
    CORE::CString session = sessionId;
    for ( UInt32 depth=0; depth<16; ++depth )
    {
        TStringMap::const_iterator page = m_sessionPages.find( session );
        if ( page != m_sessionPages.end() )
            return page->second;
        TStringMap::const_iterator parent = m_sessionParents.find( session );
        if ( parent == m_sessionParents.end() )
            break;
        session = parent->second;
    }
    return CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

bool
CChromeNetworkRecorder::SaveBody( CObservedWebRequest& observation, const CORE::CString& body )
{GUCEF_TRACE;

    const CSiteDiscoverySettings& settings = m_redactor->GetSettings();
    const CORE::CString dir = CORE::CombinePath( m_bodyDirectory, settings.bodiesDirectoryName );
    if ( !m_bodyDirectoryReady )
    {
        if ( !CORE::DirExists( dir ) && !CORE::CreateDirs( dir ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ChromeNetworkRecorder: Unable to create the folder of saved responses " << dir );
            return false;
        }
        CORE::SetOwnerOnlyAccess( dir );
        m_bodyDirectoryReady = true;
    }

    const CORE::CString mime = observation.mimeType.Lowercase();
    const char* ext = Contains( mime, "json" ) ? "json" : ( Contains( mime, "xml" ) ? "xml" : "txt" );
    char name[ 64 ];
    snprintf( name, sizeof( name ), "%06u.%s", (unsigned int) observation.sequence, ext );
    const CORE::CString path = CORE::CombinePath( dir, name );

    // a file of an earlier journal that was removed is replaced, never written through
    if ( CORE::FileExists( path ) )
        CORE::DeleteFile( path );
    CORE::CString problem;
    if ( !CORE::CreateOwnerOnlyFile( path, body.C_String(), body.IsNULLOrEmpty() ? 0 : body.ByteSize() - 1, problem ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ChromeNetworkRecorder: Unable to save a response: " << problem );
        return false;
    }
    observation.bodyFile = settings.bodiesDirectoryName + "/" + name;
    ++m_savedBodyCount;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CChromeNetworkRecorder::Complete( SPending& pending, bool fetchBody )
{GUCEF_TRACE;

    const CSiteDiscoverySettings& settings = m_redactor->GetSettings();
    CObservedWebRequest& observation = pending.observation;
    observation.sequence = m_nextSequence++;

    if ( pending.needsPostData && !observation.neverRecorded )
    {
        // a large body is not in the event, the browser has it
        CORE::CDataNode params( GUCEF_DATATYPE_OBJECT );
        params.SetAttribute( "requestId", CORE::CVariant( pending.requestId ) );
        CORE::CDataNode result;
        if ( SendOnSession( pending.sessionId, "Network.getRequestPostData", &params, &result ) )
        {
            observation.postData = GetText( result, "postData" );
            m_redactor->RegisterBodySecrets( observation.requestContentType, observation.postData );
            DescribeRequestBody( observation );
        }
    }

    const bool sizeOk = 0 == settings.maxResponseBodyBytes || observation.encodedDataLength <= settings.maxResponseBodyBytes;
    if ( fetchBody && !observation.neverRecorded && !observation.isDownload && sizeOk && observation.status >= 200 && observation.status <= 299 &&
         CSiteEndpointCatalog::Classify( observation ) == CSiteEndpointCatalog::KIND_DATA )
    {
        const CORE::CString mime = observation.mimeType.Lowercase();
        bool wantShape = false;
        CORE::CString probeKey;
        if ( settings.recordResponseShapes && Contains( mime, "json" ) )
        {
            for ( size_t i=0; i<m_sinks.size() && !wantShape; ++i )
                wantShape = m_sinks[ i ]->WantsResponseShape( observation );

            // an endpoint that keeps answering with what is already known is not fetched for ever
            CUrlPathTemplate parsedUrl;
            if ( wantShape && CUrlPathTemplate::Parse( observation.url, parsedUrl ) )
            {
                probeKey = observation.method + " " + parsedUrl.GetKey();
                const UInt32 maxProbes = settings.maxShapesPerEndpoint * ShapeProbesPerShape;
                if ( 0 != settings.maxShapesPerEndpoint && m_shapeProbes[ probeKey ] >= maxProbes )
                    wantShape = false;
            }
        }
        bool wantSave = false;
        if ( settings.saveResponseBodies && !m_bodyDirectory.IsNULLOrEmpty() )
            for ( size_t i=0; i<settings.responseBodyContentTypes.size() && !wantSave; ++i )
                wantSave = Contains( mime, settings.responseBodyContentTypes[ i ].Lowercase().C_String() );

        if ( wantShape || wantSave )
        {
            CORE::CString body;
            bool isBase64 = false;
            m_client.SetSessionId( pending.sessionId );
            const bool gotBody = m_client.GetResponseBody( pending.requestId, body, isBase64 );
            m_client.SetSessionId( CORE::CString::Empty );
            const bool bodySizeOk = 0 == settings.maxResponseBodyBytes || body.IsNULLOrEmpty() || (UInt64) ( body.ByteSize() - 1 ) <= settings.maxResponseBodyBytes;
            if ( gotBody && !isBase64 && bodySizeOk )
            {
                CORE::CDataNode document;
                const bool parsed = Contains( mime, "json" ) && COM::CJsonRpcEndpoint::ParseJsonText( body, document );
                if ( parsed )
                {
                    // a token a response hands out is a secret from here on
                    m_redactor->RegisterBodySecrets( mime, body, CSiteDiscoveryRedactor::SECRET_CLASS_RESPONSE_BODY_FIELD );
                }
                if ( wantSave )
                    SaveBody( observation, body );
                if ( wantShape && parsed )
                {
                    if ( !probeKey.IsNULLOrEmpty() )
                        ++m_shapeProbes[ probeKey ];
                    CWebBodyShape described;
                    CWebBodyShape::Describe( document, described, settings.maxShapeDepth, settings.maxShapeLines,
                                             settings.recordExampleValues, &m_redactor->GetNameRedactor() );

                    // the shape goes with the request when it adds something to what a sink that wants shapes knows of the endpoint
                    for ( size_t i=0; i<m_sinks.size() && !observation.hasResponseShape; ++i )
                        if ( m_sinks[ i ]->WantsResponseShape( observation ) && m_sinks[ i ]->IsNewResponseShape( observation, described ) )
                            observation.hasResponseShape = true;
                    if ( observation.hasResponseShape )
                        observation.responseShape = described;
                }
            }
            else
                GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "ChromeNetworkRecorder: The browser no longer has the body of " << CORE::CRedactor::ForUrl().RedactUrl( observation.url ) );
        }
    }

    ++m_recordedCount;
    for ( size_t i=0; i<m_sinks.size(); ++i )
        m_sinks[ i ]->OnWebRequestObserved( observation );
}

/*-------------------------------------------------------------------------*/

UInt32
CChromeNetworkRecorder::Process( void )
{GUCEF_TRACE;

    UInt32 done = 0;

    // new tabs. The queue is taken first: commands pump the event loop and more tabs may be announced meanwhile
    CORE::CString::StringVector targets;
    targets.swap( m_targetsToAttach );
    for ( size_t i=0; i<targets.size(); ++i )
    {
        if ( !m_attachedTargets.insert( targets[ i ] ).second )
            continue;
        CORE::CString sessionId;
        if ( m_client.AttachToTarget( targets[ i ], sessionId ) )
        {
            m_sessionTargets[ sessionId ] = targets[ i ];
            m_sessionsToSetup.erase( sessionId );
            SetupSession( sessionId, true );
            ++done;
        }
        else
        {
            m_attachedTargets.erase( targets[ i ] );
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "ChromeNetworkRecorder: Unable to attach to tab " << targets[ i ] );
        }
    }

    TStringMap sessions;
    sessions.swap( m_sessionsToSetup );
    for ( TStringMap::const_iterator i = sessions.begin(); i != sessions.end(); ++i )
    {
        SetupSession( i->first, i->second == "page" || i->second == "iframe" );
        ++done;
    }

    TPendingVector withoutBody;
    withoutBody.swap( m_completedWithoutBody );
    for ( size_t i=0; i<withoutBody.size(); ++i )
    {
        Complete( withoutBody[ i ], false );
        ++done;
    }

    TPendingVector finished;
    finished.swap( m_finished );
    for ( size_t i=0; i<finished.size(); ++i )
    {
        Complete( finished[ i ], true );
        ++done;
    }
    return done;
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

/*-------------------------------------------------------------------------*/

bool
CChromeNetworkRecorder::IsMainFrameOf( const CORE::CString& sessionId, const CORE::CString& frameId ) const
{GUCEF_TRACE;

    TStringMap::const_iterator target = m_sessionTargets.find( sessionId );
    return target != m_sessionTargets.end() && !frameId.IsNULLOrEmpty() && target->second == frameId;
}

/*-------------------------------------------------------------------------*/

UInt32
CChromeNetworkRecorder::GetActionCount( void ) const
{GUCEF_TRACE;

    return m_tracker.GetActionCount();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
