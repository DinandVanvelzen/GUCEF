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

#ifndef GUCEF_MATH_CTSVD3_H
#define GUCEF_MATH_CTSVD3_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MATH_CTSYMMETRICEIGEN3_H
#include "gucefMATH_CTSymmetricEigen3.h"
#define GUCEF_MATH_CTSYMMETRICEIGEN3_H
#endif /* GUCEF_MATH_CTSYMMETRICEIGEN3_H ? */

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
 *  Singular value decomposition of a 3x3 matrix, A = U diag( sigma ) V^T, by one-sided Jacobi rotations of the columns
 *  of A (Hestenes): V accumulates the rotations until the columns of B = A V are orthogonal to working precision; they
 *  are sorted by length, and a Givens QR of B gives the rotation U and the singular values on the diagonal of R.
 *  No A^T A is formed, so the decomposition is backward stable and graded or nearly repeated small singular values keep
 *  their relative accuracy (Kabsch / Procrustes fits).
 *
 *  - Kind: T is Float32 or Float64; computed in Float64 for both. The input is scaled by a power of two first, so every
 *    finite input works; a singular value beyond the range of T gives MATHSTATUS_OVERFLOW.
 *  - U and V are rotations (det +1). The singular values are sorted by magnitude, descending; to keep U and V rotations
 *    the sign of det A is folded into the last one (sigma_2 < 0 for a reflection). Rank-deficient input gives zero
 *    singular values and still rotations U, V.
 *  - GetPolarRotation(): U V^T, the rotation closest to A (the rotation of the polar decomposition A = R S).
 *  - Iteration cap 32 sweeps (MATHSTATUS_NOT_CONVERGED with the best estimate; typically 3 to 5 sweeps);
 *    MATHSTATUS_INVALID_OPERAND for non-finite input, which gives the identity decomposition with zero singular values.
 *  - Allocation-free, bounded time: realtime-safe.
 */
template < typename T >
class GUCEF_MATH_TEMPLATE_PUBLIC CTSvd3
{
    public:

    typedef T TValue;
    typedef CTVector3D< T > TVector;
    typedef CTMatrix3D< T > TMatrix;

    enum { MaxSweeps = 32 };

    GUCEF_MATH_STATIC_ASSERT( CTNumericTraits< T >::IsFloatingPoint, SVD_requires_a_floating_point_kind );

    /** The decomposition of the zero matrix */
    CTSvd3( void ) GUCEF_MATH_REALTIME_CONTEXT : m_u(), m_sigma( T( 0 ), T( 0 ), T( 0 ) ), m_v(), m_status( MATHSTATUS_OK ) {}

    /** Decomposes 'a'; MATHSTATUS_OK, MATHSTATUS_NOT_CONVERGED or MATHSTATUS_INVALID_OPERAND */
    TMathStatus Decompose( const TMatrix& a ) GUCEF_MATH_REALTIME_CONTEXT;

    /** The left rotation U */
    GUCEF_MATH_FORCE_INLINE const TMatrix& GetU( void ) const GUCEF_MATH_REALTIME_CONTEXT { return m_u; }

    /** The singular values, by magnitude descending; the last one negative when det A < 0 */
    GUCEF_MATH_FORCE_INLINE const TVector& GetSingularValues( void ) const GUCEF_MATH_REALTIME_CONTEXT { return m_sigma; }

    /** The right rotation V (A = U diag( sigma ) V^T) */
    GUCEF_MATH_FORCE_INLINE const TMatrix& GetV( void ) const GUCEF_MATH_REALTIME_CONTEXT { return m_v; }

    GUCEF_MATH_FORCE_INLINE TMathStatus GetStatus( void ) const GUCEF_MATH_REALTIME_CONTEXT { return m_status; }

    /** U V^T: the rotation of the polar decomposition */
    TMatrix GetPolarRotation( void ) const GUCEF_MATH_REALTIME_CONTEXT;

    /** U diag( sigma ) V^T */
    TMatrix Reconstruct( void ) const GUCEF_MATH_REALTIME_CONTEXT;

    private:

    TMatrix m_u;
    TVector m_sigma;
    TMatrix m_v;
    TMathStatus m_status;
};

/*-------------------------------------------------------------------------*/

typedef CTSvd3< Float32 > TFloat32Svd3;
typedef CTSvd3< Float64 > TFloat64Svd3;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE TMathStatus
CTSvd3< T >::Decompose( const TMatrix& input ) GUCEF_MATH_REALTIME_CONTEXT
{
    // B = A, scaled by a power of two (exact) so the largest magnitude is in [0.5, 1)
    Float64 b[ 3 ][ 3 ];
    Float64 largest = 0.0;
    for ( UInt32 r=0; r<3; ++r )
        for ( UInt32 c=0; c<3; ++c )
        {
            b[ r ][ c ] = (Float64) input( r, c );
            if ( !( fabs( b[ r ][ c ] ) <= DBL_MAX ) )
            {
                m_u = TMatrix();
                m_v = TMatrix();
                m_sigma = TVector( T( 0 ), T( 0 ), T( 0 ) );
                m_status = MATHSTATUS_INVALID_OPERAND;
                return m_status;
            }
            largest = fabs( b[ r ][ c ] ) > largest ? fabs( b[ r ][ c ] ) : largest;
        }
    int exponent = 0;
    if ( largest > 0.0 )
    {
        frexp( largest, &exponent );
        for ( UInt32 r=0; r<3; ++r )
            for ( UInt32 c=0; c<3; ++c )
                b[ r ][ c ] = ldexp( b[ r ][ c ], -exponent );
    }

    // one-sided Jacobi (Hestenes): rotate pairs of columns of B = A V until they are orthogonal to working precision;
    // backward stable, and every singular value keeps its relative accuracy (no A^T A is formed)
    Float64 v[ 3 ][ 3 ] = { { 1.0, 0.0, 0.0 }, { 0.0, 1.0, 0.0 }, { 0.0, 0.0, 1.0 } };
    m_status = MATHSTATUS_NOT_CONVERGED;
    for ( UInt32 sweep=0; sweep<MaxSweeps; ++sweep )
    {
        bool rotated = false;
        for ( UInt32 p=0; p<2; ++p )
        {
            for ( UInt32 q=p+1; q<3; ++q )
            {
                // the pair scaled by its largest magnitude: alpha, beta, gamma stay in range for tiny columns
                Float64 pairLargest = 0.0;
                for ( UInt32 k=0; k<3; ++k )
                {
                    pairLargest = fabs( b[ k ][ p ] ) > pairLargest ? fabs( b[ k ][ p ] ) : pairLargest;
                    pairLargest = fabs( b[ k ][ q ] ) > pairLargest ? fabs( b[ k ][ q ] ) : pairLargest;
                }
                if ( 0.0 == pairLargest )
                    continue;
                int pairExponent = 0;
                frexp( pairLargest, &pairExponent );
                Float64 alpha = 0.0;
                Float64 beta = 0.0;
                Float64 gamma = 0.0;
                for ( UInt32 k=0; k<3; ++k )
                {
                    const Float64 bp = ldexp( b[ k ][ p ], -pairExponent );
                    const Float64 bq = ldexp( b[ k ][ q ], -pairExponent );
                    alpha += bp * bp;
                    beta += bq * bq;
                    gamma += bp * bq;
                }
                if ( !( fabs( gamma ) > DBL_EPSILON * sqrt( alpha * beta ) ) )
                    continue;
                rotated = true;
                // t = tan( phi ), the smaller root of t^2 + 2 zeta t - 1 = 0, which makes columns p and q orthogonal
                const Float64 zeta = ( beta - alpha ) / ( 2.0 * gamma );
                const Float64 t = ( zeta >= 0.0 ? 1.0 : -1.0 ) / ( fabs( zeta ) + sqrt( 1.0 + zeta * zeta ) );
                const Float64 c = 1.0 / sqrt( 1.0 + t * t );
                const Float64 s = c * t;
                for ( UInt32 k=0; k<3; ++k )
                {
                    const Float64 bp = b[ k ][ p ];
                    const Float64 bq = b[ k ][ q ];
                    b[ k ][ p ] = c * bp - s * bq;
                    b[ k ][ q ] = s * bp + c * bq;
                    const Float64 vp = v[ k ][ p ];
                    const Float64 vq = v[ k ][ q ];
                    v[ k ][ p ] = c * vp - s * vq;
                    v[ k ][ q ] = s * vp + c * vq;
                }
            }
        }
        if ( !rotated )
        {
            m_status = MATHSTATUS_OK;
            break;
        }
    }

    // columns sorted by length (a swap of two columns negates one of them, V stays a rotation); the lengths are
    // compared through the scaled columns, so tiny columns sort correctly
    for ( UInt32 i=0; i<2; ++i )
        for ( UInt32 j=i+1; j<3; ++j )
        {
            Float64 scale = 0.0;
            for ( UInt32 k=0; k<3; ++k )
            {
                scale = fabs( b[ k ][ i ] ) > scale ? fabs( b[ k ][ i ] ) : scale;
                scale = fabs( b[ k ][ j ] ) > scale ? fabs( b[ k ][ j ] ) : scale;
            }
            if ( 0.0 == scale )
                continue;
            int scaleExponent = 0;
            frexp( scale, &scaleExponent );
            Float64 lengthI = 0.0;
            Float64 lengthJ = 0.0;
            for ( UInt32 k=0; k<3; ++k )
            {
                const Float64 bi = ldexp( b[ k ][ i ], -scaleExponent );
                const Float64 bj = ldexp( b[ k ][ j ], -scaleExponent );
                lengthI += bi * bi;
                lengthJ += bj * bj;
            }
            if ( lengthJ > lengthI )
            {
                for ( UInt32 k=0; k<3; ++k )
                {
                    const Float64 bk = b[ k ][ i ];
                    b[ k ][ i ] = -b[ k ][ j ];
                    b[ k ][ j ] = bk;
                    const Float64 vk = v[ k ][ i ];
                    v[ k ][ i ] = -v[ k ][ j ];
                    v[ k ][ j ] = vk;
                }
            }
        }

    // Givens QR of B: zero (1, 0), (2, 0), (2, 1); U accumulates the rotations, R = U^T B. The columns of B are
    // orthogonal to working precision, so the off-diagonal of R is negligible next to its diagonal (relative per column)
    Float64 u[ 3 ][ 3 ] = { { 1.0, 0.0, 0.0 }, { 0.0, 1.0, 0.0 }, { 0.0, 0.0, 1.0 } };
    const UInt32 targets[ 3 ][ 2 ] = { { 1, 0 }, { 2, 0 }, { 2, 1 } };
    for ( UInt32 g=0; g<3; ++g )
    {
        const UInt32 row = targets[ g ][ 0 ];
        const UInt32 column = targets[ g ][ 1 ];
        const Float64 x = b[ column ][ column ];
        const Float64 y = b[ row ][ column ];
        if ( 0.0 == y && x >= 0.0 )
            continue;
        // the radius without overflow or underflow
        const Float64 m = fabs( x ) > fabs( y ) ? fabs( x ) : fabs( y );
        const Float64 xs = x / m;
        const Float64 ys = y / m;
        const Float64 radius = sqrt( xs * xs + ys * ys );
        const Float64 c = xs / radius;
        const Float64 s = ys / radius;
        for ( UInt32 k=0; k<3; ++k )
        {
            const Float64 bx = b[ column ][ k ];
            const Float64 by = b[ row ][ k ];
            b[ column ][ k ] = c * bx + s * by;
            b[ row ][ k ] = c * by - s * bx;
            const Float64 ux = u[ k ][ column ];
            const Float64 uy = u[ k ][ row ];
            u[ k ][ column ] = c * ux + s * uy;
            u[ k ][ row ] = c * uy - s * ux;
        }
        b[ row ][ column ] = 0.0;
    }

    for ( UInt32 r=0; r<3; ++r )
        for ( UInt32 c=0; c<3; ++c )
        {
            m_u( r, c ) = (T) u[ r ][ c ];
            m_v( r, c ) = (T) v[ r ][ c ];
        }
    const Float64 sigma[ 3 ] = { ldexp( b[ 0 ][ 0 ], exponent ), ldexp( b[ 1 ][ 1 ], exponent ), ldexp( b[ 2 ][ 2 ], exponent ) };
    m_sigma = TVector( (T) sigma[ 0 ], (T) sigma[ 1 ], (T) sigma[ 2 ] );
    if ( !CTNumericTraits< T >::IsFinite( m_sigma.x ) )
        m_status |= MATHSTATUS_OVERFLOW;
    return m_status;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE typename CTSvd3< T >::TMatrix
CTSvd3< T >::GetPolarRotation( void ) const GUCEF_MATH_REALTIME_CONTEXT
{
    return m_u.Multiply( m_v.Transposed() );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE typename CTSvd3< T >::TMatrix
CTSvd3< T >::Reconstruct( void ) const GUCEF_MATH_REALTIME_CONTEXT
{
    return m_u.Multiply( TMatrix::Scale( m_sigma ) ).Multiply( m_v.Transposed() );
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

#define GUCEF_MATH_CTSVD3_MEMBERS( prefix, kind ) \
    prefix TMathStatus CTSvd3< kind >::Decompose( const CTMatrix3D< kind >& ) GUCEF_MATH_REALTIME_CONTEXT; \
    prefix CTMatrix3D< kind > CTSvd3< kind >::GetPolarRotation( void ) const GUCEF_MATH_REALTIME_CONTEXT; \
    prefix CTMatrix3D< kind > CTSvd3< kind >::Reconstruct( void ) const GUCEF_MATH_REALTIME_CONTEXT;

#define GUCEF_MATH_CTSVD3_PREDEFINED_KINDS( prefix ) \
    GUCEF_MATH_CTSVD3_MEMBERS( prefix, Float32 ) \
    GUCEF_MATH_CTSVD3_MEMBERS( prefix, Float64 )

#ifdef GUCEF_MATH_EXTERN_TEMPLATES_ENABLED
namespace GUCEF {
namespace MATH {
GUCEF_MATH_CTSVD3_PREDEFINED_KINDS( GUCEF_MATH_EXTERN_TEMPLATE )
}  /* namespace MATH */
}  /* namespace GUCEF */
#endif /* GUCEF_MATH_EXTERN_TEMPLATES_ENABLED ? */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MATH_CTSVD3_H ? */
