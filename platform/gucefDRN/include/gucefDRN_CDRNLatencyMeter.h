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

#ifndef GUCEF_DRN_CDRNLATENCYMETER_H
#define GUCEF_DRN_CDRNLATENCYMETER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_DRN_CIDRNLATENCYMETER_H
#include "gucefDRN_CIDRNLatencyMeter.h"
#define GUCEF_DRN_CIDRNLATENCYMETER_H
#endif /* GUCEF_DRN_CIDRNLATENCYMETER_H ? */

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
 *  The default latency meter of a link: works from the packet header on every channel, without extra traffic.
 *
 *  RTT of a packet = now - echoTime - echoHold, taken whenever a packet acknowledges a newer sequence than before.
 *  Clock offset of a sample = peerSendTime + rtt/2 - localReceiveTime. The estimate is the offset of the sample with
 *  the lowest RTT among the last 8 (the least delayed one), smoothed with an EWMA of 1/4.
 */
class GUCEF_DRN_EXPORT_CPP CDRNLatencyMeter : public CIDRNLatencyMeter
{
    public:

    CDRNLatencyMeter( void );

    virtual ~CDRNLatencyMeter();

    virtual void OnPacketSent( UInt32 sequence, UInt64 localSendTime, UInt32 packetSize ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void OnPacketReceived( const CDRNPacketHeader& header, UInt64 localReceiveTime, UInt32 lostBefore ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void GetLatencyInfo( CDRNLatencyInfo& info ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual void Reset( void ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    static const UInt32 OffsetSampleCount = 8;
    static const UInt32 MinRttBucketCount = 10;
    static const UInt32 LossHistorySize = 256;

    void AddRttSample( UInt64 rtt, UInt64 peerSendTime, UInt64 localReceiveTime );
    void AddLossHistory( bool received );

    CDRNLatencyInfo m_info;
    bool m_hasAck;
    UInt32 m_highestAck;
    Int64 m_offsetRtt[ OffsetSampleCount ];
    Int64 m_offsetValue[ OffsetSampleCount ];
    UInt32 m_offsetSamples;
    UInt32 m_offsetNext;
    UInt64 m_minRttBucketSecond[ MinRttBucketCount ];
    UInt32 m_minRttBucketValue[ MinRttBucketCount ];
    UInt8 m_lossHistory[ LossHistorySize ];
    UInt32 m_lossHistoryNext;
    UInt32 m_lossHistoryFill;
    UInt32 m_lossHistoryLost;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRN_CDRNLATENCYMETER_H ? */
