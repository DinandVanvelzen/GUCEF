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
#include "gucefASSETSTORE_CAssetPackGroup.h"

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

CAssetPackGroup::CAssetPackGroup( void )
    : CORE::CIDataNodeSerializable()
    , groupId()
    , subGroup()
    , name()
    , resourceCount( 0 )
    , sizeBytes( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetPackGroup::~CAssetPackGroup()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CAssetPackGroup::Clear( void )
{GUCEF_TRACE;

    *this = CAssetPackGroup();
}

/*-------------------------------------------------------------------------*/

bool
CAssetPackGroup::Serialize( CORE::CDataNode& domRootNode                        ,
                            const CORE::CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    bool success = true;
    success = domRootNode.SetAttribute( "groupId", groupId ) && success;
    success = domRootNode.SetAttribute( "subGroup", subGroup ) && success;
    success = domRootNode.SetAttribute( "name", name ) && success;
    success = domRootNode.SetAttribute( "resourceCount", resourceCount ) && success;
    success = domRootNode.SetAttribute( "sizeBytes", sizeBytes ) && success;
    return success;
}

/*-------------------------------------------------------------------------*/

bool
CAssetPackGroup::Deserialize( const CORE::CDataNode& domRootNode                  ,
                              const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    CAssetPackGroup defaults;
    groupId = domRootNode.GetAttributeValueOrChildValueByName( "groupId" ).AsString( defaults.groupId );
    subGroup = domRootNode.GetAttributeValueOrChildValueByName( "subGroup" ).AsString( defaults.subGroup );
    name = domRootNode.GetAttributeValueOrChildValueByName( "name" ).AsString( defaults.name );
    resourceCount = domRootNode.GetAttributeValueOrChildValueByName( "resourceCount" ).AsUInt64( defaults.resourceCount, true );
    sizeBytes = domRootNode.GetAttributeValueOrChildValueByName( "sizeBytes" ).AsUInt64( defaults.sizeBytes, true );
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CAssetPackGroup::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CAssetPackGroup( *this );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CAssetPackGroup::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::ASSETSTORE::CAssetPackGroup";
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
