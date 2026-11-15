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

#ifndef GUCEF_CORE_CPLUGINCONTROL_H
#include "CPluginControl.h"
#define GUCEF_CORE_CPLUGINCONTROL_H
#endif /* GUCEF_CORE_CPLUGINCONTROL_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */

#ifndef GUCEF_COMPUTE_CCOMPUTEBACKENDREGISTRY_H
#include "gucefCOMPUTE_CComputeBackendRegistry.h"
#define GUCEF_COMPUTE_CCOMPUTEBACKENDREGISTRY_H
#endif /* GUCEF_COMPUTE_CCOMPUTEBACKENDREGISTRY_H ? */

#ifndef GUCEF_COMPUTE_CCOMPUTEDEVICE_H
#include "gucefCOMPUTE_CComputeDevice.h"
#define GUCEF_COMPUTE_CCOMPUTEDEVICE_H
#endif /* GUCEF_COMPUTE_CCOMPUTEDEVICE_H ? */

#ifndef GUCEF_COMPUTE_CCOMPUTEKERNELREGISTRY_H
#include "gucefCOMPUTE_CComputeKernelRegistry.h"
#define GUCEF_COMPUTE_CCOMPUTEKERNELREGISTRY_H
#endif /* GUCEF_COMPUTE_CCOMPUTEKERNELREGISTRY_H ? */

#ifndef GUCEF_COMPUTE_CCOMPUTESETTINGS_H
#include "gucefCOMPUTE_CComputeSettings.h"
#define GUCEF_COMPUTE_CCOMPUTESETTINGS_H
#endif /* GUCEF_COMPUTE_CCOMPUTESETTINGS_H ? */

#ifndef GUCEF_COMPUTE_CCPUCOMPUTEBACKEND_H
#include "gucefCOMPUTE_CCpuComputeBackend.h"
#define GUCEF_COMPUTE_CCPUCOMPUTEBACKEND_H
#endif /* GUCEF_COMPUTE_CCPUCOMPUTEBACKEND_H ? */

#ifndef GUCEF_COMPUTE_CCPUPARALLELFOR_H
#include "gucefCOMPUTE_CCpuParallelFor.h"
#define GUCEF_COMPUTE_CCPUPARALLELFOR_H
#endif /* GUCEF_COMPUTE_CCPUPARALLELFOR_H ? */

#include "gucefCOMPUTE_CComputeGlobal.h"

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

static CComputeGlobal* g_instance = GUCEF_NULL;
static bool g_unloaded = false;     /**< set when the module unloaded: the global is not created again until it loads */

/*-------------------------------------------------------------------------*/

CComputeGlobal*
CComputeGlobal::Instance( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == g_instance )
    {
        if ( g_unloaded )
            return GUCEF_NULL;

        // the modules this one depends on first
        CORE::CCoreGlobal::Instance();
        VFS::CVfsGlobal::Instance();

        g_instance = GUCEF_NEW CComputeGlobal();
        g_instance->Initialize();
    }
    return g_instance;
}

/*-------------------------------------------------------------------------*/

CComputeGlobal*
CComputeGlobal::InstanceIfCreated( void )
{GUCEF_TRACE;

    return g_instance;
}

/*-------------------------------------------------------------------------*/

void
CComputeGlobal::AllowCreation( void )
{GUCEF_TRACE;

    g_unloaded = false;
}

/*-------------------------------------------------------------------------*/

void
CComputeGlobal::Deinstance( void )
{GUCEF_TRACE;

    g_unloaded = true;
    CComputeGlobal* global = g_instance;
    if ( GUCEF_NULL == global )
        return;

    // devices the application still holds: their work fails now, before the systems it uses go
    const UInt32 held = global->MarkDevicesLost( CString::Empty, "the gucefCOMPUTE module was unloaded", true );
    if ( held > 0 )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_IMPORTANT, "ComputeGlobal: " << held << " device(s) still exist while the module unloads, they are lost now" );
    }
    g_instance = GUCEF_NULL;
    GUCEF_DELETE global;
}

/*-------------------------------------------------------------------------*/

CComputeGlobal::CComputeGlobal( void )
    : m_settings( GUCEF_NULL )
    , m_settingsLock()
    , m_kernelRegistry( GUCEF_NULL )
    , m_cpuParallelFor()
    , m_deviceLock()
    , m_devices()
    , m_backendRegistry( GUCEF_NULL )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CComputeGlobal::~CComputeGlobal()
{GUCEF_TRACE;

    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "Shutting down gucefCOMPUTE global systems" );
    CORE::CCoreGlobal::Instance()->GetPluginControl().UnlistModuleForLinkBack( "gucefCOMPUTE" );

    // reverse order of creation
    if ( GUCEF_NULL != m_backendRegistry )
        m_backendRegistry->UnregisterAll();
    GUCEF_DELETE m_backendRegistry;
    m_backendRegistry = GUCEF_NULL;

    // queues of devices that still exist hold the job system: it lives on with them, without workers
    if ( !m_cpuParallelFor.IsNULL() )
        m_cpuParallelFor->Shutdown();
    m_cpuParallelFor.Unlink();

    GUCEF_DELETE m_kernelRegistry;
    m_kernelRegistry = GUCEF_NULL;

    GUCEF_DELETE m_settings;
    m_settings = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

void
CComputeGlobal::Initialize( void )
{GUCEF_TRACE;

    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "Initializing gucefCOMPUTE global systems" );

    CComputeDevice::RegisterEvents();
    CComputeBackendRegistry::RegisterEvents();

    m_settings = GUCEF_NEW CComputeSettings();
    m_kernelRegistry = GUCEF_NEW CComputeKernelRegistry();
    m_cpuParallelFor = ( GUCEF_NEW CCpuParallelFor() )->CreateSharedPtr();
    m_backendRegistry = GUCEF_NEW CComputeBackendRegistry( this );

    // the backend that needs no runtime and no plugin
    m_backendRegistry->RegisterBackend( TComputeBackendPtr( GUCEF_NEW CCpuComputeBackend() ) );

    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "gucefCOMPUTE global systems initialized" );
    CORE::CCoreGlobal::Instance()->GetPluginControl().ListModuleForLinkBack( "gucefCOMPUTE" );
}

/*-------------------------------------------------------------------------*/

CComputeBackendRegistry&
CComputeGlobal::GetBackendRegistry( void )
{GUCEF_TRACE;

    return *m_backendRegistry;
}

/*-------------------------------------------------------------------------*/

CComputeKernelRegistry&
CComputeGlobal::GetKernelRegistry( void )
{GUCEF_TRACE;

    return *m_kernelRegistry;
}

/*-------------------------------------------------------------------------*/

const CComputeKernelRegistry&
CComputeGlobal::GetKernelRegistry( void ) const
{GUCEF_TRACE;

    return *m_kernelRegistry;
}

/*-------------------------------------------------------------------------*/

CCpuParallelFor&
CComputeGlobal::GetCpuParallelFor( void )
{GUCEF_TRACE;

    return *m_cpuParallelFor;
}

/*-------------------------------------------------------------------------*/

TCpuParallelForPtr
CComputeGlobal::GetCpuParallelForPtr( void )
{GUCEF_TRACE;

    return m_cpuParallelFor;
}

/*-------------------------------------------------------------------------*/

UInt32
CComputeGlobal::GetDeviceCount( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_deviceLock );
    return static_cast< UInt32 >( m_devices.size() );
}

/*-------------------------------------------------------------------------*/

void
CComputeGlobal::RegisterDevice( CComputeDevice* device )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_deviceLock );
    m_devices.insert( device );
    device->m_global.Store( this, MT::MEMORDER_RELEASE );
}

/*-------------------------------------------------------------------------*/

void
CComputeGlobal::UnregisterDevice( CComputeDevice* device )
{GUCEF_TRACE;

    // the device calls it first in its shut down; while the global works with devices this waits
    MT::CScopeMutex lock( m_deviceLock );
    m_devices.erase( device );
}

/*-------------------------------------------------------------------------*/

UInt32
CComputeGlobal::MarkDevicesLost( const CString& backendName, const CString& reason, bool release )
{GUCEF_TRACE;

    UInt32 marked = 0;
    {
        // A device in the set is alive: one that shuts down waits in UnregisterDevice() for this lock, or, when this
        // took it over, until it is no longer busy. Work on a copy: notifying observers may create devices
        MT::CScopeMutex lock( m_deviceLock );
        GUCEF::set< CComputeDevice* > devices = m_devices;
        for ( GUCEF::set< CComputeDevice* >::iterator i = devices.begin(); i != devices.end(); ++i )
        {
            CComputeDevice* device = (*i);
            if ( !backendName.IsNULLOrEmpty() && !device->GetBackendName().Equals( backendName, false ) )
                continue;

            device->m_globalBusy.Store( 1, MT::MEMORDER_SEQ_CST );
            const bool ours = !release || GUCEF_NULL != device->m_global.Exchange( GUCEF_NULL, MT::MEMORDER_SEQ_CST );
            if ( ours )
            {
                device->MarkLost( reason );
                ++marked;
                if ( release )
                    m_devices.erase( device );
            }
            device->m_globalBusy.Store( 0, MT::MEMORDER_SEQ_CST );
        }
    }

    // devices that were shutting down meanwhile leave on their own
    while ( release )
    {
        {
            MT::CScopeMutex lock( m_deviceLock );
            if ( m_devices.empty() )
                break;
        }
        MT::PrecisionDelay( 1 );
    }
    return marked;
}

/*-------------------------------------------------------------------------*/

CComputeSettings
CComputeGlobal::GetSettings( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_settingsLock );
    return *m_settings;
}

/*-------------------------------------------------------------------------*/

void
CComputeGlobal::ApplySettings( const CComputeSettings& settings )
{GUCEF_TRACE;

    UInt32 previousWorkers = 0;
    {
        MT::CScopeMutex lock( m_settingsLock );
        previousWorkers = m_settings->GetMaxCpuWorkerCount();
        *m_settings = settings;
    }
    m_backendRegistry->ApplySettings( settings );
    m_kernelRegistry->SetOverrideRoot( settings.GetKernelOverrideRoot() );
    if ( previousWorkers != settings.GetMaxCpuWorkerCount() || m_cpuParallelFor->GetWorkerCount() != ( 0 == settings.GetMaxCpuWorkerCount() ? CCpuParallelFor::GetDefaultWorkerCount() : settings.GetMaxCpuWorkerCount() ) )
        m_cpuParallelFor->SetWorkerCount( settings.GetMaxCpuWorkerCount() );
}

/*-------------------------------------------------------------------------*/

UInt32
CComputeGlobal::Update( void )
{GUCEF_TRACE;

    // the finished work is deleted after the lock and the devices were let go: that may release the last reference to a device
    TComputePendingWorkVector finished;
    {
        MT::CScopeMutex lock( m_deviceLock );
        GUCEF::set< CComputeDevice* > devices = m_devices;
        for ( GUCEF::set< CComputeDevice* >::iterator i = devices.begin(); i != devices.end(); ++i )
            (*i)->ProcessCooperativeWork( finished );
    }
    const UInt32 count = static_cast< UInt32 >( finished.size() );
    for ( size_t i=0; i<finished.size(); ++i )
        GUCEF_DELETE finished[ i ];

    CComputeBackendRegistry::TComputeBackendPtrVector backends;
    m_backendRegistry->GetBackendsByPriority( backends );
    for ( size_t i=0; i<backends.size(); ++i )
        backends[ i ]->Update();
    return count;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace COMPUTE */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
