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

#ifndef GUCEF_COMPUTE_CCOMPUTEGLOBAL_H
#define GUCEF_COMPUTE_CCOMPUTEGLOBAL_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_COMPUTE_CCPUPARALLELFOR_H
#include "gucefCOMPUTE_CCpuParallelFor.h"
#define GUCEF_COMPUTE_CCPUPARALLELFOR_H
#endif /* GUCEF_COMPUTE_CCPUPARALLELFOR_H ? */

#ifndef GUCEF_COMPUTE_MACROS_H
#include "gucefCOMPUTE_macros.h"
#define GUCEF_COMPUTE_MACROS_H
#endif /* GUCEF_COMPUTE_MACROS_H ? */

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

class CModule;
class CComputeBackendRegistry;
class CComputeKernelRegistry;
class CComputeSettings;
class CComputeDevice;

/**
 *  Singular singleton providing access to all global compute systems, created on first use (after the globals of
 *  gucefCORE and gucefVFS) and destroyed when the module unloads. It owns, created in this order: the settings, the
 *  kernel registry, the CPU job system (CCpuParallelFor, one worker set per process) and the backend registry with the
 *  built-in CPU backend. The module is listed for plugin link-back, so backend and bridge plugins find it.
 *
 *  It knows every device that exists. When the module unloads (CModule::Unload()) devices the application still holds
 *  are marked lost first, so their pending work fails and nothing runs module code later on behalf of the global;
 *  the CPU workers stop. After that Instance() gives null: the global is not created again until the module is loaded
 *  again (CModule::Load()).
 */
class GUCEF_COMPUTE_PUBLIC_CPP CComputeGlobal
{
    public:

    /** The global, created on first use; null after the module unloaded (see the class description) */
    static CComputeGlobal* Instance( void );

    /** The global when it exists, without creating it */
    static CComputeGlobal* InstanceIfCreated( void );

    /** All backends by name; the CPU backend is always there */
    CComputeBackendRegistry& GetBackendRegistry( void );

    /** All kernels by id, with their artefacts per backend */
    CComputeKernelRegistry& GetKernelRegistry( void );
    const CComputeKernelRegistry& GetKernelRegistry( void ) const;

    /** The CPU job system, also for multi-core providers of bridge plugins */
    CCpuParallelFor& GetCpuParallelFor( void );

    /** The same, shared: holders keep it past an unload of the global */
    TCpuParallelForPtr GetCpuParallelForPtr( void );

    /** The devices that exist right now */
    UInt32 GetDeviceCount( void ) const;

    /** A copy of the settings in effect */
    CComputeSettings GetSettings( void ) const;

    /**
     *  Puts settings into effect: backend selection of the backend registry, the number of CPU workers (running
     *  workers restart), the kernel override root of the kernel registry, validation and f64 on GPUs for devices
     *  created afterwards
     */
    void ApplySettings( const CComputeSettings& settings );

    /**
     *  Processes pending completions where the application drives them: the pending work of every cooperative queue
     *  (CComputeDeviceParams::SetCooperativeCompletion(), platforms without threads, a browser main thread) without
     *  waiting, then Update() of every backend. Call it from the thread that owns the application loop; queues with a
     *  completion task do not need it. Returns the number of submissions it finished.
     */
    UInt32 Update( void );

    private:
    friend class CModule;
    friend class CComputeDevice;
    friend class CComputeBackendRegistry;

    static void Deinstance( void );

    /** Allows the global to be created again, after an unload; CModule::Load() calls it */
    static void AllowCreation( void );

    void RegisterDevice( CComputeDevice* device );
    void UnregisterDevice( CComputeDevice* device );

    /**
     *  Marks the devices of the backend (all devices for an empty name) lost. With 'release' the global also lets go of
     *  them and waits until no device is registered any more. Returns the number of devices marked
     */
    UInt32 MarkDevicesLost( const CString& backendName, const CString& reason, bool release );

    private:

    CComputeGlobal( void );

    ~CComputeGlobal();

    /** Creates the owned global systems in their dependency order, after the instance pointer is set */
    void Initialize( void );

    CComputeGlobal( const CComputeGlobal& src );            /**< not implemented */
    CComputeGlobal& operator=( const CComputeGlobal& src ); /**< not implemented */

    private:

    CComputeSettings* m_settings;
    MT::CMutex m_settingsLock;
    CComputeKernelRegistry* m_kernelRegistry;
    TCpuParallelForPtr m_cpuParallelFor;
    MT::CMutex m_deviceLock;
    GUCEF::set< CComputeDevice* > m_devices;    /**< guarded by m_deviceLock */
    CComputeBackendRegistry* m_backendRegistry;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace COMPUTE */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMPUTE_CCOMPUTEGLOBAL_H ? */
