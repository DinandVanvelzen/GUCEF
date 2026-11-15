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

#ifndef GUCEF_MATH_CTLEASTSQUARES_H
#define GUCEF_MATH_CTLEASTSQUARES_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MATH_CTQRDECOMPOSITION_H
#include "gucefMATH_CTQrDecomposition.h"
#define GUCEF_MATH_CTQRDECOMPOSITION_H
#endif /* GUCEF_MATH_CTQRDECOMPOSITION_H ? */

#ifndef GUCEF_MATH_CTCHOLESKYDECOMPOSITION_H
#include "gucefMATH_CTCholeskyDecomposition.h"
#define GUCEF_MATH_CTCHOLESKYDECOMPOSITION_H
#endif /* GUCEF_MATH_CTCHOLESKYDECOMPOSITION_H ? */

#ifndef GUCEF_MATH_CTSYMMETRICEIGEN3_H
#include "gucefMATH_CTSymmetricEigen3.h"
#define GUCEF_MATH_CTSYMMETRICEIGEN3_H
#endif /* GUCEF_MATH_CTSYMMETRICEIGEN3_H ? */

#ifndef GUCEF_MATH_CTPOINT3D_H
#include "gucefMATH_CTPoint3D.h"
#define GUCEF_MATH_CTPOINT3D_H
#endif /* GUCEF_MATH_CTPOINT3D_H ? */

#ifndef GUCEF_MATH_CTDIRECTIONVECTOR3D_H
#include "gucefMATH_CTDirectionVector3D.h"
#define GUCEF_MATH_CTDIRECTIONVECTOR3D_H
#endif /* GUCEF_MATH_CTDIRECTIONVECTOR3D_H ? */

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
 *  Least-squares fits.
 *
 *  - Kind: T is Float32 or Float64; centroids and covariances are accumulated in Float64.
 *  - FitLine3D and FitPlane (static, allocation-free, realtime-safe): principal component analysis of the points
 *    through CTSymmetricEigen3: the line runs through the centroid along the eigenvector of the largest eigenvalue of
 *    the covariance; the plane contains the centroid and its normal is the eigenvector of the smallest eigenvalue. They
 *    minimise the sum of squared orthogonal distances; exact data gives the exact line or plane. The sign of the
 *    direction or normal is not specified.
 *  - FitPolynomial: the coefficients c_0 ... c_degree (lowest power first) minimising sum ( p( x_i ) - y_i )^2, by QR of
 *    the Vandermonde matrix (no normal equations: their condition number is the square).
 *  - SolveNormalEquations: x minimising |A x - b| through the Cholesky decomposition of A^T A (fast, condition squared;
 *    prefer CTQrDecomposition::SolveLeastSquares for ill-conditioned A).
 *  - The members keep their work buffers between calls (GUCEF::vector): FitPolynomial and SolveNormalEquations allocate
 *    only when a problem grows. Not realtime-safe.
 */
template < typename T >
class GUCEF_MATH_TEMPLATE_PUBLIC CTLeastSquares
{
    public:

    typedef T TValue;
    typedef CTNumericTraits< T > TTraits;
    typedef CTDynamicMatrix< T > TMatrix;
    typedef CTDynamicVector< T > TVector;

    GUCEF_MATH_STATIC_ASSERT( CTNumericTraits< T >::IsFloatingPoint, Least_squares_requires_a_floating_point_kind );

    /**
     *  The line through 'count' points; false (outputs unchanged) for fewer than 2 points, coincident points or a
     *  non-finite coordinate
     */
    static bool FitLine3D( const CTPoint3D< T >* points, UInt32 count, CTPoint3D< T >& pointOnLine, CTDirectionVector3D< T >& direction ) GUCEF_MATH_REALTIME_CONTEXT;

    /**
     *  The plane through 'count' points as a point and a unit normal; false (outputs unchanged) for fewer than 3
     *  points, collinear points or a non-finite coordinate
     */
    static bool FitPlane( const CTPoint3D< T >* points, UInt32 count, CTPoint3D< T >& pointOnPlane, CTDirectionVector3D< T >& normal ) GUCEF_MATH_REALTIME_CONTEXT;

    /**
     *  The polynomial of the given degree through ( xs[ i ], ys[ i ] ); writes degree + 1 coefficients, lowest power
     *  first. False (coefficients unchanged) for fewer than degree + 1 points or a rank-deficient system (fewer than
     *  degree + 1 distinct x values).
     */
    bool FitPolynomial( const T* xs, const T* ys, UInt32 count, UInt32 degree, T* coefficients );

    /** x minimising |A x - b| through Cholesky of A^T A; false (x unchanged) for mismatched sizes or rank-deficient A */
    bool SolveNormalEquations( const TMatrix& a, const TVector& b, TVector& x );

    private:

    /** Centroid and covariance in Float64; false for a non-finite coordinate */
    static bool GetCovariance( const CTPoint3D< T >* points, UInt32 count, Float64 ( &centroid )[ 3 ], Float64 ( &covariance )[ 3 ][ 3 ] ) GUCEF_MATH_REALTIME_CONTEXT;

    CTQrDecomposition< T > m_qr;
    CTCholeskyDecomposition< T > m_cholesky;
    TMatrix m_vandermonde;
    TMatrix m_normal;
    TVector m_rhs;
    TVector m_solution;
};

/*-------------------------------------------------------------------------*/

typedef CTLeastSquares< Float32 > TFloat32LeastSquares;
typedef CTLeastSquares< Float64 > TFloat64LeastSquares;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE bool
CTLeastSquares< T >::GetCovariance( const CTPoint3D< T >* points, UInt32 count, Float64 ( &centroid )[ 3 ], Float64 ( &covariance )[ 3 ][ 3 ] ) GUCEF_MATH_REALTIME_CONTEXT
{
    centroid[ 0 ] = centroid[ 1 ] = centroid[ 2 ] = 0.0;
    for ( UInt32 i=0; i<count; ++i )
    {
        const Float64 p[ 3 ] = { (Float64) points[ i ].x, (Float64) points[ i ].y, (Float64) points[ i ].z };
        for ( UInt32 k=0; k<3; ++k )
        {
            if ( !( fabs( p[ k ] ) <= DBL_MAX ) )
                return false;
            centroid[ k ] += p[ k ];
        }
    }
    for ( UInt32 k=0; k<3; ++k )
        centroid[ k ] /= (Float64) count;
    for ( UInt32 r=0; r<3; ++r )
        for ( UInt32 c=0; c<3; ++c )
            covariance[ r ][ c ] = 0.0;
    for ( UInt32 i=0; i<count; ++i )
    {
        const Float64 d[ 3 ] = { (Float64) points[ i ].x - centroid[ 0 ], (Float64) points[ i ].y - centroid[ 1 ], (Float64) points[ i ].z - centroid[ 2 ] };
        for ( UInt32 r=0; r<3; ++r )
            for ( UInt32 c=0; c<3; ++c )
                covariance[ r ][ c ] += d[ r ] * d[ c ];
    }
    return true;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE bool
CTLeastSquares< T >::FitLine3D( const CTPoint3D< T >* points, UInt32 count, CTPoint3D< T >& pointOnLine, CTDirectionVector3D< T >& direction ) GUCEF_MATH_REALTIME_CONTEXT
{
    if ( count < 2 || GUCEF_NULL == points )
        return false;
    Float64 centroid[ 3 ];
    Float64 covariance[ 3 ][ 3 ];
    if ( !GetCovariance( points, count, centroid, covariance ) )
        return false;
    Float64 values[ 3 ];
    Float64 vectors[ 3 ][ 3 ];
    UInt32 sweeps = 0;
    CTSymmetricEigen3< Float64 >::DecomposeFloat64( covariance, values, vectors, sweeps );
    if ( !( values[ 0 ] > 0.0 ) )
        return false;   // all points coincide
    CTDirectionVector3D< T > unit;
    if ( !CTDirectionVector3D< T >::TryCreate( CTVector3D< T >( (T) vectors[ 0 ][ 0 ], (T) vectors[ 1 ][ 0 ], (T) vectors[ 2 ][ 0 ] ), unit ) )
        return false;
    pointOnLine = CTPoint3D< T >( (T) centroid[ 0 ], (T) centroid[ 1 ], (T) centroid[ 2 ] );
    direction = unit;
    return true;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE bool
CTLeastSquares< T >::FitPlane( const CTPoint3D< T >* points, UInt32 count, CTPoint3D< T >& pointOnPlane, CTDirectionVector3D< T >& normal ) GUCEF_MATH_REALTIME_CONTEXT
{
    if ( count < 3 || GUCEF_NULL == points )
        return false;
    Float64 centroid[ 3 ];
    Float64 covariance[ 3 ][ 3 ];
    if ( !GetCovariance( points, count, centroid, covariance ) )
        return false;
    Float64 values[ 3 ];
    Float64 vectors[ 3 ][ 3 ];
    UInt32 sweeps = 0;
    CTSymmetricEigen3< Float64 >::DecomposeFloat64( covariance, values, vectors, sweeps );
    // collinear (or coincident) points: the second eigenvalue vanishes next to the first
    if ( !( values[ 1 ] > values[ 0 ] * 1e-14 ) )
        return false;
    CTDirectionVector3D< T > unit;
    if ( !CTDirectionVector3D< T >::TryCreate( CTVector3D< T >( (T) vectors[ 0 ][ 2 ], (T) vectors[ 1 ][ 2 ], (T) vectors[ 2 ][ 2 ] ), unit ) )
        return false;
    pointOnPlane = CTPoint3D< T >( (T) centroid[ 0 ], (T) centroid[ 1 ], (T) centroid[ 2 ] );
    normal = unit;
    return true;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE bool
CTLeastSquares< T >::FitPolynomial( const T* xs, const T* ys, UInt32 count, UInt32 degree, T* coefficients )
{
    const UInt32 terms = degree + 1;
    if ( count < terms || GUCEF_NULL == xs || GUCEF_NULL == ys || GUCEF_NULL == coefficients )
        return false;
    m_vandermonde.Resize( count, terms );
    m_rhs.Resize( count );
    for ( UInt32 i=0; i<count; ++i )
    {
        T power = TTraits::One();
        for ( UInt32 j=0; j<terms; ++j )
        {
            m_vandermonde( i, j ) = power;
            power = power * xs[ i ];
        }
        m_rhs[ i ] = ys[ i ];
    }
    if ( !m_qr.Decompose( m_vandermonde ) || !m_qr.SolveLeastSquares( m_rhs, m_solution ) )
        return false;
    for ( UInt32 j=0; j<terms; ++j )
        coefficients[ j ] = m_solution[ j ];
    return true;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE bool
CTLeastSquares< T >::SolveNormalEquations( const TMatrix& a, const TVector& b, TVector& x )
{
    const UInt32 m = a.GetRows();
    const UInt32 n = a.GetColumns();
    if ( b.GetSize() != m || 0 == n || m < n )
        return false;
    // A^T A (symmetric by construction: the upper triangle is mirrored) and A^T b
    m_normal.Resize( n, n );
    m_rhs.Resize( n );
    for ( UInt32 c=0; c<n; ++c )
    {
        for ( UInt32 r=c; r<n; ++r )
        {
            const T value = CTMatrixKernels< T >::Dot( a.GetData() + (size_t) r * m, 1, a.GetData() + (size_t) c * m, 1, m );
            m_normal( r, c ) = value;
            m_normal( c, r ) = value;
        }
        m_rhs[ c ] = CTMatrixKernels< T >::Dot( a.GetData() + (size_t) c * m, 1, b.GetData(), 1, m );
    }
    if ( !m_cholesky.Decompose( m_normal ) || !m_cholesky.Solve( m_rhs, m_solution ) )
        return false;
    x = m_solution;
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

#define GUCEF_MATH_CTLEASTSQUARES_MEMBERS( prefix, kind ) \
    prefix bool CTLeastSquares< kind >::GetCovariance( const CTPoint3D< kind >*, UInt32, Float64 ( & )[ 3 ], Float64 ( & )[ 3 ][ 3 ] ) GUCEF_MATH_REALTIME_CONTEXT; \
    prefix bool CTLeastSquares< kind >::FitLine3D( const CTPoint3D< kind >*, UInt32, CTPoint3D< kind >&, CTDirectionVector3D< kind >& ) GUCEF_MATH_REALTIME_CONTEXT; \
    prefix bool CTLeastSquares< kind >::FitPlane( const CTPoint3D< kind >*, UInt32, CTPoint3D< kind >&, CTDirectionVector3D< kind >& ) GUCEF_MATH_REALTIME_CONTEXT; \
    prefix bool CTLeastSquares< kind >::FitPolynomial( const kind*, const kind*, UInt32, UInt32, kind* ); \
    prefix bool CTLeastSquares< kind >::SolveNormalEquations( const CTDynamicMatrix< kind >&, const CTDynamicVector< kind >&, CTDynamicVector< kind >& );

#define GUCEF_MATH_CTLEASTSQUARES_PREDEFINED_KINDS( prefix ) \
    GUCEF_MATH_CTLEASTSQUARES_MEMBERS( prefix, Float32 ) \
    GUCEF_MATH_CTLEASTSQUARES_MEMBERS( prefix, Float64 )

#ifdef GUCEF_MATH_EXTERN_TEMPLATES_ENABLED
namespace GUCEF {
namespace MATH {
GUCEF_MATH_CTLEASTSQUARES_PREDEFINED_KINDS( GUCEF_MATH_EXTERN_TEMPLATE )
}  /* namespace MATH */
}  /* namespace GUCEF */
#endif /* GUCEF_MATH_EXTERN_TEMPLATES_ENABLED ? */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MATH_CTLEASTSQUARES_H ? */
