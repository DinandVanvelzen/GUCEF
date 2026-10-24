/*
 *  gucefIMAGE_TestApp: Test application for the gucefIMAGE module
 *  Copyright (C) 1998 - 2026.  Dinand Vanvelzen
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

#include <string.h>
#include <stdlib.h>
#include <new>

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_IMAGE_H
#include "gucefIMAGE.h"
#define GUCEF_IMAGE_H
#endif /* GUCEF_IMAGE_H ? */

#ifndef GUCEF_IMAGE_CIMAGECODECPLUGIN_H
#include "gucefIMAGE_CImageCodecPlugin.h"
#define GUCEF_IMAGE_CIMAGECODECPLUGIN_H
#endif /* GUCEF_IMAGE_CIMAGECODECPLUGIN_H ? */

#include "TestImageCodecPlugins.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

/*-------------------------------------------------------------------------*/

static bool
LoadImageCodecPlugin( const CORE::CString& pluginName )
{GUCEF_TRACE;

    CORE::CPluginMetaData pluginMetaData;
    pluginMetaData.SetPluginType( "GucefImageCodecPlugin" );
    pluginMetaData.SetFullModulePath( "$MODULEDIR$" );
    pluginMetaData.SetModuleFilename( pluginName );
    pluginMetaData.SetAltModuleFilename( pluginName + "_d" );
    pluginMetaData.SetAllLinkBackModules( "gucefCORE,gucefIMAGE" );

    CORE::CPluginControl& pluginControl = CORE::CCoreGlobal::Instance()->GetPluginControl();
    return pluginControl.AddPluginMetaData( pluginMetaData, "imagecodecplugintests", true, true );
}

/*-------------------------------------------------------------------------*/

static void
TestUnlinkedImageCodecPlugin( void )
{GUCEF_TRACE;

    // Regression: the constructor cleared as many bytes of the image function table as it has entries,
    // so a plugin that is not linked called whatever the rest of the table held
    GUCEF_TESTFW_TESTCASE( "An image codec plugin that is not linked fails its calls" )
        try
        {
            void* memory = malloc( sizeof( IMAGE::CImageCodecPlugin ) );
            GUCEF_TESTFW_ASSERT_TRUE( GUCEF_NULL != memory );
            if ( GUCEF_NULL != memory )
            {
                memset( memory, 0xAB, sizeof( IMAGE::CImageCodecPlugin ) );
                IMAGE::CImageCodecPlugin* plugin = new( memory ) IMAGE::CImageCodecPlugin();

                CORE::CDynamicBuffer input( 16, true );
                CORE::CDynamicBufferAccess inputAccess( &input, false );
                IMAGE::CImage image;
                GUCEF_TESTFW_ASSERT_FALSE( plugin->Decode( inputAccess, image, "bmp" ) );
                CORE::CValueList metaData;
                GUCEF_TESTFW_ASSERT_FALSE( plugin->DecodeMetaData( inputAccess, metaData, "bmp" ) );

                plugin->~CImageCodecPlugin();
                free( memory );
            }
        }
        catch ( ... )
        {
            GUCEF_TESTFW_ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
TestImageCodecPluginsOnTheCodecInterface( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Image codec plugins load on the codec plugin interface and round trip an image" )
        try
        {
            IMAGE::CImageGlobal::Instance();

            // imgpluginDEVIL is left out: the DevIL library of the Linux build lacks ilSaveF, so the module does not load
            GUCEF_TESTFW_ASSERT_TRUE( LoadImageCodecPlugin( "imgpluginFreeImage" ) );

            // its codecs are in the codec registry
            CORE::CCodecRegistry& codecRegistry = CORE::CCoreGlobal::Instance()->GetCodecRegistry();
            CORE::CCodecRegistry::TICodecPtrVector providers;
            GUCEF_TESTFW_ASSERT_TRUE( codecRegistry.GetCodecs( "ImageCodec", "png", providers ) && 1 == providers.size() );
            GUCEF_TESTFW_ASSERT_TRUE( !providers.empty() && "png" == providers[ 0 ]->GetType() );
            providers.clear(); // a codec that is held keeps its plugin from unloading

            // a round trip through FreeImage
            CORE::UInt8 pixels[ 4 * 4 * 3 ];
            for ( CORE::UInt32 i=0; i<sizeof( pixels ); ++i )
                pixels[ i ] = (CORE::UInt8) ( 10 + i );
            IMAGE::CImage original( IMAGE::TPixelMapPtr( new IMAGE::CPixelMap( pixels, 4, 4, IMAGE::PSF_RGB, (IMAGE::TBuildinDataType) GUCEF_DATATYPE_UINT8 ) ) );

            CORE::CDynamicBuffer encoded;
            CORE::CDynamicBufferAccess encodedAccess( &encoded, false );
            GUCEF_TESTFW_ASSERT_TRUE( original.Save( encodedAccess, "png" ) );
            GUCEF_TESTFW_ASSERT_TRUE( encoded.GetDataSize() > 8 );

            encodedAccess.Setpos( 0 );
            IMAGE::CImage decoded;
            GUCEF_TESTFW_ASSERT_TRUE( decoded.Load( encodedAccess, "png", true, false ) );
            GUCEF_TESTFW_ASSERT_TRUE( 1 == decoded.GetFrameCount() );
            if ( 1 == decoded.GetFrameCount() )
            {
                IMAGE::TPixelMapPtr pixelMap = decoded.GetPixelMap( 0, 0 );
                GUCEF_TESTFW_ASSERT_TRUE( !pixelMap.IsNULL() && 4 == pixelMap->GetWidthInPixels() && 4 == pixelMap->GetHeightInPixels() );
            }

            CORE::CPluginControl& pluginControl = CORE::CCoreGlobal::Instance()->GetPluginControl();
            GUCEF_TESTFW_ASSERT_TRUE( pluginControl.UnloadPluginGroup( "imagecodecplugintests" ) );
            providers.clear();
            GUCEF_TESTFW_ASSERT_FALSE( codecRegistry.GetCodecs( "ImageCodec", "png", providers ) );
        }
        catch ( ... )
        {
            GUCEF_TESTFW_ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
PerformImageCodecPluginTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "Image codec plugins" );

    TestUnlinkedImageCodecPlugin();
    TestImageCodecPluginsOnTheCodecInterface();
}

/*-------------------------------------------------------------------------*/
