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

#ifndef GUCEF_MATH_CTBEZIERCURVE2_H
#include "gucefMATH_CTBezierCurve2.h"
#define GUCEF_MATH_CTBEZIERCURVE2_H
#endif /* GUCEF_MATH_CTBEZIERCURVE2_H ? */

#ifndef GUCEF_MATH_CTBEZIERCURVE3_H
#include "gucefMATH_CTBezierCurve3.h"
#define GUCEF_MATH_CTBEZIERCURVE3_H
#endif /* GUCEF_MATH_CTBEZIERCURVE3_H ? */

#ifndef GUCEF_MATH_CTHERMITESPLINE_H
#include "gucefMATH_CTHermiteSpline.h"
#define GUCEF_MATH_CTHERMITESPLINE_H
#endif /* GUCEF_MATH_CTHERMITESPLINE_H ? */

#ifndef GUCEF_MATH_CTCATMULLROMSPLINE_H
#include "gucefMATH_CTCatmullRomSpline.h"
#define GUCEF_MATH_CTCATMULLROMSPLINE_H
#endif /* GUCEF_MATH_CTCATMULLROMSPLINE_H ? */

#ifndef GUCEF_MATH_CTBSPLINE_H
#include "gucefMATH_CTBSpline.h"
#define GUCEF_MATH_CTBSPLINE_H
#endif /* GUCEF_MATH_CTBSPLINE_H ? */

#ifndef GUCEF_MATH_CTCURVE_H
#include "gucefMATH_CTCurve.h"
#define GUCEF_MATH_CTCURVE_H
#endif /* GUCEF_MATH_CTCURVE_H ? */

#ifndef GUCEF_MATH_CTPATH_H
#include "gucefMATH_CTPath.h"
#define GUCEF_MATH_CTPATH_H
#endif /* GUCEF_MATH_CTPATH_H ? */

#ifndef GUCEF_MATH_SMOOTHINGFUNCTIONS_H
#include "gucefMATH_SmoothingFunctions.h"
#define GUCEF_MATH_SMOOTHINGFUNCTIONS_H
#endif /* GUCEF_MATH_SMOOTHINGFUNCTIONS_H ? */

#ifndef GUCEF_MATH_INTEGRATORFUNCTIONS_H
#include "gucefMATH_IntegratorFunctions.h"
#define GUCEF_MATH_INTEGRATORFUNCTIONS_H
#endif /* GUCEF_MATH_INTEGRATORFUNCTIONS_H ? */

#ifndef GUCEF_MATH_CTBALLISTICS_H
#include "gucefMATH_CTBallistics.h"
#define GUCEF_MATH_CTBALLISTICS_H
#endif /* GUCEF_MATH_CTBALLISTICS_H ? */

#include "TestCurveFpContraction.h"

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
 *  flags of this file (see TestFpContraction.cpp for the mechanism). Only on x86 with GCC or Clang.
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

/** The results of the precision-sensitive code as raw values so that they compare bit for bit */
template < typename T >
struct SCurveResults
{
    T values[ 200 ];
    UInt32 count;

    SCurveResults( void ) : count( 0 ) {}
    void Add( const T& v ) { if ( count < 200 ) values[ count++ ] = v; }
    template < typename V > void Add3( const V& v ) { Add( v.x ); Add( v.y ); Add( v.z ); }
};

/*-------------------------------------------------------------------------*/

/**
 *  Calls every exported precision-sensitive curve, smoothing, integrator and ballistics function (T is Float32 or
 *  Float64). A macro, so that each caller compiles the calls in its own context.
 */
#define GUCEF_TEST_CALL_CURVE_MEMBERS( T, a, c, out )                                                                                  \
    {                                                                                                                                  \
        typedef CTVector3D< T > V;                                                                                                     \
        typedef CTPoint3D< T > P;                                                                                                      \
        out.count = 0;                                                                                                                 \
        const CTBezierCurve3< V > bezier( V( T( 0 ), T( 0 ), T( 0 ) ), V( a, c, T( 1 ) ), V( c, a, -c ), V( T( 3 ), T( 1 ), a ) );  \
        out.Add3( bezier.Evaluate( a * T( 0.4 ) ) );                                                                                   \
        out.Add3( bezier.GetDerivative( c * T( 0.3 ) ) );                                                                              \
        out.Add3( bezier.GetSecondDerivative( c * T( 0.6 ) ) );                                                                        \
        CTBezierCurve3< V > left, right;                                                                                               \
        bezier.Split( a * T( 0.45 ), left, right );                                                                                    \
        out.Add3( left.p2 ); out.Add3( right.p1 );                                                                                     \
        V boundsLow, boundsHigh;                                                                                                       \
        bezier.GetBounds( boundsLow, boundsHigh );                                                                                     \
        out.Add3( boundsLow ); out.Add3( boundsHigh );                                                                                 \
        const CTBezierCurve2< V > quadratic( V( a, c, T( 0 ) ), V( c, a, T( 1 ) ), V( T( 2 ), a, c ) );                              \
        out.Add3( quadratic.Evaluate( a * T( 0.3 ) ) ); out.Add3( quadratic.GetDerivative( c * T( 0.2 ) ) );                          \
        CTHermiteSpline< V > hermite;                                                                                                  \
        hermite.AddKey( V( T( 0 ), T( 0 ), T( 0 ) ), V( a, c, T( 0 ) ) ); hermite.AddKey( V( c, a, T( 1 ) ), V( T( 1 ), a, c ) ); hermite.AddKey( V( a, T( 2 ), c ), V( c, c, a ) ); \
        out.Add3( hermite.Evaluate( a * T( 0.8 ) ) ); out.Add3( hermite.GetDerivative( c * T( 1.3 ) ) );                              \
        CTCatmullRomSpline< V > catmull( CATMULLROM_CENTRIPETAL );                                                                     \
        catmull.AddPoint( V( T( 0 ), T( 0 ), T( 0 ) ) ); catmull.AddPoint( V( a, c, T( 1 ) ) ); catmull.AddPoint( V( c, a, c ) ); catmull.AddPoint( V( T( 3 ), a, a ) ); \
        out.Add3( catmull.Evaluate( a * T( 0.9 ) ) ); out.Add3( catmull.GetDerivative( c * T( 0.7 ) ) );                              \
        const V controls[ 4 ] = { V( T( 0 ), T( 0 ), T( 0 ) ), V( a, c, T( 1 ) ), V( c, a, c ), V( T( 3 ), a, a ) };                  \
        CTBSpline< V > bspline;                                                                                                        \
        bspline.Initialize( 3, controls, 4 );                                                                                          \
        out.Add3( bspline.Evaluate( a * T( 0.55 ) ) ); out.Add3( bspline.GetDerivative( c * T( 0.35 ) ) );                            \
        CTCurve< T > curve;                                                                                                            \
        curve.AddKey( T( 0 ), a ); curve.AddKey( c, T( 1 ) ); curve.AddKey( T( 3 ), a );                                              \
        out.Add( curve.Evaluate( a * T( 0.8 ) ) ); out.Add( curve.GetDerivative( c * T( 0.6 ) ) );                                    \
        CTPath< P > path;                                                                                                              \
        path.AddPoint( P( T( 0 ), T( 0 ), T( 0 ) ) ); path.AddPoint( P( a, c, T( 1 ) ) ); path.AddPoint( P( c, a, c ) ); path.AddPoint( P( T( 3 ), a, a ) ); \
        out.Add( path.GetLength() );                                                                                                   \
        out.Add3( path.GetPointAtDistance( a * T( 1.1 ) ) ); out.Add3( path.GetTangentAtDistance( c * T( 0.9 ) ) );                   \
        P closest; T along = T( 0 );                                                                                                   \
        path.GetClosestPoint( P( c, T( 2 ), a ), closest, along );                                                                     \
        out.Add3( closest ); out.Add( along );                                                                                         \
        SSmoothDampState< V > damp;                                                                                                    \
        out.Add3( SmoothDamp( V( a, c, T( 1 ) ), V( T( 0 ), T( 0 ), c ), damp, T( 0.7 ), T( 1 ) / T( 60 ) ) ); out.Add3( damp.velocity ); \
        out.Add3( InterpTo( V( a, c, T( 1 ) ), V( T( 0 ), T( 0 ), c ), T( 0.1 ), T( 3 ) ) );                                           \
        out.Add3( InterpConstantTo( V( a, c, T( 1 ) ), V( T( 0 ), T( 0 ), c ), T( 0.1 ), T( 3 ) ) );                                   \
        SSpringState< V > spring( V( a, c, T( 1 ) ) );                                                                                 \
        SpringStep( spring, V( T( 0 ), T( 0 ), c ), T( 40 ), T( 5 ), T( 0.01 ) ); out.Add3( spring.position ); out.Add3( spring.velocity ); \
        V euler( a, c, T( 1 ) ), eulerVelocity( c, a, T( 0 ) );                                                                        \
        SemiImplicitEulerStep( euler, eulerVelocity, V( a, a, c ), T( 0.02 ) ); out.Add3( euler ); out.Add3( eulerVelocity );          \
        CTRadian< T > low, high;                                                                                                       \
        CTBallistics< T >::SolveLaunchAngles( T( 10 ) * a, T( 5 ) * c, T( 1 ) * a, T( 9.81 ), low, high );                             \
        out.Add( low.GetValue() ); out.Add( high.GetValue() );                                                                         \
        T time = T( 0 );                                                                                                               \
        CTBallistics< T >::SolveInterceptTime( P( T( 0 ), T( 0 ), T( 0 ) ), T( 10 ) * a, P( T( 10 ) * c, T( 0 ), T( 0 ) ), V( T( 0 ), T( 0 ), T( 5 ) * a ), time ); \
        out.Add( time );                                                                                                               \
        V launch;                                                                                                                      \
        CTBallistics< T >::SolveInterceptWithGravity( P( T( 0 ), T( 0 ), T( 0 ) ), T( 15 ) * a, P( T( 20 ) * c, T( 5 ), T( 3 ) ), V( a, T( 0 ), -c ), V( T( 0 ), T( -9.81 ), T( 0 ) ), time, launch ); \
        out.Add( time ); out.Add3( launch );                                                                                           \
        P apex; T apexTime = T( 0 );                                                                                                   \
        CTBallistics< T >::GetApex( P( a, c, T( 0 ) ), V( T( 3 ) * a, T( 10 ) * c, T( 0 ) ), V( T( 0 ), T( -9.81 ), T( 0 ) ), apex, apexTime ); \
        out.Add3( apex ); out.Add( apexTime );                                                                                         \
    }

/*-------------------------------------------------------------------------*/

#ifdef GUCEF_TEST_CONTRACTION_SUPPORTED

static void CallCurveMembersPlain32( Float32 a, Float32 c, SCurveResults< Float32 >& out ) GUCEF_TEST_CALL_CURVE_MEMBERS( Float32, a, c, out )
static void CallCurveMembersPlain64( Float64 a, Float64 c, SCurveResults< Float64 >& out ) GUCEF_TEST_CALL_CURVE_MEMBERS( Float64, a, c, out )

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_TEST_CONTRACTING static T
ContractedMultiplyAdd( T a, T b, T c )
{
    GUCEF_TEST_CONTRACT_HERE
    return a * b + c;
}

/*-------------------------------------------------------------------------*/

GUCEF_TEST_CONTRACTING static void
CallCurveMembersContracted32( Float32 a, Float32 c, SCurveResults< Float32 >& out )
{
    GUCEF_TEST_CONTRACT_HERE
    GUCEF_TEST_CALL_CURVE_MEMBERS( Float32, a, c, out )
}

GUCEF_TEST_CONTRACTING static void
CallCurveMembersContracted64( Float64 a, Float64 c, SCurveResults< Float64 >& out )
{
    GUCEF_TEST_CONTRACT_HERE
    GUCEF_TEST_CALL_CURVE_MEMBERS( Float64, a, c, out )
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
PerformCurveFpContractionTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "CurveFpContraction" );

    GUCEF_TESTFW_TESTCASE( "CurveFpContraction: exported curve, smoothing, integrator and ballistics code gives the same bits from contracting FMA code" )
    #ifndef GUCEF_TEST_CONTRACTION_SUPPORTED
    GUCEF_TESTFW_SKIP( "contraction test needs GCC or Clang on x86" );
    #else
    GUCEF_TESTFW_SKIP_IF( !CpuHasFma(), "the CPU has no FMA" );

    // a = 1 + 2^-12, c = 1 + 2^-11 for Float32 ( a a rounds to c ); a = 1 + 2^-27, c = 1 + 2^-26 for Float64 ( see TestFpContraction.cpp )
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
        SCurveResults< Float32 > plain32, contracted32;
        SCurveResults< Float64 > plain64, contracted64;
        CallCurveMembersPlain32( a32, c32, plain32 );
        CallCurveMembersContracted32( a32, c32, contracted32 );
        CallCurveMembersPlain64( a64, c64, plain64 );
        CallCurveMembersContracted64( a64, c64, contracted64 );

        UInt32 differences = 0;
        for ( UInt32 i=0; i<plain32.count && i<contracted32.count; ++i )
            if ( !SameBits( plain32.values[ i ], contracted32.values[ i ] ) )
                ++differences;
        for ( UInt32 i=0; i<plain64.count && i<contracted64.count; ++i )
            if ( !SameBits( plain64.values[ i ], contracted64.values[ i ] ) )
                ++differences;
        ASSERT_TRUE( plain32.count == contracted32.count && plain64.count == contracted64.count && plain32.count > 80 && plain32.count == plain64.count );

        #ifndef GUCEF_MATH_INLINE_ALL
        // the module's copies run in both cases: bit identical
        ASSERT_TRUE( 0 == differences );
        if ( 0 != differences )
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CurveFpContraction: " << differences << " results differ between contracting and plain callers" );
        #else
        // everything is inlined: the contracting caller fuses, so results may differ (documented, not an error)
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "CurveFpContraction: GUCEF_MATH_INLINE_ALL build, " << differences << " of " << ( plain32.count + plain64.count ) << " results differ when the caller contracts" );
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
