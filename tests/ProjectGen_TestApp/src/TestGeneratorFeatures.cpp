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

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestGeneratorFeatures.h"

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

/**
 *  Creates a project with 2 modules:
 *      LibA: has source files for different processors of which the ones for others than the build target are
 *            excluded using a wildcard, and has compiler flags
 *      LibB: depends on LibA but only on the platform linux64
 */
static bool
CreateTestProject( const CORE::CString& rootDir )
{GUCEF_TRACE;

    CORE::CString libADir = CORE::CombinePath( rootDir, "LibA" );
    CORE::CString libBDir = CORE::CombinePath( rootDir, "LibB" );

    return WriteTestFile( libADir, "ModuleInfo.xml",
                          "<ModuleInfoEntry>\n"
                          "    <Module Name=\"LibA\" Type=\"SharedLibrary\" Platform=\"All\" >\n"
                          "        <Compiler>\n"
                          "            <CompilerFlags CompilerName=\"G++\" Flags=\"-std=gnu++14 -Wno-narrowing\" />\n"
                          "        </Compiler>\n"
                          "    </Module>\n"
                          "</ModuleInfoEntry>\n" ) &&
           WriteTestFile( CORE::CombinePath( libADir, "include" ), "LibA.h", "/* public header */\n" ) &&
           WriteTestFile( CORE::CombinePath( libADir, "src" ), "dsp.a.c", "/* source */\n" ) &&
           WriteTestFile( CORE::CombinePath( libADir, "src" ), "dsp.a_neon.c", "/* for arm */\n" ) &&
           WriteTestFile( CORE::CombinePath( libADir, "src" ), "dsp.a_mips_dsp_r2.c", "/* for mips */\n" ) &&
           WriteTestFile( CORE::CombinePath( libADir, "src" ), "ProjectGenInstructions.xml",
                          "<PROJECTGENERATOR>\n"
                          "    <EXCLUDES PLATFORM=\"ALL\">\n"
                          "        <ITEM NAME=\"*mips*.c\" TYPE=\"FILE\" />\n"
                          "        <ITEM NAME=\"*_neon.c\" TYPE=\"FILE\" />\n"
                          "    </EXCLUDES>\n"
                          "</PROJECTGENERATOR>\n" ) &&
           WriteTestFile( libBDir, "ModuleInfo.xml",
                          "<ModuleInfoEntry>\n"
                          "    <Module Name=\"LibB\" Type=\"SharedLibrary\" Platform=\"All\" >\n"
                          "    </Module>\n"
                          "    <Module Platform=\"linux64\" >\n"
                          "        <Dependencies>\n"
                          "            <Dependency Name=\"LibA\" />\n"
                          "        </Dependencies>\n"
                          "    </Module>\n"
                          "</ModuleInfoEntry>\n" ) &&
           WriteTestFile( CORE::CombinePath( libBDir, "src" ), "b.cpp", "/* source */\n" );
}

/*-------------------------------------------------------------------------*/

static bool
IsLibAIncludeDir( const CORE::CString& path )
{GUCEF_TRACE;

    return path.ReplaceChar( '\\', '/' ).HasSubstr( "LibA/include" ) >= 0;
}

/*-------------------------------------------------------------------------*/

void
PerformGeneratorFeatureTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "GeneratorFeatures" );

    // Without platform definitions in the config the hardcoded ones are used: each platform under its own name with
    // its own directories. They were written through one reference that was assigned again for every platform, which
    // copied each platform over the first one and left the others empty
    GUCEF_TESTFW_TESTCASE( "Generator: the hardcoded platform definitions define each platform under its own name" )
    try
    {
        PROJECTGEN::CProjectInfoPtr projectInfo = PROJECTGEN::CProjectInfo::CreateSharedObj();
        const PROJECTGEN::TPlatformDefinitionMap& platforms = projectInfo->GetPlatformDefinitions();
        const char* names[] = { "android32", "android64", "linux32", "linux64", "win32", "win64", "emscripten32", "emscripten64", "arduino" };
        for ( size_t i=0; i<sizeof( names ) / sizeof( names[ 0 ] ); ++i )
        {
            PROJECTGEN::TPlatformDefinitionMap::const_iterator p = platforms.find( names[ i ] );
            ASSERT_TRUE( p != platforms.end() );
            if ( p == platforms.end() )
                continue;
            ASSERT_TRUE( (*p).second.name == names[ i ] );
            ASSERT_TRUE( (*p).second.platformDirs.find( names[ i ] ) != (*p).second.platformDirs.end() );
        }
        const PROJECTGEN::TPlatformDefinition& android64 = (*platforms.find( "android64" )).second;
        ASSERT_TRUE( android64.platformDirs.find( "android" ) != android64.platformDirs.end() );
        ASSERT_TRUE( android64.aliases.find( "linuxlike" ) != android64.aliases.end() );
        const PROJECTGEN::TPlatformDefinition& linux64 = (*platforms.find( "linux64" )).second;
        ASSERT_TRUE( linux64.platformDirs.find( "linux" ) != linux64.platformDirs.end() );
        ASSERT_TRUE( linux64.platformDirs.find( "android" ) == linux64.platformDirs.end() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Generator: compiler flags, wildcard file excludes and platform specific dependency includes" )
    try
    {
        ASSERT_TRUE( EnsureXmlCodecIsAvailable() );

        CORE::CString rootDir = CORE::CombinePath( CORE::TempDir(), "ProjectGen_TestApp_GeneratorFeatures" );
        ASSERT_TRUE( CreateTestProject( rootDir ) );

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

        PROJECTGEN::CModuleInfoEntryPtr libAEntry = projectInfo->GetModuleInfoEntry( "LibA" );
        ASSERT_TRUE( !libAEntry.IsNULL() );
        if ( !libAEntry.IsNULL() )
        {
            PROJECTGEN::CModuleInfoPtr libAInfo = libAEntry->FindModuleInfoForPlatform( PROJECTGEN::KnownPlatforms::AllPlatforms );
            ASSERT_TRUE( !libAInfo.IsNULL() );
            if ( !libAInfo.IsNULL() )
            {
                // the compiler flags of the module are known
                CORE::CStringMap::const_iterator flags = libAInfo->compilerSettings.GetCompilerFlags().find( "G++" );
                ASSERT_TRUE( flags != libAInfo->compilerSettings.GetCompilerFlags().end() );
                if ( flags != libAInfo->compilerSettings.GetCompilerFlags().end() )
                    ASSERT_TRUE( (*flags).second == "-std=gnu++14 -Wno-narrowing" );

                // only the source file which is not for another processor is part of the module
                bool foundGeneric = false;
                bool foundOthers = false;
                PROJECTGEN::TStringSetMap::const_iterator d = libAInfo->GetSourceDirs().begin();
                while ( d != libAInfo->GetSourceDirs().end() )
                {
                    PROJECTGEN::TStringSet::const_iterator f = (*d).second.begin();
                    while ( f != (*d).second.end() )
                    {
                        if ( (*f) == "dsp.a.c" )
                            foundGeneric = true;
                        if ( (*f) == "dsp.a_neon.c" || (*f) == "dsp.a_mips_dsp_r2.c" )
                            foundOthers = true;
                        ++f;
                    }
                    ++d;
                }
                ASSERT_TRUE( foundGeneric );
                ASSERT_FALSE( foundOthers );
            }
        }

        // LibB has LibA as dependency on linux64 only, which is where it needs its include dirs, and only there
        PROJECTGEN::CModuleInfoEntryPtr libBEntry = projectInfo->GetModuleInfoEntry( "LibB" );
        ASSERT_TRUE( !libBEntry.IsNULL() );
        if ( !libBEntry.IsNULL() )
        {
            PROJECTGEN::TStringSet linuxDirs;
            libBEntry->GetIncludeDirsForPlatform( "linux64", linuxDirs, false, true );
            bool linuxHasLibA = false;
            for ( PROJECTGEN::TStringSet::const_iterator n = linuxDirs.begin(); n != linuxDirs.end(); ++n )
                linuxHasLibA = linuxHasLibA || IsLibAIncludeDir( (*n) );
            ASSERT_TRUE( linuxHasLibA );

            PROJECTGEN::TStringSet allDirs;
            libBEntry->GetIncludeDirsForPlatform( PROJECTGEN::KnownPlatforms::AllPlatforms, allDirs, false, true );
            bool allHasLibA = false;
            for ( PROJECTGEN::TStringSet::const_iterator n = allDirs.begin(); n != allDirs.end(); ++n )
                allHasLibA = allHasLibA || IsLibAIncludeDir( (*n) );
            ASSERT_FALSE( allHasLibA );

            PROJECTGEN::TStringSet windowsDirs;
            libBEntry->GetIncludeDirsForPlatform( "win64", windowsDirs, false, true );
            bool windowsHasLibA = false;
            for ( PROJECTGEN::TStringSet::const_iterator n = windowsDirs.begin(); n != windowsDirs.end(); ++n )
                windowsHasLibA = windowsHasLibA || IsLibAIncludeDir( (*n) );
            ASSERT_FALSE( windowsHasLibA );
        }
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
