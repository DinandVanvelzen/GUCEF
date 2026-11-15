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

#ifndef GUCEF_COMPUTEBRIDGEMATH_CMATHCOSTMODEL_H
#define GUCEF_COMPUTEBRIDGEMATH_CMATHCOSTMODEL_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_COMPUTE_CCOMPUTEDEVICE_H
#include "gucefCOMPUTE_CComputeDevice.h"
#define GUCEF_COMPUTE_CCOMPUTEDEVICE_H
#endif /* GUCEF_COMPUTE_CCOMPUTEDEVICE_H ? */

#ifndef GUCEF_COMPUTEBRIDGEMATH_SMATHBRIDGEKERNELINFO_H
#include "computebridgeMATH_SMathBridgeKernelInfo.h"
#define GUCEF_COMPUTEBRIDGEMATH_SMATHBRIDGEKERNELINFO_H
#endif /* GUCEF_COMPUTEBRIDGEMATH_SMATHBRIDGEKERNELINFO_H ? */

#ifndef GUCEF_COMPUTEBRIDGEMATH_MACROS_H
#include "computebridgeMATH_macros.h"
#define GUCEF_COMPUTEBRIDGEMATH_MACROS_H
#endif /* GUCEF_COMPUTEBRIDGEMATH_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COMPUTEBRIDGEMATH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The transfer cost model of COMPUTE design section 5.6: from which batch size a provider beats the best provider
 *  below it, per kernel. The cost of a call is  fixed latency + bytes moved / bus bandwidth + items / throughput,
 *  with the bus term zero for memory the device shares with the CPU (unified memory, the CPU backend).
 *
 *   - MULTICORE against the best single thread provider (SIMD): the fixed latency is the fork-join of the job system
 *     (waking workers, joining); the throughput is the single thread cost per item divided by the speedup, which is the
 *     number of threads (the workers plus the calling thread) bounded by what the memory bandwidth allows.
 *   - ACCELERATOR against the best CPU tier: the fixed latency and the bus bandwidth are measured once per device
 *     (MeasureDeviceTransfer, a few small copies); the throughput of the device is the single thread cost divided by
 *     the accelerator speedup of the kernel. Memory bound kernels never win over a discrete device: the result is then
 *     'never' (NeverCount) and the provider is not registered.
 *
 *  Both give a minimum count: the thresholds the providers are registered with. The numbers are the initial values from
 *  measurement on reference hardware (the rows of CMathKernelDecls and the fork-join latency below); per kernel the
 *  settings (CComputeSettings::SKernelProviderOverride) replace them. Pure computation, thread-safe.
 */
class COMPUTEBRIDGEMATH_PUBLIC_CPP CMathCostModel
{
    public:

    /** A count no call reaches: the provider does not pay off at any batch size */
    static const UInt32 NeverCount = 0xFFFFFFFF;

    /** The fork-join latency of the job system on the reference machine, in nanoseconds: the initial value */
    static const Float64 DefaultForkJoinNs;

    /** What moving data to and from one device costs */
    struct SDeviceTransfer
    {
        Float64 latencyNs;          /**< a submission with a small copy each way, to completion */
        Float64 bytesPerNs;         /**< bus bandwidth; 0 for memory shared with the CPU: no bus term */
        bool measured;              /**< false: the numbers are the defaults */

        SDeviceTransfer( void );
    };

    CMathCostModel( void );

    void SetForkJoinCostNs( Float64 forkJoinNs );
    Float64 GetForkJoinCostNs( void ) const;

    /** The smallest count for which the MULTICORE provider with 'workerCount' workers is faster than the best single thread provider, at least two chunks */
    UInt32 ComputeMultiCoreMinCount( const SMathBridgeKernelInfo& info, UInt32 workerCount ) const;

    /** The smallest count for which the accelerator is faster than the best CPU tier; NeverCount when it never is */
    UInt32 ComputeAcceleratorMinCount( const SMathBridgeKernelInfo& info, const SDeviceTransfer& transfer, UInt32 workerCount ) const;

    /**
     *  Measures latency and bandwidth of a device with a few small copies (well under 10 ms on a discrete device):
     *  'transfer' is the default when the device is lost or the copies fail (false is returned). Unified memory
     *  devices report a bandwidth of 0.
     */
    static bool MeasureDeviceTransfer( COMPUTE::CComputeDevice& device, SDeviceTransfer& transfer );

    private:

    Float64 m_forkJoinNs;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace COMPUTEBRIDGEMATH */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMPUTEBRIDGEMATH_CMATHCOSTMODEL_H ? */
