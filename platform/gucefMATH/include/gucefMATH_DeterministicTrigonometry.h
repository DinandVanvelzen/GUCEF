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

#ifndef GUCEF_MATH_DETERMINISTICTRIGONOMETRY_H
#define GUCEF_MATH_DETERMINISTICTRIGONOMETRY_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MATH_MACROS_H
#include "gucefMATH_macros.h"
#define GUCEF_MATH_MACROS_H
#endif /* GUCEF_MATH_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MATH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *  Sine and cosine that give the same bits on every platform, for results that must be reproducible (the twiddle
 *  factors of the FFT, replays). The C library functions differ in the last bit between libraries (glibc, musl, the
 *  Microsoft and Apple libraries), these do not: they use only addition, subtraction and multiplication of IEEE 754
 *  binary64 values (no library call, no fused multiply-add, no extended precision), in a fixed order.
 *
 *  The angle is given as an exact rational part of a turn, 2 pi * numerator / denominator, because that is what
 *  tables of equally spaced angles need and because an exact fraction reduces exactly: the octant is found with
 *  integer arithmetic and the remaining angle within the octant is formed in double-double precision (about 106 bits),
 *  so no error of a large argument creeps in. The sine and cosine of that remaining angle in [ 0, pi/4 ] are the Taylor
 *  series evaluated in double-double precision, which makes the results the correctly rounded Float64 values except in
 *  the very rare case where the exact value lies within about 1e-30 relative of the middle between two doubles. Measured
 *  against 113 bit references: every fraction j / N for every power of two N up to 2^20 (4.2 million values) and 1.9 million
 *  random fractions up to the largest denominator are correctly rounded, the error is at most 0.5 ulp. Multiples of a
 *  quarter turn are exact (0, 1, -1; +0 for a zero result), odd multiples of an eighth turn give the same value for the
 *  sine and the cosine. A call costs about 0.3 microseconds: meant for tables and set-up, not for a per sample loop.
 *
 *  Realtime-safe: no allocation, no locks, no global state, no loops of unbounded length. The module is compiled with
 *  floating point contraction off, which this code relies on (it is the one thing that would change the bits). A
 *  platform that evaluates binary64 in extended precision (x87 without SSE2) is not supported.
 */

/*-------------------------------------------------------------------------*/

/**
 *  Largest denominator that is accepted: 2^59. The reduction multiplies by 8 in 64 bit unsigned arithmetic.
 */
#define GUCEF_MATH_TURNFRACTION_MAX_DENOMINATOR     ( (GUCEF::Int64) 0x0800000000000000LL )

/*-------------------------------------------------------------------------*/

/**
 *  Sine and cosine of the angle 2 pi * numerator / denominator, as the Float64 values nearest to the exact results.
 *  The numerator may be any value (negative, beyond one turn), the denominator may be negative; it must not be zero and
 *  its magnitude must not exceed GUCEF_MATH_TURNFRACTION_MAX_DENOMINATOR.
 *  @return false for a zero denominator or one beyond the limit: the outputs are then zero.
 */
GUCEF_MATH_PUBLIC_CPP bool
SinCosOfTurnFraction( Int64 numerator, Int64 denominator, Float64& sineResult, Float64& cosineResult ) GUCEF_MATH_REALTIME_CONTEXT;

/*-------------------------------------------------------------------------*/

/**
 *  Float32 form: the Float64 results above rounded to Float32. The same bits on every platform as well.
 */
GUCEF_MATH_PUBLIC_CPP bool
SinCosOfTurnFraction( Int64 numerator, Int64 denominator, Float32& sineResult, Float32& cosineResult ) GUCEF_MATH_REALTIME_CONTEXT;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MATH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MATH_DETERMINISTICTRIGONOMETRY_H ? */
