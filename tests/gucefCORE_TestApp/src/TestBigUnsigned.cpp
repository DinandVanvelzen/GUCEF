/*
 *  gucefCORE_TestApp: GUCEF test application for the CORE module
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
#include <stdlib.h>

#ifndef GUCEF_CORE_CBIGUNSIGNED_H
#include "gucefCORE_CBigUnsigned.h"
#define GUCEF_CORE_CBIGUNSIGNED_H
#endif /* GUCEF_CORE_CBIGUNSIGNED_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestRsaVectors.h"
#include "TestBigUnsigned.h"

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

/*-------------------------------------------------------------------------*/

/** Hexadecimal text of any digit count to a number */
static CORE::CBigUnsigned
FromHex( const char* hex )
{GUCEF_TRACE;

    const UInt32 digitCount = (UInt32) strlen( hex );
    CORE::CDynamicBuffer bytes;
    UInt32 index = 0;
    if ( 0 != digitCount % 2 )
    {
        // an odd number of digits: the first byte has one digit
        char single[ 2 ] = { hex[ 0 ], '\0' };
        UInt8 value = (UInt8) strtoul( single, GUCEF_NULL, 16 );
        bytes.Append( &value, 1, true );
        index = 1;
    }
    for ( ; index+1<digitCount; index+=2 )
    {
        char pair[ 3 ] = { hex[ index ], hex[ index+1 ], '\0' };
        UInt8 value = (UInt8) strtoul( pair, GUCEF_NULL, 16 );
        bytes.Append( &value, 1, true );
    }
    CORE::CBigUnsigned number;
    number.SetFromBigEndian( bytes.GetConstBufferPtr(), bytes.GetDataSize() );
    return number;
}

/*-------------------------------------------------------------------------*/

static CORE::CBigUnsigned
FromUInt( UInt32 value )
{GUCEF_TRACE;

    UInt8 bytes[ 4 ] = { (UInt8) ( value >> 24 ), (UInt8) ( value >> 16 ), (UInt8) ( value >> 8 ), (UInt8) value };
    CORE::CBigUnsigned number;
    number.SetFromBigEndian( bytes, 4 );
    return number;
}

/*-------------------------------------------------------------------------*/

static bool
ModExpIs( const CORE::CBigUnsigned& base, const CORE::CBigUnsigned& exponent, const CORE::CBigUnsigned& modulus, const CORE::CBigUnsigned& expected )
{GUCEF_TRACE;

    CORE::CMontgomeryModulus context;
    CORE::CBigUnsigned result;
    return context.Initialize( modulus ) && context.ModExp( base, exponent, result ) && 0 == result.Compare( expected );
}

/*-------------------------------------------------------------------------*/

void
PerformBigUnsignedTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING BigUnsigned TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "BigUnsigned" );

    GUCEF_TESTFW_TESTCASE( "Test: A number converts from and to big endian bytes, leading zeros are not part of the value" )
        try
        {
            CORE::CBigUnsigned zero;
            ASSERT_TRUE( zero.IsZero() && 0 == zero.GetBitLength() && 0 == zero.GetByteLength() && 0 == zero.GetLimbCount() );
            ASSERT_TRUE( GUCEF_NULL == zero.GetLimbs() );
            ASSERT_TRUE( zero.SetFromBigEndian( GUCEF_NULL, 0 ) && zero.IsZero() );
            ASSERT_FALSE( zero.SetFromBigEndian( GUCEF_NULL, 4 ) );
            const UInt8 zeros[ 5 ] = { 0, 0, 0, 0, 0 };
            ASSERT_TRUE( zero.SetFromBigEndian( zeros, 5 ) && zero.IsZero() );

            const UInt8 bytes[ 8 ] = { 0x00, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06 };
            CORE::CBigUnsigned number;
            ASSERT_TRUE( number.SetFromBigEndian( bytes, 8 ) );
            ASSERT_TRUE( 2 == number.GetLimbCount() );
            ASSERT_TRUE( 41 == number.GetBitLength() );
            ASSERT_TRUE( 6 == number.GetByteLength() );
            ASSERT_FALSE( number.IsOdd() );
            ASSERT_TRUE( number.GetLimbs()[ 0 ] == 0x03040506 && number.GetLimbs()[ 1 ] == 0x0102 );
            ASSERT_TRUE( number.GetBit( 1 ) && number.GetBit( 2 ) && !number.GetBit( 0 ) && number.GetBit( 40 ) && !number.GetBit( 41 ) && !number.GetBit( 5000 ) );

            UInt64 value = 0;
            ASSERT_TRUE( number.TryGetUInt64( value ) && value == 0x010203040506ULL );

            UInt8 out[ 10 ];
            memset( out, 0xAA, sizeof( out ) );
            ASSERT_TRUE( number.ToBigEndian( out, 10 ) );
            const UInt8 expected[ 10 ] = { 0, 0, 0, 0, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06 };
            ASSERT_TRUE( 0 == memcmp( out, expected, 10 ) );
            ASSERT_TRUE( number.ToBigEndian( out, 6 ) && 0 == memcmp( out, expected + 4, 6 ) );
            ASSERT_FALSE( number.ToBigEndian( out, 5 ) );

            // a value of more than 64 bits does not fit
            CORE::CBigUnsigned big = FromHex( "010000000000000000" );
            ASSERT_TRUE( 65 == big.GetBitLength() && !big.TryGetUInt64( value ) );

            // comparing
            CORE::CBigUnsigned a = FromHex( "0100" );
            CORE::CBigUnsigned b = FromHex( "00ff" );
            ASSERT_TRUE( a.Compare( b ) > 0 && b.Compare( a ) < 0 && 0 == a.Compare( FromHex( "100" ) ) );
            ASSERT_TRUE( big.Compare( a ) > 0 && zero.Compare( a ) < 0 && 0 == zero.Compare( CORE::CBigUnsigned() ) );
            ASSERT_TRUE( FromHex( "0100000001" ).Compare( FromHex( "0100000002" ) ) < 0 );

            number.SetZero();
            ASSERT_TRUE( number.IsZero() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Modular exponentiation: textbook examples and the edges" )
        try
        {
            // 4^13 mod 497 = 445
            ASSERT_TRUE( ModExpIs( FromUInt( 4 ), FromUInt( 13 ), FromUInt( 497 ), FromUInt( 445 ) ) );

            // RSA with p = 61, q = 53: 65^17 mod 3233 = 2790 and 2790^2753 mod 3233 = 65
            ASSERT_TRUE( ModExpIs( FromUInt( 65 ), FromUInt( 17 ), FromUInt( 3233 ), FromUInt( 2790 ) ) );
            ASSERT_TRUE( ModExpIs( FromUInt( 2790 ), FromUInt( 2753 ), FromUInt( 3233 ), FromUInt( 65 ) ) );

            // 0^e = 0, 1^e = 1, x^1 = x, (n-1)^2 = 1
            ASSERT_TRUE( ModExpIs( CORE::CBigUnsigned(), FromUInt( 5 ), FromUInt( 3233 ), CORE::CBigUnsigned() ) );
            ASSERT_TRUE( ModExpIs( FromUInt( 1 ), FromUInt( 65537 ), FromUInt( 3233 ), FromUInt( 1 ) ) );
            ASSERT_TRUE( ModExpIs( FromUInt( 1234 ), FromUInt( 1 ), FromUInt( 3233 ), FromUInt( 1234 ) ) );
            ASSERT_TRUE( ModExpIs( FromUInt( 3232 ), FromUInt( 2 ), FromUInt( 3233 ), FromUInt( 1 ) ) );

            // a modulus of 3 and the largest 32 bit odd modulus, a modulus that is exactly 32 and 64 bits
            ASSERT_TRUE( ModExpIs( FromUInt( 2 ), FromUInt( 10 ), FromUInt( 3 ), FromUInt( 1 ) ) );
            ASSERT_TRUE( ModExpIs( FromUInt( 0xFFFFFFFE ), FromUInt( 3 ), FromUInt( 0xFFFFFFFF ), FromUInt( 0xFFFFFFFE ) ) );
            ASSERT_TRUE( ModExpIs( FromHex( "123456789abcdef0" ), FromUInt( 3 ), FromHex( "ffffffffffffffc5" ), FromHex( "28255cc31e8f0ea3" ) ) );

            // what is refused
            CORE::CMontgomeryModulus context;
            CORE::CBigUnsigned result;
            ASSERT_FALSE( context.IsInitialized() );
            ASSERT_FALSE( context.ModExp( FromUInt( 2 ), FromUInt( 3 ), result ) );
            ASSERT_FALSE( context.Initialize( FromUInt( 100 ) ) );
            ASSERT_FALSE( context.Initialize( FromUInt( 1 ) ) );
            ASSERT_FALSE( context.Initialize( CORE::CBigUnsigned() ) );
            ASSERT_FALSE( context.IsInitialized() );
            ASSERT_TRUE( context.Initialize( FromUInt( 3233 ) ) && context.IsInitialized() );
            ASSERT_FALSE( context.ModExp( FromUInt( 3233 ), FromUInt( 3 ), result ) );
            ASSERT_FALSE( context.ModExp( FromUInt( 9999 ), FromUInt( 3 ), result ) );
            ASSERT_FALSE( context.ModExp( FromUInt( 5 ), CORE::CBigUnsigned(), result ) );
            ASSERT_TRUE( result.IsZero() );

            // the context is used for many calls, with the same answers
            for ( UInt32 i=0; i<10; ++i )
            {
                ASSERT_TRUE( context.ModExp( FromUInt( 65 ), FromUInt( 17 ), result ) && 0 == result.Compare( FromUInt( 2790 ) ) );
                ASSERT_TRUE( context.ModExp( FromUInt( 2790 ), FromUInt( 2753 ), result ) && 0 == result.Compare( FromUInt( 65 ) ) );
            }

            // a context that is initialized again with another modulus
            ASSERT_TRUE( context.Initialize( FromUInt( 497 ) ) );
            ASSERT_TRUE( context.ModExp( FromUInt( 4 ), FromUInt( 13 ), result ) && 0 == result.Compare( FromUInt( 445 ) ) );
            ASSERT_TRUE( 0 == context.GetModulus().Compare( FromUInt( 497 ) ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Modular exponentiation of 64, 130, 1024 and 2048 bit numbers gives the answers of an independent big number implementation" )
        try
        {
            ASSERT_TRUE( ModExpIs( FromHex( g_modExp0BaseHex ), FromHex( g_modExp0ExponentHex ), FromHex( g_modExp0ModulusHex ), FromHex( g_modExp0ResultHex ) ) );
            ASSERT_TRUE( ModExpIs( FromHex( g_modExp1BaseHex ), FromHex( g_modExp1ExponentHex ), FromHex( g_modExp1ModulusHex ), FromHex( g_modExp1ResultHex ) ) );
            ASSERT_TRUE( ModExpIs( FromHex( g_modExp2BaseHex ), FromHex( g_modExp2ExponentHex ), FromHex( g_modExp2ModulusHex ), FromHex( g_modExp2ResultHex ) ) );
            ASSERT_TRUE( ModExpIs( FromHex( g_modExp3BaseHex ), FromHex( g_modExp3ExponentHex ), FromHex( g_modExp3ModulusHex ), FromHex( g_modExp3ResultHex ) ) );
            ASSERT_TRUE( 2048 == FromHex( g_modExp3ModulusHex ).GetBitLength() );

            // a different answer is not accepted
            CORE::CBigUnsigned wrong = FromHex( g_modExp2ResultHex );
            ASSERT_FALSE( ModExpIs( FromHex( g_modExp2BaseHex ), FromHex( g_modExp2ExponentHex ), FromHex( g_modExp2ModulusHex ), FromUInt( 1 ) ) );
            ASSERT_TRUE( !wrong.IsZero() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    CORE::CLogStreamScope::FlushLogs();
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ALL BigUnsigned TESTS COMPLETED" );
}

/*-------------------------------------------------------------------------*/
