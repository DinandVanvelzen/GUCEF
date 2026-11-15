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

#ifndef GUCEF_COMPUTEBRIDGEMATH_CMATHBRIDGEGLOBAL_H
#define GUCEF_COMPUTEBRIDGEMATH_CMATHBRIDGEGLOBAL_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_MATH_CMATHBATCHDISPATCHER_H
#include "gucefMATH_CMathBatchDispatcher.h"
#define GUCEF_MATH_CMATHBATCHDISPATCHER_H
#endif /* GUCEF_MATH_CMATHBATCHDISPATCHER_H ? */

#ifndef GUCEF_COMPUTE_CCOMPUTEGLOBAL_H
#include "gucefCOMPUTE_CComputeGlobal.h"
#define GUCEF_COMPUTE_CCOMPUTEGLOBAL_H
#endif /* GUCEF_COMPUTE_CCOMPUTEGLOBAL_H ? */

#ifndef GUCEF_COMPUTE_CCOMPUTESETTINGS_H
#include "gucefCOMPUTE_CComputeSettings.h"
#define GUCEF_COMPUTE_CCOMPUTESETTINGS_H
#endif /* GUCEF_COMPUTE_CCOMPUTESETTINGS_H ? */

#ifndef GUCEF_COMPUTEBRIDGEMATH_CMATHKERNELDECLS_H
#include "computebridgeMATH_CMathKernelDecls.h"
#define GUCEF_COMPUTEBRIDGEMATH_CMATHKERNELDECLS_H
#endif /* GUCEF_COMPUTEBRIDGEMATH_CMATHKERNELDECLS_H ? */

#ifndef GUCEF_COMPUTEBRIDGEMATH_CMATHCOSTMODEL_H
#include "computebridgeMATH_CMathCostModel.h"
#define GUCEF_COMPUTEBRIDGEMATH_CMATHCOSTMODEL_H
#endif /* GUCEF_COMPUTEBRIDGEMATH_CMATHCOSTMODEL_H ? */

#ifndef GUCEF_COMPUTEBRIDGEMATH_CMATHACCELERATOR_H
#include "computebridgeMATH_CMathAccelerator.h"
#define GUCEF_COMPUTEBRIDGEMATH_CMATHACCELERATOR_H
#endif /* GUCEF_COMPUTEBRIDGEMATH_CMATHACCELERATOR_H ? */

#ifndef GUCEF_COMPUTEBRIDGEMATH_SMATHBRIDGEDIAGNOSTICS_H
#include "computebridgeMATH_SMathBridgeDiagnostics.h"
#define GUCEF_COMPUTEBRIDGEMATH_SMATHBRIDGEDIAGNOSTICS_H
#endif /* GUCEF_COMPUTEBRIDGEMATH_SMATHBRIDGEDIAGNOSTICS_H ? */

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
 *  Everything the plugin owns, in one place (the module global of computebridgeMATH): the table of bridged kernels, the
 *  cost model, the diagnostics and entry gate, the provider contexts, the kernel factories and the accelerator state.
 *  Load() does what the design asks of a bridge (COMPUTE design 5.2): it registers the kernel declarations and CPU
 *  artefacts with gucefCOMPUTE, the MULTICORE providers (CPU backend, fork-join) and the ACCELERATOR providers (best
 *  device that is not the CPU) with the batch dispatch table of gucefMATH, and listens to devices and backends;
 *  Unload() removes all of it again, in reverse.
 *
 *  Settings: the per kernel overrides of CComputeSettings (forced tier, disabled tiers, minimum count per tier) are read
 *  by Load(); load the plugin again to apply changed settings. A forced tier of MULTICORE or ACCELERATOR registers that
 *  tier with a minimum count of 0 and no tier above it; a forced lower tier registers none of the bridge's tiers.
 *
 *  Lifetime: the object is made at the first use. Unload() removes the providers (the dispatcher returns from that after
 *  the last batch call that could have reached one has left it, so no code of the plugin runs for a batch call any more),
 *  the kernels and the device. The object itself, and the plugin image, stay for as long as CPU kernel objects made by the
 *  plugin exist (SMathBridgeDiagnostics::liveKernels): devices and applications hold them and their code is the plugin's.
 *  Deinstance() destroys the object when there are none, and tells the plugin entry point whether the image may go.
 *
 *  Thread-safe. Load(), Unload() and the refresh of the accelerator providers are serialised.
 */
class COMPUTEBRIDGEMATH_PUBLIC_CPP CMathBridgeGlobal
{
    public:

    /** The global, made on first use */
    static CMathBridgeGlobal* Instance( void );

    /**
     *  Unloads and destroys the global when no CPU kernel object of the plugin exists. False (nothing destroyed) when some do:
     *  the plugin code must then stay mapped and Instance() keeps the object
     */
    static bool Deinstance( void );

    /** Registers the kernels and the providers; false when gucefCOMPUTE or gucefMATH is not there or a kernel could not be registered (nothing stays registered then) */
    bool Load( void );

    /** Removes providers and kernels; waits for the provider calls that are running */
    void Unload( void );

    bool IsLoaded( void ) const;

    SMathBridgeDiagnostics& GetDiagnostics( void );

    const CMathKernelDecls& GetKernelDecls( void ) const;

    /** The number of kernels that Load() bridged (those the MATH build has providers for) */
    UInt32 GetBridgedKernelCount( void ) const;

    /** The minimum count the MULTICORE provider of a kernel was registered with; CMathCostModel::NeverCount when it is not registered */
    UInt32 GetMultiCoreMinCount( MATH::TMathBatchKernel kernel ) const;

    /** As above for the ACCELERATOR provider */
    UInt32 GetAcceleratorMinCount( MATH::TMathBatchKernel kernel ) const;

    /**
     *  Rebuilds the ACCELERATOR providers for the best device now: the availability of backends changed. A request that
     *  arrives while a rebuild runs makes it run once more. The lock of the global is never held while a device is made
     *  or released: those touch pulse generators, whose locks are held while events are delivered to the handlers that
     *  call this, so the lock order would be a cycle
     */
    void RefreshAcceleratorProviders( void );

    /** Removes the ACCELERATOR providers and releases the device: it was lost */
    void RemoveAcceleratorProviders( void );

    private:

    CMathBridgeGlobal( void );
    ~CMathBridgeGlobal();

    bool RegisterKernels( COMPUTE::CComputeGlobal& compute, const MATH::CMathBatchDispatcher& dispatcher );
    void UnregisterKernels( void );
    void RegisterMultiCoreProviders( COMPUTE::CComputeGlobal& compute, MATH::CMathBatchDispatcher& dispatcher, const COMPUTE::CComputeSettings& settings );
    void RefreshAcceleratorOnce( void );
    void RegisterAcceleratorProviders( COMPUTE::CComputeGlobal& compute, MATH::CMathBatchDispatcher& dispatcher, const COMPUTE::CComputeSettings& settings,
                                       CMathAccelerator::SState& state, const CMathCostModel::SDeviceTransfer& transfer );
    void RemoveAcceleratorProvidersLocked( void );
    void WaitForQuiescence( void );

    CMathBridgeGlobal( const CMathBridgeGlobal& src );              /**< not implemented */
    CMathBridgeGlobal& operator=( const CMathBridgeGlobal& src );   /**< not implemented */

    private:

    enum EState { STATE_UNLOADED = 0, STATE_LOADED = 1, STATE_UNLOADING = 2 };

    SMathBridgeDiagnostics m_diagnostics;
    CMathKernelDecls m_decls;
    CMathCostModel m_costModel;
    COMPUTE::TCpuComputeKernelFactory* m_factories[ CMathKernelDecls::KernelRowCount ];
    SMathBridgeProviderContext m_multiCoreContexts[ MATHBATCHKERNEL_SLOTS ];
    SMathBridgeProviderContext m_acceleratorContexts[ MATHBATCHKERNEL_SLOTS ];
    UInt32 m_multiCoreMinCounts[ MATHBATCHKERNEL_SLOTS ];
    UInt32 m_acceleratorMinCounts[ MATHBATCHKERNEL_SLOTS ];
    bool m_bridged[ CMathKernelDecls::KernelRowCount ];
    CMathAccelerator* m_accelerator;
    MT::CTAtomic< UInt32 > m_state;
    MT::CTAtomic< UInt32 > m_refreshing;            /**< one refresh of the accelerator providers at a time */
    MT::CTAtomic< UInt32 > m_refreshRequested;      /**< a request that came in while one was running */
    MT::CTAtomic< UInt32 > m_eventCalls;            /**< event handlers that are running, which an unload waits for */
    MT::CMutex m_lock;

    // the addresses of these are the owners the registrations are made with
    char m_kernelOwner;
    char m_multiCoreOwner;
    char m_acceleratorOwner;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace COMPUTEBRIDGEMATH */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMPUTEBRIDGEMATH_CMATHBRIDGEGLOBAL_H ? */
