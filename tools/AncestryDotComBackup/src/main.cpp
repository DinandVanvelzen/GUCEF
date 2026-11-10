/*
 *  AncestryDotComBackup: lossless backup of an Ancestry.com family tree
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

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_DVCPPOSWRAP_H
#include "DVCPPOSWRAP.h"
#define GUCEF_CORE_DVCPPOSWRAP_H
#endif /* GUCEF_CORE_DVCPPOSWRAP_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_CROLLINGFILEACCESS_H
#include "gucefCORE_CRollingFileAccess.h"
#define GUCEF_CORE_CROLLINGFILEACCESS_H
#endif /* GUCEF_CORE_CROLLINGFILEACCESS_H ? */

#ifndef GUCEF_CORE_CONFIGSTORE_H
#include "CConfigStore.h"
#define GUCEF_CORE_CONFIGSTORE_H
#endif /* GUCEF_CORE_CONFIGSTORE_H ? */

#ifndef GUCEF_CORE_CPLUGINCONTROL_H
#include "CPluginControl.h"
#define GUCEF_CORE_CPLUGINCONTROL_H
#endif /* GUCEF_CORE_CPLUGINCONTROL_H ? */

#ifndef GUCEF_CORE_CPLATFORMNATIVECONSOLELOGGER_H
#include "gucefCORE_CPlatformNativeConsoleLogger.h"
#define GUCEF_CORE_CPLATFORMNATIVECONSOLELOGGER_H
#endif /* GUCEF_CORE_CPLATFORMNATIVECONSOLELOGGER_H ? */

#ifndef GUCEF_CORE_CGLOBALCONFIGVALUELIST_H
#include "gucefCORE_CGlobalConfigValueList.h"
#define GUCEF_CORE_CGLOBALCONFIGVALUELIST_H
#endif /* GUCEF_CORE_CGLOBALCONFIGVALUELIST_H ? */

#ifndef GUCEF_COMCORE_CCOMCOREGLOBAL_H
#include "gucefCOMCORE_CComCoreGlobal.h"
#define GUCEF_COMCORE_CCOMCOREGLOBAL_H
#endif /* GUCEF_COMCORE_CCOMCOREGLOBAL_H ? */

#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */

#ifndef GUCEF_WEB_CWEBGLOBAL_H
#include "gucefWEB_CWebGlobal.h"
#define GUCEF_WEB_CWEBGLOBAL_H
#endif /* GUCEF_WEB_CWEBGLOBAL_H ? */

#ifndef ANCESTRYBACKUP_CCOMMANDS_H
#include "AncestryBackup_CCommands.h"
#define ANCESTRYBACKUP_CCOMMANDS_H
#endif /* ANCESTRYBACKUP_CCOMMANDS_H ? */

#ifndef ANCESTRYBACKUP_CSETTINGS_H
#include "AncestryBackup_CSettings.h"
#define ANCESTRYBACKUP_CSETTINGS_H
#endif /* ANCESTRYBACKUP_CSETTINGS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

static CORE::CString
LookForConfigFile( const CORE::CString& configFile )
{GUCEF_TRACE;

    if ( CORE::FileExists( configFile ) )
        return configFile;

    static const char* const places[] = { "$CURWORKDIR$", "$MODULEDIR$", "$TEMPDIR$", GUCEF_NULL };
    for ( UInt32 i=0; places[ i ] != GUCEF_NULL; ++i )
    {
        const CORE::CString path = CORE::RelativePath( CORE::CombinePath( places[ i ], configFile ) );
        GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "Checking for config file @ " + path );
        if ( CORE::FileExists( path ) )
            return path;
    }
    return CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

/**
 *  Loads the ini file, which also loads the plugins listed in it, and returns the settings it holds
 */
static bool
LoadConfig( std::map< std::string, std::string >& iniSettings )
{GUCEF_TRACE;

    #ifdef GUCEF_DEBUG_MODE
    const CORE::CString configFile = "AncestryDotComBackup_d.ini";
    #else
    const CORE::CString configFile = "AncestryDotComBackup.ini";
    #endif

    CORE::CConfigStore& configStore = CORE::CCoreGlobal::Instance()->GetConfigStore();
    CORE::CString configFilePath = LookForConfigFile( configFile );
    if ( configFilePath.IsNULLOrEmpty() )
        configFilePath = LookForConfigFile( "AncestryDotComBackup.ini" );
    if ( configFilePath.IsNULLOrEmpty() )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "AncestryBackup: Unable to locate " + configFile + ", the JSON codec plugin will not be loaded and discovery will not work" );
        return true;
    }

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Located config file @ " + configFilePath );
    configStore.SetConfigFile( configFilePath );

    CORE::CGlobalConfigValueList globalCfg;
    globalCfg.SetConfigNamespace( "Main/AppArgs" );
    globalCfg.SetAllowDuplicates( false );
    globalCfg.SetAllowMultipleValues( false );

    if ( !configStore.LoadConfig() )
        return false;

    static const char* const names[] = { "dataDir", "treeId", "chromePath", "debugPort", "startUrl", "cookieDomain", "hostFilter",
                                         "discoverSeconds", "maxBodyMegabytes", "headless", "noWait", "chromeArgs", "attachToRunning", "chromeUserDataDir", "recordDna", "retryMissing", "dnaSampleId", "baselineGedcom", "gedcom", "exportDirs", "exportMaxAgeDays", "siteTreeId", "changesSince", "changesMaxPages", "requestDelayMs", "requestJitterPercent",
                                         "maxRequestsPerRun", "maxRequestsPerDay", "grampsFile", "grampsMediaPath", "grampsHomePerson", "researcherName", "researcherEmail", GUCEF_NULL };
    for ( UInt32 i=0; names[ i ] != GUCEF_NULL; ++i )
    {
        const CORE::CString value = globalCfg.GetValueAlways( names[ i ] );
        if ( !value.IsNULLOrEmpty() )
        {
            std::string name = names[ i ];
            for ( size_t c=0; c<name.size(); ++c )
                name[ c ] = (char) tolower( name[ c ] );
            iniSettings[ name ] = value.C_String();
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
GucefAppSignalHandler( int signal )
{GUCEF_TRACE;

    ANCESTRYBACKUP::CCommands::RequestStop();
}

/*-------------------------------------------------------------------------*/

GUCEF_OSMAIN_BEGIN
{GUCEF_TRACE;

    CORE::CCoreGlobal::Instance();
    COMCORE::CComCoreGlobal::Instance();
    VFS::CVfsGlobal::Instance();
    WEB::CWebGlobal::Instance();

    std::vector< std::string > args;
    for ( int i=1; i<argc; ++i )
        args.push_back( argv[ i ] );

    std::string command;
    std::map< std::string, std::string > options;
    CORE::CString problem;
    if ( !ANCESTRYBACKUP::CSettings::ParseCommandLine( args, command, options, problem ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_CRITICAL, "AncestryBackup: " + problem );
        return ANCESTRYBACKUP::EXIT_CODE_USAGE;
    }
    if ( command.empty() || options.find( "help" ) != options.end() )
    {
        printf( "%s", ANCESTRYBACKUP::CCommands::GetUsage() );
        return command.empty() ? ANCESTRYBACKUP::EXIT_CODE_USAGE : ANCESTRYBACKUP::EXIT_CODE_OK;
    }

    // the ini file first, then the command line on top of it
    std::map< std::string, std::string > settingValues;

    CORE::CPlatformNativeConsoleLogger console;
    if ( GUCEF_APP_TYPE == GUCEF_APP_TYPE_CONSOLE )
        CORE::CCoreGlobal::Instance()->GetLogManager().AddLogger( console.GetLogger() );

    if ( !LoadConfig( settingValues ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_CRITICAL, "AncestryBackup: Failed to load the config file" );
        return ANCESTRYBACKUP::EXIT_CODE_USAGE;
    }
    for ( std::map< std::string, std::string >::const_iterator i = options.begin(); i != options.end(); ++i )
        settingValues[ i->first ] = i->second;

    ANCESTRYBACKUP::CSettings settings;
    if ( !settings.Load( settingValues, problem ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_CRITICAL, "AncestryBackup: " + problem );
        return ANCESTRYBACKUP::EXIT_CODE_USAGE;
    }

    // a log file next to the data, so a long run can be looked at afterwards
    CORE::CString dataDir = CORE::RelativePath( settings.dataDir );
    CORE::CreateDirs( dataDir );
    CORE::CRollingFileAccess logFileAccess( CORE::CombinePath( dataDir, "AncestryDotComBackup_log.txt" ), "w" );
    logFileAccess.SetMaxRolloverFilesBeforeDeletion( 10 );
    CORE::CStdLogger logger( logFileAccess );
    CORE::CCoreGlobal::Instance()->GetLogManager().AddLogger( &logger );
    CORE::CCoreGlobal::Instance()->GetLogManager().FlushBootstrapLogEntriesToLogs();

    GUCEF_OSMAIN_SIGNAL_HANDLER( GucefAppSignalHandler );

    int result = ANCESTRYBACKUP::EXIT_CODE_USAGE;
    if ( command == "login" )
        result = ANCESTRYBACKUP::CCommands::Login( settings );
    else
    if ( command == "discover" )
        result = ANCESTRYBACKUP::CCommands::Discover( settings );
    else
    if ( command == "dna" )
        result = ANCESTRYBACKUP::CCommands::Dna( settings );
    else
    if ( command == "media" )
        result = ANCESTRYBACKUP::CCommands::Media( settings );
    else
    if ( command == "changes" )
        result = ANCESTRYBACKUP::CCommands::Changes( settings );
    else
    if ( command == "export" )
        result = ANCESTRYBACKUP::CCommands::Export( settings );
    else
    if ( command == "baseline" )
        result = ANCESTRYBACKUP::CCommands::Baseline( settings );
    else
    if ( command == "gramps" )
        result = ANCESTRYBACKUP::CCommands::Gramps( settings );
    else
    if ( command == "report" )
        result = ANCESTRYBACKUP::CCommands::Report( settings );
    else
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_CRITICAL, "AncestryBackup: Unknown command \"" + CORE::CString( command.c_str() ) + "\"" );
        printf( "%s", ANCESTRYBACKUP::CCommands::GetUsage() );
    }

    // plugins that registered themselves with the framework go before the framework does
    CORE::CCoreGlobal::Instance()->GetPluginControl().UnloadAll();

    CORE::CCoreGlobal::Instance()->GetLogManager().FlushLogs();
    return result;
}
GUCEF_OSMAIN_END

/*---------------------------------------------------------------------------*/
