/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
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

#ifndef GUCEF_CORE_CTFRACTIONINTEGEROPS_H
#define GUCEF_CORE_CTFRACTIONINTEGEROPS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_TYPES_H
#include "gucef_types.h"
#define GUCEF_TYPES_H
#endif /* GUCEF_TYPES_H ? */

#ifndef GUCEF_SFINAE_H
#include "gucef_SFINAE.h"
#define GUCEF_SFINAE_H
#endif /* GUCEF_SFINAE_H ? */

#ifndef GUCEF_CORE_CUINT128_H
#include "gucefCORE_CUInt128.h"
#define GUCEF_CORE_CUINT128_H
#endif /* GUCEF_CORE_CUINT128_H ? */

#ifndef GUCEF_CORE_CINT128_H
#include "gucefCORE_CInt128.h"
#define GUCEF_CORE_CINT128_H
#endif /* GUCEF_CORE_CINT128_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_CDECIMAL_H
#include "gucefCORE_CDecimal.h"
#define GUCEF_CORE_CDECIMAL_H
#endif /* GUCEF_CORE_CDECIMAL_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Text and decimal conversions of the CTFraction integer operations for the built-in integer types: the expressions
 *  CTFraction always used (CORE::ToString, StringToNumber, a CDecimal division of the Int64 values). The primary
 *  template and the Int64 specialization inherit them; a specialization for a class type provides its own.
 */
template < typename NumeratorType, typename DenominatorType >
class CTFractionIntegerOpsDefaults
{
    public:

    static CString ToString( const NumeratorType& n, const DenominatorType& d )
    {
        return CORE::ToString( n ) + "/" + CORE::ToString( d );
    }

    /** Parses both parts with StringToNumber (as always: text that is not a number reads as 0); false for a zero denominator */
    static bool FromString( const CString& numeratorText, const CString& denominatorText, NumeratorType& n, DenominatorType& d )
    {
        DenominatorType denominator = StringToNumber< DenominatorType >( denominatorText );
        if ( 0 == denominator )
            return false;
        n = StringToNumber< NumeratorType >( numeratorText );
        d = denominator;
        return true;
    }

    /** n / d rounded once to the context */
    static TDecimalStatus ToDecimal( const NumeratorType& n, const DenominatorType& d, const CDecimalContext& context, CDecimal& result )
    {
        Int64 numerator = static_cast< Int64 >( n );
        Int64 denominator = static_cast< Int64 >( d );
        // values that do not survive the round trip (unsigned above INT64_MAX) cannot be handed to the decimal
        if ( static_cast< NumeratorType >( numerator ) != n || static_cast< DenominatorType >( denominator ) != d )
            return DECIMALSTATUS_OVERFLOW;

        CDecimal whole;
        TDecimalStatus status = CDecimal::FromInt64( numerator, 0, whole );
        if ( DECIMALSTATUS_OK != status )
            return status;
        return whole.Divide( denominator, context, result );
    }

    /** unscaled / 10^scale, not yet in lowest terms. false when the parts do not fit the types */
    static bool FromDecimal( const CDecimal& value, NumeratorType& n, DenominatorType& d )
    {
        Int64 unscaled = value.GetUnscaledValue();
        Int64 denominator = 1;
        for ( UInt8 i=0; i<value.GetScale(); ++i )
            denominator *= 10;      // scale <= 18: 10^18 < 2^63

        NumeratorType numerator = static_cast< NumeratorType >( unscaled );
        DenominatorType scaledDenominator = static_cast< DenominatorType >( denominator );
        if ( static_cast< Int64 >( numerator ) != unscaled || static_cast< Int64 >( scaledDenominator ) != denominator )
            return false;
        n = numerator;
        d = scaledDenominator;
        return true;
    }
};

/*-------------------------------------------------------------------------*/

/**
 *  Integer operations policy of CTFraction.
 *
 *  Every function works on a numerator and a denominator by reference and returns false when the exact result
 *  does not fit the types (the outputs are then left unchanged). The primary template reproduces the expressions
 *  CTFraction always used for the built in integer types, bit for bit, and never reports an overflow.
 *  The denominator passed in is always positive for signed types and the fraction is in lowest terms.
 *
 *  The specialization for Int64 numerators and denominators uses CInt128 intermediates and a 128 bit gcd:
 *  cross products cannot overflow, the result is reduced to lowest terms and then narrowed with a check.
 *  The specialization lives in this header, which CTFraction includes, so that every translation unit sees the
 *  same instantiation.
 */
template < typename NumeratorType, typename DenominatorType >
class CTFractionIntegerOps : public CTFractionIntegerOpsDefaults< NumeratorType, DenominatorType >
{
    public:

    /** Euclid. Needs non-negative inputs for a positive result */
    template < typename T >
    static T Gcd( T a, T b )
    {
        T c;
        while ( b )
        {
            c = b;
            b = a % b;
            a = c;
        }
        return a;
    }

    /** Makes the denominator positive and reduces to lowest terms. The denominator must not be 0 */
    static bool Normalize( NumeratorType& n, DenominatorType& d )
    {
        MakeDenominatorPositive( n, d );

        // gcd requires non-negative inputs to return a correct positive result.
        // The denominator is >= 0 now, the numerator may be negative for signed types.
        NumeratorType absNumerator = n;
        if ( absNumerator < NumeratorType( 0 ) )
            absNumerator = NumeratorType( 0 ) - absNumerator;
        NumeratorType gcdValue = Gcd< NumeratorType >( absNumerator, static_cast< NumeratorType >( d ) );

        n /= gcdValue;
        d /= static_cast< DenominatorType >( gcdValue );
        return true;
    }

    static bool Add( NumeratorType& n, DenominatorType& d, const NumeratorType& otherN, const DenominatorType& otherD )
    {
        n = n * otherD + otherN * d;
        d *= otherD;
        return Normalize( n, d );
    }

    static bool Subtract( NumeratorType& n, DenominatorType& d, const NumeratorType& otherN, const DenominatorType& otherD )
    {
        n = n * otherD - otherN * d;
        d *= otherD;
        return Normalize( n, d );
    }

    static bool Multiply( NumeratorType& n, DenominatorType& d, const NumeratorType& otherN, const DenominatorType& otherD )
    {
        n *= otherN;
        d *= otherD;
        return Normalize( n, d );
    }

    /** otherN must not be 0 */
    static bool Divide( NumeratorType& n, DenominatorType& d, const NumeratorType& otherN, const DenominatorType& otherD )
    {
        n *= otherD;
        d *= otherN;
        return Normalize( n, d );
    }

    static bool AddScalar( NumeratorType& n, DenominatorType& d, NumeratorType value )
    {
        n += value * d;
        return Normalize( n, d );
    }

    static bool SubtractScalar( NumeratorType& n, DenominatorType& d, NumeratorType value )
    {
        n -= value * d;
        return Normalize( n, d );
    }

    static bool MultiplyScalar( NumeratorType& n, DenominatorType& d, NumeratorType value )
    {
        n *= value;
        return Normalize( n, d );
    }

    /** value must not be 0 */
    static bool DivideScalar( NumeratorType& n, DenominatorType& d, NumeratorType value )
    {
        d *= value;
        return Normalize( n, d );
    }

    /** n = whole + fractionN / fractionD */
    static bool FromMixed( NumeratorType& n, DenominatorType& d, NumeratorType whole, const NumeratorType& fractionN, const DenominatorType& fractionD )
    {
        n = whole * fractionD + fractionN;
        d = fractionD;
        return Normalize( n, d );
    }

    /** Cross product comparison: -1, 0, 1 */
    static Int32 Compare( const NumeratorType& n1, const DenominatorType& d1, const NumeratorType& n2, const DenominatorType& d2 )
    {
        if ( n1 * d2 < n2 * d1 )
            return -1;
        if ( n1 * d2 > n2 * d1 )
            return 1;
        return 0;
    }

    static Float32 ToFloat32( const NumeratorType& n, const DenominatorType& d )
    {
        return static_cast< Float32 >( n ) / d;
    }

    static Float64 ToFloat64( const NumeratorType& n, const DenominatorType& d )
    {
        return static_cast< Float64 >( n ) / d;
    }

    private:

    template < typename T >
    static typename EnableIf< TypeIsUnsigned< T >::value, void >::type
    MakeDenominatorPositive( NumeratorType& /*n*/, T& /*d*/ )
    {
        // Nothing to do here for unsigned denominators
    }

    template < typename T >
    static typename EnableIfNot< TypeIsUnsigned< T >::value, void >::type
    MakeDenominatorPositive( NumeratorType& n, T& d )
    {
        if ( d < 0 )
        {
            n = -n;
            d = -d;
        }
    }
};

/*-------------------------------------------------------------------------*/

/**
 *  Int64 / Int64 specialization with exact 128 bit intermediates, see the primary template.
 *  Reduction results that do not fit Int64 (a numerator below -2^63, above 2^63 - 1, a denominator above
 *  2^63 - 1) are reported as false and the operands are left unchanged.
 */
template <>
class CTFractionIntegerOps< Int64, Int64 > : public CTFractionIntegerOpsDefaults< Int64, Int64 >
{
    public:

    static bool Normalize( Int64& n, Int64& d )
    {
        return Reduce( CInt128( n ), CInt128( d ), n, d );
    }

    static bool Add( Int64& n, Int64& d, const Int64& otherN, const Int64& otherD )
    {
        CInt128 num = CInt128::Add( CInt128::Multiply( n, otherD ), CInt128::Multiply( otherN, d ) );
        return Reduce( num, CInt128::Multiply( d, otherD ), n, d );
    }

    static bool Subtract( Int64& n, Int64& d, const Int64& otherN, const Int64& otherD )
    {
        CInt128 num = CInt128::Subtract( CInt128::Multiply( n, otherD ), CInt128::Multiply( otherN, d ) );
        return Reduce( num, CInt128::Multiply( d, otherD ), n, d );
    }

    static bool Multiply( Int64& n, Int64& d, const Int64& otherN, const Int64& otherD )
    {
        return Reduce( CInt128::Multiply( n, otherN ), CInt128::Multiply( d, otherD ), n, d );
    }

    /** otherN must not be 0 */
    static bool Divide( Int64& n, Int64& d, const Int64& otherN, const Int64& otherD )
    {
        return Reduce( CInt128::Multiply( n, otherD ), CInt128::Multiply( d, otherN ), n, d );
    }

    static bool AddScalar( Int64& n, Int64& d, Int64 value )
    {
        return Reduce( CInt128::Add( CInt128( n ), CInt128::Multiply( value, d ) ), CInt128( d ), n, d );
    }

    static bool SubtractScalar( Int64& n, Int64& d, Int64 value )
    {
        return Reduce( CInt128::Subtract( CInt128( n ), CInt128::Multiply( value, d ) ), CInt128( d ), n, d );
    }

    static bool MultiplyScalar( Int64& n, Int64& d, Int64 value )
    {
        return Reduce( CInt128::Multiply( n, value ), CInt128( d ), n, d );
    }

    /** value must not be 0 */
    static bool DivideScalar( Int64& n, Int64& d, Int64 value )
    {
        return Reduce( CInt128( n ), CInt128::Multiply( d, value ), n, d );
    }

    static bool FromMixed( Int64& n, Int64& d, Int64 whole, const Int64& fractionN, const Int64& fractionD )
    {
        return Reduce( CInt128::Add( CInt128::Multiply( whole, fractionD ), CInt128( fractionN ) ), CInt128( fractionD ), n, d );
    }

    /** Exact: the cross products cannot overflow 128 bits and the denominators are positive */
    static Int32 Compare( const Int64& n1, const Int64& d1, const Int64& n2, const Int64& d2 )
    {
        return CInt128::Compare( CInt128::Multiply( n1, d2 ), CInt128::Multiply( n2, d1 ) );
    }

    static Float32 ToFloat32( const Int64& n, const Int64& d )
    {
        return static_cast< Float32 >( n ) / d;
    }

    static Float64 ToFloat64( const Int64& n, const Int64& d )
    {
        return static_cast< Float64 >( n ) / d;
    }

    /** 128 bit gcd of two magnitudes, Euclid with 128 by 128 division */
    static CUInt128 Gcd( CUInt128 a, CUInt128 b )
    {
        while ( !b.IsZero() )
        {
            CUInt128 q;
            CUInt128 r;
            CUInt128::DivMod( a, b, q, r );
            a = b;
            b = r;
        }
        return a;
    }

    /**
     *  Reduces num / den (den != 0, any sign) to lowest terms with a positive denominator and narrows to Int64.
     *  Returns false and leaves outN and outD unchanged when the reduced value does not fit.
     *  outN and outD may alias the inputs only through values already copied into num and den.
     */
    static bool Reduce( const CInt128& num, const CInt128& den, Int64& outN, Int64& outD )
    {
        CUInt128 magnitudeN = num.GetMagnitude();
        CUInt128 magnitudeD = den.GetMagnitude();
        bool negative = !num.IsZero() && ( num.IsNegative() != den.IsNegative() );

        CUInt128 g = Gcd( magnitudeN, magnitudeD );
        CUInt128 q;
        CUInt128 r;
        CUInt128::DivMod( magnitudeN, g, q, r );
        magnitudeN = q;
        CUInt128::DivMod( magnitudeD, g, q, r );
        magnitudeD = q;

        const UInt64 int64Max = 0x7FFFFFFFFFFFFFFFULL;
        if ( 0 != magnitudeD.high || magnitudeD.low > int64Max )
            return false;
        if ( 0 != magnitudeN.high )
            return false;
        if ( negative )
        {
            if ( magnitudeN.low > int64Max + 1 )
                return false;
            outN = (Int64) ( 0 - magnitudeN.low );
        }
        else
        {
            if ( magnitudeN.low > int64Max )
                return false;
            outN = (Int64) magnitudeN.low;
        }
        outD = (Int64) magnitudeD.low;
        return true;
    }
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CTFRACTIONINTEGEROPS_H ? */
