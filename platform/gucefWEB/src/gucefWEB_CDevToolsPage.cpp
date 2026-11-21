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

/** What counts as a table cell: nothing of its text is reported */
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

CDevToolsPage::CDevToolsPage( CChromeDevToolsClient& client, const CORE::CString& sessionId )
    : m_client( client )
    , m_sessionId( sessionId )
    , m_guard( GUCEF_NULL )
    , m_commandTimeoutInMs( 30000 )
    , m_isolatedContextId( 0 )
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

bool
CDevToolsPage::Send( const CORE::CString& method, const CORE::CDataNode* params, CORE::CDataNode& result, bool logFailure )
{GUCEF_TRACE;

    COM::SJsonRpcError error;
    if ( m_client.SendCommandOnSessionAndWait( m_sessionId, method, params, result, &error, m_commandTimeoutInMs ) )
        return true;
    if ( logFailure )
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "DevToolsPage: " << method << " failed: " << error.message );
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CDevToolsPage::Navigate( const CORE::CString& url )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_guard && !m_guard->CheckNavigate( url ) )
        return false;
    CORE::CDataNode params( GUCEF_DATATYPE_OBJECT );
    params.SetAttribute( "url", CORE::CVariant( url ) );
    CORE::CDataNode result;
    if ( !Send( "Page.navigate", &params, result ) )
        return false;
    m_isolatedContextId = 0;
    const CORE::CString errorText = GetText( result, "errorText" );
    if ( !errorText.IsNULLOrEmpty() )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "DevToolsPage: The navigation to " << CNavigationGuard::MakeUrlTemplate( url ) << " ended with " << errorText );
        return false;
    }
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
CDevToolsPage::GetDocumentRoot( Int64& rootNodeId )
{GUCEF_TRACE;

    CORE::CDataNode params( GUCEF_DATATYPE_OBJECT );
    params.SetAttribute( "depth", (Int32) 0 );
    CORE::CDataNode result;
    if ( !Send( "DOM.getDocument", &params, result ) )
        return false;
    const CORE::CDataNode* root = result.FindChild( "root" );
    rootNodeId = GUCEF_NULL != root ? root->GetAttributeValue( "nodeId" ).AsInt64( 0 ) : 0;
    return 0 != rootNodeId;
}

/*-------------------------------------------------------------------------*/

bool
CDevToolsPage::Resolve( const CORE::CString& selector, Int64& nodeId, bool logMissing )
{GUCEF_TRACE;

    nodeId = 0;
    Int64 root = 0;
    if ( selector.IsNULLOrEmpty() || !GetDocumentRoot( root ) )
        return false;

    if ( !selector.StartsWith( AriaSelectorPrefix ) )
    {
        CORE::CDataNode params( GUCEF_DATATYPE_OBJECT );
        params.SetAttribute( "nodeId", CORE::CVariant( root ) );
        params.SetAttribute( "selector", CORE::CVariant( selector ) );
        CORE::CDataNode result;
        if ( !Send( "DOM.querySelector", &params, result ) )
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
        if ( !Send( "Accessibility.queryAXTree", &params, result ) )
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
            if ( !Send( "DOM.pushNodesByBackendIdsToFrontend", &push, pushed ) )
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
CDevToolsPage::DescribeNode( Int64 nodeId, CORE::CString& nodeName, CORE::CString::StringMap& attributes )
{GUCEF_TRACE;

    CORE::CDataNode params( GUCEF_DATATYPE_OBJECT );
    params.SetAttribute( "nodeId", CORE::CVariant( nodeId ) );
    CORE::CDataNode result;
    if ( !Send( "DOM.describeNode", &params, result ) )
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

    result.clear();
    Int64 scope = 0;
    if ( scopeSelector.IsNULLOrEmpty() )
    {
        if ( !GetDocumentRoot( scope ) )
            return false;
    }
    else
    if ( !Resolve( scopeSelector, scope ) )
        return false;

    // the controls, by role, through the accessibility tree: names as the browser computes them
    typedef GUCEF::map< Int64, CControlInfo > TControlMap;
    TControlMap controls;
    for ( UInt32 r=0; GUCEF_NULL != ControlRoles[ r ][ 0 ]; ++r )
    {
        CORE::CDataNode params( GUCEF_DATATYPE_OBJECT );
        params.SetAttribute( "nodeId", CORE::CVariant( scope ) );
        params.SetAttribute( "role", CORE::CVariant( CORE::CString( ControlRoles[ r ][ 0 ] ) ) );
        CORE::CDataNode found;
        if ( !Send( "Accessibility.queryAXTree", &params, found ) )
            return false;
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
        return true;

    // their element ids, in the order the ids were asked for
    GUCEF::vector< Int64 > backendIds;
    for ( TControlMap::const_iterator i = controls.begin(); i != controls.end(); ++i )
        backendIds.push_back( i->first );
    CORE::CDataNode push( GUCEF_DATATYPE_OBJECT );
    CORE::CDataNode* list = push.AddChild( "backendNodeIds", GUCEF_DATATYPE_ARRAY );
    for ( size_t i=0; i<backendIds.size(); ++i )
        list->AddValueAsChild( CORE::CVariant( backendIds[ i ] ) );
    CORE::CDataNode pushed;
    if ( !Send( "DOM.pushNodesByBackendIdsToFrontend", &push, pushed ) )
        return false;
    GUCEF::vector< Int64 > nodeIds;
    GetInt64List( pushed.FindChild( "nodeIds" ), nodeIds );
    if ( nodeIds.size() != backendIds.size() )
        return false;

    // which of them are inside a table cell, and the order of the document
    CORE::CDataNode cellQuery( GUCEF_DATATYPE_OBJECT );
    cellQuery.SetAttribute( "nodeId", CORE::CVariant( scope ) );
    cellQuery.SetAttribute( "selector", CORE::CVariant( CORE::CString( TableCellSelector ) ) );
    CORE::CDataNode cells;
    if ( !Send( "DOM.querySelectorAll", &cellQuery, cells ) )
        return false;
    GUCEF::vector< Int64 > cellIds;
    GetInt64List( cells.FindChild( "nodeIds" ), cellIds );
    GUCEF::set< Int64 > inCells( cellIds.begin(), cellIds.end() );

    CORE::CDataNode orderQuery( GUCEF_DATATYPE_OBJECT );
    orderQuery.SetAttribute( "nodeId", CORE::CVariant( scope ) );
    orderQuery.SetAttribute( "selector", CORE::CVariant( CORE::CString( "*" ) ) );
    CORE::CDataNode all;
    if ( !Send( "DOM.querySelectorAll", &orderQuery, all ) )
        return false;
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
        info.inTableCell = inCells.find( nodeId ) != inCells.end();

        CORE::CString nodeName;
        CORE::CString::StringMap attributes;
        if ( info.role == CControlInfo::RoleCombobox && DescribeNode( nodeId, nodeName, attributes ) && nodeName == "SELECT" )
        {
            info.role = CControlInfo::RoleSelect;
            if ( !info.inTableCell )
            {
                CORE::CDataNode params( GUCEF_DATATYPE_OBJECT );
                params.SetAttribute( "backendNodeId", CORE::CVariant( backendIds[ i ] ) );
                params.SetAttribute( "role", CORE::CVariant( CORE::CString( "option" ) ) );
                CORE::CDataNode options;
                if ( Send( "Accessibility.queryAXTree", &params, options ) )
                {
                    const CORE::CDataNode* nodes = options.FindChild( "nodes" );
                    if ( GUCEF_NULL != nodes )
                        for ( CORE::CDataNode::const_iterator o = nodes->ConstBegin(); o != nodes->ConstEnd(); ++o )
                            info.optionLabels.push_back( CleanLabel( GetValueOf( *(*o), "name" ), 80 ) );
                }
            }
        }
        if ( info.inTableCell )
        {
            info.label.Clear();
            info.optionLabels.clear();
        }

        // a box of some size: rendered, not collapsed
        if ( info.visible )
        {
            CORE::CDataNode params( GUCEF_DATATYPE_OBJECT );
            params.SetAttribute( "backendNodeId", CORE::CVariant( backendIds[ i ] ) );
            CORE::CDataNode quadsResult;
            bool hasArea = false;
            if ( Send( "DOM.getContentQuads", &params, quadsResult, false ) )
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
            info.visible = hasArea;
        }

        GUCEF::map< Int64, UInt32 >::const_iterator at = position.find( nodeId );
        ordered[ nodeId == scope ? 0 : ( at != position.end() ? at->second : unknown++ ) ] = info;
    }
    for ( TOrderedMap::const_iterator i = ordered.begin(); i != ordered.end() && result.size() < MaxControls; ++i )
        result.push_back( i->second );
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
