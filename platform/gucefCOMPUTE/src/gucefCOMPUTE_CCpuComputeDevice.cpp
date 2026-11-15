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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_DVOSWRAP_H
#include "DVOSWRAP.h"
#define GUCEF_CORE_DVOSWRAP_H
#endif /* GUCEF_CORE_DVOSWRAP_H ? */

#ifndef GUCEF_COMPUTE_CCOMPUTEGLOBAL_H
#include "gucefCOMPUTE_CComputeGlobal.h"
#define GUCEF_COMPUTE_CCOMPUTEGLOBAL_H
#endif /* GUCEF_COMPUTE_CCOMPUTEGLOBAL_H ? */

#ifndef GUCEF_COMPUTE_CCPUCOMPUTEBUFFER_H
#include "gucefCOMPUTE_CCpuComputeBuffer.h"
#define GUCEF_COMPUTE_CCPUCOMPUTEBUFFER_H
#endif /* GUCEF_COMPUTE_CCPUCOMPUTEBUFFER_H ? */

#ifndef GUCEF_COMPUTE_CCPUCOMPUTEKERNEL_H
#include "gucefCOMPUTE_CCpuComputeKernel.h"
#define GUCEF_COMPUTE_CCPUCOMPUTEKERNEL_H
#endif /* GUCEF_COMPUTE_CCPUCOMPUTEKERNEL_H ? */

#ifndef GUCEF_COMPUTE_CCPUCOMPUTEQUEUE_H
#include "gucefCOMPUTE_CCpuComputeQueue.h"
#define GUCEF_COMPUTE_CCPUCOMPUTEQUEUE_H
#endif /* GUCEF_COMPUTE_CCPUCOMPUTEQUEUE_H ? */

#include "gucefCOMPUTE_CCpuComputeDevice.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COMPUTE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CCpuComputeDevice::CCpuComputeDevice( const CComputeDeviceInfo& info, const CComputeDeviceParams& params )
    : CComputeDevice( info, params )
{GUCEF_TRACE;

    const UInt32 queueCount = params.GetQueueCount() < info.GetCapabilities().GetQueueCount() ? params.GetQueueCount() : info.GetCapabilities().GetQueueCount();
    CComputeGlobal* global = CComputeGlobal::InstanceIfCreated();
    const TCpuParallelForPtr parallelFor = GUCEF_NULL != global ? global->GetCpuParallelForPtr() : TCpuParallelForPtr();
    for ( UInt32 i=0; i<queueCount; ++i )
    {
        CCpuComputeQueue* queue = GUCEF_NEW CCpuComputeQueue( this, i, params.GetExecutionPolicy(), params.GetCooperativeCompletion(), parallelFor );
        AddQueue( queue->CreateSharedPtr() );
    }
}

/*-------------------------------------------------------------------------*/

CCpuComputeDevice::~CCpuComputeDevice()
{GUCEF_TRACE;

    ShutdownDevice();
}

/*-------------------------------------------------------------------------*/

TComputeDevicePtr
CCpuComputeDevice::Create( const CComputeDeviceInfo& info, const CComputeDeviceParams& params )
{GUCEF_TRACE;

    CCpuComputeDevice* device = GUCEF_NEW CCpuComputeDevice( info, params );
    return device->CreateSharedPtr();
}

/*-------------------------------------------------------------------------*/

CComputeDeviceInfo
CCpuComputeDevice::GetHostDeviceInfo( bool reportNativeLimits )
{GUCEF_TRACE;

    const UInt32 logicalCpus = CORE::GetLogicalCPUCount();

    CComputeDeviceInfo info;
    info.SetBackendName( "cpu" );
    info.SetDeviceName( "CPU (" + CORE::ToString( logicalCpus ) + " logical processors)" );
    info.SetDeviceType( DEVICETYPE_CPU );
    info.SetDriverVersion( "built-in" );

    CComputeDeviceCapabilities& caps = info.GetCapabilities();
    caps.SetFeatures( COMPUTEFEATURE_FLOAT16 | COMPUTEFEATURE_FLOAT64 | COMPUTEFEATURE_INT64 | COMPUTEFEATURE_INT64_ATOMICS |
                      COMPUTEFEATURE_FLOAT_ATOMICS | COMPUTEFEATURE_UNIFIED_MEMORY | COMPUTEFEATURE_MAPPABLE_DEVICE_LOCAL_MEMORY |
                      COMPUTEFEATURE_FAST_FLOAT64 | COMPUTEFEATURE_STRICT_FLOAT );
    caps.SetQueueCount( 8 );
    if ( reportNativeLimits )
    {
        caps.SetMaxInvocationsPerGroup( 1024 );
        caps.SetMaxGroupSize( 1024, 1024, 1024 );
        caps.SetMaxWorkgroupMemoryInBytes( 1024 * 1024 );
        caps.SetMaxGroupsPerDispatch( 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu );
        caps.SetMaxBufferSizeInBytes( sizeof( void* ) >= 8 ? ( static_cast< UInt64 >( 1 ) << 40 ) : 0x7FFFFFFFu );
        caps.SetMaxBindingSizeInBytes( sizeof( void* ) >= 8 ? ( static_cast< UInt64 >( 1 ) << 40 ) : 0x7FFFFFFFu );
    }
    return info;
}

/*-------------------------------------------------------------------------*/

TComputeResult
CCpuComputeDevice::OnCreateBuffer( const CComputeBufferDesc& desc, TComputeBufferPtr& buffer )
{GUCEF_TRACE;

    CCpuComputeBuffer* created = CCpuComputeBuffer::Create( this, desc );
    if ( GUCEF_NULL == created )
        return COMPUTERESULT_OUT_OF_HOST_MEMORY;
    buffer = created->CreateSharedPtr();
    return COMPUTERESULT_SUCCESS;
}

/*-------------------------------------------------------------------------*/

TComputeResult
CCpuComputeDevice::OnCreateKernel( const TComputeKernelDeclPtr& decl, const TComputeKernelArtefactPtr& artefact, TComputeKernelPtr& kernel )
{GUCEF_TRACE;

    if ( ARTEFACTFORMAT_CPU != artefact->GetFormat() || GUCEF_NULL == artefact->GetCpuFactory() )
        return COMPUTERESULT_KERNEL_NOT_FOUND;

    TCpuComputeKernelFactory::TProductPtr product = artefact->GetCpuFactory()->Create();
    if ( product.IsNULL() )
        return COMPUTERESULT_COMPILE_FAILED;
    kernel = TComputeKernelPtr( CORE::CTSharedPtr< CCpuComputeKernel, MT::CMutex >( product ) );
    return COMPUTERESULT_SUCCESS;
}

/*-------------------------------------------------------------------------*/

CString
CCpuComputeDevice::GetArtefactBackendName( void ) const
{GUCEF_TRACE;

    return "cpu";
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace COMPUTE */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
