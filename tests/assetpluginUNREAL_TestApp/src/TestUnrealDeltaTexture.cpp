/*
 *  assetpluginUNREAL_TestApp: Tests of the decoder of TSCF_UEDELTA texture sources
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

#include "gucef_test_framework.h"

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALDELTATEXTUREDECODER_H
#include "assetpluginUNREAL_CUnrealDeltaTextureDecoder.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALDELTATEXTUREDECODER_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALDELTATEXTUREDECODER_H ? */

#include "TestUnrealDeltaTexture.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;
using namespace GUCEF::ASSETPLUGIN::UNREAL;

#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ERRORHERE       GUCEF_TESTFW_ERRORHERE

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The rows of a strip for the format and sizes, 0 when refused */
static UInt32
Rows( const char* format, UInt32 width, UInt32 height )
{
    UInt32 rows = 0;
    CORE::CString error;
    if ( !CUnrealDeltaTextureDecoder::StripRows( format, width, height, rows, error ) )
        return 0;
    return rows;
}

/*-------------------------------------------------------------------------*/

/**
 *  The stored form of an image, written out rule by rule: the first row of each strip of the given height as it is, every other
 *  row as the difference to the row above (8 bit samples: the byte difference; 16 bit samples: the little-endian difference plus 0x8080)
 */
static void
Predict( const GUCEF::vector< UInt8 >& pixels, UInt32 width, UInt32 height, UInt32 bytesPerPixel, UInt32 rows, GUCEF::vector< UInt8 >& stored )
{
    const size_t pitch = (size_t) width * bytesPerPixel;
    stored = pixels;
    for ( UInt32 y=1; y<height; ++y )
    {
        if ( 0 == y % rows )
            continue;
        for ( size_t i=0; i<pitch; )
        {
            const size_t at = y * pitch + i;
            if ( 8 == bytesPerPixel )
            {
                const UInt32 value = (UInt32) pixels[ at ] | ( (UInt32) pixels[ at + 1 ] << 8 );
                const UInt32 above = (UInt32) pixels[ at - pitch ] | ( (UInt32) pixels[ at - pitch + 1 ] << 8 );
                const UInt32 code = ( value + 0x10000 - above + 0x8080 ) & 0xFFFF;
                stored[ at ] = (UInt8) ( code & 0xFF );
                stored[ at + 1 ] = (UInt8) ( code >> 8 );
                i += 2;
            }
            else
            {
                stored[ at ] = (UInt8) ( pixels[ at ] - pixels[ at - pitch ] );
                ++i;
            }
        }
    }
}

/*-------------------------------------------------------------------------*/

/** A smooth pattern with steps and wrap-arounds, so that differences of both signs and of every size occur */
static void
Pattern( UInt32 width, UInt32 height, UInt32 bytesPerPixel, UInt32 seed, GUCEF::vector< UInt8 >& pixels )
{
    pixels.resize( (size_t) width * height * bytesPerPixel );
    UInt32 state = seed;
    for ( UInt32 y=0; y<height; ++y )
        for ( UInt32 x=0; x<width * bytesPerPixel; ++x )
        {
            state = state * 1103515245 + 12345;
            const UInt32 noise = ( state >> 16 ) & 0x0F;
            pixels[ (size_t) y * width * bytesPerPixel + x ] = (UInt8) ( x * 3 + y * 5 + ( ( x / 37 + y / 11 ) & 1 ) * 120 + noise );
        }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformUnrealDeltaTextureTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "assetpluginUNREAL.DeltaTexture" );

    GUCEF_TESTFW_TESTCASE( "Delta texture sources: the strip height follows the pixel count and the minimum rows of the format; sizes with two possible answers are refused" )
    try
    {
        // TSF_BGRA8: 32768 pixels per strip, at least 32 rows
        ASSERT_TRUE( 128 == Rows( "TSF_BGRA8", 256, 256 ) && 64 == Rows( "TSF_BGRA8", 512, 512 ) && 32 == Rows( "TSF_BGRA8", 1024, 1024 ) );
        ASSERT_TRUE( 32 == Rows( "TSF_BGRA8", 2048, 2048 ) && 32 == Rows( "TSF_BGRA8", 4096, 4096 ) && 32 == Rows( "TSF_BGRA8", 8192, 8192 ) );
        ASSERT_TRUE( 32 == Rows( "TSF_BGRA8", 4096, 16 ) && 1024 == Rows( "TSF_BGRA8", 32, 32 ) );
        // TSF_RGBA16: at least 64 rows; TSF_G8: 16 rows at width 2048
        ASSERT_TRUE( 64 == Rows( "TSF_RGBA16", 2048, 2048 ) && 64 == Rows( "TSF_RGBA16", 4096, 4096 ) && 128 == Rows( "TSF_RGBA16", 256, 256 ) );
        ASSERT_TRUE( 16 == Rows( "TSF_G8", 2048, 2048 ) && 32 == Rows( "TSF_G8", 1024, 1024 ) );
        // a TSF_G8 source wider than 2048 pixels depends on the minimum of the format, which is not established, unless it is one strip
        ASSERT_TRUE( 0 == Rows( "TSF_G8", 4096, 4096 ) && 8 == Rows( "TSF_G8", 4096, 8 ) );
        // a width that does not divide 32768: refused where the rounding decides, accepted where the minimum or a single strip does
        ASSERT_TRUE( 0 == Rows( "TSF_BGRA8", 300, 200 ) && 109 == Rows( "TSF_BGRA8", 300, 109 ) && 32 == Rows( "TSF_BGRA8", 3000, 3000 ) );
        ASSERT_TRUE( 0 != Rows( "TSF_BGRA8", 134, 131 ) );
        // formats whose layout is not established, sizes that are not valid
        CORE::CString error;
        UInt32 rows = 7;
        ASSERT_TRUE( !CUnrealDeltaTextureDecoder::StripRows( "TSF_RGBA8", 64, 64, rows, error ) && 0 == rows && error.HasSubstr( "TSF_RGBA8" ) >= 0 && error.HasSubstr( "not established" ) >= 0 );
        ASSERT_TRUE( 0 == Rows( "TSF_G16", 64, 64 ) && 0 == Rows( "TSF_RGBA16F", 64, 64 ) && 0 == Rows( "TSF_RGBA32F", 64, 64 ) && 0 == Rows( "", 64, 64 ) );
        ASSERT_TRUE( !CUnrealDeltaTextureDecoder::StripRows( "TSF_BGRA8", 0, 64, rows, error ) && error.HasSubstr( "not valid" ) >= 0 );
        ASSERT_TRUE( 0 == Rows( "TSF_BGRA8", 64, 0 ) && 0 == Rows( "TSF_BGRA8", 70000, 1 ) && 0 == Rows( "TSF_BGRA8", 1, 0xFFFFFFFF ) );
        ASSERT_TRUE( 4 == CUnrealDeltaTextureDecoder::BytesPerPixel( "TSF_BGRA8" ) && 1 == CUnrealDeltaTextureDecoder::BytesPerPixel( "TSF_G8" ) && 8 == CUnrealDeltaTextureDecoder::BytesPerPixel( "TSF_RGBA16" ) && 0 == CUnrealDeltaTextureDecoder::BytesPerPixel( "TSF_RGBA8" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Delta texture sources: 8 bit samples, hand-made rows: the first row of a strip is kept, the others add the row above modulo 256" )
    try
    {
        // TSF_G8, 2048 wide: strips of 16 rows; rows 0 and 16 are kept, the others are differences
        const UInt32 width = 2048, height = 20;
        GUCEF::vector< UInt8 > data( (size_t) width * height, 0 );
        for ( UInt32 x=0; x<width; ++x )
        {
            data[ x ] = (UInt8) ( 250 + x );                 // row 0 as it is
            data[ width + x ] = 10;                          // row 1: +10, wraps above 255
            data[ 2 * width + x ] = 0xFF;                    // row 2: -1
            data[ 16 * width + x ] = (UInt8) ( 7 + x );      // row 16: a new strip, as it is
            data[ 17 * width + x ] = 0x80;                   // row 17: -128
        }
        CORE::CString error;
        ASSERT_TRUE( CUnrealDeltaTextureDecoder::Decode( "TSF_G8", width, height, &data[ 0 ], data.size(), error ) );
        for ( UInt32 x=0; x<width; ++x )
        {
            const UInt8 first = (UInt8) ( 250 + x );
            ASSERT_TRUE( data[ x ] == first );
            ASSERT_TRUE( data[ width + x ] == (UInt8) ( first + 10 ) && data[ 2 * width + x ] == (UInt8) ( first + 9 ) );
            ASSERT_TRUE( data[ 15 * width + x ] == (UInt8) ( first + 9 ) );   // rows of zero differences repeat the row above
            ASSERT_TRUE( data[ 16 * width + x ] == (UInt8) ( 7 + x ) && data[ 17 * width + x ] == (UInt8) ( 7 + x - 128 ) && data[ 19 * width + x ] == (UInt8) ( 7 + x - 128 ) );
        }
        ASSERT_TRUE( 250 == data[ 0 ] && 4 == data[ width + 0 ] && 3 == data[ 2 * width + 0 ] );

        // TSF_BGRA8: every byte is its own sample; a pattern through the rule-by-rule form comes back exactly, strips of 64 rows at width 512
        GUCEF::vector< UInt8 > pixels, stored;
        Pattern( 512, 150, 4, 1, pixels );
        Predict( pixels, 512, 150, 4, 64, stored );
        ASSERT_TRUE( stored != pixels && 0 == memcmp( &stored[ 64 * 512 * 4 ], &pixels[ 64 * 512 * 4 ], 512 * 4 ) );
        ASSERT_TRUE( CUnrealDeltaTextureDecoder::Decode( "TSF_BGRA8", 512, 150, &stored[ 0 ], stored.size(), error ) && stored == pixels );
        // the same with the wrong strip height gives other pixels: the strips matter
        Predict( pixels, 512, 150, 4, 32, stored );
        ASSERT_TRUE( CUnrealDeltaTextureDecoder::Decode( "TSF_BGRA8", 512, 150, &stored[ 0 ], stored.size(), error ) && stored != pixels );
        // the minimum of 32 rows at width 4096, and a last strip that is shorter
        Pattern( 4096, 70, 4, 2, pixels );
        Predict( pixels, 4096, 70, 4, 32, stored );
        ASSERT_TRUE( CUnrealDeltaTextureDecoder::Decode( "TSF_BGRA8", 4096, 70, &stored[ 0 ], stored.size(), error ) && stored == pixels );
        // bytes after the first image (further mips) are left as they are
        Pattern( 256, 300, 4, 3, pixels );
        Predict( pixels, 256, 300, 4, 128, stored );
        stored.push_back( 0xAB );
        ASSERT_TRUE( CUnrealDeltaTextureDecoder::Decode( "TSF_BGRA8", 256, 300, &stored[ 0 ], stored.size(), error ) && 0xAB == stored.back() && 0 == memcmp( &stored[ 0 ], &pixels[ 0 ], pixels.size() ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Delta texture sources: 16 bit samples are little-endian words, the difference is stored plus 0x8080 and carries into the high byte" )
    try
    {
        // TSF_RGBA16, 512 wide: strips of 64 rows; four samples per pixel
        const UInt32 width = 512, height = 66;
        const size_t pitch = (size_t) width * 8;
        GUCEF::vector< UInt8 > data( pitch * height, 0 );
        struct SCase { UInt16 above; UInt16 stored; UInt16 expected; };
        const SCase cases[] = {
            { 0x1234, 0x8080, 0x1234 },     // no difference
            { 0x00FF, 0x8081, 0x0100 },     // +1 carries into the high byte
            { 0xFFFF, 0x8081, 0x0000 },     // +1 wraps
            { 0x0000, 0x807F, 0xFFFF },     // -1 wraps
            { 0x0000, 0x0080, 0x8000 },     // +0x8000
            { 0x7460, 0x80A0, 0x7480 },     // +0x20
            { 0x1000, 0x7F80, 0x0F00 },     // -0x100
            { 0xABCD, 0x8080 + 0x0101, 0xACCE }
        };
        const size_t count = sizeof( cases ) / sizeof( cases[ 0 ] );
        for ( size_t k=0; k<count; ++k )
        {
            data[ k * 2 ] = (UInt8) ( cases[ k ].above & 0xFF );
            data[ k * 2 + 1 ] = (UInt8) ( cases[ k ].above >> 8 );
            data[ pitch + k * 2 ] = (UInt8) ( cases[ k ].stored & 0xFF );
            data[ pitch + k * 2 + 1 ] = (UInt8) ( cases[ k ].stored >> 8 );
        }
        // the rest of row 1 holds zero differences (80 80); row 64 starts a new strip and is kept as it is
        for ( size_t i=count * 2; i<pitch; ++i )
            data[ pitch + i ] = 0x80;
        for ( UInt32 y=2; y<height; ++y )
            for ( size_t i=0; i<pitch; ++i )
                data[ y * pitch + i ] = 64 == y ? (UInt8) i : 0x80;
        CORE::CString error;
        ASSERT_TRUE( CUnrealDeltaTextureDecoder::Decode( "TSF_RGBA16", width, height, &data[ 0 ], data.size(), error ) );
        for ( size_t k=0; k<count; ++k )
        {
            const UInt32 value = (UInt32) data[ pitch + k * 2 ] | ( (UInt32) data[ pitch + k * 2 + 1 ] << 8 );
            const UInt32 last = (UInt32) data[ 63 * pitch + k * 2 ] | ( (UInt32) data[ 63 * pitch + k * 2 + 1 ] << 8 );
            ASSERT_TRUE( cases[ k ].expected == value && cases[ k ].expected == last );
        }
        ASSERT_TRUE( 0 == data[ 64 * pitch ] && 5 == data[ 64 * pitch + 5 ] && 5 == data[ 65 * pitch + 5 ] );

        // a pattern through the rule-by-rule form comes back exactly
        GUCEF::vector< UInt8 > pixels, stored;
        Pattern( 256, 300, 8, 4, pixels );
        Predict( pixels, 256, 300, 8, 128, stored );
        ASSERT_TRUE( CUnrealDeltaTextureDecoder::Decode( "TSF_RGBA16", 256, 300, &stored[ 0 ], stored.size(), error ) && stored == pixels );
        // per byte, without the carry, the result differs: the words are what counts
        Predict( pixels, 256, 300, 8, 128, stored );
        GUCEF::vector< UInt8 > byBytes = stored;
        for ( size_t i=256 * 8; i<byBytes.size(); ++i )
            if ( 0 != ( i / ( 256 * 8 ) ) % 128 )
                byBytes[ i ] = (UInt8) ( byBytes[ i ] - 0x80 + byBytes[ i - 256 * 8 ] );
        ASSERT_TRUE( byBytes != pixels );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Delta texture sources: truncated data, wrong sizes, absurd dimensions and unknown formats are refused with a message and leave the data as it is" )
    try
    {
        GUCEF::vector< UInt8 > data( 64 * 4 * 3, 1 );
        const GUCEF::vector< UInt8 > original = data;
        CORE::CString error;
        // one byte short of the first image
        ASSERT_TRUE( !CUnrealDeltaTextureDecoder::Decode( "TSF_BGRA8", 64, 3, &data[ 0 ], data.size() - 1, error ) && error.HasSubstr( "needed for the first image" ) >= 0 && data == original );
        // sizes larger than the data, also where the product overflows 32 bits
        ASSERT_TRUE( !CUnrealDeltaTextureDecoder::Decode( "TSF_BGRA8", 64, 4, &data[ 0 ], data.size(), error ) && data == original );
        ASSERT_TRUE( !CUnrealDeltaTextureDecoder::Decode( "TSF_RGBA16", 65536, 65536, &data[ 0 ], data.size(), error ) && !error.IsNULLOrEmpty() && data == original );
        // absurd and empty dimensions
        ASSERT_TRUE( !CUnrealDeltaTextureDecoder::Decode( "TSF_BGRA8", 0x7FFFFFFF, 3, &data[ 0 ], data.size(), error ) && error.HasSubstr( "not valid" ) >= 0 && data == original );
        ASSERT_TRUE( !CUnrealDeltaTextureDecoder::Decode( "TSF_BGRA8", 0, 0, &data[ 0 ], data.size(), error ) && data == original );
        // no data at all
        ASSERT_TRUE( !CUnrealDeltaTextureDecoder::Decode( "TSF_BGRA8", 64, 3, GUCEF_NULL, 0, error ) && !error.IsNULLOrEmpty() );
        // a format whose layout is not established
        ASSERT_TRUE( !CUnrealDeltaTextureDecoder::Decode( "TSF_RGBA8", 64, 3, &data[ 0 ], data.size(), error ) && error.HasSubstr( "TSF_RGBA8" ) >= 0 && data == original );
        // a size whose strips are not established
        GUCEF::vector< UInt8 > wide( 300 * 200 * 4, 2 );
        const GUCEF::vector< UInt8 > wideOriginal = wide;
        ASSERT_TRUE( !CUnrealDeltaTextureDecoder::Decode( "TSF_BGRA8", 300, 200, &wide[ 0 ], wide.size(), error ) && error.HasSubstr( "not established" ) >= 0 && wide == wideOriginal );
        // a single row and a single pixel are their own first rows
        ASSERT_TRUE( CUnrealDeltaTextureDecoder::Decode( "TSF_BGRA8", 64, 1, &data[ 0 ], data.size(), error ) && data == original );
        ASSERT_TRUE( CUnrealDeltaTextureDecoder::Decode( "TSF_G8", 1, 1, &data[ 0 ], 1, error ) && 1 == data[ 0 ] );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
