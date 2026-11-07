/*
 *  gucefMULTIPLAY: GUCEF module providing multiplayer RAD functionality
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

#include "gucefMULTIPLAY_CMatchBinaryWriter.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MULTIPLAY {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CMatchBinaryWriter::CMatchBinaryWriter( CORE::CDynamicBuffer& target )
    : m_target( target )
{GUCEF_TRACE;

    // many small appends: the buffer grows by a percentage, not by exactly what each append needs
    m_target.SetGrowthStrategy( CORE::CDynamicBuffer::GROWTHSTRATEGY_GEOMETRIC );

}

/*-------------------------------------------------------------------------*/

void
CMatchBinaryWriter::WriteUInt8( UInt8 value )
{GUCEF_TRACE;

    m_target.Append( &value, 1 );
}

/*-------------------------------------------------------------------------*/

void
CMatchBinaryWriter::WriteUInt16( UInt16 value )
{GUCEF_TRACE;

    UInt8 bytes[ 2 ] = { (UInt8)( value >> 8 ), (UInt8) value };
    m_target.Append( bytes, 2 );
}

/*-------------------------------------------------------------------------*/

void
CMatchBinaryWriter::WriteUInt32( UInt32 value )
{GUCEF_TRACE;

    UInt8 bytes[ 4 ] = { (UInt8)( value >> 24 ), (UInt8)( value >> 16 ), (UInt8)( value >> 8 ), (UInt8) value };
    m_target.Append( bytes, 4 );
}

/*-------------------------------------------------------------------------*/

void
CMatchBinaryWriter::WriteUInt64( UInt64 value )
{GUCEF_TRACE;

    WriteUInt32( (UInt32)( value >> 32 ) );
    WriteUInt32( (UInt32)( value & 0xFFFFFFFFu ) );
}

/*-------------------------------------------------------------------------*/

void
CMatchBinaryWriter::WriteInt64( Int64 value )
{GUCEF_TRACE;

    WriteUInt64( (UInt64) value );
}

/*-------------------------------------------------------------------------*/

void
CMatchBinaryWriter::WriteBool( bool value )
{GUCEF_TRACE;

    WriteUInt8( value ? 1 : 0 );
}

/*-------------------------------------------------------------------------*/

void
CMatchBinaryWriter::WriteRaw( const void* data, UInt32 size )
{GUCEF_TRACE;

    if ( size > 0 && GUCEF_NULL != data )
        m_target.Append( data, size );
}

/*-------------------------------------------------------------------------*/

void
CMatchBinaryWriter::WriteBytes( const void* data, UInt32 size )
{GUCEF_TRACE;

    WriteUInt32( size );
    WriteRaw( data, size );
}

/*-------------------------------------------------------------------------*/

void
CMatchBinaryWriter::WriteString( const CORE::CString& text )
{GUCEF_TRACE;

    // the strings are UTF-8: the length that counts is the one in bytes
    WriteBytes( text.C_String(), (UInt32) strlen( text.C_String() ) );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MULTIPLAY */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
