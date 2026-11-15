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

#include "gucefASSETSTORE_CAssetStoreItem.h"

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

CAssetStoreItem::CAssetStoreItem( void )
    : CORE::CIDataNodeSerializable()
    , providerId()
    , itemId()
    , version()
    , title()
    , seller()
    , listingType()
    , url()
    , thumbnailUrl()
    , imageUrls()
    , description()
    , licenceName()
    , licenceUrl()
    , licenceText()
    , licenceRestrictions()
    , categories()
    , tags()
    , formats()
    , storeAttributes( "storeAttributes", GUCEF_DATATYPE_OBJECT )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetStoreItem::~CAssetStoreItem()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

const CAssetStoreFormat*
CAssetStoreItem::FindFormat( const CORE::CString& formatId ) const
{GUCEF_TRACE;

    for ( size_t i=0; i<formats.size(); ++i )
        if ( formats[ i ].formatId == formatId )
            return &formats[ i ];
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

bool
CAssetStoreItem::Serialize( CORE::CDataNode& domRootNode                        ,
                            const CORE::CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    bool success = domRootNode.SetAttribute( "providerId", providerId );
    success = domRootNode.SetAttribute( "itemId", itemId ) && success;
    success = domRootNode.SetAttribute( "version", version ) && success;
    success = domRootNode.SetAttribute( "title", title ) && success;
    success = domRootNode.SetAttribute( "seller", seller ) && success;
    success = domRootNode.SetAttribute( "listingType", listingType ) && success;
    success = domRootNode.SetAttribute( "url", url ) && success;
    success = domRootNode.SetAttribute( "thumbnailUrl", thumbnailUrl ) && success;
    success = domRootNode.SetValuesOfChildByName( "imageUrls", imageUrls ) && success;
    success = domRootNode.SetAttribute( "description", description ) && success;
    success = domRootNode.SetAttribute( "licenceName", licenceName ) && success;
    success = domRootNode.SetAttribute( "licenceUrl", licenceUrl ) && success;
    success = domRootNode.SetAttribute( "licenceText", licenceText ) && success;
    success = domRootNode.SetValuesOfChildByName( "licenceRestrictions", licenceRestrictions ) && success;
    success = domRootNode.SetValuesOfChildByName( "categories", categories ) && success;
    success = domRootNode.SetValuesOfChildByName( "tags", tags ) && success;
    CORE::CDataNode* formatsNode = domRootNode.AddChild( "formats", GUCEF_DATATYPE_ARRAY );
    if ( GUCEF_NULL == formatsNode )
        return false;
    for ( size_t i=0; i<formats.size(); ++i )
    {
        CORE::CDataNode* formatNode = formatsNode->AddChild( "format", GUCEF_DATATYPE_OBJECT );
        success = GUCEF_NULL != formatNode && formats[ i ].Serialize( *formatNode, settings ) && success;
    }
    if ( storeAttributes.HasChildren() || storeAttributes.GetAttCount() > 0 )
    {
        CORE::CDataNode* attributesNode = domRootNode.AddChild( storeAttributes );
        if ( GUCEF_NULL != attributesNode )
            attributesNode->SetName( "storeAttributes" );
    }
    return success;
}

/*-------------------------------------------------------------------------*/

bool
CAssetStoreItem::Deserialize( const CORE::CDataNode& domRootNode                  ,
                              const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    providerId = domRootNode.GetAttributeValueOrChildValueByName( "providerId" ).AsString();
    itemId = domRootNode.GetAttributeValueOrChildValueByName( "itemId" ).AsString();
    version = domRootNode.GetAttributeValueOrChildValueByName( "version" ).AsString();
    title = domRootNode.GetAttributeValueOrChildValueByName( "title" ).AsString();
    seller = domRootNode.GetAttributeValueOrChildValueByName( "seller" ).AsString();
    listingType = domRootNode.GetAttributeValueOrChildValueByName( "listingType" ).AsString();
    url = domRootNode.GetAttributeValueOrChildValueByName( "url" ).AsString();
    thumbnailUrl = domRootNode.GetAttributeValueOrChildValueByName( "thumbnailUrl" ).AsString();
    domRootNode.GetValuesOfChildByName( "imageUrls", imageUrls );
    description = domRootNode.GetAttributeValueOrChildValueByName( "description" ).AsString();
    licenceName = domRootNode.GetAttributeValueOrChildValueByName( "licenceName" ).AsString();
    licenceUrl = domRootNode.GetAttributeValueOrChildValueByName( "licenceUrl" ).AsString();
    licenceText = domRootNode.GetAttributeValueOrChildValueByName( "licenceText" ).AsString();
    licenceRestrictions.clear();
    domRootNode.GetValuesOfChildByName( "licenceRestrictions", licenceRestrictions );
    domRootNode.GetValuesOfChildByName( "categories", categories );
    domRootNode.GetValuesOfChildByName( "tags", tags );
    formats.clear();
    const CORE::CDataNode* formatsNode = domRootNode.FindChild( "formats" );
    if ( GUCEF_NULL != formatsNode )
    {
        CORE::CDataNode::const_iterator i = formatsNode->ConstBegin();
        while ( i != formatsNode->ConstEnd() )
        {
            CAssetStoreFormat format;
            if ( format.Deserialize( *(*i), settings ) )
                formats.push_back( format );
            ++i;
        }
    }
    storeAttributes.Clear();
    storeAttributes.SetName( "storeAttributes" );
    const CORE::CDataNode* attributesNode = domRootNode.FindChild( "storeAttributes" );
    if ( GUCEF_NULL != attributesNode )
    {
        storeAttributes.Copy( *attributesNode );
        storeAttributes.SetName( "storeAttributes" );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CAssetStoreItem::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CAssetStoreItem( *this );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CAssetStoreItem::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::ASSETSTORE::CAssetStoreItem";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

const char*
CAssetStoreItem::GetCloneableTypeName( void ) const
{GUCEF_TRACE;

    return CloneableTypeName();
}

/*-------------------------------------------------------------------------*/

const char*
CAssetStoreItem::CloneableTypeName( void )
{GUCEF_TRACE;

    return "GUCEF::ASSETSTORE::CAssetStoreItem";
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
