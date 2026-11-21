/*
 *  assetpluginUNREAL: Writes PNG files from raw pixels (stored deflate blocks)
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

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALPNGWRITER_H
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALPNGWRITER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_ASSETPLUGIN_UNREAL_MACROS_H
#include "assetpluginUNREAL_macros.h"
#define GUCEF_ASSETPLUGIN_UNREAL_MACROS_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_MACROS_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETPLUGIN {
namespace UNREAL {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Makes a PNG file of raw pixels, 8 or 16 bits per channel, with 1 (grey), 3 (RGB) or 4 (RGBA) channels. The image data is
 *  written in stored deflate blocks (no compression): the files are intermediate files that the texture baker reads again, and
 *  the writer needs no library. Also gives the facts of PNG and JPEG files that sources hold, without decoding them.
 */
class GUCEF_ASSETPLUGIN_UNREAL_EXPORT_CPP CUnrealPngWriter
{
    public:

    /** @param pixels width * height * channels * ( bitsPerChannel / 8 ) bytes, rows top to bottom, 16 bit values as stored (little endian is converted) */
    static bool Write( const UInt8* pixels, UInt32 width, UInt32 height, UInt32 channels, UInt32 bitsPerChannel, CORE::CDynamicBuffer& png );

    /** Reads the sizes, the channels and the bit depth from the header of a PNG file */
    static bool ReadPngInfo( const UInt8* bytes, UInt32 size, UInt32& width, UInt32& height, UInt32& channels, UInt32& bitsPerChannel );

    /** Reads the sizes and the number of components from the start of frame marker of a JPEG file */
    static bool ReadJpegInfo( const UInt8* bytes, UInt32 size, UInt32& width, UInt32& height, UInt32& channels );

    /** The CRC-32 of the bytes (the PNG and zip polynomial), for checksums in tests and chunks */
    static UInt32 Crc32( const UInt8* bytes, UInt32 size, UInt32 start = 0 );

    private:
    CUnrealPngWriter( void );   /**< only static members */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace UNREAL */
}; /* namespace ASSETPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALPNGWRITER_H ? */
