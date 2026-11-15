/*
 *  gucefIMAGE_TestApp: Test application for the gucefIMAGE module
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

#include <string.h>
#include <math.h>

#ifndef GUCEF_TESTFW_H
#include "gucef_test_framework.h"
#define GUCEF_TESTFW_H
#endif /* GUCEF_TESTFW_H ? */

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_IMAGE_H
#include "gucefIMAGE.h"
#define GUCEF_IMAGE_H
#endif /* GUCEF_IMAGE_H ? */

#include "TestPixelMapResampler.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

namespace {

/** A single channel floating point picture of 'count' values in one row */
IMAGE::TPixelMapPtr
MakeRow( const Float32* values, UInt32 count )
{
    IMAGE::TPixelMapPtr row( GUCEF_NEW IMAGE::CPixelMap( GUCEF_NULL, count, 1, IMAGE::PSF_SINGLE_CHANNEL_GRAYSCALE, GUCEF::DATATYPE_LE_FLOAT32 ) );
    memcpy( row->GetDataPtr(), values, count * sizeof( Float32 ) );
    return row;
}

bool
RowMatches( const IMAGE::CPixelMap& row, const Float32* expected, UInt32 count, Float32 tolerance )
{
    if ( row.GetWidthInPixels() != count || row.GetHeightInPixels() != 1 || IMAGE::PSF_SINGLE_CHANNEL_GRAYSCALE != row.GetPixelStorageFormat() )
        return false;
    const Float32* values = static_cast< const Float32* >( row.GetDataPtr() );
    for ( UInt32 i=0; i<count; ++i )
        if ( fabs( values[ i ] - expected[ i ] ) > tolerance )
            return false;
    return true;
}

IMAGE::TPixelMapPtr
MakeRgba8( UInt32 width, UInt32 height, const UInt8* pixels )
{
    IMAGE::TPixelMapPtr map( GUCEF_NEW IMAGE::CPixelMap( pixels, width, height, IMAGE::PSF_RGBA, GUCEF::DATATYPE_UINT8 ) );
    return map;
}

Int32
Difference( UInt8 a, UInt8 b )
{
    return a > b ? (Int32) ( a - b ) : (Int32) ( b - a );
}

/** The data types and formats the resampler has to handle */
const GUCEF::TBuildinDataType g_dataTypes[] =
{
    GUCEF::DATATYPE_UINT8, GUCEF::DATATYPE_INT8, GUCEF::DATATYPE_LE_UINT16, GUCEF::DATATYPE_LE_INT16,
    GUCEF::DATATYPE_LE_UINT32, GUCEF::DATATYPE_LE_INT32, GUCEF::DATATYPE_LE_UINT64, GUCEF::DATATYPE_LE_INT64,
    GUCEF::DATATYPE_LE_FLOAT32, GUCEF::DATATYPE_LE_FLOAT64
};

const IMAGE::TPixelStorageFormat g_formats[] =
{
    IMAGE::PSF_RGB, IMAGE::PSF_RGBA, IMAGE::PSF_BGR, IMAGE::PSF_BGRA, IMAGE::PSF_HSV, IMAGE::PSF_HSVA, IMAGE::PSF_HSL, IMAGE::PSF_HSLA,
    IMAGE::PSF_SINGLE_CHANNEL_GRAYSCALE, IMAGE::PSF_SINGLE_CHANNEL_RED, IMAGE::PSF_SINGLE_CHANNEL_GREEN, IMAGE::PSF_SINGLE_CHANNEL_BLUE,
    IMAGE::PSF_SINGLE_CHANNEL_ALPHA, IMAGE::PSF_SINGLE_CHANNEL_STD_LUMINANCE, IMAGE::PSF_SINGLE_CHANNEL_P1_LUMINANCE,
    IMAGE::PSF_SINGLE_CHANNEL_P2_LUMINANCE, IMAGE::PSF_LUMINANCE_ALPHA
};

const IMAGE::TResampleFilter g_filters[] = { IMAGE::RESAMPLEFILTER_BOX, IMAGE::RESAMPLEFILTER_TRIANGLE, IMAGE::RESAMPLEFILTER_KAISER, IMAGE::RESAMPLEFILTER_LANCZOS3 };
const IMAGE::TWrapMode g_wrapModes[] = { IMAGE::WRAPMODE_CLAMP, IMAGE::WRAPMODE_REPEAT, IMAGE::WRAPMODE_MIRROR };

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformPixelMapResamplerTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "gucefIMAGE.PixelMapResampler" );

    GUCEF_TESTFW_TESTCASE( "Resampler: Golden values of small maps under each filter, down- and upscaling" )
    try
    {
        const Float32 down[ 6 ] = { 0.0f, 1.0f, 0.25f, 0.75f, 0.5f, 0.0f };
        const Float32 up[ 3 ] = { 0.2f, 0.9f, 0.4f };

        // Independent calculation of the same filters (see the design: box is area sampling, kaiser is a sinc with a Kaiser window of alpha 4 and support 3)
        const Float32 goldenDown[ 4 ][ 3 ] = { { 0.5f, 0.5f, 0.25f }, { 0.40625f, 0.5625f, 0.28125f }, { 0.410349f, 0.644065f, 0.271691f }, { 0.413284f, 0.649643f, 0.274164f } };
        const Float32 goldenUp[ 4 ][ 5 ] = { { 0.2f, 0.433333f, 0.9f, 0.566667f, 0.4f }, { 0.2f, 0.48f, 0.9f, 0.6f, 0.4f },
                                             { 0.124152f, 0.510409f, 0.9f, 0.651983f, 0.335794f }, { 0.12272f, 0.513049f, 0.9f, 0.656318f, 0.335374f } };
        for ( UInt32 f=0; f<4; ++f )
        {
            IMAGE::TPixelMapPtr result;
            IMAGE::TPixelMapPtr source = MakeRow( down, 6 );
            ASSERT_TRUE( IMAGE::CPixelMapResampler::Resample( *source, 3, 1, g_filters[ f ], IMAGE::COLOURSPACE_LINEAR, IMAGE::WRAPMODE_CLAMP, result ) );
            ASSERT_TRUE( !result.IsNULL() && RowMatches( *result, goldenDown[ f ], 3, 0.0002f ) );

            source = MakeRow( up, 3 );
            ASSERT_TRUE( IMAGE::CPixelMapResampler::Resample( *source, 5, 1, g_filters[ f ], IMAGE::COLOURSPACE_LINEAR, IMAGE::WRAPMODE_CLAMP, result ) );
            ASSERT_TRUE( !result.IsNULL() && RowMatches( *result, goldenUp[ f ], 5, 0.0002f ) );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Resampler: A constant picture stays constant, for every filter, wrap mode and direction" )
    try
    {
        for ( UInt32 f=0; f<4; ++f )
        {
            for ( UInt32 w=0; w<3; ++w )
            {
                for ( UInt32 colourSpace=0; colourSpace<2; ++colourSpace )
                {
                    const UInt32 sizes[ 3 ][ 4 ] = { { 8, 8, 3, 5 }, { 3, 3, 7, 7 }, { 5, 9, 5, 4 } };
                    for ( UInt32 s=0; s<3; ++s )
                    {
                        IMAGE::TPixelMapPtr source( GUCEF_NEW IMAGE::CPixelMap( GUCEF_NULL, sizes[ s ][ 0 ], sizes[ s ][ 1 ], IMAGE::PSF_RGBA, GUCEF::DATATYPE_UINT8 ) );
                        UInt8* data = static_cast< UInt8* >( source->GetDataPtr() );
                        for ( UInt32 i=0; i<source->GetPixelCount(); ++i )
                        {
                            data[ i * 4 ] = 200; data[ i * 4 + 1 ] = 100; data[ i * 4 + 2 ] = 50; data[ i * 4 + 3 ] = 255;
                        }
                        IMAGE::TPixelMapPtr result;
                        ASSERT_TRUE( IMAGE::CPixelMapResampler::Resample( *source, sizes[ s ][ 2 ], sizes[ s ][ 3 ], g_filters[ f ], 0 == colourSpace ? IMAGE::COLOURSPACE_LINEAR : IMAGE::COLOURSPACE_SRGB, g_wrapModes[ w ], result ) );
                        if ( result.IsNULL() )
                            continue;
                        ASSERT_TRUE( sizes[ s ][ 2 ] == result->GetWidthInPixels() && sizes[ s ][ 3 ] == result->GetHeightInPixels() );
                        const UInt8* out = static_cast< const UInt8* >( result->GetDataPtr() );
                        bool constant = true;
                        for ( UInt32 i=0; i<result->GetPixelCount(); ++i )
                            constant = constant && 200 == out[ i * 4 ] && 100 == out[ i * 4 + 1 ] && 50 == out[ i * 4 + 2 ] && 255 == out[ i * 4 + 3 ];
                        ASSERT_TRUE( constant );
                    }
                }
            }
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Resampler: A 2:1 box downscale equals the average of the 2x2 blocks" )
    try
    {
        // Floating point: exact up to rounding of the sums
        Float32 values[ 16 ];
        for ( UInt32 i=0; i<16; ++i )
            values[ i ] = (Float32) ( ( i * 7 + 3 ) % 11 ) / 10.0f;
        IMAGE::TPixelMapPtr source( GUCEF_NEW IMAGE::CPixelMap( values, 4, 4, IMAGE::PSF_SINGLE_CHANNEL_GRAYSCALE, GUCEF::DATATYPE_LE_FLOAT32 ) ), result;
        ASSERT_TRUE( IMAGE::CPixelMapResampler::Resample( *source, 2, 2, IMAGE::RESAMPLEFILTER_BOX, IMAGE::COLOURSPACE_LINEAR, IMAGE::WRAPMODE_CLAMP, result ) );
        if ( !result.IsNULL() )
        {
            const Float32* out = static_cast< const Float32* >( result->GetDataPtr() );
            for ( UInt32 y=0; y<2; ++y )
                for ( UInt32 x=0; x<2; ++x )
                {
                    Float32 expected = ( values[ y * 8 + x * 2 ] + values[ y * 8 + x * 2 + 1 ] + values[ y * 8 + 4 + x * 2 ] + values[ y * 8 + 4 + x * 2 + 1 ] ) / 4.0f;
                    ASSERT_TRUE( fabs( out[ y * 2 + x ] - expected ) < 0.00001f );
                }
        }

        // 8 bit: within the rounding of the result
        UInt8 pixels[ 16 * 4 ];
        for ( UInt32 i=0; i<16 * 4; ++i )
            pixels[ i ] = (UInt8) ( ( i * 37 + 11 ) % 256 );
        source = MakeRgba8( 4, 4, pixels );
        ASSERT_TRUE( IMAGE::CPixelMapResampler::Resample( *source, 2, 2, IMAGE::RESAMPLEFILTER_BOX, IMAGE::COLOURSPACE_LINEAR, IMAGE::WRAPMODE_CLAMP, result ) );
        if ( !result.IsNULL() )
        {
            const UInt8* out = static_cast< const UInt8* >( result->GetDataPtr() );
            for ( UInt32 y=0; y<2; ++y )
                for ( UInt32 x=0; x<2; ++x )
                    for ( UInt32 c=0; c<4; ++c )
                    {
                        UInt32 sum = pixels[ ( ( y * 2 ) * 4 + x * 2 ) * 4 + c ] + pixels[ ( ( y * 2 ) * 4 + x * 2 + 1 ) * 4 + c ] +
                                     pixels[ ( ( y * 2 + 1 ) * 4 + x * 2 ) * 4 + c ] + pixels[ ( ( y * 2 + 1 ) * 4 + x * 2 + 1 ) * 4 + c ];
                        ASSERT_TRUE( Difference( out[ ( y * 2 + x ) * 4 + c ], (UInt8) ( ( sum + 2 ) / 4 ) ) <= 1 );
                    }
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Resampler: sRGB colour is averaged in linear light, linear data as it is" )
    try
    {
        const UInt8 blackAndWhite[ 8 ] = { 0, 0, 0, 255, 255, 255, 255, 255 };
        IMAGE::TPixelMapPtr source = MakeRgba8( 2, 1, blackAndWhite ), srgb, linear;
        ASSERT_TRUE( IMAGE::CPixelMapResampler::Resample( *source, 1, 1, IMAGE::RESAMPLEFILTER_BOX, IMAGE::COLOURSPACE_SRGB, IMAGE::WRAPMODE_CLAMP, srgb ) );
        ASSERT_TRUE( IMAGE::CPixelMapResampler::Resample( *source, 1, 1, IMAGE::RESAMPLEFILTER_BOX, IMAGE::COLOURSPACE_LINEAR, IMAGE::WRAPMODE_CLAMP, linear ) );
        if ( !srgb.IsNULL() && !linear.IsNULL() )
        {
            const UInt8* out = static_cast< const UInt8* >( srgb->GetDataPtr() );
            ASSERT_TRUE( out[ 0 ] >= 187 && out[ 0 ] <= 188 && out[ 0 ] == out[ 1 ] && out[ 1 ] == out[ 2 ] );
            ASSERT_TRUE( 255 == out[ 3 ] );
            out = static_cast< const UInt8* >( linear->GetDataPtr() );
            ASSERT_TRUE( out[ 0 ] >= 127 && out[ 0 ] <= 128 );
        }

        // The alpha channel is never converted: black and white alpha average to the plain mean
        const UInt8 alphaPair[ 8 ] = { 255, 255, 255, 0, 255, 255, 255, 255 };
        source = MakeRgba8( 2, 1, alphaPair );
        ASSERT_TRUE( IMAGE::CPixelMapResampler::Resample( *source, 1, 1, IMAGE::RESAMPLEFILTER_BOX, IMAGE::COLOURSPACE_SRGB, IMAGE::WRAPMODE_CLAMP, srgb ) );
        if ( !srgb.IsNULL() )
        {
            ASSERT_TRUE( Difference( 128, static_cast< const UInt8* >( srgb->GetDataPtr() )[ 3 ] ) <= 1 );
        }

        // The transfer functions are exact inverses and match the standard at its break point and at 0.5
        ASSERT_TRUE( fabs( IMAGE::CPixelMapChannelAccess::SrgbToLinear( 0.5f ) - 0.21404114f ) < 0.00001f );
        ASSERT_TRUE( fabs( IMAGE::CPixelMapChannelAccess::SrgbToLinear( 0.04045f ) - 0.0031308f ) < 0.00001f );
        for ( UInt32 i=0; i<=100; ++i )
        {
            Float32 value = (Float32) i / 100.0f;
            ASSERT_TRUE( fabs( IMAGE::CPixelMapChannelAccess::LinearToSrgb( IMAGE::CPixelMapChannelAccess::SrgbToLinear( value ) ) - value ) < 0.00001f );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Resampler: The wrap modes decide what the filter sees beyond the edges" )
    try
    {
        // A triangle upscale of four values: the first output looks one pixel beyond the left edge
        const Float32 values[ 4 ] = { 1.0f, 0.5f, 0.25f, 0.0f };
        IMAGE::TPixelMapPtr source = MakeRow( values, 4 ), clamp, repeat, mirror;
        ASSERT_TRUE( IMAGE::CPixelMapResampler::Resample( *source, 8, 1, IMAGE::RESAMPLEFILTER_TRIANGLE, IMAGE::COLOURSPACE_LINEAR, IMAGE::WRAPMODE_CLAMP, clamp ) );
        ASSERT_TRUE( IMAGE::CPixelMapResampler::Resample( *source, 8, 1, IMAGE::RESAMPLEFILTER_TRIANGLE, IMAGE::COLOURSPACE_LINEAR, IMAGE::WRAPMODE_REPEAT, repeat ) );
        ASSERT_TRUE( IMAGE::CPixelMapResampler::Resample( *source, 8, 1, IMAGE::RESAMPLEFILTER_TRIANGLE, IMAGE::COLOURSPACE_LINEAR, IMAGE::WRAPMODE_MIRROR, mirror ) );
        if ( !clamp.IsNULL() && !repeat.IsNULL() && !mirror.IsNULL() )
        {
            const Float32* c = static_cast< const Float32* >( clamp->GetDataPtr() );
            const Float32* r = static_cast< const Float32* >( repeat->GetDataPtr() );
            const Float32* m = static_cast< const Float32* >( mirror->GetDataPtr() );
            // taps at -1 (weight .25) and 0 (weight .75); -1 is pixel 0 when clamped or mirrored, pixel 3 when repeated
            ASSERT_TRUE( fabs( c[ 0 ] - 1.0f ) < 0.00001f && fabs( m[ 0 ] - 1.0f ) < 0.00001f );
            ASSERT_TRUE( fabs( r[ 0 ] - ( 0.25f * 0.0f + 0.75f * 1.0f ) ) < 0.00001f );
            // and at the right edge: taps at 3 (.75) and 4 (.25); 4 is pixel 3 when clamped or mirrored, pixel 0 when repeated
            ASSERT_TRUE( fabs( c[ 7 ] - 0.0f ) < 0.00001f && fabs( m[ 7 ] - 0.0f ) < 0.00001f );
            ASSERT_TRUE( fabs( r[ 7 ] - 0.25f * 1.0f ) < 0.00001f );
        }

        // With a wider filter all three differ at the edge
        IMAGE::TPixelMapPtr kClamp, kRepeat, kMirror;
        ASSERT_TRUE( IMAGE::CPixelMapResampler::Resample( *source, 8, 1, IMAGE::RESAMPLEFILTER_KAISER, IMAGE::COLOURSPACE_LINEAR, IMAGE::WRAPMODE_CLAMP, kClamp ) );
        ASSERT_TRUE( IMAGE::CPixelMapResampler::Resample( *source, 8, 1, IMAGE::RESAMPLEFILTER_KAISER, IMAGE::COLOURSPACE_LINEAR, IMAGE::WRAPMODE_REPEAT, kRepeat ) );
        ASSERT_TRUE( IMAGE::CPixelMapResampler::Resample( *source, 8, 1, IMAGE::RESAMPLEFILTER_KAISER, IMAGE::COLOURSPACE_LINEAR, IMAGE::WRAPMODE_MIRROR, kMirror ) );
        if ( !kClamp.IsNULL() && !kRepeat.IsNULL() && !kMirror.IsNULL() )
        {
            Float32 c = *static_cast< const Float32* >( kClamp->GetDataPtr() ), r = *static_cast< const Float32* >( kRepeat->GetDataPtr() ), m = *static_cast< const Float32* >( kMirror->GetDataPtr() );
            ASSERT_TRUE( fabs( c - r ) > 0.001f && fabs( c - m ) > 0.001f && fabs( r - m ) > 0.001f );
        }

        // A tiling picture stays seamless: scaling a repeated picture 2:1 with repeat wrapping gives the same on both sides of the seam
        const Float32 tile[ 4 ] = { 0.9f, 0.1f, 0.4f, 0.7f };
        IMAGE::TPixelMapPtr tileSource = MakeRow( tile, 4 ), tiled;
        ASSERT_TRUE( IMAGE::CPixelMapResampler::Resample( *tileSource, 2, 1, IMAGE::RESAMPLEFILTER_KAISER, IMAGE::COLOURSPACE_LINEAR, IMAGE::WRAPMODE_REPEAT, tiled ) );
        const Float32 doubled[ 8 ] = { 0.9f, 0.1f, 0.4f, 0.7f, 0.9f, 0.1f, 0.4f, 0.7f };
        IMAGE::TPixelMapPtr doubledSource = MakeRow( doubled, 8 ), doubledResult;
        ASSERT_TRUE( IMAGE::CPixelMapResampler::Resample( *doubledSource, 4, 1, IMAGE::RESAMPLEFILTER_KAISER, IMAGE::COLOURSPACE_LINEAR, IMAGE::WRAPMODE_REPEAT, doubledResult ) );
        if ( !tiled.IsNULL() && !doubledResult.IsNULL() )
        {
            const Float32* a = static_cast< const Float32* >( tiled->GetDataPtr() );
            const Float32* b = static_cast< const Float32* >( doubledResult->GetDataPtr() );
            ASSERT_TRUE( fabs( a[ 0 ] - b[ 0 ] ) < 0.0001f && fabs( a[ 1 ] - b[ 1 ] ) < 0.0001f && fabs( a[ 0 ] - b[ 2 ] ) < 0.0001f && fabs( a[ 1 ] - b[ 3 ] ) < 0.0001f );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Resampler: Every storage format and data type gives a result in its own format" )
    try
    {
        const UInt32 formatCount = sizeof( g_formats ) / sizeof( g_formats[ 0 ] );
        const UInt32 typeCount = sizeof( g_dataTypes ) / sizeof( g_dataTypes[ 0 ] );
        for ( UInt32 f=0; f<formatCount; ++f )
        {
            for ( UInt32 t=0; t<typeCount; ++t )
            {
                IMAGE::TPixelMapPtr source( GUCEF_NEW IMAGE::CPixelMap( GUCEF_NULL, 6, 4, g_formats[ f ], g_dataTypes[ t ] ) );
                const UInt32 valueSize = IMAGE::CPixelMap::GetPixelChannelSize( g_dataTypes[ t ] );
                UInt8* data = static_cast< UInt8* >( source->GetDataPtr() );
                const bool hue = IMAGE::CPixelMapChannelAccess::IsHueBased( g_formats[ f ] );
                for ( UInt64 i=0; i<source->GetTotalNumberOfChannelComponentValues(); ++i )
                    IMAGE::CPixelMapChannelAccess::WriteValue( hue ? 0.4f : 0.75f, g_dataTypes[ t ], data + i * valueSize );

                IMAGE::TPixelMapPtr down, up;
                ASSERT_TRUE( IMAGE::CPixelMapResampler::Resample( *source, 3, 2, IMAGE::RESAMPLEFILTER_KAISER, IMAGE::COLOURSPACE_SRGB, IMAGE::WRAPMODE_CLAMP, down ) );
                ASSERT_TRUE( IMAGE::CPixelMapResampler::Resample( *source, 9, 7, IMAGE::RESAMPLEFILTER_LANCZOS3, IMAGE::COLOURSPACE_LINEAR, IMAGE::WRAPMODE_REPEAT, up ) );
                if ( down.IsNULL() || up.IsNULL() )
                    continue;
                ASSERT_TRUE( g_formats[ f ] == down->GetPixelStorageFormat() && g_dataTypes[ t ] == down->GetPixelComponentDataType() && 3 == down->GetWidthInPixels() && 2 == down->GetHeightInPixels() );
                ASSERT_TRUE( g_formats[ f ] == up->GetPixelStorageFormat() && g_dataTypes[ t ] == up->GetPixelComponentDataType() && 9 == up->GetWidthInPixels() && 7 == up->GetHeightInPixels() );

                // a constant picture stays (about) constant in the plain formats; the hue formats are only checked for their range
                const UInt8* out = static_cast< const UInt8* >( down->GetDataPtr() );
                bool closeEnough = true;
                for ( UInt64 i=0; i<down->GetTotalNumberOfChannelComponentValues(); ++i )
                {
                    Float32 value = IMAGE::CPixelMapChannelAccess::ReadValue( out + i * valueSize, g_dataTypes[ t ] );
                    closeEnough = closeEnough && ( hue ? ( value >= 0.0f && value <= 1.0f ) : fabs( value - 0.75f ) < 0.01f );
                }
                ASSERT_TRUE( closeEnough );
            }
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Resampler: Palette input and unusable sizes and formats are refused" )
    try
    {
        IMAGE::TPixelMapPtr result;
        IMAGE::CPixelMap palettised( GUCEF_NULL, 4, 4, IMAGE::PSF_PALETTE_INDICES, GUCEF::DATATYPE_UINT8 );
        ASSERT_FALSE( IMAGE::CPixelMapResampler::Resample( palettised, 2, 2, IMAGE::RESAMPLEFILTER_BOX, IMAGE::COLOURSPACE_LINEAR, IMAGE::WRAPMODE_CLAMP, result ) );
        ASSERT_TRUE( result.IsNULL() );

        IMAGE::CPixelMap rgb( GUCEF_NULL, 4, 4, IMAGE::PSF_RGB, GUCEF::DATATYPE_UINT8 );
        ASSERT_FALSE( IMAGE::CPixelMapResampler::Resample( rgb, 0, 2, IMAGE::RESAMPLEFILTER_BOX, IMAGE::COLOURSPACE_LINEAR, IMAGE::WRAPMODE_CLAMP, result ) );
        ASSERT_FALSE( IMAGE::CPixelMapResampler::Resample( rgb, 2, 0, IMAGE::RESAMPLEFILTER_BOX, IMAGE::COLOURSPACE_LINEAR, IMAGE::WRAPMODE_CLAMP, result ) );

        IMAGE::CPixelMap bigEndian( GUCEF_NULL, 4, 4, IMAGE::PSF_RGB, GUCEF::DATATYPE_BE_UINT16 );
        ASSERT_FALSE( IMAGE::CPixelMapResampler::Resample( bigEndian, 2, 2, IMAGE::RESAMPLEFILTER_BOX, IMAGE::COLOURSPACE_LINEAR, IMAGE::WRAPMODE_CLAMP, result ) );

        IMAGE::CPixelMap empty( GUCEF_NULL, 0, 0, IMAGE::PSF_RGB, GUCEF::DATATYPE_UINT8 );
        ASSERT_FALSE( IMAGE::CPixelMapResampler::Resample( empty, 2, 2, IMAGE::RESAMPLEFILTER_BOX, IMAGE::COLOURSPACE_LINEAR, IMAGE::WRAPMODE_CLAMP, result ) );
        ASSERT_TRUE( result.IsNULL() );

        // The same size gives an equal copy
        ASSERT_TRUE( IMAGE::CPixelMapResampler::Resample( rgb, 4, 4, IMAGE::RESAMPLEFILTER_LANCZOS3, IMAGE::COLOURSPACE_SRGB, IMAGE::WRAPMODE_CLAMP, result ) );
        ASSERT_TRUE( !result.IsNULL() && &(*result) != &rgb && 4 == result->GetWidthInPixels() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Resampler: Lanczos ringing is clamped to the range of the data type, hue maps keep their colours" )
    try
    {
        // A hard edge makes lanczos overshoot; 8 bit values cannot leave 0 .. 255 and floats are left alone
        UInt8 edge[ 8 * 4 ];
        for ( UInt32 x=0; x<8; ++x )
        {
            UInt8 value = x < 4 ? 0 : 255;
            edge[ x * 4 ] = value; edge[ x * 4 + 1 ] = value; edge[ x * 4 + 2 ] = value; edge[ x * 4 + 3 ] = 255;
        }
        IMAGE::TPixelMapPtr source = MakeRgba8( 8, 1, edge ), result;
        ASSERT_TRUE( IMAGE::CPixelMapResampler::Resample( *source, 32, 1, IMAGE::RESAMPLEFILTER_LANCZOS3, IMAGE::COLOURSPACE_LINEAR, IMAGE::WRAPMODE_CLAMP, result ) );
        if ( !result.IsNULL() )
        {
            // The same picture as floating point shows the overshoot that the 8 bit result has to cut off
            Float32 edgeFloats[ 8 * 4 ];
            for ( UInt32 i=0; i<8 * 4; ++i )
                edgeFloats[ i ] = (Float32) edge[ i ] / 255.0f;
            IMAGE::TPixelMapPtr floatEdge( GUCEF_NEW IMAGE::CPixelMap( edgeFloats, 8, 1, IMAGE::PSF_RGBA, GUCEF::DATATYPE_LE_FLOAT32 ) ), floatEdgeResult;
            ASSERT_TRUE( IMAGE::CPixelMapResampler::Resample( *floatEdge, 32, 1, IMAGE::RESAMPLEFILTER_LANCZOS3, IMAGE::COLOURSPACE_LINEAR, IMAGE::WRAPMODE_CLAMP, floatEdgeResult ) );
            if ( !floatEdgeResult.IsNULL() )
            {
                const UInt8* out = static_cast< const UInt8* >( result->GetDataPtr() );
                const Float32* reference = static_cast< const Float32* >( floatEdgeResult->GetDataPtr() );
                bool clampedLikeFloat = true, overshot = false;
                for ( UInt32 i=0; i<32 * 4; ++i )
                {
                    Float32 expected = reference[ i ] < 0.0f ? 0.0f : ( reference[ i ] > 1.0f ? 1.0f : reference[ i ] );
                    overshot = overshot || reference[ i ] < 0.0f || reference[ i ] > 1.0f;
                    clampedLikeFloat = clampedLikeFloat && fabs( (Float32) out[ i ] - expected * 255.0f ) <= 1.0f;
                }
                ASSERT_TRUE( overshot && clampedLikeFloat );
            }
        }

        const Float32 hard[ 8 ] = { 0, 0, 0, 0, 1, 1, 1, 1 };
        IMAGE::TPixelMapPtr floats = MakeRow( hard, 8 ), floatResult;
        ASSERT_TRUE( IMAGE::CPixelMapResampler::Resample( *floats, 32, 1, IMAGE::RESAMPLEFILTER_LANCZOS3, IMAGE::COLOURSPACE_LINEAR, IMAGE::WRAPMODE_CLAMP, floatResult ) );
        if ( !floatResult.IsNULL() )
        {
            const Float32* out = static_cast< const Float32* >( floatResult->GetDataPtr() );
            bool overshoot = false;
            for ( UInt32 x=0; x<32; ++x )
                overshoot = overshoot || out[ x ] > 1.0f || out[ x ] < 0.0f;
            ASSERT_TRUE( overshoot );
        }

        // HSV: a constant picture keeps its hue, saturation and value
        IMAGE::TPixelMapPtr hsv( GUCEF_NEW IMAGE::CPixelMap( GUCEF_NULL, 4, 4, IMAGE::PSF_HSV, GUCEF::DATATYPE_LE_FLOAT32 ) ), hsvResult;
        Float32* hsvData = static_cast< Float32* >( hsv->GetDataPtr() );
        for ( UInt32 i=0; i<16; ++i )
        {
            hsvData[ i * 3 ] = 0.6f; hsvData[ i * 3 + 1 ] = 0.5f; hsvData[ i * 3 + 2 ] = 0.8f;
        }
        ASSERT_TRUE( IMAGE::CPixelMapResampler::Resample( *hsv, 2, 2, IMAGE::RESAMPLEFILTER_TRIANGLE, IMAGE::COLOURSPACE_LINEAR, IMAGE::WRAPMODE_CLAMP, hsvResult ) );
        if ( !hsvResult.IsNULL() )
        {
            const Float32* out = static_cast< const Float32* >( hsvResult->GetDataPtr() );
            ASSERT_TRUE( fabs( out[ 0 ] - 0.6f ) < 0.001f && fabs( out[ 1 ] - 0.5f ) < 0.001f && fabs( out[ 2 ] - 0.8f ) < 0.001f );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
