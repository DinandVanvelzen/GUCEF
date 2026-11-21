/*
 *  AssetWorkbenchLib: turns one asset of a pack into files the views read: an importer of an engine, the glTF exporter and, for effects, the Ogre cooker
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

#ifndef ASSETWORKBENCHLIB_CPACKASSETCONVERTER_H
#define ASSETWORKBENCHLIB_CPACKASSETCONVERTER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef ASSETWORKBENCHLIB_CASSETLOADCONTROL_H
#include "AssetWorkbenchLib_CAssetLoadControl.h"
#define ASSETWORKBENCHLIB_CASSETLOADCONTROL_H
#endif /* ASSETWORKBENCHLIB_CASSETLOADCONTROL_H ? */

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
 *  The bridge from a pack of an engine to the views of the workbench, for a presentation: it imports one asset of the pack (the root
 *  of a character package, the scene of a Unity or Godot pack, an effect prefab) with the importer of the engine through the
 *  conversion of a single asset of the pipeline (import, normalisation, post-processors, baking of the textures, glTF export, sidecar),
 *  and for effects cooks the result for Ogre (particle scripts, materials and a scene). Nothing is written but into a folder of the
 *  cache of the workbench, which is mounted writable for the length of the work; no licence record is made or read, as nothing leaves the
 *  workbench. Prepare() runs on a thread of a pool and takes the VFS as it is, so the mounts are made before and unmade after, on the
 *  calling thread of the interface (Mount() and Unmount()).
 */
class ASSETWORKBENCHLIB_PUBLIC_CPP CPackAssetConverter
{
    public:

    struct ASSETWORKBENCHLIB_PUBLIC_CPP SRequest
    {
        CORE::CString converter;        /**< the name of the importer: "unity", "godot", "unreal" */
        CORE::CString sourceFolder;     /**< the VFS folder of the source of the pack (the mount of the pack) */
        CORE::CString sourceDisk;       /**< a file of the disk that is the asset itself (a package): its folder is mounted and it is the only file included; empty for an asset in the mount */
        CORE::CString assetPath;        /**< the path of the asset in the source, or the name of the file of 'sourceDisk' */
        CORE::CString cacheFolder;      /**< a folder of the disk where the result goes; it is made */
        CORE::CString id;               /**< a short name for the mounts and the manifest of this conversion */
        bool cook;                      /**< also cook for Ogre: the cooked runtime files go to <cache folder>/out/desktop and are listed in the cache entry of the asset, so that a next request finds them (CCookedOutputResolver) */
        bool force;                     /**< convert and cook again, also what the cache entry says is current */
        bool mergeStatic;               /**< merge the static geometry of the scene by material for the preview (CPackPreviewPostProcessor, merging form): the scene of a pack with thousands of meshes */
        bool staticPreview;             /**< drop the rig and the levels of detail (CPackPreviewPostProcessor): the portrait of a character */
        SRequest( void );
    };

    struct ASSETWORKBENCHLIB_PUBLIC_CPP SResult
    {
        bool ok;
        CORE::CString problem;
        CORE::CString mainFile;         /**< the VFS path of the glTF, or of the cooked scene, in the writable mount */
        CORE::CString sidecarFile;      /**< the VFS path of its conversion sidecar */
        CORE::CString::StringVector files;      /**< every file the conversion wrote, as VFS paths of the writable mount */
        CORE::CDataNode sidecar;        /**< the sidecar as it was read: the extras of the scene (hints of a character) are in it */
        UInt32 warningCount;
        UInt32 errorCount;
        CORE::CString summary;          /**< what the report says, for the log */
        SResult( void );
    };

    /** The names of the two mounts of a request: the writable one of the cache folder and the read only one of the folder of the file of 'sourceDisk' */
    static CORE::CString GetCacheMountPoint( const SRequest& request );
    static CORE::CString GetSourceMountPoint( const SRequest& request );

    /** Mounts the cache folder (writable) and, with a source file of the disk, its folder; on the calling thread of the interface. False with the reason in the log */
    static bool Mount( const SRequest& request );
    static void Unmount( const SRequest& request );

    /** Converts; on any thread, between Mount() and Unmount(). Stops early when the control says it is cancelled */
    static bool Convert( const SRequest& request, SResult& result, const CAssetLoadControl* control );

    private:

    CPackAssetConverter( void );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETWORKBENCH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETWORKBENCHLIB_CPACKASSETCONVERTER_H ? */
