/*
 *  gucefDRN: GUCEF module providing RAD networking trough data replication
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

#ifndef GUCEF_DRN_CDRNWIREWRITER_H
#define GUCEF_DRN_CDRNWIREWRITER_H

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

#ifndef GUCEF_DRN_MACROS_H
#include "gucefDRN_macros.h"
#define GUCEF_DRN_MACROS_H
#endif /* GUCEF_DRN_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRN {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Writes the DRN wire encoding into a dynamic buffer.
 *
 *  Integers that are not varints are big endian. Varints are unsigned LEB128, signed values are zig-zag encoded.
 *  Strings are UTF-8 prefixed with their byte length as a varint. The buffer only grows: once it is large enough
 *  writing does not allocate. The logical data size of the buffer follows the furthest byte written.
 */
class GUCEF_DRN_EXPORT_CPP CDRNWireWriter
{
    public:

    explicit CDRNWireWriter( CORE::CDynamicBuffer& buffer, UInt32 startOffset = 0 );

    void WriteUInt8( UInt8 value );
    void WriteUInt16( UInt16 value );
    void WriteUInt32( UInt32 value );
    void WriteUInt64( UInt64 value );
    void WriteFloat32( Float32 value );
    void WriteFloat64( Float64 value );
    void WriteBool( bool value );

    void WriteVarUInt32( UInt32 value );
    void WriteVarUInt64( UInt64 value );
    void WriteVarInt32( Int32 value );
    void WriteVarInt64( Int64 value );

    /** raw bytes, no length */
    void WriteBytes( const void* data, UInt32 size );

    /** varint length + bytes */
    void WriteSizedBytes( const void* data, UInt32 size );

    /** varint byte length + UTF-8 bytes, no terminator */
    void WriteString( const CORE::CString& str );

    /**
     *  Makes room for size bytes at the current offset and moves past them
     *  @return pointer to the room, valid until the next write
     */
    UInt8* Reserve( UInt32 size );

    UInt32 GetOffset( void ) const;
    void SetOffset( UInt32 offset );
    CORE::CDynamicBuffer& GetBuffer( void );

    static UInt32 GetVarUInt32Size( UInt32 value );
    static UInt32 GetVarUInt64Size( UInt64 value );

    /** Encodes a varint into dest which must hold 10 bytes, returns the number of bytes written */
    static UInt32 EncodeVarUInt64( UInt64 value, UInt8* dest );

    static UInt32 ZigZagEncode32( Int32 value ) { return ( (UInt32) value << 1 ) ^ (UInt32) ( value >> 31 ); }
    static UInt64 ZigZagEncode64( Int64 value ) { return ( (UInt64) value << 1 ) ^ (UInt64) ( value >> 63 ); }

    private:

    UInt8* MakeRoom( UInt32 size );

    CDRNWireWriter( void );

    CORE::CDynamicBuffer* m_buffer;
    UInt32 m_offset;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRN_CDRNWIREWRITER_H ? */
