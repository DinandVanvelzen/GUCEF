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
#include "gucefASSETSTORE_CAssetSearchQuery.h"

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

CAssetSearchQuery::CAssetSearchQuery( void )
    : CORE::CIDataNodeSerializable()
    , text()
    , prefixLastWord( false )
    , kinds()
    , categories()
    , themes()
    , artStyles()
    , sources()
    , sellers()
    , licenceClasses()
    , requireLicenceRestrictions()
    , excludeLicenceRestrictions()
    , formats()
    , packFormatsOnly()
    , minAssetBytes( -1 )
    , maxAssetBytes( -1 )
    , minPackBytes( -1 )
    , maxPackBytes( -1 )
    , minTriangles( -1 )
    , maxTriangles( -1 )
    , minVertices( -1 )
    , maxVertices( -1 )
    , minTextureResolution( -1 )
    , maxTextureResolution( -1 )
    , minAnimationSeconds( -1.0 )
    , maxAnimationSeconds( -1.0 )
    , rigged( -1 )
    , animated( -1 )
    , hasLods( -1 )
    , hasCollision( -1 )
    , pbrRoles()
    , packStates()
    , unclassified( false )
    , packIds()
    , sourceItemIds()
    , groups()
    , subGroups()
    , resultKind( "assets" )
    , sort( "relevance" )
    , pageIndex( 0 )
    , pageSize( 50 )
    , withFacets( true )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetSearchQuery::~CAssetSearchQuery()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CAssetSearchQuery::Clear( void )
{GUCEF_TRACE;

    *this = CAssetSearchQuery();
}

/*-------------------------------------------------------------------------*/

bool
CAssetSearchQuery::Serialize( CORE::CDataNode& domRootNode                        ,
                             const CORE::CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    bool success = true;
    success = domRootNode.SetAttribute( "text", text ) && success;
    success = domRootNode.SetAttribute( "prefixLastWord", prefixLastWord ) && success;
    success = domRootNode.SetValuesOfChildByName( "kinds", kinds ) && success;
    success = domRootNode.SetValuesOfChildByName( "categories", categories ) && success;
    success = domRootNode.SetValuesOfChildByName( "themes", themes ) && success;
    success = domRootNode.SetValuesOfChildByName( "artStyles", artStyles ) && success;
    success = domRootNode.SetValuesOfChildByName( "sources", sources ) && success;
    success = domRootNode.SetValuesOfChildByName( "sellers", sellers ) && success;
    success = domRootNode.SetValuesOfChildByName( "licenceClasses", licenceClasses ) && success;
    success = domRootNode.SetValuesOfChildByName( "requireLicenceRestrictions", requireLicenceRestrictions ) && success;
    success = domRootNode.SetValuesOfChildByName( "excludeLicenceRestrictions", excludeLicenceRestrictions ) && success;
    success = domRootNode.SetValuesOfChildByName( "formats", formats ) && success;
    success = domRootNode.SetValuesOfChildByName( "packFormatsOnly", packFormatsOnly ) && success;
    success = domRootNode.SetAttribute( "minAssetBytes", minAssetBytes ) && success;
    success = domRootNode.SetAttribute( "maxAssetBytes", maxAssetBytes ) && success;
    success = domRootNode.SetAttribute( "minPackBytes", minPackBytes ) && success;
    success = domRootNode.SetAttribute( "maxPackBytes", maxPackBytes ) && success;
    success = domRootNode.SetAttribute( "minTriangles", minTriangles ) && success;
    success = domRootNode.SetAttribute( "maxTriangles", maxTriangles ) && success;
    success = domRootNode.SetAttribute( "minVertices", minVertices ) && success;
    success = domRootNode.SetAttribute( "maxVertices", maxVertices ) && success;
    success = domRootNode.SetAttribute( "minTextureResolution", minTextureResolution ) && success;
    success = domRootNode.SetAttribute( "maxTextureResolution", maxTextureResolution ) && success;
    success = domRootNode.SetAttribute( "minAnimationSeconds", CORE::CVariant( minAnimationSeconds ) ) && success;
    success = domRootNode.SetAttribute( "maxAnimationSeconds", CORE::CVariant( maxAnimationSeconds ) ) && success;
    success = domRootNode.SetAttribute( "rigged", rigged ) && success;
    success = domRootNode.SetAttribute( "animated", animated ) && success;
    success = domRootNode.SetAttribute( "hasLods", hasLods ) && success;
    success = domRootNode.SetAttribute( "hasCollision", hasCollision ) && success;
    success = domRootNode.SetValuesOfChildByName( "pbrRoles", pbrRoles ) && success;
    success = domRootNode.SetValuesOfChildByName( "packStates", packStates ) && success;
    success = domRootNode.SetAttribute( "unclassified", unclassified ) && success;
    success = domRootNode.SetValuesOfChildByName( "packIds", packIds ) && success;
    if ( !sourceItemIds.empty() )
        success = domRootNode.SetValuesOfChildByName( "sourceItemIds", sourceItemIds ) && success;
    success = domRootNode.SetValuesOfChildByName( "groups", groups ) && success;
    success = domRootNode.SetValuesOfChildByName( "subGroups", subGroups ) && success;
    success = domRootNode.SetAttribute( "resultKind", resultKind ) && success;
    success = domRootNode.SetAttribute( "sort", sort ) && success;
    success = domRootNode.SetAttribute( "pageIndex", pageIndex ) && success;
    success = domRootNode.SetAttribute( "pageSize", pageSize ) && success;
    success = domRootNode.SetAttribute( "withFacets", withFacets ) && success;
    return success;
}

/*-------------------------------------------------------------------------*/

bool
CAssetSearchQuery::Deserialize( const CORE::CDataNode& domRootNode                  ,
                               const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    CAssetSearchQuery defaults;
    text = domRootNode.GetAttributeValueOrChildValueByName( "text" ).AsString( defaults.text );
    prefixLastWord = domRootNode.GetAttributeValueOrChildValueByName( "prefixLastWord" ).AsBool( defaults.prefixLastWord, true );
    kinds.clear();
    domRootNode.GetValuesOfChildByName( "kinds", kinds );
    categories.clear();
    domRootNode.GetValuesOfChildByName( "categories", categories );
    themes.clear();
    domRootNode.GetValuesOfChildByName( "themes", themes );
    artStyles.clear();
    domRootNode.GetValuesOfChildByName( "artStyles", artStyles );
    sources.clear();
    domRootNode.GetValuesOfChildByName( "sources", sources );
    sellers.clear();
    domRootNode.GetValuesOfChildByName( "sellers", sellers );
    licenceClasses.clear();
    domRootNode.GetValuesOfChildByName( "licenceClasses", licenceClasses );
    requireLicenceRestrictions.clear();
    domRootNode.GetValuesOfChildByName( "requireLicenceRestrictions", requireLicenceRestrictions );
    excludeLicenceRestrictions.clear();
    domRootNode.GetValuesOfChildByName( "excludeLicenceRestrictions", excludeLicenceRestrictions );
    formats.clear();
    domRootNode.GetValuesOfChildByName( "formats", formats );
    packFormatsOnly.clear();
    domRootNode.GetValuesOfChildByName( "packFormatsOnly", packFormatsOnly );
    minAssetBytes = domRootNode.GetAttributeValueOrChildValueByName( "minAssetBytes" ).AsInt64( defaults.minAssetBytes, true );
    maxAssetBytes = domRootNode.GetAttributeValueOrChildValueByName( "maxAssetBytes" ).AsInt64( defaults.maxAssetBytes, true );
    minPackBytes = domRootNode.GetAttributeValueOrChildValueByName( "minPackBytes" ).AsInt64( defaults.minPackBytes, true );
    maxPackBytes = domRootNode.GetAttributeValueOrChildValueByName( "maxPackBytes" ).AsInt64( defaults.maxPackBytes, true );
    minTriangles = domRootNode.GetAttributeValueOrChildValueByName( "minTriangles" ).AsInt64( defaults.minTriangles, true );
    maxTriangles = domRootNode.GetAttributeValueOrChildValueByName( "maxTriangles" ).AsInt64( defaults.maxTriangles, true );
    minVertices = domRootNode.GetAttributeValueOrChildValueByName( "minVertices" ).AsInt64( defaults.minVertices, true );
    maxVertices = domRootNode.GetAttributeValueOrChildValueByName( "maxVertices" ).AsInt64( defaults.maxVertices, true );
    minTextureResolution = domRootNode.GetAttributeValueOrChildValueByName( "minTextureResolution" ).AsInt64( defaults.minTextureResolution, true );
    maxTextureResolution = domRootNode.GetAttributeValueOrChildValueByName( "maxTextureResolution" ).AsInt64( defaults.maxTextureResolution, true );
    minAnimationSeconds = domRootNode.GetAttributeValueOrChildValueByName( "minAnimationSeconds" ).AsFloat64( defaults.minAnimationSeconds, true );
    maxAnimationSeconds = domRootNode.GetAttributeValueOrChildValueByName( "maxAnimationSeconds" ).AsFloat64( defaults.maxAnimationSeconds, true );
    rigged = domRootNode.GetAttributeValueOrChildValueByName( "rigged" ).AsInt64( defaults.rigged, true );
    animated = domRootNode.GetAttributeValueOrChildValueByName( "animated" ).AsInt64( defaults.animated, true );
    hasLods = domRootNode.GetAttributeValueOrChildValueByName( "hasLods" ).AsInt64( defaults.hasLods, true );
    hasCollision = domRootNode.GetAttributeValueOrChildValueByName( "hasCollision" ).AsInt64( defaults.hasCollision, true );
    pbrRoles.clear();
    domRootNode.GetValuesOfChildByName( "pbrRoles", pbrRoles );
    packStates.clear();
    domRootNode.GetValuesOfChildByName( "packStates", packStates );
    unclassified = domRootNode.GetAttributeValueOrChildValueByName( "unclassified" ).AsBool( defaults.unclassified, true );
    packIds.clear();
    domRootNode.GetValuesOfChildByName( "packIds", packIds );
    sourceItemIds.clear();
    domRootNode.GetValuesOfChildByName( "sourceItemIds", sourceItemIds );
    groups.clear();
    domRootNode.GetValuesOfChildByName( "groups", groups );
    subGroups.clear();
    domRootNode.GetValuesOfChildByName( "subGroups", subGroups );
    resultKind = domRootNode.GetAttributeValueOrChildValueByName( "resultKind" ).AsString( defaults.resultKind );
    sort = domRootNode.GetAttributeValueOrChildValueByName( "sort" ).AsString( defaults.sort );
    pageIndex = domRootNode.GetAttributeValueOrChildValueByName( "pageIndex" ).AsInt64( defaults.pageIndex, true );
    pageSize = domRootNode.GetAttributeValueOrChildValueByName( "pageSize" ).AsInt64( defaults.pageSize, true );
    withFacets = domRootNode.GetAttributeValueOrChildValueByName( "withFacets" ).AsBool( defaults.withFacets, true );
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CAssetSearchQuery::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CAssetSearchQuery( *this );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CAssetSearchQuery::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::ASSETSTORE::CAssetSearchQuery";
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
