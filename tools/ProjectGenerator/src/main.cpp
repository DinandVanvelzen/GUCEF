/*
 *  ProjectGenerator: Tool to generate module/project files
 *  Copyright (C) 2002 - 2014.  Dinand Vanvelzen
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

#include <stdio.h>

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_DVCPPOSWRAP_H
#include "DVCPPOSWRAP.h"
#define GUCEF_CORE_DVCPPOSWRAP_H
#endif /* GUCEF_CORE_DVCPPOSWRAP_H ? */

#ifndef GUCEF_CORE_CFILEACCESS_H
#include "CFileAccess.h"
#define GUCEF_CORE_CFILEACCESS_H
#endif /* GUCEF_CORE_CFILEACCESS_H ? */

#ifndef GUCEF_CORE_CONFIGSTORE_H
#include "CConfigStore.h"
#define GUCEF_CORE_CONFIGSTORE_H
#endif /* GUCEF_CORE_CONFIGSTORE_H ? */

#ifndef GUCEF_PROJECTGEN_DATATYPES_H
#include "gucefProjectGen_DataTypes.h"
#define GUCEF_PROJECTGEN_DATATYPES_H
#endif /* GUCEF_PROJECTGEN_DATATYPES_H ? */

#ifndef GUCEF_PROJECTGEN_CIPROJECTGENERATOR_H
#include "gucefProjectGen_CIProjectGenerator.h"
#define GUCEF_PROJECTGEN_CIPROJECTGENERATOR_H
#endif /* GUCEF_PROJECTGEN_CIPROJECTGENERATOR_H ? */

#ifndef GUCEF_PROJECTGEN_CIPROJECTINFOGATHERER_H
#include "gucefProjectGen_CIProjectInfoGatherer.h"
#define GUCEF_PROJECTGEN_CIPROJECTINFOGATHERER_H
#endif /* GUCEF_PROJECTGEN_CIPROJECTINFOGATHERER_H ? */

#ifndef GUCEF_PROJECTGEN_CDIRCRAWLINGPROJECTINFOGATHERER_H
#include "gucefProjectGen_CDirCrawlingProjectInfoGatherer.h"
#define GUCEF_PROJECTGEN_CDIRCRAWLINGPROJECTINFOGATHERER_H
#endif /* GUCEF_PROJECTGEN_CDIRCRAWLINGPROJECTINFOGATHERER_H ? */

#ifndef GUCEF_PROJECTGEN_CXMLPROJECTGENERATOR_H
#include "gucefProjectGen_CXmlProjectGenerator.h"
#define GUCEF_PROJECTGEN_CXMLPROJECTGENERATOR_H
#endif /* GUCEF_PROJECTGEN_CXMLPROJECTGENERATOR_H ? */

#ifndef GUCEF_PROJECTGEN_CCMAKEPROJECTGENERATOR_H
#include "gucefProjectGen_CCMakeProjectGenerator.h"
#define GUCEF_PROJECTGEN_CCMAKEPROJECTGENERATOR_H
#endif /* GUCEF_PROJECTGEN_CCMAKEPROJECTGENERATOR_H ? */

#ifndef GUCEF_PROJECTGEN_CANDROIDMAKEFILEGENERATOR_H
#include "gucefProjectGen_CAndroidMakefileGenerator.h"
#define GUCEF_PROJECTGEN_CANDROIDMAKEFILEGENERATOR_H
#endif /* GUCEF_PROJECTGEN_CANDROIDMAKEFILEGENERATOR_H ? */

#ifndef GUCEF_PROJECTGEN_CPREMAKE4PROJECTGENERATOR_H
#include "gucefProjectGen_CPremake4ProjectGenerator.h"
#define GUCEF_PROJECTGEN_CPREMAKE4PROJECTGENERATOR_H
#endif /* GUCEF_PROJECTGEN_CPREMAKE4PROJECTGENERATOR_H ? */

#ifndef GUCEF_PROJECTGEN_CPREMAKE5PROJECTGENERATOR_H
#include "gucefProjectGen_CPremake5ProjectGenerator.h"
#define GUCEF_PROJECTGEN_CPREMAKE5PROJECTGENERATOR_H
#endif /* GUCEF_PROJECTGEN_CPREMAKE5PROJECTGENERATOR_H ? */

#ifndef GUCEF_PROJECTGEN_CPROJECTGENGLOBAL_H
#include "gucefProjectGen_CProjectGenGlobal.h"
#define GUCEF_PROJECTGEN_CPROJECTGENGLOBAL_H
#endif /* GUCEF_PROJECTGEN_CPROJECTGENGLOBAL_H ? */

#ifndef GUCEF_PROJECTGEN_CPROJECTPREPROCESSORMANAGER_H
#include "gucefProjectGen_CProjectPreprocessorManager.h"
#define GUCEF_PROJECTGEN_CPROJECTPREPROCESSORMANAGER_H
#endif /* GUCEF_PROJECTGEN_CPROJECTPREPROCESSORMANAGER_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;
using namespace GUCEF::PROJECTGEN;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

bool
LoadConfig( const CORE::CString& configPath ,
            CORE::CValueList& keyValueList  ,
            CORE::CDataNode* loadedConfig   )
{GUCEF_TRACE;

    #ifdef GUCEF_DEBUG_MODE
    const CORE::CString configFile = "ProjectGenerator_d.ini";
    #else
    const CORE::CString configFile = "ProjectGenerator.ini";
    #endif

    CORE::CString configFilePath;
    bool foundViaParam = false;
    if ( !configPath.IsNULLOrEmpty() )
    {
        GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "Checking for config file @ " + configPath );
        foundViaParam = CORE::FileExists( configPath );
        configFilePath = configPath;
    }

    if ( !foundViaParam )
    {
        configFilePath = CORE::CombinePath( "$CURWORKDIR$", configFile );
        configFilePath = CORE::RelativePath( configFilePath );

        GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "Checking for config file @ " + configFilePath );
        if ( !CORE::FileExists( configFilePath ) )
        {
            configFilePath = CORE::CombinePath( "$MODULEDIR$", configFile );
            configFilePath = CORE::RelativePath( configFilePath );

            GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "Checking for config file @ " + configFilePath );
            if ( !FileExists( configFilePath ) )
            {
                GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "Unable to locate any config file, will rely on params" );
                return false;
            }
        }
    }
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Located config file @ " + configFilePath );

    keyValueList.SetConfigNamespace( "Bootstrap/Main/AppArgs" );
    keyValueList.SetUseGlobalConfig( true );
    keyValueList.SetAllowDuplicates( false );
    keyValueList.SetAllowMultipleValues( true );

    CORE::CConfigStore& configStore = CORE::CCoreGlobal::Instance()->GetConfigStore();
    configStore.SetConfigFile( configFilePath );
    return configStore.LoadConfig( loadedConfig );
}

/*-------------------------------------------------------------------------*/

void
ParseParams( const int argc                 ,
             char* argv[]                   ,
             CORE::CValueList& keyValueList )
{GUCEF_TRACE;

    GUCEF::CORE::CString argString;
    if ( argc > 0 )
    {
        argString = *argv;

        // Combine the argument strings back into a single string because we don't want to use
        // a space as the seperator
        for ( int i=1; i<argc; ++i )
        {
            argString += ' ' + CORE::CString( argv[ i ] );
        }

        // Parse the param list based on the ' symbol
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Application parameters: " + argString );
        keyValueList.SetMultiple( argString, '*' );
    }
}

/*---------------------------------------------------------------------------*/

/*
 *      Application entry point
 */
GUCEF_OSMAIN_BEGIN
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "This tool was compiled on: " __DATE__ " @ " __TIME__ );

    CORE::CCoreGlobal::Instance();
    PROJECTGEN::CProjectGenGlobal::Instance();

    // Check for config param first
    CORE::CValueList keyValueList;
    ParseParams( argc, argv, keyValueList );
    CORE::CString configPathParam = keyValueList.GetValueAlways( "ConfigPath" );
    keyValueList.Clear();
    
    // Load settings from a config file (if any) and then override with params (if any)
    CORE::CDataNode loadedConfig;
    LoadConfig( configPathParam, keyValueList, &loadedConfig );
    ParseParams( argc, argv, keyValueList );

    CORE::Int32 minLogLevel = CORE::LOGLEVEL_BELOW_NORMAL;
    CORE::CString valueStr = keyValueList.GetValueAlways( "MinimalLogLevel" );
    if ( !valueStr.IsNULLOrEmpty() )
    {
        minLogLevel = CORE::StringToInt32( valueStr );
        CORE::CCoreGlobal::Instance()->GetLogManager().SetMinLogLevel( minLogLevel );
    }

    // Support overriding environment variables from a file.
    // This can be important for build environments which rely on such variables
    CORE::CString envOverrideFile = keyValueList.GetValueAlways( "envOverridesFile" );
    if ( !envOverrideFile.IsNULLOrEmpty() )
    {
        CORE::CString fileContent;
        if ( CORE::LoadTextFileAsString( envOverrideFile, fileContent, true, "\n" ) )
        {
            CORE::SetEnvOverrides( fileContent );
        }
    }

    CORE::CString outputDir = CORE::RelativePath( keyValueList.GetValueAlways( "outputDir" ) );
    if ( outputDir.IsNULLOrEmpty() )
    {
        outputDir = CORE::RelativePath( "$CURWORKDIR$" );
    }
    CORE::CreateDirs( outputDir );

    CORE::CString logFilename = CORE::CombinePath( outputDir, "ProjectGenerator_Log.txt" );

    keyValueList.Set( "logfile", logFilename );

    CORE::CFileAccess logFileAccess( logFilename, "w" );
    CORE::CStdLogger logger( logFileAccess );
    CORE::CCoreGlobal::Instance()->GetLogManager().AddLogger( &logger );

    CORE::CPlatformNativeConsoleLogger console;
    if ( GUCEF_APP_TYPE == GUCEF_APP_TYPE_CONSOLE )
        CORE::CCoreGlobal::Instance()->GetLogManager().AddLogger( console.GetLogger() );

    CORE::CCoreGlobal::Instance()->GetLogManager().FlushBootstrapLogEntriesToLogs();
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Flushed to log @ " + logFilename );

    TStringVector rootDirs;
    try
    {
        rootDirs = keyValueList.GetValueVector( "rootDir" );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Number of rootDir arguments passed from command line: " + CORE::UInt32ToString( rootDirs.size() ) );
    }
    catch ( CORE::CValueList::EUnknownKey& )
    {
        rootDirs.push_back( CORE::RelativePath( "$CURWORKDIR$" ) );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Using current working directory since no rootDir arguments where passed from the command line" );
    }

    bool addToolCompileTimeToOutput = CORE::StringToBool( keyValueList.GetValueAlways( "addToolCompileTimeToOutput" ) );

    // Get the generators to use
    TStringVector generatorList = keyValueList.GetValueAlways( "generators" ).ParseElements( ';', false );
    if ( generatorList.size() == 0  )
    {
        // No specific generators where specified, defaulting...
        generatorList.push_back( "xml" );
    }

    TProjectInfo projectInfo;
    ApplyConfigToProject( loadedConfig, projectInfo );

    // Set any global dir excludes that where passed as cmd parameters
    projectInfo.globalDirExcludeList = keyValueList.GetValueAlways( "dirsToIgnore" ).ParseElements( ';', false );
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "There are " + CORE::UInt32ToString( projectInfo.globalDirExcludeList.size() ) + " dirs in the global dir ignore list" );

    projectInfo.projectName = keyValueList.GetValueAlways( "projectName" );

    bool useProjectInfoCache = CORE::StringToBool( keyValueList.GetValueAlways( "useProjectInfoCache" ) );
    bool projectInfoLoadedFromCache = false;
    if ( useProjectInfoCache )
    {
        CORE::CString cachedInfoPath = CORE::CombinePath( CORE::RelativePath( outputDir ), "Project.xml" );
        projectInfoLoadedFromCache = DeserializeProjectInfo( projectInfo, cachedInfoPath );
    }

    if ( !projectInfoLoadedFromCache )
    {
        // Use an info gatherer to get all the project information for us
        CDirCrawlingProjectInfoGatherer infoGatherer;
        infoGatherer.GatherInfo( rootDirs     ,
                                 projectInfo  ,
                                 keyValueList );
    }

    // Before we hand the data we collected and generated to the generator(s) for the desired output we will check
    // for preprocessors which can be executed before any output generator. These apply changes to the project data
    // that will apply to all output generators
    const CProjectPreprocessorManager::TProjectPreprocessorsList& projectPreProcessors = CProjectGenGlobal::Instance()->GetProjectPreprocessorManager().GetProjectPreprocessors();
    CProjectPreprocessorManager::TProjectPreprocessorsList::const_iterator n = projectPreProcessors.begin();
    while ( n != projectPreProcessors.end() )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Invoking project preprocessor" );
        (*n)->ProccessProjects( projectInfo  ,
                                outputDir    ,
                                keyValueList );
        ++n;
    }

    // Now we output the project info using all generators specified
    TStringVector::iterator i = generatorList.begin();
    while ( i != generatorList.end() )
    {
        if ( (*i).Lowercase() == "xml" )
        {
            CXmlProjectGenerator xmlGenerator;
            xmlGenerator.GenerateProject( projectInfo                ,
                                          outputDir                  ,
                                          addToolCompileTimeToOutput ,
                                          keyValueList               );
        }
        else
        if ( (*i).Lowercase() == "androidmake" )
        {
            CAndroidMakefileGenerator androidMakefileGenerator;
            androidMakefileGenerator.GenerateProject( projectInfo                ,
                                                      outputDir                  ,
                                                      addToolCompileTimeToOutput ,
                                                      keyValueList               );
        }
        else
        if ( (*i).Lowercase() == "cmake" )
        {
            CCMakeProjectGenerator cmakeGenerator;
            cmakeGenerator.GenerateProject( projectInfo                ,
                                            outputDir                  ,
                                            addToolCompileTimeToOutput ,
                                            keyValueList               );
        }
        else
        if ( (*i).Lowercase() == "premake4" )
        {
            CPremake4ProjectGenerator premake4Generator;
            premake4Generator.GenerateProject( projectInfo                ,
                                               outputDir                  ,
                                               addToolCompileTimeToOutput ,
                                               keyValueList               );
        }
        else
        if ( (*i).Lowercase() == "premake5" )
        {
            CPremake5ProjectGenerator premake5Generator;
            premake5Generator.GenerateProject( projectInfo                ,
                                               outputDir                  ,
                                               addToolCompileTimeToOutput ,
                                               keyValueList               );
        }
        ++i;
    }

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Wrote log file to: " + logFilename );

    CORE::CCoreGlobal::Instance()->GetLogManager().ClearLoggers();

    return 0;
}
GUCEF_OSMAIN_END

/*-------------------------------------------------------------------------//
//                                                                         //
//      Info & Changes                                                     //
//                                                                         //
//-------------------------------------------------------------------------//

- 27-11-2004 :
        - Dinand: Initial implementation

---------------------------------------------------------------------------*/
