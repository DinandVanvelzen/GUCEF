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

#ifndef GUCEF_COM_CMCPCONFIGURABLEACCESSPOLICY_H
#define GUCEF_COM_CMCPCONFIGURABLEACCESSPOLICY_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CICONFIGURABLE_H
#include "gucefCORE_CIConfigurable.h"
#define GUCEF_CORE_CICONFIGURABLE_H
#endif /* GUCEF_CORE_CICONFIGURABLE_H ? */

#ifndef GUCEF_CORE_CGLOBPATTERN_H
#include "gucefCORE_CGlobPattern.h"
#define GUCEF_CORE_CGLOBPATTERN_H
#endif /* GUCEF_CORE_CGLOBPATTERN_H ? */

#ifndef GUCEF_COM_CIMCPACCESSPOLICY_H
#include "gucefCOM_CIMcpAccessPolicy.h"
#define GUCEF_COM_CIMCPACCESSPOLICY_H
#endif /* GUCEF_COM_CIMCPACCESSPOLICY_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COM {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  A list of glob patterns, compiled, next to their source text
 */
class GUCEF_COM_PUBLIC_CPP CMcpPatternList
{
    public:

    CMcpPatternList( void );

    /**
     *  @param isUri    '*' also matches '/', see CMcpConfigurableAccessPolicy
     *  @return false when the pattern is not a valid glob, in which case nothing is added
     */
    bool Add( const CORE::CString& pattern, bool isUri );

    void Clear( void );

    bool IsEmpty( void ) const;

    bool Matches( const CORE::CString& text ) const;

    const CORE::CString::StringVector& GetPatterns( void ) const;

    private:

    typedef GUCEF::vector< CORE::CGlobPattern > TPatternVector;

    CORE::CString::StringVector m_sources;
    TPatternVector m_patterns;
};

/*-------------------------------------------------------------------------*/

/**
 *  The rules for one principal, see CMcpConfigurableAccessPolicy. The defaults of the policy are a set of these, and so
 *  is every entry for a group of principals; an entry only holds what it sets.
 */
class GUCEF_COM_PUBLIC_CPP CMcpAccessRules
{
    public:

    /** @param isDefaults   the rules start with the defaults of the policy (all resources and prompts allowed) and not as an empty entry */
    explicit CMcpAccessRules( bool isDefaults = true );

    void SetAllowReadOnlyTools( bool allow );
    void SetHideDenied( bool hide );
    void SetAllowSubscriptions( bool allow );

    /** The Add functions return false for a pattern that is not valid */
    bool AddAllowTool( const CORE::CString& pattern );
    bool AddDenyTool( const CORE::CString& pattern );
    bool AddAllowResource( const CORE::CString& pattern );
    bool AddDenyResource( const CORE::CString& pattern );
    bool AddAllowPrompt( const CORE::CString& pattern );
    bool AddDenyPrompt( const CORE::CString& pattern );

    /** Forgets the "**" of the defaults, so that only what is added is allowed; for an entry it states that the list is its own */
    void ClearAllowResources( void );
    void ClearAllowPrompts( void );

    bool GetAllowReadOnlyTools( void ) const;
    bool GetHideDenied( void ) const;
    bool GetAllowSubscriptions( void ) const;
    const CMcpPatternList& GetAllowTools( void ) const;
    const CMcpPatternList& GetDenyTools( void ) const;
    const CMcpPatternList& GetAllowResources( void ) const;
    const CMcpPatternList& GetDenyResources( void ) const;
    const CMcpPatternList& GetAllowPrompts( void ) const;
    const CMcpPatternList& GetDenyPrompts( void ) const;

    /** Reads the keys from a node. @return false when a pattern is not valid */
    bool LoadFrom( const CORE::CDataNode& node );

    /** Writes what the rules hold: everything for defaults, only what an entry sets for an entry */
    void SaveTo( CORE::CDataNode& node ) const;

    /** Puts the rules of an entry on top of these: the keys it sets replace, its denies are added */
    void Apply( const CMcpAccessRules& entry );

    private:

    bool m_isDefaults;
    bool m_allowReadOnlyTools;
    bool m_hideDenied;
    bool m_allowSubscriptions;
    CMcpPatternList m_allowTools;
    CMcpPatternList m_denyTools;
    CMcpPatternList m_allowResources;
    CMcpPatternList m_denyResources;
    CMcpPatternList m_allowPrompts;
    CMcpPatternList m_denyPrompts;

    // which keys the rules set themselves: for entries
    bool m_setAllowReadOnlyTools;
    bool m_setHideDenied;
    bool m_setAllowSubscriptions;
    bool m_setAllowTools;
    bool m_setAllowResources;
    bool m_setAllowPrompts;
};

/*-------------------------------------------------------------------------*/

/**
 *  The built-in access policy. The defaults are closed for everything that changes something:
 *
 *      allowReadOnlyTools   true       tools with the effect MCP_TOOLEFFECT_READ_ONLY are allowed
 *      allowTools           (none)     glob patterns of tool names that are allowed whatever their effect: this is how
 *                                      a tool that adds or destroys something is switched on
 *      denyTools            (none)     names that are never allowed: wins over everything
 *      hideDenied           true       a denied tool is hidden from lists and refused as unknown (MCP_ACCESS_HIDE); when
 *                                      false it is listed and a call gets a tool execution error that says why (MCP_ACCESS_DENY)
 *      allowResources       **         glob patterns of resource URIs (and of the URI templates of templates)
 *      denyResources        (none)
 *      allowPrompts         **         glob patterns of prompt names
 *      denyPrompts          (none)
 *      allowSubscriptions   true
 *      principals           (none)     one entry per group of principals: a 'subject' glob and any of the keys above
 *
 *  Patterns are case sensitive. In the lists of URIs a '*' also matches '/', since a URI is no path: the pattern "file:///docs/" followed
 *  by a star matches "file:///docs/a/b.txt". Elsewhere the glob rules of CORE::CGlobPattern apply.
 *
 *  A principal entry applies to the principals whose subject matches its 'subject' pattern. Every matching entry is
 *  applied, in the order of the configuration, on top of the defaults. A key of an entry replaces that key of the
 *  defaults; the exception is every deny list, which only ever grows: a deny that is configured for everybody cannot
 *  be taken away for a principal. When there is an entry, what lists show depends on the principal.
 *
 *  Configuration (section "McpAccessPolicy"): the keys are attributes or children by their names. Lists are an array
 *  child of strings, or one text with elements separated by ';'. The principals are an array child "principals" of
 *  objects, each with a "subject" and the keys it overrides.
 *
 *  Thread safety. The members that decide are const and read only; SetXxx() and LoadConfig() are to be called while no
 *  request is being decided (before the policy is handed to the server, or through a copy that replaces it with
 *  CMcpServer::SetAccessPolicy()).
 */
class GUCEF_COM_PUBLIC_CPP CMcpConfigurableAccessPolicy : public CIMcpAccessPolicy    ,
                                                          public CORE::CIConfigurable
{
    public:

    static const CORE::CString ConfigSectionName;       /**< "McpAccessPolicy" */

    CMcpConfigurableAccessPolicy( void );

    CMcpConfigurableAccessPolicy( const CMcpConfigurableAccessPolicy& src );

    CMcpConfigurableAccessPolicy& operator=( const CMcpConfigurableAccessPolicy& src );

    virtual ~CMcpConfigurableAccessPolicy() GUCEF_VIRTUAL_OVERRIDE;

    /** The rules for principals without an entry of their own, which are also the base of the entries */
    CMcpAccessRules& GetDefaultRules( void );
    const CMcpAccessRules& GetDefaultRules( void ) const;

    /**
     *  Rules for the principals whose subject matches the pattern, on top of the defaults. The entry is edited with
     *  the members of CMcpAccessRules; what it does not set is taken from the defaults. The policy owns the entry.
     *  @return GUCEF_NULL when the pattern is not a valid glob
     */
    CMcpAccessRules* AddPrincipalRules( const CORE::CString& subjectPattern );

    // CIMcpAccessPolicy

    virtual bool IsVisibilityPrincipalDependent( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsToolVisible( const CMcpPrincipal& principal, const CMcpToolEntry& tool ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual EMcpAccessDecision AuthorizeToolCall( const CMcpRequestContext& context ,
                                                  const CMcpToolEntry& tool          ,
                                                  const CORE::CDataNode& arguments   ,
                                                  CORE::CString& reason              ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsResourceVisible( const CMcpPrincipal& principal, const CORE::CString& uri ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual EMcpAccessDecision AuthorizeResourceRead( const CMcpRequestContext& context, const CORE::CString& uri, CORE::CString& reason ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsPromptVisible( const CMcpPrincipal& principal, const CMcpPromptEntry& prompt ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual EMcpAccessDecision AuthorizePromptGet( const CMcpRequestContext& context, const CMcpPromptEntry& prompt, CORE::CString& reason ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual EMcpAccessDecision AuthorizeSubscription( const CMcpRequestContext& context, CORE::CString& reason ) const GUCEF_VIRTUAL_OVERRIDE;

    // CIConfigurable

    virtual bool SaveConfig( CORE::CDataNode& cfg ) const GUCEF_VIRTUAL_OVERRIDE;

    /** Reads the "McpAccessPolicy" child of cfg, or cfg itself when it has none. Nothing changes when the configuration is refused */
    virtual bool LoadConfig( const CORE::CDataNode& cfg ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    struct SPrincipalEntry
    {
        CORE::CString subjectPattern;
        CORE::CGlobPattern subject;
        CMcpAccessRules rules;

        SPrincipalEntry( void ) : subjectPattern(), subject(), rules( false ) {}
    };
    typedef GUCEF::vector< SPrincipalEntry* > TPrincipalEntryVector;

    void Clear( void );
    void CopyFrom( const CMcpConfigurableAccessPolicy& src );

    /** The rules for a principal: the defaults themselves when no entry matches, else 'storage' with the entries applied */
    const CMcpAccessRules& GetRules( const CMcpPrincipal& principal, CMcpAccessRules& storage ) const;

    static bool IsToolAllowed( const CMcpAccessRules& rules, const CMcpToolEntry& tool, CORE::CString& reason );

    private:

    CMcpAccessRules m_defaults;
    TPrincipalEntryVector m_principals;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COM_CMCPCONFIGURABLEACCESSPOLICY_H ? */
