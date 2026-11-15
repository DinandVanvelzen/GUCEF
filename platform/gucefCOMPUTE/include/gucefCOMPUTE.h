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

/*
 *      This is the main include header for the gucefCOMPUTE library.
 *      It is part of a framework called GUCEF.
 */

#ifndef GUCEF_COMPUTE_H
#define GUCEF_COMPUTE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *      Library build configuration
 */
#ifndef GUCEF_COMPUTE_CONFIG_H
#include "gucefCOMPUTE_config.h"
#define GUCEF_COMPUTE_CONFIG_H
#endif /* GUCEF_COMPUTE_CONFIG_H ? */

/*
 *      Library specific macros and generic macros
 */
#ifndef GUCEF_COMPUTE_MACROS_H
#include "gucefCOMPUTE_macros.h"
#define GUCEF_COMPUTE_MACROS_H
#endif /* GUCEF_COMPUTE_MACROS_H ? */

/*
 *      Simple types
 */
#ifndef GUCEF_COMPUTE_ETYPES_H
#include "gucefCOMPUTE_ETypes.h"
#define GUCEF_COMPUTE_ETYPES_H
#endif /* GUCEF_COMPUTE_ETYPES_H ? */

#ifdef __cplusplus

#ifndef GUCEF_COMPUTE_ALIGNEDMEMORY_H
#include "gucefCOMPUTE_AlignedMemory.h"
#define GUCEF_COMPUTE_ALIGNEDMEMORY_H
#endif /* GUCEF_COMPUTE_ALIGNEDMEMORY_H ? */

#ifndef GUCEF_COMPUTE_CCOMPUTEARGUMENTS_H
#include "gucefCOMPUTE_CComputeArguments.h"
#define GUCEF_COMPUTE_CCOMPUTEARGUMENTS_H
#endif /* GUCEF_COMPUTE_CCOMPUTEARGUMENTS_H ? */

#ifndef GUCEF_COMPUTE_CCOMPUTEBACKEND_H
#include "gucefCOMPUTE_CComputeBackend.h"
#define GUCEF_COMPUTE_CCOMPUTEBACKEND_H
#endif /* GUCEF_COMPUTE_CCOMPUTEBACKEND_H ? */

#ifndef GUCEF_COMPUTE_CCOMPUTEBACKENDREGISTRY_H
#include "gucefCOMPUTE_CComputeBackendRegistry.h"
#define GUCEF_COMPUTE_CCOMPUTEBACKENDREGISTRY_H
#endif /* GUCEF_COMPUTE_CCOMPUTEBACKENDREGISTRY_H ? */

#ifndef GUCEF_COMPUTE_CCOMPUTEBUFFER_H
#include "gucefCOMPUTE_CComputeBuffer.h"
#define GUCEF_COMPUTE_CCOMPUTEBUFFER_H
#endif /* GUCEF_COMPUTE_CCOMPUTEBUFFER_H ? */

#ifndef GUCEF_COMPUTE_CCOMPUTEBUFFERDESC_H
#include "gucefCOMPUTE_CComputeBufferDesc.h"
#define GUCEF_COMPUTE_CCOMPUTEBUFFERDESC_H
#endif /* GUCEF_COMPUTE_CCOMPUTEBUFFERDESC_H ? */

#ifndef GUCEF_COMPUTE_CCOMPUTEBUFFERMAPSCOPE_H
#include "gucefCOMPUTE_CComputeBufferMapScope.h"
#define GUCEF_COMPUTE_CCOMPUTEBUFFERMAPSCOPE_H
#endif /* GUCEF_COMPUTE_CCOMPUTEBUFFERMAPSCOPE_H ? */

#ifndef GUCEF_COMPUTE_CCOMPUTECOMMANDLIST_H
#include "gucefCOMPUTE_CComputeCommandList.h"
#define GUCEF_COMPUTE_CCOMPUTECOMMANDLIST_H
#endif /* GUCEF_COMPUTE_CCOMPUTECOMMANDLIST_H ? */

#ifndef GUCEF_COMPUTE_CCOMPUTECOMPLETIONTASKCONSUMER_H
#include "gucefCOMPUTE_CComputeCompletionTaskConsumer.h"
#define GUCEF_COMPUTE_CCOMPUTECOMPLETIONTASKCONSUMER_H
#endif /* GUCEF_COMPUTE_CCOMPUTECOMPLETIONTASKCONSUMER_H ? */

#ifndef GUCEF_COMPUTE_CCOMPUTEDEVICE_H
#include "gucefCOMPUTE_CComputeDevice.h"
#define GUCEF_COMPUTE_CCOMPUTEDEVICE_H
#endif /* GUCEF_COMPUTE_CCOMPUTEDEVICE_H ? */

#ifndef GUCEF_COMPUTE_CCOMPUTEDEVICECAPABILITIES_H
#include "gucefCOMPUTE_CComputeDeviceCapabilities.h"
#define GUCEF_COMPUTE_CCOMPUTEDEVICECAPABILITIES_H
#endif /* GUCEF_COMPUTE_CCOMPUTEDEVICECAPABILITIES_H ? */

#ifndef GUCEF_COMPUTE_CCOMPUTEDEVICEINFO_H
#include "gucefCOMPUTE_CComputeDeviceInfo.h"
#define GUCEF_COMPUTE_CCOMPUTEDEVICEINFO_H
#endif /* GUCEF_COMPUTE_CCOMPUTEDEVICEINFO_H ? */

#ifndef GUCEF_COMPUTE_CCOMPUTEDEVICEPARAMS_H
#include "gucefCOMPUTE_CComputeDeviceParams.h"
#define GUCEF_COMPUTE_CCOMPUTEDEVICEPARAMS_H
#endif /* GUCEF_COMPUTE_CCOMPUTEDEVICEPARAMS_H ? */

#ifndef GUCEF_COMPUTE_CCOMPUTEDEVICEREQUIREMENTS_H
#include "gucefCOMPUTE_CComputeDeviceRequirements.h"
#define GUCEF_COMPUTE_CCOMPUTEDEVICEREQUIREMENTS_H
#endif /* GUCEF_COMPUTE_CCOMPUTEDEVICEREQUIREMENTS_H ? */

#ifndef GUCEF_COMPUTE_CCOMPUTEDISPATCHSIZE_H
#include "gucefCOMPUTE_CComputeDispatchSize.h"
#define GUCEF_COMPUTE_CCOMPUTEDISPATCHSIZE_H
#endif /* GUCEF_COMPUTE_CCOMPUTEDISPATCHSIZE_H ? */

#ifndef GUCEF_COMPUTE_CCOMPUTEFENCE_H
#include "gucefCOMPUTE_CComputeFence.h"
#define GUCEF_COMPUTE_CCOMPUTEFENCE_H
#endif /* GUCEF_COMPUTE_CCOMPUTEFENCE_H ? */

#ifndef GUCEF_COMPUTE_CCOMPUTEGLOBAL_H
#include "gucefCOMPUTE_CComputeGlobal.h"
#define GUCEF_COMPUTE_CCOMPUTEGLOBAL_H
#endif /* GUCEF_COMPUTE_CCOMPUTEGLOBAL_H ? */

#ifndef GUCEF_COMPUTE_CCOMPUTEHOSTBATCH_H
#include "gucefCOMPUTE_CComputeHostBatch.h"
#define GUCEF_COMPUTE_CCOMPUTEHOSTBATCH_H
#endif /* GUCEF_COMPUTE_CCOMPUTEHOSTBATCH_H ? */

#ifndef GUCEF_COMPUTE_CCOMPUTEKERNEL_H
#include "gucefCOMPUTE_CComputeKernel.h"
#define GUCEF_COMPUTE_CCOMPUTEKERNEL_H
#endif /* GUCEF_COMPUTE_CCOMPUTEKERNEL_H ? */

#ifndef GUCEF_COMPUTE_CCOMPUTEKERNELARTEFACT_H
#include "gucefCOMPUTE_CComputeKernelArtefact.h"
#define GUCEF_COMPUTE_CCOMPUTEKERNELARTEFACT_H
#endif /* GUCEF_COMPUTE_CCOMPUTEKERNELARTEFACT_H ? */

#ifndef GUCEF_COMPUTE_CCOMPUTEKERNELDECL_H
#include "gucefCOMPUTE_CComputeKernelDecl.h"
#define GUCEF_COMPUTE_CCOMPUTEKERNELDECL_H
#endif /* GUCEF_COMPUTE_CCOMPUTEKERNELDECL_H ? */

#ifndef GUCEF_COMPUTE_CCOMPUTEKERNELREGISTRY_H
#include "gucefCOMPUTE_CComputeKernelRegistry.h"
#define GUCEF_COMPUTE_CCOMPUTEKERNELREGISTRY_H
#endif /* GUCEF_COMPUTE_CCOMPUTEKERNELREGISTRY_H ? */

#ifndef GUCEF_COMPUTE_CCOMPUTEKERNELSIGNATURE_H
#include "gucefCOMPUTE_CComputeKernelSignature.h"
#define GUCEF_COMPUTE_CCOMPUTEKERNELSIGNATURE_H
#endif /* GUCEF_COMPUTE_CCOMPUTEKERNELSIGNATURE_H ? */

#ifndef GUCEF_COMPUTE_CCOMPUTEPENDINGWORK_H
#include "gucefCOMPUTE_CComputePendingWork.h"
#define GUCEF_COMPUTE_CCOMPUTEPENDINGWORK_H
#endif /* GUCEF_COMPUTE_CCOMPUTEPENDINGWORK_H ? */

#ifndef GUCEF_COMPUTE_CCOMPUTEQUEUE_H
#include "gucefCOMPUTE_CComputeQueue.h"
#define GUCEF_COMPUTE_CCOMPUTEQUEUE_H
#endif /* GUCEF_COMPUTE_CCOMPUTEQUEUE_H ? */

#ifndef GUCEF_COMPUTE_CCOMPUTESETTINGS_H
#include "gucefCOMPUTE_CComputeSettings.h"
#define GUCEF_COMPUTE_CCOMPUTESETTINGS_H
#endif /* GUCEF_COMPUTE_CCOMPUTESETTINGS_H ? */

#ifndef GUCEF_COMPUTE_CCOMPUTETOLERANCE_H
#include "gucefCOMPUTE_CComputeTolerance.h"
#define GUCEF_COMPUTE_CCOMPUTETOLERANCE_H
#endif /* GUCEF_COMPUTE_CCOMPUTETOLERANCE_H ? */

#ifndef GUCEF_COMPUTE_CCPUCOMPUTEBACKEND_H
#include "gucefCOMPUTE_CCpuComputeBackend.h"
#define GUCEF_COMPUTE_CCPUCOMPUTEBACKEND_H
#endif /* GUCEF_COMPUTE_CCPUCOMPUTEBACKEND_H ? */

#ifndef GUCEF_COMPUTE_CCPUCOMPUTEBUFFER_H
#include "gucefCOMPUTE_CCpuComputeBuffer.h"
#define GUCEF_COMPUTE_CCPUCOMPUTEBUFFER_H
#endif /* GUCEF_COMPUTE_CCPUCOMPUTEBUFFER_H ? */

#ifndef GUCEF_COMPUTE_CCPUCOMPUTEDEVICE_H
#include "gucefCOMPUTE_CCpuComputeDevice.h"
#define GUCEF_COMPUTE_CCPUCOMPUTEDEVICE_H
#endif /* GUCEF_COMPUTE_CCPUCOMPUTEDEVICE_H ? */

#ifndef GUCEF_COMPUTE_CCPUCOMPUTEFENCE_H
#include "gucefCOMPUTE_CCpuComputeFence.h"
#define GUCEF_COMPUTE_CCPUCOMPUTEFENCE_H
#endif /* GUCEF_COMPUTE_CCPUCOMPUTEFENCE_H ? */

#ifndef GUCEF_COMPUTE_CCPUCOMPUTEKERNEL_H
#include "gucefCOMPUTE_CCpuComputeKernel.h"
#define GUCEF_COMPUTE_CCPUCOMPUTEKERNEL_H
#endif /* GUCEF_COMPUTE_CCPUCOMPUTEKERNEL_H ? */

#ifndef GUCEF_COMPUTE_CCPUCOMPUTEQUEUE_H
#include "gucefCOMPUTE_CCpuComputeQueue.h"
#define GUCEF_COMPUTE_CCPUCOMPUTEQUEUE_H
#endif /* GUCEF_COMPUTE_CCPUCOMPUTEQUEUE_H ? */

#ifndef GUCEF_COMPUTE_CCPUDISPATCHCONTEXT_H
#include "gucefCOMPUTE_CCpuDispatchContext.h"
#define GUCEF_COMPUTE_CCPUDISPATCHCONTEXT_H
#endif /* GUCEF_COMPUTE_CCPUDISPATCHCONTEXT_H ? */

#ifndef GUCEF_COMPUTE_CCPUPARALLELFOR_H
#include "gucefCOMPUTE_CCpuParallelFor.h"
#define GUCEF_COMPUTE_CCPUPARALLELFOR_H
#endif /* GUCEF_COMPUTE_CCPUPARALLELFOR_H ? */

#ifndef GUCEF_COMPUTE_CCPUTESTKERNELS_H
#include "gucefCOMPUTE_CCpuTestKernels.h"
#define GUCEF_COMPUTE_CCPUTESTKERNELS_H
#endif /* GUCEF_COMPUTE_CCPUTESTKERNELS_H ? */

#ifndef GUCEF_COMPUTE_CCPUWORKERTASKCONSUMER_H
#include "gucefCOMPUTE_CCpuWorkerTaskConsumer.h"
#define GUCEF_COMPUTE_CCPUWORKERTASKCONSUMER_H
#endif /* GUCEF_COMPUTE_CCPUWORKERTASKCONSUMER_H ? */

#ifndef GUCEF_COMPUTE_CMODULE_H
#include "gucefCOMPUTE_CModule.h"
#define GUCEF_COMPUTE_CMODULE_H
#endif /* GUCEF_COMPUTE_CMODULE_H ? */

#ifndef GUCEF_COMPUTE_TYPEFUNCTIONS_H
#include "gucefCOMPUTE_TypeFunctions.h"
#define GUCEF_COMPUTE_TYPEFUNCTIONS_H
#endif /* GUCEF_COMPUTE_TYPEFUNCTIONS_H ? */

#endif /* __cplusplus ? */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMPUTE_H ? */
