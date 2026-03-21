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

#ifndef PUBSUBPLUGIN_FIX_CFIXSERVERPUBSUBCLIENTTOPICCONFIG_H
#define PUBSUBPLUGIN_FIX_CFIXSERVERPUBSUBCLIENTTOPICCONFIG_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_CTSHAREDPTR_H
#include "CTSharedPtr.h"
#define GUCEF_CORE_CTSHAREDPTR_H
#endif /* GUCEF_CORE_CTSHAREDPTR_H ? */

#ifndef GUCEF_PUBSUB_CPUBSUBCLIENTTOPICCONFIG_H
#include "gucefPUBSUB_CPubSubClientTopicConfig.h"
#define GUCEF_PUBSUB_CPUBSUBCLIENTTOPICCONFIG_H
#endif /* GUCEF_PUBSUB_CPUBSUBCLIENTTOPICCONFIG_H ? */

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

/**
 *  Per-topic configuration for the FIX server pubsub backend.
 *  Each topic corresponds to one logical FIX session (one remote SenderCompID).
 */
class PUBSUBPLUGIN_FIX_PLUGIN_PRIVATE_CPP CFIXServerPubSubClientTopicConfig
    : public PUBSUB::CPubSubClientTopicConfig ,
      public CORE::CTSharedObjCreator< CFIXServerPubSubClientTopicConfig, MT::CMutex >
{
    public:

    CFIXServerPubSubClientTopicConfig( void );

    CFIXServerPubSubClientTopicConfig( const CFIXServerPubSubClientTopicConfig& src );

    CFIXServerPubSubClientTopicConfig( const PUBSUB::CPubSubClientTopicConfig& genericConfig );

    virtual ~CFIXServerPubSubClientTopicConfig() GUCEF_VIRTUAL_OVERRIDE;

    CFIXServerPubSubClientTopicConfig& operator=( const PUBSUB::CPubSubClientTopicConfig& src );

    CFIXServerPubSubClientTopicConfig& operator=( const CFIXServerPubSubClientTopicConfig& src );

    bool LoadCustomConfig( const CORE::CDataNode& config );

    bool SaveCustomConfig( CORE::CDataNode& config ) const;

    bool LoadConfig( const PUBSUB::CPubSubClientTopicConfig& src );

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    // Whether to pass FIX session-level messages (Heartbeat, Logon, etc.) to subscribers
    bool               includeSessionLevelMsgs;   // default false
    // Comma-separated MsgTypes to pass through; empty = pass all
    CORE::CAsciiString msgTypeFilter;
    // Expected remote SenderCompID; empty = accept any
    CORE::CAsciiString remoteCompId;
};

/*-------------------------------------------------------------------------*/

typedef CFIXServerPubSubClientTopicConfig::TSharedPtrType   CFIXServerPubSubClientTopicConfigPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace FIX */
}; /* namespace PUBSUBPLUGIN */
}; /* namespace GUCEF */

/*--------------------------------------------------------------------------*/

#endif /* PUBSUBPLUGIN_FIX_CFIXSERVERPUBSUBCLIENTTOPICCONFIG_H ? */
