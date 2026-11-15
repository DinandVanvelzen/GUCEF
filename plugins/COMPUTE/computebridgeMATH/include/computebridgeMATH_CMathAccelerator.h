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

#ifndef GUCEF_COMPUTEBRIDGEMATH_CMATHACCELERATOR_H
#define GUCEF_COMPUTEBRIDGEMATH_CMATHACCELERATOR_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_COBSERVER_H
#include "CObserver.h"
#define GUCEF_CORE_COBSERVER_H
#endif /* GUCEF_CORE_COBSERVER_H ? */

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

#ifndef GUCEF_COMPUTE_CCOMPUTEDEVICE_H
#include "gucefCOMPUTE_CComputeDevice.h"
#define GUCEF_COMPUTE_CCOMPUTEDEVICE_H
#endif /* GUCEF_COMPUTE_CCOMPUTEDEVICE_H ? */

#ifndef GUCEF_COMPUTE_CCOMPUTEHOSTBATCH_H
#include "gucefCOMPUTE_CComputeHostBatch.h"
#define GUCEF_COMPUTE_CCOMPUTEHOSTBATCH_H
#endif /* GUCEF_COMPUTE_CCOMPUTEHOSTBATCH_H ? */

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

class CMathBridgeGlobal;

/**
 *  The device side of the ACCELERATOR providers (COMPUTE design 5.4): the best device that is not the CPU, a
 *  CComputeHostBatch over it and the kernels created on it. The providers take a reference to the state per call, so a
 *  device that is replaced or lost while a call runs stays alive until the call ends.
 *
 *  Events: the object subscribes to DeviceLostEvent of its device and to BackendAvailabilityChangedEvent of the backend
 *  registry. Both reach it on the thread of the pulse generator of the notifier (the application's main loop for devices
 *  made with default parameters). A device that is lost has its providers removed at once; the state is rebuilt when
 *  the availability of backends changes, and through CMathBridgeGlobal::RefreshAcceleratorProviders(). The providers
 *  also notice a lost device themselves (IsLost()) and decline calls, so the result never depends on the event having
 *  been delivered yet.
 *
 *  Thread-safe.
 */
class CMathAccelerator : public CORE::CObserver
{
    public:

    /** What the providers use: one device with its host batch and kernels. Released when the last reference goes */
    struct SState
    {
        COMPUTE::TComputeDevicePtr device;
        COMPUTE::CComputeHostBatch* batch;
        COMPUTE::TComputeKernelPtr kernels[ MATHBATCHKERNEL_SLOTS ];

        SState( void );
        ~SState();

        private:
        SState( const SState& src );                /**< not implemented */
        SState& operator=( const SState& src );     /**< not implemented */
    };
    typedef CORE::CTSharedPtr< SState, MT::CMutex > TStatePtr;

    explicit CMathAccelerator( CMathBridgeGlobal& owner );

    virtual ~CMathAccelerator() GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  Makes a state for the best device that is not the CPU (devices of the settings order first) and subscribes to
     *  its loss; false when there is none. The state is not in use until InstallState(): creating a device touches the
     *  pulse generator of its events, which is why no lock of the bridge may be held for this call
     */
    bool CreateState( TStatePtr& state );

    /**
     *  Puts a state (null for none) in use. The state it replaces is handed to the caller in 'displaced', to be released
     *  when no lock is held and no event is being delivered: a device that goes unregisters from its pulse generator.
     *  A device that is lost stays in use (its providers decline calls and are removed) until the next state replaces it
     */
    void InstallState( const TStatePtr& state, TStatePtr& displaced );

    /** Drops the state and the subscriptions; the providers that still hold the state finish their call with it */
    void ReleaseState( void );

    /** The state of the device in use; false when there is none or the device is lost */
    bool GetState( TStatePtr& state ) const;

    /** Starts and stops listening to the availability of backends */
    void SubscribeToBackends( void );
    void UnsubscribeFromBackends( void );

    private:

    typedef CORE::CTEventHandlerFunctor< CMathAccelerator > TEventCallback;

    void OnDeviceLost( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnBackendAvailabilityChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );

    CMathAccelerator( const CMathAccelerator& src );                /**< not implemented */
    CMathAccelerator& operator=( const CMathAccelerator& src );     /**< not implemented */

    private:

    CMathBridgeGlobal& m_owner;
    TStatePtr m_state;                      /**< guarded by m_lock */
    mutable MT::CMutex m_lock;
    bool m_subscribedToBackends;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace COMPUTEBRIDGEMATH */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMPUTEBRIDGEMATH_CMATHACCELERATOR_H ? */
