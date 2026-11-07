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

#include "gucefCOM_CMcpPromptResult.h"

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

CMcpPromptResult::SMessage::SMessage( void )
    : role( MCP_ROLE_USER )
    , content()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMcpPromptResult::SMessage::SMessage( ERole messageRole, const CMcpContent& messageContent )
    : role( messageRole )
    , content( messageContent )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMcpPromptResult::CMcpPromptResult( void )
    : CORE::CICloneable()
    , description()
    , messages()
    , meta( GUCEF_DATATYPE_OBJECT )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMcpPromptResult::CMcpPromptResult( const CMcpPromptResult& src )
    : CORE::CICloneable( src )
    , description( src.description )
    , messages( src.messages )
    , meta( src.meta )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMcpPromptResult::~CMcpPromptResult()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMcpPromptResult&
CMcpPromptResult::operator=( const CMcpPromptResult& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        description = src.description;
        messages = src.messages;
        meta = src.meta;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CMcpPromptResult&
CMcpPromptResult::AddMessage( ERole role, const CMcpContent& content )
{GUCEF_TRACE;

    messages.push_back( SMessage( role, content ) );
    return *this;
}

/*-------------------------------------------------------------------------*/

CMcpPromptResult&
CMcpPromptResult::AddUserText( const CORE::CString& text )
{GUCEF_TRACE;

    return AddMessage( MCP_ROLE_USER, CMcpContent::Text( text ) );
}

/*-------------------------------------------------------------------------*/

CMcpPromptResult&
CMcpPromptResult::AddAssistantText( const CORE::CString& text )
{GUCEF_TRACE;

    return AddMessage( MCP_ROLE_ASSISTANT, CMcpContent::Text( text ) );
}

/*-------------------------------------------------------------------------*/

void
CMcpPromptResult::Clear( void )
{GUCEF_TRACE;

    description.Clear();
    messages.clear();
    meta = CORE::CDataNode( GUCEF_DATATYPE_OBJECT );
}

/*-------------------------------------------------------------------------*/

bool
CMcpPromptResult::ToDataNode( CORE::CDataNode& result ) const
{GUCEF_TRACE;

    if ( GUCEF_DATATYPE_OBJECT != result.GetNodeType() )
        result.SetNodeType( GUCEF_DATATYPE_OBJECT );

    if ( !description.IsNULLOrEmpty() )
        result.SetAttribute( "description", CORE::CVariant( description ) );
    CORE::CDataNode* messagesNode = result.AddChild( "messages", GUCEF_DATATYPE_ARRAY );
    for ( TMessageVector::const_iterator i = messages.begin(); i != messages.end(); ++i )
    {
        CORE::CDataNode* message = messagesNode->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT );
        message->SetAttribute( "role", CORE::CVariant( MCP_ROLE_USER == (*i).role ? "user" : "assistant" ) );
        CORE::CDataNode* content = message->AddChild( "content", GUCEF_DATATYPE_OBJECT );
        if ( !(*i).content.ToDataNode( *content ) )
            return false;
        content->SetName( "content" );
    }
    if ( meta.GetAttCount() > 0 || meta.HasChildren() )
        CJsonRpcEndpoint::AddJsonMember( result, "_meta", meta );
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CMcpPromptResult::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CMcpPromptResult( *this );
}

/*-------------------------------------------------------------------------*/

const char*
CMcpPromptResult::GetCloneableTypeName( void ) const
{GUCEF_TRACE;

    return CloneableTypeName();
}

/*-------------------------------------------------------------------------*/

const char*
CMcpPromptResult::CloneableTypeName( void )
{GUCEF_TRACE;

    return "GUCEF::COM::CMcpPromptResult";
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
