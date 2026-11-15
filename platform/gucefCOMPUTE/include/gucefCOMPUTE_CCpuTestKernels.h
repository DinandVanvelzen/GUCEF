/*
 *  gucefCOMPUTE: GUCEF module providing compute: devices, buffers and kernels over pluggable backends
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

#ifndef GUCEF_COMPUTE_CCPUTESTKERNELS_H
#define GUCEF_COMPUTE_CCPUTESTKERNELS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

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
namespace COMPUTE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CComputeKernelRegistry;

/**
 *  Built-in test kernels for the CPU backend, registered only by test applications (and later the backend conformance
 *  suites, which compare GPU artefacts of the same ids with them). The object owns the factories of the kernels and
 *  is the owner of their registration; its destructor unregisters them.
 *
 *   - FillKernelId "COMPUTE.Fill.U32": out[ i ] = base + i * step. Slot 0 write, constants { header, base, step }.
 *     Exact integer.
 *   - SaxpyKernelId "COMPUTE.Saxpy.F32": y[ i ] = a * x[ i ] + y[ i ]. Slot 0 read (x), slot 1 read-write (y),
 *     constants { header, a }. Strict float, no fused multiply-add (the product is rounded before the sum).
 *   - ReduceSumKernelId "COMPUTE.ReduceSum.F32": partial[ g ] = in[ g * 256 ] + ... in sequential order within each
 *     group of 256 items. Slot 0 read, slot 1 write (one value per group). Strict float, bit-identical for any number
 *     of workers; combining the partials in group order on the host gives a deterministic total.
 *   - ScanGroupsKernelId "COMPUTE.ScanGroups.U32": inclusive prefix sum within each group of 256 items through the
 *     workgroup memory, and the total of each group. Slot 0 read, slot 1 write (scan), slot 2 write (totals).
 *     Exact integer, 1 KiB of workgroup memory.
 *   - AddGroupOffsetsKernelId "COMPUTE.AddGroupOffsets.U32": data[ i ] += offsets[ i / 256 - 1 ] for i >= 256 (the
 *     inclusive scan of the group totals), which turns per-group scans into a scan of the whole array. Slot 0
 *     read-write, slot 1 read. Exact integer.
 *   - HashKernelId "COMPUTE.Hash.U32": out[ i ] = Hash( in[ i ] ^ seed ) with Hash() the integer finaliser of
 *     MurmurHash3 (unsigned wrap-around arithmetic and shifts only). Slot 0 read, slot 1 write, constants
 *     { header, seed }. Exact integer.
 *  Every kernel has a workgroup of 256 x 1 x 1 and bounds-checks against the item count.
 */
class GUCEF_COMPUTE_PUBLIC_CPP CCpuTestKernels
{
    public:

    static const char* FillKernelId;
    static const char* SaxpyKernelId;
    static const char* ReduceSumKernelId;
    static const char* ScanGroupsKernelId;
    static const char* AddGroupOffsetsKernelId;
    static const char* HashKernelId;

    /** Items per workgroup of every test kernel */
    static const UInt32 GroupSize;

    /** The constants of COMPUTE.Fill.U32 */
    struct SFillConstants { UInt32 baseItem; UInt32 itemCount; UInt32 base; UInt32 step; };

    /** The constants of COMPUTE.Saxpy.F32 */
    struct SSaxpyConstants { UInt32 baseItem; UInt32 itemCount; Float32 a; UInt32 padding; };

    /** The constants of COMPUTE.Hash.U32 */
    struct SHashConstants { UInt32 baseItem; UInt32 itemCount; UInt32 seed; UInt32 padding; };

    /** The integer hash of COMPUTE.Hash.U32 */
    static UInt32 Hash( UInt32 value );

    CCpuTestKernels( void );

    /** Unregisters the kernels when they are registered */
    ~CCpuTestKernels();

    /** Registers the declarations and CPU artefacts of every test kernel. False when one could not be registered */
    bool Register( CComputeKernelRegistry& registry );

    /** Unregisters the kernels of this object from the registry of the global that exists now; returns how many */
    UInt32 Unregister( void );

    bool IsRegistered( void ) const;

    private:

    enum { KernelCount = 6 };

    CCpuTestKernels( const CCpuTestKernels& src );            /**< not implemented */
    CCpuTestKernels& operator=( const CCpuTestKernels& src ); /**< not implemented */

    private:

    CComputeKernelRegistry* m_registry;
    TCpuComputeKernelFactory* m_factories[ KernelCount ];
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace COMPUTE */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMPUTE_CCPUTESTKERNELS_H ? */
