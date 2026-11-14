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

#ifndef GUCEF_MATH_CTMATHCONSTANTS_H
#define GUCEF_MATH_CTMATHCONSTANTS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MATH_CTNUMERICTRAITS_H
#include "gucefMATH_CTNumericTraits.h"
#define GUCEF_MATH_CTNUMERICTRAITS_H
#endif /* GUCEF_MATH_CTNUMERICTRAITS_H ? */

#ifndef GUCEF_MATH_CTNUMERICTRAITSFIXEDPOINT_H
#include "gucefMATH_CTNumericTraitsFixedPoint.h"
#define GUCEF_MATH_CTNUMERICTRAITSFIXEDPOINT_H
#endif /* GUCEF_MATH_CTNUMERICTRAITSFIXEDPOINT_H ? */

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
 *  Mathematical constants of a kind with transcendental functions, each the nearest representable value.
 *  Floats convert decimal literals of 36 digits (for Float32 the double literal rounds to the nearest float for every
 *  constant here, which the tests pin); fixed point derives each constant from the integer literals of
 *  gucefMATH_FixedPointFunctions.h by one rounding shift.
 */
template < typename T >
class CTMathConstants
{
    public:

    static GUCEF_MATH_FORCE_INLINE T Pi( void ) GUCEF_MATH_REALTIME_CONTEXT { CheckKind(); return T( 3.14159265358979323846264338327950288 ); }
    static GUCEF_MATH_FORCE_INLINE T TwoPi( void ) GUCEF_MATH_REALTIME_CONTEXT { return T( 6.28318530717958647692528676655900577 ); }
    static GUCEF_MATH_FORCE_INLINE T HalfPi( void ) GUCEF_MATH_REALTIME_CONTEXT { return T( 1.57079632679489661923132169163975144 ); }
    static GUCEF_MATH_FORCE_INLINE T QuarterPi( void ) GUCEF_MATH_REALTIME_CONTEXT { return T( 0.78539816339744830961566084581987572 ); }
    static GUCEF_MATH_FORCE_INLINE T InversePi( void ) GUCEF_MATH_REALTIME_CONTEXT { return T( 0.31830988618379067153776752674502872 ); }
    static GUCEF_MATH_FORCE_INLINE T E( void ) GUCEF_MATH_REALTIME_CONTEXT { return T( 2.71828182845904523536028747135266250 ); }
    static GUCEF_MATH_FORCE_INLINE T Ln2( void ) GUCEF_MATH_REALTIME_CONTEXT { return T( 0.69314718055994530941723212145817657 ); }
    static GUCEF_MATH_FORCE_INLINE T Ln10( void ) GUCEF_MATH_REALTIME_CONTEXT { return T( 2.30258509299404568401799145468436421 ); }
    static GUCEF_MATH_FORCE_INLINE T Sqrt2( void ) GUCEF_MATH_REALTIME_CONTEXT { return T( 1.41421356237309504880168872420969808 ); }
    static GUCEF_MATH_FORCE_INLINE T InverseSqrt2( void ) GUCEF_MATH_REALTIME_CONTEXT { return T( 0.70710678118654752440084436210484904 ); }
    static GUCEF_MATH_FORCE_INLINE T Sqrt3( void ) GUCEF_MATH_REALTIME_CONTEXT { return T( 1.73205080756887729352744634150587237 ); }
    static GUCEF_MATH_FORCE_INLINE T GoldenRatio( void ) GUCEF_MATH_REALTIME_CONTEXT { return T( 1.61803398874989484820458683436563812 ); }
    static GUCEF_MATH_FORCE_INLINE T DegreesPerRadian( void ) GUCEF_MATH_REALTIME_CONTEXT { return T( 57.2957795130823208767981548141051703 ); }
    static GUCEF_MATH_FORCE_INLINE T RadiansPerDegree( void ) GUCEF_MATH_REALTIME_CONTEXT { return T( 0.01745329251994329576923690768488612713 ); }

    private:

    static GUCEF_MATH_FORCE_INLINE void CheckKind( void ) GUCEF_MATH_REALTIME_CONTEXT
    {
        GUCEF_MATH_STATIC_ASSERT( CTNumericTraits< T >::IsFloatingPoint, Math_constants_require_a_kind_with_transcendentals );
    }
};

/*-------------------------------------------------------------------------*/

template < typename TStorage, UInt32 F >
class CTMathConstants< CTFixedPoint< TStorage, F > >
{
    public:

    typedef CTFixedPoint< TStorage, F > TValue;

    static GUCEF_MATH_FORCE_INLINE TValue Pi( void ) GUCEF_MATH_REALTIME_CONTEXT { return FixedFromLiteral< TStorage, F >( FixedQ61Pi(), 61 ); }
    static GUCEF_MATH_FORCE_INLINE TValue TwoPi( void ) GUCEF_MATH_REALTIME_CONTEXT { return FixedFromLiteral< TStorage, F >( FixedQ61Pi(), 60 ); }
    static GUCEF_MATH_FORCE_INLINE TValue HalfPi( void ) GUCEF_MATH_REALTIME_CONTEXT { return FixedFromLiteral< TStorage, F >( FixedQ61Pi(), 62 ); }
    static GUCEF_MATH_FORCE_INLINE TValue QuarterPi( void ) GUCEF_MATH_REALTIME_CONTEXT { return FixedFromLiteral< TStorage, F >( FixedQ61Pi(), 63 ); }
    static GUCEF_MATH_FORCE_INLINE TValue InversePi( void ) GUCEF_MATH_REALTIME_CONTEXT { return FixedFromLiteral< TStorage, F >( FixedQ61InversePi(), 61 ); }
    static GUCEF_MATH_FORCE_INLINE TValue E( void ) GUCEF_MATH_REALTIME_CONTEXT { return FixedFromLiteral< TStorage, F >( FixedQ61E(), 61 ); }
    static GUCEF_MATH_FORCE_INLINE TValue Ln2( void ) GUCEF_MATH_REALTIME_CONTEXT { return FixedFromLiteral< TStorage, F >( FixedQ61Ln2(), 61 ); }
    static GUCEF_MATH_FORCE_INLINE TValue Ln10( void ) GUCEF_MATH_REALTIME_CONTEXT { return FixedFromLiteral< TStorage, F >( FixedQ61Ln10(), 61 ); }
    static GUCEF_MATH_FORCE_INLINE TValue Sqrt2( void ) GUCEF_MATH_REALTIME_CONTEXT { return FixedFromLiteral< TStorage, F >( FixedQ61Sqrt2(), 61 ); }
    static GUCEF_MATH_FORCE_INLINE TValue InverseSqrt2( void ) GUCEF_MATH_REALTIME_CONTEXT { return FixedFromLiteral< TStorage, F >( FixedQ61InverseSqrt2(), 61 ); }
    static GUCEF_MATH_FORCE_INLINE TValue Sqrt3( void ) GUCEF_MATH_REALTIME_CONTEXT { return FixedFromLiteral< TStorage, F >( FixedQ61Sqrt3(), 61 ); }
    static GUCEF_MATH_FORCE_INLINE TValue GoldenRatio( void ) GUCEF_MATH_REALTIME_CONTEXT { return FixedFromLiteral< TStorage, F >( FixedQ61GoldenRatio(), 61 ); }
    static GUCEF_MATH_FORCE_INLINE TValue DegreesPerRadian( void ) GUCEF_MATH_REALTIME_CONTEXT { return FixedFromLiteral< TStorage, F >( FixedQ57DegreesPerRadian(), 57 ); }
    static GUCEF_MATH_FORCE_INLINE TValue RadiansPerDegree( void ) GUCEF_MATH_REALTIME_CONTEXT { return FixedFromUnsignedLiteral< TStorage, F >( FixedQ64RadiansPerDegree(), 64 ); }
};

/*-------------------------------------------------------------------------*/

/**
 *  Conversion between degrees and radians per kind: a multiplication by the constant for floats; for fixed point one
 *  rounding of the exact product with the high precision literal (a multiplication by the format's pi / 180 would
 *  lose up to half a raw unit per degree).
 */
template < typename T >
class CTAngleUnitConversion
{
    public:
    static GUCEF_MATH_FORCE_INLINE T DegreesToRadians( const T& degrees ) GUCEF_MATH_REALTIME_CONTEXT { return degrees * CTMathConstants< T >::RadiansPerDegree(); }
    static GUCEF_MATH_FORCE_INLINE T RadiansToDegrees( const T& radians ) GUCEF_MATH_REALTIME_CONTEXT { return radians * CTMathConstants< T >::DegreesPerRadian(); }
};

template < typename TStorage, UInt32 F >
class CTAngleUnitConversion< CTFixedPoint< TStorage, F > >
{
    public:
    typedef CTFixedPoint< TStorage, F > T;
    static GUCEF_MATH_FORCE_INLINE T DegreesToRadians( const T& degrees ) GUCEF_MATH_REALTIME_CONTEXT { return FixedDegreesToRadians( degrees ); }
    static GUCEF_MATH_FORCE_INLINE T RadiansToDegrees( const T& radians ) GUCEF_MATH_REALTIME_CONTEXT { return FixedRadiansToDegrees( radians ); }
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

#endif /* GUCEF_MATH_CTMATHCONSTANTS_H ? */
