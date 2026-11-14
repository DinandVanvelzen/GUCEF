/*
 *  gucefMATH_TestApp: GUCEF test application for the Math module
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

#include <math.h>

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_MATH_CTCOMPLEX_H
#include "gucefMATH_CTComplex.h"
#define GUCEF_MATH_CTCOMPLEX_H
#endif /* GUCEF_MATH_CTCOMPLEX_H ? */

#ifndef GUCEF_MATH_CPCG32_H
#include "gucefMATH_CPcg32.h"
#define GUCEF_MATH_CPCG32_H
#endif /* GUCEF_MATH_CPCG32_H ? */

#include "TestComplex.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/


typedef MATH::CTComplex< MATH::Float64 > TComplex64;
typedef MATH::CTComplex< MATH::Float32 > TComplex32;

static const MATH::Float64 Pi = 3.14159265358979323846;

/*-------------------------------------------------------------------------*/

/** A seeded value in [ -range, range ) */
static MATH::Float64
NextValue( MATH::CPcg32& random, MATH::Float64 range )
{
    return ( (MATH::Float64) random.NextUInt32() / 2147483648.0 - 1.0 ) * range;
}

/*-------------------------------------------------------------------------*/

static bool
Near( const TComplex64& a, const TComplex64& b, MATH::Float64 relative )
{
    const MATH::Float64 scale = a.GetMagnitude() > b.GetMagnitude() ? a.GetMagnitude() : b.GetMagnitude();
    return ( a - b ).GetMagnitude() <= relative * scale;
}

/*-------------------------------------------------------------------------*/

static void
PerformArithmeticTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Complex: layout is two values, arithmetic matches hand computed results" )
    {
        ASSERT_TRUE( sizeof( TComplex64 ) == 2 * sizeof( MATH::Float64 ) );
        ASSERT_TRUE( sizeof( TComplex32 ) == 2 * sizeof( MATH::Float32 ) );
        TComplex64 array[ 4 ];
        ASSERT_TRUE( reinterpret_cast< const char* >( &array[ 3 ] ) - reinterpret_cast< const char* >( &array[ 0 ] ) == (std::ptrdiff_t) ( 3 * sizeof( TComplex64 ) ) );
        ASSERT_TRUE( &array[ 0 ].im == &array[ 0 ].re + 1 );

        TComplex64 zero;
        ASSERT_TRUE( 0.0 == zero.re && 0.0 == zero.im );

        const TComplex64 product = TComplex64( 1.0, 2.0 ) * TComplex64( 3.0, 4.0 );
        ASSERT_TRUE( -5.0 == product.re && 10.0 == product.im );

        const TComplex64 sum = TComplex64( 1.0, 2.0 ) + TComplex64( 3.0, 4.0 );
        ASSERT_TRUE( 4.0 == sum.re && 6.0 == sum.im );
        const TComplex64 difference = TComplex64( 1.0, 2.0 ) - TComplex64( 3.0, 4.0 );
        ASSERT_TRUE( -2.0 == difference.re && -2.0 == difference.im );
        const TComplex64 scaled = TComplex64( 1.0, 2.0 ) * 2.0;
        ASSERT_TRUE( 2.0 == scaled.re && 4.0 == scaled.im );
        const TComplex64 divided = TComplex64( 1.0, 2.0 ) / 2.0;
        ASSERT_TRUE( 0.5 == divided.re && 1.0 == divided.im );
        const TComplex64 negated = -TComplex64( 1.0, 2.0 );
        ASSERT_TRUE( -1.0 == negated.re && -2.0 == negated.im );

        // ( 1 + 2i ) / ( 3 + 4i ) = ( 11 + 2i ) / 25
        const TComplex64 quotient = TComplex64( 1.0, 2.0 ) / TComplex64( 3.0, 4.0 );
        ASSERT_TRUE( fabs( quotient.re - 0.44 ) < 1e-15 && fabs( quotient.im - 0.08 ) < 1e-15 );

        const TComplex64 i = TComplex64::ImaginaryUnit();
        const TComplex64 minusOne = i * i;
        ASSERT_TRUE( -1.0 == minusOne.re && 0.0 == minusOne.im );

        TComplex64 accumulate( 1.0, 1.0 );
        accumulate += TComplex64( 1.0, 0.0 );
        accumulate -= TComplex64( 0.0, 1.0 );
        accumulate *= TComplex64( 0.0, 1.0 );
        ASSERT_TRUE( 0.0 == accumulate.re && 2.0 == accumulate.im );
        accumulate /= TComplex64( 0.0, 2.0 );
        ASSERT_TRUE( 1.0 == accumulate.re && 0.0 == accumulate.im );
        accumulate *= 3.0;
        accumulate /= 2.0;
        ASSERT_TRUE( 1.5 == accumulate.re );

        ASSERT_TRUE( TComplex64( 1.0, 2.0 ) == TComplex64( 1.0, 2.0 ) && TComplex64( 1.0, 2.0 ) != TComplex64( 1.0, 3.0 ) );
        ASSERT_TRUE( TComplex64( 2.5 ) == TComplex64( 2.5, 0.0 ) );

        const TComplex64 conjugate = TComplex64( 1.0, 2.0 ).Conjugate();
        ASSERT_TRUE( 1.0 == conjugate.re && -2.0 == conjugate.im );
        ASSERT_TRUE( 5.0 == TComplex64( 3.0, 4.0 ).GetMagnitude() && 25.0 == TComplex64( 3.0, 4.0 ).GetSquaredMagnitude() );
        ASSERT_TRUE( 0.0 == TComplex64().GetArgument().GetValue() );
        ASSERT_TRUE( fabs( TComplex64( 0.0, 1.0 ).GetArgument().GetValue() - Pi / 2.0 ) < 1e-15 );
        ASSERT_TRUE( fabs( TComplex64( -1.0, 0.0 ).GetArgument().GetValue() - Pi ) < 1e-15 );
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Complex: division is the inverse of multiplication for 10^4 seeded pairs, also with very different magnitudes" )
    {
        MATH::CPcg32 random( 1234, 1 );
        bool ok = true;
        for ( int n = 0; n < 10000 && ok; ++n )
        {
            const TComplex64 a( NextValue( random, 100.0 ), NextValue( random, 100.0 ) );
            const TComplex64 b( NextValue( random, 100.0 ), NextValue( random, 100.0 ) );
            if ( !Near( ( a * b ) / b, a, 1e-13 ) || !Near( ( a / b ) * b, a, 1e-13 ) )
            {
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Complex division failed at pair " << n << " of the stream with seed 1234" );
                ok = false;
            }
        }
        ASSERT_TRUE( ok );

        // Smith's algorithm keeps this finite: the plain formula overflows in the denominator
        const TComplex64 big( 1e200, 1e200 );
        const TComplex64 quotient = big / big;
        ASSERT_TRUE( fabs( quotient.re - 1.0 ) < 1e-15 && fabs( quotient.im ) < 1e-15 );
        const TComplex64 tiny = TComplex64( 1.0, 0.0 ) / TComplex64( 1e-200, 1e-200 );
        ASSERT_TRUE( tiny.re > 1e199 && tiny.im < -1e199 );
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
PerformFunctionTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Complex: Exp( i pi ) is -1 within 1e-15 and Exp and Log are inverses" )
    {
        const TComplex64 e = TComplex64( 0.0, Pi ).Exp();
        ASSERT_TRUE( fabs( e.re + 1.0 ) < 1e-15 && fabs( e.im ) < 1e-15 );

        const TComplex32 e32 = TComplex32( 0.0f, 3.14159265f ).Exp();
        ASSERT_TRUE( fabs( e32.re + 1.0f ) < 1e-6f && fabs( e32.im ) < 1e-6f );

        MATH::CPcg32 random( 77, 2 );
        bool ok = true;
        for ( int n = 0; n < 2000 && ok; ++n )
        {
            const TComplex64 z( NextValue( random, 5.0 ), NextValue( random, 5.0 ) );
            ok = Near( z.Exp().Log(), TComplex64( z.re, z.im - 2.0 * Pi * floor( ( z.im + Pi ) / ( 2.0 * Pi ) ) ), 1e-12 );
            ok = ok && Near( z.Log().Exp(), z, 1e-12 );
        }
        ASSERT_TRUE( ok );
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Complex: polar form round trips, the argument is in ( -pi, pi ]" )
    {
        MATH::CPcg32 random( 5, 5 );
        bool ok = true;
        for ( int n = 0; n < 5000 && ok; ++n )
        {
            const MATH::Float64 magnitude = 0.001 + fabs( NextValue( random, 50.0 ) );
            const MATH::Float64 argument = NextValue( random, 3.1 );
            const TComplex64 z = TComplex64::FromPolar( magnitude, MATH::CTRadian< MATH::Float64 >( argument ) );
            ok = fabs( z.GetMagnitude() - magnitude ) <= 1e-13 * magnitude && fabs( z.GetArgument().GetValue() - argument ) < 1e-13;
        }
        ASSERT_TRUE( ok );
        const TComplex64 minusOne = TComplex64::FromPolar( 1.0, MATH::CTRadian< MATH::Float64 >( Pi ) );
        ASSERT_TRUE( fabs( minusOne.re + 1.0 ) < 1e-15 );
        ASSERT_TRUE( fabs( TComplex64( -1.0, 0.0 ).GetArgument().GetValue() - Pi ) < 1e-15 );
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Complex: Sqrt is the principal root and Pow follows the polar rules" )
    {
        const TComplex64 minusFour = TComplex64( -4.0, 0.0 ).Sqrt();
        ASSERT_TRUE( 0.0 == minusFour.re && 2.0 == minusFour.im );
        const TComplex64 four = TComplex64( 4.0, 0.0 ).Sqrt();
        ASSERT_TRUE( 2.0 == four.re && 0.0 == four.im );
        ASSERT_TRUE( 0.0 == TComplex64().Sqrt().re );
        const TComplex64 below = TComplex64( -4.0, -0.0 ).Sqrt();
        ASSERT_TRUE( below.re >= 0.0 );

        MATH::CPcg32 random( 99, 3 );
        bool ok = true;
        for ( int n = 0; n < 5000 && ok; ++n )
        {
            const TComplex64 z( NextValue( random, 20.0 ), NextValue( random, 20.0 ) );
            const TComplex64 root = z.Sqrt();
            ok = root.re >= 0.0 && Near( root * root, z, 1e-13 );
        }
        ASSERT_TRUE( ok );

        const TComplex64 square = TComplex64( 1.0, 1.0 ).Pow( 2.0 );
        ASSERT_TRUE( fabs( square.re ) < 1e-14 && fabs( square.im - 2.0 ) < 1e-14 );
        const TComplex64 half = TComplex64( -3.0, 4.0 ).Pow( 0.5 );
        ASSERT_TRUE( Near( half, TComplex64( -3.0, 4.0 ).Sqrt(), 1e-13 ) );
        const TComplex64 complexExponent = TComplex64( 2.0, 0.0 ).Pow( TComplex64( 3.0, 0.0 ) );
        ASSERT_TRUE( fabs( complexExponent.re - 8.0 ) < 1e-13 && fabs( complexExponent.im ) < 1e-13 );
        const TComplex64 iToI = TComplex64( 0.0, 1.0 ).Pow( TComplex64( 0.0, 1.0 ) );        // e^( -pi / 2 )
        ASSERT_TRUE( fabs( iToI.re - exp( -Pi / 2.0 ) ) < 1e-14 && fabs( iToI.im ) < 1e-14 );
        const TComplex64 one = TComplex64( 3.0, 4.0 ).Pow( 0.0 );
        ASSERT_TRUE( 1.0 == one.re && 0.0 == one.im );
        const TComplex64 zeroPower = TComplex64( 0.0, 0.0 ).Pow( 2.0 );
        ASSERT_TRUE( 0.0 == zeroPower.re && 0.0 == zeroPower.im );
        const TComplex64 zeroNegative = TComplex64( 0.0, 0.0 ).Pow( -1.0 );
        ASSERT_FALSE( zeroNegative.re == zeroNegative.re );        // not a number
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
PerformComplexTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING Complex TESTS" );

    PerformArithmeticTests();
    PerformFunctionTests();

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Complex TESTS COMPLETED" );
}

/*-------------------------------------------------------------------------*/
