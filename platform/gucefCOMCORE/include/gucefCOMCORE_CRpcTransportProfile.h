/*
 *  gucefCOMCORE: GUCEF module providing basic communication facilities
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

#ifndef GUCEF_COMCORE_CRPCTRANSPORTPROFILE_H
#define GUCEF_COMCORE_CRPCTRANSPORTPROFILE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_COMCORE_MACROS_H
#include "gucefCOMCORE_macros.h"
#define GUCEF_COMCORE_MACROS_H
#endif /* GUCEF_COMCORE_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COMCORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The timing of a transport family (gucefCOMCORE RPC design 13). A transport reports its profile name; endpoint
 *  settings may override any value. 0 for a time means "none" (no keepalive, immediate ack).
 */
class GUCEF_COMCORE_EXPORT_CPP CRpcTransportProfile
{
    public:

    CORE::CString name;
    UInt32 maxFrameSize;                /**< default frame size limit of the family */
    UInt32 ackDelayMs;                  /**< standalone ACK after one-way traffic; 0 = at the next pulse */
    UInt32 ackEveryFrames;              /**< standalone ACK after this many unacknowledged deliveries */
    UInt32 initialRtoMs;                /**< unreliable transports: first retransmission timeout */
    UInt32 minRtoMs;
    UInt32 maxRtoMs;
    UInt32 dupThreshold;                /**< SACKed later frames that trigger a fast retransmit */
    UInt32 maxBurstFrames;              /**< retransmissions per timeout */
    UInt32 keepaliveIntervalMs;         /**< PING when idle; 0 = none */
    UInt32 keepaliveTimeoutMs;          /**< no PONG within this closes the transport */
    UInt32 resumeGraceMs;               /**< how long a suspended session is kept */
    UInt32 helloRetryMs;                /**< unreliable transports: HELLO is sent again after this */

    CRpcTransportProfile( void );

    /** The built-in profile of a family: loopback, tcp, drn, websocket, http, udp, mail, quic; tcp values otherwise */
    static CRpcTransportProfile GetProfile( const CORE::CString& profileName );

    /** Every time divided by the divisor (at least 1 ms where it was not 0): runs hours as milliseconds in tests */
    CRpcTransportProfile Scaled( UInt32 divisor ) const;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMCORE_CRPCTRANSPORTPROFILE_H ? */
