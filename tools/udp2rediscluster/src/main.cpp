/*
 *  Udp2RedisCluster: service which pushes UDP packets into clustered redis streams
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

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

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

#ifndef GUCEF_COMCORE_CUDPSOCKET_H
#include "CUDPSocket.h"
#define GUCEF_COMCORE_CUDPSOCKET_H
#endif /* GUCEF_COMCORE_CUDPSOCKET_H ? */

#ifndef GUCEF_COMCORE_CTCPSERVERSOCKET_H
#include "CTCPServerSocket.h"
#define GUCEF_COMCORE_CTCPSERVERSOCKET_H
#endif /* GUCEF_COMCORE_CTCPSERVERSOCKET_H ? */

#ifndef GUCEF_COMCORE_CCOMCOREGLOBAL_H
#include "gucefCOMCORE_CComCoreGlobal.h"
#define GUCEF_COMCORE_CCOMCOREGLOBAL_H
#endif /* GUCEF_COMCORE_CCOMCOREGLOBAL_H ? */

#ifndef GUCEF_COM_CCOMGLOBAL_H
#include "gucefCOM_CComGlobal.h"
#define GUCEF_COM_CCOMGLOBAL_H
#endif /* GUCEF_COM_CCOMGLOBAL_H ? */

#ifndef GUCEF_WEB_CWEBGLOBAL_H
#include "gucefWEB_CWebGlobal.h"
#define GUCEF_WEB_CWEBGLOBAL_H
#endif /* GUCEF_WEB_CWEBGLOBAL_H ? */

#include "udp2rediscluster.h"

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
LookForConfigFile( const CORE::CString& configFile )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "Checking for config file @ " + configFile );
    if ( !CORE::FileExists( configFile ) )
    {
        CORE::CString configFilePath = CORE::CombinePath( "$CURWORKDIR$", configFile );
        configFilePath = CORE::RelativePath( configFilePath );

        GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "Checking for config file @ " + configFilePath );
        if ( !CORE::FileExists( configFilePath ) )
        {
            configFilePath = CORE::CombinePath( "$MODULEDIR$", configFile );
            configFilePath = CORE::RelativePath( configFilePath );

            GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "Checking for config file @ " + configFilePath );
            if ( !FileExists( configFilePath ) )
            {
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
    const CORE::CString bootstrapConfigFile = "udp2rediscluster_bootstrap_d.ini";
    const CORE::CString configFile = "udp2rediscluster_d.json";
    #else
    const CORE::CString bootstrapConfigFile = "udp2rediscluster_bootstrap.ini";
    const CORE::CString configFile = "udp2rediscluster.json";
    #endif

    CORE::CConfigStore& configStore = CORE::CCoreGlobal::Instance()->GetConfigStore();

    CORE::CString bootstrapConfigFilePath = LookForConfigFile( bootstrapConfigFile );
    CORE::CString configFilePath = LookForConfigFile( configFile );

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

void
FallbackFlushBootstrapLog( void )
{GUCEF_TRACE;

    CORE::CCoreGlobal::Instance()->GetLogManager().SetMinLogLevel( CORE::LOGLEVEL_EVERYTHING );

    GUCEF_WARNING_LOG( CORE::LOGLEVEL_IMPORTANT, "FallbackFlushBootstrapLog: Flushing bootstrap log entries for diagnostics using hardcoded fallbacks" );

    CORE::CString outputDir = CORE::RelativePath( "$CURWORKDIR$" );
    if ( !CORE::CreateDirs( outputDir ) )
    {
        outputDir = CORE::RelativePath( "$MODULEDIR$" );
    }
    CORE::CString logFilename = CORE::CombinePath( outputDir, "Udp2RedisCluster_log.txt" );

    CORE::CRollingFileAccess logFileAccess( logFilename, "w" );
    logFileAccess.SetMaxRolloverFilesBeforeDeletion( 10 );
    CORE::CStdLogger logger( logFileAccess );
    CORE::CCoreGlobal::Instance()->GetLogManager().AddLogger( &logger );

    CORE::CCoreGlobal::Instance()->GetLogManager().FlushBootstrapLogEntriesToLogs();
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Flushed to fallback bootstrap log @ " + logFilename );
}

/*-------------------------------------------------------------------------*/

/*
 *      Application entry point
 */
//GUCEF_OSMAIN_BEGIN
GUCEF_OSSERVICEMAIN_BEGIN( "udp2rediscluster" )
{GUCEF_TRACE;

    int returnValue = -100;
    try
    {

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "This service was compiled on: " + CORE::CDateTime::CompileDateTime( __DATE__, __TIME__ ).ToIso8601DateTimeString( true, true ) );

    CORE::CCoreGlobal::Instance();
    COMCORE::CComCoreGlobal::Instance();
    COM::CComGlobal::Instance();
    WEB::CWebGlobal::Instance();

    CORE::CPlatformNativeConsoleLogger console;
    if ( GUCEF_APP_TYPE == GUCEF_APP_TYPE_CONSOLE )
        CORE::CCoreGlobal::Instance()->GetLogManager().AddLogger( console.GetLogger() );

    Udp2RedisCluster udp2RedisCluster;

    // Check for config param first
    CORE::CValueList keyValueList;
    ParseParams( argc, argv, keyValueList );
    CORE::CString bootstrapConfigPathParam = keyValueList.GetValueAlways( "BootstrapConfigPath" );
    CORE::CString configPathParam = keyValueList.GetValueAlways( "ConfigPath" );
    keyValueList.Clear();

    // Load settings from a config file
    if ( !LoadConfig( bootstrapConfigPathParam, configPathParam ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_CRITICAL, "Udp2RedisCluster: Exiting because LoadConfig failed" );
        FallbackFlushBootstrapLog();
        return -1;
    }

    // After load get access to the app config
    const CORE::CDataNode* appConfig = udp2RedisCluster.GetAppConfig();
    if ( GUCEF_NULL == appConfig )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_CRITICAL, "Udp2RedisCluster: Exiting because no app config is available" );
        FallbackFlushBootstrapLog();
        return -1;
    }

    CORE::Int32 minLogLevel = appConfig->GetAttributeValueOrChildValueByName( "MinimalLogLevel" ).AsInt32( CORE::LOGLEVEL_BELOW_NORMAL, true );
    CORE::CCoreGlobal::Instance()->GetLogManager().SetMinLogLevel( minLogLevel );

    CORE::CString outputDir = appConfig->GetAttributeValueOrChildValueByName( "outputDir" ).AsString( "$CURWORKDIR$", true );
    CORE::CreateDirs( outputDir );

    CORE::CString logFilename = CORE::CombinePath( outputDir, "Udp2RedisCluster_log.txt" );

    keyValueList.Set( "logfile", logFilename );

    CORE::CRollingFileAccess logFileAccess( logFilename, "w" );
    logFileAccess.SetMaxRolloverFilesBeforeDeletion( 10 );
    CORE::CStdLogger logger( logFileAccess );
    CORE::CCoreGlobal::Instance()->GetLogManager().AddLogger( &logger );

    CORE::CCoreGlobal::Instance()->GetLogManager().FlushBootstrapLogEntriesToLogs();
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Flushed to log @ " + logFilename );

    GUCEF_OSMAIN_SIGNAL_HANDLER( GucefAppSignalHandler );

    if ( !udp2RedisCluster.Start() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_CRITICAL, "Udp2RedisCluster: Failed to Start()" );
        return -2;
    }

    CORE::PulseGeneratorPtr pulseGenerator = CORE::CCoreGlobal::Instance()->GetPulseGenerator();
    pulseGenerator->RequestPulseInterval( 25 );
    pulseGenerator->RequestPulsesPerImmediatePulseRequest( 25 );

    auto& app = CORE::CCoreGlobal::Instance()->GetApplication();
    returnValue = app.main( argc, argv, true );
    udp2RedisCluster.SetStandbyMode( true );

    }
    catch ( const std::exception& e )
    {
        GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_CRITICAL, "Udp2RedisCluster: Main thread stl exeption: " + CORE::CString( e.what() ) );
        throw e;
    }
    #ifndef GUCEF_DEBUG_MODE
    catch ( ... )
    {
        GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_CRITICAL, "Udp2RedisCluster: Main thread unknown exeption occured. Caught in catch-all" );
        throw;
    }
    #endif

    return returnValue;
}
GUCEF_OSMAIN_END

/*---------------------------------------------------------------------------*/
