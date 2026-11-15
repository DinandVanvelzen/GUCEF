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

#ifndef GUCEF_IMAGE_CPIXELMAPRESAMPLER_H
#define GUCEF_IMAGE_CPIXELMAPRESAMPLER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_IMAGE_CPIXELMAPCHANNELACCESS_H
#include "gucefIMAGE_CPixelMapChannelAccess.h"
#define GUCEF_IMAGE_CPIXELMAPCHANNELACCESS_H
#endif /* GUCEF_IMAGE_CPIXELMAPCHANNELACCESS_H ? */

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
 *  Scales a pixel map to any size with a choice of filter. The filtering is separable: a horizontal pass and
 *  then a vertical pass with a Float32 intermediate per channel. Downscaling widens the filter support by the
 *  scale factor, so no aliasing occurs; upscaling and downscaling both work.
 *
 *  Every storage format except palette indices and every integer and floating point data type of the machine's
 *  byte order is supported. The source is read as Float32 channels and the result is written in the format and
 *  data type of the source, rounded and clamped. Pixel maps of palette indices are refused: convert them to
 *  RGB or RGBA first. HSV and HSL maps are filtered as RGB.
 */
class GUCEF_IMAGE_EXPORT_CPP CPixelMapResampler
{
    public:

    /**
     *  Scales 'source' to the target size.
     *
     *  @param colourSpace the colour space the colour channels of the source are stored in: for sRGB they are filtered
     *         in linear light (with the exact sRGB transfer function) and stored as sRGB again; alpha and single
     *         channel data are always filtered as they are
     *  @param wrapMode what the filter sees beyond the edges, WRAPMODE_REPEAT keeps tiling textures seamless
     *  @return false when the source is not supported (palette indices, no data) or a target side is 0
     */
    static bool Resample( const CPixelMap& source        ,
                          UInt32 targetWidth             ,
                          UInt32 targetHeight            ,
                          TResampleFilter filter         ,
                          TColourSpace colourSpace       ,
                          TWrapMode wrapMode             ,
                          TPixelMapPtr& result           );

    /**
     *  The filtering itself on interleaved Float32 values: 'channelCount' values per pixel, the values are
     *  filtered as they are. 'result' receives target width times target height times channel count values.
     */
    static bool ResampleChannels( const Float32* source                           ,
                                  UInt32 sourceWidth                              ,
                                  UInt32 sourceHeight                             ,
                                  UInt32 channelCount                             ,
                                  UInt32 targetWidth                              ,
                                  UInt32 targetHeight                             ,
                                  TResampleFilter filter                          ,
                                  TWrapMode wrapMode                              ,
                                  CPixelMapChannelAccess::TFloatVector& result    );

    private:

    CPixelMapResampler( void );
    ~CPixelMapResampler();
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace IMAGE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_IMAGE_CPIXELMAPRESAMPLER_H ? */
