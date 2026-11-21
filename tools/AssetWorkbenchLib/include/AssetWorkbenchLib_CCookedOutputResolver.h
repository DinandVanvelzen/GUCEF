/*
 *  AssetWorkbenchLib: finds the cooked Ogre output of an asset and says whether it is current
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

#ifndef ASSETWORKBENCHLIB_CCOOKEDOUTPUTRESOLVER_H
#define ASSETWORKBENCHLIB_CCOOKEDOUTPUTRESOLVER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef ASSETCONVERTERLIB_CASSETCACHEENTRY_H
#include "AssetConverterLib_CAssetCacheEntry.h"
#define ASSETCONVERTERLIB_CASSETCACHEENTRY_H
#endif /* ASSETCONVERTERLIB_CASSETCACHEENTRY_H ? */

#ifndef ASSETWORKBENCHLIB_MACROS_H
#include "AssetWorkbenchLib_macros.h"
#define ASSETWORKBENCHLIB_MACROS_H
#endif /* ASSETWORKBENCHLIB_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETWORKBENCH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Finds the cooked Ogre output (the "gucef-ogre3d" runtime files: .mesh, .skeleton, .material and .scene) of an asset, and says
 *  whether it is current. The cooked files are the converted result for Ogre 1.x; the Assimp codec of Ogre is only the quick look
 *  that the workbench falls back on when nothing was cooked, or the cooked output is stale.
 *
 *  Where the output is looked for:
 *    - a pack of the primary store (layout version 2 of ASSETSTORE::CPackStorageLayout): the path of the original of an asset,
 *      "<location>/original/<pack folder>/<asset>", names the intermediate area "<location>/intermediate/<pack folder>" (with the cache
 *      entries of the converter) and the cooked area "<location>/gucef-ogre3d/<pack folder>" (one folder per cook profile). The paths
 *      are those the service gives, so a remote service and a local one are looked at the same way;
 *    - a conversion of the workbench in its cache folder (CPackAssetConverter): the folders of that conversion are given.
 *
 *  What decides: the cache entry of the converter of the asset ("<intermediate>/.cache/<asset>.cache.json", CAssetCacheEntry) lists
 *  the files the conversion wrote, cooked ones with their profile. The output is absent when there is no entry or it lists no cooked
 *  file of the profile; stale when a cooked file is gone, the cook did not finish, or the key of the entry no longer stands for the
 *  source (the hash of the source, the version of a plugin of the stack that is registered, the hashes of its dependencies;
 *  CAssetPipeline::CheckStoredEntry()); else current. Hashing a source reads all of it: Locate() is cheap and may be called on the
 *  thread of the interface, Verify() reads the source and belongs on a thread of a pool. The hash of the settings is part of the key
 *  too, but it needs the manifest of the pack: the converter's own plan (and Convert for Ogre) notices a change of settings.
 */
class ASSETWORKBENCHLIB_PUBLIC_CPP CCookedOutputResolver
{
    public:

    enum EState
    {
        STATE_ABSENT = 0    ,   /**< nothing was cooked for the asset */
        STATE_STALE         ,   /**< cooked once, but it no longer stands for the source (the reason says why) */
        STATE_CURRENT           /**< the cooked files are there and the key of the entry stands */
    };

    struct SOptions
    {
        CORE::CString profile;      /**< the cook profile whose files are wanted: "desktop" */
        SOptions( void );
    };

    /** What was found for an asset */
    struct ASSETWORKBENCHLIB_PUBLIC_CPP SCookedOutput
    {
        EState state;
        bool verified;                          /**< Verify() has run (a located output that was not verified is current only provisionally) */
        bool sourceVerified;                    /**< the source could be read and was compared; false for a source that is gone (a pack that kept only its converted files) */
        CORE::CString origin;                   /**< "store" or "cache" */
        CORE::CString assetPath;                /**< the path of the asset in its pack, as the cache entry knows it */
        CORE::CString sourceVfsPath;            /**< the VFS path of the source the entry was made from; empty when not known */
        CORE::CString intermediateFolder;       /**< the VFS folder of the intermediate files of the pack */
        CORE::CString cookedFolder;             /**< the VFS folder of the cooked area of the pack (the folders of the profiles are below it) */
        CORE::CString profile;
        CORE::CString cacheKey;                 /**< the key of the entry */
        CORE::CString mainFile;                 /**< the VFS path of the cooked file to show: the scene of the asset, else the mesh that no other cooked file refers to */
        CORE::CString sidecarFile;              /**< the VFS path of the conversion sidecar (the hints of a character are in it), empty when there is none */
        CORE::CString::StringVector files;      /**< every cooked file of the profile, as VFS paths */
        CORE::CString reason;                   /**< why absent or stale, in words */
        ASSETCONVERTER::CAssetCacheEntry entry;
        SCookedOutput( void );
        bool IsCurrent( void ) const;
        bool IsLocated( void ) const;           /**< there is a cache entry with cooked files (current or stale), so Verify() has something to check */
    };

    /**
     *  Looks in the layout of a store. 'vfsPath' is a path of the original area of a pack ("<location>/original/<pack folder>[/<asset>]");
     *  'assetPath' is the asset in the pack, empty to take it from the path. No hashing: see Verify(). A path that is not in the layout
     *  gives an absent output with the reason
     */
    static void LocateInStore( const CORE::CString& vfsPath, const CORE::CString& assetPath, const SOptions& options, SCookedOutput& output );

    /** Looks in the folders of one conversion (the cache folder of the workbench): the folder of its intermediate files, its cooked area and the asset in it */
    static void LocateInFolders( const CORE::CString& intermediateFolder, const CORE::CString& cookedFolder, const CORE::CString& assetPath, const CORE::CString& sourceVfsPath, const SOptions& options, SCookedOutput& output );

    /** Compares the key of the located entry with the source and the plugins as they are now; reads the whole source. Sets 'state', 'reason' and 'verified' */
    static void Verify( SCookedOutput& output );

    /** Locate and verify in one call, for the layout of a store */
    static void ResolveInStore( const CORE::CString& vfsPath, const CORE::CString& assetPath, const SOptions& options, SCookedOutput& output );

    /**
     *  Whether a VFS path is in the original area of a pack of a store ("<location>/original/<pack folder>[/<asset>]", with the pack
     *  in the intermediate or cooked area of the same location): the VFS folder of the location, the folder of the pack and the path
     *  of the asset below it ("" for the pack itself)
     */
    static bool SplitStorePath( const CORE::CString& vfsPath, CORE::CString& locationRoot, CORE::CString& packFolder, CORE::CString& relative );

    /** The label for the inspector and the report: "cooked Ogre v1 output (gucef-ogre3d)" for a current output */
    static CORE::CString GetCookedLabel( void );

    /** What the state is in words, with the reason: "cooked output: current", "cooked output: stale (the source changed)", "cooked output: none (not converted yet)" */
    static CORE::CString Describe( const SCookedOutput& output );

    /** Whether a file name is a runtime file of Ogre that the views read as it is ("mesh", "scene") */
    static bool IsCookedMainExtension( const CORE::CString& extension );

    private:

    CCookedOutputResolver( void );

    static CORE::CString Reroot( const CORE::CString& recorded, const CORE::CString& cookedFolder, const CORE::CString& profile );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETWORKBENCH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETWORKBENCHLIB_CCOOKEDOUTPUTRESOLVER_H ? */
