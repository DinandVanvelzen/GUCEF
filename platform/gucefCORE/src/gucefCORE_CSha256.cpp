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

#include "gucefCORE_CSha256.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

static const UInt32 g_sha256K[ 64 ] =
{
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static inline UInt32 RotR( UInt32 x, UInt32 n ) { return ( x >> n ) | ( x << ( 32 - n ) ); }

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CSha256::CSha256( void )
{GUCEF_TRACE;

    Reset();
}

/*-------------------------------------------------------------------------*/

void
CSha256::Reset( void )
{GUCEF_TRACE;

    m_state[ 0 ] = 0x6a09e667;
    m_state[ 1 ] = 0xbb67ae85;
    m_state[ 2 ] = 0x3c6ef372;
    m_state[ 3 ] = 0xa54ff53a;
    m_state[ 4 ] = 0x510e527f;
    m_state[ 5 ] = 0x9b05688c;
    m_state[ 6 ] = 0x1f83d9ab;
    m_state[ 7 ] = 0x5be0cd19;
    m_totalSize = 0;
    m_blockFill = 0;
}

/*-------------------------------------------------------------------------*/

void
CSha256::Transform( const UInt8* block )
{GUCEF_TRACE;

    UInt32 w[ 64 ];
    for ( UInt32 i=0; i<16; ++i )
    {
        w[ i ] = ( (UInt32) block[ i*4 ] << 24 ) | ( (UInt32) block[ i*4+1 ] << 16 ) |
                 ( (UInt32) block[ i*4+2 ] << 8 ) | (UInt32) block[ i*4+3 ];
    }
    for ( UInt32 i=16; i<64; ++i )
    {
        UInt32 s0 = RotR( w[ i-15 ], 7 ) ^ RotR( w[ i-15 ], 18 ) ^ ( w[ i-15 ] >> 3 );
        UInt32 s1 = RotR( w[ i-2 ], 17 ) ^ RotR( w[ i-2 ], 19 ) ^ ( w[ i-2 ] >> 10 );
        w[ i ] = w[ i-16 ] + s0 + w[ i-7 ] + s1;
    }

    UInt32 a = m_state[ 0 ], b = m_state[ 1 ], c = m_state[ 2 ], d = m_state[ 3 ];
    UInt32 e = m_state[ 4 ], f = m_state[ 5 ], g = m_state[ 6 ], h = m_state[ 7 ];
    for ( UInt32 i=0; i<64; ++i )
    {
        UInt32 S1 = RotR( e, 6 ) ^ RotR( e, 11 ) ^ RotR( e, 25 );
        UInt32 ch = ( e & f ) ^ ( ~e & g );
        UInt32 t1 = h + S1 + ch + g_sha256K[ i ] + w[ i ];
        UInt32 S0 = RotR( a, 2 ) ^ RotR( a, 13 ) ^ RotR( a, 22 );
        UInt32 maj = ( a & b ) ^ ( a & c ) ^ ( b & c );
        UInt32 t2 = S0 + maj;
        h = g; g = f; f = e; e = d + t1;
        d = c; c = b; b = a; a = t1 + t2;
    }
    m_state[ 0 ] += a; m_state[ 1 ] += b; m_state[ 2 ] += c; m_state[ 3 ] += d;
    m_state[ 4 ] += e; m_state[ 5 ] += f; m_state[ 6 ] += g; m_state[ 7 ] += h;
}

/*-------------------------------------------------------------------------*/

void
CSha256::Update( const void* data, UInt32 size )
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
CSha256::Final( UInt8* digest )
{GUCEF_TRACE;

    UInt64 bitSize = m_totalSize * 8;

    // the padding: a 1 bit, zeros up to 56 bytes into a block, then the length in bits
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

    for ( UInt32 i=0; i<8; ++i )
    {
        digest[ i*4 ] = (UInt8) ( m_state[ i ] >> 24 );
        digest[ i*4+1 ] = (UInt8) ( m_state[ i ] >> 16 );
        digest[ i*4+2 ] = (UInt8) ( m_state[ i ] >> 8 );
        digest[ i*4+3 ] = (UInt8) m_state[ i ];
    }
}

/*-------------------------------------------------------------------------*/

void
CSha256::Hash( const void* data, UInt32 size, UInt8* digest )
{GUCEF_TRACE;

    CSha256 sha;
    sha.Update( data, size );
    sha.Final( digest );
}

/*-------------------------------------------------------------------------*/

bool
CSha256::ConstantTimeEquals( const void* a, const void* b, UInt32 size )
{GUCEF_TRACE;

    const UInt8* pa = static_cast< const UInt8* >( a );
    const UInt8* pb = static_cast< const UInt8* >( b );
    UInt8 diff = 0;
    for ( UInt32 i=0; i<size; ++i )
        diff |= (UInt8) ( pa[ i ] ^ pb[ i ] );
    return 0 == diff;
}

/*-------------------------------------------------------------------------*/

CHmacSha256::CHmacSha256( void )
    : m_inner()
    , m_innerStart()
    , m_outerStart()
{GUCEF_TRACE;

    SetKey( GUCEF_NULL, 0 );
}

/*-------------------------------------------------------------------------*/

CHmacSha256::CHmacSha256( const void* key, UInt32 keySize )
    : m_inner()
    , m_innerStart()
    , m_outerStart()
{GUCEF_TRACE;

    SetKey( key, keySize );
}

/*-------------------------------------------------------------------------*/

void
CHmacSha256::SetKey( const void* key, UInt32 keySize )
{GUCEF_TRACE;

    UInt8 keyBlock[ CSha256::BlockSize ];
    memset( keyBlock, 0, sizeof( keyBlock ) );
    if ( keySize > CSha256::BlockSize )
        CSha256::Hash( key, keySize, keyBlock );
    else
    if ( keySize > 0 && GUCEF_NULL != key )
        memcpy( keyBlock, key, keySize );

    UInt8 innerKeyPad[ CSha256::BlockSize ];
    UInt8 outerKeyPad[ CSha256::BlockSize ];
    for ( UInt32 i=0; i<CSha256::BlockSize; ++i )
    {
        innerKeyPad[ i ] = (UInt8) ( keyBlock[ i ] ^ 0x36 );
        outerKeyPad[ i ] = (UInt8) ( keyBlock[ i ] ^ 0x5c );
    }

    m_innerStart.Reset();
    m_innerStart.Update( innerKeyPad, CSha256::BlockSize );
    m_outerStart.Reset();
    m_outerStart.Update( outerKeyPad, CSha256::BlockSize );
    m_inner = m_innerStart;

    // the key does not stay on the stack
    memset( keyBlock, 0, sizeof( keyBlock ) );
    memset( innerKeyPad, 0, sizeof( innerKeyPad ) );
    memset( outerKeyPad, 0, sizeof( outerKeyPad ) );
}

/*-------------------------------------------------------------------------*/

void
CHmacSha256::Reset( void )
{GUCEF_TRACE;

    m_inner = m_innerStart;
}

/*-------------------------------------------------------------------------*/

void
CHmacSha256::Update( const void* data, UInt32 size )
{GUCEF_TRACE;

    m_inner.Update( data, size );
}

/*-------------------------------------------------------------------------*/

void
CHmacSha256::Final( UInt8* mac )
{GUCEF_TRACE;

    UInt8 innerDigest[ CSha256::DigestSize ];
    m_inner.Final( innerDigest );
    CSha256 outer( m_outerStart );
    outer.Update( innerDigest, CSha256::DigestSize );
    outer.Final( mac );
    m_inner = m_innerStart;
}

/*-------------------------------------------------------------------------*/

void
CHmacSha256::Mac( const void* key, UInt32 keySize, const void* data, UInt32 dataSize, UInt8* mac )
{GUCEF_TRACE;

    CHmacSha256 hmac( key, keySize );
    hmac.Update( data, dataSize );
    hmac.Final( mac );
}

/*-------------------------------------------------------------------------*/

void
CHkdfSha256::Extract( const void* salt, UInt32 saltSize, const void* ikm, UInt32 ikmSize, UInt8* prk )
{GUCEF_TRACE;

    // an absent salt is a string of HashLen zeros, which as an HMAC key is the same as an empty key
    CHmacSha256::Mac( salt, saltSize, ikm, ikmSize, prk );
}

/*-------------------------------------------------------------------------*/

bool
CHkdfSha256::Expand( const UInt8* prk, const void* info, UInt32 infoSize, UInt8* okm, UInt32 okmSize )
{GUCEF_TRACE;

    if ( okmSize > MaxOutputSize )
        return false;

    CHmacSha256 hmac( prk, CSha256::DigestSize );
    UInt8 block[ CSha256::DigestSize ];
    UInt32 written = 0;
    UInt8 counter = 1;
    while ( written < okmSize )
    {
        if ( counter > 1 )
            hmac.Update( block, CSha256::DigestSize );
        if ( infoSize > 0 && GUCEF_NULL != info )
            hmac.Update( info, infoSize );
        hmac.Update( &counter, 1 );
        hmac.Final( block );

        UInt32 n = okmSize - written;
        if ( n > CSha256::DigestSize )
            n = CSha256::DigestSize;
        memcpy( okm + written, block, n );
        written += n;
        ++counter;
    }
    memset( block, 0, sizeof( block ) );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CHkdfSha256::Derive( const void* salt, UInt32 saltSize, const void* ikm, UInt32 ikmSize, const void* info, UInt32 infoSize, UInt8* okm, UInt32 okmSize )
{GUCEF_TRACE;

    UInt8 prk[ CSha256::DigestSize ];
    Extract( salt, saltSize, ikm, ikmSize, prk );
    bool success = Expand( prk, info, infoSize, okm, okmSize );
    memset( prk, 0, sizeof( prk ) );
    return success;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
