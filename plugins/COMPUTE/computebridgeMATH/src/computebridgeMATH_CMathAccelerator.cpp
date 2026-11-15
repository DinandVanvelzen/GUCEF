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

#ifndef GUCEF_COMPUTE_CCOMPUTEBACKENDREGISTRY_H
#include "gucefCOMPUTE_CComputeBackendRegistry.h"
#define GUCEF_COMPUTE_CCOMPUTEBACKENDREGISTRY_H
#endif /* GUCEF_COMPUTE_CCOMPUTEBACKENDREGISTRY_H ? */

#ifndef GUCEF_COMPUTEBRIDGEMATH_CMATHBRIDGEGLOBAL_H
#include "computebridgeMATH_CMathBridgeGlobal.h"
#define GUCEF_COMPUTEBRIDGEMATH_CMATHBRIDGEGLOBAL_H
#endif /* GUCEF_COMPUTEBRIDGEMATH_CMATHBRIDGEGLOBAL_H ? */

#include "computebridgeMATH_CMathAccelerator.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COMPUTEBRIDGEMATH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CMathAccelerator::SState::SState( void )
    : device()
    , batch( GUCEF_NULL )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMathAccelerator::SState::~SState()
{GUCEF_TRACE;

    // the host batch waits for its work in flight and must go before the kernels and the device it uses
    GUCEF_DELETE batch;
    batch = GUCEF_NULL;
    for ( UInt32 i=0; i<MATHBATCHKERNEL_SLOTS; ++i )
        kernels[ i ].Unlink();
}

/*-------------------------------------------------------------------------*/

CMathAccelerator::CMathAccelerator( CMathBridgeGlobal& owner )
    : CORE::CObserver()
    , m_owner( owner )
    , m_state()
    , m_lock()
    , m_subscribedToBackends( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMathAccelerator::~CMathAccelerator()
{GUCEF_TRACE;

    UnsubscribeAllFromObserver();
}

/*-------------------------------------------------------------------------*/

void
CMathAccelerator::SubscribeToBackends( void )
{GUCEF_TRACE;

    COMPUTE::CComputeGlobal* compute = COMPUTE::CComputeGlobal::InstanceIfCreated();
    if ( GUCEF_NULL == compute || m_subscribedToBackends )
        return;

    TEventCallback callback( this, &CMathAccelerator::OnBackendAvailabilityChanged );
    SubscribeTo( &compute->GetBackendRegistry().AsNotifier(), COMPUTE::CComputeBackendRegistry::BackendAvailabilityChangedEvent, callback );
    m_subscribedToBackends = true;
}

/*-------------------------------------------------------------------------*/

void
CMathAccelerator::UnsubscribeFromBackends( void )
{GUCEF_TRACE;

    COMPUTE::CComputeGlobal* compute = COMPUTE::CComputeGlobal::InstanceIfCreated();
    if ( GUCEF_NULL != compute && m_subscribedToBackends )
        UnsubscribeFrom( compute->GetBackendRegistry().AsNotifier() );
    m_subscribedToBackends = false;
}

/*-------------------------------------------------------------------------*/

bool
CMathAccelerator::CreateState( TStatePtr& createdState )
{GUCEF_TRACE;

    createdState.Unlink();
    COMPUTE::CComputeGlobal* compute = COMPUTE::CComputeGlobal::InstanceIfCreated();
    MATH::CMathGlobal* mathGlobal = MATH::CMathGlobal::Instance();
    if ( GUCEF_NULL == compute || GUCEF_NULL == mathGlobal )
        return false;

    // any device but the CPU: the multi-core tier is the CPU backend
    COMPUTE::CComputeDeviceRequirements requirements;
    requirements.SetCpuDeviceAcceptable( false );
    COMPUTE::TComputeDevicePtr device;
    if ( !compute->GetBackendRegistry().CreateBestDevice( requirements, device ) || device.IsNULL() )
        return false;

    TStatePtr state( GUCEF_NEW SState() );
    state->device = device;
    state->batch = GUCEF_NEW COMPUTE::CComputeHostBatch( device );

    // the kernels that exist on the device: the others have no accelerator provider
    const MATH::CMathBatchDispatcher& dispatcher = mathGlobal->GetBatchDispatcher();
    const CMathKernelDecls& decls = m_owner.GetKernelDecls();
    UInt32 kernelCount = 0;
    for ( UInt32 i=0; i<decls.GetKernelCount(); ++i )
    {
        const SMathBridgeKernelInfo& info = decls.GetKernelInfo( i );
        const char* mathKernelId = dispatcher.GetKernelId( info.kernel );
        if ( GUCEF_NULL == info.acceleratorFunction || GUCEF_NULL == mathKernelId )
            continue;
        const CString kernelId = mathKernelId;
        if ( !device->IsKernelAvailable( kernelId ) )
            continue;
        if ( device->CreateKernel( kernelId, state->kernels[ info.kernel ] ) )
            ++kernelCount;
    }
    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "computebridgeMATH: accelerator device \"" << device->GetInfo().GetDeviceName() << "\" of backend \"" << device->GetBackendName() <<
                      "\" serves " << kernelCount << " kernels" );

    TEventCallback callback( this, &CMathAccelerator::OnDeviceLost );
    SubscribeTo( device.GetPointerAlways(), COMPUTE::CComputeDevice::DeviceLostEvent, callback );
    createdState = state;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CMathAccelerator::InstallState( const TStatePtr& state, TStatePtr& displaced )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    displaced = m_state;
    m_state = state;
}

/*-------------------------------------------------------------------------*/

void
CMathAccelerator::ReleaseState( void )
{GUCEF_TRACE;

    TStatePtr state;
    {
        MT::CScopeMutex lock( m_lock );
        state = m_state;
        m_state.Unlink();
    }
    if ( !state.IsNULL() && !state->device.IsNULL() )
        UnsubscribeFrom( *state->device.GetPointerAlways() );

    // the last reference goes outside the lock: a state waits for the work of its host batch
    state.Unlink();
}

/*-------------------------------------------------------------------------*/

bool
CMathAccelerator::GetState( TStatePtr& state ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    if ( m_state.IsNULL() || m_state->device->IsLost() )
        return false;
    state = m_state;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CMathAccelerator::OnDeviceLost( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "computebridgeMATH: the accelerator device was lost, its providers are removed and the next tier serves the calls" );
    m_owner.RemoveAcceleratorProviders();
}

/*-------------------------------------------------------------------------*/

void
CMathAccelerator::OnBackendAvailabilityChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    m_owner.RefreshAcceleratorProviders();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace COMPUTEBRIDGEMATH */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
