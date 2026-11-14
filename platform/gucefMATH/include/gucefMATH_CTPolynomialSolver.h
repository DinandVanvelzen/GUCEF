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

#ifndef GUCEF_MATH_CTPOLYNOMIALSOLVER_H
#define GUCEF_MATH_CTPOLYNOMIALSOLVER_H

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
 *  Real roots of polynomials up to the fourth degree, for a floating point kind T (Float32 or Float64).
 *
 *  Every function takes the coefficients from the highest power down ( a x^2 + b x + c ), writes the distinct real
 *  roots in ascending order to 'roots' (which has room for the degree's number of roots: 1, 2, 3 or 4) and returns
 *  how many there are. Entries of 'roots' beyond the returned count are not touched.
 *
 *  Degenerate input: a leading coefficient whose magnitude is at most 'epsilon' times the largest of the
 *  other coefficients (so exactly 0, and values that only rounding noise separates from 0) is dropped and the
 *  next lower degree is solved. A polynomial with all coefficients 0 has no roots reported.
 *  Roots that differ by at most 'tolerance' relative to their size are one root (a double root reported as a pair
 *  because of rounding is returned once, at the mean). Complex roots are not reported.
 *
 *  Methods: the quadratic uses the cancellation free form q = -1/2 ( b + sign( b ) sqrt( D ) ) with the roots
 *  q / a and c / q. The cubic is reduced to the depressed form: three real roots through the trigonometric method,
 *  one through Cardano's formula, a zero discriminant (within a few ULP) through the closed form of the double or
 *  triple root. The quartic uses Ferrari's method through the resolvent cubic (a biquadratic directly). Cubic and
 *  quartic roots get Newton steps on the original polynomial, which are kept only where they reduce the residual.
 *
 *  Static functions without state, no allocation, no locks: realtime safe and thread safe. T must be a float kind.
 */
template < typename T >
class CTPolynomialSolver
{
    public:

    typedef T TValue;

    /** The tolerance used by default to merge roots: 8 times the square root of the machine epsilon (about 1.2e-7 for Float64) */
    static T GetDefaultTolerance( void )
    {
        return T( 8 ) * std::sqrt( std::numeric_limits< T >::epsilon() );
    }

    /** a x + b = 0. No root when a is negligible. roots[ 1 ] */
    static UInt32 SolveLinear( const T& a, const T& b, T* roots )
    {
        if ( IsNegligibleLeading( a, b ) )
            return 0;
        roots[ 0 ] = -b / a;
        return 1;
    }

    /** a x^2 + b x + c = 0. roots[ 2 ] */
    static UInt32 SolveQuadratic( const T& a, const T& b, const T& c, T* roots, const T& tolerance = GetDefaultTolerance() )
    {
        if ( IsNegligibleLeading( a, Max2Abs( b, c ) ) )
            return SolveLinear( b, c, roots );

        T discriminant = b * b - T( 4 ) * a * c;
        const T scale = b * b + std::fabs( T( 4 ) * a * c );
        if ( discriminant < T( 0 ) )
        {
            // a double root that rounding pushed to the other side of zero
            if ( -discriminant > T( 16 ) * std::numeric_limits< T >::epsilon() * scale )
                return 0;
            discriminant = T( 0 );
        }
        if ( T( 0 ) == discriminant )
        {
            roots[ 0 ] = -b / ( T( 2 ) * a );
            return 1;
        }

        const T root = std::sqrt( discriminant );
        const T q = T( -0.5 ) * ( b < T( 0 ) ? b - root : b + root );
        // q is not 0 here: b and root are not both 0
        T r0 = q / a;
        T r1 = c / q;
        if ( r1 < r0 )
        {
            const T swap = r0;
            r0 = r1;
            r1 = swap;
        }
        roots[ 0 ] = r0;
        roots[ 1 ] = r1;
        return MergeClose( roots, 2, tolerance );
    }

    /** a x^3 + b x^2 + c x + d = 0. roots[ 3 ] */
    static UInt32 SolveCubic( const T& a, const T& b, const T& c, const T& d, T* roots, const T& tolerance = GetDefaultTolerance() )
    {
        if ( IsNegligibleLeading( a, Max3Abs( b, c, d ) ) )
            return SolveQuadratic( b, c, d, roots, tolerance );

        if ( T( 0 ) == d )
        {
            // x ( a x^2 + b x + c ) = 0
            T found[ 3 ];
            const UInt32 count = SolveQuadratic( a, b, c, found, tolerance );
            return InsertZero( found, count, roots, tolerance );
        }

        const T eps = std::numeric_limits< T >::epsilon();
        const T shift = b / ( T( 3 ) * a );
        // depressed form t^3 + p t + q = 0 with x = t - shift
        const T bb = b / a;
        const T cc = c / a;
        const T dd = d / a;
        const T p = cc - bb * bb / T( 3 );
        const T q = T( 2 ) * bb * bb * bb / T( 27 ) - bb * cc / T( 3 ) + dd;

        const T fourP3 = T( 4 ) * p * p * p;
        const T twentySevenQ2 = T( 27 ) * q * q;
        const T discriminantTerm = fourP3 + twentySevenQ2;                  // positive: one real root, negative: three
        const T discriminantScale = std::fabs( fourP3 ) + twentySevenQ2;

        T t[ 3 ];
        UInt32 count = 0;
        if ( T( 0 ) == discriminantScale || std::fabs( discriminantTerm ) <= T( 64 ) * eps * discriminantScale )
        {
            if ( T( 0 ) == p )
            {
                t[ 0 ] = T( 0 );                                            // triple root
                count = 1;
            }
            else
            {
                const T single = T( 3 ) * q / p;                            // simple root
                const T twice = T( -1.5 ) * q / p;                          // double root
                t[ 0 ] = single < twice ? single : twice;
                t[ 1 ] = single < twice ? twice : single;
                count = 2;
            }
        }
        else if ( discriminantTerm > T( 0 ) )
        {
            // Cardano: t = u + v with u v = -p / 3, the sign chosen so that nothing cancels
            const T s = std::sqrt( q * q / T( 4 ) + p * p * p / T( 27 ) );
            const T w = T( -0.5 ) * q - ( q < T( 0 ) ? -s : s );
            const T u = CubeRoot( w );
            t[ 0 ] = T( 0 ) == u ? T( 0 ) : u - p / ( T( 3 ) * u );
            count = 1;
        }
        else
        {
            // three real roots: p is negative
            const T m = T( 2 ) * std::sqrt( -p / T( 3 ) );
            T argument = T( 3 ) * q / ( T( 2 ) * p ) * std::sqrt( T( -3 ) / p );
            if ( argument > T( 1 ) ) argument = T( 1 );
            if ( argument < T( -1 ) ) argument = T( -1 );
            const T angle = std::acos( argument ) / T( 3 );
            const T twoPiThirds = T( 2.0943951023931954923084289221863353 );
            t[ 0 ] = m * std::cos( angle );
            t[ 1 ] = m * std::cos( angle - twoPiThirds );
            t[ 2 ] = m * std::cos( angle - T( 2 ) * twoPiThirds );
            count = 3;
            SortAscending( t, count );
        }

        for ( UInt32 i=0; i<count; ++i )
        {
            roots[ i ] = t[ i ] - shift;
            T coefficients[ 4 ] = { d, c, b, a };
            roots[ i ] = Polish( coefficients, 3, roots[ i ] );
        }
        SortAscending( roots, count );
        return MergeClose( roots, count, tolerance );
    }

    /** a x^4 + b x^3 + c x^2 + d x + e = 0. roots[ 4 ] */
    static UInt32 SolveQuartic( const T& a, const T& b, const T& c, const T& d, const T& e, T* roots, const T& tolerance = GetDefaultTolerance() )
    {
        if ( IsNegligibleLeading( a, Max4Abs( b, c, d, e ) ) )
            return SolveCubic( b, c, d, e, roots, tolerance );

        if ( T( 0 ) == e )
        {
            // x ( a x^3 + b x^2 + c x + d ) = 0
            T found[ 3 ];
            const UInt32 count = SolveCubic( a, b, c, d, found, tolerance );
            return InsertZero( found, count, roots, tolerance );
        }

        const T eps = std::numeric_limits< T >::epsilon();
        const T bb = b / a;
        const T cc = c / a;
        const T dd = d / a;
        const T ee = e / a;
        const T shift = bb / T( 4 );
        // depressed form y^4 + p y^2 + q y + r = 0 with x = y - shift
        const T p = cc - T( 3 ) * bb * bb / T( 8 );
        const T q = dd - bb * cc / T( 2 ) + bb * bb * bb / T( 8 );
        const T r = ee - bb * dd / T( 4 ) + bb * bb * cc / T( 16 ) - T( 3 ) * bb * bb * bb * bb / T( 256 );

        T y[ 4 ];
        UInt32 count = 0;

        const T pScale = std::pow( std::fabs( p ), T( 1.5 ) );
        const T rScale = std::pow( std::fabs( r ), T( 0.75 ) );
        const bool biquadratic = std::fabs( q ) <= T( 16 ) * eps * ( pScale > rScale ? pScale : rScale );
        T resolvent = T( 0 );
        if ( !biquadratic )
        {
            // z^3 + 2 p z^2 + ( p^2 - 4 r ) z - q^2 = 0: the largest real root is positive, as the cubic is -q^2 at 0
            T zRoots[ 3 ];
            const UInt32 zCount = SolveCubic( T( 1 ), T( 2 ) * p, p * p - T( 4 ) * r, -q * q, zRoots, T( 0 ) );
            resolvent = zCount > 0 ? zRoots[ zCount - 1 ] : T( 0 );
        }

        if ( biquadratic || !( resolvent > T( 0 ) ) )
        {
            // y^4 + p y^2 + r = 0 through u = y^2
            T uRoots[ 2 ];
            const UInt32 uCount = SolveQuadratic( T( 1 ), p, r, uRoots, tolerance );
            for ( UInt32 i=0; i<uCount; ++i )
            {
                if ( uRoots[ i ] > T( 0 ) )
                {
                    const T s = std::sqrt( uRoots[ i ] );
                    y[ count++ ] = -s;
                    y[ count++ ] = s;
                }
                else if ( T( 0 ) == uRoots[ i ] )
                {
                    y[ count++ ] = T( 0 );
                }
            }
        }
        else
        {
            // ( y^2 + p/2 + m )^2 = 2 m ( y - q / ( 4 m ) )^2 with m = resolvent / 2, so y^2 -+ s y + p/2 + m -+ q / ( 2 s ) = 0
            const T m = T( 0.5 ) * resolvent;
            const T s = std::sqrt( T( 2 ) * m );
            const T half = T( 0.5 ) * p + m;
            const T tail = q / ( T( 2 ) * s );
            T pair[ 2 ];
            UInt32 pairCount = SolveQuadratic( T( 1 ), -s, half + tail, pair, tolerance );
            for ( UInt32 i=0; i<pairCount; ++i )
                y[ count++ ] = pair[ i ];
            pairCount = SolveQuadratic( T( 1 ), s, half - tail, pair, tolerance );
            for ( UInt32 i=0; i<pairCount; ++i )
                y[ count++ ] = pair[ i ];
        }

        for ( UInt32 i=0; i<count; ++i )
        {
            roots[ i ] = y[ i ] - shift;
            T coefficients[ 5 ] = { e, d, c, b, a };
            roots[ i ] = Polish( coefficients, 4, roots[ i ] );
        }
        SortAscending( roots, count );
        return MergeClose( roots, count, tolerance );
    }

    private:

    static T Max2Abs( const T& a, const T& b )
    {
        const T x = std::fabs( a );
        const T y = std::fabs( b );
        return x > y ? x : y;
    }

    static T Max3Abs( const T& a, const T& b, const T& c )
    {
        return Max2Abs( Max2Abs( a, b ), c );
    }

    static T Max4Abs( const T& a, const T& b, const T& c, const T& d )
    {
        return Max2Abs( Max2Abs( a, b ), Max2Abs( c, d ) );
    }

    /** The leading coefficient is dropped when it is 0 or only noise next to the largest other coefficient */
    static bool IsNegligibleLeading( const T& leading, const T& largestOther )
    {
        return std::fabs( leading ) <= std::numeric_limits< T >::epsilon() * std::fabs( largestOther ) || T( 0 ) == leading;
    }

    static T CubeRoot( const T& x )
    {
        return x < T( 0 ) ? -std::pow( -x, T( 1 ) / T( 3 ) ) : std::pow( x, T( 1 ) / T( 3 ) );
    }

    static void SortAscending( T* values, UInt32 count )
    {
        for ( UInt32 i=1; i<count; ++i )
        {
            const T value = values[ i ];
            UInt32 j = i;
            while ( j > 0 && values[ j - 1 ] > value )
            {
                values[ j ] = values[ j - 1 ];
                --j;
            }
            values[ j ] = value;
        }
    }

    /** Merges neighbours that are within the relative tolerance of each other, at their mean; 'values' is sorted */
    static UInt32 MergeClose( T* values, UInt32 count, const T& tolerance )
    {
        if ( count < 2 )
            return count;
        UInt32 out = 1;
        UInt32 group = 1;
        for ( UInt32 i=1; i<count; ++i )
        {
            const T previous = values[ out - 1 ] / (T) group;           // the group is kept as a sum
            const T scale = std::fabs( previous ) > std::fabs( values[ i ] ) ? std::fabs( previous ) : std::fabs( values[ i ] );
            if ( std::fabs( values[ i ] - previous ) <= tolerance * scale )
            {
                values[ out - 1 ] += values[ i ];
                ++group;
            }
            else
            {
                values[ out - 1 ] = previous;
                values[ out ] = values[ i ];
                ++out;
                group = 1;
            }
        }
        values[ out - 1 ] = values[ out - 1 ] / (T) group;
        return out;
    }

    /** The sorted roots 'found' with the root 0 added (merged with a root that is 0 already) */
    static UInt32 InsertZero( const T* found, UInt32 count, T* roots, const T& tolerance )
    {
        UInt32 out = 0;
        bool inserted = false;
        for ( UInt32 i=0; i<count; ++i )
        {
            if ( !inserted && found[ i ] >= T( 0 ) )
            {
                roots[ out++ ] = T( 0 );
                inserted = true;
                if ( T( 0 ) == found[ i ] )
                    continue;
            }
            roots[ out++ ] = found[ i ];
        }
        if ( !inserted )
            roots[ out++ ] = T( 0 );
        return MergeClose( roots, out, tolerance );
    }

    /** Newton steps on the polynomial with the coefficients from the lowest power up; a step is kept only when the residual falls */
    static T Polish( const T* coefficients, UInt32 degree, T x )
    {
        T residual = Evaluate( coefficients, degree, x );
        for ( UInt32 step=0; step<4 && T( 0 ) != residual; ++step )
        {
            T derivative = T( 0 );
            for ( UInt32 k=degree; k>0; --k )
                derivative = derivative * x + (T) k * coefficients[ k ];
            if ( T( 0 ) == derivative )
                break;
            const T next = x - residual / derivative;
            const T nextResidual = Evaluate( coefficients, degree, next );
            if ( !( std::fabs( nextResidual ) < std::fabs( residual ) ) )
                break;
            x = next;
            residual = nextResidual;
        }
        return x;
    }

    static T Evaluate( const T* coefficients, UInt32 degree, const T& x )
    {
        T value = coefficients[ degree ];
        for ( UInt32 k=degree; k>0; --k )
            value = value * x + coefficients[ k - 1 ];
        return value;
    }
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

typedef CTPolynomialSolver< Float32 > TFloat32PolynomialSolver;
typedef CTPolynomialSolver< Float64 > TFloat64PolynomialSolver;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace MATH */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MATH_CTPOLYNOMIALSOLVER_H ? */
