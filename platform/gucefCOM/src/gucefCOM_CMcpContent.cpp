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

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#include "gucefCOM_CMcpContent.h"

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

CORE::CString
ToBase64( const CORE::CDynamicBuffer& bytes )
{GUCEF_TRACE;

    if ( 0 == bytes.GetDataSize() )
        return CORE::CString::Empty;
    return CORE::Base64Encode( bytes.GetConstBufferPtr(), bytes.GetDataSize() );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CMcpAnnotations::CMcpAnnotations( void )
    : hasAudience( false )
    , audienceUser( false )
    , audienceAssistant( false )
    , hasPriority( false )
    , priority( 0.0 )
    , lastModified()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CMcpAnnotations::IsEmpty( void ) const
{GUCEF_TRACE;

    return !hasAudience && !hasPriority && lastModified.IsNULLOrEmpty();
}

/*-------------------------------------------------------------------------*/

void
CMcpAnnotations::ToDataNode( CORE::CDataNode& node ) const
{GUCEF_TRACE;

    node.Clear();
    node.SetNodeType( GUCEF_DATATYPE_OBJECT );
    if ( hasAudience )
    {
        CORE::CDataNode* audience = node.AddChild( "audience", GUCEF_DATATYPE_ARRAY );
        if ( audienceUser )
            audience->AddValueAsChild( CORE::CVariant( "user" ) );
        if ( audienceAssistant )
            audience->AddValueAsChild( CORE::CVariant( "assistant" ) );
    }
    if ( hasPriority )
        node.SetAttribute( "priority", CORE::CVariant( priority ) );
    if ( !lastModified.IsNULLOrEmpty() )
        node.SetAttribute( "lastModified", CORE::CVariant( lastModified ) );
}

/*-------------------------------------------------------------------------*/

void
CMcpAnnotations::AddToObject( CORE::CDataNode& object ) const
{GUCEF_TRACE;

    if ( IsEmpty() )
        return;
    CORE::CDataNode* node = object.AddChild( "annotations", GUCEF_DATATYPE_OBJECT );
    ToDataNode( *node );
    node->SetName( "annotations" );
}

/*-------------------------------------------------------------------------*/

CMcpContent::CMcpContent( void )
    : type( MCP_CONTENT_TEXT )
    , text()
    , mimeType()
    , data()
    , isBlob( false )
    , uri()
    , name()
    , title()
    , description()
    , hasSize( false )
    , size( 0 )
    , annotations()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMcpContent
CMcpContent::Text( const CORE::CString& text )
{GUCEF_TRACE;

    CMcpContent content;
    content.type = MCP_CONTENT_TEXT;
    content.text = text;
    return content;
}

/*-------------------------------------------------------------------------*/

CMcpContent
CMcpContent::Image( const CORE::CDynamicBuffer& bytes, const CORE::CString& mimeType )
{GUCEF_TRACE;

    CMcpContent content;
    content.type = MCP_CONTENT_IMAGE;
    content.data = bytes;
    content.mimeType = mimeType;
    return content;
}

/*-------------------------------------------------------------------------*/

CMcpContent
CMcpContent::Audio( const CORE::CDynamicBuffer& bytes, const CORE::CString& mimeType )
{GUCEF_TRACE;

    CMcpContent content;
    content.type = MCP_CONTENT_AUDIO;
    content.data = bytes;
    content.mimeType = mimeType;
    return content;
}

/*-------------------------------------------------------------------------*/

CMcpContent
CMcpContent::ResourceLink( const CORE::CString& uri, const CORE::CString& name, const CORE::CString& mimeType )
{GUCEF_TRACE;

    CMcpContent content;
    content.type = MCP_CONTENT_RESOURCE_LINK;
    content.uri = uri;
    content.name = name;
    content.mimeType = mimeType;
    return content;
}

/*-------------------------------------------------------------------------*/

CMcpContent
CMcpContent::EmbeddedText( const CORE::CString& uri, const CORE::CString& mimeType, const CORE::CString& text )
{GUCEF_TRACE;

    CMcpContent content;
    content.type = MCP_CONTENT_RESOURCE;
    content.uri = uri;
    content.mimeType = mimeType;
    content.text = text;
    return content;
}

/*-------------------------------------------------------------------------*/

CMcpContent
CMcpContent::EmbeddedBlob( const CORE::CString& uri, const CORE::CString& mimeType, const CORE::CDynamicBuffer& bytes )
{GUCEF_TRACE;

    CMcpContent content;
    content.type = MCP_CONTENT_RESOURCE;
    content.uri = uri;
    content.mimeType = mimeType;
    content.data = bytes;
    content.isBlob = true;
    return content;
}

/*-------------------------------------------------------------------------*/

CMcpContent&
CMcpContent::SetAudience( bool user, bool assistant )
{GUCEF_TRACE;

    annotations.hasAudience = true;
    annotations.audienceUser = user;
    annotations.audienceAssistant = assistant;
    return *this;
}

/*-------------------------------------------------------------------------*/

CMcpContent&
CMcpContent::SetPriority( Float64 newPriority )
{GUCEF_TRACE;

    annotations.hasPriority = true;
    annotations.priority = newPriority < 0.0 ? 0.0 : ( newPriority > 1.0 ? 1.0 : newPriority );
    return *this;
}

/*-------------------------------------------------------------------------*/

CMcpContent&
CMcpContent::SetLastModified( const CORE::CString& iso8601 )
{GUCEF_TRACE;

    annotations.lastModified = iso8601;
    return *this;
}

/*-------------------------------------------------------------------------*/

bool
CMcpContent::ToDataNode( CORE::CDataNode& node ) const
{GUCEF_TRACE;

    node.Clear();
    node.SetNodeType( GUCEF_DATATYPE_OBJECT );
    switch ( type )
    {
        case MCP_CONTENT_TEXT:
        {
            node.SetAttribute( "type", CORE::CVariant( "text" ) );
            node.SetAttribute( "text", CORE::CVariant( text ) );
            break;
        }
        case MCP_CONTENT_IMAGE:
        case MCP_CONTENT_AUDIO:
        {
            node.SetAttribute( "type", CORE::CVariant( MCP_CONTENT_IMAGE == type ? "image" : "audio" ) );
            node.SetAttribute( "data", CORE::CVariant( ToBase64( data ) ) );
            node.SetAttribute( "mimeType", CORE::CVariant( mimeType ) );
            break;
        }
        case MCP_CONTENT_RESOURCE_LINK:
        {
            node.SetAttribute( "type", CORE::CVariant( "resource_link" ) );
            node.SetAttribute( "uri", CORE::CVariant( uri ) );
            node.SetAttribute( "name", CORE::CVariant( name ) );
            if ( !title.IsNULLOrEmpty() )
                node.SetAttribute( "title", CORE::CVariant( title ) );
            if ( !description.IsNULLOrEmpty() )
                node.SetAttribute( "description", CORE::CVariant( description ) );
            if ( !mimeType.IsNULLOrEmpty() )
                node.SetAttribute( "mimeType", CORE::CVariant( mimeType ) );
            if ( hasSize )
                node.SetAttribute( "size", CORE::CVariant( size ) );
            break;
        }
        case MCP_CONTENT_RESOURCE:
        {
            node.SetAttribute( "type", CORE::CVariant( "resource" ) );
            CORE::CDataNode* resource = node.AddChild( "resource", GUCEF_DATATYPE_OBJECT );
            resource->SetAttribute( "uri", CORE::CVariant( uri ) );
            if ( !mimeType.IsNULLOrEmpty() )
                resource->SetAttribute( "mimeType", CORE::CVariant( mimeType ) );
            if ( isBlob )
                resource->SetAttribute( "blob", CORE::CVariant( ToBase64( data ) ) );
            else
                resource->SetAttribute( "text", CORE::CVariant( text ) );
            break;
        }
        default:
        {
            return false;
        }
    }
    annotations.AddToObject( node );
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
