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

#ifndef GUCEF_IMAGE_CCHANNELOPERATIONS_H
#define GUCEF_IMAGE_CCHANNELOPERATIONS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_IMAGE_CPIXELMAP_H
#include "gucefIMAGE_CPixelMap.h"
#define GUCEF_IMAGE_CPIXELMAP_H
#endif /* GUCEF_IMAGE_CPIXELMAP_H ? */

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
 *  Operations on the channels of a pixel map: the building blocks of the texture operation chain of the asset
 *  converter. Every function has the two forms of CPixelMap: in place, and with a result that leaves the
 *  source as it is. They work on every storage format except palette indices, for the data types of the
 *  machine's byte order; they return false (and change nothing) for anything else.
 *
 *  Channel indices are in the storage order of the pixel map: for a BGRA map channel 0 is blue. A channel
 *  mask has bit i set for channel i. The values are handled normalised: 0 .. 1 for unsigned integers,
 *  -1 .. 1 for signed integers and as they are for floating point types.
 */
class GUCEF_IMAGE_EXPORT_CPP CChannelOperations
{
    public:

    /** A single channel (PSF_SINGLE_CHANNEL_GRAYSCALE) map of one channel, in the data type of the source */
    static bool ExtractChannel( const CPixelMap& source, UInt32 channelIndex, TPixelMapPtr& result );

    static bool ExtractChannel( CPixelMap& map, UInt32 channelIndex );

    /**
     *  'mapping' is one, three or four characters from R, G, B, A, 0 and 1: the channels of the result, which is a
     *  PSF_SINGLE_CHANNEL_GRAYSCALE, PSF_RGB or PSF_RGBA map; for example "GGG1", or "BGRA" to swap red and blue.
     *  R, G, B and A are the red, green, blue and alpha channel of the source, wherever its format stores them;
     *  of a single channel map the one channel is red, green and blue. 0 is a constant 0 and 1 the largest value
     *  (1 for floating point types). A channel that the source does not have refuses the mapping.
     */
    static bool Swizzle( const CPixelMap& source, const CORE::CString& mapping, TPixelMapPtr& result );

    static bool Swizzle( CPixelMap& map, const CORE::CString& mapping );

    /**
     *  'max - value' for the channels in the mask: smoothness to roughness, the green flip of DirectX normal
     *  maps. For signed integer types the value is negated, which is the same mirror within their range.
     */
    static bool Invert( CPixelMap& map, UInt32 channelMask );

    static bool Invert( const CPixelMap& source, UInt32 channelMask, TPixelMapPtr& result );

    /** 'value * scale + bias' for the channels in the mask, clamped to the range of integer data types */
    static bool ScaleBias( CPixelMap& map, UInt32 channelMask, Float32 scale, Float32 bias );

    static bool ScaleBias( const CPixelMap& source, UInt32 channelMask, Float32 scale, Float32 bias, TPixelMapPtr& result );

    /**
     *  Builds a PSF_RGBA map of one channel of each of up to four sources: 'sourceChannels[ i ]' of 'sources[ i ]'
     *  becomes channel i. A NULL source gives a constant: 0, and 1 for alpha. The sources must have equal sizes;
     *  callers resample first. The result has the data type of the first source that is not NULL; sources of
     *  another data type are converted.
     */
    static bool Pack( const TPixelMapPtr sources[ 4 ], const UInt32 sourceChannels[ 4 ], TPixelMapPtr& result );

    /** Converts the colour channels (not alpha) between sRGB and linear; refused for formats without red, green and blue */
    static bool ConvertColourSpace( CPixelMap& map, TColourSpace from, TColourSpace to );

    static bool ConvertColourSpace( const CPixelMap& source, TColourSpace from, TColourSpace to, TPixelMapPtr& result );

    /**
     *  Multiplies the colour channels by alpha. For COLOURSPACE_SRGB the colour is multiplied in linear light (decoded,
     *  multiplied, encoded again); alpha itself stays as it is. Refused for formats without red, green, blue and alpha.
     */
    static bool PremultiplyAlpha( CPixelMap& map, TColourSpace colourSpace );

    static bool PremultiplyAlpha( const CPixelMap& source, TColourSpace colourSpace, TPixelMapPtr& result );

    /**
     *  The opposite of PremultiplyAlpha(): divides the colour channels by alpha, clamped to the largest value. Texels
     *  with an alpha of 0 keep a colour of 0, since their colour is lost.
     */
    static bool UnpremultiplyAlpha( CPixelMap& map, TColourSpace colourSpace );

    static bool UnpremultiplyAlpha( const CPixelMap& source, TColourSpace colourSpace, TPixelMapPtr& result );

    private:

    CChannelOperations( void );
    ~CChannelOperations();
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace IMAGE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_IMAGE_CCHANNELOPERATIONS_H ? */
