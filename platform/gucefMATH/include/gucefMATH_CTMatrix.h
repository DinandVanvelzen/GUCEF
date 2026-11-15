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

#ifndef GUCEF_MATH_CTMATRIX_H
#define GUCEF_MATH_CTMATRIX_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MATH_CTMATRIX4D_H
#include "gucefMATH_CTMatrix4D.h"
#define GUCEF_MATH_CTMATRIX4D_H
#endif /* GUCEF_MATH_CTMATRIX4D_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MATH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The dot product of two strided sequences of run-time length in the arithmetic of the kind (the run-time counterpart
 *  of the component kernels' Dot): floating point sums the products in index order, fixed point accumulates the raw
 *  products exactly in 128 bits and rounds once (ties away from zero, saturating), integers wrap, exact kinds use their
 *  operators. Zero for count 0. Realtime-safe.
 */
template < typename T, int Kind >
class CTMathRuntimeDot
{
    public:

    /** Floating point and the exact kinds */
    static GUCEF_MATH_FORCE_INLINE T Dot( const T* a, UInt32 strideA, const T* b, UInt32 strideB, UInt32 count ) GUCEF_MATH_REALTIME_CONTEXT
    {
        if ( 0 == count )
            return CTNumericTraits< T >::Zero();
        T sum = a[ 0 ] * b[ 0 ];
        for ( UInt32 i=1; i<count; ++i )
            sum = sum + a[ i * strideA ] * b[ i * strideB ];
        return sum;
    }
};

template < typename T >
class CTMathRuntimeDot< T, MATHCOMPONENTKIND_INTEGER >
{
    public:

    typedef typename CTMathComponentTraits< T >::TKernels TKernels;

    static GUCEF_MATH_FORCE_INLINE T Dot( const T* a, UInt32 strideA, const T* b, UInt32 strideB, UInt32 count ) GUCEF_MATH_REALTIME_CONTEXT
    {
        T sum = T( 0 );
        for ( UInt32 i=0; i<count; ++i )
            sum = TKernels::Add( sum, TKernels::Multiply( a[ i * strideA ], b[ i * strideB ] ) );
        return sum;
    }
};

template < typename TStorage, UInt32 F >
class CTMathRuntimeDot< CTFixedPoint< TStorage, F >, MATHCOMPONENTKIND_FIXED >
{
    public:

    typedef CTFixedPoint< TStorage, F > T;
    enum { StorageBits = sizeof( TStorage ) * 8 };

    static T Dot( const T* a, UInt32 strideA, const T* b, UInt32 strideB, UInt32 count ) GUCEF_MATH_REALTIME_CONTEXT
    {
        CORE::CInt128 sum( (Int64) 0 );
        for ( UInt32 i=0; i<count; ++i )
        {
            const CORE::CInt128 product = CORE::CInt128::Multiply( (Int64) a[ i * strideA ].GetRaw(), (Int64) b[ i * strideB ].GetRaw() );
            if ( !CORE::CInt128::AddChecked( sum, product, sum ) )
                return product.IsNegative() ? T::Lowest() : T::Highest();
        }
        // units of 2^-2F to units of 2^-F, ties away from zero, saturated
        const bool negative = sum.IsNegative();
        const CORE::CInt128 absolute = negative ? CORE::CInt128::Negate( sum ) : sum;
        const CORE::CUInt128 magnitude( absolute.high, absolute.low );
        CORE::CUInt128 quotient = CORE::CUInt128::ShiftRight( magnitude, F );
        if ( F > 0 && CORE::CUInt128::ShiftRight( magnitude, F > 0 ? F - 1 : 0 ).IsOdd() )
            quotient = CORE::CUInt128::Add( quotient, CORE::CUInt128( (UInt64) 1 ) );
        const UInt64 limit = ( ( (UInt64) 1 << ( StorageBits - 1 ) ) - 1 ) + ( negative ? 1 : 0 );
        if ( 0 != quotient.high || quotient.low > limit )
            return negative ? T::Lowest() : T::Highest();
        return T::FromRaw( (TStorage) MathApplySign( negative, quotient.low ) );
    }
};

/*-------------------------------------------------------------------------*/

/**
 *  The precision-sensitive kernels of the general matrices (CTMatrix, CTDynamicMatrix, CTDynamicVector and the
 *  decompositions), defined out of the class so that for the predefined kinds the module's copy runs (section 5.10).
 *  Storage is column-major. Realtime-safe.
 */
template < typename T >
class GUCEF_MATH_TEMPLATE_PUBLIC CTMatrixKernels
{
    public:

    /** sum a[ i * strideA ] b[ i * strideB ] over i < count in the arithmetic of the kind; zero for count 0 */
    static T Dot( const T* a, UInt32 strideA, const T* b, UInt32 strideB, UInt32 count ) GUCEF_MATH_REALTIME_CONTEXT;

    /** out( rows x columns ) = a( rows x inner ) b( inner x columns ); out must not overlap a or b */
    static void Multiply( const T* a, const T* b, T* out, UInt32 rows, UInt32 inner, UInt32 columns ) GUCEF_MATH_REALTIME_CONTEXT;
};

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE T
CTMatrixKernels< T >::Dot( const T* a, UInt32 strideA, const T* b, UInt32 strideB, UInt32 count ) GUCEF_MATH_REALTIME_CONTEXT
{
    return CTMathRuntimeDot< T, CTMathComponentKind< T >::Value >::Dot( a, strideA, b, strideB, count );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE void
CTMatrixKernels< T >::Multiply( const T* a, const T* b, T* out, UInt32 rows, UInt32 inner, UInt32 columns ) GUCEF_MATH_REALTIME_CONTEXT
{
    for ( UInt32 c=0; c<columns; ++c )
        for ( UInt32 r=0; r<rows; ++r )
            out[ c * rows + r ] = CTMathRuntimeDot< T, CTMathComponentKind< T >::Value >::Dot( a + r, rows, b + c * inner, 1, inner );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  A fixed-size Rows x Columns matrix: the general form next to CTMatrix2D/3D/4D, for small systems, Jacobians,
 *  projections between spaces of different dimension and the fixed-size inputs of the decompositions. Column vectors
 *  are CTMatrix< T, N, 1 >.
 *
 *  - Kind: T is any numeric kind of CTNumericTraits; all members work for every kind.
 *  - Layout: column-major T m_data[ Rows * Columns ] (element (r, c) at c * Rows + r), private; access through
 *    operator()( row, column ), GetData (column-major), ToRowMajor. Size Rows * Columns * sizeof( T ), alignment that
 *    of T. No allocation.
 *  - The default constructor gives the zero matrix (Identity() gives ones on the main diagonal, also when not square).
 *  - Products are dimension-checked at compile time: ( R x K ) * ( K x C ) only. Every element of a product is one
 *    dot product of the kind (CTMatrixKernels: fixed point rounds once per element).
 *  - Conversions to and from CTMatrix2D/3D/4D compile for the matching square sizes only.
 *  - Realtime: every member except the text and DataNode members is realtime-safe.
 *  - Text "[[m00, m01, ...], [m10, ...], ...]" (row order); DataNode attributes m<row><column> (up to 10 x 10).
 */
template < typename T, UInt32 R, UInt32 C >
class GUCEF_MATH_TEMPLATE_PUBLIC CTMatrix
{
    public:

    typedef T TValue;
    typedef CTNumericTraits< T > TTraits;
    typedef typename CTMathComponentTraits< T >::TKernels TKernels;
    typedef CTMatrix< T, R, 1 > TColumn;
    typedef CTMatrix< T, 1, C > TRow;

    enum { Rows = R, Columns = C, Count = R * C };

    GUCEF_MATH_STATIC_ASSERT( R > 0 && C > 0, Matrix_dimensions_must_be_positive );

    /** The zero matrix */
    GUCEF_MATH_FORCE_INLINE CTMatrix( void )
    {
        for ( UInt32 i=0; i<Count; ++i )
            m_data[ i ] = TTraits::Zero();
    }

    /** Converts every element with CTNumericConverter (half to even, saturating; NaN into an exact kind gives 0) */
    template < typename U >
    explicit CTMatrix( const CTMatrix< U, R, C >& other )
    {
        TMathStatus status = MATHSTATUS_OK;
        for ( UInt32 i=0; i<Count; ++i )
        {
            m_data[ i ] = TTraits::Zero();
            status |= CTNumericConverter< U, T >::Convert( other.GetData()[ i ], m_data[ i ], CORE::ROUNDINGMODE_HALF_EVEN );
        }
        GUCEF_MATH_ASSERT( 0 == ( status & MATHSTATUS_INVALID_OPERAND ) );
        (void) status;
    }

    /** From a 2x2 matrix (compiles for 2 x 2 only) */
    explicit CTMatrix( const CTMatrix2D< T >& m ) { CopyFromSquare( m.GetData(), 2 ); }

    /** From a 3x3 matrix (compiles for 3 x 3 only) */
    explicit CTMatrix( const CTMatrix3D< T >& m ) { CopyFromSquare( m.GetData(), 3 ); }

    /** From a 4x4 matrix (compiles for 4 x 4 only) */
    explicit CTMatrix( const CTMatrix4D< T >& m ) { CopyFromSquare( m.GetData(), 4 ); }

    static GUCEF_MATH_FORCE_INLINE CTMatrix Zero( void ) GUCEF_MATH_REALTIME_CONTEXT { return CTMatrix(); }

    /** Ones on the main diagonal, zero elsewhere (also for rectangular matrices) */
    static GUCEF_MATH_FORCE_INLINE CTMatrix Identity( void ) GUCEF_MATH_REALTIME_CONTEXT
    {
        CTMatrix m;
        for ( UInt32 i=0; i<R && i<C; ++i )
            m.m_data[ i * R + i ] = TTraits::One();
        return m;
    }

    /** From Rows * Columns values in column-major order (the GetData order) */
    static GUCEF_MATH_FORCE_INLINE CTMatrix FromColumnMajor( const T* values ) GUCEF_MATH_REALTIME_CONTEXT
    {
        CTMatrix m;
        for ( UInt32 i=0; i<Count; ++i )
            m.m_data[ i ] = values[ i ];
        return m;
    }

    /** From Rows * Columns values in row-major order (m00, m01, ..., m10, ...) */
    static GUCEF_MATH_FORCE_INLINE CTMatrix FromRowMajor( const T* values ) GUCEF_MATH_REALTIME_CONTEXT
    {
        CTMatrix m;
        for ( UInt32 r=0; r<R; ++r )
            for ( UInt32 c=0; c<C; ++c )
                m.m_data[ c * R + r ] = values[ r * C + c ];
        return m;
    }

    /*---------------------------------------------------------------------*/
    /*  Access                                                              */
    /*---------------------------------------------------------------------*/

    /** Element (row, column); an index out of range asserts in debug builds and is clamped */
    GUCEF_MATH_FORCE_INLINE T operator()( UInt32 row, UInt32 column ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        GUCEF_MATH_ASSERT( row < R && column < C );
        return m_data[ Index( row, column ) ];
    }

    GUCEF_MATH_FORCE_INLINE T& operator()( UInt32 row, UInt32 column ) GUCEF_MATH_REALTIME_CONTEXT
    {
        GUCEF_MATH_ASSERT( row < R && column < C );
        return m_data[ Index( row, column ) ];
    }

    /** The Rows * Columns elements in column-major order */
    GUCEF_MATH_FORCE_INLINE const T* GetData( void ) const GUCEF_MATH_REALTIME_CONTEXT { return m_data; }

    /** Writes the elements in row-major order */
    GUCEF_MATH_FORCE_INLINE void ToRowMajor( T* out ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        for ( UInt32 r=0; r<R; ++r )
            for ( UInt32 c=0; c<C; ++c )
                out[ r * C + c ] = m_data[ c * R + r ];
    }

    /** Column c (clamped) as a column vector */
    GUCEF_MATH_FORCE_INLINE TColumn GetColumn( UInt32 c ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        return TColumn::FromColumnMajor( m_data + Index( 0, c ) );
    }

    /** Row r (clamped) as a row vector */
    GUCEF_MATH_FORCE_INLINE TRow GetRow( UInt32 r ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        TRow row;
        for ( UInt32 c=0; c<C; ++c )
            row( 0, c ) = m_data[ Index( r, c ) ];
        return row;
    }

    GUCEF_MATH_FORCE_INLINE void SetColumn( UInt32 c, const TColumn& column ) GUCEF_MATH_REALTIME_CONTEXT
    {
        for ( UInt32 r=0; r<R; ++r )
            m_data[ Index( r, c ) ] = column( r, 0 );
    }

    GUCEF_MATH_FORCE_INLINE void SetRow( UInt32 r, const TRow& row ) GUCEF_MATH_REALTIME_CONTEXT
    {
        for ( UInt32 c=0; c<C; ++c )
            m_data[ Index( r, c ) ] = row( 0, c );
    }

    /** As a 2x2 matrix (compiles for 2 x 2 only) */
    GUCEF_MATH_FORCE_INLINE CTMatrix2D< T > ToMatrix2D( void ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        GUCEF_MATH_STATIC_ASSERT( 2 == R && 2 == C, ToMatrix2D_requires_a_2x2_matrix );
        return CTMatrix2D< T >::FromColumnMajor( m_data );
    }

    /** As a 3x3 matrix (compiles for 3 x 3 only) */
    GUCEF_MATH_FORCE_INLINE CTMatrix3D< T > ToMatrix3D( void ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        GUCEF_MATH_STATIC_ASSERT( 3 == R && 3 == C, ToMatrix3D_requires_a_3x3_matrix );
        return CTMatrix3D< T >::FromColumnMajor( m_data );
    }

    /** As a 4x4 matrix (compiles for 4 x 4 only) */
    GUCEF_MATH_FORCE_INLINE CTMatrix4D< T > ToMatrix4D( void ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        GUCEF_MATH_STATIC_ASSERT( 4 == R && 4 == C, ToMatrix4D_requires_a_4x4_matrix );
        return CTMatrix4D< T >::FromColumnMajor( m_data );
    }

    /*---------------------------------------------------------------------*/
    /*  Algebra                                                             */
    /*---------------------------------------------------------------------*/

    friend GUCEF_MATH_FORCE_INLINE CTMatrix operator+( const CTMatrix& a, const CTMatrix& b ) GUCEF_MATH_REALTIME_CONTEXT
    {
        CTMatrix m;
        for ( UInt32 i=0; i<Count; ++i )
            m.m_data[ i ] = TKernels::Add( a.m_data[ i ], b.m_data[ i ] );
        return m;
    }

    friend GUCEF_MATH_FORCE_INLINE CTMatrix operator-( const CTMatrix& a, const CTMatrix& b ) GUCEF_MATH_REALTIME_CONTEXT
    {
        CTMatrix m;
        for ( UInt32 i=0; i<Count; ++i )
            m.m_data[ i ] = TKernels::Subtract( a.m_data[ i ], b.m_data[ i ] );
        return m;
    }

    GUCEF_MATH_FORCE_INLINE CTMatrix operator-( void ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        CTMatrix m;
        for ( UInt32 i=0; i<Count; ++i )
            m.m_data[ i ] = TKernels::Negate( m_data[ i ] );
        return m;
    }

    friend GUCEF_MATH_FORCE_INLINE CTMatrix operator*( const CTMatrix& a, const T& factor ) GUCEF_MATH_REALTIME_CONTEXT
    {
        CTMatrix m;
        for ( UInt32 i=0; i<Count; ++i )
            m.m_data[ i ] = TKernels::Multiply( a.m_data[ i ], factor );
        return m;
    }

    friend GUCEF_MATH_FORCE_INLINE CTMatrix operator*( const T& factor, const CTMatrix& a ) GUCEF_MATH_REALTIME_CONTEXT
    {
        return a * factor;
    }

    /** a * b, ( R x C ) times ( C x K ); other shapes do not compile */
    template < UInt32 K >
    friend GUCEF_MATH_FORCE_INLINE CTMatrix< T, R, K > operator*( const CTMatrix& a, const CTMatrix< T, C, K >& b ) GUCEF_MATH_REALTIME_CONTEXT
    {
        return a.Multiply( b );
    }

    GUCEF_MATH_FORCE_INLINE CTMatrix& operator+=( const CTMatrix& other ) GUCEF_MATH_REALTIME_CONTEXT { *this = *this + other; return *this; }

    GUCEF_MATH_FORCE_INLINE CTMatrix& operator-=( const CTMatrix& other ) GUCEF_MATH_REALTIME_CONTEXT { *this = *this - other; return *this; }

    friend GUCEF_MATH_FORCE_INLINE bool operator==( const CTMatrix& a, const CTMatrix& b ) GUCEF_MATH_REALTIME_CONTEXT
    {
        for ( UInt32 i=0; i<Count; ++i )
            if ( !( a.m_data[ i ] == b.m_data[ i ] ) )
                return false;
        return true;
    }

    friend GUCEF_MATH_FORCE_INLINE bool operator!=( const CTMatrix& a, const CTMatrix& b ) GUCEF_MATH_REALTIME_CONTEXT
    {
        return !( a == b );
    }

    /** this * other; every element one dot product of the kind (CTMatrixKernels::Multiply) */
    template < UInt32 K >
    GUCEF_MATH_FORCE_INLINE CTMatrix< T, R, K > Multiply( const CTMatrix< T, C, K >& other ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        CTMatrix< T, R, K > m;
        CTMatrixKernels< T >::Multiply( m_data, other.m_data, m.m_data, R, C, K );
        return m;
    }

    GUCEF_MATH_FORCE_INLINE CTMatrix< T, C, R > Transposed( void ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        CTMatrix< T, C, R > m;
        for ( UInt32 r=0; r<R; ++r )
            for ( UInt32 c=0; c<C; ++c )
                m.m_data[ r * C + c ] = m_data[ c * R + r ];
        return m;
    }

    /** The sum of the main diagonal (also for rectangular matrices) */
    GUCEF_MATH_FORCE_INLINE T GetTrace( void ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        T sum = TTraits::Zero();
        for ( UInt32 i=0; i<R && i<C; ++i )
            sum = TKernels::Add( sum, m_data[ i * R + i ] );
        return sum;
    }

    /*---------------------------------------------------------------------*/
    /*  Queries and tolerant comparison                                     */
    /*---------------------------------------------------------------------*/

    GUCEF_MATH_FORCE_INLINE bool IsFinite( void ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        for ( UInt32 i=0; i<Count; ++i )
            if ( !TTraits::IsFinite( m_data[ i ] ) )
                return false;
        return true;
    }

    /** Every element equal within the default tolerances of the kind */
    GUCEF_MATH_FORCE_INLINE bool Equals( const CTMatrix& other ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        return Equals( other, TTraits::DefaultAbsoluteTolerance(), TTraits::DefaultRelativeTolerance() );
    }

    /** Every element: |a - b| <= max( absolute, relative * max( |a|, |b| ) ) */
    bool Equals( const CTMatrix& other, const T& absolute, const T& relative ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        for ( UInt32 i=0; i<Count; ++i )
            if ( !TTraits::EqualsWithin( m_data[ i ], other.m_data[ i ], absolute, relative ) )
                return false;
        return true;
    }

    /*---------------------------------------------------------------------*/
    /*  Text and DataNode (not realtime-safe: they allocate)                */
    /*---------------------------------------------------------------------*/

    /** "[[m00, m01, ...], [m10, ...], ...]" */
    CORE::CString ToString( void ) const
    {
        T rows[ Count ];
        ToRowMajor( rows );
        return CTMatrixComponentsCodec< T, R, C >::ToString( rows );
    }

    /** Parses the ToString form; false leaves this matrix unchanged */
    bool FromString( const CORE::CString& text )
    {
        T rows[ Count ];
        if ( !CTMatrixComponentsCodec< T, R, C >::FromString( text, rows ) )
            return false;
        *this = FromRowMajor( rows );
        return true;
    }

    /** Writes the attributes m<row><column> (single-digit indices: up to 10 x 10) */
    bool Serialize( CORE::CDataNode& node, const CORE::CDataNodeSerializableSettings& /*settings*/ ) const
    {
        GUCEF_MATH_STATIC_ASSERT( R <= 10 && C <= 10, DataNode_form_supports_up_to_10_rows_and_columns );
        T rows[ Count ];
        ToRowMajor( rows );
        return CTMatrixComponentsCodec< T, R, C >::Serialize( node, rows );
    }

    /** Reads the attributes m<row><column>; a missing or invalid attribute fails and leaves this matrix unchanged */
    bool Deserialize( const CORE::CDataNode& node, const CORE::CDataNodeSerializableSettings& /*settings*/ )
    {
        GUCEF_MATH_STATIC_ASSERT( R <= 10 && C <= 10, DataNode_form_supports_up_to_10_rows_and_columns );
        T rows[ Count ];
        if ( !CTMatrixComponentsCodec< T, R, C >::Deserialize( node, rows ) )
            return false;
        *this = FromRowMajor( rows );
        return true;
    }

    private:

    template < typename U, UInt32 R2, UInt32 C2 > friend class CTMatrix;

    static GUCEF_MATH_FORCE_INLINE UInt32 Index( UInt32 row, UInt32 column ) GUCEF_MATH_REALTIME_CONTEXT
    {
        return ( column < C ? column : C - 1 ) * R + ( row < R ? row : R - 1 );
    }

    void CopyFromSquare( const T* columnMajor, UInt32 n )
    {
        GUCEF_MATH_ASSERT( n == R && n == C );
        for ( UInt32 i=0; i<Count && i<n*n; ++i )
            m_data[ i ] = columnMajor[ i ];
    }

    T m_data[ Count ];
};

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

/*
 *  The shape of a general matrix is a template parameter, so its products cannot be instantiated in advance; they all go
 *  through CTMatrixKernels< T >, whose two members are instantiated for the predefined kinds (section 5.10 of the
 *  design). The fractions instantiate in their user's code (they are exact in either mode).
 */

#define GUCEF_MATH_CTMATRIXKERNELS_MEMBERS( prefix, kind ) \
    prefix kind CTMatrixKernels< kind >::Dot( const kind*, UInt32, const kind*, UInt32, UInt32 ) GUCEF_MATH_REALTIME_CONTEXT; \
    prefix void CTMatrixKernels< kind >::Multiply( const kind*, const kind*, kind*, UInt32, UInt32, UInt32 ) GUCEF_MATH_REALTIME_CONTEXT;

#define GUCEF_MATH_CTMATRIXKERNELS_PREDEFINED_KINDS( prefix ) \
    GUCEF_MATH_CTMATRIXKERNELS_MEMBERS( prefix, Float32 ) \
    GUCEF_MATH_CTMATRIXKERNELS_MEMBERS( prefix, Float64 ) \
    GUCEF_MATH_CTMATRIXKERNELS_MEMBERS( prefix, TFixed16_16 ) \
    GUCEF_MATH_CTMATRIXKERNELS_MEMBERS( prefix, TFixed32_32 ) \
    GUCEF_MATH_CTMATRIXKERNELS_MEMBERS( prefix, TFixed48_16 ) \
    GUCEF_MATH_CTMATRIXKERNELS_MEMBERS( prefix, Int8 ) \
    GUCEF_MATH_CTMATRIXKERNELS_MEMBERS( prefix, UInt8 ) \
    GUCEF_MATH_CTMATRIXKERNELS_MEMBERS( prefix, Int16 ) \
    GUCEF_MATH_CTMATRIXKERNELS_MEMBERS( prefix, UInt16 ) \
    GUCEF_MATH_CTMATRIXKERNELS_MEMBERS( prefix, Int32 ) \
    GUCEF_MATH_CTMATRIXKERNELS_MEMBERS( prefix, UInt32 ) \
    GUCEF_MATH_CTMATRIXKERNELS_MEMBERS( prefix, Int64 ) \
    GUCEF_MATH_CTMATRIXKERNELS_MEMBERS( prefix, UInt64 )

#ifdef GUCEF_MATH_EXTERN_TEMPLATES_ENABLED
namespace GUCEF {
namespace MATH {
GUCEF_MATH_CTMATRIXKERNELS_PREDEFINED_KINDS( GUCEF_MATH_EXTERN_TEMPLATE )
}  /* namespace MATH */
}  /* namespace GUCEF */
#endif /* GUCEF_MATH_EXTERN_TEMPLATES_ENABLED ? */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MATH_CTMATRIX_H ? */
