/*
 *  GucefLogService: Independent logging service for GUCEF
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

#ifndef GUCEF_COMCORE_CCOMCOREGLOBAL_H
#include "gucefCOMCORE_CComCoreGlobal.h"
#define GUCEF_COMCORE_CCOMCOREGLOBAL_H
#endif /* GUCEF_COMCORE_CCOMCOREGLOBAL_H ? */

#ifndef GUCEF_COM_CCOMGLOBAL_H
#include "gucefCOM_CComGlobal.h"
#define GUCEF_COM_CCOMGLOBAL_H
#endif /* GUCEF_COM_CCOMGLOBAL_H ? */

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

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

const bool defaultShowConsoleState = false;
const bool defaultUseConsoleLoggerState = true;
CORE::UInt16 defaultListnenPort = 43557;

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

static void
GucefAppSignalHandler( int signal )
{GUCEF_TRACE; 

    GUCEF_LOG( CORE::LOGLEVEL_IMPORTANT, "GucefAppSignalHandler: Received signal " << signal );
    
    // signal context: only the async-signal-safe stop request, the application stops at its next pulse
    ::GUCEF::CORE::CGUCEFApplication::RequestStopFromSignal( signal );
}

/*-------------------------------------------------------------------------*/

GUCEF_OSMAIN_BEGIN
{GUCEF_TRACE;

    // a service: a stop signal during the start is kept and handed to the stop handler once it is installed, instead
    // of ending the process at once (or being ignored, for process 1 of a container)
    GUCEF_OSMAIN_DEFER_STOP_SIGNALS;

    // Initialize modules we depend on
    CORE::CCoreGlobal::Instance();
    COMCORE::CComCoreGlobal::Instance();
    COM::CComGlobal::Instance();

    #ifdef GUCEF_CALLSTACK_TRACING
    CORE::GUCEF_LogStackTo( "GucefLogServiceApp_Callstack.cvs" );
    CORE::GUCEF_SetStackLoggingInCvsFormat( 1 );
    CORE::GUCEF_SetStackLogging( 1 );
    #endif /* GUCEF_CALLSTACK_TRACING ? */

    CORE::CString logFilename = GUCEF::CORE::RelativePath( "$CURWORKDIR$" );
    CORE::AppendToPath( logFilename, "GucefLogServiceApp_Log.txt" );
    CORE::CFileAccess logFileAccess( logFilename, "w" );

    CORE::CStdLogger logger( logFileAccess );
    CORE::CCoreGlobal::Instance()->GetLogManager().AddLogger( &logger );

    // Parse the application parameters
    CORE::CValueList keyValueList;
    ParseParams( argc, argv, keyValueList );

    CORE::CPlatformNativeConsoleWindow consoleWindow;
    bool showConsole = defaultShowConsoleState;
    CORE::CVariant value;
    if ( keyValueList.TryGetValue( "showConsole", value ) )
    {
        showConsole = value.AsBool( showConsole );
    }
    if ( showConsole )
    {
        consoleWindow.CreateConsole();
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "GucefLogServiceApp: Enabled console window" );
    }

    // Do we want to use the console logger?
    CORE::CPlatformNativeConsoleLogger* consoleOut = NULL;
    bool useConsoleLogger = defaultUseConsoleLoggerState;
    if ( keyValueList.TryGetValue( "useConsoleLogger", value ) )
    {
        useConsoleLogger = value.AsBool( useConsoleLogger );
    }
    if ( showConsole )
    {
        consoleOut = new CORE::CPlatformNativeConsoleLogger();
        CORE::CCoreGlobal::Instance()->GetLogManager().AddLogger( consoleOut->GetLogger() );

        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "GucefLogServiceApp: Enabled console output" );
    }

    // Flush bootstrap logging now that we attached all our loggers
    CORE::CCoreGlobal::Instance()->GetLogManager().FlushBootstrapLogEntriesToLogs();

    // Get the desired listnen port for the server
    CORE::UInt16 serverListnenPort = defaultListnenPort;
    if ( keyValueList.TryGetValue( "port", value ) )
    {
        serverListnenPort = value.AsUInt16( serverListnenPort );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "GucefLogServiceApp: Changing server port from default " << defaultListnenPort << " to " << serverListnenPort );
    }

    // Get the desired output path for the log files
    CORE::CString logFilesOutputDir = "$MODULEDIR$";
    if ( keyValueList.TryGetValue( "outputDir", value ) )
    {
        logFilesOutputDir = value.AsString( logFilesOutputDir, true );
    }
    else
    {
        logFilesOutputDir = CORE::RelativePath( "$MODULEDIR$" );
    }
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "GucefLogServiceApp: Setting server output dir for log files to " << logFilesOutputDir );

    int appRetValue = 0;
    LOGSERVICELIB::CLogSvcServerFileLogger fileLogger( logFilesOutputDir );
    LOGSERVICELIB::CLogSvcServer logServiceServer;
    logServiceServer.RegisterLogger( &fileLogger );

    if ( logServiceServer.ListenOnPort( serverListnenPort ) )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "GucefLogServiceApp: Server set to listnen on port " << serverListnenPort );

        GUCEF_OSMAIN_SIGNAL_HANDLER( GucefAppSignalHandler );
        appRetValue = CORE::CCoreGlobal::Instance()->GetApplication().main( argc, argv, true );
    }

    logServiceServer.UnregisterLogger( &fileLogger );

    CORE::CCoreGlobal::Instance()->GetLogManager().ClearLoggers();

    delete consoleOut;
    consoleOut = NULL;

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

