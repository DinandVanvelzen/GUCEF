/*
 *  assetpluginUNREAL_TestApp: Tests of the importer unreal: registration, recognition, the not yet supported report and the package index
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

#ifndef GUCEF_CORE_GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_CORE_GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_CORE_GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_CORE_ASSETCONVERTERLIB_H
#include "AssetConverterLib.h"
#define GUCEF_CORE_ASSETCONVERTERLIB_H
#endif /* GUCEF_CORE_ASSETCONVERTERLIB_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALIMPORTER_H
#include "assetpluginUNREAL_CUnrealImporter.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALIMPORTER_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALIMPORTER_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALOBJECTPATHRESOLVER_H
#include "assetpluginUNREAL_CUnrealObjectPathResolver.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALOBJECTPATHRESOLVER_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALOBJECTPATHRESOLVER_H ? */

#ifndef GUCEF_CORE_TESTUNREALSUPPORT_H
#include "TestUnrealSupport.h"
#define GUCEF_CORE_TESTUNREALSUPPORT_H
#endif /* GUCEF_CORE_TESTUNREALSUPPORT_H ? */

#ifndef GUCEF_CORE_TESTUNREALWRITER_H
#include "TestUnrealWriter.h"
#define GUCEF_CORE_TESTUNREALWRITER_H
#endif /* GUCEF_CORE_TESTUNREALWRITER_H ? */

#ifndef GUCEF_CORE_TESTUNREALPROPERTIES_H
#include "TestUnrealProperties.h"
#define GUCEF_CORE_TESTUNREALPROPERTIES_H
#endif /* GUCEF_CORE_TESTUNREALPROPERTIES_H ? */

#include "TestUnrealImporter.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;
using namespace GUCEF::ASSETPLUGIN::UNREAL;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF::ASSETCONVERTER;

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

/*-------------------------------------------------------------------------*/

static UInt32
CountReportItems( const CConversionReport& report, TReportItemKind kind, const CORE::CString& text )
{
    UInt32 count = 0;
    const CConversionReport::TEntryVector& entries = report.GetEntries();
    for ( size_t e=0; e<entries.size(); ++e )
        for ( size_t i=0; i<entries[ e ].items.size(); ++i )
            if ( entries[ e ].items[ i ].kind == kind && entries[ e ].items[ i ].message.HasSubstr( text ) >= 0 )
                ++count;
    return count;
}

/*-------------------------------------------------------------------------*/

void
PerformUnrealImporterTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "assetpluginUNREAL.Importer" );

    GUCEF_TESTFW_TESTCASE( "Importer: the plugin registers \"unreal\", recognises packages by their tag and archives of packages by extension" )
    try
    {
        TAssetImporterPtr importer;
        ASSERT_TRUE( CAssetConverterGlobal::Instance()->GetPluginRegistry().TryGetImporter( "unreal", importer ) );
        ASSERT_TRUE( "unreal" == importer->GetName() && importer->GetVersion() >= 1 );
        ASSERT_TRUE( CUnrealImporter::GetCapabilities().HasSubstr( "not yet supported" ) >= 0 );

        GUCEF::vector< UInt8 > bytes;
        BuildRichPackage( -9, 522, 1018, bytes );
        ASSERT_TRUE( WriteWorkFile( "importer/Good.uasset", bytes ) );
        ASSERT_TRUE( WriteWorkFile( "importer/Good.umap", bytes ) );
        bytes[ 0 ] ^= 0x55;
        ASSERT_TRUE( WriteWorkFile( "importer/Bad.uasset", bytes ) );
        const UInt8 text[] = { 'h', 'e', 'l', 'l', 'o' };
        ASSERT_TRUE( WriteWorkFile( "importer/notes.txt", GUCEF::vector< UInt8 >( text, text + 5 ) ) );
        CAssetPackContext context;
        ASSERT_TRUE( importer->CanImport( WorkVfsPath( "importer/Good.uasset" ), context ) );
        ASSERT_TRUE( importer->CanImport( WorkVfsPath( "importer/Good.umap" ), context ) );
        ASSERT_FALSE( importer->CanImport( WorkVfsPath( "importer/Bad.uasset" ), context ) );
        ASSERT_FALSE( importer->CanImport( WorkVfsPath( "importer/notes.txt" ), context ) );
        ASSERT_FALSE( importer->CanImport( WorkVfsPath( "importer/missing.uasset" ), context ) );
        ASSERT_TRUE( importer->CanImport( WorkVfsPath( "importer/anything.mhpkg" ), context ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Importer: a package gives a report with its versions and the asset kinds that are not yet supported, a damaged one an error" )
    try
    {
        TAssetImporterPtr importer;
        ASSERT_TRUE( CAssetConverterGlobal::Instance()->GetPluginRegistry().TryGetImporter( "unreal", importer ) );
        CAssetPackContext context;
        {
            CAssetScene scene;
            CConversionReport report;
            ASSERT_FALSE( importer->Import( WorkVfsPath( "importer/Good.uasset" ), context, scene, report ) );
            ASSERT_TRUE( 1 == CountReportItems( report, REPORTITEM_INFO, "UE5 file version 1018" ) );
            // a static mesh without source models is a damaged or cooked mesh: an error of that asset, the report goes on
            ASSERT_TRUE( CountReportItems( report, REPORTITEM_ERROR, "The static mesh has no source models" ) >= 1 );
            ASSERT_TRUE( "unreal" == scene.provenance.importerName );
        }
        {
            CAssetScene scene;
            CConversionReport report;
            ASSERT_FALSE( importer->Import( WorkVfsPath( "importer/Bad.uasset" ), context, scene, report ) );
            ASSERT_TRUE( 1 == CountReportItems( report, REPORTITEM_ERROR, "Not an Unreal package" ) );
        }
        {
            CAssetScene scene;
            CConversionReport report;
            ASSERT_FALSE( importer->Import( WorkVfsPath( "importer/anything.mhpkg" ), context, scene, report ) );
            // the archive is mounted through the VFS: a file that is not an archive is an error, not a crash
            ASSERT_TRUE( 1 == CountReportItems( report, REPORTITEM_ERROR, "The archive cannot be mounted" ) );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Relations: the object paths the resolver resolves for an import are recorded with their kinds and the object path; engine content is kept unresolved" )
    try
    {
        CAssetScene scene;
        CUnrealObjectPathResolver resolver;
        resolver.AddPackage( "/Game/Materials/M_Rock", "work/importer/M_Rock.uasset" );
        CORE::CString vfsPath;
        // without a recorder nothing is recorded
        ASSERT_TRUE( resolver.ResolveReference( "/Game/Materials/M_Rock.M_Rock", CAssetReference::KindUsesMaterial, vfsPath ) && scene.references.empty() );
        resolver.SetRecorder( &scene );
        ASSERT_TRUE( resolver.ResolveReference( "/Game/Materials/M_Rock.M_Rock", CAssetReference::KindUsesMaterial, vfsPath ) && "work/importer/M_Rock.uasset" == vfsPath );
        ASSERT_FALSE( resolver.ResolveReference( "/Engine/EngineMaterials/DefaultMaterial.DefaultMaterial", CAssetReference::KindUsesMaterial, vfsPath ) );
        ASSERT_TRUE( 2 == scene.references.size() );
        ASSERT_TRUE( CAssetReference::KindUsesMaterial == scene.references[ 0 ].kind && "work/importer/M_Rock.uasset" == scene.references[ 0 ].target && "/Game/Materials/M_Rock.M_Rock" == scene.references[ 0 ].via );
        ASSERT_TRUE( !scene.references[ 1 ].IsResolved() && "/Engine/EngineMaterials/DefaultMaterial.DefaultMaterial" == scene.references[ 1 ].via );
        TAssetImporterPtr importer;
        ASSERT_TRUE( CAssetConverterGlobal::Instance()->GetPluginRegistry().TryGetImporter( "unreal", importer ) && 6 == importer->GetVersion() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Importer: the index of a pack maps the package names of its files to their paths" )
    try
    {
        GUCEF::vector< UInt8 > a;
        GUCEF::vector< UInt8 > b;
        BuildRichPackage( -8, 522, 1009, a );
        BuildRichPackage( -9, 522, 1018, b );
        ASSERT_TRUE( WriteWorkFile( "pack/Content/A.uasset", a ) );
        ASSERT_TRUE( WriteWorkFile( "pack/Content/Sub/B.uasset", b ) );
        ASSERT_TRUE( WriteWorkFile( "pack/Content/readme.txt", GUCEF::vector< UInt8 >( 3, 'x' ) ) );
        CAssetPackManifest manifest;
        manifest.id = "unrealtest";
        manifest.source = "/work/pack";
        manifest.importer = "unreal";
        manifest.intermediate = "/work/intermediate";
        CAssetPackContext context;
        ASSERT_TRUE( context.Open( manifest ) );
        CUnrealImporter importer;
        ASSERT_TRUE( importer.IndexPack( context ) );
        // both packages carry the package name of the test writer: the index holds it once
        ASSERT_TRUE( 1 == context.GetIndexSize( CUnrealImporter::PackageIndexName ) );
        CORE::CString path;
        ASSERT_TRUE( context.TryResolve( CUnrealImporter::PackageIndexName, "/Game/Test/Package", path ) && path.HasSubstr( "Content/" ) >= 0 );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

