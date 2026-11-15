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

            // its codecs are in the codec registry, before the built-in PNG encoder of gucefIMAGE
            CORE::CCodecRegistry& codecRegistry = CORE::CCoreGlobal::Instance()->GetCodecRegistry();
            CORE::CCodecRegistry::TICodecPtrVector providers;
            GUCEF_TESTFW_ASSERT_TRUE( codecRegistry.GetCodecs( "ImageCodec", "png", providers ) && 2 == providers.size() );
            GUCEF_TESTFW_ASSERT_TRUE( !providers.empty() && "png" == providers[ 0 ]->GetType() );
            GUCEF_TESTFW_ASSERT_TRUE( !providers.empty() && "GUCEF::IMAGE::CPngImageCodec" != providers[ 0 ]->GetClassTypeName() );
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
            // only the built-in PNG encoder is left
            providers.clear();
            GUCEF_TESTFW_ASSERT_TRUE( codecRegistry.GetCodecs( "ImageCodec", "png", providers ) && 1 == providers.size() );
            GUCEF_TESTFW_ASSERT_TRUE( !providers.empty() && "GUCEF::IMAGE::CPngImageCodec" == providers[ 0 ]->GetClassTypeName() );
            providers.clear();
        }
        catch ( ... )
        {
            GUCEF_TESTFW_ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

/** A 9 x 4 RGB picture whose pixels differ: its rows are 27 bytes, not a multiple of 4 */
static IMAGE::TPixelMapPtr
MakeTestPicture( void )
{GUCEF_TRACE;

    IMAGE::TPixelMapPtr pixels( new IMAGE::CPixelMap( GUCEF_NULL, 9, 4, IMAGE::PSF_RGB, GUCEF::DATATYPE_UINT8 ) );
    CORE::UInt8* data = static_cast< CORE::UInt8* >( pixels->GetDataPtr() );
    for ( CORE::UInt32 i=0; i<pixels->GetTotalSizeInBytes(); ++i )
        data[ i ] = (CORE::UInt8) ( 3 + i * 5 );
    return pixels;
}

/*-------------------------------------------------------------------------*/

/** The PNG of MakeTestPicture(), made by the built-in encoder */
static bool
MakeTestPng( CORE::CDynamicBuffer& png )
{GUCEF_TRACE;

    CORE::CString problem;
    return IMAGE::CPngImageCodec::EncodePixelMap( *MakeTestPicture(), png, problem );
}

/*-------------------------------------------------------------------------*/

/** Whether a decoded picture has the pixels of the RGB picture, in RGB or BGR order */
static bool
HasPixelsOf( const IMAGE::CPixelMap& decoded, const IMAGE::CPixelMap& rgb )
{GUCEF_TRACE;

    const bool bgr = IMAGE::PSF_BGR == decoded.GetPixelStorageFormat();
    if ( ( !bgr && IMAGE::PSF_RGB != decoded.GetPixelStorageFormat() ) ||
         decoded.GetWidthInPixels() != rgb.GetWidthInPixels() || decoded.GetHeightInPixels() != rgb.GetHeightInPixels() ||
         decoded.GetTotalSizeInBytes() != rgb.GetTotalSizeInBytes() )
        return false;
    const CORE::UInt8* a = static_cast< const CORE::UInt8* >( decoded.GetDataPtr() );
    const CORE::UInt8* b = static_cast< const CORE::UInt8* >( rgb.GetDataPtr() );
    for ( CORE::UInt32 i=0; i<rgb.GetTotalSizeInBytes(); i+=3 )
        for ( CORE::UInt32 c=0; c<3; ++c )
            if ( a[ i + ( bgr ? 2 - c : c ) ] != b[ i + c ] )
                return false;
    return true;
}

/*-------------------------------------------------------------------------*/

/** Loads, with meta-data, saves as PNG and loads that again: what the users of CImage do with a picture */
static void
CheckLoadWithMetaDataAndSavePng( const CORE::CString& pluginUse )
{GUCEF_TRACE;

    CORE::CDynamicBuffer png;
    GUCEF_TESTFW_ASSERT_TRUE( MakeTestPng( png ) );

    // the pixels and the meta-data, from a stream and from a file
    CORE::CDynamicBufferAccess pngAccess( &png, false );
    IMAGE::CImage loaded;
    GUCEF_TESTFW_ASSERT_TRUE( loaded.Load( pngAccess, "png", true, true ) );
    const IMAGE::TPixelMapPtr original = MakeTestPicture();
    IMAGE::TPixelMapPtr pixels;
    GUCEF_TESTFW_ASSERT_TRUE( loaded.TryGetPixelMap( 0, 0, pixels ) && !pixels.IsNULL() );
    if ( !pixels.IsNULL() )
        GUCEF_TESTFW_ASSERT_TRUE( HasPixelsOf( *pixels, *original ) );

    const CORE::CString file = CORE::CombinePath( CORE::TempDir(), "gucefIMAGE_TestApp_" + pluginUse + ".png" );
    GUCEF_TESTFW_ASSERT_TRUE( png.WriteContentToFile( file, 0, true ) );
    IMAGE::CImage fromFile;
    GUCEF_TESTFW_ASSERT_TRUE( fromFile.Load( file ) );
    GUCEF_TESTFW_ASSERT_TRUE( 1 == fromFile.GetFrameCount() );

    // saved as PNG, which reads back with the same pixels; saving leaves the pixels of the image as they are
    {
        IMAGE::CImage image( IMAGE::TPixelMapPtr( new IMAGE::CPixelMap( *original ) ) );
        CORE::CDynamicBuffer saved;
        CORE::CDynamicBufferAccess savedAccess( &saved, false );
        GUCEF_TESTFW_ASSERT_TRUE( image.Save( savedAccess, "png" ) );
        GUCEF_TESTFW_ASSERT_TRUE( saved.GetDataSize() > 8 && 0 == memcmp( saved.GetConstBufferPtr(), "\x89PNG\r\n\x1a\n", 8 ) );
        savedAccess.Setpos( 0 );
        IMAGE::CImage reloaded;
        GUCEF_TESTFW_ASSERT_TRUE( reloaded.Load( savedAccess, "png", true, true ) );
        IMAGE::TPixelMapPtr reloadedPixels;
        GUCEF_TESTFW_ASSERT_TRUE( reloaded.TryGetPixelMap( 0, 0, reloadedPixels ) && !reloadedPixels.IsNULL() );
        if ( !reloadedPixels.IsNULL() )
            GUCEF_TESTFW_ASSERT_TRUE( HasPixelsOf( *reloadedPixels, *original ) );
        IMAGE::TPixelMapPtr imagePixels;
        GUCEF_TESTFW_ASSERT_TRUE( image.TryGetPixelMap( 0, 0, imagePixels ) && !imagePixels.IsNULL() && HasPixelsOf( *imagePixels, *original ) );

        // and to a file
        const CORE::CString savedFile = CORE::CombinePath( CORE::TempDir(), "gucefIMAGE_TestApp_" + pluginUse + "_saved.png" );
        GUCEF_TESTFW_ASSERT_TRUE( image.Save( savedFile, "png" ) );
        IMAGE::CImage savedFromFile;
        GUCEF_TESTFW_ASSERT_TRUE( savedFromFile.Load( savedFile ) && 1 == savedFromFile.GetFrameCount() );
        IMAGE::TPixelMapPtr savedPixels;
        GUCEF_TESTFW_ASSERT_TRUE( savedFromFile.TryGetPixelMap( 0, 0, savedPixels ) && !savedPixels.IsNULL() && HasPixelsOf( *savedPixels, *original ) );
        CORE::DeleteFile( savedFile );
    }

    // the decoded picture, in the channel order of the codec, saved twice: the same pixels both times
    if ( !pixels.IsNULL() )
    {
        IMAGE::CImage decodedImage( pixels );
        for ( CORE::UInt32 i=0; i<2; ++i )
        {
            CORE::CDynamicBuffer saved;
            CORE::CDynamicBufferAccess savedAccess( &saved, false );
            GUCEF_TESTFW_ASSERT_TRUE( decodedImage.Save( savedAccess, "png" ) );
            savedAccess.Setpos( 0 );
            IMAGE::CImage reloaded;
            IMAGE::TPixelMapPtr reloadedPixels;
            GUCEF_TESTFW_ASSERT_TRUE( reloaded.Load( savedAccess, "png", true, false ) && reloaded.TryGetPixelMap( 0, 0, reloadedPixels ) &&
                                      !reloadedPixels.IsNULL() && HasPixelsOf( *reloadedPixels, *original ) );
            GUCEF_TESTFW_ASSERT_TRUE( HasPixelsOf( *pixels, *original ) );
        }
    }
    CORE::DeleteFile( file );
}

/*-------------------------------------------------------------------------*/

static void
TestImageLoadWithMetaDataAndSavePng( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "CImage loads with meta-data and saves PNG through imgpluginFreeImage loaded as an image codec plugin" )
        try
        {
            IMAGE::CImageGlobal::Instance();
            GUCEF_TESTFW_ASSERT_TRUE( LoadImageCodecPlugin( "imgpluginFreeImage" ) );
            GUCEF_TESTFW_ASSERT_TRUE( IMAGE::CImageGlobal::Instance()->GetImageCodecRegistry().IsRegistered( "png" ) );
            CheckLoadWithMetaDataAndSavePng( "typed" );
            GUCEF_TESTFW_ASSERT_TRUE( CORE::CCoreGlobal::Instance()->GetPluginControl().UnloadPluginGroup( "imagecodecplugintests" ) );
        }
        catch ( ... )
        {
            GUCEF_TESTFW_ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CImage loads with meta-data and saves PNG through imgpluginFreeImage loaded without a plugin type" )
        try
        {
            // the plugin control finds the plugin manager by trial and error, as for plugins from a directory
            IMAGE::CImageGlobal::Instance();
            CORE::CPluginControl& pluginControl = CORE::CCoreGlobal::Instance()->GetPluginControl();
            GUCEF_TESTFW_ASSERT_TRUE( pluginControl.AddPluginFromDir( "$MODULEDIR$/imgpluginFreeImage_d", "imagecodecplugintests", true ) ||
                                      pluginControl.AddPluginFromDir( "$MODULEDIR$/imgpluginFreeImage", "imagecodecplugintests", true ) );
            CheckLoadWithMetaDataAndSavePng( "untyped" );
            GUCEF_TESTFW_ASSERT_TRUE( pluginControl.UnloadPluginGroup( "imagecodecplugintests" ) );
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
    TestImageLoadWithMetaDataAndSavePng();
}

/*-------------------------------------------------------------------------*/
