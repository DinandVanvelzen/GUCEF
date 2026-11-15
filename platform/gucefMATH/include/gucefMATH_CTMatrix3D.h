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

#ifndef GUCEF_MATH_CTMATRIX3D_H
#define GUCEF_MATH_CTMATRIX3D_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MATH_CTMATRIX2D_H
#include "gucefMATH_CTMatrix2D.h"
#define GUCEF_MATH_CTMATRIX2D_H
#endif /* GUCEF_MATH_CTMATRIX2D_H ? */

#ifndef GUCEF_MATH_CTVECTOR3D_H
#include "gucefMATH_CTVector3D.h"
#define GUCEF_MATH_CTVECTOR3D_H
#endif /* GUCEF_MATH_CTVECTOR3D_H ? */

#ifndef GUCEF_MATH_CTPOINT2D_H
#include "gucefMATH_CTPoint2D.h"
#define GUCEF_MATH_CTPOINT2D_H
#endif /* GUCEF_MATH_CTPOINT2D_H ? */

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

template < typename T > class CTQuaternion;
template < typename T > class CTDirectionVector3D;

/*-------------------------------------------------------------------------*/

/**
 *  The inverse of 3x3 and 4x4 matrices of the fixed-point kinds with one rounding per element, used by
 *  CTMatrix3D::TryInverse and CTMatrix4D::TryInverse. The other kinds report IsAvailable == 0 and are not used.
 *  - The raw values are equilibrated by powers of two (rows, then columns) into integers A with the largest magnitude of
 *    every row and column in [2^29, 2^30] (30 significant bits: the 4x4 determinant must fit 128 bits); shifting left
 *    is exact, values beyond 2^30 raw units are rounded (Q16.16 and Q48.16 from 16384 up, Q32.32 from 0.25 up).
 *  - The 2x2 minors are exact in 64 bits, the cofactors and the determinant exact in 128 bits (|det A| < 2^125).
 *  - Every element of the inverse is adj( A )_ij * 2^( 2F + q_i + p_j ) / det( A ), rounded once to the nearest raw
 *    value (ties away from zero) and saturated: within half a unit in the last place of the exact inverse of the
 *    equilibrated input, which equals the input only when no value was rounded by the equilibration.
 *  - Singular: a zero row or column, det( A ) == 0, or |det( E )| <= tolerance for E = A / 2^30 (the relative test of
 *    the floating-point kinds; a negative tolerance acts as 0).
 *  Realtime-safe: no allocation, bounded loops.
 */
template < typename T, int Kind >
class CTMathExactMatrixInverse
{
    public:

    enum { IsAvailable = 0 };

    static GUCEF_MATH_FORCE_INLINE bool TryInverse( const T* /*in*/, UInt32 /*n*/, const T& /*tolerance*/, T* /*out*/ ) GUCEF_MATH_REALTIME_CONTEXT { return false; }
};

template < typename TStorage, UInt32 F >
class CTMathExactMatrixInverse< CTFixedPoint< TStorage, F >, MATHCOMPONENTKIND_FIXED >
{
    public:

    typedef CTFixedPoint< TStorage, F > T;

    enum { IsAvailable = 1 };
    enum { MantissaBits = 30 };
    enum { StorageBits = sizeof( TStorage ) * 8 };

    /** in and out column-major n x n, n 3 or 4; false (out unchanged) when singular */
    static bool TryInverse( const T* in, UInt32 n, const T& tolerance, T* out ) GUCEF_MATH_REALTIME_CONTEXT
    {
        GUCEF_MATH_ASSERT( 3 == n || 4 == n );
        if ( 3 != n && 4 != n )
            return false;
        Int64 a[ 16 ];
        Int32 rowShift[ 4 ];
        Int32 columnShift[ 4 ];
        for ( UInt32 i=0; i<n*n; ++i )
            a[ i ] = (Int64) in[ i ].GetRaw();
        for ( UInt32 r=0; r<n; ++r )
        {
            UInt64 largest = 0;
            for ( UInt32 c=0; c<n; ++c )
                largest = MathMax64( largest, MathMagnitude( a[ c * n + r ] ) );
            if ( 0 == largest )
                return false;
            rowShift[ r ] = NormalizingShift( largest );
            for ( UInt32 c=0; c<n; ++c )
                a[ c * n + r ] = ShiftRounded( a[ c * n + r ], rowShift[ r ] );
        }
        for ( UInt32 c=0; c<n; ++c )
        {
            UInt64 largest = 0;
            for ( UInt32 r=0; r<n; ++r )
                largest = MathMax64( largest, MathMagnitude( a[ c * n + r ] ) );
            if ( 0 == largest )
                return false;
            columnShift[ c ] = NormalizingShift( largest );
            for ( UInt32 r=0; r<n; ++r )
                a[ c * n + r ] = ShiftRounded( a[ c * n + r ], columnShift[ c ] );
        }

        CORE::CInt128 adjugate[ 16 ];    // adjugate[ row * n + column ]
        CORE::CInt128 determinant;
        if ( 3 == n )
            Adjugate3( a, adjugate, determinant );
        else
            Adjugate4( a, adjugate, determinant );
        if ( IsSingular( determinant, n, tolerance ) )
            return false;

        for ( UInt32 r=0; r<n; ++r )
            for ( UInt32 c=0; c<n; ++c )
                out[ c * n + r ] = T::FromRaw( ScaledQuotient( adjugate[ r * n + c ], 2 * (Int32) F + columnShift[ r ] + rowShift[ c ], determinant ) );
        return true;
    }

    private:

    static GUCEF_MATH_FORCE_INLINE UInt64 MathMax64( UInt64 a, UInt64 b ) GUCEF_MATH_REALTIME_CONTEXT { return a < b ? b : a; }

    static GUCEF_MATH_FORCE_INLINE UInt64 MaxPositiveRaw( void ) GUCEF_MATH_REALTIME_CONTEXT { return ( (UInt64) 1 << ( StorageBits - 1 ) ) - 1; }

    /** The number of significant bits of a 128 bit magnitude */
    static UInt32 BitLength( const CORE::CUInt128& value ) GUCEF_MATH_REALTIME_CONTEXT
    {
        if ( 0 != value.high )
            return 128 - CORE::CUInt128::CountLeadingZeros( value.high );
        if ( 0 != value.low )
            return 64 - CORE::CUInt128::CountLeadingZeros( value.low );
        return 0;
    }

    /** k with magnitude * 2^k in [ 2^29, 2^30 ] (magnitude > 0) */
    static Int32 NormalizingShift( UInt64 magnitude ) GUCEF_MATH_REALTIME_CONTEXT
    {
        if ( ( (UInt64) 1 << MantissaBits ) == magnitude )
            return 0;
        const Int32 bits = 64 - (Int32) CORE::CUInt128::CountLeadingZeros( magnitude );
        return (Int32) MantissaBits - bits;
    }

    /** value * 2^shift; shifting right rounds half away from zero (the caller bounds the result by 2^30) */
    static Int64 ShiftRounded( Int64 value, Int32 shift ) GUCEF_MATH_REALTIME_CONTEXT
    {
        const bool negative = value < 0;
        UInt64 magnitude = MathMagnitude( value );
        if ( shift >= 0 )
        {
            magnitude = 0 == magnitude ? 0 : ( shift >= 64 ? 0 : magnitude << shift );
        }
        else
        {
            const Int32 right = -shift;
            if ( right > 64 )
                magnitude = 0;
            else
            {
                const UInt64 roundBit = ( magnitude >> ( right - 1 ) ) & 1;
                magnitude = ( 64 == right ? 0 : magnitude >> right ) + roundBit;
            }
        }
        return MathApplySign( negative, magnitude );
    }

    /** a0 b0 - a1 b1, exact (|a|, |b| <= 2^30) */
    static GUCEF_MATH_FORCE_INLINE Int64 Minor2( Int64 a0, Int64 b0, Int64 a1, Int64 b1 ) GUCEF_MATH_REALTIME_CONTEXT
    {
        return a0 * b0 - a1 * b1;
    }

    /** a0 b0 + a1 b1 + a2 b2 in 128 bits */
    static GUCEF_MATH_FORCE_INLINE CORE::CInt128 Dot3( Int64 a0, Int64 b0, Int64 a1, Int64 b1, Int64 a2, Int64 b2 ) GUCEF_MATH_REALTIME_CONTEXT
    {
        return CORE::CInt128::Add( CORE::CInt128::Add( CORE::CInt128::Multiply( a0, b0 ), CORE::CInt128::Multiply( a1, b1 ) ), CORE::CInt128::Multiply( a2, b2 ) );
    }

    /** 3x3: the rows of the adjugate are c1 x c2, c2 x c0, c0 x c1 of the columns; det = c0 . ( c1 x c2 ) */
    static void Adjugate3( const Int64* a, CORE::CInt128* adjugate, CORE::CInt128& determinant ) GUCEF_MATH_REALTIME_CONTEXT
    {
        const Int64* col[ 3 ] = { a, a + 3, a + 6 };
        for ( UInt32 row=0; row<3; ++row )
        {
            const Int64* u = col[ ( row + 1 ) % 3 ];
            const Int64* v = col[ ( row + 2 ) % 3 ];
            adjugate[ row * 3 + 0 ] = CORE::CInt128( Minor2( u[ 1 ], v[ 2 ], u[ 2 ], v[ 1 ] ) );
            adjugate[ row * 3 + 1 ] = CORE::CInt128( Minor2( u[ 2 ], v[ 0 ], u[ 0 ], v[ 2 ] ) );
            adjugate[ row * 3 + 2 ] = CORE::CInt128( Minor2( u[ 0 ], v[ 1 ], u[ 1 ], v[ 0 ] ) );
        }
        determinant = Dot3( a[ 0 ], (Int64) adjugate[ 0 ].low, a[ 1 ], (Int64) adjugate[ 1 ].low, a[ 2 ], (Int64) adjugate[ 2 ].low );
    }

    /** 4x4 by the 2x2 minors of rows 0, 1 (s) and rows 2, 3 (c), as CTMatrix4D does in the kind */
    static void Adjugate4( const Int64* m, CORE::CInt128* b, CORE::CInt128& determinant ) GUCEF_MATH_REALTIME_CONTEXT
    {
        #define GUCEF_MATH_A4( r, c ) m[ ( c ) * 4 + ( r ) ]
        Int64 s[ 6 ];
        Int64 c[ 6 ];
        s[ 0 ] = Minor2( GUCEF_MATH_A4( 0, 0 ), GUCEF_MATH_A4( 1, 1 ), GUCEF_MATH_A4( 1, 0 ), GUCEF_MATH_A4( 0, 1 ) );
        s[ 1 ] = Minor2( GUCEF_MATH_A4( 0, 0 ), GUCEF_MATH_A4( 1, 2 ), GUCEF_MATH_A4( 1, 0 ), GUCEF_MATH_A4( 0, 2 ) );
        s[ 2 ] = Minor2( GUCEF_MATH_A4( 0, 0 ), GUCEF_MATH_A4( 1, 3 ), GUCEF_MATH_A4( 1, 0 ), GUCEF_MATH_A4( 0, 3 ) );
        s[ 3 ] = Minor2( GUCEF_MATH_A4( 0, 1 ), GUCEF_MATH_A4( 1, 2 ), GUCEF_MATH_A4( 1, 1 ), GUCEF_MATH_A4( 0, 2 ) );
        s[ 4 ] = Minor2( GUCEF_MATH_A4( 0, 1 ), GUCEF_MATH_A4( 1, 3 ), GUCEF_MATH_A4( 1, 1 ), GUCEF_MATH_A4( 0, 3 ) );
        s[ 5 ] = Minor2( GUCEF_MATH_A4( 0, 2 ), GUCEF_MATH_A4( 1, 3 ), GUCEF_MATH_A4( 1, 2 ), GUCEF_MATH_A4( 0, 3 ) );
        c[ 5 ] = Minor2( GUCEF_MATH_A4( 2, 2 ), GUCEF_MATH_A4( 3, 3 ), GUCEF_MATH_A4( 3, 2 ), GUCEF_MATH_A4( 2, 3 ) );
        c[ 4 ] = Minor2( GUCEF_MATH_A4( 2, 1 ), GUCEF_MATH_A4( 3, 3 ), GUCEF_MATH_A4( 3, 1 ), GUCEF_MATH_A4( 2, 3 ) );
        c[ 3 ] = Minor2( GUCEF_MATH_A4( 2, 1 ), GUCEF_MATH_A4( 3, 2 ), GUCEF_MATH_A4( 3, 1 ), GUCEF_MATH_A4( 2, 2 ) );
        c[ 2 ] = Minor2( GUCEF_MATH_A4( 2, 0 ), GUCEF_MATH_A4( 3, 3 ), GUCEF_MATH_A4( 3, 0 ), GUCEF_MATH_A4( 2, 3 ) );
        c[ 1 ] = Minor2( GUCEF_MATH_A4( 2, 0 ), GUCEF_MATH_A4( 3, 2 ), GUCEF_MATH_A4( 3, 0 ), GUCEF_MATH_A4( 2, 2 ) );
        c[ 0 ] = Minor2( GUCEF_MATH_A4( 2, 0 ), GUCEF_MATH_A4( 3, 1 ), GUCEF_MATH_A4( 3, 0 ), GUCEF_MATH_A4( 2, 1 ) );
        determinant = CORE::CInt128::Add( Dot3( s[ 0 ], c[ 5 ], -s[ 1 ], c[ 4 ], s[ 2 ], c[ 3 ] ), Dot3( s[ 3 ], c[ 2 ], -s[ 4 ], c[ 1 ], s[ 5 ], c[ 0 ] ) );
        b[ 0 ] = Dot3( GUCEF_MATH_A4( 1, 1 ), c[ 5 ], -GUCEF_MATH_A4( 1, 2 ), c[ 4 ], GUCEF_MATH_A4( 1, 3 ), c[ 3 ] );
        b[ 1 ] = Dot3( -GUCEF_MATH_A4( 0, 1 ), c[ 5 ], GUCEF_MATH_A4( 0, 2 ), c[ 4 ], -GUCEF_MATH_A4( 0, 3 ), c[ 3 ] );
        b[ 2 ] = Dot3( GUCEF_MATH_A4( 3, 1 ), s[ 5 ], -GUCEF_MATH_A4( 3, 2 ), s[ 4 ], GUCEF_MATH_A4( 3, 3 ), s[ 3 ] );
        b[ 3 ] = Dot3( -GUCEF_MATH_A4( 2, 1 ), s[ 5 ], GUCEF_MATH_A4( 2, 2 ), s[ 4 ], -GUCEF_MATH_A4( 2, 3 ), s[ 3 ] );
        b[ 4 ] = Dot3( -GUCEF_MATH_A4( 1, 0 ), c[ 5 ], GUCEF_MATH_A4( 1, 2 ), c[ 2 ], -GUCEF_MATH_A4( 1, 3 ), c[ 1 ] );
        b[ 5 ] = Dot3( GUCEF_MATH_A4( 0, 0 ), c[ 5 ], -GUCEF_MATH_A4( 0, 2 ), c[ 2 ], GUCEF_MATH_A4( 0, 3 ), c[ 1 ] );
        b[ 6 ] = Dot3( -GUCEF_MATH_A4( 3, 0 ), s[ 5 ], GUCEF_MATH_A4( 3, 2 ), s[ 2 ], -GUCEF_MATH_A4( 3, 3 ), s[ 1 ] );
        b[ 7 ] = Dot3( GUCEF_MATH_A4( 2, 0 ), s[ 5 ], -GUCEF_MATH_A4( 2, 2 ), s[ 2 ], GUCEF_MATH_A4( 2, 3 ), s[ 1 ] );
        b[ 8 ] = Dot3( GUCEF_MATH_A4( 1, 0 ), c[ 4 ], -GUCEF_MATH_A4( 1, 1 ), c[ 2 ], GUCEF_MATH_A4( 1, 3 ), c[ 0 ] );
        b[ 9 ] = Dot3( -GUCEF_MATH_A4( 0, 0 ), c[ 4 ], GUCEF_MATH_A4( 0, 1 ), c[ 2 ], -GUCEF_MATH_A4( 0, 3 ), c[ 0 ] );
        b[ 10 ] = Dot3( GUCEF_MATH_A4( 3, 0 ), s[ 4 ], -GUCEF_MATH_A4( 3, 1 ), s[ 2 ], GUCEF_MATH_A4( 3, 3 ), s[ 0 ] );
        b[ 11 ] = Dot3( -GUCEF_MATH_A4( 2, 0 ), s[ 4 ], GUCEF_MATH_A4( 2, 1 ), s[ 2 ], -GUCEF_MATH_A4( 2, 3 ), s[ 0 ] );
        b[ 12 ] = Dot3( -GUCEF_MATH_A4( 1, 0 ), c[ 3 ], GUCEF_MATH_A4( 1, 1 ), c[ 1 ], -GUCEF_MATH_A4( 1, 2 ), c[ 0 ] );
        b[ 13 ] = Dot3( GUCEF_MATH_A4( 0, 0 ), c[ 3 ], -GUCEF_MATH_A4( 0, 1 ), c[ 1 ], GUCEF_MATH_A4( 0, 2 ), c[ 0 ] );
        b[ 14 ] = Dot3( -GUCEF_MATH_A4( 3, 0 ), s[ 3 ], GUCEF_MATH_A4( 3, 1 ), s[ 1 ], -GUCEF_MATH_A4( 3, 2 ), s[ 0 ] );
        b[ 15 ] = Dot3( GUCEF_MATH_A4( 2, 0 ), s[ 3 ], -GUCEF_MATH_A4( 2, 1 ), s[ 1 ], GUCEF_MATH_A4( 2, 2 ), s[ 0 ] );
        #undef GUCEF_MATH_A4
    }

    /** det == 0, or |det| <= tolerance * 2^( 30 n ) in raw units of the tolerance */
    static bool IsSingular( const CORE::CInt128& determinant, UInt32 n, const T& tolerance ) GUCEF_MATH_REALTIME_CONTEXT
    {
        const CORE::CInt128 absolute = determinant.IsNegative() ? CORE::CInt128::Negate( determinant ) : determinant;
        const CORE::CUInt128 magnitude( absolute.high, absolute.low );
        if ( magnitude.IsZero() )
            return true;
        const Int64 rawTolerance = (Int64) tolerance.GetRaw();
        if ( rawTolerance <= 0 )
            return false;
        const Int32 shift = (Int32) ( MantissaBits * n ) - (Int32) F;
        CORE::CUInt128 threshold( (UInt64) rawTolerance );
        if ( shift >= 0 )
        {
            if ( BitLength( threshold ) + (UInt32) shift >= 127 )
                return true;    // beyond any determinant (|det| < 2^125)
            threshold = CORE::CUInt128::ShiftLeft( threshold, (UInt32) shift );
        }
        else
        {
            threshold = CORE::CUInt128::ShiftRight( threshold, (UInt32) -shift );
        }
        return CORE::CUInt128::Compare( magnitude, threshold ) <= 0;
    }

    /** round( numerator * 2^k / denominator ), ties away from zero, saturated to the storage (denominator != 0) */
    static TStorage ScaledQuotient( const CORE::CInt128& numerator, Int32 k, const CORE::CInt128& denominator ) GUCEF_MATH_REALTIME_CONTEXT
    {
        const bool negative = numerator.IsNegative() != denominator.IsNegative();
        const CORE::CInt128 nAbs = numerator.IsNegative() ? CORE::CInt128::Negate( numerator ) : numerator;
        const CORE::CInt128 dAbs = denominator.IsNegative() ? CORE::CInt128::Negate( denominator ) : denominator;
        const CORE::CUInt128 n( nAbs.high, nAbs.low );
        const CORE::CUInt128 d( dAbs.high, dAbs.low );
        const UInt64 limit = MaxPositiveRaw() + ( negative ? 1 : 0 );
        const TStorage saturated = negative ? T::Lowest().GetRaw() : T::Highest().GetRaw();
        if ( n.IsZero() )
            return 0;

        CORE::CUInt128 q;
        CORE::CUInt128 r;
        CORE::CUInt128::DivMod( n, d, q, r );
        if ( k < 0 )
        {
            const UInt32 right = (UInt32) -k;
            if ( right > 127 )
                return 0;
            const bool roundUp = 0 != CORE::CUInt128::ShiftRight( q, right - 1 ).IsOdd();
            q = CORE::CUInt128::ShiftRight( q, right );
            if ( roundUp )
                q = CORE::CUInt128::Add( q, CORE::CUInt128( (UInt64) 1 ) );
        }
        else
        {
            // long division in chunks: r < d, so r can be shifted by its leading zeros without overflow
            UInt32 remaining = (UInt32) k;
            while ( remaining > 0 )
            {
                // the result is at least q 2^remaining: beyond every storage once that needs more than 64 bits
                if ( !q.IsZero() && BitLength( q ) + remaining > 64 + 1 )
                    return saturated;
                UInt32 step = 128 - BitLength( r );
                if ( step > remaining )
                    step = remaining;
                if ( 0 == step )
                    step = 1;    // not reached: r < d < 2^127 leaves a leading zero
                CORE::CUInt128 chunk;
                CORE::CUInt128::DivMod( CORE::CUInt128::ShiftLeft( r, step ), d, chunk, r );
                q = CORE::CUInt128::Add( CORE::CUInt128::ShiftLeft( q, step ), chunk );
                remaining -= step;
            }
            // half away from zero: 2 r >= d
            if ( CORE::CUInt128::Compare( r, CORE::CUInt128::Subtract( d, r ) ) >= 0 )
                q = CORE::CUInt128::Add( q, CORE::CUInt128( (UInt64) 1 ) );
        }
        if ( 0 != q.high || q.low > limit )
            return saturated;
        return (TStorage) MathApplySign( negative, q.low );
    }
};

/*-------------------------------------------------------------------------*/

/**
 *  A 3x3 matrix: the linear maps of space (rotation, scale, shear) and the homogeneous affine maps of the plane.
 *
 *  - Kind: T is any numeric kind of CTNumericTraits. Inversion needs division; rotation builders need
 *    transcendentals; quaternion conversion, orthonormalisation and direction transforms need a square root.
 *  - Layout: column-major T m_data[ 9 ] (element (r, c) at c * 3 + r), private; access through operator()( row, column ),
 *    GetColumn, GetRow, GetData (column-major), ToRowMajor. Size 9 * sizeof( T ), alignment that of T.
 *  - Conventions (section 4.1 of the design): column vectors, v' = M v; positive rotations follow the right-hand rule
 *    (RotationY( 90 deg ) * UnitX == -UnitZ); 2D homogeneous maps use (x, y, 1) with the translation in column 2.
 *  - Products: every element is a 3 term dot product of the component kernels (fixed point rounds once).
 *  - Realtime: every member except the text and DataNode members is realtime-safe.
 *  - Text "[[m00, m01, m02], [m10, ...], [m20, ...]]" (row order); DataNode attributes m00 ... m22.
 */
template < typename T >
class GUCEF_MATH_TEMPLATE_PUBLIC CTMatrix3D
{
    public:

    typedef T TValue;
    typedef CTNumericTraits< T > TTraits;
    typedef typename CTMathComponentTraits< T >::TKernels TKernels;
    typedef CTVector3D< T > TVector;

    /** The identity */
    GUCEF_MATH_FORCE_INLINE CTMatrix3D( void )
    {
        for ( UInt32 i=0; i<9; ++i )
            m_data[ i ] = ( 0 == i % 4 ) ? TTraits::One() : TTraits::Zero();
    }

    /** Converts every element with CTNumericConverter (half to even, saturating; NaN into an exact kind gives 0) */
    template < typename U >
    explicit CTMatrix3D( const CTMatrix3D< U >& other )
    {
        TMathStatus status = MATHSTATUS_OK;
        for ( UInt32 i=0; i<9; ++i )
        {
            m_data[ i ] = TTraits::Zero();
            status |= CTNumericConverter< U, T >::Convert( other.GetData()[ i ], m_data[ i ], CORE::ROUNDINGMODE_HALF_EVEN );
        }
        GUCEF_MATH_ASSERT( 0 == ( status & MATHSTATUS_INVALID_OPERAND ) );
        (void) status;
    }

    /** The 2x2 matrix in the upper-left block, the rest of the identity */
    explicit CTMatrix3D( const CTMatrix2D< T >& linear )
    {
        *this = CTMatrix3D();
        for ( UInt32 c=0; c<2; ++c )
            for ( UInt32 r=0; r<2; ++r )
                m_data[ c * 3 + r ] = linear( r, c );
    }

    static GUCEF_MATH_FORCE_INLINE CTMatrix3D Identity( void ) GUCEF_MATH_REALTIME_CONTEXT { return CTMatrix3D(); }

    static GUCEF_MATH_FORCE_INLINE CTMatrix3D Zero( void ) GUCEF_MATH_REALTIME_CONTEXT
    {
        CTMatrix3D m;
        for ( UInt32 i=0; i<9; ++i )
            m.m_data[ i ] = TTraits::Zero();
        return m;
    }

    static GUCEF_MATH_FORCE_INLINE CTMatrix3D FromColumns( const TVector& c0, const TVector& c1, const TVector& c2 ) GUCEF_MATH_REALTIME_CONTEXT
    {
        CTMatrix3D m;
        m.SetColumn( 0, c0 );
        m.SetColumn( 1, c1 );
        m.SetColumn( 2, c2 );
        return m;
    }

    /** From 9 values in column-major order (the GetData order) */
    static GUCEF_MATH_FORCE_INLINE CTMatrix3D FromColumnMajor( const T* values9 ) GUCEF_MATH_REALTIME_CONTEXT
    {
        CTMatrix3D m;
        for ( UInt32 i=0; i<9; ++i )
            m.m_data[ i ] = values9[ i ];
        return m;
    }

    /** From 9 values in row-major order (m00, m01, m02, m10, ...) */
    static GUCEF_MATH_FORCE_INLINE CTMatrix3D FromRowMajor( const T* values9 ) GUCEF_MATH_REALTIME_CONTEXT
    {
        CTMatrix3D m;
        for ( UInt32 r=0; r<3; ++r )
            for ( UInt32 c=0; c<3; ++c )
                m.m_data[ c * 3 + r ] = values9[ r * 3 + c ];
        return m;
    }

    /*---------------------------------------------------------------------*/
    /*  Builders                                                            */
    /*---------------------------------------------------------------------*/

    /** diag( s.x, s.y, s.z ) */
    static GUCEF_MATH_FORCE_INLINE CTMatrix3D Scale( const TVector& s ) GUCEF_MATH_REALTIME_CONTEXT
    {
        CTMatrix3D m;
        m.m_data[ 0 ] = s.x;
        m.m_data[ 4 ] = s.y;
        m.m_data[ 8 ] = s.z;
        return m;
    }

    /** Rotation about +X by the right-hand rule (HasTranscendentals): RotationX( 90 deg ) * UnitY == UnitZ */
    static CTMatrix3D RotationX( const CTRadian< T >& angle ) GUCEF_MATH_REALTIME_CONTEXT;

    /** Rotation about +Y by the right-hand rule: RotationY( 90 deg ) * UnitX == -UnitZ */
    static CTMatrix3D RotationY( const CTRadian< T >& angle ) GUCEF_MATH_REALTIME_CONTEXT;

    /** Rotation about +Z by the right-hand rule: RotationZ( 90 deg ) * UnitX == UnitY */
    static CTMatrix3D RotationZ( const CTRadian< T >& angle ) GUCEF_MATH_REALTIME_CONTEXT;

    /** Rotation about 'axis' by the right-hand rule (Rodrigues, with 1 - cos computed as 2 sin^2( angle / 2 )) */
    static CTMatrix3D RotationAxisAngle( const CTDirectionVector3D< T >& axis, const CTRadian< T >& angle ) GUCEF_MATH_REALTIME_CONTEXT;

    /** The rotation of q (HasSqrt); q need not be of unit length (it is scaled by 2 / |q|^2); zero q gives the identity */
    static CTMatrix3D FromQuaternion( const CTQuaternion< T >& q ) GUCEF_MATH_REALTIME_CONTEXT;

    /** 2D homogeneous translation: (x, y, 1) -> (x + t.x, y + t.y, 1) */
    static GUCEF_MATH_FORCE_INLINE CTMatrix3D Translation2D( const CTVector2D< T >& t ) GUCEF_MATH_REALTIME_CONTEXT
    {
        CTMatrix3D m;
        m.m_data[ 6 ] = t.x;
        m.m_data[ 7 ] = t.y;
        return m;
    }

    /** 2D homogeneous rotation about the origin, counter-clockwise positive (HasTranscendentals) */
    static CTMatrix3D Rotation2D( const CTRadian< T >& angle ) GUCEF_MATH_REALTIME_CONTEXT;

    /** 2D homogeneous scale */
    static GUCEF_MATH_FORCE_INLINE CTMatrix3D Scale2D( const CTVector2D< T >& s ) GUCEF_MATH_REALTIME_CONTEXT
    {
        CTMatrix3D m;
        m.m_data[ 0 ] = s.x;
        m.m_data[ 4 ] = s.y;
        return m;
    }

    /*---------------------------------------------------------------------*/
    /*  Access                                                              */
    /*---------------------------------------------------------------------*/

    /** Element (row, column); an index out of range asserts in debug builds and is clamped */
    GUCEF_MATH_FORCE_INLINE T operator()( UInt32 row, UInt32 column ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        GUCEF_MATH_ASSERT( row < 3 && column < 3 );
        return m_data[ Index( row, column ) ];
    }

    GUCEF_MATH_FORCE_INLINE T& operator()( UInt32 row, UInt32 column ) GUCEF_MATH_REALTIME_CONTEXT
    {
        GUCEF_MATH_ASSERT( row < 3 && column < 3 );
        return m_data[ Index( row, column ) ];
    }

    GUCEF_MATH_FORCE_INLINE TVector GetColumn( UInt32 c ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        return TVector( ( *this )( 0, c ), ( *this )( 1, c ), ( *this )( 2, c ) );
    }

    GUCEF_MATH_FORCE_INLINE void SetColumn( UInt32 c, const TVector& v ) GUCEF_MATH_REALTIME_CONTEXT
    {
        ( *this )( 0, c ) = v.x;
        ( *this )( 1, c ) = v.y;
        ( *this )( 2, c ) = v.z;
    }

    GUCEF_MATH_FORCE_INLINE TVector GetRow( UInt32 r ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        return TVector( ( *this )( r, 0 ), ( *this )( r, 1 ), ( *this )( r, 2 ) );
    }

    /** The 9 elements in column-major order */
    GUCEF_MATH_FORCE_INLINE const T* GetData( void ) const GUCEF_MATH_REALTIME_CONTEXT { return m_data; }

    /** Writes the 9 elements in row-major order */
    GUCEF_MATH_FORCE_INLINE void ToRowMajor( T* out9 ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        for ( UInt32 r=0; r<3; ++r )
            for ( UInt32 c=0; c<3; ++c )
                out9[ r * 3 + c ] = m_data[ c * 3 + r ];
    }

    /** The upper-left 2x2 block */
    GUCEF_MATH_FORCE_INLINE CTMatrix2D< T > GetLinear2D( void ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        const T values[ 4 ] = { m_data[ 0 ], m_data[ 1 ], m_data[ 3 ], m_data[ 4 ] };
        return CTMatrix2D< T >::FromColumnMajor( values );
    }

    /*---------------------------------------------------------------------*/
    /*  Algebra                                                             */
    /*---------------------------------------------------------------------*/

    friend GUCEF_MATH_FORCE_INLINE CTMatrix3D operator+( const CTMatrix3D& a, const CTMatrix3D& b ) GUCEF_MATH_REALTIME_CONTEXT
    {
        CTMatrix3D m;
        for ( UInt32 i=0; i<9; ++i )
            m.m_data[ i ] = TKernels::Add( a.m_data[ i ], b.m_data[ i ] );
        return m;
    }

    friend GUCEF_MATH_FORCE_INLINE CTMatrix3D operator-( const CTMatrix3D& a, const CTMatrix3D& b ) GUCEF_MATH_REALTIME_CONTEXT
    {
        CTMatrix3D m;
        for ( UInt32 i=0; i<9; ++i )
            m.m_data[ i ] = TKernels::Subtract( a.m_data[ i ], b.m_data[ i ] );
        return m;
    }

    GUCEF_MATH_FORCE_INLINE CTMatrix3D operator-( void ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        CTMatrix3D m;
        for ( UInt32 i=0; i<9; ++i )
            m.m_data[ i ] = TKernels::Negate( m_data[ i ] );
        return m;
    }

    friend GUCEF_MATH_FORCE_INLINE CTMatrix3D operator*( const CTMatrix3D& a, const T& factor ) GUCEF_MATH_REALTIME_CONTEXT
    {
        CTMatrix3D m;
        for ( UInt32 i=0; i<9; ++i )
            m.m_data[ i ] = TKernels::Multiply( a.m_data[ i ], factor );
        return m;
    }

    friend GUCEF_MATH_FORCE_INLINE CTMatrix3D operator*( const T& factor, const CTMatrix3D& a ) GUCEF_MATH_REALTIME_CONTEXT
    {
        return a * factor;
    }

    /** a * b: applies b first */
    friend GUCEF_MATH_FORCE_INLINE CTMatrix3D operator*( const CTMatrix3D& a, const CTMatrix3D& b ) GUCEF_MATH_REALTIME_CONTEXT
    {
        return a.Multiply( b );
    }

    friend GUCEF_MATH_FORCE_INLINE TVector operator*( const CTMatrix3D& m, const TVector& v ) GUCEF_MATH_REALTIME_CONTEXT
    {
        return m.Transform( v );
    }

    GUCEF_MATH_FORCE_INLINE CTMatrix3D& operator*=( const CTMatrix3D& other ) GUCEF_MATH_REALTIME_CONTEXT { *this = Multiply( other ); return *this; }

    friend GUCEF_MATH_FORCE_INLINE bool operator==( const CTMatrix3D& a, const CTMatrix3D& b ) GUCEF_MATH_REALTIME_CONTEXT
    {
        for ( UInt32 i=0; i<9; ++i )
            if ( !( a.m_data[ i ] == b.m_data[ i ] ) )
                return false;
        return true;
    }

    friend GUCEF_MATH_FORCE_INLINE bool operator!=( const CTMatrix3D& a, const CTMatrix3D& b ) GUCEF_MATH_REALTIME_CONTEXT
    {
        return !( a == b );
    }

    /** this * other (other applied first); every element a 3 term dot product of the kind */
    CTMatrix3D Multiply( const CTMatrix3D& other ) const GUCEF_MATH_REALTIME_CONTEXT;

    /** this * v */
    TVector Transform( const TVector& v ) const GUCEF_MATH_REALTIME_CONTEXT;

    /** The direction of this * d (HasSqrt); d itself when the result is zero or not finite */
    CTDirectionVector3D< T > TransformDirection( const CTDirectionVector3D< T >& d ) const GUCEF_MATH_REALTIME_CONTEXT;

    /**
     *  The surface normal n transformed by the inverse transpose, renormalised (HasSqrt): computed from the cofactor
     *  matrix (no division), with the sign of the determinant, so reflections keep normals outward. n itself when the
     *  matrix is singular.
     */
    CTDirectionVector3D< T > TransformNormal( const CTDirectionVector3D< T >& n ) const GUCEF_MATH_REALTIME_CONTEXT;

    /** The 2D homogeneous map of a point: (x, y, 1), affine (the last row is not used, no division) */
    CTPoint2D< T > TransformPoint2D( const CTPoint2D< T >& p ) const GUCEF_MATH_REALTIME_CONTEXT;

    /** The 2D homogeneous map of a displacement: (x, y, 0) */
    CTVector2D< T > TransformVector2D( const CTVector2D< T >& v ) const GUCEF_MATH_REALTIME_CONTEXT;

    GUCEF_MATH_FORCE_INLINE CTMatrix3D Transposed( void ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        CTMatrix3D m;
        for ( UInt32 r=0; r<3; ++r )
            for ( UInt32 c=0; c<3; ++c )
                m.m_data[ r * 3 + c ] = m_data[ c * 3 + r ];
        return m;
    }

    /** c0 . ( c1 x c2 ) of the columns */
    T GetDeterminant( void ) const GUCEF_MATH_REALTIME_CONTEXT;

    /**
     *  The inverse (HasDivision) from the cofactors (rows c1 x c2, c2 x c0, c0 x c1 divided by the determinant) of the
     *  equilibrated matrix E = Dr M Dc: each row, then each column, is scaled by the power of two that brings its largest
     *  magnitude into [0.5, 1) (exact; it keeps fixed point precise at small and large scales). False, 'out' unchanged,
     *  for a zero row or column, or when det( E ) is not finite or |det( E )| <= singularTolerance (>= 0): a relative
     *  test, so the outcome does not depend on the scale of the world or of single rows and columns.
     *  Fixed point: the adjugate and the determinant are exact in 128 bits (CTMathExactMatrixInverse), every element is
     *  rounded once. The equilibration keeps 30 significant bits per value: raw magnitudes above 2^30 (Q16.16 and Q48.16
     *  values of 16384 and more, Q32.32 values of 0.25 and more) are rounded first, a relative input error up to 2^-31
     *  that the condition number amplifies (Q32.32 rotations and translations: errors of 10^4 to 10^6 ULP). Without
     *  such rounding (Q16.16 below 16384) the result is within half a unit in the last place of the exact inverse.
     */
    bool TryInverse( CTMatrix3D& out, const T& singularTolerance ) const GUCEF_MATH_REALTIME_CONTEXT;

    /** The inverse (HasDivision); the zero matrix and a debug assertion when the determinant is zero */
    CTMatrix3D Inverse( void ) const GUCEF_MATH_REALTIME_CONTEXT;

    /**
     *  Gram-Schmidt in column order (HasSqrt): column 0 normalised, column 1 made orthogonal to it and normalised,
     *  column 2 orthogonal to both. A degenerate column is replaced: column 0 by +X, column 1 by a perpendicular of
     *  column 0, column 2 by the cross product of the first two (so the result is a rotation when the input was close
     *  to one).
     */
    CTMatrix3D Orthonormalized( void ) const GUCEF_MATH_REALTIME_CONTEXT;

    /**
     *  The quaternion of this rotation matrix by Shepperd's method (HasSqrt), normalised; the matrix must be
     *  orthonormal with determinant 1 (otherwise the result is the quaternion of a nearby rotation)
     */
    CTQuaternion< T > ToQuaternion( void ) const GUCEF_MATH_REALTIME_CONTEXT;

    /*---------------------------------------------------------------------*/
    /*  Queries and tolerant comparison                                     */
    /*---------------------------------------------------------------------*/

    GUCEF_MATH_FORCE_INLINE bool IsIdentity( void ) const GUCEF_MATH_REALTIME_CONTEXT { return *this == CTMatrix3D(); }

    GUCEF_MATH_FORCE_INLINE bool IsFinite( void ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        for ( UInt32 i=0; i<9; ++i )
            if ( !TTraits::IsFinite( m_data[ i ] ) )
                return false;
        return true;
    }

    /** Every element equal within the default tolerances of the kind */
    GUCEF_MATH_FORCE_INLINE bool Equals( const CTMatrix3D& other ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        return Equals( other, TTraits::DefaultAbsoluteTolerance(), TTraits::DefaultRelativeTolerance() );
    }

    /** Every element: |a - b| <= max( absolute, relative * max( |a|, |b| ) ) */
    bool Equals( const CTMatrix3D& other, const T& absolute, const T& relative ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        for ( UInt32 i=0; i<9; ++i )
            if ( !TTraits::EqualsWithin( m_data[ i ], other.m_data[ i ], absolute, relative ) )
                return false;
        return true;
    }

    /*---------------------------------------------------------------------*/
    /*  Text and DataNode (not realtime-safe: they allocate)                */
    /*---------------------------------------------------------------------*/

    /** "[[m00, m01, m02], [m10, m11, m12], [m20, m21, m22]]" */
    CORE::CString ToString( void ) const
    {
        T rows[ 9 ];
        ToRowMajor( rows );
        return CTMatrixComponentsCodec< T, 3, 3 >::ToString( rows );
    }

    /** Parses the ToString form; false leaves this matrix unchanged */
    bool FromString( const CORE::CString& text )
    {
        T rows[ 9 ];
        if ( !CTMatrixComponentsCodec< T, 3, 3 >::FromString( text, rows ) )
            return false;
        *this = FromRowMajor( rows );
        return true;
    }

    /** Writes the attributes m00 ... m22 */
    bool Serialize( CORE::CDataNode& node, const CORE::CDataNodeSerializableSettings& /*settings*/ ) const
    {
        T rows[ 9 ];
        ToRowMajor( rows );
        return CTMatrixComponentsCodec< T, 3, 3 >::Serialize( node, rows );
    }

    /** Reads the attributes m00 ... m22; a missing or invalid attribute fails and leaves this matrix unchanged */
    bool Deserialize( const CORE::CDataNode& node, const CORE::CDataNodeSerializableSettings& /*settings*/ )
    {
        T rows[ 9 ];
        if ( !CTMatrixComponentsCodec< T, 3, 3 >::Deserialize( node, rows ) )
            return false;
        *this = FromRowMajor( rows );
        return true;
    }

    private:

    static GUCEF_MATH_FORCE_INLINE UInt32 Index( UInt32 row, UInt32 column ) GUCEF_MATH_REALTIME_CONTEXT
    {
        return ( column < 3 ? column : 2 ) * 3 + ( row < 3 ? row : 2 );
    }

    /** The elementary rotation about axis 'axis' (0 = X, 1 = Y, 2 = Z) by the right-hand rule */
    static CTMatrix3D AxisRotation( UInt32 axis, const CTRadian< T >& angle ) GUCEF_MATH_REALTIME_CONTEXT
    {
        GUCEF_MATH_STATIC_ASSERT( TTraits::HasTranscendentals, Rotation_requires_transcendentals );
        const T c = TTraits::Cos( angle.GetValue() );
        const T s = TTraits::Sin( angle.GetValue() );
        const UInt32 j = ( axis + 1 ) % 3;
        const UInt32 k = ( axis + 2 ) % 3;
        CTMatrix3D m;
        m( j, j ) = c;
        m( k, k ) = c;
        m( k, j ) = s;
        m( j, k ) = ComponentNegate( s );
        return m;
    }

    T m_data[ 9 ];
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *  The precision-sensitive members, out of the class: with GUCEF_MATH_PRECISE_INLINE (empty by default) they are not
 *  inline, so for the predefined kinds the copy compiled in the module (without floating-point contraction) runs.
 */

template < typename T >
GUCEF_MATH_PRECISE_INLINE CTMatrix3D< T >
CTMatrix3D< T >::RotationX( const CTRadian< T >& angle ) GUCEF_MATH_REALTIME_CONTEXT
{
    return AxisRotation( 0, angle );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE CTMatrix3D< T >
CTMatrix3D< T >::RotationY( const CTRadian< T >& angle ) GUCEF_MATH_REALTIME_CONTEXT
{
    return AxisRotation( 1, angle );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE CTMatrix3D< T >
CTMatrix3D< T >::RotationZ( const CTRadian< T >& angle ) GUCEF_MATH_REALTIME_CONTEXT
{
    return AxisRotation( 2, angle );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE CTMatrix3D< T >
CTMatrix3D< T >::RotationAxisAngle( const CTDirectionVector3D< T >& axis, const CTRadian< T >& angle ) GUCEF_MATH_REALTIME_CONTEXT
{
    GUCEF_MATH_STATIC_ASSERT( TTraits::HasTranscendentals, Rotation_requires_transcendentals );
    const T c = TTraits::Cos( angle.GetValue() );
    const T s = TTraits::Sin( angle.GetValue() );
    const T halfSine = TTraits::Sin( angle.GetValue() * TTraits::Half() );
    const T oneMinusCos = TTraits::Two() * ( halfSine * halfSine );
    const TVector k = axis.AsVector();
    const T kk[ 3 ] = { k.x, k.y, k.z };
    // element (r, c) = ( 1 - cos ) k_r k_c + cos delta_rc + sin epsilon_rc, as one 2 term dot product
    CTMatrix3D m;
    for ( UInt32 r=0; r<3; ++r )
    {
        const T scaled = oneMinusCos * kk[ r ];
        for ( UInt32 col=0; col<3; ++col )
        {
            T a[ 2 ] = { scaled, TTraits::Zero() };
            T b[ 2 ] = { kk[ col ], TTraits::Zero() };
            if ( r == col )
            {
                a[ 1 ] = c;
                b[ 1 ] = TTraits::One();
            }
            else
            {
                // the remaining axis o and the sign of the permutation ( r, col, o )
                const UInt32 o = 3 - r - col;
                a[ 1 ] = ( ( r + 1 ) % 3 == col ) ? ComponentNegate( s ) : s;
                b[ 1 ] = kk[ o ];
            }
            m.m_data[ col * 3 + r ] = TKernels::Dot( a, b );
        }
    }
    return m;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE CTMatrix3D< T >
CTMatrix3D< T >::FromQuaternion( const CTQuaternion< T >& q ) GUCEF_MATH_REALTIME_CONTEXT
{
    GUCEF_MATH_STATIC_ASSERT( TTraits::HasSqrt, Quaternion_conversion_requires_a_numeric_kind_with_sqrt );
    const T x = q.x, y = q.y, z = q.z, w = q.w;
    const T qq[ 4 ] = { x, y, z, w };
    const T n = CTVector4DotOrder< T, TTraits::IsFloatingPoint >::Dot( qq, qq );
    if ( !( TTraits::Zero() < n ) )
        return CTMatrix3D();
    const T s = TTraits::Two() / n;
    const T sx = s * x, sy = s * y, sz = s * z;
    const T one = TTraits::One();
    // every element one 2 term dot product of the kind
    const T e00a[ 2 ] = { sy, sz }; const T e00b[ 2 ] = { y, z };
    const T e11a[ 2 ] = { sx, sz }; const T e11b[ 2 ] = { x, z };
    const T e22a[ 2 ] = { sx, sy }; const T e22b[ 2 ] = { x, y };
    const T e01a[ 2 ] = { sx, ComponentNegate( sz ) }; const T e01b[ 2 ] = { y, w };
    const T e10a[ 2 ] = { sx, sz }; const T e10b[ 2 ] = { y, w };
    const T e02a[ 2 ] = { sx, sy }; const T e02b[ 2 ] = { z, w };
    const T e20a[ 2 ] = { sx, ComponentNegate( sy ) }; const T e20b[ 2 ] = { z, w };
    const T e12a[ 2 ] = { sy, ComponentNegate( sx ) }; const T e12b[ 2 ] = { z, w };
    const T e21a[ 2 ] = { sy, sx }; const T e21b[ 2 ] = { z, w };
    CTMatrix3D m;
    m.m_data[ 0 ] = one - TKernels::Dot( e00a, e00b );
    m.m_data[ 4 ] = one - TKernels::Dot( e11a, e11b );
    m.m_data[ 8 ] = one - TKernels::Dot( e22a, e22b );
    m.m_data[ 3 ] = TKernels::Dot( e01a, e01b );    // (0, 1)
    m.m_data[ 1 ] = TKernels::Dot( e10a, e10b );    // (1, 0)
    m.m_data[ 6 ] = TKernels::Dot( e02a, e02b );    // (0, 2)
    m.m_data[ 2 ] = TKernels::Dot( e20a, e20b );    // (2, 0)
    m.m_data[ 7 ] = TKernels::Dot( e12a, e12b );    // (1, 2)
    m.m_data[ 5 ] = TKernels::Dot( e21a, e21b );    // (2, 1)
    return m;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE CTMatrix3D< T >
CTMatrix3D< T >::Rotation2D( const CTRadian< T >& angle ) GUCEF_MATH_REALTIME_CONTEXT
{
    return AxisRotation( 2, angle );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE CTMatrix3D< T >
CTMatrix3D< T >::Multiply( const CTMatrix3D& other ) const GUCEF_MATH_REALTIME_CONTEXT
{
    CTMatrix3D m;
    for ( UInt32 r=0; r<3; ++r )
    {
        const T row[ 3 ] = { m_data[ r ], m_data[ 3 + r ], m_data[ 6 + r ] };
        for ( UInt32 c=0; c<3; ++c )
        {
            const T column[ 3 ] = { other.m_data[ c * 3 ], other.m_data[ c * 3 + 1 ], other.m_data[ c * 3 + 2 ] };
            m.m_data[ c * 3 + r ] = TKernels::Dot( row, column );
        }
    }
    return m;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE typename CTMatrix3D< T >::TVector
CTMatrix3D< T >::Transform( const TVector& v ) const GUCEF_MATH_REALTIME_CONTEXT
{
    const T b[ 3 ] = { v.x, v.y, v.z };
    T result[ 3 ];
    for ( UInt32 r=0; r<3; ++r )
    {
        const T row[ 3 ] = { m_data[ r ], m_data[ 3 + r ], m_data[ 6 + r ] };
        result[ r ] = TKernels::Dot( row, b );
    }
    return TVector( result[ 0 ], result[ 1 ], result[ 2 ] );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE CTDirectionVector3D< T >
CTMatrix3D< T >::TransformDirection( const CTDirectionVector3D< T >& d ) const GUCEF_MATH_REALTIME_CONTEXT
{
    CTDirectionVector3D< T > result( d );
    if ( !CTDirectionVector3D< T >::TryCreate( Transform( d.AsVector() ), result ) )
        return d;
    return result;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE CTDirectionVector3D< T >
CTMatrix3D< T >::TransformNormal( const CTDirectionVector3D< T >& n ) const GUCEF_MATH_REALTIME_CONTEXT
{
    const TVector c0 = GetColumn( 0 );
    const TVector c1 = GetColumn( 1 );
    const TVector c2 = GetColumn( 2 );
    // ( M^-1 )^T = [ c1 x c2 | c2 x c0 | c0 x c1 ] / det (columns)
    const CTMatrix3D cofactors = FromColumns( c1.Cross( c2 ), c2.Cross( c0 ), c0.Cross( c1 ) );
    TVector transformed = cofactors.Transform( n.AsVector() );
    if ( TTraits::IsNegative( GetDeterminant() ) )
        transformed = -transformed;
    CTDirectionVector3D< T > result( n );
    if ( !CTDirectionVector3D< T >::TryCreate( transformed, result ) )
        return n;
    return result;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE CTPoint2D< T >
CTMatrix3D< T >::TransformPoint2D( const CTPoint2D< T >& p ) const GUCEF_MATH_REALTIME_CONTEXT
{
    const T b[ 3 ] = { p.x, p.y, TTraits::One() };
    const T row0[ 3 ] = { m_data[ 0 ], m_data[ 3 ], m_data[ 6 ] };
    const T row1[ 3 ] = { m_data[ 1 ], m_data[ 4 ], m_data[ 7 ] };
    return CTPoint2D< T >( TKernels::Dot( row0, b ), TKernels::Dot( row1, b ) );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE CTVector2D< T >
CTMatrix3D< T >::TransformVector2D( const CTVector2D< T >& v ) const GUCEF_MATH_REALTIME_CONTEXT
{
    const T b[ 2 ] = { v.x, v.y };
    const T row0[ 2 ] = { m_data[ 0 ], m_data[ 3 ] };
    const T row1[ 2 ] = { m_data[ 1 ], m_data[ 4 ] };
    return CTVector2D< T >( TKernels::Dot( row0, b ), TKernels::Dot( row1, b ) );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE T
CTMatrix3D< T >::GetDeterminant( void ) const GUCEF_MATH_REALTIME_CONTEXT
{
    return GetColumn( 0 ).Dot( GetColumn( 1 ).Cross( GetColumn( 2 ) ) );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE bool
CTMatrix3D< T >::TryInverse( CTMatrix3D& out, const T& singularTolerance ) const GUCEF_MATH_REALTIME_CONTEXT
{
    GUCEF_MATH_STATIC_ASSERT( TTraits::HasDivision, Inverse_requires_a_numeric_kind_with_division );
    typedef CTMathExactMatrixInverse< T, CTMathComponentKind< T >::Value > TExactInverse;
    if ( TExactInverse::IsAvailable )
    {
        // fixed point: exact 128 bit adjugate and determinant, one rounding per element
        T values[ 9 ];
        if ( !TExactInverse::TryInverse( m_data, 3, singularTolerance, values ) )
            return false;
        out = FromColumnMajor( values );
        return true;
    }
    typedef CTMathBinaryScaling< T, CTMathComponentKind< T >::Value > TScaling;
    // rows equilibrated by powers of two
    Int32 exponent[ 3 ];
    CTMatrix3D e;
    for ( UInt32 r=0; r<3; ++r )
    {
        const T largest = TTraits::Max( TTraits::Max( TTraits::Abs( m_data[ r ] ), TTraits::Abs( m_data[ 3 + r ] ) ), TTraits::Abs( m_data[ 6 + r ] ) );
        if ( !( TTraits::Zero() < largest ) )
            return false;
        exponent[ r ] = TScaling::NormalizingExponent( largest );
        for ( UInt32 c=0; c<3; ++c )
            e.m_data[ c * 3 + r ] = TScaling::Scale( m_data[ c * 3 + r ], exponent[ r ] );
    }
    // then the columns: E = Dr M Dc, M^-1 = Dc E^-1 Dr
    Int32 columnExponent[ 3 ];
    for ( UInt32 c=0; c<3; ++c )
    {
        const T largest = TTraits::Max( TTraits::Max( TTraits::Abs( e.m_data[ c * 3 ] ), TTraits::Abs( e.m_data[ c * 3 + 1 ] ) ), TTraits::Abs( e.m_data[ c * 3 + 2 ] ) );
        if ( !( TTraits::Zero() < largest ) )
            return false;
        columnExponent[ c ] = TScaling::NormalizingExponent( largest );
        for ( UInt32 r=0; r<3; ++r )
            e.m_data[ c * 3 + r ] = TScaling::Scale( e.m_data[ c * 3 + r ], columnExponent[ c ] );
    }
    const TVector c0 = e.GetColumn( 0 );
    const TVector c1 = e.GetColumn( 1 );
    const TVector c2 = e.GetColumn( 2 );
    const TVector r0 = c1.Cross( c2 );
    const T determinant = c0.Dot( r0 );
    if ( !TTraits::IsFinite( determinant ) || !( singularTolerance < TTraits::Abs( determinant ) ) )
        return false;
    const TVector r1 = c2.Cross( c0 );
    const TVector r2 = c0.Cross( c1 );
    const TVector rows[ 3 ] = { r0, r1, r2 };
    CTMatrix3D m;
    for ( UInt32 r=0; r<3; ++r )
    {
        m( r, 0 ) = TScaling::Scale( rows[ r ].x / determinant, columnExponent[ r ] + exponent[ 0 ] );
        m( r, 1 ) = TScaling::Scale( rows[ r ].y / determinant, columnExponent[ r ] + exponent[ 1 ] );
        m( r, 2 ) = TScaling::Scale( rows[ r ].z / determinant, columnExponent[ r ] + exponent[ 2 ] );
    }
    out = m;
    return true;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE CTMatrix3D< T >
CTMatrix3D< T >::Inverse( void ) const GUCEF_MATH_REALTIME_CONTEXT
{
    CTMatrix3D m = Zero();
    bool invertible = TryInverse( m, TTraits::Zero() );
    GUCEF_MATH_ASSERT( invertible );
    (void) invertible;
    return m;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE CTMatrix3D< T >
CTMatrix3D< T >::Orthonormalized( void ) const GUCEF_MATH_REALTIME_CONTEXT
{
    GUCEF_MATH_STATIC_ASSERT( TTraits::HasSqrt, Orthonormalization_requires_a_numeric_kind_with_sqrt );
    TVector u0;
    if ( !GetColumn( 0 ).TryNormalize( u0, TTraits::Zero() ) )
        u0 = TVector::UnitX();
    const TVector c1 = GetColumn( 1 );
    TVector u1;
    if ( !( c1 - u0 * u0.Dot( c1 ) ).TryNormalize( u1, TTraits::Zero() ) )
        u1 = u0.GetAnyPerpendicular().Normalized();
    const TVector c2 = GetColumn( 2 );
    TVector u2;
    if ( !( c2 - u0 * u0.Dot( c2 ) - u1 * u1.Dot( c2 ) ).TryNormalize( u2, TTraits::Zero() ) )
        u2 = u0.Cross( u1 ).Normalized();
    return FromColumns( u0, u1, u2 );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE CTQuaternion< T >
CTMatrix3D< T >::ToQuaternion( void ) const GUCEF_MATH_REALTIME_CONTEXT
{
    GUCEF_MATH_STATIC_ASSERT( TTraits::HasSqrt, Quaternion_conversion_requires_a_numeric_kind_with_sqrt );
    const CTMatrix3D& m = *this;
    const T one = TTraits::One();
    const T quarter = TTraits::Half() * TTraits::Half();
    const T trace = m( 0, 0 ) + m( 1, 1 ) + m( 2, 2 );
    T x, y, z, w;
    if ( TTraits::Zero() < trace )
    {
        const T s = TTraits::Sqrt( trace + one ) * TTraits::Two();    // 4 w
        w = s * quarter;
        x = ( m( 2, 1 ) - m( 1, 2 ) ) / s;
        y = ( m( 0, 2 ) - m( 2, 0 ) ) / s;
        z = ( m( 1, 0 ) - m( 0, 1 ) ) / s;
    }
    else if ( m( 1, 1 ) < m( 0, 0 ) && m( 2, 2 ) < m( 0, 0 ) )
    {
        const T s = TTraits::Sqrt( one + m( 0, 0 ) - m( 1, 1 ) - m( 2, 2 ) ) * TTraits::Two();    // 4 x
        w = ( m( 2, 1 ) - m( 1, 2 ) ) / s;
        x = s * quarter;
        y = ( m( 0, 1 ) + m( 1, 0 ) ) / s;
        z = ( m( 0, 2 ) + m( 2, 0 ) ) / s;
    }
    else if ( m( 2, 2 ) < m( 1, 1 ) )
    {
        const T s = TTraits::Sqrt( one + m( 1, 1 ) - m( 0, 0 ) - m( 2, 2 ) ) * TTraits::Two();    // 4 y
        w = ( m( 0, 2 ) - m( 2, 0 ) ) / s;
        x = ( m( 0, 1 ) + m( 1, 0 ) ) / s;
        y = s * quarter;
        z = ( m( 1, 2 ) + m( 2, 1 ) ) / s;
    }
    else
    {
        const T s = TTraits::Sqrt( one + m( 2, 2 ) - m( 0, 0 ) - m( 1, 1 ) ) * TTraits::Two();    // 4 z
        w = ( m( 1, 0 ) - m( 0, 1 ) ) / s;
        x = ( m( 0, 2 ) + m( 2, 0 ) ) / s;
        y = ( m( 1, 2 ) + m( 2, 1 ) ) / s;
        z = s * quarter;
    }
    return CTQuaternion< T >::FromXYZW( x, y, z, w ).Normalized();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace MATH */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

/* The related classes used by the quaternion and direction members, included after the class (any header first) */

#ifndef GUCEF_MATH_CTQUATERNION_H
#include "gucefMATH_CTQuaternion.h"
#define GUCEF_MATH_CTQUATERNION_H
#endif /* GUCEF_MATH_CTQUATERNION_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      EXPORTED INSTANTIATIONS                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *  The precision-sensitive members of the predefined kinds, member by member (section 5.10 of the design; see
 *  gucefMATH_CTVector3D.h for the scheme): ALL_KINDS, DIVISION, SQRT and TRANSCENDENTAL groups.
 */

#define GUCEF_MATH_CTMATRIX3D_ALL_KINDS_MEMBERS( prefix, kind ) \
    prefix CTMatrix3D< kind > CTMatrix3D< kind >::Multiply( const CTMatrix3D< kind >& ) const GUCEF_MATH_REALTIME_CONTEXT; \
    prefix CTVector3D< kind > CTMatrix3D< kind >::Transform( const CTVector3D< kind >& ) const GUCEF_MATH_REALTIME_CONTEXT; \
    prefix CTPoint2D< kind > CTMatrix3D< kind >::TransformPoint2D( const CTPoint2D< kind >& ) const GUCEF_MATH_REALTIME_CONTEXT; \
    prefix CTVector2D< kind > CTMatrix3D< kind >::TransformVector2D( const CTVector2D< kind >& ) const GUCEF_MATH_REALTIME_CONTEXT; \
    prefix kind CTMatrix3D< kind >::GetDeterminant( void ) const GUCEF_MATH_REALTIME_CONTEXT;

#define GUCEF_MATH_CTMATRIX3D_DIVISION_MEMBERS( prefix, kind ) \
    prefix bool CTMatrix3D< kind >::TryInverse( CTMatrix3D< kind >&, const kind& ) const GUCEF_MATH_REALTIME_CONTEXT; \
    prefix CTMatrix3D< kind > CTMatrix3D< kind >::Inverse( void ) const GUCEF_MATH_REALTIME_CONTEXT;

#define GUCEF_MATH_CTMATRIX3D_SQRT_MEMBERS( prefix, kind ) \
    prefix CTMatrix3D< kind > CTMatrix3D< kind >::FromQuaternion( const CTQuaternion< kind >& ) GUCEF_MATH_REALTIME_CONTEXT; \
    prefix CTDirectionVector3D< kind > CTMatrix3D< kind >::TransformDirection( const CTDirectionVector3D< kind >& ) const GUCEF_MATH_REALTIME_CONTEXT; \
    prefix CTDirectionVector3D< kind > CTMatrix3D< kind >::TransformNormal( const CTDirectionVector3D< kind >& ) const GUCEF_MATH_REALTIME_CONTEXT; \
    prefix CTMatrix3D< kind > CTMatrix3D< kind >::Orthonormalized( void ) const GUCEF_MATH_REALTIME_CONTEXT; \
    prefix CTQuaternion< kind > CTMatrix3D< kind >::ToQuaternion( void ) const GUCEF_MATH_REALTIME_CONTEXT;

#define GUCEF_MATH_CTMATRIX3D_TRANSCENDENTAL_MEMBERS( prefix, kind ) \
    prefix CTMatrix3D< kind > CTMatrix3D< kind >::RotationX( const CTRadian< kind >& ) GUCEF_MATH_REALTIME_CONTEXT; \
    prefix CTMatrix3D< kind > CTMatrix3D< kind >::RotationY( const CTRadian< kind >& ) GUCEF_MATH_REALTIME_CONTEXT; \
    prefix CTMatrix3D< kind > CTMatrix3D< kind >::RotationZ( const CTRadian< kind >& ) GUCEF_MATH_REALTIME_CONTEXT; \
    prefix CTMatrix3D< kind > CTMatrix3D< kind >::RotationAxisAngle( const CTDirectionVector3D< kind >&, const CTRadian< kind >& ) GUCEF_MATH_REALTIME_CONTEXT; \
    prefix CTMatrix3D< kind > CTMatrix3D< kind >::Rotation2D( const CTRadian< kind >& ) GUCEF_MATH_REALTIME_CONTEXT;

#define GUCEF_MATH_CTMATRIX3D_REAL_KIND( prefix, kind ) GUCEF_MATH_CTMATRIX3D_ALL_KINDS_MEMBERS( prefix, kind ) GUCEF_MATH_CTMATRIX3D_DIVISION_MEMBERS( prefix, kind ) GUCEF_MATH_CTMATRIX3D_SQRT_MEMBERS( prefix, kind ) GUCEF_MATH_CTMATRIX3D_TRANSCENDENTAL_MEMBERS( prefix, kind )
#define GUCEF_MATH_CTMATRIX3D_INTEGER_KIND( prefix, kind ) GUCEF_MATH_CTMATRIX3D_ALL_KINDS_MEMBERS( prefix, kind )
#define GUCEF_MATH_CTMATRIX3D_EXACT_KIND( prefix, kind ) GUCEF_MATH_CTMATRIX3D_ALL_KINDS_MEMBERS( prefix, kind ) GUCEF_MATH_CTMATRIX3D_DIVISION_MEMBERS( prefix, kind )

/** Every predefined kind except the fractions (those are in gucefMATH_fractiontypes.h, which includes CBigInteger) */
#define GUCEF_MATH_CTMATRIX3D_PREDEFINED_KINDS( prefix ) \
    GUCEF_MATH_CTMATRIX3D_REAL_KIND( prefix, Float32 ) \
    GUCEF_MATH_CTMATRIX3D_REAL_KIND( prefix, Float64 ) \
    GUCEF_MATH_CTMATRIX3D_REAL_KIND( prefix, TFixed16_16 ) \
    GUCEF_MATH_CTMATRIX3D_REAL_KIND( prefix, TFixed32_32 ) \
    GUCEF_MATH_CTMATRIX3D_REAL_KIND( prefix, TFixed48_16 ) \
    GUCEF_MATH_CTMATRIX3D_INTEGER_KIND( prefix, Int8 ) \
    GUCEF_MATH_CTMATRIX3D_INTEGER_KIND( prefix, UInt8 ) \
    GUCEF_MATH_CTMATRIX3D_INTEGER_KIND( prefix, Int16 ) \
    GUCEF_MATH_CTMATRIX3D_INTEGER_KIND( prefix, UInt16 ) \
    GUCEF_MATH_CTMATRIX3D_INTEGER_KIND( prefix, Int32 ) \
    GUCEF_MATH_CTMATRIX3D_INTEGER_KIND( prefix, UInt32 ) \
    GUCEF_MATH_CTMATRIX3D_INTEGER_KIND( prefix, Int64 ) \
    GUCEF_MATH_CTMATRIX3D_INTEGER_KIND( prefix, UInt64 )

#define GUCEF_MATH_CTMATRIX3D_FRACTION_KINDS( prefix ) \
    GUCEF_MATH_CTMATRIX3D_EXACT_KIND( prefix, CORE::TInt64Fraction ) \
    GUCEF_MATH_CTMATRIX3D_EXACT_KIND( prefix, TMathBigFraction )

#ifdef GUCEF_MATH_EXTERN_TEMPLATES_ENABLED
namespace GUCEF {
namespace MATH {
GUCEF_MATH_CTMATRIX3D_PREDEFINED_KINDS( GUCEF_MATH_EXTERN_TEMPLATE )
}  /* namespace MATH */
}  /* namespace GUCEF */
#endif /* GUCEF_MATH_EXTERN_TEMPLATES_ENABLED ? */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MATH_CTMATRIX3D_H ? */
