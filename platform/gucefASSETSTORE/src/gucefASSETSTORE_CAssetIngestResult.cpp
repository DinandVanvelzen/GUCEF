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

#include "gucefASSETSTORE_CAssetIngestResult.h"

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

CAssetIngestResult::CAssetIngestResult( void )
    : CORE::CIDataNodeSerializable()
    , ingestId()
    , packId()
    , status( ASSETSTORE_STATUS_OK )
    , state( ASSETPACK_STATE_STAGED )
    , failedStep( ASSETINGEST_STEP_NONE )
    , message()
    , duplicateOfPackId()
    , repairedFiles()
    , assetCount( 0 )
    , problems()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetIngestResult::~CAssetIngestResult()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CAssetIngestResult::Serialize( CORE::CDataNode& domRootNode                        ,
                               const CORE::CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    bool success = true;
    success = domRootNode.SetAttribute( "ingestId", ingestId ) && success;
    success = domRootNode.SetAttribute( "packId", packId ) && success;
    success = domRootNode.SetAttribute( "status", CORE::CString( AssetStoreStatusToString( status ) ) ) && success;
    success = domRootNode.SetAttribute( "state", CORE::CString( AssetPackStateToString( state ) ) ) && success;
    success = domRootNode.SetAttribute( "failedStep", CORE::CString( AssetIngestStepToString( failedStep ) ) ) && success;
    success = domRootNode.SetAttribute( "message", message ) && success;
    success = domRootNode.SetAttribute( "duplicateOfPackId", duplicateOfPackId ) && success;
    if ( !repairedFiles.empty() )
        success = domRootNode.SetValuesOfChildByName( "repairedFiles", repairedFiles ) && success;
    success = domRootNode.SetAttribute( "assetCount", assetCount ) && success;
    success = domRootNode.SetValuesOfChildByName( "problems", problems ) && success;
    return success;
}

/*-------------------------------------------------------------------------*/

bool
CAssetIngestResult::Deserialize( const CORE::CDataNode& domRootNode                  ,
                                 const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    *this = CAssetIngestResult();
    ingestId = domRootNode.GetAttributeValueOrChildValueByName( "ingestId" ).AsString( ingestId );
    packId = domRootNode.GetAttributeValueOrChildValueByName( "packId" ).AsString( packId );
    CORE::CString text = domRootNode.GetAttributeValueOrChildValueByName( "status" ).AsString();
    if ( !text.IsNULLOrEmpty() )
        status = AssetStoreStatusFromString( text );
    text = domRootNode.GetAttributeValueOrChildValueByName( "state" ).AsString();
    if ( !text.IsNULLOrEmpty() )
        state = AssetPackStateFromString( text );
    text = domRootNode.GetAttributeValueOrChildValueByName( "failedStep" ).AsString();
    if ( !text.IsNULLOrEmpty() )
        failedStep = AssetIngestStepFromString( text );
    message = domRootNode.GetAttributeValueOrChildValueByName( "message" ).AsString( message );
    duplicateOfPackId = domRootNode.GetAttributeValueOrChildValueByName( "duplicateOfPackId" ).AsString( duplicateOfPackId );
    domRootNode.GetValuesOfChildByName( "repairedFiles", repairedFiles );
    assetCount = domRootNode.GetAttributeValueOrChildValueByName( "assetCount" ).AsUInt32( assetCount, true );
    domRootNode.GetValuesOfChildByName( "problems", problems );
    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CAssetIngestResult::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::ASSETSTORE::CAssetIngestResult";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CAssetIngestResult::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CAssetIngestResult( *this );
}

/*-------------------------------------------------------------------------*/

const char*
CAssetIngestResult::GetCloneableTypeName( void ) const
{GUCEF_TRACE;

    return CloneableTypeName();
}

/*-------------------------------------------------------------------------*/

const char*
CAssetIngestResult::CloneableTypeName( void )
{GUCEF_TRACE;

    return "GUCEF::ASSETSTORE::CAssetIngestResult";
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
