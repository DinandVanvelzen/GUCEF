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

#ifndef GUCEF_DRNUTILS_CREMOTESHAREMOUNTS_H
#define GUCEF_DRNUTILS_CREMOTESHAREMOUNTS_H

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

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_CPULSEGENERATOR_H
#include "gucefCORE_CPulseGenerator.h"
#define GUCEF_CORE_CPULSEGENERATOR_H
#endif /* GUCEF_CORE_CPULSEGENERATOR_H ? */

#ifndef GUCEF_AUTH_ICREDENTIALSTORE_H
#include "gucefAUTH_ICredentialStore.h"
#define GUCEF_AUTH_ICREDENTIALSTORE_H
#endif /* GUCEF_AUTH_ICREDENTIALSTORE_H ? */

#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */

#ifndef GUCEF_DRNUTILS_REMOTESERVICETYPES_H
#include "gucefDRNUTILS_RemoteServiceTypes.h"
#define GUCEF_DRNUTILS_REMOTESERVICETYPES_H
#endif /* GUCEF_DRNUTILS_REMOTESERVICETYPES_H ? */

#ifndef GUCEF_DRNUTILS_CREMOTESERVICECONNECTION_H
#include "gucefDRNUTILS_CRemoteServiceConnection.h"
#define GUCEF_DRNUTILS_CREMOTESERVICECONNECTION_H
#endif /* GUCEF_DRNUTILS_CREMOTESERVICECONNECTION_H ? */

#ifndef GUCEF_DRNUTILS_CREMOTESHAREMOUNT_H
#include "gucefDRNUTILS_CRemoteShareMount.h"
#define GUCEF_DRNUTILS_CREMOTESHAREMOUNT_H
#endif /* GUCEF_DRNUTILS_CREMOTESHAREMOUNT_H ? */

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

/**
 *  The remote shares of an application, from one configuration block: the connections to the services they belong to and a CRemoteShareMount for each
 *  of them. The block (the section "RemoteShares"; the node handed to LoadConfig() is the section) holds
 *
 *    Service   (optional, any number) the connection settings of one service: the attribute "slot" names the service (the slot of its credentials in
 *              the credential store), the rest are the settings of CRemoteServiceConnectionSettings (bulkChannels, clientKind, reconnect times, ...).
 *              A service that no Service child describes connects with the defaults
 *    Share     (any number) one share: "service" (the slot), "share", "mountPoint", "channel" ("auto" or an index) and the child "BridgeSettings",
 *              see CRemoteShareMountSettings
 *
 *  Several shares of one service share one connection (one CRemoteServiceConnection per distinct slot), and the shares of a service on one channel share
 *  its session. The credentials come from the credential store (SetCredentialStore(), not owned) by the slot, as the pairing left them there: this
 *  class does not pair, pairing needs a code from a human. A share whose service has no credentials (not paired) is FAILED with the reason, the other
 *  shares go on. The shares are mounted at Start(), before the connections are up, so they are browsable at once and come online by themselves.
 *
 *  The aggregate state (GetState(), StateChangedEvent) is: IDLE when not started; FAILED when every share is FAILED (or there is none that is not);
 *  ONLINE when every share that is not FAILED is ONLINE; OFFLINE when one of them is OFFLINE; else WAITING. GetEntryState() has the state of each.
 *
 *  Threading: as CRemoteShareMount; the connections live on the notification pulse generator given at construction. Start() and Stop() are for
 *  the thread of the application that drives that pulse generator (or for a time when nobody drives it): Stop() waits (briefly) for the connections to
 *  say goodbye.
 */
class GUCEF_DRNUTILS_PUBLIC_CPP CRemoteShareMounts : public CORE::CObservingNotifier
{
    public:

    static const CORE::CEvent StateChangedEvent;            /**< TRemoteShareStateEventData: the new aggregate state */

    static void RegisterEvents( void );

    /** @param vfs the VFS to mount into, NULL for the global VFS */
    CRemoteShareMounts( const CORE::PulseGeneratorPtr& notificationPulseGenerator, VFS::CVFS* vfs = GUCEF_NULL );

    virtual ~CRemoteShareMounts() GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  Replaces the shares and services by those of the configuration. False, with the reason, when the configuration is not usable (an unknown key, a
     *  Share without a service, a share or a mount point, a mount point twice, a channel that is none, a Service without a slot or one slot twice,
     *  connection settings that cannot be loaded); what was configured before stays then. Refused (false) while started
     */
    bool LoadConfig( const CORE::CDataNode& config, CORE::CString& error );

    /** The Service children that were given and the Share children, in the form LoadConfig() takes */
    bool SaveConfig( CORE::CDataNode& config ) const;

    /** The credential store the credentials of the services are in (by slot); not owned, it must stay until Stop() */
    void SetCredentialStore( AUTH::ICredentialStore* store );

    /**
     *  Makes the connections and the mounts and starts them. False, with the reason, when it is started already or there is no credential store to
     *  ask; a share that cannot be started is FAILED (GetEntryState(), GetEntryError()), which is not a failure of Start()
     */
    bool Start( CORE::CString& error );

    /** Unmounts the shares, removes the registrations they made and closes the connections; safe to call twice */
    void Stop( void );

    bool IsStarted( void ) const;

    UInt32 GetEntryCount( void ) const;
    UInt32 GetServiceCount( void ) const;

    /** The shares by their order in the configuration; IDLE and an empty error for an index that is none */
    TRemoteShareState GetEntryState( UInt32 index ) const;
    CORE::CString GetEntryError( UInt32 index ) const;
    const CRemoteShareMountSettings* GetEntrySettings( UInt32 index ) const;

    /** The mount of an index, NULL before Start() and for an index that is none */
    CRemoteShareMount* GetEntryMount( UInt32 index ) const;

    /** The mount at a mount point (separators at the ends and the kind of separator make no difference), NULL when there is none (or before Start()) */
    CRemoteShareMount* FindByMountPoint( const CORE::CString& mountPoint ) const;

    /** The connection of a service by its slot, NULL when there is none (or before Start()) */
    CRemoteServiceConnection* GetConnection( const CORE::CString& slot ) const;

    TRemoteShareState GetState( void ) const;

    /** How many of the shares are in a state */
    UInt32 GetStateCount( TRemoteShareState state ) const;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    typedef CORE::CTEventHandlerFunctor< CRemoteShareMounts > TEventCallback;

    struct SService
    {
        CORE::CString slot;
        CRemoteServiceConnectionSettings settings;
        bool declared;                              /**< a Service child gave it; else it is there because a share names it, with the defaults */
        CRemoteServiceConnection* connection;

        SService( void );
    };
    typedef GUCEF::vector< SService > TServiceVector;

    struct SEntry
    {
        CRemoteShareMountSettings settings;
        CRemoteShareMount* mount;

        SEntry( void );
    };
    typedef GUCEF::vector< SEntry > TEntryVector;

    CRemoteShareMounts( const CRemoteShareMounts& src );                /**< not implemented */
    CRemoteShareMounts& operator=( const CRemoteShareMounts& src );     /**< not implemented */

    void OnMountState( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );

    TRemoteShareState ComputeStateLocked( void ) const;
    void RaiseIfChanged( void );
    static CORE::CString NormalizeMountPoint( const CORE::CString& mountPoint );

    CORE::PulseGeneratorPtr m_pulse;
    VFS::CVFS* m_vfs;
    AUTH::ICredentialStore* m_store;
    mutable MT::CMutex m_lock;
    TServiceVector m_services;
    TEntryVector m_entries;
    bool m_started;
    TRemoteShareState m_lastRaised;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNUTILS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRNUTILS_CREMOTESHAREMOUNTS_H ? */
