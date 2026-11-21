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

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_COBSERVER_H
#include "CObserver.h"
#define GUCEF_CORE_COBSERVER_H
#endif /* GUCEF_CORE_COBSERVER_H ? */

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

#include "gucefWEB_CDevToolsPage.h"

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

const CORE::CString CDevToolsPage::AriaSelectorPrefix = "aria/";
const UInt32 CDevToolsPage::MaxControls = 1000;

/** The isolated world the page driver's few scripts run in */
static const char* const PageDriverWorldName = "__gucefPageDriver";

/** Chooses an option of a select element by label, else by value, and tells the page as a change would */
static const char* const SelectOptionFunction =
    "function(option){if(!this||this.tagName!=='SELECT')return 'not-a-select';"
    "var norm=function(s){return String(s==null?'':s).replace(/\\s+/g,' ').trim();};var want=norm(option);var o=this.options;var hit=-1;"
    "for(var i=0;i<o.length&&hit<0;i++){if(norm(o[i].label||o[i].text)===want)hit=i;}"
    "for(var j=0;j<o.length&&hit<0;j++){if(o[j].value===option)hit=j;}"
    "if(hit<0)return 'no-such-option';if(o[hit].disabled)return 'disabled';"
    "this.selectedIndex=hit;this.dispatchEvent(new Event('input',{bubbles:true}));this.dispatchEvent(new Event('change',{bubbles:true}));return 'ok';}";

/** Selects the content of a field, so that the text typed next replaces it */
static const char* const SelectContentFunction =
    "function(){if(typeof this.select==='function'){this.select();return 'ok';}"
    "if(this.isContentEditable){var r=document.createRange();r.selectNodeContents(this);var s=window.getSelection();s.removeAllRanges();s.addRange(r);return 'ok';}"
    "return 'not-a-field';}";

/** Roles of the accessibility tree that are controls, and the role ListControls() reports for them */
static const char* const ControlRoles[][ 2 ] = {
    { "button", "button" }, { "link", "link" }, { "tab", "tab" }, { "combobox", "combobox" }, { "listbox", "combobox" },
    { "menuitem", "menuitem" }, { "menuitemcheckbox", "menuitem" }, { "menuitemradio", "menuitem" },
    { "checkbox", "checkbox" }, { "switch", "checkbox" }, { "radio", "radio" }, { GUCEF_NULL, GUCEF_NULL } };

/** What counts as a table cell: the text of the cell is never reported, the names of the controls in it only when asked for (default) */
static const char* const TableCellSelector =
    "td,th,[role=cell],[role=gridcell],[role=rowheader],[role=columnheader],"
    "td *,th *,[role=cell] *,[role=gridcell] *,[role=rowheader] *,[role=columnheader] *";

/*-------------------------------------------------------------------------*/

static CORE::CString
GetText( const CORE::CDataNode& node, const char* name )
{
    return node.GetAttributeValue( name ).AsString( CORE::CString::Empty, false );
}

/*-------------------------------------------------------------------------*/

static UInt32
ByteLength( const CORE::CString& text )
{
    return text.IsNULLOrEmpty() ? 0 : text.ByteSize() - 1;
}

/*-------------------------------------------------------------------------*/

/** Whitespace collapsed, trimmed, at most maxChars characters */
static CORE::CString
CleanLabel( const CORE::CString& text, UInt32 maxChars )
{
    CORE::CString result;
    bool space = false;
    const char* t = text.C_String();
    const UInt32 len = ByteLength( text );
    for ( UInt32 i=0; i<len; ++i )
    {
        const char c = t[ i ];
        if ( c == ' ' || c == '\t' || c == '\r' || c == '\n' || c == '\f' || c == '\v' )
        {
            space = !result.IsNULLOrEmpty();
            continue;
        }
        if ( space )
            result += ' ';
        space = false;
        result += c;
    }
    if ( result.Length() > maxChars )
        result = result.SubstrToIndex( maxChars, true ).Trim( false );
    return result;
}

/*-------------------------------------------------------------------------*/

/** The nested value of a protocol field such as "name": { "type": "computedString", "value": "Export" } */
static CORE::CString
GetValueOf( const CORE::CDataNode& node, const char* name )
{
    const CORE::CDataNode* child = node.FindChild( name );
    return GUCEF_NULL != child ? GetText( *child, "value" ) : CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

static void
GetInt64List( const CORE::CDataNode* list, GUCEF::vector< Int64 >& values )
{
    values.clear();
    if ( GUCEF_NULL == list )
        return;
    for ( CORE::CDataNode::const_iterator i = list->ConstBegin(); i != list->ConstEnd(); ++i )
        values.push_back( (*i)->GetValue().AsInt64( 0 ) );
}

/*-------------------------------------------------------------------------*/

static bool
Mentions( const CORE::CString& text, const char* part )
{
    return text.HasSubstr( part, true ) >= 0;
}

/*-------------------------------------------------------------------------*/

/** An answer of the browser that says the node is no longer there (removed, or its id is stale) */
static bool
IsNodeGoneError( const COM::SJsonRpcError& error )
{
    const CORE::CString message = error.message.Lowercase();
    return Mentions( message, "could not find node" ) || Mentions( message, "no node" ) || Mentions( message, "node not found" ) ||
           Mentions( message, "detached" ) || Mentions( message, "does not belong to the document" ) || Mentions( message, "no document" );
}

/*-------------------------------------------------------------------------*/

static UInt64
NowInMs( void )
{
    return (UInt64) ( MT::PrecisionTickCount() / ( MT::PrecisionTimerResolution() / 1000.0 ) );
}

/*-------------------------------------------------------------------------*/

namespace {

/**
 *  What the browser reports about the documents of one session while Navigate() waits, in the order it arrives.
 *  Subscribed for its own lifetime only, before the navigation is sent, so nothing that comes before the answer is missed
 */
class CNavigationWatch : public CORE::CObserver
{
    public:

    struct SEvent
    {
        CORE::CString method;
        CORE::CString frameId;
        CORE::CString parentId;
        CORE::CString loaderId;
        CORE::CString name;         /**< of Page.lifecycleEvent */
    };

    typedef GUCEF::vector< SEvent > TEventVector;

    TEventVector events;
    bool sessionEnded;

    CNavigationWatch( CChromeDevToolsClient& client, const CORE::CString& sessionId )
        : CORE::CObserver()
        , events()
        , sessionEnded( false )
        , m_sessionId( sessionId )
    {GUCEF_TRACE;

        TEventCallback callback( this, &CNavigationWatch::OnClientEvent );
        SubscribeTo( &client, CChromeDevToolsClient::ProtocolEvent, callback );
        SubscribeTo( &client, CChromeDevToolsClient::DisconnectedEvent, callback );
    }

    virtual ~CNavigationWatch()
    {GUCEF_TRACE;

        UnsubscribeAllFromObserver();
    }

    private:

    typedef CORE::CTEventHandlerFunctor< CNavigationWatch > TEventCallback;

    CNavigationWatch( const CNavigationWatch& src );
    CNavigationWatch& operator=( const CNavigationWatch& src );

    void OnClientEvent( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
    {GUCEF_TRACE;

        if ( eventId == CChromeDevToolsClient::DisconnectedEvent )
        {
            sessionEnded = true;
            return;
        }
        if ( GUCEF_NULL == eventData )
            return;
        const SChromeProtocolEvent& event = static_cast< TChromeProtocolEventData* >( eventData )->GetData();

        // the session itself can end: on the browser's own session, or as the last event of ours
        if ( event.method == "Target.detachedFromTarget" )
        {
            if ( GetText( event.params, "sessionId" ) == m_sessionId )
                sessionEnded = true;
            return;
        }
        if ( event.sessionId != m_sessionId )
            return;
        if ( event.method == "Inspector.detached" )
        {
            sessionEnded = true;
            return;
        }

        SEvent entry;
        entry.method = event.method;
        if ( event.method == "Page.frameNavigated" )
        {
            const CORE::CDataNode* frame = event.params.FindChild( "frame" );
            if ( GUCEF_NULL == frame )
                return;
            entry.frameId = GetText( *frame, "id" );
            entry.parentId = GetText( *frame, "parentId" );
            entry.loaderId = GetText( *frame, "loaderId" );
        }
        else
        if ( event.method == "Page.navigatedWithinDocument" || event.method == "Page.lifecycleEvent" )
        {
            entry.frameId = GetText( event.params, "frameId" );
            entry.loaderId = GetText( event.params, "loaderId" );
            entry.name = GetText( event.params, "name" );
        }
        else
        if ( event.method != "Page.loadEventFired" && event.method != "Page.domContentEventFired" )
            return;
        events.push_back( entry );
    }

    CORE::CString m_sessionId;
};

}; /* anonymous namespace */

/*-------------------------------------------------------------------------*/

CDevToolsPage::CDevToolsPage( CChromeDevToolsClient& client, const CORE::CString& sessionId )
    : m_client( client )
    , m_sessionId( sessionId )
    , m_guard( GUCEF_NULL )
    , m_commandTimeoutInMs( 30000 )
    , m_reportTableCellControlNames( true )
    , m_isolatedContextId( 0 )
    , m_navigationTimeoutInMs( 30000 )
    , m_pageEnabled( false )
    , m_lastNavigationLoaded( false )
    , m_lastNavigationError()
    , m_lastListSkippedCount( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDevToolsPage::~CDevToolsPage()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CDevToolsPage::AttachToTarget( const CORE::CString& targetId )
{GUCEF_TRACE;

    CORE::CDataNode params( GUCEF_DATATYPE_OBJECT );
    params.SetAttribute( "targetId", CORE::CVariant( targetId ) );
    params.SetAttribute( "flatten", true );
    CORE::CDataNode result;
    if ( !m_client.SendCommandOnSessionAndWait( CORE::CString::Empty, "Target.attachToTarget", &params, result, GUCEF_NULL, m_commandTimeoutInMs ) )
        return false;
    const CORE::CString sessionId = GetText( result, "sessionId" );
    if ( sessionId.IsNULLOrEmpty() )
        return false;
    SetSessionId( sessionId );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CDevToolsPage::SetSessionId( const CORE::CString& sessionId )
{GUCEF_TRACE;

    m_sessionId = sessionId;
    m_isolatedContextId = 0;
    m_pageEnabled = false;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CDevToolsPage::GetSessionId( void ) const
{GUCEF_TRACE;

    return m_sessionId;
}

/*-------------------------------------------------------------------------*/

void
CDevToolsPage::SetNavigationGuard( CNavigationGuard* guard )
{GUCEF_TRACE;

    m_guard = guard;
}

/*-------------------------------------------------------------------------*/

CNavigationGuard*
CDevToolsPage::GetNavigationGuard( void ) const
{GUCEF_TRACE;

    return m_guard;
}

/*-------------------------------------------------------------------------*/

void
CDevToolsPage::SetCommandTimeout( UInt32 timeoutInMs )
{GUCEF_TRACE;

    m_commandTimeoutInMs = timeoutInMs;
}

/*-------------------------------------------------------------------------*/

void
CDevToolsPage::SetReportTableCellControlNames( bool report )
{GUCEF_TRACE;

    m_reportTableCellControlNames = report;
}

/*-------------------------------------------------------------------------*/

bool
CDevToolsPage::GetReportTableCellControlNames( void ) const
{GUCEF_TRACE;

    return m_reportTableCellControlNames;
}

/*-------------------------------------------------------------------------*/

void
CDevToolsPage::SetNavigationTimeout( UInt32 timeoutInMs )
{GUCEF_TRACE;

    m_navigationTimeoutInMs = timeoutInMs;
}

/*-------------------------------------------------------------------------*/

UInt32
CDevToolsPage::GetNavigationTimeout( void ) const
{GUCEF_TRACE;

    return m_navigationTimeoutInMs;
}

/*-------------------------------------------------------------------------*/

bool
CDevToolsPage::GetLastNavigationLoaded( void ) const
{GUCEF_TRACE;

    return m_lastNavigationLoaded;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CDevToolsPage::GetLastNavigationError( void ) const
{GUCEF_TRACE;

    return m_lastNavigationError;
}

/*-------------------------------------------------------------------------*/

UInt32
CDevToolsPage::GetLastListSkippedCount( void ) const
{GUCEF_TRACE;

    return m_lastListSkippedCount;
}

/*-------------------------------------------------------------------------*/

bool
CDevToolsPage::Send( const CORE::CString& method, const CORE::CDataNode* params, CORE::CDataNode& result, bool logFailure, COM::SJsonRpcError* error )
{GUCEF_TRACE;

    COM::SJsonRpcError localError;
    COM::SJsonRpcError& failure = GUCEF_NULL != error ? *error : localError;
    failure.code = 0;
    failure.message.Clear();
    if ( m_client.SendCommandOnSessionAndWait( m_sessionId, method, params, result, &failure, m_commandTimeoutInMs ) )
        return true;
    if ( 0 == failure.code )
        failure.code = COM::JSONRPC_ERROR_INTERNAL;
    if ( logFailure )
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "DevToolsPage: " << method << " failed: " << failure.message );
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CDevToolsPage::IsSessionLost( const COM::SJsonRpcError& error ) const
{GUCEF_TRACE;

    // no answer at all, or the browser no longer knows the session (Chrome answers that with -32001 too)
    return !m_client.IsConnected() || error.code == COM::JSONRPC_ERROR_TRANSPORT_CLOSED || error.code == COM::JSONRPC_ERROR_CALL_TIMEOUT ||
           Mentions( error.message, "Session with given id not found" ) || Mentions( error.message, "Target closed" );
}

/*-------------------------------------------------------------------------*/

void
CDevToolsPage::SetNavigationError( const CORE::CString& url, const CORE::CString& reason )
{GUCEF_TRACE;

    m_lastNavigationError = reason;
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "DevToolsPage: The navigation to " << CNavigationGuard::MakeUrlTemplate( url ) << " failed: " << reason );
}

/*-------------------------------------------------------------------------*/

bool
CDevToolsPage::NavigateWithoutWait( const CORE::CString& url )
{GUCEF_TRACE;

    CORE::CDataNode params( GUCEF_DATATYPE_OBJECT );
    params.SetAttribute( "url", CORE::CVariant( url ) );
    CORE::CDataNode result;
    COM::SJsonRpcError error;
    if ( !Send( "Page.navigate", &params, result, true, &error ) )
    {
        SetNavigationError( url, "Page.navigate failed: " + error.message );
        return false;
    }
    m_isolatedContextId = 0;
    const CORE::CString errorText = GetText( result, "errorText" );
    if ( !errorText.IsNULLOrEmpty() )
    {
        SetNavigationError( url, errorText );
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDevToolsPage::Navigate( const CORE::CString& url )
{GUCEF_TRACE;

    m_lastNavigationLoaded = false;
    m_lastNavigationError.Clear();
    if ( GUCEF_NULL != m_guard && !m_guard->CheckNavigate( url ) )
    {
        m_lastNavigationError = "refused by the navigation guard";
        return false;
    }
    if ( 0 == m_navigationTimeoutInMs )
        return NavigateWithoutWait( url );

    // the Page domain reports the commit and the load of the documents of this session
    if ( !m_pageEnabled )
    {
        CORE::CDataNode enabled;
        COM::SJsonRpcError error;
        if ( !Send( "Page.enable", GUCEF_NULL, enabled, true, &error ) )
        {
            SetNavigationError( url, "Page.enable failed: " + error.message );
            return false;
        }
        m_pageEnabled = true;
    }

    CNavigationWatch watch( m_client, m_sessionId );
    const UInt64 deadline = NowInMs() + m_navigationTimeoutInMs;
    CORE::CDataNode params( GUCEF_DATATYPE_OBJECT );
    params.SetAttribute( "url", CORE::CVariant( url ) );
    CORE::CDataNode result;
    COM::SJsonRpcError error;
    if ( !Send( "Page.navigate", &params, result, true, &error ) )
    {
        SetNavigationError( url, "Page.navigate failed: " + error.message );
        return false;
    }
    m_isolatedContextId = 0;

    // the answer comes when the navigation started or failed, not when the new document is there
    const CORE::CString errorText = GetText( result, "errorText" );
    if ( !errorText.IsNULLOrEmpty() )
    {
        SetNavigationError( url, errorText );
        return false;
    }
    if ( result.GetAttributeValue( "isDownload" ).AsBool( false ) )
    {
        SetNavigationError( url, "the address is a download, the page stays where it was" );
        return false;
    }
    const CORE::CString frameId = GetText( result, "frameId" );
    const CORE::CString loaderId = GetText( result, "loaderId" );
    const bool sameDocument = loaderId.IsNULLOrEmpty();

    bool committed = false;
    bool loaded = false;
    bool domContentLoaded = false;
    size_t next = 0;
    CORE::PulseGeneratorPtr pulseGenerator = m_client.GetPulseGenerator();
    while ( true )
    {
        for ( ; next < watch.events.size(); ++next )
        {
            const CNavigationWatch::SEvent& event = watch.events[ next ];
            const bool ourFrame = frameId.IsNULLOrEmpty() || event.frameId == frameId;
            if ( !committed )
            {
                if ( sameDocument )
                    committed = event.method == "Page.navigatedWithinDocument" && ourFrame;
                else
                    committed = event.method == "Page.frameNavigated" && event.parentId.IsNULLOrEmpty() && ourFrame &&
                                ( event.loaderId.IsNULLOrEmpty() || event.loaderId == loaderId );
                continue;
            }
            // only what comes after the commit belongs to the new document
            if ( event.method == "Page.loadEventFired" )
                loaded = true;
            else
            if ( event.method == "Page.domContentEventFired" )
                domContentLoaded = true;
            else
            if ( event.method == "Page.lifecycleEvent" && ourFrame && ( event.loaderId.IsNULLOrEmpty() || event.loaderId == loaderId ) )
            {
                if ( event.name == "load" )
                    loaded = true;
                else
                if ( event.name == "DOMContentLoaded" )
                    domContentLoaded = true;
            }
        }

        if ( committed && ( loaded || sameDocument ) )
            break;
        if ( sameDocument && frameId.IsNULLOrEmpty() )
        {
            // an answer without a frame cannot be matched to a report of the browser: it accepted the navigation, take that
            committed = true;
            break;
        }
        if ( watch.sessionEnded || !m_client.IsConnected() )
        {
            SetNavigationError( url, "the connection or the session ended during the navigation" );
            return false;
        }
        if ( NowInMs() >= deadline || pulseGenerator.IsNULL() )
            break;
        pulseGenerator->ForceDirectPulse();
        MT::ThreadDelay( 1 );
    }

    if ( !committed )
    {
        if ( sameDocument )
            SetNavigationError( url, "the browser did not report the navigation within the document within " + CORE::UInt32ToString( m_navigationTimeoutInMs ) + " ms" );
        else
            SetNavigationError( url, "no new document was committed within " + CORE::UInt32ToString( m_navigationTimeoutInMs ) +
                                     " ms (a response without content, a download or a slow site)" );
        return false;
    }
    m_lastNavigationLoaded = loaded || domContentLoaded || sameDocument;
    if ( !loaded && !sameDocument )
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "DevToolsPage: The document of " << CNavigationGuard::MakeUrlTemplate( url ) << " is committed but did not load within " <<
                         m_navigationTimeoutInMs << " ms" << ( domContentLoaded ? " (its DOM is complete)" : "" ) );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDevToolsPage::GetUrl( CORE::CString& url )
{GUCEF_TRACE;

    CORE::CDataNode result;
    if ( !Send( "Page.getNavigationHistory", GUCEF_NULL, result ) )
        return false;
    const Int64 current = result.GetAttributeValue( "currentIndex" ).AsInt64( -1 );
    const CORE::CDataNode* entries = result.FindChild( "entries" );
    if ( GUCEF_NULL == entries || current < 0 )
        return false;
    Int64 index = 0;
    for ( CORE::CDataNode::const_iterator i = entries->ConstBegin(); i != entries->ConstEnd(); ++i, ++index )
    {
        if ( index == current )
        {
            url = GetText( *(*i), "url" );
            return true;
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CDevToolsPage::GetDocumentRoot( Int64& rootNodeId, COM::SJsonRpcError* error )
{GUCEF_TRACE;

    rootNodeId = 0;
    CORE::CDataNode params( GUCEF_DATATYPE_OBJECT );
    params.SetAttribute( "depth", (Int32) 0 );
    CORE::CDataNode result;
    if ( !Send( "DOM.getDocument", &params, result, true, error ) )
        return false;
    const CORE::CDataNode* root = result.FindChild( "root" );
    rootNodeId = GUCEF_NULL != root ? root->GetAttributeValue( "nodeId" ).AsInt64( 0 ) : 0;
    return 0 != rootNodeId;
}

/*-------------------------------------------------------------------------*/

bool
CDevToolsPage::Resolve( const CORE::CString& selector, Int64& nodeId, bool logMissing, COM::SJsonRpcError* error )
{GUCEF_TRACE;

    nodeId = 0;
    if ( GUCEF_NULL != error )
        error->code = 0;
    Int64 root = 0;
    if ( selector.IsNULLOrEmpty() || !GetDocumentRoot( root, error ) )
        return false;

    if ( !selector.StartsWith( AriaSelectorPrefix ) )
    {
        CORE::CDataNode params( GUCEF_DATATYPE_OBJECT );
        params.SetAttribute( "nodeId", CORE::CVariant( root ) );
        params.SetAttribute( "selector", CORE::CVariant( selector ) );
        CORE::CDataNode result;
        if ( !Send( "DOM.querySelector", &params, result, true, error ) )
            return false;
        nodeId = result.GetAttributeValue( "nodeId" ).AsInt64( 0 );
    }
    else
    {
        // the accessible name as the browser computes it; a control rather than the text inside it
        const CORE::CString name = selector.CutChars( AriaSelectorPrefix.Length(), true );
        CORE::CDataNode params( GUCEF_DATATYPE_OBJECT );
        params.SetAttribute( "nodeId", CORE::CVariant( root ) );
        params.SetAttribute( "accessibleName", CORE::CVariant( name ) );
        CORE::CDataNode result;
        if ( !Send( "Accessibility.queryAXTree", &params, result, true, error ) )
            return false;
        Int64 backendNodeId = 0;
        Int64 textNodeId = 0;
        const CORE::CDataNode* nodes = result.FindChild( "nodes" );
        if ( GUCEF_NULL != nodes )
        {
            for ( CORE::CDataNode::const_iterator i = nodes->ConstBegin(); i != nodes->ConstEnd() && 0 == backendNodeId; ++i )
            {
                const CORE::CDataNode& node = *(*i);
                const Int64 id = node.GetAttributeValue( "backendDOMNodeId" ).AsInt64( 0 );
                if ( 0 == id || node.GetAttributeValue( "ignored" ).AsBool( false ) )
                    continue;
                const CORE::CString role = GetValueOf( node, "role" );
                if ( role == "StaticText" || role == "InlineTextBox" || role == "none" || role == "generic" || role == "RootWebArea" )
                {
                    if ( 0 == textNodeId )
                        textNodeId = id;
                    continue;
                }
                backendNodeId = id;
            }
        }
        if ( 0 == backendNodeId )
            backendNodeId = textNodeId;
        if ( 0 != backendNodeId )
        {
            CORE::CDataNode push( GUCEF_DATATYPE_OBJECT );
            push.AddChild( "backendNodeIds", GUCEF_DATATYPE_ARRAY )->AddValueAsChild( CORE::CVariant( backendNodeId ) );
            CORE::CDataNode pushed;
            if ( !Send( "DOM.pushNodesByBackendIdsToFrontend", &push, pushed, true, error ) )
                return false;
            GUCEF::vector< Int64 > ids;
            GetInt64List( pushed.FindChild( "nodeIds" ), ids );
            nodeId = ids.empty() ? 0 : ids[ 0 ];
        }
    }
    if ( 0 == nodeId && logMissing )
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "DevToolsPage: No element matches " << selector );
    return 0 != nodeId;
}

/*-------------------------------------------------------------------------*/

bool
CDevToolsPage::DescribeNode( Int64 nodeId, CORE::CString& nodeName, CORE::CString::StringMap& attributes, COM::SJsonRpcError* error )
{GUCEF_TRACE;

    CORE::CDataNode params( GUCEF_DATATYPE_OBJECT );
    params.SetAttribute( "nodeId", CORE::CVariant( nodeId ) );
    CORE::CDataNode result;
    if ( !Send( "DOM.describeNode", &params, result, true, error ) )
        return false;
    const CORE::CDataNode* node = result.FindChild( "node" );
    if ( GUCEF_NULL == node )
        return false;
    nodeName = GetText( *node, "nodeName" ).Uppercase();
    attributes.clear();
    const CORE::CDataNode* list = node->FindChild( "attributes" );
    if ( GUCEF_NULL != list )
    {
        // name, value, name, value...
        CORE::CString name;
        bool isName = true;
        for ( CORE::CDataNode::const_iterator i = list->ConstBegin(); i != list->ConstEnd(); ++i )
        {
            const CORE::CString text = (*i)->GetValue().AsString( CORE::CString::Empty, false );
            if ( isName )
                name = text.Lowercase();
            else
                attributes[ name ] = text;
            isName = !isName;
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDevToolsPage::CallOnNode( Int64 nodeId, const CORE::CString& functionDeclaration, const CORE::CString* argument, CORE::CString& returned )
{GUCEF_TRACE;

    CORE::CDataNode params( GUCEF_DATATYPE_OBJECT );
    params.SetAttribute( "nodeId", CORE::CVariant( nodeId ) );
    params.SetAttribute( "objectGroup", CORE::CVariant( CORE::CString( PageDriverWorldName ) ) );
    CORE::CDataNode resolved;
    if ( !Send( "DOM.resolveNode", &params, resolved ) )
        return false;
    const CORE::CDataNode* object = resolved.FindChild( "object" );
    const CORE::CString objectId = GUCEF_NULL != object ? GetText( *object, "objectId" ) : CORE::CString::Empty;
    if ( objectId.IsNULLOrEmpty() )
        return false;

    CORE::CDataNode call( GUCEF_DATATYPE_OBJECT );
    call.SetAttribute( "objectId", CORE::CVariant( objectId ) );
    call.SetAttribute( "functionDeclaration", CORE::CVariant( functionDeclaration ) );
    call.SetAttribute( "returnByValue", true );
    if ( GUCEF_NULL != argument )
    {
        // the text is an argument of the function, never part of its source
        CORE::CDataNode* arguments = call.AddChild( "arguments", GUCEF_DATATYPE_ARRAY );
        CORE::CDataNode* first = arguments->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT );
        first->SetAttribute( "value", CORE::CVariant( *argument ) );
    }
    CORE::CDataNode result;
    const bool called = Send( "Runtime.callFunctionOn", &call, result );

    CORE::CDataNode release( GUCEF_DATATYPE_OBJECT );
    release.SetAttribute( "objectGroup", CORE::CVariant( CORE::CString( PageDriverWorldName ) ) );
    CORE::CDataNode released;
    Send( "Runtime.releaseObjectGroup", &release, released, false );

    if ( !called || GUCEF_NULL != result.FindChild( "exceptionDetails" ) )
        return false;
    const CORE::CDataNode* value = result.FindChild( "result" );
    returned = GUCEF_NULL != value ? GetText( *value, "value" ) : CORE::CString::Empty;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDevToolsPage::Click( const CORE::CString& selector )
{GUCEF_TRACE;

    Int64 nodeId = 0;
    if ( !Resolve( selector, nodeId ) )
        return false;
    CORE::CDataNode params( GUCEF_DATATYPE_OBJECT );
    params.SetAttribute( "nodeId", CORE::CVariant( nodeId ) );
    CORE::CDataNode result;
    Send( "DOM.scrollIntoViewIfNeeded", &params, result, false );
    CORE::CDataNode quadsResult;
    if ( !Send( "DOM.getContentQuads", &params, quadsResult ) )
        return false;

    // the centre of the first quad with an area
    const CORE::CDataNode* quads = quadsResult.FindChild( "quads" );
    Float64 x = 0.0;
    Float64 y = 0.0;
    bool found = false;
    if ( GUCEF_NULL != quads )
    {
        for ( CORE::CDataNode::const_iterator q = quads->ConstBegin(); q != quads->ConstEnd() && !found; ++q )
        {
            GUCEF::vector< Float64 > points;
            for ( CORE::CDataNode::const_iterator p = (*q)->ConstBegin(); p != (*q)->ConstEnd(); ++p )
                points.push_back( (*p)->GetValue().AsFloat64( 0.0 ) );
            if ( points.size() < 8 )
                continue;
            const Float64 width = points[ 2 ] - points[ 0 ];
            const Float64 height = points[ 5 ] - points[ 1 ];
            if ( width * width < 0.25 && height * height < 0.25 )
                continue;
            x = ( points[ 0 ] + points[ 2 ] + points[ 4 ] + points[ 6 ] ) / 4.0;
            y = ( points[ 1 ] + points[ 3 ] + points[ 5 ] + points[ 7 ] ) / 4.0;
            found = true;
        }
    }
    if ( !found )
    {
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "DevToolsPage: The element " << selector << " has no area to click" );
        return false;
    }

    static const char* const steps[] = { "mouseMoved", "mousePressed", "mouseReleased", GUCEF_NULL };
    for ( UInt32 i=0; GUCEF_NULL != steps[ i ]; ++i )
    {
        CORE::CDataNode mouse( GUCEF_DATATYPE_OBJECT );
        mouse.SetAttribute( "type", CORE::CVariant( CORE::CString( steps[ i ] ) ) );
        mouse.SetAttribute( "x", CORE::CVariant( x ) );
        mouse.SetAttribute( "y", CORE::CVariant( y ) );
        if ( 0 != i )
        {
            mouse.SetAttribute( "button", CORE::CVariant( CORE::CString( "left" ) ) );
            mouse.SetAttribute( "clickCount", (Int32) 1 );
        }
        if ( !Send( "Input.dispatchMouseEvent", &mouse, result ) )
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDevToolsPage::TypeInto( const CORE::CString& selector, const CORE::CString& text )
{GUCEF_TRACE;

    Int64 nodeId = 0;
    CORE::CString nodeName;
    CORE::CString::StringMap attributes;
    if ( !Resolve( selector, nodeId ) || !DescribeNode( nodeId, nodeName, attributes ) )
        return false;
    if ( nodeName == "INPUT" && attributes[ "type" ].Lowercase() == "password" )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "DevToolsPage: TypeInto() refuses password fields; secrets are typed by the sign-in browser" );
        return false;
    }
    CORE::CDataNode params( GUCEF_DATATYPE_OBJECT );
    params.SetAttribute( "nodeId", CORE::CVariant( nodeId ) );
    CORE::CDataNode result;
    if ( !Send( "DOM.focus", &params, result ) )
        return false;
    CORE::CString selected;
    if ( !CallOnNode( nodeId, SelectContentFunction, GUCEF_NULL, selected ) || selected != "ok" )
    {
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "DevToolsPage: The element " << selector << " is not a field" );
        return false;
    }
    // what is typed is not logged, also not on failure
    CORE::CDataNode insert( GUCEF_DATATYPE_OBJECT );
    insert.SetAttribute( "text", CORE::CVariant( text ) );
    COM::SJsonRpcError error;
    return m_client.SendCommandOnSessionAndWait( m_sessionId, "Input.insertText", &insert, result, &error, m_commandTimeoutInMs );
}

/*-------------------------------------------------------------------------*/

bool
CDevToolsPage::SelectOption( const CORE::CString& selector, const CORE::CString& option )
{GUCEF_TRACE;

    Int64 nodeId = 0;
    if ( !Resolve( selector, nodeId ) )
        return false;
    CORE::CString outcome;
    if ( !CallOnNode( nodeId, SelectOptionFunction, &option, outcome ) )
        return false;
    if ( outcome != "ok" )
    {
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "DevToolsPage: No option was chosen in " << selector << ": " << outcome );
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDevToolsPage::GetElementAttribute( const CORE::CString& selector, const CORE::CString& name, CORE::CString& value, UInt32 maxBytes )
{GUCEF_TRACE;

    value.Clear();
    Int64 nodeId = 0;
    CORE::CString nodeName;
    CORE::CString::StringMap attributes;
    if ( !Resolve( selector, nodeId ) || !DescribeNode( nodeId, nodeName, attributes ) )
        return false;
    const CORE::CString lowerName = name.Lowercase();
    if ( lowerName == "value" && ( nodeName == "INPUT" || nodeName == "TEXTAREA" ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "DevToolsPage: GetElementAttribute() does not read the values of fields" );
        return false;
    }
    CORE::CString::StringMap::const_iterator i = attributes.find( lowerName );
    if ( i == attributes.end() )
        return false;
    if ( ByteLength( i->second ) > maxBytes )
    {
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "DevToolsPage: The attribute " << name << " of " << selector << " is longer than " << maxBytes << " bytes" );
        return false;
    }
    value = i->second;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDevToolsPage::ElementExists( const CORE::CString& selector )
{GUCEF_TRACE;

    Int64 nodeId = 0;
    return Resolve( selector, nodeId, false );
}

/*-------------------------------------------------------------------------*/

bool
CDevToolsPage::GetIsolatedContext( Int64& contextId )
{GUCEF_TRACE;

    if ( 0 != m_isolatedContextId )
    {
        contextId = m_isolatedContextId;
        return true;
    }
    CORE::CDataNode tree;
    if ( !Send( "Page.getFrameTree", GUCEF_NULL, tree ) )
        return false;
    const CORE::CDataNode* frameTree = tree.FindChild( "frameTree" );
    const CORE::CDataNode* frame = GUCEF_NULL != frameTree ? frameTree->FindChild( "frame" ) : GUCEF_NULL;
    const CORE::CString frameId = GUCEF_NULL != frame ? GetText( *frame, "id" ) : CORE::CString::Empty;
    if ( frameId.IsNULLOrEmpty() )
        return false;
    CORE::CDataNode params( GUCEF_DATATYPE_OBJECT );
    params.SetAttribute( "frameId", CORE::CVariant( frameId ) );
    params.SetAttribute( "worldName", CORE::CVariant( CORE::CString( PageDriverWorldName ) ) );
    CORE::CDataNode result;
    if ( !Send( "Page.createIsolatedWorld", &params, result ) )
        return false;
    m_isolatedContextId = result.GetAttributeValue( "executionContextId" ).AsInt64( 0 );
    contextId = m_isolatedContextId;
    return 0 != contextId;
}

/*-------------------------------------------------------------------------*/

bool
CDevToolsPage::WaitForDomChange( UInt32 timeoutInMs )
{GUCEF_TRACE;

    Int64 contextId = 0;
    if ( !GetIsolatedContext( contextId ) )
        return false;
    // one promise in the page: settled 100 ms after the first change of the DOM, or when the time is up
    const CORE::CString expression =
        "new Promise(function(r){var o=new MutationObserver(function(){o.disconnect();setTimeout(function(){r(true);},100);});"
        "o.observe(document,{subtree:true,childList:true,attributes:true,characterData:true});"
        "setTimeout(function(){o.disconnect();r(false);}," + CORE::UInt32ToString( timeoutInMs ) + ");})";
    CORE::CDataNode params( GUCEF_DATATYPE_OBJECT );
    params.SetAttribute( "expression", CORE::CVariant( expression ) );
    params.SetAttribute( "contextId", CORE::CVariant( contextId ) );
    params.SetAttribute( "awaitPromise", true );
    params.SetAttribute( "returnByValue", true );
    CORE::CDataNode result;
    COM::SJsonRpcError error;
    if ( !m_client.SendCommandOnSessionAndWait( m_sessionId, "Runtime.evaluate", &params, result, &error, timeoutInMs + m_commandTimeoutInMs ) )
    {
        // the document went away (a navigation): a new world is needed for the next wait
        m_isolatedContextId = 0;
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDevToolsPage::WaitForElement( const CORE::CString& selector, UInt32 timeoutInMs )
{GUCEF_TRACE;

    const Float64 ticksPerMs = MT::PrecisionTimerResolution() / 1000.0;
    const UInt64 start = MT::PrecisionTickCount();
    UInt32 failedWaits = 0;
    while ( true )
    {
        Int64 nodeId = 0;
        if ( Resolve( selector, nodeId, false ) )
            return true;
        const UInt64 elapsed = (UInt64) ( ( MT::PrecisionTickCount() - start ) / ticksPerMs );
        if ( elapsed >= timeoutInMs )
            return false;
        const UInt32 left = (UInt32) ( timeoutInMs - elapsed );
        if ( WaitForDomChange( left < 30000 ? left : 30000 ) )
            failedWaits = 0;
        else
        if ( ++failedWaits > 1 )
            MT::ThreadDelay( 50 );      // no document to watch right now, between two pages
    }
}

/*-------------------------------------------------------------------------*/

bool
CDevToolsPage::ListControls( const CORE::CString& scopeSelector, TControlInfoVector& result )
{GUCEF_TRACE;

    m_lastListSkippedCount = 0;
    for ( UInt32 attempt=0; attempt<2; ++attempt )
    {
        UInt32 skipped = 0;
        const EListOutcome outcome = ListControlsOnce( scopeSelector, result, skipped );
        if ( LIST_DONE == outcome )
        {
            m_lastListSkippedCount = skipped;
            if ( 0 != skipped )
                GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "DevToolsPage: ListControls() left out " << skipped << " control(s) that went away while they were listed" );
            return true;
        }
        result.clear();
        if ( LIST_FAILED == outcome )
            return false;
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "DevToolsPage: The scope of ListControls() went away (the document changed)" <<
                         ( 0 == attempt ? ", listing once more on a fresh document" : " again, giving up" ) );
    }
    return false;
}

/*-------------------------------------------------------------------------*/

CDevToolsPage::EListOutcome
CDevToolsPage::ListControlsOnce( const CORE::CString& scopeSelector, TControlInfoVector& result, UInt32& skipped )
{GUCEF_TRACE;

    result.clear();
    skipped = 0;

    // the scope on a fresh document: node ids of an earlier DOM.getDocument may be stale
    COM::SJsonRpcError error;
    Int64 scope = 0;
    const bool resolved = scopeSelector.IsNULLOrEmpty() ? GetDocumentRoot( scope, &error ) : Resolve( scopeSelector, scope, true, &error );
    if ( !resolved )
    {
        if ( 0 == error.code )
            return LIST_FAILED;     // the browser answered: there is no such scope
        return IsSessionLost( error ) ? LIST_FAILED : LIST_STALE;
    }

    // the controls, by role, through the accessibility tree: names as the browser computes them
    typedef GUCEF::map< Int64, CControlInfo > TControlMap;
    TControlMap controls;
    for ( UInt32 r=0; GUCEF_NULL != ControlRoles[ r ][ 0 ]; ++r )
    {
        CORE::CDataNode params( GUCEF_DATATYPE_OBJECT );
        params.SetAttribute( "nodeId", CORE::CVariant( scope ) );
        params.SetAttribute( "role", CORE::CVariant( CORE::CString( ControlRoles[ r ][ 0 ] ) ) );
        CORE::CDataNode found;
        if ( !Send( "Accessibility.queryAXTree", &params, found, true, &error ) )
            return IsSessionLost( error ) ? LIST_FAILED : LIST_STALE;
        const CORE::CDataNode* nodes = found.FindChild( "nodes" );
        if ( GUCEF_NULL == nodes )
            continue;
        for ( CORE::CDataNode::const_iterator i = nodes->ConstBegin(); i != nodes->ConstEnd(); ++i )
        {
            const CORE::CDataNode& node = *(*i);
            const Int64 backendNodeId = node.GetAttributeValue( "backendDOMNodeId" ).AsInt64( 0 );
            if ( 0 == backendNodeId || controls.find( backendNodeId ) != controls.end() )
                continue;
            CControlInfo info;
            info.backendNodeId = backendNodeId;
            info.role = ControlRoles[ r ][ 1 ];
            info.label = CleanLabel( GetValueOf( node, "name" ), 80 );
            info.visible = !node.GetAttributeValue( "ignored" ).AsBool( false );
            info.enabled = true;
            const CORE::CDataNode* properties = node.FindChild( "properties" );
            if ( GUCEF_NULL != properties )
                for ( CORE::CDataNode::const_iterator p = properties->ConstBegin(); p != properties->ConstEnd(); ++p )
                    if ( GetText( *(*p), "name" ) == "disabled" && GetValueOf( *(*p), "value" ).Lowercase() == "true" )
                        info.enabled = false;
            controls[ backendNodeId ] = info;
            if ( controls.size() >= MaxControls )
                break;
        }
    }
    if ( controls.empty() )
        return LIST_DONE;

    // their element ids, in the order the ids were asked for; 0 for an element that is gone
    GUCEF::vector< Int64 > backendIds;
    for ( TControlMap::const_iterator i = controls.begin(); i != controls.end(); ++i )
        backendIds.push_back( i->first );
    CORE::CDataNode push( GUCEF_DATATYPE_OBJECT );
    CORE::CDataNode* list = push.AddChild( "backendNodeIds", GUCEF_DATATYPE_ARRAY );
    for ( size_t i=0; i<backendIds.size(); ++i )
        list->AddValueAsChild( CORE::CVariant( backendIds[ i ] ) );
    CORE::CDataNode pushed;
    GUCEF::vector< Int64 > nodeIds;
    const bool allPushed = Send( "DOM.pushNodesByBackendIdsToFrontend", &push, pushed, true, &error );
    if ( !allPushed && IsSessionLost( error ) )
        return LIST_FAILED;
    if ( allPushed )
        GetInt64List( pushed.FindChild( "nodeIds" ), nodeIds );
    if ( !allPushed || nodeIds.size() != backendIds.size() )
    {
        // one by one, so that an element that went away does not take the others with it
        nodeIds.clear();
        for ( size_t i=0; i<backendIds.size(); ++i )
        {
            CORE::CDataNode single( GUCEF_DATATYPE_OBJECT );
            single.AddChild( "backendNodeIds", GUCEF_DATATYPE_ARRAY )->AddValueAsChild( CORE::CVariant( backendIds[ i ] ) );
            CORE::CDataNode one;
            GUCEF::vector< Int64 > ids;
            if ( Send( "DOM.pushNodesByBackendIdsToFrontend", &single, one, false, &error ) )
                GetInt64List( one.FindChild( "nodeIds" ), ids );
            else
            if ( IsSessionLost( error ) )
                return LIST_FAILED;
            nodeIds.push_back( ids.size() == 1 ? ids[ 0 ] : 0 );
        }
    }

    // which of them are inside a table cell, and the order of the document
    CORE::CDataNode cellQuery( GUCEF_DATATYPE_OBJECT );
    cellQuery.SetAttribute( "nodeId", CORE::CVariant( scope ) );
    cellQuery.SetAttribute( "selector", CORE::CVariant( CORE::CString( TableCellSelector ) ) );
    CORE::CDataNode cells;
    if ( !Send( "DOM.querySelectorAll", &cellQuery, cells, true, &error ) )
        return IsSessionLost( error ) ? LIST_FAILED : LIST_STALE;
    GUCEF::vector< Int64 > cellIds;
    GetInt64List( cells.FindChild( "nodeIds" ), cellIds );
    GUCEF::set< Int64 > inCells( cellIds.begin(), cellIds.end() );

    CORE::CDataNode orderQuery( GUCEF_DATATYPE_OBJECT );
    orderQuery.SetAttribute( "nodeId", CORE::CVariant( scope ) );
    orderQuery.SetAttribute( "selector", CORE::CVariant( CORE::CString( "*" ) ) );
    CORE::CDataNode all;
    if ( !Send( "DOM.querySelectorAll", &orderQuery, all, true, &error ) )
        return IsSessionLost( error ) ? LIST_FAILED : LIST_STALE;
    GUCEF::vector< Int64 > allIds;
    GetInt64List( all.FindChild( "nodeIds" ), allIds );
    GUCEF::map< Int64, UInt32 > position;
    for ( size_t i=0; i<allIds.size(); ++i )
        position[ allIds[ i ] ] = (UInt32) i + 1;      // the scope itself, which is not in the list, comes first

    typedef GUCEF::map< UInt32, CControlInfo > TOrderedMap;
    TOrderedMap ordered;
    UInt32 unknown = (UInt32) allIds.size() + 1;
    for ( size_t i=0; i<backendIds.size(); ++i )
    {
        CControlInfo info = controls[ backendIds[ i ] ];
        const Int64 nodeId = nodeIds[ i ];
        if ( 0 == nodeId )
        {
            ++skipped;      // gone since the accessibility tree reported it
            continue;
        }
        info.inTableCell = inCells.find( nodeId ) != inCells.end();

        if ( info.role == CControlInfo::RoleCombobox )
        {
            CORE::CString nodeName;
            CORE::CString::StringMap attributes;
            if ( !DescribeNode( nodeId, nodeName, attributes, &error ) )
            {
                if ( IsSessionLost( error ) )
                    return LIST_FAILED;
                ++skipped;
                continue;
            }
            if ( nodeName == "SELECT" )
            {
                info.role = CControlInfo::RoleSelect;
                if ( !info.inTableCell || m_reportTableCellControlNames )
                {
                    CORE::CDataNode params( GUCEF_DATATYPE_OBJECT );
                    params.SetAttribute( "backendNodeId", CORE::CVariant( backendIds[ i ] ) );
                    params.SetAttribute( "role", CORE::CVariant( CORE::CString( "option" ) ) );
                    CORE::CDataNode options;
                    if ( !Send( "Accessibility.queryAXTree", &params, options, true, &error ) )
                    {
                        if ( IsSessionLost( error ) )
                            return LIST_FAILED;
                        ++skipped;
                        continue;
                    }
                    const CORE::CDataNode* nodes = options.FindChild( "nodes" );
                    if ( GUCEF_NULL != nodes )
                        for ( CORE::CDataNode::const_iterator o = nodes->ConstBegin(); o != nodes->ConstEnd(); ++o )
                            info.optionLabels.push_back( CleanLabel( GetValueOf( *(*o), "name" ), 80 ) );
                }
            }
        }
        if ( info.inTableCell && !m_reportTableCellControlNames )
        {
            info.label.Clear();
            info.optionLabels.clear();
        }

        // a box of some size: rendered, not collapsed. An element without layout has no quads, which is no failure
        if ( info.visible )
        {
            CORE::CDataNode params( GUCEF_DATATYPE_OBJECT );
            params.SetAttribute( "backendNodeId", CORE::CVariant( backendIds[ i ] ) );
            CORE::CDataNode quadsResult;
            bool hasArea = false;
            if ( Send( "DOM.getContentQuads", &params, quadsResult, false, &error ) )
            {
                const CORE::CDataNode* quads = quadsResult.FindChild( "quads" );
                if ( GUCEF_NULL != quads )
                {
                    for ( CORE::CDataNode::const_iterator q = quads->ConstBegin(); q != quads->ConstEnd() && !hasArea; ++q )
                    {
                        GUCEF::vector< Float64 > points;
                        for ( CORE::CDataNode::const_iterator p = (*q)->ConstBegin(); p != (*q)->ConstEnd(); ++p )
                            points.push_back( (*p)->GetValue().AsFloat64( 0.0 ) );
                        hasArea = points.size() >= 8 && ( points[ 2 ] - points[ 0 ] ) * ( points[ 5 ] - points[ 1 ] ) > 0.0;
                    }
                }
            }
            else
            {
                if ( IsSessionLost( error ) )
                    return LIST_FAILED;
                if ( IsNodeGoneError( error ) )
                {
                    ++skipped;
                    continue;
                }
            }
            info.visible = hasArea;
        }

        GUCEF::map< Int64, UInt32 >::const_iterator at = position.find( nodeId );
        ordered[ nodeId == scope ? 0 : ( at != position.end() ? at->second : unknown++ ) ] = info;
    }
    for ( TOrderedMap::const_iterator i = ordered.begin(); i != ordered.end() && result.size() < MaxControls; ++i )
        result.push_back( i->second );
    return LIST_DONE;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
