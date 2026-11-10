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

#ifndef GUCEF_COM_CJSONRPCENDPOINT_H
#include "gucefCOM_CJsonRpcEndpoint.h"
#define GUCEF_COM_CJSONRPCENDPOINT_H
#endif /* GUCEF_COM_CJSONRPCENDPOINT_H ? */

#include "gucefCOM_CMcpResourceDescriptor.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COM {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

void
AddCommonMembers( CORE::CDataNode& node              ,
                  const CORE::CString& title         ,
                  const CORE::CString& description   ,
                  const CORE::CString& mimeType      ,
                  const CMcpAnnotations& annotations ,
                  const CORE::CDataNode& icons       )
{GUCEF_TRACE;

    if ( !title.IsNULLOrEmpty() )
        node.SetAttribute( "title", CORE::CVariant( title ) );
    if ( !description.IsNULLOrEmpty() )
        node.SetAttribute( "description", CORE::CVariant( description ) );
    if ( !mimeType.IsNULLOrEmpty() )
        node.SetAttribute( "mimeType", CORE::CVariant( mimeType ) );
    annotations.AddToObject( node );
    if ( GUCEF_DATATYPE_ARRAY == icons.GetNodeType() && icons.HasChildren() )
        CJsonRpcEndpoint::AddJsonMember( node, "icons", icons );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CMcpResourceDescriptor::CMcpResourceDescriptor( void )
    : uri()
    , name()
    , title()
    , description()
    , mimeType()
    , hasSize( false )
    , size( 0 )
    , annotations()
    , icons( GUCEF_DATATYPE_ARRAY )
    , meta( GUCEF_DATATYPE_OBJECT )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CMcpResourceDescriptor::ToDataNode( CORE::CDataNode& node ) const
{GUCEF_TRACE;

    node.Clear();
    node.SetNodeType( GUCEF_DATATYPE_OBJECT );
    node.SetAttribute( "uri", CORE::CVariant( uri ) );
    node.SetAttribute( "name", CORE::CVariant( name ) );
    AddCommonMembers( node, title, description, mimeType, annotations, icons );
    if ( hasSize )
        node.SetAttribute( "size", CORE::CVariant( size ) );
    if ( meta.GetAttCount() > 0 || meta.HasChildren() )
        CJsonRpcEndpoint::AddJsonMember( node, "_meta", meta );
    return true;
}

/*-------------------------------------------------------------------------*/

CMcpResourceTemplateDescriptor::CMcpResourceTemplateDescriptor( void )
    : name()
    , uriTemplate()
    , title()
    , description()
    , mimeType()
    , annotations()
    , icons( GUCEF_DATATYPE_ARRAY )
    , meta( GUCEF_DATATYPE_OBJECT )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CMcpResourceTemplateDescriptor::ToDataNode( CORE::CDataNode& node ) const
{GUCEF_TRACE;

    node.Clear();
    node.SetNodeType( GUCEF_DATATYPE_OBJECT );
    node.SetAttribute( "uriTemplate", CORE::CVariant( uriTemplate ) );
    node.SetAttribute( "name", CORE::CVariant( name ) );
    AddCommonMembers( node, title, description, mimeType, annotations, icons );
    if ( meta.GetAttCount() > 0 || meta.HasChildren() )
        CJsonRpcEndpoint::AddJsonMember( node, "_meta", meta );
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
