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

#ifndef GUCEF_DRN_CDRNRESOURCETRANSFERSETTINGS_H
#define GUCEF_DRN_CDRNRESOURCETRANSFERSETTINGS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CICONFIGURABLE_H
#include "gucefCORE_CIConfigurable.h"
#define GUCEF_CORE_CICONFIGURABLE_H
#endif /* GUCEF_CORE_CICONFIGURABLE_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_DRN_MACROS_H
#include "gucefDRN_macros.h"
#define GUCEF_DRN_MACROS_H
#endif /* GUCEF_DRN_MACROS_H ? */

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

/**
 *  The limits of the resource transfers of a session (CDRNResourceSessionComponent), for both of its roles: serving
 *  through providers and requesting. They are local decisions of the node and are not part of the security policy
 *  (whose hash both ends of a link compare). Config section "DRNResourceTransferSettings".
 *
 *  Two tiers of limits on the serving side, set independently: a name without a suffix is a server-wide limit (all
 *  members together, like maxUploadBytesPerSecond), a name with the suffix PerPeer is the limit of one member. A request
 *  is refused (BUSY, the message names the tier) when either limit is reached, so a member is held to the lower of the two.
 *  0 is no maximum for both. The per-member values are what CIDRNPeerLimitPolicy starts from; the server-wide values are
 *  never changed by it. NS_HELLO announces the lower of both tiers, for the asking member, to the requester.
 */
class GUCEF_DRN_EXPORT_CPP CDRNResourceTransferSettings : public CORE::CIConfigurable
{
    public:

    static const CORE::CString ClassTypeName;

    UInt32 maxOpenHandlesPerPeer;     /**< serving: resources one member may hold open at once, BUSY beyond, 0 = no maximum, default 32 */
    UInt32 maxOpenHandles;            /**< serving: resources all members together may hold open at once, BUSY beyond, 0 = no maximum, default 4096 */
    UInt32 maxActiveReadsPerPeer;     /**< serving: reads of one member served at once, BUSY beyond, 0 = no maximum, default 8 */
    UInt32 maxActiveReads;            /**< serving: reads of all members together served at once, BUSY beyond, 0 = no maximum, default 256 */
    UInt32 readAheadChunks;           /**< serving: chunks read ahead of sending per read, within its credit, default 8 */
    UInt32 maxBufferedBytes;          /**< serving: bytes read and not yet sent, all reads together; no read ahead beyond it, default 32 * 1024 * 1024 */
    UInt32 maxBufferedBytesPerPeer;   /**< serving: bytes read and not yet sent for one member; no read ahead for it beyond, 0 = no maximum, default 0 */
    UInt32 maxLinkBacklogBytes;       /**< serving: chunks are sent while less than this waits to be sent on the link (stream queue and channel), default 512 * 1024 */
    UInt32 maxUnackedMessages;        /**< serving, datagram links: chunks are sent while fewer reliable messages than this are unacknowledged, default 256 */
    UInt32 maxUploadBytesPerSecond;   /**< serving: bytes per second sent by all reads together, 0 = no limit, default 0 */
    UInt32 maxUploadBytesPerSecondPerPeer; /**< serving: bytes per second sent to one member by its reads together, 0 = no limit, default 0 */
    UInt32 maxConcurrentTreeBuilds;   /**< serving: hash trees built at once; 0 = no trees are built (only those providers keep), default 1 */
    UInt64 maxTreeResourceBytes;      /**< serving: largest resource a hash tree is built for, default 4 * 1024 * 1024 * 1024 */
    UInt32 maxTreeCacheBytes;         /**< serving: bytes of hash trees kept in memory for later opens, default 64 * 1024 * 1024 */
    CORE::CString treeCacheDirectory; /**< serving: where hash trees are kept on disk across runs (CDRNResourceTreeStore); empty: in memory only, the default. Never the folder of what is served unless an application points it there */
    UInt64 treeCacheMaxDiskBytes;     /**< serving: bytes the tree cache directory may hold, the oldest trees go first, default 1 GiB */
    UInt32 treeCacheMaxAgeSeconds;    /**< serving: trees written longer ago than this leave the tree cache directory, default 30 days, 0 = no maximum */
    UInt32 handleIdleTimeoutMs;       /**< serving: an open resource without a read for this long is closed (the requester reopens it), default 300000, 0 = none */
    UInt32 maxNamespaceStatPaths;     /**< serving, namespace: paths in one STAT, TOO_LARGE beyond, default 256 */
    UInt32 maxNamespacePageEntries;   /**< serving, namespace: entries in one page of a listing (and what a request that names no number gets), TOO_LARGE beyond, default 1000 */
    UInt32 maxNamespaceAnswerBytes;   /**< serving, namespace: largest answer, a page is cut to it; never more than the security policy's maxMessageSize, default 512 * 1024 */
    UInt32 maxNamespaceListingsPerPeer; /**< serving, namespace: listings a member has under way (continued with a cursor), BUSY beyond, 0 = no maximum, default 8 */
    UInt32 maxNamespaceListings;      /**< serving, namespace: listings all members together have under way, BUSY beyond, 0 = no maximum, default 256 */
    UInt32 namespaceListingLifetimeMs;/**< serving, namespace: a listing not continued for this long is forgotten (CURSOR_EXPIRED), default 60000 */
    UInt32 maxNamespaceRequestsPerPeer; /**< serving, namespace: requests of a member being worked on at once, BUSY beyond, 0 = no maximum, default 16 */
    UInt32 maxNamespaceRequests;      /**< serving, namespace: requests of all members together being worked on at once, BUSY beyond, 0 = no maximum, default 512 */
    bool announceLimits;              /**< serving, namespace: NS_HELLO tells the asking member the limits that apply to it (the lower of both tiers), so that its client keeps within them; default true */
    UInt32 maxCreditChunksPerLink;    /**< requesting: credit outstanding on one link, all transfers together; datagram links at most half the policy's maxFragmentGroups, default 64 */
    UInt32 maxDownloadBytesPerSecond; /**< requesting: bytes per second asked for on one link, 0 = only the receive limits of the security policy, default 0 */
    UInt32 receiveLimitSharePercent;  /**< requesting: share of the receive limits of the security policy that credit may use, so a transfer never trips them, default 75 */
    UInt32 stateSaveIntervalMs;       /**< requesting: how often a resumable target records its state, default 1000 */
    bool allowUnprotectedLinks;       /**< both: serve and request over links without packet protection, default false */
    CORE::CString threadPoolName;     /**< both: the task manager pool for file I/O and hashing, empty for the default pool, default none */

    CDRNResourceTransferSettings( void );
    virtual ~CDRNResourceTransferSettings();

    virtual bool SaveConfig( CORE::CDataNode& cfg ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool LoadConfig( const CORE::CDataNode& cfg ) GUCEF_VIRTUAL_OVERRIDE;
    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRN_CDRNRESOURCETRANSFERSETTINGS_H ? */
