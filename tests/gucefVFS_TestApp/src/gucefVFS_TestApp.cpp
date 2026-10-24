/*
 *  gucefVFS: GUCEF module implementing a Virtual File System
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
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA 
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

/* the test framework implementation is compiled as part of this file */
#define GUCEF_TESTFW_IMPL

#include <string.h>

#ifndef GUCEF_MACROS_H
#include "gucef_macros.h"
#define GUCEF_MACROS_H
#endif /* GUCEF_MACROS_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_VFS_H
#include "gucefVFS.h"
#define GUCEF_VFS_H
#endif /* GUCEF_VFS_H ? */

#ifndef GUCEF_CORE_CGLOBALCONFIGVALUELIST_H
#include "gucefCORE_CGlobalConfigValueList.h"
#define GUCEF_CORE_CGLOBALCONFIGVALUELIST_H
#endif /* GUCEF_CORE_CGLOBALCONFIGVALUELIST_H ? */

#ifndef TESTCODE_VFSFILELOADUNLOADTEST_H
#include "TestCode_VFSFileLoadUnloadTest.h"
#define TESTCODE_VFSFILELOADUNLOADTEST_H
#endif /* TESTCODE_VFSFILELOADUNLOADTEST_H ? */

#ifndef GUCEF_VFS_TESTAPP_TESTZIPARCHIVEWRITER_H
#include "TestZipArchiveWriter.h"
#define GUCEF_VFS_TESTAPP_TESTZIPARCHIVEWRITER_H
#endif /* GUCEF_VFS_TESTAPP_TESTZIPARCHIVEWRITER_H ? */

#ifndef GUCEF_VFS_TESTAPP_TESTISO9660ARCHIVE_H
#include "TestIso9660Archive.h"
#include "TestCueBinArchive.h"
#define GUCEF_VFS_TESTAPP_TESTISO9660ARCHIVE_H
#endif /* GUCEF_VFS_TESTAPP_TESTISO9660ARCHIVE_H ? */

#ifndef GUCEF_VFS_TESTAPP_TESTITVARCHIVE_H
#include "TestItvArchive.h"
#define GUCEF_VFS_TESTAPP_TESTITVARCHIVE_H
#endif /* GUCEF_VFS_TESTAPP_TESTITVARCHIVE_H ? */

#ifndef GUCEF_VFS_TESTAPP_TESTMOUNTFROMRESOURCE_H
#include "TestMountFromResource.h"
#define GUCEF_VFS_TESTAPP_TESTMOUNTFROMRESOURCE_H
#endif /* GUCEF_VFS_TESTAPP_TESTMOUNTFROMRESOURCE_H ? */

#ifndef GUCEF_VFS_TESTAPP_TESTLIBARCHIVE_H
#include "TestLibArchive.h"
#define GUCEF_VFS_TESTAPP_TESTLIBARCHIVE_H
#endif /* GUCEF_VFS_TESTAPP_TESTLIBARCHIVE_H ? */

#ifndef GUCEF_VFS_TESTAPP_TESTFILESYSTEMACCESS_H
#include "TestFileSystemAccess.h"
#define GUCEF_VFS_TESTAPP_TESTFILESYSTEMACCESS_H
#endif /* GUCEF_VFS_TESTAPP_TESTFILESYSTEMACCESS_H ? */

#ifndef GUCEF_VFS_TESTAPP_TESTVFSCONFIG_H
#include "TestVfsConfig.h"
#define GUCEF_VFS_TESTAPP_TESTVFSCONFIG_H
#endif /* GUCEF_VFS_TESTAPP_TESTVFSCONFIG_H ? */

#ifndef GUCEF_VFS_TESTAPP_TESTVFSCODECS_H
#include "TestVfsCodecs.h"
#define GUCEF_VFS_TESTAPP_TESTVFSCODECS_H
#endif /* GUCEF_VFS_TESTAPP_TESTVFSCODECS_H ? */

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
LoadConfig( const CORE::CString& bootstrapConfigPath   ,
            const CORE::CString& configPath            ,
            CORE::CValueList& keyValueList             ,
            CORE::CDataNode* loadedConfig = GUCEF_NULL )
{GUCEF_TRACE;

    #ifdef GUCEF_DEBUG_MODE
    const CORE::CString bootstrapConfigFile = "ProcessMetrics_bootstrap_d.ini";
    const CORE::CString configFile = "ProcessMetrics_d.ini";
    #else
    const CORE::CString bootstrapConfigFile = "ProcessMetrics_bootstrap.ini";
    const CORE::CString configFile = "ProcessMetrics.ini";
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

    CORE::CGlobalConfigValueList globalCfg;
    globalCfg.SetConfigNamespace( "Main/AppArgs" );    
    globalCfg.SetAllowDuplicates( false );
    globalCfg.SetAllowMultipleValues( true );

    bool loadSuccess = configStore.LoadConfig( loadedConfig );

    keyValueList = globalCfg;
    return loadSuccess;
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

/*-------------------------------------------------------------------------*/

void
SetupLogging( const CORE::CString& outputDir ,
              CORE::CString& logFilename     )
{GUCEF_TRACE;

    CORE::CString outDir = CORE::RelativePath( outputDir );
    CORE::CreateDirs( outDir );

    // setup file logger
    logFilename = CORE::CombinePath( outDir, "gucefVFS_TestApp_Log.txt" );
   
    // Note: the logger stays registered with the log manager so it has to outlive this function
    static CORE::CFileAccess logFileAccess( logFilename, "w" );
    static CORE::CStdLogger logger( logFileAccess );
    CORE::CCoreGlobal::Instance()->GetLogManager().AddLogger( &logger );

    // setup console logger
    static CORE::CPlatformNativeConsoleLogger consoleOut;
    CORE::CCoreGlobal::Instance()->GetLogManager().AddLogger( consoleOut.GetLogger() );

    // flush startup log entries
    CORE::CCoreGlobal::Instance()->GetLogManager().FlushBootstrapLogEntriesToLogs();
}

/*-------------------------------------------------------------------------*/

void
SetupLogging( void )
{GUCEF_TRACE;

    CORE::CString logFilename;
    SetupLogging( "$MODULEDIR$", logFilename );
}

/*-------------------------------------------------------------------------*/

/**
 *  The original interactive mode of the test app, available via the -manual switch
 */
static int
RunManualTests( int argc, char** argv )
{GUCEF_TRACE;

    #ifdef GUCEF_VFS_DEBUG_MODE
    //CORE::GUCEF_LogStackToStdOut();
    //CORE::GUCEF_SetStackLogging( 1 );
    #endif /* GUCEF_VFS_DEBUG_MODE ? */

    try
    {
        // Initialize systems
        GUCEF::CORE::CCoreGlobal::Instance();
        GUCEF::VFS::CVfsGlobal::Instance();

        // Check for config param first
        CORE::CValueList keyValueList;
        ParseParams( argc, argv, keyValueList );
        CORE::CString bootstrapConfigPathParam = keyValueList.GetValueAlways( "BootstrapConfigPath" );
        CORE::CString configPathParam = keyValueList.GetValueAlways( "ConfigPath" );
        keyValueList.Clear();

        // Load settings from a config file (if any) and then override with params (if any)
        CORE::CDataNode* globalConfig = new CORE::CDataNode();
        if ( !LoadConfig( bootstrapConfigPathParam, configPathParam, keyValueList, globalConfig ) )
        {
            SetupLogging();
            CORE::ShowErrorMessage( "Initialization error"                ,
                                    "Failures occured loading the config" );

            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "gucefVFS_TestApp: Failed to load config" );
            return -1;
        }
        ParseParams( argc, argv, keyValueList );

        CORE::Int32 minLogLevel = CORE::LOGLEVEL_BELOW_NORMAL;
        CORE::CString valueStr = keyValueList.GetValueAlways( "MinimalLogLevel" );
        if ( !valueStr.IsNULLOrEmpty() )
        {
            minLogLevel = CORE::StringToInt32( valueStr );
            CORE::CCoreGlobal::Instance()->GetLogManager().SetMinLogLevel( minLogLevel );
        }

        CORE::CString logFilename;
        CORE::CString outputDir = CORE::RelativePath( keyValueList.GetValueAlways( "outputDir", "$CURWORKDIR$" ) );
        SetupLogging( outputDir, logFilename );
        keyValueList.Set( "logfile", logFilename );

        // Create console window for easy test interaction
        CORE::CPlatformNativeConsoleWindow consoleWindow;
        consoleWindow.CreateConsole();

        // Now actually run the tests...
        //PerformVFSFileLoadUnloadTest();

        auto& app = CORE::CCoreGlobal::Instance()->GetApplication();
        app.GetPulseGenerator()->RequestPulseInterval( 10 );
        int appReturnValue = app.main( argc, argv, true );

        CORE::CCoreGlobal::Instance()->GetLogManager().ClearLoggers();
        return appReturnValue;
    }
    catch ( ... )
    {
        #ifdef GUCEF_VFS_DEBUG_MODE
        //DRGUP_PrintCallstack();
        //DRGUP_DumpCallstack( "gucefVFS_TestApp_callstack.txt" );
        #endif /* GUCEF_VFS_DEBUG_MODE ? */

        try
        {
            CORE::CCoreGlobal::Instance()->GetLogManager().ClearLoggers();
        }
        catch ( ... ) {}

        CORE::ShowErrorMessage( "Unknown exception"                                                                 ,
                                "Unhandled exception during program execution, the application will now terminate"  );
    }
    return 1;
}

/*-------------------------------------------------------------------------*/

/*
 *      Application entry point
 */
GUCEF_OSMAIN_BEGIN
{GUCEF_TRACE;

    try
    {
        for ( int i=1; i<argc; ++i )
        {
            if ( 0 == strcmp( argv[ i ], "-manual" ) )
                return RunManualTests( argc, argv );
            if ( 0 == strcmp( argv[ i ], "-large" ) )
                CORE::GUCEFSetEnv( "GUCEF_TEST_LARGE_FILES", "1" );
        }

        CORE::CString logFilename = CORE::RelativePath( "$CURWORKDIR$" );
        CORE::AppendToPath( logFilename, "gucefVFS_TestApp_Log.txt" );
        CORE::CFileAccess logFileAccess( logFilename, "w" );

        CORE::CStdLogger logger( logFileAccess );
        CORE::CCoreGlobal::Instance()->GetLogManager().AddLogger( &logger );

        CORE::CPlatformNativeConsoleLogger console;
        if ( GUCEF_APP_TYPE == GUCEF_APP_TYPE_CONSOLE )
            CORE::CCoreGlobal::Instance()->GetLogManager().AddLogger( console.GetLogger() );

        // initializes the module, as every application using gucefVFS has to do
        VFS::CVfsGlobal::Instance();

        GUCEF_TESTFW_INIT( "gucefVFS_TestApp" );
        GUCEF_TESTFW_SET_CALLBACK( CORE::TestFrameworkLogCallback, GUCEF_NULL );

        PerformVFSFileLoadUnloadTest();
        PerformZipArchiveWriterTests();
        PerformMountFromResourceTests();
        PerformLibArchiveTests();
        PerformItvArchiveTests();
        PerformIso9660ArchiveTests();
        PerformCueBinArchiveTests();
        PerformFileSystemAccessTests();
        PerformVfsConfigTests();
        PerformVfsCodecTests();

        CORE::CLogStreamScope::FlushLogs();

        CORE::CString xmlPath = CORE::RelativePath( "$CURWORKDIR$" );
        CORE::AppendToPath( xmlPath, "gucefVFS_TestApp_Results.xml" );
        GUCEF_TESTFW_WRITE_RESULTS( xmlPath.C_String() );

        CORE::CCoreGlobal::Instance()->GetLogManager().ClearLoggers();
        return GUCEF_TESTFW_EXITCODE;
    }
    catch ( ... )
    {
        CORE::ShowErrorMessage( "Unknown exception"                                                                 ,
                                "Unhandled exception during program execution, the application will now terminate"  );
    }
    return 1;
}
GUCEF_OSMAIN_END

/*-------------------------------------------------------------------------//
//                                                                         //
//      Info & Changes                                                     //
//                                                                         //
//-------------------------------------------------------------------------//

- 10-12-2006
        - Dinand: Added this comment section

-----------------------------------------------------------------------------*/