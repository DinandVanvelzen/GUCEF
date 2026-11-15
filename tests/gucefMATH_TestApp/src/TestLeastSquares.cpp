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

#include <float.h>
#include <math.h>
#include <string.h>

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_MATH_VECTORS_H
#include "gucefMATH_vectors.h"
#define GUCEF_MATH_VECTORS_H
#endif /* GUCEF_MATH_VECTORS_H ? */

#ifndef GUCEF_MATH_DIRECTIONS_H
#include "gucefMATH_directions.h"
#define GUCEF_MATH_DIRECTIONS_H
#endif /* GUCEF_MATH_DIRECTIONS_H ? */

#ifndef GUCEF_MATH_QUATERNIONS_H
#include "gucefMATH_quaternions.h"
#define GUCEF_MATH_QUATERNIONS_H
#endif /* GUCEF_MATH_QUATERNIONS_H ? */

#ifndef GUCEF_MATH_POINTS_H
#include "gucefMATH_points.h"
#define GUCEF_MATH_POINTS_H
#endif /* GUCEF_MATH_POINTS_H ? */

#ifndef GUCEF_MATH_CTLEASTSQUARES_H
#include "gucefMATH_CTLeastSquares.h"
#define GUCEF_MATH_CTLEASTSQUARES_H
#endif /* GUCEF_MATH_CTLEASTSQUARES_H ? */

#include "TestMathReference.h"
#include "TestMathAllocationCounter.h"
#include "TestLeastSquares.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

using namespace GUCEF;
using namespace GUCEF::MATH;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The distance of point p from the line through 'origin' along the unit 'direction' */
static Float64
LineDistance( const TFloat64Point3D& p, const TFloat64Point3D& origin, const TFloat64DirectionVector3D& direction )
{
    const TFloat64Vector3D d = p - origin;
    return d.Cross( direction.AsVector() ).GetLength();
}

/*-------------------------------------------------------------------------*/

/** Fits from a realtime context: a RealtimeSanitizer build reports any allocation */
static Float32
RealtimeFitWork( const TFloat32Point3D* points, UInt32 count ) GUCEF_MATH_REALTIME_CONTEXT
{
    TFloat32Point3D origin;
    TFloat32DirectionVector3D direction;
    TFloat32Point3D centre;
    TFloat32DirectionVector3D normal;
    bool ok = TFloat32LeastSquares::FitLine3D( points, count, origin, direction );
    ok = TFloat32LeastSquares::FitPlane( points, count, centre, normal ) && ok;
    return ( ok ? 1.0f : 0.0f ) + origin.x + direction.GetX() + centre.y + normal.GetZ();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformLeastSquaresTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "LeastSquares" );

    GUCEF_TESTFW_TESTCASE( "LeastSquares: line and plane fits are exact for exact data, degenerate input is rejected" )
    try
    {
        const UInt64 seed = 0x4B60;
        MathReference::CSeededSequence generator( seed );
        UInt32 failures = 0;
        for ( UInt32 i=0; i<200; ++i )
        {
            const Float64 ox = generator.NextFloat64( -100, 100 );
            const Float64 oy = generator.NextFloat64( -100, 100 );
            const Float64 oz = generator.NextFloat64( -100, 100 );
            const Float64 dx = generator.NextFloat64( -1, 1 );
            const Float64 dy = generator.NextFloat64( -1, 1 );
            const Float64 dz = generator.NextFloat64( -1, 1 );
            const Float64 ex = generator.NextFloat64( -1, 1 );
            const Float64 ey = generator.NextFloat64( -1, 1 );
            const Float64 ez = generator.NextFloat64( -1, 1 );
            const TFloat64Point3D origin( ox, oy, oz );
            const TFloat64DirectionVector3D u( TFloat64Vector3D( dx, dy, dz ) );
            const TFloat64DirectionVector3D w( TFloat64Vector3D( ex, ey, ez ) );
            const TFloat64Vector3D v = w.AsVector() - u.AsVector() * u.Dot( w.AsVector() );
            // points on the line, then on the plane spanned by u and v
            TFloat64Point3D linePoints[ 12 ];
            TFloat64Point3D planePoints[ 12 ];
            for ( UInt32 k=0; k<12; ++k )
            {
                const Float64 s = generator.NextFloat64( -10, 10 );
                const Float64 t = generator.NextFloat64( -10, 10 );
                linePoints[ k ] = origin + u.AsVector() * s;
                planePoints[ k ] = origin + u.AsVector() * s + v * t;
            }
            TFloat64Point3D onLine;
            TFloat64DirectionVector3D direction;
            if ( !TFloat64LeastSquares::FitLine3D( linePoints, 12, onLine, direction ) )
                ++failures;
            else if ( !( LineDistance( onLine, origin, u ) <= 1e-12 * 200.0 ) || !( direction.AsVector().Cross( u.AsVector() ).GetLength() <= 1e-12 ) )
                ++failures;
            TFloat64Point3D onPlane;
            TFloat64DirectionVector3D normal;
            if ( !TFloat64LeastSquares::FitPlane( planePoints, 12, onPlane, normal ) )
                ++failures;
            else
            {
                // the normal is perpendicular to the plane, the centroid on it
                const TFloat64Vector3D expectedNormal = u.AsVector().Cross( v ).Normalized();
                if ( !( normal.AsVector().Cross( expectedNormal ).GetLength() <= 1e-12 ) || !( fabs( ( onPlane - origin ).Dot( expectedNormal ) ) <= 1e-12 * 200.0 ) )
                    ++failures;
            }
        }
        ASSERT_TRUE( 0 == failures );
        if ( 0 != failures )
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "LeastSquares: " << failures << " line or plane fits failed, seed " << seed );

        // a known fit: points on the z = 0 plane and the x axis
        const TFloat64Point3D square[ 4 ] = { TFloat64Point3D( 0, 0, 0 ), TFloat64Point3D( 2, 0, 0 ), TFloat64Point3D( 2, 2, 0 ), TFloat64Point3D( 0, 2, 0 ) };
        TFloat64Point3D centre;
        TFloat64DirectionVector3D normal;
        ASSERT_TRUE( TFloat64LeastSquares::FitPlane( square, 4, centre, normal ) && TFloat64Point3D( 1, 1, 0 ) == centre && 1.0 == fabs( normal.GetZ() ) );
        TFloat64Point3D onLine;
        TFloat64DirectionVector3D direction;
        ASSERT_TRUE( TFloat64LeastSquares::FitLine3D( square, 2, onLine, direction ) && TFloat64Point3D( 1, 0, 0 ) == onLine && 1.0 == fabs( direction.GetX() ) );
        // degenerate input: outputs unchanged
        const TFloat64Point3D same[ 3 ] = { TFloat64Point3D( 1, 2, 3 ), TFloat64Point3D( 1, 2, 3 ), TFloat64Point3D( 1, 2, 3 ) };
        const TFloat64Point3D collinear[ 3 ] = { TFloat64Point3D( 0, 0, 0 ), TFloat64Point3D( 1, 1, 1 ), TFloat64Point3D( 3, 3, 3 ) };
        TFloat64Point3D keep( 7, 7, 7 );
        ASSERT_FALSE( TFloat64LeastSquares::FitLine3D( same, 1, keep, direction ) );
        ASSERT_FALSE( TFloat64LeastSquares::FitLine3D( same, 3, keep, direction ) );
        ASSERT_FALSE( TFloat64LeastSquares::FitPlane( same, 2, keep, normal ) );
        ASSERT_FALSE( TFloat64LeastSquares::FitPlane( collinear, 3, keep, normal ) );
        ASSERT_TRUE( TFloat64Point3D( 7, 7, 7 ) == keep );
        ASSERT_TRUE( TFloat64LeastSquares::FitLine3D( collinear, 3, keep, direction ) && fabs( direction.GetX() - direction.GetY() ) < 1e-15 );
        TFloat64Point3D broken[ 3 ] = { TFloat64Point3D( 0, 0, 0 ), TFloat64Point3D( 1, 0, 0 ), TFloat64Point3D( 0, 1, 0 ) };
        broken[ 1 ].y = MathQuietNaN64();
        ASSERT_FALSE( TFloat64LeastSquares::FitPlane( broken, 3, keep, normal ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "LeastSquares: FitPolynomial recovers quadratic coefficients within 1e-10 and the least-squares line of noisy data" )
    try
    {
        TFloat64LeastSquares fit;
        // y = 1.5 - 2 x + 0.25 x^2 at x = -3 ... 3
        Float64 xs[ 13 ];
        Float64 ys[ 13 ];
        for ( UInt32 i=0; i<13; ++i )
        {
            xs[ i ] = -3.0 + 0.5 * (Float64) i;
            ys[ i ] = 1.5 - 2.0 * xs[ i ] + 0.25 * xs[ i ] * xs[ i ];
        }
        Float64 coefficients[ 3 ] = { 0, 0, 0 };
        ASSERT_TRUE( fit.FitPolynomial( xs, ys, 13, 2, coefficients ) );
        ASSERT_TRUE( fabs( coefficients[ 0 ] - 1.5 ) <= 1e-10 && fabs( coefficients[ 1 ] + 2.0 ) <= 1e-10 && fabs( coefficients[ 2 ] - 0.25 ) <= 1e-10 );
        // a higher degree recovers a zero cubic term
        Float64 cubic[ 4 ];
        ASSERT_TRUE( fit.FitPolynomial( xs, ys, 13, 3, cubic ) && fabs( cubic[ 3 ] ) <= 1e-10 && fabs( cubic[ 2 ] - 0.25 ) <= 1e-10 );
        // the line through ( 0, 0 ), ( 1, 1 ), ( 2, 1 ), ( 3, 3 ): least squares y = -0.1 + 0.9 x (closed form)
        const Float64 lx[ 4 ] = { 0, 1, 2, 3 };
        const Float64 ly[ 4 ] = { 0, 1, 1, 3 };
        Float64 line[ 2 ];
        ASSERT_TRUE( fit.FitPolynomial( lx, ly, 4, 1, line ) && fabs( line[ 0 ] + 0.1 ) <= 1e-14 && fabs( line[ 1 ] - 0.9 ) <= 1e-14 );
        // too few points, repeated x values: false, coefficients unchanged
        Float64 keep[ 3 ] = { 9, 9, 9 };
        ASSERT_FALSE( fit.FitPolynomial( xs, ys, 2, 2, keep ) );
        const Float64 repeatedX[ 4 ] = { 1, 1, 2, 2 };
        ASSERT_FALSE( fit.FitPolynomial( repeatedX, ly, 4, 2, keep ) );
        ASSERT_TRUE( 9 == keep[ 0 ] && 9 == keep[ 2 ] );
        // Float32
        TFloat32LeastSquares fit32;
        Float32 xs32[ 13 ];
        Float32 ys32[ 13 ];
        for ( UInt32 i=0; i<13; ++i )
        {
            xs32[ i ] = (Float32) xs[ i ];
            ys32[ i ] = (Float32) ys[ i ];
        }
        Float32 c32[ 3 ];
        ASSERT_TRUE( fit32.FitPolynomial( xs32, ys32, 13, 2, c32 ) && fabs( c32[ 0 ] - 1.5f ) <= 1e-5f && fabs( c32[ 2 ] - 0.25f ) <= 1e-5f );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "LeastSquares: the normal equations agree with QR and reject mismatched or rank-deficient systems" )
    try
    {
        const UInt64 seed = 0x4B61;
        MathReference::CSeededSequence generator( seed );
        CTDynamicMatrix< Float64 > a( 9, 4 );
        CTDynamicVector< Float64 > b( 9 );
        for ( UInt32 r=0; r<9; ++r )
        {
            for ( UInt32 c=0; c<4; ++c )
            {
                const Float64 value = generator.NextFloat64( -1, 1 );
                a( r, c ) = value;
            }
            const Float64 value = generator.NextFloat64( -1, 1 );
            b[ r ] = value;
        }
        TFloat64LeastSquares fit;
        CTDynamicVector< Float64 > normal;
        ASSERT_TRUE( fit.SolveNormalEquations( a, b, normal ) && 4 == normal.GetSize() );
        CTQrDecomposition< Float64 > qr;
        CTDynamicVector< Float64 > reference;
        ASSERT_TRUE( qr.Decompose( a ) && qr.SolveLeastSquares( b, reference ) );
        for ( UInt32 k=0; k<4; ++k )
            ASSERT_TRUE( fabs( normal[ k ] - reference[ k ] ) <= 1e-10 );
        CTDynamicVector< Float64 > keep( 2 );
        ASSERT_FALSE( fit.SolveNormalEquations( a, CTDynamicVector< Float64 >( 8 ), keep ) );
        CTDynamicMatrix< Float64 > deficient = a;
        for ( UInt32 r=0; r<9; ++r )
            deficient( r, 3 ) = 2.0 * deficient( r, 1 );
        ASSERT_FALSE( fit.SolveNormalEquations( deficient, b, keep ) );
        ASSERT_TRUE( 2 == keep.GetSize() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "LeastSquares: line and plane fits run in a realtime context" )
    try
    {
        const TFloat32Point3D points[ 4 ] = { TFloat32Point3D( 0, 0, 1 ), TFloat32Point3D( 2, 0, 1 ), TFloat32Point3D( 2, 3, 1 ), TFloat32Point3D( 0, 3, 1 ) };
        Float32 result = RealtimeFitWork( points, 4 );
        ASSERT_TRUE( CTNumericTraits< Float32 >::IsFinite( result ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "LeastSquares: realtime-safe and right-sized operations do not allocate" )
    GUCEF_TESTFW_SKIP_IF( !MathAllocationCounter::IsAvailable(), "no allocation counter with a sanitizer that replaces the allocator (the RealtimeSanitizer build proves the same in a realtime context)" );
    try
    {
        // the counter counts: one probe allocation
        MathAllocationCounter::StartCounting();
        MathAllocationCounter::AllocateProbe();
        ASSERT_TRUE( 1 == MathAllocationCounter::StopCounting() );
        const TFloat32Point3D points[ 4 ] = { TFloat32Point3D( 0, 0, 1 ), TFloat32Point3D( 2, 0, 1 ), TFloat32Point3D( 2, 3, 1 ), TFloat32Point3D( 0, 3, 1 ) };
        Float64 xs[ 20 ];
        Float64 ys[ 20 ];
        for ( UInt32 i=0; i<20; ++i )
        {
            xs[ i ] = 0.1 * (Float64) i;
            ys[ i ] = 1.0 + xs[ i ] * xs[ i ];
        }
        Float64 coefficients[ 4 ];
        TFloat64LeastSquares fit;
        CTDynamicMatrix< Float64 > a( 6, 3 );
        CTDynamicVector< Float64 > b( 6 );
        for ( UInt32 i=0; i<6; ++i )
        {
            a( i, 0 ) = 1.0;
            a( i, 1 ) = (Float64) i;
            a( i, 2 ) = (Float64) ( i * i );
            b[ i ] = (Float64) i;
        }
        CTDynamicVector< Float64 > x;
        // the first calls size the work buffers
        ASSERT_TRUE( fit.FitPolynomial( xs, ys, 20, 3, coefficients ) && fit.SolveNormalEquations( a, b, x ) );
        bool ok = true;
        MathAllocationCounter::StartCounting();
        Float32 result = RealtimeFitWork( points, 4 );
        for ( UInt32 i=0; i<10; ++i )
            ok = fit.FitPolynomial( xs, ys, 20, 3, coefficients ) && fit.SolveNormalEquations( a, b, x ) && ok;
        const UInt32 allocations = MathAllocationCounter::StopCounting();
        ASSERT_TRUE( 0 == allocations && ok && CTNumericTraits< Float32 >::IsFinite( result ) );
        ASSERT_TRUE( fabs( coefficients[ 2 ] - 1.0 ) < 1e-10 && fabs( x[ 1 ] - 1.0 ) < 1e-10 );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
