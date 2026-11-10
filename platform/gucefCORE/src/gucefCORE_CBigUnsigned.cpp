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

#include "gucefCORE_CBigUnsigned.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CBigUnsigned::CBigUnsigned( void )
    : m_limbs()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CBigUnsigned::SetZero( void )
{GUCEF_TRACE;

    m_limbs.resize( 0 );
}

/*-------------------------------------------------------------------------*/

bool
CBigUnsigned::SetFromBigEndian( const void* bytes, UInt32 size )
{GUCEF_TRACE;

    m_limbs.resize( 0 );
    if ( 0 == size )
        return true;
    if ( GUCEF_NULL == bytes )
        return false;

    const UInt8* p = static_cast< const UInt8* >( bytes );
    const UInt32 limbCount = ( size + 3 ) / 4;
    m_limbs.resize( limbCount, 0 );
    for ( UInt32 i=0; i<size; ++i )
    {
        // byte 'i' counted from the end is byte 'i' of the number
        UInt32 byteFromEnd = size - 1 - i;
        m_limbs[ byteFromEnd / 4 ] |= (UInt32) p[ i ] << ( 8 * ( byteFromEnd % 4 ) );
    }
    while ( !m_limbs.empty() && 0 == m_limbs.back() )
        m_limbs.pop_back();
    return true;
}

/*-------------------------------------------------------------------------*/

void
CBigUnsigned::SetFromLimbs( const UInt32* limbs, UInt32 limbCount )
{GUCEF_TRACE;

    while ( limbCount > 0 && 0 == limbs[ limbCount-1 ] )
        --limbCount;
    m_limbs.resize( limbCount );
    if ( limbCount > 0 )
        memcpy( &m_limbs[ 0 ], limbs, limbCount * sizeof( UInt32 ) );
}

/*-------------------------------------------------------------------------*/

bool
CBigUnsigned::ToBigEndian( void* bytes, UInt32 size ) const
{GUCEF_TRACE;

    if ( GetByteLength() > size || ( GUCEF_NULL == bytes && size > 0 ) )
        return false;

    UInt8* p = static_cast< UInt8* >( bytes );
    for ( UInt32 i=0; i<size; ++i )
    {
        UInt32 byteFromEnd = size - 1 - i;
        UInt32 limbIndex = byteFromEnd / 4;
        p[ i ] = limbIndex < (UInt32) m_limbs.size() ? (UInt8) ( m_limbs[ limbIndex ] >> ( 8 * ( byteFromEnd % 4 ) ) ) : 0;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

UInt32
CBigUnsigned::GetBitLength( void ) const
{GUCEF_TRACE;

    if ( m_limbs.empty() )
        return 0;

    UInt32 top = m_limbs.back();
    UInt32 bits = 0;
    while ( 0 != top )
    {
        ++bits;
        top >>= 1;
    }
    return (UInt32) ( ( m_limbs.size() - 1 ) * 32 ) + bits;
}

/*-------------------------------------------------------------------------*/

UInt32
CBigUnsigned::GetByteLength( void ) const
{GUCEF_TRACE;

    return ( GetBitLength() + 7 ) / 8;
}

/*-------------------------------------------------------------------------*/

bool
CBigUnsigned::IsZero( void ) const
{GUCEF_TRACE;

    return m_limbs.empty();
}

/*-------------------------------------------------------------------------*/

bool
CBigUnsigned::IsOdd( void ) const
{GUCEF_TRACE;

    return !m_limbs.empty() && 0 != ( m_limbs[ 0 ] & 1 );
}

/*-------------------------------------------------------------------------*/

bool
CBigUnsigned::GetBit( UInt32 bitIndex ) const
{GUCEF_TRACE;

    UInt32 limbIndex = bitIndex / 32;
    if ( limbIndex >= (UInt32) m_limbs.size() )
        return false;
    return 0 != ( ( m_limbs[ limbIndex ] >> ( bitIndex % 32 ) ) & 1 );
}

/*-------------------------------------------------------------------------*/

bool
CBigUnsigned::TryGetUInt64( UInt64& value ) const
{GUCEF_TRACE;

    if ( m_limbs.size() > 2 )
        return false;
    value = 0;
    if ( m_limbs.size() > 0 )
        value = m_limbs[ 0 ];
    if ( m_limbs.size() > 1 )
        value |= (UInt64) m_limbs[ 1 ] << 32;
    return true;
}

/*-------------------------------------------------------------------------*/

Int32
CBigUnsigned::Compare( const CBigUnsigned& other ) const
{GUCEF_TRACE;

    if ( m_limbs.size() != other.m_limbs.size() )
        return m_limbs.size() < other.m_limbs.size() ? -1 : 1;
    for ( size_t i=m_limbs.size(); i>0; --i )
    {
        if ( m_limbs[ i-1 ] != other.m_limbs[ i-1 ] )
            return m_limbs[ i-1 ] < other.m_limbs[ i-1 ] ? -1 : 1;
    }
    return 0;
}

/*-------------------------------------------------------------------------*/

UInt32
CBigUnsigned::GetLimbCount( void ) const
{GUCEF_TRACE;

    return (UInt32) m_limbs.size();
}

/*-------------------------------------------------------------------------*/

const UInt32*
CBigUnsigned::GetLimbs( void ) const
{GUCEF_TRACE;

    return m_limbs.empty() ? GUCEF_NULL : &m_limbs[ 0 ];
}

/*-------------------------------------------------------------------------*/

CMontgomeryModulus::CMontgomeryModulus( void )
    : m_modulus()
    , m_limbCount( 0 )
    , m_n0Inverse( 0 )
    , m_rr()
    , m_scratch()
    , m_baseMontgomery()
    , m_accumulator()
    , m_one()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CMontgomeryModulus::IsInitialized( void ) const
{GUCEF_TRACE;

    return m_limbCount > 0;
}

/*-------------------------------------------------------------------------*/

const CBigUnsigned&
CMontgomeryModulus::GetModulus( void ) const
{GUCEF_TRACE;

    return m_modulus;
}

/*-------------------------------------------------------------------------*/

/** Whether the k limb numbers a >= b */
static bool
IsGreaterOrEqual( const UInt32* a, const UInt32* b, UInt32 k )
{GUCEF_TRACE;

    for ( UInt32 i=k; i>0; --i )
    {
        if ( a[ i-1 ] != b[ i-1 ] )
            return a[ i-1 ] > b[ i-1 ];
    }
    return true;
}

/*-------------------------------------------------------------------------*/

/** a -= b over k limbs, returns the borrow */
static UInt32
SubtractInPlace( UInt32* a, const UInt32* b, UInt32 k )
{GUCEF_TRACE;

    UInt64 borrow = 0;
    for ( UInt32 i=0; i<k; ++i )
    {
        UInt64 difference = (UInt64) a[ i ] - (UInt64) b[ i ] - borrow;
        a[ i ] = (UInt32) difference;
        borrow = ( difference >> 32 ) & 1;
    }
    return (UInt32) borrow;
}

/*-------------------------------------------------------------------------*/

void
CMontgomeryModulus::DoubleMod( UInt32* value ) const
{GUCEF_TRACE;

    // value < n, so 2 * value < 2 * n and one subtraction is enough
    const UInt32* n = m_modulus.GetLimbs();
    UInt32 carry = 0;
    for ( UInt32 i=0; i<m_limbCount; ++i )
    {
        UInt32 nextCarry = value[ i ] >> 31;
        value[ i ] = ( value[ i ] << 1 ) | carry;
        carry = nextCarry;
    }
    if ( 0 != carry || IsGreaterOrEqual( value, n, m_limbCount ) )
        SubtractInPlace( value, n, m_limbCount );
}

/*-------------------------------------------------------------------------*/

void
CMontgomeryModulus::MontMul( const UInt32* a, const UInt32* b, UInt32* out )
{GUCEF_TRACE;

    // Coarsely integrated operand scanning (CIOS), a * b / R mod n. 'out' may be 'a' or 'b'
    const UInt32 k = m_limbCount;
    const UInt32* n = m_modulus.GetLimbs();
    UInt32* t = &m_scratch[ 0 ];
    memset( t, 0, ( k + 2 ) * sizeof( UInt32 ) );

    for ( UInt32 i=0; i<k; ++i )
    {
        UInt64 carry = 0;
        const UInt64 bi = b[ i ];
        for ( UInt32 j=0; j<k; ++j )
        {
            UInt64 sum = (UInt64) t[ j ] + (UInt64) a[ j ] * bi + carry;
            t[ j ] = (UInt32) sum;
            carry = sum >> 32;
        }
        UInt64 sum = (UInt64) t[ k ] + carry;
        t[ k ] = (UInt32) sum;
        t[ k+1 ] = (UInt32) ( sum >> 32 );

        const UInt64 m = (UInt32) ( t[ 0 ] * m_n0Inverse );
        sum = (UInt64) t[ 0 ] + m * (UInt64) n[ 0 ];
        carry = sum >> 32;
        for ( UInt32 j=1; j<k; ++j )
        {
            sum = (UInt64) t[ j ] + m * (UInt64) n[ j ] + carry;
            t[ j-1 ] = (UInt32) sum;
            carry = sum >> 32;
        }
        sum = (UInt64) t[ k ] + carry;
        t[ k-1 ] = (UInt32) sum;
        t[ k ] = t[ k+1 ] + (UInt32) ( sum >> 32 );
    }

    if ( 0 != t[ k ] || IsGreaterOrEqual( t, n, k ) )
        SubtractInPlace( t, n, k );
    memcpy( out, t, k * sizeof( UInt32 ) );
}

/*-------------------------------------------------------------------------*/

bool
CMontgomeryModulus::Initialize( const CBigUnsigned& modulus )
{GUCEF_TRACE;

    m_limbCount = 0;
    if ( !modulus.IsOdd() || modulus.GetBitLength() < 2 )
        return false;

    m_modulus = modulus;
    const UInt32 k = modulus.GetLimbCount();
    const UInt32 bitLength = modulus.GetBitLength();
    const UInt32* n = modulus.GetLimbs();

    // -1/n mod 2^32 by Newton iteration: every step doubles the number of correct bits
    UInt32 inverse = n[ 0 ];
    for ( UInt32 i=0; i<5; ++i )
        inverse *= 2 - n[ 0 ] * inverse;
    m_n0Inverse = 0 - inverse;

    m_limbCount = k;
    m_rr.assign( k, 0 );
    m_scratch.assign( k + 2, 0 );
    m_baseMontgomery.assign( k, 0 );
    m_accumulator.assign( k, 0 );
    m_one.assign( k, 0 );
    m_one[ 0 ] = 1;

    // R mod n: start at 2^(bitLength-1), which is below n, and double up to 2^(32k)
    UInt32* x = &m_accumulator[ 0 ];
    x[ ( bitLength - 1 ) / 32 ] = (UInt32) 1 << ( ( bitLength - 1 ) % 32 );
    for ( UInt32 i=bitLength-1; i<32*k; ++i )
        DoubleMod( x );

    // x is 1 in Montgomery form, doubled it is 2. R^2 mod n is 2^(32k) in Montgomery form:
    // square and double along the bits of 32k
    DoubleMod( x );
    const UInt32 exponent = 32 * k;
    UInt32 topBit = 31;
    while ( 0 == ( ( exponent >> topBit ) & 1 ) )
        --topBit;
    for ( UInt32 bit=topBit; bit>0; --bit )
    {
        MontMul( x, x, x );
        if ( 0 != ( ( exponent >> ( bit - 1 ) ) & 1 ) )
            DoubleMod( x );
    }
    memcpy( &m_rr[ 0 ], x, k * sizeof( UInt32 ) );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMontgomeryModulus::ModExp( const CBigUnsigned& base      ,
                            const CBigUnsigned& exponent  ,
                            CBigUnsigned& result          )
{GUCEF_TRACE;

    result.SetZero();
    if ( !IsInitialized() || exponent.IsZero() || base.Compare( m_modulus ) >= 0 )
        return false;

    const UInt32 k = m_limbCount;
    UInt32* acc = &m_accumulator[ 0 ];
    UInt32* baseM = &m_baseMontgomery[ 0 ];

    // the base in Montgomery form: base * R mod n
    memset( acc, 0, k * sizeof( UInt32 ) );
    if ( base.GetLimbCount() > 0 )
        memcpy( acc, base.GetLimbs(), base.GetLimbCount() * sizeof( UInt32 ) );
    MontMul( acc, &m_rr[ 0 ], baseM );

    // left to right square and multiply
    memcpy( acc, baseM, k * sizeof( UInt32 ) );
    const UInt32 bitLength = exponent.GetBitLength();
    for ( UInt32 bit=bitLength-1; bit>0; --bit )
    {
        MontMul( acc, acc, acc );
        if ( exponent.GetBit( bit - 1 ) )
            MontMul( acc, baseM, acc );
    }

    // and out of Montgomery form
    MontMul( acc, &m_one[ 0 ], acc );
    result.SetFromLimbs( acc, k );
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
