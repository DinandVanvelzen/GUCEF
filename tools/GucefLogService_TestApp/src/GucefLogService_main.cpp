/*
 *  GucefLogService_TestApp: Test application for the logging service for GUCEF
 *
 *  Copyright (C) 2002 - 2009.  Dinand Vanvelzen
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

#ifndef GUCEF_OSMAIN_H
#include "gucef_osmain.h"
#define GUCEF_OSMAIN_H
#endif /* GUCEF_OSMAIN_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CGUCEFAPPLICATION_H
#include "CGUCEFApplication.h"
#define GUCEF_CORE_CGUCEFAPPLICATION_H
#endif /* GUCEF_CORE_CGUCEFAPPLICATION_H ? */

#ifndef GUCEF_CORE_CFILEACCESS_H
#include "CFileAccess.h"
#define GUCEF_CORE_CFILEACCESS_H
#endif /* GUCEF_CORE_CFILEACCESS_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CTFACTORY_H
#include "CTFactory.h"
#define GUCEF_CORE_CTFACTORY_H
#endif /* GUCEF_CORE_CTFACTORY_H ? */

#ifndef GUCEF_CORE_CTASKMANAGER_H
#include "gucefCORE_CTaskManager.h"
#define GUCEF_CORE_CTASKMANAGER_H
#endif /* GUCEF_CORE_CTASKMANAGER_H ? */

#ifndef GUCEF_CORE_CPLUGINCONTROL_H
#include "CPluginControl.h"
#define GUCEF_CORE_CPLUGINCONTROL_H
#endif /* GUCEF_CORE_CPLUGINCONTROL_H ? */

#ifndef GUCEF_CORE_CVALUELIST_H
#include "CValueList.h"
#define GUCEF_CORE_CVALUELIST_H
#endif /* GUCEF_CORE_CVALUELIST_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CPLATFORMNATIVECONSOLEWINDOW_H
#include "gucefCORE_CPlatformNativeConsoleWindow.h"
#define GUCEF_CORE_CPLATFORMNATIVECONSOLEWINDOW_H
#endif /* GUCEF_CORE_CPLATFORMNATIVECONSOLEWINDOW_H ? */

#ifdef GUCEF_MSWIN_BUILD
#include <windows.h>
#endif /* GUCEF_MSWIN_BUILD ? */

#ifndef GUCEF_LOGSERVICELIB_H
#include "GucefLogServiceLib.h"
#define GUCEF_LOGSERVICELIB_H
#endif /* GUCEF_LOGSERVICELIB_H ? */

#ifndef GUCEF_LOGSERVICE_CLOGMSGSIMULATIONTASK_H
#include "GucefLogService_CLogMsgSimulationTask.h"
#define GUCEF_LOGSERVICE_CLOGMSGSIMULATIONTASK_H
#endif /* GUCEF_LOGSERVICE_CLOGMSGSIMULATIONTASK_H ? */

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

const bool defaultShowConsoleState = true;
const bool defaultUseConsoleLoggerState = true;
CORE::UInt16 defaultListnenPort = 43557;

#ifdef GUCEF_DEBUG_MODE
const char* loggingServiceClientPluginFilename = "GucefLogServiceClientPlugin_d";
#else
const char* loggingServiceClientPluginFilename = "GucefLogServiceClientPlugin";
#endif

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

void
ParseParams( const int argc                 ,
             char* argv[]                   ,
             CORE::CValueList& keyValueList )
{GUCEF_TRACE;

    keyValueList.DeleteAll();
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
        keyValueList.SetMultiple( argString, '\'' );
    }
}

/*-------------------------------------------------------------------------*/

GUCEF_OSMAIN_BEGIN
{GUCEF_TRACE;

    int appRetValue = 0;
    
    #ifdef GUCEF_CALLSTACK_TRACING
    CORE::GUCEF_LogStackTo( "GucefLogService_TestApp_Callstack.cvs" );
    CORE::GUCEF_SetStackLoggingInCvsFormat( 1 );
    CORE::GUCEF_SetStackLogging( 1 );
    #endif /* GUCEF_CALLSTACK_TRACING ? */

    CORE::CString logFilename = GUCEF::CORE::RelativePath( "$CURWORKDIR$" );
    CORE::AppendToPath( logFilename, "GucefLogService_TestApp_Log.txt" );
    CORE::CFileAccess logFileAccess( logFilename, "w" );

    CORE::CStdLogger logger( logFileAccess );
    CORE::CCoreGlobal::Instance()->GetLogManager().AddLogger( &logger );

    // Parse the application parameters
    CORE::CValueList keyValueList;
    ParseParams( argc, argv, keyValueList );

    CORE::CPlatformNativeConsoleWindow consoleWindow;
    bool showConsole = defaultShowConsoleState;
    if ( keyValueList.HasKey( "showConsole" ) )
    {
        showConsole = CORE::StringToBool( keyValueList.GetValue( "showConsole" ) );
    }
    if ( showConsole )
    {
        consoleWindow.CreateConsole();
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "GucefLogService_TestApp: Enabled console window" );
    }

    // Do we want to use the console logger?
    CORE::CPlatformNativeConsoleLogger* consoleOut = GUCEF_NULL;
    bool useConsoleLogger = defaultUseConsoleLoggerState;
    if ( keyValueList.HasKey( "useConsoleLogger" ) )
    {
        showConsole = CORE::StringToBool( keyValueList.GetValue( "useConsoleLogger" ) );
    }
    if ( showConsole )
    {
        consoleOut = new CORE::CPlatformNativeConsoleLogger();
        CORE::CCoreGlobal::Instance()->GetLogManager().AddLogger( consoleOut->GetLogger() );

        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "GucefLogService_TestApp: Enabled console output" );
    }

    // Flush bootstrap logging now that we attached all our loggers
    CORE::CCoreGlobal::Instance()->GetLogManager().FlushBootstrapLogEntriesToLogs();

    // Get the desired listnen port for the server
    CORE::UInt16 serverListnenPort = defaultListnenPort;
    if ( keyValueList.HasKey( "port" ) )
    {
        serverListnenPort = CORE::StringToUInt16( keyValueList.GetValue( "port" ) );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "GucefLogService_TestApp: Changing server port from default " + CORE::UInt16ToString( defaultListnenPort ) + " to " + CORE::UInt16ToString( serverListnenPort ) );
    }

    // Get the desired output path for the log files
    CORE::CString logFilesOutputDir = "$MODULEDIR$";
    if ( keyValueList.HasKey( "outputDir" ) )
    {
        logFilesOutputDir = keyValueList.GetValue( "outputDir" );
    }
    else
    {
        logFilesOutputDir = CORE::RelativePath( "$MODULEDIR$" );
    }
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "GucefLogService_TestApp: Setting server output dir for log files to " + logFilesOutputDir );

    // Start the logging server        
    LOGSERVICELIB::CLogSvcServerFileLogger fileLogger( logFilesOutputDir );
    LOGSERVICELIB::CLogSvcServer logServiceServer;
    logServiceServer.RegisterLogger( &fileLogger );
    if ( logServiceServer.ListenOnPort( serverListnenPort ) )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "GucefLogService_TestApp: Server set to listnen on port " + CORE::UInt16ToString( serverListnenPort ) );

        // Load up the client plugin
        CORE::CPluginControl& pluginControl = CORE::CCoreGlobal::Instance()->GetPluginControl();
        pluginControl.AddPluginDir( "$MODULEDIR$" );
        pluginControl.AddPluginDir( "$MODULEDIR$/../lib" );
        pluginControl.AddPluginDir( "$CURWORKDIR$" );
        if ( pluginControl.SearchForPluginInPluginDirs( loggingServiceClientPluginFilename, "", true ) )
        {
            // Start the simulated logging
            typedef CORE::CTFactory< CORE::CTaskConsumer, LOGSERVICE::CLogMsgSimulationTask, MT::CMutex > TLogMsgSimulationFactory;
            TLogMsgSimulationFactory loggingTaskFactory;
            CORE::CTaskManager& taskManager = CORE::CCoreGlobal::Instance()->GetTaskManager();
            taskManager.RegisterTaskConsumerFactory( LOGSERVICE::CLogMsgSimulationTask::TaskType, &loggingTaskFactory );
            if ( taskManager.StartTask( CORE::CTaskManager::DefaultThreadPoolName, LOGSERVICE::CLogMsgSimulationTask::TaskType ).HasAFuture() )
            {
                appRetValue = CORE::CCoreGlobal::Instance()->GetApplication().main( argc, argv, true );
                taskManager.RequestAllThreadsToStop( true, false, 5000 );
            }
            else
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "GucefLogService_TestApp: Failed to start simulated logging task" );
            }
            taskManager.UnregisterTaskConsumerFactory( LOGSERVICE::CLogMsgSimulationTask::TaskType );
        }
        else
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "GucefLogService_TestApp: Unable to locate and load the client plugin" );
        }
        logServiceServer.UnregisterLogger( &fileLogger );
    }
    else
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "GucefLogService_TestApp: Unable to start server listning on port" );
    }            

    CORE::CCoreGlobal::Instance()->GetLogManager().ClearLoggers();

    delete consoleOut;
    consoleOut = GUCEF_NULL;

    return appRetValue;
}
GUCEF_OSMAIN_END

/*-------------------------------------------------------------------------//
//                                                                         //
//      Info & Changes                                                     //
//                                                                         //
//-------------------------------------------------------------------------//

- 11-02-2008 :
        - Initial implementation
          
---------------------------------------------------------------------------*/

