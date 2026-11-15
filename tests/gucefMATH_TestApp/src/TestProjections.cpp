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

#ifndef GUCEF_MATH_MATRICES_H
#include "gucefMATH_matrices.h"
#define GUCEF_MATH_MATRICES_H
#endif /* GUCEF_MATH_MATRICES_H ? */

#ifndef GUCEF_MATH_SPROJECTIONCONVENTION_H
#include "gucefMATH_SProjectionConvention.h"
#define GUCEF_MATH_SPROJECTIONCONVENTION_H
#endif /* GUCEF_MATH_SPROJECTIONCONVENTION_H ? */

#include "TestMathReference.h"
#include "TestProjections.h"

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

/** The normalised device coordinates of a view-space point */
static TFloat64Vector3D
Ndc( const TFloat64Matrix4D& projection, Float64 x, Float64 y, Float64 z )
{
    TFloat64Vector4D clip = projection * TFloat64Vector4D( x, y, z, 1.0 );
    return TFloat64Vector3D( clip.x / clip.w, clip.y / clip.w, clip.z / clip.w );
}

/*-------------------------------------------------------------------------*/

/** Calls the projection builders from a realtime context */
static Float32
RealtimeProjectionWork( Float32 aspect ) GUCEF_MATH_REALTIME_CONTEXT
{
    TFloat32Matrix4D p = TFloat32Matrix4D::Perspective( TFloat32Radian( 1.0f ), aspect, 0.1f, 100.0f, SProjectionConvention::Vulkan() );
    TFloat32Matrix4D o = TFloat32Matrix4D::Orthographic( -1, 1, -1, 1, 0.1f, 10.0f, SProjectionConvention::OpenGL() );
    TFloat32Matrix4D f = TFloat32Matrix4D::PerspectiveOffCenter( -1, 2, -1, 1, 0.5f, 50.0f, SProjectionConvention::ReversedZInfinite( HANDEDNESS_LEFT ) );
    TFloat32Matrix4D v = TFloat32Matrix4D::LookAt( TFloat32Point3D( 1, 2, 3 ), TFloat32Point3D( 0, 0, 0 ), TFloat32Vector3D::Up(), HANDEDNESS_RIGHT );
    return ( p * v )( 2, 2 ) + o( 2, 3 ) + f( 3, 2 ) + (Float32) SProjectionConvention::Direct3D().GetFarDepth();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformProjectionsTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "Projections" );

    GUCEF_TESTFW_TESTCASE( "Projections: the conventions of the presets" )
    try
    {
        SProjectionConvention gl = SProjectionConvention::OpenGL();
        SProjectionConvention vk = SProjectionConvention::Vulkan();
        SProjectionConvention dx = SProjectionConvention::Direct3D();
        SProjectionConvention rz = SProjectionConvention::ReversedZInfinite( HANDEDNESS_RIGHT );
        ASSERT_TRUE( HANDEDNESS_RIGHT == gl.handedness && DEPTHRANGE_MINUS_ONE_TO_ONE == gl.depthRange && !gl.reversedZ && !gl.infiniteFar && !gl.flipY );
        ASSERT_TRUE( HANDEDNESS_RIGHT == vk.handedness && DEPTHRANGE_ZERO_TO_ONE == vk.depthRange && !vk.reversedZ && !vk.infiniteFar && vk.flipY );
        ASSERT_TRUE( HANDEDNESS_LEFT == dx.handedness && DEPTHRANGE_ZERO_TO_ONE == dx.depthRange && !dx.reversedZ && !dx.infiniteFar && !dx.flipY );
        ASSERT_TRUE( HANDEDNESS_RIGHT == rz.handedness && DEPTHRANGE_ZERO_TO_ONE == rz.depthRange && rz.reversedZ && rz.infiniteFar && !rz.flipY );
        ASSERT_TRUE( -1 == gl.GetNearDepth() && 1 == gl.GetFarDepth() && 0 == vk.GetNearDepth() && 1 == vk.GetFarDepth() && 1 == rz.GetNearDepth() && 0 == rz.GetFarDepth() );
        ASSERT_TRUE( gl == SProjectionConvention::OpenGL() && gl != vk );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Projections: near and far planes map to the depth range of each convention" )
    try
    {
        // acceptance 11
        const Float64 n = 0.5;
        const Float64 f = 200.0;
        const TFloat64Radian fov( 1.2 );
        TFloat64Matrix4D gl = TFloat64Matrix4D::Perspective( fov, 1.5, n, f, SProjectionConvention::OpenGL() );
        ASSERT_TRUE( fabs( Ndc( gl, 0, 0, -n ).z + 1.0 ) < 1e-12 && fabs( Ndc( gl, 0, 0, -f ).z - 1.0 ) < 1e-12 );
        TFloat64Matrix4D vk = TFloat64Matrix4D::Perspective( fov, 1.5, n, f, SProjectionConvention::Vulkan() );
        ASSERT_TRUE( fabs( Ndc( vk, 0, 0, -n ).z ) < 1e-12 && fabs( Ndc( vk, 0, 0, -f ).z - 1.0 ) < 1e-12 );
        TFloat64Matrix4D dx = TFloat64Matrix4D::Perspective( fov, 1.5, n, f, SProjectionConvention::Direct3D() );
        ASSERT_TRUE( fabs( Ndc( dx, 0, 0, n ).z ) < 1e-12 && fabs( Ndc( dx, 0, 0, f ).z - 1.0 ) < 1e-12 );
        // reversed Z: near -> 1, far -> 0, both depth ranges
        TFloat64Matrix4D reversed = TFloat64Matrix4D::Perspective( fov, 1.5, n, f, SProjectionConvention::Make( HANDEDNESS_RIGHT, DEPTHRANGE_ZERO_TO_ONE, true, false, false ) );
        ASSERT_TRUE( fabs( Ndc( reversed, 0, 0, -n ).z - 1.0 ) < 1e-12 && fabs( Ndc( reversed, 0, 0, -f ).z ) < 1e-12 );
        TFloat64Matrix4D reversedGl = TFloat64Matrix4D::Perspective( fov, 1.5, n, f, SProjectionConvention::Make( HANDEDNESS_RIGHT, DEPTHRANGE_MINUS_ONE_TO_ONE, true, false, false ) );
        ASSERT_TRUE( fabs( Ndc( reversedGl, 0, 0, -n ).z - 1.0 ) < 1e-12 && fabs( Ndc( reversedGl, 0, 0, -f ).z + 1.0 ) < 1e-12 );
        // infinite far plane: depth approaches the far value monotonically, the far argument is ignored
        TFloat64Matrix4D infinite = TFloat64Matrix4D::Perspective( fov, 1.5, n, f, SProjectionConvention::ReversedZInfinite( HANDEDNESS_RIGHT ) );
        ASSERT_TRUE( infinite == TFloat64Matrix4D::Perspective( fov, 1.5, n, 1e9, SProjectionConvention::ReversedZInfinite( HANDEDNESS_RIGHT ) ) );
        ASSERT_TRUE( fabs( Ndc( infinite, 0, 0, -n ).z - 1.0 ) < 1e-12 );
        Float64 previous = 1.0;
        bool monotonic = true;
        for ( Float64 d = 1.0; d < 1e12; d *= 10.0 )
        {
            Float64 depth = Ndc( infinite, 0, 0, -d ).z;
            if ( !( depth < previous && depth > 0.0 ) )
                monotonic = false;
            previous = depth;
        }
        ASSERT_TRUE( monotonic && previous < 1e-11 );
        TFloat64Matrix4D infiniteGl = TFloat64Matrix4D::Perspective( fov, 1.5, n, f, SProjectionConvention::Make( HANDEDNESS_RIGHT, DEPTHRANGE_MINUS_ONE_TO_ONE, false, true, false ) );
        ASSERT_TRUE( fabs( Ndc( infiniteGl, 0, 0, -n ).z + 1.0 ) < 1e-12 && fabs( Ndc( infiniteGl, 0, 0, -1e12 ).z - 1.0 ) < 1e-11 );
        // Float32 within 1e-6
        TFloat32Matrix4D gl32 = TFloat32Matrix4D::Perspective( TFloat32Radian( 1.2f ), 1.5f, 0.5f, 200.0f, SProjectionConvention::OpenGL() );
        TFloat32Vector4D clipNear = gl32 * TFloat32Vector4D( 0, 0, -0.5f, 1 );
        ASSERT_TRUE( fabs( clipNear.z / clipNear.w + 1.0f ) < 1e-6f );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Projections: field of view, aspect, off-centre frusta, flipY and orthographic boxes" )
    try
    {
        const Float64 n = 1.0;
        const Float64 f = 100.0;
        const Float64 halfHeight = tan( 0.5 );    // fovY = 1 rad
        TFloat64Matrix4D gl = TFloat64Matrix4D::Perspective( TFloat64Radian( 1.0 ), 2.0, n, f, SProjectionConvention::OpenGL() );
        // the top centre and the right edge of the near plane map to y = 1 and x = 1
        ASSERT_TRUE( fabs( Ndc( gl, 0, halfHeight, -n ).y - 1.0 ) < 1e-12 && fabs( Ndc( gl, 2.0 * halfHeight, 0, -n ).x - 1.0 ) < 1e-12 );
        // flipY inverts clip Y and nothing else
        TFloat64Matrix4D vk = TFloat64Matrix4D::Perspective( TFloat64Radian( 1.0 ), 2.0, n, f, SProjectionConvention::Vulkan() );
        ASSERT_TRUE( fabs( Ndc( vk, 0, halfHeight, -n ).y + 1.0 ) < 1e-12 && fabs( Ndc( vk, 2.0 * halfHeight, 0, -n ).x - 1.0 ) < 1e-12 );
        // a symmetric off-centre frustum equals Perspective
        TFloat64Matrix4D offCentre = TFloat64Matrix4D::PerspectiveOffCenter( -2.0 * halfHeight, 2.0 * halfHeight, -halfHeight, halfHeight, n, f, SProjectionConvention::OpenGL() );
        Float64 largest = 0.0;
        for ( UInt32 r=0; r<4; ++r )
            for ( UInt32 c=0; c<4; ++c )
                largest = fabs( offCentre( r, c ) - gl( r, c ) ) > largest ? fabs( offCentre( r, c ) - gl( r, c ) ) : largest;
        ASSERT_TRUE( largest < 1e-14 );
        // an asymmetric frustum maps its corners to the NDC corners, for both handedness conventions
        TFloat64Matrix4D asym = TFloat64Matrix4D::PerspectiveOffCenter( -1.0, 3.0, -0.5, 2.0, 2.0, 50.0, SProjectionConvention::Vulkan() );
        TFloat64Vector3D corner = Ndc( asym, 3.0, 2.0, -2.0 );
        ASSERT_TRUE( fabs( corner.x - 1.0 ) < 1e-12 && fabs( corner.y + 1.0 ) < 1e-12 && fabs( corner.z ) < 1e-12 );
        corner = Ndc( asym, -1.0 * 25.0, -0.5 * 25.0, -50.0 );
        ASSERT_TRUE( fabs( corner.x + 1.0 ) < 1e-12 && fabs( corner.y - 1.0 ) < 1e-12 && fabs( corner.z - 1.0 ) < 1e-12 );
        TFloat64Matrix4D asymLeft = TFloat64Matrix4D::PerspectiveOffCenter( -1.0, 3.0, -0.5, 2.0, 2.0, 50.0, SProjectionConvention::Direct3D() );
        corner = Ndc( asymLeft, 3.0, 2.0, 2.0 );
        ASSERT_TRUE( fabs( corner.x - 1.0 ) < 1e-12 && fabs( corner.y - 1.0 ) < 1e-12 && fabs( corner.z ) < 1e-12 );
        // orthographic boxes
        TFloat64Matrix4D ortho = TFloat64Matrix4D::Orthographic( -4, 4, -2, 6, 1, 11, SProjectionConvention::OpenGL() );
        TFloat64Vector3D a = Ndc( ortho, 4, 6, -1 );
        TFloat64Vector3D b = Ndc( ortho, -4, -2, -11 );
        ASSERT_TRUE( fabs( a.x - 1.0 ) < 1e-15 && fabs( a.y - 1.0 ) < 1e-15 && fabs( a.z + 1.0 ) < 1e-15 && fabs( b.x + 1.0 ) < 1e-15 && fabs( b.y + 1.0 ) < 1e-15 && fabs( b.z - 1.0 ) < 1e-15 );
        TFloat64Matrix4D orthoDx = TFloat64Matrix4D::Orthographic( -4, 4, -2, 6, 1, 11, SProjectionConvention::Direct3D() );
        ASSERT_TRUE( fabs( Ndc( orthoDx, 0, 0, 1 ).z ) < 1e-15 && fabs( Ndc( orthoDx, 0, 0, 11 ).z - 1.0 ) < 1e-15 );
        TFloat64Matrix4D orthoReversed = TFloat64Matrix4D::Orthographic( -4, 4, -2, 6, 1, 11, SProjectionConvention::Make( HANDEDNESS_RIGHT, DEPTHRANGE_ZERO_TO_ONE, true, false, true ) );
        ASSERT_TRUE( fabs( Ndc( orthoReversed, 0, 0, -1 ).z - 1.0 ) < 1e-15 && fabs( Ndc( orthoReversed, 0, 0, -11 ).z ) < 1e-15 && fabs( Ndc( orthoReversed, 0, 6, -1 ).y + 1.0 ) < 1e-15 );
        ASSERT_TRUE( ortho.IsAffine() && !gl.IsAffine() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Projections: LookAt view matrices for both handedness conventions" )
    try
    {
        // acceptance 11: LookAt( ( 0, 0, 5 ), origin, Up ) maps the origin to ( 0, 0, -5 )
        TFloat64Matrix4D view = TFloat64Matrix4D::LookAt( TFloat64Point3D( 0, 0, 5 ), TFloat64Point3D::Origin(), TFloat64Vector3D::Up(), HANDEDNESS_RIGHT );
        ASSERT_TRUE( TFloat64Point3D( 0, 0, -5 ) == view * TFloat64Point3D::Origin() );
        ASSERT_TRUE( view * TFloat64Point3D( 0, 0, 5 ) == TFloat64Point3D::Origin() );
        // left-handed views look down +Z
        TFloat64Matrix4D left = TFloat64Matrix4D::LookAt( TFloat64Point3D( 0, 0, 5 ), TFloat64Point3D::Origin(), TFloat64Vector3D::Up(), HANDEDNESS_LEFT );
        ASSERT_TRUE( TFloat64Point3D( 0, 0, 5 ) == left * TFloat64Point3D::Origin() );
        // a general view: the target lies on the view axis, up stays up, the matrix is rigid
        TFloat64Point3D eye( 3, 4, -2 );
        TFloat64Point3D target( -1, 0.5, 6 );
        TFloat64Matrix4D general = TFloat64Matrix4D::LookAt( eye, target, TFloat64Vector3D::Up(), HANDEDNESS_RIGHT );
        TFloat64Point3D inView = general * target;
        ASSERT_TRUE( fabs( inView.x ) < 1e-14 && fabs( inView.y ) < 1e-14 && fabs( inView.z + ( target - eye ).GetLength() ) < 1e-13 );
        ASSERT_TRUE( ( general * ( eye + TFloat64Vector3D::Up() ) ).y > 0.0 && fabs( general.GetLinear().GetDeterminant() - 1.0 ) < 1e-14 );
        ASSERT_TRUE( fabs( ( general.InverseRigid() * general ).GetDeterminant() - 1.0 ) < 1e-14 );
        // degenerate input still gives a rigid view
        TFloat64Matrix4D straight = TFloat64Matrix4D::LookAt( eye, eye + TFloat64Vector3D( 0, 3, 0 ), TFloat64Vector3D::Up(), HANDEDNESS_RIGHT );
        ASSERT_TRUE( fabs( straight.GetLinear().GetDeterminant() - 1.0 ) < 1e-14 );
        ASSERT_TRUE( fabs( TFloat64Matrix4D::LookAt( eye, eye, TFloat64Vector3D::Up(), HANDEDNESS_RIGHT ).GetLinear().GetDeterminant() - 1.0 ) < 1e-14 );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Projections: realtime-safe builders run in a realtime context" )
    try
    {
        ASSERT_TRUE( CTNumericTraits< Float32 >::IsFinite( RealtimeProjectionWork( 1.7f ) ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
