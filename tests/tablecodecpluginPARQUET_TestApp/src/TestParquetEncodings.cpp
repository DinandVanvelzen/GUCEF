/*
 *  tablecodecpluginPARQUET_TestApp: tests of the parquet table codec plugin
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

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef TABLECODECPLUGIN_PARQUET_CRLEBITPACKEDDECODER_H
#include "tablecodecpluginPARQUET_CRleBitPackedDecoder.h"
#define TABLECODECPLUGIN_PARQUET_CRLEBITPACKEDDECODER_H
#endif /* TABLECODECPLUGIN_PARQUET_CRLEBITPACKEDDECODER_H ? */

#ifndef TABLECODECPLUGIN_PARQUET_CRLEBITPACKEDENCODER_H
#include "tablecodecpluginPARQUET_CRleBitPackedEncoder.h"
#define TABLECODECPLUGIN_PARQUET_CRLEBITPACKEDENCODER_H
#endif /* TABLECODECPLUGIN_PARQUET_CRLEBITPACKEDENCODER_H ? */

#include "TestParquetSupport.h"
#include "TestParquetEncodings.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;
using namespace GUCEF::CORE;
using namespace GUCEF::TABLECODECPLUGIN::PARQUET;
using namespace PARQUETTEST;

namespace {

typedef GUCEF::vector< UInt32 > TValues;
typedef GUCEF::vector< UInt8 > TBytes;

void
PutVarint( TBytes& out, UInt64 value )
{
    do
    {
        UInt8 byte = (UInt8) ( value & 0x7F );
        value >>= 7;
        if ( 0 != value ) byte |= 0x80;
        out.push_back( byte );
    }
    while ( 0 != value );
}

/** The reference: a literal run packed bit by bit, as the specification describes it */
void
ReferenceLiteral( TBytes& out, const TValues& values, UInt32 first, UInt32 groups, UInt32 bitWidth )
{
    PutVarint( out, ( (UInt64) groups << 1 ) | 1 );
    const size_t start = out.size();
    out.resize( start + (size_t) groups * bitWidth, 0 );
    for ( UInt32 i=0; i<groups*8; ++i )
    {
        const UInt32 value = first + i < values.size() ? values[ first + i ] : 0;
        for ( UInt32 b=0; b<bitWidth; ++b )
        {
            if ( 0 != ( value & ( 1u << b ) ) )
            {
                const UInt64 bit = (UInt64) i * bitWidth + b;
                out[ start + (size_t) ( bit >> 3 ) ] |= (UInt8) ( 1u << ( bit & 7 ) );
            }
        }
    }
}

/** The reference: an RLE run */
void
ReferenceRepeat( TBytes& out, UInt32 value, UInt32 count, UInt32 bitWidth )
{
    PutVarint( out, (UInt64) count << 1 );
    for ( UInt32 b=0; b<( bitWidth + 7 ) / 8; ++b )
        out.push_back( (UInt8) ( value >> ( 8 * b ) ) );
}

/** The reference decoder: bit by bit */
bool
ReferenceDecode( const UInt8* data, UInt32 size, UInt32 bitWidth, UInt32 count, TValues& values )
{
    values.clear();
    UInt32 position = 0;
    while ( values.size() < count && position < size )
    {
        UInt64 header = 0;
        UInt32 shift = 0;
        for ( ;; )
        {
            if ( position >= size ) return false;
            const UInt8 byte = data[ position++ ];
            header |= (UInt64) ( byte & 0x7F ) << shift;
            if ( 0 == ( byte & 0x80 ) ) break;
            shift += 7;
        }
        if ( 0 != ( header & 1 ) )
        {
            const UInt64 n = ( header >> 1 ) * 8;
            for ( UInt64 i=0; i<n; ++i )
            {
                UInt32 value = 0;
                for ( UInt32 b=0; b<bitWidth; ++b )
                {
                    const UInt64 bit = i * bitWidth + b;
                    if ( position + ( bit >> 3 ) >= size ) return false;
                    if ( 0 != ( data[ position + ( bit >> 3 ) ] & ( 1u << ( bit & 7 ) ) ) )
                        value |= 1u << b;
                }
                if ( values.size() < count )
                    values.push_back( value );
            }
            position += (UInt32) ( ( header >> 1 ) * bitWidth );
        }
        else
        {
            UInt32 value = 0;
            for ( UInt32 b=0; b<( bitWidth + 7 ) / 8; ++b )
                value |= (UInt32) data[ position++ ] << ( 8 * b );
            for ( UInt64 i=0; i<( header >> 1 ) && values.size() < count; ++i )
                values.push_back( value );
        }
    }
    return values.size() == count;
}

/** Values of a pattern: random, one repeated value, runs of varying lengths, or a ramp */
void
MakeValues( TValues& values, UInt32 count, UInt32 bitWidth, UInt32 pattern, CTestRandom& random )
{
    const UInt64 mask = 0 == bitWidth ? 0 : ( ( (UInt64) 1 << bitWidth ) - 1 );
    values.resize( count );
    UInt32 run = 0;
    UInt32 runValue = 0;
    for ( UInt32 i=0; i<count; ++i )
    {
        switch ( pattern )
        {
            case 0: { values[ i ] = (UInt32) ( random.Next() & mask ); break; }
            case 1: { values[ i ] = (UInt32) mask; break; }
            case 2:
            {
                if ( 0 == run ) { run = 1 + random.Below( 20 ); runValue = (UInt32) ( random.Next() & mask ); }
                values[ i ] = runValue;
                --run;
                break;
            }
            default: { values[ i ] = (UInt32) ( i & mask ); break; }
        }
    }
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformParquetEncodingsTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "tablecodecpluginPARQUET.Encodings" );

    GUCEF_TESTFW_TESTCASE( "Hybrid: the example of the specification, the numbers 0 to 7 at bit width 3, and an RLE run" )
    try
    {
        const UInt8 packed[] = { 0x03, 0x88, 0xC6, 0xFA };      // one group of 8, bytes 10001000 11000110 11111010
        CRleBitPackedDecoder decoder;
        ASSERT_TRUE( decoder.Reset( packed, sizeof( packed ), 3 ) );
        UInt32 values[ 8 ] = { 0 };
        ASSERT_TRUE( 8 == decoder.GetValues( values, 8 ) );
        for ( UInt32 i=0; i<8; ++i )
            ASSERT_TRUE( i == values[ i ] );
        ASSERT_TRUE( 0 == decoder.GetValues( values, 1 ) && !decoder.HasFailed() );

        const UInt8 repeated[] = { 0x10, 0x05 };                // 8 times the value 5 in one byte
        ASSERT_TRUE( decoder.Reset( repeated, sizeof( repeated ), 3 ) );
        ASSERT_TRUE( 8 == decoder.GetValues( values, 8 ) );
        for ( UInt32 i=0; i<8; ++i )
            ASSERT_TRUE( 5 == values[ i ] );

        CDynamicBuffer out;
        TValues ramp;
        for ( UInt32 i=0; i<8; ++i ) ramp.push_back( i );
        CRleBitPackedEncoder::EncodeValues( &ramp[ 0 ], 8, 3, out );
        ASSERT_TRUE( 4 == out.GetDataSize() && 0 == memcmp( out.GetConstBufferPtr(), packed, 4 ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Hybrid: every bit width 0 to 32, every pattern and boundary length, decoded as the reference loop decodes and encoded as the reference loop reads" )
    try
    {
        const UInt32 counts[] = { 0, 1, 7, 8, 9, 15, 16, 17, 63, 64, 65, 1000, 4097 };
        CTestRandom random( 7 );
        UInt32 failures = 0;
        for ( UInt32 bitWidth=0; bitWidth<=32; ++bitWidth )
        {
            for ( UInt32 pattern=0; pattern<4; ++pattern )
            {
                for ( UInt32 c=0; c<sizeof( counts ) / sizeof( counts[ 0 ] ); ++c )
                {
                    const UInt32 count = counts[ c ];
                    TValues values;
                    MakeValues( values, count, bitWidth, pattern, random );

                    // our encoder, read by the reference decoder and by ours
                    CDynamicBuffer encoded;
                    if ( count > 0 )
                        CRleBitPackedEncoder::EncodeValues( &values[ 0 ], count, bitWidth, encoded );
                    TValues reference;
                    const bool referenceOk = ReferenceDecode( static_cast< const UInt8* >( encoded.GetConstBufferPtr() ), encoded.GetDataSize(), bitWidth, count, reference );
                    if ( !referenceOk || reference != values )
                        ++failures;

                    CRleBitPackedDecoder decoder;
                    TValues decoded( count + 1, 0xDEADBEEF );
                    decoder.Reset( static_cast< const UInt8* >( encoded.GetConstBufferPtr() ), encoded.GetDataSize(), bitWidth );
                    // in pieces of varying size, so runs and groups cross the boundaries of the calls
                    UInt32 got = 0;
                    UInt32 piece = 1;
                    while ( got < count )
                    {
                        const UInt32 n = decoder.GetValues( &decoded[ got ], count - got < piece ? count - got : piece );
                        if ( 0 == n ) break;
                        got += n;
                        piece = piece * 3 % 37 + 1;
                    }
                    decoded.resize( count );
                    if ( got != count || decoded != values || decoder.HasFailed() )
                        ++failures;

                    // the reference encoding (literal groups and runs alternating), read by our decoder
                    TBytes manual;
                    UInt32 i = 0;
                    UInt32 turn = 0;
                    while ( i < count )
                    {
                        if ( 0 == turn % 2 )
                        {
                            const UInt32 groups = 1 + random.Below( 3 );
                            ReferenceLiteral( manual, values, i, groups, bitWidth );
                            i += groups * 8;
                        }
                        else
                        {
                            UInt32 run = 1;
                            while ( i + run < count && values[ i + run ] == values[ i ] && run < 1000 ) ++run;
                            ReferenceRepeat( manual, values[ i ], run, bitWidth );
                            i += run;
                        }
                        ++turn;
                    }
                    decoded.assign( count, 0 );
                    decoder.Reset( manual.empty() ? GUCEF_NULL : &manual[ 0 ], (UInt32) manual.size(), bitWidth );
                    const UInt32 manualGot = 0 == count ? 0 : decoder.GetValues( &decoded[ 0 ], count );
                    if ( manualGot != count || decoded != values )
                        ++failures;
                }
            }
        }
        if ( 0 != failures )
            GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "Hybrid: " << failures << " combinations differ from the reference" );
        ASSERT_TRUE( 0 == failures );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Hybrid: definition levels become validity bits at any bit offset, bitmaps round trip byte wise, skipping keeps the position" )
    try
    {
        CTestRandom random( 11 );
        const UInt32 counts[] = { 1, 5, 8, 13, 64, 100, 1000, 16384, 16389 };
        for ( UInt32 c=0; c<sizeof( counts ) / sizeof( counts[ 0 ] ); ++c )
        {
            const UInt32 count = counts[ c ];
            for ( UInt32 pattern=0; pattern<4; ++pattern )
            {
                // a validity bitmap: random, all set, all clear, long stretches
                TBytes bitmap( ( count + 7 ) / 8, 0 );
                UInt32 zeros = 0;
                for ( UInt32 i=0; i<count; ++i )
                {
                    bool bit = true;
                    if ( 0 == pattern ) bit = 0 != ( random.Next() & 1 );
                    else if ( 2 == pattern ) bit = false;
                    else if ( 3 == pattern ) bit = ( i / 50 ) % 3 != 1;
                    if ( bit ) bitmap[ i >> 3 ] |= (UInt8) ( 1u << ( i & 7 ) ); else ++zeros;
                }
                CDynamicBuffer encoded;
                CRleBitPackedEncoder::EncodeBits( &bitmap[ 0 ], count, encoded );
                const UInt32 offsets[] = { 0, 3, 8, 13 };
                for ( UInt32 o=0; o<4; ++o )
                {
                    const UInt32 first = offsets[ o ];
                    TBytes target( ( first + count + 7 ) / 8 + 8, 0 );
                    CRleBitPackedDecoder decoder;
                    ASSERT_TRUE( decoder.Reset( static_cast< const UInt8* >( encoded.GetConstBufferPtr() ), encoded.GetDataSize(), 1 ) );
                    UInt32 decodedZeros = 0;
                    ASSERT_TRUE( count == decoder.GetBits( &target[ 0 ], first, count, decodedZeros ) );
                    ASSERT_TRUE( zeros == decodedZeros );
                    bool same = true;
                    for ( UInt32 i=0; i<count; ++i )
                    {
                        const bool expected = 0 != ( bitmap[ i >> 3 ] & ( 1u << ( i & 7 ) ) );
                        const UInt32 bit = first + i;
                        same = same && expected == ( 0 != ( target[ bit >> 3 ] & ( 1u << ( bit & 7 ) ) ) );
                    }
                    for ( UInt32 i=0; i<first; ++i )
                        same = same && 0 == ( target[ i >> 3 ] & ( 1u << ( i & 7 ) ) );
                    ASSERT_TRUE( same );
                }

                // the same levels as values
                TValues values;
                CRleBitPackedDecoder decoder;
                decoder.Reset( static_cast< const UInt8* >( encoded.GetConstBufferPtr() ), encoded.GetDataSize(), 1 );
                values.resize( count );
                ASSERT_TRUE( count == decoder.GetValues( &values[ 0 ], count ) );
                bool same = true;
                for ( UInt32 i=0; i<count; ++i )
                    same = same && values[ i ] == ( 0 != ( bitmap[ i >> 3 ] & ( 1u << ( i & 7 ) ) ) ? 1u : 0u );
                ASSERT_TRUE( same );

                // skip some, read the rest
                decoder.Reset( static_cast< const UInt8* >( encoded.GetConstBufferPtr() ), encoded.GetDataSize(), 1 );
                const UInt32 skip = count / 3;
                ASSERT_TRUE( skip == decoder.Skip( skip ) );
                ASSERT_TRUE( count - skip == decoder.GetValues( &values[ 0 ], count - skip ) );
                same = true;
                for ( UInt32 i=skip; i<count; ++i )
                    same = same && values[ i - skip ] == ( 0 != ( bitmap[ i >> 3 ] & ( 1u << ( i & 7 ) ) ) ? 1u : 0u );
                ASSERT_TRUE( same );
            }
        }

        // all valid is one run of a few bytes, whatever the count
        CDynamicBuffer ones;
        CRleBitPackedEncoder::EncodeRepeatedBit( true, 16384, ones );
        ASSERT_TRUE( ones.GetDataSize() <= 4 );
        ASSERT_TRUE( 0 == CRleBitPackedEncoder::BitWidthFor( 0 ) && 1 == CRleBitPackedEncoder::BitWidthFor( 1 ) && 2 == CRleBitPackedEncoder::BitWidthFor( 3 ) );
        ASSERT_TRUE( 3 == CRleBitPackedEncoder::BitWidthFor( 4 ) && 31 == CRleBitPackedEncoder::BitWidthFor( 0x7FFFFFFF ) && 32 == CRleBitPackedEncoder::BitWidthFor( 0xFFFFFFFF ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Hybrid: hostile runs fail with an error and never read past the data: bit widths above 32, zero length runs, oversized values and lengths, truncation" )
    try
    {
        CRleBitPackedDecoder decoder;
        const UInt8 some[] = { 0x10, 0x01 };
        const UInt32 widths[] = { 33, 40, 63, 64, 65, 255 };
        for ( UInt32 w=0; w<sizeof( widths ) / sizeof( widths[ 0 ] ); ++w )
        {
            ASSERT_FALSE( decoder.Reset( some, sizeof( some ), widths[ w ] ) );
            UInt32 value = 0;
            ASSERT_TRUE( 0 == decoder.GetValues( &value, 1 ) && decoder.HasFailed() );
        }

        UInt32 values[ 64 ];
        const UInt8 zeroRle[] = { 0x00, 0x01 };
        ASSERT_TRUE( decoder.Reset( zeroRle, sizeof( zeroRle ), 1 ) );
        ASSERT_TRUE( 0 == decoder.GetValues( values, 1 ) && decoder.HasFailed() );
        const UInt8 zeroLiteral[] = { 0x01, 0xFF };
        ASSERT_TRUE( decoder.Reset( zeroLiteral, sizeof( zeroLiteral ), 1 ) );
        ASSERT_TRUE( 0 == decoder.GetValues( values, 1 ) && decoder.HasFailed() );
        const UInt8 valueTooBig[] = { 0x02, 0x08 };                    // value 8 at bit width 3
        ASSERT_TRUE( decoder.Reset( valueTooBig, sizeof( valueTooBig ), 3 ) );
        ASSERT_TRUE( 0 == decoder.GetValues( values, 1 ) && decoder.HasFailed() );
        const UInt8 hugeRun[] = { 0xFE, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x01 };   // 2^32 - 1 repeats: above 2^31 - 1
        ASSERT_TRUE( decoder.Reset( hugeRun, sizeof( hugeRun ), 1 ) );
        ASSERT_TRUE( 0 == decoder.GetValues( values, 1 ) && decoder.HasFailed() );
        const UInt8 overlong[] = { 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x01 };
        ASSERT_TRUE( decoder.Reset( overlong, sizeof( overlong ), 1 ) );
        ASSERT_TRUE( 0 == decoder.GetValues( values, 1 ) && decoder.HasFailed() );
        const UInt8 rleCut[] = { 0x10 };                               // a run without its value
        ASSERT_TRUE( decoder.Reset( rleCut, sizeof( rleCut ), 8 ) );
        ASSERT_TRUE( 0 == decoder.GetValues( values, 1 ) && decoder.HasFailed() );

        // a literal run that the data ends in: the complete values are given, nothing past them
        const UInt8 literalCut[] = { 0x05, 0xFF, 0xFF };                // 2 groups of width 4 announced: 8 bytes, 2 present
        ASSERT_TRUE( decoder.Reset( literalCut, sizeof( literalCut ), 4 ) );
        ASSERT_TRUE( 4 == decoder.GetValues( values, 16 ) );
        for ( UInt32 i=0; i<4; ++i )
            ASSERT_TRUE( 15 == values[ i ] );

        // random bytes at every bit width: whatever comes out, the decoder stays inside the data (ASan checks it)
        CTestRandom random( 99 );
        for ( UInt32 round=0; round<2000; ++round )
        {
            TBytes junk( 1 + random.Below( 40 ) );
            for ( size_t i=0; i<junk.size(); ++i )
                junk[ i ] = (UInt8) random.Next();
            decoder.Reset( &junk[ 0 ], (UInt32) junk.size(), random.Below( 33 ) );
            TValues out( 300 );
            decoder.GetValues( &out[ 0 ], 300 );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
