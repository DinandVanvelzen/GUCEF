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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_DRN_CDRNGLOBAL_H
#include "gucefDRN_CDRNGlobal.h"
#define GUCEF_DRN_CDRNGLOBAL_H
#endif /* GUCEF_DRN_CDRNGLOBAL_H ? */

#ifndef GUCEF_DRN_CDRNSESSIONREGISTRY_H
#include "gucefDRN_CDRNSessionRegistry.h"
#define GUCEF_DRN_CDRNSESSIONREGISTRY_H
#endif /* GUCEF_DRN_CDRNSESSIONREGISTRY_H ? */

#ifndef GUCEF_DRN_CDRNNODE_H
#include "gucefDRN_CDRNNode.h"
#define GUCEF_DRN_CDRNNODE_H
#endif /* GUCEF_DRN_CDRNNODE_H ? */

#ifndef GUCEF_DRN_CDRNRESOURCESESSIONCOMPONENT_H
#include "gucefDRN_CDRNResourceSessionComponent.h"
#define GUCEF_DRN_CDRNRESOURCESESSIONCOMPONENT_H
#endif /* GUCEF_DRN_CDRNRESOURCESESSIONCOMPONENT_H ? */

#ifndef GUCEF_DRN_CDRNPEERLINK_H
#include "gucefDRN_CDRNPeerLink.h"
#define GUCEF_DRN_CDRNPEERLINK_H
#endif /* GUCEF_DRN_CDRNPEERLINK_H ? */

#ifndef GUCEF_VFSPLUGIN_DRN_CDRNBRIDGEGLOBAL_H
#include "vfspluginDRN_CDrnBridgeGlobal.h"
#define GUCEF_VFSPLUGIN_DRN_CDRNBRIDGEGLOBAL_H
#endif /* GUCEF_VFSPLUGIN_DRN_CDRNBRIDGEGLOBAL_H ? */

#include "vfspluginDRN_CDrnBridgeConnection.h"

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
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CDrnBridgeConnection::SequencerMemberName = "sequencer";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CDrnBridgeConnection::CDrnBridgeConnection( DRN::CDRNSession& session, UInt32 peerId )
    : CORE::CObserver()
    , m_lock()
    , m_session( &session )
    , m_sessionName( session.GetName() )
    , m_fixedPeerId( peerId )
    , m_member()
    , m_memberDescription( CORE::ToString( peerId ) )
    , m_id()
    , m_link()
    , m_byName( false )
    , m_bindLock()
    , m_handlePools()
    , m_loggedWrongThread( false )
    , m_statCalls( 0 )
    , m_listCalls( 0 )
    , m_openCalls( 0 )
    , m_busyRetries( 0 )
{GUCEF_TRACE;

    Initialize();
}

/*-------------------------------------------------------------------------*/

CDrnBridgeConnection::CDrnBridgeConnection( DRN::CDRNSession& session, const CORE::CString& member )
    : CORE::CObserver()
    , m_lock()
    , m_session( &session )
    , m_sessionName( session.GetName() )
    , m_fixedPeerId( 0 )
    , m_member( member )
    , m_memberDescription( member )
    , m_id()
    , m_link()
    , m_byName( false )
    , m_bindLock()
    , m_handlePools()
    , m_loggedWrongThread( false )
    , m_statCalls( 0 )
    , m_listCalls( 0 )
    , m_openCalls( 0 )
    , m_busyRetries( 0 )
{GUCEF_TRACE;

    Initialize();
}

/*-------------------------------------------------------------------------*/

CDrnBridgeConnection::CDrnBridgeConnection( const CORE::CString& sessionName, const CORE::CString& member )
    : CORE::CObserver()
    , m_lock()
    , m_session( GUCEF_NULL )
    , m_sessionName( sessionName )
    , m_fixedPeerId( 0 )
    , m_member( member )
    , m_memberDescription( member )
    , m_id()
    , m_link()
    , m_byName( true )
    , m_bindLock()
    , m_handlePools()
    , m_loggedWrongThread( false )
    , m_statCalls( 0 )
    , m_listCalls( 0 )
    , m_openCalls( 0 )
    , m_busyRetries( 0 )
{GUCEF_TRACE;

    // the session may exist already: it is taken over now, and again each time the connection is used
    SyncSession();
}

/*-------------------------------------------------------------------------*/

CDrnBridgeConnection::~CDrnBridgeConnection()
{GUCEF_TRACE;

    SignalUpcomingObserverDestruction();
}

/*-------------------------------------------------------------------------*/

void
CDrnBridgeConnection::Initialize( void )
{GUCEF_TRACE;

    // the session tells that it is going away through its destruction event, which every observer gets
    if ( GUCEF_NULL != m_session )
        SubscribeTo( m_session, DRN::CDRNSession::SessionClosedEvent );
}

/*-------------------------------------------------------------------------*/

DRN::CDRNSession*
CDrnBridgeConnection::SyncSession( void ) const
{GUCEF_TRACE;

    // what is registered under the name now; the registry has a lock of its own, so it is asked before any lock of this connection is taken
    DRN::CDRNSession* registered = DRN::CDRNGlobal::Instance()->GetSessionRegistry().Find( m_sessionName );

    MT::CScopeMutex bindLock( m_bindLock );
    CDrnBridgeConnection* self = const_cast< CDrnBridgeConnection* >( this );
    DRN::CDRNSession* left = GUCEF_NULL;
    {
        MT::CScopeMutex lock( m_lock );
        if ( m_session == registered )
            return registered;

        // another session than the one held (none yet, one that is replaced by one of the same name, or none registered any more): the
        // calls follow the session that is there now. The member link belongs to the session that is left. A session that was destroyed
        // told us so already (OnNotify), what is held is then NULL
        left = m_session;
        self->m_session = registered;
        self->m_link.Unlink();
    }
    if ( GUCEF_NULL != left )
        self->UnsubscribeFrom( *left );
    if ( GUCEF_NULL != registered )
        self->SubscribeTo( registered, DRN::CDRNSession::SessionClosedEvent );
    return registered;
}

/*-------------------------------------------------------------------------*/

void
CDrnBridgeConnection::OnNotify( CORE::CNotifier* notifier, const CORE::CEvent& eventid, CORE::CICloneable* eventdata )
{GUCEF_TRACE;

    if ( CORE::CNotifier::DestructionEvent == eventid )
    {
        MT::CScopeMutex lock( m_lock );
        if ( static_cast< CORE::CNotifier* >( m_session ) == notifier )
        {
            m_session = GUCEF_NULL;
            m_link.Unlink();
        }
    }
}

/*-------------------------------------------------------------------------*/

CORE::CString
CDrnBridgeConnection::GetId( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_id;
}

/*-------------------------------------------------------------------------*/

void
CDrnBridgeConnection::SetId( const CORE::CString& id )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_id = id;
}

/*-------------------------------------------------------------------------*/

DRN::CDRNSession*
CDrnBridgeConnection::GetSession( void ) const
{GUCEF_TRACE;

    if ( m_byName )
        return SyncSession();

    MT::CScopeMutex lock( m_lock );
    return m_session;
}

/*-------------------------------------------------------------------------*/

bool
CDrnBridgeConnection::IsBoundByName( void ) const
{GUCEF_TRACE;

    return m_byName;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CDrnBridgeConnection::GetSessionName( void ) const
{GUCEF_TRACE;

    return m_sessionName;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CDrnBridgeConnection::GetMemberDescription( void ) const
{GUCEF_TRACE;

    return m_memberDescription;
}

/*-------------------------------------------------------------------------*/

bool
CDrnBridgeConnection::ResolvePeerIdOfSession( DRN::CDRNSession& session, UInt32& peerId ) const
{GUCEF_TRACE;

    // the node's lock is held by the caller: the members of a session are the node's state
    if ( 0 != m_fixedPeerId )
    {
        peerId = m_fixedPeerId;
        return session.HasMember( peerId );
    }

    DRN::TDRNSessionMemberInfoVector members;
    session.GetMembers( members );
    const bool wantsSequencer = m_member.Equals( SequencerMemberName, false );
    for ( DRN::TDRNSessionMemberInfoVector::const_iterator i = members.begin(); i != members.end(); ++i )
    {
        if ( (*i).isLocal )
            continue;
        if ( wantsSequencer ? (*i).isSequencer : (*i).nodeId == m_member )
        {
            peerId = (*i).peerId;
            return true;
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CDrnBridgeConnection::ResolvePeerId( UInt32& peerId ) const
{GUCEF_TRACE;

    DRN::CDRNSession* session = GetSession();
    if ( GUCEF_NULL == session )
        return false;
    MT::CScopeMutex nodeLock( session->GetNode().GetLock() );
    return session->IsActive() && ResolvePeerIdOfSession( *session, peerId );
}

/*-------------------------------------------------------------------------*/

bool
CDrnBridgeConnection::IsConnected( void ) const
{GUCEF_TRACE;

    DRN::CDRNSession* session = GetSession();
    if ( GUCEF_NULL == session )
        return false;

    MT::CScopeMutex nodeLock( session->GetNode().GetLock() );
    UInt32 peerId = 0;
    if ( !session->IsActive() || !ResolvePeerIdOfSession( *session, peerId ) )
        return false;
    DRN::CDRNPeerLinkPtr link = session->GetLinkToMember( peerId );
    return !link.IsNULL() && link->IsOperational();
}

/*-------------------------------------------------------------------------*/

bool
CDrnBridgeConnection::IsOnNodeThread( void ) const
{GUCEF_TRACE;

    DRN::CDRNSession* session = GetSession();
    return GUCEF_NULL != session && session->GetNode().IsWorkingOnThisThread();
}

/*-------------------------------------------------------------------------*/

CDrnBridgeResult
CDrnBridgeConnection::Unreachable( const CORE::CString& why ) const
{GUCEF_TRACE;

    return CDrnBridgeResult( DRN::DRN_NSRESULT_UNAVAILABLE, DRN::DRN_RESSTATUS_LINK_LOST,
        "the share of member " + m_memberDescription + " of session \"" + m_sessionName + "\" cannot be reached: " + why );
}

/*-------------------------------------------------------------------------*/

CDrnBridgeResult
CDrnBridgeConnection::WrongThread( void ) const
{GUCEF_TRACE;

    bool first = false;
    {
        MT::CScopeMutex lock( m_lock );
        first = !m_loggedWrongThread;
        m_loggedWrongThread = true;
    }
    if ( first )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "DrnBridgeConnection: A blocking call of the archive was made on the thread that works on the node of session \"" << m_sessionName
            << "\", where its answer could never be delivered: the call is refused. Read through the archive from another thread (this is logged once)" );
    }
    return CDrnBridgeResult( DRN::DRN_NSRESULT_WRONG_THREAD, DRN::DRN_RESSTATUS_TIMEOUT,
        "a blocking call on the thread that works on the node of the session would never be answered, call from another thread" );
}

/*-------------------------------------------------------------------------*/

bool
CDrnBridgeConnection::Prepare( DRN::CDRNSession*& session, UInt32& peerId, CDrnBridgeResult& result ) const
{GUCEF_TRACE;

    session = GetSession();
    if ( GUCEF_NULL == session )
    {
        result = Unreachable( m_byName ? "no session \"" + m_sessionName + "\" is registered (yet, or any more)" : CORE::CString( "the session no longer exists" ) );

        return false;
    }
    if ( session->GetNode().IsWorkingOnThisThread() )
    {
        result = WrongThread();
        return false;
    }

    MT::CScopeMutex nodeLock( session->GetNode().GetLock() );
    if ( !session->IsActive() )
    {
        result = Unreachable( "the session is not active" );
        return false;
    }
    if ( !ResolvePeerIdOfSession( *session, peerId ) )
    {
        result = Unreachable( "the member is not in the session" );
        return false;
    }
    DRN::CDRNPeerLinkPtr link = session->GetLinkToMember( peerId );
    if ( link.IsNULL() || !link->IsOperational() )
    {
        result = Unreachable( "there is no operational direct link to the member" );
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

CDrnBridgeMemberLinkPtr
CDrnBridgeConnection::GetMemberLink( void )
{GUCEF_TRACE;

    // a connection by name takes over the session that is registered now first: the link of the session that is gone is dropped by that
    DRN::CDRNSession* session = GetSession();
    {
        MT::CScopeMutex lock( m_lock );
        if ( !m_link.IsNULL() )
            return m_link;
    }
    if ( GUCEF_NULL == session )
        return CDrnBridgeMemberLinkPtr();

    // outside the lock: the global has locks of its own
    CDrnBridgeMemberLinkPtr link = CDrnBridgeGlobal::Instance()->AcquireMemberLink( *session, m_memberDescription );
    MT::CScopeMutex lock( m_lock );
    if ( m_link.IsNULL() && m_session == session )
    {
        m_link = link;

        // the pools that use this connection take part in the division of the handle limit of the member of the session that is here now
        for ( THandlePoolVector::const_iterator i = m_handlePools.begin(); i != m_handlePools.end(); ++i )
            m_link->RegisterHandleUser( (*i) );
    }
    return m_link;
}

/*-------------------------------------------------------------------------*/

CDrnBridgeConnection::TClientPtr
CDrnBridgeConnection::ObtainClient( DRN::CDRNSession& session, UInt32 peerId, UInt32 timeoutInMs )
{GUCEF_TRACE;

    CDrnBridgeMemberLinkPtr link = GetMemberLink();
    if ( link.IsNULL() )
        return TClientPtr();
    return link->ObtainClient( session, peerId, timeoutInMs );
}

/*-------------------------------------------------------------------------*/

bool
CDrnBridgeConnection::IsBusy( const CDrnBridgeResult& result )
{GUCEF_TRACE;

    return DRN::DRN_NSRESULT_BUSY == result.code;
}

/*-------------------------------------------------------------------------*/

namespace {

/** A condition that is never met: the wait of a pump thread that only wants the node worked for a while */
class CNeverMetCondition : public DRN::CIDRNWaitCondition
{
    public:

    virtual bool IsMet( void ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        return false;
    }
};

}

void
CDrnBridgeConnection::BackOff( UInt32 attempt, UInt32 backoffMs )
{GUCEF_TRACE;

    // every retry waits twice as long as the one before, at most two seconds
    UInt64 wait = backoffMs;
    for ( UInt32 i=0; i<attempt && wait < 2000; ++i )
        wait *= 2;
    if ( wait > 2000 )
        wait = 2000;
    if ( 0 == wait )
        return;

    DRN::CDRNSession* session = GetSession();
    if ( GUCEF_NULL != session && session->GetNode().IsPumpThread() )
    {
        // the node is worked on this thread: it must not stand still while the serving side gets time to give room
        CNeverMetCondition never;
        session->GetNode().PumpUntil( never, (UInt32) wait );
    }
    else
    {
        MT::ThreadDelay( (UInt32) wait );
    }
}

/*-------------------------------------------------------------------------*/

CDrnBridgeResult
CDrnBridgeConnection::Stat( const CORE::CString& share, const CORE::CString::StringVector& paths, const CORE::CString::StringVector& knownRevisions,
                            DRN::CDRNNamespaceClient::TEntryVector& results, UInt32 timeoutInMs, UInt32 busyRetries, UInt32 busyBackoffMs )
{GUCEF_TRACE;

    DRN::CDRNSession* session = GUCEF_NULL;
    UInt32 peerId = 0;
    CDrnBridgeResult result;
    if ( !Prepare( session, peerId, result ) )
        return result;

    m_statCalls.FetchAdd( 1 );
    for ( UInt32 attempt=0; ; ++attempt )
    {
        TClientPtr client = ObtainClient( *session, peerId, timeoutInMs );
        if ( client.IsNULL() )
            return Unreachable( "the session no longer exists" );
        result = CDrnBridgeResult::FromNamespaceResult( client->Stat( share, paths, knownRevisions, results, timeoutInMs ) );
        if ( !IsBusy( result ) || attempt >= busyRetries )
            return result;
        m_busyRetries.FetchAdd( 1 );
        BackOff( attempt, busyBackoffMs );
    }
}

/*-------------------------------------------------------------------------*/

CDrnBridgeResult
CDrnBridgeConnection::List( const CORE::CString& share, const CORE::CString& dir, const CORE::CString& cursor, UInt32 maxEntries,
                            const CORE::CString& ifRevisionNot, DRN::CDRNNamespaceListPage& page, UInt32 timeoutInMs, UInt32 busyRetries, UInt32 busyBackoffMs )
{GUCEF_TRACE;

    DRN::CDRNSession* session = GUCEF_NULL;
    UInt32 peerId = 0;
    CDrnBridgeResult result;
    if ( !Prepare( session, peerId, result ) )
        return result;

    m_listCalls.FetchAdd( 1 );
    for ( UInt32 attempt=0; ; ++attempt )
    {
        TClientPtr client = ObtainClient( *session, peerId, timeoutInMs );
        if ( client.IsNULL() )
            return Unreachable( "the session no longer exists" );
        result = CDrnBridgeResult::FromNamespaceResult( client->List( share, dir, cursor, maxEntries, ifRevisionNot, page, timeoutInMs ) );
        if ( !IsBusy( result ) || attempt >= busyRetries )
            return result;
        m_busyRetries.FetchAdd( 1 );
        BackOff( attempt, busyBackoffMs );
    }
}

/*-------------------------------------------------------------------------*/

CDrnBridgeResult
CDrnBridgeConnection::ListAll( const CORE::CString& share, const CORE::CString& dir, DRN::CDRNNamespaceClient::TEntryVector& entries, CORE::CString& dirRevision,
                               bool& changedDuringListing, UInt32 maxTotalEntries, UInt32 pageEntries, UInt32 timeoutInMs, UInt32 busyRetries, UInt32 busyBackoffMs )
{GUCEF_TRACE;

    DRN::CDRNSession* session = GUCEF_NULL;
    UInt32 peerId = 0;
    CDrnBridgeResult result;
    if ( !Prepare( session, peerId, result ) )
        return result;

    m_listCalls.FetchAdd( 1 );
    for ( UInt32 attempt=0; ; ++attempt )
    {
        TClientPtr client = ObtainClient( *session, peerId, timeoutInMs );
        if ( client.IsNULL() )
            return Unreachable( "the session no longer exists" );
        result = CDrnBridgeResult::FromNamespaceResult( client->ListAll( share, dir, entries, dirRevision, changedDuringListing, maxTotalEntries, pageEntries, 2, timeoutInMs ) );
        if ( !IsBusy( result ) || attempt >= busyRetries )
            return result;
        m_busyRetries.FetchAdd( 1 );
        BackOff( attempt, busyBackoffMs );
    }
}

/*-------------------------------------------------------------------------*/

CDrnBridgeResult
CDrnBridgeConnection::OpenHandle( const CORE::CString& share, const CORE::CString& path, const DRN::CDRNResourceTransferOptions& options,
                                  UInt32 timeoutInMs, DRN::CDRNResourceHandlePtr& handle, UInt32 busyRetries, UInt32 busyBackoffMs )
{GUCEF_TRACE;

    handle.Unlink();

    DRN::CDRNSession* session = GUCEF_NULL;
    UInt32 peerId = 0;
    CDrnBridgeResult result;
    if ( !Prepare( session, peerId, result ) )
        return result;

    m_openCalls.FetchAdd( 1 );
    for ( UInt32 attempt=0; ; ++attempt )
    {
        DRN::CDRNResourceHandlePtr opened = session->GetResources().OpenHandle( peerId, share, path, options );
        if ( opened.IsNULL() )
            return Unreachable( "the handle could not be made" );
        if ( opened->WaitOpen( timeoutInMs ) )
        {
            handle = opened;
            return CDrnBridgeResult();
        }
        const DRN::CDRNResourceTransferResult failure = opened->GetResult();
        opened->Close();
        result = CDrnBridgeResult::FromTransferResult( failure );
        if ( !IsBusy( result ) || attempt >= busyRetries )
            return result;
        m_busyRetries.FetchAdd( 1 );
        BackOff( attempt, busyBackoffMs );
    }
}

/*-------------------------------------------------------------------------*/

void
CDrnBridgeConnection::RegisterHandleUser( const CDrnBridgeHandlePool* user )
{GUCEF_TRACE;

    {
        MT::CScopeMutex lock( m_lock );
        m_handlePools.push_back( user );
    }
    CDrnBridgeMemberLinkPtr link = GetMemberLink();
    if ( !link.IsNULL() )
        link->RegisterHandleUser( user );
}

/*-------------------------------------------------------------------------*/

void
CDrnBridgeConnection::UnregisterHandleUser( const CDrnBridgeHandlePool* user )
{GUCEF_TRACE;

    // the link is let go of outside the lock, with the last pool: it is gone with the last of the mounts
    CDrnBridgeMemberLinkPtr link;
    CDrnBridgeMemberLinkPtr released;
    {
        MT::CScopeMutex lock( m_lock );
        link = m_link;
        for ( THandlePoolVector::iterator i = m_handlePools.begin(); i != m_handlePools.end(); ++i )
        {
            if ( (*i) == user )
            {
                m_handlePools.erase( i );
                break;
            }
        }
        if ( m_handlePools.empty() )
        {
            released = m_link;
            m_link.Unlink();
        }
    }
    if ( !link.IsNULL() )
        link->UnregisterHandleUser( user );
}

/*-------------------------------------------------------------------------*/

UInt32
CDrnBridgeConnection::GetHandleShare( const CDrnBridgeHandlePool* user, UInt32 timeoutInMs )
{GUCEF_TRACE;

    CDrnBridgeMemberLinkPtr link = GetMemberLink();
    if ( link.IsNULL() )
        return 0;
    if ( !link->HasNegotiated() )
    {
        // the member is asked once what it allows: a share of that is what a pool may hold. When it cannot be asked (no link
        // yet), the pool goes by its own setting for now
        DRN::CDRNSession* session = GUCEF_NULL;
        UInt32 peerId = 0;
        CDrnBridgeResult result;
        if ( Prepare( session, peerId, result ) )
        {
            TClientPtr client = link->ObtainClient( *session, peerId, timeoutInMs );
            DRN::CDRNNamespaceInfo info;
            client->Negotiate( info, timeoutInMs );
        }
    }
    return link->GetHandleShare( user );
}

/*-------------------------------------------------------------------------*/

UInt64
CDrnBridgeConnection::GetStatCallCount( void ) const
{GUCEF_TRACE;

    return m_statCalls.Load();
}

/*-------------------------------------------------------------------------*/

UInt64
CDrnBridgeConnection::GetListCallCount( void ) const
{GUCEF_TRACE;

    return m_listCalls.Load();
}

/*-------------------------------------------------------------------------*/

UInt64
CDrnBridgeConnection::GetOpenCallCount( void ) const
{GUCEF_TRACE;

    return m_openCalls.Load();
}

/*-------------------------------------------------------------------------*/

UInt64
CDrnBridgeConnection::GetBusyRetryCount( void ) const
{GUCEF_TRACE;

    return m_busyRetries.Load();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNBRIDGE */
}; /* namespace VFSPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
