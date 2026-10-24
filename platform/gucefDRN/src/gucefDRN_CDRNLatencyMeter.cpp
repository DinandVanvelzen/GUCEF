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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "gucefDRN_CDRNLatencyMeter.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRN {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CDRNLatencyMeter::CDRNLatencyMeter( void )
    : CIDRNLatencyMeter()
{GUCEF_TRACE;

    Reset();
}

/*-------------------------------------------------------------------------*/

CDRNLatencyMeter::~CDRNLatencyMeter()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CDRNLatencyMeter::Reset( void )
{GUCEF_TRACE;

    m_info.Clear();
    m_hasAck = false;
    m_highestAck = 0;
    m_offsetSamples = 0;
    m_offsetNext = 0;
    for ( UInt32 i=0; i<OffsetSampleCount; ++i )
    {
        m_offsetRtt[ i ] = 0;
        m_offsetValue[ i ] = 0;
    }
    for ( UInt32 i=0; i<MinRttBucketCount; ++i )
    {
        m_minRttBucketSecond[ i ] = GUCEF_UINT64MAX;
        m_minRttBucketValue[ i ] = 0;
    }
    for ( UInt32 i=0; i<LossHistorySize; ++i )
        m_lossHistory[ i ] = 0;
    m_lossHistoryNext = 0;
    m_lossHistoryFill = 0;
    m_lossHistoryLost = 0;
}

/*-------------------------------------------------------------------------*/

void
CDRNLatencyMeter::OnPacketSent( UInt32 /* sequence */, UInt64 /* localSendTime */, UInt32 /* packetSize */ )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CDRNLatencyMeter::AddLossHistory( bool received )
{GUCEF_TRACE;
    UInt8 lost = received ? 0 : 1;
    if ( m_lossHistoryFill == LossHistorySize )
        m_lossHistoryLost -= m_lossHistory[ m_lossHistoryNext ];
    else
        ++m_lossHistoryFill;
    m_lossHistory[ m_lossHistoryNext ] = lost;
    m_lossHistoryLost += lost;
    m_lossHistoryNext = ( m_lossHistoryNext + 1 ) % LossHistorySize;
}

/*-------------------------------------------------------------------------*/

void
CDRNLatencyMeter::OnPacketReceived( const CDRNPacketHeader& header, UInt64 localReceiveTime, UInt32 lostBefore )
{GUCEF_TRACE;
    if ( lostBefore > LossHistorySize )
        lostBefore = LossHistorySize;
    for ( UInt32 i=0; i<lostBefore; ++i )
        AddLossHistory( false );
    AddLossHistory( true );
    m_info.lossRatio = m_lossHistoryFill > 0 ? (Float32) m_lossHistoryLost / (Float32) m_lossHistoryFill : 0.0f;

    if ( 0 == header.ackSequence )
        return;
    if ( m_hasAck && !DRNIsNewerSequence( header.ackSequence, m_highestAck ) )
        return;
    m_hasAck = true;
    m_highestAck = header.ackSequence;

    // the round trip as seen by the original sender, without the time the peer held on to the packet
    Int64 rtt = (Int64) localReceiveTime - (Int64) header.echoTime - (Int64) header.echoHold;
    if ( rtt < 0 )
        rtt = 0;
    AddRttSample( (UInt64) rtt, header.sendTime, localReceiveTime );
}

/*-------------------------------------------------------------------------*/

void
CDRNLatencyMeter::AddRttSample( UInt64 rtt, UInt64 peerSendTime, UInt64 localReceiveTime )
{GUCEF_TRACE;
    UInt32 rtt32 = rtt > 0xFFFFFFFF ? 0xFFFFFFFF : (UInt32) rtt;
    if ( 0 == m_info.sampleCount )
    {
        m_info.smoothedRtt = rtt32;
        m_info.rttVariance = rtt32 / 2;
    }
    else
    {
        Int64 deviation = (Int64) m_info.smoothedRtt - (Int64) rtt32;
        if ( deviation < 0 )
            deviation = -deviation;
        m_info.rttVariance = (UInt32) ( ( 3 * (Int64) m_info.rttVariance + deviation ) / 4 );
        m_info.smoothedRtt = (UInt32) ( ( 7 * (Int64) m_info.smoothedRtt + (Int64) rtt32 ) / 8 );
    }
    m_info.lastRtt = rtt32;
    ++m_info.sampleCount;
    m_info.sampleTime = localReceiveTime;

    // minimum over the last 10 seconds in buckets of one second
    UInt64 second = localReceiveTime / 1000000;
    UInt32 bucket = (UInt32) ( second % MinRttBucketCount );
    if ( m_minRttBucketSecond[ bucket ] != second || rtt32 < m_minRttBucketValue[ bucket ] )
    {
        if ( m_minRttBucketSecond[ bucket ] != second )
            m_minRttBucketValue[ bucket ] = rtt32;
        else
            m_minRttBucketValue[ bucket ] = rtt32 < m_minRttBucketValue[ bucket ] ? rtt32 : m_minRttBucketValue[ bucket ];
        m_minRttBucketSecond[ bucket ] = second;
    }
    UInt32 minRtt = rtt32;
    for ( UInt32 i=0; i<MinRttBucketCount; ++i )
    {
        if ( GUCEF_UINT64MAX != m_minRttBucketSecond[ i ] && second - m_minRttBucketSecond[ i ] < MinRttBucketCount )
        {
            if ( m_minRttBucketValue[ i ] < minRtt )
                minRtt = m_minRttBucketValue[ i ];
        }
    }
    m_info.minRtt = minRtt;

    // clock offset: the least delayed of the last samples gives the best estimate
    Int64 offset = (Int64) peerSendTime + (Int64) ( rtt / 2 ) - (Int64) localReceiveTime;
    m_offsetRtt[ m_offsetNext ] = (Int64) rtt;
    m_offsetValue[ m_offsetNext ] = offset;
    m_offsetNext = ( m_offsetNext + 1 ) % OffsetSampleCount;
    if ( m_offsetSamples < OffsetSampleCount )
        ++m_offsetSamples;

    UInt32 best = 0;
    for ( UInt32 i=1; i<m_offsetSamples; ++i )
    {
        if ( m_offsetRtt[ i ] < m_offsetRtt[ best ] )
            best = i;
    }
    Int64 bestOffset = m_offsetValue[ best ];
    if ( !m_info.hasClockOffset )
    {
        m_info.clockOffset = bestOffset;
        m_info.hasClockOffset = true;
    }
    else
    {
        m_info.clockOffset += ( bestOffset - m_info.clockOffset ) / 4;
    }
}

/*-------------------------------------------------------------------------*/

void
CDRNLatencyMeter::GetLatencyInfo( CDRNLatencyInfo& info ) const
{GUCEF_TRACE;
    info = m_info;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
