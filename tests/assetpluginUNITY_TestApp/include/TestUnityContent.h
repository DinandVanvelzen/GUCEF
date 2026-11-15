/*
 *  assetpluginUNITY_TestApp: Test application for the Unity plugin of the asset converter (assetpluginUNITY)
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

#ifndef GUCEF_ASSETPLUGIN_UNITY_TESTAPP_TESTUNITYCONTENT_H
#define GUCEF_ASSETPLUGIN_UNITY_TESTAPP_TESTUNITYCONTENT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include "TestUnitySupport.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  A synthetic Unity project written by the tests: assets with their GUIDs and .meta texts, written either as a
 *  .unitypackage (a gzip tar of GUID folders with asset, asset.meta and pathname, through the archive writer of the
 *  VFS) or as a plain project folder.
 */
class CTestUnityProject
{
    public:

    struct SAsset
    {
        GUCEF::CORE::CString pathname;
        GUCEF::CORE::CString guid;
        GUCEF::CORE::CDynamicBuffer content;
        GUCEF::CORE::CString meta;
        bool folder;
    };
    typedef GUCEF::vector< SAsset > TAssetVector;

    void AddText( const GUCEF::CORE::CString& pathname, const GUCEF::CORE::CString& guid, const GUCEF::CORE::CString& text, const GUCEF::CORE::CString& meta );

    void AddBytes( const GUCEF::CORE::CString& pathname, const GUCEF::CORE::CString& guid, const GUCEF::CORE::CDynamicBuffer& bytes, const GUCEF::CORE::CString& meta );

    void AddFolder( const GUCEF::CORE::CString& pathname, const GUCEF::CORE::CString& guid );

    /** Writes the package to a file of the disk */
    bool WritePackage( const GUCEF::CORE::CString& diskPath ) const;

    /** Writes the project tree below a folder of the disk */
    bool WriteFolder( const GUCEF::CORE::CString& diskFolder ) const;

    const TAssetVector& GetAssets( void ) const;

    private:

    TAssetVector m_assets;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The GUIDs of the test project */
extern const char* const GuidHousePrefab;
extern const char* const GuidStreetPrefab;
extern const char* const GuidUrpMaterial;
extern const char* const GuidHouseModel;

/** A .meta text: the GUID and the importer settings (indented by two spaces below the importer) */
GUCEF::CORE::CString MakeMeta( const GUCEF::CORE::CString& guid, const GUCEF::CORE::CString& importerBlock );

/**
 *  The test project: textures, a house model (centimetres, globalScale 100) with duplicate node names, the same model
 *  at globalScale 1 and with the later ID table, a model with a take and two clips, materials of the Standard, URP Lit,
 *  custom and shader graph shaders and one whose shader is missing, a generated convex mesh, an older prefab, a newer
 *  prefab with a prefab instance, a scene and an animation clip
 */
bool BuildTestProject( CTestUnityProject& project );

/** Writes the test project as a package and as a folder below the work folder: "<name>/test.unitypackage" and "<name>/project" */
bool WriteTestProject( const GUCEF::CORE::CString& name );

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETPLUGIN_UNITY_TESTAPP_TESTUNITYCONTENT_H ? */
