/*
 *  ProjectGen_TestApp: GUCEF test application for the ProjectGen module
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
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

#include "TestIncludeDirInstruction.h"

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

/**
 *  The processing instructions are XML: make sure the XML codec plugin is available
 */
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
 *  Creates a small project with 2 modules:
 *      TestLib: a library with headers in 'include' and in 'include/internal',
 *               the latter has instructions stating it is not to be used as an include search path
 *      TestApp: an executable which depends on TestLib
 */
static bool
CreateTestProject( const CORE::CString& rootDir )
{GUCEF_TRACE;

    CORE::CString libDir = CORE::CombinePath( rootDir, "TestLib" );
    CORE::CString appDir = CORE::CombinePath( rootDir, "TestApp" );

    return WriteTestFile( libDir, "ModuleInfo.xml",
                          "<ModuleInfoEntry>\n"
                          "    <Module Name=\"TestLib\" Type=\"SharedLibrary\" Platform=\"All\" >\n"
                          "    </Module>\n"
                          "    <Module Platform=\"linux64\" >\n"
                          "        <Preprocessor>\n"
                          "            <Define String=\"TESTLIB_LINUX\" />\n"
                          "        </Preprocessor>\n"
                          "    </Module>\n"
                          "    <Module Platform=\"win64\" >\n"
                          "        <Preprocessor>\n"
                          "            <Define String=\"TESTLIB_WINDOWS\" />\n"
                          "        </Preprocessor>\n"
                          "    </Module>\n"
                          "</ModuleInfoEntry>\n" ) &&
           WriteTestFile( CORE::CombinePath( libDir, "include" ), "TestLib.h", "/* public header */\n" ) &&
           WriteTestFile( CORE::CombinePath( libDir, "include/internal" ), "endian.h", "/* named like a system header */\n" ) &&
           WriteTestFile( CORE::CombinePath( libDir, "include/internal" ), "ProjectGenInstructions.xml",
                          "<PROJECTGENERATOR>\n"
                          "    <EXCLUDES PLATFORM=\"ALL\">\n"
                          "        <ITEM NAME=\".\" TYPE=\"INCLUDEDIR\" />\n"
                          "    </EXCLUDES>\n"
                          "</PROJECTGENERATOR>\n" ) &&
           WriteTestFile( CORE::CombinePath( libDir, "src" ), "TestLib.cpp", "/* source */\n" ) &&
           WriteTestFile( appDir, "ModuleInfo.xml",
                          "<ModuleInfoEntry>\n"
                          "    <Module Name=\"TestApp\" Type=\"Executable\" Platform=\"All\" >\n"
                          "        <Dependencies>\n"
                          "            <Dependency Name=\"TestLib\" />\n"
                          "        </Dependencies>\n"
                          "        <Linker>\n"
                          "            <Dependency Name=\"TestLib\" />\n"
                          "        </Linker>\n"
                          "    </Module>\n"
                          "</ModuleInfoEntry>\n" ) &&
           WriteTestFile( CORE::CombinePath( appDir, "src" ), "main.cpp", "/* source */\n" ) &&
           WriteTestFile( appDir, "ProjectGenInstructions.xml",
                          "<PROJECTGENERATOR>\n"
                          "    <PLATFORMS>\n"
                          "        <PLATFORM NAME=\"testos\" ALIASES=\"testlike\" PLATFORMDIRS=\"testosdir\" />\n"
                          "    </PLATFORMS>\n"
                          "</PROJECTGENERATOR>\n" );
}

/*-------------------------------------------------------------------------*/

static bool
IsInternalDir( const CORE::CString& path )
{GUCEF_TRACE;

    return path.ReplaceChar( '\\', '/' ).HasSubstr( "include/internal" ) >= 0;
}

/*-------------------------------------------------------------------------*/

void
PerformIncludeDirInstructionTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "IncludeDirInstruction" );

    GUCEF_TESTFW_TESTCASE( "INCLUDEDIR: dir keeps its headers but is not an include search path" )
    try
    {
        ASSERT_TRUE( EnsureXmlCodecIsAvailable() );

        // Note: a fixed location is used, each run rewrites the same files
        CORE::CString rootDir = CORE::CombinePath( CORE::TempDir(), "ProjectGen_TestApp_IncludeDirTest" );
        ASSERT_TRUE( CreateTestProject( rootDir ) );

        PROJECTGEN::CProjectInfoPtr projectInfo = PROJECTGEN::CProjectInfo::CreateSharedObj();

        // Same platform definitions as ProjectGenerator gets via its config, at least 2 platforms are required
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

        // The library keeps the headers of the excluded dir as part of the module
        PROJECTGEN::CModuleInfoEntryPtr libEntry = projectInfo->GetModuleInfoEntry( "TestLib" );
        ASSERT_TRUE( !libEntry.IsNULL() );
        PROJECTGEN::CModuleInfoPtr libInfo;
        if ( !libEntry.IsNULL() )
            libInfo = libEntry->FindModuleInfoForPlatform( PROJECTGEN::KnownPlatforms::AllPlatforms );
        ASSERT_TRUE( !libInfo.IsNULL() );

        // Note that the assertions do not end the test case, the remaining checks require the module information
        if ( !libInfo.IsNULL() )
        {

        CORE::CString internalDir;
        bool internalHeaderListed = false;
        PROJECTGEN::TStringSetMap::const_iterator i = libInfo->GetIncludeDirs().begin();
        while ( i != libInfo->GetIncludeDirs().end() )
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "TestLib include dir: \"" + (*i).first + "\" with " + CORE::ToString( (UInt32) (*i).second.size() ) + " headers, excluded from search paths: " +
                       CORE::BoolToString( libInfo->IsIncludeDirExcludedFromSearchPaths( (*i).first ) ) );
            if ( IsInternalDir( (*i).first ) )
            {
                internalDir = (*i).first;
                internalHeaderListed = (*i).second.find( "endian.h" ) != (*i).second.end();
            }
            ++i;
        }
        ASSERT_TRUE( !internalDir.IsNULLOrEmpty() );
        ASSERT_TRUE( internalHeaderListed );
        ASSERT_TRUE( libInfo->IsIncludeDirExcludedFromSearchPaths( internalDir ) );

        // ... but the dir is not one of the library's include search paths
        PROJECTGEN::TStringSet libSearchDirs;
        libEntry->GetIncludeDirsForPlatform( PROJECTGEN::KnownPlatforms::AllPlatforms, libSearchDirs, false, false );
        bool publicDirIsSearchPath = false;
        PROJECTGEN::TStringSet::const_iterator n = libSearchDirs.begin();
        while ( n != libSearchDirs.end() )
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "TestLib include search path: \"" + (*n) + "\"" );
            ASSERT_FALSE( IsInternalDir( (*n) ) );
            if ( (*n).ReplaceChar( '\\', '/' ) == "include" )
                publicDirIsSearchPath = true;
            ++n;
        }
        ASSERT_TRUE( publicDirIsSearchPath );

        // Modules that depend on the library do not get the excluded dir as an include search path either
        PROJECTGEN::CModuleInfoEntryPtr appEntry = projectInfo->GetModuleInfoEntry( "TestApp" );
        ASSERT_TRUE( !appEntry.IsNULL() );
        PROJECTGEN::TStringSet appSearchDirs;
        if ( !appEntry.IsNULL() )
            appEntry->GetIncludeDirsForPlatform( PROJECTGEN::KnownPlatforms::AllPlatforms, appSearchDirs, true, true );
        bool libPublicDirInherited = false;
        n = appSearchDirs.begin();
        while ( n != appSearchDirs.end() )
        {
            ASSERT_FALSE( IsInternalDir( (*n) ) );
            if ( (*n).ReplaceChar( '\\', '/' ).HasSubstr( "TestLib/include" ) >= 0 )
                libPublicDirInherited = true;
            ++n;
        }
        ASSERT_TRUE( libPublicDirInherited );

        // The exclusion survives the project information round trip used for the Project.xml cache
        CORE::CString projectXmlPath = CORE::CombinePath( rootDir, "Project.xml" );
        CORE::CDataNodeSerializableSettings settings;
        settings.levelOfDetail = CORE::CDataNodeSerializableSettings::DataNodeSerializableLod_MaximumDetails;
        ASSERT_TRUE( projectInfo->Serialize( projectXmlPath, settings ) );
        PROJECTGEN::CProjectInfoPtr restoredProjectInfo = PROJECTGEN::CProjectInfo::CreateSharedObj();
        restoredProjectInfo->ApplyConfig( config );
        ASSERT_TRUE( restoredProjectInfo->Deserialize( projectXmlPath ) );
        PROJECTGEN::CModuleInfoEntryPtr restoredEntry = restoredProjectInfo->GetModuleInfoEntry( "TestLib" );
        ASSERT_TRUE( !restoredEntry.IsNULL() );
        PROJECTGEN::CModuleInfoPtr restoredInfo;
        if ( !restoredEntry.IsNULL() )
            restoredInfo = restoredEntry->FindModuleInfoForPlatform( PROJECTGEN::KnownPlatforms::AllPlatforms );
        ASSERT_TRUE( !restoredInfo.IsNULL() && restoredInfo->IsIncludeDirExcludedFromSearchPaths( internalDir ) );
        }

    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    // Regression: parsing a PLATFORMS section in the processing instructions looped forever
    GUCEF_TESTFW_TESTCASE( "PLATFORMS: platforms defined in processing instructions are parsed" )
    try
    {
        ASSERT_TRUE( EnsureXmlCodecIsAvailable() );

        CORE::CString rootDir = CORE::CombinePath( CORE::TempDir(), "ProjectGen_TestApp_IncludeDirTest" );
        ASSERT_TRUE( CreateTestProject( rootDir ) );

        PROJECTGEN::CProjectInfoPtr projectInfo = PROJECTGEN::CProjectInfo::CreateSharedObj();
        PROJECTGEN::CDirCrawlingProjectInfoGatherer infoGatherer;
        CORE::CValueList params;
        PROJECTGEN::TStringVector rootDirs;
        rootDirs.push_back( rootDir );
        infoGatherer.GatherInfo( rootDirs, projectInfo, params );

        bool foundPlatformDefinition = false;
        PROJECTGEN::TDirProcessingInstructionsMap::const_iterator i = projectInfo->dirProcessingInstructions.begin();
        while ( i != projectInfo->dirProcessingInstructions.end() )
        {
            PROJECTGEN::TPlatformDefinitionMap::const_iterator p = (*i).second.platforms.find( "testos" );
            if ( p != (*i).second.platforms.end() )
            {
                foundPlatformDefinition = true;
                ASSERT_TRUE( (*p).second.name == "testos" );
                ASSERT_TRUE( (*p).second.aliases.find( "testlike" ) != (*p).second.aliases.end() );
                ASSERT_TRUE( (*p).second.platformDirs.find( "testosdir" ) != (*p).second.platformDirs.end() );
            }
            ++i;
        }
        ASSERT_TRUE( foundPlatformDefinition );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
