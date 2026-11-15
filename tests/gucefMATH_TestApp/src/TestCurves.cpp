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

#ifndef GUCEF_CORE_CIDATANODESERIALIZABLE_H
#include "gucefCORE_CIDataNodeSerializable.h"
#define GUCEF_CORE_CIDATANODESERIALIZABLE_H
#endif /* GUCEF_CORE_CIDATANODESERIALIZABLE_H ? */

#ifndef GUCEF_MATH_CTBEZIERCURVE2_H
#include "gucefMATH_CTBezierCurve2.h"
#define GUCEF_MATH_CTBEZIERCURVE2_H
#endif /* GUCEF_MATH_CTBEZIERCURVE2_H ? */

#ifndef GUCEF_MATH_CTBEZIERCURVE3_H
#include "gucefMATH_CTBezierCurve3.h"
#define GUCEF_MATH_CTBEZIERCURVE3_H
#endif /* GUCEF_MATH_CTBEZIERCURVE3_H ? */

#ifndef GUCEF_MATH_CTHERMITESPLINE_H
#include "gucefMATH_CTHermiteSpline.h"
#define GUCEF_MATH_CTHERMITESPLINE_H
#endif /* GUCEF_MATH_CTHERMITESPLINE_H ? */

#ifndef GUCEF_MATH_CTCATMULLROMSPLINE_H
#include "gucefMATH_CTCatmullRomSpline.h"
#define GUCEF_MATH_CTCATMULLROMSPLINE_H
#endif /* GUCEF_MATH_CTCATMULLROMSPLINE_H ? */

#ifndef GUCEF_MATH_CTBSPLINE_H
#include "gucefMATH_CTBSpline.h"
#define GUCEF_MATH_CTBSPLINE_H
#endif /* GUCEF_MATH_CTBSPLINE_H ? */

#ifndef GUCEF_MATH_CTARCLENGTHTABLE_H
#include "gucefMATH_CTArcLengthTable.h"
#define GUCEF_MATH_CTARCLENGTHTABLE_H
#endif /* GUCEF_MATH_CTARCLENGTHTABLE_H ? */

#include "TestMathReference.h"
#include "TestCurves.h"

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

GUCEF_MATH_STATIC_ASSERT( CORE::TypeHasMemberFunctionForDataNodeSerialization< CTBezierCurve3< TFloat64Vector3D > >::value, Bezier3_has_Serialize );
GUCEF_MATH_STATIC_ASSERT( CORE::TypeHasMemberFunctionForDataNodeDeserialization< CTBSpline< TFloat32Point2D > >::value, BSpline_has_Deserialize );

/** Adaptive Simpson integral of f over [ a, b ] (independent reference for the Gauss-Legendre table) */
template < typename TFunction >
static Float64
SimpsonRecursive( const TFunction& f, Float64 a, Float64 b, Float64 fa, Float64 fm, Float64 fb, Float64 whole, Float64 tolerance, UInt32 depth )
{
    const Float64 m = 0.5 * ( a + b );
    const Float64 lm = 0.5 * ( a + m );
    const Float64 rm = 0.5 * ( m + b );
    const Float64 flm = f( lm );
    const Float64 frm = f( rm );
    const Float64 left = ( m - a ) / 6.0 * ( fa + 4.0 * flm + fm );
    const Float64 right = ( b - m ) / 6.0 * ( fm + 4.0 * frm + fb );
    if ( depth == 0 || fabs( left + right - whole ) <= 15.0 * tolerance )
        return left + right + ( left + right - whole ) / 15.0;
    return SimpsonRecursive( f, a, m, fa, flm, fm, left, 0.5 * tolerance, depth - 1 ) + SimpsonRecursive( f, m, b, fm, frm, fb, right, 0.5 * tolerance, depth - 1 );
}

template < typename TFunction >
static Float64
Simpson( const TFunction& f, Float64 a, Float64 b )
{
    const Float64 fa = f( a );
    const Float64 fb = f( b );
    const Float64 fm = f( 0.5 * ( a + b ) );
    const Float64 whole = ( b - a ) / 6.0 * ( fa + 4.0 * fm + fb );
    return SimpsonRecursive( f, a, b, fa, fm, fb, whole, 1e-14, 40 );
}

/** The speed of a curve as a function object for Simpson() */
template < typename TCurve >
struct SSpeed
{
    const TCurve& curve;
    explicit SSpeed( const TCurve& c ) : curve( c ) {}
    Float64 operator()( Float64 t ) const { return TCurve::TTraits::Length( curve.GetDerivative( t ) ); }
};

static Float64 Dist3( const TFloat64Point3D& a, const TFloat64Point3D& b ) { return ( a - b ).GetLength(); }
static Float64 Dist2( const TFloat64Point2D& a, const TFloat64Point2D& b ) { return ( a - b ).GetLength(); }

/** A 3D cubic with a mild S shape used by several tests */
static CTBezierCurve3< TFloat64Point3D >
MakeCubic( void )
{
    return CTBezierCurve3< TFloat64Point3D >( TFloat64Point3D( 0.0, 0.0, 0.0 ), TFloat64Point3D( 2.0, 3.0, -1.0 ), TFloat64Point3D( 4.0, -2.0, 2.0 ), TFloat64Point3D( 6.0, 1.0, 0.5 ) );
}

/** Round trips a curve through a DataNode */
template < typename TCurve >
static bool
DataNodeRoundTrip( const TCurve& curve, TCurve& back )
{
    CORE::CDataNodeSerializableSettings settings;
    CORE::CDataNode node( "curve", GUCEF_DATATYPE_OBJECT );
    return curve.Serialize( node, settings ) && back.Deserialize( node, settings );
}

/** Uniform cubic B-spline segment j with control points p[ j ] .. p[ j + 3 ] in matrix form, local parameter u */
static Float64
UniformCubicMatrixForm( const Float64* p, Float64 u )
{
    const Float64 u2 = u * u;
    const Float64 u3 = u2 * u;
    return ( p[ 0 ] * ( -u3 + 3.0 * u2 - 3.0 * u + 1.0 ) + p[ 1 ] * ( 3.0 * u3 - 6.0 * u2 + 4.0 ) + p[ 2 ] * ( -3.0 * u3 + 3.0 * u2 + 3.0 * u + 1.0 ) + p[ 3 ] * u3 ) / 6.0;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformCurvesTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING Curves TESTS" );
    GUCEF_TESTFW_SUITE_SCOPE( "Curves" );

    GUCEF_TESTFW_TESTCASE( "Curves: cubic Bezier endpoints, end derivatives and second derivative" )
        const CTBezierCurve3< TFloat64Point3D > c = MakeCubic();
        ASSERT_TRUE( c.Evaluate( 0.0 ) == c.p0 && c.Evaluate( 1.0 ) == c.p3 );
        const TFloat64Vector3D d0 = c.GetDerivative( 0.0 );
        const TFloat64Vector3D d1 = c.GetDerivative( 1.0 );
        ASSERT_TRUE( d0.Equals( ( c.p1 - c.p0 ) * 3.0, 1e-12, 1e-12 ) );
        ASSERT_TRUE( d1.Equals( ( c.p3 - c.p2 ) * 3.0, 1e-12, 1e-12 ) );
        // derivatives against central differences
        for ( Float64 t = 0.05; t < 0.96; t += 0.1 )
        {
            const Float64 h = 1e-5;
            const TFloat64Vector3D numeric = ( c.Evaluate( t + h ) - c.Evaluate( t - h ) ) * ( 0.5 / h );
            ASSERT_TRUE( numeric.Equals( c.GetDerivative( t ), 1e-6, 1e-6 ) );
            const TFloat64Vector3D numeric2 = ( c.GetDerivative( t + h ) - c.GetDerivative( t - h ) ) * ( 0.5 / h );
            ASSERT_TRUE( numeric2.Equals( c.GetSecondDerivative( t ), 1e-5, 1e-6 ) );
        }
        // Float32 and the other point types
        const CTBezierCurve3< Float32 > scalar( 0.0f, 1.0f, 2.0f, 3.0f );
        ASSERT_TRUE( fabs( scalar.Evaluate( 0.25f ) - 0.75f ) < 1e-6f && fabs( scalar.GetDerivative( 0.5f ) - 3.0f ) < 1e-5f );
        const CTBezierCurve3< TFloat32Vector2D > v2( TFloat32Vector2D( 0.0f, 0.0f ), TFloat32Vector2D( 1.0f, 2.0f ), TFloat32Vector2D( 3.0f, 2.0f ), TFloat32Vector2D( 4.0f, 0.0f ) );
        ASSERT_TRUE( fabs( v2.Evaluate( 0.5f ).x - 2.0f ) < 1e-6f && fabs( v2.Evaluate( 0.5f ).y - 1.5f ) < 1e-6f );
        const CTBezierCurve3< TFloat64Vector4D > v4( TFloat64Vector4D( 0, 0, 0, 0 ), TFloat64Vector4D( 1, 1, 1, 1 ), TFloat64Vector4D( 2, 2, 2, 2 ), TFloat64Vector4D( 3, 3, 3, 3 ) );
        ASSERT_TRUE( fabs( v4.Evaluate( 0.5 ).w - 1.5 ) < 1e-12 );
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Curves: de Casteljau Split reproduces the curve (cubic and quadratic)" )
        const CTBezierCurve3< TFloat64Point3D > c = MakeCubic();
        const Float64 splits[ 4 ] = { 0.0, 0.3, 0.5, 0.9 };
        for ( UInt32 s=0; s<4; ++s )
        {
            CTBezierCurve3< TFloat64Point3D > left, right;
            c.Split( splits[ s ], left, right );
            for ( Float64 u = 0.0; u <= 1.0; u += 0.125 )
            {
                ASSERT_TRUE( Dist3( left.Evaluate( u ), c.Evaluate( splits[ s ] * u ) ) < 1e-12 );
                ASSERT_TRUE( Dist3( right.Evaluate( u ), c.Evaluate( splits[ s ] + ( 1.0 - splits[ s ] ) * u ) ) < 1e-12 );
            }
            ASSERT_TRUE( left.p3 == right.p0 );
        }
        CTBezierCurve3< TFloat64Point3D > inPlaceLeft = c, inPlaceRight;
        inPlaceLeft.Split( 0.5, inPlaceLeft, inPlaceRight );
        ASSERT_TRUE( Dist3( inPlaceLeft.Evaluate( 1.0 ), c.Evaluate( 0.5 ) ) < 1e-12 );

        const CTBezierCurve2< TFloat64Point2D > q( TFloat64Point2D( 0.0, 0.0 ), TFloat64Point2D( 2.0, 4.0 ), TFloat64Point2D( 4.0, 0.0 ) );
        ASSERT_TRUE( q.Evaluate( 0.0 ) == q.p0 && q.Evaluate( 1.0 ) == q.p2 );
        ASSERT_TRUE( q.GetDerivative( 0.0 ).Equals( ( q.p1 - q.p0 ) * 2.0 ) && q.GetDerivative( 1.0 ).Equals( ( q.p2 - q.p1 ) * 2.0 ) );
        ASSERT_TRUE( q.GetSecondDerivative( 0.3 ).Equals( TFloat64Vector2D( 0.0, -16.0 ) ) );
        CTBezierCurve2< TFloat64Point2D > ql, qr;
        q.Split( 0.4, ql, qr );
        for ( Float64 u = 0.0; u <= 1.0; u += 0.125 )
        {
            ASSERT_TRUE( Dist2( ql.Evaluate( u ), q.Evaluate( 0.4 * u ) ) < 1e-12 );
            ASSERT_TRUE( Dist2( qr.Evaluate( u ), q.Evaluate( 0.4 + 0.6 * u ) ) < 1e-12 );
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Curves: Bezier bounds are tight (against dense sampling)" )
        const CTBezierCurve3< TFloat64Point3D > c = MakeCubic();
        TFloat64Point3D lo, hi;
        c.GetBounds( lo, hi );
        TFloat64Point3D sampleLo = c.Evaluate( 0.0 ), sampleHi = sampleLo;
        for ( UInt32 i=0; i<=20000; ++i )
        {
            const TFloat64Point3D p = c.Evaluate( i / 20000.0 );
            for ( UInt32 k=0; k<3; ++k )
            {
                if ( p.GetComponent( k ) < sampleLo.GetComponent( k ) ) sampleLo.SetComponent( k, p.GetComponent( k ) );
                if ( p.GetComponent( k ) > sampleHi.GetComponent( k ) ) sampleHi.SetComponent( k, p.GetComponent( k ) );
            }
        }
        for ( UInt32 k=0; k<3; ++k )
        {
            ASSERT_TRUE( lo.GetComponent( k ) <= sampleLo.GetComponent( k ) + 1e-12 && sampleLo.GetComponent( k ) - lo.GetComponent( k ) < 1e-6 );
            ASSERT_TRUE( hi.GetComponent( k ) >= sampleHi.GetComponent( k ) - 1e-12 && hi.GetComponent( k ) - sampleHi.GetComponent( k ) < 1e-6 );
        }
        // the control polygon is looser than the bounds
        ASSERT_TRUE( hi.y < 3.0 - 0.1 );
        const CTBezierCurve2< TFloat64Point2D > q( TFloat64Point2D( 0.0, 0.0 ), TFloat64Point2D( 2.0, 4.0 ), TFloat64Point2D( 4.0, 0.0 ) );
        TFloat64Point2D qlo, qhi;
        q.GetBounds( qlo, qhi );
        ASSERT_TRUE( qlo.x == 0.0 && qhi.x == 4.0 && qlo.y == 0.0 && fabs( qhi.y - 2.0 ) < 1e-12 );
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Curves: arc length of collinear and quarter circle Beziers" )
        {
            const CTBezierCurve3< TFloat64Point3D > line( TFloat64Point3D( 0, 0, 0 ), TFloat64Point3D( 1, 2, 2 ), TFloat64Point3D( 2, 4, 4 ), TFloat64Point3D( 3, 6, 6 ) );
            CTArcLengthTable< Float64 > table;
            ASSERT_TRUE( table.Build( line ) );
            ASSERT_TRUE( fabs( table.GetLength() - 9.0 ) < 1e-9 );    // chord ( 3, 6, 6 ) is 9 long
        }
        {
            const Float64 k = 0.5522847498;
            const CTBezierCurve3< TFloat64Point2D > arc( TFloat64Point2D( 1.0, 0.0 ), TFloat64Point2D( 1.0, k ), TFloat64Point2D( k, 1.0 ), TFloat64Point2D( 0.0, 1.0 ) );
            CTArcLengthTable< Float64 > table;
            ASSERT_TRUE( table.Build( arc ) );
            const Float64 reference = Simpson( SSpeed< CTBezierCurve3< TFloat64Point2D > >( arc ), 0.0, 1.0 );
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Curves: quarter circle cubic length " << table.GetLength() << " reference " << reference << " intervals " << table.GetIntervalCount() );
            ASSERT_TRUE( fabs( table.GetLength() - reference ) < 1e-9 );
            ASSERT_TRUE( fabs( reference - 1.5707963267948966 ) < 5e-4 );   // the cubic approximates the circle
        }
        {
            const CTBezierCurve2< TFloat64Point2D > q( TFloat64Point2D( 0.0, 0.0 ), TFloat64Point2D( 2.0, 4.0 ), TFloat64Point2D( 4.0, 0.0 ) );
            CTArcLengthTable< Float64 > table;
            ASSERT_TRUE( table.Build( q ) );
            ASSERT_TRUE( fabs( table.GetLength() - Simpson( SSpeed< CTBezierCurve2< TFloat64Point2D > >( q ), 0.0, 1.0 ) ) < 1e-9 );
        }
        {
            const CTBezierCurve3< TFloat64Point3D > c = MakeCubic();
            CTArcLengthTable< Float64 > table;
            ASSERT_TRUE( table.Build( c ) );
            ASSERT_TRUE( fabs( table.GetLength() - Simpson( SSpeed< CTBezierCurve3< TFloat64Point3D > >( c ), 0.0, 1.0 ) ) < 1e-9 );
            CTArcLengthTable< Float32 > table32;
            const CTBezierCurve3< TFloat32Point3D > c32( TFloat32Point3D( 0, 0, 0 ), TFloat32Point3D( 2, 3, -1 ), TFloat32Point3D( 4, -2, 2 ), TFloat32Point3D( 6, 1, 0.5f ) );
            ASSERT_TRUE( table32.Build( c32 ) && fabs( table32.GetLength() - table.GetLength() ) < 1e-3 );
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Curves: arc length table splits the length into halves and inverts" )
        const CTBezierCurve3< TFloat64Point3D > c = MakeCubic();
        CTArcLengthTable< Float64 > table;
        ASSERT_TRUE( table.Build( c ) );
        const Float64 length = table.GetLength();
        const Float64 mid = table.GetParameterAtDistance( c, 0.5 * length );
        const Float64 firstHalf = Simpson( SSpeed< CTBezierCurve3< TFloat64Point3D > >( c ), 0.0, mid );
        ASSERT_TRUE( fabs( firstHalf - 0.5 * length ) / length < 1e-6 );
        // inverse consistency
        for ( Float64 t = 0.0; t <= 1.0; t += 0.0625 )
        {
            const Float64 d = table.GetDistanceAtParameter( c, t );
            ASSERT_TRUE( fabs( table.GetParameterAtDistance( c, d ) - t ) < 1e-7 );
        }
        ASSERT_TRUE( table.GetParameterAtDistance( c, -1.0 ) == 0.0 && table.GetParameterAtDistance( c, 2.0 * length ) == 1.0 );
        ASSERT_TRUE( table.GetDistanceAtParameter( c, -1.0 ) == 0.0 && table.GetDistanceAtParameter( c, 2.0 ) == length );
        ASSERT_TRUE( table.GetBoundaryParameter( 0 ) == 0.0 && table.GetBoundaryDistance( table.GetIntervalCount() ) == length );
        // an empty table and a degenerate curve
        CTArcLengthTable< Float64 > empty;
        ASSERT_TRUE( !empty.IsValid() && 0.0 == empty.GetLength() && 0.0 == empty.GetParameterAtDistance( c, 1.0 ) );
        const CTBezierCurve3< TFloat64Point3D > point( TFloat64Point3D( 1, 1, 1 ), TFloat64Point3D( 1, 1, 1 ), TFloat64Point3D( 1, 1, 1 ), TFloat64Point3D( 1, 1, 1 ) );
        CTArcLengthTable< Float64 > degenerate;
        ASSERT_TRUE( degenerate.Build( point ) && 0.0 == degenerate.GetLength() );
        ASSERT_TRUE( 0.0 == degenerate.GetParameterAtDistance( point, 0.3 ) );
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Curves: Hermite spline interpolates its keys with the given tangents" )
        CTHermiteSpline< TFloat64Point3D > spline;
        spline.AddKey( TFloat64Point3D( 0, 0, 0 ), TFloat64Vector3D( 1, 2, 0 ) );
        spline.AddKey( TFloat64Point3D( 1, 1, 1 ), TFloat64Vector3D( 0, 3, 1 ) );
        spline.AddKey( TFloat64Point3D( 3, 0, 2 ), TFloat64Vector3D( 2, -1, 0 ), TFloat64Vector3D( 4, 0, 1 ) );
        spline.AddKey( TFloat64Point3D( 5, 5, 5 ), TFloat64Vector3D( 1, 1, 1 ) );
        ASSERT_TRUE( 4 == spline.GetKeyCount() && 3 == spline.GetSegmentCount() && 3.0 == spline.GetMaxParameter() );
        for ( UInt32 i=0; i<4; ++i )
            ASSERT_TRUE( spline.Evaluate( (Float64) i ) == spline.GetKey( i ).position );
        // derivative at a key equals its tangent (out tangent when leaving, in tangent when arriving)
        ASSERT_TRUE( spline.GetDerivative( 0.0 ).Equals( TFloat64Vector3D( 1, 2, 0 ) ) );
        ASSERT_TRUE( spline.GetDerivative( 1.0 ).Equals( TFloat64Vector3D( 0, 3, 1 ), 1e-12, 1e-12 ) );
        ASSERT_TRUE( spline.GetDerivative( 2.0 - 1e-12 ).Equals( TFloat64Vector3D( 2, -1, 0 ), 1e-9, 1e-9 ) );
        ASSERT_TRUE( spline.GetDerivative( 2.0 + 1e-12 ).Equals( TFloat64Vector3D( 4, 0, 1 ), 1e-9, 1e-9 ) );
        // second derivative against differences, inside a segment
        const Float64 h = 1e-5;
        const TFloat64Vector3D numeric = ( spline.GetDerivative( 0.5 + h ) - spline.GetDerivative( 0.5 - h ) ) * ( 0.5 / h );
        ASSERT_TRUE( numeric.Equals( spline.GetSecondDerivative( 0.5 ), 1e-5, 1e-6 ) );
        // clamping and the degenerate splines
        ASSERT_TRUE( spline.Evaluate( -5.0 ) == spline.GetKey( 0 ).position && spline.Evaluate( 99.0 ) == spline.GetKey( 3 ).position );
        CTHermiteSpline< TFloat64Point3D > empty;
        ASSERT_TRUE( TFloat64Point3D() == empty.Evaluate( 0.3 ) && empty.GetDerivative( 0.3 ).IsZero() );
        empty.AddKey( TFloat64Point3D( 1, 2, 3 ), TFloat64Vector3D( 1, 1, 1 ) );
        ASSERT_TRUE( TFloat64Point3D( 1, 2, 3 ) == empty.Evaluate( 7.0 ) && empty.GetDerivative( 0.0 ).IsZero() );
        ASSERT_TRUE( spline.RemoveKey( 1 ) && 3 == spline.GetKeyCount() && !spline.RemoveKey( 9 ) );
        ASSERT_TRUE( !spline.SetKey( 9, CTHermiteSpline< TFloat64Point3D >::SKey() ) );
        CTHermiteSpline< TFloat64Point3D > back;
        ASSERT_TRUE( DataNodeRoundTrip( spline, back ) && 3 == back.GetKeyCount() && back.Evaluate( 0.7 ) == spline.Evaluate( 0.7 ) );
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Curves: Catmull-Rom interpolates its points and is C1 at the joints for every parametrisation" )
        const TFloat64Point3D pts[ 6 ] = { TFloat64Point3D( 0, 0, 0 ), TFloat64Point3D( 1, 2, 0 ), TFloat64Point3D( 3, 3, 1 ), TFloat64Point3D( 3.5, 0, 2 ), TFloat64Point3D( 6, -1, 2 ), TFloat64Point3D( 8, 2, 5 ) };
        const TCatmullRomParametrisation kinds[ 3 ] = { CATMULLROM_UNIFORM, CATMULLROM_CENTRIPETAL, CATMULLROM_CHORDAL };
        for ( UInt32 k=0; k<3; ++k )
        {
            CTCatmullRomSpline< TFloat64Point3D > spline( kinds[ k ] );
            spline.SetPoints( pts, 6 );
            ASSERT_TRUE( 5 == spline.GetSegmentCount() && 0.0 == spline.GetKnot( 0 ) && spline.GetKnot( 5 ) == spline.GetMaxParameter() );
            for ( UInt32 i=0; i<6; ++i )
                ASSERT_TRUE( Dist3( spline.Evaluate( spline.GetKnot( i ) ), pts[ i ] ) < 1e-12 );
            for ( UInt32 i=1; i<5; ++i )
            {
                const Float64 knot = spline.GetKnot( i );
                const TFloat64Vector3D before = spline.GetDerivative( knot - 1e-9 );
                const TFloat64Vector3D after = spline.GetDerivative( knot + 1e-9 );
                ASSERT_TRUE( before.Equals( after, 1e-6, 1e-6 ) );
                // position continuity across the joint
                ASSERT_TRUE( Dist3( spline.Evaluate( knot - 1e-9 ), spline.Evaluate( knot + 1e-9 ) ) < 1e-7 );
            }
            // derivative against differences inside a segment
            const Float64 t = 0.5 * ( spline.GetKnot( 2 ) + spline.GetKnot( 3 ) );
            const TFloat64Vector3D numeric = ( spline.Evaluate( t + 1e-6 ) - spline.Evaluate( t - 1e-6 ) ) * ( 0.5e6 );
            ASSERT_TRUE( numeric.Equals( spline.GetDerivative( t ), 1e-6, 1e-6 ) );
        }
        // uniform: matches the textbook form 0.5 ( 2 p1 + ( p2 - p0 ) u + ( 2 p0 - 5 p1 + 4 p2 - p3 ) u^2 + ( 3 ( p1 - p2 ) + p3 - p0 ) u^3 )
        CTCatmullRomSpline< TFloat64Point3D > uniform( CATMULLROM_UNIFORM );
        uniform.SetPoints( pts, 6 );
        const Float64 u = 0.37;
        const TFloat64Point3D e = uniform.Evaluate( 2.0 + u );
        for ( UInt32 c=0; c<3; ++c )
        {
            const Float64 p0 = pts[ 1 ].GetComponent( c ), p1 = pts[ 2 ].GetComponent( c ), p2 = pts[ 3 ].GetComponent( c ), p3 = pts[ 4 ].GetComponent( c );
            const Float64 expected = 0.5 * ( 2.0 * p1 + ( p2 - p0 ) * u + ( 2.0 * p0 - 5.0 * p1 + 4.0 * p2 - p3 ) * u * u + ( 3.0 * ( p1 - p2 ) + p3 - p0 ) * u * u * u );
            ASSERT_TRUE( fabs( e.GetComponent( c ) - expected ) < 1e-12 );
        }
        // degenerate: coincident points do not give non-finite results
        CTCatmullRomSpline< TFloat64Point3D > coincident( CATMULLROM_CENTRIPETAL );
        coincident.AddPoint( TFloat64Point3D( 0, 0, 0 ) ); coincident.AddPoint( TFloat64Point3D( 0, 0, 0 ) ); coincident.AddPoint( TFloat64Point3D( 1, 0, 0 ) ); coincident.AddPoint( TFloat64Point3D( 2, 1, 0 ) );
        for ( Float64 t = 0.0; t <= coincident.GetMaxParameter(); t += coincident.GetMaxParameter() / 20.0 )
            ASSERT_TRUE( coincident.Evaluate( t ).IsFinite() && coincident.GetDerivative( t ).IsFinite() );
        // few points
        CTCatmullRomSpline< TFloat64Point3D > few;
        ASSERT_TRUE( TFloat64Point3D() == few.Evaluate( 1.0 ) && few.GetDerivative( 1.0 ).IsZero() );
        few.AddPoint( TFloat64Point3D( 1, 1, 1 ) );
        ASSERT_TRUE( TFloat64Point3D( 1, 1, 1 ) == few.Evaluate( 5.0 ) );
        few.AddPoint( TFloat64Point3D( 3, 3, 3 ) );
        ASSERT_TRUE( Dist3( few.Evaluate( 0.5 * few.GetMaxParameter() ), TFloat64Point3D( 2, 2, 2 ) ) < 1e-12 );
        ASSERT_TRUE( few.SetPoint( 1, TFloat64Point3D( 5, 5, 5 ) ) && !few.SetPoint( 7, TFloat64Point3D() ) && few.RemovePoint( 0 ) && !few.RemovePoint( 5 ) );
        CTCatmullRomSpline< TFloat64Point3D > back;
        CTCatmullRomSpline< TFloat64Point3D > original( CATMULLROM_CHORDAL );
        original.SetPoints( pts, 6 );
        ASSERT_TRUE( DataNodeRoundTrip( original, back ) && CATMULLROM_CHORDAL == back.GetParametrisation() && 6 == back.GetPointCount() && Dist3( back.Evaluate( 2.2 ), original.Evaluate( 2.2 ) ) < 1e-15 );
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Curves: Float32 Catmull-Rom with large coordinates and coincident points stays finite" )
        const TCatmullRomParametrisation kinds[ 3 ] = { CATMULLROM_UNIFORM, CATMULLROM_CENTRIPETAL, CATMULLROM_CHORDAL };
        for ( UInt32 k=0; k<3; ++k )
        {
            CTCatmullRomSpline< TFloat32Point3D > spline( kinds[ k ] );
            spline.AddPoint( TFloat32Point3D( 0.0f, 0.0f, 0.0f ) );
            spline.AddPoint( TFloat32Point3D( 1e4f, 0.0f, 0.0f ) );
            spline.AddPoint( TFloat32Point3D( 1e4f, 0.0f, 0.0f ) );
            spline.AddPoint( TFloat32Point3D( 2e4f, 0.0f, 0.0f ) );
            spline.AddPoint( TFloat32Point3D( 2e4f, 0.0f, 0.0f ) );
            for ( UInt32 i=1; i<5; ++i )
                ASSERT_TRUE( spline.GetKnot( i ) > spline.GetKnot( i - 1 ) );
            const Float32 end = spline.GetMaxParameter();
            for ( UInt32 i=0; i<=40; ++i )
            {
                const Float32 t = end * ( i / 40.0f );
                ASSERT_TRUE( spline.Evaluate( t ).IsFinite() && spline.GetDerivative( t ).IsFinite() );
            }
        }
        // the chordal case of the review: knots beyond 8 in Float32
        CTCatmullRomSpline< TFloat32Point2D > chordal( CATMULLROM_CHORDAL );
        chordal.AddPoint( TFloat32Point2D( 0.0f, 0.0f ) ); chordal.AddPoint( TFloat32Point2D( 1e4f, 0.0f ) ); chordal.AddPoint( TFloat32Point2D( 1e4f, 0.0f ) ); chordal.AddPoint( TFloat32Point2D( 2e4f, 0.0f ) );
        ASSERT_TRUE( chordal.Evaluate( 5000.0f ).IsFinite() && chordal.Evaluate( 15000.0f ).IsFinite() );
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Curves: centripetal Catmull-Rom has bounded curvature where the uniform one makes a cusp" )
        // four points with a short middle segment ( the configuration of Yuksel, Schaefer and Keyser )
        const TFloat64Point2D pts[ 4 ] = { TFloat64Point2D( -1.0, 0.0 ), TFloat64Point2D( 0.0, 0.0 ), TFloat64Point2D( 0.1, 0.01 ), TFloat64Point2D( 1.1, 0.0 ) };
        Float64 worst[ 2 ] = { 0.0, 0.0 };
        const TCatmullRomParametrisation kinds[ 2 ] = { CATMULLROM_CENTRIPETAL, CATMULLROM_UNIFORM };
        for ( UInt32 k=0; k<2; ++k )
        {
            CTCatmullRomSpline< TFloat64Point2D > spline( kinds[ k ] );
            spline.SetPoints( pts, 4 );
            const Float64 a = spline.GetKnot( 1 );
            const Float64 b = spline.GetKnot( 2 );
            const Float64 h = ( b - a ) * 1e-4;
            for ( UInt32 i=5; i<=995; ++i )
            {
                const Float64 t = a + ( b - a ) * ( i / 1000.0 );
                const TFloat64Vector2D v = spline.GetDerivative( t );
                const TFloat64Vector2D acc = ( spline.GetDerivative( t + h ) - spline.GetDerivative( t - h ) ) * ( 0.5 / h );
                const Float64 speed = v.GetLength();
                if ( speed > 1e-9 )
                {
                    const Float64 curvature = fabs( v.x * acc.y - v.y * acc.x ) / ( speed * speed * speed );
                    if ( curvature > worst[ k ] ) worst[ k ] = curvature;
                }
            }
        }
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Curves: max curvature centripetal " << worst[ 0 ] << " uniform " << worst[ 1 ] );
        ASSERT_TRUE( worst[ 0 ] < 10.0 && worst[ 1 ] > 1000.0 );
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Curves: B-spline equals the uniform cubic matrix form and has partition of unity" )
        const Float64 raw[ 8 ] = { 0.0, 2.0, 1.0, 4.0, 3.0, 6.0, 2.5, 5.0 };
        {
            CTBSpline< Float64 > spline;
            ASSERT_TRUE( spline.InitializeUniform( 3, raw, 8 ) && 3.0 == spline.GetMinParameter() && 8.0 == spline.GetMaxParameter() && 12 == spline.GetKnotCount() );
            for ( UInt32 j=0; j<5; ++j )
                for ( Float64 u = 0.0; u < 1.0; u += 0.125 )
                    ASSERT_TRUE( fabs( spline.Evaluate( 3.0 + j + u ) - UniformCubicMatrixForm( raw + j, u ) ) < 1e-12 );
        }
        for ( UInt32 degree=1; degree<=5; ++degree )
        {
            CTBSpline< Float64 > spline;
            ASSERT_TRUE( spline.Initialize( degree, raw, 8 ) );
            ASSERT_TRUE( fabs( spline.Evaluate( spline.GetMinParameter() ) - raw[ 0 ] ) < 1e-12 && fabs( spline.Evaluate( spline.GetMaxParameter() ) - raw[ 7 ] ) < 1e-12 );
            Float64 basis[ 6 ];
            for ( Float64 t = spline.GetMinParameter(); t <= spline.GetMaxParameter() + 1e-12; t += ( spline.GetMaxParameter() - spline.GetMinParameter() ) / 97.0 )
            {
                UInt32 first = 0;
                ASSERT_TRUE( spline.GetBasisFunctions( t, first, basis ) );
                Float64 sum = 0.0;
                Float64 value = 0.0;
                for ( UInt32 i=0; i<=degree; ++i )
                {
                    ASSERT_TRUE( basis[ i ] >= -1e-15 );
                    sum += basis[ i ];
                    value += basis[ i ] * raw[ first + i ];
                }
                ASSERT_TRUE( fabs( sum - 1.0 ) < 1e-12 && fabs( value - spline.Evaluate( t ) ) < 1e-12 );
            }
            // derivative against central differences away from the ends
            if ( degree >= 2 )
                for ( Float64 t = spline.GetMinParameter() + 0.13; t < spline.GetMaxParameter() - 0.1; t += 0.37 )
                    ASSERT_TRUE( fabs( spline.GetDerivative( t ) - ( spline.Evaluate( t + 1e-6 ) - spline.Evaluate( t - 1e-6 ) ) * 0.5e6 ) < 1e-5 );
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Curves: clamped cubic B-spline with four points is the Bezier curve; invalid input is rejected" )
        const TFloat64Point3D cp[ 4 ] = { TFloat64Point3D( 0, 0, 0 ), TFloat64Point3D( 2, 3, -1 ), TFloat64Point3D( 4, -2, 2 ), TFloat64Point3D( 6, 1, 0.5 ) };
        CTBSpline< TFloat64Point3D > spline;
        ASSERT_TRUE( spline.Initialize( 3, cp, 4 ) && 0.0 == spline.GetMinParameter() && 1.0 == spline.GetMaxParameter() );
        const CTBezierCurve3< TFloat64Point3D > bezier = MakeCubic();
        for ( Float64 t = 0.0; t <= 1.0; t += 0.0625 )
        {
            ASSERT_TRUE( Dist3( spline.Evaluate( t ), bezier.Evaluate( t ) ) < 1e-12 );
            ASSERT_TRUE( spline.GetDerivative( t ).Equals( bezier.GetDerivative( t ), 1e-10, 1e-10 ) );
        }
        CTBSpline< TFloat64Point3D > invalid;
        ASSERT_TRUE( !invalid.IsValid() && TFloat64Point3D() == invalid.Evaluate( 0.5 ) && invalid.GetDerivative( 0.5 ).IsZero() );
        ASSERT_TRUE( !invalid.Initialize( 0, cp, 4 ) && !invalid.Initialize( 3, cp, 3 ) && !invalid.Initialize( 16, cp, 4 ) && !invalid.InitializeUniform( 4, cp, 4 ) );
        const Float64 badKnots[ 8 ] = { 0, 0, 0, 0, 1, 0.5, 1, 1 };
        ASSERT_TRUE( !invalid.SetKnots( 3, cp, 4, badKnots, 8 ) && !invalid.IsValid() );
        // knots are kept: a repeated interior knot lowers the smoothness but still evaluates
        const TFloat64Point3D five[ 5 ] = { cp[ 0 ], cp[ 1 ], cp[ 2 ], cp[ 3 ], TFloat64Point3D( 7, 7, 7 ) };
        const Float64 knots[ 9 ] = { 0, 0, 0, 0, 0.5, 1, 1, 1, 1 };
        ASSERT_TRUE( invalid.SetKnots( 3, five, 5, knots, 9 ) && Dist3( invalid.Evaluate( 1.0 ), five[ 4 ] ) < 1e-12 && Dist3( invalid.Evaluate( 0.0 ), five[ 0 ] ) < 1e-12 );
        CTBSpline< TFloat64Point3D > back;
        ASSERT_TRUE( DataNodeRoundTrip( invalid, back ) && back.GetDegree() == 3 && Dist3( back.Evaluate( 0.3 ), invalid.Evaluate( 0.3 ) ) < 1e-15 );
        CTBSpline< TFloat64Point3D > noBack;
        CORE::CDataNode empty( "curve", GUCEF_DATATYPE_OBJECT );
        ASSERT_TRUE( !noBack.Deserialize( empty, CORE::CDataNodeSerializableSettings() ) && !noBack.IsValid() );
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Curves: arc length table works for splines and gives half lengths" )
        const TFloat64Point3D pts[ 6 ] = { TFloat64Point3D( 0, 0, 0 ), TFloat64Point3D( 1, 2, 0 ), TFloat64Point3D( 3, 3, 1 ), TFloat64Point3D( 3.5, 0, 2 ), TFloat64Point3D( 6, -1, 2 ), TFloat64Point3D( 8, 2, 5 ) };
        CTCatmullRomSpline< TFloat64Point3D > crSpline( CATMULLROM_CENTRIPETAL );
        crSpline.SetPoints( pts, 6 );
        CTArcLengthTable< Float64 > table;
        ASSERT_TRUE( table.BuildPiecewise( crSpline ) );
        // the reference integrates piece by piece between the knots, where the speed has its kinks
        Float64 reference = 0.0;
        for ( UInt32 i=0; i<5; ++i )
            reference += Simpson( SSpeed< CTCatmullRomSpline< TFloat64Point3D > >( crSpline ), crSpline.GetKnot( i ), crSpline.GetKnot( i + 1 ) );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Curves: Catmull-Rom length " << table.GetLength() << " reference " << reference << " intervals " << table.GetIntervalCount() );
        ASSERT_TRUE( fabs( table.GetLength() - reference ) / reference < 1e-8 );
        const Float64 mid = table.GetParameterAtDistance( crSpline, 0.5 * table.GetLength() );
        ASSERT_TRUE( fabs( Simpson( SSpeed< CTCatmullRomSpline< TFloat64Point3D > >( crSpline ), crSpline.GetMinParameter(), mid ) - 0.5 * table.GetLength() ) / table.GetLength() < 1e-6 );

        CTBSpline< TFloat64Point3D > bSpline;
        ASSERT_TRUE( bSpline.Initialize( 3, pts, 6 ) );
        CTArcLengthTable< Float64 > bTable;
        ASSERT_TRUE( bTable.BuildPiecewise( bSpline ) );
        ASSERT_TRUE( fabs( bTable.GetLength() - Simpson( SSpeed< CTBSpline< TFloat64Point3D > >( bSpline ), bSpline.GetMinParameter(), bSpline.GetMaxParameter() ) ) / bTable.GetLength() < 1e-8 );
        const Float64 bMid = bTable.GetParameterAtDistance( bSpline, 0.5 * bTable.GetLength() );
        ASSERT_TRUE( fabs( Simpson( SSpeed< CTBSpline< TFloat64Point3D > >( bSpline ), bSpline.GetMinParameter(), bMid ) - 0.5 * bTable.GetLength() ) / bTable.GetLength() < 1e-6 );

        CTHermiteSpline< TFloat64Point3D > hermite;
        hermite.AddKey( pts[ 0 ], TFloat64Vector3D( 1, 2, 0 ) ); hermite.AddKey( pts[ 1 ], TFloat64Vector3D( 2, 0, 1 ) ); hermite.AddKey( pts[ 2 ], TFloat64Vector3D( 1, -3, 1 ) );
        CTArcLengthTable< Float64 > hTable;
        ASSERT_TRUE( hTable.BuildPiecewise( hermite ) );
        ASSERT_TRUE( fabs( hTable.GetLength() - Simpson( SSpeed< CTHermiteSpline< TFloat64Point3D > >( hermite ), 0.0, 2.0 ) ) / hTable.GetLength() < 1e-8 );
        // an empty parameter range fails
        CTHermiteSpline< TFloat64Point3D > none;
        CTArcLengthTable< Float64 > failed;
        ASSERT_TRUE( !failed.Build( none ) && !failed.BuildPiecewise( none ) && !failed.IsValid() );
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Curves: seeded random Beziers keep the endpoint, split and derivative identities" )
        const UInt64 seed = 20260314;
        MathReference::CSeededSequence rng( seed );
        bool ok = true;
        for ( UInt32 i=0; i<500; ++i )
        {
            const CTBezierCurve3< TFloat64Point3D > c( TFloat64Point3D( rng.NextFloat64( -50, 50 ), rng.NextFloat64( -50, 50 ), rng.NextFloat64( -50, 50 ) ),
                                                          TFloat64Point3D( rng.NextFloat64( -50, 50 ), rng.NextFloat64( -50, 50 ), rng.NextFloat64( -50, 50 ) ),
                                                          TFloat64Point3D( rng.NextFloat64( -50, 50 ), rng.NextFloat64( -50, 50 ), rng.NextFloat64( -50, 50 ) ),
                                                          TFloat64Point3D( rng.NextFloat64( -50, 50 ), rng.NextFloat64( -50, 50 ), rng.NextFloat64( -50, 50 ) ) );
            const Float64 t = rng.NextFloat64( 0.0, 1.0 );
            CTBezierCurve3< TFloat64Point3D > l, r;
            c.Split( t, l, r );
            const Float64 u = rng.NextFloat64( 0.0, 1.0 );
            ok = ok && Dist3( l.Evaluate( u ), c.Evaluate( t * u ) ) < 1e-10 && Dist3( r.Evaluate( u ), c.Evaluate( t + ( 1.0 - t ) * u ) ) < 1e-10;
            ok = ok && c.GetDerivative( 0.0 ).Equals( ( c.p1 - c.p0 ) * 3.0, 1e-10, 1e-10 );
        }
        if ( !ok )
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Curves: random Bezier sweep failed, seed " << seed );
        ASSERT_TRUE( ok );
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
