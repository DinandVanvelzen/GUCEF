/*
 *  gucefCOM: GUCEF module providing communication implementations
 *  for standardized protocols
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

#ifndef GUCEF_COM_CMIMECODEC_H
#include "gucefCOM_CMimeCodec.h"
#define GUCEF_COM_CMIMECODEC_H
#endif /* GUCEF_COM_CMIMECODEC_H ? */

#include "gucefCOM_CImapValue.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COM {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CImapValue::CImapValue( void )
    : m_type( TYPE_NIL )
    , m_text()
    , m_bytes()
    , m_number( 0 )
    , m_elements()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CImapValue::CImapValue( const CImapValue& src )
    : m_type( TYPE_NIL )
    , m_text()
    , m_bytes()
    , m_number( 0 )
    , m_elements()
{GUCEF_TRACE;

    CopyFrom( src );
}

/*-------------------------------------------------------------------------*/

CImapValue::~CImapValue()
{GUCEF_TRACE;

    Clear();
}

/*-------------------------------------------------------------------------*/

CImapValue&
CImapValue::operator=( const CImapValue& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        Clear();
        CopyFrom( src );
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

void
CImapValue::CopyFrom( const CImapValue& src )
{GUCEF_TRACE;

    m_type = src.m_type;
    m_text = src.m_text;
    m_bytes = src.m_bytes;
    m_number = src.m_number;
    for ( size_t i=0; i<src.m_elements.size(); ++i )
        m_elements.push_back( GUCEF_NEW CImapValue( *src.m_elements[ i ] ) );
}

/*-------------------------------------------------------------------------*/

void
CImapValue::Clear( void )
{GUCEF_TRACE;

    // an iterative release would be needed for lists nested without a limit: the parser limits the nesting
    for ( size_t i=0; i<m_elements.size(); ++i )
        GUCEF_DELETE m_elements[ i ];
    m_elements.clear();
    m_type = TYPE_NIL;
    m_text.Clear();
    m_bytes.Clear();
    m_number = 0;
}

/*-------------------------------------------------------------------------*/

CImapValue::EType CImapValue::GetType( void ) const { return m_type; }
bool CImapValue::IsNil( void ) const { return TYPE_NIL == m_type; }
bool CImapValue::IsAtom( void ) const { return TYPE_ATOM == m_type; }
bool CImapValue::IsNumber( void ) const { return TYPE_NUMBER == m_type; }
bool CImapValue::IsString( void ) const { return TYPE_STRING == m_type; }
bool CImapValue::IsList( void ) const { return TYPE_LIST == m_type; }
bool CImapValue::IsText( void ) const { return TYPE_ATOM == m_type || TYPE_STRING == m_type; }

/*-------------------------------------------------------------------------*/

CORE::CString
CImapValue::GetString( void ) const
{GUCEF_TRACE;

    switch ( m_type )
    {
        case TYPE_ATOM:
        case TYPE_NUMBER:
            return m_text;
        case TYPE_STRING:
            return m_bytes.GetDataSize() > 0 ? CMimeCodec::SanitizeUtf8( static_cast< const char* >( m_bytes.GetConstBufferPtr() ), m_bytes.GetDataSize() ) : CORE::CString();
        default:
            return CORE::CString();
    }
}

/*-------------------------------------------------------------------------*/

const CORE::CDynamicBuffer&
CImapValue::GetBytes( void ) const
{GUCEF_TRACE;

    return m_bytes;
}

/*-------------------------------------------------------------------------*/

UInt64
CImapValue::GetNumber( void ) const
{GUCEF_TRACE;

    return m_number;
}

/*-------------------------------------------------------------------------*/

bool
CImapValue::AtomEquals( const char* text ) const
{GUCEF_TRACE;

    return TYPE_ATOM == m_type && m_text.Equals( text, false );
}

/*-------------------------------------------------------------------------*/

UInt32
CImapValue::GetListSize( void ) const
{GUCEF_TRACE;

    return (UInt32) m_elements.size();
}

/*-------------------------------------------------------------------------*/

const CImapValue&
CImapValue::GetElement( UInt32 index ) const
{GUCEF_TRACE;

    static const CImapValue nil;
    return index < m_elements.size() ? *m_elements[ index ] : nil;
}

/*-------------------------------------------------------------------------*/

void CImapValue::SetNil( void ) { Clear(); }

void
CImapValue::SetAtom( const CORE::CString& text )
{GUCEF_TRACE;

    Clear();
    m_type = TYPE_ATOM;
    m_text = text;
}

void
CImapValue::SetNumber( UInt64 number )
{GUCEF_TRACE;

    Clear();
    m_type = TYPE_NUMBER;
    m_number = number;
    m_text = CORE::UInt64ToString( number );
}

void
CImapValue::SetString( const void* data, UInt32 size )
{GUCEF_TRACE;

    Clear();
    m_type = TYPE_STRING;
    if ( size > 0 )
        m_bytes.CopyFrom( size, data );
}

void
CImapValue::SetList( void )
{GUCEF_TRACE;

    Clear();
    m_type = TYPE_LIST;
}

/*-------------------------------------------------------------------------*/

CImapValue*
CImapValue::AddElement( void )
{GUCEF_TRACE;

    CImapValue* element = GUCEF_NEW CImapValue();
    m_elements.push_back( element );
    return element;
}

/*-------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
