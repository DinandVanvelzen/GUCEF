/*
 *  gucefCOMPUTE: GUCEF module providing compute: devices, buffers and kernels over pluggable backends
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

#ifndef GUCEF_COMPUTE_CCOMPUTETOLERANCE_H
#define GUCEF_COMPUTE_CCOMPUTETOLERANCE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_COMPUTE_MACROS_H
#include "gucefCOMPUTE_macros.h"
#define GUCEF_COMPUTE_MACROS_H
#endif /* GUCEF_COMPUTE_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COMPUTE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  How far the results of a floating-point kernel may be from the reference (the CPU backend): a ULP distance, an
 *  absolute and a relative error, and the statistic over a result array that has to meet them. An element is within
 *  the tolerance when it meets any one of the three bounds that is set (a bound of 0 is not set; all three 0 means
 *  exact equality). With TOLERANCESTATISTIC_MAXIMUM every element has to be within, with
 *  TOLERANCESTATISTIC_PERCENTILE_99_9 at least 99.9 % of them. NaN equals NaN, infinities only themselves.
 *  Integer kernels (DETERMINISM_EXACT_INTEGER) use exact equality.
 */
class GUCEF_COMPUTE_PUBLIC_CPP CComputeTolerance
{
    public:

    enum EStatistic
    {
        TOLERANCESTATISTIC_MAXIMUM = 0     ,
        TOLERANCESTATISTIC_PERCENTILE_99_9
    };
    typedef enum EStatistic TStatistic;

    /** What a comparison of two arrays found */
    struct GUCEF_COMPUTE_PUBLIC_CPP SComparison
    {
        UInt32 count;                   /**< elements compared */
        UInt32 outsideCount;            /**< elements outside the tolerance */
        UInt32 firstOutsideIndex;       /**< the first of them, count when there is none */
        UInt64 maxUlpDistance;
        Float64 maxAbsoluteError;
        Float64 maxRelativeError;

        SComparison( void );
    };

    /** Exact equality, the maximum statistic */
    CComputeTolerance( void );

    CComputeTolerance( UInt32 maxUlp, Float64 maxAbsoluteError, Float64 maxRelativeError, TStatistic statistic = TOLERANCESTATISTIC_MAXIMUM );

    UInt32 GetMaxUlp( void ) const;
    void SetMaxUlp( UInt32 maxUlp );

    Float64 GetMaxAbsoluteError( void ) const;
    void SetMaxAbsoluteError( Float64 maxError );

    Float64 GetMaxRelativeError( void ) const;
    void SetMaxRelativeError( Float64 maxError );

    TStatistic GetStatistic( void ) const;
    void SetStatistic( TStatistic statistic );

    /** True when the tolerance demands exact equality (all bounds 0) */
    bool IsExact( void ) const;

    /** The number of representable Float32 values between a and b; 0 for equal values (+0 and -0 are equal), the largest value for NaN against a number */
    static UInt64 UlpDistance( Float32 a, Float32 b );

    /** The same for Float64 */
    static UInt64 UlpDistance( Float64 a, Float64 b );

    /** Whether one value is within the tolerance of the expected one */
    bool IsWithin( Float32 expected, Float32 actual ) const;
    bool IsWithin( Float64 expected, Float64 actual ) const;

    /** Compares 'count' values element by element; true when the statistic is met. 'comparison' when given receives the details */
    bool Compare( const Float32* expected, const Float32* actual, UInt32 count, SComparison* comparison = GUCEF_NULL ) const;
    bool Compare( const Float64* expected, const Float64* actual, UInt32 count, SComparison* comparison = GUCEF_NULL ) const;

    private:

    template < typename T >
    bool CompareArrays( const T* expected, const T* actual, UInt32 count, SComparison* comparison ) const;

    template < typename T >
    bool IsWithinImpl( T expected, T actual, UInt64& ulp, Float64& absoluteError, Float64& relativeError ) const;

    UInt32 m_maxUlp;
    Float64 m_maxAbsoluteError;
    Float64 m_maxRelativeError;
    TStatistic m_statistic;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace COMPUTE */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMPUTE_CCOMPUTETOLERANCE_H ? */
