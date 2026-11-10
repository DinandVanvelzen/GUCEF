/*
 *  gucefAUDIO_TestApp: GUCEF test application for the AUDIO module
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <math.h>
#include <string.h>

#ifndef GUCEFAUDIO_H
#include "gucefAUDIO.h"
#define GUCEFAUDIO_H
#endif /* GUCEFAUDIO_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestAudioConversion.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

static bool
Near( Float32 a, Float32 b, Float32 tolerance = 1e-5f )
{
    return fabsf( a - b ) <= tolerance;
}

/*-------------------------------------------------------------------------*/

static void
PerformSampleConversionTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "AudioConverter: 16 bit goes to float and back unchanged for every value" )
    try
    {
        GUCEF::vector< Int16 > all( 65536 ), back( 65536 );
        GUCEF::vector< Float32 > floats( 65536 );
        for ( UInt32 i=0; i<65536; ++i )
            all[ i ] = (Int16) ( (Int32) i - 32768 );
        AUDIO::CAudioConverter::ConvertToFloat32( &all[ 0 ], AUDIO::SAMPLETYPE_INT16, &floats[ 0 ], 65536 );
        AUDIO::CAudioConverter::ConvertFromFloat32( &floats[ 0 ], &back[ 0 ], AUDIO::SAMPLETYPE_INT16, 65536 );
        ASSERT_TRUE( 0 == memcmp( &all[ 0 ], &back[ 0 ], 65536 * 2 ) );
        ASSERT_TRUE( Near( -1.0f, floats[ 0 ], 0.0f ) );                     // -32768 is exactly -1.0
        ASSERT_TRUE( Near( 0.0f, floats[ 32768 ], 0.0f ) );
        ASSERT_TRUE( floats[ 65535 ] < 1.0f );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioConverter: 8 bit unsigned goes to float and back unchanged for every value" )
    try
    {
        UInt8 all[ 256 ], back[ 256 ];
        Float32 floats[ 256 ];
        for ( UInt32 i=0; i<256; ++i )
            all[ i ] = (UInt8) i;
        AUDIO::CAudioConverter::ConvertToFloat32( all, AUDIO::SAMPLETYPE_UINT8, floats, 256 );
        AUDIO::CAudioConverter::ConvertFromFloat32( floats, back, AUDIO::SAMPLETYPE_UINT8, 256 );
        ASSERT_TRUE( 0 == memcmp( all, back, 256 ) );
        ASSERT_TRUE( Near( 0.0f, floats[ 128 ], 0.0f ) );
        ASSERT_TRUE( Near( -1.0f, floats[ 0 ], 0.0f ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioConverter: 24 bit packed samples round trip, including the extremes and negative values" )
    try
    {
        const Int32 values[ 9 ] = { -8388608, -8388607, -1000000, -1, 0, 1, 12345, 8388606, 8388607 };
        UInt8 packed[ 27 ], back[ 27 ];
        Float32 floats[ 9 ];
        for ( UInt32 i=0; i<9; ++i )
        {
            packed[ i * 3 ] = (UInt8) ( values[ i ] & 0xFF );
            packed[ i * 3 + 1 ] = (UInt8) ( ( values[ i ] >> 8 ) & 0xFF );
            packed[ i * 3 + 2 ] = (UInt8) ( ( values[ i ] >> 16 ) & 0xFF );
        }
        AUDIO::CAudioConverter::ConvertToFloat32( packed, AUDIO::SAMPLETYPE_INT24, floats, 9 );
        ASSERT_TRUE( Near( -1.0f, floats[ 0 ], 0.0f ) );
        AUDIO::CAudioConverter::ConvertFromFloat32( floats, back, AUDIO::SAMPLETYPE_INT24, 9 );
        ASSERT_TRUE( 0 == memcmp( packed, back, 27 ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioConverter: 32 bit integers and 64 bit floats keep the precision of a 32 bit float" )
    try
    {
        const Int32 values[ 5 ] = { -2147483647 - 1, -1073741824, 0, 536870912, 2147483647 };
        Float32 floats[ 5 ];
        AUDIO::CAudioConverter::ConvertToFloat32( values, AUDIO::SAMPLETYPE_INT32, floats, 5 );
        ASSERT_TRUE( Near( -1.0f, floats[ 0 ], 0.0f ) );
        ASSERT_TRUE( Near( -0.5f, floats[ 1 ], 0.0f ) );
        ASSERT_TRUE( Near( 0.25f, floats[ 3 ], 0.0f ) );
        Int32 back[ 5 ];
        AUDIO::CAudioConverter::ConvertFromFloat32( floats, back, AUDIO::SAMPLETYPE_INT32, 5 );
        ASSERT_TRUE( back[ 0 ] == values[ 0 ] && back[ 1 ] == values[ 1 ] && back[ 2 ] == 0 && back[ 3 ] == values[ 3 ] );
        ASSERT_TRUE( back[ 4 ] == 2147483647 );                              // 1.0 is limited to the largest value

        const Float64 doubles[ 3 ] = { -0.5, 0.0, 0.25 };
        Float32 fromDoubles[ 3 ];
        AUDIO::CAudioConverter::ConvertToFloat32( doubles, AUDIO::SAMPLETYPE_FLOAT64, fromDoubles, 3 );
        ASSERT_TRUE( Near( -0.5f, fromDoubles[ 0 ], 0.0f ) && Near( 0.25f, fromDoubles[ 2 ], 0.0f ) );
        Float64 backDoubles[ 3 ];
        AUDIO::CAudioConverter::ConvertFromFloat32( fromDoubles, backDoubles, AUDIO::SAMPLETYPE_FLOAT64, 3 );
        ASSERT_TRUE( backDoubles[ 0 ] == -0.5 && backDoubles[ 2 ] == 0.25 );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioConverter: Loud floats are limited, not wrapped, and not a number is silence" )
    try
    {
        const Float32 loud[ 6 ] = { 2.0f, -2.0f, 1.0f, -1.0f, 0.5f, 0.0f };
        Int16 asInt16[ 6 ];
        AUDIO::CAudioConverter::ConvertFromFloat32( loud, asInt16, AUDIO::SAMPLETYPE_INT16, 6 );
        ASSERT_TRUE( 32767 == asInt16[ 0 ] && -32768 == asInt16[ 1 ] && 32767 == asInt16[ 2 ] && -32768 == asInt16[ 3 ] && 16384 == asInt16[ 4 ] && 0 == asInt16[ 5 ] );

        UInt8 asUInt8[ 3 ];
        const Float32 ends[ 3 ] = { 5.0f, -5.0f, 0.0f };
        AUDIO::CAudioConverter::ConvertFromFloat32( ends, asUInt8, AUDIO::SAMPLETYPE_UINT8, 3 );
        ASSERT_TRUE( 255 == asUInt8[ 0 ] && 0 == asUInt8[ 1 ] && 128 == asUInt8[ 2 ] );

        const Float32 nan = 0.0f / ( (Float32) asInt16[ 5 ] );             // 0 / 0 at run time
        Int16 silent = 123;
        AUDIO::CAudioConverter::ConvertFromFloat32( &nan, &silent, AUDIO::SAMPLETYPE_INT16, 1 );
        ASSERT_TRUE( 0 == silent );

        // floats pass through untouched
        Float32 passed;
        const Float32 big = 3.5f;
        AUDIO::CAudioConverter::ConvertFromFloat32( &big, &passed, AUDIO::SAMPLETYPE_FLOAT32, 1 );
        ASSERT_TRUE( 3.5f == passed );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioConverter: Direct conversion between any two types, in place when sizes allow" )
    try
    {
        const Int16 source[ 4 ] = { -32768, -16384, 0, 16384 };
        Float32 asFloat[ 4 ];
        AUDIO::CAudioConverter::ConvertSamples( source, AUDIO::SAMPLETYPE_INT16, asFloat, AUDIO::SAMPLETYPE_FLOAT32, 4 );
        ASSERT_TRUE( Near( -1.0f, asFloat[ 0 ], 0.0f ) && Near( 0.5f, asFloat[ 3 ], 0.0f ) );

        UInt8 asBytes[ 4 ];
        AUDIO::CAudioConverter::ConvertSamples( source, AUDIO::SAMPLETYPE_INT16, asBytes, AUDIO::SAMPLETYPE_UINT8, 4 );
        ASSERT_TRUE( 0 == asBytes[ 0 ] && 64 == asBytes[ 1 ] && 128 == asBytes[ 2 ] && 192 == asBytes[ 3 ] );

        Int32 inPlace[ 2 ] = { 0, 0 };
        Float32 asF[ 2 ] = { 0.5f, -0.5f };
        memcpy( inPlace, asF, 8 );
        AUDIO::CAudioConverter::ConvertSamples( inPlace, AUDIO::SAMPLETYPE_FLOAT32, inPlace, AUDIO::SAMPLETYPE_INT32, 2 );
        ASSERT_TRUE( 1073741824 == inPlace[ 0 ] && -1073741824 == inPlace[ 1 ] );

        // many samples cross the internal block size
        GUCEF::vector< Int16 > many( 10000 ), manyBack( 10000 );
        GUCEF::vector< UInt8 > manyBytes( 10000 );
        for ( UInt32 i=0; i<10000; ++i )
            many[ i ] = (Int16) ( ( i * 7 ) % 65536 - 32768 );
        AUDIO::CAudioConverter::ConvertSamples( &many[ 0 ], AUDIO::SAMPLETYPE_INT16, &manyBytes[ 0 ], AUDIO::SAMPLETYPE_UINT8, 10000 );
        ASSERT_TRUE( ( (Int32) manyBytes[ 9999 ] - 128 ) * 256 - many[ 9999 ] <= 128 && many[ 9999 ] - ( (Int32) manyBytes[ 9999 ] - 128 ) * 256 <= 128 );
        AUDIO::CAudioConverter::ConvertSamples( &many[ 0 ], AUDIO::SAMPLETYPE_INT16, &manyBack[ 0 ], AUDIO::SAMPLETYPE_INT16, 10000 );
        ASSERT_TRUE( 0 == memcmp( &many[ 0 ], &manyBack[ 0 ], 20000 ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static Float32
MatrixAt( const GUCEF::vector< Float32 >& matrix, UInt32 sourceChannels, UInt32 destination, UInt32 source )
{
    return matrix[ (size_t) destination * sourceChannels + source ];
}

static void
PerformChannelTests( void )
{GUCEF_TRACE;

    using AUDIO::CAudioFormat;
    using AUDIO::CAudioConverter;
    const AUDIO::TSampleType f32 = AUDIO::SAMPLETYPE_FLOAT32;

    GUCEF_TESTFW_TESTCASE( "AudioConverter: Same layout is an identity matrix" )
    try
    {
        GUCEF::vector< Float32 > matrix;
        ASSERT_TRUE( CAudioConverter::BuildChannelMatrix( CAudioFormat( 48000, 6, f32 ), CAudioFormat( 44100, 6, AUDIO::SAMPLETYPE_INT16 ), matrix ) );
        ASSERT_TRUE( 36 == matrix.size() );
        for ( UInt32 d=0; d<6; ++d )
            for ( UInt32 s=0; s<6; ++s )
                ASSERT_TRUE( MatrixAt( matrix, 6, d, s ) == ( d == s ? 1.0f : 0.0f ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioConverter: Mono goes to both fronts, stereo goes to mono as the average" )
    try
    {
        GUCEF::vector< Float32 > up, down;
        ASSERT_TRUE( CAudioConverter::BuildChannelMatrix( CAudioFormat( 44100, 1, f32 ), CAudioFormat( 44100, 2, f32 ), up ) );
        ASSERT_TRUE( 1.0f == up[ 0 ] && 1.0f == up[ 1 ] );
        ASSERT_TRUE( CAudioConverter::BuildChannelMatrix( CAudioFormat( 44100, 2, f32 ), CAudioFormat( 44100, 1, f32 ), down ) );
        ASSERT_TRUE( 0.5f == down[ 0 ] && 0.5f == down[ 1 ] );

        // mono into a layout that has a centre speaker goes to the centre
        GUCEF::vector< Float32 > toSurround;
        ASSERT_TRUE( CAudioConverter::BuildChannelMatrix( CAudioFormat( 44100, 1, f32 ), CAudioFormat( 44100, 6, f32 ), toSurround ) );
        ASSERT_TRUE( 1.0f == MatrixAt( toSurround, 1, 2, 0 ) );             // centre is the third channel of 5.1
        ASSERT_TRUE( 0.0f == MatrixAt( toSurround, 1, 0, 0 ) && 0.0f == MatrixAt( toSurround, 1, 1, 0 ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioConverter: Surround to stereo folds centre and back channels in without overloading" )
    try
    {
        GUCEF::vector< Float32 > matrix;
        ASSERT_TRUE( CAudioConverter::BuildChannelMatrix( CAudioFormat( 48000, 6, f32 ), CAudioFormat( 48000, 2, f32 ), matrix ) );
        // 5.1 order: FL FR FC LFE BL BR
        for ( UInt32 d=0; d<2; ++d )
        {
            Float32 sum = 0.0f;
            for ( UInt32 s=0; s<6; ++s )
                sum += MatrixAt( matrix, 6, d, s );
            ASSERT_TRUE( Near( 1.0f, sum, 1e-5f ) );                        // exactly full level with all channels at full level
            ASSERT_TRUE( 0.0f == MatrixAt( matrix, 6, d, 3 ) );              // the low frequency channel is left out
        }
        ASSERT_TRUE( MatrixAt( matrix, 6, 0, 0 ) > MatrixAt( matrix, 6, 0, 2 ) );   // front left matters more than the centre
        ASSERT_TRUE( 0.0f == MatrixAt( matrix, 6, 0, 1 ) && 0.0f == MatrixAt( matrix, 6, 1, 0 ) );
        ASSERT_TRUE( MatrixAt( matrix, 6, 0, 4 ) > 0.0f && 0.0f == MatrixAt( matrix, 6, 0, 5 ) );    // left back to left only
        ASSERT_TRUE( MatrixAt( matrix, 6, 1, 5 ) > 0.0f && 0.0f == MatrixAt( matrix, 6, 1, 4 ) );

        // quad has no centre: the back channels fold forward
        GUCEF::vector< Float32 > quad;
        ASSERT_TRUE( CAudioConverter::BuildChannelMatrix( CAudioFormat( 48000, 4, f32 ), CAudioFormat( 48000, 2, f32 ), quad ) );
        ASSERT_TRUE( Near( 1.0f, MatrixAt( quad, 4, 0, 0 ) + MatrixAt( quad, 4, 0, 2 ), 1e-5f ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioConverter: Stereo to surround keeps the fronts and leaves the rest silent, 7.1 folds to 5.1" )
    try
    {
        GUCEF::vector< Float32 > matrix;
        ASSERT_TRUE( CAudioConverter::BuildChannelMatrix( CAudioFormat( 48000, 2, f32 ), CAudioFormat( 48000, 6, f32 ), matrix ) );
        ASSERT_TRUE( 1.0f == MatrixAt( matrix, 2, 0, 0 ) && 1.0f == MatrixAt( matrix, 2, 1, 1 ) );
        for ( UInt32 d=2; d<6; ++d )
            ASSERT_TRUE( 0.0f == MatrixAt( matrix, 2, d, 0 ) && 0.0f == MatrixAt( matrix, 2, d, 1 ) );

        // 7.1 is FL FR FC LFE BL BR SL SR, 5.1 has BL BR where 7.1 has SL SR: the sides become the backs
        GUCEF::vector< Float32 > down;
        ASSERT_TRUE( CAudioConverter::BuildChannelMatrix( CAudioFormat( 48000, 8, f32 ), CAudioFormat( 48000, 6, f32 ), down ) );
        ASSERT_TRUE( 1.0f == MatrixAt( down, 8, 0, 0 ) && 1.0f == MatrixAt( down, 8, 2, 2 ) && 1.0f == MatrixAt( down, 8, 3, 3 ) );
        ASSERT_TRUE( MatrixAt( down, 8, 4, 4 ) > 0.0f && MatrixAt( down, 8, 4, 6 ) > 0.0f );       // back left gets back left and side left
        ASSERT_TRUE( MatrixAt( down, 8, 5, 5 ) > 0.0f && MatrixAt( down, 8, 5, 7 ) > 0.0f );
        ASSERT_TRUE( 0.0f == MatrixAt( down, 8, 4, 7 ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioConverter: Unrelated layouts fall back to channel order and invalid formats are refused" )
    try
    {
        GUCEF::vector< Float32 > matrix;
        const CAudioFormat topA( 48000, 0, f32, AUDIO::AUDIOSPEAKER_TOP_CENTER | AUDIO::AUDIOSPEAKER_TOP_BACK_CENTER );
        const CAudioFormat topB( 48000, 0, f32, AUDIO::AUDIOSPEAKER_TOP_FRONT_LEFT | AUDIO::AUDIOSPEAKER_TOP_FRONT_RIGHT | AUDIO::AUDIOSPEAKER_TOP_BACK_LEFT );
        ASSERT_TRUE( CAudioConverter::BuildChannelMatrix( topA, topB, matrix ) );
        ASSERT_TRUE( 6 == matrix.size() );
        ASSERT_FALSE( CAudioConverter::BuildChannelMatrix( CAudioFormat(), CAudioFormat( 48000, 2, f32 ), matrix ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioConverter: Remixing frames applies the matrix" )
    try
    {
        // stereo to mono
        const Float32 stereo[ 6 ] = { 1.0f, 0.0f, 0.5f, 0.5f, -1.0f, 1.0f };
        GUCEF::vector< Float32 > matrix;
        ASSERT_TRUE( CAudioConverter::BuildChannelMatrix( CAudioFormat( 48000, 2, f32 ), CAudioFormat( 48000, 1, f32 ), matrix ) );
        Float32 mono[ 3 ];
        CAudioConverter::RemixChannels( stereo, 2, mono, 1, 3, matrix );
        ASSERT_TRUE( Near( 0.5f, mono[ 0 ] ) && Near( 0.5f, mono[ 1 ] ) && Near( 0.0f, mono[ 2 ] ) );

        // and back to stereo
        GUCEF::vector< Float32 > up;
        ASSERT_TRUE( CAudioConverter::BuildChannelMatrix( CAudioFormat( 48000, 1, f32 ), CAudioFormat( 48000, 2, f32 ), up ) );
        Float32 stereoAgain[ 6 ];
        CAudioConverter::RemixChannels( mono, 1, stereoAgain, 2, 3, up );
        ASSERT_TRUE( Near( 0.5f, stereoAgain[ 0 ] ) && Near( 0.5f, stereoAgain[ 1 ] ) && Near( 0.0f, stereoAgain[ 4 ] ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
PerformResamplerTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "LinearResampler: Ratio 1 hands the audio through unchanged" )
    try
    {
        AUDIO::CLinearResampler resampler;
        ASSERT_FALSE( resampler.Configure( 0, 44100.0, 44100.0 ) );
        ASSERT_FALSE( resampler.Configure( 1, 0.0, 44100.0 ) );
        ASSERT_TRUE( resampler.Configure( 2, 44100.0, 44100.0 ) );

        Float32 input[ 20 ], output[ 20 ];
        for ( UInt32 i=0; i<20; ++i )
            input[ i ] = (Float32) i * 0.01f;
        UInt32 consumed = 0, produced = 0;
        resampler.Process( input, 10, consumed, output, 10, produced );
        ASSERT_TRUE( 10 == consumed );
        ASSERT_TRUE( 9 == produced );                                  // the last frame waits for its neighbour
        for ( UInt32 i=0; i<9 * 2; ++i )
            ASSERT_TRUE( Near( input[ i ], output[ i ], 1e-6f ) );
        ASSERT_TRUE( 1 == resampler.GetLatencyInFrames() );
        ASSERT_TRUE( 0 == strcmp( "linear", resampler.GetName() ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "LinearResampler: Doubling the rate interpolates, halving it skips" )
    try
    {
        AUDIO::CLinearResampler up;
        ASSERT_TRUE( up.Configure( 1, 100.0, 200.0 ) );
        const Float32 ramp[ 5 ] = { 0.0f, 1.0f, 2.0f, 3.0f, 4.0f };
        Float32 out[ 16 ];
        UInt32 consumed = 0, produced = 0;
        up.Process( ramp, 5, consumed, out, 16, produced );
        ASSERT_TRUE( 8 == produced );
        for ( UInt32 i=0; i<produced; ++i )
            ASSERT_TRUE( Near( (Float32) i * 0.5f, out[ i ] ) );       // a ramp stays a ramp

        AUDIO::CLinearResampler down;
        ASSERT_TRUE( down.Configure( 1, 200.0, 100.0 ) );
        const Float32 many[ 9 ] = { 0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f };
        down.Process( many, 9, consumed, out, 16, produced );
        ASSERT_TRUE( 4 == produced );
        for ( UInt32 i=0; i<produced; ++i )
            ASSERT_TRUE( Near( (Float32) i * 2.0f, out[ i ] ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "LinearResampler: The size of the pieces the audio comes in does not change the result" )
    try
    {
        const UInt32 total = 1000;
        GUCEF::vector< Float32 > signal( total * 2 );
        for ( UInt32 i=0; i<total; ++i )
        {
            signal[ i * 2 ] = (Float32) sin( i * 0.07 );
            signal[ i * 2 + 1 ] = (Float32) cos( i * 0.031 );
        }

        // 22050 to 32000 does not divide evenly, the position inside a frame has to carry over between pieces
        GUCEF::vector< Float32 > reference( 4000 * 2 );
        AUDIO::CLinearResampler whole;
        ASSERT_TRUE( whole.Configure( 2, 22050.0, 32000.0 ) );
        UInt32 consumed = 0, produced = 0;
        whole.Process( &signal[ 0 ], total, consumed, &reference[ 0 ], 4000, produced );
        const UInt32 referenceFrames = produced;
        ASSERT_TRUE( referenceFrames > 1000 );

        const UInt32 pieces[ 6 ] = { 1, 2, 3, 7, 64, 333 };
        for ( UInt32 p=0; p<6; ++p )
        {
            AUDIO::CLinearResampler chunked;
            ASSERT_TRUE( chunked.Configure( 2, 22050.0, 32000.0 ) );
            GUCEF::vector< Float32 > result;
            UInt32 position = 0;
            while ( position < total )
            {
                const UInt32 n = ( total - position < pieces[ p ] ) ? ( total - position ) : pieces[ p ];
                Float32 piece[ 4000 * 2 ];
                chunked.Process( &signal[ (size_t) position * 2 ], n, consumed, piece, 4000, produced );
                ASSERT_TRUE( consumed == n );
                result.insert( result.end(), piece, piece + (size_t) produced * 2 );
                position += n;
            }
            ASSERT_TRUE( result.size() == (size_t) referenceFrames * 2 );
            bool same = result.size() == (size_t) referenceFrames * 2;
            for ( size_t i=0; same && i<result.size(); ++i )
                same = Near( result[ i ], reference[ i ], 1e-5f );
            ASSERT_TRUE( same );
        }
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "LinearResampler: Rates can change while the stream plays, reset and clone work" )
    try
    {
        AUDIO::CLinearResampler resampler;
        ASSERT_TRUE( resampler.Configure( 1, 100.0, 100.0 ) );
        Float32 ramp[ 100 ], out[ 400 ];
        for ( UInt32 i=0; i<100; ++i )
            ramp[ i ] = (Float32) i;
        UInt32 consumed = 0, produced = 0;
        resampler.Process( ramp, 50, consumed, out, 400, produced );
        ASSERT_TRUE( 49 == produced );
        ASSERT_TRUE( resampler.SetRates( 100.0, 200.0 ) );                 // from now on twice as many output frames per input frame
        resampler.Process( ramp + 50, 50, consumed, out, 400, produced );
        ASSERT_TRUE( produced >= 98 );
        ASSERT_TRUE( Near( 49.0f, out[ 0 ] ) );                            // continues right where the first piece stopped
        ASSERT_FALSE( resampler.SetRates( 0.0, 50.0 ) );

        AUDIO::IAudioResampler* clone = resampler.Clone();
        ASSERT_TRUE( GUCEF_NULL != clone );
        clone->Process( ramp, 10, consumed, out, 400, produced );
        ASSERT_TRUE( produced > 0 && Near( 0.0f, out[ 0 ] ) );             // a clone starts a new stream
        GUCEF_DELETE clone;

        resampler.Reset();
        resampler.Process( ramp, 10, consumed, out, 400, produced );
        ASSERT_TRUE( Near( 0.0f, out[ 0 ] ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "LinearResampler: A small output buffer does not lose or repeat audio" )
    try
    {
        AUDIO::CLinearResampler resampler;
        ASSERT_TRUE( resampler.Configure( 1, 100.0, 300.0 ) );
        const Float32 ramp[ 4 ] = { 0.0f, 3.0f, 6.0f, 9.0f };
        Float32 out[ 2 ];
        UInt32 consumed = 0, produced = 0;
        GUCEF::vector< Float32 > all;
        UInt32 rounds = 0;
        do
        {
            resampler.Process( ramp, 4, consumed, out, 2, produced );
            all.insert( all.end(), out, out + produced );
            ++rounds;
        }
        while ( 0 == consumed && rounds < 50 );
        ASSERT_TRUE( 4 == consumed );
        ASSERT_TRUE( 9 == all.size() );
        for ( UInt32 i=0; i<all.size(); ++i )
            ASSERT_TRUE( Near( (Float32) i, all[ i ] ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
PerformBufferConversionTests( void )
{GUCEF_TRACE;

    using AUDIO::CAudioFormat;
    using AUDIO::CAudioBuffer;

    GUCEF_TESTFW_TESTCASE( "AudioBuffer: Converting to the same format changes nothing" )
    try
    {
        CAudioBuffer buffer( CAudioFormat( 44100, 2, AUDIO::SAMPLETYPE_INT16 ), 10 );
        buffer.SetSampleFromFloat( 3, 1, 0.5f );
        ASSERT_TRUE( buffer.ConvertFormatTo( CAudioFormat( 44100, 2, AUDIO::SAMPLETYPE_INT16 ) ) );
        ASSERT_TRUE( 10 == buffer.GetFrameCount() );
        ASSERT_TRUE( Near( 0.5f, buffer.GetSampleAsFloat( 3, 1 ), 1e-4f ) );

        AUDIO::TAudioBufferPtr copy;
        ASSERT_TRUE( buffer.ConvertFormatTo( buffer.GetFormat(), copy ) );
        ASSERT_FALSE( copy.IsNULL() );
        ASSERT_TRUE( 10 == copy->GetFrameCount() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioBuffer: Sample type conversion keeps every sample" )
    try
    {
        CAudioBuffer buffer( CAudioFormat( 22050, 2, AUDIO::SAMPLETYPE_INT16 ), 1000 );
        for ( UInt32 f=0; f<1000; ++f )
        {
            buffer.SetSampleFromFloat( f, 0, (Float32) sin( f * 0.1 ) * 0.9f );
            buffer.SetSampleFromFloat( f, 1, (Float32) cos( f * 0.05 ) * 0.4f );
        }
        CAudioBuffer original( buffer );
        ASSERT_TRUE( buffer.ConvertFormatTo( AUDIO::SAMPLETYPE_FLOAT32 ) );
        ASSERT_TRUE( AUDIO::SAMPLETYPE_FLOAT32 == buffer.GetFormat().GetSampleType() );
        ASSERT_TRUE( 1000 == buffer.GetFrameCount() );
        for ( UInt32 f=0; f<1000; f+=13 )
            ASSERT_TRUE( Near( original.GetSampleAsFloat( f, 0 ), buffer.GetSampleAsFloat( f, 0 ), 1e-6f ) );
        ASSERT_TRUE( buffer.ConvertFormatTo( AUDIO::SAMPLETYPE_INT16 ) );
        ASSERT_TRUE( 0 == memcmp( original.GetDataPtr(), buffer.GetDataPtr(), original.GetSizeInBytes() ) );    // and back, bit for bit
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioBuffer: Converting channels and sample rate together" )
    try
    {
        // one second of constant stereo at 44100 Hz to mono at 22050 Hz
        CAudioBuffer stereo( CAudioFormat( 44100, 2, AUDIO::SAMPLETYPE_FLOAT32 ), 44100 );
        for ( UInt32 f=0; f<44100; ++f )
        {
            stereo.SetSampleFromFloat( f, 0, 0.8f );
            stereo.SetSampleFromFloat( f, 1, 0.4f );
        }
        AUDIO::TAudioBufferPtr mono;
        ASSERT_TRUE( stereo.ConvertFormatTo( CAudioFormat( 22050, 1, AUDIO::SAMPLETYPE_FLOAT32 ), mono ) );
        ASSERT_TRUE( 22050 == mono->GetFrameCount() );
        ASSERT_TRUE( 1 == mono->GetFormat().GetChannelCount() );
        for ( UInt32 f=0; f<22050; f+=1001 )
            ASSERT_TRUE( Near( 0.6f, mono->GetSampleAsFloat( f, 0 ), 1e-5f ) );
        ASSERT_TRUE( Near( 0.6f, mono->GetSampleAsFloat( 22049, 0 ), 1e-5f ) );        // the tail is there too
        ASSERT_TRUE( Near( 44100.0f / 44100.0f, (Float32) stereo.GetDurationInSeconds() ) && Near( 1.0f, (Float32) mono->GetDurationInSeconds() ) );

        // the source is unchanged
        ASSERT_TRUE( 44100 == stereo.GetFrameCount() && 2 == stereo.GetFormat().GetChannelCount() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioBuffer: Upsampling doubles the length and keeps a ramp a ramp, sample types change on the way" )
    try
    {
        CAudioBuffer slow( CAudioFormat( 8000, 1, AUDIO::SAMPLETYPE_FLOAT32 ), 800 );
        for ( UInt32 f=0; f<800; ++f )
            slow.SetSampleFromFloat( f, 0, (Float32) f / 1000.0f );
        AUDIO::TAudioBufferPtr fast;
        ASSERT_TRUE( slow.ConvertFormatTo( CAudioFormat( 16000, 1, AUDIO::SAMPLETYPE_INT16 ), fast ) );
        ASSERT_TRUE( 1600 == fast->GetFrameCount() );
        for ( UInt32 f=0; f<1599; f+=97 )
            ASSERT_TRUE( Near( (Float32) f / 2000.0f, fast->GetSampleAsFloat( f, 0 ), 1e-3f ) );

        // empty buffers and invalid formats
        CAudioBuffer empty( CAudioFormat( 8000, 1, AUDIO::SAMPLETYPE_INT16 ), 0 );
        ASSERT_TRUE( empty.ConvertFormatTo( CAudioFormat( 44100, 2, AUDIO::SAMPLETYPE_FLOAT32 ) ) );
        ASSERT_TRUE( empty.IsEmpty() && 44100 == empty.GetFormat().GetSampleRate() );
        ASSERT_FALSE( slow.ConvertFormatTo( CAudioFormat() ) );
        ASSERT_TRUE( 800 == slow.GetFrameCount() );                         // refused conversion leaves the buffer as it was
        CAudioBuffer none;
        AUDIO::TAudioBufferPtr nothing;
        ASSERT_FALSE( none.ConvertFormatTo( CAudioFormat( 44100, 2, AUDIO::SAMPLETYPE_INT16 ), nothing ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioBuffer: Conversion with a resampler of your own" )
    try
    {
        // A resampler that is not the default is used when given
        AUDIO::CLinearResampler custom;
        CAudioBuffer source( CAudioFormat( 100, 1, AUDIO::SAMPLETYPE_FLOAT32 ), 100 );
        for ( UInt32 f=0; f<100; ++f )
            source.SetSampleFromFloat( f, 0, (Float32) f / 100.0f );
        CAudioBuffer result;
        ASSERT_TRUE( AUDIO::CAudioConverter::Convert( source, CAudioFormat( 300, 1, AUDIO::SAMPLETYPE_FLOAT32 ), result, &custom ) );
        ASSERT_TRUE( 300 == result.GetFrameCount() );
        ASSERT_TRUE( Near( 0.5f, result.GetSampleAsFloat( 150, 0 ), 1e-4f ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
PerformAudioConversionTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING AudioConversion TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "gucefAUDIO.Conversion" );

    PerformSampleConversionTests();
    PerformChannelTests();
    PerformResamplerTests();
    PerformBufferConversionTests();
}

/*-------------------------------------------------------------------------*/
