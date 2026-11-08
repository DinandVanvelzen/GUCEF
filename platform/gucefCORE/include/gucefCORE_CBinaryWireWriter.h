/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
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

#ifndef GUCEF_CORE_CBINARYWIREWRITER_H
#define GUCEF_CORE_CBINARYWIREWRITER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_CORE_MACROS_H
#include "gucefCORE_macros.h"
#define GUCEF_CORE_MACROS_H
#endif /* GUCEF_CORE_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CDecimal;
class CDateTime;
class CVariant;

/*-------------------------------------------------------------------------*/

/**
 *  Writes a compact binary wire encoding into a dynamic buffer, the counterpart of CBinaryWireReader.
 *
 *  Encodings (the value layer of protobuf, with GUCEF's mapping for signed integers and typed values):
 *      - varint: unsigned LEB128, at most 10 bytes for 64 bits
 *      - signed integers: zig-zag, then varint
 *      - fixed 32 / 64 bit integers and IEEE 754 floats: little endian
 *      - sized bytes: varint length, then the bytes; strings are always UTF-8 on the wire whatever the build's
 *        string format is
 *      - decimal: zig-zag unscaled value, varint scale (scale 0 ... 18 preserved)
 *      - date: zig-zag days since 1970-01-01; timestamp and duration: zig-zag microseconds
 *      - date time: zig-zag milliseconds since the Unix epoch in UTC, zig-zag UTC offset in minutes
 *      - variant: one byte GUCEF type id, then the value by the rules above for that id (integers by their
 *        signedness, strings as UTF-8, binary as sized bytes); ids without a binary rule travel in string form
 *
 *  The buffer only grows: once it is large enough writing does not allocate. The logical data size of the buffer
 *  follows the furthest byte written. Not thread safe: one writer per buffer.
 *
 *  A write that would pass the end of the 32 bit range, the maximum size (SetMaxSize()) or for which the buffer
 *  cannot grow writes nothing and marks the writer failed (HasFailed()); every later write then writes nothing as
 *  well, so an encoder checks once at its end.
 */
class GUCEF_CORE_PUBLIC_CPP CBinaryWireWriter
{
    public:

    /** Longest varint of a 64 bit value */
    static const UInt32 MaxVarIntSize = 10;

    explicit CBinaryWireWriter( CDynamicBuffer& buffer, UInt32 startOffset = 0 );

    void WriteUInt8( UInt8 value );
    void WriteBool( bool value );                       /**< one byte, 0 or 1 */
    void WriteFixed32( UInt32 value );                  /**< little endian */
    void WriteFixed64( UInt64 value );                  /**< little endian */
    void WriteFloat32( Float32 value );                 /**< IEEE 754 little endian */
    void WriteFloat64( Float64 value );                 /**< IEEE 754 little endian */
    void WriteUInt64BigEndian( UInt64 value );          /**< network order, for hash inputs that specify it */

    void WriteVarUInt32( UInt32 value );
    void WriteVarUInt64( UInt64 value );
    void WriteVarInt32( Int32 value );                  /**< zig-zag */
    void WriteVarInt64( Int64 value );                  /**< zig-zag */

    /** raw bytes, no length */
    void WriteBytes( const void* data, UInt32 size );

    /** varint length + bytes */
    void WriteSizedBytes( const void* data, UInt32 size );

    /** varint length + bytes of the buffer's data */
    void WriteSizedBytes( const CDynamicBuffer& data );

    /** varint byte length + UTF-8 bytes, no terminator */
    void WriteString( const CString& str );

    /** varint byte length + the given UTF-8 bytes */
    void WriteUtf8( const char* utf8, UInt32 byteSize );

    /** zig-zag unscaled value, varint scale */
    void WriteDecimal( const CDecimal& value );

    /** zig-zag days since 1970-01-01 */
    void WriteDate( Int32 daysSinceEpoch );

    /** zig-zag microseconds since the Unix epoch, UTC; also the encoding of a duration in microseconds */
    void WriteTimestamp( Int64 microsSinceEpoch );

    /**
     *  zig-zag milliseconds since the Unix epoch in UTC, zig-zag UTC offset in minutes.
     *  False (nothing written) for a date time outside the 64 bit millisecond range.
     */
    bool WriteDateTime( const CDateTime& value );

    /** one byte type id, then the value (see the class comment). False when the variant cannot be written */
    bool WriteVariant( const CVariant& value );

    /**
     *  Makes room for size bytes at the current offset and moves past them
     *  @return pointer to the room, valid until the next write; GUCEF_NULL when the writer failed (HasFailed())
     */
    UInt8* Reserve( UInt32 size );

    /** True once a write did not fit (see the class comment); the writer stays failed */
    bool HasFailed( void ) const;

    /** The largest offset a write may reach, 0 (the default) for no maximum within the 32 bit range */
    void SetMaxSize( UInt32 maxBytes );
    UInt32 GetMaxSize( void ) const;

    UInt32 GetOffset( void ) const;
    void SetOffset( UInt32 offset );
    CDynamicBuffer& GetBuffer( void );

    static UInt32 GetVarUInt32Size( UInt32 value );
    static UInt32 GetVarUInt64Size( UInt64 value );

    /** Encodes a varint into dest which must hold MaxVarIntSize bytes, returns the number of bytes written */
    static UInt32 EncodeVarUInt64( UInt64 value, UInt8* dest );

    static UInt32 ZigZagEncode32( Int32 value ) { return ( (UInt32) value << 1 ) ^ (UInt32) ( value >> 31 ); }
    static UInt64 ZigZagEncode64( Int64 value ) { return ( (UInt64) value << 1 ) ^ (UInt64) ( value >> 63 ); }

    private:

    UInt8* MakeRoom( UInt32 size );

    CBinaryWireWriter( void );                                          /**< not implemented */
    CBinaryWireWriter( const CBinaryWireWriter& src );                  /**< not implemented */
    CBinaryWireWriter& operator=( const CBinaryWireWriter& src );       /**< not implemented */

    CDynamicBuffer* m_buffer;
    UInt32 m_offset;
    UInt32 m_maxSize;
    bool m_failed;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CBINARYWIREWRITER_H ? */
