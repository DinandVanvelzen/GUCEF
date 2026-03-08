/*
 *  gucefGUI_TestApp: Application for testing gucefGUI
 *  Copyright (C) 2002 - 2007.  Dinand Vanvelzen
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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_CFILEACCESS_H
#include "CFileAccess.h"
#define GUCEF_CORE_CFILEACCESS_H
#endif /* GUCEF_CORE_CFILEACCESS_H ? */

#ifndef GUCEF_CORE_DVOSWRAP_H
#include "DVOSWRAP.h"
#define GUCEF_CORE_DVOSWRAP_H
#endif /* GUCEF_CORE_DVOSWRAP_H ? */

#ifndef GUCEF_CORE_CONFIGSTORE_H
#include "CConfigStore.h"
#define GUCEF_CORE_CONFIGSTORE_H
#endif /* GUCEF_CORE_CONFIGSTORE_H ? */

#ifndef GUCEF_CORE_CPLUGINCONTROL_H
#include "CPluginControl.h"
#define GUCEF_CORE_CPLUGINCONTROL_H
#endif /* GUCEF_CORE_CPLUGINCONTROL_H ? */

#ifndef GUCEF_CORE_CPLUGINMETADATA_H
#include "gucefCORE_CPluginMetaData.h"
#define GUCEF_CORE_CPLUGINMETADATA_H
#endif /* GUCEF_CORE_CPLUGINMETADATA_H ? */

#ifndef GUCEF_CORE_CGUCEFAPPLICATION_H
#include "CGUCEFApplication.h"
#define GUCEF_CORE_CGUCEFAPPLICATION_H
#endif /* GUCEF_CORE_CGUCEFAPPLICATION_H ? */

#ifndef GUCEF_CORE_CPLATFORMNATIVECONSOLEWINDOW_H
#include "gucefCORE_CPlatformNativeConsoleWindow.h"
#define GUCEF_CORE_CPLATFORMNATIVECONSOLEWINDOW_H
#endif /* GUCEF_CORE_CPLATFORMNATIVECONSOLEWINDOW_H ? */

#include "gucef_dynnewon.h"

#if defined( GUCEF_USE_MEMORY_LEAK_CHECKER ) && defined( GUCEF_USE_PLATFORM_MEMORY_LEAK_CHECKER ) && !defined( GUCEF_DYNNEWON_DISABLED )

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

bool
LoadConfig( void )
{
    #ifdef GUCEF_GUI_DEBUG_MODE
    const CORE::CString configFile = "bootstrap_d.ini";
    #else
    const CORE::CString configFile = "bootstrap.ini";
    #endif

    CORE::CString configFilePath = CORE::CombinePath( "$MODULEDIR$", configFile );
    configFilePath = CORE::RelativePath( configFilePath );

    if ( !FileExists( configFilePath ) )
    {
        configFilePath = CORE::CombinePath( "$CURWORKDIR$", configFile );
        configFilePath = CORE::RelativePath( configFilePath );

        if ( !FileExists( configFilePath ) )
        {
            // hardcoded relative path from compiler output bin to testdata folder in the archive
            #if GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN
            configFilePath = CORE::CombinePath( "$MODULEDIR$/../../../TestData/MemoryLeakFinder_TestApp/mswin", configFile );
            #elif GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX
            configFilePath = CORE::CombinePath( "$MODULEDIR$/../../TestData/MemoryLeakFinder_TestApp/linux", configFile );
            #endif
            configFilePath = CORE::RelativePath( configFilePath );
        }
    }

    CORE::CConfigStore& configStore = CORE::CCoreGlobal::Instance()->GetConfigStore();
    configStore.SetConfigFile( configFilePath );
    return configStore.LoadConfig();
}

/*-------------------------------------------------------------------------*/

/*
 *      Application entry point
 */
GUCEF_OSMAIN_BEGIN
{GUCEF_TRACE;

    #ifdef GUCEF_DEBUG_MODE
    //MEMMAN_LogStackToStdOut();
    //MEMMAN_SetStackLogging( 1 );
    #endif /* GUCEF_DEBUG_MODE ? */

    try
    {
        // Initialize systems
        GUCEF::CORE::CCoreGlobal::Instance();

        // setup file logger
        CORE::CString logFilename = GUCEF::CORE::RelativePath( "$CURWORKDIR$" );
        CORE::AppendToPath( logFilename, "MemoryLeakFinder_TestApp_Log.txt" );
        CORE::CFileAccess logFileAccess( logFilename, "w" );

        CORE::CStdLogger logger( logFileAccess );
        CORE::CCoreGlobal::Instance()->GetLogManager().AddLogger( &logger );

        // setup console logger
        CORE::CPlatformNativeConsoleLogger consoleOut;
        CORE::CCoreGlobal::Instance()->GetLogManager().AddLogger( consoleOut.GetLogger() );

        // flush startup log entries
        CORE::CCoreGlobal::Instance()->GetLogManager().FlushBootstrapLogEntriesToLogs();

        // Create console window for easy test interaction
        CORE::CPlatformNativeConsoleWindow consoleWindow;
        consoleWindow.CreateConsole();

        bool errorOccured = false;

        // Since this is a test we will explicitly invoke the loader
        MEMMAN_LazyLoadMemoryManager();
        
        fp_MEMMAN_Initialize();
        
        #ifdef MEMCHECK_OLEAPI

        wchar_t* ptr = SysAllocStringByteLen( "TEST", 10 );

        fp_MEMMAN_DumpMemoryAllocations();

        SysFreeString( ptr );

        fp_MEMMAN_DumpMemoryAllocations();
        fp_MEMMAN_DumpLogReport();

        #endif /* MEMCHECK_OLEAPI ? */


        // Load the config for this test
        //if ( !LoadConfig() )
        //{
        //    CORE::ShowErrorMessage( "Initialization error"                ,
        //                            "Failures occured loading the config" );
        //}

        CORE::CCoreGlobal::Instance()->GetLogManager().ClearLoggers();
        return 1;
    }
    catch ( ... )
    {
        #ifdef GUCEF_DEBUG_MODE
        //CORE::GUCEF_PrintCallstack();
        //CORE::GUCEF_DumpCallstack( "gucefGUI_TestApp_callstack.txt" );
        #endif /* GUCEF_DEBUG_MODE ? */

        CORE::ShowErrorMessage( "Unknown exception"                                                                 ,
                                "Unhandled exception during program execution, the application will now terminate"  );
    }
    return 1;
}
GUCEF_OSMAIN_END

/*-------------------------------------------------------------------------*/

#else

GUCEF_OSMAIN_BEGIN
{GUCEF_TRACE;
}
GUCEF_OSMAIN_END

#endif

/*-------------------------------------------------------------------------*/
