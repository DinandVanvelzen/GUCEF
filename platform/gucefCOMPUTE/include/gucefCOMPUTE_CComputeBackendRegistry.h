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

#ifndef GUCEF_COMPUTE_CCOMPUTEBACKENDREGISTRY_H
#define GUCEF_COMPUTE_CCOMPUTEBACKENDREGISTRY_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTGLOBALONREGISTRY_H
#include "gucefCORE_CTGlobalONRegistry.h"
#define GUCEF_CORE_CTGLOBALONREGISTRY_H
#endif /* GUCEF_CORE_CTGLOBALONREGISTRY_H ? */

#ifndef GUCEF_COMPUTE_CCOMPUTEBACKEND_H
#include "gucefCOMPUTE_CComputeBackend.h"
#define GUCEF_COMPUTE_CCOMPUTEBACKEND_H
#endif /* GUCEF_COMPUTE_CCOMPUTEBACKEND_H ? */

#ifndef GUCEF_COMPUTE_CCOMPUTEDEVICEREQUIREMENTS_H
#include "gucefCOMPUTE_CComputeDeviceRequirements.h"
#define GUCEF_COMPUTE_CCOMPUTEDEVICEREQUIREMENTS_H
#endif /* GUCEF_COMPUTE_CCOMPUTEDEVICEREQUIREMENTS_H ? */

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

class CComputeGlobal;
class CComputeSettings;

/**
 *  All compute backends by name (lower case). The CPU backend is always registered, so CreateBestDevice() with
 *  default requirements always succeeds. Reach it through CComputeGlobal::GetBackendRegistry(). Thread-safe.
 *
 *  Best device: the requirements are met, then the order of the settings (the preferred device first, then the
 *  backend order), then the device type (discrete, integrated, accelerator, virtual, unknown, CPU), then the backend
 *  priority, then the order of enumeration. Backends that the settings disable, or that failed to initialise, are
 *  skipped.
 *
 *  BackendAvailabilityChangedEvent (event data: a CORE::TCloneableString with the backend name) is sent on the thread
 *  that registers, unregisters or re-initialises a backend, whenever the availability of a backend changes.
 */
class GUCEF_COMPUTE_PUBLIC_CPP CComputeBackendRegistry : public CORE::CTGlobalONRegistry< CComputeBackend, MT::CMutex >
{
    public:

    static const CORE::CEvent BackendAvailabilityChangedEvent;

    static void RegisterEvents( void );

    typedef GUCEF::vector< TComputeBackendPtr > TComputeBackendPtrVector;

    /**
     *  Registers a backend under its lower-case name and initialises it. True when it was registered, available or not
     *  (IsBackendAvailable() tells); false when the name is taken or the backend is null.
     */
    /** @param builtIn whether the entry is functionality of the module itself instead of coming from a plugin: built-in entries survive the unregistration of all plugins */
    bool RegisterBackend( const TComputeBackendPtr& backend, bool builtIn = false );

    /**
     *  Unregisters and shuts down the backend with the name (without regard for case). False when there is none.
     *  Devices of the backend that still exist are marked lost first ("the backend was unregistered", with a warning):
     *  their pending work fails and they refuse new work. A plugin releases its devices before it unloads, their code
     *  is in the plugin
     */
    virtual bool TryUnregister( const CString& backendName ) GUCEF_VIRTUAL_OVERRIDE;

    /** Unregisters and shuts down every backend */
    virtual void UnregisterAll( void ) GUCEF_VIRTUAL_OVERRIDE;

    /** Finds a backend by name without regard for case, available or not */
    bool TryFindBackend( const CString& backendName, TComputeBackendPtr& backend ) const;

    /** The usable backends (available and enabled by the settings) in the order of the settings, then by priority */
    void GetBackendsByPriority( TComputeBackendPtrVector& backends ) const;

    /** True for a registered backend that initialised and is enabled by the settings */
    bool IsBackendAvailable( const CString& backendName ) const;

    /** The registered backends that are not usable: failed to initialise or disabled by the settings */
    void GetUnavailableBackendNames( CString::StringVector& backendNames ) const;

    /** The devices of every usable backend, best first */
    bool EnumerateAllDevices( TComputeDeviceInfoVector& devices ) const;

    /** Creates the best device that meets the requirements (see the class description) */
    bool CreateBestDevice( const CComputeDeviceRequirements& requirements, TComputeDevicePtr& device, TComputeResult* result = GUCEF_NULL ) const;

    /** Creates a device of the named backend with the parameters */
    bool CreateDevice( const CString& backendName, const CComputeDeviceParams& params, TComputeDevicePtr& device, TComputeResult* result = GUCEF_NULL ) const;

    /** Shuts the backend down and initialises it again, for example after a runtime was installed. True when it is available after */
    bool ReinitializeBackend( const CString& backendName );

    /** Test hook: makes the initialisation of the named backend fail without calling it, as when its runtime is absent */
    void SetForcedInitializeFailure( const CString& backendName, bool forceFailure );

    /** Takes the backend selection of the settings: enabled, disabled, backend order and preferred device */
    void ApplySettings( const CComputeSettings& settings );

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    friend class CComputeGlobal;

    explicit CComputeBackendRegistry( CComputeGlobal* global );

    virtual ~CComputeBackendRegistry() GUCEF_VIRTUAL_OVERRIDE;

    bool InitializeBackend( CComputeBackend& backend );
    bool IsEnabledBySettings( const CString& lowerCaseName ) const;
    Int32 GetSettingsOrder( const CString& lowerCaseName ) const;
    void NotifyAvailabilityChanged( const CString& backendName );

    CComputeBackendRegistry( const CComputeBackendRegistry& src );            /**< not implemented */
    CComputeBackendRegistry& operator=( const CComputeBackendRegistry& src ); /**< not implemented */

    private:

    CComputeGlobal* m_global;
    MT::CMutex m_selectionLock;
    CString::StringSet m_forcedFailures;
    CString::StringSet m_enabledBackends;
    CString::StringSet m_disabledBackends;
    CString::StringVector m_backendOrder;
    CString m_preferredDevice;
    bool m_validation;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace COMPUTE */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMPUTE_CCOMPUTEBACKENDREGISTRY_H ? */
