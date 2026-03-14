/*
 *  gucefPUBSUB_TestApp: GUCEF test application for the PUBSUB module
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

#ifndef GUCEF_MACROS_H
#include "gucef_macros.h"
#define GUCEF_MACROS_H
#endif /* GUCEF_MACROS_H ? */

#define GUCEF_TESTFW_IMPL
#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_PUBSUB_H
#include "gucefPUBSUB.h"
#define GUCEF_PUBSUB_H
#endif /* GUCEF_PUBSUB_H ? */

#ifndef GUCEF_PUBSUB_TESTAPP_TESTBASICPUBSUBMSG_H
#include "TestBasicPubSubMsg.h"
#define GUCEF_PUBSUB_TESTAPP_TESTBASICPUBSUBMSG_H
#endif /* GUCEF_PUBSUB_TESTAPP_TESTBASICPUBSUBMSG_H ? */

#ifndef GUCEF_PUBSUB_TESTAPP_TESTPUBSUBMSGBINARYSERIALIZER_H
#include "TestPubSubMsgBinarySerializer.h"
#define GUCEF_PUBSUB_TESTAPP_TESTPUBSUBMSGBINARYSERIALIZER_H
#endif /* GUCEF_PUBSUB_TESTAPP_TESTPUBSUBMSGBINARYSERIALIZER_H ? */

#ifndef GUCEF_PUBSUB_TESTAPP_TESTPUBSUBMSGCONTAINERBINARYSERIALIZER_H
#include "TestPubSubMsgContainerBinarySerializer.h"
#define GUCEF_PUBSUB_TESTAPP_TESTPUBSUBMSGCONTAINERBINARYSERIALIZER_H
#endif /* GUCEF_PUBSUB_TESTAPP_TESTPUBSUBMSGCONTAINERBINARYSERIALIZER_H ? */

#ifndef GUCEF_PUBSUB_TESTAPP_TESTPUBSUBBOOKMARK_H
#include "TestPubSubBookmark.h"
#define GUCEF_PUBSUB_TESTAPP_TESTPUBSUBBOOKMARK_H
#endif /* GUCEF_PUBSUB_TESTAPP_TESTPUBSUBBOOKMARK_H ? */

#ifndef GUCEF_PUBSUB_TESTAPP_TESTPUBSUBCLIENTCONFIG_H
#include "TestPubSubClientConfig.h"
#define GUCEF_PUBSUB_TESTAPP_TESTPUBSUBCLIENTCONFIG_H
#endif /* GUCEF_PUBSUB_TESTAPP_TESTPUBSUBCLIENTCONFIG_H ? */

#ifndef GUCEF_PUBSUB_TESTAPP_TESTPUBSUBCLIENTFEATURES_H
#include "TestPubSubClientFeatures.h"
#define GUCEF_PUBSUB_TESTAPP_TESTPUBSUBCLIENTFEATURES_H
#endif /* GUCEF_PUBSUB_TESTAPP_TESTPUBSUBCLIENTFEATURES_H ? */

#ifndef GUCEF_PUBSUB_TESTAPP_TESTPUBSUBCLIENTSIDE_H
#include "TestPubSubClientSide.h"
#define GUCEF_PUBSUB_TESTAPP_TESTPUBSUBCLIENTSIDE_H
#endif /* GUCEF_PUBSUB_TESTAPP_TESTPUBSUBCLIENTSIDE_H ? */

#ifndef GUCEF_PUBSUB_TESTAPP_TESTPUBSUBFLOWROUTER_H
#include "TestPubSubFlowRouter.h"
#define GUCEF_PUBSUB_TESTAPP_TESTPUBSUBFLOWROUTER_H
#endif /* GUCEF_PUBSUB_TESTAPP_TESTPUBSUBFLOWROUTER_H ? */

#ifndef GUCEF_PUBSUB_TESTAPP_TESTPUBSUBFLOWROUTERCONFIG_H
#include "TestPubSubFlowRouterConfig.h"
#define GUCEF_PUBSUB_TESTAPP_TESTPUBSUBFLOWROUTERCONFIG_H
#endif /* GUCEF_PUBSUB_TESTAPP_TESTPUBSUBFLOWROUTERCONFIG_H ? */

#ifndef GUCEF_PUBSUB_TESTAPP_TESTPUBSUBPLUGIN_AWSSNS_H
#include "TestPubSubPlugin_AWSSNS.h"
#define GUCEF_PUBSUB_TESTAPP_TESTPUBSUBPLUGIN_AWSSNS_H
#endif /* GUCEF_PUBSUB_TESTAPP_TESTPUBSUBPLUGIN_AWSSNS_H ? */

#ifndef GUCEF_PUBSUB_TESTAPP_TESTPUBSUBPLUGIN_AWSSQS_H
#include "TestPubSubPlugin_AWSSQS.h"
#define GUCEF_PUBSUB_TESTAPP_TESTPUBSUBPLUGIN_AWSSQS_H
#endif /* GUCEF_PUBSUB_TESTAPP_TESTPUBSUBPLUGIN_AWSSQS_H ? */

#ifndef GUCEF_PUBSUB_TESTAPP_TESTPUBSUBPLUGIN_KAFKA_H
#include "TestPubSubPlugin_KAFKA.h"
#define GUCEF_PUBSUB_TESTAPP_TESTPUBSUBPLUGIN_KAFKA_H
#endif /* GUCEF_PUBSUB_TESTAPP_TESTPUBSUBPLUGIN_KAFKA_H ? */

#ifndef GUCEF_PUBSUB_TESTAPP_TESTPUBSUBPLUGIN_MSMQ_H
#include "TestPubSubPlugin_MSMQ.h"
#define GUCEF_PUBSUB_TESTAPP_TESTPUBSUBPLUGIN_MSMQ_H
#endif /* GUCEF_PUBSUB_TESTAPP_TESTPUBSUBPLUGIN_MSMQ_H ? */

#ifndef GUCEF_PUBSUB_TESTAPP_TESTPUBSUBPLUGIN_REDISCLUSTER_H
#include "TestPubSubPlugin_REDISCLUSTER.h"
#define GUCEF_PUBSUB_TESTAPP_TESTPUBSUBPLUGIN_REDISCLUSTER_H
#endif /* GUCEF_PUBSUB_TESTAPP_TESTPUBSUBPLUGIN_REDISCLUSTER_H ? */

#ifndef GUCEF_PUBSUB_TESTAPP_TESTPUBSUBPLUGIN_STORAGE_H
#include "TestPubSubPlugin_STORAGE.h"
#define GUCEF_PUBSUB_TESTAPP_TESTPUBSUBPLUGIN_STORAGE_H
#endif /* GUCEF_PUBSUB_TESTAPP_TESTPUBSUBPLUGIN_STORAGE_H ? */

#ifndef GUCEF_PUBSUB_TESTAPP_TESTPUBSUBPLUGIN_TEST_H
#include "TestPubSubPlugin_TEST.h"
#define GUCEF_PUBSUB_TESTAPP_TESTPUBSUBPLUGIN_TEST_H
#endif /* GUCEF_PUBSUB_TESTAPP_TESTPUBSUBPLUGIN_TEST_H ? */

#ifndef GUCEF_PUBSUB_TESTAPP_TESTPUBSUBPLUGIN_UDP_H
#include "TestPubSubPlugin_UDP.h"
#define GUCEF_PUBSUB_TESTAPP_TESTPUBSUBPLUGIN_UDP_H
#endif /* GUCEF_PUBSUB_TESTAPP_TESTPUBSUBPLUGIN_UDP_H ? */

#ifndef GUCEF_PUBSUB_TESTAPP_TESTPUBSUBPLUGIN_WEB_H
#include "TestPubSubPlugin_WEB.h"
#define GUCEF_PUBSUB_TESTAPP_TESTPUBSUBPLUGIN_WEB_H
#endif /* GUCEF_PUBSUB_TESTAPP_TESTPUBSUBPLUGIN_WEB_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

/*-------------------------------------------------------------------------*/

/*
 *      Application entry point
 */
GUCEF_OSMAIN_BEGIN
{GUCEF_TRACE;

    try
    {
        GUCEF::CORE::CString logFilename = GUCEF::CORE::RelativePath( "$CURWORKDIR$" );
        GUCEF::CORE::AppendToPath( logFilename, "gucefPUBSUB_TestApp_Log.txt" );
        GUCEF::CORE::CFileAccess logFileAccess( logFilename, "w" );

        GUCEF::CORE::CStdLogger logger( logFileAccess );
        GUCEF::CORE::CCoreGlobal::Instance()->GetLogManager().AddLogger( &logger );

        GUCEF::CORE::CPlatformNativeConsoleLogger console;
        if ( GUCEF_APP_TYPE == GUCEF_APP_TYPE_CONSOLE )
            GUCEF::CORE::CCoreGlobal::Instance()->GetLogManager().AddLogger( console.GetLogger() );

        GUCEF::CORE::CCoreGlobal::Instance()->GetLogManager().FlushBootstrapLogEntriesToLogs();

        GUCEF_TESTFW_INIT( "gucefPUBSUB_TestApp" );
        GUCEF_TESTFW_SET_CALLBACK( GUCEF::CORE::TestFrameworkLogCallback, GUCEF_NULL );

        #define FLUSH_LOGS GUCEF::CORE::CCoreGlobal::Instance()->GetLogManager().FlushLogs()

        PerformPubSubBookmarkTests();                     FLUSH_LOGS;
        PerformPubSubClientFeaturesTests();               FLUSH_LOGS;
        PerformBasicPubSubMsgTests();                     FLUSH_LOGS;
        PerformPubSubMsgBinarySerializerTests();          FLUSH_LOGS;
        PerformPubSubMsgContainerBinarySerializerTests(); FLUSH_LOGS;
        PerformPubSubClientConfigTests();                 FLUSH_LOGS;
        PerformPubSubFlowRouterConfigTests();             FLUSH_LOGS;
        PerformPubSubFlowRouterTests();                   FLUSH_LOGS;
        PerformPubSubClientSideTests();                   FLUSH_LOGS;

        PerformPubSubPlugin_AWSSNSTests();                FLUSH_LOGS;
        PerformPubSubPlugin_AWSSQSTests();                FLUSH_LOGS;
        PerformPubSubPlugin_KAFKATests();                 FLUSH_LOGS;
        PerformPubSubPlugin_MSMQTests();                  FLUSH_LOGS;
        PerformPubSubPlugin_REDISCLUSTERTests();          FLUSH_LOGS;
        PerformPubSubPlugin_STORAGETests();               FLUSH_LOGS;
        PerformPubSubPlugin_TESTTests();                  FLUSH_LOGS;
        PerformPubSubPlugin_UDPTests();                   FLUSH_LOGS;
        PerformPubSubPlugin_WEBTests();                   FLUSH_LOGS;

        GUCEF::CORE::CString xmlPath = GUCEF::CORE::RelativePath( "$CURWORKDIR$" );
        GUCEF::CORE::AppendToPath( xmlPath, "gucefPUBSUB_TestApp_Results.xml" );
        GUCEF_TESTFW_WRITE_RESULTS( xmlPath.C_String() );
        return GUCEF_TESTFW_EXITCODE;
    }
    catch ( ... )
    {
        GUCEF::CORE::ShowErrorMessage( "Unknown exception"                                                                 ,
                                       "Unhandled exception during program execution, the application will now terminate"  );
    }
    return 1;
}
GUCEF_OSMAIN_END

/*-------------------------------------------------------------------------*/
