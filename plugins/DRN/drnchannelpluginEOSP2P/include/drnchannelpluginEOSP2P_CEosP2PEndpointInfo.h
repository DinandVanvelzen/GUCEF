/*
 *  drnchannelpluginEOSP2P: DRN channel plugin that moves DRN packets over the Epic Online Services P2P interface
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

#ifndef GUCEF_DRNPLUGIN_EOSP2P_CEOSP2PENDPOINTINFO_H
#define GUCEF_DRNPLUGIN_EOSP2P_CEOSP2PENDPOINTINFO_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_DRN_CDRNENDPOINT_H
#include "gucefDRN_CDRNEndpoint.h"
#define GUCEF_DRN_CDRNENDPOINT_H
#endif /* GUCEF_DRN_CDRNENDPOINT_H ? */

#ifndef GUCEF_PLUGINGLUE_EOS_EOSSHIM_H
#include "pluginglueEOS_EosShim.h"
#define GUCEF_PLUGINGLUE_EOS_EOSSHIM_H
#endif /* GUCEF_PLUGINGLUE_EOS_EOSSHIM_H ? */

#ifndef GUCEF_DRNPLUGIN_EOSP2P_MACROS_H
#include "drnchannelpluginEOSP2P_macros.h"
#define GUCEF_DRNPLUGIN_EOSP2P_MACROS_H
#endif /* GUCEF_DRNPLUGIN_EOSP2P_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRNPLUGIN {
namespace EOSP2P {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The endpoint of the "eos-p2p" channel: eos-p2p://<Product User Id>/<socket name>[?parameters]
 *
 *  A connecting endpoint names the peer (its Product User Id, the text of EOS_ProductUserId_ToString) and the socket
 *  id of the P2P interface to reach it on. A listening endpoint names the local user ("*" or empty: the user that is
 *  logged in) and the socket. The socket name is 1 to 32 letters, digits, '-' and '_'.
 *
 *  Parameters:
 *    local               the Product User Id to talk as, for a process with more than one user; default: the one logged in
 *    reliability         unreliable (default), reliable or ordered: the transport mode of the SDK. DRN has its own
 *                        reliability on a datagram channel; the channel is a datagram channel in every mode
 *    maxPacketSize       the budget of a packet, at most 1170 (default)
 *    connectTimeoutMs    a connection that is not established by then ends the channel (default 15000)
 *    tickIntervalInMs    how often the platform is ticked while the channel is open (default 10)
 *    relay               allow (default), none or force: the relay control of the SDK, which is a setting of the platform
 *  Listening only:
 *    acceptAny           1: every peer that asks is accepted. Without it only the peers on the allow list of the
 *                        socket are (CEosP2PSocket::SetAllowedRemotes), so a socket nobody has put a list on takes nobody
 *    allow               a ',' separated list of Product User Ids to start the allow list with
 *    maxConnections      the most connections at once (default 64, 0 for no limit)
 */
class GUCEF_DRNPLUGIN_EOSP2P_EXPORT_CPP CEosP2PEndpointInfo
{
    public:

    static const CORE::CString ChannelType;         /**< "eos-p2p" */

    static const UInt32 DefaultConnectTimeoutInMs = 15000;
    static const UInt32 DefaultTickIntervalInMs = 10;
    static const UInt32 DefaultMaxConnections = 64;

    CORE::CString remoteUser;
    CORE::CString localUser;
    CORE::CString socketName;
    PLUGINGLUE::EOS::EEosPacketReliability reliability;
    UInt32 maxPacketSize;
    UInt32 connectTimeoutInMs;
    UInt32 tickIntervalInMs;
    bool hasRelayControl;
    PLUGINGLUE::EOS::EEosRelayControl relayControl;
    bool acceptAny;
    CORE::CString::StringSet allowedRemotes;
    UInt32 maxConnections;

    CEosP2PEndpointInfo( void );

    /** Reads an endpoint to connect to or to listen on; false with the reason when it is not one */
    bool Parse( const DRN::CDRNEndpoint& endpoint, bool forListening, CORE::CString& reason );

    /** The URI of a peer's socket: eos-p2p://<puid>/<socket> */
    static DRN::CDRNEndpoint MakeEndpoint( const CORE::CString& productUserId, const CORE::CString& socketName );

    /** Whether the text can be a Product User Id: 1 to 63 letters and digits */
    static bool IsValidUserId( const CORE::CString& text );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace EOSP2P */
}; /* namespace DRNPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRNPLUGIN_EOSP2P_CEOSP2PENDPOINTINFO_H ? */
