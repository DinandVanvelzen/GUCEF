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

/*
 *  This translation unit compiles its test functions WITH floating point contraction and FMA (function attributes and
 *  pragmas below, no build-file flags), the situation of an application built with -ffp-contract=fast -mfma. The
 *  precision-sensitive functions of gucefMATH must still give the results of the module, which is compiled without
 *  contraction (section 5.10 of the design). A probe in the same functions shows that contraction really happens here,
 *  otherwise the test is skipped. With GUCEF_MATH_INLINE_ALL the functions are inlined into this code and follow its
 *  flags: the test then only reports the differences.
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

#ifndef GUCEF_MATH_SCALARFUNCTIONS_H
#include "gucefMATH_ScalarFunctions.h"
#define GUCEF_MATH_SCALARFUNCTIONS_H
#endif /* GUCEF_MATH_SCALARFUNCTIONS_H ? */

#ifndef GUCEF_MATH_ANGLEFUNCTIONS_H
#include "gucefMATH_AngleFunctions.h"
#define GUCEF_MATH_ANGLEFUNCTIONS_H
#endif /* GUCEF_MATH_ANGLEFUNCTIONS_H ? */

#ifndef GUCEF_MATH_CTCOMPENSATEDSUM_H
#include "gucefMATH_CTCompensatedSum.h"
#define GUCEF_MATH_CTCOMPENSATEDSUM_H
#endif /* GUCEF_MATH_CTCOMPENSATEDSUM_H ? */

#include "TestMathReference.h"
#include "TestScalarFpContraction.h"

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

/*
 *  Contraction is enabled only after the library headers, so only the functions of this file are affected.
 *  x86 with GCC or Clang: the functions carry target( "fma" ); GCC adds optimize( "fp-contract=fast" ) per function,
 *  Clang uses the fp contract pragma for the rest of the file.
 */
#if ( defined( __x86_64__ ) || defined( __i386__ ) ) && ( defined( __GNUC__ ) || defined( __clang__ ) )
  #define GUCEF_MATH_TEST_FP_CONTRACTION_AVAILABLE
  #if defined( __clang__ )
    #pragma clang fp contract( fast )
    #define CONTRACTED_FUNCTION __attribute__(( target( "fma" ), noinline ))
  #else
    #define CONTRACTED_FUNCTION __attribute__(( target( "fma" ), optimize( "O2", "fp-contract=fast" ), noinline ))
  #endif
#endif

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifdef GUCEF_MATH_TEST_FP_CONTRACTION_AVAILABLE

/* The probes: plain expressions of this file, which the compiler is allowed to fuse */
CONTRACTED_FUNCTION static Float64 ProbeMultiplyAdd( Float64 a, Float64 b, Float64 c ) { return a * b + c; }
CONTRACTED_FUNCTION static Float64 ProbeLerp( Float64 a, Float64 b, Float64 t ) { return a + ( b - a ) * t; }

/* The library called from contracted code */
CONTRACTED_FUNCTION static Float64 LibraryMultiplyAdd( Float64 a, Float64 b, Float64 c ) { return CTNumericTraits< Float64 >::MultiplyAdd( a, b, c ); }
CONTRACTED_FUNCTION static Float32 LibraryMultiplyAdd32( Float32 a, Float32 b, Float32 c ) { return CTNumericTraits< Float32 >::MultiplyAdd( a, b, c ); }
CONTRACTED_FUNCTION static Float64 LibraryLerp( Float64 a, Float64 b, Float64 t ) { return Lerp( a, b, t ); }
CONTRACTED_FUNCTION static Float32 LibraryLerp32( Float32 a, Float32 b, Float32 t ) { return Lerp( a, b, t ); }
CONTRACTED_FUNCTION static Float64 LibrarySmootherStep( Float64 x ) { return SmootherStep( 0.0, 1.0, x ); }
CONTRACTED_FUNCTION static Float64 LibraryRemap( Float64 x, Float64 outMin, Float64 outMax ) { return Remap( x, -3.0, 5.0, outMin, outMax ); }
CONTRACTED_FUNCTION static Float64 LibraryLerpAngle( Float64 a, Float64 b, Float64 t ) { return LerpAngle( Radians( a ), Radians( b ), t ).GetValue(); }

CONTRACTED_FUNCTION static Float64
LibraryCompensatedSum( const Float64* values, UInt32 count )
{
    CTCompensatedSum< Float64 > sum;
    for ( UInt32 i=0; i<count; ++i )
        sum.Add( values[ i ] * values[ ( i + 1 ) % count ] );     // the product feeds the addition: contractible
    return sum.GetSum();
}

/*-------------------------------------------------------------------------*/

/* The references: every product is stored in a volatile, so nothing can be fused, also under the clang pragma */

static Float64 ReferenceMultiplyAdd( Float64 a, Float64 b, Float64 c ) { volatile Float64 p = a * b; return p + c; }
static Float32 ReferenceMultiplyAdd32( Float32 a, Float32 b, Float32 c ) { volatile Float32 p = a * b; return p + c; }
static Float64 ReferenceLerp( Float64 a, Float64 b, Float64 t ) { volatile Float64 d = b - a; volatile Float64 p = d * t; return a + p; }
static Float32 ReferenceLerp32( Float32 a, Float32 b, Float32 t ) { volatile Float32 d = b - a; volatile Float32 p = d * t; return a + p; }

static Float64
ReferenceSmootherStep( Float64 x )
{
    Float64 t = x < 0.0 ? 0.0 : ( x > 1.0 ? 1.0 : x );
    volatile Float64 t2 = t * t;
    volatile Float64 t3 = t2 * t;
    volatile Float64 t6 = t * 6.0;
    volatile Float64 inner = t6 - 15.0;
    volatile Float64 tInner = t * inner;
    volatile Float64 polynomial = tInner + 10.0;
    return t3 * polynomial;
}

static Float64
ReferenceRemap( Float64 x, Float64 outMin, Float64 outMax )
{
    volatile Float64 t = ( x - -3.0 ) / ( 5.0 - -3.0 );
    return ReferenceLerp( outMin, outMax, t );
}

static Float64
ReferenceLerpAngle( Float64 a, Float64 b, Float64 t )
{
    volatile Float64 delta = DeltaAngle( Radians( a ), Radians( b ) ).GetValue();
    volatile Float64 p = delta * t;
    return a + p;
}

static Float64
ReferenceCompensatedSum( const Float64* values, UInt32 count )
{
    volatile Float64 sum = 0.0;
    volatile Float64 compensation = 0.0;
    for ( UInt32 i=0; i<count; ++i )
    {
        volatile Float64 value = values[ i ] * values[ ( i + 1 ) % count ];
        volatile Float64 t = sum + value;
        if ( fabs( value ) <= fabs( sum ) )
            compensation = compensation + ( ( sum - t ) + value );
        else
            compensation = compensation + ( ( value - t ) + sum );
        sum = t;
    }
    return sum + compensation;
}

/*-------------------------------------------------------------------------*/

static bool SameBits( Float64 a, Float64 b ) { return 0 == memcmp( &a, &b, sizeof( a ) ); }
static bool SameBits32( Float32 a, Float32 b ) { return 0 == memcmp( &a, &b, sizeof( a ) ); }

static bool
CpuHasFma( void )
{
    __builtin_cpu_init();
    return 0 != __builtin_cpu_supports( "fma" );
}

#endif /* GUCEF_MATH_TEST_FP_CONTRACTION_AVAILABLE ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformScalarFpContractionTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "ScalarFpContraction" );

    GUCEF_TESTFW_TESTCASE( "ScalarFpContraction: precision-sensitive scalar functions are not fused into contracted FMA code" )
    try
    {
        #ifndef GUCEF_MATH_TEST_FP_CONTRACTION_AVAILABLE
        GUCEF_TESTFW_SKIP( "needs x86 with GCC or Clang to compile code with contraction and FMA" );
        #else
        GUCEF_TESTFW_SKIP_IF( !CpuHasFma(), "the CPU has no FMA instructions" );

        // inputs through a volatile so that nothing is folded at compile time
        volatile Float64 one = 1.0;
        const Float64 a = one + ldexp( 1.0, -27 );
        const Float64 b = one - ldexp( 1.0, -27 );
        const Float64 fused = ProbeMultiplyAdd( a, b, -one );

        // the probes: fusing must be visible in this file, otherwise the test cannot detect anything
        const UInt64 seed = 0xF3A11ULL;
        MathReference::CSeededSequence generator( seed );
        UInt32 probeDifferences = 0;
        UInt32 libraryDifferences = 0;
        Float64 products[ 64 ];
        for ( UInt32 i=0; i<20000; ++i )
        {
            Float64 x = generator.NextFloat64( -10.0, 10.0 );
            Float64 y = generator.NextFloat64( -10.0, 10.0 );
            Float64 t = generator.NextFloat64( -0.5, 1.5 );
            Float32 x32 = (Float32) x, y32 = (Float32) y, t32 = (Float32) t;
            products[ i % 64 ] = x;
            if ( !SameBits( ProbeLerp( x, y, t ), ReferenceLerp( x, y, t ) ) )
                ++probeDifferences;
            if ( !SameBits( LibraryLerp( x, y, t ), ReferenceLerp( x, y, t ) ) )
                ++libraryDifferences;
            if ( !SameBits32( LibraryLerp32( x32, y32, t32 ), ReferenceLerp32( x32, y32, t32 ) ) )
                ++libraryDifferences;
            if ( !SameBits( LibraryMultiplyAdd( x, y, t ), ReferenceMultiplyAdd( x, y, t ) ) )
                ++libraryDifferences;
            if ( !SameBits32( LibraryMultiplyAdd32( x32, y32, t32 ), ReferenceMultiplyAdd32( x32, y32, t32 ) ) )
                ++libraryDifferences;
            if ( !SameBits( LibrarySmootherStep( t ), ReferenceSmootherStep( t ) ) )
                ++libraryDifferences;
            if ( !SameBits( LibraryRemap( x, y, t ), ReferenceRemap( x, y, t ) ) )
                ++libraryDifferences;
            if ( !SameBits( LibraryLerpAngle( x, y, t ), ReferenceLerpAngle( x, y, t ) ) )
                ++libraryDifferences;
            if ( 63 == ( i % 64 ) && !SameBits( LibraryCompensatedSum( products, 64 ), ReferenceCompensatedSum( products, 64 ) ) )
                ++libraryDifferences;
        }

        GUCEF_TESTFW_SKIP_IF( 0.0 == fused && 0 == probeDifferences, "the compiler did not contract the probes: nothing to detect" );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ScalarFpContraction: seed " << seed << " probe differences " << probeDifferences << " of 20000, library differences " << libraryDifferences );

        #ifndef GUCEF_MATH_INLINE_ALL
        // the module's copies run: two roundings, exactly the reference results
        ASSERT_TRUE( -ldexp( 1.0, -54 ) == fused );
        ASSERT_TRUE( 0.0 == LibraryMultiplyAdd( a, b, -one ) );
        ASSERT_TRUE( 0 == libraryDifferences );
        #else
        // inlined: the results follow the flags of this file; only reported
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ScalarFpContraction: GUCEF_MATH_INLINE_ALL, MultiplyAdd( 1 + 2^-27, 1 - 2^-27, -1 ) = " << LibraryMultiplyAdd( a, b, -one ) );
        #endif
        #endif /* GUCEF_MATH_TEST_FP_CONTRACTION_AVAILABLE ? */
    }
    catch( const ::GUCEF::TESTFW::STestSkipRequest& )
    {
        throw;      // a skip is reported by the test case, not an error
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
