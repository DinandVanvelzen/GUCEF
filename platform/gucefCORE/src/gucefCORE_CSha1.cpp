/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#include "gucefCORE_CSha1.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

inline UInt32 RotL( UInt32 x, UInt32 n ) { return ( x << n ) | ( x >> ( 32 - n ) ); }

inline Int32
HexDigitValue( char c )
{
    if ( c >= '0' && c <= '9' )
        return c - '0';
    if ( c >= 'a' && c <= 'f' )
        return c - 'a' + 10;
    if ( c >= 'A' && c <= 'F' )
        return c - 'A' + 10;
    return -1;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CSha1::CSha1( void )
{GUCEF_TRACE;

    Reset();
}

/*-------------------------------------------------------------------------*/

void
CSha1::Reset( void )
{GUCEF_TRACE;

    m_state[ 0 ] = 0x67452301;
    m_state[ 1 ] = 0xEFCDAB89;
    m_state[ 2 ] = 0x98BADCFE;
    m_state[ 3 ] = 0x10325476;
    m_state[ 4 ] = 0xC3D2E1F0;
    m_totalSize = 0;
    m_blockFill = 0;
}

/*-------------------------------------------------------------------------*/

void
CSha1::Transform( const UInt8* block )
{GUCEF_TRACE;

    UInt32 w[ 80 ];
    for ( UInt32 i=0; i<16; ++i )
    {
        w[ i ] = ( (UInt32) block[ i*4 ] << 24 ) | ( (UInt32) block[ i*4+1 ] << 16 ) |
                 ( (UInt32) block[ i*4+2 ] << 8 ) | (UInt32) block[ i*4+3 ];
    }
    for ( UInt32 i=16; i<80; ++i )
        w[ i ] = RotL( w[ i-3 ] ^ w[ i-8 ] ^ w[ i-14 ] ^ w[ i-16 ], 1 );

    UInt32 a = m_state[ 0 ], b = m_state[ 1 ], c = m_state[ 2 ], d = m_state[ 3 ], e = m_state[ 4 ];
    for ( UInt32 i=0; i<80; ++i )
    {
        UInt32 f, k;
        if ( i < 20 )
        {
            f = ( b & c ) | ( ~b & d );
            k = 0x5A827999;
        }
        else
        if ( i < 40 )
        {
            f = b ^ c ^ d;
            k = 0x6ED9EBA1;
        }
        else
        if ( i < 60 )
        {
            f = ( b & c ) | ( b & d ) | ( c & d );
            k = 0x8F1BBCDC;
        }
        else
        {
            f = b ^ c ^ d;
            k = 0xCA62C1D6;
        }
        const UInt32 t = RotL( a, 5 ) + f + e + k + w[ i ];
        e = d;
        d = c;
        c = RotL( b, 30 );
        b = a;
        a = t;
    }
    m_state[ 0 ] += a; m_state[ 1 ] += b; m_state[ 2 ] += c; m_state[ 3 ] += d; m_state[ 4 ] += e;
}

/*-------------------------------------------------------------------------*/

void
CSha1::Update( const void* data, UInt32 size )
{GUCEF_TRACE;

    const UInt8* p = static_cast< const UInt8* >( data );
    m_totalSize += size;
    while ( size > 0 )
    {
        if ( 0 == m_blockFill && size >= BlockSize )
        {
            Transform( p );
            p += BlockSize;
            size -= BlockSize;
            continue;
        }
        UInt32 n = BlockSize - m_blockFill;
        if ( n > size )
            n = size;
        memcpy( m_block + m_blockFill, p, n );
        m_blockFill += n;
        p += n;
        size -= n;
        if ( BlockSize == m_blockFill )
        {
            Transform( m_block );
            m_blockFill = 0;
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CSha1::Final( UInt8* digest )
{GUCEF_TRACE;

    UInt64 bitSize = m_totalSize * 8;

    // the padding: a 1 bit, zeros up to 56 bytes into a block, then the length in bits, big endian
    UInt8 padding[ BlockSize + 8 ];
    memset( padding, 0, sizeof( padding ) );
    padding[ 0 ] = 0x80;
    UInt32 paddingSize = ( m_blockFill < 56 ) ? ( 56 - m_blockFill ) : ( BlockSize + 56 - m_blockFill );
    for ( Int32 i=7; i>=0; --i )
    {
        padding[ paddingSize + i ] = (UInt8) bitSize;
        bitSize >>= 8;
    }
    Update( padding, paddingSize + 8 );

    for ( UInt32 i=0; i<5; ++i )
    {
        digest[ i*4 ] = (UInt8) ( m_state[ i ] >> 24 );
        digest[ i*4+1 ] = (UInt8) ( m_state[ i ] >> 16 );
        digest[ i*4+2 ] = (UInt8) ( m_state[ i ] >> 8 );
        digest[ i*4+3 ] = (UInt8) m_state[ i ];
    }
}

/*-------------------------------------------------------------------------*/

CString
CSha1::FinalHex( void )
{GUCEF_TRACE;

    UInt8 digest[ DigestSize ];
    Final( digest );
    return DigestToHex( digest );
}

/*-------------------------------------------------------------------------*/

void
CSha1::Hash( const void* data, UInt32 size, UInt8* digest )
{GUCEF_TRACE;

    CSha1 sha;
    sha.Update( data, size );
    sha.Final( digest );
}

/*-------------------------------------------------------------------------*/

CString
CSha1::HashHex( const void* data, UInt32 size )
{GUCEF_TRACE;

    UInt8 digest[ DigestSize ];
    Hash( data, size, digest );
    return DigestToHex( digest );
}

/*-------------------------------------------------------------------------*/

CString
CSha1::DigestToHex( const UInt8* digest )
{GUCEF_TRACE;

    static const char digits[] = "0123456789abcdef";
    char text[ HexDigestLength + 1 ];
    for ( UInt32 i=0; i<DigestSize; ++i )
    {
        text[ i*2 ] = digits[ digest[ i ] >> 4 ];
        text[ i*2+1 ] = digits[ digest[ i ] & 0x0F ];
    }
    text[ HexDigestLength ] = '\0';
    return CString( text );
}

/*-------------------------------------------------------------------------*/

bool
CSha1::DigestFromHex( const CString& hex, UInt8* digest )
{GUCEF_TRACE;

    const CAsciiString ascii = ToAsciiString( hex );
    if ( HexDigestLength != ascii.Length() )
        return false;
    const char* text = ascii.C_String();
    UInt8 bytes[ DigestSize ];
    for ( UInt32 i=0; i<DigestSize; ++i )
    {
        const Int32 high = HexDigitValue( text[ i*2 ] );
        const Int32 low = HexDigitValue( text[ i*2+1 ] );
        if ( high < 0 || low < 0 )
            return false;
        bytes[ i ] = (UInt8) ( ( high << 4 ) | low );
    }
    memcpy( digest, bytes, DigestSize );
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
