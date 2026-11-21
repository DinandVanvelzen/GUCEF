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

#ifndef GUCEF_DRNUTILS_CREMOTESHAREMOUNT_H
#define GUCEF_DRNUTILS_CREMOTESHAREMOUNT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_COBSERVINGNOTIFIER_H
#include "CObservingNotifier.h"
#define GUCEF_CORE_COBSERVINGNOTIFIER_H
#endif /* GUCEF_CORE_COBSERVINGNOTIFIER_H ? */

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

#ifndef GUCEF_CORE_CTCLONEABLEOBJ_H
#include "CTCloneableObj.h"
#define GUCEF_CORE_CTCLONEABLEOBJ_H
#endif /* GUCEF_CORE_CTCLONEABLEOBJ_H ? */

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */

#ifndef GUCEF_DRN_CDRNSESSION_H
#include "gucefDRN_CDRNSession.h"
#define GUCEF_DRN_CDRNSESSION_H
#endif /* GUCEF_DRN_CDRNSESSION_H ? */

#ifndef GUCEF_DRNUTILS_REMOTESERVICETYPES_H
#include "gucefDRNUTILS_RemoteServiceTypes.h"
#define GUCEF_DRNUTILS_REMOTESERVICETYPES_H
#endif /* GUCEF_DRNUTILS_REMOTESERVICETYPES_H ? */

#ifndef GUCEF_DRNUTILS_CREMOTESERVICECONNECTION_H
#include "gucefDRNUTILS_CRemoteServiceConnection.h"
#define GUCEF_DRNUTILS_CREMOTESERVICECONNECTION_H
#endif /* GUCEF_DRNUTILS_CREMOTESERVICECONNECTION_H ? */

#ifndef GUCEF_DRNUTILS_CREMOTESHAREMOUNTSETTINGS_H
#include "gucefDRNUTILS_CRemoteShareMountSettings.h"
#define GUCEF_DRNUTILS_CREMOTESHAREMOUNTSETTINGS_H
#endif /* GUCEF_DRNUTILS_CREMOTESHAREMOUNTSETTINGS_H ? */

#ifndef GUCEF_DRNUTILS_MACROS_H
#include "gucefDRNUTILS_macros.h"
#define GUCEF_DRNUTILS_MACROS_H
#endif /* GUCEF_DRNUTILS_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRNUTILS {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

typedef CORE::CTCloneableObj< TRemoteShareState > TRemoteShareStateEventData;

/**
 *  One share of a remote service, mounted into the VFS as an archive of type "drn" (the VFS plugin for DRN, which this module does not link:
 *  the VFS makes the archive from its type name, so the plugin has to be loaded, or linked and its global made, by the application).
 *  It binds one CRemoteServiceConnection (not owned: it must outlive the mount, and the mount is stopped first) to the share:
 *
 *    - the DRN session of the channel that carries the files (CRemoteShareMountSettings::channel) is made known by name in the DRN session
 *      registry (CDRNGlobal::GetSessionRegistry()) when it exists, under the DRN session name of the credentials, and the registration is
 *      moved to the new session when the connection makes one (Close() and StartConnect() again, a connection that gave up and starts over);
 *      a registration is removed only when this mount made it. Every channel of a connection joins the one session name, so one channel's session
 *      holds the name: a mount of another channel of the same connection, or any other session of that name, is a collision and is refused
 *      (FAILED, the mount is taken away again: it would serve the share of the other session)
 *    - the share is mounted at Start() with the address drn://<session name>/sequencer/<share>, before the connection is up: the mount is
 *      there at once, answers that the share cannot be reached until the session is, and comes online by itself (late session binding of the
 *      plugin). It stays mounted over drops, reconnects and new sessions
 *
 *  States (StateChangedEvent, GetState()): IDLE (not started), WAITING (started, never online since), ONLINE (the connection is connected, and for
 *  a bulk channel the channel is attached), OFFLINE (was online, the link is lost: the mount serves what it has cached), FAILED (the mount was
 *  refused or taken away, the name is taken, the connection gave up, SetFailed()): GetLastError() has the reason. The state follows the events
 *  of the connection (its StateChangedEvent, and its ChannelChangedEvent for the bulk channels), nothing is polled.
 *
 *  Several mounts of one service, on one channel, share the one session: the first of them registers it, the others find it registered and hold a
 *  share of it (a count per session); the registration is removed by the last of them to let go (Stop(), or a new session of the connection), so
 *  stopping one mount leaves the others online. A session the application registered is never counted or removed.
 *  CRemoteShareMounts (a configuration with several shares) keeps them together.
 *
 *  Threading: the connection lives on its notification pulse generator and its events arrive there; the state changes and the events of the
 *  mount are made there, or on the thread that calls Start() or Stop(). The mount never waits for the connection: Start() returns at once
 *  (the VFS calls it makes do not block on the network).
 */
class GUCEF_DRNUTILS_PUBLIC_CPP CRemoteShareMount : public CORE::CObservingNotifier
{
    public:

    static const CORE::CEvent StateChangedEvent;            /**< TRemoteShareStateEventData: the new state */

    static void RegisterEvents( void );

    /** The archive type that is mounted: "drn" */
    static const char* const ArchiveTypeName;

    /** The member of the session that serves the share: the host of the client-server session, which is the service */
    static const char* const MemberName;

    /** @param vfs the VFS to mount into, NULL for the global VFS (VFS::CVfsGlobal) */
    CRemoteShareMount( CRemoteServiceConnection& connection, const CRemoteShareMountSettings& settings, VFS::CVFS* vfs = GUCEF_NULL );

    /** Stops: unmounts and removes the registration it made */
    virtual ~CRemoteShareMount() GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  Mounts the share and starts following the connection. 'drnSessionName' is the name of the DRN session of the service, which is the
     *  drnSessionName of the credentials of the connection (CDeviceCredentials); the second form takes it from the credentials the connection
     *  has (StartConnect() or Connect() was called). False, with the reason (also in GetLastError(), and the state is FAILED), when it is
     *  started already, the settings or the channel are not usable, the session name is registered for another session, the VFS does not know the
     *  archive type, the mount point or the archive name is in use, or the VFS refuses the mount
     */
    bool Start( const CORE::CString& drnSessionName, CORE::CString& error );
    bool Start( CORE::CString& error );

    /** Unmounts the share (only the archive this mount made) and removes the registration it made (only that one); the state is IDLE. Safe to call twice */
    void Stop( void );

    /** The owner knows the mount cannot work (no credentials, ...): the state is FAILED with the reason, until Stop() */
    void SetFailed( const CORE::CString& reason );

    TRemoteShareState GetState( void ) const;
    CORE::CString GetLastError( void ) const;

    const CRemoteShareMountSettings& GetSettings( void ) const;

    bool IsStarted( void ) const;

    /** Whether the archive this mount made is in the VFS */
    bool IsMounted( void ) const;

    /** The channel that carries the files: the setting, with "auto" resolved at Start(); AutoChannel before */
    UInt32 GetChannel( void ) const;

    /** The DRN session name the share is mounted by, empty before Start() */
    CORE::CString GetSessionName( void ) const;

    CORE::CString GetAddress( void ) const;
    CORE::CString GetArchiveName( void ) const;

    /** Whether this mount made the registration of the session now in the DRN session registry (the registration goes with the last mount that holds it) */
    bool HasRegisteredSession( void ) const;

    /** Whether this mount holds a share of a registration that a mount of this class made (made by it, or found made by another mount of this class) */
    bool HoldsRegistration( void ) const;

    /** The number of mounts that hold the registration of the session (0 for a session that no mount of this class registered, as one the application registered) */
    static UInt32 GetRegistrationHoldCount( const DRN::CDRNSession* session );

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual void OnNotify( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    typedef CORE::CTEventHandlerFunctor< CRemoteShareMount > TEventCallback;

    CRemoteShareMount( const CRemoteShareMount& src );                  /**< not implemented */
    CRemoteShareMount& operator=( const CRemoteShareMount& src );       /**< not implemented */

    void OnConnectionEvent( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );

    bool StartImp( const CORE::CString& drnSessionName, CORE::CString& error );

    /** Brings the registration, and so the state, in line with the connection; with the lock. True when the state changed */
    bool Follow( void );

    void ReleaseRegistration( void );
    void Unmount( void );
    bool SetStateLocked( TRemoteShareState state, const CORE::CString& error );
    void Raise( bool changed, TRemoteShareState state );

    VFS::CVFS& GetVfs( void ) const;

    mutable MT::CMutex m_lock;
    CRemoteServiceConnection* m_connection;
    CRemoteShareMountSettings m_settings;
    VFS::CVFS* m_vfs;
    bool m_started;
    bool m_failedSticky;
    bool m_wasOnline;
    TRemoteShareState m_state;
    CORE::CString m_lastError;
    UInt32 m_channel;
    CORE::CString m_sessionName;
    CORE::CString m_address;
    CORE::CString m_archiveName;
    bool m_mounted;
    const VFS::CArchive* m_archive;                 /**< the identity of the archive this mount made, never used to call */
    DRN::CDRNSession* m_registeredSession;          /**< the session this mount registered, NULL when it did not; compared, never called, until the registry says it is still there */
    DRN::CDRNSession* m_heldSession;                /**< the session whose registration this mount holds a share of (the one it made, or one made by another mount of this class); same rule */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNUTILS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRNUTILS_CREMOTESHAREMOUNT_H ? */
