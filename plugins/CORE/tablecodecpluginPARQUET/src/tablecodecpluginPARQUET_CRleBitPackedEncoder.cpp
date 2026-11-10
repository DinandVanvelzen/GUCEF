/*
 *  tablecodecpluginPARQUET: GUCEF table codec plugin for Apache Parquet files
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

#ifndef TABLECODECPLUGIN_PARQUET_CRLEBITPACKEDDECODER_H
#include "tablecodecpluginPARQUET_CRleBitPackedDecoder.h"
#define TABLECODECPLUGIN_PARQUET_CRLEBITPACKEDDECODER_H
#endif /* TABLECODECPLUGIN_PARQUET_CRLEBITPACKEDDECODER_H ? */

#include "tablecodecpluginPARQUET_CRleBitPackedEncoder.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace TABLECODECPLUGIN {
namespace PARQUET {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** Equal values from index start, counting up to cap */
inline UInt32
RunLength( const UInt32* values, UInt32 start, UInt32 count, UInt32 cap )
{
    const UInt32 value = values[ start ];
    UInt32 end = start + 1;
    const UInt32 limit = ( count - start > cap ) ? start + cap : count;
    while ( end < limit && values[ end ] == value )
        ++end;
    return end - start;
}

/** Whether the bitmap byte is all set or all clear over the bits of the count it holds, and which */
inline bool
IsUniformByte( const UInt8* bitmap, UInt32 byteIndex, UInt32 count, bool& bit )
{
    const UInt32 bitsInByte = ( count - byteIndex * 8 ) >= 8 ? 8 : count - byteIndex * 8;
    const UInt8 mask = (UInt8) ( 8 == bitsInByte ? 0xFF : ( ( 1u << bitsInByte ) - 1 ) );
    const UInt8 value = (UInt8) ( bitmap[ byteIndex ] & mask );
    if ( value == mask ) { bit = true; return true; }
    if ( 0 == value ) { bit = false; return true; }
    return false;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

UInt32
CRleBitPackedEncoder::BitWidthFor( UInt32 maxValue )
{
    UInt32 width = 0;
    while ( width < 32 && ( (UInt64) 1 << width ) <= maxValue )
        ++width;
    return width;
}

/*-------------------------------------------------------------------------*/

void
CRleBitPackedEncoder::WriteVarint( UInt64 value, CORE::CDynamicBuffer& output )
{
    UInt8 bytes[ 10 ];
    UInt32 count = 0;
    do
    {
        UInt8 byte = (UInt8) ( value & 0x7F );
        value >>= 7;
        if ( 0 != value )
            byte |= 0x80;
        bytes[ count++ ] = byte;
    }
    while ( 0 != value );
    output.Append( bytes, count );
}

/*-------------------------------------------------------------------------*/

void
CRleBitPackedEncoder::WriteRleRun( UInt32 value, UInt32 count, UInt32 bitWidth, CORE::CDynamicBuffer& output )
{
    const UInt32 valueBytes = ( bitWidth + 7 ) / 8;
    while ( count > 0 )
    {
        const UInt32 run = count > CRleBitPackedDecoder::MaxRunLength ? CRleBitPackedDecoder::MaxRunLength : count;
        WriteVarint( (UInt64) run << 1, output );
        UInt8 bytes[ 4 ];
        for ( UInt32 i=0; i<valueBytes; ++i )
            bytes[ i ] = (UInt8) ( value >> ( 8 * i ) );
        if ( valueBytes > 0 )
            output.Append( bytes, valueBytes );
        count -= run;
    }
}

/*-------------------------------------------------------------------------*/

void
CRleBitPackedEncoder::WriteLiteralRun( const UInt32* values, UInt32 count, UInt32 groups, UInt32 bitWidth, CORE::CDynamicBuffer& output )
{
    WriteVarint( ( (UInt64) groups << 1 ) | 1, output );
    if ( 0 == bitWidth )
        return;

    // 8 values of the width make bitWidth bytes; the values past count are zero padding
    UInt8 group[ 32 ];
    for ( UInt32 g=0; g<groups; ++g )
    {
        UInt64 accumulator = 0;
        UInt32 bits = 0;
        UInt32 written = 0;
        for ( UInt32 i=0; i<8; ++i )
        {
            const UInt32 index = g * 8 + i;
            const UInt64 value = index < count ? values[ index ] : 0;
            accumulator |= value << bits;
            bits += bitWidth;
            while ( bits >= 8 )
            {
                group[ written++ ] = (UInt8) accumulator;
                accumulator >>= 8;
                bits -= 8;
            }
        }
        output.Append( group, written );
    }
}

/*-------------------------------------------------------------------------*/

void
CRleBitPackedEncoder::EncodeValues( const UInt32* values, UInt32 count, UInt32 bitWidth, CORE::CDynamicBuffer& output )
{GUCEF_TRACE;

    UInt32 i = 0;
    while ( i < count )
    {
        const UInt32 run = RunLength( values, i, count, count );
        if ( run >= 8 )
        {
            WriteRleRun( values[ i ], run, bitWidth, output );
            i += run;
            continue;
        }

        // bit-packed groups until a run of 8 or more starts on a group boundary
        const UInt32 start = i;
        UInt64 end = i;
        for ( ;; )
        {
            end += 8;
            if ( end >= count )
                break;
            if ( ( end - start ) / 8 >= CRleBitPackedDecoder::MaxRunLength / 8 )
                break;
            if ( RunLength( values, (UInt32) end, count, 8 ) >= 8 )
                break;
        }
        const UInt32 stop = end >= count ? count : (UInt32) end;
        WriteLiteralRun( values + start, stop - start, (UInt32) ( ( end - start ) / 8 ), bitWidth, output );
        i = stop;
    }
}

/*-------------------------------------------------------------------------*/

void
CRleBitPackedEncoder::EncodeRepeatedBit( bool bit, UInt32 count, CORE::CDynamicBuffer& output )
{GUCEF_TRACE;

    if ( count > 0 )
        WriteRleRun( bit ? 1 : 0, count, 1, output );
}

/*-------------------------------------------------------------------------*/

void
CRleBitPackedEncoder::EncodeBits( const UInt8* bitmap, UInt32 count, CORE::CDynamicBuffer& output )
{GUCEF_TRACE;

    const UInt32 byteCount = ( count + 7 ) / 8;
    UInt32 b = 0;
    while ( b < byteCount )
    {
        bool bit = false;
        if ( IsUniformByte( bitmap, b, count, bit ) )
        {
            UInt32 e = b + 1;
            bool next = false;
            while ( e < byteCount && IsUniformByte( bitmap, e, count, next ) && next == bit )
                ++e;
            if ( e - b >= 2 )
            {
                const UInt32 bits = ( e == byteCount ? count : e * 8 ) - b * 8;
                WriteRleRun( bit ? 1 : 0, bits, 1, output );
                b = e;
                continue;
            }
        }

        // bitmap bytes as they are, until two uniform bytes of the same value follow
        const UInt32 start = b;
        ++b;
        while ( b < byteCount && b - start < CRleBitPackedDecoder::MaxRunLength / 8 )
        {
            bool first = false, second = false;
            if ( IsUniformByte( bitmap, b, count, first ) && b + 1 < byteCount &&
                 IsUniformByte( bitmap, b + 1, count, second ) && first == second )
                break;
            ++b;
        }
        WriteVarint( ( (UInt64) ( b - start ) << 1 ) | 1, output );
        const bool partialLast = ( b == byteCount && 0 != ( count & 7 ) );
        const UInt32 wholeBytes = partialLast ? b - start - 1 : b - start;
        if ( wholeBytes > 0 )
            output.Append( bitmap + start, wholeBytes );
        if ( partialLast )
        {
            const UInt8 value = (UInt8) ( bitmap[ byteCount - 1 ] & ( ( 1u << ( count & 7 ) ) - 1 ) );
            output.Append( &value, 1 );
        }
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace PARQUET */
}; /* namespace TABLECODECPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
