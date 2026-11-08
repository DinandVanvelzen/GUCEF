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

#include <string.h>

#ifndef GUCEF_CORE_CSHA256_H
#include "gucefCORE_CSha256.h"
#define GUCEF_CORE_CSHA256_H
#endif /* GUCEF_CORE_CSHA256_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "gucefCOMCORE_CRpcContractHash.h"

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

CRpcContractHash::CRpcContractHash( void )
{GUCEF_TRACE;

    memset( m_bytes, 0, Size );
}

/*-------------------------------------------------------------------------*/

CRpcContractHash::CRpcContractHash( const UInt8* bytes )
{GUCEF_TRACE;

    if ( GUCEF_NULL != bytes )
        memcpy( m_bytes, bytes, Size );
    else
        memset( m_bytes, 0, Size );
}

/*-------------------------------------------------------------------------*/

CRpcContractHash::CRpcContractHash( const CRpcContractHash& src )
{GUCEF_TRACE;

    memcpy( m_bytes, src.m_bytes, Size );
}

/*-------------------------------------------------------------------------*/

CRpcContractHash&
CRpcContractHash::operator=( const CRpcContractHash& src )
{GUCEF_TRACE;

    if ( &src != this )
        memcpy( m_bytes, src.m_bytes, Size );
    return *this;
}

/*-------------------------------------------------------------------------*/

CRpcContractHash
CRpcContractHash::Compute( const void* data, UInt32 size )
{GUCEF_TRACE;

    CRpcContractHash hash;
    CORE::CSha256::Hash( data, size, hash.m_bytes );
    return hash;
}

/*-------------------------------------------------------------------------*/

bool
CRpcContractHash::IsNull( void ) const
{GUCEF_TRACE;

    for ( UInt32 i=0; i<Size; ++i )
        if ( 0 != m_bytes[ i ] )
            return false;
    return true;
}

/*-------------------------------------------------------------------------*/

const UInt8*
CRpcContractHash::GetBytes( void ) const
{GUCEF_TRACE;

    return m_bytes;
}

/*-------------------------------------------------------------------------*/

void
CRpcContractHash::SetBytes( const UInt8* bytes )
{GUCEF_TRACE;

    memcpy( m_bytes, bytes, Size );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CRpcContractHash::ToHexString( void ) const
{GUCEF_TRACE;

    static const char digits[] = "0123456789abcdef";
    char text[ Size * 2 + 1 ];
    for ( UInt32 i=0; i<Size; ++i )
    {
        text[ i*2 ] = digits[ m_bytes[ i ] >> 4 ];
        text[ i*2+1 ] = digits[ m_bytes[ i ] & 0x0F ];
    }
    text[ Size * 2 ] = '\0';
    return CORE::CString( text );
}

/*-------------------------------------------------------------------------*/

bool
CRpcContractHash::FromHexString( const CORE::CString& hex )
{GUCEF_TRACE;

    CORE::CAsciiString ascii = CORE::ToAsciiString( hex );
    if ( ascii.Length() != Size * 2 )
        return false;
    const char* text = ascii.C_String();
    UInt8 bytes[ Size ];
    for ( UInt32 i=0; i<Size*2; ++i )
    {
        char c = text[ i ];
        UInt8 nibble = 0;
        if ( c >= '0' && c <= '9' ) nibble = (UInt8) ( c - '0' );
        else if ( c >= 'a' && c <= 'f' ) nibble = (UInt8) ( c - 'a' + 10 );
        else if ( c >= 'A' && c <= 'F' ) nibble = (UInt8) ( c - 'A' + 10 );
        else return false;
        if ( 0 == ( i & 1 ) )
            bytes[ i/2 ] = (UInt8) ( nibble << 4 );
        else
            bytes[ i/2 ] |= nibble;
    }
    memcpy( m_bytes, bytes, Size );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CRpcContractHash::operator==( const CRpcContractHash& other ) const
{GUCEF_TRACE;

    return 0 == memcmp( m_bytes, other.m_bytes, Size );
}

/*-------------------------------------------------------------------------*/

bool
CRpcContractHash::operator!=( const CRpcContractHash& other ) const
{GUCEF_TRACE;

    return 0 != memcmp( m_bytes, other.m_bytes, Size );
}

/*-------------------------------------------------------------------------*/

bool
CRpcContractHash::operator<( const CRpcContractHash& other ) const
{GUCEF_TRACE;

    return memcmp( m_bytes, other.m_bytes, Size ) < 0;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
