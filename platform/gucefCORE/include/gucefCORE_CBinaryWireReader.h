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

#ifndef GUCEF_CORE_CBINARYWIREREADER_H
#define GUCEF_CORE_CBINARYWIREREADER_H

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
 *  Reads the binary wire encoding of CBinaryWireWriter from memory it does not own.
 *
 *  Every read checks the remaining size and the encoding: a read past the end, a varint longer than 10 bytes or
 *  above the width asked for, invalid UTF-8 in a string, a decimal scale above CDecimal::MaxScale fail, return
 *  false and mark the reader as failed, after which every further read fails too. The reader never allocates
 *  for views (ReadSizedBytes with pointer output).
 */
class GUCEF_CORE_PUBLIC_CPP CBinaryWireReader
{
    public:

    CBinaryWireReader( const void* data, UInt32 size );

    explicit CBinaryWireReader( const CDynamicBuffer& buffer, UInt32 startOffset = 0 );

    bool ReadUInt8( UInt8& value );
    bool ReadBool( bool& value );                   /**< one byte, 0 or 1; other values fail */
    bool ReadFixed32( UInt32& value );              /**< little endian */
    bool ReadFixed64( UInt64& value );              /**< little endian */
    bool ReadFloat32( Float32& value );
    bool ReadFloat64( Float64& value );
    bool ReadUInt64BigEndian( UInt64& value );

    bool ReadVarUInt32( UInt32& value );            /**< fails above 32 bits */
    bool ReadVarUInt64( UInt64& value );
    bool ReadVarInt32( Int32& value );              /**< zig-zag; fails above 32 bits */
    bool ReadVarInt64( Int64& value );              /**< zig-zag */

    bool ReadBytes( void* dest, UInt32 size );
    bool SkipBytes( UInt32 size );

    /** varint length + bytes, gives a view on the bytes without copying them */
    bool ReadSizedBytes( const UInt8*& data, UInt32& size );

    /** varint length + bytes, copied into dest */
    bool ReadSizedBytes( CDynamicBuffer& dest );

    /** varint length + UTF-8 bytes, validated, converted to the build's string format */
    bool ReadString( CString& str );

    /** varint length + UTF-8 bytes, validated, as a view */
    bool ReadUtf8( const char*& utf8, UInt32& byteSize );

    bool ReadDecimal( CDecimal& value );
    bool ReadDate( Int32& daysSinceEpoch );
    bool ReadTimestamp( Int64& microsSinceEpoch );
    bool ReadDateTime( CDateTime& value );

    /**
     *  One byte type id, then the value (see CBinaryWireWriter).
     *  With linkBinary a binary value links to the memory being read instead of copying it: valid while that lives.
     */
    bool ReadVariant( CVariant& value, bool linkBinary = false );

    UInt32 GetRemaining( void ) const;
    UInt32 GetPosition( void ) const;
    UInt32 GetSize( void ) const;
    const UInt8* GetCurrentPtr( void ) const;
    const UInt8* GetData( void ) const;
    bool HasFailed( void ) const;
    bool IsAtEnd( void ) const;

    /** Marks the reader as failed, for callers that find the content invalid. Returns false */
    bool Fail( void );

    /** True when the bytes are well-formed UTF-8 (no overlongs, no surrogates, at most U+10FFFF) */
    static bool IsValidUtf8( const void* data, UInt32 size );

    static Int32 ZigZagDecode32( UInt32 value ) { return (Int32) ( value >> 1 ) ^ -(Int32) ( value & 1 ); }
    static Int64 ZigZagDecode64( UInt64 value ) { return (Int64) ( value >> 1 ) ^ -(Int64) ( value & 1 ); }

    private:

    CBinaryWireReader( void );                                          /**< not implemented */

    const UInt8* m_data;
    UInt32 m_size;
    UInt32 m_pos;
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

#endif /* GUCEF_CORE_CBINARYWIREREADER_H ? */
