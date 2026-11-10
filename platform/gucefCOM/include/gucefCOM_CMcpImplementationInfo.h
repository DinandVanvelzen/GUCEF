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

#ifndef GUCEF_COM_CMCPIMPLEMENTATIONINFO_H
#define GUCEF_COM_CMCPIMPLEMENTATIONINFO_H

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
 *  The Implementation of the MCP schema: who a server or a client says it is. Self reported and never verified,
 *  so it is used for display, logging and audit only.
 */
class GUCEF_COM_PUBLIC_CPP CMcpImplementationInfo
{
    public:

    CORE::CString name;             /**< required */
    CORE::CString title;
    CORE::CString version;          /**< required */
    CORE::CString description;
    CORE::CString websiteUrl;
    CORE::CDataNode icons;          /**< an array node of Icon objects, written as given; no children: none */

    CMcpImplementationInfo( void );

    /** Writes the JSON object of the Implementation into 'node', which becomes an object node */
    bool ToDataNode( CORE::CDataNode& node ) const;

    /** Reads it from a JSON object; false when it is not an object or name or version is not a string */
    bool FromDataNode( const CORE::CDataNode& node );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COM_CMCPIMPLEMENTATIONINFO_H ? */
