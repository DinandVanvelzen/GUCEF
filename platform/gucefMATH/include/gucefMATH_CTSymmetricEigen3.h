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

#ifndef GUCEF_MATH_CTSYMMETRICEIGEN3_H
#define GUCEF_MATH_CTSYMMETRICEIGEN3_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <float.h>
#include <math.h>

#ifndef GUCEF_MATH_CTMATRIX3D_H
#include "gucefMATH_CTMatrix3D.h"
#define GUCEF_MATH_CTMATRIX3D_H
#endif /* GUCEF_MATH_CTMATRIX3D_H ? */

#ifndef GUCEF_MATH_MATHSTATUS_H
#include "gucefMATH_MathStatus.h"
#define GUCEF_MATH_MATHSTATUS_H
#endif /* GUCEF_MATH_MATHSTATUS_H ? */

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
 *  Eigenvalues and eigenvectors of a symmetric 3x3 matrix by the cyclic Jacobi method: A = V diag( lambda ) V^T.
 *
 *  - Kind: T is Float32 or Float64; the iteration runs in Float64 for both (a Float32 result is the rounded Float64
 *    result).
 *  - Input: the symmetric part ( A + A^T ) / 2 is decomposed.
 *  - Results: eigenvalues sorted descending; the eigenvectors are the columns of V, orthonormal and right-handed (V is a
 *    rotation, det V = +1; the sign of an eigenvector is otherwise arbitrary). A diagonal matrix is returned unchanged
 *    (its diagonal sorted, V a signed permutation). Repeated eigenvalues give an orthonormal basis of their eigenspace.
 *  - The input is scaled by a power of two (exact) before the iteration, so every finite input works, up to the largest
 *    and down to subnormal magnitudes; an eigenvalue beyond the range of Float64 gives MATHSTATUS_OVERFLOW (infinity).
 *  - Iteration cap 32 sweeps: MATHSTATUS_NOT_CONVERGED with the best estimate otherwise (not observed for finite input:
 *    Jacobi converges quadratically, typically in 4 to 6 sweeps). A non-finite element gives MATHSTATUS_INVALID_OPERAND
 *    and the identity decomposition.
 *  - Allocation-free, bounded time: realtime-safe.
 */
template < typename T >
class GUCEF_MATH_TEMPLATE_PUBLIC CTSymmetricEigen3
{
    public:

    typedef T TValue;
    typedef CTNumericTraits< T > TTraits;
    typedef CTVector3D< T > TVector;
    typedef CTMatrix3D< T > TMatrix;

    enum { MaxSweeps = 32 };

    GUCEF_MATH_STATIC_ASSERT( CTNumericTraits< T >::IsFloatingPoint, Symmetric_eigen_decomposition_requires_a_floating_point_kind );

    /** The decomposition of the zero matrix: eigenvalues 0, V the identity */
    CTSymmetricEigen3( void ) GUCEF_MATH_REALTIME_CONTEXT : m_eigenvalues( T( 0 ), T( 0 ), T( 0 ) ), m_eigenvectors(), m_status( MATHSTATUS_OK ), m_sweeps( 0 ) {}

    /** Decomposes 'symmetric'; MATHSTATUS_OK, MATHSTATUS_NOT_CONVERGED or MATHSTATUS_INVALID_OPERAND */
    TMathStatus Decompose( const TMatrix& symmetric ) GUCEF_MATH_REALTIME_CONTEXT;

    /** The eigenvalues, descending */
    GUCEF_MATH_FORCE_INLINE const TVector& GetEigenvalues( void ) const GUCEF_MATH_REALTIME_CONTEXT { return m_eigenvalues; }

    /** The rotation whose columns are the eigenvectors, in the order of the eigenvalues */
    GUCEF_MATH_FORCE_INLINE const TMatrix& GetEigenvectors( void ) const GUCEF_MATH_REALTIME_CONTEXT { return m_eigenvectors; }

    /** Eigenvector i (0 = largest eigenvalue); index clamped */
    GUCEF_MATH_FORCE_INLINE TVector GetEigenvector( UInt32 i ) const GUCEF_MATH_REALTIME_CONTEXT { return m_eigenvectors.GetColumn( i < 3 ? i : 2 ); }

    GUCEF_MATH_FORCE_INLINE TMathStatus GetStatus( void ) const GUCEF_MATH_REALTIME_CONTEXT { return m_status; }

    /** The number of Jacobi sweeps of the last Decompose */
    GUCEF_MATH_FORCE_INLINE UInt32 GetSweepCount( void ) const GUCEF_MATH_REALTIME_CONTEXT { return m_sweeps; }

    /**
     *  The Float64 core on a row-major symmetric 3x3 array: eigenvalues descending into 'values', eigenvectors as the
     *  columns of the row-major rotation 'vectors'. Returns the status; 'sweeps' receives the sweep count.
     */
    static TMathStatus DecomposeFloat64( const Float64 ( &a )[ 3 ][ 3 ], Float64 ( &values )[ 3 ], Float64 ( &vectors )[ 3 ][ 3 ], UInt32& sweeps ) GUCEF_MATH_REALTIME_CONTEXT;

    private:

    TVector m_eigenvalues;
    TMatrix m_eigenvectors;
    TMathStatus m_status;
    UInt32 m_sweeps;
};

/*-------------------------------------------------------------------------*/

typedef CTSymmetricEigen3< Float32 > TFloat32SymmetricEigen3;
typedef CTSymmetricEigen3< Float64 > TFloat64SymmetricEigen3;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE TMathStatus
CTSymmetricEigen3< T >::DecomposeFloat64( const Float64 ( &input )[ 3 ][ 3 ], Float64 ( &values )[ 3 ], Float64 ( &vectors )[ 3 ][ 3 ], UInt32& sweeps ) GUCEF_MATH_REALTIME_CONTEXT
{
    Float64 a[ 3 ][ 3 ];
    Float64 v[ 3 ][ 3 ];
    sweeps = 0;
    for ( UInt32 r=0; r<3; ++r )
        for ( UInt32 c=0; c<3; ++c )
        {
            a[ r ][ c ] = 0.5 * input[ r ][ c ] + 0.5 * input[ c ][ r ];
            v[ r ][ c ] = r == c ? 1.0 : 0.0;
            if ( !( fabs( a[ r ][ c ] ) <= DBL_MAX ) )
            {
                for ( UInt32 i=0; i<3; ++i )
                {
                    values[ i ] = 0.0;
                    for ( UInt32 j=0; j<3; ++j )
                        vectors[ i ][ j ] = i == j ? 1.0 : 0.0;
                }
                return MATHSTATUS_INVALID_OPERAND;
            }
        }

    // scaled by a power of two (exact) so the largest magnitude is in [0.5, 1): no overflow or underflow in theta, t and
    // the rotations for any finite input; the eigenvalues are scaled back at the end
    Float64 largest = 0.0;
    for ( UInt32 r=0; r<3; ++r )
        for ( UInt32 c=0; c<3; ++c )
            largest = fabs( a[ r ][ c ] ) > largest ? fabs( a[ r ][ c ] ) : largest;
    int exponent = 0;
    if ( largest > 0.0 )
    {
        frexp( largest, &exponent );
        for ( UInt32 r=0; r<3; ++r )
            for ( UInt32 c=0; c<3; ++c )
                a[ r ][ c ] = ldexp( a[ r ][ c ], -exponent );
    }

    TMathStatus status = MATHSTATUS_NOT_CONVERGED;
    for ( UInt32 sweep=0; sweep<MaxSweeps; ++sweep )
    {
        const Float64 off = fabs( a[ 0 ][ 1 ] ) + fabs( a[ 0 ][ 2 ] ) + fabs( a[ 1 ][ 2 ] );
        if ( 0.0 == off )
        {
            status = MATHSTATUS_OK;
            break;
        }
        ++sweeps;
        for ( UInt32 p=0; p<2; ++p )
        {
            for ( UInt32 q=p+1; q<3; ++q )
            {
                const Float64 apq = a[ p ][ q ];
                if ( 0.0 == apq )
                    continue;
                // after a few sweeps an element negligible next to both diagonal elements is set to zero
                const Float64 g = 100.0 * fabs( apq );
                if ( sweep > 3 && fabs( a[ p ][ p ] ) + g == fabs( a[ p ][ p ] ) && fabs( a[ q ][ q ] ) + g == fabs( a[ q ][ q ] ) )
                {
                    a[ p ][ q ] = 0.0;
                    a[ q ][ p ] = 0.0;
                    continue;
                }
                // the rotation that zeroes a_pq: t = tan( phi ), the smaller root of t^2 + 2 theta t - 1 = 0
                const Float64 theta = ( a[ q ][ q ] - a[ p ][ p ] ) / ( 2.0 * apq );
                Float64 t;
                if ( fabs( theta ) > 1e150 )
                    t = 0.5 / theta;
                else
                    t = ( theta >= 0.0 ? 1.0 : -1.0 ) / ( fabs( theta ) + sqrt( theta * theta + 1.0 ) );
                const Float64 c = 1.0 / sqrt( t * t + 1.0 );
                const Float64 s = t * c;
                // A' = J^T A J with J the rotation in the (p, q) plane
                for ( UInt32 k=0; k<3; ++k )
                {
                    const Float64 akp = a[ k ][ p ];
                    const Float64 akq = a[ k ][ q ];
                    a[ k ][ p ] = c * akp - s * akq;
                    a[ k ][ q ] = s * akp + c * akq;
                }
                for ( UInt32 k=0; k<3; ++k )
                {
                    const Float64 apk = a[ p ][ k ];
                    const Float64 aqk = a[ q ][ k ];
                    a[ p ][ k ] = c * apk - s * aqk;
                    a[ q ][ k ] = s * apk + c * aqk;
                }
                a[ p ][ q ] = 0.0;
                a[ q ][ p ] = 0.0;
                for ( UInt32 k=0; k<3; ++k )
                {
                    const Float64 vkp = v[ k ][ p ];
                    const Float64 vkq = v[ k ][ q ];
                    v[ k ][ p ] = c * vkp - s * vkq;
                    v[ k ][ q ] = s * vkp + c * vkq;
                }
            }
        }
    }
    if ( MATHSTATUS_OK != status )
    {
        const Float64 off = fabs( a[ 0 ][ 1 ] ) + fabs( a[ 0 ][ 2 ] ) + fabs( a[ 1 ][ 2 ] );
        if ( 0.0 == off )
            status = MATHSTATUS_OK;
    }

    // sort descending (columns of v with their eigenvalues)
    UInt32 order[ 3 ] = { 0, 1, 2 };
    for ( UInt32 i=0; i<2; ++i )
        for ( UInt32 j=i+1; j<3; ++j )
            if ( a[ order[ j ] ][ order[ j ] ] > a[ order[ i ] ][ order[ i ] ] )
            {
                const UInt32 swap = order[ i ];
                order[ i ] = order[ j ];
                order[ j ] = swap;
            }
    for ( UInt32 i=0; i<3; ++i )
    {
        values[ i ] = ldexp( a[ order[ i ] ][ order[ i ] ], exponent );
        if ( !( fabs( values[ i ] ) <= DBL_MAX ) )
            status |= MATHSTATUS_OVERFLOW;
        for ( UInt32 k=0; k<3; ++k )
            vectors[ k ][ i ] = v[ k ][ order[ i ] ];
    }
    // right-handed: det V = +1
    const Float64 determinant = vectors[ 0 ][ 0 ] * ( vectors[ 1 ][ 1 ] * vectors[ 2 ][ 2 ] - vectors[ 2 ][ 1 ] * vectors[ 1 ][ 2 ] )
                              - vectors[ 0 ][ 1 ] * ( vectors[ 1 ][ 0 ] * vectors[ 2 ][ 2 ] - vectors[ 2 ][ 0 ] * vectors[ 1 ][ 2 ] )
                              + vectors[ 0 ][ 2 ] * ( vectors[ 1 ][ 0 ] * vectors[ 2 ][ 1 ] - vectors[ 2 ][ 0 ] * vectors[ 1 ][ 1 ] );
    if ( determinant < 0.0 )
        for ( UInt32 k=0; k<3; ++k )
            vectors[ k ][ 2 ] = -vectors[ k ][ 2 ];
    return status;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE TMathStatus
CTSymmetricEigen3< T >::Decompose( const TMatrix& symmetric ) GUCEF_MATH_REALTIME_CONTEXT
{
    Float64 a[ 3 ][ 3 ];
    for ( UInt32 r=0; r<3; ++r )
        for ( UInt32 c=0; c<3; ++c )
            a[ r ][ c ] = (Float64) symmetric( r, c );
    Float64 values[ 3 ];
    Float64 vectors[ 3 ][ 3 ];
    m_status = DecomposeFloat64( a, values, vectors, m_sweeps );
    m_eigenvalues = TVector( (T) values[ 0 ], (T) values[ 1 ], (T) values[ 2 ] );
    for ( UInt32 r=0; r<3; ++r )
        for ( UInt32 c=0; c<3; ++c )
            m_eigenvectors( r, c ) = (T) vectors[ r ][ c ];
    return m_status;
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

#define GUCEF_MATH_CTSYMMETRICEIGEN3_MEMBERS( prefix, kind ) \
    prefix TMathStatus CTSymmetricEigen3< kind >::Decompose( const CTMatrix3D< kind >& ) GUCEF_MATH_REALTIME_CONTEXT; \
    prefix TMathStatus CTSymmetricEigen3< kind >::DecomposeFloat64( const Float64 ( & )[ 3 ][ 3 ], Float64 ( & )[ 3 ], Float64 ( & )[ 3 ][ 3 ], UInt32& ) GUCEF_MATH_REALTIME_CONTEXT;

#define GUCEF_MATH_CTSYMMETRICEIGEN3_PREDEFINED_KINDS( prefix ) \
    GUCEF_MATH_CTSYMMETRICEIGEN3_MEMBERS( prefix, Float32 ) \
    GUCEF_MATH_CTSYMMETRICEIGEN3_MEMBERS( prefix, Float64 )

#ifdef GUCEF_MATH_EXTERN_TEMPLATES_ENABLED
namespace GUCEF {
namespace MATH {
GUCEF_MATH_CTSYMMETRICEIGEN3_PREDEFINED_KINDS( GUCEF_MATH_EXTERN_TEMPLATE )
}  /* namespace MATH */
}  /* namespace GUCEF */
#endif /* GUCEF_MATH_EXTERN_TEMPLATES_ENABLED ? */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MATH_CTSYMMETRICEIGEN3_H ? */
