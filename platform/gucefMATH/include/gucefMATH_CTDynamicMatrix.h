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

#ifndef GUCEF_MATH_CTDYNAMICMATRIX_H
#define GUCEF_MATH_CTDYNAMICMATRIX_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MATH_CTDYNAMICVECTOR_H
#include "gucefMATH_CTDynamicVector.h"
#define GUCEF_MATH_CTDYNAMICVECTOR_H
#endif /* GUCEF_MATH_CTDYNAMICVECTOR_H ? */

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
 *  A matrix of run-time size: the general input of the decompositions and least-squares fits.
 *
 *  - Kind: T is any numeric kind.
 *  - Storage: column-major in a GUCEF::vector< T > (GUCEF allocator), element (r, c) at c * rows + r. Allocations happen
 *    only in: the sizing constructors, FromColumnMajor / FromRowMajor / Identity, Resize to other dimensions, GetBlock,
 *    GetRow, GetColumn and Transposed (they return or fill a new object), and the Try* operations when the output has
 *    other dimensions or, for the products, when the output is one of the operands. Element access, SetBlock, Scale,
 *    SetZero, SetIdentity and the Try* operations into a distinct output of the right dimensions do not allocate.
 *  - Operations check the dimensions and return false (output unchanged) when they do not fit.
 *  - Products: every element one CTMatrixKernels dot product of the kind (fixed point rounds once per element).
 *  - Text "[[m00, m01, ...], [m10, ...], ...]" (ToString only: the dimensions are not part of a fixed form).
 */
template < typename T >
class GUCEF_MATH_TEMPLATE_PUBLIC CTDynamicMatrix
{
    public:

    typedef T TValue;
    typedef CTNumericTraits< T > TTraits;
    typedef typename CTMathComponentTraits< T >::TKernels TKernels;
    typedef CTDynamicVector< T > TVector;

    /** The empty 0 x 0 matrix */
    CTDynamicMatrix( void ) : m_rows( 0 ), m_columns( 0 ) {}

    /** rows x columns zeros */
    CTDynamicMatrix( UInt32 rows, UInt32 columns ) : m_data( (size_t) rows * columns, TTraits::Zero() ), m_rows( rows ), m_columns( columns ) {}

    /** From a fixed-size matrix */
    template < UInt32 R, UInt32 C >
    explicit CTDynamicMatrix( const CTMatrix< T, R, C >& m ) : m_data( m.GetData(), m.GetData() + R * C ), m_rows( R ), m_columns( C ) {}

    /** Into a fixed-size matrix; false (out unchanged) when the dimensions are not R x C */
    template < UInt32 R, UInt32 C >
    bool ToMatrix( CTMatrix< T, R, C >& out ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        if ( R != m_rows || C != m_columns )
            return false;
        out = CTMatrix< T, R, C >::FromColumnMajor( GetData() );
        return true;
    }

    /** From rows * columns values in column-major order */
    static CTDynamicMatrix FromColumnMajor( const T* values, UInt32 rows, UInt32 columns )
    {
        CTDynamicMatrix m( rows, columns );
        for ( size_t i=0; i<m.m_data.size(); ++i )
            m.m_data[ i ] = values[ i ];
        return m;
    }

    /** From rows * columns values in row-major order */
    static CTDynamicMatrix FromRowMajor( const T* values, UInt32 rows, UInt32 columns )
    {
        CTDynamicMatrix m( rows, columns );
        for ( UInt32 r=0; r<rows; ++r )
            for ( UInt32 c=0; c<columns; ++c )
                m.m_data[ (size_t) c * rows + r ] = values[ (size_t) r * columns + c ];
        return m;
    }

    /** The n x n identity */
    static CTDynamicMatrix Identity( UInt32 n )
    {
        CTDynamicMatrix m( n, n );
        m.SetIdentity();
        return m;
    }

    /** Changes the dimensions; the elements of the overlapping block keep their value, new elements are zero */
    void Resize( UInt32 rows, UInt32 columns )
    {
        if ( rows == m_rows && columns == m_columns )
            return;
        GUCEF::vector< T > data( (size_t) rows * columns, TTraits::Zero() );
        for ( UInt32 c=0; c<columns && c<m_columns; ++c )
            for ( UInt32 r=0; r<rows && r<m_rows; ++r )
                data[ (size_t) c * rows + r ] = m_data[ (size_t) c * m_rows + r ];
        m_data.swap( data );
        m_rows = rows;
        m_columns = columns;
    }

    /** Every element zero, dimensions unchanged */
    void SetZero( void ) GUCEF_MATH_REALTIME_CONTEXT
    {
        for ( size_t i=0; i<m_data.size(); ++i )
            m_data[ i ] = TTraits::Zero();
    }

    /** Ones on the main diagonal, zero elsewhere (also for rectangular matrices), dimensions unchanged */
    void SetIdentity( void ) GUCEF_MATH_REALTIME_CONTEXT
    {
        SetZero();
        for ( UInt32 i=0; i<m_rows && i<m_columns; ++i )
            m_data[ (size_t) i * m_rows + i ] = TTraits::One();
    }

    GUCEF_MATH_FORCE_INLINE UInt32 GetRows( void ) const GUCEF_MATH_REALTIME_CONTEXT { return m_rows; }

    GUCEF_MATH_FORCE_INLINE UInt32 GetColumns( void ) const GUCEF_MATH_REALTIME_CONTEXT { return m_columns; }

    GUCEF_MATH_FORCE_INLINE bool IsSquare( void ) const GUCEF_MATH_REALTIME_CONTEXT { return m_rows == m_columns; }

    GUCEF_MATH_FORCE_INLINE bool IsEmpty( void ) const GUCEF_MATH_REALTIME_CONTEXT { return m_data.empty(); }

    /** Element (row, column); out of range asserts in debug builds (the matrix must not be empty) */
    GUCEF_MATH_FORCE_INLINE T& operator()( UInt32 row, UInt32 column ) GUCEF_MATH_REALTIME_CONTEXT
    {
        GUCEF_MATH_ASSERT( row < m_rows && column < m_columns );
        return m_data[ (size_t) column * m_rows + row ];
    }

    GUCEF_MATH_FORCE_INLINE const T& operator()( UInt32 row, UInt32 column ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        GUCEF_MATH_ASSERT( row < m_rows && column < m_columns );
        return m_data[ (size_t) column * m_rows + row ];
    }

    /** The elements in column-major order; null for the empty matrix */
    GUCEF_MATH_FORCE_INLINE const T* GetData( void ) const GUCEF_MATH_REALTIME_CONTEXT { return m_data.empty() ? GUCEF_NULL : &m_data[ 0 ]; }

    GUCEF_MATH_FORCE_INLINE T* GetData( void ) GUCEF_MATH_REALTIME_CONTEXT { return m_data.empty() ? GUCEF_NULL : &m_data[ 0 ]; }

    /** The rows x columns block at (row, column); false (out unchanged) when it does not fit inside this matrix */
    bool GetBlock( UInt32 row, UInt32 column, UInt32 rows, UInt32 columns, CTDynamicMatrix& out ) const
    {
        if ( row > m_rows || column > m_columns || rows > m_rows - row || columns > m_columns - column )
            return false;
        CTDynamicMatrix block( rows, columns );
        for ( UInt32 c=0; c<columns; ++c )
            for ( UInt32 r=0; r<rows; ++r )
                block( r, c ) = ( *this )( row + r, column + c );
        out.Swap( block );
        return true;
    }

    /** Writes 'block' at (row, column); false (unchanged) when it does not fit */
    bool SetBlock( UInt32 row, UInt32 column, const CTDynamicMatrix& block ) GUCEF_MATH_REALTIME_CONTEXT
    {
        if ( row > m_rows || column > m_columns || block.m_rows > m_rows - row || block.m_columns > m_columns - column )
            return false;
        for ( UInt32 c=0; c<block.m_columns; ++c )
            for ( UInt32 r=0; r<block.m_rows; ++r )
                ( *this )( row + r, column + c ) = block( r, c );
        return true;
    }

    /** Column c; false (out unchanged) when out of range */
    bool GetColumn( UInt32 c, TVector& out ) const
    {
        if ( c >= m_columns )
            return false;
        out = TVector( GetData() + (size_t) c * m_rows, m_rows );
        return true;
    }

    /** Row r; false (out unchanged) when out of range */
    bool GetRow( UInt32 r, TVector& out ) const
    {
        if ( r >= m_rows )
            return false;
        TVector row( m_columns );
        for ( UInt32 c=0; c<m_columns; ++c )
            row[ c ] = ( *this )( r, c );
        out = row;
        return true;
    }

    /** out = this + other; false (out unchanged) when the dimensions differ */
    bool TryAdd( const CTDynamicMatrix& other, CTDynamicMatrix& out ) const
    {
        if ( other.m_rows != m_rows || other.m_columns != m_columns )
            return false;
        // element-wise: out may be this or other; allocates only when out has other dimensions
        out.Resize( m_rows, m_columns );
        for ( size_t i=0; i<m_data.size(); ++i )
            out.m_data[ i ] = TKernels::Add( m_data[ i ], other.m_data[ i ] );
        return true;
    }

    /** out = this - other; false (out unchanged) when the dimensions differ */
    bool TrySubtract( const CTDynamicMatrix& other, CTDynamicMatrix& out ) const
    {
        if ( other.m_rows != m_rows || other.m_columns != m_columns )
            return false;
        // element-wise: out may be this or other; allocates only when out has other dimensions
        out.Resize( m_rows, m_columns );
        for ( size_t i=0; i<m_data.size(); ++i )
            out.m_data[ i ] = TKernels::Subtract( m_data[ i ], other.m_data[ i ] );
        return true;
    }

    /** Every element multiplied by 'factor' */
    void Scale( const T& factor ) GUCEF_MATH_REALTIME_CONTEXT
    {
        for ( size_t i=0; i<m_data.size(); ++i )
            m_data[ i ] = TKernels::Multiply( m_data[ i ], factor );
    }

    /**
     *  out = this * other; false (out unchanged) when the inner dimensions differ. out may be this or other (that case
     *  computes into a temporary: it allocates)
     */
    bool TryMultiply( const CTDynamicMatrix& other, CTDynamicMatrix& out ) const;

    /** out = this * v; false (out unchanged) when v has not GetColumns() elements. out may be v (that allocates a temporary) */
    bool TryMultiply( const TVector& v, TVector& out ) const;

    /** The transpose */
    CTDynamicMatrix Transposed( void ) const
    {
        CTDynamicMatrix m( m_columns, m_rows );
        for ( UInt32 c=0; c<m_columns; ++c )
            for ( UInt32 r=0; r<m_rows; ++r )
                m( c, r ) = ( *this )( r, c );
        return m;
    }

    /** Exchanges the content with 'other' without copying elements */
    void Swap( CTDynamicMatrix& other ) GUCEF_MATH_REALTIME_CONTEXT
    {
        m_data.swap( other.m_data );
        const UInt32 rows = m_rows;
        const UInt32 columns = m_columns;
        m_rows = other.m_rows;
        m_columns = other.m_columns;
        other.m_rows = rows;
        other.m_columns = columns;
    }

    friend GUCEF_MATH_FORCE_INLINE bool operator==( const CTDynamicMatrix& a, const CTDynamicMatrix& b ) GUCEF_MATH_REALTIME_CONTEXT
    {
        if ( a.m_rows != b.m_rows || a.m_columns != b.m_columns )
            return false;
        for ( size_t i=0; i<a.m_data.size(); ++i )
            if ( !( a.m_data[ i ] == b.m_data[ i ] ) )
                return false;
        return true;
    }

    friend GUCEF_MATH_FORCE_INLINE bool operator!=( const CTDynamicMatrix& a, const CTDynamicMatrix& b ) GUCEF_MATH_REALTIME_CONTEXT
    {
        return !( a == b );
    }

    /** Same dimensions and every element: |a - b| <= max( absolute, relative * max( |a|, |b| ) ) */
    bool Equals( const CTDynamicMatrix& other, const T& absolute, const T& relative ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        if ( other.m_rows != m_rows || other.m_columns != m_columns )
            return false;
        for ( size_t i=0; i<m_data.size(); ++i )
            if ( !TTraits::EqualsWithin( m_data[ i ], other.m_data[ i ], absolute, relative ) )
                return false;
        return true;
    }

    /** "[[m00, m01, ...], [m10, ...], ...]"; "[]" for the empty matrix (not realtime-safe) */
    CORE::CString ToString( void ) const
    {
        CORE::CString text( "[" );
        for ( UInt32 r=0; r<m_rows; ++r )
        {
            if ( r > 0 )
                text += ", ";
            text += '[';
            for ( UInt32 c=0; c<m_columns; ++c )
            {
                if ( c > 0 )
                    text += ", ";
                text += TTraits::ToString( ( *this )( r, c ) );
            }
            text += ']';
        }
        text += ']';
        return text;
    }

    private:

    GUCEF::vector< T > m_data;
    UInt32 m_rows;
    UInt32 m_columns;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE bool
CTDynamicMatrix< T >::TryMultiply( const CTDynamicMatrix& other, CTDynamicMatrix& out ) const
{
    if ( other.m_rows != m_columns )
        return false;
    if ( &out == this || &out == &other )
    {
        CTDynamicMatrix m;
        TryMultiply( other, m );
        out.Swap( m );
        return true;
    }
    out.Resize( m_rows, other.m_columns );
    if ( !out.m_data.empty() )
    {
        if ( 0 == m_columns )
            out.SetZero();
        else
            CTMatrixKernels< T >::Multiply( GetData(), other.GetData(), out.GetData(), m_rows, m_columns, other.m_columns );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE bool
CTDynamicMatrix< T >::TryMultiply( const TVector& v, TVector& out ) const
{
    if ( v.GetSize() != m_columns )
        return false;
    if ( &out == &v )
    {
        TVector result;
        TryMultiply( v, result );
        out = result;
        return true;
    }
    out.Resize( m_rows );
    for ( UInt32 r=0; r<m_rows; ++r )
        out[ r ] = CTMatrixKernels< T >::Dot( GetData() + r, m_rows, v.GetData(), 1, m_columns );
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

#define GUCEF_MATH_CTDYNAMICMATRIX_MEMBERS( prefix, kind ) \
    prefix bool CTDynamicMatrix< kind >::TryMultiply( const CTDynamicMatrix< kind >&, CTDynamicMatrix< kind >& ) const; \
    prefix bool CTDynamicMatrix< kind >::TryMultiply( const CTDynamicVector< kind >&, CTDynamicVector< kind >& ) const;

#define GUCEF_MATH_CTDYNAMICMATRIX_PREDEFINED_KINDS( prefix ) \
    GUCEF_MATH_CTDYNAMICMATRIX_MEMBERS( prefix, Float32 ) \
    GUCEF_MATH_CTDYNAMICMATRIX_MEMBERS( prefix, Float64 ) \
    GUCEF_MATH_CTDYNAMICMATRIX_MEMBERS( prefix, TFixed16_16 ) \
    GUCEF_MATH_CTDYNAMICMATRIX_MEMBERS( prefix, TFixed32_32 ) \
    GUCEF_MATH_CTDYNAMICMATRIX_MEMBERS( prefix, TFixed48_16 ) \
    GUCEF_MATH_CTDYNAMICMATRIX_MEMBERS( prefix, Int8 ) \
    GUCEF_MATH_CTDYNAMICMATRIX_MEMBERS( prefix, UInt8 ) \
    GUCEF_MATH_CTDYNAMICMATRIX_MEMBERS( prefix, Int16 ) \
    GUCEF_MATH_CTDYNAMICMATRIX_MEMBERS( prefix, UInt16 ) \
    GUCEF_MATH_CTDYNAMICMATRIX_MEMBERS( prefix, Int32 ) \
    GUCEF_MATH_CTDYNAMICMATRIX_MEMBERS( prefix, UInt32 ) \
    GUCEF_MATH_CTDYNAMICMATRIX_MEMBERS( prefix, Int64 ) \
    GUCEF_MATH_CTDYNAMICMATRIX_MEMBERS( prefix, UInt64 )

#ifdef GUCEF_MATH_EXTERN_TEMPLATES_ENABLED
namespace GUCEF {
namespace MATH {
GUCEF_MATH_CTDYNAMICMATRIX_PREDEFINED_KINDS( GUCEF_MATH_EXTERN_TEMPLATE )
}  /* namespace MATH */
}  /* namespace GUCEF */
#endif /* GUCEF_MATH_EXTERN_TEMPLATES_ENABLED ? */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MATH_CTDYNAMICMATRIX_H ? */
