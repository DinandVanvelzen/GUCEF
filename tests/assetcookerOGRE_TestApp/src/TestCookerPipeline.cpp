/*
 *  assetcookerOGRE_TestApp: Test application for the Ogre 1.x cooker of the asset converter (assetcookerOGRE)
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

#include "Ogre.h"

#ifndef GUCEF_TESTFW_H
#include "gucef_test_framework.h"
#define GUCEF_TESTFW_H
#endif /* GUCEF_TESTFW_H ? */

#include "TestCookerSupport.h"
#include "TestCookerScenes.h"
#include "TestCookerPipeline.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;
using namespace GUCEF::ASSETCONVERTER;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

namespace {

/** The cooker "ogre" of the plugin with a version that the test sets, counting its Cook() calls */
class CVersionedCooker : public CIAssetCooker
{
    public:

    CVersionedCooker( const TAssetCookerPtr& cooker )
        : CIAssetCooker()
        , m_cooker( cooker )
        , m_version( cooker->GetVersion() )
        , m_calls( 0 )
    {
    }

    virtual CORE::CString GetTargetName( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_cooker->GetTargetName(); }
    virtual UInt32 GetVersion( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_version; }
    virtual UInt32 GetMaxBonesPerSubMesh( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_cooker->GetMaxBonesPerSubMesh(); }

    virtual bool Cook( const CORE::CString& intermediateDir, const CORE::CString& outDir, const CCookSettings& settings, CConversionReport& report ) GUCEF_VIRTUAL_OVERRIDE
    {
        ++m_calls;
        return m_cooker->Cook( intermediateDir, outDir, settings, report );
    }

    void SetVersion( UInt32 version ) { m_version = version; }
    UInt32 GetCalls( void ) const { return m_calls; }

    private:

    TAssetCookerPtr m_cooker;
    UInt32 m_version;
    UInt32 m_calls;
};

/** A command line: the texts and the char pointers a program gets */
class CCommandLine
{
    public:

    CCommandLine( void )
        : m_texts()
        , m_pointers()
    {
        Add( "AssetConverter" );
    }

    CCommandLine& Add( const CORE::CString& argument )
    {
        m_texts.push_back( argument );
        return *this;
    }

    int GetCount( void ) const
    {
        return (int) m_texts.size();
    }

    char** GetArguments( void )
    {
        m_pointers.clear();
        for ( size_t i=0; i<m_texts.size(); ++i )
            m_pointers.push_back( const_cast< char* >( m_texts[ i ].C_String() ) );
        m_pointers.push_back( GUCEF_NULL );
        return &m_pointers[ 0 ];
    }

    private:

    CORE::CString::StringVector m_texts;
    GUCEF::vector< char* > m_pointers;
};

/** Writes the character as the glTF source "work/<folder>/src/Hero.gltf" with its textures, and a licence record */
bool
WriteCharacterSource( const CORE::CString& folder )
{
    MakeWorkFolder( folder );
    const CORE::CString source = "work/" + folder + "/src";
    if ( !WriteTestPng( source + "/tex/skin.png", 4, 4, 200, 150, 120, 255 ) || !WriteTestPng( source + "/tex/normal.png", 4, 4, 128, 128, 255, 255 ) )
        return false;
    TAssetExporterPtr exporter;
    if ( !CAssetConverterGlobal::Instance()->GetPluginRegistry().TryGetExporter( "gltf", exporter ) )
        return false;
    CAssetScene scene;
    BuildCharacterScene( scene, source + "/tex" );
    scene.name = "Hero";
    CConversionReport report;
    return exporter->Export( scene, source, report ) && WriteLicence( folder + "/licences", "lic" );
}

/** The entry of a file in references.json or sizes.json, NULL when there is none */
const CORE::CDataNode*
FindFileEntry( const CORE::CDataNode& document, const CORE::CString& file )
{
    const CORE::CDataNode* files = document.FindChild( "files" );
    if ( GUCEF_NULL == files )
        return GUCEF_NULL;
    CORE::CDataNode::const_iterator i = files->ConstBegin();
    while ( i != files->ConstEnd() )
    {
        if ( file == (*i)->GetAttributeValueOrChildValueByName( "file" ).AsString() )
            return *i;
        ++i;
    }
    return GUCEF_NULL;
}

bool
Contains( const CORE::CString::StringVector& values, const CORE::CString& value )
{
    for ( size_t i=0; i<values.size(); ++i )
        if ( values[ i ] == value )
            return true;
    return false;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformCookerPipelineTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "assetcookerOGRE.Pipeline" );

    GUCEF_TESTFW_TESTCASE( "Pipeline: a pack cooked for Ogre, its reference graph and size report; a second run is a cache hit; another cooker version cooks again" )
    try
    {
        ASSERT_TRUE( WriteCharacterSource( "pipe" ) );
        ASSERT_TRUE( WriteWorkText( "pipe/heroes.assetpack.json",
            "{ \"id\": \"heroes\", \"licenceRecord\": \"lic\", \"licenceFolder\": \"licences\", \"source\": \"src\", \"importer\": \"gltf\",\n"
            "  \"intermediate\": \"inter\", \"output\": \"out\", \"cookTargets\": [ \"ogre\" ] }\n" ) );
        CAssetPackManifest manifest;
        ASSERT_TRUE( manifest.Load( "work/pipe/heroes.assetpack.json" ) );

        // the cooker of the plugin behind one whose version the test sets
        CAssetPluginRegistry& registry = CAssetConverterGlobal::Instance()->GetPluginRegistry();
        TAssetCookerPtr real = GetOgreCooker();
        ASSERT_TRUE( !real.IsNULL() );
        CVersionedCooker* versioned = GUCEF_NEW CVersionedCooker( real );
        TAssetCookerPtr wrapper( versioned );
        registry.UnregisterCooker( "ogre" );
        ASSERT_TRUE( registry.RegisterCooker( wrapper ) );

        CAssetPipeline pipeline;
        CAssetPipelineOptions options;
        CConversionReport report;
        ASSERT_TRUE( pipeline.RunPack( manifest, options, report ) );
        ASSERT_TRUE( 1 == versioned->GetCalls() );
        ASSERT_TRUE( 1 == report.GetAssetCount( ASSETSTATUS_CONVERTED ) + report.GetAssetCount( ASSETSTATUS_APPROXIMATED ) );
        const CORE::CString profile = "work/pipe/out/desktop";
        ASSERT_TRUE( VfsFileExists( profile + "/heroes/Hero/BodyMesh.mesh" ) && VfsFileExists( profile + "/heroes/Hero/Rig.skeleton" ) );
        ASSERT_TRUE( VfsFileExists( profile + "/heroes/Hero/Hero.material" ) && VfsFileExists( profile + "/heroes/Hero/Hero.scene" ) );

        // references.json: the mesh references its material script and skeleton, the script its texture
        CORE::CDataNode references;
        ASSERT_TRUE( CCanonicalJson::Load( profile + "/references.json", references ) );
        const CORE::CDataNode* mesh = FindFileEntry( references, "heroes/Hero/BodyMesh.mesh" );
        ASSERT_TRUE( GUCEF_NULL != mesh );
        if ( GUCEF_NULL != mesh )
        {
            ASSERT_TRUE( "Hero.gltf" == mesh->GetAttributeValueOrChildValueByName( "asset" ).AsString() );
            CORE::CString::StringVector targets;
            mesh->GetValuesOfChildByName( "references", targets );
            ASSERT_TRUE( Contains( targets, "heroes/Hero/Hero.material" ) && Contains( targets, "heroes/Hero/Rig.skeleton" ) );
        }
        const CORE::CDataNode* script = FindFileEntry( references, "heroes/Hero/Hero.material" );
        ASSERT_TRUE( GUCEF_NULL != script );
        if ( GUCEF_NULL != script )
        {
            CORE::CString::StringVector targets;
            script->GetValuesOfChildByName( "references", targets );
            ASSERT_TRUE( 2 == targets.size() );
            for ( size_t t=0; t<targets.size(); ++t )
                ASSERT_TRUE( targets[ t ].StartsWith( "heroes/textures/" ) );
        }
        const CORE::CDataNode* scene = FindFileEntry( references, "heroes/Hero/Hero.scene" );
        ASSERT_TRUE( GUCEF_NULL != scene );

        // sizes.json: the size of every file and the size of the asset
        CORE::CDataNode sizes;
        ASSERT_TRUE( CCanonicalJson::Load( profile + "/sizes.json", sizes ) );
        const CORE::CDataNode* meshSize = FindFileEntry( sizes, "heroes/Hero/BodyMesh.mesh" );
        ASSERT_TRUE( GUCEF_NULL != meshSize );
        if ( GUCEF_NULL != meshSize )
            ASSERT_TRUE( CORE::FileSize( WorkDiskPath( "pipe/out/desktop/heroes/Hero/BodyMesh.mesh" ) ) == meshSize->GetAttributeValueOrChildValueByName( "size" ).AsUInt64() );
        const CORE::CDataNode* assets = sizes.FindChild( "assets" );
        ASSERT_TRUE( GUCEF_NULL != assets && 1 == assets->GetNrOfDirectChildNodes() );
        if ( GUCEF_NULL != assets && 1 == assets->GetNrOfDirectChildNodes() )
        {
            const CORE::CDataNode* asset = *assets->ConstBegin();
            ASSERT_TRUE( "Hero.gltf" == asset->GetAttributeValueOrChildValueByName( "asset" ).AsString() );
            ASSERT_TRUE( asset->GetAttributeValueOrChildValueByName( "exclusiveSize" ).AsUInt64() > meshSize->GetAttributeValueOrChildValueByName( "size" ).AsUInt64() );
        }

        // again: nothing changed, the cooker is not asked, the cooked files stay listed
        CConversionReport second;
        ASSERT_TRUE( pipeline.RunPack( manifest, options, second ) );
        ASSERT_TRUE( 1 == second.GetAssetCount( ASSETSTATUS_CACHED ) );
        ASSERT_TRUE( 1 == versioned->GetCalls() );
        ASSERT_TRUE( ReportHasFile( second, profile + "/heroes/Hero/BodyMesh.mesh" ) );
        CORE::CDataNode referencesAgain;
        ASSERT_TRUE( CCanonicalJson::Load( profile + "/references.json", referencesAgain ) && GUCEF_NULL != FindFileEntry( referencesAgain, "heroes/Hero/BodyMesh.mesh" ) );

        // another version of the cooker: the cache entry is no longer current
        versioned->SetVersion( real->GetVersion() + 1 );
        CConversionReport third;
        ASSERT_TRUE( pipeline.RunPack( manifest, options, third ) );
        ASSERT_TRUE( 0 == third.GetAssetCount( ASSETSTATUS_CACHED ) );
        ASSERT_TRUE( 2 == versioned->GetCalls() );
        const CImportActivityLog::SRun& run = pipeline.GetLastRun();
        ASSERT_TRUE( 1 == run.entries.size() && ACTIVITYREASON_VERSION_CHANGED == run.entries[ 0 ].reason );

        registry.UnregisterCooker( "ogre" );
        ASSERT_TRUE( registry.RegisterCooker( real ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CLI: a glTF source converts to the intermediate and cooks to Ogre files that load" )
    try
    {
        ASSERT_TRUE( WriteCharacterSource( "cli" ) );
        CCommandLine commandLine;
        commandLine.Add( "--mount" ).Add( "src=" + WorkDiskPath( "cli/src" ) ).Add( "--input" ).Add( "src/Hero.gltf" ).Add( "--importer" ).Add( "gltf" )
                   .Add( "--licence" ).Add( WorkDiskPath( "cli/licences/lic.licence.json" ) ).Add( "--cook" ).Add( "ogre" )
                   .Add( "--out" ).Add( WorkDiskPath( "cli/out" ) ).Add( "--report" ).Add( WorkDiskPath( "cli/report.json" ) );
        CConversionReport report;
        {
            CAssetConverterApp app( commandLine.GetCount(), commandLine.GetArguments() );
            ASSERT_TRUE( app.ParseCommandLine() );
            ASSERT_TRUE( CAssetConverterApp::ExitSuccess == app.Run() );
            report = app.GetReport();
            app.Unmount();
        }
        ASSERT_FALSE( report.HasFailures() );
        ASSERT_TRUE( CORE::FileExists( WorkDiskPath( "cli/out/intermediate/Hero.gltf" ) ) );
        ASSERT_TRUE( CORE::FileExists( WorkDiskPath( "cli/out/desktop/Hero/Hero/BodyMesh.mesh" ) ) );
        ASSERT_TRUE( CORE::FileExists( WorkDiskPath( "cli/out/desktop/references.json" ) ) && CORE::FileExists( WorkDiskPath( "cli/out/desktop/sizes.json" ) ) );

        const Ogre::String group = "CookerCli";
        ASSERT_TRUE( UseOgreGroup( group, "work/cli/out/desktop" ) );
        Ogre::MeshPtr mesh = Ogre::MeshManager::getSingleton().load( "Hero/Hero/BodyMesh.mesh", group );
        ASSERT_TRUE( mesh && 2 == mesh->getNumSubMeshes() && mesh->hasSkeleton() );
        if ( mesh && mesh->hasSkeleton() )
        {
            mesh->getSkeleton()->load();
            ASSERT_TRUE( 6 == mesh->getSkeleton()->getNumBones() && 2 == mesh->getSkeleton()->getNumAnimations() );
        }
        DropOgreGroup( group );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
