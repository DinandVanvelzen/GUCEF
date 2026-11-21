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

#include "gucefASSETSTORE_CAssetVerificationReport.h"

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

CAssetVerificationReport::CAssetVerificationReport( void )
    : CORE::CIDataNodeSerializable()
    , locationId()
    , status( ASSETSTORE_STATUS_OK )
    , packsChecked( 0 )
    , filesChecked( 0 )
    , missingFiles()
    , mismatchedFiles()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetVerificationReport::~CAssetVerificationReport()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CAssetVerificationReport::IsIntact( void ) const
{GUCEF_TRACE;

    return ASSETSTORE_STATUS_OK == status && missingFiles.empty() && mismatchedFiles.empty();
}

/*-------------------------------------------------------------------------*/

bool
CAssetVerificationReport::Serialize( CORE::CDataNode& domRootNode                        ,
                                     const CORE::CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    bool success = true;
    success = domRootNode.SetAttribute( "locationId", locationId ) && success;
    success = domRootNode.SetAttribute( "status", CORE::CString( AssetStoreStatusToString( status ) ) ) && success;
    success = domRootNode.SetAttribute( "packsChecked", packsChecked ) && success;
    success = domRootNode.SetAttribute( "filesChecked", filesChecked ) && success;
    success = domRootNode.SetValuesOfChildByName( "missingFiles", missingFiles ) && success;
    success = domRootNode.SetValuesOfChildByName( "mismatchedFiles", mismatchedFiles ) && success;
    return success;
}

/*-------------------------------------------------------------------------*/

bool
CAssetVerificationReport::Deserialize( const CORE::CDataNode& domRootNode                  ,
                                       const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    *this = CAssetVerificationReport();
    locationId = domRootNode.GetAttributeValueOrChildValueByName( "locationId" ).AsString( locationId );
    const CORE::CString text = domRootNode.GetAttributeValueOrChildValueByName( "status" ).AsString();
    if ( !text.IsNULLOrEmpty() )
        status = AssetStoreStatusFromString( text );
    packsChecked = domRootNode.GetAttributeValueOrChildValueByName( "packsChecked" ).AsUInt64( packsChecked, true );
    filesChecked = domRootNode.GetAttributeValueOrChildValueByName( "filesChecked" ).AsUInt64( filesChecked, true );
    domRootNode.GetValuesOfChildByName( "missingFiles", missingFiles );
    domRootNode.GetValuesOfChildByName( "mismatchedFiles", mismatchedFiles );
    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CAssetVerificationReport::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::ASSETSTORE::CAssetVerificationReport";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CAssetVerificationReport::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CAssetVerificationReport( *this );
}

/*-------------------------------------------------------------------------*/

const char*
CAssetVerificationReport::GetCloneableTypeName( void ) const
{GUCEF_TRACE;

    return CloneableTypeName();
}

/*-------------------------------------------------------------------------*/

const char*
CAssetVerificationReport::CloneableTypeName( void )
{GUCEF_TRACE;

    return "GUCEF::ASSETSTORE::CAssetVerificationReport";
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
