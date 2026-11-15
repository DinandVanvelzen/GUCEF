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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <string.h>

#include <math.h>

#include "gucefCOMPUTE_CComputeTolerance.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COMPUTE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** Maps the bits of a float onto an unsigned scale where neighbouring values differ by 1 and -0 equals +0 */
UInt64
OrderedBits( Float32 value )
{
    UInt32 bits = 0;
    memcpy( &bits, &value, sizeof( bits ) );
    if ( 0 != ( bits & 0x80000000u ) )
        return static_cast< UInt64 >( 0x80000000u ) - static_cast< UInt64 >( bits & 0x7FFFFFFFu );
    return static_cast< UInt64 >( 0x80000000u ) + static_cast< UInt64 >( bits );
}

/*-------------------------------------------------------------------------*/

UInt64
OrderedBits( Float64 value )
{
    UInt64 bits = 0;
    memcpy( &bits, &value, sizeof( bits ) );
    const UInt64 signBit = static_cast< UInt64 >( 1 ) << 63;
    if ( 0 != ( bits & signBit ) )
        return signBit - ( bits & ~signBit );
    return signBit + bits;
}

/*-------------------------------------------------------------------------*/

template < typename T >
UInt64
UlpDistanceImpl( T a, T b )
{
    const bool aNan = a != a;
    const bool bNan = b != b;
    if ( aNan || bNan )
        return ( aNan && bNan ) ? 0 : ~static_cast< UInt64 >( 0 );
    const UInt64 ao = OrderedBits( a );
    const UInt64 bo = OrderedBits( b );
    return ao > bo ? ao - bo : bo - ao;
}

}

/*-------------------------------------------------------------------------*/

CComputeTolerance::SComparison::SComparison( void )
    : count( 0 )
    , outsideCount( 0 )
    , firstOutsideIndex( 0 )
    , maxUlpDistance( 0 )
    , maxAbsoluteError( 0.0 )
    , maxRelativeError( 0.0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CComputeTolerance::CComputeTolerance( void )
    : m_maxUlp( 0 )
    , m_maxAbsoluteError( 0.0 )
    , m_maxRelativeError( 0.0 )
    , m_statistic( TOLERANCESTATISTIC_MAXIMUM )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CComputeTolerance::CComputeTolerance( UInt32 maxUlp, Float64 maxAbsoluteError, Float64 maxRelativeError, TStatistic statistic )
    : m_maxUlp( maxUlp )
    , m_maxAbsoluteError( maxAbsoluteError )
    , m_maxRelativeError( maxRelativeError )
    , m_statistic( statistic )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

UInt32
CComputeTolerance::GetMaxUlp( void ) const
{GUCEF_TRACE;

    return m_maxUlp;
}

/*-------------------------------------------------------------------------*/

void
CComputeTolerance::SetMaxUlp( UInt32 maxUlp )
{GUCEF_TRACE;

    m_maxUlp = maxUlp;
}

/*-------------------------------------------------------------------------*/

Float64
CComputeTolerance::GetMaxAbsoluteError( void ) const
{GUCEF_TRACE;

    return m_maxAbsoluteError;
}

/*-------------------------------------------------------------------------*/

void
CComputeTolerance::SetMaxAbsoluteError( Float64 maxError )
{GUCEF_TRACE;

    m_maxAbsoluteError = maxError;
}

/*-------------------------------------------------------------------------*/

Float64
CComputeTolerance::GetMaxRelativeError( void ) const
{GUCEF_TRACE;

    return m_maxRelativeError;
}

/*-------------------------------------------------------------------------*/

void
CComputeTolerance::SetMaxRelativeError( Float64 maxError )
{GUCEF_TRACE;

    m_maxRelativeError = maxError;
}

/*-------------------------------------------------------------------------*/

CComputeTolerance::TStatistic
CComputeTolerance::GetStatistic( void ) const
{GUCEF_TRACE;

    return m_statistic;
}

/*-------------------------------------------------------------------------*/

void
CComputeTolerance::SetStatistic( TStatistic statistic )
{GUCEF_TRACE;

    m_statistic = statistic;
}

/*-------------------------------------------------------------------------*/

bool
CComputeTolerance::IsExact( void ) const
{GUCEF_TRACE;

    return 0 == m_maxUlp && m_maxAbsoluteError <= 0.0 && m_maxRelativeError <= 0.0;
}

/*-------------------------------------------------------------------------*/

UInt64
CComputeTolerance::UlpDistance( Float32 a, Float32 b )
{GUCEF_TRACE;

    return UlpDistanceImpl( a, b );
}

/*-------------------------------------------------------------------------*/

UInt64
CComputeTolerance::UlpDistance( Float64 a, Float64 b )
{GUCEF_TRACE;

    return UlpDistanceImpl( a, b );
}

/*-------------------------------------------------------------------------*/

template < typename T >
bool
CComputeTolerance::IsWithinImpl( T expected, T actual, UInt64& ulp, Float64& absoluteError, Float64& relativeError ) const
{
    ulp = UlpDistanceImpl( expected, actual );
    if ( 0 == ulp )
    {
        absoluteError = 0.0;
        relativeError = 0.0;
        return true;
    }
    const bool expectedFinite = ( expected == expected ) && ( expected - expected == expected - expected );
    const bool actualFinite = ( actual == actual ) && ( actual - actual == actual - actual );
    if ( !expectedFinite || !actualFinite )
    {
        // NaN against a number, or an infinity against anything else
        absoluteError = HUGE_VAL;
        relativeError = HUGE_VAL;
        return false;
    }
    absoluteError = fabs( static_cast< Float64 >( expected ) - static_cast< Float64 >( actual ) );
    const Float64 magnitude = fabs( static_cast< Float64 >( expected ) );
    relativeError = magnitude > 0.0 ? absoluteError / magnitude : HUGE_VAL;

    if ( 0 != m_maxUlp && ulp <= m_maxUlp )
        return true;
    if ( m_maxAbsoluteError > 0.0 && absoluteError <= m_maxAbsoluteError )
        return true;
    if ( m_maxRelativeError > 0.0 && relativeError <= m_maxRelativeError )
        return true;
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CComputeTolerance::IsWithin( Float32 expected, Float32 actual ) const
{GUCEF_TRACE;

    UInt64 ulp = 0;
    Float64 absoluteError = 0.0;
    Float64 relativeError = 0.0;
    return IsWithinImpl( expected, actual, ulp, absoluteError, relativeError );
}

/*-------------------------------------------------------------------------*/

bool
CComputeTolerance::IsWithin( Float64 expected, Float64 actual ) const
{GUCEF_TRACE;

    UInt64 ulp = 0;
    Float64 absoluteError = 0.0;
    Float64 relativeError = 0.0;
    return IsWithinImpl( expected, actual, ulp, absoluteError, relativeError );
}

/*-------------------------------------------------------------------------*/

template < typename T >
bool
CComputeTolerance::CompareArrays( const T* expected, const T* actual, UInt32 count, SComparison* comparison ) const
{
    SComparison result;
    result.count = count;
    result.firstOutsideIndex = count;
    for ( UInt32 i=0; i<count; ++i )
    {
        UInt64 ulp = 0;
        Float64 absoluteError = 0.0;
        Float64 relativeError = 0.0;
        if ( !IsWithinImpl( expected[ i ], actual[ i ], ulp, absoluteError, relativeError ) )
        {
            if ( 0 == result.outsideCount )
                result.firstOutsideIndex = i;
            ++result.outsideCount;
        }
        if ( ulp > result.maxUlpDistance )
            result.maxUlpDistance = ulp;
        if ( absoluteError > result.maxAbsoluteError )
            result.maxAbsoluteError = absoluteError;
        if ( relativeError > result.maxRelativeError )
            result.maxRelativeError = relativeError;
    }
    if ( GUCEF_NULL != comparison )
        *comparison = result;

    if ( TOLERANCESTATISTIC_PERCENTILE_99_9 == m_statistic )
    {
        // at most 1 in 1000 may be outside, compared in integers: outside * 1000 <= count
        return static_cast< UInt64 >( result.outsideCount ) * 1000 <= static_cast< UInt64 >( count );
    }
    return 0 == result.outsideCount;
}

/*-------------------------------------------------------------------------*/

bool
CComputeTolerance::Compare( const Float32* expected, const Float32* actual, UInt32 count, SComparison* comparison ) const
{GUCEF_TRACE;

    if ( 0 != count && ( GUCEF_NULL == expected || GUCEF_NULL == actual ) )
        return false;
    return CompareArrays( expected, actual, count, comparison );
}

/*-------------------------------------------------------------------------*/

bool
CComputeTolerance::Compare( const Float64* expected, const Float64* actual, UInt32 count, SComparison* comparison ) const
{GUCEF_TRACE;

    if ( 0 != count && ( GUCEF_NULL == expected || GUCEF_NULL == actual ) )
        return false;
    return CompareArrays( expected, actual, count, comparison );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace COMPUTE */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
