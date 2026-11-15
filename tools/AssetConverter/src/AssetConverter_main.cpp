/*
 *  AssetConverter: converts asset packs into the engine-neutral intermediate and cooks it
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

#ifndef GUCEF_OSMAIN_H
#include "gucef_osmain.h"
#define GUCEF_OSMAIN_H
#endif /* GUCEF_OSMAIN_H ? */

#ifndef ASSETCONVERTERLIB_CASSETCONVERTERGLOBAL_H
#include "AssetConverterLib_CAssetConverterGlobal.h"
#define ASSETCONVERTERLIB_CASSETCONVERTERGLOBAL_H
#endif /* ASSETCONVERTERLIB_CASSETCONVERTERGLOBAL_H ? */

#ifndef ASSETCONVERTERLIB_CASSETCONVERTERAPP_H
#include "AssetConverterLib_CAssetConverterApp.h"
#define ASSETCONVERTERLIB_CASSETCONVERTERAPP_H
#endif /* ASSETCONVERTERLIB_CASSETCONVERTERAPP_H ? */

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
    GUCEF::ASSETCONVERTER::CAssetConverterGlobal::Instance();

    // the console, as the GUCEF_CONSOLE_LOGGER environment variable allows
    GUCEF::CORE::CPlatformNativeConsoleLogger console;
    GUCEF::CORE::CCoreGlobal::Instance()->GetLogManager().AddLogger( console.GetLogger() );

    GUCEF::Int32 exitCode = GUCEF::ASSETCONVERTER::CAssetConverterApp::ExitUsage;
    {
        GUCEF::ASSETCONVERTER::CAssetConverterApp converter( argc, argv );
        if ( !converter.ParseCommandLine() )
        {
            GUCEF::ASSETCONVERTER::CAssetConverterApp::LogUsage();
            GUCEF::CORE::CCoreGlobal::Instance()->GetLogManager().FlushBootstrapLogEntriesToLogs();
            GUCEF::CORE::CCoreGlobal::Instance()->GetLogManager().RemoveLogger( console.GetLogger() );
            return exitCode;
        }

        // the plugins and the settings
        #ifdef GUCEF_DEBUG_MODE
        const GUCEF::CORE::CString bootstrapName = GUCEF::ASSETCONVERTER::CAssetConverterApp::AppName + "_bootstrap_d.ini";
        #else
        const GUCEF::CORE::CString bootstrapName = GUCEF::ASSETCONVERTER::CAssetConverterApp::AppName + "_bootstrap.ini";
        #endif
        GUCEF::CORE::CConfigStore& configStore = GUCEF::CORE::CCoreGlobal::Instance()->GetConfigStore();
        const GUCEF::CORE::CString bootstrapPath = LookForConfigFile( converter.GetBootstrapConfigFile(), bootstrapName );
        const GUCEF::CORE::CString configPath = converter.GetConfigFile().IsNULLOrEmpty() ? GUCEF::CORE::CString::Empty : LookForConfigFile( converter.GetConfigFile(), GUCEF::ASSETCONVERTER::CAssetConverterApp::AppName + ".json" );
        if ( !bootstrapPath.IsNULLOrEmpty() )
            configStore.SetBootstrapConfigFile( bootstrapPath );
        else
            GUCEF_WARNING_LOG( GUCEF::CORE::LOGLEVEL_IMPORTANT, GUCEF::ASSETCONVERTER::CAssetConverterApp::AppName << ": There is no " << bootstrapName << ", no plugins are loaded" );
        if ( !configPath.IsNULLOrEmpty() )
            configStore.SetConfigFile( configPath );
        if ( !configStore.LoadConfig() )
        {
            GUCEF_CONSOLE_LOG( GUCEF::CORE::LOGLEVEL_IMPORTANT, "The config of the converter could not be loaded (" << bootstrapPath << ", " << configPath << ")" );
            GUCEF::CORE::CCoreGlobal::Instance()->GetLogManager().RemoveLogger( console.GetLogger() );
            return GUCEF::ASSETCONVERTER::CAssetConverterApp::ExitConfig;
        }

        // the log file of the run
        GUCEF::CORE::CCoreGlobal::Instance()->GetLogManager().SetMinLogLevel( converter.GetMinimalLogLevel() );
        GUCEF::CORE::CFileAccess* logAccess = GUCEF_NULL;
        GUCEF::CORE::CStdLogger* logger = GUCEF_NULL;
        if ( !converter.GetLogFile().IsNULLOrEmpty() )
        {
            logAccess = new GUCEF::CORE::CFileAccess( GUCEF::CORE::RelativePath( converter.GetLogFile() ), "w" );
            logger = new GUCEF::CORE::CStdLogger( *logAccess );
            GUCEF::CORE::CCoreGlobal::Instance()->GetLogManager().AddLogger( logger );
        }
        GUCEF::CORE::CCoreGlobal::Instance()->GetLogManager().FlushBootstrapLogEntriesToLogs();

        // the application runs while the pipeline does: its tasks are on the threads of the pool, this thread waits for them
        GUCEF::CORE::CCoreGlobal::Instance()->GetApplication().main( argc, argv, false );
        exitCode = converter.Run();
        converter.Unmount();
        GUCEF::CORE::CCoreGlobal::Instance()->GetApplication().Stop();

        if ( GUCEF_NULL != logger )
        {
            GUCEF::CORE::CCoreGlobal::Instance()->GetLogManager().RemoveLogger( logger );
            delete logger;
            delete logAccess;
        }
    }
    GUCEF::CORE::CCoreGlobal::Instance()->GetLogManager().RemoveLogger( console.GetLogger() );
    return exitCode;
}
GUCEF_OSMAIN_END
