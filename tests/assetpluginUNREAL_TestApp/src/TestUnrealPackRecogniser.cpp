/*
 *  assetpluginUNREAL_TestApp: the test of the kind declarations and the pack recogniser of the Unreal importer
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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_TESTFW_H
#include "gucef_test_framework.h"
#define GUCEF_TESTFW_H
#endif /* GUCEF_TESTFW_H ? */

#include "AssetConverterLib_CAssetPluginRegistry.h"
#include "AssetConverterLib_CAssetConverterGlobal.h"

#include "TestUnrealPackRecogniser.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;
using namespace GUCEF::ASSETCONVERTER;

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformUnrealPackRecogniserTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "assetpluginUNREAL.PackRecogniser" );

    GUCEF_TESTFW_TESTCASE( "Recogniser: an Unreal project is recognised by its .uproject, with the engine association, the default map and the noise folders" )
    try
    {
        TAssetImporterPtr importer;
        ASSERT_TRUE( CAssetConverterGlobal::Instance()->GetPluginRegistry().TryGetImporter( "unreal", importer ) );
        CAssetPackListing project;
        project.AddFile( "Game.uproject", "{ \"FileVersion\": 3, \"EngineAssociation\": \"5.3\", \"Category\": \"\" }" );
        project.AddFile( "Config/DefaultEngine.ini", "[/Script/EngineSettings.GameMapsSettings]\r\nEditorStartupMap=/Game/Maps/Edit.Edit\r\nGameDefaultMap=/Game/Maps/Entry.Entry\r\n" );
        project.AddFile( "Content/Maps/Entry.umap" );
        project.AddFile( "Content/Maps/Edit.umap" );
        project.AddFile( "Content/Maps/DemoLevel.umap" );
        project.AddFile( "Content/Characters/Hero.uasset" );
        project.AddFile( "Content/Characters/Hero.uexp" );
        project.AddFile( "Saved/Logs/a.log" );
        CAssetPackProfile profile;
        ASSERT_TRUE( importer->RecognisePack( project, profile ) );
        ASSERT_TRUE( "unreal-project" == profile.packType && "Unreal project" == profile.packTypeName && "Unreal Engine" == profile.engine && "5.3" == profile.engineVersion && 90 == profile.confidence );
        ASSERT_TRUE( 3 == profile.showcaseCandidates.size() && "Content/Maps/Entry.umap" == profile.showcaseCandidates[ 0 ] && "Content/Maps/DemoLevel.umap" == profile.showcaseCandidates[ 1 ] );
        ASSERT_TRUE( 2 == profile.groupingHints.size() && "Content/Characters" == profile.groupingHints[ 0 ] && "Content/Maps" == profile.groupingHints[ 1 ] );
        ASSERT_TRUE( profile.IsNoise( "Saved/Logs/a.log" ) && !profile.IsNoise( "Content/Maps/Entry.umap" ) );

        // loose packages without a project
        CAssetPackListing loose;
        loose.AddFile( "Content/Props/Crate.uasset" );
        ASSERT_TRUE( importer->RecognisePack( loose, profile ) );
        ASSERT_TRUE( "unreal-package" == profile.packType && 50 == profile.confidence && profile.engineVersion.IsNULLOrEmpty() );

        CAssetPackListing other;
        other.AddFile( "scene.gltf" );
        ASSERT_FALSE( importer->RecognisePack( other, profile ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Recogniser: a MetaHuman package (a mounted .mhpkg) is told by its manifest and file list, with the engine version of the manifest" )
    try
    {
        TAssetImporterPtr importer;
        ASSERT_TRUE( CAssetConverterGlobal::Instance()->GetPluginRegistry().TryGetImporter( "unreal", importer ) );
        CAssetPackListing package;
        package.AddFile( "Manifest.json", "{ \"name\": \"scarlett\", \"engineVersion\": \"5.8\" }" );
        package.AddFile( "FileList.json", "[]" );
        package.AddFile( "scarlett.uasset" );
        package.AddFile( "Meshes/Body.uasset" );
        CAssetPackProfile profile;
        ASSERT_TRUE( importer->RecognisePack( package, profile ) );
        ASSERT_TRUE( "metahuman-package" == profile.packType && "MetaHuman package" == profile.packTypeName && "5.8" == profile.engineVersion && 95 == profile.confidence );
        ASSERT_TRUE( 1 == profile.showcaseCandidates.size() && "scarlett.uasset" == profile.showcaseCandidates[ 0 ] );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Recogniser: the Unreal importer declares its file kinds, and the .uexp, .ubulk and .uptnl files belong to the package they sit next to" )
    try
    {
        TAssetImporterPtr importer;
        ASSERT_TRUE( CAssetConverterGlobal::Instance()->GetPluginRegistry().TryGetImporter( "unreal", importer ) );
        TAssetFileKindVector declared;
        importer->GetFileKinds( declared );
        CAssetKindRegistry kinds;
        kinds.RegisterFileKinds( declared, "importer:unreal" );
        ASSERT_TRUE( ASSETKIND_PACKAGE == kinds.GetKindOfPath( "Hero.uasset" ) && ASSETKIND_SCENE == kinds.GetKindOfPath( "Entry.UMAP" ) && ASSETKIND_PACKAGE == kinds.GetKindOfPath( "a.mhpkg" ) );
        ASSERT_TRUE( ASSETKIND_METADATA == kinds.GetKindOfPath( "Hero.uexp" ) && ASSETKIND_METADATA == kinds.GetKindOfPath( "Hero.ubulk" ) && ASSETKIND_METADATA == kinds.GetKindOfPath( "Hero.uptnl" ) );
        // the plugin registered with the registry of the converter, so its declarations are in the table of the tools
        ASSERT_TRUE( ASSETKIND_METADATA == CAssetConverterGlobal::Instance()->GetPluginRegistry().GetKinds().GetKindOfPath( "Content/Hero.uexp" ) );
        CORE::CString::StringVector owners;
        ASSERT_TRUE( kinds.GetOwnerPaths( "Content/Hero.ubulk", owners ) && 2 == owners.size() && "Content/Hero.uasset" == owners[ 0 ] );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
