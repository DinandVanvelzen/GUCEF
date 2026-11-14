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

#ifndef GUCEF_MATH_COMPONENTFUNCTIONS_H
#define GUCEF_MATH_COMPONENTFUNCTIONS_H

/*
 *  Functions over the scalar components of value types (vectors, points, directions, quaternions, colours), shared
 *  so that every value type behaves the same:
 *  - CTMathComponentKernels: the arithmetic rules per family of kinds (wrapping integers, exact wide fixed point
 *    accumulation, overflow-free float lengths), with ComponentAdd / Subtract / Multiply / Negate for single values;
 *  - ComponentsToString / ComponentsFromString: the text form "(c0, c1, ...)" of section 9.1 of the design;
 *  - SerializeComponents / DeserializeComponents: one DataNode attribute per component (section 9.2): float kinds as
 *    Float32 / Float64 variants, integers as integer variants, fixed point and fractions as exact text.
 *  The kernels are realtime-safe; the text and DataNode functions allocate and are not.
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <float.h>

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_CINT128_H
#include "gucefCORE_CInt128.h"
#define GUCEF_CORE_CINT128_H
#endif /* GUCEF_CORE_CINT128_H ? */

#ifndef GUCEF_MATH_CTNUMERICTRAITS_H
#include "gucefMATH_CTNumericTraits.h"
#define GUCEF_MATH_CTNUMERICTRAITS_H
#endif /* GUCEF_MATH_CTNUMERICTRAITS_H ? */

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

template < typename TStorage, UInt32 FractionBits > class CTFixedPoint;

/*-------------------------------------------------------------------------*/

/** The arithmetic family of a numeric kind, which selects the component kernels */
enum EMathComponentKind
{
    MATHCOMPONENTKIND_FLOAT = 0,    /**< Float32, Float64: IEEE arithmetic, Chebyshev scaling against overflow and underflow */
    MATHCOMPONENTKIND_FIXED,        /**< CTFixedPoint: exact wide accumulation, one rounding, saturation */
    MATHCOMPONENTKIND_INTEGER,      /**< built-in integers: wrapping through the unsigned image, exact wide dot products */
    MATHCOMPONENTKIND_EXACT         /**< fractions: the operators of the kind are exact */
};

template < typename T >
class CTMathComponentKind
{
    public:

    typedef CTNumericTraits< T > TTraits;

    enum
    {
        Value = TTraits::IsFloatingPoint ? MATHCOMPONENTKIND_FLOAT :
                ( TTraits::IsFixedPoint ? MATHCOMPONENTKIND_FIXED :
                ( TTraits::IsInteger ? MATHCOMPONENTKIND_INTEGER : MATHCOMPONENTKIND_EXACT ) )
    };
};

/*-------------------------------------------------------------------------*/

/**
 *  Integer helper types: TUnsigned (the unsigned type of the same width), TWork (the unsigned type in which wrapping
 *  arithmetic is computed: at least 32 bits, so that the promotion to int never overflows) and TDotWide (the type that
 *  holds a dot product of up to 4 components exactly: 32 bits for 8 bit components, 64 bits for 16 bit components,
 *  CInt128 / CUInt128 for 32 and 64 bit components; for 64 bit components the dot product is exact while it stays
 *  below 2^127 in magnitude, that is for components below 2^62).
 */
template < typename T > class CTMathIntegerComponentTypes;
template <> class CTMathIntegerComponentTypes< Int8 > { public: typedef UInt8 TUnsigned; typedef UInt32 TWork; typedef Int32 TDotWide; };
template <> class CTMathIntegerComponentTypes< UInt8 > { public: typedef UInt8 TUnsigned; typedef UInt32 TWork; typedef UInt32 TDotWide; };
template <> class CTMathIntegerComponentTypes< Int16 > { public: typedef UInt16 TUnsigned; typedef UInt32 TWork; typedef Int64 TDotWide; };
template <> class CTMathIntegerComponentTypes< UInt16 > { public: typedef UInt16 TUnsigned; typedef UInt32 TWork; typedef UInt64 TDotWide; };
template <> class CTMathIntegerComponentTypes< Int32 > { public: typedef UInt32 TUnsigned; typedef UInt32 TWork; typedef CORE::CInt128 TDotWide; };
template <> class CTMathIntegerComponentTypes< UInt32 > { public: typedef UInt32 TUnsigned; typedef UInt32 TWork; typedef CORE::CUInt128 TDotWide; };
template <> class CTMathIntegerComponentTypes< Int64 > { public: typedef UInt64 TUnsigned; typedef UInt64 TWork; typedef CORE::CInt128 TDotWide; };
template <> class CTMathIntegerComponentTypes< UInt64 > { public: typedef UInt64 TUnsigned; typedef UInt64 TWork; typedef CORE::CUInt128 TDotWide; };

/*-------------------------------------------------------------------------*/

/** Accumulates exact integer products in the dot product type of CTMathIntegerComponentTypes */
template < typename TDotWide >
class CTMathWideAccumulator
{
    public:

    template < typename T >
    static GUCEF_MATH_FORCE_INLINE TDotWide Product( const T& a, const T& b ) GUCEF_MATH_REALTIME_CONTEXT { return (TDotWide) a * (TDotWide) b; }
    static GUCEF_MATH_FORCE_INLINE TDotWide Add( const TDotWide& a, const TDotWide& b ) GUCEF_MATH_REALTIME_CONTEXT { return a + b; }
    static GUCEF_MATH_FORCE_INLINE TDotWide Zero( void ) GUCEF_MATH_REALTIME_CONTEXT { return TDotWide( 0 ); }
};

template <>
class CTMathWideAccumulator< CORE::CInt128 >
{
    public:

    template < typename T >
    static GUCEF_MATH_FORCE_INLINE CORE::CInt128 Product( const T& a, const T& b ) GUCEF_MATH_REALTIME_CONTEXT { return CORE::CInt128::Multiply( (Int64) a, (Int64) b ); }
    static GUCEF_MATH_FORCE_INLINE CORE::CInt128 Add( const CORE::CInt128& a, const CORE::CInt128& b ) GUCEF_MATH_REALTIME_CONTEXT { return CORE::CInt128::Add( a, b ); }
    static GUCEF_MATH_FORCE_INLINE CORE::CInt128 Zero( void ) GUCEF_MATH_REALTIME_CONTEXT { return CORE::CInt128( (Int64) 0 ); }
};

template <>
class CTMathWideAccumulator< CORE::CUInt128 >
{
    public:

    template < typename T >
    static GUCEF_MATH_FORCE_INLINE CORE::CUInt128 Product( const T& a, const T& b ) GUCEF_MATH_REALTIME_CONTEXT { return CORE::CUInt128::Multiply( (UInt64) a, (UInt64) b ); }
    static GUCEF_MATH_FORCE_INLINE CORE::CUInt128 Add( const CORE::CUInt128& a, const CORE::CUInt128& b ) GUCEF_MATH_REALTIME_CONTEXT { return CORE::CUInt128::Add( a, b ); }
    static GUCEF_MATH_FORCE_INLINE CORE::CUInt128 Zero( void ) GUCEF_MATH_REALTIME_CONTEXT { return CORE::CUInt128( (UInt64) 0 ); }
};

/*-------------------------------------------------------------------------*/

/** floor( sqrt( value ) ) and the remainder value - root^2, by the restoring digit-by-digit method (no division) */
inline UInt64
MathSqrtFloor64( UInt64 value, UInt64& remainder ) GUCEF_MATH_REALTIME_CONTEXT
{
    UInt64 root = 0;
    UInt64 bit = (UInt64) 1 << 62;
    while ( bit > value )
        bit >>= 2;
    remainder = value;
    while ( 0 != bit )
    {
        if ( remainder >= root + bit )
        {
            remainder -= root + bit;
            root = ( root >> 1 ) + bit;
        }
        else
        {
            root >>= 1;
        }
        bit >>= 2;
    }
    return root;
}

/*-------------------------------------------------------------------------*/

/**
 *  sqrt( value ) rounded to the nearest integer, for every 128 bit value: the floor root s is incremented when
 *  value - s^2 > s (ties cannot occur for an integer radicand). At most 64 steps; deterministic on every platform.
 */
inline UInt64
MathSqrtRounded128( const CORE::CUInt128& value ) GUCEF_MATH_REALTIME_CONTEXT
{
    if ( 0 == value.high )
    {
        UInt64 remainder = 0;
        UInt64 root = MathSqrtFloor64( value.low, remainder );
        return remainder > root ? root + 1 : root;
    }
    CORE::CUInt128 remainder = value;
    CORE::CUInt128 root( (UInt64) 0 );
    CORE::CUInt128 bit = CORE::CUInt128::ShiftLeft( CORE::CUInt128( (UInt64) 1 ), 126 );
    while ( CORE::CUInt128::Compare( bit, value ) > 0 )
        bit = CORE::CUInt128::ShiftRight( bit, 2 );
    while ( !bit.IsZero() )
    {
        CORE::CUInt128 trial = CORE::CUInt128::Add( root, bit );
        if ( CORE::CUInt128::Compare( remainder, trial ) >= 0 )
        {
            remainder = CORE::CUInt128::Subtract( remainder, trial );
            root = CORE::CUInt128::Add( CORE::CUInt128::ShiftRight( root, 1 ), bit );
        }
        else
        {
            root = CORE::CUInt128::ShiftRight( root, 1 );
        }
        bit = CORE::CUInt128::ShiftRight( bit, 2 );
    }
    // root < 2^64 here
    return CORE::CUInt128::Compare( remainder, root ) > 0 ? root.low + 1 : root.low;
}

/*-------------------------------------------------------------------------*/

/**
 *  The component kernels: the arithmetic of the value types for one family of kinds (EMathComponentKind). Every value
 *  type computes through them, so that each family has one implementation of its rules:
 *  - Add, Subtract, Multiply, Negate: the operators of the kind; integers wrap through the unsigned image (defined
 *    behaviour, as the built-in unsigned types wrap);
 *  - Dot< N >: floats sum in component order; fixed point forms the exact sum of the raw products in 128 bits and
 *    rounds once (ties away from zero, saturating); integers wrap; fractions are exact;
 *  - DotWide< N > (integers): exact, in CTMathIntegerComponentTypes< T >::TDotWide;
 *  - Length< N >, Normalize< N >, ScaleToUnitChebyshev< N > (floats and fixed point): floats use the plain formula
 *    when the squared length is safely inside the range and otherwise divide by the largest magnitude first, so
 *    neither overflow nor underflow occurs; fixed point takes one correctly rounded 128 bit square root of the exact
 *    sum of squares, and normalises after dividing by the largest magnitude, which keeps every intermediate in range
 *    and every component at full precision.
 *  All functions are realtime-safe.
 */
template < typename T, int Kind >
class CTMathComponentKernels;

/*-------------------------------------------------------------------------*/

/** Shared by the kinds whose operators are the arithmetic: floats, fixed point (wrapping operators) and fractions */
template < typename T >
class CTMathOperatorComponentKernels
{
    public:

    typedef CTNumericTraits< T > TTraits;

    static GUCEF_MATH_FORCE_INLINE T Add( const T& a, const T& b ) GUCEF_MATH_REALTIME_CONTEXT { return a + b; }
    static GUCEF_MATH_FORCE_INLINE T Subtract( const T& a, const T& b ) GUCEF_MATH_REALTIME_CONTEXT { return a - b; }
    static GUCEF_MATH_FORCE_INLINE T Multiply( const T& a, const T& b ) GUCEF_MATH_REALTIME_CONTEXT { return a * b; }
};

/*-------------------------------------------------------------------------*/

template < typename T >
class CTMathComponentKernels< T, MATHCOMPONENTKIND_FLOAT > : public CTMathOperatorComponentKernels< T >
{
    public:

    typedef CTNumericTraits< T > TTraits;
    typedef T TDotWide;

    static GUCEF_MATH_FORCE_INLINE T Negate( const T& x ) GUCEF_MATH_REALTIME_CONTEXT { return -x; }

    template < UInt32 N >
    static GUCEF_MATH_FORCE_INLINE T Dot( const T ( &a )[ N ], const T ( &b )[ N ] ) GUCEF_MATH_REALTIME_CONTEXT
    {
        T sum = a[ 0 ] * b[ 0 ];
        for ( UInt32 i=1; i<N; ++i )
            sum = sum + a[ i ] * b[ i ];
        return sum;
    }

    template < UInt32 N >
    static GUCEF_MATH_FORCE_INLINE T DotWide( const T ( &a )[ N ], const T ( &b )[ N ] ) GUCEF_MATH_REALTIME_CONTEXT { return Dot( a, b ); }

    /**
     *  True when a squared length can be used as it is: finite, and large enough that no square lost precision to
     *  the subnormal range (at least the smallest normal number divided by epsilon)
     */
    static GUCEF_MATH_FORCE_INLINE bool IsSafeSquaredLength( const T& squaredLength ) GUCEF_MATH_REALTIME_CONTEXT
    {
        return squaredLength >= SafeSquaredLengthMinimum() && squaredLength <= TTraits::Max();
    }

    static GUCEF_MATH_FORCE_INLINE T SafeSquaredLengthMinimum( void ) GUCEF_MATH_REALTIME_CONTEXT
    {
        return sizeof( T ) == sizeof( Float32 ) ? (T) ( FLT_MIN / FLT_EPSILON ) : (T) ( DBL_MIN / DBL_EPSILON );
    }

    /** The largest magnitude; NaN when a component is NaN */
    template < UInt32 N >
    static GUCEF_MATH_FORCE_INLINE T MaxMagnitude( const T ( &a )[ N ] ) GUCEF_MATH_REALTIME_CONTEXT
    {
        T m = TTraits::Abs( a[ 0 ] );
        for ( UInt32 i=1; i<N; ++i )
        {
            T c = TTraits::Abs( a[ i ] );
            if ( !( c <= m ) )
                m = c;
        }
        return m;
    }

    /** a divided by its largest magnitude (components in [-1, 1], one of them +-1); false for zero or non-finite a */
    template < UInt32 N >
    static GUCEF_MATH_FORCE_INLINE bool ScaleToUnitChebyshev( const T ( &a )[ N ], T ( &out )[ N ] ) GUCEF_MATH_REALTIME_CONTEXT
    {
        T m = MaxMagnitude( a );
        if ( !( TTraits::Zero() < m ) || !TTraits::IsFinite( m ) )
            return false;
        for ( UInt32 i=0; i<N; ++i )
            out[ i ] = a[ i ] / m;
        return true;
    }

    /** sqrt( sum of squares ) without overflow or underflow of the intermediate; NaN for NaN components */
    template < UInt32 N >
    static GUCEF_MATH_FORCE_INLINE T Length( const T ( &a )[ N ] ) GUCEF_MATH_REALTIME_CONTEXT
    {
        T squaredLength = Dot( a, a );
        if ( IsSafeSquaredLength( squaredLength ) )
            return TTraits::Sqrt( squaredLength );
        return ScaledLength( a, squaredLength );
    }

    /** a divided by its length; false (out unchanged) for zero and non-finite a */
    template < UInt32 N >
    static GUCEF_MATH_FORCE_INLINE bool Normalize( const T ( &a )[ N ], T ( &out )[ N ] ) GUCEF_MATH_REALTIME_CONTEXT
    {
        return NormalizeWithSquaredLength( a, Dot( a, a ), out );
    }

    /**
     *  Normalize with the squared length already formed by the caller (CTVector4Ops forms it in its canonical order).
     *  The fast path divides by its root; otherwise the vector is first divided by its largest magnitude.
     */
    template < UInt32 N >
    static bool NormalizeWithSquaredLength( const T ( &a )[ N ], const T& squaredLength, T ( &out )[ N ] ) GUCEF_MATH_REALTIME_CONTEXT
    {
        if ( IsSafeSquaredLength( squaredLength ) )
        {
            T length = TTraits::Sqrt( squaredLength );
            for ( UInt32 i=0; i<N; ++i )
                out[ i ] = a[ i ] / length;
            return true;
        }
        T scaled[ N ];
        if ( !( squaredLength == squaredLength ) || !ScaleToUnitChebyshev( a, scaled ) )
            return false;
        T length = TTraits::Sqrt( Dot( scaled, scaled ) );
        for ( UInt32 i=0; i<N; ++i )
            out[ i ] = scaled[ i ] / length;
        return true;
    }

    private:

    template < UInt32 N >
    static T ScaledLength( const T ( &a )[ N ], const T& squaredLength ) GUCEF_MATH_REALTIME_CONTEXT
    {
        if ( !( squaredLength == squaredLength ) )
            return squaredLength;
        T m = MaxMagnitude( a );
        if ( !( TTraits::Zero() < m ) || !TTraits::IsFinite( m ) )
            return m;
        T scaled[ N ];
        for ( UInt32 i=0; i<N; ++i )
            scaled[ i ] = a[ i ] / m;
        return m * TTraits::Sqrt( Dot( scaled, scaled ) );
    }
};

/*-------------------------------------------------------------------------*/

template < typename TStorage, UInt32 F >
class CTMathComponentKernels< CTFixedPoint< TStorage, F >, MATHCOMPONENTKIND_FIXED > : public CTMathOperatorComponentKernels< CTFixedPoint< TStorage, F > >
{
    public:

    typedef CTFixedPoint< TStorage, F > T;
    typedef CTNumericTraits< T > TTraits;
    typedef T TDotWide;

    enum { StorageBits = sizeof( TStorage ) * 8 };

    static GUCEF_MATH_FORCE_INLINE T Negate( const T& x ) GUCEF_MATH_REALTIME_CONTEXT { return -x; }

    /** The exact sum of the raw products, rounded once to the format (ties away from zero) and saturated */
    template < UInt32 N >
    static T Dot( const T ( &a )[ N ], const T ( &b )[ N ] ) GUCEF_MATH_REALTIME_CONTEXT
    {
        CORE::CInt128 sum = CORE::CInt128::Multiply( (Int64) a[ 0 ].GetRaw(), (Int64) b[ 0 ].GetRaw() );
        for ( UInt32 i=1; i<N; ++i )
            sum = CORE::CInt128::Add( sum, CORE::CInt128::Multiply( (Int64) a[ i ].GetRaw(), (Int64) b[ i ].GetRaw() ) );
        return FromDoubleScaleRaw( sum );
    }

    template < UInt32 N >
    static GUCEF_MATH_FORCE_INLINE T DotWide( const T ( &a )[ N ], const T ( &b )[ N ] ) GUCEF_MATH_REALTIME_CONTEXT { return Dot( a, b ); }

    /** The correctly rounded length: one 128 bit integer square root of the exact sum of the squared raw values */
    template < UInt32 N >
    static T Length( const T ( &a )[ N ] ) GUCEF_MATH_REALTIME_CONTEXT
    {
        CORE::CUInt128 sum( (UInt64) 0 );
        for ( UInt32 i=0; i<N; ++i )
        {
            UInt64 magnitude = MathMagnitude( (Int64) a[ i ].GetRaw() );
            if ( !CORE::CUInt128::AddChecked( sum, CORE::CUInt128::Multiply( magnitude, magnitude ), sum ) )
                return T::Highest();
        }
        UInt64 root = MathSqrtRounded128( sum );
        return root > MaxPositiveRaw() ? T::Highest() : T::FromRaw( (TStorage) root );
    }

    /** a divided by its largest magnitude (components in [-1, 1], one of them +-1); false for the zero vector */
    template < UInt32 N >
    static bool ScaleToUnitChebyshev( const T ( &a )[ N ], T ( &out )[ N ] ) GUCEF_MATH_REALTIME_CONTEXT
    {
        UInt64 largest = 0;
        for ( UInt32 i=0; i<N; ++i )
        {
            UInt64 magnitude = MathMagnitude( (Int64) a[ i ].GetRaw() );
            if ( magnitude > largest )
                largest = magnitude;
        }
        if ( 0 == largest )
            return false;
        // the magnitude of the lowest value is not representable: divide by the highest instead (a quotient of -1)
        T divisor = T::FromRaw( (TStorage) ( largest > MaxPositiveRaw() ? MaxPositiveRaw() : largest ) );
        for ( UInt32 i=0; i<N; ++i )
            out[ i ] = TTraits::Max( TTraits::Min( a[ i ] / divisor, TTraits::One() ), ComponentNegateOne() );
        return true;
    }

    /** a divided by its length, after scaling by the largest magnitude; false (out unchanged) for the zero vector */
    template < UInt32 N >
    static bool Normalize( const T ( &a )[ N ], T ( &out )[ N ] ) GUCEF_MATH_REALTIME_CONTEXT
    {
        T scaled[ N ];
        if ( !ScaleToUnitChebyshev( a, scaled ) )
            return false;
        T length = Length( scaled );
        for ( UInt32 i=0; i<N; ++i )
            out[ i ] = scaled[ i ] / length;
        return true;
    }

    private:

    static GUCEF_MATH_FORCE_INLINE UInt64 MaxPositiveRaw( void ) GUCEF_MATH_REALTIME_CONTEXT { return ( (UInt64) 1 << ( StorageBits - 1 ) ) - 1; }
    static GUCEF_MATH_FORCE_INLINE T ComponentNegateOne( void ) GUCEF_MATH_REALTIME_CONTEXT { return -TTraits::One(); }

    /** A value in units of 2^-2F rounded to units of 2^-F, ties away from zero, saturated to the format */
    static T FromDoubleScaleRaw( const CORE::CInt128& value ) GUCEF_MATH_REALTIME_CONTEXT
    {
        bool negative = value.IsNegative();
        CORE::CInt128 absolute = negative ? CORE::CInt128::Negate( value ) : value;
        CORE::CUInt128 magnitude( absolute.high, absolute.low );
        CORE::CUInt128 quotient = CORE::CUInt128::ShiftRight( magnitude, F );
        if ( F > 0 && 0 != ( ( magnitude.low >> ( F > 0 ? F - 1 : 0 ) ) & 1 ) )
            quotient = CORE::CUInt128::Add( quotient, CORE::CUInt128( (UInt64) 1 ) );
        UInt64 limit = MaxPositiveRaw() + ( negative ? 1 : 0 );
        if ( 0 != quotient.high || quotient.low > limit )
            return negative ? T::Lowest() : T::Highest();
        return T::FromRaw( (TStorage) MathApplySign( negative, quotient.low ) );
    }
};

/*-------------------------------------------------------------------------*/

template < typename T >
class CTMathComponentKernels< T, MATHCOMPONENTKIND_INTEGER >
{
    public:

    typedef CTNumericTraits< T > TTraits;
    typedef typename CTMathIntegerComponentTypes< T >::TUnsigned TUnsigned;
    typedef typename CTMathIntegerComponentTypes< T >::TWork TWork;
    typedef typename CTMathIntegerComponentTypes< T >::TDotWide TDotWide;
    typedef CTMathWideAccumulator< TDotWide > TAccumulator;

    static GUCEF_MATH_FORCE_INLINE T Add( const T& a, const T& b ) GUCEF_MATH_REALTIME_CONTEXT { return (T) (TUnsigned) ( (TWork) (TUnsigned) a + (TWork) (TUnsigned) b ); }
    static GUCEF_MATH_FORCE_INLINE T Subtract( const T& a, const T& b ) GUCEF_MATH_REALTIME_CONTEXT { return (T) (TUnsigned) ( (TWork) (TUnsigned) a - (TWork) (TUnsigned) b ); }
    static GUCEF_MATH_FORCE_INLINE T Multiply( const T& a, const T& b ) GUCEF_MATH_REALTIME_CONTEXT { return (T) (TUnsigned) ( (TWork) (TUnsigned) a * (TWork) (TUnsigned) b ); }
    static GUCEF_MATH_FORCE_INLINE T Negate( const T& x ) GUCEF_MATH_REALTIME_CONTEXT { return (T) (TUnsigned) ( (TWork) 0 - (TWork) (TUnsigned) x ); }

    /** The dot product in the kind, wrapping as the built-in type wraps */
    template < UInt32 N >
    static GUCEF_MATH_FORCE_INLINE T Dot( const T ( &a )[ N ], const T ( &b )[ N ] ) GUCEF_MATH_REALTIME_CONTEXT
    {
        T sum = Multiply( a[ 0 ], b[ 0 ] );
        for ( UInt32 i=1; i<N; ++i )
            sum = Add( sum, Multiply( a[ i ], b[ i ] ) );
        return sum;
    }

    /** The exact dot product in TDotWide */
    template < UInt32 N >
    static GUCEF_MATH_FORCE_INLINE TDotWide DotWide( const T ( &a )[ N ], const T ( &b )[ N ] ) GUCEF_MATH_REALTIME_CONTEXT
    {
        TDotWide sum = TAccumulator::Product( a[ 0 ], b[ 0 ] );
        for ( UInt32 i=1; i<N; ++i )
            sum = TAccumulator::Add( sum, TAccumulator::Product( a[ i ], b[ i ] ) );
        return sum;
    }
};

/*-------------------------------------------------------------------------*/

template < typename T >
class CTMathComponentKernels< T, MATHCOMPONENTKIND_EXACT > : public CTMathOperatorComponentKernels< T >
{
    public:

    typedef CTNumericTraits< T > TTraits;
    typedef T TDotWide;

    /** Fractions have no unary minus */
    static GUCEF_MATH_FORCE_INLINE T Negate( const T& x ) GUCEF_MATH_REALTIME_CONTEXT { return TTraits::Zero() - x; }

    template < UInt32 N >
    static GUCEF_MATH_FORCE_INLINE T Dot( const T ( &a )[ N ], const T ( &b )[ N ] ) GUCEF_MATH_REALTIME_CONTEXT
    {
        T sum = a[ 0 ] * b[ 0 ];
        for ( UInt32 i=1; i<N; ++i )
            sum = sum + a[ i ] * b[ i ];
        return sum;
    }

    template < UInt32 N >
    static GUCEF_MATH_FORCE_INLINE T DotWide( const T ( &a )[ N ], const T ( &b )[ N ] ) GUCEF_MATH_REALTIME_CONTEXT { return Dot( a, b ); }
};

/*-------------------------------------------------------------------------*/

/** The kernels of the kind T and the type of its exact dot product */
template < typename T >
class CTMathComponentTraits
{
    public:

    typedef CTMathComponentKernels< T, CTMathComponentKind< T >::Value > TKernels;
    typedef typename TKernels::TDotWide TDotWide;
};

/*-------------------------------------------------------------------------*/

/**
 *  DataNode attribute value of one component: float kinds read numbers directly (and text such as "nan"), every other
 *  kind reads the exact text form of its traits, which also accepts an integer variant (its text is the number).
 */
template < typename T, int IsFloatingPoint >
class CTComponentVariantCodec
{
    public:

    static bool FromVariant( const CORE::CVariant& value, T& out )
    {
        if ( !value.IsInitialized() )
            return false;
        return CTNumericTraits< T >::FromString( value.AsString().Trim( true ).Trim( false ), out );
    }
};

template < typename T >
class CTComponentVariantCodec< T, 1 >
{
    public:

    static bool FromVariant( const CORE::CVariant& value, T& out )
    {
        if ( value.IsFloat() || value.IsInteger() )
        {
            T result = T( 0 );
            if ( MathStatusIsError( CTNumericTraits< T >::FromFloat64( value.AsFloat64(), CORE::ROUNDINGMODE_HALF_EVEN, result ) ) )
                return false;
            out = result;
            return true;
        }
        if ( value.IsString() )
            return CTNumericTraits< T >::FromString( value.AsString().Trim( true ).Trim( false ), out );
        return false;
    }
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** -x for every numeric kind: integers wrap (the negation of the lowest value is itself), fractions use Zero() - x */
template < typename T >
GUCEF_MATH_FORCE_INLINE T
ComponentNegate( const T& x ) GUCEF_MATH_REALTIME_CONTEXT
{
    return CTMathComponentTraits< T >::TKernels::Negate( x );
}

/*-------------------------------------------------------------------------*/

/** a + b in the kind; integers wrap through the unsigned image (defined behaviour) */
template < typename T >
GUCEF_MATH_FORCE_INLINE T
ComponentAdd( const T& a, const T& b ) GUCEF_MATH_REALTIME_CONTEXT
{
    return CTMathComponentTraits< T >::TKernels::Add( a, b );
}

/*-------------------------------------------------------------------------*/

/** a - b in the kind; integers wrap */
template < typename T >
GUCEF_MATH_FORCE_INLINE T
ComponentSubtract( const T& a, const T& b ) GUCEF_MATH_REALTIME_CONTEXT
{
    return CTMathComponentTraits< T >::TKernels::Subtract( a, b );
}

/*-------------------------------------------------------------------------*/

/** a * b in the kind; integers wrap */
template < typename T >
GUCEF_MATH_FORCE_INLINE T
ComponentMultiply( const T& a, const T& b ) GUCEF_MATH_REALTIME_CONTEXT
{
    return CTMathComponentTraits< T >::TKernels::Multiply( a, b );
}

/*-------------------------------------------------------------------------*/

/** The attribute value of one component: Float32 / Float64 / integer variants for those kinds */
inline CORE::CVariant ComponentToVariant( Float32 value ) { return CORE::CVariant( value ); }
inline CORE::CVariant ComponentToVariant( Float64 value ) { return CORE::CVariant( value ); }
inline CORE::CVariant ComponentToVariant( Int8 value ) { return CORE::CVariant( value ); }
inline CORE::CVariant ComponentToVariant( UInt8 value ) { return CORE::CVariant( value ); }
inline CORE::CVariant ComponentToVariant( Int16 value ) { return CORE::CVariant( value ); }
inline CORE::CVariant ComponentToVariant( UInt16 value ) { return CORE::CVariant( value ); }
inline CORE::CVariant ComponentToVariant( Int32 value ) { return CORE::CVariant( value ); }
inline CORE::CVariant ComponentToVariant( UInt32 value ) { return CORE::CVariant( value ); }
inline CORE::CVariant ComponentToVariant( Int64 value ) { return CORE::CVariant( value ); }
inline CORE::CVariant ComponentToVariant( UInt64 value ) { return CORE::CVariant( value ); }

/** Every other kind (fixed point, fractions) as its exact text, so no conversion loses information */
template < typename T >
inline CORE::CVariant
ComponentToVariant( const T& value )
{
    return CORE::CVariant( CTNumericTraits< T >::ToString( value ) );
}

/*-------------------------------------------------------------------------*/

/** Reads one component from an attribute value; false (and 'out' unchanged) when it does not hold a value of the kind */
template < typename T >
inline bool
ComponentFromVariant( const CORE::CVariant& value, T& out )
{
    return CTComponentVariantCodec< T, CTNumericTraits< T >::IsFloatingPoint >::FromVariant( value, out );
}

/*-------------------------------------------------------------------------*/

/** "(c0, c1, ...)" with the locale independent, exactly round-tripping text of each component */
template < typename T, UInt32 Count >
CORE::CString
ComponentsToString( const T ( &components )[ Count ] )
{
    CORE::CString text( "(" );
    for ( UInt32 i=0; i<Count; ++i )
    {
        if ( i > 0 )
            text += ", ";
        text += CTNumericTraits< T >::ToString( components[ i ] );
    }
    text += ')';
    return text;
}

/*-------------------------------------------------------------------------*/

/** True for the ASCII white space characters accepted around and inside the text forms */
GUCEF_MATH_FORCE_INLINE bool
IsComponentTextWhitespace( char c )
{
    return ' ' == c || '\t' == c || '\r' == c || '\n' == c || '\v' == c || '\f' == c;
}

/*-------------------------------------------------------------------------*/

/**
 *  Parses "(c0, c1, ...)" with exactly Count components; ASCII white space is accepted around the parentheses and
 *  around each component. Returns false for any malformed text and leaves 'components' unchanged.
 */
template < typename T, UInt32 Count >
bool
ComponentsFromString( const CORE::CString& text, T ( &components )[ Count ] )
{
    const char* data = text.C_String();
    UInt32 length = text.Length();
    if ( GUCEF_NULL == data )
        return false;

    UInt32 position = 0;
    while ( position < length && IsComponentTextWhitespace( data[ position ] ) )
        ++position;
    if ( position >= length || '(' != data[ position ] )
        return false;
    ++position;

    T parsed[ Count ];
    for ( UInt32 i=0; i<Count; ++i )
    {
        const char terminator = ( i + 1 < Count ) ? ',' : ')';
        UInt32 end = position;
        while ( end < length && terminator != data[ end ] && ',' != data[ end ] && ')' != data[ end ] )
            ++end;
        if ( end >= length || terminator != data[ end ] )
            return false;

        UInt32 first = position;
        UInt32 last = end;
        while ( first < last && IsComponentTextWhitespace( data[ first ] ) )
            ++first;
        while ( last > first && IsComponentTextWhitespace( data[ last - 1 ] ) )
            --last;
        if ( first == last )
            return false;
        if ( !CTNumericTraits< T >::FromString( CORE::CString( data + first, last - first ), parsed[ i ] ) )
            return false;
        position = end + 1;
    }

    while ( position < length && IsComponentTextWhitespace( data[ position ] ) )
        ++position;
    if ( position != length )
        return false;

    for ( UInt32 i=0; i<Count; ++i )
        components[ i ] = parsed[ i ];
    return true;
}

/*-------------------------------------------------------------------------*/

/** Writes one attribute per component into 'node'; naming the node is the caller's choice */
template < typename T, UInt32 Count >
bool
SerializeComponents( CORE::CDataNode& node, const char* const ( &names )[ Count ], const T ( &components )[ Count ] )
{
    for ( UInt32 i=0; i<Count; ++i )
    {
        const CORE::CVariant value = ComponentToVariant( components[ i ] );
        if ( !node.SetAttribute( CORE::CString( names[ i ] ), value, false ) )
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

/** Reads one attribute per component; a missing or invalid attribute is a failure and leaves 'components' unchanged */
template < typename T, UInt32 Count >
bool
DeserializeComponents( const CORE::CDataNode& node, const char* const ( &names )[ Count ], T ( &components )[ Count ] )
{
    T parsed[ Count ];
    for ( UInt32 i=0; i<Count; ++i )
    {
        const CORE::CDataNode::TKeyValuePair* attribute = node.GetAttribute( names[ i ] );
        if ( GUCEF_NULL == attribute || !ComponentFromVariant( attribute->second, parsed[ i ] ) )
            return false;
    }
    for ( UInt32 i=0; i<Count; ++i )
        components[ i ] = parsed[ i ];
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace MATH */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MATH_COMPONENTFUNCTIONS_H ? */
