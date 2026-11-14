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

#ifndef GUCEF_MATH_CTPOLYNOMIALSOLVER_H
#include "gucefMATH_CTPolynomialSolver.h"
#define GUCEF_MATH_CTPOLYNOMIALSOLVER_H
#endif /* GUCEF_MATH_CTPOLYNOMIALSOLVER_H ? */

#ifndef GUCEF_MATH_CPCG32_H
#include "gucefMATH_CPcg32.h"
#define GUCEF_MATH_CPCG32_H
#endif /* GUCEF_MATH_CPCG32_H ? */

#include "TestPolynomialSolver.h"

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

typedef MATH::CTPolynomialSolver< MATH::Float64 > TSolver;
typedef MATH::CTPolynomialSolver< MATH::Float32 > TSolver32;
typedef MATH::Float64 F64;

/*-------------------------------------------------------------------------*/

static bool
Near( F64 a, F64 b, F64 tolerance )
{
    return fabs( a - b ) <= tolerance;
}

/*-------------------------------------------------------------------------*/

/** Coefficients from the highest power down of the polynomial with the given roots and leading coefficient 'lead' */
static void
Expand( const F64* roots, int degree, F64 lead, F64* coefficients )
{
    for ( int i = 0; i <= degree; ++i )
        coefficients[ i ] = 0.0;
    coefficients[ 0 ] = lead;
    for ( int r = 0; r < degree; ++r )
    {
        for ( int i = r + 1; i >= 1; --i )
            coefficients[ i ] -= roots[ r ] * coefficients[ i - 1 ];
    }
}

/** The polynomial at x, and the sum of the magnitudes of its terms there (the scale for a relative residual) */
static F64
RelativeResidual( const F64* coefficients, int degree, F64 x )
{
    F64 value = 0.0;
    F64 scale = 0.0;
    for ( int i = 0; i <= degree; ++i )
    {
        value = value * x + coefficients[ i ];
        scale = scale * fabs( x ) + fabs( coefficients[ i ] );
    }
    return scale > 0.0 ? fabs( value ) / scale : fabs( value );
}

/*-------------------------------------------------------------------------*/

static void
PerformFixedCaseTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "PolynomialSolver: linear and quadratic hand computed cases, degradation and negative discriminant" )
    {
        F64 roots[ 4 ];
        ASSERT_TRUE( 1 == TSolver::SolveLinear( 2.0, -6.0, roots ) && 3.0 == roots[ 0 ] );
        ASSERT_TRUE( 0 == TSolver::SolveLinear( 0.0, 5.0, roots ) );

        ASSERT_TRUE( 2 == TSolver::SolveQuadratic( 1.0, -3.0, 2.0, roots ) );
        ASSERT_TRUE( 1.0 == roots[ 0 ] && 2.0 == roots[ 1 ] );
        ASSERT_TRUE( 2 == TSolver::SolveQuadratic( -2.0, 2.0, 4.0, roots ) );          // -2 ( x + 1 ) ( x - 2 )
        ASSERT_TRUE( -1.0 == roots[ 0 ] && 2.0 == roots[ 1 ] );
        ASSERT_TRUE( 1 == TSolver::SolveQuadratic( 1.0, -2.0, 1.0, roots ) && 1.0 == roots[ 0 ] );
        ASSERT_TRUE( 1 == TSolver::SolveQuadratic( 1.0, 0.0, 0.0, roots ) && 0.0 == roots[ 0 ] );
        ASSERT_TRUE( 2 == TSolver::SolveQuadratic( 1.0, 0.0, -4.0, roots ) && -2.0 == roots[ 0 ] && 2.0 == roots[ 1 ] );
        ASSERT_TRUE( 2 == TSolver::SolveQuadratic( 1.0, -1.0, 0.0, roots ) && 0.0 == roots[ 0 ] && 1.0 == roots[ 1 ] );
        ASSERT_TRUE( 0 == TSolver::SolveQuadratic( 1.0, 0.0, 1.0, roots ) );          // x^2 + 1
        ASSERT_TRUE( 0 == TSolver::SolveQuadratic( 1.0, 1.0, 1.0, roots ) );

        // a = 0 degrades to linear
        roots[ 0 ] = 99.0; roots[ 1 ] = 99.0;
        ASSERT_TRUE( 1 == TSolver::SolveQuadratic( 0.0, 2.0, -6.0, roots ) && 3.0 == roots[ 0 ] && 99.0 == roots[ 1 ] );
        ASSERT_TRUE( 1 == TSolver::SolveCubic( 0.0, 0.0, 2.0, -6.0, roots ) && 3.0 == roots[ 0 ] );
        ASSERT_TRUE( 2 == TSolver::SolveCubic( 0.0, 1.0, -3.0, 2.0, roots ) && 1.0 == roots[ 0 ] && 2.0 == roots[ 1 ] );
        ASSERT_TRUE( 3 == TSolver::SolveQuartic( 0.0, 1.0, -6.0, 11.0, -6.0, roots ) );
        ASSERT_TRUE( 0 == TSolver::SolveQuartic( 0.0, 0.0, 0.0, 0.0, 0.0, roots ) );
        // a leading coefficient that is only rounding noise next to the others is dropped too
        ASSERT_TRUE( 1 == TSolver::SolveQuadratic( 1e-18, 2.0, -6.0, roots ) && Near( roots[ 0 ], 3.0, 1e-12 ) );

        // x^2 - 1e8 x + 1: the small root keeps its digits: 1e-8 with a relative error of at most 1e-15
        ASSERT_TRUE( 2 == TSolver::SolveQuadratic( 1.0, -1e8, 1.0, roots ) );
        ASSERT_TRUE( fabs( roots[ 0 ] - 1e-8 ) <= 1e-15 * 1e-8 );
        ASSERT_TRUE( fabs( roots[ 1 ] - 1e8 ) <= 1e-15 * 1e8 );
        // and with the other sign of b
        ASSERT_TRUE( 2 == TSolver::SolveQuadratic( 1.0, 1e8, 1.0, roots ) );
        ASSERT_TRUE( fabs( roots[ 1 ] + 1e-8 ) <= 1e-15 * 1e-8 );

        // roots that differ by less than the tolerance are one root, by default 8 sqrt( epsilon ) relative
        ASSERT_TRUE( 2 == TSolver::SolveQuadratic( 1.0, -2.001, 1.001, roots ) );                  // 1 and 1.001
        ASSERT_TRUE( Near( roots[ 0 ], 1.0, 1e-12 ) && Near( roots[ 1 ], 1.001, 1e-12 ) );
        ASSERT_TRUE( 1 == TSolver::SolveQuadratic( 1.0, -2.001, 1.001, roots, 0.01 ) );            // merged at the mean
        ASSERT_TRUE( Near( roots[ 0 ], 1.0005, 1e-12 ) );
        ASSERT_TRUE( 1 == TSolver::SolveQuadratic( 1.0, -2.0000000001, 1.0000000001, roots ) );    // 1 and 1 + 1e-10
        ASSERT_TRUE( Near( roots[ 0 ], 1.0, 1e-7 ) );
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "PolynomialSolver: cubic cases, three roots, a triple root, one real root, a zero root" )
    {
        F64 roots[ 4 ];
        ASSERT_TRUE( 3 == TSolver::SolveCubic( 1.0, -6.0, 11.0, -6.0, roots ) );       // ( x - 1 ) ( x - 2 ) ( x - 3 )
        ASSERT_TRUE( Near( roots[ 0 ], 1.0, 1e-12 ) && Near( roots[ 1 ], 2.0, 1e-12 ) && Near( roots[ 2 ], 3.0, 1e-12 ) );
        ASSERT_TRUE( roots[ 0 ] < roots[ 1 ] && roots[ 1 ] < roots[ 2 ] );

        ASSERT_TRUE( 1 == TSolver::SolveCubic( 1.0, -3.0, 3.0, -1.0, roots ) );        // ( x - 1 )^3
        ASSERT_TRUE( Near( roots[ 0 ], 1.0, 1e-12 ) );

        ASSERT_TRUE( 2 == TSolver::SolveCubic( 1.0, -5.0, 8.0, -4.0, roots ) );        // ( x - 1 ) ( x - 2 )^2
        ASSERT_TRUE( Near( roots[ 0 ], 1.0, 1e-7 ) && Near( roots[ 1 ], 2.0, 1e-7 ) );

        ASSERT_TRUE( 1 == TSolver::SolveCubic( 1.0, 0.0, 1.0, 0.0, roots ) );          // x ( x^2 + 1 )
        ASSERT_TRUE( 0.0 == roots[ 0 ] );
        ASSERT_TRUE( 1 == TSolver::SolveCubic( 1.0, 0.0, 0.0, -8.0, roots ) );         // x^3 = 8
        ASSERT_TRUE( Near( roots[ 0 ], 2.0, 1e-14 ) );
        ASSERT_TRUE( 1 == TSolver::SolveCubic( 1.0, 0.0, 1.0, 2.0, roots ) );          // x^3 + x + 2 = ( x + 1 ) ( x^2 - x + 2 )
        ASSERT_TRUE( Near( roots[ 0 ], -1.0, 1e-14 ) );
        ASSERT_TRUE( 3 == TSolver::SolveCubic( 2.0, 0.0, -2.0, 0.0, roots ) );         // 2 x ( x - 1 ) ( x + 1 )
        ASSERT_TRUE( -1.0 == roots[ 0 ] && 0.0 == roots[ 1 ] && 1.0 == roots[ 2 ] );
        ASSERT_TRUE( 3 == TSolver::SolveCubic( -1.0, 6.0, -11.0, 6.0, roots ) );       // the negated polynomial has the same roots
        ASSERT_TRUE( Near( roots[ 0 ], 1.0, 1e-12 ) && Near( roots[ 2 ], 3.0, 1e-12 ) );
        ASSERT_TRUE( 3 == TSolver::SolveCubic( 1.0, 0.0, -3.0, 1.0, roots ) );         // three roots with an irreducible cubic: 2 cos( 2 pi k / 9 ) type
        ASSERT_TRUE( Near( roots[ 0 ] * roots[ 1 ] * roots[ 2 ], -1.0, 1e-13 ) && Near( roots[ 0 ] + roots[ 1 ] + roots[ 2 ], 0.0, 1e-13 ) );
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "PolynomialSolver: quartic cases, four roots, none, biquadratic, a zero root and double roots" )
    {
        F64 roots[ 4 ];
        ASSERT_TRUE( 4 == TSolver::SolveQuartic( 1.0, -10.0, 35.0, -50.0, 24.0, roots ) );       // ( x - 1 ) ( x - 2 ) ( x - 3 ) ( x - 4 )
        ASSERT_TRUE( Near( roots[ 0 ], 1.0, 1e-11 ) && Near( roots[ 1 ], 2.0, 1e-11 ) && Near( roots[ 2 ], 3.0, 1e-11 ) && Near( roots[ 3 ], 4.0, 1e-11 ) );

        ASSERT_TRUE( 0 == TSolver::SolveQuartic( 1.0, 0.0, 0.0, 0.0, 1.0, roots ) );             // x^4 + 1
        ASSERT_TRUE( 0 == TSolver::SolveQuartic( 1.0, 0.0, 2.0, 0.0, 5.0, roots ) );             // ( x^2 + 1 )^2 + 4
        ASSERT_TRUE( 0 == TSolver::SolveQuartic( 1.0, 1.0, 1.0, 1.0, 1.0, roots ) );             // the fifth roots of unity but 1
        ASSERT_TRUE( 4 == TSolver::SolveQuartic( 1.0, 0.0, -5.0, 0.0, 4.0, roots ) );            // ( x^2 - 1 ) ( x^2 - 4 )
        ASSERT_TRUE( -2.0 == roots[ 0 ] && -1.0 == roots[ 1 ] && 1.0 == roots[ 2 ] && 2.0 == roots[ 3 ] );
        ASSERT_TRUE( 2 == TSolver::SolveQuartic( 1.0, 0.0, 0.0, 0.0, -16.0, roots ) );           // x^4 = 16 -> +-2
        ASSERT_TRUE( Near( roots[ 0 ], -2.0, 1e-13 ) && Near( roots[ 1 ], 2.0, 1e-13 ) );
        ASSERT_TRUE( 1 == TSolver::SolveQuartic( 1.0, 0.0, 0.0, 0.0, 0.0, roots ) && 0.0 == roots[ 0 ] );   // x^4

        ASSERT_TRUE( 4 == TSolver::SolveQuartic( 1.0, -6.0, 11.0, -6.0, 0.0, roots ) );          // x ( x - 1 ) ( x - 2 ) ( x - 3 )
        ASSERT_TRUE( 0.0 == roots[ 0 ] && Near( roots[ 1 ], 1.0, 1e-12 ) && Near( roots[ 3 ], 3.0, 1e-12 ) );
        ASSERT_TRUE( 3 == TSolver::SolveQuartic( 1.0, -4.0, 5.0, -2.0, 0.0, roots ) );           // x ( x - 1 )^2 ( x - 2 )
        ASSERT_TRUE( 0.0 == roots[ 0 ] && Near( roots[ 1 ], 1.0, 1e-6 ) && Near( roots[ 2 ], 2.0, 1e-6 ) );

        ASSERT_TRUE( 1 == TSolver::SolveQuartic( 1.0, -4.0, 6.0, -4.0, 1.0, roots ) );           // ( x - 1 )^4
        ASSERT_TRUE( Near( roots[ 0 ], 1.0, 1e-6 ) );
        ASSERT_TRUE( 2 == TSolver::SolveQuartic( 1.0, -6.0, 13.0, -12.0, 4.0, roots ) );         // ( x - 1 )^2 ( x - 2 )^2
        ASSERT_TRUE( Near( roots[ 0 ], 1.0, 1e-6 ) && Near( roots[ 1 ], 2.0, 1e-6 ) );
        ASSERT_TRUE( 1 == TSolver::SolveQuartic( 1.0, -2.0, 2.0, -2.0, 1.0, roots ) );           // ( x - 1 )^2 ( x^2 + 1 )
        ASSERT_TRUE( Near( roots[ 0 ], 1.0, 1e-6 ) );
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
PerformSeededTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "PolynomialSolver: 10^4 polynomials built from seeded real roots in [ -100, 100 ] recover them, residual at most 1e-9 relative" )
    {
        MATH::CPcg32 random( 2024, 7 );
        F64 worstResidual = 0.0;
        F64 worstMiss = 0.0;
        bool ok = true;
        for ( int n = 0; n < 10000 && ok; ++n )
        {
            const int degree = 2 + n % 3;
            F64 truth[ 4 ];
            // roots at least 1 apart: closer roots are ill conditioned by the problem itself, not by the method
            for ( int i = 0; i < degree; ++i )
            {
                bool separated = false;
                while ( !separated )
                {
                    truth[ i ] = ( (F64) random.NextUInt32() / 2147483648.0 - 1.0 ) * 100.0;
                    separated = true;
                    for ( int j = 0; j < i; ++j )
                        if ( fabs( truth[ i ] - truth[ j ] ) < 1.0 )
                            separated = false;
                }
            }
            const F64 lead = ( 0 == n % 2 ? 1.0 : -2.5 );
            F64 c[ 5 ];
            Expand( truth, degree, lead, c );

            F64 found[ 4 ];
            UInt32 count = 0;
            if ( 2 == degree ) count = TSolver::SolveQuadratic( c[ 0 ], c[ 1 ], c[ 2 ], found );
            else if ( 3 == degree ) count = TSolver::SolveCubic( c[ 0 ], c[ 1 ], c[ 2 ], c[ 3 ], found );
            else count = TSolver::SolveQuartic( c[ 0 ], c[ 1 ], c[ 2 ], c[ 3 ], c[ 4 ], found );

            ok = (int) count == degree;
            for ( UInt32 i = 0; ok && i < count; ++i )
            {
                if ( i > 0 && !( found[ i - 1 ] < found[ i ] ) )
                    ok = false;
                const F64 residual = RelativeResidual( c, degree, found[ i ] );
                if ( residual > worstResidual )
                    worstResidual = residual;
                if ( residual > 1e-9 )
                    ok = false;
                F64 nearest = 1e300;
                for ( int j = 0; j < degree; ++j )
                    if ( fabs( found[ i ] - truth[ j ] ) < nearest )
                        nearest = fabs( found[ i ] - truth[ j ] );
                if ( nearest > worstMiss )
                    worstMiss = nearest;
                if ( nearest > 1e-8 )
                    ok = false;
            }
            if ( !ok )
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "PolynomialSolver: seeded polynomial " << n << " (degree " << degree << ", stream seed 2024 / 7) was not solved" );
        }
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "PolynomialSolver: worst relative residual " << worstResidual << ", worst distance to a true root " << worstMiss );
        ASSERT_TRUE( ok );
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "PolynomialSolver: polynomials with complex pairs report exactly the real roots, in Float64 and Float32" )
    {
        MATH::CPcg32 random( 31, 9 );
        bool ok = true;
        for ( int n = 0; n < 2000 && ok; ++n )
        {
            // ( x - r1 ) ( x - r2 ) ( x^2 + s x + t ) with a negative discriminant: two real roots
            const F64 r1 = ( (F64) random.NextUInt32() / 2147483648.0 - 1.0 ) * 20.0;
            const F64 r2 = r1 + 1.0 + (F64) random.NextUInt32() / 4294967296.0 * 15.0;
            const F64 s = ( (F64) random.NextUInt32() / 2147483648.0 - 1.0 ) * 10.0;
            const F64 t = s * s / 4.0 + 1.0 + (F64) random.NextUInt32() / 4294967296.0 * 10.0;
            const F64 lin[ 3 ] = { 1.0, -( r1 + r2 ), r1 * r2 };
            const F64 quad[ 3 ] = { 1.0, s, t };
            F64 c[ 5 ];
            for ( int i = 0; i < 5; ++i ) c[ i ] = 0.0;
            for ( int i = 0; i < 3; ++i )
                for ( int j = 0; j < 3; ++j )
                    c[ i + j ] += lin[ i ] * quad[ j ];
            F64 found[ 4 ];
            const UInt32 count = TSolver::SolveQuartic( c[ 0 ], c[ 1 ], c[ 2 ], c[ 3 ], c[ 4 ], found );
            ok = 2 == count && Near( found[ 0 ], r1, 1e-7 ) && Near( found[ 1 ], r2, 1e-7 );
            if ( !ok )
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "PolynomialSolver: complex pair case " << n << " failed (stream seed 31 / 9)" );
        }
        ASSERT_TRUE( ok );

        // Float32 with its own tolerance
        MATH::Float32 roots32[ 4 ];
        ASSERT_TRUE( 3 == TSolver32::SolveCubic( 1.0f, -6.0f, 11.0f, -6.0f, roots32 ) );
        ASSERT_TRUE( fabs( roots32[ 0 ] - 1.0f ) < 1e-4f && fabs( roots32[ 1 ] - 2.0f ) < 1e-4f && fabs( roots32[ 2 ] - 3.0f ) < 1e-4f );
        ASSERT_TRUE( 2 == TSolver32::SolveQuadratic( 1.0f, -3.0f, 2.0f, roots32 ) && 1.0f == roots32[ 0 ] && 2.0f == roots32[ 1 ] );
        ASSERT_TRUE( 4 == TSolver32::SolveQuartic( 1.0f, -10.0f, 35.0f, -50.0f, 24.0f, roots32 ) );
        ASSERT_TRUE( fabs( roots32[ 0 ] - 1.0f ) < 1e-3f && fabs( roots32[ 3 ] - 4.0f ) < 1e-3f );
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
PerformPolynomialSolverTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING PolynomialSolver TESTS" );

    PerformFixedCaseTests();
    PerformSeededTests();

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "PolynomialSolver TESTS COMPLETED" );
}

/*-------------------------------------------------------------------------*/
