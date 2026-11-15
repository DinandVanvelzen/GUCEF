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

#ifndef GUCEF_GEOMETRY_GEOSPACES_H
#include "gucefGEOMETRY_geospaces.h"
#define GUCEF_GEOMETRY_GEOSPACES_H
#endif /* GUCEF_GEOMETRY_GEOSPACES_H ? */

#ifndef GUCEF_GEOMETRY_CTRAYCASTER_H
#include "gucefGEOMETRY_CTRayCaster.h"
#define GUCEF_GEOMETRY_CTRAYCASTER_H
#endif /* GUCEF_GEOMETRY_CTRAYCASTER_H ? */

#ifndef GUCEF_GEOMETRY_TIMEOFIMPACT_H
#include "gucefGEOMETRY_TimeOfImpact.h"
#define GUCEF_GEOMETRY_TIMEOFIMPACT_H
#endif /* GUCEF_GEOMETRY_TIMEOFIMPACT_H ? */

#include "TestConvexReference.h"
#include "TestGeometryAllocationCounter.h"
#include "TestEpa.h"

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

/** The pinned FNV-1a hash of the TFixed32_32 contact run: identical in every build (gcc, clang, ASan, GUCEF_MATH_INLINE_ALL) */
static const UInt64 PinnedFixedEpaHash = 12767040127697083745ULL;

/** The pinned FNV-1a hash of the TFixed32_32 exact-depth run (ExactDepthAgreement): identical in every build */
static const UInt64 PinnedExactDepthHash = 12515617788984925393ULL;

/** The pinned FNV-1a hash of the TFixed32_32 zero-margin cast run (ZeroMarginCastSweep): identical in every build */
static const UInt64 PinnedZeroMarginCastHash = 1122764527615971405ULL;

/*-------------------------------------------------------------------------*/

/** EPA on the cores of two support mappings from their GJK simplex */
template < typename T, typename TA, typename TB >
static SEpaResult< T >
EpaOf( const TA& a, const TB& b, const CTGeometryTolerance< T >& tolerance, bool& coresOverlap )
{
    SGjkResult< T > gjk;
    CTGjk< T >::ComputeDistance( a, b, tolerance, GUCEF_NULL, gjk );
    coresOverlap = gjk.coresOverlapping;
    SEpaResult< T > epa;
    CTEpa< T >::Penetration( a, b, gjk.simplex, tolerance, epa );
    return epa;
}

/*-------------------------------------------------------------------------*/

/**
 *  The support of the Minkowski difference of the whole shapes along the unit direction u (Float64): the translation of
 *  B along u that separates the shapes. The penetration depth is its minimum over all directions.
 */
template < typename TA, typename TB >
static Float64
SeparatingTranslation( const TA& a, const TB& b, const V3& u )
{
    const P3 pa = GetSupportWithMargin( CTSupportMapping< TA >::Make( a ), u );
    const P3 pb = GetSupportWithMargin( CTSupportMapping< TB >::Make( b ), -u );
    return ( pa - pb ).Dot( u );
}

/*-------------------------------------------------------------------------*/

/** The penetration of overlapping spheres: the contact query (point cores plus margins) and EPA on the curved sphere supports */
template < typename T >
static bool
SphereDepthSweep( UInt64 seed, UInt32 count )
{
    CRandom random( seed );
    UInt32 bad = 0;
    UInt32 checked = 0;
    UInt32 epaChecked = 0;
    UInt32 notConverged = 0;
    Float64 worst = 0.0;
    Float64 worstEpa = 0.0;
    for ( UInt32 i=0; i<count; ++i )
    {
        CTSphere< T > a;
        CTSphere< T > b;
        RandomShape( random, a );
        RandomShape( random, b );
        const CTSphere< Float64 > wa = Widen( a );
        const CTSphere< Float64 > wb = Widen( b );
        const V3 between = wb.centre - wa.centre;
        const Float64 d = between.GetLength();
        const Float64 expected = wa.radius + wb.radius - d;
        if ( expected < 1.0e-3 || d < 1.0e-3 )
            continue;
        ++checked;
        CTContactManifold< T > manifold;
        const bool touching = ComputeContact( a, b, manifold );
        const Float64 depth = (Float64) manifold.GetPoint( 0 ).depth;
        const V3 n = Widen( manifold.normal );
        const V3 gap = Widen( manifold.GetPoint( 0 ).pointOnA ) - Widen( manifold.GetPoint( 0 ).pointOnB );
        const Float64 error = fabs( depth - expected );
        worst = error > worst ? error : worst;
        if ( !touching || error > 1.0e-4 || n.Dot( between * ( 1.0 / d ) ) < 1.0 - 1.0e-4 || ( gap - n * depth ).GetLength() > 1.0e-4 )
        {
            ++bad;
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Epa: sphere contact depth " << depth << " expected " << expected << ", seed " << seed << " pair " << i );
        }
        if ( 0 == i % 5 )
        {
            /* EPA itself on the spheres as curved shapes without margins */
            ++epaChecked;
            bool coresOverlap = false;
            const CTGeometryTolerance< T > tolerance( T( 1.0e-9 ), sizeof( T ) == 4 ? T( 1.0e-6 ) : T( 1.0e-8 ), T( 1.0e-9 ) );
            const SEpaResult< T > epa = EpaOf< T >( CTShapeSupport< CTSphere< T > >( a ), CTShapeSupport< CTSphere< T > >( b ), tolerance, coresOverlap );
            const Float64 epaError = fabs( (Float64) epa.depth - expected );
            /*
             *  a deep overlap of curved shapes needs more faces than the caps allow: EPA then reports NOT_CONVERGED with
             *  a lower bound of the depth within 1e-2 (counted, at most 5 % of the runs); converged runs are within 1e-4
             */
            const bool converged = 0 == ( epa.status & MATHSTATUS_NOT_CONVERGED );
            notConverged += converged ? 0 : 1;
            worstEpa = converged && epaError > worstEpa ? epaError : worstEpa;
            const bool depthOk = converged ? epaError <= 1.0e-4 : ( (Float64) epa.depth <= expected + 1.0e-6 && epaError <= 1.0e-2 );
            const bool normalOk = !converged || Widen( epa.normal ).Dot( between * ( 1.0 / d ) ) >= 1.0 - 1.0e-3;
            if ( !coresOverlap || !depthOk || !normalOk || epa.iterations > (UInt32) CTEpa< T >::MaxIterations )
            {
                ++bad;
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Epa: sphere EPA depth " << (Float64) epa.depth << " expected " << expected << " status " << epa.status << ", seed " << seed << " pair " << i );
            }
        }
    }
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Epa: sphere depths seed " << seed << ": " << checked << " contacts (worst " << worst << "), " << epaChecked << " EPA runs (worst converged "
               << worstEpa << ", " << notConverged << " at a cap), " << bad << " bad" );
    return 0 == bad && checked > count / 10 && notConverged * 20 <= epaChecked;
}

/*-------------------------------------------------------------------------*/

/** EPA on overlapping oriented boxes (support mappings, no SAT) against the SAT minimum overlap */
template < typename T >
static bool
BoxDepthSweep( UInt64 seed, UInt32 count )
{
    CRandom random( seed );
    UInt32 bad = 0;
    UInt32 checked = 0;
    Float64 worst = 0.0;
    Float64 worstWitness = 0.0;
    for ( UInt32 i=0; i<count; ++i )
    {
        CTOrientedBox3D< T > a;
        CTOrientedBox3D< T > b;
        RandomShape( random, a );
        RandomShape( random, b );
        SSatBoxesResult< T > sat;
        if ( !CTSatBoxes< T >::Test( a, b, CTGeometryTolerance< T >(), sat ) || sat.depth < T( 1.0e-3 ) )
            continue;
        ++checked;
        bool coresOverlap = false;
        const SEpaResult< T > epa = EpaOf< T >( CTShapeSupport< CTOrientedBox3D< T > >( a ), CTShapeSupport< CTOrientedBox3D< T > >( b ), CTGeometryTolerance< T >(), coresOverlap );
        const Float64 depth = (Float64) epa.depth;
        const Float64 error = fabs( depth - (Float64) sat.depth );
        worst = error > worst ? error : worst;
        /* the normal is a direction of minimum overlap: separating B along it takes the depth */
        const V3 n = Widen( epa.normal );
        const Float64 along = SeparatingTranslation( Widen( a ), Widen( b ), n );
        const V3 gap = Widen( epa.pointOnA ) - Widen( epa.pointOnB );
        /* the witness points come from barycentric weights on a face of the polytope: Float32 resolves them to about 1e-4 */
        const Float64 witnessError = ( gap - n * depth ).GetLength();
        worstWitness = witnessError > worstWitness ? witnessError : worstWitness;
        if ( !coresOverlap || error > 1.0e-4 || fabs( along - depth ) > 1.0e-4 || witnessError > ( sizeof( T ) == 4 ? 5.0e-4 : 1.0e-6 ) || 0 != ( epa.status & MATHSTATUS_NOT_CONVERGED ) )
        {
            ++bad;
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Epa: box depth " << depth << " SAT " << (Float64) sat.depth << " along normal " << along << " witness error " << ( gap - n * depth ).GetLength()
                             << " status " << epa.status << ", seed " << seed << " pair " << i );
        }
    }
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Epa: box depths seed " << seed << ": " << checked << " overlapping pairs, worst difference to SAT " << worst << ", worst witness error " << worstWitness << ", " << bad << " bad" );
    return 0 == bad && checked > count / 10;
}

/*-------------------------------------------------------------------------*/

/**
 *  The contact query of random convex pairs: for overlapping pairs the depth is no larger than the separating
 *  translation along any of 200 directions (it is the minimum), equals it along the normal, and the points differ by
 *  normal * depth; for separated pairs the depth is minus the distance.
 */
template < typename TA, typename TB >
static bool
ContactSweep( const char* name, UInt64 seed, UInt32 count )
{
    CRandom random( seed );
    GUCEF::vector< V3 > directions;
    for ( UInt32 k=0; k<200; ++k )
        directions.push_back( random.Direction().AsVector() );
    UInt32 bad = 0;
    UInt32 overlapping = 0;
    for ( UInt32 i=0; i<count; ++i )
    {
        TA a;
        TB b;
        RandomShape( random, a );
        RandomShape( random, b );
        CTContactManifold< Float64 > manifold;
        const bool touching = ComputeContact( a, b, manifold );
        const SContactPoint< Float64 >& point = manifold.GetPoint( 0 );
        const V3 n = manifold.normal;
        bool ok = 1 == manifold.GetPointCount() && fabs( n.GetLength() - 1.0 ) < 1.0e-9;
        if ( touching && point.depth > 1.0e-4 )
        {
            ++overlapping;
            const Float64 along = SeparatingTranslation( a, b, n );
            ok = ok && fabs( along - point.depth ) < 1.0e-4 && ( ( point.pointOnA - point.pointOnB ) - n * point.depth ).GetLength() < 1.0e-4;
            for ( UInt32 k=0; k<directions.size() && ok; ++k )
                ok = SeparatingTranslation( a, b, directions[ k ] ) > point.depth - 1.0e-4;
        }
        else if ( !touching )
        {
            ok = ok && fabs( -point.depth - Distance( a, b ) ) < 1.0e-6;
        }
        if ( !ok )
        {
            ++bad;
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Epa: contact " << name << " depth " << point.depth << ", seed " << seed << " pair " << i );
        }
    }
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Epa: contact " << name << " seed " << seed << ": " << overlapping << " overlapping of " << count << ", " << bad << " bad" );
    return 0 == bad && overlapping > count / 20;
}

/*-------------------------------------------------------------------------*/

/** Degenerate polytopes for EPA: touching, identical, point cores, coplanar flat shapes, far from the origin */
template < typename T >
static bool
DegenerateEpaCases( const CTVector3D< T >& offset, Float64 tolerance )
{
    typedef CTPoint3D< T > P;
    typedef CTVector3D< T > V;
    bool ok = true;
    const T zero = T( 0 );
    const T one = T( 1 );
    const T two = T( 2 );
    CTContactManifold< T > manifold;

    /* touching boxes: depth 0 */
    const CTOrientedBox3D< T > unit( P( zero, zero, zero ) + offset, CTQuaternion< T >(), V( one, one, one ) );
    const CTOrientedBox3D< T > touching( P( two, zero, zero ) + offset, CTQuaternion< T >(), V( one, one, one ) );
    bool coresOverlap = false;
    SEpaResult< T > epa = EpaOf< T >( CTShapeSupport< CTOrientedBox3D< T > >( unit ), CTShapeSupport< CTOrientedBox3D< T > >( touching ), CTGeometryTolerance< T >(), coresOverlap );
    ok = ok && fabs( CTNumericTraits< T >::ToFloat64( epa.depth ) ) <= tolerance;
    /* identical boxes: the depth is the smallest full extent */
    const CTOrientedBox3D< T > flat( P( zero, zero, zero ) + offset, CTQuaternion< T >(), V( two, one / two, one ) );
    epa = EpaOf< T >( CTShapeSupport< CTOrientedBox3D< T > >( flat ), CTShapeSupport< CTOrientedBox3D< T > >( flat ), CTGeometryTolerance< T >(), coresOverlap );
    ok = ok && coresOverlap && fabs( CTNumericTraits< T >::ToFloat64( epa.depth ) - 1.0 ) <= tolerance && fabs( fabs( CTNumericTraits< T >::ToFloat64( epa.normal.y ) ) - 1.0 ) <= tolerance;
    /* identical spheres: point cores in one place, the depth is the sum of the radii in some unit direction */
    const CTSphere< T > sphere( P( one, zero, zero ) + offset, one );
    ok = ok && ComputeContact( sphere, sphere, manifold ) && fabs( CTNumericTraits< T >::ToFloat64( manifold.GetPoint( 0 ).depth ) - 2.0 ) <= tolerance;
    ok = ok && fabs( CTNumericTraits< T >::ToFloat64( manifold.normal.GetLength() ) - 1.0 ) <= tolerance;
    /* crossing capsules: segment cores crossing, a flat Minkowski difference of the cores */
    const CTCapsule< T > c1( P( -one, zero, zero ) + offset, P( one, zero, zero ) + offset, one / two );
    const CTCapsule< T > c2( P( zero, -one, zero ) + offset, P( zero, one, zero ) + offset, one / two );
    ok = ok && ComputeContact( c1, c2, manifold ) && fabs( CTNumericTraits< T >::ToFloat64( manifold.GetPoint( 0 ).depth ) - 1.0 ) <= tolerance;
    ok = ok && fabs( fabs( CTNumericTraits< T >::ToFloat64( manifold.normal.z ) ) - 1.0 ) <= tolerance;
    /* coplanar triangles that overlap: a flat difference, depth 0 along the plane normal */
    const CTTriangle3D< T > t1( P( zero, zero, zero ) + offset, P( two, zero, zero ) + offset, P( zero, two, zero ) + offset );
    const CTTriangle3D< T > t2( P( one / two, one / two, zero ) + offset, P( two + one, one / two, zero ) + offset, P( one / two, two + one, zero ) + offset );
    epa = EpaOf< T >( CTShapeSupport< CTTriangle3D< T > >( t1 ), CTShapeSupport< CTTriangle3D< T > >( t2 ), CTGeometryTolerance< T >(), coresOverlap );
    ok = ok && coresOverlap && 0 != ( epa.status & MATHSTATUS_DEGENERATE ) && fabs( CTNumericTraits< T >::ToFloat64( epa.depth ) ) <= tolerance;
    ok = ok && fabs( fabs( CTNumericTraits< T >::ToFloat64( epa.normal.z ) ) - 1.0 ) <= tolerance;
    if ( !ok )
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Epa: degenerate cases failed at offset " << offset.ToString() << " for a kind of " << (UInt32) sizeof( T ) << " bytes" );
    return ok;
}

/*-------------------------------------------------------------------------*/

/** The realtime-safe calls of this package on prepared shapes */
template < typename T >
static void
CallConvexRealtimeQueries( UInt32& accumulator )
{
    typedef CTPoint3D< T > P;
    typedef CTVector3D< T > V;
    typedef CTDirectionVector3D< T > D;
    const CTOrientedBox3D< T > box1( P( 0, 0, 0 ), CTQuaternion< T >::FromAxisAngle( D( V( 1, 1, 0 ) ), CTRadian< T >( T( 0.4 ) ) ), V( 1, 2, 1 ) );
    const CTOrientedBox3D< T > box2( P( T( 1.5 ), 1, 0 ), CTQuaternion< T >::FromAxisAngle( D( V( 0, 1, 1 ) ), CTRadian< T >( T( 0.7 ) ) ), V( 1, 1, 1 ) );
    const CTCylinder< T > cylinder( P( 0, 0, 0 ), P( 1, 2, 1 ), T( 0.6 ) );
    const CTCone< T > cone( P( 1, 0, 0 ), D( V( 0, 1, 1 ) ), 2, 1 );
    const CTCapsule< T > capsule( P( 0, 1, 0 ), P( 1, 2, 1 ), T( 0.5 ) );
    const CTSphere< T > sphere( P( 1, 1, 0 ), T( 0.8 ) );
    P p1;
    P p2;
    CTContactManifold< T > manifold;
    SRayCastResult< T > hit;
    SToiResult< T > toi;
    UInt32 bits = 0;
    bits += ClosestPoints( cylinder, cone, p1, p2 ) ? 1 : 0;
    bits += ClosestPoints( capsule, box1, p1, p2 ) ? 1 : 0;
    bits += Intersects( sphere, cone ) ? 1 : 0;
    bits += ComputeContact( box1, box2, manifold ) ? 1 : 0;
    bits += ComputeContact( cylinder, cone, manifold ) ? 1 : 0;
    bits += ComputeContact( capsule, box2, manifold ) ? 1 : 0;
    bits += ComputeContact( sphere, cylinder, manifold ) ? 1 : 0;
    bits += SphereCast( CTRay3D< T >( P( -4, 0, 0 ), D( V( 1, 0, 0 ) ) ), T( 0.3 ), box1, T( 10 ), hit ) ? 1 : 0;
    bits += SphereCast( CTRay3D< T >( P( -4, 0, 0 ), D( V( 1, 0, 0 ) ) ), T( 0.3 ), CTAxisAlignedBox3D< T >( P( 0, 0, 0 ), P( 1, 1, 1 ) ), T( 10 ), hit ) ? 1 : 0;
    bits += ShapeCast( capsule, D( V( 0, -1, 0 ) ), T( 10 ), cone, hit ) ? 1 : 0;
    bits += TimeOfImpact( box1, CTMotion3D< T >( P( -4, 0, 0 ), CTQuaternion< T >(), V( 8, 0, 0 ), V( 0, 0, 3 ) ), box2, CTMotion3D< T >(), T( 0.01 ), toi ) ? 1 : 0;
    SSatBoxesResult< T > sat;
    bits += CTSatBoxes< T >::Test( box1, box2, CTGeometryTolerance< T >(), sat ) ? 1 : 0;
    bits += CTSatBoxes< T >::ComputeContact( box1, box2, CTGeometryTolerance< T >(), T( 0.1 ), 6, manifold ) ? 1 : 0;
    static CTEpa< T > workspace;
    bits += 0 == ComputeSupportContact( CTSupportMapping< CTCylinder< T > >::Make( cylinder ), CTSupportMapping< CTCone< T > >::Make( cone ), CTGeometryTolerance< T >(), workspace, manifold ) ? 1 : 0;
    accumulator += bits;
}

/*-------------------------------------------------------------------------*/

/** The Float64 value of a fixed-point vector (exact for components below 2^21) */
static V3
FixedToV3( const CTVector3D< TFixed32_32 >& v )
{
    return V3( FixedToFloat64( v.x ), FixedToFloat64( v.y ), FixedToFloat64( v.z ) );
}

/*-------------------------------------------------------------------------*/

/**
 *  TFixed32_32 EPA in a caller-provided workspace on overlapping box pairs at sizes 10 to 1000 and offsets 10^3 to
 *  10^5: the depth against Float64 EPA on the same boxes, the witness points (pointOnA - pointOnB = normal * depth, on
 *  the boxes), a unit normal, and the same bits as the convenience form with the work object on the stack
 */
static bool
FixedScaleEpaSweep( UInt64 seed )
{
    typedef TFixed32_32 TFixed;
    typedef CTOrientedBox3D< TFixed > TBox;
    typedef CTShapeSupport< TBox > TSupport;
    CRandom random( seed );
    CTEpa< TFixed > workspace;
    const Float64 sizes[ 3 ] = { 10.0, 100.0, 1000.0 };
    const Float64 offsets[ 3 ] = { 1.0e3, 1.0e4, 1.0e5 };
    UInt32 bad = 0;
    UInt32 checked = 0;
    Float64 worstDepth = 0.0;
    Float64 worstWitness = 0.0;
    for ( UInt32 s=0; s<3; ++s )
    {
        for ( UInt32 o=0; o<3; ++o )
        {
            const Float64 size = sizes[ s ];
            const Float64 band = 1.0e-5 * size;
            const P3 base( offsets[ o ], -offsets[ o ], 0.5 * offsets[ o ] );
            for ( UInt32 i=0; i<40; ++i )
            {
                const V3 offsetA = random.Vector( 0.5 * size );
                const V3 offsetB = random.Vector( 0.5 * size );
                const Q4 rotationA = random.Rotation();
                const Q4 rotationB = random.Rotation();
                const Float64 ax = random.Uniform( 0.2, 1.0 ) * size;
                const Float64 ay = random.Uniform( 0.2, 1.0 ) * size;
                const Float64 az = random.Uniform( 0.2, 1.0 ) * size;
                const Float64 bx = random.Uniform( 0.2, 1.0 ) * size;
                const Float64 by = random.Uniform( 0.2, 1.0 ) * size;
                const Float64 bz = random.Uniform( 0.2, 1.0 ) * size;
                const TBox fa( ToP< TFixed >( base + offsetA ), ToQ< TFixed >( rotationA ), ToV< TFixed >( V3( ax, ay, az ) ) );
                const TBox fb( ToP< TFixed >( base + offsetB ), ToQ< TFixed >( rotationB ), ToV< TFixed >( V3( bx, by, bz ) ) );
                const CTOrientedBox3D< Float64 > wa( FixedToP3( fa.centre ), Q4::FromXYZW( FixedToFloat64( fa.rotation.x ), FixedToFloat64( fa.rotation.y ), FixedToFloat64( fa.rotation.z ), FixedToFloat64( fa.rotation.w ) ),
                                                     FixedToV3( fa.halfExtents ) );
                const CTOrientedBox3D< Float64 > wb( FixedToP3( fb.centre ), Q4::FromXYZW( FixedToFloat64( fb.rotation.x ), FixedToFloat64( fb.rotation.y ), FixedToFloat64( fb.rotation.z ), FixedToFloat64( fb.rotation.w ) ),
                                                     FixedToV3( fb.halfExtents ) );
                SGjkResult< TFixed > gjk;
                CTGjk< TFixed >::ComputeDistance( TSupport( fa ), TSupport( fb ), CTGeometryTolerance< TFixed >(), GUCEF_NULL, gjk );
                bool referenceOverlap = false;
                const SEpaResult< Float64 > reference = EpaOf< Float64 >( CTShapeSupport< CTOrientedBox3D< Float64 > >( wa ), CTShapeSupport< CTOrientedBox3D< Float64 > >( wb ), CTGeometryTolerance< Float64 >(), referenceOverlap );
                if ( !gjk.coresOverlapping || !referenceOverlap || reference.depth < band )
                    continue;
                ++checked;
                SEpaResult< TFixed > epa;
                const MATH::TMathStatus status = CTEpa< TFixed >::Penetration( TSupport( fa ), TSupport( fb ), gjk.simplex, CTGeometryTolerance< TFixed >(), workspace, epa );
                SEpaResult< TFixed > convenience;
                CTEpa< TFixed >::Penetration( TSupport( fa ), TSupport( fb ), gjk.simplex, CTGeometryTolerance< TFixed >(), convenience );
                const Float64 depth = FixedToFloat64( epa.depth );
                const V3 normal = FixedToV3( epa.normal );
                const P3 onA = FixedToP3( epa.pointOnA );
                const P3 onB = FixedToP3( epa.pointOnB );
                const Float64 depthError = fabs( depth - reference.depth );
                const Float64 witnessError = ( ( onA - onB ) - normal * depth ).GetLength();
                worstDepth = depthError / size > worstDepth ? depthError / size : worstDepth;
                worstWitness = witnessError / size > worstWitness ? witnessError / size : worstWitness;
                bool ok = 0 == ( status & MATHSTATUS_NOT_CONVERGED ) && depthError <= band && witnessError <= band && fabs( normal.GetLength() - 1.0 ) <= 1.0e-6;
                ok = ok && wa.GetDistance( onA ) <= band && wb.GetDistance( onB ) <= band;
                ok = ok && convenience.depth == epa.depth && convenience.normal == epa.normal && convenience.pointOnA == epa.pointOnA && convenience.pointOnB == epa.pointOnB;
                if ( !ok )
                {
                    ++bad;
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Epa: fixed-point box pair " << i << " size " << size << " offset " << offsets[ o ] << ": depth " << depth << " (Float64 " << reference.depth
                                     << "), witness error " << witnessError << ", status " << status << ", seed " << seed );
                }
            }
        }
    }
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Epa: TFixed32_32 at sizes 10 to 1000 and offsets 10^3 to 10^5, seed " << seed << ": " << checked << " overlapping pairs, worst depth error / size "
               << worstDepth << ", worst witness error / size " << worstWitness << ", " << bad << " bad" );
    return 0 == bad && checked > 200;
}

/*-------------------------------------------------------------------------*/

/** The status of the contact query: in its result and in manifold.status, NOT_CONVERGED at a cap, DEGENERATE for flat touching cores */
static bool
ContactStatusCases( void )
{
    typedef CTPoint3D< Float64 > P;
    /* curved supports with a zero tolerance: EPA can only stop at its cap */
    const CTSphere< Float64 > a( P( 0.0, 0.0, 0.0 ), 1.0 );
    const CTSphere< Float64 > b( P( 1.0, 0.3, 0.0 ), 1.0 );
    CTContactManifold< Float64 > capped;
    const MATH::TMathStatus cappedStatus = ComputeSupportContact( CTShapeSupport< CTSphere< Float64 > >( a ), CTShapeSupport< CTSphere< Float64 > >( b ), CTGeometryTolerance< Float64 >( 0.0, 0.0, 0.0 ), capped );
    bool ok = 0 != ( cappedStatus & MATHSTATUS_NOT_CONVERGED ) && cappedStatus == capped.status && 1 == capped.GetPointCount();
    /* the same through a workspace */
    CTEpa< Float64 > workspace;
    CTContactManifold< Float64 > cappedInWorkspace;
    ok = ok && cappedStatus == ComputeSupportContact( CTShapeSupport< CTSphere< Float64 > >( a ), CTShapeSupport< CTSphere< Float64 > >( b ), CTGeometryTolerance< Float64 >( 0.0, 0.0, 0.0 ), workspace, cappedInWorkspace );
    ok = ok && cappedInWorkspace.GetPoint( 0 ).depth == capped.GetPoint( 0 ).depth && cappedInWorkspace.normal == capped.normal;
    /* two coplanar overlapping triangles: a flat Minkowski difference, touching at depth 0 */
    const CTTriangle3D< Float64 > t1( P( 0.0, 0.0, 0.0 ), P( 2.0, 0.0, 0.0 ), P( 0.0, 2.0, 0.0 ) );
    const CTTriangle3D< Float64 > t2( P( 0.5, 0.5, 0.0 ), P( 2.5, 0.5, 0.0 ), P( 0.5, 2.5, 0.0 ) );
    CTContactManifold< Float64 > flat;
    ok = ok && ComputeContact( t1, t2, flat ) && 0 != ( flat.status & MATHSTATUS_DEGENERATE ) && fabs( flat.GetPoint( 0 ).depth ) <= 1.0e-9;
    /* an ordinary overlap: status OK; Clear resets the status */
    CTContactManifold< Float64 > plain;
    ok = ok && ComputeContact( CTSphere< Float64 >( P( 0.0, 0.0, 0.0 ), 1.0 ), CTCapsule< Float64 >( P( 1.5, -1.0, 0.0 ), P( 1.5, 1.0, 0.0 ), 1.0 ), plain ) && MATHSTATUS_OK == plain.status;
    capped.Clear();
    ok = ok && MATHSTATUS_OK == capped.status && 0 == capped.GetPointCount();
    if ( !ok )
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Epa: contact status cases failed: capped " << cappedStatus << ", flat " << flat.status << ", plain " << plain.status );
    return ok;
}

/*-------------------------------------------------------------------------*/

/** The TFixed32_32 contact run: the contact query on fixed-point shapes from integer draws against Float64, hashed */
static bool
FixedPointContactRun( UInt64& hash )
{
    typedef TFixed32_32 TFixed;
    typedef CTPoint3D< TFixed > P;
    typedef CTVector3D< TFixed > V;
    CFixedRandom random( 9002 );
    CRawHash raw;
    UInt32 bad = 0;
    UInt32 overlapping = 0;
    Float64 worst = 0.0;
    for ( UInt32 i=0; i<2000; ++i )
    {
        const UInt32 kind = i % 3;
        const P ca = random.Point( 65536 );
        const P cb = random.Point( 65536 );
        const TFixed ra = random.Uniform( 13107, 65536 );
        const TFixed rb = random.Uniform( 13107, 65536 );
        const TFixed haX = random.Uniform( 13107, 65536 );
        const TFixed haY = random.Uniform( 13107, 65536 );
        const TFixed haZ = random.Uniform( 13107, 65536 );
        const V ha( haX, haY, haZ );
        const CTQuaternion< TFixed > qa = random.Rotation();
        const V axis = random.Vector( 65536 );
        CTContactManifold< TFixed > fixed;
        CTContactManifold< Float64 > reference;
        if ( 0 == kind )
        {
            ComputeContact( CTCapsule< TFixed >( ca, ca + axis, ra ), CTCapsule< TFixed >( cb, cb + ha, rb ), fixed );
            ComputeContact( CTCapsule< Float64 >( FixedToP3( ca ), FixedToP3( ca + axis ), FixedToFloat64( ra ) ), CTCapsule< Float64 >( FixedToP3( cb ), FixedToP3( cb + ha ), FixedToFloat64( rb ) ), reference );
        }
        else if ( 1 == kind )
        {
            const CTOrientedBox3D< TFixed > box( ca, qa, ha );
            const CTOrientedBox3D< Float64 > wide( FixedToP3( ca ), Q4::FromXYZW( FixedToFloat64( qa.x ), FixedToFloat64( qa.y ), FixedToFloat64( qa.z ), FixedToFloat64( qa.w ) ), V3( FixedToFloat64( ha.x ), FixedToFloat64( ha.y ), FixedToFloat64( ha.z ) ) );
            ComputeContact( CTSphere< TFixed >( cb, rb ), box, fixed );
            ComputeContact( CTSphere< Float64 >( FixedToP3( cb ), FixedToFloat64( rb ) ), wide, reference );
        }
        else
        {
            P points[ 8 ];
            CTPoint3D< Float64 > widened[ 8 ];
            for ( UInt32 k=0; k<8; ++k )
            {
                points[ k ] = cb + random.Vector( 65536 );
                widened[ k ] = FixedToP3( points[ k ] );
            }
            ComputeContact( CTCylinder< TFixed >( ca, ca + axis, ra ), CTConvexHull3D< TFixed >( points, 8 ), fixed );
            ComputeContact( CTCylinder< Float64 >( FixedToP3( ca ), FixedToP3( ca + axis ), FixedToFloat64( ra ) ), CTConvexHull3D< Float64 >( widened, 8 ), reference );
        }
        raw.Add3( fixed.normal );
        raw.Add( fixed.GetPointCount() );
        for ( UInt32 k=0; k<fixed.GetPointCount(); ++k )
        {
            raw.Add3( fixed.GetPoint( k ).pointOnA );
            raw.Add3( fixed.GetPoint( k ).pointOnB );
            raw.Add( fixed.GetPoint( k ).depth );
        }
        const Float64 depth = FixedToFloat64( fixed.GetPoint( 0 ).depth );
        overlapping += depth > 0.0 ? 1 : 0;
        const Float64 error = fabs( depth - reference.GetPoint( 0 ).depth );
        worst = error > worst ? error : worst;
        /* EPA on curved cores (cylinder) converges to its tolerance only: 1e-5 */
        if ( error > ( 2 == kind ? 1.0e-5 : 1.0e-6 ) )
        {
            ++bad;
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Epa: fixed-point contact depth " << depth << " Float64 " << reference.GetPoint( 0 ).depth << " kind " << kind << " pair " << i );
        }
    }
    hash = raw.GetHash();
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Epa: TFixed32_32 contact run: " << raw.GetCount() << " raw values, hash " << CORE::ToString( hash ) << ", " << overlapping
               << " overlapping, worst difference to Float64 " << worst << ", " << bad << " bad" );
    return 0 == bad && overlapping > 200;
}

/*-------------------------------------------------------------------------*/

/** The Float64 value of the three exported kinds (fixed point from its raw value) */
static Float64 ToWide( const Float32 value ) { return (Float64) value; }
static Float64 ToWide( const Float64 value ) { return value; }
static Float64 ToWide( const TFixed32_32& value ) { return FixedToFloat64( value ); }

/** A fixed-point draw in the kind T: exact in Float64 and TFixed32_32, rounded once in Float32 */
template < typename T >
static T FromFixed( const TFixed32_32& value ) { return (T) FixedToFloat64( value ); }

template <>
TFixed32_32 FromFixed< TFixed32_32 >( const TFixed32_32& value ) { return value; }

template < typename T >
static CTPoint3D< T > PointFromFixed( const CTPoint3D< TFixed32_32 >& p ) { return CTPoint3D< T >( FromFixed< T >( p.x ), FromFixed< T >( p.y ), FromFixed< T >( p.z ) ); }

template < typename T >
static CTVector3D< T > VectorFromFixed( const CTVector3D< TFixed32_32 >& v ) { return CTVector3D< T >( FromFixed< T >( v.x ), FromFixed< T >( v.y ), FromFixed< T >( v.z ) ); }

template < typename T >
static CTQuaternion< T > RotationFromFixed( const CTQuaternion< TFixed32_32 >& q ) { return CTQuaternion< T >::FromXYZW( FromFixed< T >( q.x ), FromFixed< T >( q.y ), FromFixed< T >( q.z ), FromFixed< T >( q.w ) ).Normalized(); }

template < typename T >
static P3 WideP( const CTPoint3D< T >& p ) { return P3( ToWide( p.x ), ToWide( p.y ), ToWide( p.z ) ); }

template < typename T >
static V3 WideV( const CTVector3D< T >& v ) { return V3( ToWide( v.x ), ToWide( v.y ), ToWide( v.z ) ); }

template < typename T >
static CTOrientedBox3D< Float64 >
WideBox( const CTOrientedBox3D< T >& b )
{
    const Q4 rotation = Q4::FromXYZW( ToWide( b.rotation.x ), ToWide( b.rotation.y ), ToWide( b.rotation.z ), ToWide( b.rotation.w ) ).Normalized();
    return CTOrientedBox3D< Float64 >( WideP( b.centre ), rotation, WideV( b.halfExtents ) );
}

/** The fixed-point values of a result into the hash; the float kinds are not hashed */
static void HashValue( CRawHash& /*raw*/, const Float32 /*value*/ ) {}
static void HashValue( CRawHash& /*raw*/, const Float64 /*value*/ ) {}
static void HashValue( CRawHash& raw, const TFixed32_32& value ) { raw.Add( value ); }

template < typename V >
static void HashVector( CRawHash& raw, const V& v )
{
    HashValue( raw, v.x );
    HashValue( raw, v.y );
    HashValue( raw, v.z );
}

/*-------------------------------------------------------------------------*/

/** The exact penetration depth of two overlapping boxes: the smallest overlap over the 15 axes of the separating axis test, and that axis */
static Float64
ExactBoxBoxDepth( const CTOrientedBox3D< Float64 >& a, const CTOrientedBox3D< Float64 >& b, UInt32& axis )
{
    SSatBoxesResult< Float64 > sat;
    CTSatBoxes< Float64 >::Test( a, b, CTGeometryTolerance< Float64 >(), sat );
    axis = sat.axis;
    return sat.overlapping ? sat.depth : -1.0;
}

/*-------------------------------------------------------------------------*/

/** The distance of a point from a box centred at the origin, in the frame of the box */
static Float64
PointBoxDistance( const Float64 ( &p )[ 3 ], const Float64 ( &half )[ 3 ] )
{
    Float64 squared = 0.0;
    for ( UInt32 k=0; k<3; ++k )
    {
        const Float64 excess = fabs( p[ k ] ) - half[ k ];
        squared += excess > 0.0 ? excess * excess : 0.0;
    }
    return sqrt( squared );
}

/*-------------------------------------------------------------------------*/

/**
 *  The exact penetration depth of a capsule and a box: the radius plus the signed depth of the segment core. Cores
 *  that overlap: the smallest overlap over the face normals of the Minkowski difference of the segment and the box
 *  (the 3 box axes and the 3 cross products of the segment with them); cores apart: minus their distance, by a
 *  ternary search over the segment (the distance to a convex set is convex along it). A sphere is a capsule with
 *  start = end.
 */
static Float64
ExactCapsuleBoxDepth( const P3& start, const P3& end, const Float64 radius, const CTOrientedBox3D< Float64 >& box )
{
    V3 axes[ 3 ];
    box.GetAxes( axes );
    const Float64 half[ 3 ] = { box.halfExtents.x, box.halfExtents.y, box.halfExtents.z };
    Float64 s[ 3 ];
    Float64 d[ 3 ];
    for ( UInt32 k=0; k<3; ++k )
    {
        s[ k ] = axes[ k ].Dot( start - box.centre );
        d[ k ] = axes[ k ].Dot( end - box.centre ) - s[ k ];
    }
    bool separated = false;
    Float64 depth = 1.0e300;
    for ( UInt32 k=0; k<3; ++k )
    {
        const Float64 low = d[ k ] < 0.0 ? s[ k ] + d[ k ] : s[ k ];
        const Float64 high = d[ k ] < 0.0 ? s[ k ] : s[ k ] + d[ k ];
        const Float64 overlap = high + half[ k ] < half[ k ] - low ? high + half[ k ] : half[ k ] - low;
        separated = separated || overlap < 0.0;
        depth = overlap < depth ? overlap : depth;
    }
    const Float64 length = sqrt( d[ 0 ] * d[ 0 ] + d[ 1 ] * d[ 1 ] + d[ 2 ] * d[ 2 ] );
    for ( UInt32 k=0; k<3 && length > 0.0; ++k )
    {
        /* d x e_k in the frame of the box */
        Float64 u[ 3 ] = { 0.0, 0.0, 0.0 };
        u[ ( k + 1 ) % 3 ] = d[ ( k + 2 ) % 3 ];
        u[ ( k + 2 ) % 3 ] = -d[ ( k + 1 ) % 3 ];
        const Float64 norm = sqrt( u[ 0 ] * u[ 0 ] + u[ 1 ] * u[ 1 ] + u[ 2 ] * u[ 2 ] );
        if ( norm < 1.0e-9 * length )
            continue;
        const Float64 reach = ( half[ 0 ] * fabs( u[ 0 ] ) + half[ 1 ] * fabs( u[ 1 ] ) + half[ 2 ] * fabs( u[ 2 ] ) ) / norm;
        const Float64 overlap = reach - fabs( s[ 0 ] * u[ 0 ] + s[ 1 ] * u[ 1 ] + s[ 2 ] * u[ 2 ] ) / norm;
        separated = separated || overlap < 0.0;
        depth = overlap < depth ? overlap : depth;
    }
    if ( !separated )
        return radius + depth;
    Float64 low = 0.0;
    Float64 high = 1.0;
    for ( UInt32 n=0; n<200; ++n )
    {
        const Float64 t1 = low + ( high - low ) / 3.0;
        const Float64 t2 = high - ( high - low ) / 3.0;
        const Float64 p1[ 3 ] = { s[ 0 ] + d[ 0 ] * t1, s[ 1 ] + d[ 1 ] * t1, s[ 2 ] + d[ 2 ] * t1 };
        const Float64 p2[ 3 ] = { s[ 0 ] + d[ 0 ] * t2, s[ 1 ] + d[ 1 ] * t2, s[ 2 ] + d[ 2 ] * t2 };
        if ( PointBoxDistance( p1, half ) < PointBoxDistance( p2, half ) )
            high = t2;
        else
            low = t1;
    }
    const Float64 t = 0.5 * ( low + high );
    const Float64 p[ 3 ] = { s[ 0 ] + d[ 0 ] * t, s[ 1 ] + d[ 1 ] * t, s[ 2 ] + d[ 2 ] * t };
    return radius - PointBoxDistance( p, half );
}

/*-------------------------------------------------------------------------*/

/**
 *  EPA on the full support mappings (no margins) of overlapping pairs against their exact penetration depth: boxes
 *  axis aligned and rotated (with the pairs whose minimum is an edge-edge axis counted), sphere-sphere, sphere-box and
 *  capsule-box. The guarantee of a converged EPA (see CTEpa, Accuracy): with allowed = max( absolute,
 *  relative * depth, the fixed-point resolution ) its termination band, on the boxes exact - allowed <= depth <=
 *  exact up to rounding (a few units of the kind times the coordinate scale), on the curved shapes
 *  | depth - exact | <= allowed (the normals of the small faces resolve to the rounding unit over their size); along the normal the separating translation is depth up to the band on the boxes and within 1e-4
 *  on the curved shapes. Boxes always converge; a curved run stopped at a cap (NOT_CONVERGED) is a lower bound. The
 *  fixed-point results go into the hash.
 */
template < typename T >
static bool
ExactDepthAgreement( UInt64 seed, UInt32 count, UInt64& hash )
{
    typedef TFixed32_32 TFixed;
    typedef CTNumericTraits< T > TTraits;
    typedef CTOrientedBox3D< T > TBox;
    typedef CTSphere< T > TSphere;
    typedef CTCapsule< T > TCapsule;
    enum { KindCount = 5 };
    static const char* const kindNames[ KindCount ] = { "axis aligned box-box", "rotated box-box", "sphere-sphere", "sphere-box", "capsule-box" };
    CFixedRandom random( seed );
    CRawHash raw;
    const CTGeometryTolerance< T > tolerance;
    /* the coordinates of the Minkowski differences stay below 8: the rounding unit and the fixed-point resolution of EPA (4096 units) at that scale */
    const Float64 scale = 8.0;
    const Float64 unit = ToWide( TTraits::Epsilon() ) * scale;
    const Float64 resolution = TTraits::IsFloatingPoint ? 0.0 : 4096.0 * unit;
    const Float64 rounding = 16.0 * unit;
    UInt32 checked[ KindCount ] = { 0, 0, 0, 0, 0 };
    UInt32 capped[ KindCount ] = { 0, 0, 0, 0, 0 };
    Float64 worst[ KindCount ] = { 0.0, 0.0, 0.0, 0.0, 0.0 };
    Float64 worstWitness[ KindCount ] = { 0.0, 0.0, 0.0, 0.0, 0.0 };
    Float64 worstAlong[ KindCount ] = { 0.0, 0.0, 0.0, 0.0, 0.0 };
    UInt32 edgeMinimum = 0;
    UInt32 bad = 0;
    for ( UInt32 i=0; i<count; ++i )
    {
        const UInt32 kind = i % KindCount;
        /* every draw into its own local: the same shapes in every kind and build */
        const CTPoint3D< TFixed > ca = random.Point( 65536 );
        const CTPoint3D< TFixed > cb = random.Point( 65536 );
        const CTQuaternion< TFixed > qa = random.Rotation();
        const CTQuaternion< TFixed > qb = random.Rotation();
        const TFixed ax = random.Uniform( 13107, 65536 );
        const TFixed ay = random.Uniform( 13107, 65536 );
        const TFixed az = random.Uniform( 13107, 65536 );
        const TFixed bx = random.Uniform( 13107, 65536 );
        const TFixed by = random.Uniform( 13107, 65536 );
        const TFixed bz = random.Uniform( 13107, 65536 );
        const TFixed ra = random.Uniform( 13107, 78643 );
        const TFixed rb = random.Uniform( 13107, 78643 );
        const CTVector3D< TFixed > segment = random.Vector( 52429 );

        const CTQuaternion< T > identity;
        const TBox boxA( PointFromFixed< T >( ca ), 0 == kind ? identity : RotationFromFixed< T >( qa ), VectorFromFixed< T >( CTVector3D< TFixed >( ax, ay, az ) ) );
        const TBox boxB( PointFromFixed< T >( cb ), 0 == kind ? identity : RotationFromFixed< T >( qb ), VectorFromFixed< T >( CTVector3D< TFixed >( bx, by, bz ) ) );
        const TSphere sphereA( PointFromFixed< T >( ca ), FromFixed< T >( ra ) );
        const TSphere sphereB( PointFromFixed< T >( cb ), FromFixed< T >( rb ) );
        const TCapsule capsule( PointFromFixed< T >( ca ), PointFromFixed< T >( ca + segment ), FromFixed< T >( ra ) );
        const CTOrientedBox3D< Float64 > wideA = WideBox( boxA );
        const CTOrientedBox3D< Float64 > wideB = WideBox( boxB );
        const CTSphere< Float64 > wideSphereA( WideP( sphereA.centre ), ToWide( sphereA.radius ) );
        const CTSphere< Float64 > wideSphereB( WideP( sphereB.centre ), ToWide( sphereB.radius ) );
        const CTCapsule< Float64 > wideCapsule( WideP( capsule.start ), WideP( capsule.end ), ToWide( capsule.radius ) );

        Float64 exact = -1.0;
        UInt32 satAxis = 0;
        if ( kind < 2 )
            exact = ExactBoxBoxDepth( wideA, wideB, satAxis );
        else if ( 2 == kind )
            exact = ( wideSphereB.centre - wideSphereA.centre ).GetLength() < 1.0e-3 ? -1.0 : wideSphereA.radius + wideSphereB.radius - ( wideSphereB.centre - wideSphereA.centre ).GetLength();
        else if ( 3 == kind )
            exact = ExactCapsuleBoxDepth( wideSphereA.centre, wideSphereA.centre, wideSphereA.radius, wideB );
        else
            exact = ExactCapsuleBoxDepth( wideCapsule.start, wideCapsule.end, wideCapsule.radius, wideB );
        if ( exact < 1.0e-3 )
            continue;

        bool coresOverlap = false;
        SEpaResult< T > epa;
        if ( kind < 2 )
            epa = EpaOf< T >( CTShapeSupport< TBox >( boxA ), CTShapeSupport< TBox >( boxB ), tolerance, coresOverlap );
        else if ( 2 == kind )
            epa = EpaOf< T >( CTShapeSupport< TSphere >( sphereA ), CTShapeSupport< TSphere >( sphereB ), tolerance, coresOverlap );
        else if ( 3 == kind )
            epa = EpaOf< T >( CTShapeSupport< TSphere >( sphereA ), CTShapeSupport< TBox >( boxB ), tolerance, coresOverlap );
        else
            epa = EpaOf< T >( CTShapeSupport< TCapsule >( capsule ), CTShapeSupport< TBox >( boxB ), tolerance, coresOverlap );
        ++checked[ kind ];
        edgeMinimum += 1 == kind && satAxis >= 6 ? 1 : 0;

        HashValue( raw, epa.depth );
        HashVector( raw, epa.normal );
        HashVector( raw, epa.pointOnA );
        HashVector( raw, epa.pointOnB );
        raw.Add( (UInt32) epa.status );
        raw.Add( epa.iterations );

        const Float64 depth = ToWide( epa.depth );
        const V3 normal = WideV( epa.normal );
        Float64 along = 0.0;
        if ( kind < 2 )
            along = SeparatingTranslation( wideA, wideB, normal );
        else if ( 2 == kind )
            along = SeparatingTranslation( wideSphereA, wideSphereB, normal );
        else if ( 3 == kind )
            along = SeparatingTranslation( wideSphereA, wideB, normal );
        else
            along = SeparatingTranslation( wideCapsule, wideB, normal );

        const Float64 allowed = TTraits::IsFloatingPoint ? ( ToWide( tolerance.relative ) * exact > ToWide( tolerance.absolute ) ? ToWide( tolerance.relative ) * exact : ToWide( tolerance.absolute ) )
                                                         : ( resolution > ToWide( tolerance.absolute ) ? resolution : ToWide( tolerance.absolute ) );
        const bool converged = 0 == ( epa.status & MATHSTATUS_NOT_CONVERGED );
        capped[ kind ] += converged ? 0 : 1;
        const Float64 witness = ( ( WideP( epa.pointOnA ) - WideP( epa.pointOnB ) ) - normal * depth ).GetLength();
        worstWitness[ kind ] = converged && witness > worstWitness[ kind ] ? witness : worstWitness[ kind ];
        /*
         *  The excess over the guaranteed band in rounding units. Above the exact depth: rounding on the boxes; on a
         *  converged curved shape the normals of its small faces resolve to the rounding unit over the face size,
         *  which keeps the depth within the band of the exact depth from either side. Along
         *  the normal: the band on the boxes; on the curved shapes the normal is the one of the adjacent face that
         *  holds the projection of the origin (see CTEpa), whose support can reach up to 1e-4 of the size further.
         */
        const bool curved = kind >= 2;
        const Float64 above = curved ? rounding + allowed : rounding;
        const Float64 alongLimit = curved ? allowed + 1.0e-4 * ( 1.0 + exact ) : allowed + rounding;
        Float64 excess = depth - exact - above;
        if ( converged )
        {
            excess = ( exact - allowed - rounding ) - depth > excess ? ( exact - allowed - rounding ) - depth : excess;
            excess = along - depth - alongLimit > excess ? along - depth - alongLimit : excess;
        }
        excess = excess > 0.0 ? excess / unit : 0.0;
        worst[ kind ] = excess > worst[ kind ] ? excess : worst[ kind ];
        worstAlong[ kind ] = converged && along - depth > worstAlong[ kind ] ? along - depth : worstAlong[ kind ];
        const Float64 normalLimit = TTraits::IsFloatingPoint ? 16.0 * ToWide( TTraits::Epsilon() ) : 1.0e-6;
        const bool ok = coresOverlap && 0.0 == excess && fabs( normal.GetLength() - 1.0 ) <= normalLimit && ( converged || curved );
        if ( !ok )
        {
            ++bad;
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Epa: exact depth " << kindNames[ kind ] << " pair " << i << ": EPA " << depth << " exact " << exact << ", in 1e-9: EPA - exact " << ( depth - exact ) * 1.0e9 << " along the normal - EPA " << ( along - depth ) * 1.0e9
                             << " allowed " << allowed * 1.0e9 << " |normal| - 1 " << normal.GetLength() - 1.0 << " cores overlap " << ( coresOverlap ? 1 : 0 ) << " status " << epa.status << " iterations " << epa.iterations << ", seed " << seed );
        }
    }
    hash = raw.GetHash();
    UInt32 total = 0;
    for ( UInt32 k=0; k<(UInt32) KindCount; ++k )
    {
        total += checked[ k ];
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Epa: exact depth (kind size " << (UInt32) sizeof( T ) << ") " << kindNames[ k ] << ": " << checked[ k ] << " overlapping, " << capped[ k ]
                   << " at a cap, worst excess beyond the limits " << worst[ k ] << " rounding units, worst separating translation along the normal - depth (1e-9) "
                   << worstAlong[ k ] * 1.0e9 << ", worst witness error (1e-9) " << worstWitness[ k ] * 1.0e9 );
    }
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Epa: exact depth seed " << seed << ": " << total << " pairs, " << edgeMinimum << " with an edge-edge minimum, hash " << CORE::ToString( hash ) << ", " << bad << " bad" );
    bool enough = edgeMinimum > 0;
    for ( UInt32 k=0; k<(UInt32) KindCount; ++k )
        /* deep curved pairs can need more faces than the caps allow: at most a fifth of the runs (Float64 sphere-sphere, relative 1e-10, about 9 %) */
        enough = enough && checked[ k ] * KindCount * 10 > count && capped[ k ] * 5 <= checked[ k ] && ( k >= 2 || 0 == capped[ k ] );
    return 0 == bad && enough;
}

/**
 *  A TFixed32_32 sphere pair overlapping by 0.0213 on which GJK once stopped without progress with the origin about
 *  8e-8 beyond its final triangle (a tetrahedron within the resolution of the origin) and reported the cores as
 *  separated by that: GJK reports the overlap and EPA the depth r1 + r2 - d within the fixed-point band
 */
static bool
FixedGjkNoProgressOverlap( void )
{
    typedef TFixed32_32 TFixed;
    typedef CTPoint3D< TFixed > P;
    const CTSphere< TFixed > a( P( TFixed::FromRaw( 412893253 ), TFixed::FromRaw( -3524777511LL ), TFixed::FromRaw( -4013540587LL ) ), TFixed::FromRaw( 5145274403LL ) );
    const CTSphere< TFixed > b( P( TFixed::FromRaw( 3021469020LL ), TFixed::FromRaw( 3432606389LL ), TFixed::FromRaw( 1945952503LL ) ), TFixed::FromRaw( 4471059526LL ) );
    const CTSphere< Float64 > wa( FixedToP3( a.centre ), FixedToFloat64( a.radius ) );
    const CTSphere< Float64 > wb( FixedToP3( b.centre ), FixedToFloat64( b.radius ) );
    const Float64 expected = wa.radius + wb.radius - ( wb.centre - wa.centre ).GetLength();
    SGjkResult< TFixed > gjk;
    CTGjk< TFixed >::ComputeDistance( CTShapeSupport< CTSphere< TFixed > >( a ), CTShapeSupport< CTSphere< TFixed > >( b ), CTGeometryTolerance< TFixed >(), GUCEF_NULL, gjk );
    bool coresOverlap = false;
    const SEpaResult< TFixed > epa = EpaOf< TFixed >( CTShapeSupport< CTSphere< TFixed > >( a ), CTShapeSupport< CTSphere< TFixed > >( b ), CTGeometryTolerance< TFixed >(), coresOverlap );
    CTContactManifold< TFixed > manifold;
    const bool touching = 0 == ( ComputeSupportContact( CTShapeSupport< CTSphere< TFixed > >( a ), CTShapeSupport< CTSphere< TFixed > >( b ), CTGeometryTolerance< TFixed >(), manifold ) & MATHSTATUS_NOT_CONVERGED ) && manifold.IsTouching();
    const Float64 depth = FixedToFloat64( epa.depth );
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Epa: TFixed32_32 GJK without progress near the origin: overlapping " << ( gjk.coresOverlapping ? 1 : 0 ) << ", EPA depth " << depth << " expected " << expected
               << ", contact depth " << FixedToFloat64( manifold.GetPoint( 0 ).depth ) );
    return expected > 0.02 && gjk.coresOverlapping && gjk.overlapping && coresOverlap && fabs( depth - expected ) <= 1.0e-5 && touching && fabs( FixedToFloat64( manifold.GetPoint( 0 ).depth ) - expected ) <= 1.0e-5;
}

/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/

/** The distance of A moved by t * translation from B (Float64 GJK, independent of the cast) */
template < typename TA, typename TB >
static Float64
MovedDistance( const TA& a, const V3& translation, const Float64 t, const TB& b )
{
    typedef typename CTSupportMapping< TA >::TSupport TSupportA;
    const CTTransformedSupport< TSupportA > moved( CTSupportMapping< TA >::Make( a ), CTMatrix3D< Float64 >::Identity(), translation * t );
    SGjkResult< Float64 > gjk;
    CTGjk< Float64 >::ComputeDistance( moved, CTSupportMapping< TB >::Make( b ), CTGeometryTolerance< Float64 >(), GUCEF_NULL, gjk );
    return gjk.distance;
}

/*-------------------------------------------------------------------------*/

/**
 *  The brute-force reference of a cast (Float64): the distance of the moved A from B is convex in the fraction t, so
 *  a ternary search finds its minimum and bisections the first and the last fraction of contact. 'closest' is that
 *  minimum (0 when they meet); 'depth' is the penetration depth (EPA) halfway between the first and the last fraction
 *  of contact, to tell clear hits from grazing ones; returns the first fraction of contact, or -1 for a miss.
 */
template < typename TA, typename TB >
static Float64
ReferenceCast( const TA& a, const V3& translation, const TB& b, Float64& closest, Float64& depth )
{
    Float64 low = 0.0;
    Float64 high = 1.0;
    Float64 meets = -1.0;
    for ( UInt32 n=0; n<100 && meets < 0.0; ++n )
    {
        const Float64 t1 = low + ( high - low ) / 3.0;
        const Float64 t2 = high - ( high - low ) / 3.0;
        const Float64 d1 = MovedDistance( a, translation, t1, b );
        const Float64 d2 = MovedDistance( a, translation, t2, b );
        /* a fraction of contact ends the search: the distance is flat (0) over the contact interval */
        if ( d1 <= 0.0 )
            meets = t1;
        else if ( d2 <= 0.0 )
            meets = t2;
        else if ( d1 <= d2 )
            high = t2;
        else
            low = t1;
    }
    if ( meets < 0.0 )
        meets = 0.5 * ( low + high );
    closest = MovedDistance( a, translation, meets, b );
    depth = 0.0;
    if ( closest > 0.0 )
        return -1.0;
    Float64 first = 0.0;
    if ( MovedDistance( a, translation, 0.0, b ) > 0.0 )
    {
        Float64 apart = 0.0;
        first = meets;
        for ( UInt32 n=0; n<100; ++n )
        {
            const Float64 middle = 0.5 * ( apart + first );
            if ( MovedDistance( a, translation, middle, b ) > 0.0 )
                apart = middle;
            else
                first = middle;
        }
    }
    Float64 last = 1.0;
    if ( MovedDistance( a, translation, 1.0, b ) > 0.0 )
    {
        Float64 apart = 1.0;
        last = meets;
        for ( UInt32 n=0; n<100; ++n )
        {
            const Float64 middle = 0.5 * ( apart + last );
            if ( MovedDistance( a, translation, middle, b ) > 0.0 )
                apart = middle;
            else
                last = middle;
        }
    }
    const Float64 deepest = 0.5 * ( first + last );
    typedef typename CTSupportMapping< TA >::TSupport TSupportA;
    const CTTransformedSupport< TSupportA > moved( CTSupportMapping< TA >::Make( a ), CTMatrix3D< Float64 >::Identity(), translation * deepest );
    CTContactManifold< Float64 > contact;
    ComputeSupportContact( moved, CTSupportMapping< TB >::Make( b ), CTGeometryTolerance< Float64 >(), contact );
    depth = contact.GetPointCount() > 0 ? contact.GetPoint( 0 ).depth : 0.0;
    return first;
}

/*-------------------------------------------------------------------------*/

/**
 *  GJK casts (CTGjk::Cast) of an oriented box without margin against a triangle, an oriented box and a hull (no
 *  margins either) against the brute-force reference: a clear hit (penetration deeper than 'grazing' at the
 *  deepest fraction) is a hit at the reference fraction within 'tolerance' of the translation length, a clear miss
 *  (closest distance beyond 'grazing') is a miss; grazing pairs are only counted. The fixed-point results go into the
 *  hash.
 */
template < typename T >
static bool
ZeroMarginCastSweep( UInt64 seed, UInt32 count, const Float64 tolerance, const Float64 grazing, UInt64& hash )
{
    typedef TFixed32_32 TFixed;
    typedef CTNumericTraits< T > TTraits;
    typedef CTOrientedBox3D< T > TBox;
    enum { KindCount = 3 };
    static const char* const kindNames[ KindCount ] = { "box-triangle", "box-box", "box-hull" };
    CFixedRandom random( seed );
    CRawHash raw;
    UInt32 hits[ KindCount ] = { 0, 0, 0 };
    UInt32 misses[ KindCount ] = { 0, 0, 0 };
    UInt32 grazed[ KindCount ] = { 0, 0, 0 };
    Float64 worst[ KindCount ] = { 0.0, 0.0, 0.0 };
    UInt32 bad = 0;
    for ( UInt32 i=0; i<count; ++i )
    {
        const UInt32 kind = i % KindCount;
        /* every draw into its own local */
        const CTPoint3D< TFixed > movingCentre = random.Point( 3 * 65536 );
        const CTQuaternion< TFixed > movingRotation = random.Rotation();
        const TFixed hx = random.Uniform( 3277, 39322 );
        const TFixed hy = random.Uniform( 3277, 39322 );
        const TFixed hz = random.Uniform( 3277, 39322 );
        const CTPoint3D< TFixed > targetCentre = random.Point( 2 * 65536 );
        const CTQuaternion< TFixed > targetRotation = random.Rotation();
        const TFixed tx = random.Uniform( 6554, 39322 );
        const TFixed ty = random.Uniform( 6554, 39322 );
        const TFixed tz = random.Uniform( 6554, 39322 );
        const CTVector3D< TFixed > aim = random.Vector( 98304 );
        CTPoint3D< TFixed > corners[ 8 ];
        for ( UInt32 k=0; k<8; ++k )
            corners[ k ] = targetCentre + random.Vector( 26214 );
        /* aimed at the target with an offset: about half hit */
        const CTVector3D< TFixed > translation = ( targetCentre - movingCentre ) + ( targetCentre - movingCentre ) / TFixed( 2 ) + aim;

        const TBox moving( PointFromFixed< T >( movingCentre ), RotationFromFixed< T >( movingRotation ), VectorFromFixed< T >( CTVector3D< TFixed >( hx, hy, hz ) ) );
        const CTTriangle3D< T > triangle( PointFromFixed< T >( corners[ 0 ] ), PointFromFixed< T >( corners[ 1 ] ), PointFromFixed< T >( corners[ 2 ] ) );
        const TBox box( PointFromFixed< T >( targetCentre ), RotationFromFixed< T >( targetRotation ), VectorFromFixed< T >( CTVector3D< TFixed >( tx, ty, tz ) ) );
        CTPoint3D< T > points[ 8 ];
        CTPoint3D< Float64 > widePoints[ 8 ];
        for ( UInt32 k=0; k<8; ++k )
        {
            points[ k ] = PointFromFixed< T >( corners[ k ] );
            widePoints[ k ] = WideP( points[ k ] );
        }
        const CTConvexHull3D< T > hull( points, 8 );
        const CTVector3D< T > r = VectorFromFixed< T >( translation );
        const V3 wideR = WideV( r );
        const CTOrientedBox3D< Float64 > wideMoving = WideBox( moving );

        SGjkCastResult< T > cast;
        Float64 reference = -1.0;
        Float64 closest = 0.0;
        Float64 depth = 0.0;
        if ( 0 == kind )
        {
            CTGjk< T >::Cast( CTSupportMapping< TBox >::Make( moving ), r, CTNumericTraits< T >::One(), CTSupportMapping< CTTriangle3D< T > >::Make( triangle ), CTGeometryTolerance< T >(), cast );
            reference = ReferenceCast( wideMoving, wideR, CTTriangle3D< Float64 >( WideP( triangle.a ), WideP( triangle.b ), WideP( triangle.c ) ), closest, depth );
        }
        else if ( 1 == kind )
        {
            CTGjk< T >::Cast( CTSupportMapping< TBox >::Make( moving ), r, CTNumericTraits< T >::One(), CTSupportMapping< TBox >::Make( box ), CTGeometryTolerance< T >(), cast );
            reference = ReferenceCast( wideMoving, wideR, WideBox( box ), closest, depth );
        }
        else
        {
            CTGjk< T >::Cast( CTSupportMapping< TBox >::Make( moving ), r, CTNumericTraits< T >::One(), CTSupportMapping< CTConvexHull3D< T > >::Make( hull ), CTGeometryTolerance< T >(), cast );
            reference = ReferenceCast( wideMoving, wideR, CTConvexHull3D< Float64 >( widePoints, 8 ), closest, depth );
        }
        raw.Add( cast.hit );
        HashValue( raw, cast.fraction );
        HashVector( raw, cast.normal );
        HashVector( raw, cast.pointOnA );
        HashVector( raw, cast.pointOnB );
        raw.Add( cast.iterations );

        const Float64 length = wideR.GetLength();
        const bool clearHit = reference >= 0.0 && depth > grazing;
        const bool clearMiss = reference < 0.0 && closest > grazing;
        bool ok = true;
        if ( clearHit )
        {
            ++hits[ kind ];
            const Float64 error = fabs( ToWide( cast.fraction ) - reference ) * length;
            worst[ kind ] = error > worst[ kind ] ? error : worst[ kind ];
            ok = cast.hit && 0 == ( cast.status & MATHSTATUS_NOT_CONVERGED ) && error <= tolerance;
            /* a hit after the start: the normal is a unit vector against the motion */
            if ( ok && reference > 0.0 )
                ok = fabs( WideV( cast.normal ).GetLength() - 1.0 ) <= 1.0e-3 && !( WideV( cast.normal ).Dot( wideR ) > 0.0 );
        }
        else if ( clearMiss )
        {
            ++misses[ kind ];
            ok = !cast.hit;
        }
        else
        {
            ++grazed[ kind ];
        }
        if ( !ok )
        {
            ++bad;
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Epa: zero-margin cast " << kindNames[ kind ] << " pair " << i << ": hit " << ( cast.hit ? 1 : 0 ) << " fraction " << ToWide( cast.fraction ) << " status " << cast.status
                             << " iterations " << cast.iterations << ", reference " << reference << " closest " << closest << " depth " << depth << ", seed " << seed );
        }
    }
    hash = raw.GetHash();
    for ( UInt32 k=0; k<(UInt32) KindCount; ++k )
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Epa: zero-margin cast (kind size " << (UInt32) sizeof( T ) << ") " << kindNames[ k ] << ": " << hits[ k ] << " hits, " << misses[ k ] << " misses, " << grazed[ k ]
                   << " grazing, worst hit distance error " << worst[ k ] );
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Epa: zero-margin casts seed " << seed << ": hash " << CORE::ToString( hash ) << ", " << bad << " bad" );
    bool enough = true;
    for ( UInt32 k=0; k<(UInt32) KindCount; ++k )
        enough = enough && hits[ k ] * KindCount * 5 > count && misses[ k ] * KindCount * 5 > count;
    return 0 == bad && enough;
}

/*-------------------------------------------------------------------------*/

/**
 *  CTRayCaster::ShapeCast of an oriented box (no margin) over a scene of oriented boxes against the brute-force
 *  reference: the first contact over all boxes. Casts with a grazing box (contact or miss within 'grazing') are only
 *  counted.
 */
template < typename T >
static bool
RayCasterBoxCastSweep( UInt64 seed, UInt32 count, const Float64 tolerance, const Float64 grazing )
{
    typedef TFixed32_32 TFixed;
    typedef CTOrientedBox3D< T > TBox;
    enum { ObjectCount = 12 };
    CFixedRandom random( seed );
    GUCEF::vector< CTGeoObject< TBox >* > objects;
    GUCEF::vector< CTOrientedBox3D< Float64 > > wideObjects;
    CTRayCaster< T > caster;
    for ( UInt32 k=0; k<(UInt32) ObjectCount; ++k )
    {
        const CTPoint3D< TFixed > centre = random.Point( 4 * 65536 );
        const CTQuaternion< TFixed > rotation = random.Rotation();
        const TFixed hx = random.Uniform( 13107, 52429 );
        const TFixed hy = random.Uniform( 13107, 52429 );
        const TFixed hz = random.Uniform( 13107, 52429 );
        const TBox box( PointFromFixed< T >( centre ), RotationFromFixed< T >( rotation ), VectorFromFixed< T >( CTVector3D< TFixed >( hx, hy, hz ) ) );
        objects.push_back( new CTGeoObject< TBox >( box ) );
        wideObjects.push_back( WideBox( box ) );
        caster.AddObject( *objects.back() );
    }
    caster.Update();
    UInt32 hits = 0;
    UInt32 misses = 0;
    UInt32 grazed = 0;
    UInt32 bad = 0;
    Float64 worst = 0.0;
    for ( UInt32 i=0; i<count; ++i )
    {
        const CTPoint3D< TFixed > startCentre = random.Point( 6 * 65536 );
        const CTQuaternion< TFixed > startRotation = random.Rotation();
        const TFixed hx = random.Uniform( 6554, 39322 );
        const TFixed hy = random.Uniform( 6554, 39322 );
        const TFixed hz = random.Uniform( 6554, 39322 );
        const UInt32 target = random.Index( ObjectCount );
        const CTVector3D< TFixed > aim = random.Vector( 4 * 65536 );
        const CTVector3D< T > start = VectorFromFixed< T >( startCentre.ToPositionVector() );
        const CTQuaternion< T > rotation = RotationFromFixed< T >( startRotation );
        const CTVector3D< T > half = VectorFromFixed< T >( CTVector3D< TFixed >( hx, hy, hz ) );
        const CTVector3D< T > translation = ( ( objects[ target ]->GetShape().centre.ToPositionVector() - start ) + VectorFromFixed< T >( aim ) ) * ( T( 3 ) / T( 2 ) );
        const TBox localBox( CTPoint3D< T >(), CTQuaternion< T >(), half );
        const MATH::CTTransform3D< T > placement( start, rotation, CTVector3D< T >( T( 1 ), T( 1 ), T( 1 ) ) );
        SGeoRayHit< T > hit;
        const bool gotHit = caster.ShapeCast( localBox, placement, translation, 0xFFFFFFFF, hit );

        const CTOrientedBox3D< Float64 > wideMoving( P3( WideV( start ) ), WideBox( TBox( CTPoint3D< T >(), rotation, half ) ).rotation, WideV( half ) );
        const V3 wideTranslation = WideV( translation );
        bool graze = false;
        Float64 first = -1.0;
        for ( UInt32 k=0; k<(UInt32) ObjectCount; ++k )
        {
            Float64 closest = 0.0;
            Float64 depth = 0.0;
            const Float64 fraction = ReferenceCast( wideMoving, wideTranslation, wideObjects[ k ], closest, depth );
            if ( fraction >= 0.0 && depth > grazing )
                first = first < 0.0 || fraction < first ? fraction : first;
            else if ( fraction >= 0.0 || closest <= grazing )
                graze = true;
        }
        if ( graze )
        {
            ++grazed;
            continue;
        }
        const Float64 expected = first * wideTranslation.GetLength();
        bool ok = gotHit == ( first >= 0.0 );
        if ( first >= 0.0 )
        {
            ++hits;
            const Float64 error = fabs( ToWide( hit.distance ) - expected );
            worst = error > worst ? error : worst;
            ok = ok && error <= tolerance;
        }
        else
        {
            ++misses;
        }
        if ( !ok )
        {
            ++bad;
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Epa: ray caster box cast " << i << ": hit " << ( gotHit ? 1 : 0 ) << " at " << ToWide( hit.distance ) << ", reference " << ( first >= 0.0 ? 1 : 0 ) << " at " << expected << ", seed " << seed );
        }
    }
    for ( size_t k=0; k<objects.size(); ++k )
        delete objects[ k ];
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Epa: ray caster box casts (kind size " << (UInt32) sizeof( T ) << "), seed " << seed << ": " << hits << " hits, " << misses << " misses, " << grazed << " grazing, worst distance error "
               << worst << ", " << bad << " bad" );
    return 0 == bad && hits * 5 > count && misses * 5 > count;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformEpaTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "Epa" );

    GUCEF_TESTFW_TESTCASE( "Epa: overlapping spheres: depth r1 + r2 - d within 1e-4 and the normal along the centres, contact query and EPA on curved supports" )
    try
    {
        ASSERT_TRUE( SphereDepthSweep< Float64 >( 601, ScaledCount( 10000 ) ) );
        ASSERT_TRUE( SphereDepthSweep< Float32 >( 602, ScaledCount( 10000 ) ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Epa: overlapping boxes: depth equal to the SAT minimum overlap within 1e-4, the normal a direction of that overlap, witness points" )
    try
    {
        ASSERT_TRUE( BoxDepthSweep< Float64 >( 611, ScaledCount( 10000 ) ) );
        ASSERT_TRUE( BoxDepthSweep< Float32 >( 612, ScaledCount( 10000 ) ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Epa: the contact query of convex pairs: the depth is the minimum separating translation over 200 directions (Float64)" )
    try
    {
        ASSERT_TRUE( ( ContactSweep< CTCapsule< Float64 >, CTOrientedBox3D< Float64 > >( "capsule-OBB", 621, ScaledCount( 2000 ) ) ) );
        ASSERT_TRUE( ( ContactSweep< CTCylinder< Float64 >, CTOrientedBox3D< Float64 > >( "cylinder-OBB", 622, ScaledCount( 2000 ) ) ) );
        ASSERT_TRUE( ( ContactSweep< CTCapsule< Float64 >, CTCapsule< Float64 > >( "capsule-capsule", 623, ScaledCount( 2000 ) ) ) );
        ASSERT_TRUE( ( ContactSweep< CTSphere< Float64 >, CTCone< Float64 > >( "sphere-cone", 624, ScaledCount( 2000 ) ) ) );
        ASSERT_TRUE( ( ContactSweep< CTOrientedBox3D< Float64 >, CTCone< Float64 > >( "OBB-cone", 625, ScaledCount( 2000 ) ) ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Epa: degenerate polytopes: touching, identical shapes, point cores, crossing segment cores, coplanar flat shapes, 10^4 from the origin" )
    try
    {
        ASSERT_TRUE( DegenerateEpaCases< Float64 >( CTVector3D< Float64 >(), 1.0e-6 ) );
        ASSERT_TRUE( DegenerateEpaCases< Float64 >( CTVector3D< Float64 >( 1.0e4, -1.0e4, 1.0e4 ), 1.0e-5 ) );
        ASSERT_TRUE( DegenerateEpaCases< Float32 >( CTVector3D< Float32 >(), 1.0e-4 ) );
        ASSERT_TRUE( DegenerateEpaCases< TFixed32_32 >( CTVector3D< TFixed32_32 >(), 1.0e-6 ) );
        ASSERT_TRUE( DegenerateEpaCases< TFixed32_32 >( CTVector3D< TFixed32_32 >( TFixed32_32( 10000 ), TFixed32_32( -10000 ), TFixed32_32( 10000 ) ), 1.0e-6 ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Epa: the caps are reported as NOT_CONVERGED and never exceeded" )
    try
    {
        /* a curved shape with a zero tolerance can only stop at a cap */
        const CTSphere< Float64 > a( CTPoint3D< Float64 >( 0.0, 0.0, 0.0 ), 1.0 );
        const CTSphere< Float64 > b( CTPoint3D< Float64 >( 1.0, 0.3, 0.0 ), 1.0 );
        bool coresOverlap = false;
        const SEpaResult< Float64 > epa = EpaOf< Float64 >( CTShapeSupport< CTSphere< Float64 > >( a ), CTShapeSupport< CTSphere< Float64 > >( b ), CTGeometryTolerance< Float64 >( 0.0, 0.0, 0.0 ), coresOverlap );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Epa: zero tolerance on spheres: status " << epa.status << " after " << epa.iterations << " iterations, depth " << epa.depth );
        ASSERT_TRUE( 0 != ( epa.status & MATHSTATUS_NOT_CONVERGED ) );
        ASSERT_TRUE( epa.iterations <= (UInt32) CTEpa< Float64 >::MaxIterations );
        ASSERT_TRUE( fabs( epa.depth - ( 2.0 - sqrt( 1.09 ) ) ) < 1.0e-6 );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Epa: GJK, EPA, SAT, the contact query, casts and time of impact do not allocate" )
    GUCEF_TESTFW_SKIP_IF( !GeometryAllocationCounter::IsAvailable(), "no allocation counter with a sanitizer that replaces the allocator (the RealtimeSanitizer build proves the same in a realtime context)" );
    try
    {
        GeometryAllocationCounter::StartCounting();
        GeometryAllocationCounter::AllocateProbe();
        ASSERT_TRUE( 1 == GeometryAllocationCounter::StopCounting() );
        UInt32 accumulator = 0;
        CallConvexRealtimeQueries< Float32 >( accumulator );
        CallConvexRealtimeQueries< Float64 >( accumulator );
        CallConvexRealtimeQueries< TFixed32_32 >( accumulator );
        GeometryAllocationCounter::StartCounting();
        CallConvexRealtimeQueries< Float32 >( accumulator );
        CallConvexRealtimeQueries< Float64 >( accumulator );
        CallConvexRealtimeQueries< TFixed32_32 >( accumulator );
        const UInt32 allocations = GeometryAllocationCounter::StopCounting();
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Epa: " << allocations << " allocations in GJK, EPA, SAT, casts and time of impact (" << accumulator << ")" );
        ASSERT_TRUE( 0 == allocations );
        ASSERT_TRUE( accumulator > 20 );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Epa: TFixed32_32 EPA in a workspace at sizes 10 to 1000 and offsets 10^3 to 10^5: depth and witness points against Float64, same bits as the stack form" )
    try
    {
        ASSERT_TRUE( FixedScaleEpaSweep( 631 ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Epa: the contact query reports NOT_CONVERGED and DEGENERATE in its result and in manifold.status, also with a workspace" )
    try
    {
        ASSERT_TRUE( ContactStatusCases() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Epa: EPA on box-box (axis aligned, rotated, edge-edge minimum), sphere-sphere, sphere-box and capsule-box equals the exact depth within its termination band, TFixed32_32 matches the pinned hash, GJK hands EPA a fixed-point overlap it once missed" )
    try
    {
        UInt64 hash = 0;
        ASSERT_TRUE( ExactDepthAgreement< Float32 >( 641, ScaledCount( 5000 ), hash ) );
        ASSERT_TRUE( ExactDepthAgreement< Float64 >( 641, ScaledCount( 5000 ), hash ) );
        /* a fixed count: the hash is the same in the debug builds */
        ASSERT_TRUE( ExactDepthAgreement< TFixed32_32 >( 641, 2000, hash ) );
        ASSERT_TRUE( PinnedExactDepthHash == hash );
        ASSERT_TRUE( FixedGjkNoProgressOverlap() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Epa: GJK casts of a box without margin against a triangle, a box and a hull agree with a brute-force reference (Float32, Float64, TFixed32_32), TFixed32_32 matches the pinned hash, CTRayCaster::ShapeCast of a box likewise" )
    try
    {
        UInt64 hash = 0;
        ASSERT_TRUE( ZeroMarginCastSweep< Float64 >( 651, ScaledCount( 1500 ), 1.0e-6, 1.0e-5, hash ) );
        ASSERT_TRUE( ZeroMarginCastSweep< Float32 >( 651, ScaledCount( 1500 ), 1.0e-3, 1.0e-3, hash ) );
        /* a fixed count: the hash is the same in the debug builds */
        ASSERT_TRUE( ZeroMarginCastSweep< TFixed32_32 >( 651, 600, 1.0e-4, 1.0e-4, hash ) );
        ASSERT_TRUE( PinnedZeroMarginCastHash == hash );
        ASSERT_TRUE( RayCasterBoxCastSweep< Float64 >( 661, ScaledCount( 300 ), 1.0e-6, 1.0e-5 ) );
        ASSERT_TRUE( RayCasterBoxCastSweep< Float32 >( 661, ScaledCount( 300 ), 2.0e-3, 1.0e-3 ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Epa: TFixed32_32 contacts (capsule-capsule, sphere-box, cylinder-hull) agree with Float64 and match the pinned hash" )
    try
    {
        UInt64 hash = 0;
        ASSERT_TRUE( FixedPointContactRun( hash ) );
        ASSERT_TRUE( PinnedFixedEpaHash == hash );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
