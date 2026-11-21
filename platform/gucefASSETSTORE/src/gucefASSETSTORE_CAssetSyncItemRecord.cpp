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

#include "gucefASSETSTORE_CAssetSyncItemRecord.h"

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

CAssetSyncItemRecord::CAssetSyncItemRecord( void )
    : CORE::CIDataNodeSerializable()
    , sourceId()
    , itemId()
    , lastSeenMs( 0 )
    , listedRevision()
    , listedChangedMs( 0 )
    , listingFingerprint()
    , formatFingerprint()
    , enrichedMs( 0 )
    , sizeEstimate( 0 )
    , available( true )
    , unavailableReason()
    , attempts( 0 )
    , lastAttemptMs( 0 )
    , nextAttemptMs( 0 )
    , lastStatus()
    , lastError()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetSyncItemRecord::~CAssetSyncItemRecord()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CAssetSyncItemRecord::Serialize( CORE::CDataNode& domRootNode                        ,
                                 const CORE::CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    bool success = domRootNode.SetAttribute( "sourceId", sourceId );
    success = domRootNode.SetAttribute( "itemId", itemId ) && success;
    success = domRootNode.SetAttribute( "lastSeenMs", lastSeenMs ) && success;
    success = domRootNode.SetAttribute( "listedRevision", listedRevision ) && success;
    success = domRootNode.SetAttribute( "listedChangedMs", listedChangedMs ) && success;
    success = domRootNode.SetAttribute( "listingFingerprint", listingFingerprint ) && success;
    success = domRootNode.SetAttribute( "formatFingerprint", formatFingerprint ) && success;
    success = domRootNode.SetAttribute( "enrichedMs", enrichedMs ) && success;
    success = domRootNode.SetAttribute( "sizeEstimate", sizeEstimate ) && success;
    success = domRootNode.SetAttribute( "available", available ) && success;
    success = domRootNode.SetAttribute( "unavailableReason", unavailableReason ) && success;
    success = domRootNode.SetAttribute( "attempts", attempts ) && success;
    success = domRootNode.SetAttribute( "lastAttemptMs", lastAttemptMs ) && success;
    success = domRootNode.SetAttribute( "nextAttemptMs", nextAttemptMs ) && success;
    success = domRootNode.SetAttribute( "lastStatus", lastStatus ) && success;
    success = domRootNode.SetAttribute( "lastError", lastError ) && success;
    return success;
}

/*-------------------------------------------------------------------------*/

bool
CAssetSyncItemRecord::Deserialize( const CORE::CDataNode& domRootNode                  ,
                                   const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    sourceId = domRootNode.GetAttributeValueOrChildValueByName( "sourceId" ).AsString();
    itemId = domRootNode.GetAttributeValueOrChildValueByName( "itemId" ).AsString();
    lastSeenMs = domRootNode.GetAttributeValueOrChildValueByName( "lastSeenMs" ).AsInt64( 0, true );
    listedRevision = domRootNode.GetAttributeValueOrChildValueByName( "listedRevision" ).AsString();
    listedChangedMs = domRootNode.GetAttributeValueOrChildValueByName( "listedChangedMs" ).AsInt64( 0, true );
    listingFingerprint = domRootNode.GetAttributeValueOrChildValueByName( "listingFingerprint" ).AsString();
    formatFingerprint = domRootNode.GetAttributeValueOrChildValueByName( "formatFingerprint" ).AsString();
    enrichedMs = domRootNode.GetAttributeValueOrChildValueByName( "enrichedMs" ).AsInt64( 0, true );
    sizeEstimate = domRootNode.GetAttributeValueOrChildValueByName( "sizeEstimate" ).AsInt64( 0, true );
    available = domRootNode.GetAttributeValueOrChildValueByName( "available" ).AsBool( true, true );
    unavailableReason = domRootNode.GetAttributeValueOrChildValueByName( "unavailableReason" ).AsString();
    attempts = domRootNode.GetAttributeValueOrChildValueByName( "attempts" ).AsUInt32( 0, true );
    lastAttemptMs = domRootNode.GetAttributeValueOrChildValueByName( "lastAttemptMs" ).AsInt64( 0, true );
    nextAttemptMs = domRootNode.GetAttributeValueOrChildValueByName( "nextAttemptMs" ).AsInt64( 0, true );
    lastStatus = domRootNode.GetAttributeValueOrChildValueByName( "lastStatus" ).AsString();
    lastError = domRootNode.GetAttributeValueOrChildValueByName( "lastError" ).AsString();
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CAssetSyncItemRecord::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CAssetSyncItemRecord( *this );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CAssetSyncItemRecord::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::ASSETSTORE::CAssetSyncItemRecord";
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
