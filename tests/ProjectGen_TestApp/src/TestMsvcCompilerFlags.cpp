/*
 *  ProjectGen_TestApp: GUCEF test application for the ProjectGen module
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_PROJECTGEN_CDIRCRAWLINGPROJECTINFOGATHERER_H
#include "gucefProjectGen_CDirCrawlingProjectInfoGatherer.h"
#define GUCEF_PROJECTGEN_CDIRCRAWLINGPROJECTINFOGATHERER_H
#endif /* GUCEF_PROJECTGEN_CDIRCRAWLINGPROJECTINFOGATHERER_H ? */

#ifndef GUCEF_PROJECTGEN_CPROJECTINFO_H
#include "gucefProjectGen_CProjectInfo.h"
#define GUCEF_PROJECTGEN_CPROJECTINFO_H
#endif /* GUCEF_PROJECTGEN_CPROJECTINFO_H ? */

#ifndef GUCEF_PROJECTGEN_CMODULEINFOENTRY_H
#include "gucefProjectGen_CModuleInfoEntry.h"
#define GUCEF_PROJECTGEN_CMODULEINFOENTRY_H
#endif /* GUCEF_PROJECTGEN_CMODULEINFOENTRY_H ? */

#ifndef GUCEF_PROJECTGEN_CCMAKEPROJECTGENERATOR_H
#include "gucefProjectGen_CCMakeProjectGenerator.h"
#define GUCEF_PROJECTGEN_CCMAKEPROJECTGENERATOR_H
#endif /* GUCEF_PROJECTGEN_CCMAKEPROJECTGENERATOR_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestMsvcCompilerFlags.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

/*-------------------------------------------------------------------------*/

static bool
EnsureXmlCodecIsAvailable( void )
{GUCEF_TRACE;

    CORE::CCoreGlobal* coreGlobal = CORE::CCoreGlobal::Instance();
    CORE::CDStoreCodecRegistry::TDStoreCodecPtr xmlCodec;
    if ( coreGlobal->GetDStoreCodecRegistry().TryLookup( "xml", xmlCodec, false ) )
        return true;

    #ifdef GUCEF_DEBUG_MODE
    CORE::CString firstPluginPath = "$MODULEDIR$/dstorepluginPARSIFALXML_d";
    CORE::CString secondPluginPath = "$MODULEDIR$/dstorepluginPARSIFALXML";
    #else
    CORE::CString firstPluginPath = "$MODULEDIR$/dstorepluginPARSIFALXML";
    CORE::CString secondPluginPath = "$MODULEDIR$/dstorepluginPARSIFALXML_d";
    #endif

    CORE::CPluginControl& pluginControl = coreGlobal->GetPluginControl();
    if ( !pluginControl.AddPluginFromDir( firstPluginPath, "test", true ) )
        pluginControl.AddPluginFromDir( secondPluginPath, "test", true );

    return coreGlobal->GetDStoreCodecRegistry().TryLookup( "xml", xmlCodec, false );
}

/*-------------------------------------------------------------------------*/

static bool
WriteTestFile( const CORE::CString& dir      ,
               const CORE::CString& filename ,
               const CORE::CString& content  )
{GUCEF_TRACE;

    if ( !CORE::CreateDirs( dir ) )
        return false;
    return CORE::WriteStringAsTextFile( CORE::CombinePath( dir, filename ), content );
}

/*-------------------------------------------------------------------------*/

void
PerformMsvcCompilerFlagsTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "MsvcCompilerFlags" );

    GUCEF_TESTFW_TESTCASE( "CMake generator: MSVC compiler flags are parsed and emitted as target options for MSVC only" )
    try
    {
        ASSERT_TRUE( EnsureXmlCodecIsAvailable() );

        CORE::CString rootDir = CORE::CombinePath( CORE::TempDir(), "ProjectGen_TestApp_MsvcCompilerFlags" );
        CORE::CString libDir = CORE::CombinePath( rootDir, "LibM" );
        ASSERT_TRUE( WriteTestFile( libDir, "ModuleInfo.xml",
                                    "<ModuleInfoEntry>\n"
                                    "    <Module Name=\"LibM\" Type=\"SharedLibrary\" Platform=\"All\" >\n"
                                    "        <Compiler>\n"
                                    "            <CompilerFlags CompilerName=\"G++\" Flags=\"-ffp-contract=off\" />\n"
                                    "            <CompilerFlags CompilerName=\"MSVC\" Flags=\"/fp:precise /Gy\" />\n"
                                    "        </Compiler>\n"
                                    "    </Module>\n"
                                    "    <Module Platform=\"linux64\" >\n"
                                    "        <Preprocessor>\n"
                                    "            <Define String=\"LIBM_LINUX\" />\n"
                                    "        </Preprocessor>\n"
                                    "    </Module>\n"
                                    "</ModuleInfoEntry>\n" ) );
        ASSERT_TRUE( WriteTestFile( CORE::CombinePath( libDir, "include" ), "LibM.h", "/* public header */\n" ) );
        ASSERT_TRUE( WriteTestFile( CORE::CombinePath( libDir, "src" ), "m.cpp", "/* source */\n" ) );

        PROJECTGEN::CProjectInfoPtr projectInfo = PROJECTGEN::CProjectInfo::CreateSharedObj();
        CORE::CDataNode config( "ProjectGenerator" );
        CORE::CDataNode* platformDefs = config.AddChild( "PlatformDefinitions" );
        CORE::CDataNode* linux64 = platformDefs->AddChild( "linux64" );
        linux64->SetAttribute( "Aliases", "linux;linuxlike" );
        linux64->SetAttribute( "PlatformDirs", "linux64;linux" );
        CORE::CDataNode* win64 = platformDefs->AddChild( "win64" );
        win64->SetAttribute( "Aliases", "mswin;windows" );
        win64->SetAttribute( "PlatformDirs", "win64;mswin;windows" );
        projectInfo->ApplyConfig( config );
        PROJECTGEN::CDirCrawlingProjectInfoGatherer infoGatherer;
        CORE::CValueList params;
        PROJECTGEN::TStringVector rootDirs;
        rootDirs.push_back( rootDir );
        ASSERT_TRUE( infoGatherer.GatherInfo( rootDirs, projectInfo, params ) );

        PROJECTGEN::CModuleInfoEntryPtr entry = projectInfo->GetModuleInfoEntry( "LibM" );
        ASSERT_TRUE( !entry.IsNULL() );
        if ( entry.IsNULL() )
            return;

        PROJECTGEN::CModuleInfoPtr info = entry->FindModuleInfoForPlatform( PROJECTGEN::KnownPlatforms::AllPlatforms );
        ASSERT_TRUE( !info.IsNULL() );
        if ( !info.IsNULL() )
        {
            const CORE::CStringMap& flags = info->compilerSettings.GetCompilerFlags();
            CORE::CStringMap::const_iterator msvc = flags.find( "MSVC" );
            ASSERT_TRUE( msvc != flags.end() );
            if ( msvc != flags.end() )
                ASSERT_TRUE( (*msvc).second == "/fp:precise /Gy" );
        }

        PROJECTGEN::CCMakeProjectGenerator generator;
        CORE::CString outputDir = CORE::CombinePath( rootDir, "out" );
        ASSERT_TRUE( generator.GenerateProject( *projectInfo, outputDir, false, params ) );

        CORE::CString cmakeLists;
        ASSERT_TRUE( CORE::LoadTextFileAsString( CORE::CombinePath( libDir, "CMakeLists.txt" ), cmakeLists, true, "\n" ) );
        ASSERT_TRUE( cmakeLists.HasSubstr( "if ( MSVC )\n  target_compile_options( ${MODULE_NAME} PRIVATE /fp:precise;/Gy )\nendif()\n" ) >= 0 );
        ASSERT_TRUE( cmakeLists.HasSubstr( "$<$<COMPILE_LANGUAGE:CXX>:-ffp-contract=off>" ) >= 0 );

        // The GNU flags must not leak into the MSVC block and the other way around
        ASSERT_TRUE( cmakeLists.HasSubstr( "/fp:precise;-ffp-contract=off" ) < 0 );
        ASSERT_TRUE( cmakeLists.HasSubstr( "-ffp-contract=off;/fp:precise" ) < 0 );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
