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

#ifndef GUCEF_IMAGE_CIMAGECODECCAPABILITIES_H
#include "gucefIMAGE_CImageCodecCapabilities.h"
#define GUCEF_IMAGE_CIMAGECODECCAPABILITIES_H
#endif /* GUCEF_IMAGE_CIMAGECODECCAPABILITIES_H ? */

#include "TestImageCodecPlugins.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

/*-------------------------------------------------------------------------*/

static bool
LoadImageCodecPlugin( const CORE::CString& pluginName, const CORE::CString& groupName = "imagecodecplugintests" )
{GUCEF_TRACE;

    CORE::CPluginMetaData pluginMetaData;
    pluginMetaData.SetPluginType( "GucefImageCodecPlugin" );
    pluginMetaData.SetFullModulePath( "$MODULEDIR$" );
    pluginMetaData.SetModuleFilename( pluginName );
    pluginMetaData.SetAltModuleFilename( pluginName + "_d" );
    pluginMetaData.SetAllLinkBackModules( "gucefCORE,gucefIMAGE" );

    CORE::CPluginControl& pluginControl = CORE::CCoreGlobal::Instance()->GetPluginControl();
    return pluginControl.AddPluginMetaData( pluginMetaData, groupName, true, true );
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

/** Whether the codec of a type that a plugin provides advertises encoding exactly when it can encode */
static void
CheckEncodeCapabilityMatchesBehaviour( const CORE::CString& type, bool shouldEncode )
{GUCEF_TRACE;

    CORE::CCodecRegistry& codecRegistry = CORE::CCoreGlobal::Instance()->GetCodecRegistry();
    CORE::CCodecRegistry::TICodecPtrVector providers;
    GUCEF_TESTFW_ASSERT_TRUE( codecRegistry.GetCodecs( "ImageCodec", type, providers ) && !providers.empty() );
    if ( providers.empty() )
        return;

    CORE::CICodec* codec = providers[ 0 ].GetPointer();
    GUCEF_TESTFW_ASSERT_TRUE( GUCEF_NULL != codec );
    if ( GUCEF_NULL == codec )
        return;

    GUCEF_TESTFW_ASSERT_TRUE( 0 != ( codec->GetCapabilities() & GUCEF_CODEC_CAPABILITY_DECODE ) );
    GUCEF_TESTFW_ASSERT_TRUE( shouldEncode == ( 0 != ( codec->GetCapabilities() & GUCEF_CODEC_CAPABILITY_ENCODE ) ) );

    CORE::CDynamicBuffer input( 64, true );
    CORE::CDynamicBufferAccess inputAccess( &input, false );
    CORE::CDynamicBuffer output;
    CORE::CDynamicBufferAccess outputAccess( &output, false );
    CORE::CValueList params;
    const bool encoded = codec->Encode( inputAccess, outputAccess, params );
    if ( !shouldEncode )
    {
        GUCEF_TESTFW_ASSERT_FALSE( encoded );
        GUCEF_TESTFW_ASSERT_TRUE( CORE::GUCEF_CODEC_STATUS_UNSUPPORTED == codec->GetLastStatus() );
    }
}

/*-------------------------------------------------------------------------*/

static void
TestImageCodecEncodeCapabilities( void )
{GUCEF_TRACE;

    // The FLIC and ITV plugins only decode and used to advertise encoding anyway
    GUCEF_TESTFW_TESTCASE( "Image codec plugins advertise encoding only when they can encode" )
        try
        {
            IMAGE::CImageGlobal::Instance();
            GUCEF_TESTFW_ASSERT_TRUE( LoadImageCodecPlugin( "imgpluginFLIC", "imagecodeccapabilitytests" ) );
            GUCEF_TESTFW_ASSERT_TRUE( LoadImageCodecPlugin( "imgpluginITV", "imagecodeccapabilitytests" ) );
            GUCEF_TESTFW_ASSERT_TRUE( LoadImageCodecPlugin( "imgpluginFreeImage", "imagecodeccapabilitytests" ) );

            CheckEncodeCapabilityMatchesBehaviour( "flic", false );
            CheckEncodeCapabilityMatchesBehaviour( "itv19", false );
            CheckEncodeCapabilityMatchesBehaviour( "itv1", false );
            CheckEncodeCapabilityMatchesBehaviour( "itv5", false );

            // the control: a plugin that does encode keeps advertising it
            CORE::CCodecRegistry::TICodecPtrVector providers;
            GUCEF_TESTFW_ASSERT_TRUE( CORE::CCoreGlobal::Instance()->GetCodecRegistry().GetCodecs( "ImageCodec", "png", providers ) && !providers.empty() );
            GUCEF_TESTFW_ASSERT_TRUE( !providers.empty() && 0 != ( providers[ 0 ]->GetCapabilities() & GUCEF_CODEC_CAPABILITY_ENCODE ) );
            providers.clear();

            GUCEF_TESTFW_ASSERT_TRUE( CORE::CCoreGlobal::Instance()->GetPluginControl().UnloadPluginGroup( "imagecodeccapabilitytests" ) );
        }
        catch ( ... )
        {
            GUCEF_TESTFW_ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
TestFreeImageWritesByFileExtension( void )
{GUCEF_TRACE;

    // The writer looked the format up by the names FreeImage gives its formats, which are not the extensions of jpg, tga and tif files
    GUCEF_TESTFW_TESTCASE( "imgpluginFreeImage writes the formats whose file extension differs from their name in FreeImage" )
        try
        {
            IMAGE::CImageGlobal::Instance();
            GUCEF_TESTFW_ASSERT_TRUE( LoadImageCodecPlugin( "imgpluginFreeImage", "imagecodecwritetests" ) );

            const char* const types[] = { "jpg", "jpeg", "tga", "tif", "tiff", "bmp", "png" };
            for ( size_t i=0; i<sizeof( types ) / sizeof( types[ 0 ] ); ++i )
            {
                IMAGE::TPixelMapPtr pixels( GUCEF_NEW IMAGE::CPixelMap( GUCEF_NULL, 8, 6, IMAGE::PSF_RGB, GUCEF::DATATYPE_UINT8 ) );
                UInt8* data = static_cast< UInt8* >( pixels->GetDataPtr() );
                for ( UInt32 b=0; b<pixels->GetTotalSizeInBytes(); ++b )
                    data[ b ] = (UInt8) ( b * 5 );
                IMAGE::CImage image( pixels );

                CORE::CDynamicBuffer file;
                CORE::CDynamicBufferAccess fileAccess( &file, false );
                GUCEF_TESTFW_ASSERT_TRUE( image.Save( fileAccess, types[ i ] ) );
                GUCEF_TESTFW_ASSERT_TRUE( file.GetDataSize() > 0 );

                fileAccess.Setpos( 0 );
                IMAGE::CImage loaded;
                GUCEF_TESTFW_ASSERT_TRUE( loaded.Load( fileAccess, types[ i ], true, false ) && 1 == loaded.GetFrameCount() );
                if ( loaded.HasFrames() )
                    GUCEF_TESTFW_ASSERT_TRUE( 8 == loaded.GetPixelMap( 0, 0 )->GetWidthInPixels() && 6 == loaded.GetPixelMap( 0, 0 )->GetHeightInPixels() );
            }

            // a type nobody has is refused
            IMAGE::CImage image( IMAGE::TPixelMapPtr( GUCEF_NEW IMAGE::CPixelMap( GUCEF_NULL, 2, 2, IMAGE::PSF_RGB, GUCEF::DATATYPE_UINT8 ) ) );
            CORE::CDynamicBuffer none;
            CORE::CDynamicBufferAccess noneAccess( &none, false );
            GUCEF_TESTFW_ASSERT_FALSE( image.Save( noneAccess, "nosuchtype" ) );

            GUCEF_TESTFW_ASSERT_TRUE( CORE::CCoreGlobal::Instance()->GetPluginControl().UnloadPluginGroup( "imagecodecwritetests" ) );
        }
        catch ( ... )
        {
            GUCEF_TESTFW_ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
TestFreeImageGifFramesAndCapabilities( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "imgpluginFreeImage reads the frames of an animated GIF with their durations, and advertises writing only for the formats FreeImage can write" )
        try
        {
            IMAGE::CImageGlobal::Instance();
            GUCEF_TESTFW_ASSERT_TRUE( LoadImageCodecPlugin( "imgpluginFreeImage", "imagecodecgiftests" ) );

            // 8 x 8 pixels, three frames that are shown 100, 150 and 200 milliseconds, a loop count of 0 (forever)
            static const UInt8 gif[] =
            {
                0x47, 0x49, 0x46, 0x38, 0x39, 0x61, 0x08, 0x00, 0x08, 0x00, 0xF1, 0x00, 0x00, 0xFF, 0x00, 0x00,
                0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x21, 0xFF, 0x0B, 0x4E, 0x45, 0x54, 0x53,
                0x43, 0x41, 0x50, 0x45, 0x32, 0x2E, 0x30, 0x03, 0x01, 0x00, 0x00, 0x00, 0x21, 0xF9, 0x04, 0x00,
                0x0A, 0x00, 0x00, 0x00, 0x2C, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x08, 0x00, 0x00, 0x02, 0x25,
                0x04, 0x08, 0x30, 0x61, 0x42, 0x80, 0x08, 0x13, 0x46, 0x04, 0x98, 0x30, 0xA1, 0x44, 0x84, 0x09,
                0x23, 0x4A, 0x4C, 0x98, 0x50, 0xA2, 0xC4, 0x84, 0x11, 0x25, 0x6A, 0x4C, 0x28, 0x51, 0xE2, 0xC6,
                0x88, 0x12, 0x35, 0x6E, 0x2C, 0x00, 0x21, 0xF9, 0x04, 0x00, 0x0F, 0x00, 0x00, 0x00, 0x2C, 0x00,
                0x00, 0x00, 0x00, 0x08, 0x00, 0x08, 0x00, 0x00, 0x02, 0x25, 0x4C, 0x98, 0x50, 0xA2, 0xC4, 0x84,
                0x11, 0x25, 0x6A, 0x4C, 0x28, 0x51, 0xE2, 0xC6, 0x88, 0x12, 0x35, 0x6E, 0x94, 0x28, 0x71, 0xE3,
                0x46, 0x89, 0x1A, 0x37, 0x0E, 0x94, 0xB8, 0x71, 0x23, 0x40, 0x8D, 0x1B, 0x07, 0x02, 0x2C, 0x00,
                0x21, 0xF9, 0x04, 0x00, 0x14, 0x00, 0x00, 0x00, 0x2C, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x08,
                0x00, 0x00, 0x02, 0x25, 0x94, 0x28, 0x71, 0xE3, 0x46, 0x89, 0x1A, 0x37, 0x0E, 0x94, 0xB8, 0x71,
                0x23, 0x40, 0x8D, 0x1B, 0x07, 0x02, 0xDC, 0xB8, 0x11, 0x20, 0xC0, 0x8D, 0x03, 0x01, 0x22, 0xDC,
                0x08, 0x10, 0x60, 0xC2, 0x81, 0x00, 0x11, 0x26, 0x2C, 0x00, 0x3B
            };
            CORE::CDynamicBuffer file;
            file.CopyFrom( sizeof( gif ), gif );
            CORE::CDynamicBufferAccess access( &file, false );
            access.Setpos( 0 );

            IMAGE::CImage image;
            GUCEF_TESTFW_ASSERT_TRUE( image.Load( access, "gif", true, false ) );
            GUCEF_TESTFW_ASSERT_TRUE( 3 == image.GetFrameCount() );
            if ( 3 == image.GetFrameCount() )
            {
                GUCEF_TESTFW_ASSERT_TRUE( 100 == image.GetFrameDurationInMs( 0 ) && 150 == image.GetFrameDurationInMs( 1 ) && 200 == image.GetFrameDurationInMs( 2 ) );
                GUCEF_TESTFW_ASSERT_TRUE( image.IsAnimation() && 0 == image.GetLoopCount() );
                for ( UInt32 f=0; f<3; ++f )
                    GUCEF_TESTFW_ASSERT_TRUE( 8 == image.GetPixelMap( f, 0 )->GetWidthInPixels() && 8 == image.GetPixelMap( f, 0 )->GetHeightInPixels() );

                // the frames show different pictures
                GUCEF_TESTFW_ASSERT_TRUE( 0 != memcmp( image.GetPixelMap( 0, 0 )->GetDataPtr(), image.GetPixelMap( 1, 0 )->GetDataPtr(), image.GetPixelMap( 0, 0 )->GetTotalSizeInBytes() ) );
                GUCEF_TESTFW_ASSERT_TRUE( 0 != memcmp( image.GetPixelMap( 1, 0 )->GetDataPtr(), image.GetPixelMap( 2, 0 )->GetDataPtr(), image.GetPixelMap( 1, 0 )->GetTotalSizeInBytes() ) );
            }

            // only the formats FreeImage can write advertise that they can
            const char* const writable[] = { "png", "bmp", "jpg", "jpeg", "tga", "tif", "tiff", "ppm" };
            for ( size_t i=0; i<sizeof( writable ) / sizeof( writable[ 0 ] ); ++i )
            {
                IMAGE::CImageCodecCapabilities::SImageCodecInfo info;
                GUCEF_TESTFW_ASSERT_TRUE( IMAGE::CImageCodecCapabilities::TryGetCodec( writable[ i ], info ) && info.canDecode && info.canEncode );
            }
            const char* const readOnly[] = { "dds", "cut" };
            for ( size_t i=0; i<sizeof( readOnly ) / sizeof( readOnly[ 0 ] ); ++i )
            {
                IMAGE::CImageCodecCapabilities::SImageCodecInfo info;
                GUCEF_TESTFW_ASSERT_TRUE( IMAGE::CImageCodecCapabilities::TryGetCodec( readOnly[ i ], info ) && info.canDecode && !info.canEncode );
            }
            IMAGE::CImageCodecCapabilities::SImageCodecInfo gifInfo;
            GUCEF_TESTFW_ASSERT_TRUE( IMAGE::CImageCodecCapabilities::TryGetCodec( "gif", gifInfo ) && gifInfo.multipleFrames );

            GUCEF_TESTFW_ASSERT_TRUE( CORE::CCoreGlobal::Instance()->GetPluginControl().UnloadPluginGroup( "imagecodecgiftests" ) );
        }
        catch ( ... )
        {
            GUCEF_TESTFW_ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static IMAGE::TPixelMapPtr
MakeTestPixels( UInt32 width, UInt32 height, IMAGE::TPixelStorageFormat format, GUCEF::TBuildinDataType type, UInt32 seed )
{
    IMAGE::TPixelMapPtr pixels( GUCEF_NEW IMAGE::CPixelMap( GUCEF_NULL, width, height, format, type ) );
    UInt8* data = static_cast< UInt8* >( pixels->GetDataPtr() );
    const UInt32 size = pixels->GetTotalSizeInBytes();
    for ( UInt32 i=0; i<size; ++i )
        data[ i ] = (UInt8) ( i * 3 + seed * 41 + ( i / 7 ) );
    return pixels;
}

/*-------------------------------------------------------------------------*/

static bool
SaveAndLoad( IMAGE::CImage& image, const CORE::CString& type, IMAGE::CImage& loaded )
{
    CORE::CDynamicBuffer file;
    CORE::CDynamicBufferAccess access( &file, false );
    if ( !image.Save( access, type ) || 0 == file.GetDataSize() )
        return false;
    access.Setpos( 0 );
    return loaded.Load( access, type, true, false );
}

/*-------------------------------------------------------------------------*/

static void
TestFreeImageWritesPaletteFloatAndAnimation( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "imgpluginFreeImage writes palette, black and white, floating point and 16 bit pictures, animated GIFs and the icon sizes of ICO" )
        try
        {
            IMAGE::CImageGlobal::Instance();
            GUCEF_TESTFW_ASSERT_TRUE( LoadImageCodecPlugin( "imgpluginFreeImage", "imagecodecwritetests2" ) );
            IMAGE::CImage loaded;

            // GIF holds a palette: the colours are reduced to it
            IMAGE::CImage colour( MakeTestPixels( 16, 16, IMAGE::PSF_RGBA, GUCEF::DATATYPE_UINT8, 1 ) );
            GUCEF_TESTFW_ASSERT_TRUE( SaveAndLoad( colour, "gif", loaded ) && 1 == loaded.GetFrameCount() );
            if ( loaded.HasFrames() )
                GUCEF_TESTFW_ASSERT_TRUE( 16 == loaded.GetPixelMap( 0, 0 )->GetWidthInPixels() && 16 == loaded.GetPixelMap( 0, 0 )->GetHeightInPixels() );

            // an animation keeps its frames, their durations and its loop count
            IMAGE::CImage animation( MakeTestPixels( 12, 10, IMAGE::PSF_RGB, GUCEF::DATATYPE_UINT8, 2 ) );
            IMAGE::TPixelMapPtr second = MakeTestPixels( 12, 10, IMAGE::PSF_RGB, GUCEF::DATATYPE_UINT8, 3 );
            IMAGE::TPixelMapPtr third = MakeTestPixels( 12, 10, IMAGE::PSF_RGB, GUCEF::DATATYPE_UINT8, 4 );
            animation.AddFrame( second, 60 );
            animation.AddFrame( third, 80 );
            animation.SetFrameDurationInMs( 0, 40 );
            animation.SetLoopCount( 2 );
            IMAGE::CImage animationLoaded;
            GUCEF_TESTFW_ASSERT_TRUE( SaveAndLoad( animation, "gif", animationLoaded ) && 3 == animationLoaded.GetFrameCount() );
            if ( 3 == animationLoaded.GetFrameCount() )
            {
                GUCEF_TESTFW_ASSERT_TRUE( 40 == animationLoaded.GetFrameDurationInMs( 0 ) && 60 == animationLoaded.GetFrameDurationInMs( 1 ) && 80 == animationLoaded.GetFrameDurationInMs( 2 ) );
                GUCEF_TESTFW_ASSERT_TRUE( 2 == animationLoaded.GetLoopCount() );
                GUCEF_TESTFW_ASSERT_TRUE( 0 != memcmp( animationLoaded.GetPixelMap( 0, 0 )->GetDataPtr(), animationLoaded.GetPixelMap( 2, 0 )->GetDataPtr(), animationLoaded.GetPixelMap( 0, 0 )->GetTotalSizeInBytes() ) );
            }

            // black and white
            IMAGE::CImage bilevel( MakeTestPixels( 16, 16, IMAGE::PSF_RGB, GUCEF::DATATYPE_UINT8, 5 ) );
            IMAGE::CImage bilevelLoaded;
            GUCEF_TESTFW_ASSERT_TRUE( SaveAndLoad( bilevel, "wbmp", bilevelLoaded ) && bilevelLoaded.HasFrames() );
            if ( bilevelLoaded.HasFrames() )
            {
                const UInt8* data = static_cast< const UInt8* >( bilevelLoaded.GetPixelMap( 0, 0 )->GetDataPtr() );
                bool onlyBlackAndWhite = true;
                for ( UInt32 i=0; i<bilevelLoaded.GetPixelMap( 0, 0 )->GetTotalSizeInBytes(); ++i )
                    onlyBlackAndWhite = onlyBlackAndWhite && ( 0 == data[ i ] || 255 == data[ i ] );
                GUCEF_TESTFW_ASSERT_TRUE( onlyBlackAndWhite );
            }

            // floating point pictures keep their values
            IMAGE::CImage floats( IMAGE::TPixelMapPtr( GUCEF_NEW IMAGE::CPixelMap( GUCEF_NULL, 4, 4, IMAGE::PSF_RGBA, GUCEF::DATATYPE_LE_FLOAT32 ) ) );
            Float32* floatData = static_cast< Float32* >( floats.GetPixelMap( 0, 0 )->GetDataPtr() );
            for ( UInt32 i=0; i<4 * 4 * 4; ++i )
                floatData[ i ] = (Float32) ( i % 17 ) / 16.0f;
            IMAGE::CImage floatsLoaded;
            GUCEF_TESTFW_ASSERT_TRUE( SaveAndLoad( floats, "exr", floatsLoaded ) && floatsLoaded.HasFrames() );
            if ( floatsLoaded.HasFrames() )
            {
                IMAGE::TPixelMapPtr pixels = floatsLoaded.GetPixelMap( 0, 0 );
                GUCEF_TESTFW_ASSERT_TRUE( GUCEF::DATATYPE_LE_FLOAT32 == pixels->GetPixelComponentDataType() && IMAGE::PSF_RGBA == pixels->GetPixelStorageFormat() );
                const Float32* out = static_cast< const Float32* >( pixels->GetDataPtr() );
                bool same = 4 == pixels->GetWidthInPixels();
                for ( UInt32 i=0; same && i<4 * 4 * 4; ++i )
                    same = out[ i ] > floatData[ i ] - 0.001f && out[ i ] < floatData[ i ] + 0.001f;
                GUCEF_TESTFW_ASSERT_TRUE( same );
            }

            // 16 bit channels are kept as they are
            IMAGE::CImage wide( MakeTestPixels( 6, 5, IMAGE::PSF_RGB, GUCEF::DATATYPE_LE_UINT16, 6 ) );
            IMAGE::CImage wideLoaded;
            GUCEF_TESTFW_ASSERT_TRUE( SaveAndLoad( wide, "tif", wideLoaded ) && wideLoaded.HasFrames() );
            if ( wideLoaded.HasFrames() )
            {
                IMAGE::TPixelMapPtr pixels = wideLoaded.GetPixelMap( 0, 0 );
                GUCEF_TESTFW_ASSERT_TRUE( GUCEF::DATATYPE_LE_UINT16 == pixels->GetPixelComponentDataType() && IMAGE::PSF_RGB == pixels->GetPixelStorageFormat() );
                GUCEF_TESTFW_ASSERT_TRUE( pixels->GetTotalSizeInBytes() == wide.GetPixelMap( 0, 0 )->GetTotalSizeInBytes() &&
                                          0 == memcmp( pixels->GetDataPtr(), wide.GetPixelMap( 0, 0 )->GetDataPtr(), pixels->GetTotalSizeInBytes() ) );
            }

            // an icon is square and 16 to 256 pixels
            IMAGE::CImage icon( MakeTestPixels( 32, 32, IMAGE::PSF_RGBA, GUCEF::DATATYPE_UINT8, 7 ) );
            IMAGE::CImage iconLoaded;
            GUCEF_TESTFW_ASSERT_TRUE( SaveAndLoad( icon, "ico", iconLoaded ) && 32 == iconLoaded.GetPixelMap( 0, 0 )->GetWidthInPixels() );
            IMAGE::CImage notAnIcon( MakeTestPixels( 20, 10, IMAGE::PSF_RGBA, GUCEF::DATATYPE_UINT8, 8 ) );
            CORE::CDynamicBuffer refused;
            CORE::CDynamicBufferAccess refusedAccess( &refused, false );
            GUCEF_TESTFW_ASSERT_FALSE( notAnIcon.Save( refusedAccess, "ico" ) );

            // GIF and the animation formats say they hold frames; a format of one picture does not
            IMAGE::CImageCodecCapabilities::SImageCodecInfo info;
            GUCEF_TESTFW_ASSERT_TRUE( IMAGE::CImageCodecCapabilities::TryGetCodec( "gif", info ) && info.multipleFrames && info.canEncode );
            GUCEF_TESTFW_ASSERT_TRUE( IMAGE::CImageCodecCapabilities::TryGetCodec( "png", info ) && !info.multipleFrames );

            GUCEF_TESTFW_ASSERT_TRUE( CORE::CCoreGlobal::Instance()->GetPluginControl().UnloadPluginGroup( "imagecodecwritetests2" ) );
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
    TestImageCodecEncodeCapabilities();
    TestFreeImageWritesByFileExtension();
    TestFreeImageGifFramesAndCapabilities();
    TestFreeImageWritesPaletteFloatAndAnimation();
}

/*-------------------------------------------------------------------------*/
