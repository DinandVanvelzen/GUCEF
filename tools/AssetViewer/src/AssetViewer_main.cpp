/*
 *  AssetViewer: viewer and content browser for models, textures, materials and scenes
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

#ifndef GUCEF_OSMAIN_H
#include "gucef_osmain.h"
#define GUCEF_OSMAIN_H
#endif /* GUCEF_OSMAIN_H ? */

#ifndef ASSETVIEWERLIB_CVIEWERAPP_H
#include "AssetViewerLib_CViewerApp.h"
#define ASSETVIEWERLIB_CVIEWERAPP_H
#endif /* ASSETVIEWERLIB_CVIEWERAPP_H ? */

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

    GUCEF::ASSETVIEWER::CViewerApp viewer( argc, argv );
    if ( !viewer.ParseCommandLine() )
    {
        GUCEF::ASSETVIEWER::CViewerApp::LogUsage();
        return 1;
    }
    viewer.RegisterEventHandlers();

    // the plugins and the settings
    #ifdef GUCEF_DEBUG_MODE
    const GUCEF::CORE::CString bootstrapName = GUCEF::ASSETVIEWER::CViewerApp::AppName + "_bootstrap_d.ini";
    #else
    const GUCEF::CORE::CString bootstrapName = GUCEF::ASSETVIEWER::CViewerApp::AppName + "_bootstrap.ini";
    #endif
    GUCEF::CORE::CConfigStore& configStore = GUCEF::CORE::CCoreGlobal::Instance()->GetConfigStore();
    const GUCEF::CORE::CString bootstrapPath = LookForConfigFile( viewer.GetBootstrapConfigFile(), bootstrapName );
    // the settings: those of the user where the bootstrap config says, unless a settings file is given
    const GUCEF::CORE::CString configPath = viewer.GetConfigFile().IsNULLOrEmpty() ? GUCEF::CORE::CString::Empty : LookForConfigFile( viewer.GetConfigFile(), GUCEF::ASSETVIEWER::CViewerApp::AppName + ".json" );
    if ( !bootstrapPath.IsNULLOrEmpty() )
        configStore.SetBootstrapConfigFile( bootstrapPath );
    else
        GUCEF_WARNING_LOG( GUCEF::CORE::LOGLEVEL_IMPORTANT, GUCEF::ASSETVIEWER::CViewerApp::AppName << ": There is no " << bootstrapName << ", no plugins are loaded" );
    if ( !configPath.IsNULLOrEmpty() )
        configStore.SetConfigFile( configPath );
    if ( !configStore.LoadConfig() )
    {
        GUCEF_CONSOLE_LOG( GUCEF::CORE::LOGLEVEL_IMPORTANT, "The config of the viewer could not be loaded (" << bootstrapPath << ", " << configPath << ")" );
        return 2;
    }

    // the log of the engine
    GUCEF::CORE::CCoreGlobal::Instance()->GetLogManager().SetMinLogLevel( viewer.GetMinimalLogLevel() );
    GUCEF::CORE::CFileAccess* logAccess = GUCEF_NULL;
    GUCEF::CORE::CStdLogger* logger = GUCEF_NULL;
    if ( !viewer.GetLogFile().IsNULLOrEmpty() )
    {
        logAccess = new GUCEF::CORE::CFileAccess( GUCEF::CORE::RelativePath( viewer.GetLogFile() ), "w" );
        logger = new GUCEF::CORE::CStdLogger( *logAccess );
        GUCEF::CORE::CCoreGlobal::Instance()->GetLogManager().AddLogger( logger );
    }

    // the console log as the settings have it, from now on; what was logged so far goes to the loggers there are
    viewer.ApplyLogSettings();
    GUCEF::CORE::CCoreGlobal::Instance()->GetLogManager().FlushBootstrapLogEntriesToLogs();

    // the application starts (the viewer starts with its first cycle), and is pulsed from here until the viewer has had enough: the
    // windows and the GUI cannot be taken down inside a pulse of the application, and Ogre goes with the stop of the application
    GUCEF::CORE::CCoreGlobal::Instance()->GetApplication().main( argc, argv, false );
    GUCEF::CORE::PulseGeneratorPtr pulseGenerator = GUCEF::CORE::CCoreGlobal::Instance()->GetPulseGenerator();
    while ( !viewer.IsFinished() && !GUCEF::CORE::CGUCEFApplication::IsStopRequestedFromSignal() )
    {
        pulseGenerator->ForceDirectPulse();
        GUCEF::MT::PrecisionDelay( 4 );
    }
    viewer.Shutdown();
    GUCEF::CORE::CCoreGlobal::Instance()->GetApplication().Stop();

    if ( GUCEF_NULL != logger )
    {
        GUCEF::CORE::CCoreGlobal::Instance()->GetLogManager().RemoveLogger( logger );
        delete logger;
        delete logAccess;
    }
    return viewer.GetExitCode();
}
GUCEF_OSMAIN_END
