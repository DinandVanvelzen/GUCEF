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

#ifndef GUCEF_MATH_CTDYNAMICVECTOR_H
#define GUCEF_MATH_CTDYNAMICVECTOR_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MATH_CTMATRIX_H
#include "gucefMATH_CTMatrix.h"
#define GUCEF_MATH_CTMATRIX_H
#endif /* GUCEF_MATH_CTMATRIX_H ? */

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
 *  A vector of run-time size, the right-hand sides and solutions of the dynamic decompositions.
 *
 *  - Kind: T is any numeric kind; GetLength needs a square root.
 *  - Storage: GUCEF::vector< T > (GUCEF allocator). Resize, the sizing constructors and the operations that write into an
 *    output vector of another size allocate: not realtime-safe. Element access and the operations into an output of
 *    the right size do not allocate.
 *  - Operations between vectors check the sizes and return false (output unchanged) when they differ.
 *  - The dot product is one CTMatrixKernels dot product of the kind (fixed point rounds once).
 *  - Text "(v0, v1, ...)" (ToString only: the size is not part of a fixed form).
 */
template < typename T >
class GUCEF_MATH_TEMPLATE_PUBLIC CTDynamicVector
{
    public:

    typedef T TValue;
    typedef CTNumericTraits< T > TTraits;
    typedef typename CTMathComponentTraits< T >::TKernels TKernels;

    /** The empty vector */
    CTDynamicVector( void ) {}

    /** 'size' zeros */
    explicit CTDynamicVector( UInt32 size ) : m_data( size, TTraits::Zero() ) {}

    /** A copy of 'count' values */
    CTDynamicVector( const T* values, UInt32 count ) : m_data( values, values + count ) {}

    /** From a fixed-size column vector */
    template < UInt32 N >
    explicit CTDynamicVector( const CTMatrix< T, N, 1 >& column ) : m_data( column.GetData(), column.GetData() + N ) {}

    /** Into a fixed-size column vector; false (out unchanged) when the size is not N */
    template < UInt32 N >
    bool ToMatrix( CTMatrix< T, N, 1 >& out ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        if ( N != GetSize() )
            return false;
        out = CTMatrix< T, N, 1 >::FromColumnMajor( GetData() );
        return true;
    }

    /** Changes the size; kept elements keep their value, new elements are zero (allocates when growing) */
    void Resize( UInt32 size ) { m_data.resize( size, TTraits::Zero() ); }

    /** Every element zero, size unchanged */
    void SetZero( void ) GUCEF_MATH_REALTIME_CONTEXT
    {
        for ( size_t i=0; i<m_data.size(); ++i )
            m_data[ i ] = TTraits::Zero();
    }

    GUCEF_MATH_FORCE_INLINE UInt32 GetSize( void ) const GUCEF_MATH_REALTIME_CONTEXT { return (UInt32) m_data.size(); }

    GUCEF_MATH_FORCE_INLINE bool IsEmpty( void ) const GUCEF_MATH_REALTIME_CONTEXT { return m_data.empty(); }

    /** Element i; out of range asserts in debug builds (the vector must not be empty) */
    GUCEF_MATH_FORCE_INLINE T& operator[]( UInt32 i ) GUCEF_MATH_REALTIME_CONTEXT { GUCEF_MATH_ASSERT( i < GetSize() ); return m_data[ i ]; }

    GUCEF_MATH_FORCE_INLINE const T& operator[]( UInt32 i ) const GUCEF_MATH_REALTIME_CONTEXT { GUCEF_MATH_ASSERT( i < GetSize() ); return m_data[ i ]; }

    GUCEF_MATH_FORCE_INLINE T& operator()( UInt32 i ) GUCEF_MATH_REALTIME_CONTEXT { return ( *this )[ i ]; }

    GUCEF_MATH_FORCE_INLINE const T& operator()( UInt32 i ) const GUCEF_MATH_REALTIME_CONTEXT { return ( *this )[ i ]; }

    /** The elements; null for the empty vector */
    GUCEF_MATH_FORCE_INLINE const T* GetData( void ) const GUCEF_MATH_REALTIME_CONTEXT { return m_data.empty() ? GUCEF_NULL : &m_data[ 0 ]; }

    GUCEF_MATH_FORCE_INLINE T* GetData( void ) GUCEF_MATH_REALTIME_CONTEXT { return m_data.empty() ? GUCEF_NULL : &m_data[ 0 ]; }

    /** out = this + other; false (out unchanged) when the sizes differ */
    bool TryAdd( const CTDynamicVector& other, CTDynamicVector& out ) const
    {
        if ( other.GetSize() != GetSize() )
            return false;
        out.Resize( GetSize() );
        for ( UInt32 i=0; i<GetSize(); ++i )
            out.m_data[ i ] = TKernels::Add( m_data[ i ], other.m_data[ i ] );
        return true;
    }

    /** out = this - other; false (out unchanged) when the sizes differ */
    bool TrySubtract( const CTDynamicVector& other, CTDynamicVector& out ) const
    {
        if ( other.GetSize() != GetSize() )
            return false;
        out.Resize( GetSize() );
        for ( UInt32 i=0; i<GetSize(); ++i )
            out.m_data[ i ] = TKernels::Subtract( m_data[ i ], other.m_data[ i ] );
        return true;
    }

    /** Every element multiplied by 'factor' */
    void Scale( const T& factor ) GUCEF_MATH_REALTIME_CONTEXT
    {
        for ( size_t i=0; i<m_data.size(); ++i )
            m_data[ i ] = TKernels::Multiply( m_data[ i ], factor );
    }

    /** The dot product (one dot product of the kind); false (out unchanged) when the sizes differ */
    bool TryDot( const CTDynamicVector& other, T& out ) const GUCEF_MATH_REALTIME_CONTEXT;

    /** The Euclidean length (HasSqrt): the square root of the dot product with itself; 0 for the empty vector */
    T GetLength( void ) const GUCEF_MATH_REALTIME_CONTEXT;

    friend GUCEF_MATH_FORCE_INLINE bool operator==( const CTDynamicVector& a, const CTDynamicVector& b ) GUCEF_MATH_REALTIME_CONTEXT
    {
        if ( a.GetSize() != b.GetSize() )
            return false;
        for ( UInt32 i=0; i<a.GetSize(); ++i )
            if ( !( a.m_data[ i ] == b.m_data[ i ] ) )
                return false;
        return true;
    }

    friend GUCEF_MATH_FORCE_INLINE bool operator!=( const CTDynamicVector& a, const CTDynamicVector& b ) GUCEF_MATH_REALTIME_CONTEXT
    {
        return !( a == b );
    }

    /** Same size and every element: |a - b| <= max( absolute, relative * max( |a|, |b| ) ) */
    bool Equals( const CTDynamicVector& other, const T& absolute, const T& relative ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        if ( other.GetSize() != GetSize() )
            return false;
        for ( UInt32 i=0; i<GetSize(); ++i )
            if ( !TTraits::EqualsWithin( m_data[ i ], other.m_data[ i ], absolute, relative ) )
                return false;
        return true;
    }

    /** "(v0, v1, ...)"; "()" for the empty vector (not realtime-safe) */
    CORE::CString ToString( void ) const
    {
        CORE::CString text( "(" );
        for ( UInt32 i=0; i<GetSize(); ++i )
        {
            if ( i > 0 )
                text += ", ";
            text += TTraits::ToString( m_data[ i ] );
        }
        text += ')';
        return text;
    }

    private:

    GUCEF::vector< T > m_data;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE bool
CTDynamicVector< T >::TryDot( const CTDynamicVector& other, T& out ) const GUCEF_MATH_REALTIME_CONTEXT
{
    if ( other.GetSize() != GetSize() )
        return false;
    out = CTMatrixKernels< T >::Dot( GetData(), 1, other.GetData(), 1, GetSize() );
    return true;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE T
CTDynamicVector< T >::GetLength( void ) const GUCEF_MATH_REALTIME_CONTEXT
{
    GUCEF_MATH_STATIC_ASSERT( TTraits::HasSqrt, Length_requires_a_numeric_kind_with_sqrt );
    return TTraits::Sqrt( CTMatrixKernels< T >::Dot( GetData(), 1, GetData(), 1, GetSize() ) );
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

#define GUCEF_MATH_CTDYNAMICVECTOR_ALL_KINDS_MEMBERS( prefix, kind ) \
    prefix bool CTDynamicVector< kind >::TryDot( const CTDynamicVector< kind >&, kind& ) const GUCEF_MATH_REALTIME_CONTEXT;

#define GUCEF_MATH_CTDYNAMICVECTOR_SQRT_MEMBERS( prefix, kind ) \
    prefix kind CTDynamicVector< kind >::GetLength( void ) const GUCEF_MATH_REALTIME_CONTEXT;

#define GUCEF_MATH_CTDYNAMICVECTOR_REAL_KIND( prefix, kind ) GUCEF_MATH_CTDYNAMICVECTOR_ALL_KINDS_MEMBERS( prefix, kind ) GUCEF_MATH_CTDYNAMICVECTOR_SQRT_MEMBERS( prefix, kind )

#define GUCEF_MATH_CTDYNAMICVECTOR_PREDEFINED_KINDS( prefix ) \
    GUCEF_MATH_CTDYNAMICVECTOR_REAL_KIND( prefix, Float32 ) \
    GUCEF_MATH_CTDYNAMICVECTOR_REAL_KIND( prefix, Float64 ) \
    GUCEF_MATH_CTDYNAMICVECTOR_REAL_KIND( prefix, TFixed16_16 ) \
    GUCEF_MATH_CTDYNAMICVECTOR_REAL_KIND( prefix, TFixed32_32 ) \
    GUCEF_MATH_CTDYNAMICVECTOR_REAL_KIND( prefix, TFixed48_16 ) \
    GUCEF_MATH_CTDYNAMICVECTOR_ALL_KINDS_MEMBERS( prefix, Int8 ) \
    GUCEF_MATH_CTDYNAMICVECTOR_ALL_KINDS_MEMBERS( prefix, UInt8 ) \
    GUCEF_MATH_CTDYNAMICVECTOR_ALL_KINDS_MEMBERS( prefix, Int16 ) \
    GUCEF_MATH_CTDYNAMICVECTOR_ALL_KINDS_MEMBERS( prefix, UInt16 ) \
    GUCEF_MATH_CTDYNAMICVECTOR_ALL_KINDS_MEMBERS( prefix, Int32 ) \
    GUCEF_MATH_CTDYNAMICVECTOR_ALL_KINDS_MEMBERS( prefix, UInt32 ) \
    GUCEF_MATH_CTDYNAMICVECTOR_ALL_KINDS_MEMBERS( prefix, Int64 ) \
    GUCEF_MATH_CTDYNAMICVECTOR_ALL_KINDS_MEMBERS( prefix, UInt64 )

#ifdef GUCEF_MATH_EXTERN_TEMPLATES_ENABLED
namespace GUCEF {
namespace MATH {
GUCEF_MATH_CTDYNAMICVECTOR_PREDEFINED_KINDS( GUCEF_MATH_EXTERN_TEMPLATE )
}  /* namespace MATH */
}  /* namespace GUCEF */
#endif /* GUCEF_MATH_EXTERN_TEMPLATES_ENABLED ? */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MATH_CTDYNAMICVECTOR_H ? */
