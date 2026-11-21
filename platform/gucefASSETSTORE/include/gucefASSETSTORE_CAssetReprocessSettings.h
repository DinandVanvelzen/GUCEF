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

#ifndef GUCEF_ASSETSTORE_CASSETREPROCESSSETTINGS_H
#define GUCEF_ASSETSTORE_CASSETREPROCESSSETTINGS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */
#ifndef GUCEF_CORE_CIDATANODESERIALIZABLE_H
#include "gucefCORE_CIDataNodeSerializable.h"
#define GUCEF_CORE_CIDATANODESERIALIZABLE_H
#endif /* GUCEF_CORE_CIDATANODESERIALIZABLE_H ? */
#ifndef GUCEF_ASSETSTORE_MACROS_H
#include "gucefASSETSTORE_macros.h"
#define GUCEF_ASSETSTORE_MACROS_H
#endif /* GUCEF_ASSETSTORE_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The limits of the reprocessing of packs (CAssetReprocessRequest), the child "reprocess" of the configuration of the service
 *  (CAssetStoreServiceConfig). They protect the service and the stores from a machine caller (an LLM through MCP) that repeats
 *  itself: per pack and per hour, per machine and per hour, the downloads again per day of a machine, and the jobs at a time for
 *  everyone. A download again from a store also counts against the caps of the sync profile of that store (maxItemsPerHour,
 *  maxItemsPerDay, maxBytesPerDay, the space reserve), whoever asks. A limit of 0 is no maximum unless the member says otherwise.
 *  The hour and day counters are kept in memory: a restart of the service starts them again (the caps of the stores are kept over
 *  restarts with the library sync).
 *
 *  The packs are independent of each other, so the jobs of different packs run in parallel: as many as maxConcurrentJobs, each
 *  with a share of the threads of the machine (threadsPerJob) for its converter run, and only while the memory that the running
 *  jobs are estimated to need fits memoryBudgetMB (CAssetReprocessBudget computes the automatic values from the machine).
 *
 *  The settings can be changed while the service runs (CLocalAssetStoreService::ApplyReprocessSettings()): ApplyChanges() takes a
 *  set of changed keys, checks every one and changes nothing when one of them is not valid.
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CAssetReprocessSettings : public CORE::CIDataNodeSerializable
{
    public:

    bool enabled;                               /**< the reprocessing operations are offered, default true */
    UInt32 maxConcurrentJobs;                   /**< reprocessing jobs that run at a time; the others wait their turn; default 0: automatic, a quarter of the logical cores, at least 1 and at most 8 */
    UInt32 threadsPerJob;                       /**< converter threads of one running job; default 0: automatic, the cores that are not reserved divided by the jobs at a time, at least 1 */
    UInt32 reservedCores;                       /**< logical cores the automatic threadsPerJob leaves to the service itself (sessions, ingests, the database), default 2 */
    UInt32 maxQueuedJobs;                       /**< reprocessing jobs that wait or run at a time; one more is refused as busy, default 64 */
    UInt32 maxJobsPerPackPerHour;               /**< reprocessing jobs of one pack in an hour, for a machine caller, default 4 */
    UInt32 maxMachineJobsPerHour;               /**< reprocessing jobs of machine callers together in an hour, default 20 */
    UInt32 maxMachineRedownloadsPerDay;         /**< downloads again of originals by machine callers together in a day, default 6 */
    bool limitOwner;                            /**< the per pack and per hour limits apply to devices with the role owner too, default false */
    UInt32 downloadTimeoutMinutes;              /**< a download again that takes longer is cancelled, default 360 */
    UInt32 memoryBudgetMB;                      /**< the memory the running jobs may be estimated to need together; default 0: automatic, 70 percent of the memory limit of the container (cgroup) or of the machine */
    UInt32 memoryPerJobMB;                      /**< the least memory a job with a conversion step is estimated to need, default 2048 */
    UInt32 memoryLargestAssetFactor;            /**< a job with a conversion step is estimated at this many times the largest asset of its pack when that is more than memoryPerJobMB, default 4; 0: memoryPerJobMB alone */
    UInt32 maxConcurrentWritesPerLocation;      /**< running jobs that write to the staging of the same storage location at a time, default 0: no limit (a network share may want 1 or 2) */

    CAssetReprocessSettings( void );
    virtual ~CAssetReprocessSettings() GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  Takes the keys of 'changes' (attributes or child values named as the members, the others keep their value) when every one
     *  of them is known and valid; otherwise nothing changes and 'errors' holds a reason per key that is not. True when applied.
     */
    bool ApplyChanges( const CORE::CDataNode& changes, CORE::CString::StringMap& errors );

    virtual bool Serialize( CORE::CDataNode& domRootNode                        ,
                            const CORE::CDataNodeSerializableSettings& settings ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Deserialize( const CORE::CDataNode& domRootNode                  ,
                              const CORE::CDataNodeSerializableSettings& settings ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETSTORE_CASSETREPROCESSSETTINGS_H ? */
