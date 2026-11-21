/*
 *  gucefLabLink: links the machines of a home network to its k3s cluster
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

#ifndef GUCEF_LABLINK_REGISTRY_H
#define GUCEF_LABLINK_REGISTRY_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#include "gucefLabLink_NodeReport.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEFLabLink {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** Whether the hub currently hears from a node */
enum ENodeLiveness
{
    NODELIVENESS_UNKNOWN = 0,       /**< loaded from storage, no report received since the hub started */
    NODELIVENESS_ONLINE,
    NODELIVENESS_OFFLINE
};

/** What happened to a report */
enum EReportResult
{
    REPORT_ACCEPTED = 0,
    REPORT_INVALID,                 /**< no machine id */
    REPORT_TABLE_FULL,              /**< the table of verified or unverified nodes is full */
    REPORT_DUPLICATE_INSTANCE,      /**< another instance of this machine id reported within the offline window: a cloned disk */
    REPORT_UNVERIFIED_CLAIM         /**< an unverified reporter used the machine id of a verified node */
};

/** The bounds and timings of the registry */
struct SRegistryLimits
{
    GUCEF::UInt32 maxVerifiedNodes;
    GUCEF::UInt32 maxUnverifiedNodes;
    GUCEF::UInt32 offlineAfterInSecs;               /**< a node that did not report for this long is offline: three report intervals */
    GUCEF::UInt32 forgetVerifiedAfterInSecs;
    GUCEF::UInt32 forgetUnverifiedAfterInSecs;
    GUCEF::UInt32 flushIntervalInSecs;              /**< how often an online node is written to storage when nothing else changed */

    SRegistryLimits( void )
        : maxVerifiedNodes( 256 ), maxUnverifiedNodes( 64 ), offlineAfterInSecs( 60 )
        , forgetVerifiedAfterInSecs( 90 * 24 * 3600 ), forgetUnverifiedAfterInSecs( 7 * 24 * 3600 ), flushIntervalInSecs( 300 )
    {}
};

/** What the registry knows of one node */
struct SNodeEntry
{
    SNodeReport report;                             /**< the last accepted report */
    bool verified;                                  /**< reported with the token of its approval, not with the lab secret */
    GUCEF::CORE::CString address;
    ENodeLiveness liveness;
    GUCEF::UInt64 firstSeenInUnixSecs;
    GUCEF::UInt64 lastSeenInUnixSecs;               /**< the hub's clock at the last report */
    GUCEF::UInt64 lastSeenInMonoMs;                 /**< monotonic; 0 when no report was received since the hub started */
    GUCEF::UInt32 duplicateInstanceRefusals;        /**< reports of another instance that were refused */
    bool dirty;                                     /**< changed since it was last handed out for storage */
    GUCEF::UInt64 lastPersistedInUnixSecs;

    SNodeEntry( void )
        : report(), verified( false ), address(), liveness( NODELIVENESS_UNKNOWN ), firstSeenInUnixSecs( 0 ), lastSeenInUnixSecs( 0 )
        , lastSeenInMonoMs( 0 ), duplicateInstanceRefusals( 0 ), dirty( false ), lastPersistedInUnixSecs( 0 )
    {}
};

/** A change of a node worth keeping a history of */
struct SNodeEvent
{
    GUCEF::UInt64 unixSecs;
    GUCEF::CORE::CString machineId;
    GUCEF::CORE::CString kind;                      /**< "online", "offline", "state", "instance", "verified" */
    GUCEF::CORE::CString detail;

    SNodeEvent( void ) : unixSecs( 0 ), machineId(), kind(), detail() {}
};

/**
 *  The nodes the hub knows, keyed by machine id. The table in memory is authoritative while the hub runs; storage
 *  gets what CollectForPersist hands out. Time is passed in by the caller, as a monotonic clock for online/offline
 *  decisions (immune to changes of the wall clock) and the wall clock in unix seconds for what is shown and stored.
 *  Thread safe: reports arrive on the threads of the RPC pool, the website reads from others.
 */
class CNodeRegistry
{
    public:

    explicit CNodeRegistry( const SRegistryLimits& limits = SRegistryLimits() );

    /** The entries from storage after a start of the hub: liveness unknown, not dirty */
    void Load( const GUCEF::vector< SNodeEntry >& entries, GUCEF::UInt64 monoNowInMs );

    EReportResult OnReport( const SNodeReport& report, bool verified, const GUCEF::CORE::CString& address,
                            GUCEF::UInt64 monoNowInMs, GUCEF::UInt64 unixNowInSecs );

    /** Online to offline, unknown to offline after the first window, and forgetting of old nodes. Call about once a second */
    void Evaluate( GUCEF::UInt64 monoNowInMs, GUCEF::UInt64 unixNowInSecs );

    bool Find( const GUCEF::CORE::CString& machineId, SNodeEntry& entry ) const;

    void GetAll( GUCEF::vector< SNodeEntry >& entries ) const;

    GUCEF::UInt32 GetCount( void ) const;

    /** Moves the events since the last call out */
    void TakeEvents( GUCEF::vector< SNodeEvent >& events );

    /** Moves the machine ids of the nodes that were forgotten since the last call out, for removal from storage */
    void TakeForgotten( GUCEF::CORE::CString::StringVector& machineIds );

    /** The entries that changed, and the online ones not written for the flush interval; they count as written afterwards */
    void CollectForPersist( GUCEF::UInt64 unixNowInSecs, GUCEF::vector< SNodeEntry >& entries );

    /** After a write to storage failed: everything counts as changed again */
    void MarkAllDirty( void );

    const SRegistryLimits& GetLimits( void ) const;

    private:

    typedef GUCEF::map< GUCEF::CORE::CString, SNodeEntry > TEntryMap;

    CNodeRegistry( const CNodeRegistry& src );
    CNodeRegistry& operator=( const CNodeRegistry& src );

    void AddEvent( GUCEF::UInt64 unixSecs, const GUCEF::CORE::CString& machineId, const char* kind, const GUCEF::CORE::CString& detail );
    GUCEF::UInt32 CountVerified( bool verified ) const;

    SRegistryLimits m_limits;
    TEntryMap m_entries;
    GUCEF::vector< SNodeEvent > m_events;
    GUCEF::CORE::CString::StringVector m_forgotten;
    GUCEF::UInt64 m_startInMonoMs;
    bool m_started;
    mutable GUCEF::MT::CMutex m_lock;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUCEFLabLink */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_LABLINK_REGISTRY_H ? */
