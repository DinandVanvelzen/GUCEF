/*
 *  vfspluginDRN: GUCEF VFS plugin presenting a share of another DRN node as a VFS archive
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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_DRN_CDRNRESOURCEIOACCESS_H
#include "gucefDRN_CDRNResourceIOAccess.h"
#define GUCEF_DRN_CDRNRESOURCEIOACCESS_H
#endif /* GUCEF_DRN_CDRNRESOURCEIOACCESS_H ? */

#include "vfspluginDRN_CDrnBridgeSettings.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace VFSPLUGIN {
namespace DRNBRIDGE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

const char* const CDrnBridgeSettings::KeyMetadataTtlMs = "metadataTtlMs";
const char* const CDrnBridgeSettings::KeyNegativeTtlMs = "negativeTtlMs";
const char* const CDrnBridgeSettings::KeyMaxCachedEntries = "maxCachedEntries";
const char* const CDrnBridgeSettings::KeyServeStaleWhenOffline = "serveStaleWhenOffline";
const char* const CDrnBridgeSettings::KeyMaxOpenHandles = "maxOpenHandles";
const char* const CDrnBridgeSettings::KeyChunkCacheChunks = "chunkCacheChunks";
const char* const CDrnBridgeSettings::KeyReadAheadChunks = "readAheadChunks";
const char* const CDrnBridgeSettings::KeyCallTimeoutMs = "callTimeoutMs";
const char* const CDrnBridgeSettings::KeyOpenTimeoutMs = "openTimeoutMs";
const char* const CDrnBridgeSettings::KeyListPageEntries = "listPageEntries";
const char* const CDrnBridgeSettings::KeyMaxEntriesPerDirectory = "maxEntriesPerDirectory";
const char* const CDrnBridgeSettings::KeyMaxWalkedDirectories = "maxWalkedDirectories";
const char* const CDrnBridgeSettings::KeyVerifyMode = "verifyMode";
const char* const CDrnBridgeSettings::KeyBusyRetries = "busyRetries";
const char* const CDrnBridgeSettings::KeyBusyBackoffMs = "busyBackoffMs";
const char* const CDrnBridgeSettings::KeyBlockCacheFolder = "blockCacheFolder";
const char* const CDrnBridgeSettings::KeyBlockCacheMaxBytes = "blockCacheMaxBytes";
const char* const CDrnBridgeSettings::KeyBlockCacheBlockBytes = "blockCacheBlockBytes";
const char* const CDrnBridgeSettings::KeyCacheIdentity = "cacheIdentity";
const char* const CDrnBridgeSettings::KeyReadAheadAfterBlocks = "readAheadAfterBlocks";
const char* const CDrnBridgeSettings::KeyReadAheadBlocks = "readAheadBlocks";
const char* const CDrnBridgeSettings::KeyMaxBlocksPerFetch = "maxBlocksPerFetch";
const char* const CDrnBridgeSettings::KeyMaxBackgroundFetches = "maxBackgroundFetches";
const char* const CDrnBridgeSettings::KeyMaxQueuedFetches = "maxQueuedFetches";
const char* const CDrnBridgeSettings::KeyFetchThreadPool = "fetchThreadPool";
const char* const CDrnBridgeSettings::KeyVerifyCompleteFiles = "verifyCompleteFiles";
const char* const CDrnBridgeSettings::KeyRefetchPinned = "refetchPinned";
const char* const CDrnBridgeSettings::KeyOfflineListings = "offlineListings";
const char* const CDrnBridgeSettings::KeyOpenRequiresCompleteFile = "openRequiresCompleteFile";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CDrnBridgeSettings::CDrnBridgeSettings( void )
    : metadataTtlMs( 3000 )
    , negativeTtlMs( 1000 )
    , maxCachedEntries( 50000 )
    , serveStaleWhenOffline( true )
    , maxOpenHandles( 12 )
    , chunkCacheChunks( DRN::CDRNResourceIOAccess::DefaultCacheChunks )
    , readAheadChunks( DRN::CDRNResourceIOAccess::DefaultReadAheadChunks )
    , callTimeoutMs( 15000 )
    , openTimeoutMs( 15000 )
    , listPageEntries( 0 )
    , maxEntriesPerDirectory( 500000 )
    , maxWalkedDirectories( 100000 )
    , verifyMode( DRN::CDRNResourceTransferOptions::VERIFY_LINK )
    , busyRetries( 8 )
    , busyBackoffMs( 25 )
    , blockCacheFolder()
    , blockCacheMaxBytes( 0 )
    , blockCacheBlockBytes( 0 )
    , cacheIdentity()
    , readAheadAfterBlocks( 2 )
    , readAheadBlocks( 8 )
    , maxBlocksPerFetch( 16 )
    , maxBackgroundFetches( 8 )
    , maxQueuedFetches( 256 )
    , fetchThreadPool( "VfsRemoteOps" )
    , verifyCompleteFiles( true )
    , refetchPinned( true )
    , offlineListings( true )
    , openRequiresCompleteFile( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CDrnBridgeSettings::LoadFromArchiveSettings( const VFS::CArchiveSettings& settings )
{GUCEF_TRACE;

    return LoadFromValueList( settings.GetArchiveSpecificSettings() );
}

/*-------------------------------------------------------------------------*/

bool
CDrnBridgeSettings::LoadFromValueList( const CORE::CValueList& values )
{GUCEF_TRACE;

    bool allGood = true;
    CORE::CVariant value;

    if ( values.TryGetValue( KeyMetadataTtlMs, value ) )
        metadataTtlMs = value.AsUInt32( metadataTtlMs, true );
    if ( values.TryGetValue( KeyNegativeTtlMs, value ) )
        negativeTtlMs = value.AsUInt32( negativeTtlMs, true );
    if ( values.TryGetValue( KeyMaxCachedEntries, value ) )
        maxCachedEntries = value.AsUInt32( maxCachedEntries, true );
    if ( values.TryGetValue( KeyServeStaleWhenOffline, value ) )
        serveStaleWhenOffline = value.AsBool( serveStaleWhenOffline, true );
    if ( values.TryGetValue( KeyMaxOpenHandles, value ) )
        maxOpenHandles = value.AsUInt32( maxOpenHandles, true );
    if ( values.TryGetValue( KeyChunkCacheChunks, value ) )
        chunkCacheChunks = value.AsUInt32( chunkCacheChunks, true );
    if ( values.TryGetValue( KeyReadAheadChunks, value ) )
        readAheadChunks = value.AsUInt32( readAheadChunks, true );
    if ( values.TryGetValue( KeyCallTimeoutMs, value ) )
        callTimeoutMs = value.AsUInt32( callTimeoutMs, true );
    if ( values.TryGetValue( KeyOpenTimeoutMs, value ) )
        openTimeoutMs = value.AsUInt32( openTimeoutMs, true );
    if ( values.TryGetValue( KeyListPageEntries, value ) )
        listPageEntries = value.AsUInt32( listPageEntries, true );
    if ( values.TryGetValue( KeyMaxEntriesPerDirectory, value ) )
        maxEntriesPerDirectory = value.AsUInt32( maxEntriesPerDirectory, true );
    if ( values.TryGetValue( KeyMaxWalkedDirectories, value ) )
        maxWalkedDirectories = value.AsUInt32( maxWalkedDirectories, true );
    if ( values.TryGetValue( KeyBusyRetries, value ) )
        busyRetries = value.AsUInt32( busyRetries, true );
    if ( values.TryGetValue( KeyBusyBackoffMs, value ) )
        busyBackoffMs = value.AsUInt32( busyBackoffMs, true );

    if ( values.TryGetValue( KeyBlockCacheFolder, value ) )
        blockCacheFolder = value.AsString( blockCacheFolder, true );
    if ( values.TryGetValue( KeyBlockCacheMaxBytes, value ) )
        blockCacheMaxBytes = value.AsUInt64( blockCacheMaxBytes, true );
    if ( values.TryGetValue( KeyBlockCacheBlockBytes, value ) )
        blockCacheBlockBytes = value.AsUInt32( blockCacheBlockBytes, true );
    if ( values.TryGetValue( KeyCacheIdentity, value ) )
        cacheIdentity = value.AsString( cacheIdentity, true );
    if ( values.TryGetValue( KeyReadAheadAfterBlocks, value ) )
        readAheadAfterBlocks = value.AsUInt32( readAheadAfterBlocks, true );
    if ( values.TryGetValue( KeyReadAheadBlocks, value ) )
        readAheadBlocks = value.AsUInt32( readAheadBlocks, true );
    if ( values.TryGetValue( KeyMaxBlocksPerFetch, value ) )
        maxBlocksPerFetch = value.AsUInt32( maxBlocksPerFetch, true );
    if ( values.TryGetValue( KeyMaxBackgroundFetches, value ) )
        maxBackgroundFetches = value.AsUInt32( maxBackgroundFetches, true );
    if ( values.TryGetValue( KeyMaxQueuedFetches, value ) )
        maxQueuedFetches = value.AsUInt32( maxQueuedFetches, true );
    if ( values.TryGetValue( KeyFetchThreadPool, value ) )
        fetchThreadPool = value.AsString( fetchThreadPool, true );
    if ( values.TryGetValue( KeyVerifyCompleteFiles, value ) )
        verifyCompleteFiles = value.AsBool( verifyCompleteFiles, true );
    if ( values.TryGetValue( KeyRefetchPinned, value ) )
        refetchPinned = value.AsBool( refetchPinned, true );
    if ( values.TryGetValue( KeyOfflineListings, value ) )
        offlineListings = value.AsBool( offlineListings, true );
    if ( values.TryGetValue( KeyOpenRequiresCompleteFile, value ) )
        openRequiresCompleteFile = value.AsBool( openRequiresCompleteFile, true );

    if ( values.TryGetValue( KeyVerifyMode, value ) )
    {
        const CORE::CString mode = value.AsString( CORE::CString::Empty, true );
        if ( mode.Equals( "link", false ) )
            verifyMode = DRN::CDRNResourceTransferOptions::VERIFY_LINK;
        else
        if ( mode.Equals( "tree", false ) )
            verifyMode = DRN::CDRNResourceTransferOptions::VERIFY_TREE;
        else
        if ( mode.Equals( "treeIfAvailable", false ) )
            verifyMode = DRN::CDRNResourceTransferOptions::VERIFY_TREE_IF_AVAILABLE;
        else
        {
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "DrnBridgeSettings: \"" << mode << "\" is not a verifyMode (link, tree or treeIfAvailable), keeping the default" );
            allGood = false;
        }
    }

    Normalize();
    return allGood;
}

/*-------------------------------------------------------------------------*/

void
CDrnBridgeSettings::SaveToArchiveConfig( CORE::CDataNode& archiveConfig ) const
{GUCEF_TRACE;

    const CORE::CString prefix = VFS::CArchiveSettings::ArchiveSpecificSettingsPrefix;
    archiveConfig.SetAttribute( ( prefix + KeyMetadataTtlMs ).C_String(), CORE::UInt32ToString( metadataTtlMs ) );
    archiveConfig.SetAttribute( ( prefix + KeyNegativeTtlMs ).C_String(), CORE::UInt32ToString( negativeTtlMs ) );
    archiveConfig.SetAttribute( ( prefix + KeyMaxCachedEntries ).C_String(), CORE::UInt32ToString( maxCachedEntries ) );
    archiveConfig.SetAttribute( ( prefix + KeyServeStaleWhenOffline ).C_String(), CORE::CString( serveStaleWhenOffline ? "true" : "false" ) );
    archiveConfig.SetAttribute( ( prefix + KeyMaxOpenHandles ).C_String(), CORE::UInt32ToString( maxOpenHandles ) );
    archiveConfig.SetAttribute( ( prefix + KeyChunkCacheChunks ).C_String(), CORE::UInt32ToString( chunkCacheChunks ) );
    archiveConfig.SetAttribute( ( prefix + KeyReadAheadChunks ).C_String(), CORE::UInt32ToString( readAheadChunks ) );
    archiveConfig.SetAttribute( ( prefix + KeyCallTimeoutMs ).C_String(), CORE::UInt32ToString( callTimeoutMs ) );
    archiveConfig.SetAttribute( ( prefix + KeyOpenTimeoutMs ).C_String(), CORE::UInt32ToString( openTimeoutMs ) );
    archiveConfig.SetAttribute( ( prefix + KeyListPageEntries ).C_String(), CORE::UInt32ToString( listPageEntries ) );
    archiveConfig.SetAttribute( ( prefix + KeyMaxEntriesPerDirectory ).C_String(), CORE::UInt32ToString( maxEntriesPerDirectory ) );
    archiveConfig.SetAttribute( ( prefix + KeyMaxWalkedDirectories ).C_String(), CORE::UInt32ToString( maxWalkedDirectories ) );
    archiveConfig.SetAttribute( ( prefix + KeyBusyRetries ).C_String(), CORE::UInt32ToString( busyRetries ) );
    archiveConfig.SetAttribute( ( prefix + KeyBusyBackoffMs ).C_String(), CORE::UInt32ToString( busyBackoffMs ) );
    archiveConfig.SetAttribute( ( prefix + KeyBlockCacheFolder ).C_String(), blockCacheFolder );
    archiveConfig.SetAttribute( ( prefix + KeyBlockCacheMaxBytes ).C_String(), CORE::UInt64ToString( blockCacheMaxBytes ) );
    archiveConfig.SetAttribute( ( prefix + KeyBlockCacheBlockBytes ).C_String(), CORE::UInt32ToString( blockCacheBlockBytes ) );
    archiveConfig.SetAttribute( ( prefix + KeyCacheIdentity ).C_String(), cacheIdentity );
    archiveConfig.SetAttribute( ( prefix + KeyReadAheadAfterBlocks ).C_String(), CORE::UInt32ToString( readAheadAfterBlocks ) );
    archiveConfig.SetAttribute( ( prefix + KeyReadAheadBlocks ).C_String(), CORE::UInt32ToString( readAheadBlocks ) );
    archiveConfig.SetAttribute( ( prefix + KeyMaxBlocksPerFetch ).C_String(), CORE::UInt32ToString( maxBlocksPerFetch ) );
    archiveConfig.SetAttribute( ( prefix + KeyMaxBackgroundFetches ).C_String(), CORE::UInt32ToString( maxBackgroundFetches ) );
    archiveConfig.SetAttribute( ( prefix + KeyMaxQueuedFetches ).C_String(), CORE::UInt32ToString( maxQueuedFetches ) );
    archiveConfig.SetAttribute( ( prefix + KeyFetchThreadPool ).C_String(), fetchThreadPool );
    archiveConfig.SetAttribute( ( prefix + KeyVerifyCompleteFiles ).C_String(), CORE::CString( verifyCompleteFiles ? "true" : "false" ) );
    archiveConfig.SetAttribute( ( prefix + KeyRefetchPinned ).C_String(), CORE::CString( refetchPinned ? "true" : "false" ) );
    archiveConfig.SetAttribute( ( prefix + KeyOfflineListings ).C_String(), CORE::CString( offlineListings ? "true" : "false" ) );
    archiveConfig.SetAttribute( ( prefix + KeyOpenRequiresCompleteFile ).C_String(), CORE::CString( openRequiresCompleteFile ? "true" : "false" ) );

    const char* mode = "link";
    if ( DRN::CDRNResourceTransferOptions::VERIFY_TREE == verifyMode )
        mode = "tree";
    else
    if ( DRN::CDRNResourceTransferOptions::VERIFY_TREE_IF_AVAILABLE == verifyMode )
        mode = "treeIfAvailable";
    archiveConfig.SetAttribute( ( prefix + KeyVerifyMode ).C_String(), CORE::CString( mode ) );
}

/*-------------------------------------------------------------------------*/

void
CDrnBridgeSettings::Normalize( void )
{GUCEF_TRACE;

    if ( maxOpenHandles < 1 )
        maxOpenHandles = 1;
    if ( chunkCacheChunks < 1 )
        chunkCacheChunks = 1;
    if ( maxEntriesPerDirectory < 1 )
        maxEntriesPerDirectory = 1;
    if ( maxWalkedDirectories < 1 )
        maxWalkedDirectories = 1;
    if ( callTimeoutMs < 1 )
        callTimeoutMs = 1;
    if ( openTimeoutMs < 1 )
        openTimeoutMs = 1;
    if ( busyRetries > 20 )
        busyRetries = 20;
    if ( maxBlocksPerFetch < 1 )
        maxBlocksPerFetch = 1;
    if ( maxBackgroundFetches < 1 )
        maxBackgroundFetches = 1;
    if ( fetchThreadPool.IsNULLOrEmpty() )
        fetchThreadPool = "VfsRemoteOps";
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNBRIDGE */
}; /* namespace VFSPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
