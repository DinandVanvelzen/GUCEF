/*
 *  AssetConverterLib_TestApp: Test application for the asset converter library (AssetConverterLib)
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
#include <algorithm>

#ifndef GUCEF_TESTFW_H
#include "gucef_test_framework.h"
#define GUCEF_TESTFW_H
#endif /* GUCEF_TESTFW_H ? */

#include "TestAssetConverterSupport.h"
#include "TestAssetConverterProcessing.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;
using namespace GUCEF::ASSETCONVERTER;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

namespace {

typedef GUCEF::vector< UInt8 > TBytes;

void PutU16( TBytes& b, UInt32 v ) { b.push_back( (UInt8) ( v & 0xFF ) ); b.push_back( (UInt8) ( ( v >> 8 ) & 0xFF ) ); }
void PutU32( TBytes& b, UInt32 v ) { PutU16( b, v & 0xFFFF ); PutU16( b, v >> 16 ); }
void PutU16BE( TBytes& b, UInt32 v ) { b.push_back( (UInt8) ( ( v >> 8 ) & 0xFF ) ); b.push_back( (UInt8) ( v & 0xFF ) ); }
void PutU32BE( TBytes& b, UInt32 v ) { PutU16BE( b, v >> 16 ); PutU16BE( b, v & 0xFFFF ); }
void PutText( TBytes& b, const char* text ) { while ( 0 != *text ) b.push_back( (UInt8) *text++ ); }
void PutFloat( TBytes& b, Float32 v ) { UInt32 bits = 0; memcpy( &bits, &v, 4 ); PutU32( b, bits ); }

const UInt32 FormatSize = 4;

/** The texel of the test pictures: the colour tells where in the picture it is, so a flipped or swapped picture is told */
void
TestTexel( UInt32 x, UInt32 y, UInt8& r, UInt8& g, UInt8& b )
{
    r = (UInt8) ( 40 + x * 50 );
    g = (UInt8) ( 40 + y * 50 );
    b = 90;
}

TBytes
MakeTga( void )
{
    TBytes b;
    const UInt8 header[ 12 ] = { 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    b.insert( b.end(), header, header + 12 );
    PutU16( b, FormatSize ); PutU16( b, FormatSize );
    b.push_back( 24 ); b.push_back( 0 );     // 24 bits, the first row is the bottom one
    for ( UInt32 y=FormatSize; y>0; --y )
        for ( UInt32 x=0; x<FormatSize; ++x )
        {
            UInt8 r, g, bl;
            TestTexel( x, y - 1, r, g, bl );
            b.push_back( bl ); b.push_back( g ); b.push_back( r );
        }
    return b;
}

TBytes
MakeBmp( void )
{
    TBytes b;
    PutText( b, "BM" );
    PutU32( b, 14 + 40 + FormatSize * FormatSize * 3 ); PutU32( b, 0 ); PutU32( b, 14 + 40 );
    PutU32( b, 40 ); PutU32( b, FormatSize ); PutU32( b, FormatSize ); PutU16( b, 1 ); PutU16( b, 24 );
    PutU32( b, 0 ); PutU32( b, FormatSize * FormatSize * 3 ); PutU32( b, 2835 ); PutU32( b, 2835 ); PutU32( b, 0 ); PutU32( b, 0 );
    for ( UInt32 y=FormatSize; y>0; --y )
        for ( UInt32 x=0; x<FormatSize; ++x )
        {
            UInt8 r, g, bl;
            TestTexel( x, y - 1, r, g, bl );
            b.push_back( bl ); b.push_back( g ); b.push_back( r );
        }
    return b;
}

TBytes
MakePsd( void )
{
    TBytes b;
    PutText( b, "8BPS" );
    PutU16BE( b, 1 );
    for ( UInt32 i=0; i<6; ++i ) b.push_back( 0 );
    PutU16BE( b, 3 ); PutU32BE( b, FormatSize ); PutU32BE( b, FormatSize ); PutU16BE( b, 8 ); PutU16BE( b, 3 );
    PutU32BE( b, 0 ); PutU32BE( b, 0 ); PutU32BE( b, 0 );    // colour mode data, image resources, layers
    PutU16BE( b, 0 );                                        // raw, planar
    for ( UInt32 plane=0; plane<3; ++plane )
        for ( UInt32 y=0; y<FormatSize; ++y )
            for ( UInt32 x=0; x<FormatSize; ++x )
            {
                UInt8 r, g, bl;
                TestTexel( x, y, r, g, bl );
                b.push_back( 0 == plane ? r : ( 1 == plane ? g : bl ) );
            }
    return b;
}

TBytes
MakeTiff( void )
{
    TBytes b;
    PutText( b, "II" ); PutU16( b, 42 ); PutU32( b, 8 );
    const UInt32 entries = 10;
    const UInt32 bitsOffset = 8 + 2 + entries * 12 + 4;
    const UInt32 dataOffset = bitsOffset + 6;
    PutU16( b, entries );
    struct SEntry { UInt32 tag, type, count, value; };
    const SEntry list[ entries ] = {
        { 256, 4, 1, FormatSize }, { 257, 4, 1, FormatSize }, { 258, 3, 3, bitsOffset }, { 259, 3, 1, 1 }, { 262, 3, 1, 2 },
        { 273, 4, 1, dataOffset }, { 277, 3, 1, 3 }, { 278, 4, 1, FormatSize }, { 279, 4, 1, FormatSize * FormatSize * 3 }, { 284, 3, 1, 1 } };
    for ( UInt32 e=0; e<entries; ++e )
    {
        PutU16( b, list[ e ].tag ); PutU16( b, list[ e ].type ); PutU32( b, list[ e ].count );
        if ( 3 == list[ e ].type && 1 == list[ e ].count ) { PutU16( b, list[ e ].value ); PutU16( b, 0 ); } else PutU32( b, list[ e ].value );
    }
    PutU32( b, 0 );
    PutU16( b, 8 ); PutU16( b, 8 ); PutU16( b, 8 );
    for ( UInt32 y=0; y<FormatSize; ++y )
        for ( UInt32 x=0; x<FormatSize; ++x )
        {
            UInt8 r, g, bl;
            TestTexel( x, y, r, g, bl );
            b.push_back( r ); b.push_back( g ); b.push_back( bl );
        }
    return b;
}

TBytes
MakeDds( void )
{
    TBytes b;
    PutText( b, "DDS " );
    PutU32( b, 124 ); PutU32( b, 0x100F ); PutU32( b, FormatSize ); PutU32( b, FormatSize ); PutU32( b, FormatSize * 4 ); PutU32( b, 0 ); PutU32( b, 0 );
    for ( UInt32 i=0; i<11; ++i ) PutU32( b, 0 );
    PutU32( b, 32 ); PutU32( b, 0x41 ); PutU32( b, 0 ); PutU32( b, 32 ); PutU32( b, 0x00FF0000 ); PutU32( b, 0x0000FF00 ); PutU32( b, 0x000000FF ); PutU32( b, 0xFF000000 );
    PutU32( b, 0x1000 ); PutU32( b, 0 ); PutU32( b, 0 ); PutU32( b, 0 ); PutU32( b, 0 );
    for ( UInt32 y=0; y<FormatSize; ++y )
        for ( UInt32 x=0; x<FormatSize; ++x )
        {
            UInt8 r, g, bl;
            TestTexel( x, y, r, g, bl );
            b.push_back( bl ); b.push_back( g ); b.push_back( r ); b.push_back( 255 );
        }
    return b;
}

/** Radiance HDR, flat (not run length coded) RGBE texels with the 8 bit value of TestTexel as mantissa and the exponent 128 */
TBytes
MakeHdr( void )
{
    TBytes b;
    PutText( b, "#?RADIANCE\nFORMAT=32-bit_rle_rgbe\n\n-Y 4 +X 4\n" );
    for ( UInt32 y=0; y<FormatSize; ++y )
        for ( UInt32 x=0; x<FormatSize; ++x )
        {
            UInt8 r, g, bl;
            TestTexel( x, y, r, g, bl );
            // the value of RGBE texel (m, m, m, 128) is m / 256
            b.push_back( r ); b.push_back( g ); b.push_back( bl ); b.push_back( 128 );
        }
    return b;
}

/** OpenEXR, scanlines, no compression, 32 bit float channels B, G, R holding the 8 bit value of TestTexel over 255 */
TBytes
MakeExr( void )
{
    TBytes b;
    b.push_back( 0x76 ); b.push_back( 0x2F ); b.push_back( 0x31 ); b.push_back( 0x01 );
    PutU32( b, 2 );
    PutText( b, "channels" ); b.push_back( 0 ); PutText( b, "chlist" ); b.push_back( 0 );
    PutU32( b, 3 * ( 2 + 16 ) + 1 );
    const char* const names[ 3 ] = { "B", "G", "R" };
    for ( UInt32 c=0; c<3; ++c )
    {
        PutText( b, names[ c ] ); b.push_back( 0 );
        PutU32( b, 2 ); b.push_back( 0 ); b.push_back( 0 ); b.push_back( 0 ); b.push_back( 0 ); PutU32( b, 1 ); PutU32( b, 1 );
    }
    b.push_back( 0 );
    PutText( b, "compression" ); b.push_back( 0 ); PutText( b, "compression" ); b.push_back( 0 ); PutU32( b, 1 ); b.push_back( 0 );
    const char* const boxes[ 2 ] = { "dataWindow", "displayWindow" };
    for ( UInt32 w=0; w<2; ++w )
    {
        PutText( b, boxes[ w ] ); b.push_back( 0 ); PutText( b, "box2i" ); b.push_back( 0 ); PutU32( b, 16 );
        PutU32( b, 0 ); PutU32( b, 0 ); PutU32( b, FormatSize - 1 ); PutU32( b, FormatSize - 1 );
    }
    PutText( b, "lineOrder" ); b.push_back( 0 ); PutText( b, "lineOrder" ); b.push_back( 0 ); PutU32( b, 1 ); b.push_back( 0 );
    PutText( b, "pixelAspectRatio" ); b.push_back( 0 ); PutText( b, "float" ); b.push_back( 0 ); PutU32( b, 4 ); PutFloat( b, 1.0f );
    PutText( b, "screenWindowCenter" ); b.push_back( 0 ); PutText( b, "v2f" ); b.push_back( 0 ); PutU32( b, 8 ); PutFloat( b, 0.0f ); PutFloat( b, 0.0f );
    PutText( b, "screenWindowWidth" ); b.push_back( 0 ); PutText( b, "float" ); b.push_back( 0 ); PutU32( b, 4 ); PutFloat( b, 1.0f );
    b.push_back( 0 );
    const UInt32 lineBytes = 8 + 3 * FormatSize * 4;
    const UInt32 tableStart = (UInt32) b.size();
    const UInt32 dataStart = tableStart + FormatSize * 8;
    for ( UInt32 y=0; y<FormatSize; ++y )
    {
        PutU32( b, dataStart + y * lineBytes ); PutU32( b, 0 );
    }
    for ( UInt32 y=0; y<FormatSize; ++y )
    {
        PutU32( b, y ); PutU32( b, 3 * FormatSize * 4 );
        for ( UInt32 c=0; c<3; ++c )
            for ( UInt32 x=0; x<FormatSize; ++x )
            {
                UInt8 r, g, bl;
                TestTexel( x, y, r, g, bl );
                PutFloat( b, (Float32) ( 0 == c ? bl : ( 1 == c ? g : r ) ) / 255.0f );
            }
    }
    return b;
}

TBytes
MakeJpg( void )
{
    static const UInt8 jpg[] = {
    0xFF, 0xD8, 0xFF, 0xE0, 0x00, 0x10, 0x4A, 0x46, 0x49, 0x46, 0x00, 0x01, 0x01, 0x00, 0x00, 0x01,
    0x00, 0x01, 0x00, 0x00, 0xFF, 0xDB, 0x00, 0x43, 0x00, 0x02, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02,
    0x01, 0x01, 0x01, 0x02, 0x02, 0x02, 0x02, 0x02, 0x04, 0x03, 0x02, 0x02, 0x02, 0x02, 0x05, 0x04,
    0x04, 0x03, 0x04, 0x06, 0x05, 0x06, 0x06, 0x06, 0x05, 0x06, 0x06, 0x06, 0x07, 0x09, 0x08, 0x06,
    0x07, 0x09, 0x07, 0x06, 0x06, 0x08, 0x0B, 0x08, 0x09, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x06, 0x08,
    0x0B, 0x0C, 0x0B, 0x0A, 0x0C, 0x09, 0x0A, 0x0A, 0x0A, 0xFF, 0xDB, 0x00, 0x43, 0x01, 0x02, 0x02,
    0x02, 0x02, 0x02, 0x02, 0x05, 0x03, 0x03, 0x05, 0x0A, 0x07, 0x06, 0x07, 0x0A, 0x0A, 0x0A, 0x0A,
    0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A,
    0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A,
    0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0xFF, 0xC0,
    0x00, 0x11, 0x08, 0x00, 0x08, 0x00, 0x08, 0x03, 0x01, 0x11, 0x00, 0x02, 0x11, 0x01, 0x03, 0x11,
    0x01, 0xFF, 0xC4, 0x00, 0x1F, 0x00, 0x00, 0x01, 0x05, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09,
    0x0A, 0x0B, 0xFF, 0xC4, 0x00, 0xB5, 0x10, 0x00, 0x02, 0x01, 0x03, 0x03, 0x02, 0x04, 0x03, 0x05,
    0x05, 0x04, 0x04, 0x00, 0x00, 0x01, 0x7D, 0x01, 0x02, 0x03, 0x00, 0x04, 0x11, 0x05, 0x12, 0x21,
    0x31, 0x41, 0x06, 0x13, 0x51, 0x61, 0x07, 0x22, 0x71, 0x14, 0x32, 0x81, 0x91, 0xA1, 0x08, 0x23,
    0x42, 0xB1, 0xC1, 0x15, 0x52, 0xD1, 0xF0, 0x24, 0x33, 0x62, 0x72, 0x82, 0x09, 0x0A, 0x16, 0x17,
    0x18, 0x19, 0x1A, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A,
    0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A,
    0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A,
    0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8A, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99,
    0x9A, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7,
    0xB8, 0xB9, 0xBA, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8, 0xC9, 0xCA, 0xD2, 0xD3, 0xD4, 0xD5,
    0xD6, 0xD7, 0xD8, 0xD9, 0xDA, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 0xE8, 0xE9, 0xEA, 0xF1,
    0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7, 0xF8, 0xF9, 0xFA, 0xFF, 0xC4, 0x00, 0x1F, 0x01, 0x00, 0x03,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
    0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0xFF, 0xC4, 0x00, 0xB5, 0x11, 0x00,
    0x02, 0x01, 0x02, 0x04, 0x04, 0x03, 0x04, 0x07, 0x05, 0x04, 0x04, 0x00, 0x01, 0x02, 0x77, 0x00,
    0x01, 0x02, 0x03, 0x11, 0x04, 0x05, 0x21, 0x31, 0x06, 0x12, 0x41, 0x51, 0x07, 0x61, 0x71, 0x13,
    0x22, 0x32, 0x81, 0x08, 0x14, 0x42, 0x91, 0xA1, 0xB1, 0xC1, 0x09, 0x23, 0x33, 0x52, 0xF0, 0x15,
    0x62, 0x72, 0xD1, 0x0A, 0x16, 0x24, 0x34, 0xE1, 0x25, 0xF1, 0x17, 0x18, 0x19, 0x1A, 0x26, 0x27,
    0x28, 0x29, 0x2A, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49,
    0x4A, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69,
    0x6A, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88,
    0x89, 0x8A, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9A, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6,
    0xA7, 0xA8, 0xA9, 0xAA, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0xBA, 0xC2, 0xC3, 0xC4,
    0xC5, 0xC6, 0xC7, 0xC8, 0xC9, 0xCA, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7, 0xD8, 0xD9, 0xDA, 0xE2,
    0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 0xE8, 0xE9, 0xEA, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7, 0xF8, 0xF9,
    0xFA, 0xFF, 0xDA, 0x00, 0x0C, 0x03, 0x01, 0x00, 0x02, 0x11, 0x03, 0x11, 0x00, 0x3F, 0x00, 0xCF,
    0xAF, 0xE5, 0x73, 0xFB, 0x60, 0xFF, 0xD9,
    };
    return TBytes( jpg, jpg + sizeof( jpg ) );
}

/** The CRC-32 of PNG chunks */
UInt32
PngCrc( const UInt8* bytes, size_t size )
{
    UInt32 crc = 0xFFFFFFFF;
    for ( size_t i=0; i<size; ++i )
    {
        crc ^= bytes[ i ];
        for ( UInt32 k=0; k<8; ++k )
            crc = ( crc & 1 ) ? ( 0xEDB88320U ^ ( crc >> 1 ) ) : ( crc >> 1 );
    }
    return crc ^ 0xFFFFFFFF;
}

void
PutPngChunk( TBytes& b, const char* type, const TBytes& data )
{
    PutU32BE( b, (UInt32) data.size() );
    TBytes typed;
    PutText( typed, type );
    typed.insert( typed.end(), data.begin(), data.end() );
    b.insert( b.end(), typed.begin(), typed.end() );
    PutU32BE( b, PngCrc( &typed[ 0 ], typed.size() ) );
}

/** The grey value of the greyscale test picture at a texel */
UInt8
GreyTexel( UInt32 x, UInt32 y )
{
    return (UInt8) ( 30 + x * 40 + y * 9 );
}

/** A greyscale PNG (colour type 0) of FormatSize x FormatSize with 8 or 16 bits per texel, its data in one stored deflate block */
TBytes
MakeGreyPng( UInt32 bits )
{
    TBytes b;
    const UInt8 signature[ 8 ] = { 0x89, 'P', 'N', 'G', 0x0D, 0x0A, 0x1A, 0x0A };
    b.insert( b.end(), signature, signature + 8 );
    TBytes header;
    PutU32BE( header, FormatSize ); PutU32BE( header, FormatSize );
    header.push_back( (UInt8) bits ); header.push_back( 0 ); header.push_back( 0 ); header.push_back( 0 ); header.push_back( 0 );
    PutPngChunk( b, "IHDR", header );
    TBytes scan;
    for ( UInt32 y=0; y<FormatSize; ++y )
    {
        scan.push_back( 0 );
        for ( UInt32 x=0; x<FormatSize; ++x )
        {
            const UInt8 grey = GreyTexel( x, y );
            scan.push_back( grey );
            if ( 16 == bits )
                scan.push_back( grey );     // big endian: grey * 257
        }
    }
    UInt32 a = 1, c = 0;
    for ( size_t i=0; i<scan.size(); ++i )
    {
        a = ( a + scan[ i ] ) % 65521;
        c = ( c + a ) % 65521;
    }
    TBytes zlib;
    zlib.push_back( 0x78 ); zlib.push_back( 0x01 );
    zlib.push_back( 1 );
    PutU16( zlib, (UInt32) scan.size() ); PutU16( zlib, (UInt32) ( ~scan.size() & 0xFFFF ) );
    zlib.insert( zlib.end(), scan.begin(), scan.end() );
    PutU32BE( zlib, ( c << 16 ) | a );
    PutPngChunk( b, "IDAT", zlib );
    PutPngChunk( b, "IEND", TBytes() );
    return b;
}

bool
StoreBytes( const CORE::CString& vfsPath, const TBytes& bytes )
{
    CORE::CDynamicBuffer buffer;
    buffer.CopyFrom( (UInt32) bytes.size(), &bytes[ 0 ] );
    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    const CORE::CString folder = CVfsPaths::FolderOf( vfsPath );
    if ( !folder.IsNULLOrEmpty() )
        vfs.CreateDir( folder );
    return vfs.StoreAsFile( vfsPath, buffer, 0, true );
}

/** A box of the sizes around the origin of the node: 8 corners, 12 triangles */
Int32
AddBoxMesh( CAssetScene& scene, UInt32 node, Float32 sx, Float32 sy, Float32 sz )
{
    CAssetMesh mesh;
    mesh.name = scene.nodes[ node ].name;
    CAssetSubMesh subMesh;
    for ( UInt32 i=0; i<8; ++i )
    {
        subMesh.positions.push_back( ( i & 1 ) ? sx * 0.5f : -sx * 0.5f );
        subMesh.positions.push_back( ( i & 2 ) ? sy * 0.5f : -sy * 0.5f );
        subMesh.positions.push_back( ( i & 4 ) ? sz * 0.5f : -sz * 0.5f );
    }
    const UInt32 faces[] = { 0,2,1, 1,2,3,  4,5,6, 5,7,6,  0,1,4, 1,5,4,  2,6,3, 3,6,7,  0,4,2, 2,4,6,  1,3,5, 3,7,5 };
    for ( UInt32 i=0; i<sizeof( faces ) / sizeof( faces[ 0 ] ); ++i )
        subMesh.indices.push_back( faces[ i ] );
    mesh.subMeshes.push_back( subMesh );
    scene.meshes.push_back( mesh );
    scene.nodes[ node ].mesh = (Int32) scene.meshes.size() - 1;
    return scene.nodes[ node ].mesh;
}

UInt32
CountItems( const CConversionReport& report, TReportItemKind kind, const CORE::CString& source )
{
    UInt32 count = 0;
    const CConversionReport::TEntryVector& entries = report.GetEntries();
    for ( size_t e=0; e<entries.size(); ++e )
        for ( size_t i=0; i<entries[ e ].items.size(); ++i )
            if ( entries[ e ].items[ i ].kind == kind && entries[ e ].items[ i ].source == source )
                ++count;
    return count;
}

const CAssetCollider*
FindCollider( const CAssetNode& node, TColliderShape shape )
{
    for ( size_t c=0; c<node.colliders.size(); ++c )
        if ( node.colliders[ c ].shape == shape )
            return &node.colliders[ c ];
    return GUCEF_NULL;
}

bool
SameRotation( const MATH::TFloat32Quaternion& a, const MATH::TFloat32Quaternion& b )
{
    const Float32 dot = a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
    return fabs( fabs( dot ) - 1.0f ) < 1e-4f;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** An importer that declares file kinds and recognises a pack by a marker file, for the tests of the kind registry */
class CKindsTestImporter : public CIAssetImporter
{
    public:

    CKindsTestImporter( const CORE::CString& name, const CORE::CString& marker, UInt32 confidence )
        : CIAssetImporter(), m_name( name ), m_marker( marker ), m_confidence( confidence ) {}

    virtual CORE::CString GetName( void ) const { return m_name; }
    virtual UInt32 GetVersion( void ) const { return 1; }
    virtual bool CanImport( const CORE::CString&, CAssetPackContext& ) const { return false; }
    virtual bool Import( const CORE::CString&, CAssetPackContext&, CAssetScene&, CConversionReport& ) { return false; }

    virtual void GetFileKinds( TAssetFileKindVector& kinds ) const
    {
        kinds.push_back( CAssetFileKind( "kt" + m_name, ASSETKIND_PREFAB ) );
        kinds.push_back( CAssetFileKind( "PNG", ASSETKIND_TEXTURE ) );          // a later declaration of a known extension wins, in any case
        kinds.push_back( CAssetFileKind( ".kt" + m_name + "side", ASSETKIND_METADATA, "kt" + m_name ) );
    }

    virtual bool RecognisePack( const CAssetPackListing& listing, CAssetPackProfile& profile ) const
    {
        if ( !listing.HasFile( m_marker ) )
            return false;
        profile.packType = m_name + "-pack";
        profile.packTypeName = m_name + " pack";
        profile.engine = m_name;
        profile.confidence = m_confidence;
        return true;
    }

    private:

    CORE::CString m_name;
    CORE::CString m_marker;
    UInt32 m_confidence;
};

}

/*-------------------------------------------------------------------------*/

static void
TestAssetKinds( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Kinds: the built-in table knows images, textures, models, audio, fonts, documents, data, packages and metadata; letter case does not matter, unknown is other" )
    try
    {
        CAssetKindRegistry kinds;
        const char* const images[] = { "a.png", "a.JPG", "a.jpeg", "a.tga", "a.bmp", "a.gif", "a.tif", "a.tiff", "a.psd", "a.webp", "a.hdr", "a.exr" };
        for ( UInt32 i=0; i<sizeof( images ) / sizeof( images[ 0 ] ); ++i )
            ASSERT_TRUE( ASSETKIND_IMAGE == kinds.GetKindOfPath( images[ i ] ) );
        const char* const textures[] = { "a.dds", "a.KTX", "a.ktx2", "a.astc", "a.pvr", "a.pkm", "a.basis" };
        for ( UInt32 i=0; i<sizeof( textures ) / sizeof( textures[ 0 ] ); ++i )
            ASSERT_TRUE( ASSETKIND_TEXTURE == kinds.GetKindOfPath( textures[ i ] ) );
        const char* const audio[] = { "a.wav", "a.OGG", "a.opus", "a.flac", "a.mp3" };
        for ( UInt32 i=0; i<sizeof( audio ) / sizeof( audio[ 0 ] ); ++i )
            ASSERT_TRUE( ASSETKIND_AUDIO == kinds.GetKindOfPath( audio[ i ] ) );
        ASSERT_TRUE( ASSETKIND_MODEL == kinds.GetKindOfPath( "dir/House.FBX" ) && ASSETKIND_MODEL == kinds.GetKindOfPath( "x.glb" ) && ASSETKIND_MODEL == kinds.GetKindOfPath( "x.mesh" ) );
        ASSERT_TRUE( ASSETKIND_MATERIAL == kinds.GetKindOfPath( "m.material" ) && ASSETKIND_SCENE == kinds.GetKindOfPath( "l.scene" ) && ASSETKIND_SKELETON == kinds.GetKindOfPath( "r.skeleton" ) && ASSETKIND_PARTICLE == kinds.GetKindOfPath( "p.particle" ) );
        ASSERT_TRUE( ASSETKIND_FONT == kinds.GetKindOfPath( "f.ttf" ) && ASSETKIND_DOCUMENT == kinds.GetKindOfPath( "readme.txt" ) && ASSETKIND_DATA == kinds.GetKindOfPath( "d.json" ) );
        ASSERT_TRUE( ASSETKIND_PACKAGE == kinds.GetKindOfPath( "a.zip" ) && ASSETKIND_PACKAGE == kinds.GetKindOfPath( "a.unitypackage" ) && ASSETKIND_PACKAGE == kinds.GetKindOfPath( "a.mhpkg" ) );
        ASSERT_TRUE( ASSETKIND_METADATA == kinds.GetKindOfPath( "a.png.meta" ) && ASSETKIND_METADATA == kinds.GetKindOfPath( "a.png.import" ) && ASSETKIND_METADATA == kinds.GetKindOfPath( "a.UEXP" ) );
        ASSERT_TRUE( ASSETKIND_OTHER == kinds.GetKindOfPath( "a.unknownext" ) && ASSETKIND_OTHER == kinds.GetKindOfPath( "noextension" ) && ASSETKIND_OTHER == kinds.GetKindOfPath( "folder.fbx/inside" ) && ASSETKIND_OTHER == kinds.GetKindOfPath( "" ) && ASSETKIND_OTHER == kinds.GetKindOfPath( "dot." ) );

        ASSERT_TRUE( "Image" == CAssetKindRegistry::GetKindName( ASSETKIND_IMAGE ) && "Texture" == CAssetKindRegistry::GetKindName( ASSETKIND_TEXTURE ) && "Other" == CAssetKindRegistry::GetKindName( ASSETKIND_OTHER ) );
        EAssetKind parsed = ASSETKIND_OTHER;
        ASSERT_TRUE( CAssetKindRegistry::TryParseKindId( "Audio", parsed ) && ASSETKIND_AUDIO == parsed && "audio" == CAssetKindRegistry::GetKindId( parsed ) && !CAssetKindRegistry::TryParseKindId( "nonsense", parsed ) );
        for ( UInt32 k=0; k<(UInt32) ASSETKIND_COUNT; ++k )
        {
            EAssetKind roundTrip = ASSETKIND_COUNT;
            ASSERT_TRUE( CAssetKindRegistry::TryParseKindId( CAssetKindRegistry::GetKindId( (EAssetKind) k ), roundTrip ) && (EAssetKind) k == roundTrip && !CAssetKindRegistry::GetKindName( (EAssetKind) k ).IsNULLOrEmpty() );
        }

        // the extensions of a kind, for the filters of the dialogs
        CORE::CString::StringVector extensions;
        kinds.GetExtensions( ASSETKIND_IMAGE, extensions );
        ASSERT_TRUE( extensions.size() >= 12 && extensions[ 0 ] == "png" );
        extensions.clear();
        kinds.GetExtensions( ASSETKIND_TEXTURE, extensions );
        ASSERT_TRUE( 7 == extensions.size() && extensions[ 0 ] == "dds" );
        extensions.clear();
        kinds.GetExtensions( ASSETKIND_OTHER, extensions );
        ASSERT_TRUE( extensions.empty() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Kinds: metadata files name the asset they belong to, and an asset the metadata files it can have" )
    try
    {
        CAssetKindRegistry kinds;
        CORE::CString::StringVector owners;
        ASSERT_TRUE( kinds.IsMetadataPath( "Assets/Rock.fbx.meta" ) && !kinds.IsMetadataPath( "Assets/Rock.fbx" ) );
        ASSERT_TRUE( kinds.GetOwnerPaths( "Assets/Rock.fbx.meta", owners ) && 1 == owners.size() && "Assets/Rock.fbx" == owners[ 0 ] );
        owners.clear();
        ASSERT_TRUE( kinds.GetOwnerPaths( "icon.png.import", owners ) && 1 == owners.size() && "icon.png" == owners[ 0 ] );
        owners.clear();
        ASSERT_TRUE( kinds.GetOwnerPaths( "Content/Rock.uexp", owners ) && 2 == owners.size() && "Content/Rock.uasset" == owners[ 0 ] && "Content/Rock.umap" == owners[ 1 ] );
        owners.clear();
        ASSERT_FALSE( kinds.GetOwnerPaths( "Content/Rock.uasset", owners ) || kinds.GetOwnerPaths( "noextension", owners ) );

        CORE::CString::StringVector sidecars;
        kinds.GetMetadataPaths( "Assets/Rock.fbx", sidecars );
        bool hasMeta = false, hasImport = false, hasUexp = false;
        for ( size_t i=0; i<sidecars.size(); ++i )
        {
            hasMeta = hasMeta || "Assets/Rock.fbx.meta" == sidecars[ i ];
            hasImport = hasImport || "Assets/Rock.fbx.import" == sidecars[ i ];
            hasUexp = hasUexp || "Assets/Rock.uexp" == sidecars[ i ];
        }
        ASSERT_TRUE( hasMeta && hasImport && !hasUexp );
        sidecars.clear();
        kinds.GetMetadataPaths( "Content/Rock.uasset", sidecars );
        hasUexp = false;
        for ( size_t i=0; i<sidecars.size(); ++i )
            hasUexp = hasUexp || "Content/Rock.uexp" == sidecars[ i ];
        ASSERT_TRUE( hasUexp );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Kinds: the declarations of an importer join the table when it registers and leave when it unregisters; a later declaration wins" )
    try
    {
        CAssetPluginRegistry& plugins = CAssetConverterGlobal::Instance()->GetPluginRegistry();
        CAssetKindRegistry& kinds = plugins.GetKinds();
        ASSERT_TRUE( ASSETKIND_IMAGE == kinds.GetKindOfPath( "a.png" ) && ASSETKIND_OTHER == kinds.GetKindOfPath( "a.ktkinds" ) );
        const UInt32 before = kinds.GetDeclarationCount();

        ASSERT_TRUE( plugins.RegisterImporter( TAssetImporterPtr( GUCEF_NEW CKindsTestImporter( "kinds", "kinds.marker", 40 ) ) ) );
        ASSERT_TRUE( before + 3 == kinds.GetDeclarationCount() );
        ASSERT_TRUE( ASSETKIND_PREFAB == kinds.GetKindOfPath( "x/y.KTKINDS" ) );
        ASSERT_TRUE( ASSETKIND_TEXTURE == kinds.GetKindOfPath( "a.png" ) && ASSETKIND_TEXTURE == kinds.GetKindOfPath( "a.PNG" ) );
        CORE::CString::StringVector extensions;
        kinds.GetExtensions( ASSETKIND_IMAGE, extensions );
        bool pngListedAsImage = false;
        for ( size_t i=0; i<extensions.size(); ++i )
            pngListedAsImage = pngListedAsImage || "png" == extensions[ i ];
        ASSERT_FALSE( pngListedAsImage );
        CORE::CString::StringVector owners;
        ASSERT_TRUE( ASSETKIND_METADATA == kinds.GetKindOfPath( "a.ktkinds.ktkindsside" ) && kinds.GetOwnerPaths( "a.ktkindsside", owners ) && 1 == owners.size() && "a.ktkinds" == owners[ 0 ] );

        // a second registration under the same name is refused and adds nothing
        ASSERT_FALSE( plugins.RegisterImporter( TAssetImporterPtr( GUCEF_NEW CKindsTestImporter( "kinds", "kinds.marker", 40 ) ) ) );
        ASSERT_TRUE( before + 3 == kinds.GetDeclarationCount() );

        ASSERT_TRUE( plugins.UnregisterImporter( "kinds" ) );
        ASSERT_TRUE( before == kinds.GetDeclarationCount() );
        ASSERT_TRUE( ASSETKIND_OTHER == kinds.GetKindOfPath( "x/y.ktkinds" ) && ASSETKIND_IMAGE == kinds.GetKindOfPath( "a.png" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Kinds: the pack recognisers of the importers are asked with a listing, and the most sure one fills the profile" )
    try
    {
        CAssetPluginRegistry& plugins = CAssetConverterGlobal::Instance()->GetPluginRegistry();
        ASSERT_TRUE( plugins.RegisterImporter( TAssetImporterPtr( GUCEF_NEW CKindsTestImporter( "weak", "common.marker", 20 ) ) ) );
        ASSERT_TRUE( plugins.RegisterImporter( TAssetImporterPtr( GUCEF_NEW CKindsTestImporter( "strong", "common.marker", 80 ) ) ) );

        CAssetPackListing listing;
        listing.AddFile( "Readme.txt" );
        CAssetPackProfile profile;
        ASSERT_FALSE( plugins.RecognisePack( listing, profile ) || profile.IsRecognised() );
        listing.AddFile( "Common.Marker" );
        ASSERT_TRUE( plugins.RecognisePack( listing, profile ) );
        ASSERT_TRUE( "strong-pack" == profile.packType && 80 == profile.confidence && "strong" == profile.recognisedBy );

        ASSERT_TRUE( plugins.UnregisterImporter( "strong" ) );
        ASSERT_TRUE( plugins.RecognisePack( listing, profile ) && "weak-pack" == profile.packType );
        ASSERT_TRUE( plugins.UnregisterImporter( "weak" ) );
        ASSERT_FALSE( plugins.RecognisePack( listing, profile ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Kinds: a pack listing finds files in any letter case, reads their text, ranks candidates and lists folders; a profile knows its noise" )
    try
    {
        CAssetPackListing listing;
        listing.AddFile( "Assets/Scenes/Level1.unity" );
        listing.AddFile( "Assets/Scenes/DemoScene.unity" );
        listing.AddFile( "Assets/Deep/Er/Sample.unity" );
        listing.AddFile( "Assets/Models/Rock.fbx" );
        listing.AddFile( "Assets/Models/Rock.fbx.meta" );
        listing.AddFile( "ProjectSettings/ProjectVersion.txt", "m_EditorVersion: 2022.3.10f1\n" );
        ASSERT_TRUE( 6 == listing.GetFileCount() && listing.HasFile( "assets/models/ROCK.fbx" ) );
        CORE::CString listed, text;
        ASSERT_TRUE( listing.FindFile( "projectsettings/projectversion.txt", listed ) && "ProjectSettings/ProjectVersion.txt" == listed );
        ASSERT_TRUE( listing.ReadText( "ProjectSettings/ProjectVersion.txt", text, 4096 ) && "m_EditorVersion: 2022.3.10f1\n" == text );
        ASSERT_TRUE( listing.ReadText( "ProjectSettings/ProjectVersion.txt", text, 4 ) && "m_Ed" == text );
        ASSERT_FALSE( listing.ReadText( "Assets/Models/Rock.fbx", text ) || listing.ReadText( "none.txt", text ) );

        CORE::CString::StringVector found;
        ASSERT_TRUE( 3 == listing.FindFiles( "*.UNITY", found ) );
        CORE::CString::StringVector keywords, ranked;
        keywords.push_back( "demo" );
        keywords.push_back( "sample" );
        ASSERT_TRUE( 2 == listing.FindFilesRanked( "*.unity", keywords, ranked, 2 ) && "Assets/Scenes/DemoScene.unity" == ranked[ 0 ] && "Assets/Deep/Er/Sample.unity" == ranked[ 1 ] );
        CORE::CString::StringVector folders;
        listing.GetSubFolders( "Assets", folders );
        ASSERT_TRUE( 3 == folders.size() && "Assets/Deep" == folders[ 0 ] && "Assets/Models" == folders[ 1 ] && "Assets/Scenes" == folders[ 2 ] );
        folders.clear();
        listing.GetSubFolders( CORE::CString::Empty, folders );
        ASSERT_TRUE( 2 == folders.size() && "Assets" == folders[ 0 ] && "ProjectSettings" == folders[ 1 ] );

        CAssetPackProfile profile;
        ASSERT_FALSE( profile.IsRecognised() );
        profile.packType = "x";
        profile.noisePatterns.push_back( "*.meta" );
        profile.noisePatterns.push_back( "Library/*" );
        ASSERT_TRUE( "a b" == CAssetPackListing::TrimText( " \r\na b\t\r\n" ) && CAssetPackListing::TrimText( " \r " ).IsNULLOrEmpty() );
        ASSERT_TRUE( profile.IsRecognised() && profile.IsNoise( "Assets/Rock.fbx.META" ) && profile.IsNoise( "library/Cache/a.bin" ) && !profile.IsNoise( "Assets/Rock.fbx" ) );
        profile.Clear();
        ASSERT_FALSE( profile.IsRecognised() || profile.IsNoise( "a.meta" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
PerformAssetConverterProcessingTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "AssetConverterLib.Processing" );

    TestAssetKinds();

    GUCEF_TESTFW_TESTCASE( "Normalisation: a left-handed Y up source is mirrored into glTF conventions, DirectX normal maps get a green flip" )
    try
    {
        CAssetScene scene;
        scene.coordinateSystem = MATH::CCoordinateSystem::Unity();
        scene.normalMapConvention = NORMALMAPCONVENTION_DIRECTX;
        const UInt32 node = scene.AddNode( "Node" );
        scene.nodes[ node ].transform.position = MATH::TFloat32Transform3D::TPositionVector( 1.0f, 2.0f, 3.0f );
        scene.nodes[ node ].transform.scale = MATH::TFloat32Vector3D( 1.0f, 2.0f, 3.0f );
        const Float32 s = (Float32) sqrt( 0.5 );
        scene.nodes[ node ].transform.rotation = MATH::TFloat32Quaternion::FromXYZW( 0.0f, s, 0.0f, s );
        CAssetCollider box;
        box.shape = COLLIDERSHAPE_BOX;
        box.halfExtents[ 0 ] = 1.0f; box.halfExtents[ 1 ] = 2.0f; box.halfExtents[ 2 ] = 3.0f;
        box.transform.position = MATH::TFloat32Transform3D::TPositionVector( 0.0f, 0.0f, 1.0f );
        scene.nodes[ node ].colliders.push_back( box );

        CAssetMesh mesh;
        CAssetSubMesh subMesh;
        const Float32 positions[] = { 1,2,3,  4,5,6,  7,8,9 };
        for ( UInt32 i=0; i<9; ++i )
            subMesh.positions.push_back( positions[ i ] );
        for ( UInt32 v=0; v<3; ++v )
        {
            subMesh.normals.push_back( 0.0f ); subMesh.normals.push_back( 0.0f ); subMesh.normals.push_back( 1.0f );
            subMesh.tangents.push_back( 1.0f ); subMesh.tangents.push_back( 0.0f ); subMesh.tangents.push_back( 0.0f ); subMesh.tangents.push_back( 1.0f );
        }
        subMesh.indices.push_back( 0 ); subMesh.indices.push_back( 1 ); subMesh.indices.push_back( 2 );
        mesh.subMeshes.push_back( subMesh );
        scene.meshes.push_back( mesh );
        scene.nodes[ node ].mesh = 0;

        // one normal map used by two materials is flipped once
        CAssetTexture normalMap;
        normalMap.name = "normal";
        normalMap.usage = TEXTUREUSAGE_NORMAL;
        scene.textures.push_back( normalMap );
        CAssetMaterial material;
        material.normalTexture = CAssetTextureReference( 0 );
        scene.materials.push_back( material );
        scene.materials.push_back( material );

        CConversionReport report;
        report.SelectAsset( "unity.stub" );
        ASSERT_TRUE( CAssetNormaliser::Normalise( scene, report ) );
        const CAssetSubMesh& converted = scene.meshes[ 0 ].subMeshes[ 0 ];
        ASSERT_TRUE( Near( 1.0f, converted.positions[ 0 ] ) && Near( 2.0f, converted.positions[ 1 ] ) && Near( -3.0f, converted.positions[ 2 ] ) );
        ASSERT_TRUE( Near( -9.0f, converted.positions[ 8 ] ) );
        ASSERT_TRUE( Near( -1.0f, converted.normals[ 2 ] ) && Near( 1.0f, converted.tangents[ 0 ] ) && Near( -1.0f, converted.tangents[ 3 ] ) );
        ASSERT_TRUE( 0 == converted.indices[ 0 ] && 2 == converted.indices[ 1 ] && 1 == converted.indices[ 2 ] );
        const MATH::TFloat32Transform3D& transform = scene.nodes[ node ].transform;
        ASSERT_TRUE( Near( 1.0f, transform.position.x ) && Near( 2.0f, transform.position.y ) && Near( -3.0f, transform.position.z ) );
        ASSERT_TRUE( Near( 1.0f, transform.scale.x ) && Near( 2.0f, transform.scale.y ) && Near( 3.0f, transform.scale.z ) );
        ASSERT_TRUE( SameRotation( MATH::TFloat32Quaternion::FromXYZW( 0.0f, -s, 0.0f, s ), transform.rotation ) );
        const CAssetCollider& collider = scene.nodes[ node ].colliders[ 0 ];
        ASSERT_TRUE( Near( -1.0f, collider.transform.position.z ) && Near( 2.0f, collider.halfExtents[ 1 ] ) && Near( 3.0f, collider.halfExtents[ 2 ] ) );

        ASSERT_TRUE( 1 == scene.textures[ 0 ].operations.size() && TEXTUREOPERATION_GREENFLIP == scene.textures[ 0 ].operations[ 0 ].type );
        ASSERT_TRUE( NORMALMAPCONVENTION_OPENGL == scene.normalMapConvention && scene.coordinateSystem == MATH::CCoordinateSystem::GucefDefault() );
        ASSERT_TRUE( scene.normalisation.GetAttributeValueOrChildValueByName( "mirrored" ).AsBool( false ) );
        ASSERT_TRUE( 1 == scene.normalisation.GetAttributeValueOrChildValueByName( "normalMapsFlipped" ).AsUInt32() );
        ASSERT_TRUE( MATH::CCoordinateSystem::Unity().ToString() == scene.normalisation.GetAttributeValueOrChildValueByName( "from" ).AsString() );
        ASSERT_TRUE( 2 == report.GetItemCount( REPORTITEM_INFO ) );

        // normalised again: nothing changes
        ASSERT_TRUE( CAssetNormaliser::Normalise( scene, report ) );
        ASSERT_TRUE( Near( -3.0f, scene.meshes[ 0 ].subMeshes[ 0 ].positions[ 2 ] ) && 1 == scene.textures[ 0 ].operations.size() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Normalisation: centimetres and Z up become metres and Y up, skins stay bound; settings override the system of the importer" )
    try
    {
        const MATH::CCoordinateSystem unreal = MATH::CCoordinateSystem::Unreal();
        const MATH::CCoordinateSystem target = MATH::CCoordinateSystem::GucefDefault();
        const MATH::TFloat32Matrix4D conversion = unreal.GetConversionMatrix4To< Float32 >( target );

        CAssetScene scene;
        scene.coordinateSystem = unreal;
        const UInt32 root = scene.AddNode( "Root" );
        const UInt32 joint = scene.AddNode( "Joint", (Int32) root );
        scene.nodes[ root ].transform.position = MATH::TFloat32Transform3D::TPositionVector( 100.0f, 0.0f, 50.0f );
        scene.nodes[ joint ].transform.position = MATH::TFloat32Transform3D::TPositionVector( 0.0f, 30.0f, 0.0f );
        scene.nodes[ joint ].transform.rotation = MATH::TFloat32Quaternion::FromXYZW( 0.0f, 0.0f, (Float32) sin( 0.3 ), (Float32) cos( 0.3 ) );
        CAssetSkin skin;
        skin.joints.push_back( joint );
        skin.inverseBindMatrices.push_back( scene.GetWorldMatrix( joint ).Inverse() );
        scene.skins.push_back( skin );
        CAssetMesh mesh;
        CAssetSubMesh subMesh;
        subMesh.positions.push_back( 100.0f ); subMesh.positions.push_back( 200.0f ); subMesh.positions.push_back( 300.0f );
        mesh.subMeshes.push_back( subMesh );
        scene.meshes.push_back( mesh );
        CAssetAnimation animation;
        CAssetAnimationChannel channel;
        channel.targetNode = joint;
        channel.times.push_back( 0.0f );
        channel.values.push_back( 100.0f ); channel.values.push_back( 200.0f ); channel.values.push_back( 300.0f );
        animation.channels.push_back( channel );
        scene.animations.push_back( animation );

        CConversionReport report;
        ASSERT_TRUE( CAssetNormaliser::Normalise( scene, report ) );
        const MATH::TFloat32Point3D expected = conversion.TransformPoint( MATH::TFloat32Point3D( 100.0f, 200.0f, 300.0f ) );
        const CAssetSubMesh& converted = scene.meshes[ 0 ].subMeshes[ 0 ];
        ASSERT_TRUE( Near( expected.x, converted.positions[ 0 ] ) && Near( expected.y, converted.positions[ 1 ] ) && Near( expected.z, converted.positions[ 2 ] ) );
        ASSERT_TRUE( Near( 3.0f, converted.positions[ 1 ] ) );    // 300 cm up is 3 m along +Y
        const CAssetAnimationChannel& keys = scene.animations[ 0 ].channels[ 0 ];
        ASSERT_TRUE( Near( expected.x, keys.values[ 0 ] ) && Near( expected.y, keys.values[ 1 ] ) && Near( expected.z, keys.values[ 2 ] ) );
        ASSERT_TRUE( Near( 0.01f, scene.normalisation.GetAttributeValueOrChildValueByName( "scale" ).AsFloat32() ) );

        // the joint still sits where its inverse bind matrix says: the product is the identity
        const MATH::TFloat32Matrix4D bound = scene.skins[ 0 ].inverseBindMatrices[ 0 ] * scene.GetWorldMatrix( joint );
        bool identity = true;
        for ( UInt32 r=0; r<4; ++r )
            for ( UInt32 c=0; c<4; ++c )
                identity = identity && Near( r == c ? 1.0f : 0.0f, bound( r, c ), 1e-3f );
        ASSERT_TRUE( identity );
        const MATH::TFloat32Point3D jointWorld = conversion.TransformPoint( MATH::TFloat32Point3D( 100.0f, 30.0f, 50.0f ) );
        const MATH::TFloat32Matrix4D world = scene.GetWorldMatrix( joint );
        ASSERT_TRUE( Near( jointWorld.x, world( 0, 3 ), 1e-3f ) && Near( jointWorld.y, world( 1, 3 ), 1e-3f ) && Near( jointWorld.z, world( 2, 3 ), 1e-3f ) );

        // the units of the settings over those of the importer
        CAssetScene scaled;
        scaled.settings.SetAttribute( "unitsPerMetre", (UInt32) 100 );
        scaled.meshes.push_back( mesh );
        ASSERT_TRUE( CAssetNormaliser::Normalise( scaled, report ) );
        ASSERT_TRUE( Near( 1.0f, scaled.meshes[ 0 ].subMeshes[ 0 ].positions[ 0 ] ) && Near( 3.0f, scaled.meshes[ 0 ].subMeshes[ 0 ].positions[ 2 ] ) );

        // a whole system from the settings: Z up becomes Y up
        CAssetScene blender;
        blender.settings.SetAttribute( "coordinateSystem", MATH::CCoordinateSystem::Blender().ToString() );
        blender.meshes.push_back( mesh );
        ASSERT_TRUE( CAssetNormaliser::Normalise( blender, report ) );
        ASSERT_TRUE( Near( 300.0f, blender.meshes[ 0 ].subMeshes[ 0 ].positions[ 1 ] ) );

        // a setting that is no coordinate system is reported, and the system of the importer stays
        CAssetScene garbage;
        garbage.settings.SetAttribute( "coordinateSystem", "sideways" );
        garbage.meshes.push_back( mesh );
        const UInt32 warnings = report.GetItemCount( REPORTITEM_WARNING );
        ASSERT_TRUE( CAssetNormaliser::Normalise( garbage, report ) && warnings + 1 == report.GetItemCount( REPORTITEM_WARNING ) );
        ASSERT_TRUE( Near( 100.0f, garbage.meshes[ 0 ].subMeshes[ 0 ].positions[ 0 ] ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Normalisation: an invalid coordinate system of the importer is refused with an error, the scene stays as it is" )
    try
    {
        // the validity assert of the constructor is off in this test application (GUCEF_MATH_NO_COORDINATESYSTEM_VALIDITY_ASSERT)
        const MATH::CCoordinateSystem invalid( MATH::SIGNEDAXIS_POSITIVE_X, MATH::SIGNEDAXIS_POSITIVE_X, MATH::SIGNEDAXIS_NEGATIVE_Z, 1.0 );
        ASSERT_FALSE( invalid.IsValid() );

        CAssetMesh mesh;
        CAssetSubMesh subMesh;
        subMesh.positions.push_back( 1.0f ); subMesh.positions.push_back( 2.0f ); subMesh.positions.push_back( 3.0f );
        mesh.subMeshes.push_back( subMesh );

        CAssetScene scene;
        scene.coordinateSystem = invalid;
        scene.meshes.push_back( mesh );
        CConversionReport report;
        ASSERT_FALSE( CAssetNormaliser::Normalise( scene, report ) );
        ASSERT_TRUE( 1 == report.GetItemCount( REPORTITEM_ERROR ) );
        ASSERT_TRUE( scene.coordinateSystem == invalid );
        ASSERT_TRUE( Near( 1.0f, scene.meshes[ 0 ].subMeshes[ 0 ].positions[ 0 ] ) && Near( 3.0f, scene.meshes[ 0 ].subMeshes[ 0 ].positions[ 2 ] ) );

        // units of the settings do not make the axes valid
        CAssetScene scaled;
        scaled.coordinateSystem = invalid;
        scaled.settings.SetAttribute( "unitsPerMetre", (UInt32) 100 );
        scaled.meshes.push_back( mesh );
        ASSERT_FALSE( CAssetNormaliser::Normalise( scaled, report ) );
        ASSERT_TRUE( 2 == report.GetItemCount( REPORTITEM_ERROR ) );

        // a whole system from the settings takes the place of the invalid one
        CAssetScene overridden;
        overridden.coordinateSystem = invalid;
        overridden.settings.SetAttribute( "coordinateSystem", MATH::CCoordinateSystem::Blender().ToString() );
        overridden.meshes.push_back( mesh );
        ASSERT_TRUE( CAssetNormaliser::Normalise( overridden, report ) );
        ASSERT_TRUE( 2 == report.GetItemCount( REPORTITEM_ERROR ) );
        ASSERT_TRUE( overridden.coordinateSystem == MATH::CCoordinateSystem::GucefDefault() );
        ASSERT_TRUE( Near( 3.0f, overridden.meshes[ 0 ].subMeshes[ 0 ].positions[ 1 ] ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "NameHints: Unreal collision prefixes become colliders of their mesh, fitted to the vertices" )
    try
    {
        CAssetScene scene;
        const UInt32 root = scene.AddNode( "Root" );
        const UInt32 rock = scene.AddNode( "Rock", (Int32) root );
        scene.nodes[ rock ].transform.position = MATH::TFloat32Transform3D::TPositionVector( 5.0f, 0.0f, 0.0f );
        AddBoxMesh( scene, rock, 2.0f, 4.0f, 6.0f );
        const char* const names[] = { "UBX_Rock", "USP_Rock_01", "UCP_Rock", "UCX_Rock" };
        for ( UInt32 i=0; i<4; ++i )
        {
            const UInt32 node = scene.AddNode( names[ i ], (Int32) root );
            scene.nodes[ node ].transform.position = MATH::TFloat32Transform3D::TPositionVector( 5.0f, 0.0f, 0.0f );
            if ( 2 == i )
                AddBoxMesh( scene, node, 2.0f, 2.0f, 10.0f );
            else
                AddBoxMesh( scene, node, 2.0f, 4.0f, 6.0f );
        }
        const UInt32 orphan = scene.AddNode( "UBX_Nothing", (Int32) root );
        AddBoxMesh( scene, orphan, 1.0f, 1.0f, 1.0f );

        CNameHintsPostProcessor processor;
        CORE::CDataNode settings( "settings", GUCEF_DATATYPE_OBJECT );
        settings.SetAttribute( "set", "unreal" );
        CConversionReport report;
        report.SelectAsset( "rock.stub" );
        ASSERT_TRUE( processor.Process( scene, settings, report ) );

        // the collision nodes are gone, the one without a mesh to belong to stays
        ASSERT_TRUE( 3 == scene.nodes.size() && scene.FindNode( "UBX_Nothing" ) >= 0 && scene.FindNode( "UCX_Rock" ) < 0 );
        ASSERT_TRUE( 1 == CountItems( report, REPORTITEM_WARNING, "NameHints" ) );
        const CAssetNode& rockNode = scene.nodes[ scene.FindNode( "Rock" ) ];
        ASSERT_TRUE( 4 == rockNode.colliders.size() );

        const CAssetCollider* fittedBox = FindCollider( rockNode, COLLIDERSHAPE_BOX );
        ASSERT_TRUE( GUCEF_NULL != fittedBox );
        if ( GUCEF_NULL != fittedBox )
        {
            Float32 extents[ 3 ] = { fittedBox->halfExtents[ 0 ], fittedBox->halfExtents[ 1 ], fittedBox->halfExtents[ 2 ] };
            std::sort( extents, extents + 3 );
            ASSERT_TRUE( Near( 1.0f, extents[ 0 ], 1e-3f ) && Near( 2.0f, extents[ 1 ], 1e-3f ) && Near( 3.0f, extents[ 2 ], 1e-3f ) );
            ASSERT_TRUE( Near( 0.0f, fittedBox->transform.position.x, 1e-3f ) );   // in the frame of Rock
        }
        const CAssetCollider* sphere = FindCollider( rockNode, COLLIDERSHAPE_SPHERE );
        ASSERT_TRUE( GUCEF_NULL != sphere && Near( (Float32) sqrt( 14.0 ), sphere->radius, 1e-3f ) );
        const CAssetCollider* capsule = FindCollider( rockNode, COLLIDERSHAPE_CAPSULE );
        ASSERT_TRUE( GUCEF_NULL != capsule );
        if ( GUCEF_NULL != capsule )
        {
            ASSERT_TRUE( Near( (Float32) sqrt( 2.0 ), capsule->radius, 1e-3f ) && Near( 2.0f * ( 5.0f + (Float32) sqrt( 2.0 ) ), capsule->height, 1e-2f ) );
            const MATH::TFloat32Vector3D axis = capsule->transform.rotation.Rotate( MATH::TFloat32Vector3D( 0.0f, 1.0f, 0.0f ) );
            ASSERT_TRUE( Near( 1.0f, (Float32) fabs( axis.z ), 1e-3f ) );
        }
        const CAssetCollider* convex = FindCollider( rockNode, COLLIDERSHAPE_CONVEX );
        ASSERT_TRUE( GUCEF_NULL != convex && convex->meshIndex >= 0 && (size_t) convex->meshIndex < scene.meshes.size() );
        if ( GUCEF_NULL != convex && convex->meshIndex >= 0 )
        {
            ASSERT_TRUE( 8 == scene.meshes[ convex->meshIndex ].GetVertexCount() && 12 == scene.meshes[ convex->meshIndex ].GetTriangleCount() );
        }
        // the meshes of the collision nodes went, except the hull of the convex collider
        ASSERT_TRUE( 3 == scene.meshes.size() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "NameHints: LOD suffixes make LOD groups; Godot suffixes make colliders, leave nodes out and loop clips; auto picks the set" )
    try
    {
        CAssetScene scene;
        const UInt32 root = scene.AddNode( "Root" );
        const UInt32 tree0 = scene.AddNode( "Tree_LOD0", (Int32) root );
        const UInt32 tree1 = scene.AddNode( "Tree_LOD1", (Int32) root );
        const UInt32 tree = scene.AddNode( "Tree", (Int32) root );
        const UInt32 bush1 = scene.AddNode( "Bush_LOD1" );
        const UInt32 bush0 = scene.AddNode( "Bush_LOD0" );
        const UInt32 grass = scene.AddNode( "Grass", (Int32) root );
        const UInt32 grass0 = scene.AddNode( "Grass_lod0", (Int32) grass );
        const UInt32 grass1 = scene.AddNode( "Grass_lod1", (Int32) grass );
        const UInt32 wall = scene.AddNode( "Wall-col", (Int32) root );
        AddBoxMesh( scene, wall, 1.0f, 1.0f, 1.0f );
        const UInt32 fence = scene.AddNode( "Fence-convcolonly", (Int32) root );
        AddBoxMesh( scene, fence, 1.0f, 1.0f, 1.0f );
        const UInt32 junk = scene.AddNode( "Junk-noimp", (Int32) root );
        scene.AddNode( "JunkChild", (Int32) junk );
        const UInt32 box = scene.AddNode( "UBX_Wall", (Int32) root );
        AddBoxMesh( scene, box, 1.0f, 1.0f, 1.0f );
        CAssetAnimation animation;
        animation.name = "Run-loop";
        scene.animations.push_back( animation );
        animation.name = "Walk-cycle";
        scene.animations.push_back( animation );
        animation.name = "Idle";
        scene.animations.push_back( animation );
        ( void ) tree0; ( void ) tree1; ( void ) bush1; ( void ) bush0; ( void ) grass0; ( void ) grass1;

        CNameHintsPostProcessor processor;
        CConversionReport report;
        report.SelectAsset( "godot.stub" );
        scene.settings.SetAttribute( "nameHints", "godot" );
        CORE::CDataNode settings( "settings", GUCEF_DATATYPE_OBJECT );
        ASSERT_TRUE( processor.Process( scene, settings, report ) );

        // LOD groups, held by the sibling of the name, by the parent that holds only them, or by a new node
        ASSERT_TRUE( 3 == scene.lodGroups.size() );
        const CAssetNode& treeNode = scene.nodes[ scene.FindNode( "Tree" ) ];
        ASSERT_TRUE( treeNode.lodGroup >= 0 );
        if ( treeNode.lodGroup >= 0 )
        {
            ASSERT_TRUE( 2 == scene.lodGroups[ treeNode.lodGroup ].levels.size() );
            ASSERT_TRUE( scene.FindNode( "Tree_LOD0" ) == (Int32) scene.lodGroups[ treeNode.lodGroup ].levels[ 0 ].node );
            ASSERT_TRUE( scene.lodGroups[ treeNode.lodGroup ].levels[ 0 ].screenRelativeHeight < 0.0f );
        }
        const CAssetNode& grassNode = scene.nodes[ scene.FindNode( "Grass" ) ];
        ASSERT_TRUE( grassNode.lodGroup >= 0 && 2 == scene.lodGroups[ grassNode.lodGroup < 0 ? 0 : grassNode.lodGroup ].levels.size() );
        const Int32 bushHolder = scene.FindNode( "Bush" );
        ASSERT_TRUE( bushHolder >= 0 && scene.nodes[ bushHolder ].lodGroup >= 0 );
        if ( bushHolder >= 0 && scene.nodes[ bushHolder ].lodGroup >= 0 )
        {
            const CAssetLodGroup& bushGroup = scene.lodGroups[ scene.nodes[ bushHolder ].lodGroup ];
            ASSERT_TRUE( scene.FindNode( "Bush_LOD0" ) == (Int32) bushGroup.levels[ 0 ].node && scene.FindNode( "Bush_LOD1" ) == (Int32) bushGroup.levels[ 1 ].node );
            ASSERT_TRUE( bushHolder == scene.nodes[ bushGroup.levels[ 0 ].node ].parent );
        }
        ( void ) tree;

        // Godot: colliders, left out nodes, loops; the Unreal prefixes are not of this set
        const Int32 wallNode = scene.FindNode( "Wall" );
        ASSERT_TRUE( wallNode >= 0 && 1 == scene.nodes[ wallNode ].colliders.size() && COLLIDERSHAPE_TRIMESH == scene.nodes[ wallNode ].colliders[ 0 ].shape && scene.nodes[ wallNode ].mesh >= 0 );
        const Int32 fenceNode = scene.FindNode( "Fence" );
        ASSERT_TRUE( fenceNode >= 0 && 1 == scene.nodes[ fenceNode ].colliders.size() && COLLIDERSHAPE_CONVEX == scene.nodes[ fenceNode ].colliders[ 0 ].shape );
        ASSERT_TRUE( fenceNode >= 0 && scene.nodes[ fenceNode ].mesh < 0 && scene.nodes[ fenceNode ].colliders[ 0 ].meshIndex >= 0 );
        ASSERT_TRUE( scene.FindNode( "Junk-noimp" ) < 0 && scene.FindNode( "JunkChild" ) < 0 && 1 == CountItems( report, REPORTITEM_SKIPPED, "NameHints" ) );
        ASSERT_TRUE( scene.FindNode( "UBX_Wall" ) >= 0 );
        ASSERT_TRUE( "Run" == scene.animations[ 0 ].name && scene.animations[ 0 ].loop && "Walk" == scene.animations[ 1 ].name && scene.animations[ 1 ].loop && !scene.animations[ 2 ].loop );

        // what "auto" stands for
        CAssetScene probe;
        ASSERT_TRUE( "both" == CNameHintsPostProcessor::ResolveSet( "auto", probe ) );
        probe.provenance.sourceEngine = "Unreal";
        ASSERT_TRUE( "unreal" == CNameHintsPostProcessor::ResolveSet( "", probe ) );
        probe.provenance.sourceEngine = "unity";
        ASSERT_TRUE( "none" == CNameHintsPostProcessor::ResolveSet( "auto", probe ) );
        probe.provenance.sourceEngine.Clear();
        probe.provenance.importerName = "godot";
        ASSERT_TRUE( "godot" == CNameHintsPostProcessor::ResolveSet( "AUTO", probe ) );
        ASSERT_TRUE( "none" == CNameHintsPostProcessor::ResolveSet( "none", probe ) );

        // the set "none" changes nothing
        CAssetScene untouched;
        untouched.AddNode( "UBX_A" );
        untouched.AddNode( "A" );
        AddBoxMesh( untouched, 0, 1.0f, 1.0f, 1.0f );
        settings.SetAttribute( "set", "none" );
        ASSERT_TRUE( processor.Process( untouched, settings, report ) && 2 == untouched.nodes.size() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GenerateTangents: normal mapped submeshes without tangents get them from the UVs, with the handedness in w" )
    try
    {
        CAssetScene scene;
        CAssetMaterial normalMapped;
        normalMapped.normalTexture = CAssetTextureReference( 0 );
        scene.materials.push_back( normalMapped );
        scene.materials.push_back( CAssetMaterial() );
        CAssetMesh mesh;
        CAssetSubMesh quad;
        const Float32 positions[] = { 0,0,0,  1,0,0,  1,1,0,  0,1,0 };
        const Float32 uvs[] = { 0,0,  1,0,  1,1,  0,1 };
        for ( UInt32 i=0; i<12; ++i )
            quad.positions.push_back( positions[ i ] );
        for ( UInt32 i=0; i<8; ++i )
            quad.uvSets[ 0 ].push_back( uvs[ i ] );
        for ( UInt32 v=0; v<4; ++v )
        {
            quad.normals.push_back( 0.0f ); quad.normals.push_back( 0.0f ); quad.normals.push_back( 1.0f );
        }
        const UInt32 indices[] = { 0,1,2, 0,2,3 };
        for ( UInt32 i=0; i<6; ++i )
            quad.indices.push_back( indices[ i ] );
        quad.materialIndex = 0;
        mesh.subMeshes.push_back( quad );
        CAssetSubMesh mirrored = quad;
        for ( UInt32 v=0; v<4; ++v )
            mirrored.uvSets[ 0 ][ v*2 ] = 1.0f - mirrored.uvSets[ 0 ][ v*2 ];
        mesh.subMeshes.push_back( mirrored );
        CAssetSubMesh plain = quad;
        plain.materialIndex = 1;
        mesh.subMeshes.push_back( plain );
        CAssetSubMesh noNormals = quad;
        noNormals.normals.clear();
        mesh.subMeshes.push_back( noNormals );
        scene.meshes.push_back( mesh );

        CGenerateTangentsPostProcessor processor;
        CConversionReport report;
        report.SelectAsset( "quad.stub" );
        ASSERT_TRUE( processor.Process( scene, CORE::CDataNode(), report ) );
        const CAssetSubMesh& first = scene.meshes[ 0 ].subMeshes[ 0 ];
        ASSERT_TRUE( first.HasTangents() );
        for ( UInt32 v=0; v<4; ++v )
            ASSERT_TRUE( Near( 1.0f, first.tangents[ v*4 ] ) && Near( 0.0f, first.tangents[ v*4+1 ] ) && Near( 0.0f, first.tangents[ v*4+2 ] ) && Near( 1.0f, first.tangents[ v*4+3 ] ) );
        const CAssetSubMesh& second = scene.meshes[ 0 ].subMeshes[ 1 ];
        ASSERT_TRUE( second.HasTangents() && Near( -1.0f, second.tangents[ 0 ] ) && Near( -1.0f, second.tangents[ 3 ] ) );
        ASSERT_FALSE( scene.meshes[ 0 ].subMeshes[ 2 ].HasTangents() );
        ASSERT_FALSE( scene.meshes[ 0 ].subMeshes[ 3 ].HasTangents() );
        ASSERT_TRUE( 1 == CountItems( report, REPORTITEM_APPROXIMATION, "GenerateTangents" ) && 1 == CountItems( report, REPORTITEM_WARNING, "GenerateTangents" ) );

        // tangents that are there stay
        scene.meshes[ 0 ].subMeshes[ 0 ].tangents[ 0 ] = 0.5f;
        CConversionReport again;
        ASSERT_TRUE( processor.Process( scene, CORE::CDataNode(), again ) );
        ASSERT_TRUE( 0.5f == scene.meshes[ 0 ].subMeshes[ 0 ].tangents[ 0 ] );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "PruneEmptyNodes and MergeIdenticalMaterials: empty leaves go, materials that look the same are merged" )
    try
    {
        CAssetScene scene;
        const UInt32 root = scene.AddNode( "Root" );
        const UInt32 empty = scene.AddNode( "Empty", (Int32) root );
        scene.AddNode( "EmptyChild", (Int32) empty );
        const UInt32 joint = scene.AddNode( "Joint", (Int32) root );
        const UInt32 animated = scene.AddNode( "Animated", (Int32) root );
        const UInt32 holder = scene.AddNode( "Holder", (Int32) root );
        const UInt32 tagged = scene.AddNode( "Tagged", (Int32) root );
        scene.nodes[ tagged ].extras.SetAttribute( "tag", "keep" );
        AddBoxMesh( scene, holder, 1.0f, 1.0f, 1.0f );
        CAssetSkin skin;
        skin.joints.push_back( joint );
        skin.inverseBindMatrices.push_back( MATH::TFloat32Matrix4D::Identity() );
        scene.skins.push_back( skin );
        CAssetAnimation animation;
        CAssetAnimationChannel channel;
        channel.targetNode = animated;
        animation.channels.push_back( channel );
        scene.animations.push_back( animation );

        CPruneEmptyNodesPostProcessor prune;
        CConversionReport report;
        report.SelectAsset( "prune.stub" );
        ASSERT_TRUE( prune.Process( scene, CORE::CDataNode(), report ) );
        ASSERT_TRUE( 5 == scene.nodes.size() && scene.FindNode( "Empty" ) < 0 && scene.FindNode( "EmptyChild" ) < 0 );
        ASSERT_TRUE( scene.FindNode( "Joint" ) >= 0 && scene.FindNode( "Animated" ) >= 0 && scene.FindNode( "Holder" ) >= 0 && scene.FindNode( "Tagged" ) >= 0 );
        ASSERT_TRUE( scene.IsJoint( (UInt32) scene.FindNode( "Joint" ) ) && scene.IsAnimationTarget( (UInt32) scene.FindNode( "Animated" ) ) );
        ASSERT_TRUE( 1 == CountItems( report, REPORTITEM_INFO, "PruneEmptyNodes" ) );

        CAssetScene materials;
        const char* const names[] = { "Red", "Blue", "RedToo", "Unused" };
        const Float32 colours[][ 3 ] = { { 1, 0, 0 }, { 0, 0, 1 }, { 1, 0, 0 }, { 0, 1, 0 } };
        for ( UInt32 i=0; i<4; ++i )
        {
            CAssetMaterial material;
            material.name = names[ i ];
            for ( UInt32 c=0; c<3; ++c )
                material.baseColorFactor[ c ] = colours[ i ][ c ];
            material.source.shaderName = names[ i ];
            materials.materials.push_back( material );
        }
        CAssetMesh mesh;
        for ( Int32 i=0; i<3; ++i )
        {
            CAssetSubMesh subMesh;
            subMesh.materialIndex = i;
            mesh.subMeshes.push_back( subMesh );
        }
        materials.meshes.push_back( mesh );
        ASSERT_TRUE( materials.materials[ 0 ].HasSameAppearance( materials.materials[ 2 ] ) && !materials.materials[ 0 ].HasSameAppearance( materials.materials[ 1 ] ) );

        CMergeIdenticalMaterialsPostProcessor merge;
        ASSERT_TRUE( merge.Process( materials, CORE::CDataNode(), report ) );
        ASSERT_TRUE( 3 == materials.materials.size() && "Red" == materials.materials[ 0 ].name && "Blue" == materials.materials[ 1 ].name && "Unused" == materials.materials[ 2 ].name );
        ASSERT_TRUE( 0 == materials.meshes[ 0 ].subMeshes[ 0 ].materialIndex && 1 == materials.meshes[ 0 ].subMeshes[ 1 ].materialIndex && 0 == materials.meshes[ 0 ].subMeshes[ 2 ].materialIndex );
        ASSERT_TRUE( 1 == CountItems( report, REPORTITEM_INFO, "MergeIdenticalMaterials" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Validators: texture size, missing textures, bone limit, triangle budget and naming, with their severities" )
    try
    {
        CAssetPluginRegistry& plugins = CAssetConverterGlobal::Instance()->GetPluginRegistry();
        CAssetScene scene;
        scene.provenance.assetPath = "props/rock.stub";
        CAssetTexture big;
        big.name = "big";
        big.width = 4096; big.height = 4096;
        scene.textures.push_back( big );
        CAssetTexture odd;
        odd.name = "odd";
        odd.width = 300; odd.height = 200;
        scene.textures.push_back( odd );
        CAssetTexture lost;
        lost.name = "lost";
        lost.missing = true;
        lost.sources.push_back( "work/none.png" );
        scene.textures.push_back( lost );
        CAssetMaterial material;
        material.name = "M_Rock";
        material.emissiveTexture = CAssetTextureReference( 9 );
        scene.materials.push_back( material );

        TAssetValidatorPtr validator;
        CORE::CDataNode settings( "settings", GUCEF_DATATYPE_OBJECT );
        CConversionReport report;
        report.SelectAsset( scene.provenance.assetPath );

        ASSERT_TRUE( plugins.TryGetValidator( "TextureSize", validator ) );
        settings.SetAttribute( "maxSize", (UInt32) 2048 );
        settings.SetAttribute( "powerOfTwo", true );
        ASSERT_TRUE( validator->Validate( scene, settings, report ) );
        ASSERT_TRUE( 2 == CountItems( report, REPORTITEM_ERROR, "TextureSize" ) );
        settings.SetAttribute( "severity", "warning" );
        ASSERT_TRUE( validator->Validate( scene, settings, report ) && 2 == CountItems( report, REPORTITEM_WARNING, "TextureSize" ) );

        ASSERT_TRUE( plugins.TryGetValidator( "MissingTextures", validator ) );
        ASSERT_TRUE( validator->Validate( scene, CORE::CDataNode(), report ) && 2 == CountItems( report, REPORTITEM_ERROR, "MissingTextures" ) );

        CAssetMesh mesh;
        mesh.name = "Rock";
        CAssetSubMesh skinned;
        skinned.influencesPerVertex = 4;
        for ( UInt32 j=0; j<4; ++j )
        {
            skinned.joints.push_back( j );
            skinned.weights.push_back( 0.25f );
        }
        for ( UInt32 i=0; i<12; ++i )
            skinned.indices.push_back( i % 3 );
        mesh.subMeshes.push_back( skinned );
        scene.meshes.push_back( mesh );
        ASSERT_TRUE( 4 == skinned.GetUsedJointCount() );
        ASSERT_TRUE( plugins.TryGetValidator( "BoneLimit", validator ) );
        CORE::CDataNode bones( "settings", GUCEF_DATATYPE_OBJECT );
        ASSERT_TRUE( validator->Validate( scene, bones, report ) && 0 == CountItems( report, REPORTITEM_ERROR, "BoneLimit" ) );
        bones.SetAttribute( "maxBones", (UInt32) 3 );
        ASSERT_TRUE( validator->Validate( scene, bones, report ) && 1 == CountItems( report, REPORTITEM_ERROR, "BoneLimit" ) );

        // triangles per LOD level: the level nodes with what is below them, the rest counts for level 0
        const UInt32 level0 = scene.AddNode( "Level0" );
        const UInt32 level1 = scene.AddNode( "Level1" );
        const UInt32 other = scene.AddNode( "Other" );
        scene.nodes[ level0 ].mesh = 0;
        scene.nodes[ level1 ].mesh = 0;
        scene.nodes[ other ].mesh = 0;
        CAssetLodGroup group;
        group.AddLevel( level0, 0.5f );
        group.AddLevel( level1, 0.2f );
        scene.lodGroups.push_back( group );
        ASSERT_TRUE( plugins.TryGetValidator( "TriangleBudget", validator ) );
        CORE::CDataNode budget( "settings", GUCEF_DATATYPE_OBJECT );
        CORE::CDataNode* budgets = budget.AddChild( "budgets", GUCEF_DATATYPE_ARRAY );
        CORE::CDataNode* elsewhere = budgets->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT );
        elsewhere->SetAttribute( "match", "characters/**" );
        CORE::CDataNode::TVariantVector limits;
        limits.push_back( CORE::CVariant( (UInt32) 1 ) );
        elsewhere->SetValuesOfChildByName( "levels", limits );
        CORE::CDataNode* props = budgets->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT );
        props->SetAttribute( "match", "props/**" );
        limits.clear();
        limits.push_back( CORE::CVariant( (UInt32) 7 ) );
        limits.push_back( CORE::CVariant( (UInt32) 7 ) );
        props->SetValuesOfChildByName( "levels", limits );
        ASSERT_TRUE( validator->Validate( scene, budget, report ) );
        ASSERT_TRUE( 1 == CountItems( report, REPORTITEM_WARNING, "TriangleBudget" ) );

        ASSERT_TRUE( plugins.TryGetValidator( "Naming", validator ) );
        CORE::CDataNode naming( "settings", GUCEF_DATATYPE_OBJECT );
        CORE::CDataNode* patterns = naming.AddChild( "patterns", GUCEF_DATATYPE_OBJECT );
        patterns->SetAttribute( "mesh", "SM_*" );
        patterns->SetAttribute( "material", "M_*" );
        patterns->SetAttribute( "asset", "*.stub" );
        ASSERT_TRUE( validator->Validate( scene, naming, report ) && 1 == CountItems( report, REPORTITEM_WARNING, "Naming" ) );
        CORE::CString::StringVector globs;
        globs.push_back( "SM_*" );
        globs.push_back( "Rock" );
        patterns->DelAttribute( "mesh" );
        patterns->SetValuesOfChildByName( "mesh", globs );
        ASSERT_TRUE( validator->Validate( scene, naming, report ) && 1 == CountItems( report, REPORTITEM_WARNING, "Naming" ) );
        ASSERT_TRUE( REPORTITEM_ERROR == CIAssetValidator::GetSeverity( CORE::CDataNode(), REPORTITEM_ERROR ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Texture operations: every step of a chain, on the operations of gucefIMAGE" )
    try
    {
        CTextureBaker::TPixelMapVector sources;
        sources.push_back( MakePicture( 2, 2, 200, 100, 50, 255 ) );
        sources.push_back( MakePicture( 4, 4, 10, 20, 30, 40 ) );
        CAssetTexture texture;
        IMAGE::TPixelMapPtr result;
        CORE::CString problem;

        texture.operations.push_back( CTextureOperation::ExtractChannel( 1 ) );
        ASSERT_TRUE( CTextureBaker::ApplyOperations( texture, sources, result, problem ) );
        ASSERT_TRUE( IMAGE::PSF_SINGLE_CHANNEL_GRAYSCALE == result->GetPixelStorageFormat() && 100 == Texels( *result )[ 0 ] );

        texture.operations.clear();
        texture.operations.push_back( CTextureOperation::Swizzle( "bgra" ) );
        ASSERT_TRUE( CTextureBaker::ApplyOperations( texture, sources, result, problem ) && 50 == Texels( *result )[ 0 ] && 200 == Texels( *result )[ 2 ] );

        texture.operations.clear();
        texture.operations.push_back( CTextureOperation::Invert( 0x8 ) );
        texture.operations.push_back( CTextureOperation::ScaleBias( 0x1, 0.5f, 0.0f ) );
        ASSERT_TRUE( CTextureBaker::ApplyOperations( texture, sources, result, problem ) );
        ASSERT_TRUE( 0 == Texels( *result )[ 3 ] && 100 == Texels( *result )[ 0 ] && 100 == Texels( *result )[ 1 ] );

        // the green flip of a DirectX normal map
        texture.operations.clear();
        texture.AddGreenFlip();
        ASSERT_TRUE( CTextureBaker::ApplyOperations( texture, sources, result, problem ) && 155 == Texels( *result )[ 1 ] && 200 == Texels( *result )[ 0 ] );

        // a repack of channels of two sources of other sizes: the second is resampled to the size of the first
        const Int32 packSources[ 4 ] = { 0, 1, -1, 1 };
        const UInt32 packChannels[ 4 ] = { 0, 1, 0, 3 };
        texture.operations.clear();
        texture.operations.push_back( CTextureOperation::Pack( packSources, packChannels ) );
        ASSERT_TRUE( CTextureBaker::ApplyOperations( texture, sources, result, problem ) );
        ASSERT_TRUE( 2 == result->GetWidthInPixels() && 200 == Texels( *result )[ 0 ] && 20 == Texels( *result )[ 1 ] && 0 == Texels( *result )[ 2 ] && 40 == Texels( *result )[ 3 ] );

        // metallic in R and smoothness in A of a source become glTF metal-roughness: B = R, G = 1 - A
        CTextureBaker::TPixelMapVector metallic;
        metallic.push_back( MakePicture( 2, 2, 255, 0, 0, 204 ) );
        const Int32 mrSources[ 4 ] = { -1, 0, 0, -1 };
        const UInt32 mrChannels[ 4 ] = { 0, 3, 0, 0 };
        texture.operations.clear();
        texture.operations.push_back( CTextureOperation::Pack( mrSources, mrChannels ) );
        texture.operations.push_back( CTextureOperation::Invert( 0x2 ) );
        ASSERT_TRUE( CTextureBaker::ApplyOperations( texture, metallic, result, problem ) );
        ASSERT_TRUE( 0 == Texels( *result )[ 0 ] && 51 == Texels( *result )[ 1 ] && 255 == Texels( *result )[ 2 ] );

        // alpha premultiplication on and off
        CTextureBaker::TPixelMapVector translucent;
        translucent.push_back( MakePicture( 1, 1, 200, 100, 50, 128 ) );
        texture.operations.clear();
        texture.operations.push_back( CTextureOperation::PremultiplyAlpha( IMAGE::COLOURSPACE_LINEAR ) );
        ASSERT_TRUE( CTextureBaker::ApplyOperations( texture, translucent, result, problem ) );
        ASSERT_TRUE( abs( 100 - (Int32) Texels( *result )[ 0 ] ) <= 1 && 128 == Texels( *result )[ 3 ] );
        texture.operations.push_back( CTextureOperation::UnpremultiplyAlpha( IMAGE::COLOURSPACE_LINEAR ) );
        ASSERT_TRUE( CTextureBaker::ApplyOperations( texture, translucent, result, problem ) && abs( 200 - (Int32) Texels( *result )[ 0 ] ) <= 2 );

        // a colour space tag names the space and changes no texel
        texture.operations.clear();
        texture.usage = TEXTUREUSAGE_COLOUR;
        ASSERT_TRUE( IMAGE::COLOURSPACE_SRGB == texture.GetColourSpace() );
        texture.operations.push_back( CTextureOperation::ColourSpaceTag( IMAGE::COLOURSPACE_LINEAR ) );
        ASSERT_TRUE( IMAGE::COLOURSPACE_LINEAR == texture.GetColourSpace() && !texture.HasTexelOperations() && texture.GetChainKeyText().IsNULLOrEmpty() );
        ASSERT_TRUE( CTextureBaker::ApplyOperations( texture, sources, result, problem ) && 200 == Texels( *result )[ 0 ] && 100 == Texels( *result )[ 1 ] );
        texture.usage = TEXTUREUSAGE_NORMAL;
        texture.operations.clear();
        ASSERT_TRUE( IMAGE::COLOURSPACE_LINEAR == texture.GetColourSpace() );

        // the operations as data and as key text
        CTextureOperation op = CTextureOperation::ScaleBias( 0x3, 0.25f, 0.5f );
        CORE::CDataNode node( "op", GUCEF_DATATYPE_OBJECT );
        op.Serialize( node );
        CTextureOperation read;
        ASSERT_TRUE( read.Deserialize( node ) && read == op && "scaleBias(3,0.25,0.5)" == op.ToKeyText() );
        CTextureOperation pack = CTextureOperation::Pack( packSources, packChannels );
        node = CORE::CDataNode( "op", GUCEF_DATATYPE_OBJECT );
        pack.Serialize( node );
        ASSERT_TRUE( read.Deserialize( node ) && read == pack && -1 == read.packSources[ 2 ] && 3 == read.packChannels[ 3 ] );

        // a step that cannot be done fails the chain, with the step named
        texture.operations.clear();
        texture.operations.push_back( CTextureOperation::ExtractChannel( 7 ) );
        ASSERT_FALSE( CTextureBaker::ApplyOperations( texture, sources, result, problem ) );
        ASSERT_TRUE( problem.HasSubstr( "extractChannel(7)", 0 ) >= 0 );
        texture.operations.clear();
        const Int32 badSources[ 4 ] = { 5, -1, -1, -1 };
        texture.operations.push_back( CTextureOperation::Pack( badSources, packChannels ) );
        ASSERT_FALSE( CTextureBaker::ApplyOperations( texture, sources, result, problem ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Texture formats: every format of the textures that packs ship is read through the image codecs and baked to PNG, with the right pixels; what cannot be read is reported" )
    try
    {
        MakeWorkFolder( "formats" );
        struct SFormat { const char* extension; TBytes bytes; UInt32 tolerance; };    // a tolerance of 1000 or more: the picture is one flat colour
        SFormat formats[] = {
            { "png", TBytes(), 0 }, { "tga", MakeTga(), 0 }, { "bmp", MakeBmp(), 0 }, { "psd", MakePsd(), 0 }, { "tif", MakeTiff(), 0 }, { "tiff", MakeTiff(), 0 },
            { "dds", MakeDds(), 0 }, { "hdr", MakeHdr(), 2 }, { "exr", MakeExr(), 2 }, { "jpg", MakeJpg(), 1008 } };
        const size_t formatCount = sizeof( formats ) / sizeof( formats[ 0 ] );
        CConversionReport report;
        report.SelectAsset( "formats.stub" );
        CTextureBaker baker( "work/formats/textures" );
        CORE::CString supported;
        CORE::CString unsupported;
        for ( size_t f=0; f<formatCount; ++f )
        {
            const CORE::CString extension = formats[ f ].extension;
            const CORE::CString source = "work/formats/src/t_" + extension + "." + extension;
            if ( "png" == extension )
            {
                // the picture of the other formats, as a PNG
                IMAGE::TPixelMapPtr picture = MakePicture( FormatSize, FormatSize, 0, 0, 0, 255 );
                UInt8* texels = const_cast< UInt8* >( Texels( *picture ) );
                for ( UInt32 y=0; y<FormatSize; ++y )
                    for ( UInt32 x=0; x<FormatSize; ++x )
                    {
                        UInt8* texel = texels + ( y * FormatSize + x ) * 4;
                        TestTexel( x, y, texel[ 0 ], texel[ 1 ], texel[ 2 ] );
                    }
                ASSERT_TRUE( CTextureBaker::SavePng( source, *picture ) );
            }
            else
            {
                ASSERT_TRUE( StoreBytes( source, formats[ f ].bytes ) );
            }
            CAssetTexture texture;
            texture.name = extension;
            texture.sources.push_back( source );
            const bool baked = baker.Bake( texture, report );
            bool pixelsRight = false;
            if ( baked )
            {
                IMAGE::TPixelMapPtr back;
                const UInt32 expectedSize = formats[ f ].tolerance >= 1000 ? 8 : FormatSize;     // the JPEG is 8 x 8
                pixelsRight = expectedSize == texture.width && expectedSize == texture.height && ( "jpg" == extension ? "jpg" : "png" ) == CVfsPaths::ExtensionOf( texture.bakedPath ) &&
                              CTextureBaker::LoadPicture( texture.bakedPath, back ) && !back.IsNULL();
                if ( pixelsRight )
                {
                    const UInt32 channels = IMAGE::PSF_RGBA == back->GetPixelStorageFormat() ? 4 : ( IMAGE::PSF_RGB == back->GetPixelStorageFormat() ? 3 : 0 );
                    pixelsRight = channels >= 3;
                    for ( UInt32 y=0; y<FormatSize && pixelsRight; ++y )
                        for ( UInt32 x=0; x<FormatSize && pixelsRight; ++x )
                        {
                            UInt8 r, g, b;
                            TestTexel( x, y, r, g, b );
                            if ( formats[ f ].tolerance >= 1000 )
                            {
                                r = 200; g = 100; b = 50;    // the JPEG is one flat colour
                            }
                            const UInt8* texel = Texels( *back ) + ( y * expectedSize + x ) * channels;
                            const UInt32 tolerance = formats[ f ].tolerance % 1000;
                            pixelsRight = (UInt32) abs( (Int32) texel[ 0 ] - (Int32) r ) <= tolerance && (UInt32) abs( (Int32) texel[ 1 ] - (Int32) g ) <= tolerance &&
                                          (UInt32) abs( (Int32) texel[ 2 ] - (Int32) b ) <= tolerance;
                        }
                }
            }
            if ( baked && !pixelsRight )
            {
                IMAGE::TPixelMapPtr back;
                if ( CTextureBaker::LoadPicture( texture.bakedPath, back ) && !back.IsNULL() )
                    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Texture formats: ." << extension << " first texel " << (UInt32) Texels( *back )[ 0 ] << "," << (UInt32) Texels( *back )[ 1 ] << "," << (UInt32) Texels( *back )[ 2 ] <<
                               " storage format " << (UInt32) back->GetPixelStorageFormat() << " size " << back->GetWidthInPixels() << "x" << back->GetHeightInPixels() );
            }
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Texture formats: ." << extension << ( baked ? ( pixelsRight ? ( "jpg" == extension ? " kept as JPEG (glTF reads it), decodes with the right pixels" : " baked to PNG with the right pixels" ) : " baked, the pixels are WRONG" ) : " not read" ) );
            ( baked && pixelsRight ? supported : unsupported ) += CORE::CString( " " ) + extension;
            ASSERT_TRUE( baked && pixelsRight );
        }
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Texture formats: supported:" << supported << ", not supported:" << unsupported );

        // a file that no codec reads is reported with the reason, the texture is marked missing: it is not dropped silently
        TBytes garbage;
        PutText( garbage, "this is not a picture" );
        ASSERT_TRUE( StoreBytes( "work/formats/src/broken.xyz", garbage ) );
        CAssetTexture broken;
        broken.name = "broken";
        broken.sources.push_back( "work/formats/src/broken.xyz" );
        ASSERT_FALSE( baker.Bake( broken, report ) );
        ASSERT_TRUE( broken.missing && CountItems( report, REPORTITEM_ERROR, "TextureBaker" ) >= 1 );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Texture formats: greyscale PNG files of 8 and 16 bits (as the source pictures of masks are) are read as one grey channel and baked with their values" )
    try
    {
        MakeWorkFolder( "greyformats" );
        CConversionReport report;
        report.SelectAsset( "grey.stub" );
        CTextureBaker baker( "work/greyformats/textures" );
        const UInt32 bitDepths[ 2 ] = { 8, 16 };
        for ( UInt32 d=0; d<2; ++d )
        {
            const CORE::CString source = "work/greyformats/src/grey" + CORE::UInt32ToString( bitDepths[ d ] ) + ".png";
            ASSERT_TRUE( StoreBytes( source, MakeGreyPng( bitDepths[ d ] ) ) );
            IMAGE::TPixelMapPtr picture;
            CORE::CString problem;
            const bool read = CTextureBaker::LoadPicture( source, picture, problem );
            if ( !read )
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Grey PNG of " << bitDepths[ d ] << " bits: " << problem );
            ASSERT_TRUE( read && !picture.IsNULL() );
            if ( !read || picture.IsNULL() )
                continue;
            ASSERT_TRUE( IMAGE::PSF_SINGLE_CHANNEL_GRAYSCALE == picture->GetPixelStorageFormat() && GUCEF::DATATYPE_UINT8 == picture->GetPixelComponentDataType() );
            ASSERT_TRUE( FormatSize == picture->GetWidthInPixels() && FormatSize == picture->GetHeightInPixels() );
            bool valuesRight = GUCEF::DATATYPE_UINT8 == picture->GetPixelComponentDataType();
            for ( UInt32 y=0; y<FormatSize && valuesRight; ++y )
                for ( UInt32 x=0; x<FormatSize && valuesRight; ++x )
                    valuesRight = abs( (Int32) Texels( *picture )[ y * FormatSize + x ] - (Int32) GreyTexel( x, y ) ) <= 1;
            ASSERT_TRUE( valuesRight );

            // the baker writes it as a PNG that reads back the same
            CAssetTexture texture;
            texture.name = "grey" + CORE::UInt32ToString( bitDepths[ d ] );
            texture.sources.push_back( source );
            ASSERT_TRUE( baker.Bake( texture, report ) && !texture.missing );
            IMAGE::TPixelMapPtr back;
            ASSERT_TRUE( CTextureBaker::LoadPicture( texture.bakedPath, back ) && !back.IsNULL() && FormatSize == back->GetWidthInPixels() );
        }
        ASSERT_TRUE( 0 == CountItems( report, REPORTITEM_ERROR, "TextureBaker" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Kinds: the scenes of the pack itself rank before those of a pack bundled with it, by the words of the name of the pack" )
    try
    {
        CORE::CString::StringVector words;
        CAssetPackListing::SplitWords( "POLYGON_Shops_Unity_2022_3_v1_6_6", words );
        ASSERT_TRUE( 8 == words.size() && "polygon" == words[ 0 ] && "shops" == words[ 1 ] && "v1" == words[ 5 ] );
        words.clear();
        CAssetPackListing::SplitWords( "PolygonShopsDemo_Lite01", words );
        ASSERT_TRUE( 4 == words.size() && "polygon" == words[ 0 ] && "shops" == words[ 1 ] && "demo" == words[ 2 ] && "lite01" == words[ 3 ] );

        CAssetPackListing listing;
        listing.SetPackName( "POLYGON_Shops_Unity_2022_3_v1_6_6" );
        listing.AddFile( "Assets/Synty/PolygonGeneric/Scenes/Overview.unity" );
        listing.AddFile( "Assets/Synty/PolygonShops/Scenes/Overview.unity" );
        listing.AddFile( "Assets/Synty/PolygonShops/Scenes/Demo_Lite.unity" );
        listing.AddFile( "Assets/Synty/PolygonShops/Scenes/Demo.unity" );
        CORE::CString::StringVector keywords, ranked;
        keywords.push_back( "demo" );
        keywords.push_back( "overview" );
        ASSERT_TRUE( 4 == listing.FindFilesRankedForPack( "*.unity", keywords, ranked, 16 ) );
        ASSERT_TRUE( "Assets/Synty/PolygonShops/Scenes/Demo.unity" == ranked[ 0 ] && "Assets/Synty/PolygonShops/Scenes/Demo_Lite.unity" == ranked[ 1 ] &&
                     "Assets/Synty/PolygonShops/Scenes/Overview.unity" == ranked[ 2 ] && "Assets/Synty/PolygonGeneric/Scenes/Overview.unity" == ranked[ 3 ] );
        // a name of the pack that has only words that say nothing leaves the order to the keywords
        listing.SetPackName( "Unity_Pack_v2" );
        ranked.clear();
        ASSERT_TRUE( 2 == listing.FindFilesRankedForPack( "*.unity", keywords, ranked, 2 ) && "Assets/Synty/PolygonShops/Scenes/Demo.unity" == ranked[ 0 ] );
        // the name of the pack is the last part of the folder the listing was read from: set by hand it wins
        ASSERT_TRUE( "Unity_Pack_v2" == listing.GetPackName() && CORE::CString::Empty == CAssetPackListing().GetPackName() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Texture baker: the result of a chain is written once, PNG sources without operations are copied, missing sources reported" )
    try
    {
        MakeWorkFolder( "baker" );
        ASSERT_TRUE( WriteTestPng( "work/baker/src/a.png", 4, 4, 200, 100, 50, 255 ) );
        IMAGE::TPixelMapPtr written;
        ASSERT_TRUE( CTextureBaker::LoadPicture( "work/baker/src/a.png", written ) );
        ASSERT_TRUE( !written.IsNULL() && IMAGE::PSF_RGBA == written->GetPixelStorageFormat() && 200 == Texels( *written )[ 0 ] && 50 == Texels( *written )[ 2 ] );

        CConversionReport report;
        report.SelectAsset( "baker.stub" );
        CTextureBaker baker( "work/baker/textures" );
        CAssetTexture flipped;
        flipped.name = "flipped";
        flipped.sources.push_back( "work/baker/src/a.png" );
        flipped.operations.push_back( CTextureOperation::Invert( 0x2 ) );
        CAssetTexture same = flipped;
        CAssetTexture tagged = flipped;
        tagged.operations.push_back( CTextureOperation::ColourSpaceTag( IMAGE::COLOURSPACE_LINEAR ) );
        CAssetTexture copied;
        copied.name = "copied";
        copied.sources.push_back( "work/baker/src/a.png" );
        CAssetTexture lost;
        lost.name = "lost";
        lost.sources.push_back( "work/baker/src/none.png" );

        ASSERT_TRUE( baker.Bake( flipped, report ) && baker.Bake( same, report ) && baker.Bake( tagged, report ) && baker.Bake( copied, report ) );
        ASSERT_TRUE( flipped.bakedPath == same.bakedPath && flipped.bakedPath == tagged.bakedPath && flipped.bakedPath != copied.bakedPath );
        ASSERT_TRUE( "work/baker/textures" == CVfsPaths::FolderOf( flipped.bakedPath ) && CVfsPaths::NameOf( flipped.bakedPath ).HasSubstr( "a.", 0 ) == 0 );
        ASSERT_TRUE( 4 == flipped.width && 4 == flipped.height && !flipped.missing );
        ASSERT_TRUE( 2 == baker.GetWrittenCount() && 2 == baker.GetReusedCount() );
        IMAGE::TPixelMapPtr baked;
        ASSERT_TRUE( CTextureBaker::LoadPicture( flipped.bakedPath, baked ) && 155 == Texels( *baked )[ 1 ] && 200 == Texels( *baked )[ 0 ] );
        ASSERT_TRUE( CContentHash::HashFile( copied.bakedPath ) == CContentHash::HashFile( "work/baker/src/a.png" ) );

        ASSERT_FALSE( baker.Bake( lost, report ) );
        ASSERT_TRUE( lost.missing && 1 == CountItems( report, REPORTITEM_ERROR, "TextureBaker" ) );
        CAssetTexture sourceless;
        ASSERT_FALSE( baker.Bake( sourceless, report ) );

        // a later run finds the file of the chain and writes nothing
        CTextureBaker later( "work/baker/textures" );
        CAssetTexture again;
        again.sources.push_back( "work/baker/src/a.png" );
        again.operations.push_back( CTextureOperation::Invert( 0x2 ) );
        ASSERT_TRUE( later.Bake( again, report ) && again.bakedPath == flipped.bakedPath && 0 == later.GetWrittenCount() && 1 == later.GetReusedCount() );

        // the files are listed for the asset
        const CConversionReportEntry* entry = report.FindEntry( "baker.stub" );
        ASSERT_TRUE( GUCEF_NULL != entry && 2 == entry->files.size() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Model: ExtractTriangles copies the triangles with only their vertices and every stream" )
    try
    {
        CAssetSubMesh source;
        source.materialIndex = 3;
        source.influencesPerVertex = 5;
        CAssetMorphTarget target;
        target.name = "smile";
        target.defaultWeight = 0.5f;
        for ( UInt32 v=0; v<4; ++v )
        {
            const Float32 f = (Float32) v;
            source.positions.push_back( f ); source.positions.push_back( f + 0.1f ); source.positions.push_back( f + 0.2f );
            source.normals.push_back( 0.0f ); source.normals.push_back( 0.0f ); source.normals.push_back( 1.0f );
            for ( UInt32 c=0; c<4; ++c ) source.tangents.push_back( c == 3 ? -1.0f : f );
            source.uvSets[ 0 ].push_back( f * 0.25f ); source.uvSets[ 0 ].push_back( 1.0f - f * 0.25f );
            source.uvSets[ 1 ].push_back( f ); source.uvSets[ 1 ].push_back( f );
            for ( UInt32 c=0; c<4; ++c ) source.colours.push_back( f / 4.0f );
            for ( UInt32 c=0; c<4; ++c ) source.masks.push_back( c == 0 ? f / 3.0f : 1.0f );
            for ( UInt32 k=0; k<5; ++k ) { source.joints.push_back( v * 10 + k ); source.weights.push_back( 0.2f ); }
            for ( UInt32 c=0; c<3; ++c ) { target.positionDeltas.push_back( f * 2.0f ); target.normalDeltas.push_back( 0.0f ); }
        }
        source.morphTargets.push_back( target );
        const UInt32 indices[] = { 0, 1, 2,  2, 1, 3 };
        source.indices.assign( indices, indices + 6 );

        CAssetSubMesh::TUInt32Vector second;
        second.push_back( 1 );
        CAssetSubMesh out;
        source.ExtractTriangles( second, out );
        // the vertices 2, 1, 3 become 0, 1, 2 in the order of first use
        ASSERT_TRUE( 3 == out.GetVertexCount() && 1 == out.GetTriangleCount() && 0 == out.indices[ 0 ] && 1 == out.indices[ 1 ] && 2 == out.indices[ 2 ] );
        ASSERT_TRUE( 3 == out.materialIndex && 5 == out.influencesPerVertex && 15 == out.joints.size() && 15 == out.weights.size() );
        ASSERT_TRUE( 2.0f == out.positions[ 0 ] && 1.0f == out.positions[ 3 ] && 3.0f == out.positions[ 6 ] );
        ASSERT_TRUE( 20 == out.joints[ 0 ] && 24 == out.joints[ 4 ] && 10 == out.joints[ 5 ] && 30 == out.joints[ 10 ] );
        ASSERT_TRUE( 12 == out.tangents.size() && 6 == out.uvSets[ 0 ].size() && 6 == out.uvSets[ 1 ].size() && out.uvSets[ 2 ].empty() );
        ASSERT_TRUE( 0.5f == out.uvSets[ 0 ][ 0 ] && 0.5f == out.uvSets[ 0 ][ 1 ] );
        ASSERT_TRUE( 12 == out.colours.size() && 0.5f == out.colours[ 0 ] && 12 == out.masks.size() && fabs( out.masks[ 0 ] - 2.0f / 3.0f ) < 1e-6f && 1.0f == out.masks[ 1 ] );
        ASSERT_TRUE( 1 == out.morphTargets.size() && "smile" == out.morphTargets[ 0 ].name && 0.5f == out.morphTargets[ 0 ].defaultWeight );
        ASSERT_TRUE( 9 == out.morphTargets[ 0 ].positionDeltas.size() && 4.0f == out.morphTargets[ 0 ].positionDeltas[ 0 ] && out.morphTargets[ 0 ].tangentDeltas.empty() );

        // both triangles: every vertex once, a stream that is not there stays empty
        CAssetSubMesh::TUInt32Vector both;
        both.push_back( 0 );
        both.push_back( 1 );
        source.tangents.clear();
        source.ExtractTriangles( both, out );
        ASSERT_TRUE( 4 == out.GetVertexCount() && 2 == out.GetTriangleCount() && out.tangents.empty() && 16 == out.masks.size() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Validators: LayerPenetration counts the vertices of a body that show through its garments" )
    try
    {
        // a body: a flat patch of 11 x 11 vertices, 0.2 m square in x and y, facing +z; two garments: one 1 cm in front of the
        // part with x below 0.099 m (covers it), one 5 mm behind the rest (the body shows through there)
        CAssetScene scene;
        scene.name = "layers";
        CAssetMesh bodyMesh;
        bodyMesh.name = "Body";
        CAssetSubMesh patch;
        for ( UInt32 y=0; y<=10; ++y )
        {
            for ( UInt32 x=0; x<=10; ++x )
            {
                patch.positions.push_back( 0.02f * x ); patch.positions.push_back( 0.02f * y ); patch.positions.push_back( 0.0f );
                patch.normals.push_back( 0.0f ); patch.normals.push_back( 0.0f ); patch.normals.push_back( 1.0f );
                if ( x < 10 && y < 10 )
                {
                    const UInt32 a = y * 11 + x;
                    patch.indices.push_back( a ); patch.indices.push_back( a + 1 ); patch.indices.push_back( a + 12 );
                    patch.indices.push_back( a ); patch.indices.push_back( a + 12 ); patch.indices.push_back( a + 11 );
                }
            }
        }
        bodyMesh.subMeshes.push_back( patch );
        scene.meshes.push_back( bodyMesh );
        CAssetMesh clothMesh;
        clothMesh.name = "Cloth";
        const Float32 quads[ 2 ][ 3 ] = { { -0.01f, 0.099f, 0.01f }, { 0.099f, 0.21f, -0.005f } };
        for ( UInt32 q=0; q<2; ++q )
        {
            CAssetSubMesh quad;
            const Float32 x0 = quads[ q ][ 0 ], x1 = quads[ q ][ 1 ], z = quads[ q ][ 2 ];
            const Float32 corners[ 4 ][ 2 ] = { { x0, -0.01f }, { x1, -0.01f }, { x1, 0.21f }, { x0, 0.21f } };
            for ( UInt32 c=0; c<4; ++c )
            {
                quad.positions.push_back( corners[ c ][ 0 ] ); quad.positions.push_back( corners[ c ][ 1 ] ); quad.positions.push_back( z );
                quad.normals.push_back( 0.0f ); quad.normals.push_back( 0.0f ); quad.normals.push_back( 1.0f );
            }
            const UInt32 indices[] = { 0, 1, 2,  0, 2, 3 };
            quad.indices.assign( indices, indices + 6 );
            clothMesh.subMeshes.push_back( quad );
        }
        scene.meshes.push_back( clothMesh );

        CAssetPluginRegistry& plugins = CAssetConverterGlobal::Instance()->GetPluginRegistry();
        TAssetValidatorPtr validator;
        ASSERT_TRUE( plugins.TryGetValidator( "LayerPenetration", validator ) );

        // without parts there is nothing to measure
        const UInt32 root = scene.AddNode( "Character" );
        const UInt32 bodyHolder = scene.AddNode( "BodyHolder", (Int32) root );
        const UInt32 bodyNode = scene.AddNode( "BodyLod0", (Int32) bodyHolder );
        scene.nodes[ bodyNode ].mesh = 0;
        const UInt32 clothHolder = scene.AddNode( "ClothHolder", (Int32) root );
        const UInt32 clothNode = scene.AddNode( "ClothLod0", (Int32) clothHolder );
        scene.nodes[ clothNode ].mesh = 1;
        CConversionReport report;
        report.SelectAsset( "layers.stub" );
        CLayerPenetrationValidator::SResult result;
        ASSERT_FALSE( CLayerPenetrationValidator::Measure( scene, 0.10f, 0.03f, 0.0002f, result ) );
        ASSERT_TRUE( validator->Validate( scene, CORE::CDataNode(), report ) && 0 == CountItems( report, REPORTITEM_WARNING, "LayerPenetration" ) );

        scene.nodes[ bodyHolder ].extras.SetAttribute( CLayerPenetrationValidator::PartAttribute, CLayerPenetrationValidator::BodyPart );
        scene.nodes[ clothHolder ].extras.SetAttribute( CLayerPenetrationValidator::PartAttribute, CLayerPenetrationValidator::GarmentPart );
        ASSERT_TRUE( CLayerPenetrationValidator::Measure( scene, 0.10f, 0.03f, 0.0002f, result ) );
        ASSERT_TRUE( 121 == result.bodyVertices && 55 == result.coveredVertices && 66 == result.pokingVertices );
        ASSERT_TRUE( fabs( result.deepestPoke - 0.005f ) < 1e-5f && 1 == result.pokesPerJoint.size() && 66 == result.pokesPerJoint[ "none" ] );
        ASSERT_TRUE( validator->Validate( scene, CORE::CDataNode(), report ) && 1 == CountItems( report, REPORTITEM_WARNING, "LayerPenetration" ) && 1 == CountItems( report, REPORTITEM_INFO, "LayerPenetration" ) );

        // a garment surface further behind than the reach does not count; a deeper minimum ignores the shallow pokes
        ASSERT_TRUE( CLayerPenetrationValidator::Measure( scene, 0.10f, 0.004f, 0.0002f, result ) && 0 == result.pokingVertices && 55 == result.coveredVertices );
        ASSERT_TRUE( CLayerPenetrationValidator::Measure( scene, 0.10f, 0.03f, 0.006f, result ) && 0 == result.pokingVertices );

        // with the poking garment moved 2 mm in front, the whole body is covered
        for ( UInt32 c=0; c<4; ++c )
            scene.meshes[ 1 ].subMeshes[ 1 ].positions[ c * 3 + 2 ] = 0.002f;
        ASSERT_TRUE( CLayerPenetrationValidator::Measure( scene, 0.10f, 0.03f, 0.0002f, result ) && 121 == result.coveredVertices && 0 == result.pokingVertices );
        CORE::CDataNode lenient( "settings", GUCEF_DATATYPE_OBJECT );
        lenient.SetAttribute( "severity", "error" );
        ASSERT_TRUE( validator->Validate( scene, lenient, report ) && 0 == CountItems( report, REPORTITEM_ERROR, "LayerPenetration" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Validators: LayerPenetration measures the visible body: the faces that the hidden face set of a worn outfit hides are left out" )
    try
    {
        // the body patch of the test above; the garment covers the left part (1 cm in front) and lies 5 mm behind the right part.
        // The outfit hides the triangles that touch the right part (x of 0.1 m and more): the body stays complete in two submeshes
        CAssetScene scene;
        scene.name = "outfit";
        CAssetMesh bodyMesh;
        bodyMesh.name = "Body";
        CAssetSubMesh patch;
        patch.materialIndex = 0;
        for ( UInt32 y=0; y<=10; ++y )
        {
            for ( UInt32 x=0; x<=10; ++x )
            {
                patch.positions.push_back( 0.02f * x ); patch.positions.push_back( 0.02f * y ); patch.positions.push_back( 0.0f );
                patch.normals.push_back( 0.0f ); patch.normals.push_back( 0.0f ); patch.normals.push_back( 1.0f );
                if ( x < 10 && y < 10 )
                {
                    const UInt32 a = y * 11 + x;
                    patch.indices.push_back( a ); patch.indices.push_back( a + 1 ); patch.indices.push_back( a + 12 );
                    patch.indices.push_back( a ); patch.indices.push_back( a + 12 ); patch.indices.push_back( a + 11 );
                }
            }
        }
        CAssetSubMesh::TUInt32Vector shown;
        CAssetSubMesh::TUInt32Vector hidden;
        for ( UInt32 t=0; t<patch.GetTriangleCount(); ++t )
        {
            bool right = false;
            for ( UInt32 c=0; c<3; ++c )
                right = right || patch.positions[ patch.indices[ t * 3 + c ] * 3 ] >= 0.099f;
            ( right ? hidden : shown ).push_back( t );
        }
        CAssetSubMesh shownPart;
        CAssetSubMesh hiddenPart;
        patch.ExtractTriangles( shown, shownPart );
        patch.hiddenBy = CAssetHiddenFaceSet::BitOf( 0 );
        patch.ExtractTriangles( hidden, hiddenPart );
        ASSERT_TRUE( 1 == hiddenPart.hiddenBy && 0 == hiddenPart.materialIndex && 0 == shownPart.hiddenBy );   // ExtractTriangles keeps the sets of its source
        ASSERT_TRUE( 80 == shownPart.GetTriangleCount() && 120 == hiddenPart.GetTriangleCount() && 55 == shownPart.GetVertexCount() && 77 == hiddenPart.GetVertexCount() );
        bodyMesh.subMeshes.push_back( shownPart );
        bodyMesh.subMeshes.push_back( hiddenPart );
        scene.meshes.push_back( bodyMesh );
        CAssetMesh shellMesh;
        shellMesh.name = "Shell";
        const Float32 quads[ 2 ][ 3 ] = { { -0.01f, 0.099f, 0.01f }, { 0.099f, 0.21f, -0.005f } };
        for ( UInt32 q=0; q<2; ++q )
        {
            CAssetSubMesh quad;
            const Float32 x0 = quads[ q ][ 0 ], x1 = quads[ q ][ 1 ], z = quads[ q ][ 2 ];
            const Float32 corners[ 4 ][ 2 ] = { { x0, -0.01f }, { x1, -0.01f }, { x1, 0.21f }, { x0, 0.21f } };
            for ( UInt32 c=0; c<4; ++c )
            {
                quad.positions.push_back( corners[ c ][ 0 ] ); quad.positions.push_back( corners[ c ][ 1 ] ); quad.positions.push_back( z );
                quad.normals.push_back( 0.0f ); quad.normals.push_back( 0.0f ); quad.normals.push_back( 1.0f );
            }
            const UInt32 indices[] = { 0, 1, 2,  0, 2, 3 };
            quad.indices.assign( indices, indices + 6 );
            shellMesh.subMeshes.push_back( quad );
        }
        scene.meshes.push_back( shellMesh );
        const UInt32 root = scene.AddNode( "Character" );
        const UInt32 bodyNode = scene.AddNode( "Body", (Int32) root );
        scene.nodes[ bodyNode ].mesh = 0;
        scene.nodes[ bodyNode ].extras.SetAttribute( CLayerPenetrationValidator::PartAttribute, CLayerPenetrationValidator::BodyPart );
        const UInt32 shellNode = scene.AddNode( "Shell", (Int32) root );
        scene.nodes[ shellNode ].mesh = 1;
        scene.nodes[ shellNode ].extras.SetAttribute( CLayerPenetrationValidator::PartAttribute, CLayerPenetrationValidator::GarmentPart );
        CAssetHiddenFaceSet outfit;
        outfit.name = "Outfit";
        outfit.garments.push_back( "Shell" );
        outfit.hiddenTriangles = 120;
        scene.hiddenFaceSets.push_back( outfit );

        // the outfit not worn: the whole body counts, as without sets (the vertices on the seam are in both submeshes)
        CLayerPenetrationValidator::SResult result;
        ASSERT_TRUE( 0 == scene.GetActiveHiddenFaceMask() && !scene.IsHiddenByActiveSet( hiddenPart ) );
        ASSERT_TRUE( CLayerPenetrationValidator::Measure( scene, 0.10f, 0.03f, 0.0002f, result ) );
        ASSERT_TRUE( 132 == result.bodyVertices && 0 == result.hiddenBodyVertices && 66 == result.coveredVertices && 66 == result.pokingVertices );

        // worn: only the visible part is measured, and it is covered
        scene.hiddenFaceSets[ 0 ].active = true;
        ASSERT_TRUE( 1 == scene.GetActiveHiddenFaceMask() && scene.IsHiddenByActiveSet( scene.meshes[ 0 ].subMeshes[ 1 ] ) && !scene.IsHiddenByActiveSet( scene.meshes[ 0 ].subMeshes[ 0 ] ) );
        ASSERT_TRUE( CLayerPenetrationValidator::Measure( scene, 0.10f, 0.03f, 0.0002f, result ) );
        ASSERT_TRUE( 55 == result.bodyVertices && 77 == result.hiddenBodyVertices && 55 == result.coveredVertices && 0 == result.pokingVertices && 0.0f == result.deepestPoke );
        CAssetPluginRegistry& plugins = CAssetConverterGlobal::Instance()->GetPluginRegistry();
        TAssetValidatorPtr validator;
        ASSERT_TRUE( plugins.TryGetValidator( "LayerPenetration", validator ) && 2 == validator->GetVersion() );
        CConversionReport report;
        report.SelectAsset( "outfit.stub" );
        ASSERT_TRUE( validator->Validate( scene, CORE::CDataNode(), report ) && 0 == CountItems( report, REPORTITEM_WARNING, "LayerPenetration" ) && 1 == CountItems( report, REPORTITEM_INFO, "LayerPenetration" ) );

        // the sets are part of the model: the sidecar lists them
        CORE::CDataNode array( "sets", GUCEF_DATATYPE_ARRAY );
        ASSERT_TRUE( CAssetHiddenFaceSet::SerializeList( scene.hiddenFaceSets, array ) );
        CAssetHiddenFaceSet::THiddenFaceSetVector back;
        ASSERT_TRUE( CAssetHiddenFaceSet::DeserializeList( array, back ) && 1 == back.size() && "Outfit" == back[ 0 ].name && back[ 0 ].active && 120 == back[ 0 ].hiddenTriangles );
        ASSERT_TRUE( 1 == back[ 0 ].garments.size() && "Shell" == back[ 0 ].garments[ 0 ] );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
