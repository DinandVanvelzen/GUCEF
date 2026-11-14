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

#ifndef GUCEF_MATH_POINTS_H
#include "gucefMATH_points.h"
#define GUCEF_MATH_POINTS_H
#endif /* GUCEF_MATH_POINTS_H ? */

#ifndef GUCEF_MATH_DIRECTIONS_H
#include "gucefMATH_directions.h"
#define GUCEF_MATH_DIRECTIONS_H
#endif /* GUCEF_MATH_DIRECTIONS_H ? */

#include "TestMathReference.h"
#include "TestFpContraction.h"

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

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

template < typename T >
static bool
SameBits( const T& a, const T& b )
{
    return 0 == memcmp( &a, &b, sizeof( T ) );
}

/*-------------------------------------------------------------------------*/

/**
 *  The results of the precision-sensitive members for one set of inputs, as raw component values so they compare
 *  bit for bit. Inputs are chosen so that a fused multiply-add changes them: x x' rounds away a low bit that the
 *  following addition would cancel.
 */
template < typename T >
struct SPreciseResults
{
    T values[ 40 ];
    UInt32 count;

    void Add( const T& v ) { if ( count < 40 ) values[ count++ ] = v; }
    template < typename V > void Add3( const V& v ) { Add( v.x ); Add( v.y ); Add( v.z ); }
};

/*-------------------------------------------------------------------------*/

/**
 *  Calls every exported precision-sensitive member (T is Float32 or Float64). A macro, so that each caller compiles the
 *  calls in its own context: with GUCEF_MATH_INLINE_ALL the member code is then inlined with the caller's contraction.
 */
#define GUCEF_TEST_CALL_PRECISE_MEMBERS( T, a, c, out )                                                                   \
    {                                                                                                                     \
        typedef CTVector3D< T > V;                                                                                        \
        typedef CTVector2D< T > V2;                                                                                       \
        typedef CTVector4D< T > V4;                                                                                       \
        typedef CTPoint3D< T > P;                                                                                         \
        typedef CTDirectionVector3D< T > D;                                                                               \
        V v( a, T( 1 ), T( 0 ) );                                                                                         \
        V w( a, -c, T( 0 ) );                                                                                             \
        out.count = 0;                                                                                                    \
        out.Add( v.Dot( w ) );                                                                                            \
        out.Add( w.Dot( v ) );                                                                                            \
        out.Add3( v.Cross( V( T( 1 ), a, -c ) ) );                                                                        \
        out.Add( V( a, c, a ).GetSquaredLength() );                                                                       \
        out.Add( V( a, c, a ).GetLength() );                                                                              \
        out.Add3( V( a, c, a ).Normalized() );                                                                            \
        out.Add3( V::Lerp( v, w, a ) );                                                                                   \
        out.Add3( V( a, c, a ).ClampLength( T( 1 ) ) );                                                                   \
        out.Add3( V::MoveTowards( v, V( a, a, c ), T( 0.5 ) ) );                                                          \
        out.Add3( w.Reflect( D( V( a, c, T( 1 ) ) ) ) );                                                                  \
        out.Add3( w.ProjectOnPlane( D( V( a, c, T( 1 ) ) ) ) );                                                           \
        out.Add( V::Angle( v, V( a, a, c ) ).GetValue() );                                                                \
        out.Add( V2( a, T( 1 ) ).Dot( V2( a, -c ) ) );                                                                    \
        out.Add( V2( a, c ).Cross( V2( c, a ) ) );                                                                        \
        out.Add( V4( a, T( 1 ), T( 0 ), a ).Dot( V4( a, -c, T( 0 ), -c ) ) );                                            \
        out.Add( P( a, T( 0 ), c ).GetDistance( P( T( 0 ), a, T( 1 ) ) ) );                                               \
        out.Add3( D::Slerp( D( V( a, c, T( 1 ) ) ), D( V( c, T( 1 ), a ) ), T( 0.3 ) ).AsVector() );                      \
    }

/*-------------------------------------------------------------------------*/

#ifdef GUCEF_TEST_CONTRACTION_SUPPORTED

/** The same calls compiled with the flags of this file (no contraction) */
static void CallPreciseMembersPlain32( Float32 a, Float32 c, SPreciseResults< Float32 >& out ) GUCEF_TEST_CALL_PRECISE_MEMBERS( Float32, a, c, out )
static void CallPreciseMembersPlain64( Float64 a, Float64 c, SPreciseResults< Float64 >& out ) GUCEF_TEST_CALL_PRECISE_MEMBERS( Float64, a, c, out )

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
    GUCEF_TEST_CALL_PRECISE_MEMBERS( Float32, a, c, out )
}

GUCEF_TEST_CONTRACTING static void
CallPreciseMembersContracted64( Float64 a, Float64 c, SPreciseResults< Float64 >& out )
{
    GUCEF_TEST_CONTRACT_HERE
    GUCEF_TEST_CALL_PRECISE_MEMBERS( Float64, a, c, out )
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
PerformFpContractionTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "FpContraction" );

    GUCEF_TESTFW_TESTCASE( "FpContraction: exported vector, point and direction members give the same bits from contracting FMA code" )
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
        Float32 product32 = a32 * a32;
        ASSERT_TRUE( 0.0f == product32 - c32 );

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
            if ( !SameBits( plain32.values[ i ], contracted32.values[ i ] ) )
                ++differences;
        for ( UInt32 i=0; i<plain64.count; ++i )
            if ( !SameBits( plain64.values[ i ], contracted64.values[ i ] ) )
                ++differences;
        ASSERT_TRUE( plain32.count == contracted32.count && plain64.count == contracted64.count && plain32.count > 30 );
        // the unfused dot product cancels exactly
        ASSERT_TRUE( 0.0f == plain32.values[ 0 ] && 0.0 == plain64.values[ 0 ] );

        #ifndef GUCEF_MATH_INLINE_ALL
        // the module's copies run in both cases: bit identical
        ASSERT_TRUE( 0 == differences );
        if ( 0 != differences )
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "FpContraction: " << differences << " results differ between contracting and plain callers" );
        #else
        // everything is inlined: the contracting caller fuses, so results may differ (documented, not an error)
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "FpContraction: GUCEF_MATH_INLINE_ALL build, " << differences << " of " << ( plain32.count + plain64.count ) << " results differ when the caller contracts" );
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
