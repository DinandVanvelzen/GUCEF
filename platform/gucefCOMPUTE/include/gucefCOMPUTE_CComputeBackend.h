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

#ifndef GUCEF_COMPUTE_CCOMPUTEBACKEND_H
#define GUCEF_COMPUTE_CCOMPUTEBACKEND_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CTATOMIC_H
#include "gucefMT_CTAtomic.h"
#define GUCEF_MT_CTATOMIC_H
#endif /* GUCEF_MT_CTATOMIC_H ? */

#ifndef GUCEF_COMPUTE_CCOMPUTEDEVICEINFO_H
#include "gucefCOMPUTE_CComputeDeviceInfo.h"
#define GUCEF_COMPUTE_CCOMPUTEDEVICEINFO_H
#endif /* GUCEF_COMPUTE_CCOMPUTEDEVICEINFO_H ? */

#ifndef GUCEF_COMPUTE_CCOMPUTEDEVICEPARAMS_H
#include "gucefCOMPUTE_CComputeDeviceParams.h"
#define GUCEF_COMPUTE_CCOMPUTEDEVICEPARAMS_H
#endif /* GUCEF_COMPUTE_CCOMPUTEDEVICEPARAMS_H ? */

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

/**
 *  A way to compute devices: the built-in CPU backend ("cpu") or a backend plugin ("vulkan", "cuda", "webgpu",
 *  "metal", "opencl"). Backends are C++ link-back plugins, like audio drivers: a subclass registered in
 *  CComputeBackendRegistry from GUCEFPlugin_Load with RegisterBackend() and removed with TryUnregister() in
 *  GUCEFPlugin_Unload, after the plugin released every device it created. Statically linked builds register the same
 *  class directly.
 *
 *  Initialize() binds the runtime of the backend (driver libraries are loaded by name at run time). A backend whose
 *  runtime is absent returns false: it is skipped with one line in the system log, which is never an error.
 */
class GUCEF_COMPUTE_PUBLIC_CPP CComputeBackend
{
    public:

    CComputeBackend( void );

    virtual ~CComputeBackend();

    /** The short name that selects the backend, lower case ("cpu", "vulkan", ...) */
    virtual CString GetName( void ) const = 0;

    virtual CString GetDescription( void ) const;

    /** Breaks ties between devices of equal type; the CPU backend is far below everything */
    virtual Int32 GetPriority( void ) const;

    /** Binds the runtime. False when it is absent or unusable */
    virtual bool Initialize( void ) = 0;

    /** Releases the runtime; every device of the backend has been released before */
    virtual void Shutdown( void ) = 0;

    /** Lists the devices the backend offers right now */
    virtual bool EnumerateDevices( TComputeDeviceInfoVector& devices ) = 0;

    /** Creates the device that the parameters select */
    virtual TComputeResult CreateDevice( const CComputeDeviceParams& params, TComputeDevicePtr& device ) = 0;

    /** Processes pending callbacks where the platform needs the application to drive them (CComputeGlobal::Update()) */
    virtual void Update( void );

    /** True after a successful Initialize() through the registry */
    bool IsAvailable( void ) const;

    private:

    friend class CComputeBackendRegistry;

    void SetAvailable( bool available );

    CComputeBackend( const CComputeBackend& src );            /**< not implemented */
    CComputeBackend& operator=( const CComputeBackend& src ); /**< not implemented */

    private:

    MT::CTAtomic< UInt32 > m_available;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace COMPUTE */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMPUTE_CCOMPUTEBACKEND_H ? */
