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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_MACROS_H
#include "gucefCORE_macros.h"
#define GUCEF_CORE_MACROS_H
#endif /* GUCEF_CORE_MACROS_H ? */

#ifndef GUCEF_CORE_CVARIANTBINARYSERIALIZER_H
#include "gucefCORE_CVariantBinarySerializer.h"
#define GUCEF_CORE_CVARIANTBINARYSERIALIZER_H
#endif /* GUCEF_CORE_CVARIANTBINARYSERIALIZER_H ? */

#include "gucefCORE_CVariantStream.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CVariantStream::CVariantStream( void )
    : m_buffer()
    , m_writePosition( 0 )
    , m_readPosition( 0 )
    , m_isValid( true )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CVariantStream::CVariantStream( UInt32 initialBufferSize )
    : m_buffer( initialBufferSize, true )
    , m_writePosition( 0 )
    , m_readPosition( 0 )
    , m_isValid( true )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CVariantStream::CVariantStream( const CVariantStream& src )
    : m_buffer( src.m_buffer )
    , m_writePosition( src.m_writePosition )
    , m_readPosition( src.m_readPosition )
    , m_isValid( src.m_isValid )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

#ifdef GUCEF_MOVE_SEMANTICS_SUPPORTED

CVariantStream::CVariantStream( CVariantStream&& src ) GUCEF_NOEXCEPT
    : m_buffer( GUCEF_MOVE( src.m_buffer ) )
    , m_writePosition( src.m_writePosition )
    , m_readPosition( src.m_readPosition )
    , m_isValid( src.m_isValid )
{GUCEF_TRACE;

    src.m_writePosition = 0;
    src.m_readPosition = 0;
    src.m_isValid = true;
}

#endif

/*-------------------------------------------------------------------------*/

CVariantStream::~CVariantStream()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CVariantStream&
CVariantStream::operator=( const CVariantStream& src )
{GUCEF_TRACE;

    if ( this != &src )
    {
        m_buffer = src.m_buffer;
        m_writePosition = src.m_writePosition;
        m_readPosition = src.m_readPosition;
        m_isValid = src.m_isValid;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

#ifdef GUCEF_MOVE_SEMANTICS_SUPPORTED

CVariantStream&
CVariantStream::operator=( CVariantStream&& src ) GUCEF_NOEXCEPT
{GUCEF_TRACE;

    if ( this != &src )
    {
        m_buffer = GUCEF_MOVE( src.m_buffer );
        m_writePosition = src.m_writePosition;
        m_readPosition = src.m_readPosition;
        m_isValid = src.m_isValid;
        
        src.m_writePosition = 0;
        src.m_readPosition = 0;
        src.m_isValid = true;
    }
    return *this;
}

#endif

/*-------------------------------------------------------------------------*/

CVariantStream&
CVariantStream::operator<<( bool data )
{GUCEF_TRACE;

    CVariant var( data );
    UInt32 bytesWritten = 0;
    if ( CVariantBinarySerializer::Serialize( var, m_writePosition, m_buffer, bytesWritten ) )
    {
        m_writePosition += bytesWritten;
    }
    else
    {
        m_isValid = false;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CVariantStream&
CVariantStream::operator<<( Int8 data )
{GUCEF_TRACE;

    CVariant var( data );
    UInt32 bytesWritten = 0;
    if ( CVariantBinarySerializer::Serialize( var, m_writePosition, m_buffer, bytesWritten ) )
    {
        m_writePosition += bytesWritten;
    }
    else
    {
        m_isValid = false;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CVariantStream&
CVariantStream::operator<<( UInt8 data )
{GUCEF_TRACE;

    CVariant var( data );
    UInt32 bytesWritten = 0;
    if ( CVariantBinarySerializer::Serialize( var, m_writePosition, m_buffer, bytesWritten ) )
    {
        m_writePosition += bytesWritten;
    }
    else
    {
        m_isValid = false;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CVariantStream&
CVariantStream::operator<<( Int16 data )
{GUCEF_TRACE;

    CVariant var( data );
    UInt32 bytesWritten = 0;
    if ( CVariantBinarySerializer::Serialize( var, m_writePosition, m_buffer, bytesWritten ) )
    {
        m_writePosition += bytesWritten;
    }
    else
    {
        m_isValid = false;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CVariantStream&
CVariantStream::operator<<( UInt16 data )
{GUCEF_TRACE;

    CVariant var( data );
    UInt32 bytesWritten = 0;
    if ( CVariantBinarySerializer::Serialize( var, m_writePosition, m_buffer, bytesWritten ) )
    {
        m_writePosition += bytesWritten;
    }
    else
    {
        m_isValid = false;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CVariantStream&
CVariantStream::operator<<( Int32 data )
{GUCEF_TRACE;

    CVariant var( data );
    UInt32 bytesWritten = 0;
    if ( CVariantBinarySerializer::Serialize( var, m_writePosition, m_buffer, bytesWritten ) )
    {
        m_writePosition += bytesWritten;
    }
    else
    {
        m_isValid = false;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CVariantStream&
CVariantStream::operator<<( UInt32 data )
{GUCEF_TRACE;

    CVariant var( data );
    UInt32 bytesWritten = 0;
    if ( CVariantBinarySerializer::Serialize( var, m_writePosition, m_buffer, bytesWritten ) )
    {
        m_writePosition += bytesWritten;
    }
    else
    {
        m_isValid = false;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CVariantStream&
CVariantStream::operator<<( Int64 data )
{GUCEF_TRACE;

    CVariant var( data );
    UInt32 bytesWritten = 0;
    if ( CVariantBinarySerializer::Serialize( var, m_writePosition, m_buffer, bytesWritten ) )
    {
        m_writePosition += bytesWritten;
    }
    else
    {
        m_isValid = false;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CVariantStream&
CVariantStream::operator<<( UInt64 data )
{GUCEF_TRACE;

    CVariant var( data );
    UInt32 bytesWritten = 0;
    if ( CVariantBinarySerializer::Serialize( var, m_writePosition, m_buffer, bytesWritten ) )
    {
        m_writePosition += bytesWritten;
    }
    else
    {
        m_isValid = false;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CVariantStream&
CVariantStream::operator<<( Float32 data )
{GUCEF_TRACE;

    CVariant var( data );
    UInt32 bytesWritten = 0;
    if ( CVariantBinarySerializer::Serialize( var, m_writePosition, m_buffer, bytesWritten ) )
    {
        m_writePosition += bytesWritten;
    }
    else
    {
        m_isValid = false;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CVariantStream&
CVariantStream::operator<<( Float64 data )
{GUCEF_TRACE;

    CVariant var( data );
    UInt32 bytesWritten = 0;
    if ( CVariantBinarySerializer::Serialize( var, m_writePosition, m_buffer, bytesWritten ) )
    {
        m_writePosition += bytesWritten;
    }
    else
    {
        m_isValid = false;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CVariantStream&
CVariantStream::operator<<( const char* data )
{GUCEF_TRACE;

    CVariant var;
    var.LinkTo( data );
    UInt32 bytesWritten = 0;
    if ( CVariantBinarySerializer::Serialize( var, m_writePosition, m_buffer, bytesWritten ) )
    {
        m_writePosition += bytesWritten;
    }
    else
    {
        m_isValid = false;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CVariantStream&
CVariantStream::operator<<( const CAsciiString& data )
{GUCEF_TRACE;

    CVariant var;
    var.LinkTo( data );
    UInt32 bytesWritten = 0;
    if ( CVariantBinarySerializer::Serialize( var, m_writePosition, m_buffer, bytesWritten ) )
    {
        m_writePosition += bytesWritten;
    }
    else
    {
        m_isValid = false;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CVariantStream&
CVariantStream::operator<<( const CUtf8String& data )
{GUCEF_TRACE;

    CVariant var;
    var.LinkTo( data );
    UInt32 bytesWritten = 0;
    if ( CVariantBinarySerializer::Serialize( var, m_writePosition, m_buffer, bytesWritten ) )
    {
        m_writePosition += bytesWritten;
    }
    else
    {
        m_isValid = false;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CVariantStream&
CVariantStream::operator<<( const std::string& data )
{GUCEF_TRACE;

    CVariant var;
    var.LinkTo( data );
    UInt32 bytesWritten = 0;
    if ( CVariantBinarySerializer::Serialize( var, m_writePosition, m_buffer, bytesWritten ) )
    {
        m_writePosition += bytesWritten;
    }
    else
    {
        m_isValid = false;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CVariantStream&
CVariantStream::operator<<( const CDateTime& data )
{GUCEF_TRACE;

    CVariant var( data );
    UInt32 bytesWritten = 0;
    if ( CVariantBinarySerializer::Serialize( var, m_writePosition, m_buffer, bytesWritten ) )
    {
        m_writePosition += bytesWritten;
    }
    else
    {
        m_isValid = false;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CVariantStream&
CVariantStream::operator<<( const CVariant& data )
{GUCEF_TRACE;

    UInt32 bytesWritten = 0;
    if ( CVariantBinarySerializer::Serialize( data, m_writePosition, m_buffer, bytesWritten ) )
    {
        m_writePosition += bytesWritten;
    }
    else
    {
        m_isValid = false;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CVariantStream&
CVariantStream::operator<<( const CDynamicBuffer& data )
{GUCEF_TRACE;

    CVariant var;
    var.LinkTo( data );
    UInt32 bytesWritten = 0;
    if ( CVariantBinarySerializer::Serialize( var, m_writePosition, m_buffer, bytesWritten ) )
    {
        m_writePosition += bytesWritten;
    }
    else
    {
        m_isValid = false;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CVariantStream&
CVariantStream::operator<<( void* data )
{GUCEF_TRACE;

    CVariant var( data );
    UInt32 bytesWritten = 0;
    if ( CVariantBinarySerializer::Serialize( var, m_writePosition, m_buffer, bytesWritten ) )
    {
        m_writePosition += bytesWritten;
    }
    else
    {
        m_isValid = false;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CVariantStream&
CVariantStream::operator<<( TDefaultFuncPtr data )
{GUCEF_TRACE;

    CVariant var( data );
    UInt32 bytesWritten = 0;
    if ( CVariantBinarySerializer::Serialize( var, m_writePosition, m_buffer, bytesWritten ) )
    {
        m_writePosition += bytesWritten;
    }
    else
    {
        m_isValid = false;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CVariantStream&
CVariantStream::operator>>( bool& data )
{GUCEF_TRACE;

    CVariant var;
    UInt32 bytesRead = 0;
    if ( CVariantBinarySerializer::Deserialize( var, m_readPosition, m_buffer, false, bytesRead ) )
    {
        m_readPosition += bytesRead;
        data = var.AsBool();
    }
    else
    {
        m_isValid = false;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CVariantStream&
CVariantStream::operator>>( Int8& data )
{GUCEF_TRACE;

    CVariant var;
    UInt32 bytesRead = 0;
    if ( CVariantBinarySerializer::Deserialize( var, m_readPosition, m_buffer, false, bytesRead ) )
    {
        m_readPosition += bytesRead;
        data = var.AsInt8();
    }
    else
    {
        m_isValid = false;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CVariantStream&
CVariantStream::operator>>( UInt8& data )
{GUCEF_TRACE;

    CVariant var;
    UInt32 bytesRead = 0;
    if ( CVariantBinarySerializer::Deserialize( var, m_readPosition, m_buffer, false, bytesRead ) )
    {
        m_readPosition += bytesRead;
        data = var.AsUInt8();
    }
    else
    {
        m_isValid = false;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CVariantStream&
CVariantStream::operator>>( Int16& data )
{GUCEF_TRACE;

    CVariant var;
    UInt32 bytesRead = 0;
    if ( CVariantBinarySerializer::Deserialize( var, m_readPosition, m_buffer, false, bytesRead ) )
    {
        m_readPosition += bytesRead;
        data = var.AsInt16();
    }
    else
    {
        m_isValid = false;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CVariantStream&
CVariantStream::operator>>( UInt16& data )
{GUCEF_TRACE;

    CVariant var;
    UInt32 bytesRead = 0;
    if ( CVariantBinarySerializer::Deserialize( var, m_readPosition, m_buffer, false, bytesRead ) )
    {
        m_readPosition += bytesRead;
        data = var.AsUInt16();
    }
    else
    {
        m_isValid = false;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CVariantStream&
CVariantStream::operator>>( Int32& data )
{GUCEF_TRACE;

    CVariant var;
    UInt32 bytesRead = 0;
    if ( CVariantBinarySerializer::Deserialize( var, m_readPosition, m_buffer, false, bytesRead ) )
    {
        m_readPosition += bytesRead;
        data = var.AsInt32();
    }
    else
    {
        m_isValid = false;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CVariantStream&
CVariantStream::operator>>( UInt32& data )
{GUCEF_TRACE;

    CVariant var;
    UInt32 bytesRead = 0;
    if ( CVariantBinarySerializer::Deserialize( var, m_readPosition, m_buffer, false, bytesRead ) )
    {
        m_readPosition += bytesRead;
        data = var.AsUInt32();
    }
    else
    {
        m_isValid = false;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CVariantStream&
CVariantStream::operator>>( Int64& data )
{GUCEF_TRACE;

    CVariant var;
    UInt32 bytesRead = 0;
    if ( CVariantBinarySerializer::Deserialize( var, m_readPosition, m_buffer, false, bytesRead ) )
    {
        m_readPosition += bytesRead;
        data = var.AsInt64();
    }
    else
    {
        m_isValid = false;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CVariantStream&
CVariantStream::operator>>( UInt64& data )
{GUCEF_TRACE;

    CVariant var;
    UInt32 bytesRead = 0;
    if ( CVariantBinarySerializer::Deserialize( var, m_readPosition, m_buffer, false, bytesRead ) )
    {
        m_readPosition += bytesRead;
        data = var.AsUInt64();
    }
    else
    {
        m_isValid = false;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CVariantStream&
CVariantStream::operator>>( Float32& data )
{GUCEF_TRACE;

    CVariant var;
    UInt32 bytesRead = 0;
    if ( CVariantBinarySerializer::Deserialize( var, m_readPosition, m_buffer, false, bytesRead ) )
    {
        m_readPosition += bytesRead;
        data = var.AsFloat32();
    }
    else
    {
        m_isValid = false;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CVariantStream&
CVariantStream::operator>>( Float64& data )
{GUCEF_TRACE;

    CVariant var;
    UInt32 bytesRead = 0;
    if ( CVariantBinarySerializer::Deserialize( var, m_readPosition, m_buffer, false, bytesRead ) )
    {
        m_readPosition += bytesRead;
        data = var.AsFloat64();
    }
    else
    {
        m_isValid = false;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CVariantStream&
CVariantStream::operator>>( CAsciiString& data )
{GUCEF_TRACE;

    CVariant var;
    UInt32 bytesRead = 0;
    if ( CVariantBinarySerializer::Deserialize( var, m_readPosition, m_buffer, false, bytesRead ) )
    {
        m_readPosition += bytesRead;
        data = var.AsAsciiString();
    }
    else
    {
        m_isValid = false;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CVariantStream&
CVariantStream::operator>>( CUtf8String& data )
{GUCEF_TRACE;

    CVariant var;
    UInt32 bytesRead = 0;
    if ( CVariantBinarySerializer::Deserialize( var, m_readPosition, m_buffer, false, bytesRead ) )
    {
        m_readPosition += bytesRead;
        data = var.AsUtf8String();
    }
    else
    {
        m_isValid = false;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CVariantStream&
CVariantStream::operator>>( std::string& data )
{GUCEF_TRACE;

    CVariant var;
    UInt32 bytesRead = 0;
    if ( CVariantBinarySerializer::Deserialize( var, m_readPosition, m_buffer, false, bytesRead ) )
    {
        m_readPosition += bytesRead;
        data = var.AsString().STL_String();
    }
    else
    {
        m_isValid = false;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CVariantStream&
CVariantStream::operator>>( CDateTime& data )
{GUCEF_TRACE;

    CVariant var;
    UInt32 bytesRead = 0;
    if ( CVariantBinarySerializer::Deserialize( var, m_readPosition, m_buffer, false, bytesRead ) )
    {
        m_readPosition += bytesRead;
        data = var.AsDateTime();
    }
    else
    {
        m_isValid = false;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CVariantStream&
CVariantStream::operator>>( CVariant& data )
{GUCEF_TRACE;

    UInt32 bytesRead = 0;
    if ( CVariantBinarySerializer::Deserialize( data, m_readPosition, m_buffer, false, bytesRead ) )
    {
        m_readPosition += bytesRead;
    }
    else
    {
        m_isValid = false;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CVariantStream&
CVariantStream::operator>>( CDynamicBuffer& data )
{GUCEF_TRACE;

    CVariant var;
    UInt32 bytesRead = 0;
    if ( CVariantBinarySerializer::Deserialize( var, m_readPosition, m_buffer, false, bytesRead ) )
    {
        m_readPosition += bytesRead;
        data = var.AsBuffer();
    }
    else
    {
        m_isValid = false;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

void
CVariantStream::Clear( void )
{GUCEF_TRACE;

    m_buffer.Clear();
    m_writePosition = 0;
    m_readPosition = 0;
    m_isValid = true;
}

/*-------------------------------------------------------------------------*/

UInt32
CVariantStream::GetDataSize( void ) const
{GUCEF_TRACE;

    return m_buffer.GetDataSize();
}

/*-------------------------------------------------------------------------*/

UInt32
CVariantStream::GetWritePosition( void ) const
{GUCEF_TRACE;

    return m_writePosition;
}

/*-------------------------------------------------------------------------*/

UInt32
CVariantStream::GetReadPosition( void ) const
{GUCEF_TRACE;

    return m_readPosition;
}

/*-------------------------------------------------------------------------*/

void
CVariantStream::SetReadPosition( UInt32 position )
{GUCEF_TRACE;

    m_readPosition = position;
}

/*-------------------------------------------------------------------------*/

void
CVariantStream::ResetReadPosition( void )
{GUCEF_TRACE;

    m_readPosition = 0;
}

/*-------------------------------------------------------------------------*/

bool
CVariantStream::HasUnreadData( void ) const
{GUCEF_TRACE;

    return m_readPosition < m_buffer.GetDataSize();
}

/*-------------------------------------------------------------------------*/

UInt32
CVariantStream::GetUnreadDataSize( void ) const
{GUCEF_TRACE;

    UInt32 dataSize = m_buffer.GetDataSize();
    if ( m_readPosition < dataSize )
        return dataSize - m_readPosition;
    return 0;
}

/*-------------------------------------------------------------------------*/

const CDynamicBuffer&
CVariantStream::GetBuffer( void ) const
{GUCEF_TRACE;

    return m_buffer;
}

/*-------------------------------------------------------------------------*/

CDynamicBuffer&
CVariantStream::GetBuffer( void )
{GUCEF_TRACE;

    return m_buffer;
}

/*-------------------------------------------------------------------------*/

bool
CVariantStream::IsValid( void ) const
{GUCEF_TRACE;

    return m_isValid;
}

/*-------------------------------------------------------------------------*/

void
CVariantStream::ResetValidState( void )
{GUCEF_TRACE;

    m_isValid = true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
