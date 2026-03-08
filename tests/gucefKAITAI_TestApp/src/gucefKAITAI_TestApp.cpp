/*
 *  gucefKAITAI: Platform module supporting the Kaitai binary description format
 *
 *  Copyright (C) 1998 - 2023.  Dinand Vanvelzen
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

#ifndef GUCEF_MACROS_H
#include "gucef_macros.h"
#define GUCEF_MACROS_H
#endif /* GUCEF_MACROS_H ? */

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_VFS_H
#include "gucefVFS.h"
#define GUCEF_VFS_H
#endif /* GUCEF_VFS_H ? */

#ifndef GUCEF_KAITAI_H
#include "gucefKAITAI.h"
#define GUCEF_KAITAI_H
#endif /* GUCEF_KAITAI_H ? */

#ifndef GUCEF_CORE_CGLOBALCONFIGVALUELIST_H
#include "gucefCORE_CGlobalConfigValueList.h"
#define GUCEF_CORE_CGLOBALCONFIGVALUELIST_H
#endif /* GUCEF_CORE_CGLOBALCONFIGVALUELIST_H ? */

#ifndef TESTCODE_KAITAISCHEMALOADTEST_H
#include "TestCode_KaitaiSchemaLoadTest.h"
#define TESTCODE_KAITAISCHEMALOADTEST_H
#endif /* TESTCODE_KAITAISCHEMALOADTEST_H ? */

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
    const CORE::CString bootstrapConfigFile = "gucefKAITAI_TestApp_bootstrap_d.ini";
    const CORE::CString configFile = "gucefKAITAI_TestApp_d.ini";
    #else
    const CORE::CString bootstrapConfigFile = "gucefKAITAI_TestApp_bootstrap.ini";
    const CORE::CString configFile = "gucefKAITAI_TestApp.ini";
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
    logFilename = CORE::CombinePath( outDir, "gucefKAITAI_TestApp_Log.txt" );
   
    CORE::CFileAccess logFileAccess( logFilename, "w" );
    CORE::CStdLogger logger( logFileAccess );
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

/*
 *      Application entry point
 */
GUCEF_OSMAIN_BEGIN
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
        GUCEF::KAITAI::CKaitaiGlobal::Instance();

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

            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "gucefKAITAI_TestApp: Failed to load config" );
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
        PerformKaitaiTest_ConfigDrivenCodecSpecification();

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
        //DRGUP_DumpCallstack( "gucefKAITAI_TestApp_callstack.txt" );
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
GUCEF_OSMAIN_END
