/*
 *  gucefAUTH_TestApp: Application for testing gucefAUTH
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
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

#ifndef GUCEF_AUTH_H
#include "gucefAUTH.h"
#define GUCEF_AUTH_H
#endif /* GUCEF_AUTH_H ? */

#include "TestRsaSigner.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CTestRsaKey::TBig
CTestRsaKey::FromHex( const char* hex )
{
    TBig value;
    size_t length = strlen( hex );
    // little endian limbs, the last hex digits are the first limb
    for ( size_t end=length; end>0; )
    {
        size_t start = end >= 8 ? end - 8 : 0;
        UInt32 limb = 0;
        for ( size_t i=start; i<end; ++i )
        {
            char c = hex[ i ];
            UInt32 digit = c >= 'a' ? (UInt32) ( c - 'a' + 10 ) : ( c >= 'A' ? (UInt32) ( c - 'A' + 10 ) : (UInt32) ( c - '0' ) );
            limb = ( limb << 4 ) | digit;
        }
        value.push_back( limb );
        end = start;
    }
    Trim( value );
    return value;
}

/*-------------------------------------------------------------------------*/

void
CTestRsaKey::Trim( TBig& value )
{
    while ( value.size() > 1 && 0 == value.back() )
        value.pop_back();
    if ( value.empty() )
        value.push_back( 0 );
}

/*-------------------------------------------------------------------------*/

bool
CTestRsaKey::GreaterOrEqual( const TBig& a, const TBig& b )
{
    size_t size = a.size() > b.size() ? a.size() : b.size();
    for ( size_t i=size; i>0; --i )
    {
        UInt32 x = i-1 < a.size() ? a[ i-1 ] : 0;
        UInt32 y = i-1 < b.size() ? b[ i-1 ] : 0;
        if ( x != y )
            return x > y;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
CTestRsaKey::SubtractInPlace( TBig& a, const TBig& b )
{
    Int64 borrow = 0;
    for ( size_t i=0; i<a.size(); ++i )
    {
        Int64 value = (Int64) a[ i ] - borrow - ( i < b.size() ? (Int64) b[ i ] : 0 );
        if ( value < 0 )
        {
            value += 4294967296LL;
            borrow = 1;
        }
        else
            borrow = 0;
        a[ i ] = (UInt32) value;
    }
}

/*-------------------------------------------------------------------------*/

CTestRsaKey::TBig
CTestRsaKey::Reduce( const TBig& value, const TBig& modulus )
{
    // Bit by bit: slow and simple, used for a handful of reductions
    TBig remainder( modulus.size() + 1, 0 );
    for ( size_t limb=value.size(); limb>0; --limb )
    {
        for ( int bit=31; bit>=0; --bit )
        {
            UInt32 carry = ( value[ limb-1 ] >> bit ) & 1;
            for ( size_t i=0; i<remainder.size(); ++i )
            {
                UInt32 next = remainder[ i ] >> 31;
                remainder[ i ] = ( remainder[ i ] << 1 ) | carry;
                carry = next;
            }
            if ( GreaterOrEqual( remainder, modulus ) )
                SubtractInPlace( remainder, modulus );
        }
    }
    remainder.resize( modulus.size() );
    return remainder;
}

/*-------------------------------------------------------------------------*/

CTestRsaKey::TBig
CTestRsaKey::Multiply( const TBig& a, const TBig& b )
{
    TBig product( a.size() + b.size(), 0 );
    for ( size_t i=0; i<a.size(); ++i )
    {
        UInt64 carry = 0;
        for ( size_t j=0; j<b.size(); ++j )
        {
            UInt64 current = (UInt64) product[ i+j ] + (UInt64) a[ i ] * b[ j ] + carry;
            product[ i+j ] = (UInt32) current;
            carry = current >> 32;
        }
        product[ i + b.size() ] = (UInt32) carry;
    }
    return product;
}

/*-------------------------------------------------------------------------*/

/** Montgomery multiplication, a * b / R mod n, with R = 2^(32 * limbs of n); all of s limbs, the scratch has s + 2 */
static void
MontgomeryMultiply( const UInt32* a, const UInt32* b, const UInt32* n, UInt32 n0inv, size_t s, UInt32* t, UInt32* result )
{
    for ( size_t i=0; i<s+2; ++i )
        t[ i ] = 0;
    for ( size_t i=0; i<s; ++i )
    {
        UInt64 carry = 0;
        UInt64 bi = b[ i ];
        for ( size_t j=0; j<s; ++j )
        {
            UInt64 current = (UInt64) t[ j ] + (UInt64) a[ j ] * bi + carry;
            t[ j ] = (UInt32) current;
            carry = current >> 32;
        }
        UInt64 top = (UInt64) t[ s ] + carry;
        t[ s ] = (UInt32) top;
        t[ s+1 ] = (UInt32) ( top >> 32 );

        UInt32 m = t[ 0 ] * n0inv;
        UInt64 current = (UInt64) t[ 0 ] + (UInt64) m * n[ 0 ];
        carry = current >> 32;
        for ( size_t j=1; j<s; ++j )
        {
            current = (UInt64) t[ j ] + (UInt64) m * n[ j ] + carry;
            t[ j-1 ] = (UInt32) current;
            carry = current >> 32;
        }
        current = (UInt64) t[ s ] + carry;
        t[ s-1 ] = (UInt32) current;
        t[ s ] = t[ s+1 ] + (UInt32) ( current >> 32 );
    }

    bool subtract = 0 != t[ s ];
    if ( !subtract )
    {
        subtract = true;
        for ( size_t i=s; i>0; --i )
        {
            if ( t[ i-1 ] != n[ i-1 ] )
            {
                subtract = t[ i-1 ] > n[ i-1 ];
                break;
            }
        }
    }
    if ( subtract )
    {
        Int64 borrow = 0;
        for ( size_t i=0; i<s; ++i )
        {
            Int64 value = (Int64) t[ i ] - borrow - (Int64) n[ i ];
            if ( value < 0 )
            {
                value += 4294967296LL;
                borrow = 1;
            }
            else
                borrow = 0;
            result[ i ] = (UInt32) value;
        }
    }
    else
    {
        for ( size_t i=0; i<s; ++i )
            result[ i ] = t[ i ];
    }
}

/*-------------------------------------------------------------------------*/

CTestRsaKey::TBig
CTestRsaKey::PowerMod( const TBig& base, const TBig& exponent, const TBig& modulus )
{
    size_t s = modulus.size();

    // -1 / n[0] mod 2^32 by Newton iteration
    UInt32 inverse = modulus[ 0 ];
    for ( int i=0; i<5; ++i )
        inverse *= 2 - modulus[ 0 ] * inverse;
    UInt32 n0inv = (UInt32) ( 0 - inverse );

    // R^2 mod n: 1 doubled 64 * s times
    TBig r2( s + 1, 0 );
    r2[ 0 ] = 1;
    TBig modulusWide( modulus );
    modulusWide.push_back( 0 );
    for ( size_t i=0; i<64 * s; ++i )
    {
        UInt32 carry = 0;
        for ( size_t k=0; k<r2.size(); ++k )
        {
            UInt32 next = r2[ k ] >> 31;
            r2[ k ] = ( r2[ k ] << 1 ) | carry;
            carry = next;
        }
        if ( GreaterOrEqual( r2, modulusWide ) )
            SubtractInPlace( r2, modulusWide );
    }
    r2.resize( s );

    TBig one( s, 0 );
    one[ 0 ] = 1;
    TBig baseReduced = Reduce( base, modulus );
    TBig baseMont( s, 0 ), accumulator( s, 0 ), scratch( s, 0 ), t( s + 2, 0 );
    MontgomeryMultiply( &baseReduced[ 0 ], &r2[ 0 ], &modulus[ 0 ], n0inv, s, &t[ 0 ], &baseMont[ 0 ] );
    MontgomeryMultiply( &one[ 0 ], &r2[ 0 ], &modulus[ 0 ], n0inv, s, &t[ 0 ], &accumulator[ 0 ] );

    for ( size_t limb=exponent.size(); limb>0; --limb )
    {
        for ( int bit=31; bit>=0; --bit )
        {
            MontgomeryMultiply( &accumulator[ 0 ], &accumulator[ 0 ], &modulus[ 0 ], n0inv, s, &t[ 0 ], &scratch[ 0 ] );
            accumulator.swap( scratch );
            if ( 0 != ( ( exponent[ limb-1 ] >> bit ) & 1 ) )
            {
                MontgomeryMultiply( &accumulator[ 0 ], &baseMont[ 0 ], &modulus[ 0 ], n0inv, s, &t[ 0 ], &scratch[ 0 ] );
                accumulator.swap( scratch );
            }
        }
    }
    TBig result( s, 0 );
    MontgomeryMultiply( &accumulator[ 0 ], &one[ 0 ], &modulus[ 0 ], n0inv, s, &t[ 0 ], &result[ 0 ] );
    return result;
}

/*-------------------------------------------------------------------------*/

CTestRsaKey::TBig
CTestRsaKey::MultiplyMod( const TBig& a, const TBig& b, const TBig& modulus )
{
    return Reduce( Multiply( a, b ), modulus );
}

/*-------------------------------------------------------------------------*/

CTestRsaKey::CTestRsaKey( const char* nHex, const char* eHex, const char* pHex, const char* qHex, const char* dpHex, const char* dqHex, const char* qinvHex )
    : m_n( FromHex( nHex ) )
    , m_p( FromHex( pHex ) )
    , m_q( FromHex( qHex ) )
    , m_dp( FromHex( dpHex ) )
    , m_dq( FromHex( dqHex ) )
    , m_qinv( FromHex( qinvHex ) )
    , m_modulus()
    , m_exponent()
{
    TBig e = FromHex( eHex );
    // big endian bytes of the modulus and the exponent
    for ( size_t i=m_n.size(); i>0; --i )
        for ( int shift=24; shift>=0; shift-=8 )
        {
            UInt8 byte = (UInt8) ( m_n[ i-1 ] >> shift );
            if ( 0 != m_modulus.GetDataSize() || 0 != byte )
                m_modulus.Append( &byte, 1 );
        }
    for ( size_t i=e.size(); i>0; --i )
        for ( int shift=24; shift>=0; shift-=8 )
        {
            UInt8 byte = (UInt8) ( e[ i-1 ] >> shift );
            if ( 0 != m_exponent.GetDataSize() || 0 != byte )
                m_exponent.Append( &byte, 1 );
        }
}

/*-------------------------------------------------------------------------*/

bool
CTestRsaKey::SignSha256( const void* message, UInt32 size, CORE::CDynamicBuffer& signature ) const
{
    // EMSA-PKCS1-v1_5: 00 01 FF..FF 00 DigestInfo
    static const UInt8 prefix[ 19 ] = { 0x30, 0x31, 0x30, 0x0d, 0x06, 0x09, 0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x01, 0x05, 0x00, 0x04, 0x20 };
    UInt32 k = m_modulus.GetDataSize();
    UInt8 digest[ AUTH::CAuthCrypto::DigestSize ];
    AUTH::CAuthCrypto::Sha256( message, size, digest );

    CORE::CDynamicBuffer encoded;
    encoded.SetDataSize( k );
    UInt8* em = static_cast< UInt8* >( encoded.GetBufferPtr() );
    memset( em, 0xFF, k );
    em[ 0 ] = 0x00;
    em[ 1 ] = 0x01;
    UInt32 tLength = (UInt32) sizeof( prefix ) + AUTH::CAuthCrypto::DigestSize;
    em[ k - tLength - 1 ] = 0x00;
    memcpy( em + k - tLength, prefix, sizeof( prefix ) );
    memcpy( em + k - AUTH::CAuthCrypto::DigestSize, digest, AUTH::CAuthCrypto::DigestSize );

    // the message as a number
    TBig c;
    for ( UInt32 i=k; i>0; i-=4 )
    {
        UInt32 limb = ( (UInt32) em[ i-4 ] << 24 ) | ( (UInt32) em[ i-3 ] << 16 ) | ( (UInt32) em[ i-2 ] << 8 ) | (UInt32) em[ i-1 ];
        c.push_back( limb );
    }
    Trim( c );

    // the Chinese remainder theorem
    TBig m1 = PowerMod( Reduce( c, m_p ), m_dp, m_p );
    TBig m2 = PowerMod( Reduce( c, m_q ), m_dq, m_q );
    TBig m2p = Reduce( m2, m_p );
    TBig diff( m1 );
    if ( !GreaterOrEqual( m1, m2p ) )
    {
        // m1 + p - m2p
        TBig sum( m_p.size() + 1, 0 );
        UInt64 carry = 0;
        for ( size_t i=0; i<sum.size(); ++i )
        {
            UInt64 current = ( i < m1.size() ? (UInt64) m1[ i ] : 0 ) + ( i < m_p.size() ? (UInt64) m_p[ i ] : 0 ) + carry;
            sum[ i ] = (UInt32) current;
            carry = current >> 32;
        }
        diff = sum;
    }
    SubtractInPlace( diff, m2p );
    Trim( diff );
    TBig h = MultiplyMod( m_qinv, diff, m_p );
    TBig product = Multiply( h, m_q );
    TBig result( m_n.size() + 1, 0 );
    UInt64 carry = 0;
    for ( size_t i=0; i<result.size(); ++i )
    {
        UInt64 current = ( i < product.size() ? (UInt64) product[ i ] : 0 ) + ( i < m2.size() ? (UInt64) m2[ i ] : 0 ) + carry;
        result[ i ] = (UInt32) current;
        carry = current >> 32;
    }

    signature.Clear( true );
    signature.SetDataSize( k );
    UInt8* out = static_cast< UInt8* >( signature.GetBufferPtr() );
    for ( UInt32 i=0; i<k; ++i )
    {
        UInt32 limbIndex = i / 4;
        UInt32 shift = ( i % 4 ) * 8;
        out[ k - 1 - i ] = limbIndex < result.size() ? (UInt8) ( result[ limbIndex ] >> shift ) : 0;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CTestRsaKey::ToJwk( const CORE::CString& keyId, const CORE::CString& algorithm ) const
{
    CORE::CString n, e;
    CORE::Base64UrlEncode( m_modulus.GetConstBufferPtr(), m_modulus.GetDataSize(), n );
    CORE::Base64UrlEncode( m_exponent.GetConstBufferPtr(), m_exponent.GetDataSize(), e );
    CORE::CString jwk = "{\"kty\":\"RSA\",\"use\":\"sig\"";
    if ( !algorithm.IsNULLOrEmpty() )
        jwk += ",\"alg\":\"" + algorithm + "\"";
    jwk += ",\"kid\":\"" + keyId + "\",\"n\":\"" + n + "\",\"e\":\"" + e + "\"}";
    return jwk;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CTestRsaKey::MakeToken( const CORE::CString& headerJson, const CORE::CString& payloadJson ) const
{
    CORE::CString header, payload, signatureText;
    CORE::Base64UrlEncode( headerJson.C_String(), AUTH::GetByteLength( headerJson ), header );
    CORE::Base64UrlEncode( payloadJson.C_String(), AUTH::GetByteLength( payloadJson ), payload );
    CORE::CString signingInput = header + "." + payload;
    CORE::CDynamicBuffer signature;
    SignSha256( signingInput.C_String(), AUTH::GetByteLength( signingInput ), signature );
    CORE::Base64UrlEncode( signature.GetConstBufferPtr(), signature.GetDataSize(), signatureText );
    return signingInput + "." + signatureText;
}

/*-------------------------------------------------------------------------*/
