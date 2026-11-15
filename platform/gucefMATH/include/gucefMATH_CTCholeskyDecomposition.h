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

#ifndef GUCEF_MATH_CTCHOLESKYDECOMPOSITION_H
#define GUCEF_MATH_CTCHOLESKYDECOMPOSITION_H

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
 *  Cholesky decomposition of a symmetric matrix: A = L L^T for positive definite matrices (Decompose) and
 *  A = L D L^T with unit lower triangular L for positive semidefinite matrices (DecomposeLdlt).
 *
 *  - Kind: T is Float32 or Float64.
 *  - Input: a CTDynamicMatrix or a fixed-size CTMatrix< T, N, N > (or column-major values). Only the lower triangle is
 *    read after the symmetry check: a matrix that is not symmetric within n * epsilon relative to its largest element
 *    is rejected.
 *  - Decompose returns false for a matrix that is not positive definite (a pivot <= n * epsilon * max |a_ij|, so a
 *    singular matrix is rejected also when rounding leaves a tiny positive pivot); DecomposeLdlt returns false
 *    for a negative pivot (beyond n * epsilon * max |a_ij|), and accepts zero pivots when the rest of their column is
 *    zero as well (semidefinite). Solve then fails for a zero pivot (singular).
 *  - Memory: GUCEF::vector members reused between calls. Allocations happen only in: Decompose when the size grows,
 *    GetL / GetD (they fill new objects) and Solve( TVector ) when x has another size. Solve on values, the fixed-size
 *    Solve and Solve( TVector ) into a right-sized x (or in place) do not allocate (realtime-safe).
 */
template < typename T >
class GUCEF_MATH_TEMPLATE_PUBLIC CTCholeskyDecomposition
{
    public:

    typedef T TValue;
    typedef CTNumericTraits< T > TTraits;
    typedef CTDynamicMatrix< T > TMatrix;
    typedef CTDynamicVector< T > TVector;

    GUCEF_MATH_STATIC_ASSERT( CTNumericTraits< T >::IsFloatingPoint, Cholesky_decomposition_requires_a_floating_point_kind );

    CTCholeskyDecomposition( void ) : m_size( 0 ), m_valid( false ), m_ldlt( false ) {}

    /** A = L L^T; false when the n x n matrix is not symmetric or not positive definite */
    bool Decompose( const T* columnMajor, UInt32 n ) { return DecomposeImpl( columnMajor, n, false ); }

    bool Decompose( const TMatrix& matrix ) { return matrix.IsSquare() ? Decompose( matrix.GetData(), matrix.GetRows() ) : Fail(); }

    template < UInt32 N >
    bool Decompose( const CTMatrix< T, N, N >& matrix ) { return Decompose( matrix.GetData(), N ); }

    /** A = L D L^T; false when the n x n matrix is not symmetric or not positive semidefinite */
    bool DecomposeLdlt( const T* columnMajor, UInt32 n ) { return DecomposeImpl( columnMajor, n, true ); }

    bool DecomposeLdlt( const TMatrix& matrix ) { return matrix.IsSquare() ? DecomposeLdlt( matrix.GetData(), matrix.GetRows() ) : Fail(); }

    template < UInt32 N >
    bool DecomposeLdlt( const CTMatrix< T, N, N >& matrix ) { return DecomposeLdlt( matrix.GetData(), N ); }

    GUCEF_MATH_FORCE_INLINE bool IsValid( void ) const GUCEF_MATH_REALTIME_CONTEXT { return m_valid; }

    /** True when the valid decomposition is L D L^T */
    GUCEF_MATH_FORCE_INLINE bool IsLdlt( void ) const GUCEF_MATH_REALTIME_CONTEXT { return m_valid && m_ldlt; }

    GUCEF_MATH_FORCE_INLINE UInt32 GetSize( void ) const GUCEF_MATH_REALTIME_CONTEXT { return m_size; }

    /** The lower triangular factor (unit diagonal for L D L^T); false (out unchanged) without a valid decomposition */
    bool GetL( TMatrix& out ) const
    {
        if ( !m_valid )
            return false;
        TMatrix l = TMatrix::FromColumnMajor( &m_l[ 0 ], m_size, m_size );
        out.Swap( l );
        return true;
    }

    /** The diagonal D of L D L^T (all ones for L L^T); false (out unchanged) without a valid decomposition */
    bool GetD( TVector& out ) const
    {
        if ( !m_valid )
            return false;
        out = TVector( &m_d[ 0 ], m_size );
        return true;
    }

    /** Solves A x = b for GetSize() values; false without a valid decomposition or for a zero pivot. b and x may be equal. */
    bool Solve( const T* b, T* x ) const GUCEF_MATH_REALTIME_CONTEXT;

    /** Solves A x = b; false (x unchanged) without a valid decomposition, for a b of the wrong size or a zero pivot. x may be b. */
    bool Solve( const TVector& b, TVector& x ) const
    {
        if ( !m_valid || b.GetSize() != m_size || HasZeroPivot() )
            return false;
        if ( &x != &b )
        {
            x.Resize( m_size );
            for ( UInt32 i=0; i<m_size; ++i )
                x[ i ] = b[ i ];
        }
        return Solve( x.GetData(), x.GetData() );
    }

    template < UInt32 N >
    bool Solve( const CTMatrix< T, N, 1 >& b, CTMatrix< T, N, 1 >& x ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        if ( !m_valid || N != m_size )
            return false;
        T result[ N ];
        if ( !Solve( b.GetData(), result ) )
            return false;
        x = CTMatrix< T, N, 1 >::FromColumnMajor( result );
        return true;
    }

    private:

    bool Fail( void ) { m_valid = false; return false; }

    /** L D L^T with a zero pivot: singular, Solve fails */
    bool HasZeroPivot( void ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        if ( !m_ldlt )
            return false;
        for ( UInt32 i=0; i<m_size; ++i )
            if ( TTraits::Zero() == m_d[ i ] )
                return true;
        return false;
    }

    bool DecomposeImpl( const T* columnMajor, UInt32 n, bool ldlt );

    GUCEF::vector< T > m_l;     /**< column-major n x n, zero above the diagonal */
    GUCEF::vector< T > m_d;
    UInt32 m_size;
    bool m_valid;
    bool m_ldlt;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE bool
CTCholeskyDecomposition< T >::DecomposeImpl( const T* a, UInt32 n, bool ldlt )
{
    m_valid = false;
    m_ldlt = ldlt;
    m_size = n;
    if ( 0 == n )
        return false;
    T largest = TTraits::Zero();
    for ( size_t i=0; i<(size_t) n * n; ++i )
    {
        if ( !TTraits::IsFinite( a[ i ] ) )
            return false;
        largest = TTraits::Max( largest, TTraits::Abs( a[ i ] ) );
    }
    const T tolerance = TTraits::FromInt32( (Int32) n ) * TTraits::Epsilon() * largest;
    for ( UInt32 c=0; c<n; ++c )
        for ( UInt32 r=c+1; r<n; ++r )
            if ( tolerance < TTraits::Abs( a[ (size_t) c * n + r ] - a[ (size_t) r * n + c ] ) )
                return false;

    m_l.assign( (size_t) n * n, TTraits::Zero() );
    m_d.assign( n, TTraits::One() );
    T* l = &m_l[ 0 ];
    T* d = &m_d[ 0 ];
    for ( UInt32 j=0; j<n; ++j )
    {
        if ( !ldlt )
        {
            // l_jj = sqrt( a_jj - sum_k l_jk^2 ), l_ij = ( a_ij - sum_k l_ik l_jk ) / l_jj
            const T pivot = a[ (size_t) j * n + j ] - CTMatrixKernels< T >::Dot( l + j, n, l + j, n, j );
            if ( !( tolerance < pivot ) )
                return false;   // not positive definite (also a pivot at the rounding level of a singular matrix)
            const T diagonal = TTraits::Sqrt( pivot );
            l[ (size_t) j * n + j ] = diagonal;
            for ( UInt32 i=j+1; i<n; ++i )
                l[ (size_t) j * n + i ] = ( a[ (size_t) j * n + i ] - CTMatrixKernels< T >::Dot( l + i, n, l + j, n, j ) ) / diagonal;
        }
        else
        {
            // d_j = a_jj - sum_k l_jk^2 d_k, l_ij = ( a_ij - sum_k l_ik l_jk d_k ) / d_j
            T pivot = a[ (size_t) j * n + j ];
            for ( UInt32 k=0; k<j; ++k )
                pivot = pivot - l[ (size_t) k * n + j ] * l[ (size_t) k * n + j ] * d[ k ];
            l[ (size_t) j * n + j ] = TTraits::One();
            if ( pivot < -tolerance )
                return false;
            if ( !( tolerance < pivot ) )
            {
                // a zero pivot: semidefinite only when the rest of the column vanishes as well
                d[ j ] = TTraits::Zero();
                for ( UInt32 i=j+1; i<n; ++i )
                {
                    T value = a[ (size_t) j * n + i ];
                    for ( UInt32 k=0; k<j; ++k )
                        value = value - l[ (size_t) k * n + i ] * l[ (size_t) k * n + j ] * d[ k ];
                    if ( tolerance < TTraits::Abs( value ) )
                        return false;
                }
                continue;
            }
            d[ j ] = pivot;
            for ( UInt32 i=j+1; i<n; ++i )
            {
                T value = a[ (size_t) j * n + i ];
                for ( UInt32 k=0; k<j; ++k )
                    value = value - l[ (size_t) k * n + i ] * l[ (size_t) k * n + j ] * d[ k ];
                l[ (size_t) j * n + i ] = value / pivot;
            }
        }
    }
    m_valid = true;
    return true;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE bool
CTCholeskyDecomposition< T >::Solve( const T* b, T* x ) const GUCEF_MATH_REALTIME_CONTEXT
{
    if ( !m_valid )
        return false;
    const UInt32 n = m_size;
    const T* l = &m_l[ 0 ];
    const T* d = &m_d[ 0 ];
    if ( m_ldlt )
        for ( UInt32 i=0; i<n; ++i )
            if ( TTraits::Zero() == d[ i ] )
                return false;
    // L y = b
    for ( UInt32 i=0; i<n; ++i )
    {
        T sum = b[ i ];
        for ( UInt32 k=0; k<i; ++k )
            sum = sum - l[ (size_t) k * n + i ] * x[ k ];
        x[ i ] = sum / l[ (size_t) i * n + i ];
    }
    // D z = y
    if ( m_ldlt )
        for ( UInt32 i=0; i<n; ++i )
            x[ i ] = x[ i ] / d[ i ];
    // L^T x = z
    for ( UInt32 i=n; i-- > 0; )
    {
        T sum = x[ i ];
        for ( UInt32 k=i+1; k<n; ++k )
            sum = sum - l[ (size_t) i * n + k ] * x[ k ];
        x[ i ] = sum / l[ (size_t) i * n + i ];
    }
    return true;
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

#define GUCEF_MATH_CTCHOLESKYDECOMPOSITION_MEMBERS( prefix, kind ) \
    prefix bool CTCholeskyDecomposition< kind >::DecomposeImpl( const kind*, UInt32, bool ); \
    prefix bool CTCholeskyDecomposition< kind >::Solve( const kind*, kind* ) const GUCEF_MATH_REALTIME_CONTEXT;

#define GUCEF_MATH_CTCHOLESKYDECOMPOSITION_PREDEFINED_KINDS( prefix ) \
    GUCEF_MATH_CTCHOLESKYDECOMPOSITION_MEMBERS( prefix, Float32 ) \
    GUCEF_MATH_CTCHOLESKYDECOMPOSITION_MEMBERS( prefix, Float64 )

#ifdef GUCEF_MATH_EXTERN_TEMPLATES_ENABLED
namespace GUCEF {
namespace MATH {
GUCEF_MATH_CTCHOLESKYDECOMPOSITION_PREDEFINED_KINDS( GUCEF_MATH_EXTERN_TEMPLATE )
}  /* namespace MATH */
}  /* namespace GUCEF */
#endif /* GUCEF_MATH_EXTERN_TEMPLATES_ENABLED ? */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MATH_CTCHOLESKYDECOMPOSITION_H ? */
