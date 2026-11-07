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

#ifndef GUCEF_COM_CMCPRESOURCEREGISTRY_H
#define GUCEF_COM_CMCPRESOURCEREGISTRY_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_COM_CMCPREGISTRYBASE_H
#include "gucefCOM_CMcpRegistryBase.h"
#define GUCEF_COM_CMCPREGISTRYBASE_H
#endif /* GUCEF_COM_CMCPREGISTRYBASE_H ? */

#ifndef GUCEF_COM_CIMCPRESOURCE_H
#include "gucefCOM_CIMcpResource.h"
#define GUCEF_COM_CIMCPRESOURCE_H
#endif /* GUCEF_COM_CIMCPRESOURCE_H ? */

#ifndef GUCEF_COM_CIMCPRESOURCETEMPLATE_H
#include "gucefCOM_CIMcpResourceTemplate.h"
#define GUCEF_COM_CIMCPRESOURCETEMPLATE_H
#endif /* GUCEF_COM_CIMCPRESOURCETEMPLATE_H ? */

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
 *  An RFC 6570 URI template compiled into a matcher. Only simple string expansion ({var}) and reserved expansion
 *  ({+var}) are supported; other operators, modifiers and lists of variables are refused when it is parsed.
 *  {var} matches one or more unreserved characters or percent-encoded triplets and gives the value percent-decoded;
 *  {+var} matches one or more of any character and gives the value as it is in the URI.
 */
class GUCEF_COM_PUBLIC_CPP CMcpUriTemplate
{
    public:

    /** The most matching steps a URI may take: ambiguous templates on long URIs fail instead of taking long */
    static const UInt32 MaxMatchSteps = 100000;

    CMcpUriTemplate( void );

    bool Parse( const CORE::CString& uriTemplate, CORE::CString& error );

    bool Match( const CORE::CString& uri, CORE::CValueList& variables ) const;

    UInt32 GetVariableCount( void ) const;

    private:

    enum ESegmentKind
    {
        SEGMENT_LITERAL     ,
        SEGMENT_SIMPLE      ,
        SEGMENT_RESERVED
    };

    struct SSegment
    {
        ESegmentKind kind;
        CORE::CString text;         /**< the literal, or the name of the variable */
    };
    typedef GUCEF::vector< SSegment > TSegmentVector;

    struct SMatchState;

    bool MatchFrom( SMatchState& state, UInt32 segmentIndex, UInt32 position ) const;

    TSegmentVector m_segments;
    UInt32 m_variableCount;
};

/*-------------------------------------------------------------------------*/

/** A registered fixed resource, read only once registered */
class GUCEF_COM_PUBLIC_CPP CMcpResourceEntry
{
    public:

    CIMcpResourcePtr resource;
    CMcpResourceDescriptor descriptor;
    CORE::CDataNode descriptorNode;         /**< the Resource object of resources/list */
    UInt64 generation;

    CMcpResourceEntry( void );
};

typedef CORE::CTSharedPtr< CMcpResourceEntry, MT::CMutex > CMcpResourceEntryPtr;

/*-------------------------------------------------------------------------*/

/** A registered resource template, read only once registered */
class GUCEF_COM_PUBLIC_CPP CMcpResourceTemplateEntry
{
    public:

    CIMcpResourceTemplatePtr resourceTemplate;
    CMcpResourceTemplateDescriptor descriptor;
    CMcpUriTemplate matcher;
    CORE::CDataNode descriptorNode;         /**< the ResourceTemplate object of resources/templates/list */
    UInt64 generation;

    CMcpResourceTemplateEntry( void );
};

typedef CORE::CTSharedPtr< CMcpResourceTemplateEntry, MT::CMutex > CMcpResourceTemplateEntryPtr;

/*-------------------------------------------------------------------------*/

/**
 *  The fixed resources a server exposes, keyed by URI, and the resource templates, keyed by name, both in byte
 *  order. A URI is resolved to the exact resource first, then to the first template in name order that matches.
 *  See CMcpRegistryBase for events and thread safety; the list change covers resources and templates.
 */
class GUCEF_COM_PUBLIC_CPP CMcpResourceRegistry : public CMcpRegistryBase
{
    public:

    static const CORE::CEvent ResourceRegisteredEvent;      /**< TNameEventData: the URI */
    static const CORE::CEvent ResourceUnregisteredEvent;    /**< TNameEventData: the URI */
    static const CORE::CEvent TemplateRegisteredEvent;      /**< TNameEventData: the name */
    static const CORE::CEvent TemplateUnregisteredEvent;    /**< TNameEventData: the name */

    typedef GUCEF::vector< CMcpResourceEntryPtr > TResourceEntryVector;
    typedef GUCEF::vector< CMcpResourceTemplateEntryPtr > TTemplateEntryVector;

    enum EResolveResult
    {
        RESOLVE_NONE        ,
        RESOLVE_RESOURCE    ,
        RESOLVE_TEMPLATE
    };

    static void RegisterEvents( void );

    /**
     *  The one spelling of a resource URI that the server reads, authorizes and answers with (RFC 3986 6.2.2):
     *  the scheme and the host in lower case, percent-encoded unreserved characters decoded, the hexadecimal digits of
     *  the other percent-encodings in upper case, and the dot segments of the path removed. Refused, because no
     *  spelling of them is safe to compare or to hand to a handler: a malformed percent-encoding, a percent-encoded
     *  NUL, '/' or '\' in the authority or the path, percent-encoded or raw bytes that are no UTF-8, control
     *  characters and spaces, and text without a scheme.
     *  @return false with the reason in 'problem' when the URI is refused
     */
    static bool NormalizeUri( const CORE::CString& uri, CORE::CString& normalized, CORE::CString& problem );

    explicit CMcpResourceRegistry( const CORE::PulseGeneratorPtr& pulseGenerator = CORE::PulseGeneratorPtr() );

    virtual ~CMcpResourceRegistry() GUCEF_VIRTUAL_OVERRIDE;

    /** Refused without a URI or a name, or when another resource has the URI; the same resource again replaces it */
    bool RegisterResource( const CIMcpResourcePtr& resource, CORE::CString& error );

    bool UnregisterResource( const CORE::CString& uri );

    /** Refused without a name, with a URI template that does not parse, or when another template has the name */
    bool RegisterTemplate( const CIMcpResourceTemplatePtr& resourceTemplate, CORE::CString& error );

    bool UnregisterTemplate( const CORE::CString& name );

    void UnregisterAll( void );

    bool TryGetResource( const CORE::CString& uri, CMcpResourceEntryPtr& entry ) const;

    bool TryGetTemplate( const CORE::CString& name, CMcpResourceTemplateEntryPtr& entry ) const;

    /** Finds what serves a URI: the exact resource, or the first template in name order that matches */
    EResolveResult Resolve( const CORE::CString& uri                        ,
                            CMcpResourceEntryPtr& resourceEntry             ,
                            CMcpResourceTemplateEntryPtr& templateEntry     ,
                            CORE::CValueList& variables                     ) const;

    void GetResources( TResourceEntryVector& entries ) const;

    void GetTemplates( TTemplateEntryVector& entries ) const;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    typedef GUCEF::map< CORE::CString, CMcpResourceEntryPtr > TResourceEntryMap;
    typedef GUCEF::map< CORE::CString, CMcpResourceTemplateEntryPtr > TTemplateEntryMap;

    TResourceEntryMap m_resources;
    TTemplateEntryMap m_templates;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COM_CMCPRESOURCEREGISTRY_H ? */
