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

#include "gucefMATH_BatchKernels.h"

#if ( GUCEF_MATH_SIMD_BATCH_ALLOW_F16C == 1 )
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

#if ( GUCEF_MATH_SIMD_BATCH_ALLOW_F16C == 1 )

/*
 *  F16C conversion providers, compiled with the target attribute (GUCEF_MATH_SIMD_TARGET_F16C) and registered only when
 *  the CPU reports F16C with operating system AVX support. Float32 to half rounds to nearest even (immediate 0), which
 *  equals CFloat16::FromFloat32 for every input including NaN (quiet bit set, top payload bits kept). Half to Float32
 *  is exact as in CFloat16::ToFloat32, except that the instruction quiets signalling NaN, which the software conversion
 *  keeps: blocks that contain a NaN go through the software conversion. Eight values per step; tails in software.
 */
namespace {

GUCEF_MATH_SIMD_TARGET_F16C GUCEF_MATH_FORCE_INLINE void
SoftwareF32ToF16( const UInt8* in, UInt8* out, UInt32 i ) GUCEF_MATH_REALTIME_CONTEXT
{
    Float32 value;
    memcpy( &value, in + (size_t) i * sizeof( Float32 ), sizeof( value ) );
    const UInt16 bits = CFloat16::FromFloat32( value ).GetBits();
    memcpy( out + (size_t) i * sizeof( UInt16 ), &bits, sizeof( bits ) );
}

/*-------------------------------------------------------------------------*/

GUCEF_MATH_SIMD_TARGET_F16C GUCEF_MATH_FORCE_INLINE void
SoftwareF16ToF32( const UInt8* in, UInt8* out, UInt32 i ) GUCEF_MATH_REALTIME_CONTEXT
{
    UInt16 bits;
    memcpy( &bits, in + (size_t) i * sizeof( UInt16 ), sizeof( bits ) );
    const Float32 value = CFloat16::FromBits( bits ).ToFloat32();
    memcpy( out + (size_t) i * sizeof( Float32 ), &value, sizeof( value ) );
}

/*-------------------------------------------------------------------------*/

/** Halves i .. i + 7 to Float32; false (nothing written) when one of them is a NaN */
GUCEF_MATH_SIMD_TARGET_F16C GUCEF_MATH_FORCE_INLINE bool
ConvertF16Block8( const UInt8* in, UInt8* out, UInt32 i ) GUCEF_MATH_REALTIME_CONTEXT
{
    const __m128i halves = _mm_loadu_si128( reinterpret_cast< const __m128i* >( in + (size_t) i * sizeof( UInt16 ) ) );
    const __m128i exponentBits = _mm_set1_epi16( 0x7C00 );
    const __m128i maxExponent = _mm_cmpeq_epi16( _mm_and_si128( halves, exponentBits ), exponentBits );
    const __m128i zeroFraction = _mm_cmpeq_epi16( _mm_and_si128( halves, _mm_set1_epi16( 0x03FF ) ), _mm_setzero_si128() );
    if ( 0 != _mm_movemask_epi8( _mm_andnot_si128( zeroFraction, maxExponent ) ) )
        return false;
    _mm256_storeu_ps( reinterpret_cast< Float32* >( out + (size_t) i * sizeof( Float32 ) ), _mm256_cvtph_ps( halves ) );
    return true;
}

/*-------------------------------------------------------------------------*/

/** Forward: each block of halves is written below the floats still to be read, so the same array works in place */
GUCEF_MATH_SIMD_TARGET_F16C TMathStatus
F16cConvertF32ToF16( void* /* context */, const SMathConvertF32ToF16Args& args ) GUCEF_MATH_REALTIME_CONTEXT
{
    const UInt8* in = reinterpret_cast< const UInt8* >( args.in );
    UInt8* out = reinterpret_cast< UInt8* >( args.out );
    const UInt32 count = args.count;
    UInt32 i = 0;
    for ( ; i + 8 <= count; i += 8 )
    {
        const __m256 values = _mm256_loadu_ps( reinterpret_cast< const Float32* >( in + (size_t) i * sizeof( Float32 ) ) );
        const __m128i halves = _mm256_cvtps_ph( values, 0 );        // round to nearest even
        _mm_storeu_si128( reinterpret_cast< __m128i* >( out + (size_t) i * sizeof( UInt16 ) ), halves );
    }
    for ( ; i<count; ++i )
        SoftwareF32ToF16( in, out, i );
    return MATHSTATUS_OK;
}

/*-------------------------------------------------------------------------*/

/** In place the floats are wider than the halves: from the end of the array, so nothing is overwritten before it is read */
GUCEF_MATH_SIMD_TARGET_F16C TMathStatus
F16cConvertF16ToF32( void* /* context */, const SMathConvertF16ToF32Args& args ) GUCEF_MATH_REALTIME_CONTEXT
{
    const UInt8* in = reinterpret_cast< const UInt8* >( args.in );
    UInt8* out = reinterpret_cast< UInt8* >( args.out );
    const UInt32 count = args.count;
    if ( static_cast< const void* >( in ) == static_cast< const void* >( out ) )
    {
        UInt32 i = count;
        while ( 0 != i % 8 )
        {
            --i;
            SoftwareF16ToF32( in, out, i );
        }
        while ( i > 0 )
        {
            i -= 8;
            if ( !ConvertF16Block8( in, out, i ) )
            {
                for ( UInt32 j=i+8; j>i; --j )
                    SoftwareF16ToF32( in, out, j - 1 );
            }
        }
        return MATHSTATUS_OK;
    }

    UInt32 i = 0;
    for ( ; i + 8 <= count; i += 8 )
    {
        if ( !ConvertF16Block8( in, out, i ) )
        {
            for ( UInt32 j=i; j<i+8; ++j )
                SoftwareF16ToF32( in, out, j );
        }
    }
    for ( ; i<count; ++i )
        SoftwareF16ToF32( in, out, i );
    return MATHSTATUS_OK;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
MathBatchGetF16cKernels( SMathBatchKernelSet& set )
{
    set.name = "F16C";
    set.tier = MATHBATCHTIER_SIMD;
    set.requiredCpuFeatures = GUCEF_CPU_ISA_AVX | GUCEF_CPU_ISA_F16C;
    for ( UInt32 k=0; k<MATHBATCHKERNEL_MATH_COUNT; ++k )
        set.functions[ k ] = GUCEF_NULL;
    set.functions[ MATHBATCHKERNEL_CONVERT_F32_TO_F16 ] = reinterpret_cast< TMathBatchGenericFunction >( static_cast< TMathConvertF32ToF16Function >( &F16cConvertF32ToF16 ) );
    set.functions[ MATHBATCHKERNEL_CONVERT_F16_TO_F32 ] = reinterpret_cast< TMathBatchGenericFunction >( static_cast< TMathConvertF16ToF32Function >( &F16cConvertF16ToF32 ) );
}

#else /* GUCEF_MATH_SIMD_BATCH_ALLOW_F16C ? */

void
MathBatchGetF16cKernels( SMathBatchKernelSet& set )
{
    set.name = "F16C";
    set.tier = MATHBATCHTIER_SIMD;
    set.requiredCpuFeatures = GUCEF_CPU_ISA_AVX | GUCEF_CPU_ISA_F16C;
    for ( UInt32 k=0; k<MATHBATCHKERNEL_MATH_COUNT; ++k )
        set.functions[ k ] = GUCEF_NULL;
}

#endif /* GUCEF_MATH_SIMD_BATCH_ALLOW_F16C ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace MATH */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
