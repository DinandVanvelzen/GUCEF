/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
 *  Copyright (C) 2002 - 2008.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#ifndef GUCEF_CORE_CVARIANTSTREAM_H
#define GUCEF_CORE_CVARIANTSTREAM_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_MACROS_H
#include "gucefCORE_macros.h"
#define GUCEF_CORE_MACROS_H
#endif /* GUCEF_CORE_MACROS_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_CORE_CVARIANT_H
#include "gucefCORE_CVariant.h"
#define GUCEF_CORE_CVARIANT_H
#endif /* GUCEF_CORE_CVARIANT_H ? */

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

/**
 *  A stream class that wraps CDynamicBuffer and provides stream-like << and >> operators
 *  for writing and reading variant data. Each value written is stored as a CVariant using
 *  the CVariantBinarySerializer format, preserving type information without string conversion.
 *  
 *  This class is designed for high-performance logging scenarios where per-thread streams
 *  can accumulate log data without costly string conversions during the logging operation itself.
 */
class GUCEF_CORE_PUBLIC_CPP CVariantStream
{
    public:

    CVariantStream( void );

    explicit CVariantStream( UInt32 initialBufferSize );

    CVariantStream( const CVariantStream& src );

    #ifdef GUCEF_MOVE_SEMANTICS_SUPPORTED
    CVariantStream( CVariantStream&& src ) GUCEF_NOEXCEPT;
    #endif

    ~CVariantStream();

    CVariantStream& operator=( const CVariantStream& src );

    #ifdef GUCEF_MOVE_SEMANTICS_SUPPORTED
    CVariantStream& operator=( CVariantStream&& src ) GUCEF_NOEXCEPT;
    #endif

    /**
     *  Stream insertion operators for writing variant data
     *  Each value is serialized as a CVariant, preserving type information
     */
    CVariantStream& operator<<( bool data );
    CVariantStream& operator<<( Int8 data );
    CVariantStream& operator<<( UInt8 data );
    CVariantStream& operator<<( Int16 data );
    CVariantStream& operator<<( UInt16 data );
    CVariantStream& operator<<( Int32 data );
    CVariantStream& operator<<( UInt32 data );
    CVariantStream& operator<<( Int64 data );
    CVariantStream& operator<<( UInt64 data );
    CVariantStream& operator<<( Float32 data );
    CVariantStream& operator<<( Float64 data );
    CVariantStream& operator<<( const char* data );
    CVariantStream& operator<<( const CAsciiString& data );
    CVariantStream& operator<<( const CUtf8String& data );
    CVariantStream& operator<<( const std::string& data );
    CVariantStream& operator<<( const CDateTime& data );
    CVariantStream& operator<<( const CVariant& data );
    CVariantStream& operator<<( const CDynamicBuffer& data );
    CVariantStream& operator<<( void* data );
    CVariantStream& operator<<( TDefaultFuncPtr data );

    /**
     *  Stream extraction operators for reading variant data
     *  Each value is deserialized from a CVariant, with automatic type conversion if needed
     */
    CVariantStream& operator>>( bool& data );
    CVariantStream& operator>>( Int8& data );
    CVariantStream& operator>>( UInt8& data );
    CVariantStream& operator>>( Int16& data );
    CVariantStream& operator>>( UInt16& data );
    CVariantStream& operator>>( Int32& data );
    CVariantStream& operator>>( UInt32& data );
    CVariantStream& operator>>( Int64& data );
    CVariantStream& operator>>( UInt64& data );
    CVariantStream& operator>>( Float32& data );
    CVariantStream& operator>>( Float64& data );
    CVariantStream& operator>>( CAsciiString& data );
    CVariantStream& operator>>( CUtf8String& data );
    CVariantStream& operator>>( std::string& data );
    CVariantStream& operator>>( CDateTime& data );
    CVariantStream& operator>>( CVariant& data );
    CVariantStream& operator>>( CDynamicBuffer& data );

    /**
     *  Clears all data from the stream and resets the read/write positions
     */
    void Clear( void );

    /**
     *  Returns the total number of bytes written to the stream
     */
    UInt32 GetDataSize( void ) const;

    /**
     *  Returns the current write position
     */
    UInt32 GetWritePosition( void ) const;

    /**
     *  Returns the current read position
     */
    UInt32 GetReadPosition( void ) const;

    /**
     *  Sets the read position for subsequent extraction operations
     */
    void SetReadPosition( UInt32 position );

    /**
     *  Resets the read position to the beginning of the stream
     */
    void ResetReadPosition( void );

    /**
     *  Returns whether there is more data available to read
     */
    bool HasUnreadData( void ) const;

    /**
     *  Returns the number of bytes remaining to be read
     */
    UInt32 GetUnreadDataSize( void ) const;

    /**
     *  Returns direct access to the underlying buffer (read-only)
     */
    const CDynamicBuffer& GetBuffer( void ) const;

    /**
     *  Returns direct access to the underlying buffer
     */
    CDynamicBuffer& GetBuffer( void );

    /**
     *  Returns whether the stream is in a valid state (last operation succeeded)
     */
    bool IsValid( void ) const;

    /**
     *  Resets the valid state (useful after handling an error)
     */
    void ResetValidState( void );

    private:

    CDynamicBuffer m_buffer;
    UInt32 m_writePosition;
    UInt32 m_readPosition;
    bool m_isValid;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CVARIANTSTREAM_H ? */
