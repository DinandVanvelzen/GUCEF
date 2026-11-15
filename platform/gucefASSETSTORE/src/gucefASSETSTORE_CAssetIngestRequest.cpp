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
#include "gucefASSETSTORE_CAssetIngestRequest.h"

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

CAssetIngestRequest::CAssetIngestRequest( void )
    : CORE::CIDataNodeSerializable()
    , sourceId()
    , itemId()
    , item()
    , contentPath()
    , copyContent( -1 )
    , keep()
    , locationId()
    , duplicatePolicy()
    , downloadJobId()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetIngestRequest::~CAssetIngestRequest()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CAssetIngestRequest::Serialize( CORE::CDataNode& domRootNode                        ,
                                const CORE::CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    bool success = domRootNode.SetAttribute( "sourceId", sourceId );
    success = domRootNode.SetAttribute( "itemId", itemId ) && success;
    success = item.Serialize( *domRootNode.AddChild( "item", GUCEF_DATATYPE_OBJECT ), settings ) && success;
    success = domRootNode.SetAttribute( "contentPath", contentPath ) && success;
    success = domRootNode.SetAttribute( "copyContent", copyContent ) && success;
    success = domRootNode.SetAttribute( "keep", keep ) && success;
    success = domRootNode.SetAttribute( "locationId", locationId ) && success;
    success = domRootNode.SetAttribute( "duplicatePolicy", duplicatePolicy ) && success;
    success = domRootNode.SetAttribute( "downloadJobId", downloadJobId ) && success;
    return success;
}

/*-------------------------------------------------------------------------*/

bool
CAssetIngestRequest::Deserialize( const CORE::CDataNode& domRootNode                  ,
                                  const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    sourceId = domRootNode.GetAttributeValueOrChildValueByName( "sourceId" ).AsString();
    itemId = domRootNode.GetAttributeValueOrChildValueByName( "itemId" ).AsString();
    const CORE::CDataNode* itemNode = domRootNode.FindChild( "item" );
    if ( GUCEF_NULL != itemNode )
        item.Deserialize( *itemNode, settings );
    contentPath = domRootNode.GetAttributeValueOrChildValueByName( "contentPath" ).AsString();
    copyContent = domRootNode.GetAttributeValueOrChildValueByName( "copyContent" ).AsInt32( -1, true );
    keep = domRootNode.GetAttributeValueOrChildValueByName( "keep" ).AsString();
    locationId = domRootNode.GetAttributeValueOrChildValueByName( "locationId" ).AsString();
    duplicatePolicy = domRootNode.GetAttributeValueOrChildValueByName( "duplicatePolicy" ).AsString();
    downloadJobId = domRootNode.GetAttributeValueOrChildValueByName( "downloadJobId" ).AsString();
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CAssetIngestRequest::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CAssetIngestRequest( *this );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CAssetIngestRequest::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::ASSETSTORE::CAssetIngestRequest";
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
