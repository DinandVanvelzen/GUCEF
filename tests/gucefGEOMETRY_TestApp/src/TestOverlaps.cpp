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
#include "TestOverlaps.h"

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

/** The counts of an overlap sweep */
struct SOverlapSweep
{
    UInt32 pairs;
    UInt32 overlapping;         /**< pairs the reference found overlapping ( the alternating projection converged to a common point ) */
    UInt32 separated;           /**< pairs the reference found apart by more than 1e-3 */
    UInt32 skipped;             /**< pairs in between: too close to call */
    UInt32 bad;

    SOverlapSweep( void ) : pairs( 0 ), overlapping( 0 ), separated( 0 ), skipped( 0 ), bad( 0 ) {}
};

/*-------------------------------------------------------------------------*/

/**
 *  'count' seeded pairs of random shapes: Intersects in both orders against the alternating projection reference
 *  (distance below 1e-9: overlapping, above 1e-3: separated, in between: not decided by the reference).
 */
template < typename T, typename TA, typename TB >
static void
OverlapSweep( const char* name, UInt64 seed, UInt32 count, SOverlapSweep& sweep )
{
    CRandom random( seed );
    for ( UInt32 i=0; i<count; ++i )
    {
        TA a;
        TB b;
        RandomShape( random, a );
        RandomShape( random, b );
        ++sweep.pairs;
        const bool got = Intersects( a, b );
        const bool gotSwapped = Intersects( b, a );
        P3 onA;
        P3 onB;
        const Float64 distance = ConvergedDistance( Widen( a ), Widen( b ), onA, onB );
        if ( got != gotSwapped )
        {
            ++sweep.bad;
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Overlaps: " << name << " differs between the two orders of the arguments, seed " << seed << " pair " << i );
            continue;
        }
        if ( distance < 1.0e-9 )
        {
            ++sweep.overlapping;
            if ( !got )
            {
                ++sweep.bad;
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Overlaps: " << name << " missed an overlap, seed " << seed << " pair " << i );
            }
        }
        else if ( distance > 1.0e-3 )
        {
            ++sweep.separated;
            if ( got )
            {
                ++sweep.bad;
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Overlaps: " << name << " reports an overlap of shapes " << distance << " apart, seed " << seed << " pair " << i );
            }
        }
        else
        {
            ++sweep.skipped;
        }
    }
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Overlaps: " << name << " seed " << seed << ": " << sweep.pairs << " pairs, " << sweep.overlapping << " overlapping, " << sweep.separated << " separated, "
               << sweep.skipped << " undecided, " << sweep.bad << " disagreements" );
}

/*-------------------------------------------------------------------------*/

/** The sweep passes when nothing disagrees, most pairs were decided, and both outcomes occurred ( unless one is impossible, as for random segments ) */
static bool
OverlapPassed( const SOverlapSweep& sweep, bool bothOutcomes )
{
    if ( 0 != sweep.bad || sweep.skipped > sweep.pairs / 10 )
        return false;
    if ( bothOutcomes )
        return sweep.overlapping > sweep.pairs / 20 && sweep.separated > sweep.pairs / 20;
    return true;
}

/*-------------------------------------------------------------------------*/

template < typename T, typename TA, typename TB >
static bool
RunOverlapCase( const char* name, UInt64 seed, bool bothOutcomes )
{
    SOverlapSweep sweep;
    OverlapSweep< T, TA, TB >( name, seed, 10000, sweep );
    return OverlapPassed( sweep, bothOutcomes );
}

/*-------------------------------------------------------------------------*/

/** A pair of shapes at a known gap: Intersects must follow the sign of the gap, 0 touches */
template < typename T >
static bool
TouchingCasesPass( void )
{
    typedef CTPoint3D< T > P;
    typedef CTVector3D< T > V;
    typedef CTDirectionVector3D< T > D;
    const T half = (T) 0.5;
    const T gap = (T) 1.0e-3;
    bool ok = true;

    /* sphere and box: the sphere of radius 0.5 centred 1 + 0.5 + g from the middle of the +X face */
    const CTAxisAlignedBox3D< T > box( P( -1, -1, -1 ), P( 1, 1, 1 ) );
    ok = ok && Intersects( CTSphere< T >( P( (T) 1.5, 0, 0 ), half ), box );                                    /* exactly touching */
    ok = ok && Intersects( CTSphere< T >( P( (T) 1.5 - gap, 0, 0 ), half ), box );
    ok = ok && !Intersects( CTSphere< T >( P( (T) 1.5 + gap, 0, 0 ), half ), box );
    /* the corner: the sphere on the diagonal */
    const T diagonal = (T) 1.0 + (T) ( 0.5 / sqrt( 3.0 ) );
    ok = ok && Intersects( CTSphere< T >( P( diagonal - gap, diagonal - gap, diagonal - gap ), half ), box );
    ok = ok && !Intersects( CTSphere< T >( P( diagonal + gap, diagonal + gap, diagonal + gap ), half ), box );

    /* sphere and oriented box, rotated a quarter turn: the face moved to the Y axis */
    const CTQuaternion< T > quarter = CTQuaternion< T >::FromAxisAngle( D( V( 0, 0, 1 ) ), CTRadian< T >( (T) 1.5707963267948966 ) );
    const CTOrientedBox3D< T > oriented( P( 0, 0, 0 ), quarter, V( 2, 1, 1 ) );
    ok = ok && Intersects( CTSphere< T >( P( 0, (T) 2.5 - gap, 0 ), half ), oriented );
    ok = ok && !Intersects( CTSphere< T >( P( 0, (T) 2.5 + gap, 0 ), half ), oriented );

    /* boxes: faces within a gap, exactly touching, interpenetrating, and the same for the oriented boxes */
    ok = ok && Intersects( CTAxisAlignedBox3D< T >( P( 1, -1, -1 ), P( 3, 1, 1 ) ), box );
    ok = ok && Intersects( CTAxisAlignedBox3D< T >( P( 1 - gap, -1, -1 ), P( 3, 1, 1 ) ), box );
    ok = ok && !Intersects( CTAxisAlignedBox3D< T >( P( 1 + gap, -1, -1 ), P( 3, 1, 1 ) ), box );
    const CTOrientedBox3D< T > unit( P( 0, 0, 0 ), CTQuaternion< T >::Identity(), V( 1, 1, 1 ) );
    ok = ok && Intersects( CTOrientedBox3D< T >( P( 2, 0, 0 ), CTQuaternion< T >::Identity(), V( 1, 1, 1 ) ), unit );
    ok = ok && !Intersects( CTOrientedBox3D< T >( P( 2 + gap, 0, 0 ), CTQuaternion< T >::Identity(), V( 1, 1, 1 ) ), unit );
    ok = ok && Intersects( CTOrientedBox3D< T >( P( 2 - gap, 0, 0 ), quarter, V( 1, 1, 1 ) ), unit );
    ok = ok && !Intersects( CTOrientedBox3D< T >( P( 2 + gap, 0, 0 ), quarter, V( 1, 1, 1 ) ), unit );
    ok = ok && !Intersects( CTOrientedBox3D< T >( P( 2 + gap, 0, 0 ), quarter, V( 1, 1, 1 ) ), box );
    ok = ok && Intersects( CTOrientedBox3D< T >( P( 2 - gap, 0, 0 ), quarter, V( 1, 1, 1 ) ), box );

    /* triangle and box: a vertex on a face, an edge in a face plane, a vertex away */
    ok = ok && Intersects( CTTriangle3D< T >( P( 1, 0, 0 ), P( 3, 1, 0 ), P( 3, -1, 0 ) ), box );
    ok = ok && !Intersects( CTTriangle3D< T >( P( 1 + gap, 0, 0 ), P( 3, 1, 0 ), P( 3, -1, 0 ) ), box );
    ok = ok && Intersects( CTTriangle3D< T >( P( 1 - gap, 0, 0 ), P( 3, 1, 0 ), P( 3, -1, 0 ) ), box );
    ok = ok && Intersects( CTTriangle3D< T >( P( -3, 1, 0 ), P( 3, 1, 0 ), P( 0, 2, 2 ) ), box );                /* an edge in the +Y face plane */
    ok = ok && Intersects( CTTriangle3D< T >( P( -3, -3, 1 ), P( 3, -3, 1 ), P( 0, 3, 1 ) ), box );              /* the triangle in the +Z face plane */
    ok = ok && !Intersects( CTTriangle3D< T >( P( -3, -3, 1 + gap ), P( 3, -3, 1 + gap ), P( 0, 3, 1 + gap ) ), box );

    /* segment and box: in a face plane, crossing, ending on a face, a zero-length segment as a point */
    ok = ok && Intersects( CTSegment3D< T >( P( -3, 1, 0 ), P( 3, 1, 0 ) ), box );
    ok = ok && !Intersects( CTSegment3D< T >( P( -3, 1 + gap, 0 ), P( 3, 1 + gap, 0 ) ), box );
    ok = ok && Intersects( CTSegment3D< T >( P( 1, 0, 0 ), P( 3, 0, 0 ) ), box );
    ok = ok && !Intersects( CTSegment3D< T >( P( 1 + gap, 0, 0 ), P( 3, 0, 0 ) ), box );
    ok = ok && Intersects( CTSegment3D< T >( P( 0, 0, 0 ), P( 0, 0, 0 ) ), box );
    ok = ok && Intersects( CTSegment3D< T >( P( 1, 1, 1 ), P( 1, 1, 1 ) ), box );
    ok = ok && !Intersects( CTSegment3D< T >( P( 2, 0, 0 ), P( 2, 0, 0 ) ), box );
    ok = ok && Intersects( CTSegment3D< T >( P( 0, 0, 0 ), P( 0, 0, 0 ) ), oriented ) && !Intersects( CTSegment3D< T >( P( 0, 0, 2 ), P( 0, 0, 2 ) ), oriented );
    ok = ok && Intersects( CTSegment3D< T >( P( -5, 2, 0 ), P( 5, 2, 0 ) ), oriented ) && !Intersects( CTSegment3D< T >( P( -5, 2 + gap, 0 ), P( 5, 2 + gap, 0 ) ), oriented );

    /* sphere and triangle, sphere and capsule, capsules */
    const CTTriangle3D< T > flat( P( -2, -2, 0 ), P( 2, -2, 0 ), P( 0, 2, 0 ) );
    ok = ok && Intersects( CTSphere< T >( P( 0, 0, (T) 0.5 ), half ), flat );
    ok = ok && Intersects( CTSphere< T >( P( 0, 0, (T) 0.5 - gap ), half ), flat );
    ok = ok && !Intersects( CTSphere< T >( P( 0, 0, (T) 0.5 + gap ), half ), flat );
    const CTCapsule< T > capsule( P( 0, 0, 0 ), P( 0, 0, 2 ), half );
    ok = ok && Intersects( CTSphere< T >( P( 1, 0, 1 ), half ), capsule );
    ok = ok && !Intersects( CTSphere< T >( P( 1 + gap, 0, 1 ), half ), capsule );
    ok = ok && Intersects( CTCapsule< T >( P( 1, -1, 1 ), P( 1, 1, 1 ), half ), capsule );
    ok = ok && !Intersects( CTCapsule< T >( P( 1 + gap, -1, 1 ), P( 1 + gap, 1, 1 ), half ), capsule );
    ok = ok && Intersects( CTCapsule< T >( P( 0, 0, 2 ), P( 0, 0, 4 ), half ), capsule );                            /* the end spheres overlap */
    ok = ok && !Intersects( CTCapsule< T >( P( 0, 0, 3 + gap ), P( 0, 0, 4 ), half ), capsule );

    /* segments: crossing, touching end to end, collinear overlap, apart and parallel, zero length */
    ok = ok && Intersects( CTSegment3D< T >( P( -1, 0, 0 ), P( 1, 0, 0 ) ), CTSegment3D< T >( P( 0, -1, 0 ), P( 0, 1, 0 ) ) );
    ok = ok && Intersects( CTSegment3D< T >( P( 0, 0, 0 ), P( 1, 0, 0 ) ), CTSegment3D< T >( P( 1, 0, 0 ), P( 1, 1, 0 ) ) );
    ok = ok && Intersects( CTSegment3D< T >( P( 0, 0, 0 ), P( 2, 0, 0 ) ), CTSegment3D< T >( P( 1, 0, 0 ), P( 3, 0, 0 ) ) );
    ok = ok && !Intersects( CTSegment3D< T >( P( 0, 0, 0 ), P( 2, 0, 0 ) ), CTSegment3D< T >( P( 0, gap, 0 ), P( 2, gap, 0 ) ) );
    ok = ok && !Intersects( CTSegment3D< T >( P( 0, 0, 0 ), P( 1, 0, 0 ) ), CTSegment3D< T >( P( 2, 0, 0 ), P( 3, 0, 0 ) ) );
    ok = ok && Intersects( CTSegment3D< T >( P( 0, 0, 0 ), P( 0, 0, 0 ) ), CTSegment3D< T >( P( -1, 0, 0 ), P( 1, 0, 0 ) ) );
    ok = ok && Intersects( CTSegment3D< T >( P( 0, 0, 0 ), P( 0, 0, 0 ) ), CTSegment3D< T >( P( 0, 0, 0 ), P( 0, 0, 0 ) ) );
    ok = ok && !Intersects( CTSegment3D< T >( P( 0, 0, 0 ), P( 0, 0, 0 ) ), CTSegment3D< T >( P( 0, 1, 0 ), P( 0, 1, 0 ) ) );
    return ok;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformOverlapsTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "Overlaps" );

    GUCEF_TESTFW_TESTCASE( "Overlaps: sphere-AABB, sphere-OBB, AABB-AABB and OBB-OBB, 10000 seeded pairs each in Float32 and Float64" )
    try
    {
        ASSERT_TRUE( ( RunOverlapCase< Float32, CTSphere< Float32 >, CTAxisAlignedBox3D< Float32 > >( "sphere-aabb32", 201, true ) ) );
        ASSERT_TRUE( ( RunOverlapCase< Float64, CTSphere< Float64 >, CTAxisAlignedBox3D< Float64 > >( "sphere-aabb64", 202, true ) ) );
        ASSERT_TRUE( ( RunOverlapCase< Float32, CTSphere< Float32 >, CTOrientedBox3D< Float32 > >( "sphere-obb32", 203, true ) ) );
        ASSERT_TRUE( ( RunOverlapCase< Float64, CTSphere< Float64 >, CTOrientedBox3D< Float64 > >( "sphere-obb64", 204, true ) ) );
        ASSERT_TRUE( ( RunOverlapCase< Float32, CTAxisAlignedBox3D< Float32 >, CTAxisAlignedBox3D< Float32 > >( "aabb-aabb32", 205, true ) ) );
        ASSERT_TRUE( ( RunOverlapCase< Float64, CTAxisAlignedBox3D< Float64 >, CTAxisAlignedBox3D< Float64 > >( "aabb-aabb64", 206, true ) ) );
        ASSERT_TRUE( ( RunOverlapCase< Float32, CTOrientedBox3D< Float32 >, CTOrientedBox3D< Float32 > >( "obb-obb32", 207, true ) ) );
        ASSERT_TRUE( ( RunOverlapCase< Float64, CTOrientedBox3D< Float64 >, CTOrientedBox3D< Float64 > >( "obb-obb64", 208, true ) ) );
        ASSERT_TRUE( ( RunOverlapCase< Float32, CTAxisAlignedBox3D< Float32 >, CTOrientedBox3D< Float32 > >( "aabb-obb32", 209, true ) ) );
        ASSERT_TRUE( ( RunOverlapCase< Float64, CTAxisAlignedBox3D< Float64 >, CTOrientedBox3D< Float64 > >( "aabb-obb64", 210, true ) ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Overlaps: triangle-AABB, triangle-OBB, segment-AABB, segment-OBB, 10000 seeded pairs each in Float32 and Float64" )
    try
    {
        ASSERT_TRUE( ( RunOverlapCase< Float32, CTTriangle3D< Float32 >, CTAxisAlignedBox3D< Float32 > >( "triangle-aabb32", 211, true ) ) );
        ASSERT_TRUE( ( RunOverlapCase< Float64, CTTriangle3D< Float64 >, CTAxisAlignedBox3D< Float64 > >( "triangle-aabb64", 212, true ) ) );
        ASSERT_TRUE( ( RunOverlapCase< Float32, CTTriangle3D< Float32 >, CTOrientedBox3D< Float32 > >( "triangle-obb32", 213, true ) ) );
        ASSERT_TRUE( ( RunOverlapCase< Float64, CTTriangle3D< Float64 >, CTOrientedBox3D< Float64 > >( "triangle-obb64", 214, true ) ) );
        ASSERT_TRUE( ( RunOverlapCase< Float32, CTSegment3D< Float32 >, CTAxisAlignedBox3D< Float32 > >( "segment-aabb32", 215, true ) ) );
        ASSERT_TRUE( ( RunOverlapCase< Float64, CTSegment3D< Float64 >, CTAxisAlignedBox3D< Float64 > >( "segment-aabb64", 216, true ) ) );
        ASSERT_TRUE( ( RunOverlapCase< Float32, CTSegment3D< Float32 >, CTOrientedBox3D< Float32 > >( "segment-obb32", 217, true ) ) );
        ASSERT_TRUE( ( RunOverlapCase< Float64, CTSegment3D< Float64 >, CTOrientedBox3D< Float64 > >( "segment-obb64", 218, true ) ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Overlaps: sphere-triangle, sphere-capsule, capsule-capsule, triangle-capsule, 10000 seeded pairs each in Float32 and Float64" )
    try
    {
        ASSERT_TRUE( ( RunOverlapCase< Float32, CTSphere< Float32 >, CTTriangle3D< Float32 > >( "sphere-triangle32", 221, true ) ) );
        ASSERT_TRUE( ( RunOverlapCase< Float64, CTSphere< Float64 >, CTTriangle3D< Float64 > >( "sphere-triangle64", 222, true ) ) );
        ASSERT_TRUE( ( RunOverlapCase< Float32, CTSphere< Float32 >, CTCapsule< Float32 > >( "sphere-capsule32", 223, true ) ) );
        ASSERT_TRUE( ( RunOverlapCase< Float64, CTSphere< Float64 >, CTCapsule< Float64 > >( "sphere-capsule64", 224, true ) ) );
        ASSERT_TRUE( ( RunOverlapCase< Float32, CTCapsule< Float32 >, CTCapsule< Float32 > >( "capsule-capsule32", 225, true ) ) );
        ASSERT_TRUE( ( RunOverlapCase< Float64, CTCapsule< Float64 >, CTCapsule< Float64 > >( "capsule-capsule64", 226, true ) ) );
        ASSERT_TRUE( ( RunOverlapCase< Float32, CTTriangle3D< Float32 >, CTCapsule< Float32 > >( "triangle-capsule32", 227, true ) ) );
        ASSERT_TRUE( ( RunOverlapCase< Float64, CTTriangle3D< Float64 >, CTCapsule< Float64 > >( "triangle-capsule64", 228, true ) ) );
        ASSERT_TRUE( ( RunOverlapCase< Float32, CTSphere< Float32 >, CTSphere< Float32 > >( "sphere-sphere32", 229, true ) ) );
        ASSERT_TRUE( ( RunOverlapCase< Float64, CTSphere< Float64 >, CTSphere< Float64 > >( "sphere-sphere64", 230, true ) ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Overlaps: segment-triangle, segment-sphere, segment-capsule, triangle-triangle, segment-segment, 10000 seeded pairs each" )
    try
    {
        ASSERT_TRUE( ( RunOverlapCase< Float32, CTSegment3D< Float32 >, CTTriangle3D< Float32 > >( "segment-triangle32", 231, true ) ) );
        ASSERT_TRUE( ( RunOverlapCase< Float64, CTSegment3D< Float64 >, CTTriangle3D< Float64 > >( "segment-triangle64", 232, true ) ) );
        ASSERT_TRUE( ( RunOverlapCase< Float32, CTSegment3D< Float32 >, CTSphere< Float32 > >( "segment-sphere32", 233, true ) ) );
        ASSERT_TRUE( ( RunOverlapCase< Float64, CTSegment3D< Float64 >, CTSphere< Float64 > >( "segment-sphere64", 234, true ) ) );
        ASSERT_TRUE( ( RunOverlapCase< Float32, CTSegment3D< Float32 >, CTCapsule< Float32 > >( "segment-capsule32", 235, true ) ) );
        ASSERT_TRUE( ( RunOverlapCase< Float64, CTSegment3D< Float64 >, CTCapsule< Float64 > >( "segment-capsule64", 236, true ) ) );
        ASSERT_TRUE( ( RunOverlapCase< Float32, CTTriangle3D< Float32 >, CTTriangle3D< Float32 > >( "triangle-triangle32", 237, true ) ) );
        ASSERT_TRUE( ( RunOverlapCase< Float64, CTTriangle3D< Float64 >, CTTriangle3D< Float64 > >( "triangle-triangle64", 238, true ) ) );
        /* two random segments in space never meet: the reference says apart for all but the zero length ones that sit on the other */
        ASSERT_TRUE( ( RunOverlapCase< Float32, CTSegment3D< Float32 >, CTSegment3D< Float32 > >( "segment-segment32", 239, false ) ) );
        ASSERT_TRUE( ( RunOverlapCase< Float64, CTSegment3D< Float64 >, CTSegment3D< Float64 > >( "segment-segment64", 240, false ) ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Overlaps: touching and nearly touching configurations, exact touch, 1e-3 apart and 1e-3 into each other" )
    try
    {
        ASSERT_TRUE( TouchingCasesPass< Float32 >() );
        ASSERT_TRUE( TouchingCasesPass< Float64 >() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
