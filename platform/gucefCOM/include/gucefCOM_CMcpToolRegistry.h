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

#ifndef GUCEF_COM_CMCPTOOLREGISTRY_H
#define GUCEF_COM_CMCPTOOLREGISTRY_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CJSONSCHEMA_H
#include "gucefCORE_CJsonSchema.h"
#define GUCEF_CORE_CJSONSCHEMA_H
#endif /* GUCEF_CORE_CJSONSCHEMA_H ? */

#ifndef GUCEF_COM_CMCPREGISTRYBASE_H
#include "gucefCOM_CMcpRegistryBase.h"
#define GUCEF_COM_CMCPREGISTRYBASE_H
#endif /* GUCEF_COM_CMCPREGISTRYBASE_H ? */

#ifndef GUCEF_COM_CIMCPTOOL_H
#include "gucefCOM_CIMcpTool.h"
#define GUCEF_COM_CIMCPTOOL_H
#endif /* GUCEF_COM_CIMCPTOOL_H ? */

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
 *  A registered tool: the tool, its descriptor as asked once at registration, both schemas compiled, the parameters
 *  that are mirrored into HTTP headers (x-mcp-header) and the descriptor already written as the Tool object of
 *  tools/list. Read only once registered, so it is shared between threads without a lock.
 */
class GUCEF_COM_PUBLIC_CPP CMcpToolEntry
{
    public:

    /** A parameter with an x-mcp-header annotation, reachable from the root through properties only */
    struct GUCEF_COM_PUBLIC_CPP SHeaderParameter
    {
        CORE::CString headerName;                   /**< the {Name} of the Mcp-Param-{Name} header */
        CORE::CString::StringVector propertyPath;   /**< the property names from the root of the arguments */
        CORE::CString type;                         /**< "string", "integer" or "boolean" */
    };
    typedef GUCEF::vector< SHeaderParameter > THeaderParameterVector;

    CIMcpToolPtr tool;
    CMcpToolDescriptor descriptor;
    CORE::CJsonSchema inputSchema;
    CORE::CJsonSchema outputSchema;
    bool hasOutputSchema;
    bool inputSchemaRequiresProperties;     /**< the root of the input schema names required properties */
    THeaderParameterVector headerParameters;
    CORE::CDataNode descriptorNode;         /**< the Tool object of tools/list */
    UInt64 generation;                      /**< the generation of the registry the tool registered in */

    CMcpToolEntry( void );

    private:
    CMcpToolEntry( const CMcpToolEntry& src );              /**< not implemented: a compiled schema is not copied */
    CMcpToolEntry& operator=( const CMcpToolEntry& src );   /**< not implemented */
};

typedef CORE::CTSharedPtr< CMcpToolEntry, MT::CMutex > CMcpToolEntryPtr;

/*-------------------------------------------------------------------------*/

/**
 *  The tools a server exposes, keyed by name in byte order: the same set gives the same order on every request,
 *  whatever the order of registration. See CMcpRegistryBase for events and thread safety.
 */
class GUCEF_COM_PUBLIC_CPP CMcpToolRegistry : public CMcpRegistryBase
{
    public:

    static const CORE::CEvent ToolRegisteredEvent;      /**< TNameEventData: the name */
    static const CORE::CEvent ToolUnregisteredEvent;    /**< TNameEventData: the name */

    static const UInt32 MaxToolNameLength = 128;

    typedef GUCEF::vector< CMcpToolEntryPtr > TToolEntryVector;

    static void RegisterEvents( void );

    explicit CMcpToolRegistry( const CORE::PulseGeneratorPtr& pulseGenerator = CORE::PulseGeneratorPtr() );

    virtual ~CMcpToolRegistry() GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  Asks the tool for its descriptor, checks it and stores it. Refused, with the reason in 'error' and in the log:
     *  - a name that is not 1 to 128 characters of A-Z a-z 0-9 _ - . (the rules of the tools page, as hard rules);
     *  - a name another tool has; the same tool registered again replaces its entry, which is how it changes;
     *  - an input schema that is not an object schema ("type": "object"), or a schema that does not compile;
     *  - an x-mcp-header annotation that breaks the rules of the Streamable HTTP transport.
     */
    bool Register( const CIMcpToolPtr& tool, CORE::CString& error );

    bool Unregister( const CORE::CString& name );

    void UnregisterAll( void );

    bool TryGet( const CORE::CString& name, CMcpToolEntryPtr& entry ) const;

    /** All entries in name order */
    void GetEntries( TToolEntryVector& entries ) const;

    UInt32 GetCount( void ) const;

    /** The name rules of the tools page */
    static bool IsValidToolName( const CORE::CString& name, CORE::CString& error );

    /**
     *  Checks the x-mcp-header annotations of an input schema: each a non-empty RFC 9110 token, case-insensitively
     *  unique, on a property of type string, integer or boolean that is reachable from the root through properties
     *  keys only. Fills 'parameters' with the annotated properties.
     */
    static bool CheckHeaderAnnotations( const CORE::CDataNode& inputSchema                      ,
                                        CMcpToolEntry::THeaderParameterVector& parameters       ,
                                        CORE::CString& error                                    );

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    typedef GUCEF::map< CORE::CString, CMcpToolEntryPtr > TToolEntryMap;

    TToolEntryMap m_entries;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COM_CMCPTOOLREGISTRY_H ? */
