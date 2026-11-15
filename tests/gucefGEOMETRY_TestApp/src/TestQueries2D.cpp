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
#include <algorithm>

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestQueryReference.h"
#include "TestQueries2D.h"

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

typedef CTPoint2D< Float64 > Q;
typedef CTVector2D< Float64 > W;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static Q
RandomQ( CRandom& r, Float64 range )
{
    const Float64 x = r.Uniform( -range, range );
    const Float64 y = r.Uniform( -range, range );
    return Q( x, y );
}

/*-------------------------------------------------------------------------*/

static void
RandomShape2D( CRandom& r, CTSegment2D< Float64 >& s )
{
    const Q a = RandomQ( r, 1.5 );
    const Q b = r.Chance( 0.03 ) ? a : RandomQ( r, 1.5 );
    s = CTSegment2D< Float64 >( a, b );
}

static void
RandomShape2D( CRandom& r, CTCircle< Float64 >& c )
{
    const Q centre = RandomQ( r, 1.5 );
    const Float64 radius = r.Uniform( 0.2, 1.0 );
    c = CTCircle< Float64 >( centre, radius );
}

static void
RandomShape2D( CRandom& r, CTRectangle< Float64 >& rect )
{
    const Q centre = RandomQ( r, 1.5 );
    const Float64 hx = r.Uniform( 0.2, 1.0 );
    const Float64 hy = r.Uniform( 0.2, 1.0 );
    rect = CTRectangle< Float64 >::FromCentreHalfExtents( centre, W( hx, hy ) );
}

static void
RandomShape2D( CRandom& r, CTOrientedRectangle< Float64 >& rect )
{
    const Q centre = RandomQ( r, 1.5 );
    const Float64 angle = r.Uniform( 0.0, 6.283185307179586 );
    const Float64 hx = r.Uniform( 0.2, 1.0 );
    const Float64 hy = r.Uniform( 0.2, 1.0 );
    rect = CTOrientedRectangle< Float64 >( centre, CTRadian< Float64 >( angle ), W( hx, hy ) );
}

static void
RandomShape2D( CRandom& r, CTTriangle2D< Float64 >& t )
{
    const Q a = RandomQ( r, 1.5 );
    const Q b = RandomQ( r, 1.5 );
    const Q c = RandomQ( r, 1.5 );
    t = CTTriangle2D< Float64 >( a, b, c );
}

/** A convex polygon: the hull of seven random points (Andrew's monotone chain) */
static void
RandomConvexPolygon( CRandom& r, CTPolygon2D< Float64 >& polygon )
{
    GUCEF::vector< Q > points;
    for ( UInt32 i=0; i<7; ++i )
        points.push_back( RandomQ( r, 1.5 ) );
    struct SLess { static bool Compare( const Q& a, const Q& b ) { return a.x < b.x || ( a.x == b.x && a.y < b.y ); } };
    std::sort( points.begin(), points.end(), SLess::Compare );
    GUCEF::vector< Q > hull( 2 * points.size() );
    size_t k = 0;
    for ( size_t i=0; i<points.size(); ++i )
    {
        while ( k >= 2 && ( hull[ k - 1 ] - hull[ k - 2 ] ).Cross( points[ i ] - hull[ k - 2 ] ) <= 0 )
            --k;
        hull[ k++ ] = points[ i ];
    }
    for ( size_t i=points.size()-1, t=k+1; i>0; --i )
    {
        while ( k >= t && ( hull[ k - 1 ] - hull[ k - 2 ] ).Cross( points[ i - 1 ] - hull[ k - 2 ] ) <= 0 )
            --k;
        hull[ k++ ] = points[ i - 1 ];
    }
    polygon = CTPolygon2D< Float64 >( &hull[ 0 ], (UInt32) ( k - 1 ) );
}

static void
RandomShape2D( CRandom& r, CTPolygon2D< Float64 >& polygon )
{
    RandomConvexPolygon( r, polygon );
}

/** A star shaped polygon with radii that vary so much that it is not convex */
static void
RandomStarPolygon( CRandom& r, CTPolygon2D< Float64 >& polygon )
{
    const Q centre = RandomQ( r, 1.0 );
    const UInt32 n = 6 + (UInt32) r.Index( 5 );
    polygon.Clear();
    for ( UInt32 i=0; i<n; ++i )
    {
        const Float64 angle = 6.283185307179586 * ( (Float64) i + r.Uniform( 0.0, 0.8 ) ) / (Float64) n;
        const Float64 radius = ( 0 == i % 2 ) ? r.Uniform( 0.9, 1.4 ) : r.Uniform( 0.2, 0.5 );
        polygon.AddPoint( Q( centre.x + radius * cos( angle ), centre.y + radius * sin( angle ) ) );
    }
}

/*-------------------------------------------------------------------------*/

/** Alternating projections between two convex 2D shapes with ClosestPoint members; the distance of the final pair, 0 for shapes that overlap */
template < typename TA, typename TB >
static Float64
AlternatingProjection2D( const TA& a, const TB& b, Q& onA, Q& onB )
{
    Q x = a.GetBounds().GetCentre();
    Q y = b.ClosestPoint( x );
    for ( UInt32 pass=0; pass<2; ++pass )
    {
        Float64 previous = -1;
        for ( UInt32 i=0; i<( 0 == pass ? 3000u : 300000u ); ++i )
        {
            x = a.ClosestPoint( y );
            y = b.ClosestPoint( x );
            const Float64 d = ( y - x ).GetLength();
            if ( ( fabs( previous - d ) < 1.0e-15 && d > 0 ) || d < 1.0e-13 )
                break;
            previous = d;
        }
        const Float64 d = ( y - x ).GetLength();
        if ( d < 1.0e-9 || d > 0.05 )
            break;
    }
    onA = x;
    onB = y;
    return ( y - x ).GetLength();
}

/*-------------------------------------------------------------------------*/

struct SSweep2D
{
    UInt32 pairs;
    UInt32 overlapping;
    UInt32 separated;
    UInt32 bad;

    SSweep2D( void ) : pairs( 0 ), overlapping( 0 ), separated( 0 ), bad( 0 ) {}
};

/** Intersects, Distance and ClosestPoints of seeded pairs of convex shapes against alternating projections */
template < typename TA, typename TB >
static bool
Convex2DSweep( const char* name, UInt64 seed, UInt32 count, UInt32 minimumSeparated = 0 )
{
    CRandom random( seed );
    SSweep2D sweep;
    for ( UInt32 i=0; i<count; ++i )
    {
        TA a;
        TB b;
        RandomShape2D( random, a );
        RandomShape2D( random, b );
        ++sweep.pairs;
        Q refA;
        Q refB;
        const Float64 reference = AlternatingProjection2D( a, b, refA, refB );
        const bool got = Intersects( a, b );
        const bool gotSwapped = Intersects( b, a );
        Q pa;
        Q pb;
        const bool haveClosest = ClosestPoints( a, b, pa, pb );
        const Float64 distance = haveClosest ? ( pb - pa ).GetLength() : -1.0;
        bool ok = got == gotSwapped && haveClosest && NearTo( distance, (Float64) Distance( a, b ), 1.0e-12 );
        ok = ok && ( a.ClosestPoint( pa ) - pa ).GetLength() < 1.0e-9 && ( b.ClosestPoint( pb ) - pb ).GetLength() < 1.0e-9;
        if ( reference < 1.0e-9 )
        {
            ++sweep.overlapping;
            ok = ok && got && distance < 1.0e-9;
        }
        else if ( reference > 1.0e-3 )
        {
            ++sweep.separated;
            ok = ok && !got && distance >= 0 && distance <= reference + 1.0e-9;
            if ( ( a.ClosestPoint( refB ) - refA ).GetLength() + ( b.ClosestPoint( refA ) - refB ).GetLength() < 1.0e-9 )
                ok = ok && NearTo( distance, reference, 1.0e-6 );
        }
        if ( !ok )
        {
            ++sweep.bad;
            if ( sweep.bad < 4 )
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Queries2D: " << name << " got " << got << " distance " << distance << " reference " << reference << ", seed " << seed << " pair " << i );
        }
    }
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Queries2D: " << name << " seed " << seed << ": " << sweep.pairs << " pairs, " << sweep.overlapping << " overlapping, " << sweep.separated << " separated, " << sweep.bad << " disagreements" );
    return 0 == sweep.bad && sweep.overlapping > count / 20 && sweep.separated > ( 0 == minimumSeparated ? count / 20 : minimumSeparated );
}

/*-------------------------------------------------------------------------*/

/** Whether a point of a 2D shape is in the other, by sampling a grid: the reference for shapes that are not convex */
template < typename TA, typename TB >
static bool
GridWitness( const TA& a, const TB& b )
{
    const CTRectangle< Float64 > bounds = a.GetBounds().Intersection( b.GetBounds() );
    if ( bounds.IsEmpty() )
        return false;
    const UInt32 steps = 160;
    for ( UInt32 i=0; i<=steps; ++i )
    {
        for ( UInt32 j=0; j<=steps; ++j )
        {
            const Q p( bounds.min.x + ( bounds.max.x - bounds.min.x ) * (Float64) i / (Float64) steps, bounds.min.y + ( bounds.max.y - bounds.min.y ) * (Float64) j / (Float64) steps );
            if ( a.Contains( p ) && b.Contains( p ) )
                return true;
        }
    }
    return false;
}

/** A segment has no area: the witness is a point along it that the polygon contains */
static bool
GridWitness( const CTSegment2D< Float64 >& segment, const CTPolygon2D< Float64 >& polygon )
{
    for ( UInt32 i=0; i<=4000; ++i )
    {
        if ( polygon.Contains( segment.GetPoint( (Float64) i / 4000.0 ) ) )
            return true;
    }
    return false;
}

/** The smallest distance between the boundaries of two shapes, by sampling the points of one and asking the other for the nearest point */
template < typename TA, typename TB >
static Float64
BoundarySampleDistance( const TA& a, const TB& b, CRandom& random )
{
    const CTRectangle< Float64 > bounds = a.GetBounds();
    Float64 best = 1.0e300;
    for ( UInt32 i=0; i<4000; ++i )
    {
        const Q p( random.Uniform( bounds.min.x, bounds.max.x ), random.Uniform( bounds.min.y, bounds.max.y ) );
        if ( a.Contains( p ) )
            best = fmin( best, ( b.ClosestPoint( p ) - p ).GetLength() );
    }
    return best;
}

static Float64
BoundarySampleDistance( const CTSegment2D< Float64 >& segment, const CTPolygon2D< Float64 >& polygon, CRandom& )
{
    Float64 best = 1.0e300;
    for ( UInt32 i=0; i<=400; ++i )
    {
        const Q p = segment.GetPoint( (Float64) i / 400.0 );
        best = fmin( best, ( polygon.ClosestPoint( p ) - p ).GetLength() );
    }
    return best;
}

/** Shapes against a star shaped (concave) polygon: overlap by grid witnesses on both sides, apart by the absence of witnesses and a margin */
template < typename TShape >
static bool
ConcaveSweep( const char* name, UInt64 seed, UInt32 count )
{
    CRandom random( seed );
    UInt32 bad = 0;
    UInt32 yes = 0;
    UInt32 no = 0;
    for ( UInt32 i=0; i<count; ++i )
    {
        TShape shape;
        RandomShape2D( random, shape );
        CTPolygon2D< Float64 > star;
        RandomStarPolygon( random, star );
        if ( star.IsConvex() )
            continue;
        const bool got = Intersects( shape, star );
        if ( got != Intersects( star, shape ) )
            ++bad;
        const bool witness = GridWitness( shape, star );
        if ( witness )
        {
            ++yes;
            if ( !got )
                ++bad;
        }
        else
        {
            /* no common grid point: apart when the boundaries are far enough, else not decided */
            const Float64 gap = fmin( BoundarySampleDistance( shape, star, random ), BoundarySampleDistance( star, shape, random ) );
            if ( gap > 0.06 )
            {
                ++no;
                if ( got )
                    ++bad;
            }
        }
    }
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Queries2D: " << name << " against a concave polygon, seed " << seed << ": " << yes << " overlapping and " << no << " apart checked, " << bad << " disagreements" );
    return 0 == bad && yes > 20 && no > 20;
}

/*-------------------------------------------------------------------------*/

/** The expected kind of intersection of two segments from the parameters, by solving the two linear equations in Float64 */
static void
ReferenceIntersection( const CTSegment2D< Float64 >& a, const CTSegment2D< Float64 >& b, Float64& t, Float64& u, bool& parallel )
{
    const W r = a.end - a.start;
    const W s = b.end - b.start;
    const Float64 denominator = r.Cross( s );
    parallel = 0.0 == denominator;
    if ( parallel )
        return;
    const W qp = b.start - a.start;
    t = qp.Cross( s ) / denominator;
    u = qp.Cross( r ) / denominator;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformQueries2DTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "Queries2D" );

    typedef CTSegment2D< Float64 > Segment;

    GUCEF_TESTFW_TESTCASE( "Queries2D: segment-segment intersection: crossing, touching, collinear overlap, touching and apart, parallel, zero length" )
    try
    {
        SSegmentIntersection2D< Float64 > hit;
        /* crossing */
        ASSERT_TRUE( SEGMENTINTERSECTION_POINT == IntersectSegments( Segment( Q( -1, 0 ), Q( 1, 0 ) ), Segment( Q( 0, -1 ), Q( 0, 1 ) ), hit ) );
        ASSERT_TRUE( NearTo( hit.first.x, 0.0, 1e-12 ) && NearTo( hit.first.y, 0.0, 1e-12 ) && hit.first == hit.second && NearTo( hit.t, 0.5, 1e-12 ) && NearTo( hit.u, 0.5, 1e-12 ) );
        ASSERT_TRUE( Intersects( Segment( Q( -1, 0 ), Q( 1, 0 ) ), Segment( Q( 0, -1 ), Q( 0, 1 ) ) ) );
        /* a T: the end of one on the interior of the other, and end to end */
        ASSERT_TRUE( SEGMENTINTERSECTION_POINT == IntersectSegments( Segment( Q( 0, 0 ), Q( 4, 0 ) ), Segment( Q( 1, 0 ), Q( 1, 3 ) ), hit ) && NearTo( hit.first.x, 1.0, 1e-12 ) && NearTo( hit.t, 0.25, 1e-12 ) && NearTo( hit.u, 0.0, 1e-12 ) );
        ASSERT_TRUE( SEGMENTINTERSECTION_POINT == IntersectSegments( Segment( Q( 0, 0 ), Q( 1, 0 ) ), Segment( Q( 1, 0 ), Q( 1, 1 ) ), hit ) && NearTo( hit.first.x, 1.0, 1e-12 ) && NearTo( hit.t, 1.0, 1e-12 ) );
        ASSERT_TRUE( Intersects( Segment( Q( 0, 0 ), Q( 1, 0 ) ), Segment( Q( 1, 0 ), Q( 1, 1 ) ) ) );
        /* near misses */
        ASSERT_TRUE( SEGMENTINTERSECTION_NONE == IntersectSegments( Segment( Q( 0, 0 ), Q( 1, 0 ) ), Segment( Q( 1.001, 0 ), Q( 1.001, 1 ) ), hit ) );
        ASSERT_FALSE( Intersects( Segment( Q( 0, 0 ), Q( 1, 0 ) ), Segment( Q( 1.001, 0 ), Q( 1.001, 1 ) ) ) );
        ASSERT_TRUE( SEGMENTINTERSECTION_NONE == IntersectSegments( Segment( Q( 0, 0 ), Q( 1, 0 ) ), Segment( Q( 0.5, 0.001 ), Q( 0.5, 1 ) ), hit ) );

        /* collinear: overlapping in both directions, one inside the other, touching at an end, apart */
        ASSERT_TRUE( SEGMENTINTERSECTION_OVERLAP == IntersectSegments( Segment( Q( 0, 0 ), Q( 2, 0 ) ), Segment( Q( 1, 0 ), Q( 3, 0 ) ), hit ) );
        ASSERT_TRUE( NearTo( hit.first.x, 1.0, 1e-12 ) && NearTo( hit.second.x, 2.0, 1e-12 ) && NearTo( hit.first.y, 0.0, 1e-12 ) );
        ASSERT_TRUE( SEGMENTINTERSECTION_OVERLAP == IntersectSegments( Segment( Q( 0, 0 ), Q( 2, 0 ) ), Segment( Q( 3, 0 ), Q( 1, 0 ) ), hit ) && NearTo( hit.first.x, 1.0, 1e-12 ) && NearTo( hit.second.x, 2.0, 1e-12 ) );
        ASSERT_TRUE( SEGMENTINTERSECTION_OVERLAP == IntersectSegments( Segment( Q( 0, 0 ), Q( 4, 4 ) ), Segment( Q( 1, 1 ), Q( 2, 2 ) ), hit ) && NearTo( hit.first.x, 1.0, 1e-12 ) && NearTo( hit.second.x, 2.0, 1e-12 ) );
        ASSERT_TRUE( SEGMENTINTERSECTION_OVERLAP == IntersectSegments( Segment( Q( 1, 1 ), Q( 2, 2 ) ), Segment( Q( 0, 0 ), Q( 4, 4 ) ), hit ) && NearTo( hit.first.x, 1.0, 1e-12 ) && NearTo( hit.second.x, 2.0, 1e-12 ) );
        ASSERT_TRUE( SEGMENTINTERSECTION_POINT == IntersectSegments( Segment( Q( 0, 0 ), Q( 1, 0 ) ), Segment( Q( 1, 0 ), Q( 2, 0 ) ), hit ) && NearTo( hit.first.x, 1.0, 1e-12 ) && hit.first == hit.second );
        ASSERT_TRUE( SEGMENTINTERSECTION_POINT == IntersectSegments( Segment( Q( 0, 0 ), Q( 1, 0 ) ), Segment( Q( 1, 0 ), Q( 0.5, 0 ) ), hit ) || SEGMENTINTERSECTION_OVERLAP == IntersectSegments( Segment( Q( 0, 0 ), Q( 1, 0 ) ), Segment( Q( 1, 0 ), Q( 0.5, 0 ) ), hit ) );
        ASSERT_TRUE( SEGMENTINTERSECTION_NONE == IntersectSegments( Segment( Q( 0, 0 ), Q( 1, 0 ) ), Segment( Q( 2, 0 ), Q( 3, 0 ) ), hit ) );
        ASSERT_TRUE( Intersects( Segment( Q( 0, 0 ), Q( 2, 0 ) ), Segment( Q( 1, 0 ), Q( 3, 0 ) ) ) && !Intersects( Segment( Q( 0, 0 ), Q( 1, 0 ) ), Segment( Q( 2, 0 ), Q( 3, 0 ) ) ) );
        ASSERT_TRUE( SEGMENTINTERSECTION_OVERLAP == IntersectSegments( Segment( Q( 0, 0 ), Q( 2, 0 ) ), Segment( Q( 0, 0 ), Q( 2, 0 ) ), hit ) );          /* identical */

        /* parallel and apart, nearly parallel crossing */
        ASSERT_TRUE( SEGMENTINTERSECTION_NONE == IntersectSegments( Segment( Q( 0, 0 ), Q( 2, 0 ) ), Segment( Q( 0, 1 ), Q( 2, 1 ) ), hit ) );
        ASSERT_TRUE( SEGMENTINTERSECTION_NONE == IntersectSegments( Segment( Q( 0, 0 ), Q( 2, 0 ) ), Segment( Q( 0, 0.001 ), Q( 2, 0.001 ) ), hit ) );
        ASSERT_TRUE( SEGMENTINTERSECTION_POINT == IntersectSegments( Segment( Q( 0, 0 ), Q( 2, 0.001 ) ), Segment( Q( 0, 0.001 ), Q( 2, 0 ) ), hit ) && NearTo( hit.first.x, 1.0, 1e-9 ) );
        ASSERT_FALSE( Intersects( Segment( Q( 0, 0 ), Q( 2, 0 ) ), Segment( Q( 0, 1 ), Q( 2, 1 ) ) ) );

        /* zero length: a point on, off, and at the end of a segment, two points */
        ASSERT_TRUE( SEGMENTINTERSECTION_POINT == IntersectSegments( Segment( Q( 1, 1 ), Q( 1, 1 ) ), Segment( Q( 0, 0 ), Q( 2, 2 ) ), hit ) && NearTo( hit.first.x, 1.0, 1e-12 ) );
        ASSERT_TRUE( SEGMENTINTERSECTION_POINT == IntersectSegments( Segment( Q( 0, 0 ), Q( 2, 2 ) ), Segment( Q( 2, 2 ), Q( 2, 2 ) ), hit ) );
        ASSERT_TRUE( SEGMENTINTERSECTION_NONE == IntersectSegments( Segment( Q( 1, 1.1 ), Q( 1, 1.1 ) ), Segment( Q( 0, 0 ), Q( 2, 2 ) ), hit ) );
        ASSERT_TRUE( SEGMENTINTERSECTION_POINT == IntersectSegments( Segment( Q( 1, 1 ), Q( 1, 1 ) ), Segment( Q( 1, 1 ), Q( 1, 1 ) ), hit ) && SEGMENTINTERSECTION_NONE == IntersectSegments( Segment( Q( 1, 1 ), Q( 1, 1 ) ), Segment( Q( 1, 2 ), Q( 1, 2 ) ), hit ) );
        ASSERT_TRUE( Intersects( Segment( Q( 1, 1 ), Q( 1, 1 ) ), Segment( Q( 0, 0 ), Q( 2, 2 ) ) ) && !Intersects( Segment( Q( 1, 1.1 ), Q( 1, 1.1 ) ), Segment( Q( 0, 0 ), Q( 2, 2 ) ) ) );

        /* the same cases in Float32 through the exported instantiation */
        typedef CTSegment2D< Float32 > Segment32;
        typedef CTPoint2D< Float32 > Q32;
        SSegmentIntersection2D< Float32 > hit32;
        ASSERT_TRUE( SEGMENTINTERSECTION_OVERLAP == IntersectSegments( Segment32( Q32( 0, 0 ), Q32( 2, 0 ) ), Segment32( Q32( 1, 0 ), Q32( 3, 0 ) ), hit32 ) && hit32.first.x == 1.0f && hit32.second.x == 2.0f );
        ASSERT_TRUE( SEGMENTINTERSECTION_NONE == IntersectSegments( Segment32( Q32( 0, 0 ), Q32( 2, 0 ) ), Segment32( Q32( 0, 1 ), Q32( 2, 1 ) ), hit32 ) );
        ASSERT_TRUE( SEGMENTINTERSECTION_POINT == IntersectSegments( Segment32( Q32( -1, 0 ), Q32( 1, 0 ) ), Segment32( Q32( 0, -1 ), Q32( 0, 1 ) ), hit32 ) && hit32.first.x == 0.0f );

        /* 10000 seeded pairs: the kind of intersection and the point against the linear equations in Float64, both orders */
        CRandom random( 1001 );
        UInt32 bad = 0;
        UInt32 points = 0;
        for ( UInt32 i=0; i<10000; ++i )
        {
            const Segment a( RandomQ( random, 1.5 ), RandomQ( random, 1.5 ) );
            const Segment b( RandomQ( random, 1.5 ), RandomQ( random, 1.5 ) );
            Float64 t = 0;
            Float64 u = 0;
            bool parallel = false;
            ReferenceIntersection( a, b, t, u, parallel );
            if ( parallel )
                continue;
            const Float64 margin = fmin( fmin( t, 1.0 - t ), fmin( u, 1.0 - u ) );
            SSegmentIntersection2D< Float64 > found;
            const TSegmentIntersection2D kind = IntersectSegments( a, b, found );
            if ( fabs( margin ) < 1.0e-6 )
                continue;
            if ( ( margin > 0 ) != ( SEGMENTINTERSECTION_POINT == kind ) || ( margin > 0 ) != Intersects( a, b ) || ( margin > 0 ) != Intersects( b, a ) )
                ++bad;
            else if ( margin > 0 )
            {
                ++points;
                if ( ( found.first - a.GetPoint( t ) ).GetLength() > 1.0e-9 )
                    ++bad;
            }
        }
        ASSERT_TRUE( 0 == bad && points > 1500 );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Queries2D: circle against circle, segment, rectangle, oriented rectangle, triangle and polygon" )
    try
    {
        ASSERT_TRUE( ( Convex2DSweep< CTCircle< Float64 >, CTCircle< Float64 > >( "circle-circle", 1011, 5000 ) ) );
        ASSERT_TRUE( ( Convex2DSweep< CTSegment2D< Float64 >, CTCircle< Float64 > >( "segment-circle", 1012, 5000 ) ) );
        ASSERT_TRUE( ( Convex2DSweep< CTCircle< Float64 >, CTRectangle< Float64 > >( "circle-rectangle", 1013, 5000 ) ) );
        ASSERT_TRUE( ( Convex2DSweep< CTCircle< Float64 >, CTOrientedRectangle< Float64 > >( "circle-oriented", 1014, 5000 ) ) );
        ASSERT_TRUE( ( Convex2DSweep< CTCircle< Float64 >, CTTriangle2D< Float64 > >( "circle-triangle", 1015, 5000 ) ) );
        ASSERT_TRUE( ( Convex2DSweep< CTCircle< Float64 >, CTPolygon2D< Float64 > >( "circle-polygon", 1016, 5000 ) ) );
        const CTCircle< Float64 > circle( Q( 0, 0 ), 1.0 );
        const CTRectangle< Float64 > rect( Q( 1, -1 ), Q( 3, 1 ) );
        ASSERT_TRUE( Intersects( circle, rect ) && Intersects( rect, circle ) );                                           /* touching at ( 1, 0 ) */
        ASSERT_TRUE( !Intersects( CTCircle< Float64 >( Q( 0, 0 ), 0.999 ), rect ) );
        ASSERT_TRUE( Intersects( CTCircle< Float64 >( Q( 2, 2 ), 1.0 ), rect ) && !Intersects( CTCircle< Float64 >( Q( 4, 2 ), 1.0 ), rect ) );
        Q onCircle;
        Q onRect;
        ASSERT_TRUE( ClosestPoints( CTCircle< Float64 >( Q( 5, 0 ), 1.0 ), rect, onCircle, onRect ) && NearTo( onCircle.x, 4.0, 1e-12 ) && NearTo( onRect.x, 3.0, 1e-12 ) && NearTo( Distance( CTCircle< Float64 >( Q( 5, 0 ), 1.0 ), rect ), 1.0, 1e-12 ) );
        ASSERT_FALSE( Intersects( CTCircle< Float64 >( Q( 0, 0 ), -1.0 ), rect ) );
        ASSERT_FALSE( ClosestPoints( CTCircle< Float64 >( Q( 0, 0 ), -1.0 ), rect, onCircle, onRect ) );
        ASSERT_FALSE( Intersects( CTRectangle< Float64 >(), circle ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Queries2D: separating axis test of segments, rectangles, oriented rectangles, triangles and convex polygons against alternating projections" )
    try
    {
        ASSERT_TRUE( ( Convex2DSweep< Segment, CTRectangle< Float64 > >( "segment-rectangle", 1021, 5000 ) ) );
        ASSERT_TRUE( ( Convex2DSweep< Segment, CTOrientedRectangle< Float64 > >( "segment-oriented", 1022, 5000 ) ) );
        ASSERT_TRUE( ( Convex2DSweep< Segment, CTTriangle2D< Float64 > >( "segment-triangle", 1023, 5000 ) ) );
        ASSERT_TRUE( ( Convex2DSweep< Segment, CTPolygon2D< Float64 > >( "segment-polygon", 1024, 5000 ) ) );
        ASSERT_TRUE( ( Convex2DSweep< CTRectangle< Float64 >, CTRectangle< Float64 > >( "rectangle-rectangle", 1025, 5000 ) ) );
        ASSERT_TRUE( ( Convex2DSweep< CTRectangle< Float64 >, CTOrientedRectangle< Float64 > >( "rectangle-oriented", 1026, 5000 ) ) );
        ASSERT_TRUE( ( Convex2DSweep< CTOrientedRectangle< Float64 >, CTOrientedRectangle< Float64 > >( "oriented-oriented", 1027, 5000 ) ) );
        ASSERT_TRUE( ( Convex2DSweep< CTRectangle< Float64 >, CTTriangle2D< Float64 > >( "rectangle-triangle", 1028, 5000 ) ) );
        ASSERT_TRUE( ( Convex2DSweep< CTOrientedRectangle< Float64 >, CTTriangle2D< Float64 > >( "oriented-triangle", 1029, 5000 ) ) );
        ASSERT_TRUE( ( Convex2DSweep< CTTriangle2D< Float64 >, CTTriangle2D< Float64 > >( "triangle-triangle", 1030, 5000 ) ) );
        ASSERT_TRUE( ( Convex2DSweep< CTRectangle< Float64 >, CTPolygon2D< Float64 > >( "rectangle-polygon", 1031, 5000 ) ) );
        ASSERT_TRUE( ( Convex2DSweep< CTTriangle2D< Float64 >, CTPolygon2D< Float64 > >( "triangle-polygon", 1032, 5000 ) ) );
        ASSERT_TRUE( ( Convex2DSweep< CTPolygon2D< Float64 >, CTPolygon2D< Float64 > >( "polygon-polygon", 1033, 5000, 20 ) ) );
        ASSERT_TRUE( ( Convex2DSweep< CTOrientedRectangle< Float64 >, CTPolygon2D< Float64 > >( "oriented-polygon", 1034, 5000 ) ) );

        /* touching: a shared edge, a shared corner, and a gap of 1e-3 */
        const CTRectangle< Float64 > unit( Q( 0, 0 ), Q( 1, 1 ) );
        ASSERT_TRUE( Intersects( unit, CTRectangle< Float64 >( Q( 1, 0 ), Q( 2, 1 ) ) ) && Intersects( unit, CTRectangle< Float64 >( Q( 1, 1 ), Q( 2, 2 ) ) ) );
        ASSERT_TRUE( !Intersects( unit, CTRectangle< Float64 >( Q( 1.001, 0 ), Q( 2, 1 ) ) ) );
        const CTOrientedRectangle< Float64 > diamond( Q( 2.5, 0.5 ), CTRadian< Float64 >( 0.7853981633974483 ), W( 0.7071067811865476, 0.7071067811865476 ) );
        ASSERT_TRUE( Intersects( diamond, CTRectangle< Float64 >( Q( 0, 0 ), Q( 1.5001, 1 ) ) ) && !Intersects( diamond, CTRectangle< Float64 >( Q( 0, 0 ), Q( 1.499, 1 ) ) ) );
        const CTTriangle2D< Float64 > tri( Q( 0, 0 ), Q( 4, 0 ), Q( 0, 4 ) );
        ASSERT_TRUE( Intersects( tri, CTRectangle< Float64 >( Q( 1, 1 ), Q( 2, 2 ) ) ) && Intersects( tri, CTRectangle< Float64 >( Q( 2, 2 ), Q( 3, 3 ) ) ) && !Intersects( tri, CTRectangle< Float64 >( Q( 2.01, 2.01 ), Q( 3, 3 ) ) ) );
        /* a polygon given clockwise, a segment crossing a rectangle without an end point inside, an empty rectangle */
        CTPolygon2D< Float64 > clockwise;
        clockwise.AddPoint( Q( 0, 0 ) );
        clockwise.AddPoint( Q( 0, 1 ) );
        clockwise.AddPoint( Q( 1, 1 ) );
        clockwise.AddPoint( Q( 1, 0 ) );
        ASSERT_TRUE( Intersects( clockwise, CTRectangle< Float64 >( Q( 0.5, 0.5 ), Q( 3, 3 ) ) ) && !Intersects( clockwise, CTRectangle< Float64 >( Q( 1.5, 0.5 ), Q( 3, 3 ) ) ) );
        ASSERT_TRUE( Intersects( Segment( Q( -1, 0.5 ), Q( 2, 0.5 ) ), unit ) && !Intersects( Segment( Q( -1, 1.5 ), Q( 2, 1.5 ) ), unit ) );
        ASSERT_FALSE( Intersects( CTRectangle< Float64 >(), unit ) || Intersects( unit, CTRectangle< Float64 >() ) );
        Q pa;
        Q pb;
        ASSERT_FALSE( ClosestPoints( CTRectangle< Float64 >(), unit, pa, pb ) );
        ASSERT_FALSE( Intersects( CTPolygon2D< Float64 >(), unit ) );

        /* the Float32 instantiation */
        typedef CTPoint2D< Float32 > Q32;
        const CTRectangle< Float32 > a32( Q32( 0, 0 ), Q32( 1, 1 ) );
        const CTRectangle< Float32 > b32( Q32( 1, 0 ), Q32( 2, 1 ) );
        const CTTriangle2D< Float32 > t32( Q32( 3, 0 ), Q32( 4, 0 ), Q32( 3, 1 ) );
        Q32 pa32;
        Q32 pb32;
        ASSERT_TRUE( Intersects( a32, b32 ) && !Intersects( a32, t32 ) && ClosestPoints( b32, t32, pa32, pb32 ) && pa32.x == 2.0f && pb32.x == 3.0f && Distance( b32, t32 ) == 1.0f );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Queries2D: shapes against a concave polygon: overlap by grid witnesses, apart by margins" )
    try
    {
        ASSERT_TRUE( ConcaveSweep< Segment >( "segment", 1041, 600 ) );
        ASSERT_TRUE( ConcaveSweep< CTCircle< Float64 > >( "circle", 1042, 600 ) );
        ASSERT_TRUE( ConcaveSweep< CTRectangle< Float64 > >( "rectangle", 1043, 600 ) );
        ASSERT_TRUE( ConcaveSweep< CTTriangle2D< Float64 > >( "triangle", 1044, 600 ) );
        ASSERT_TRUE( ConcaveSweep< CTOrientedRectangle< Float64 > >( "oriented rectangle", 1045, 600 ) );
        /* a U shaped polygon: the segment inside the notch does not touch it */
        CTPolygon2D< Float64 > u;
        const Q ring[ 8 ] = { Q( 0, 0 ), Q( 3, 0 ), Q( 3, 3 ), Q( 2, 3 ), Q( 2, 1 ), Q( 1, 1 ), Q( 1, 3 ), Q( 0, 3 ) };
        for ( UInt32 i=0; i<8; ++i )
            u.AddPoint( ring[ i ] );
        ASSERT_FALSE( u.IsConvex() );
        ASSERT_FALSE( Intersects( Segment( Q( 1.2, 1.5 ), Q( 1.8, 2.5 ) ), u ) );
        ASSERT_TRUE( Intersects( Segment( Q( 1.2, 0.5 ), Q( 1.8, 2.5 ) ), u ) );
        ASSERT_FALSE( Intersects( CTCircle< Float64 >( Q( 1.5, 2.0 ), 0.4 ), u ) );
        ASSERT_TRUE( Intersects( CTCircle< Float64 >( Q( 1.5, 2.0 ), 0.6 ), u ) );
        ASSERT_FALSE( Intersects( CTRectangle< Float64 >( Q( 1.1, 1.5 ), Q( 1.9, 2.9 ) ), u ) );
        ASSERT_TRUE( Intersects( CTRectangle< Float64 >( Q( 0.9, 1.5 ), Q( 1.9, 2.9 ) ), u ) );
        ASSERT_TRUE( Intersects( CTRectangle< Float64 >( Q( 1.2, 1.2 ), Q( 1.8, 1.8 ) ), CTRectangle< Float64 >( Q( 1.5, 1.5 ), Q( 4, 4 ) ) ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Queries2D: segment-segment nearest points against dense sampling and the point cells" )
    try
    {
        CRandom random( 1051 );
        UInt32 bad = 0;
        for ( UInt32 i=0; i<3000; ++i )
        {
            const Segment a( RandomQ( random, 1.5 ), RandomQ( random, 1.5 ) );
            const Segment b = 0 == i % 40 ? Segment( a.start, a.start ) : Segment( RandomQ( random, 1.5 ), RandomQ( random, 1.5 ) );
            Q pa;
            Q pb;
            if ( !ClosestPoints( a, b, pa, pb ) )
            {
                ++bad;
                continue;
            }
            const Float64 d = ( pb - pa ).GetLength();
            if ( a.GetDistance( pa ) > 1e-12 || b.GetDistance( pb ) > 1e-12 || !NearTo( d, (Float64) Distance( a, b ), 1e-12 ) )
                ++bad;
            Float64 best = 1.0e300;
            for ( UInt32 k=0; k<=200; ++k )
            {
                const Float64 t = (Float64) k / 200.0;
                best = fmin( best, ( b.ClosestPoint( a.GetPoint( t ) ) - a.GetPoint( t ) ).GetLength() );
            }
            if ( d > best + 1e-12 || d < best - 0.02 )
                ++bad;
        }
        ASSERT_TRUE( 0 == bad );
        ASSERT_TRUE( Intersects( Q( 0.5, 0.5 ), CTRectangle< Float64 >( Q( 0, 0 ), Q( 1, 1 ) ) ) && Intersects( CTCircle< Float64 >( Q( 0, 0 ), 1.0 ), Q( 0.5, 0 ) ) );
        ASSERT_TRUE( NearTo( Distance( Q( 3, 0 ), CTCircle< Float64 >( Q( 0, 0 ), 1.0 ) ), 2.0, 1e-12 ) && NearTo( Distance( CTCircle< Float64 >( Q( 0, 0 ), 1.0 ), Q( 3, 0 ) ), 2.0, 1e-12 ) );
        Q onPoint;
        Q onShape;
        ASSERT_TRUE( ClosestPoints( Q( 3, 0 ), CTCircle< Float64 >( Q( 0, 0 ), 1.0 ), onPoint, onShape ) && onPoint == Q( 3, 0 ) && NearTo( onShape.x, 1.0, 1e-12 ) );
        ASSERT_TRUE( ClosestPoints( CTCircle< Float64 >( Q( 0, 0 ), 1.0 ), Q( 3, 0 ), onShape, onPoint ) && onPoint == Q( 3, 0 ) && NearTo( onShape.x, 1.0, 1e-12 ) );
        /* overlapping circles: one witness in both */
        ASSERT_TRUE( ClosestPoints( CTCircle< Float64 >( Q( 0, 0 ), 1.0 ), CTCircle< Float64 >( Q( 0.5, 0 ), 2.0 ), onPoint, onShape ) && onPoint == onShape &&
                     CTCircle< Float64 >( Q( 0, 0 ), 1.0 ).Contains( onPoint ) && CTCircle< Float64 >( Q( 0.5, 0 ), 2.0 ).Contains( onShape ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
