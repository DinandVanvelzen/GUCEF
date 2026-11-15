/*
 *  gucefGEOMETRY_TestApp: GUCEF test application for the Geometry module
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
#include <string.h>

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestQueryReference.h"
#include "TestClosestPoints.h"

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
using namespace GUCEF::GEOMETRY;
using namespace QueryReference;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The counts of a closest point sweep */
struct SClosestSweep
{
    UInt32 pairs;
    UInt32 apart;               /**< pairs that are apart: compared with the alternating projection distance */
    UInt32 overlapping;
    UInt32 bad;

    SClosestSweep( void ) : pairs( 0 ), apart( 0 ), overlapping( 0 ), bad( 0 ) {}
};

/** The tolerance of positions of the kind T for shapes of unit size: a few thousand ulp, and the 1e-4 of the acceptance for the distances */
template < typename T > static Float64 MemberTolerance( void ) { return 8.0 * (Float64) CTNumericTraits< T >::Epsilon() * 16.0; }

/*-------------------------------------------------------------------------*/

/**
 *  'count' seeded pairs: the nearest points returned by ClosestPoints must be points of the two shapes (the nearest point
 *  member of the shape finds them at distance 0), the distance must equal the distance of the points and that of Distance,
 *  must not exceed the distance of any pair of 400 sampled points, and must equal the alternating projection distance of
 *  the Float64 shapes within 1e-4 ( for overlapping shapes it must be 0 ).
 */
template < typename T, typename TA, typename TB >
static void
ClosestSweep( const char* name, UInt64 seed, UInt32 count, SClosestSweep& sweep )
{
    CRandom random( seed );
    const Float64 memberTolerance = MemberTolerance< T >();
    for ( UInt32 i=0; i<count; ++i )
    {
        TA a;
        TB b;
        RandomShape( random, a );
        RandomShape( random, b );
        ++sweep.pairs;
        CTPoint3D< T > pa;
        CTPoint3D< T > pb;
        if ( !ClosestPoints( a, b, pa, pb ) )
        {
            ++sweep.bad;
            continue;
        }
        const Float64 distance = ( Widen( pb ) - Widen( pa ) ).GetLength();
        const Float64 reported = (Float64) Distance( a, b );
        const typename SWidened< TA >::Type a64 = Widen( a );
        const typename SWidened< TB >::Type b64 = Widen( b );

        /* the points belong to the shapes */
        const Float64 offA = ( a64.ClosestPoint( Widen( pa ) ) - Widen( pa ) ).GetLength();
        const Float64 offB = ( b64.ClosestPoint( Widen( pb ) ) - Widen( pb ) ).GetLength();
        bool ok = offA <= memberTolerance && offB <= memberTolerance && NearTo( distance, reported, memberTolerance );

        /* no pair of sampled points is nearer */
        GUCEF::vector< P3 > samplesA;
        GUCEF::vector< P3 > samplesB;
        SampleShape( a64, random, 150, samplesA );
        SampleShape( b64, random, 150, samplesB );
        if ( distance > SampledDistance( samplesA, samplesB ) + memberTolerance )
            ok = false;

        /* the alternating projections of the Float64 shapes: an upper bound of the distance that is exact once they have converged to a fixed point */
        P3 refA;
        P3 refB;
        const Float64 reference = ConvergedDistance( a64, b64, refA, refB );
        const Float64 referenceFixed = ( a64.ClosestPoint( refB ) - refA ).GetLength() + ( b64.ClosestPoint( refA ) - refB ).GetLength();
        if ( distance > reference + memberTolerance )
            ok = false;                 /* the reference is the distance of two points of the shapes: nothing can be nearer than that */
        if ( reference > 1.0e-3 )
        {
            ++sweep.apart;
            if ( referenceFixed < 1.0e-9 && !NearTo( distance, reference, 1.0e-4 ) )
                ok = false;
        }
        else if ( reference < 1.0e-9 )
        {
            ++sweep.overlapping;
            if ( distance > memberTolerance * 4.0 )
                ok = false;
        }
        if ( !ok )
        {
            ++sweep.bad;
            if ( sweep.bad < 4 )
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ClosestPoints: " << name << " distance " << distance << " reference " << reference << ", points off " << offA << " and " << offB << ", seed " << seed << " pair " << i );
        }
    }
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ClosestPoints: " << name << " seed " << seed << ": " << sweep.pairs << " pairs, " << sweep.apart << " apart, " << sweep.overlapping << " overlapping, " << sweep.bad << " disagreements" );
}

/*-------------------------------------------------------------------------*/

template < typename T, typename TA, typename TB >
static bool
RunClosestCase( const char* name, UInt64 seed, bool needApart )
{
    SClosestSweep sweep;
    ClosestSweep< T, TA, TB >( name, seed, 2000, sweep );
    return 0 == sweep.bad && sweep.apart > sweep.pairs / 10 && ( !needApart || sweep.overlapping > sweep.pairs / 20 || sweep.pairs == sweep.apart );
}

/*-------------------------------------------------------------------------*/

/**
 *  The plane against a convex shape: the range of signed distances must contain the signed distance of every sampled
 *  point and be attained by points of the shape ( the nearest points of ClosestPoints ), which makes it exact.
 */
template < typename T, typename TShape >
static UInt32
PlaneShapeSweep( UInt64 seed, UInt32 count )
{
    CRandom random( seed );
    const Float64 tolerance = MemberTolerance< T >() * 4.0;
    UInt32 bad = 0;
    for ( UInt32 i=0; i<count; ++i )
    {
        CTPlane< T > plane;
        TShape shape;
        RandomShape( random, plane );
        RandomShape( random, shape );
        T lowest = 0;
        T highest = 0;
        GetSignedDistanceRange( plane, shape, lowest, highest );
        const CTPlane< Float64 > plane64 = Widen( plane );
        const typename SWidened< TShape >::Type shape64 = Widen( shape );
        GUCEF::vector< P3 > samples;
        SampleShape( shape64, random, 200, samples );
        for ( size_t k=0; k<samples.size(); ++k )
        {
            const Float64 d = plane64.GetSignedDistance( samples[ k ] );
            if ( d < (Float64) lowest - tolerance || d > (Float64) highest + tolerance )
                ++bad;
        }
        /* the ends of the range are attained: through the support points of the shape, which are points of it */
        const V3 n = plane64.normal.AsVector();
        const P3 low = shape64.GetSupport( n * -1.0 );
        const P3 high = shape64.GetSupport( n );
        if ( !NearTo( plane64.GetSignedDistance( low ), (Float64) lowest, tolerance ) || !NearTo( plane64.GetSignedDistance( high ), (Float64) highest, tolerance ) )
            ++bad;
        if ( ( shape64.ClosestPoint( low ) - low ).GetLength() > tolerance || ( shape64.ClosestPoint( high ) - high ).GetLength() > tolerance )
            ++bad;

        /* classification and nearest points follow the range */
        const TPlaneSide side = Classify( plane, shape );
        const bool straddles = lowest <= 0 && 0 <= highest;
        if ( ( PLANESIDE_STRADDLING == side ) != straddles || ( 0 < lowest && PLANESIDE_FRONT != side ) || ( highest < 0 && PLANESIDE_BEHIND != side ) )
            ++bad;
        if ( straddles != Intersects( plane, shape ) || straddles != Intersects( shape, plane ) )
            ++bad;
        CTPoint3D< T > onPlane;
        CTPoint3D< T > onShape;
        if ( !ClosestPoints( plane, shape, onPlane, onShape ) )
            ++bad;
        else
        {
            const Float64 expected = straddles ? 0.0 : ( 0 < lowest ? (Float64) lowest : -(Float64) highest );
            if ( !NearTo( ( Widen( onShape ) - Widen( onPlane ) ).GetLength(), expected, tolerance ) || fabs( plane64.GetSignedDistance( Widen( onPlane ) ) ) > tolerance ||
                 ( shape64.ClosestPoint( Widen( onShape ) ) - Widen( onShape ) ).GetLength() > tolerance )
                ++bad;
            if ( !NearTo( (Float64) Distance( plane, shape ), expected, tolerance ) || !NearTo( (Float64) Distance( shape, plane ), expected, tolerance ) )
                ++bad;
        }
    }
    return bad;
}

/*-------------------------------------------------------------------------*/

/** The same for the frustum, which has no nearest point member: the corners give the exact range */
template < typename T >
static UInt32
PlaneFrustumSweep( UInt64 seed, UInt32 count )
{
    CRandom random( seed );
    const Float64 tolerance = MemberTolerance< T >() * 64.0;
    UInt32 bad = 0;
    for ( UInt32 i=0; i<count; ++i )
    {
        CTPlane< T > plane;
        CTFrustum< T > frustum;
        RandomShape( random, plane );
        RandomShape( random, frustum );
        const CTPlane< Float64 > plane64 = Widen( plane );
        Float64 low = 1.0e300;
        Float64 high = -1.0e300;
        for ( UInt32 c=0; c<8; ++c )
        {
            const Float64 d = plane64.GetSignedDistance( Widen( frustum.GetCorner( c ) ) );
            low = fmin( low, d );
            high = fmax( high, d );
        }
        T lowest = 0;
        T highest = 0;
        GetSignedDistanceRange( plane, frustum, lowest, highest );
        const Float64 scale = fmax( 1.0, fmax( fabs( low ), fabs( high ) ) );
        if ( !NearTo( (Float64) lowest, low, tolerance * scale ) || !NearTo( (Float64) highest, high, tolerance * scale ) )
            ++bad;
    }
    return bad;
}

/*-------------------------------------------------------------------------*/

/** The segment against a plane: crossing gives a point on both, otherwise the nearer end point and its foot */
template < typename T >
static UInt32
SegmentPlaneSweep( UInt64 seed, UInt32 count )
{
    CRandom random( seed );
    const Float64 tolerance = MemberTolerance< T >() * 4.0;
    UInt32 bad = 0;
    for ( UInt32 i=0; i<count; ++i )
    {
        CTPlane< T > plane;
        CTSegment3D< T > segment;
        RandomShape( random, plane );
        RandomShape( random, segment );
        const CTPlane< Float64 > plane64 = Widen( plane );
        const Float64 da = plane64.GetSignedDistance( Widen( segment.start ) );
        const Float64 db = plane64.GetSignedDistance( Widen( segment.end ) );
        const bool crossing = ( da <= 0 && db >= 0 ) || ( da >= 0 && db <= 0 );
        CTPoint3D< T > onSegment;
        CTPoint3D< T > onPlane;
        if ( !ClosestPoints( segment, plane, onSegment, onPlane ) )
        {
            ++bad;
            continue;
        }
        const Float64 expected = crossing ? 0.0 : fmin( fabs( da ), fabs( db ) );
        if ( !NearTo( ( Widen( onSegment ) - Widen( onPlane ) ).GetLength(), expected, tolerance ) || fabs( plane64.GetSignedDistance( Widen( onPlane ) ) ) > tolerance )
            ++bad;
        if ( ( Widen( segment ).ClosestPoint( Widen( onSegment ) ) - Widen( onSegment ) ).GetLength() > tolerance )
            ++bad;
        if ( crossing != Intersects( segment, plane ) || crossing != Intersects( plane, segment ) )
            if ( fabs( da ) > tolerance && fabs( db ) > tolerance )
                ++bad;
    }
    return bad;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformClosestPointsTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "ClosestPoints" );

    GUCEF_TESTFW_TESTCASE( "ClosestPoints: segment cells against dense sampling and alternating projection, Float32 and Float64" )
    try
    {
        ASSERT_TRUE( ( RunClosestCase< Float32, CTSegment3D< Float32 >, CTSegment3D< Float32 > >( "segment-segment32", 301, false ) ) );
        ASSERT_TRUE( ( RunClosestCase< Float64, CTSegment3D< Float64 >, CTSegment3D< Float64 > >( "segment-segment64", 302, false ) ) );
        ASSERT_TRUE( ( RunClosestCase< Float32, CTSegment3D< Float32 >, CTTriangle3D< Float32 > >( "segment-triangle32", 303, true ) ) );
        ASSERT_TRUE( ( RunClosestCase< Float64, CTSegment3D< Float64 >, CTTriangle3D< Float64 > >( "segment-triangle64", 304, true ) ) );
        ASSERT_TRUE( ( RunClosestCase< Float32, CTSegment3D< Float32 >, CTSphere< Float32 > >( "segment-sphere32", 305, true ) ) );
        ASSERT_TRUE( ( RunClosestCase< Float64, CTSegment3D< Float64 >, CTSphere< Float64 > >( "segment-sphere64", 306, true ) ) );
        ASSERT_TRUE( ( RunClosestCase< Float32, CTSegment3D< Float32 >, CTCapsule< Float32 > >( "segment-capsule32", 307, true ) ) );
        ASSERT_TRUE( ( RunClosestCase< Float64, CTSegment3D< Float64 >, CTCapsule< Float64 > >( "segment-capsule64", 308, true ) ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "ClosestPoints: triangle and sphere cells against dense sampling and alternating projection, Float32 and Float64" )
    try
    {
        ASSERT_TRUE( ( RunClosestCase< Float32, CTTriangle3D< Float32 >, CTTriangle3D< Float32 > >( "triangle-triangle32", 311, true ) ) );
        ASSERT_TRUE( ( RunClosestCase< Float64, CTTriangle3D< Float64 >, CTTriangle3D< Float64 > >( "triangle-triangle64", 312, true ) ) );
        ASSERT_TRUE( ( RunClosestCase< Float32, CTTriangle3D< Float32 >, CTSphere< Float32 > >( "triangle-sphere32", 313, true ) ) );
        ASSERT_TRUE( ( RunClosestCase< Float64, CTTriangle3D< Float64 >, CTSphere< Float64 > >( "triangle-sphere64", 314, true ) ) );
        ASSERT_TRUE( ( RunClosestCase< Float32, CTTriangle3D< Float32 >, CTCapsule< Float32 > >( "triangle-capsule32", 315, true ) ) );
        ASSERT_TRUE( ( RunClosestCase< Float64, CTTriangle3D< Float64 >, CTCapsule< Float64 > >( "triangle-capsule64", 316, true ) ) );
        ASSERT_TRUE( ( RunClosestCase< Float32, CTSphere< Float32 >, CTSphere< Float32 > >( "sphere-sphere32", 317, true ) ) );
        ASSERT_TRUE( ( RunClosestCase< Float64, CTSphere< Float64 >, CTSphere< Float64 > >( "sphere-sphere64", 318, true ) ) );
        ASSERT_TRUE( ( RunClosestCase< Float32, CTSphere< Float32 >, CTCapsule< Float32 > >( "sphere-capsule32", 319, true ) ) );
        ASSERT_TRUE( ( RunClosestCase< Float64, CTSphere< Float64 >, CTCapsule< Float64 > >( "sphere-capsule64", 320, true ) ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "ClosestPoints: sphere-box, capsule-capsule and box-box cells against dense sampling and alternating projection, Float32 and Float64" )
    try
    {
        ASSERT_TRUE( ( RunClosestCase< Float32, CTSphere< Float32 >, CTAxisAlignedBox3D< Float32 > >( "sphere-aabb32", 331, true ) ) );
        ASSERT_TRUE( ( RunClosestCase< Float64, CTSphere< Float64 >, CTAxisAlignedBox3D< Float64 > >( "sphere-aabb64", 332, true ) ) );
        ASSERT_TRUE( ( RunClosestCase< Float32, CTSphere< Float32 >, CTOrientedBox3D< Float32 > >( "sphere-obb32", 333, true ) ) );
        ASSERT_TRUE( ( RunClosestCase< Float64, CTSphere< Float64 >, CTOrientedBox3D< Float64 > >( "sphere-obb64", 334, true ) ) );
        ASSERT_TRUE( ( RunClosestCase< Float32, CTCapsule< Float32 >, CTCapsule< Float32 > >( "capsule-capsule32", 335, true ) ) );
        ASSERT_TRUE( ( RunClosestCase< Float64, CTCapsule< Float64 >, CTCapsule< Float64 > >( "capsule-capsule64", 336, true ) ) );
        ASSERT_TRUE( ( RunClosestCase< Float32, CTAxisAlignedBox3D< Float32 >, CTAxisAlignedBox3D< Float32 > >( "aabb-aabb32", 337, true ) ) );
        ASSERT_TRUE( ( RunClosestCase< Float64, CTAxisAlignedBox3D< Float64 >, CTAxisAlignedBox3D< Float64 > >( "aabb-aabb64", 338, true ) ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "ClosestPoints: plane against triangle, sphere, capsule, boxes, cylinder, cone and frustum, and the segment against the plane" )
    try
    {
        ASSERT_TRUE( 0 == ( PlaneShapeSweep< Float32, CTTriangle3D< Float32 > >( 341, 1000 ) ) );
        ASSERT_TRUE( 0 == ( PlaneShapeSweep< Float64, CTTriangle3D< Float64 > >( 342, 1000 ) ) );
        ASSERT_TRUE( 0 == ( PlaneShapeSweep< Float32, CTSphere< Float32 > >( 343, 1000 ) ) );
        ASSERT_TRUE( 0 == ( PlaneShapeSweep< Float64, CTSphere< Float64 > >( 344, 1000 ) ) );
        ASSERT_TRUE( 0 == ( PlaneShapeSweep< Float32, CTCapsule< Float32 > >( 345, 1000 ) ) );
        ASSERT_TRUE( 0 == ( PlaneShapeSweep< Float64, CTCapsule< Float64 > >( 346, 1000 ) ) );
        ASSERT_TRUE( 0 == ( PlaneShapeSweep< Float32, CTAxisAlignedBox3D< Float32 > >( 347, 1000 ) ) );
        ASSERT_TRUE( 0 == ( PlaneShapeSweep< Float64, CTAxisAlignedBox3D< Float64 > >( 348, 1000 ) ) );
        ASSERT_TRUE( 0 == ( PlaneShapeSweep< Float32, CTOrientedBox3D< Float32 > >( 349, 1000 ) ) );
        ASSERT_TRUE( 0 == ( PlaneShapeSweep< Float64, CTOrientedBox3D< Float64 > >( 350, 1000 ) ) );
        ASSERT_TRUE( 0 == PlaneFrustumSweep< Float32 >( 353, 1000 ) );
        ASSERT_TRUE( 0 == PlaneFrustumSweep< Float64 >( 354, 1000 ) );
        ASSERT_TRUE( 0 == SegmentPlaneSweep< Float32 >( 355, 2000 ) );
        ASSERT_TRUE( 0 == SegmentPlaneSweep< Float64 >( 356, 2000 ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "ClosestPoints: plane against cylinder and cone" )
    try
    {
        ASSERT_TRUE( 0 == ( PlaneShapeSweep< Float32, CTCylinder< Float32 > >( 351, 1000 ) ) );
        ASSERT_TRUE( 0 == ( PlaneShapeSweep< Float64, CTCylinder< Float64 > >( 352, 1000 ) ) );
        ASSERT_TRUE( 0 == ( PlaneShapeSweep< Float32, CTCone< Float32 > >( 357, 1000 ) ) );
        ASSERT_TRUE( 0 == ( PlaneShapeSweep< Float64, CTCone< Float64 > >( 358, 1000 ) ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "ClosestPoints: facade conventions, point cells, invalid shapes and the coincident witness of overlapping shapes" )
    try
    {
        typedef TFloat64Point3D P;
        typedef TFloat64Vector3D V;
        const CTSphere< Float64 > a( P( 0, 0, 0 ), 1.0 );
        const CTSphere< Float64 > b( P( 3, 0, 0 ), 0.5 );
        P pa;
        P pb;
        ASSERT_TRUE( ClosestPoints( a, b, pa, pb ) && NearTo( pa.x, 1.0, 1e-12 ) && NearTo( pb.x, 2.5, 1e-12 ) );
        ASSERT_TRUE( NearTo( Distance( a, b ), 1.5, 1e-12 ) && NearTo( Distance( b, a ), 1.5, 1e-12 ) );
        ASSERT_TRUE( ClosestPoints( b, a, pb, pa ) && NearTo( pa.x, 1.0, 1e-12 ) && NearTo( pb.x, 2.5, 1e-12 ) );

        /* overlapping shapes: distance 0 and one witness point that lies in both */
        const CTSphere< Float64 > c( P( 1, 0, 0 ), 1.0 );
        ASSERT_TRUE( ClosestPoints( a, c, pa, pb ) && pa == pb && a.Contains( pa ) && c.Contains( pb ) && 0.0 == Distance( a, c ) );
        const CTAxisAlignedBox3D< Float64 > box( P( -1, -1, -1 ), P( 1, 1, 1 ) );
        ASSERT_TRUE( ClosestPoints( CTSphere< Float64 >( P( 0.5, 0, 0 ), 0.25 ), box, pa, pb ) && pa == pb && box.Contains( pa ) );
        ASSERT_TRUE( ClosestPoints( box, box, pa, pb ) && pa == pb && box.Contains( pa ) );

        /* shapes without points: an empty box, a negative radius: no points, the largest distance */
        const CTAxisAlignedBox3D< Float64 > empty;
        ASSERT_FALSE( ClosestPoints( empty, box, pa, pb ) );
        ASSERT_FALSE( ClosestPoints( a, empty, pa, pb ) );
        ASSERT_TRUE( CTNumericTraits< Float64 >::Max() == Distance( box, empty ) );
        ASSERT_FALSE( ClosestPoints( CTSphere< Float64 >( P( 0, 0, 0 ), -1.0 ), a, pa, pb ) );
        ASSERT_FALSE( Intersects( CTSphere< Float64 >( P( 0, 0, 0 ), -1.0 ), a ) );
        ASSERT_FALSE( Intersects( empty, box ) );

        /* point cells: through the members of the shape */
        ASSERT_TRUE( NearTo( Distance( P( 3, 0, 0 ), a ), 2.0, 1e-12 ) && NearTo( Distance( a, P( 3, 0, 0 ) ), 2.0, 1e-12 ) );
        ASSERT_TRUE( ClosestPoints( P( 3, 0, 0 ), a, pa, pb ) && pa == P( 3, 0, 0 ) && NearTo( pb.x, 1.0, 1e-12 ) );
        ASSERT_TRUE( Intersects( P( 0.5, 0, 0 ), a ) && Intersects( a, P( 0.5, 0, 0 ) ) && !Intersects( P( 2, 0, 0 ), a ) );
        ASSERT_TRUE( Intersects( P( 0, 0, 0 ), box ) && Intersects( P( 0, 0, 0 ), CTOrientedBox3D< Float64 >( P( 0, 0, 0 ), CTQuaternion< Float64 >::Identity(), V( 1, 1, 1 ) ) ) );
        ASSERT_TRUE( NearTo( Distance( P( 4, 0, 0 ), CTCylinder< Float64 >( P( 0, 0, 0 ), P( 0, 0, 2 ), 1.0 ) ), 3.0, 1e-12 ) );

        /* segment against plane, zero-length segment */
        const CTPlane< Float64 > plane = CTPlane< Float64 >::FromPointNormal( P( 0, 1, 0 ), TFloat64DirectionVector3D( V( 0, 1, 0 ) ) );
        ASSERT_TRUE( ClosestPoints( CTSegment3D< Float64 >( P( 0, 3, 0 ), P( 0, 3, 0 ) ), plane, pa, pb ) && NearTo( pb.y, 1.0, 1e-12 ) && NearTo( Distance( CTSegment3D< Float64 >( P( 0, 3, 0 ), P( 0, 3, 0 ) ), plane ), 2.0, 1e-12 ) );
        ASSERT_TRUE( ClosestPoints( CTSegment3D< Float64 >( P( 0, 0, 0 ), P( 0, 2, 0 ) ), plane, pa, pb ) && NearTo( pa.y, 1.0, 1e-12 ) && pa == pb );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
