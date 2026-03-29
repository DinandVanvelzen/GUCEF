/*
 *  pubsubpluginTEST: Generic GUCEF PUBSUB plugin for adding integration/system tests
 *
 *  Copyright (C) 1998 - 2020.  Dinand Vanvelzen
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

#include "pubsubpluginTEST.h"

#ifndef GUCEF_PUBSUB_CPUBSUBCLIENTFACTORY_H
#include "gucefPUBSUB_CPubSubClientFactory.h"
#define GUCEF_PUBSUB_CPUBSUBCLIENTFACTORY_H
#endif /* GUCEF_PUBSUB_CPUBSUBCLIENTFACTORY_H ? */

#ifndef GUCEF_PUBSUB_CPUBSUBGLOBAL_H
#include "gucefPUBSUB_CPubSubGlobal.h"
#define GUCEF_PUBSUB_CPUBSUBGLOBAL_H
#endif /* GUCEF_PUBSUB_CPUBSUBGLOBAL_H ? */

#ifndef PUBSUBPLUGIN_TEST_CTESTPUBSUBCLIENT_H
#include "pubsubpluginTEST_CTestPubSubClient.h"
#define PUBSUBPLUGIN_TEST_CTESTPUBSUBCLIENT_H
#endif /* PUBSUBPLUGIN_TEST_CTESTPUBSUBCLIENT_H ? */

#ifndef PUBSUBPLUGIN_STORAGE_H
#include "pubsubpluginSTORAGE.h"
#define PUBSUBPLUGIN_STORAGE_H
#endif /* PUBSUBPLUGIN_STORAGE_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

typedef CORE::CTFactoryWithParam< PUBSUB::CPubSubClient, PUBSUBPLUGIN::TEST::CTestPubSubClient, PUBSUB::CPubSubClientConfig, MT::CMutex >    TTestPubSubClientFactory;

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

TTestPubSubClientFactory g_testPubSubClientFactory;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/
              int  foo () { return 0; }
CORE::Int32 PUBSUB_TEST_PLUGIN_CALLSPEC_PREFIX
pubsubpluginTEST_GUCEFPlugin_Load( CORE::UInt32 argc, const char** argv ) PUBSUB_TEST_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Load called on PUBSUB plugin TEST" );

    pubsubpluginSTORAGE_GUCEFPlugin_Load( argc, argv );

    PUBSUB::CPubSubGlobal::Instance()->GetPubSubClientFactory().RegisterConcreteFactory( PUBSUBPLUGIN::TEST::CTestPubSubClient::TypeName, &g_testPubSubClientFactory );

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Load finished for PUBSUB plugin TEST" );
    return 1;
}

/*--------------------------------------------------------------------------*/

void PUBSUB_TEST_PLUGIN_CALLSPEC_PREFIX
pubsubpluginTEST_GUCEFPlugin_Unload( void ) PUBSUB_TEST_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Unload called on PUBSUB plugin TEST" );

    PUBSUB::CPubSubGlobal::Instance()->GetPubSubClientFactory().UnregisterConcreteFactory( PUBSUBPLUGIN::TEST::CTestPubSubClient::TypeName );

    pubsubpluginSTORAGE_GUCEFPlugin_Unload();

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Unload finished for PUBSUB plugin TEST" );
}

/*--------------------------------------------------------------------------*/

void PUBSUB_TEST_PLUGIN_CALLSPEC_PREFIX
pubsubpluginTEST_GUCEFPlugin_GetVersion( CORE::TVersion* versionInfo ) PUBSUB_TEST_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    if ( GUCEF_NULL != versionInfo )
    {
        versionInfo->major = 0;
        versionInfo->minor = 0;
        versionInfo->patch = 0;
        versionInfo->release = 0;
    }
}

/*--------------------------------------------------------------------------*/

const char* PUBSUB_TEST_PLUGIN_CALLSPEC_PREFIX
pubsubpluginTEST_GUCEFPlugin_GetCopyright( void ) PUBSUB_TEST_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    return "Copyright (C) Dinand Vanvelzen, Apache License v2";
}

/*--------------------------------------------------------------------------*/

const char* PUBSUB_TEST_PLUGIN_CALLSPEC_PREFIX
pubsubpluginTEST_GUCEFPlugin_GetDescription( void ) PUBSUB_TEST_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    return "Generic GUCEF PUBSUB plugin for adding integration/system tests";
}

/*--------------------------------------------------------------------------*/

const char* PUBSUB_TEST_PLUGIN_CALLSPEC_PREFIX
pubsubpluginTEST_GUCEFPlugin_GetLinkBackModuleDependencies( void ) PUBSUB_TEST_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    return "gucefCORE,gucefVFS,gucefPUBSUB";
}

/*--------------------------------------------------------------------------*/
