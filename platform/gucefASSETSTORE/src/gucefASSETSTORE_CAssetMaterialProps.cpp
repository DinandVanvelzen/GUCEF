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
#include "gucefASSETSTORE_CAssetMaterialProps.h"

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

CAssetMaterialProps::CAssetMaterialProps( void )
    : CORE::CIDataNodeSerializable()
    , isSet( false )
    , shadingModel()
    , textureCount( 0 )
    , pbrRoles()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetMaterialProps::~CAssetMaterialProps()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CAssetMaterialProps::Clear( void )
{GUCEF_TRACE;

    *this = CAssetMaterialProps();
}

/*-------------------------------------------------------------------------*/

bool
CAssetMaterialProps::Serialize( CORE::CDataNode& domRootNode                        ,
                               const CORE::CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    bool success = true;
    success = domRootNode.SetAttribute( "isSet", isSet ) && success;
    success = domRootNode.SetAttribute( "shadingModel", shadingModel ) && success;
    success = domRootNode.SetAttribute( "textureCount", textureCount ) && success;
    success = domRootNode.SetValuesOfChildByName( "pbrRoles", pbrRoles ) && success;
    return success;
}

/*-------------------------------------------------------------------------*/

bool
CAssetMaterialProps::Deserialize( const CORE::CDataNode& domRootNode                  ,
                                 const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    CAssetMaterialProps defaults;
    isSet = domRootNode.GetAttributeValueOrChildValueByName( "isSet" ).AsBool( defaults.isSet, true );
    shadingModel = domRootNode.GetAttributeValueOrChildValueByName( "shadingModel" ).AsString( defaults.shadingModel );
    textureCount = domRootNode.GetAttributeValueOrChildValueByName( "textureCount" ).AsInt64( defaults.textureCount, true );
    pbrRoles.clear();
    domRootNode.GetValuesOfChildByName( "pbrRoles", pbrRoles );
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CAssetMaterialProps::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CAssetMaterialProps( *this );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CAssetMaterialProps::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::ASSETSTORE::CAssetMaterialProps";
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
