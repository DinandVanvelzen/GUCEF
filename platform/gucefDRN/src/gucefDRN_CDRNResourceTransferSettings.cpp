/*
 *  gucefDRN: GUCEF module providing RAD networking trough data replication
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

#include "gucefDRN_CDRNResourceTransferSettings.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRN {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CDRNResourceTransferSettings::ClassTypeName = "GUCEF::DRN::CDRNResourceTransferSettings";

/*-------------------------------------------------------------------------*/

CDRNResourceTransferSettings::CDRNResourceTransferSettings( void )
    : CORE::CIConfigurable()
    , maxOpenHandlesPerPeer( 32 )
    , maxOpenHandles( 4096 )
    , maxActiveReadsPerPeer( 8 )
    , maxActiveReads( 256 )
    , readAheadChunks( 8 )
    , maxBufferedBytes( 32 * 1024 * 1024 )
    , maxBufferedBytesPerPeer( 0 )
    , maxLinkBacklogBytes( 512 * 1024 )
    , maxUnackedMessages( 256 )
    , maxUploadBytesPerSecond( 0 )
    , maxUploadBytesPerSecondPerPeer( 0 )
    , maxConcurrentTreeBuilds( 1 )
    , maxTreeResourceBytes( (UInt64) 4 * 1024 * 1024 * 1024 )
    , maxTreeCacheBytes( 64 * 1024 * 1024 )
    , treeCacheDirectory()
    , treeCacheMaxDiskBytes( (UInt64) 1024 * 1024 * 1024 )
    , treeCacheMaxAgeSeconds( 30 * 24 * 3600 )
    , handleIdleTimeoutMs( 300000 )
    , maxNamespaceStatPaths( 256 )
    , maxNamespacePageEntries( 1000 )
    , maxNamespaceAnswerBytes( 512 * 1024 )
    , maxNamespaceListingsPerPeer( 8 )
    , maxNamespaceListings( 256 )
    , namespaceListingLifetimeMs( 60000 )
    , maxNamespaceRequestsPerPeer( 16 )
    , maxNamespaceRequests( 512 )
    , announceLimits( true )
    , maxCreditChunksPerLink( 64 )
    , maxDownloadBytesPerSecond( 0 )
    , receiveLimitSharePercent( 75 )
    , stateSaveIntervalMs( 1000 )
    , allowUnprotectedLinks( false )
    , threadPoolName( CORE::CString::Empty )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDRNResourceTransferSettings::~CDRNResourceTransferSettings()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CDRNResourceTransferSettings::SaveConfig( CORE::CDataNode& cfg ) const
{GUCEF_TRACE;

    bool totalSuccess = true;
    totalSuccess = cfg.SetAttribute( "maxOpenHandlesPerPeer", maxOpenHandlesPerPeer ) && totalSuccess;
    totalSuccess = cfg.SetAttribute( "maxOpenHandles", maxOpenHandles ) && totalSuccess;
    totalSuccess = cfg.SetAttribute( "maxActiveReadsPerPeer", maxActiveReadsPerPeer ) && totalSuccess;
    totalSuccess = cfg.SetAttribute( "maxActiveReads", maxActiveReads ) && totalSuccess;
    totalSuccess = cfg.SetAttribute( "readAheadChunks", readAheadChunks ) && totalSuccess;
    totalSuccess = cfg.SetAttribute( "maxBufferedBytes", maxBufferedBytes ) && totalSuccess;
    totalSuccess = cfg.SetAttribute( "maxBufferedBytesPerPeer", maxBufferedBytesPerPeer ) && totalSuccess;
    totalSuccess = cfg.SetAttribute( "maxLinkBacklogBytes", maxLinkBacklogBytes ) && totalSuccess;
    totalSuccess = cfg.SetAttribute( "maxUnackedMessages", maxUnackedMessages ) && totalSuccess;
    totalSuccess = cfg.SetAttribute( "maxUploadBytesPerSecond", maxUploadBytesPerSecond ) && totalSuccess;
    totalSuccess = cfg.SetAttribute( "maxUploadBytesPerSecondPerPeer", maxUploadBytesPerSecondPerPeer ) && totalSuccess;
    totalSuccess = cfg.SetAttribute( "maxConcurrentTreeBuilds", maxConcurrentTreeBuilds ) && totalSuccess;
    totalSuccess = cfg.SetAttribute( "maxTreeResourceBytes", maxTreeResourceBytes ) && totalSuccess;
    totalSuccess = cfg.SetAttribute( "maxTreeCacheBytes", maxTreeCacheBytes ) && totalSuccess;
    totalSuccess = cfg.SetAttribute( "treeCacheDirectory", treeCacheDirectory ) && totalSuccess;
    totalSuccess = cfg.SetAttribute( "treeCacheMaxDiskBytes", treeCacheMaxDiskBytes ) && totalSuccess;
    totalSuccess = cfg.SetAttribute( "treeCacheMaxAgeSeconds", treeCacheMaxAgeSeconds ) && totalSuccess;
    totalSuccess = cfg.SetAttribute( "handleIdleTimeoutMs", handleIdleTimeoutMs ) && totalSuccess;
    totalSuccess = cfg.SetAttribute( "maxNamespaceStatPaths", maxNamespaceStatPaths ) && totalSuccess;
    totalSuccess = cfg.SetAttribute( "maxNamespacePageEntries", maxNamespacePageEntries ) && totalSuccess;
    totalSuccess = cfg.SetAttribute( "maxNamespaceAnswerBytes", maxNamespaceAnswerBytes ) && totalSuccess;
    totalSuccess = cfg.SetAttribute( "maxNamespaceListingsPerPeer", maxNamespaceListingsPerPeer ) && totalSuccess;
    totalSuccess = cfg.SetAttribute( "maxNamespaceListings", maxNamespaceListings ) && totalSuccess;
    totalSuccess = cfg.SetAttribute( "namespaceListingLifetimeMs", namespaceListingLifetimeMs ) && totalSuccess;
    totalSuccess = cfg.SetAttribute( "maxNamespaceRequestsPerPeer", maxNamespaceRequestsPerPeer ) && totalSuccess;
    totalSuccess = cfg.SetAttribute( "maxNamespaceRequests", maxNamespaceRequests ) && totalSuccess;
    totalSuccess = cfg.SetAttribute( "announceLimits", announceLimits ) && totalSuccess;
    totalSuccess = cfg.SetAttribute( "maxCreditChunksPerLink", maxCreditChunksPerLink ) && totalSuccess;
    totalSuccess = cfg.SetAttribute( "maxDownloadBytesPerSecond", maxDownloadBytesPerSecond ) && totalSuccess;
    totalSuccess = cfg.SetAttribute( "receiveLimitSharePercent", receiveLimitSharePercent ) && totalSuccess;
    totalSuccess = cfg.SetAttribute( "stateSaveIntervalMs", stateSaveIntervalMs ) && totalSuccess;
    totalSuccess = cfg.SetAttribute( "allowUnprotectedLinks", allowUnprotectedLinks ) && totalSuccess;
    totalSuccess = cfg.SetAttribute( "threadPoolName", threadPoolName ) && totalSuccess;
    return totalSuccess;
}

/*-------------------------------------------------------------------------*/

bool
CDRNResourceTransferSettings::LoadConfig( const CORE::CDataNode& cfg )
{GUCEF_TRACE;

    maxOpenHandlesPerPeer = cfg.GetAttributeValueOrChildValueByName( "maxOpenHandlesPerPeer" ).AsUInt32( maxOpenHandlesPerPeer, true );
    maxOpenHandles = cfg.GetAttributeValueOrChildValueByName( "maxOpenHandles" ).AsUInt32( maxOpenHandles, true );
    maxActiveReadsPerPeer = cfg.GetAttributeValueOrChildValueByName( "maxActiveReadsPerPeer" ).AsUInt32( maxActiveReadsPerPeer, true );
    maxActiveReads = cfg.GetAttributeValueOrChildValueByName( "maxActiveReads" ).AsUInt32( maxActiveReads, true );
    readAheadChunks = cfg.GetAttributeValueOrChildValueByName( "readAheadChunks" ).AsUInt32( readAheadChunks, true );
    maxBufferedBytes = cfg.GetAttributeValueOrChildValueByName( "maxBufferedBytes" ).AsUInt32( maxBufferedBytes, true );
    maxBufferedBytesPerPeer = cfg.GetAttributeValueOrChildValueByName( "maxBufferedBytesPerPeer" ).AsUInt32( maxBufferedBytesPerPeer, true );
    maxLinkBacklogBytes = cfg.GetAttributeValueOrChildValueByName( "maxLinkBacklogBytes" ).AsUInt32( maxLinkBacklogBytes, true );
    maxUnackedMessages = cfg.GetAttributeValueOrChildValueByName( "maxUnackedMessages" ).AsUInt32( maxUnackedMessages, true );
    maxUploadBytesPerSecond = cfg.GetAttributeValueOrChildValueByName( "maxUploadBytesPerSecond" ).AsUInt32( maxUploadBytesPerSecond, true );
    maxUploadBytesPerSecondPerPeer = cfg.GetAttributeValueOrChildValueByName( "maxUploadBytesPerSecondPerPeer" ).AsUInt32( maxUploadBytesPerSecondPerPeer, true );
    maxConcurrentTreeBuilds = cfg.GetAttributeValueOrChildValueByName( "maxConcurrentTreeBuilds" ).AsUInt32( maxConcurrentTreeBuilds, true );
    maxTreeResourceBytes = cfg.GetAttributeValueOrChildValueByName( "maxTreeResourceBytes" ).AsUInt64( maxTreeResourceBytes, true );
    maxTreeCacheBytes = cfg.GetAttributeValueOrChildValueByName( "maxTreeCacheBytes" ).AsUInt32( maxTreeCacheBytes, true );
    treeCacheDirectory = cfg.GetAttributeValueOrChildValueByName( "treeCacheDirectory" ).AsString( treeCacheDirectory, true );
    treeCacheMaxDiskBytes = cfg.GetAttributeValueOrChildValueByName( "treeCacheMaxDiskBytes" ).AsUInt64( treeCacheMaxDiskBytes, true );
    treeCacheMaxAgeSeconds = cfg.GetAttributeValueOrChildValueByName( "treeCacheMaxAgeSeconds" ).AsUInt32( treeCacheMaxAgeSeconds, true );
    handleIdleTimeoutMs = cfg.GetAttributeValueOrChildValueByName( "handleIdleTimeoutMs" ).AsUInt32( handleIdleTimeoutMs, true );
    maxNamespaceStatPaths = cfg.GetAttributeValueOrChildValueByName( "maxNamespaceStatPaths" ).AsUInt32( maxNamespaceStatPaths, true );
    maxNamespacePageEntries = cfg.GetAttributeValueOrChildValueByName( "maxNamespacePageEntries" ).AsUInt32( maxNamespacePageEntries, true );
    maxNamespaceAnswerBytes = cfg.GetAttributeValueOrChildValueByName( "maxNamespaceAnswerBytes" ).AsUInt32( maxNamespaceAnswerBytes, true );
    maxNamespaceListingsPerPeer = cfg.GetAttributeValueOrChildValueByName( "maxNamespaceListingsPerPeer" ).AsUInt32( maxNamespaceListingsPerPeer, true );
    maxNamespaceListings = cfg.GetAttributeValueOrChildValueByName( "maxNamespaceListings" ).AsUInt32( maxNamespaceListings, true );
    namespaceListingLifetimeMs = cfg.GetAttributeValueOrChildValueByName( "namespaceListingLifetimeMs" ).AsUInt32( namespaceListingLifetimeMs, true );
    maxNamespaceRequestsPerPeer = cfg.GetAttributeValueOrChildValueByName( "maxNamespaceRequestsPerPeer" ).AsUInt32( maxNamespaceRequestsPerPeer, true );
    maxNamespaceRequests = cfg.GetAttributeValueOrChildValueByName( "maxNamespaceRequests" ).AsUInt32( maxNamespaceRequests, true );
    announceLimits = cfg.GetAttributeValueOrChildValueByName( "announceLimits" ).AsBool( announceLimits, true );
    maxCreditChunksPerLink = cfg.GetAttributeValueOrChildValueByName( "maxCreditChunksPerLink" ).AsUInt32( maxCreditChunksPerLink, true );
    maxDownloadBytesPerSecond = cfg.GetAttributeValueOrChildValueByName( "maxDownloadBytesPerSecond" ).AsUInt32( maxDownloadBytesPerSecond, true );
    receiveLimitSharePercent = cfg.GetAttributeValueOrChildValueByName( "receiveLimitSharePercent" ).AsUInt32( receiveLimitSharePercent, true );
    stateSaveIntervalMs = cfg.GetAttributeValueOrChildValueByName( "stateSaveIntervalMs" ).AsUInt32( stateSaveIntervalMs, true );
    allowUnprotectedLinks = cfg.GetAttributeValueOrChildValueByName( "allowUnprotectedLinks" ).AsBool( allowUnprotectedLinks, true );
    threadPoolName = cfg.GetAttributeValueOrChildValueByName( "threadPoolName" ).AsString( threadPoolName, true );
    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CDRNResourceTransferSettings::GetClassTypeName( void ) const
{GUCEF_TRACE;

    return ClassTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
