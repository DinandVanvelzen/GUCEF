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

#ifndef GUCEF_MATH_ANGLEFUNCTIONS_H
#define GUCEF_MATH_ANGLEFUNCTIONS_H

/*
 *  Trigonometric functions on strong angles and angle arithmetic (wrapping, shortest difference, interpolation).
 *  Every function exists for CTRadian and CTDegree; the CTDegree forms compute in degrees (period 360), so whole
 *  degree arguments give exact results (DeltaAngle( 350, 10 ) is exactly 20). Realtime-safe, allocation free.
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MATH_CTRADIAN_H
#include "gucefMATH_CTRadian.h"
#define GUCEF_MATH_CTRADIAN_H
#endif /* GUCEF_MATH_CTRADIAN_H ? */

#ifndef GUCEF_MATH_CTDEGREE_H
#include "gucefMATH_CTDegree.h"
#define GUCEF_MATH_CTDEGREE_H
#endif /* GUCEF_MATH_CTDEGREE_H ? */

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
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** value wrapped to ( -halfPeriod, halfPeriod ] */
template < typename T >
GUCEF_MATH_PRECISE_INLINE T
WrapPeriodicSigned( const T& value, const T& period, const T& halfPeriod ) GUCEF_MATH_REALTIME_CONTEXT
{
    T r = CTNumericTraits< T >::ModFloor( value, period );
    if ( halfPeriod < r )
    {
        r = r - period;
        // rounding can land on -halfPeriod, which the half-open range excludes: the nearest value inside is halfPeriod
        if ( !( -halfPeriod < r ) )
            r = halfPeriod;
    }
    return r;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_FORCE_INLINE T
Sin( const CTRadian< T >& angle ) GUCEF_MATH_REALTIME_CONTEXT
{
    GUCEF_MATH_STATIC_ASSERT( CTNumericTraits< T >::HasTranscendentals, Rotation_from_angles_requires_transcendentals );
    return CTNumericTraits< T >::Sin( angle.GetValue() );
}

template < typename T >
GUCEF_MATH_FORCE_INLINE T
Cos( const CTRadian< T >& angle ) GUCEF_MATH_REALTIME_CONTEXT
{
    GUCEF_MATH_STATIC_ASSERT( CTNumericTraits< T >::HasTranscendentals, Rotation_from_angles_requires_transcendentals );
    return CTNumericTraits< T >::Cos( angle.GetValue() );
}

template < typename T >
GUCEF_MATH_FORCE_INLINE T
Tan( const CTRadian< T >& angle ) GUCEF_MATH_REALTIME_CONTEXT
{
    GUCEF_MATH_STATIC_ASSERT( CTNumericTraits< T >::HasTranscendentals, Rotation_from_angles_requires_transcendentals );
    return CTNumericTraits< T >::Tan( angle.GetValue() );
}

template < typename T > GUCEF_MATH_FORCE_INLINE T Sin( const CTDegree< T >& angle ) GUCEF_MATH_REALTIME_CONTEXT { return Sin( CTRadian< T >( angle ) ); }
template < typename T > GUCEF_MATH_FORCE_INLINE T Cos( const CTDegree< T >& angle ) GUCEF_MATH_REALTIME_CONTEXT { return Cos( CTRadian< T >( angle ) ); }
template < typename T > GUCEF_MATH_FORCE_INLINE T Tan( const CTDegree< T >& angle ) GUCEF_MATH_REALTIME_CONTEXT { return Tan( CTRadian< T >( angle ) ); }

/*-------------------------------------------------------------------------*/

/** asin in [ -pi/2, pi/2 ] */
template < typename T >
GUCEF_MATH_FORCE_INLINE CTRadian< T >
Asin( const T& value ) GUCEF_MATH_REALTIME_CONTEXT
{
    return CTRadian< T >( CTNumericTraits< T >::Asin( value ) );
}

/** acos in [ 0, pi ] */
template < typename T >
GUCEF_MATH_FORCE_INLINE CTRadian< T >
Acos( const T& value ) GUCEF_MATH_REALTIME_CONTEXT
{
    return CTRadian< T >( CTNumericTraits< T >::Acos( value ) );
}

/** atan in ( -pi/2, pi/2 ) */
template < typename T >
GUCEF_MATH_FORCE_INLINE CTRadian< T >
Atan( const T& value ) GUCEF_MATH_REALTIME_CONTEXT
{
    return CTRadian< T >( CTNumericTraits< T >::Atan( value ) );
}

/** atan2 in ( -pi, pi ] */
template < typename T >
GUCEF_MATH_FORCE_INLINE CTRadian< T >
Atan2( const T& y, const T& x ) GUCEF_MATH_REALTIME_CONTEXT
{
    return CTRadian< T >( CTNumericTraits< T >::Atan2( y, x ) );
}

/*-------------------------------------------------------------------------*/

/** angle wrapped to ( -pi, pi ] */
template < typename T >
GUCEF_MATH_PRECISE_INLINE CTRadian< T >
WrapAngle( const CTRadian< T >& angle ) GUCEF_MATH_REALTIME_CONTEXT
{
    return CTRadian< T >( WrapPeriodicSigned( angle.GetValue(), CTMathConstants< T >::TwoPi(), CTMathConstants< T >::Pi() ) );
}

/** angle wrapped to ( -180, 180 ] */
template < typename T >
GUCEF_MATH_PRECISE_INLINE CTDegree< T >
WrapAngle( const CTDegree< T >& angle ) GUCEF_MATH_REALTIME_CONTEXT
{
    typedef CTNumericTraits< T > TTraits;
    return CTDegree< T >( WrapPeriodicSigned( angle.GetValue(), TTraits::FromInt32( 360 ), TTraits::FromInt32( 180 ) ) );
}

/*-------------------------------------------------------------------------*/

/** angle wrapped to [ 0, 2 pi ) */
template < typename T >
GUCEF_MATH_PRECISE_INLINE CTRadian< T >
WrapAnglePositive( const CTRadian< T >& angle ) GUCEF_MATH_REALTIME_CONTEXT
{
    return CTRadian< T >( CTNumericTraits< T >::ModFloor( angle.GetValue(), CTMathConstants< T >::TwoPi() ) );
}

/** angle wrapped to [ 0, 360 ) */
template < typename T >
GUCEF_MATH_PRECISE_INLINE CTDegree< T >
WrapAnglePositive( const CTDegree< T >& angle ) GUCEF_MATH_REALTIME_CONTEXT
{
    return CTDegree< T >( CTNumericTraits< T >::ModFloor( angle.GetValue(), CTNumericTraits< T >::FromInt32( 360 ) ) );
}

/*-------------------------------------------------------------------------*/

/** Shortest signed difference to - from, in ( -pi, pi ] */
template < typename T >
GUCEF_MATH_PRECISE_INLINE CTRadian< T >
DeltaAngle( const CTRadian< T >& from, const CTRadian< T >& to ) GUCEF_MATH_REALTIME_CONTEXT
{
    return WrapAngle( to - from );
}

/** Shortest signed difference to - from, in ( -180, 180 ] */
template < typename T >
GUCEF_MATH_PRECISE_INLINE CTDegree< T >
DeltaAngle( const CTDegree< T >& from, const CTDegree< T >& to ) GUCEF_MATH_REALTIME_CONTEXT
{
    return WrapAngle( to - from );
}

/*-------------------------------------------------------------------------*/

/** a + DeltaAngle( a, b ) * t: interpolates along the shortest arc; t is not clamped and the result is not wrapped */
template < typename T >
GUCEF_MATH_PRECISE_INLINE CTRadian< T >
LerpAngle( const CTRadian< T >& a, const CTRadian< T >& b, const T& t ) GUCEF_MATH_REALTIME_CONTEXT
{
    return a + DeltaAngle( a, b ) * t;
}

template < typename T >
GUCEF_MATH_PRECISE_INLINE CTDegree< T >
LerpAngle( const CTDegree< T >& a, const CTDegree< T >& b, const T& t ) GUCEF_MATH_REALTIME_CONTEXT
{
    return a + DeltaAngle( a, b ) * t;
}

/*-------------------------------------------------------------------------*/

/** Moves current toward target along the shortest arc by at most maxDelta (>= 0) without overshooting */
template < typename T >
GUCEF_MATH_PRECISE_INLINE CTRadian< T >
MoveTowardsAngle( const CTRadian< T >& current, const CTRadian< T >& target, const CTRadian< T >& maxDelta ) GUCEF_MATH_REALTIME_CONTEXT
{
    CTRadian< T > delta = DeltaAngle( current, target );
    if ( -maxDelta < delta && delta < maxDelta )
        return target;
    return CTRadian< T >( MoveTowards( current.GetValue(), ( current + delta ).GetValue(), maxDelta.GetValue() ) );
}

template < typename T >
GUCEF_MATH_PRECISE_INLINE CTDegree< T >
MoveTowardsAngle( const CTDegree< T >& current, const CTDegree< T >& target, const CTDegree< T >& maxDelta ) GUCEF_MATH_REALTIME_CONTEXT
{
    CTDegree< T > delta = DeltaAngle( current, target );
    if ( -maxDelta < delta && delta < maxDelta )
        return target;
    return CTDegree< T >( MoveTowards( current.GetValue(), ( current + delta ).GetValue(), maxDelta.GetValue() ) );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      EXPORTED INSTANTIATIONS                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/* For Float32 and Float64 the module's instantiations (gucefMATH_AngleFunctions.cpp) are used, see section 5.10 */
#ifdef GUCEF_MATH_EXTERN_TEMPLATES_ENABLED
GUCEF_MATH_EXTERN_TEMPLATE Float32 WrapPeriodicSigned< Float32 >( const Float32&, const Float32&, const Float32& ) GUCEF_MATH_REALTIME_CONTEXT;
GUCEF_MATH_EXTERN_TEMPLATE CTRadian< Float32 > WrapAngle< Float32 >( const CTRadian< Float32 >& ) GUCEF_MATH_REALTIME_CONTEXT;
GUCEF_MATH_EXTERN_TEMPLATE CTRadian< Float32 > WrapAnglePositive< Float32 >( const CTRadian< Float32 >& ) GUCEF_MATH_REALTIME_CONTEXT;
GUCEF_MATH_EXTERN_TEMPLATE CTRadian< Float32 > DeltaAngle< Float32 >( const CTRadian< Float32 >&, const CTRadian< Float32 >& ) GUCEF_MATH_REALTIME_CONTEXT;
GUCEF_MATH_EXTERN_TEMPLATE CTRadian< Float32 > LerpAngle< Float32 >( const CTRadian< Float32 >&, const CTRadian< Float32 >&, const Float32& ) GUCEF_MATH_REALTIME_CONTEXT;
GUCEF_MATH_EXTERN_TEMPLATE CTRadian< Float32 > MoveTowardsAngle< Float32 >( const CTRadian< Float32 >&, const CTRadian< Float32 >&, const CTRadian< Float32 >& ) GUCEF_MATH_REALTIME_CONTEXT;
GUCEF_MATH_EXTERN_TEMPLATE CTDegree< Float32 > WrapAngle< Float32 >( const CTDegree< Float32 >& ) GUCEF_MATH_REALTIME_CONTEXT;
GUCEF_MATH_EXTERN_TEMPLATE CTDegree< Float32 > WrapAnglePositive< Float32 >( const CTDegree< Float32 >& ) GUCEF_MATH_REALTIME_CONTEXT;
GUCEF_MATH_EXTERN_TEMPLATE CTDegree< Float32 > DeltaAngle< Float32 >( const CTDegree< Float32 >&, const CTDegree< Float32 >& ) GUCEF_MATH_REALTIME_CONTEXT;
GUCEF_MATH_EXTERN_TEMPLATE CTDegree< Float32 > LerpAngle< Float32 >( const CTDegree< Float32 >&, const CTDegree< Float32 >&, const Float32& ) GUCEF_MATH_REALTIME_CONTEXT;
GUCEF_MATH_EXTERN_TEMPLATE CTDegree< Float32 > MoveTowardsAngle< Float32 >( const CTDegree< Float32 >&, const CTDegree< Float32 >&, const CTDegree< Float32 >& ) GUCEF_MATH_REALTIME_CONTEXT;
GUCEF_MATH_EXTERN_TEMPLATE Float64 WrapPeriodicSigned< Float64 >( const Float64&, const Float64&, const Float64& ) GUCEF_MATH_REALTIME_CONTEXT;
GUCEF_MATH_EXTERN_TEMPLATE CTRadian< Float64 > WrapAngle< Float64 >( const CTRadian< Float64 >& ) GUCEF_MATH_REALTIME_CONTEXT;
GUCEF_MATH_EXTERN_TEMPLATE CTRadian< Float64 > WrapAnglePositive< Float64 >( const CTRadian< Float64 >& ) GUCEF_MATH_REALTIME_CONTEXT;
GUCEF_MATH_EXTERN_TEMPLATE CTRadian< Float64 > DeltaAngle< Float64 >( const CTRadian< Float64 >&, const CTRadian< Float64 >& ) GUCEF_MATH_REALTIME_CONTEXT;
GUCEF_MATH_EXTERN_TEMPLATE CTRadian< Float64 > LerpAngle< Float64 >( const CTRadian< Float64 >&, const CTRadian< Float64 >&, const Float64& ) GUCEF_MATH_REALTIME_CONTEXT;
GUCEF_MATH_EXTERN_TEMPLATE CTRadian< Float64 > MoveTowardsAngle< Float64 >( const CTRadian< Float64 >&, const CTRadian< Float64 >&, const CTRadian< Float64 >& ) GUCEF_MATH_REALTIME_CONTEXT;
GUCEF_MATH_EXTERN_TEMPLATE CTDegree< Float64 > WrapAngle< Float64 >( const CTDegree< Float64 >& ) GUCEF_MATH_REALTIME_CONTEXT;
GUCEF_MATH_EXTERN_TEMPLATE CTDegree< Float64 > WrapAnglePositive< Float64 >( const CTDegree< Float64 >& ) GUCEF_MATH_REALTIME_CONTEXT;
GUCEF_MATH_EXTERN_TEMPLATE CTDegree< Float64 > DeltaAngle< Float64 >( const CTDegree< Float64 >&, const CTDegree< Float64 >& ) GUCEF_MATH_REALTIME_CONTEXT;
GUCEF_MATH_EXTERN_TEMPLATE CTDegree< Float64 > LerpAngle< Float64 >( const CTDegree< Float64 >&, const CTDegree< Float64 >&, const Float64& ) GUCEF_MATH_REALTIME_CONTEXT;
GUCEF_MATH_EXTERN_TEMPLATE CTDegree< Float64 > MoveTowardsAngle< Float64 >( const CTDegree< Float64 >&, const CTDegree< Float64 >&, const CTDegree< Float64 >& ) GUCEF_MATH_REALTIME_CONTEXT;
#endif /* GUCEF_MATH_EXTERN_TEMPLATES_ENABLED ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace MATH */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MATH_ANGLEFUNCTIONS_H ? */
