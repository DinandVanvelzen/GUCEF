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

#ifndef GUCEF_MATH_CTNUMERICTRAITSFIXEDPOINT_H
#define GUCEF_MATH_CTNUMERICTRAITSFIXEDPOINT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MATH_CTNUMERICTRAITS_H
#include "gucefMATH_CTNumericTraits.h"
#define GUCEF_MATH_CTNUMERICTRAITS_H
#endif /* GUCEF_MATH_CTNUMERICTRAITS_H ? */

#ifndef GUCEF_MATH_FIXEDPOINTFUNCTIONS_H
#include "gucefMATH_FixedPointFunctions.h"
#define GUCEF_MATH_FIXEDPOINTFUNCTIONS_H
#endif /* GUCEF_MATH_FIXEDPOINTFUNCTIONS_H ? */

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
 *  Numeric traits of CTFixedPoint (see CTNumericTraits for the contract). Every operation is deterministic:
 *  the same raw bits on every platform. Epsilon is one raw unit; the default absolute tolerance is 2 raw units for
 *  Int32 storage and 16 raw units for Int64 storage, the default relative tolerance 0.
 */
template < typename TStorage, UInt32 F >
class CTNumericTraits< CTFixedPoint< TStorage, F > >
{
    public:

    typedef CTFixedPoint< TStorage, F > TValue;
    typedef TValue TWide;
    typedef typename CTFixedPointKernels< TStorage >::TFloat TFloat;   /**< Float32 for Int32 storage, Float64 for Int64 */

    enum
    {
        IsFloatingPoint = 0,
        IsFixedPoint = 1,
        IsInteger = 0,
        IsSigned = 1,
        IsExact = 0,
        HasSqrt = 1,
        HasTranscendentals = 1,
        HasDivision = 1,
        IsDeterministic = 1,
        IsRealtimeSafe = 1,
        IsSimdCandidate = 0
    };

    static GUCEF_MATH_FORCE_INLINE TValue Zero( void ) GUCEF_MATH_REALTIME_CONTEXT { return TValue(); }
    static GUCEF_MATH_FORCE_INLINE TValue One( void ) GUCEF_MATH_REALTIME_CONTEXT { return TValue( 1 ); }
    static GUCEF_MATH_FORCE_INLINE TValue Two( void ) GUCEF_MATH_REALTIME_CONTEXT { return TValue( 2 ); }
    static GUCEF_MATH_FORCE_INLINE TValue Half( void ) GUCEF_MATH_REALTIME_CONTEXT { return TValue::FromRaw( (TStorage) ( F > 0 ? (TStorage) 1 << ( F - 1 ) : 0 ) ); }
    static GUCEF_MATH_FORCE_INLINE TValue Epsilon( void ) GUCEF_MATH_REALTIME_CONTEXT { return TValue::FromRaw( 1 ); }
    static GUCEF_MATH_FORCE_INLINE TValue DefaultAbsoluteTolerance( void ) GUCEF_MATH_REALTIME_CONTEXT { return TValue::FromRaw( sizeof( TStorage ) == 4 ? 2 : 16 ); }
    static GUCEF_MATH_FORCE_INLINE TValue DefaultRelativeTolerance( void ) GUCEF_MATH_REALTIME_CONTEXT { return TValue(); }
    static GUCEF_MATH_FORCE_INLINE TValue Lowest( void ) GUCEF_MATH_REALTIME_CONTEXT { return TValue::Lowest(); }
    static GUCEF_MATH_FORCE_INLINE TValue Max( void ) GUCEF_MATH_REALTIME_CONTEXT { return TValue::Highest(); }

    /** |x|; the magnitude of the lowest value is not representable and wraps to the lowest value */
    static GUCEF_MATH_FORCE_INLINE TValue Abs( const TValue& x ) GUCEF_MATH_REALTIME_CONTEXT { return x.GetRaw() < 0 ? -x : x; }
    static GUCEF_MATH_FORCE_INLINE TValue Min( const TValue& a, const TValue& b ) GUCEF_MATH_REALTIME_CONTEXT { return b < a ? b : a; }
    static GUCEF_MATH_FORCE_INLINE TValue Max( const TValue& a, const TValue& b ) GUCEF_MATH_REALTIME_CONTEXT { return a < b ? b : a; }
    static GUCEF_MATH_FORCE_INLINE bool IsNegative( const TValue& x ) GUCEF_MATH_REALTIME_CONTEXT { return x.GetRaw() < 0; }
    static GUCEF_MATH_FORCE_INLINE TValue Sign( const TValue& x ) GUCEF_MATH_REALTIME_CONTEXT { return x.GetRaw() < 0 ? TValue( -1 ) : ( 0 == x.GetRaw() ? TValue() : TValue( 1 ) ); }

    /** Integral part toward -infinity: the low fraction bits cleared (exact in two's complement) */
    static GUCEF_MATH_FORCE_INLINE TValue Floor( const TValue& x ) GUCEF_MATH_REALTIME_CONTEXT
    {
        return TValue::FromRaw( (TStorage) ( x.GetRaw() & ~(TStorage) ( ( (UInt64) 1 << F ) - 1 ) ) );
    }

    /** Integral part toward +infinity; saturates at the top of the range */
    static TValue Ceil( const TValue& x ) GUCEF_MATH_REALTIME_CONTEXT
    {
        return Round( x, CORE::ROUNDINGMODE_CEILING );
    }

    static TValue Truncate( const TValue& x ) GUCEF_MATH_REALTIME_CONTEXT
    {
        return Round( x, CORE::ROUNDINGMODE_TOWARD_ZERO );
    }

    /** Integral value rounded with 'mode', saturating; ROUNDINGMODE_UNNECESSARY truncates */
    static TValue Round( const TValue& x, TRoundingMode mode ) GUCEF_MATH_REALTIME_CONTEXT
    {
        if ( !MathIsValidRoundingMode( mode ) || CORE::ROUNDINGMODE_UNNECESSARY == mode )
            mode = CORE::ROUNDINGMODE_TOWARD_ZERO;
        Int64 whole = 0;
        MathRoundPartsToSigned( x.ToParts(), 0, 64, mode, whole );
        TValue result = x;
        TValue::FromParts( MathMakeValueParts( whole < 0, MathMagnitude( whole ), 0, 1 ), CORE::ROUNDINGMODE_TOWARD_ZERO, result );
        return result;
    }

    /** Floored modulo on the raw values (exact), in [0, period) for a positive period; period must not be 0 */
    static TValue ModFloor( const TValue& x, const TValue& period ) GUCEF_MATH_REALTIME_CONTEXT
    {
        Int64 p = (Int64) period.GetRaw();
        if ( -1 == p )
            return TValue();       // every raw value is a multiple of one negative raw unit; the lowest % -1 would trap
        Int64 r = (Int64) x.GetRaw() % p;
        if ( 0 != r && ( r < 0 ) != ( p < 0 ) )
            r += p;
        return TValue::FromRaw( (TStorage) r );
    }

    static GUCEF_MATH_FORCE_INLINE TValue Sqrt( const TValue& x ) GUCEF_MATH_REALTIME_CONTEXT { return FixedSqrt( x ); }
    static GUCEF_MATH_FORCE_INLINE TValue InverseSqrt( const TValue& x ) GUCEF_MATH_REALTIME_CONTEXT { return FixedInverseSqrt( x ); }
    static GUCEF_MATH_FORCE_INLINE TValue Sin( const TValue& x ) GUCEF_MATH_REALTIME_CONTEXT { return FixedSin( x ); }
    static GUCEF_MATH_FORCE_INLINE TValue Cos( const TValue& x ) GUCEF_MATH_REALTIME_CONTEXT { return FixedCos( x ); }
    static GUCEF_MATH_FORCE_INLINE TValue Tan( const TValue& x ) GUCEF_MATH_REALTIME_CONTEXT { return FixedTan( x ); }
    static GUCEF_MATH_FORCE_INLINE TValue Asin( const TValue& x ) GUCEF_MATH_REALTIME_CONTEXT { return FixedAsin( x ); }
    static GUCEF_MATH_FORCE_INLINE TValue Acos( const TValue& x ) GUCEF_MATH_REALTIME_CONTEXT { return FixedAcos( x ); }
    static GUCEF_MATH_FORCE_INLINE TValue Atan( const TValue& x ) GUCEF_MATH_REALTIME_CONTEXT { return FixedAtan( x ); }
    static GUCEF_MATH_FORCE_INLINE TValue Atan2( const TValue& y, const TValue& x ) GUCEF_MATH_REALTIME_CONTEXT { return FixedAtan2( y, x ); }
    static GUCEF_MATH_FORCE_INLINE TValue Exp( const TValue& x ) GUCEF_MATH_REALTIME_CONTEXT { return FixedExp( x ); }
    static GUCEF_MATH_FORCE_INLINE TValue Log( const TValue& x ) GUCEF_MATH_REALTIME_CONTEXT { return FixedLog( x ); }
    static GUCEF_MATH_FORCE_INLINE TValue Exp2( const TValue& x ) GUCEF_MATH_REALTIME_CONTEXT { return FixedExp2( x ); }
    static GUCEF_MATH_FORCE_INLINE TValue Log2( const TValue& x ) GUCEF_MATH_REALTIME_CONTEXT { return FixedLog2( x ); }
    static GUCEF_MATH_FORCE_INLINE TValue Pow( const TValue& a, const TValue& b ) GUCEF_MATH_REALTIME_CONTEXT { return FixedPow( a, b ); }

    static GUCEF_MATH_FORCE_INLINE TValue Pi( void ) GUCEF_MATH_REALTIME_CONTEXT { return FixedFromLiteral< TStorage, F >( FixedQ61Pi(), 61 ); }
    static GUCEF_MATH_FORCE_INLINE TValue TwoPi( void ) GUCEF_MATH_REALTIME_CONTEXT { return FixedFromLiteral< TStorage, F >( FixedQ61Pi(), 60 ); }
    static GUCEF_MATH_FORCE_INLINE TValue HalfPi( void ) GUCEF_MATH_REALTIME_CONTEXT { return FixedFromLiteral< TStorage, F >( FixedQ61Pi(), 62 ); }
    static GUCEF_MATH_FORCE_INLINE TValue E( void ) GUCEF_MATH_REALTIME_CONTEXT { return FixedFromLiteral< TStorage, F >( FixedQ61E(), 61 ); }
    static GUCEF_MATH_FORCE_INLINE TValue Ln2( void ) GUCEF_MATH_REALTIME_CONTEXT { return FixedFromLiteral< TStorage, F >( FixedQ61Ln2(), 61 ); }
    static GUCEF_MATH_FORCE_INLINE TValue Sqrt2( void ) GUCEF_MATH_REALTIME_CONTEXT { return FixedFromLiteral< TStorage, F >( FixedQ61Sqrt2(), 61 ); }

    static GUCEF_MATH_FORCE_INLINE TValue FromInt32( Int32 value ) GUCEF_MATH_REALTIME_CONTEXT { return TValue( value ); }

    static TMathStatus FromInt64( Int64 value, TRoundingMode mode, TValue& out ) GUCEF_MATH_REALTIME_CONTEXT
    {
        return TValue::FromParts( MathMakeValueParts( value < 0, MathMagnitude( value ), 0, 1 ), mode, out );
    }

    static GUCEF_MATH_FORCE_INLINE TMathStatus FromFloat64( Float64 value, TRoundingMode mode, TValue& out ) GUCEF_MATH_REALTIME_CONTEXT { return TValue::FromFloat64( value, mode, out ); }
    static GUCEF_MATH_FORCE_INLINE Float64 ToFloat64( const TValue& x ) GUCEF_MATH_REALTIME_CONTEXT { return x.ToFloat64(); }
    static GUCEF_MATH_FORCE_INLINE Float32 ToFloat32( const TValue& x ) GUCEF_MATH_REALTIME_CONTEXT { return x.ToFloat32(); }
    static GUCEF_MATH_FORCE_INLINE TNumericValueParts ToParts( const TValue& x ) GUCEF_MATH_REALTIME_CONTEXT { return x.ToParts(); }
    static GUCEF_MATH_FORCE_INLINE TMathStatus FromParts( const TNumericValueParts& parts, TRoundingMode mode, TValue& out ) GUCEF_MATH_REALTIME_CONTEXT { return TValue::FromParts( parts, mode, out ); }
    static GUCEF_MATH_FORCE_INLINE bool IsFinite( const TValue& /*x*/ ) GUCEF_MATH_REALTIME_CONTEXT { return true; }

    /** a * b + c with two roundings in that order */
    static GUCEF_MATH_FORCE_INLINE TValue MultiplyAdd( const TValue& a, const TValue& b, const TValue& c ) GUCEF_MATH_REALTIME_CONTEXT { return a * b + c; }

    static GUCEF_MATH_FORCE_INLINE CORE::CString ToString( const TValue& x ) { return x.ToString(); }
    static GUCEF_MATH_FORCE_INLINE bool FromString( const CORE::CString& text, TValue& out ) { return out.FromString( text ); }

    /** |a - b| <= max( absolute, relative * max( |a|, |b| ) ) on the raw values, without overflow */
    static bool EqualsWithin( const TValue& a, const TValue& b, const TValue& absoluteTolerance, const TValue& relativeTolerance ) GUCEF_MATH_REALTIME_CONTEXT
    {
        UInt64 distance = RawDistance( a, b );
        if ( absoluteTolerance.GetRaw() >= 0 && distance <= (UInt64) absoluteTolerance.GetRaw() )
            return true;
        if ( relativeTolerance.GetRaw() <= 0 )
            return false;
        TValue largest = Max( Abs( a ), Abs( b ) );
        TValue limit = TValue::MultiplySaturated( relativeTolerance, largest );
        return limit.GetRaw() >= 0 && distance <= (UInt64) limit.GetRaw();
    }

    /** Raw distance within 'ulps' units */
    static GUCEF_MATH_FORCE_INLINE bool EqualsUlp( const TValue& a, const TValue& b, UInt32 ulps ) GUCEF_MATH_REALTIME_CONTEXT
    {
        return RawDistance( a, b ) <= (UInt64) ulps;
    }

    private:

    static GUCEF_MATH_FORCE_INLINE UInt64 RawDistance( const TValue& a, const TValue& b ) GUCEF_MATH_REALTIME_CONTEXT
    {
        Int64 ra = (Int64) a.GetRaw();
        Int64 rb = (Int64) b.GetRaw();
        return ra < rb ? (UInt64) rb - (UInt64) ra : (UInt64) ra - (UInt64) rb;
    }
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace MATH */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MATH_CTNUMERICTRAITSFIXEDPOINT_H ? */
