/*
 *  ImageConvertor: converts images between the formats of the loaded image codecs
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

#ifndef GUCEF_IMAGE_CIMAGEGLOBAL_H
#include "gucefIMAGE_CImageGlobal.h"
#define GUCEF_IMAGE_CIMAGEGLOBAL_H
#endif /* GUCEF_IMAGE_CIMAGEGLOBAL_H ? */

#ifndef GUCEF_OSMAIN_H
#include "gucef_osmain.h"
#define GUCEF_OSMAIN_H
#endif /* GUCEF_OSMAIN_H ? */

#ifndef IMAGECONVERTOR_CIMAGECONVERTORAPP_H
#include "ImageConvertor_CImageConvertorApp.h"
#define IMAGECONVERTOR_CIMAGECONVERTORAPP_H
#endif /* IMAGECONVERTOR_CIMAGECONVERTORAPP_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

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
    GUCEF::IMAGE::CImageGlobal::Instance();

    // the console, as the GUCEF_CONSOLE_LOGGER environment variable allows
    GUCEF::CORE::CPlatformNativeConsoleLogger console;
    GUCEF::CORE::CCoreGlobal::Instance()->GetLogManager().AddLogger( console.GetLogger() );

    GUCEF::Int32 exitCode = GUCEF::IMAGECONVERTOR::CImageConvertorApp::ExitUsage;
    {
        GUCEF::IMAGECONVERTOR::CImageConvertorApp convertor( argc, argv );
        if ( !convertor.ParseCommandLine() )
        {
            GUCEF::IMAGECONVERTOR::CImageConvertorApp::LogUsage();
            GUCEF::CORE::CCoreGlobal::Instance()->GetLogManager().FlushBootstrapLogEntriesToLogs();
            GUCEF::CORE::CCoreGlobal::Instance()->GetLogManager().RemoveLogger( console.GetLogger() );
            return exitCode;
        }

        // the plugins
        #ifdef GUCEF_DEBUG_MODE
        const GUCEF::CORE::CString bootstrapName = GUCEF::IMAGECONVERTOR::CImageConvertorApp::AppName + "_bootstrap_d.ini";
        #else
        const GUCEF::CORE::CString bootstrapName = GUCEF::IMAGECONVERTOR::CImageConvertorApp::AppName + "_bootstrap.ini";
        #endif
        GUCEF::CORE::CCoreGlobal::Instance()->GetLogManager().SetMinLogLevel( convertor.GetMinimalLogLevel() );
        GUCEF::CORE::CConfigStore& configStore = GUCEF::CORE::CCoreGlobal::Instance()->GetConfigStore();
        const GUCEF::CORE::CString bootstrapPath = LookForConfigFile( convertor.GetBootstrapConfigFile(), bootstrapName );
        if ( !bootstrapPath.IsNULLOrEmpty() )
            configStore.SetBootstrapConfigFile( bootstrapPath );
        else
            GUCEF_WARNING_LOG( GUCEF::CORE::LOGLEVEL_IMPORTANT, GUCEF::IMAGECONVERTOR::CImageConvertorApp::AppName << ": There is no " << bootstrapName << ", no plugins are loaded" );
        if ( !configStore.LoadConfig() )
        {
            GUCEF_CONSOLE_LOG( GUCEF::CORE::LOGLEVEL_CRITICAL, "The bootstrap configuration of the convertor could not be loaded (" << bootstrapPath << ")" );
            GUCEF::CORE::CCoreGlobal::Instance()->GetLogManager().RemoveLogger( console.GetLogger() );
            return GUCEF::IMAGECONVERTOR::CImageConvertorApp::ExitConfig;
        }

        GUCEF::CORE::CCoreGlobal::Instance()->GetLogManager().FlushBootstrapLogEntriesToLogs();

        // the task manager runs the conversions on the threads of a pool: this thread waits for them
        GUCEF::CORE::CCoreGlobal::Instance()->GetApplication().main( argc, argv, false );
        exitCode = convertor.Run();
        GUCEF::CORE::CCoreGlobal::Instance()->GetApplication().Stop();
    }
    GUCEF::CORE::CCoreGlobal::Instance()->GetLogManager().RemoveLogger( console.GetLogger() );
    return exitCode;
}
GUCEF_OSMAIN_END
