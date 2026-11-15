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

#include "gucefCORE_CAes.h"

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

/** The S-box of FIPS-197 5.1.1 */
const UInt8 g_sbox[ 256 ] =
{
    0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76,
    0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0,
    0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15,
    0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75,
    0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84,
    0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf,
    0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8,
    0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2,
    0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73,
    0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb,
    0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79,
    0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08,
    0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a,
    0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e,
    0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf,
    0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16
};

/** The inverse S-box, made once from the S-box */
struct SInverseSbox
{
    UInt8 table[ 256 ];
    SInverseSbox( void ) { for ( int i=0; i<256; ++i ) table[ g_sbox[ i ] ] = (UInt8) i; }
};

const UInt8*
InverseSbox( void )
{
    static const SInverseSbox inverse;
    return inverse.table;
}

inline UInt8
XTime( UInt8 x )
{
    return (UInt8) ( ( x << 1 ) ^ ( ( x & 0x80 ) ? 0x1b : 0x00 ) );
}

/** Multiplication in GF(2^8) */
inline UInt8
Mul( UInt8 a, UInt8 b )
{
    UInt8 result = 0;
    while ( 0 != b )
    {
        if ( b & 1 )
            result ^= a;
        a = XTime( a );
        b >>= 1;
    }
    return result;
}

inline void
AddRoundKey( UInt8* state, const UInt8* roundKey )
{
    for ( int i=0; i<16; ++i )
        state[ i ] ^= roundKey[ i ];
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CAes::CAes( void )
    : m_rounds( 0 )
    , m_keySet( false )
{GUCEF_TRACE;

    memset( m_roundKeys, 0, sizeof( m_roundKeys ) );
}

/*-------------------------------------------------------------------------*/

CAes::~CAes()
{GUCEF_TRACE;

    Clear();
}

/*-------------------------------------------------------------------------*/

void
CAes::Clear( void )
{GUCEF_TRACE;

    // a volatile pointer so that the wipe is not optimised away
    volatile UInt8* p = m_roundKeys;
    for ( UInt32 i=0; i<sizeof( m_roundKeys ); ++i )
        p[ i ] = 0;
    m_rounds = 0;
    m_keySet = false;
}

/*-------------------------------------------------------------------------*/

bool
CAes::SetKey( const UInt8* key, UInt32 keySize )
{GUCEF_TRACE;

    Clear();
    if ( GUCEF_NULL == key || ( 16 != keySize && 24 != keySize && 32 != keySize ) )
        return false;

    // the key expansion of FIPS-197 5.2
    const UInt32 nk = keySize / 4;
    m_rounds = nk + 6;
    const UInt32 words = 4 * ( m_rounds + 1 );
    memcpy( m_roundKeys, key, keySize );
    UInt8 rcon = 0x01;
    for ( UInt32 i=nk; i<words; ++i )
    {
        UInt8 temp[ 4 ];
        memcpy( temp, m_roundKeys + 4 * ( i - 1 ), 4 );
        if ( 0 == i % nk )
        {
            const UInt8 first = temp[ 0 ];
            temp[ 0 ] = (UInt8) ( g_sbox[ temp[ 1 ] ] ^ rcon );
            temp[ 1 ] = g_sbox[ temp[ 2 ] ];
            temp[ 2 ] = g_sbox[ temp[ 3 ] ];
            temp[ 3 ] = g_sbox[ first ];
            rcon = XTime( rcon );
        }
        else
        if ( nk > 6 && 4 == i % nk )
        {
            for ( int b=0; b<4; ++b )
                temp[ b ] = g_sbox[ temp[ b ] ];
        }
        for ( int b=0; b<4; ++b )
            m_roundKeys[ 4 * i + b ] = (UInt8) ( m_roundKeys[ 4 * ( i - nk ) + b ] ^ temp[ b ] );
    }
    m_keySet = true;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CAes::EncryptBlock( const UInt8* in, UInt8* out ) const
{GUCEF_TRACE;

    if ( !m_keySet )
        return;
    // the state in column order, as the input bytes come (FIPS-197 3.4)
    UInt8 s[ 16 ];
    memcpy( s, in, 16 );
    AddRoundKey( s, m_roundKeys );
    for ( UInt32 round=1; round<=m_rounds; ++round )
    {
        // SubBytes and ShiftRows: row r of column c moves to column c - r
        UInt8 t[ 16 ];
        for ( int c=0; c<4; ++c )
            for ( int r=0; r<4; ++r )
                t[ 4 * c + r ] = g_sbox[ s[ 4 * ( ( c + r ) & 3 ) + r ] ];
        if ( round != m_rounds )
        {
            // MixColumns
            for ( int c=0; c<4; ++c )
            {
                const UInt8 a0 = t[ 4 * c ], a1 = t[ 4 * c + 1 ], a2 = t[ 4 * c + 2 ], a3 = t[ 4 * c + 3 ];
                s[ 4 * c ] = (UInt8) ( XTime( a0 ) ^ ( XTime( a1 ) ^ a1 ) ^ a2 ^ a3 );
                s[ 4 * c + 1 ] = (UInt8) ( a0 ^ XTime( a1 ) ^ ( XTime( a2 ) ^ a2 ) ^ a3 );
                s[ 4 * c + 2 ] = (UInt8) ( a0 ^ a1 ^ XTime( a2 ) ^ ( XTime( a3 ) ^ a3 ) );
                s[ 4 * c + 3 ] = (UInt8) ( ( XTime( a0 ) ^ a0 ) ^ a1 ^ a2 ^ XTime( a3 ) );
            }
        }
        else
        {
            memcpy( s, t, 16 );
        }
        AddRoundKey( s, m_roundKeys + 16 * round );
    }
    memcpy( out, s, 16 );
}

/*-------------------------------------------------------------------------*/

void
CAes::DecryptBlock( const UInt8* in, UInt8* out ) const
{GUCEF_TRACE;

    if ( !m_keySet )
        return;
    // the inverse cipher of FIPS-197 5.3
    const UInt8* inverse = InverseSbox();
    UInt8 s[ 16 ];
    memcpy( s, in, 16 );
    AddRoundKey( s, m_roundKeys + 16 * m_rounds );
    for ( UInt32 round=m_rounds; round>0; --round )
    {
        // InvShiftRows and InvSubBytes: row r of column c moves to column c + r
        UInt8 t[ 16 ];
        for ( int c=0; c<4; ++c )
            for ( int r=0; r<4; ++r )
                t[ 4 * ( ( c + r ) & 3 ) + r ] = inverse[ s[ 4 * c + r ] ];
        AddRoundKey( t, m_roundKeys + 16 * ( round - 1 ) );
        if ( round > 1 )
        {
            // InvMixColumns
            for ( int c=0; c<4; ++c )
            {
                const UInt8 a0 = t[ 4 * c ], a1 = t[ 4 * c + 1 ], a2 = t[ 4 * c + 2 ], a3 = t[ 4 * c + 3 ];
                s[ 4 * c ] = (UInt8) ( Mul( a0, 0x0e ) ^ Mul( a1, 0x0b ) ^ Mul( a2, 0x0d ) ^ Mul( a3, 0x09 ) );
                s[ 4 * c + 1 ] = (UInt8) ( Mul( a0, 0x09 ) ^ Mul( a1, 0x0e ) ^ Mul( a2, 0x0b ) ^ Mul( a3, 0x0d ) );
                s[ 4 * c + 2 ] = (UInt8) ( Mul( a0, 0x0d ) ^ Mul( a1, 0x09 ) ^ Mul( a2, 0x0e ) ^ Mul( a3, 0x0b ) );
                s[ 4 * c + 3 ] = (UInt8) ( Mul( a0, 0x0b ) ^ Mul( a1, 0x0d ) ^ Mul( a2, 0x09 ) ^ Mul( a3, 0x0e ) );
            }
        }
        else
        {
            memcpy( s, t, 16 );
        }
    }
    memcpy( out, s, 16 );
}

/*-------------------------------------------------------------------------*/

bool
CAes::CbcDecrypt( const UInt8* key, UInt32 keySize, const UInt8* iv, const UInt8* input, UInt32 inputSize, bool removePadding, CDynamicBuffer& output )
{GUCEF_TRACE;

    output.Clear();
    if ( GUCEF_NULL == iv || ( GUCEF_NULL == input && 0 != inputSize ) || 0 != inputSize % BlockSize || ( removePadding && 0 == inputSize ) )
        return false;
    CAes aes;
    if ( !aes.SetKey( key, keySize ) )
        return false;
    output.SetDataSize( inputSize );
    UInt8* out = static_cast< UInt8* >( output.GetBufferPtr() );
    UInt8 previous[ 16 ];
    memcpy( previous, iv, 16 );
    for ( UInt32 offset=0; offset<inputSize; offset+=BlockSize )
    {
        UInt8 block[ 16 ];
        aes.DecryptBlock( input + offset, block );
        for ( int i=0; i<16; ++i )
            out[ offset + i ] = (UInt8) ( block[ i ] ^ previous[ i ] );
        memcpy( previous, input + offset, 16 );
    }
    if ( removePadding )
    {
        // PKCS#7: the last byte n (1 to 16) and n bytes of n
        const UInt8 n = out[ inputSize - 1 ];
        bool valid = n >= 1 && n <= BlockSize;
        for ( UInt32 i=0; valid && i<n; ++i )
            valid = out[ inputSize - 1 - i ] == n;
        if ( !valid )
        {
            memset( out, 0, inputSize );
            output.Clear();
            return false;
        }
        output.SetDataSize( inputSize - n );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAes::CbcEncrypt( const UInt8* key, UInt32 keySize, const UInt8* iv, const UInt8* input, UInt32 inputSize, bool addPadding, CDynamicBuffer& output )
{GUCEF_TRACE;

    output.Clear();
    if ( GUCEF_NULL == iv || ( GUCEF_NULL == input && 0 != inputSize ) || ( !addPadding && 0 != inputSize % BlockSize ) )
        return false;
    CAes aes;
    if ( !aes.SetKey( key, keySize ) )
        return false;
    const UInt32 padding = addPadding ? BlockSize - inputSize % BlockSize : 0;
    const UInt32 total = inputSize + padding;
    output.SetDataSize( total );
    UInt8* out = static_cast< UInt8* >( output.GetBufferPtr() );
    if ( inputSize > 0 )
        memcpy( out, input, inputSize );
    for ( UInt32 i=0; i<padding; ++i )
        out[ inputSize + i ] = (UInt8) padding;
    const UInt8* previous = iv;
    for ( UInt32 offset=0; offset<total; offset+=BlockSize )
    {
        for ( int i=0; i<16; ++i )
            out[ offset + i ] ^= previous[ i ];
        aes.EncryptBlock( out + offset, out + offset );
        previous = out + offset;
    }
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
