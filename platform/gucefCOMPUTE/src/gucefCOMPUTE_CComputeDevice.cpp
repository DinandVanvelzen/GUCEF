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

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CLONEABLES_H
#include "cloneables.h"
#define GUCEF_CORE_CLONEABLES_H
#endif /* GUCEF_CORE_CLONEABLES_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_COMPUTE_CCOMPUTEGLOBAL_H
#include "gucefCOMPUTE_CComputeGlobal.h"
#define GUCEF_COMPUTE_CCOMPUTEGLOBAL_H
#endif /* GUCEF_COMPUTE_CCOMPUTEGLOBAL_H ? */

#ifndef GUCEF_COMPUTE_CCOMPUTEKERNELREGISTRY_H
#include "gucefCOMPUTE_CComputeKernelRegistry.h"
#define GUCEF_COMPUTE_CCOMPUTEKERNELREGISTRY_H
#endif /* GUCEF_COMPUTE_CCOMPUTEKERNELREGISTRY_H ? */

#ifndef GUCEF_COMPUTE_CCOMPUTESETTINGS_H
#include "gucefCOMPUTE_CComputeSettings.h"
#define GUCEF_COMPUTE_CCOMPUTESETTINGS_H
#endif /* GUCEF_COMPUTE_CCOMPUTESETTINGS_H ? */

#ifndef GUCEF_COMPUTE_TYPEFUNCTIONS_H
#include "gucefCOMPUTE_TypeFunctions.h"
#define GUCEF_COMPUTE_TYPEFUNCTIONS_H
#endif /* GUCEF_COMPUTE_TYPEFUNCTIONS_H ? */

#include "gucefCOMPUTE_CComputeDevice.h"

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

const CORE::CEvent CComputeDevice::DeviceLostEvent = "GUCEF::COMPUTE::CComputeDevice::DeviceLostEvent";

/*-------------------------------------------------------------------------*/

namespace {

CORE::PulseGeneratorPtr
EventPulseGenerator( const CComputeDeviceParams& params )
{
    if ( !params.GetEventPulseGenerator().IsNULL() )
        return params.GetEventPulseGenerator();
    return CORE::CCoreGlobal::Instance()->GetPulseGenerator();
}

}

/*-------------------------------------------------------------------------*/

void
CComputeDevice::RegisterEvents( void )
{GUCEF_TRACE;

    DeviceLostEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CComputeDevice::CComputeDevice( const CComputeDeviceInfo& info, const CComputeDeviceParams& params )
    : CORE::CTSGNotifier( EventPulseGenerator( params ), true, false )
    , CORE::CTSharedObjCreator< CComputeDevice, MT::CMutex >( this )
    , m_info( info )
    , m_params( params )
    , m_queues()
    , m_queueLock()
    , m_kernels()
    , m_kernelLock()
    , m_lost( 0 )
    , m_lostReason()
    , m_lostLock()
    , m_isShutdown( false )
    , m_global( GUCEF_NULL )
    , m_globalBusy( 0 )
{GUCEF_TRACE;

    RegisterEvents();
    CComputeGlobal* global = CComputeGlobal::InstanceIfCreated();
    if ( GUCEF_NULL != global )
        global->RegisterDevice( this );
}

/*-------------------------------------------------------------------------*/

CComputeDevice::~CComputeDevice()
{GUCEF_TRACE;

    ShutdownDevice();
}

/*-------------------------------------------------------------------------*/

void
CComputeDevice::ShutdownDevice( void )
{GUCEF_TRACE;

    if ( m_isShutdown )
        return;
    m_isShutdown = true;

    // Leave the global first. If it is busy with this device (an unload or a backend that goes) it finishes first
    CComputeGlobal* global = m_global.Exchange( GUCEF_NULL, MT::MEMORDER_ACQ_REL );
    if ( GUCEF_NULL != global )
    {
        global->UnregisterDevice( this );
    }
    else
    {
        while ( 0 != m_globalBusy.Load( MT::MEMORDER_ACQUIRE ) )
            MT::PrecisionDelay( 1 );
    }

    SignalUpcomingDestruction();
    GUCEF::vector< TComputeQueuePtr > queues;
    {
        MT::CScopeMutex lock( m_queueLock );
        queues.swap( m_queues );
    }
    for ( size_t i=0; i<queues.size(); ++i )
        queues[ i ]->Detach();
    ClearKernelCache();
}

/*-------------------------------------------------------------------------*/

const CString&
CComputeDevice::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString typeName = "GUCEF::COMPUTE::CComputeDevice";
    return typeName;
}

/*-------------------------------------------------------------------------*/

const CComputeDeviceInfo&
CComputeDevice::GetInfo( void ) const
{GUCEF_TRACE;

    return m_info;
}

/*-------------------------------------------------------------------------*/

const CComputeDeviceCapabilities&
CComputeDevice::GetCapabilities( void ) const
{GUCEF_TRACE;

    return m_info.GetCapabilities();
}

/*-------------------------------------------------------------------------*/

const CString&
CComputeDevice::GetBackendName( void ) const
{GUCEF_TRACE;

    return m_info.GetBackendName();
}

/*-------------------------------------------------------------------------*/

const CComputeDeviceParams&
CComputeDevice::GetParams( void ) const
{GUCEF_TRACE;

    return m_params;
}

/*-------------------------------------------------------------------------*/

CString
CComputeDevice::GetArtefactBackendName( void ) const
{GUCEF_TRACE;

    return m_info.GetBackendName();
}

/*-------------------------------------------------------------------------*/

void
CComputeDevice::AddQueue( const TComputeQueuePtr& queue )
{GUCEF_TRACE;

    if ( !queue.IsNULL() )
    {
        MT::CScopeMutex lock( m_queueLock );
        m_queues.push_back( queue );
    }
}

/*-------------------------------------------------------------------------*/

bool
CComputeDevice::CreateBuffer( const CComputeBufferDesc& desc, TComputeBufferPtr& buffer, TComputeResult* result )
{GUCEF_TRACE;

    buffer.Unlink();
    TComputeResult outcome = COMPUTERESULT_SUCCESS;
    const CComputeDeviceCapabilities& caps = m_info.GetCapabilities();
    if ( IsLost() )
        outcome = COMPUTERESULT_DEVICE_LOST;
    else
    if ( 0 == desc.GetSizeInBytes() || 0 == desc.GetUsage() || desc.GetSizeInBytes() > caps.GetMaxBufferSizeInBytes() )
        outcome = COMPUTERESULT_INVALID_ARGUMENT;
    else
    if ( MEMORYLOCATION_UNIFIED == desc.GetMemoryLocation() &&
         !caps.HasFeatures( COMPUTEFEATURE_UNIFIED_MEMORY ) && !caps.HasFeatures( COMPUTEFEATURE_MAPPABLE_DEVICE_LOCAL_MEMORY ) )
        outcome = COMPUTERESULT_INVALID_ARGUMENT;
    else
        outcome = OnCreateBuffer( desc, buffer );

    if ( COMPUTERESULT_SUCCESS != outcome )
        buffer.Unlink();
    if ( GUCEF_NULL != result )
        *result = outcome;
    return COMPUTERESULT_SUCCESS == outcome;
}

/*-------------------------------------------------------------------------*/

bool
CComputeDevice::ResolveKernel( const CString& kernelId, TComputeKernelDeclPtr& decl, TComputeKernelArtefactPtr& artefact, TComputeResult& result ) const
{GUCEF_TRACE;

    CComputeGlobal* global = CComputeGlobal::InstanceIfCreated();
    if ( GUCEF_NULL == global )
    {
        result = COMPUTERESULT_KERNEL_NOT_FOUND;
        return false;
    }
    const CComputeKernelRegistry& registry = global->GetKernelRegistry();
    if ( !registry.TryGetKernel( kernelId, decl ) || !registry.GetArtefact( kernelId, GetArtefactBackendName(), artefact ) )
    {
        result = COMPUTERESULT_KERNEL_NOT_FOUND;
        return false;
    }

    const CComputeDeviceCapabilities& caps = m_info.GetCapabilities();
    const TComputeFeatures required = decl->GetRequiredFeatures();
    if ( !caps.HasFeatures( required ) )
    {
        result = COMPUTERESULT_UNSUPPORTED;
        return false;
    }
    // f64 on a GPU only where the settings allow it or the device is fast at it
    if ( 0 != ( required & COMPUTEFEATURE_FLOAT64 ) && DEVICETYPE_CPU != m_info.GetDeviceType() &&
         !caps.HasFeatures( COMPUTEFEATURE_FAST_FLOAT64 ) && !global->GetSettings().GetAllowFloat64OnGpu() )
    {
        result = COMPUTERESULT_UNSUPPORTED;
        return false;
    }
    const CComputeKernelSignature& signature = decl->GetSignature();
    if ( signature.GetInvocationsPerGroup() > caps.GetMaxInvocationsPerGroup() ||
         signature.GetWorkgroupMemoryInBytes() > caps.GetMaxWorkgroupMemoryInBytes() ||
         signature.GetBindingCount() > caps.GetMaxStorageBuffersPerKernel() ||
         signature.GetConstantsSizeInBytes() > caps.GetMaxConstantsSizeInBytes() )
    {
        result = COMPUTERESULT_UNSUPPORTED;
        return false;
    }
    result = COMPUTERESULT_SUCCESS;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CComputeDevice::IsKernelAvailable( const CString& kernelId ) const
{GUCEF_TRACE;

    if ( IsLost() )
        return false;
    TComputeKernelDeclPtr decl;
    TComputeKernelArtefactPtr artefact;
    TComputeResult result = COMPUTERESULT_SUCCESS;
    return ResolveKernel( kernelId, decl, artefact, result );
}

/*-------------------------------------------------------------------------*/

bool
CComputeDevice::CreateKernel( const CString& kernelId, TComputeKernelPtr& kernel, TComputeResult* result )
{GUCEF_TRACE;

    kernel.Unlink();
    TComputeResult outcome = COMPUTERESULT_SUCCESS;
    if ( IsLost() )
    {
        if ( GUCEF_NULL != result )
            *result = COMPUTERESULT_DEVICE_LOST;
        return false;
    }

    TComputeKernelDeclPtr decl;
    TComputeKernelArtefactPtr artefact;
    if ( !ResolveKernel( kernelId, decl, artefact, outcome ) )
    {
        if ( GUCEF_NULL != result )
            *result = outcome;
        return false;
    }

    MT::CScopeMutex lock( m_kernelLock );

    // the cached kernel only while it exists and the registry still has the same declaration and artefact
    TKernelCache::iterator i = m_kernels.find( kernelId );
    if ( i != m_kernels.end() )
    {
        TComputeKernelPtr cached = (*i).second.GetSharedPtr();
        if ( !cached.IsNULL() && &cached->GetDecl() == decl.GetPointerAlways() && &cached->GetArtefact() == artefact.GetPointerAlways() )
        {
            kernel = cached;
            if ( GUCEF_NULL != result )
                *result = COMPUTERESULT_SUCCESS;
            return true;
        }
        m_kernels.erase( i );
    }

    TComputeKernelPtr created;
    outcome = OnCreateKernel( decl, artefact, created );
    if ( COMPUTERESULT_SUCCESS == outcome && created.IsNULL() )
        outcome = COMPUTERESULT_COMPILE_FAILED;
    if ( COMPUTERESULT_SUCCESS != outcome )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ComputeDevice: The kernel \"" << kernelId << "\" could not be created for device \"" <<
            m_info.GetDeviceName() << "\": " << ComputeResultToString( outcome ) );
        if ( GUCEF_NULL != result )
            *result = outcome;
        return false;
    }

    created->Bind( CreateSharedPtr(), decl, artefact );
    if ( !m_kernels[ kernelId ].InitializeUsingRelatedType( created ) )
        m_kernels.erase( kernelId );
    kernel = created;
    if ( GUCEF_NULL != result )
        *result = COMPUTERESULT_SUCCESS;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CComputeDevice::ClearKernelCache( void )
{GUCEF_TRACE;

    TKernelCache released;
    {
        MT::CScopeMutex lock( m_kernelLock );
        released.swap( m_kernels );
    }
}

/*-------------------------------------------------------------------------*/

UInt32
CComputeDevice::GetKernelCacheSize( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_kernelLock );
    UInt32 count = 0;
    for ( TKernelCache::const_iterator i = m_kernels.begin(); i != m_kernels.end(); ++i )
        if ( !(*i).second.IsExpired() )
            ++count;
    return count;
}

/*-------------------------------------------------------------------------*/

UInt32
CComputeDevice::GetQueueCount( void ) const
{GUCEF_TRACE;

    return static_cast< UInt32 >( m_queues.size() );
}

/*-------------------------------------------------------------------------*/

TComputeQueuePtr
CComputeDevice::GetQueue( UInt32 index ) const
{GUCEF_TRACE;

    if ( index < m_queues.size() )
        return m_queues[ index ];
    return TComputeQueuePtr();
}

/*-------------------------------------------------------------------------*/

TComputeQueuePtr
CComputeDevice::GetDefaultQueue( void ) const
{GUCEF_TRACE;

    return GetQueue( 0 );
}

/*-------------------------------------------------------------------------*/

bool
CComputeDevice::IsLost( void ) const
{GUCEF_TRACE;

    return 0 != m_lost.Load( MT::MEMORDER_ACQUIRE );
}

/*-------------------------------------------------------------------------*/

CString
CComputeDevice::GetLostReason( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lostLock );
    return m_lostReason;
}

/*-------------------------------------------------------------------------*/

void
CComputeDevice::ProcessCooperativeWork( TComputePendingWorkVector& finished )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_queueLock );
    for ( size_t i=0; i<m_queues.size(); ++i )
        if ( m_queues[ i ]->IsCooperative() )
            m_queues[ i ]->DrainCooperativeWork( ~static_cast< UInt64 >( 0 ), 0, finished );
}

/*-------------------------------------------------------------------------*/

bool
CComputeDevice::SimulateDeviceLoss( const CString& reason )
{GUCEF_TRACE;

    return MarkLost( reason );
}

/*-------------------------------------------------------------------------*/

bool
CComputeDevice::MarkLost( const CString& reason )
{GUCEF_TRACE;

    {
        MT::CScopeMutex lock( m_lostLock );
        UInt32 expected = 0;
        if ( !m_lost.CompareExchangeStrong( expected, 1, MT::MEMORDER_ACQ_REL, MT::MEMORDER_ACQUIRE ) )
            return false;
        m_lostReason = reason;
    }

    GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "ComputeDevice: The device \"" << m_info.GetDeviceName() << "\" of backend \"" <<
        m_info.GetBackendName() << "\" is lost: " << reason );

    GUCEF::vector< TComputeQueuePtr > queues;
    {
        MT::CScopeMutex lock( m_queueLock );
        queues = m_queues;
    }
    for ( size_t i=0; i<queues.size(); ++i )
        queues[ i ]->NotifyDeviceLost();

    CORE::TCloneableString eventData( reason );
    NotifyObserversFromThread( DeviceLostEvent, &eventData );
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace COMPUTE */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
