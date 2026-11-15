/*
 *  AssetViewerLib: the opening of a folder, an archive or a file of the disk in the viewer
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

#ifndef ASSETVIEWERLIB_CASSETOPENER_H
#define ASSETVIEWERLIB_CASSETOPENER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef ASSETVIEWERLIB_CASSETMOUNTER_H
#include "AssetViewerLib_CAssetMounter.h"
#define ASSETVIEWERLIB_CASSETMOUNTER_H
#endif /* ASSETVIEWERLIB_CASSETMOUNTER_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETVIEWER {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  What the user opens in the viewer, a place of the disk: the one way in for the open dialogs, a double click in the content
 *  browser on the files of the computer, the places the command line names and the list of recent places. It makes the place a
 *  mount of the CAssetMounter and says where in the VFS it is:
 *    - a folder is mounted (at a mount point made of its name, see CAssetMounter::MountUnique()): the folder to show is the mount;
 *    - an archive or pack (zip, 7z, rar, tar, unitypackage, ...) is mounted the same way and acts as a folder;
 *    - a file that the viewer has a view for is looked for in the folders that are mounted already, and else its folder is
 *      mounted: the file to open and the folder it is in are given.
 *  What is mounted already is used again: opening a place twice mounts it once. The class has no forms; the shell does the
 *  showing, with the result.
 */
class ASSETVIEWERLIB_PUBLIC_CPP CAssetOpener
{
    public:

    enum EOpenKind
    {
        OPENKIND_NONE = 0   ,   /**< there is nothing to open: the place is not there, or the viewer has nothing to do with it */
        OPENKIND_FOLDER     ,
        OPENKIND_ARCHIVE    ,
        OPENKIND_FILE
    };

    struct ASSETVIEWERLIB_PUBLIC_CPP SOpenResult
    {
        EOpenKind kind;
        CORE::CString diskPath;         /**< the place as the mounter spells it */
        CORE::CString mountPoint;       /**< the mount that holds it */
        CORE::CString vfsFolder;        /**< the VFS folder to show: the mount (folder, archive), or the folder of the file */
        CORE::CString vfsAsset;         /**< the VFS path of the file to open (OPENKIND_FILE) */
        CORE::CString problem;          /**< why nothing was opened */

        SOpenResult( void );
    };

    explicit CAssetOpener( CAssetMounter& mounter );

    ~CAssetOpener();

    /** What a place of the disk is for the viewer; archives are known by the formats the VFS can read (the plugins that are loaded) */
    static EOpenKind Classify( const CORE::CString& diskPath );

    /** Mounts what the place needs and tells where it is in the VFS; false with the reason in the result when it cannot be opened */
    bool Open( const CORE::CString& diskPath, SOpenResult& result );

    /*
     *  The kinds of files that the dialogs offer, as patterns of file names ("*.zip")
     */

    /** The archives and packs the viewer reads: zip, 7z, rar, tar and its compressed forms, unitypackage */
    static void GetArchivePatterns( CORE::CString::StringVector& patterns );

    /** The 3D models, scenes, skeletons and material scripts: fbx, obj, gltf, glb, stl, 3mf, dae, mesh, scene, ... */
    static void GetModelPatterns( CORE::CString::StringVector& patterns );

    /** The pictures: png, jpg, tga, dds, ktx, exr, ... */
    static void GetTexturePatterns( CORE::CString::StringVector& patterns );

    /** The models and the pictures, and the archives as well */
    static void GetSupportedPatterns( CORE::CString::StringVector& patterns );

    private:

    CAssetOpener( const CAssetOpener& src );
    CAssetOpener& operator=( const CAssetOpener& src );

    private:

    CAssetMounter& m_mounter;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETVIEWER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETVIEWERLIB_CASSETOPENER_H ? */
