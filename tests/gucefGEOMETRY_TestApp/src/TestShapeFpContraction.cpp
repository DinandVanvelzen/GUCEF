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

#ifndef GUCEF_GEOMETRY_SHAPES_H
#include "gucefGEOMETRY_shapes.h"
#define GUCEF_GEOMETRY_SHAPES_H
#endif /* GUCEF_GEOMETRY_SHAPES_H ? */

#include "TestGeometryReference.h"
#include "TestShapeFpContraction.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

/*
 *  Functions marked GUCEF_TEST_CONTRACTING are compiled for FMA with floating-point contraction enabled, whatever the
 *  flags of this file (the test application builds with contraction off): the target attribute allows FMA instructions,
 *  GCC enables contraction per function with the optimize attribute, Clang with the fp pragma inside the body.
 *  Only on x86 with GCC or Clang, and the test runs only when the CPU has FMA.
 */
#if ( defined( __GNUC__ ) || defined( __clang__ ) ) && ( defined( __x86_64__ ) || defined( __i386__ ) )
  #define GUCEF_TEST_CONTRACTION_SUPPORTED
  #if defined( __clang__ )
    #define GUCEF_TEST_CONTRACTING  __attribute__(( target( "fma" ), noinline ))
    #define GUCEF_TEST_CONTRACT_HERE  _Pragma( "clang fp contract(fast)" )
  #else
    #define GUCEF_TEST_CONTRACTING  __attribute__(( target( "fma" ), optimize( "fp-contract=fast" ), noinline ))
    #define GUCEF_TEST_CONTRACT_HERE
  #endif
#endif

using namespace GUCEF;
using namespace GUCEF::MATH;
using namespace GUCEF::GEOMETRY;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The results of the precision-sensitive members for one set of inputs, as raw component values so they compare
 *  bit for bit. Inputs are chosen so that a fused multiply-add changes them: x x' rounds away a low bit that the
 *  following addition would cancel.
 */
template < typename T >
struct SPreciseResults
{
    T values[ 128 ];
    UInt32 count;

    SPreciseResults( void ) : count( 0 ) { memset( values, 0, sizeof( values ) ); }
    void Add( const T& v ) { if ( count < 128 ) values[ count++ ] = v; }
    template < typename V > void Add3( const V& v ) { Add( v.x ); Add( v.y ); Add( v.z ); }
    template < typename V > void Add2( const V& v ) { Add( v.x ); Add( v.y ); }
};

/*-------------------------------------------------------------------------*/

/**
 *  Calls the precision-sensitive members of the shapes (T is Float32 or Float64). A macro, so that each caller compiles
 *  the calls in its own context: with GUCEF_GEOMETRY_INLINE_ALL the member code is then inlined with the caller's
 *  contraction.
 */
#define GUCEF_TEST_CALL_PRECISE_SHAPE_MEMBERS( T, a, c, out )                                                             \
    {                                                                                                                     \
        typedef CTPoint3D< T > P;                                                                                         \
        typedef CTVector3D< T > V;                                                                                        \
        typedef CTDirectionVector3D< T > D;                                                                               \
        typedef CTPoint2D< T > P2;                                                                                        \
        typedef CTVector2D< T > V2;                                                                                       \
        typedef CTDirectionVector2D< T > D2;                                                                              \
        typedef SRayCastResult< T > Hit;                                                                                  \
        const T zero = T( 0 );                                                                                            \
        const T one = T( 1 );                                                                                             \
        out.count = 0;                                                                                                    \
        const CTSegment3D< T > segment( P( -c, zero, zero ), P( a, a, c ) );                                              \
        out.Add3( segment.GetPoint( a ) );                                                                                \
        out.Add( segment.GetClosestParameter( P( a, c, a ) ) );                                                           \
        out.Add3( segment.ClosestPoint( P( c, c, a ) ) );                                                                 \
        const CTRay3D< T > ray( P( a, c, zero ), D( V( one, a, c ) ) );                                                   \
        out.Add3( ray.GetPoint( a ) );                                                                                    \
        const CTLine3D< T > line( P( c, zero, a ), D( V( a, one, -c ) ) );                                                \
        out.Add3( line.GetPoint( c ) );                                                                                   \
        const CTPlane< T > plane = CTPlane< T >::FromPointNormal( P( a, c, a ), D( V( a, c, one ) ) );                    \
        const CTPlane< T > other = CTPlane< T >::FromPointNormal( P( c, a, zero ), D( V( -c, a, one ) ) );                \
        const CTPlane< T > third = CTPlane< T >::FromPointNormal( P( a, zero, c ), D( V( one, -a, c ) ) );                \
        out.Add( plane.GetSignedDistance( P( c, a, c ) ) );                                                               \
        out.Add3( plane.Project( P( c, a, c ) ) );                                                                        \
        out.Add3( plane.Mirror( P( c, a, c ) ) );                                                                         \
        { CTLine3D< T > l; if ( plane.IntersectPlane( other, l ) ) { out.Add3( l.point ); out.Add3( l.direction.AsVector() ); } } \
        { P p; if ( CTPlane< T >::IntersectPlanes( plane, other, third, p ) ) out.Add3( p ); }                            \
        { P p; if ( plane.IntersectLine( line, p ) ) out.Add3( p ); }                                                     \
        { Hit h; out.Add( plane.RayCast( ray, T( 1000 ), h ) ? h.distance : T( -1 ) ); }                                  \
        const CTTriangle3D< T > triangle( P( zero, zero, zero ), P( a, zero, c ), P( zero, c, a ) );                      \
        out.Add3( triangle.ClosestPoint( P( c, a, a ) ) );                                                                \
        out.Add3( triangle.ClosestPoint( P( T( 0.1 ), T( 0.1 ), T( 3 ) ) ) );                                              \
        out.Add( triangle.GetArea() );                                                                                    \
        out.Add3( triangle.GetBarycentric( P( T( 0.1 ), T( 0.1 ), T( 0.2 ) ) ) );                                         \
        { Hit h; out.Add( triangle.RayCast( CTRay3D< T >( P( T( 0.1 ), T( 0.1 ), T( 3 ) ), D( V( zero, zero, -one ) ) ), T( 1000 ), h ) ? h.u + h.v : T( -1 ) ); } \
        const CTSphere< T > sphere( P( a, zero, c ), c );                                                                 \
        out.Add3( sphere.ClosestPoint( P( c, c, a ) ) );                                                                  \
        out.Add( sphere.GetVolume() );                                                                                    \
        out.Add( sphere.GetSurfaceArea() );                                                                               \
        { Hit h; out.Add( sphere.RayCast( CTRay3D< T >( P( -a, c, c ), D( V( one, zero, zero ) ) ), T( 1000 ), h ) ? h.distance : T( -1 ) ); } \
        const CTCapsule< T > capsule( P( zero, zero, zero ), P( a, c, a ), c );                                           \
        out.Add3( capsule.ClosestPoint( P( c, c, a ) ) );                                                                 \
        out.Add( capsule.GetVolume() );                                                                                   \
        out.Add( capsule.GetSurfaceArea() );                                                                              \
        { Hit h; out.Add( capsule.RayCast( CTRay3D< T >( P( -a, c, c ), D( V( one, zero, zero ) ) ), T( 1000 ), h ) ? h.distance : T( -1 ) ); } \
        const CTAxisAlignedBox3D< T > box( P( -a, -c, zero ), P( a, c, a ) );                                             \
        out.Add( box.GetSurfaceArea() );                                                                                  \
        { Hit h; out.Add( box.RayCast( CTRay3D< T >( P( -c, a, -a ), D( V( a, -c, one ) ) ), T( 1000 ), h ) ? h.distance : T( -1 ) ); } \
        CTPolyline3D< T > polyline;                                                                                       \
        polyline.AddPoint( P( zero, zero, zero ) );                                                                       \
        polyline.AddPoint( P( a, c, a ) );                                                                                \
        polyline.AddPoint( P( c, a, c ) );                                                                                \
        out.Add( polyline.GetLength() );                                                                                  \
        out.Add3( polyline.GetPointAtDistance( a ) );                                                                     \
        const CTSegment2D< T > segment2( P2( -c, zero ), P2( a, a ) );                                                    \
        out.Add2( segment2.GetPoint( a ) );                                                                               \
        out.Add( segment2.GetClosestParameter( P2( a, c ) ) );                                                            \
        const CTLine2D< T > line2( P2( c, zero ), D2( V2( a, one ) ) );                                                   \
        out.Add2( line2.GetPoint( c ) );                                                                                  \
        { P2 p; if ( line2.Intersect( CTLine2D< T >( P2( a, a ), D2( V2( -c, a ) ) ), p ) ) out.Add2( p ); }              \
        const CTCircle< T > circle( P2( a, c ), c );                                                                      \
        out.Add( circle.GetArea() );                                                                                      \
        out.Add2( circle.ClosestPoint( P2( c, c ) ) );                                                                    \
        const CTOrientedRectangle< T > oriented( P2( a, c ), CTRadian< T >( a ), V2( c, a ) );                            \
        out.Add2( oriented.ToLocal( P2( c, a ) ) );                                                                       \
        out.Add2( oriented.FromLocal( V2( c, a ) ) );                                                                     \
        out.Add2( oriented.ClosestPoint( P2( c, -a ) ) );                                                                 \
        const CTTriangle2D< T > triangle2( P2( zero, zero ), P2( a, zero ), P2( zero, c ) );                              \
        out.Add( triangle2.GetSignedArea() );                                                                             \
        { MATH::CTVector3D< T > b; if ( triangle2.TryGetBarycentric( P2( T( 0.1 ), T( 0.1 ) ), b ) ) out.Add3( b ); }     \
        out.Add2( triangle2.ClosestPoint( P2( c, c ) ) );                                                                 \
        CTPolygon2D< T > polygon;                                                                                         \
        polygon.AddPoint( P2( zero, zero ) );                                                                             \
        polygon.AddPoint( P2( a, zero ) );                                                                                \
        polygon.AddPoint( P2( a, c ) );                                                                                   \
        polygon.AddPoint( P2( zero, a ) );                                                                                \
        out.Add( polygon.GetSignedArea() );                                                                               \
        out.Add2( polygon.GetCentroid() );                                                                                \
        out.Add( polygon.GetPerimeter() );                                                                                \
        out.Add2( polygon.ClosestPoint( P2( c, -a ) ) );                                                                  \
        CTPolygonWithHoles2D< T > region( polygon );                                                                      \
        region.AddHole( CTPolygon2D< T >() );                                                                             \
        out.Add( region.GetArea() );                                                                                      \
    }

/*-------------------------------------------------------------------------*/

#ifdef GUCEF_TEST_CONTRACTION_SUPPORTED

/** The same calls compiled with the flags of this file (no contraction) */
static void CallPreciseMembersPlain32( Float32 a, Float32 c, SPreciseResults< Float32 >& out ) GUCEF_TEST_CALL_PRECISE_SHAPE_MEMBERS( Float32, a, c, out )
static void CallPreciseMembersPlain64( Float64 a, Float64 c, SPreciseResults< Float64 >& out ) GUCEF_TEST_CALL_PRECISE_SHAPE_MEMBERS( Float64, a, c, out )

/*-------------------------------------------------------------------------*/

/** a * b + c written inline in a contracting function: fused when the compiler contracts */
template < typename T >
GUCEF_TEST_CONTRACTING static T
ContractedMultiplyAdd( T a, T b, T c )
{
    GUCEF_TEST_CONTRACT_HERE
    return a * b + c;
}

/*-------------------------------------------------------------------------*/

GUCEF_TEST_CONTRACTING static void
CallPreciseMembersContracted32( Float32 a, Float32 c, SPreciseResults< Float32 >& out )
{
    GUCEF_TEST_CONTRACT_HERE
    GUCEF_TEST_CALL_PRECISE_SHAPE_MEMBERS( Float32, a, c, out )
}

GUCEF_TEST_CONTRACTING static void
CallPreciseMembersContracted64( Float64 a, Float64 c, SPreciseResults< Float64 >& out )
{
    GUCEF_TEST_CONTRACT_HERE
    GUCEF_TEST_CALL_PRECISE_SHAPE_MEMBERS( Float64, a, c, out )
}

/*-------------------------------------------------------------------------*/

static bool
CpuHasFma( void )
{
    __builtin_cpu_init();
    return 0 != __builtin_cpu_supports( "fma" );
}

#endif /* GUCEF_TEST_CONTRACTION_SUPPORTED ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformShapeFpContractionTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "ShapeFpContraction" );

    GUCEF_TESTFW_TESTCASE( "ShapeFpContraction: exported shape members give the same bits from contracting FMA code" )
    #ifndef GUCEF_TEST_CONTRACTION_SUPPORTED
    GUCEF_TESTFW_SKIP( "contraction test needs GCC or Clang on x86" );
    #else
    GUCEF_TESTFW_SKIP_IF( !CpuHasFma(), "the CPU has no FMA" );

    // a = 1 + 2^-12, c = 1 + 2^-11: a a = 1 + 2^-11 + 2^-24 rounds to c in Float32, so a a - c is 0 unfused and
    // 2^-24 fused. For Float64 a = 1 + 2^-27, c = 1 + 2^-26 (2^-54 is lost). volatile keeps the values opaque.
    volatile Float32 a32 = 1.0f + ldexpf( 1.0f, -12 );
    volatile Float32 c32 = 1.0f + ldexpf( 1.0f, -11 );
    volatile Float64 a64 = 1.0 + ldexp( 1.0, -27 );
    volatile Float64 c64 = 1.0 + ldexp( 1.0, -26 );

    // the probe: the contracting functions of this file must really fuse, otherwise the test cannot detect anything
    // (an unoptimised build does not contract): then it is skipped, not failed
    const Float32 fused32 = ContractedMultiplyAdd< Float32 >( a32, a32, -c32 );
    const Float64 fused64 = ContractedMultiplyAdd< Float64 >( a64, a64, -c64 );
    GUCEF_TESTFW_SKIP_IF( ldexpf( 1.0f, -24 ) != fused32 || ldexp( 1.0, -54 ) != fused64, "this build does not contract a * b + c (no optimisation), the test cannot detect fusion" );
    try
    {
        SPreciseResults< Float32 > contracted32;
        SPreciseResults< Float32 > plain32;
        CallPreciseMembersContracted32( a32, c32, contracted32 );
        CallPreciseMembersPlain32( a32, c32, plain32 );
        SPreciseResults< Float64 > contracted64;
        SPreciseResults< Float64 > plain64;
        CallPreciseMembersContracted64( a64, c64, contracted64 );
        CallPreciseMembersPlain64( a64, c64, plain64 );

        UInt32 differences = 0;
        for ( UInt32 i=0; i<plain32.count; ++i )
            if ( !GeometryReference::SameBits( plain32.values[ i ], contracted32.values[ i ] ) )
                ++differences;
        for ( UInt32 i=0; i<plain64.count; ++i )
            if ( !GeometryReference::SameBits( plain64.values[ i ], contracted64.values[ i ] ) )
                ++differences;
        ASSERT_TRUE( plain32.count == contracted32.count && plain64.count == contracted64.count && plain32.count > 80 );

        #ifndef GUCEF_GEOMETRY_INLINE_ALL
        // the module's copies run in both cases: bit identical
        ASSERT_TRUE( 0 == differences );
        if ( 0 != differences )
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ShapeFpContraction: " << differences << " results differ between contracting and plain callers" );
        #else
        // everything is inlined: the contracting caller fuses, so results may differ (documented, not an error)
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ShapeFpContraction: GUCEF_GEOMETRY_INLINE_ALL build, " << differences << " of " << ( plain32.count + plain64.count ) << " results differ when the caller contracts" );
        #endif
    }
    catch( ... )
    {
        ERRORHERE;
    }
    #endif /* GUCEF_TEST_CONTRACTION_SUPPORTED ? */
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
