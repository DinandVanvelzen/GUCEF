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

#ifndef GUCEF_COM_CMCPRESOURCEDESCRIPTOR_H
#define GUCEF_COM_CMCPRESOURCEDESCRIPTOR_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_COM_CMCPCONTENT_H
#include "gucefCOM_CMcpContent.h"
#define GUCEF_COM_CMCPCONTENT_H
#endif /* GUCEF_COM_CMCPCONTENT_H ? */

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

/** The metadata of a fixed resource: the Resource object of resources/list */
class GUCEF_COM_PUBLIC_CPP CMcpResourceDescriptor
{
    public:

    CORE::CString uri;                  /**< unique, an RFC 3986 URI */
    CORE::CString name;                 /**< required */
    CORE::CString title;
    CORE::CString description;
    CORE::CString mimeType;
    bool hasSize;
    UInt64 size;                        /**< in bytes */
    CMcpAnnotations annotations;
    CORE::CDataNode icons;              /**< an array node of Icon objects, written as given */
    CORE::CDataNode meta;               /**< the _meta of the Resource, an object node, written when it has members */

    CMcpResourceDescriptor( void );

    bool ToDataNode( CORE::CDataNode& node ) const;
};

/*-------------------------------------------------------------------------*/

/** The metadata of a resource template: the ResourceTemplate object of resources/templates/list */
class GUCEF_COM_PUBLIC_CPP CMcpResourceTemplateDescriptor
{
    public:

    CORE::CString name;                 /**< unique among the templates, required */
    CORE::CString uriTemplate;          /**< RFC 6570; {var} and {+var} expressions only */
    CORE::CString title;
    CORE::CString description;
    CORE::CString mimeType;
    CMcpAnnotations annotations;
    CORE::CDataNode icons;
    CORE::CDataNode meta;

    CMcpResourceTemplateDescriptor( void );

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

#endif /* GUCEF_COM_CMCPRESOURCEDESCRIPTOR_H ? */
