/*
 *  assetpluginUNREAL: Writes PNG files from raw pixels (stored deflate blocks)
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

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALPNGWRITER_H
#include "assetpluginUNREAL_CUnrealPngWriter.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALPNGWRITER_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALPNGWRITER_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETPLUGIN {
namespace UNREAL {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

UInt32
CUnrealPngWriter::Crc32( const UInt8* bytes, UInt32 size, UInt32 start )
{GUCEF_TRACE;

    static UInt32 table[ 256 ];
    static bool ready = false;
    if ( !ready )
    {
        for ( UInt32 i=0; i<256; ++i )
        {
            UInt32 c = i;
            for ( UInt32 k=0; k<8; ++k )
                c = ( c & 1 ) ? 0xEDB88320 ^ ( c >> 1 ) : c >> 1;
            table[ i ] = c;
        }
        ready = true;
    }
    UInt32 crc = ~start;
    for ( UInt32 i=0; i<size; ++i )
        crc = table[ ( crc ^ bytes[ i ] ) & 0xFF ] ^ ( crc >> 8 );
    return ~crc;
}

/*-------------------------------------------------------------------------*/

static void
PutUInt32BE( CORE::CDynamicBuffer& buffer, UInt32 value )
{GUCEF_TRACE;

    const UInt8 bytes[ 4 ] = { (UInt8) ( value >> 24 ), (UInt8) ( value >> 16 ), (UInt8) ( value >> 8 ), (UInt8) value };
    buffer.Append( bytes, 4 );
}

/*-------------------------------------------------------------------------*/

/** One chunk: length, type, data and the CRC of type and data */
static void
PutChunk( CORE::CDynamicBuffer& png, const char* type, const UInt8* data, UInt32 size )
{GUCEF_TRACE;

    PutUInt32BE( png, size );
    const UInt32 start = png.GetDataSize();
    png.Append( type, 4 );
    if ( size > 0 )
        png.Append( data, size );
    const UInt32 crc = CUnrealPngWriter::Crc32( static_cast< const UInt8* >( png.GetConstBufferPtr( start ) ), size + 4 );
    PutUInt32BE( png, crc );
}

/*-------------------------------------------------------------------------*/

bool
CUnrealPngWriter::Write( const UInt8* pixels, UInt32 width, UInt32 height, UInt32 channels, UInt32 bitsPerChannel, CORE::CDynamicBuffer& png )
{GUCEF_TRACE;

    if ( 0 == width || 0 == height || ( 8 != bitsPerChannel && 16 != bitsPerChannel ) || ( 1 != channels && 3 != channels && 4 != channels ) )
        return false;
    const UInt64 rowBytes = (UInt64) width * channels * ( bitsPerChannel / 8 );
    const UInt64 raw = ( rowBytes + 1 ) * height;
    if ( raw > 0xF0000000ULL )
        return false;

    png.Clear( true );
    static const UInt8 signature[ 8 ] = { 0x89, 'P', 'N', 'G', 0x0D, 0x0A, 0x1A, 0x0A };
    png.Append( signature, 8 );

    UInt8 header[ 13 ];
    header[ 0 ] = (UInt8) ( width >> 24 ); header[ 1 ] = (UInt8) ( width >> 16 ); header[ 2 ] = (UInt8) ( width >> 8 ); header[ 3 ] = (UInt8) width;
    header[ 4 ] = (UInt8) ( height >> 24 ); header[ 5 ] = (UInt8) ( height >> 16 ); header[ 6 ] = (UInt8) ( height >> 8 ); header[ 7 ] = (UInt8) height;
    header[ 8 ] = (UInt8) bitsPerChannel;
    header[ 9 ] = 1 == channels ? 0 : ( 3 == channels ? 2 : 6 );
    header[ 10 ] = 0; header[ 11 ] = 0; header[ 12 ] = 0;
    PutChunk( png, "IHDR", header, 13 );

    // the scan lines (filter type none) in a zlib stream of stored blocks of at most 65535 bytes
    GUCEF::vector< UInt8 > zlib;
    zlib.reserve( (size_t) raw + raw / 65535 * 5 + 64 );
    zlib.push_back( 0x78 );
    zlib.push_back( 0x01 );
    UInt32 adlerA = 1, adlerB = 0;
    UInt64 produced = 0;
    UInt32 blockLeft = 0;
    UInt64 row = 0;
    UInt64 inRow = 0;      // position in the scan line including the filter byte
    GUCEF::vector< UInt8 > block;
    block.reserve( 65535 );
    while ( produced < raw )
    {
        block.clear();
        while ( block.size() < 65535 && produced + block.size() < raw )
        {
            UInt8 value;
            if ( 0 == inRow )
                value = 0;
            else
            {
                const UInt64 index = row * rowBytes + ( inRow - 1 );
                if ( 16 == bitsPerChannel )
                {
                    // the pixels are little endian words, PNG wants big endian
                    value = pixels[ index ^ 1 ];
                }
                else
                    value = pixels[ index ];
            }
            block.push_back( value );
            if ( ++inRow > rowBytes )
            {
                inRow = 0;
                ++row;
            }
        }
        const bool last = produced + block.size() >= raw;
        const UInt16 length = (UInt16) block.size();
        zlib.push_back( last ? 1 : 0 );
        zlib.push_back( (UInt8) ( length & 0xFF ) );
        zlib.push_back( (UInt8) ( length >> 8 ) );
        zlib.push_back( (UInt8) ( ~length & 0xFF ) );
        zlib.push_back( (UInt8) ( ( ~length >> 8 ) & 0xFF ) );
        for ( size_t i=0; i<block.size(); ++i )
        {
            adlerA = ( adlerA + block[ i ] ) % 65521;
            adlerB = ( adlerB + adlerA ) % 65521;
        }
        zlib.insert( zlib.end(), block.begin(), block.end() );
        produced += block.size();
        blockLeft = 0;
    }
    (void) blockLeft;
    const UInt32 adler = ( adlerB << 16 ) | adlerA;
    zlib.push_back( (UInt8) ( adler >> 24 ) ); zlib.push_back( (UInt8) ( adler >> 16 ) ); zlib.push_back( (UInt8) ( adler >> 8 ) ); zlib.push_back( (UInt8) adler );
    if ( zlib.size() > 0xF0000000ULL )
        return false;
    PutChunk( png, "IDAT", &zlib[ 0 ], (UInt32) zlib.size() );
    PutChunk( png, "IEND", GUCEF_NULL, 0 );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CUnrealPngWriter::ReadPngInfo( const UInt8* bytes, UInt32 size, UInt32& width, UInt32& height, UInt32& channels, UInt32& bitsPerChannel )
{GUCEF_TRACE;

    static const UInt8 signature[ 8 ] = { 0x89, 'P', 'N', 'G', 0x0D, 0x0A, 0x1A, 0x0A };
    if ( size < 29 || 0 != memcmp( bytes, signature, 8 ) || 0 != memcmp( bytes + 12, "IHDR", 4 ) )
        return false;
    width = ( (UInt32) bytes[ 16 ] << 24 ) | ( (UInt32) bytes[ 17 ] << 16 ) | ( (UInt32) bytes[ 18 ] << 8 ) | bytes[ 19 ];
    height = ( (UInt32) bytes[ 20 ] << 24 ) | ( (UInt32) bytes[ 21 ] << 16 ) | ( (UInt32) bytes[ 22 ] << 8 ) | bytes[ 23 ];
    bitsPerChannel = bytes[ 24 ];
    switch ( bytes[ 25 ] )
    {
        case 0: channels = 1; break;
        case 2: channels = 3; break;
        case 3: channels = 3; break;
        case 4: channels = 2; break;
        case 6: channels = 4; break;
        default: return false;
    }
    return width > 0 && height > 0;
}

/*-------------------------------------------------------------------------*/

bool
CUnrealPngWriter::ReadJpegInfo( const UInt8* bytes, UInt32 size, UInt32& width, UInt32& height, UInt32& channels )
{GUCEF_TRACE;

    if ( size < 4 || 0xFF != bytes[ 0 ] || 0xD8 != bytes[ 1 ] )
        return false;
    UInt32 at = 2;
    while ( at + 4 <= size )
    {
        if ( 0xFF != bytes[ at ] )
        {
            ++at;
            continue;
        }
        const UInt8 marker = bytes[ at + 1 ];
        if ( 0xFF == marker || 0x00 == marker || ( marker >= 0xD0 && marker <= 0xD8 ) || 0x01 == marker )
        {
            at += ( 0xFF == marker ) ? 1 : 2;
            continue;
        }
        const UInt32 length = ( (UInt32) bytes[ at + 2 ] << 8 ) | bytes[ at + 3 ];
        // the start of frame markers (not the tables DHT, JPG and DAC that lie between them)
        if ( marker >= 0xC0 && marker <= 0xCF && 0xC4 != marker && 0xC8 != marker && 0xCC != marker )
        {
            if ( at + 10 > size )
                return false;
            height = ( (UInt32) bytes[ at + 5 ] << 8 ) | bytes[ at + 6 ];
            width = ( (UInt32) bytes[ at + 7 ] << 8 ) | bytes[ at + 8 ];
            channels = bytes[ at + 9 ];
            return width > 0 && height > 0;
        }
        at += 2 + length;
    }
    return false;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace UNREAL */
}; /* namespace ASSETPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
