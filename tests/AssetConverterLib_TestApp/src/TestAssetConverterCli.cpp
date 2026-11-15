/*
 *  AssetConverterLib_TestApp: Test application for the asset converter library (AssetConverterLib)
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

#include "TestAssetConverterSupport.h"
#include "TestAssetConverterCli.h"

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

bool
Parses( CCommandLine& commandLine )
{
    CAssetConverterApp app( commandLine.GetCount(), commandLine.GetArguments() );
    return app.ParseCommandLine();
}

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

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformAssetConverterCliTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "AssetConverterLib.Cli" );

    GUCEF_TESTFW_TESTCASE( "CLI: the command line is read; bad command lines and help are refused; the settings give defaults" )
    try
    {
        CCommandLine none;
        ASSERT_FALSE( Parses( none ) );
        CCommandLine both;
        both.Add( "--manifest" ).Add( "a.json" ).Add( "--input" ).Add( "b.stub" );
        ASSERT_FALSE( Parses( both ) );
        CCommandLine inputOnly;
        inputOnly.Add( "--input" ).Add( "b.stub" ).Add( "--out" ).Add( "o" );
        ASSERT_FALSE( Parses( inputOnly ) );
        CCommandLine noOutput;
        noOutput.Add( "--input" ).Add( "b.stub" ).Add( "--importer" ).Add( "stub" ).Add( "--licence" ).Add( "l.json" );
        ASSERT_FALSE( Parses( noOutput ) );
        CCommandLine unknown;
        unknown.Add( "--manifest" ).Add( "a.json" ).Add( "--bogus" );
        ASSERT_FALSE( Parses( unknown ) );
        CCommandLine missingValue;
        missingValue.Add( "--manifest" );
        ASSERT_FALSE( Parses( missingValue ) );
        CCommandLine help;
        help.Add( "--manifest" ).Add( "a.json" ).Add( "--help" );
        ASSERT_FALSE( Parses( help ) );

        CCommandLine full;
        full.Add( "--manifest" ).Add( "a.json" ).Add( "--cook" ).Add( "ogre, ogrenext" ).Add( "--cook" ).Add( "stubcook" ).Add( "--force" ).Add( "--reimport" )
            .Add( "--threads" ).Add( "3" ).Add( "--out" ).Add( "o" ).Add( "--report" ).Add( "r.json" ).Add( "--log" ).Add( "x.log" ).Add( "--bootstrap" ).Add( "b.ini" ).Add( "--config" ).Add( "c.json" );
        CAssetConverterApp app( full.GetCount(), full.GetArguments() );
        ASSERT_TRUE( app.ParseCommandLine() );
        const CAssetPipelineOptions& options = app.GetOptions();
        ASSERT_TRUE( 3 == options.cookTargets.size() && "ogre" == options.cookTargets[ 0 ] && "ogrenext" == options.cookTargets[ 1 ] && "stubcook" == options.cookTargets[ 2 ] );
        ASSERT_TRUE( options.force && options.reimport && 3 == options.threads );
        ASSERT_TRUE( "x.log" == app.GetLogFile() && "b.ini" == app.GetBootstrapConfigFile() && "c.json" == app.GetConfigFile() );

        // the settings: the command line wins; what it leaves open the settings fill
        CORE::CDataNode config( "config", GUCEF_DATATYPE_OBJECT );
        CORE::CDataNode* section = config.AddChild( CAssetConverterApp::AppName, GUCEF_DATATYPE_OBJECT );
        section->SetAttribute( "threads", (UInt32) 7 );
        section->SetAttribute( "threadPoolName", "Converters" );
        ASSERT_TRUE( app.LoadConfig( config ) );
        ASSERT_TRUE( 3 == app.GetOptions().threads && "Converters" == app.GetOptions().threadPoolName );
        CCommandLine plain;
        plain.Add( "--manifest" ).Add( "a.json" );
        CAssetConverterApp other( plain.GetCount(), plain.GetArguments() );
        ASSERT_TRUE( other.ParseCommandLine() && other.LoadConfig( config ) && 7 == other.GetOptions().threads );
        CORE::CDataNode saved( "config", GUCEF_DATATYPE_OBJECT );
        ASSERT_TRUE( other.SaveConfig( saved ) && GUCEF_NULL != saved.FindChild( CAssetConverterApp::AppName ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CLI: a manifest runs end to end, with the exit codes of success, cache, refusal, failures and an unreadable manifest" )
    try
    {
        WriteStandardPack( "cli", CORE::CString::Empty );
        const CORE::CString manifestFile = WorkDiskPath( "cli/pack.assetpack.json" );
        const CORE::CString outFolder = WorkDiskPath( "cli/cooked" );
        const CORE::CString reportFile = WorkDiskPath( "cli/reports/run.json" );

        CCommandLine commandLine;
        commandLine.Add( "--manifest" ).Add( manifestFile ).Add( "--out" ).Add( outFolder ).Add( "--report" ).Add( reportFile ).Add( "--threads" ).Add( "2" );
        CConversionReport report;
        ASSERT_TRUE( CAssetConverterApp::ExitSuccess == RunCommandLine( commandLine, &report ) );
        ASSERT_TRUE( 2 == report.GetAssetCount( ASSETSTATUS_CONVERTED ) );
        ASSERT_TRUE( CORE::FileExists( reportFile ) && CORE::FileExists( WorkDiskPath( "cli/reports/run.txt" ) ) );
        ASSERT_TRUE( CORE::FileExists( WorkDiskPath( "cli/intermediate/props/rock.gltf" ) ) && CORE::FileExists( WorkDiskPath( "cli/intermediate/props/rock.conversion.json" ) ) );
        ASSERT_TRUE( CORE::FileExists( CORE::CombinePath( outFolder, "desktop/cli/rock.mesh" ) ) && CORE::FileExists( CORE::CombinePath( outFolder, "desktop/references.json" ) ) );
        ASSERT_FALSE( VFS::CVfsGlobal::Instance()->GetVfs().IsMountedArchive( "out" ) );

        // again: from the cache
        ASSERT_TRUE( CAssetConverterApp::ExitSuccess == RunCommandLine( commandLine, &report ) );
        ASSERT_TRUE( 2 == report.GetAssetCount( ASSETSTATUS_CACHED ) );

        // without the licence record: refused
        CORE::DeleteFile( WorkDiskPath( "cli/licences/rec.licence.json" ) );
        ASSERT_TRUE( CAssetConverterApp::ExitRefused == RunCommandLine( commandLine, &report ) );
        ASSERT_TRUE( report.IsRefused() );

        // with an asset that fails
        WriteLicence( "cli/licences", "rec", false, false, false, false );
        WriteWorkFile( "cli/src/props/broken.stub", "{ \"fail\": true }" );
        ASSERT_TRUE( CAssetConverterApp::ExitFailures == RunCommandLine( commandLine, &report ) );
        ASSERT_TRUE( 1 == report.GetAssetCount( ASSETSTATUS_FAILED ) && 2 == report.GetAssetCount( ASSETSTATUS_CACHED ) );

        // a manifest that is not there
        CCommandLine missing;
        missing.Add( "--manifest" ).Add( WorkDiskPath( "cli/none.assetpack.json" ) );
        ASSERT_TRUE( CAssetConverterApp::ExitConfig == RunCommandLine( missing ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CLI: one source with an importer and a licence record runs as a pack of its own" )
    try
    {
        WriteStandardPack( "clisingle", CORE::CString::Empty );
        const CORE::CString outFolder = WorkDiskPath( "clisingle/cooked" );
        CCommandLine commandLine;
        commandLine.Add( "--mount" ).Add( "single=" + WorkDiskPath( "clisingle/src" ) ).Add( "--input" ).Add( "single/props/crate.stub" )
                   .Add( "--importer" ).Add( "stub" ).Add( "--licence" ).Add( WorkDiskPath( "clisingle/licences/rec.licence.json" ) )
                   .Add( "--out" ).Add( outFolder ).Add( "--cook" ).Add( "stubcook" );
        CConversionReport report;
        ASSERT_TRUE( CAssetConverterApp::ExitSuccess == RunCommandLine( commandLine, &report ) );
        ASSERT_TRUE( "crate" == report.GetPackId() && "rec" == report.GetLicenceRecordId() && 1 == report.GetAssetCount( ASSETSTATUS_CONVERTED ) );
        ASSERT_TRUE( CORE::FileExists( CORE::CombinePath( outFolder, "intermediate/crate.gltf" ) ) );
        ASSERT_TRUE( CORE::FileExists( CORE::CombinePath( outFolder, "desktop/crate/crate.mesh" ) ) );
        ASSERT_FALSE( VFS::CVfsGlobal::Instance()->GetVfs().IsMountedArchive( "single" ) );

        // an intermediate folder of its own
        const CORE::CString interFolder = WorkDiskPath( "clisingle/inter" );
        commandLine.Add( "--intermediate" ).Add( interFolder ).Add( "--pack" ).Add( "single" );
        ASSERT_TRUE( CAssetConverterApp::ExitSuccess == RunCommandLine( commandLine, &report ) );
        ASSERT_TRUE( "single" == report.GetPackId() && CORE::FileExists( CORE::CombinePath( interFolder, "crate.gltf" ) ) );

        // an importer that is not there fails, a licence record that is not there is a config error
        CCommandLine noImporter;
        noImporter.Add( "--mount" ).Add( "single=" + WorkDiskPath( "clisingle/src" ) ).Add( "--input" ).Add( "single/props/crate.stub" )
                  .Add( "--importer" ).Add( "nosuchimporter" ).Add( "--licence" ).Add( WorkDiskPath( "clisingle/licences/rec.licence.json" ) ).Add( "--out" ).Add( outFolder );
        ASSERT_TRUE( CAssetConverterApp::ExitFailures == RunCommandLine( noImporter ) );
        CCommandLine noLicence;
        noLicence.Add( "--input" ).Add( "single/props/crate.stub" ).Add( "--importer" ).Add( "stub" ).Add( "--licence" ).Add( WorkDiskPath( "clisingle/none.licence.json" ) ).Add( "--out" ).Add( outFolder );
        ASSERT_TRUE( CAssetConverterApp::ExitConfig == RunCommandLine( noLicence ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CLI: --mount takes [<mount point>=]<folder>; a folder whose path contains '=' mounts with and without a mount point" )
    try
    {
        WriteStandardPack( "cli=eq", CORE::CString::Empty );
        const CORE::CString source = WorkDiskPath( "cli=eq/src" );
        const CORE::CString outFolder = WorkDiskPath( "cli=eq/cooked" );

        // no mount point: the text before the first '=' holds a path separator, so it is all folder, mounted by its last directory name
        CCommandLine plain;
        plain.Add( "--mount" ).Add( source ).Add( "--input" ).Add( "src/props/crate.stub" ).Add( "--importer" ).Add( "stub" )
             .Add( "--licence" ).Add( WorkDiskPath( "cli=eq/licences/rec.licence.json" ) ).Add( "--out" ).Add( outFolder );
        CConversionReport report;
        ASSERT_TRUE( CAssetConverterApp::ExitSuccess == RunCommandLine( plain, &report ) );
        ASSERT_TRUE( 1 == report.GetAssetCount( ASSETSTATUS_CONVERTED ) );

        // a mount point before the first '=', the rest is the folder, '=' included
        CCommandLine named;
        named.Add( "--mount" ).Add( "eqpoint=" + source ).Add( "--input" ).Add( "eqpoint/props/crate.stub" ).Add( "--importer" ).Add( "stub" )
             .Add( "--licence" ).Add( WorkDiskPath( "cli=eq/licences/rec.licence.json" ) ).Add( "--out" ).Add( WorkDiskPath( "cli=eq/cooked_named" ) );
        ASSERT_TRUE( CAssetConverterApp::ExitSuccess == RunCommandLine( named, &report ) );
        ASSERT_TRUE( 1 == report.GetAssetCount( ASSETSTATUS_CONVERTED ) );
        ASSERT_FALSE( VFS::CVfsGlobal::Instance()->GetVfs().IsMountedArchive( "eqpoint" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CLI: --dry-run, --only, --validate-only and --cook-only, and the exit code of a locked intermediate" )
    try
    {
        WriteStandardPack( "climodes", CORE::CString::Empty );
        const CORE::CString manifestFile = WorkDiskPath( "climodes/pack.assetpack.json" );
        const CORE::CString planFile = WorkDiskPath( "climodes/reports/plan.json" );

        // a dry run: the plan, nothing else
        CCommandLine dryRun;
        dryRun.Add( "--manifest" ).Add( manifestFile ).Add( "--dry-run" ).Add( "--report" ).Add( planFile );
        ASSERT_TRUE( CAssetConverterApp::ExitSuccess == RunCommandLine( dryRun ) );
        ASSERT_TRUE( CORE::FileExists( planFile ) && !CORE::DirExists( WorkDiskPath( "climodes/intermediate" ) ) && !CORE::DirExists( WorkDiskPath( "climodes/out" ) ) );
        CCommandLine parsedDryRun;
        parsedDryRun.Add( "--manifest" ).Add( manifestFile ).Add( "--dry-run" ).Add( "--only" ).Add( "props/rock*" ).Add( "--only" ).Add( "props/crate.stub" ).Add( "--validate-only" );
        {
            CAssetConverterApp app( parsedDryRun.GetCount(), parsedDryRun.GetArguments() );
            ASSERT_TRUE( app.ParseCommandLine() && app.IsDryRun() && 2 == app.GetOptions().assetGlobs.size() && PIPELINEMODE_VALIDATE_ONLY == app.GetOptions().mode );
        }

        // validate-only writes nothing without a report file
        CCommandLine validate;
        validate.Add( "--manifest" ).Add( manifestFile ).Add( "--validate-only" );
        CConversionReport report;
        ASSERT_TRUE( CAssetConverterApp::ExitSuccess == RunCommandLine( validate, &report ) );
        ASSERT_TRUE( PIPELINEMODE_VALIDATE_ONLY == report.GetMode() && !CORE::DirExists( WorkDiskPath( "climodes/intermediate" ) ) );

        // one asset only
        CCommandLine only;
        only.Add( "--manifest" ).Add( manifestFile ).Add( "--only" ).Add( "props/rock.stub" );
        ASSERT_TRUE( CAssetConverterApp::ExitSuccess == RunCommandLine( only, &report ) );
        ASSERT_TRUE( GUCEF_NULL != report.FindEntry( "props/rock.stub" ) && GUCEF_NULL == report.FindEntry( "props/crate.stub" ) );

        // all, then cook-only
        CCommandLine all;
        all.Add( "--manifest" ).Add( manifestFile );
        ASSERT_TRUE( CAssetConverterApp::ExitSuccess == RunCommandLine( all, &report ) );
        CCommandLine cookOnly;
        cookOnly.Add( "--manifest" ).Add( manifestFile ).Add( "--cook-only" );
        ASSERT_TRUE( CAssetConverterApp::ExitSuccess == RunCommandLine( cookOnly, &report ) );
        ASSERT_TRUE( PIPELINEMODE_COOK_ONLY == report.GetMode() && 2 == report.GetAssetCount( ASSETSTATUS_CACHED ) );

        // both modes at once: refused
        CCommandLine both;
        both.Add( "--manifest" ).Add( manifestFile ).Add( "--cook-only" ).Add( "--validate-only" );
        ASSERT_FALSE( Parses( both ) );

        // the intermediate locked by another run
        {
            CIntermediateLock lock( "work/climodes/intermediate" );
            ASSERT_TRUE( lock.IsHeld() );
            ASSERT_TRUE( CAssetConverterApp::ExitBusy == RunCommandLine( all, &report ) );
            ASSERT_TRUE( report.IsLocked() );
        }
        ASSERT_TRUE( CAssetConverterApp::ExitSuccess == RunCommandLine( all, &report ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
