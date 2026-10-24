/*
 *  gucefLOBBYSERVER: The standalone GUCEF lobby server
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

#ifndef GUCEF_CORE_CPLUGINCONTROL_H
#include "CPluginControl.h"
#define GUCEF_CORE_CPLUGINCONTROL_H
#endif /* GUCEF_CORE_CPLUGINCONTROL_H ? */

#ifndef GUCEF_CORE_CFILEACCESS_H
#include "CFileAccess.h"
#define GUCEF_CORE_CFILEACCESS_H
#endif /* GUCEF_CORE_CFILEACCESS_H ? */

#ifndef GUCEF_CORE_CVALUELIST_H
#include "CValueList.h"
#define GUCEF_CORE_CVALUELIST_H
#endif /* GUCEF_CORE_CVALUELIST_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CPLATFORMNATIVECONSOLELOGGER_H
#include "gucefCORE_CPlatformNativeConsoleLogger.h"
#define GUCEF_CORE_CPLATFORMNATIVECONSOLELOGGER_H
#endif /* GUCEF_CORE_CPLATFORMNATIVECONSOLELOGGER_H ? */

#ifndef GUCEF_MULTIPLAY_CMULTIPLAYGLOBAL_H
#include "gucefMULTIPLAY_CMultiplayGlobal.h"
#define GUCEF_MULTIPLAY_CMULTIPLAYGLOBAL_H
#endif /* GUCEF_MULTIPLAY_CMULTIPLAYGLOBAL_H ? */

#ifndef GUCEF_MULTIPLAY_CGUCEFLOBBYSERVERAPP_H
#include "gucefMULTIPLAY_CGucefLobbyServerApp.h"
#define GUCEF_MULTIPLAY_CGUCEFLOBBYSERVERAPP_H
#endif /* GUCEF_MULTIPLAY_CGUCEFLOBBYSERVERAPP_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

/** Loads a plugin of the directory of the executable, the debug or the release build of it */
static bool
LoadPlugin( const CORE::CString& pluginName )
{GUCEF_TRACE;

    #ifdef GUCEF_DEBUG_MODE
    CORE::CString firstPath = "$MODULEDIR$/" + pluginName + "_d";
    CORE::CString secondPath = "$MODULEDIR$/" + pluginName;
    #else
    CORE::CString firstPath = "$MODULEDIR$/" + pluginName;
    CORE::CString secondPath = "$MODULEDIR$/" + pluginName + "_d";
    #endif
    CORE::CPluginControl& pluginControl = CORE::CCoreGlobal::Instance()->GetPluginControl();
    return pluginControl.AddPluginFromDir( firstPath, "codecs", true ) || pluginControl.AddPluginFromDir( secondPath, "codecs", true );
}

/*-------------------------------------------------------------------------*/

/** The arguments: config=<path of the config file> */
static void
ParseParams( const int argc, char* argv[], CORE::CValueList& keyValueList )
{GUCEF_TRACE;

    keyValueList.DeleteAll();
    for ( int i=1; i<argc; ++i )
        keyValueList.SetMultiple( CORE::CString( argv[ i ] ), ' ' );
}

/*-------------------------------------------------------------------------*/

static void
LobbyServerSignalHandler( int signal )
{GUCEF_TRACE;

    CORE::CCoreGlobal::Instance()->GetApplication().Stop();
}

/*-------------------------------------------------------------------------*/

GUCEF_OSMAIN_BEGIN
{GUCEF_TRACE;

    CORE::CCoreGlobal::Instance();
    MULTIPLAY::CMultiplayGlobal::Instance();

    CORE::CString logFilename = CORE::CombinePath( CORE::RelativePath( "$CURWORKDIR$" ), "gucefLOBBYSERVER_Log.txt" );
    CORE::CFileAccess logFileAccess( logFilename, "w" );
    CORE::CStdLogger logger( logFileAccess );
    CORE::CCoreGlobal::Instance()->GetLogManager().AddLogger( &logger );
    CORE::CPlatformNativeConsoleLogger console;
    if ( GUCEF_APP_TYPE == GUCEF_APP_TYPE_CONSOLE )
        CORE::CCoreGlobal::Instance()->GetLogManager().AddLogger( console.GetLogger() );
    CORE::CCoreGlobal::Instance()->GetLogManager().FlushBootstrapLogEntriesToLogs();

    CORE::CValueList params;
    ParseParams( argc, argv, params );
    CORE::CString configPath = params.GetValueAlways( "config", "$MODULEDIR$/gucefLOBBYSERVER.json" ).AsString();

    LoadPlugin( "dstorepluginJSONPARSER" );
    LoadPlugin( "dstorepluginPARSIFALXML" );
    // The server signs its tokens with ed25519 and its node encrypts with x25519 and an AEAD cipher
    LoadPlugin( "codecspluginMONOCYPHER" );

    int result = 1;
    {
        MULTIPLAY::CGucefLobbyServerApp app;
        if ( app.LoadConfigFile( configPath ) )
        {
            GUCEF_OSMAIN_SIGNAL_HANDLER( LobbyServerSignalHandler );
            result = app.Run( argc, argv );
        }
        else
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_CRITICAL, "gucefLOBBYSERVER: Could not use the config file " << configPath );
        }
    }

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "gucefLOBBYSERVER: Exits with " << result );
    CORE::CCoreGlobal::Instance()->GetPluginControl().UnloadPluginGroup( "codecs" );
    CORE::CCoreGlobal::Instance()->GetLogManager().ClearLoggers();
    return result;
}
GUCEF_OSMAIN_END

/*-------------------------------------------------------------------------*/
