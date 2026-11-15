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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_TESTFW_H
#include "gucef_test_framework.h"
#define GUCEF_TESTFW_H
#endif /* GUCEF_TESTFW_H ? */

#include "TestUnitySupport.h"
#include "TestUnityContent.h"
#include "TestUnityCli.h"

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
PerformUnityCliTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "assetpluginUNITY.Cli" );

    GUCEF_TESTFW_TESTCASE( "CLI: a package mounted with --mount converts to valid glTF with sidecars, textures and a report of warnings, not errors" )
    try
    {
        ASSERT_TRUE( WriteTestProject( "cli" ) );
        ASSERT_TRUE( WriteLicence( "cli/manifest", "rec" ) );
        ASSERT_TRUE( WriteWorkText( "cli/manifest/test.assetpack.json", "{ \"id\": \"unitycli\", \"licenceRecord\": \"rec\", \"source\": \"/pkg/test.unitypackage\", "
                                    "\"importer\": \"unity\", \"exclude\": [ \"Assets/Models/mover.fbx\" ] }" ) );
        const CORE::CString outFolder = WorkDiskPath( "cli/out" );
        CCommandLine commandLine;
        commandLine.Add( "--mount" ).Add( "pkg=" + WorkDiskPath( "cli" ) ).Add( "--manifest" ).Add( WorkDiskPath( "cli/manifest/test.assetpack.json" ) )
                   .Add( "--out" ).Add( outFolder ).Add( "--report" ).Add( WorkDiskPath( "cli/report.json" ) ).Add( "--threads" ).Add( "2" );
        CConversionReport report;
        ASSERT_TRUE( CAssetConverterApp::ExitSuccess == RunCommandLine( commandLine, &report ) );
        ASSERT_TRUE( 0 == report.GetAssetCount( ASSETSTATUS_FAILED ) && 0 == report.GetItemCount( REPORTITEM_ERROR ) );
        // five prefabs, the scene, six models and the clip
        ASSERT_TRUE( 13 == report.GetAssetCount( ASSETSTATUS_CONVERTED ) + report.GetAssetCount( ASSETSTATUS_APPROXIMATED ) );
        ASSERT_TRUE( report.GetItemCount( REPORTITEM_WARNING ) > 0 );

        const CORE::CString intermediate = WorkDiskPath( "cli/manifest/intermediate" );
        const char* const assets[] = { "Assets/Prefabs/House", "Assets/Prefabs/Street", "Assets/Scenes/Demo", "Assets/Models/house", "Assets/Animation/Spin" };
        for ( UInt32 a=0; a<5; ++a )
        {
            const CORE::CString gltf = CORE::CombinePath( intermediate, CORE::CString( assets[ a ] ) + ".gltf" );
            ASSERT_TRUE( CORE::FileExists( gltf ) && CORE::FileExists( CORE::CombinePath( intermediate, CORE::CString( assets[ a ] ) + ".conversion.json" ) ) );
            CORE::CString problem;
            ASSERT_TRUE( ValidateWithCgltf( gltf, problem ) );
        }
        CORE::CString text;
        ASSERT_TRUE( CORE::LoadTextFileAsString( CORE::CombinePath( intermediate, "Assets/Prefabs/House.gltf" ), text ) );
        ASSERT_TRUE( text.HasSubstr( "textures/" ) >= 0 && text.HasSubstr( "OMI_physics_shape" ) >= 0 && text.HasSubstr( "MSFT_lod" ) >= 0 );

        // the sidecar: provenance with the GUID, the materials of the source, the instance overrides
        CORE::CDataNode sidecar;
        ASSERT_TRUE( LoadJsonFile( CORE::CombinePath( intermediate, "Assets/Prefabs/Street.conversion.json" ), sidecar ) );
        const CORE::CDataNode* provenance = sidecar.FindChild( "provenance" );
        ASSERT_TRUE( GUCEF_NULL != provenance && "unity" == provenance->GetAttributeValueOrChildValueByName( "importer" ).AsString() &&
                     CORE::CString( GuidStreetPrefab ) == provenance->GetAttributeValueOrChildValueByName( "sourceId" ).AsString() );
        const CORE::CDataNode* normalisation = sidecar.FindChild( "normalisation" );
        ASSERT_TRUE( GUCEF_NULL != normalisation && "(+X, +Y, +Z, 1)" == normalisation->GetAttributeValueOrChildValueByName( "from" ).AsString() );
        const CORE::CDataNode* prefabs = sidecar.FindChild( "prefabs" );
        ASSERT_TRUE( GUCEF_NULL != prefabs && 1 == prefabs->GetNrOfDirectChildNodes() );
        ASSERT_TRUE( CORE::FileExists( WorkDiskPath( "cli/report.json" ) ) );

        // again: from the cache
        ASSERT_TRUE( CAssetConverterApp::ExitSuccess == RunCommandLine( commandLine, &report ) );
        ASSERT_TRUE( 13 == report.GetAssetCount( ASSETSTATUS_CACHED ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
