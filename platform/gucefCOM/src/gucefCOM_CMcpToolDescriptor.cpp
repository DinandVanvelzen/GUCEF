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

#include "gucefCOM_CMcpToolDescriptor.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COM {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CMcpToolDescriptor::CMcpToolDescriptor( void )
    : name()
    , title()
    , description()
    , icons( GUCEF_DATATYPE_ARRAY )
    , inputSchema( GUCEF_DATATYPE_OBJECT )
    , outputSchema( GUCEF_DATATYPE_OBJECT )
    , hasOutputSchema( false )
    , effect( MCP_TOOLEFFECT_READ_ONLY )
    , idempotent( false )
    , openWorld( false )
    , maxDurationMs( 0 )
    , validateOutput( true )
    , secretArguments()
    , meta( GUCEF_DATATYPE_OBJECT )
{GUCEF_TRACE;

    inputSchema.SetAttribute( "type", CORE::CVariant( "object" ) );
}

/*-------------------------------------------------------------------------*/

CMcpToolDescriptor&
CMcpToolDescriptor::SetNoArguments( void )
{GUCEF_TRACE;

    inputSchema = CORE::CDataNode( GUCEF_DATATYPE_OBJECT );
    inputSchema.SetAttribute( "type", CORE::CVariant( "object" ) );
    inputSchema.SetAttribute( "additionalProperties", false );
    return *this;
}

/*-------------------------------------------------------------------------*/

bool
CMcpToolDescriptor::GetReadOnlyHint( void ) const
{GUCEF_TRACE;

    return MCP_TOOLEFFECT_READ_ONLY == effect;
}

/*-------------------------------------------------------------------------*/

bool
CMcpToolDescriptor::GetDestructiveHint( void ) const
{GUCEF_TRACE;

    return MCP_TOOLEFFECT_DESTRUCTIVE == effect;
}

/*-------------------------------------------------------------------------*/

bool
CMcpToolDescriptor::ToDataNode( CORE::CDataNode& node ) const
{GUCEF_TRACE;

    node.Clear();
    node.SetNodeType( GUCEF_DATATYPE_OBJECT );
    node.SetAttribute( "name", CORE::CVariant( name ) );
    if ( !title.IsNULLOrEmpty() )
        node.SetAttribute( "title", CORE::CVariant( title ) );
    if ( !description.IsNULLOrEmpty() )
        node.SetAttribute( "description", CORE::CVariant( description ) );
    if ( GUCEF_DATATYPE_ARRAY == icons.GetNodeType() && icons.HasChildren() )
        CJsonRpcEndpoint::AddJsonMember( node, "icons", icons );
    CJsonRpcEndpoint::AddJsonMember( node, "inputSchema", inputSchema );
    if ( hasOutputSchema )
        CJsonRpcEndpoint::AddJsonMember( node, "outputSchema", outputSchema );

    // All four hints are written: the defaults of the schema (destructiveHint and openWorldHint true) must not apply
    CORE::CDataNode* annotations = node.AddChild( "annotations", GUCEF_DATATYPE_OBJECT );
    annotations->SetAttribute( "readOnlyHint", GetReadOnlyHint() );
    annotations->SetAttribute( "destructiveHint", GetDestructiveHint() );
    annotations->SetAttribute( "idempotentHint", idempotent );
    annotations->SetAttribute( "openWorldHint", openWorld );

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
