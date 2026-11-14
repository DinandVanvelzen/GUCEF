/*
 *  gucefCORE_TestApp: GUCEF test application for the CORE module
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

#include <math.h>
#include <string.h>
#include <float.h>
#include <stdlib.h>

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_CORE_CBIGINTEGER_H
#include "gucefCORE_CBigInteger.h"
#define GUCEF_CORE_CBIGINTEGER_H
#endif /* GUCEF_CORE_CBIGINTEGER_H ? */

#ifndef GUCEF_CORE_CBIGUNSIGNED_H
#include "gucefCORE_CBigUnsigned.h"
#define GUCEF_CORE_CBIGUNSIGNED_H
#endif /* GUCEF_CORE_CBIGUNSIGNED_H ? */

#ifndef GUCEF_CORE_TESTAPP_TESTDECIMALREFERENCE_H
#include "TestDecimalReference.h"
#define GUCEF_CORE_TESTAPP_TESTDECIMALREFERENCE_H
#endif /* GUCEF_CORE_TESTAPP_TESTDECIMALREFERENCE_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_CSTOPWATCH_H
#include "gucefCORE_CStopwatch.h"
#define GUCEF_CORE_CSTOPWATCH_H
#endif /* GUCEF_CORE_CSTOPWATCH_H ? */

#ifndef GUCEF_CORE_TESTAPP_TESTALLOCATIONCOUNTER_H
#include "TestAllocationCounter.h"
#define GUCEF_CORE_TESTAPP_TESTALLOCATIONCOUNTER_H
#endif /* GUCEF_CORE_TESTAPP_TESTALLOCATIONCOUNTER_H ? */

#include "TestBigInteger.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;
using namespace TestDecimalReference;
typedef CORE::CBigInteger CBigInteger;
typedef CORE::TBigIntegerStatus TBigIntegerStatus;

/*-------------------------------------------------------------------------*/

static const UInt64 g_testSeedBigInteger = 0x5EEDB161A7E6E5ULL;
static const UInt32 g_karatsubaThreshold = CBigInteger::KaratsubaThresholdLimbs;

/*-------------------------------------------------------------------------*/

/** The value of a big integer in the reference, built from the limbs only (no code of CBigInteger beyond GetLimbs) */
static CRefInt
RefOf( const CBigInteger& value )
{
    // Horner in base 10^9: acc = acc * 2^32 + limb, from the most significant limb down
    CRefInt r;
    const UInt32* limbs = value.GetLimbs();
    for ( UInt32 i=value.GetLimbCount(); i>0; --i )
    {
        UInt64 carry = limbs[ i-1 ];
        for ( size_t k=0; k<r.limbs.size(); ++k )
        {
            UInt64 p = ( (UInt64) r.limbs[ k ] << 32 ) + carry;     // < 10^9 * 2^32 + 2^33
            r.limbs[ k ] = (UInt32) ( p % CRefInt::Base );
            carry = p / CRefInt::Base;
        }
        while ( 0 != carry )
        {
            r.limbs.push_back( (UInt32) ( carry % CRefInt::Base ) );
            carry /= CRefInt::Base;
        }
    }
    r.Trim();
    r.negative = value.IsNegative() && !r.IsZero();
    return r;
}

/*-------------------------------------------------------------------------*/

/** Decimal text of a reference value with its sign */
static CORE::CString
RefText( const CRefInt& value )
{
    GUCEF::vector< char > digits = value.Digits();
    CORE::CString text;
    if ( value.negative )
        text = "-";
    text += CORE::CString( &digits[ 0 ], (UInt32) digits.size() );
    return text;
}

/*-------------------------------------------------------------------------*/

/** The representation invariants: no leading zero limb, zero never negative */
static bool
IsCanonical( const CBigInteger& value )
{
    UInt32 n = value.GetLimbCount();
    if ( 0 == n )
        return !value.IsNegative();
    return 0 != value.GetLimbs()[ n-1 ];
}

/*-------------------------------------------------------------------------*/

static bool
SameAsRef( const CBigInteger& value, const CRefInt& expected )
{
    return IsCanonical( value ) && 0 == CRefInt::Compare( RefOf( value ), expected );
}

/*-------------------------------------------------------------------------*/

/** Random magnitude of 'limbs' limbs with a non-zero top limb, limbs biased toward 0 and all ones at times */
static CBigInteger
RandomBig( CSplitMix64& rng, UInt32 limbs, bool allowNegative )
{
    GUCEF::vector< UInt32 > data( limbs + 1, 0 );
    UInt32 style = (UInt32) rng.Below( 8 );
    for ( UInt32 i=0; i<limbs; ++i )
    {
        UInt64 r = rng.Next();
        switch ( style )
        {
            case 0: data[ i ] = 0 == ( r & 3 ) ? 0xFFFFFFFFUL : (UInt32) ( r >> 32 ); break;
            case 1: data[ i ] = 0 == ( r & 3 ) ? 0 : (UInt32) ( r >> 32 ); break;
            default: data[ i ] = (UInt32) ( r >> 32 ); break;
        }
    }
    if ( limbs > 0 && 0 == data[ limbs-1 ] )
        data[ limbs-1 ] = 1 + (UInt32) rng.Below( 0xFFFFFFFEULL );
    CBigInteger value;
    CBigInteger::FromLimbs( &data[ 0 ], limbs, allowNegative && 0 != ( rng.Next() & 1 ), value );
    return value;
}

/*-------------------------------------------------------------------------*/

/** Operand sizes concentrated on the interesting boundaries: small, inline / heap, Karatsuba threshold, large */
static UInt32
RandomSize( CSplitMix64& rng, UInt32 maxLimbs )
{
    UInt32 pick = (UInt32) rng.Below( 100 );
    UInt32 size = 0;
    if ( pick < 35 )
        size = (UInt32) rng.Below( 11 );
    else
    if ( pick < 65 )
    {
        static const UInt32 special[] = { 7, 8, 9, 0, 0, 0, 0 };
        UInt32 which = (UInt32) rng.Below( 7 );
        switch ( which )
        {
            case 3: size = g_karatsubaThreshold - 1; break;
            case 4: size = g_karatsubaThreshold; break;
            case 5: size = g_karatsubaThreshold + 1; break;
            case 6: size = 2 * g_karatsubaThreshold; break;
            default: size = special[ which ]; break;
        }
    }
    else
    if ( pick < 97 )
        size = (UInt32) rng.Below( 70 );
    else
        size = (UInt32) rng.Below( maxLimbs + 1 );
    return size > maxLimbs ? maxLimbs : size;
}

/*-------------------------------------------------------------------------*/

static CBigInteger
Big( const char* decimal )
{
    return CBigInteger::Parse( decimal );
}

/*-------------------------------------------------------------------------*/

static CBigInteger
PowerOfTwo( UInt32 bits )
{
    CBigInteger value( 1 );
    CBigInteger::ThrowOnError( value.ShiftLeft( bits, value ) );
    return value;
}

/*-------------------------------------------------------------------------*/

static CBigInteger
Factorial( UInt32 n )
{
    CBigInteger value( 1 );
    for ( UInt32 k=2; k<=n; ++k )
        CBigInteger::ThrowOnError( value.Multiply( k, value ) );
    return value;
}

/*-------------------------------------------------------------------------*/

static bool
TextIs( const CBigInteger& value, const char* text, UInt32 base = 10 )
{
    return value.ToString( base ) == CORE::CString( text );
}

/*-------------------------------------------------------------------------*/

/** The digit sum of the decimal text */
static UInt32
DigitSum( const CORE::CString& text )
{
    UInt32 sum = 0;
    const char* chars = text.C_String();
    for ( UInt32 i=0; i<text.Length(); ++i )
        if ( chars[ i ] >= '0' && chars[ i ] <= '9' )
            sum += (UInt32) ( chars[ i ] - '0' );
    return sum;
}

/*-------------------------------------------------------------------------*/

static bool
StartsWith( const CORE::CString& text, const char* prefix )
{
    UInt32 n = (UInt32) strlen( prefix );
    return text.Length() >= n && 0 == memcmp( text.C_String(), prefix, n );
}

/*-------------------------------------------------------------------------*/

static bool
EndsWith( const CORE::CString& text, const char* suffix )
{
    UInt32 n = (UInt32) strlen( suffix );
    return text.Length() >= n && 0 == memcmp( text.C_String() + text.Length() - n, suffix, n );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      STEP 1: REPRESENTATION, ADD / SUBTRACT / COMPARE, SHIFTS, TEXT     //
//                                                                         //
//-------------------------------------------------------------------------*/

static void
TestRepresentation( void )
{
    GUCEF_TESTFW_TESTCASE( "CBigInteger: zero is never negative and the status bits equal the decimal status bits" )
        try
        {
            CBigInteger zero;
            ASSERT_TRUE( zero.IsZero() && !zero.IsNegative() && 0 == zero.GetSign() && 0 == zero.GetLimbCount() );
            CBigInteger negated = -zero;
            ASSERT_TRUE( negated.IsZero() && !negated.IsNegative() );
            CBigInteger product = CBigInteger( 0 ) * CBigInteger( -5 );
            ASSERT_TRUE( product.IsZero() && !product.IsNegative() );
            CBigInteger sum = CBigInteger( -5 ) + CBigInteger( 5 );
            ASSERT_TRUE( sum.IsZero() && !sum.IsNegative() && IsCanonical( sum ) );
            CBigInteger parsed = Big( "-0" );
            ASSERT_TRUE( parsed.IsZero() && !parsed.IsNegative() );
            ASSERT_TRUE( TextIs( parsed, "0" ) );

            ASSERT_TRUE( (UInt32) CORE::BIGINTEGERSTATUS_OK == (UInt32) CORE::DECIMALSTATUS_OK );
            ASSERT_TRUE( (UInt32) CORE::BIGINTEGERSTATUS_INEXACT == (UInt32) CORE::DECIMALSTATUS_INEXACT );
            ASSERT_TRUE( (UInt32) CORE::BIGINTEGERSTATUS_OVERFLOW == (UInt32) CORE::DECIMALSTATUS_OVERFLOW );
            ASSERT_TRUE( (UInt32) CORE::BIGINTEGERSTATUS_DIVISION_BY_ZERO == (UInt32) CORE::DECIMALSTATUS_DIVISION_BY_ZERO );
            ASSERT_TRUE( (UInt32) CORE::BIGINTEGERSTATUS_INVALID_SCALE == (UInt32) CORE::DECIMALSTATUS_INVALID_SCALE );
            ASSERT_TRUE( (UInt32) CORE::BIGINTEGERSTATUS_INVALID_FORMAT == (UInt32) CORE::DECIMALSTATUS_INVALID_FORMAT );
            ASSERT_TRUE( (UInt32) CORE::BIGINTEGERSTATUS_INVALID_OPERAND == (UInt32) CORE::DECIMALSTATUS_INVALID_OPERAND );
            ASSERT_TRUE( (UInt32) CORE::BIGINTEGERSTATUS_ROUNDING_REQUIRED == (UInt32) CORE::DECIMALSTATUS_ROUNDING_REQUIRED );
            ASSERT_TRUE( 0x80 == (UInt32) CORE::BIGINTEGERSTATUS_OUT_OF_MEMORY );
            ASSERT_FALSE( CORE::BigIntegerStatusIsError( CORE::BIGINTEGERSTATUS_INEXACT ) );
            ASSERT_TRUE( CORE::BigIntegerStatusIsError( CORE::BIGINTEGERSTATUS_OVERFLOW | CORE::BIGINTEGERSTATUS_INEXACT ) );

            // the object stays small: 8 inline limbs, size, capacity and sign
            if ( sizeof( void* ) == 8 )
                ASSERT_TRUE( sizeof( CBigInteger ) <= 48 );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "CBigInteger: INT64_MIN, UInt64 max and every built-in integer type construct exactly" )
        try
        {
            const Int64 int64Min = -9223372036854775807LL - 1;
            CBigInteger minimum( int64Min );
            ASSERT_TRUE( minimum.IsNegative() && 64 == minimum.GetBitLength() && 63 == minimum.GetLowestSetBit() );
            ASSERT_TRUE( TextIs( minimum, "-9223372036854775808" ) );
            Int64 back = 0;
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_OK == minimum.ToInt64( back ) && int64Min == back );
            Int64 untouched = 7;
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_OVERFLOW == ( minimum - 1 ).ToInt64( untouched ) && 7 == untouched );
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_OVERFLOW == PowerOfTwo( 63 ).ToInt64( untouched ) && 7 == untouched );
            ASSERT_TRUE( minimum.FitsInt64() && !( minimum - 1 ).FitsInt64() && !PowerOfTwo( 63 ).FitsInt64() );

            const UInt64 uint64Max = 18446744073709551615ULL;
            CBigInteger maximum( uint64Max );
            ASSERT_TRUE( !maximum.IsNegative() && TextIs( maximum, "18446744073709551615" ) );
            UInt64 ub = 0;
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_OK == maximum.ToUInt64( ub ) && uint64Max == ub );
            UInt64 uuntouched = 3;
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_OVERFLOW == ( maximum + 1 ).ToUInt64( uuntouched ) && 3 == uuntouched );
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_OVERFLOW == CBigInteger( -1 ).ToUInt64( uuntouched ) && 3 == uuntouched );

            long l = -123456789L;
            unsigned long ul = 4000000000UL;
            size_t sz = (size_t) 1234567;
            short s = -12;
            unsigned short us = 65535;
            signed char sc = -128;
            unsigned char uc = 255;
            unsigned int ui = 4294967295U;
            ASSERT_TRUE( TextIs( CBigInteger( l ), "-123456789" ) );
            ASSERT_TRUE( TextIs( CBigInteger( ul ), "4000000000" ) );
            ASSERT_TRUE( TextIs( CBigInteger( sz ), "1234567" ) );
            ASSERT_TRUE( TextIs( CBigInteger( s ), "-12" ) );
            ASSERT_TRUE( TextIs( CBigInteger( us ), "65535" ) );
            ASSERT_TRUE( TextIs( CBigInteger( sc ), "-128" ) );
            ASSERT_TRUE( TextIs( CBigInteger( uc ), "255" ) );
            ASSERT_TRUE( TextIs( CBigInteger( ui ), "4294967295" ) );
            ASSERT_TRUE( CBigInteger( 5 ) == 5 && 5 == CBigInteger( 5 ) && CBigInteger( 5 ) < 10L && CBigInteger( uint64Max ) > 0 );
            ASSERT_TRUE( 0 == maximum.Compare( uint64Max ) && maximum.Compare( int64Min ) > 0 && minimum.Compare( int64Min ) == 0 );
            ASSERT_TRUE( minimum.Compare( (Int64) -1 ) < 0 && CBigInteger( -1 ).Compare( int64Min ) > 0 );

            CBigInteger viaFactory;
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_OK == CBigInteger::FromInt64( int64Min, viaFactory ) && viaFactory == minimum );
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_OK == CBigInteger::FromUInt64( uint64Max, viaFactory ) && viaFactory == maximum );
            ASSERT_TRUE( CBigInteger::Zero().IsZero() && CBigInteger::One().IsOne() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "CBigInteger: values of 7, 8 and 9 limbs across the inline to heap boundary, copy, move, swap, aliasing" )
        try
        {
            CSplitMix64 rng( g_testSeedBigInteger );
            for ( UInt32 limbs=7; limbs<=9; ++limbs )
            {
                CBigInteger a = RandomBig( rng, limbs, true );
                CBigInteger b = RandomBig( rng, limbs, true );
                CRefInt ra = RefOf( a );
                CRefInt rb = RefOf( b );
                ASSERT_TRUE( a.IsInline() == ( limbs <= CBigInteger::InlineLimbCount ) );

                CBigInteger copy( a );
                ASSERT_TRUE( copy == a && SameAsRef( copy, ra ) );
                CBigInteger assigned;
                assigned = b;
                ASSERT_TRUE( assigned == b );
                assigned = assigned;
                ASSERT_TRUE( assigned == b );
                assigned.Swap( copy );
                ASSERT_TRUE( assigned == a && copy == b );

                #ifdef GUCEF_MOVE_SEMANTICS_SUPPORTED
                CBigInteger moved( GUCEF_MOVE( copy ) );
                ASSERT_TRUE( moved == b && copy.IsZero() && IsCanonical( copy ) );
                copy = GUCEF_MOVE( moved );
                ASSERT_TRUE( copy == b && moved.IsZero() );
                #endif

                // results aliasing the operands
                CBigInteger x( a );
                ASSERT_TRUE( CORE::BIGINTEGERSTATUS_OK == x.Add( x, x ) );
                ASSERT_TRUE( SameAsRef( x, CRefInt::Add( ra, ra ) ) );
                x = a;
                ASSERT_TRUE( CORE::BIGINTEGERSTATUS_OK == x.Multiply( x, x ) );
                ASSERT_TRUE( SameAsRef( x, CRefInt::Multiply( ra, ra ) ) );
                x = a;
                ASSERT_TRUE( CORE::BIGINTEGERSTATUS_OK == x.Subtract( b, x ) );
                ASSERT_TRUE( SameAsRef( x, CRefInt::Subtract( ra, rb ) ) );
                x = b;
                ASSERT_TRUE( CORE::BIGINTEGERSTATUS_OK == a.Subtract( x, x ) );
                ASSERT_TRUE( SameAsRef( x, CRefInt::Subtract( ra, rb ) ) );
                x = a;
                ASSERT_TRUE( CORE::BIGINTEGERSTATUS_OK == x.ShiftLeft( 37, x ) );
                ASSERT_TRUE( SameAsRef( x, CRefInt::Multiply( ra, CRefInt::PowerOfTwo( 37 ) ) ) );
                x = a;
                ASSERT_TRUE( CORE::BIGINTEGERSTATUS_OK == x.Multiply( b, x ) );
                ASSERT_TRUE( SameAsRef( x, CRefInt::Multiply( ra, rb ) ) );
                x = b;
                ASSERT_TRUE( CORE::BIGINTEGERSTATUS_OK == a.Multiply( x, x ) );
                ASSERT_TRUE( SameAsRef( x, CRefInt::Multiply( ra, rb ) ) );

                // a heap value assigned back into a small one keeps its block until Compact
                CBigInteger big = a * b * b;
                ASSERT_FALSE( big.IsInline() );
                CBigInteger small( 12345 );
                big = small;        // a copy into the existing block
                ASSERT_TRUE( big == 12345 && !big.IsInline() );
                ASSERT_TRUE( CORE::BIGINTEGERSTATUS_OK == big.Compact() && big.IsInline() && big == 12345 );
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;
}

/*-------------------------------------------------------------------------*/

static void
TestAddSubtractCompare( void )
{
    GUCEF_TESTFW_TESTCASE( "CBigInteger: add, subtract and compare against the base 10^9 reference, every sign combination" )
        try
        {
            CSplitMix64 rng( g_testSeedBigInteger + 1 );
            UInt32 failures = 0;
            for ( UInt32 i=0; i<10000 && failures < 5; ++i )
            {
                CBigInteger a = RandomBig( rng, RandomSize( rng, 300 ), true );
                CBigInteger b = 0 == rng.Below( 10 ) ? a : RandomBig( rng, RandomSize( rng, 300 ), true );
                if ( 0 == rng.Below( 10 ) )
                    b = -a;
                CRefInt ra = RefOf( a );
                CRefInt rb = RefOf( b );
                CBigInteger sum;
                CBigInteger difference;
                bool ok = CORE::BIGINTEGERSTATUS_OK == a.Add( b, sum ) && CORE::BIGINTEGERSTATUS_OK == a.Subtract( b, difference );
                ok = ok && SameAsRef( sum, CRefInt::Add( ra, rb ) ) && SameAsRef( difference, CRefInt::Subtract( ra, rb ) );
                ok = ok && a.Compare( b ) == CRefInt::Compare( ra, rb ) && b.Compare( a ) == CRefInt::Compare( rb, ra );
                ok = ok && ( a < b ) == ( CRefInt::Compare( ra, rb ) < 0 ) && ( a == b ) == ( 0 == CRefInt::Compare( ra, rb ) );
                ok = ok && a.CompareMagnitude( b ) == CRefInt::CompareMagnitude( ra, rb );

                // built-in operand fast paths
                Int64 small = (Int64) rng.Next();
                UInt64 usmall = rng.Next();
                CBigInteger s1;
                CBigInteger s2;
                CBigInteger s3;
                ok = ok && CORE::BIGINTEGERSTATUS_OK == a.Add( small, s1 ) && CORE::BIGINTEGERSTATUS_OK == a.Subtract( usmall, s2 ) && CORE::BIGINTEGERSTATUS_OK == a.Subtract( small, s3 );
                ok = ok && SameAsRef( s1, CRefInt::Add( ra, CRefInt::FromInt64( small ) ) );
                ok = ok && SameAsRef( s2, CRefInt::Subtract( ra, CRefInt::FromUInt64( usmall ) ) );
                ok = ok && SameAsRef( s3, CRefInt::Subtract( ra, CRefInt::FromInt64( small ) ) );
                ok = ok && a.Compare( small ) == CRefInt::Compare( ra, CRefInt::FromInt64( small ) );
                ok = ok && a.Compare( usmall ) == CRefInt::Compare( ra, CRefInt::FromUInt64( usmall ) );
                if ( !ok )
                {
                    ++failures;
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CBigInteger add/subtract/compare failed at iteration " << i << " seed " << ( g_testSeedBigInteger + 1 ) << " a=" << a.ToString( 16 ) << " b=" << b.ToString( 16 ) );
                    ERRORHERE;
                }
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "CBigInteger: invariants hold after every operation of a seeded sequence of mixed operations" )
        try
        {
            CSplitMix64 rng( g_testSeedBigInteger + 2 );
            CBigInteger values[ 4 ];
            UInt32 failures = 0;
            for ( UInt32 i=0; i<100000 && failures < 5; ++i )
            {
                CBigInteger& r = values[ rng.Below( 4 ) ];
                const CBigInteger& a = values[ rng.Below( 4 ) ];
                const CBigInteger& b = values[ rng.Below( 4 ) ];
                TBigIntegerStatus status = CORE::BIGINTEGERSTATUS_OK;
                switch ( rng.Below( 9 ) )
                {
                    case 0: status = a.Add( b, r ); break;
                    case 1: status = a.Subtract( b, r ); break;
                    case 2: if ( a.GetLimbCount() + b.GetLimbCount() < 40 ) status = a.Multiply( b, r ); break;
                    case 3: status = a.ShiftRight( (UInt32) rng.Below( 70 ), r ); break;
                    case 4: if ( a.GetLimbCount() < 30 ) status = a.ShiftLeft( (UInt32) rng.Below( 70 ), r ); break;
                    case 5: r = RandomBig( rng, (UInt32) rng.Below( 12 ), true ); break;
                    case 6: status = a.Negate( r ); break;
                    case 7: status = a.Subtract( (Int64) rng.Below( 5 ) - 2, r ); break;
                    default: status = a.Add( (Int64) rng.Next(), r ); break;
                }
                if ( CORE::BIGINTEGERSTATUS_OK != status || !IsCanonical( r ) )
                {
                    ++failures;
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CBigInteger invariant failed at iteration " << i << " seed " << ( g_testSeedBigInteger + 2 ) );
                    ERRORHERE;
                }
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;
}

/*-------------------------------------------------------------------------*/

static void
TestShifts( void )
{
    GUCEF_TESTFW_TESTCASE( "CBigInteger: shifts are multiplication and floor division by powers of two" )
        try
        {
            ASSERT_TRUE( ( CBigInteger( -5 ) >> 1 ) == -3 );
            ASSERT_TRUE( ( CBigInteger( -1 ) >> 1000 ) == -1 );
            ASSERT_TRUE( ( CBigInteger( 5 ) >> 1000 ) == 0 );
            ASSERT_TRUE( ( CBigInteger( -5 ) << 3 ) == -40 );
            CBigInteger magnitudeShift;
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_OK == CBigInteger( -5 ).ShiftRightMagnitude( 1, magnitudeShift ) && magnitudeShift == -2 );
            ASSERT_TRUE( ( CBigInteger( -4 ) >> 1 ) == -2 && ( CBigInteger( -4 ) >> 2 ) == -1 && ( CBigInteger( -4 ) >> 3 ) == -1 );

            CSplitMix64 rng( g_testSeedBigInteger + 3 );
            static const UInt32 shifts[] = { 0, 1, 31, 32, 33, 63, 64, 65, 100, 255, 256, 257, 1000 };
            UInt32 failures = 0;
            for ( UInt32 i=0; i<400 && failures < 5; ++i )
            {
                CBigInteger a = RandomBig( rng, RandomSize( rng, 40 ), true );
                CRefInt ra = RefOf( a );
                for ( UInt32 s=0; s<sizeof( shifts ) / sizeof( shifts[ 0 ] ); ++s )
                {
                    UInt32 k = shifts[ s ];
                    CRefInt power = CRefInt::PowerOfTwo( k );
                    CBigInteger left;
                    CBigInteger right;
                    CBigInteger towardZero;
                    bool ok = CORE::BIGINTEGERSTATUS_OK == a.ShiftLeft( k, left ) && CORE::BIGINTEGERSTATUS_OK == a.ShiftRight( k, right );
                    ok = ok && CORE::BIGINTEGERSTATUS_OK == a.ShiftRightMagnitude( k, towardZero );
                    ok = ok && SameAsRef( left, CRefInt::Multiply( ra, power ) );

                    // floor( a / 2^k ) from the truncating reference division
                    CRefInt q;
                    CRefInt r;
                    CRefInt::DivMod( ra, power, q, r );
                    q.negative = ra.negative && !q.IsZero();
                    ok = ok && SameAsRef( towardZero, q );
                    if ( ra.negative && !r.IsZero() )
                        q = CRefInt::Subtract( q, CRefInt::FromUInt64( 1 ) );
                    ok = ok && SameAsRef( right, q );

                    // in place
                    CBigInteger inPlace( a );
                    ok = ok && CORE::BIGINTEGERSTATUS_OK == inPlace.ShiftLeft( k, inPlace ) && inPlace == left;
                    ok = ok && CORE::BIGINTEGERSTATUS_OK == inPlace.ShiftRight( k, inPlace ) && inPlace == a;
                    if ( !ok )
                    {
                        ++failures;
                        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CBigInteger shift failed at iteration " << i << " shift " << k << " seed " << ( g_testSeedBigInteger + 3 ) );
                        ERRORHERE;
                    }
                }
            }

            // a shift beyond the representable size fails fast and leaves the result unchanged
            CBigInteger result( 77 );
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_OVERFLOW == CBigInteger( 3 ).ShiftLeft( 0xFFFFFFFFUL, result ) && result == 77 );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;
}

/*-------------------------------------------------------------------------*/

static void
TestText( void )
{
    GUCEF_TESTFW_TESTCASE( "CBigInteger: decimal text equals the reference digits and every base round trips" )
        try
        {
            CSplitMix64 rng( g_testSeedBigInteger + 4 );
            UInt32 failures = 0;
            for ( UInt32 i=0; i<200 && failures < 5; ++i )
            {
                CBigInteger a = RandomBig( rng, (UInt32) rng.Below( 201 ), true );
                if ( 0 == i ) a = CBigInteger();
                if ( 1 == i ) a = CBigInteger( 1 );
                if ( 2 == i ) a = CBigInteger( -1 );
                if ( 3 == i ) a = CBigInteger( 4294967295UL );
                if ( 4 == i ) a = CBigInteger( 4294967296ULL );
                if ( 5 == i ) a = -CBigInteger( 4294967296ULL );
                bool ok = a.ToString() == RefText( RefOf( a ) );
                for ( UInt32 base=2; base<=36 && ok; ++base )
                {
                    CBigInteger back;
                    ok = CORE::BIGINTEGERSTATUS_OK == CBigInteger::FromString( a.ToString( base ), base, back ) && back == a;
                    ok = ok && CORE::BIGINTEGERSTATUS_OK == CBigInteger::FromString( a.ToString( base, true ), base, back ) && back == a;

                    // base^k and base^k - 1
                    UInt32 k = 1 + (UInt32) rng.Below( 60 );
                    CBigInteger power( 1 );
                    for ( UInt32 j=0; j<k; ++j )
                        power *= base;
                    CBigInteger below = power - 1;
                    ok = ok && CORE::BIGINTEGERSTATUS_OK == CBigInteger::FromString( power.ToString( base ), base, back ) && back == power;
                    ok = ok && power.ToString( base ).Length() == k + 1;
                    ok = ok && below.ToString( base ).Length() == k;
                    ok = ok && CORE::BIGINTEGERSTATUS_OK == CBigInteger::FromString( below.ToString( base ), base, back ) && back == below;
                }

                // power of two bases equal the bit regrouping of the limbs
                const UInt32* limbs = a.GetLimbs();
                CORE::CString hex = a.IsNegative() ? "-" : "";
                bool leading = true;
                for ( UInt32 l=a.GetLimbCount(); l>0; --l )
                {
                    for ( Int32 nibble=7; nibble>=0; --nibble )
                    {
                        UInt32 digit = ( limbs[ l-1 ] >> ( 4 * nibble ) ) & 0xF;
                        if ( leading && 0 == digit )
                            continue;
                        leading = false;
                        hex += "0123456789abcdef"[ digit ];
                    }
                }
                if ( a.IsZero() )
                    hex = "0";
                ok = ok && a.ToString( 16 ) == hex;
                if ( !ok )
                {
                    ++failures;
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CBigInteger text round trip failed at iteration " << i << " seed " << ( g_testSeedBigInteger + 4 ) );
                    ERRORHERE;
                }
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "CBigInteger: parse grammar, flags and failures" )
        try
        {
            CBigInteger v;
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_OK == CBigInteger::FromString( "zz", 36, v ) && v == 1295 );
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_OK == CBigInteger::FromString( "ZZ", 36, v ) && v == 1295 );
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_OK == CBigInteger::FromString( "+5", 10, v ) && v == 5 );
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_OK == CBigInteger::FromString( "-0", 10, v ) && v.IsZero() && !v.IsNegative() );
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_OK == CBigInteger::FromString( "000123", 10, v ) && v == 123 );
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_OK == CBigInteger::FromString( "-00", 16, v ) && v.IsZero() );
            ASSERT_TRUE( TextIs( CBigInteger( 255 ), "FF", 16 ) == false && CBigInteger( 255 ).ToString( 16, true ) == "FF" );

            // prefixes only with the flag, base 0 detects
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_INVALID_FORMAT == CBigInteger::FromString( "0x1f", 16, v ) );
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_INVALID_OPERAND == CBigInteger::FromString( "0x1f", 0, v ) );
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_OK == CBigInteger::FromString( "0x1f", 16, v, CORE::BIGINTEGERPARSE_ALLOW_BASE_PREFIX ) && v == 31 );
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_OK == CBigInteger::FromString( "-0x1F", 0, v, CORE::BIGINTEGERPARSE_ALLOW_BASE_PREFIX ) && v == -31 );
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_OK == CBigInteger::FromString( "0b101", 0, v, CORE::BIGINTEGERPARSE_ALLOW_BASE_PREFIX ) && v == 5 );
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_OK == CBigInteger::FromString( "0o17", 0, v, CORE::BIGINTEGERPARSE_ALLOW_BASE_PREFIX ) && v == 15 );
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_OK == CBigInteger::FromString( "017", 0, v, CORE::BIGINTEGERPARSE_ALLOW_BASE_PREFIX ) && v == 17 );
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_INVALID_FORMAT == CBigInteger::FromString( "0x", 0, v, CORE::BIGINTEGERPARSE_ALLOW_BASE_PREFIX ) );
            // in base 16 '0b1' is the hexadecimal number b1, the prefix of another base is not recognised
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_OK == CBigInteger::FromString( "0b1", 16, v, CORE::BIGINTEGERPARSE_ALLOW_BASE_PREFIX ) && v == 0xB1 );
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_INVALID_FORMAT == CBigInteger::FromString( "0x1", 2, v, CORE::BIGINTEGERPARSE_ALLOW_BASE_PREFIX ) );

            // separators only with the flag, single and between digits
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_INVALID_FORMAT == CBigInteger::FromString( "1_000", 10, v ) );
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_OK == CBigInteger::FromString( "1_000_000", 10, v, CORE::BIGINTEGERPARSE_ALLOW_DIGIT_SEPARATORS ) && v == 1000000 );
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_INVALID_FORMAT == CBigInteger::FromString( "1__0", 10, v, CORE::BIGINTEGERPARSE_ALLOW_DIGIT_SEPARATORS ) );
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_INVALID_FORMAT == CBigInteger::FromString( "_10", 10, v, CORE::BIGINTEGERPARSE_ALLOW_DIGIT_SEPARATORS ) );
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_INVALID_FORMAT == CBigInteger::FromString( "10_", 10, v, CORE::BIGINTEGERPARSE_ALLOW_DIGIT_SEPARATORS ) );
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_OK == CBigInteger::FromString( "ff_ff", 16, v, CORE::BIGINTEGERPARSE_ALLOW_DIGIT_SEPARATORS ) && v == 65535 );

            // whitespace only with the flag
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_INVALID_FORMAT == CBigInteger::FromString( " 12", 10, v ) );
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_OK == CBigInteger::FromString( " \t-12\t ", 10, v, CORE::BIGINTEGERPARSE_ALLOW_SURROUNDING_WHITESPACE ) && v == -12 );

            // failures leave the result unchanged
            v = 42;
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_INVALID_FORMAT == CBigInteger::FromString( "", 10, v ) && v == 42 );
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_INVALID_FORMAT == CBigInteger::FromString( "-", 10, v ) && v == 42 );
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_INVALID_FORMAT == CBigInteger::FromString( "+", 10, v ) && v == 42 );
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_INVALID_FORMAT == CBigInteger::FromString( "12", 2, v ) && v == 42 );
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_INVALID_FORMAT == CBigInteger::FromString( "1 2", 10, v ) && v == 42 );
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_INVALID_FORMAT == CBigInteger::FromString( "12abc", 10, v ) && v == 42 );
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_INVALID_FORMAT == CBigInteger::FromString( "--1", 10, v ) && v == 42 );
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_INVALID_OPERAND == CBigInteger::FromString( "1", 1, v ) && v == 42 );
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_INVALID_OPERAND == CBigInteger::FromString( "1", 37, v ) && v == 42 );

            bool threw = false;
            try
            {
                CBigInteger::Parse( "12x" );
            }
            catch ( const GUCEF::invalid_argument& )
            {
                threw = true;
            }
            ASSERT_TRUE( threw );
            ASSERT_TRUE( CBigInteger::Parse( "-1234567890123456789012345678901234567890" ).ToString() == "-1234567890123456789012345678901234567890" );

            // ToChars: exact sizing, a short buffer writes nothing, the upper bound always suffices
            CBigInteger x = Big( "-123456789012345678901234567890" );
            char buffer[ 64 ];
            memset( buffer, 'Z', sizeof( buffer ) );
            ASSERT_TRUE( 0 == x.ToChars( buffer, 31 ) && 'Z' == buffer[ 0 ] && 'Z' == buffer[ 30 ] );
            ASSERT_TRUE( 31 == x.ToChars( buffer, 32 ) && 0 == strcmp( buffer, "-123456789012345678901234567890" ) );
            ASSERT_TRUE( x.GetTextLengthUpperBound( 10 ) >= 32 && 0 == x.GetTextLengthUpperBound( 1 ) );
            ASSERT_TRUE( 0 == x.ToChars( buffer, sizeof( buffer ), 37 ) );
            ASSERT_TRUE( x.ToString( 37 ).IsNULLOrEmpty() );
            CSplitMix64 rng( g_testSeedBigInteger + 5 );
            for ( UInt32 i=0; i<100; ++i )
            {
                CBigInteger value = RandomBig( rng, (UInt32) rng.Below( 40 ), true );
                for ( UInt32 base=2; base<=36; ++base )
                    ASSERT_TRUE( value.GetTextLengthUpperBound( base ) >= value.ToString( base ).Length() + 1 );
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;
}

/*-------------------------------------------------------------------------*/

static void
TestNoAllocationForSmallValues( void )
{
    GUCEF_TESTFW_TESTCASE( "CBigInteger: operations on values up to 256 bits make no heap allocation" )
        GUCEF_TESTFW_SKIP_IF( !CORETEST::IsAllocationCountingAvailable(), CORETEST::GetAllocationCountingUnavailableReason() );
        try
        {

            CBigInteger a = Big( "-57896044618658097711785492504343953926634992332820282019728792003956564819967" );    // 255 bits
            CBigInteger b = Big( "340282366920938463463374607431768211455" );    // 128 bits
            CBigInteger c = Big( "18446744073709551629" );
            CBigInteger r;
            CBigInteger q;
            char buffer[ 128 ];
            Int64 small = 0;

            CORETEST::StartAllocationCounting();
            r = a;
            a.Add( b, r );
            a.Subtract( b, r );
            b.Multiply( c, r );
            b.Square( r );
            r.Add( 5, r );
            c.Multiply( (Int64) -7, r );
            a.ShiftRight( 37, r );
            b.ShiftLeft( 100, r );
            a.Compare( b );
            a.ToChars( buffer, sizeof( buffer ) );
            b.ToChars( buffer, sizeof( buffer ), 16 );
            CBigInteger::FromChars( "123456789012345678901234567890", 30, 10, r );
            CBigInteger copy( b );
            copy.Swap( r );
            r.Negate( r );
            CBigInteger( 3 ).Pow( 150, r );
            a.DivMod( c, q, r );
            a.DivMod( b, q, r );
            a.DivMod( (Int64) 1000000007, q, small );
            a.Modulo( b, r );
            a.Gcd( b, r );
            b.ISqrt( r );
            b.And( c, r );
            a.Or( c, r );
            a.Xor( b, r );
            UInt32 allocations = CORETEST::StopAllocationCounting();
            ASSERT_TRUE( 0 == allocations );
            if ( 0 != allocations )
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CBigInteger small value allocations: " << allocations << "\n" << CORETEST::GetCountedAllocationStacks() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      STEP 2: MULTIPLICATION, SQUARING, POW                              //
//                                                                         //
//-------------------------------------------------------------------------*/

static void
TestMultiplication( void )
{
    GUCEF_TESTFW_TESTCASE( "CBigInteger: schoolbook and Karatsuba products and squares against the reference" )
        try
        {
            CSplitMix64 rng( g_testSeedBigInteger + 10 );
            UInt32 failures = 0;
            for ( UInt32 i=0; i<10000 && failures < 5; ++i )
            {
                UInt32 an = RandomSize( rng, 300 );
                UInt32 bn = RandomSize( rng, 300 );
                if ( 0 == rng.Below( 8 ) )
                {
                    // unbalanced n = 3m
                    bn = 1 + (UInt32) rng.Below( 60 );
                    an = 3 * bn;
                }
                CBigInteger a = RandomBig( rng, an, true );
                CBigInteger b = RandomBig( rng, bn, true );
                CRefInt ra = RefOf( a );
                CRefInt rb = RefOf( b );
                CBigInteger product;
                CBigInteger square;
                bool ok = CORE::BIGINTEGERSTATUS_OK == a.Multiply( b, product ) && SameAsRef( product, CRefInt::Multiply( ra, rb ) );
                ok = ok && CORE::BIGINTEGERSTATUS_OK == a.Square( square ) && SameAsRef( square, CRefInt::Multiply( ra, ra ) );
                ok = ok && a * b == b * a;
                Int64 small = (Int64) rng.Next();
                CBigInteger scaled;
                ok = ok && CORE::BIGINTEGERSTATUS_OK == a.Multiply( small, scaled ) && SameAsRef( scaled, CRefInt::Multiply( ra, CRefInt::FromInt64( small ) ) );
                if ( !ok )
                {
                    ++failures;
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CBigInteger multiply failed at iteration " << i << " sizes " << an << "x" << bn << " seed " << ( g_testSeedBigInteger + 10 ) );
                    ERRORHERE;
                }
            }

            // the threshold neighbourhood with all ones (the largest carries)
            for ( UInt32 n=g_karatsubaThreshold-2; n<=2*g_karatsubaThreshold+3; ++n )
            {
                CBigInteger ones = PowerOfTwo( 32 * n ) - 1;
                CBigInteger expected = PowerOfTwo( 64 * n ) - PowerOfTwo( 32 * n + 1 ) + 1;
                ASSERT_TRUE( ones * ones == expected );
                CBigInteger square;
                ASSERT_TRUE( CORE::BIGINTEGERSTATUS_OK == ones.Square( square ) && square == expected );
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "CBigInteger: factorials and published constants" )
        try
        {
            CBigInteger f100 = Factorial( 100 );
            CORE::CString text = f100.ToString();
            ASSERT_TRUE( 158 == text.Length() );
            UInt32 trailingZeros = 0;
            for ( UInt32 i=text.Length(); i>0 && '0' == text.C_String()[ i-1 ]; --i )
                ++trailingZeros;
            ASSERT_TRUE( 24 == trailingZeros );
            ASSERT_TRUE( 648 == DigitSum( text ) );
            ASSERT_TRUE( 525 == f100.GetBitLength() );
            ASSERT_TRUE( StartsWith( f100.ToString( 16 ), "1b30964ec395dc2406" ) );
            ASSERT_TRUE( Factorial( 50 ) * Factorial( 50 ) * Big( "100891344545564193334812497256" ) == f100 );
            ASSERT_TRUE( Factorial( 98 ) * 9900 == f100 );
            ASSERT_TRUE( TextIs( Factorial( 20 ), "2432902008176640000" ) );
            ASSERT_TRUE( TextIs( Factorial( 25 ), "15511210043330985984000000" ) );
            UInt64 u = 0;
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_OK == Factorial( 20 ).ToUInt64( u ) && 2432902008176640000ULL == u );
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_OVERFLOW == Factorial( 21 ).ToUInt64( u ) );
            ASSERT_TRUE( Factorial( 57 ).IsInline() && 255 == Factorial( 57 ).GetBitLength() );

            CBigInteger mersenne = PowerOfTwo( 521 ) - 1;
            CORE::CString m = mersenne.ToString();
            ASSERT_TRUE( 157 == m.Length() && StartsWith( m, "686479766013" ) && EndsWith( m, "291115057151" ) );
            ASSERT_TRUE( 521 == mersenne.GetBitLength() );
            CORE::CString hex = mersenne.ToString( 16 );
            bool allF = 131 == hex.Length() && '1' == hex.C_String()[ 0 ];
            for ( UInt32 i=1; i<hex.Length(); ++i )
                allF = allF && 'f' == hex.C_String()[ i ];
            ASSERT_TRUE( allF );

            CBigInteger p;
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_OK == CBigInteger( 2 ).Pow( 1000, p ) );
            ASSERT_TRUE( 302 == p.ToString().Length() && StartsWith( p.ToString(), "1071508607" ) );
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_OK == CBigInteger( 3 ).Pow( 1000, p ) && 478 == p.ToString().Length() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "CBigInteger: Pow edge cases, repeated multiplication and the size limit" )
        try
        {
            CBigInteger r;
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_OK == CBigInteger( 0 ).Pow( 0, r ) && r == 1 );
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_OK == CBigInteger( 0 ).Pow( 5, r ) && r == 0 );
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_OK == CBigInteger( -1 ).Pow( 7, r ) && r == -1 );
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_OK == CBigInteger( -1 ).Pow( 8, r ) && r == 1 );
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_OK == CBigInteger( 12345 ).Pow( 1, r ) && r == 12345 );
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_OK == CBigInteger( -2 ).Pow( 63, r ) && r == CBigInteger( -9223372036854775807LL - 1 ) );
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_OK == CBigInteger( 2 ).Pow( 64, r ) && TextIs( r, "18446744073709551616" ) );
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_OK == CBigInteger( 10 ).Pow( 100, r ) && r.ToString().Length() == 101 );
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_OK == CBigInteger( -8 ).Pow( 21, r ) && r == -PowerOfTwo( 63 ) );

            CSplitMix64 rng( g_testSeedBigInteger + 11 );
            for ( UInt32 i=0; i<40; ++i )
            {
                CBigInteger base = RandomBig( rng, 1 + (UInt32) rng.Below( 3 ), true );
                CBigInteger repeated( 1 );
                for ( UInt32 e=0; e<=40; ++e )
                {
                    CBigInteger power;
                    ASSERT_TRUE( CORE::BIGINTEGERSTATUS_OK == base.Pow( e, power ) && power == repeated );
                    repeated *= base;
                }
            }

            // an absurd size fails fast without allocating and leaves the result unchanged
            r = 99;
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_OVERFLOW == CBigInteger( 3 ).Pow( 0xFFFFFFFFUL, r ) && r == 99 );
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_OVERFLOW == CBigInteger( 2 ).Pow( 0xFFFFFFFFUL, r ) && r == 99 );
            bool threw = false;
            try
            {
                CBigInteger huge = PowerOfTwo( 1 ) << 0xFFFFFFF0UL;
            }
            catch ( const std::length_error& )
            {
                threw = true;
            }
            ASSERT_TRUE( threw );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      STEP 3: DIVISION                                                   //
//                                                                         //
//-------------------------------------------------------------------------*/

/** n == q d + r, |r| < |d| and r has the sign of n (or is 0): the truncating division, checked in the reference */
static bool
IsTruncatingDivision( const CRefInt& n, const CRefInt& d, const CBigInteger& q, const CBigInteger& r )
{
    if ( !IsCanonical( q ) || !IsCanonical( r ) )
        return false;
    CRefInt rq = RefOf( q );
    CRefInt rr = RefOf( r );
    if ( 0 != CRefInt::Compare( CRefInt::Add( CRefInt::Multiply( rq, d ), rr ), n ) )
        return false;
    if ( CRefInt::CompareMagnitude( rr, d ) >= 0 )
        return false;
    return rr.IsZero() || rr.negative == n.negative;
}

/*-------------------------------------------------------------------------*/

/** Little endian limbs into a value */
static CBigInteger
FromLimbList( const UInt32* limbs, UInt32 count )
{
    CBigInteger value;
    CBigInteger::FromLimbs( limbs, count, false, value );
    return value;
}

/*-------------------------------------------------------------------------*/

static void
TestDivision( void )
{
    GUCEF_TESTFW_TESTCASE( "CBigInteger: truncating, floored and Euclidean division against the reference" )
        try
        {
            CSplitMix64 rng( g_testSeedBigInteger + 20 );
            UInt32 failures = 0;
            for ( UInt32 i=0; i<10000 && failures < 5; ++i )
            {
                UInt32 dn = RandomSize( rng, 300 );
                if ( 0 == dn )
                    dn = 1;
                UInt32 nn = RandomSize( rng, 300 );
                if ( 0 == rng.Below( 3 ) )
                    nn = dn + (UInt32) rng.Below( 4 );      // close sizes: the most quotient digit corrections
                CBigInteger n = RandomBig( rng, nn, true );
                CBigInteger d = RandomBig( rng, dn, true );
                CRefInt rn = RefOf( n );
                CRefInt rd = RefOf( d );

                CBigInteger q;
                CBigInteger r;
                bool ok = CORE::BIGINTEGERSTATUS_OK == n.DivMod( d, q, r ) && IsTruncatingDivision( rn, rd, q, r );
                if ( nn < 12 && dn < 12 )
                {
                    // the reference's own long division on the magnitudes
                    CRefInt eq;
                    CRefInt er;
                    CRefInt::DivMod( rn, rd, eq, er );
                    eq.negative = rn.negative != rd.negative && !eq.IsZero();
                    er.negative = rn.negative && !er.IsZero();
                    ok = ok && SameAsRef( q, eq ) && SameAsRef( r, er );
                }

                // floored: the remainder takes the sign of the divisor
                CBigInteger fq;
                CBigInteger fr;
                ok = ok && CORE::BIGINTEGERSTATUS_OK == n.DivModFloor( d, fq, fr );
                CRefInt rfr = RefOf( fr );
                ok = ok && 0 == CRefInt::Compare( CRefInt::Add( CRefInt::Multiply( RefOf( fq ), rd ), rfr ), rn );
                ok = ok && CRefInt::CompareMagnitude( rfr, rd ) < 0 && ( rfr.IsZero() || rfr.negative == rd.negative );

                // Euclidean: 0 <= r < |d|, same residue
                CBigInteger m;
                ok = ok && CORE::BIGINTEGERSTATUS_OK == n.Modulo( d, m );
                ok = ok && !m.IsNegative() && m.CompareMagnitude( d ) < 0;
                ok = ok && ( fr.IsZero() ? m.IsZero() : ( m == fr || m == fr + ( d.IsNegative() ? -d : d ) || m == fr - d ) );
                ok = ok && ( n / d ) == q && ( n % d ) == r;

                // built-in divisor fast path
                Int64 smallDivisor = (Int64) rng.Next() >> (UInt32) rng.Below( 63 );
                if ( 0 != smallDivisor )
                {
                    CBigInteger sq;
                    Int64 sr = 0;
                    ok = ok && CORE::BIGINTEGERSTATUS_OK == n.DivMod( smallDivisor, sq, sr );
                    ok = ok && IsTruncatingDivision( rn, CRefInt::FromInt64( smallDivisor ), sq, CBigInteger( sr ) );
                }
                if ( !ok )
                {
                    ++failures;
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CBigInteger division failed at iteration " << i << " sizes " << nn << "/" << dn << " seed " << ( g_testSeedBigInteger + 20 ) );
                    ERRORHERE;
                }
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "CBigInteger: Knuth algorithm D correction and add back vectors, and the exhaustive limb alphabet search" )
        try
        {
            // the four vectors (u / v, 32 bit limbs little endian); a model of the algorithm counts for them
            // ( q-hat corrections in step D3, add backs in step D6 ): ( 1, 1 ), ( 2, 0 ), ( 1, 0 ), ( 0, 1 )
            static const UInt32 u1[] = { 0, 0, 0, 1 };
            static const UInt32 v1[] = { 1, 0, 1 };
            static const UInt32 u2[] = { 0, 0, 1 };
            static const UInt32 v2[] = { 1, 1 };
            static const UInt32 u3[] = { 0, 0, 1 };
            static const UInt32 v3[] = { 1, 0x80000000UL };
            static const UInt32 u4[] = { 0, 0, 0, 1 };
            static const UInt32 v4[] = { 1, 0, 0x80000000UL };
            const UInt32* us[] = { u1, u2, u3, u4 };
            const UInt32* vs[] = { v1, v2, v3, v4 };
            const UInt32 un[] = { 4, 3, 3, 4 };
            const UInt32 vn[] = { 3, 2, 2, 3 };
            for ( UInt32 k=0; k<4; ++k )
            {
                CBigInteger u = FromLimbList( us[ k ], un[ k ] );
                CBigInteger v = FromLimbList( vs[ k ], vn[ k ] );
                CBigInteger q;
                CBigInteger r;
                ASSERT_TRUE( CORE::BIGINTEGERSTATUS_OK == u.DivMod( v, q, r ) );
                ASSERT_TRUE( IsTruncatingDivision( RefOf( u ), RefOf( v ), q, r ) );
                CRefInt eq;
                CRefInt er;
                CRefInt::DivMod( RefOf( u ), RefOf( v ), eq, er );
                ASSERT_TRUE( SameAsRef( q, eq ) && SameAsRef( r, er ) );
            }
            // 2^96 / ( 2^64 + 1 ) = 2^32 - 1 remainder 2^64 - 2^32 + 1
            CBigInteger q;
            CBigInteger r;
            FromLimbList( u1, 4 ).DivMod( FromLimbList( v1, 3 ), q, r );
            ASSERT_TRUE( q == 4294967295ULL && r == CBigInteger( 18446744069414584321ULL ) );

            // every dividend of 3 to 5 limbs and divisor of 2 to 3 limbs over the alphabet (top limb non-zero):
            // q v + r == u and 0 <= r < v for all of them, the reference long division for a deterministic sample
            static const UInt32 alphabet[ 7 ] = { 0, 1, 0x7FFFFFFFUL, 0x80000000UL, 0x80000001UL, 0xFFFFFFFEUL, 0xFFFFFFFFUL };
            GUCEF::vector< CBigInteger > divisors;
            for ( UInt32 len=2; len<=3; ++len )
            {
                UInt32 combinations = 1;
                for ( UInt32 i=0; i<len; ++i )
                    combinations *= 7;
                for ( UInt32 c=0; c<combinations; ++c )
                {
                    UInt32 limbs[ 3 ];
                    UInt32 rest = c;
                    for ( UInt32 i=0; i<len; ++i )
                    {
                        limbs[ i ] = alphabet[ rest % 7 ];
                        rest /= 7;
                    }
                    if ( 0 != limbs[ len-1 ] )
                        divisors.push_back( FromLimbList( limbs, len ) );
                }
            }
            UInt64 checked = 0;
            UInt32 failures = 0;
            CBigInteger check;
            for ( UInt32 len=3; len<=5 && failures < 5; ++len )
            {
                UInt32 combinations = 1;
                for ( UInt32 i=0; i<len; ++i )
                    combinations *= 7;
                for ( UInt32 c=0; c<combinations && failures < 5; ++c )
                {
                    UInt32 limbs[ 5 ];
                    UInt32 rest = c;
                    for ( UInt32 i=0; i<len; ++i )
                    {
                        limbs[ i ] = alphabet[ rest % 7 ];
                        rest /= 7;
                    }
                    if ( 0 == limbs[ len-1 ] )
                        continue;
                    CBigInteger u = FromLimbList( limbs, len );
                    for ( size_t d=0; d<divisors.size(); ++d )
                    {
                        const CBigInteger& v = divisors[ d ];
                        bool ok = CORE::BIGINTEGERSTATUS_OK == u.DivMod( v, q, r ) && !r.IsNegative() && r.CompareMagnitude( v ) < 0;
                        ok = ok && CORE::BIGINTEGERSTATUS_OK == q.Multiply( v, check ) && CORE::BIGINTEGERSTATUS_OK == check.Add( r, check ) && check == u;
                        if ( ok && 0 == ( checked % 4093 ) )
                        {
                            CRefInt eq;
                            CRefInt er;
                            CRefInt::DivMod( RefOf( u ), RefOf( v ), eq, er );
                            ok = SameAsRef( q, eq ) && SameAsRef( r, er );
                        }
                        ++checked;
                        if ( !ok )
                        {
                            ++failures;
                            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CBigInteger Knuth D alphabet case failed: " << u.ToString( 16 ) << " / " << v.ToString( 16 ) );
                            ERRORHERE;
                        }
                    }
                }
            }
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "CBigInteger Knuth D limb alphabet search: " << checked << " divisions checked" );
            ASSERT_TRUE( checked > 4000000 );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "CBigInteger: division edge cases" )
        try
        {
            CSplitMix64 rng( g_testSeedBigInteger + 21 );
            UInt32 failures = 0;
            static const UInt32 smallDivisors[] = { 1, 2, 1000000000UL, 0x80000000UL, 0xFFFFFFFFUL };
            for ( UInt32 i=0; i<200; ++i )
            {
                CBigInteger n = RandomBig( rng, RandomSize( rng, 60 ), true );
                CRefInt rn = RefOf( n );
                for ( UInt32 k=0; k<5; ++k )
                {
                    CBigInteger d( smallDivisors[ k ] );
                    CBigInteger q;
                    CBigInteger r;
                    bool ok = CORE::BIGINTEGERSTATUS_OK == n.DivMod( d, q, r ) && IsTruncatingDivision( rn, RefOf( d ), q, r );
                    // powers of two
                    CBigInteger p = PowerOfTwo( (UInt32) rng.Below( 400 ) );
                    ok = ok && CORE::BIGINTEGERSTATUS_OK == n.DivMod( p, q, r ) && IsTruncatingDivision( rn, RefOf( p ), q, r );
                    if ( !ok )
                    {
                        ++failures;
                        ERRORHERE;
                    }
                }

                // u = v q + r with r = v - 1 and r = 0, quotients with zero limbs inside
                CBigInteger v = RandomBig( rng, 2 + (UInt32) rng.Below( 20 ), false );
                CBigInteger qq = RandomBig( rng, 1 + (UInt32) rng.Below( 20 ), false );
                if ( qq.GetLimbCount() > 2 )
                {
                    for ( UInt32 bit=32; bit<64; ++bit )
                        qq.ClearBit( bit );     // a zero limb inside the quotient
                }
                CBigInteger q;
                CBigInteger r;
                CBigInteger u = v * qq + ( v - 1 );
                ASSERT_TRUE( CORE::BIGINTEGERSTATUS_OK == u.DivMod( v, q, r ) && q == qq && r == v - 1 );
                u = v * qq;
                ASSERT_TRUE( CORE::BIGINTEGERSTATUS_OK == u.DivMod( v, q, r ) && q == qq && r.IsZero() );

                // dividend shorter than, equal to and one less than the divisor
                ASSERT_TRUE( CORE::BIGINTEGERSTATUS_OK == ( v - 1 ).DivMod( v, q, r ) && q.IsZero() && r == v - 1 );
                ASSERT_TRUE( CORE::BIGINTEGERSTATUS_OK == v.DivMod( v, q, r ) && q == 1 && r.IsZero() );
                ASSERT_TRUE( CORE::BIGINTEGERSTATUS_OK == ( -v ).DivMod( v, q, r ) && q == -1 && r.IsZero() );
                CBigInteger shorter = RandomBig( rng, v.GetLimbCount() - 1, true );
                ASSERT_TRUE( CORE::BIGINTEGERSTATUS_OK == shorter.DivMod( v, q, r ) && q.IsZero() && r == shorter );

                // a dividend of n + 1 limbs whose top limb is below and equal to the divisor's top limb
                UInt32 vTop = v.GetLimbs()[ v.GetLimbCount()-1 ];
                for ( UInt32 variant=0; variant<2; ++variant )
                {
                    GUCEF::vector< UInt32 > limbs( v.GetLimbCount() + 1 );
                    for ( size_t l=0; l<limbs.size(); ++l )
                        limbs[ l ] = (UInt32) rng.Next();
                    limbs.back() = 0 == variant ? ( vTop > 1 ? vTop - 1 : 1 ) : vTop;
                    CBigInteger w = FromLimbList( &limbs[ 0 ], (UInt32) limbs.size() );
                    ASSERT_TRUE( CORE::BIGINTEGERSTATUS_OK == w.DivMod( v, q, r ) && IsTruncatingDivision( RefOf( w ), RefOf( v ), q, r ) );
                }
            }

            // divisor with a zero low limb, 2^( 32 k ) - 1 divided by 2^( 32 j ) - 1
            static const UInt32 zeroLow[] = { 0, 1 };
            CBigInteger q;
            CBigInteger r;
            CBigInteger big = Factorial( 60 );
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_OK == big.DivMod( FromLimbList( zeroLow, 2 ), q, r ) && q == ( big >> 32 ) && r == ( big & CBigInteger( 0xFFFFFFFFUL ) ) );
            for ( UInt32 k=2; k<=12; ++k )
            {
                for ( UInt32 j=1; j<=k; ++j )
                {
                    CBigInteger a = PowerOfTwo( 32 * k ) - 1;
                    CBigInteger b = PowerOfTwo( 32 * j ) - 1;
                    ASSERT_TRUE( CORE::BIGINTEGERSTATUS_OK == a.DivMod( b, q, r ) && IsTruncatingDivision( RefOf( a ), RefOf( b ), q, r ) );
                }
            }

            // division by zero, quotient and remainder the same object, aliasing
            CBigInteger x( 1234567 );
            q = 5;
            r = 6;
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_DIVISION_BY_ZERO == x.DivMod( CBigInteger(), q, r ) && q == 5 && r == 6 );
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_DIVISION_BY_ZERO == x.DivModFloor( CBigInteger(), q, r ) && q == 5 && r == 6 );
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_DIVISION_BY_ZERO == x.Modulo( CBigInteger(), q ) && q == 5 );
            Int64 smallRemainder = 9;
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_DIVISION_BY_ZERO == x.DivMod( (Int64) 0, q, smallRemainder ) && 9 == smallRemainder );
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_INVALID_OPERAND == x.DivMod( CBigInteger( 3 ), q, q ) && q == 5 );
            CBigInteger y = Factorial( 40 );
            CBigInteger z = Factorial( 25 );
            CBigInteger expectedQ = y / z;
            CBigInteger expectedR = y % z;
            CBigInteger yy( y );
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_OK == yy.DivMod( z, yy, r ) && yy == expectedQ && r == expectedR );
            yy = y;
            CBigInteger zz( z );
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_OK == yy.DivMod( zz, q, zz ) && q == expectedQ && zz == expectedR );
            bool threw = false;
            try
            {
                CBigInteger bad = x / 0;
            }
            catch ( const GUCEF::invalid_argument& )
            {
                threw = true;
            }
            ASSERT_TRUE( threw );
            threw = false;
            try
            {
                CBigInteger bad = x % CBigInteger();
            }
            catch ( const GUCEF::invalid_argument& )
            {
                threw = true;
            }
            ASSERT_TRUE( threw );

            // INT64_MIN as a built-in divisor and dividend
            const Int64 int64Min = -9223372036854775807LL - 1;
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_OK == CBigInteger( int64Min ).DivMod( int64Min, q, smallRemainder ) && q == 1 && 0 == smallRemainder );
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_OK == ( PowerOfTwo( 100 ) + 5 ).DivMod( int64Min, q, smallRemainder ) && q == -PowerOfTwo( 37 ) && 5 == smallRemainder );
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_OK == CBigInteger( -7 ).DivMod( (Int64) 2, q, smallRemainder ) && q == -3 && -1 == smallRemainder );

            // factorial quotients
            ASSERT_TRUE( Factorial( 100 ) / Factorial( 98 ) == 9900 );
            ASSERT_TRUE( Factorial( 100 ) / ( Factorial( 50 ) * Factorial( 50 ) ) == Big( "100891344545564193334812497256" ) );
            ASSERT_TRUE( ( Factorial( 100 ) % ( Factorial( 50 ) * Factorial( 50 ) ) ).IsZero() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "CBigInteger: division semantics table and rounded division against CDecimal" )
        try
        {
            // n, d, truncating q r, floor q r, Euclidean q r
            static const Int32 table[ 4 ][ 8 ] =
            {
                { -7, 2, -3, -1, -4, 1, -4, 1 },
                { 7, -2, -3, 1, -4, -1, -3, 1 },
                { -7, -2, 3, -1, 3, -1, 4, 1 },
                { 7, 2, 3, 1, 3, 1, 3, 1 }
            };
            for ( UInt32 i=0; i<4; ++i )
            {
                CBigInteger n( table[ i ][ 0 ] );
                CBigInteger d( table[ i ][ 1 ] );
                CBigInteger q;
                CBigInteger r;
                ASSERT_TRUE( CORE::BIGINTEGERSTATUS_OK == n.DivMod( d, q, r ) && q == table[ i ][ 2 ] && r == table[ i ][ 3 ] );
                ASSERT_TRUE( CORE::BIGINTEGERSTATUS_OK == n.DivModFloor( d, q, r ) && q == table[ i ][ 4 ] && r == table[ i ][ 5 ] );
                ASSERT_TRUE( CORE::BIGINTEGERSTATUS_OK == n.Modulo( d, r ) && r == table[ i ][ 7 ] );
                ASSERT_TRUE( n == CBigInteger( table[ i ][ 6 ] ) * d + r );
                ASSERT_TRUE( CORE::BIGINTEGERSTATUS_INEXACT == n.DivideRounded( d, CORE::ROUNDINGMODE_TOWARD_ZERO, q, r ) && q == table[ i ][ 2 ] && r == table[ i ][ 3 ] );
                ASSERT_TRUE( CORE::BIGINTEGERSTATUS_INEXACT == n.DivideRounded( d, CORE::ROUNDINGMODE_FLOOR, q, r ) && q == table[ i ][ 4 ] && r == table[ i ][ 5 ] );
            }

            // the rounding mode table of gucefCORE_RoundingMode.h at scale 0: 2.5, -2.5, 2.6, -0.4 as 25/10, -25/10, 26/10, -4/10
            static const Int32 numerators[ 4 ] = { 25, -25, 26, -4 };
            static const Int32 expected[ 4 ][ 7 ] =
            {
                { 2, 3, 2, 2, 3, 2, 3 },
                { -2, -3, -2, -2, -3, -3, -2 },
                { 3, 3, 3, 2, 3, 2, 3 },
                { 0, 0, 0, 0, -1, -1, 0 }
            };
            for ( UInt32 i=0; i<4; ++i )
            {
                for ( UInt32 mode=0; mode<7; ++mode )
                {
                    CBigInteger q;
                    CBigInteger r;
                    ASSERT_TRUE( CORE::BIGINTEGERSTATUS_INEXACT == CBigInteger( numerators[ i ] ).DivideRounded( 10, (CORE::TRoundingMode) mode, q, r ) );
                    ASSERT_TRUE( q == expected[ i ][ mode ] && r == CBigInteger( numerators[ i ] ) - q * 10 );
                }
                CBigInteger q( 11 );
                CBigInteger r( 12 );
                ASSERT_TRUE( CORE::BIGINTEGERSTATUS_ROUNDING_REQUIRED == CBigInteger( numerators[ i ] ).DivideRounded( 10, CORE::ROUNDINGMODE_UNNECESSARY, q, r ) && q == 11 && r == 12 );
                ASSERT_TRUE( CORE::BIGINTEGERSTATUS_INVALID_OPERAND == CBigInteger( numerators[ i ] ).DivideRounded( 10, CORE::ROUNDINGMODE_INVALID, q, r ) && q == 11 );
            }
            CBigInteger q;
            CBigInteger r;
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_OK == CBigInteger( 30 ).DivideRounded( 10, CORE::ROUNDINGMODE_UNNECESSARY, q, r ) && q == 3 && r.IsZero() );

            // against CDecimal::Divide at scale 0 for seeded Int64 pairs and every mode
            CSplitMix64 rng( g_testSeedBigInteger + 22 );
            UInt32 failures = 0;
            for ( UInt32 i=0; i<20000 && failures < 5; ++i )
            {
                Int64 n = (Int64) rng.Next() >> (UInt32) rng.Below( 63 );
                Int64 d = (Int64) rng.Next() >> (UInt32) rng.Below( 63 );
                if ( 0 == d || n == ( -9223372036854775807LL - 1 ) )
                    continue;
                CORE::CDecimal decimalN;
                CORE::CDecimal::FromInt64( n, 0, decimalN );
                for ( UInt32 mode=0; mode<=(UInt32) CORE::ROUNDINGMODE_UNNECESSARY; ++mode )
                {
                    CORE::CDecimal decimalQ;
                    CORE::TDecimalStatus decimalStatus = decimalN.Divide( d, CORE::CDecimalContext( 0, (CORE::TRoundingMode) mode ), decimalQ );
                    TBigIntegerStatus status = CBigInteger( n ).DivideRounded( d, (CORE::TRoundingMode) mode, q, r );
                    bool ok = decimalStatus == status;
                    if ( ok && !CORE::BigIntegerStatusIsError( status ) )
                        ok = q == decimalQ.GetUnscaledValue() && CBigInteger( n ) == q * d + r;
                    if ( !ok )
                    {
                        ++failures;
                        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CBigInteger DivideRounded differs from CDecimal for " << n << " / " << d << " mode " << mode << " seed " << ( g_testSeedBigInteger + 22 ) );
                        ERRORHERE;
                    }
                }
            }

            // big values against the reference rounding
            for ( UInt32 i=0; i<300 && failures < 5; ++i )
            {
                CBigInteger n = RandomBig( rng, RandomSize( rng, 20 ), true );
                CBigInteger d = RandomBig( rng, 1 + (UInt32) rng.Below( 10 ), true );
                if ( 0 == rng.Below( 4 ) )
                {
                    // exact halves
                    d = d * 2;
                    n = d * RandomBig( rng, 3, true ) + ( d >> 1 ) * ( 0 == rng.Below( 2 ) ? 1 : -1 );
                }
                for ( UInt32 mode=0; mode<(UInt32) CORE::ROUNDINGMODE_UNNECESSARY; ++mode )
                {
                    bool inexact = false;
                    bool roundingRequired = false;
                    CRefInt expectedQ = RoundedQuotient( RefOf( n ), RefOf( d ), (CORE::TRoundingMode) mode, inexact, roundingRequired );
                    // RoundedQuotient takes the sign from the numerator, the divisor is positive there
                    if ( d.IsNegative() )
                        expectedQ = RoundedQuotient( RefOf( -n ), RefOf( -d ), (CORE::TRoundingMode) mode, inexact, roundingRequired );
                    TBigIntegerStatus status = n.DivideRounded( d, (CORE::TRoundingMode) mode, q, r );
                    bool ok = ( inexact ? CORE::BIGINTEGERSTATUS_INEXACT : CORE::BIGINTEGERSTATUS_OK ) == status && SameAsRef( q, expectedQ ) && n == q * d + r;
                    if ( !ok )
                    {
                        ++failures;
                        ERRORHERE;
                    }
                }
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;
}

/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------//
//                                                                         //
//      STEP 4: NUMBER THEORY, ISQRT, BITS                                 //
//                                                                         //
//-------------------------------------------------------------------------*/

static CBigInteger
Fibonacci( UInt32 n )
{
    CBigInteger a;
    CBigInteger b( 1 );
    for ( UInt32 i=0; i<n; ++i )
    {
        CBigInteger next = a + b;
        a.Swap( b );
        b.Swap( next );
    }
    return a;
}

/*-------------------------------------------------------------------------*/

static void
TestNumberTheory( void )
{
    GUCEF_TESTFW_TESTCASE( "CBigInteger: Gcd and Lcm identities and published values" )
        try
        {
            CBigInteger g;
            CBigInteger l;
            CBigInteger x = Big( "-123456789123456789123456789" );
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_OK == CBigInteger().Gcd( CBigInteger(), g ) && g.IsZero() );
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_OK == CBigInteger().Gcd( x, g ) && g == -x );
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_OK == x.Gcd( -x, g ) && g == -x );
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_OK == x.Lcm( CBigInteger(), l ) && l.IsZero() );
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_OK == ( PowerOfTwo( 1260 ) - 1 ).Gcd( PowerOfTwo( 720 ) - 1, g ) && g == PowerOfTwo( 180 ) - 1 );
            CBigInteger f1000 = Fibonacci( 1000 );
            ASSERT_TRUE( 209 == f1000.ToString().Length() );
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_OK == f1000.Gcd( Fibonacci( 600 ), g ) && g == Fibonacci( 200 ) );
            CBigInteger power;
            CBigInteger three;
            CBigInteger( 3 ).Pow( 20, three );
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_OK == Factorial( 30 ).Gcd( PowerOfTwo( 40 ) * three, g ) && g == CBigInteger( 320979616137216ULL ) );
            CBigInteger a = Big( "12345678901234567890" );
            CBigInteger b = Big( "98765432109876543210" );
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_OK == a.Gcd( b, g ) && g == CBigInteger( 900000000090ULL ) );
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_OK == a.Lcm( b, l ) && l == Big( "1354807012498094801236261410" ) );
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_OK == ( -a ).Lcm( b, l ) && l == Big( "1354807012498094801236261410" ) );

            CSplitMix64 rng( g_testSeedBigInteger + 30 );
            UInt32 failures = 0;
            for ( UInt32 i=0; i<2000 && failures < 5; ++i )
            {
                CBigInteger common = RandomBig( rng, (UInt32) rng.Below( 6 ), false );
                CBigInteger p = RandomBig( rng, RandomSize( rng, 40 ), true ) * ( common.IsZero() ? CBigInteger( 1 ) : common );
                CBigInteger q = RandomBig( rng, RandomSize( rng, 40 ), true ) * ( common.IsZero() ? CBigInteger( 1 ) : common );
                bool ok = CORE::BIGINTEGERSTATUS_OK == p.Gcd( q, g ) && CORE::BIGINTEGERSTATUS_OK == p.Lcm( q, l );
                CBigInteger product = p * q;
                ok = ok && !g.IsNegative() && !l.IsNegative() && g * l == ( product.IsNegative() ? -product : product );
                if ( ok && !g.IsZero() )
                {
                    // g divides both and the cofactors are coprime
                    CBigInteger cofactorGcd;
                    ok = ( p % g ).IsZero() && ( q % g ).IsZero();
                    ok = ok && CORE::BIGINTEGERSTATUS_OK == ( p / g ).Gcd( q / g, cofactorGcd ) && ( cofactorGcd.IsOne() || ( p.IsZero() || q.IsZero() ) );
                }
                // the gcd also through ExtendedGcd, with Bezout
                CBigInteger eg;
                CBigInteger ex;
                CBigInteger ey;
                ok = ok && CORE::BIGINTEGERSTATUS_OK == p.ExtendedGcd( q, eg, ex, ey ) && eg == g && p * ex + q * ey == g;
                if ( !ok )
                {
                    ++failures;
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CBigInteger gcd failed at iteration " << i << " seed " << ( g_testSeedBigInteger + 30 ) );
                    ERRORHERE;
                }
            }

            // Bezout for 10,000 random pairs including negatives and zero
            for ( UInt32 i=0; i<10000 && failures < 5; ++i )
            {
                CBigInteger p = RandomBig( rng, (UInt32) rng.Below( 6 ), true );
                CBigInteger q = RandomBig( rng, (UInt32) rng.Below( 6 ), true );
                CBigInteger eg;
                CBigInteger ex;
                CBigInteger ey;
                bool ok = CORE::BIGINTEGERSTATUS_OK == p.ExtendedGcd( q, eg, ex, ey ) && p * ex + q * ey == eg && !eg.IsNegative();
                CBigInteger g2;
                ok = ok && CORE::BIGINTEGERSTATUS_OK == p.Gcd( q, g2 ) && g2 == eg;
                if ( !ok )
                {
                    ++failures;
                    ERRORHERE;
                }
            }
            CBigInteger eg;
            CBigInteger ex;
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_INVALID_OPERAND == a.ExtendedGcd( b, eg, eg, ex ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "CBigInteger: ModInverse, ModPow, Lucas-Lehmer and Fermat" )
        try
        {
            CSplitMix64 rng( g_testSeedBigInteger + 31 );
            for ( UInt32 i=0; i<500; ++i )
            {
                CBigInteger m = RandomBig( rng, 1 + (UInt32) rng.Below( 8 ), false );
                if ( m.IsOne() )
                    continue;
                CBigInteger a = RandomBig( rng, 1 + (UInt32) rng.Below( 10 ), true );
                CBigInteger g;
                a.Gcd( m, g );
                CBigInteger inverse( 77 );
                TBigIntegerStatus status = a.ModInverse( m, inverse );
                if ( g.IsOne() )
                {
                    CBigInteger check;
                    ASSERT_TRUE( CORE::BIGINTEGERSTATUS_OK == status && !inverse.IsNegative() && inverse < m );
                    ASSERT_TRUE( CORE::BIGINTEGERSTATUS_OK == ( a * inverse ).Modulo( m, check ) && check.IsOne() );
                }
                else
                {
                    ASSERT_TRUE( CORE::BIGINTEGERSTATUS_INVALID_OPERAND == status && inverse == 77 );
                }
            }
            CBigInteger r;
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_DIVISION_BY_ZERO == CBigInteger( 3 ).ModInverse( CBigInteger(), r ) );
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_OK == CBigInteger( 3 ).ModInverse( 7, r ) && r == 5 );
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_OK == CBigInteger( -3 ).ModInverse( 7, r ) && r == 2 );

            // Lucas-Lehmer: s0 = 4, s = s^2 - 2 mod M, p - 2 steps
            CBigInteger mersenne = PowerOfTwo( 521 ) - 1;
            CBigInteger s( 4 );
            for ( UInt32 i=0; i<519; ++i )
            {
                CBigInteger::ThrowOnError( s.Square( s ) );
                CBigInteger::ThrowOnError( s.Subtract( 2, s ) );
                CBigInteger::ThrowOnError( s.Modulo( mersenne, s ) );
            }
            ASSERT_TRUE( s.IsZero() );
            CBigInteger m11 = PowerOfTwo( 11 ) - 1;
            s = 4;
            for ( UInt32 i=0; i<9; ++i )
            {
                CBigInteger::ThrowOnError( s.Square( s ) );
                CBigInteger::ThrowOnError( s.Subtract( 2, s ) );
                CBigInteger::ThrowOnError( s.Modulo( m11, s ) );
            }
            ASSERT_TRUE( s == 1736 );

            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_OK == CBigInteger( 2 ).ModPow( mersenne - 1, mersenne, r ) && r.IsOne() );
            // 561 = 3 * 11 * 17 passes the Fermat test although it is composite
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_OK == CBigInteger( 2 ).ModPow( 560, 561, r ) && r.IsOne() );
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_OK == CBigInteger( 5 ).ModPow( 0, 7, r ) && r.IsOne() );
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_OK == CBigInteger( 5 ).ModPow( 3, 1, r ) && r.IsZero() );
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_OK == CBigInteger( -2 ).ModPow( 3, 7, r ) && r == 6 );
            r = 9;
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_INVALID_OPERAND == CBigInteger( 2 ).ModPow( -1, 7, r ) && r == 9 );
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_INVALID_OPERAND == CBigInteger( 2 ).ModPow( 3, -7, r ) && r == 9 );
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_DIVISION_BY_ZERO == CBigInteger( 2 ).ModPow( 3, 0, r ) && r == 9 );

            // against Pow followed by Modulo
            for ( UInt32 i=0; i<200; ++i )
            {
                CBigInteger base = RandomBig( rng, 1 + (UInt32) rng.Below( 4 ), true );
                UInt32 e = (UInt32) rng.Below( 200 );
                CBigInteger m = RandomBig( rng, 1 + (UInt32) rng.Below( 5 ), false );
                CBigInteger power;
                CBigInteger expected;
                CBigInteger::ThrowOnError( base.Pow( e, power ) );
                CBigInteger::ThrowOnError( power.Modulo( m, expected ) );
                ASSERT_TRUE( CORE::BIGINTEGERSTATUS_OK == base.ModPow( e, m, r ) && r == expected );
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "CBigInteger: integer square root" )
        try
        {
            CBigInteger r;
            CBigInteger ten40;
            CBigInteger( 10 ).Pow( 40, ten40 );
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_OK == ten40.ISqrt( r ) && TextIs( r, "100000000000000000000" ) );
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_OK == ( ten40 - 1 ).ISqrt( r ) && TextIs( r, "99999999999999999999" ) );
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_OK == PowerOfTwo( 1042 ).ISqrt( r ) && r == PowerOfTwo( 521 ) );
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_OK == CBigInteger().ISqrt( r ) && r.IsZero() );
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_OK == CBigInteger( 1 ).ISqrt( r ) && r == 1 );
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_OK == CBigInteger( 3 ).ISqrt( r ) && r == 1 );
            r = 5;
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_INVALID_OPERAND == CBigInteger( -1 ).ISqrt( r ) && r == 5 );

            CSplitMix64 rng( g_testSeedBigInteger + 32 );
            UInt32 failures = 0;
            for ( UInt32 i=0; i<10000 && failures < 5; ++i )
            {
                CBigInteger root = RandomBig( rng, (UInt32) rng.Below( 12 ), false );
                CBigInteger square = root * root;
                CBigInteger next = ( root + 1 ) * ( root + 1 );
                CBigInteger candidates[ 3 ] = { square, square + RandomBig( rng, 1, false ) % ( next - square ), next - 1 };
                bool ok = true;
                for ( UInt32 k=0; k<3 && ok; ++k )
                    ok = CORE::BIGINTEGERSTATUS_OK == candidates[ k ].ISqrt( r ) && r == root;
                // random n: r^2 <= n < ( r + 1 )^2
                CBigInteger n = RandomBig( rng, RandomSize( rng, 40 ), false );
                ok = ok && CORE::BIGINTEGERSTATUS_OK == n.ISqrt( r ) && r * r <= n && n < ( r + 1 ) * ( r + 1 );
                if ( !ok )
                {
                    ++failures;
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CBigInteger ISqrt failed at iteration " << i << " seed " << ( g_testSeedBigInteger + 32 ) );
                    ERRORHERE;
                }
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;
}

/*-------------------------------------------------------------------------*/

/** Bit i of x in two's complement, from the arithmetic definition: floor( x / 2^i ) mod 2 */
static bool
ArithmeticBit( const CBigInteger& x, UInt32 i )
{
    CBigInteger q;
    CBigInteger r;
    CBigInteger::ThrowOnError( x.DivModFloor( PowerOfTwo( i ), q, r ) );
    CBigInteger m;
    CBigInteger::ThrowOnError( q.Modulo( 2, m ) );
    return m.IsOne();
}

/*-------------------------------------------------------------------------*/

static void
TestBits( void )
{
    GUCEF_TESTFW_TESTCASE( "CBigInteger: two's complement bit operations" )
        try
        {
            ASSERT_TRUE( ( CBigInteger( -5 ) & CBigInteger( 3 ) ) == 3 );
            ASSERT_TRUE( ( CBigInteger( -5 ) | CBigInteger( 3 ) ) == -5 );
            ASSERT_TRUE( ( CBigInteger( -5 ) ^ CBigInteger( 3 ) ) == -8 );
            ASSERT_TRUE( ~CBigInteger( 0 ) == -1 && ~CBigInteger( -1 ) == 0 && ~CBigInteger( 5 ) == -6 );
            ASSERT_TRUE( CBigInteger( -1 ).TestBit( 1000000 ) && !CBigInteger( 1 ).TestBit( 1000000 ) );
            UInt32 count = 0;
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_OK == CBigInteger( 0xF0F0 ).PopCount( count ) && 8 == count );
            count = 3;
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_INVALID_OPERAND == CBigInteger( -1 ).PopCount( count ) && 3 == count );
            ASSERT_TRUE( 0 == CBigInteger().GetLowestSetBit() && 4 == CBigInteger( -48 ).GetLowestSetBit() );
            ASSERT_TRUE( 0 == CBigInteger().GetBitLength() && 1 == CBigInteger( 1 ).GetBitLength() && 3 == CBigInteger( -5 ).GetBitLength() );

            CSplitMix64 rng( g_testSeedBigInteger + 40 );
            UInt32 failures = 0;
            for ( UInt32 i=0; i<2000 && failures < 5; ++i )
            {
                // sizes on both sides of the inline buffer, limb boundary bit positions
                CBigInteger a = RandomBig( rng, (UInt32) rng.Below( 14 ), true );
                CBigInteger b = RandomBig( rng, (UInt32) rng.Below( 14 ), true );
                CBigInteger andAB = a & b;
                CBigInteger orAB = a | b;
                CBigInteger xorAB = a ^ b;
                bool ok = IsCanonical( andAB ) && IsCanonical( orAB ) && IsCanonical( xorAB );
                ok = ok && andAB + orAB == a + b;
                ok = ok && xorAB == orAB - andAB;
                ok = ok && ~a == -a - 1 && ~~a == a;
                ok = ok && ( a & CBigInteger( -1 ) ) == a && ( a | CBigInteger() ) == a && ( a ^ a ).IsZero();
                static const UInt32 positions[] = { 0, 1, 31, 32, 33, 63, 64, 65, 255, 256, 257, 300, 600 };
                UInt32 index = positions[ rng.Below( sizeof( positions ) / sizeof( positions[ 0 ] ) ) ];
                ok = ok && a.TestBit( index ) == ArithmeticBit( a, index );
                ok = ok && andAB.TestBit( index ) == ( a.TestBit( index ) && b.TestBit( index ) );
                ok = ok && orAB.TestBit( index ) == ( a.TestBit( index ) || b.TestBit( index ) );
                ok = ok && xorAB.TestBit( index ) == ( a.TestBit( index ) != b.TestBit( index ) );

                // SetBit / ClearBit / FlipBit against x | 2^i, x & ~2^i, x ^ 2^i
                CBigInteger mask = PowerOfTwo( index );
                CBigInteger x( a );
                ok = ok && CORE::BIGINTEGERSTATUS_OK == x.SetBit( index ) && x == ( a | mask ) && x.TestBit( index ) && IsCanonical( x );
                x = a;
                ok = ok && CORE::BIGINTEGERSTATUS_OK == x.ClearBit( index ) && x == ( a & ~mask ) && !x.TestBit( index ) && IsCanonical( x );
                x = a;
                ok = ok && CORE::BIGINTEGERSTATUS_OK == x.FlipBit( index ) && x == ( a ^ mask ) && IsCanonical( x );
                // the set bit has the arithmetic meaning: x | 2^i == x + 2^i when the bit was clear
                if ( !a.TestBit( index ) )
                    ok = ok && ( a | mask ) == a + mask;
                if ( !ok )
                {
                    ++failures;
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CBigInteger bit operations failed at iteration " << i << " seed " << ( g_testSeedBigInteger + 40 ) );
                    ERRORHERE;
                }
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;
}

/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------//
//                                                                         //
//      STEP 5: CONVERSIONS                                                //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The exact value of a finite double as numerator / denominator (denominator a power of two) */
static void
DoubleToFraction( Float64 value, CBigInteger& numerator, CBigInteger& denominator )
{
    int exponent = 0;
    Float64 fraction = frexp( value < 0 ? -value : value, &exponent );
    UInt64 mantissa = (UInt64) ldexp( fraction, 53 );
    Int32 e = exponent - 53;
    numerator = CBigInteger( mantissa );
    denominator = CBigInteger( 1 );
    if ( e >= 0 )
        numerator <<= (UInt32) e;
    else
        denominator <<= (UInt32) -e;
    if ( value < 0 )
        numerator = -numerator;
}

/*-------------------------------------------------------------------------*/

/** Sign of n / d - value, exact (d > 0) */
static Int32
CompareRatioToDouble( const CBigInteger& n, const CBigInteger& d, Float64 value )
{
    CBigInteger vn;
    CBigInteger vd;
    DoubleToFraction( value, vn, vd );
    return ( n * vd ).Compare( vn * d );
}

/*-------------------------------------------------------------------------*/

/** Sign of n / d - ( a + b ) / 2, exact (d > 0) */
static Int32
CompareRatioToMidpoint( const CBigInteger& n, const CBigInteger& d, Float64 a, Float64 b )
{
    CBigInteger an;
    CBigInteger ad;
    CBigInteger bn;
    CBigInteger bd;
    DoubleToFraction( a, an, ad );
    DoubleToFraction( b, bn, bd );
    // ( an bd + bn ad ) / ( 2 ad bd )
    CBigInteger mn = an * bd + bn * ad;
    CBigInteger md = ad * bd * 2;
    return ( n * md ).Compare( mn * d );
}

/*-------------------------------------------------------------------------*/

/**
 *  Checks that 'value' is n / d (d > 0, n > 0) correctly rounded with 'mode' to the double format,
 *  from the exact position of n / d relative to the neighbours of 'value' and the midpoints.
 */
static bool
IsCorrectlyRoundedDouble( const CBigInteger& n, const CBigInteger& d, CORE::TRoundingMode mode, Float64 value )
{
    Float64 below = nextafter( value, 0.0 );
    Float64 above = nextafter( value, HUGE_VAL );
    switch ( mode )
    {
        case CORE::ROUNDINGMODE_FLOOR:
        case CORE::ROUNDINGMODE_TOWARD_ZERO:
            return CompareRatioToDouble( n, d, value ) >= 0 && ( above == HUGE_VAL || CompareRatioToDouble( n, d, above ) < 0 );
        case CORE::ROUNDINGMODE_CEILING:
        case CORE::ROUNDINGMODE_AWAY_FROM_ZERO:
            return CompareRatioToDouble( n, d, value ) <= 0 && ( 0.0 == value ? true : ( value == below || CompareRatioToDouble( n, d, below ) > 0 ) );
        case CORE::ROUNDINGMODE_HALF_EVEN:
        {
            if ( 0.0 == value )
                return CompareRatioToMidpoint( n, d, 0.0, above ) <= 0;
            Int32 low = CompareRatioToMidpoint( n, d, below, value );
            Int32 high = above == HUGE_VAL ? -1 : CompareRatioToMidpoint( n, d, value, above );
            int exponent = 0;
            bool even = 0 == ( (UInt64) ldexp( frexp( value, &exponent ), 53 ) & 1 ) || value < DBL_MIN;
            if ( value < DBL_MIN )
            {
                // subnormal: the significand is value / 2^-1074
                even = 0 == ( (UInt64) ldexp( value, 1074 ) & 1 );
            }
            if ( low < 0 || high > 0 )
                return false;
            if ( 0 == low || 0 == high )
                return even;
            return true;
        }
        default:
            return false;
    }
}

/*-------------------------------------------------------------------------*/

static void
TestFloatConversions( void )
{
    GUCEF_TESTFW_TESTCASE( "CBigInteger: ToFloat64 and ToFloat32 round once with every mode" )
        try
        {
            CBigInteger x = PowerOfTwo( 53 ) + 1;
            Float64 two53 = 9007199254740992.0;
            Float64 out = 0.0;
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_INEXACT == x.ToFloat64( CORE::ROUNDINGMODE_HALF_EVEN, out ) && two53 == out );
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_INEXACT == x.ToFloat64( CORE::ROUNDINGMODE_HALF_UP, out ) && two53 + 2 == out );
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_INEXACT == x.ToFloat64( CORE::ROUNDINGMODE_FLOOR, out ) && two53 == out );
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_INEXACT == x.ToFloat64( CORE::ROUNDINGMODE_CEILING, out ) && two53 + 2 == out );
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_INEXACT == ( -x ).ToFloat64( CORE::ROUNDINGMODE_HALF_EVEN, out ) && -two53 == out );
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_INEXACT == ( -x ).ToFloat64( CORE::ROUNDINGMODE_HALF_UP, out ) && -two53 - 2 == out );
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_INEXACT == ( -x ).ToFloat64( CORE::ROUNDINGMODE_FLOOR, out ) && -two53 - 2 == out );
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_INEXACT == ( -x ).ToFloat64( CORE::ROUNDINGMODE_CEILING, out ) && -two53 == out );
            out = 1.0;
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_ROUNDING_REQUIRED == x.ToFloat64( CORE::ROUNDINGMODE_UNNECESSARY, out ) && 1.0 == out );
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_OK == PowerOfTwo( 1000 ).ToFloat64( CORE::ROUNDINGMODE_UNNECESSARY, out ) && ldexp( 1.0, 1000 ) == out );

            // the edge of the range: 2^1024 - 2^970 is the midpoint between the largest double and 2^1024
            CBigInteger midpoint = PowerOfTwo( 1024 ) - PowerOfTwo( 970 );
            out = 1.0;
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_OVERFLOW == midpoint.ToFloat64( CORE::ROUNDINGMODE_HALF_EVEN, out ) && 1.0 == out );
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_INEXACT == ( midpoint - 1 ).ToFloat64( CORE::ROUNDINGMODE_HALF_EVEN, out ) && DBL_MAX == out );
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_INEXACT == PowerOfTwo( 1024 ).ToFloat64( CORE::ROUNDINGMODE_TOWARD_ZERO, out ) && DBL_MAX == out );
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_INEXACT == PowerOfTwo( 5000 ).ToFloat64( CORE::ROUNDINGMODE_FLOOR, out ) && DBL_MAX == out );
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_INEXACT == ( -PowerOfTwo( 5000 ) ).ToFloat64( CORE::ROUNDINGMODE_CEILING, out ) && -DBL_MAX == out );
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_OVERFLOW == PowerOfTwo( 1024 ).ToFloat64( CORE::ROUNDINGMODE_UNNECESSARY, out ) );
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_OVERFLOW == ( -PowerOfTwo( 5000 ) ).ToFloat64( CORE::ROUNDINGMODE_FLOOR, out ) );
            ASSERT_TRUE( HUGE_VAL == PowerOfTwo( 1024 ).ToFloat64() && -HUGE_VAL == ( -PowerOfTwo( 1024 ) ).ToFloat64() );
            ASSERT_TRUE( DBL_MAX == ( midpoint - 1 ).ToFloat64() );
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_INVALID_OPERAND == x.ToFloat64( CORE::ROUNDINGMODE_INVALID, out ) );

            // float: no double rounding through Float64. ( 2^24 + 1 ) 2^40 + 1 is just above a float midpoint, its
            // double is the midpoint itself, which ties to even
            CBigInteger y = ( PowerOfTwo( 24 ) + 1 ) * PowerOfTwo( 40 ) + 1;
            Float32 f = 0.0f;
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_INEXACT == y.ToFloat32( CORE::ROUNDINGMODE_HALF_EVEN, f ) );
            ASSERT_TRUE( (Float64) f == ldexp( 16777218.0, 40 ) );
            ASSERT_TRUE( (Float32) y.ToFloat64() == (Float32) ldexp( 16777216.0, 40 ) );     // the double rounded result differs
            ASSERT_TRUE( y.ToFloat32() == f );
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_OVERFLOW == PowerOfTwo( 128 ).ToFloat32( CORE::ROUNDINGMODE_HALF_EVEN, f ) );
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_INEXACT == PowerOfTwo( 128 ).ToFloat32( CORE::ROUNDINGMODE_TOWARD_ZERO, f ) && FLT_MAX == f );
            ASSERT_TRUE( HUGE_VAL == (Float64) PowerOfTwo( 128 ).ToFloat32() );

            // random values: the C library's correctly rounded strtod / strtof of the decimal text is the reference
            // for nearest even, the exact neighbour test for the directed modes
            CSplitMix64 rng( g_testSeedBigInteger + 50 );
            UInt32 failures = 0;
            for ( UInt32 i=0; i<3000 && failures < 5; ++i )
            {
                CBigInteger v = RandomBig( rng, 1 + (UInt32) rng.Below( 34 ), true );
                if ( 0 == rng.Below( 4 ) )
                    v = v >> (UInt32) rng.Below( 32 * v.GetLimbCount() );
                if ( v.IsZero() )
                    continue;
                CORE::CString text = v.ToString();
                Float64 nearest = 0.0;
                Float32 nearest32 = 0.0f;
                TBigIntegerStatus status = v.ToFloat64( CORE::ROUNDINGMODE_HALF_EVEN, nearest );
                Float64 reference = strtod( text.C_String(), GUCEF_NULL );
                bool ok = CORE::BIGINTEGERSTATUS_OVERFLOW == status ? fabs( reference ) == HUGE_VAL : ( !CORE::BigIntegerStatusIsError( status ) && nearest == reference );
                status = v.ToFloat32( CORE::ROUNDINGMODE_HALF_EVEN, nearest32 );
                Float32 reference32 = strtof( text.C_String(), GUCEF_NULL );
                ok = ok && ( CORE::BIGINTEGERSTATUS_OVERFLOW == status ? fabs( (Float64) reference32 ) == HUGE_VAL : ( !CORE::BigIntegerStatusIsError( status ) && nearest32 == reference32 ) );
                CBigInteger magnitude = v.IsNegative() ? -v : v;
                for ( UInt32 mode=0; mode<=(UInt32) CORE::ROUNDINGMODE_CEILING && ok; ++mode )
                {
                    Float64 r = 0.0;
                    status = magnitude.ToFloat64( (CORE::TRoundingMode) mode, r );
                    if ( CORE::BIGINTEGERSTATUS_OVERFLOW == status )
                        continue;
                    if ( CORE::ROUNDINGMODE_HALF_UP == mode || CORE::ROUNDINGMODE_HALF_DOWN == mode )
                        continue;
                    ok = IsCorrectlyRoundedDouble( magnitude, CBigInteger( 1 ), (CORE::TRoundingMode) mode, r );
                    ok = ok && ( CORE::BIGINTEGERSTATUS_OK == status ) == ( 0 == CompareRatioToDouble( magnitude, CBigInteger( 1 ), r ) );
                }
                if ( !ok )
                {
                    ++failures;
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CBigInteger float conversion failed at iteration " << i << " for " << text << " seed " << ( g_testSeedBigInteger + 50 ) );
                    ERRORHERE;
                }
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "CBigInteger: FromFloat64 rounds the exact binary value" )
        try
        {
            static const Float64 inputs[ 5 ] = { 0.5, 1.5, 2.5, -2.5, -0.4 };
            static const Int32 expected[ 5 ][ 7 ] =
            {
                { 0, 1, 0, 0, 1, 0, 1 },
                { 2, 2, 1, 1, 2, 1, 2 },
                { 2, 3, 2, 2, 3, 2, 3 },
                { -2, -3, -2, -2, -3, -3, -2 },
                { 0, 0, 0, 0, -1, -1, 0 }
            };
            CBigInteger r;
            for ( UInt32 i=0; i<5; ++i )
            {
                for ( UInt32 mode=0; mode<7; ++mode )
                    ASSERT_TRUE( CORE::BIGINTEGERSTATUS_INEXACT == CBigInteger::FromFloat64( inputs[ i ], (CORE::TRoundingMode) mode, r ) && r == expected[ i ][ mode ] && IsCanonical( r ) );
                r = 42;
                ASSERT_TRUE( CORE::BIGINTEGERSTATUS_ROUNDING_REQUIRED == CBigInteger::FromFloat64( inputs[ i ], CORE::ROUNDINGMODE_UNNECESSARY, r ) && r == 42 );
            }
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_OK == CBigInteger::FromFloat64( 1e22, CORE::ROUNDINGMODE_UNNECESSARY, r ) && TextIs( r, "10000000000000000000000" ) );
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_OK == CBigInteger::FromFloat64( 1e23, CORE::ROUNDINGMODE_UNNECESSARY, r ) && TextIs( r, "99999999999999991611392" ) );
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_OK == CBigInteger::FromFloat64( ldexp( 1.0, 63 ), CORE::ROUNDINGMODE_UNNECESSARY, r ) && r == PowerOfTwo( 63 ) );
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_OK == CBigInteger::FromFloat64( -ldexp( 1.0, 100 ), CORE::ROUNDINGMODE_UNNECESSARY, r ) && r == -PowerOfTwo( 100 ) );
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_OK == CBigInteger::FromFloat64( DBL_MAX, CORE::ROUNDINGMODE_UNNECESSARY, r ) && r == PowerOfTwo( 1024 ) - PowerOfTwo( 971 ) );
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_OK == CBigInteger::FromFloat64( -0.0, CORE::ROUNDINGMODE_UNNECESSARY, r ) && r.IsZero() && !r.IsNegative() );
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_INEXACT == CBigInteger::FromFloat64( 4.9e-324, CORE::ROUNDINGMODE_CEILING, r ) && r == 1 );
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_INEXACT == CBigInteger::FromFloat64( -4.9e-324, CORE::ROUNDINGMODE_HALF_UP, r ) && r.IsZero() && !r.IsNegative() );
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_INEXACT == CBigInteger::FromFloat32( 2.5f, CORE::ROUNDINGMODE_HALF_EVEN, r ) && r == 2 );
            Float64 zero = 0.0;
            r = 7;
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_INVALID_OPERAND == CBigInteger::FromFloat64( zero / zero, CORE::ROUNDINGMODE_HALF_EVEN, r ) && r == 7 );
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_INVALID_OPERAND == CBigInteger::FromFloat64( HUGE_VAL, CORE::ROUNDINGMODE_HALF_EVEN, r ) && r == 7 );
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_INVALID_OPERAND == CBigInteger::FromFloat64( -HUGE_VAL, CORE::ROUNDINGMODE_HALF_EVEN, r ) && r == 7 );

            // integral doubles round trip
            CSplitMix64 rng( g_testSeedBigInteger + 51 );
            for ( UInt32 i=0; i<10000; ++i )
            {
                UInt64 bits = rng.Next() & 0x7FEFFFFFFFFFFFFFULL;
                Float64 value = 0.0;
                memcpy( &value, &bits, sizeof( value ) );
                value = floor( value );
                if ( 0 != ( rng.Next() & 1 ) )
                    value = -value;
                bool ok = CORE::BIGINTEGERSTATUS_OK == CBigInteger::FromFloat64( value, CORE::ROUNDINGMODE_UNNECESSARY, r ) && r.ToFloat64() == value;
                Float64 back = 0.0;
                ok = ok && CORE::BIGINTEGERSTATUS_OK == r.ToFloat64( CORE::ROUNDINGMODE_UNNECESSARY, back ) && back == value;
                if ( !ok )
                {
                    ERRORHERE;
                    break;
                }
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "CBigInteger: RatioToFloat64 and RatioToFloat32 are correctly rounded, subnormals and overflow included" )
        try
        {
            Float64 out = 0.0;
            CBigInteger n( 3706778661852469502LL );
            CBigInteger d( 239877 );
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_INEXACT == CBigInteger::RatioToFloat64( n, d, CORE::ROUNDINGMODE_HALF_EVEN, out ) );
            ASSERT_TRUE( strtod( "15452830666768.676", GUCEF_NULL ) == out );
            // the reason for the helper: dividing the separately rounded doubles gives the neighbour
            ASSERT_TRUE( strtod( "15452830666768.674", GUCEF_NULL ) == (Float64) 3706778661852469502LL / 239877.0 );

            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_OK == CBigInteger::RatioToFloat64( 1, PowerOfTwo( 1074 ), CORE::ROUNDINGMODE_UNNECESSARY, out ) && ldexp( 1.0, -1074 ) == out );
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_INEXACT == CBigInteger::RatioToFloat64( 1, PowerOfTwo( 1075 ), CORE::ROUNDINGMODE_HALF_EVEN, out ) && 0.0 == out );
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_INEXACT == CBigInteger::RatioToFloat64( 3, PowerOfTwo( 1076 ), CORE::ROUNDINGMODE_HALF_EVEN, out ) && ldexp( 1.0, -1074 ) == out );
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_INEXACT == CBigInteger::RatioToFloat64( 1, PowerOfTwo( 3000 ), CORE::ROUNDINGMODE_CEILING, out ) && ldexp( 1.0, -1074 ) == out );
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_INEXACT == CBigInteger::RatioToFloat64( -1, PowerOfTwo( 3000 ), CORE::ROUNDINGMODE_FLOOR, out ) && -ldexp( 1.0, -1074 ) == out );
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_INEXACT == CBigInteger::RatioToFloat64( 1, 3, CORE::ROUNDINGMODE_HALF_EVEN, out ) && 1.0 / 3.0 == out );
            out = 5.0;
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_OVERFLOW == CBigInteger::RatioToFloat64( PowerOfTwo( 1025 ), 2, CORE::ROUNDINGMODE_HALF_EVEN, out ) && 5.0 == out );
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_DIVISION_BY_ZERO == CBigInteger::RatioToFloat64( 1, 0, CORE::ROUNDINGMODE_HALF_EVEN, out ) && 5.0 == out );
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_OK == CBigInteger::RatioToFloat64( 0, -7, CORE::ROUNDINGMODE_HALF_EVEN, out ) && 0.0 == out );
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_OK == CBigInteger::RatioToFloat64( -6, -4, CORE::ROUNDINGMODE_UNNECESSARY, out ) && 1.5 == out );
            Float32 f = 0.0f;
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_INEXACT == CBigInteger::RatioToFloat32( 1, 3, CORE::ROUNDINGMODE_HALF_EVEN, f ) && 1.0f / 3.0f == f );
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_OK == CBigInteger::RatioToFloat32( 1, PowerOfTwo( 149 ), CORE::ROUNDINGMODE_UNNECESSARY, f ) && (Float32) ldexp( 1.0, -149 ) == f );
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_OVERFLOW == CBigInteger::RatioToFloat32( PowerOfTwo( 129 ), 2, CORE::ROUNDINGMODE_HALF_EVEN, f ) );

            CSplitMix64 rng( g_testSeedBigInteger + 52 );
            UInt32 failures = 0;
            static const CORE::TRoundingMode modes[ 3 ] = { CORE::ROUNDINGMODE_HALF_EVEN, CORE::ROUNDINGMODE_FLOOR, CORE::ROUNDINGMODE_CEILING };
            for ( UInt32 i=0; i<2000 && failures < 5; ++i )
            {
                CBigInteger a = RandomBig( rng, 1 + (UInt32) rng.Below( 6 ), false );
                CBigInteger b = RandomBig( rng, 1 + (UInt32) rng.Below( 6 ), false );
                UInt32 kind = (UInt32) rng.Below( 4 );
                if ( 1 == kind )
                    b <<= 900 + (UInt32) rng.Below( 300 );     // around and below the subnormal range
                else
                if ( 2 == kind )
                    a <<= 800 + (UInt32) rng.Below( 300 );     // around the overflow threshold
                for ( UInt32 m=0; m<3; ++m )
                {
                    Float64 r = 0.0;
                    TBigIntegerStatus status = CBigInteger::RatioToFloat64( a, b, modes[ m ], r );
                    if ( CORE::BIGINTEGERSTATUS_OVERFLOW == status )
                    {
                        // only when the ratio is beyond the largest double (rounded up for nearest and ceiling)
                        if ( CompareRatioToDouble( a, b, DBL_MAX ) <= 0 )
                        {
                            ++failures;
                            ERRORHERE;
                        }
                        continue;
                    }
                    bool ok = !CORE::BigIntegerStatusIsError( status ) && IsCorrectlyRoundedDouble( a, b, modes[ m ], r );
                    ok = ok && ( CORE::BIGINTEGERSTATUS_OK == status ) == ( 0 == CompareRatioToDouble( a, b, r ) );
                    // the negated ratio mirrors
                    Float64 negative = 0.0;
                    CORE::TRoundingMode mirrored = CORE::ROUNDINGMODE_FLOOR == modes[ m ] ? CORE::ROUNDINGMODE_CEILING : ( CORE::ROUNDINGMODE_CEILING == modes[ m ] ? CORE::ROUNDINGMODE_FLOOR : modes[ m ] );
                    ok = ok && status == CBigInteger::RatioToFloat64( -a, b, mirrored, negative ) && negative == -r;
                    if ( !ok )
                    {
                        ++failures;
                        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CBigInteger RatioToFloat64 failed at iteration " << i << " mode " << modes[ m ] << " seed " << ( g_testSeedBigInteger + 52 ) );
                        ERRORHERE;
                    }
                }
                // float: against the double of the exact ratio when that cannot double round (both exact in double)
                Float32 r32 = 0.0f;
                if ( 0 == kind && a.GetBitLength() <= 53 && b.GetBitLength() <= 24 )
                {
                    // the ratio of two exact doubles rounded to float through long division has a single rounding
                    TBigIntegerStatus status = CBigInteger::RatioToFloat32( a, b, CORE::ROUNDINGMODE_HALF_EVEN, r32 );
                    Float64 r64 = 0.0;
                    CBigInteger::RatioToFloat64( a, b, CORE::ROUNDINGMODE_TOWARD_ZERO, r64 );
                    ASSERT_TRUE( !CORE::BigIntegerStatusIsError( status ) );
                    ASSERT_TRUE( fabs( (Float64) r32 - r64 ) <= ldexp( fabs( r64 ), -24 ) );
                }
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;
}

/*-------------------------------------------------------------------------*/

static void
TestDecimalAndWideConversions( void )
{
    GUCEF_TESTFW_TESTCASE( "CBigInteger: decimal conversions and the CDecimal::Divide cross check" )
        try
        {
            CORE::CDecimal decimal;
            ASSERT_TRUE( CORE::DECIMALSTATUS_INEXACT == CBigInteger::RatioToDecimal( 1, 3, CORE::CDecimalContext( 18, CORE::ROUNDINGMODE_HALF_EVEN ), decimal ) );
            ASSERT_TRUE( decimal.ToString() == "0.333333333333333333" );
            ASSERT_TRUE( CORE::DECIMALSTATUS_INEXACT == CBigInteger::RatioToDecimal( 2, 3, CORE::CDecimalContext( 18, CORE::ROUNDINGMODE_HALF_EVEN ), decimal ) );
            ASSERT_TRUE( decimal.ToString() == "0.666666666666666667" );
            ASSERT_TRUE( CORE::DECIMALSTATUS_OK == CBigInteger::RatioToDecimal( Factorial( 30 ), Factorial( 28 ), CORE::CDecimalContext( 2, CORE::ROUNDINGMODE_UNNECESSARY ), decimal ) && decimal.ToString() == "870.00" );
            ASSERT_TRUE( CORE::DECIMALSTATUS_OVERFLOW == CBigInteger::RatioToDecimal( PowerOfTwo( 63 ), 1, CORE::CDecimalContext( 0, CORE::ROUNDINGMODE_HALF_EVEN ), decimal ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_DIVISION_BY_ZERO == CBigInteger::RatioToDecimal( 1, 0, CORE::CDecimalContext( 0, CORE::ROUNDINGMODE_HALF_EVEN ), decimal ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_INVALID_SCALE == CBigInteger::RatioToDecimal( 1, 3, CORE::CDecimalContext( 19, CORE::ROUNDINGMODE_HALF_EVEN ), decimal ) );

            CBigInteger r;
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_INEXACT == CBigInteger::FromDecimal( CORE::CDecimal::Parse( "2.5" ), CORE::ROUNDINGMODE_HALF_EVEN, r ) && r == 2 );
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_INEXACT == CBigInteger::FromDecimal( CORE::CDecimal::Parse( "-2.5" ), CORE::ROUNDINGMODE_HALF_UP, r ) && r == -3 );
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_OK == CBigInteger::FromDecimal( CORE::CDecimal::Parse( "-9223372036854775807" ), CORE::ROUNDINGMODE_UNNECESSARY, r ) && r == CBigInteger( -9223372036854775807LL ) );
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_ROUNDING_REQUIRED == CBigInteger::FromDecimal( CORE::CDecimal::Parse( "0.000000000000000001" ), CORE::ROUNDINGMODE_UNNECESSARY, r ) );
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_INEXACT == CBigInteger::FromDecimal( CORE::CDecimal::Parse( "-0.000000000000000001" ), CORE::ROUNDINGMODE_FLOOR, r ) && r == -1 );

            CBigInteger tenTo18;
            CBigInteger( 10 ).Pow( 18, tenTo18 );
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_OK == tenTo18.ToDecimal( 0, decimal ) && decimal.ToString() == "1000000000000000000" );
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_OVERFLOW == PowerOfTwo( 63 ).ToDecimal( 0, decimal ) );
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_OK == CBigInteger( -9223372036854775807LL ).ToDecimal( 0, decimal ) );
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_OK == CBigInteger( -5 ).ToDecimal( 18, decimal ) && decimal.ToString() == "-5.000000000000000000" );
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_OVERFLOW == CBigInteger( 10 ).ToDecimal( 18, decimal ) );
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_INVALID_SCALE == CBigInteger( 1 ).ToDecimal( 19, decimal ) );

            // the cross check: for seeded Int64 pairs, every mode and a random scale, RatioToDecimal equals CDecimal::Divide, status included
            CSplitMix64 rng( g_testSeedBigInteger + 60 );
            UInt32 failures = 0;
            for ( UInt32 i=0; i<100000 && failures < 5; ++i )
            {
                Int64 n = (Int64) rng.Next() >> (UInt32) rng.Below( 64 );
                Int64 d = (Int64) rng.Next() >> (UInt32) rng.Below( 64 );
                if ( n == ( -9223372036854775807LL - 1 ) )
                    continue;   // not a CDecimal unscaled value
                UInt8 scale = (UInt8) rng.Below( 20 );     // 19 is invalid
                CORE::CDecimal decimalN;
                CORE::CDecimal::FromInt64( n, 0, decimalN );
                for ( UInt32 mode=0; mode<=(UInt32) CORE::ROUNDINGMODE_UNNECESSARY; ++mode )
                {
                    CORE::CDecimalContext ctx( scale, (CORE::TRoundingMode) mode );
                    CORE::CDecimal expected;
                    CORE::CDecimal actual;
                    CORE::TDecimalStatus expectedStatus = decimalN.Divide( d, ctx, expected );
                    CORE::TDecimalStatus actualStatus = CBigInteger::RatioToDecimal( n, d, ctx, actual );
                    bool ok = expectedStatus == actualStatus;
                    if ( ok && !CORE::DecimalStatusIsError( actualStatus ) )
                        ok = expected.IsIdentical( actual );
                    if ( !ok )
                    {
                        ++failures;
                        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CBigInteger RatioToDecimal differs from CDecimal::Divide for " << n << " / " << d << " scale " << (UInt32) scale << " mode " << mode << " seed " << ( g_testSeedBigInteger + 60 ) );
                        ERRORHERE;
                    }
                }
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "CBigInteger: CUInt128, CInt128, bytes, two's complement and CBigUnsigned conversions" )
        try
        {
            CORE::CUInt128 u128( 0x0123456789ABCDEFULL, 0xFEDCBA9876543210ULL );
            CBigInteger v;
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_OK == CBigInteger::FromCUInt128( u128, v ) && v == ( CBigInteger( 0x0123456789ABCDEFULL ) << 64 ) + CBigInteger( 0xFEDCBA9876543210ULL ) );
            CORE::CUInt128 u128Back;
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_OK == v.ToCUInt128( u128Back ) && u128Back == u128 );
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_OK == ( PowerOfTwo( 128 ) - 1 ).ToCUInt128( u128Back ) && ~0ULL == u128Back.high && ~0ULL == u128Back.low );
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_OVERFLOW == PowerOfTwo( 128 ).ToCUInt128( u128Back ) && ~0ULL == u128Back.high );
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_OVERFLOW == CBigInteger( -1 ).ToCUInt128( u128Back ) );

            CORE::CInt128 i128;
            CORE::CInt128 minimum( 0x8000000000000000ULL, 0 );
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_OK == CBigInteger::FromCInt128( minimum, v ) && v == -PowerOfTwo( 127 ) );
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_OK == v.ToCInt128( i128 ) && i128 == minimum );
            ASSERT_TRUE( v.FitsCInt128() && !( v - 1 ).FitsCInt128() && !PowerOfTwo( 127 ).FitsCInt128() && ( PowerOfTwo( 127 ) - 1 ).FitsCInt128() );
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_OVERFLOW == PowerOfTwo( 127 ).ToCInt128( i128 ) && i128 == minimum );
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_OK == CBigInteger( -5 ).ToCInt128( i128 ) && i128 == CORE::CInt128( -5 ) );
            CSplitMix64 rng( g_testSeedBigInteger + 61 );
            for ( UInt32 i=0; i<1000; ++i )
            {
                CORE::CInt128 value( rng.Next() >> (UInt32) rng.Below( 64 ), rng.Next() );
                CBigInteger big;
                ASSERT_TRUE( CORE::BIGINTEGERSTATUS_OK == CBigInteger::FromCInt128( value, big ) && CORE::BIGINTEGERSTATUS_OK == big.ToCInt128( i128 ) && i128 == value );
                ASSERT_TRUE( big.ToString() == RefText( RefOf( big ) ) && ( big.IsNegative() == value.IsNegative() ) );
            }

            // big endian bytes, sign passed separately
            static const UInt8 bytes[] = { 0x00, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09 };
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_OK == CBigInteger::FromBigEndianBytes( bytes, sizeof( bytes ), true, v ) && v == -Big( "18591708106338011145" ) );
            ASSERT_TRUE( 9 == v.GetByteLength() );
            UInt8 out[ 16 ];
            memset( out, 0xAA, sizeof( out ) );
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_OVERFLOW == v.ToBigEndianBytes( out, 8 ) && 0xAA == out[ 0 ] );
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_OK == v.ToBigEndianBytes( out, 11 ) && 0 == memcmp( out, bytes, 11 ) );
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_OK == CBigInteger::FromBigEndianBytes( GUCEF_NULL, 0, true, v ) && v.IsZero() && !v.IsNegative() );

            // two's complement
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_OK == CBigInteger( -1 ).ToTwosComplementBigEndian( out, 3 ) && 0xFF == out[ 0 ] && 0xFF == out[ 1 ] && 0xFF == out[ 2 ] );
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_OK == CBigInteger( -9223372036854775807LL - 1 ).ToTwosComplementBigEndian( out, 8 ) && 0x80 == out[ 0 ] && 0 == out[ 7 ] );
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_OVERFLOW == PowerOfTwo( 63 ).ToTwosComplementBigEndian( out, 8 ) );
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_OK == PowerOfTwo( 63 ).ToTwosComplementBigEndian( out, 9 ) && 0 == out[ 0 ] && 0x80 == out[ 1 ] );
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_OVERFLOW == ( -PowerOfTwo( 63 ) - 1 ).ToTwosComplementBigEndian( out, 8 ) );
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_OK == CBigInteger( 127 ).ToTwosComplementBigEndian( out, 1 ) && CORE::BIGINTEGERSTATUS_OVERFLOW == CBigInteger( 128 ).ToTwosComplementBigEndian( out, 1 ) );
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_OK == CBigInteger( -128 ).ToTwosComplementBigEndian( out, 1 ) && 0x80 == out[ 0 ] && CORE::BIGINTEGERSTATUS_OVERFLOW == CBigInteger( -129 ).ToTwosComplementBigEndian( out, 1 ) );
            for ( UInt32 i=0; i<2000; ++i )
            {
                CBigInteger value = RandomBig( rng, (UInt32) rng.Below( 12 ), true );
                UInt32 size = value.GetByteLength() + 1 + (UInt32) rng.Below( 3 );
                GUCEF::vector< UInt8 > buffer( size );
                CBigInteger back;
                bool ok = CORE::BIGINTEGERSTATUS_OK == value.ToTwosComplementBigEndian( &buffer[ 0 ], size );
                ok = ok && CORE::BIGINTEGERSTATUS_OK == CBigInteger::FromTwosComplementBigEndian( &buffer[ 0 ], size, back ) && back == value;
                // the pattern is value mod 2^( 8 size )
                CBigInteger pattern;
                CBigInteger::FromBigEndianBytes( &buffer[ 0 ], size, false, pattern );
                CBigInteger residue;
                ok = ok && CORE::BIGINTEGERSTATUS_OK == value.Modulo( PowerOfTwo( 8 * size ), residue ) && residue == pattern;
                ok = ok && CORE::BIGINTEGERSTATUS_OK == value.ToBigEndianBytes( &buffer[ 0 ], size );
                ok = ok && CORE::BIGINTEGERSTATUS_OK == CBigInteger::FromBigEndianBytes( &buffer[ 0 ], size, value.IsNegative(), back ) && back == value;
                if ( !ok )
                {
                    ERRORHERE;
                    break;
                }
            }

            // CBigUnsigned: equal bytes both ways, and ModPow agrees with the Montgomery ModExp of the RSA code
            for ( UInt32 i=0; i<50; ++i )
            {
                GUCEF::vector< UInt8 > raw( 1 + (size_t) rng.Below( 256 ) );
                for ( size_t b=0; b<raw.size(); ++b )
                    raw[ b ] = (UInt8) rng.Next();
                CORE::CBigUnsigned unsignedValue;
                unsignedValue.SetFromBigEndian( &raw[ 0 ], (UInt32) raw.size() );
                CBigInteger big;
                ASSERT_TRUE( CORE::BIGINTEGERSTATUS_OK == CBigInteger::FromBigUnsigned( unsignedValue, big ) );
                CBigInteger fromBytes;
                CBigInteger::FromBigEndianBytes( &raw[ 0 ], (UInt32) raw.size(), false, fromBytes );
                ASSERT_TRUE( big == fromBytes );
                CORE::CBigUnsigned back;
                ASSERT_TRUE( CORE::BIGINTEGERSTATUS_OK == big.ToBigUnsigned( back ) && 0 == back.Compare( unsignedValue ) );

                CBigInteger modulus = RandomBig( rng, 2 + (UInt32) rng.Below( 64 ), false );
                modulus.SetBit( 0 );
                CBigInteger base;
                big.Modulo( modulus, base );
                CBigInteger exponent = RandomBig( rng, 1 + (UInt32) rng.Below( 4 ), false );
                CORE::CBigUnsigned modulusU;
                CORE::CBigUnsigned baseU;
                CORE::CBigUnsigned exponentU;
                CORE::CBigUnsigned resultU;
                modulus.ToBigUnsigned( modulusU );
                base.ToBigUnsigned( baseU );
                exponent.ToBigUnsigned( exponentU );
                CORE::CMontgomeryModulus montgomery;
                ASSERT_TRUE( montgomery.Initialize( modulusU ) && montgomery.ModExp( baseU, exponentU, resultU ) );
                CBigInteger expected;
                CBigInteger::FromBigUnsigned( resultU, expected );
                CBigInteger actual;
                ASSERT_TRUE( CORE::BIGINTEGERSTATUS_OK == big.ModPow( exponent, modulus, actual ) && actual == expected );
            }
            CORE::CBigUnsigned unchanged;
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_INVALID_OPERAND == CBigInteger( -3 ).ToBigUnsigned( unchanged ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;
}

/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------//
//                                                                         //
//      STEP 6: DIVIDE AND CONQUER PARSING, DECIMAL FLOAT, THREADS, HASH   //
//                                                                         //
//-------------------------------------------------------------------------*/

/** Horner by chunks of 9 digits through the public arithmetic: the plain path the divide and conquer parser must equal */
static CBigInteger
HornerDecimal( const char* digits, UInt32 count )
{
    CBigInteger value;
    UInt32 position = 0;
    UInt32 first = count % 9;
    UInt32 take = 0 == first ? 9 : first;
    while ( position < count )
    {
        UInt32 chunk = 0;
        UInt32 scale = 1;
        for ( UInt32 i=0; i<take; ++i )
        {
            chunk = chunk * 10 + (UInt32) ( digits[ position + i ] - '0' );
            scale *= 10;
        }
        CBigInteger::ThrowOnError( value.Multiply( scale, value ) );
        CBigInteger::ThrowOnError( value.Add( chunk, value ) );
        position += take;
        take = 9;
    }
    return value;
}

/*-------------------------------------------------------------------------*/

static void
TestDivideAndConquerParsing( void )
{
    GUCEF_TESTFW_TESTCASE( "CBigInteger: divide and conquer parsing equals Horner and round trips, up to 100,000 digits" )
        try
        {
            CSplitMix64 rng( g_testSeedBigInteger + 70 );
            static const UInt32 lengths[] = { 1, 9, 10, 500, 576, 577, 600, 700, 1200, 5000, 20000, 100000 };
            for ( UInt32 k=0; k<sizeof( lengths ) / sizeof( lengths[ 0 ] ); ++k )
            {
                UInt32 length = lengths[ k ];
                GUCEF::vector< char > digits( length + 2 );
                digits[ 0 ] = '-';
                for ( UInt32 i=0; i<length; ++i )
                    digits[ i + 1 ] = (char) ( '0' + rng.Below( 10 ) );
                if ( '0' == digits[ 1 ] )
                    digits[ 1 ] = '7';
                digits[ length + 1 ] = '\0';

                CBigInteger parsed;
                ASSERT_TRUE( CORE::BIGINTEGERSTATUS_OK == CBigInteger::FromChars( &digits[ 0 ], length + 1, 10, parsed ) );
                CBigInteger horner = HornerDecimal( &digits[ 1 ], length );
                ASSERT_TRUE( parsed == -horner );
                // the text conversion back is the repeated division, an independent algorithm
                ASSERT_TRUE( 0 == strcmp( parsed.ToString().C_String(), &digits[ 0 ] ) );
                if ( length <= 1200 )
                    ASSERT_TRUE( RefText( RefOf( parsed ) ) == CORE::CString( &digits[ 0 ] ) );

                // other bases through the same path
                if ( length <= 5000 )
                {
                    for ( UInt32 base=3; base<=36; base+=11 )
                    {
                        CBigInteger back;
                        ASSERT_TRUE( CORE::BIGINTEGERSTATUS_OK == CBigInteger::FromString( parsed.ToString( base ), base, back ) && back == parsed );
                    }
                }
            }
            // leading zeros in front of a long number
            CORE::CString padded = CORE::CString( "0000000000" ) + Factorial( 400 ).ToString();
            CBigInteger value;
            ASSERT_TRUE( CORE::BIGINTEGERSTATUS_OK == CBigInteger::FromString( padded, 10, value ) && value == Factorial( 400 ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;
}

/*-------------------------------------------------------------------------*/

static void
TestDecimalToFloat32( void )
{
    GUCEF_TESTFW_TESTCASE( "CDecimal::ToFloat32 is correctly rounded (reference: CBigInteger::RatioToFloat32)" )
        try
        {
            // ( 2^24 + 1 ) 2^29 + 1: the double is a float midpoint and ties to even, the exact value rounds up
            CORE::CDecimal d;
            ASSERT_TRUE( CORE::DECIMALSTATUS_OK == CORE::CDecimal::FromInt64( 9007199791611905LL, 0, d ) );
            ASSERT_TRUE( (Float64) d.ToFloat32() == ldexp( 16777218.0, 29 ) );
            ASSERT_TRUE( (Float64) (Float32) d.ToFloat64() == ldexp( 16777216.0, 29 ) );
            ASSERT_TRUE( 0.0f == CORE::CDecimal().ToFloat32() );
            ASSERT_TRUE( -1.5f == CORE::CDecimal::Parse( "-1.50" ).ToFloat32() );
            ASSERT_TRUE( 0.1f == CORE::CDecimal::Parse( "0.1" ).ToFloat32() );

            CSplitMix64 rng( g_testSeedBigInteger + 80 );
            UInt32 failures = 0;
            UInt32 doubleRoundingCases = 0;
            static const UInt64 pow10[ 19 ] = { 1ULL, 10ULL, 100ULL, 1000ULL, 10000ULL, 100000ULL, 1000000ULL, 10000000ULL, 100000000ULL, 1000000000ULL,
                10000000000ULL, 100000000000ULL, 1000000000000ULL, 10000000000000ULL, 100000000000000ULL, 1000000000000000ULL,
                10000000000000000ULL, 100000000000000000ULL, 1000000000000000000ULL };
            for ( UInt32 i=0; i<100000 && failures < 5; ++i )
            {
                Int64 unscaled = (Int64) ( rng.Next() >> ( 1 + (UInt32) rng.Below( 63 ) ) );
                if ( 0 != ( rng.Next() & 1 ) )
                    unscaled = -unscaled;
                if ( 0 == rng.Below( 8 ) )
                {
                    // near a float midpoint: m 2^k + 2^( k - 1 ) + small
                    UInt32 k = 1 + (UInt32) rng.Below( 38 );
                    UInt64 m = ( 1ULL << 23 ) + rng.Below( 1ULL << 23 );
                    unscaled = (Int64) ( ( m << k ) + ( 1ULL << ( k - 1 ) ) ) + (Int64) rng.Below( 3 ) - 1;
                }
                UInt8 scale = (UInt8) rng.Below( 19 );
                CORE::CDecimal value;
                if ( CORE::DECIMALSTATUS_OK != CORE::CDecimal::FromUnscaled( unscaled, scale, value ) )
                    continue;
                Float32 expected = 0.0f;
                CBigInteger::RatioToFloat32( CBigInteger( unscaled ), CBigInteger( pow10[ scale ] ), CORE::ROUNDINGMODE_HALF_EVEN, expected );
                Float32 actual = value.ToFloat32();
                if ( (Float32) value.ToFloat64() != expected )
                    ++doubleRoundingCases;
                if ( actual != expected )
                {
                    ++failures;
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CDecimal::ToFloat32 differs for " << value.ToString() << " seed " << ( g_testSeedBigInteger + 80 ) );
                    ERRORHERE;
                }
            }
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "CDecimal::ToFloat32: " << doubleRoundingCases << " seeded values where the float of ToFloat64 differs" );
            ASSERT_TRUE( doubleRoundingCases > 0 );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;
}

/*-------------------------------------------------------------------------*/

namespace {

/** One reader thread of the thread safety test: const calls on a shared value, results kept per thread */
struct SBigIntegerReader
{
    const CBigInteger* shared;
    CORE::CString expectedText;
    UInt64 expectedHash;
    CBigInteger expectedSquare;
    bool ok;
};

UInt32 GUCEF_CALLSPEC_PREFIX
BigIntegerReaderMain( void* data ) GUCEF_CALLSPEC_SUFFIX
{
    SBigIntegerReader* reader = static_cast< SBigIntegerReader* >( data );
    bool ok = true;
    for ( UInt32 i=0; i<20 && ok; ++i )
    {
        CBigInteger square;
        ok = reader->shared->ToString() == reader->expectedText;
        ok = ok && 0 == reader->shared->Compare( *reader->shared );
        ok = ok && reader->shared->GetHash() == reader->expectedHash;
        ok = ok && CORE::BIGINTEGERSTATUS_OK == reader->shared->Multiply( *reader->shared, square ) && square == reader->expectedSquare;
    }
    reader->ok = ok;
    return 0;
}

}; /* anonymous namespace */

/*-------------------------------------------------------------------------*/

static void
TestThreadsHashAndSerialization( void )
{
    GUCEF_TESTFW_TESTCASE( "CBigInteger: concurrent const calls on one shared value" )
        try
        {
            CBigInteger shared;
            CBigInteger( 3 ).Pow( 6310, shared );      // about 10^4 bits
            SBigIntegerReader readers[ 4 ];
            MT::SThreadData* threads[ 4 ];
            for ( UInt32 i=0; i<4; ++i )
            {
                readers[ i ].shared = &shared;
                readers[ i ].expectedText = shared.ToString();
                readers[ i ].expectedHash = shared.GetHash();
                readers[ i ].expectedSquare = shared * shared;
                readers[ i ].ok = false;
                threads[ i ] = MT::ThreadDataReserve();
                ASSERT_TRUE( GUCEF_NULL != threads[ i ] && 0 != MT::ThreadCreate( threads[ i ], &BigIntegerReaderMain, &readers[ i ] ) );
            }
            for ( UInt32 i=0; i<4; ++i )
            {
                if ( GUCEF_NULL != threads[ i ] )
                {
                    MT::ThreadWait( threads[ i ], 120000 );
                    MT::ThreadDataCleanup( threads[ i ] );
                }
                ASSERT_TRUE( readers[ i ].ok );
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "CBigInteger: equal values have equal hashes, decimal text serialization through a DataNode" )
        try
        {
            CSplitMix64 rng( g_testSeedBigInteger + 90 );
            for ( UInt32 i=0; i<500; ++i )
            {
                CBigInteger a = RandomBig( rng, (UInt32) rng.Below( 20 ), true );
                CBigInteger b = RandomBig( rng, (UInt32) rng.Below( 20 ), true );
                // the same value reached by different operation sequences, inline and heap storage
                CBigInteger viaSum = ( a + b ) - b;
                CBigInteger viaProduct = a.IsZero() ? CBigInteger() : ( a * b * 3 ) / ( b * 3 + ( b.IsZero() ? 1 : 0 ) );
                if ( b.IsZero() )
                    viaProduct = a;
                CBigInteger viaText = Big( a.ToString().C_String() );
                CBigInteger viaHeap = ( a << 300 ) >> 300;
                ASSERT_TRUE( viaSum == a && viaProduct == a && viaText == a && viaHeap == a );
                ASSERT_TRUE( viaSum.GetHash() == a.GetHash() && viaProduct.GetHash() == a.GetHash() && viaText.GetHash() == a.GetHash() && viaHeap.GetHash() == a.GetHash() );

                // a DataNode attribute carries the decimal text
                CORE::CDataNode node;
                node.SetName( "number" );
                ASSERT_TRUE( node.SetAttribute( "value", a.ToString() ) );
                CBigInteger back;
                ASSERT_TRUE( CORE::BIGINTEGERSTATUS_OK == CBigInteger::FromString( node.GetAttributeValue( "value" ).AsString(), 10, back ) && back == a );
            }
            ASSERT_TRUE( CBigInteger( 1 ).GetHash() != CBigInteger( -1 ).GetHash() );
            ASSERT_TRUE( CBigInteger().GetHash() != CBigInteger( 1 ).GetHash() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;
}

/*-------------------------------------------------------------------------*/

/** Seconds of one call of 'Run' averaged over enough repetitions to take about 50 ms */
template < class TOperation >
static Float64
TimeOperation( TOperation& operation )
{
    UInt32 repetitions = 1;
    for ( ;; )
    {
        CORE::CStopwatch stopwatch;
        stopwatch.Start();
        for ( UInt32 i=0; i<repetitions; ++i )
            operation.Run();
        Float64 seconds = stopwatch.GetElapsedMicroseconds() / 1e6;
        if ( seconds >= 0.05 || repetitions >= ( 1UL << 24 ) )
            return seconds / repetitions;
        repetitions *= 2;
    }
}

/*-------------------------------------------------------------------------*/

struct SMultiplyBenchmark
{
    CBigInteger a;
    CBigInteger b;
    CBigInteger r;
    void Run( void ) { a.Multiply( b, r ); }
};

struct SDivideBenchmark
{
    CBigInteger a;
    CBigInteger b;
    CBigInteger q;
    CBigInteger r;
    void Run( void ) { a.DivMod( b, q, r ); }
};

struct SGcdBenchmark
{
    CBigInteger a;
    CBigInteger b;
    CBigInteger r;
    void Run( void ) { a.Gcd( b, r ); }
};

struct SToStringBenchmark
{
    CBigInteger a;
    void Run( void ) { a.ToString(); }
};

struct SFromStringBenchmark
{
    CORE::CString text;
    CBigInteger r;
    void Run( void ) { CBigInteger::FromString( text, 10, r ); }
};

/*-------------------------------------------------------------------------*/

static void
TestBenchmarks( void )
{
    GUCEF_TESTFW_TESTCASE( "CBigInteger: benchmarks (opt-in: GUCEF_CORE_TESTAPP_BIGINTEGER_BENCHMARK)" )
        GUCEF_TESTFW_REQUIRE_ENV( enabled, "GUCEF_CORE_TESTAPP_BIGINTEGER_BENCHMARK" );
        try
        {
            CSplitMix64 rng( g_testSeedBigInteger + 100 );
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "CBigInteger benchmark: Karatsuba threshold " << g_karatsubaThreshold << " limbs" );
            static const UInt32 sizes[] = { 16, 24, 32, 48, 64, 128, 256, 1024, 4096 };
            for ( UInt32 i=0; i<sizeof( sizes ) / sizeof( sizes[ 0 ] ); ++i )
            {
                SMultiplyBenchmark multiply;
                multiply.a = RandomBig( rng, sizes[ i ], false );
                multiply.b = RandomBig( rng, sizes[ i ], false );
                SDivideBenchmark divide;
                divide.a = RandomBig( rng, 2 * sizes[ i ], false );
                divide.b = RandomBig( rng, sizes[ i ], false );
                Float64 multiplySeconds = TimeOperation( multiply );
                Float64 divideSeconds = TimeOperation( divide );
                Float64 gcdSeconds = 0.0;
                if ( sizes[ i ] <= 256 )
                {
                    SGcdBenchmark gcd;
                    gcd.a = multiply.a;
                    gcd.b = multiply.b;
                    gcdSeconds = TimeOperation( gcd );
                }
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "CBigInteger benchmark: " << sizes[ i ] << " limbs: multiply " << multiplySeconds * 1e6 << " us, divide " << 2 * sizes[ i ] << "/" << sizes[ i ] << " " << divideSeconds * 1e6 << " us, gcd " << gcdSeconds * 1e6 << " us" );
            }
            static const UInt32 digits[] = { 100, 1000, 10000, 100000 };
            for ( UInt32 i=0; i<sizeof( digits ) / sizeof( digits[ 0 ] ); ++i )
            {
                GUCEF::vector< char > text( digits[ i ] + 1 );
                for ( UInt32 k=0; k<digits[ i ]; ++k )
                    text[ k ] = (char) ( '1' + rng.Below( 9 ) );
                text[ digits[ i ] ] = '\0';
                SFromStringBenchmark parse;
                parse.text = CORE::CString( &text[ 0 ] );
                SToStringBenchmark print;
                CBigInteger::FromString( parse.text, 10, print.a );
                Float64 parseSeconds = TimeOperation( parse );
                Float64 printSeconds = TimeOperation( print );
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "CBigInteger benchmark: " << digits[ i ] << " digits: FromString " << parseSeconds * 1e6 << " us, ToString " << printSeconds * 1e6 << " us" );
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;
}

/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformBigIntegerTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "CBigInteger" );
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING CBigInteger TESTS" );

    TestRepresentation();
    TestAddSubtractCompare();
    TestShifts();
    TestText();
    TestNoAllocationForSmallValues();
    TestMultiplication();
    TestDivision();
    TestNumberTheory();
    TestBits();
    TestFloatConversions();
    TestDecimalAndWideConversions();
    TestDivideAndConquerParsing();
    TestDecimalToFloat32();
    TestThreadsHashAndSerialization();
    TestBenchmarks();
}

/*-------------------------------------------------------------------------*/
