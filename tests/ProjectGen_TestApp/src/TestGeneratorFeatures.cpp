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

#ifndef GUCEF_PROJECTGEN_CMODULEDEPENDENCYNODE_H
#include "gucefProjectGen_CModuleDependencyNode.h"
#define GUCEF_PROJECTGEN_CMODULEDEPENDENCYNODE_H
#endif /* GUCEF_PROJECTGEN_CMODULEDEPENDENCYNODE_H ? */

#ifndef GUCEF_PROJECTGEN_CPROJECTTARGETINFOBUNDLE_H
#include "gucefProjectGen_CProjectTargetInfoBundle.h"
#define GUCEF_PROJECTGEN_CPROJECTTARGETINFOBUNDLE_H
#endif /* GUCEF_PROJECTGEN_CPROJECTTARGETINFOBUNDLE_H ? */

#ifndef GUCEF_CORE_CSTOPWATCH_H
#include "gucefCORE_CStopwatch.h"
#define GUCEF_CORE_CSTOPWATCH_H
#endif /* GUCEF_CORE_CSTOPWATCH_H ? */

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

static const char* g_closureTestPlatforms[] = { "linux32", "linux64", "win32", "win64", "ios", "osx", "android32", "android64",
                                                "emscripten32", "emscripten64", "arduino", "unix" };
static const UInt32 g_closureTestPlatformCount = sizeof( g_closureTestPlatforms ) / sizeof( g_closureTestPlatforms[ 0 ] );
static const UInt32 g_closureTestLayers = 20;
static const UInt32 g_closureTestLayerWidth = 10;

// Generous bound for a debug build on a slow machine: with the dependency graph below, walking every dependency path
// instead of every module once does not finish at all
static const UInt64 g_closureTestMaxMicroseconds = 20000000;

/*-------------------------------------------------------------------------*/

static CORE::CString
ClosureTestModuleName( UInt32 layer, UInt32 index )
{GUCEF_TRACE;

    return "Mod" + CORE::ToString( layer ) + "_" + CORE::ToString( index );
}

/*-------------------------------------------------------------------------*/

static CORE::CString
ClosureTestDependencyList( const CORE::CStringVector& dependencyNames )
{GUCEF_TRACE;

    CORE::CString dependencies;
    CORE::CString linker;
    for ( CORE::CStringVector::const_iterator d = dependencyNames.begin(); d != dependencyNames.end(); ++d )
    {
        dependencies += "            <Dependency Name=\"" + (*d) + "\" />\n";
        linker += "            <Dependency Name=\"" + (*d) + "\" />\n";
    }
    return "        <Dependencies>\n" + dependencies + "        </Dependencies>\n"
           "        <Linker>\n" + linker + "        </Linker>\n";
}

/*-------------------------------------------------------------------------*/

/**
 *  Creates a project of 200 modules in 20 layers of 10. Each module depends on 3 modules of the layer below it, as
 *  dependency and as linker dependency, so every module of a layer is reachable through very many paths. The top layer
 *  are executables tagged "closuretest_apps", half of the middle layer is tagged "closuretest_mid".
 *  On linux64 the first module of each layer also depends on a linux64 only module "LinuxOnly<layer>", which depends on
 *  the one of the layer below, so the linux64 targets have modules that the 'all' platform targets do not have.
 *  A leaf module has a definition for each of the 12 platforms, so they are all platforms used by the project.
 */
static bool
CreateDependencyClosureTestProject( const CORE::CString& rootDir )
{GUCEF_TRACE;

    bool success = true;
    for ( UInt32 layer=0; layer<g_closureTestLayers; ++layer )
    {
        for ( UInt32 index=0; index<g_closureTestLayerWidth; ++index )
        {
            CORE::CString name = ClosureTestModuleName( layer, index );
            bool isTopLayer = ( layer == g_closureTestLayers-1 );

            CORE::CString tags;
            if ( isTopLayer )
                tags = " Tags=\"closuretest_apps\"";
            else
            if ( layer == g_closureTestLayers/2 && 0 == index % 2 )
                tags = " Tags=\"closuretest_mid\"";

            CORE::CString xml = "<ModuleInfoEntry>\n"
                                "    <Module Name=\"" + name + "\" Type=\"" + ( isTopLayer ? "Executable" : "SharedLibrary" ) + "\" Platform=\"All\"" + tags + " >\n";
            if ( layer > 0 )
            {
                CORE::CStringVector dependencyNames;
                dependencyNames.push_back( ClosureTestModuleName( layer-1, index ) );
                dependencyNames.push_back( ClosureTestModuleName( layer-1, ( index+1 ) % g_closureTestLayerWidth ) );
                dependencyNames.push_back( ClosureTestModuleName( layer-1, ( index+3 ) % g_closureTestLayerWidth ) );
                xml += ClosureTestDependencyList( dependencyNames );
            }
            if ( isTopLayer && 0 == index )
            {
                xml += "        <RuntimeDependencies>\n"
                       "            <RuntimeDependency Name=\"" + ClosureTestModuleName( g_closureTestLayers/2, 5 ) + "\" />\n"
                       "        </RuntimeDependencies>\n";
            }
            xml += "    </Module>\n";

            if ( layer > 0 && 0 == index )
            {
                CORE::CStringVector dependencyNames;
                dependencyNames.push_back( "LinuxOnly" + CORE::ToString( layer ) );
                xml += "    <Module Platform=\"linux64\" >\n" + ClosureTestDependencyList( dependencyNames ) + "    </Module>\n";
            }
            if ( 0 == layer && 0 == index )
            {
                for ( UInt32 p=0; p<g_closureTestPlatformCount; ++p )
                {
                    xml += "    <Module Platform=\"" + CORE::CString( g_closureTestPlatforms[ p ] ) + "\" >\n"
                           "        <Preprocessor>\n"
                           "            <Define String=\"CLOSURETEST_PLATFORM_" + CORE::CString( g_closureTestPlatforms[ p ] ).Uppercase() + "\" />\n"
                           "        </Preprocessor>\n"
                           "    </Module>\n";
                }
            }
            xml += "</ModuleInfoEntry>\n";

            CORE::CString moduleDir = CORE::CombinePath( rootDir, name );
            success = WriteTestFile( moduleDir, "ModuleInfo.xml", xml ) &&
                      WriteTestFile( CORE::CombinePath( moduleDir, "src" ), name + ".cpp", "/* source */\n" ) && success;
        }
    }

    for ( UInt32 layer=1; layer<g_closureTestLayers; ++layer )
    {
        CORE::CString name = "LinuxOnly" + CORE::ToString( layer );
        CORE::CString xml = "<ModuleInfoEntry>\n"
                            "    <Module Name=\"" + name + "\" Type=\"SharedLibrary\" Platform=\"linux64\" >\n";
        if ( layer > 1 )
        {
            CORE::CStringVector dependencyNames;
            dependencyNames.push_back( "LinuxOnly" + CORE::ToString( layer-1 ) );
            xml += ClosureTestDependencyList( dependencyNames );
        }
        xml += "    </Module>\n"
               "</ModuleInfoEntry>\n";

        CORE::CString moduleDir = CORE::CombinePath( rootDir, name );
        success = WriteTestFile( moduleDir, "ModuleInfo.xml", xml ) &&
                  WriteTestFile( CORE::CombinePath( moduleDir, "src" ), name + ".cpp", "/* source */\n" ) && success;
    }
    return success;
}

/*-------------------------------------------------------------------------*/

/**
 *  Reference for the gathering of the dependencies of dependencies: the modules of every node reachable from the given
 *  node through dependency, linker and runtime dependency links (and logical ones if asked), each node visited once
 */
static void
ReferenceDependencyClosure( const PROJECTGEN::CModuleDependencyNodePtr& node ,
                            bool includeLogicalDependencies                  ,
                            PROJECTGEN::TModuleInfoEntryPtrSet& closure      )
{GUCEF_TRACE;

    PROJECTGEN::TModuleDependencyNodePtrSet visited;
    GUCEF::vector< PROJECTGEN::CModuleDependencyNodePtr > toVisit;
    toVisit.push_back( node );
    visited.insert( node );
    while ( !toVisit.empty() )
    {
        PROJECTGEN::CModuleDependencyNodePtr current = toVisit.back();
        toVisit.pop_back();

        const PROJECTGEN::TModuleDependencyNodePtrMap* links[] = { &current->GetDependencies(), &current->GetLinkerDependencies(),
                                                                   &current->GetRuntimeDependencies(), &current->GetLogicalDependencies() };
        UInt32 linkTypes = includeLogicalDependencies ? 4 : 3;
        for ( UInt32 t=0; t<linkTypes; ++t )
        {
            for ( PROJECTGEN::TModuleDependencyNodePtrMap::const_iterator d = links[ t ]->begin(); d != links[ t ]->end(); ++d )
            {
                const PROJECTGEN::CModuleDependencyNodePtr& dependency = (*d).second;
                if ( dependency.IsNULL() )
                    continue;
                closure.insert( dependency->GetModule() );
                if ( visited.insert( dependency ).second )
                    toVisit.push_back( dependency );
            }
        }
    }
}

/*-------------------------------------------------------------------------*/

/**
 *  Reference for the module set of a tag target: the tagged modules which apply to their platform plus everything
 *  they depend on
 */
static void
ReferenceTaggedModuleSet( const PROJECTGEN::CProjectInfo& projectInfo     ,
                          const CORE::CString& tag                        ,
                          const CORE::CString& platform                   ,
                          bool onlyApplicableModules                      ,
                          PROJECTGEN::TModuleInfoEntryPtrSet& moduleSet   )
{GUCEF_TRACE;

    CORE::CStringSet taggedModuleNames;
    projectInfo.GetTaggedModulesByName( tag, taggedModuleNames, platform );
    for ( CORE::CStringSet::const_iterator n = taggedModuleNames.begin(); n != taggedModuleNames.end(); ++n )
    {
        PROJECTGEN::CModuleDependencyNodePtr chain;
        if ( !projectInfo.TryGetModuleDependencyChain( chain, (*n), platform, true ) || chain.IsNULL() )
            continue;
        PROJECTGEN::CModuleInfoEntryPtr module = chain->GetModule();
        if ( module.IsNULL() )
            continue;
        if ( onlyApplicableModules && !module->IsApplicableForPlatform( chain->GetTargetPlatform() ) )
            continue;
        moduleSet.insert( module );
        ReferenceDependencyClosure( chain, false, moduleSet );
    }
}

/*-------------------------------------------------------------------------*/

static PROJECTGEN::TModuleInfoEntryPtrSet
ModuleSetDifference( const PROJECTGEN::TModuleInfoEntryPtrSet& modules ,
                     const PROJECTGEN::TModuleInfoEntryPtrSet& minus   )
{GUCEF_TRACE;

    PROJECTGEN::TModuleInfoEntryPtrSet difference;
    for ( PROJECTGEN::TModuleInfoEntryPtrSet::const_iterator m = modules.begin(); m != modules.end(); ++m )
        if ( minus.find( (*m) ) == minus.end() )
            difference.insert( (*m) );
    return difference;
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

    // The dependencies of dependencies were gathered by following every path through the dependency graph instead of
    // visiting every module once, which grows exponentially with the depth of the graph: generating the build files of
    // a few hundred modules took minutes, spent determining the targets per platform
    GUCEF_TESTFW_TESTCASE( "Generator: dependency closures and per platform targets of a deep dependency graph are quick and complete" )
    try
    {
        ASSERT_TRUE( EnsureXmlCodecIsAvailable() );

        CORE::CString rootDir = CORE::CombinePath( CORE::TempDir(), "ProjectGen_TestApp_DependencyClosure" );
        CORE::DeleteDir( rootDir, true );
        ASSERT_TRUE( CreateDependencyClosureTestProject( rootDir ) );

        PROJECTGEN::CProjectInfoPtr projectInfo = PROJECTGEN::CProjectInfo::CreateSharedObj();
        CORE::CDataNode config( "ProjectGenerator" );
        CORE::CDataNode* platformDefs = config.AddChild( "PlatformDefinitions" );
        PROJECTGEN::TStringSet platforms;
        platforms.insert( PROJECTGEN::KnownPlatforms::AllPlatforms );
        for ( UInt32 p=0; p<g_closureTestPlatformCount; ++p )
        {
            CORE::CDataNode* platformDef = platformDefs->AddChild( g_closureTestPlatforms[ p ] );
            platformDef->SetAttribute( "PlatformDirs", g_closureTestPlatforms[ p ] );
            platforms.insert( g_closureTestPlatforms[ p ] );
        }
        projectInfo->ApplyConfig( config );

        // Gathering the information includes building the dependency chains and determining all targets
        PROJECTGEN::CDirCrawlingProjectInfoGatherer infoGatherer;
        CORE::CValueList params;
        PROJECTGEN::TStringVector rootDirs;
        rootDirs.push_back( rootDir );
        CORE::CStopwatch gatherStopwatch;
        gatherStopwatch.Start();
        ASSERT_TRUE( infoGatherer.GatherInfo( rootDirs, projectInfo, params ) );
        UInt64 gatherMicroseconds = gatherStopwatch.GetElapsedMicroseconds();
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "DependencyClosure test: gathering the project info took " << gatherMicroseconds << " microseconds" );
        ASSERT_TRUE( gatherMicroseconds < g_closureTestMaxMicroseconds );

        PROJECTGEN::TStringSet platformsUsed;
        projectInfo->GetAllEnabledPlatformsUsed( platformsUsed, true );
        ASSERT_TRUE( platformsUsed == platforms );

        PROJECTGEN::CProjectTargetInfoBundlePtr targets = PROJECTGEN::CProjectTargetInfoBundle::CreateSharedObj();
        CORE::CStopwatch targetsStopwatch;
        targetsStopwatch.Start();
        ASSERT_TRUE( projectInfo->GetAllTargets( *targets, true, true, platforms ) );
        UInt64 targetsMicroseconds = targetsStopwatch.GetElapsedMicroseconds();
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "DependencyClosure test: determining the targets for " << platforms.size() << " platforms took " << targetsMicroseconds << " microseconds" );
        ASSERT_TRUE( targetsMicroseconds < g_closureTestMaxMicroseconds );

        // The gathering functions of every dependency chain node give the same modules as the reference
        UInt32 chainsChecked = 0;
        for ( PROJECTGEN::TStringSet::const_iterator p = platforms.begin(); p != platforms.end(); ++p )
        {
            const CORE::CString& platform = (*p);
            for ( PROJECTGEN::TStringToModuleInfoEntryPtrMap::const_iterator m = projectInfo->modules.begin(); m != projectInfo->modules.end(); ++m )
            {
                PROJECTGEN::CModuleDependencyNodePtr chain;
                if ( !projectInfo->TryGetModuleDependencyChain( chain, (*m).first, platform, true ) || chain.IsNULL() )
                    continue;
                ++chainsChecked;

                PROJECTGEN::TModuleInfoEntryPtrSet expected;
                ReferenceDependencyClosure( chain, false, expected );

                PROJECTGEN::TModuleInfoEntryPtrSet gathered;
                ASSERT_TRUE( chain->GatherDependencyModules( gathered, true ) );
                ASSERT_TRUE( chain->GatherLinkerDependencyModules( gathered, true ) );
                ASSERT_TRUE( chain->GatherRuntimeDependencyModules( gathered, true ) );
                ASSERT_TRUE( gathered == expected );

                PROJECTGEN::TModuleInfoEntryPtrSet dependenciesOfDependencies;
                ASSERT_TRUE( chain->GatherDependenciesOfDependencies( dependenciesOfDependencies, false ) );
                ASSERT_TRUE( dependenciesOfDependencies == expected );

                PROJECTGEN::TModuleInfoEntryPtrSet expectedWithLogical;
                ReferenceDependencyClosure( chain, true, expectedWithLogical );
                PROJECTGEN::TModuleInfoEntryPtrSet withLogical;
                ASSERT_TRUE( chain->GatherDependenciesOfDependencies( withLogical, true ) );
                ASSERT_TRUE( withLogical == expectedWithLogical );
            }
        }
        ASSERT_TRUE( chainsChecked >= platforms.size() * g_closureTestLayers * g_closureTestLayerWidth );

        // The tag targets: the 'all' platform target has the tagged modules and their dependencies, a specific platform
        // target has what that platform has on top of the 'all' platform target
        PROJECTGEN::TStringSet tags;
        projectInfo->GetAllTagsUsed( tags );
        ASSERT_TRUE( tags.find( "closuretest_apps" ) != tags.end() );
        ASSERT_TRUE( tags.find( "closuretest_mid" ) != tags.end() );
        for ( PROJECTGEN::TStringSet::const_iterator t = tags.begin(); t != tags.end(); ++t )
        {
            const CORE::CString& tag = (*t);
            CORE::CString targetName = projectInfo->GetProjectName() + "_tag_" + tag;

            PROJECTGEN::TModuleInfoEntryPtrSet expectedAll;
            ReferenceTaggedModuleSet( *projectInfo, tag, PROJECTGEN::KnownPlatforms::AllPlatforms, false, expectedAll );
            PROJECTGEN::CProjectTargetInfoPtr allTarget = targets->GetPlatformProjectTarget( targetName, PROJECTGEN::KnownPlatforms::AllPlatforms );
            ASSERT_TRUE( !allTarget.IsNULL() );
            if ( allTarget.IsNULL() )
                continue;
            ASSERT_TRUE( allTarget->modules == expectedAll );
            ASSERT_FALSE( expectedAll.empty() );

            PROJECTGEN::TModuleInfoEntryPtrSet baseline;
            ReferenceTaggedModuleSet( *projectInfo, tag, PROJECTGEN::KnownPlatforms::AllPlatforms, true, baseline );
            for ( PROJECTGEN::TStringSet::const_iterator p = platforms.begin(); p != platforms.end(); ++p )
            {
                const CORE::CString& platform = (*p);
                if ( platform == PROJECTGEN::KnownPlatforms::AllPlatforms )
                    continue;

                PROJECTGEN::TModuleInfoEntryPtrSet platformSet;
                ReferenceTaggedModuleSet( *projectInfo, tag, platform, true, platformSet );
                PROJECTGEN::TModuleInfoEntryPtrSet expectedDelta = ModuleSetDifference( platformSet, baseline );

                PROJECTGEN::CProjectTargetInfoPtr platformTarget = targets->GetPlatformProjectTarget( targetName, platform );
                ASSERT_TRUE( !platformTarget.IsNULL() );
                if ( platformTarget.IsNULL() )
                    continue;
                ASSERT_TRUE( platformTarget->modules == expectedDelta );

                // only linux64 adds modules, for the tags whose modules depend on the first module of a layer
                if ( platform == "linux64" && tag == "closuretest_apps" )
                    ASSERT_TRUE( expectedDelta.size() == g_closureTestLayers-1 );
                if ( platform != "linux64" )
                    ASSERT_TRUE( expectedDelta.empty() );
            }
        }

        // The executable targets: the 'all' platform target has the executable and its dependencies, a specific
        // platform target has the dependencies which that platform has on top of those
        PROJECTGEN::TModuleInfoEntryPtrSet executables;
        projectInfo->GetExecutables( executables, PROJECTGEN::KnownPlatforms::AllPlatforms );
        ASSERT_TRUE( executables.size() == g_closureTestLayerWidth );
        for ( PROJECTGEN::TModuleInfoEntryPtrSet::const_iterator e = executables.begin(); e != executables.end(); ++e )
        {
            const PROJECTGEN::CModuleInfoEntryPtr& executable = (*e);
            const CORE::CString& executableName = executable->GetConsensusName();
            CORE::CString targetName = projectInfo->GetProjectName() + "_exe_" + executableName;

            PROJECTGEN::CModuleDependencyNodePtr allChain;
            ASSERT_TRUE( projectInfo->TryGetModuleDependencyChain( allChain, executableName, PROJECTGEN::KnownPlatforms::AllPlatforms, true ) && !allChain.IsNULL() );
            if ( allChain.IsNULL() )
                continue;
            PROJECTGEN::TModuleInfoEntryPtrSet allDependencies;
            ReferenceDependencyClosure( allChain, false, allDependencies );
            PROJECTGEN::TModuleInfoEntryPtrSet expectedAll = allDependencies;
            expectedAll.insert( executable );

            PROJECTGEN::CProjectTargetInfoPtr allTarget = targets->GetPlatformProjectTarget( targetName, PROJECTGEN::KnownPlatforms::AllPlatforms );
            ASSERT_TRUE( !allTarget.IsNULL() );
            if ( allTarget.IsNULL() )
                continue;
            ASSERT_TRUE( allTarget->modules == expectedAll );
            ASSERT_TRUE( allTarget->modules.size() > ( g_closureTestLayers-1 ) * 3 );

            for ( PROJECTGEN::TStringSet::const_iterator p = platforms.begin(); p != platforms.end(); ++p )
            {
                const CORE::CString& platform = (*p);
                if ( platform == PROJECTGEN::KnownPlatforms::AllPlatforms )
                    continue;

                PROJECTGEN::CModuleDependencyNodePtr platformChain;
                ASSERT_TRUE( projectInfo->TryGetModuleDependencyChain( platformChain, executableName, platform, true ) && !platformChain.IsNULL() );
                if ( platformChain.IsNULL() )
                    continue;
                PROJECTGEN::TModuleInfoEntryPtrSet platformDependencies;
                ReferenceDependencyClosure( platformChain, false, platformDependencies );
                PROJECTGEN::TModuleInfoEntryPtrSet expectedDelta = ModuleSetDifference( platformDependencies, allDependencies );

                PROJECTGEN::CProjectTargetInfoPtr platformTarget = targets->GetPlatformProjectTarget( targetName, platform );
                ASSERT_TRUE( !platformTarget.IsNULL() );
                if ( platformTarget.IsNULL() )
                    continue;
                ASSERT_TRUE( platformTarget->modules == expectedDelta );
                if ( platform != "linux64" )
                    ASSERT_TRUE( expectedDelta.empty() );
                else
                    ASSERT_FALSE( expectedDelta.empty() );
            }
        }
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
