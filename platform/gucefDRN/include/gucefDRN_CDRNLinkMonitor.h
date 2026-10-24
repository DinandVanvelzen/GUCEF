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

#ifndef GUCEF_DRN_CDRNLINKMONITOR_H
#define GUCEF_DRN_CDRNLINKMONITOR_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTCLONEABLEOBJ_H
#include "CTCloneableObj.h"
#define GUCEF_CORE_CTCLONEABLEOBJ_H
#endif /* GUCEF_CORE_CTCLONEABLEOBJ_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_DRN_CDRNSECURITYPOLICY_H
#include "gucefDRN_CDRNSecurityPolicy.h"
#define GUCEF_DRN_CDRNSECURITYPOLICY_H
#endif /* GUCEF_DRN_CDRNSECURITYPOLICY_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRN {

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

/** What a link reports about its peer */
enum TDRNLinkAnomalyKind
{
    DRN_ANOMALY_NONE                    = 0,

    // PeerLimitExceededEvent
    DRN_LIMIT_MESSAGES_PER_SECOND       = 1,    /**< more messages than maxMessagesPerSecond */
    DRN_LIMIT_BYTES_PER_SECOND          = 2,    /**< more bytes than maxBytesPerSecond */
    DRN_LIMIT_MESSAGE_SIZE              = 3,    /**< a message larger than maxMessageSize */
    DRN_LIMIT_FRAGMENT_GROUPS           = 4,    /**< more messages in fragments at once than maxFragmentGroups */
    DRN_LIMIT_FRAGMENT_BYTES            = 5,    /**< more bytes in fragments at once than maxFragmentBytes */
    DRN_LIMIT_DEFERRED_BYTES            = 6,    /**< stream channels: more held back for the limits than a second's worth */

    // ClockAnomalyEvent
    DRN_ANOMALY_CLOCK_DRIFT             = 20,   /**< the peer's sendTime runs faster or slower than our clock */

    // TrafficAnomalyEvent
    DRN_ANOMALY_BURST_AFTER_SILENCE     = 30,   /**< silence and then a burst of packets: a lag switch */
    DRN_ANOMALY_FLOOD                   = 31,   /**< more packets in a second than floodPacketsPerSecond */

    DRN_LINK_LIMIT_KIND_COUNT           = 7     /**< the limit kinds are 1 to 6 */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** Event data of PeerLimitExceededEvent, ClockAnomalyEvent and TrafficAnomalyEvent */
class GUCEF_DRN_EXPORT_CPP CDRNLinkAnomaly
{
    public:

    TDRNLinkAnomalyKind kind;
    Float64 measured;           /**< what was seen: a rate per second, a size, a drift in percent, a silence in ms */
    Float64 threshold;          /**< the value of the policy it went over */
    UInt64 nodeTime;            /**< CDRNClock time */
    CORE::CString text;

    CDRNLinkAnomaly( void );

    static CORE::CString KindToString( TDRNLinkAnomalyKind kind );
};

typedef CORE::CTCloneableObj< CDRNLinkAnomaly > CDRNLinkAnomalyEventData;

/*-------------------------------------------------------------------------*/

/**
 *  Watches what a link receives from its peer: the receive limits of the security policy (token buckets of one
 *  second), the counters of the telemetry, and the anomalies (clock drift, silence then a burst, floods).
 *  Used by CDRNPeerLink on the node's thread. Works without allocating.
 */
class GUCEF_DRN_EXPORT_CPP CDRNLinkMonitor
{
    public:

    CDRNLinkMonitor( void );

    void Reset( const CDRNPolicyLimits& limits, const CDRNAnomalyThresholds& thresholds, UInt64 nodeNow );

    /**
     *  Whether a packet may be taken now: false while the peer used up its messages or bytes of the last second
     *  @param exceeded the limit that was used up
     */
    bool MayReceive( UInt64 nodeNow, TDRNLinkAnomalyKind& exceeded );

    /** Charges what a packet that was taken carried */
    void OnReceived( UInt32 bytes, UInt32 messages );

    /**
     *  Counts a limit hit
     *  @return true if it should be reported now: at most once per second per kind
     */
    bool OnLimitHit( TDRNLinkAnomalyKind kind, UInt64 nodeNow );

    /** A packet that was not taken because of a limit */
    void OnPacketDropped( void );

    /** True once the peer went over a receive limit in sustainedLimitSeconds consecutive seconds */
    bool IsOverLimitsSustained( UInt64 nodeNow );

    /**
     *  Every packet that arrives, taken or not: arrival intervals, floods, silence followed by a burst
     *  @return true if a traffic anomaly was found, described by anomaly
     */
    bool OnPacketArrival( UInt64 nodeNow, UInt32 bytes, CDRNLinkAnomaly& anomaly );

    /**
     *  A packet that was taken, with the sendTime of its header and our link time when it arrived
     *  @return true if the clock drift went over the threshold for long enough, described by anomaly
     */
    bool OnPacketTimed( UInt64 nodeNow, UInt64 localLinkTime, UInt64 peerSendTime, CDRNLinkAnomaly& anomaly );

    /** Adds the counters to the node as attributes */
    void GetTelemetry( CORE::CDataNode& node ) const;

    UInt64 GetLimitHitCount( void ) const;
    UInt64 GetLimitHitCount( TDRNLinkAnomalyKind kind ) const;
    UInt64 GetDroppedPacketCount( void ) const;
    UInt64 GetClockAnomalyCount( void ) const;
    UInt64 GetTrafficAnomalyCount( void ) const;

    /** The drift in percent of the last finished clock window */
    Float64 GetLastClockDriftPercent( void ) const;

    private:

    void Refill( UInt64 nodeNow );
    void RollSecond( UInt64 nodeNow );

    CDRNPolicyLimits m_limits;
    CDRNAnomalyThresholds m_thresholds;

    // token buckets
    Int64 m_messageTokens;
    Int64 m_byteTokens;
    UInt64 m_lastRefill;

    // the current second
    UInt64 m_secondStart;
    UInt32 m_packetsThisSecond;
    UInt32 m_messagesThisSecond;
    UInt64 m_bytesThisSecond;
    bool m_limitHitThisSecond;
    bool m_floodReportedThisSecond;
    UInt32 m_consecutiveLimitSeconds;

    // the last whole second and the peaks
    UInt32 m_packetsLastSecond;
    UInt32 m_messagesLastSecond;
    UInt64 m_bytesLastSecond;
    UInt32 m_peakPacketsPerSecond;
    UInt32 m_peakMessagesPerSecond;
    UInt64 m_peakBytesPerSecond;

    // arrivals
    UInt64 m_lastArrival;
    Int64 m_meanIntervalUs;
    Int64 m_jitterUs;
    UInt64 m_maxIntervalUs;
    bool m_burstWatch;
    UInt64 m_burstWatchStart;
    UInt64 m_burstSilenceUs;
    UInt32 m_burstCount;

    // clock windows: the least delayed sample (highest sendTime - local time) per window
    bool m_clockWindowActive;
    UInt64 m_clockWindowStart;
    Int64 m_clockBestOffset;
    UInt64 m_clockBestTime;
    bool m_clockHavePrevious;
    Int64 m_clockPreviousOffset;
    UInt64 m_clockPreviousTime;
    UInt64 m_clockDriftSinceUs;         /**< how long the drift has been over the threshold */
    bool m_clockDrifting;
    Float64 m_lastDriftPercent;

    // counters
    UInt64 m_packetsArrived;
    UInt64 m_bytesArrived;
    UInt64 m_messagesTaken;
    UInt64 m_droppedPackets;
    UInt64 m_limitHits[ DRN_LINK_LIMIT_KIND_COUNT ];
    UInt64 m_lastLimitReport[ DRN_LINK_LIMIT_KIND_COUNT ];
    UInt64 m_clockAnomalies;
    UInt64 m_trafficAnomalies;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRN_CDRNLINKMONITOR_H ? */
