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
#include "gucefASSETSTORE_CAssetModelProps.h"

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

CAssetModelProps::CAssetModelProps( void )
    : CORE::CIDataNodeSerializable()
    , isSet( false )
    , triangles( 0 )
    , vertices( 0 )
    , subMeshes( 0 )
    , bones( 0 )
    , animations( 0 )
    , lods( 1 )
    , boundsX( 0.0 )
    , boundsY( 0.0 )
    , boundsZ( 0.0 )
    , upAxis()
    , unit()
    , rigged( false )
    , animated( false )
    , hasCollision( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetModelProps::~CAssetModelProps()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CAssetModelProps::Clear( void )
{GUCEF_TRACE;

    *this = CAssetModelProps();
}

/*-------------------------------------------------------------------------*/

bool
CAssetModelProps::Serialize( CORE::CDataNode& domRootNode                        ,
                            const CORE::CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    bool success = true;
    success = domRootNode.SetAttribute( "isSet", isSet ) && success;
    success = domRootNode.SetAttribute( "triangles", triangles ) && success;
    success = domRootNode.SetAttribute( "vertices", vertices ) && success;
    success = domRootNode.SetAttribute( "subMeshes", subMeshes ) && success;
    success = domRootNode.SetAttribute( "bones", bones ) && success;
    success = domRootNode.SetAttribute( "animations", animations ) && success;
    success = domRootNode.SetAttribute( "lods", lods ) && success;
    success = domRootNode.SetAttribute( "boundsX", CORE::CVariant( boundsX ) ) && success;
    success = domRootNode.SetAttribute( "boundsY", CORE::CVariant( boundsY ) ) && success;
    success = domRootNode.SetAttribute( "boundsZ", CORE::CVariant( boundsZ ) ) && success;
    success = domRootNode.SetAttribute( "upAxis", upAxis ) && success;
    success = domRootNode.SetAttribute( "unit", unit ) && success;
    success = domRootNode.SetAttribute( "rigged", rigged ) && success;
    success = domRootNode.SetAttribute( "animated", animated ) && success;
    success = domRootNode.SetAttribute( "hasCollision", hasCollision ) && success;
    return success;
}

/*-------------------------------------------------------------------------*/

bool
CAssetModelProps::Deserialize( const CORE::CDataNode& domRootNode                  ,
                              const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    CAssetModelProps defaults;
    isSet = domRootNode.GetAttributeValueOrChildValueByName( "isSet" ).AsBool( defaults.isSet, true );
    triangles = domRootNode.GetAttributeValueOrChildValueByName( "triangles" ).AsInt64( defaults.triangles, true );
    vertices = domRootNode.GetAttributeValueOrChildValueByName( "vertices" ).AsInt64( defaults.vertices, true );
    subMeshes = domRootNode.GetAttributeValueOrChildValueByName( "subMeshes" ).AsInt64( defaults.subMeshes, true );
    bones = domRootNode.GetAttributeValueOrChildValueByName( "bones" ).AsInt64( defaults.bones, true );
    animations = domRootNode.GetAttributeValueOrChildValueByName( "animations" ).AsInt64( defaults.animations, true );
    lods = domRootNode.GetAttributeValueOrChildValueByName( "lods" ).AsInt64( defaults.lods, true );
    boundsX = domRootNode.GetAttributeValueOrChildValueByName( "boundsX" ).AsFloat64( defaults.boundsX, true );
    boundsY = domRootNode.GetAttributeValueOrChildValueByName( "boundsY" ).AsFloat64( defaults.boundsY, true );
    boundsZ = domRootNode.GetAttributeValueOrChildValueByName( "boundsZ" ).AsFloat64( defaults.boundsZ, true );
    upAxis = domRootNode.GetAttributeValueOrChildValueByName( "upAxis" ).AsString( defaults.upAxis );
    unit = domRootNode.GetAttributeValueOrChildValueByName( "unit" ).AsString( defaults.unit );
    rigged = domRootNode.GetAttributeValueOrChildValueByName( "rigged" ).AsBool( defaults.rigged, true );
    animated = domRootNode.GetAttributeValueOrChildValueByName( "animated" ).AsBool( defaults.animated, true );
    hasCollision = domRootNode.GetAttributeValueOrChildValueByName( "hasCollision" ).AsBool( defaults.hasCollision, true );
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CAssetModelProps::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CAssetModelProps( *this );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CAssetModelProps::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::ASSETSTORE::CAssetModelProps";
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
