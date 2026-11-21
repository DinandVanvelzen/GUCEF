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

#include "gucefASSETSTORE_CRemoteFilesSettings.h"

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

CRemoteFilesSettings::CRemoteFilesSettings( void )
    : CORE::CIDataNodeSerializable()
    , cacheFolder( CORE::CString() )
    , shareName( "assets" )
    , maxCacheBytes( 20ULL * 1024 * 1024 * 1024 )
    , readAheadAfterBlocks( 2 )
    , readAheadBlocks( 8 )
    , maxBlocksPerRequest( 16 )
    , maxListEntries( 100000 )
    , listPageEntries( 1000 )
    , metadataEntries( 200000 )
    , metadataTtlMs( 3000 )
    , negativeTtlMs( 3000 )
    , blockingReadTimeoutMs( 30000 )
    , openWithoutPrefetchBytes( 8 * 1024 * 1024 )
    , maxPrefetchBytes( 2ULL * 1024 * 1024 * 1024 )
    , refetchPinned( true )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CRemoteFilesSettings::~CRemoteFilesSettings()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CRemoteFilesSettings::Serialize( CORE::CDataNode& domRootNode                        ,
                                 const CORE::CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    bool success = true;
    success = domRootNode.SetAttribute( "cacheFolder", cacheFolder ) && success;
    success = domRootNode.SetAttribute( "shareName", shareName ) && success;
    success = domRootNode.SetAttribute( "maxCacheBytes", maxCacheBytes ) && success;
    success = domRootNode.SetAttribute( "readAheadAfterBlocks", readAheadAfterBlocks ) && success;
    success = domRootNode.SetAttribute( "readAheadBlocks", readAheadBlocks ) && success;
    success = domRootNode.SetAttribute( "maxBlocksPerRequest", maxBlocksPerRequest ) && success;
    success = domRootNode.SetAttribute( "maxListEntries", maxListEntries ) && success;
    success = domRootNode.SetAttribute( "listPageEntries", listPageEntries ) && success;
    success = domRootNode.SetAttribute( "metadataEntries", metadataEntries ) && success;
    success = domRootNode.SetAttribute( "metadataTtlMs", metadataTtlMs ) && success;
    success = domRootNode.SetAttribute( "negativeTtlMs", negativeTtlMs ) && success;
    success = domRootNode.SetAttribute( "blockingReadTimeoutMs", blockingReadTimeoutMs ) && success;
    success = domRootNode.SetAttribute( "openWithoutPrefetchBytes", openWithoutPrefetchBytes ) && success;
    success = domRootNode.SetAttribute( "maxPrefetchBytes", maxPrefetchBytes ) && success;
    success = domRootNode.SetAttribute( "refetchPinned", refetchPinned ) && success;
    return success;
}

/*-------------------------------------------------------------------------*/

bool
CRemoteFilesSettings::Deserialize( const CORE::CDataNode& domRootNode                  ,
                                   const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    CRemoteFilesSettings defaults;
    cacheFolder = domRootNode.GetAttributeValueOrChildValueByName( "cacheFolder" ).AsString( defaults.cacheFolder, true );
    shareName = domRootNode.GetAttributeValueOrChildValueByName( "shareName" ).AsString( defaults.shareName, true );
    maxCacheBytes = domRootNode.GetAttributeValueOrChildValueByName( "maxCacheBytes" ).AsUInt64( defaults.maxCacheBytes, true );
    readAheadAfterBlocks = domRootNode.GetAttributeValueOrChildValueByName( "readAheadAfterBlocks" ).AsUInt32( defaults.readAheadAfterBlocks, true );
    readAheadBlocks = domRootNode.GetAttributeValueOrChildValueByName( "readAheadBlocks" ).AsUInt32( defaults.readAheadBlocks, true );
    maxBlocksPerRequest = domRootNode.GetAttributeValueOrChildValueByName( "maxBlocksPerRequest" ).AsUInt32( defaults.maxBlocksPerRequest, true );
    maxListEntries = domRootNode.GetAttributeValueOrChildValueByName( "maxListEntries" ).AsUInt32( defaults.maxListEntries, true );
    listPageEntries = domRootNode.GetAttributeValueOrChildValueByName( "listPageEntries" ).AsUInt32( defaults.listPageEntries, true );
    metadataEntries = domRootNode.GetAttributeValueOrChildValueByName( "metadataEntries" ).AsUInt32( defaults.metadataEntries, true );
    metadataTtlMs = domRootNode.GetAttributeValueOrChildValueByName( "metadataTtlMs" ).AsUInt32( defaults.metadataTtlMs, true );
    negativeTtlMs = domRootNode.GetAttributeValueOrChildValueByName( "negativeTtlMs" ).AsUInt32( defaults.negativeTtlMs, true );
    blockingReadTimeoutMs = domRootNode.GetAttributeValueOrChildValueByName( "blockingReadTimeoutMs" ).AsUInt32( defaults.blockingReadTimeoutMs, true );
    openWithoutPrefetchBytes = domRootNode.GetAttributeValueOrChildValueByName( "openWithoutPrefetchBytes" ).AsUInt32( defaults.openWithoutPrefetchBytes, true );
    maxPrefetchBytes = domRootNode.GetAttributeValueOrChildValueByName( "maxPrefetchBytes" ).AsUInt64( defaults.maxPrefetchBytes, true );
    refetchPinned = domRootNode.GetAttributeValueOrChildValueByName( "refetchPinned" ).AsBool( defaults.refetchPinned, true );
    if ( shareName.IsNULLOrEmpty() )
        shareName = defaults.shareName;
    if ( 0 == maxBlocksPerRequest )
        maxBlocksPerRequest = defaults.maxBlocksPerRequest;
    if ( 0 == listPageEntries )
        listPageEntries = defaults.listPageEntries;
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CRemoteFilesSettings::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CRemoteFilesSettings( *this );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CRemoteFilesSettings::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::ASSETSTORE::CRemoteFilesSettings";
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
