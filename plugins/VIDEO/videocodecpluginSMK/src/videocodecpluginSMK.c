/*
 *  videocodecpluginSMK: gucefVIDEO codec plugin for Smacker video
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

/*
 *  The Smacker format, as published:
 *
 *  Header (104 bytes, little endian): signature "SMK2" or "SMK4", width, height, frame count, frame rate
 *  (positive: milliseconds per frame, negative: units of 10 microseconds per frame, 0: 10 frames per second),
 *  flags (1 = a ring frame follows the last frame, 2 = Y interlaced, 4 = Y doubled), the largest audio chunk per
 *  track (7), the size of the Huffman tree data, four sizes for the trees, the audio rate and flags per track
 *  (7: bit 31 compressed, bit 30 present, bit 29 16 bit, bit 28 stereo, bits 26-27 a later compression, the
 *  low 24 bits the sample rate) and a reserved word.
 *  Then the frame sizes (the low 2 bits are flags), one type byte per frame (bit 0: a palette chunk, bit 1+n: an
 *  audio chunk of track n), and the tree data, read least significant bit first: four "big" Huffman trees for
 *  the 4x4 blocks of the picture (MMap: bit maps, MClr: colour pairs, Full: pixel pairs, Type: block types).
 *  A big tree has 16 bit leaves described with two 8 bit trees (low and high byte); three of the values are
 *  escapes that stand for a cache of the three last decoded values.
 *  A frame holds the palette chunk (length / 4 in its first byte; entries kept, copied from the previous
 *  palette, or new with 6 bit components), the audio chunks (length first), then the block data.
 *  Compressed audio is delta coded: per channel a start value, then deltas from 8 bit Huffman trees.
 *
 *  This plugin only uses the plugin interface headers and links nothing from GUCEF.
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <stdlib.h>
#include <string.h>

#ifndef GUCEF_CORE_C_LOGGING_H
#include "gucefCORE_c_logging.h"
#define GUCEF_CORE_C_LOGGING_H
#endif /* GUCEF_CORE_C_LOGGING_H ? */

#include "videocodecpluginSMK.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#define SMK_HEADER_SIZE             104
#define SMK_MAX_TRACKS              7
#define SMK_MAX_DIMENSION           4096
#define SMK_MAX_FRAMES              ( 1UL << 24 )
#define SMK_MAX_TREE_NODES          ( 1UL << 20 )

#define SMK_FLAG_RING_FRAME         0x01
#define SMK_FLAG_Y_INTERLACED       0x02
#define SMK_FLAG_Y_DOUBLED          0x04

#define SMK_AUDIO_PACKED            0x80000000UL
#define SMK_AUDIO_PRESENT           0x40000000UL
#define SMK_AUDIO_16BIT             0x20000000UL
#define SMK_AUDIO_STEREO            0x10000000UL
#define SMK_AUDIO_LATER_COMPRESSION 0x0C000000UL
#define SMK_AUDIO_RATE_MASK         0x00FFFFFFUL

#define SMK_BLOCK_MONO              0
#define SMK_BLOCK_FULL              1
#define SMK_BLOCK_SKIP              2
#define SMK_BLOCK_FILL              3

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

/* Reads bits least significant first. Past the end it gives zeros and remembers that it went there */
typedef struct SSmkBitReader
{
    const UInt8* data;
    UInt32 sizeInBits;
    UInt32 position;
    int overrun;
} TSmkBitReader;

/* A Huffman tree as nodes: a branch has two children, a leaf the index of its value slot */
typedef struct SSmkTree
{
    Int32* child0;              /* -1 for a leaf */
    Int32* child1;
    UInt32* slot;               /* leaves: the value slot */
    UInt32 nodeCount;
    UInt32 nodeCapacity;
    UInt16* values;             /* value slots; big trees have three extra slots for the cache when no escape leaf exists */
    UInt32 valueCount;
    UInt32 valueCapacity;
    UInt32 last[ 3 ];           /* big trees: the value slots that hold the cache of the last three values */
    int present;
} TSmkTree;

typedef struct SSmkTrack
{
    UInt32 smackerTrack;        /* 0 to 6, the bit in the frame type */
    UInt32 sampleRate;
    UInt32 channels;
    UInt32 bits;
    int packed;
} TSmkTrack;

typedef struct SSmkPlugin
{
    TGucefCoreCApi libApi;
} TSmkPlugin;

typedef struct SSmkSource
{
    TSmkPlugin* plugin;
    TIOAccess* input;
    UInt64 dataStart;           /* where the first frame is in the input */
    UInt64 inputEnd;            /* where the input ends, 0 when it cannot tell */
    int smk4;
    UInt32 width;
    UInt32 height;              /* lines as stored */
    UInt32 outputHeight;        /* lines as shown */
    UInt32 flags;
    UInt32 frameCount;
    UInt64* frameOffsets;       /* per frame, from the start of the input */
    UInt32* frameSizes;
    UInt8* frameTypes;
    TSmkTree trees[ 4 ];        /* MMap, MClr, Full, Type */
    TSmkTrack tracks[ SMK_MAX_TRACKS ];
    UInt32 trackCount;
    UInt32 trackOfSmackerTrack[ SMK_MAX_TRACKS ];   /* index in 'tracks', or SMK_MAX_TRACKS when not handed out */
    UInt8* pixels;              /* width x height palette indices, kept from frame to frame */
    UInt8 palette[ 768 ];
    UInt8 previousPalette[ 768 ];
    UInt8* frameData;
    UInt32 frameDataCapacity;
    UInt8* audioData;
    UInt32 audioDataCapacity;
    UInt32 position;            /* the next frame */
    int paletteDirty;           /* the next picture handed out reports a new palette (after a seek) */
} TSmkSource;

enum { SMK_TREE_MMAP = 0, SMK_TREE_MCLR = 1, SMK_TREE_FULL = 2, SMK_TREE_TYPE = 3 };

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

static const char* g_codecTypes[] = { "smk" };

/* The run lengths of the block types */
static const UInt32 g_blockRuns[ 64 ] =
{
     1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16,
    17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32,
    33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48,
    49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 128, 256, 512, 1024, 2048
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static UInt32
ReadLE32( const UInt8* bytes )
{
    return (UInt32) bytes[ 0 ] | ( (UInt32) bytes[ 1 ] << 8 ) | ( (UInt32) bytes[ 2 ] << 16 ) | ( (UInt32) bytes[ 3 ] << 24 );
}

/*-------------------------------------------------------------------------*/

static void
BitReaderInit( TSmkBitReader* reader, const UInt8* data, UInt32 size )
{
    reader->data = data;
    reader->sizeInBits = size > 0x1FFFFFFFUL ? 0xFFFFFFF8UL : size * 8;
    reader->position = 0;
    reader->overrun = 0;
}

/*-------------------------------------------------------------------------*/

static UInt32
ReadBit( TSmkBitReader* reader )
{
    UInt32 bit;
    if ( reader->position >= reader->sizeInBits )
    {
        reader->overrun = 1;
        return 0;
    }
    bit = ( reader->data[ reader->position >> 3 ] >> ( reader->position & 7 ) ) & 1;
    ++reader->position;
    return bit;
}

/*-------------------------------------------------------------------------*/

static UInt32
ReadBits( TSmkBitReader* reader, UInt32 count )
{
    UInt32 value = 0;
    UInt32 i;
    for ( i=0; i<count; ++i )
        value |= ReadBit( reader ) << i;
    return value;
}

/*-------------------------------------------------------------------------*/

static void
TreeFree( TSmkTree* tree )
{
    free( tree->child0 );
    free( tree->child1 );
    free( tree->slot );
    free( tree->values );
    memset( tree, 0, sizeof( TSmkTree ) );
}

/*-------------------------------------------------------------------------*/

/* A new node, -1 when there is no room. Children start unset */
static Int32
TreeAddNode( TSmkTree* tree )
{
    if ( tree->nodeCount >= tree->nodeCapacity )
    {
        UInt32 capacity = 0 == tree->nodeCapacity ? 64 : tree->nodeCapacity * 2;
        Int32* child0;
        Int32* child1;
        UInt32* slot;
        if ( capacity > SMK_MAX_TREE_NODES )
            return -1;
        child0 = (Int32*) realloc( tree->child0, capacity * sizeof( Int32 ) );
        if ( NULL != child0 )
            tree->child0 = child0;
        child1 = (Int32*) realloc( tree->child1, capacity * sizeof( Int32 ) );
        if ( NULL != child1 )
            tree->child1 = child1;
        slot = (UInt32*) realloc( tree->slot, capacity * sizeof( UInt32 ) );
        if ( NULL != slot )
            tree->slot = slot;
        if ( NULL == child0 || NULL == child1 || NULL == slot )
            return -1;
        tree->nodeCapacity = capacity;
    }
    tree->child0[ tree->nodeCount ] = -1;
    tree->child1[ tree->nodeCount ] = -1;
    tree->slot[ tree->nodeCount ] = 0;
    return (Int32) tree->nodeCount++;
}

/*-------------------------------------------------------------------------*/

/* A new value slot, -1 when there is no room */
static Int32
TreeAddValue( TSmkTree* tree, UInt16 value )
{
    if ( tree->valueCount >= tree->valueCapacity )
    {
        UInt32 capacity = 0 == tree->valueCapacity ? 64 : tree->valueCapacity * 2;
        UInt16* values;
        if ( capacity > SMK_MAX_TREE_NODES )
            return -1;
        values = (UInt16*) realloc( tree->values, capacity * sizeof( UInt16 ) );
        if ( NULL == values )
            return -1;
        tree->values = values;
        tree->valueCapacity = capacity;
    }
    tree->values[ tree->valueCount ] = value;
    return (Int32) tree->valueCount++;
}

/*-------------------------------------------------------------------------*/

/* Walks a tree with the bits of the reader, returns the value slot of the leaf it ends on */
static UInt32
TreeDecodeSlot( const TSmkTree* tree, TSmkBitReader* reader )
{
    Int32 node = 0;
    while ( tree->child0[ node ] >= 0 )
        node = ReadBit( reader ) ? tree->child1[ node ] : tree->child0[ node ];
    return tree->slot[ node ];
}

/*-------------------------------------------------------------------------*/

/* 8 bit trees: an absent tree gives 0 */
static UInt32
SmallTreeDecode( const TSmkTree* tree, TSmkBitReader* reader )
{
    if ( !tree->present )
        return 0;
    return tree->values[ TreeDecodeSlot( tree, reader ) ];
}

/*-------------------------------------------------------------------------*/

/*
 *  Reads the shape of a tree: bit 1 is a branch whose 0 side and then 1 side follow, bit 0 is a leaf whose value
 *  is read by 'readLeaf'. Iterative, so a hostile file cannot exhaust the stack. Returns 0 on failure.
 */
typedef int ( *TSmkReadLeaf )( void* context, TSmkTree* tree, Int32 node, TSmkBitReader* reader );

static int
TreeReadShape( TSmkTree* tree, TSmkBitReader* reader, TSmkReadLeaf readLeaf, void* context )
{
    /* the stack holds the branches whose 1 side is still to be read */
    Int32* pending = NULL;
    UInt32 pendingCount = 0;
    UInt32 pendingCapacity = 0;
    Int32 node = TreeAddNode( tree );
    int ok = node >= 0;

    while ( ok )
    {
        if ( ReadBit( reader ) )
        {
            /* a branch: read its 0 side now, remember its 1 side */
            Int32 child = TreeAddNode( tree );
            if ( child < 0 )
            {
                ok = 0;
                break;
            }
            if ( pendingCount >= pendingCapacity )
            {
                UInt32 capacity = 0 == pendingCapacity ? 64 : pendingCapacity * 2;
                Int32* grown = (Int32*) realloc( pending, capacity * sizeof( Int32 ) );
                if ( NULL == grown )
                {
                    ok = 0;
                    break;
                }
                pending = grown;
                pendingCapacity = capacity;
            }
            tree->child0[ node ] = child;
            pending[ pendingCount++ ] = node;
            node = child;
        }
        else
        {
            if ( !readLeaf( context, tree, node, reader ) )
            {
                ok = 0;
                break;
            }
            if ( 0 == pendingCount )
                break;
            {
                Int32 branch = pending[ --pendingCount ];
                Int32 child = TreeAddNode( tree );
                if ( child < 0 )
                {
                    ok = 0;
                    break;
                }
                tree->child1[ branch ] = child;
                node = child;
            }
        }
        if ( reader->overrun )
            ok = 0;
    }
    free( pending );
    return ok;
}

/*-------------------------------------------------------------------------*/

static int
ReadSmallLeaf( void* context, TSmkTree* tree, Int32 node, TSmkBitReader* reader )
{
    Int32 slot = TreeAddValue( tree, (UInt16) ReadBits( reader, 8 ) );
    if ( slot < 0 )
        return 0;
    tree->slot[ node ] = (UInt32) slot;
    return 1;
}

/*-------------------------------------------------------------------------*/

/* An 8 bit tree with its presence bit and, when present, the closing bit */
static int
SmallTreeRead( TSmkTree* tree, TSmkBitReader* reader )
{
    memset( tree, 0, sizeof( TSmkTree ) );
    if ( !ReadBit( reader ) )
        return !reader->overrun;
    tree->present = 1;
    if ( !TreeReadShape( tree, reader, ReadSmallLeaf, NULL ) )
        return 0;
    ReadBit( reader );
    return !reader->overrun;
}

/*-------------------------------------------------------------------------*/

typedef struct SSmkBigTreeContext
{
    TSmkTree low;
    TSmkTree high;
    UInt16 escapes[ 3 ];
} TSmkBigTreeContext;

static int
ReadBigLeaf( void* context, TSmkTree* tree, Int32 node, TSmkBitReader* reader )
{
    TSmkBigTreeContext* big = (TSmkBigTreeContext*) context;
    UInt32 value = SmallTreeDecode( &big->low, reader ) | ( SmallTreeDecode( &big->high, reader ) << 8 );
    Int32 slot = TreeAddValue( tree, (UInt16) value );
    UInt32 k;
    if ( slot < 0 )
        return 0;
    tree->slot[ node ] = (UInt32) slot;

    /* an escape value: this leaf is the cache slot of that escape, starting at 0 */
    for ( k=0; k<3; ++k )
    {
        if ( value == big->escapes[ k ] )
        {
            tree->last[ k ] = (UInt32) slot;
            tree->values[ slot ] = 0;
            break;
        }
    }
    return 1;
}

/*-------------------------------------------------------------------------*/

/* A big tree. Absent trees give 0 */
static int
BigTreeRead( TSmkTree* tree, TSmkBitReader* reader )
{
    TSmkBigTreeContext context;
    int ok;
    UInt32 k;

    memset( tree, 0, sizeof( TSmkTree ) );
    memset( &context, 0, sizeof( context ) );

    /* the three cache slots for escapes without a leaf come first; slot 0 is also the value of an absent tree */
    for ( k=0; k<3; ++k )
        tree->last[ k ] = (UInt32) TreeAddValue( tree, 0 );
    if ( tree->valueCount < 3 )
        return 0;

    if ( !ReadBit( reader ) )
    {
        /* absent: a single leaf with value 0 that is not one of the cache slots */
        Int32 node = TreeAddNode( tree );
        Int32 slot = TreeAddValue( tree, 0 );
        if ( node < 0 || slot < 0 )
            return 0;
        tree->slot[ node ] = (UInt32) slot;
        tree->present = 0;
        return !reader->overrun;
    }

    ok = SmallTreeRead( &context.low, reader ) && SmallTreeRead( &context.high, reader );
    if ( ok )
    {
        for ( k=0; k<3; ++k )
            context.escapes[ k ] = (UInt16) ReadBits( reader, 16 );
        ok = TreeReadShape( tree, reader, ReadBigLeaf, &context );
        ReadBit( reader );
        ok = ok && !reader->overrun;
    }
    tree->present = 1;
    TreeFree( &context.low );
    TreeFree( &context.high );
    return ok;
}

/*-------------------------------------------------------------------------*/

/* The cache of a big tree starts from 0 in every frame */
static void
BigTreeReset( TSmkTree* tree )
{
    tree->values[ tree->last[ 0 ] ] = 0;
    tree->values[ tree->last[ 1 ] ] = 0;
    tree->values[ tree->last[ 2 ] ] = 0;
}

/*-------------------------------------------------------------------------*/

static UInt32
BigTreeDecode( TSmkTree* tree, TSmkBitReader* reader )
{
    UInt32 value = tree->values[ TreeDecodeSlot( tree, reader ) ];
    UInt16* values = tree->values;
    if ( value != values[ tree->last[ 0 ] ] )
    {
        values[ tree->last[ 2 ] ] = values[ tree->last[ 1 ] ];
        values[ tree->last[ 1 ] ] = values[ tree->last[ 0 ] ];
        values[ tree->last[ 0 ] ] = (UInt16) value;
    }
    return value;
}

/*-------------------------------------------------------------------------*/

static void
SourceFree( TSmkSource* source )
{
    UInt32 i;
    if ( NULL == source )
        return;
    for ( i=0; i<4; ++i )
        TreeFree( &source->trees[ i ] );
    free( source->frameOffsets );
    free( source->frameSizes );
    free( source->frameTypes );
    free( source->pixels );
    free( source->frameData );
    free( source->audioData );
    free( source );
}

/*-------------------------------------------------------------------------*/

static int
ReadExact( TIOAccess* input, void* destination, UInt32 size )
{
    if ( 0 == size )
        return 1;
    return 1 == input->read( input, destination, size, 1 );
}

/*-------------------------------------------------------------------------*/

static UInt8*
EnsureBuffer( UInt8** buffer, UInt32* capacity, UInt32 size )
{
    if ( size > *capacity )
    {
        UInt8* grown = (UInt8*) realloc( *buffer, size );
        if ( NULL == grown )
            return NULL;
        *buffer = grown;
        *capacity = size;
    }
    return *buffer;
}

/*-------------------------------------------------------------------------*/

static void
UnpackPalette( TSmkSource* source, const UInt8* chunk, UInt32 size )
{
    UInt32 i = 0;
    UInt32 q = 0;

    /* copies come from the palette as it was before this chunk */
    memcpy( source->previousPalette, source->palette, sizeof( source->palette ) );
    while ( i < 256 && q < size )
    {
        UInt32 op = chunk[ q++ ];
        if ( op & 0x80 )
        {
            /* keep this many colours */
            i += ( op & 0x7F ) + 1;
        }
        else
        if ( op & 0x40 )
        {
            UInt32 from;
            UInt32 count = ( op & 0x3F ) + 1;
            if ( q >= size )
                break;
            from = chunk[ q++ ];
            if ( from + count > 256 )
                count = 256 - from;
            if ( i + count > 256 )
                count = 256 - i;
            memcpy( source->palette + i * 3, source->previousPalette + from * 3, count * 3 );
            i += count;
        }
        else
        {
            UInt32 r, g, b;
            if ( q + 2 > size )
                break;
            r = op & 0x3F;
            g = chunk[ q ] & 0x3F;
            b = chunk[ q + 1 ] & 0x3F;
            q += 2;
            source->palette[ i * 3 ] = (UInt8) ( ( r << 2 ) | ( r >> 4 ) );
            source->palette[ i * 3 + 1 ] = (UInt8) ( ( g << 2 ) | ( g >> 4 ) );
            source->palette[ i * 3 + 2 ] = (UInt8) ( ( b << 2 ) | ( b >> 4 ) );
            ++i;
        }
    }
}

/*-------------------------------------------------------------------------*/

/* Decodes the 4x4 blocks of a frame into source->pixels. Returns 0 for data that cannot be decoded */
static int
UnpackBlocks( TSmkSource* source, TSmkBitReader* reader )
{
    const UInt32 blocksPerRow = source->width / 4;
    const UInt32 blockCount = blocksPerRow * ( source->height / 4 );
    const UInt32 stride = source->width;
    UInt32 block = 0;
    UInt32 k;

    for ( k=0; k<4; ++k )
        BigTreeReset( &source->trees[ k ] );

    while ( block < blockCount )
    {
        UInt32 code = BigTreeDecode( &source->trees[ SMK_TREE_TYPE ], reader );
        UInt32 run = g_blockRuns[ ( code >> 2 ) & 0x3F ];
        UInt32 kind = code & 3;

        if ( reader->overrun )
            return 0;

        switch ( kind )
        {
            case SMK_BLOCK_MONO:
            {
                while ( run-- > 0 && block < blockCount )
                {
                    UInt32 colours = BigTreeDecode( &source->trees[ SMK_TREE_MCLR ], reader );
                    UInt32 map = BigTreeDecode( &source->trees[ SMK_TREE_MMAP ], reader );
                    UInt8 high = (UInt8) ( colours >> 8 );
                    UInt8 low = (UInt8) ( colours & 0xFF );
                    UInt8* out = source->pixels + ( block / blocksPerRow ) * 4 * stride + ( block % blocksPerRow ) * 4;
                    UInt32 row, column;
                    for ( row=0; row<4; ++row, out+=stride )
                        for ( column=0; column<4; ++column, map>>=1 )
                            out[ column ] = ( map & 1 ) ? high : low;
                    ++block;
                }
                break;
            }
            case SMK_BLOCK_FULL:
            {
                if ( source->smk4 && ( ReadBit( reader ) || ReadBit( reader ) ) )
                {
                    /* the two extra full block modes of SMK4 are not supported */
                    GUCEF_C_ERROR_LOG( source->plugin->libApi, GUCEF_LOGLEVEL_NORMAL, "videocodecpluginSMK: this SMK4 video uses full block modes that are not supported" );
                    return 0;
                }
                while ( run-- > 0 && block < blockCount )
                {
                    UInt8* out = source->pixels + ( block / blocksPerRow ) * 4 * stride + ( block % blocksPerRow ) * 4;
                    UInt32 row;
                    for ( row=0; row<4; ++row, out+=stride )
                    {
                        UInt32 pair = BigTreeDecode( &source->trees[ SMK_TREE_FULL ], reader );
                        out[ 2 ] = (UInt8) ( pair & 0xFF );
                        out[ 3 ] = (UInt8) ( pair >> 8 );
                        pair = BigTreeDecode( &source->trees[ SMK_TREE_FULL ], reader );
                        out[ 0 ] = (UInt8) ( pair & 0xFF );
                        out[ 1 ] = (UInt8) ( pair >> 8 );
                    }
                    ++block;
                }
                break;
            }
            case SMK_BLOCK_SKIP:
            {
                block += run < blockCount - block ? run : blockCount - block;
                break;
            }
            default:
            {
                UInt8 colour = (UInt8) ( code >> 8 );
                while ( run-- > 0 && block < blockCount )
                {
                    UInt8* out = source->pixels + ( block / blocksPerRow ) * 4 * stride + ( block % blocksPerRow ) * 4;
                    UInt32 row;
                    for ( row=0; row<4; ++row, out+=stride )
                        memset( out, colour, 4 );
                    ++block;
                }
                break;
            }
        }
        if ( reader->overrun )
            return 0;
    }
    return 1;
}

/*-------------------------------------------------------------------------*/

/* Decodes one audio chunk of a track and hands the frames to the host. Returns 0 for data that cannot be decoded */
static int
DecodeAudio( TSmkSource* source, UInt32 trackIndex, const UInt8* chunk, UInt32 size, const TVideoAudioOutput* audio )
{
    const TSmkTrack* track = &source->tracks[ trackIndex ];
    const UInt32 frameBytes = track->channels * ( track->bits / 8 );

    if ( !track->packed )
    {
        /* plain PCM, 16 bit is little endian in the file */
        UInt32 frames = size / frameBytes;
        UInt8* out;
        UInt32 i;
        if ( 0 == frames )
            return 1;
        out = EnsureBuffer( &source->audioData, &source->audioDataCapacity, frames * frameBytes );
        if ( NULL == out )
            return 0;
        if ( 16 == track->bits )
        {
            Int16* samples = (Int16*) out;
            for ( i=0; i<frames*track->channels; ++i )
                samples[ i ] = (Int16) (UInt16) ( chunk[ i * 2 ] | ( chunk[ i * 2 + 1 ] << 8 ) );
        }
        else
        {
            memcpy( out, chunk, frames * frameBytes );
        }
        audio->onAudio( audio->userData, trackIndex, out, frames );
        return 1;
    }
    else
    {
        TSmkBitReader reader;
        TSmkTree trees[ 4 ];
        UInt32 unpackedSize;
        UInt32 treeCount, stereo, sixteen, sampleCount, i, channels;
        Int32 predictors[ 2 ];
        UInt8* out;
        int ok = 1;

        if ( size < 4 )
            return 0;
        unpackedSize = ReadLE32( chunk );
        BitReaderInit( &reader, chunk + 4, size - 4 );
        if ( !ReadBit( &reader ) )
            return 1;                       /* no sound in this frame */
        stereo = ReadBit( &reader );
        sixteen = ReadBit( &reader );
        channels = stereo + 1;
        if ( channels != track->channels || ( sixteen ? 16 : 8 ) != track->bits )
        {
            GUCEF_C_ERROR_LOG( source->plugin->libApi, GUCEF_LOGLEVEL_NORMAL, "videocodecpluginSMK: an audio chunk does not have the format of its track" );
            return 0;
        }

        memset( trees, 0, sizeof( trees ) );
        treeCount = 1U << ( sixteen + stereo );
        for ( i=0; i<treeCount && ok; ++i )
            ok = SmallTreeRead( &trees[ i ], &reader );

        sampleCount = sixteen ? unpackedSize / 2 : unpackedSize;
        sampleCount -= sampleCount % channels;
        out = ok && sampleCount > 0 ? EnsureBuffer( &source->audioData, &source->audioDataCapacity, sampleCount * ( sixteen ? 2 : 1 ) ) : NULL;
        if ( ok && NULL != out )
        {
            Int32 ch;
            /* the start values, last channel first */
            for ( ch=(Int32)stereo; ch>=0; --ch )
            {
                if ( sixteen )
                {
                    UInt32 raw = ReadBits( &reader, 16 );
                    predictors[ ch ] = (Int32) (Int16) (UInt16) ( ( ( raw & 0xFF ) << 8 ) | ( raw >> 8 ) );
                }
                else
                {
                    predictors[ ch ] = (Int32) ReadBits( &reader, 8 );
                }
            }

            if ( sixteen )
            {
                Int16* samples = (Int16*) out;
                for ( i=0; i<channels && i<sampleCount; ++i )
                    samples[ i ] = (Int16) predictors[ i ];
                for ( ; i<sampleCount; ++i )
                {
                    UInt32 c = i & stereo;
                    UInt32 low = SmallTreeDecode( &trees[ c * 2 ], &reader );
                    UInt32 high = SmallTreeDecode( &trees[ c * 2 + 1 ], &reader );
                    Int32 value = predictors[ c ] + (Int32) (Int16) (UInt16) ( low | ( high << 8 ) );
                    if ( value > 32767 ) value = 32767;
                    if ( value < -32768 ) value = -32768;
                    predictors[ c ] = value;
                    samples[ i ] = (Int16) value;
                }
            }
            else
            {
                for ( i=0; i<channels && i<sampleCount; ++i )
                    out[ i ] = (UInt8) predictors[ i ];
                for ( ; i<sampleCount; ++i )
                {
                    UInt32 c = i & stereo;
                    Int32 value = predictors[ c ] + (Int32) (Int8) (UInt8) SmallTreeDecode( &trees[ c ], &reader );
                    if ( value > 255 ) value = 255;
                    if ( value < 0 ) value = 0;
                    predictors[ c ] = value;
                    out[ i ] = (UInt8) value;
                }
            }
            if ( reader.overrun )
            {
                GUCEF_C_ERROR_LOG( source->plugin->libApi, GUCEF_LOGLEVEL_NORMAL, "videocodecpluginSMK: an audio chunk is shorter than its sound" );
                ok = 0;
            }
            else
            {
                audio->onAudio( audio->userData, trackIndex, out, sampleCount / channels );
            }
        }
        for ( i=0; i<4; ++i )
            TreeFree( &trees[ i ] );
        return ok;
    }
}

/*-------------------------------------------------------------------------*/

/* Decodes frame source->position; 'picture' and 'audio' may be NULL (decoding only to get to a later frame) */
static int
DecodeFrame( TSmkSource* source, TVideoPictureBuffer* picture, const TVideoAudioOutput* audio )
{
    const UInt32 index = source->position;
    const UInt32 size = source->frameSizes[ index ];
    const UInt8 type = source->frameTypes[ index ];
    UInt8* data;
    UInt32 offset = 0;
    UInt32 track;
    int paletteChanged = 0 == index || source->paletteDirty;
    TSmkBitReader reader;

    if ( 0 != source->inputEnd && source->frameOffsets[ index ] + size > source->inputEnd )
    {
        GUCEF_C_ERROR_LOG( source->plugin->libApi, GUCEF_LOGLEVEL_NORMAL, "videocodecpluginSMK: a frame goes past the end of the file" );
        return 0;
    }
    data = EnsureBuffer( &source->frameData, &source->frameDataCapacity, size > 0 ? size : 1 );
    if ( NULL == data || 0 != source->input->setpos( source->input, source->frameOffsets[ index ] ) || !ReadExact( source->input, data, size ) )
    {
        GUCEF_C_ERROR_LOG( source->plugin->libApi, GUCEF_LOGLEVEL_NORMAL, "videocodecpluginSMK: a frame cannot be read, the file is shorter than its header says" );
        return 0;
    }

    if ( type & 1 )
    {
        UInt32 paletteSize = size > 0 ? (UInt32) data[ 0 ] * 4 : 0;
        if ( paletteSize < 1 || paletteSize > size )
        {
            GUCEF_C_ERROR_LOG( source->plugin->libApi, GUCEF_LOGLEVEL_NORMAL, "videocodecpluginSMK: a palette chunk does not fit its frame" );
            return 0;
        }
        UnpackPalette( source, data + 1, paletteSize - 1 );
        offset = paletteSize;
        paletteChanged = 1;
    }

    for ( track=0; track<SMK_MAX_TRACKS; ++track )
    {
        if ( type & ( 2 << track ) )
        {
            UInt32 chunkSize;
            if ( offset + 4 > size )
                return 0;
            chunkSize = ReadLE32( data + offset );
            if ( chunkSize < 4 || chunkSize > size - offset )
            {
                GUCEF_C_ERROR_LOG( source->plugin->libApi, GUCEF_LOGLEVEL_NORMAL, "videocodecpluginSMK: an audio chunk does not fit its frame" );
                return 0;
            }
            if ( NULL != audio && NULL != audio->onAudio && source->trackOfSmackerTrack[ track ] < SMK_MAX_TRACKS )
            {
                if ( !DecodeAudio( source, source->trackOfSmackerTrack[ track ], data + offset + 4, chunkSize - 4, audio ) )
                    return 0;
            }
            offset += chunkSize;
        }
    }

    BitReaderInit( &reader, data + offset, size - offset );
    if ( !UnpackBlocks( source, &reader ) )
    {
        GUCEF_C_ERROR_LOG( source->plugin->libApi, GUCEF_LOGLEVEL_NORMAL, "videocodecpluginSMK: the picture data of a frame cannot be decoded" );
        return 0;
    }

    if ( NULL != picture && NULL != picture->pixels )
    {
        UInt32 y;
        for ( y=0; y<source->outputHeight; ++y )
        {
            UInt8* line = (UInt8*) picture->pixels + (size_t) y * picture->pitch;
            if ( source->flags & SMK_FLAG_Y_DOUBLED )
                memcpy( line, source->pixels + (size_t) ( y / 2 ) * source->width, source->width );
            else
            if ( source->flags & SMK_FLAG_Y_INTERLACED )
            {
                if ( y & 1 )
                    memset( line, 0, source->width );
                else
                    memcpy( line, source->pixels + (size_t) ( y / 2 ) * source->width, source->width );
            }
            else
                memcpy( line, source->pixels + (size_t) y * source->width, source->width );
        }
        if ( NULL != picture->palette )
            memcpy( picture->palette, source->palette, sizeof( source->palette ) );
        picture->paletteChanged = paletteChanged ? 1 : 0;
        source->paletteDirty = 0;
    }

    ++source->position;
    return 1;
}

/*-------------------------------------------------------------------------*/

/* Back to before the first frame: an empty picture and palette */
static void
Rewind( TSmkSource* source )
{
    memset( source->pixels, 0, (size_t) source->width * source->height );
    memset( source->palette, 0, sizeof( source->palette ) );
    source->position = 0;
    source->paletteDirty = 1;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      PLUGIN API                                                         //
//                                                                         //
//-------------------------------------------------------------------------*/

UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
MODULE_API_FUNCTION( VIDEOCODECPLUG_Init )( void** plugdata, TGucefCoreCApi* libApi ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    TSmkPlugin* plugin = (TSmkPlugin*) calloc( 1, sizeof( TSmkPlugin ) );
    if ( NULL == plugin )
        return 0;
    if ( NULL != libApi )
        plugin->libApi = *libApi;
    *plugdata = plugin;

    GUCEF_C_LOG( plugin->libApi, GUCEF_LOGLEVEL_BELOW_NORMAL, "videocodecpluginSMK: initialized" );
    return 1;
}

/*-------------------------------------------------------------------------*/

void GUCEF_PLUGIN_CALLSPEC_PREFIX
MODULE_API_FUNCTION( VIDEOCODECPLUG_Shutdown )( void** plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    free( *plugdata );
    *plugdata = NULL;
}

/*-------------------------------------------------------------------------*/

const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
MODULE_API_FUNCTION( VIDEOCODECPLUG_Name )( const void* plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    return "gucefVIDEO codec plugin for Smacker video";
}

/*-------------------------------------------------------------------------*/

const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
MODULE_API_FUNCTION( VIDEOCODECPLUG_Copyright )( const void* plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    return "Copyright (C) Dinand Vanvelzen. Licensed under the Apache License, Version 2.0";
}

/*-------------------------------------------------------------------------*/

const TVersion* GUCEF_PLUGIN_CALLSPEC_PREFIX
MODULE_API_FUNCTION( VIDEOCODECPLUG_Version )( const void* plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    static TVersion version;
    version.major = 0;
    version.minor = 1;
    version.patch = 0;
    version.release = 0;
    return &version;
}

/*-------------------------------------------------------------------------*/

UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
MODULE_API_FUNCTION( VIDEOCODECPLUG_Type_Count )( const void* plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    return (UInt32) ( sizeof( g_codecTypes ) / sizeof( g_codecTypes[ 0 ] ) );
}

/*-------------------------------------------------------------------------*/

const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
MODULE_API_FUNCTION( VIDEOCODECPLUG_Type )( const void* plugdata, UInt32 index ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    if ( index >= sizeof( g_codecTypes ) / sizeof( g_codecTypes[ 0 ] ) )
        return NULL;
    return g_codecTypes[ index ];
}

/*-------------------------------------------------------------------------*/

UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
MODULE_API_FUNCTION( VIDEOCODECPLUG_Create_Codec )( void** plugdata, const char* codecType, void** codecdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    /* one format, no per codec state */
    *codecdata = NULL;
    return NULL != codecType && 0 == strcmp( codecType, "smk" ) ? 1 : 0;
}

/*-------------------------------------------------------------------------*/

void GUCEF_PLUGIN_CALLSPEC_PREFIX
MODULE_API_FUNCTION( VIDEOCODECPLUG_Destroy_Codec )( void** plugdata, void** codecdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    *codecdata = NULL;
}

/*-------------------------------------------------------------------------*/

UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
MODULE_API_FUNCTION( VIDEOCODECPLUG_Src_Open )( void** plugdata, void** codecdata, void** sourcedata, TIOAccess* input, const TVariantMapApi* options, TVideoFormatInfo* format, UInt64* frameCount ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    TSmkPlugin* plugin = (TSmkPlugin*) *plugdata;
    TSmkSource* source;
    UInt8 header[ SMK_HEADER_SIZE ];
    UInt32 tableCount, treeSize, i;
    Int32 frameRate;
    UInt64 offset;
    UInt8* treeData;
    TSmkBitReader reader;
    int ok;

    *sourcedata = NULL;
    if ( NULL == input || NULL == input->read || NULL == input->tell || NULL == input->setpos || NULL == format )
        return 0;
    if ( !ReadExact( input, header, SMK_HEADER_SIZE ) || 0 != memcmp( header, "SMK", 3 ) || ( '2' != header[ 3 ] && '4' != header[ 3 ] ) )
        return 0;

    source = (TSmkSource*) calloc( 1, sizeof( TSmkSource ) );
    if ( NULL == source )
        return 0;
    source->plugin = plugin;
    source->input = input;
    source->smk4 = '4' == header[ 3 ];
    source->width = ReadLE32( header + 4 );
    source->height = ReadLE32( header + 8 );
    source->frameCount = ReadLE32( header + 12 );
    frameRate = (Int32) ReadLE32( header + 16 );
    source->flags = ReadLE32( header + 20 );
    treeSize = ReadLE32( header + 52 );
    if ( NULL != input->size )
        source->inputEnd = input->size( input );
    source->outputHeight = ( source->flags & ( SMK_FLAG_Y_DOUBLED | SMK_FLAG_Y_INTERLACED ) ) ? source->height * 2 : source->height;

    if ( 0 == source->width || 0 == source->height || source->width > SMK_MAX_DIMENSION || source->height > SMK_MAX_DIMENSION ||
         source->frameCount > SMK_MAX_FRAMES )
    {
        GUCEF_C_ERROR_LOG( plugin->libApi, GUCEF_LOGLEVEL_NORMAL, "videocodecpluginSMK: the header has a size or frame count that is not plausible" );
        SourceFree( source );
        return 0;
    }

    /* audio tracks: the ones that are there and can be decoded are handed out, in their order */
    for ( i=0; i<SMK_MAX_TRACKS; ++i )
    {
        UInt32 rate = ReadLE32( header + 72 + i * 4 );
        source->trackOfSmackerTrack[ i ] = SMK_MAX_TRACKS;
        if ( 0 == ( rate & SMK_AUDIO_PRESENT ) )
            continue;
        if ( 0 != ( rate & SMK_AUDIO_LATER_COMPRESSION ) || 0 == ( rate & SMK_AUDIO_RATE_MASK ) )
        {
            GUCEF_C_LOG( plugin->libApi, GUCEF_LOGLEVEL_NORMAL, "videocodecpluginSMK: an audio track uses a compression that is not supported, it is left out" );
            continue;
        }
        source->tracks[ source->trackCount ].smackerTrack = i;
        source->tracks[ source->trackCount ].sampleRate = rate & SMK_AUDIO_RATE_MASK;
        source->tracks[ source->trackCount ].channels = ( rate & SMK_AUDIO_STEREO ) ? 2 : 1;
        source->tracks[ source->trackCount ].bits = ( rate & SMK_AUDIO_16BIT ) ? 16 : 8;
        source->tracks[ source->trackCount ].packed = 0 != ( rate & SMK_AUDIO_PACKED );
        source->trackOfSmackerTrack[ i ] = source->trackCount;
        ++source->trackCount;
    }

    /* the frame tables; the ring frame is in them but not handed out */
    tableCount = source->frameCount + ( ( source->flags & SMK_FLAG_RING_FRAME ) ? 1 : 0 );
    if ( 0 != source->inputEnd && input->tell( input ) + (UInt64) tableCount * 5 + treeSize > source->inputEnd )
    {
        GUCEF_C_ERROR_LOG( plugin->libApi, GUCEF_LOGLEVEL_NORMAL, "videocodecpluginSMK: the file is shorter than its header says" );
        SourceFree( source );
        return 0;
    }
    source->frameOffsets = (UInt64*) calloc( tableCount + 1, sizeof( UInt64 ) );
    source->frameSizes = (UInt32*) calloc( tableCount + 1, sizeof( UInt32 ) );
    source->frameTypes = (UInt8*) calloc( tableCount + 1, 1 );
    source->pixels = (UInt8*) calloc( (size_t) source->width * source->height, 1 );
    treeData = (UInt8*) malloc( treeSize > 0 ? treeSize : 1 );
    ok = NULL != source->frameOffsets && NULL != source->frameSizes && NULL != source->frameTypes && NULL != source->pixels && NULL != treeData;
    for ( i=0; ok && i<tableCount; ++i )
    {
        UInt8 bytes[ 4 ];
        ok = ReadExact( input, bytes, 4 );
        source->frameSizes[ i ] = ReadLE32( bytes ) & ~3UL;
    }
    ok = ok && ReadExact( input, source->frameTypes, tableCount ) && ReadExact( input, treeData, treeSize );
    if ( ok )
    {
        BitReaderInit( &reader, treeData, treeSize );
        for ( i=0; i<4 && ok; ++i )
            ok = BigTreeRead( &source->trees[ i ], &reader );
        if ( !ok )
            GUCEF_C_ERROR_LOG( plugin->libApi, GUCEF_LOGLEVEL_NORMAL, "videocodecpluginSMK: the Huffman trees cannot be read" );
    }
    free( treeData );
    if ( !ok )
    {
        SourceFree( source );
        return 0;
    }

    source->dataStart = input->tell( input );
    offset = source->dataStart;
    for ( i=0; i<tableCount; ++i )
    {
        source->frameOffsets[ i ] = offset;
        offset += source->frameSizes[ i ];
    }

    format->width = source->width;
    format->height = source->outputHeight;
    format->pixelFormat = GUCEF_VIDEOCODEC_PIXELFORMAT_PALETTE8;
    if ( frameRate > 0 )
    {
        format->frameRateNumerator = 1000;
        format->frameRateDenominator = (UInt32) frameRate;
    }
    else
    if ( frameRate < 0 )
    {
        format->frameRateNumerator = 100000;
        format->frameRateDenominator = (UInt32) -frameRate;
    }
    else
    {
        format->frameRateNumerator = 10;
        format->frameRateDenominator = 1;
    }
    format->audioTrackCount = source->trackCount;
    if ( NULL != frameCount )
        *frameCount = source->frameCount;

    *sourcedata = source;
    return 1;
}

/*-------------------------------------------------------------------------*/

UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
MODULE_API_FUNCTION( VIDEOCODECPLUG_Src_Get_Audio_Track_Format )( void** plugdata, void** codecdata, void** sourcedata, UInt32 track, TAudioFormatInfo* format ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    TSmkSource* source = (TSmkSource*) *sourcedata;
    if ( NULL == source || NULL == format || track >= source->trackCount )
        return 0;

    format->sampleRate = source->tracks[ track ].sampleRate;
    format->channelCount = source->tracks[ track ].channels;
    format->sampleType = 16 == source->tracks[ track ].bits ? GUCEF_AUDIOCODEC_SAMPLETYPE_INT16 : GUCEF_AUDIOCODEC_SAMPLETYPE_UINT8;
    format->channelLayout = 0;
    return 1;
}

/*-------------------------------------------------------------------------*/

UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
MODULE_API_FUNCTION( VIDEOCODECPLUG_Src_Decode_Frame )( void** plugdata, void** codecdata, void** sourcedata, TVideoPictureBuffer* picture, const TVideoAudioOutput* audio, UInt32* endOfStream ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    TSmkSource* source = (TSmkSource*) *sourcedata;
    if ( NULL == source || NULL == endOfStream )
        return 0;

    *endOfStream = 0;
    if ( source->position >= source->frameCount )
    {
        *endOfStream = 1;
        return 1;
    }
    return DecodeFrame( source, picture, audio ) ? 1 : 0;
}

/*-------------------------------------------------------------------------*/

UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
MODULE_API_FUNCTION( VIDEOCODECPLUG_Src_Seek_Frame )( void** plugdata, void** codecdata, void** sourcedata, UInt64 frame ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    TSmkSource* source = (TSmkSource*) *sourcedata;
    if ( NULL == source || frame > source->frameCount )
        return 0;

    /* every frame builds on the one before: decode from the start up to the frame, without the sound */
    if ( frame < source->position )
        Rewind( source );
    while ( source->position < frame )
    {
        if ( !DecodeFrame( source, NULL, NULL ) )
        {
            Rewind( source );
            return 0;
        }
    }
    source->paletteDirty = 1;
    return 1;
}

/*-------------------------------------------------------------------------*/

void GUCEF_PLUGIN_CALLSPEC_PREFIX
MODULE_API_FUNCTION( VIDEOCODECPLUG_Src_Close )( void** plugdata, void** codecdata, void** sourcedata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    SourceFree( (TSmkSource*) *sourcedata );
    *sourcedata = NULL;
}

/*-------------------------------------------------------------------------*/
