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

#include "gucefCORE_CSha512.h"

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

/** 64 bit constants as { high, low } so no 64 bit literals are needed */
static const UInt32 g_sha512K[ 80 ][ 2 ] =
{
    { 0x428a2f98, 0xd728ae22 }, { 0x71374491, 0x23ef65cd },
    { 0xb5c0fbcf, 0xec4d3b2f }, { 0xe9b5dba5, 0x8189dbbc },
    { 0x3956c25b, 0xf348b538 }, { 0x59f111f1, 0xb605d019 },
    { 0x923f82a4, 0xaf194f9b }, { 0xab1c5ed5, 0xda6d8118 },
    { 0xd807aa98, 0xa3030242 }, { 0x12835b01, 0x45706fbe },
    { 0x243185be, 0x4ee4b28c }, { 0x550c7dc3, 0xd5ffb4e2 },
    { 0x72be5d74, 0xf27b896f }, { 0x80deb1fe, 0x3b1696b1 },
    { 0x9bdc06a7, 0x25c71235 }, { 0xc19bf174, 0xcf692694 },
    { 0xe49b69c1, 0x9ef14ad2 }, { 0xefbe4786, 0x384f25e3 },
    { 0x0fc19dc6, 0x8b8cd5b5 }, { 0x240ca1cc, 0x77ac9c65 },
    { 0x2de92c6f, 0x592b0275 }, { 0x4a7484aa, 0x6ea6e483 },
    { 0x5cb0a9dc, 0xbd41fbd4 }, { 0x76f988da, 0x831153b5 },
    { 0x983e5152, 0xee66dfab }, { 0xa831c66d, 0x2db43210 },
    { 0xb00327c8, 0x98fb213f }, { 0xbf597fc7, 0xbeef0ee4 },
    { 0xc6e00bf3, 0x3da88fc2 }, { 0xd5a79147, 0x930aa725 },
    { 0x06ca6351, 0xe003826f }, { 0x14292967, 0x0a0e6e70 },
    { 0x27b70a85, 0x46d22ffc }, { 0x2e1b2138, 0x5c26c926 },
    { 0x4d2c6dfc, 0x5ac42aed }, { 0x53380d13, 0x9d95b3df },
    { 0x650a7354, 0x8baf63de }, { 0x766a0abb, 0x3c77b2a8 },
    { 0x81c2c92e, 0x47edaee6 }, { 0x92722c85, 0x1482353b },
    { 0xa2bfe8a1, 0x4cf10364 }, { 0xa81a664b, 0xbc423001 },
    { 0xc24b8b70, 0xd0f89791 }, { 0xc76c51a3, 0x0654be30 },
    { 0xd192e819, 0xd6ef5218 }, { 0xd6990624, 0x5565a910 },
    { 0xf40e3585, 0x5771202a }, { 0x106aa070, 0x32bbd1b8 },
    { 0x19a4c116, 0xb8d2d0c8 }, { 0x1e376c08, 0x5141ab53 },
    { 0x2748774c, 0xdf8eeb99 }, { 0x34b0bcb5, 0xe19b48a8 },
    { 0x391c0cb3, 0xc5c95a63 }, { 0x4ed8aa4a, 0xe3418acb },
    { 0x5b9cca4f, 0x7763e373 }, { 0x682e6ff3, 0xd6b2b8a3 },
    { 0x748f82ee, 0x5defb2fc }, { 0x78a5636f, 0x43172f60 },
    { 0x84c87814, 0xa1f0ab72 }, { 0x8cc70208, 0x1a6439ec },
    { 0x90befffa, 0x23631e28 }, { 0xa4506ceb, 0xde82bde9 },
    { 0xbef9a3f7, 0xb2c67915 }, { 0xc67178f2, 0xe372532b },
    { 0xca273ece, 0xea26619c }, { 0xd186b8c7, 0x21c0c207 },
    { 0xeada7dd6, 0xcde0eb1e }, { 0xf57d4f7f, 0xee6ed178 },
    { 0x06f067aa, 0x72176fba }, { 0x0a637dc5, 0xa2c898a6 },
    { 0x113f9804, 0xbef90dae }, { 0x1b710b35, 0x131c471b },
    { 0x28db77f5, 0x23047d84 }, { 0x32caab7b, 0x40c72493 },
    { 0x3c9ebe0a, 0x15c9bebc }, { 0x431d67c4, 0x9c100d4c },
    { 0x4cc5d4be, 0xcb3e42b6 }, { 0x597f299c, 0xfc657e2a },
    { 0x5fcb6fab, 0x3ad6faec }, { 0x6c44198c, 0x4a475817 }
};

static const UInt32 g_sha512Start[ 8 ][ 2 ] =
{
    { 0x6a09e667, 0xf3bcc908 }, { 0xbb67ae85, 0x84caa73b },
    { 0x3c6ef372, 0xfe94f82b }, { 0xa54ff53a, 0x5f1d36f1 },
    { 0x510e527f, 0xade682d1 }, { 0x9b05688c, 0x2b3e6c1f },
    { 0x1f83d9ab, 0xfb41bd6b }, { 0x5be0cd19, 0x137e2179 }
};

static const UInt32 g_sha384Start[ 8 ][ 2 ] =
{
    { 0xcbbb9d5d, 0xc1059ed8 }, { 0x629a292a, 0x367cd507 },
    { 0x9159015a, 0x3070dd17 }, { 0x152fecd8, 0xf70e5939 },
    { 0x67332667, 0xffc00b31 }, { 0x8eb44a87, 0x68581511 },
    { 0xdb0c2e0d, 0x64f98fa7 }, { 0x47b5481d, 0xbefa4fa4 }
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static inline UInt64 RotR64( UInt64 x, UInt32 n ) { return ( x >> n ) | ( x << ( 64 - n ) ); }

static inline UInt64 MakeUInt64( const UInt32* pair ) { return ( (UInt64) pair[ 0 ] << 32 ) | (UInt64) pair[ 1 ]; }

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CSha512::CSha512( bool is384 )
    : m_totalSize( 0 )
    , m_blockFill( 0 )
    , m_is384( is384 )
{GUCEF_TRACE;

    Reset();
}

/*-------------------------------------------------------------------------*/

void
CSha512::Reset( void )
{GUCEF_TRACE;

    const UInt32 (*start)[ 2 ] = m_is384 ? g_sha384Start : g_sha512Start;
    for ( UInt32 i=0; i<8; ++i )
        m_state[ i ] = MakeUInt64( start[ i ] );
    m_totalSize = 0;
    m_blockFill = 0;
}

/*-------------------------------------------------------------------------*/

UInt32
CSha512::GetDigestSize( void ) const
{GUCEF_TRACE;

    return m_is384 ? Digest384Size : DigestSize;
}

/*-------------------------------------------------------------------------*/

void
CSha512::Transform( const UInt8* block )
{GUCEF_TRACE;

    UInt64 w[ 80 ];
    for ( UInt32 i=0; i<16; ++i )
    {
        UInt64 value = 0;
        for ( UInt32 b=0; b<8; ++b )
            value = ( value << 8 ) | (UInt64) block[ i*8 + b ];
        w[ i ] = value;
    }
    for ( UInt32 i=16; i<80; ++i )
    {
        UInt64 s0 = RotR64( w[ i-15 ], 1 ) ^ RotR64( w[ i-15 ], 8 ) ^ ( w[ i-15 ] >> 7 );
        UInt64 s1 = RotR64( w[ i-2 ], 19 ) ^ RotR64( w[ i-2 ], 61 ) ^ ( w[ i-2 ] >> 6 );
        w[ i ] = w[ i-16 ] + s0 + w[ i-7 ] + s1;
    }

    UInt64 a = m_state[ 0 ], b = m_state[ 1 ], c = m_state[ 2 ], d = m_state[ 3 ];
    UInt64 e = m_state[ 4 ], f = m_state[ 5 ], g = m_state[ 6 ], h = m_state[ 7 ];
    for ( UInt32 i=0; i<80; ++i )
    {
        UInt64 S1 = RotR64( e, 14 ) ^ RotR64( e, 18 ) ^ RotR64( e, 41 );
        UInt64 ch = ( e & f ) ^ ( ~e & g );
        UInt64 t1 = h + S1 + ch + MakeUInt64( g_sha512K[ i ] ) + w[ i ];
        UInt64 S0 = RotR64( a, 28 ) ^ RotR64( a, 34 ) ^ RotR64( a, 39 );
        UInt64 maj = ( a & b ) ^ ( a & c ) ^ ( b & c );
        UInt64 t2 = S0 + maj;
        h = g; g = f; f = e; e = d + t1;
        d = c; c = b; b = a; a = t1 + t2;
    }
    m_state[ 0 ] += a; m_state[ 1 ] += b; m_state[ 2 ] += c; m_state[ 3 ] += d;
    m_state[ 4 ] += e; m_state[ 5 ] += f; m_state[ 6 ] += g; m_state[ 7 ] += h;
}

/*-------------------------------------------------------------------------*/

void
CSha512::Update( const void* data, UInt32 size )
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
CSha512::Final( UInt8* digest )
{GUCEF_TRACE;

    // the length in bits is a 128 bit number, the upper 64 bits are the bits of the size that do not fit
    UInt64 bitSizeLow = m_totalSize << 3;
    UInt64 bitSizeHigh = m_totalSize >> 61;

    // the padding: a 1 bit, zeros up to 112 bytes into a block, then the length in bits
    UInt8 padding[ BlockSize + 16 ];
    memset( padding, 0, sizeof( padding ) );
    padding[ 0 ] = 0x80;
    UInt32 paddingSize = ( m_blockFill < 112 ) ? ( 112 - m_blockFill ) : ( BlockSize + 112 - m_blockFill );
    for ( Int32 i=7; i>=0; --i )
    {
        padding[ paddingSize + i ] = (UInt8) bitSizeHigh;
        padding[ paddingSize + 8 + i ] = (UInt8) bitSizeLow;
        bitSizeHigh >>= 8;
        bitSizeLow >>= 8;
    }
    Update( padding, paddingSize + 16 );

    const UInt32 wordCount = GetDigestSize() / 8;
    for ( UInt32 i=0; i<wordCount; ++i )
    {
        for ( UInt32 b=0; b<8; ++b )
            digest[ i*8 + b ] = (UInt8) ( m_state[ i ] >> ( 56 - 8*b ) );
    }
}

/*-------------------------------------------------------------------------*/

void
CSha512::Hash( const void* data, UInt32 size, UInt8* digest )
{GUCEF_TRACE;

    CSha512 sha( false );
    sha.Update( data, size );
    sha.Final( digest );
}

/*-------------------------------------------------------------------------*/

void
CSha512::Hash384( const void* data, UInt32 size, UInt8* digest )
{GUCEF_TRACE;

    CSha512 sha( true );
    sha.Update( data, size );
    sha.Final( digest );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
