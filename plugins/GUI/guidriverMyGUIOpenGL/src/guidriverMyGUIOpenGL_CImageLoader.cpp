/*
 *  guidriverMyGUIOpenGL: glue module for the MyGUI GUI backend using OpenGL
 *  Copyright (C) 2002 - 2008.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

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

#include "guidriverMyGUIOpenGL_CImageLoader.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MYGUIGL {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

CImageLoader::CImageLoader( void )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CImageLoader::~CImageLoader()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

MyGUI::PixelFormat
CImageLoader::ConvertPixelFormat( IMAGE::TPixelMapPtr pixelMap )
{
    if ( pixelMap->GetPixelChannelSize() == 1 )
    {
        switch ( pixelMap->GetPixelStorageFormat() )
        {
            case IMAGE::PSF_RGB : return MyGUI::PixelFormat( MyGUI::PixelFormat::R8G8B8 );
            case IMAGE::PSF_RGBA : return MyGUI::PixelFormat( MyGUI::PixelFormat::R8G8B8A8 );
            case IMAGE::PSF_BGR :
            {
                pixelMap->ConvertFormatTo( IMAGE::PSF_RGB, pixelMap );
                return MyGUI::PixelFormat( MyGUI::PixelFormat::R8G8B8 );
            }
            case IMAGE::PSF_BGRA :
            {
                pixelMap->ConvertFormatTo( IMAGE::PSF_RGBA, pixelMap );
                return MyGUI::PixelFormat( MyGUI::PixelFormat::R8G8B8A8 );
            }
            case IMAGE::PSF_SINGLE_CHANNEL_GRAYSCALE : return MyGUI::PixelFormat( MyGUI::PixelFormat::L8 );
            case IMAGE::PSF_SINGLE_CHANNEL_RED : return MyGUI::PixelFormat( MyGUI::PixelFormat::L8 );
            case IMAGE::PSF_SINGLE_CHANNEL_GREEN : return MyGUI::PixelFormat( MyGUI::PixelFormat::L8 );
            case IMAGE::PSF_SINGLE_CHANNEL_BLUE : return MyGUI::PixelFormat( MyGUI::PixelFormat::L8 );
            case IMAGE::PSF_SINGLE_CHANNEL_ALPHA : return MyGUI::PixelFormat( MyGUI::PixelFormat::L8 );
            case IMAGE::PSF_SINGLE_CHANNEL_STD_LUMINANCE : return MyGUI::PixelFormat( MyGUI::PixelFormat::L8 );
            case IMAGE::PSF_SINGLE_CHANNEL_P1_LUMINANCE : return MyGUI::PixelFormat( MyGUI::PixelFormat::L8 );
            case IMAGE::PSF_SINGLE_CHANNEL_P2_LUMINANCE : return MyGUI::PixelFormat( MyGUI::PixelFormat::L8 );

            case IMAGE::PSF_LUMINANCE_ALPHA : return MyGUI::PixelFormat( MyGUI::PixelFormat::L8A8 );

            default:
            {
                return MyGUI::PixelFormat( MyGUI::PixelFormat::Unknow );
            }
        }
    }
    // else: MyGui cannot handle multi-byte channels
    return MyGUI::PixelFormat( MyGUI::PixelFormat::Unknow );
}

/*-------------------------------------------------------------------------*/

IMAGE::TPixelStorageFormat
CImageLoader::TranslatePixelFormat( const MyGUI::PixelFormat myGuiPixelFormat )
{
    if ( myGuiPixelFormat == MyGUI::PixelFormat( MyGUI::PixelFormat::R8G8B8 ) ) return IMAGE::PSF_RGB;
    if ( myGuiPixelFormat == MyGUI::PixelFormat( MyGUI::PixelFormat::R8G8B8A8 ) ) return IMAGE::PSF_RGBA;
    if ( myGuiPixelFormat == MyGUI::PixelFormat( MyGUI::PixelFormat::L8 ) ) return IMAGE::PSF_SINGLE_CHANNEL_STD_LUMINANCE;
    if ( myGuiPixelFormat == MyGUI::PixelFormat( MyGUI::PixelFormat::L8A8 ) ) return IMAGE::PSF_LUMINANCE_ALPHA;
    return IMAGE::PSF_UNKNOWN;
}

/*-------------------------------------------------------------------------*/

void*
CImageLoader::loadImage( int& _width                  ,
                         int& _height                 ,
                         MyGUI::PixelFormat& _format  ,
                         const std::string& _filename )
{GUCEF_TRACE;

    CORE::CString dataType = CORE::ExtractFileExtention( _filename ).Lowercase();
    
    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    VFS::TBasicVfsResourcePtr filePtr = vfs.GetFile( _filename );

    if ( !filePtr.IsNULL() )
    {
        IMAGE::CImage image;
        if ( image.Load( *filePtr->GetAccess() ,
                         dataType              ) )
        {
            // We don't care about mipmapping or frames, get 0,0
            IMAGE::TPixelMapPtr pixelMap = image.GetPixelMap( 0, 0 );
            if ( 0 != pixelMap )
            {
                // Translate pixel format enum and convert as needed
                _format = ConvertPixelFormat( pixelMap );
                if ( _format != MyGUI::PixelFormat( MyGUI::PixelFormat::Unknow ) )
                {
                    pixelMap->FlipHorizontal();
                    
                    // We have our data, release to MyGui, it takes ownership
                    _width = (int) pixelMap->GetWidthInPixels();
                    _height = (int) pixelMap->GetHeightInPixels();

                    return pixelMap->RelinquishPixelDataOwnership();
                }
            }
        }
    }

    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

void
CImageLoader::saveImage( int _width                   ,
                         int _height                  ,
                         MyGUI::PixelFormat _format   ,
                         void* _texture               ,
                         const std::string& _filename )
{
    CORE::CString dataType = CORE::ExtractFileExtention( _filename ).Lowercase();
    
    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    VFS::TBasicVfsResourcePtr filePtr = vfs.GetFile( _filename, "wb" );

    if ( !filePtr.IsNULL() )
    {
        IMAGE::TPixelMapPtr pixelMap( new IMAGE::CPixelMap( _texture                         , 
                                                            _width                           ,
                                                            _height                          ,
                                                            TranslatePixelFormat( _format )  ,
                                                            DATATYPE_UINT8                   ) );
        
        IMAGE::CImage image;
        image.Assign( pixelMap );
        image.Save( *filePtr->GetAccess() ,
                    dataType              );
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MYGUI */
}; /* namespace GUCE */

/*-------------------------------------------------------------------------*/
