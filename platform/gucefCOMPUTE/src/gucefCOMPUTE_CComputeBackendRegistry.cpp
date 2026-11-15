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

#include <algorithm>

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

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

#ifndef GUCEF_COMPUTE_CCOMPUTEDEVICE_H
#include "gucefCOMPUTE_CComputeDevice.h"
#define GUCEF_COMPUTE_CCOMPUTEDEVICE_H
#endif /* GUCEF_COMPUTE_CCOMPUTEDEVICE_H ? */

#ifndef GUCEF_COMPUTE_CCOMPUTESETTINGS_H
#include "gucefCOMPUTE_CComputeSettings.h"
#define GUCEF_COMPUTE_CCOMPUTESETTINGS_H
#endif /* GUCEF_COMPUTE_CCOMPUTESETTINGS_H ? */

#ifndef GUCEF_COMPUTE_TYPEFUNCTIONS_H
#include "gucefCOMPUTE_TypeFunctions.h"
#define GUCEF_COMPUTE_TYPEFUNCTIONS_H
#endif /* GUCEF_COMPUTE_TYPEFUNCTIONS_H ? */

#include "gucefCOMPUTE_CComputeBackendRegistry.h"

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

const CORE::CEvent CComputeBackendRegistry::BackendAvailabilityChangedEvent = "GUCEF::COMPUTE::CComputeBackendRegistry::BackendAvailabilityChangedEvent";

/*-------------------------------------------------------------------------*/

namespace {

/** Discrete first, CPU last */
UInt32
DeviceTypeRank( TComputeDeviceType deviceType )
{
    switch ( deviceType )
    {
        case DEVICETYPE_DISCRETE_GPU: return 0;
        case DEVICETYPE_INTEGRATED_GPU: return 1;
        case DEVICETYPE_ACCELERATOR: return 2;
        case DEVICETYPE_VIRTUAL_GPU: return 3;
        case DEVICETYPE_CPU: return 5;
        case DEVICETYPE_UNKNOWN:
        default: return 4;
    }
}

struct SCandidate
{
    CComputeDeviceInfo info;
    TComputeBackendPtr backend;
    bool preferred;
    Int32 settingsOrder;
    Int32 priority;
    UInt32 sequence;
};

struct SCandidateLess
{
    bool operator()( const SCandidate& a, const SCandidate& b ) const
    {
        if ( a.preferred != b.preferred )
            return a.preferred;
        if ( a.settingsOrder != b.settingsOrder )
            return a.settingsOrder < b.settingsOrder;
        const UInt32 rankA = DeviceTypeRank( a.info.GetDeviceType() );
        const UInt32 rankB = DeviceTypeRank( b.info.GetDeviceType() );
        if ( rankA != rankB )
            return rankA < rankB;
        if ( a.priority != b.priority )
            return a.priority > b.priority;
        return a.sequence < b.sequence;
    }
};

}

/*-------------------------------------------------------------------------*/

void
CComputeBackendRegistry::RegisterEvents( void )
{GUCEF_TRACE;

    BackendAvailabilityChangedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CComputeBackendRegistry::CComputeBackendRegistry( CComputeGlobal* global )
    : CORE::CTGlobalONRegistry< CComputeBackend, MT::CMutex >()
    , m_global( global )
    , m_selectionLock()
    , m_forcedFailures()
    , m_enabledBackends()
    , m_disabledBackends()
    , m_backendOrder()
    , m_preferredDevice()
    , m_validation( false )
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CComputeBackendRegistry::~CComputeBackendRegistry()
{GUCEF_TRACE;

    UnregisterAll();
}

/*-------------------------------------------------------------------------*/

const CString&
CComputeBackendRegistry::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString typeName = "GUCEF::COMPUTE::CComputeBackendRegistry";
    return typeName;
}

/*-------------------------------------------------------------------------*/

void
CComputeBackendRegistry::NotifyAvailabilityChanged( const CString& backendName )
{GUCEF_TRACE;

    CORE::TCloneableString eventData( backendName );
    NotifyObservers( BackendAvailabilityChangedEvent, &eventData );
}

/*-------------------------------------------------------------------------*/

bool
CComputeBackendRegistry::InitializeBackend( CComputeBackend& backend )
{GUCEF_TRACE;

    const CString name = backend.GetName().Lowercase();
    bool forcedFailure = false;
    {
        MT::CScopeMutex lock( m_selectionLock );
        forcedFailure = m_forcedFailures.end() != m_forcedFailures.find( name );
    }

    bool available = false;
    if ( !forcedFailure )
    {
        try
        {
            available = backend.Initialize();
        }
        catch ( ... )
        {
            available = false;
        }
    }
    backend.SetAvailable( available );
    if ( !available )
    {
        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "ComputeBackendRegistry: The compute backend \"" << name << "\" is not available on this system and is skipped" );
    }
    else
    {
        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "ComputeBackendRegistry: The compute backend \"" << name << "\" is available" );
    }
    return available;
}

/*-------------------------------------------------------------------------*/

bool
CComputeBackendRegistry::RegisterBackend( const TComputeBackendPtr& backend, bool builtIn )
{GUCEF_TRACE;

    if ( backend.IsNULL() )
        return false;
    const CString name = backend->GetName().Lowercase();
    if ( name.IsNULLOrEmpty() )
        return false;
    if ( !( builtIn ? TryRegisterBuiltIn( name, backend ) : TryRegister( name, backend ) ) )
        return false;

    TComputeBackendPtr registered( backend );
    InitializeBackend( *registered );
    NotifyAvailabilityChanged( name );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CComputeBackendRegistry::TryUnregister( const CString& backendName )
{GUCEF_TRACE;

    const CString name = backendName.Lowercase();
    TComputeBackendPtr backend;
    if ( !TryFindBackend( name, backend ) )
        return false;
    if ( !CORE::CTGlobalONRegistry< CComputeBackend, MT::CMutex >::TryUnregister( name ) )
        return false;

    const bool wasAvailable = backend->IsAvailable();
    backend->SetAvailable( false );

    // the contract is that its devices are gone; those that are not stop working before the backend shuts down
    const UInt32 alive = GUCEF_NULL != m_global ? m_global->MarkDevicesLost( name, "the backend was unregistered", false ) : 0;
    if ( alive > 0 )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_IMPORTANT, "ComputeBackendRegistry: " << alive << " device(s) of backend \"" << name <<
            "\" still exist while it is unregistered, they are lost now" );
    }
    if ( wasAvailable )
        backend->Shutdown();
    NotifyAvailabilityChanged( name );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CComputeBackendRegistry::UnregisterAll( void )
{GUCEF_TRACE;

    CString::StringVector names;
    GetRegisteredObjNames( names );
    for ( size_t i=0; i<names.size(); ++i )
        TryUnregister( names[ i ] );
}

/*-------------------------------------------------------------------------*/

bool
CComputeBackendRegistry::TryFindBackend( const CString& backendName, TComputeBackendPtr& backend ) const
{GUCEF_TRACE;

    TRegisteredObjPtr found;
    if ( !TryLookup( backendName.Lowercase(), found ) || found.IsNULL() )
        return false;
    backend = TComputeBackendPtr( found );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CComputeBackendRegistry::IsEnabledBySettings( const CString& lowerCaseName ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_selectionLock );
    if ( m_disabledBackends.end() != m_disabledBackends.find( lowerCaseName ) )
        return false;
    if ( !m_enabledBackends.empty() && m_enabledBackends.end() == m_enabledBackends.find( lowerCaseName ) )
        return false;
    return true;
}

/*-------------------------------------------------------------------------*/

Int32
CComputeBackendRegistry::GetSettingsOrder( const CString& lowerCaseName ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_selectionLock );
    for ( size_t i=0; i<m_backendOrder.size(); ++i )
        if ( m_backendOrder[ i ] == lowerCaseName )
            return static_cast< Int32 >( i );
    return static_cast< Int32 >( m_backendOrder.size() );
}

/*-------------------------------------------------------------------------*/

bool
CComputeBackendRegistry::IsBackendAvailable( const CString& backendName ) const
{GUCEF_TRACE;

    TComputeBackendPtr backend;
    if ( !TryFindBackend( backendName, backend ) )
        return false;
    return backend->IsAvailable() && IsEnabledBySettings( backendName.Lowercase() );
}

/*-------------------------------------------------------------------------*/

void
CComputeBackendRegistry::GetUnavailableBackendNames( CString::StringVector& backendNames ) const
{GUCEF_TRACE;

    backendNames.clear();
    CString::StringVector names;
    GetRegisteredObjNames( names );
    for ( size_t i=0; i<names.size(); ++i )
        if ( !IsBackendAvailable( names[ i ] ) )
            backendNames.push_back( names[ i ] );
}

/*-------------------------------------------------------------------------*/

void
CComputeBackendRegistry::GetBackendsByPriority( TComputeBackendPtrVector& backends ) const
{GUCEF_TRACE;

    backends.clear();
    TRegisteredObjPtrVector all;
    GetRegisteredObjs( all );

    // insertion sort on (settings order, priority): there are only a handful of backends
    GUCEF::vector< Int32 > orders;
    for ( size_t i=0; i<all.size(); ++i )
    {
        if ( all[ i ].IsNULL() )
            continue;
        TComputeBackendPtr backend( all[ i ] );
        const CString name = backend->GetName().Lowercase();
        if ( !backend->IsAvailable() || !IsEnabledBySettings( name ) )
            continue;
        const Int32 order = GetSettingsOrder( name );
        size_t at = backends.size();
        while ( at > 0 && ( orders[ at-1 ] > order || ( orders[ at-1 ] == order && backends[ at-1 ]->GetPriority() < backend->GetPriority() ) ) )
            --at;
        backends.insert( backends.begin() + at, backend );
        orders.insert( orders.begin() + at, order );
    }
}

/*-------------------------------------------------------------------------*/

bool
CComputeBackendRegistry::EnumerateAllDevices( TComputeDeviceInfoVector& devices ) const
{GUCEF_TRACE;

    devices.clear();
    TComputeBackendPtrVector backends;
    GetBackendsByPriority( backends );

    CString preferred;
    {
        MT::CScopeMutex lock( m_selectionLock );
        preferred = m_preferredDevice;
    }

    GUCEF::vector< SCandidate > candidates;
    UInt32 sequence = 0;
    for ( size_t b=0; b<backends.size(); ++b )
    {
        TComputeDeviceInfoVector found;
        if ( !backends[ b ]->EnumerateDevices( found ) )
            continue;
        const Int32 order = GetSettingsOrder( backends[ b ]->GetName().Lowercase() );
        for ( size_t d=0; d<found.size(); ++d )
        {
            SCandidate candidate;
            candidate.info = found[ d ];
            candidate.backend = backends[ b ];
            candidate.preferred = !preferred.IsNULLOrEmpty() && preferred.Equals( found[ d ].GetIdentifier(), false );
            candidate.settingsOrder = order;
            candidate.priority = backends[ b ]->GetPriority();
            candidate.sequence = sequence++;
            candidates.push_back( candidate );
        }
    }
    std::stable_sort( candidates.begin(), candidates.end(), SCandidateLess() );
    for ( size_t i=0; i<candidates.size(); ++i )
        devices.push_back( candidates[ i ].info );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CComputeBackendRegistry::CreateBestDevice( const CComputeDeviceRequirements& requirements, TComputeDevicePtr& device, TComputeResult* result ) const
{GUCEF_TRACE;

    device.Unlink();
    TComputeDeviceInfoVector devices;
    EnumerateAllDevices( devices );

    bool validation = false;
    {
        MT::CScopeMutex lock( m_selectionLock );
        validation = m_validation;
    }

    TComputeResult lastResult = COMPUTERESULT_UNSUPPORTED;
    for ( size_t i=0; i<devices.size(); ++i )
    {
        if ( !requirements.IsMetBy( devices[ i ] ) )
            continue;
        CComputeDeviceParams params;
        params.SetDeviceIdentifier( devices[ i ].GetIdentifier() );
        if ( validation )
            params.SetValidation( true );
        if ( CreateDevice( devices[ i ].GetBackendName(), params, device, &lastResult ) )
        {
            if ( GUCEF_NULL != result )
                *result = COMPUTERESULT_SUCCESS;
            return true;
        }
    }
    if ( GUCEF_NULL != result )
        *result = lastResult;
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CComputeBackendRegistry::CreateDevice( const CString& backendName, const CComputeDeviceParams& params, TComputeDevicePtr& device, TComputeResult* result ) const
{GUCEF_TRACE;

    device.Unlink();
    TComputeBackendPtr backend;
    TComputeResult outcome = COMPUTERESULT_UNSUPPORTED;
    if ( TryFindBackend( backendName, backend ) && IsBackendAvailable( backendName ) )
    {
        try
        {
            outcome = backend->CreateDevice( params, device );
        }
        catch ( ... )
        {
            outcome = COMPUTERESULT_BACKEND_ERROR;
        }
        if ( COMPUTERESULT_SUCCESS == outcome && device.IsNULL() )
            outcome = COMPUTERESULT_BACKEND_ERROR;
        if ( COMPUTERESULT_SUCCESS != outcome )
        {
            device.Unlink();
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ComputeBackendRegistry: The backend \"" << backendName << "\" could not create a device: " <<
                ComputeResultToString( outcome ) );
        }
    }
    if ( GUCEF_NULL != result )
        *result = outcome;
    return COMPUTERESULT_SUCCESS == outcome;
}

/*-------------------------------------------------------------------------*/

bool
CComputeBackendRegistry::ReinitializeBackend( const CString& backendName )
{GUCEF_TRACE;

    TComputeBackendPtr backend;
    if ( !TryFindBackend( backendName, backend ) )
        return false;

    const bool wasAvailable = backend->IsAvailable();
    if ( wasAvailable )
    {
        backend->SetAvailable( false );
        backend->Shutdown();
    }
    const bool available = InitializeBackend( *backend );
    if ( available != wasAvailable )
        NotifyAvailabilityChanged( backendName.Lowercase() );
    return available;
}

/*-------------------------------------------------------------------------*/

void
CComputeBackendRegistry::SetForcedInitializeFailure( const CString& backendName, bool forceFailure )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_selectionLock );
    if ( forceFailure )
        m_forcedFailures.insert( backendName.Lowercase() );
    else
        m_forcedFailures.erase( backendName.Lowercase() );
}

/*-------------------------------------------------------------------------*/

void
CComputeBackendRegistry::ApplySettings( const CComputeSettings& settings )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_selectionLock );
    m_enabledBackends.clear();
    m_disabledBackends.clear();
    m_backendOrder.clear();
    const CString::StringSet& enabled = settings.GetEnabledBackends();
    for ( CString::StringSet::const_iterator i = enabled.begin(); i != enabled.end(); ++i )
        m_enabledBackends.insert( (*i).Lowercase() );
    const CString::StringSet& disabled = settings.GetDisabledBackends();
    for ( CString::StringSet::const_iterator i = disabled.begin(); i != disabled.end(); ++i )
        m_disabledBackends.insert( (*i).Lowercase() );
    const CString::StringVector& order = settings.GetBackendOrder();
    for ( size_t i=0; i<order.size(); ++i )
        m_backendOrder.push_back( order[ i ].Lowercase() );
    m_preferredDevice = settings.GetPreferredDevice();
    m_validation = settings.GetValidation();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace COMPUTE */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
