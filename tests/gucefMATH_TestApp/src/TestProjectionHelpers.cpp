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

#include <float.h>
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

#ifndef GUCEF_MATH_VECTORS_H
#include "gucefMATH_vectors.h"
#define GUCEF_MATH_VECTORS_H
#endif /* GUCEF_MATH_VECTORS_H ? */

#ifndef GUCEF_MATH_POINTS_H
#include "gucefMATH_points.h"
#define GUCEF_MATH_POINTS_H
#endif /* GUCEF_MATH_POINTS_H ? */

#ifndef GUCEF_MATH_DIRECTIONS_H
#include "gucefMATH_directions.h"
#define GUCEF_MATH_DIRECTIONS_H
#endif /* GUCEF_MATH_DIRECTIONS_H ? */

#ifndef GUCEF_MATH_MATRICES_H
#include "gucefMATH_matrices.h"
#define GUCEF_MATH_MATRICES_H
#endif /* GUCEF_MATH_MATRICES_H ? */

#ifndef GUCEF_MATH_PROJECTIONFUNCTIONS_H
#include "gucefMATH_ProjectionFunctions.h"
#define GUCEF_MATH_PROJECTIONFUNCTIONS_H
#endif /* GUCEF_MATH_PROJECTIONFUNCTIONS_H ? */

#include "TestMathReference.h"
#include "TestProjectionHelpers.h"

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

static const UInt32 ConventionCount = 7;

/** The presets of 4.5.6 and variations that the presets do not cover (finite reversed depth, infinite far with a flipped Y) */
static SProjectionConvention
GetConvention( UInt32 index )
{
    switch ( index )
    {
        case 0: return SProjectionConvention::OpenGL();
        case 1: return SProjectionConvention::Vulkan();
        case 2: return SProjectionConvention::Direct3D();
        case 3: return SProjectionConvention::ReversedZInfinite( HANDEDNESS_RIGHT );
        case 4: return SProjectionConvention::ReversedZInfinite( HANDEDNESS_LEFT );
        case 5: return SProjectionConvention::Make( HANDEDNESS_RIGHT, DEPTHRANGE_ZERO_TO_ONE, true, false, false );
        default: return SProjectionConvention::Make( HANDEDNESS_LEFT, DEPTHRANGE_MINUS_ONE_TO_ONE, false, true, true );
    }
}

/*-------------------------------------------------------------------------*/

/** The view space z of a point 'distance' in front of a camera of the given handedness */
static Float64
ViewZ( const SProjectionConvention& convention, Float64 distance )
{
    return HANDEDNESS_LEFT == convention.handedness ? distance : -distance;
}

/*-------------------------------------------------------------------------*/

/** Calls the projection helpers from a realtime context: a RealtimeSanitizer build reports any allocation */
static Float32
RealtimeProjectionHelpersWork( Float32 distance ) GUCEF_MATH_REALTIME_CONTEXT
{
    const SProjectionConvention convention = SProjectionConvention::Vulkan();
    const TFloat32Matrix4D vp = TFloat32Matrix4D::Perspective( TFloat32Radian( 1.0f ), 1.5f, 0.1f, 100.0f, convention ) *
                                TFloat32Matrix4D::LookAt( TFloat32Point3D( 0, 1, 2 ), TFloat32Point3D( 0, 0, -distance ), TFloat32Vector3D::Up(), HANDEDNESS_RIGHT );
    TFloat32Matrix4D inverse;
    if ( !vp.TryInverse( inverse, 0.0f ) )
        return 0.0f;
    const TFloat32Viewport viewport = TFloat32Viewport::FromConvention( 0.0f, 0.0f, 640.0f, 480.0f, convention );
    TFloat32Point2D screen;
    Float32 depth = 0.0f;
    TFloat32Point3D world;
    TFloat32Point3D origin;
    TFloat32DirectionVector3D direction;
    const bool projected = Project( TFloat32Point3D( 0.25f, 0.5f, -distance ), vp, viewport, screen, depth );
    const bool unprojected = Unproject( screen, depth, inverse, viewport, world );
    const bool ray = GetPickRay( screen, inverse, viewport, convention, origin, direction );
    return projected && unprojected && ray ? screen.x + screen.y + depth + world.x + origin.y + direction.GetZ() : 0.0f;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformProjectionHelpersTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "ProjectionHelpers" );

    GUCEF_TESTFW_TESTCASE( "ProjectionHelpers: the viewport, its validity, conventions and text and DataNode forms" )
    try
    {
        TFloat64Viewport unit;
        ASSERT_TRUE( 0.0 == unit.x && 0.0 == unit.y && 1.0 == unit.width && 1.0 == unit.height && !unit.ndcYDown && unit.IsValid() );
        ASSERT_TRUE( TFloat64Viewport::FromConvention( 1, 2, 3, 4, SProjectionConvention::Vulkan() ).ndcYDown );
        ASSERT_FALSE( TFloat64Viewport::FromConvention( 1, 2, 3, 4, SProjectionConvention::OpenGL() ).ndcYDown );
        ASSERT_FALSE( TFloat64Viewport( 0, 0, 0, 10, false ).IsValid() );
        ASSERT_FALSE( TFloat64Viewport( 0, 0, 10, -1, false ).IsValid() );
        ASSERT_TRUE( TFloat64Viewport( 0, 0, 10, 10, false ).IsValid() );
        ASSERT_FALSE( TFloat64Viewport( 0, 0, CTNumericTraits< Float64 >::Max() * 2.0, 10, false ).IsValid() );

        TFloat64Viewport a( 10.5, 20.25, 1920, 1080, true );
        TFloat64Viewport b( 10.5, 20.25, 1920, 1080, false );
        ASSERT_TRUE( a != b && a == TFloat64Viewport( 10.5, 20.25, 1920, 1080, true ) );

        // text
        CORE::CString textA = a.ToString();
        CORE::CString textB = b.ToString();
        ASSERT_TRUE( textB == "(10.5, 20.25, 1920, 1080)" );
        ASSERT_TRUE( textA == "(10.5, 20.25, 1920, 1080, ndcYDown)" );
        TFloat64Viewport readA;
        TFloat64Viewport readB( 0, 0, 5, 5, true );
        ASSERT_TRUE( readA.FromString( textA ) && readA == a );
        ASSERT_TRUE( readB.FromString( CORE::CString( " " ) + textB + " " ) && readB == b );
        TFloat64Viewport untouched( 1, 2, 3, 4, true );
        ASSERT_FALSE( untouched.FromString( "(1, 2, 3)" ) );
        ASSERT_FALSE( untouched.FromString( "(1, 2, 3, 4, flipped)" ) );
        ASSERT_FALSE( untouched.FromString( "" ) );
        ASSERT_TRUE( untouched == TFloat64Viewport( 1, 2, 3, 4, true ) );

        // DataNode
        CORE::CDataNodeSerializableSettings settings;
        CORE::CDataNode node;
        ASSERT_TRUE( a.Serialize( node, settings ) );
        TFloat64Viewport fromNode;
        ASSERT_TRUE( fromNode.Deserialize( node, settings ) && fromNode == a );
        CORE::CDataNode incomplete;
        ASSERT_TRUE( incomplete.SetAttribute( "x", CORE::CVariant( 1.0 ) ) );
        TFloat64Viewport keep( 7, 8, 9, 10, false );
        ASSERT_FALSE( keep.Deserialize( incomplete, settings ) );
        ASSERT_TRUE( keep == TFloat64Viewport( 7, 8, 9, 10, false ) );

        // the layout is four values and a flag
        ASSERT_TRUE( sizeof( TFloat32Viewport ) <= 5 * sizeof( Float32 ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "ProjectionHelpers: the screen position is the same for every convention and follows the pinhole geometry; depth follows the convention" )
    try
    {
        // acceptance 5: a view-space point in front of an identity view; screen Y points down, the world's up is the top of the screen
        const Float64 fov = 1.1;
        const Float64 aspect = 16.0 / 9.0;
        const Float64 nearZ = 0.25;
        const Float64 farZ = 300.0;
        const Float64 tangent = tan( fov * 0.5 );
        UInt32 failures = 0;
        for ( UInt32 c = 0; c < ConventionCount; ++c )
        {
            const SProjectionConvention convention = GetConvention( c );
            const TFloat64Matrix4D projection = TFloat64Matrix4D::Perspective( TFloat64Radian( fov ), aspect, nearZ, farZ, convention );
            const TFloat64Viewport viewport = TFloat64Viewport::FromConvention( 100.0, 50.0, 1600.0, 900.0, convention );
            const Float64 samples[ 6 ][ 3 ] = { { 0.0, 0.0, 5.0 }, { 1.0, 0.5, 5.0 }, { -2.0, 1.5, 10.0 }, { 3.0, -2.0, 7.5 }, { 0.001, -0.001, 250.0 }, { -0.1, 0.05, 0.3 } };
            for ( UInt32 s = 0; s < 6; ++s )
            {
                const Float64 distance = samples[ s ][ 2 ];
                TFloat64Point2D screen;
                Float64 depth = 0.0;
                const bool ok = Project( TFloat64Point3D( samples[ s ][ 0 ], samples[ s ][ 1 ], ViewZ( convention, distance ) ), projection, viewport, screen, depth );
                const Float64 ndcX = samples[ s ][ 0 ] / ( distance * tangent * aspect );
                const Float64 ndcY = samples[ s ][ 1 ] / ( distance * tangent );
                const Float64 expectedX = 100.0 + ( ndcX + 1.0 ) * 0.5 * 1600.0;
                const Float64 expectedY = 50.0 + ( 1.0 - ndcY ) * 0.5 * 900.0;
                if ( !ok || fabs( screen.x - expectedX ) > 1e-8 || fabs( screen.y - expectedY ) > 1e-8 )
                    ++failures;
            }

            // the depth of the near plane, and of the far plane when it is finite
            TFloat64Point2D screen;
            Float64 depth = 0.0;
            ASSERT_TRUE( Project( TFloat64Point3D( 0, 0, ViewZ( convention, nearZ ) ), projection, viewport, screen, depth ) );
            ASSERT_TRUE( fabs( depth - (Float64) convention.GetNearDepth() ) < 1e-12 );
            if ( !convention.infiniteFar )
            {
                ASSERT_TRUE( Project( TFloat64Point3D( 0, 0, ViewZ( convention, farZ ) ), projection, viewport, screen, depth ) );
                ASSERT_TRUE( fabs( depth - (Float64) convention.GetFarDepth() ) < 1e-9 );
            }
            else
            {
                // an infinite far plane approaches the far depth
                ASSERT_TRUE( Project( TFloat64Point3D( 0, 0, ViewZ( convention, 1.0e7 ) ), projection, viewport, screen, depth ) );
                ASSERT_TRUE( fabs( depth - (Float64) convention.GetFarDepth() ) < 1e-6 );
            }
            // the centre of the viewport
            ASSERT_TRUE( Project( TFloat64Point3D( 0, 0, ViewZ( convention, 3.0 ) ), projection, viewport, screen, depth ) );
            ASSERT_TRUE( fabs( screen.x - 900.0 ) < 1e-9 && fabs( screen.y - 500.0 ) < 1e-9 );
            // up in the world is the top of the screen, right is the right of the screen
            TFloat64Point2D up;
            TFloat64Point2D rightOf;
            ASSERT_TRUE( Project( TFloat64Point3D( 0, 1, ViewZ( convention, 3.0 ) ), projection, viewport, up, depth ) );
            ASSERT_TRUE( Project( TFloat64Point3D( 1, 0, ViewZ( convention, 3.0 ) ), projection, viewport, rightOf, depth ) );
            ASSERT_TRUE( up.y < screen.y && fabs( up.x - screen.x ) < 1e-9 && rightOf.x > screen.x && fabs( rightOf.y - screen.y ) < 1e-9 );
        }
        ASSERT_TRUE( 0 == failures );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "ProjectionHelpers: Project and Unproject round trip within 1e-5 for each convention (seeded views and points)" )
    try
    {
        // acceptance 5
        const UInt64 seed = 0x9A71;
        MathReference::CSeededSequence generator( seed );
        UInt32 failures = 0;
        UInt32 checked = 0;
        Float64 worst = 0.0;
        for ( UInt32 c = 0; c < ConventionCount; ++c )
        {
            const SProjectionConvention convention = GetConvention( c );
            const TFloat64Viewport viewport = TFloat64Viewport::FromConvention( 10.0, 20.0, 1920.0, 1080.0, convention );
            for ( UInt32 i = 0; i < 300; ++i )
            {
                const Float64 ex = generator.NextFloat64( -50.0, 50.0 );
                const Float64 ey = generator.NextFloat64( -50.0, 50.0 );
                const Float64 ez = generator.NextFloat64( -50.0, 50.0 );
                const Float64 tx = generator.NextFloat64( -50.0, 50.0 );
                const Float64 ty = generator.NextFloat64( -50.0, 50.0 );
                const Float64 tz = generator.NextFloat64( -50.0, 50.0 );
                const Float64 ux = generator.NextFloat64( -1.0, 1.0 );
                const Float64 uy = generator.NextFloat64( 0.5, 1.0 );
                const Float64 uz = generator.NextFloat64( -1.0, 1.0 );
                const TFloat64Point3D eye( ex, ey, ez );
                const TFloat64Point3D target( tx, ty, tz );
                if ( ( target - eye ).GetLength() < 1.0 )
                    continue;
                ++checked;
                const TFloat64Matrix4D view = TFloat64Matrix4D::LookAt( eye, target, TFloat64Vector3D( ux, uy, uz ), convention.handedness );
                const TFloat64Matrix4D projection = TFloat64Matrix4D::Perspective( TFloat64Radian( 1.0 ), 16.0 / 9.0, 0.1, 1000.0, convention );
                const TFloat64Matrix4D viewProjection = projection * view;
                TFloat64Matrix4D inverse;
                ASSERT_TRUE( viewProjection.TryInverse( inverse, 0.0 ) );

                // a view space point inside the frustum, then its world position
                const Float64 distance = generator.NextFloat64( 0.5, 500.0 );
                const Float64 fx = generator.NextFloat64( -0.9, 0.9 );
                const Float64 fy = generator.NextFloat64( -0.9, 0.9 );
                const Float64 half = tan( 0.5 );
                const TFloat64Point3D viewPoint( fx * distance * half * 16.0 / 9.0, fy * distance * half, ViewZ( convention, distance ) );
                const TFloat64Point3D world = view.Inverse() * viewPoint;

                TFloat64Point2D screen;
                Float64 depth = 0.0;
                TFloat64Point3D back;
                const bool projected = Project( world, viewProjection, viewport, screen, depth );
                const bool unprojected = projected && Unproject( screen, depth, inverse, viewport, back );
                const Float64 error = unprojected ? ( back - world ).GetLength() / ( 1.0 + ( world - TFloat64Point3D::Origin() ).GetLength() ) : 1.0;
                if ( error > worst )
                    worst = error;
                // the point is inside the viewport rectangle by construction
                const bool inside = screen.x >= 10.0 && screen.x <= 1930.0 && screen.y >= 20.0 && screen.y <= 1100.0;
                if ( !projected || !unprojected || !inside || error > 1e-5 )
                    ++failures;
            }
        }
        ASSERT_TRUE( 0 == failures );
        ASSERT_TRUE( checked > 2000 && worst < 1e-7 );
        if ( 0 != failures )
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ProjectionHelpers: " << failures << " round trips failed, seed " << seed << ", worst error " << worst );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "ProjectionHelpers: points behind the camera, on the camera plane and at infinity are reported, not divided" )
    try
    {
        for ( UInt32 c = 0; c < ConventionCount; ++c )
        {
            const SProjectionConvention convention = GetConvention( c );
            const TFloat64Matrix4D projection = TFloat64Matrix4D::Perspective( TFloat64Radian( 1.0 ), 1.0, 0.1, 100.0, convention );
            const TFloat64Viewport viewport = TFloat64Viewport::FromConvention( 0.0, 0.0, 800.0, 600.0, convention );
            TFloat64Point2D screen( -1.0, -2.0 );
            Float64 depth = -3.0;
            // behind the camera: the opposite side along the view direction
            ASSERT_FALSE( Project( TFloat64Point3D( 0.5, 0.5, ViewZ( convention, -2.0 ) ), projection, viewport, screen, depth ) );
            // on the camera plane the homogeneous w is zero
            ASSERT_FALSE( Project( TFloat64Point3D( 0.5, 0.5, 0.0 ), projection, viewport, screen, depth ) );
            ASSERT_TRUE( -1.0 == screen.x && -2.0 == screen.y && -3.0 == depth );
            // an invalid viewport
            ASSERT_FALSE( Project( TFloat64Point3D( 0, 0, ViewZ( convention, 2.0 ) ), projection, TFloat64Viewport( 0, 0, 0, 600, false ), screen, depth ) );
            TFloat64Point3D world( 9.0, 9.0, 9.0 );
            ASSERT_FALSE( Unproject( TFloat64Point2D( 1, 1 ), 0.0, projection.Inverse(), TFloat64Viewport( 0, 0, 800, -5, false ), world ) );
            ASSERT_TRUE( 9.0 == world.x && 9.0 == world.y && 9.0 == world.z );

            // the far plane at infinity cannot be unprojected
            if ( convention.infiniteFar )
            {
                TFloat64Matrix4D inverse;
                ASSERT_TRUE( projection.TryInverse( inverse, 0.0 ) );
                ASSERT_FALSE( Unproject( TFloat64Point2D( 400, 300 ), (Float64) convention.GetFarDepth(), inverse, viewport, world ) );
                ASSERT_TRUE( 9.0 == world.x );
                // just inside the far depth the point exists
                const Float64 inside = 0.5 * ( (Float64) convention.GetNearDepth() + (Float64) convention.GetFarDepth() );
                ASSERT_TRUE( Unproject( TFloat64Point2D( 400, 300 ), inside, inverse, viewport, world ) );
            }
        }

        // an orthographic projection has w = 1: every point projects, whatever its distance
        const SProjectionConvention gl = SProjectionConvention::OpenGL();
        const TFloat64Matrix4D ortho = TFloat64Matrix4D::Orthographic( -10.0, 10.0, -5.0, 5.0, 0.1, 100.0, gl );
        const TFloat64Viewport viewport = TFloat64Viewport::FromConvention( 0.0, 0.0, 200.0, 100.0, gl );
        TFloat64Point2D screen;
        Float64 depth = 0.0;
        ASSERT_TRUE( Project( TFloat64Point3D( 0, 0, -50 ), ortho, viewport, screen, depth ) && fabs( screen.x - 100.0 ) < 1e-9 && fabs( screen.y - 50.0 ) < 1e-9 );
        ASSERT_TRUE( Project( TFloat64Point3D( 10, 5, 3 ), ortho, viewport, screen, depth ) && fabs( screen.x - 200.0 ) < 1e-9 && fabs( screen.y - 0.0 ) < 1e-9 );
        ASSERT_TRUE( Project( TFloat64Point3D( -10, -5, -99 ), ortho, viewport, screen, depth ) && fabs( screen.x ) < 1e-9 && fabs( screen.y - 100.0 ) < 1e-9 );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "ProjectionHelpers: Unproject of a depth outside the depth range of the convention is reported, not mirrored behind the camera" )
    try
    {
        for ( UInt32 c = 0; c < ConventionCount; ++c )
        {
            const SProjectionConvention convention = GetConvention( c );
            const TFloat64Matrix4D projection = TFloat64Matrix4D::Perspective( TFloat64Radian( 1.0 ), 1.0, 0.1, 100.0, convention );
            const TFloat64Viewport viewport = TFloat64Viewport::FromConvention( 0.0, 0.0, 800.0, 600.0, convention );
            TFloat64Matrix4D inverse;
            ASSERT_TRUE( projection.TryInverse( inverse, 0.0 ) );
            const Float64 nearDepth = (Float64) convention.GetNearDepth();
            const Float64 farDepth = (Float64) convention.GetFarDepth();
            const Float64 towardsFar = farDepth > nearDepth ? 1.0 : -1.0;
            TFloat64Point3D world( 9.0, 9.0, 9.0 );
            const TFloat64Point2D screen( 400.0, 300.0 );

            // the near depth is in front of the camera and projects back to the view of the camera
            ASSERT_TRUE( Unproject( screen, nearDepth, inverse, viewport, world ) );
            const Float64 viewDistance = HANDEDNESS_LEFT == convention.handedness ? world.z : -world.z;
            ASSERT_TRUE( fabs( viewDistance - 0.1 ) < 1e-6 );

            // a depth beyond the far depth: finite projections still have a point there until the depth of infinity, an
            // infinite projection has w <= 0 as soon as the depth leaves the range; far beyond it all of them have w < 0
            TFloat64Point3D far( 9.0, 9.0, 9.0 );
            if ( convention.infiniteFar )
            {
                const Float64 noise[ 3 ] = { 1.0e-9, 1.0e-6, 1.0e-3 };
                for ( UInt32 n = 0; n < 3; ++n )
                {
                    world = TFloat64Point3D( 9.0, 9.0, 9.0 );
                    ASSERT_FALSE( Unproject( screen, farDepth + towardsFar * noise[ n ], inverse, viewport, world ) );
                    ASSERT_TRUE( 9.0 == world.x && 9.0 == world.y && 9.0 == world.z );
                }
                // just inside the range the point is in front of the camera
                ASSERT_TRUE( Unproject( screen, farDepth - towardsFar * 1.0e-3, inverse, viewport, world ) );
                ASSERT_TRUE( ( HANDEDNESS_LEFT == convention.handedness ? world.z : -world.z ) > 0.0 );
            }
            else
            {
                ASSERT_TRUE( Unproject( screen, farDepth, inverse, viewport, far ) );
                ASSERT_TRUE( fabs( ( HANDEDNESS_LEFT == convention.handedness ? far.z : -far.z ) - 100.0 ) < 1e-4 );
            }
            world = TFloat64Point3D( 9.0, 9.0, 9.0 );
            ASSERT_FALSE( Unproject( screen, farDepth + towardsFar * 0.5, inverse, viewport, world ) );
            ASSERT_TRUE( 9.0 == world.x && 9.0 == world.y && 9.0 == world.z );
            // a pick ray still works for every convention
            TFloat64Point3D origin;
            TFloat64DirectionVector3D direction;
            ASSERT_TRUE( GetPickRay( screen, inverse, viewport, convention, origin, direction ) );
        }
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "ProjectionHelpers: the pick ray passes through the point that projects to the screen position, for each convention (seeded)" )
    try
    {
        // acceptance 5
        const UInt64 seed = 0x71C4;
        MathReference::CSeededSequence generator( seed );
        UInt32 failures = 0;
        Float64 worst = 0.0;
        for ( UInt32 c = 0; c < ConventionCount; ++c )
        {
            const SProjectionConvention convention = GetConvention( c );
            const TFloat64Viewport viewport = TFloat64Viewport::FromConvention( 0.0, 0.0, 1280.0, 720.0, convention );
            for ( UInt32 i = 0; i < 300; ++i )
            {
                const Float64 ex = generator.NextFloat64( -30.0, 30.0 );
                const Float64 ey = generator.NextFloat64( -30.0, 30.0 );
                const Float64 ez = generator.NextFloat64( -30.0, 30.0 );
                const Float64 yaw = generator.NextFloat64( -3.1, 3.1 );
                const Float64 pitch = generator.NextFloat64( -1.2, 1.2 );
                const TFloat64Point3D eye( ex, ey, ez );
                const TFloat64Vector3D forward( sin( yaw ) * cos( pitch ), sin( pitch ), cos( yaw ) * cos( pitch ) );
                const TFloat64Matrix4D view = TFloat64Matrix4D::LookAt( eye, eye + forward, TFloat64Vector3D::Up(), convention.handedness );
                const TFloat64Matrix4D projection = TFloat64Matrix4D::Perspective( TFloat64Radian( 1.3 ), 1280.0 / 720.0, 0.2, 500.0, convention );
                const TFloat64Matrix4D viewProjection = projection * view;
                TFloat64Matrix4D inverse;
                ASSERT_TRUE( viewProjection.TryInverse( inverse, 0.0 ) );

                const Float64 distance = generator.NextFloat64( 1.0, 200.0 );
                const Float64 fx = generator.NextFloat64( -0.8, 0.8 );
                const Float64 fy = generator.NextFloat64( -0.8, 0.8 );
                const Float64 half = tan( 0.65 );
                const TFloat64Point3D world = view.Inverse() * TFloat64Point3D( fx * distance * half * 1280.0 / 720.0, fy * distance * half, ViewZ( convention, distance ) );

                TFloat64Point2D screen;
                Float64 depth = 0.0;
                TFloat64Point3D origin;
                TFloat64DirectionVector3D direction;
                const bool projected = Project( world, viewProjection, viewport, screen, depth );
                const bool ray = projected && GetPickRay( screen, inverse, viewport, convention, origin, direction );
                if ( !projected || !ray )
                {
                    ++failures;
                    continue;
                }
                // the distance of the world point to the ray, and the ray pointing away from the camera through the point
                const TFloat64Vector3D toPoint = world - origin;
                const Float64 along = direction.Dot( toPoint );
                const Float64 off = ( toPoint - direction.AsVector() * along ).GetLength();
                if ( off > worst )
                    worst = off;
                // the origin is on the near plane: it projects to the depth of the near plane
                TFloat64Point2D originScreen;
                Float64 originDepth = 0.0;
                const bool originProjects = Project( origin, viewProjection, viewport, originScreen, originDepth );
                if ( !( along > 0.0 ) || off > 1e-5 || !originProjects || fabs( originDepth - (Float64) convention.GetNearDepth() ) > 1e-8 ||
                     fabs( originScreen.x - screen.x ) > 1e-6 || fabs( originScreen.y - screen.y ) > 1e-6 )
                    ++failures;
            }
        }
        ASSERT_TRUE( 0 == failures );
        ASSERT_TRUE( worst < 1e-7 );
        if ( 0 != failures )
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ProjectionHelpers: " << failures << " pick rays failed, seed " << seed << ", worst distance " << worst );

        // the centre pixel of an identity view looks along the view direction, from the near plane
        for ( UInt32 c = 0; c < ConventionCount; ++c )
        {
            const SProjectionConvention convention = GetConvention( c );
            const TFloat64Matrix4D projection = TFloat64Matrix4D::Perspective( TFloat64Radian( 1.0 ), 1.0, 0.5, 50.0, convention );
            TFloat64Matrix4D inverse;
            ASSERT_TRUE( projection.TryInverse( inverse, 0.0 ) );
            const TFloat64Viewport viewport = TFloat64Viewport::FromConvention( 0.0, 0.0, 100.0, 100.0, convention );
            TFloat64Point3D origin;
            TFloat64DirectionVector3D direction;
            ASSERT_TRUE( GetPickRay( TFloat64Point2D( 50.0, 50.0 ), inverse, viewport, convention, origin, direction ) );
            const Float64 sign = HANDEDNESS_LEFT == convention.handedness ? 1.0 : -1.0;
            ASSERT_TRUE( fabs( origin.x ) < 1e-9 && fabs( origin.y ) < 1e-9 && fabs( origin.z - sign * 0.5 ) < 1e-9 );
            ASSERT_TRUE( fabs( direction.GetX() ) < 1e-9 && fabs( direction.GetY() ) < 1e-9 && fabs( direction.GetZ() - sign ) < 1e-9 );
            // the top left corner of the screen looks up and to the left
            ASSERT_TRUE( GetPickRay( TFloat64Point2D( 0.0, 0.0 ), inverse, viewport, convention, origin, direction ) );
            ASSERT_TRUE( direction.GetX() < 0.0 && direction.GetY() > 0.0 && sign * direction.GetZ() > 0.0 );
            // an invalid viewport gives no ray
            ASSERT_FALSE( GetPickRay( TFloat64Point2D( 50.0, 50.0 ), inverse, TFloat64Viewport( 0, 0, -1, 100, false ), convention, origin, direction ) );
        }

        // an orthographic projection: parallel rays along the view direction, the origin follows the pixel
        const SProjectionConvention gl = SProjectionConvention::OpenGL();
        const TFloat64Matrix4D ortho = TFloat64Matrix4D::Orthographic( -10.0, 10.0, -5.0, 5.0, 0.1, 100.0, gl );
        TFloat64Matrix4D orthoInverse;
        ASSERT_TRUE( ortho.TryInverse( orthoInverse, 0.0 ) );
        const TFloat64Viewport viewport = TFloat64Viewport::FromConvention( 0.0, 0.0, 200.0, 100.0, gl );
        TFloat64Point3D origin;
        TFloat64DirectionVector3D direction;
        ASSERT_TRUE( GetPickRay( TFloat64Point2D( 150.0, 25.0 ), orthoInverse, viewport, gl, origin, direction ) );
        ASSERT_TRUE( fabs( origin.x - 5.0 ) < 1e-9 && fabs( origin.y - 2.5 ) < 1e-9 && fabs( origin.z + 0.1 ) < 1e-9 );
        ASSERT_TRUE( fabs( direction.GetX() ) < 1e-12 && fabs( direction.GetY() ) < 1e-12 && fabs( direction.GetZ() + 1.0 ) < 1e-12 );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "ProjectionHelpers: Float32 round trips of Project, Unproject and GetPickRay" )
    try
    {
        const UInt64 seed = 0x3F32;
        MathReference::CSeededSequence generator( seed );
        UInt32 failures = 0;
        for ( UInt32 c = 0; c < ConventionCount; ++c )
        {
            const SProjectionConvention convention = GetConvention( c );
            const TFloat32Viewport viewport = TFloat32Viewport::FromConvention( 0.0f, 0.0f, 1024.0f, 768.0f, convention );
            const TFloat32Matrix4D view = TFloat32Matrix4D::LookAt( TFloat32Point3D( 1.0f, 2.0f, 3.0f ), TFloat32Point3D( -2.0f, 0.5f, 0.0f ), TFloat32Vector3D::Up(), convention.handedness );
            const TFloat32Matrix4D projection = TFloat32Matrix4D::Perspective( TFloat32Radian( 1.0f ), 4.0f / 3.0f, 0.5f, 100.0f, convention );
            const TFloat32Matrix4D viewProjection = projection * view;
            TFloat32Matrix4D inverse;
            ASSERT_TRUE( viewProjection.TryInverse( inverse, 0.0f ) );
            for ( UInt32 i = 0; i < 100; ++i )
            {
                const Float32 distance = (Float32) generator.NextFloat64( 1.0, 15.0 );
                const Float32 fx = (Float32) generator.NextFloat64( -0.7, 0.7 );
                const Float32 fy = (Float32) generator.NextFloat64( -0.7, 0.7 );
                const Float32 half = tanf( 0.5f );
                const TFloat32Point3D world = view.Inverse() * TFloat32Point3D( fx * distance * half * 4.0f / 3.0f, fy * distance * half, (Float32) ViewZ( convention, distance ) );
                TFloat32Point2D screen;
                Float32 depth = 0.0f;
                TFloat32Point3D back;
                TFloat32Point3D origin;
                TFloat32DirectionVector3D direction;
                const bool ok = Project( world, viewProjection, viewport, screen, depth ) && Unproject( screen, depth, inverse, viewport, back ) &&
                                GetPickRay( screen, inverse, viewport, convention, origin, direction );
                if ( !ok )
                {
                    ++failures;
                    continue;
                }
                const TFloat32Vector3D toPoint = world - origin;
                const Float32 along = direction.Dot( toPoint );
                const Float32 off = ( toPoint - direction.AsVector() * along ).GetLength();
                if ( ( back - world ).GetLength() > 5e-3f || off > 5e-3f || !( along > 0.0f ) )
                    ++failures;
            }
        }
        ASSERT_TRUE( 0 == failures );
        if ( 0 != failures )
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ProjectionHelpers: " << failures << " Float32 checks failed, seed " << seed );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "ProjectionHelpers: realtime-safe functions run in a realtime context" )
    try
    {
        ASSERT_TRUE( CTNumericTraits< Float32 >::IsFinite( RealtimeProjectionHelpersWork( 3.0f ) ) );
        ASSERT_TRUE( 0.0f != RealtimeProjectionHelpersWork( 3.0f ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
