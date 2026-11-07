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

#ifndef GUCEF_COM_CMCPPROMPTDESCRIPTOR_H
#define GUCEF_COM_CMCPPROMPTDESCRIPTOR_H

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

/** The metadata of a prompt: the Prompt object of prompts/list */
class GUCEF_COM_PUBLIC_CPP CMcpPromptDescriptor
{
    public:

    struct GUCEF_COM_PUBLIC_CPP SArgument
    {
        CORE::CString name;
        CORE::CString title;
        CORE::CString description;
        bool required;              /**< a prompts/get without it is refused with -32602 before the handler runs */

        SArgument( void );
        SArgument( const CORE::CString& argumentName, const CORE::CString& argumentDescription, bool isRequired );
    };
    typedef GUCEF::vector< SArgument > TArgumentVector;

    CORE::CString name;             /**< unique, required */
    CORE::CString title;
    CORE::CString description;
    TArgumentVector arguments;
    CORE::CDataNode icons;
    CORE::CDataNode meta;

    CMcpPromptDescriptor( void );

    CMcpPromptDescriptor& AddArgument( const CORE::CString& argumentName, const CORE::CString& argumentDescription, bool isRequired );

    bool ToDataNode( CORE::CDataNode& node ) const;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COM_CMCPPROMPTDESCRIPTOR_H ? */
