/*
 *  gucefCORE_TestApp: GUCEF test application for the CORE module
 *  Copyright (C) 2002 - 2007.  Dinand Vanvelzen
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

#ifndef GUCEF_CORE_CTFRACTION_H
#include "gucefCORE_CTFraction.h"
#define GUCEF_CORE_CTFRACTION_H
#endif /* GUCEF_CORE_CTFRACTION_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestFraction.h"

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

void
PerformFractionTests( void )
{
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING CTFraction TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "CTFraction" );

    // Test 1: Default construction (0/1)
    GUCEF_TESTFW_TESTCASE( "Test 1: Default construction" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 1: Default construction" );
            CORE::TInt32Fraction f;
            ASSERT_TRUE( f.GetNumerator() == 0 );
            ASSERT_TRUE( f.GetDenominator() == 1 );
            ASSERT_TRUE( f.ToFloat32() == 0.0f );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 2: Basic parameterized construction - coprime values
    GUCEF_TESTFW_TESTCASE( "Test 2: Basic construction - coprime values" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 2: Basic construction - coprime values" );
            CORE::TInt32Fraction f( 1, 2 );
            ASSERT_TRUE( f.GetNumerator() == 1 );
            ASSERT_TRUE( f.GetDenominator() == 2 );

            CORE::TInt32Fraction g( 3, 7 );
            ASSERT_TRUE( g.GetNumerator() == 3 );
            ASSERT_TRUE( g.GetDenominator() == 7 );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 3: Normalization by GCD reduction (positive values)
    GUCEF_TESTFW_TESTCASE( "Test 3: Normalization by GCD" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 3: Normalization by GCD" );
            // 6/4 -> GCD(6,4)=2 -> 3/2
            CORE::TInt32Fraction f( 6, 4 );
            ASSERT_TRUE( f.GetNumerator() == 3 );
            ASSERT_TRUE( f.GetDenominator() == 2 );

            // 9/3 -> GCD(9,3)=3 -> 3/1
            CORE::TInt32Fraction g( 9, 3 );
            ASSERT_TRUE( g.GetNumerator() == 3 );
            ASSERT_TRUE( g.GetDenominator() == 1 );

            // 2/6 -> GCD(2,6)=2 -> 1/3
            CORE::TInt32Fraction h( 2, 6 );
            ASSERT_TRUE( h.GetNumerator() == 1 );
            ASSERT_TRUE( h.GetDenominator() == 3 );

            // Already normalized: 1/1
            CORE::TInt32Fraction i( 4, 4 );
            ASSERT_TRUE( i.GetNumerator() == 1 );
            ASSERT_TRUE( i.GetDenominator() == 1 );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 4: Zero denominator throws an exception
    GUCEF_TESTFW_TESTCASE( "Test 4: Zero denominator throws" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 4: Zero denominator throws" );
            bool exceptionThrown = false;
            try
            {
                CORE::TInt32Fraction f( 1, 0 );
            }
            catch( const GUCEF::invalid_argument& )
            {
                exceptionThrown = true;
            }
            catch( const std::invalid_argument& )
            {
                exceptionThrown = true;
            }
            catch( ... )
            {
                exceptionThrown = true;
            }
            ASSERT_TRUE( exceptionThrown );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 5: Unsigned type construction
    GUCEF_TESTFW_TESTCASE( "Test 5: Unsigned type construction" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 5: Unsigned type construction" );
            CORE::TUInt32Fraction f( 3, 6 );  // GCD(3,6)=3 -> 1/2
            ASSERT_TRUE( f.GetNumerator() == 1 );
            ASSERT_TRUE( f.GetDenominator() == 2 );

            CORE::TUInt64UInt32Fraction g( 10, 4 );  // GCD(10,4)=2 -> 5/2
            ASSERT_TRUE( g.GetNumerator() == 5 );
            ASSERT_TRUE( g.GetDenominator() == 2 );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 6: Construction from TInt32T2Fraction C-style struct
    GUCEF_TESTFW_TESTCASE( "Test 6: Construction from TInt32T2Fraction" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 6: Construction from TInt32T2Fraction" );
            CORE::TInt32T2Fraction cs;
            cs.numerator = 3;
            cs.denominator = 4;
            CORE::TInt32Fraction f( cs );
            ASSERT_TRUE( f.GetNumerator() == 3 );
            ASSERT_TRUE( f.GetDenominator() == 4 );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 7: Construction from TUInt32T2Fraction C-style struct
    GUCEF_TESTFW_TESTCASE( "Test 7: Construction from TUInt32T2Fraction" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 7: Construction from TUInt32T2Fraction" );
            CORE::TUInt32T2Fraction cs;
            cs.numerator = 5;
            cs.denominator = 6;
            CORE::TUInt32Fraction f( cs );
            ASSERT_TRUE( f.GetNumerator() == 5 );
            ASSERT_TRUE( f.GetDenominator() == 6 );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 8: Construction from TInt64WInt32Fraction C-style struct
    GUCEF_TESTFW_TESTCASE( "Test 8: Construction from TInt64WInt32Fraction" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 8: Construction from TInt64WInt32Fraction" );
            CORE::TInt64WInt32Fraction cs;
            cs.numerator = 7;
            cs.denominator = 2;
            CORE::TInt64Int32Fraction f( cs );
            ASSERT_TRUE( f.GetNumerator() == 7 );
            ASSERT_TRUE( f.GetDenominator() == 2 );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 9: Construction from TUInt64WUInt32Fraction C-style struct
    GUCEF_TESTFW_TESTCASE( "Test 9: Construction from TUInt64WUInt32Fraction" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 9: Construction from TUInt64WUInt32Fraction" );
            CORE::TUInt64WUInt32Fraction cs;
            cs.numerator = 8;
            cs.denominator = 3;
            CORE::TUInt64UInt32Fraction f( cs );
            ASSERT_TRUE( f.GetNumerator() == 8 );
            ASSERT_TRUE( f.GetDenominator() == 3 );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 10: Construction from TInt64T2Fraction C-style struct
    GUCEF_TESTFW_TESTCASE( "Test 10: Construction from TInt64T2Fraction" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 10: Construction from TInt64T2Fraction" );
            CORE::TInt64T2Fraction cs;
            cs.numerator = 5;
            cs.denominator = 9;
            CORE::TInt64Fraction f( cs );
            ASSERT_TRUE( f.GetNumerator() == 5 );
            ASSERT_TRUE( f.GetDenominator() == 9 );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 11: operator== and operator!=
    GUCEF_TESTFW_TESTCASE( "Test 11: operator== and operator!=" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 11: operator== and operator!=" );
            CORE::TInt32Fraction a( 1, 2 );
            CORE::TInt32Fraction b( 1, 2 );
            CORE::TInt32Fraction c( 2, 4 );  // normalizes to 1/2
            CORE::TInt32Fraction d( 1, 3 );

            ASSERT_TRUE( a == b );
            ASSERT_TRUE( a == c );  // both normalize to 1/2
            ASSERT_FALSE( a == d );
            ASSERT_TRUE( a != d );
            ASSERT_FALSE( a != b );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 12: Comparison operators <, <=, >, >=
    GUCEF_TESTFW_TESTCASE( "Test 12: Comparison operators" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 12: Comparison operators" );
            CORE::TInt32Fraction oneThird( 1, 3 );
            CORE::TInt32Fraction oneHalf( 1, 2 );
            CORE::TInt32Fraction twoThirds( 2, 3 );
            CORE::TInt32Fraction oneHalfDup( 1, 2 );

            ASSERT_TRUE( oneThird < oneHalf );
            ASSERT_TRUE( oneHalf < twoThirds );
            ASSERT_TRUE( oneThird < twoThirds );

            ASSERT_FALSE( oneHalf < oneThird );
            ASSERT_FALSE( oneHalf < oneHalf );

            ASSERT_TRUE( oneHalf <= oneHalf );
            ASSERT_TRUE( oneThird <= oneHalf );
            ASSERT_FALSE( oneHalf <= oneThird );

            ASSERT_TRUE( twoThirds > oneHalf );
            ASSERT_FALSE( oneThird > oneHalf );
            ASSERT_FALSE( oneHalf > oneHalf );

            ASSERT_TRUE( oneHalf >= oneHalf );
            ASSERT_TRUE( twoThirds >= oneHalf );
            ASSERT_FALSE( oneThird >= oneHalf );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 13: operator+= with fraction
    GUCEF_TESTFW_TESTCASE( "Test 13: operator+= with fraction" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 13: operator+= with fraction" );
            // 1/4 + 1/4 = 2/4 = 1/2
            CORE::TInt32Fraction f( 1, 4 );
            f += CORE::TInt32Fraction( 1, 4 );
            ASSERT_TRUE( f == CORE::TInt32Fraction( 1, 2 ) );

            // 1/3 + 1/6 = 2/6 + 1/6 = 3/6 = 1/2
            CORE::TInt32Fraction g( 1, 3 );
            g += CORE::TInt32Fraction( 1, 6 );
            ASSERT_TRUE( g == CORE::TInt32Fraction( 1, 2 ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 14: operator-= with fraction
    GUCEF_TESTFW_TESTCASE( "Test 14: operator-= with fraction" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 14: operator-= with fraction" );
            // 3/4 - 1/4 = 2/4 = 1/2
            CORE::TInt32Fraction f( 3, 4 );
            f -= CORE::TInt32Fraction( 1, 4 );
            ASSERT_TRUE( f == CORE::TInt32Fraction( 1, 2 ) );

            // 1/1 - 1/2 = 2/2 - 1/2 = 1/2
            CORE::TInt32Fraction g( 1, 1 );
            g -= CORE::TInt32Fraction( 1, 2 );
            ASSERT_TRUE( g == CORE::TInt32Fraction( 1, 2 ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 15: operator*= with fraction
    GUCEF_TESTFW_TESTCASE( "Test 15: operator*= with fraction" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 15: operator*= with fraction" );
            // 2/3 * 3/4 = 6/12 = 1/2
            CORE::TInt32Fraction f( 2, 3 );
            f *= CORE::TInt32Fraction( 3, 4 );
            ASSERT_TRUE( f == CORE::TInt32Fraction( 1, 2 ) );

            // 3/5 * 5/3 = 15/15 = 1/1
            CORE::TInt32Fraction g( 3, 5 );
            g *= CORE::TInt32Fraction( 5, 3 );
            ASSERT_TRUE( g == CORE::TInt32Fraction( 1, 1 ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 16: operator/= with fraction
    GUCEF_TESTFW_TESTCASE( "Test 16: operator/= with fraction" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 16: operator/= with fraction" );
            // 1/2 / 1/4 = 1/2 * 4/1 = 4/2 = 2/1
            CORE::TInt32Fraction f( 1, 2 );
            f /= CORE::TInt32Fraction( 1, 4 );
            ASSERT_TRUE( f == CORE::TInt32Fraction( 2, 1 ) );

            // 2/3 / 4/3 = 2/3 * 3/4 = 6/12 = 1/2
            CORE::TInt32Fraction g( 2, 3 );
            g /= CORE::TInt32Fraction( 4, 3 );
            ASSERT_TRUE( g == CORE::TInt32Fraction( 1, 2 ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 17: operator/= with fraction - division by zero fraction
    GUCEF_TESTFW_TESTCASE( "Test 17: Division by zero fraction throws" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 17: Division by zero fraction throws" );
            bool exceptionThrown = false;
            try
            {
                CORE::TInt32Fraction f( 1, 2 );
                f /= CORE::TInt32Fraction( 0, 1 );
            }
            catch( const GUCEF::invalid_argument& )
            {
                exceptionThrown = true;
            }
            catch( const std::invalid_argument& )
            {
                exceptionThrown = true;
            }
            catch( ... )
            {
                exceptionThrown = true;
            }
            ASSERT_TRUE( exceptionThrown );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 18: operator+= with scalar
    GUCEF_TESTFW_TESTCASE( "Test 18: operator+= with scalar" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 18: operator+= with scalar" );
            // 1/4 + 1 = 1/4 + 4/4 = 5/4
            CORE::TInt32Fraction f( 1, 4 );
            f += (Int32) 1;
            ASSERT_TRUE( f == CORE::TInt32Fraction( 5, 4 ) );

            // 1/3 + 2 = 1/3 + 6/3 = 7/3
            CORE::TInt32Fraction g( 1, 3 );
            g += (Int32) 2;
            ASSERT_TRUE( g == CORE::TInt32Fraction( 7, 3 ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 19: operator-= with scalar
    GUCEF_TESTFW_TESTCASE( "Test 19: operator-= with scalar" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 19: operator-= with scalar" );
            // 5/4 - 1 = 5/4 - 4/4 = 1/4
            CORE::TInt32Fraction f( 5, 4 );
            f -= (Int32) 1;
            ASSERT_TRUE( f == CORE::TInt32Fraction( 1, 4 ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 20: operator*= with scalar
    GUCEF_TESTFW_TESTCASE( "Test 20: operator*= with scalar" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 20: operator*= with scalar" );
            // 1/4 * 3 = 3/4
            CORE::TInt32Fraction f( 1, 4 );
            f *= (Int32) 3;
            ASSERT_TRUE( f == CORE::TInt32Fraction( 3, 4 ) );

            // 1/6 * 2 = 2/6 = 1/3
            CORE::TInt32Fraction g( 1, 6 );
            g *= (Int32) 2;
            ASSERT_TRUE( g == CORE::TInt32Fraction( 1, 3 ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 21: operator/= with scalar
    GUCEF_TESTFW_TESTCASE( "Test 21: operator/= with scalar" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 21: operator/= with scalar" );
            // 3/4 / 3 = 3/(4*3) = 3/12 = 1/4
            CORE::TInt32Fraction f( 3, 4 );
            f /= (Int32) 3;
            ASSERT_TRUE( f == CORE::TInt32Fraction( 1, 4 ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 22: operator/= with scalar zero throws
    GUCEF_TESTFW_TESTCASE( "Test 22: Scalar division by zero throws" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 22: Scalar division by zero throws" );
            bool exceptionThrown = false;
            try
            {
                CORE::TInt32Fraction f( 1, 2 );
                f /= (Int32) 0;
            }
            catch( const GUCEF::invalid_argument& )
            {
                exceptionThrown = true;
            }
            catch( const std::invalid_argument& )
            {
                exceptionThrown = true;
            }
            catch( ... )
            {
                exceptionThrown = true;
            }
            ASSERT_TRUE( exceptionThrown );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 23: Binary operator+
    GUCEF_TESTFW_TESTCASE( "Test 23: Binary operator+" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 23: Binary operator+" );
            // 1/4 + 1/4 = 1/2
            CORE::TInt32Fraction result = CORE::TInt32Fraction( 1, 4 ) + CORE::TInt32Fraction( 1, 4 );
            ASSERT_TRUE( result == CORE::TInt32Fraction( 1, 2 ) );

            // 1/3 + 2 = 7/3
            CORE::TInt32Fraction result2 = CORE::TInt32Fraction( 1, 3 ) + (Int32) 2;
            ASSERT_TRUE( result2 == CORE::TInt32Fraction( 7, 3 ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 24: Binary operator-
    GUCEF_TESTFW_TESTCASE( "Test 24: Binary operator-" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 24: Binary operator-" );
            // 3/4 - 1/4 = 1/2
            CORE::TInt32Fraction result = CORE::TInt32Fraction( 3, 4 ) - CORE::TInt32Fraction( 1, 4 );
            ASSERT_TRUE( result == CORE::TInt32Fraction( 1, 2 ) );

            // 5/4 - 1 = 1/4
            CORE::TInt32Fraction result2 = CORE::TInt32Fraction( 5, 4 ) - (Int32) 1;
            ASSERT_TRUE( result2 == CORE::TInt32Fraction( 1, 4 ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 25: Binary operator*
    GUCEF_TESTFW_TESTCASE( "Test 25: Binary operator*" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 25: Binary operator*" );
            // 1/2 * 1/3 = 1/6
            CORE::TInt32Fraction result = CORE::TInt32Fraction( 1, 2 ) * CORE::TInt32Fraction( 1, 3 );
            ASSERT_TRUE( result == CORE::TInt32Fraction( 1, 6 ) );

            // 1/4 * 2 = 2/4 = 1/2
            CORE::TInt32Fraction result2 = CORE::TInt32Fraction( 1, 4 ) * (Int32) 2;
            ASSERT_TRUE( result2 == CORE::TInt32Fraction( 1, 2 ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 26: Binary operator/
    GUCEF_TESTFW_TESTCASE( "Test 26: Binary operator/" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 26: Binary operator/" );
            // 1/2 / 1/4 = 2
            CORE::TInt32Fraction result = CORE::TInt32Fraction( 1, 2 ) / CORE::TInt32Fraction( 1, 4 );
            ASSERT_TRUE( result == CORE::TInt32Fraction( 2, 1 ) );

            // 3/4 / 3 = 1/4
            CORE::TInt32Fraction result2 = CORE::TInt32Fraction( 3, 4 ) / (Int32) 3;
            ASSERT_TRUE( result2 == CORE::TInt32Fraction( 1, 4 ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 27: ToFloat32 - exact fractions
    GUCEF_TESTFW_TESTCASE( "Test 27: ToFloat32" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 27: ToFloat32" );
            ASSERT_TRUE( CORE::TInt32Fraction( 1, 2 ).ToFloat32() == 0.5f );
            ASSERT_TRUE( CORE::TInt32Fraction( 1, 4 ).ToFloat32() == 0.25f );
            ASSERT_TRUE( CORE::TInt32Fraction( 3, 4 ).ToFloat32() == 0.75f );
            ASSERT_TRUE( CORE::TInt32Fraction( 1, 1 ).ToFloat32() == 1.0f );
            ASSERT_TRUE( CORE::TInt32Fraction( 0, 1 ).ToFloat32() == 0.0f );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 28: ToFloat64 - exact fractions
    GUCEF_TESTFW_TESTCASE( "Test 28: ToFloat64" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 28: ToFloat64" );
            ASSERT_TRUE( CORE::TInt32Fraction( 1, 2 ).ToFloat64() == 0.5 );
            ASSERT_TRUE( CORE::TInt32Fraction( 1, 4 ).ToFloat64() == 0.25 );
            ASSERT_TRUE( CORE::TInt32Fraction( 3, 4 ).ToFloat64() == 0.75 );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 29: FromFloat32 and back
    GUCEF_TESTFW_TESTCASE( "Test 29: FromFloat32 round-trip" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 29: FromFloat32 round-trip" );
            CORE::TInt32Fraction f;
            f.FromFloat32( 0.5f );
            ASSERT_TRUE( f.ToFloat32() == 0.5f );

            CORE::TInt32Fraction g;
            g.FromFloat32( 0.25f );
            ASSERT_TRUE( g.ToFloat32() == 0.25f );

            CORE::TInt32Fraction h;
            h.FromFloat32( 0.75f );
            ASSERT_TRUE( h.ToFloat32() == 0.75f );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 30: ConvertFloat32ToFraction normalizes to exact fraction
    GUCEF_TESTFW_TESTCASE( "Test 30: ConvertFloat32ToFraction" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 30: ConvertFloat32ToFraction" );
            // 0.5 * 1000000 = 500000 / 1000000 -> GCD -> 1/2
            CORE::TInt32Fraction f = CORE::TInt32Fraction::ConvertFloat32ToFraction( 0.5f );
            ASSERT_TRUE( f == CORE::TInt32Fraction( 1, 2 ) );

            // 0.25 * 1000000 = 250000 / 1000000 -> GCD -> 1/4
            CORE::TInt32Fraction g = CORE::TInt32Fraction::ConvertFloat32ToFraction( 0.25f );
            ASSERT_TRUE( g == CORE::TInt32Fraction( 1, 4 ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 31: FromFloat64 and back
    GUCEF_TESTFW_TESTCASE( "Test 31: FromFloat64 round-trip" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 31: FromFloat64 round-trip" );
            CORE::TInt32Fraction f;
            f.FromFloat64( 0.5 );
            ASSERT_TRUE( f.ToFloat64() == 0.5 );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 32: ToMixedFraction - proper fraction (< 1)
    GUCEF_TESTFW_TESTCASE( "Test 32: ToMixedFraction - proper fraction" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 32: ToMixedFraction - proper fraction" );
            CORE::TInt32Fraction f( 3, 4 );
            Int32 wholePart = 99;
            CORE::TInt32Fraction fractionalPart;
            bool ok = f.ToMixedFraction( wholePart, fractionalPart );
            ASSERT_TRUE( ok );
            ASSERT_TRUE( wholePart == 0 );
            ASSERT_TRUE( fractionalPart == CORE::TInt32Fraction( 3, 4 ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 33: ToMixedFraction - improper fraction (>= 1)
    GUCEF_TESTFW_TESTCASE( "Test 33: ToMixedFraction - improper fraction" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 33: ToMixedFraction - improper fraction" );
            // 7/4 = 1 + 3/4
            CORE::TInt32Fraction f( 7, 4 );
            Int32 wholePart = 0;
            CORE::TInt32Fraction fractionalPart;
            bool ok = f.ToMixedFraction( wholePart, fractionalPart );
            ASSERT_TRUE( ok );
            ASSERT_TRUE( wholePart == 1 );
            ASSERT_TRUE( fractionalPart == CORE::TInt32Fraction( 3, 4 ) );

            // 9/3 = 3 (normalizes to 3/1) -> wholePart=3, fractional=0/1
            CORE::TInt32Fraction g( 9, 3 );
            Int32 wholePart2 = 0;
            CORE::TInt32Fraction frac2;
            g.ToMixedFraction( wholePart2, frac2 );
            ASSERT_TRUE( wholePart2 == 3 );
            ASSERT_TRUE( frac2 == CORE::TInt32Fraction( 0, 1 ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 34: FromMixedFraction round-trip
    GUCEF_TESTFW_TESTCASE( "Test 34: FromMixedFraction round-trip" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 34: FromMixedFraction round-trip" );
            // 1 + 3/4 = 7/4
            CORE::TInt32Fraction f;
            f.FromMixedFraction( (Int32) 1, CORE::TInt32Fraction( 3, 4 ) );
            ASSERT_TRUE( f == CORE::TInt32Fraction( 7, 4 ) );

            // 0 + 1/2 = 1/2
            CORE::TInt32Fraction g;
            g.FromMixedFraction( (Int32) 0, CORE::TInt32Fraction( 1, 2 ) );
            ASSERT_TRUE( g == CORE::TInt32Fraction( 1, 2 ) );

            // 3 + 0/1 = 3/1
            CORE::TInt32Fraction h;
            h.FromMixedFraction( (Int32) 3, CORE::TInt32Fraction( 0, 1 ) );
            ASSERT_TRUE( h == CORE::TInt32Fraction( 3, 1 ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 35: ToString
    GUCEF_TESTFW_TESTCASE( "Test 35: ToString" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 35: ToString" );
            ASSERT_TRUE( CORE::TInt32Fraction( 1, 2 ).ToString() == "1/2" );
            ASSERT_TRUE( CORE::TInt32Fraction( 3, 4 ).ToString() == "3/4" );
            ASSERT_TRUE( CORE::TInt32Fraction( 0, 1 ).ToString() == "0/1" );
            ASSERT_TRUE( CORE::TInt32Fraction( 1, 1 ).ToString() == "1/1" );

            // Normalized form: 6/4 -> 3/2
            ASSERT_TRUE( CORE::TInt32Fraction( 6, 4 ).ToString() == "3/2" );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 36: FromString - valid input
    // NOTE: As of this writing, FromString has a bug where m_denominator is never assigned
    // from the parsed string (only checked for zero). The test below checks the intended
    // behavior; if it fails, the implementation needs to be fixed.
    GUCEF_TESTFW_TESTCASE( "Test 36: FromString - valid input" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 36: FromString - valid input" );
            CORE::TInt32Fraction f;
            bool ok = f.FromString( "3/4" );
            ASSERT_TRUE( ok );
            ASSERT_TRUE( f.GetNumerator() == 3 );
            ASSERT_TRUE( f.GetDenominator() == 4 );

            CORE::TInt32Fraction g;
            ok = g.FromString( "1/2" );
            ASSERT_TRUE( ok );
            ASSERT_TRUE( g.GetNumerator() == 1 );
            ASSERT_TRUE( g.GetDenominator() == 2 );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 37: FromString - invalid input (no slash)
    GUCEF_TESTFW_TESTCASE( "Test 37: FromString - no slash returns false" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 37: FromString - no slash returns false" );
            CORE::TInt32Fraction f;
            ASSERT_FALSE( f.FromString( "34" ) );
            ASSERT_FALSE( f.FromString( "" ) );
            ASSERT_FALSE( f.FromString( "abc" ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 38: FromString - zero denominator returns false
    GUCEF_TESTFW_TESTCASE( "Test 38: FromString - zero denominator returns false" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 38: FromString - zero denominator returns false" );
            CORE::TInt32Fraction f;
            ASSERT_FALSE( f.FromString( "1/0" ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 39: ToString/FromString round-trip
    // NOTE: Depends on FromString being correctly implemented (see Test 36 note).
    GUCEF_TESTFW_TESTCASE( "Test 39: ToString/FromString round-trip" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 39: ToString/FromString round-trip" );
            CORE::TInt32Fraction original( 5, 7 );
            CORE::CString str = original.ToString();

            CORE::TInt32Fraction parsed;
            ASSERT_TRUE( parsed.FromString( str ) );
            ASSERT_TRUE( parsed == original );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 40: Multiple typedef instantiations
    GUCEF_TESTFW_TESTCASE( "Test 40: Multiple typedef instantiations" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 40: Multiple typedef instantiations" );
            CORE::TInt8Fraction   i8f( 1, 4 );
            ASSERT_TRUE( i8f.GetNumerator() == 1 && i8f.GetDenominator() == 4 );

            CORE::TUInt8Fraction  u8f( 3, 6 );
            ASSERT_TRUE( u8f.GetNumerator() == 1 && u8f.GetDenominator() == 2 );

            CORE::TInt16Fraction  i16f( 5, 10 );
            ASSERT_TRUE( i16f.GetNumerator() == 1 && i16f.GetDenominator() == 2 );

            CORE::TUInt16Fraction u16f( 7, 3 );
            ASSERT_TRUE( u16f.GetNumerator() == 7 && u16f.GetDenominator() == 3 );

            CORE::TInt64Fraction  i64f( 100, 25 );
            ASSERT_TRUE( i64f.GetNumerator() == 4 && i64f.GetDenominator() == 1 );

            CORE::TUInt64Fraction u64f( 8, 4 );
            ASSERT_TRUE( u64f.GetNumerator() == 2 && u64f.GetDenominator() == 1 );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Test 41: Identity operations
    GUCEF_TESTFW_TESTCASE( "Test 41: Identity operations" )
        try
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Test 41: Identity operations" );
            CORE::TInt32Fraction f( 3, 7 );

            // Adding zero fraction
            CORE::TInt32Fraction result = f + CORE::TInt32Fraction( 0, 1 );
            ASSERT_TRUE( result == f );

            // Multiplying by 1/1
            result = f * CORE::TInt32Fraction( 1, 1 );
            ASSERT_TRUE( result == f );

            // Dividing by 1/1
            result = f / CORE::TInt32Fraction( 1, 1 );
            ASSERT_TRUE( result == f );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    CORE::CLogStreamScope::FlushLogs();
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ALL CTFraction TESTS COMPLETED" );
}

/*-------------------------------------------------------------------------*/
