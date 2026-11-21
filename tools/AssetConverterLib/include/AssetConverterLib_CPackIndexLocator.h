/*
 *  AssetConverterLib: where the pack index of a pack lives and how its files are named
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

#ifndef ASSETCONVERTERLIB_CPACKINDEXLOCATOR_H
#define ASSETCONVERTERLIB_CPACKINDEXLOCATOR_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef ASSETCONVERTERLIB_MACROS_H
#include "AssetConverterLib_macros.h"
#define ASSETCONVERTERLIB_MACROS_H
#endif /* ASSETCONVERTERLIB_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETCONVERTER {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The places of the pack index (section 2.2 of the design). A pack with an intermediate keeps pack-index.json in the
 *  intermediate folder (in a primary store that folder is <location>/intermediate/<pack folder>); a standalone pack folder with
 *  neither a store nor an intermediate gets its index in a user cache folder keyed by the pack key. The files of one index:
 *
 *      <folder>/pack-index.json                       the manifest
 *      <folder>/pack-index.d/shard-NNNN-<hash>.json   the resource records of a large pack
 *      <folder>/pack-index.d/.lock                    the lock of the writers (CIntermediateLock on pack-index.d)
 */
class ASSETCONVERTERLIB_PUBLIC_CPP CPackIndexLocator
{
    public:

    static const CORE::CString ManifestFileName;     /**< "pack-index.json" */
    static const CORE::CString ShardFolderName;      /**< "pack-index.d" */
    static const CORE::CString UserCacheMountName;   /**< the VFS archive name of the user cache mount */
    static const CORE::CString UserCacheEnvVar;      /**< GUCEF_PACKINDEX_CACHE: overrides the folder of the user cache */

    /** The VFS path of the manifest of the index in the folder */
    static CORE::CString GetManifestPath( const CORE::CString& indexFolder );

    /** The VFS path of the folder of the shards, which is also the folder the writers lock */
    static CORE::CString GetShardFolder( const CORE::CString& indexFolder );

    /** The VFS path of a shard file */
    static CORE::CString GetShardPath( const CORE::CString& indexFolder, const CORE::CString& shardFile );

    /** The folder on the disk of the user cache: $GUCEF_PACKINDEX_CACHE, else <home>/.cache/gucef/PackIndex (<local app data>\gucef\PackIndex on Windows) */
    static CORE::CString GetUserCacheDiskFolder( void );

    /**
     *  Mounts the user cache folder, writeable, at the VFS root path that is returned in 'vfsRoot' (once per process; the
     *  next call returns the same root). The folder is made when it is not there. False when it cannot be mounted.
     */
    static bool MountUserCache( CORE::CString& vfsRoot );

    /** The index folder of a pack in the user cache (a VFS path below the mounted cache): the first 24 characters of the pack key name it */
    static CORE::CString GetUserCacheFolder( const CORE::CString& vfsCacheRoot, const CORE::CString& packKey );

    private:

    CPackIndexLocator( void );
    ~CPackIndexLocator();
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETCONVERTERLIB_CPACKINDEXLOCATOR_H ? */

