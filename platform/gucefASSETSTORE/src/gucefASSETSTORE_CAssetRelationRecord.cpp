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
#include "gucefASSETSTORE_CAssetRelationRecord.h"

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

CAssetRelationRecord::CAssetRelationRecord( void )
    : CORE::CIDataNodeSerializable()
    , fromAssetId()
    , kind()
    , toAssetId()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetRelationRecord::~CAssetRelationRecord()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CAssetRelationRecord::Clear( void )
{GUCEF_TRACE;

    *this = CAssetRelationRecord();
}

/*-------------------------------------------------------------------------*/

bool
CAssetRelationRecord::Serialize( CORE::CDataNode& domRootNode                        ,
                                const CORE::CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    bool success = true;
    success = domRootNode.SetAttribute( "fromAssetId", fromAssetId ) && success;
    success = domRootNode.SetAttribute( "kind", kind ) && success;
    success = domRootNode.SetAttribute( "toAssetId", toAssetId ) && success;
    return success;
}

/*-------------------------------------------------------------------------*/

bool
CAssetRelationRecord::Deserialize( const CORE::CDataNode& domRootNode                  ,
                                  const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    CAssetRelationRecord defaults;
    fromAssetId = domRootNode.GetAttributeValueOrChildValueByName( "fromAssetId" ).AsString( defaults.fromAssetId );
    kind = domRootNode.GetAttributeValueOrChildValueByName( "kind" ).AsString( defaults.kind );
    toAssetId = domRootNode.GetAttributeValueOrChildValueByName( "toAssetId" ).AsString( defaults.toAssetId );
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CAssetRelationRecord::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CAssetRelationRecord( *this );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CAssetRelationRecord::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::ASSETSTORE::CAssetRelationRecord";
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
