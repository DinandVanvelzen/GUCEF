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

#include "gucefMATH_BatchKernels.h"

#if ( GUCEF_MATH_SIMD_HAVE_WASM128 == 1 )
  #include "gucefMATH_CTVector4Ops_Wasm128.h"
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
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *  WebAssembly SIMD128 providers: the kernels of gucefMATH_BatchKernelsSimd.h over the lane policy CMathWasm128. The
 *  conversions between Float32 and binary16 have no SIMD128 instructions and stay with the scalar provider.
 */
void
MathBatchGetWasm128Kernels( SMathBatchKernelSet& set )
{
    set.name = "WASM128";
    set.tier = MATHBATCHTIER_SIMD;
    set.requiredCpuFeatures = GUCEF_CPU_ISA_WASM_SIMD128;
    for ( UInt32 k=0; k<MATHBATCHKERNEL_MATH_COUNT; ++k )
        set.functions[ k ] = GUCEF_NULL;
    #if ( GUCEF_MATH_SIMD_HAVE_WASM128 == 1 )
    CMathBatchSimd< CMathWasm128 >::FillKernelSet( set );
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
