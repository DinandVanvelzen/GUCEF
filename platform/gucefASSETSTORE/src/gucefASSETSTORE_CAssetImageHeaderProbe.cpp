/*
 *  gucefASSETSTORE: GUCEF module for online asset stores and the primary asset store
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

#include "gucefASSETSTORE_CAssetImageHeaderProbe.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

UInt32 ReadBE16( const UInt8* p ) { return ( (UInt32) p[ 0 ] << 8 ) | p[ 1 ]; }
UInt32 ReadBE32( const UInt8* p ) { return ( (UInt32) p[ 0 ] << 24 ) | ( (UInt32) p[ 1 ] << 16 ) | ( (UInt32) p[ 2 ] << 8 ) | p[ 3 ]; }
UInt32 ReadLE16( const UInt8* p ) { return ( (UInt32) p[ 1 ] << 8 ) | p[ 0 ]; }
UInt32 ReadLE32( const UInt8* p ) { return ( (UInt32) p[ 3 ] << 24 ) | ( (UInt32) p[ 2 ] << 16 ) | ( (UInt32) p[ 1 ] << 8 ) | p[ 0 ]; }

void
Reset( CAssetImageHeaderProbe::SImageInfo& info )
{
    info.width = 0;
    info.height = 0;
    info.channels = 0;
    info.bitDepth = 0;
    info.compression.Clear();
    info.format.Clear();
}

bool
ProbePng( const UInt8* data, UInt32 size, CAssetImageHeaderProbe::SImageInfo& info )
{
    static const UInt8 signature[ 8 ] = { 0x89, 'P', 'N', 'G', 0x0D, 0x0A, 0x1A, 0x0A };
    if ( size < 29 || 0 != memcmp( data, signature, 8 ) || 0 != memcmp( data + 12, "IHDR", 4 ) )
        return false;
    info.width = ReadBE32( data + 16 );
    info.height = ReadBE32( data + 20 );
    const UInt32 bitDepth = data[ 24 ];
    switch ( data[ 25 ] )
    {
        case 0: info.channels = 1; info.bitDepth = bitDepth; break;
        case 2: info.channels = 3; info.bitDepth = bitDepth; break;
        case 3: info.channels = 3; info.bitDepth = 8; break;      // palette entries are 8 bit RGB
        case 4: info.channels = 2; info.bitDepth = bitDepth; break;
        case 6: info.channels = 4; info.bitDepth = bitDepth; break;
        default: return false;
    }
    info.compression = "png";
    info.format = "png";
    return info.width > 0 && info.height > 0;
}

bool
ProbeJpeg( const UInt8* data, UInt32 size, CAssetImageHeaderProbe::SImageInfo& info )
{
    if ( size < 4 || 0xFF != data[ 0 ] || 0xD8 != data[ 1 ] )
        return false;
    UInt32 offset = 2;
    while ( offset + 4 <= size )
    {
        if ( 0xFF != data[ offset ] )
            return false;
        const UInt8 marker = data[ offset+1 ];
        if ( 0xFF == marker )
        {
            ++offset;
            continue;
        }
        if ( 0xD8 == marker || 0x01 == marker || ( marker >= 0xD0 && marker <= 0xD7 ) )
        {
            offset += 2;
            continue;
        }
        const UInt32 length = ReadBE16( data + offset + 2 );
        const bool isFrame = marker >= 0xC0 && marker <= 0xCF && 0xC4 != marker && 0xC8 != marker && 0xCC != marker;
        if ( isFrame )
        {
            if ( offset + 10 > size )
                return false;
            info.bitDepth = data[ offset+4 ];
            info.height = ReadBE16( data + offset + 5 );
            info.width = ReadBE16( data + offset + 7 );
            info.channels = data[ offset+9 ];
            info.compression = "jpeg";
            info.format = "jpeg";
            return info.width > 0 && info.height > 0;
        }
        if ( length < 2 )
            return false;
        offset += 2 + length;
    }
    return false;
}

bool
ProbeBmp( const UInt8* data, UInt32 size, CAssetImageHeaderProbe::SImageInfo& info )
{
    if ( size < 30 || 'B' != data[ 0 ] || 'M' != data[ 1 ] )
        return false;
    const Int32 width = (Int32) ReadLE32( data + 18 );
    const Int32 height = (Int32) ReadLE32( data + 22 );
    const UInt32 bitsPerPixel = ReadLE16( data + 28 );
    info.width = (UInt32) ( width < 0 ? -width : width );
    info.height = (UInt32) ( height < 0 ? -height : height );
    info.channels = 32 == bitsPerPixel ? 4 : ( bitsPerPixel <= 8 ? 3 : 3 );
    info.bitDepth = 8;
    info.compression = "none";
    info.format = "bmp";
    return info.width > 0 && info.height > 0;
}

bool
ProbeGif( const UInt8* data, UInt32 size, CAssetImageHeaderProbe::SImageInfo& info )
{
    if ( size < 10 || 0 != memcmp( data, "GIF8", 4 ) )
        return false;
    info.width = ReadLE16( data + 6 );
    info.height = ReadLE16( data + 8 );
    info.channels = 3;
    info.bitDepth = 8;
    info.compression = "lzw";
    info.format = "gif";
    return info.width > 0 && info.height > 0;
}

bool
ProbeDds( const UInt8* data, UInt32 size, CAssetImageHeaderProbe::SImageInfo& info )
{
    if ( size < 128 || 0 != memcmp( data, "DDS ", 4 ) || 124 != ReadLE32( data + 4 ) )
        return false;
    info.height = ReadLE32( data + 12 );
    info.width = ReadLE32( data + 16 );
    info.format = "dds";
    const UInt32 pixelFlags = ReadLE32( data + 80 );
    if ( 0 != ( pixelFlags & 0x4 ) )
    {
        const char* fourCC = (const char*) data + 84;
        info.bitDepth = 8;
        info.channels = 4;
        if ( 0 == memcmp( fourCC, "DXT1", 4 ) ) { info.compression = "bc1"; info.channels = 3; }
        else if ( 0 == memcmp( fourCC, "DXT3", 4 ) ) info.compression = "bc2";
        else if ( 0 == memcmp( fourCC, "DXT5", 4 ) ) info.compression = "bc3";
        else if ( 0 == memcmp( fourCC, "ATI1", 4 ) || 0 == memcmp( fourCC, "BC4U", 4 ) ) { info.compression = "bc4"; info.channels = 1; }
        else if ( 0 == memcmp( fourCC, "ATI2", 4 ) || 0 == memcmp( fourCC, "BC5U", 4 ) ) { info.compression = "bc5"; info.channels = 2; }
        else if ( 0 == memcmp( fourCC, "DX10", 4 ) && size >= 148 )
        {
            const UInt32 dxgi = ReadLE32( data + 128 );
            if ( dxgi >= 70 && dxgi <= 72 ) { info.compression = "bc1"; info.channels = 3; }
            else if ( dxgi >= 73 && dxgi <= 75 ) info.compression = "bc2";
            else if ( dxgi >= 76 && dxgi <= 78 ) info.compression = "bc3";
            else if ( dxgi >= 79 && dxgi <= 81 ) { info.compression = "bc4"; info.channels = 1; }
            else if ( dxgi >= 82 && dxgi <= 84 ) { info.compression = "bc5"; info.channels = 2; }
            else if ( dxgi >= 94 && dxgi <= 96 ) { info.compression = "bc6h"; info.channels = 3; info.bitDepth = 16; }
            else if ( dxgi >= 97 && dxgi <= 99 ) info.compression = "bc7";
            else info.compression = "none";
        }
        else info.compression = "other";
    }
    else
    {
        const UInt32 bits = ReadLE32( data + 88 );
        info.compression = "none";
        info.bitDepth = 8;
        info.channels = bits >= 32 ? 4 : ( bits >= 24 ? 3 : ( bits >= 16 ? 2 : 1 ) );
    }
    return info.width > 0 && info.height > 0;
}

bool
ProbeTga( const UInt8* data, UInt32 size, CAssetImageHeaderProbe::SImageInfo& info )
{
    if ( size < 18 )
        return false;
    const UInt8 colourMapType = data[ 1 ];
    const UInt8 imageType = data[ 2 ];
    const UInt32 bitsPerPixel = data[ 16 ];
    const bool knownType = 1 == imageType || 2 == imageType || 3 == imageType || 9 == imageType || 10 == imageType || 11 == imageType;
    if ( colourMapType > 1 || !knownType || ( 8 != bitsPerPixel && 15 != bitsPerPixel && 16 != bitsPerPixel && 24 != bitsPerPixel && 32 != bitsPerPixel ) )
        return false;
    info.width = ReadLE16( data + 12 );
    info.height = ReadLE16( data + 14 );
    info.channels = ( 3 == imageType || 11 == imageType ) ? 1 : ( 32 == bitsPerPixel ? 4 : 3 );
    info.bitDepth = 8;
    info.compression = imageType >= 9 ? "rle" : "none";
    info.format = "tga";
    return info.width > 0 && info.height > 0;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

bool
CAssetImageHeaderProbe::Probe( const UInt8* data, UInt32 size, SImageInfo& info )
{GUCEF_TRACE;

    Reset( info );
    if ( GUCEF_NULL == data )
        return false;
    if ( ProbePng( data, size, info ) || ProbeJpeg( data, size, info ) || ProbeDds( data, size, info ) ||
         ProbeGif( data, size, info ) || ProbeBmp( data, size, info ) )
        return true;
    Reset( info );
    if ( ProbeTga( data, size, info ) )        // no signature: tried last
        return true;
    Reset( info );
    return false;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
