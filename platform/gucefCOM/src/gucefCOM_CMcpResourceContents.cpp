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

#ifndef GUCEF_COM_CJSONRPCENDPOINT_H
#include "gucefCOM_CJsonRpcEndpoint.h"
#define GUCEF_COM_CJSONRPCENDPOINT_H
#endif /* GUCEF_COM_CJSONRPCENDPOINT_H ? */

#include "gucefCOM_CMcpResourceContents.h"

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

CMcpResourceContents::SItem::SItem( void )
    : uri()
    , mimeType()
    , text()
    , blob()
    , isBlob( false )
    , meta( GUCEF_DATATYPE_OBJECT )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMcpResourceContents::CMcpResourceContents( void )
    : CORE::CICloneable()
    , items()
    , hasCacheHints( false )
    , ttlMs( 0 )
    , cacheScope( MCP_CACHESCOPE_PRIVATE )
    , meta( GUCEF_DATATYPE_OBJECT )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMcpResourceContents::CMcpResourceContents( const CMcpResourceContents& src )
    : CORE::CICloneable( src )
    , items( src.items )
    , hasCacheHints( src.hasCacheHints )
    , ttlMs( src.ttlMs )
    , cacheScope( src.cacheScope )
    , meta( src.meta )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMcpResourceContents::~CMcpResourceContents()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMcpResourceContents&
CMcpResourceContents::operator=( const CMcpResourceContents& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        items = src.items;
        hasCacheHints = src.hasCacheHints;
        ttlMs = src.ttlMs;
        cacheScope = src.cacheScope;
        meta = src.meta;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CMcpResourceContents&
CMcpResourceContents::AddText( const CORE::CString& uri, const CORE::CString& mimeType, const CORE::CString& text )
{GUCEF_TRACE;

    SItem item;
    item.uri = uri;
    item.mimeType = mimeType;
    item.text = text;
    items.push_back( item );
    return *this;
}

/*-------------------------------------------------------------------------*/

CMcpResourceContents&
CMcpResourceContents::AddBlob( const CORE::CString& uri, const CORE::CString& mimeType, const CORE::CDynamicBuffer& bytes )
{GUCEF_TRACE;

    SItem item;
    item.uri = uri;
    item.mimeType = mimeType;
    item.blob = bytes;
    item.isBlob = true;
    items.push_back( item );
    return *this;
}

/*-------------------------------------------------------------------------*/

CMcpResourceContents&
CMcpResourceContents::SetCacheHints( UInt32 ttlInMs, EMcpCacheScope scope )
{GUCEF_TRACE;

    hasCacheHints = true;
    ttlMs = ttlInMs;
    cacheScope = scope;
    return *this;
}

/*-------------------------------------------------------------------------*/

void
CMcpResourceContents::Clear( void )
{GUCEF_TRACE;

    items.clear();
    hasCacheHints = false;
    ttlMs = 0;
    cacheScope = MCP_CACHESCOPE_PRIVATE;
    meta = CORE::CDataNode( GUCEF_DATATYPE_OBJECT );
}

/*-------------------------------------------------------------------------*/

bool
CMcpResourceContents::ToDataNode( CORE::CDataNode& result ) const
{GUCEF_TRACE;

    if ( GUCEF_DATATYPE_OBJECT != result.GetNodeType() )
        result.SetNodeType( GUCEF_DATATYPE_OBJECT );

    CORE::CDataNode* contents = result.AddChild( "contents", GUCEF_DATATYPE_ARRAY );
    for ( TItemVector::const_iterator i = items.begin(); i != items.end(); ++i )
    {
        const SItem& item = (*i);
        CORE::CDataNode* node = contents->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT );
        node->SetAttribute( "uri", CORE::CVariant( item.uri ) );
        if ( !item.mimeType.IsNULLOrEmpty() )
            node->SetAttribute( "mimeType", CORE::CVariant( item.mimeType ) );
        if ( item.isBlob )
        {
            CORE::CString blob;
            if ( item.blob.GetDataSize() > 0 )
                blob = CORE::Base64Encode( item.blob.GetConstBufferPtr(), item.blob.GetDataSize() );
            node->SetAttribute( "blob", CORE::CVariant( blob ) );
        }
        else
        {
            node->SetAttribute( "text", CORE::CVariant( item.text ) );
        }
        if ( item.meta.GetAttCount() > 0 || item.meta.HasChildren() )
            CJsonRpcEndpoint::AddJsonMember( *node, "_meta", item.meta );
    }
    if ( meta.GetAttCount() > 0 || meta.HasChildren() )
        CJsonRpcEndpoint::AddJsonMember( result, "_meta", meta );
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CMcpResourceContents::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CMcpResourceContents( *this );
}

/*-------------------------------------------------------------------------*/

const char*
CMcpResourceContents::GetCloneableTypeName( void ) const
{GUCEF_TRACE;

    return CloneableTypeName();
}

/*-------------------------------------------------------------------------*/

const char*
CMcpResourceContents::CloneableTypeName( void )
{GUCEF_TRACE;

    return "GUCEF::COM::CMcpResourceContents";
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
