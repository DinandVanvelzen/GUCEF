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
#include "gucefASSETSTORE_CAssetStoreTransfer.h"

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

CAssetStoreTransfer::CAssetStoreTransfer( void )
    : CORE::CIDataNodeSerializable()
    , fileId()
    , url()
    , size( 0 )
    , digest()
    , digestAlgorithm()
    , targetName()
    , expiresAtMs( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetStoreTransfer::~CAssetStoreTransfer()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CAssetStoreTransfer::IsExpired( Int64 nowUtcMs ) const
{GUCEF_TRACE;

    return expiresAtMs > 0 && nowUtcMs >= expiresAtMs;
}

/*-------------------------------------------------------------------------*/

bool
CAssetStoreTransfer::Serialize( CORE::CDataNode& domRootNode                        ,
                                const CORE::CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    bool success = domRootNode.SetAttribute( "fileId", fileId );
    success = domRootNode.SetAttribute( "url", url ) && success;
    success = domRootNode.SetAttribute( "size", size ) && success;
    success = domRootNode.SetAttribute( "digest", digest ) && success;
    success = domRootNode.SetAttribute( "digestAlgorithm", digestAlgorithm ) && success;
    success = domRootNode.SetAttribute( "targetName", targetName ) && success;
    success = domRootNode.SetAttribute( "expiresAtMs", expiresAtMs ) && success;
    return success;
}

/*-------------------------------------------------------------------------*/

bool
CAssetStoreTransfer::Deserialize( const CORE::CDataNode& domRootNode                  ,
                                  const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    fileId = domRootNode.GetAttributeValueOrChildValueByName( "fileId" ).AsString();
    url = domRootNode.GetAttributeValueOrChildValueByName( "url" ).AsString();
    size = domRootNode.GetAttributeValueOrChildValueByName( "size" ).AsUInt64( 0, true );
    digest = domRootNode.GetAttributeValueOrChildValueByName( "digest" ).AsString().Lowercase();
    digestAlgorithm = domRootNode.GetAttributeValueOrChildValueByName( "digestAlgorithm" ).AsString().Lowercase();
    targetName = domRootNode.GetAttributeValueOrChildValueByName( "targetName" ).AsString();
    expiresAtMs = domRootNode.GetAttributeValueOrChildValueByName( "expiresAtMs" ).AsInt64( 0, true );
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CAssetStoreTransfer::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CAssetStoreTransfer( *this );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CAssetStoreTransfer::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::ASSETSTORE::CAssetStoreTransfer";
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
