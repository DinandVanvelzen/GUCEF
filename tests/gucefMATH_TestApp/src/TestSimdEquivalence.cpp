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
#include <string.h>

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

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

#ifndef GUCEF_MATH_MATRICES_H
#include "gucefMATH_matrices.h"
#define GUCEF_MATH_MATRICES_H
#endif /* GUCEF_MATH_MATRICES_H ? */

#ifndef GUCEF_MATH_QUATERNIONS_H
#include "gucefMATH_quaternions.h"
#define GUCEF_MATH_QUATERNIONS_H
#endif /* GUCEF_MATH_QUATERNIONS_H ? */

#if ( GUCEF_MATH_SIMD_HAVE_SSE2 == 1 )
  // the lane generic implementation of the NEON and WASM128 backends, instantiated over the SSE2 lane policy so that
  // it runs (and is checked) on every x86 machine as well
  #include "gucefMATH_CTVector4Ops_Lanes.h"
  #include "gucefMATH_CTQuaternionOps_Lanes.h"
  #include "gucefMATH_CTMatrix4Ops_Lanes.h"
#endif

#include "TestMathReference.h"
#include "TestSimdEquivalence.h"

#if ( GUCEF_MATH_SIMD_HAVE_SSE2 == 1 )

namespace GUCEF {
namespace MATH {

/** A backend tag of this test: the lane generic operations over the SSE2 lane policy */
struct SMathSimdTestLanes {};

template <> class CTVector4Ops< Float32, SMathSimdTestLanes > : public CTVector4OpsLanes< CMathSse2 > {};
template <> class CTQuaternionOps< Float32, SMathSimdTestLanes > : public CTQuaternionOpsLanes< CMathSse2 > {};
template <> class CTMatrix4Ops< Float32, SMathSimdTestLanes > : public CTMatrix4OpsLanes< CMathSse2 > {};

}  /* namespace MATH */
}  /* namespace GUCEF */

#endif

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

namespace {

const UInt64 EquivalenceSeed = 0x5EED5A1D0005A2ULL;
const UInt32 SampleCount = 1000000;

enum EOperation
{
    OP_VECTOR4_ADD = 0,
    OP_VECTOR4_SUBTRACT,
    OP_VECTOR4_NEGATE,
    OP_VECTOR4_MULTIPLY,
    OP_VECTOR4_DOT,
    OP_VECTOR4_MIN,
    OP_VECTOR4_MAX,
    OP_VECTOR4_NORMALIZE,
    OP_QUATERNION_MULTIPLY,
    OP_QUATERNION_ROTATE,
    OP_QUATERNION_DOT,
    OP_QUATERNION_NORMALIZE,
    OP_MATRIX4_MULTIPLY,
    OP_MATRIX4_TRANSFORM,
    OP_MATRIX4_TRANSPOSE,
    OP_COUNT
};

const char* const OperationNames[ OP_COUNT ] =
{
    "Vector4.Add", "Vector4.Subtract", "Vector4.Negate", "Vector4.Multiply", "Vector4.Dot", "Vector4.Min", "Vector4.Max",
    "Vector4.Normalize", "Quaternion.Multiply", "Quaternion.Rotate", "Quaternion.Dot", "Quaternion.Normalize",
    "Matrix4.Multiply", "Matrix4.Transform", "Matrix4.Transpose"
};

/** The result of one operation: up to 16 values and a success flag */
struct SResult
{
    Float32 values[ 16 ];
    UInt32 valueCount;
    UInt32 flag;
};

/*-------------------------------------------------------------------------*/

Float32
FromBits( UInt32 bits )
{
    Float32 value;
    memcpy( &value, &bits, sizeof( value ) );
    return value;
}

/*-------------------------------------------------------------------------*/

/**
 *  A test value: mostly ordinary magnitudes, a fifth any finite bit pattern (subnormals and values that overflow in
 *  products), the rest special values (signed zeros, infinities, the limits). No NaN inputs: which of two NaN operands
 *  propagates is not part of the bit identity, NaN created by the operations is.
 */
Float32
NextTestFloat( MathReference::CSeededSequence& sequence )
{
    const UInt32 kind = sequence.NextUInt32() % 100;
    const UInt32 bits = sequence.NextUInt32();
    const Float64 ordinary = sequence.NextFloat64( -100.0, 100.0 );
    if ( kind < 70 )
        return (Float32) ordinary;
    if ( kind < 90 )
    {
        UInt32 finiteBits = bits;
        if ( 0x7F800000 == ( finiteBits & 0x7F800000 ) )
            finiteBits &= 0xFF7FFFFF;
        return FromBits( finiteBits );
    }
    const Float32 specials[ 10 ] = { 0.0f, -0.0f, FromBits( 0x7F800000 ), FromBits( 0xFF800000 ), FLT_MAX, -FLT_MAX, FLT_MIN, FromBits( 0x00000001 ), 1.0f, -1.0f };
    return specials[ bits % 10 ];
}

/*-------------------------------------------------------------------------*/

void
SetResult( SResult& result, const Float32* values, UInt32 count, UInt32 flag )
{
    memset( &result, 0, sizeof( result ) );
    memcpy( result.values, values, count * sizeof( Float32 ) );
    result.valueCount = count;
    result.flag = flag;
}

/*-------------------------------------------------------------------------*/

/** One operation through the operation policies of the given backend */
template < typename TBackend >
void
RunOperation( UInt32 operation, const Float32 ( &a )[ 16 ], const Float32 ( &b )[ 16 ], SResult& result )
{
    typedef CTVector4Ops< Float32, TBackend > TVectorOps;
    typedef CTQuaternionOps< Float32, TBackend > TQuaternionOps;
    typedef CTMatrix4Ops< Float32, TBackend > TMatrixOps;

    const TFloat32Vector4D va( a[ 0 ], a[ 1 ], a[ 2 ], a[ 3 ] );
    const TFloat32Vector4D vb( b[ 0 ], b[ 1 ], b[ 2 ], b[ 3 ] );
    const Float32 qa[ 4 ] = { a[ 0 ], a[ 1 ], a[ 2 ], a[ 3 ] };
    const Float32 qb[ 4 ] = { b[ 0 ], b[ 1 ], b[ 2 ], b[ 3 ] };
    TFloat32Vector4D v;
    Float32 q[ 4 ] = { 0.0f, 0.0f, 0.0f, 0.0f };
    Float32 m[ 16 ];
    switch ( operation )
    {
        case OP_VECTOR4_ADD: TVectorOps::Add( va, vb, v ); SetResult( result, &v.x, 4, 0 ); break;
        case OP_VECTOR4_SUBTRACT: TVectorOps::Subtract( va, vb, v ); SetResult( result, &v.x, 4, 0 ); break;
        case OP_VECTOR4_NEGATE: TVectorOps::Negate( va, v ); SetResult( result, &v.x, 4, 0 ); break;
        case OP_VECTOR4_MULTIPLY: TVectorOps::Multiply( va, b[ 0 ], v ); SetResult( result, &v.x, 4, 0 ); break;
        case OP_VECTOR4_DOT: { const Float32 d = TVectorOps::Dot( va, vb ); SetResult( result, &d, 1, 0 ); break; }
        case OP_VECTOR4_MIN: TVectorOps::Min( va, vb, v ); SetResult( result, &v.x, 4, 0 ); break;
        case OP_VECTOR4_MAX: TVectorOps::Max( va, vb, v ); SetResult( result, &v.x, 4, 0 ); break;
        case OP_VECTOR4_NORMALIZE: TVectorOps::Normalize( va, v ); SetResult( result, &v.x, 4, 0 ); break;
        case OP_QUATERNION_MULTIPLY: TQuaternionOps::Multiply( qa, qb, q ); SetResult( result, q, 4, 0 ); break;
        case OP_QUATERNION_ROTATE:
        {
            const Float32 in[ 3 ] = { b[ 0 ], b[ 1 ], b[ 2 ] };
            Float32 out[ 3 ];
            TQuaternionOps::Rotate( qa, in, out );
            SetResult( result, out, 3, 0 );
            break;
        }
        case OP_QUATERNION_DOT: { const Float32 d = TQuaternionOps::Dot( qa, qb ); SetResult( result, &d, 1, 0 ); break; }
        case OP_QUATERNION_NORMALIZE: { const bool ok = TQuaternionOps::Normalize( qa, q ); SetResult( result, q, 4, ok ? 1 : 0 ); break; }
        case OP_MATRIX4_MULTIPLY: TMatrixOps::Multiply( a, b, m ); SetResult( result, m, 16, 0 ); break;
        case OP_MATRIX4_TRANSFORM:
        {
            const Float32 in[ 4 ] = { b[ 0 ], b[ 1 ], b[ 2 ], b[ 3 ] };
            Float32 out[ 4 ];
            TMatrixOps::Transform( a, in, out );
            SetResult( result, out, 4, 0 );
            break;
        }
        case OP_MATRIX4_TRANSPOSE: TMatrixOps::Transpose( a, m ); SetResult( result, m, 16, 0 ); break;
        default: break;
    }
}

/*-------------------------------------------------------------------------*/

/** One operation through the value types (operators and members, the default backend of the build) */
void
RunValueTypeOperation( UInt32 operation, const Float32 ( &a )[ 16 ], const Float32 ( &b )[ 16 ], SResult& result )
{
    const TFloat32Vector4D va( a[ 0 ], a[ 1 ], a[ 2 ], a[ 3 ] );
    const TFloat32Vector4D vb( b[ 0 ], b[ 1 ], b[ 2 ], b[ 3 ] );
    const TFloat32Quaternion qa = TFloat32Quaternion::FromXYZW( a[ 0 ], a[ 1 ], a[ 2 ], a[ 3 ] );
    const TFloat32Quaternion qb = TFloat32Quaternion::FromXYZW( b[ 0 ], b[ 1 ], b[ 2 ], b[ 3 ] );
    const TFloat32Matrix4D ma = TFloat32Matrix4D::FromColumnMajor( a );
    const TFloat32Matrix4D mb = TFloat32Matrix4D::FromColumnMajor( b );
    TFloat32Vector4D v;
    switch ( operation )
    {
        case OP_VECTOR4_ADD: v = va + vb; SetResult( result, &v.x, 4, 0 ); break;
        case OP_VECTOR4_SUBTRACT: v = va - vb; SetResult( result, &v.x, 4, 0 ); break;
        case OP_VECTOR4_NEGATE: v = -va; SetResult( result, &v.x, 4, 0 ); break;
        case OP_VECTOR4_MULTIPLY: v = va * b[ 0 ]; SetResult( result, &v.x, 4, 0 ); break;
        case OP_VECTOR4_DOT: { const Float32 d = va.Dot( vb ); SetResult( result, &d, 1, 0 ); break; }
        case OP_VECTOR4_MIN: v = va.Min( vb ); SetResult( result, &v.x, 4, 0 ); break;
        case OP_VECTOR4_MAX: v = va.Max( vb ); SetResult( result, &v.x, 4, 0 ); break;
        case OP_VECTOR4_NORMALIZE: v = va.Normalized(); SetResult( result, &v.x, 4, 0 ); break;
        case OP_QUATERNION_MULTIPLY:
        {
            const TFloat32Quaternion p = TFloat32Quaternion::Multiply( qa, qb );
            const Float32 values[ 4 ] = { p.x, p.y, p.z, p.w };
            SetResult( result, values, 4, 0 );
            break;
        }
        case OP_QUATERNION_ROTATE:
        {
            const TFloat32Vector3D r = qa.Rotate( TFloat32Vector3D( b[ 0 ], b[ 1 ], b[ 2 ] ) );
            const Float32 values[ 3 ] = { r.x, r.y, r.z };
            SetResult( result, values, 3, 0 );
            break;
        }
        case OP_QUATERNION_DOT: { const Float32 d = qa.Dot( qb ); SetResult( result, &d, 1, 0 ); break; }
        case OP_QUATERNION_NORMALIZE:
        {
            const TFloat32Quaternion n = qa.Normalized();
            const Float32 values[ 4 ] = { n.x, n.y, n.z, n.w };
            SetResult( result, values, 4, 0 );
            break;
        }
        case OP_MATRIX4_MULTIPLY: SetResult( result, ma.Multiply( mb ).GetData(), 16, 0 ); break;
        case OP_MATRIX4_TRANSFORM: v = ma.Transform( vb ); SetResult( result, &v.x, 4, 0 ); break;
        case OP_MATRIX4_TRANSPOSE: SetResult( result, ma.Transposed().GetData(), 16, 0 ); break;
        default: break;
    }
}

/*-------------------------------------------------------------------------*/

bool
IsNaNBits( UInt32 bits )
{
    return 0x7F800000 == ( bits & 0x7F800000 ) && 0 != ( bits & 0x007FFFFF );
}

/**
 *  Bit identity of two results. A NaN created by the arithmetic is NaN with every backend, but its sign and payload
 *  are not part of the bit identity: compilers may rewrite ( -a ) b as -( a b ), which flips the sign of a NaN.
 */
bool
SameResult( const SResult& a, const SResult& b )
{
    if ( a.valueCount != b.valueCount || a.flag != b.flag )
        return false;
    for ( UInt32 i=0; i<a.valueCount; ++i )
    {
        UInt32 x = 0;
        UInt32 y = 0;
        memcpy( &x, &a.values[ i ], sizeof( x ) );
        memcpy( &y, &b.values[ i ], sizeof( y ) );
        if ( x != y && !( IsNaNBits( x ) && IsNaNBits( y ) ) )
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

/** Hash of a result with every NaN as one quiet NaN */
UInt64
HashResult( const SResult& result, UInt64 hash )
{
    for ( UInt32 i=0; i<result.valueCount; ++i )
    {
        UInt32 bits = 0;
        memcpy( &bits, &result.values[ i ], sizeof( bits ) );
        if ( IsNaNBits( bits ) )
            bits = 0x7FC00000;
        hash = MathReference::Fnv1a( &bits, sizeof( bits ), hash );
    }
    return MathReference::Fnv1aInt64( (Int64) result.flag, hash );
}

/*-------------------------------------------------------------------------*/

void
DrawSample( MathReference::CSeededSequence& sequence, Float32 ( &a )[ 16 ], Float32 ( &b )[ 16 ] )
{
    for ( UInt32 i=0; i<16; ++i )
    {
        const Float32 value = NextTestFloat( sequence );
        a[ i ] = value;
    }
    for ( UInt32 i=0; i<16; ++i )
    {
        const Float32 value = NextTestFloat( sequence );
        b[ i ] = value;
    }
}

/*-------------------------------------------------------------------------*/

/** Seconds per call of 'operation' over 'iterations' calls on a ring of samples, with the given backend */
template < typename TBackend >
Float64
TimeOperation( UInt32 operation, const Float32 ( *samples )[ 16 ], UInt32 sampleCount, UInt32 iterations )
{
    SResult result;
    volatile Float32 sink = 0.0f;
    const UInt64 start = MT::PrecisionTickCount();
    for ( UInt32 i=0; i<iterations; ++i )
    {
        RunOperation< TBackend >( operation, samples[ i % sampleCount ], samples[ ( i + 1 ) % sampleCount ], result );
        sink = sink + result.values[ 0 ];
    }
    const UInt64 end = MT::PrecisionTickCount();
    return (Float64) ( end - start ) / (Float64) MT::PrecisionTimerResolution() / (Float64) iterations;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformSimdEquivalenceTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "SimdEquivalence" );

    GUCEF_TESTFW_TESTCASE( "SimdEquivalence: the configuration selects one backend for the value types" )
    try
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "SimdEquivalence: GUCEF_MATH_SIMD_LEVEL " << (UInt32) GUCEF_MATH_SIMD_LEVEL << ", SSE2 " << (UInt32) GUCEF_MATH_SIMD_HAVE_SSE2
                  << ", batch AVX2 " << (UInt32) GUCEF_MATH_SIMD_BATCH_ALLOW_AVX2 << ", batch F16C " << (UInt32) GUCEF_MATH_SIMD_BATCH_ALLOW_F16C
                  << ", batch max tier " << (UInt32) GUCEF_MATH_BATCH_MAX_TIER_LEVEL );
        ASSERT_TRUE( GUCEF_MATH_SIMD_HAVE_SSE2 + GUCEF_MATH_SIMD_HAVE_NEON + GUCEF_MATH_SIMD_HAVE_WASM128 <= 1 );
        ASSERT_TRUE( 0 == GUCEF_MATH_SIMD_BATCH_ALLOW_FMA );
        #if ( GUCEF_MATH_SIMD_LEVEL == GUCEF_MATH_SIMD_NONE )
        ASSERT_TRUE( 0 == GUCEF_MATH_SIMD_HAVE_SSE2 && 0 == GUCEF_MATH_SIMD_BATCH_ALLOW_AVX2 && 0 == GUCEF_MATH_SIMD_BATCH_ALLOW_F16C );
        ASSERT_TRUE( sizeof( TMathSimdDefaultBackend ) == sizeof( SMathSimdScalar ) );
        #endif
        #if defined( __x86_64__ ) || defined( _M_X64 )
        #if !defined( GUCEF_MATH_SIMD )
        ASSERT_TRUE( 1 == GUCEF_MATH_SIMD_HAVE_SSE2 );       // x86-64 always has SSE2 unless a build caps it
        #endif
        #endif
        // the layouts the specialisations rely on
        ASSERT_TRUE( 16 == sizeof( TFloat32Vector4D ) && 16 == sizeof( TFloat32Quaternion ) && 64 == sizeof( TFloat32Matrix4D ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "SimdEquivalence: 10^6 seeded inputs per operation, the build's backend equals the scalar primary templates bit for bit" )
    try
    {
        MathReference::CSeededSequence sequence( EquivalenceSeed );
        UInt64 hashes[ OP_COUNT ];
        UInt32 mismatches[ OP_COUNT ];
        for ( UInt32 op=0; op<OP_COUNT; ++op )
        {
            hashes[ op ] = MathReference::Fnv1aOffsetBasis();
            mismatches[ op ] = 0;
        }
        Float32 a[ 16 ];
        Float32 b[ 16 ];
        for ( UInt32 i=0; i<SampleCount; ++i )
        {
            DrawSample( sequence, a, b );
            for ( UInt32 op=0; op<OP_COUNT; ++op )
            {
                SResult simd;
                SResult scalar;
                RunOperation< TMathSimdDefaultBackend >( op, a, b, simd );
                RunOperation< SMathSimdScalar >( op, a, b, scalar );
                if ( !SameResult( simd, scalar ) )
                {
                    if ( 0 == mismatches[ op ] )
                        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "SimdEquivalence: " << OperationNames[ op ] << " differs at sample " << i << " (seed " << CORE::ToString( EquivalenceSeed ) << ")" );
                    ++mismatches[ op ];
                }
                hashes[ op ] = HashResult( simd, hashes[ op ] );
            }
        }
        UInt64 total = MathReference::Fnv1aOffsetBasis();
        for ( UInt32 op=0; op<OP_COUNT; ++op )
        {
            ASSERT_TRUE( 0 == mismatches[ op ] );
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "SimdEquivalence hash ops " << OperationNames[ op ] << " " << CORE::ToString( hashes[ op ] ) );
            total = MathReference::Fnv1aInt64( (Int64) hashes[ op ], total );
        }
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "SimdEquivalence hash ops total " << CORE::ToString( total ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "SimdEquivalence: the lane generic operations of the NEON and WASM128 backends equal the scalar primary templates (instantiated over SSE2 where available)" )
    try
    {
        #if ( GUCEF_MATH_SIMD_HAVE_SSE2 == 1 )
        MathReference::CSeededSequence sequence( EquivalenceSeed ^ 0x1A4E5 );
        UInt32 mismatches[ OP_COUNT ];
        for ( UInt32 op=0; op<OP_COUNT; ++op )
            mismatches[ op ] = 0;
        Float32 a[ 16 ];
        Float32 b[ 16 ];
        for ( UInt32 i=0; i<SampleCount; ++i )
        {
            DrawSample( sequence, a, b );
            for ( UInt32 op=0; op<OP_COUNT; ++op )
            {
                SResult lanes;
                SResult scalar;
                RunOperation< SMathSimdTestLanes >( op, a, b, lanes );
                RunOperation< SMathSimdScalar >( op, a, b, scalar );
                if ( !SameResult( lanes, scalar ) )
                {
                    if ( 0 == mismatches[ op ] )
                        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "SimdEquivalence: lane generic " << OperationNames[ op ] << " differs at sample " << i );
                    ++mismatches[ op ];
                }
            }
        }
        for ( UInt32 op=0; op<OP_COUNT; ++op )
            ASSERT_TRUE( 0 == mismatches[ op ] );
        #else
        // NEON and WASM128 builds run the lane generic operations as their backend in the test above
        ASSERT_TRUE( 1 == GUCEF_MATH_SIMD_HAVE_NEON + GUCEF_MATH_SIMD_HAVE_WASM128 + (int) ( GUCEF_MATH_SIMD_LEVEL == GUCEF_MATH_SIMD_NONE ) );
        #endif
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "SimdEquivalence: hashes of the value-type results for the comparison with the GUCEF_MATH_SIMD=NONE build" )
    try
    {
        MathReference::CSeededSequence sequence( EquivalenceSeed );
        UInt64 hashes[ OP_COUNT ];
        for ( UInt32 op=0; op<OP_COUNT; ++op )
            hashes[ op ] = MathReference::Fnv1aOffsetBasis();
        Float32 a[ 16 ];
        Float32 b[ 16 ];
        for ( UInt32 i=0; i<SampleCount; ++i )
        {
            DrawSample( sequence, a, b );
            for ( UInt32 op=0; op<OP_COUNT; ++op )
            {
                SResult result;
                RunValueTypeOperation( op, a, b, result );
                hashes[ op ] = HashResult( result, hashes[ op ] );
            }
        }
        UInt64 total = MathReference::Fnv1aOffsetBasis();
        for ( UInt32 op=0; op<OP_COUNT; ++op )
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "SimdEquivalence hash values " << OperationNames[ op ] << " " << CORE::ToString( hashes[ op ] ) );
            total = MathReference::Fnv1aInt64( (Int64) hashes[ op ], total );
        }
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "SimdEquivalence hash values total " << CORE::ToString( total ) );
        ASSERT_TRUE( MathReference::Fnv1aOffsetBasis() != total );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "SimdEquivalence: value-type benchmarks, scalar against the build's backend (opt-in: GUCEF_TEST_MATH_BENCH=1)" )
    GUCEF_TESTFW_REQUIRE_ENV( bench, "GUCEF_TEST_MATH_BENCH" );
    GUCEF_TESTFW_SKIP_IF( bench != "1", "GUCEF_TEST_MATH_BENCH is not 1" );
    try
    {
        const UInt32 ringSize = 1024;
        const UInt32 iterations = 4000000;
        GUCEF::vector< Float32 > storage( ringSize * 16 );
        MathReference::CSeededSequence sequence( EquivalenceSeed );
        for ( UInt32 i=0; i<ringSize * 16; ++i )
        {
            const Float64 value = sequence.NextFloat64( -2.0, 2.0 );
            storage[ i ] = (Float32) value;
        }
        const Float32 ( *samples )[ 16 ] = reinterpret_cast< const Float32 ( * )[ 16 ] >( &storage[ 0 ] );
        const UInt32 benchmarked[ 4 ] = { OP_MATRIX4_MULTIPLY, OP_QUATERNION_ROTATE, OP_VECTOR4_DOT, OP_MATRIX4_TRANSFORM };
        for ( UInt32 k=0; k<4; ++k )
        {
            const UInt32 op = benchmarked[ k ];
            const Float64 scalarSeconds = TimeOperation< SMathSimdScalar >( op, samples, ringSize, iterations );
            const Float64 simdSeconds = TimeOperation< TMathSimdDefaultBackend >( op, samples, ringSize, iterations );
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "SimdEquivalence bench " << OperationNames[ op ] << ": scalar " << CORE::ToString( scalarSeconds * 1.0e9 )
                      << " ns, build backend " << CORE::ToString( simdSeconds * 1.0e9 ) << " ns per call" );
        }
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
