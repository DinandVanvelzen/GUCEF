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

#ifndef GUCEF_DRN_CDRNWIREREADER_H
#define GUCEF_DRN_CDRNWIREREADER_H

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
 *  Reads the DRN wire encoding (see CDRNWireWriter) from memory it does not own.
 *  Every read checks the remaining size: a read past the end fails, returns false and marks the reader as failed,
 *  after which every further read fails too.
 */
class GUCEF_DRN_EXPORT_CPP CDRNWireReader
{
    public:

    CDRNWireReader( const void* data, UInt32 size );

    explicit CDRNWireReader( const CORE::CDynamicBuffer& buffer, UInt32 startOffset = 0 );

    bool ReadUInt8( UInt8& value );
    bool ReadUInt16( UInt16& value );
    bool ReadUInt32( UInt32& value );
    bool ReadUInt64( UInt64& value );
    bool ReadFloat32( Float32& value );
    bool ReadFloat64( Float64& value );
    bool ReadBool( bool& value );

    bool ReadVarUInt32( UInt32& value );
    bool ReadVarUInt64( UInt64& value );
    bool ReadVarInt32( Int32& value );
    bool ReadVarInt64( Int64& value );

    bool ReadBytes( void* dest, UInt32 size );
    bool SkipBytes( UInt32 size );

    /** varint length + bytes, gives a view on the bytes without copying them */
    bool ReadSizedBytes( const UInt8*& data, UInt32& size );

    /** varint length + bytes, copied into dest */
    bool ReadSizedBytes( CORE::CDynamicBuffer& dest );

    bool ReadString( CORE::CString& str );

    UInt32 GetRemaining( void ) const;
    UInt32 GetPosition( void ) const;
    UInt32 GetSize( void ) const;
    const UInt8* GetCurrentPtr( void ) const;
    bool HasFailed( void ) const;

    static Int32 ZigZagDecode32( UInt32 value ) { return (Int32) ( value >> 1 ) ^ -(Int32) ( value & 1 ); }
    static Int64 ZigZagDecode64( UInt64 value ) { return (Int64) ( value >> 1 ) ^ -(Int64) ( value & 1 ); }

    private:

    bool Fail( void );

    CDRNWireReader( void );

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

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRN_CDRNWIREREADER_H ? */
