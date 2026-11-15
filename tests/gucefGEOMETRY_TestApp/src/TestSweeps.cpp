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

#include "TestConvexReference.h"
#include "TestSweeps.h"

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

/** The pinned FNV-1a hash of the TFixed32_32 cast run: identical in every build (gcc, clang, ASan, GUCEF_MATH_INLINE_ALL) */
static const UInt64 PinnedFixedSweepHash = 10499446873942833977ULL;

/*-------------------------------------------------------------------------*/

/** The distance from the solid shape to a point (the plane: the distance to its surface), Float64 */
template < typename TShape >
static Float64
SurfaceGap( const TShape& shape, const P3& p )
{
    return shape.GetDistance( p );
}

/*-------------------------------------------------------------------------*/

/**
 *  The reference sphere cast: the first t in [0, maxDistance] at which the distance from the moving centre to the shape
 *  falls to the radius, by stepping and bisection on the shapes' own distance members. 'decided' is false for grazing
 *  rays (the centre passes within 1e-6 of the grown surface without entering it clearly).
 */
template < typename TShape >
static bool
ReferenceSphereCast( const TShape& shape, const P3& origin, const V3& direction, Float64 radius, Float64 maxDistance, Float64& t, bool& decided )
{
    decided = true;
    const UInt32 steps = 4000;
    Float64 previous = 0.0;
    Float64 minimum = 1.0e300;
    for ( UInt32 i=0; i<=steps; ++i )
    {
        const Float64 s = maxDistance * (Float64) i / (Float64) steps;
        const Float64 f = SurfaceGap( shape, origin + direction * s ) - radius;
        minimum = f < minimum ? f : minimum;
        if ( f <= 0.0 )
        {
            if ( 0 == i )
            {
                t = 0.0;
                return true;
            }
            Float64 lo = previous;
            Float64 hi = s;
            for ( UInt32 k=0; k<80; ++k )
            {
                const Float64 mid = ( lo + hi ) * 0.5;
                if ( SurfaceGap( shape, origin + direction * mid ) - radius <= 0.0 )
                    hi = mid;
                else
                    lo = mid;
            }
            t = hi;
            /* grazing: barely inside after the hit */
            const Float64 after = SurfaceGap( shape, origin + direction * ( t + 1.0e-3 ) ) - radius;
            decided = after < -1.0e-7 || t + 1.0e-3 > maxDistance;
            return true;
        }
        previous = s;
    }
    decided = minimum > 1.0e-4;
    return false;
}

/*-------------------------------------------------------------------------*/

/** The GJK sphere cast of a bounded shape (ShapeCast of a sphere) */
template < typename T, typename TShape >
static bool
GjkSphereCast( const CTRay3D< T >& ray, const T& radius, const TShape& shape, const T& maxDistance, SRayCastResult< T >& hit )
{
    return ShapeCast( CTSphere< T >( ray.origin, radius ), ray.direction, maxDistance, shape, hit );
}

/** A plane is unbounded and has no support mapping: the analytic cast stands in */
template < typename T >
static bool
GjkSphereCast( const CTRay3D< T >& ray, const T& radius, const CTPlane< T >& plane, const T& maxDistance, SRayCastResult< T >& hit )
{
    return SphereCast( ray, radius, plane, maxDistance, hit );
}

/*-------------------------------------------------------------------------*/

/** Sphere casts against a shape: the analytic cast against the bisection reference and against the GJK cast */
template < typename T, typename TShape >
static bool
SphereCastSweep( const char* name, UInt64 seed, UInt32 count, Float64 tolerance )
{
    CRandom random( seed );
    UInt32 bad = 0;
    UInt32 hits = 0;
    UInt32 misses = 0;
    UInt32 inside = 0;
    Float64 worst = 0.0;
    for ( UInt32 i=0; i<count; ++i )
    {
        TShape shape;
        RandomShape( random, shape );
        const P3 origin = random.Point( 4.0 );
        const P3 target = random.Point( 1.5 );
        const Float64 radius = random.Uniform( 0.05, 0.6 );
        D3 direction;
        if ( !D3::TryCreate( target - origin, direction ) )
            continue;
        const typename SWidened< TShape >::Type wide = Widen( shape );
        const CTRay3D< T > ray( ToP< T >( origin ), ToD< T >( direction ) );
        const P3 rayOrigin = Widen( ray.origin );
        const V3 rayDirection = WidenD( ray.direction ).AsVector();
        Float64 expected = 0.0;
        bool decided = true;
        const bool expectHit = ReferenceSphereCast( wide, rayOrigin, rayDirection, (Float64) (T) radius, 8.0, expected, decided );
        if ( !decided )
            continue;
        SRayCastResult< T > hit;
        const bool got = SphereCast( ray, (T) radius, shape, T( 8 ), hit );
        SRayCastResult< T > gjkHit;
        const bool gotGjk = GjkSphereCast( ray, (T) radius, shape, T( 8 ), gjkHit );
        bool ok = got == expectHit && gotGjk == expectHit;
        if ( ok && got )
        {
            if ( 0.0 == expected )
            {
                ++inside;
                ok = hit.StartedInside() && gjkHit.StartedInside() && (Float64) hit.distance == 0.0;
            }
            else
            {
                ++hits;
                const Float64 error = fabs( (Float64) hit.distance - expected );
                worst = error > worst ? error : worst;
                const P3 centre = rayOrigin + rayDirection * (Float64) hit.distance;
                const P3 contact = Widen( hit.point );
                const V3 normal = Widen( hit.normal );
                ok = !hit.StartedInside() && error <= tolerance * ( 1.0 + expected ) && fabs( (Float64) gjkHit.distance - expected ) <= tolerance * 10.0 * ( 1.0 + expected );
                ok = ok && fabs( normal.GetLength() - 1.0 ) < tolerance * 10.0 && SurfaceGap( wide, contact ) <= tolerance * 10.0;
                ok = ok && ( centre - ( contact + normal * (Float64) (T) radius ) ).GetLength() <= tolerance * 10.0;
            }
        }
        else if ( ok )
        {
            ++misses;
        }
        if ( !ok )
        {
            ++bad;
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Sweeps: sphere cast " << name << " got " << got << " / GJK " << gotGjk << " at " << (Float64) hit.distance << " / " << (Float64) gjkHit.distance
                             << ", reference " << expectHit << " at " << expected << ", seed " << seed << " ray " << i );
        }
    }
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Sweeps: sphere cast " << name << " seed " << seed << ": " << hits << " hits, " << inside << " started inside, " << misses << " misses, worst error "
               << worst << ", " << bad << " bad" );
    return 0 == bad && hits > count / 20 && misses > count / 20;
}

/*-------------------------------------------------------------------------*/

/** The GJK cast of a box against a large axis aligned box face: the analytic time of the leading corner reaching the face */
template < typename T >
static bool
BoxAgainstFaceSweep( UInt64 seed, UInt32 count, Float64 tolerance, Float64 earlyAllowance )
{
    CRandom random( seed );
    UInt32 bad = 0;
    Float64 worstEarly = 0.0;
    for ( UInt32 i=0; i<count; ++i )
    {
        CTOrientedBox3D< T > moving;
        RandomShape( random, moving );
        const Float64 gap = random.Uniform( 0.5, 2.0 );
        const D3 tilt = random.Direction();
        const V3 raw( 1.0, tilt.GetY() * 0.6, tilt.GetZ() * 0.6 );
        D3 direction;
        D3::TryCreate( raw, direction );
        P3 corners[ 8 ];
        Widen( moving ).GetCorners( corners );
        Float64 leading = -1.0e300;
        for ( UInt32 k=0; k<8; ++k )
            leading = corners[ k ].x > leading ? corners[ k ].x : leading;
        const Float64 wall = leading + gap;
        const CTAxisAlignedBox3D< T > target( ToP< T >( P3( wall, -10.0, -10.0 ) ), ToP< T >( P3( wall + 1.0, 10.0, 10.0 ) ) );
        const Float64 expected = ( (Float64) (T) wall - leading ) / WidenD( ToD< T >( direction ) ).GetX();
        SRayCastResult< T > hit;
        const bool got = ShapeCast( moving, ToD< T >( direction ), T( 50 ), target, hit );
        SRayCastResult< T > hitAabb;
        const bool gotAabb = ShapeCast( moving.GetBounds(), ToD< T >( direction ), T( 50 ), target, hitAabb );
        /*
         *  Float64 meets the analytic time; Float32 stops a little early where its rounding stalls the cast (by up to
         *  about 1e-3 of the travel, never after the contact): 'earlyAllowance' admits that conservative error
         */
        const Float64 error = (Float64) hit.distance - expected;
        const bool timeOk = fabs( error ) <= tolerance * ( 1.0 + expected ) || ( error <= tolerance && -error <= earlyAllowance * ( 1.0 + expected ) );
        worstEarly = -error > worstEarly ? -error : worstEarly;
        bool ok = got && timeOk && fabs( (Float64) hit.normal.x + 1.0 ) < tolerance * 10.0;
        ok = ok && gotAabb && fabs( (Float64) hit.point.x - (Float64) (T) wall ) <= ( tolerance + earlyAllowance ) * 10.0;
        if ( !ok )
        {
            ++bad;
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Sweeps: box cast " << (Float64) hit.distance << " expected " << expected << ", seed " << seed << " case " << i );
        }
    }
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Sweeps: box against a face seed " << seed << ": " << count << " casts, earliest stop " << worstEarly << " before the contact, " << bad << " bad" );
    return 0 == bad;
}

/*-------------------------------------------------------------------------*/

/** Casts that miss, start inside, or are not allowed */
template < typename T >
static bool
EdgeCases( void )
{
    typedef CTPoint3D< T > P;
    typedef CTVector3D< T > V;
    typedef CTDirectionVector3D< T > D;
    const CTSphere< T > sphere( P( T( 5 ), T( 0 ), T( 0 ) ), T( 1 ) );
    const CTAxisAlignedBox3D< T > box( P( T( 4 ), T( -1 ), T( -1 ) ), P( T( 6 ), T( 1 ), T( 1 ) ) );
    const CTRay3D< T > towards( P(), D( V( T( 1 ), T( 0 ), T( 0 ) ) ) );
    const CTRay3D< T > away( P(), D( V( T( -1 ), T( 0 ), T( 0 ) ) ) );
    SRayCastResult< T > hit;
    bool ok = SphereCast( towards, T( 1 ), sphere, T( 10 ), hit ) && CTNumericTraits< T >::ToFloat64( hit.distance ) > 2.99 && CTNumericTraits< T >::ToFloat64( hit.distance ) < 3.01;
    ok = ok && !SphereCast( away, T( 1 ), sphere, T( 10 ), hit ) && !SphereCast( towards, T( 1 ), sphere, T( 2 ), hit ) && !SphereCast( towards, T( -1 ), sphere, T( 10 ), hit );
    ok = ok && !SphereCast( away, T( 1 ), box, T( 10 ), hit ) && SphereCast( towards, T( 1 ), box, T( 10 ), hit ) && CTNumericTraits< T >::ToFloat64( hit.distance ) > 2.99 && CTNumericTraits< T >::ToFloat64( hit.distance ) < 3.01;
    ok = ok && SphereCast( CTRay3D< T >( P( T( 5 ), T( 0 ), T( 0 ) ), towards.direction ), T( 1 ), box, T( 10 ), hit ) && hit.StartedInside();
    ok = ok && ShapeCast( sphere, away.direction, T( 10 ), box, hit ) && hit.StartedInside() && T( 0 ) == hit.distance;
    ok = ok && !ShapeCast( CTSphere< T >( P(), T( 1 ) ), away.direction, T( 10 ), box, hit );
    const CTAxisAlignedBox3D< T > empty;
    ok = ok && !ShapeCast( CTSphere< T >( P(), T( 1 ) ), towards.direction, T( 10 ), empty, hit ) && !SphereCast( towards, T( 1 ), empty, T( 10 ), hit );
    if ( !ok )
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Sweeps: edge cases failed for a kind of " << (UInt32) sizeof( T ) << " bytes" );
    return ok;
}

/*-------------------------------------------------------------------------*/

/** The TFixed32_32 run: sphere casts (analytic AABB, GJK OBB) and box casts on fixed-point shapes from integer draws, against Float64, hashed */
static bool
FixedPointCastRun( UInt64& hash )
{
    typedef TFixed32_32 TFixed;
    typedef CTPoint3D< TFixed > P;
    typedef CTVector3D< TFixed > V;
    CFixedRandom random( 9004 );
    CRawHash raw;
    UInt32 bad = 0;
    UInt32 hits = 0;
    Float64 worst = 0.0;
    for ( UInt32 i=0; i<2000; ++i )
    {
        const P origin = random.Point( 4 * 65536 );
        const P centre = random.Point( 65536 );
        const TFixed halfX = random.Uniform( 13107, 65536 );
        const TFixed halfY = random.Uniform( 13107, 65536 );
        const TFixed halfZ = random.Uniform( 13107, 65536 );
        const V half( halfX, halfY, halfZ );
        const TFixed radius = random.Uniform( 6553, 39321 );
        const CTQuaternion< TFixed > rotation = random.Rotation();
        CTDirectionVector3D< TFixed > direction;
        if ( !CTDirectionVector3D< TFixed >::TryCreate( ( centre + random.Vector( 32768 ) ) - origin, direction ) )
            continue;
        const CTRay3D< TFixed > ray( origin, direction );
        const CTRay3D< Float64 > wideRay( FixedToP3( origin ), D3::FromNormalized( FixedToP3( P( direction.AsVector() ) ).ToPositionVector() ) );
        const CTOrientedBox3D< TFixed > obb( centre, rotation, half );
        const CTOrientedBox3D< Float64 > wideObb( FixedToP3( centre ), Q4::FromXYZW( FixedToFloat64( rotation.x ), FixedToFloat64( rotation.y ), FixedToFloat64( rotation.z ), FixedToFloat64( rotation.w ) ),
                                                  V3( FixedToFloat64( half.x ), FixedToFloat64( half.y ), FixedToFloat64( half.z ) ) );
        const CTAxisAlignedBox3D< TFixed > aabb( centre - half, centre + half );
        const CTAxisAlignedBox3D< Float64 > wideAabb( FixedToP3( centre - half ), FixedToP3( centre + half ) );
        SRayCastResult< TFixed > results[ 3 ];
        SRayCastResult< Float64 > references[ 3 ];
        const bool got[ 3 ] = { SphereCast( ray, radius, aabb, TFixed( 10 ), results[ 0 ] ), SphereCast( ray, radius, obb, TFixed( 10 ), results[ 1 ] ),
                                ShapeCast( CTOrientedBox3D< TFixed >( origin, rotation, half ), direction, TFixed( 10 ), aabb, results[ 2 ] ) };
        const bool expected[ 3 ] = { SphereCast( wideRay, FixedToFloat64( radius ), wideAabb, 10.0, references[ 0 ] ), SphereCast( wideRay, FixedToFloat64( radius ), wideObb, 10.0, references[ 1 ] ),
                                     ShapeCast( CTOrientedBox3D< Float64 >( FixedToP3( origin ), wideObb.rotation, wideObb.halfExtents ), wideRay.direction, 10.0, wideAabb, references[ 2 ] ) };
        for ( UInt32 k=0; k<3; ++k )
        {
            raw.Add( got[ k ] );
            raw.Add( results[ k ].distance );
            raw.Add3( results[ k ].point );
            raw.Add3( results[ k ].normal );
            hits += got[ k ] ? 1 : 0;
            /* like Float32, the box cast may stop slightly early where the resolution stalls it (never late) */
            const Float64 signedError = got[ k ] && expected[ k ] ? FixedToFloat64( results[ k ].distance ) - references[ k ].distance : 0.0;
            const Float64 error = fabs( signedError );
            worst = error > worst ? error : worst;
            const bool timeOk = error <= 1.0e-5 || ( 2 == k && signedError <= 1.0e-6 && error <= 1.0e-4 );
            if ( got[ k ] != expected[ k ] || !timeOk )
            {
                ++bad;
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Sweeps: fixed-point cast " << k << " got " << got[ k ] << " at " << FixedToFloat64( results[ k ].distance ) << ", Float64 "
                                 << expected[ k ] << " at " << references[ k ].distance << ", case " << i );
            }
        }
    }
    hash = raw.GetHash();
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Sweeps: TFixed32_32 run: " << raw.GetCount() << " raw values, hash " << CORE::ToString( hash ) << ", " << hits << " hits, worst difference to Float64 "
               << worst << ", " << bad << " bad" );
    return 0 == bad && hits > 500;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformSweepsTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "Sweeps" );

    GUCEF_TESTFW_TESTCASE( "Sweeps: sphere casts equal the analytic ray cast on the grown shape and the bisection reference, 4000 seeded rays per shape (Float64)" )
    try
    {
        ASSERT_TRUE( ( SphereCastSweep< Float64, CTPlane< Float64 > >( "plane", 801, ScaledCount( 4000 ), 1.0e-6 ) ) );
        ASSERT_TRUE( ( SphereCastSweep< Float64, CTSphere< Float64 > >( "sphere", 802, ScaledCount( 4000 ), 1.0e-6 ) ) );
        ASSERT_TRUE( ( SphereCastSweep< Float64, CTCapsule< Float64 > >( "capsule", 803, ScaledCount( 4000 ), 1.0e-6 ) ) );
        ASSERT_TRUE( ( SphereCastSweep< Float64, CTAxisAlignedBox3D< Float64 > >( "AABB", 804, ScaledCount( 4000 ), 1.0e-6 ) ) );
        ASSERT_TRUE( ( SphereCastSweep< Float64, CTTriangle3D< Float64 > >( "triangle", 805, ScaledCount( 4000 ), 1.0e-6 ) ) );
        ASSERT_TRUE( ( SphereCastSweep< Float64, CTOrientedBox3D< Float64 > >( "OBB", 806, ScaledCount( 4000 ), 1.0e-6 ) ) );
        ASSERT_TRUE( ( SphereCastSweep< Float64, CTCylinder< Float64 > >( "cylinder", 807, ScaledCount( 4000 ), 1.0e-6 ) ) );
        ASSERT_TRUE( ( SphereCastSweep< Float64, CTCone< Float64 > >( "cone", 808, ScaledCount( 4000 ), 1.0e-6 ) ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Sweeps: sphere casts in Float32 within 1e-4 of the reference" )
    try
    {
        ASSERT_TRUE( ( SphereCastSweep< Float32, CTSphere< Float32 > >( "sphere32", 812, ScaledCount( 2000 ), 1.0e-4 ) ) );
        ASSERT_TRUE( ( SphereCastSweep< Float32, CTAxisAlignedBox3D< Float32 > >( "AABB32", 814, ScaledCount( 2000 ), 1.0e-4 ) ) );
        ASSERT_TRUE( ( SphereCastSweep< Float32, CTTriangle3D< Float32 > >( "triangle32", 815, ScaledCount( 2000 ), 1.0e-4 ) ) );
        ASSERT_TRUE( ( SphereCastSweep< Float32, CTOrientedBox3D< Float32 > >( "OBB32", 816, ScaledCount( 2000 ), 1.0e-4 ) ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Sweeps: the GJK cast of a box against a plane-aligned box face equals the analytic time (Float64 1e-6; Float32 never late, at most 2e-3 early), 2000 seeded casts" )
    try
    {
        ASSERT_TRUE( BoxAgainstFaceSweep< Float64 >( 821, ScaledCount( 2000 ), 1.0e-6, 0.0 ) );
        ASSERT_TRUE( BoxAgainstFaceSweep< Float32 >( 822, ScaledCount( 2000 ), 1.0e-4, 2.0e-3 ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Sweeps: misses, casts that start inside, negative radii and shapes without points" )
    try
    {
        ASSERT_TRUE( EdgeCases< Float64 >() );
        ASSERT_TRUE( EdgeCases< Float32 >() );
        ASSERT_TRUE( EdgeCases< TFixed32_32 >() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Sweeps: TFixed32_32 sphere and box casts agree with Float64 (1e-5; box casts never late, at most 1e-4 early) and match the pinned hash" )
    try
    {
        UInt64 hash = 0;
        ASSERT_TRUE( FixedPointCastRun( hash ) );
        ASSERT_TRUE( PinnedFixedSweepHash == hash );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
