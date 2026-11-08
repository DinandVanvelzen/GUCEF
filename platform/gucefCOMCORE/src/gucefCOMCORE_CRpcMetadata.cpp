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

#include "gucefCOMCORE_CRpcMetadata.h"

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

const char* CRpcMetadata::IdempotencyKey = "idempotency-key";
const char* CRpcMetadata::TraceId = "trace-id";
const char* CRpcMetadata::SpanId = "span-id";
const char* CRpcMetadata::Locale = "locale";
const char* CRpcMetadata::ClientVersion = "client-version";
const char* CRpcMetadata::ClientTimeZone = "client-time-zone";

/*-------------------------------------------------------------------------*/

UInt32
CRpcMetadata::GetWellKnownKeyRef( const CORE::CString& key )
{GUCEF_TRACE;

    for ( UInt32 i=2; i<=7; ++i )
        if ( key == GetWellKnownKey( i ) )
            return i;
    return 0;
}

/*-------------------------------------------------------------------------*/

const char*
CRpcMetadata::GetWellKnownKey( UInt32 keyRef )
{GUCEF_TRACE;

    switch ( keyRef )
    {
        case 2: return IdempotencyKey;
        case 3: return TraceId;
        case 4: return SpanId;
        case 5: return Locale;
        case 6: return ClientVersion;
        case 7: return ClientTimeZone;
        default: return "";
    }
}

/*-------------------------------------------------------------------------*/

CRpcMetadata::CRpcMetadata( void )
    : m_keys()
    , m_values()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CRpcMetadata::CRpcMetadata( const CRpcMetadata& src )
    : m_keys( src.m_keys )
    , m_values( src.m_values )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CRpcMetadata::~CRpcMetadata()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CRpcMetadata&
CRpcMetadata::operator=( const CRpcMetadata& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        m_keys = src.m_keys;
        m_values = src.m_values;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

bool
CRpcMetadata::Set( const CORE::CString& key, const CORE::CVariant& value )
{GUCEF_TRACE;

    if ( !value.IsString() && !value.IsBlob() && !value.IsInteger() && !value.IsBoolean() )
        return false;
    for ( UInt32 i=0; i<m_keys.size(); ++i )
    {
        if ( m_keys[ i ] == key )
        {
            m_values[ i ] = value;
            return true;
        }
    }
    m_keys.push_back( key );
    m_values.push_back( value );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CRpcMetadata::Get( const CORE::CString& key, CORE::CVariant& value ) const
{GUCEF_TRACE;

    for ( UInt32 i=0; i<m_keys.size(); ++i )
    {
        if ( m_keys[ i ] == key )
        {
            value = m_values[ i ];
            return true;
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CRpcMetadata::GetString( const CORE::CString& key ) const
{GUCEF_TRACE;

    CORE::CVariant value;
    if ( Get( key, value ) )
        return value.AsString();
    return CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

bool
CRpcMetadata::Has( const CORE::CString& key ) const
{GUCEF_TRACE;

    for ( UInt32 i=0; i<m_keys.size(); ++i )
        if ( m_keys[ i ] == key )
            return true;
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CRpcMetadata::Remove( const CORE::CString& key )
{GUCEF_TRACE;

    for ( UInt32 i=0; i<m_keys.size(); ++i )
    {
        if ( m_keys[ i ] == key )
        {
            m_keys.erase( m_keys.begin() + i );
            m_values.erase( m_values.begin() + i );
            return true;
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

void
CRpcMetadata::Clear( void )
{GUCEF_TRACE;

    m_keys.clear();
    m_values.clear();
}

/*-------------------------------------------------------------------------*/

UInt32
CRpcMetadata::GetCount( void ) const
{GUCEF_TRACE;

    return (UInt32) m_keys.size();
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CRpcMetadata::GetKeyAt( UInt32 index ) const
{GUCEF_TRACE;

    return m_keys[ index ];
}

/*-------------------------------------------------------------------------*/

const CORE::CVariant&
CRpcMetadata::GetValueAt( UInt32 index ) const
{GUCEF_TRACE;

    return m_values[ index ];
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
