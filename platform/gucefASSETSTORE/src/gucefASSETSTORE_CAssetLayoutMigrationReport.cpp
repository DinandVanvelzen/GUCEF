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

#include "gucefASSETSTORE_CAssetLayoutMigrationReport.h"

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

CAssetLayoutMigrationReport::CAssetLayoutMigrationReport( void )
    : CORE::CIDataNodeSerializable()
    , locationId()
    , status( ASSETSTORE_STATUS_OK )
    , dryRun( false )
    , packsInOldLayout( 0 )
    , packsMigrated( 0 )
    , packsRefused( 0 )
    , actions()
    , problems()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetLayoutMigrationReport::~CAssetLayoutMigrationReport()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CAssetLayoutMigrationReport::Serialize( CORE::CDataNode& domRootNode                        ,
                                        const CORE::CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    bool success = true;
    success = domRootNode.SetAttribute( "locationId", locationId ) && success;
    success = domRootNode.SetAttribute( "status", CORE::CString( AssetStoreStatusToString( status ) ) ) && success;
    success = domRootNode.SetAttribute( "dryRun", dryRun ) && success;
    success = domRootNode.SetAttribute( "packsInOldLayout", packsInOldLayout ) && success;
    success = domRootNode.SetAttribute( "packsMigrated", packsMigrated ) && success;
    success = domRootNode.SetAttribute( "packsRefused", packsRefused ) && success;
    success = domRootNode.SetValuesOfChildByName( "actions", actions ) && success;
    success = domRootNode.SetValuesOfChildByName( "problems", problems ) && success;
    return success;
}

/*-------------------------------------------------------------------------*/

bool
CAssetLayoutMigrationReport::Deserialize( const CORE::CDataNode& domRootNode                  ,
                                          const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    *this = CAssetLayoutMigrationReport();
    locationId = domRootNode.GetAttributeValueOrChildValueByName( "locationId" ).AsString( locationId );
    const CORE::CString text = domRootNode.GetAttributeValueOrChildValueByName( "status" ).AsString();
    if ( !text.IsNULLOrEmpty() )
        status = AssetStoreStatusFromString( text );
    dryRun = domRootNode.GetAttributeValueOrChildValueByName( "dryRun" ).AsBool( dryRun, true );
    packsInOldLayout = domRootNode.GetAttributeValueOrChildValueByName( "packsInOldLayout" ).AsUInt64( packsInOldLayout, true );
    packsMigrated = domRootNode.GetAttributeValueOrChildValueByName( "packsMigrated" ).AsUInt64( packsMigrated, true );
    packsRefused = domRootNode.GetAttributeValueOrChildValueByName( "packsRefused" ).AsUInt64( packsRefused, true );
    domRootNode.GetValuesOfChildByName( "actions", actions );
    domRootNode.GetValuesOfChildByName( "problems", problems );
    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CAssetLayoutMigrationReport::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::ASSETSTORE::CAssetLayoutMigrationReport";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CAssetLayoutMigrationReport::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CAssetLayoutMigrationReport( *this );
}

/*-------------------------------------------------------------------------*/

const char*
CAssetLayoutMigrationReport::GetCloneableTypeName( void ) const
{GUCEF_TRACE;

    return CloneableTypeName();
}

/*-------------------------------------------------------------------------*/

const char*
CAssetLayoutMigrationReport::CloneableTypeName( void )
{GUCEF_TRACE;

    return "GUCEF::ASSETSTORE::CAssetLayoutMigrationReport";
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
