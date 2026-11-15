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
#include "gucefASSETSTORE_CAssetPackRecord.h"

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

CAssetPackRecord::CAssetPackRecord( void )
    : CORE::CIDataNodeSerializable()
    , packId()
    , sourceId()
    , sourceItemId()
    , sourceVersion()
    , sourceUrl()
    , title()
    , seller()
    , description()
    , sourceCategories()
    , sourceTags()
    , categories()
    , unmappedCategories()
    , kinds()
    , themes()
    , artStyles()
    , pbrRoles()
    , keywords()
    , licence()
    , formatsOriginal()
    , formatsConverted()
    , keep( ASSETKEEP_BOTH )
    , locationId()
    , rootPath()
    , sizeBytes( 0 )
    , fileCount( 0 )
    , assetCount( 0 )
    , ingestedAtMs( 0 )
    , normalizer()
    , analyzers()
    , conversion()
    , state( ASSETPACK_STATE_STAGED )
    , stateReason()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetPackRecord::~CAssetPackRecord()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CAssetPackRecord::Serialize( CORE::CDataNode& domRootNode                        ,
                             const CORE::CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    bool success = domRootNode.SetAttribute( "packId", packId );
    success = domRootNode.SetAttribute( "sourceId", sourceId ) && success;
    success = domRootNode.SetAttribute( "sourceItemId", sourceItemId ) && success;
    success = domRootNode.SetAttribute( "sourceVersion", sourceVersion ) && success;
    success = domRootNode.SetAttribute( "sourceUrl", sourceUrl ) && success;
    success = domRootNode.SetAttribute( "title", title ) && success;
    success = domRootNode.SetAttribute( "seller", seller ) && success;
    success = domRootNode.SetAttribute( "description", description ) && success;
    success = domRootNode.SetValuesOfChildByName( "sourceCategories", sourceCategories ) && success;
    success = domRootNode.SetValuesOfChildByName( "sourceTags", sourceTags ) && success;
    success = domRootNode.SetValuesOfChildByName( "categories", categories ) && success;
    success = domRootNode.SetValuesOfChildByName( "unmappedCategories", unmappedCategories ) && success;
    success = domRootNode.SetValuesOfChildByName( "kinds", kinds ) && success;
    success = domRootNode.SetValuesOfChildByName( "themes", themes ) && success;
    success = domRootNode.SetValuesOfChildByName( "artStyles", artStyles ) && success;
    success = domRootNode.SetValuesOfChildByName( "pbrRoles", pbrRoles ) && success;
    CORE::CDataNode* keywordsNode = domRootNode.AddChild( "keywords", GUCEF_DATATYPE_ARRAY );
    for ( size_t i=0; GUCEF_NULL != keywordsNode && i<keywords.size(); ++i )
        success = keywords[ i ].Serialize( *keywordsNode->AddChild( "keyword", GUCEF_DATATYPE_OBJECT ), settings ) && success;
    success = licence.Serialize( *domRootNode.AddChild( "licence", GUCEF_DATATYPE_OBJECT ), settings ) && success;
    success = domRootNode.SetValuesOfChildByName( "formatsOriginal", formatsOriginal ) && success;
    success = domRootNode.SetValuesOfChildByName( "formatsConverted", formatsConverted ) && success;
    success = domRootNode.SetAttribute( "keep", CORE::CString( AssetKeepPolicyToString( keep ) ) ) && success;
    success = domRootNode.SetAttribute( "locationId", locationId ) && success;
    success = domRootNode.SetAttribute( "rootPath", rootPath ) && success;
    success = domRootNode.SetAttribute( "sizeBytes", sizeBytes ) && success;
    success = domRootNode.SetAttribute( "fileCount", fileCount ) && success;
    success = domRootNode.SetAttribute( "assetCount", assetCount ) && success;
    success = domRootNode.SetAttribute( "ingestedAtMs", ingestedAtMs ) && success;
    success = domRootNode.SetAttribute( "normalizer", normalizer ) && success;
    success = domRootNode.SetValuesOfChildByName( "analyzers", analyzers ) && success;
    success = conversion.Serialize( *domRootNode.AddChild( "conversion", GUCEF_DATATYPE_OBJECT ), settings ) && success;
    success = domRootNode.SetAttribute( "state", CORE::CString( AssetPackStateToString( state ) ) ) && success;
    success = domRootNode.SetAttribute( "stateReason", stateReason ) && success;
    return success;
}

/*-------------------------------------------------------------------------*/

bool
CAssetPackRecord::Deserialize( const CORE::CDataNode& domRootNode                  ,
                               const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    *this = CAssetPackRecord();
    packId = domRootNode.GetAttributeValueOrChildValueByName( "packId" ).AsString();
    sourceId = domRootNode.GetAttributeValueOrChildValueByName( "sourceId" ).AsString();
    sourceItemId = domRootNode.GetAttributeValueOrChildValueByName( "sourceItemId" ).AsString();
    sourceVersion = domRootNode.GetAttributeValueOrChildValueByName( "sourceVersion" ).AsString();
    sourceUrl = domRootNode.GetAttributeValueOrChildValueByName( "sourceUrl" ).AsString();
    title = domRootNode.GetAttributeValueOrChildValueByName( "title" ).AsString();
    seller = domRootNode.GetAttributeValueOrChildValueByName( "seller" ).AsString();
    description = domRootNode.GetAttributeValueOrChildValueByName( "description" ).AsString();
    domRootNode.GetValuesOfChildByName( "sourceCategories", sourceCategories );
    domRootNode.GetValuesOfChildByName( "sourceTags", sourceTags );
    domRootNode.GetValuesOfChildByName( "categories", categories );
    domRootNode.GetValuesOfChildByName( "unmappedCategories", unmappedCategories );
    domRootNode.GetValuesOfChildByName( "kinds", kinds );
    domRootNode.GetValuesOfChildByName( "themes", themes );
    domRootNode.GetValuesOfChildByName( "artStyles", artStyles );
    domRootNode.GetValuesOfChildByName( "pbrRoles", pbrRoles );
    const CORE::CDataNode* keywordsNode = domRootNode.FindChild( "keywords" );
    if ( GUCEF_NULL != keywordsNode )
    {
        CORE::CDataNode::const_iterator i = keywordsNode->ConstBegin();
        while ( i != keywordsNode->ConstEnd() )
        {
            CAssetKeyword keyword;
            if ( keyword.Deserialize( *(*i), settings ) )
                keywords.push_back( keyword );
            ++i;
        }
    }
    const CORE::CDataNode* licenceNode = domRootNode.FindChild( "licence" );
    if ( GUCEF_NULL != licenceNode )
        licence.Deserialize( *licenceNode, settings );
    domRootNode.GetValuesOfChildByName( "formatsOriginal", formatsOriginal );
    domRootNode.GetValuesOfChildByName( "formatsConverted", formatsConverted );
    keep = AssetKeepPolicyFromString( domRootNode.GetAttributeValueOrChildValueByName( "keep" ).AsString() );
    locationId = domRootNode.GetAttributeValueOrChildValueByName( "locationId" ).AsString();
    rootPath = domRootNode.GetAttributeValueOrChildValueByName( "rootPath" ).AsString();
    sizeBytes = domRootNode.GetAttributeValueOrChildValueByName( "sizeBytes" ).AsInt64( 0, true );
    fileCount = domRootNode.GetAttributeValueOrChildValueByName( "fileCount" ).AsInt64( 0, true );
    assetCount = domRootNode.GetAttributeValueOrChildValueByName( "assetCount" ).AsInt64( 0, true );
    ingestedAtMs = domRootNode.GetAttributeValueOrChildValueByName( "ingestedAtMs" ).AsInt64( 0, true );
    normalizer = domRootNode.GetAttributeValueOrChildValueByName( "normalizer" ).AsString();
    domRootNode.GetValuesOfChildByName( "analyzers", analyzers );
    const CORE::CDataNode* conversionNode = domRootNode.FindChild( "conversion" );
    if ( GUCEF_NULL != conversionNode )
        conversion.Deserialize( *conversionNode, settings );
    state = AssetPackStateFromString( domRootNode.GetAttributeValueOrChildValueByName( "state" ).AsString() );
    stateReason = domRootNode.GetAttributeValueOrChildValueByName( "stateReason" ).AsString();
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CAssetPackRecord::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CAssetPackRecord( *this );
}

/*-------------------------------------------------------------------------*/

const char*
CAssetPackRecord::GetCloneableTypeName( void ) const
{GUCEF_TRACE;

    return CloneableTypeName();
}

/*-------------------------------------------------------------------------*/

const char*
CAssetPackRecord::CloneableTypeName( void )
{GUCEF_TRACE;

    return "GUCEF::ASSETSTORE::CAssetPackRecord";
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CAssetPackRecord::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::ASSETSTORE::CAssetPackRecord";
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
