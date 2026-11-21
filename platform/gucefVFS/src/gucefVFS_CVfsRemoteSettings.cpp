/*
 *  gucefVFS: GUCEF module implementing a Virtual File System
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

#include "gucefVFS_CVfsRemoteSettings.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace VFS {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CVfsRemoteSettings::CVfsRemoteSettings( void )
    : cacheIdentity()
    , metadataTtlMs( 3000 )
    , maxMetadataEntries( 4096 )
    , maxDigestHints( 4096 )
    , readAheadAfterBlocks( 2 )
    , readAheadBlocks( 8 )
    , maxBlocksPerFetch( 16 )
    , verifyCompleteFiles( true )
    , offlineListings( true )
    , refetchPinned( true )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CVfsRemoteSettings::CVfsRemoteSettings( const CVfsRemoteSettings& src )
    : cacheIdentity( src.cacheIdentity )
    , metadataTtlMs( src.metadataTtlMs )
    , maxMetadataEntries( src.maxMetadataEntries )
    , maxDigestHints( src.maxDigestHints )
    , readAheadAfterBlocks( src.readAheadAfterBlocks )
    , readAheadBlocks( src.readAheadBlocks )
    , maxBlocksPerFetch( src.maxBlocksPerFetch )
    , verifyCompleteFiles( src.verifyCompleteFiles )
    , offlineListings( src.offlineListings )
    , refetchPinned( src.refetchPinned )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CVfsRemoteSettings&
CVfsRemoteSettings::operator=( const CVfsRemoteSettings& src )
{GUCEF_TRACE;

    if ( this != &src )
    {
        cacheIdentity = src.cacheIdentity;
        metadataTtlMs = src.metadataTtlMs;
        maxMetadataEntries = src.maxMetadataEntries;
        maxDigestHints = src.maxDigestHints;
        readAheadAfterBlocks = src.readAheadAfterBlocks;
        readAheadBlocks = src.readAheadBlocks;
        maxBlocksPerFetch = src.maxBlocksPerFetch;
        verifyCompleteFiles = src.verifyCompleteFiles;
        offlineListings = src.offlineListings;
        refetchPinned = src.refetchPinned;
    }
    return *this;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace VFS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
