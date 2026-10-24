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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "gucefDRN_CDRNLinkMonitor.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRN {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static const UInt64 OneSecondUs = 1000000;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CDRNLinkAnomaly::CDRNLinkAnomaly( void )
    : kind( DRN_ANOMALY_NONE )
    , measured( 0 )
    , threshold( 0 )
    , nodeTime( 0 )
    , text()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CString
CDRNLinkAnomaly::KindToString( TDRNLinkAnomalyKind kind )
{GUCEF_TRACE;

    switch ( kind )
    {
        case DRN_LIMIT_MESSAGES_PER_SECOND:     return "messagesPerSecond";
        case DRN_LIMIT_BYTES_PER_SECOND:        return "bytesPerSecond";
        case DRN_LIMIT_MESSAGE_SIZE:            return "messageSize";
        case DRN_LIMIT_FRAGMENT_GROUPS:         return "fragmentGroups";
        case DRN_LIMIT_FRAGMENT_BYTES:          return "fragmentBytes";
        case DRN_LIMIT_DEFERRED_BYTES:          return "deferredBytes";
        case DRN_ANOMALY_CLOCK_DRIFT:           return "clockDrift";
        case DRN_ANOMALY_BURST_AFTER_SILENCE:   return "burstAfterSilence";
        case DRN_ANOMALY_FLOOD:                 return "flood";
        default:                                return "none";
    }
}

/*-------------------------------------------------------------------------*/

CDRNLinkMonitor::CDRNLinkMonitor( void )
{GUCEF_TRACE;

    Reset( CDRNPolicyLimits(), CDRNAnomalyThresholds(), 0 );
}

/*-------------------------------------------------------------------------*/

void
CDRNLinkMonitor::Reset( const CDRNPolicyLimits& limits, const CDRNAnomalyThresholds& thresholds, UInt64 nodeNow )
{GUCEF_TRACE;

    m_limits = limits;
    m_thresholds = thresholds;

    m_messageTokens = (Int64) m_limits.maxMessagesPerSecond;
    m_byteTokens = (Int64) m_limits.maxBytesPerSecond;
    m_lastRefill = nodeNow;

    m_secondStart = nodeNow;
    m_packetsThisSecond = 0;
    m_messagesThisSecond = 0;
    m_bytesThisSecond = 0;
    m_limitHitThisSecond = false;
    m_floodReportedThisSecond = false;
    m_consecutiveLimitSeconds = 0;

    m_packetsLastSecond = 0;
    m_messagesLastSecond = 0;
    m_bytesLastSecond = 0;
    m_peakPacketsPerSecond = 0;
    m_peakMessagesPerSecond = 0;
    m_peakBytesPerSecond = 0;

    m_lastArrival = 0;
    m_meanIntervalUs = 0;
    m_jitterUs = 0;
    m_maxIntervalUs = 0;
    m_burstWatch = false;
    m_burstWatchStart = 0;
    m_burstSilenceUs = 0;
    m_burstCount = 0;

    m_clockWindowActive = false;
    m_clockWindowStart = 0;
    m_clockBestOffset = 0;
    m_clockBestTime = 0;
    m_clockHavePrevious = false;
    m_clockPreviousOffset = 0;
    m_clockPreviousTime = 0;
    m_clockDriftSinceUs = 0;
    m_clockDrifting = false;
    m_lastDriftPercent = 0;

    m_packetsArrived = 0;
    m_bytesArrived = 0;
    m_messagesTaken = 0;
    m_droppedPackets = 0;
    for ( UInt32 i=0; i<DRN_LINK_LIMIT_KIND_COUNT; ++i )
    {
        m_limitHits[ i ] = 0;
        m_lastLimitReport[ i ] = 0;
    }
    m_clockAnomalies = 0;
    m_trafficAnomalies = 0;
}

/*-------------------------------------------------------------------------*/

void
CDRNLinkMonitor::Refill( UInt64 nodeNow )
{GUCEF_TRACE;

    if ( nodeNow <= m_lastRefill )
        return;
    UInt64 elapsed = nodeNow - m_lastRefill;
    m_lastRefill = nodeNow;

    // a bucket holds at most one second worth
    if ( 0 != m_limits.maxMessagesPerSecond )
    {
        Int64 capacity = (Int64) m_limits.maxMessagesPerSecond;
        Int64 add = elapsed >= OneSecondUs ? capacity : (Int64) ( ( (UInt64) m_limits.maxMessagesPerSecond * elapsed ) / OneSecondUs );
        m_messageTokens = m_messageTokens + add > capacity ? capacity : m_messageTokens + add;
    }
    if ( 0 != m_limits.maxBytesPerSecond )
    {
        Int64 capacity = (Int64) m_limits.maxBytesPerSecond;
        Int64 add = elapsed >= OneSecondUs ? capacity : (Int64) ( ( (UInt64) m_limits.maxBytesPerSecond * elapsed ) / OneSecondUs );
        m_byteTokens = m_byteTokens + add > capacity ? capacity : m_byteTokens + add;
    }
}

/*-------------------------------------------------------------------------*/

void
CDRNLinkMonitor::RollSecond( UInt64 nodeNow )
{GUCEF_TRACE;

    if ( nodeNow < m_secondStart + OneSecondUs )
        return;

    m_packetsLastSecond = m_packetsThisSecond;
    m_messagesLastSecond = m_messagesThisSecond;
    m_bytesLastSecond = m_bytesThisSecond;
    if ( m_packetsThisSecond > m_peakPacketsPerSecond )
        m_peakPacketsPerSecond = m_packetsThisSecond;
    if ( m_messagesThisSecond > m_peakMessagesPerSecond )
        m_peakMessagesPerSecond = m_messagesThisSecond;
    if ( m_bytesThisSecond > m_peakBytesPerSecond )
        m_peakBytesPerSecond = m_bytesThisSecond;

    // a second without a limit hit, or a gap of more than a second, ends a run of seconds over the limits
    bool gap = nodeNow >= m_secondStart + 2 * OneSecondUs;
    if ( m_limitHitThisSecond && !gap )
        ++m_consecutiveLimitSeconds;
    else
    if ( m_limitHitThisSecond )
        m_consecutiveLimitSeconds = 1;
    else
        m_consecutiveLimitSeconds = 0;

    m_secondStart = gap ? nodeNow : m_secondStart + OneSecondUs;
    m_packetsThisSecond = 0;
    m_messagesThisSecond = 0;
    m_bytesThisSecond = 0;
    m_limitHitThisSecond = false;
    m_floodReportedThisSecond = false;
}

/*-------------------------------------------------------------------------*/

bool
CDRNLinkMonitor::MayReceive( UInt64 nodeNow, TDRNLinkAnomalyKind& exceeded )
{GUCEF_TRACE;

    exceeded = DRN_ANOMALY_NONE;
    Refill( nodeNow );
    if ( 0 != m_limits.maxMessagesPerSecond && m_messageTokens <= 0 )
    {
        exceeded = DRN_LIMIT_MESSAGES_PER_SECOND;
        return false;
    }
    if ( 0 != m_limits.maxBytesPerSecond && m_byteTokens <= 0 )
    {
        exceeded = DRN_LIMIT_BYTES_PER_SECOND;
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
CDRNLinkMonitor::OnReceived( UInt32 bytes, UInt32 messages )
{GUCEF_TRACE;

    // charged after the fact: a packet larger than what is left takes the bucket below zero, the peer waits it off
    if ( 0 != m_limits.maxMessagesPerSecond )
        m_messageTokens -= (Int64) messages;
    if ( 0 != m_limits.maxBytesPerSecond )
        m_byteTokens -= (Int64) bytes;
    m_messagesThisSecond += messages;
    m_messagesTaken += messages;
}

/*-------------------------------------------------------------------------*/

bool
CDRNLinkMonitor::OnLimitHit( TDRNLinkAnomalyKind kind, UInt64 nodeNow )
{GUCEF_TRACE;

    RollSecond( nodeNow );
    m_limitHitThisSecond = true;
    if ( kind <= DRN_ANOMALY_NONE || kind >= DRN_LINK_LIMIT_KIND_COUNT )
        return true;
    ++m_limitHits[ kind ];
    if ( 0 != m_lastLimitReport[ kind ] && nodeNow < m_lastLimitReport[ kind ] + OneSecondUs )
        return false;
    m_lastLimitReport[ kind ] = nodeNow;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CDRNLinkMonitor::OnPacketDropped( void )
{GUCEF_TRACE;

    ++m_droppedPackets;
}

/*-------------------------------------------------------------------------*/

bool
CDRNLinkMonitor::IsOverLimitsSustained( UInt64 nodeNow )
{GUCEF_TRACE;

    RollSecond( nodeNow );
    return 0 != m_limits.sustainedLimitSeconds && m_consecutiveLimitSeconds >= m_limits.sustainedLimitSeconds;
}

/*-------------------------------------------------------------------------*/

bool
CDRNLinkMonitor::OnPacketArrival( UInt64 nodeNow, UInt32 bytes, CDRNLinkAnomaly& anomaly )
{GUCEF_TRACE;

    RollSecond( nodeNow );
    ++m_packetsArrived;
    m_bytesArrived += bytes;
    ++m_packetsThisSecond;
    m_bytesThisSecond += bytes;

    bool found = false;
    if ( 0 != m_lastArrival && nodeNow >= m_lastArrival )
    {
        UInt64 interval = nodeNow - m_lastArrival;
        if ( interval > m_maxIntervalUs )
            m_maxIntervalUs = interval;
        Int64 deviation = (Int64) interval - m_meanIntervalUs;
        m_meanIntervalUs += deviation / 8;
        Int64 absDeviation = deviation < 0 ? -deviation : deviation;
        m_jitterUs += ( absDeviation - m_jitterUs ) / 16;

        // silence and then a burst: packets that were held back and let go at once
        if ( 0 != m_thresholds.silenceMs && 0 != m_thresholds.burstPackets && interval >= (UInt64) m_thresholds.silenceMs * 1000 )
        {
            m_burstWatch = true;
            m_burstWatchStart = nodeNow;
            m_burstSilenceUs = interval;
            m_burstCount = 1;
        }
        else
        if ( m_burstWatch )
        {
            if ( nodeNow - m_burstWatchStart <= (UInt64) m_thresholds.burstWindowMs * 1000 )
            {
                ++m_burstCount;
                if ( m_burstCount >= m_thresholds.burstPackets )
                {
                    m_burstWatch = false;
                    ++m_trafficAnomalies;
                    anomaly.kind = DRN_ANOMALY_BURST_AFTER_SILENCE;
                    anomaly.measured = (Float64) m_burstSilenceUs / 1000.0;
                    anomaly.threshold = (Float64) m_thresholds.silenceMs;
                    anomaly.nodeTime = nodeNow;
                    anomaly.text = "nothing for " + CORE::UInt64ToString( m_burstSilenceUs / 1000 ) + " ms, then " +
                                   CORE::UInt32ToString( m_burstCount ) + " packets within " + CORE::UInt32ToString( m_thresholds.burstWindowMs ) + " ms";
                    found = true;
                }
            }
            else
            {
                m_burstWatch = false;
            }
        }
    }
    m_lastArrival = nodeNow;

    if ( !found && 0 != m_thresholds.floodPacketsPerSecond && !m_floodReportedThisSecond && m_packetsThisSecond > m_thresholds.floodPacketsPerSecond )
    {
        m_floodReportedThisSecond = true;
        ++m_trafficAnomalies;
        anomaly.kind = DRN_ANOMALY_FLOOD;
        anomaly.measured = (Float64) m_packetsThisSecond;
        anomaly.threshold = (Float64) m_thresholds.floodPacketsPerSecond;
        anomaly.nodeTime = nodeNow;
        anomaly.text = "more than " + CORE::UInt32ToString( m_thresholds.floodPacketsPerSecond ) + " packets in a second";
        found = true;
    }
    return found;
}

/*-------------------------------------------------------------------------*/

bool
CDRNLinkMonitor::OnPacketTimed( UInt64 nodeNow, UInt64 localLinkTime, UInt64 peerSendTime, CDRNLinkAnomaly& anomaly )
{GUCEF_TRACE;

    if ( 0 == m_thresholds.clockDriftPercent || 0 == m_thresholds.clockWindowMs )
        return false;

    // sendTime minus our time: constant plus the delay of the packet for clocks that run at the same speed; the
    // least delayed packet of a window gives the best estimate, its trend over the windows is the drift
    Int64 offset = (Int64) peerSendTime - (Int64) localLinkTime;
    if ( !m_clockWindowActive )
    {
        m_clockWindowActive = true;
        m_clockWindowStart = nodeNow;
        m_clockBestOffset = offset;
        m_clockBestTime = nodeNow;
        return false;
    }
    if ( offset > m_clockBestOffset )
    {
        m_clockBestOffset = offset;
        m_clockBestTime = nodeNow;
    }
    if ( nodeNow < m_clockWindowStart + (UInt64) m_thresholds.clockWindowMs * 1000 )
        return false;

    // the window is done
    bool found = false;
    if ( m_clockHavePrevious && m_clockBestTime > m_clockPreviousTime )
    {
        Float64 span = (Float64) ( m_clockBestTime - m_clockPreviousTime );
        m_lastDriftPercent = ( (Float64) ( m_clockBestOffset - m_clockPreviousOffset ) / span ) * 100.0;
        Float64 absDrift = m_lastDriftPercent < 0 ? -m_lastDriftPercent : m_lastDriftPercent;
        if ( absDrift >= (Float64) m_thresholds.clockDriftPercent )
        {
            if ( !m_clockDrifting )
            {
                m_clockDrifting = true;
                m_clockDriftSinceUs = m_clockPreviousTime;
            }
            if ( nodeNow >= m_clockDriftSinceUs + (UInt64) m_thresholds.clockSustainMs * 1000 )
            {
                m_clockDrifting = false;
                ++m_clockAnomalies;
                anomaly.kind = DRN_ANOMALY_CLOCK_DRIFT;
                anomaly.measured = m_lastDriftPercent;
                anomaly.threshold = (Float64) m_thresholds.clockDriftPercent;
                anomaly.nodeTime = nodeNow;
                anomaly.text = "the peer's clock drifts " + CORE::DoubleToString( m_lastDriftPercent ) + " percent";
                found = true;
            }
        }
        else
        {
            m_clockDrifting = false;
        }
    }
    m_clockHavePrevious = true;
    m_clockPreviousOffset = m_clockBestOffset;
    m_clockPreviousTime = m_clockBestTime;
    m_clockWindowStart = nodeNow;
    m_clockBestOffset = offset;
    m_clockBestTime = nodeNow;
    return found;
}

/*-------------------------------------------------------------------------*/

void
CDRNLinkMonitor::GetTelemetry( CORE::CDataNode& node ) const
{GUCEF_TRACE;

    node.SetAttribute( "packetsArrived", m_packetsArrived );
    node.SetAttribute( "bytesArrived", m_bytesArrived );
    node.SetAttribute( "messagesTaken", m_messagesTaken );
    node.SetAttribute( "packetsLastSecond", m_packetsLastSecond );
    node.SetAttribute( "messagesLastSecond", m_messagesLastSecond );
    node.SetAttribute( "bytesLastSecond", m_bytesLastSecond );
    node.SetAttribute( "peakPacketsPerSecond", m_peakPacketsPerSecond );
    node.SetAttribute( "peakMessagesPerSecond", m_peakMessagesPerSecond );
    node.SetAttribute( "peakBytesPerSecond", m_peakBytesPerSecond );
    node.SetAttribute( "meanIntervalUs", (UInt64) ( m_meanIntervalUs > 0 ? m_meanIntervalUs : 0 ) );
    node.SetAttribute( "intervalJitterUs", (UInt64) ( m_jitterUs > 0 ? m_jitterUs : 0 ) );
    node.SetAttribute( "maxIntervalUs", m_maxIntervalUs );
    node.SetAttribute( "droppedForLimits", m_droppedPackets );
    node.SetAttribute( "limitHits", GetLimitHitCount() );
    for ( UInt32 kind=1; kind<DRN_LINK_LIMIT_KIND_COUNT; ++kind )
    {
        if ( m_limitHits[ kind ] > 0 )
            node.SetAttribute( "limitHits." + CDRNLinkAnomaly::KindToString( (TDRNLinkAnomalyKind) kind ), m_limitHits[ kind ] );
    }
    node.SetAttribute( "clockAnomalies", m_clockAnomalies );
    node.SetAttribute( "trafficAnomalies", m_trafficAnomalies );
    node.SetAttribute( "clockDriftPercent", CORE::CVariant( m_lastDriftPercent ) );
}

/*-------------------------------------------------------------------------*/

UInt64
CDRNLinkMonitor::GetLimitHitCount( void ) const
{GUCEF_TRACE;

    UInt64 total = 0;
    for ( UInt32 i=0; i<DRN_LINK_LIMIT_KIND_COUNT; ++i )
        total += m_limitHits[ i ];
    return total;
}

/*-------------------------------------------------------------------------*/

UInt64
CDRNLinkMonitor::GetLimitHitCount( TDRNLinkAnomalyKind kind ) const
{GUCEF_TRACE;

    if ( kind <= DRN_ANOMALY_NONE || kind >= DRN_LINK_LIMIT_KIND_COUNT )
        return 0;
    return m_limitHits[ kind ];
}

/*-------------------------------------------------------------------------*/

UInt64
CDRNLinkMonitor::GetDroppedPacketCount( void ) const
{GUCEF_TRACE;

    return m_droppedPackets;
}

/*-------------------------------------------------------------------------*/

UInt64
CDRNLinkMonitor::GetClockAnomalyCount( void ) const
{GUCEF_TRACE;

    return m_clockAnomalies;
}

/*-------------------------------------------------------------------------*/

UInt64
CDRNLinkMonitor::GetTrafficAnomalyCount( void ) const
{GUCEF_TRACE;

    return m_trafficAnomalies;
}

/*-------------------------------------------------------------------------*/

Float64
CDRNLinkMonitor::GetLastClockDriftPercent( void ) const
{GUCEF_TRACE;

    return m_lastDriftPercent;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
