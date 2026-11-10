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

#ifndef GUCEF_DRN_CIDRNLATENCYMETER_H
#define GUCEF_DRN_CIDRNLATENCYMETER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_DRN_CDRNLATENCYINFO_H
#include "gucefDRN_CDRNLatencyInfo.h"
#define GUCEF_DRN_CDRNLATENCYINFO_H
#endif /* GUCEF_DRN_CDRNLATENCYINFO_H ? */

#ifndef GUCEF_DRN_CDRNPACKETCODEC_H
#include "gucefDRN_CDRNPacketCodec.h"
#define GUCEF_DRN_CDRNPACKETCODEC_H
#endif /* GUCEF_DRN_CDRNPACKETCODEC_H ? */

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
 *  Measures the latency of a link. The link feeds it every packet it sends and receives.
 *  A channel can supply a meter of its own (for example one reporting the figures of a networking SDK),
 *  otherwise the link uses CDRNLatencyMeter which works from the packet header.
 *  All times are microseconds of the local link clock.
 */
class GUCEF_DRN_EXPORT_CPP CIDRNLatencyMeter
{
    public:

    CIDRNLatencyMeter( void );

    virtual ~CIDRNLatencyMeter();

    virtual void OnPacketSent( UInt32 sequence, UInt64 localSendTime, UInt32 packetSize ) = 0;

    /**
     *  @param header the header of a packet that was accepted (not a duplicate)
     *  @param localReceiveTime when it arrived
     *  @param lostBefore how many sequences were skipped right before it (gap to the previous highest)
     */
    virtual void OnPacketReceived( const CDRNPacketHeader& header, UInt64 localReceiveTime, UInt32 lostBefore ) = 0;

    virtual void GetLatencyInfo( CDRNLatencyInfo& info ) const = 0;

    virtual void Reset( void ) = 0;

    private:

    CIDRNLatencyMeter( const CIDRNLatencyMeter& src );
    CIDRNLatencyMeter& operator=( const CIDRNLatencyMeter& src );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRN_CIDRNLATENCYMETER_H ? */
