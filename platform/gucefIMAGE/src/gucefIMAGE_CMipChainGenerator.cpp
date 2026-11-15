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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <math.h>

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_IMAGE_CPIXELMAPRESAMPLER_H
#include "gucefIMAGE_CPixelMapResampler.h"
#define GUCEF_IMAGE_CPIXELMAPRESAMPLER_H
#endif /* GUCEF_IMAGE_CPIXELMAPRESAMPLER_H ? */

#include "gucefIMAGE_CMipChainGenerator.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace IMAGE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

inline UInt32
HalveSide( UInt32 side, UInt32 level )
{
    UInt32 halved = level >= 32 ? 0 : ( side >> level );
    return halved > 0 ? halved : 1;
}

/** The number of texels whose alpha, scaled, is above the cutoff */
UInt32
CountAboveCutoff( const Float32* values, UInt32 pixelCount, UInt32 channelCount, UInt32 alphaIndex, Float32 scale, Float32 cutoff )
{
    UInt32 count = 0;
    for ( UInt32 i=0; i<pixelCount; ++i )
        if ( values[ (size_t) i * channelCount + alphaIndex ] * scale > cutoff )
            ++count;
    return count;
}

/** Finds the scale of the alpha for which the number of texels above the cutoff is closest to 'wantedCount' */
Float32
FindAlphaScale( const Float32* values, UInt32 pixelCount, UInt32 channelCount, UInt32 alphaIndex, Float32 cutoff, UInt32 wantedCount )
{
    Float32 low = 0.0f, high = 64.0f;
    for ( UInt32 i=0; i<32; ++i )
    {
        Float32 middle = ( low + high ) * 0.5f;
        if ( CountAboveCutoff( values, pixelCount, channelCount, alphaIndex, middle, cutoff ) < wantedCount )
            low = middle;
        else
            high = middle;
    }
    UInt32 lowCount = CountAboveCutoff( values, pixelCount, channelCount, alphaIndex, low, cutoff );
    UInt32 highCount = CountAboveCutoff( values, pixelCount, channelCount, alphaIndex, high, cutoff );
    UInt32 lowError = wantedCount > lowCount ? wantedCount - lowCount : lowCount - wantedCount;
    UInt32 highError = wantedCount > highCount ? wantedCount - highCount : highCount - wantedCount;
    return lowError < highError ? low : high;
}

/** Makes every texel of a normal map unit length again; 'signedValues' tells if the components are -1..1 or 0..1 */
void
RenormaliseNormals( Float32* values, UInt32 pixelCount, UInt32 channelCount, UInt32 xIndex, UInt32 yIndex, UInt32 zIndex, bool signedValues )
{
    const Float32 scale = signedValues ? 1.0f : 2.0f;
    const Float32 bias = signedValues ? 0.0f : -1.0f;
    for ( UInt32 i=0; i<pixelCount; ++i )
    {
        Float32* texel = values + (size_t) i * channelCount;
        Float32 x = texel[ xIndex ] * scale + bias;
        Float32 y = texel[ yIndex ] * scale + bias;
        Float32 z = texel[ zIndex ] * scale + bias;
        Float32 length = (Float32) sqrt( (Float64) x * x + (Float64) y * y + (Float64) z * z );
        if ( length > 1e-6f )
        {
            x /= length; y /= length; z /= length;
        }
        else
        {
            x = 0.0f; y = 0.0f; z = 1.0f;
        }
        if ( signedValues )
        {
            texel[ xIndex ] = x; texel[ yIndex ] = y; texel[ zIndex ] = z;
        }
        else
        {
            texel[ xIndex ] = x * 0.5f + 0.5f; texel[ yIndex ] = y * 0.5f + 0.5f; texel[ zIndex ] = z * 0.5f + 0.5f;
        }
    }
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

UInt32
CMipChainGenerator::GetLevelCount( UInt32 width, UInt32 height, UInt32 minimumSize )
{GUCEF_TRACE;

    if ( 0 == width || 0 == height )
        return 0;

    UInt32 count = 1;
    for ( UInt32 level=1; level<33; ++level )
    {
        UInt32 levelWidth = HalveSide( width, level ), levelHeight = HalveSide( height, level );
        UInt32 previousWidth = HalveSide( width, level - 1 ), previousHeight = HalveSide( height, level - 1 );
        if ( levelWidth == previousWidth && levelHeight == previousHeight )
            break; // both sides are 1 already
        UInt32 larger = levelWidth > levelHeight ? levelWidth : levelHeight;
        if ( larger < minimumSize )
            break;
        ++count;
    }
    return count;
}

/*-------------------------------------------------------------------------*/

bool
CMipChainGenerator::Generate( const CPixelMap& level0, const CMipChainSettings& settings, TPixelMapPtrVector& levels )
{GUCEF_TRACE;

    levels.clear();

    if ( PSF_PALETTE_INDICES == level0.GetPixelStorageFormat() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "MipChainGenerator: Palette pictures cannot get a mip chain, convert them to RGB or RGBA first" );
        return false;
    }
    if ( !CPixelMapChannelAccess::IsSupported( level0 ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "MipChainGenerator: The storage format or data type of the picture is not supported" );
        return false;
    }

    const TPixelStorageFormat format = level0.GetPixelStorageFormat();
    const TBuildinDataType dataType = level0.GetPixelComponentDataType();
    const UInt32 channelCount = CPixelMap::GetChannelCountForFormat( format );
    const UInt32 width = level0.GetWidthInPixels(), height = level0.GetHeightInPixels();
    const UInt32 pixelCount = level0.GetPixelCount();

    // The channels of a normal map: red, green and blue are x, y and z
    UInt32 xIndex = 0, yIndex = 1, zIndex = 2;
    if ( IMAGECONTENT_NORMALMAP == settings.content )
    {
        if ( 0 == CPixelMapChannelAccess::GetColourChannelCount( format ) || CPixelMapChannelAccess::IsHueBased( format ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "MipChainGenerator: A normal map needs a picture with red, green and blue channels" );
            return false;
        }
        if ( PSF_BGR == format || PSF_BGRA == format )
        {
            xIndex = 2; zIndex = 0;
        }
    }

    // Level 0 is kept as it is
    TPixelMapPtrVector result;
    result.push_back( TPixelMapPtr( GUCEF_NEW CPixelMap( level0 ) ) );

    const UInt32 levelCount = GetLevelCount( width, height, settings.minimumSize );
    if ( levelCount > 1 )
    {
        // Colour is filtered in linear light when it is stored as sRGB; normal maps and data are linear
        const TColourSpace storedSpace = IMAGECONTENT_COLOUR == settings.content ? settings.colourSpace : COLOURSPACE_LINEAR;

        CPixelMapChannelAccess::TFloatVector source;
        if ( !CPixelMapChannelAccess::Read( level0, source ) )
            return false;

        const Int32 alphaIndex = CPixelMapChannelAccess::GetAlphaChannelIndex( format );
        const bool keepCoverage = settings.alphaCoverageCutoff >= 0.0f && alphaIndex >= 0;
        Float64 coverage = 0.0;
        if ( keepCoverage )
        {
            coverage = (Float64) CountAboveCutoff( &source[ 0 ], pixelCount, channelCount, (UInt32) alphaIndex, 1.0f, settings.alphaCoverageCutoff ) / (Float64) pixelCount;
        }
        CPixelMapChannelAccess::ToWorkingSpace( &source[ 0 ], pixelCount, format, storedSpace );

        CPixelMapChannelAccess::TFloatVector filtered;
        for ( UInt32 level=1; level<levelCount; ++level )
        {
            const UInt32 levelWidth = HalveSide( width, level ), levelHeight = HalveSide( height, level );
            if ( !CPixelMapResampler::ResampleChannels( &source[ 0 ], width, height, channelCount, levelWidth, levelHeight, settings.filter, settings.wrapMode, filtered ) )
                return false;

            const UInt32 levelPixelCount = levelWidth * levelHeight;
            if ( IMAGECONTENT_NORMALMAP == settings.content )
                RenormaliseNormals( &filtered[ 0 ], levelPixelCount, channelCount, xIndex, yIndex, zIndex, settings.normalMapSigned );

            if ( keepCoverage )
            {
                UInt32 wantedCount = (UInt32) floor( coverage * (Float64) levelPixelCount + 0.5 );
                Float32 scale = FindAlphaScale( &filtered[ 0 ], levelPixelCount, channelCount, (UInt32) alphaIndex, settings.alphaCoverageCutoff, wantedCount );
                for ( UInt32 i=0; i<levelPixelCount; ++i )
                {
                    Float32& alpha = filtered[ (size_t) i * channelCount + alphaIndex ];
                    alpha *= scale;
                    if ( alpha > 1.0f )
                        alpha = 1.0f;
                }
            }

            CPixelMapChannelAccess::FromWorkingSpace( &filtered[ 0 ], levelPixelCount, format, storedSpace );

            TPixelMapPtr levelMap( GUCEF_NEW CPixelMap( GUCEF_NULL, levelWidth, levelHeight, format, dataType ) );
            if ( !CPixelMapChannelAccess::Write( &filtered[ 0 ], *levelMap ) )
                return false;
            result.push_back( levelMap );
        }
    }

    levels = result;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMipChainGenerator::Generate( CImage& image, const CMipChainSettings& settings )
{GUCEF_TRACE;

    const UInt32 frameCount = image.GetFrameCount();
    if ( 0 == frameCount )
        return false;

    GUCEF::vector< TPixelMapPtrVector > frames;
    frames.resize( frameCount );
    for ( UInt32 frame=0; frame<frameCount; ++frame )
    {
        TPixelMapPtr level0;
        if ( !image.TryGetPixelMap( frame, 0, level0 ) || level0.IsNULL() )
            return false;
        if ( !Generate( *level0, settings, frames[ frame ] ) )
            return false;
    }

    for ( UInt32 frame=0; frame<frameCount; ++frame )
    {
        CImage::TMipMapList& mipLevels = image.GetFrame( frame );
        mipLevels.clear();
        for ( size_t i=0; i<frames[ frame ].size(); ++i )
            mipLevels.push_back( frames[ frame ][ i ] );
    }
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace IMAGE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
