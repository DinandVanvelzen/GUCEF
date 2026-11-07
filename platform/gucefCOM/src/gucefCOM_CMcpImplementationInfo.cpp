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

#include "gucefCOM_CMcpImplementationInfo.h"

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
SetOptionalString( CORE::CDataNode& node, const char* name, const CORE::CString& value )
{GUCEF_TRACE;

    if ( !value.IsNULLOrEmpty() )
        node.SetAttribute( name, CORE::CVariant( value ) );
}

CORE::CString
GetOptionalString( const CORE::CDataNode& node, const char* name )
{GUCEF_TRACE;

    const CORE::CDataNode::TKeyValuePair* attribute = node.GetAttribute( name );
    if ( GUCEF_NULL != attribute && (*attribute).second.IsString() )
        return (*attribute).second.AsString();
    return CORE::CString::Empty;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CMcpImplementationInfo::CMcpImplementationInfo( void )
    : name()
    , title()
    , version()
    , description()
    , websiteUrl()
    , icons( GUCEF_DATATYPE_ARRAY )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CMcpImplementationInfo::ToDataNode( CORE::CDataNode& node ) const
{GUCEF_TRACE;

    node.Clear();
    node.SetNodeType( GUCEF_DATATYPE_OBJECT );
    node.SetAttribute( "name", CORE::CVariant( name ) );
    SetOptionalString( node, "title", title );
    node.SetAttribute( "version", CORE::CVariant( version ) );
    SetOptionalString( node, "description", description );
    SetOptionalString( node, "websiteUrl", websiteUrl );
    if ( GUCEF_DATATYPE_ARRAY == icons.GetNodeType() && icons.HasChildren() )
        CJsonRpcEndpoint::AddJsonMember( node, "icons", icons );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMcpImplementationInfo::FromDataNode( const CORE::CDataNode& node )
{GUCEF_TRACE;

    if ( GUCEF_DATATYPE_OBJECT != node.GetNodeType() )
        return false;
    const CORE::CDataNode::TKeyValuePair* nameAttribute = node.GetAttribute( "name" );
    const CORE::CDataNode::TKeyValuePair* versionAttribute = node.GetAttribute( "version" );
    if ( GUCEF_NULL == nameAttribute || !(*nameAttribute).second.IsString() ||
         GUCEF_NULL == versionAttribute || !(*versionAttribute).second.IsString() )
        return false;

    name = (*nameAttribute).second.AsString();
    version = (*versionAttribute).second.AsString();
    title = GetOptionalString( node, "title" );
    description = GetOptionalString( node, "description" );
    websiteUrl = GetOptionalString( node, "websiteUrl" );
    const CORE::CDataNode* iconsNode = CJsonRpcEndpoint::FindJsonMemberNode( node, "icons" );
    if ( GUCEF_NULL != iconsNode && GUCEF_DATATYPE_ARRAY == iconsNode->GetNodeType() )
    {
        icons = *iconsNode;
        icons.SetName( CORE::CString::Empty );
    }
    else
    {
        icons = CORE::CDataNode( GUCEF_DATATYPE_ARRAY );
    }
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
