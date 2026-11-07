/*
 *  gucefMT_TestApp: GUCEF test application for the MT module
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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CTATOMIC_H
#include "gucefMT_CTAtomic.h"
#define GUCEF_MT_CTATOMIC_H
#endif /* GUCEF_MT_CTATOMIC_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestThreadData.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

const UInt32 RaceIterations = 2000;
const UInt32 DistinctThreadCount = 8;
const Int32 ThreadWaitLimitInMs = 60000;

/** What the thread function sees, shared between the creating thread and the new thread */
struct SThreadTestData
{
    SThreadTestData( void )
        : td( GUCEF_NULL )
        , selfId( 0 )
        , hold( false )
        , release( false )
    {}

    MT::SThreadData* td;     /**< set by the creator before MT::ThreadCreate(), read only after */
    UInt32 selfId;              /**< written by the new thread, read by the creator after MT::ThreadWait() */
    MT::CTAtomic< bool > hold;  /**< when true the thread function keeps running until release is set */
    MT::CTAtomic< bool > release;
};

/*-------------------------------------------------------------------------*/

UInt32 GUCEF_CALLSPEC_PREFIX
ThreadTestFunc( void* data ) GUCEF_CALLSPEC_SUFFIX
{
    SThreadTestData* testData = static_cast< SThreadTestData* >( data );
    testData->selfId = MT::ThreadID( testData->td );
    while ( testData->hold.Load( MT::MEMORDER_ACQUIRE ) && !testData->release.Load( MT::MEMORDER_ACQUIRE ) )
        MT::ThreadDelay( 1 );
    return 0;
}

}; /* anonymous namespace */

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformThreadDataTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING ThreadData TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "gucefMT.ThreadData" );

    GUCEF_TESTFW_TESTCASE( "ThreadID is valid and consistent right after ThreadCreate returns" )
    try
    {
        UInt32 zeroCount = 0;
        UInt32 mismatchCount = 0;
        UInt32 failedCount = 0;
        for ( UInt32 i=0; i<RaceIterations; ++i )
        {
            SThreadTestData testData;
            testData.td = MT::ThreadDataReserve();
            ASSERT_TRUE( GUCEF_NULL != testData.td );
            if ( 0 == MT::ThreadCreate( testData.td, &ThreadTestFunc, &testData ) )
            {
                ++failedCount;
                MT::ThreadDataCleanup( testData.td );
                continue;
            }

            // Read from the creating thread, concurrent with the new thread starting up
            UInt32 idAfterCreate = MT::ThreadID( testData.td );

            ASSERT_TRUE( GUCEF_THREAD_WAIT_OK == MT::ThreadWait( testData.td, ThreadWaitLimitInMs ) );
            if ( 0 == idAfterCreate )
                ++zeroCount;
            else
            if ( idAfterCreate != testData.selfId )
                ++mismatchCount;
            MT::ThreadDataCleanup( testData.td );
        }
        ASSERT_TRUE( 0 == failedCount );
        ASSERT_TRUE( 0 == zeroCount );
        ASSERT_TRUE( 0 == mismatchCount );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Concurrently running threads have distinct non-zero ids as seen by creator and thread itself" )
    try
    {
        SThreadTestData testData[ DistinctThreadCount ];
        for ( UInt32 i=0; i<DistinctThreadCount; ++i )
        {
            testData[ i ].hold.Store( true, MT::MEMORDER_RELEASE );
            testData[ i ].td = MT::ThreadDataReserve();
            ASSERT_TRUE( GUCEF_NULL != testData[ i ].td );
            ASSERT_TRUE( 0 != MT::ThreadCreate( testData[ i ].td, &ThreadTestFunc, &testData[ i ] ) );
        }

        UInt32 creatorIds[ DistinctThreadCount ];
        for ( UInt32 i=0; i<DistinctThreadCount; ++i )
        {
            creatorIds[ i ] = MT::ThreadID( testData[ i ].td );
            ASSERT_TRUE( 0 != creatorIds[ i ] );
        }
        for ( UInt32 i=0; i<DistinctThreadCount; ++i )
            for ( UInt32 n=i+1; n<DistinctThreadCount; ++n )
                ASSERT_TRUE( creatorIds[ i ] != creatorIds[ n ] );

        for ( UInt32 i=0; i<DistinctThreadCount; ++i )
            testData[ i ].release.Store( true, MT::MEMORDER_RELEASE );
        for ( UInt32 i=0; i<DistinctThreadCount; ++i )
        {
            ASSERT_TRUE( GUCEF_THREAD_WAIT_OK == MT::ThreadWait( testData[ i ].td, ThreadWaitLimitInMs ) );
            ASSERT_TRUE( creatorIds[ i ] == testData[ i ].selfId );
            MT::ThreadDataCleanup( testData[ i ].td );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
