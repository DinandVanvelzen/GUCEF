/*
 *  OgreCodec_GucefIMAGE: Ogre plugin that decodes and encodes the image formats of gucefIMAGE for Ogre
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

#ifndef GUCEF_OGRECODEC_GUCEFIMAGE_CGUCEFIMAGECONVERTER_H
#define GUCEF_OGRECODEC_GUCEFIMAGE_CGUCEFIMAGECONVERTER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include "OgreImage.h"
#include "OgrePixelFormat.h"

#ifndef GUCEF_IMAGE_CIMAGE_H
#include "gucefIMAGE_CImage.h"
#define GUCEF_IMAGE_CIMAGE_H
#endif /* GUCEF_IMAGE_CIMAGE_H ? */

#ifndef GUCEF_OGRECODEC_GUCEFIMAGE_MACROS_H
#include "OgreCodec_GucefIMAGE_macros.h"
#define GUCEF_OGRECODEC_GUCEFIMAGE_MACROS_H
#endif /* GUCEF_OGRECODEC_GUCEFIMAGE_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace PLUGINGLUE {
namespace OGRE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Converts between the images of gucefIMAGE and Ogre::Image.
 *
 *  An Ogre image has one face or six (a cube map) and the same number of mipmap levels on every face, laid out
 *  face after face with the levels of a face one after the other. The conversion follows that:
 *
 *      - an image of 6 frames that is not an animation is a cube map, otherwise only the first frame is taken
 *      - the number of mipmap levels is the lowest of the faces, where the levels are valid: every level has
 *        half the size of the one before it (at least 1), the levels from the first one that does not are dropped
 *      - the pixel format of the first level of the first face is the format of the whole image, the other levels
 *        are converted to it
 *
 *  Pixel formats that Ogre has no format for are converted to 8 bit RGBA. Palette images are refused: the
 *  palette is not part of the pixel map.
 */
class GUCEF_OGRECODEC_GUCEFIMAGE_EXPORT_CPP CGucefImageConverter
{
    public:

    /**
     *  The Ogre pixel format that holds the pixels of a pixel storage format and data type as they are,
     *  Ogre::PF_UNKNOWN when there is none
     */
    static Ogre::PixelFormat ToOgrePixelFormat( IMAGE::TPixelStorageFormat storageFormat ,
                                                TBuildinDataType componentDataType       );

    /** The inverse for the formats that gucefIMAGE can write, false for the others */
    static bool FromOgrePixelFormat( Ogre::PixelFormat format                    ,
                                     IMAGE::TPixelStorageFormat& storageFormat   ,
                                     TBuildinDataType& componentDataType         );

    /** Makes the Ogre image from the decoded image, false with the reason in 'problem' */
    static bool ToOgreImage( const IMAGE::CImage& source ,
                             Ogre::Image& target         ,
                             Ogre::String& problem       );

    /** Makes an image with one frame from the first face and level of the Ogre image, false with the reason in 'problem' */
    static bool FromOgreImage( const Ogre::Image& source ,
                               IMAGE::CImage& target     ,
                               Ogre::String& problem     );

    private:

    CGucefImageConverter( void );
    ~CGucefImageConverter();
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace OGRE */
}; /* namespace PLUGINGLUE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_OGRECODEC_GUCEFIMAGE_CGUCEFIMAGECONVERTER_H ? */
