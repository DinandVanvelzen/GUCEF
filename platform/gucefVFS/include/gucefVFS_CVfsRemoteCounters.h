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

#ifndef GUCEF_VFS_CVFSREMOTECOUNTERS_H
#define GUCEF_VFS_CVFSREMOTECOUNTERS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CTATOMIC_H
#include "gucefMT_CTAtomic.h"
#define GUCEF_MT_CTATOMIC_H
#endif /* GUCEF_MT_CTATOMIC_H ? */

#ifndef GUCEF_VFS_CVFSREMOTESTATISTICS_H
#include "gucefVFS_CVfsRemoteStatistics.h"
#define GUCEF_VFS_CVFSREMOTESTATISTICS_H
#endif /* GUCEF_VFS_CVFSREMOTESTATISTICS_H ? */

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

/** The counters of a remote archive, which any thread may raise; Fill() copies them into the CVfsRemoteStatistics that applications read */
class GUCEF_VFS_PUBLIC_CPP CVfsRemoteCounters
{
    public:

    MT::CTAtomic< UInt64 > bytesFetched;
    MT::CTAtomic< UInt64 > fetchCalls;
    MT::CTAtomic< UInt64 > readAheadBlocks;
    MT::CTAtomic< UInt64 > listCalls;
    MT::CTAtomic< UInt64 > statCalls;
    MT::CTAtomic< UInt64 > openCalls;
    MT::CTAtomic< UInt64 > policyMisses;
    MT::CTAtomic< UInt64 > digestMismatches;
    MT::CTAtomic< UInt64 > filesVerified;
    MT::CTAtomic< UInt64 > fetchJobsAccepted;
    MT::CTAtomic< UInt64 > fetchJobsMerged;
    MT::CTAtomic< UInt64 > fetchJobsRefused;
    MT::CTAtomic< UInt64 > fetchJobsFailed;
    MT::CTAtomic< UInt64 > fetchJobsCancelled;
    MT::CTAtomic< UInt64 > eventsRaised;
    MT::CTAtomic< UInt64 > offlineReads;
    MT::CTAtomic< UInt64 > offlineMisses;
    MT::CTAtomic< UInt64 > staleServes;

    CVfsRemoteCounters( void );

    /** Sets the fields of the statistics that these counters own; the others are left as they are */
    void Fill( CVfsRemoteStatistics& statistics ) const;

    private:

    CVfsRemoteCounters( const CVfsRemoteCounters& src );                    /**< not implemented */
    CVfsRemoteCounters& operator=( const CVfsRemoteCounters& src );         /**< not implemented */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace VFS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_VFS_CVFSREMOTECOUNTERS_H ? */
