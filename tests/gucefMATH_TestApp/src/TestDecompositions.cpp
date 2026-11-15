/*
 *  gucefMATH_TestApp: GUCEF test application for the Math module
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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <float.h>
#include <math.h>
#include <string.h>

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_MATH_VECTORS_H
#include "gucefMATH_vectors.h"
#define GUCEF_MATH_VECTORS_H
#endif /* GUCEF_MATH_VECTORS_H ? */

#ifndef GUCEF_MATH_DIRECTIONS_H
#include "gucefMATH_directions.h"
#define GUCEF_MATH_DIRECTIONS_H
#endif /* GUCEF_MATH_DIRECTIONS_H ? */

#ifndef GUCEF_MATH_QUATERNIONS_H
#include "gucefMATH_quaternions.h"
#define GUCEF_MATH_QUATERNIONS_H
#endif /* GUCEF_MATH_QUATERNIONS_H ? */

#ifndef GUCEF_MATH_FRACTIONTYPES_H
#include "gucefMATH_fractiontypes.h"
#define GUCEF_MATH_FRACTIONTYPES_H
#endif /* GUCEF_MATH_FRACTIONTYPES_H ? */

#ifndef GUCEF_MATH_MATRICES_H
#include "gucefMATH_matrices.h"
#define GUCEF_MATH_MATRICES_H
#endif /* GUCEF_MATH_MATRICES_H ? */

#ifndef GUCEF_MATH_CTLUDECOMPOSITION_H
#include "gucefMATH_CTLuDecomposition.h"
#define GUCEF_MATH_CTLUDECOMPOSITION_H
#endif /* GUCEF_MATH_CTLUDECOMPOSITION_H ? */

#ifndef GUCEF_MATH_CTQRDECOMPOSITION_H
#include "gucefMATH_CTQrDecomposition.h"
#define GUCEF_MATH_CTQRDECOMPOSITION_H
#endif /* GUCEF_MATH_CTQRDECOMPOSITION_H ? */

#ifndef GUCEF_MATH_CTCHOLESKYDECOMPOSITION_H
#include "gucefMATH_CTCholeskyDecomposition.h"
#define GUCEF_MATH_CTCHOLESKYDECOMPOSITION_H
#endif /* GUCEF_MATH_CTCHOLESKYDECOMPOSITION_H ? */

#ifndef GUCEF_MATH_CTSVD3_H
#include "gucefMATH_CTSvd3.h"
#define GUCEF_MATH_CTSVD3_H
#endif /* GUCEF_MATH_CTSVD3_H ? */

#ifndef GUCEF_MATH_CTRANDOMSTREAM_H
#include "gucefMATH_CTRandomStream.h"
#define GUCEF_MATH_CTRANDOMSTREAM_H
#endif /* GUCEF_MATH_CTRANDOMSTREAM_H ? */

#include "TestMathReference.h"
#include "TestMathAllocationCounter.h"
#include "TestDecompositions.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

using namespace GUCEF;
using namespace GUCEF::MATH;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

typedef CTDynamicMatrix< Float64 > TMatrix64;
typedef CTDynamicVector< Float64 > TVector64;

/*-------------------------------------------------------------------------*/

/** The largest |a_ij - b_ij| of two dynamic matrices of the same shape; infinity for different shapes */
template < typename T >
static Float64
DynamicDistance( const CTDynamicMatrix< T >& a, const CTDynamicMatrix< T >& b )
{
    if ( a.GetRows() != b.GetRows() || a.GetColumns() != b.GetColumns() )
        return DBL_MAX;
    Float64 largest = 0.0;
    for ( UInt32 r=0; r<a.GetRows(); ++r )
        for ( UInt32 c=0; c<a.GetColumns(); ++c )
        {
            const Float64 d = fabs( (Float64) a( r, c ) - (Float64) b( r, c ) );
            if ( !( d <= largest ) )
                largest = d;
        }
    return largest;
}

/*-------------------------------------------------------------------------*/

/** The largest |a_ij - b_ij| of two 3x3 matrices */
template < typename T >
static Float64
Distance3( const CTMatrix3D< T >& a, const CTMatrix3D< T >& b )
{
    Float64 largest = 0.0;
    for ( UInt32 r=0; r<3; ++r )
        for ( UInt32 c=0; c<3; ++c )
        {
            const Float64 d = fabs( (Float64) a( r, c ) - (Float64) b( r, c ) );
            if ( !( d <= largest ) )
                largest = d;
        }
    return largest;
}

/*-------------------------------------------------------------------------*/

/** A rows x columns matrix of uniform values in [-1, 1), each drawn into a local first */
static TMatrix64
RandomMatrix64( MathReference::CSeededSequence& generator, UInt32 rows, UInt32 columns )
{
    TMatrix64 m( rows, columns );
    for ( UInt32 c=0; c<columns; ++c )
        for ( UInt32 r=0; r<rows; ++r )
        {
            const Float64 value = generator.NextFloat64( -1.0, 1.0 );
            m( r, c ) = value;
        }
    return m;
}

/*-------------------------------------------------------------------------*/

/** The n x n Hilbert matrix 1 / ( i + j + 1 ) */
static TMatrix64
Hilbert64( UInt32 n )
{
    TMatrix64 h( n, n );
    for ( UInt32 r=0; r<n; ++r )
        for ( UInt32 c=0; c<n; ++c )
            h( r, c ) = 1.0 / (Float64) ( r + c + 1 );
    return h;
}

/*-------------------------------------------------------------------------*/

/** U diag( s0, s1, s2 ) V^T for two rotations of the stream (Float64) */
static TFloat64Matrix3D
SvdTestMatrix( CRandomStream& stream, Float64 s0, Float64 s1, Float64 s2 )
{
    const TFloat64Quaternion qu = stream.RandomRotationAs< Float64 >();
    const TFloat64Quaternion qv = stream.RandomRotationAs< Float64 >();
    const TFloat64Matrix3D u = TFloat64Matrix3D::FromQuaternion( qu );
    const TFloat64Matrix3D v = TFloat64Matrix3D::FromQuaternion( qv );
    return u * TFloat64Matrix3D::Scale( TFloat64Vector3D( s0, s1, s2 ) ) * v.Transposed();
}

/*-------------------------------------------------------------------------*/

/** The largest |a_ij| */
static Float64
MaxAbs3( const TFloat64Matrix3D& a )
{
    Float64 largest = 0.0;
    for ( UInt32 r=0; r<3; ++r )
        for ( UInt32 c=0; c<3; ++c )
            largest = fabs( a( r, c ) ) > largest ? fabs( a( r, c ) ) : largest;
    return largest;
}

/*-------------------------------------------------------------------------*/

/** The decompositions with fixed-size data from a realtime context: a RealtimeSanitizer build reports any allocation */
static Float32
RealtimeDecompositionWork( const CTLuDecomposition< Float32 >& lu, const CTCholeskyDecomposition< Float32 >& cholesky, const TFloat32Matrix3D& a ) GUCEF_MATH_REALTIME_CONTEXT
{
    const Float32 b[ 3 ] = { 1.0f, 2.0f, 3.0f };
    CTMatrix< Float32, 3, 1 > rhs = CTMatrix< Float32, 3, 1 >::FromColumnMajor( b );
    CTMatrix< Float32, 3, 1 > x;
    bool ok = lu.Solve( rhs, x );
    CTMatrix< Float32, 3, 3 > inverse;
    ok = lu.GetInverse( inverse ) && ok;
    CTMatrix< Float32, 3, 1 > y;
    ok = cholesky.Solve( rhs, y ) && ok;
    TFloat32SymmetricEigen3 eigen;
    eigen.Decompose( a.Multiply( a.Transposed() ) );
    TFloat32Svd3 svd;
    svd.Decompose( a );
    return ( ok ? 1.0f : 0.0f ) + x( 0, 0 ) + inverse( 1, 1 ) + y( 2, 0 ) + lu.GetDeterminant() + eigen.GetEigenvalues().x + svd.GetSingularValues().y + svd.GetPolarRotation()( 0, 0 );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformDecompositionsTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "Decompositions" );

    GUCEF_TESTFW_TESTCASE( "Decompositions: LU solves the Hilbert 5x5 system within 1e-10 times its condition number" )
    try
    {
        const UInt32 n = 5;
        TMatrix64 h = Hilbert64( n );
        const Float64 ones[ 5 ] = { 1, 1, 1, 1, 1 };
        TVector64 expected( ones, n );
        TVector64 b;
        ASSERT_TRUE( h.TryMultiply( expected, b ) );
        CTLuDecomposition< Float64 > lu;
        ASSERT_TRUE( lu.Decompose( h ) && lu.IsValid() && n == lu.GetSize() );
        TVector64 x;
        ASSERT_TRUE( lu.Solve( b, x ) );
        // the condition number in the infinity norm from the exact inverse (published: 943656)
        TMatrix64 inverse;
        ASSERT_TRUE( lu.GetInverse( inverse ) );
        Float64 normH = 0.0;
        Float64 normInverse = 0.0;
        for ( UInt32 r=0; r<n; ++r )
        {
            Float64 rowH = 0.0;
            Float64 rowInverse = 0.0;
            for ( UInt32 c=0; c<n; ++c )
            {
                rowH += fabs( h( r, c ) );
                rowInverse += fabs( inverse( r, c ) );
            }
            normH = rowH > normH ? rowH : normH;
            normInverse = rowInverse > normInverse ? rowInverse : normInverse;
        }
        const Float64 condition = normH * normInverse;
        ASSERT_TRUE( fabs( condition - 943656.0 ) < 1.0 );
        TVector64 hx;
        ASSERT_TRUE( h.TryMultiply( x, hx ) );
        Float64 residual = 0.0;
        Float64 error = 0.0;
        for ( UInt32 i=0; i<n; ++i )
        {
            residual = fabs( hx[ i ] - b[ i ] ) > residual ? fabs( hx[ i ] - b[ i ] ) : residual;
            error = fabs( x[ i ] - 1.0 ) > error ? fabs( x[ i ] - 1.0 ) : error;
        }
        GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "Decompositions: Hilbert 5x5 in units of 1e-15: residual " << residual * 1e15 << ", error " << error * 1e15 << "; condition " << condition );
        ASSERT_TRUE( residual <= 1e-10 * condition && error <= 1e-10 * condition );
        // the exact inverse of the Hilbert matrix has integer elements: ( 0, 0 ) = 25
        ASSERT_TRUE( fabs( inverse( 0, 0 ) - 25.0 ) < 1e-6 && fabs( inverse( 4, 4 ) - 44100.0 ) < 1e-2 );
        // the determinant of H5 is 1 / 266716800000
        ASSERT_TRUE( fabs( lu.GetDeterminant() * 266716800000.0 - 1.0 ) < 1e-6 );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Decompositions: LU pivots [[0, 1], [1, 0]], detects singular matrices and tracks the determinant sign" )
    try
    {
        const Float64 swapValues[ 4 ] = { 0, 1, 1, 0 };
        CTLuDecomposition< Float64 > lu;
        ASSERT_TRUE( ( lu.Decompose( CTMatrix< Float64, 2, 2 >::FromRowMajor( swapValues ) ) ) );
        ASSERT_TRUE( -1 == lu.GetPivotSign() && 1 == lu.GetPivot( 0 ) && 0 == lu.GetPivot( 1 ) && -1.0 == lu.GetDeterminant() );
        const Float64 rhs[ 2 ] = { 2, 3 };
        CTMatrix< Float64, 2, 1 > x;
        ASSERT_TRUE( ( lu.Solve( CTMatrix< Float64, 2, 1 >::FromColumnMajor( rhs ), x ) && 3.0 == x( 0, 0 ) && 2.0 == x( 1, 0 ) ) );

        // singular: dependent rows, a zero matrix, a non-square matrix; afterwards no valid decomposition
        const Float64 dependent[ 4 ] = { 1, 2, 2, 4 };
        ASSERT_FALSE( ( lu.Decompose( CTMatrix< Float64, 2, 2 >::FromRowMajor( dependent ) ) ) );
        ASSERT_FALSE( lu.IsValid() );
        ASSERT_TRUE( 0.0 == lu.GetDeterminant() );
        ASSERT_FALSE( ( lu.Solve( CTMatrix< Float64, 2, 1 >::FromColumnMajor( rhs ), x ) ) );
        const Float64 rank2[ 9 ] = { 1, 2, 3, 4, 5, 6, 7, 8, 9 };
        ASSERT_FALSE( ( lu.Decompose( CTMatrix< Float64, 3, 3 >::FromRowMajor( rank2 ) ) ) );
        ASSERT_FALSE( lu.Decompose( TMatrix64( 3, 3 ) ) );
        ASSERT_FALSE( lu.Decompose( TMatrix64( 2, 3 ) ) );
        // the tolerance is relative: a scaled singular matrix stays singular, a scaled regular one regular
        CTMatrix< Float64, 2, 2 > tiny = CTMatrix< Float64, 2, 2 >::FromRowMajor( swapValues ) * 1e-200;
        ASSERT_TRUE( lu.Decompose( tiny ) );
        ASSERT_FALSE( ( lu.Decompose( CTMatrix< Float64, 2, 2 >::FromRowMajor( dependent ) * 1e200 ) ) );
        // a near-singular matrix with an explicit tolerance
        const Float64 nearly[ 4 ] = { 1, 1, 1, 1 + 1e-9 };
        ASSERT_TRUE( ( lu.Decompose( CTMatrix< Float64, 2, 2 >::FromRowMajor( nearly ) ) ) );
        ASSERT_FALSE( ( lu.Decompose( CTMatrix< Float64, 2, 2 >::FromRowMajor( nearly ), 1e-6 ) ) );

        // the determinant against CTMatrix4D, and its sign under a row swap
        const UInt64 seed = 0x4B10;
        MathReference::CSeededSequence generator( seed );
        UInt32 failures = 0;
        for ( UInt32 i=0; i<200; ++i )
        {
            TMatrix64 a = RandomMatrix64( generator, 4, 4 );
            TFloat64Matrix4D m4 = TFloat64Matrix4D::FromColumnMajor( a.GetData() );
            if ( !lu.Decompose( a ) )
            {
                ++failures;
                continue;
            }
            const Float64 determinant = lu.GetDeterminant();
            if ( !( fabs( determinant - m4.GetDeterminant() ) <= 1e-13 ) )
                ++failures;
            TMatrix64 swapped = a;
            for ( UInt32 c=0; c<4; ++c )
            {
                swapped( 0, c ) = a( 2, c );
                swapped( 2, c ) = a( 0, c );
            }
            if ( !lu.Decompose( swapped ) || !( fabs( lu.GetDeterminant() + determinant ) <= 1e-13 ) )
                ++failures;
            // the inverse against CTMatrix4D
            CTMatrix< Float64, 4, 4 > inverse;
            if ( !lu.Decompose( a ) || !lu.GetInverse( inverse ) )
                ++failures;
            else
            {
                TFloat64Matrix4D reference = m4.Inverse();
                Float64 largest = 0.0;
                Float64 difference = 0.0;
                for ( UInt32 k=0; k<16; ++k )
                {
                    largest = fabs( reference.GetData()[ k ] ) > largest ? fabs( reference.GetData()[ k ] ) : largest;
                    difference = fabs( reference.GetData()[ k ] - inverse.GetData()[ k ] ) > difference ? fabs( reference.GetData()[ k ] - inverse.GetData()[ k ] ) : difference;
                }
                if ( !( difference <= 1e-9 * largest * largest ) )
                    ++failures;
            }
        }
        ASSERT_TRUE( 0 == failures );
        if ( 0 != failures )
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Decompositions: " << failures << " LU determinant or inverse checks failed, seed " << seed );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Decompositions: LU with fractions is exact, with fixed point close to Float64" )
    try
    {
        // the Hilbert 4x4 system in exact fractions: x = ( 1, 1, 1, 1 ) exactly, det = 1 / 6048000
        typedef CORE::TInt64Fraction F;
        CTMatrix< F, 4, 4 > h;
        CTMatrix< F, 4, 1 > b;
        for ( UInt32 r=0; r<4; ++r )
        {
            F sum( 0, 1 );
            for ( UInt32 c=0; c<4; ++c )
            {
                h( r, c ) = F( 1, (Int64) ( r + c + 1 ) );
                sum = sum + h( r, c );
            }
            b( r, 0 ) = sum;
        }
        CTLuDecomposition< F > exact;
        ASSERT_TRUE( exact.Decompose( h ) );
        CTMatrix< F, 4, 1 > x;
        ASSERT_TRUE( exact.Solve( b, x ) );
        for ( UInt32 i=0; i<4; ++i )
            ASSERT_TRUE( F( 1, 1 ) == x( i, 0 ) );
        ASSERT_TRUE( F( 1, 6048000 ) == exact.GetDeterminant() );

        // Q32.32: a well-conditioned 3x3 system against Float64
        const Float64 values[ 9 ] = { 4, -1, 0.5, -1, 3, 0.25, 0.5, 0.25, 2 };
        const Float64 rhs[ 3 ] = { 1, -2, 0.75 };
        CTLuDecomposition< TFixed32_32 > fixed;
        CTMatrix< TFixed32_32, 3, 3 > a32( CTMatrix< Float64, 3, 3 >::FromRowMajor( values ) );
        ASSERT_TRUE( fixed.Decompose( a32 ) );
        CTMatrix< TFixed32_32, 3, 1 > x32;
        ASSERT_TRUE( ( fixed.Solve( CTMatrix< TFixed32_32, 3, 1 >( CTMatrix< Float64, 3, 1 >::FromColumnMajor( rhs ) ), x32 ) ) );
        CTLuDecomposition< Float64 > reference;
        CTMatrix< Float64, 3, 1 > x64;
        ASSERT_TRUE( ( reference.Decompose( CTMatrix< Float64, 3, 3 >::FromRowMajor( values ) ) && reference.Solve( CTMatrix< Float64, 3, 1 >::FromColumnMajor( rhs ), x64 ) ) );
        for ( UInt32 i=0; i<3; ++i )
            ASSERT_TRUE( fabs( x32( i, 0 ).ToFloat64() - x64( i, 0 ) ) < 1e-8 );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Decompositions: QR gives an orthogonal Q, an upper triangular R with QR = A, and exact least squares of consistent systems" )
    try
    {
        const UInt64 seed = 0x4B20;
        MathReference::CSeededSequence generator( seed );
        UInt32 failures = 0;
        Float64 worstOrthogonality = 0.0;
        Float64 worstProduct = 0.0;
        Float64 worstSolution = 0.0;
        CTQrDecomposition< Float64 > qr;
        for ( UInt32 i=0; i<200; ++i )
        {
            const UInt32 rows = 3 + i % 6;
            const UInt32 columns = 1 + i % 3;
            TMatrix64 a = RandomMatrix64( generator, rows, columns );
            if ( !qr.Decompose( a ) )
            {
                ++failures;
                continue;
            }
            TMatrix64 q;
            TMatrix64 r;
            if ( !qr.GetQ( q ) || !qr.GetR( r ) || rows != q.GetRows() || rows != q.GetColumns() || rows != r.GetRows() || columns != r.GetColumns() )
            {
                ++failures;
                continue;
            }
            TMatrix64 qtq;
            TMatrix64 product;
            q.Transposed().TryMultiply( q, qtq );
            q.TryMultiply( r, product );
            const Float64 orthogonality = DynamicDistance( qtq, TMatrix64::Identity( rows ) );
            const Float64 productError = DynamicDistance( product, a );
            worstOrthogonality = orthogonality > worstOrthogonality ? orthogonality : worstOrthogonality;
            worstProduct = productError > worstProduct ? productError : worstProduct;
            if ( !( orthogonality <= 1e-12 ) || !( productError <= 1e-12 ) )
                ++failures;
            // upper triangular: exact zeros below the diagonal
            for ( UInt32 c=0; c<columns; ++c )
                for ( UInt32 rr=c+1; rr<rows; ++rr )
                    if ( 0.0 != r( rr, c ) )
                        ++failures;
            // a consistent overdetermined system b = A x
            TVector64 expected( columns );
            for ( UInt32 k=0; k<columns; ++k )
            {
                const Float64 value = generator.NextFloat64( -5.0, 5.0 );
                expected[ k ] = value;
            }
            TVector64 b;
            a.TryMultiply( expected, b );
            TVector64 x;
            if ( !qr.SolveLeastSquares( b, x ) || x.GetSize() != columns )
            {
                ++failures;
                continue;
            }
            for ( UInt32 k=0; k<columns; ++k )
            {
                const Float64 error = fabs( x[ k ] - expected[ k ] ) / ( 1.0 + fabs( expected[ k ] ) );
                worstSolution = error > worstSolution ? error : worstSolution;
            }
        }
        GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "Decompositions: QR worst in units of 1e-15: |Q^T Q - I| " << worstOrthogonality * 1e15 << ", |QR - A| " << worstProduct * 1e15 << ", least squares " << worstSolution * 1e15 );
        ASSERT_TRUE( 0 == failures && worstSolution <= 1e-12 );
        if ( 0 != failures )
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Decompositions: " << failures << " QR checks failed, seed " << seed );

        // an inconsistent system: the least-squares solution satisfies the normal equations A^T ( A x - b ) = 0
        TMatrix64 a = RandomMatrix64( generator, 7, 3 );
        TVector64 b( 7 );
        for ( UInt32 k=0; k<7; ++k )
            b[ k ] = (Float64) k - 3.0;
        TVector64 x;
        ASSERT_TRUE( qr.Decompose( a ) && qr.SolveLeastSquares( b, x ) && 3 == qr.GetRank() && qr.IsFullRank() );
        TVector64 ax;
        TVector64 residual;
        TVector64 gradient;
        ASSERT_TRUE( a.TryMultiply( x, ax ) && ax.TrySubtract( b, residual ) && a.Transposed().TryMultiply( residual, gradient ) );
        for ( UInt32 k=0; k<3; ++k )
            ASSERT_TRUE( fabs( gradient[ k ] ) < 1e-12 );

        // rank deficiency: column 2 = column 0 + column 1
        TMatrix64 deficient = RandomMatrix64( generator, 5, 3 );
        for ( UInt32 rr=0; rr<5; ++rr )
            deficient( rr, 2 ) = deficient( rr, 0 ) + deficient( rr, 1 );
        ASSERT_TRUE( qr.Decompose( deficient ) && 2 == qr.GetRank() && !qr.IsFullRank() );
        TVector64 unchanged( 3 );
        ASSERT_FALSE( qr.SolveLeastSquares( TVector64( 5 ), unchanged ) );
        ASSERT_TRUE( TVector64( 3 ) == unchanged );
        // fewer rows than columns, a wrong right-hand side size
        ASSERT_FALSE( qr.Decompose( TMatrix64( 2, 3 ) ) );
        ASSERT_FALSE( qr.IsValid() || 0 != qr.GetRank() );
        ASSERT_TRUE( qr.Decompose( a ) );
        ASSERT_FALSE( qr.SolveLeastSquares( TVector64( 6 ), x ) );

        // fixed-size input and Float32
        const Float32 values[ 6 ] = { 1, 0, 1, 1, 1, 2 };
        CTQrDecomposition< Float32 > qr32;
        ASSERT_TRUE( ( qr32.Decompose( CTMatrix< Float32, 3, 2 >::FromRowMajor( values ) ) ) );
        const Float32 line[ 3 ] = { 1, 3, 5 };     // y = 1 + 2 t at t = 0, 1, 2
        CTMatrix< Float32, 2, 1 > coefficients;
        ASSERT_TRUE( ( qr32.SolveLeastSquares( CTMatrix< Float32, 3, 1 >::FromColumnMajor( line ), coefficients ) ) );
        ASSERT_TRUE( fabs( coefficients( 0, 0 ) - 1.0f ) < 1e-5f && fabs( coefficients( 1, 0 ) - 2.0f ) < 1e-5f );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Decompositions: Cholesky decomposes and solves SPD matrices, rejects the others; LDL^T accepts semidefinite ones" )
    try
    {
        const UInt64 seed = 0x4B30;
        MathReference::CSeededSequence generator( seed );
        UInt32 failures = 0;
        CTCholeskyDecomposition< Float64 > cholesky;
        for ( UInt32 i=0; i<100; ++i )
        {
            const UInt32 n = 1 + i % 6;
            TMatrix64 b = RandomMatrix64( generator, n, n );
            TMatrix64 a;
            b.Transposed().TryMultiply( b, a );
            for ( UInt32 k=0; k<n; ++k )
                a( k, k ) = a( k, k ) + 1.0;
            // A = B^T B + I is SPD
            if ( !cholesky.Decompose( a ) )
            {
                ++failures;
                continue;
            }
            TMatrix64 l;
            TMatrix64 llt;
            cholesky.GetL( l );
            l.TryMultiply( l.Transposed(), llt );
            if ( !( DynamicDistance( llt, a ) <= 1e-12 * ( 1.0 + n ) ) )
                ++failures;
            TVector64 rhs = TVector64( n );
            for ( UInt32 k=0; k<n; ++k )
                rhs[ k ] = (Float64) k + 0.5;
            TVector64 x;
            TVector64 ax;
            if ( !cholesky.Solve( rhs, x ) || !a.TryMultiply( x, ax ) )
                ++failures;
            else
                for ( UInt32 k=0; k<n; ++k )
                    if ( !( fabs( ax[ k ] - rhs[ k ] ) <= 1e-11 ) )
                        ++failures;
            // L D L^T gives the same solution
            TVector64 xLdlt;
            if ( !cholesky.DecomposeLdlt( a ) || !cholesky.IsLdlt() || !cholesky.Solve( rhs, xLdlt ) )
                ++failures;
            else
                for ( UInt32 k=0; k<n; ++k )
                    if ( !( fabs( xLdlt[ k ] - x[ k ] ) <= 1e-10 * ( 1.0 + fabs( x[ k ] ) ) ) )
                        ++failures;
        }
        ASSERT_TRUE( 0 == failures );
        if ( 0 != failures )
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Decompositions: " << failures << " Cholesky checks failed, seed " << seed );

        // not positive definite, not symmetric, not square
        const Float64 indefinite[ 4 ] = { 1, 2, 2, 1 };
        ASSERT_FALSE( ( cholesky.Decompose( CTMatrix< Float64, 2, 2 >::FromRowMajor( indefinite ) ) ) );
        ASSERT_FALSE( cholesky.IsValid() );
        ASSERT_FALSE( ( cholesky.DecomposeLdlt( CTMatrix< Float64, 2, 2 >::FromRowMajor( indefinite ) ) ) );
        const Float64 asymmetric[ 4 ] = { 2, 1, 0, 2 };
        ASSERT_FALSE( ( cholesky.Decompose( CTMatrix< Float64, 2, 2 >::FromRowMajor( asymmetric ) ) ) );
        ASSERT_FALSE( cholesky.Decompose( TMatrix64( 2, 3 ) ) );
        // positive semidefinite [[1, 1], [1, 1]]: LL^T fails, LDL^T succeeds with D = ( 1, 0 ), and Solve reports singular
        const Float64 semidefinite[ 4 ] = { 1, 1, 1, 1 };
        ASSERT_FALSE( ( cholesky.Decompose( CTMatrix< Float64, 2, 2 >::FromRowMajor( semidefinite ) ) ) );
        ASSERT_TRUE( ( cholesky.DecomposeLdlt( CTMatrix< Float64, 2, 2 >::FromRowMajor( semidefinite ) ) ) );
        TVector64 d;
        ASSERT_TRUE( cholesky.GetD( d ) && 1.0 == d[ 0 ] && 0.0 == d[ 1 ] );
        CTMatrix< Float64, 2, 1 > x2;
        ASSERT_FALSE( ( cholesky.Solve( CTMatrix< Float64, 2, 1 >::Identity(), x2 ) ) );
        // the LL^T factor of a known matrix: [[4, 2], [2, 5]] = [[2, 0], [1, 2]] [[2, 1], [0, 2]]
        const Float64 known[ 4 ] = { 4, 2, 2, 5 };
        ASSERT_TRUE( ( cholesky.Decompose( CTMatrix< Float64, 2, 2 >::FromRowMajor( known ) ) && !cholesky.IsLdlt() ) );
        TMatrix64 l;
        ASSERT_TRUE( cholesky.GetL( l ) && 2.0 == l( 0, 0 ) && 1.0 == l( 1, 0 ) && 2.0 == l( 1, 1 ) && 0.0 == l( 0, 1 ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Decompositions: Jacobi eigenvalues of [[2, 1, 0], [1, 2, 1], [0, 1, 2]], diagonal and repeated eigenvalues" )
    try
    {
        const Float64 values[ 9 ] = { 2, 1, 0, 1, 2, 1, 0, 1, 2 };
        TFloat64Matrix3D a = TFloat64Matrix3D::FromRowMajor( values );
        TFloat64SymmetricEigen3 eigen;
        ASSERT_TRUE( MATHSTATUS_OK == eigen.Decompose( a ) && eigen.GetSweepCount() <= 32 );
        const Float64 root2 = sqrt( 2.0 );
        ASSERT_TRUE( fabs( eigen.GetEigenvalues().x - ( 2.0 + root2 ) ) <= 1e-12 );
        ASSERT_TRUE( fabs( eigen.GetEigenvalues().y - 2.0 ) <= 1e-12 );
        ASSERT_TRUE( fabs( eigen.GetEigenvalues().z - ( 2.0 - root2 ) ) <= 1e-12 );
        TFloat64Matrix3D v = eigen.GetEigenvectors();
        ASSERT_TRUE( Distance3( v.Transposed() * v, TFloat64Matrix3D() ) <= 1e-12 );
        ASSERT_TRUE( fabs( v.GetDeterminant() - 1.0 ) <= 1e-12 );
        ASSERT_TRUE( Distance3( a * v, v * TFloat64Matrix3D::Scale( eigen.GetEigenvalues() ) ) <= 1e-12 );

        // a diagonal matrix is returned unchanged
        TFloat64Matrix3D diagonal = TFloat64Matrix3D::Scale( TFloat64Vector3D( 3, 2, 1 ) );
        ASSERT_TRUE( MATHSTATUS_OK == eigen.Decompose( diagonal ) && 0 == eigen.GetSweepCount() );
        ASSERT_TRUE( TFloat64Vector3D( 3, 2, 1 ) == eigen.GetEigenvalues() && eigen.GetEigenvectors() == TFloat64Matrix3D() );
        // unsorted diagonal: sorted, V a signed permutation (a rotation)
        ASSERT_TRUE( MATHSTATUS_OK == eigen.Decompose( TFloat64Matrix3D::Scale( TFloat64Vector3D( 1, 3, 2 ) ) ) );
        ASSERT_TRUE( TFloat64Vector3D( 3, 2, 1 ) == eigen.GetEigenvalues() && 1.0 == eigen.GetEigenvectors().GetDeterminant() );
        ASSERT_TRUE( 1.0 == fabs( eigen.GetEigenvector( 0 ).y ) && 1.0 == fabs( eigen.GetEigenvector( 1 ).z ) && 1.0 == fabs( eigen.GetEigenvector( 2 ).x ) );

        // repeated eigenvalues: [[2, 1, 1], [1, 2, 1], [1, 1, 2]] has 4, 1, 1
        const Float64 repeated[ 9 ] = { 2, 1, 1, 1, 2, 1, 1, 1, 2 };
        TFloat64Matrix3D r = TFloat64Matrix3D::FromRowMajor( repeated );
        ASSERT_TRUE( MATHSTATUS_OK == eigen.Decompose( r ) );
        ASSERT_TRUE( fabs( eigen.GetEigenvalues().x - 4.0 ) <= 1e-12 && fabs( eigen.GetEigenvalues().y - 1.0 ) <= 1e-12 && fabs( eigen.GetEigenvalues().z - 1.0 ) <= 1e-12 );
        v = eigen.GetEigenvectors();
        ASSERT_TRUE( Distance3( v.Transposed() * v, TFloat64Matrix3D() ) <= 1e-12 && fabs( v.GetDeterminant() - 1.0 ) <= 1e-12 );
        ASSERT_TRUE( Distance3( r * v, v * TFloat64Matrix3D::Scale( eigen.GetEigenvalues() ) ) <= 1e-12 );
        // all equal: any basis
        ASSERT_TRUE( MATHSTATUS_OK == eigen.Decompose( TFloat64Matrix3D() * 5.0 ) && TFloat64Vector3D( 5, 5, 5 ) == eigen.GetEigenvalues() );
        // the zero matrix and non-finite input
        ASSERT_TRUE( MATHSTATUS_OK == eigen.Decompose( TFloat64Matrix3D::Zero() ) && TFloat64Vector3D( 0, 0, 0 ) == eigen.GetEigenvalues() );
        TFloat64Matrix3D broken = a;
        broken( 1, 2 ) = MathQuietNaN64();
        ASSERT_TRUE( MATHSTATUS_INVALID_OPERAND == eigen.Decompose( broken ) && eigen.GetEigenvectors() == TFloat64Matrix3D() );

        // seeded sweep: V diag( lambda ) V^T = A, sorted, rotations, converged
        const UInt64 seed = 0x4B40;
        MathReference::CSeededSequence generator( seed );
        UInt32 failures = 0;
        UInt32 mostSweeps = 0;
        for ( UInt32 i=0; i<2000; ++i )
        {
            TFloat64Matrix3D s;
            for ( UInt32 rr=0; rr<3; ++rr )
                for ( UInt32 c=rr; c<3; ++c )
                {
                    const Float64 value = generator.NextFloat64( -10.0, 10.0 );
                    s( rr, c ) = value;
                    s( c, rr ) = value;
                }
            if ( i % 5 == 0 )
                s( 2, 2 ) = s( 1, 1 );      // nearly repeated in some cases
            if ( MATHSTATUS_OK != eigen.Decompose( s ) )
                ++failures;
            mostSweeps = eigen.GetSweepCount() > mostSweeps ? eigen.GetSweepCount() : mostSweeps;
            const TFloat64Matrix3D vv = eigen.GetEigenvectors();
            const TFloat64Vector3D lambda = eigen.GetEigenvalues();
            if ( !( Distance3( vv * TFloat64Matrix3D::Scale( lambda ) * vv.Transposed(), s ) <= 1e-12 * 30.0 ) )
                ++failures;
            if ( !( fabs( vv.GetDeterminant() - 1.0 ) <= 1e-12 ) || !( lambda.x >= lambda.y && lambda.y >= lambda.z ) )
                ++failures;
        }
        GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "Decompositions: Jacobi needed at most " << mostSweeps << " sweeps" );
        ASSERT_TRUE( 0 == failures );
        if ( 0 != failures )
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Decompositions: " << failures << " eigen decompositions failed, seed " << seed );

        // Float32
        TFloat32SymmetricEigen3 eigen32;
        ASSERT_TRUE( MATHSTATUS_OK == eigen32.Decompose( TFloat32Matrix3D( a ) ) );
        ASSERT_TRUE( fabs( eigen32.GetEigenvalues().x - (Float32) ( 2.0 + root2 ) ) <= 1e-6f );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Decompositions: SVD reconstructs 10^4 Float32 matrices within 1e-5, rotations U and V, sorted values with the sign folded into the last" )
    try
    {
        const UInt64 seed = 0x4B50;
        MathReference::CSeededSequence generator( seed );
        UInt32 failures = 0;
        UInt32 negativeLast = 0;
        UInt32 rankDeficient = 0;
        Float64 worst = 0.0;
        TFloat32Svd3 svd;
        for ( UInt32 i=0; i<10000; ++i )
        {
            TFloat32Matrix3D a;
            for ( UInt32 r=0; r<3; ++r )
                for ( UInt32 c=0; c<3; ++c )
                {
                    const Float64 value = generator.NextFloat64( -1.0, 1.0 );
                    a( r, c ) = (Float32) value;
                }
            if ( 1 == i % 4 )
            {
                // rank 2: column 2 a combination of columns 0 and 1
                const Float64 s = generator.NextFloat64( -1.0, 1.0 );
                const Float64 t = generator.NextFloat64( -1.0, 1.0 );
                for ( UInt32 r=0; r<3; ++r )
                    a( r, 2 ) = (Float32) ( s * a( r, 0 ) + t * a( r, 1 ) );
                ++rankDeficient;
            }
            else if ( 3 == i % 8 )
            {
                // rank 1: every column a multiple of column 0
                const Float64 s = generator.NextFloat64( -1.0, 1.0 );
                const Float64 t = generator.NextFloat64( -1.0, 1.0 );
                for ( UInt32 r=0; r<3; ++r )
                {
                    a( r, 1 ) = (Float32) ( s * a( r, 0 ) );
                    a( r, 2 ) = (Float32) ( t * a( r, 0 ) );
                }
                ++rankDeficient;
            }
            if ( MATHSTATUS_OK != svd.Decompose( a ) )
                ++failures;
            const Float64 error = Distance3( svd.Reconstruct(), a );
            worst = error > worst ? error : worst;
            if ( !( error <= 1e-5 ) )
                ++failures;
            if ( !( fabs( svd.GetU().GetDeterminant() - 1.0f ) <= 1e-5f ) || !( fabs( svd.GetV().GetDeterminant() - 1.0f ) <= 1e-5f ) )
                ++failures;
            const TFloat32Vector3D sigma = svd.GetSingularValues();
            if ( !( sigma.x >= 0.0f && sigma.y >= 0.0f && sigma.x >= sigma.y - 1e-6f && sigma.y >= fabs( sigma.z ) - 1e-6f ) )
                ++failures;
            const Float64 determinant = TFloat64Matrix3D( a ).GetDeterminant();
            if ( fabs( determinant ) > 1e-4 && ( determinant < 0.0 ) != ( sigma.z < 0.0f ) )
                ++failures;
            if ( sigma.z < 0.0f )
                ++negativeLast;
        }
        GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "Decompositions: SVD worst |U S V^T - A| in units of 1e-7 " << worst * 1e7 << " over 10^4 matrices (" << rankDeficient << " rank-deficient, " << negativeLast << " reflections)" );
        ASSERT_TRUE( 0 == failures && negativeLast > 1000 );
        if ( 0 != failures )
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Decompositions: " << failures << " SVD checks failed, seed " << seed );

        // a rank-deficient matrix has zero singular values; the polar rotation of R S (S symmetric positive) is R
        TFloat64Svd3 svd64;
        ASSERT_TRUE( MATHSTATUS_OK == svd64.Decompose( TFloat64Matrix3D::Zero() ) && TFloat64Vector3D( 0, 0, 0 ) == svd64.GetSingularValues() );
        ASSERT_TRUE( fabs( svd64.GetU().GetDeterminant() - 1.0 ) < 1e-15 && fabs( svd64.GetV().GetDeterminant() - 1.0 ) < 1e-15 );
        TFloat64Matrix3D rotation = TFloat64Matrix3D::RotationAxisAngle( TFloat64DirectionVector3D( TFloat64Vector3D( 1, -2, 0.5 ) ), TFloat64Radian( 1.1 ) );
        const Float64 stretchValues[ 9 ] = { 3, 0.5, 0, 0.5, 2, 0.25, 0, 0.25, 1 };
        TFloat64Matrix3D stretch = TFloat64Matrix3D::FromRowMajor( stretchValues );
        ASSERT_TRUE( MATHSTATUS_OK == svd64.Decompose( rotation * stretch ) );
        ASSERT_TRUE( Distance3( svd64.GetPolarRotation(), rotation ) <= 1e-12 );
        ASSERT_TRUE( Distance3( svd64.Reconstruct(), rotation * stretch ) <= 1e-12 );
        // a reflection: diag( 1, 2, -3 ) gives 3, 2, -1
        ASSERT_TRUE( MATHSTATUS_OK == svd64.Decompose( TFloat64Matrix3D::Scale( TFloat64Vector3D( 1, 2, -3 ) ) ) );
        ASSERT_TRUE( fabs( svd64.GetSingularValues().x - 3.0 ) < 1e-14 && fabs( svd64.GetSingularValues().y - 2.0 ) < 1e-14 && fabs( svd64.GetSingularValues().z + 1.0 ) < 1e-14 );
        TFloat64Matrix3D broken;
        broken( 0, 1 ) = MathQuietNaN64();
        ASSERT_TRUE( MATHSTATUS_INVALID_OPERAND == svd64.Decompose( broken ) && TFloat64Vector3D( 0, 0, 0 ) == svd64.GetSingularValues() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Decompositions: realtime-safe operations run in a realtime context" )
    try
    {
        const Float32 values[ 9 ] = { 4, 1, 0.5f, 1, 3, 0.25f, 0.5f, 0.25f, 2 };
        CTMatrix< Float32, 3, 3 > a = CTMatrix< Float32, 3, 3 >::FromRowMajor( values );
        CTLuDecomposition< Float32 > lu;
        CTCholeskyDecomposition< Float32 > cholesky;
        ASSERT_TRUE( lu.Decompose( a ) && cholesky.Decompose( a ) );
        Float32 result = RealtimeDecompositionWork( lu, cholesky, a.ToMatrix3D() );
        ASSERT_TRUE( CTNumericTraits< Float32 >::IsFinite( result ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Decompositions: realtime-safe and right-sized operations do not allocate" )
    GUCEF_TESTFW_SKIP_IF( !MathAllocationCounter::IsAvailable(), "no allocation counter with a sanitizer that replaces the allocator (the RealtimeSanitizer build proves the same in a realtime context)" );
    try
    {
        // the counter counts: one probe allocation
        MathAllocationCounter::StartCounting();
        MathAllocationCounter::AllocateProbe();
        ASSERT_TRUE( 1 == MathAllocationCounter::StopCounting() );
        const Float32 values[ 9 ] = { 4, 1, 0.5f, 1, 3, 0.25f, 0.5f, 0.25f, 2 };
        CTMatrix< Float32, 3, 3 > a = CTMatrix< Float32, 3, 3 >::FromRowMajor( values );
        CTLuDecomposition< Float32 > lu;
        CTCholeskyDecomposition< Float32 > cholesky;
        CTQrDecomposition< Float32 > qr;
        ASSERT_TRUE( lu.Decompose( a ) && cholesky.Decompose( a ) && qr.Decompose( a ) );
        const Float32 rhs[ 3 ] = { 1, 2, 3 };
        const CTDynamicVector< Float32 > b( rhs, 3 );
        CTDynamicVector< Float32 > x( 3 );
        CTDynamicVector< Float32 > y( 3 );
        CTDynamicVector< Float32 > z( 3 );
        CTDynamicVector< Float32 > inPlace( rhs, 3 );
        CTDynamicMatrix< Float32 > inverse( 3, 3 );
        bool ok = true;
        MathAllocationCounter::StartCounting();
        Float32 result = RealtimeDecompositionWork( lu, cholesky, a.ToMatrix3D() );
        for ( UInt32 i=0; i<10; ++i )
        {
            // same size again: the factors reuse their storage
            ok = lu.Decompose( a ) && cholesky.Decompose( a ) && qr.Decompose( a ) && ok;
            ok = lu.Solve( b, x ) && cholesky.Solve( b, y ) && qr.SolveLeastSquares( b, z ) && ok;
            ok = cholesky.Solve( inPlace, inPlace ) && lu.GetInverse( inverse ) && ok;
            inPlace = b;
        }
        const UInt32 allocations = MathAllocationCounter::StopCounting();
        ASSERT_TRUE( 0 == allocations && ok && CTNumericTraits< Float32 >::IsFinite( result ) );
        for ( UInt32 i=0; i<3; ++i )
            ASSERT_TRUE( fabs( x[ i ] - y[ i ] ) < 1e-5f && fabs( x[ i ] - z[ i ] ) < 1e-5f );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Decompositions: SVD and Jacobi at extreme scales, graded and nearly repeated small singular values (Float64)" )
    try
    {
        // regression: A^T A overflowed at 1e160 and underflowed at 1e-170, and its squared condition lost
        // graded or nearly repeated small singular values; the eigen decomposition overflowed near 1e308
        const UInt64 seed = 0x4B90;
        CRandomStream stream( CPcg32( seed, 11 ) );
        TFloat64Svd3 svd;
        UInt32 failures = 0;
        Float64 worstRelative = 0.0;
        const Float64 cases[ 6 ][ 3 ] = { { 3, 2, 1 }, { 3, 2, -1 }, { 1, 1e-6, 1e-12 }, { 1, 1e-9, 5e-10 }, { 1, 1e-5, 1e-5 }, { 1, 1e-15, -1e-15 } };
        const Float64 scales[ 5 ] = { 1.0, 1e150, 1e-150, 1e160, 1e-170 };
        for ( UInt32 k=0; k<5; ++k )
        {
            for ( UInt32 c=0; c<6; ++c )
            {
                for ( UInt32 i=0; i<20; ++i )
                {
                    const Float64 s = scales[ k ];
                    TFloat64Matrix3D a = SvdTestMatrix( stream, cases[ c ][ 0 ] * s, cases[ c ][ 1 ] * s, cases[ c ][ 2 ] * s );
                    const Float64 norm = MaxAbs3( a );
                    if ( MATHSTATUS_OK != svd.Decompose( a ) )
                    {
                        ++failures;
                        continue;
                    }
                    // backward stable: the reconstruction is accurate relative to |A|
                    const Float64 relative = Distance3( svd.Reconstruct(), a ) / norm;
                    worstRelative = relative > worstRelative ? relative : worstRelative;
                    if ( !( relative <= 1e-14 ) )
                        ++failures;
                    // every singular value within a few epsilon of sigma_0 of the constructed one (the construction
                    // itself rounds at that level), sorted, the sign of the determinant on the last one
                    const TFloat64Vector3D sigma = svd.GetSingularValues();
                    const Float64 expected[ 3 ] = { fabs( cases[ c ][ 0 ] ) * s, fabs( cases[ c ][ 1 ] ) * s, cases[ c ][ 2 ] * s };
                    const Float64 got[ 3 ] = { sigma.x, sigma.y, sigma.z };
                    for ( UInt32 j=0; j<3; ++j )
                        if ( !( fabs( fabs( got[ j ] ) - fabs( expected[ j ] ) ) <= 1e-14 * fabs( expected[ 0 ] ) ) )
                            ++failures;
                    if ( fabs( expected[ 2 ] ) > 1e-6 * s && ( ( got[ 2 ] < 0.0 ) != ( expected[ 2 ] < 0.0 ) ) )
                        ++failures;
                    if ( !( got[ 0 ] >= got[ 1 ] && got[ 1 ] >= fabs( got[ 2 ] ) ) )
                        ++failures;
                    if ( !( fabs( svd.GetU().GetDeterminant() - 1.0 ) <= 1e-14 ) || !( fabs( svd.GetV().GetDeterminant() - 1.0 ) <= 1e-14 ) )
                        ++failures;
                }
            }
        }
        GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "Decompositions: SVD worst relative reconstruction at extreme scales in units of 1e-16 " << worstRelative * 1e16 );
        ASSERT_TRUE( 0 == failures );
        if ( 0 != failures )
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Decompositions: " << failures << " extreme SVD checks failed, seed " << seed );
        // a diagonal matrix with a nearly repeated tiny pair: exact singular values
        ASSERT_TRUE( MATHSTATUS_OK == svd.Decompose( TFloat64Matrix3D::Scale( TFloat64Vector3D( 5e-10, -1.0, 1e-9 ) ) ) );
        ASSERT_TRUE( 1.0 == svd.GetSingularValues().x && 1e-9 == svd.GetSingularValues().y && -5e-10 == svd.GetSingularValues().z );

        // eigenvalues near the top of the range: [[-1e308, 1e308], [1e308, 1e308]] has +-sqrt( 2 ) 1e308
        const Float64 huge[ 9 ] = { -1e308, 1e308, 0, 1e308, 1e308, 0, 0, 0, 1 };
        TFloat64SymmetricEigen3 eigen;
        ASSERT_TRUE( MATHSTATUS_OK == eigen.Decompose( TFloat64Matrix3D::FromRowMajor( huge ) ) );
        ASSERT_TRUE( fabs( eigen.GetEigenvalues().x / ( sqrt( 2.0 ) * 1e308 ) - 1.0 ) < 1e-15 && 1.0 == eigen.GetEigenvalues().y );
        ASSERT_TRUE( fabs( eigen.GetEigenvalues().z / ( sqrt( 2.0 ) * 1e308 ) + 1.0 ) < 1e-15 );
        // and near the bottom: a random symmetric matrix scaled by 1e-300 decomposes like the unscaled one
        for ( UInt32 i=0; i<50; ++i )
        {
            TFloat64Matrix3D small = SvdTestMatrix( stream, 1, 0, 0 );
            small = small + small.Transposed();
            TFloat64SymmetricEigen3 reference;
            reference.Decompose( small );
            ASSERT_TRUE( MATHSTATUS_OK == eigen.Decompose( small * 1e-300 ) );
            const TFloat64Vector3D scaled = eigen.GetEigenvalues() * 1e300;
            ASSERT_TRUE( fabs( scaled.x - reference.GetEigenvalues().x ) < 1e-12 && fabs( scaled.z - reference.GetEigenvalues().z ) < 1e-12 );
        }
        // an eigenvalue beyond the range: overflow reported
        const Float64 tooLarge[ 9 ] = { 1.7e308, 1.7e308, 0, 1.7e308, 1.7e308, 0, 0, 0, 1 };
        ASSERT_TRUE( 0 != ( MATHSTATUS_OVERFLOW & eigen.Decompose( TFloat64Matrix3D::FromRowMajor( tooLarge ) ) ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Decompositions: fixed-point LU near the range limit solves correctly or reports failure, never a saturated result" )
    try
    {
        // regression: [[20000, 20000], [-20000, 20000]] x = ( 20000, 0 ) saturated during elimination (40000)
        // and returned ( -1.49, -0.78 ) instead of ( 0.5, 0.5 )
        const Float64 rows[ 4 ] = { 20000, 20000, -20000, 20000 };
        CTLuDecomposition< TFixed16_16 > lu;
        ASSERT_TRUE( lu.Decompose( CTMatrix< TFixed16_16, 2, 2 >( CTMatrix< Float64, 2, 2 >::FromRowMajor( rows ) ) ) );
        const Float64 rhs[ 2 ] = { 20000, 0 };
        CTMatrix< TFixed16_16, 2, 1 > x;
        ASSERT_TRUE( ( lu.Solve( CTMatrix< TFixed16_16, 2, 1 >( CTMatrix< Float64, 2, 1 >::FromColumnMajor( rhs ) ), x ) ) );
        ASSERT_TRUE( fabs( x( 0, 0 ).ToFloat64() - 0.5 ) < 1e-3 && fabs( x( 1, 0 ).ToFloat64() - 0.5 ) < 1e-3 );
        // the determinant 8e8 is beyond Q16.16: it saturates (a scalar result, documented), the inverse is fine
        CTMatrix< TFixed16_16, 2, 2 > inverse;
        ASSERT_TRUE( lu.GetInverse( inverse ) && fabs( inverse( 0, 0 ).ToFloat64() - 0.000025 ) < 2e-5 );

        // seeded systems with entries up to 30000 against Float64: correct within the precision the row scaling keeps,
        // or false; never a wrong answer
        const UInt64 seed = 0x4B91;
        MathReference::CSeededSequence generator( seed );
        UInt32 failures = 0;
        UInt32 solved = 0;
        for ( UInt32 i=0; i<500; ++i )
        {
            CTMatrix< Float64, 3, 3 > a;
            CTMatrix< Float64, 3, 1 > b;
            for ( UInt32 r=0; r<3; ++r )
            {
                for ( UInt32 c=0; c<3; ++c )
                {
                    const Float64 value = generator.NextFloat64( -30000.0, 30000.0 );
                    a( r, c ) = value;
                }
                const Float64 value = generator.NextFloat64( -30000.0, 30000.0 );
                b( r, 0 ) = value;
            }
            CTLuDecomposition< Float64 > reference;
            CTMatrix< Float64, 3, 1 > expected;
            if ( !reference.Decompose( a ) || !reference.Solve( b, expected ) )
                continue;
            CTLuDecomposition< TFixed16_16 > fixed;
            CTMatrix< TFixed16_16, 3, 3 > a16( a );
            CTMatrix< TFixed16_16, 3, 1 > x16;
            if ( !fixed.Decompose( a16 ) || !fixed.Solve( CTMatrix< TFixed16_16, 3, 1 >( b ), x16 ) )
                continue;
            ++solved;
            // the residual of the fixed-point solution in the rounded system, relative to the scale of the problem
            CTLuDecomposition< Float64 > rounded;
            CTMatrix< Float64, 3, 1 > exact16;
            if ( !rounded.Decompose( CTMatrix< Float64, 3, 3 >( a16 ) ) || !rounded.Solve( CTMatrix< Float64, 3, 1 >( CTMatrix< TFixed16_16, 3, 1 >( b ) ), exact16 ) )
                continue;
            for ( UInt32 r=0; r<3; ++r )
                if ( !( fabs( x16( r, 0 ).ToFloat64() - exact16( r, 0 ) ) <= 0.05 * ( 1.0 + fabs( exact16( r, 0 ) ) ) ) )
                    ++failures;
        }
        GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "Decompositions: fixed-point LU solved " << solved << " of 500 systems with entries up to 30000" );
        ASSERT_TRUE( 0 == failures && solved > 300 );
        if ( 0 != failures )
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Decompositions: " << failures << " fixed-point LU solutions wrong, seed " << seed );

        // a solution beyond the range of the format: false, never a saturated value
        const Float64 tiny[ 4 ] = { 0.001, 0, 0, 1 };
        ASSERT_TRUE( lu.Decompose( CTMatrix< TFixed16_16, 2, 2 >( CTMatrix< Float64, 2, 2 >::FromRowMajor( tiny ) ) ) );
        const Float64 big[ 2 ] = { 100, 0 };
        ASSERT_FALSE( ( lu.Solve( CTMatrix< TFixed16_16, 2, 1 >( CTMatrix< Float64, 2, 1 >::FromColumnMajor( big ) ), x ) ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
