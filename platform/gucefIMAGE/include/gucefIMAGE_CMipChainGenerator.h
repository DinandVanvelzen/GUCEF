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

#ifndef GUCEF_IMAGE_CMIPCHAINGENERATOR_H
#define GUCEF_IMAGE_CMIPCHAINGENERATOR_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_IMAGE_CIMAGE_H
#include "gucefIMAGE_CImage.h"
#define GUCEF_IMAGE_CIMAGE_H
#endif /* GUCEF_IMAGE_CIMAGE_H ? */

#ifndef GUCEF_IMAGE_CMIPCHAINSETTINGS_H
#include "gucefIMAGE_CMipChainSettings.h"
#define GUCEF_IMAGE_CMIPCHAINSETTINGS_H
#endif /* GUCEF_IMAGE_CMIPCHAINSETTINGS_H ? */

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
 *  Makes the mip chain of a picture. Every level is made from level 0, not from the previous level, so
 *  errors do not add up. Sizes halve per level and are rounded down, never below 1.
 *
 *  Colour content is filtered in linear light when the colour space is sRGB. Normal maps (the red, green and
 *  blue channels are x, y and z) are renormalised to unit length after filtering and are always filtered
 *  linearly. Data (masks, roughness, ...) is filtered linearly, each channel independent. With an alpha
 *  coverage cutoff the alpha of each level is scaled until the fraction of texels above the cutoff equals
 *  that of level 0, which keeps alpha tested foliage and fences from thinning out with distance.
 */
class GUCEF_IMAGE_EXPORT_CPP CMipChainGenerator
{
    public:

    typedef GUCEF::vector< TPixelMapPtr > TPixelMapPtrVector;

    /**
     *  Makes the chain: 'levels' receives a copy of level 0 followed by the generated levels, smallest last.
     *  Fails for the formats the resampler refuses and for normal maps that have no x, y and z channel.
     */
    static bool Generate( const CPixelMap& level0, const CMipChainSettings& settings, TPixelMapPtrVector& levels );

    /** Replaces the mip levels of every frame of the image by level 0 and the generated levels; the image is not changed when it fails */
    static bool Generate( CImage& image, const CMipChainSettings& settings );

    /** The number of levels of a chain, level 0 included: levels are added while their larger side is at least 'minimumSize' */
    static UInt32 GetLevelCount( UInt32 width, UInt32 height, UInt32 minimumSize );

    private:

    CMipChainGenerator( void );
    ~CMipChainGenerator();
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace IMAGE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_IMAGE_CMIPCHAINGENERATOR_H ? */
