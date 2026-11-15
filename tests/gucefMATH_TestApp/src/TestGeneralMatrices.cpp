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

#ifndef GUCEF_MATH_CTDYNAMICMATRIX_H
#include "gucefMATH_CTDynamicMatrix.h"
#define GUCEF_MATH_CTDYNAMICMATRIX_H
#endif /* GUCEF_MATH_CTDYNAMICMATRIX_H ? */

#include "TestMathReference.h"
#include "TestMathAllocationCounter.h"
#include "TestGeneralMatrices.h"

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

typedef CTMatrix< Float64, 2, 3 > TTestMatrix23;
typedef CTMatrix< Float64, 3, 4 > TTestMatrix34;
typedef CTMatrix< Float64, 2, 4 > TTestMatrix24;

GUCEF_MATH_STATIC_ASSERT( sizeof( TTestMatrix23 ) == 6 * sizeof( Float64 ), CTMatrix_has_no_overhead );
GUCEF_MATH_STATIC_ASSERT( sizeof( CTMatrix< Float32, 4, 4 > ) == 64, CTMatrix_4x4_of_Float32_is_64_bytes );

/*-------------------------------------------------------------------------*/

/** A uniformly distributed value of [low, high), drawn into a local by the caller (evaluation order) */
static Float64
Draw( MathReference::CSeededSequence& generator, Float64 low, Float64 high )
{
    return generator.NextFloat64( low, high );
}

/*-------------------------------------------------------------------------*/

/** Fixed-size products and conversions from a realtime context: a RealtimeSanitizer build reports any allocation */
static Float32
RealtimeGeneralMatrixWork( Float32 a ) GUCEF_MATH_REALTIME_CONTEXT
{
    const Float32 values[ 6 ] = { a, 2.0f, -1.0f, 0.5f, 3.0f, a };
    CTMatrix< Float32, 2, 3 > m = CTMatrix< Float32, 2, 3 >::FromRowMajor( values );
    CTMatrix< Float32, 3, 2 > t = m.Transposed();
    CTMatrix< Float32, 2, 2 > p = m * t;
    CTMatrix< Float32, 3, 3 > q = t.Multiply( m ) + CTMatrix< Float32, 3, 3 >::Identity();
    TFloat32Matrix3D m3 = q.ToMatrix3D();
    return p.GetTrace() + m3.GetDeterminant() + q.GetRow( 1 )( 0, 2 ) + q.GetColumn( 2 )( 1, 0 );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformGeneralMatricesTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "GeneralMatrices" );

    GUCEF_TESTFW_TESTCASE( "GeneralMatrices: CTMatrix storage, identity, rows, columns and transpose" )
    try
    {
        const Float64 rowMajor[ 6 ] = { 1, 2, 3, 4, 5, 6 };
        TTestMatrix23 m = TTestMatrix23::FromRowMajor( rowMajor );
        ASSERT_TRUE( 2 == TTestMatrix23::Rows && 3 == TTestMatrix23::Columns && 6 == TTestMatrix23::Count );
        ASSERT_TRUE( 1 == m( 0, 0 ) && 3 == m( 0, 2 ) && 4 == m( 1, 0 ) && 6 == m( 1, 2 ) );
        // column-major storage
        ASSERT_TRUE( 1 == m.GetData()[ 0 ] && 4 == m.GetData()[ 1 ] && 2 == m.GetData()[ 2 ] );
        Float64 back[ 6 ];
        m.ToRowMajor( back );
        ASSERT_TRUE( 0 == memcmp( back, rowMajor, sizeof( back ) ) );
        ASSERT_TRUE( m == TTestMatrix23::FromColumnMajor( m.GetData() ) );
        // the default is the zero matrix, Identity has ones on the diagonal also when rectangular
        TTestMatrix23 zero;
        ASSERT_TRUE( zero == TTestMatrix23::Zero() && 0 == zero( 1, 2 ) );
        TTestMatrix23 identity = TTestMatrix23::Identity();
        ASSERT_TRUE( 1 == identity( 0, 0 ) && 1 == identity( 1, 1 ) && 0 == identity( 0, 2 ) && 0 == identity( 1, 2 ) );
        ASSERT_TRUE( 2.0 == identity.GetTrace() && 6.0 == m.GetTrace() );
        // rows, columns, transpose
        CTMatrix< Float64, 1, 3 > row = m.GetRow( 1 );
        CTMatrix< Float64, 2, 1 > column = m.GetColumn( 2 );
        ASSERT_TRUE( 4 == row( 0, 0 ) && 6 == row( 0, 2 ) && 3 == column( 0, 0 ) && 6 == column( 1, 0 ) );
        TTestMatrix23 changed = m;
        changed.SetRow( 0, row );
        changed.SetColumn( 0, column );
        ASSERT_TRUE( 3 == changed( 0, 0 ) && 6 == changed( 1, 0 ) && 5 == changed( 0, 1 ) );
        CTMatrix< Float64, 3, 2 > transposed = m.Transposed();
        ASSERT_TRUE( 4 == transposed( 0, 1 ) && 3 == transposed( 2, 0 ) && m == transposed.Transposed() );
        // algebra
        ASSERT_TRUE( ( m + m ) == m * 2.0 && ( m - m ) == zero && -m == m * -1.0 && 2.0 * m == m * 2.0 );
        TTestMatrix23 accumulated = m;
        accumulated += m;
        accumulated -= m;
        ASSERT_TRUE( accumulated == m && accumulated.Equals( m ) && accumulated.IsFinite() );
        ASSERT_TRUE( m.Equals( m + TTestMatrix23::Identity() * 1e-13, 1e-12, 0.0 ) && !m.Equals( m + TTestMatrix23::Identity(), 1e-12, 0.0 ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GeneralMatrices: CTMatrix products against a brute-force reference, conversions to CTMatrix3D and CTMatrix4D" )
    try
    {
        const UInt64 seed = 0x4B01;
        MathReference::CSeededSequence generator( seed );
        UInt32 failures = 0;
        for ( UInt32 i=0; i<100; ++i )
        {
            TTestMatrix23 a;
            TTestMatrix34 b;
            for ( UInt32 r=0; r<2; ++r )
                for ( UInt32 c=0; c<3; ++c )
                {
                    const Float64 value = Draw( generator, -10, 10 );
                    a( r, c ) = value;
                }
            for ( UInt32 r=0; r<3; ++r )
                for ( UInt32 c=0; c<4; ++c )
                {
                    const Float64 value = Draw( generator, -10, 10 );
                    b( r, c ) = value;
                }
            TTestMatrix24 product = a * b;
            for ( UInt32 r=0; r<2; ++r )
                for ( UInt32 c=0; c<4; ++c )
                {
                    // the same order of summation: bit identical
                    Float64 sum = a( r, 0 ) * b( 0, c );
                    sum = sum + a( r, 1 ) * b( 1, c );
                    sum = sum + a( r, 2 ) * b( 2, c );
                    if ( !( sum == product( r, c ) ) )
                        ++failures;
                }
        }
        ASSERT_TRUE( 0 == failures );
        if ( 0 != failures )
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "GeneralMatrices: " << failures << " product elements differ, seed " << seed );

        // square conversions and the agreement with the fixed-size families
        TFloat64Matrix3D r3 = TFloat64Matrix3D::RotationAxisAngle( TFloat64DirectionVector3D( TFloat64Vector3D( 1, 2, 3 ) ), TFloat64Radian( 0.7 ) );
        CTMatrix< Float64, 3, 3 > g3( r3 );
        ASSERT_TRUE( g3.ToMatrix3D() == r3 );
        ASSERT_TRUE( ( g3 * g3 ).ToMatrix3D() == r3 * r3 );
        TFloat64Matrix4D r4 = TFloat64Matrix4D::FromTRS( TFloat64Vector3D( 1, 2, 3 ), TFloat64Quaternion::FromXYZW( 0.1, 0.2, 0.3, 0.9 ).Normalized(), TFloat64Vector3D( 2, 2, 2 ) );
        CTMatrix< Float64, 4, 4 > g4( r4 );
        // CTMatrix4D sums its 4 term dot products in its own order: equal within rounding
        ASSERT_TRUE( g4.ToMatrix4D() == r4 && ( g4 * g4 ).ToMatrix4D().Equals( r4 * r4, 1e-13, 1e-15 ) );
        TFloat64Matrix2D r2 = TFloat64Matrix2D::Rotation( TFloat64Radian( 0.3 ) );
        ASSERT_TRUE( ( CTMatrix< Float64, 2, 2 >( r2 ).ToMatrix2D() == r2 ) );
        // a column vector times a row vector is the outer product, a row times a column the dot product
        const Float64 u[ 3 ] = { 1, 2, 3 };
        CTMatrix< Float64, 3, 1 > column = CTMatrix< Float64, 3, 1 >::FromColumnMajor( u );
        CTMatrix< Float64, 3, 3 > outer = column * column.Transposed();
        CTMatrix< Float64, 1, 1 > inner = column.Transposed() * column;
        ASSERT_TRUE( 6 == outer( 1, 2 ) && 14 == inner( 0, 0 ) );
        // conversion between kinds
        CTMatrix< Float32, 2, 3 > narrowed( TTestMatrix23::Identity() * 0.5 );
        ASSERT_TRUE( 0.5f == narrowed( 1, 1 ) && 0.0f == narrowed( 1, 2 ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GeneralMatrices: CTMatrix in fixed point rounds once per element, integers wrap, fractions are exact" )
    try
    {
        // Q16.16 with raw values below 2^20: the exact raw sum of products fits a Float64 exactly, the reference rounds
        // it once, half away from zero
        const UInt64 seed = 0x4B02;
        MathReference::CSeededSequence generator( seed );
        UInt32 failures = 0;
        for ( UInt32 i=0; i<200; ++i )
        {
            CTMatrix< TFixed16_16, 2, 5 > a;
            CTMatrix< TFixed16_16, 5, 2 > b;
            for ( UInt32 k=0; k<10; ++k )
            {
                const Int64 rawA = generator.NextSignedWithRandomLength( 20 );
                const Int64 rawB = generator.NextSignedWithRandomLength( 20 );
                a( k % 2, k / 2 ) = TFixed16_16::FromRaw( (Int32) rawA );
                b( k / 2, k % 2 ) = TFixed16_16::FromRaw( (Int32) rawB );
            }
            CTMatrix< TFixed16_16, 2, 2 > product = a * b;
            for ( UInt32 r=0; r<2; ++r )
                for ( UInt32 c=0; c<2; ++c )
                {
                    Float64 sum = 0.0;
                    for ( UInt32 k=0; k<5; ++k )
                        sum += (Float64) a( r, k ).GetRaw() * (Float64) b( k, c ).GetRaw();
                    const Float64 scaled = sum / 65536.0;
                    const Float64 rounded = scaled < 0.0 ? -floor( -scaled + 0.5 ) : floor( scaled + 0.5 );
                    const Float64 clamped = rounded > 2147483647.0 ? 2147483647.0 : ( rounded < -2147483648.0 ? -2147483648.0 : rounded );
                    if ( !( (Float64) product( r, c ).GetRaw() == clamped ) )
                        ++failures;
                }
        }
        ASSERT_TRUE( 0 == failures );
        if ( 0 != failures )
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "GeneralMatrices: " << failures << " fixed point product elements are not rounded once, seed " << seed );

        // a fixed point product that saturates
        CTMatrix< TFixed16_16, 1, 2 > big;
        big( 0, 0 ) = TFixed16_16( 30000.0 );
        big( 0, 1 ) = TFixed16_16( 30000.0 );
        ASSERT_TRUE( TFixed16_16::Highest() == ( big * big.Transposed() )( 0, 0 ) );

        // integers wrap through the unsigned image: 100 * 3 = 300 = 44 modulo 256
        CTMatrix< Int8, 1, 1 > a8;
        a8( 0, 0 ) = 100;
        CTMatrix< Int8, 1, 1 > b8;
        b8( 0, 0 ) = 3;
        ASSERT_TRUE( 44 == ( a8 * b8 )( 0, 0 ) );
        CTMatrix< Int64, 2, 2 > i64 = CTMatrix< Int64, 2, 2 >::Identity() * (Int64) 3;
        ASSERT_TRUE( 9 == ( i64 * i64 )( 1, 1 ) && 0 == ( i64 * i64 )( 0, 1 ) );

        // exact fractions: ( 1/3 ) ( 3/7 ) + ( 1/2 ) ( 2/5 ) = 1/7 + 1/5 = 12/35
        CTMatrix< CORE::TInt64Fraction, 1, 2 > f;
        f( 0, 0 ) = CORE::TInt64Fraction( 1, 3 );
        f( 0, 1 ) = CORE::TInt64Fraction( 1, 2 );
        CTMatrix< CORE::TInt64Fraction, 2, 1 > g;
        g( 0, 0 ) = CORE::TInt64Fraction( 3, 7 );
        g( 1, 0 ) = CORE::TInt64Fraction( 2, 5 );
        ASSERT_TRUE( CORE::TInt64Fraction( 12, 35 ) == ( f * g )( 0, 0 ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GeneralMatrices: CTMatrix text and DataNode forms round trip; failures leave the matrix unchanged" )
    try
    {
        const Float64 rowMajor[ 6 ] = { 1.5, -2, 3, 0.25, 5, -6 };
        TTestMatrix23 m = TTestMatrix23::FromRowMajor( rowMajor );
        CORE::CString text = m.ToString();
        ASSERT_TRUE( text == "[[1.5, -2, 3], [0.25, 5, -6]]" );
        TTestMatrix23 parsed;
        ASSERT_TRUE( parsed.FromString( " [ [1.5, -2, 3] , [0.25,5,-6] ] " ) && parsed == m );
        TTestMatrix23 unchanged = m;
        ASSERT_FALSE( unchanged.FromString( "[[1, 2], [3, 4]]" ) );
        ASSERT_TRUE( unchanged == m );

        CORE::CDataNodeSerializableSettings settings;
        CORE::CDataNode node( "value", GUCEF_DATATYPE_OBJECT );
        ASSERT_TRUE( m.Serialize( node, settings ) );
        TTestMatrix23 restored;
        ASSERT_TRUE( restored.Deserialize( node, settings ) && restored == m );
        CORE::CDataNode incomplete( "value", GUCEF_DATATYPE_OBJECT );
        incomplete.SetAttribute( "m00", CORE::CVariant( 1.0 ) );
        ASSERT_FALSE( restored.Deserialize( incomplete, settings ) );
        ASSERT_TRUE( restored == m );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GeneralMatrices: CTDynamicVector sizes, element-wise operations and dimension checks" )
    try
    {
        typedef CTDynamicVector< Float64 > TVector;
        TVector empty;
        ASSERT_TRUE( empty.IsEmpty() && 0 == empty.GetSize() && GUCEF_NULL == empty.GetData() && "()" == empty.ToString() );
        ASSERT_TRUE( 0.0 == empty.GetLength() );
        const Float64 values[ 3 ] = { 3, 4, 12 };
        TVector v( values, 3 );
        ASSERT_TRUE( 3 == v.GetSize() && 4 == v[ 1 ] && 12 == v( 2 ) && 13.0 == v.GetLength() );
        ASSERT_TRUE( "(3, 4, 12)" == v.ToString() );
        Float64 dot = 0.0;
        ASSERT_TRUE( v.TryDot( v, dot ) && 169.0 == dot );
        TVector sum;
        ASSERT_TRUE( v.TryAdd( v, sum ) && 24 == sum[ 2 ] );
        ASSERT_TRUE( v.TrySubtract( v, sum ) && 0 == sum[ 0 ] && 3 == sum.GetSize() );
        // a size mismatch: false, output unchanged
        TVector two( 2 );
        TVector out( values, 3 );
        ASSERT_FALSE( v.TryAdd( two, out ) );
        ASSERT_FALSE( v.TrySubtract( two, out ) );
        dot = 7.0;
        ASSERT_FALSE( v.TryDot( two, dot ) );
        ASSERT_TRUE( 7.0 == dot && out == v );
        // resize keeps the elements, new ones are zero
        TVector grown = v;
        grown.Resize( 5 );
        ASSERT_TRUE( 5 == grown.GetSize() && 12 == grown[ 2 ] && 0 == grown[ 4 ] );
        grown.Resize( 2 );
        ASSERT_TRUE( 2 == grown.GetSize() && 4 == grown[ 1 ] );
        grown.Scale( 0.5 );
        ASSERT_TRUE( 2.0 == grown[ 1 ] );
        grown.SetZero();
        ASSERT_TRUE( 0.0 == grown[ 0 ] && grown != v && grown == TVector( 2 ) );
        ASSERT_TRUE( v.Equals( v, 0.0, 0.0 ) && !v.Equals( two, 1.0, 1.0 ) );
        // fixed-size column vectors
        CTMatrix< Float64, 3, 1 > column;
        ASSERT_TRUE( v.ToMatrix( column ) && 4 == column( 1, 0 ) );
        CTMatrix< Float64, 2, 1 > wrong;
        ASSERT_FALSE( v.ToMatrix( wrong ) );
        ASSERT_TRUE( TVector( column ) == v );
        // fixed point: the dot product rounds once
        CTDynamicVector< TFixed16_16 > fixed( 3 );
        fixed[ 0 ] = TFixed16_16::FromRaw( 1 );
        fixed[ 1 ] = TFixed16_16::FromRaw( 1 );
        fixed[ 2 ] = TFixed16_16::FromRaw( 65536 );
        TFixed16_16 fixedDot;
        // 2^-32 + 2^-32 + 1: rounded once to 1 (each square alone would round to 0 as well, but the sum is exact)
        ASSERT_TRUE( fixed.TryDot( fixed, fixedDot ) && 65536 == fixedDot.GetRaw() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GeneralMatrices: CTDynamicMatrix resize, blocks, products, transpose and dimension checks" )
    try
    {
        typedef CTDynamicMatrix< Float64 > TMatrix;
        typedef CTDynamicVector< Float64 > TVector;
        TMatrix empty;
        ASSERT_TRUE( empty.IsEmpty() && 0 == empty.GetRows() && "[]" == empty.ToString() );
        const Float64 rowMajor[ 6 ] = { 1, 2, 3, 4, 5, 6 };
        TMatrix m = TMatrix::FromRowMajor( rowMajor, 2, 3 );
        ASSERT_TRUE( 2 == m.GetRows() && 3 == m.GetColumns() && !m.IsSquare() && 6 == m( 1, 2 ) && 4 == m.GetData()[ 1 ] );
        ASSERT_TRUE( m == TMatrix( TTestMatrix23::FromRowMajor( rowMajor ) ) );
        ASSERT_TRUE( "[[1, 2, 3], [4, 5, 6]]" == m.ToString() );
        TTestMatrix23 fixedSize;
        ASSERT_TRUE( m.ToMatrix( fixedSize ) && fixedSize == TTestMatrix23::FromRowMajor( rowMajor ) );
        CTMatrix< Float64, 3, 2 > wrongShape;
        ASSERT_FALSE( m.ToMatrix( wrongShape ) );
        ASSERT_TRUE( m == TMatrix::FromColumnMajor( m.GetData(), 2, 3 ) );

        // resize keeps the overlapping block
        TMatrix resized = m;
        resized.Resize( 3, 2 );
        ASSERT_TRUE( 1 == resized( 0, 0 ) && 5 == resized( 1, 1 ) && 0 == resized( 2, 1 ) );
        resized.SetIdentity();
        ASSERT_TRUE( 1 == resized( 1, 1 ) && 0 == resized( 2, 1 ) && 0 == resized( 1, 0 ) );
        ASSERT_TRUE( TMatrix::Identity( 3 )( 2, 2 ) == 1.0 );

        // blocks
        TMatrix block;
        ASSERT_TRUE( m.GetBlock( 0, 1, 2, 2, block ) && 2 == block( 0, 0 ) && 6 == block( 1, 1 ) );
        ASSERT_FALSE( m.GetBlock( 1, 1, 2, 2, block ) );
        ASSERT_TRUE( 2 == block( 0, 0 ) );
        TMatrix target( 3, 3 );
        ASSERT_TRUE( target.SetBlock( 1, 1, block ) && 2 == target( 1, 1 ) && 6 == target( 2, 2 ) && 0 == target( 0, 0 ) );
        ASSERT_FALSE( target.SetBlock( 2, 2, block ) );
        TVector row;
        TVector column;
        ASSERT_TRUE( m.GetRow( 1, row ) && 3 == row.GetSize() && 5 == row[ 1 ] );
        ASSERT_TRUE( m.GetColumn( 2, column ) && 2 == column.GetSize() && 6 == column[ 1 ] );
        ASSERT_FALSE( m.GetRow( 2, row ) || m.GetColumn( 3, column ) );

        // products: ( 2 x 3 ) ( 3 x 2 ), dimension mismatch, aliasing of the output
        TMatrix t = m.Transposed();
        ASSERT_TRUE( 3 == t.GetRows() && 4 == t( 0, 1 ) );
        TMatrix p;
        ASSERT_TRUE( m.TryMultiply( t, p ) && 2 == p.GetRows() && 2 == p.GetColumns() && 14 == p( 0, 0 ) && 32 == p( 0, 1 ) && 77 == p( 1, 1 ) );
        TMatrix unchanged = p;
        ASSERT_FALSE( m.TryMultiply( m, unchanged ) );
        ASSERT_TRUE( unchanged == p );
        TMatrix self = m;
        ASSERT_TRUE( self.TryMultiply( t, self ) && self == p );
        const Float64 ones[ 3 ] = { 1, 1, 1 };
        TVector v( ones, 3 );
        TVector mv;
        ASSERT_TRUE( m.TryMultiply( v, mv ) && 2 == mv.GetSize() && 6 == mv[ 0 ] && 15 == mv[ 1 ] );
        ASSERT_FALSE( t.TryMultiply( v, mv ) );
        // element-wise operations
        TMatrix sum;
        ASSERT_TRUE( m.TryAdd( m, sum ) && 12 == sum( 1, 2 ) );
        ASSERT_TRUE( sum.TrySubtract( m, sum ) && sum == m );
        ASSERT_FALSE( m.TryAdd( t, sum ) || m.TrySubtract( t, sum ) );
        sum.Scale( 2.0 );
        ASSERT_TRUE( 12 == sum( 1, 2 ) && sum.Equals( sum, 0.0, 0.0 ) && !sum.Equals( m, 0.0, 0.0 ) && sum != m );
        sum.SetZero();
        ASSERT_TRUE( sum == TMatrix( 2, 3 ) );
        // a 0-column product is the zero matrix
        TMatrix wide( 2, 0 );
        TMatrix tall( 0, 3 );
        TMatrix zeroProduct;
        ASSERT_TRUE( wide.TryMultiply( tall, zeroProduct ) && 2 == zeroProduct.GetRows() && 3 == zeroProduct.GetColumns() && 0 == zeroProduct( 1, 2 ) );
        // the general product agrees with the fixed-size family within rounding
        TFloat64Matrix4D r4 = TFloat64Matrix4D::FromTRS( TFloat64Vector3D( 1, 2, 3 ), TFloat64Quaternion::FromXYZW( 0.1, 0.2, 0.3, 0.9 ).Normalized(), TFloat64Vector3D( 2, 2, 2 ) );
        TMatrix d4 = TMatrix::FromColumnMajor( r4.GetData(), 4, 4 );
        TMatrix d44;
        ASSERT_TRUE( d4.TryMultiply( d4, d44 ) && d44.Equals( TMatrix::FromColumnMajor( ( r4 * r4 ).GetData(), 4, 4 ), 1e-13, 1e-15 ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GeneralMatrices: realtime-safe operations run in a realtime context" )
    try
    {
        Float32 result = RealtimeGeneralMatrixWork( 1.5f );
        ASSERT_TRUE( CTNumericTraits< Float32 >::IsFinite( result ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GeneralMatrices: realtime-safe and right-sized operations do not allocate" )
    GUCEF_TESTFW_SKIP_IF( !MathAllocationCounter::IsAvailable(), "no allocation counter with a sanitizer that replaces the allocator (the RealtimeSanitizer build proves the same in a realtime context)" );
    try
    {
        // the counter counts: one probe allocation
        MathAllocationCounter::StartCounting();
        MathAllocationCounter::AllocateProbe();
        ASSERT_TRUE( 1 == MathAllocationCounter::StopCounting() );
        typedef CTDynamicMatrix< Float64 > TMatrix;
        typedef CTDynamicVector< Float64 > TVector;
        const Float64 rowMajor[ 6 ] = { 1, 2, 3, 4, 5, 6 };
        const TMatrix a = TMatrix::FromRowMajor( rowMajor, 2, 3 );
        const TMatrix t = a.Transposed();
        TMatrix product( 2, 2 );
        TMatrix sum( 2, 3 );
        const Float64 ones[ 3 ] = { 1, 1, 1 };
        const TVector v( ones, 3 );
        TVector mv( 2 );
        TVector vsum( 3 );
        Float64 dot = 0.0;
        MathAllocationCounter::StartCounting();
        Float32 result = RealtimeGeneralMatrixWork( 1.5f );
        for ( UInt32 i=0; i<10; ++i )
        {
            a.TryMultiply( t, product );
            a.TryAdd( a, sum );
            sum.TrySubtract( a, sum );
            a.TryMultiply( v, mv );
            v.TryAdd( v, vsum );
            v.TryDot( v, dot );
        }
        const UInt32 allocations = MathAllocationCounter::StopCounting();
        ASSERT_TRUE( 0 == allocations && CTNumericTraits< Float32 >::IsFinite( result ) );
        ASSERT_TRUE( 77 == product( 1, 1 ) && 6 == sum( 1, 2 ) && 15 == mv[ 1 ] && 3.0 == dot );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
