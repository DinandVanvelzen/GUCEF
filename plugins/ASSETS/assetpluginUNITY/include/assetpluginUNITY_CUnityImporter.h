/*
 *  assetpluginUNITY: the importer of Unity projects and packages
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

#ifndef GUCEF_ASSETPLUGIN_UNITY_CUNITYIMPORTER_H
#define GUCEF_ASSETPLUGIN_UNITY_CUNITYIMPORTER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef ASSETCONVERTERLIB_CIASSETIMPORTER_H
#include "AssetConverterLib_CIAssetImporter.h"
#define ASSETCONVERTERLIB_CIASSETIMPORTER_H
#endif /* ASSETCONVERTERLIB_CIASSETIMPORTER_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNITY_MACROS_H
#include "assetpluginUNITY_macros.h"
#define GUCEF_ASSETPLUGIN_UNITY_MACROS_H
#endif /* GUCEF_ASSETPLUGIN_UNITY_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETPLUGIN {
namespace UNITY {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CUnityProject;
class CUnityMaterialReader;

/**
 *  The importer "unity": reads a Unity project, or the project tree of a .unitypackage mounted by
 *  vfspluginUNITYPACKAGE, into the model of the converter. The assets it imports:
 *
 *  - prefabs (.prefab) and scenes (.unity) with their hierarchies, meshes, materials, colliders, LOD groups, lights
 *    and expanded prefab instances (CUnityPrefabReader);
 *  - model files with a .meta file (.fbx, and the other formats the registry has an importer for), read by the
 *    importer of their format and imported as Unity imports them: scale, axis conversion, the materials the .meta
 *    names (or Unity's default material when it imports none), clips split per clipAnimations (CUnityModel);
 *  - animation clips (.anim) with generic curves (CUnityAnimationClipReader).
 *
 *  IndexPack() resolves every GUID of the project to a VFS path (the index "unity.guid"), from the .meta files below
 *  the project root (the folder above "Assets", so a single asset of a project also resolves its references). The
 *  pack rules "unity" { "overlayPackages": [ globs ] } name packages inside the pack (render-pipeline variants) that
 *  are mounted on top: their GUIDs replace those of the pack. Such a package is copied to "<intermediate>/.overlays/"
 *  and mounted from there (the package reader needs a source it can open twice). The mounts stay until the next index
 *  of the pack, or until the importer is destroyed.
 *
 *  Settings (the member "unity" of the effective settings): "importScenes" and "importModels" (also read from the
 *  pack rules, for CanImport()), "skipInactive" (false), "shaderPropertyMaps" (see CUnityMaterialReader).
 *  With the importer "auto" the FBX importer takes model files first; Unity packs name "unity" in their manifest.
 */
class GUCEF_ASSETPLUGIN_UNITY_EXPORT_CPP CUnityImporter : public ASSETCONVERTER::CIAssetImporter
{
    public:

    CUnityImporter( void );

    virtual ~CUnityImporter();

    virtual CORE::CString GetName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual UInt32 GetVersion( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual void GetFileKinds( ASSETCONVERTER::TAssetFileKindVector& kinds ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool RecognisePack( const ASSETCONVERTER::CAssetPackListing& listing, ASSETCONVERTER::CAssetPackProfile& profile ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool CanImport( const CORE::CString& vfsPath, ASSETCONVERTER::CAssetPackContext& context ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Import( const CORE::CString& vfsPath, ASSETCONVERTER::CAssetPackContext& context, ASSETCONVERTER::CAssetScene& out, ASSETCONVERTER::CConversionReport& report ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IndexPack( ASSETCONVERTER::CAssetPackContext& context ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void GetDefaultSettings( CORE::CDataNode& settings ) const GUCEF_VIRTUAL_OVERRIDE;

    /** The root of the Unity project of a VFS folder: the folder above its "Assets" folder, else the folder itself */
    static CORE::CString GetProjectRoot( const CORE::CString& vfsFolder );

    /** The GUID of a .meta text, empty when it has none */
    static CORE::CString ReadGuid( const CORE::CString& metaText );

    /** Indexes the .meta files below the VFS folder into the context; the number indexed */
    static UInt32 IndexFolder( ASSETCONVERTER::CAssetPackContext& context, const CORE::CString& vfsFolder );

    private:

    bool ImportModel( CUnityProject& project, CUnityMaterialReader& materials, const CORE::CString& vfsPath );

    void UnmountOverlays( const CORE::CString& packId );

    typedef GUCEF::map< CORE::CString, CORE::CString::StringVector > TMountMap;

    MT::CMutex m_lock;
    TMountMap m_overlays;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace UNITY */
}; /* namespace ASSETPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETPLUGIN_UNITY_CUNITYIMPORTER_H ? */
