/*
 *  gucefASSETSTORE: GUCEF module for online asset stores and the primary asset store
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

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */
#include "gucefASSETSTORE_CAssetThumbnail.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CAssetThumbnail::CAssetThumbnail( void )
    : CORE::CIDataNodeSerializable()
    , assetId()
    , key()
    , url()
    , size( 0 )
    , kind()
    , status( ASSETSTORE_STATUS_OK )
    , message()
    , pixels()
    , encoded()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetThumbnail::~CAssetThumbnail()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CAssetThumbnail::Clear( void )
{GUCEF_TRACE;

    *this = CAssetThumbnail();
}

/*-------------------------------------------------------------------------*/

bool
CAssetThumbnail::Serialize( CORE::CDataNode& domRootNode                        ,
                           const CORE::CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    bool success = true;
    success = domRootNode.SetAttribute( "assetId", assetId ) && success;
    success = key.Serialize( *domRootNode.AddChild( "key", GUCEF_DATATYPE_OBJECT ), settings ) && success;
    success = domRootNode.SetAttribute( "url", url ) && success;
    success = domRootNode.SetAttribute( "size", size ) && success;
    success = domRootNode.SetAttribute( "kind", kind ) && success;
    success = domRootNode.SetAttribute( "status", CORE::CString( AssetStoreStatusToString( status ) ) ) && success;
    success = domRootNode.SetAttribute( "message", message ) && success;
    return success;
}

/*-------------------------------------------------------------------------*/

bool
CAssetThumbnail::Deserialize( const CORE::CDataNode& domRootNode                  ,
                             const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    Clear();
    CORE::CString text;
    assetId = domRootNode.GetAttributeValueOrChildValueByName( "assetId" ).AsString( assetId );
    const CORE::CDataNode* keyNode = domRootNode.FindChild( "key" );
    if ( GUCEF_NULL != keyNode )
        key.Deserialize( *keyNode, settings );
    url = domRootNode.GetAttributeValueOrChildValueByName( "url" ).AsString( url );
    size = domRootNode.GetAttributeValueOrChildValueByName( "size" ).AsUInt32( size, true );
    kind = domRootNode.GetAttributeValueOrChildValueByName( "kind" ).AsString( kind );
    text = domRootNode.GetAttributeValueOrChildValueByName( "status" ).AsString();
    if ( !text.IsNULLOrEmpty() )
        status = AssetStoreStatusFromString( text );
    message = domRootNode.GetAttributeValueOrChildValueByName( "message" ).AsString( message );
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CAssetThumbnail::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CAssetThumbnail( *this );
}

/*-------------------------------------------------------------------------*/

const char*
CAssetThumbnail::GetCloneableTypeName( void ) const
{GUCEF_TRACE;

    return CloneableTypeName();
}

/*-------------------------------------------------------------------------*/

const char*
CAssetThumbnail::CloneableTypeName( void )
{GUCEF_TRACE;

    return "GUCEF::ASSETSTORE::CAssetThumbnail";
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CAssetThumbnail::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::ASSETSTORE::CAssetThumbnail";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
