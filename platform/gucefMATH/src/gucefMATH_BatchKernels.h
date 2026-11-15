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

#ifndef GUCEF_MATH_BATCHKERNELS_H
#define GUCEF_MATH_BATCHKERNELS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_DVOSWRAP_H
#include "DVOSWRAP.h"                   /* GUCEF_CPU_ISA_* */
#define GUCEF_CORE_DVOSWRAP_H
#endif /* GUCEF_CORE_DVOSWRAP_H ? */

#ifndef GUCEF_MATH_MATHBATCHTYPES_H
#include "gucefMATH_MathBatchTypes.h"
#define GUCEF_MATH_MATHBATCHTYPES_H
#endif /* GUCEF_MATH_MATHBATCHTYPES_H ? */

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

/*
 *  Private to the module: the providers of the MATH batch kernels per instruction set, and the scalar element
 *  operations that define each kernel's result. The SIMD kernels call the element operations for the elements they do
 *  not handle in SIMD (tails, lanes outside a fast path), so every provider is bit-identical with the scalar one by
 *  construction there and by the canonical evaluation order elsewhere. No tracing, no allocation, no locks.
 */

/** The providers of one instruction set; a NULL function means the set has no implementation of that kernel */
struct SMathBatchKernelSet
{
    const char* name;
    TMathBatchTier tier;
    UInt64 requiredCpuFeatures;
    TMathBatchGenericFunction functions[ MATHBATCHKERNEL_MATH_COUNT ];
    UInt32 notBitExactKernels;          /**< bit k set: the provider of kernel k is registered without MATHPROVIDER_BITEXACT (0 unless a getter sets it) */
};

/** Fills 'set': the scalar providers of every kernel */
GUCEF_MATH_PRIVATE_CPP void MathBatchGetScalarKernels( SMathBatchKernelSet& set );

/** Fills 'set': the SSE2 providers; every function NULL when SSE2 is not configured */
GUCEF_MATH_PRIVATE_CPP void MathBatchGetSse2Kernels( SMathBatchKernelSet& set );

/** Fills 'set': the AVX2 providers; every function NULL when GUCEF_MATH_SIMD_BATCH_ALLOW_AVX2 is 0 */
GUCEF_MATH_PRIVATE_CPP void MathBatchGetAvx2Kernels( SMathBatchKernelSet& set );

/** Fills 'set': the F16C conversion providers; every function NULL when GUCEF_MATH_SIMD_BATCH_ALLOW_F16C is 0 */
GUCEF_MATH_PRIVATE_CPP void MathBatchGetF16cKernels( SMathBatchKernelSet& set );

/** Fills 'set': the NEON (AArch64) providers; every function NULL when NEON is not configured */
GUCEF_MATH_PRIVATE_CPP void MathBatchGetNeonKernels( SMathBatchKernelSet& set );

/** Fills 'set': the WebAssembly SIMD128 providers; every function NULL when WASM128 is not configured */
GUCEF_MATH_PRIVATE_CPP void MathBatchGetWasm128Kernels( SMathBatchKernelSet& set );

/*-------------------------------------------------------------------------*/

/**
 *  The access of the VbapGains2D providers to a prepared CTVbap2D (a friend of the class): the first half of
 *  ComputeGains, which locates the arc of a source, and the data of the second half. The providers compute the
 *  arithmetic of the second half for several sources at once and write the same values, because it only uses the basic
 *  operations (correctly rounded) in the order of ComputeGains.
 */
struct SMathVbapBatchAccess
{
    typedef CTNumericTraits< Float32 > TTraits;

    /**
     *  Finds the arc that contains 'azimuth' and its sin and cos, as ComputeGains does. False when the source is not
     *  in the plain VBAP case (the azimuth is not finite or the arc is the open side of a stereo pair): such a source
     *  must go through ComputeGains as a whole.
     */
    static GUCEF_MATH_FORCE_INLINE bool
    Locate( const TFloat32Vbap2D& vbap, Float32 azimuth, UInt32& arc, Float32& sinPosition, Float32& cosPosition ) GUCEF_MATH_REALTIME_CONTEXT
    {
        if ( !TTraits::IsFinite( azimuth ) )
            return false;
        const Float32 start = vbap.m_angle[ 0 ];
        Float32 position = start + TTraits::ModFloor( azimuth - start, CTMathConstants< Float32 >::TwoPi() );
        if ( !( position < vbap.m_angle[ vbap.m_count ] ) )
            position = start;
        UInt32 k = 0;
        while ( k + 1 < vbap.m_count && vbap.m_angle[ k + 1 ] <= position )
            ++k;
        if ( !vbap.m_covered[ k ] )
            return false;
        arc = k;
        sinPosition = TTraits::Sin( position );
        cosPosition = TTraits::Cos( position );
        return true;
    }

    /** Element 'index' (column-major, 0 to 3) of the inverse of the basis of an arc */
    static GUCEF_MATH_FORCE_INLINE Float32 GetInverse( const TFloat32Vbap2D& vbap, UInt32 arc, UInt32 index ) GUCEF_MATH_REALTIME_CONTEXT { return vbap.m_inverse[ arc ].GetData()[ index ]; }

    /** The index, as given to Prepare, of the speaker at the start of an arc and of the one at its end */
    static GUCEF_MATH_FORCE_INLINE UInt32 GetFirstSpeaker( const TFloat32Vbap2D& vbap, UInt32 arc ) GUCEF_MATH_REALTIME_CONTEXT { return vbap.m_order[ arc ]; }
    static GUCEF_MATH_FORCE_INLINE UInt32 GetSecondSpeaker( const TFloat32Vbap2D& vbap, UInt32 arc ) GUCEF_MATH_REALTIME_CONTEXT { return vbap.m_order[ ( arc + 1 ) % vbap.m_count ]; }
};

/** ComputeGains of one source: the azimuth is read from 'azimuth' (4 bytes), GetSpeakerCount() gains are written to 'gains' */
GUCEF_MATH_PRIVATE_CPP void MathBatchVbapElement( const TFloat32Vbap2D& vbap, const void* azimuth, Float32 targetPower, void* gains ) GUCEF_MATH_REALTIME_CONTEXT;

/**
 *  The scalar noise field of the arguments, complete (every noise type), for the items and noise types the SIMD
 *  providers do not compute themselves: a SIMD provider calls it with a copy of its arguments whose baseItem, out and
 *  count describe the part left.
 */
GUCEF_MATH_PRIVATE_CPP void MathBatchNoiseField2DScalar( const SMathNoiseField2DF32Args& args ) GUCEF_MATH_REALTIME_CONTEXT;
GUCEF_MATH_PRIVATE_CPP void MathBatchNoiseField3DScalar( const SMathNoiseField3DF32Args& args ) GUCEF_MATH_REALTIME_CONTEXT;

/*-------------------------------------------------------------------------*/

/** The cofactor matrix of the upper 3 x 3 block (columns c1 x c2, c2 x c0, c0 x c1) and whether its determinant is negative */
struct SMathBatchNormalMatrix
{
    Float32 columns[ 3 ][ 3 ];
    bool negate;
};

/** As CTMatrix3D::TransformNormal prepares it */
GUCEF_MATH_PRIVATE_CPP void MathBatchComputeNormalMatrix( const TFloat32Matrix4D& m, SMathBatchNormalMatrix& out ) GUCEF_MATH_REALTIME_CONTEXT;

/*
 *  Element operations on byte addresses (no alignment beyond what memcpy needs). 'in' may equal 'out'.
 */

/** Points (w = 1) and vectors (w = 0): the first 3 components of m ( x, y, z, w ) */
GUCEF_MATH_PRIVATE_CPP void MathBatchTransformElement3( const Float32 ( &m )[ 16 ], const Float32 w, const void* in, void* out ) GUCEF_MATH_REALTIME_CONTEXT;

GUCEF_MATH_PRIVATE_CPP void MathBatchTransformElement4( const Float32 ( &m )[ 16 ], const void* in, void* out ) GUCEF_MATH_REALTIME_CONTEXT;

GUCEF_MATH_PRIVATE_CPP void MathBatchTransformNormalElement( const SMathBatchNormalMatrix& n, const void* in, void* out ) GUCEF_MATH_REALTIME_CONTEXT;

/** The normalisation step of a normal: 'transformed' (already transformed and sign corrected) normalised into 'out', or 'in' copied when that is not possible */
GUCEF_MATH_PRIVATE_CPP void MathBatchFinishNormalElement( const Float32 ( &transformed )[ 3 ], const void* in, void* out ) GUCEF_MATH_REALTIME_CONTEXT;

GUCEF_MATH_PRIVATE_CPP void MathBatchMultiplyElement( const void* a, const void* b, void* out ) GUCEF_MATH_REALTIME_CONTEXT;

GUCEF_MATH_PRIVATE_CPP void MathBatchQuaternionToMatrixElement( const void* in, void* out ) GUCEF_MATH_REALTIME_CONTEXT;

GUCEF_MATH_PRIVATE_CPP void MathBatchDotElement3( const void* a, const void* b, void* out ) GUCEF_MATH_REALTIME_CONTEXT;

GUCEF_MATH_PRIVATE_CPP void MathBatchNormalizeElement3( const void* in, void* out ) GUCEF_MATH_REALTIME_CONTEXT;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace MATH */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MATH_BATCHKERNELS_H ? */
