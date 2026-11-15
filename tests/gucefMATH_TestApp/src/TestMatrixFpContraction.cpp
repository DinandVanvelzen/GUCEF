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

#ifndef GUCEF_MATH_QUATERNIONS_H
#include "gucefMATH_quaternions.h"
#define GUCEF_MATH_QUATERNIONS_H
#endif /* GUCEF_MATH_QUATERNIONS_H ? */

#ifndef GUCEF_MATH_TRANSFORMS_H
#include "gucefMATH_transforms.h"
#define GUCEF_MATH_TRANSFORMS_H
#endif /* GUCEF_MATH_TRANSFORMS_H ? */

#include "TestMathReference.h"
#include "TestMatrixFpContraction.h"

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
  #define GUCEF_TEST_MATRIX_CONTRACTION_SUPPORTED
  #if defined( __clang__ )
    #define GUCEF_TEST_MATRIX_CONTRACTING  __attribute__(( target( "fma" ), noinline ))
    #define GUCEF_TEST_MATRIX_CONTRACT_HERE  _Pragma( "clang fp contract(fast)" )
  #else
    #define GUCEF_TEST_MATRIX_CONTRACTING  __attribute__(( target( "fma" ), optimize( "fp-contract=fast" ), noinline ))
    #define GUCEF_TEST_MATRIX_CONTRACT_HERE
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

/**
 *  The results of the precision-sensitive members for one set of inputs, as raw component values so they compare
 *  bit for bit. The inputs make a fused multiply-add observable: a a - c is 0 unfused and a small power of two fused.
 */
template < typename T >
struct SMatrixPreciseResults
{
    T values[ 200 ];
    UInt32 count;

    void Add( const T& v ) { if ( count < 200 ) values[ count++ ] = v; }
    template < typename V > void Add3( const V& v ) { Add( v.x ); Add( v.y ); Add( v.z ); }
    template < typename Q > void Add4( const Q& q ) { Add( q.x ); Add( q.y ); Add( q.z ); Add( q.w ); }
    template < typename M > void AddMatrix( const M& m, UInt32 n ) { for ( UInt32 r=0; r<n; ++r ) for ( UInt32 c=0; c<n; ++c ) Add( m( r, c ) ); }
};

/*-------------------------------------------------------------------------*/

/**
 *  Calls exported precision-sensitive members of the matrix, rotation and transform classes (T is Float32 or Float64).
 *  A macro, so that each caller compiles the calls in its own context: with GUCEF_MATH_INLINE_ALL the member code is
 *  then inlined with the caller's contraction.
 */
#define GUCEF_TEST_CALL_MATRIX_PRECISE_MEMBERS( T, a, c, out )                                                            \
    {                                                                                                                     \
        typedef CTMatrix2D< T > M2;                                                                                       \
        typedef CTMatrix3D< T > M3;                                                                                       \
        typedef CTMatrix4D< T > M4;                                                                                       \
        typedef CTQuaternion< T > Q;                                                                                      \
        typedef CTVector3D< T > V;                                                                                        \
        typedef CTDirectionVector3D< T > D;                                                                               \
        out.count = 0;                                                                                                    \
        const T r2[ 4 ] = { a, -c, T( 1 ), a };                                                                           \
        M2 m2 = M2::FromRowMajor( r2 );                                                                                   \
        CTVector2D< T > v2 = m2.Transform( CTVector2D< T >( a, T( 1 ) ) );                                                \
        out.Add( v2.x ); out.Add( v2.y );                                                                                 \
        out.Add( m2.GetDeterminant() );                                                                                   \
        out.AddMatrix( m2.Multiply( m2 ), 2 );                                                                            \
        const T r3[ 9 ] = { a, -c, T( 0 ), T( 1 ), a, T( 0 ), c, T( 0 ), T( 1 ) };                                       \
        M3 m3 = M3::FromRowMajor( r3 );                                                                                   \
        out.Add3( m3.Transform( V( a, T( 1 ), T( 0 ) ) ) );                                                               \
        out.Add( m3.GetDeterminant() );                                                                                   \
        out.AddMatrix( m3.Multiply( m3 ), 3 );                                                                            \
        out.AddMatrix( m3.Inverse(), 3 );                                                                                 \
        CTPoint2D< T > p2 = m3.TransformPoint2D( CTPoint2D< T >( a, T( 1 ) ) );                                           \
        out.Add( p2.x ); out.Add( p2.y );                                                                                 \
        M4 m4( m3 );                                                                                                      \
        m4( 0, 3 ) = a; m4( 1, 3 ) = -c;                                                                                  \
        out.AddMatrix( m4.Multiply( m4 ), 4 );                                                                            \
        out.Add3( m4.TransformPoint( CTPoint3D< T >( a, T( 1 ), c ) ) );                                                  \
        out.Add3( m4.TransformVector( V( a, T( 1 ), c ) ) );                                                              \
        out.Add( m4.GetDeterminant() );                                                                                   \
        out.AddMatrix( m4.Inverse(), 4 );                                                                                 \
        out.AddMatrix( m4.InverseAffine(), 4 );                                                                           \
        Q qa = Q::FromXYZW( T( 1 ), T( 0 ), a, a );                                                                       \
        Q qb = Q::FromXYZW( c, T( 0 ), T( 1 ), a );                                                                       \
        out.Add4( Q::Multiply( qa, qb ) );                                                                                \
        out.Add( qa.Dot( qb ) );                                                                                          \
        out.Add3( qa.Rotate( V( a, -c, T( 1 ) ) ) );                                                                      \
        out.Add4( qa.Normalized() );                                                                                      \
        out.Add4( Q::Slerp( qa.Normalized(), qb.Normalized(), T( 0.3 ) ) );                                               \
        out.AddMatrix( qa.Normalized().ToRotationMatrix3D(), 3 );                                                         \
        out.Add4( Q::FromTo( D( V( a, c, T( 1 ) ) ), D( V( c, T( 1 ), a ) ) ) );                                          \
        out.Add4( Q::FromAxisAngle( D( V( a, -c, T( 1 ) ) ), CTRadian< T >( a ) ) );                                      \
        out.Add4( CTEulerAngles< T >( CTRadian< T >( a ), CTRadian< T >( c ), CTRadian< T >( T( 0.5 ) ), EULERORDER_XYZ, EULERFRAME_INTRINSIC ).ToQuaternion() ); \
        out.AddMatrix( CTAxisAngle< T >( D( V( a, -c, T( 1 ) ) ), CTRadian< T >( c ) ).ToRotationMatrix3D(), 3 );         \
        CTTransform3D< T > t( V( a, -c, T( 1 ) ), qa.Normalized(), V( a, c, T( 1 ) ) );                                   \
        out.Add3( t.TransformPoint( CTPoint3D< T >( a, T( 1 ), -c ) ) );                                                  \
        out.AddMatrix( t.ToMatrix(), 4 );                                                                                 \
        out.AddMatrix( M4::LookAt( CTPoint3D< T >( a, T( 1 ), c ), CTPoint3D< T >( c, T( 0 ), a ), V::Up(), HANDEDNESS_RIGHT ), 4 ); \
        out.AddMatrix( M4::Perspective( CTRadian< T >( a ), c, T( 0.5 ), T( 100 ), SProjectionConvention::OpenGL() ), 4 ); \
    }

/*-------------------------------------------------------------------------*/

#ifdef GUCEF_TEST_MATRIX_CONTRACTION_SUPPORTED

/** The same calls compiled with the flags of this file (no contraction) */
static void CallMatrixMembersPlain32( Float32 a, Float32 c, SMatrixPreciseResults< Float32 >& out ) GUCEF_TEST_CALL_MATRIX_PRECISE_MEMBERS( Float32, a, c, out )
static void CallMatrixMembersPlain64( Float64 a, Float64 c, SMatrixPreciseResults< Float64 >& out ) GUCEF_TEST_CALL_MATRIX_PRECISE_MEMBERS( Float64, a, c, out )

/*-------------------------------------------------------------------------*/

/** a * b + c written inline in a contracting function: fused when the compiler contracts */
template < typename T >
GUCEF_TEST_MATRIX_CONTRACTING static T
MatrixContractedMultiplyAdd( T a, T b, T c )
{
    GUCEF_TEST_MATRIX_CONTRACT_HERE
    return a * b + c;
}

/*-------------------------------------------------------------------------*/

GUCEF_TEST_MATRIX_CONTRACTING static void
CallMatrixMembersContracted32( Float32 a, Float32 c, SMatrixPreciseResults< Float32 >& out )
{
    GUCEF_TEST_MATRIX_CONTRACT_HERE
    GUCEF_TEST_CALL_MATRIX_PRECISE_MEMBERS( Float32, a, c, out )
}

GUCEF_TEST_MATRIX_CONTRACTING static void
CallMatrixMembersContracted64( Float64 a, Float64 c, SMatrixPreciseResults< Float64 >& out )
{
    GUCEF_TEST_MATRIX_CONTRACT_HERE
    GUCEF_TEST_CALL_MATRIX_PRECISE_MEMBERS( Float64, a, c, out )
}

/*-------------------------------------------------------------------------*/

static bool
MatrixCpuHasFma( void )
{
    __builtin_cpu_init();
    return 0 != __builtin_cpu_supports( "fma" );
}

#endif /* GUCEF_TEST_MATRIX_CONTRACTION_SUPPORTED ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformMatrixFpContractionTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "MatrixFpContraction" );

    GUCEF_TESTFW_TESTCASE( "MatrixFpContraction: exported matrix, rotation and transform members give the same bits from contracting FMA code" )
    #ifndef GUCEF_TEST_MATRIX_CONTRACTION_SUPPORTED
    GUCEF_TESTFW_SKIP( "contraction test needs GCC or Clang on x86" );
    #else
    GUCEF_TESTFW_SKIP_IF( !MatrixCpuHasFma(), "the CPU has no FMA" );

    // a = 1 + 2^-12, c = 1 + 2^-11: a a = 1 + 2^-11 + 2^-24 rounds to c in Float32, so a a - c is 0 unfused and
    // 2^-24 fused. For Float64 a = 1 + 2^-27, c = 1 + 2^-26 (2^-54 is lost). volatile keeps the values opaque.
    volatile Float32 a32 = 1.0f + ldexpf( 1.0f, -12 );
    volatile Float32 c32 = 1.0f + ldexpf( 1.0f, -11 );
    volatile Float64 a64 = 1.0 + ldexp( 1.0, -27 );
    volatile Float64 c64 = 1.0 + ldexp( 1.0, -26 );

    // the probe: the contracting functions of this file must really fuse, otherwise the test cannot detect anything
    // (an unoptimised build does not contract): then it is skipped, not failed
    const Float32 fused32 = MatrixContractedMultiplyAdd< Float32 >( a32, a32, -c32 );
    const Float64 fused64 = MatrixContractedMultiplyAdd< Float64 >( a64, a64, -c64 );
    GUCEF_TESTFW_SKIP_IF( ldexpf( 1.0f, -24 ) != fused32 || ldexp( 1.0, -54 ) != fused64, "this build does not contract a * b + c (no optimisation), the test cannot detect fusion" );
    try
    {
        SMatrixPreciseResults< Float32 > contracted32;
        SMatrixPreciseResults< Float32 > plain32;
        CallMatrixMembersContracted32( a32, c32, contracted32 );
        CallMatrixMembersPlain32( a32, c32, plain32 );
        SMatrixPreciseResults< Float64 > contracted64;
        SMatrixPreciseResults< Float64 > plain64;
        CallMatrixMembersContracted64( a64, c64, contracted64 );
        CallMatrixMembersPlain64( a64, c64, plain64 );

        UInt32 differences = 0;
        for ( UInt32 i=0; i<plain32.count; ++i )
            if ( !SameBits( plain32.values[ i ], contracted32.values[ i ] ) )
                ++differences;
        for ( UInt32 i=0; i<plain64.count; ++i )
            if ( !SameBits( plain64.values[ i ], contracted64.values[ i ] ) )
                ++differences;
        ASSERT_TRUE( plain32.count == contracted32.count && plain64.count == contracted64.count && 183 == plain32.count );
        // the unfused 2x2 transform cancels exactly
        ASSERT_TRUE( 0.0f == plain32.values[ 0 ] && 0.0 == plain64.values[ 0 ] );

        #ifndef GUCEF_MATH_INLINE_ALL
        // the module's copies run in both cases: bit identical
        ASSERT_TRUE( 0 == differences );
        if ( 0 != differences )
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "MatrixFpContraction: " << differences << " results differ between contracting and plain callers" );
        #else
        // everything is inlined: the contracting caller fuses, so results may differ (documented, not an error)
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "MatrixFpContraction: GUCEF_MATH_INLINE_ALL build, " << differences << " of " << ( plain32.count + plain64.count ) << " results differ when the caller contracts" );
        #endif
    }
    catch( ... )
    {
        ERRORHERE;
    }
    #endif /* GUCEF_TEST_MATRIX_CONTRACTION_SUPPORTED ? */
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
