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

#ifndef GUCEF_MATH_SIMD_BACKENDS_H
#define GUCEF_MATH_SIMD_BACKENDS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MATH_SIMD_CONFIG_H
#include "gucefMATH_simd_config.h"
#define GUCEF_MATH_SIMD_CONFIG_H
#endif /* GUCEF_MATH_SIMD_CONFIG_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MATH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *      Backend tags of the operation policies CTVector4Ops, CTQuaternionOps and CTMatrix4Ops (section 5.3 of the
 *      design), in the style of the gucefMT atomic backends: the configuration (gucefMATH_simd_config.h) selects a tag,
 *      the policies are specialised per tag for Float32. Every tag other than the scalar one only has an effect where
 *      its specialisation header is compiled in (GUCEF_MATH_SIMD_HAVE_*), so naming a tag is always valid.
 */

/** The scalar primary templates: the canonical evaluation order, for every kind */
struct SMathSimdScalar {};

/** SSE2 (x86 and x86-64) */
struct SMathSimdSse2 {};

/** NEON (AArch64 and ARMv7 with NEON) */
struct SMathSimdNeon {};

/** WebAssembly SIMD128 */
struct SMathSimdWasm128 {};

/** The backend of the value types in this build */
#if ( GUCEF_MATH_SIMD_HAVE_SSE2 == 1 )
typedef SMathSimdSse2 TMathSimdDefaultBackend;
#elif ( GUCEF_MATH_SIMD_HAVE_NEON == 1 )
typedef SMathSimdNeon TMathSimdDefaultBackend;
#elif ( GUCEF_MATH_SIMD_HAVE_WASM128 == 1 )
typedef SMathSimdWasm128 TMathSimdDefaultBackend;
#else
typedef SMathSimdScalar TMathSimdDefaultBackend;
#endif

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace MATH */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MATH_SIMD_BACKENDS_H ? */
