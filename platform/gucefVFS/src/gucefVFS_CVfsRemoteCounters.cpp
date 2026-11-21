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

#include "gucefVFS_CVfsRemoteCounters.h"

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

CVfsRemoteCounters::CVfsRemoteCounters( void )
    : bytesFetched( 0 )
    , fetchCalls( 0 )
    , readAheadBlocks( 0 )
    , listCalls( 0 )
    , statCalls( 0 )
    , openCalls( 0 )
    , policyMisses( 0 )
    , digestMismatches( 0 )
    , filesVerified( 0 )
    , fetchJobsAccepted( 0 )
    , fetchJobsMerged( 0 )
    , fetchJobsRefused( 0 )
    , fetchJobsFailed( 0 )
    , fetchJobsCancelled( 0 )
    , eventsRaised( 0 )
    , offlineReads( 0 )
    , offlineMisses( 0 )
    , staleServes( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CVfsRemoteCounters::Fill( CVfsRemoteStatistics& statistics ) const
{GUCEF_TRACE;

    statistics.bytesFetched = bytesFetched.Load();
    statistics.fetchCalls = fetchCalls.Load();
    statistics.readAheadBlocks = readAheadBlocks.Load();
    statistics.listCalls = listCalls.Load();
    statistics.statCalls = statCalls.Load();
    statistics.openCalls = openCalls.Load();
    statistics.policyMisses = policyMisses.Load();
    statistics.digestMismatches = digestMismatches.Load();
    statistics.filesVerified = filesVerified.Load();
    statistics.fetchJobsAccepted = fetchJobsAccepted.Load();
    statistics.fetchJobsMerged = fetchJobsMerged.Load();
    statistics.fetchJobsRefused = fetchJobsRefused.Load();
    statistics.fetchJobsFailed = fetchJobsFailed.Load();
    statistics.fetchJobsCancelled = fetchJobsCancelled.Load();
    statistics.eventsRaised = eventsRaised.Load();
    statistics.offlineReads = offlineReads.Load();
    statistics.offlineMisses = offlineMisses.Load();
    statistics.staleServes = staleServes.Load();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace VFS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
