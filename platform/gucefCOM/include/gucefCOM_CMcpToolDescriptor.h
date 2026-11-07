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

#ifndef GUCEF_COM_CMCPTOOLDESCRIPTOR_H
#define GUCEF_COM_CMCPTOOLDESCRIPTOR_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_COM_MCPTYPES_H
#include "gucefCOM_McpTypes.h"
#define GUCEF_COM_MCPTYPES_H
#endif /* GUCEF_COM_MCPTYPES_H ? */

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
 *  The metadata of a tool. The wire annotations (readOnlyHint, destructiveHint, idempotentHint, openWorldHint) are
 *  generated from effect, idempotent and openWorld, so what a client is told cannot differ from what the access
 *  policy uses: a tool cannot set them directly.
 */
class GUCEF_COM_PUBLIC_CPP CMcpToolDescriptor
{
    public:

    CORE::CString name;                         /**< 1 to 128 characters of A-Z a-z 0-9 _ - . ; unique, case-sensitive */
    CORE::CString title;
    CORE::CString description;
    CORE::CDataNode icons;                      /**< an array node of Icon objects, written as given */
    CORE::CDataNode inputSchema;                /**< a JSON Schema object with "type": "object" */
    CORE::CDataNode outputSchema;               /**< see hasOutputSchema */
    bool hasOutputSchema;
    EMcpToolEffect effect;
    bool idempotent;
    bool openWorld;                             /**< interacts with an open world of external entities */
    UInt32 maxDurationMs;                       /**< 0: the maxRequestDurationMs of the server */
    bool validateOutput;                        /**< validate structuredContent against the output schema, default true */
    CORE::CString::StringSet secretArguments;   /**< top level argument names that are masked in audit previews */
    CORE::CDataNode meta;                       /**< the _meta of the Tool, an object node, written when it has members */

    CMcpToolDescriptor( void );

    /** Sets the input schema of a tool without arguments: { "type": "object", "additionalProperties": false } */
    CMcpToolDescriptor& SetNoArguments( void );

    /** Writes the Tool object of tools/list into 'node', which becomes an object node */
    bool ToDataNode( CORE::CDataNode& node ) const;

    /** The generated hints */
    bool GetReadOnlyHint( void ) const;
    bool GetDestructiveHint( void ) const;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COM_CMCPTOOLDESCRIPTOR_H ? */
