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

#ifndef GUCEF_COMPUTE_CCPUCOMPUTEDEVICE_H
#define GUCEF_COMPUTE_CCPUCOMPUTEDEVICE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_COMPUTE_CCOMPUTEDEVICE_H
#include "gucefCOMPUTE_CComputeDevice.h"
#define GUCEF_COMPUTE_CCOMPUTEDEVICE_H
#endif /* GUCEF_COMPUTE_CCOMPUTEDEVICE_H ? */

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
 *  The device of the CPU backend (DEVICETYPE_CPU): every numeric feature, unified memory, bit-exact integer and strict
 *  float. It reports the portable limits of every backend unless the backend option "reportNativeLimits" is true.
 *  Its kernels are the CPU artefacts (ARTEFACTFORMAT_CPU) of the kernel registry, whatever backend name the device
 *  info carries, so tests can present it as another kind of device.
 */
class GUCEF_COMPUTE_PUBLIC_CPP CCpuComputeDevice : public CComputeDevice
{
    public:

    /** Creates the device with its queues */
    static TComputeDevicePtr Create( const CComputeDeviceInfo& info, const CComputeDeviceParams& params );

    /** The device info of the CPU device of this machine */
    static CComputeDeviceInfo GetHostDeviceInfo( bool reportNativeLimits );

    virtual ~CCpuComputeDevice() GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual TComputeResult OnCreateBuffer( const CComputeBufferDesc& desc, TComputeBufferPtr& buffer ) GUCEF_VIRTUAL_OVERRIDE;

    virtual TComputeResult OnCreateKernel( const TComputeKernelDeclPtr& decl, const TComputeKernelArtefactPtr& artefact, TComputeKernelPtr& kernel ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CString GetArtefactBackendName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    CCpuComputeDevice( const CComputeDeviceInfo& info, const CComputeDeviceParams& params );

    CCpuComputeDevice( const CCpuComputeDevice& src );            /**< not implemented */
    CCpuComputeDevice& operator=( const CCpuComputeDevice& src ); /**< not implemented */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace COMPUTE */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMPUTE_CCPUCOMPUTEDEVICE_H ? */
