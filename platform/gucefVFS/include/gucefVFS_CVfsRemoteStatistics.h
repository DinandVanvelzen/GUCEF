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

#ifndef GUCEF_VFS_CVFSREMOTESTATISTICS_H
#define GUCEF_VFS_CVFSREMOTESTATISTICS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

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

/**
 *  What a remote archive counted: the work it did for the remote place, the misses of threads that must not wait, the
 *  background jobs, the offline answers and the state of the block cache it uses (the cache is shared by the archives
 *  that use it, so the cache fields are those of the whole cache). Everything counts from the start of the archive.
 */
class GUCEF_VFS_PUBLIC_CPP CVfsRemoteStatistics
{
    public:

    // what was fetched
    UInt64 bytesFetched;                /**< bytes read from the remote place into the cache or a window */
    UInt64 fetchCalls;                  /**< runs of blocks asked of a source */
    UInt64 readAheadBlocks;             /**< blocks fetched ahead of what a read asked for */

    // calls to the remote place
    UInt64 listCalls;
    UInt64 statCalls;
    UInt64 openCalls;                   /**< sources opened */
    UInt64 busyRetries;                 /**< retries of the remote side's busy answer, as the archive counts them */

    // thread policy
    UInt64 policyMisses;                /**< misses that returned at once because the calling thread must not wait */

    // digests
    UInt64 digestMismatches;
    UInt64 filesVerified;

    // background jobs of this archive
    UInt64 fetchJobsAccepted;
    UInt64 fetchJobsMerged;
    UInt64 fetchJobsRefused;
    UInt64 fetchJobsFailed;
    UInt64 fetchJobsCancelled;
    UInt64 eventsRaised;

    // offline
    UInt64 offlineReads;                /**< reads and opens answered from the cache while the link was down */
    UInt64 offlineMisses;               /**< calls that the cache could not answer while the link was down */
    UInt64 staleServes;                 /**< opens and listings answered from an older, pinned, complete revision because the newest one was not held completely */

    // the block cache
    UInt64 maxBytes;
    UInt64 usedBytes;
    UInt64 pinnedBytes;
    UInt32 entries;
    UInt32 pinnedEntries;
    UInt64 blockHits;
    UInt64 blockMisses;
    UInt64 blocksStored;
    UInt64 corruptBlocks;
    UInt64 evictedEntries;
    UInt64 evictedBytes;
    UInt64 supersededEvictions;
    UInt64 pinsRefused;                 /**< pins refused because the pinned files would pass the size limit of the cache */
    UInt64 storeFailures;
    UInt32 openFiles;
    UInt64 fileReopens;

    // the handles of the transport (an archive that has them fills these)
    UInt32 handlesOpen;
    UInt32 handlesPeak;
    UInt64 handleEvictions;
    UInt64 handleReopens;

    CVfsRemoteStatistics( void );

    /** Sets every field to zero */
    void Reset( void );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace VFS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_VFS_CVFSREMOTESTATISTICS_H ? */
