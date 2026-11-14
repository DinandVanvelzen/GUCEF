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

#ifndef GUCEF_MATH_CTRUNNINGSTATISTICS_H
#define GUCEF_MATH_CTRUNNINGSTATISTICS_H

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
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Single pass, numerically stable statistics of a stream of values: count, mean, variance, standard deviation,
 *  minimum and maximum, by the method of Welford. Two accumulators over parts of a data set combine with
 *  Merge() (the pairwise update of Chan et al.) to the statistics of the whole.
 *
 *  T is a floating point kind (Float32 or Float64; accumulate Float32 data in a Float64 accumulator for
 *  precision). A value type: no allocation, copyable. Not thread safe: one thread per object (accumulate per
 *  thread and Merge). Realtime safe.
 *
 *  An empty accumulator reports 0 for every statistic. The variance of fewer than two values is 0 for the sample
 *  variance as well, there is no sentinel value.
 */
template< typename T >
class CTRunningStatistics
{
    public:

    typedef T value_type;

    CTRunningStatistics( void ) GUCEF_MATH_REALTIME_CONTEXT
        : m_count( 0 ) ,
          m_mean( 0 )  ,
          m_m2( 0 )    ,
          m_min( 0 )   ,
          m_max( 0 )
    {
    }

    /** Back to the empty state */
    void Reset( void ) GUCEF_MATH_REALTIME_CONTEXT
    {
        m_count = 0;
        m_mean = T( 0 );
        m_m2 = T( 0 );
        m_min = T( 0 );
        m_max = T( 0 );
    }

    /** Adds one value */
    void Add( const T& value ) GUCEF_MATH_REALTIME_CONTEXT
    {
        if ( 0 == m_count )
        {
            m_min = value;
            m_max = value;
        }
        else
        {
            if ( value < m_min ) m_min = value;
            if ( value > m_max ) m_max = value;
        }
        ++m_count;
        const T delta = value - m_mean;
        m_mean += delta / T( m_count );
        m_m2 += delta * ( value - m_mean );
    }

    /** Adds an array of values */
    void Add( const T* values, UInt32 count ) GUCEF_MATH_REALTIME_CONTEXT
    {
        for ( UInt32 i = 0; i < count; ++i )
            Add( values[ i ] );
    }

    /** Combines the statistics of another accumulator into this one, as if all its values had been added here */
    void Merge( const CTRunningStatistics& other ) GUCEF_MATH_REALTIME_CONTEXT
    {
        if ( 0 == other.m_count )
            return;
        if ( 0 == m_count )
        {
            *this = other;
            return;
        }
        const T countA = T( m_count );
        const T countB = T( other.m_count );
        const T total = countA + countB;
        const T delta = other.m_mean - m_mean;
        m_m2 = m_m2 + other.m_m2 + delta * delta * countA * countB / total;
        m_mean = m_mean + delta * countB / total;
        if ( other.m_min < m_min ) m_min = other.m_min;
        if ( other.m_max > m_max ) m_max = other.m_max;
        m_count += other.m_count;
    }

    UInt64 GetCount( void ) const GUCEF_MATH_REALTIME_CONTEXT { return m_count; }
    T GetMean( void ) const GUCEF_MATH_REALTIME_CONTEXT { return m_mean; }
    T GetMin( void ) const GUCEF_MATH_REALTIME_CONTEXT { return m_min; }
    T GetMax( void ) const GUCEF_MATH_REALTIME_CONTEXT { return m_max; }

    /** Sum of the squared deviations from the mean */
    T GetSumOfSquaredDeviations( void ) const GUCEF_MATH_REALTIME_CONTEXT { return m_m2; }

    /** Variance of the values as a whole population: divided by n */
    T GetPopulationVariance( void ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        return m_count > 0 ? m_m2 / T( m_count ) : T( 0 );
    }

    /** Unbiased variance estimate of a sample: divided by n - 1 */
    T GetSampleVariance( void ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        return m_count > 1 ? m_m2 / T( m_count - 1 ) : T( 0 );
    }

    T GetPopulationStandardDeviation( void ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        return std::sqrt( GetPopulationVariance() );
    }

    T GetSampleStandardDeviation( void ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        return std::sqrt( GetSampleVariance() );
    }

    private:

    UInt64 m_count;
    T m_mean;
    T m_m2;
    T m_min;
    T m_max;
};

/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace MATH */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MATH_CTRUNNINGSTATISTICS_H ? */
