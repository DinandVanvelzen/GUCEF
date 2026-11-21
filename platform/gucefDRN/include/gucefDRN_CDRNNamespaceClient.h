/*
 *  gucefDRN: GUCEF module providing RAD networking trough data replication
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

#ifndef GUCEF_DRN_CDRNNAMESPACECLIENT_H
#define GUCEF_DRN_CDRNNAMESPACECLIENT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CFUTURERESULT_H
#include "gucefCORE_CFutureResult.h"
#define GUCEF_CORE_CFUTURERESULT_H
#endif /* GUCEF_CORE_CFUTURERESULT_H ? */

#ifndef GUCEF_DRN_CDRNNAMESPACERESPONSE_H
#include "gucefDRN_CDRNNamespaceResponse.h"
#define GUCEF_DRN_CDRNNAMESPACERESPONSE_H
#endif /* GUCEF_DRN_CDRNNAMESPACERESPONSE_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRN {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CDRNSession;

/**
 *  Browsing the shares of one member of a DRN session: what shares it offers (SHARES), what files and directories they
 *  hold (STAT, LIST), with sizes, times, revisions and, when the member knows them, digests. The bytes of a file are read
 *  through CDRNResourceSessionComponent::OpenHandle() with the share name as the provider name and the path as the
 *  resource name; the revision of an entry is the one such an open checks (CDRNResourceTransferOptions::expectedRevision).
 *
 *  Bound to a session and the peer id of the serving member, nothing more: the client does not discover, connect or
 *  authenticate, the application's session does that. It needs a direct, operational link to the member whose node speaks
 *  protocol 2.3 and that the settings allow (protected, unless allowUnprotectedLinks); anything else is UNSUPPORTED or
 *  UNAVAILABLE.
 *
 *  Threading. The blocking calls do their work and their waiting on the calling thread. A call from inside work of the
 *  node (a handler the node delivers, on the node's own thread or on the thread that pumps a node without one) is
 *  refused at once with DRN_NSRESULT_WRONG_THREAD, because the answer could never be delivered there. A call from the
 *  thread that pumps a node without a thread of its own (CDRNNode::IsPumpThread()), outside work of the node, works the
 *  node on that thread while it waits (CDRNNode::PumpUntil()), so it is answered, or ends at its timeout. They never
 *  pump the application's pulse generator. The asynchronous calls (...Async) queue the same call as a task on the pool of the resource transfer
 *  settings and return a CORE::CFutureResult; what they deliver is in the CDRNNamespaceResponse they hand out, to be read
 *  once the future is ready. All calls may be made from several threads at once. The session (its node) has to outlive
 *  the client and the calls under way; stopping the node ends calls under way with UNAVAILABLE.
 *
 *  Limits. The first call asks the member for its limits (Negotiate(), also callable on its own); a call that goes beyond
 *  them is shrunk to them: Stat() splits a long list of paths into batches, List() asks no more entries than a page may
 *  hold. A share with stricter limits than the member's refuses with TOO_LARGE and the call retries once with smaller
 *  numbers. Every blocking call takes a timeout in milliseconds (0 = the default of SetDefaultTimeoutMs()).
 *
 *  Budget. The member also announces how many STAT and LIST requests it works on at once for this member and how many
 *  listings it keeps (the lower of its server-wide and its per-member limit: caps, not what is free at the moment). The
 *  client keeps to them: never more requests under way than announced, never more ListAll() listings under way than
 *  announced; a call that finds the budget used up waits for its turn, first come first served, and gives up with
 *  TIMEOUT when its timeout (which includes the wait) is over. A member that announces nothing, or announces 0, is not
 *  throttled. Several threads, or several users of one client object, share the budget; two client objects of the same
 *  member do not, and neither does anything else that sends to the member (another application, or the single-page List(),
 *  whose listing is only ended by the caller reading its last page). A refusal that still happens (others use the
 *  member's server-wide budget, or limits changed since) comes back as BUSY, for the caller to retry.
 *
 *  Revisions. 'knownRevisions' of Stat() and 'ifRevisionNot' of List() make the member answer UNCHANGED (no body) when the
 *  revision the caller holds is still the current one; Stat() then marks that path's entry with status
 *  DRN_RESSTATUS_UNCHANGED, List() gives a page with 'unchanged' set.
 */
class GUCEF_DRN_EXPORT_CPP CDRNNamespaceClient
{
    public:

    typedef GUCEF::vector< CDRNNamespaceEntry > TEntryVector;

    /**
     *  The task manager pool the asynchronous calls run on unless SetThreadPoolName() says otherwise. A call holds its pool
     *  thread while it waits for the member's answer, so it has a pool of its own (made with four worker threads the
     *  first time): on the pool of the resource transfer settings, which the serving side's provider work uses as well, a
     *  process that serves and browses at once could wait for itself.
     */
    static const CORE::CString DefaultThreadPoolName;

    /** The peer id is the one of the serving member in the session (the host of a client-server session, any member of a mesh) */
    CDRNNamespaceClient( CDRNSession& session, UInt32 peerId );

    ~CDRNNamespaceClient();

    UInt32 GetPeerId( void ) const;

    /** The pool of the asynchronous calls, DefaultThreadPoolName until set; a pool of the application's own must have a thread for each call that can wait at once */
    void SetThreadPoolName( const CORE::CString& threadPoolName );
    CORE::CString GetThreadPoolName( void ) const;

    /** The timeout of a call that names none, default 15000 */
    void SetDefaultTimeoutMs( UInt32 timeoutInMs );
    UInt32 GetDefaultTimeoutMs( void ) const;

    // negotiation

    /** Asks the member for the version and limits (NS_HELLO) and keeps them for the calls that follow */
    CDRNNamespaceResult Negotiate( CDRNNamespaceInfo& info, UInt32 timeoutInMs = 0 );

    /** What the last negotiation gave; false when none happened yet */
    bool GetNegotiatedInfo( CDRNNamespaceInfo& info ) const;

    /**
     *  The limits the member announced for this client's member (the answer to the last negotiation), 0 when there is no
     *  maximum, when the member did not announce them or when none happened yet. They are caps, not free capacity.
     */
    UInt32 GetRequestLimit( void ) const;           /**< STAT and LIST requests under way at once, kept to by the client */
    UInt32 GetListingLimit( void ) const;           /**< listings under way at once, kept to by ListAll() */
    UInt32 GetOpenHandleLimit( void ) const;        /**< resources that may be held open (for the one who opens: CDRNResourceSessionComponent::OpenHandle()) */
    UInt32 GetActiveReadLimit( void ) const;        /**< reads served at once */
    UInt32 GetUploadRateLimit( void ) const;        /**< bytes per second the member sends to this one at most */

    /**
     *  Keeping to the announced request and listing limits is on by default. Without it the calls go out as they are made and
     *  the member's BUSY refusals are what limits them
     */
    void SetSelfThrottle( bool enabled );
    bool GetSelfThrottle( void ) const;

    /** Requests (STAT, LIST) of this client under way, waiting for their turn, and the most that were under way at once */
    UInt32 GetRequestsInFlight( void ) const;
    UInt32 GetRequestsWaiting( void ) const;
    UInt32 GetPeakRequestsInFlight( void ) const;

    /** Listings (ListAll) of this client under way and waiting for their turn */
    UInt32 GetListingsInFlight( void ) const;
    UInt32 GetListingsWaiting( void ) const;
    UInt32 GetPeakListingsInFlight( void ) const;

    // blocking

    /** The shares the member lets this link's member see (by role and policy), with their capabilities and limits */
    CDRNNamespaceResult Shares( GUCEF::vector< CDRNNamespaceShareInfo >& shares, UInt32 timeoutInMs = 0 );

    /**
     *  Describes paths of a share ("" is the root): results[ i ] is the entry of paths[ i ] with its own status (OK,
     *  NOT_FOUND, FORBIDDEN, INVALID_NAME, UNCHANGED, ...); the call itself is OK when the member answered. 'knownRevisions'
     *  is empty, or holds the revision the caller has of each path ("" for none). Never reads or hashes a file on the member.
     */
    CDRNNamespaceResult Stat( const CORE::CString& share, const CORE::CString::StringVector& paths, const CORE::CString::StringVector& knownRevisions,
                              TEntryVector& results, UInt32 timeoutInMs = 0 );

    /**
     *  One page of a directory ("" is the root of the share): the first with an empty cursor, the next with the cursor of
     *  the page before. 'maxEntries' 0 is a page of the member's default size. With 'ifRevisionNot' set to the revision the
     *  caller holds of the directory the page is 'unchanged' (and the result UNCHANGED) when it still is that one. A cursor the
     *  member forgot answers CURSOR_EXPIRED: list again from the start.
     */
    CDRNNamespaceResult List( const CORE::CString& share, const CORE::CString& dir, const CORE::CString& cursor, UInt32 maxEntries,
                              const CORE::CString& ifRevisionNot, CDRNNamespaceListPage& page, UInt32 timeoutInMs = 0 );

    /**
     *  Every entry of a directory by following the cursors, at most 'maxTotalEntries' of them (TOO_LARGE, with the entries
     *  read so far, when there are more). A listing that expired is started over, at most 'maxRestarts' times; a directory
     *  whose revision changed between pages is reported in 'changedDuringListing' (pages are not a snapshot). 'dirRevision'
     *  is the revision of the directory at its first page. The timeout is per page.
     */
    CDRNNamespaceResult ListAll( const CORE::CString& share, const CORE::CString& dir, TEntryVector& entries, CORE::CString& dirRevision,
                                 bool& changedDuringListing, UInt32 maxTotalEntries = 1000000, UInt32 pageEntries = 0, UInt32 maxRestarts = 2,
                                 UInt32 timeoutInMs = 0 );

    // asynchronous

    CORE::CFutureResult NegotiateAsync( CDRNNamespaceResponsePtr& response, UInt32 timeoutInMs = 0 );

    CORE::CFutureResult SharesAsync( CDRNNamespaceResponsePtr& response, UInt32 timeoutInMs = 0 );

    CORE::CFutureResult StatAsync( const CORE::CString& share, const CORE::CString::StringVector& paths, const CORE::CString::StringVector& knownRevisions,
                                   CDRNNamespaceResponsePtr& response, UInt32 timeoutInMs = 0 );

    CORE::CFutureResult ListAsync( const CORE::CString& share, const CORE::CString& dir, const CORE::CString& cursor, UInt32 maxEntries,
                                   const CORE::CString& ifRevisionNot, CDRNNamespaceResponsePtr& response, UInt32 timeoutInMs = 0 );

    CORE::CFutureResult ListAllAsync( const CORE::CString& share, const CORE::CString& dir, CDRNNamespaceResponsePtr& response,
                                      UInt32 maxTotalEntries = 1000000, UInt32 pageEntries = 0, UInt32 maxRestarts = 2, UInt32 timeoutInMs = 0 );

    private:

    class CState;
    class CCallJob;

    typedef CORE::CTSharedPtr< CState, MT::CMutex > TStatePtr;

    CDRNNamespaceClient( const CDRNNamespaceClient& src );                  /**< not implemented */
    CDRNNamespaceClient& operator=( const CDRNNamespaceClient& src );       /**< not implemented */

    CORE::CFutureResult Queue( CCallJob* job, CDRNNamespaceResponsePtr& response );

    TStatePtr m_state;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRN_CDRNNAMESPACECLIENT_H ? */
