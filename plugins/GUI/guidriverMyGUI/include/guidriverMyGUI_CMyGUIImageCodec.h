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

#ifndef GUCEF_guidriverMyGUI_CMYGUIIMAGECODEC_H
#define GUCEF_guidriverMyGUI_CMYGUIIMAGECODEC_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include "MyGUI_RenderFormat.h"

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_IMAGE_CPIXELMAP_H
#include "gucefIMAGE_CPixelMap.h"
#define GUCEF_IMAGE_CPIXELMAP_H
#endif /* GUCEF_IMAGE_CPIXELMAP_H ? */

#ifndef GUCEF_guidriverMyGUI_MACROS_H
#include "guidriverMyGUI_macros.h"
#define GUCEF_guidriverMyGUI_MACROS_H
#endif /* GUCEF_guidriverMyGUI_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace guidriverMyGUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  What the image loader interfaces of the MyGUI OpenGL platforms need, on the image codecs of gucefIMAGE and
 *  the files of the virtual file system. The platform specific loader classes of the renderer plugins call it.
 *
 *  Pictures come out as 8 bits per channel with alpha, top row first, in a buffer made with new[] (MyGUI frees
 *  it with delete[]). MyGUI names the memory layout "R8G8B8A8" but the platforms disagree about the order of the
 *  bytes: the OpenGL and Ogre platforms read B, G, R, A (a 32 bit value of A, R, G, B on a little endian CPU), the
 *  OpenGL ES platform reads R, G, B, A. 'bgraOrder' says which one the platform has.
 */
class GUCEF_guidriverMyGUI_PUBLIC_CPP CMyGUIImageCodec
{
    public:

    /** Loads the picture in the file (the codec comes from the file extension); NULL when it cannot be loaded */
    static void* LoadImage( const CORE::CString& vfsPath, int& width, int& height, MyGUI::PixelFormat& format, bool bgraOrder = false );

    /** Gives the picture, converted to RGBA when needed, the same way; NULL when it is empty */
    static void* CopyPixels( IMAGE::CPixelMap& pixels, int& width, int& height, MyGUI::PixelFormat& format, bool bgraOrder = false );

    /** Stores the pixels as the file type of the extension of the path says (png for example) */
    static bool SaveImage( int width, int height, MyGUI::PixelFormat format, const void* pixels, const CORE::CString& vfsPath, bool bgraOrder = false );

    /** The pixels MyGUI hands over as a pixel map (RGBA or RGB or luminance), a copy */
    static IMAGE::TPixelMapPtr ToPixelMap( int width, int height, MyGUI::PixelFormat format, const void* pixels, bool bgraOrder = false );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace guidriverMyGUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_guidriverMyGUI_CMYGUIIMAGECODEC_H ? */
