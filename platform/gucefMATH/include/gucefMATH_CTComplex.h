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

#ifndef GUCEF_MATH_CTCOMPLEX_H
#define GUCEF_MATH_CTCOMPLEX_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <cmath>
#include <limits>

#ifndef GUCEF_MATH_MACROS_H
#include "gucefMATH_macros.h"
#define GUCEF_MATH_MACROS_H
#endif /* GUCEF_MATH_MACROS_H ? */

#ifndef GUCEF_MATH_CTRADIAN_H
#include "gucefMATH_CTRadian.h"
#define GUCEF_MATH_CTRADIAN_H
#endif /* GUCEF_MATH_CTRADIAN_H ? */

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
 *  A complex number re + im * i of a floating point kind T (Float32 or Float64).
 *
 *  The layout is two consecutive values of T, real part first: sizeof( CTComplex< T > ) == 2 * sizeof( T ), so an
 *  array of CTComplex is interchangeable with the interleaved spectrum layout of CRealFft and CComplexFft.
 *  The arguments of angles are strong angles: FromPolar takes and GetArgument returns a CTRadian.
 *
 *  No allocation, no locks, no virtual functions: all operations are realtime safe. Division uses Smith's
 *  algorithm so that it does not overflow where the quotient is representable. GetMagnitude is the plain
 *  square root of the sum of squares: it overflows where the squared magnitude does.
 */
template < typename T >
class CTComplex
{
    public:

    typedef T TValue;

    /** The real part and the imaginary part: public as the layout is part of the contract */
    T re;
    T im;

    GUCEF_MATH_FORCE_INLINE CTComplex( void )
        : re( T( 0 ) )
        , im( T( 0 ) )
    {
    }

    /** A real number as a complex number */
    GUCEF_MATH_FORCE_INLINE CTComplex( const T& real )
        : re( real )
        , im( T( 0 ) )
    {
    }

    GUCEF_MATH_FORCE_INLINE CTComplex( const T& real, const T& imaginary )
        : re( real )
        , im( imaginary )
    {
    }

    /** magnitude * ( cos( argument ) + i sin( argument ) ) */
    static GUCEF_MATH_FORCE_INLINE CTComplex FromPolar( const T& magnitude, const CTRadian< T >& argument )
    {
        return CTComplex( magnitude * std::cos( argument.GetValue() ), magnitude * std::sin( argument.GetValue() ) );
    }

    static GUCEF_MATH_FORCE_INLINE CTComplex ImaginaryUnit( void ) { return CTComplex( T( 0 ), T( 1 ) ); }

    GUCEF_MATH_FORCE_INLINE CTComplex Conjugate( void ) const { return CTComplex( re, -im ); }

    GUCEF_MATH_FORCE_INLINE T GetSquaredMagnitude( void ) const { return re * re + im * im; }

    GUCEF_MATH_FORCE_INLINE T GetMagnitude( void ) const { return std::sqrt( re * re + im * im ); }

    /** The principal argument in ( -pi, pi ]; 0 for the number 0 */
    GUCEF_MATH_FORCE_INLINE CTRadian< T > GetArgument( void ) const { return CTRadian< T >( std::atan2( im, re ) ); }

    /** exp( re ) * ( cos( im ) + i sin( im ) ) */
    GUCEF_MATH_FORCE_INLINE CTComplex Exp( void ) const
    {
        const T scale = std::exp( re );
        return CTComplex( scale * std::cos( im ), scale * std::sin( im ) );
    }

    /** The principal logarithm ln( |z| ) + i arg( z ); the logarithm of 0 is -infinity */
    GUCEF_MATH_FORCE_INLINE CTComplex Log( void ) const
    {
        return CTComplex( std::log( std::sqrt( re * re + im * im ) ), std::atan2( im, re ) );
    }

    /** The principal square root: real part not negative */
    CTComplex Sqrt( void ) const
    {
        if ( T( 0 ) == re && T( 0 ) == im )
            return CTComplex( T( 0 ), im );
        const T magnitude = std::sqrt( re * re + im * im );
        // the root of the larger part first, the other from the product relation, so that no digits cancel
        if ( re >= T( 0 ) )
        {
            const T t = std::sqrt( T( 0.5 ) * ( magnitude + re ) );
            return CTComplex( t, im / ( T( 2 ) * t ) );
        }
        const T t = std::sqrt( T( 0.5 ) * ( magnitude - re ) );
        return CTComplex( std::fabs( im ) / ( T( 2 ) * t ), im < T( 0 ) ? -t : t );
    }

    /** exp( exponent * Log( z ) ); z^0 is 1, 0^w is 0 for a positive real part of w and not a number otherwise */
    CTComplex Pow( const CTComplex& exponent ) const
    {
        if ( T( 0 ) == exponent.re && T( 0 ) == exponent.im )
            return CTComplex( T( 1 ), T( 0 ) );
        if ( T( 0 ) == re && T( 0 ) == im )
            return exponent.re > T( 0 ) ? CTComplex( T( 0 ), T( 0 ) ) : CTComplex( std::numeric_limits< T >::quiet_NaN(), std::numeric_limits< T >::quiet_NaN() );
        return ( exponent * Log() ).Exp();
    }

    /** z^exponent for a real exponent, through the polar form */
    CTComplex Pow( const T& exponent ) const
    {
        return Pow( CTComplex( exponent, T( 0 ) ) );
    }

    GUCEF_MATH_FORCE_INLINE CTComplex operator-( void ) const { return CTComplex( -re, -im ); }
    GUCEF_MATH_FORCE_INLINE CTComplex operator+( void ) const { return *this; }

    friend GUCEF_MATH_FORCE_INLINE CTComplex operator+( const CTComplex& a, const CTComplex& b ) { return CTComplex( a.re + b.re, a.im + b.im ); }
    friend GUCEF_MATH_FORCE_INLINE CTComplex operator-( const CTComplex& a, const CTComplex& b ) { return CTComplex( a.re - b.re, a.im - b.im ); }
    friend GUCEF_MATH_FORCE_INLINE CTComplex operator*( const CTComplex& a, const CTComplex& b ) { return CTComplex( a.re * b.re - a.im * b.im, a.re * b.im + a.im * b.re ); }
    friend GUCEF_MATH_FORCE_INLINE CTComplex operator*( const CTComplex& a, const T& factor ) { return CTComplex( a.re * factor, a.im * factor ); }
    friend GUCEF_MATH_FORCE_INLINE CTComplex operator*( const T& factor, const CTComplex& a ) { return CTComplex( factor * a.re, factor * a.im ); }
    friend GUCEF_MATH_FORCE_INLINE CTComplex operator/( const CTComplex& a, const T& divisor ) { return CTComplex( a.re / divisor, a.im / divisor ); }

    /** Smith's algorithm */
    friend CTComplex operator/( const CTComplex& a, const CTComplex& b )
    {
        if ( std::fabs( b.re ) >= std::fabs( b.im ) )
        {
            const T ratio = b.im / b.re;
            const T denominator = b.re + b.im * ratio;
            return CTComplex( ( a.re + a.im * ratio ) / denominator, ( a.im - a.re * ratio ) / denominator );
        }
        const T ratio = b.re / b.im;
        const T denominator = b.re * ratio + b.im;
        return CTComplex( ( a.re * ratio + a.im ) / denominator, ( a.im * ratio - a.re ) / denominator );
    }

    GUCEF_MATH_FORCE_INLINE CTComplex& operator+=( const CTComplex& other ) { re += other.re; im += other.im; return *this; }
    GUCEF_MATH_FORCE_INLINE CTComplex& operator-=( const CTComplex& other ) { re -= other.re; im -= other.im; return *this; }
    GUCEF_MATH_FORCE_INLINE CTComplex& operator*=( const CTComplex& other ) { *this = *this * other; return *this; }
    GUCEF_MATH_FORCE_INLINE CTComplex& operator*=( const T& factor ) { re *= factor; im *= factor; return *this; }
    GUCEF_MATH_FORCE_INLINE CTComplex& operator/=( const CTComplex& other ) { *this = *this / other; return *this; }
    GUCEF_MATH_FORCE_INLINE CTComplex& operator/=( const T& divisor ) { re /= divisor; im /= divisor; return *this; }

    friend GUCEF_MATH_FORCE_INLINE bool operator==( const CTComplex& a, const CTComplex& b ) { return a.re == b.re && a.im == b.im; }
    friend GUCEF_MATH_FORCE_INLINE bool operator!=( const CTComplex& a, const CTComplex& b ) { return !( a == b ); }
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

typedef CTComplex< Float32 > TFloat32Complex;
typedef CTComplex< Float64 > TFloat64Complex;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace MATH */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MATH_CTCOMPLEX_H ? */
