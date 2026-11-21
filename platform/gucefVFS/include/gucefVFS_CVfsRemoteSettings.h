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

#ifndef GUCEF_VFS_CVFSREMOTESETTINGS_H
#define GUCEF_VFS_CVFSREMOTESETTINGS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_VFS_MACROS_H
#include "gucefVFS_macros.h"
#define GUCEF_VFS_MACROS_H
#endif /* GUCEF_VFS_MACROS_H ? */

#ifndef GUCEF_VFS_ETYPES_H
#include "gucefVFS_ETypes.h"
#define GUCEF_VFS_ETYPES_H
#endif /* GUCEF_VFS_ETYPES_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace VFS {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The settings of the generic remote archive layer (CVfsRemoteArchiveCore): what it keeps in memory, how it reads ahead */
class GUCEF_VFS_PUBLIC_CPP CVfsRemoteSettings
{
    public:

    CORE::CString cacheIdentity;        /**< the name of the remote place in the block cache: the cache never interprets it (at most 256 bytes) */
    UInt32 metadataTtlMs;               /**< how long metadata and listings count as fresh (default 3000); 0: never fresh, always asked */
    UInt32 maxMetadataEntries;          /**< the records and listings kept in memory (default 4096) */
    UInt32 maxDigestHints;              /**< the digest hints kept (default 4096) */
    UInt32 readAheadAfterBlocks;        /**< sequential blocks read before read ahead starts (default 2) */
    UInt32 readAheadBlocks;             /**< the blocks fetched ahead in the same run (default 8) */
    UInt32 maxBlocksPerFetch;           /**< the blocks in one run asked of a source (default 16) */
    bool verifyCompleteFiles;           /**< a file that became complete is checked against its digest when one is known (default true) */
    bool offlineListings;               /**< a listing is made from the block cache when the link is down and nothing else is known (default true) */
    bool refetchPinned;                 /**< a kept (pinned) file whose current revision is not held completely is fetched again in the background while the link is up, and the pin moves to it (default true) */

    CVfsRemoteSettings( void );

    CVfsRemoteSettings( const CVfsRemoteSettings& src );

    CVfsRemoteSettings& operator=( const CVfsRemoteSettings& src );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace VFS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_VFS_CVFSREMOTESETTINGS_H ? */
