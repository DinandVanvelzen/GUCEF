/*
 *  dbpluginPOSTGRESQL: gucefCOMCORE database backend for PostgreSQL
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

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#include "dbpluginPOSTGRESQL_CPostgreSqlParameters.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DBPLUGIN {
namespace POSTGRESQL {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CPostgreSqlParameters::CPostgreSqlParameters( void )
    : m_bytes()
    , m_offsets()
    , m_lengths()
    , m_formats()
    , m_kinds()
    , m_values()
    , m_prepared( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

UInt32
CPostgreSqlParameters::Add( EKind kind, const void* bytes, UInt32 byteLength )
{GUCEF_TRACE;

    if ( m_kinds.size() >= MaxCount )
        return 0;
    m_prepared = false;
    m_offsets.push_back( (UInt32) m_bytes.size() );
    m_kinds.push_back( (UInt8) kind );
    m_formats.push_back( KIND_BINARY == kind ? 1 : 0 );
    m_lengths.push_back( KIND_NULL == kind ? 0 : (int) byteLength );
    if ( KIND_NULL != kind )
    {
        const char* first = static_cast< const char* >( bytes );
        if ( byteLength > 0 )
            m_bytes.insert( m_bytes.end(), first, first + byteLength );
        m_bytes.push_back( '\0' );
    }
    return (UInt32) m_kinds.size();
}

/*-------------------------------------------------------------------------*/

UInt32
CPostgreSqlParameters::AddText( const char* text, UInt32 byteLength )
{GUCEF_TRACE;

    // PostgreSQL text cannot hold a NUL byte, and libpq takes text values up to the first one
    if ( byteLength > 0 && GUCEF_NULL != memchr( text, 0, byteLength ) )
        return 0;
    return Add( KIND_TEXT, text, byteLength );
}

/*-------------------------------------------------------------------------*/

UInt32
CPostgreSqlParameters::AddText( const CORE::CString& text )
{GUCEF_TRACE;

    const CORE::CUtf8String utf8 = CORE::ToUtf8String( text );
    return AddText( utf8.C_String(), utf8.ByteSize() > 0 ? utf8.ByteSize() - 1 : 0 );
}

/*-------------------------------------------------------------------------*/

UInt32
CPostgreSqlParameters::AddBinary( const void* bytes, UInt32 byteLength )
{GUCEF_TRACE;

    return Add( KIND_BINARY, bytes, byteLength );
}

/*-------------------------------------------------------------------------*/

UInt32
CPostgreSqlParameters::AddNull( void )
{GUCEF_TRACE;

    return Add( KIND_NULL, GUCEF_NULL, 0 );
}

/*-------------------------------------------------------------------------*/

UInt32
CPostgreSqlParameters::GetCount( void ) const
{GUCEF_TRACE;

    return (UInt32) m_kinds.size();
}

/*-------------------------------------------------------------------------*/

bool
CPostgreSqlParameters::IsNull( UInt32 number ) const
{GUCEF_TRACE;

    return number >= 1 && number <= m_kinds.size() && KIND_NULL == m_kinds[ number-1 ];
}

/*-------------------------------------------------------------------------*/

bool
CPostgreSqlParameters::IsBinary( UInt32 number ) const
{GUCEF_TRACE;

    return number >= 1 && number <= m_kinds.size() && KIND_BINARY == m_kinds[ number-1 ];
}

/*-------------------------------------------------------------------------*/

CORE::CString
CPostgreSqlParameters::GetDisplayText( UInt32 number ) const
{GUCEF_TRACE;

    if ( number < 1 || number > m_kinds.size() )
        return CORE::CString::Empty;
    const UInt32 index = number - 1;
    if ( KIND_NULL == m_kinds[ index ] )
        return "NULL";
    const char* bytes = &m_bytes[ m_offsets[ index ] ];
    if ( KIND_TEXT == m_kinds[ index ] )
        return CORE::CString( bytes );
    static const char digits[] = "0123456789abcdef";
    CORE::CString text;
    for ( int i=0; i<m_lengths[ index ]; ++i )
    {
        const UInt8 b = (UInt8) bytes[ i ];
        char pair[ 3 ] = { digits[ b >> 4 ], digits[ b & 0x0F ], '\0' };
        text += pair;
    }
    return text;
}

/*-------------------------------------------------------------------------*/

void
CPostgreSqlParameters::Clear( void )
{GUCEF_TRACE;

    m_bytes.clear();
    m_offsets.clear();
    m_lengths.clear();
    m_formats.clear();
    m_kinds.clear();
    m_values.clear();
    m_prepared = false;
}

/*-------------------------------------------------------------------------*/

void
CPostgreSqlParameters::Prepare( void ) const
{GUCEF_TRACE;

    if ( m_prepared )
        return;
    m_values.resize( m_kinds.size() );
    for ( UInt32 i=0; i<(UInt32) m_kinds.size(); ++i )
        m_values[ i ] = KIND_NULL == m_kinds[ i ] ? GUCEF_NULL : &m_bytes[ m_offsets[ i ] ];
    m_prepared = true;
}

/*-------------------------------------------------------------------------*/

const char* const*
CPostgreSqlParameters::GetValues( void ) const
{GUCEF_TRACE;

    Prepare();
    return m_values.empty() ? GUCEF_NULL : &m_values[ 0 ];
}

/*-------------------------------------------------------------------------*/

const int*
CPostgreSqlParameters::GetLengths( void ) const
{GUCEF_TRACE;

    return m_lengths.empty() ? GUCEF_NULL : &m_lengths[ 0 ];
}

/*-------------------------------------------------------------------------*/

const int*
CPostgreSqlParameters::GetFormats( void ) const
{GUCEF_TRACE;

    return m_formats.empty() ? GUCEF_NULL : &m_formats[ 0 ];
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace POSTGRESQL */
}; /* namespace DBPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
