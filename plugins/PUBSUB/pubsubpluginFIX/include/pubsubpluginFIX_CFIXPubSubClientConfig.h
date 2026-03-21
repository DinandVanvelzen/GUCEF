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

#ifndef PUBSUBPLUGIN_FIX_CFIXPUBSUBCLIENTCONFIG_H
#define PUBSUBPLUGIN_FIX_CFIXPUBSUBCLIENTCONFIG_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_PUBSUB_CPUBSUBCLIENTCONFIG_H
#include "gucefPUBSUB_CPubSubClientConfig.h"
#define GUCEF_PUBSUB_CPUBSUBCLIENTCONFIG_H
#endif /* GUCEF_PUBSUB_CPUBSUBCLIENTCONFIG_H ? */

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

class CFIXPubSubClient;

/**
 *  Standard pub-sub client config with FIX-protocol-specific settings added.
 *
 *  Remote host/port is taken from the base class remoteAddresses (first entry).
 */
class PUBSUBPLUGIN_FIX_PLUGIN_PRIVATE_CPP CFIXPubSubClientConfig : public PUBSUB::CPubSubClientConfig
{
    public:

    CFIXPubSubClientConfig( void );

    CFIXPubSubClientConfig( const PUBSUB::CPubSubClientConfig& genericConfig );

    virtual ~CFIXPubSubClientConfig() GUCEF_VIRTUAL_OVERRIDE;

    CFIXPubSubClientConfig& operator=( const PUBSUB::CPubSubClientConfig& src );

    CFIXPubSubClientConfig& operator=( const CFIXPubSubClientConfig& src );

    bool LoadCustomConfig( const CORE::CDataNode& config );

    bool SaveCustomConfig( CORE::CDataNode& config ) const;

    bool SerializeCustomConfigToGenericConfig( void );

    bool LoadConfig( const CORE::CDataNode& config );

    bool SaveConfig( CORE::CDataNode& config ) const;

    bool LoadConfig( const PUBSUB::CPubSubClientConfig& cfg );

    bool SaveConfig( PUBSUB::CPubSubClientConfig& cfg ) const;

    // FIX session identity
    CORE::CString senderCompId;            // FIX tag 49 - required
    CORE::CString targetCompId;            // FIX tag 56 - required
    CORE::CString fixVersion;              // "FIX.4.2" | "FIX.4.4" | "FIXT.1.1"

    // Session behaviour
    CORE::UInt32 heartbeatIntervalSecs;    // FIX tag 108 in Logon, default=30
    bool         resetSeqNumOnLogon;       // send tag 141=Y on logon, default=false
    CORE::UInt32 logonTimeoutInMs;         // milliseconds before reconnect if no logon reply, default=10000
    bool         useSsl;                   // TLS (stretch goal), default=false
    bool         allowPublishing;          // allow sending FIX msgs via Publish(), default=false
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

#endif /* PUBSUBPLUGIN_FIX_CFIXPUBSUBCLIENTCONFIG_H ? */
