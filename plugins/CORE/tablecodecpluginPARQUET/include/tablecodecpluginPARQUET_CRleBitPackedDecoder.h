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

#ifndef TABLECODECPLUGIN_PARQUET_CRLEBITPACKEDDECODER_H
#define TABLECODECPLUGIN_PARQUET_CRLEBITPACKEDDECODER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_TYPES_H
#include "gucef_types.h"
#define GUCEF_TYPES_H
#endif /* GUCEF_TYPES_H ? */

#ifndef TABLECODECPLUGIN_PARQUET_MACROS_H
#include "tablecodecpluginPARQUET_macros.h"
#define TABLECODECPLUGIN_PARQUET_MACROS_H
#endif /* TABLECODECPLUGIN_PARQUET_MACROS_H ? */

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
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Decodes the RLE / bit-packed hybrid of the format: a sequence of runs, each headed by a ULEB128 varint. Lowest bit 1:
 *  a bit-packed run of ( header >> 1 ) groups of 8 values of the bit width, packed LSB first. Lowest bit 0: an RLE run
 *  of header >> 1 repeats of one value stored in ceil( bitWidth / 8 ) little endian bytes.
 *
 *  Bit widths 0 to 32 (the format's maximum for dictionary indices; levels and booleans use far less). Groups of 8 values
 *  are unpacked by one kernel per bit width. The input is untrusted: a run of length 0 or above 2^31 - 1, an RLE value
 *  that does not fit the bit width, a varint of more than 10 bytes and a run past the end of the data make the decoder
 *  fail (HasFailed()); it never reads outside the given bytes. A bit-packed run that the data ends in the middle of
 *  yields the values that are complete, as other readers accept for the last run.
 *
 *  Reset() makes no allocation; the decoder can be reused.
 */
class TABLECODECPLUGIN_PARQUET_PUBLIC_CPP CRleBitPackedDecoder
{
    public:

    static const UInt32 MaxBitWidth = 32;
    static const UInt32 MaxRunLength = 0x7FFFFFFF;

    CRleBitPackedDecoder( void );

    /** False (and failed) for a bit width above MaxBitWidth */
    bool Reset( const UInt8* data, UInt32 size, UInt32 bitWidth );

    /** Decodes up to count values; returns how many were decoded: fewer at the end of the data or on a failure */
    UInt32 GetValues( UInt32* out, UInt32 count );

    /**
     *  Bit width 1 only: decodes up to count values as bits into the bitmap (LSB first) starting at bit firstBit; the
     *  bitmap bits must be clear. Adds the number of zero values to zeroCount. Returns how many values were decoded.
     */
    UInt32 GetBits( UInt8* bitmap, UInt32 firstBit, UInt32 count, UInt32& zeroCount );

    /** Skips up to count values without decoding them; returns how many were skipped */
    UInt32 Skip( UInt32 count );

    inline bool HasFailed( void ) const { return m_failed; }
    inline const char* GetError( void ) const { return GUCEF_NULL != m_error ? m_error : ""; }
    inline UInt32 GetBitWidth( void ) const { return m_bitWidth; }

    /** Bytes of the input the runs read so far took */
    inline UInt32 GetBytesConsumed( void ) const { return m_position; }

    /** Extracts value index of bit-packed bytes of the bit width; the caller guarantees the bytes of the value exist */
    static UInt32 ExtractPacked( const UInt8* packed, UInt32 index, UInt32 bitWidth, UInt32 availableBytes );

    private:

    bool NextRun( void );
    bool Fail( const char* message );

    const UInt8* m_data;
    UInt32 m_size;
    UInt32 m_position;
    UInt32 m_bitWidth;
    UInt32 m_valueBytes;
    UInt64 m_maxValue;

    bool m_isRle;
    UInt32 m_runRemaining;                  /**< values left in the current run */
    UInt32 m_rleValue;
    const UInt8* m_packed;                  /**< start of the current bit-packed run */
    UInt32 m_packedBytes;                   /**< bytes of it that are present */
    UInt32 m_packedIndex;                   /**< the next value of the bit-packed run */

    bool m_failed;
    const char* m_error;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace PARQUET */
}; /* namespace TABLECODECPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* TABLECODECPLUGIN_PARQUET_CRLEBITPACKEDDECODER_H ? */
