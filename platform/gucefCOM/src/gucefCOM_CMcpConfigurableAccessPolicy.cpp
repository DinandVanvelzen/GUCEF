/*
 *  gucefCOM: GUCEF module providing communication implementations
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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "gucefCOM_CMcpConfigurableAccessPolicy.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COM {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CMcpConfigurableAccessPolicy::ConfigSectionName = "McpAccessPolicy";

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** The separator of a list that is given as one text; the patterns themselves can hold ',' in {a,b} */
const char ListSeparator = ';';

/**
 *  In a pattern for URIs a single '*' matches '/' too: a URI is no path. "**" stays as it is and an escaped
 *  character is copied.
 */
CORE::CString
UriPatternFromText( const CORE::CString& pattern )
{
    CORE::CString result;
    const UInt32 length = pattern.Length();
    const char* text = pattern.C_String();
    for ( UInt32 i=0; i<length; ++i )
    {
        const char c = text[ i ];
        if ( '\\' == c && i+1 < length )
        {
            result.Append( text + i, 2 );
            ++i;
        }
        else
        if ( '*' == c )
        {
            if ( i+1 < length && '*' == text[ i+1 ] )
            {
                result.Append( "**", 2 );
                ++i;
            }
            else
            {
                result.Append( "**", 2 );
            }
        }
        else
        {
            result.Append( &c, 1 );
        }
    }
    return result;
}

/*-------------------------------------------------------------------------*/

bool
LoadPatterns( const CORE::CDataNode& node, const CORE::CString& key, bool isUri, CMcpPatternList& list, bool& found )
{
    CORE::CString::StringVector elements;
    found = node.GetStringListByName( key, elements, ListSeparator );
    list.Clear();
    for ( size_t i=0; i<elements.size(); ++i )
    {
        if ( !list.Add( elements[ i ], isUri ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "McpConfigurableAccessPolicy: The pattern \"" << elements[ i ] << "\" of " << key << " is not a valid glob pattern" );
            return false;
        }
    }
    return true;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CMcpPatternList::CMcpPatternList( void )
    : m_sources()
    , m_patterns()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CMcpPatternList::Add( const CORE::CString& pattern, bool isUri )
{GUCEF_TRACE;

    CORE::CGlobPattern compiled;
    // names and URIs are case sensitive
    if ( !compiled.SetPattern( isUri ? UriPatternFromText( pattern ) : pattern, true ) )
        return false;
    m_sources.push_back( pattern );
    m_patterns.push_back( compiled );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CMcpPatternList::Clear( void )
{GUCEF_TRACE;

    m_sources.clear();
    m_patterns.clear();
}

/*-------------------------------------------------------------------------*/

bool
CMcpPatternList::IsEmpty( void ) const
{GUCEF_TRACE;

    return m_patterns.empty();
}

/*-------------------------------------------------------------------------*/

bool
CMcpPatternList::Matches( const CORE::CString& text ) const
{GUCEF_TRACE;

    for ( size_t i=0; i<m_patterns.size(); ++i )
        if ( m_patterns[ i ].IsMatch( text ) )
            return true;
    return false;
}

/*-------------------------------------------------------------------------*/

const CORE::CString::StringVector&
CMcpPatternList::GetPatterns( void ) const
{GUCEF_TRACE;

    return m_sources;
}

/*-------------------------------------------------------------------------*/

CMcpAccessRules::CMcpAccessRules( bool isDefaults )
    : m_isDefaults( isDefaults )
    , m_allowReadOnlyTools( true )
    , m_hideDenied( true )
    , m_allowSubscriptions( true )
    , m_allowTools()
    , m_denyTools()
    , m_allowResources()
    , m_denyResources()
    , m_allowPrompts()
    , m_denyPrompts()
    , m_setAllowReadOnlyTools( false )
    , m_setHideDenied( false )
    , m_setAllowSubscriptions( false )
    , m_setAllowTools( false )
    , m_setAllowResources( false )
    , m_setAllowPrompts( false )
{GUCEF_TRACE;

    if ( m_isDefaults )
    {
        m_allowResources.Add( "**", true );
        m_allowPrompts.Add( "**", false );
    }
}

/*-------------------------------------------------------------------------*/

void
CMcpAccessRules::SetAllowReadOnlyTools( bool allow )
{GUCEF_TRACE;

    m_allowReadOnlyTools = allow;
    m_setAllowReadOnlyTools = true;
}

/*-------------------------------------------------------------------------*/

void
CMcpAccessRules::SetHideDenied( bool hide )
{GUCEF_TRACE;

    m_hideDenied = hide;
    m_setHideDenied = true;
}

/*-------------------------------------------------------------------------*/

void
CMcpAccessRules::SetAllowSubscriptions( bool allow )
{GUCEF_TRACE;

    m_allowSubscriptions = allow;
    m_setAllowSubscriptions = true;
}

/*-------------------------------------------------------------------------*/

bool
CMcpAccessRules::AddAllowTool( const CORE::CString& pattern )
{GUCEF_TRACE;

    m_setAllowTools = true;
    return m_allowTools.Add( pattern, false );
}

/*-------------------------------------------------------------------------*/

bool
CMcpAccessRules::AddDenyTool( const CORE::CString& pattern )
{GUCEF_TRACE;

    return m_denyTools.Add( pattern, false );
}

/*-------------------------------------------------------------------------*/

bool
CMcpAccessRules::AddAllowResource( const CORE::CString& pattern )
{GUCEF_TRACE;

    m_setAllowResources = true;
    return m_allowResources.Add( pattern, true );
}

/*-------------------------------------------------------------------------*/

bool
CMcpAccessRules::AddDenyResource( const CORE::CString& pattern )
{GUCEF_TRACE;

    return m_denyResources.Add( pattern, true );
}

/*-------------------------------------------------------------------------*/

bool
CMcpAccessRules::AddAllowPrompt( const CORE::CString& pattern )
{GUCEF_TRACE;

    m_setAllowPrompts = true;
    return m_allowPrompts.Add( pattern, false );
}

/*-------------------------------------------------------------------------*/

bool
CMcpAccessRules::AddDenyPrompt( const CORE::CString& pattern )
{GUCEF_TRACE;

    return m_denyPrompts.Add( pattern, false );
}

/*-------------------------------------------------------------------------*/

void
CMcpAccessRules::ClearAllowResources( void )
{GUCEF_TRACE;

    m_allowResources.Clear();
    m_setAllowResources = true;
}

/*-------------------------------------------------------------------------*/

void
CMcpAccessRules::ClearAllowPrompts( void )
{GUCEF_TRACE;

    m_allowPrompts.Clear();
    m_setAllowPrompts = true;
}

/*-------------------------------------------------------------------------*/

bool CMcpAccessRules::GetAllowReadOnlyTools( void ) const {GUCEF_TRACE; return m_allowReadOnlyTools; }
bool CMcpAccessRules::GetHideDenied( void ) const {GUCEF_TRACE; return m_hideDenied; }
bool CMcpAccessRules::GetAllowSubscriptions( void ) const {GUCEF_TRACE; return m_allowSubscriptions; }
const CMcpPatternList& CMcpAccessRules::GetAllowTools( void ) const {GUCEF_TRACE; return m_allowTools; }
const CMcpPatternList& CMcpAccessRules::GetDenyTools( void ) const {GUCEF_TRACE; return m_denyTools; }
const CMcpPatternList& CMcpAccessRules::GetAllowResources( void ) const {GUCEF_TRACE; return m_allowResources; }
const CMcpPatternList& CMcpAccessRules::GetDenyResources( void ) const {GUCEF_TRACE; return m_denyResources; }
const CMcpPatternList& CMcpAccessRules::GetAllowPrompts( void ) const {GUCEF_TRACE; return m_allowPrompts; }
const CMcpPatternList& CMcpAccessRules::GetDenyPrompts( void ) const {GUCEF_TRACE; return m_denyPrompts; }

/*-------------------------------------------------------------------------*/

bool
CMcpAccessRules::LoadFrom( const CORE::CDataNode& node )
{GUCEF_TRACE;

    CMcpAccessRules loaded( m_isDefaults );
    bool found = false;

    const CORE::CVariant readOnly = node.GetAttributeValueOrChildValueByName( "allowReadOnlyTools" );
    if ( readOnly.IsInitialized() )
        loaded.SetAllowReadOnlyTools( readOnly.AsBool( true, true ) );
    const CORE::CVariant hide = node.GetAttributeValueOrChildValueByName( "hideDenied" );
    if ( hide.IsInitialized() )
        loaded.SetHideDenied( hide.AsBool( true, true ) );
    const CORE::CVariant subscriptions = node.GetAttributeValueOrChildValueByName( "allowSubscriptions" );
    if ( subscriptions.IsInitialized() )
        loaded.SetAllowSubscriptions( subscriptions.AsBool( true, true ) );

    if ( !LoadPatterns( node, "allowTools", false, loaded.m_allowTools, found ) )
        return false;
    loaded.m_setAllowTools = found;
    if ( !LoadPatterns( node, "denyTools", false, loaded.m_denyTools, found ) )
        return false;

    CMcpPatternList list;
    if ( !LoadPatterns( node, "allowResources", true, list, found ) )
        return false;
    if ( found )
    {
        loaded.m_allowResources = list;
        loaded.m_setAllowResources = true;
    }
    if ( !LoadPatterns( node, "denyResources", true, loaded.m_denyResources, found ) )
        return false;
    if ( !LoadPatterns( node, "allowPrompts", false, list, found ) )
        return false;
    if ( found )
    {
        loaded.m_allowPrompts = list;
        loaded.m_setAllowPrompts = true;
    }
    if ( !LoadPatterns( node, "denyPrompts", false, loaded.m_denyPrompts, found ) )
        return false;

    *this = loaded;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CMcpAccessRules::SaveTo( CORE::CDataNode& node ) const
{GUCEF_TRACE;

    if ( m_isDefaults || m_setAllowReadOnlyTools )
        node.SetAttribute( "allowReadOnlyTools", m_allowReadOnlyTools );
    if ( m_isDefaults || m_setHideDenied )
        node.SetAttribute( "hideDenied", m_hideDenied );
    if ( m_isDefaults || m_setAllowSubscriptions )
        node.SetAttribute( "allowSubscriptions", m_allowSubscriptions );
    if ( m_isDefaults || m_setAllowTools )
        node.SetValuesOfChildByName( "allowTools", m_allowTools.GetPatterns() );
    if ( m_isDefaults || !m_denyTools.IsEmpty() )
        node.SetValuesOfChildByName( "denyTools", m_denyTools.GetPatterns() );
    if ( m_isDefaults || m_setAllowResources )
        node.SetValuesOfChildByName( "allowResources", m_allowResources.GetPatterns() );
    if ( m_isDefaults || !m_denyResources.IsEmpty() )
        node.SetValuesOfChildByName( "denyResources", m_denyResources.GetPatterns() );
    if ( m_isDefaults || m_setAllowPrompts )
        node.SetValuesOfChildByName( "allowPrompts", m_allowPrompts.GetPatterns() );
    if ( m_isDefaults || !m_denyPrompts.IsEmpty() )
        node.SetValuesOfChildByName( "denyPrompts", m_denyPrompts.GetPatterns() );
}

/*-------------------------------------------------------------------------*/

void
CMcpAccessRules::Apply( const CMcpAccessRules& entry )
{GUCEF_TRACE;

    if ( entry.m_setAllowReadOnlyTools )
        m_allowReadOnlyTools = entry.m_allowReadOnlyTools;
    if ( entry.m_setHideDenied )
        m_hideDenied = entry.m_hideDenied;
    if ( entry.m_setAllowSubscriptions )
        m_allowSubscriptions = entry.m_allowSubscriptions;
    if ( entry.m_setAllowTools )
        m_allowTools = entry.m_allowTools;
    if ( entry.m_setAllowResources )
        m_allowResources = entry.m_allowResources;
    if ( entry.m_setAllowPrompts )
        m_allowPrompts = entry.m_allowPrompts;

    // a deny is never taken away
    const CORE::CString::StringVector& denyTools = entry.m_denyTools.GetPatterns();
    for ( size_t i=0; i<denyTools.size(); ++i )
        m_denyTools.Add( denyTools[ i ], false );
    const CORE::CString::StringVector& denyResources = entry.m_denyResources.GetPatterns();
    for ( size_t i=0; i<denyResources.size(); ++i )
        m_denyResources.Add( denyResources[ i ], true );
    const CORE::CString::StringVector& denyPrompts = entry.m_denyPrompts.GetPatterns();
    for ( size_t i=0; i<denyPrompts.size(); ++i )
        m_denyPrompts.Add( denyPrompts[ i ], false );
}

/*-------------------------------------------------------------------------*/

CMcpConfigurableAccessPolicy::CMcpConfigurableAccessPolicy( void )
    : CIMcpAccessPolicy()
    , CORE::CIConfigurable()
    , m_defaults( true )
    , m_principals()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMcpConfigurableAccessPolicy::CMcpConfigurableAccessPolicy( const CMcpConfigurableAccessPolicy& src )
    : CIMcpAccessPolicy()
    , CORE::CITypeNamed()
    , CORE::CIConfigurable()
    , m_defaults( true )
    , m_principals()
{GUCEF_TRACE;

    CopyFrom( src );
}

/*-------------------------------------------------------------------------*/

CMcpConfigurableAccessPolicy&
CMcpConfigurableAccessPolicy::operator=( const CMcpConfigurableAccessPolicy& src )
{GUCEF_TRACE;

    if ( this != &src )
    {
        Clear();
        CopyFrom( src );
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CMcpConfigurableAccessPolicy::~CMcpConfigurableAccessPolicy()
{GUCEF_TRACE;

    Clear();
}

/*-------------------------------------------------------------------------*/

void
CMcpConfigurableAccessPolicy::Clear( void )
{GUCEF_TRACE;

    for ( size_t i=0; i<m_principals.size(); ++i )
        GUCEF_DELETE m_principals[ i ];
    m_principals.clear();
}

/*-------------------------------------------------------------------------*/

void
CMcpConfigurableAccessPolicy::CopyFrom( const CMcpConfigurableAccessPolicy& src )
{GUCEF_TRACE;

    m_defaults = src.m_defaults;
    for ( size_t i=0; i<src.m_principals.size(); ++i )
        m_principals.push_back( GUCEF_NEW SPrincipalEntry( *src.m_principals[ i ] ) );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CMcpConfigurableAccessPolicy::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::COM::CMcpConfigurableAccessPolicy";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

CMcpAccessRules&
CMcpConfigurableAccessPolicy::GetDefaultRules( void )
{GUCEF_TRACE;

    return m_defaults;
}

/*-------------------------------------------------------------------------*/

const CMcpAccessRules&
CMcpConfigurableAccessPolicy::GetDefaultRules( void ) const
{GUCEF_TRACE;

    return m_defaults;
}

/*-------------------------------------------------------------------------*/

CMcpAccessRules*
CMcpConfigurableAccessPolicy::AddPrincipalRules( const CORE::CString& subjectPattern )
{GUCEF_TRACE;

    SPrincipalEntry* entry = GUCEF_NEW SPrincipalEntry();
    entry->subjectPattern = subjectPattern;
    if ( !entry->subject.SetPattern( subjectPattern, true ) )
    {
        GUCEF_DELETE entry;
        return GUCEF_NULL;
    }
    m_principals.push_back( entry );
    return &entry->rules;
}

/*-------------------------------------------------------------------------*/

bool
CMcpConfigurableAccessPolicy::IsVisibilityPrincipalDependent( void ) const
{GUCEF_TRACE;

    return !m_principals.empty();
}

/*-------------------------------------------------------------------------*/

const CMcpAccessRules&
CMcpConfigurableAccessPolicy::GetRules( const CMcpPrincipal& principal, CMcpAccessRules& storage ) const
{GUCEF_TRACE;

    bool copied = false;
    for ( size_t i=0; i<m_principals.size(); ++i )
    {
        if ( m_principals[ i ]->subject.IsMatch( principal.subject ) )
        {
            if ( !copied )
            {
                storage = m_defaults;
                copied = true;
            }
            storage.Apply( m_principals[ i ]->rules );
        }
    }
    return copied ? storage : m_defaults;
}

/*-------------------------------------------------------------------------*/

bool
CMcpConfigurableAccessPolicy::IsToolAllowed( const CMcpAccessRules& rules, const CMcpToolEntry& tool, CORE::CString& reason )
{GUCEF_TRACE;

    const CORE::CString& name = tool.descriptor.name;
    if ( rules.GetDenyTools().Matches( name ) )
    {
        reason = "the tool is denied by the access policy";
        return false;
    }
    if ( rules.GetAllowTools().Matches( name ) )
        return true;
    if ( MCP_TOOLEFFECT_READ_ONLY == tool.descriptor.effect && rules.GetAllowReadOnlyTools() )
        return true;
    reason = MCP_TOOLEFFECT_READ_ONLY == tool.descriptor.effect ? "the access policy does not allow read-only tools"
                                                                : "the tool is not read-only and the access policy does not allow it";
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CMcpConfigurableAccessPolicy::IsToolVisible( const CMcpPrincipal& principal, const CMcpToolEntry& tool ) const
{GUCEF_TRACE;

    CMcpAccessRules storage( false );
    const CMcpAccessRules& rules = GetRules( principal, storage );
    CORE::CString reason;
    return IsToolAllowed( rules, tool, reason ) || !rules.GetHideDenied();
}

/*-------------------------------------------------------------------------*/

EMcpAccessDecision
CMcpConfigurableAccessPolicy::AuthorizeToolCall( const CMcpRequestContext& context ,
                                                 const CMcpToolEntry& tool          ,
                                                 const CORE::CDataNode& arguments   ,
                                                 CORE::CString& reason              ) const
{GUCEF_TRACE;

    CMcpAccessRules storage( false );
    const CMcpAccessRules& rules = GetRules( context.GetPrincipal(), storage );
    if ( IsToolAllowed( rules, tool, reason ) )
        return MCP_ACCESS_ALLOW;
    return rules.GetHideDenied() ? MCP_ACCESS_HIDE : MCP_ACCESS_DENY;
}

/*-------------------------------------------------------------------------*/

bool
CMcpConfigurableAccessPolicy::IsResourceVisible( const CMcpPrincipal& principal, const CORE::CString& uri ) const
{GUCEF_TRACE;

    CMcpAccessRules storage( false );
    const CMcpAccessRules& rules = GetRules( principal, storage );
    const bool allowed = !rules.GetDenyResources().Matches( uri ) && rules.GetAllowResources().Matches( uri );
    return allowed || !rules.GetHideDenied();
}

/*-------------------------------------------------------------------------*/

EMcpAccessDecision
CMcpConfigurableAccessPolicy::AuthorizeResourceRead( const CMcpRequestContext& context, const CORE::CString& uri, CORE::CString& reason ) const
{GUCEF_TRACE;

    CMcpAccessRules storage( false );
    const CMcpAccessRules& rules = GetRules( context.GetPrincipal(), storage );
    if ( rules.GetDenyResources().Matches( uri ) )
        reason = "the resource is denied by the access policy";
    else
    if ( !rules.GetAllowResources().Matches( uri ) )
        reason = "the access policy does not allow the resource";
    else
        return MCP_ACCESS_ALLOW;
    return rules.GetHideDenied() ? MCP_ACCESS_HIDE : MCP_ACCESS_DENY;
}

/*-------------------------------------------------------------------------*/

bool
CMcpConfigurableAccessPolicy::IsPromptVisible( const CMcpPrincipal& principal, const CMcpPromptEntry& prompt ) const
{GUCEF_TRACE;

    CMcpAccessRules storage( false );
    const CMcpAccessRules& rules = GetRules( principal, storage );
    const CORE::CString& name = prompt.descriptor.name;
    const bool allowed = !rules.GetDenyPrompts().Matches( name ) && rules.GetAllowPrompts().Matches( name );
    return allowed || !rules.GetHideDenied();
}

/*-------------------------------------------------------------------------*/

EMcpAccessDecision
CMcpConfigurableAccessPolicy::AuthorizePromptGet( const CMcpRequestContext& context, const CMcpPromptEntry& prompt, CORE::CString& reason ) const
{GUCEF_TRACE;

    CMcpAccessRules storage( false );
    const CMcpAccessRules& rules = GetRules( context.GetPrincipal(), storage );
    const CORE::CString& name = prompt.descriptor.name;
    if ( rules.GetDenyPrompts().Matches( name ) )
        reason = "the prompt is denied by the access policy";
    else
    if ( !rules.GetAllowPrompts().Matches( name ) )
        reason = "the access policy does not allow the prompt";
    else
        return MCP_ACCESS_ALLOW;
    return rules.GetHideDenied() ? MCP_ACCESS_HIDE : MCP_ACCESS_DENY;
}

/*-------------------------------------------------------------------------*/

EMcpAccessDecision
CMcpConfigurableAccessPolicy::AuthorizeSubscription( const CMcpRequestContext& context, CORE::CString& reason ) const
{GUCEF_TRACE;

    CMcpAccessRules storage( false );
    const CMcpAccessRules& rules = GetRules( context.GetPrincipal(), storage );
    if ( rules.GetAllowSubscriptions() )
        return MCP_ACCESS_ALLOW;
    reason = "the access policy does not allow subscriptions";
    return MCP_ACCESS_DENY;
}

/*-------------------------------------------------------------------------*/

bool
CMcpConfigurableAccessPolicy::SaveConfig( CORE::CDataNode& cfg ) const
{GUCEF_TRACE;

    CORE::CDataNode* section = cfg.FindChild( ConfigSectionName );
    if ( GUCEF_NULL == section )
        section = cfg.AddChild( ConfigSectionName, GUCEF_DATATYPE_OBJECT );
    section->DelChild( "principals" );
    m_defaults.SaveTo( *section );

    if ( !m_principals.empty() )
    {
        CORE::CDataNode* principals = section->AddChild( "principals", GUCEF_DATATYPE_ARRAY );
        for ( size_t i=0; i<m_principals.size(); ++i )
        {
            CORE::CDataNode* entry = principals->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT );
            entry->SetAttribute( "subject", CORE::CVariant( m_principals[ i ]->subjectPattern ) );
            m_principals[ i ]->rules.SaveTo( *entry );
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMcpConfigurableAccessPolicy::LoadConfig( const CORE::CDataNode& cfg )
{GUCEF_TRACE;

    const CORE::CDataNode* section = cfg.FindChild( ConfigSectionName );
    if ( GUCEF_NULL == section )
        section = &cfg;

    CMcpConfigurableAccessPolicy loaded;
    if ( !loaded.m_defaults.LoadFrom( *section ) )
        return false;

    const CORE::CDataNode* principals = section->FindChild( "principals" );
    if ( GUCEF_NULL != principals )
    {
        for ( CORE::CDataNode::const_iterator i = principals->ConstBegin(); i != principals->ConstEnd(); ++i )
        {
            const CORE::CDataNode& entryNode = *(*i);
            const CORE::CString subject = entryNode.GetAttributeValueOrChildValueByName( "subject" ).AsString( CORE::CString::Empty, true );
            if ( subject.IsNULLOrEmpty() )
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "McpConfigurableAccessPolicy: An entry of principals has no subject pattern" );
                return false;
            }
            CMcpAccessRules* rules = loaded.AddPrincipalRules( subject );
            if ( GUCEF_NULL == rules )
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "McpConfigurableAccessPolicy: The subject pattern \"" << subject << "\" is not a valid glob pattern" );
                return false;
            }
            if ( !rules->LoadFrom( entryNode ) )
                return false;
        }
    }

    *this = loaded;
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
