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

#ifndef GUCEF_MATH_CTRANDOMSTREAM_H
#include "gucefMATH_CTRandomStream.h"
#define GUCEF_MATH_CTRANDOMSTREAM_H
#endif /* GUCEF_MATH_CTRANDOMSTREAM_H ? */

#ifndef GUCEF_MATH_CTALIASTABLE_H
#include "gucefMATH_CTAliasTable.h"
#define GUCEF_MATH_CTALIASTABLE_H
#endif /* GUCEF_MATH_CTALIASTABLE_H ? */

#ifndef GUCEF_MATH_SAMPLEWARPING_H
#include "gucefMATH_SampleWarping.h"
#define GUCEF_MATH_SAMPLEWARPING_H
#endif /* GUCEF_MATH_SAMPLEWARPING_H ? */

#ifndef GUCEF_MATH_LOWDISCREPANCYSEQUENCES_H
#include "gucefMATH_LowDiscrepancySequences.h"
#define GUCEF_MATH_LOWDISCREPANCYSEQUENCES_H
#endif /* GUCEF_MATH_LOWDISCREPANCYSEQUENCES_H ? */

#ifndef GUCEF_MATH_CTQUANTIZER_H
#include "gucefMATH_CTQuantizer.h"
#define GUCEF_MATH_CTQUANTIZER_H
#endif /* GUCEF_MATH_CTQUANTIZER_H ? */

#ifndef GUCEF_MATH_COCTAHEDRALENCODING_H
#include "gucefMATH_COctahedralEncoding.h"
#define GUCEF_MATH_COCTAHEDRALENCODING_H
#endif /* GUCEF_MATH_COCTAHEDRALENCODING_H ? */

#ifndef GUCEF_MATH_CTFRACTALNOISE_H
#include "gucefMATH_CTFractalNoise.h"
#define GUCEF_MATH_CTFRACTALNOISE_H
#endif /* GUCEF_MATH_CTFRACTALNOISE_H ? */

#ifndef GUCEF_MATH_CTDOMAINWARP_H
#include "gucefMATH_CTDomainWarp.h"
#define GUCEF_MATH_CTDOMAINWARP_H
#endif /* GUCEF_MATH_CTDOMAINWARP_H ? */

#include "TestRandomFpContraction.h"

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
struct SRandomResults
{
    T values[ 400 ];
    UInt32 count;

    SRandomResults( void ) : count( 0 ) {}
    void Add( const T& v ) { if ( count < 400 ) values[ count++ ] = v; }
    template < typename V > void Add2( const V& v ) { Add( v.x ); Add( v.y ); }
    template < typename V > void Add3( const V& v ) { Add( v.x ); Add( v.y ); Add( v.z ); }
};

/*-------------------------------------------------------------------------*/

/**
 *  Calls every exported precision-sensitive function of the random stream, the sampling warps, the alias table, the
 *  quantizer, the noise, the fractal noise and the domain warp (T is Float32 or Float64). A macro, so that each caller
 *  compiles the calls in its own context. The octahedral encoder is called through the Float64 values.
 */
#define GUCEF_TEST_CALL_RANDOM_MEMBERS( T, a, c, out )                                                                                  \
    {                                                                                                                                   \
        typedef CTVector3D< T > V;                                                                                                      \
        out.count = 0;                                                                                                                  \
        CTRandomStream< CPcg32 > stream( CPcg32( 42, 54 ) );                                                                            \
        CTRandomStream< CXoshiro256StarStar > xoshiro( CXoshiro256StarStar( 7 ) );                                                      \
        out.Add( stream.Range( a, c * T( 9 ) ) ); out.Add( stream.Normal( a, c ) ); out.Add( stream.Exponential( a ) );                \
        out.Add( xoshiro.Normal( c, a ) ); out.Add( xoshiro.Range( -a, c ) );                                                           \
        out.Add2( stream.template InUnitCircleAs< T >() ); out.Add2( stream.template OnUnitCircleAs< T >() );                          \
        out.Add3( stream.template InUnitSphereAs< T >() ); out.Add3( stream.template OnUnitSphereAs< T >() );                          \
        const CTDirectionVector3D< T > axis( V( a, c, T( 0.5 ) ) );                                                                     \
        out.Add3( stream.InCone( axis, CTRadian< T >( a * T( 0.7 ) ) ) );                                                               \
        out.Add3( stream.InBox( V( -a, c, T( 0 ) ), V( c, T( 4 ) * a, T( 9 ) ) ) );                                                     \
        out.Add3( stream.OnHemisphere( axis ) ); out.Add3( stream.CosineWeightedHemisphere( axis ) );                                   \
        const CTQuaternion< T > rotation = stream.template RandomRotationAs< T >();                                                     \
        out.Add( rotation.x ); out.Add( rotation.y ); out.Add( rotation.z ); out.Add( rotation.w );                                     \
        const T weights[ 4 ] = { a, c, T( 0.25 ), a * c };                                                                              \
        out.Add( (T) stream.Pick( weights, 4 ) ); out.Add( (T) xoshiro.Pick( weights, 4 ) );                                            \
        CTAliasTable< T > table;                                                                                                        \
        table.Initialize( weights, 4 );                                                                                                 \
        for ( UInt32 i=0; i<4; ++i ) { out.Add( table.GetAcceptance( i ) ); out.Add( (T) table.GetAlias( i ) ); }                       \
        out.Add2( UniformDisk( a * T( 0.3 ), c * T( 0.6 ) ) ); out.Add2( ConcentricDisk( a * T( 0.3 ), c * T( 0.6 ) ) );                \
        out.Add2( UniformCircle( a * T( 0.3 ) ) );                                                                                      \
        out.Add3( UniformSphere( a * T( 0.3 ), c * T( 0.6 ) ) ); out.Add3( UniformHemisphere( a * T( 0.3 ), c * T( 0.6 ) ) );           \
        out.Add3( CosineHemisphere( a * T( 0.3 ), c * T( 0.6 ) ) ); out.Add3( UniformCone( a * T( 0.3 ), c * T( 0.6 ), T( 0.4 ) * a ) ); \
        out.Add3( UniformTriangle( a * T( 0.3 ), c * T( 0.6 ) ) );                                                                      \
        V tangent, bitangent;                                                                                                           \
        BuildOrthonormalBasis( V( a * T( 0.36 ), c * T( 0.48 ), T( 0.8 ) ), tangent, bitangent );                                       \
        out.Add3( tangent ); out.Add3( bitangent );                                                                                     \
        out.Add3( OrientAroundNormal( V( a * T( 0.6 ), T( 0 ), T( 0.8 ) ), V( T( 0.36 ), T( 0.48 ), T( 0.8 ) ) ) );                     \
        out.Add3( FibonacciSphere< T >( 7, 100 ) );                                                                                     \
        const CTQuantizer< T > quantizer( -a * T( 3 ), c * T( 7 ), 13 );                                                                \
        out.Add( (T) quantizer.Encode( a * T( 1.1 ) ) ); out.Add( quantizer.Decode( 1234 ) ); out.Add( quantizer.GetStep() );          \
        UInt32 codes[ 3 ];                                                                                                              \
        quantizer.EncodeVector3( V( a, c, a * c ), codes );                                                                             \
        out.Add3( quantizer.DecodeVector3( codes ) );                                                                                   \
        CPerlinNoise perlin( 5 ); COpenSimplex2Noise simplex( 5 ); CValueNoise value( 5 ); CWorleyNoise worley( 5 );                    \
        const T x = a * T( 3.7 ), y = c * T( 5.3 ), z = a * T( 1.9 ), w = c * T( 2.3 );                                                 \
        out.Add( perlin.Noise1D( x ) ); out.Add( perlin.Noise2D( x, y ) ); out.Add( perlin.Noise3D( x, y, z ) ); out.Add( perlin.Noise4D( x, y, z, w ) ); \
        out.Add( simplex.Noise2D( x, y ) ); out.Add( simplex.Noise3D( x, y, z ) );                                                      \
        out.Add( value.Noise1D( x ) ); out.Add( value.Noise2D( x, y ) ); out.Add( value.Noise3D( x, y, z ) );                           \
        out.Add( worley.Noise2D( x, y ) ); out.Add( worley.Noise3D( x, y, z ) );                                                        \
        SFractalNoiseSettings settings;                                                                                                 \
        settings.type = FRACTALTYPE_RIDGED; settings.weightedStrength = 0.5;                                                            \
        out.Add( CTFractalNoise< CPerlinNoise >( perlin, settings ).Noise2D( x, y ) );                                                  \
        out.Add( CTFractalNoise< COpenSimplex2Noise >( simplex, settings ).Noise3D( x, y, z ) );                                        \
        out.Add( CTFractalNoise< CValueNoise >( value, settings ).Noise3D( x, y, z ) );                                                 \
        out.Add( CTFractalNoise< CWorleyNoise >( worley, settings ).Noise2D( x, y ) );                                                  \
        SDomainWarpSettings warpSettings;                                                                                               \
        warpSettings.octaves = 3; warpSettings.amplitude = 0.7;                                                                         \
        T wx = x, wy = y, wz = z;                                                                                                       \
        CTDomainWarp< CPerlinNoise >( perlin, warpSettings ).Warp3D( wx, wy, wz );                                                      \
        out.Add( wx ); out.Add( wy ); out.Add( wz );                                                                                    \
        T vx = x, vy = y;                                                                                                               \
        CTDomainWarp< COpenSimplex2Noise >( simplex, warpSettings ).Warp2D( vx, vy );                                                   \
        out.Add( vx ); out.Add( vy );                                                                                                   \
        const V direction( a * T( 0.3 ), c * T( -0.5 ), a * T( 0.81 ) );                                                                \
        TFloat64Vector3D decoded;                                                                                                       \
        COctahedralEncoding::Decode16( COctahedralEncoding::EncodePrecise16( direction ), decoded );                                    \
        out.Add3( decoded );                                                                                                            \
        COctahedralEncoding::Decode32( COctahedralEncoding::Encode32( direction ), decoded );                                           \
        out.Add3( decoded );                                                                                                            \
    }

/*-------------------------------------------------------------------------*/

#ifdef GUCEF_TEST_CONTRACTION_SUPPORTED

static void CallRandomMembersPlain32( Float32 a, Float32 c, SRandomResults< Float32 >& out ) GUCEF_TEST_CALL_RANDOM_MEMBERS( Float32, a, c, out )
static void CallRandomMembersPlain64( Float64 a, Float64 c, SRandomResults< Float64 >& out ) GUCEF_TEST_CALL_RANDOM_MEMBERS( Float64, a, c, out )

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
CallRandomMembersContracted32( Float32 a, Float32 c, SRandomResults< Float32 >& out )
{
    GUCEF_TEST_CONTRACT_HERE
    GUCEF_TEST_CALL_RANDOM_MEMBERS( Float32, a, c, out )
}

GUCEF_TEST_CONTRACTING static void
CallRandomMembersContracted64( Float64 a, Float64 c, SRandomResults< Float64 >& out )
{
    GUCEF_TEST_CONTRACT_HERE
    GUCEF_TEST_CALL_RANDOM_MEMBERS( Float64, a, c, out )
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
PerformRandomFpContractionTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "RandomFpContraction" );

    GUCEF_TESTFW_TESTCASE( "RandomFpContraction: exported random, sampling, noise, octahedral and quantiser code gives the same bits from contracting FMA code" )
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
        SRandomResults< Float32 > plain32, contracted32;
        SRandomResults< Float64 > plain64, contracted64;
        CallRandomMembersPlain32( a32, c32, plain32 );
        CallRandomMembersContracted32( a32, c32, contracted32 );
        CallRandomMembersPlain64( a64, c64, plain64 );
        CallRandomMembersContracted64( a64, c64, contracted64 );

        UInt32 differences = 0;
        for ( UInt32 i=0; i<plain32.count && i<contracted32.count; ++i )
            if ( !SameBits( plain32.values[ i ], contracted32.values[ i ] ) )
                ++differences;
        for ( UInt32 i=0; i<plain64.count && i<contracted64.count; ++i )
            if ( !SameBits( plain64.values[ i ], contracted64.values[ i ] ) )
                ++differences;
        ASSERT_TRUE( plain32.count == contracted32.count && plain64.count == contracted64.count && plain32.count > 100 && plain32.count == plain64.count );

        #ifndef GUCEF_MATH_INLINE_ALL
        // the module's copies run in both cases: bit identical
        ASSERT_TRUE( 0 == differences );
        if ( 0 != differences )
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "RandomFpContraction: " << differences << " results differ between contracting and plain callers" );
        #else
        // everything inlined that is a template: the contracting caller fuses, so results may differ (documented, not an error)
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "RandomFpContraction: GUCEF_MATH_INLINE_ALL build, " << differences << " of " << ( plain32.count + plain64.count ) << " results differ when the caller contracts" );
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
