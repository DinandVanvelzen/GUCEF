/*
 *  guidriverMyGUI: GUI backend using MyGUI
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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */

#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */

#ifndef GUCEF_IMAGE_CIMAGE_H
#include "gucefIMAGE_CImage.h"
#define GUCEF_IMAGE_CIMAGE_H
#endif /* GUCEF_IMAGE_CIMAGE_H ? */

#include "MyGUI_DataManager.h"

#include "guidriverMyGUI_CMyGUIImageCodec.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace guidriverMyGUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void*
CMyGUIImageCodec::CopyPixels( IMAGE::CPixelMap& pixels, int& width, int& height, MyGUI::PixelFormat& format, bool bgraOrder )
{GUCEF_TRACE;

    if ( pixels.GetWidthInPixels() == 0 || pixels.GetHeightInPixels() == 0 )
        return GUCEF_NULL;

    if ( GUCEF::DATATYPE_UINT8 != pixels.GetPixelComponentDataType() )
        pixels.ConvertFormatTo( GUCEF::DATATYPE_UINT8 );
    const IMAGE::TPixelStorageFormat wanted = bgraOrder ? IMAGE::PSF_BGRA : IMAGE::PSF_RGBA;
    if ( wanted != pixels.GetPixelStorageFormat() )
    {
        if ( !pixels.ConvertFormatTo( wanted ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "MyGUIImageCodec: A picture could not be converted to " << ( bgraOrder ? "BGRA" : "RGBA" ) );
            return GUCEF_NULL;
        }
    }

    width = (int) pixels.GetWidthInPixels();
    height = (int) pixels.GetHeightInPixels();
    format = MyGUI::PixelFormat( MyGUI::PixelFormat::R8G8B8A8 );
    const size_t rowSize = (size_t) width * 4;
    unsigned char* copy = new unsigned char[ rowSize * height ];
    for ( int y=0; y<height; ++y )
        memcpy( copy + y * rowSize, pixels.GetDataAtScanLine( (UInt32) y ), rowSize );
    return copy;
}

/*-------------------------------------------------------------------------*/

void*
CMyGUIImageCodec::LoadImage( const CORE::CString& vfsPath, int& width, int& height, MyGUI::PixelFormat& format, bool bgraOrder )
{GUCEF_TRACE;

    // MyGUI names pictures like the other resources: the data manager knows where they are
    CORE::CString path = vfsPath;
    if ( GUCEF_NULL != MyGUI::DataManager::getInstancePtr() )
    {
        std::string resolved = MyGUI::DataManager::getInstance().getDataPath( vfsPath.C_String() );
        if ( !resolved.empty() )
            path = resolved.c_str();
    }

    VFS::TBasicVfsResourcePtr file = VFS::CVfsGlobal::Instance()->GetVfs().GetFile( path );
    if ( file.IsNULL() || file->GetAccess().IsNULL() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "MyGUIImageCodec: The picture " << vfsPath << " is not in the virtual file system" );
        return GUCEF_NULL;
    }

    IMAGE::CImage image;
    CORE::CString type = CORE::ExtractFileExtention( path ).Lowercase();
    if ( !image.Load( *file->GetAccess(), type, true, false ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "MyGUIImageCodec: No image codec could decode " << vfsPath );
        return GUCEF_NULL;
    }
    IMAGE::TPixelMapPtr pixels = image.GetPixelMap( 0, 0 );
    if ( pixels.IsNULL() )
        return GUCEF_NULL;
    return CopyPixels( *pixels, width, height, format, bgraOrder );
}

/*-------------------------------------------------------------------------*/

IMAGE::TPixelMapPtr
CMyGUIImageCodec::ToPixelMap( int width, int height, MyGUI::PixelFormat format, const void* pixels, bool bgraOrder )
{GUCEF_TRACE;

    IMAGE::TPixelStorageFormat storage = IMAGE::PSF_UNKNOWN;
    if ( format == MyGUI::PixelFormat( MyGUI::PixelFormat::R8G8B8A8 ) )
        storage = bgraOrder ? IMAGE::PSF_BGRA : IMAGE::PSF_RGBA;
    else
    if ( format == MyGUI::PixelFormat( MyGUI::PixelFormat::R8G8B8 ) )
        storage = IMAGE::PSF_RGB;
    else
    if ( format == MyGUI::PixelFormat( MyGUI::PixelFormat::L8A8 ) )
        storage = IMAGE::PSF_LUMINANCE_ALPHA;
    else
    if ( format == MyGUI::PixelFormat( MyGUI::PixelFormat::L8 ) )
        storage = IMAGE::PSF_SINGLE_CHANNEL_STD_LUMINANCE;
    if ( IMAGE::PSF_UNKNOWN == storage || width <= 0 || height <= 0 || GUCEF_NULL == pixels )
        return IMAGE::TPixelMapPtr();
    return IMAGE::TPixelMapPtr( GUCEF_NEW IMAGE::CPixelMap( pixels, (UInt32) width, (UInt32) height, storage, GUCEF::DATATYPE_UINT8 ) );
}

/*-------------------------------------------------------------------------*/

bool
CMyGUIImageCodec::SaveImage( int width, int height, MyGUI::PixelFormat format, const void* pixels, const CORE::CString& vfsPath, bool bgraOrder )
{GUCEF_TRACE;

    IMAGE::TPixelMapPtr pixelMap = ToPixelMap( width, height, format, pixels, bgraOrder );
    if ( pixelMap.IsNULL() )
        return false;

    VFS::TBasicVfsResourcePtr file = VFS::CVfsGlobal::Instance()->GetVfs().GetFile( vfsPath, "wb", true );
    if ( file.IsNULL() || file->GetAccess().IsNULL() )
        return false;

    IMAGE::CImage image;
    image.Assign( pixelMap );
    return image.Save( *file->GetAccess(), CORE::ExtractFileExtention( vfsPath ).Lowercase() );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace guidriverMyGUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
