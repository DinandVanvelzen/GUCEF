/*
 *  gucefIMAGE: GUCEF module providing image utilities
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

#define GUCEF_TESTFW_IMPL

#include <string.h>

#ifndef GUCEF_TESTFW_H
#include "gucef_test_framework.h"
#define GUCEF_TESTFW_H
#endif /* GUCEF_TESTFW_H ? */

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_IMAGE_H
#include "gucefIMAGE.h"
#define GUCEF_IMAGE_H
#endif /* GUCEF_IMAGE_H ? */

#include "TestImageAnimation.h"
#include "TestPixelMapConversion.h"
#include "TestImageUpscaler.h"
#include "TestImageCodecPlugins.h"

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
            configFilePath = CORE::CombinePath( "$MODULEDIR$/../../../TestData/gucefIMAGE_TestApp/mswin", configFile );
            #elif GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX
            configFilePath = CORE::CombinePath( "$MODULEDIR$/../../TestData/gucefIMAGE_TestApp/linux", configFile );
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
 *      Manual test code which expects the original game data of Into the Void at hardcoded locations
 */
int
RunManualTests( int argc, char** argv )
{GUCEF_TRACE;

    #ifdef GUCEF_IMAGE_DEBUG_MODE
    //GUCEF::CORE::GUCEF_LogStackToStdOut();
    //GUCEF::CORE::GUCEF_SetStackLogging( 1 );
    #endif /* GUCEF_IMAGE_DEBUG_MODE ? */
               
    try 
    {                               

        GUCEF::CORE::CString logFilename = GUCEF::CORE::RelativePath( "$CURWORKDIR$" );
        GUCEF::CORE::AppendToPath( logFilename, "gucefIMAGE_TestApp_Log.txt" );
        GUCEF::CORE::CFileAccess logFileAccess( logFilename, "w" );
        
        GUCEF::CORE::CStdLogger logger( logFileAccess );
        GUCEF::CORE::CCoreGlobal::Instance()->GetLogManager().AddLogger( &logger );
        
        GUCEF::CORE::CPlatformNativeConsoleLogger consoleOut;
        GUCEF::CORE::CCoreGlobal::Instance()->GetLogManager().AddLogger( consoleOut.GetLogger() );

        GUCEF::IMAGE::CImageGlobal::Instance();
        
        LoadConfig();
        
        GUCEF::IMAGE::CImage image;
        image.Load( "C:\\CODE\\intovoid\\RES\\IV_SCRB_RES\\870501.1" );

        //GUCEF::IMAGE::CPixelMap::TColorCounters colorCountMap;
        //image.GetPixelMap()->GetColorCounters( colorCountMap );

        GUCEF::IMAGE::CImage palette;
        palette.Load( "C:\\CODE\\intovoid\\RES\\IV_PAL_RES\\100.5" );
        palette.GetPixelMap()->ApplyGamma( 0.2 );

        //GUCEF::IMAGE::CPixelMap::TColorCounters colorCountMap2;
        //palette.GetPixelMap()->GetColorCounters( colorCountMap2 );

        //
        //const IMAGE::CPixel& lowPixel = (*colorCountMap.begin()).first;
        //const IMAGE::CPixel& highPixel = (*colorCountMap.rbegin()).first;
        //
        //CORE::UInt8 lowIndex = *(CORE::UInt8*) lowPixel.GetDataPtr();
        //CORE::UInt8 highIndex = *(CORE::UInt8*) highPixel.GetDataPtr();
        //
        //GUCEF::IMAGE::CPixelMap::TColorCounters::iterator i = colorCountMap.begin();        
        //while ( i != colorCountMap.end() )
        //{
        //    CORE::UInt8 index = *(CORE::UInt8*) (*i).first.GetDataPtr();
        //    if ( index > highIndex )
        //        highIndex = index;
        //    if ( index < lowIndex )
        //        lowIndex = index;
        //    ++i;
        //}

        //CORE::CFileAccess imageDump;
        //imageDump.Open( "C:\\CODE\\intovoid\\RES\\IV_SCRB_RES\\700000.1.raw", "wb" );
        //image.GetPixelMap()->CopyTo( imageDump );
        //imageDump.Close();

        //CORE::CFileAccess paletteDump;
        //paletteDump.Open( "C:\\CODE\\intovoid\\RES\\IV_PAL_RES\\100.5.raw", "wb" );
        //palette.GetPixelMap()->CopyTo( paletteDump );
        //paletteDump.Close();

        //image.GetPixelMap()->ConvertFormatTo( GUCEF::IMAGE::PSF_RGB );
        
        image.GetPixelMap()->ApplyPalette( palette.GetPixelMap() );

        image.Save( "C:\\CODE\\intovoid\\RES\\IV_SCRB_RES\\870501.png" );
        
        return 1;                                                                            
    }
    catch ( ... )
    {
        #ifdef GUCEF_IMAGE_DEBUG_MODE
        GUCEF_PrintCallstack();
        GUCEF_DumpCallstack( "gucefIMAGE_TestApp_callstack.txt" );
        #endif /* GUCEF_IMAGE_DEBUG_MODE ? */
        
        GUCEF::CORE::ShowErrorMessage( "Unknown exception"                                                                 ,
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
        }

        CORE::CString logFilename = CORE::RelativePath( "$CURWORKDIR$" );
        CORE::AppendToPath( logFilename, "gucefIMAGE_TestApp_Log.txt" );
        CORE::CFileAccess logFileAccess( logFilename, "w" );

        CORE::CStdLogger logger( logFileAccess );
        CORE::CCoreGlobal::Instance()->GetLogManager().AddLogger( &logger );

        CORE::CPlatformNativeConsoleLogger console;
        if ( GUCEF_APP_TYPE == GUCEF_APP_TYPE_CONSOLE )
            CORE::CCoreGlobal::Instance()->GetLogManager().AddLogger( console.GetLogger() );

        // initializes the module, as every application using gucefIMAGE has to do
        IMAGE::CImageGlobal::Instance();

        GUCEF_TESTFW_INIT( "gucefIMAGE_TestApp" );
        GUCEF_TESTFW_SET_CALLBACK( CORE::TestFrameworkLogCallback, GUCEF_NULL );

        PerformImageAnimationTests();
        PerformPixelMapConversionTests();
        PerformImageUpscalerTests();
        PerformImageCodecPluginTests();

        CORE::CLogStreamScope::FlushLogs();

        CORE::CString xmlPath = CORE::RelativePath( "$CURWORKDIR$" );
        CORE::AppendToPath( xmlPath, "gucefIMAGE_TestApp_Results.xml" );
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