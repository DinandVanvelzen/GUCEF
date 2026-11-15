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

#if ( GUCEF_MATH_SIMD_HAVE_NEON == 1 )
  #include "gucefMATH_CTVector4Ops_Neon.h"
  #include "gucefMATH_BatchKernelsSimd.h"
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

#if ( GUCEF_MATH_SIMD_HAVE_NEON == 1 )

namespace {

/**
 *  The conversions use the AArch64 instructions FCVTN and FCVTL (vcvt_f16_f32, vcvt_f32_f16), which are part of the
 *  base architecture: round to nearest even (the default rounding mode of the floating-point control register), exact
 *  widening, NaN payloads kept in the upper bits and quieted. They are registered without MATHPROVIDER_BITEXACT (so
 *  deterministic calls use the scalar provider): FCVTL quiets signalling NaN halves, which CFloat16::ToFloat32 keeps,
 *  and both instructions follow the floating-point control register (FPCR.AHP alternative half precision, FPCR.DN
 *  default NaN, FPCR.FZ16, the rounding mode), which the integer software conversion does not. With FPCR at its
 *  defaults the results equal the software conversion for every input except signalling NaN halves.
 *  Forward in place conversion to halves is safe (each block of 4 writes below the bytes it has read).
 */
TMathStatus
NeonConvertF32ToF16( void* /* context */, const SMathConvertF32ToF16Args& args ) GUCEF_MATH_REALTIME_CONTEXT
{
    const Float32* in = args.in;
    UInt16* out = reinterpret_cast< UInt16* >( args.out );
    const UInt32 count = args.count;
    UInt32 i = 0;
    for ( ; i + 4 <= count; i += 4 )
    {
        const float16x4_t h = vcvt_f16_f32( vld1q_f32( in + i ) );
        vst1_u16( out + i, vreinterpret_u16_f16( h ) );
    }
    if ( i < count )
    {
        Float32 t[ 4 ] = { 0.0f, 0.0f, 0.0f, 0.0f };
        const UInt32 rest = count - i;
        for ( UInt32 k=0; k<rest; ++k )
            t[ k ] = in[ i + k ];
        UInt16 h[ 4 ];
        vst1_u16( h, vreinterpret_u16_f16( vcvt_f16_f32( vld1q_f32( t ) ) ) );
        for ( UInt32 k=0; k<rest; ++k )
            out[ i + k ] = h[ k ];
    }
    return MATHSTATUS_OK;
}

/*-------------------------------------------------------------------------*/

/** From the end of the array: in place the floats are wider than the halves */
TMathStatus
NeonConvertF16ToF32( void* /* context */, const SMathConvertF16ToF32Args& args ) GUCEF_MATH_REALTIME_CONTEXT
{
    const UInt16* in = reinterpret_cast< const UInt16* >( args.in );
    Float32* out = args.out;
    const UInt32 count = args.count;
    const UInt32 head = count & 3U;
    for ( UInt32 b=count; b>head; b -= 4 )
    {
        const float32x4_t f = vcvt_f32_f16( vreinterpret_f16_u16( vld1_u16( in + ( b - 4 ) ) ) );
        vst1q_f32( out + ( b - 4 ), f );
    }
    if ( head > 0 )
    {
        UInt16 t[ 4 ] = { 0, 0, 0, 0 };
        for ( UInt32 k=0; k<head; ++k )
            t[ k ] = in[ k ];
        Float32 f[ 4 ];
        vst1q_f32( f, vcvt_f32_f16( vreinterpret_f16_u16( vld1_u16( t ) ) ) );
        for ( UInt32 k=0; k<head; ++k )
            out[ k ] = f[ k ];
    }
    return MATHSTATUS_OK;
}

}

#endif /* GUCEF_MATH_SIMD_HAVE_NEON ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *  NEON (AArch64) providers: the kernels of gucefMATH_BatchKernelsSimd.h over the lane policy CMathNeon, and the
 *  half conversions with the base architecture instructions.
 */
void
MathBatchGetNeonKernels( SMathBatchKernelSet& set )
{
    set.name = "NEON";
    set.tier = MATHBATCHTIER_SIMD;
    set.requiredCpuFeatures = GUCEF_CPU_ISA_NEON;
    for ( UInt32 k=0; k<MATHBATCHKERNEL_MATH_COUNT; ++k )
        set.functions[ k ] = GUCEF_NULL;
    #if ( GUCEF_MATH_SIMD_HAVE_NEON == 1 )
    CMathBatchSimd< CMathNeon >::FillKernelSet( set );
    set.functions[ MATHBATCHKERNEL_CONVERT_F32_TO_F16 ] = reinterpret_cast< TMathBatchGenericFunction >( static_cast< TMathConvertF32ToF16Function >( &NeonConvertF32ToF16 ) );
    set.functions[ MATHBATCHKERNEL_CONVERT_F16_TO_F32 ] = reinterpret_cast< TMathBatchGenericFunction >( static_cast< TMathConvertF16ToF32Function >( &NeonConvertF16ToF32 ) );
    set.notBitExactKernels = ( 1u << MATHBATCHKERNEL_CONVERT_F32_TO_F16 ) | ( 1u << MATHBATCHKERNEL_CONVERT_F16_TO_F32 );
    #endif
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace MATH */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
