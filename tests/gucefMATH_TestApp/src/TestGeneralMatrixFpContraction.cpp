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

#ifndef GUCEF_MATH_DIRECTIONS_H
#include "gucefMATH_directions.h"
#define GUCEF_MATH_DIRECTIONS_H
#endif /* GUCEF_MATH_DIRECTIONS_H ? */

#ifndef GUCEF_MATH_QUATERNIONS_H
#include "gucefMATH_quaternions.h"
#define GUCEF_MATH_QUATERNIONS_H
#endif /* GUCEF_MATH_QUATERNIONS_H ? */

#ifndef GUCEF_MATH_POINTS_H
#include "gucefMATH_points.h"
#define GUCEF_MATH_POINTS_H
#endif /* GUCEF_MATH_POINTS_H ? */

#ifndef GUCEF_MATH_CTLUDECOMPOSITION_H
#include "gucefMATH_CTLuDecomposition.h"
#define GUCEF_MATH_CTLUDECOMPOSITION_H
#endif /* GUCEF_MATH_CTLUDECOMPOSITION_H ? */

#ifndef GUCEF_MATH_CTLEASTSQUARES_H
#include "gucefMATH_CTLeastSquares.h"
#define GUCEF_MATH_CTLEASTSQUARES_H
#endif /* GUCEF_MATH_CTLEASTSQUARES_H ? */

#ifndef GUCEF_MATH_CTSVD3_H
#include "gucefMATH_CTSvd3.h"
#define GUCEF_MATH_CTSVD3_H
#endif /* GUCEF_MATH_CTSVD3_H ? */

#ifndef GUCEF_MATH_CTSPHERICALHARMONICSROTATION_H
#include "gucefMATH_CTSphericalHarmonicsRotation.h"
#define GUCEF_MATH_CTSPHERICALHARMONICSROTATION_H
#endif /* GUCEF_MATH_CTSPHERICALHARMONICSROTATION_H ? */

#include "TestGeneralMatrixFpContraction.h"

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
 *  Functions marked GUCEF_TEST_GENERAL_CONTRACTING are compiled for FMA with floating-point contraction enabled,
 *  whatever the flags of this file (the test application builds with contraction off): the target attribute allows FMA
 *  instructions, GCC enables contraction per function with the optimize attribute, Clang with the fp pragma inside the
 *  body. Only on x86 with GCC or Clang, and the test runs only when the CPU has FMA.
 */
#if ( defined( __GNUC__ ) || defined( __clang__ ) ) && ( defined( __x86_64__ ) || defined( __i386__ ) )
  #define GUCEF_TEST_GENERAL_CONTRACTION_SUPPORTED
  #if defined( __clang__ )
    #define GUCEF_TEST_GENERAL_CONTRACTING  __attribute__(( target( "fma" ), noinline ))
    #define GUCEF_TEST_GENERAL_CONTRACT_HERE  _Pragma( "clang fp contract(fast)" )
  #else
    #define GUCEF_TEST_GENERAL_CONTRACTING  __attribute__(( target( "fma" ), optimize( "fp-contract=fast" ), noinline ))
    #define GUCEF_TEST_GENERAL_CONTRACT_HERE
  #endif
#endif

/*-------------------------------------------------------------------------*/

template < typename T >
static bool
SameBitsGeneral( const T& a, const T& b )
{
    return 0 == memcmp( &a, &b, sizeof( T ) );
}

/*-------------------------------------------------------------------------*/

/** The results of the precision-sensitive members for one set of inputs, as raw values that compare bit for bit */
template < typename T >
struct SGeneralPreciseResults
{
    T values[ 400 ];
    UInt32 count;
    void Add( const T& v ) { if ( count < 400 ) values[ count++ ] = v; }
};

/*-------------------------------------------------------------------------*/

/**
 *  Calls exported precision-sensitive members of the general matrices, decompositions, fits and spherical harmonics
 *  (T is Float32 or Float64). A macro, so that each caller compiles the calls in its own context: with
 *  GUCEF_MATH_INLINE_ALL the member code is then inlined with the caller's contraction.
 */
#define GUCEF_TEST_CALL_GENERAL_PRECISE_MEMBERS( T, a, c, out )                                                         \
    {                                                                                                                   \
        out.count = 0;                                                                                                  \
        const T rowsA[ 6 ] = { a, -c, T( 1 ), T( 1 ), a, T( 0.5 ) };                                                    \
        CTMatrix< T, 2, 3 > m23 = CTMatrix< T, 2, 3 >::FromRowMajor( rowsA );                                           \
        CTMatrix< T, 2, 2 > m22 = m23 * m23.Transposed();                                                               \
        for ( UInt32 k=0; k<4; ++k ) out.Add( m22.GetData()[ k ] );                                                     \
        CTDynamicMatrix< T > d( m23 );                                                                                  \
        CTDynamicMatrix< T > dd;                                                                                        \
        d.Transposed().TryMultiply( d, dd );                                                                            \
        for ( UInt32 k=0; k<9; ++k ) out.Add( dd.GetData()[ k ] );                                                      \
        const T ones[ 3 ] = { a, T( 1 ), -c };                                                                          \
        CTDynamicVector< T > v( ones, 3 );                                                                              \
        CTDynamicVector< T > dv;                                                                                        \
        d.TryMultiply( v, dv );                                                                                         \
        out.Add( dv[ 0 ] ); out.Add( dv[ 1 ] );                                                                         \
        T dot = T( 0 );                                                                                                 \
        v.TryDot( v, dot );                                                                                             \
        out.Add( dot ); out.Add( v.GetLength() );                                                                       \
        const T rows3[ 9 ] = { a, T( 0.25 ), T( 0.5 ), T( 0.25 ), c, T( 0.125 ), T( 0.5 ), T( 0.125 ), a };            \
        CTMatrix< T, 3, 3 > s3 = CTMatrix< T, 3, 3 >::FromRowMajor( rows3 );                                            \
        CTLuDecomposition< T > lu;                                                                                      \
        lu.Decompose( s3 );                                                                                             \
        CTMatrix< T, 3, 3 > inverse;                                                                                    \
        lu.GetInverse( inverse );                                                                                       \
        for ( UInt32 k=0; k<9; ++k ) out.Add( inverse.GetData()[ k ] );                                                 \
        out.Add( lu.GetDeterminant() );                                                                                 \
        CTCholeskyDecomposition< T > cholesky;                                                                          \
        cholesky.Decompose( s3 );                                                                                       \
        CTMatrix< T, 3, 1 > x;                                                                                          \
        cholesky.Solve( CTMatrix< T, 3, 1 >::FromColumnMajor( ones ), x );                                              \
        for ( UInt32 k=0; k<3; ++k ) out.Add( x.GetData()[ k ] );                                                       \
        CTQrDecomposition< T > qr;                                                                                      \
        qr.Decompose( d.Transposed() );                                                                                 \
        CTDynamicMatrix< T > q;                                                                                         \
        qr.GetQ( q );                                                                                                   \
        for ( UInt32 k=0; k<9; ++k ) out.Add( q.GetData()[ k ] );                                                       \
        CTSymmetricEigen3< T > eigen;                                                                                   \
        eigen.Decompose( s3.ToMatrix3D() );                                                                             \
        out.Add( eigen.GetEigenvalues().x ); out.Add( eigen.GetEigenvectors()( 1, 2 ) );                                \
        CTSvd3< T > svd;                                                                                                \
        svd.Decompose( s3.ToMatrix3D() * CTMatrix3D< T >::RotationZ( CTRadian< T >( a ) ) );                            \
        out.Add( svd.GetSingularValues().y ); out.Add( svd.GetPolarRotation()( 0, 1 ) );                                \
        CTLeastSquares< T > fit;                                                                                        \
        const T xs[ 4 ] = { T( 0 ), a, T( 2 ), c };                                                                     \
        const T ys[ 4 ] = { T( 1 ), c, a, T( 3 ) };                                                                     \
        T coefficients[ 3 ];                                                                                            \
        fit.FitPolynomial( xs, ys, 4, 2, coefficients );                                                                \
        out.Add( coefficients[ 0 ] ); out.Add( coefficients[ 2 ] );                                                     \
        const CTDirectionVector3D< T > direction( CTVector3D< T >( a, -c, T( 1 ) ) );                                   \
        CTSphericalHarmonics< T, 3 > sh;                                                                                \
        sh.AddDirection( direction, a, SHNORM_N3D );                                                                    \
        out.Add( sh.Evaluate( direction, SHNORM_SN3D ) ); out.Add( sh.Dot( sh ) ); out.Add( sh.GetBandEnergy( 3 ) );    \
        CTSphericalHarmonicsRotation< T, 3 > rotation( CTMatrix3D< T >::RotationX( CTRadian< T >( c ) ) );            \
        rotation.Apply( sh, sh );                                                                                       \
        for ( UInt32 k=0; k<16; ++k ) out.Add( sh.coefficients[ k ] );                                                 \
    }

/*-------------------------------------------------------------------------*/

#ifdef GUCEF_TEST_GENERAL_CONTRACTION_SUPPORTED

/** The same calls compiled with the flags of this file (no contraction) */
static void CallGeneralMembersPlain32( Float32 a, Float32 c, SGeneralPreciseResults< Float32 >& out ) GUCEF_TEST_CALL_GENERAL_PRECISE_MEMBERS( Float32, a, c, out )
static void CallGeneralMembersPlain64( Float64 a, Float64 c, SGeneralPreciseResults< Float64 >& out ) GUCEF_TEST_CALL_GENERAL_PRECISE_MEMBERS( Float64, a, c, out )

/*-------------------------------------------------------------------------*/

/** a * b + c written inline in a contracting function: fused when the compiler contracts */
template < typename T >
GUCEF_TEST_GENERAL_CONTRACTING static T
GeneralContractedMultiplyAdd( T a, T b, T c )
{
    GUCEF_TEST_GENERAL_CONTRACT_HERE
    return a * b + c;
}

/*-------------------------------------------------------------------------*/

GUCEF_TEST_GENERAL_CONTRACTING static void
CallGeneralMembersContracted32( Float32 a, Float32 c, SGeneralPreciseResults< Float32 >& out )
{
    GUCEF_TEST_GENERAL_CONTRACT_HERE
    GUCEF_TEST_CALL_GENERAL_PRECISE_MEMBERS( Float32, a, c, out )
}

GUCEF_TEST_GENERAL_CONTRACTING static void
CallGeneralMembersContracted64( Float64 a, Float64 c, SGeneralPreciseResults< Float64 >& out )
{
    GUCEF_TEST_GENERAL_CONTRACT_HERE
    GUCEF_TEST_CALL_GENERAL_PRECISE_MEMBERS( Float64, a, c, out )
}

/*-------------------------------------------------------------------------*/

static bool
GeneralCpuHasFma( void )
{
    __builtin_cpu_init();
    return 0 != __builtin_cpu_supports( "fma" );
}

#endif /* GUCEF_TEST_GENERAL_CONTRACTION_SUPPORTED ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformGeneralMatrixFpContractionTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "GeneralMatrixFpContraction" );

    GUCEF_TESTFW_TESTCASE( "GeneralMatrixFpContraction: exported general matrix, decomposition and spherical-harmonic members give the same bits from contracting FMA code" )
    #ifndef GUCEF_TEST_GENERAL_CONTRACTION_SUPPORTED
    GUCEF_TESTFW_SKIP( "contraction test needs GCC or Clang on x86" );
    #else
    GUCEF_TESTFW_SKIP_IF( !GeneralCpuHasFma(), "the CPU has no FMA" );
    // a = 1 + 2^-12, c = 1 + 2^-11: a a = 1 + 2^-11 + 2^-24 rounds to c in Float32, so a a - c is 0 unfused and
    // 2^-24 fused. For Float64 a = 1 + 2^-27, c = 1 + 2^-26 (2^-54 is lost). volatile keeps the values opaque.
    volatile Float32 a32 = 1.0f + ldexpf( 1.0f, -12 );
    volatile Float32 c32 = 1.0f + ldexpf( 1.0f, -11 );
    volatile Float64 a64 = 1.0 + ldexp( 1.0, -27 );
    volatile Float64 c64 = 1.0 + ldexp( 1.0, -26 );
    // the probe: the contracting functions of this file must really fuse, otherwise the test cannot detect anything
    const Float32 fused32 = GeneralContractedMultiplyAdd< Float32 >( a32, a32, -c32 );
    const Float64 fused64 = GeneralContractedMultiplyAdd< Float64 >( a64, a64, -c64 );
    GUCEF_TESTFW_SKIP_IF( ldexpf( 1.0f, -24 ) != fused32 || ldexp( 1.0, -54 ) != fused64, "this build does not contract a * b + c (no optimisation), the test cannot detect fusion" );
    try
    {
        SGeneralPreciseResults< Float32 > contracted32;
        SGeneralPreciseResults< Float32 > plain32;
        CallGeneralMembersContracted32( a32, c32, contracted32 );
        CallGeneralMembersPlain32( a32, c32, plain32 );
        SGeneralPreciseResults< Float64 > contracted64;
        SGeneralPreciseResults< Float64 > plain64;
        CallGeneralMembersContracted64( a64, c64, contracted64 );
        CallGeneralMembersPlain64( a64, c64, plain64 );
        UInt32 differences = 0;
        for ( UInt32 i=0; i<plain32.count; ++i )
            if ( !SameBitsGeneral( plain32.values[ i ], contracted32.values[ i ] ) )
                ++differences;
        for ( UInt32 i=0; i<plain64.count; ++i )
            if ( !SameBitsGeneral( plain64.values[ i ], contracted64.values[ i ] ) )
                ++differences;
        ASSERT_TRUE( plain32.count == contracted32.count && plain64.count == contracted64.count && 64 == plain32.count );
        #ifndef GUCEF_MATH_INLINE_ALL
        // the module's copies run in both cases: bit identical
        ASSERT_TRUE( 0 == differences );
        if ( 0 != differences )
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "GeneralMatrixFpContraction: " << differences << " results differ between contracting and plain callers" );
        #else
        // everything is inlined: the contracting caller fuses, so results may differ (documented, not an error)
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "GeneralMatrixFpContraction: GUCEF_MATH_INLINE_ALL build, " << differences << " of " << ( plain32.count + plain64.count ) << " results differ when the caller contracts" );
        #endif
    }
    catch( ... )
    {
        ERRORHERE;
    }
    #endif /* GUCEF_TEST_GENERAL_CONTRACTION_SUPPORTED ? */
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
