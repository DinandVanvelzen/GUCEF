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

#ifndef GUCEF_ASSETSTORE_CREMOTEFILESSETTINGS_H
#define GUCEF_ASSETSTORE_CREMOTEFILESSETTINGS_H

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
 *  The settings of CRemoteAssetFileSystem, the client side of the files of a remote asset store service (remote service design 5.4):
 *  the block cache on the disk, the read ahead and the bounds of the lists. A limit of 0 means no limit where that is noted.
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CRemoteFilesSettings : public CORE::CIDataNodeSerializable
{
    public:

    CORE::CString cacheFolder;                  /**< a folder of the disk for the block cache; empty: no cache on the disk (reads go to the service and are not kept) */
    UInt64 maxCacheBytes;                       /**< the limit of the block cache, default 20 GiB; the entries that were used longest ago go first, pinned ones stay */
    UInt32 readAheadAfterBlocks;                /**< sequential blocks that were read before the next ones are fetched ahead, default 2; 0: no read ahead */
    UInt32 readAheadBlocks;                     /**< blocks fetched ahead, default 8 (2 MiB) */
    UInt32 maxBlocksPerRequest;                 /**< blocks asked of the service in one call, default 16 (4 MiB) */
    UInt32 maxParallelFiles;                    /**< files fetched at once by a transfer, default 4 */
    UInt32 maxListEntries;                      /**< entries of a recursive list of a folder of the service, default 100000; the log says when a list was cut */
    UInt32 listPageEntries;                     /**< entries asked per page of a list of a folder, default 1000 */
    UInt32 metadataEntries;                     /**< files and folders whose size, digest and kind are remembered, default 200000 */
    UInt32 negativeTtlMs;                       /**< a path that the service does not have is not asked for again within this time, default 3000 */
    UInt32 blockingReadTimeoutMs;               /**< the most a read waits for the service, default 30000 */
    UInt32 transferQueueThresholdBytes;         /**< a set of files above this size is shown in the transfer queue as a fetch, default 16 MiB */
    UInt32 openWithoutPrefetchBytes;            /**< a set larger than the maximum is opened with what is there after this much, default 8 MiB */
    UInt64 maxPrefetchBytes;                    /**< the largest set that is fetched before an asset is opened, default 2 GiB */

    CRemoteFilesSettings( void );
    virtual ~CRemoteFilesSettings() GUCEF_VIRTUAL_OVERRIDE;

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

#endif /* GUCEF_ASSETSTORE_CREMOTEFILESSETTINGS_H ? */
