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

#include "TestConvexReference.h"
#include "TestSatManifold.h"

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
using namespace ConvexReference;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The pinned FNV-1a hash of the TFixed32_32 box manifold run: identical in every build (gcc, clang, ASan, GUCEF_MATH_INLINE_ALL) */
static const UInt64 PinnedFixedSatHash = 3781882355827646953ULL;

/*-------------------------------------------------------------------------*/

/** The value of a kind in Float64 */
template < typename T >
static Float64
F64( const T& value )
{
    return CTNumericTraits< T >::ToFloat64( value );
}

template < typename T >
static P3
F64( const CTPoint3D< T >& p )
{
    return P3( F64( p.x ), F64( p.y ), F64( p.z ) );
}

template < typename T >
static V3
F64( const CTVector3D< T >& v )
{
    return V3( F64( v.x ), F64( v.y ), F64( v.z ) );
}

/*-------------------------------------------------------------------------*/

/**
 *  The checks every manifold of two boxes must pass: a unit normal from A towards B, every point of A in A's tolerance
 *  band and every point of B in B's, the points of a contact differing by normal * depth.
 */
template < typename T >
static bool
ManifoldIsConsistent( const CTOrientedBox3D< T >& a, const CTOrientedBox3D< T >& b, const CTContactManifold< T >& manifold, Float64 band )
{
    const CTOrientedBox3D< Float64 > wa( F64( a.centre ), Q4::FromXYZW( F64( a.rotation.x ), F64( a.rotation.y ), F64( a.rotation.z ), F64( a.rotation.w ) ).Normalized(), F64( a.halfExtents ) );
    const CTOrientedBox3D< Float64 > wb( F64( b.centre ), Q4::FromXYZW( F64( b.rotation.x ), F64( b.rotation.y ), F64( b.rotation.z ), F64( b.rotation.w ) ).Normalized(), F64( b.halfExtents ) );
    const V3 n = F64( manifold.normal );
    /* the overlap of the projections along the normal: no point can be deeper */
    const Float64 overlap = ( wa.GetSupport( n ) - wb.GetSupport( -n ) ).Dot( n );
    UInt32 reason = 0;
    if ( manifold.GetPointCount() < 1 || manifold.GetPointCount() > 4 || fabs( n.GetLength() - 1.0 ) >= band )
        reason = 1;
    for ( UInt32 i=0; i<manifold.GetPointCount() && 0 == reason; ++i )
    {
        const SContactPoint< T >& p = manifold.GetPoint( i );
        const P3 onA = F64( p.pointOnA );
        const P3 onB = F64( p.pointOnB );
        if ( wa.GetDistance( onA ) > band || wb.GetDistance( onB ) > band )
            reason = 2;
        else if ( ( ( onA - onB ) - n * F64( p.depth ) ).GetLength() > band )
            reason = 3;
        else if ( F64( p.depth ) < -band || F64( p.depth ) > overlap + band )
            reason = 4;
    }
    if ( 0 != reason )
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "SatManifold: inconsistent manifold (reason " << reason << ", overlap along the normal " << overlap << ")" );
    return 0 == reason;
}

/*-------------------------------------------------------------------------*/

/** Face-face stacking: a box on a larger box (rotated about the normal, offset in the face): four points on the contact face */
template < typename T >
static bool
StackingCases( UInt64 seed, Float64 band )
{
    typedef CTPoint3D< T > P;
    typedef CTVector3D< T > V;
    CRandom random( seed );
    UInt32 bad = 0;
    for ( UInt32 i=0; i<500; ++i )
    {
        const Float64 angle = random.Uniform( 0.0, 6.283185307179586 );
        const Float64 dx = random.Uniform( -0.4, 0.4 );
        const Float64 dz = random.Uniform( -0.4, 0.4 );
        const Float64 penetration = random.Uniform( 0.0, 0.05 );
        const Float64 small = random.Uniform( 0.2, 0.3 );
        const bool swap = 0 == i % 2;
        const CTOrientedBox3D< T > base( P( T( 0 ), T( 0 ), T( 0 ) ), CTQuaternion< T >(), V( T( 1 ), T( 1 ), T( 1 ) ) );
        const CTOrientedBox3D< T > top( ToP< T >( P3( dx, 1.0 + small - penetration, dz ) ), ToQ< T >( Q4::FromAxisAngle( D3::UnitY(), CTRadian< Float64 >( angle ) ) ), ToV< T >( V3( small, small, small ) ) );
        const CTOrientedBox3D< T >& a = swap ? top : base;
        const CTOrientedBox3D< T >& b = swap ? base : top;
        CTContactManifold< T > manifold;
        const bool touching = ComputeContact( a, b, manifold );
        bool ok = touching && 4 == manifold.GetPointCount() && ManifoldIsConsistent( a, b, manifold, band );
        const Float64 expectedNormalY = swap ? -1.0 : 1.0;
        ok = ok && fabs( F64( manifold.normal.y ) - expectedNormalY ) < band;
        for ( UInt32 k=0; k<manifold.GetPointCount() && ok; ++k )
        {
            const SContactPoint< T >& p = manifold.GetPoint( k );
            const P3 onBase = F64( swap ? p.pointOnB : p.pointOnA );
            ok = fabs( F64( p.depth ) - penetration ) < band && fabs( onBase.y - 1.0 ) < band;
        }
        if ( !ok )
        {
            ++bad;
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "SatManifold: stacking case " << i << " gave " << manifold.ToString() << ", seed " << seed );
        }
    }
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "SatManifold: stacking seed " << seed << ": 500 cases, " << bad << " bad" );
    return 0 == bad;
}

/*-------------------------------------------------------------------------*/

/** Edge against edge: a box turned 45 degrees about z under a box turned 45 degrees about x: one point */
template < typename T >
static bool
EdgeEdgeCases( Float64 band )
{
    typedef CTPoint3D< T > P;
    typedef CTVector3D< T > V;
    bool ok = true;
    const Float64 root2 = 1.4142135623730951;
    for ( UInt32 i=0; i<20; ++i )
    {
        const Float64 penetration = 0.001 + 0.002 * i;
        const Float64 shift = 0.03 * i - 0.3;
        const CTOrientedBox3D< T > a( P( T( 0 ), T( 0 ), T( 0 ) ), ToQ< T >( Q4::FromAxisAngle( D3::UnitZ(), CTRadian< Float64 >( 0.7853981633974483 ) ) ), V( T( 1 ), T( 1 ), T( 1 ) ) );
        const CTOrientedBox3D< T > b( ToP< T >( P3( shift, 2.0 * root2 - penetration, -shift ) ), ToQ< T >( Q4::FromAxisAngle( D3::UnitX(), CTRadian< Float64 >( 0.7853981633974483 ) ) ), V( T( 1 ), T( 1 ), T( 1 ) ) );
        CTContactManifold< T > manifold;
        const bool touching = ComputeContact( a, b, manifold );
        bool caseOk = touching && 1 == manifold.GetPointCount() && 0x2000 == ( manifold.GetPoint( 0 ).featureId & 0xF000 ) && ManifoldIsConsistent( a, b, manifold, band );
        caseOk = caseOk && fabs( F64( manifold.GetPoint( 0 ).depth ) - penetration ) < band && fabs( F64( manifold.normal.y ) - 1.0 ) < band;
        if ( !caseOk )
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "SatManifold: edge-edge case " << i << " gave " << manifold.ToString() );
        ok = ok && caseOk;
    }
    return ok;
}

/*-------------------------------------------------------------------------*/

/** Random box pairs: overlapping pairs give consistent manifolds whose deepest point has the SAT depth; separated pairs one point at minus the distance */
template < typename T >
static bool
RandomBoxSweep( UInt64 seed, UInt32 count, Float64 band )
{
    CRandom random( seed );
    UInt32 bad = 0;
    UInt32 overlapping = 0;
    UInt32 pointsTotal = 0;
    for ( UInt32 i=0; i<count; ++i )
    {
        CTOrientedBox3D< T > a;
        CTOrientedBox3D< T > b;
        RandomShape( random, a );
        RandomShape( random, b );
        CTContactManifold< T > manifold;
        const bool touching = ComputeContact( a, b, manifold );
        CTContactManifold< T > swapped;
        const bool touchingSwapped = ComputeContact( b, a, swapped );
        SSatBoxesResult< T > sat;
        CTSatBoxes< T >::Test( a, b, CTGeometryTolerance< T >(), sat );
        bool ok = touching == touchingSwapped;
        if ( touching )
        {
            ++overlapping;
            pointsTotal += manifold.GetPointCount();
            ok = ok && ManifoldIsConsistent( a, b, manifold, band ) && ManifoldIsConsistent( b, a, swapped, band );
            /* the deepest point reaches at least the penetration depth of the boxes (the minimum overlap) */
            ok = ok && F64( manifold.GetMaxDepth() ) >= F64( sat.depth ) - band;
        }
        else
        {
            ok = ok && 1 == manifold.GetPointCount() && fabs( -F64( manifold.GetPoint( 0 ).depth ) - F64( Distance( a, b ) ) ) <= band;
        }
        if ( !ok )
        {
            ++bad;
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "SatManifold: random pair " << i << " gave " << manifold.ToString() << ", seed " << seed );
        }
    }
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "SatManifold: random boxes seed " << seed << ": " << overlapping << " touching of " << count << ", " << pointsTotal << " points, " << bad << " bad" );
    return 0 == bad && overlapping > count / 10;
}

/*-------------------------------------------------------------------------*/

/** Reduce on a hand-made manifold: the deepest point stays, then the area is maximised */
static bool
ReduceCases( void )
{
    typedef CTPoint3D< Float64 > P;
    CTContactManifold< Float64 > manifold;
    manifold.normal = V3( 0.0, 1.0, 0.0 );
    /* an octagon of radius 1 in the plane y = 0 and its centre; the centre is the deepest point */
    const Float64 depths[ 9 ] = { 0.01, 0.02, 0.01, 0.02, 0.01, 0.02, 0.01, 0.02, 0.05 };
    for ( UInt32 k=0; k<8; ++k )
    {
        const Float64 angle = 0.7853981633974483 * k;
        const P p( cos( angle ), 0.0, sin( angle ) );
        manifold.AddPoint( p, p - manifold.normal * depths[ k ], depths[ k ], k );
    }
    manifold.AddPoint( P(), P() - manifold.normal * depths[ 8 ], depths[ 8 ], 8 );
    bool ok = !manifold.AddPoint( P(), P(), 0.0, 99 ) && 8 == manifold.GetPointCount();
    /* the ninth point did not fit: rebuild with eight points, the centre replacing the first */
    manifold.Clear();
    manifold.normal = V3( 0.0, 1.0, 0.0 );
    manifold.AddPoint( P(), P() - manifold.normal * 0.05, 0.05, 8 );
    for ( UInt32 k=1; k<8; ++k )
    {
        const Float64 angle = 0.7853981633974483 * k;
        const P p( cos( angle ), 0.0, sin( angle ) );
        manifold.AddPoint( p, p - manifold.normal * depths[ k ], depths[ k ], k );
    }
    CTContactManifold< Float64 > one = manifold;
    one.Reduce( 1 );
    ok = ok && 1 == one.GetPointCount() && 8 == one.GetPoint( 0 ).featureId;
    CTContactManifold< Float64 > two = manifold;
    two.Reduce( 2 );
    ok = ok && 2 == two.GetPointCount() && 8 == two.GetPoint( 0 ).featureId;
    CTContactManifold< Float64 > four = manifold;
    four.Reduce( 4 );
    ok = ok && 4 == four.GetPointCount() && 8 == four.GetPoint( 0 ).featureId;
    /* the three rim points span a large area: at least half of the octagon area spanned by any three of its points */
    const V3 e1 = four.GetPoint( 2 ).pointOnA - four.GetPoint( 1 ).pointOnA;
    const V3 e2 = four.GetPoint( 3 ).pointOnA - four.GetPoint( 1 ).pointOnA;
    ok = ok && e1.Cross( e2 ).GetLength() * 0.5 > 0.9;
    /* requests of 5, 6 and 7 points are honoured: the deepest point first, distinct points */
    for ( UInt32 request=5; request<=7; ++request )
    {
        CTContactManifold< Float64 > more = manifold;
        more.Reduce( request );
        ok = ok && request == more.GetPointCount() && 8 == more.GetPoint( 0 ).featureId;
        for ( UInt32 k=0; k<more.GetPointCount(); ++k )
            for ( UInt32 j=k+1; j<more.GetPointCount(); ++j )
                ok = ok && more.GetPoint( k ).featureId != more.GetPoint( j ).featureId;
        /* the first four are the ones Reduce( 4 ) keeps */
        for ( UInt32 k=0; k<4; ++k )
            ok = ok && more.GetPoint( k ).featureId == four.GetPoint( k ).featureId;
    }
    CTContactManifold< Float64 > all = manifold;
    all.Reduce( 8 );
    ok = ok && 8 == all.GetPointCount();
    CTContactManifold< Float64 > flipped = four;
    flipped.Flip();
    ok = ok && flipped.normal == -four.normal && flipped.GetPoint( 0 ).pointOnA == four.GetPoint( 0 ).pointOnB;
    ok = ok && fabs( four.GetMaxDepth() - 0.05 ) < 1.0e-15 && four.IsTouching();
    if ( !ok )
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "SatManifold: reduce cases failed: " << four.ToString() );
    return ok;
}

/*-------------------------------------------------------------------------*/

/** The axis aligned box forms and the empty box */
static bool
AxisAlignedCases( void )
{
    typedef CTPoint3D< Float64 > P;
    const CTAxisAlignedBox3D< Float64 > a( P( 0.0, 0.0, 0.0 ), P( 2.0, 2.0, 2.0 ) );
    const CTAxisAlignedBox3D< Float64 > b( P( 0.5, 1.9, 0.5 ), P( 1.5, 3.0, 1.5 ) );
    CTContactManifold< Float64 > m1;
    CTContactManifold< Float64 > m2;
    CTContactManifold< Float64 > m3;
    bool ok = ComputeContact( a, b, m1 ) && 4 == m1.GetPointCount() && fabs( m1.GetPoint( 0 ).depth - 0.1 ) < 1.0e-12 && m1.normal == V3( 0.0, 1.0, 0.0 );
    ok = ok && ComputeContact( a, CTOrientedBox3D< Float64 >::FromAxisAlignedBox( b ), m2 ) && 4 == m2.GetPointCount();
    ok = ok && ComputeContact( CTOrientedBox3D< Float64 >::FromAxisAlignedBox( b ), a, m3 ) && 4 == m3.GetPointCount() && m3.normal == V3( 0.0, -1.0, 0.0 );
    const CTAxisAlignedBox3D< Float64 > empty;
    ok = ok && !ComputeContact( a, empty, m1 ) && 0 == m1.GetPointCount();
    const CTOrientedBox3D< Float64 > invalid( P(), CTQuaternion< Float64 >(), V3( -1.0, 1.0, 1.0 ) );
    ok = ok && !ComputeContact( invalid, CTOrientedBox3D< Float64 >(), m1 ) && 0 == m1.GetPointCount();
    if ( !ok )
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "SatManifold: axis aligned cases failed: " << m1.ToString() );
    return ok;
}

/*-------------------------------------------------------------------------*/

/**
 *  A box resting on a larger box with its bottom face tilted by a tiny angle (nearly parallel edges, whose cross
 *  products are rounding noise): a face contact along the face normal, never an edge axis
 */
template < typename T >
static bool
NearlyParallelEdgeCases( Float64 band )
{
    typedef CTPoint3D< T > P;
    typedef CTVector3D< T > V;
    bool ok = true;
    const Float64 angles[ 8 ] = { 3.0e-3, 1.0e-3, 3.0e-4, 1.0e-4, 3.0e-5, 1.0e-5, 3.0e-6, 1.0e-6 };
    const Float64 bases[ 2 ] = { 1.0, 100.0 };
    for ( UInt32 b=0; b<2; ++b )
    {
        for ( UInt32 i=0; i<8; ++i )
        {
            const Float64 base = bases[ b ];
            const Q4 tilt = Q4::FromAxisAngle( D3( V3( 1.0, 0.0, 0.3 ) ), CTRadian< Float64 >( angles[ i ] ) );
            const CTOrientedBox3D< T > lower( P( T( 0 ), T( 0 ), T( 0 ) ), CTQuaternion< T >(), V( T( base ), T( base ), T( base ) ) );
            const CTOrientedBox3D< T > upper( ToP< T >( P3( 0.6 * base, 1.5 * base - 0.001 * base, 0.6 * base ) ), ToQ< T >( tilt ), ToV< T >( V3( 0.5 * base, 0.5 * base, 0.5 * base ) ) );
            CTContactManifold< T > manifold;
            bool caseOk = ComputeContact( lower, upper, manifold ) && manifold.GetPointCount() >= 1 && fabs( F64( manifold.normal.y ) - 1.0 ) < band;
            for ( UInt32 k=0; k<manifold.GetPointCount() && caseOk; ++k )
                caseOk = 0 == ( manifold.GetPoint( k ).featureId & 0x6000 );
            caseOk = caseOk && ManifoldIsConsistent( lower, upper, manifold, band * base );
            if ( !caseOk )
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "SatManifold: nearly parallel edges, angle " << angles[ i ] << " base " << base << " gave " << manifold.ToString() );
            ok = ok && caseOk;
        }
    }
    return ok;
}

/*-------------------------------------------------------------------------*/

/**
 *  A small rotated box sliding over the edge of a face: a point keeps its feature id from step to step (it moves with
 *  the slide), also when other points appear or disappear at the edge of the reference face
 */
template < typename T >
static bool
FeatureIdStability( Float64 band )
{
    typedef CTPoint3D< T > P;
    typedef CTVector3D< T > V;
    const CTOrientedBox3D< T > lower( P( T( 0 ), T( 0 ), T( 0 ) ), CTQuaternion< T >(), V( T( 1 ), T( 1 ), T( 1 ) ) );
    CTContactManifold< T > previous;
    bool ok = true;
    UInt32 kept = 0;
    for ( UInt32 step=0; step<=60 && ok; ++step )
    {
        const Float64 dx = 0.4 + 0.01 * step;
        const CTOrientedBox3D< T > upper( ToP< T >( P3( dx, 1.24, 0.0 ) ), ToQ< T >( Q4::FromAxisAngle( D3::UnitY(), CTRadian< Float64 >( 0.3 ) ) ), ToV< T >( V3( 0.25, 0.25, 0.25 ) ) );
        CTContactManifold< T > manifold;
        ok = ComputeContact( lower, upper, manifold ) && ManifoldIsConsistent( lower, upper, manifold, band );
        for ( UInt32 k=0; k<manifold.GetPointCount() && ok; ++k )
        {
            for ( UInt32 j=0; j<previous.GetPointCount(); ++j )
            {
                if ( previous.GetPoint( j ).featureId != manifold.GetPoint( k ).featureId )
                    continue;
                /*
                 *  the same feature: an incident corner moves with the slide (0.01), a cut of an incident edge by a side
                 *  plane along that edge (at most 0.01 / sin( 0.3 ), about 0.034)
                 */
                const Float64 moved = ( F64( manifold.GetPoint( k ).pointOnB ) - F64( previous.GetPoint( j ).pointOnB ) ).GetLength();
                const Float64 limit = ( manifold.GetPoint( k ).featureId & 0x3F ) < 4 ? 0.0101 : 0.035;
                ok = moved <= limit + band;
                ++kept;
                if ( !ok )
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "SatManifold: feature " << manifold.GetPoint( k ).featureId << " jumped by " << moved << " at slide " << dx );
            }
        }
        previous = manifold;
    }
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "SatManifold: sliding box: " << kept << " points kept their feature id between steps" );
    return ok && kept > 150;
}

/*-------------------------------------------------------------------------*/

/**
 *  Slightly separated face pairs: without a speculative distance one point at minus the gap; with one larger than the
 *  gap four face points at minus the gap (false: not touching); a tilted box keeps the corners within the distance
 */
template < typename T >
static bool
SpeculativeFaceCases( Float64 band )
{
    typedef CTPoint3D< T > P;
    typedef CTVector3D< T > V;
    const CTGeometryTolerance< T > tolerance;
    bool ok = true;
    const Float64 gaps[ 3 ] = { 0.001, 0.01, 0.04 };
    for ( UInt32 i=0; i<3; ++i )
    {
        const Float64 gap = gaps[ i ];
        const CTOrientedBox3D< T > lower( P( T( 0 ), T( 0 ), T( 0 ) ), CTQuaternion< T >(), V( T( 1 ), T( 1 ), T( 1 ) ) );
        const CTOrientedBox3D< T > upper( ToP< T >( P3( 0.2, 1.25 + gap, -0.1 ) ), ToQ< T >( Q4::FromAxisAngle( D3::UnitY(), CTRadian< Float64 >( 0.4 ) ) ), ToV< T >( V3( 0.25, 0.25, 0.25 ) ) );
        CTContactManifold< T > plain;
        bool caseOk = !CTSatBoxes< T >::ComputeContact( lower, upper, tolerance, 4, plain ) && 1 == plain.GetPointCount() && 0x4000 == plain.GetPoint( 0 ).featureId;
        caseOk = caseOk && fabs( F64( plain.GetPoint( 0 ).depth ) + gap ) < band;
        CTContactManifold< T > speculative;
        caseOk = caseOk && !CTSatBoxes< T >::ComputeContact( lower, upper, tolerance, T( 0.05 ), 4, speculative ) && 4 == speculative.GetPointCount();
        caseOk = caseOk && fabs( F64( speculative.normal.y ) - 1.0 ) < band;
        for ( UInt32 k=0; k<speculative.GetPointCount() && caseOk; ++k )
        {
            const SContactPoint< T >& p = speculative.GetPoint( k );
            caseOk = fabs( F64( p.depth ) + gap ) < band && fabs( F64( p.pointOnA.y ) - 1.0 ) < band && 0 == ( p.featureId & 0x6000 );
            caseOk = caseOk && ( ( F64( p.pointOnA ) - F64( p.pointOnB ) ) - F64( speculative.normal ) * F64( p.depth ) ).GetLength() < band;
        }
        /* a speculative distance below the gap: as without one */
        CTContactManifold< T > tooShort;
        caseOk = caseOk && !CTSatBoxes< T >::ComputeContact( lower, upper, tolerance, T( gap * 0.5 ), 4, tooShort ) && 1 == tooShort.GetPointCount() && 0x4000 == tooShort.GetPoint( 0 ).featureId;
        if ( !caseOk )
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "SatManifold: speculative gap " << gap << " gave " << plain.ToString() << " and " << speculative.ToString() );
        ok = ok && caseOk;
    }
    /* touching boxes with a tilted top: the corners that hover within the speculative distance are kept with negative depth */
    const CTOrientedBox3D< T > lower( P( T( 0 ), T( 0 ), T( 0 ) ), CTQuaternion< T >(), V( T( 1 ), T( 1 ), T( 1 ) ) );
    const CTOrientedBox3D< T > tilted( ToP< T >( P3( 0.0, 1.0 + 0.25 * cos( 0.05 ) + 0.25 * sin( 0.05 ) - 0.005, 0.0 ) ), ToQ< T >( Q4::FromAxisAngle( D3::UnitZ(), CTRadian< Float64 >( 0.05 ) ) ),
                                       ToV< T >( V3( 0.25, 0.25, 0.25 ) ) );
    CTContactManifold< T > plain;
    CTContactManifold< T > speculative;
    ok = ok && CTSatBoxes< T >::ComputeContact( lower, tilted, tolerance, 4, plain ) && 2 == plain.GetPointCount();
    ok = ok && CTSatBoxes< T >::ComputeContact( lower, tilted, tolerance, T( 0.1 ), 4, speculative ) && 4 == speculative.GetPointCount();
    UInt32 hovering = 0;
    for ( UInt32 k=0; k<speculative.GetPointCount(); ++k )
        hovering += F64( speculative.GetPoint( k ).depth ) < 0.0 ? 1 : 0;
    ok = ok && 2 == hovering;
    if ( !ok )
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "SatManifold: speculative tilted box gave " << plain.ToString() << " and " << speculative.ToString() );
    return ok;
}

/*-------------------------------------------------------------------------*/

/** The TFixed32_32 run: box manifolds of fixed-point boxes from integer draws, against Float64, hashed */
static bool
FixedPointManifoldRun( UInt64& hash )
{
    typedef TFixed32_32 TFixed;
    typedef CTPoint3D< TFixed > P;
    typedef CTVector3D< TFixed > V;
    CFixedRandom random( 9003 );
    CRawHash raw;
    UInt32 bad = 0;
    UInt32 touching = 0;
    for ( UInt32 i=0; i<2000; ++i )
    {
        const P ca = random.Point( 65536 );
        const P cb = random.Point( 65536 );
        const TFixed haX = random.Uniform( 13107, 65536 );
        const TFixed haY = random.Uniform( 13107, 65536 );
        const TFixed haZ = random.Uniform( 13107, 65536 );
        const V ha( haX, haY, haZ );
        const TFixed hbX = random.Uniform( 13107, 65536 );
        const TFixed hbY = random.Uniform( 13107, 65536 );
        const TFixed hbZ = random.Uniform( 13107, 65536 );
        const V hb( hbX, hbY, hbZ );
        const CTQuaternion< TFixed > qa = random.Rotation();
        const CTQuaternion< TFixed > qb = 0 == i % 4 ? CTQuaternion< TFixed >() : random.Rotation();
        const CTOrientedBox3D< TFixed > a( ca, 0 == i % 4 ? CTQuaternion< TFixed >() : qa, ha );
        const CTOrientedBox3D< TFixed > b( cb, qb, hb );
        CTContactManifold< TFixed > manifold;
        const bool contact = ComputeContact( a, b, manifold );
        touching += contact ? 1 : 0;
        raw.Add( contact );
        raw.Add3( manifold.normal );
        raw.Add( manifold.GetPointCount() );
        for ( UInt32 k=0; k<manifold.GetPointCount(); ++k )
        {
            raw.Add3( manifold.GetPoint( k ).pointOnA );
            raw.Add3( manifold.GetPoint( k ).pointOnB );
            raw.Add( manifold.GetPoint( k ).depth );
            raw.Add( manifold.GetPoint( k ).featureId );
        }
        if ( contact && !ManifoldIsConsistent( a, b, manifold, 1.0e-6 ) )
        {
            ++bad;
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "SatManifold: fixed-point manifold " << i << " inconsistent: " << manifold.ToString() );
        }
        /*
         *  An independent reference in Float64 on the same boxes: the penetration depth by EPA (Minkowski difference
         *  expansion, not axis tests) and the distance by brute force over the polytope features
         */
        const CTOrientedBox3D< Float64 > wa( FixedToP3( a.centre ), Q4::FromXYZW( FixedToFloat64( a.rotation.x ), FixedToFloat64( a.rotation.y ), FixedToFloat64( a.rotation.z ), FixedToFloat64( a.rotation.w ) ),
                                             V3( FixedToFloat64( a.halfExtents.x ), FixedToFloat64( a.halfExtents.y ), FixedToFloat64( a.halfExtents.z ) ) );
        const CTOrientedBox3D< Float64 > wb( FixedToP3( b.centre ), Q4::FromXYZW( FixedToFloat64( b.rotation.x ), FixedToFloat64( b.rotation.y ), FixedToFloat64( b.rotation.z ), FixedToFloat64( b.rotation.w ) ),
                                             V3( FixedToFloat64( b.halfExtents.x ), FixedToFloat64( b.halfExtents.y ), FixedToFloat64( b.halfExtents.z ) ) );
        const SPolytope pa = BoxPolytope( wa );
        const SPolytope pb = BoxPolytope( wb );
        const Float64 separation = PolytopeSeparation( pa, pb );
        bool referenceOk = true;
        if ( separation > 1.0e-6 )
        {
            referenceOk = !contact && fabs( -F64( manifold.GetPoint( 0 ).depth ) - PolytopeDistance( pa, pb ) ) <= 1.0e-6;
        }
        else if ( separation < -1.0e-6 )
        {
            CTContactManifold< Float64 > epa;
            ComputeSupportContact( CTSupportMapping< CTOrientedBox3D< Float64 > >::Make( wa ), CTSupportMapping< CTOrientedBox3D< Float64 > >::Make( wb ), CTGeometryTolerance< Float64 >(), epa );
            SSatBoxesResult< TFixed > sat;
            CTSatBoxes< TFixed >::Test( a, b, CTGeometryTolerance< TFixed >(), sat );
            const Float64 depth = epa.GetPoint( 0 ).depth;
            /* the SAT minimum overlap is the penetration depth; the manifold may prefer a face axis up to 7 % deeper */
            referenceOk = contact && fabs( F64( sat.depth ) - depth ) <= 1.0e-6 && F64( manifold.GetMaxDepth() ) >= depth - 1.0e-6 && F64( manifold.GetMaxDepth() ) <= depth * 1.08 + 0.01;
        }
        if ( !referenceOk )
        {
            ++bad;
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "SatManifold: fixed-point manifold " << i << " differs from the Float64 EPA and brute-force reference: " << manifold.ToString() );
        }
    }
    hash = raw.GetHash();
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "SatManifold: TFixed32_32 run: " << raw.GetCount() << " raw values, hash " << CORE::ToString( hash ) << ", " << touching << " touching, " << bad << " bad" );
    return 0 == bad && touching > 200;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformSatManifoldTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "SatManifold" );

    GUCEF_TESTFW_TESTCASE( "SatManifold: face-face stacking gives four points on the contact face, within both boxes' tolerance bands" )
    try
    {
        ASSERT_TRUE( StackingCases< Float64 >( 701, 1.0e-9 ) );
        ASSERT_TRUE( StackingCases< Float32 >( 702, 1.0e-5 ) );
        ASSERT_TRUE( StackingCases< TFixed32_32 >( 703, 1.0e-6 ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "SatManifold: edge against edge gives one point with the edge feature" )
    try
    {
        ASSERT_TRUE( EdgeEdgeCases< Float64 >( 1.0e-9 ) );
        ASSERT_TRUE( EdgeEdgeCases< Float32 >( 1.0e-5 ) );
        ASSERT_TRUE( EdgeEdgeCases< TFixed32_32 >( 1.0e-6 ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "SatManifold: 10000 seeded box pairs: consistent manifolds in both orders, no point deeper than SAT, separated pairs at minus the distance" )
    try
    {
        ASSERT_TRUE( RandomBoxSweep< Float64 >( 711, ScaledCount( 10000 ), 1.0e-6 ) );
        ASSERT_TRUE( RandomBoxSweep< Float32 >( 712, ScaledCount( 10000 ), 1.0e-4 ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "SatManifold: Reduce keeps the deepest point and maximises the area, honouring requests of 1 to 8 points; Flip; the axis aligned forms and boxes without points" )
    try
    {
        ASSERT_TRUE( ReduceCases() );
        ASSERT_TRUE( AxisAlignedCases() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "SatManifold: nearly parallel edges give the face contact; feature ids follow a sliding box; speculative contacts of slightly separated faces" )
    try
    {
        ASSERT_TRUE( NearlyParallelEdgeCases< Float64 >( 1.0e-9 ) );
        ASSERT_TRUE( NearlyParallelEdgeCases< Float32 >( 1.0e-4 ) );
        ASSERT_TRUE( NearlyParallelEdgeCases< TFixed32_32 >( 1.0e-6 ) );
        ASSERT_TRUE( FeatureIdStability< Float64 >( 1.0e-9 ) );
        ASSERT_TRUE( FeatureIdStability< Float32 >( 1.0e-5 ) );
        ASSERT_TRUE( FeatureIdStability< TFixed32_32 >( 1.0e-6 ) );
        ASSERT_TRUE( SpeculativeFaceCases< Float64 >( 1.0e-9 ) );
        ASSERT_TRUE( SpeculativeFaceCases< Float32 >( 1.0e-5 ) );
        ASSERT_TRUE( SpeculativeFaceCases< TFixed32_32 >( 1.0e-6 ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "SatManifold: TFixed32_32 box manifolds are consistent, agree with an independent Float64 reference (EPA depth, brute-force distance) and match the pinned hash" )
    try
    {
        UInt64 hash = 0;
        ASSERT_TRUE( FixedPointManifoldRun( hash ) );
        ASSERT_TRUE( PinnedFixedSatHash == hash );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
