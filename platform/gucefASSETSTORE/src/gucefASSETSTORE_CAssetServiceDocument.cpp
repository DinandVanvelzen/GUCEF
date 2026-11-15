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
#include "gucefASSETSTORE_CAssetServiceDocument.h"

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

CAssetServiceDocument::CAssetServiceDocument( void )
    : CORE::CIDataNodeSerializable()
    , status( ASSETSTORE_STATUS_OK )
    , message()
    , document()
    , nextPageToken()
    , truncated( false )
    , images()
    , changeToken()
    , jobId()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetServiceDocument::~CAssetServiceDocument()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CAssetServiceDocument::Clear( void )
{GUCEF_TRACE;

    *this = CAssetServiceDocument();
}

/*-------------------------------------------------------------------------*/

bool
CAssetServiceDocument::Serialize( CORE::CDataNode& domRootNode                        ,
                                 const CORE::CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    bool success = true;
    success = domRootNode.SetAttribute( "status", CORE::CString( AssetStoreStatusToString( status ) ) ) && success;
    success = domRootNode.SetAttribute( "message", message ) && success;
    CORE::CDataNode* documentNode = domRootNode.AddChild( "document", GUCEF_DATATYPE_OBJECT );
    if ( GUCEF_NULL != documentNode )
    {
        documentNode->Copy( document );
        documentNode->SetName( "document" );
    }
    else
        success = false;
    success = domRootNode.SetAttribute( "nextPageToken", nextPageToken ) && success;
    success = domRootNode.SetAttribute( "truncated", truncated ) && success;
    CORE::CDataNode* imagesNode = domRootNode.AddChild( "images", GUCEF_DATATYPE_ARRAY );
    for ( size_t i=0; GUCEF_NULL != imagesNode && i<images.size(); ++i )
        success = images[ i ].Serialize( *imagesNode->AddChild( "image", GUCEF_DATATYPE_OBJECT ), settings ) && success;
    success = domRootNode.SetAttribute( "changeToken", changeToken ) && success;
    success = domRootNode.SetAttribute( "jobId", jobId ) && success;
    return success;
}

/*-------------------------------------------------------------------------*/

bool
CAssetServiceDocument::Deserialize( const CORE::CDataNode& domRootNode                  ,
                                   const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    Clear();
    CORE::CString text;
    text = domRootNode.GetAttributeValueOrChildValueByName( "status" ).AsString();
    if ( !text.IsNULLOrEmpty() )
        status = AssetStoreStatusFromString( text );
    message = domRootNode.GetAttributeValueOrChildValueByName( "message" ).AsString( message );
    const CORE::CDataNode* documentNode = domRootNode.FindChild( "document" );
    if ( GUCEF_NULL != documentNode )
        document.Copy( *documentNode );
    nextPageToken = domRootNode.GetAttributeValueOrChildValueByName( "nextPageToken" ).AsString( nextPageToken );
    truncated = domRootNode.GetAttributeValueOrChildValueByName( "truncated" ).AsBool( truncated, true );
    const CORE::CDataNode* imagesNode = domRootNode.FindChild( "images" );
    if ( GUCEF_NULL != imagesNode )
    {
        CORE::CDataNode::const_iterator i = imagesNode->ConstBegin();
        while ( i != imagesNode->ConstEnd() )
        {
            CAssetThumbnail entry;
            if ( entry.Deserialize( *(*i), settings ) )
                images.push_back( entry );
            ++i;
        }
    }
    changeToken = domRootNode.GetAttributeValueOrChildValueByName( "changeToken" ).AsString( changeToken );
    jobId = domRootNode.GetAttributeValueOrChildValueByName( "jobId" ).AsString( jobId );
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CAssetServiceDocument::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CAssetServiceDocument( *this );
}

/*-------------------------------------------------------------------------*/

const char*
CAssetServiceDocument::GetCloneableTypeName( void ) const
{GUCEF_TRACE;

    return CloneableTypeName();
}

/*-------------------------------------------------------------------------*/

const char*
CAssetServiceDocument::CloneableTypeName( void )
{GUCEF_TRACE;

    return "GUCEF::ASSETSTORE::CAssetServiceDocument";
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CAssetServiceDocument::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::ASSETSTORE::CAssetServiceDocument";
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
