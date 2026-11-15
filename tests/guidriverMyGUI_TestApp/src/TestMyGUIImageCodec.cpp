/*
 *  guidriverMyGUI_TestApp: Application for testing the MyGUI GUI drivers
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

#include <string.h>

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_VFS_H
#include "gucefVFS.h"
#define GUCEF_VFS_H
#endif /* GUCEF_VFS_H ? */

#ifndef GUCEF_IMAGE_H
#include "gucefIMAGE.h"
#define GUCEF_IMAGE_H
#endif /* GUCEF_IMAGE_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "gucefGUI.h"
#include "guidriverMyGUI_CMyGUIImageCodec.h"

#include "guidriverMyGUI_TestCommon.h"

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

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

namespace {

const char* const MountName = "mygui_test_images";

/** 4 x 2 pixels, every one different, the top row first: red, green, blue, white over grey, a half see through yellow, dark, a mix */
const UInt8 g_expected[ 4 * 2 * 4 ] =
{
    255,   0,   0, 255,     0, 255,   0, 255,     0,   0, 255, 255,   255, 255, 255, 255,
    128, 128, 128, 255,   255, 255,   0, 128,    16,  32,  48, 255,    200, 100,  50,  25
};

/** The pixels as the PNG encoder of gucefIMAGE wrote them at the top left (no alpha is premultiplied anywhere) */
bool
SamePixels( const void* a, const void* b, UInt32 pixelCount )
{
    return 0 == memcmp( a, b, (size_t) pixelCount * 4 );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformMyGUIImageCodecTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "MyGUI image codec over gucefIMAGE and the VFS" );

    bool haveCodecs = LoadImageCodecs();

    // the picture is written with the encoder of gucefIMAGE (the top row first), outside of what is under test
    CORE::CString directory = CORE::RelativePath( "$CURWORKDIR$" );
    CORE::AppendToPath( directory, MountName );
    CORE::CreateDirs( directory );
    CORE::CString pngPath = directory;
    CORE::AppendToPath( pngPath, "reference.png" );
    bool prepared = false;
    {
        IMAGE::CPixelMap pixels( g_expected, 4, 2, IMAGE::PSF_RGBA, GUCEF::DATATYPE_UINT8 );
        CORE::CString problem;
        prepared = IMAGE::CPngImageCodec::WritePixelMapToFile( pixels, pngPath, problem );
    }
    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    if ( prepared )
        prepared = vfs.MountArchive( MountName, directory, VFS::CVFS::FileSystemArchiveTypeName, MountName, true, false, directory );

    GUCEF_TESTFW_TESTCASE( "A PNG of the VFS comes out as RGBA, the top row first, in a buffer MyGUI can delete[]" )
    try
    {
        ASSERT_TRUE( haveCodecs );
        ASSERT_TRUE( prepared );
        if ( haveCodecs && prepared )
        {
            int width = 0, height = 0;
            MyGUI::PixelFormat format;
            void* data = guidriverMyGUI::CMyGUIImageCodec::LoadImage( CORE::CString( MountName ) + "/reference.png", width, height, format );
            ASSERT_TRUE( GUCEF_NULL != data );
            ASSERT_TRUE( 4 == width && 2 == height );
            ASSERT_TRUE( format == MyGUI::PixelFormat( MyGUI::PixelFormat::R8G8B8A8 ) );
            if ( GUCEF_NULL != data )
            {
                ASSERT_TRUE( SamePixels( data, g_expected, 8 ) );
                delete[] static_cast< unsigned char* >( data );
            }

            // a file that is not there, and one that is no picture
            ASSERT_TRUE( GUCEF_NULL == guidriverMyGUI::CMyGUIImageCodec::LoadImage( CORE::CString( MountName ) + "/missing.png", width, height, format ) );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "MyGUI pixels stored to a VFS file are read back the same" )
    try
    {
        ASSERT_TRUE( haveCodecs && prepared );
        if ( haveCodecs && prepared )
        {
            ASSERT_TRUE( guidriverMyGUI::CMyGUIImageCodec::SaveImage( 4, 2, MyGUI::PixelFormat( MyGUI::PixelFormat::R8G8B8A8 ), g_expected,
                                                                      CORE::CString( MountName ) + "/stored.png" ) );
            int width = 0, height = 0;
            MyGUI::PixelFormat format;
            void* data = guidriverMyGUI::CMyGUIImageCodec::LoadImage( CORE::CString( MountName ) + "/stored.png", width, height, format );
            ASSERT_TRUE( GUCEF_NULL != data && 4 == width && 2 == height );
            if ( GUCEF_NULL != data )
            {
                ASSERT_TRUE( SamePixels( data, g_expected, 8 ) );
                delete[] static_cast< unsigned char* >( data );
            }

            // pixels that are not a MyGUI format are refused
            ASSERT_FALSE( guidriverMyGUI::CMyGUIImageCodec::SaveImage( 4, 2, MyGUI::PixelFormat( MyGUI::PixelFormat::Unknow ), g_expected,
                                                                       CORE::CString( MountName ) + "/never.png" ) );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Pixel maps in other formats are converted to RGBA" )
    try
    {
        // RGB, luminance and 16 bit channels all arrive as 8 bit RGBA
        const UInt8 rgb[ 2 * 1 * 3 ] = { 10, 20, 30, 40, 50, 60 };
        IMAGE::CPixelMap rgbMap( rgb, 2, 1, IMAGE::PSF_RGB, GUCEF::DATATYPE_UINT8 );
        int width = 0, height = 0;
        MyGUI::PixelFormat format;
        unsigned char* data = static_cast< unsigned char* >( guidriverMyGUI::CMyGUIImageCodec::CopyPixels( rgbMap, width, height, format ) );
        ASSERT_TRUE( GUCEF_NULL != data && 2 == width && 1 == height );
        if ( GUCEF_NULL != data )
        {
            ASSERT_TRUE( 10 == data[ 0 ] && 20 == data[ 1 ] && 30 == data[ 2 ] && 255 == data[ 3 ] );
            ASSERT_TRUE( 40 == data[ 4 ] && 50 == data[ 5 ] && 60 == data[ 6 ] && 255 == data[ 7 ] );
            delete[] data;
        }

        const UInt8 bgr[ 1 * 1 * 3 ] = { 1, 2, 3 };
        IMAGE::CPixelMap bgrMap( bgr, 1, 1, IMAGE::PSF_BGR, GUCEF::DATATYPE_UINT8 );
        data = static_cast< unsigned char* >( guidriverMyGUI::CMyGUIImageCodec::CopyPixels( bgrMap, width, height, format ) );
        ASSERT_TRUE( GUCEF_NULL != data );
        if ( GUCEF_NULL != data )
        {
            ASSERT_TRUE( 3 == data[ 0 ] && 2 == data[ 1 ] && 1 == data[ 2 ] );
            delete[] data;
        }

        IMAGE::CPixelMap empty( GUCEF_NULL, 0, 0, IMAGE::PSF_RGBA, GUCEF::DATATYPE_UINT8 );
        ASSERT_TRUE( GUCEF_NULL == guidriverMyGUI::CMyGUIImageCodec::CopyPixels( empty, width, height, format ) );

        IMAGE::TPixelMapPtr copy = guidriverMyGUI::CMyGUIImageCodec::ToPixelMap( 4, 2, MyGUI::PixelFormat( MyGUI::PixelFormat::R8G8B8A8 ), g_expected );
        ASSERT_TRUE( !copy.IsNULL() && 4 == copy->GetWidthInPixels() && IMAGE::PSF_RGBA == copy->GetPixelStorageFormat() );
        ASSERT_TRUE( copy.IsNULL() || SamePixels( copy->GetDataPtr(), g_expected, 8 ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Platforms that read B, G, R, A get that order, in both directions" )
    try
    {
        ASSERT_TRUE( haveCodecs && prepared );
        if ( haveCodecs && prepared )
        {
            int width = 0, height = 0;
            MyGUI::PixelFormat format;
            unsigned char* data = static_cast< unsigned char* >( guidriverMyGUI::CMyGUIImageCodec::LoadImage( CORE::CString( MountName ) + "/reference.png", width, height, format, true ) );
            ASSERT_TRUE( GUCEF_NULL != data && 4 == width && 2 == height );
            if ( GUCEF_NULL != data )
            {
                // the first pixel is red: B, G, R, A
                ASSERT_TRUE( 0 == data[ 0 ] && 0 == data[ 1 ] && 255 == data[ 2 ] && 255 == data[ 3 ] );
                // the third is blue
                ASSERT_TRUE( 255 == data[ 8 ] && 0 == data[ 9 ] && 0 == data[ 10 ] );
                ASSERT_TRUE( guidriverMyGUI::CMyGUIImageCodec::SaveImage( width, height, format, data, CORE::CString( MountName ) + "/bgra.png", true ) );
                delete[] data;
            }
            data = static_cast< unsigned char* >( guidriverMyGUI::CMyGUIImageCodec::LoadImage( CORE::CString( MountName ) + "/bgra.png", width, height, format, false ) );
            ASSERT_TRUE( GUCEF_NULL != data && SamePixels( data, g_expected, 8 ) );
            delete[] data;
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    vfs.UnmountArchiveByName( MountName );
}

/*-------------------------------------------------------------------------*/
