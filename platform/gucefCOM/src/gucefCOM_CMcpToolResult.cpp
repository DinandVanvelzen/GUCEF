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

#include "gucefCOM_CMcpToolResult.h"

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

CMcpToolResult::CMcpToolResult( void )
    : CORE::CICloneable()
    , content()
    , structuredContent()
    , hasStructuredContent( false )
    , isError( false )
    , meta( GUCEF_DATATYPE_OBJECT )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMcpToolResult::CMcpToolResult( const CMcpToolResult& src )
    : CORE::CICloneable( src )
    , content( src.content )
    , structuredContent( src.structuredContent )
    , hasStructuredContent( src.hasStructuredContent )
    , isError( src.isError )
    , meta( src.meta )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMcpToolResult::~CMcpToolResult()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMcpToolResult&
CMcpToolResult::operator=( const CMcpToolResult& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        content = src.content;
        structuredContent = src.structuredContent;
        hasStructuredContent = src.hasStructuredContent;
        isError = src.isError;
        meta = src.meta;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CMcpToolResult&
CMcpToolResult::AddText( const CORE::CString& text )
{GUCEF_TRACE;

    content.push_back( CMcpContent::Text( text ) );
    return *this;
}

/*-------------------------------------------------------------------------*/

CMcpToolResult&
CMcpToolResult::AddContent( const CMcpContent& block )
{GUCEF_TRACE;

    content.push_back( block );
    return *this;
}

/*-------------------------------------------------------------------------*/

CMcpToolResult&
CMcpToolResult::SetStructuredContent( const CORE::CDataNode& value, bool alsoAsText )
{GUCEF_TRACE;

    structuredContent = value;
    structuredContent.SetName( CORE::CString::Empty );
    hasStructuredContent = true;
    if ( alsoAsText )
    {
        CORE::CString jsonText;
        if ( CJsonRpcEndpoint::ToJsonText( structuredContent, jsonText ) )
            AddText( jsonText );
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CMcpToolResult&
CMcpToolResult::SetError( const CORE::CString& messageForTheModel )
{GUCEF_TRACE;

    isError = true;
    return AddText( messageForTheModel );
}

/*-------------------------------------------------------------------------*/

void
CMcpToolResult::Clear( void )
{GUCEF_TRACE;

    content.clear();
    structuredContent = CORE::CDataNode();
    hasStructuredContent = false;
    isError = false;
    meta = CORE::CDataNode( GUCEF_DATATYPE_OBJECT );
}

/*-------------------------------------------------------------------------*/

bool
CMcpToolResult::ToDataNode( CORE::CDataNode& result ) const
{GUCEF_TRACE;

    if ( GUCEF_DATATYPE_OBJECT != result.GetNodeType() )
        result.SetNodeType( GUCEF_DATATYPE_OBJECT );

    CORE::CDataNode* contentNode = result.AddChild( "content", GUCEF_DATATYPE_ARRAY );
    for ( TContentVector::const_iterator i = content.begin(); i != content.end(); ++i )
    {
        CORE::CDataNode* block = contentNode->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT );
        if ( !(*i).ToDataNode( *block ) )
            return false;
    }
    if ( hasStructuredContent )
        CJsonRpcEndpoint::AddJsonMember( result, "structuredContent", structuredContent );
    result.SetAttribute( "isError", isError );
    if ( meta.GetAttCount() > 0 || meta.HasChildren() )
        CJsonRpcEndpoint::AddJsonMember( result, "_meta", meta );
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CMcpToolResult::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CMcpToolResult( *this );
}

/*-------------------------------------------------------------------------*/

const char*
CMcpToolResult::GetCloneableTypeName( void ) const
{GUCEF_TRACE;

    return CloneableTypeName();
}

/*-------------------------------------------------------------------------*/

const char*
CMcpToolResult::CloneableTypeName( void )
{GUCEF_TRACE;

    return "GUCEF::COM::CMcpToolResult";
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
