/*
 *  pubsub2storage: service which transfers between pubsub and storage
 *
 *  Copyright (C) 1998 - 2020.  Dinand Vanvelzen
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

#ifndef GUCEF_CORE_CGUCEFAPPLICATION_H
#include "CGUCEFApplication.h"
#define GUCEF_CORE_CGUCEFAPPLICATION_H
#endif /* GUCEF_CORE_CGUCEFAPPLICATION_H ? */

#ifndef GUCEF_CORE_PROCESSARGUMENTS_H
#include "gucefCORE_ProcessArguments.h"
#define GUCEF_CORE_PROCESSARGUMENTS_H
#endif /* GUCEF_CORE_PROCESSARGUMENTS_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"        
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPOSWRAP_H
#include "DVCPPOSWRAP.h"
#define GUCEF_CORE_DVCPPOSWRAP_H
#endif /* GUCEF_CORE_DVCPPOSWRAP_H ? */

#ifndef GUCEF_CORE_CROLLINGFILEACCESS_H
#include "gucefCORE_CRollingFileAccess.h"
#define GUCEF_CORE_CROLLINGFILEACCESS_H
#endif /* GUCEF_CORE_CROLLINGFILEACCESS_H ? */

#ifndef GUCEF_CORE_CONFIGSTORE_H
#include "CConfigStore.h"
#define GUCEF_CORE_CONFIGSTORE_H
#endif /* GUCEF_CORE_CONFIGSTORE_H ? */

#ifndef GUCEF_CORE_CPLATFORMNATIVECONSOLEWINDOW_H
#include "gucefCORE_CPlatformNativeConsoleWindow.h"
#define GUCEF_CORE_CPLATFORMNATIVECONSOLEWINDOW_H
#endif /* GUCEF_CORE_CPLATFORMNATIVECONSOLEWINDOW_H ? */

#ifndef GUCEF_CORE_CGLOBALCONFIGVALUELIST_H
#include "gucefCORE_CGlobalConfigValueList.h"
#define GUCEF_CORE_CGLOBALCONFIGVALUELIST_H
#endif /* GUCEF_CORE_CGLOBALCONFIGVALUELIST_H ? */

#ifndef GUCEF_CORE_CTIMER_H
#include "CTimer.h"
#define GUCEF_CORE_CTIMER_H
#endif /* GUCEF_CORE_CTIMER_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_COMCORE_CCOMCOREGLOBAL_H
#include "gucefCOMCORE_CComCoreGlobal.h"
#define GUCEF_COMCORE_CCOMCOREGLOBAL_H
#endif /* GUCEF_COMCORE_CCOMCOREGLOBAL_H ? */

#ifndef GUCEF_COM_CCOMGLOBAL_H
#include "gucefCOM_CComGlobal.h"
#define GUCEF_COM_CCOMGLOBAL_H
#endif /* GUCEF_COM_CCOMGLOBAL_H ? */

#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */

#ifndef GUCEF_WEB_CWEBGLOBAL_H
#include "gucefWEB_CWebGlobal.h"
#define GUCEF_WEB_CWEBGLOBAL_H
#endif /* GUCEF_WEB_CWEBGLOBAL_H ? */

#ifndef GUCEF_PUBSUB_CPUBSUBGLOBAL_H
#include "gucefPUBSUB_CPubSubGlobal.h"
#define GUCEF_PUBSUB_CPUBSUBGLOBAL_H
#endif /* GUCEF_PUBSUB_CPUBSUBGLOBAL_H ? */

#ifndef GUCEF_PUBSUB_CPUBSUB2PUBSUB_H
#include "gucefPUBSUB_CPubSub2PubSub.h"
#define GUCEF_PUBSUB_CPUBSUB2PUBSUB_H
#endif /* GUCEF_PUBSUB_CPUBSUB2PUBSUB_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

CORE::CString
LookForConfigFile( const CORE::CString& configPath, const CORE::CString& configFile )
{GUCEF_TRACE;
    
    if ( !configPath.IsNULLOrEmpty() )
    {
        // Maybe we received a fully qualified path to the config file
        GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "Checking for config file @ " + configPath );
        if ( !CORE::FileExists( configPath ) )
        {
            // Maybe we only received a parent dir and should look for the default filename
            CORE::CString configFileDir = CORE::StripFilename( configPath );
            if ( !configFileDir.IsNULLOrEmpty() )
            {
                CORE::CString configFilePath = CORE::CombinePath( configFileDir, configFile );
                if ( CORE::FileExists( configFilePath ) )
                    return configFilePath; 
            }
        }
        else
        {
            return configPath;
        }
    }       
        
    // Maybe just checking for the file itself works
    GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "Checking for config file @ " + configFile );
    if ( !CORE::FileExists( configFile ) )
    {
        // Maybe its in the current working dir, usually the same thing as the check above
        CORE::CString configFilePath = CORE::CombinePath( "$CURWORKDIR$", configFile );
        configFilePath = CORE::RelativePath( configFilePath );

        GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "Checking for config file @ " + configFilePath );
        if ( !CORE::FileExists( configFilePath ) )
        {
            // Maybe its in the same dir as the program binaries
            configFilePath = CORE::CombinePath( "$MODULEDIR$", configFile );
            configFilePath = CORE::RelativePath( configFilePath );
            
            GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "Checking for config file @ " + configFilePath );
            if ( !FileExists( configFilePath ) )
            {            
                // Maybe its in the O/S temp folder due to write restrictions
                configFilePath = CORE::CombinePath( "$TEMPDIR$", configFile );
                configFilePath = CORE::RelativePath( configFilePath );

                GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "Checking for config file @ " + configFilePath );
                if ( !FileExists( configFilePath ) )
                {                                        
                    return CORE::CString::Empty;
                }
            }
        }

        return configFilePath; 
    }

    return configFile;
}

/*-------------------------------------------------------------------------*/

bool
LoadConfig( const CORE::CString& bootstrapConfigPath ,
            const CORE::CString& configPath          )
{GUCEF_TRACE;

    #ifdef GUCEF_DEBUG_MODE
    const CORE::CString bootstrapConfigFile = "pubsub2pubsub_bootstrap_d.ini";
    const CORE::CString configFile = "pubsub2pubsub_d.json";
    #else
    const CORE::CString bootstrapConfigFile = "pubsub2pubsub_bootstrap.ini";
    const CORE::CString configFile = "pubsub2pubsub.json";
    #endif

    CORE::CConfigStore& configStore = CORE::CCoreGlobal::Instance()->GetConfigStore();
    
    CORE::CString bootstrapConfigFilePath = LookForConfigFile( bootstrapConfigPath, bootstrapConfigFile );
    CORE::CString configFilePath = LookForConfigFile( configPath, configFile );

    if ( !bootstrapConfigFilePath.IsNULLOrEmpty() )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Located bootstrap config file @ " + bootstrapConfigFilePath );
        configStore.SetBootstrapConfigFile( bootstrapConfigFilePath );
    }
    if ( !configFilePath.IsNULLOrEmpty() )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Located config file @ " + configFilePath );
        configStore.SetConfigFile( configFilePath );
    }
    if ( bootstrapConfigFilePath.IsNULLOrEmpty() && configFilePath.IsNULLOrEmpty() )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "Unable to locate any config file, will rely on params only" );
    }

    bool loadSuccess = configStore.LoadConfig();
    return loadSuccess;
}

/*-------------------------------------------------------------------------*/

void
ParseParams( const int argc                 ,
             char* argv[]                   ,
             CORE::CValueList& keyValueList )
{GUCEF_TRACE;

    CORE::ParseApplicationArguments( argc, argv, keyValueList, '*' );
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Application parameters: " << CORE::ToString( keyValueList ) );
}

/*-------------------------------------------------------------------------*/

void
GucefAppSignalHandler( int signal )
{GUCEF_TRACE; 
    
    GUCEF_LOG( CORE::LOGLEVEL_IMPORTANT, "GucefAppSignalHandler: Received signal " << signal );
    
    // signal context: only the async-signal-safe stop request, the application stops at its next pulse
    ::GUCEF::CORE::CGUCEFApplication::RequestStopFromSignal( signal );
}

/*-------------------------------------------------------------------------*/

/*
 *      Application entry point
 */
//GUCEF_OSMAIN_BEGIN
GUCEF_OSSERVICEMAIN_BEGIN( "pubsub2pubsub" )
{GUCEF_TRACE;

    int returnValue = -100;
    try
    {

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "This service was compiled on: " + CORE::CDateTime::CompileDateTime( __DATE__, __TIME__ ).ToIso8601DateTimeString( true, true ) );

    CORE::CCoreGlobal::Instance();
    COMCORE::CComCoreGlobal::Instance();
    COM::CComGlobal::Instance();
    VFS::CVfsGlobal::Instance();
    WEB::CWebGlobal::Instance();
    PUBSUB::CPubSubGlobal::Instance();

    CORE::CPlatformNativeConsoleLogger console;
    if ( GUCEF_APP_TYPE == GUCEF_APP_TYPE_CONSOLE )
        CORE::CCoreGlobal::Instance()->GetLogManager().AddLogger( console.GetLogger() );

    PUBSUB::PubSub2PubSub pubSub2PubSub;

    // Check for config param first
    CORE::CValueList keyValueList;
    ParseParams( argc, argv, keyValueList );
    CORE::CString bootstrapConfigPathParam = keyValueList.GetValueAlways( "bootstrapConfigPath" );
    CORE::CString configPathParam = keyValueList.GetValueAlways( "configPath" );
    keyValueList.Clear();

    // Load settings from a config file
    if ( !LoadConfig( bootstrapConfigPathParam, configPathParam ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_CRITICAL, "pubsub2pubsub: Exiting because LoadConfig failed" );
        ::GUCEF::CORE::CCoreGlobal::Instance()->GetApplication().Stop();
        return -1;
    }
    
    // After load get access to the app config
    const CORE::CDataNode* appConfig = pubSub2PubSub.GetAppConfig();
    if ( GUCEF_NULL == appConfig )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_CRITICAL, "pubsub2pubsub: Exiting because no app config is available" );
        ::GUCEF::CORE::CCoreGlobal::Instance()->GetApplication().Stop();
        return -1;
    }

    CORE::Int32 minLogLevel = appConfig->GetAttributeValueOrChildValueByName( "minimalLogLevel" ).AsInt32( CORE::LOGLEVEL_BELOW_NORMAL, true );
    CORE::CCoreGlobal::Instance()->GetLogManager().SetMinLogLevel( minLogLevel );

    CORE::CString outputDir = appConfig->GetAttributeValueOrChildValueByName( "outputDir" ).AsString( "$CURWORKDIR$", true );
    CORE::CreateDirs( outputDir );

    CORE::CString logFilename = CORE::CombinePath( outputDir, "pubsub2pubsub_log.txt" );

    keyValueList.Set( "logfile", logFilename );

    CORE::CRollingFileAccess logFileAccess( logFilename, "w" );
    logFileAccess.SetMaxRolloverFilesBeforeDeletion( 10 );
    CORE::CStdLogger logger( logFileAccess );
    CORE::CCoreGlobal::Instance()->GetLogManager().AddLogger( &logger );
           
    CORE::CCoreGlobal::Instance()->GetLogManager().FlushBootstrapLogEntriesToLogs();
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Flushed to log @ " + logFilename );

    GUCEF_OSMAIN_SIGNAL_HANDLER( GucefAppSignalHandler );

    if ( !pubSub2PubSub.Start() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_CRITICAL, "pubsub2pubsub: Failed to Start()" );
        ::GUCEF::CORE::CCoreGlobal::Instance()->GetApplication().Stop();
        return -2;
    }

    CORE::PulseGeneratorPtr pulseGenerator = CORE::CCoreGlobal::Instance()->GetPulseGenerator();
    pulseGenerator->RequestPulseInterval( 25 );
    pulseGenerator->RequestPulsesPerImmediatePulseRequest( 25 );
    
    CORE::CGUCEFApplication& app = CORE::CCoreGlobal::Instance()->GetApplication();
    returnValue = app.main( argc, argv, true );
    pubSub2PubSub.SetStandbyMode( true );

    }
    catch ( const std::exception& e )
    {
        GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_CRITICAL, "pubsub2pubsub: Main thread stl exeption: " + CORE::CString( e.what() ) );
        throw e;
    }
    #ifndef GUCEF_DEBUG_MODE
    catch ( ... )
    {
        GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_CRITICAL, "pubsub2pubsub: Main thread unknown exeption occured. Caught in catch-all" );
        throw;
    }
    #endif

    return returnValue;
}
GUCEF_OSMAIN_END

/*---------------------------------------------------------------------------*/
