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
#include <string.h>

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "gucefIMAGE_CPixelMapResampler.h"

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

const Float64 PI = 3.14159265358979323846;

/** The taps of every output position along one axis: the (already wrapped) source indices and normalised weights */
struct SAxisWeights
{
    GUCEF::vector< UInt32 > start;      /**< per output position: the first tap in 'indices' and 'weights'; one extra entry marks the end */
    GUCEF::vector< UInt32 > indices;
    GUCEF::vector< Float32 > weights;
};

inline Float64
Sinc( Float64 x )
{
    if ( x > -1e-9 && x < 1e-9 )
        return 1.0;
    return sin( PI * x ) / ( PI * x );
}

/** Modified Bessel function of the first kind, order 0 */
Float64
BesselI0( Float64 x )
{
    Float64 sum = 1.0, term = 1.0, halfX = x * 0.5;
    for ( UInt32 k=1; k<40; ++k )
    {
        term *= ( halfX / (Float64) k ) * ( halfX / (Float64) k );
        sum += term;
        if ( term < sum * 1e-12 )
            break;
    }
    return sum;
}

Float64
FilterSupport( TResampleFilter filter )
{
    switch ( filter )
    {
        case RESAMPLEFILTER_BOX : return 0.5;
        case RESAMPLEFILTER_TRIANGLE : return 1.0;
        default : return 3.0;
    }
}

Float64
FilterKernel( TResampleFilter filter, Float64 x )
{
    Float64 magnitude = x < 0.0 ? -x : x;
    switch ( filter )
    {
        case RESAMPLEFILTER_TRIANGLE :
        {
            return magnitude < 1.0 ? 1.0 - magnitude : 0.0;
        }
        case RESAMPLEFILTER_LANCZOS3 :
        {
            return magnitude < 3.0 ? Sinc( magnitude ) * Sinc( magnitude / 3.0 ) : 0.0;
        }
        case RESAMPLEFILTER_KAISER :
        {
            // sinc windowed with a Kaiser window of alpha 4 over the support of 3
            static const Float64 alpha = 4.0;
            static const Float64 normaliser = BesselI0( alpha );
            if ( magnitude >= 3.0 )
                return 0.0;
            Float64 ratio = magnitude / 3.0;
            return Sinc( magnitude ) * BesselI0( alpha * sqrt( 1.0 - ratio * ratio ) ) / normaliser;
        }
        default :
        {
            return magnitude < 0.5 ? 1.0 : 0.0;
        }
    }
}

inline UInt32
WrapIndex( Int64 index, UInt32 size, TWrapMode wrapMode )
{
    Int64 n = (Int64) size;
    switch ( wrapMode )
    {
        case WRAPMODE_REPEAT :
        {
            Int64 wrapped = index % n;
            return (UInt32) ( wrapped < 0 ? wrapped + n : wrapped );
        }
        case WRAPMODE_MIRROR :
        {
            Int64 period = 2 * n;
            Int64 wrapped = index % period;
            if ( wrapped < 0 )
                wrapped += period;
            return (UInt32) ( wrapped < n ? wrapped : period - 1 - wrapped );
        }
        default :
        {
            return (UInt32) ( index < 0 ? 0 : ( index >= n ? n - 1 : index ) );
        }
    }
}

void
ComputeAxisWeights( UInt32 sourceSize, UInt32 targetSize, TResampleFilter filter, TWrapMode wrapMode, SAxisWeights& axis )
{
    axis.start.clear();
    axis.indices.clear();
    axis.weights.clear();
    axis.start.reserve( targetSize + 1 );

    Float64 scale = (Float64) sourceSize / (Float64) targetSize;
    GUCEF::vector< Float64 > tapWeights;
    for ( UInt32 o=0; o<targetSize; ++o )
    {
        axis.start.push_back( (UInt32) axis.indices.size() );
        tapWeights.clear();
        Int64 firstIndex;

        if ( RESAMPLEFILTER_BOX == filter )
        {
            // Area sampling: the weight of a source pixel is the part of it that the target pixel covers
            Float64 lo = (Float64) o * scale;
            Float64 hi = (Float64) ( o + 1 ) * scale;
            firstIndex = (Int64) floor( lo + 1e-9 );
            Int64 lastIndex = (Int64) ceil( hi - 1e-9 ) - 1;
            if ( lastIndex < firstIndex )
                lastIndex = firstIndex;
            for ( Int64 i=firstIndex; i<=lastIndex; ++i )
            {
                Float64 overlapLo = lo > (Float64) i ? lo : (Float64) i;
                Float64 overlapHi = hi < (Float64) ( i + 1 ) ? hi : (Float64) ( i + 1 );
                Float64 overlap = overlapHi - overlapLo;
                tapWeights.push_back( overlap > 0.0 ? overlap : 0.0 );
            }
        }
        else
        {
            Float64 filterScale = scale > 1.0 ? scale : 1.0;
            Float64 support = FilterSupport( filter ) * filterScale;
            Float64 center = ( (Float64) o + 0.5 ) * scale - 0.5;
            firstIndex = (Int64) ceil( center - support );
            Int64 lastIndex = (Int64) floor( center + support );
            for ( Int64 i=firstIndex; i<=lastIndex; ++i )
                tapWeights.push_back( FilterKernel( filter, ( (Float64) i - center ) / filterScale ) );
        }

        Float64 sum = 0.0;
        for ( size_t t=0; t<tapWeights.size(); ++t )
            sum += tapWeights[ t ];

        if ( sum > 1e-12 || sum < -1e-12 )
        {
            for ( size_t t=0; t<tapWeights.size(); ++t )
            {
                axis.indices.push_back( WrapIndex( firstIndex + (Int64) t, sourceSize, wrapMode ) );
                axis.weights.push_back( (Float32) ( tapWeights[ t ] / sum ) );
            }
        }
        else
        {
            // No weight at all, take the nearest source pixel
            Int64 nearest = (Int64) floor( ( (Float64) o + 0.5 ) * scale );
            axis.indices.push_back( WrapIndex( nearest, sourceSize, wrapMode ) );
            axis.weights.push_back( 1.0f );
        }
    }
    axis.start.push_back( (UInt32) axis.indices.size() );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

bool
CPixelMapResampler::ResampleChannels( const Float32* source                           ,
                                      UInt32 sourceWidth                              ,
                                      UInt32 sourceHeight                             ,
                                      UInt32 channelCount                             ,
                                      UInt32 targetWidth                              ,
                                      UInt32 targetHeight                             ,
                                      TResampleFilter filter                          ,
                                      TWrapMode wrapMode                              ,
                                      CPixelMapChannelAccess::TFloatVector& result    )
{GUCEF_TRACE;

    if ( GUCEF_NULL == source || 0 == sourceWidth || 0 == sourceHeight || 0 == channelCount || 0 == targetWidth || 0 == targetHeight )
        return false;

    const size_t sourceRowSize = (size_t) sourceWidth * channelCount;
    const size_t targetRowSize = (size_t) targetWidth * channelCount;

    if ( sourceWidth == targetWidth && sourceHeight == targetHeight )
    {
        result.assign( source, source + sourceRowSize * sourceHeight );
        return true;
    }

    SAxisWeights horizontal, vertical;
    ComputeAxisWeights( sourceWidth, targetWidth, filter, wrapMode, horizontal );
    ComputeAxisWeights( sourceHeight, targetHeight, filter, wrapMode, vertical );

    // Horizontal pass: every source row to the target width
    CPixelMapChannelAccess::TFloatVector intermediate;
    intermediate.resize( targetRowSize * sourceHeight );
    for ( UInt32 y=0; y<sourceHeight; ++y )
    {
        const Float32* sourceRow = source + y * sourceRowSize;
        Float32* targetRow = &intermediate[ y * targetRowSize ];
        for ( UInt32 x=0; x<targetWidth; ++x )
        {
            Float32* out = targetRow + (size_t) x * channelCount;
            const UInt32 tapEnd = horizontal.start[ x + 1 ];
            if ( 4 == channelCount )
            {
                Float32 a0 = 0.0f, a1 = 0.0f, a2 = 0.0f, a3 = 0.0f;
                for ( UInt32 t=horizontal.start[ x ]; t<tapEnd; ++t )
                {
                    const Float32* in = sourceRow + (size_t) horizontal.indices[ t ] * 4;
                    const Float32 weight = horizontal.weights[ t ];
                    a0 += in[ 0 ] * weight; a1 += in[ 1 ] * weight; a2 += in[ 2 ] * weight; a3 += in[ 3 ] * weight;
                }
                out[ 0 ] = a0; out[ 1 ] = a1; out[ 2 ] = a2; out[ 3 ] = a3;
            }
            else
            {
                for ( UInt32 c=0; c<channelCount; ++c )
                    out[ c ] = 0.0f;
                for ( UInt32 t=horizontal.start[ x ]; t<tapEnd; ++t )
                {
                    const Float32* in = sourceRow + (size_t) horizontal.indices[ t ] * channelCount;
                    const Float32 weight = horizontal.weights[ t ];
                    for ( UInt32 c=0; c<channelCount; ++c )
                        out[ c ] += in[ c ] * weight;
                }
            }
        }
    }

    // Vertical pass: every target row is the weighted sum of whole rows of the intermediate
    result.assign( targetRowSize * targetHeight, 0.0f );
    for ( UInt32 y=0; y<targetHeight; ++y )
    {
        Float32* out = &result[ y * targetRowSize ];
        const UInt32 tapEnd = vertical.start[ y + 1 ];
        for ( UInt32 t=vertical.start[ y ]; t<tapEnd; ++t )
        {
            const Float32* in = &intermediate[ (size_t) vertical.indices[ t ] * targetRowSize ];
            const Float32 weight = vertical.weights[ t ];
            for ( size_t i=0; i<targetRowSize; ++i )
                out[ i ] += in[ i ] * weight;
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CPixelMapResampler::Resample( const CPixelMap& source        ,
                              UInt32 targetWidth             ,
                              UInt32 targetHeight            ,
                              TResampleFilter filter         ,
                              TColourSpace colourSpace       ,
                              TWrapMode wrapMode             ,
                              TPixelMapPtr& result           )
{GUCEF_TRACE;

    if ( 0 == targetWidth || 0 == targetHeight )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PixelMapResampler: Cannot resample to a size of " << targetWidth << "x" << targetHeight );
        return false;
    }
    if ( PSF_PALETTE_INDICES == source.GetPixelStorageFormat() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PixelMapResampler: Palette pictures cannot be resampled, convert them to RGB or RGBA first" );
        return false;
    }
    if ( !CPixelMapChannelAccess::IsSupported( source ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PixelMapResampler: The storage format or data type of the picture is not supported" );
        return false;
    }

    const TPixelStorageFormat format = source.GetPixelStorageFormat();
    const TBuildinDataType dataType = source.GetPixelComponentDataType();
    const UInt32 channelCount = CPixelMap::GetChannelCountForFormat( format );

    if ( source.GetWidthInPixels() == targetWidth && source.GetHeightInPixels() == targetHeight )
    {
        result = TPixelMapPtr( GUCEF_NEW CPixelMap( source ) );
        return true;
    }

    CPixelMapChannelAccess::TFloatVector values, filtered;
    if ( !CPixelMapChannelAccess::Read( source, values ) )
        return false;
    CPixelMapChannelAccess::ToWorkingSpace( &values[ 0 ], source.GetPixelCount(), format, colourSpace );

    if ( !ResampleChannels( &values[ 0 ], source.GetWidthInPixels(), source.GetHeightInPixels(), channelCount, targetWidth, targetHeight, filter, wrapMode, filtered ) )
        return false;
    CPixelMapChannelAccess::TFloatVector().swap( values );

    CPixelMapChannelAccess::FromWorkingSpace( &filtered[ 0 ], targetWidth * targetHeight, format, colourSpace );

    TPixelMapPtr target( GUCEF_NEW CPixelMap( GUCEF_NULL, targetWidth, targetHeight, format, dataType ) );
    if ( !CPixelMapChannelAccess::Write( &filtered[ 0 ], *target ) )
        return false;
    result = target;
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
