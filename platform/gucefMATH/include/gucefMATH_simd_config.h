/*
 *  gucefMATH: GUCEF module providing math: scalar, vector, matrix and numeric classes
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

#ifndef GUCEF_MATH_SIMD_CONFIG_H
#define GUCEF_MATH_SIMD_CONFIG_H

/*
 *      Selection of the SIMD backend of the value types and of the instruction sets that the batch kernels may be
 *      compiled for (section 5.2 of the design). This header contains no intrinsics.
 *
 *      GUCEF_MATH_SIMD, when the build defines it, is the cap: NONE, SSE2, AVX2, NEON or WASM128, given as the name
 *      (-DGUCEF_MATH_SIMD=NONE) or as the numeric value of the GUCEF_MATH_SIMD_<name> constant below. Requesting a level
 *      that the compiler cannot target is a compile error. Without it the level is detected from the compiler target:
 *      __wasm_simd128__ gives WASM128; AArch64 (__aarch64__, __arm64__ or _M_ARM64) gives NEON; __SSE2__, _M_X64 or
 *      _M_IX86_FP >= 2 give SSE2 (always on x86-64); anything else NONE. NEON means AArch64 Advanced SIMD: it has the
 *      correctly rounded divide and square root of the scalar path and no flush of denormal numbers, which the 32 bit
 *      ARM NEON lacks, so 32 bit ARM builds use the scalar path to keep the results bit-identical.
 *
 *      GUCEF_MATH_SIMD=NONE removes every intrinsic from the value types and the batch kernels: it is the reference
 *      build of the equivalence tests.
 *
 *      Defined here:
 *      - GUCEF_MATH_SIMD_LEVEL: the level of the value types (AVX2 as cap gives SSE2 value types: AVX2 is a batch
 *        matter);
 *      - GUCEF_MATH_SIMD_HAVE_SSE2, _HAVE_SSE41, _HAVE_NEON, _HAVE_WASM128 (0 or 1);
 *      - GUCEF_MATH_SIMD_BATCH_ALLOW_AVX2 and _ALLOW_F16C: 1 on x86 compilers that can compile single functions for
 *        those instruction sets (the target attribute of GCC and Clang, or Microsoft Visual C++), unless the cap is
 *        below AVX2; such kernels are selected at run time from the CPU features;
 *      - GUCEF_MATH_SIMD_BATCH_ALLOW_FMA: 0 unless the build defines it as 1 (section 5.5: fused kernels are not
 *        bit-identical with the scalar kernels and are never selected for deterministic calls);
 *      - GUCEF_MATH_BATCH_MAX_TIER_LEVEL: the build cap of the batch dispatcher (0 SCALAR, 1 SIMD, 2 MULTICORE,
 *        3 ACCELERATOR) from GUCEF_MATH_BATCH_MAX_TIER, given as the name (-DGUCEF_MATH_BATCH_MAX_TIER=SIMD) or the
 *        number; ACCELERATOR by default. Shared with gucefCOMPUTE (its section 5.5).
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#define GUCEF_MATH_SIMD_NONE        0
#define GUCEF_MATH_SIMD_SSE2        1
#define GUCEF_MATH_SIMD_AVX2        2
#define GUCEF_MATH_SIMD_NEON        3
#define GUCEF_MATH_SIMD_WASM128     4

/* the numeric forms of the request, so -DGUCEF_MATH_SIMD=0 and -DGUCEF_MATH_SIMD=GUCEF_MATH_SIMD_NONE work as well */
#define GUCEF_MATH_SIMD_0           GUCEF_MATH_SIMD_NONE
#define GUCEF_MATH_SIMD_1           GUCEF_MATH_SIMD_SSE2
#define GUCEF_MATH_SIMD_2           GUCEF_MATH_SIMD_AVX2
#define GUCEF_MATH_SIMD_3           GUCEF_MATH_SIMD_NEON
#define GUCEF_MATH_SIMD_4           GUCEF_MATH_SIMD_WASM128

#define GUCEF_MATH_BATCH_MAX_TIER_SCALAR        0
#define GUCEF_MATH_BATCH_MAX_TIER_SIMD          1
#define GUCEF_MATH_BATCH_MAX_TIER_MULTICORE     2
#define GUCEF_MATH_BATCH_MAX_TIER_ACCELERATOR   3
#define GUCEF_MATH_BATCH_MAX_TIER_0             0
#define GUCEF_MATH_BATCH_MAX_TIER_1             1
#define GUCEF_MATH_BATCH_MAX_TIER_2             2
#define GUCEF_MATH_BATCH_MAX_TIER_3             3

#define GUCEF_MATH_SIMD_PASTE2( prefix, name )   prefix##name
#define GUCEF_MATH_SIMD_PASTE( prefix, name )    GUCEF_MATH_SIMD_PASTE2( prefix, name )

/*-------------------------------------------------------------------------//
//                                                                         //
//      DETECTION                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/* What the compiler targets */
#if defined( __wasm_simd128__ )
  #define GUCEF_MATH_SIMD_TARGET_LEVEL  GUCEF_MATH_SIMD_WASM128
#elif defined( __aarch64__ ) || defined( __arm64__ ) || defined( _M_ARM64 )
  #define GUCEF_MATH_SIMD_TARGET_LEVEL  GUCEF_MATH_SIMD_NEON
#elif defined( __SSE2__ ) || defined( _M_X64 ) || ( defined( _M_IX86_FP ) && ( _M_IX86_FP >= 2 ) )
  #define GUCEF_MATH_SIMD_TARGET_LEVEL  GUCEF_MATH_SIMD_SSE2
#else
  #define GUCEF_MATH_SIMD_TARGET_LEVEL  GUCEF_MATH_SIMD_NONE
#endif

/* x86 compilers that can compile one function for a higher instruction set than the rest of the file */
#if ( GUCEF_MATH_SIMD_TARGET_LEVEL == GUCEF_MATH_SIMD_SSE2 ) && ( defined( _MSC_VER ) || defined( __GNUC__ ) || defined( __clang__ ) )
  #define GUCEF_MATH_SIMD_TARGET_ATTRIBUTE_SUPPORTED    1
#else
  #define GUCEF_MATH_SIMD_TARGET_ATTRIBUTE_SUPPORTED    0
#endif

/* The cap requested by the build */
#ifdef GUCEF_MATH_SIMD
  #define GUCEF_MATH_SIMD_REQUESTED     GUCEF_MATH_SIMD_PASTE( GUCEF_MATH_SIMD_, GUCEF_MATH_SIMD )
  #if ( GUCEF_MATH_SIMD_REQUESTED < GUCEF_MATH_SIMD_NONE ) || ( GUCEF_MATH_SIMD_REQUESTED > GUCEF_MATH_SIMD_WASM128 )
    #error "GUCEF_MATH_SIMD must be NONE, SSE2, AVX2, NEON or WASM128"
  #endif
  #if ( GUCEF_MATH_SIMD_REQUESTED == GUCEF_MATH_SIMD_SSE2 ) || ( GUCEF_MATH_SIMD_REQUESTED == GUCEF_MATH_SIMD_AVX2 )
    #if ( GUCEF_MATH_SIMD_TARGET_LEVEL != GUCEF_MATH_SIMD_SSE2 )
      #error "GUCEF_MATH_SIMD requests SSE2 or AVX2 but the compiler does not target SSE2"
    #endif
  #elif ( GUCEF_MATH_SIMD_REQUESTED == GUCEF_MATH_SIMD_NEON )
    #if ( GUCEF_MATH_SIMD_TARGET_LEVEL != GUCEF_MATH_SIMD_NEON )
      #error "GUCEF_MATH_SIMD requests NEON but the compiler does not target AArch64 Advanced SIMD"
    #endif
  #elif ( GUCEF_MATH_SIMD_REQUESTED == GUCEF_MATH_SIMD_WASM128 )
    #if ( GUCEF_MATH_SIMD_TARGET_LEVEL != GUCEF_MATH_SIMD_WASM128 )
      #error "GUCEF_MATH_SIMD requests WASM128 but the compiler does not target WebAssembly SIMD128"
    #endif
  #endif
  #define GUCEF_MATH_SIMD_CAP           GUCEF_MATH_SIMD_REQUESTED
#else
  #if ( GUCEF_MATH_SIMD_TARGET_LEVEL == GUCEF_MATH_SIMD_SSE2 )
    #define GUCEF_MATH_SIMD_CAP         GUCEF_MATH_SIMD_AVX2
  #else
    #define GUCEF_MATH_SIMD_CAP         GUCEF_MATH_SIMD_TARGET_LEVEL
  #endif
#endif

/*-------------------------------------------------------------------------//
//                                                                         //
//      SELECTED CONFIGURATION                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#if ( GUCEF_MATH_SIMD_CAP == GUCEF_MATH_SIMD_AVX2 )
  #define GUCEF_MATH_SIMD_LEVEL         GUCEF_MATH_SIMD_SSE2
#else
  #define GUCEF_MATH_SIMD_LEVEL         GUCEF_MATH_SIMD_CAP
#endif

#if ( GUCEF_MATH_SIMD_LEVEL == GUCEF_MATH_SIMD_SSE2 )
  #define GUCEF_MATH_SIMD_HAVE_SSE2     1
#else
  #define GUCEF_MATH_SIMD_HAVE_SSE2     0
#endif

#if ( GUCEF_MATH_SIMD_LEVEL == GUCEF_MATH_SIMD_SSE2 ) && defined( __SSE4_1__ )
  #define GUCEF_MATH_SIMD_HAVE_SSE41    1
#else
  #define GUCEF_MATH_SIMD_HAVE_SSE41    0
#endif

#if ( GUCEF_MATH_SIMD_LEVEL == GUCEF_MATH_SIMD_NEON )
  #define GUCEF_MATH_SIMD_HAVE_NEON     1
#else
  #define GUCEF_MATH_SIMD_HAVE_NEON     0
#endif

#if ( GUCEF_MATH_SIMD_LEVEL == GUCEF_MATH_SIMD_WASM128 )
  #define GUCEF_MATH_SIMD_HAVE_WASM128  1
#else
  #define GUCEF_MATH_SIMD_HAVE_WASM128  0
#endif

#if ( GUCEF_MATH_SIMD_CAP == GUCEF_MATH_SIMD_AVX2 ) && ( GUCEF_MATH_SIMD_TARGET_ATTRIBUTE_SUPPORTED == 1 )
  #define GUCEF_MATH_SIMD_BATCH_ALLOW_AVX2  1
  #define GUCEF_MATH_SIMD_BATCH_ALLOW_F16C  1
#else
  #define GUCEF_MATH_SIMD_BATCH_ALLOW_AVX2  0
  #define GUCEF_MATH_SIMD_BATCH_ALLOW_F16C  0
#endif

#ifndef GUCEF_MATH_SIMD_BATCH_ALLOW_FMA
  #define GUCEF_MATH_SIMD_BATCH_ALLOW_FMA   0
#endif

#ifdef GUCEF_MATH_BATCH_MAX_TIER
  #define GUCEF_MATH_BATCH_MAX_TIER_LEVEL   GUCEF_MATH_SIMD_PASTE( GUCEF_MATH_BATCH_MAX_TIER_, GUCEF_MATH_BATCH_MAX_TIER )
  #if ( GUCEF_MATH_BATCH_MAX_TIER_LEVEL < 0 ) || ( GUCEF_MATH_BATCH_MAX_TIER_LEVEL > 3 )
    #error "GUCEF_MATH_BATCH_MAX_TIER must be SCALAR, SIMD, MULTICORE or ACCELERATOR"
  #endif
#else
  #define GUCEF_MATH_BATCH_MAX_TIER_LEVEL   GUCEF_MATH_BATCH_MAX_TIER_ACCELERATOR
#endif

/*
 *      Function attributes of the kernels compiled for an instruction set above the build baseline. GCC and Clang
 *      need the target attribute on every function that uses such intrinsics; Microsoft Visual C++ needs none.
 *      Selected at run time only (CPU features), never called otherwise.
 */
#if ( GUCEF_MATH_SIMD_TARGET_ATTRIBUTE_SUPPORTED == 1 ) && ( defined( __GNUC__ ) || defined( __clang__ ) )
  #define GUCEF_MATH_SIMD_TARGET_AVX2   __attribute__(( target( "avx2" ) ))
  #define GUCEF_MATH_SIMD_TARGET_F16C   __attribute__(( target( "avx,f16c" ) ))
#else
  #define GUCEF_MATH_SIMD_TARGET_AVX2
  #define GUCEF_MATH_SIMD_TARGET_F16C
#endif

/*
 *      Disables floating-point contraction for the rest of the enclosing compound statement on compilers that honour
 *      the standard pragma inside a function (Clang); empty elsewhere. The module itself is compiled with contraction off.
 */
#if defined( __clang__ )
  #define GUCEF_MATH_FP_CONTRACT_OFF    _Pragma( "STDC FP_CONTRACT OFF" )
#else
  #define GUCEF_MATH_FP_CONTRACT_OFF
#endif

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MATH_SIMD_CONFIG_H ? */
