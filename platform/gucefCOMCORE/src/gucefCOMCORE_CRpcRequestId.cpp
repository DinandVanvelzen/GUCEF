/*
 *  gucefCOMCORE: GUCEF module providing basic communication facilities
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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#include "gucefCOMCORE_CRpcRequestId.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COMCORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CRpcRequestId::CRpcRequestId( void )
    : CORE::CICloneable()
    , m_kind( KIND_NONE )
    , m_number( 0 )
    , m_text()
    , m_variant()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CRpcRequestId::CRpcRequestId( UInt64 id )
    : CORE::CICloneable()
    , m_kind( KIND_UINT64 )
    , m_number( id )
    , m_text()
    , m_variant( id )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CRpcRequestId::CRpcRequestId( const CORE::CString& id )
    : CORE::CICloneable()
    , m_kind( KIND_STRING )
    , m_number( 0 )
    , m_text( id )
    , m_variant( id )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CRpcRequestId::CRpcRequestId( const CRpcRequestId& src )
    : CORE::CICloneable( src )
    , m_kind( src.m_kind )
    , m_number( src.m_number )
    , m_text( src.m_text )
    , m_variant( src.m_variant )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CRpcRequestId::~CRpcRequestId()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CRpcRequestId&
CRpcRequestId::operator=( const CRpcRequestId& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        m_kind = src.m_kind;
        m_number = src.m_number;
        m_text = src.m_text;
        m_variant = src.m_variant;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

bool
CRpcRequestId::FromVariant( const CORE::CVariant& id )
{GUCEF_TRACE;

    if ( id.IsString() )
    {
        *this = CRpcRequestId( id.AsString() );
        return true;
    }
    if ( id.IsUnsignedInteger() )
    {
        *this = CRpcRequestId( id.AsUInt64() );
        return true;
    }
    if ( id.IsSignedInteger() && id.AsInt64() >= 0 )
    {
        *this = CRpcRequestId( (UInt64) id.AsInt64() );
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

CRpcRequestId::EKind
CRpcRequestId::GetKind( void ) const
{GUCEF_TRACE;

    return m_kind;
}

/*-------------------------------------------------------------------------*/

bool
CRpcRequestId::IsSet( void ) const
{GUCEF_TRACE;

    return KIND_NONE != m_kind;
}

/*-------------------------------------------------------------------------*/

UInt64
CRpcRequestId::GetUInt64( void ) const
{GUCEF_TRACE;

    return m_number;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CRpcRequestId::GetString( void ) const
{GUCEF_TRACE;

    return m_text;
}

/*-------------------------------------------------------------------------*/

const CORE::CVariant&
CRpcRequestId::GetVariant( void ) const
{GUCEF_TRACE;

    return m_variant;
}

/*-------------------------------------------------------------------------*/

void
CRpcRequestId::Clear( void )
{GUCEF_TRACE;

    m_kind = KIND_NONE;
    m_number = 0;
    m_text.Clear();
    m_variant.Clear();
}

/*-------------------------------------------------------------------------*/

CORE::CString
CRpcRequestId::ToString( void ) const
{GUCEF_TRACE;

    switch ( m_kind )
    {
        case KIND_UINT64: return "#" + CORE::ToString( m_number );
        case KIND_STRING: return m_text;
        default: return CORE::CString::Empty;
    }
}

/*-------------------------------------------------------------------------*/

bool
CRpcRequestId::operator==( const CRpcRequestId& other ) const
{GUCEF_TRACE;

    if ( m_kind != other.m_kind )
        return false;
    if ( KIND_UINT64 == m_kind )
        return m_number == other.m_number;
    if ( KIND_STRING == m_kind )
        return m_text == other.m_text;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CRpcRequestId::operator!=( const CRpcRequestId& other ) const
{GUCEF_TRACE;

    return !( *this == other );
}

/*-------------------------------------------------------------------------*/

bool
CRpcRequestId::operator<( const CRpcRequestId& other ) const
{GUCEF_TRACE;

    if ( m_kind != other.m_kind )
        return m_kind < other.m_kind;
    if ( KIND_UINT64 == m_kind )
        return m_number < other.m_number;
    if ( KIND_STRING == m_kind )
        return m_text < other.m_text;
    return false;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CRpcRequestId::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CRpcRequestId( *this );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
