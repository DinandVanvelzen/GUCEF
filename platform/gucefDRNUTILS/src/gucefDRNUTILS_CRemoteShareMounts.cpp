/*
 *  gucefDRNUTILS: GUCEF module providing remote view and control of job schedulers and the task manager
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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_DRNUTILS_CDEVICECREDENTIALS_H
#include "gucefDRNUTILS_CDeviceCredentials.h"
#define GUCEF_DRNUTILS_CDEVICECREDENTIALS_H
#endif /* GUCEF_DRNUTILS_CDEVICECREDENTIALS_H ? */

#include "gucefDRNUTILS_CRemoteShareMounts.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRNUTILS {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CEvent CRemoteShareMounts::StateChangedEvent = "GUCEF::DRNUTILS::CRemoteShareMounts::StateChangedEvent";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CRemoteShareMounts::SService::SService( void )
    : slot()
    , settings()
    , declared( false )
    , connection( GUCEF_NULL )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CRemoteShareMounts::SEntry::SEntry( void )
    : settings()
    , mount( GUCEF_NULL )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CRemoteShareMounts::RegisterEvents( void )
{GUCEF_TRACE;

    StateChangedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CRemoteShareMounts::CRemoteShareMounts( const CORE::PulseGeneratorPtr& notificationPulseGenerator, VFS::CVFS* vfs )
    : CORE::CObservingNotifier()
    , m_pulse( notificationPulseGenerator )
    , m_vfs( vfs )
    , m_store( GUCEF_NULL )
    , m_lock()
    , m_services()
    , m_entries()
    , m_started( false )
    , m_lastRaised( REMOTESHARE_IDLE )
{GUCEF_TRACE;

    RegisterEvents();
    CRemoteShareMount::RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CRemoteShareMounts::~CRemoteShareMounts()
{GUCEF_TRACE;

    Stop();
    UnsubscribeAllFromObserver();
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CRemoteShareMounts::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::DRNUTILS::CRemoteShareMounts";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CRemoteShareMounts::NormalizeMountPoint( const CORE::CString& mountPoint )
{GUCEF_TRACE;

    CORE::CString normalized = mountPoint.ReplaceChar( '\\', '/' );
    while ( normalized.Length() > 0 && '/' == normalized[ 0 ] )
        normalized = normalized.CutChars( 1, true, 0 );
    while ( normalized.Length() > 0 && '/' == normalized[ normalized.Length() - 1 ] )
        normalized = normalized.CutChars( 1, false, 0 );
    return normalized;
}

/*-------------------------------------------------------------------------*/

bool
CRemoteShareMounts::LoadConfig( const CORE::CDataNode& config, CORE::CString& error )
{GUCEF_TRACE;

    {
        MT::CScopeMutex lock( m_lock );
        if ( m_started )
        {
            error = "the shares are started: stop them to change their configuration";
            return false;
        }
    }

    TServiceVector services;
    TEntryVector entries;

    if ( config.AttributeBegin() != config.AttributeEnd() )
    {
        error = "the section holds the setting \"" + (*config.AttributeBegin()).first + "\", which is not known: it holds Service and Share children";
        return false;
    }

    // the services that are described
    for ( CORE::CDataNode::const_iterator c = config.ConstBegin(); c != config.ConstEnd(); ++c )
    {
        const CORE::CDataNode& child = *(*c);
        if ( child.GetName() == "Share" )
            continue;
        if ( child.GetName() != "Service" )
        {
            error = "the section holds the child \"" + child.GetName() + "\", which is not known: it holds Service and Share children";
            return false;
        }
        SService service;
        service.slot = child.GetAttributeValue( "slot", CORE::CVariant::Empty ).AsString().Trim( true ).Trim( false );
        if ( service.slot.IsNULLOrEmpty() )
        {
            error = "a Service has no slot: its \"slot\" names the slot of the credentials of the service";
            return false;
        }
        for ( UInt32 i=0; i<services.size(); ++i )
        {
            if ( services[ i ].slot == service.slot )
            {
                error = "the service \"" + service.slot + "\" is described twice";
                return false;
            }
        }
        CORE::CDataNode connectionConfig( child );
        connectionConfig.DelAttribute( "slot" );
        if ( !service.settings.LoadConfig( connectionConfig ) )
        {
            error = "the settings of the service \"" + service.slot + "\" are not usable";
            return false;
        }
        service.declared = true;
        services.push_back( service );
    }

    // the shares
    UInt32 number = 0;
    for ( CORE::CDataNode::const_iterator c = config.ConstBegin(); c != config.ConstEnd(); ++c )
    {
        const CORE::CDataNode& child = *(*c);
        if ( child.GetName() != "Share" )
            continue;
        ++number;
        SEntry entry;
        if ( !entry.settings.LoadConfig( child ) )
        {
            error = "the Share " + CORE::ToString( number ) + " is not usable: a setting is not known or has a value that is none";
            return false;
        }
        CORE::CString reason;
        if ( !entry.settings.IsValid( reason ) )
        {
            error = "the Share " + CORE::ToString( number ) + " is not usable: " + reason;
            return false;
        }
        entry.settings.service = entry.settings.service.Trim( true ).Trim( false );
        if ( entry.settings.service.IsNULLOrEmpty() )
        {
            error = "the Share " + CORE::ToString( number ) + " (\"" + entry.settings.share + "\") names no service: its \"service\" is the slot of the credentials of the service";
            return false;
        }
        if ( CRemoteShareMountSettings::AutoChannel != entry.settings.channel && entry.settings.channel > 2 )
        {
            error = "the Share " + CORE::ToString( number ) + " (\"" + entry.settings.share + "\") names the channel " + CORE::ToString( entry.settings.channel ) + ": the channels are 0 (control) to 2, or auto";
            return false;
        }
        const CORE::CString mountPoint = NormalizeMountPoint( entry.settings.mountPoint );
        if ( mountPoint.IsNULLOrEmpty() )
        {
            error = "the Share " + CORE::ToString( number ) + " (\"" + entry.settings.share + "\") has the root as its mount point";
            return false;
        }
        for ( UInt32 i=0; i<entries.size(); ++i )
        {
            if ( NormalizeMountPoint( entries[ i ].settings.mountPoint ) == mountPoint )
            {
                error = "the mount point \"" + entry.settings.mountPoint + "\" is used by more than one Share";
                return false;
            }
        }
        entries.push_back( entry );
    }

    // a service that a share names and no Service describes connects with the defaults
    for ( UInt32 e=0; e<entries.size(); ++e )
    {
        bool known = false;
        for ( UInt32 s=0; s<services.size() && !known; ++s )
            known = services[ s ].slot == entries[ e ].settings.service;
        if ( !known )
        {
            SService service;
            service.slot = entries[ e ].settings.service;
            services.push_back( service );
        }
    }

    MT::CScopeMutex lock( m_lock );
    if ( m_started )
    {
        error = "the shares are started: stop them to change their configuration";
        return false;
    }
    m_services = services;
    m_entries = entries;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CRemoteShareMounts::SaveConfig( CORE::CDataNode& config ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    for ( UInt32 i=0; i<m_services.size(); ++i )
    {
        if ( !m_services[ i ].declared )
            continue;
        CORE::CDataNode* node = config.AddChild( "Service", GUCEF_DATATYPE_OBJECT );
        m_services[ i ].settings.SaveConfig( *node );
        node->SetAttribute( "slot", m_services[ i ].slot );
    }
    for ( UInt32 i=0; i<m_entries.size(); ++i )
    {
        CORE::CDataNode* node = config.AddChild( "Share", GUCEF_DATATYPE_OBJECT );
        m_entries[ i ].settings.SaveConfig( *node );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
CRemoteShareMounts::SetCredentialStore( AUTH::ICredentialStore* store )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_store = store;
}

/*-------------------------------------------------------------------------*/

bool
CRemoteShareMounts::Start( CORE::CString& error )
{GUCEF_TRACE;

    // the objects are made under the lock, started without it: what they report comes back here, and takes the lock
    GUCEF::vector< CDeviceCredentials > credentials;
    GUCEF::vector< bool > paired;
    GUCEF::vector< CORE::CString > unpairedReason;
    {
        MT::CScopeMutex lock( m_lock );
        if ( m_started )
        {
            error = "the shares are started already";
            return false;
        }
        if ( GUCEF_NULL == m_store && !m_entries.empty() )
        {
            error = "no credential store holds the credentials of the services";
            return false;
        }
        m_started = true;

        credentials.resize( m_services.size() );
        paired.resize( m_services.size(), false );
        unpairedReason.resize( m_services.size() );
        for ( UInt32 s=0; s<m_services.size(); ++s )
        {
            SService& service = m_services[ s ];
            CORE::CString loadError;
            paired[ s ] = credentials[ s ].LoadFromStore( *m_store, service.slot, loadError ) && credentials[ s ].IsComplete();
            if ( !paired[ s ] )
            {
                unpairedReason[ s ] = "not paired: the credential store holds no complete credentials for the service \"" + service.slot + "\"";
                GUCEF_WARNING_LOG( CORE::LOGLEVEL_IMPORTANT, "RemoteShareMounts: " << unpairedReason[ s ] );
            }
            service.connection = GUCEF_NEW CRemoteServiceConnection( m_pulse, service.settings );
            service.connection->SetCredentialStore( m_store, service.slot );
        }
        for ( UInt32 e=0; e<m_entries.size(); ++e )
        {
            SEntry& entry = m_entries[ e ];
            for ( UInt32 s=0; s<m_services.size(); ++s )
            {
                if ( m_services[ s ].slot == entry.settings.service )
                {
                    entry.mount = GUCEF_NEW CRemoteShareMount( *m_services[ s ].connection, entry.settings, m_vfs );
                    TEventCallback onState( this, &CRemoteShareMounts::OnMountState );
                    SubscribeTo( entry.mount, CRemoteShareMount::StateChangedEvent, onState );
                    break;
                }
            }
        }
    }

    // the mounts first: the shares are browsable before the connections are up
    for ( UInt32 e=0; e<m_entries.size(); ++e )
    {
        SEntry& entry = m_entries[ e ];
        UInt32 serviceIndex = 0;
        for ( UInt32 s=0; s<m_services.size(); ++s )
            if ( m_services[ s ].slot == entry.settings.service )
                serviceIndex = s;
        CORE::CString mountError;
        if ( !paired[ serviceIndex ] )
            entry.mount->SetFailed( unpairedReason[ serviceIndex ] );
        else
            entry.mount->Start( credentials[ serviceIndex ].drnSessionName, mountError );
    }

    for ( UInt32 s=0; s<m_services.size(); ++s )
    {
        if ( paired[ s ] )
        {
            CORE::CString connectError;
            if ( !m_services[ s ].connection->StartConnect( credentials[ s ], connectError ) )
            {
                for ( UInt32 e=0; e<m_entries.size(); ++e )
                    if ( m_entries[ e ].settings.service == m_services[ s ].slot )
                        m_entries[ e ].mount->SetFailed( "the service \"" + m_services[ s ].slot + "\" cannot be connected: " + connectError );
            }
        }
        credentials[ s ].Wipe();
    }

    RaiseIfChanged();
    return true;
}

/*-------------------------------------------------------------------------*/

void
CRemoteShareMounts::Stop( void )
{GUCEF_TRACE;

    GUCEF::vector< CRemoteShareMount* > mounts;
    GUCEF::vector< CRemoteServiceConnection* > connections;
    {
        MT::CScopeMutex lock( m_lock );
        if ( !m_started )
            return;
        for ( UInt32 e=0; e<m_entries.size(); ++e )
        {
            mounts.push_back( m_entries[ e ].mount );
            m_entries[ e ].mount = GUCEF_NULL;
        }
        for ( UInt32 s=0; s<m_services.size(); ++s )
        {
            connections.push_back( m_services[ s ].connection );
            m_services[ s ].connection = GUCEF_NULL;
        }
        m_started = false;
    }

    // the mounts first, they hold the connections; no lock: they report their last state to this class
    for ( UInt32 i=0; i<mounts.size(); ++i )
    {
        if ( GUCEF_NULL != mounts[ i ] )
        {
            mounts[ i ]->Stop();
            UnsubscribeFrom( mounts[ i ] );
            GUCEF_DELETE mounts[ i ];
        }
    }
    for ( UInt32 i=0; i<connections.size(); ++i )
    {
        if ( GUCEF_NULL != connections[ i ] )
        {
            connections[ i ]->Close( true, 3000 );
            GUCEF_DELETE connections[ i ];
        }
    }
    RaiseIfChanged();
}

/*-------------------------------------------------------------------------*/

bool
CRemoteShareMounts::IsStarted( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_started;
}

/*-------------------------------------------------------------------------*/

UInt32
CRemoteShareMounts::GetEntryCount( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return (UInt32) m_entries.size();
}

/*-------------------------------------------------------------------------*/

UInt32
CRemoteShareMounts::GetServiceCount( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return (UInt32) m_services.size();
}

/*-------------------------------------------------------------------------*/

TRemoteShareState
CRemoteShareMounts::GetEntryState( UInt32 index ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    if ( index >= m_entries.size() || GUCEF_NULL == m_entries[ index ].mount )
        return REMOTESHARE_IDLE;
    return m_entries[ index ].mount->GetState();
}

/*-------------------------------------------------------------------------*/

CORE::CString
CRemoteShareMounts::GetEntryError( UInt32 index ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    if ( index >= m_entries.size() || GUCEF_NULL == m_entries[ index ].mount )
        return CORE::CString::Empty;
    return m_entries[ index ].mount->GetLastError();
}

/*-------------------------------------------------------------------------*/

const CRemoteShareMountSettings*
CRemoteShareMounts::GetEntrySettings( UInt32 index ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return index < m_entries.size() ? &m_entries[ index ].settings : GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

CRemoteShareMount*
CRemoteShareMounts::GetEntryMount( UInt32 index ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return index < m_entries.size() ? m_entries[ index ].mount : GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

CRemoteShareMount*
CRemoteShareMounts::FindByMountPoint( const CORE::CString& mountPoint ) const
{GUCEF_TRACE;

    const CORE::CString wanted = NormalizeMountPoint( mountPoint );
    MT::CScopeMutex lock( m_lock );
    for ( UInt32 i=0; i<m_entries.size(); ++i )
    {
        if ( NormalizeMountPoint( m_entries[ i ].settings.mountPoint ) == wanted )
            return m_entries[ i ].mount;
    }
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

CRemoteServiceConnection*
CRemoteShareMounts::GetConnection( const CORE::CString& slot ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    for ( UInt32 i=0; i<m_services.size(); ++i )
    {
        if ( m_services[ i ].slot == slot )
            return m_services[ i ].connection;
    }
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

UInt32
CRemoteShareMounts::GetStateCount( TRemoteShareState state ) const
{GUCEF_TRACE;

    UInt32 count = 0;
    MT::CScopeMutex lock( m_lock );
    for ( UInt32 i=0; i<m_entries.size(); ++i )
    {
        if ( GUCEF_NULL != m_entries[ i ].mount && state == m_entries[ i ].mount->GetState() )
            ++count;
    }
    return count;
}

/*-------------------------------------------------------------------------*/

TRemoteShareState
CRemoteShareMounts::ComputeStateLocked( void ) const
{GUCEF_TRACE;

    if ( !m_started || m_entries.empty() )
        return REMOTESHARE_IDLE;

    UInt32 failed = 0;
    UInt32 online = 0;
    UInt32 offline = 0;
    for ( UInt32 i=0; i<m_entries.size(); ++i )
    {
        const TRemoteShareState state = GUCEF_NULL != m_entries[ i ].mount ? m_entries[ i ].mount->GetState() : REMOTESHARE_IDLE;
        if ( REMOTESHARE_FAILED == state )
            ++failed;
        else
        if ( REMOTESHARE_ONLINE == state )
            ++online;
        else
        if ( REMOTESHARE_OFFLINE == state )
            ++offline;
    }
    if ( failed == m_entries.size() )
        return REMOTESHARE_FAILED;
    if ( online + failed == m_entries.size() )
        return REMOTESHARE_ONLINE;
    if ( offline > 0 )
        return REMOTESHARE_OFFLINE;
    return REMOTESHARE_WAITING;
}

/*-------------------------------------------------------------------------*/

TRemoteShareState
CRemoteShareMounts::GetState( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return ComputeStateLocked();
}

/*-------------------------------------------------------------------------*/

void
CRemoteShareMounts::RaiseIfChanged( void )
{GUCEF_TRACE;

    TRemoteShareState state = REMOTESHARE_IDLE;
    bool changed = false;
    {
        MT::CScopeMutex lock( m_lock );
        state = ComputeStateLocked();
        changed = state != m_lastRaised;
        m_lastRaised = state;
    }
    if ( changed )
    {
        TRemoteShareStateEventData data( state );
        NotifyObservers( StateChangedEvent, &data );
    }
}

/*-------------------------------------------------------------------------*/

void
CRemoteShareMounts::OnMountState( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    RaiseIfChanged();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNUTILS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
