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

#include "gucefCOM_CMcpPromptDescriptor.h"

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

CMcpPromptDescriptor::SArgument::SArgument( void )
    : name()
    , title()
    , description()
    , required( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMcpPromptDescriptor::SArgument::SArgument( const CORE::CString& argumentName, const CORE::CString& argumentDescription, bool isRequired )
    : name( argumentName )
    , title()
    , description( argumentDescription )
    , required( isRequired )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMcpPromptDescriptor::CMcpPromptDescriptor( void )
    : name()
    , title()
    , description()
    , arguments()
    , icons( GUCEF_DATATYPE_ARRAY )
    , meta( GUCEF_DATATYPE_OBJECT )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMcpPromptDescriptor&
CMcpPromptDescriptor::AddArgument( const CORE::CString& argumentName, const CORE::CString& argumentDescription, bool isRequired )
{GUCEF_TRACE;

    arguments.push_back( SArgument( argumentName, argumentDescription, isRequired ) );
    return *this;
}

/*-------------------------------------------------------------------------*/

bool
CMcpPromptDescriptor::ToDataNode( CORE::CDataNode& node ) const
{GUCEF_TRACE;

    node.Clear();
    node.SetNodeType( GUCEF_DATATYPE_OBJECT );
    node.SetAttribute( "name", CORE::CVariant( name ) );
    if ( !title.IsNULLOrEmpty() )
        node.SetAttribute( "title", CORE::CVariant( title ) );
    if ( !description.IsNULLOrEmpty() )
        node.SetAttribute( "description", CORE::CVariant( description ) );
    if ( !arguments.empty() )
    {
        CORE::CDataNode* argumentsNode = node.AddChild( "arguments", GUCEF_DATATYPE_ARRAY );
        for ( TArgumentVector::const_iterator i = arguments.begin(); i != arguments.end(); ++i )
        {
            CORE::CDataNode* argument = argumentsNode->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT );
            argument->SetAttribute( "name", CORE::CVariant( (*i).name ) );
            if ( !(*i).title.IsNULLOrEmpty() )
                argument->SetAttribute( "title", CORE::CVariant( (*i).title ) );
            if ( !(*i).description.IsNULLOrEmpty() )
                argument->SetAttribute( "description", CORE::CVariant( (*i).description ) );
            argument->SetAttribute( "required", (*i).required );
        }
    }
    if ( GUCEF_DATATYPE_ARRAY == icons.GetNodeType() && icons.HasChildren() )
        CJsonRpcEndpoint::AddJsonMember( node, "icons", icons );
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
