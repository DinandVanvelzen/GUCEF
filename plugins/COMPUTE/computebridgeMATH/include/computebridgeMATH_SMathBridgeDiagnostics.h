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

#ifndef GUCEF_COMPUTEBRIDGEMATH_SMATHBRIDGEDIAGNOSTICS_H
#define GUCEF_COMPUTEBRIDGEMATH_SMATHBRIDGEDIAGNOSTICS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CTATOMIC_H
#include "gucefMT_CTAtomic.h"
#define GUCEF_MT_CTATOMIC_H
#endif /* GUCEF_MT_CTATOMIC_H ? */

#ifndef GUCEF_COMPUTEBRIDGEMATH_ETYPES_H
#include "computebridgeMATH_ETypes.h"
#define GUCEF_COMPUTEBRIDGEMATH_ETYPES_H
#endif /* GUCEF_COMPUTEBRIDGEMATH_ETYPES_H ? */

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
 *  What the providers of the bridge count and the hooks that tests use, plain data with atomics, owned by
 *  CMathBridgeGlobal. Tests do not link the plugin: they reach this struct through the 'context' of the providers the
 *  plugin registered (SMathBridgeProviderContext::diagnostics), which is why the struct is complete in a header.
 *
 *  There is no entry gate: CMathBatchDispatcher::UnregisterProviders() returns only after every call that selected a
 *  provider of the plugin has left it, so once the providers are unregistered nothing of the plugin runs on behalf of a
 *  batch call.
 */
struct SMathBridgeDiagnostics
{
    /** Per kernel: calls that reached the provider of the tier, and the calls it refused or failed (the dispatcher continued with the next tier) */
    MT::CTAtomic< UInt32 > multiCoreCalls[ MATHBATCHKERNEL_SLOTS ];
    MT::CTAtomic< UInt32 > multiCoreFailures[ MATHBATCHKERNEL_SLOTS ];
    MT::CTAtomic< UInt32 > acceleratorCalls[ MATHBATCHKERNEL_SLOTS ];
    MT::CTAtomic< UInt32 > acceleratorFailures[ MATHBATCHKERNEL_SLOTS ];

    /** Test hook: the next this many MULTICORE calls fail before they touch any output, as a provider that cannot serve a call does */
    MT::CTAtomic< UInt32 > forcedMultiCoreFailures;

    /** Test hook: items per chunk of the MULTICORE providers instead of the grain of the kernel; 0 for the grain of the kernel */
    MT::CTAtomic< UInt32 > grainOverrideItems;

    /** The device of the ACCELERATOR providers, null when there is none. For tests that make it lose its work; owned by the bridge */
    MT::CTAtomic< COMPUTE::CComputeDevice* > acceleratorDevice;

    /** How often the ACCELERATOR providers were rebuilt (a device was found, lost, or the backends changed) */
    MT::CTAtomic< UInt32 > acceleratorGeneration;

    /** The CPU kernel objects of the plugin that exist (devices and applications hold them): the code they run is the plugin's */
    MT::CTAtomic< UInt32 > liveKernels;

    SMathBridgeDiagnostics( void );

    /** Takes one forced failure when there is one */
    bool TakeForcedMultiCoreFailure( void );

    /** Clears the counters and hooks for a new load of the plugin */
    void Reset( void );

    private:
    SMathBridgeDiagnostics( const SMathBridgeDiagnostics& src );               /**< not implemented */
    SMathBridgeDiagnostics& operator=( const SMathBridgeDiagnostics& src );    /**< not implemented */
};

/*-------------------------------------------------------------------------*/

struct SMathBridgeKernelInfo;
class CMathAccelerator;

/** The 'context' of every provider the bridge registers: which kernel and tier, and the shared diagnostics */
struct SMathBridgeProviderContext
{
    MATH::TMathBatchKernel kernel;
    MATH::TMathBatchTier tier;
    SMathBridgeDiagnostics* diagnostics;
    const SMathBridgeKernelInfo* info;
    CMathAccelerator* accelerator;          /**< ACCELERATOR providers: the state they run on */

    SMathBridgeProviderContext( void );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace COMPUTEBRIDGEMATH */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMPUTEBRIDGEMATH_SMATHBRIDGEDIAGNOSTICS_H ? */
