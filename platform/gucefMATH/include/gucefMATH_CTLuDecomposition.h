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

#ifndef GUCEF_MATH_CTLUDECOMPOSITION_H
#define GUCEF_MATH_CTLUDECOMPOSITION_H

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
 *  The arithmetic of the LU elimination: a - b c and a / b, reporting results that do not fit the kind. The operators of
 *  the fixed-point kinds wrap, so their named operations with a status are used (an overflow saturates and is
 *  reported); the other kinds use their operators (floating point overflows to infinity, fractions are exact).
 */
template < typename T, int Kind >
class CTLuArithmetic
{
    public:

    static GUCEF_MATH_FORCE_INLINE T MultiplySubtract( const T& a, const T& b, const T& c, bool& /*overflow*/ ) GUCEF_MATH_REALTIME_CONTEXT { return a - b * c; }
    static GUCEF_MATH_FORCE_INLINE T Divide( const T& a, const T& b, bool& /*overflow*/ ) GUCEF_MATH_REALTIME_CONTEXT { return a / b; }
    static GUCEF_MATH_FORCE_INLINE T SaturatingMultiply( const T& a, const T& b ) GUCEF_MATH_REALTIME_CONTEXT { return a * b; }
};

template < typename TStorage, UInt32 F >
class CTLuArithmetic< CTFixedPoint< TStorage, F >, MATHCOMPONENTKIND_FIXED >
{
    public:

    typedef CTFixedPoint< TStorage, F > T;

    static GUCEF_MATH_FORCE_INLINE T MultiplySubtract( const T& a, const T& b, const T& c, bool& overflow ) GUCEF_MATH_REALTIME_CONTEXT
    {
        T product;
        T result;
        TMathStatus status = T::Multiply( b, c, CORE::ROUNDINGMODE_HALF_UP, product );
        status |= T::Subtract( a, product, CORE::ROUNDINGMODE_HALF_UP, result );
        if ( 0 != ( status & MATHSTATUS_OVERFLOW ) )
            overflow = true;
        return result;
    }

    static GUCEF_MATH_FORCE_INLINE T Divide( const T& a, const T& b, bool& overflow ) GUCEF_MATH_REALTIME_CONTEXT
    {
        T result;
        if ( 0 != ( T::Divide( a, b, CORE::ROUNDINGMODE_HALF_UP, result ) & ( MATHSTATUS_OVERFLOW | MATHSTATUS_DIVISION_BY_ZERO ) ) )
            overflow = true;
        return result;
    }

    static GUCEF_MATH_FORCE_INLINE T SaturatingMultiply( const T& a, const T& b ) GUCEF_MATH_REALTIME_CONTEXT
    {
        T result;
        T::Multiply( a, b, CORE::ROUNDINGMODE_HALF_UP, result );
        return result;
    }
};

/*-------------------------------------------------------------------------*/

/**
 *  LU decomposition with partial pivoting, P A = L U (Doolittle: L unit lower triangular), of a square matrix.
 *
 *  - Kind: T needs division (floating point, fixed point, fractions). With fractions every result is exact.
 *  - Input: a CTDynamicMatrix or a fixed-size CTMatrix< T, N, N > (or column-major values).
 *  - Pivoting: in every column the row with the largest magnitude becomes the pivot row ([[0, 1], [1, 0]] is
 *    decomposed with one swap).
 *  - Singular: Decompose returns false when a pivot magnitude is <= tolerance * max |a_ij| (relative, so independent of
 *    the scale of the matrix). The default tolerance is n * epsilon of the kind (0 for the exact kinds: only an exact zero
 *    pivot is singular). A failed Decompose leaves no valid decomposition (IsValid() false).
 *  - Fixed point: rows whose largest magnitude leaves too little headroom for the elimination (the growth of partial
 *    pivoting, up to 2^( n - 1 )) are scaled down by a power of two, and the right-hand sides of Solve with them, so
 *    large entries do not overflow; the elimination and the substitutions use the named operations of the format
 *    (the operators wrap), and any value that still does not fit makes Decompose or Solve return false (a result is
 *    never silently wrapped or saturated). Scaling a row down rounds its smallest bits. GetDeterminant saturates.
 *  - Memory: the factors are kept in GUCEF::vector members that are reused. Allocations happen only in: Decompose when
 *    the size grows; GetInverse( CTDynamicMatrix ) when 'out' has other dimensions; Solve( TVector ) when x has another
 *    size or is the same object as b. Solve on values, the fixed-size Solve and GetInverse, GetDeterminant and the
 *    TVector Solve into a distinct right-sized x do not allocate (realtime-safe).
 */
template < typename T >
class GUCEF_MATH_TEMPLATE_PUBLIC CTLuDecomposition
{
    public:

    typedef T TValue;
    typedef CTNumericTraits< T > TTraits;
    typedef CTDynamicMatrix< T > TMatrix;
    typedef CTDynamicVector< T > TVector;

    GUCEF_MATH_STATIC_ASSERT( CTNumericTraits< T >::HasDivision, LU_decomposition_requires_a_numeric_kind_with_division );

    CTLuDecomposition( void ) : m_size( 0 ), m_pivotSign( 1 ), m_valid( false ) {}

    /** Decomposes the n x n matrix of column-major values with the given relative tolerance (>= 0); false when singular */
    bool Decompose( const T* columnMajor, UInt32 n, const T& relativeTolerance );

    /** Decomposes with the default tolerance (n * epsilon); false when singular */
    bool Decompose( const T* columnMajor, UInt32 n ) { return Decompose( columnMajor, n, DefaultTolerance( n ) ); }

    /** Decomposes a square dynamic matrix; false when it is not square or singular */
    bool Decompose( const TMatrix& matrix, const T& relativeTolerance )
    {
        if ( !matrix.IsSquare() )
        {
            m_valid = false;
            return false;
        }
        return Decompose( matrix.GetData(), matrix.GetRows(), relativeTolerance );
    }

    bool Decompose( const TMatrix& matrix ) { return Decompose( matrix, DefaultTolerance( matrix.GetRows() ) ); }

    /** Decomposes a fixed-size square matrix */
    template < UInt32 N >
    bool Decompose( const CTMatrix< T, N, N >& matrix, const T& relativeTolerance ) { return Decompose( matrix.GetData(), N, relativeTolerance ); }

    template < UInt32 N >
    bool Decompose( const CTMatrix< T, N, N >& matrix ) { return Decompose( matrix.GetData(), N, DefaultTolerance( N ) ); }

    /** True after a successful Decompose */
    GUCEF_MATH_FORCE_INLINE bool IsValid( void ) const GUCEF_MATH_REALTIME_CONTEXT { return m_valid; }

    /** The dimension of the decomposed matrix */
    GUCEF_MATH_FORCE_INLINE UInt32 GetSize( void ) const GUCEF_MATH_REALTIME_CONTEXT { return m_size; }

    /** +1 or -1: the sign of the row permutation */
    GUCEF_MATH_FORCE_INLINE Int32 GetPivotSign( void ) const GUCEF_MATH_REALTIME_CONTEXT { return m_pivotSign; }

    /** The row of A that became row i of P A; 0 outside the range */
    GUCEF_MATH_FORCE_INLINE UInt32 GetPivot( UInt32 i ) const GUCEF_MATH_REALTIME_CONTEXT { return i < m_size ? m_pivots[ i ] : 0; }

    /**
     *  Solves A x = b for GetSize() values; false without a valid decomposition, and for fixed point when a value reaches
     *  the limit of the format (x is then unspecified). b and x must not overlap.
     */
    bool Solve( const T* b, T* x ) const GUCEF_MATH_REALTIME_CONTEXT;

    /**
     *  Solves A x = b; false (x unchanged) without a valid decomposition or when b has the wrong size; false with x
     *  unspecified after a fixed-point saturation. x may be b (that case allocates a temporary).
     */
    bool Solve( const TVector& b, TVector& x ) const
    {
        if ( !m_valid || b.GetSize() != m_size )
            return false;
        if ( &x == &b )
        {
            TVector result( m_size );
            if ( !Solve( b.GetData(), result.GetData() ) )
                return false;
            x = result;
            return true;
        }
        x.Resize( m_size );
        return Solve( b.GetData(), x.GetData() );
    }

    /** Solves A x = b for a fixed-size right-hand side (realtime-safe) */
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

    /** det A = sign * product of the diagonal of U; zero without a valid decomposition */
    T GetDeterminant( void ) const GUCEF_MATH_REALTIME_CONTEXT;

    /**
     *  A^-1 by solving for the unit vectors; false (out unchanged) without a valid decomposition; false with 'out'
     *  unspecified after a fixed-point saturation. Allocates only when 'out' has other dimensions.
     */
    bool GetInverse( TMatrix& out ) const;

    /** A^-1 for a fixed size (realtime-safe); false (out unchanged) without a valid decomposition of size N */
    template < UInt32 N >
    bool GetInverse( CTMatrix< T, N, N >& out ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        if ( !m_valid || N != m_size )
            return false;
        T inverse[ N * N ];
        T unit[ N ];
        for ( UInt32 c=0; c<N; ++c )
        {
            for ( UInt32 i=0; i<N; ++i )
                unit[ i ] = i == c ? TTraits::One() : TTraits::Zero();
            if ( !Solve( unit, inverse + c * N ) )
                return false;
        }
        out = CTMatrix< T, N, N >::FromColumnMajor( inverse );
        return true;
    }

    /** The default relative tolerance for an n x n matrix: n * epsilon of the kind */
    static T DefaultTolerance( UInt32 n ) GUCEF_MATH_REALTIME_CONTEXT
    {
        return TTraits::FromInt32( (Int32) ( n > 0 ? n : 1 ) ) * TTraits::Epsilon();
    }

    private:

    typedef CTMathBinaryScaling< T, CTMathComponentKind< T >::Value > TScaling;

    enum { IsFixedPoint = (int) MATHCOMPONENTKIND_FIXED == (int) CTMathComponentKind< T >::Value };

    typedef CTLuArithmetic< T, CTMathComponentKind< T >::Value > TArithmetic;

    /** Solve for the right-hand side b, or for the unit vector e_unit when b is null */
    bool SolveImpl( const T* b, UInt32 unit, T* x ) const GUCEF_MATH_REALTIME_CONTEXT;

    GUCEF::vector< T > m_lu;            /**< L (strictly below the diagonal) and U, column-major */
    GUCEF::vector< UInt32 > m_pivots;
    GUCEF::vector< Int32 > m_rowExponent;   /**< fixed point: the power of two each row of A was scaled by (<= 0) */
    UInt32 m_size;
    Int32 m_pivotSign;
    bool m_valid;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE bool
CTLuDecomposition< T >::Decompose( const T* columnMajor, UInt32 n, const T& relativeTolerance )
{
    m_valid = false;
    m_size = n;
    m_pivotSign = 1;
    m_lu.assign( columnMajor, columnMajor + (size_t) n * n );
    m_pivots.resize( n );
    m_rowExponent.assign( n, 0 );
    if ( 0 == n )
        return false;
    T* lu = &m_lu[ 0 ];

    for ( size_t i=0; i<(size_t) n * n; ++i )
        if ( !TTraits::IsFinite( lu[ i ] ) )
            return false;
    if ( IsFixedPoint )
    {
        // rows scaled down by powers of two until their largest magnitude is at most 2^( E - n ), E the exponent of the
        // largest value of the format: the elimination growth of partial pivoting (2^( n - 1 )) then fits
        int formatExponent = 0;
        frexp( TTraits::ToFloat64( TTraits::Max() ), &formatExponent );
        for ( UInt32 r=0; r<n; ++r )
        {
            T rowLargest = TTraits::Zero();
            for ( UInt32 c=0; c<n; ++c )
                rowLargest = TTraits::Max( rowLargest, TTraits::Abs( lu[ (size_t) c * n + r ] ) );
            if ( !( TTraits::Zero() < rowLargest ) )
                continue;
            // rowLargest 2^k in [ 0.5, 1 ) for k = NormalizingExponent: k + E - n brings it to [ 2^( E - n - 1 ), 2^( E - n ) )
            Int32 exponent = TScaling::NormalizingExponent( rowLargest ) + (Int32) formatExponent - (Int32) n;
            if ( exponent >= 0 )
                continue;
            m_rowExponent[ r ] = exponent;
            for ( UInt32 c=0; c<n; ++c )
                lu[ (size_t) c * n + r ] = TScaling::Scale( lu[ (size_t) c * n + r ], exponent );
        }
    }

    T largest = TTraits::Zero();
    for ( size_t i=0; i<(size_t) n * n; ++i )
        largest = TTraits::Max( largest, TTraits::Abs( lu[ i ] ) );
    const T threshold = relativeTolerance * largest;
    for ( UInt32 i=0; i<n; ++i )
        m_pivots[ i ] = i;

    for ( UInt32 k=0; k<n; ++k )
    {
        // the pivot: the largest magnitude in column k at or below the diagonal
        UInt32 pivot = k;
        T pivotMagnitude = TTraits::Abs( lu[ k * n + k ] );
        for ( UInt32 r=k+1; r<n; ++r )
        {
            const T magnitude = TTraits::Abs( lu[ k * n + r ] );
            if ( pivotMagnitude < magnitude )
            {
                pivot = r;
                pivotMagnitude = magnitude;
            }
        }
        if ( !( threshold < pivotMagnitude ) )
            return false;
        if ( pivot != k )
        {
            for ( UInt32 c=0; c<n; ++c )
            {
                const T swap = lu[ c * n + k ];
                lu[ c * n + k ] = lu[ c * n + pivot ];
                lu[ c * n + pivot ] = swap;
            }
            const UInt32 swapIndex = m_pivots[ k ];
            m_pivots[ k ] = m_pivots[ pivot ];
            m_pivots[ pivot ] = swapIndex;
            m_pivotSign = -m_pivotSign;
        }
        const T diagonal = lu[ k * n + k ];
        bool overflow = false;
        for ( UInt32 r=k+1; r<n; ++r )
            lu[ k * n + r ] = TArithmetic::Divide( lu[ k * n + r ], diagonal, overflow );
        // the trailing block: a_rc -= l_rk u_kc
        for ( UInt32 c=k+1; c<n; ++c )
        {
            const T u = lu[ c * n + k ];
            for ( UInt32 r=k+1; r<n; ++r )
                lu[ c * n + r ] = TArithmetic::MultiplySubtract( lu[ c * n + r ], lu[ k * n + r ], u, overflow );
        }
        if ( overflow )
            return false;   // fixed point: a value beyond the format
    }
    m_valid = true;
    return true;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE bool
CTLuDecomposition< T >::Solve( const T* b, T* x ) const GUCEF_MATH_REALTIME_CONTEXT
{
    return SolveImpl( b, 0, x );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE bool
CTLuDecomposition< T >::SolveImpl( const T* b, UInt32 unit, T* x ) const GUCEF_MATH_REALTIME_CONTEXT
{
    if ( !m_valid )
        return false;
    const UInt32 n = m_size;
    const T* lu = &m_lu[ 0 ];
    bool overflow = false;
    // forward substitution with the permuted (and for fixed point row-scaled) right-hand side: L y = P D b
    for ( UInt32 i=0; i<n; ++i )
    {
        const UInt32 row = m_pivots[ i ];
        const T value = GUCEF_NULL != b ? b[ row ] : ( row == unit ? TTraits::One() : TTraits::Zero() );
        T sum = IsFixedPoint ? TScaling::Scale( value, m_rowExponent[ row ] ) : value;
        for ( UInt32 j=0; j<i; ++j )
            sum = TArithmetic::MultiplySubtract( sum, lu[ j * n + i ], x[ j ], overflow );
        x[ i ] = sum;
    }
    // back substitution: U x = y
    for ( UInt32 i=n; i-- > 0; )
    {
        T sum = x[ i ];
        for ( UInt32 j=i+1; j<n; ++j )
            sum = TArithmetic::MultiplySubtract( sum, lu[ j * n + i ], x[ j ], overflow );
        x[ i ] = TArithmetic::Divide( sum, lu[ i * n + i ], overflow );
    }
    return !overflow;   // fixed point: a value beyond the format
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE T
CTLuDecomposition< T >::GetDeterminant( void ) const GUCEF_MATH_REALTIME_CONTEXT
{
    if ( !m_valid )
        return TTraits::Zero();
    T determinant = m_pivotSign < 0 ? TTraits::Zero() - TTraits::One() : TTraits::One();
    Int32 exponent = 0;
    for ( UInt32 i=0; i<m_size; ++i )
    {
        determinant = TArithmetic::SaturatingMultiply( determinant, m_lu[ (size_t) i * m_size + i ] );
        exponent -= m_rowExponent[ i ];
    }
    // det( D A ) = det( A ) 2^( sum of the row exponents )
    return 0 == exponent ? determinant : TScaling::Scale( determinant, exponent );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE bool
CTLuDecomposition< T >::GetInverse( TMatrix& out ) const
{
    if ( !m_valid )
        return false;
    out.Resize( m_size, m_size );
    for ( UInt32 c=0; c<m_size; ++c )
    {
        T* column = out.GetData() + (size_t) c * m_size;
        if ( !SolveImpl( GUCEF_NULL, c, column ) )
            return false;
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

#define GUCEF_MATH_CTLUDECOMPOSITION_MEMBERS( prefix, kind ) \
    prefix bool CTLuDecomposition< kind >::Decompose( const kind*, UInt32, const kind& ); \
    prefix bool CTLuDecomposition< kind >::Solve( const kind*, kind* ) const GUCEF_MATH_REALTIME_CONTEXT; \
    prefix bool CTLuDecomposition< kind >::SolveImpl( const kind*, UInt32, kind* ) const GUCEF_MATH_REALTIME_CONTEXT; \
    prefix kind CTLuDecomposition< kind >::GetDeterminant( void ) const GUCEF_MATH_REALTIME_CONTEXT; \
    prefix bool CTLuDecomposition< kind >::GetInverse( CTDynamicMatrix< kind >& ) const;

#define GUCEF_MATH_CTLUDECOMPOSITION_PREDEFINED_KINDS( prefix ) \
    GUCEF_MATH_CTLUDECOMPOSITION_MEMBERS( prefix, Float32 ) \
    GUCEF_MATH_CTLUDECOMPOSITION_MEMBERS( prefix, Float64 ) \
    GUCEF_MATH_CTLUDECOMPOSITION_MEMBERS( prefix, TFixed16_16 ) \
    GUCEF_MATH_CTLUDECOMPOSITION_MEMBERS( prefix, TFixed32_32 ) \
    GUCEF_MATH_CTLUDECOMPOSITION_MEMBERS( prefix, TFixed48_16 )

#ifdef GUCEF_MATH_EXTERN_TEMPLATES_ENABLED
namespace GUCEF {
namespace MATH {
GUCEF_MATH_CTLUDECOMPOSITION_PREDEFINED_KINDS( GUCEF_MATH_EXTERN_TEMPLATE )
}  /* namespace MATH */
}  /* namespace GUCEF */
#endif /* GUCEF_MATH_EXTERN_TEMPLATES_ENABLED ? */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MATH_CTLUDECOMPOSITION_H ? */
