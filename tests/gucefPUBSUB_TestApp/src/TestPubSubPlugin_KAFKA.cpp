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

#include <string.h>

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_PUBSUB_H
#include "gucefPUBSUB.h"
#define GUCEF_PUBSUB_H
#endif /* GUCEF_PUBSUB_H ? */

#ifndef GUCEF_CORE_CPLUGINCONTROL_H
#include "CPluginControl.h"
#define GUCEF_CORE_CPLUGINCONTROL_H
#endif /* GUCEF_CORE_CPLUGINCONTROL_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestPubSubPlugin_KAFKA.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformPubSubPlugin_KAFKATests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING pubsubpluginKAFKA backend TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "pubsubpluginKAFKA" );

    // The topic index bookmark of the plugin is a blob of packed 12 byte entries, a UInt32 partition id and an Int64
    // offset in host byte order: CKafkaPubSubClientTopic::GetCurrentBookmark() writes it, VariantToPartitionOffsets()
    // reads it (entry count = size / sizeof( TPartitionOffset ), a packed struct of 12 bytes). Both need a broker to
    // run, so this pins the layout instead: written as the writer does (AppendValue(), no typed store at the offsets
    // that are not aligned), compared with the image of the writer before (each value at its offset, no padding), and
    // read back as the reader does (ReadValue() at the same offsets).
    GUCEF_TESTFW_TESTCASE( "Bookmark blob layout: packed 12 byte entries of a UInt32 partition id and an Int64 offset" )
        try
        {
            const CORE::UInt32 entryCount = 3;
            const CORE::UInt32 entrySize = sizeof( CORE::UInt32 ) + sizeof( CORE::Int64 );
            const CORE::UInt32 partitionIds[ 3 ] = { 0, 7, 0x80000001 };
            const CORE::Int64 partitionOffsets[ 3 ] = { 0, 1234567890123LL, -2 };

            CORE::CDynamicBuffer written( entryCount * entrySize );
            for ( CORE::UInt32 i=0; i<entryCount; ++i )
            {
                written.AppendValue< CORE::UInt32 >( partitionIds[ i ] );
                written.AppendValue< CORE::Int64 >( partitionOffsets[ i ] );
            }
            ASSERT_TRUE( 12 == entrySize && entryCount * entrySize == written.GetDataSize() );

            CORE::UInt8 expected[ 36 ];
            for ( CORE::UInt32 i=0; i<entryCount; ++i )
            {
                memcpy( expected + i*entrySize, &partitionIds[ i ], sizeof( CORE::UInt32 ) );
                memcpy( expected + i*entrySize + sizeof( CORE::UInt32 ), &partitionOffsets[ i ], sizeof( CORE::Int64 ) );
            }
            ASSERT_TRUE( 0 == memcmp( expected, written.GetConstBufferPtr(), sizeof( expected ) ) );

            CORE::CVariant bookmarkBlob( written );
            CORE::CDynamicBuffer read;
            read.LinkTo( bookmarkBlob );
            ASSERT_TRUE( entryCount == read.GetDataSize() / entrySize );
            for ( CORE::UInt32 i=0; i<entryCount; ++i )
            {
                ASSERT_TRUE( partitionIds[ i ] == read.ReadValue< CORE::UInt32 >( i*entrySize ) );
                ASSERT_TRUE( partitionOffsets[ i ] == read.ReadValue< CORE::Int64 >( i*entrySize + sizeof( CORE::UInt32 ) ) );
            }
        }
        catch ( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    CORE::CPluginControl& pluginControl = CORE::CCoreGlobal::Instance()->GetPluginControl();

    #ifdef GUCEF_DEBUG_MODE
    CORE::CString firstPluginPath  = "$MODULEDIR$/pubsubpluginKAFKA_d";
    CORE::CString secondPluginPath = "$MODULEDIR$/pubsubpluginKAFKA";
    #else
    CORE::CString firstPluginPath  = "$MODULEDIR$/pubsubpluginKAFKA";
    CORE::CString secondPluginPath = "$MODULEDIR$/pubsubpluginKAFKA_d";
    #endif

    bool pluginLoaded = pluginControl.AddPluginFromDir( firstPluginPath, "pubsubpluginKAFKA_tests", true );
    if ( !pluginLoaded )
        pluginLoaded = pluginControl.AddPluginFromDir( secondPluginPath, "pubsubpluginKAFKA_tests", true );

    if ( !pluginLoaded )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "pubsubpluginKAFKA binary not available, skipping backend tests" );
        return;
    }

    // Test 1: Verify the plugin registers its client type in the PubSub factory
    GUCEF_TESTFW_TESTCASE( "Test 1: Plugin registers \"Kafka\" client type in PubSubClientFactory" )
        try
        {
            PUBSUB::CPubSubClientFactory& factory = PUBSUB::CPubSubGlobal::Instance()->GetPubSubClientFactory();
            ASSERT_TRUE( factory.IsConstructible( "Kafka" ) );
        }
        catch ( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    pluginControl.UnloadPluginGroup( "pubsubpluginKAFKA_tests" );

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ALL pubsubpluginKAFKA backend TESTS COMPLETED" );
}

/*-------------------------------------------------------------------------*/
