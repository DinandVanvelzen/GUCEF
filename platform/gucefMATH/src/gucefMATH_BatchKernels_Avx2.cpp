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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <string.h>
#include <float.h>

#ifndef GUCEF_MATH_VECTORS_H
#include "gucefMATH_vectors.h"
#define GUCEF_MATH_VECTORS_H
#endif /* GUCEF_MATH_VECTORS_H ? */

#include "gucefMATH_BatchKernels.h"

#if ( GUCEF_MATH_SIMD_BATCH_ALLOW_AVX2 == 1 )
  #include <immintrin.h>
#endif

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

#if ( GUCEF_MATH_SIMD_BATCH_ALLOW_AVX2 == 1 )

/*
 *  AVX2 providers, compiled with the target attribute (GUCEF_MATH_SIMD_TARGET_AVX2) on every function of this file
 *  instead of a per-file compiler flag, and registered only when the CPU and the operating system support AVX2. No FMA:
 *  each lane is the canonical order of the scalar element operation, as in the SSE2 providers, with two elements per
 *  256 bit register (transforms, matrix products) or eight in structure of arrays form (dot products, normalisation).
 *  Tails go through the scalar element operations.
 */
namespace {

typedef CTMathComponentTraits< Float32 >::TKernels TKernels;

/*-------------------------------------------------------------------------*/

GUCEF_MATH_SIMD_TARGET_AVX2 GUCEF_MATH_FORCE_INLINE __m128
LoadElement3( const UInt8* p, bool last ) GUCEF_MATH_REALTIME_CONTEXT
{
    if ( !last )
        return _mm_loadu_ps( reinterpret_cast< const Float32* >( p ) );
    Float32 t[ 4 ];
    memcpy( t, p, 3 * sizeof( Float32 ) );
    t[ 3 ] = 0.0f;
    return _mm_loadu_ps( t );
}

/*-------------------------------------------------------------------------*/

GUCEF_MATH_SIMD_TARGET_AVX2 GUCEF_MATH_FORCE_INLINE void
StoreElement3( UInt8* p, __m128 v ) GUCEF_MATH_REALTIME_CONTEXT
{
    Float32 t[ 4 ];
    _mm_storeu_ps( t, v );
    memcpy( p, t, 3 * sizeof( Float32 ) );
}

/*-------------------------------------------------------------------------*/

GUCEF_MATH_SIMD_TARGET_AVX2 GUCEF_MATH_FORCE_INLINE __m256
Pair( __m128 low, __m128 high ) GUCEF_MATH_REALTIME_CONTEXT
{
    return _mm256_insertf128_ps( _mm256_castps128_ps256( low ), high, 1 );
}

/*-------------------------------------------------------------------------*/

GUCEF_MATH_SIMD_TARGET_AVX2 GUCEF_MATH_FORCE_INLINE __m256
Broadcast4( const Float32* p ) GUCEF_MATH_REALTIME_CONTEXT
{
    const __m128 v = _mm_loadu_ps( p );
    return Pair( v, v );
}

/*-------------------------------------------------------------------------*/

/** Per lane ( c0 x + c2 z ) + ( c1 y + c3 w ) */
GUCEF_MATH_SIMD_TARGET_AVX2 GUCEF_MATH_FORCE_INLINE __m256
Combine8( __m256 c0, __m256 c1, __m256 c2, __m256 c3, __m256 x, __m256 y, __m256 z, __m256 w ) GUCEF_MATH_REALTIME_CONTEXT
{
    GUCEF_MATH_FP_CONTRACT_OFF
    return _mm256_add_ps( _mm256_add_ps( _mm256_mul_ps( c0, x ), _mm256_mul_ps( c2, z ) ), _mm256_add_ps( _mm256_mul_ps( c1, y ), _mm256_mul_ps( c3, w ) ) );
}

/*-------------------------------------------------------------------------*/

/** Transposes the 4 x 4 blocks of both 128 bit halves: rows [ e0 | e4 ] .. [ e3 | e7 ] become x, y, z, w of e0..e7 */
GUCEF_MATH_SIMD_TARGET_AVX2 GUCEF_MATH_FORCE_INLINE void
Transpose4x8( __m256& r0, __m256& r1, __m256& r2, __m256& r3 ) GUCEF_MATH_REALTIME_CONTEXT
{
    const __m256 t0 = _mm256_unpacklo_ps( r0, r1 );
    const __m256 t1 = _mm256_unpackhi_ps( r0, r1 );
    const __m256 t2 = _mm256_unpacklo_ps( r2, r3 );
    const __m256 t3 = _mm256_unpackhi_ps( r2, r3 );
    r0 = _mm256_shuffle_ps( t0, t2, _MM_SHUFFLE( 1, 0, 1, 0 ) );
    r1 = _mm256_shuffle_ps( t0, t2, _MM_SHUFFLE( 3, 2, 3, 2 ) );
    r2 = _mm256_shuffle_ps( t1, t3, _MM_SHUFFLE( 1, 0, 1, 0 ) );
    r3 = _mm256_shuffle_ps( t1, t3, _MM_SHUFFLE( 3, 2, 3, 2 ) );
}

/*-------------------------------------------------------------------------*/

/** Elements first .. first + 7 as x, y, z of 8 lanes; the last one of the array is read with an exact 12 byte load */
GUCEF_MATH_SIMD_TARGET_AVX2 GUCEF_MATH_FORCE_INLINE void
LoadSoA8( const UInt8* base, UInt32 stride, UInt32 first, UInt32 count, __m256& x, __m256& y, __m256& z ) GUCEF_MATH_REALTIME_CONTEXT
{
    __m256 r[ 4 ];
    for ( UInt32 k=0; k<4; ++k )
    {
        const __m128 low = LoadElement3( base + (size_t) ( first + k ) * stride, first + k + 1 == count );
        const __m128 high = LoadElement3( base + (size_t) ( first + k + 4 ) * stride, first + k + 5 == count );
        r[ k ] = Pair( low, high );
    }
    Transpose4x8( r[ 0 ], r[ 1 ], r[ 2 ], r[ 3 ] );
    x = r[ 0 ];
    y = r[ 1 ];
    z = r[ 2 ];
}

/*-------------------------------------------------------------------------*/

template < int W >
GUCEF_MATH_SIMD_TARGET_AVX2 TMathStatus
Avx2Transform3( void* /* context */, const SMathTransformPoints3F32Args& args ) GUCEF_MATH_REALTIME_CONTEXT
{
    Float32 m[ 16 ];
    memcpy( m, args.matrix->GetData(), sizeof( m ) );
    const __m256 c0 = Broadcast4( m );
    const __m256 c1 = Broadcast4( m + 4 );
    const __m256 c2 = Broadcast4( m + 8 );
    const __m256 c3 = Broadcast4( m + 12 );
    const __m256 w = _mm256_set1_ps( (Float32) W );
    const UInt8* in = static_cast< const UInt8* >( args.in );
    UInt8* out = static_cast< UInt8* >( args.out );
    const UInt32 count = args.count;
    UInt32 i = 0;
    for ( ; i + 2 <= count; i += 2 )
    {
        const __m128 low = LoadElement3( in + (size_t) i * args.inStrideBytes, false );
        const __m128 high = LoadElement3( in + (size_t) ( i + 1 ) * args.inStrideBytes, i + 2 == count );
        const __m256 v = Pair( low, high );
        const __m256 r = Combine8( c0, c1, c2, c3, _mm256_shuffle_ps( v, v, 0x00 ), _mm256_shuffle_ps( v, v, 0x55 ), _mm256_shuffle_ps( v, v, 0xAA ), w );
        StoreElement3( out + (size_t) i * args.outStrideBytes, _mm256_castps256_ps128( r ) );
        StoreElement3( out + (size_t) ( i + 1 ) * args.outStrideBytes, _mm256_extractf128_ps( r, 1 ) );
    }
    if ( i < count )
        MathBatchTransformElement3( m, (Float32) W, in + (size_t) i * args.inStrideBytes, out + (size_t) i * args.outStrideBytes );
    return MATHSTATUS_OK;
}

/*-------------------------------------------------------------------------*/

GUCEF_MATH_SIMD_TARGET_AVX2 TMathStatus
Avx2TransformVectors4( void* /* context */, const SMathTransformVectors4F32Args& args ) GUCEF_MATH_REALTIME_CONTEXT
{
    Float32 m[ 16 ];
    memcpy( m, args.matrix->GetData(), sizeof( m ) );
    const __m256 c0 = Broadcast4( m );
    const __m256 c1 = Broadcast4( m + 4 );
    const __m256 c2 = Broadcast4( m + 8 );
    const __m256 c3 = Broadcast4( m + 12 );
    const UInt8* in = static_cast< const UInt8* >( args.in );
    UInt8* out = static_cast< UInt8* >( args.out );
    const UInt32 count = args.count;
    UInt32 i = 0;
    for ( ; i + 2 <= count; i += 2 )
    {
        const __m128 low = _mm_loadu_ps( reinterpret_cast< const Float32* >( in + (size_t) i * args.inStrideBytes ) );
        const __m128 high = _mm_loadu_ps( reinterpret_cast< const Float32* >( in + (size_t) ( i + 1 ) * args.inStrideBytes ) );
        const __m256 v = Pair( low, high );
        const __m256 r = Combine8( c0, c1, c2, c3, _mm256_shuffle_ps( v, v, 0x00 ), _mm256_shuffle_ps( v, v, 0x55 ), _mm256_shuffle_ps( v, v, 0xAA ), _mm256_shuffle_ps( v, v, 0xFF ) );
        _mm_storeu_ps( reinterpret_cast< Float32* >( out + (size_t) i * args.outStrideBytes ), _mm256_castps256_ps128( r ) );
        _mm_storeu_ps( reinterpret_cast< Float32* >( out + (size_t) ( i + 1 ) * args.outStrideBytes ), _mm256_extractf128_ps( r, 1 ) );
    }
    if ( i < count )
        MathBatchTransformElement4( m, in + (size_t) i * args.inStrideBytes, out + (size_t) i * args.outStrideBytes );
    return MATHSTATUS_OK;
}

/*-------------------------------------------------------------------------*/

/** Two result columns per register: the columns of a in both halves, a column of b per half */
GUCEF_MATH_SIMD_TARGET_AVX2 TMathStatus
Avx2MultiplyMatrices4( void* /* context */, const SMathMultiplyMatrices4F32Args& args ) GUCEF_MATH_REALTIME_CONTEXT
{
    const UInt8* a = static_cast< const UInt8* >( args.a );
    const UInt8* b = static_cast< const UInt8* >( args.b );
    UInt8* out = static_cast< UInt8* >( args.out );
    for ( UInt32 i=0; i<args.count; ++i )
    {
        const Float32* pa = reinterpret_cast< const Float32* >( a + (size_t) i * args.aStrideBytes );
        const Float32* pb = reinterpret_cast< const Float32* >( b + (size_t) i * args.bStrideBytes );
        Float32* po = reinterpret_cast< Float32* >( out + (size_t) i * args.outStrideBytes );
        const __m256 a0 = Broadcast4( pa );
        const __m256 a1 = Broadcast4( pa + 4 );
        const __m256 a2 = Broadcast4( pa + 8 );
        const __m256 a3 = Broadcast4( pa + 12 );
        const __m256 b01 = _mm256_loadu_ps( pb );
        const __m256 b23 = _mm256_loadu_ps( pb + 8 );
        const __m256 r01 = Combine8( a0, a1, a2, a3, _mm256_shuffle_ps( b01, b01, 0x00 ), _mm256_shuffle_ps( b01, b01, 0x55 ), _mm256_shuffle_ps( b01, b01, 0xAA ), _mm256_shuffle_ps( b01, b01, 0xFF ) );
        const __m256 r23 = Combine8( a0, a1, a2, a3, _mm256_shuffle_ps( b23, b23, 0x00 ), _mm256_shuffle_ps( b23, b23, 0x55 ), _mm256_shuffle_ps( b23, b23, 0xAA ), _mm256_shuffle_ps( b23, b23, 0xFF ) );
        _mm256_storeu_ps( po, r01 );
        _mm256_storeu_ps( po + 8, r23 );
    }
    return MATHSTATUS_OK;
}

/*-------------------------------------------------------------------------*/

GUCEF_MATH_SIMD_TARGET_AVX2 TMathStatus
Avx2DotProducts3( void* /* context */, const SMathDotProducts3F32Args& args ) GUCEF_MATH_REALTIME_CONTEXT
{
    GUCEF_MATH_FP_CONTRACT_OFF
    const UInt8* a = static_cast< const UInt8* >( args.a );
    const UInt8* b = static_cast< const UInt8* >( args.b );
    UInt8* out = static_cast< UInt8* >( args.out );
    const UInt32 count = args.count;
    UInt32 i = 0;
    for ( ; i + 8 <= count; i += 8 )
    {
        __m256 ax, ay, az, bx, by, bz;
        LoadSoA8( a, args.aStrideBytes, i, count, ax, ay, az );
        LoadSoA8( b, args.bStrideBytes, i, count, bx, by, bz );
        const __m256 d = _mm256_add_ps( _mm256_add_ps( _mm256_mul_ps( ax, bx ), _mm256_mul_ps( ay, by ) ), _mm256_mul_ps( az, bz ) );
        Float32 lanes[ 8 ];
        _mm256_storeu_ps( lanes, d );
        for ( UInt32 k=0; k<8; ++k )
            memcpy( out + (size_t) ( i + k ) * args.outStrideBytes, &lanes[ k ], sizeof( Float32 ) );
    }
    for ( ; i<count; ++i )
        MathBatchDotElement3( a + (size_t) i * args.aStrideBytes, b + (size_t) i * args.bStrideBytes, out + (size_t) i * args.outStrideBytes );
    return MATHSTATUS_OK;
}

/*-------------------------------------------------------------------------*/

GUCEF_MATH_SIMD_TARGET_AVX2 TMathStatus
Avx2NormalizeVectors3( void* /* context */, const SMathNormalizeVectors3F32Args& args ) GUCEF_MATH_REALTIME_CONTEXT
{
    GUCEF_MATH_FP_CONTRACT_OFF
    const UInt8* in = static_cast< const UInt8* >( args.in );
    UInt8* out = static_cast< UInt8* >( args.out );
    const UInt32 count = args.count;
    const __m256 minimum = _mm256_set1_ps( TKernels::SafeSquaredLengthMinimum() );
    const __m256 maximum = _mm256_set1_ps( FLT_MAX );
    UInt32 i = 0;
    for ( ; i + 8 <= count; i += 8 )
    {
        __m256 x, y, z;
        LoadSoA8( in, args.inStrideBytes, i, count, x, y, z );
        const __m256 squared = _mm256_add_ps( _mm256_add_ps( _mm256_mul_ps( x, x ), _mm256_mul_ps( y, y ) ), _mm256_mul_ps( z, z ) );
        const int safe = _mm256_movemask_ps( _mm256_and_ps( _mm256_cmp_ps( squared, minimum, _CMP_GE_OQ ), _mm256_cmp_ps( squared, maximum, _CMP_LE_OQ ) ) );
        const __m256 length = _mm256_sqrt_ps( squared );
        __m256 r[ 4 ] = { _mm256_div_ps( x, length ), _mm256_div_ps( y, length ), _mm256_div_ps( z, length ), _mm256_setzero_ps() };
        Transpose4x8( r[ 0 ], r[ 1 ], r[ 2 ], r[ 3 ] );
        for ( UInt32 k=0; k<8; ++k )
        {
            UInt8* o = out + (size_t) ( i + k ) * args.outStrideBytes;
            if ( 0 != ( safe & ( 1 << k ) ) )
                StoreElement3( o, k < 4 ? _mm256_castps256_ps128( r[ k ] ) : _mm256_extractf128_ps( r[ k - 4 ], 1 ) );
            else
                MathBatchNormalizeElement3( in + (size_t) ( i + k ) * args.inStrideBytes, o );
        }
    }
    for ( ; i<count; ++i )
        MathBatchNormalizeElement3( in + (size_t) i * args.inStrideBytes, out + (size_t) i * args.outStrideBytes );
    return MATHSTATUS_OK;
}


/*-------------------------------------------------------------------------*/

/** Float64: one element per 256 bit register ( 4 doubles ), per lane ( c0 x + c2 z ) + ( c1 y + c3 w ), the canonical order */
GUCEF_MATH_SIMD_TARGET_AVX2 GUCEF_MATH_FORCE_INLINE __m256d
Combine4F64( __m256d c0, __m256d c1, __m256d c2, __m256d c3, __m256d x, __m256d y, __m256d z, __m256d w ) GUCEF_MATH_REALTIME_CONTEXT
{
    GUCEF_MATH_FP_CONTRACT_OFF
    return _mm256_add_pd( _mm256_add_pd( _mm256_mul_pd( c0, x ), _mm256_mul_pd( c2, z ) ), _mm256_add_pd( _mm256_mul_pd( c1, y ), _mm256_mul_pd( c3, w ) ) );
}

/*-------------------------------------------------------------------------*/

template < int W >
GUCEF_MATH_SIMD_TARGET_AVX2 TMathStatus
Avx2Transform3F64( void* /* context */, const SMathTransformPoints3F64Args& args ) GUCEF_MATH_REALTIME_CONTEXT
{
    const Float64* m = args.matrix->GetData();
    const __m256d c0 = _mm256_loadu_pd( m );
    const __m256d c1 = _mm256_loadu_pd( m + 4 );
    const __m256d c2 = _mm256_loadu_pd( m + 8 );
    const __m256d c3 = _mm256_loadu_pd( m + 12 );
    const __m256d w = _mm256_set1_pd( (Float64) W );
    const UInt8* in = static_cast< const UInt8* >( args.in );
    UInt8* out = static_cast< UInt8* >( args.out );
    for ( UInt32 i=0; i<args.count; ++i )
    {
        Float64 v[ 3 ];
        memcpy( v, in + (size_t) i * args.inStrideBytes, sizeof( v ) );
        const __m256d r = Combine4F64( c0, c1, c2, c3, _mm256_set1_pd( v[ 0 ] ), _mm256_set1_pd( v[ 1 ] ), _mm256_set1_pd( v[ 2 ] ), w );
        Float64 t[ 4 ];
        _mm256_storeu_pd( t, r );
        memcpy( out + (size_t) i * args.outStrideBytes, t, 3 * sizeof( Float64 ) );
    }
    return MATHSTATUS_OK;
}

/*-------------------------------------------------------------------------*/

GUCEF_MATH_SIMD_TARGET_AVX2 TMathStatus
Avx2MultiplyMatrices4F64( void* /* context */, const SMathMultiplyMatrices4F64Args& args ) GUCEF_MATH_REALTIME_CONTEXT
{
    const UInt8* a = static_cast< const UInt8* >( args.a );
    const UInt8* b = static_cast< const UInt8* >( args.b );
    UInt8* out = static_cast< UInt8* >( args.out );
    for ( UInt32 i=0; i<args.count; ++i )
    {
        const Float64* pa = reinterpret_cast< const Float64* >( a + (size_t) i * args.aStrideBytes );
        const Float64* pb = reinterpret_cast< const Float64* >( b + (size_t) i * args.bStrideBytes );
        Float64* po = reinterpret_cast< Float64* >( out + (size_t) i * args.outStrideBytes );
        const __m256d a0 = _mm256_loadu_pd( pa );
        const __m256d a1 = _mm256_loadu_pd( pa + 4 );
        const __m256d a2 = _mm256_loadu_pd( pa + 8 );
        const __m256d a3 = _mm256_loadu_pd( pa + 12 );
        __m256d r[ 4 ];
        for ( UInt32 c=0; c<4; ++c )
        {
            // an unaligned vector load and lane broadcasts: the elements may be only 4 byte aligned, typed Float64 reads would be undefined
            const __m256d column = _mm256_loadu_pd( pb + 4 * c );
            r[ c ] = Combine4F64( a0, a1, a2, a3, _mm256_permute4x64_pd( column, 0x00 ), _mm256_permute4x64_pd( column, 0x55 ),
                                  _mm256_permute4x64_pd( column, 0xAA ), _mm256_permute4x64_pd( column, 0xFF ) );
        }
        _mm256_storeu_pd( po, r[ 0 ] );
        _mm256_storeu_pd( po + 4, r[ 1 ] );
        _mm256_storeu_pd( po + 8, r[ 2 ] );
        _mm256_storeu_pd( po + 12, r[ 3 ] );
    }
    return MATHSTATUS_OK;
}
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
MathBatchGetAvx2Kernels( SMathBatchKernelSet& set )
{
    set.name = "AVX2";
    set.tier = MATHBATCHTIER_SIMD;
    set.requiredCpuFeatures = GUCEF_CPU_ISA_AVX | GUCEF_CPU_ISA_AVX2;
    for ( UInt32 k=0; k<MATHBATCHKERNEL_MATH_COUNT; ++k )
        set.functions[ k ] = GUCEF_NULL;
    set.functions[ MATHBATCHKERNEL_TRANSFORM_POINTS3_F32 ] = reinterpret_cast< TMathBatchGenericFunction >( static_cast< TMathTransformPoints3F32Function >( &Avx2Transform3< 1 > ) );
    set.functions[ MATHBATCHKERNEL_TRANSFORM_VECTORS3_F32 ] = reinterpret_cast< TMathBatchGenericFunction >( static_cast< TMathTransformVectors3F32Function >( &Avx2Transform3< 0 > ) );
    set.functions[ MATHBATCHKERNEL_TRANSFORM_VECTORS4_F32 ] = reinterpret_cast< TMathBatchGenericFunction >( static_cast< TMathTransformVectors4F32Function >( &Avx2TransformVectors4 ) );
    set.functions[ MATHBATCHKERNEL_MULTIPLY_MATRICES4_F32 ] = reinterpret_cast< TMathBatchGenericFunction >( static_cast< TMathMultiplyMatrices4F32Function >( &Avx2MultiplyMatrices4 ) );
    set.functions[ MATHBATCHKERNEL_DOT_PRODUCTS3_F32 ] = reinterpret_cast< TMathBatchGenericFunction >( static_cast< TMathDotProducts3F32Function >( &Avx2DotProducts3 ) );
    set.functions[ MATHBATCHKERNEL_NORMALIZE_VECTORS3_F32 ] = reinterpret_cast< TMathBatchGenericFunction >( static_cast< TMathNormalizeVectors3F32Function >( &Avx2NormalizeVectors3 ) );
    set.functions[ MATHBATCHKERNEL_TRANSFORM_POINTS3_F64 ] = reinterpret_cast< TMathBatchGenericFunction >( static_cast< TMathTransformPoints3F64Function >( &Avx2Transform3F64< 1 > ) );
    set.functions[ MATHBATCHKERNEL_TRANSFORM_VECTORS3_F64 ] = reinterpret_cast< TMathBatchGenericFunction >( static_cast< TMathTransformVectors3F64Function >( &Avx2Transform3F64< 0 > ) );
    set.functions[ MATHBATCHKERNEL_MULTIPLY_MATRICES4_F64 ] = reinterpret_cast< TMathBatchGenericFunction >( static_cast< TMathMultiplyMatrices4F64Function >( &Avx2MultiplyMatrices4F64 ) );
}

#else /* GUCEF_MATH_SIMD_BATCH_ALLOW_AVX2 ? */

void
MathBatchGetAvx2Kernels( SMathBatchKernelSet& set )
{
    set.name = "AVX2";
    set.tier = MATHBATCHTIER_SIMD;
    set.requiredCpuFeatures = GUCEF_CPU_ISA_AVX | GUCEF_CPU_ISA_AVX2;
    for ( UInt32 k=0; k<MATHBATCHKERNEL_MATH_COUNT; ++k )
        set.functions[ k ] = GUCEF_NULL;
}

#endif /* GUCEF_MATH_SIMD_BATCH_ALLOW_AVX2 ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace MATH */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
