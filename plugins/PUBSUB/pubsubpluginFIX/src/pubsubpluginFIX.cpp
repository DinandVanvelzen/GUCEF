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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_PUBSUB_CPUBSUBCLIENTFACTORY_H
#include "gucefPUBSUB_CPubSubClientFactory.h"
#define GUCEF_PUBSUB_CPUBSUBCLIENTFACTORY_H
#endif /* GUCEF_PUBSUB_CPUBSUBCLIENTFACTORY_H ? */

#ifndef GUCEF_PUBSUB_CPUBSUBGLOBAL_H
#include "gucefPUBSUB_CPubSubGlobal.h"
#define GUCEF_PUBSUB_CPUBSUBGLOBAL_H
#endif /* GUCEF_PUBSUB_CPUBSUBGLOBAL_H ? */

#ifndef PUBSUBPLUGIN_FIX_CFIXCLIENTPUBSUBCLIENT_H
#include "pubsubpluginFIX_CFIXClientPubSubClient.h"
#define PUBSUBPLUGIN_FIX_CFIXCLIENTPUBSUBCLIENT_H
#endif /* PUBSUBPLUGIN_FIX_CFIXCLIENTPUBSUBCLIENT_H ? */

#ifndef PUBSUBPLUGIN_FIX_CFIXSERVERPUBSUBCLIENT_H
#include "pubsubpluginFIX_CFIXServerPubSubClient.h"
#define PUBSUBPLUGIN_FIX_CFIXSERVERPUBSUBCLIENT_H
#endif /* PUBSUBPLUGIN_FIX_CFIXSERVERPUBSUBCLIENT_H ? */

#include "pubsubpluginFIX.h"

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
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

typedef CORE::CTFactoryWithParam< PUBSUB::CPubSubClient, CFIXClientPubSubClient, PUBSUB::CPubSubClientConfig, MT::CMutex >    TFIXClientPubSubClientFactory;
typedef CORE::CTFactoryWithParam< PUBSUB::CPubSubClient, CFIXServerPubSubClient, PUBSUB::CPubSubClientConfig, MT::CMutex >    TFIXServerPubSubClientFactory;

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

TFIXClientPubSubClientFactory g_fixClientPubSubClientFactory;
TFIXServerPubSubClientFactory g_fixServerPubSubClientFactory;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CORE::Int32 GUCEF_PLUGIN_CALLSPEC_PREFIX
GUCEFPlugin_Load( CORE::UInt32 argc, const char** argv ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "GUCEFPlugin_Load: Load called on PUBSUB plugin FIX" );

    PUBSUB::CPubSubGlobal::Instance()->GetPubSubClientFactory().RegisterConcreteFactory( CFIXClientPubSubClient::TypeName, &g_fixClientPubSubClientFactory );
    PUBSUB::CPubSubGlobal::Instance()->GetPubSubClientFactory().RegisterConcreteFactory( CFIXServerPubSubClient::TypeName, &g_fixServerPubSubClientFactory );

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "GUCEFPlugin_Load: Load finished for PUBSUB plugin FIX" );
    return 1;
}

/*--------------------------------------------------------------------------*/

void GUCEF_PLUGIN_CALLSPEC_PREFIX
GUCEFPlugin_Unload( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "GUCEFPlugin_Unload: Unload called on PUBSUB plugin FIX" );

    PUBSUB::CPubSubGlobal::Instance()->GetPubSubClientFactory().UnregisterConcreteFactory( CFIXClientPubSubClient::TypeName );
    PUBSUB::CPubSubGlobal::Instance()->GetPubSubClientFactory().UnregisterConcreteFactory( CFIXServerPubSubClient::TypeName );

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "GUCEFPlugin_Unload: Unload finished for PUBSUB plugin FIX" );
}

/*--------------------------------------------------------------------------*/

void GUCEF_PLUGIN_CALLSPEC_PREFIX
GUCEFPlugin_GetVersion( CORE::TVersion* versionInfo ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    if ( GUCEF_NULL != versionInfo )
    {
        versionInfo->major   = 0;
        versionInfo->minor   = 1;
        versionInfo->patch   = 0;
        versionInfo->release = 0;
    }
}

/*--------------------------------------------------------------------------*/

const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
GUCEFPlugin_GetCopyright( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    return "Copyright (C) Dinand Vanvelzen, Apache License v2";
}

/*--------------------------------------------------------------------------*/

const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
GUCEFPlugin_GetDescription( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    return "Generic GUCEF plugin for PUBSUB via FIX protocol (FIX 4.2 / 4.4 / FIXT 1.1)";
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace FIX */
}; /* namespace PUBSUBPLUGIN */
}; /* namespace GUCEF */

/*--------------------------------------------------------------------------*/
