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

#ifndef GUCEF_DRN_CDRNRESOURCESESSIONCOMPONENT_H
#define GUCEF_DRN_CDRNRESOURCESESSIONCOMPONENT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_CORE_CRATELIMITER_H
#include "gucefCORE_CRateLimiter.h"
#define GUCEF_CORE_CRATELIMITER_H
#endif /* GUCEF_CORE_CRATELIMITER_H ? */

#ifndef GUCEF_DRN_CIDRNSESSIONCOMPONENT_H
#include "gucefDRN_CIDRNSessionComponent.h"
#define GUCEF_DRN_CIDRNSESSIONCOMPONENT_H
#endif /* GUCEF_DRN_CIDRNSESSIONCOMPONENT_H ? */

#ifndef GUCEF_DRN_CIDRNRESOURCEPROVIDER_H
#include "gucefDRN_CIDRNResourceProvider.h"
#define GUCEF_DRN_CIDRNRESOURCEPROVIDER_H
#endif /* GUCEF_DRN_CIDRNRESOURCEPROVIDER_H ? */

#ifndef GUCEF_DRN_CDRNRESOURCESHARERULES_H
#include "gucefDRN_CDRNResourceShareRules.h"
#define GUCEF_DRN_CDRNRESOURCESHARERULES_H
#endif /* GUCEF_DRN_CDRNRESOURCESHARERULES_H ? */

#ifndef GUCEF_DRN_CDRNRESOURCEHANDLE_H
#include "gucefDRN_CDRNResourceHandle.h"
#define GUCEF_DRN_CDRNRESOURCEHANDLE_H
#endif /* GUCEF_DRN_CDRNRESOURCEHANDLE_H ? */

#ifndef GUCEF_DRN_CDRNNAMESPACEREQUEST_H
#include "gucefDRN_CDRNNamespaceRequest.h"
#define GUCEF_DRN_CDRNNAMESPACEREQUEST_H
#endif /* GUCEF_DRN_CDRNNAMESPACEREQUEST_H ? */

#ifndef GUCEF_DRN_CDRNNAMESPACEENTRY_H
#include "gucefDRN_CDRNNamespaceEntry.h"
#define GUCEF_DRN_CDRNNAMESPACEENTRY_H
#endif /* GUCEF_DRN_CDRNNAMESPACEENTRY_H ? */

#ifndef GUCEF_DRN_CDRNRESOURCETRANSFER_H
#include "gucefDRN_CDRNResourceTransfer.h"
#define GUCEF_DRN_CDRNRESOURCETRANSFER_H
#endif /* GUCEF_DRN_CDRNRESOURCETRANSFER_H ? */

#ifndef GUCEF_DRN_CDRNRESOURCETRANSFERSETTINGS_H
#include "gucefDRN_CDRNResourceTransferSettings.h"
#define GUCEF_DRN_CDRNRESOURCETRANSFERSETTINGS_H
#endif /* GUCEF_DRN_CDRNRESOURCETRANSFERSETTINGS_H ? */

#ifndef GUCEF_DRN_CDRNRESOURCETRANSFERSTATS_H
#include "gucefDRN_CDRNResourceTransferStats.h"
#define GUCEF_DRN_CDRNRESOURCETRANSFERSTATS_H
#endif /* GUCEF_DRN_CDRNRESOURCETRANSFERSTATS_H ? */

#ifndef GUCEF_DRN_CDRNPEERLIMITS_H
#include "gucefDRN_CDRNPeerLimits.h"
#define GUCEF_DRN_CDRNPEERLIMITS_H
#endif /* GUCEF_DRN_CDRNPEERLIMITS_H ? */

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
class CDRNAuthResult;
class CIDRNPeerLimitPolicy;

/**
 *  The resource transfers of a session: the session component of DRN_MSGCLASS_RESOURCE (protocol 2.2), built into
 *  every session (CDRNSession::GetResources()), so that every member of protocol 2.2 answers.
 *
 *  Serving: the application registers providers under names (RegisterProvider()); a member opens a resource of a
 *  provider by name, reads windows of it with a credit, and gets its chunks, with their audit paths when it asked for
 *  the hash tree. Who may read is the provider's role set and the node's CIDRNAccessPolicy::MayReadResourceAs().
 *  Provider calls and hash trees run on the pool; chunks are sent from the node's thread while the link's send
 *  backlog is below the bound of the settings, round robin over the reads of the member.
 *
 *  Requesting: Fetch() and Read() make a CDRNResourceTransfer into a sink; ReadRange() waits for a window on the
 *  calling thread. Credit is shared round robin by the transfers of a link, within the link's cap and below this
 *  node's own receive limits.
 *
 *  Namespace (protocol 2.3): the providers that override the namespace operations of CIDRNResourceProvider are shares
 *  members can browse (SHARES, STAT, LIST; DRN_RES_NS_*) before they open anything. The component enforces on its own
 *  what a share must not do: names are checked before a provider sees them, the roles, the subtrees of the share rules and
 *  the access policy (MayBrowseResourceAs(), MayListEntryAs()) decide what a member sees, entries with names the resource
 *  rules refuse are left out and counted, every answer is bounded by the policy's maxMessageSize and the settings, a
 *  member has at most a few listings under way (cursors with a lifetime), and a request that would change a share is
 *  answered READ_ONLY. Provider work runs on the pool. The requesting side is CDRNNamespaceClient.
 *
 *  Limits (CDRNResourceTransferSettings): the open handles, active reads, namespace requests being worked on and listings under
 *  way each have a server-wide limit (all members together) and a per-member limit, set independently; a request is refused BUSY
 *  when either is reached, and the message names which. Upload rate and read ahead have both tiers too. The per-member values can
 *  be changed per member by a CIDRNPeerLimitPolicy (SetPeerLimitPolicy()), the server-wide ones never. NS_HELLO tells the asking
 *  member the lower of both tiers, so that CDRNNamespaceClient keeps within them.
 *
 *  Direct links only: a member without a direct link (a client of a CLIENT_SERVER session to another client) cannot be
 *  asked (UNSUPPORTED). Everything runs under the node lock; calls of the application take it.
 */
class GUCEF_DRN_EXPORT_CPP CDRNResourceSessionComponent : public CIDRNSessionComponent
{
    public:

    explicit CDRNResourceSessionComponent( CDRNSession& session );

    virtual ~CDRNResourceSessionComponent();

    CDRNSession& GetSession( void ) const;

    void SetSettings( const CDRNResourceTransferSettings& settings );
    CDRNResourceTransferSettings GetSettings( void ) const;

    CDRNResourceTransferStats GetStats( void ) const;

    /**
     *  Serves a provider under a name (CDRNResourceName::IsValidProviderName()), to the members whose authentication
     *  What it shares and with whom are the rules (CDRNResourceShareRules): without rules nothing is readable, unless
     *  the node's access policy allows it. False when the name is taken or not valid, or a subtree of the rules is not.
     */
    bool RegisterProvider( const CORE::CString& providerName, const CIDRNResourceProviderPtr& provider,
                           const CDRNResourceShareRules& rules = CDRNResourceShareRules() );

    /** Stops serving a provider: its open handles are closed (CLOSED for reads under way) */
    bool UnregisterProvider( const CORE::CString& providerName );

    bool HasProvider( const CORE::CString& providerName ) const;

    /**
     *  Asks 'policy' for the limits of every member from now on instead of using the per-member values of the settings
     *  as they are (CIDRNPeerLimitPolicy; NULL for the settings as they are, the default). Not owned: it has to outlive the
     *  component or be unset first. The server-wide limits of the settings are not passed through it.
     */
    void SetPeerLimitPolicy( CIDRNPeerLimitPolicy* policy );

    /**
     *  The answers of the policy are kept for a short time per member. Call it when the policy's answer for a member changed
     *  and the change has to apply now (peerId 0 for every member)
     */
    void RefreshPeerLimits( UInt32 peerId = 0 );

    /** The per-member limits that apply to a member now: the configured values, or what the policy made of them */
    CDRNPeerLimits GetEffectivePeerLimits( UInt32 peerId );

    /** Resources a member holds open here (serving side), or all members together for peerId 0 */
    UInt32 GetOpenHandleCount( UInt32 peerId ) const;

    /** Reads served here that have not ended, of a member or of all members together (peerId 0) */
    UInt32 GetActiveReadCount( UInt32 peerId ) const;

    /** Bytes read for a member and not yet sent (serving side), or for all members together (peerId 0) */
    UInt64 GetBufferedBytes( UInt32 peerId ) const;

    /** Namespace requests of a member (STAT, LIST) a pool job works on here, or of all members together (peerId 0) */
    UInt32 GetNamespaceServingCount( UInt32 peerId ) const;

    /** Fetches a whole resource of a member into a sink */
    CDRNResourceTransfer* Fetch( UInt32 peerId, const CORE::CString& providerName, const CORE::CString& resourceName,
                                 const CIDRNResourceSinkPtr& sink, const CDRNResourceTransferOptions& options = CDRNResourceTransferOptions() );

    /** Reads [offset, offset + length) of a resource of a member into a sink, in order for a sink that wants it */
    CDRNResourceTransfer* Read( UInt32 peerId, const CORE::CString& providerName, const CORE::CString& resourceName,
                                UInt64 offset, UInt64 length, const CIDRNResourceSinkPtr& sink,
                                const CDRNResourceTransferOptions& options = CDRNResourceTransferOptions() );

    /**
     *  Reads [offset, offset + length) of a resource into 'data', waiting on the calling thread (at most 2 GiB). Not on
     *  a thread that works on the node (TIMEOUT, at once). The thread that pumps a node without a thread of its own
     *  (CDRNNode::IsPumpThread()) works the node itself while it waits.
     */
    CDRNResourceTransferResult ReadRange( UInt32 peerId, const CORE::CString& providerName, const CORE::CString& resourceName,
                                          UInt64 offset, UInt64 length, CORE::CDynamicBuffer& data,
                                          const CDRNResourceTransferOptions& options, UInt32 timeoutInMs );

    /**
     *  Opens a resource of a member and keeps it open: the first request opens it, every window read through the
     *  handle after that uses the same handle (CDRNResourceHandle::ReadChunks(), ReadRange(), CDRNResourceIOAccess).
     *  The handle is the caller's: it closes when its last reference goes, or with Close(). Not NULL; a handle that
     *  cannot be opened is closed at once with the reason (GetResult()), or once the serving side answered.
     */
    CDRNResourceHandlePtr OpenHandle( UInt32 peerId, const CORE::CString& providerName, const CORE::CString& resourceName,
                                      const CDRNResourceTransferOptions& options = CDRNResourceTransferOptions() );

    /** Handles of this side that are open or opening */
    UInt32 GetClientHandleCount( void ) const;

    /** Lets go of a finished transfer; false while it runs */
    bool Release( CDRNResourceTransfer* transfer );

    /** A new run of the session: transfers and handles end */
    void Reset( void );

    /** A request id for a namespace call, unique for this component (CDRNNamespaceClient takes one per call) */
    UInt32 GetNextNamespaceRequestId( void );

    /**
     *  Sends one namespace request to a member and waits for its answer on the calling thread (the low level of
     *  CDRNNamespaceClient, which encodes the request with CDRNNamespaceCodec: 'requestBody' is the whole body, starting
     *  with 'requestId' as GetNextNamespaceRequestId() gave it). Returns the status of the answer, or LINK_LOST, UNSUPPORTED
     *  (no direct link, or the member's node is below protocol 2.3), INSECURE_LINK or TIMEOUT when there is none; for OK
     *  (and UNCHANGED) 'answerBody' holds the body of the answer that follows its header. Not on a thread that works on the node.
     */
    TDRNResourceStatus CallNamespace( UInt32 peerId, UInt32 opcode, UInt32 requestId, const CORE::CDynamicBuffer& requestBody,
                                      UInt32 timeoutInMs, CORE::CString& message, CORE::CDynamicBuffer& answerBody );

    /** Namespace requests of this side waiting for their answer */
    UInt32 GetNamespaceRequestCount( void ) const;

    /** Listings members have under way here (serving side), of one member or of all (peerId 0) */
    UInt32 GetNamespaceListingCount( UInt32 peerId ) const;

    virtual UInt8 GetMessageClass( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnSessionMessage( CDRNSession& session, CDRNPeerLink& link, UInt32 fromPeerId, const CDRNLinkMessage& message ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnSessionTick( CDRNSession& session, UInt64 nodeNow ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnMemberLeft( CDRNSession& session, UInt32 peerId ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnSessionClosed( CDRNSession& session ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    friend class CDRNResourceTransfer;
    friend class CDRNResourceHandle;

    class CProviderEntry;
    class CServeHandle;
    class CServeRead;
    class CResolveJob;
    class CTreeJob;
    class CReadJob;
    class CPeerBudget;
    class CCachedTree;
    class CNsListing;
    class CNsServe;
    class CNsStatJob;
    class CNsListJob;
    class CPeerLimitEntry;

    typedef CORE::CTSharedPtr< CDRNResourceHashTree, MT::CMutex > TTreePtr;
    typedef GUCEF::map< CORE::CString, CProviderEntry* > TProviderMap;
    typedef GUCEF::vector< CServeHandle* > THandleVector;
    typedef GUCEF::vector< CDRNResourceTransfer* > TTransferVector;
    typedef GUCEF::map< UInt32, CPeerBudget* > TPeerBudgetMap;
    typedef GUCEF::map< CORE::CString, CCachedTree* > TTreeCache;
    typedef GUCEF::vector< CDRNResourceHandle* > TClientHandleVector;
    typedef GUCEF::vector< CNsListing* > TNsListingVector;
    typedef GUCEF::vector< CNsServe* > TNsServeVector;
    typedef GUCEF::vector< CDRNNamespaceRequestPtr > TNsPendingVector;
    typedef GUCEF::map< UInt32, CPeerLimitEntry* > TPeerLimitMap;

    CDRNResourceSessionComponent( const CDRNResourceSessionComponent& src );               /**< not implemented */
    CDRNResourceSessionComponent& operator=( const CDRNResourceSessionComponent& src );    /**< not implemented */

    CDRNResourceTransfer* StartTransfer( UInt32 peerId, const CORE::CString& providerName, const CORE::CString& resourceName,
                                         bool wholeResource, UInt64 offset, UInt64 length, const CIDRNResourceSinkPtr& sink,
                                         const CDRNResourceTransferOptions& options );

    /** The direct link to a member that can carry the class (operational, protocol 2.2, protected unless allowed) */
    CDRNPeerLinkPtr GetUsableLink( UInt32 peerId, TDRNResourceStatus& status, CORE::CString& why, UInt8 minimumMinor = DRN_PROTOCOL_MINOR_RESOURCE ) const;
    bool Send( UInt32 peerId, UInt8 lane, UInt32 opcode, const void* part1, UInt32 size1, const void* part2 = GUCEF_NULL, UInt32 size2 = 0 );

    // serving
    void OnOpen( UInt32 peerId, CDRNPeerLink& link, CDRNWireReader& reader );
    void OnRead( UInt32 peerId, CDRNWireReader& reader );
    void OnCredit( UInt32 peerId, CDRNWireReader& reader );
    void OnCancel( UInt32 peerId, CDRNWireReader& reader );
    void OnClose( UInt32 peerId, CDRNWireReader& reader );
    void SendInfo( UInt32 peerId, UInt32 handle, TDRNResourceStatus status, const CORE::CString& message, const CDRNResourceInfo* info );
    void SendReadEnd( UInt32 peerId, UInt32 handle, UInt32 readId, TDRNResourceStatus status, const CORE::CString& message );
    CServeHandle* FindHandle( UInt32 peerId, UInt32 handle ) const;
    UInt32 CountHandles( UInt32 peerId ) const;
    UInt32 CountReads( UInt32 peerId ) const;
    UInt32 CountAllReads( void ) const;
    UInt64 CountBufferedBytes( UInt32 peerId ) const;
    CDRNPeerLimits GetConfiguredPeerLimits( void ) const;
    CDRNPeerLimits GetPeerLimits( UInt32 peerId );
    void DropPeerLimits( UInt32 peerId, bool allPeers );
    bool IsLimitReached( UInt32 serverCount, UInt32 serverMax, UInt32 peerCount, UInt32 peerMax, const char* what, CORE::CString& why );
    void UpdatePeerUploadLimiter( CPeerBudget& budget, UInt32 bytesPerSecond );
    void DropHandle( CServeHandle* handle );
    void DropHandlesOf( UInt32 peerId, bool allPeers );
    void TickServing( UInt64 nowMs );
    void OnResolved( CServeHandle& handle );
    void ObtainTree( CServeHandle& handle );
    void StartTreeBuilds( void );
    void OnTreeBuilt( CServeHandle& handle );
    void CacheTree( const CORE::CString& key, const TTreePtr& tree, UInt64 nowMs );
    void IssueReads( CServeHandle& handle );
    void CollectRead( CServeHandle& handle, CServeRead& read );
    void SendChunks( UInt64 nowMs );
    void UpdateServerBuffered( Int64 delta );

    // serving the namespace
    void OnNamespaceMessage( UInt32 peerId, CDRNPeerLink& link, const CDRNLinkMessage& message );
    void OnNsHello( UInt32 peerId, UInt32 requestId, CDRNWireReader& reader );
    void OnNsShares( UInt32 peerId, UInt32 requestId );
    void OnNsStat( UInt32 peerId, UInt32 requestId, CDRNWireReader& reader );
    void OnNsList( UInt32 peerId, UInt32 requestId, CDRNWireReader& reader );
    void SendNsAnswer( UInt32 peerId, UInt32 requestId, UInt32 kind, TDRNResourceStatus status, const CORE::CString& message );
    void RefuseNs( UInt32 peerId, UInt32 requestId, UInt32 kind, TDRNResourceStatus status, const CORE::CString& message );
    UInt32 GetNsAnswerBytes( void ) const;
    UInt32 GetNsStatLimit( const CIDRNResourceProvider& provider ) const;
    UInt32 GetNsPageLimit( const CIDRNResourceProvider& provider ) const;
    UInt32 CountNsServing( UInt32 peerId ) const;
    CNsListing* FindNsListing( UInt32 peerId, UInt32 listingId ) const;
    bool IsHiddenByPolicy( UInt32 peerId, const CDRNAuthResult& auth, const CORE::CString& providerName, const CORE::CString& name, bool leafIsDirectory );
    void TickNamespace( UInt64 nowMs );
    void CompleteNsStat( CNsServe& serve );
    void CompleteNsList( CNsServe& serve );
    void FinishNsList( CNsServe& serve, TDRNResourceStatus status, const CORE::CString& message, bool unchanged );
    void DropNsServe( CNsServe* serve );
    void DropNsListing( CNsListing* listing );
    void DropNamespaceOf( UInt32 peerId, bool allPeers );
    void DropNamespaceOfProvider( const CORE::CString& providerName );

    // requesting the namespace
    void OnNsAnswer( UInt32 peerId, CDRNWireReader& reader );
    void FailNamespaceRequestsOf( UInt32 peerId, bool allPeers, TDRNResourceStatus status, const CORE::CString& why );

    // requesting
    CDRNResourceTransfer* FindTransfer( UInt32 peerId, UInt32 handle ) const;
    void OnInfo( UInt32 peerId, CDRNWireReader& reader );
    void OnData( UInt32 peerId, CDRNWireReader& reader );
    void OnReadEnd( UInt32 peerId, CDRNWireReader& reader );
    CPeerBudget& GetBudget( UInt32 peerId );
    CPeerBudget& UpdateBudget( UInt32 peerId, const CDRNPeerLink& link );
    /** Takes the bytes from the receive rate of the link to the member: 0 when taken, else milliseconds to wait first */
    UInt32 AcquireReceiveBudget( UInt32 peerId, UInt32 bytes );
    CDRNResourceHandle* FindClientHandle( UInt32 peerId, UInt32 handle ) const;
    void ForgetHandle( CDRNResourceHandle* handle );
    void LoseClientHandles( UInt32 peerId, bool allPeers, TDRNResourceStatus status, const CORE::CString& why );
    void ShareCredit( UInt64 nowMs );
    void AddLinkOutstanding( UInt32 peerId, Int64 delta );
    void UpdateClientBuffered( Int64 delta );
    void FailTransfersOf( UInt32 peerId, bool allPeers, TDRNResourceStatus status, const CORE::CString& why );

    CDRNSession* m_session;
    CDRNResourceTransferSettings m_settings;
    CDRNResourceTransferStats m_stats;
    TProviderMap m_providers;
    THandleVector m_handles;
    THandleVector m_treeQueue;          /**< handles waiting for a slot to build their tree */
    UInt32 m_treeBuilds;
    TTreeCache m_treeCache;
    UInt64 m_treeCacheBytes;
    CORE::CRateLimiter* m_uploadLimiter;
    UInt32 m_uploadLimiterRate;
    CIDRNPeerLimitPolicy* m_peerLimitPolicy;    /**< not owned, NULL: the configured per-member values apply as they are */
    TPeerLimitMap m_peerLimits;                 /**< what the policy said per member, kept for a short time */
    TTransferVector m_transfers;
    TTransferVector m_retired;          /**< let go of by the component (timed out synchronous reads, earlier runs): they go with it */
    TPeerBudgetMap m_budgets;
    TClientHandleVector m_clientHandles;    /**< not owned: the handles are their callers', they leave this list when they close */
    UInt32 m_nextHandle;
    UInt32 m_creditRound;
    CORE::CDynamicBuffer m_txBuffer;
    CORE::CDynamicBuffer m_nsBuffer;        /**< the entries of a page, while the answer is built */
    TNsListingVector m_nsListings;
    TNsServeVector m_nsServing;
    TNsPendingVector m_nsPending;
    UInt32 m_nextNsListingId;
    UInt32 m_nextNsRequestId;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRN_CDRNRESOURCESESSIONCOMPONENT_H ? */
