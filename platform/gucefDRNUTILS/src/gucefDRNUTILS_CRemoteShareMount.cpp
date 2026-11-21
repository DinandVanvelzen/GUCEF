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

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */

#ifndef GUCEF_VFS_CARCHIVESETTINGS_H
#include "gucefVFS_CArchiveSettings.h"
#define GUCEF_VFS_CARCHIVESETTINGS_H
#endif /* GUCEF_VFS_CARCHIVESETTINGS_H ? */

#ifndef GUCEF_DRN_CDRNGLOBAL_H
#include "gucefDRN_CDRNGlobal.h"
#define GUCEF_DRN_CDRNGLOBAL_H
#endif /* GUCEF_DRN_CDRNGLOBAL_H ? */

#ifndef GUCEF_DRN_CDRNSESSIONREGISTRY_H
#include "gucefDRN_CDRNSessionRegistry.h"
#define GUCEF_DRN_CDRNSESSIONREGISTRY_H
#endif /* GUCEF_DRN_CDRNSESSIONREGISTRY_H ? */

#ifndef GUCEF_DRNUTILS_CDRNUTILSGLOBAL_H
#include "gucefDRNUTILS_CDrnUtilsGlobal.h"
#define GUCEF_DRNUTILS_CDRNUTILSGLOBAL_H
#endif /* GUCEF_DRNUTILS_CDRNUTILSGLOBAL_H ? */

#include "gucefDRNUTILS_CRemoteShareMount.h"

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

const CORE::CEvent CRemoteShareMount::StateChangedEvent = "GUCEF::DRNUTILS::CRemoteShareMount::StateChangedEvent";
const char* const CRemoteShareMount::ArchiveTypeName = "drn";
const char* const CRemoteShareMount::MemberName = "sequencer";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CRemoteShareMount::RegisterEvents( void )
{GUCEF_TRACE;

    StateChangedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CRemoteShareMount::CRemoteShareMount( CRemoteServiceConnection& connection, const CRemoteShareMountSettings& settings, VFS::CVFS* vfs )
    : CORE::CObservingNotifier()
    , m_lock()
    , m_connection( &connection )
    , m_settings( settings )
    , m_vfs( vfs )
    , m_started( false )
    , m_failedSticky( false )
    , m_wasOnline( false )
    , m_state( REMOTESHARE_IDLE )
    , m_lastError()
    , m_channel( CRemoteShareMountSettings::AutoChannel )
    , m_sessionName()
    , m_address()
    , m_archiveName()
    , m_mounted( false )
    , m_archive( GUCEF_NULL )
    , m_registeredSession( GUCEF_NULL )
    , m_heldSession( GUCEF_NULL )
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CRemoteShareMount::~CRemoteShareMount()
{GUCEF_TRACE;

    Stop();
    UnsubscribeAllFromObserver();
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CRemoteShareMount::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::DRNUTILS::CRemoteShareMount";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

VFS::CVFS&
CRemoteShareMount::GetVfs( void ) const
{GUCEF_TRACE;

    return GUCEF_NULL != m_vfs ? *m_vfs : VFS::CVfsGlobal::Instance()->GetVfs();
}

/*-------------------------------------------------------------------------*/

bool
CRemoteShareMount::SetStateLocked( TRemoteShareState state, const CORE::CString& error )
{GUCEF_TRACE;

    const bool changed = m_state != state || ( REMOTESHARE_FAILED == state && m_lastError != error );
    m_state = state;
    if ( REMOTESHARE_FAILED == state )
        m_lastError = error;
    return changed;
}

/*-------------------------------------------------------------------------*/

void
CRemoteShareMount::Raise( bool changed, TRemoteShareState state )
{GUCEF_TRACE;

    if ( !changed )
        return;
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "RemoteShareMount: The share at \"" << m_settings.mountPoint << "\" is " << RemoteShareStateToString( state ) );
    TRemoteShareStateEventData data( state );
    NotifyObservers( StateChangedEvent, &data );
}

/*-------------------------------------------------------------------------*/

void
CRemoteShareMount::ReleaseRegistration( void )
{GUCEF_TRACE;

    // only what this mount registered, and only while the registry still has it: a session that was destroyed is forgotten by the registry,
    // and the pointer this mount kept of it is not used
    // the registration is shared by the mounts of one connection and channel: it goes with the last of them to let go of it
    m_registeredSession = GUCEF_NULL;
    if ( GUCEF_NULL == m_heldSession )
        return;
    DRN::CDRNSession* held = m_heldSession;
    m_heldSession = GUCEF_NULL;
    if ( !CDrnUtilsGlobal::Instance()->GetShareSessionHolds().Release( held ) )
        return;
    DRN::CDRNSessionRegistry& registry = DRN::CDRNGlobal::Instance()->GetSessionRegistry();
    if ( registry.Find( m_sessionName ) == held )
        registry.Unregister( *held );
}

/*-------------------------------------------------------------------------*/

void
CRemoteShareMount::Unmount( void )
{GUCEF_TRACE;

    if ( !m_mounted )
        return;
    m_mounted = false;

    // only the archive this mount made: the name could have been given to another archive since
    VFS::CVFS& vfs = GetVfs();
    VFS::CVFS::TArchivePtr archive = vfs.GetMountedArchive( m_archiveName );
    if ( !archive.IsNULL() && archive.GetPointerAlways() == m_archive )
        vfs.UnmountArchiveByName( m_archiveName );
    m_archive = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

bool
CRemoteShareMount::Follow( void )
{GUCEF_TRACE;

    if ( !m_started )
        return false;
    if ( GUCEF_NULL == m_connection )
        return SetStateLocked( REMOTESHARE_FAILED, "the connection to the service is gone" );

    DRN::CDRNSessionRegistry& registry = DRN::CDRNGlobal::Instance()->GetSessionRegistry();
    DRN::CDRNSession* session = m_connection->GetDrnSession( m_channel );
    const bool sessionIsHere = GUCEF_NULL != session && session->GetName() == m_sessionName;

    // a registration of a session that is not the connection's session now (replaced, or gone) goes
    if ( GUCEF_NULL != m_heldSession && ( !sessionIsHere || m_heldSession != session ) )
        ReleaseRegistration();

    bool sessionRegistered = false;
    if ( sessionIsHere && !m_failedSticky )
    {
        DRN::CDRNSession* registered = registry.Find( m_sessionName );
        if ( GUCEF_NULL == registered )
        {
            if ( registry.Register( *session ) )
            {
                m_registeredSession = session;
                m_heldSession = session;
                CDrnUtilsGlobal::Instance()->GetShareSessionHolds().Created( session );
                sessionRegistered = true;
            }
        }
        else
        if ( registered == session )
        {
            // registered already: by the mount of another share of the service, whose registration is shared (counted), or by the
            // application, which keeps it (it is not ours to remove)
            if ( GUCEF_NULL == m_heldSession && CDrnUtilsGlobal::Instance()->GetShareSessionHolds().Join( session ) )
                m_heldSession = session;
            sessionRegistered = true;
        }
        else
        {
            // another session has the name: the mount by that name would show the share of that one
            m_failedSticky = true;
            m_lastError = "the DRN session name \"" + m_sessionName + "\" is registered for another session than the one of this connection (channel " + CORE::ToString( m_channel ) + ")";
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "RemoteShareMount: " << m_lastError << ": the mount at \"" << m_settings.mountPoint << "\" is taken away" );
            Unmount();
        }
    }

    if ( m_failedSticky )
        return SetStateLocked( REMOTESHARE_FAILED, m_lastError );

    const TRemoteConnectionState connectionState = m_connection->GetState();
    if ( REMOTECONNECTION_FAILED == connectionState )
        return SetStateLocked( REMOTESHARE_FAILED, "the connection to the service gave up: " + m_connection->GetLastError() );
    if ( REMOTECONNECTION_CONNECTED == connectionState && sessionRegistered && ( 0 == m_channel || m_connection->IsBulkChannelReady( m_channel ) ) )
    {
        m_wasOnline = true;
        return SetStateLocked( REMOTESHARE_ONLINE, CORE::CString::Empty );
    }
    return SetStateLocked( m_wasOnline ? REMOTESHARE_OFFLINE : REMOTESHARE_WAITING, CORE::CString::Empty );
}

/*-------------------------------------------------------------------------*/

bool
CRemoteShareMount::StartImp( const CORE::CString& drnSessionName, CORE::CString& error )
{GUCEF_TRACE;

    if ( m_started )
    {
        error = "the mount is started already";
        return false;
    }
    if ( m_failedSticky )
    {
        error = "the mount was marked as failed (" + m_lastError + "): stop it first";
        return false;
    }
    m_wasOnline = false;

    // what is refused before anything is mounted
    CORE::CString reason;
    if ( GUCEF_NULL == m_connection )
        reason = "the connection to the service is gone";
    else
    if ( !m_settings.IsValid( reason ) )
        reason = "the settings are not usable: " + reason;
    if ( reason.IsNULLOrEmpty() )
    {
        if ( CRemoteShareMountSettings::AutoChannel == m_settings.channel )
            m_channel = GUCEF_NULL != m_connection->GetChannel( 1 ) ? 1 : 0;
        else
        if ( GUCEF_NULL == m_connection->GetChannel( m_settings.channel ) )
            reason = "the connection has no channel " + CORE::ToString( m_settings.channel );
        else
            m_channel = m_settings.channel;
    }
    CORE::CString name = drnSessionName;
    if ( reason.IsNULLOrEmpty() && name.IsNULLOrEmpty() )
    {
        CDeviceCredentials credentials = m_connection->GetCredentials();
        name = credentials.drnSessionName;
        credentials.Wipe();
        if ( name.IsNULLOrEmpty() )
            reason = "the DRN session name of the service is not known: it is the drnSessionName of the credentials, which the connection has once it was started";
    }

    DRN::CDRNSessionRegistry& registry = DRN::CDRNGlobal::Instance()->GetSessionRegistry();
    if ( reason.IsNULLOrEmpty() )
    {
        // the name is the address of the mount: a session of that name that is not the connection's is another service's
        DRN::CDRNSession* registered = registry.Find( name );
        if ( GUCEF_NULL != registered && registered != m_connection->GetDrnSession( m_channel ) )
            reason = "the DRN session name \"" + name + "\" is registered for another session than the one of this connection";
    }

    VFS::CVFS& vfs = GetVfs();
    const CORE::CString archiveName = "remoteshare:" + m_settings.mountPoint;
    if ( reason.IsNULLOrEmpty() )
    {
        VFS::CVFS::TAbstractArchiveFactory::TKeySet types;
        vfs.GetListOfSupportedArchiveTypes( types );
        if ( types.find( ArchiveTypeName ) == types.end() )
            reason = "the VFS does not know the archive type \"" + CORE::CString( ArchiveTypeName ) + "\": the VFS plugin for DRN is not loaded";
        else
        if ( vfs.IsMountedArchive( m_settings.mountPoint ) )
            reason = "the mount point \"" + m_settings.mountPoint + "\" is in use by another archive: mounting there would overlay it";
        else
        if ( !vfs.GetMountedArchive( archiveName ).IsNULL() )
            reason = "an archive named \"" + archiveName + "\" is mounted already";
    }

    if ( reason.IsNULLOrEmpty() )
    {
        // the archive settings in the form a configuration holds them: the settings of the bridge archive go in as its archive specific settings
        const CORE::CString address = "drn://" + name + "/" + MemberName + "/" + m_settings.share;
        CORE::CDataNode config;
        config.SetAttribute( "archiveName", archiveName );
        config.SetAttribute( "archiveType", CORE::CString( ArchiveTypeName ) );
        config.SetAttribute( "actualArchivePath", address );
        config.SetAttribute( "path", address );
        config.SetAttribute( "mountPath", m_settings.mountPoint );
        config.SetAttribute( "writeable", false );
        for ( CORE::CDataNode::TAttributeMap::const_iterator a = m_settings.bridgeSettings.AttributeBegin(); a != m_settings.bridgeSettings.AttributeEnd(); ++a )
            config.SetAttribute( ( VFS::CArchiveSettings::ArchiveSpecificSettingsPrefix + (*a).first ).C_String(), (*a).second.AsString() );
        VFS::CArchiveSettings archiveSettings;
        archiveSettings.LoadConfig( config );
        archiveSettings.SetActualArchivePath( address );

        if ( !vfs.MountArchive( archiveSettings ) )
            reason = "the VFS refused the mount of \"" + address + "\" at \"" + m_settings.mountPoint + "\"";
        else
        {
            VFS::CVFS::TArchivePtr archive = vfs.GetMountedArchive( archiveName );
            if ( archive.IsNULL() )
                reason = "the archive \"" + archiveName + "\" is not in the VFS after it was mounted";
            else
            {
                m_mounted = true;
                m_archive = archive.GetPointerAlways();
                m_archiveName = archiveName;
                m_address = address;
                m_sessionName = name;
            }
        }
    }

    if ( !reason.IsNULLOrEmpty() )
    {
        error = reason;
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "RemoteShareMount: Cannot mount share \"" << m_settings.share << "\" at \"" << m_settings.mountPoint << "\": " << reason );
        SetStateLocked( REMOTESHARE_FAILED, reason );
        return false;
    }

    m_started = true;
    TEventCallback onConnectionEvent( this, &CRemoteShareMount::OnConnectionEvent );
    SubscribeTo( m_connection, CRemoteServiceConnection::StateChangedEvent, onConnectionEvent );
    TEventCallback onChannelEvent( this, &CRemoteShareMount::OnConnectionEvent );
    SubscribeTo( m_connection, CRemoteServiceConnection::ChannelChangedEvent, onChannelEvent );

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "RemoteShareMount: Mounted share \"" << m_settings.share << "\" of the service at \"" << m_settings.mountPoint << "\" by the session name \"" << name << "\", channel " << m_channel );
    SetStateLocked( REMOTESHARE_WAITING, CORE::CString::Empty );
    Follow();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CRemoteShareMount::Start( const CORE::CString& drnSessionName, CORE::CString& error )
{GUCEF_TRACE;

    bool ok = false;
    bool changed = false;
    TRemoteShareState state = REMOTESHARE_IDLE;
    {
        MT::CScopeMutex lock( m_lock );
        const TRemoteShareState before = m_state;
        const CORE::CString errorBefore = m_lastError;
        ok = StartImp( drnSessionName, error );
        state = m_state;
        changed = before != state || ( REMOTESHARE_FAILED == state && errorBefore != m_lastError );
    }
    Raise( changed, state );
    return ok;
}

/*-------------------------------------------------------------------------*/

bool
CRemoteShareMount::Start( CORE::CString& error )
{GUCEF_TRACE;

    return Start( CORE::CString::Empty, error );
}

/*-------------------------------------------------------------------------*/

void
CRemoteShareMount::Stop( void )
{GUCEF_TRACE;

    bool changed = false;
    {
        MT::CScopeMutex lock( m_lock );
        if ( GUCEF_NULL != m_connection )
        {
            UnsubscribeFrom( m_connection );
        }
        ReleaseRegistration();
        Unmount();
        m_started = false;
        m_failedSticky = false;
        m_wasOnline = false;
        m_lastError.Clear();
        changed = m_state != REMOTESHARE_IDLE;
        m_state = REMOTESHARE_IDLE;
    }
    Raise( changed, REMOTESHARE_IDLE );
}

/*-------------------------------------------------------------------------*/

void
CRemoteShareMount::SetFailed( const CORE::CString& reason )
{GUCEF_TRACE;

    bool changed = false;
    {
        MT::CScopeMutex lock( m_lock );
        m_failedSticky = true;
        changed = SetStateLocked( REMOTESHARE_FAILED, reason );
    }
    Raise( changed, REMOTESHARE_FAILED );
}

/*-------------------------------------------------------------------------*/

void
CRemoteShareMount::OnConnectionEvent( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    bool changed = false;
    TRemoteShareState state = REMOTESHARE_IDLE;
    {
        MT::CScopeMutex lock( m_lock );
        changed = Follow();
        state = m_state;
    }
    Raise( changed, state );
}

/*-------------------------------------------------------------------------*/

void
CRemoteShareMount::OnNotify( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( CORE::CNotifier::DestructionEvent != eventId )
        return;

    // the connection is going: what follows it follows no more
    bool changed = false;
    {
        MT::CScopeMutex lock( m_lock );
        if ( static_cast< CORE::CNotifier* >( m_connection ) != notifier )
            return;
        m_connection = GUCEF_NULL;
        if ( !m_started )
            return;
        ReleaseRegistration();
        Unmount();
        m_started = false;
        m_failedSticky = true;
        changed = SetStateLocked( REMOTESHARE_FAILED, "the connection to the service was destroyed while the mount was started" );
    }
    Raise( changed, REMOTESHARE_FAILED );
}

/*-------------------------------------------------------------------------*/

TRemoteShareState
CRemoteShareMount::GetState( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_state;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CRemoteShareMount::GetLastError( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_lastError;
}

/*-------------------------------------------------------------------------*/

const CRemoteShareMountSettings&
CRemoteShareMount::GetSettings( void ) const
{GUCEF_TRACE;

    return m_settings;
}

/*-------------------------------------------------------------------------*/

bool
CRemoteShareMount::IsStarted( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_started;
}

/*-------------------------------------------------------------------------*/

bool
CRemoteShareMount::IsMounted( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    if ( !m_mounted )
        return false;
    VFS::CVFS::TArchivePtr archive = GetVfs().GetMountedArchive( m_archiveName );
    return !archive.IsNULL() && archive.GetPointerAlways() == m_archive;
}

/*-------------------------------------------------------------------------*/

UInt32
CRemoteShareMount::GetChannel( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_channel;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CRemoteShareMount::GetSessionName( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_sessionName;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CRemoteShareMount::GetAddress( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_address;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CRemoteShareMount::GetArchiveName( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_archiveName;
}

/*-------------------------------------------------------------------------*/

bool
CRemoteShareMount::HasRegisteredSession( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return GUCEF_NULL != m_registeredSession;
}

/*-------------------------------------------------------------------------*/

bool
CRemoteShareMount::HoldsRegistration( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return GUCEF_NULL != m_heldSession;
}

/*-------------------------------------------------------------------------*/

UInt32
CRemoteShareMount::GetRegistrationHoldCount( const DRN::CDRNSession* session )
{GUCEF_TRACE;

    return CDrnUtilsGlobal::Instance()->GetShareSessionHolds().GetCount( session );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNUTILS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
