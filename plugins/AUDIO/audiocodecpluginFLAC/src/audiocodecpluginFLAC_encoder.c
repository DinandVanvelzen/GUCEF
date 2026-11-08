/*
 *  audiocodecpluginFLAC: gucefAUDIO codec plugin for FLAC audio
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
 *  A small FLAC encoder (RFC 9639): STREAMINFO with the MD5 of the audio, an optional VORBIS_COMMENT, and frames
 *  with fixed blocks of independent channels. Each sub frame is constant, a fixed predictor of order 0 to 4 with
 *  Rice coded residual (the order and the partitioning that are smallest), or verbatim when nothing is smaller.
 *  It is meant for lossless export and for tests that need FLAC data without stored files. It does not try to
 *  match the compression of the reference encoder.
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

#include "audiocodecpluginFLAC_internal.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#define FLACENC_DEFAULT_BLOCKSIZE   4096
#define FLACENC_MAX_ORDER           4
#define FLACENC_MAX_PARTITION_ORDER 8
#define FLACENC_MD5_CHUNK_FRAMES    4096
#define FLACENC_VENDOR              "GUCEF audiocodecpluginFLAC"

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

/** Writes bits, most significant first, into a buffer that was sized for the worst case */
typedef struct SFlacBitWriter
{
    UInt8* buffer;
    size_t capacity;
    size_t size;
    UInt64 accumulator;
    UInt32 bitCount;
    int isOverflown;
} TFlacBitWriter;

typedef struct SFlacMd5
{
    UInt32 state[ 4 ];
    UInt64 byteCount;
    UInt8 block[ 64 ];
} TFlacMd5;

typedef struct SFlacEncoder
{
    UInt32 channels;
    UInt32 sampleRate;
    UInt32 sampleType;
    UInt32 bits;                /* bits per sample in the file */
    UInt32 blockSize;
    UInt16 crc16Table[ 256 ];
    UInt8 crc8Table[ 256 ];
} TFlacEncoder;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static int
IsHostBigEndian( void )
{
    const UInt16 probe = 0x0102;
    return 0x01 == *( (const UInt8*) &probe );
}

/*-------------------------------------------------------------------------*/

static void
BuildCrcTables( TFlacEncoder* enc )
{
    UInt32 i, n;
    for ( i=0; i<256; ++i )
    {
        UInt32 crc8 = i;
        UInt32 crc16 = i << 8;
        for ( n=0; n<8; ++n )
        {
            crc8 = ( crc8 & 0x80 ) ? ( ( crc8 << 1 ) ^ 0x07 ) : ( crc8 << 1 );
            crc16 = ( crc16 & 0x8000 ) ? ( ( crc16 << 1 ) ^ 0x8005 ) : ( crc16 << 1 );
        }
        enc->crc8Table[ i ] = (UInt8) ( crc8 & 0xFF );
        enc->crc16Table[ i ] = (UInt16) ( crc16 & 0xFFFF );
    }
}

/*-------------------------------------------------------------------------*/

static UInt8
Crc8( const TFlacEncoder* enc, const UInt8* data, size_t size )
{
    UInt8 crc = 0;
    size_t i;
    for ( i=0; i<size; ++i )
        crc = enc->crc8Table[ crc ^ data[ i ] ];
    return crc;
}

/*-------------------------------------------------------------------------*/

static UInt16
Crc16( const TFlacEncoder* enc, const UInt8* data, size_t size )
{
    UInt16 crc = 0;
    size_t i;
    for ( i=0; i<size; ++i )
        crc = (UInt16) ( ( crc << 8 ) ^ enc->crc16Table[ ( ( crc >> 8 ) ^ data[ i ] ) & 0xFF ] );
    return crc;
}

/*-------------------------------------------------------------------------*/

static void
BwInit( TFlacBitWriter* bw, UInt8* buffer, size_t capacity )
{
    bw->buffer = buffer;
    bw->capacity = capacity;
    bw->size = 0;
    bw->accumulator = 0;
    bw->bitCount = 0;
    bw->isOverflown = 0;
}

/*-------------------------------------------------------------------------*/

/** Writes the low 'count' bits of 'value', count 0 to 32 */
static void
BwPut( TFlacBitWriter* bw, UInt32 value, UInt32 count )
{
    if ( 0 == count )
        return;
    if ( count < 32 )
        value &= ( 1UL << count ) - 1;
    bw->accumulator = ( bw->accumulator << count ) | value;
    bw->bitCount += count;
    while ( bw->bitCount >= 8 )
    {
        if ( bw->size >= bw->capacity )
        {
            bw->isOverflown = 1;
            bw->bitCount = 0;
            return;
        }
        bw->buffer[ bw->size++ ] = (UInt8) ( ( bw->accumulator >> ( bw->bitCount - 8 ) ) & 0xFF );
        bw->bitCount -= 8;
    }
}

/*-------------------------------------------------------------------------*/

/** Writes 'quotient' zero bits and a one */
static void
BwPutUnary( TFlacBitWriter* bw, UInt32 quotient )
{
    while ( quotient >= 31 )
    {
        BwPut( bw, 0, 31 );
        quotient -= 31;
    }
    BwPut( bw, 1, quotient + 1 );
}

/*-------------------------------------------------------------------------*/

static void
BwAlign( TFlacBitWriter* bw )
{
    if ( 0 != bw->bitCount % 8 )
        BwPut( bw, 0, 8 - ( bw->bitCount % 8 ) );
}

/*-------------------------------------------------------------------------*/

/** A signed value as 'count' bits of two's complement */
static void
BwPutSigned( TFlacBitWriter* bw, Int32 value, UInt32 count )
{
    BwPut( bw, (UInt32) value, count );
}

/*-------------------------------------------------------------------------*/

static void
Md5Init( TFlacMd5* md5 )
{
    md5->state[ 0 ] = 0x67452301UL;
    md5->state[ 1 ] = 0xefcdab89UL;
    md5->state[ 2 ] = 0x98badcfeUL;
    md5->state[ 3 ] = 0x10325476UL;
    md5->byteCount = 0;
}

/*-------------------------------------------------------------------------*/

#define MD5_ROL( x, n ) ( ( ( ( x ) << ( n ) ) | ( ( x ) >> ( 32 - ( n ) ) ) ) & 0xFFFFFFFFUL )

static void
Md5Block( TFlacMd5* md5, const UInt8* data )
{
    static const UInt32 k[ 64 ] = {
        0xd76aa478UL, 0xe8c7b756UL, 0x242070dbUL, 0xc1bdceeeUL, 0xf57c0fafUL, 0x4787c62aUL, 0xa8304613UL, 0xfd469501UL,
        0x698098d8UL, 0x8b44f7afUL, 0xffff5bb1UL, 0x895cd7beUL, 0x6b901122UL, 0xfd987193UL, 0xa679438eUL, 0x49b40821UL,
        0xf61e2562UL, 0xc040b340UL, 0x265e5a51UL, 0xe9b6c7aaUL, 0xd62f105dUL, 0x02441453UL, 0xd8a1e681UL, 0xe7d3fbc8UL,
        0x21e1cde6UL, 0xc33707d6UL, 0xf4d50d87UL, 0x455a14edUL, 0xa9e3e905UL, 0xfcefa3f8UL, 0x676f02d9UL, 0x8d2a4c8aUL,
        0xfffa3942UL, 0x8771f681UL, 0x6d9d6122UL, 0xfde5380cUL, 0xa4beea44UL, 0x4bdecfa9UL, 0xf6bb4b60UL, 0xbebfbc70UL,
        0x289b7ec6UL, 0xeaa127faUL, 0xd4ef3085UL, 0x04881d05UL, 0xd9d4d039UL, 0xe6db99e5UL, 0x1fa27cf8UL, 0xc4ac5665UL,
        0xf4292244UL, 0x432aff97UL, 0xab9423a7UL, 0xfc93a039UL, 0x655b59c3UL, 0x8f0ccc92UL, 0xffeff47dUL, 0x85845dd1UL,
        0x6fa87e4fUL, 0xfe2ce6e0UL, 0xa3014314UL, 0x4e0811a1UL, 0xf7537e82UL, 0xbd3af235UL, 0x2ad7d2bbUL, 0xeb86d391UL };
    static const UInt8 s[ 64 ] = {
        7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22,
        5,  9, 14, 20, 5,  9, 14, 20, 5,  9, 14, 20, 5,  9, 14, 20,
        4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23,
        6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21 };
    UInt32 m[ 16 ];
    UInt32 a = md5->state[ 0 ], b = md5->state[ 1 ], c = md5->state[ 2 ], d = md5->state[ 3 ];
    UInt32 i;

    for ( i=0; i<16; ++i )
        m[ i ] = (UInt32) data[ i*4 ] | ( (UInt32) data[ i*4+1 ] << 8 ) | ( (UInt32) data[ i*4+2 ] << 16 ) | ( (UInt32) data[ i*4+3 ] << 24 );

    for ( i=0; i<64; ++i )
    {
        UInt32 f, g, t;
        if ( i < 16 )      { f = ( b & c ) | ( ~b & d ); g = i; }
        else if ( i < 32 ) { f = ( d & b ) | ( ~d & c ); g = ( 5*i + 1 ) % 16; }
        else if ( i < 48 ) { f = b ^ c ^ d;              g = ( 3*i + 5 ) % 16; }
        else               { f = c ^ ( b | ( ~d & 0xFFFFFFFFUL ) ); g = ( 7*i ) % 16; }
        t = d;
        d = c;
        c = b;
        b = ( b + MD5_ROL( ( a + f + k[ i ] + m[ g ] ) & 0xFFFFFFFFUL, s[ i ] ) ) & 0xFFFFFFFFUL;
        a = t;
    }
    md5->state[ 0 ] = ( md5->state[ 0 ] + a ) & 0xFFFFFFFFUL;
    md5->state[ 1 ] = ( md5->state[ 1 ] + b ) & 0xFFFFFFFFUL;
    md5->state[ 2 ] = ( md5->state[ 2 ] + c ) & 0xFFFFFFFFUL;
    md5->state[ 3 ] = ( md5->state[ 3 ] + d ) & 0xFFFFFFFFUL;
}

/*-------------------------------------------------------------------------*/

static void
Md5Update( TFlacMd5* md5, const UInt8* data, size_t size )
{
    while ( size > 0 )
    {
        UInt32 used = (UInt32) ( md5->byteCount % 64 );
        size_t take = 64 - used;
        if ( take > size )
            take = size;
        memcpy( md5->block + used, data, take );
        md5->byteCount += take;
        data += take;
        size -= take;
        if ( 64 == used + take )
            Md5Block( md5, md5->block );
    }
}

/*-------------------------------------------------------------------------*/

static void
Md5Final( TFlacMd5* md5, UInt8* digest )
{
    UInt8 padding[ 72 ];
    UInt64 bitCount = md5->byteCount * 8;
    UInt32 used = (UInt32) ( md5->byteCount % 64 );
    UInt32 padSize = ( used < 56 ) ? 56 - used : 120 - used;
    UInt32 i;

    memset( padding, 0, sizeof( padding ) );
    padding[ 0 ] = 0x80;
    for ( i=0; i<8; ++i )
        padding[ padSize + i ] = (UInt8) ( ( bitCount >> ( 8 * i ) ) & 0xFF );
    Md5Update( md5, padding, padSize + 8 );

    for ( i=0; i<16; ++i )
        digest[ i ] = (UInt8) ( ( md5->state[ i / 4 ] >> ( 8 * ( i % 4 ) ) ) & 0xFF );
}

/*-------------------------------------------------------------------------*/

static const char*
GetTextOption( const TVariantMapApi* map, const char* key )
{
    TVariantData keyData;
    TVariantData valueData;

    if ( NULL == map || NULL == map->at_key )
        return NULL;

    memset( &keyData, 0, sizeof( keyData ) );
    keyData.containedType = GUCEF_DATATYPE_ASCII_STRING;
    keyData.union_data.heap_data.union_data.const_char_heap_data = key;
    keyData.union_data.heap_data.heap_data_size = (UInt32) strlen( key );
    keyData.union_data.heap_data.heap_data_is_linked = 1;

    memset( &valueData, 0, sizeof( valueData ) );
    map->at_key( map->privateData, &keyData, &valueData );
    if ( GUCEF_DATATYPE_ASCII_STRING != valueData.containedType && GUCEF_DATATYPE_UTF8_STRING != valueData.containedType )
        return NULL;
    return valueData.union_data.heap_data.union_data.const_char_heap_data;
}

/*-------------------------------------------------------------------------*/

static UInt32
GetUIntOption( const TVariantMapApi* map, const char* key, UInt32 defaultValue, UInt32 minimum, UInt32 maximum )
{
    const char* text = GetTextOption( map, key );
    char* end;
    unsigned long value;
    if ( NULL == text )
        return defaultValue;
    value = strtoul( text, &end, 10 );
    if ( end == text || value < minimum || value > maximum )
        return defaultValue;
    return (UInt32) value;
}

/*-------------------------------------------------------------------------*/

/** The sample at 'index' of interleaved input as a signed integer of 'bits' bits */
static Int32
GetSample( const UInt8* data, UInt32 sampleType, UInt64 index, UInt32 bits )
{
    switch ( sampleType )
    {
        case GUCEF_AUDIOCODEC_SAMPLETYPE_UINT8:
        {
            return (Int32) data[ index ] - 128;
        }
        case GUCEF_AUDIOCODEC_SAMPLETYPE_INT16:
        {
            Int16 value;
            memcpy( &value, data + index * 2, 2 );
            return value;
        }
        case GUCEF_AUDIOCODEC_SAMPLETYPE_INT24:
        {
            const UInt8* p = data + index * 3;
            Int32 value = IsHostBigEndian() ? ( ( (Int32) p[ 0 ] << 16 ) | ( (Int32) p[ 1 ] << 8 ) | (Int32) p[ 2 ] )
                                            : ( ( (Int32) p[ 2 ] << 16 ) | ( (Int32) p[ 1 ] << 8 ) | (Int32) p[ 0 ] );
            return ( value & 0x800000 ) ? value - 0x1000000 : value;
        }
        case GUCEF_AUDIOCODEC_SAMPLETYPE_INT32:
        {
            Int32 value;
            memcpy( &value, data + index * 4, 4 );
            return value >> ( 32 - bits );
        }
        default:
        {
            double value;
            double scale = (double) ( 1UL << ( bits - 1 ) );
            if ( GUCEF_AUDIOCODEC_SAMPLETYPE_FLOAT64 == sampleType )
                memcpy( &value, data + index * 8, 8 );
            else
            {
                float single;
                memcpy( &single, data + index * 4, 4 );
                value = single;
            }
            value *= scale;
            if ( value > scale - 1.0 )
                value = scale - 1.0;
            if ( value < -scale )
                value = -scale;
            return (Int32) ( value >= 0.0 ? value + 0.5 : value - 0.5 );
        }
    }
}

/*-------------------------------------------------------------------------*/

static UInt32
BytesPerSample( UInt32 sampleType )
{
    switch ( sampleType )
    {
        case GUCEF_AUDIOCODEC_SAMPLETYPE_UINT8: return 1;
        case GUCEF_AUDIOCODEC_SAMPLETYPE_INT16: return 2;
        case GUCEF_AUDIOCODEC_SAMPLETYPE_INT24: return 3;
        case GUCEF_AUDIOCODEC_SAMPLETYPE_INT32: return 4;
        case GUCEF_AUDIOCODEC_SAMPLETYPE_FLOAT32: return 4;
        case GUCEF_AUDIOCODEC_SAMPLETYPE_FLOAT64: return 8;
        default: return 0;
    }
}

/*-------------------------------------------------------------------------*/

static UInt32
ResidualToUnsigned( Int32 residual )
{
    return ( (UInt32) residual << 1 ) ^ (UInt32) ( residual >> 31 );
}

/*-------------------------------------------------------------------------*/

/** Bits to Rice code the values with parameter k */
static UInt64
RiceBits( const UInt32* values, UInt32 count, UInt32 k )
{
    UInt64 bits = 0;
    UInt32 i;
    for ( i=0; i<count; ++i )
        bits += (UInt64) ( values[ i ] >> k ) + 1 + k;
    return bits;
}

/*-------------------------------------------------------------------------*/

/** The best Rice parameter for the values (0 to maxParam) and the bits it takes */
static UInt32
BestRiceParameter( const UInt32* values, UInt32 count, UInt32 maxParam, UInt64* bits )
{
    UInt64 sum = 0;
    UInt32 estimate = 0;
    UInt32 best, k;
    UInt64 bestBits;
    UInt32 i;

    for ( i=0; i<count; ++i )
        sum += values[ i ];
    while ( estimate < maxParam && ( (UInt64) count << ( estimate + 1 ) ) <= sum )
        ++estimate;

    best = estimate;
    bestBits = RiceBits( values, count, estimate );
    for ( k = ( estimate > 0 ? estimate - 1 : 0 ); k <= estimate + 1 && k <= maxParam; ++k )
    {
        UInt64 candidate;
        if ( k == estimate )
            continue;
        candidate = RiceBits( values, count, k );
        if ( candidate < bestBits )
        {
            bestBits = candidate;
            best = k;
        }
    }
    *bits = bestBits;
    return best;
}

/*-------------------------------------------------------------------------*/

/** Computes the residual of a fixed predictor into 'residual' (n - order values) */
static void
FixedResidual( const Int32* s, UInt32 n, UInt32 order, Int32* residual )
{
    UInt32 i;
    for ( i=order; i<n; ++i )
    {
        Int32 prediction;
        switch ( order )
        {
            case 0: prediction = 0; break;
            case 1: prediction = s[ i-1 ]; break;
            case 2: prediction = 2*s[ i-1 ] - s[ i-2 ]; break;
            case 3: prediction = 3*s[ i-1 ] - 3*s[ i-2 ] + s[ i-3 ]; break;
            default: prediction = 4*s[ i-1 ] - 6*s[ i-2 ] + 4*s[ i-3 ] - s[ i-4 ]; break;
        }
        residual[ i-order ] = s[ i ] - prediction;
    }
}

/*-------------------------------------------------------------------------*/

/** What the Rice coded residual of a given partitioning takes: the parameters per partition */
static UInt64
PartitionedBits( const UInt32* unsignedValues, UInt32 n, UInt32 order, UInt32 partitionOrder, UInt8* params, UInt32* isLargeParam )
{
    UInt32 partitions = 1UL << partitionOrder;
    UInt32 perPartition = n >> partitionOrder;
    UInt64 total = 0;
    UInt32 offset = 0;
    UInt32 p;
    UInt32 maxParam = 0;

    for ( p=0; p<partitions; ++p )
    {
        UInt32 count = ( 0 == p ) ? perPartition - order : perPartition;
        UInt64 bits;
        UInt32 k = BestRiceParameter( unsignedValues + offset, count, 30, &bits );
        params[ p ] = (UInt8) k;
        total += bits;
        if ( k > maxParam )
            maxParam = k;
        offset += count;
    }
    *isLargeParam = maxParam > 14 ? 1 : 0;
    return total + (UInt64) partitions * ( *isLargeParam ? 5 : 4 );
}

/*-------------------------------------------------------------------------*/

/**
 *  Writes one sub frame of 'n' samples: the smallest of constant, fixed predictor and verbatim.
 *  'work' holds 2 * n Int32 and 'unsignedWork' n UInt32.
 */
static void
WriteSubframe( TFlacBitWriter* bw, const Int32* s, UInt32 n, UInt32 bits, Int32* work, UInt32* unsignedWork )
{
    UInt32 i, order, bestOrder = 0;
    UInt64 bestSum = 0;
    UInt64 verbatimBits = 8 + (UInt64) n * bits;
    int isConstant = 1;
    UInt32 maxPartitionOrder, po, bestPo = 0;
    UInt64 bestBits = 0;
    UInt8 params[ 1UL << FLACENC_MAX_PARTITION_ORDER ];
    UInt8 bestParams[ 1UL << FLACENC_MAX_PARTITION_ORDER ];
    UInt32 largeParam = 0, bestLarge = 0;
    Int32* residual = work;

    for ( i=1; i<n; ++i )
    {
        if ( s[ i ] != s[ 0 ] )
        {
            isConstant = 0;
            break;
        }
    }
    if ( isConstant )
    {
        BwPut( bw, 0, 8 );      /* zero bit, type constant, no wasted bits */
        BwPutSigned( bw, s[ 0 ], bits );
        return;
    }

    /* the order that leaves the smallest residual */
    for ( order=0; order<=FLACENC_MAX_ORDER && order < n; ++order )
    {
        UInt64 sum = 0;
        FixedResidual( s, n, order, residual );
        for ( i=0; i<n-order; ++i )
            sum += ( residual[ i ] < 0 ) ? (UInt64) ( -(Int64) residual[ i ] ) : (UInt64) residual[ i ];
        if ( 0 == order || sum < bestSum )
        {
            bestSum = sum;
            bestOrder = order;
        }
    }

    FixedResidual( s, n, bestOrder, residual );
    for ( i=0; i<n-bestOrder; ++i )
        unsignedWork[ i ] = ResidualToUnsigned( residual[ i ] );

    /* the partitioning: the number of partitions must divide the block and leave samples in the first one */
    maxPartitionOrder = 0;
    while ( maxPartitionOrder < FLACENC_MAX_PARTITION_ORDER &&
            0 == ( n % ( 1UL << ( maxPartitionOrder + 1 ) ) ) &&
            ( n >> ( maxPartitionOrder + 1 ) ) > bestOrder )
        ++maxPartitionOrder;

    for ( po=0; po<=maxPartitionOrder; ++po )
    {
        UInt64 candidate = PartitionedBits( unsignedWork, n, bestOrder, po, params, &largeParam );
        if ( 0 == po || candidate < bestBits )
        {
            bestBits = candidate;
            bestPo = po;
            bestLarge = largeParam;
            memcpy( bestParams, params, 1UL << po );
        }
    }
    bestBits += 8 + (UInt64) bestOrder * bits + 2 + 4;

    if ( bestBits >= verbatimBits )
    {
        BwPut( bw, 0x02, 8 );   /* verbatim */
        for ( i=0; i<n; ++i )
            BwPutSigned( bw, s[ i ], bits );
        return;
    }

    BwPut( bw, ( 0x10U | ( bestOrder << 1 ) ), 8 );     /* type fixed 001xxx: 8 + order, shifted over the zero bit and the wasted bits flag */
    for ( i=0; i<bestOrder; ++i )
        BwPutSigned( bw, s[ i ], bits );
    BwPut( bw, bestLarge, 2 );
    BwPut( bw, bestPo, 4 );
    {
        UInt32 partitions = 1UL << bestPo;
        UInt32 perPartition = n >> bestPo;
        UInt32 offset = 0;
        UInt32 p;
        for ( p=0; p<partitions; ++p )
        {
            UInt32 count = ( 0 == p ) ? perPartition - bestOrder : perPartition;
            UInt32 k = bestParams[ p ];
            UInt32 mask = ( 0 == k ) ? 0 : ( ( 1UL << k ) - 1 );
            BwPut( bw, k, bestLarge ? 5 : 4 );
            for ( i=0; i<count; ++i )
            {
                UInt32 u = unsignedWork[ offset + i ];
                BwPutUnary( bw, u >> k );
                BwPut( bw, u & mask, k );
            }
            offset += count;
        }
    }
}

/*-------------------------------------------------------------------------*/

static void
WriteUtf8Number( TFlacBitWriter* bw, UInt32 value )
{
    if ( value < 0x80 )
        BwPut( bw, value, 8 );
    else if ( value < 0x800 )
    {
        BwPut( bw, 0xC0 | ( value >> 6 ), 8 );
        BwPut( bw, 0x80 | ( value & 0x3F ), 8 );
    }
    else if ( value < 0x10000UL )
    {
        BwPut( bw, 0xE0 | ( value >> 12 ), 8 );
        BwPut( bw, 0x80 | ( ( value >> 6 ) & 0x3F ), 8 );
        BwPut( bw, 0x80 | ( value & 0x3F ), 8 );
    }
    else if ( value < 0x200000UL )
    {
        BwPut( bw, 0xF0 | ( value >> 18 ), 8 );
        BwPut( bw, 0x80 | ( ( value >> 12 ) & 0x3F ), 8 );
        BwPut( bw, 0x80 | ( ( value >> 6 ) & 0x3F ), 8 );
        BwPut( bw, 0x80 | ( value & 0x3F ), 8 );
    }
    else if ( value < 0x4000000UL )
    {
        BwPut( bw, 0xF8 | ( value >> 24 ), 8 );
        BwPut( bw, 0x80 | ( ( value >> 18 ) & 0x3F ), 8 );
        BwPut( bw, 0x80 | ( ( value >> 12 ) & 0x3F ), 8 );
        BwPut( bw, 0x80 | ( ( value >> 6 ) & 0x3F ), 8 );
        BwPut( bw, 0x80 | ( value & 0x3F ), 8 );
    }
    else
    {
        BwPut( bw, 0xFC | ( value >> 30 ), 8 );
        BwPut( bw, 0x80 | ( ( value >> 24 ) & 0x3F ), 8 );
        BwPut( bw, 0x80 | ( ( value >> 18 ) & 0x3F ), 8 );
        BwPut( bw, 0x80 | ( ( value >> 12 ) & 0x3F ), 8 );
        BwPut( bw, 0x80 | ( ( value >> 6 ) & 0x3F ), 8 );
        BwPut( bw, 0x80 | ( value & 0x3F ), 8 );
    }
}

/*-------------------------------------------------------------------------*/

static UInt32
BlockSizeCode( UInt32 blockSize )
{
    switch ( blockSize )
    {
        case 192: return 1;
        case 576: return 2;
        case 1152: return 3;
        case 2304: return 4;
        case 4608: return 5;
        case 256: return 8;
        case 512: return 9;
        case 1024: return 10;
        case 2048: return 11;
        case 4096: return 12;
        case 8192: return 13;
        case 16384: return 14;
        case 32768UL: return 15;
        default: return blockSize <= 256 ? 6 : 7;
    }
}

/*-------------------------------------------------------------------------*/

static UInt32
SampleSizeCode( UInt32 bits )
{
    switch ( bits )
    {
        case 8: return 1;
        case 12: return 2;
        case 16: return 4;
        case 20: return 5;
        case 24: return 6;
        default: return 0;
    }
}

/*-------------------------------------------------------------------------*/

static int
WriteBytes( TIOAccess* output, const void* data, size_t size )
{
    const UInt8* p = (const UInt8*) data;
    while ( size > 0 )
    {
        UInt32 chunk = size > 0x40000000UL ? 0x40000000UL : (UInt32) size;
        if ( chunk != output->write( output, p, 1, chunk ) )
            return 0;
        p += chunk;
        size -= chunk;
    }
    return 1;
}

/*-------------------------------------------------------------------------*/

static void
PutBE( UInt8* out, UInt64 value, UInt32 bytes )
{
    UInt32 i;
    for ( i=0; i<bytes; ++i )
        out[ i ] = (UInt8) ( ( value >> ( 8 * ( bytes - 1 - i ) ) ) & 0xFF );
}

/*-------------------------------------------------------------------------*/

static void
PutLE32( UInt8* out, UInt32 value )
{
    out[ 0 ] = (UInt8) ( value & 0xFF );
    out[ 1 ] = (UInt8) ( ( value >> 8 ) & 0xFF );
    out[ 2 ] = (UInt8) ( ( value >> 16 ) & 0xFF );
    out[ 3 ] = (UInt8) ( ( value >> 24 ) & 0xFF );
}

/*-------------------------------------------------------------------------*/

static UInt32
CountBits( UInt32 value )
{
    UInt32 count = 0;
    while ( 0 != value )
    {
        count += value & 1U;
        value >>= 1;
    }
    return count;
}

/*-------------------------------------------------------------------------*/

/** The VORBIS_COMMENT block (header included), NULL when there is nothing to put in it or memory ran out */
static UInt8*
BuildCommentBlock( const TFlacEncoder* enc, const TAudioFormatInfo* format, const char* tags, size_t* blockSize )
{
    size_t vendorSize = strlen( FLACENC_VENDOR );
    size_t tagsSize = NULL != tags ? strlen( tags ) : 0;
    size_t capacity = 4 + 4 + vendorSize + 4 + tagsSize * 2 + 4 + 64 + 8 * 4;
    UInt8* block = (UInt8*) malloc( capacity );
    size_t pos, countPos;
    UInt32 count = 0;
    int addMask;

    if ( NULL == block )
        return NULL;

    pos = 4;
    PutLE32( block + pos, (UInt32) vendorSize );
    pos += 4;
    memcpy( block + pos, FLACENC_VENDOR, vendorSize );
    pos += vendorSize;
    countPos = pos;
    pos += 4;

    addMask = 0 != format->channelLayout && CountBits( format->channelLayout ) == format->channelCount &&
              !( 1 == format->channelCount && 0x4 == format->channelLayout ) && !( 2 == format->channelCount && 0x3 == format->channelLayout );
    if ( addMask )
    {
        char text[ 64 ];
        size_t length;
        const char* digits = "0123456789ABCDEF";
        UInt32 n, started = 0;
        strcpy( text, "WAVEFORMATEXTENSIBLE_CHANNEL_MASK=0x" );
        length = strlen( text );
        for ( n=0; n<8; ++n )
        {
            UInt32 digit = ( format->channelLayout >> ( 28 - 4 * n ) ) & 0xF;
            if ( digit || started || 7 == n )
            {
                text[ length++ ] = digits[ digit ];
                started = 1;
            }
        }
        PutLE32( block + pos, (UInt32) length );
        pos += 4;
        memcpy( block + pos, text, length );
        pos += length;
        ++count;
    }

    /* lines of NAME=value */
    if ( NULL != tags )
    {
        const char* line = tags;
        while ( '\0' != *line )
        {
            const char* end = strchr( line, '\n' );
            size_t length = NULL != end ? (size_t) ( end - line ) : strlen( line );
            const char* next = NULL != end ? end + 1 : line + length;
            if ( length > 0 && '\r' == line[ length - 1 ] )
                --length;
            if ( length > 2 && NULL != memchr( line, '=', length ) && '=' != line[ 0 ] )
            {
                PutLE32( block + pos, (UInt32) length );
                pos += 4;
                memcpy( block + pos, line, length );
                pos += length;
                ++count;
            }
            line = next;
        }
    }
    PutLE32( block + countPos, count );
    (void) enc;

    if ( 0 == count )
    {
        free( block );
        return NULL;
    }
    /* header: type 4, length in 24 bits, the last-block flag is set by the caller */
    block[ 0 ] = 4;
    PutBE( block + 1, (UInt64) ( pos - 4 ), 3 );
    *blockSize = pos;
    return block;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      ENCODER                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

UInt32
FlacEncode( const TGucefCoreCApi* libApi, const TAudioFormatInfo* format, const void* frames, UInt64 frameCount, TIOAccess* output, const TVariantMapApi* options )
{
    TFlacEncoder* enc;
    const UInt8* data = (const UInt8*) frames;
    UInt32 bytesPerSample = BytesPerSample( format->sampleType );
    UInt32 channels = format->channelCount;
    UInt32 bytesPerSampleInFile;
    UInt64 position;
    UInt8 streamInfo[ 4 + 4 + 34 ];
    UInt8 md5Digest[ 16 ];
    UInt8* commentBlock = NULL;
    size_t commentSize = 0;
    Int32* channelSamples = NULL;
    Int32* work = NULL;
    UInt32* unsignedWork = NULL;
    UInt8* frameBuffer = NULL;
    size_t frameCapacity;
    UInt32 frameNumber = 0;
    UInt32 c;
    UInt32 result = 0;
    const char* tags;

    if ( NULL == output || NULL == output->write || 0 == bytesPerSample || channels < 1 || channels > 8 ||
         format->sampleRate < 1 || format->sampleRate > 655350UL || ( NULL == frames && 0 != frameCount ) )
    {
        GUCEF_C_ERROR_LOG( (*libApi), GUCEF_LOGLEVEL_NORMAL, "audiocodecpluginFLAC: the audio can not be written as FLAC (1 to 8 channels, sample rate up to 655350)" );
        return 0;
    }

    enc = (TFlacEncoder*) calloc( 1, sizeof( TFlacEncoder ) );
    if ( NULL == enc )
        return 0;
    BuildCrcTables( enc );
    enc->channels = channels;
    enc->sampleRate = format->sampleRate;
    enc->sampleType = format->sampleType;
    enc->blockSize = GetUIntOption( options, "blocksize", FLACENC_DEFAULT_BLOCKSIZE, 16, 65535 );

    switch ( format->sampleType )
    {
        case GUCEF_AUDIOCODEC_SAMPLETYPE_UINT8: enc->bits = 8; break;
        case GUCEF_AUDIOCODEC_SAMPLETYPE_INT16: enc->bits = 16; break;
        case GUCEF_AUDIOCODEC_SAMPLETYPE_INT24: enc->bits = 24; break;
        case GUCEF_AUDIOCODEC_SAMPLETYPE_INT32: enc->bits = GetUIntOption( options, "bits", 24, 8, 24 ); break;
        default: enc->bits = GetUIntOption( options, "bits", 16, 8, 24 ); break;
    }
    bytesPerSampleInFile = ( enc->bits + 7 ) / 8;

    /* The MD5 of the samples as the file holds them is part of STREAMINFO, so it takes a first pass */
    {
        TFlacMd5 md5;
        UInt8* chunk = (UInt8*) malloc( (size_t) FLACENC_MD5_CHUNK_FRAMES * channels * bytesPerSampleInFile );
        UInt64 done = 0;
        if ( NULL == chunk )
            goto cleanup;
        Md5Init( &md5 );
        while ( done < frameCount )
        {
            UInt64 take = frameCount - done;
            UInt64 i;
            size_t out = 0;
            if ( take > FLACENC_MD5_CHUNK_FRAMES )
                take = FLACENC_MD5_CHUNK_FRAMES;
            for ( i=0; i<take * channels; ++i )
            {
                Int32 sample = GetSample( data, format->sampleType, done * channels + i, enc->bits );
                UInt32 b;
                for ( b=0; b<bytesPerSampleInFile; ++b )
                    chunk[ out++ ] = (UInt8) ( ( (UInt32) sample >> ( 8 * b ) ) & 0xFF );
            }
            Md5Update( &md5, chunk, out );
            done += take;
        }
        Md5Final( &md5, md5Digest );
        free( chunk );
    }

    tags = GetTextOption( options, "tags" );
    commentBlock = BuildCommentBlock( enc, format, tags, &commentSize );

    /* fLaC, STREAMINFO */
    memcpy( streamInfo, "fLaC", 4 );
    streamInfo[ 4 ] = (UInt8) ( ( NULL == commentBlock ? 0x80 : 0x00 ) | 0 );
    PutBE( streamInfo + 5, 34, 3 );
    PutBE( streamInfo + 8, enc->blockSize, 2 );
    PutBE( streamInfo + 10, ( frameCount < enc->blockSize ) ? (UInt32) frameCount : enc->blockSize, 2 );
    PutBE( streamInfo + 12, 0, 3 );     /* smallest frame in bytes: unknown */
    PutBE( streamInfo + 15, 0, 3 );     /* largest frame in bytes: unknown */
    /* rate 20 bits, channels - 1 in 3 bits, bits - 1 in 5 bits, total frames 36 bits */
    {
        UInt64 packed = ( (UInt64) enc->sampleRate << 44 ) | ( (UInt64) ( channels - 1 ) << 41 ) | ( (UInt64) ( enc->bits - 1 ) << 36 ) | ( frameCount & 0xFFFFFFFFFULL );
        PutBE( streamInfo + 18, packed, 8 );
    }
    memcpy( streamInfo + 26, md5Digest, 16 );
    if ( !WriteBytes( output, streamInfo, sizeof( streamInfo ) ) )
        goto cleanup;
    if ( NULL != commentBlock )
    {
        commentBlock[ 0 ] |= 0x80;
        if ( !WriteBytes( output, commentBlock, commentSize ) )
            goto cleanup;
    }

    /* the frames */
    channelSamples = (Int32*) malloc( sizeof( Int32 ) * (size_t) channels * enc->blockSize );
    work = (Int32*) malloc( sizeof( Int32 ) * (size_t) enc->blockSize );
    unsignedWork = (UInt32*) malloc( sizeof( UInt32 ) * (size_t) enc->blockSize );
    frameCapacity = (size_t) channels * ( (size_t) enc->blockSize * 4 + 16 ) + 64;
    frameBuffer = (UInt8*) malloc( frameCapacity );
    if ( NULL == channelSamples || NULL == work || NULL == unsignedWork || NULL == frameBuffer )
        goto cleanup;

    for ( position=0; position<frameCount; position += enc->blockSize, ++frameNumber )
    {
        UInt32 n = ( frameCount - position < enc->blockSize ) ? (UInt32) ( frameCount - position ) : enc->blockSize;
        TFlacBitWriter bw;
        UInt32 blockCode = BlockSizeCode( n );
        UInt32 i;
        UInt16 crc16;

        for ( i=0; i<n; ++i )
            for ( c=0; c<channels; ++c )
                channelSamples[ (size_t) c * n + i ] = GetSample( data, format->sampleType, ( position + i ) * channels + c, enc->bits );

        BwInit( &bw, frameBuffer, frameCapacity );
        BwPut( &bw, 0xFFF8, 16 );                       /* sync, fixed block size */
        BwPut( &bw, blockCode, 4 );
        BwPut( &bw, 0, 4 );                             /* sample rate: from STREAMINFO */
        BwPut( &bw, channels - 1, 4 );                  /* independent channels */
        BwPut( &bw, SampleSizeCode( enc->bits ), 3 );
        BwPut( &bw, 0, 1 );
        WriteUtf8Number( &bw, frameNumber );
        if ( 6 == blockCode )
            BwPut( &bw, n - 1, 8 );
        else if ( 7 == blockCode )
            BwPut( &bw, n - 1, 16 );
        BwPut( &bw, Crc8( enc, bw.buffer, bw.size ), 8 );

        for ( c=0; c<channels; ++c )
            WriteSubframe( &bw, channelSamples + (size_t) c * n, n, enc->bits, work, unsignedWork );
        BwAlign( &bw );
        crc16 = Crc16( enc, bw.buffer, bw.size );
        BwPut( &bw, crc16, 16 );

        if ( bw.isOverflown || !WriteBytes( output, bw.buffer, bw.size ) )
            goto cleanup;
    }
    result = 1;

cleanup:
    free( frameBuffer );
    free( unsignedWork );
    free( work );
    free( channelSamples );
    free( commentBlock );
    free( enc );
    return result;
}

/*-------------------------------------------------------------------------*/
