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

#ifndef GUCEF_MATH_CTPIECEWISELINEARCURVE_H
#define GUCEF_MATH_CTPIECEWISELINEARCURVE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MATH_MACROS_H
#include "gucefMATH_macros.h"
#define GUCEF_MATH_MACROS_H
#endif /* GUCEF_MATH_MACROS_H ? */

#ifndef GUCEF_MATH_SCALARFUNCTIONS_H
#include "gucefMATH_ScalarFunctions.h"
#define GUCEF_MATH_SCALARFUNCTIONS_H
#endif /* GUCEF_MATH_SCALARFUNCTIONS_H ? */


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
 *  Piecewise linear function through up to MaxPoints points, for response and attenuation curves.
 *
 *  Fixed capacity, no allocation: a value type that is cheap to copy. The points are kept sorted by x; adding a
 *  point at an x that exists replaces its y. Evaluation is clamped at the ends: left of the first point the
 *  value of the first point, right of the last point the value of the last point. An empty curve evaluates to 0.
 *  T is a floating point kind. Not thread safe for modification; concurrent evaluation of a const curve is safe.
 *  Evaluate() is realtime safe.
 */
template< typename T, UInt32 MaxPoints >
class CTPiecewiseLinearCurve
{
    public:

    typedef T value_type;

    static const UInt32 Capacity = MaxPoints;

    CTPiecewiseLinearCurve( void ) GUCEF_MATH_REALTIME_CONTEXT
        : m_count( 0 )
    {
    }

    /** Removes all points */
    void Clear( void ) GUCEF_MATH_REALTIME_CONTEXT
    {
        m_count = 0;
    }

    /**
     *  Adds a point at its sorted position, or replaces the y of the point with the same x.
     *  Returns false, changing nothing, when the curve is full and x is new, or when x or y is not a number.
     */
    bool AddPoint( const T& x, const T& y ) GUCEF_MATH_REALTIME_CONTEXT
    {
        if ( x != x || y != y )
            return false;

        UInt32 position = 0;
        while ( position < m_count && m_x[ position ] < x )
            ++position;

        if ( position < m_count && m_x[ position ] == x )
        {
            m_y[ position ] = y;
            return true;
        }
        if ( m_count >= MaxPoints )
            return false;

        for ( UInt32 i = m_count; i > position; --i )
        {
            m_x[ i ] = m_x[ i - 1 ];
            m_y[ i ] = m_y[ i - 1 ];
        }
        m_x[ position ] = x;
        m_y[ position ] = y;
        ++m_count;
        return true;
    }

    UInt32 GetPointCount( void ) const GUCEF_MATH_REALTIME_CONTEXT { return m_count; }

    /** The x of the point at the sorted index, 0 for an index out of range */
    T GetPointX( UInt32 index ) const GUCEF_MATH_REALTIME_CONTEXT { return index < m_count ? m_x[ index ] : T( 0 ); }

    /** The y of the point at the sorted index, 0 for an index out of range */
    T GetPointY( UInt32 index ) const GUCEF_MATH_REALTIME_CONTEXT { return index < m_count ? m_y[ index ] : T( 0 ); }

    /** Value at x by linear interpolation between the neighbouring points, clamped at both ends */
    T Evaluate( const T& x ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        if ( 0 == m_count )
            return T( 0 );
        if ( !( x > m_x[ 0 ] ) )
            return m_y[ 0 ];
        if ( !( x < m_x[ m_count - 1 ] ) )
            return m_y[ m_count - 1 ];

        UInt32 upper = 1;
        while ( upper < m_count - 1 && !( x < m_x[ upper ] ) )
            ++upper;
        return Lerp( m_y[ upper - 1 ], m_y[ upper ], InverseLerp( m_x[ upper - 1 ], m_x[ upper ], x ) );
    }

    bool operator==( const CTPiecewiseLinearCurve& other ) const
    {
        if ( m_count != other.m_count )
            return false;
        for ( UInt32 i = 0; i < m_count; ++i )
            if ( m_x[ i ] != other.m_x[ i ] || m_y[ i ] != other.m_y[ i ] )
                return false;
        return true;
    }

    bool operator!=( const CTPiecewiseLinearCurve& other ) const
    {
        return !( *this == other );
    }

    private:

    T m_x[ MaxPoints ];
    T m_y[ MaxPoints ];
    UInt32 m_count;
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

#endif /* GUCEF_MATH_CTPIECEWISELINEARCURVE_H ? */
