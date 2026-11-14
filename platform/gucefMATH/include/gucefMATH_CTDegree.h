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

#ifndef GUCEF_MATH_CTDEGREE_H
#define GUCEF_MATH_CTDEGREE_H

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

template < typename T > class CTRadian;

/*-------------------------------------------------------------------------*/

/**
 *  An angle in degrees: a strong type, so an angle is never confused with another number. A raw number becomes an
 *  angle only explicitly (CTDegree< T >( x ) or the helper functions Degrees( x ) / Radians( x )); CTRadian and CTDegree
 *  convert into each other implicitly because both are angles. Mixed arithmetic of the two is ambiguous on purpose:
 *  convert one side explicitly. The kind T needs transcendental functions (floats or fixed point).
 *  Same size and alignment as T, no virtual functions, realtime-safe.
 */
template < typename T >
class GUCEF_MATH_TEMPLATE_PUBLIC CTDegree
{
    public:

    typedef T TValue;

    GUCEF_MATH_FORCE_INLINE CTDegree( void ) GUCEF_MATH_REALTIME_CONTEXT
        : m_value( CTNumericTraits< T >::Zero() )
    {
    }

    GUCEF_MATH_FORCE_INLINE explicit CTDegree( const T& degrees ) GUCEF_MATH_REALTIME_CONTEXT
        : m_value( degrees )
    {
    }

    /** Implicit: both are angles */
    CTDegree( const CTRadian< T >& radians );

    /** The angle in degrees */
    GUCEF_MATH_FORCE_INLINE T GetValue( void ) const GUCEF_MATH_REALTIME_CONTEXT { return m_value; }

    GUCEF_MATH_FORCE_INLINE T GetRadians( void ) const GUCEF_MATH_REALTIME_CONTEXT { return CTAngleUnitConversion< T >::DegreesToRadians( m_value ); }

    GUCEF_MATH_FORCE_INLINE CTDegree operator-( void ) const GUCEF_MATH_REALTIME_CONTEXT { return CTDegree( -m_value ); }
    GUCEF_MATH_FORCE_INLINE CTDegree operator+( void ) const GUCEF_MATH_REALTIME_CONTEXT { return *this; }

    friend GUCEF_MATH_FORCE_INLINE CTDegree operator+( const CTDegree& a, const CTDegree& b ) GUCEF_MATH_REALTIME_CONTEXT { return CTDegree( a.m_value + b.m_value ); }
    friend GUCEF_MATH_FORCE_INLINE CTDegree operator-( const CTDegree& a, const CTDegree& b ) GUCEF_MATH_REALTIME_CONTEXT { return CTDegree( a.m_value - b.m_value ); }
    friend GUCEF_MATH_FORCE_INLINE CTDegree operator*( const CTDegree& a, const T& factor ) GUCEF_MATH_REALTIME_CONTEXT { return CTDegree( a.m_value * factor ); }
    friend GUCEF_MATH_FORCE_INLINE CTDegree operator*( const T& factor, const CTDegree& a ) GUCEF_MATH_REALTIME_CONTEXT { return CTDegree( factor * a.m_value ); }
    friend GUCEF_MATH_FORCE_INLINE CTDegree operator/( const CTDegree& a, const T& divisor ) GUCEF_MATH_REALTIME_CONTEXT { return CTDegree( a.m_value / divisor ); }

    GUCEF_MATH_FORCE_INLINE CTDegree& operator+=( const CTDegree& other ) GUCEF_MATH_REALTIME_CONTEXT { m_value = m_value + other.m_value; return *this; }
    GUCEF_MATH_FORCE_INLINE CTDegree& operator-=( const CTDegree& other ) GUCEF_MATH_REALTIME_CONTEXT { m_value = m_value - other.m_value; return *this; }
    GUCEF_MATH_FORCE_INLINE CTDegree& operator*=( const T& factor ) GUCEF_MATH_REALTIME_CONTEXT { m_value = m_value * factor; return *this; }
    GUCEF_MATH_FORCE_INLINE CTDegree& operator/=( const T& divisor ) GUCEF_MATH_REALTIME_CONTEXT { m_value = m_value / divisor; return *this; }

    friend GUCEF_MATH_FORCE_INLINE bool operator==( const CTDegree& a, const CTDegree& b ) GUCEF_MATH_REALTIME_CONTEXT { return a.m_value == b.m_value; }
    friend GUCEF_MATH_FORCE_INLINE bool operator!=( const CTDegree& a, const CTDegree& b ) GUCEF_MATH_REALTIME_CONTEXT { return !( a.m_value == b.m_value ); }
    friend GUCEF_MATH_FORCE_INLINE bool operator<( const CTDegree& a, const CTDegree& b ) GUCEF_MATH_REALTIME_CONTEXT { return a.m_value < b.m_value; }
    friend GUCEF_MATH_FORCE_INLINE bool operator<=( const CTDegree& a, const CTDegree& b ) GUCEF_MATH_REALTIME_CONTEXT { return a.m_value <= b.m_value; }
    friend GUCEF_MATH_FORCE_INLINE bool operator>( const CTDegree& a, const CTDegree& b ) GUCEF_MATH_REALTIME_CONTEXT { return a.m_value > b.m_value; }
    friend GUCEF_MATH_FORCE_INLINE bool operator>=( const CTDegree& a, const CTDegree& b ) GUCEF_MATH_REALTIME_CONTEXT { return a.m_value >= b.m_value; }

    private:

    T m_value;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

typedef CTDegree< Float32 > TFloat32Degree;
typedef CTDegree< Float64 > TFloat64Degree;
typedef CTDegree< TFixed32 > TFixed32Degree;
typedef CTDegree< TFixed64 > TFixed64Degree;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace MATH */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#ifndef GUCEF_MATH_CTRADIAN_H
#include "gucefMATH_CTRadian.h"
#define GUCEF_MATH_CTRADIAN_H
#endif /* GUCEF_MATH_CTRADIAN_H ? */

namespace GUCEF {
namespace MATH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_FORCE_INLINE
CTDegree< T >::CTDegree( const CTRadian< T >& radians ) GUCEF_MATH_REALTIME_CONTEXT
    : m_value( CTAngleUnitConversion< T >::RadiansToDegrees( radians.GetValue() ) )
{
}

/* The Float32 and Float64 instantiations are compiled in the module (gucefMATH_CTDegree.cpp), section 5.10 */
GUCEF_MATH_EXTERN_TEMPLATE_CLASS( CTDegree< Float32 > )
GUCEF_MATH_EXTERN_TEMPLATE_CLASS( CTDegree< Float64 > )

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** An angle from a number of degrees; the Int32 overload keeps Degrees( 90 ) unambiguous */
GUCEF_MATH_FORCE_INLINE CTDegree< Float32 > Degrees( Float32 value ) GUCEF_MATH_REALTIME_CONTEXT { return CTDegree< Float32 >( value ); }
GUCEF_MATH_FORCE_INLINE CTDegree< Float64 > Degrees( Float64 value ) GUCEF_MATH_REALTIME_CONTEXT { return CTDegree< Float64 >( value ); }
GUCEF_MATH_FORCE_INLINE CTDegree< Float32 > Degrees( Int32 value ) GUCEF_MATH_REALTIME_CONTEXT { return CTDegree< Float32 >( (Float32) value ); }

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace MATH */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MATH_CTDEGREE_H ? */
