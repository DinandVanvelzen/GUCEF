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

#include "TestGeometryReference.h"
#include "TestQueryReference.h"
#include "TestGeometryAllocationCounter.h"
#include "TestQueryFpContraction.h"

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
using namespace QueryReference;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The results of the exported functions for one set of inputs as raw component values, so that they compare bit for bit */
template < typename T >
struct SQueryResults
{
    T values[ 512 ];
    UInt32 count;

    SQueryResults( void ) : count( 0 ) { memset( values, 0, sizeof( values ) ); }
    void Add( const T& v ) { if ( count < 512 ) values[ count++ ] = v; }
    template < typename V > void Add3( const V& v ) { Add( v.x ); Add( v.y ); Add( v.z ); }
    template < typename V > void Add2( const V& v ) { Add( v.x ); Add( v.y ); }
};

/*-------------------------------------------------------------------------*/

/**
 *  Calls the precision-sensitive exported functions of the queries and the new shapes ( T is Float32 or Float64 ). A
 *  macro, so that each caller compiles the calls in its own context: with GUCEF_GEOMETRY_INLINE_ALL the code is then
 *  inlined with the contraction of the caller.
 */
#define GUCEF_TEST_CALL_PRECISE_QUERIES( T, a, c, out )                                                                   \
    {                                                                                                                     \
        typedef CTPoint3D< T > P;                                                                                         \
        typedef CTVector3D< T > V;                                                                                        \
        typedef CTDirectionVector3D< T > D;                                                                               \
        typedef CTPoint2D< T > P2;                                                                                        \
        typedef SRayCastResult< T > Hit;                                                                                  \
        const T zero = T( 0 );                                                                                            \
        const T one = T( 1 );                                                                                             \
        out.count = 0;                                                                                                    \
        const CTSegment3D< T > s1( P( -c, zero, zero ), P( a, a, c ) );                                                   \
        const CTSegment3D< T > s2( P( c, -a, a ), P( a, c, -c ) );                                                        \
        const CTTriangle3D< T > t1( P( zero, zero, zero ), P( a, zero, c ), P( zero, c, a ) );                            \
        const CTTriangle3D< T > t2( P( a, c, -c ), P( c, a, c ), P( -a, a, a ) );                                         \
        const CTSphere< T > sphere( P( a, c, zero ), c );                                                                 \
        const CTCapsule< T > capsule( P( zero, zero, zero ), P( a, c, a ), c );                                           \
        const CTAxisAlignedBox3D< T > box( P( -a, -c, zero ), P( a, c, a ) );                                             \
        const CTOrientedBox3D< T > obb( P( c, a, zero ), CTQuaternion< T >::FromAxisAngle( D( V( a, c, one ) ), CTRadian< T >( c ) ), V( a, c, a ) ); \
        const CTCylinder< T > cylinder( P( zero, zero, zero ), P( a, c, a ), c );                                         \
        const CTCone< T > cone( P( c, zero, a ), D( V( a, one, c ) ), a, c );                                             \
        const CTPlane< T > plane = CTPlane< T >::FromPointNormal( P( a, c, a ), D( V( a, c, one ) ) );                    \
        const CTRay3D< T > ray( P( a, c, zero ), D( V( one, a, c ) ) );                                                   \
        P p1;                                                                                                             \
        P p2;                                                                                                             \
        ClosestPoints( s1, s2, p1, p2 ); out.Add3( p1 ); out.Add3( p2 );                                                  \
        ClosestPoints( s1, t1, p1, p2 ); out.Add3( p1 ); out.Add3( p2 );                                                  \
        ClosestPoints( s1, sphere, p1, p2 ); out.Add3( p1 ); out.Add3( p2 );                                              \
        ClosestPoints( s1, capsule, p1, p2 ); out.Add3( p1 ); out.Add3( p2 );                                             \
        ClosestPoints( s1, plane, p1, p2 ); out.Add3( p1 ); out.Add3( p2 );                                               \
        ClosestPoints( t1, t2, p1, p2 ); out.Add3( p1 ); out.Add3( p2 );                                                  \
        ClosestPoints( t1, sphere, p1, p2 ); out.Add3( p1 ); out.Add3( p2 );                                              \
        ClosestPoints( t1, capsule, p1, p2 ); out.Add3( p1 ); out.Add3( p2 );                                             \
        ClosestPoints( sphere, capsule, p1, p2 ); out.Add3( p1 ); out.Add3( p2 );                                         \
        ClosestPoints( sphere, box, p1, p2 ); out.Add3( p1 ); out.Add3( p2 );                                             \
        ClosestPoints( sphere, obb, p1, p2 ); out.Add3( p1 ); out.Add3( p2 );                                             \
        ClosestPoints( capsule, CTCapsule< T >( P( c, c, c ), P( a, -a, c ), a ), p1, p2 ); out.Add3( p1 ); out.Add3( p2 ); \
        ClosestPoints( plane, cone, p1, p2 ); out.Add3( p1 ); out.Add3( p2 );                                             \
        ClosestPoints( plane, cylinder, p1, p2 ); out.Add3( p1 ); out.Add3( p2 );                                         \
        out.Add( Intersects( s1, box ) ? one : zero );                                                                    \
        out.Add( Intersects( s1, obb ) ? one : zero );                                                                    \
        out.Add( Intersects( t1, box ) ? one : zero );                                                                    \
        out.Add( Intersects( t1, obb ) ? one : zero );                                                                    \
        out.Add( Intersects( obb, CTOrientedBox3D< T >( P( a, a, c ), CTQuaternion< T >::Identity(), V( a, a, a ) ) ) ? one : zero ); \
        out.Add( Intersects( box, obb ) ? one : zero );                                                                   \
        { T lo = zero; T hi = zero; GetSignedDistanceRange( plane, obb, lo, hi ); out.Add( lo ); out.Add( hi ); }         \
        { T lo = zero; T hi = zero; GetSignedDistanceRange( plane, sphere, lo, hi ); out.Add( lo ); out.Add( hi ); }      \
        { Hit h; out.Add( RayCastTriangleWatertight( ray, t1, T( 1000 ), h ) ? h.distance : T( -1 ) ); out.Add( h.u ); out.Add( h.v ); } \
        { Hit h; out.Add( obb.RayCast( ray, T( 1000 ), h ) ? h.distance : T( -1 ) ); out.Add3( h.normal ); }              \
        { Hit h; out.Add( cylinder.RayCast( ray, T( 1000 ), h ) ? h.distance : T( -1 ) ); out.Add3( h.normal ); }         \
        { Hit h; out.Add( cone.RayCast( ray, T( 1000 ), h ) ? h.distance : T( -1 ) ); out.Add3( h.normal ); }             \
        out.Add3( obb.ClosestPoint( P( a, -c, a ) ) );                                                                    \
        out.Add3( cylinder.ClosestPoint( P( a, -c, a ) ) );                                                               \
        out.Add3( cone.ClosestPoint( P( a, -c, a ) ) );                                                                   \
        out.Add3( obb.GetSupport( V( a, c, -a ) ) );                                                                      \
        out.Add3( cylinder.GetSupport( V( a, c, -a ) ) );                                                                 \
        out.Add3( cone.GetSupport( V( a, c, -a ) ) );                                                                     \
        out.Add( obb.GetVolume() ); out.Add( cylinder.GetVolume() ); out.Add( cone.GetVolume() );                         \
        out.Add( obb.GetSurfaceArea() ); out.Add( cylinder.GetSurfaceArea() ); out.Add( cone.GetSurfaceArea() );          \
        out.Add3( obb.GetBounds().max ); out.Add3( cylinder.GetBounds().max ); out.Add3( cone.GetBounds().max );          \
        {                                                                                                                 \
            const SProjectionConvention convention = SProjectionConvention::OpenGL();                                     \
            const CTMatrix4D< T > projection = CTMatrix4D< T >::Perspective( CTRadian< T >( c ), a, c, a + a + a, convention ); \
            const CTFrustum< T > frustum = CTFrustum< T >::FromViewProjection( projection, convention );                  \
            for ( UInt32 i=0; i<6; ++i ) { out.Add3( frustum.GetPlane( i ).normal.AsVector() ); out.Add( frustum.GetPlane( i ).offset ); } \
            for ( UInt32 i=0; i<8; ++i ) out.Add3( frustum.GetCorner( i ) );                                              \
            out.Add( frustum.GetVolume() );                                                                               \
            out.Add( T( frustum.Classify( sphere ) ) ); out.Add( T( frustum.Classify( box ) ) );                          \
            CTPlaneBoundedVolume< T > volume = CTPlaneBoundedVolume< T >::FromFrustum( frustum );                         \
            const CTPoint3D< T > poly[ 3 ] = { P( -a, -a, -a ), P( a, zero, -a - a ), P( zero, a, -c ) };                  \
            CTPoint3D< T > clipped[ 16 ]; CTPoint3D< T > scratch[ 16 ]; UInt32 clippedCount = 0;                          \
            volume.ClipPolygon( poly, 3, clipped, scratch, 16, clippedCount );                                            \
            out.Add( T( clippedCount ) );                                                                                 \
            for ( UInt32 i=0; i<clippedCount && i<16; ++i ) out.Add3( clipped[ i ] );                                     \
            out.Add( Intersects( frustum, t1 ) ? one : zero );                                                            \
            out.Add( Intersects( frustum, s1 ) ? one : zero );                                                            \
            out.Add( volume.IntersectsExact( sphere ) ? one : zero );                                                     \
            GUCEF::vector< P > vertices; GUCEF::vector< SConvexBodyFace > faces;                                          \
            if ( volume.BuildConvexBody( vertices, faces ) ) { out.Add( T( vertices.size() ) ); for ( size_t i=0; i<vertices.size(); ++i ) out.Add3( vertices[ i ] ); } \
        }                                                                                                                 \
        {                                                                                                                 \
            const CTSegment2D< T > g1( P2( -c, zero ), P2( a, a ) );                                                      \
            const CTSegment2D< T > g2( P2( a, -c ), P2( -c, a ) );                                                        \
            const CTCircle< T > circle( P2( a, c ), c );                                                                  \
            const CTOrientedRectangle< T > oriented( P2( a, c ), CTRadian< T >( a ), CTVector2D< T >( c, a ) );          \
            P2 q1; P2 q2;                                                                                                 \
            ClosestPoints( g1, g2, q1, q2 ); out.Add2( q1 ); out.Add2( q2 );                                              \
            ClosestPoints( g1, circle, q1, q2 ); out.Add2( q1 ); out.Add2( q2 );                                          \
            ClosestPoints( circle, oriented, q1, q2 ); out.Add2( q1 ); out.Add2( q2 );                                    \
            ClosestPoints( oriented, CTRectangle< T >( P2( -a, -a ), P2( zero, c ) ), q1, q2 ); out.Add2( q1 ); out.Add2( q2 ); \
            SSegmentIntersection2D< T > crossing;                                                                         \
            out.Add( T( IntersectSegments( g1, g2, crossing ) ) ); out.Add2( crossing.first ); out.Add( crossing.t );     \
            out.Add( Intersects( oriented, CTTriangle2D< T >( P2( zero, zero ), P2( a, zero ), P2( zero, c ) ) ) ? one : zero ); \
        }                                                                                                                 \
    }

/*-------------------------------------------------------------------------*/

#ifdef GUCEF_TEST_CONTRACTION_SUPPORTED

static void CallPreciseQueriesPlain32( Float32 a, Float32 c, SQueryResults< Float32 >& out ) GUCEF_TEST_CALL_PRECISE_QUERIES( Float32, a, c, out )
static void CallPreciseQueriesPlain64( Float64 a, Float64 c, SQueryResults< Float64 >& out ) GUCEF_TEST_CALL_PRECISE_QUERIES( Float64, a, c, out )

/** a * b + c written inline in a contracting function: fused when the compiler contracts */
template < typename T >
GUCEF_TEST_CONTRACTING static T
ContractedMultiplyAdd( T a, T b, T c )
{
    GUCEF_TEST_CONTRACT_HERE
    return a * b + c;
}

GUCEF_TEST_CONTRACTING static void
CallPreciseQueriesContracted32( Float32 a, Float32 c, SQueryResults< Float32 >& out )
{
    GUCEF_TEST_CONTRACT_HERE
    GUCEF_TEST_CALL_PRECISE_QUERIES( Float32, a, c, out )
}

GUCEF_TEST_CONTRACTING static void
CallPreciseQueriesContracted64( Float64 a, Float64 c, SQueryResults< Float64 >& out )
{
    GUCEF_TEST_CONTRACT_HERE
    GUCEF_TEST_CALL_PRECISE_QUERIES( Float64, a, c, out )
}

static bool
CpuHasFma( void )
{
    __builtin_cpu_init();
    return 0 != __builtin_cpu_supports( "fma" );
}

#endif /* GUCEF_TEST_CONTRACTION_SUPPORTED ? */

/*-------------------------------------------------------------------------*/

/** The realtime-safe queries on prepared objects: the allocation counter must stay at 0 over all of them */
template < typename T >
static void
CallRealtimeQueries( UInt32& accumulator )
{
    typedef CTPoint3D< T > P;
    typedef CTVector3D< T > V;
    typedef CTDirectionVector3D< T > D;
    typedef SRayCastResult< T > Hit;
    const CTSegment3D< T > segment( P( -1, 0, 0 ), P( 2, 1, 1 ) );
    const CTTriangle3D< T > triangle( P( 0, 0, 0 ), P( 2, 0, 1 ), P( 0, 2, 1 ) );
    const CTSphere< T > sphere( P( 1, 1, 0 ), (T) 0.8 );
    const CTCapsule< T > capsule( P( 0, 0, 0 ), P( 1, 2, 1 ), (T) 0.5 );
    const CTAxisAlignedBox3D< T > box( P( -1, -1, -1 ), P( 1, 1, 1 ) );
    const CTOrientedBox3D< T > obb( P( 0, 1, 0 ), CTQuaternion< T >::FromAxisAngle( D( V( 1, 1, 0 ) ), CTRadian< T >( (T) 0.7 ) ), V( 1, 2, 1 ) );
    const CTCylinder< T > cylinder( P( 0, 0, 0 ), P( 1, 2, 1 ), (T) 0.6 );
    const CTCone< T > cone( P( 0, 0, 0 ), D( V( 0, 1, 1 ) ), 2, 1 );
    const CTPlane< T > plane = CTPlane< T >::FromPointNormal( P( 0, (T) 0.5, 0 ), D( V( 0, 1, 0 ) ) );
    const CTRay3D< T > ray( P( -4, (T) 0.3, (T) 0.2 ), D( V( 1, 0, 0 ) ) );
    const SProjectionConvention convention = SProjectionConvention::Vulkan();
    const CTFrustum< T > frustum = CTFrustum< T >::FromViewProjection( CTMatrix4D< T >::Perspective( CTRadian< T >( 1 ), 1, 1, 10, convention ), convention );
    CTPlaneBoundedVolume< T > volume = CTPlaneBoundedVolume< T >::FromAxisAlignedBox( box );
    P p1;
    P p2;
    Hit hit;
    UInt32 bits = 0;
    bits += ClosestPoints( segment, triangle, p1, p2 ) ? 1 : 0;
    bits += ClosestPoints( triangle, capsule, p1, p2 ) ? 1 : 0;
    bits += ClosestPoints( sphere, obb, p1, p2 ) ? 1 : 0;
    bits += ClosestPoints( plane, cone, p1, p2 ) ? 1 : 0;
    bits += Intersects( segment, obb ) ? 1 : 0;
    bits += Intersects( triangle, box ) ? 1 : 0;
    bits += Intersects( obb, box ) ? 1 : 0;
    bits += Intersects( sphere, capsule ) ? 1 : 0;
    bits += Intersects( frustum, triangle ) ? 1 : 0;
    bits += Intersects( volume, segment ) ? 1 : 0;
    bits += Intersects( frustum, cylinder ) ? 1 : 0;
    bits += Contains( box, obb ) ? 1 : 0;
    bits += Contains( volume, sphere ) ? 1 : 0;
    bits += RayCastTriangleWatertight( ray, triangle, (T) 100, hit ) ? 1 : 0;
    bits += obb.RayCast( ray, (T) 100, hit ) ? 1 : 0;
    bits += cylinder.RayCast( ray, (T) 100, hit ) ? 1 : 0;
    bits += cone.RayCast( ray, (T) 100, hit ) ? 1 : 0;
    bits += frustum.RayCast( ray, (T) 100, hit ) ? 1 : 0;
    bits += volume.RayCast( ray, (T) 100, hit ) ? 1 : 0;
    bits += obb.Contains( p1 ) ? 1 : 0;
    bits += (UInt32) frustum.Classify( box );
    bits += (UInt32) volume.Classify( sphere );
    const P poly[ 3 ] = { P( -2, -2, 0 ), P( 2, -2, 0 ), P( 0, 2, 0 ) };
    P clipped[ 16 ];
    P scratch[ 16 ];
    UInt32 count = 0;
    volume.ClipPolygon( poly, 3, clipped, scratch, 16, count );
    bits += count;
    accumulator += bits;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformQueryFpContractionTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "QueryFpContraction" );

    GUCEF_TESTFW_TESTCASE( "QueryFpContraction: exported query functions and volume shape members give the same bits from contracting FMA code" )
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
    const Float32 fused32 = ContractedMultiplyAdd< Float32 >( a32, a32, -c32 );
    const Float64 fused64 = ContractedMultiplyAdd< Float64 >( a64, a64, -c64 );
    GUCEF_TESTFW_SKIP_IF( ldexpf( 1.0f, -24 ) != fused32 || ldexp( 1.0, -54 ) != fused64, "this build does not contract a * b + c (no optimisation), the test cannot detect fusion" );
    try
    {
        SQueryResults< Float32 > contracted32;
        SQueryResults< Float32 > plain32;
        CallPreciseQueriesContracted32( a32, c32, contracted32 );
        CallPreciseQueriesPlain32( a32, c32, plain32 );
        SQueryResults< Float64 > contracted64;
        SQueryResults< Float64 > plain64;
        CallPreciseQueriesContracted64( a64, c64, contracted64 );
        CallPreciseQueriesPlain64( a64, c64, plain64 );

        UInt32 differences = 0;
        for ( UInt32 i=0; i<plain32.count; ++i )
            if ( !GeometryReference::SameBits( plain32.values[ i ], contracted32.values[ i ] ) )
                ++differences;
        for ( UInt32 i=0; i<plain64.count; ++i )
            if ( !GeometryReference::SameBits( plain64.values[ i ], contracted64.values[ i ] ) )
                ++differences;
        ASSERT_TRUE( plain32.count == contracted32.count && plain64.count == contracted64.count && plain32.count > 200 );

        #ifndef GUCEF_GEOMETRY_INLINE_ALL
        // the module's copies run in both cases: bit identical
        ASSERT_TRUE( 0 == differences );
        if ( 0 != differences )
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "QueryFpContraction: " << differences << " results differ between contracting and plain callers" );
        #else
        // everything is inlined: the contracting caller fuses, so results may differ (documented, not an error)
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "QueryFpContraction: GUCEF_GEOMETRY_INLINE_ALL build, " << differences << " of " << ( plain32.count + plain64.count ) << " results differ when the caller contracts" );
        #endif
    }
    catch( ... )
    {
        ERRORHERE;
    }
    #endif /* GUCEF_TEST_CONTRACTION_SUPPORTED ? */
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "QueryFpContraction: the realtime-safe queries and the allocation-free members do not allocate" )
    GUCEF_TESTFW_SKIP_IF( !GeometryAllocationCounter::IsAvailable(), "no allocation counter with a sanitizer that replaces the allocator (the RealtimeSanitizer build proves the same in a realtime context)" );
    try
    {
        // the counter counts: a probe allocation is seen
        GeometryAllocationCounter::StartCounting();
        GeometryAllocationCounter::AllocateProbe();
        ASSERT_TRUE( 1 == GeometryAllocationCounter::StopCounting() );

        UInt32 accumulator = 0;
        CallRealtimeQueries< Float32 >( accumulator );          // warm: first use of any lazily initialised state happens here
        CallRealtimeQueries< Float64 >( accumulator );
        GeometryAllocationCounter::StartCounting();
        CallRealtimeQueries< Float32 >( accumulator );
        CallRealtimeQueries< Float64 >( accumulator );
        const UInt32 allocations = GeometryAllocationCounter::StopCounting();
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "QueryFpContraction: " << allocations << " allocations in the realtime queries (" << accumulator << ")" );
        ASSERT_TRUE( 0 == allocations );
        ASSERT_TRUE( accumulator > 20 );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
