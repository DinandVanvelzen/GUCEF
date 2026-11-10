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

#include <string.h>

#ifndef GUCEF_CORE_MACROS_H
#include "gucefCORE_macros.h"
#define GUCEF_CORE_MACROS_H
#endif /* GUCEF_CORE_MACROS_H ? */

#include "tablecodecpluginPARQUET_CRleBitPackedDecoder.h"

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

/** Unpacks one group of 8 values of W bits, reading exactly W bytes; the loop unrolls for the constant W */
template < UInt32 W >
void
Unpack8( const UInt8* in, UInt32* out )
{
    const UInt64 mask = ( (UInt64) 1 << W ) - 1;
    UInt64 accumulator = 0;
    UInt32 bits = 0;
    for ( UInt32 i=0; i<8; ++i )
    {
        while ( bits < W )
        {
            accumulator |= (UInt64) *in++ << bits;
            bits += 8;
        }
        out[ i ] = (UInt32) ( accumulator & mask );
        accumulator >>= W;
        bits -= W;
    }
}

/** Width 0: every value is 0 */
void
Unpack8Zero( const UInt8*, UInt32* out )
{
    memset( out, 0, 8 * sizeof( UInt32 ) );
}

typedef void ( *TUnpack8 )( const UInt8* in, UInt32* out );

const TUnpack8 Unpack8Kernels[ CRleBitPackedDecoder::MaxBitWidth + 1 ] =
{
    Unpack8Zero,
    Unpack8< 1 >,  Unpack8< 2 >,  Unpack8< 3 >,  Unpack8< 4 >,  Unpack8< 5 >,  Unpack8< 6 >,  Unpack8< 7 >,  Unpack8< 8 >,
    Unpack8< 9 >,  Unpack8< 10 >, Unpack8< 11 >, Unpack8< 12 >, Unpack8< 13 >, Unpack8< 14 >, Unpack8< 15 >, Unpack8< 16 >,
    Unpack8< 17 >, Unpack8< 18 >, Unpack8< 19 >, Unpack8< 20 >, Unpack8< 21 >, Unpack8< 22 >, Unpack8< 23 >, Unpack8< 24 >,
    Unpack8< 25 >, Unpack8< 26 >, Unpack8< 27 >, Unpack8< 28 >, Unpack8< 29 >, Unpack8< 30 >, Unpack8< 31 >, Unpack8< 32 >
};

inline void
SetBitRange( UInt8* bitmap, UInt32 first, UInt32 count )
{
    UInt32 bit = first;
    const UInt32 end = first + count;
    while ( bit < end && 0 != ( bit & 7 ) )
    {
        bitmap[ bit >> 3 ] |= (UInt8) ( 1u << ( bit & 7 ) );
        ++bit;
    }
    if ( end - bit >= 8 )
    {
        const UInt32 bytes = ( end - bit ) >> 3;
        memset( bitmap + ( bit >> 3 ), 0xFF, bytes );
        bit += bytes << 3;
    }
    while ( bit < end )
    {
        bitmap[ bit >> 3 ] |= (UInt8) ( 1u << ( bit & 7 ) );
        ++bit;
    }
}

inline UInt32
PopCount8( UInt8 value )
{
    UInt32 count = 0;
    while ( 0 != value )
    {
        value &= (UInt8) ( value - 1 );
        ++count;
    }
    return count;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CRleBitPackedDecoder::CRleBitPackedDecoder( void )
    : m_data( GUCEF_NULL )
    , m_size( 0 )
    , m_position( 0 )
    , m_bitWidth( 0 )
    , m_valueBytes( 0 )
    , m_maxValue( 0 )
    , m_isRle( false )
    , m_runRemaining( 0 )
    , m_rleValue( 0 )
    , m_packed( GUCEF_NULL )
    , m_packedBytes( 0 )
    , m_packedIndex( 0 )
    , m_failed( false )
    , m_error( GUCEF_NULL )
{GUCEF_TRACE;
}

/*-------------------------------------------------------------------------*/

bool
CRleBitPackedDecoder::Fail( const char* message )
{
    if ( !m_failed )
    {
        m_failed = true;
        m_error = message;
    }
    m_runRemaining = 0;
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CRleBitPackedDecoder::Reset( const UInt8* data, UInt32 size, UInt32 bitWidth )
{GUCEF_TRACE;

    m_data = data;
    m_size = GUCEF_NULL != data ? size : 0;
    m_position = 0;
    m_isRle = false;
    m_runRemaining = 0;
    m_rleValue = 0;
    m_packed = GUCEF_NULL;
    m_packedBytes = 0;
    m_packedIndex = 0;
    m_failed = false;
    m_error = GUCEF_NULL;
    m_bitWidth = bitWidth;
    if ( bitWidth > MaxBitWidth )
    {
        m_bitWidth = 0;
        return Fail( "RLE / bit-packed bit width above 32" );
    }
    m_valueBytes = ( bitWidth + 7 ) / 8;
    m_maxValue = ( (UInt64) 1 << bitWidth ) - 1;
    return true;
}

/*-------------------------------------------------------------------------*/

UInt32
CRleBitPackedDecoder::ExtractPacked( const UInt8* packed, UInt32 index, UInt32 bitWidth, UInt32 availableBytes )
{
    if ( 0 == bitWidth )
        return 0;
    const UInt64 bitPosition = (UInt64) index * bitWidth;
    UInt32 byte = (UInt32) ( bitPosition >> 3 );
    const UInt32 shift = (UInt32) ( bitPosition & 7 );
    const UInt32 needed = ( shift + bitWidth + 7 ) / 8;
    UInt64 accumulator = 0;
    for ( UInt32 i=0; i<needed && byte + i < availableBytes; ++i )
        accumulator |= (UInt64) packed[ byte + i ] << ( 8 * i );
    return (UInt32) ( ( accumulator >> shift ) & ( ( (UInt64) 1 << bitWidth ) - 1 ) );
}

/*-------------------------------------------------------------------------*/

bool
CRleBitPackedDecoder::NextRun( void )
{
    if ( m_failed )
        return false;
    if ( m_position >= m_size )
        return false;   // the end of the data: no more values, not a failure by itself

    // the run header, a ULEB128 varint
    UInt64 header = 0;
    UInt32 shift = 0;
    bool done = false;
    for ( UInt32 i=0; i<10; ++i )
    {
        if ( m_position >= m_size )
            return Fail( "RLE / bit-packed run header runs past the end of the data" );
        const UInt8 byte = m_data[ m_position++ ];
        header |= (UInt64) ( byte & 0x7F ) << shift;
        if ( 0 == ( byte & 0x80 ) )
        {
            done = true;
            break;
        }
        shift += 7;
    }
    if ( !done )
        return Fail( "RLE / bit-packed run header is longer than 10 bytes" );

    if ( 0 != ( header & 1 ) )
    {
        const UInt64 groups = header >> 1;
        if ( 0 == groups || groups > MaxRunLength / 8 )
            return Fail( "RLE / bit-packed literal run of length 0 or above 2^31 - 1 values" );
        const UInt64 runBytes = groups * m_bitWidth;
        const UInt32 available = (UInt32) ( runBytes < (UInt64) ( m_size - m_position ) ? runBytes : (UInt64) ( m_size - m_position ) );
        UInt64 values = groups * 8;
        if ( available < runBytes )
        {
            // the data ends in the run: the values whose bits are all there
            values = ( (UInt64) available * 8 ) / m_bitWidth;
            if ( 0 == values )
                return Fail( "RLE / bit-packed literal run is cut off before its first value" );
        }
        m_isRle = false;
        m_packed = m_data + m_position;
        m_packedBytes = available;
        m_packedIndex = 0;
        m_runRemaining = (UInt32) values;
        m_position += available;
        return true;
    }

    const UInt64 count = header >> 1;
    if ( 0 == count || count > MaxRunLength )
        return Fail( "RLE / bit-packed repeated run of length 0 or above 2^31 - 1 values" );
    if ( m_size - m_position < m_valueBytes )
        return Fail( "RLE / bit-packed repeated value runs past the end of the data" );
    UInt64 value = 0;
    for ( UInt32 i=0; i<m_valueBytes; ++i )
        value |= (UInt64) m_data[ m_position + i ] << ( 8 * i );
    m_position += m_valueBytes;
    if ( value > m_maxValue )
        return Fail( "RLE / bit-packed repeated value does not fit the bit width" );
    m_isRle = true;
    m_rleValue = (UInt32) value;
    m_runRemaining = (UInt32) count;
    return true;
}

/*-------------------------------------------------------------------------*/

UInt32
CRleBitPackedDecoder::GetValues( UInt32* out, UInt32 count )
{
    UInt32 done = 0;
    while ( done < count )
    {
        if ( 0 == m_runRemaining && !NextRun() )
            break;
        UInt32 take = count - done;
        if ( take > m_runRemaining )
            take = m_runRemaining;
        if ( m_isRle )
        {
            UInt32* target = out + done;
            const UInt32 value = m_rleValue;
            for ( UInt32 i=0; i<take; ++i )
                target[ i ] = value;
        }
        else
        {
            UInt32 i = 0;
            // values before a group boundary, then whole groups through the kernel, then the rest
            while ( i < take && 0 != ( ( m_packedIndex + i ) & 7 ) )
            {
                out[ done + i ] = ExtractPacked( m_packed, m_packedIndex + i, m_bitWidth, m_packedBytes );
                ++i;
            }
            const TUnpack8 kernel = Unpack8Kernels[ m_bitWidth ];
            while ( take - i >= 8 && ( (UInt64) ( ( m_packedIndex + i ) / 8 ) + 1 ) * m_bitWidth <= m_packedBytes )
            {
                kernel( m_packed + ( ( m_packedIndex + i ) / 8 ) * m_bitWidth, out + done + i );
                i += 8;
            }
            while ( i < take )
            {
                out[ done + i ] = ExtractPacked( m_packed, m_packedIndex + i, m_bitWidth, m_packedBytes );
                ++i;
            }
            m_packedIndex += take;
        }
        m_runRemaining -= take;
        done += take;
    }
    return done;
}

/*-------------------------------------------------------------------------*/

UInt32
CRleBitPackedDecoder::GetBits( UInt8* bitmap, UInt32 firstBit, UInt32 count, UInt32& zeroCount )
{
    if ( 1 != m_bitWidth )
    {
        Fail( "RLE / bit-packed bits need bit width 1" );
        return 0;
    }
    UInt32 done = 0;
    while ( done < count )
    {
        if ( 0 == m_runRemaining && !NextRun() )
            break;
        UInt32 take = count - done;
        if ( take > m_runRemaining )
            take = m_runRemaining;
        const UInt32 target = firstBit + done;
        if ( m_isRle )
        {
            if ( 0 != m_rleValue )
                SetBitRange( bitmap, target, take );
            else
                zeroCount += take;
        }
        else
        {
            UInt32 i = 0;
            if ( 0 == ( target & 7 ) && 0 == ( m_packedIndex & 7 ) )
            {
                // both on byte boundaries: the packed bytes are the bitmap bytes
                const UInt32 wholeBytes = take / 8;
                const UInt8* source = m_packed + m_packedIndex / 8;
                UInt8* destination = bitmap + target / 8;
                UInt32 ones = 0;
                for ( UInt32 b=0; b<wholeBytes; ++b )
                {
                    destination[ b ] = source[ b ];
                    ones += PopCount8( source[ b ] );
                }
                zeroCount += wholeBytes * 8 - ones;
                i = wholeBytes * 8;
            }
            for ( ; i<take; ++i )
            {
                const UInt32 index = m_packedIndex + i;
                const UInt32 bit = target + i;
                if ( 0 != ( m_packed[ index >> 3 ] & ( 1u << ( index & 7 ) ) ) )
                    bitmap[ bit >> 3 ] |= (UInt8) ( 1u << ( bit & 7 ) );
                else
                    ++zeroCount;
            }
            m_packedIndex += take;
        }
        m_runRemaining -= take;
        done += take;
    }
    return done;
}

/*-------------------------------------------------------------------------*/

UInt32
CRleBitPackedDecoder::Skip( UInt32 count )
{
    UInt32 done = 0;
    while ( done < count )
    {
        if ( 0 == m_runRemaining && !NextRun() )
            break;
        UInt32 take = count - done;
        if ( take > m_runRemaining )
            take = m_runRemaining;
        if ( !m_isRle )
            m_packedIndex += take;
        m_runRemaining -= take;
        done += take;
    }
    return done;
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
