/*
 *  assetpluginFBX_TestApp: Test application for the FBX plugin of the asset converter (assetpluginFBX)
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

#include "TestFbxSupport.h"
#include "TestFbxWriter.h"
#include "TestFbxCli.h"

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
PerformFbxCliTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "assetpluginFBX.Cli" );

    GUCEF_TESTFW_TESTCASE( "CLI: a generated FBX in centimetres with Z up converts to the intermediate in metres with Y up, with textures, sidecar and report" )
    try
    {
        MakeWorkFolder( "cli" );
        const double translation[ 3 ] = { 100.0, 200.0, 50.0 };
        ASSERT_TRUE( WriteWorkText( "cli/src/cube.fbx", MakeCubeFbx( FbxZUpCentimetres(), 100.0, translation, "albedo.png", CORE::CString::Empty ) ) );
        ASSERT_TRUE( WriteTestPng( "work/cli/src/albedo.png", 8, 8, 200, 30, 30, 255 ) );
        ASSERT_TRUE( WriteLicence( "cli/licences", "rec" ) );

        const CORE::CString outFolder = WorkDiskPath( "cli/out" );
        CCommandLine commandLine;
        commandLine.Add( "--mount" ).Add( "single=" + WorkDiskPath( "cli/src" ) ).Add( "--input" ).Add( "single/cube.fbx" ).Add( "--importer" ).Add( "fbx" )
                   .Add( "--licence" ).Add( WorkDiskPath( "cli/licences/rec.licence.json" ) ).Add( "--out" ).Add( outFolder ).Add( "--report" ).Add( WorkDiskPath( "cli/report.json" ) );
        CConversionReport report;
        ASSERT_TRUE( CAssetConverterApp::ExitSuccess == RunCommandLine( commandLine, &report ) );
        ASSERT_TRUE( 0 == report.GetAssetCount( ASSETSTATUS_FAILED ) && ( 1 == report.GetAssetCount( ASSETSTATUS_CONVERTED ) || 1 == report.GetAssetCount( ASSETSTATUS_APPROXIMATED ) ) );
        ASSERT_TRUE( CORE::FileExists( WorkDiskPath( "cli/report.json" ) ) );

        const CORE::CString intermediate = CORE::CombinePath( outFolder, "intermediate" );
        const CORE::CString gltfPath = CORE::CombinePath( intermediate, "cube.gltf" );
        ASSERT_TRUE( CORE::FileExists( gltfPath ) && CORE::FileExists( CORE::CombinePath( intermediate, "cube.bin" ) ) && CORE::FileExists( CORE::CombinePath( intermediate, "cube.conversion.json" ) ) );
        CORE::CString problem;
        ASSERT_TRUE( ValidateWithCgltf( gltfPath, problem ) );
        CORE::CString gltfText;
        ASSERT_TRUE( CORE::LoadTextFileAsString( gltfPath, gltfText ) && gltfText.HasSubstr( "textures/" ) >= 0 );

        // the sidecar: where it came from, what the normalisation did, what the file held
        CORE::CDataNode sidecar;
        ASSERT_TRUE( LoadJsonFile( CORE::CombinePath( intermediate, "cube.conversion.json" ), sidecar ) );
        const CORE::CDataNode* provenance = sidecar.FindChild( "provenance" );
        ASSERT_TRUE( GUCEF_NULL != provenance && "fbx" == provenance->GetAttributeValueOrChildValueByName( "importer" ).AsString() && "fbx" == provenance->GetAttributeValueOrChildValueByName( "sourceFormat" ).AsString() );
        const CORE::CDataNode* normalisation = sidecar.FindChild( "normalisation" );
        ASSERT_TRUE( GUCEF_NULL != normalisation );
        if ( GUCEF_NULL != normalisation )
        {
            ASSERT_TRUE( "(+X, +Z, +Y, 100)" == normalisation->GetAttributeValueOrChildValueByName( "from" ).AsString() && "(+X, +Y, -Z, 1)" == normalisation->GetAttributeValueOrChildValueByName( "to" ).AsString() );
        }
        const CORE::CDataNode* summary = sidecar.FindChild( "summary" );
        ASSERT_TRUE( GUCEF_NULL != summary && 24 == summary->GetAttributeValueOrChildValueByName( "vertices" ).AsInt32() && 12 == summary->GetAttributeValueOrChildValueByName( "triangles" ).AsInt32() );
        if ( GUCEF_NULL != summary && GUCEF_NULL != summary->FindChild( "bounds" ) )
        {
            // the cube at (100, 200, 50) cm of a Z up file is at (1, 0.5, -2) m, one metre wide
            CORE::CDataNode::TVariantVector min;
            CORE::CDataNode::TVariantVector max;
            summary->FindChild( "bounds" )->GetValuesOfChildByName( "min", min );
            summary->FindChild( "bounds" )->GetValuesOfChildByName( "max", max );
            ASSERT_TRUE( 3 == min.size() && 3 == max.size() );
            if ( 3 == min.size() && 3 == max.size() )
            {
                ASSERT_TRUE( Near( min[ 0 ].AsFloat32(), 1.0f, 1e-4f ) && Near( min[ 1 ].AsFloat32(), 0.5f, 1e-4f ) && Near( min[ 2 ].AsFloat32(), -3.0f, 1e-4f ) );
                ASSERT_TRUE( Near( max[ 0 ].AsFloat32(), 2.0f, 1e-4f ) && Near( max[ 1 ].AsFloat32(), 1.5f, 1e-4f ) && Near( max[ 2 ].AsFloat32(), -2.0f, 1e-4f ) );
            }
        }
        const CORE::CDataNode* materials = sidecar.FindChild( "materials" );
        ASSERT_TRUE( GUCEF_NULL != materials && 1 == materials->GetNrOfDirectChildNodes() );
        const CORE::CDataNode* lossy = sidecar.FindChild( "lossySteps" );
        ASSERT_TRUE( GUCEF_NULL != lossy && lossy->GetNrOfDirectChildNodes() >= 1 );
        const CORE::CDataNode* extras = sidecar.FindChild( "extras" );
        ASSERT_TRUE( GUCEF_NULL != extras && GUCEF_NULL != extras->FindChild( "fbx" ) );

        // again: from the cache
        ASSERT_TRUE( CAssetConverterApp::ExitSuccess == RunCommandLine( commandLine, &report ) );
        ASSERT_TRUE( 1 == report.GetAssetCount( ASSETSTATUS_CACHED ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CLI: a skinned FBX with a take converts to a glTF that validates, with skin and animation in it" )
    try
    {
        MakeWorkFolder( "cliskin" );
        ASSERT_TRUE( WriteWorkText( "cliskin/src/strip.fbx", MakeSkinnedFbx( FbxYUpMetres(), 5 ) ) );
        ASSERT_TRUE( WriteLicence( "cliskin/licences", "rec" ) );
        const CORE::CString outFolder = WorkDiskPath( "cliskin/out" );
        CCommandLine commandLine;
        commandLine.Add( "--mount" ).Add( "single=" + WorkDiskPath( "cliskin/src" ) ).Add( "--input" ).Add( "single/strip.fbx" ).Add( "--importer" ).Add( "fbx" )
                   .Add( "--licence" ).Add( WorkDiskPath( "cliskin/licences/rec.licence.json" ) ).Add( "--out" ).Add( outFolder );
        CConversionReport report;
        ASSERT_TRUE( CAssetConverterApp::ExitSuccess == RunCommandLine( commandLine, &report ) );

        const CORE::CString gltfPath = CORE::CombinePath( CORE::CombinePath( outFolder, "intermediate" ), "strip.gltf" );
        CORE::CString problem;
        ASSERT_TRUE( ValidateWithCgltf( gltfPath, problem ) );
        CORE::CString gltfText;
        ASSERT_TRUE( CORE::LoadTextFileAsString( gltfPath, gltfText ) );
        ASSERT_TRUE( gltfText.HasSubstr( "\"skins\"" ) >= 0 && gltfText.HasSubstr( "JOINTS_1" ) >= 0 && gltfText.HasSubstr( "\"animations\"" ) >= 0 && gltfText.HasSubstr( "Take 001" ) >= 0 );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
