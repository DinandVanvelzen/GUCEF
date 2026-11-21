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
 *  maxItemsPerDay, maxBytesPerDay, the space reserve), whoever asks. A limit of 0 is no maximum. The hour and day counters are kept
 *  in memory: a restart of the service starts them again (the caps of the stores are kept over restarts with the library sync).
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CAssetReprocessSettings : public CORE::CIDataNodeSerializable
{
    public:

    bool enabled;                               /**< the reprocessing operations are offered, default true */
    UInt32 maxConcurrentJobs;                   /**< reprocessing jobs that run at a time; the others wait their turn, default 1 */
    UInt32 maxQueuedJobs;                       /**< reprocessing jobs that wait or run at a time; one more is refused as busy, default 8 */
    UInt32 maxJobsPerPackPerHour;               /**< reprocessing jobs of one pack in an hour, for a machine caller, default 4 */
    UInt32 maxMachineJobsPerHour;               /**< reprocessing jobs of machine callers together in an hour, default 20 */
    UInt32 maxMachineRedownloadsPerDay;         /**< downloads again of originals by machine callers together in a day, default 6 */
    bool limitOwner;                            /**< the per pack and per hour limits apply to devices with the role owner too, default false */
    UInt32 downloadTimeoutMinutes;              /**< a download again that takes longer is cancelled, default 360 */

    CAssetReprocessSettings( void );
    virtual ~CAssetReprocessSettings() GUCEF_VIRTUAL_OVERRIDE;

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
