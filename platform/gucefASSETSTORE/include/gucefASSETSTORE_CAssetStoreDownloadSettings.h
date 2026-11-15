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

#ifndef GUCEF_ASSETSTORE_CASSETSTOREDOWNLOADSETTINGS_H
#define GUCEF_ASSETSTORE_CASSETSTOREDOWNLOADSETTINGS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

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
 *  The settings of the download manager (section 4.4), part of the .json settings of the application.
 *
 *  Serialized as a DataNode object with the members by their names. A limit of 0 means no limit where that is noted.
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CAssetStoreDownloadSettings : public CORE::CIDataNodeSerializable
{
    public:

    static const CORE::CString DefaultThreadPoolName;   /**< "AssetStoreTransfers" */

    UInt32 maxConcurrentJobs;               /**< jobs running at once over all stores, default 4, 0: no limit */
    UInt32 maxConcurrentJobsPerProvider;    /**< jobs running at once per store, default 2, 0: no limit */
    UInt32 maxAttempts;                     /**< attempts of a transfer that fails with a network error, a 5xx or a 429, default 5 */
    UInt32 retryBaseDelayInMs;              /**< the first back-off, doubled per attempt, with jitter; default 1000 */
    UInt32 retryMaxDelayInMs;               /**< the longest back-off, also the cap on Retry-After; default 60000 */
    UInt32 progressIntervalInMs;            /**< at most one progress event per job in this time, default 250 */
    UInt32 connectTimeoutInMs;              /**< default 30000, 0: no limit */
    UInt64 maxBytesPerSecond;               /**< per job, 0 (the default): no limit */
    CORE::CString queueFilePath;            /**< VFS path of the queue file (JSON); empty: the queue is not kept */
    CORE::CString threadPoolName;
    bool autoSaveQueue;                     /**< saved on every change of a job, default true */

    CAssetStoreDownloadSettings( void );
    virtual ~CAssetStoreDownloadSettings() GUCEF_VIRTUAL_OVERRIDE;

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

#endif /* GUCEF_ASSETSTORE_CASSETSTOREDOWNLOADSETTINGS_H ? */
