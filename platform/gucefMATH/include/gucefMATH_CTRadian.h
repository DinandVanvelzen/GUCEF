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

#ifndef GUCEF_MATH_CTRADIAN_H
#define GUCEF_MATH_CTRADIAN_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MATH_CTMATHCONSTANTS_H
#include "gucefMATH_CTMathConstants.h"
#define GUCEF_MATH_CTMATHCONSTANTS_H
#endif /* GUCEF_MATH_CTMATHCONSTANTS_H ? */

#ifndef GUCEF_MATH_FIXEDPOINT_H
#include "gucefMATH_fixedpoint.h"
#define GUCEF_MATH_FIXEDPOINT_H
#endif /* GUCEF_MATH_FIXEDPOINT_H ? */

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

template < typename T > class CTDegree;

/*-------------------------------------------------------------------------*/

/**
 *  An angle in radians: a strong type, so an angle is never confused with another number. A raw number becomes an
 *  angle only explicitly (CTRadian< T >( x ) or the helper functions Radians( x ) / Degrees( x )); CTRadian and CTDegree
 *  convert into each other implicitly because both are angles. Mixed arithmetic of the two is ambiguous on purpose:
 *  convert one side explicitly. The kind T needs transcendental functions (floats or fixed point).
 *  Same size and alignment as T, no virtual functions, realtime-safe.
 */
template < typename T >
class GUCEF_MATH_TEMPLATE_PUBLIC CTRadian
{
    public:

    typedef T TValue;

    GUCEF_MATH_FORCE_INLINE CTRadian( void ) GUCEF_MATH_REALTIME_CONTEXT
        : m_value( CTNumericTraits< T >::Zero() )
    {
    }

    GUCEF_MATH_FORCE_INLINE explicit CTRadian( const T& radians ) GUCEF_MATH_REALTIME_CONTEXT
        : m_value( radians )
    {
    }

    /** Implicit: both are angles */
    CTRadian( const CTDegree< T >& degrees );

    /** The angle in radians */
    GUCEF_MATH_FORCE_INLINE T GetValue( void ) const GUCEF_MATH_REALTIME_CONTEXT { return m_value; }

    GUCEF_MATH_FORCE_INLINE T GetDegrees( void ) const GUCEF_MATH_REALTIME_CONTEXT { return CTAngleUnitConversion< T >::RadiansToDegrees( m_value ); }

    GUCEF_MATH_FORCE_INLINE CTRadian operator-( void ) const GUCEF_MATH_REALTIME_CONTEXT { return CTRadian( -m_value ); }
    GUCEF_MATH_FORCE_INLINE CTRadian operator+( void ) const GUCEF_MATH_REALTIME_CONTEXT { return *this; }

    friend GUCEF_MATH_FORCE_INLINE CTRadian operator+( const CTRadian& a, const CTRadian& b ) GUCEF_MATH_REALTIME_CONTEXT { return CTRadian( a.m_value + b.m_value ); }
    friend GUCEF_MATH_FORCE_INLINE CTRadian operator-( const CTRadian& a, const CTRadian& b ) GUCEF_MATH_REALTIME_CONTEXT { return CTRadian( a.m_value - b.m_value ); }
    friend GUCEF_MATH_FORCE_INLINE CTRadian operator*( const CTRadian& a, const T& factor ) GUCEF_MATH_REALTIME_CONTEXT { return CTRadian( a.m_value * factor ); }
    friend GUCEF_MATH_FORCE_INLINE CTRadian operator*( const T& factor, const CTRadian& a ) GUCEF_MATH_REALTIME_CONTEXT { return CTRadian( factor * a.m_value ); }
    friend GUCEF_MATH_FORCE_INLINE CTRadian operator/( const CTRadian& a, const T& divisor ) GUCEF_MATH_REALTIME_CONTEXT { return CTRadian( a.m_value / divisor ); }

    GUCEF_MATH_FORCE_INLINE CTRadian& operator+=( const CTRadian& other ) GUCEF_MATH_REALTIME_CONTEXT { m_value = m_value + other.m_value; return *this; }
    GUCEF_MATH_FORCE_INLINE CTRadian& operator-=( const CTRadian& other ) GUCEF_MATH_REALTIME_CONTEXT { m_value = m_value - other.m_value; return *this; }
    GUCEF_MATH_FORCE_INLINE CTRadian& operator*=( const T& factor ) GUCEF_MATH_REALTIME_CONTEXT { m_value = m_value * factor; return *this; }
    GUCEF_MATH_FORCE_INLINE CTRadian& operator/=( const T& divisor ) GUCEF_MATH_REALTIME_CONTEXT { m_value = m_value / divisor; return *this; }

    friend GUCEF_MATH_FORCE_INLINE bool operator==( const CTRadian& a, const CTRadian& b ) GUCEF_MATH_REALTIME_CONTEXT { return a.m_value == b.m_value; }
    friend GUCEF_MATH_FORCE_INLINE bool operator!=( const CTRadian& a, const CTRadian& b ) GUCEF_MATH_REALTIME_CONTEXT { return !( a.m_value == b.m_value ); }
    friend GUCEF_MATH_FORCE_INLINE bool operator<( const CTRadian& a, const CTRadian& b ) GUCEF_MATH_REALTIME_CONTEXT { return a.m_value < b.m_value; }
    friend GUCEF_MATH_FORCE_INLINE bool operator<=( const CTRadian& a, const CTRadian& b ) GUCEF_MATH_REALTIME_CONTEXT { return a.m_value <= b.m_value; }
    friend GUCEF_MATH_FORCE_INLINE bool operator>( const CTRadian& a, const CTRadian& b ) GUCEF_MATH_REALTIME_CONTEXT { return a.m_value > b.m_value; }
    friend GUCEF_MATH_FORCE_INLINE bool operator>=( const CTRadian& a, const CTRadian& b ) GUCEF_MATH_REALTIME_CONTEXT { return a.m_value >= b.m_value; }

    private:

    T m_value;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

typedef CTRadian< Float32 > TFloat32Radian;
typedef CTRadian< Float64 > TFloat64Radian;
typedef CTRadian< TFixed32 > TFixed32Radian;
typedef CTRadian< TFixed64 > TFixed64Radian;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace MATH */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#ifndef GUCEF_MATH_CTDEGREE_H
#include "gucefMATH_CTDegree.h"
#define GUCEF_MATH_CTDEGREE_H
#endif /* GUCEF_MATH_CTDEGREE_H ? */

namespace GUCEF {
namespace MATH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_FORCE_INLINE
CTRadian< T >::CTRadian( const CTDegree< T >& degrees ) GUCEF_MATH_REALTIME_CONTEXT
    : m_value( CTAngleUnitConversion< T >::DegreesToRadians( degrees.GetValue() ) )
{
}

/* The Float32 and Float64 instantiations are compiled in the module (gucefMATH_CTRadian.cpp), section 5.10 */
GUCEF_MATH_EXTERN_TEMPLATE_CLASS( CTRadian< Float32 > )
GUCEF_MATH_EXTERN_TEMPLATE_CLASS( CTRadian< Float64 > )

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** An angle from a number of radians */
GUCEF_MATH_FORCE_INLINE CTRadian< Float32 > Radians( Float32 value ) GUCEF_MATH_REALTIME_CONTEXT { return CTRadian< Float32 >( value ); }
GUCEF_MATH_FORCE_INLINE CTRadian< Float64 > Radians( Float64 value ) GUCEF_MATH_REALTIME_CONTEXT { return CTRadian< Float64 >( value ); }

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace MATH */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MATH_CTRADIAN_H ? */
