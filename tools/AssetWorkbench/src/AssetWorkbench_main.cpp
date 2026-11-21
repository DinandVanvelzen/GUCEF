/*
 *  AssetWorkbench: workbench and content browser for models, textures, materials and scenes
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

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_VFS_H
#include "gucefVFS.h"
#define GUCEF_VFS_H
#endif /* GUCEF_VFS_H ? */

#ifndef GUCEF_IMAGE_CIMAGEGLOBAL_H
#include "gucefIMAGE_CImageGlobal.h"
#define GUCEF_IMAGE_CIMAGEGLOBAL_H
#endif /* GUCEF_IMAGE_CIMAGEGLOBAL_H ? */

#ifndef GUCEF_INPUT_CINPUTGLOBAL_H
#include "gucefINPUT_CInputGlobal.h"
#define GUCEF_INPUT_CINPUTGLOBAL_H
#endif /* GUCEF_INPUT_CINPUTGLOBAL_H ? */

#ifndef GUCEF_GUI_CGUIGLOBAL_H
#include "gucefGUI_CGuiGlobal.h"
#define GUCEF_GUI_CGUIGLOBAL_H
#endif /* GUCEF_GUI_CGUIGLOBAL_H ? */

#ifndef GUCEF_COMCORE_CCOMCOREGLOBAL_H
#include "gucefCOMCORE_CComCoreGlobal.h"
#define GUCEF_COMCORE_CCOMCOREGLOBAL_H
#endif /* GUCEF_COMCORE_CCOMCOREGLOBAL_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETSTOREGLOBAL_H
#include "gucefASSETSTORE_CAssetStoreGlobal.h"
#define GUCEF_ASSETSTORE_CASSETSTOREGLOBAL_H
#endif /* GUCEF_ASSETSTORE_CASSETSTOREGLOBAL_H ? */

#ifndef GUCEF_OSMAIN_H
#include "gucef_osmain.h"
#define GUCEF_OSMAIN_H
#endif /* GUCEF_OSMAIN_H ? */

#ifndef ASSETWORKBENCHLIB_CWORKBENCHAPP_H
#include "AssetWorkbenchLib_CWorkbenchApp.h"
#define ASSETWORKBENCHLIB_CWORKBENCHAPP_H
#endif /* ASSETWORKBENCHLIB_CWORKBENCHAPP_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** A config file: the given file, the default name in the given folder, in the working folder or next to the program */
GUCEF::CORE::CString
LookForConfigFile( const GUCEF::CORE::CString& given, const GUCEF::CORE::CString& defaultName )
{GUCEF_TRACE;

    if ( !given.IsNULLOrEmpty() )
    {
        const GUCEF::CORE::CString path = GUCEF::CORE::RelativePath( given );
        if ( GUCEF::CORE::FileExists( path ) )
            return path;
        const GUCEF::CORE::CString inFolder = GUCEF::CORE::CombinePath( path, defaultName );
        if ( GUCEF::CORE::FileExists( inFolder ) )
            return inFolder;
        return GUCEF::CORE::CString::Empty;
    }
    const char* const folders[] = { "$CURWORKDIR$", "$MODULEDIR$" };
    for ( size_t i=0; i<sizeof( folders ) / sizeof( folders[ 0 ] ); ++i )
    {
        const GUCEF::CORE::CString path = GUCEF::CORE::RelativePath( GUCEF::CORE::CombinePath( folders[ i ], defaultName ) );
        if ( GUCEF::CORE::FileExists( path ) )
            return path;
    }
    return GUCEF::CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

/**
 *  The tool had another name before (AssetViewer). When the settings file of the user of that name is there and the one of the
 *  current name is not, the old one is taken over once: the name of the tool inside the file follows the new name.
 */
void
TakeOverSettingsOfFormerName( const GUCEF::CORE::CString& appName )
{GUCEF_TRACE;

    const GUCEF::CORE::CString formerName = "AssetViewer";
    const GUCEF::CORE::CString settingsFile = GUCEF::CORE::RelativePath( "$APPCONFIGDIR$/" + appName + '/' + appName + ".json" );
    if ( GUCEF::CORE::FileExists( settingsFile ) )
        return;
    const GUCEF::CORE::CString formerFile = GUCEF::CORE::RelativePath( "$APPCONFIGDIR$/" + formerName + '/' + formerName + ".json" );
    if ( !GUCEF::CORE::FileExists( formerFile ) )
        return;

    GUCEF::CORE::CString content;
    if ( !GUCEF::CORE::LoadTextFileAsString( formerFile, content ) ||
         !GUCEF::CORE::CreatePathDirectories( settingsFile ) ||
         !GUCEF::CORE::WriteStringAsTextFile( settingsFile, content.ReplaceSubstr( formerName, appName ) ) )
    {
        GUCEF_WARNING_LOG( GUCEF::CORE::LOGLEVEL_IMPORTANT, appName << ": The settings of the former name " << formerFile << " cannot be taken over" );
        return;
    }
    GUCEF_LOG( GUCEF::CORE::LOGLEVEL_IMPORTANT, appName << ": No settings of this name yet, took over the settings of the former name " << formerFile << " into " << settingsFile );
}

}

/*-------------------------------------------------------------------------*/

GUCEF_OSMAIN_BEGIN
{GUCEF_TRACE;

    // the systems whose plugins the bootstrap config loads
    GUCEF::CORE::CCoreGlobal::Instance();
    GUCEF::VFS::CVfsGlobal::Instance();
    GUCEF::IMAGE::CImageGlobal::Instance();
    GUCEF::GUI::CGuiGlobal::Instance();
    GUCEF::INPUT::CInputGlobal::Instance();
    GUCEF::COMCORE::CComCoreGlobal::Instance();
    GUCEF::ASSETSTORE::CAssetStoreGlobal::Instance();

    GUCEF::ASSETWORKBENCH::CWorkbenchApp workbench( argc, argv );
    if ( !workbench.ParseCommandLine() )
    {
        // the settings are not loaded yet and the console log is off in them: the person who asked for the usage gets it on the console of the terminal
        GUCEF::CORE::CLogManager& logManager = GUCEF::CORE::CCoreGlobal::Instance()->GetLogManager();
        logManager.SetNativeConsoleLoggerInOwnWindow( false );
        logManager.SetNativeConsoleLoggerEnabled( true );
        GUCEF::ASSETWORKBENCH::CWorkbenchApp::LogUsage();
        return 1;
    }
    workbench.RegisterEventHandlers();

    // the plugins and the settings
    #ifdef GUCEF_DEBUG_MODE
    const GUCEF::CORE::CString bootstrapName = GUCEF::ASSETWORKBENCH::CWorkbenchApp::AppName + "_bootstrap_d.ini";
    #else
    const GUCEF::CORE::CString bootstrapName = GUCEF::ASSETWORKBENCH::CWorkbenchApp::AppName + "_bootstrap.ini";
    #endif
    GUCEF::CORE::CConfigStore& configStore = GUCEF::CORE::CCoreGlobal::Instance()->GetConfigStore();
    const GUCEF::CORE::CString bootstrapPath = LookForConfigFile( workbench.GetBootstrapConfigFile(), bootstrapName );
    // the settings: those of the user where the bootstrap config says, unless a settings file is given
    const GUCEF::CORE::CString configPath = workbench.GetConfigFile().IsNULLOrEmpty() ? GUCEF::CORE::CString::Empty : LookForConfigFile( workbench.GetConfigFile(), GUCEF::ASSETWORKBENCH::CWorkbenchApp::AppName + ".json" );
    if ( !bootstrapPath.IsNULLOrEmpty() )
        configStore.SetBootstrapConfigFile( bootstrapPath );
    else
        GUCEF_WARNING_LOG( GUCEF::CORE::LOGLEVEL_IMPORTANT, GUCEF::ASSETWORKBENCH::CWorkbenchApp::AppName << ": There is no " << bootstrapName << ", no plugins are loaded" );
    if ( !configPath.IsNULLOrEmpty() )
        configStore.SetConfigFile( configPath );
    else
        TakeOverSettingsOfFormerName( GUCEF::ASSETWORKBENCH::CWorkbenchApp::AppName );
    if ( !configStore.LoadConfig() )
    {
        GUCEF_CONSOLE_LOG( GUCEF::CORE::LOGLEVEL_IMPORTANT, "The config of the workbench could not be loaded (" << bootstrapPath << ", " << configPath << ")" );
        return 2;
    }

    // the log of the engine
    GUCEF::CORE::CCoreGlobal::Instance()->GetLogManager().SetMinLogLevel( workbench.GetMinimalLogLevel() );
    GUCEF::CORE::CFileAccess* logAccess = GUCEF_NULL;
    GUCEF::CORE::CStdLogger* logger = GUCEF_NULL;
    if ( !workbench.GetLogFile().IsNULLOrEmpty() )
    {
        logAccess = new GUCEF::CORE::CFileAccess( GUCEF::CORE::RelativePath( workbench.GetLogFile() ), "w" );
        logger = new GUCEF::CORE::CStdLogger( *logAccess );
        GUCEF::CORE::CCoreGlobal::Instance()->GetLogManager().AddLogger( logger );
    }

    // the console log as the settings have it, from now on; what was logged so far goes to the loggers there are
    workbench.ApplyLogSettings();
    GUCEF::CORE::CCoreGlobal::Instance()->GetLogManager().FlushBootstrapLogEntriesToLogs();

    // the application starts (the workbench starts with its first cycle), and is pulsed from here until the workbench has had enough: the
    // windows and the GUI cannot be taken down inside a pulse of the application, and Ogre goes with the stop of the application
    GUCEF::CORE::CCoreGlobal::Instance()->GetApplication().main( argc, argv, false );
    GUCEF::CORE::PulseGeneratorPtr pulseGenerator = GUCEF::CORE::CCoreGlobal::Instance()->GetPulseGenerator();
    while ( !workbench.IsFinished() && !GUCEF::CORE::CGUCEFApplication::IsStopRequestedFromSignal() )
    {
        pulseGenerator->ForceDirectPulse();
        GUCEF::MT::PrecisionDelay( 4 );
    }
    workbench.Shutdown();
    GUCEF::CORE::CCoreGlobal::Instance()->GetApplication().Stop();

    if ( GUCEF_NULL != logger )
    {
        GUCEF::CORE::CCoreGlobal::Instance()->GetLogManager().RemoveLogger( logger );
        delete logger;
        delete logAccess;
    }
    return workbench.GetExitCode();
}
GUCEF_OSMAIN_END
