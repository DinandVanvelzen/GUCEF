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

#ifndef PUBSUBPLUGIN_FIX_CFIXCLIENTPUBSUBCLIENTCONFIG_H
#define PUBSUBPLUGIN_FIX_CFIXCLIENTPUBSUBCLIENTCONFIG_H

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

class CFIXClientPubSubClient;

/**
 *  Standard pub-sub client config with FIX-protocol-specific settings added.
 *
 *  Remote host/port is taken from the base class remoteAddresses (first entry).
 */
class PUBSUBPLUGIN_FIX_PLUGIN_PRIVATE_CPP CFIXClientPubSubClientConfig : public PUBSUB::CPubSubClientConfig
{
    public:

    CFIXClientPubSubClientConfig( void );

    CFIXClientPubSubClientConfig( const PUBSUB::CPubSubClientConfig& genericConfig );

    virtual ~CFIXClientPubSubClientConfig() GUCEF_VIRTUAL_OVERRIDE;

    CFIXClientPubSubClientConfig& operator=( const PUBSUB::CPubSubClientConfig& src );

    CFIXClientPubSubClientConfig& operator=( const CFIXClientPubSubClientConfig& src );

    bool LoadCustomConfig( const CORE::CDataNode& config );

    bool SaveCustomConfig( CORE::CDataNode& config ) const;

    bool SerializeCustomConfigToGenericConfig( void );

    bool LoadConfig( const CORE::CDataNode& config );

    bool SaveConfig( CORE::CDataNode& config ) const;

    bool LoadConfig( const PUBSUB::CPubSubClientConfig& cfg );

    bool SaveConfig( PUBSUB::CPubSubClientConfig& cfg ) const;

    // FIX session identity
    CORE::CAsciiString senderCompId;            // FIX tag 49 - required
    CORE::CAsciiString targetCompId;            // FIX tag 56 - required
    CORE::CAsciiString fixVersion;              // "FIX.4.2" | "FIX.4.4" | "FIXT.1.1"

    // Session behaviour
    CORE::UInt32 heartbeatIntervalSecs;             // FIX tag 108 in Logon, default=30
    bool         resetSeqNumOnLogon;                // send tag 141=Y on logon, default=false
    CORE::UInt32 logonTimeoutInMs;                  // milliseconds before reconnect if no logon reply, default=10000
    bool         useSsl;                            // TLS (stretch goal), default=false
    bool         allowPublishing;                   // allow sending FIX msgs via Publish(), default=false

    // Security / framing settings
    CORE::UInt32 maxMsgSizeBytes;                   // max BodyLength accepted [S1], default=65536
    bool         disableChecksumValidation;         // disable FIX checksum check [S7], default=false
    CORE::UInt32 maxConsecutiveChecksumFailures;    // disconnect after N consecutive bad checksums, default=3
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

#endif /* PUBSUBPLUGIN_FIX_CFIXCLIENTPUBSUBCLIENTCONFIG_H ? */
