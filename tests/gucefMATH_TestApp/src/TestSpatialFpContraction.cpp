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

#ifndef GUCEF_MATH_CTPOLARCOORDINATES_H
#include "gucefMATH_CTPolarCoordinates.h"
#define GUCEF_MATH_CTPOLARCOORDINATES_H
#endif /* GUCEF_MATH_CTPOLARCOORDINATES_H ? */

#ifndef GUCEF_MATH_CTSPHERICALCOORDINATES_H
#include "gucefMATH_CTSphericalCoordinates.h"
#define GUCEF_MATH_CTSPHERICALCOORDINATES_H
#endif /* GUCEF_MATH_CTSPHERICALCOORDINATES_H ? */

#ifndef GUCEF_MATH_CTORTHONORMALFRAME3D_H
#include "gucefMATH_CTOrthonormalFrame3D.h"
#define GUCEF_MATH_CTORTHONORMALFRAME3D_H
#endif /* GUCEF_MATH_CTORTHONORMALFRAME3D_H ? */

#ifndef GUCEF_MATH_SPATIALAUDIOVECTORFUNCTIONS_H
#include "gucefMATH_SpatialAudioVectorFunctions.h"
#define GUCEF_MATH_SPATIALAUDIOVECTORFUNCTIONS_H
#endif /* GUCEF_MATH_SPATIALAUDIOVECTORFUNCTIONS_H ? */

#ifndef GUCEF_MATH_CTVBAP2D_H
#include "gucefMATH_CTVbap2D.h"
#define GUCEF_MATH_CTVBAP2D_H
#endif /* GUCEF_MATH_CTVBAP2D_H ? */

#ifndef GUCEF_MATH_CTVBAP3D_H
#include "gucefMATH_CTVbap3D.h"
#define GUCEF_MATH_CTVBAP3D_H
#endif /* GUCEF_MATH_CTVBAP3D_H ? */

#include "TestMathReference.h"
#include "TestSpatialFpContraction.h"

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

/*
 *  Functions marked GUCEF_TEST_CONTRACTING are compiled for FMA with floating-point contraction enabled, whatever the
 *  flags of this file (the test application builds with contraction off): the target attribute allows FMA instructions,
 *  GCC enables contraction per function with the optimize attribute, Clang with the fp pragma inside the body.
 *  Only on x86 with GCC or Clang, and the test runs only when the CPU has FMA.
 */
#if ( defined( __GNUC__ ) || defined( __clang__ ) ) && ( defined( __x86_64__ ) || defined( __i386__ ) )
  #define GUCEF_TEST_SPATIAL_CONTRACTION_SUPPORTED
  #if defined( __clang__ )
    #define GUCEF_TEST_SPATIAL_CONTRACTING  __attribute__(( target( "fma" ), noinline ))
    #define GUCEF_TEST_SPATIAL_CONTRACT_HERE  _Pragma( "clang fp contract(fast)" )
  #else
    #define GUCEF_TEST_SPATIAL_CONTRACTING  __attribute__(( target( "fma" ), optimize( "fp-contract=fast" ), noinline ))
    #define GUCEF_TEST_SPATIAL_CONTRACT_HERE
  #endif
#endif

/*-------------------------------------------------------------------------*/

template < typename T >
static bool
SameBits( const T& a, const T& b )
{
    return 0 == memcmp( &a, &b, sizeof( T ) );
}

/*-------------------------------------------------------------------------*/

/** The results of the precision-sensitive members for one set of inputs, as raw component values so they compare bit for bit */
template < typename T >
struct SSpatialPreciseResults
{
    T values[ 300 ];
    UInt32 count;

    void Add( const T& v ) { if ( count < 300 ) values[ count++ ] = v; }
    template < typename V > void Add3( const V& v ) { Add( v.x ); Add( v.y ); Add( v.z ); }
};

/*-------------------------------------------------------------------------*/

/**
 *  Calls exported precision-sensitive members of the spherical, frame, projection and panning classes (T is Float32 or
 *  Float64). A macro, so that each caller compiles the calls in its own context: with GUCEF_MATH_INLINE_ALL the member code
 *  is then inlined with the caller's contraction. The inputs make a fused multiply-add observable.
 */
#define GUCEF_TEST_CALL_SPATIAL_PRECISE_MEMBERS( T, a, c, out )                                                           \
    {                                                                                                                     \
        typedef CTVector3D< T > V;                                                                                        \
        typedef CTPoint3D< T > P;                                                                                         \
        typedef CTMatrix4D< T > M4;                                                                                       \
        out.count = 0;                                                                                                    \
        CTPolarCoordinates< T > polar = CTPolarCoordinates< T >::FromCartesian( CTVector2D< T >( a, c ) );                \
        out.Add( polar.radius ); out.Add( polar.angle.GetValue() );                                                       \
        CTVector2D< T > cartesian = CTPolarCoordinates< T >( a, CTRadian< T >( c ) ).ToCartesian();                       \
        out.Add( cartesian.x ); out.Add( cartesian.y );                                                                   \
        CTSphericalCoordinates< T > spherical = CTSphericalCoordinates< T >::FromFrameCoordinates( a, T( 1 ), c );        \
        out.Add( spherical.radius ); out.Add( spherical.azimuth.GetValue() ); out.Add( spherical.elevation.GetValue() );  \
        out.Add3( spherical.ToFrameCoordinates() );                                                                       \
        CTRadian< T > theta;                                                                                              \
        CTRadian< T > phi;                                                                                                \
        spherical.ToMathematical( theta, phi );                                                                           \
        out.Add( theta.GetValue() ); out.Add( phi.GetValue() );                                                           \
        CTSphericalCoordinates< T > fromMath = CTSphericalCoordinates< T >::FromMathematical( a, theta, phi );            \
        out.Add( fromMath.radius ); out.Add( fromMath.azimuth.GetValue() ); out.Add( fromMath.elevation.GetValue() );     \
        CTOrthonormalFrame3D< T > frame;                                                                                  \
        CTOrthonormalFrame3D< T >::TryFromForwardUp( V( a, T( 1 ), -c ), V( c, a, T( 1 ) ), frame );                      \
        out.Add( frame.GetRight().GetX() ); out.Add( frame.GetUp().GetY() ); out.Add( frame.GetForward().GetZ() );        \
        CTOrthonormalFrame3D< T > leftFrame;                                                                              \
        CTOrthonormalFrame3D< T >::TryFromForwardUp( V( c, a, T( 1 ) ), V( a, T( 1 ), c ), MATH::HANDEDNESS_LEFT, leftFrame );   \
        out.Add( leftFrame.GetRight().GetX() ); out.Add( leftFrame.GetUp().GetY() ); out.Add( leftFrame.GetForward().GetZ() ); \
        V world( a, -c, T( 2 ) );                                                                                         \
        out.Add3( frame.ToFrameCoordinates( world ) ); out.Add3( frame.FromFrameCoordinates( world ) );                   \
        out.Add( frame.GetAzimuth( world ).GetValue() ); out.Add( frame.GetElevation( world ).GetValue() );               \
        CTSphericalCoordinates< T > viewed = frame.ToSpherical( world );                                                  \
        out.Add( viewed.radius ); out.Add( viewed.azimuth.GetValue() ); out.Add( viewed.elevation.GetValue() );           \
        out.Add( DopplerRatio( P( a, c, T( 0 ) ), V( c, a, T( 3 ) ), P( -c, T( 5 ), a ), V( a, -c, T( 7 ) ), T( 343.3 ), T( 1 ), T( 0.5 ), T( 2 ) ) ); \
        SProjectionConvention convention = SProjectionConvention::Vulkan();                                               \
        M4 projection = M4::Perspective( CTRadian< T >( a ), c, T( 0.5 ), T( 100 ), convention );                         \
        M4 view = M4::LookAt( P( a, T( 1 ), c ), P( c, T( 0 ), a ), V::Up(), MATH::HANDEDNESS_RIGHT );                           \
        M4 viewProjection = projection * view;                                                                            \
        M4 inverse;                                                                                                       \
        viewProjection.TryInverse( inverse, T( 0 ) );                                                                     \
        CTViewport< T > viewport = CTViewport< T >::FromConvention( a, c, T( 1920 ), T( 1080 ), convention );             \
        CTPoint2D< T > screen;                                                                                            \
        T depth = T( 0 );                                                                                                 \
        CTPoint3D< T > unprojected;                                                                                       \
        CTPoint3D< T > rayOrigin;                                                                                         \
        CTDirectionVector3D< T > rayDirection;                                                                            \
        Project( P( a, c, T( -3 ) ), viewProjection, viewport, screen, depth );                                           \
        out.Add( screen.x ); out.Add( screen.y ); out.Add( depth );                                                       \
        Unproject( CTPoint2D< T >( T( 900 ), T( 500 ) ), T( 0.5 ), inverse, viewport, unprojected );                      \
        out.Add3( unprojected );                                                                                          \
        GetPickRay( CTPoint2D< T >( T( 700 ), T( 300 ) ), inverse, viewport, convention, rayOrigin, rayDirection );        \
        out.Add3( rayOrigin ); out.Add( rayDirection.GetX() ); out.Add( rayDirection.GetY() ); out.Add( rayDirection.GetZ() ); \
        const CTRadian< T > ring[ 5 ] = { CTRadian< T >( -T( 2.2 ) * a ), CTRadian< T >( -T( 0.9 ) * c ), CTRadian< T >( T( 0.1 ) * a ), CTRadian< T >( T( 0.8 ) * c ), CTRadian< T >( T( 2 ) * a ) }; \
        CTVbap2D< T > panner2D;                                                                                           \
        out.Add( panner2D.Prepare( ring, 5 ) ? T( 1 ) : T( 0 ) );                                                         \
        T gains2D[ 5 ] = { T( 0 ), T( 0 ), T( 0 ), T( 0 ), T( 0 ) };                                                      \
        panner2D.ComputeGains( CTRadian< T >( T( 0.3 ) * c ), gains2D, T( 2 ) );                                          \
        for ( UInt32 i = 0; i < 5; ++i ) out.Add( gains2D[ i ] );                                                         \
        panner2D.ComputeGains( CTRadian< T >( T( 3 ) ), gains2D, T( 2 ) );                                                \
        for ( UInt32 i = 0; i < 5; ++i ) out.Add( gains2D[ i ] );                                                         \
        const CTRadian< T > domeAzimuth[ 6 ] = { CTRadian< T >( -a ), CTRadian< T >( c ), CTRadian< T >( T( 2 ) * a ), CTRadian< T >( -c * T( 2 ) ), CTRadian< T >( T( 0.2 ) * a ), CTRadian< T >( T( 3 ) ) }; \
        const CTRadian< T > domeElevation[ 6 ] = { CTRadian< T >( T( 0.1 ) * a ), CTRadian< T >( T( 0.2 ) * c ), CTRadian< T >( T( 0.3 ) ), CTRadian< T >( T( 0.35 ) * a ), CTRadian< T >( T( 0.9 ) ), CTRadian< T >( T( -0.4 ) * c ) }; \
        CTVbap3D< T > panner3D;                                                                                           \
        out.Add( panner3D.Prepare( domeAzimuth, domeElevation, 6 ) ? T( 1 ) : T( 0 ) );                                   \
        T gains3D[ 6 ] = { T( 0 ), T( 0 ), T( 0 ), T( 0 ), T( 0 ), T( 0 ) };                                              \
        panner3D.ComputeGains( CTRadian< T >( T( 0.7 ) * a ), CTRadian< T >( T( 0.2 ) * c ), gains3D, T( 2 ) );           \
        for ( UInt32 i = 0; i < 6; ++i ) out.Add( gains3D[ i ] );                                                         \
        panner3D.ComputeGains( CTDirectionVector3D< T >( V( c, -a, T( 0.3 ) ) ), gains3D, T( 2 ) );                       \
        for ( UInt32 i = 0; i < 6; ++i ) out.Add( gains3D[ i ] );                                                         \
    }

/*-------------------------------------------------------------------------*/

#ifdef GUCEF_TEST_SPATIAL_CONTRACTION_SUPPORTED

/** The same calls compiled with the flags of this file (no contraction) */
static void CallSpatialMembersPlain32( Float32 a, Float32 c, SSpatialPreciseResults< Float32 >& out ) GUCEF_TEST_CALL_SPATIAL_PRECISE_MEMBERS( Float32, a, c, out )
static void CallSpatialMembersPlain64( Float64 a, Float64 c, SSpatialPreciseResults< Float64 >& out ) GUCEF_TEST_CALL_SPATIAL_PRECISE_MEMBERS( Float64, a, c, out )

/*-------------------------------------------------------------------------*/

/** a * b + c written inline in a contracting function: fused when the compiler contracts */
template < typename T >
GUCEF_TEST_SPATIAL_CONTRACTING static T
SpatialContractedMultiplyAdd( T a, T b, T c )
{
    GUCEF_TEST_SPATIAL_CONTRACT_HERE
    return a * b + c;
}

/*-------------------------------------------------------------------------*/

GUCEF_TEST_SPATIAL_CONTRACTING static void
CallSpatialMembersContracted32( Float32 a, Float32 c, SSpatialPreciseResults< Float32 >& out )
{
    GUCEF_TEST_SPATIAL_CONTRACT_HERE
    GUCEF_TEST_CALL_SPATIAL_PRECISE_MEMBERS( Float32, a, c, out )
}

GUCEF_TEST_SPATIAL_CONTRACTING static void
CallSpatialMembersContracted64( Float64 a, Float64 c, SSpatialPreciseResults< Float64 >& out )
{
    GUCEF_TEST_SPATIAL_CONTRACT_HERE
    GUCEF_TEST_CALL_SPATIAL_PRECISE_MEMBERS( Float64, a, c, out )
}

/*-------------------------------------------------------------------------*/

static bool
SpatialCpuHasFma( void )
{
    __builtin_cpu_init();
    return 0 != __builtin_cpu_supports( "fma" );
}

#endif /* GUCEF_TEST_SPATIAL_CONTRACTION_SUPPORTED ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformSpatialFpContractionTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "SpatialFpContraction" );

    GUCEF_TESTFW_TESTCASE( "SpatialFpContraction: exported spherical, frame, projection, Doppler and panning members give the same bits from contracting FMA code" )
    #ifndef GUCEF_TEST_SPATIAL_CONTRACTION_SUPPORTED
    GUCEF_TESTFW_SKIP( "contraction test needs GCC or Clang on x86" );
    #else
    GUCEF_TESTFW_SKIP_IF( !SpatialCpuHasFma(), "the CPU has no FMA" );

    // a = 1 + 2^-12, c = 1 + 2^-11: a a = 1 + 2^-11 + 2^-24 rounds to c in Float32, so a a - c is 0 unfused and
    // 2^-24 fused. For Float64 a = 1 + 2^-27, c = 1 + 2^-26 (2^-54 is lost). volatile keeps the values opaque.
    volatile Float32 a32 = 1.0f + ldexpf( 1.0f, -12 );
    volatile Float32 c32 = 1.0f + ldexpf( 1.0f, -11 );
    volatile Float64 a64 = 1.0 + ldexp( 1.0, -27 );
    volatile Float64 c64 = 1.0 + ldexp( 1.0, -26 );

    // the probe: the contracting functions of this file must really fuse, otherwise the test cannot detect anything
    // (an unoptimised build does not contract): then it is skipped, not failed
    const Float32 fused32 = SpatialContractedMultiplyAdd< Float32 >( a32, a32, -c32 );
    const Float64 fused64 = SpatialContractedMultiplyAdd< Float64 >( a64, a64, -c64 );
    GUCEF_TESTFW_SKIP_IF( ldexpf( 1.0f, -24 ) != fused32 || ldexp( 1.0, -54 ) != fused64, "this build does not contract a * b + c (no optimisation), the test cannot detect fusion" );
    try
    {
        SSpatialPreciseResults< Float32 > contracted32;
        SSpatialPreciseResults< Float32 > plain32;
        CallSpatialMembersContracted32( a32, c32, contracted32 );
        CallSpatialMembersPlain32( a32, c32, plain32 );
        SSpatialPreciseResults< Float64 > contracted64;
        SSpatialPreciseResults< Float64 > plain64;
        CallSpatialMembersContracted64( a64, c64, contracted64 );
        CallSpatialMembersPlain64( a64, c64, plain64 );

        UInt32 differences = 0;
        for ( UInt32 i=0; i<plain32.count; ++i )
        {
            if ( !SameBits( plain32.values[ i ], contracted32.values[ i ] ) )
            {
                ++differences;
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "SpatialFpContraction: Float32 result " << i << " differs: " << plain32.values[ i ] << " and " << contracted32.values[ i ] );
            }
        }
        for ( UInt32 i=0; i<plain64.count; ++i )
        {
            if ( !SameBits( plain64.values[ i ], contracted64.values[ i ] ) )
            {
                ++differences;
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "SpatialFpContraction: Float64 result " << i << " differs: " << plain64.values[ i ] << " and " << contracted64.values[ i ] );
            }
        }
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "SpatialFpContraction: " << plain32.count << " Float32 and " << plain64.count << " Float64 results compared" );
        ASSERT_TRUE( plain32.count == contracted32.count && plain64.count == contracted64.count && plain32.count == plain64.count && plain32.count > 60 && 1 == plain32.values[ 45 ] && 1 == plain64.values[ 45 ] && 1 == plain32.values[ 56 ] && 1 == plain64.values[ 56 ] );

        #ifndef GUCEF_MATH_INLINE_ALL
        // the module's copies run in both cases: bit identical
        ASSERT_TRUE( 0 == differences );
        if ( 0 != differences )
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "SpatialFpContraction: " << differences << " results differ between contracting and plain callers" );
        #else
        // everything is inlined: the contracting caller fuses, so results may differ (documented, not an error)
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "SpatialFpContraction: GUCEF_MATH_INLINE_ALL build, " << differences << " of " << ( plain32.count + plain64.count ) << " results differ when the caller contracts" );
        #endif
    }
    catch( ... )
    {
        ERRORHERE;
    }
    #endif /* GUCEF_TEST_SPATIAL_CONTRACTION_SUPPORTED ? */
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
