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

#include "gucefVFS_CVfsRemoteStatistics.h"

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

CVfsRemoteStatistics::CVfsRemoteStatistics( void )
{GUCEF_TRACE;

    Reset();
}

/*-------------------------------------------------------------------------*/

void
CVfsRemoteStatistics::Reset( void )
{GUCEF_TRACE;

    bytesFetched = 0;
    fetchCalls = 0;
    readAheadBlocks = 0;
    listCalls = 0;
    statCalls = 0;
    openCalls = 0;
    busyRetries = 0;
    policyMisses = 0;
    digestMismatches = 0;
    filesVerified = 0;
    fetchJobsAccepted = 0;
    fetchJobsMerged = 0;
    fetchJobsRefused = 0;
    fetchJobsFailed = 0;
    fetchJobsCancelled = 0;
    eventsRaised = 0;
    offlineReads = 0;
    offlineMisses = 0;
    staleServes = 0;
    maxBytes = 0;
    usedBytes = 0;
    pinnedBytes = 0;
    entries = 0;
    pinnedEntries = 0;
    blockHits = 0;
    blockMisses = 0;
    blocksStored = 0;
    corruptBlocks = 0;
    evictedEntries = 0;
    evictedBytes = 0;
    supersededEvictions = 0;
    pinsRefused = 0;
    storeFailures = 0;
    openFiles = 0;
    fileReopens = 0;
    handlesOpen = 0;
    handlesPeak = 0;
    handleEvictions = 0;
    handleReopens = 0;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace VFS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
