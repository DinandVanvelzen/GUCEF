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

#ifndef GUCEF_COMPUTEBRIDGEMATH_SMATHBRIDGEKERNELINFO_H
#define GUCEF_COMPUTEBRIDGEMATH_SMATHBRIDGEKERNELINFO_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_COMPUTEBRIDGEMATH_ETYPES_H
#include "computebridgeMATH_ETypes.h"
#define GUCEF_COMPUTEBRIDGEMATH_ETYPES_H
#endif /* GUCEF_COMPUTEBRIDGEMATH_ETYPES_H ? */

#ifndef GUCEF_COMPUTE_CCOMPUTEKERNELDECL_H
#include "gucefCOMPUTE_CComputeKernelDecl.h"
#define GUCEF_COMPUTE_CCOMPUTEKERNELDECL_H
#endif /* GUCEF_COMPUTE_CCOMPUTEKERNELDECL_H ? */

#ifndef GUCEF_COMPUTE_CCOMPUTEKERNELARTEFACT_H
#include "gucefCOMPUTE_CComputeKernelArtefact.h"
#define GUCEF_COMPUTE_CCOMPUTEKERNELARTEFACT_H
#endif /* GUCEF_COMPUTE_CCOMPUTEKERNELARTEFACT_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COMPUTEBRIDGEMATH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Everything the bridge knows about one MATH batch kernel: how to chunk it, the numbers of the cost model, how it is
 *  declared to gucefCOMPUTE and which provider functions serve it. One row per kernel in CMathKernelDecls; adding a
 *  bridged kernel is a traits struct (computebridgeMATH_MathKernelTraits.h) and a row.
 */
struct SMathBridgeKernelInfo
{
    MATH::TMathBatchKernel kernel;
    const char* description;                    /**< for the log and the declaration */

    UInt32 grainItems;                          /**< items per chunk of a multi-core call: depends on the kernel only, never on the number of workers */
    UInt32 bytesPerItem;                        /**< bytes read and written per item, for the transfer cost model */
    Float64 simdNsPerItem;                      /**< cost of the best single thread provider per item, reference hardware (cost model) */
    Float64 maxMultiCoreSpeedup;                /**< what the memory bandwidth allows over one thread, the bound of the cost model */
    Float64 acceleratorSpeedup;                 /**< the compute throughput of an accelerator over one thread, for the cost model */

    COMPUTE::TComputeDeterminism determinism;   /**< EXACT_INTEGER only for kernels that are integer operations of defined semantics */
    UInt32 maxUlp;                              /**< the tolerance other backends get against the CPU kernel (0 for exact kernels) */

    MATH::TMathBatchGenericFunction multiCoreFunction;      /**< the typed MULTICORE provider, cast */
    MATH::TMathBatchGenericFunction acceleratorFunction;    /**< the typed ACCELERATOR provider, cast; null for a kernel without device form */
    COMPUTE::TCpuComputeKernelFactory* ( *createCpuFactory )( void );           /**< null for a kernel without device form */
    void ( *describe )( COMPUTE::CComputeKernelDecl& decl );                    /**< fills bindings, constants and workgroup of the declaration */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace COMPUTEBRIDGEMATH */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMPUTEBRIDGEMATH_SMATHBRIDGEKERNELINFO_H ? */
