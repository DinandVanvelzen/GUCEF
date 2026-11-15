/*
 *  assetpluginGLTF_TestApp: Test application for the glTF plugin of the asset converter (assetpluginGLTF)
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

#include "TestGltfSupport.h"
#include "TestGltfScenes.h"
#include "TestGltfCli.h"

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

Int32
RunCommandLine( CCommandLine& commandLine, CConversionReport* report = GUCEF_NULL )
{
    CAssetConverterApp app( commandLine.GetCount(), commandLine.GetArguments() );
    if ( !app.ParseCommandLine() )
        return -1;
    const Int32 exitCode = app.Run();
    if ( GUCEF_NULL != report )
        *report = app.GetReport();
    app.Unmount();
    return exitCode;
}

bool
LoadJsonFile( const CORE::CString& diskPath, CORE::CDataNode& node )
{
    CORE::CString text;
    return CORE::LoadTextFileAsString( diskPath, text ) && CCanonicalJson::Parse( text, node );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformGltfCliTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "assetpluginGLTF.Cli" );

    GUCEF_TESTFW_TESTCASE( "CLI: a generated glTF converts to the intermediate with its textures, sidecar and report, and then comes from the cache" )
    try
    {
        MakeWorkFolder( "cli" );

        // the source: a glTF the exporter writes, with a texture next to it
        TAssetExporterPtr exporter;
        ASSERT_TRUE( CAssetConverterGlobal::Instance()->GetPluginRegistry().TryGetExporter( "gltf", exporter ) );
        CAssetScene scene;
        BuildTriangleScene( scene, "model" );
        scene.textures[ 0 ].sources[ 0 ] = "work/cli/src/tri.png";
        CConversionReport exportReport;
        ASSERT_TRUE( exporter->Export( scene, "work/cli/src", exportReport ) );
        ASSERT_TRUE( WriteTestPng( "work/cli/src/tri.png", 8, 8, 10, 120, 240, 255 ) );
        ASSERT_TRUE( WriteLicence( "cli/licences", "rec" ) );

        const CORE::CString outFolder = WorkDiskPath( "cli/out" );
        CCommandLine commandLine;
        commandLine.Add( "--mount" ).Add( "single=" + WorkDiskPath( "cli/src" ) ).Add( "--input" ).Add( "single/model.gltf" ).Add( "--importer" ).Add( "gltf" )
                   .Add( "--licence" ).Add( WorkDiskPath( "cli/licences/rec.licence.json" ) ).Add( "--out" ).Add( outFolder ).Add( "--report" ).Add( WorkDiskPath( "cli/report.json" ) );
        CConversionReport report;
        ASSERT_TRUE( CAssetConverterApp::ExitSuccess == RunCommandLine( commandLine, &report ) );
        ASSERT_TRUE( 1 == report.GetAssetCount( ASSETSTATUS_CONVERTED ) && "rec" == report.GetLicenceRecordId() );

        const CORE::CString intermediate = CORE::CombinePath( outFolder, "intermediate" );
        const CORE::CString gltfPath = CORE::CombinePath( intermediate, "model.gltf" );
        ASSERT_TRUE( CORE::FileExists( gltfPath ) && CORE::FileExists( CORE::CombinePath( intermediate, "model.bin" ) ) && CORE::FileExists( CORE::CombinePath( intermediate, "model.conversion.json" ) ) );
        ASSERT_TRUE( CORE::FileExists( WorkDiskPath( "cli/report.json" ) ) );
        CORE::CString problem;
        ASSERT_TRUE( ValidateWithCgltf( gltfPath, problem ) );

        // the sidecar says where the asset came from and what it holds
        CORE::CDataNode sidecar;
        ASSERT_TRUE( LoadJsonFile( CORE::CombinePath( intermediate, "model.conversion.json" ), sidecar ) );
        const CORE::CDataNode* provenance = sidecar.FindChild( "provenance" );
        ASSERT_TRUE( GUCEF_NULL != provenance );
        if ( GUCEF_NULL != provenance )
        {
            ASSERT_TRUE( "gltf" == provenance->GetAttributeValueOrChildValueByName( "importer" ).AsString() || "gltf" == provenance->GetAttributeValueOrChildValueByName( "importerName" ).AsString() );
            ASSERT_TRUE( "gltf" == provenance->GetAttributeValueOrChildValueByName( "sourceFormat" ).AsString() );
        }
        const CORE::CDataNode* summary = sidecar.FindChild( "summary" );
        ASSERT_TRUE( GUCEF_NULL != summary && 3 == summary->GetAttributeValueOrChildValueByName( "vertices" ).AsInt32() && 1 == summary->GetAttributeValueOrChildValueByName( "triangles" ).AsInt32() );
        const CORE::CDataNode* extras = sidecar.FindChild( "extras" );
        ASSERT_TRUE( GUCEF_NULL != extras && GUCEF_NULL != extras->FindChild( "gltf" ) );
        const CORE::CDataNode* materials = sidecar.FindChild( "materials" );
        ASSERT_TRUE( GUCEF_NULL != materials && 1 == materials->GetNrOfDirectChildNodes() );

        // the texture was baked into the intermediate and the glTF names it
        CORE::CString gltfText;
        ASSERT_TRUE( CORE::LoadTextFileAsString( gltfPath, gltfText ) );
        ASSERT_TRUE( gltfText.HasSubstr( "textures/" ) >= 0 );

        // again: nothing changed
        ASSERT_TRUE( CAssetConverterApp::ExitSuccess == RunCommandLine( commandLine, &report ) );
        ASSERT_TRUE( 1 == report.GetAssetCount( ASSETSTATUS_CACHED ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
