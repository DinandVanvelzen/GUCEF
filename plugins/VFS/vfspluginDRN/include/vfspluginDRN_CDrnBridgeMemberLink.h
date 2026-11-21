/*
 *  vfspluginDRN: GUCEF VFS plugin presenting a share of another DRN node as a VFS archive
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

#ifndef GUCEF_VFSPLUGIN_DRN_CDRNBRIDGEMEMBERLINK_H
#define GUCEF_VFSPLUGIN_DRN_CDRNBRIDGEMEMBERLINK_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_COBSERVER_H
#include "CObserver.h"
#define GUCEF_CORE_COBSERVER_H
#endif /* GUCEF_CORE_COBSERVER_H ? */

#ifndef GUCEF_CORE_CTSHAREDPTR_H
#include "CTSharedPtr.h"
#define GUCEF_CORE_CTSHAREDPTR_H
#endif /* GUCEF_CORE_CTSHAREDPTR_H ? */

#ifndef GUCEF_DRN_CDRNSESSION_H
#include "gucefDRN_CDRNSession.h"
#define GUCEF_DRN_CDRNSESSION_H
#endif /* GUCEF_DRN_CDRNSESSION_H ? */

#ifndef GUCEF_DRN_CDRNNAMESPACECLIENT_H
#include "gucefDRN_CDRNNamespaceClient.h"
#define GUCEF_DRN_CDRNNAMESPACECLIENT_H
#endif /* GUCEF_DRN_CDRNNAMESPACECLIENT_H ? */

#ifndef GUCEF_VFSPLUGIN_DRN_MACROS_H
#include "vfspluginDRN_macros.h"
#define GUCEF_VFSPLUGIN_DRN_MACROS_H
#endif /* GUCEF_VFSPLUGIN_DRN_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace VFSPLUGIN {
namespace DRNBRIDGE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CDrnBridgeHandlePool;

/**
 *  What every mount of one (session, member) shares: the namespace client with its request budget (one FIFO gate for
 *  all mounts, so a heavy mount cannot starve a light one) and the handle budget of the member.
 *
 *  The global (CDrnBridgeGlobal::AcquireMemberLink()) hands out one link per (session, member) and keeps only a weak
 *  reference to it: it is gone when the last mount lets go of it, and made again for the next one.
 *
 *  Handle budget. A serving member announces how many resources one member may hold open at once (the lower of its
 *  server-wide and its per-member limit, 0 = no maximum or unknown). The handle pools of the mounts (one per mount) that
 *  use the link divide that number between them, in the order they registered, each at least 1: with a cap of 8 and three
 *  mounts the pools may hold 3, 3 and 2. A pool that has no room left closes its own least recently used idle handle first
 *  (that is the pool's existing rule); it fails with the retry-later result only when it has none to close. The shares
 *  follow the pools that exist, so a mount that goes gives its share to the others. A member the bridge could not ask
 *  (no announcement) leaves every pool to its own setting.
 *
 *  The link follows the session: when the session is destroyed the client is dropped.
 */
class GUCEF_VFSPLUGIN_DRN_EXPORT_CPP CDrnBridgeMemberLink : public CORE::CObserver
{
    public:

    typedef CORE::CTSharedPtr< DRN::CDRNNamespaceClient, MT::CMutex > TClientPtr;

    /** 'memberKey': the member as the mounts name it (a node id, "sequencer" or a peer id as text) */
    CDrnBridgeMemberLink( DRN::CDRNSession& session, const CORE::CString& memberKey );

    virtual ~CDrnBridgeMemberLink() GUCEF_VIRTUAL_OVERRIDE;

    /** The session, NULL once it was destroyed */
    DRN::CDRNSession* GetSession( void ) const;

    const CORE::CString& GetMemberKey( void ) const;

    /** The one client of the member as peer 'peerId' of the session: made when there is none or the member is another peer now */
    TClientPtr ObtainClient( DRN::CDRNSession& session, UInt32 peerId, UInt32 timeoutInMs );

    /** The client now, NULL when there is none yet; for diagnostics */
    TClientPtr GetClient( void ) const;

    /** Whether the member answered a hello to the client of now (what it announced is known, which may be nothing) */
    bool HasNegotiated( void ) const;

    /** Whether it announced limits in that answer */
    bool HasAnnouncedLimits( void ) const;

    /** Clients made since the link was made: 1 for as long as the member keeps its peer id */
    UInt32 GetClientsMade( void ) const;

    // the handle budget

    /** A handle pool that uses the link takes part in the division of the member's handle limit */
    void RegisterHandleUser( const CDrnBridgeHandlePool* user );

    void UnregisterHandleUser( const CDrnBridgeHandlePool* user );

    UInt32 GetHandleUserCount( void ) const;

    /**
     *  The handles 'user' may hold open at once: its share of the announced limit of the member; 0 when the member announced
     *  no limit (or none is known): the pool then uses its own setting
     */
    UInt32 GetHandleShare( const CDrnBridgeHandlePool* user ) const;

    protected:

    virtual void OnNotify( CORE::CNotifier* notifier, const CORE::CEvent& eventid, CORE::CICloneable* eventdata = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    typedef GUCEF::vector< const CDrnBridgeHandlePool* > TUserVector;

    CDrnBridgeMemberLink( const CDrnBridgeMemberLink& src );                /**< not implemented */
    CDrnBridgeMemberLink& operator=( const CDrnBridgeMemberLink& src );     /**< not implemented */

    mutable MT::CMutex m_lock;
    DRN::CDRNSession* m_session;
    CORE::CString m_memberKey;
    TClientPtr m_client;
    UInt32 m_clientPeerId;
    UInt32 m_clientsMade;
    TUserVector m_users;
};

typedef CORE::CTSharedPtr< CDrnBridgeMemberLink, MT::CMutex > CDrnBridgeMemberLinkPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNBRIDGE */
}; /* namespace VFSPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_VFSPLUGIN_DRN_CDRNBRIDGEMEMBERLINK_H ? */
