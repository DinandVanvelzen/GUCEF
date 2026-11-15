/*
 *  computebridgeMATH: GUCEF compute bridge plugin of gucefMATH: multi-core and accelerator providers for the batch kernels
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

#ifndef GUCEF_CORE_CTFACTORY_H
#include "CTFactory.h"
#define GUCEF_CORE_CTFACTORY_H
#endif /* GUCEF_CORE_CTFACTORY_H ? */

#ifndef GUCEF_COMPUTEBRIDGEMATH_CTMATHBATCHCPUKERNEL_H
#include "computebridgeMATH_CTMathBatchCpuKernel.h"
#define GUCEF_COMPUTEBRIDGEMATH_CTMATHBATCHCPUKERNEL_H
#endif /* GUCEF_COMPUTEBRIDGEMATH_CTMATHBATCHCPUKERNEL_H ? */

#ifndef GUCEF_COMPUTEBRIDGEMATH_CTMATHMULTICOREPROVIDER_H
#include "computebridgeMATH_CTMathMulticoreProvider.h"
#define GUCEF_COMPUTEBRIDGEMATH_CTMATHMULTICOREPROVIDER_H
#endif /* GUCEF_COMPUTEBRIDGEMATH_CTMATHMULTICOREPROVIDER_H ? */

#ifndef GUCEF_COMPUTEBRIDGEMATH_CTMATHACCELERATORPROVIDER_H
#include "computebridgeMATH_CTMathAcceleratorProvider.h"
#define GUCEF_COMPUTEBRIDGEMATH_CTMATHACCELERATORPROVIDER_H
#endif /* GUCEF_COMPUTEBRIDGEMATH_CTMATHACCELERATORPROVIDER_H ? */

#include "computebridgeMATH_CMathKernelDecls.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COMPUTEBRIDGEMATH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** Items per workgroup of every bridged kernel, and workgroups per chunk the CPU backend processes at a time */
const UInt32 GroupSize = 256;
const UInt32 GroupsPerChunk = 64;

template < typename TKernelTraits >
void
DescribeKernel( COMPUTE::CComputeKernelDecl& decl )
{GUCEF_TRACE;

    COMPUTE::CComputeKernelSignature signature;
    signature.SetWorkgroupSize( GroupSize, 1, 1 );
    TKernelTraits::Describe( signature );
    decl.SetSignature( signature );
}

/*-------------------------------------------------------------------------*/

template < typename TKernelTraits >
COMPUTE::TCpuComputeKernelFactory*
CreateCpuFactory( void )
{GUCEF_TRACE;

    return GUCEF_NEW CORE::CTFactory< COMPUTE::CCpuComputeKernel, CTMathBatchCpuKernel< TKernelTraits >, MT::CMutex >();
}

/*-------------------------------------------------------------------------*/

/**
 *  One row. The cost numbers are single thread costs per item of the best provider and the speedups of the
 *  benchmarks of this bridge on the reference machine (GUCEF_TEST_COMPUTE_BENCH=1), the initial values of the cost
 *  model; CComputeSettings replaces the thresholds that follow from them.
 */
template < typename TKernelTraits >
SMathBridgeKernelInfo
MakeRow( const char* description, UInt32 grainItems, UInt32 bytesPerItem, Float64 simdNsPerItem, Float64 maxMultiCoreSpeedup,
         Float64 acceleratorSpeedup, COMPUTE::TComputeDeterminism determinism, UInt32 maxUlp )
{GUCEF_TRACE;

    SMathBridgeKernelInfo row;
    row.kernel = TKernelTraits::Kernel();
    row.description = description;
    row.grainItems = grainItems;
    row.bytesPerItem = bytesPerItem;
    row.simdNsPerItem = simdNsPerItem;
    row.maxMultiCoreSpeedup = maxMultiCoreSpeedup;
    row.acceleratorSpeedup = acceleratorSpeedup;
    row.determinism = determinism;
    row.maxUlp = maxUlp;
    row.multiCoreFunction = reinterpret_cast< MATH::TMathBatchGenericFunction >( &CTMathMulticoreProvider< TKernelTraits >::Provide );
    row.acceleratorFunction = reinterpret_cast< MATH::TMathBatchGenericFunction >( &CTMathAcceleratorProvider< TKernelTraits >::Provide );
    row.createCpuFactory = &CreateCpuFactory< TKernelTraits >;
    row.describe = &DescribeKernel< TKernelTraits >;
    return row;
}

/*-------------------------------------------------------------------------*/

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CMathKernelDecls::CMathKernelDecls( void )
{GUCEF_TRACE;

    UInt32 i = 0;
    //                                                           description                            grain  bytes  ns/item  speedup  accel  determinism                         ulp
    m_rows[ i++ ] = MakeRow< SMathTransformPoints3F32Kernel >(       "transform arrays of points by a matrix",          4096, 24, 1.9, 6.0, 30.0, COMPUTE::DETERMINISM_FLOAT_STRICT,   4 );
    m_rows[ i++ ] = MakeRow< SMathTransformVectors3F32Kernel >(      "transform arrays of vectors by a matrix",         4096, 24, 1.8, 6.0, 30.0, COMPUTE::DETERMINISM_FLOAT_STRICT,   4 );
    m_rows[ i++ ] = MakeRow< SMathTransformNormals3F32Kernel >(      "transform arrays of normals, normalised",         4096, 24, 4.5, 8.0, 30.0, COMPUTE::DETERMINISM_FLOAT_STRICT,   4 );
    m_rows[ i++ ] = MakeRow< SMathTransformVectors4F32Kernel >(      "transform arrays of 4 component vectors",         4096, 32, 1.3, 5.0, 30.0, COMPUTE::DETERMINISM_FLOAT_STRICT,   4 );
    m_rows[ i++ ] = MakeRow< SMathMultiplyMatrices4F32Kernel >(      "multiply arrays of 4 x 4 matrices pairwise",       512, 192, 6.2, 8.0, 40.0, COMPUTE::DETERMINISM_FLOAT_STRICT,   4 );
    m_rows[ i++ ] = MakeRow< SMathQuaternionsToMatricesF32Kernel >(  "quaternions to 4 x 4 matrices",                    1024, 80, 5.7, 8.0, 30.0, COMPUTE::DETERMINISM_FLOAT_STRICT,   4 );
    m_rows[ i++ ] = MakeRow< SMathDotProducts3F32Kernel >(           "dot products of 3 component vector pairs",         4096, 28, 1.95, 6.0, 20.0, COMPUTE::DETERMINISM_FLOAT_STRICT,   4 );
    m_rows[ i++ ] = MakeRow< SMathNormalizeVectors3F32Kernel >(      "normalise arrays of 3 component vectors",          4096, 24, 3.0, 8.0, 30.0, COMPUTE::DETERMINISM_FLOAT_STRICT,   4 );
    m_rows[ i++ ] = MakeRow< SMathConvertF32ToF16Kernel >(           "convert Float32 to Float16, nearest even",         8192, 6, 0.055, 3.0, 20.0, COMPUTE::DETERMINISM_EXACT_INTEGER,  0 );
    m_rows[ i++ ] = MakeRow< SMathConvertF16ToF32Kernel >(           "convert Float16 to Float32",                       8192, 6, 0.68, 8.0, 20.0, COMPUTE::DETERMINISM_EXACT_INTEGER,  0 );
    // Float64 and fixed point: no accelerator form for the cost model (Float64 on GPUs is off by default, fixed point is never
    // on a GPU by default), the numbers are those of the CPU
    m_rows[ i++ ] = MakeRow< SMathTransformPoints3F64Kernel >(       "transform arrays of Float64 points by a matrix",   2048, 48, 2.7, 6.0, 4.0, COMPUTE::DETERMINISM_FLOAT_STRICT,   4 );
    m_rows[ i++ ] = MakeRow< SMathTransformVectors3F64Kernel >(      "transform arrays of Float64 vectors by a matrix",  2048, 48, 2.8, 6.0, 4.0, COMPUTE::DETERMINISM_FLOAT_STRICT,   4 );
    m_rows[ i++ ] = MakeRow< SMathMultiplyMatrices4F64Kernel >(      "multiply arrays of Float64 4 x 4 matrices",         256, 384, 7.9, 4.0, 4.0, COMPUTE::DETERMINISM_FLOAT_STRICT,   4 );
    m_rows[ i++ ] = MakeRow< SMathTransformPoints3Fx32Kernel >(      "transform arrays of Q16.16 points by a matrix",    4096, 24, 46.7, 12.0, 10.0, COMPUTE::DETERMINISM_EXACT_INTEGER,  0 );
    m_rows[ i++ ] = MakeRow< SMathMultiplyMatrices4Fx32Kernel >(     "multiply arrays of Q16.16 4 x 4 matrices",          512, 192, 394.0, 12.0, 10.0, COMPUTE::DETERMINISM_EXACT_INTEGER,  0 );
    m_rows[ i++ ] = MakeRow< SMathVbapGains2DF32Kernel >(            "vector base amplitude panning gains per source",   1024, 36, 37.0, 12.0, 30.0, COMPUTE::DETERMINISM_FLOAT_STRICT,   8 );
    m_rows[ i++ ] = MakeRow< SMathNoiseField2DF32Kernel >(           "samples of a noise over a 2D grid",                 512, 4, 11.3, 10.0, 60.0, COMPUTE::DETERMINISM_FLOAT_STRICT,   8 );
    m_rows[ i++ ] = MakeRow< SMathNoiseField3DF32Kernel >(           "samples of a noise over a 3D grid",                 512, 4, 24.0, 10.0, 60.0, COMPUTE::DETERMINISM_FLOAT_STRICT,   8 );
    GUCEF_ASSERT( KernelRowCount == i );
}

/*-------------------------------------------------------------------------*/

CMathKernelDecls::~CMathKernelDecls()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

UInt32
CMathKernelDecls::GetKernelCount( void ) const
{GUCEF_TRACE;

    return KernelRowCount;
}

/*-------------------------------------------------------------------------*/

const SMathBridgeKernelInfo&
CMathKernelDecls::GetKernelInfo( UInt32 index ) const
{GUCEF_TRACE;

    return m_rows[ index < KernelRowCount ? index : 0 ];
}

/*-------------------------------------------------------------------------*/

const SMathBridgeKernelInfo*
CMathKernelDecls::FindKernelInfo( MATH::TMathBatchKernel kernel ) const
{GUCEF_TRACE;

    for ( UInt32 i=0; i<KernelRowCount; ++i )
    {
        if ( m_rows[ i ].kernel == kernel )
            return &m_rows[ i ];
    }
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

void
CMathKernelDecls::MakeDecl( const SMathBridgeKernelInfo& info, const CString& kernelId, COMPUTE::CComputeKernelDecl& decl )
{GUCEF_TRACE;

    decl = COMPUTE::CComputeKernelDecl( kernelId, InterfaceVersion );
    info.describe( decl );
    decl.SetDeterminism( info.determinism );
    if ( COMPUTE::DETERMINISM_EXACT_INTEGER == info.determinism )
        decl.SetTolerance( COMPUTE::CComputeTolerance() );
    else
        decl.SetTolerance( COMPUTE::CComputeTolerance( info.maxUlp, 1.0e-6, 1.0e-5 ) );
    decl.SetPreferredGrain( GroupsPerChunk );
    decl.SetDescription( info.description );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace COMPUTEBRIDGEMATH */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
