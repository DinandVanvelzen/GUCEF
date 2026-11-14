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

#ifndef GUCEF_MATH_CDECIMALSTATISTICS_H
#define GUCEF_MATH_CDECIMALSTATISTICS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDECIMAL_H
#include "gucefCORE_CDecimal.h"
#define GUCEF_CORE_CDECIMAL_H
#endif /* GUCEF_CORE_CDECIMAL_H ? */

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
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Statistics of arrays of exact decimals.
 *
 *  Every result is computed from exact sums and rounded once with the result scale and rounding mode of the
 *  CORE::CDecimalContext that the caller passes: the results are deterministic and independent of the order of
 *  the values. The variance, covariance and regression sums are formed from the values shifted by the first
 *  value (exact), so large offsets do not cost digits. Each function returns a CORE::TDecimalStatus; on an error
 *  status (CORE::DecimalStatusIsError) the results are left unchanged.
 *
 *  Limits that come from the 64 bit CORE::CDecimal: the sum of the values, and the intermediates named below, have
 *  to fit the unscaled 64 bit range at their scale (DECIMALSTATUS_OVERFLOW otherwise). Products are formed at the
 *  sum of the scales, so for the variance twice the largest scale of the values, and for the covariance and the
 *  regression the sum of the largest scales of both arrays, must not exceed CORE::CDecimal::MaxScale
 *  (DECIMALSTATUS_INVALID_SCALE otherwise).
 *
 *  Static functions only, no state: thread safe. Not realtime safe (CDecimal arithmetic can take the logging
 *  path of its trace macros).
 */
class GUCEF_MATH_PUBLIC_CPP CDecimalStatistics
{
    public:

    /**
     *  Arithmetic mean: the exact sum divided by the count, rounded once.
     *  An empty array gives 0 at the result scale. The exact sum must fit 64 bits (OVERFLOW otherwise).
     */
    static CORE::TDecimalStatus Mean( const CORE::CDecimal* values, UInt32 count, const CORE::CDecimalContext& ctx, CORE::CDecimal& result );

    /**
     *  Variance of the population (sample false, divided by n) or the sample estimate (sample true, divided by n - 1),
     *  evaluated as ( n * sum of squares - square of sum ) / divisor on the shifted values, exactly, rounded once.
     *  Fewer values than the divisor needs (0 for the population, fewer than 2 for the sample) give 0.
     */
    static CORE::TDecimalStatus Variance( const CORE::CDecimal* values, UInt32 count, bool sample, const CORE::CDecimalContext& ctx, CORE::CDecimal& result );

    /**
     *  Standard deviation, the square root of the variance of Variance(), without the intermediate rounding: the
     *  root of the exact quotient is formed with CORE::CBigInteger::ISqrt on the quotient scaled by 10^( 2 * result
     *  scale ) and rounded once with the mode of the context (every mode is honoured; ROUNDINGMODE_UNNECESSARY gives
     *  DECIMALSTATUS_ROUNDING_REQUIRED when the root is not exact). The standard deviation of { 2, 4, 4, 4, 5, 5, 7, 9 }
     *  is exactly 2.
     */
    static CORE::TDecimalStatus StandardDeviation( const CORE::CDecimal* values, UInt32 count, bool sample, const CORE::CDecimalContext& ctx, CORE::CDecimal& result );

    /** Covariance of two arrays of equal length, population or sample form as for Variance(). Same exactness */
    static CORE::TDecimalStatus Covariance( const CORE::CDecimal* xs, const CORE::CDecimal* ys, UInt32 count, bool sample, const CORE::CDecimalContext& ctx, CORE::CDecimal& result );

    /**
     *  Least squares line y = slope * x + intercept. The slope is the exact quotient of the exact sums, rounded once.
     *  The intercept is the mean of y minus the exact product of the mean of x and the slope, rounded in two steps,
     *  so it is within one unit in the last place of the result scale of the exact value.
     *  rSquared is the coefficient of determination, within two units in the last place; 1 when all y are equal.
     *  Fewer than two points, or all x equal: DECIMALSTATUS_DIVISION_BY_ZERO.
     */
    static CORE::TDecimalStatus LinearRegression( const CORE::CDecimal* xs, const CORE::CDecimal* ys, UInt32 count, const CORE::CDecimalContext& ctx,
                                                  CORE::CDecimal& slope, CORE::CDecimal& intercept, CORE::CDecimal& rSquared );

    private:

    CDecimalStatistics( void );     /**< not instantiable */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace MATH */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MATH_CDECIMALSTATISTICS_H ? */
