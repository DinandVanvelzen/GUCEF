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

#ifndef GUCEF_COM_CMIMECODEC_H
#include "gucefCOM_CMimeCodec.h"
#define GUCEF_COM_CMIMECODEC_H
#endif /* GUCEF_COM_CMIMECODEC_H ? */

#include "gucefCOM_CEMailHeaders.h"

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

CEMailHeaders::CEMailHeaders( void )
    : m_fields()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CEMailHeaders::CEMailHeaders( const CEMailHeaders& src )
    : m_fields( src.m_fields )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CEMailHeaders::~CEMailHeaders()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CEMailHeaders&
CEMailHeaders::operator=( const CEMailHeaders& src )
{GUCEF_TRACE;

    if ( &src != this )
        m_fields = src.m_fields;
    return *this;
}

/*-------------------------------------------------------------------------*/

bool
CEMailHeaders::Add( const CORE::CString& name, const CORE::CString& wireValue )
{GUCEF_TRACE;

    if ( !CMimeCodec::IsValidHeaderName( name ) || CMimeCodec::HasLineBreakOrNul( wireValue ) )
        return false;
    TField field;
    field.name = name;
    field.value = wireValue;
    m_fields.push_back( field );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CEMailHeaders::AddText( const CORE::CString& name, const CORE::CString& text )
{GUCEF_TRACE;

    return Add( name, CMimeCodec::EncodeHeaderText( text ) );
}

/*-------------------------------------------------------------------------*/

bool
CEMailHeaders::Set( const CORE::CString& name, const CORE::CString& wireValue )
{GUCEF_TRACE;

    if ( !CMimeCodec::IsValidHeaderName( name ) || CMimeCodec::HasLineBreakOrNul( wireValue ) )
        return false;
    Remove( name );
    return Add( name, wireValue );
}

/*-------------------------------------------------------------------------*/

UInt32
CEMailHeaders::Remove( const CORE::CString& name )
{GUCEF_TRACE;

    UInt32 removed = 0;
    TFieldVector::iterator i = m_fields.begin();
    while ( i != m_fields.end() )
    {
        if ( (*i).name.Equals( name, false ) )
        {
            i = m_fields.erase( i );
            ++removed;
        }
        else
            ++i;
    }
    return removed;
}

/*-------------------------------------------------------------------------*/

bool
CEMailHeaders::Has( const CORE::CString& name ) const
{GUCEF_TRACE;

    for ( size_t i=0; i<m_fields.size(); ++i )
        if ( m_fields[ i ].name.Equals( name, false ) )
            return true;
    return false;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CEMailHeaders::Get( const CORE::CString& name ) const
{GUCEF_TRACE;

    for ( size_t i=0; i<m_fields.size(); ++i )
        if ( m_fields[ i ].name.Equals( name, false ) )
            return m_fields[ i ].value;
    return CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CEMailHeaders::GetDecoded( const CORE::CString& name ) const
{GUCEF_TRACE;

    return CMimeCodec::DecodeHeaderText( Get( name ) );
}

/*-------------------------------------------------------------------------*/

void
CEMailHeaders::GetAll( const CORE::CString& name, CORE::CString::StringVector& values ) const
{GUCEF_TRACE;

    for ( size_t i=0; i<m_fields.size(); ++i )
        if ( m_fields[ i ].name.Equals( name, false ) )
            values.push_back( m_fields[ i ].value );
}

/*-------------------------------------------------------------------------*/

UInt32
CEMailHeaders::GetCount( void ) const
{GUCEF_TRACE;

    return (UInt32) m_fields.size();
}

/*-------------------------------------------------------------------------*/

const CEMailHeaders::TField&
CEMailHeaders::GetField( UInt32 index ) const
{GUCEF_TRACE;

    static const TField emptyField;
    return index < m_fields.size() ? m_fields[ index ] : emptyField;
}

/*-------------------------------------------------------------------------*/

const CEMailHeaders::TFieldVector&
CEMailHeaders::GetFields( void ) const
{GUCEF_TRACE;

    return m_fields;
}

/*-------------------------------------------------------------------------*/

void
CEMailHeaders::Clear( void )
{GUCEF_TRACE;

    m_fields.clear();
}

/*-------------------------------------------------------------------------*/

bool
CEMailHeaders::AppendTo( CORE::CDynamicBuffer& out ) const
{GUCEF_TRACE;

    CORE::CDynamicBuffer block;
    for ( size_t i=0; i<m_fields.size(); ++i )
        if ( !CMimeCodec::AppendHeaderField( block, m_fields[ i ].name, m_fields[ i ].value ) )
            return false;
    out.Append( block, true );
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
