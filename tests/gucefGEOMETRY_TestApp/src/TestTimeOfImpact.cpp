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

#ifndef GUCEF_GEOMETRY_TIMEOFIMPACT_H
#include "gucefGEOMETRY_TimeOfImpact.h"
#define GUCEF_GEOMETRY_TIMEOFIMPACT_H
#endif /* GUCEF_GEOMETRY_TIMEOFIMPACT_H ? */

#include "TestConvexReference.h"
#include "TestTimeOfImpact.h"

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

/** The pinned FNV-1a hash of the TFixed32_32 time of impact run: identical in every build (gcc, clang, ASan, GUCEF_MATH_INLINE_ALL) */
static const UInt64 PinnedFixedToiHash = 9745398686575784660ULL;

/*-------------------------------------------------------------------------*/

/** The box of a body at time t: its local box placed by the motion */
template < typename T >
static CTOrientedBox3D< T >
PlacedBox( const CTOrientedBox3D< T >& local, const CTMotion3D< T >& motion, const T& t )
{
    const CTQuaternion< T > rotation = motion.GetRotation( t );
    return CTOrientedBox3D< T >( motion.GetPosition( t ) + rotation.Rotate( local.centre.ToPositionVector() ), ( rotation * local.rotation ).Normalized(), local.halfExtents );
}

/*-------------------------------------------------------------------------*/

/** One thin fast rotating box against a wall: the scene of acceptance item 5 */
template < typename T >
struct SThinBoxScene
{
    CTOrientedBox3D< T > box;
    CTMotion3D< T > motion;
    CTAxisAlignedBox3D< T > wall;
};

template < typename T >
static SThinBoxScene< T >
MakeThinBoxScene( CRandom& random )
{
    SThinBoxScene< T > scene;
    const Float64 length = random.Uniform( 0.3, 0.8 );
    const Float64 thickness = random.Uniform( 0.005, 0.03 );
    const Float64 y = random.Uniform( -1.0, 1.0 );
    const Float64 speed = random.Uniform( 5.0, 9.0 );
    const Float64 spin = random.Uniform( -20.0, 20.0 );
    const Float64 tumble = random.Uniform( -3.0, 3.0 );
    const Float64 startAngle = random.Uniform( 0.0, 6.283185307179586 );
    scene.box = CTOrientedBox3D< T >( CTPoint3D< T >(), CTQuaternion< T >(), ToV< T >( V3( length, thickness, 0.2 ) ) );
    scene.motion = CTMotion3D< T >( ToP< T >( P3( -speed * 0.5, y, 0.0 ) ), ToQ< T >( Q4::FromAxisAngle( D3::UnitZ(), CTRadian< Float64 >( startAngle ) ) ),
                                    ToV< T >( V3( speed, 0.0, 0.0 ) ), ToV< T >( V3( tumble, 0.0, spin ) ) );
    scene.wall = CTAxisAlignedBox3D< T >( ToP< T >( P3( 0.0, -5.0, -5.0 ) ), ToP< T >( P3( 0.05, 5.0, 5.0 ) ) );
    return scene;
}

/*-------------------------------------------------------------------------*/

/**
 *  Thin fast rotating boxes against a wall: apart at the start and at the end of the interval (a discrete step would
 *  tunnel), TimeOfImpact reports a hit no later than the first penetrating sample of a fine scan (1e-4 of the
 *  interval), and the shapes are apart by at most the target distance at the reported time.
 */
template < typename T >
static bool
ThinBoxSweep( UInt64 seed, UInt32 count, Float64 band )
{
    CRandom random( seed );
    const T target = T( 1 ) / T( 1000 );
    UInt32 bad = 0;
    UInt32 tunnelling = 0;
    UInt32 maxIterations = 0;
    for ( UInt32 i=0; i<count; ++i )
    {
        const SThinBoxScene< T > scene = MakeThinBoxScene< T >( random );
        const bool apartAtStart = !Intersects( PlacedBox( scene.box, scene.motion, T( 0 ) ), scene.wall );
        const bool apartAtEnd = !Intersects( PlacedBox( scene.box, scene.motion, T( 1 ) ), scene.wall );
        tunnelling += apartAtStart && apartAtEnd ? 1 : 0;
        Float64 firstPenetration = 2.0;
        for ( UInt32 k=1; k<=10000; ++k )
        {
            const T t = T( (Float64) k / 10000.0 );
            if ( Intersects( PlacedBox( scene.box, scene.motion, t ), scene.wall ) )
            {
                firstPenetration = (Float64) k / 10000.0;
                break;
            }
        }
        SToiResult< T > result;
        const bool hit = TimeOfImpact( scene.box, scene.motion, scene.wall, CTMotion3D< T >(), target, result );
        maxIterations = result.iterations > maxIterations ? result.iterations : maxIterations;
        const Float64 time = CTNumericTraits< T >::ToFloat64( result.time );
        /* the gap at the reported time measured in Float64 on the placed box of the kind */
        const CTOrientedBox3D< T > placed = PlacedBox( scene.box, scene.motion, result.time );
        const CTOrientedBox3D< Float64 > widePlaced( P3( CTNumericTraits< T >::ToFloat64( placed.centre.x ), CTNumericTraits< T >::ToFloat64( placed.centre.y ), CTNumericTraits< T >::ToFloat64( placed.centre.z ) ),
                                                     Q4::FromXYZW( CTNumericTraits< T >::ToFloat64( placed.rotation.x ), CTNumericTraits< T >::ToFloat64( placed.rotation.y ),
                                                                   CTNumericTraits< T >::ToFloat64( placed.rotation.z ), CTNumericTraits< T >::ToFloat64( placed.rotation.w ) ).Normalized(),
                                                     V3( CTNumericTraits< T >::ToFloat64( placed.halfExtents.x ), CTNumericTraits< T >::ToFloat64( placed.halfExtents.y ), CTNumericTraits< T >::ToFloat64( placed.halfExtents.z ) ) );
        const CTAxisAlignedBox3D< Float64 > wideWall( P3( CTNumericTraits< T >::ToFloat64( scene.wall.min.x ), CTNumericTraits< T >::ToFloat64( scene.wall.min.y ), CTNumericTraits< T >::ToFloat64( scene.wall.min.z ) ),
                                                      P3( CTNumericTraits< T >::ToFloat64( scene.wall.max.x ), CTNumericTraits< T >::ToFloat64( scene.wall.max.y ), CTNumericTraits< T >::ToFloat64( scene.wall.max.z ) ) );
        const Float64 gap = Distance( widePlaced, wideWall );
        bool ok = apartAtStart && result.iterations <= (UInt32) TIMEOFIMPACT_MAX_ITERATIONS;
        if ( firstPenetration <= 1.0 )
            ok = ok && hit && TOISTATE_HIT == result.state && time <= firstPenetration && gap > 0.0 && gap <= CTNumericTraits< T >::ToFloat64( target ) + band;
        else
            ok = ok && ( !hit || gap <= CTNumericTraits< T >::ToFloat64( target ) + band );
        if ( !ok )
        {
            ++bad;
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "TimeOfImpact: thin box " << i << " state " << (UInt32) result.state << " time " << time << " first penetration " << firstPenetration
                             << " gap " << gap << " iterations " << result.iterations << ", seed " << seed );
        }
    }
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "TimeOfImpact: thin boxes seed " << seed << ": " << count << " scenes, " << tunnelling << " would tunnel in one discrete step, at most "
               << maxIterations << " iterations, " << bad << " bad" );
    return 0 == bad && tunnelling > count / 2;
}

/*-------------------------------------------------------------------------*/

/** The time at which two linearly moving spheres are 'gap' apart (the first root), or 2 when never */
static Float64
SphereGapTime( const P3& ca, const V3& va, const P3& cb, const V3& vb, Float64 reach )
{
    const V3 d = cb - ca;
    const V3 v = vb - va;
    const Float64 a = v.Dot( v );
    const Float64 b = 2.0 * d.Dot( v );
    const Float64 c = d.Dot( d ) - reach * reach;
    if ( c <= 0.0 )
        return 0.0;
    const Float64 discriminant = b * b - 4.0 * a * c;
    if ( a <= 0.0 || discriminant < 0.0 )
        return 2.0;
    const Float64 t = ( -b - sqrt( discriminant ) ) / ( 2.0 * a );
    return t >= 0.0 ? t : 2.0;
}

/*-------------------------------------------------------------------------*/

/** Linearly moving spheres: the reported time lies between the analytic times of the target distance and of half of it */
template < typename T >
static bool
LinearSphereSweep( UInt64 seed, UInt32 count, Float64 band )
{
    CRandom random( seed );
    const Float64 target = 1.0e-3;
    UInt32 bad = 0;
    UInt32 hits = 0;
    UInt32 misses = 0;
    for ( UInt32 i=0; i<count; ++i )
    {
        CTSphere< T > a;
        CTSphere< T > b;
        RandomShape( random, a );
        RandomShape( random, b );
        const V3 va = random.Vector( 4.0 );
        const V3 vb = random.Vector( 4.0 );
        const CTSphere< Float64 > wa = Widen( a );
        const CTSphere< Float64 > wb = Widen( b );
        const V3 wva = Widen( ToV< T >( va ) );
        const V3 wvb = Widen( ToV< T >( vb ) );
        const Float64 reach = wa.radius + wb.radius;
        if ( ( wb.centre - wa.centre ).GetLength() <= reach + target )
            continue;
        const Float64 early = SphereGapTime( wa.centre, wva, wb.centre, wvb, reach + target );
        const Float64 late = SphereGapTime( wa.centre, wva, wb.centre, wvb, reach + target * 0.5 );
        /* the spheres in their own frames, placed by the motions */
        const CTSphere< T > localA( CTPoint3D< T >(), a.radius );
        const CTSphere< T > localB( CTPoint3D< T >(), b.radius );
        SToiResult< T > result;
        const bool hit = TimeOfImpact( localA, CTMotion3D< T >( a.centre, CTQuaternion< T >(), ToV< T >( va ), CTVector3D< T >() ), localB,
                                       CTMotion3D< T >( b.centre, CTQuaternion< T >(), ToV< T >( vb ), CTVector3D< T >() ), T( target ), result );
        const Float64 time = CTNumericTraits< T >::ToFloat64( result.time );
        bool ok = true;
        if ( early <= 1.0 - 1.0e-6 )
        {
            ++hits;
            ok = hit && TOISTATE_HIT == result.state && time >= early - band && time <= late + band;
        }
        else if ( early > 1.0 + 1.0e-6 )
        {
            ++misses;
            ok = !hit && TOISTATE_SEPARATED == result.state;
        }
        if ( !ok )
        {
            ++bad;
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "TimeOfImpact: spheres " << i << " state " << (UInt32) result.state << " time " << time << " expected in [" << early << ", " << late << "], seed " << seed );
        }
    }
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "TimeOfImpact: linear spheres seed " << seed << ": " << hits << " hits, " << misses << " misses, " << bad << " bad" );
    return 0 == bad && hits > count / 20 && misses > count / 20;
}

/*-------------------------------------------------------------------------*/

/** The rigid motion type and the start states of TimeOfImpact */
template < typename T >
static bool
MotionChecks( void )
{
    typedef CTPoint3D< T > P;
    typedef CTVector3D< T > V;
    CRandom random( 921 );
    bool ok = true;
    for ( UInt32 i=0; i<100 && ok; ++i )
    {
        const P p0 = ToP< T >( random.Point( 3.0 ) );
        const P p1 = ToP< T >( random.Point( 3.0 ) );
        const CTQuaternion< T > q0 = ToQ< T >( random.Rotation() );
        const CTQuaternion< T > q1 = ToQ< T >( random.Rotation() );
        const CTMotion3D< T > motion = CTMotion3D< T >::FromPlacements( p0, q0, p1, q1 );
        const CTQuaternion< T > end = motion.GetRotation( T( 1 ) );
        const Float64 alignment = fabs( CTNumericTraits< T >::ToFloat64( end.Dot( q1 ) ) );
        ok = motion.GetPosition( T( 0 ) ) == p0 && motion.GetRotation( T( 0 ) ) == q0 && ( motion.GetPosition( T( 1 ) ) - p1 ).GetLength() < T( 1 ) / T( 10000 );
        ok = ok && alignment > 1.0 - 1.0e-5;
        ok = ok && CTNumericTraits< T >::ToFloat64( motion.angularVelocity.GetLength() ) <= 3.1416;
    }
    const CTMotion3D< T > spin( P(), CTQuaternion< T >(), V( T( 2 ), T( 0 ), T( 0 ) ), V( T( 0 ), T( 0 ), T( 3 ) ) );
    ok = ok && fabs( CTNumericTraits< T >::ToFloat64( spin.GetMaxDisplacement( T( 2 ) ) ) - 8.0 ) < 1.0e-5 && !spin.IsAtRest() && CTMotion3D< T >().IsAtRest();
    CTMotion3D< T > parsed;
    ok = ok && parsed.FromString( spin.ToString() ) && parsed == spin && !parsed.FromString( "Motion3D(position (0, 0, 0))" ) && parsed == spin;
    CORE::CDataNode node;
    CTMotion3D< T > restored;
    ok = ok && spin.Serialize( node, CORE::CDataNodeSerializableSettings() ) && restored.Deserialize( node, CORE::CDataNodeSerializableSettings() ) && restored == spin;

    /* the states: overlapping at the start, moving apart, an invalid shape */
    const CTSphere< T > unit( P(), T( 1 ) );
    SToiResult< T > result;
    ok = ok && TimeOfImpact( unit, CTMotion3D< T >(), unit, CTMotion3D< T >::AtRest( P( T( 1 ), T( 0 ), T( 0 ) ), CTQuaternion< T >() ), T( 1 ) / T( 100 ), result ) && TOISTATE_OVERLAPPING == result.state;
    ok = ok && !TimeOfImpact( unit, CTMotion3D< T >( P(), CTQuaternion< T >(), V( T( -5 ), T( 0 ), T( 0 ) ), V() ), unit,
                              CTMotion3D< T >::AtRest( P( T( 3 ), T( 0 ), T( 0 ) ), CTQuaternion< T >() ), T( 1 ) / T( 100 ), result ) && TOISTATE_SEPARATED == result.state;
    ok = ok && !TimeOfImpact( CTSphere< T >( P(), T( -1 ) ), CTMotion3D< T >(), unit, CTMotion3D< T >(), T( 1 ) / T( 100 ), result );
    /*
     *  a fast spin of a sphere about its centre does not change it: its core radius is 0, so the spin does not slow the
     *  advancement; the hit comes in a few iterations just before t = 0.25
     */
    const CTOrientedBox3D< T > block( P( T( 3 ), T( 0 ), T( 0 ) ), CTQuaternion< T >(), V( T( 1 ), T( 1 ), T( 1 ) ) );
    ok = ok && TimeOfImpact( unit, CTMotion3D< T >( P(), CTQuaternion< T >(), V( T( 4 ), T( 0 ), T( 0 ) ), V( T( 0 ), T( 0 ), T( 1000 ) ) ), block, CTMotion3D< T >(), T( 1 ) / T( 1000 ), result );
    ok = ok && TOISTATE_HIT == result.state && result.iterations <= 10 && fabs( CTNumericTraits< T >::ToFloat64( result.time ) - 0.25 ) < 1.0e-3 && CTNumericTraits< T >::ToFloat64( result.time ) < 0.25;
    /* a target distance of 0 is reached through the minimum gap: a hit, not the cap */
    ok = ok && TimeOfImpact( unit, CTMotion3D< T >( P(), CTQuaternion< T >(), V( T( 4 ), T( 0 ), T( 0 ) ), V() ), block, CTMotion3D< T >(), T( 0 ), result );
    ok = ok && TOISTATE_HIT == result.state && result.iterations < 64 && fabs( CTNumericTraits< T >::ToFloat64( result.time ) - 0.25 ) < 1.0e-5;
    /*
     *  a long bar (core radius 100) spinning a thousand times faster than it approaches a far wall: the advancement
     *  stops at the cap; TimeOfImpact returns true (stop the motion at result.time), and that time is safe
     */
    const CTOrientedBox3D< T > bar( P(), CTQuaternion< T >(), V( T( 100 ), T( 1 ) / T( 10 ), T( 1 ) / T( 10 ) ) );
    const CTOrientedBox3D< T > wall( P( T( 300 ), T( 0 ), T( 0 ) ), CTQuaternion< T >(), V( T( 10 ), T( 200 ), T( 200 ) ) );
    const bool capped = TimeOfImpact( bar, CTMotion3D< T >( P(), CTQuaternion< T >(), V( T( 4 ), T( 0 ), T( 0 ) ), V( T( 0 ), T( 0 ), T( 1000 ) ) ), wall, CTMotion3D< T >(), T( 1 ) / T( 1000 ), result );
    ok = ok && capped && TOISTATE_NOT_CONVERGED == result.state && 0 != ( result.status & MATHSTATUS_NOT_CONVERGED ) && (UInt32) TIMEOFIMPACT_MAX_ITERATIONS == result.iterations;
    ok = ok && result.distance > T( 0 ) && CTNumericTraits< T >::ToFloat64( result.time ) > 0.0 && CTNumericTraits< T >::ToFloat64( result.time ) < 1.0;
    /* the core radii: the margin does not count, boxes and hulls use their furthest vertex */
    const CTOrientedBox3D< T > box123( P(), ToQ< T >( Q4::FromAxisAngle( D3::UnitZ(), CTRadian< Float64 >( 0.7 ) ) ), V( T( 1 ), T( 2 ), T( 3 ) ) );
    ok = ok && fabs( CTNumericTraits< T >::ToFloat64( GetSupportCoreRadius( CTSupportMapping< CTSphere< T > >::Make( CTSphere< T >( P( T( 3 ), T( 4 ), T( 0 ) ), T( 7 ) ) ) ) ) - 5.0 ) < 1.0e-4;
    ok = ok && fabs( CTNumericTraits< T >::ToFloat64( GetSupportCoreRadius( CTSupportMapping< CTCapsule< T > >::Make( CTCapsule< T >( P( T( 1 ), T( 0 ), T( 0 ) ), P( T( 0 ), T( 2 ), T( 0 ) ), T( 5 ) ) ) ) ) - 2.0 ) < 1.0e-4;
    ok = ok && fabs( CTNumericTraits< T >::ToFloat64( GetSupportCoreRadius( CTSupportMapping< CTOrientedBox3D< T > >::Make( box123 ) ) ) - sqrt( 14.0 ) ) < 1.0e-4;
    const P hullPoints[ 4 ] = { P( T( 1 ), T( 0 ), T( 0 ) ), P( T( 0 ), T( 3 ), T( 0 ) ), P( T( 0 ), T( 0 ), T( 2 ) ), P( T( -1 ), T( -1 ), T( -1 ) ) };
    ok = ok && fabs( CTNumericTraits< T >::ToFloat64( GetSupportCoreRadius( CTSupportMapping< CTConvexHull3D< T > >::Make( CTConvexHull3D< T >( hullPoints, 4 ) ) ) ) - 3.0 ) < 1.0e-4;
    if ( !ok )
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "TimeOfImpact: motion checks failed for a kind of " << (UInt32) sizeof( T ) << " bytes" );
    return ok;
}

/*-------------------------------------------------------------------------*/

/** The TFixed32_32 run: thin rotating boxes and spheres from integer draws, against Float64, hashed */
static bool
FixedPointToiRun( UInt64& hash )
{
    typedef TFixed32_32 TFixed;
    typedef CTPoint3D< TFixed > P;
    typedef CTVector3D< TFixed > V;
    CFixedRandom random( 9005 );
    CRawHash raw;
    UInt32 bad = 0;
    UInt32 hits = 0;
    Float64 worst = 0.0;
    const TFixed target = TFixed( 1 ) / TFixed( 1000 );
    for ( UInt32 i=0; i<500; ++i )
    {
        const TFixed length = random.Uniform( 19660, 52428 );
        const TFixed thickness = random.Uniform( 327, 1966 );
        const TFixed y = random.Uniform( -65536, 65536 );
        const TFixed speed = random.Uniform( 5 * 65536, 9 * 65536 );
        const TFixed spin = random.Uniform( -20 * 65536, 20 * 65536 );
        const CTQuaternion< TFixed > start = random.Rotation();
        const CTOrientedBox3D< TFixed > box( P(), CTQuaternion< TFixed >(), V( length, thickness, TFixed( 1 ) / TFixed( 5 ) ) );
        const CTMotion3D< TFixed > motion( P( -speed / TFixed( 2 ), y, TFixed( 0 ) ), start, V( speed, TFixed( 0 ), TFixed( 0 ) ), V( TFixed( 0 ), TFixed( 0 ), spin ) );
        const CTAxisAlignedBox3D< TFixed > wall( P( TFixed( 0 ), TFixed( -5 ), TFixed( -5 ) ), P( TFixed( 1 ) / TFixed( 20 ), TFixed( 5 ), TFixed( 5 ) ) );
        SToiResult< TFixed > result;
        const bool hit = TimeOfImpact( box, motion, wall, CTMotion3D< TFixed >(), target, result );

        const CTOrientedBox3D< Float64 > wideBox( P3(), Q4(), V3( FixedToFloat64( length ), FixedToFloat64( thickness ), FixedToFloat64( TFixed( 1 ) / TFixed( 5 ) ) ) );
        const CTMotion3D< Float64 > wideMotion( FixedToP3( motion.startPosition ), Q4::FromXYZW( FixedToFloat64( start.x ), FixedToFloat64( start.y ), FixedToFloat64( start.z ), FixedToFloat64( start.w ) ),
                                                V3( FixedToFloat64( speed ), 0.0, 0.0 ), V3( 0.0, 0.0, FixedToFloat64( spin ) ) );
        const CTAxisAlignedBox3D< Float64 > wideWall( FixedToP3( wall.min ), FixedToP3( wall.max ) );
        SToiResult< Float64 > reference;
        const bool referenceHit = TimeOfImpact( wideBox, wideMotion, wideWall, CTMotion3D< Float64 >(), FixedToFloat64( target ), reference );

        raw.Add( hit );
        raw.Add( (UInt32) result.state );
        raw.Add( result.time );
        raw.Add( result.distance );
        raw.Add3( result.normal );
        raw.Add3( result.pointOnA );
        raw.Add( result.iterations );
        hits += hit ? 1 : 0;
        /* conservative advancement converges to the target band, not to one time: the times agree to the band over the speed */
        const Float64 error = hit && referenceHit ? fabs( FixedToFloat64( result.time ) - reference.time ) : 0.0;
        worst = error > worst ? error : worst;
        if ( hit != referenceHit || error > 2.0e-4 )
        {
            ++bad;
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "TimeOfImpact: fixed-point time " << FixedToFloat64( result.time ) << " Float64 " << reference.time << " case " << i );
        }
    }
    hash = raw.GetHash();
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "TimeOfImpact: TFixed32_32 run: " << raw.GetCount() << " raw values, hash " << CORE::ToString( hash ) << ", " << hits << " hits, worst difference to Float64 "
               << worst << ", " << bad << " bad" );
    return 0 == bad && hits > 100;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformTimeOfImpactTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "TimeOfImpact" );

    GUCEF_TESTFW_TESTCASE( "TimeOfImpact: a fast thin box against a wall with rotation does not tunnel: the reported time precedes the first penetrating sample" )
    try
    {
        ASSERT_TRUE( ThinBoxSweep< Float64 >( 901, ScaledCount( 300 ), 1.0e-9 ) );
        ASSERT_TRUE( ThinBoxSweep< Float32 >( 902, ScaledCount( 300 ), 1.0e-5 ) );
        ASSERT_TRUE( ThinBoxSweep< TFixed32_32 >( 903, ScaledCount( 100 ), 1.0e-8 ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "TimeOfImpact: linearly moving spheres hit between the analytic times of the target distance and of half of it" )
    try
    {
        ASSERT_TRUE( LinearSphereSweep< Float64 >( 911, ScaledCount( 4000 ), 1.0e-9 ) );
        ASSERT_TRUE( LinearSphereSweep< Float32 >( 912, ScaledCount( 4000 ), 1.0e-5 ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "TimeOfImpact: the rigid motion type (placements, bound, text, DataNode), the states, core radii (spin of a ball is free), target 0, and the iteration cap returning true" )
    try
    {
        ASSERT_TRUE( MotionChecks< Float64 >() );
        ASSERT_TRUE( MotionChecks< Float32 >() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "TimeOfImpact: TFixed32_32 rotating thin boxes agree with Float64 and match the pinned hash" )
    try
    {
        UInt64 hash = 0;
        ASSERT_TRUE( FixedPointToiRun( hash ) );
        ASSERT_TRUE( PinnedFixedToiHash == hash );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
