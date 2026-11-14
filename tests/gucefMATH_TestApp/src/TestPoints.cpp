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

#ifndef GUCEF_MATH_FRACTIONTYPES_H
#include "gucefMATH_fractiontypes.h"
#define GUCEF_MATH_FRACTIONTYPES_H
#endif /* GUCEF_MATH_FRACTIONTYPES_H ? */

#ifndef GUCEF_MATH_DIRECTIONS_H
#include "gucefMATH_directions.h"
#define GUCEF_MATH_DIRECTIONS_H
#endif /* GUCEF_MATH_DIRECTIONS_H ? */

#include "TestMathReference.h"
#include "TestPoints.h"

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

/*
 *  Overload probe: the size of the result tells which type an expression has, so the test proves that point - point
 *  is a vector and point + vector is a point without relying on conversions.
 */
typedef char TIsVector[ 1 ];
typedef char TIsPoint[ 2 ];
template < typename T > static TIsVector& TypeProbe( const CTVector3D< T >& );
template < typename T > static TIsPoint& TypeProbe( const CTPoint3D< T >& );
template < typename T > static TIsVector& TypeProbe2D( const CTVector2D< T >& );
template < typename T > static TIsPoint& TypeProbe2D( const CTPoint2D< T >& );

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformPointsTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "Points" );

    GUCEF_TESTFW_TESTCASE( "Points: affine semantics, point - point is a vector, point +/- vector is a point" )
    try
    {
        typedef TFloat32Point3D P;
        typedef TFloat32Vector3D V;
        P p1( 1, 2, 3 );
        P p2( 4, 6, 3 );
        ASSERT_TRUE( sizeof( TIsVector ) == sizeof( TypeProbe( p2 - p1 ) ) );
        ASSERT_TRUE( sizeof( TIsPoint ) == sizeof( TypeProbe( p1 + V( 1, 1, 1 ) ) ) );
        ASSERT_TRUE( sizeof( TIsPoint ) == sizeof( TypeProbe( p1 - V( 1, 1, 1 ) ) ) );
        ASSERT_TRUE( sizeof( TIsVector ) == sizeof( TypeProbe2D( TFloat32Point2D( 1, 2 ) - TFloat32Point2D( 0, 0 ) ) ) );
        ASSERT_TRUE( sizeof( TIsPoint ) == sizeof( TypeProbe2D( TFloat32Point2D( 1, 2 ) + TFloat32Vector2D( 0, 0 ) ) ) );

        ASSERT_TRUE( p2 - p1 == V( 3, 4, 0 ) );
        ASSERT_TRUE( p1 + V( 3, 4, 0 ) == p2 && p2 - V( 3, 4, 0 ) == p1 );
        P p3 = p1;
        p3 += V( 3, 4, 0 );  ASSERT_TRUE( p3 == p2 );
        p3 -= V( 3, 4, 0 );  ASSERT_TRUE( p3 == p1 );
        ASSERT_TRUE( p1 != p2 );
        ASSERT_TRUE( P() == P::Origin() && P::Origin() == P( 0, 0, 0 ) );
        ASSERT_TRUE( P( V( 7, 8, 9 ) ) == P( 7, 8, 9 ) && P( 7, 8, 9 ).ToPositionVector() == V( 7, 8, 9 ) );
        ASSERT_TRUE( 5.0f == p1.GetDistance( p2 ) && 25.0f == p1.GetSquaredDistance( p2 ) );
        ASSERT_TRUE( p1.XY() == TFloat32Point2D( 1, 2 ) );
        ASSERT_TRUE( TFloat32Point2D( 1, 2 ).ToPoint3D( 5 ) == P( 1, 2, 5 ) );
        ASSERT_TRUE( p1.Min( P( 0, 5, 3 ) ) == P( 0, 2, 3 ) && p1.Max( P( 0, 5, 3 ) ) == P( 1, 5, 3 ) );
        ASSERT_TRUE( p1.Equals( P( 1 + 5e-7f, 2, 3 ) ) && !p1.Equals( P( 1.001f, 2, 3 ) ) );
        ASSERT_TRUE( p1.EqualsUlp( P( nextafterf( 1.0f, 2.0f ), 2, 3 ), 1 ) && p1.IsFinite() );
        for ( UInt32 i=0; i<3; ++i )
        {
            P p;
            p.SetComponent( i, 4.0f );
            ASSERT_TRUE( 4.0f == p.GetComponent( i ) && 16.0f == p.GetSquaredDistance( P::Origin() ) );
        }
        for ( UInt32 i=0; i<2; ++i )
        {
            TFloat32Point2D p;
            p.SetComponent( i, 3.0f );
            ASSERT_TRUE( 3.0f == p.GetComponent( i ) && 3.0f == p.GetDistance( TFloat32Point2D::Origin() ) );
        }
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Points: Lerp, Midpoint, Centroid and AffineCombination" )
    try
    {
        typedef TFloat32Point3D P;
        P a( 0, 0, 0 );
        P b( 2, 4, 6 );
        ASSERT_TRUE( P::Lerp( a, b, 0.25f ) == P( 0.5f, 1, 1.5f ) );
        ASSERT_TRUE( P::Midpoint( a, b ) == P( 1, 2, 3 ) );
        const P points[ 4 ] = { P( 0, 0, 0 ), P( 4, 0, 0 ), P( 0, 4, 0 ), P( 0, 0, 4 ) };
        ASSERT_TRUE( P::Centroid( points, 4 ) == P( 1, 1, 1 ) );
        ASSERT_TRUE( P::Centroid( points, 1 ) == P( 0, 0, 0 ) );
        const Float32 weights[ 4 ] = { 0.25f, 0.25f, 0.25f, 0.25f };
        ASSERT_TRUE( P::AffineCombination( points, weights, 4 ) == P( 1, 1, 1 ) );
        const Float32 barycentric[ 3 ] = { 0.2f, 0.3f, 0.5f };
        ASSERT_TRUE( P::AffineCombination( points, barycentric, 3 ).Equals( P( 1.2f, 2.0f, 0 ) ) );

        // large coordinates: formed relative to the first point, so a far away cluster keeps its precision
        const P far[ 2 ] = { P( 1.0e6f, 0, 0 ), P( 1.0e6f + 1.0f, 0, 0 ) };
        ASSERT_TRUE( P::Centroid( far, 2 ) == P( 1.0e6f + 0.5f, 0, 0 ) );

        TFloat64Point2D q[ 3 ] = { TFloat64Point2D( 0, 0 ), TFloat64Point2D( 3, 0 ), TFloat64Point2D( 0, 3 ) };
        ASSERT_TRUE( TFloat64Point2D::Centroid( q, 3 ) == TFloat64Point2D( 1, 1 ) );
        ASSERT_TRUE( TFloat64Point2D::Midpoint( q[ 1 ], q[ 2 ] ) == TFloat64Point2D( 1.5, 1.5 ) );

        // exact rationals: the centroid of a triangle is exact
        typedef CORE::TInt64Fraction F;
        typedef TInt64FractionPoint3D FP;
        const FP triangle[ 3 ] = { FP( F( 0 ), F( 0 ), F( 0 ) ), FP( F( 1 ), F( 0 ), F( 0 ) ), FP( F( 0 ), F( 1 ), F( 0 ) ) };
        ASSERT_TRUE( FP::Centroid( triangle, 3 ) == FP( F( 1, 3 ), F( 1, 3 ), F( 0 ) ) );
        ASSERT_TRUE( FP::Midpoint( triangle[ 1 ], triangle[ 2 ] ) == FP( F( 1, 2 ), F( 1, 2 ), F( 0 ) ) );
        ASSERT_TRUE( F( 2 ) == triangle[ 1 ].GetSquaredDistance( triangle[ 2 ] ) );

        // integer points: distances squared exact, Lerp with integral t
        TInt32Point3D i1( 1, 2, 3 );
        TInt32Point3D i2( 4, 6, 3 );
        ASSERT_TRUE( 25 == i1.GetSquaredDistance( i2 ) && i2 - i1 == TInt32Vector3D( 3, 4, 0 ) );
        ASSERT_TRUE( TInt32Point3D::Lerp( i1, i2, 2 ) == TInt32Point3D( 7, 10, 3 ) );

        // fixed point
        TFixed32Point3D f1( TFixed32( 1 ), TFixed32( 1 ), TFixed32( 1 ) );
        TFixed32Point3D f2( TFixed32( 4 ), TFixed32( 5 ), TFixed32( 1 ) );
        ASSERT_TRUE( TFixed32( 5 ) == f1.GetDistance( f2 ) );
        ASSERT_TRUE( TFixed32Point3D::Midpoint( f1, f2 ) == TFixed32Point3D( TFixed32( 5 ) / TFixed32( 2 ), TFixed32( 3 ), TFixed32( 1 ) ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Points: seeded affine combinations equal the Float64 reference" )
    try
    {
        const UInt64 seed = 8675309;
        MathReference::CSeededSequence generator( seed );
        UInt32 failures = 0;
        for ( UInt32 i=0; i<10000; ++i )
        {
            TFloat64Point3D points[ 4 ];
            Float64 weights[ 4 ];
            Float64 sum = 0.0;
            for ( UInt32 k=0; k<4; ++k )
            {
                points[ k ] = TFloat64Point3D( generator.NextFloat64( -1000, 1000 ), generator.NextFloat64( -1000, 1000 ), generator.NextFloat64( -1000, 1000 ) );
                weights[ k ] = generator.NextFloat64( 0.0, 1.0 );
                sum += weights[ k ];
            }
            weights[ 3 ] = 1.0 - ( weights[ 0 ] + weights[ 1 ] + weights[ 2 ] );
            TFloat64Vector3D reference;
            for ( UInt32 k=0; k<4; ++k )
                reference += points[ k ].ToPositionVector() * weights[ k ];
            TFloat64Point3D combined = TFloat64Point3D::AffineCombination( points, weights, 4 );
            if ( !combined.Equals( TFloat64Point3D( reference ), 1e-9, 1e-12 ) )
                ++failures;
            (void) sum;
        }
        ASSERT_TRUE( 0 == failures );
        if ( 0 != failures )
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Points: affine combination seed " << seed << " failures " << failures );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
