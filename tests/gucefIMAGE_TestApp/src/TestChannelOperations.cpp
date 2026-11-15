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

#include "TestChannelOperations.h"

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

/** A 2x2 picture with known values: pixel i has channels (10+i, 50+i, 90+i, 130+i) */
IMAGE::TPixelMapPtr
MakeKnownMap( IMAGE::TPixelStorageFormat format = IMAGE::PSF_RGBA )
{
    UInt8 pixels[ 16 ];
    for ( UInt32 i=0; i<4; ++i )
        for ( UInt32 c=0; c<4; ++c )
            pixels[ i * 4 + c ] = (UInt8) ( 10 + c * 40 + i );
    return IMAGE::TPixelMapPtr( GUCEF_NEW IMAGE::CPixelMap( pixels, 2, 2, format, GUCEF::DATATYPE_UINT8 ) );
}

const UInt8*
Bytes( const IMAGE::CPixelMap& map )
{
    return static_cast< const UInt8* >( map.GetDataPtr() );
}

UInt8
ExpectedScaleBias( UInt8 value, Float32 scale, Float32 bias )
{
    Float32 result = ( (Float32) value / 255.0f ) * scale + bias;
    result = result < 0.0f ? 0.0f : ( result > 1.0f ? 1.0f : result );
    return (UInt8) ( result * 255.0f + 0.5f );
}

Int32
Difference( UInt8 a, UInt8 b )
{
    return a > b ? (Int32) ( a - b ) : (Int32) ( b - a );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformChannelOperationsTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "gucefIMAGE.ChannelOperations" );

    GUCEF_TESTFW_TESTCASE( "ChannelOperations: ExtractChannel gives a single channel map of one channel" )
    try
    {
        IMAGE::TPixelMapPtr source = MakeKnownMap(), result;
        for ( UInt32 c=0; c<4; ++c )
        {
            ASSERT_TRUE( IMAGE::CChannelOperations::ExtractChannel( *source, c, result ) );
            ASSERT_TRUE( !result.IsNULL() && IMAGE::PSF_SINGLE_CHANNEL_GRAYSCALE == result->GetPixelStorageFormat() && 2 == result->GetWidthInPixels() && 2 == result->GetHeightInPixels() );
            if ( !result.IsNULL() )
                for ( UInt32 i=0; i<4; ++i )
                    ASSERT_TRUE( 10 + c * 40 + i == Bytes( *result )[ i ] );
        }
        ASSERT_FALSE( IMAGE::CChannelOperations::ExtractChannel( *source, 4, result ) );

        // the source stays as it is, the in place form changes the map
        ASSERT_TRUE( IMAGE::PSF_RGBA == source->GetPixelStorageFormat() );
        ASSERT_TRUE( IMAGE::CChannelOperations::ExtractChannel( *source, 3 ) );
        ASSERT_TRUE( IMAGE::PSF_SINGLE_CHANNEL_GRAYSCALE == source->GetPixelStorageFormat() && 130 == Bytes( *source )[ 0 ] && 133 == Bytes( *source )[ 3 ] );
        ASSERT_FALSE( IMAGE::CChannelOperations::ExtractChannel( *source, 1 ) );

        // lossless for wide data types too
        IMAGE::CPixelMap wide( GUCEF_NULL, 1, 1, IMAGE::PSF_RGB, GUCEF::DATATYPE_LE_UINT32 );
        UInt32* wideData = static_cast< UInt32* >( wide.GetDataPtr() );
        wideData[ 0 ] = 4000000001u; wideData[ 1 ] = 4000000003u; wideData[ 2 ] = 4000000005u;
        ASSERT_TRUE( IMAGE::CChannelOperations::ExtractChannel( wide, 1, result ) && 4000000003u == *static_cast< const UInt32* >( result->GetDataPtr() ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "ChannelOperations: Swizzle reorders channels and makes constants" )
    try
    {
        IMAGE::TPixelMapPtr source = MakeKnownMap(), result;

        ASSERT_TRUE( IMAGE::CChannelOperations::Swizzle( *source, "GGG1", result ) );
        ASSERT_TRUE( !result.IsNULL() && IMAGE::PSF_RGBA == result->GetPixelStorageFormat() );
        if ( !result.IsNULL() )
            for ( UInt32 i=0; i<4; ++i )
                ASSERT_TRUE( 50 + i == Bytes( *result )[ i * 4 ] && 50 + i == Bytes( *result )[ i * 4 + 1 ] && 50 + i == Bytes( *result )[ i * 4 + 2 ] && 255 == Bytes( *result )[ i * 4 + 3 ] );

        ASSERT_TRUE( IMAGE::CChannelOperations::Swizzle( *source, "BGRA", result ) );
        if ( !result.IsNULL() )
            ASSERT_TRUE( 90 == Bytes( *result )[ 0 ] && 50 == Bytes( *result )[ 1 ] && 10 == Bytes( *result )[ 2 ] && 130 == Bytes( *result )[ 3 ] && 91 == Bytes( *result )[ 4 ] );

        ASSERT_TRUE( IMAGE::CChannelOperations::Swizzle( *source, "ARG0", result ) );
        if ( !result.IsNULL() )
            ASSERT_TRUE( 130 == Bytes( *result )[ 0 ] && 10 == Bytes( *result )[ 1 ] && 50 == Bytes( *result )[ 2 ] && 0 == Bytes( *result )[ 3 ] );

        // three channels and one channel
        ASSERT_TRUE( IMAGE::CChannelOperations::Swizzle( *source, "bgr", result ) );
        ASSERT_TRUE( !result.IsNULL() && IMAGE::PSF_RGB == result->GetPixelStorageFormat() && 90 == Bytes( *result )[ 0 ] && 10 == Bytes( *result )[ 2 ] );
        ASSERT_TRUE( IMAGE::CChannelOperations::Swizzle( *source, "A", result ) );
        ASSERT_TRUE( !result.IsNULL() && IMAGE::PSF_SINGLE_CHANNEL_GRAYSCALE == result->GetPixelStorageFormat() && 131 == Bytes( *result )[ 1 ] );

        // the letters are channels, wherever the format stores them: red of a BGRA map is its third channel
        IMAGE::TPixelMapPtr bgra = MakeKnownMap( IMAGE::PSF_BGRA );
        ASSERT_TRUE( IMAGE::CChannelOperations::Swizzle( *bgra, "RGBA", result ) );
        if ( !result.IsNULL() )
            ASSERT_TRUE( 90 == Bytes( *result )[ 0 ] && 50 == Bytes( *result )[ 1 ] && 10 == Bytes( *result )[ 2 ] && 130 == Bytes( *result )[ 3 ] );

        // a one channel map has the one channel as red, green and blue but no alpha
        IMAGE::TPixelMapPtr grey( GUCEF_NEW IMAGE::CPixelMap( GUCEF_NULL, 1, 1, IMAGE::PSF_SINGLE_CHANNEL_GRAYSCALE, GUCEF::DATATYPE_UINT8 ) );
        *static_cast< UInt8* >( grey->GetDataPtr() ) = 77;
        ASSERT_TRUE( IMAGE::CChannelOperations::Swizzle( *grey, "RGB1", result ) );
        ASSERT_TRUE( !result.IsNULL() && 77 == Bytes( *result )[ 0 ] && 77 == Bytes( *result )[ 2 ] && 255 == Bytes( *result )[ 3 ] );
        ASSERT_FALSE( IMAGE::CChannelOperations::Swizzle( *grey, "RGBA", result ) );

        // refused mappings
        IMAGE::TPixelMapPtr rgb;
        ASSERT_TRUE( IMAGE::CChannelOperations::Swizzle( *source, "RGB", rgb ) );
        ASSERT_FALSE( IMAGE::CChannelOperations::Swizzle( *rgb, "RGBA", result ) );
        ASSERT_FALSE( IMAGE::CChannelOperations::Swizzle( *source, "RG", result ) );
        ASSERT_FALSE( IMAGE::CChannelOperations::Swizzle( *source, "RGBAR", result ) );
        ASSERT_FALSE( IMAGE::CChannelOperations::Swizzle( *source, "RXB1", result ) );
        ASSERT_FALSE( IMAGE::CChannelOperations::Swizzle( *source, "", result ) );

        // floating point constants: 1 is 1.0
        IMAGE::CPixelMap floats( GUCEF_NULL, 1, 1, IMAGE::PSF_RGB, GUCEF::DATATYPE_LE_FLOAT32 );
        ASSERT_TRUE( IMAGE::CChannelOperations::Swizzle( floats, "R01A" ) == false );
        ASSERT_TRUE( IMAGE::CChannelOperations::Swizzle( floats, "R01" ) );
        ASSERT_TRUE( IMAGE::PSF_RGB == floats.GetPixelStorageFormat() && 1.0f == static_cast< const Float32* >( floats.GetDataPtr() )[ 2 ] );

        // the in place form
        ASSERT_TRUE( IMAGE::CChannelOperations::Swizzle( *source, "BGRA" ) );
        ASSERT_TRUE( 90 == Bytes( *source )[ 0 ] && 10 == Bytes( *source )[ 2 ] );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "ChannelOperations: Invert and ScaleBias act on the channels of the mask only" )
    try
    {
        IMAGE::TPixelMapPtr source = MakeKnownMap(), result;

        ASSERT_TRUE( IMAGE::CChannelOperations::Invert( *source, 0x2, result ) );
        ASSERT_TRUE( !result.IsNULL() && 10 == Bytes( *result )[ 0 ] && 255 - 50 == Bytes( *result )[ 1 ] && 90 == Bytes( *result )[ 2 ] && 130 == Bytes( *result )[ 3 ] );
        ASSERT_TRUE( 50 == Bytes( *source )[ 1 ] );
        ASSERT_TRUE( IMAGE::CChannelOperations::Invert( *source, 0x9 ) );
        ASSERT_TRUE( 245 == Bytes( *source )[ 0 ] && 50 == Bytes( *source )[ 1 ] && 90 == Bytes( *source )[ 2 ] && 125 == Bytes( *source )[ 3 ] );
        ASSERT_TRUE( IMAGE::CChannelOperations::Invert( *source, 0xFFFFFFFF ) );
        ASSERT_TRUE( 10 == Bytes( *source )[ 0 ] && 205 == Bytes( *source )[ 1 ] && 165 == Bytes( *source )[ 2 ] && 130 == Bytes( *source )[ 3 ] );

        // floating point: 1 - value; signed integers: the negated value
        IMAGE::CPixelMap floats( GUCEF_NULL, 1, 1, IMAGE::PSF_RGB, GUCEF::DATATYPE_LE_FLOAT32 );
        Float32* floatData = static_cast< Float32* >( floats.GetDataPtr() );
        floatData[ 0 ] = 0.25f; floatData[ 1 ] = 0.5f; floatData[ 2 ] = 1.0f;
        ASSERT_TRUE( IMAGE::CChannelOperations::Invert( floats, 0x7 ) );
        ASSERT_TRUE( 0.75f == floatData[ 0 ] && 0.5f == floatData[ 1 ] && 0.0f == floatData[ 2 ] );
        IMAGE::CPixelMap signedMap( GUCEF_NULL, 1, 1, IMAGE::PSF_RGB, GUCEF::DATATYPE_INT8 );
        Int8* signedData = static_cast< Int8* >( signedMap.GetDataPtr() );
        signedData[ 0 ] = 100; signedData[ 1 ] = -50; signedData[ 2 ] = 0;
        ASSERT_TRUE( IMAGE::CChannelOperations::Invert( signedMap, 0x7 ) );
        ASSERT_TRUE( -100 == signedData[ 0 ] && 50 == signedData[ 1 ] && 0 == signedData[ 2 ] );

        // ScaleBias: value * scale + bias, clamped
        source = MakeKnownMap();
        ASSERT_TRUE( IMAGE::CChannelOperations::ScaleBias( *source, 0x5, 2.0f, -0.25f, result ) );
        if ( !result.IsNULL() )
        {
            for ( UInt32 i=0; i<4; ++i )
            {
                ASSERT_TRUE( Difference( ExpectedScaleBias( (UInt8) ( 10 + i ), 2.0f, -0.25f ), Bytes( *result )[ i * 4 ] ) <= 1 );
                ASSERT_TRUE( 50 + i == Bytes( *result )[ i * 4 + 1 ] );
                ASSERT_TRUE( Difference( ExpectedScaleBias( (UInt8) ( 90 + i ), 2.0f, -0.25f ), Bytes( *result )[ i * 4 + 2 ] ) <= 1 );
                ASSERT_TRUE( 130 + i == Bytes( *result )[ i * 4 + 3 ] );
            }
        }
        ASSERT_TRUE( 10 == Bytes( *source )[ 0 ] );
        // the values that leave the range are clamped
        ASSERT_TRUE( IMAGE::CChannelOperations::ScaleBias( *source, 0xF, 4.0f, 0.0f ) );
        ASSERT_TRUE( 255 == Bytes( *source )[ 3 ] && 255 == Bytes( *source )[ 2 ] && 0 < Bytes( *source )[ 0 ] );
        ASSERT_TRUE( IMAGE::CChannelOperations::ScaleBias( *source, 0xF, 1.0f, -1.0f ) );
        ASSERT_TRUE( 0 == Bytes( *source )[ 2 ] && 0 == Bytes( *source )[ 3 ] );

        // the HDRP mask map remap: (value * 2 - 1) stored back as 0..1 is the identity of scale 1 and bias 0, a 0..1 to 0.25..0.75 squeeze otherwise
        IMAGE::CPixelMap squeeze( GUCEF_NULL, 2, 1, IMAGE::PSF_SINGLE_CHANNEL_GRAYSCALE, GUCEF::DATATYPE_LE_FLOAT32 );
        Float32* squeezeData = static_cast< Float32* >( squeeze.GetDataPtr() );
        squeezeData[ 0 ] = 0.0f; squeezeData[ 1 ] = 1.0f;
        ASSERT_TRUE( IMAGE::CChannelOperations::ScaleBias( squeeze, 0x1, 0.5f, 0.25f ) );
        ASSERT_TRUE( 0.25f == squeezeData[ 0 ] && 0.75f == squeezeData[ 1 ] );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "ChannelOperations: Pack builds an RGBA map from channels of up to four sources, NULL gives a constant" )
    try
    {
        IMAGE::TPixelMapPtr a = MakeKnownMap(), b = MakeKnownMap(), result;
        IMAGE::TPixelMapPtr sources[ 4 ] = { a, IMAGE::TPixelMapPtr(), b, IMAGE::TPixelMapPtr() };
        UInt32 channels[ 4 ] = { 3, 0, 1, 0 };
        ASSERT_TRUE( IMAGE::CChannelOperations::Pack( sources, channels, result ) );
        ASSERT_TRUE( !result.IsNULL() && IMAGE::PSF_RGBA == result->GetPixelStorageFormat() && 2 == result->GetWidthInPixels() && 2 == result->GetHeightInPixels() && GUCEF::DATATYPE_UINT8 == result->GetPixelComponentDataType() );
        if ( !result.IsNULL() )
            for ( UInt32 i=0; i<4; ++i )
                ASSERT_TRUE( 130 + i == Bytes( *result )[ i * 4 ] && 0 == Bytes( *result )[ i * 4 + 1 ] && 50 + i == Bytes( *result )[ i * 4 + 2 ] && 255 == Bytes( *result )[ i * 4 + 3 ] );

        // all four from sources: a single channel map is a source of channel 0
        IMAGE::TPixelMapPtr single;
        ASSERT_TRUE( IMAGE::CChannelOperations::ExtractChannel( *a, 2, single ) );
        IMAGE::TPixelMapPtr all[ 4 ] = { single, single, single, single };
        UInt32 zero[ 4 ] = { 0, 0, 0, 0 };
        ASSERT_TRUE( IMAGE::CChannelOperations::Pack( all, zero, result ) );
        ASSERT_TRUE( !result.IsNULL() && 92 == Bytes( *result )[ 8 ] && 92 == Bytes( *result )[ 11 ] );

        // sources that are not NULL decide the size and the data type
        IMAGE::TPixelMapPtr onlyAlpha[ 4 ] = { IMAGE::TPixelMapPtr(), IMAGE::TPixelMapPtr(), IMAGE::TPixelMapPtr(), single };
        ASSERT_TRUE( IMAGE::CChannelOperations::Pack( onlyAlpha, zero, result ) );
        ASSERT_TRUE( !result.IsNULL() && 0 == Bytes( *result )[ 0 ] && 90 == Bytes( *result )[ 3 ] );

        // a source of another data type is converted
        IMAGE::TPixelMapPtr floatSource( GUCEF_NEW IMAGE::CPixelMap( GUCEF_NULL, 2, 2, IMAGE::PSF_SINGLE_CHANNEL_GRAYSCALE, GUCEF::DATATYPE_LE_FLOAT32 ) );
        Float32* floatData = static_cast< Float32* >( floatSource->GetDataPtr() );
        floatData[ 0 ] = 1.0f; floatData[ 1 ] = 0.0f; floatData[ 2 ] = 1.0f; floatData[ 3 ] = 0.0f;
        IMAGE::TPixelMapPtr mixed[ 4 ] = { single, floatSource, IMAGE::TPixelMapPtr(), IMAGE::TPixelMapPtr() };
        ASSERT_TRUE( IMAGE::CChannelOperations::Pack( mixed, zero, result ) );
        ASSERT_TRUE( !result.IsNULL() && GUCEF::DATATYPE_UINT8 == result->GetPixelComponentDataType() && 90 == Bytes( *result )[ 0 ] && 255 == Bytes( *result )[ 1 ] && 0 == Bytes( *result )[ 5 ] );

        // the size of every source has to be the same
        IMAGE::TPixelMapPtr small( GUCEF_NEW IMAGE::CPixelMap( GUCEF_NULL, 1, 2, IMAGE::PSF_RGBA, GUCEF::DATATYPE_UINT8 ) );
        IMAGE::TPixelMapPtr mismatched[ 4 ] = { a, small, IMAGE::TPixelMapPtr(), IMAGE::TPixelMapPtr() };
        result = IMAGE::TPixelMapPtr();
        ASSERT_FALSE( IMAGE::CChannelOperations::Pack( mismatched, zero, result ) );
        ASSERT_TRUE( result.IsNULL() );

        // no source at all, a channel the source does not have and a palette source are refused
        IMAGE::TPixelMapPtr none[ 4 ];
        ASSERT_FALSE( IMAGE::CChannelOperations::Pack( none, zero, result ) );
        UInt32 outOfRange[ 4 ] = { 4, 0, 0, 0 };
        IMAGE::TPixelMapPtr one[ 4 ] = { a, IMAGE::TPixelMapPtr(), IMAGE::TPixelMapPtr(), IMAGE::TPixelMapPtr() };
        ASSERT_FALSE( IMAGE::CChannelOperations::Pack( one, outOfRange, result ) );
        IMAGE::TPixelMapPtr palettised( GUCEF_NEW IMAGE::CPixelMap( GUCEF_NULL, 2, 2, IMAGE::PSF_PALETTE_INDICES, GUCEF::DATATYPE_UINT8 ) );
        IMAGE::TPixelMapPtr withPalette[ 4 ] = { palettised, IMAGE::TPixelMapPtr(), IMAGE::TPixelMapPtr(), IMAGE::TPixelMapPtr() };
        ASSERT_FALSE( IMAGE::CChannelOperations::Pack( withPalette, zero, result ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "ChannelOperations: ConvertColourSpace converts the colour channels between sRGB and linear" )
    try
    {
        IMAGE::TPixelMapPtr source = MakeKnownMap(), result;
        ASSERT_TRUE( IMAGE::CChannelOperations::ConvertColourSpace( *source, IMAGE::COLOURSPACE_SRGB, IMAGE::COLOURSPACE_LINEAR, result ) );
        if ( !result.IsNULL() )
        {
            // 8 bit sRGB 128 is linear 0.2158 or 55; alpha is left alone
            UInt8 expected = ExpectedScaleBias( 0, 1.0f, (Float32) IMAGE::CPixelMapChannelAccess::SrgbToLinear( 50.0f / 255.0f ) );
            ASSERT_TRUE( Difference( expected, Bytes( *result )[ 1 ] ) <= 1 && Bytes( *result )[ 1 ] < 50 );
            ASSERT_TRUE( 130 == Bytes( *result )[ 3 ] && 133 == Bytes( *result )[ 15 ] );

            // and back again: within the rounding of 8 bit values
            IMAGE::TPixelMapPtr again;
            ASSERT_TRUE( IMAGE::CChannelOperations::ConvertColourSpace( *result, IMAGE::COLOURSPACE_LINEAR, IMAGE::COLOURSPACE_SRGB, again ) );
            if ( !again.IsNULL() )
            {
                // dark values lose most precision in 8 bit linear
                ASSERT_TRUE( Difference( Bytes( *again )[ 1 ], 50 ) <= 3 && Difference( Bytes( *again )[ 2 ], 90 ) <= 2 );
            }
        }
        ASSERT_TRUE( 50 == Bytes( *source )[ 1 ] );

        // the exact curve on a known value
        IMAGE::CPixelMap grey( GUCEF_NULL, 1, 1, IMAGE::PSF_RGB, GUCEF::DATATYPE_LE_FLOAT32 );
        Float32* data = static_cast< Float32* >( grey.GetDataPtr() );
        data[ 0 ] = 0.5f; data[ 1 ] = 0.0f; data[ 2 ] = 1.0f;
        ASSERT_TRUE( IMAGE::CChannelOperations::ConvertColourSpace( grey, IMAGE::COLOURSPACE_SRGB, IMAGE::COLOURSPACE_LINEAR ) );
        ASSERT_TRUE( fabs( data[ 0 ] - 0.21404114f ) < 0.00001f && 0.0f == data[ 1 ] && fabs( data[ 2 ] - 1.0f ) < 0.00001f );

        // no change when the spaces are the same, refused for maps without colour
        ASSERT_TRUE( IMAGE::CChannelOperations::ConvertColourSpace( grey, IMAGE::COLOURSPACE_LINEAR, IMAGE::COLOURSPACE_LINEAR ) && fabs( data[ 0 ] - 0.21404114f ) < 0.00001f );
        IMAGE::CPixelMap single( GUCEF_NULL, 1, 1, IMAGE::PSF_SINGLE_CHANNEL_GRAYSCALE, GUCEF::DATATYPE_UINT8 );
        ASSERT_FALSE( IMAGE::CChannelOperations::ConvertColourSpace( single, IMAGE::COLOURSPACE_SRGB, IMAGE::COLOURSPACE_LINEAR ) );
        IMAGE::CPixelMap hsv( GUCEF_NULL, 1, 1, IMAGE::PSF_HSV, GUCEF::DATATYPE_UINT8 );
        ASSERT_FALSE( IMAGE::CChannelOperations::ConvertColourSpace( hsv, IMAGE::COLOURSPACE_SRGB, IMAGE::COLOURSPACE_LINEAR ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "ChannelOperations: PremultiplyAlpha and UnpremultiplyAlpha scale the colour by alpha, in linear light for sRGB" )
    try
    {
        // linear: the stored values are multiplied, alpha stays
        UInt8 pixels[ 8 ] = { 200, 100, 50, 128,   90, 60, 30, 0 };
        IMAGE::TPixelMapPtr source( GUCEF_NEW IMAGE::CPixelMap( pixels, 2, 1, IMAGE::PSF_RGBA, GUCEF::DATATYPE_UINT8 ) ), result;
        ASSERT_TRUE( IMAGE::CChannelOperations::PremultiplyAlpha( *source, IMAGE::COLOURSPACE_LINEAR, result ) );
        ASSERT_TRUE( !result.IsNULL() );
        if ( !result.IsNULL() )
        {
            const UInt8* p = Bytes( *result );
            ASSERT_TRUE( Difference( p[ 0 ], 100 ) <= 1 && Difference( p[ 1 ], 50 ) <= 1 && Difference( p[ 2 ], 25 ) <= 1 && 128 == p[ 3 ] );
            ASSERT_TRUE( 0 == p[ 4 ] && 0 == p[ 5 ] && 0 == p[ 6 ] && 0 == p[ 7 ] );

            // and back, within the rounding of 8 bits; a texel with alpha 0 keeps colour 0
            IMAGE::TPixelMapPtr back;
            ASSERT_TRUE( IMAGE::CChannelOperations::UnpremultiplyAlpha( *result, IMAGE::COLOURSPACE_LINEAR, back ) );
            if ( !back.IsNULL() )
            {
                const UInt8* b = Bytes( *back );
                ASSERT_TRUE( Difference( b[ 0 ], 200 ) <= 2 && Difference( b[ 1 ], 100 ) <= 2 && Difference( b[ 2 ], 50 ) <= 2 && 128 == b[ 3 ] );
                ASSERT_TRUE( 0 == b[ 4 ] && 0 == b[ 7 ] );
            }
        }
        ASSERT_TRUE( 200 == Bytes( *source )[ 0 ] );

        // sRGB: full red at half alpha is 0.5 in linear light, which is about 188 in sRGB (not 128)
        UInt8 red[ 4 ] = { 255, 0, 0, 128 };
        IMAGE::CPixelMap srgb( red, 1, 1, IMAGE::PSF_RGBA, GUCEF::DATATYPE_UINT8 );
        ASSERT_TRUE( IMAGE::CChannelOperations::PremultiplyAlpha( srgb, IMAGE::COLOURSPACE_SRGB ) );
        ASSERT_TRUE( Difference( Bytes( srgb )[ 0 ], 188 ) <= 1 && 0 == Bytes( srgb )[ 1 ] && 128 == Bytes( srgb )[ 3 ] );
        ASSERT_TRUE( IMAGE::CChannelOperations::UnpremultiplyAlpha( srgb, IMAGE::COLOURSPACE_SRGB ) );
        ASSERT_TRUE( Difference( Bytes( srgb )[ 0 ], 255 ) <= 1 );

        // the alpha of a BGRA map is its fourth channel too, its colour the first three
        UInt8 bgra[ 4 ] = { 50, 100, 200, 128 };
        IMAGE::CPixelMap swapped( bgra, 1, 1, IMAGE::PSF_BGRA, GUCEF::DATATYPE_UINT8 );
        ASSERT_TRUE( IMAGE::CChannelOperations::PremultiplyAlpha( swapped, IMAGE::COLOURSPACE_LINEAR ) );
        ASSERT_TRUE( Difference( Bytes( swapped )[ 0 ], 25 ) <= 1 && Difference( Bytes( swapped )[ 2 ], 100 ) <= 1 && 128 == Bytes( swapped )[ 3 ] );

        // no alpha, or no colour: refused and unchanged
        UInt8 rgbPixels[ 3 ] = { 10, 20, 30 };
        IMAGE::CPixelMap rgb( rgbPixels, 1, 1, IMAGE::PSF_RGB, GUCEF::DATATYPE_UINT8 );
        ASSERT_FALSE( IMAGE::CChannelOperations::PremultiplyAlpha( rgb, IMAGE::COLOURSPACE_LINEAR ) );
        ASSERT_FALSE( IMAGE::CChannelOperations::UnpremultiplyAlpha( rgb, IMAGE::COLOURSPACE_LINEAR ) );
        ASSERT_TRUE( 10 == Bytes( rgb )[ 0 ] );
        IMAGE::CPixelMap grey( GUCEF_NULL, 1, 1, IMAGE::PSF_SINGLE_CHANNEL_GRAYSCALE, GUCEF::DATATYPE_UINT8 );
        ASSERT_FALSE( IMAGE::CChannelOperations::PremultiplyAlpha( grey, IMAGE::COLOURSPACE_LINEAR ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "ChannelOperations: Unusable pictures are refused and left alone" )
    try
    {
        IMAGE::CPixelMap palettised( GUCEF_NULL, 2, 2, IMAGE::PSF_PALETTE_INDICES, GUCEF::DATATYPE_UINT8 );
        IMAGE::TPixelMapPtr result;
        ASSERT_FALSE( IMAGE::CChannelOperations::ExtractChannel( palettised, 0, result ) );
        ASSERT_FALSE( IMAGE::CChannelOperations::Swizzle( palettised, "R", result ) );
        ASSERT_FALSE( IMAGE::CChannelOperations::Invert( palettised, 1 ) );
        ASSERT_FALSE( IMAGE::CChannelOperations::ScaleBias( palettised, 1, 1.0f, 0.0f ) );
        ASSERT_FALSE( IMAGE::CChannelOperations::ConvertColourSpace( palettised, IMAGE::COLOURSPACE_SRGB, IMAGE::COLOURSPACE_LINEAR ) );
        ASSERT_FALSE( IMAGE::CChannelOperations::PremultiplyAlpha( palettised, IMAGE::COLOURSPACE_LINEAR ) );
        ASSERT_TRUE( result.IsNULL() );

        IMAGE::CPixelMap bigEndian( GUCEF_NULL, 2, 2, IMAGE::PSF_RGB, GUCEF::DATATYPE_BE_UINT16 );
        ASSERT_FALSE( IMAGE::CChannelOperations::Invert( bigEndian, 1 ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
