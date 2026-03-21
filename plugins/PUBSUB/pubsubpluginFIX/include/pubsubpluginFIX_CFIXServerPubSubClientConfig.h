/*
 *  pubsubpluginFIX: Generic GUCEF COMCORE plugin for providing pubsub via FIX protocol
 *
 *  Copyright (C) 1998 - 2024.  Dinand Vanvelzen
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

#ifndef PUBSUBPLUGIN_FIX_CFIXSERVERPUBSUBCLIENTCONFIG_H
#define PUBSUBPLUGIN_FIX_CFIXSERVERPUBSUBCLIENTCONFIG_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_PUBSUB_CPUBSUBCLIENTCONFIG_H
#include "gucefPUBSUB_CPubSubClientConfig.h"
#define GUCEF_PUBSUB_CPUBSUBCLIENTCONFIG_H
#endif /* GUCEF_PUBSUB_CPUBSUBCLIENTCONFIG_H ? */

#ifndef GUCEF_CORE_CASCIISTRING_H
#include "gucefCORE_CAsciiString.h"
#define GUCEF_CORE_CASCIISTRING_H
#endif /* GUCEF_CORE_CASCIISTRING_H ? */

#ifndef PUBSUBPLUGIN_FIX_MACROS_H
#include "pubsubpluginFIX_macros.h"
#define PUBSUBPLUGIN_FIX_MACROS_H
#endif /* PUBSUBPLUGIN_FIX_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace PUBSUBPLUGIN {
namespace FIX {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CFIXServerPubSubClient;

/**
 *  Standard pub-sub client config with FIX-server-specific settings added.
 *
 *  The server listens for incoming FIX TCP connections and creates one topic
 *  per unique remote SenderCompID.
 */
class PUBSUBPLUGIN_FIX_PLUGIN_PRIVATE_CPP CFIXServerPubSubClientConfig : public PUBSUB::CPubSubClientConfig
{
    public:

    CFIXServerPubSubClientConfig( void );

    CFIXServerPubSubClientConfig( const PUBSUB::CPubSubClientConfig& genericConfig );

    virtual ~CFIXServerPubSubClientConfig() GUCEF_VIRTUAL_OVERRIDE;

    CFIXServerPubSubClientConfig& operator=( const PUBSUB::CPubSubClientConfig& src );

    CFIXServerPubSubClientConfig& operator=( const CFIXServerPubSubClientConfig& src );

    bool LoadCustomConfig( const CORE::CDataNode& config );

    bool SaveCustomConfig( CORE::CDataNode& config ) const;

    bool SerializeCustomConfigToGenericConfig( void );

    bool LoadConfig( const CORE::CDataNode& config );

    bool SaveConfig( CORE::CDataNode& config ) const;

    bool LoadConfig( const PUBSUB::CPubSubClientConfig& cfg );

    bool SaveConfig( PUBSUB::CPubSubClientConfig& cfg ) const;

    // FIX session identity (server's own IDs for outgoing messages)
    CORE::CAsciiString senderCompId;            // Server's own SenderCompID (tag 49 in outgoing msgs)
    CORE::CAsciiString fixVersion;              // "FIX.4.2" | "FIX.4.4" | "FIXT.1.1"

    // Listen settings
    CORE::UInt16       listenPort;              // TCP port to listen on, default 9878
    CORE::CAsciiString bindAddress;             // Bind IP, empty = all interfaces

    // Session behaviour
    CORE::UInt32       heartbeatIntervalSecs;   // FIX tag 108 in Logon reply, default 30
    CORE::UInt32       logonTimeoutInMs;        // Close pending conn if no Logon arrives, default 10000
    bool               allowPublishing;         // Allow Publish() to send FIX msgs, default false

    // Security / framing settings
    CORE::UInt32       maxMsgSizeBytes;                   // max BodyLength accepted [S1], default 65536
    bool               disableChecksumValidation;         // disable FIX checksum check [S7], default false
    CORE::UInt32       maxConsecutiveChecksumFailures;    // disconnect after N consecutive bad checksums, default 3
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace FIX */
}; /* namespace PUBSUBPLUGIN */
}; /* namespace GUCEF */

/*--------------------------------------------------------------------------*/

#endif /* PUBSUBPLUGIN_FIX_CFIXSERVERPUBSUBCLIENTCONFIG_H ? */
