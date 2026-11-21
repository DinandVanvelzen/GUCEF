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
#include "gucefASSETSTORE_CAssetPackResource.h"

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

CAssetPackResource::CAssetPackResource( void )
    : CORE::CIDataNodeSerializable()
    , resourceId()
    , packId()
    , assetId()
    , path()
    , name()
    , title()
    , kind()
    , storeKind()
    , groupId()
    , subGroup()
    , groupRule()
    , format()
    , sizeBytes( 0 )
    , digest()
    , status()
    , analysed( false )
    , converted( false )
    , hasThumbnail( false )
    , presentedUnder()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetPackResource::~CAssetPackResource()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CAssetPackResource::Clear( void )
{GUCEF_TRACE;

    *this = CAssetPackResource();
}

/*-------------------------------------------------------------------------*/

bool
CAssetPackResource::Serialize( CORE::CDataNode& domRootNode                        ,
                               const CORE::CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    bool success = true;
    success = domRootNode.SetAttribute( "resourceId", resourceId ) && success;
    success = domRootNode.SetAttribute( "packId", packId ) && success;
    success = domRootNode.SetAttribute( "assetId", assetId ) && success;
    success = domRootNode.SetAttribute( "path", path ) && success;
    success = domRootNode.SetAttribute( "name", name ) && success;
    success = domRootNode.SetAttribute( "title", title ) && success;
    success = domRootNode.SetAttribute( "kind", kind ) && success;
    success = domRootNode.SetAttribute( "storeKind", storeKind ) && success;
    success = domRootNode.SetAttribute( "groupId", groupId ) && success;
    success = domRootNode.SetAttribute( "subGroup", subGroup ) && success;
    success = domRootNode.SetAttribute( "groupRule", groupRule ) && success;
    success = domRootNode.SetAttribute( "format", format ) && success;
    success = domRootNode.SetAttribute( "sizeBytes", sizeBytes ) && success;
    success = domRootNode.SetAttribute( "digest", digest ) && success;
    success = domRootNode.SetAttribute( "status", status ) && success;
    success = domRootNode.SetAttribute( "analysed", analysed ) && success;
    success = domRootNode.SetAttribute( "converted", converted ) && success;
    success = domRootNode.SetAttribute( "hasThumbnail", hasThumbnail ) && success;
    success = domRootNode.SetAttribute( "presentedUnder", presentedUnder ) && success;
    return success;
}

/*-------------------------------------------------------------------------*/

bool
CAssetPackResource::Deserialize( const CORE::CDataNode& domRootNode                  ,
                                  const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    CAssetPackResource defaults;
    resourceId = domRootNode.GetAttributeValueOrChildValueByName( "resourceId" ).AsString( defaults.resourceId );
    packId = domRootNode.GetAttributeValueOrChildValueByName( "packId" ).AsString( defaults.packId );
    assetId = domRootNode.GetAttributeValueOrChildValueByName( "assetId" ).AsString( defaults.assetId );
    path = domRootNode.GetAttributeValueOrChildValueByName( "path" ).AsString( defaults.path );
    name = domRootNode.GetAttributeValueOrChildValueByName( "name" ).AsString( defaults.name );
    title = domRootNode.GetAttributeValueOrChildValueByName( "title" ).AsString( defaults.title );
    kind = domRootNode.GetAttributeValueOrChildValueByName( "kind" ).AsString( defaults.kind );
    storeKind = domRootNode.GetAttributeValueOrChildValueByName( "storeKind" ).AsString( defaults.storeKind );
    groupId = domRootNode.GetAttributeValueOrChildValueByName( "groupId" ).AsString( defaults.groupId );
    subGroup = domRootNode.GetAttributeValueOrChildValueByName( "subGroup" ).AsString( defaults.subGroup );
    groupRule = domRootNode.GetAttributeValueOrChildValueByName( "groupRule" ).AsString( defaults.groupRule );
    format = domRootNode.GetAttributeValueOrChildValueByName( "format" ).AsString( defaults.format );
    sizeBytes = domRootNode.GetAttributeValueOrChildValueByName( "sizeBytes" ).AsUInt64( defaults.sizeBytes, true );
    digest = domRootNode.GetAttributeValueOrChildValueByName( "digest" ).AsString( defaults.digest );
    status = domRootNode.GetAttributeValueOrChildValueByName( "status" ).AsString( defaults.status );
    analysed = domRootNode.GetAttributeValueOrChildValueByName( "analysed" ).AsBool( defaults.analysed, true );
    converted = domRootNode.GetAttributeValueOrChildValueByName( "converted" ).AsBool( defaults.converted, true );
    hasThumbnail = domRootNode.GetAttributeValueOrChildValueByName( "hasThumbnail" ).AsBool( defaults.hasThumbnail, true );
    presentedUnder = domRootNode.GetAttributeValueOrChildValueByName( "presentedUnder" ).AsString( defaults.presentedUnder );
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CAssetPackResource::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CAssetPackResource( *this );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CAssetPackResource::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::ASSETSTORE::CAssetPackResource";
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
