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

#ifndef TABLECODECPLUGIN_PARQUET_CRLEBITPACKEDENCODER_H
#define TABLECODECPLUGIN_PARQUET_CRLEBITPACKEDENCODER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

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
 *  Encodes the RLE / bit-packed hybrid (see CRleBitPackedDecoder), appending to a buffer. Repeats of 8 or more values
 *  become RLE runs, the rest bit-packed groups of 8 (the last group padded with zeros). Bitmaps (definition levels from
 *  a validity bitmap, booleans) are encoded byte wise: stretches of two or more bytes that are all set or all clear as
 *  RLE runs, the other bytes as bit-packed groups that are the bitmap bytes themselves.
 */
class TABLECODECPLUGIN_PARQUET_PUBLIC_CPP CRleBitPackedEncoder
{
    public:

    /** The smallest bit width that holds maxValue (0 for 0) */
    static UInt32 BitWidthFor( UInt32 maxValue );

    /** Every value must fit the bit width (0..32) */
    static void EncodeValues( const UInt32* values, UInt32 count, UInt32 bitWidth, CORE::CDynamicBuffer& output );

    /** count bits of the bitmap (LSB first) with bit width 1 */
    static void EncodeBits( const UInt8* bitmap, UInt32 count, CORE::CDynamicBuffer& output );

    /** count values that are all the given bit (bit width 1), one RLE run */
    static void EncodeRepeatedBit( bool bit, UInt32 count, CORE::CDynamicBuffer& output );

    static void WriteVarint( UInt64 value, CORE::CDynamicBuffer& output );

    private:

    static void WriteRleRun( UInt32 value, UInt32 count, UInt32 bitWidth, CORE::CDynamicBuffer& output );
    static void WriteLiteralRun( const UInt32* values, UInt32 count, UInt32 groups, UInt32 bitWidth, CORE::CDynamicBuffer& output );

    CRleBitPackedEncoder( void );       /**< not implemented: static members only */
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

#endif /* TABLECODECPLUGIN_PARQUET_CRLEBITPACKEDENCODER_H ? */
