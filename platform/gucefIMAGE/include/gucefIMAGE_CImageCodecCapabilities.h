/*
 *  gucefIMAGE: GUCEF module providing image utilities
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
#ifndef GUCEF_IMAGE_CIMAGECODECCAPABILITIES_H
#define GUCEF_IMAGE_CIMAGECODECCAPABILITIES_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEFIMAGE_MACROS_H
#include "gucefIMAGE_macros.h"       /* module macro's */
#define GUCEFIMAGE_MACROS_H
#endif /* GUCEFIMAGE_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace IMAGE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  What the image formats that can be read or written right now can do, for tools that list formats or pick one.
 *
 *  The image codec registry (codecs with the image interface, such as the plugins and the native format) is consulted
 *  first and the "ImageCodec" family of the codec registry (codecs of the basic interface, such as the built-in PNG
 *  writer) second, as CImage::Load() and CImage::Save() do. Formats are known by type, which is the file extension.
 */
class GUCEF_IMAGE_EXPORT_CPP CImageCodecCapabilities
{
    public:

    /** What is known of one format */
    struct SImageCodecInfo
    {
        CORE::CString type;         /**< the format, as its file extension in lower case */
        bool canDecode;             /**< images of the format can be read */
        bool canEncode;             /**< images of the format can be written */
        bool multipleFrames;        /**< a file holds more than one picture: an animation or a set */
        bool mipLevels;             /**< a file holds the mipmap levels of its pictures */

        SImageCodecInfo( void ) : type(), canDecode( false ), canEncode( false ), multipleFrames( false ), mipLevels( false ) {}
    };
    typedef GUCEF::vector< SImageCodecInfo > TImageCodecInfoVector;

    /** The formats that are available now, ordered by type */
    static void GetCodecs( TImageCodecInfoVector& codecs );

    /** What is known of a format by its type (case insensitive), false when no codec offers it */
    static bool TryGetCodec( const CORE::CString& type ,
                             SImageCodecInfo& info     );

    private:

    CImageCodecCapabilities( void );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace IMAGE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_IMAGE_CIMAGECODECCAPABILITIES_H ? */
