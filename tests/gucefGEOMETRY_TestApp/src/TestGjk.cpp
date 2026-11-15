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

#ifndef GUCEF_GEOMETRY_SWEEPQUERIES_H
#include "gucefGEOMETRY_SweepQueries.h"
#define GUCEF_GEOMETRY_SWEEPQUERIES_H
#endif /* GUCEF_GEOMETRY_SWEEPQUERIES_H ? */

#ifndef GUCEF_GEOMETRY_TIMEOFIMPACT_H
#include "gucefGEOMETRY_TimeOfImpact.h"
#define GUCEF_GEOMETRY_TIMEOFIMPACT_H
#endif /* GUCEF_GEOMETRY_TIMEOFIMPACT_H ? */

#ifndef GUCEF_GEOMETRY_BOUNDINGVOLUMEFITTING_H
#include "gucefGEOMETRY_BoundingVolumeFitting.h"
#define GUCEF_GEOMETRY_BOUNDINGVOLUMEFITTING_H
#endif /* GUCEF_GEOMETRY_BOUNDINGVOLUMEFITTING_H ? */

#include "TestGeometryReference.h"
#include "TestConvexReference.h"
#include "TestGjk.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

/*
 *  Functions marked GUCEF_TEST_CONTRACTING are compiled for FMA with floating-point contraction enabled (see
 *  TestQueryFpContraction.cpp, which uses the same construction).
 */
#if ( defined( __GNUC__ ) || defined( __clang__ ) ) && ( defined( __x86_64__ ) || defined( __i386__ ) )
  #define GUCEF_TEST_GJK_CONTRACTION_SUPPORTED
  #if defined( __clang__ )
    #define GUCEF_TEST_GJK_CONTRACTING  __attribute__(( target( "fma" ), noinline ))
    #define GUCEF_TEST_GJK_CONTRACT_HERE  _Pragma( "clang fp contract(fast)" )
  #else
    #define GUCEF_TEST_GJK_CONTRACTING  __attribute__(( target( "fma" ), optimize( "fp-contract=fast" ), noinline ))
    #define GUCEF_TEST_GJK_CONTRACT_HERE
  #endif
#endif

using namespace GUCEF;
using namespace GUCEF::MATH;
using namespace GUCEF::GEOMETRY;
using namespace QueryReference;
using namespace ConvexReference;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The pinned FNV-1a hash of the TFixed32_32 GJK run: identical in every build (gcc, clang, ASan, GUCEF_MATH_INLINE_ALL) */
static const UInt64 PinnedFixedGjkHash = 7671413778832220455ULL;

/*-------------------------------------------------------------------------*/

/** GJK distance of two shapes through their support mappings (the GJK path even where the facade has an analytic cell) */
template < typename T, typename TA, typename TB >
static SGjkResult< T >
GjkOf( const TA& a, const TB& b )
{
    SGjkResult< T > result;
    CTGjk< T >::ComputeDistance( CTSupportMapping< TA >::Make( a ), CTSupportMapping< TB >::Make( b ), CTGeometryTolerance< T >(), GUCEF_NULL, result );
    return result;
}

/*-------------------------------------------------------------------------*/

/** 'count' seeded sphere pairs: the GJK distance against |c1 - c2| - r1 - r2 within relativeError * max( |c1 - c2|, 0.01 ) */
template < typename T >
static bool
SphereDistanceSweep( UInt64 seed, UInt32 count, Float64 relativeError )
{
    CRandom random( seed );
    UInt32 bad = 0;
    UInt32 separated = 0;
    UInt32 overlapping = 0;
    UInt32 maxIterations = 0;
    Float64 worst = 0.0;
    for ( UInt32 i=0; i<count; ++i )
    {
        CTSphere< T > a;
        CTSphere< T > b;
        RandomShape( random, a );
        RandomShape( random, b );
        const CTSphere< Float64 > wa = Widen( a );
        const CTSphere< Float64 > wb = Widen( b );
        const Float64 between = ( wa.centre - wb.centre ).GetLength();
        const Float64 expected = between - wa.radius - wb.radius;
        const SGjkResult< T > result = GjkOf< T >( a, b );
        maxIterations = result.iterations > maxIterations ? result.iterations : maxIterations;
        const Float64 allowed = relativeError * ( between > 0.01 ? between : 0.01 );
        if ( expected > allowed )
        {
            ++separated;
            const Float64 error = fabs( (Float64) result.distance - expected );
            worst = error > worst ? error : worst;
            if ( error > allowed || result.overlapping )
            {
                ++bad;
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Gjk: sphere distance " << (Float64) result.distance << " expected " << expected << ", seed " << seed << " pair " << i );
            }
        }
        else if ( expected < -allowed )
        {
            ++overlapping;
            if ( !result.overlapping || (Float64) result.distance != 0.0 )
            {
                ++bad;
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Gjk: overlapping spheres reported apart, seed " << seed << " pair " << i );
            }
        }
    }
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Gjk: sphere sweep seed " << seed << ": " << separated << " separated, " << overlapping << " overlapping, worst error " << worst
               << ", at most " << maxIterations << " iterations, " << bad << " bad" );
    return 0 == bad && separated > count / 10 && overlapping > count / 10 && maxIterations <= (UInt32) CTGjk< T >::MaxIterations;
}

/*-------------------------------------------------------------------------*/

/** A random hull of the kind T (6 to 12 points), with or without faces, and its brute-force polytope from the rounded vertices */
template < typename T >
static void
RandomHull( CRandom& random, bool withFaces, CTConvexHull3D< T >& hull, SPolytope& polytope )
{
    const P3 centre = random.Point( 1.5 );
    const Float64 radius = random.Uniform( 0.4, 1.2 );
    const UInt32 count = 6 + (UInt32) random.Index( 7 );
    const SPolytope raw = BruteForceHull( RandomPointCloud( random, centre, radius, count ) );
    hull = ToHull< T >( raw, withFaces );
    GUCEF::vector< P3 > rounded;
    for ( UInt32 i=0; i<hull.GetVertexCount(); ++i )
        rounded.push_back( Widen( hull.GetVertex( i ) ) );
    polytope = BruteForceHull( rounded );
}

/*-------------------------------------------------------------------------*/

/** The counts of a polytope sweep */
struct SPolytopeSweep
{
    UInt32 separated;
    UInt32 overlapping;
    UInt32 undecided;
    UInt32 bad;
    Float64 worst;
    UInt32 maxIterations;

    SPolytopeSweep( void ) : separated( 0 ), overlapping( 0 ), undecided( 0 ), bad( 0 ), worst( 0.0 ), maxIterations( 0 ) {}
};

/*-------------------------------------------------------------------------*/

/** Checks one polytope pair against the brute-force distance (separated by more than 1e-3) or the SAT reference (overlapping) */
template < typename T >
static void
CheckPolytopePair( const char* name, const SGjkResult< T >& result, const SPolytope& a, const SPolytope& b, Float64 relativeError, UInt64 seed, UInt32 index, SPolytopeSweep& sweep )
{
    sweep.maxIterations = result.iterations > sweep.maxIterations ? result.iterations : sweep.maxIterations;
    const Float64 separation = PolytopeSeparation( a, b );
    if ( separation > 1.0e-3 )
    {
        ++sweep.separated;
        const Float64 expected = PolytopeDistance( a, b );
        const Float64 error = fabs( (Float64) result.distance - expected );
        sweep.worst = error > sweep.worst ? error : sweep.worst;
        if ( error > relativeError * ( expected > 1.0 ? expected : 1.0 ) || result.overlapping )
        {
            ++sweep.bad;
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Gjk: " << name << " distance " << (Float64) result.distance << " expected " << expected << ", seed " << seed << " pair " << index );
        }
    }
    else if ( separation < -1.0e-3 )
    {
        ++sweep.overlapping;
        if ( !result.overlapping )
        {
            ++sweep.bad;
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Gjk: " << name << " missed an overlap, seed " << seed << " pair " << index );
        }
    }
    else
    {
        ++sweep.undecided;
    }
}

/*-------------------------------------------------------------------------*/

/** Oriented box pairs: GJK against the brute-force polytope distance, and the overlap against CTSatBoxes and the SAT facade */
template < typename T >
static bool
BoxDistanceSweep( UInt64 seed, UInt32 count, Float64 relativeError )
{
    CRandom random( seed );
    SPolytopeSweep sweep;
    UInt32 satDisagreements = 0;
    for ( UInt32 i=0; i<count; ++i )
    {
        CTOrientedBox3D< T > a;
        CTOrientedBox3D< T > b;
        RandomShape( random, a );
        RandomShape( random, b );
        const SGjkResult< T > result = GjkOf< T >( a, b );
        CheckPolytopePair( "box-box", result, BoxPolytope( Widen( a ) ), BoxPolytope( Widen( b ) ), relativeError, seed, i, sweep );
        SSatBoxesResult< T > sat;
        const bool satOverlap = CTSatBoxes< T >::Test( a, b, CTGeometryTolerance< T >(), sat );
        if ( satOverlap != result.overlapping && fabs( (Float64) sat.depth ) > 1.0e-5 )
        {
            ++satDisagreements;
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Gjk: GJK and SAT disagree on box overlap, seed " << seed << " pair " << i );
        }
    }
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Gjk: box sweep seed " << seed << ": " << sweep.separated << " separated, " << sweep.overlapping << " overlapping, " << sweep.undecided
               << " undecided, worst error " << sweep.worst << ", at most " << sweep.maxIterations << " iterations, " << sweep.bad << " bad, " << satDisagreements << " SAT disagreements" );
    return 0 == sweep.bad && 0 == satDisagreements && sweep.separated > count / 10 && sweep.overlapping > count / 10 && sweep.maxIterations <= (UInt32) CTGjk< T >::MaxIterations;
}

/*-------------------------------------------------------------------------*/

/** Random hull pairs (with and without faces) against the brute-force polytope distance and overlap */
template < typename T >
static bool
HullDistanceSweep( UInt64 seed, UInt32 count, Float64 relativeError )
{
    CRandom random( seed );
    SPolytopeSweep sweep;
    for ( UInt32 i=0; i<count; ++i )
    {
        CTConvexHull3D< T > a;
        CTConvexHull3D< T > b;
        SPolytope pa;
        SPolytope pb;
        RandomHull( random, 0 == i % 2, a, pa );
        RandomHull( random, 0 == i % 3, b, pb );
        const SGjkResult< T > result = GjkOf< T >( a, b );
        CheckPolytopePair( "hull-hull", result, pa, pb, relativeError, seed, i, sweep );
    }
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Gjk: hull sweep seed " << seed << ": " << sweep.separated << " separated, " << sweep.overlapping << " overlapping, " << sweep.undecided
               << " undecided, worst error " << sweep.worst << ", at most " << sweep.maxIterations << " iterations, " << sweep.bad << " bad" );
    return 0 == sweep.bad && sweep.separated > count / 10 && sweep.overlapping > count / 20 && sweep.maxIterations <= (UInt32) CTGjk< T >::MaxIterations;
}

/*-------------------------------------------------------------------------*/

/** Cross-check: the SAT box-box overlap tests of the facade equal the GJK result, including touching pairs */
template < typename T >
static bool
SatGjkCrossCheck( UInt64 seed, UInt32 count )
{
    CRandom random( seed );
    UInt32 disagreements = 0;
    UInt32 overlaps = 0;
    for ( UInt32 i=0; i<count; ++i )
    {
        CTOrientedBox3D< T > a;
        CTOrientedBox3D< T > b;
        CTAxisAlignedBox3D< T > c;
        CTAxisAlignedBox3D< T > d;
        RandomShape( random, a );
        RandomShape( random, b );
        RandomShape( random, c );
        RandomShape( random, d );
        if ( 0 == i % 10 )
        {
            /* a touching pair: d placed so that its min x face lies exactly on the max x face of c */
            const T width = d.max.x - d.min.x;
            d = CTAxisAlignedBox3D< T >( CTPoint3D< T >( c.max.x, d.min.y, d.min.z ), CTPoint3D< T >( c.max.x + width, d.max.y, d.max.z ) );
        }
        const struct { bool sat; bool gjk; Float64 margin; } checks[ 3 ] =
        {
            { Intersects( a, b ), GjkIntersects( a, b ), (Float64) GjkOf< T >( a, b ).distance },
            { Intersects( c, a ), GjkIntersects( c, a ), (Float64) GjkOf< T >( c, a ).distance },
            { Intersects( c, d ), GjkIntersects( c, d ), (Float64) GjkOf< T >( c, d ).distance }
        };
        for ( UInt32 k=0; k<3; ++k )
        {
            overlaps += checks[ k ].gjk ? 1 : 0;
            /* a pair closer than 1e-5 without touching may be called either way within the tolerances */
            if ( checks[ k ].sat != checks[ k ].gjk && !( checks[ k ].margin > 0.0 && checks[ k ].margin < 1.0e-5 ) )
            {
                ++disagreements;
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Gjk: SAT and GJK overlap differ for check " << k << " (SAT " << checks[ k ].sat << ", GJK distance " << checks[ k ].margin << "), seed " << seed << " pair " << i );
            }
        }
    }
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Gjk: SAT-GJK cross-check seed " << seed << ": " << 3 * count << " box pairs, " << overlaps << " overlapping, " << disagreements << " disagreements" );
    return 0 == disagreements && overlaps > count / 5;
}

/*-------------------------------------------------------------------------*/

/** The G/E facade cells against alternating projections between the analytic ClosestPoint members (Float64) */
template < typename TA, typename TB >
static bool
FacadeCellSweep( const char* name, UInt64 seed, UInt32 count )
{
    CRandom random( seed );
    UInt32 bad = 0;
    UInt32 separated = 0;
    UInt32 overlapping = 0;
    for ( UInt32 i=0; i<count; ++i )
    {
        TA a;
        TB b;
        RandomShape( random, a );
        RandomShape( random, b );
        P3 onA;
        P3 onB;
        P3 refA;
        P3 refB;
        const bool found = ClosestPoints( a, b, onA, onB );
        P3 swappedB;
        P3 swappedA;
        ClosestPoints( b, a, swappedB, swappedA );
        const bool hit = Intersects( a, b );
        const bool hitSwapped = Intersects( b, a );
        const Float64 reference = ConvergedDistance( a, b, refA, refB );
        const Float64 distance = ( onB - onA ).GetLength();
        bool ok = found && hit == hitSwapped && fabs( ( swappedA - swappedB ).GetLength() - distance ) < 1.0e-9;
        if ( reference > 1.0e-3 )
        {
            /*
             *  the points lie on the shapes, so their distance bounds the true distance from above; the alternating
             *  projections give an upper bound too, which converges slowly at grazing angles: GJK must not exceed it
             */
            ++separated;
            ok = ok && !hit && distance < reference + 1.0e-4 && distance > reference - 1.0e-3 && a.GetDistance( onA ) < 1.0e-6 && b.GetDistance( onB ) < 1.0e-6;
        }
        else if ( reference < 1.0e-9 )
        {
            ++overlapping;
            ok = ok && hit && distance < 1.0e-6;
        }
        if ( !ok )
        {
            ++bad;
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Gjk: facade cell " << name << " distance " << distance << " reference " << reference << ", seed " << seed << " pair " << i );
        }
    }
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Gjk: facade cell " << name << " seed " << seed << ": " << separated << " separated, " << overlapping << " overlapping, " << bad << " bad" );
    return 0 == bad && separated > count / 10;
}

/*-------------------------------------------------------------------------*/

/** A support mapping that is not a fixed convex set: every call returns a new point a little closer to the origin, so GJK keeps making progress and must stop at its cap */
class CApproachingSupport
{
    public:
    typedef Float64 TValue;
    CApproachingSupport( void ) : m_calls( 0 ) {}
    CTPoint3D< Float64 > GetSupport( const CTVector3D< Float64 >& /*direction*/ ) const
    {
        ++m_calls;
        return CTPoint3D< Float64 >( 1.0 + 1.0 / (Float64) m_calls, 0.0, 0.0 );
    }
    Float64 GetMargin( void ) const { return 0.0; }
    CTPoint3D< Float64 > GetAnyPoint( void ) const { return CTPoint3D< Float64 >( 3.0, 0.0, 0.0 ); }
    private:
    mutable UInt32 m_calls;
};

/*-------------------------------------------------------------------------*/

/** The degenerate configurations of acceptance item 3 for one kind, moved by 'offset' (far from the origin) */
template < typename T >
static bool
DegenerateCases( const CTVector3D< T >& offset, Float64 tolerance, const T& gap )
{
    typedef CTPoint3D< T > P;
    bool ok = true;
    const T zero = T( 0 );
    const T one = T( 1 );
    const T two = T( 2 );
    const CTAxisAlignedBox3D< T > unit( P( zero, zero, zero ) + offset, P( one, one, one ) + offset );
    const CTAxisAlignedBox3D< T > touching( P( one, zero, zero ) + offset, P( two, one, one ) + offset );
    const CTAxisAlignedBox3D< T > beside( P( one + one / two, zero, zero ) + offset, P( two + one, one, one ) + offset );

    /* touching boxes: distance 0, overlapping */
    SGjkResult< T > r = GjkOf< T >( unit, touching );
    ok = ok && r.overlapping && CTNumericTraits< T >::ToFloat64( r.distance ) <= tolerance;
    /* coplanar faces half a unit apart */
    r = GjkOf< T >( unit, beside );
    ok = ok && !r.overlapping && fabs( CTNumericTraits< T >::ToFloat64( r.distance ) - 0.5 ) <= tolerance && fabs( CTNumericTraits< T >::ToFloat64( r.pointOnB.x - r.pointOnA.x ) - 0.5 ) <= tolerance;
    /* identical shapes */
    r = GjkOf< T >( unit, unit );
    ok = ok && r.overlapping && CTNumericTraits< T >::ToFloat64( r.distance ) == 0.0;
    const CTCylinder< T > cylinder( P( zero, zero, zero ) + offset, P( zero, two, zero ) + offset, one );
    r = GjkOf< T >( cylinder, cylinder );
    ok = ok && r.overlapping && CTNumericTraits< T >::ToFloat64( r.distance ) == 0.0;
    /* touching spheres */
    const CTSphere< T > s1( P( zero, zero, zero ) + offset, one );
    const CTSphere< T > s2( P( two, zero, zero ) + offset, one );
    r = GjkOf< T >( s1, s2 );
    ok = ok && r.overlapping && CTNumericTraits< T >::ToFloat64( r.distance ) <= tolerance;
    /* a point on a box face, and just off it */
    r = GjkOf< T >( unit, P( one / two, one / two, one ) + offset );
    ok = ok && r.overlapping && CTNumericTraits< T >::ToFloat64( r.distance ) <= tolerance;
    r = GjkOf< T >( unit, P( one / two, one / two, one + gap ) + offset );
    ok = ok && !r.overlapping && fabs( CTNumericTraits< T >::ToFloat64( r.distance ) - CTNumericTraits< T >::ToFloat64( gap ) ) <= tolerance;
    if ( !ok )
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Gjk: degenerate cases failed at offset " << offset.ToString() << " for a kind of " << (UInt32) sizeof( T ) << " bytes" );
    return ok;
}

/*-------------------------------------------------------------------------*/

/**
 *  Directions within 1e-9 radians of the axis of random cylinders and cones: the supports of the adaptors
 *  (CTSupportMapping) lie on the surface. The supports of the shapes' own GetSupport members are measured as well and
 *  logged: they leave the surface for such directions (the perpendicular part of the direction is rounding noise that
 *  is not perpendicular to the axis), which is why GJK uses the adaptors.
 */
template < typename T >
static bool
NearAxisSupportChecks( UInt64 seed )
{
    CRandom random( seed );
    const Float64 tolerance = sizeof( T ) == 4 ? 1.0e-5 : 1.0e-12;
    UInt32 bad = 0;
    UInt32 memberOff = 0;
    Float64 memberWorst = 0.0;
    for ( UInt32 i=0; i<2000; ++i )
    {
        CTCylinder< T > cylinder;
        CTCone< T > cone;
        RandomShape( random, cylinder );
        RandomShape( random, cone );
        const D3 tilt = random.Direction();
        const Float64 sign = random.Chance( 0.5 ) ? 1.0 : -1.0;
        const V3 cylinderAxis = Widen( cylinder.end - cylinder.start ).Normalized() * sign;
        const V3 coneAxis = WidenD( cone.axis ).AsVector() * sign;
        const CTVector3D< T > d1 = ToV< T >( cylinderAxis + tilt.AsVector() * 1.0e-9 );
        const CTVector3D< T > d2 = ToV< T >( coneAxis + tilt.AsVector() * 1.0e-9 );
        const Float64 scale = 1.0 + Widen( cylinder.start ).ToPositionVector().GetChebyshevLength();
        const Float64 adaptorCylinder = cylinder.GetDistance( CTSupportMapping< CTCylinder< T > >::Make( cylinder ).GetSupport( d1 ) );
        const Float64 adaptorCone = cone.GetDistance( CTSupportMapping< CTCone< T > >::Make( cone ).GetSupport( d2 ) );
        if ( adaptorCylinder > tolerance * scale * 10.0 || adaptorCone > tolerance * scale * 10.0 )
        {
            ++bad;
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Gjk: adaptor support off the surface by " << adaptorCylinder << " / " << adaptorCone << ", seed " << seed << " shape " << i );
        }
        const Float64 memberCylinder = cylinder.GetDistance( cylinder.GetSupport( d1 ) );
        const Float64 memberCone = cone.GetDistance( cone.GetSupport( d2 ) );
        const Float64 member = memberCylinder > memberCone ? memberCylinder : memberCone;
        memberWorst = member > memberWorst ? member : memberWorst;
        memberOff += member > tolerance * scale * 10.0 ? 1 : 0;
    }
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Gjk: near-axis supports, seed " << seed << ": adaptors " << bad << " off the surface; the shapes' own GetSupport " << memberOff
               << " of 2000 off the surface, by up to " << memberWorst );
    return 0 == bad;
}

/*-------------------------------------------------------------------------*/

/** The convex hull shape: members against the box it is built from */
template < typename T >
static bool
HullShapeChecks( void )
{
    typedef CTPoint3D< T > P;
    typedef CTVector3D< T > V;
    bool ok = true;
    CRandom random( 4401 );
    for ( UInt32 i=0; i<200; ++i )
    {
        CTOrientedBox3D< T > box;
        RandomShape( random, box );
        const CTConvexHull3D< T > hull = CTConvexHull3D< T >::FromOrientedBox( box );
        const CTConvexHull3D< T > vertexOnly( &hull.GetVertices()[ 0 ], hull.GetVertexCount() );
        const Float64 tolerance = sizeof( T ) == 4 ? 2.0e-4 : 1.0e-9;
        ok = ok && fabs( (Float64) hull.GetVolume() - (Float64) box.GetVolume() ) < tolerance * 10.0;
        ok = ok && fabs( (Float64) hull.GetSurfaceArea() - (Float64) box.GetSurfaceArea() ) < tolerance * 10.0;
        const P3 q = random.Point( 3.0 );
        const P point = ToP< T >( q );
        const bool inside = box.Contains( point );
        if ( fabs( (Float64) box.GetDistance( point ) ) > 1.0e-3 || inside )
        {
            ok = ok && hull.Contains( point ) == inside && vertexOnly.Contains( point ) == inside;
            ok = ok && ( hull.ClosestPoint( point ) - box.ClosestPoint( point ) ).GetLength() < tolerance * 100.0;
            ok = ok && fabs( (Float64) vertexOnly.GetDistance( point ) - (Float64) box.GetDistance( point ) ) < tolerance * 100.0;
        }
        const P3 rayOrigin = random.Point( 4.0 );
        const D3 rayDirection = random.Direction();
        const CTRay3D< T > ray( ToP< T >( rayOrigin ), ToD< T >( rayDirection ) );
        SRayCastResult< T > expected;
        SRayCastResult< T > got;
        SRayCastResult< T > gotGjk;
        const bool hitBox = box.RayCast( ray, T( 100 ), expected );
        const bool hitHull = hull.RayCast( ray, T( 100 ), got );
        const bool hitGjk = vertexOnly.RayCast( ray, T( 100 ), gotGjk );
        ok = ok && hitBox == hitHull;
        if ( hitBox && hitHull )
            ok = ok && fabs( (Float64) ( expected.distance - got.distance ) ) < tolerance * 100.0 && ( expected.normal - got.normal ).GetLength() < 1.0e-3;
        if ( hitBox && !expected.StartedInside() && ( expected.distance > T( 1 ) / T( 1000 ) ) )
            ok = ok && hitGjk && fabs( (Float64) ( expected.distance - gotGjk.distance ) ) < tolerance * 1000.0;
        if ( !ok )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Gjk: hull shape checks failed at box " << i );
            return false;
        }
    }

    /* faces: wireframe, mirror transform, rejected faces, empty hull */
    const CTConvexHull3D< T > cube = CTConvexHull3D< T >::FromAxisAlignedBox( CTAxisAlignedBox3D< T >( P( T( 0 ), T( 0 ), T( 0 ) ), P( T( 1 ), T( 2 ), T( 3 ) ) ) );
    GUCEF::vector< CTSegment3D< T > > lines;
    cube.GetWireframeLines( lines, 8 );
    ok = ok && 12 == lines.size() && 6 == cube.GetFaceCount() && fabs( (Float64) cube.GetVolume() - 6.0 ) < 1.0e-6;
    const CTMatrix4D< T > mirror = CTMatrix4D< T >::FromTRS( V(), CTQuaternion< T >(), V( T( -1 ), T( 1 ), T( 1 ) ) );
    const CTConvexHull3D< T > mirrored = cube.Transformed( mirror );
    ok = ok && fabs( (Float64) mirrored.GetVolume() - 6.0 ) < 1.0e-6 && mirrored.Contains( P( T( -1 ) / T( 2 ), T( 1 ), T( 1 ) ) ) && !mirrored.Contains( P( T( 1 ) / T( 2 ), T( 1 ), T( 1 ) ) );
    for ( UInt32 f=0; f<mirrored.GetFaceCount(); ++f )
        ok = ok && mirrored.GetFacePlane( f ).GetSignedDistance( P( T( -1 ) / T( 2 ), T( 1 ), T( 1 ) ) ) < T( 0 );
    CTConvexHull3D< T > rejected = cube;
    const UInt32 badIndices[ 3 ] = { 0, 1, 99 };
    const UInt32 badSizes[ 1 ] = { 3 };
    ok = ok && !rejected.SetFaces( badIndices, badSizes, 1 ) && 6 == rejected.GetFaceCount();
    const CTConvexHull3D< T > empty;
    SRayCastResult< T > hit;
    ok = ok && !empty.Contains( P() ) && empty.GetBounds().IsEmpty() && !empty.RayCast( CTRay3D< T >(), T( 10 ), hit ) && P() == empty.GetSupport( V( T( 1 ), T( 0 ), T( 0 ) ) );
    P onA;
    P onB;
    ok = ok && !ClosestPoints( empty, cube, onA, onB ) && !Intersects( cube, empty );

    /* text and DataNode round trips */
    CTConvexHull3D< T > parsed;
    ok = ok && parsed.FromString( cube.ToString() ) && parsed == cube && !parsed.FromString( "ConvexHull3D(vertices ((0, 0, 0)), faces ((0, 1, 2)))" ) && parsed == cube;
    CORE::CDataNode node;
    CTConvexHull3D< T > restored;
    ok = ok && cube.Serialize( node, CORE::CDataNodeSerializableSettings() ) && restored.Deserialize( node, CORE::CDataNodeSerializableSettings() ) && restored == cube;

    /* the plane and volume cells of the hull */
    T lowest = T( 0 );
    T highest = T( 0 );
    GetSignedDistanceRange( CTPlane< T >( CTDirectionVector3D< T >::UnitY(), T( 1 ) ), cube, lowest, highest );
    ok = ok && T( -1 ) == lowest && T( 1 ) == highest && Intersects( CTPlane< T >( CTDirectionVector3D< T >::UnitY(), T( 1 ) ), cube );
    if ( !ok )
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Gjk: hull face, text or cell checks failed" );
    return ok;
}

/*-------------------------------------------------------------------------*/

/** The Float64 value of a fixed-point vector (exact for components below 2^21) */
static V3
FixedToV3( const CTVector3D< TFixed32_32 >& v )
{
    return V3( FixedToFloat64( v.x ), FixedToFloat64( v.y ), FixedToFloat64( v.z ) );
}

/** A TFixed32_32 oriented box from Float64 values and the same box exactly in Float64 */
static void
MakeFixedBox( const P3& centre, const Q4& rotation, const V3& half, CTOrientedBox3D< TFixed32_32 >& fixed, CTOrientedBox3D< Float64 >& wide )
{
    fixed = CTOrientedBox3D< TFixed32_32 >( ToP< TFixed32_32 >( centre ), ToQ< TFixed32_32 >( rotation ), ToV< TFixed32_32 >( half ) );
    const CTQuaternion< TFixed32_32 >& q = fixed.rotation;
    wide = CTOrientedBox3D< Float64 >( FixedToP3( fixed.centre ), Q4::FromXYZW( FixedToFloat64( q.x ), FixedToFloat64( q.y ), FixedToFloat64( q.z ), FixedToFloat64( q.w ) ), FixedToV3( fixed.halfExtents ) );
}

/** The checks of a separated fixed-point GJK result against the reference distance: the distance, the points on the shapes, pointOnB - pointOnA = normal * distance */
template < typename TWideA, typename TWideB >
static bool
SeparatedResultMatches( const SGjkResult< TFixed32_32 >& result, MATH::TMathStatus status, const TWideA& a, const TWideB& b, Float64 reference, Float64 band )
{
    const Float64 distance = FixedToFloat64( result.distance );
    const P3 onA = FixedToP3( result.pointOnA );
    const P3 onB = FixedToP3( result.pointOnB );
    return 0 == ( status & MATHSTATUS_NOT_CONVERGED ) && !result.overlapping && fabs( distance - reference ) <= band && a.GetDistance( onA ) <= band && b.GetDistance( onB ) <= band &&
           ( ( onB - onA ) - FixedToV3( result.normal ) * distance ).GetLength() <= band;
}

/*-------------------------------------------------------------------------*/

/**
 *  TFixed32_32 GJK and the GJK ray cast at sizes 10 to 1000 and offsets 10^3 to 10^5 from the origin (the search
 *  directions are scaled, so the support dot products stay in range): box pairs against the brute-force polytope
 *  distance; hulls and triangles 10^4 from a box at offset 10^5; box casts against Float64
 */
static bool
FixedScaleGjkSweep( UInt64 seed )
{
    typedef TFixed32_32 TFixed;
    typedef CTOrientedBox3D< TFixed > TBox;
    CRandom random( seed );
    const Float64 sizes[ 3 ] = { 10.0, 100.0, 1000.0 };
    const Float64 offsets[ 3 ] = { 1.0e3, 1.0e4, 1.0e5 };
    UInt32 bad = 0;
    UInt32 separated = 0;
    UInt32 overlapping = 0;
    UInt32 casts = 0;
    for ( UInt32 s=0; s<3; ++s )
    {
        for ( UInt32 o=0; o<3; ++o )
        {
            const Float64 size = sizes[ s ];
            const Float64 band = 1.0e-6 * size;
            const P3 base( offsets[ o ], -offsets[ o ], 0.5 * offsets[ o ] );
            for ( UInt32 i=0; i<30; ++i )
            {
                /* box pairs, separated or overlapping */
                const V3 offsetA = random.Vector( size );
                const V3 offsetB = random.Vector( 3.0 * size );
                const Q4 rotationA = random.Rotation();
                const Q4 rotationB = random.Rotation();
                const Float64 ax = random.Uniform( 0.2, 1.0 ) * size;
                const Float64 ay = random.Uniform( 0.2, 1.0 ) * size;
                const Float64 az = random.Uniform( 0.2, 1.0 ) * size;
                const Float64 bx = random.Uniform( 0.2, 1.0 ) * size;
                const Float64 by = random.Uniform( 0.2, 1.0 ) * size;
                const Float64 bz = random.Uniform( 0.2, 1.0 ) * size;
                TBox fa;
                TBox fb;
                CTOrientedBox3D< Float64 > wa;
                CTOrientedBox3D< Float64 > wb;
                MakeFixedBox( base + offsetA, rotationA, V3( ax, ay, az ), fa, wa );
                MakeFixedBox( base + offsetB, rotationB, V3( bx, by, bz ), fb, wb );
                SGjkResult< TFixed > result;
                const MATH::TMathStatus status = CTGjk< TFixed >::ComputeDistance( CTSupportMapping< TBox >::Make( fa ), CTSupportMapping< TBox >::Make( fb ), CTGeometryTolerance< TFixed >(), GUCEF_NULL, result );
                const SPolytope pa = BoxPolytope( wa );
                const SPolytope pb = BoxPolytope( wb );
                const Float64 separation = PolytopeSeparation( pa, pb );
                bool ok = true;
                if ( separation > band )
                {
                    ++separated;
                    ok = SeparatedResultMatches( result, status, wa, wb, PolytopeDistance( pa, pb ), band );
                }
                else if ( separation < -band )
                {
                    ++overlapping;
                    ok = 0 == ( status & MATHSTATUS_NOT_CONVERGED ) && result.overlapping;
                }
                if ( !ok )
                {
                    ++bad;
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Gjk: fixed-point box pair " << i << " size " << size << " offset " << offsets[ o ] << ": distance " << FixedToFloat64( result.distance )
                                     << ", status " << status << ", seed " << seed );
                }

                /* a hull of 8 points and a triangle 10^4 away from a box, at offset 10^5 */
                if ( 2 == o )
                {
                    const V3 away = random.Vector( 1.0 );
                    const Float64 awayLength = away.GetLength();
                    const V3 direction = awayLength > 0.1 ? away * ( 1.0 / awayLength ) : V3( 1.0, 0.0, 0.0 );
                    CTPoint3D< TFixed > hullPoints[ 8 ];
                    GUCEF::vector< P3 > wideHull;
                    for ( UInt32 k=0; k<8; ++k )
                    {
                        const V3 corner = random.Vector( size );
                        hullPoints[ k ] = ToP< TFixed >( base + offsetA + direction * 1.0e4 + corner );
                        wideHull.push_back( FixedToP3( hullPoints[ k ] ) );
                    }
                    const CTConvexHull3D< TFixed > hull( hullPoints, 8 );
                    SGjkResult< TFixed > hullResult;
                    const MATH::TMathStatus hullStatus = CTGjk< TFixed >::ComputeDistance( CTSupportMapping< CTConvexHull3D< TFixed > >::Make( hull ), CTSupportMapping< TBox >::Make( fa ),
                                                                                         CTGeometryTolerance< TFixed >(), GUCEF_NULL, hullResult );
                    const SPolytope ph = BruteForceHull( wideHull );
                    const CTConvexHull3D< Float64 > wideHullShape( &wideHull[ 0 ], 8 );
                    ok = SeparatedResultMatches( hullResult, hullStatus, wideHullShape, wa, PolytopeDistance( ph, pa ), band );
                    const CTTriangle3D< TFixed > triangle( hullPoints[ 0 ], hullPoints[ 1 ], hullPoints[ 2 ] );
                    const CTTriangle3D< Float64 > wideTriangle( wideHull[ 0 ], wideHull[ 1 ], wideHull[ 2 ] );
                    SGjkResult< TFixed > triangleResult;
                    const MATH::TMathStatus triangleStatus = CTGjk< TFixed >::ComputeDistance( CTSupportMapping< CTTriangle3D< TFixed > >::Make( triangle ), CTSupportMapping< TBox >::Make( fa ),
                                                                                             CTGeometryTolerance< TFixed >(), GUCEF_NULL, triangleResult );
                    SGjkResult< Float64 > wideTriangleResult;
                    CTGjk< Float64 >::ComputeDistance( CTSupportMapping< CTTriangle3D< Float64 > >::Make( wideTriangle ), CTSupportMapping< CTOrientedBox3D< Float64 > >::Make( wa ),
                                                       CTGeometryTolerance< Float64 >(), GUCEF_NULL, wideTriangleResult );
                    ok = ok && SeparatedResultMatches( triangleResult, triangleStatus, wideTriangle, wa, wideTriangleResult.distance, band );
                    if ( !ok )
                    {
                        ++bad;
                        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Gjk: fixed-point hull or triangle 10^4 from a box, case " << i << " size " << size << ": hull distance "
                                         << FixedToFloat64( hullResult.distance ) << ", triangle distance " << FixedToFloat64( triangleResult.distance ) << ", seed " << seed );
                    }
                }

                /* box A cast towards box B (through its centre) against the Float64 cast of the same boxes */
                const V3 towards = wb.centre - wa.centre;
                const Float64 length = towards.GetLength();
                if ( length > size )
                {
                    const D3 direction( towards );
                    const CTDirectionVector3D< TFixed > fixedDirection( ToV< TFixed >( direction.AsVector() ) );
                    const D3 wideDirection( FixedToV3( fixedDirection.AsVector() ) );
                    const Float64 maxDistance = length + 4.0 * size;
                    SRayCastResult< TFixed > hit;
                    SRayCastResult< Float64 > wideHit;
                    const TFixed fixedMaxDistance( maxDistance );
                    const bool got = ShapeCast( fa, fixedDirection, fixedMaxDistance, fb, hit );
                    const bool expected = ShapeCast( wa, wideDirection, FixedToFloat64( fixedMaxDistance ), wb, wideHit );
                    ++casts;
                    /* the cast stops slightly early (by at most its tolerance), never late */
                    ok = got == expected;
                    if ( ok && got && !wideHit.StartedInside() )
                    {
                        const Float64 early = wideHit.distance - FixedToFloat64( hit.distance );
                        ok = early >= -band && early <= 1.0e-4 * size && wb.GetDistance( FixedToP3( hit.point ) ) <= 1.0e-4 * size;
                    }
                    if ( !ok )
                    {
                        ++bad;
                        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Gjk: fixed-point box cast " << i << " size " << size << " offset " << offsets[ o ] << ": hit " << got << " at "
                                         << FixedToFloat64( hit.distance ) << ", Float64 " << expected << " at " << wideHit.distance << ", seed " << seed );
                    }
                }
            }
        }
    }
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Gjk: TFixed32_32 at sizes 10 to 1000 and offsets 10^3 to 10^5, seed " << seed << ": " << separated << " separated, " << overlapping << " overlapping box pairs, "
               << casts << " casts, " << bad << " bad" );
    return 0 == bad && separated > 50 && overlapping > 20;
}

/*-------------------------------------------------------------------------*/

/** The TFixed32_32 run: GJK of fixed-point shapes from integer draws against Float64 GJK of the same values, hashed */
static bool
FixedPointGjkRun( UInt64& hash, UInt32& hashed )
{
    typedef TFixed32_32 TFixed;
    typedef CTPoint3D< TFixed > P;
    typedef CTVector3D< TFixed > V;
    CFixedRandom random( 9001 );
    CRawHash raw;
    UInt32 bad = 0;
    Float64 worst = 0.0;
    for ( UInt32 i=0; i<2000; ++i )
    {
        const UInt32 kind = i % 4;
        const P ca = random.Point( 2 * 65536 );
        const P cb = random.Point( 2 * 65536 );
        const TFixed ra = random.Uniform( 13107, 65536 );
        const TFixed rb = random.Uniform( 13107, 65536 );
        const TFixed haX = random.Uniform( 13107, 65536 );
        const TFixed haY = random.Uniform( 13107, 65536 );
        const TFixed haZ = random.Uniform( 13107, 65536 );
        const V ha( haX, haY, haZ );
        const TFixed hbX = random.Uniform( 13107, 65536 );
        const TFixed hbY = random.Uniform( 13107, 65536 );
        const TFixed hbZ = random.Uniform( 13107, 65536 );
        const V hb( hbX, hbY, hbZ );
        const CTQuaternion< TFixed > qa = random.Rotation();
        const CTQuaternion< TFixed > qb = random.Rotation();
        const V axis = random.Vector( 65536 );
        SGjkResult< TFixed > fixed;
        SGjkResult< Float64 > reference;
        if ( 0 == kind )
        {
            fixed = GjkOf< TFixed >( CTSphere< TFixed >( ca, ra ), CTSphere< TFixed >( cb, rb ) );
            reference = GjkOf< Float64 >( CTSphere< Float64 >( FixedToP3( ca ), FixedToFloat64( ra ) ), CTSphere< Float64 >( FixedToP3( cb ), FixedToFloat64( rb ) ) );
        }
        else if ( 1 == kind )
        {
            const CTOrientedBox3D< TFixed > a( ca, qa, ha );
            const CTOrientedBox3D< TFixed > b( cb, qb, hb );
            fixed = GjkOf< TFixed >( a, b );
            const CTOrientedBox3D< Float64 > wa( FixedToP3( ca ), Q4::FromXYZW( FixedToFloat64( qa.x ), FixedToFloat64( qa.y ), FixedToFloat64( qa.z ), FixedToFloat64( qa.w ) ), V3( FixedToFloat64( ha.x ), FixedToFloat64( ha.y ), FixedToFloat64( ha.z ) ) );
            const CTOrientedBox3D< Float64 > wb( FixedToP3( cb ), Q4::FromXYZW( FixedToFloat64( qb.x ), FixedToFloat64( qb.y ), FixedToFloat64( qb.z ), FixedToFloat64( qb.w ) ), V3( FixedToFloat64( hb.x ), FixedToFloat64( hb.y ), FixedToFloat64( hb.z ) ) );
            reference = GjkOf< Float64 >( wa, wb );
        }
        else if ( 2 == kind )
        {
            fixed = GjkOf< TFixed >( CTCapsule< TFixed >( ca, ca + axis, ra ), CTCylinder< TFixed >( cb, cb + ha, rb ) );
            reference = GjkOf< Float64 >( CTCapsule< Float64 >( FixedToP3( ca ), FixedToP3( ca + axis ), FixedToFloat64( ra ) ), CTCylinder< Float64 >( FixedToP3( cb ), FixedToP3( cb + ha ), FixedToFloat64( rb ) ) );
        }
        else
        {
            P pointsA[ 8 ];
            P pointsB[ 8 ];
            CTPoint3D< Float64 > widenedA[ 8 ];
            CTPoint3D< Float64 > widenedB[ 8 ];
            for ( UInt32 k=0; k<8; ++k )
            {
                pointsA[ k ] = ca + random.Vector( 65536 );
                pointsB[ k ] = cb + random.Vector( 65536 );
                widenedA[ k ] = FixedToP3( pointsA[ k ] );
                widenedB[ k ] = FixedToP3( pointsB[ k ] );
            }
            fixed = GjkOf< TFixed >( CTConvexHull3D< TFixed >( pointsA, 8 ), CTConvexHull3D< TFixed >( pointsB, 8 ) );
            reference = GjkOf< Float64 >( CTConvexHull3D< Float64 >( widenedA, 8 ), CTConvexHull3D< Float64 >( widenedB, 8 ) );
        }
        raw.Add( fixed.distance );
        raw.Add( fixed.overlapping );
        raw.Add3( fixed.pointOnA );
        raw.Add3( fixed.pointOnB );
        raw.Add3( fixed.normal );
        raw.Add( fixed.iterations );
        raw.Add( fixed.status );
        const Float64 error = fabs( FixedToFloat64( fixed.distance ) - reference.distance );
        worst = error > worst ? error : worst;
        /* the shapes are rounded to Q32.32 differently from Float64 only in the rotation matrix: 1e-6 */
        if ( error > 1.0e-6 || ( fixed.overlapping != reference.overlapping && reference.distance > 1.0e-6 ) || fixed.iterations > (UInt32) CTGjk< TFixed >::MaxIterations )
        {
            ++bad;
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Gjk: fixed-point distance " << FixedToFloat64( fixed.distance ) << " Float64 " << reference.distance << " kind " << kind << " pair " << i );
        }
    }
    hash = raw.GetHash();
    hashed = raw.GetCount();
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Gjk: TFixed32_32 run: " << hashed << " raw values, hash " << CORE::ToString( hash ) << ", worst difference to Float64 " << worst << ", " << bad << " bad" );
    return 0 == bad;
}

/*-------------------------------------------------------------------------*/

/** Raw results of the exported functions of this package for one set of inputs (T is Float32 or Float64) */
template < typename T >
struct SConvexResults
{
    T values[ 256 ];
    UInt32 count;

    SConvexResults( void ) : count( 0 ) { memset( values, 0, sizeof( values ) ); }
    void Add( const T& v ) { if ( count < 256 ) values[ count++ ] = v; }
    template < typename V > void Add3( const V& v ) { Add( v.x ); Add( v.y ); Add( v.z ); }
};

/** Calls the precision-sensitive exported functions of this package; a macro, so that each caller compiles the calls in its own context */
#define GUCEF_TEST_CALL_CONVEX_QUERIES( T, a, c, out )                                                                    \
    {                                                                                                                     \
        typedef CTPoint3D< T > P;                                                                                         \
        typedef CTVector3D< T > V;                                                                                        \
        typedef CTDirectionVector3D< T > D;                                                                               \
        const T zero = T( 0 );                                                                                            \
        const T one = T( 1 );                                                                                             \
        out.count = 0;                                                                                                    \
        const CTOrientedBox3D< T > box1( P( zero, zero, zero ), CTQuaternion< T >::FromAxisAngle( D( V( a, c, one ) ), CTRadian< T >( c ) ), V( a, c, a ) ); \
        const CTOrientedBox3D< T > box2( P( c, a, zero ), CTQuaternion< T >::FromAxisAngle( D( V( c, one, a ) ), CTRadian< T >( a ) ), V( c, a, c ) ); \
        const CTCylinder< T > cylinder( P( a, c, zero ), P( c + c, a, c ), c );                                           \
        const CTSphere< T > sphere( P( a, a, c ), c );                                                                    \
        const CTTriangle3D< T > triangle( P( zero, zero, zero ), P( a, zero, c ), P( zero, c, a ) );                      \
        P p1;                                                                                                             \
        P p2;                                                                                                             \
        ClosestPoints( box1, box2, p1, p2 ); out.Add3( p1 ); out.Add3( p2 );                                              \
        ClosestPoints( box1, cylinder, p1, p2 ); out.Add3( p1 ); out.Add3( p2 );                                          \
        ClosestPoints( sphere, cylinder, p1, p2 ); out.Add3( p1 ); out.Add3( p2 );                                        \
        CTContactManifold< T > manifold;                                                                                  \
        ComputeContact( box1, box2, manifold ); out.Add3( manifold.normal );                                              \
        for ( UInt32 i=0; i<manifold.GetPointCount(); ++i ) { out.Add3( manifold.GetPoint( i ).pointOnA ); out.Add( manifold.GetPoint( i ).depth ); } \
        ComputeContact( sphere, cylinder, manifold ); out.Add3( manifold.normal ); out.Add( manifold.GetPoint( 0 ).depth ); \
        ComputeContact( cylinder, CTCylinder< T >( P( a, a, a ), P( c, c + a, zero ), a ), manifold ); out.Add3( manifold.normal ); out.Add( manifold.GetPoint( 0 ).depth ); \
        SRayCastResult< T > hit;                                                                                          \
        const CTRay3D< T > ray( P( -c - c, a, zero ), D( V( one, a / ( c + c ), zero ) ) );                              \
        out.Add( SphereCast( ray, a / ( c + c ), CTAxisAlignedBox3D< T >( P( zero, zero, zero ), P( a, c, a ) ), c + c + c, hit ) ? hit.distance : T( -1 ) ); \
        out.Add( SphereCast( ray, a / ( c + c ), triangle, c + c + c, hit ) ? hit.distance : T( -1 ) );                   \
        out.Add( ShapeCast( box2, D( V( -one, a, zero ) ), c + c, cylinder, hit ) ? hit.distance : T( -1 ) );              \
        SToiResult< T > toi;                                                                                              \
        TimeOfImpact( box1, CTMotion3D< T >( P( -c - c, zero, zero ), CTQuaternion< T >(), V( c + c + c, zero, zero ), V( zero, zero, a ) ), \
                      box2, CTMotion3D< T >::AtRest( P( c, zero, zero ), CTQuaternion< T >() ), a / T( 100 ), toi );      \
        out.Add( toi.time );                                                                                              \
        const P cloud[ 6 ] = { P( a, zero, c ), P( -c, a, zero ), P( zero, c, -a ), P( c, c, a ), P( -a, -c, c ), P( a, a, -c ) }; \
        P scratch[ 6 ];                                                                                                   \
        CTSphere< T > fitted;                                                                                             \
        FitMinimumSphere( cloud, 6, scratch, 7, fitted ); out.Add3( fitted.centre ); out.Add( fitted.radius );            \
        CTOrientedBox3D< T > fittedBox;                                                                                   \
        FitOrientedBox( cloud, 6, fittedBox ); out.Add3( fittedBox.halfExtents );                                         \
    }

#ifdef GUCEF_TEST_GJK_CONTRACTION_SUPPORTED

static void CallConvexQueriesPlain32( Float32 a, Float32 c, SConvexResults< Float32 >& out ) GUCEF_TEST_CALL_CONVEX_QUERIES( Float32, a, c, out )
static void CallConvexQueriesPlain64( Float64 a, Float64 c, SConvexResults< Float64 >& out ) GUCEF_TEST_CALL_CONVEX_QUERIES( Float64, a, c, out )

template < typename T >
GUCEF_TEST_GJK_CONTRACTING static T
ConvexContractedMultiplyAdd( T a, T b, T c )
{
    GUCEF_TEST_GJK_CONTRACT_HERE
    return a * b + c;
}

GUCEF_TEST_GJK_CONTRACTING static void
CallConvexQueriesContracted32( Float32 a, Float32 c, SConvexResults< Float32 >& out )
{
    GUCEF_TEST_GJK_CONTRACT_HERE
    GUCEF_TEST_CALL_CONVEX_QUERIES( Float32, a, c, out )
}

GUCEF_TEST_GJK_CONTRACTING static void
CallConvexQueriesContracted64( Float64 a, Float64 c, SConvexResults< Float64 >& out )
{
    GUCEF_TEST_GJK_CONTRACT_HERE
    GUCEF_TEST_CALL_CONVEX_QUERIES( Float64, a, c, out )
}

static bool
ConvexCpuHasFma( void )
{
    __builtin_cpu_init();
    return 0 != __builtin_cpu_supports( "fma" );
}

#endif /* GUCEF_TEST_GJK_CONTRACTION_SUPPORTED ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformGjkTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "Gjk" );

    GUCEF_TESTFW_TESTCASE( "Gjk: sphere distances equal |c1 - c2| - r1 - r2 within 1e-5 relative, 10000 seeded pairs in Float32 and Float64" )
    try
    {
        ASSERT_TRUE( SphereDistanceSweep< Float64 >( 501, ScaledCount( 10000 ), 1.0e-5 ) );
        ASSERT_TRUE( SphereDistanceSweep< Float32 >( 502, ScaledCount( 10000 ), 1.0e-5 ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Gjk: box and hull distances equal the brute-force polytope distance within 1e-5 (Float64; 1e-4 Float32), overlap equals SAT, 10000 seeded pairs" )
    try
    {
        ASSERT_TRUE( BoxDistanceSweep< Float64 >( 511, ScaledCount( 10000 ), 1.0e-5 ) );
        ASSERT_TRUE( BoxDistanceSweep< Float32 >( 512, ScaledCount( 10000 ), 1.0e-4 ) );
        ASSERT_TRUE( HullDistanceSweep< Float64 >( 513, ScaledCount( 10000 ), 1.0e-5 ) );
        ASSERT_TRUE( HullDistanceSweep< Float32 >( 514, ScaledCount( 4000 ), 1.0e-4 ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Gjk: the SAT box-box overlap tests of the facade (OBB-OBB, AABB-OBB, AABB-AABB) equal GJK, 10000 seeded triples including touching pairs" )
    try
    {
        ASSERT_TRUE( SatGjkCrossCheck< Float64 >( 521, ScaledCount( 10000 ) ) );
        ASSERT_TRUE( SatGjkCrossCheck< Float32 >( 522, ScaledCount( 10000 ) ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Gjk: the G/E cells of the facade agree with alternating projections, 1000 seeded pairs per cell (Float64)" )
    try
    {
        ASSERT_TRUE( ( FacadeCellSweep< CTSegment3D< Float64 >, CTCylinder< Float64 > >( "segment-cylinder", 531, ScaledCount( 1000 ) ) ) );
        ASSERT_TRUE( ( FacadeCellSweep< CTSegment3D< Float64 >, CTCone< Float64 > >( "segment-cone", 532, ScaledCount( 1000 ) ) ) );
        ASSERT_TRUE( ( FacadeCellSweep< CTSegment3D< Float64 >, CTAxisAlignedBox3D< Float64 > >( "segment-AABB", 533, ScaledCount( 1000 ) ) ) );
        ASSERT_TRUE( ( FacadeCellSweep< CTSegment3D< Float64 >, CTOrientedBox3D< Float64 > >( "segment-OBB", 534, ScaledCount( 1000 ) ) ) );
        ASSERT_TRUE( ( FacadeCellSweep< CTTriangle3D< Float64 >, CTCylinder< Float64 > >( "triangle-cylinder", 535, ScaledCount( 1000 ) ) ) );
        ASSERT_TRUE( ( FacadeCellSweep< CTTriangle3D< Float64 >, CTCone< Float64 > >( "triangle-cone", 536, ScaledCount( 1000 ) ) ) );
        ASSERT_TRUE( ( FacadeCellSweep< CTTriangle3D< Float64 >, CTAxisAlignedBox3D< Float64 > >( "triangle-AABB", 537, ScaledCount( 1000 ) ) ) );
        ASSERT_TRUE( ( FacadeCellSweep< CTTriangle3D< Float64 >, CTOrientedBox3D< Float64 > >( "triangle-OBB", 538, ScaledCount( 1000 ) ) ) );
        ASSERT_TRUE( ( FacadeCellSweep< CTSphere< Float64 >, CTCylinder< Float64 > >( "sphere-cylinder", 539, ScaledCount( 1000 ) ) ) );
        ASSERT_TRUE( ( FacadeCellSweep< CTSphere< Float64 >, CTCone< Float64 > >( "sphere-cone", 540, ScaledCount( 1000 ) ) ) );
        ASSERT_TRUE( ( FacadeCellSweep< CTCapsule< Float64 >, CTAxisAlignedBox3D< Float64 > >( "capsule-AABB", 541, ScaledCount( 1000 ) ) ) );
        ASSERT_TRUE( ( FacadeCellSweep< CTCapsule< Float64 >, CTOrientedBox3D< Float64 > >( "capsule-OBB", 542, ScaledCount( 1000 ) ) ) );
        ASSERT_TRUE( ( FacadeCellSweep< CTCapsule< Float64 >, CTCylinder< Float64 > >( "capsule-cylinder", 543, ScaledCount( 1000 ) ) ) );
        ASSERT_TRUE( ( FacadeCellSweep< CTCapsule< Float64 >, CTCone< Float64 > >( "capsule-cone", 544, ScaledCount( 1000 ) ) ) );
        ASSERT_TRUE( ( FacadeCellSweep< CTAxisAlignedBox3D< Float64 >, CTOrientedBox3D< Float64 > >( "AABB-OBB", 545, ScaledCount( 1000 ) ) ) );
        ASSERT_TRUE( ( FacadeCellSweep< CTAxisAlignedBox3D< Float64 >, CTCylinder< Float64 > >( "AABB-cylinder", 546, ScaledCount( 1000 ) ) ) );
        ASSERT_TRUE( ( FacadeCellSweep< CTAxisAlignedBox3D< Float64 >, CTCone< Float64 > >( "AABB-cone", 547, ScaledCount( 1000 ) ) ) );
        ASSERT_TRUE( ( FacadeCellSweep< CTOrientedBox3D< Float64 >, CTOrientedBox3D< Float64 > >( "OBB-OBB", 548, ScaledCount( 1000 ) ) ) );
        ASSERT_TRUE( ( FacadeCellSweep< CTOrientedBox3D< Float64 >, CTCylinder< Float64 > >( "OBB-cylinder", 549, ScaledCount( 1000 ) ) ) );
        ASSERT_TRUE( ( FacadeCellSweep< CTOrientedBox3D< Float64 >, CTCone< Float64 > >( "OBB-cone", 550, ScaledCount( 1000 ) ) ) );
        ASSERT_TRUE( ( FacadeCellSweep< CTCylinder< Float64 >, CTCylinder< Float64 > >( "cylinder-cylinder", 551, ScaledCount( 1000 ) ) ) );
        ASSERT_TRUE( ( FacadeCellSweep< CTCylinder< Float64 >, CTCone< Float64 > >( "cylinder-cone", 552, ScaledCount( 1000 ) ) ) );
        ASSERT_TRUE( ( FacadeCellSweep< CTCone< Float64 >, CTCone< Float64 > >( "cone-cone", 553, ScaledCount( 1000 ) ) ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Gjk: degenerate cases: touching, identical shapes, coplanar faces, a point on a box face, and the same 10^4 units from the origin" )
    try
    {
        /* the tolerance of the queries grows with the coordinates (relative 1e-10 for Float64, 1e-5 for Float32, absolute only for fixed point) */
        ASSERT_TRUE( DegenerateCases< Float64 >( CTVector3D< Float64 >(), 1.0e-9, 1.0e-3 ) );
        ASSERT_TRUE( DegenerateCases< Float64 >( CTVector3D< Float64 >( 1.0e4, -1.0e4, 1.0e4 ), 2.0e-6, 1.0e-3 ) );
        ASSERT_TRUE( DegenerateCases< Float32 >( CTVector3D< Float32 >(), 1.0e-5, 1.0e-3f ) );
        ASSERT_TRUE( DegenerateCases< Float32 >( CTVector3D< Float32 >( 1.0e4f, -1.0e4f, 1.0e4f ), 0.2, 0.5f ) );
        ASSERT_TRUE( DegenerateCases< TFixed32_32 >( CTVector3D< TFixed32_32 >(), 1.0e-8, TFixed32_32( 1 ) / TFixed32_32( 1000 ) ) );
        ASSERT_TRUE( DegenerateCases< TFixed32_32 >( CTVector3D< TFixed32_32 >( TFixed32_32( 10000 ), TFixed32_32( -10000 ), TFixed32_32( 10000 ) ), 1.0e-8, TFixed32_32( 1 ) / TFixed32_32( 1000 ) ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Gjk: the iteration cap is reported as NOT_CONVERGED and never exceeded; the warm start cache" )
    try
    {
        CApproachingSupport approaching;
        SGjkResult< Float64 > result;
        const MATH::TMathStatus status = CTGjk< Float64 >::ComputeDistance( approaching, CTPointSupport< Float64 >( CTPoint3D< Float64 >() ), CTGeometryTolerance< Float64 >(), GUCEF_NULL, result );
        ASSERT_TRUE( 0 != ( status & MATHSTATUS_NOT_CONVERGED ) && 0 != ( result.status & MATHSTATUS_NOT_CONVERGED ) );
        ASSERT_TRUE( (UInt32) CTGjk< Float64 >::MaxIterations == result.iterations );
        ASSERT_TRUE( result.distance > 1.0 && result.distance < 1.1 );
        /* at the cap the distance is an upper bound; the lower bound stays below it */
        ASSERT_TRUE( result.distanceLowerBound <= result.distance && result.distanceLowerBound > 0.9 );

        /* the warm start: the same pair again from the cache needs no more iterations and gives the same distance */
        CRandom random( 561 );
        UInt32 cold = 0;
        UInt32 warm = 0;
        bool same = true;
        for ( UInt32 i=0; i<1000; ++i )
        {
            CTCylinder< Float64 > a;
            CTCone< Float64 > b;
            RandomShape( random, a );
            RandomShape( random, b );
            SGjkCache< Float64 > cache;
            SGjkResult< Float64 > first;
            SGjkResult< Float64 > second;
            CTGjk< Float64 >::ComputeDistance( CTShapeSupport< CTCylinder< Float64 > >( a ), CTShapeSupport< CTCone< Float64 > >( b ), CTGeometryTolerance< Float64 >(), &cache, first );
            CTGjk< Float64 >::ComputeDistance( CTShapeSupport< CTCylinder< Float64 > >( a ), CTShapeSupport< CTCone< Float64 > >( b ), CTGeometryTolerance< Float64 >(), &cache, second );
            cold += first.iterations;
            warm += second.iterations;
            same = same && fabs( first.distance - second.distance ) < 1.0e-9 && first.overlapping == second.overlapping;
        }
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Gjk: warm start: " << cold << " iterations cold, " << warm << " warm over 1000 pairs" );
        ASSERT_TRUE( same && warm < cold );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Gjk: the cylinder and cone support adaptors stay on the surface for directions nearly parallel to the axis" )
    try
    {
        ASSERT_TRUE( NearAxisSupportChecks< Float64 >( 571 ) );
        ASSERT_TRUE( NearAxisSupportChecks< Float32 >( 572 ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Gjk: the convex hull shape: containment, closest points and ray casts against the box it is built from, faces, mirror transform, text and DataNode" )
    try
    {
        ASSERT_TRUE( HullShapeChecks< Float64 >() );
        ASSERT_TRUE( HullShapeChecks< Float32 >() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Gjk: TFixed32_32 GJK (spheres, boxes, capsule-cylinder, hulls) agrees with Float64 within 1e-6 and matches the pinned hash" )
    try
    {
        UInt64 hash = 0;
        UInt32 hashed = 0;
        ASSERT_TRUE( FixedPointGjkRun( hash, hashed ) );
        ASSERT_TRUE( PinnedFixedGjkHash == hash );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Gjk: TFixed32_32 GJK and casts at sizes 10 to 1000 and offsets 10^3 to 10^5 (boxes, hulls and triangles 10^4 apart at 10^5) match the Float64 references" )
    try
    {
        ASSERT_TRUE( FixedScaleGjkSweep( 571 ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Gjk: exported GJK, EPA, SAT, cast, time of impact and fitting functions give the same bits from contracting FMA code" )
    #ifndef GUCEF_TEST_GJK_CONTRACTION_SUPPORTED
    GUCEF_TESTFW_SKIP( "contraction test needs GCC or Clang on x86" );
    #else
    GUCEF_TESTFW_SKIP_IF( !ConvexCpuHasFma(), "the CPU has no FMA" );
    volatile Float32 a32 = 1.0f + ldexpf( 1.0f, -12 );
    volatile Float32 c32 = 1.0f + ldexpf( 1.0f, -11 );
    volatile Float64 a64 = 1.0 + ldexp( 1.0, -27 );
    volatile Float64 c64 = 1.0 + ldexp( 1.0, -26 );
    const Float32 fused32 = ConvexContractedMultiplyAdd< Float32 >( a32, a32, -c32 );
    const Float64 fused64 = ConvexContractedMultiplyAdd< Float64 >( a64, a64, -c64 );
    GUCEF_TESTFW_SKIP_IF( ldexpf( 1.0f, -24 ) != fused32 || ldexp( 1.0, -54 ) != fused64, "this build does not contract a * b + c (no optimisation), the test cannot detect fusion" );
    try
    {
        SConvexResults< Float32 > contracted32;
        SConvexResults< Float32 > plain32;
        CallConvexQueriesContracted32( a32, c32, contracted32 );
        CallConvexQueriesPlain32( a32, c32, plain32 );
        SConvexResults< Float64 > contracted64;
        SConvexResults< Float64 > plain64;
        CallConvexQueriesContracted64( a64, c64, contracted64 );
        CallConvexQueriesPlain64( a64, c64, plain64 );
        UInt32 differences = 0;
        for ( UInt32 i=0; i<plain32.count; ++i )
            if ( !GeometryReference::SameBits( plain32.values[ i ], contracted32.values[ i ] ) )
                ++differences;
        for ( UInt32 i=0; i<plain64.count; ++i )
            if ( !GeometryReference::SameBits( plain64.values[ i ], contracted64.values[ i ] ) )
                ++differences;
        ASSERT_TRUE( plain32.count == contracted32.count && plain64.count == contracted64.count && plain32.count > 40 );
        #ifndef GUCEF_GEOMETRY_INLINE_ALL
        ASSERT_TRUE( 0 == differences );
        if ( 0 != differences )
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Gjk: " << differences << " results differ between contracting and plain callers" );
        #else
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Gjk: GUCEF_GEOMETRY_INLINE_ALL build, " << differences << " of " << ( plain32.count + plain64.count ) << " results differ when the caller contracts" );
        #endif
    }
    catch( ... )
    {
        ERRORHERE;
    }
    #endif /* GUCEF_TEST_GJK_CONTRACTION_SUPPORTED ? */
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
