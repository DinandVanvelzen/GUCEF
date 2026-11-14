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

#ifndef GUCEF_MATH_STATISTICSFUNCTIONS_H
#define GUCEF_MATH_STATISTICSFUNCTIONS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <cmath>

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
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

/** Definitions of the percentile of a sorted sample, named after the types of Hyndman and Fan */
enum EPercentileMethod
{
    PERCENTILE_LINEAR_INCLUSIVE = 0 ,   /**< type 7: position ( n - 1 ) p, linear interpolation. The default of NumPy and spreadsheets (PERCENTILE.INC) */
    PERCENTILE_LINEAR_EXCLUSIVE     ,   /**< type 6: position ( n + 1 ) p - 1 clamped to the sample, linear interpolation (PERCENTILE.EXC without the range error) */
    PERCENTILE_NEAREST_RANK         ,   /**< type 1 / nearest rank: the value at rank ceil( n p ), no interpolation */
    PERCENTILE_LOWER                ,   /**< the value below the type 7 position */
    PERCENTILE_HIGHER               ,   /**< the value above the type 7 position */
    PERCENTILE_MIDPOINT                 /**< the mean of the values below and above the type 7 position */
};
typedef enum EPercentileMethod TPercentileMethod;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *  Statistics of arrays of floating point values (T is Float32 or Float64). All functions return 0 for an empty
 *  array, there are no sentinel values. The mean and variance use a corrected two pass algorithm that is exact to
 *  the working precision even for data with a large offset. The functions do not allocate; Median() and
 *  Percentile() sort the array they are given, so the caller provides a scratch copy.
 */

/*-------------------------------------------------------------------------*/

/** Arithmetic mean */
template< typename T >
T
Mean( const T* values, UInt32 count ) GUCEF_MATH_REALTIME_CONTEXT
{
    if ( 0 == count )
        return T( 0 );
    T sum = T( 0 );
    for ( UInt32 i = 0; i < count; ++i )
        sum += values[ i ];
    const T mean = sum / T( count );
    T correction = T( 0 );
    for ( UInt32 i = 0; i < count; ++i )
        correction += values[ i ] - mean;
    return mean + correction / T( count );
}

/*-------------------------------------------------------------------------*/

/** Variance: of the population (divided by n) or, with 'sample' set, the unbiased sample estimate (n - 1) */
template< typename T >
T
Variance( const T* values, UInt32 count, bool sample = false ) GUCEF_MATH_REALTIME_CONTEXT
{
    const UInt32 divisor = sample ? count - 1 : count;
    if ( count < 1 || 0 == divisor )
        return T( 0 );
    const T mean = Mean( values, count );
    T sumOfSquares = T( 0 );
    T sumOfDeviations = T( 0 );
    for ( UInt32 i = 0; i < count; ++i )
    {
        const T deviation = values[ i ] - mean;
        sumOfSquares += deviation * deviation;
        sumOfDeviations += deviation;
    }
    return ( sumOfSquares - sumOfDeviations * sumOfDeviations / T( count ) ) / T( divisor );
}

/*-------------------------------------------------------------------------*/

/** Square root of the variance */
template< typename T >
T
StandardDeviation( const T* values, UInt32 count, bool sample = false ) GUCEF_MATH_REALTIME_CONTEXT
{
    return std::sqrt( Variance( values, count, sample ) );
}

/*-------------------------------------------------------------------------*/

/** Covariance of two arrays of equal length, population or sample form as in Variance() */
template< typename T >
T
Covariance( const T* xs, const T* ys, UInt32 count, bool sample = false ) GUCEF_MATH_REALTIME_CONTEXT
{
    const UInt32 divisor = sample ? count - 1 : count;
    if ( count < 1 || 0 == divisor )
        return T( 0 );
    const T meanX = Mean( xs, count );
    const T meanY = Mean( ys, count );
    T sum = T( 0 );
    T sumX = T( 0 );
    T sumY = T( 0 );
    for ( UInt32 i = 0; i < count; ++i )
    {
        const T dx = xs[ i ] - meanX;
        const T dy = ys[ i ] - meanY;
        sum += dx * dy;
        sumX += dx;
        sumY += dy;
    }
    return ( sum - sumX * sumY / T( count ) ) / T( divisor );
}

/*-------------------------------------------------------------------------*/

/** Pearson correlation coefficient in [-1, 1]; 0 when either array has no variance */
template< typename T >
T
Correlation( const T* xs, const T* ys, UInt32 count ) GUCEF_MATH_REALTIME_CONTEXT
{
    if ( count < 2 )
        return T( 0 );
    const T meanX = Mean( xs, count );
    const T meanY = Mean( ys, count );
    T sxy = T( 0 );
    T sxx = T( 0 );
    T syy = T( 0 );
    for ( UInt32 i = 0; i < count; ++i )
    {
        const T dx = xs[ i ] - meanX;
        const T dy = ys[ i ] - meanY;
        sxy += dx * dy;
        sxx += dx * dx;
        syy += dy * dy;
    }
    if ( !( sxx > T( 0 ) ) || !( syy > T( 0 ) ) )
        return T( 0 );
    T r = sxy / std::sqrt( sxx * syy );
    if ( r > T( 1 ) ) r = T( 1 );
    if ( r < T( -1 ) ) r = T( -1 );
    return r;
}

/*-------------------------------------------------------------------------*/

/**
 *  Least squares line y = slope x + intercept through the points. rSquared is the coefficient of determination
 *  (1 when all y are equal and fit). Returns false, with all outputs 0, when there are fewer than two points or
 *  all x are equal.
 */
template< typename T >
bool
LinearRegression( const T* xs, const T* ys, UInt32 count, T& slope, T& intercept, T& rSquared ) GUCEF_MATH_REALTIME_CONTEXT
{
    slope = T( 0 );
    intercept = T( 0 );
    rSquared = T( 0 );
    if ( count < 2 )
        return false;
    const T meanX = Mean( xs, count );
    const T meanY = Mean( ys, count );
    T sxy = T( 0 );
    T sxx = T( 0 );
    T syy = T( 0 );
    for ( UInt32 i = 0; i < count; ++i )
    {
        const T dx = xs[ i ] - meanX;
        const T dy = ys[ i ] - meanY;
        sxy += dx * dy;
        sxx += dx * dx;
        syy += dy * dy;
    }
    if ( !( sxx > T( 0 ) ) )
        return false;
    slope = sxy / sxx;
    intercept = meanY - slope * meanX;
    rSquared = ( syy > T( 0 ) ) ? ( sxy * sxy ) / ( sxx * syy ) : T( 1 );
    return true;
}

/*-------------------------------------------------------------------------*/

/** Sorts ascending in place with a heap sort: O( n log n ) worst case, no recursion, no allocation */
template< typename T >
void
SortAscending( T* values, UInt32 count ) GUCEF_MATH_REALTIME_CONTEXT
{
    if ( count < 2 )
        return;

    // Build the max heap
    for ( UInt32 start = count / 2; start > 0; --start )
    {
        UInt32 root = start - 1;
        const T item = values[ root ];
        for ( ;; )
        {
            UInt32 child = 2 * root + 1;
            if ( child >= count ) break;
            if ( child + 1 < count && values[ child ] < values[ child + 1 ] ) ++child;
            if ( !( item < values[ child ] ) ) break;
            values[ root ] = values[ child ];
            root = child;
        }
        values[ root ] = item;
    }

    // Move the maximum to the end and restore the heap
    for ( UInt32 end = count - 1; end > 0; --end )
    {
        const T item = values[ end ];
        values[ end ] = values[ 0 ];
        UInt32 root = 0;
        for ( ;; )
        {
            UInt32 child = 2 * root + 1;
            if ( child >= end ) break;
            if ( child + 1 < end && values[ child ] < values[ child + 1 ] ) ++child;
            if ( !( item < values[ child ] ) ) break;
            values[ root ] = values[ child ];
            root = child;
        }
        values[ root ] = item;
    }
}

/*-------------------------------------------------------------------------*/

/**
 *  Percentile p (0 to 100, clamped) of the values. The array is sorted in place: pass a scratch copy when the
 *  original order matters. 0 for an empty array.
 */
template< typename T >
T
Percentile( T* values, UInt32 count, T p, TPercentileMethod method = PERCENTILE_LINEAR_INCLUSIVE ) GUCEF_MATH_REALTIME_CONTEXT
{
    if ( 0 == count )
        return T( 0 );
    SortAscending( values, count );
    if ( !( p > T( 0 ) ) ) p = T( 0 );
    if ( p > T( 100 ) ) p = T( 100 );
    const T fraction = p / T( 100 );
    const T last = T( count - 1 );

    switch ( method )
    {
        case PERCENTILE_NEAREST_RANK:
        {
            T rank = std::ceil( fraction * T( count ) );
            if ( rank < T( 1 ) ) rank = T( 1 );
            UInt32 index = (UInt32) rank - 1;
            return values[ index < count ? index : count - 1 ];
        }
        case PERCENTILE_LINEAR_EXCLUSIVE:
        {
            T position = fraction * T( count + 1 ) - T( 1 );
            if ( position < T( 0 ) ) position = T( 0 );
            if ( position > last ) position = last;
            const UInt32 lower = (UInt32) std::floor( position );
            const UInt32 upper = lower + 1 < count ? lower + 1 : lower;
            return values[ lower ] + ( position - T( lower ) ) * ( values[ upper ] - values[ lower ] );
        }
        default:
        {
            const T position = fraction * last;
            const UInt32 lower = (UInt32) std::floor( position );
            const UInt32 upper = lower + 1 < count ? lower + 1 : lower;
            switch ( method )
            {
                case PERCENTILE_LOWER:    return values[ lower ];
                case PERCENTILE_HIGHER:   return ( position > T( lower ) ) ? values[ upper ] : values[ lower ];
                case PERCENTILE_MIDPOINT: return ( values[ lower ] + values[ upper ] ) / T( 2 );
                default:                  return values[ lower ] + ( position - T( lower ) ) * ( values[ upper ] - values[ lower ] );
            }
        }
    }
}

/*-------------------------------------------------------------------------*/

/** Median: the 50th percentile with interpolation. Sorts the array in place, pass a scratch copy */
template< typename T >
T
Median( T* values, UInt32 count ) GUCEF_MATH_REALTIME_CONTEXT
{
    return Percentile( values, count, T( 50 ), PERCENTILE_LINEAR_INCLUSIVE );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace MATH */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MATH_STATISTICSFUNCTIONS_H ? */
