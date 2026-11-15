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

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_MATH_CMATHBATCHDISPATCHER_H
#include "gucefMATH_CMathBatchDispatcher.h"
#define GUCEF_MATH_CMATHBATCHDISPATCHER_H
#endif /* GUCEF_MATH_CMATHBATCHDISPATCHER_H ? */

#ifndef GUCEF_MATH_CMATHGLOBAL_H
#include "gucefMATH_CMathGlobal.h"
#define GUCEF_MATH_CMATHGLOBAL_H
#endif /* GUCEF_MATH_CMATHGLOBAL_H ? */

#ifndef GUCEF_COMPUTE_CCOMPUTEGLOBAL_H
#include "gucefCOMPUTE_CComputeGlobal.h"
#define GUCEF_COMPUTE_CCOMPUTEGLOBAL_H
#endif /* GUCEF_COMPUTE_CCOMPUTEGLOBAL_H ? */

#ifndef GUCEF_COMPUTE_CCOMPUTEKERNELREGISTRY_H
#include "gucefCOMPUTE_CComputeKernelRegistry.h"
#define GUCEF_COMPUTE_CCOMPUTEKERNELREGISTRY_H
#endif /* GUCEF_COMPUTE_CCOMPUTEKERNELREGISTRY_H ? */

#ifndef GUCEF_COMPUTEBRIDGEMATH_CMATHACCELERATOR_H
#include "computebridgeMATH_CMathAccelerator.h"
#define GUCEF_COMPUTEBRIDGEMATH_CMATHACCELERATOR_H
#endif /* GUCEF_COMPUTEBRIDGEMATH_CMATHACCELERATOR_H ? */

#include "computebridgeMATH_CMathBridgeGlobal.h"

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

/** The most an event handler or a refresh that is running is given to finish when the plugin unloads */
const UInt32 QuiescenceTimeoutInMs = 30000;

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

// made at the first use; destroyed when the plugin unloads and no CPU kernel object of it exists any more (see Deinstance())
static CMathBridgeGlobal* g_instance = GUCEF_NULL;

/*-------------------------------------------------------------------------*/

CMathBridgeGlobal*
CMathBridgeGlobal::Instance( void )
{GUCEF_TRACE;

    // called by the plugin entry points only, which the plugin control calls one at a time
    if ( GUCEF_NULL == g_instance )
        g_instance = GUCEF_NEW CMathBridgeGlobal();
    return g_instance;
}

/*-------------------------------------------------------------------------*/

bool
CMathBridgeGlobal::Deinstance( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == g_instance )
        return true;
    g_instance->Unload();
    if ( 0 != g_instance->m_diagnostics.liveKernels.Load() )
        return false;
    GUCEF_DELETE g_instance;
    g_instance = GUCEF_NULL;
    return true;
}

/*-------------------------------------------------------------------------*/

CMathBridgeGlobal::CMathBridgeGlobal( void )
    : m_diagnostics()
    , m_decls()
    , m_costModel()
    , m_accelerator( GUCEF_NULL )
    , m_state( STATE_UNLOADED )
    , m_refreshing( 0 )
    , m_refreshRequested( 0 )
    , m_eventCalls( 0 )
    , m_lock()
    , m_kernelOwner( 0 )
    , m_multiCoreOwner( 0 )
    , m_acceleratorOwner( 0 )
{GUCEF_TRACE;

    for ( UInt32 i=0; i<CMathKernelDecls::KernelRowCount; ++i )
    {
        m_factories[ i ] = m_decls.GetKernelInfo( i ).createCpuFactory();
        m_bridged[ i ] = false;
    }
    for ( UInt32 k=0; k<MATHBATCHKERNEL_SLOTS; ++k )
    {
        m_multiCoreMinCounts[ k ] = CMathCostModel::NeverCount;
        m_acceleratorMinCounts[ k ] = CMathCostModel::NeverCount;
    }
}

/*-------------------------------------------------------------------------*/

CMathBridgeGlobal::~CMathBridgeGlobal()
{GUCEF_TRACE;

    // only reached by an owner that destroys the instance explicitly: nothing may be in flight then
    Unload();
    for ( UInt32 i=0; i<CMathKernelDecls::KernelRowCount; ++i )
    {
        GUCEF_DELETE m_factories[ i ];
        m_factories[ i ] = GUCEF_NULL;
    }
}

/*-------------------------------------------------------------------------*/

SMathBridgeDiagnostics&
CMathBridgeGlobal::GetDiagnostics( void )
{GUCEF_TRACE;

    return m_diagnostics;
}

/*-------------------------------------------------------------------------*/

const CMathKernelDecls&
CMathBridgeGlobal::GetKernelDecls( void ) const
{GUCEF_TRACE;

    return m_decls;
}

/*-------------------------------------------------------------------------*/

bool
CMathBridgeGlobal::IsLoaded( void ) const
{GUCEF_TRACE;

    return STATE_LOADED == m_state.Load();
}

/*-------------------------------------------------------------------------*/

UInt32
CMathBridgeGlobal::GetBridgedKernelCount( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    UInt32 count = 0;
    for ( UInt32 i=0; i<CMathKernelDecls::KernelRowCount; ++i )
        count += m_bridged[ i ] ? 1 : 0;
    return count;
}

/*-------------------------------------------------------------------------*/

UInt32
CMathBridgeGlobal::GetMultiCoreMinCount( MATH::TMathBatchKernel kernel ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return (UInt32) kernel < MATHBATCHKERNEL_SLOTS ? m_multiCoreMinCounts[ kernel ] : CMathCostModel::NeverCount;
}

/*-------------------------------------------------------------------------*/

UInt32
CMathBridgeGlobal::GetAcceleratorMinCount( MATH::TMathBatchKernel kernel ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return (UInt32) kernel < MATHBATCHKERNEL_SLOTS ? m_acceleratorMinCounts[ kernel ] : CMathCostModel::NeverCount;
}

/*-------------------------------------------------------------------------*/

bool
CMathBridgeGlobal::RegisterKernels( COMPUTE::CComputeGlobal& compute, const MATH::CMathBatchDispatcher& dispatcher )
{GUCEF_TRACE;

    COMPUTE::CComputeKernelRegistry& registry = compute.GetKernelRegistry();
    bool allRegistered = true;
    for ( UInt32 i=0; i<m_decls.GetKernelCount(); ++i )
    {
        const SMathBridgeKernelInfo& info = m_decls.GetKernelInfo( i );
        const char* kernelId = dispatcher.GetKernelId( info.kernel );
        if ( GUCEF_NULL == kernelId || 0 == dispatcher.GetProviderCount( info.kernel ) )
        {
            GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "computebridgeMATH: the MATH batch kernel " << (UInt32) info.kernel << " has no provider in this build of gucefMATH, it is not bridged" );
            continue;
        }

        COMPUTE::CComputeKernelDecl decl;
        CMathKernelDecls::MakeDecl( info, kernelId, decl );
        COMPUTE::CComputeKernelArtefact artefact = COMPUTE::CComputeKernelArtefact::ForCpu( m_factories[ i ] );
        artefact.SetInterfaceVersion( CMathKernelDecls::InterfaceVersion );
        if ( !registry.RegisterKernel( decl, &m_kernelOwner ) || !registry.RegisterArtefact( decl.GetKernelId(), artefact ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "computebridgeMATH: unable to register the compute kernel \"" << kernelId << "\", is it registered already?" );
            allRegistered = false;
            continue;
        }
        m_bridged[ i ] = true;
    }
    return allRegistered;
}

/*-------------------------------------------------------------------------*/

void
CMathBridgeGlobal::UnregisterKernels( void )
{GUCEF_TRACE;

    // the registry of the compute global that exists now: after an unload of the module the one registered with is gone
    COMPUTE::CComputeGlobal* compute = COMPUTE::CComputeGlobal::InstanceIfCreated();
    if ( GUCEF_NULL != compute )
        compute->GetKernelRegistry().UnregisterModuleKernels( &m_kernelOwner );
    for ( UInt32 i=0; i<CMathKernelDecls::KernelRowCount; ++i )
        m_bridged[ i ] = false;
}

/*-------------------------------------------------------------------------*/

void
CMathBridgeGlobal::RegisterMultiCoreProviders( COMPUTE::CComputeGlobal& compute, MATH::CMathBatchDispatcher& dispatcher, const COMPUTE::CComputeSettings& settings )
{GUCEF_TRACE;

    const UInt32 workerCount = compute.GetCpuParallelFor().GetWorkerCount();
    for ( UInt32 i=0; i<m_decls.GetKernelCount(); ++i )
    {
        const SMathBridgeKernelInfo& info = m_decls.GetKernelInfo( i );
        m_multiCoreMinCounts[ info.kernel ] = CMathCostModel::NeverCount;
        if ( !m_bridged[ i ] )
            continue;

        COMPUTE::CComputeSettings::SKernelProviderOverride override;
        settings.GetKernelProviderOverride( dispatcher.GetKernelId( info.kernel ), override );
        if ( COMPUTE::PROVIDERTIER_NONE != override.forcedTier && override.forcedTier < COMPUTE::PROVIDERTIER_MULTICORE )
            continue;
        if ( override.IsTierDisabled( COMPUTE::PROVIDERTIER_MULTICORE ) )
            continue;

        UInt32 minCount = override.minCount[ COMPUTE::PROVIDERTIER_MULTICORE ];
        if ( COMPUTE::PROVIDERTIER_MULTICORE == override.forcedTier )
            minCount = 0;
        else if ( 0 == minCount )
            minCount = m_costModel.ComputeMultiCoreMinCount( info, workerCount );
        if ( CMathCostModel::NeverCount == minCount )
            continue;

        SMathBridgeProviderContext& context = m_multiCoreContexts[ info.kernel ];
        context.kernel = info.kernel;
        context.tier = MATH::MATHBATCHTIER_MULTICORE;
        context.diagnostics = &m_diagnostics;
        context.info = &info;
        context.accelerator = GUCEF_NULL;

        MATH::SMathBatchProvider provider;
        provider.tier = MATH::MATHBATCHTIER_MULTICORE;
        provider.name = "computebridgeMATH multi-core";
        provider.function = info.multiCoreFunction;
        provider.minCount = minCount;
        provider.maxCount = 0xFFFFFFFF;
        provider.flags = MATH::MATHPROVIDER_BITEXACT | MATH::MATHPROVIDER_MAY_FAIL;
        provider.alignment = 0;
        provider.requiredCpuFeatures = 0;
        provider.context = &context;
        if ( dispatcher.RegisterProvider( info.kernel, provider, &m_multiCoreOwner ) )
            m_multiCoreMinCounts[ info.kernel ] = minCount;
        else
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "computebridgeMATH: unable to register the multi-core provider of kernel \"" << dispatcher.GetKernelId( info.kernel ) << "\"" );
    }
}

/*-------------------------------------------------------------------------*/

void
CMathBridgeGlobal::RegisterAcceleratorProviders( COMPUTE::CComputeGlobal& compute, MATH::CMathBatchDispatcher& dispatcher, const COMPUTE::CComputeSettings& settings,
                                                 CMathAccelerator::SState& state, const CMathCostModel::SDeviceTransfer& transfer )
{GUCEF_TRACE;

    // called with m_lock held: only the table of providers and the books of this object are touched here, no shared pointer
    // is copied or released (their locks would join the lock of this object in a cycle with the pulse generators)
    const UInt32 workerCount = compute.GetCpuParallelFor().GetWorkerCount();
    const UInt64 maxBindingBytes = state.device->GetCapabilities().GetMaxBindingSizeInBytes();

    for ( UInt32 i=0; i<m_decls.GetKernelCount(); ++i )
    {
        const SMathBridgeKernelInfo& info = m_decls.GetKernelInfo( i );
        if ( !m_bridged[ i ] || GUCEF_NULL == info.acceleratorFunction || state.kernels[ info.kernel ].IsNULL() )
            continue;

        COMPUTE::CComputeSettings::SKernelProviderOverride override;
        settings.GetKernelProviderOverride( dispatcher.GetKernelId( info.kernel ), override );
        if ( COMPUTE::PROVIDERTIER_NONE != override.forcedTier && override.forcedTier < COMPUTE::PROVIDERTIER_ACCELERATOR )
            continue;
        if ( override.IsTierDisabled( COMPUTE::PROVIDERTIER_ACCELERATOR ) )
            continue;

        UInt32 minCount = override.minCount[ COMPUTE::PROVIDERTIER_ACCELERATOR ];
        if ( COMPUTE::PROVIDERTIER_ACCELERATOR == override.forcedTier )
            minCount = 0;
        else if ( 0 == minCount )
            minCount = m_costModel.ComputeAcceleratorMinCount( info, transfer, workerCount );
        if ( CMathCostModel::NeverCount == minCount )
            continue;

        SMathBridgeProviderContext& context = m_acceleratorContexts[ info.kernel ];
        context.kernel = info.kernel;
        context.tier = MATH::MATHBATCHTIER_ACCELERATOR;
        context.diagnostics = &m_diagnostics;
        context.info = &info;
        context.accelerator = m_accelerator;

        const UInt64 maxItems = maxBindingBytes / ( info.bytesPerItem > 0 ? info.bytesPerItem : 1 );

        MATH::SMathBatchProvider provider;
        provider.tier = MATH::MATHBATCHTIER_ACCELERATOR;
        provider.name = "computebridgeMATH accelerator";
        provider.function = info.acceleratorFunction;
        provider.minCount = minCount;
        provider.maxCount = maxItems > 0xFFFFFFFFULL ? 0xFFFFFFFF : static_cast< UInt32 >( maxItems );
        provider.flags = MATH::MATHPROVIDER_MAY_FAIL | ( COMPUTE::DETERMINISM_EXACT_INTEGER == info.determinism ? MATH::MATHPROVIDER_BITEXACT : 0 );
        provider.alignment = 0;
        provider.requiredCpuFeatures = 0;
        provider.context = &context;
        if ( provider.minCount <= provider.maxCount && dispatcher.RegisterProvider( info.kernel, provider, &m_acceleratorOwner ) )
            m_acceleratorMinCounts[ info.kernel ] = minCount;
    }
    m_diagnostics.acceleratorDevice.Store( state.device.GetPointerAlways() );
}

/*-------------------------------------------------------------------------*/

void
CMathBridgeGlobal::RefreshAcceleratorOnce( void )
{GUCEF_TRACE;

    CMathAccelerator* accelerator = GUCEF_NULL;
    {
        MT::CScopeMutex lock( m_lock );
        if ( STATE_LOADED != m_state.Load() )
            return;
        accelerator = m_accelerator;
    }
    COMPUTE::CComputeGlobal* compute = COMPUTE::CComputeGlobal::InstanceIfCreated();
    MATH::CMathGlobal* mathGlobal = MATH::CMathGlobal::Instance();
    if ( GUCEF_NULL == compute || GUCEF_NULL == mathGlobal || GUCEF_NULL == accelerator )
        return;
    const COMPUTE::CComputeSettings settings = compute->GetSettings();

    // the device, its kernels and the measurement of its transfer cost, with no lock held (see RefreshAcceleratorProviders)
    CMathAccelerator::TStatePtr state;
    CMathCostModel::SDeviceTransfer transfer;
    if ( accelerator->CreateState( state ) )
    {
        CMathCostModel::MeasureDeviceTransfer( *state->device, transfer );
        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "computebridgeMATH: transfer cost of the accelerator device: latency " << transfer.latencyNs << " ns, bus bandwidth " << transfer.bytesPerNs <<
                          " bytes per ns" << ( transfer.bytesPerNs > 0.0 ? "" : " (none: shared memory)" ) << ( transfer.measured ? "" : ", the defaults" ) );
    }

    CMathAccelerator::TStatePtr displaced;
    bool loaded = false;
    {
        MT::CScopeMutex lock( m_lock );
        if ( STATE_LOADED == m_state.Load() )
        {
            loaded = true;
            MATH::CMathBatchDispatcher& dispatcher = mathGlobal->GetBatchDispatcher();
            dispatcher.UnregisterProviders( &m_acceleratorOwner );
            for ( UInt32 k=0; k<MATHBATCHKERNEL_SLOTS; ++k )
                m_acceleratorMinCounts[ k ] = CMathCostModel::NeverCount;
            m_diagnostics.acceleratorDevice.Store( GUCEF_NULL );
            if ( !state.IsNULL() )
                RegisterAcceleratorProviders( *compute, dispatcher, settings, *state.GetPointerAlways(), transfer );
        }
    }
    // The state goes in use after the lock of this object is let go: installing it copies shared pointers, whose locks the
    // release of a device takes the lock of a pulse generator under, and an event of a pulse generator takes ours. Calls that
    // reach a provider in between find no state and decline: the next tier serves them.
    if ( loaded )
    {
        accelerator->InstallState( state, displaced );
        m_diagnostics.acceleratorGeneration.FetchAdd( 1 );
    }
    // the states that go: the one that was in use, and one that was not installed (the plugin unloaded meanwhile)
    displaced.Unlink();
    state.Unlink();
}

/*-------------------------------------------------------------------------*/

bool
CMathBridgeGlobal::Load( void )
{GUCEF_TRACE;

    CMathAccelerator* accelerator = GUCEF_NULL;
    UInt32 multiCore = 0;
    UInt32 workerCount = 0;
    {
        MT::CScopeMutex lock( m_lock );
        if ( STATE_LOADED == m_state.Load() )
            return true;

        COMPUTE::CComputeGlobal* compute = COMPUTE::CComputeGlobal::Instance();
        MATH::CMathGlobal* mathGlobal = MATH::CMathGlobal::Instance();
        if ( GUCEF_NULL == compute || GUCEF_NULL == mathGlobal )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "computebridgeMATH: gucefCOMPUTE or gucefMATH is not available" );
            return false;
        }
        MATH::CMathBatchDispatcher& dispatcher = mathGlobal->GetBatchDispatcher();

        m_diagnostics.Reset();
        if ( !RegisterKernels( *compute, dispatcher ) )
        {
            UnregisterKernels();
            return false;
        }

        const COMPUTE::CComputeSettings settings = compute->GetSettings();
        RegisterMultiCoreProviders( *compute, dispatcher, settings );

        m_accelerator = GUCEF_NEW CMathAccelerator( *this );
        accelerator = m_accelerator;
        m_state.Store( STATE_LOADED );

        for ( UInt32 k=0; k<MATHBATCHKERNEL_SLOTS; ++k )
            multiCore += CMathCostModel::NeverCount != m_multiCoreMinCounts[ k ] ? 1 : 0;
        workerCount = compute->GetCpuParallelFor().GetWorkerCount();
    }

    // the accelerator: listening to the backends and making a device touch notifiers, which no lock of ours may be held for
    accelerator->SubscribeToBackends();
    RefreshAcceleratorProviders();

    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "computebridgeMATH: loaded, " << multiCore << " multi-core providers over " << workerCount << " workers, accelerator device: " <<
                      ( GUCEF_NULL != m_diagnostics.acceleratorDevice.Load() ? "yes" : "none" ) );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CMathBridgeGlobal::WaitForQuiescence( void )
{GUCEF_TRACE;

    // The provider calls need no waiting for: UnregisterProviders() returned after the last call that could reach a
    // provider of the plugin had left it. What is left are the event handlers and a refresh of the accelerator providers
    UInt32 waited = 0;
    while ( ( 0 != m_eventCalls.Load() || 0 != m_refreshing.Load() ) && waited < QuiescenceTimeoutInMs )
    {
        MT::ThreadDelay( 1 );
        ++waited;
    }
    if ( 0 != m_eventCalls.Load() || 0 != m_refreshing.Load() )
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "computebridgeMATH: an event handler or refresh did not end within " << QuiescenceTimeoutInMs << " ms of the unload" );
}

/*-------------------------------------------------------------------------*/

void
CMathBridgeGlobal::Unload( void )
{GUCEF_TRACE;

    CMathAccelerator* accelerator = GUCEF_NULL;
    {
        MT::CScopeMutex lock( m_lock );
        if ( STATE_LOADED != m_state.Load() )
            return;
        m_state.Store( STATE_UNLOADING );
        accelerator = m_accelerator;
    }

    // outside the lock: an event handler that is running waits for the lock, and unsubscribing waits for that handler
    if ( GUCEF_NULL != accelerator )
    {
        accelerator->UnsubscribeFromBackends();
    }

    // Each call returns after the last batch call that could have reached a provider of the plugin has left it
    MATH::CMathGlobal* mathGlobal = MATH::CMathGlobal::Instance();
    if ( GUCEF_NULL != mathGlobal )
    {
        MATH::CMathBatchDispatcher& dispatcher = mathGlobal->GetBatchDispatcher();
        dispatcher.UnregisterProviders( &m_acceleratorOwner );
        dispatcher.UnregisterProviders( &m_multiCoreOwner );
    }

    // the event handlers and a refresh of the accelerator providers that is running
    WaitForQuiescence();

    // the device and its kernels: a refresh that was running may have installed a state before it saw the unload
    if ( GUCEF_NULL != accelerator )
        accelerator->ReleaseState();
    {
        MT::CScopeMutex lock( m_lock );
        UnregisterKernels();
        m_accelerator = GUCEF_NULL;
        for ( UInt32 k=0; k<MATHBATCHKERNEL_SLOTS; ++k )
        {
            m_multiCoreMinCounts[ k ] = CMathCostModel::NeverCount;
            m_acceleratorMinCounts[ k ] = CMathCostModel::NeverCount;
        }
        m_diagnostics.acceleratorDevice.Store( GUCEF_NULL );
        for ( UInt32 k=0; k<MATHBATCHKERNEL_SLOTS; ++k )
            m_acceleratorContexts[ k ].accelerator = GUCEF_NULL;
        m_state.Store( STATE_UNLOADED );
    }
    GUCEF_DELETE accelerator;
    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "computebridgeMATH: unloaded" );
}

/*-------------------------------------------------------------------------*/

void
CMathBridgeGlobal::RefreshAcceleratorProviders( void )
{GUCEF_TRACE;

    // counted in for the unload, which waits for the handlers that run
    m_eventCalls.FetchAdd( 1 );

    for ( ;; )
    {
        UInt32 expected = 0;
        if ( !m_refreshing.CompareExchangeStrong( expected, 1 ) )
        {
            // a refresh is running: it makes another round for this request
            m_refreshRequested.Store( 1 );
            break;
        }
        do
        {
            m_refreshRequested.Store( 0 );
            RefreshAcceleratorOnce();
        }
        while ( 0 != m_refreshRequested.Load() );
        m_refreshing.Store( 0 );
        if ( 0 == m_refreshRequested.Load() )
            break;
    }
    m_eventCalls.FetchSub( 1 );
}

/*-------------------------------------------------------------------------*/

void
CMathBridgeGlobal::RemoveAcceleratorProviders( void )
{GUCEF_TRACE;

    // This runs in the delivery of the loss event of the device, with the lock of its pulse generator held: it only
    // touches the table of providers and the books, never a shared pointer to a state or a device (see InstallState())
    m_eventCalls.FetchAdd( 1 );
    {
        MT::CScopeMutex lock( m_lock );
        if ( STATE_LOADED == m_state.Load() )
        {
            MATH::CMathGlobal* mathGlobal = MATH::CMathGlobal::Instance();
            if ( GUCEF_NULL != mathGlobal )
                mathGlobal->GetBatchDispatcher().UnregisterProviders( &m_acceleratorOwner );
            for ( UInt32 k=0; k<MATHBATCHKERNEL_SLOTS; ++k )
                m_acceleratorMinCounts[ k ] = CMathCostModel::NeverCount;
            m_diagnostics.acceleratorDevice.Store( GUCEF_NULL );
            m_diagnostics.acceleratorGeneration.FetchAdd( 1 );
        }
    }
    m_eventCalls.FetchSub( 1 );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace COMPUTEBRIDGEMATH */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
