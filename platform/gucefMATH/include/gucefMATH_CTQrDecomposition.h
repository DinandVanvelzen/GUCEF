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

#ifndef GUCEF_MATH_CTQRDECOMPOSITION_H
#define GUCEF_MATH_CTQRDECOMPOSITION_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MATH_CTDYNAMICMATRIX_H
#include "gucefMATH_CTDynamicMatrix.h"
#define GUCEF_MATH_CTDYNAMICMATRIX_H
#endif /* GUCEF_MATH_CTDYNAMICMATRIX_H ? */

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
 *  QR decomposition by Householder reflections, A = Q R, of an m x n matrix with m >= n: Q is m x m orthogonal, R is
 *  m x n upper triangular. The least-squares solution of an overdetermined system follows without forming Q.
 *
 *  - Kind: T is Float32 or Float64 (the reflections need square roots and are only accurate in floating point).
 *  - Input: a CTDynamicMatrix or a fixed-size CTMatrix< T, M, N > (or column-major values).
 *  - Norms are computed with scaling (no overflow or underflow for representable inputs).
 *  - Rank: GetRank( relativeTolerance ) counts the diagonal elements of R with |r_kk| > tolerance * max |r_ii| (an
 *    estimate: there is no column pivoting). SolveLeastSquares fails for a rank-deficient matrix (a zero diagonal
 *    element of R within n * epsilon relative).
 *  - Memory: the reflections and a work vector are GUCEF::vector members that are reused (one thread at a time per
 *    object). Allocations happen only in: Decompose when the size grows, GetQ / GetR when 'out' is filled anew, and
 *    SolveLeastSquares( TVector ) when x has another size. The fixed-size SolveLeastSquares does not allocate.
 */
template < typename T >
class GUCEF_MATH_TEMPLATE_PUBLIC CTQrDecomposition
{
    public:

    typedef T TValue;
    typedef CTNumericTraits< T > TTraits;
    typedef CTDynamicMatrix< T > TMatrix;
    typedef CTDynamicVector< T > TVector;

    GUCEF_MATH_STATIC_ASSERT( CTNumericTraits< T >::IsFloatingPoint, QR_decomposition_requires_a_floating_point_kind );

    CTQrDecomposition( void ) : m_rows( 0 ), m_columns( 0 ), m_valid( false ) {}

    /** Decomposes the rows x columns matrix of column-major values; false when rows < columns or a value is not finite */
    bool Decompose( const T* columnMajor, UInt32 rows, UInt32 columns );

    bool Decompose( const TMatrix& matrix ) { return Decompose( matrix.GetData(), matrix.GetRows(), matrix.GetColumns() ); }

    template < UInt32 M, UInt32 N >
    bool Decompose( const CTMatrix< T, M, N >& matrix ) { return Decompose( matrix.GetData(), M, N ); }

    GUCEF_MATH_FORCE_INLINE bool IsValid( void ) const GUCEF_MATH_REALTIME_CONTEXT { return m_valid; }

    GUCEF_MATH_FORCE_INLINE UInt32 GetRows( void ) const GUCEF_MATH_REALTIME_CONTEXT { return m_rows; }

    GUCEF_MATH_FORCE_INLINE UInt32 GetColumns( void ) const GUCEF_MATH_REALTIME_CONTEXT { return m_columns; }

    /** The m x m orthogonal factor; false (out unchanged) without a valid decomposition */
    bool GetQ( TMatrix& out ) const;

    /** The m x n upper triangular factor; false (out unchanged) without a valid decomposition */
    bool GetR( TMatrix& out ) const;

    /** The number of diagonal elements of R above relativeTolerance * max |r_ii|; 0 without a valid decomposition */
    UInt32 GetRank( const T& relativeTolerance ) const;

    /** GetRank with n * epsilon */
    UInt32 GetRank( void ) const { return GetRank( DefaultTolerance() ); }

    /** True when GetRank() == n */
    bool IsFullRank( void ) const { return m_valid && GetRank() == m_columns; }

    /**
     *  The x that minimises |A x - b| (the exact solution of a consistent system); false (x unchanged) without a valid
     *  decomposition, for a b of the wrong size or a rank-deficient A. Uses the work buffer of this object (one thread
     *  at a time); allocates only when x has another size. x may be b.
     */
    bool SolveLeastSquares( const TVector& b, TVector& x );

    /** SolveLeastSquares for fixed-size vectors (allocation-free) */
    template < UInt32 M, UInt32 N >
    bool SolveLeastSquares( const CTMatrix< T, M, 1 >& b, CTMatrix< T, N, 1 >& x )
    {
        if ( !CanSolve( M, N ) )
            return false;
        for ( UInt32 i=0; i<M; ++i )
            m_work[ i ] = b( i, 0 );
        SolveInWork();
        x = CTMatrix< T, N, 1 >::FromColumnMajor( &m_work[ 0 ] );
        return true;
    }

    private:

    T DefaultTolerance( void ) const { return TTraits::FromInt32( (Int32) ( m_columns > 0 ? m_columns : 1 ) ) * TTraits::Epsilon(); }

    /** A valid, full-rank decomposition of a rows x columns matrix */
    bool CanSolve( UInt32 rows, UInt32 columns ) const { return m_valid && rows == m_rows && columns == m_columns && GetRank() == m_columns; }

    /** Overwrites the right-hand side in m_work with Q^T b, then its first n values with the solution of R x = Q^T b */
    void SolveInWork( void );

    GUCEF::vector< T > m_work;          /**< m values: the right-hand side and solution of SolveLeastSquares */

    GUCEF::vector< T > m_qr;            /**< the Householder vectors on and below the diagonal, R above it */
    GUCEF::vector< T > m_rDiagonal;     /**< the diagonal of R */
    UInt32 m_rows;
    UInt32 m_columns;
    bool m_valid;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE bool
CTQrDecomposition< T >::Decompose( const T* columnMajor, UInt32 rows, UInt32 columns )
{
    m_valid = false;
    m_rows = rows;
    m_columns = columns;
    if ( rows < columns || 0 == columns )
        return false;
    m_qr.assign( columnMajor, columnMajor + (size_t) rows * columns );
    m_rDiagonal.assign( columns, TTraits::Zero() );
    m_work.assign( rows, TTraits::Zero() );
    for ( size_t i=0; i<m_qr.size(); ++i )
        if ( !TTraits::IsFinite( m_qr[ i ] ) )
            return false;
    T* qr = &m_qr[ 0 ];
    const UInt32 m = rows;

    for ( UInt32 k=0; k<columns; ++k )
    {
        T* column = qr + (size_t) k * m;
        // the norm of column k from the diagonal down, scaled by its largest magnitude
        T largest = TTraits::Zero();
        for ( UInt32 i=k; i<m; ++i )
            largest = TTraits::Max( largest, TTraits::Abs( column[ i ] ) );
        T norm = TTraits::Zero();
        if ( TTraits::Zero() < largest )
        {
            T sum = TTraits::Zero();
            for ( UInt32 i=k; i<m; ++i )
            {
                const T scaled = column[ i ] / largest;
                sum = sum + scaled * scaled;
            }
            norm = largest * TTraits::Sqrt( sum );
        }
        if ( !( TTraits::Zero() == norm ) )
        {
            if ( column[ k ] < TTraits::Zero() )
                norm = -norm;
            for ( UInt32 i=k; i<m; ++i )
                column[ i ] = column[ i ] / norm;
            column[ k ] = column[ k ] + TTraits::One();
            // apply the reflection to the remaining columns
            for ( UInt32 j=k+1; j<columns; ++j )
            {
                T* other = qr + (size_t) j * m;
                T s = CTMatrixKernels< T >::Dot( column + k, 1, other + k, 1, m - k );
                s = -s / column[ k ];
                for ( UInt32 i=k; i<m; ++i )
                    other[ i ] = other[ i ] + s * column[ i ];
            }
        }
        m_rDiagonal[ k ] = -norm;
    }
    m_valid = true;
    return true;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE bool
CTQrDecomposition< T >::GetQ( TMatrix& out ) const
{
    if ( !m_valid )
        return false;
    const UInt32 m = m_rows;
    TMatrix q = TMatrix::Identity( m );
    const T* qr = &m_qr[ 0 ];
    // Q = H_0 H_1 ... H_(n-1): apply the reflections to the identity, the last one first
    for ( UInt32 k=m_columns; k-- > 0; )
    {
        const T* column = qr + (size_t) k * m;
        if ( TTraits::Zero() == column[ k ] )
            continue;
        for ( UInt32 j=0; j<m; ++j )
        {
            T* target = q.GetData() + (size_t) j * m;
            T s = CTMatrixKernels< T >::Dot( column + k, 1, target + k, 1, m - k );
            s = -s / column[ k ];
            for ( UInt32 i=k; i<m; ++i )
                target[ i ] = target[ i ] + s * column[ i ];
        }
    }
    out.Swap( q );
    return true;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE bool
CTQrDecomposition< T >::GetR( TMatrix& out ) const
{
    if ( !m_valid )
        return false;
    TMatrix r( m_rows, m_columns );
    for ( UInt32 c=0; c<m_columns; ++c )
    {
        for ( UInt32 i=0; i<c; ++i )
            r( i, c ) = m_qr[ (size_t) c * m_rows + i ];
        r( c, c ) = m_rDiagonal[ c ];
    }
    out.Swap( r );
    return true;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE UInt32
CTQrDecomposition< T >::GetRank( const T& relativeTolerance ) const
{
    if ( !m_valid )
        return 0;
    T largest = TTraits::Zero();
    for ( UInt32 k=0; k<m_columns; ++k )
        largest = TTraits::Max( largest, TTraits::Abs( m_rDiagonal[ k ] ) );
    UInt32 rank = 0;
    for ( UInt32 k=0; k<m_columns; ++k )
        if ( relativeTolerance * largest < TTraits::Abs( m_rDiagonal[ k ] ) )
            ++rank;
    return rank;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE bool
CTQrDecomposition< T >::SolveLeastSquares( const TVector& b, TVector& x )
{
    if ( !CanSolve( b.GetSize(), m_columns ) )
        return false;
    for ( UInt32 i=0; i<m_rows; ++i )
        m_work[ i ] = b[ i ];
    SolveInWork();
    x.Resize( m_columns );
    for ( UInt32 k=0; k<m_columns; ++k )
        x[ k ] = m_work[ k ];
    return true;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE void
CTQrDecomposition< T >::SolveInWork( void )
{
    const UInt32 m = m_rows;
    const UInt32 n = m_columns;
    T* y = &m_work[ 0 ];
    const T* qr = &m_qr[ 0 ];
    // y = Q^T b
    for ( UInt32 k=0; k<n; ++k )
    {
        const T* column = qr + (size_t) k * m;
        if ( TTraits::Zero() == column[ k ] )
            continue;
        T s = CTMatrixKernels< T >::Dot( column + k, 1, y + k, 1, m - k );
        s = -s / column[ k ];
        for ( UInt32 i=k; i<m; ++i )
            y[ i ] = y[ i ] + s * column[ i ];
    }
    // R x = y in place (the first n rows): y[ j ] for j > k already holds x_j
    for ( UInt32 k=n; k-- > 0; )
    {
        T sum = y[ k ];
        for ( UInt32 j=k+1; j<n; ++j )
            sum = sum - qr[ (size_t) j * m + k ] * y[ j ];
        y[ k ] = sum / m_rDiagonal[ k ];
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace MATH */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------//
//                                                                         //
//      EXPORTED INSTANTIATIONS                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

#define GUCEF_MATH_CTQRDECOMPOSITION_MEMBERS( prefix, kind ) \
    prefix bool CTQrDecomposition< kind >::Decompose( const kind*, UInt32, UInt32 ); \
    prefix bool CTQrDecomposition< kind >::GetQ( CTDynamicMatrix< kind >& ) const; \
    prefix bool CTQrDecomposition< kind >::GetR( CTDynamicMatrix< kind >& ) const; \
    prefix UInt32 CTQrDecomposition< kind >::GetRank( const kind& ) const; \
    prefix bool CTQrDecomposition< kind >::SolveLeastSquares( const CTDynamicVector< kind >&, CTDynamicVector< kind >& ); \
    prefix void CTQrDecomposition< kind >::SolveInWork( void );

#define GUCEF_MATH_CTQRDECOMPOSITION_PREDEFINED_KINDS( prefix ) \
    GUCEF_MATH_CTQRDECOMPOSITION_MEMBERS( prefix, Float32 ) \
    GUCEF_MATH_CTQRDECOMPOSITION_MEMBERS( prefix, Float64 )

#ifdef GUCEF_MATH_EXTERN_TEMPLATES_ENABLED
namespace GUCEF {
namespace MATH {
GUCEF_MATH_CTQRDECOMPOSITION_PREDEFINED_KINDS( GUCEF_MATH_EXTERN_TEMPLATE )
}  /* namespace MATH */
}  /* namespace GUCEF */
#endif /* GUCEF_MATH_EXTERN_TEMPLATES_ENABLED ? */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MATH_CTQRDECOMPOSITION_H ? */
