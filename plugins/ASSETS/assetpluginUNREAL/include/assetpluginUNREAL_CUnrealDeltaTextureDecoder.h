/*
 *  assetpluginUNREAL: Reverses the row prediction of texture sources stored as TSCF_UEDELTA
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

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALDELTATEXTUREDECODER_H
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALDELTATEXTUREDECODER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_ASSETPLUGIN_UNREAL_MACROS_H
#include "assetpluginUNREAL_macros.h"
#define GUCEF_ASSETPLUGIN_UNREAL_MACROS_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_MACROS_H ? */

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

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
 *  Turns the bytes of a TSCF_UEDELTA texture source, after the payload decompression, into the raw pixels of its format
 *  (the layout of TSCF_None). The form keeps the size of the raw pixels; it is a prediction filter that makes the pixels
 *  compress better:
 *
 *   - The image is cut into horizontal strips of StripRows() rows. The first row of each strip holds its pixels as they are.
 *   - Every other row holds, per sample, the difference to the same sample of the row above, modulo the sample size.
 *   - 8 bit samples (TSF_G8, TSF_BGRA8): the byte difference, no offset.
 *   - 16 bit samples (TSF_RGBA16): the little-endian 16 bit difference plus 0x8080 (a zero difference is stored as 80 80).
 *   - Strip height: as many rows as hold 32768 pixels (32768 / width), and at least 8 rows per byte of a pixel
 *     (TSF_BGRA8 32 rows, TSF_RGBA16 64 rows).
 *
 *  The strip height is the part that depends on the size. It is known for the sizes and formats the MetaHuman packages hold
 *  (widths 256 to 8192 in TSF_BGRA8, 2048 and 4096 in TSF_RGBA16, 2048 in TSF_G8) and for every size where the
 *  rules above leave one answer. A size where the rules leave two answers (a width that does not divide 32768 where the
 *  pixel count decides, a TSF_G8 source wider than 2048 pixels where the unobserved minimum decides) and every other format
 *  is refused with a reason rather than decoded on a guess. Only the first image (mip 0 of slice 0) is decoded; it is the
 *  start of the data and its strips do not depend on the later images.
 */
class GUCEF_ASSETPLUGIN_UNREAL_EXPORT_CPP CUnrealDeltaTextureDecoder
{
    public:

    static const UInt32 StripPixels = 32768;    /**< the pixel count a strip holds at least */
    static const UInt32 MaxDimension = 65536;   /**< widths and heights above this are refused */

    /** Bytes per pixel of a format this decoder handles; 0 for the others */
    static UInt32 BytesPerPixel( const CORE::CString& format );

    /**
     *  The rows of a strip for a source of the format and sizes.
     *  @return false with the reason when the format is not handled or the strip height of that size is not established
     */
    static bool StripRows( const CORE::CString& format, UInt32 width, UInt32 height, UInt32& rows, CORE::CString& error );

    /**
     *  Reverses the prediction of the first image in place. data holds at least width * height * bytes per pixel bytes
     *  (size); the bytes after the first image are left as they are.
     *  @return false with the reason for formats, sizes and data that are refused; data is not changed then
     */
    static bool Decode( const CORE::CString& format, UInt32 width, UInt32 height, UInt8* data, UInt64 size, CORE::CString& error );

    private:
    CUnrealDeltaTextureDecoder( void );     /**< only static members */
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

#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALDELTATEXTUREDECODER_H ? */
