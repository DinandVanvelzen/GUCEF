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

#ifndef GUCEF_MATH_CTPATH_H
#include "gucefMATH_CTPath.h"
#define GUCEF_MATH_CTPATH_H
#endif /* GUCEF_MATH_CTPATH_H ? */

#ifndef GUCEF_MATH_CURVEQUERYFUNCTIONS_H
#include "gucefMATH_CurveQueryFunctions.h"
#define GUCEF_MATH_CURVEQUERYFUNCTIONS_H
#endif /* GUCEF_MATH_CURVEQUERYFUNCTIONS_H ? */

#ifndef GUCEF_MATH_CTBEZIERCURVE3_H
#include "gucefMATH_CTBezierCurve3.h"
#define GUCEF_MATH_CTBEZIERCURVE3_H
#endif /* GUCEF_MATH_CTBEZIERCURVE3_H ? */

#ifndef GUCEF_MATH_CTCATMULLROMSPLINE_H
#include "gucefMATH_CTCatmullRomSpline.h"
#define GUCEF_MATH_CTCATMULLROMSPLINE_H
#endif /* GUCEF_MATH_CTCATMULLROMSPLINE_H ? */

#ifndef GUCEF_MATH_CTBSPLINE_H
#include "gucefMATH_CTBSpline.h"
#define GUCEF_MATH_CTBSPLINE_H
#endif /* GUCEF_MATH_CTBSPLINE_H ? */

#include "TestMathReference.h"
#include "TestPath.h"

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

GUCEF_MATH_STATIC_ASSERT( CORE::TypeHasMemberFunctionForDataNodeSerialization< CTPath< TFloat32Point3D > >::value, CTPath_has_Serialize );

typedef CTPath< TFloat64Point2D > TPath2D;

/** Reference: nearest point on a polyline by brute force over the segments, written without any gucefMATH vector code */
static Float64
BruteForceDistance( const TPath2D& path, Float64 px, Float64 py, Float64& along )
{
    Float64 best = 1e300;
    Float64 travelled = 0.0;
    along = 0.0;
    for ( UInt32 i=0; i+1<path.GetPointCount(); ++i )
    {
        const TFloat64Point2D a = path.GetPoint( i );
        const TFloat64Point2D b = path.GetPoint( i + 1 );
        const Float64 dx = b.x - a.x, dy = b.y - a.y;
        const Float64 length = sqrt( dx * dx + dy * dy );
        Float64 s = length > 0.0 ? ( ( px - a.x ) * dx + ( py - a.y ) * dy ) / ( length * length ) : 0.0;
        s = s < 0.0 ? 0.0 : ( s > 1.0 ? 1.0 : s );
        const Float64 qx = a.x + s * dx - px, qy = a.y + s * dy - py;
        const Float64 distance = sqrt( qx * qx + qy * qy );
        if ( distance < best )
        {
            best = distance;
            along = travelled + s * length;
        }
        travelled += length;
    }
    return best;
}

/** Reference: closest parameter by dense sampling and golden section refinement of the sampled neighbourhood */
template < typename TCurve >
static Float64
DenseClosestDistance( const TCurve& curve, const typename TCurve::TTraits::TPoint& p, Float64& parameter )
{
    const UInt32 samples = 40000;
    const Float64 start = curve.GetMinParameter();
    const Float64 end = curve.GetMaxParameter();
    UInt32 bestIndex = 0;
    Float64 best = 1e300;
    for ( UInt32 i=0; i<=samples; ++i )
    {
        const Float64 d = ( curve.Evaluate( start + ( end - start ) * i / samples ) - p ).GetLength();
        if ( d < best )
        {
            best = d;
            bestIndex = i;
        }
    }
    Float64 lo = start + ( end - start ) * ( bestIndex > 0 ? bestIndex - 1 : 0 ) / samples;
    Float64 hi = start + ( end - start ) * ( bestIndex < samples ? bestIndex + 1 : samples ) / samples;
    const Float64 invPhi = 0.6180339887498949;
    Float64 a = hi - invPhi * ( hi - lo );
    Float64 b = lo + invPhi * ( hi - lo );
    Float64 fa = ( curve.Evaluate( a ) - p ).GetLength();
    Float64 fb = ( curve.Evaluate( b ) - p ).GetLength();
    for ( UInt32 i=0; i<200; ++i )
    {
        if ( fa < fb )
        {
            hi = b; b = a; fb = fa;
            a = hi - invPhi * ( hi - lo );
            fa = ( curve.Evaluate( a ) - p ).GetLength();
        }
        else
        {
            lo = a; a = b; fa = fb;
            b = lo + invPhi * ( hi - lo );
            fb = ( curve.Evaluate( b ) - p ).GetLength();
        }
    }
    parameter = 0.5 * ( lo + hi );
    const Float64 refined = ( curve.Evaluate( parameter ) - p ).GetLength();
    return refined < best ? refined : best;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformPathTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING Path TESTS" );
    GUCEF_TESTFW_SUITE_SCOPE( "Path" );

    GUCEF_TESTFW_TESTCASE( "Path: point and tangent at a distance on a polyline are exact" )
        const TFloat64Point3D pts[ 4 ] = { TFloat64Point3D( 0, 0, 0 ), TFloat64Point3D( 3, 4, 0 ), TFloat64Point3D( 3, 4, 12 ), TFloat64Point3D( 3, 4, 12 ) };
        CTPath< TFloat64Point3D > path;
        path.SetPoints( pts, 4 );
        ASSERT_TRUE( 4 == path.GetPointCount() && 17.0 == path.GetLength() && 5.0 == path.GetDistanceOfPoint( 1 ) && 17.0 == path.GetDistanceOfPoint( 3 ) );
        ASSERT_TRUE( path.GetPointAtDistance( 0.0 ) == pts[ 0 ] && path.GetPointAtDistance( 5.0 ) == pts[ 1 ] && path.GetPointAtDistance( 17.0 ) == pts[ 3 ] );
        ASSERT_TRUE( ( path.GetPointAtDistance( 2.5 ) - TFloat64Point3D( 1.5, 2.0, 0.0 ) ).GetLength() < 1e-15 );
        ASSERT_TRUE( ( path.GetPointAtDistance( 11.0 ) - TFloat64Point3D( 3, 4, 6 ) ).GetLength() < 1e-15 );
        ASSERT_TRUE( path.GetPointAtDistance( -3.0 ) == pts[ 0 ] && path.GetPointAtDistance( 99.0 ) == pts[ 3 ] );
        ASSERT_TRUE( path.GetTangentAtDistance( 2.0 ).Equals( TFloat64Vector3D( 0.6, 0.8, 0.0 ), 1e-15, 1e-15 ) );
        ASSERT_TRUE( path.GetTangentAtDistance( 5.0 ).Equals( TFloat64Vector3D( 0.0, 0.0, 1.0 ), 1e-15, 1e-15 ) );
        ASSERT_TRUE( path.GetTangentAtDistance( 17.0 ).Equals( TFloat64Vector3D( 0.0, 0.0, 1.0 ), 1e-15, 1e-15 ) );
        ASSERT_TRUE( path.GetTangentAtDistance( -1.0 ).Equals( TFloat64Vector3D( 0.6, 0.8, 0.0 ), 1e-15, 1e-15 ) );
        // the empty, one point and zero length paths
        CTPath< TFloat64Point3D > empty;
        ASSERT_TRUE( 0.0 == empty.GetLength() && TFloat64Point3D() == empty.GetPointAtDistance( 1.0 ) && empty.GetTangentAtDistance( 1.0 ).IsZero() );
        TFloat64Point3D closest;
        Float64 along = 5.0;
        ASSERT_TRUE( !empty.GetClosestPoint( TFloat64Point3D( 1, 1, 1 ), closest, along ) && 5.0 == along );
        empty.AddPoint( TFloat64Point3D( 2, 2, 2 ) );
        ASSERT_TRUE( TFloat64Point3D( 2, 2, 2 ) == empty.GetPointAtDistance( 3.0 ) && empty.GetTangentAtDistance( 0.0 ).IsZero() );
        ASSERT_TRUE( empty.GetClosestPoint( TFloat64Point3D( 0, 0, 0 ), closest, along ) && TFloat64Point3D( 2, 2, 2 ) == closest && 0.0 == along );
        empty.AddPoint( TFloat64Point3D( 2, 2, 2 ) );
        ASSERT_TRUE( 0.0 == empty.GetLength() && empty.GetTangentAtDistance( 0.0 ).IsZero() && TFloat64Point3D( 2, 2, 2 ) == empty.GetPointAtDistance( 1.0 ) );
        // Float32 and scalar paths
        CTPath< Float32 > line;
        line.AddPoint( 0.0f ); line.AddPoint( 4.0f ); line.AddPoint( 1.0f );
        ASSERT_TRUE( 7.0f == line.GetLength() && fabs( line.GetPointAtDistance( 5.0f ) - 3.0f ) < 1e-6f && fabs( line.GetTangentAtDistance( 5.0f ) + 1.0f ) < 1e-6f );
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Path: closest point on a circle approximating path lies within the sagitta" )
        const UInt32 segments = 64;
        TPath2D circle;
        for ( UInt32 i=0; i<=segments; ++i )
        {
            const Float64 a = 6.283185307179586 * i / segments;
            circle.AddPoint( TFloat64Point2D( 10.0 * cos( a ), 10.0 * sin( a ) ) );
        }
        const Float64 sagitta = 10.0 * ( 1.0 - cos( 3.141592653589793 / segments ) );
        MathReference::CSeededSequence rng( 99 );
        for ( UInt32 i=0; i<500; ++i )
        {
            const Float64 angle = rng.NextFloat64( 0.0, 6.283185307179586 );
            const Float64 radius = rng.NextFloat64( 1.0, 30.0 );
            const TFloat64Point2D p( radius * cos( angle ), radius * sin( angle ) );
            TFloat64Point2D closest;
            Float64 along = 0.0;
            ASSERT_TRUE( circle.GetClosestPoint( p, closest, along ) );
            // the true circle distance is | radius - 10 |; the polyline is at most the sagitta inside the circle
            const Float64 distance = ( p - closest ).GetLength();
            ASSERT_TRUE( fabs( distance - fabs( radius - 10.0 ) ) <= sagitta + 1e-9 );
            ASSERT_TRUE( along >= 0.0 && along <= circle.GetLength() && ( circle.GetPointAtDistance( along ) - closest ).GetLength() < 1e-9 );
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Path: closest point matches brute force on random polylines (the coarse bound skips nothing relevant)" )
        const UInt64 seed = 424242;
        MathReference::CSeededSequence rng( seed );
        bool ok = true;
        for ( UInt32 round=0; round<200 && ok; ++round )
        {
            TPath2D path;
            const UInt32 count = 2 + rng.NextUInt32() % 60;
            for ( UInt32 i=0; i<count; ++i )
                path.AddPoint( TFloat64Point2D( rng.NextFloat64( -100.0, 100.0 ), rng.NextFloat64( -100.0, 100.0 ) ) );
            for ( UInt32 q=0; q<20; ++q )
            {
                const TFloat64Point2D p( rng.NextFloat64( -150.0, 150.0 ), rng.NextFloat64( -150.0, 150.0 ) );
                TFloat64Point2D closest;
                Float64 along = 0.0;
                Float64 referenceAlong = 0.0;
                const Float64 reference = BruteForceDistance( path, p.x, p.y, referenceAlong );
                ok = ok && path.GetClosestPoint( p, closest, along ) && fabs( ( p - closest ).GetLength() - reference ) < 1e-9;
            }
        }
        if ( !ok )
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Path: closest point sweep failed, seed " << seed );
        ASSERT_TRUE( ok );
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Path: built from a curve, resampled and round tripped through a DataNode" )
        const CTBezierCurve3< TFloat64Point2D > curve( TFloat64Point2D( 0, 0 ), TFloat64Point2D( 1, 5 ), TFloat64Point2D( 4, -5 ), TFloat64Point2D( 5, 0 ) );
        TPath2D path;
        path.SetFromCurve( curve, 400 );
        ASSERT_TRUE( 401 == path.GetPointCount() && path.GetPoint( 0 ) == curve.p0 && path.GetPoint( 400 ) == curve.p3 );
        // the sampled polyline is just below the true curve length
        TPath2D fine;
        fine.SetFromCurve( curve, 4000 );
        ASSERT_TRUE( path.GetLength() <= fine.GetLength() + 1e-12 && fine.GetLength() - path.GetLength() < 1e-3 );
        // resampling at a fixed spacing
        TPath2D resampled;
        ASSERT_TRUE( path.Resample( 0.25, resampled ) );
        const UInt32 expected = (UInt32) ceil( path.GetLength() / 0.25 ) + 1;
        ASSERT_TRUE( expected == resampled.GetPointCount() && resampled.GetPoint( 0 ) == path.GetPoint( 0 ) && resampled.GetPoint( expected - 1 ) == path.GetPoint( 400 ) );
        for ( UInt32 i=0; i+2<resampled.GetPointCount(); ++i )
            ASSERT_TRUE( fabs( ( resampled.GetPoint( i + 1 ) - resampled.GetPoint( i ) ).GetLength() - 0.25 ) < 5e-3 );
        ASSERT_TRUE( resampled.GetLength() <= path.GetLength() + 1e-9 && path.GetLength() - resampled.GetLength() < 0.1 );
        // spacing that is not positive and a path with one point give a copy
        TPath2D copy;
        ASSERT_TRUE( path.Resample( 0.0, copy ) && copy.GetPointCount() == path.GetPointCount() );
        ASSERT_TRUE( path.Resample( -1.0, copy ) && copy.GetPointCount() == path.GetPointCount() );
        // a spacing that would give more than the allowed number of points is refused and changes nothing
        TPath2D untouched = path;
        ASSERT_TRUE( !path.Resample( 1e-9, untouched ) && untouched.GetPointCount() == path.GetPointCount() );
        ASSERT_TRUE( !path.Resample( 0.25, untouched, 10 ) && untouched.GetPointCount() == path.GetPointCount() );
        // Float32 path of 20000 long with a tiny spacing: refused in short time ( the step counter of a Float32 stops at 2^24 )
        CTPath< Float32 > longPath;
        longPath.AddPoint( 0.0f ); longPath.AddPoint( 20000.0f );
        CTPath< Float32 > longResult;
        ASSERT_TRUE( !longPath.Resample( 0.001f, longResult ) && 0 == longResult.GetPointCount() );
        ASSERT_TRUE( longPath.Resample( 0.5f, longResult ) && 40001 == longResult.GetPointCount() && 20000.0f == longResult.GetPointAtDistance( 1e9f ) );
        CORE::CDataNodeSerializableSettings settings;
        CORE::CDataNode node( "path", GUCEF_DATATYPE_OBJECT );
        TPath2D back;
        ASSERT_TRUE( resampled.Serialize( node, settings ) && back.Deserialize( node, settings ) && back.GetPointCount() == resampled.GetPointCount() && back.GetLength() == resampled.GetLength() );
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Path: closest point on a parametric curve matches dense sampling with golden section refinement" )
        const UInt64 seed = 8675309;
        MathReference::CSeededSequence rng( seed );
        const CTBezierCurve3< TFloat64Point2D > bezier( TFloat64Point2D( 0, 0 ), TFloat64Point2D( 1, 5 ), TFloat64Point2D( 4, -5 ), TFloat64Point2D( 5, 0 ) );
        const TFloat64Point2D pts[ 6 ] = { TFloat64Point2D( 0, 0 ), TFloat64Point2D( 2, 3 ), TFloat64Point2D( 4, 1 ), TFloat64Point2D( 6, 4 ), TFloat64Point2D( 8, 0 ), TFloat64Point2D( 9, 3 ) };
        CTCatmullRomSpline< TFloat64Point2D > spline( CATMULLROM_CENTRIPETAL );
        spline.SetPoints( pts, 6 );
        CTBSpline< TFloat64Point2D > bspline;
        bspline.Initialize( 3, pts, 6 );
        bool ok = true;
        for ( UInt32 i=0; i<150; ++i )
        {
            const TFloat64Point2D p( rng.NextFloat64( -3.0, 12.0 ), rng.NextFloat64( -6.0, 8.0 ) );
            Float64 t = 0.0, referenceT = 0.0;
            TFloat64Point2D closest;
            const bool foundBezier = ClosestPointOnCurve( bezier, p, t, closest );
            const Float64 differenceBezier = fabs( ( p - closest ).GetLength() - DenseClosestDistance( bezier, p, referenceT ) );
            const bool foundSpline = ClosestPointOnCurve( spline, p, t, closest, 200 );
            const Float64 differenceSpline = fabs( ( p - closest ).GetLength() - DenseClosestDistance( spline, p, referenceT ) );
            const bool foundBSpline = ClosestPointOnCurve( bspline, p, t, closest, 200 );
            const Float64 differenceBSpline = fabs( ( p - closest ).GetLength() - DenseClosestDistance( bspline, p, referenceT ) );
            const bool consistent = ( closest - bspline.Evaluate( t ) ).GetLength() < 1e-12;
            if ( !( foundBezier && foundSpline && foundBSpline && consistent && differenceBezier < 1e-7 && differenceSpline < 1e-7 && differenceBSpline < 1e-7 ) )
            {
                ok = false;
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Path: closest point mismatch for (" << p.x << ", " << p.y << ") differences " << differenceBezier << " " << differenceSpline << " " << differenceBSpline );
            }
        }
        if ( !ok )
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Path: curve closest point sweep failed, seed " << seed );
        ASSERT_TRUE( ok );
        // end points are found as such; a point on the curve gives distance zero and the parameter
        Float64 t = 0.0;
        TFloat64Point2D closest;
        ASSERT_TRUE( ClosestPointOnCurve( bezier, TFloat64Point2D( -10.0, 0.0 ), t, closest ) && 0.0 == t && closest == bezier.p0 );
        ASSERT_TRUE( ClosestPointOnCurve( bezier, TFloat64Point2D( 20.0, 0.0 ), t, closest ) && 1.0 == t && closest == bezier.p3 );
        const TFloat64Point2D onCurve = bezier.Evaluate( 0.37 );
        ASSERT_TRUE( ClosestPointOnCurve( bezier, onCurve, t, closest ) && fabs( t - 0.37 ) < 1e-6 && ( closest - onCurve ).GetLength() < 1e-9 );
        // scalar and Float32 curves, empty range
        CTBezierCurve3< Float64 > scalar( 0.0, 1.0, 2.0, 3.0 );
        Float64 sc = 0.0, st = 0.0;
        ASSERT_TRUE( ClosestPointOnCurve( scalar, 1.5, st, sc ) && fabs( st - 0.5 ) < 1e-9 && fabs( sc - 1.5 ) < 1e-9 );
        CTBSpline< TFloat64Point2D > invalid;
        ASSERT_TRUE( !ClosestPointOnCurve( invalid, TFloat64Point2D( 1, 1 ), t, closest ) );
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
