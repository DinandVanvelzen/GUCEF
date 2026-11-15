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
#include "gucefASSETSTORE_CAssetSearchHit.h"

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

CAssetSearchHit::CAssetSearchHit( void )
    : CORE::CIDataNodeSerializable()
    , assetId()
    , packId()
    , name()
    , kind()
    , categoryId()
    , sourceId()
    , seller()
    , packTitle()
    , licenceClass()
    , locationId()
    , vfsPath()
    , available( false )
    , score( 0 )
    , sizeBytes( 0 )
    , ingestedAtMs( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetSearchHit::~CAssetSearchHit()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CAssetSearchHit::Clear( void )
{GUCEF_TRACE;

    *this = CAssetSearchHit();
}

/*-------------------------------------------------------------------------*/

bool
CAssetSearchHit::Serialize( CORE::CDataNode& domRootNode                        ,
                           const CORE::CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    bool success = true;
    success = domRootNode.SetAttribute( "assetId", assetId ) && success;
    success = domRootNode.SetAttribute( "packId", packId ) && success;
    success = domRootNode.SetAttribute( "name", name ) && success;
    success = domRootNode.SetAttribute( "kind", kind ) && success;
    success = domRootNode.SetAttribute( "categoryId", categoryId ) && success;
    success = domRootNode.SetAttribute( "sourceId", sourceId ) && success;
    success = domRootNode.SetAttribute( "seller", seller ) && success;
    success = domRootNode.SetAttribute( "packTitle", packTitle ) && success;
    success = domRootNode.SetAttribute( "licenceClass", licenceClass ) && success;
    success = domRootNode.SetAttribute( "locationId", locationId ) && success;
    success = domRootNode.SetAttribute( "vfsPath", vfsPath ) && success;
    success = domRootNode.SetAttribute( "available", available ) && success;
    success = domRootNode.SetAttribute( "score", score ) && success;
    success = domRootNode.SetAttribute( "sizeBytes", sizeBytes ) && success;
    success = domRootNode.SetAttribute( "ingestedAtMs", ingestedAtMs ) && success;
    return success;
}

/*-------------------------------------------------------------------------*/

bool
CAssetSearchHit::Deserialize( const CORE::CDataNode& domRootNode                  ,
                             const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    CAssetSearchHit defaults;
    assetId = domRootNode.GetAttributeValueOrChildValueByName( "assetId" ).AsString( defaults.assetId );
    packId = domRootNode.GetAttributeValueOrChildValueByName( "packId" ).AsString( defaults.packId );
    name = domRootNode.GetAttributeValueOrChildValueByName( "name" ).AsString( defaults.name );
    kind = domRootNode.GetAttributeValueOrChildValueByName( "kind" ).AsString( defaults.kind );
    categoryId = domRootNode.GetAttributeValueOrChildValueByName( "categoryId" ).AsString( defaults.categoryId );
    sourceId = domRootNode.GetAttributeValueOrChildValueByName( "sourceId" ).AsString( defaults.sourceId );
    seller = domRootNode.GetAttributeValueOrChildValueByName( "seller" ).AsString( defaults.seller );
    packTitle = domRootNode.GetAttributeValueOrChildValueByName( "packTitle" ).AsString( defaults.packTitle );
    licenceClass = domRootNode.GetAttributeValueOrChildValueByName( "licenceClass" ).AsString( defaults.licenceClass );
    locationId = domRootNode.GetAttributeValueOrChildValueByName( "locationId" ).AsString( defaults.locationId );
    vfsPath = domRootNode.GetAttributeValueOrChildValueByName( "vfsPath" ).AsString( defaults.vfsPath );
    available = domRootNode.GetAttributeValueOrChildValueByName( "available" ).AsBool( defaults.available, true );
    score = domRootNode.GetAttributeValueOrChildValueByName( "score" ).AsInt64( defaults.score, true );
    sizeBytes = domRootNode.GetAttributeValueOrChildValueByName( "sizeBytes" ).AsInt64( defaults.sizeBytes, true );
    ingestedAtMs = domRootNode.GetAttributeValueOrChildValueByName( "ingestedAtMs" ).AsInt64( defaults.ingestedAtMs, true );
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CAssetSearchHit::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CAssetSearchHit( *this );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CAssetSearchHit::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::ASSETSTORE::CAssetSearchHit";
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
