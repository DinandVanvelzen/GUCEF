/*
 *  AssetWorkbenchLib: decoder of the block compressed formats BC1 to BC5 (DXT1 to DXT5, ATI1, ATI2)
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

#include "AssetWorkbenchLib_CBlockDecoder.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETWORKBENCH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

UInt32
GetBlockBytes( Ogre::PixelFormat format )
{
    switch ( format )
    {
        case Ogre::PF_DXT1:
        case Ogre::PF_BC4_UNORM:
        case Ogre::PF_BC4_SNORM:    return 8;
        case Ogre::PF_DXT2:
        case Ogre::PF_DXT3:
        case Ogre::PF_DXT4:
        case Ogre::PF_DXT5:
        case Ogre::PF_BC5_UNORM:
        case Ogre::PF_BC5_SNORM:    return 16;
        default:                    return 0;
    }
}

void
Expand565( UInt16 colour, UInt8* rgb )
{
    const UInt32 r = ( colour >> 11 ) & 0x1F;
    const UInt32 g = ( colour >> 5 ) & 0x3F;
    const UInt32 b = colour & 0x1F;
    rgb[ 0 ] = (UInt8) ( ( r << 3 ) | ( r >> 2 ) );
    rgb[ 1 ] = (UInt8) ( ( g << 2 ) | ( g >> 4 ) );
    rgb[ 2 ] = (UInt8) ( ( b << 3 ) | ( b >> 2 ) );
}

UInt16
Read16( const UInt8* p )
{
    return (UInt16) ( p[ 0 ] | ( p[ 1 ] << 8 ) );
}

/** The 4 x 4 colour block of BC1 (also the colour half of BC2 and BC3): 'opaqueOnly' keeps the 4 colour mode, as BC2 and BC3 have it */
void
DecodeColourBlock( const UInt8* block, bool allowTransparent, UInt8 out[ 16 ][ 4 ] )
{
    const UInt16 c0 = Read16( block );
    const UInt16 c1 = Read16( block + 2 );
    UInt8 palette[ 4 ][ 4 ];
    Expand565( c0, palette[ 0 ] );
    Expand565( c1, palette[ 1 ] );
    palette[ 0 ][ 3 ] = palette[ 1 ][ 3 ] = palette[ 2 ][ 3 ] = 255;
    palette[ 3 ][ 3 ] = 255;
    if ( c0 > c1 || !allowTransparent )
    {
        for ( UInt32 c=0; c<3; ++c )
        {
            palette[ 2 ][ c ] = (UInt8) ( ( 2 * palette[ 0 ][ c ] + palette[ 1 ][ c ] ) / 3 );
            palette[ 3 ][ c ] = (UInt8) ( ( palette[ 0 ][ c ] + 2 * palette[ 1 ][ c ] ) / 3 );
        }
    }
    else
    {
        for ( UInt32 c=0; c<3; ++c )
        {
            palette[ 2 ][ c ] = (UInt8) ( ( palette[ 0 ][ c ] + palette[ 1 ][ c ] ) / 2 );
            palette[ 3 ][ c ] = 0;
        }
        palette[ 3 ][ 3 ] = 0;
    }
    const UInt32 bits = (UInt32) block[ 4 ] | ( (UInt32) block[ 5 ] << 8 ) | ( (UInt32) block[ 6 ] << 16 ) | ( (UInt32) block[ 7 ] << 24 );
    for ( UInt32 i=0; i<16; ++i )
    {
        const UInt32 index = ( bits >> ( 2 * i ) ) & 3;
        for ( UInt32 c=0; c<4; ++c )
            out[ i ][ c ] = palette[ index ][ c ];
    }
}

/** The 4 x 4 block of one channel of BC3 alpha or BC4 and BC5 */
void
DecodeChannelBlock( const UInt8* block, bool isSigned, UInt8 out[ 16 ] )
{
    // the signed variant is read as unsigned around the middle: -127 to 127 becomes 0 to 255
    Int32 a0 = isSigned ? (Int32) (Int8) block[ 0 ] : (Int32) block[ 0 ];
    Int32 a1 = isSigned ? (Int32) (Int8) block[ 1 ] : (Int32) block[ 1 ];
    Int32 palette[ 8 ];
    palette[ 0 ] = a0;
    palette[ 1 ] = a1;
    if ( a0 > a1 )
    {
        for ( Int32 i=1; i<7; ++i )
            palette[ 1 + i ] = ( ( 7 - i ) * a0 + i * a1 ) / 7;
    }
    else
    {
        for ( Int32 i=1; i<5; ++i )
            palette[ 1 + i ] = ( ( 5 - i ) * a0 + i * a1 ) / 5;
        palette[ 6 ] = isSigned ? -127 : 0;
        palette[ 7 ] = isSigned ? 127 : 255;
    }
    UInt64 bits = 0;
    for ( UInt32 i=0; i<6; ++i )
        bits |= (UInt64) block[ 2 + i ] << ( 8 * i );
    for ( UInt32 i=0; i<16; ++i )
    {
        Int32 value = palette[ ( bits >> ( 3 * i ) ) & 7 ];
        if ( isSigned )
            value = ( value + 127 ) * 255 / 254;
        out[ i ] = (UInt8) ( value < 0 ? 0 : ( value > 255 ? 255 : value ) );
    }
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

bool
CBlockDecoder::CanDecode( Ogre::PixelFormat format )
{GUCEF_TRACE;

    return GetBlockBytes( format ) > 0;
}

/*-------------------------------------------------------------------------*/

UInt32
CBlockDecoder::GetBlocksSize( Ogre::PixelFormat format, UInt32 width, UInt32 height )
{GUCEF_TRACE;

    return ( ( width + 3 ) / 4 ) * ( ( height + 3 ) / 4 ) * GetBlockBytes( format );
}

/*-------------------------------------------------------------------------*/

bool
CBlockDecoder::Decode( Ogre::PixelFormat format, const void* blocks, UInt32 width, UInt32 height, UInt8* rgba )
{GUCEF_TRACE;

    const UInt32 blockBytes = GetBlockBytes( format );
    if ( 0 == blockBytes || GUCEF_NULL == blocks || GUCEF_NULL == rgba )
        return false;

    const UInt8* source = static_cast< const UInt8* >( blocks );
    const UInt32 blocksX = ( width + 3 ) / 4;
    const UInt32 blocksY = ( height + 3 ) / 4;
    for ( UInt32 by=0; by<blocksY; ++by )
    {
        for ( UInt32 bx=0; bx<blocksX; ++bx )
        {
            const UInt8* block = source + ( (size_t) by * blocksX + bx ) * blockBytes;
            UInt8 pixels[ 16 ][ 4 ];
            switch ( format )
            {
                case Ogre::PF_DXT1:
                {
                    DecodeColourBlock( block, true, pixels );
                    break;
                }
                case Ogre::PF_DXT2:
                case Ogre::PF_DXT3:
                {
                    DecodeColourBlock( block + 8, false, pixels );
                    for ( UInt32 i=0; i<16; ++i )
                    {
                        const UInt32 nibble = ( block[ i / 2 ] >> ( 4 * ( i & 1 ) ) ) & 0xF;
                        pixels[ i ][ 3 ] = (UInt8) ( nibble * 17 );
                    }
                    break;
                }
                case Ogre::PF_DXT4:
                case Ogre::PF_DXT5:
                {
                    DecodeColourBlock( block + 8, false, pixels );
                    UInt8 alpha[ 16 ];
                    DecodeChannelBlock( block, false, alpha );
                    for ( UInt32 i=0; i<16; ++i )
                        pixels[ i ][ 3 ] = alpha[ i ];
                    break;
                }
                case Ogre::PF_BC4_UNORM:
                case Ogre::PF_BC4_SNORM:
                {
                    UInt8 grey[ 16 ];
                    DecodeChannelBlock( block, Ogre::PF_BC4_SNORM == format, grey );
                    for ( UInt32 i=0; i<16; ++i )
                    {
                        pixels[ i ][ 0 ] = pixels[ i ][ 1 ] = pixels[ i ][ 2 ] = grey[ i ];
                        pixels[ i ][ 3 ] = 255;
                    }
                    break;
                }
                default:
                {
                    UInt8 red[ 16 ], green[ 16 ];
                    DecodeChannelBlock( block, Ogre::PF_BC5_SNORM == format, red );
                    DecodeChannelBlock( block + 8, Ogre::PF_BC5_SNORM == format, green );
                    for ( UInt32 i=0; i<16; ++i )
                    {
                        pixels[ i ][ 0 ] = red[ i ];
                        pixels[ i ][ 1 ] = green[ i ];
                        pixels[ i ][ 2 ] = 0;
                        pixels[ i ][ 3 ] = 255;
                    }
                    break;
                }
            }

            // the pixels of the block that are inside of the picture
            for ( UInt32 y=0; y<4; ++y )
            {
                for ( UInt32 x=0; x<4; ++x )
                {
                    const UInt32 px = bx * 4 + x;
                    const UInt32 py = by * 4 + y;
                    if ( px >= width || py >= height )
                        continue;
                    UInt8* target = rgba + ( (size_t) py * width + px ) * 4;
                    for ( UInt32 c=0; c<4; ++c )
                        target[ c ] = pixels[ y * 4 + x ][ c ];
                }
            }
        }
    }
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETWORKBENCH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
