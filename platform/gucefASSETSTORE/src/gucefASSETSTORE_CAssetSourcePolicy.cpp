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
#include "gucefASSETSTORE_CAssetSourcePolicy.h"

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

CAssetSourcePolicy::CAssetSourcePolicy( void )
    : CORE::CIDataNodeSerializable()
    , sourceId()
    , mode( ASSETINGEST_MODE_MANUAL )
    , keep( ASSETKEEP_BOTH )
    , locationId()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetSourcePolicy::~CAssetSourcePolicy()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CAssetSourcePolicy::Serialize( CORE::CDataNode& domRootNode                        ,
                               const CORE::CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    bool success = domRootNode.SetAttribute( "sourceId", sourceId );
    success = domRootNode.SetAttribute( "mode", CORE::CString( AssetIngestModeToString( mode ) ) ) && success;
    success = domRootNode.SetAttribute( "keep", CORE::CString( AssetKeepPolicyToString( keep ) ) ) && success;
    success = domRootNode.SetAttribute( "locationId", locationId ) && success;
    return success;
}

/*-------------------------------------------------------------------------*/

bool
CAssetSourcePolicy::Deserialize( const CORE::CDataNode& domRootNode                  ,
                                 const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    sourceId = domRootNode.GetAttributeValueOrChildValueByName( "sourceId" ).AsString().Lowercase();
    mode = AssetIngestModeFromString( domRootNode.GetAttributeValueOrChildValueByName( "mode" ).AsString(), ASSETINGEST_MODE_MANUAL );
    keep = AssetKeepPolicyFromString( domRootNode.GetAttributeValueOrChildValueByName( "keep" ).AsString(), ASSETKEEP_BOTH );
    locationId = domRootNode.GetAttributeValueOrChildValueByName( "locationId" ).AsString();
    return !sourceId.IsNULLOrEmpty();
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CAssetSourcePolicy::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CAssetSourcePolicy( *this );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CAssetSourcePolicy::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::ASSETSTORE::CAssetSourcePolicy";
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
