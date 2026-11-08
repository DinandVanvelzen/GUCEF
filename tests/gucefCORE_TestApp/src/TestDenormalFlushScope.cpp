/*
 *  gucefCORE_TestApp: GUCEF test application for the CORE module
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

#include <float.h>

#ifndef GUCEF_MT_CTATOMIC_H
#include "gucefMT_CTAtomic.h"
#define GUCEF_MT_CTATOMIC_H
#endif /* GUCEF_MT_CTATOMIC_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_CDENORMALFLUSHSCOPE_H
#include "gucefCORE_CDenormalFlushScope.h"
#define GUCEF_CORE_CDENORMALFLUSHSCOPE_H
#endif /* GUCEF_CORE_CDENORMALFLUSHSCOPE_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestDenormalFlushScope.h"

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

/** Half the smallest normal number: a denormal. Volatile so the arithmetic is really done at run time */
Float32
HalfSmallestNormal( void )
{
    volatile Float32 smallest = FLT_MIN;
    volatile Float32 half = 0.5f;
    volatile Float32 result = smallest * half;
    return result;
}

/** True when the result of the multiplication of two normal numbers is a denormal that was kept */
bool
ProducesDenormal( void )
{
    return 0.0f != HalfSmallestNormal();
}

/** True when a denormal that is read as input counts as zero */
bool
ReadsDenormalAsZero( void )
{
    const Float32 denormal = HalfSmallestNormal();
    volatile Float32 input = denormal;
    volatile Float32 factor = 1048576.0f;
    volatile Float32 result = input * factor;
    return 0.0f == result;
}

/** What a thread of its own sees of the flush state, and the handshake with the thread that opens the scope */
struct SThreadResult
{
    SThreadResult( void ) : activeInThread( true ), producedInThread( false ), go( false ) {}

    bool activeInThread;
    bool producedInThread;
    MT::CTAtomic< bool > go;
};

UInt32 GUCEF_CALLSPEC_PREFIX
ThreadMain( void* data ) GUCEF_CALLSPEC_SUFFIX
{
    SThreadResult* result = static_cast< SThreadResult* >( data );
    while ( !result->go.Load( MT::MEMORDER_ACQUIRE ) )
        MT::ThreadDelay( 1 );
    result->activeInThread = CORE::CDenormalFlushScope::IsActive();
    result->producedInThread = ProducesDenormal();
    return 0;
}

}; /* anonymous namespace */

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformDenormalFlushScopeTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING DenormalFlushScope TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "gucefCORE.DenormalFlushScope" );

    const bool supported = CORE::CDenormalFlushScope::IsSupported();
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "DenormalFlushScope: flush control supported on this platform: " << supported );

    GUCEF_TESTFW_TESTCASE( "DenormalFlushScope: Outside of any scope denormals are kept" )
    try
    {
        ASSERT_FALSE( CORE::CDenormalFlushScope::IsActive() );
        ASSERT_TRUE( ProducesDenormal() );
        ASSERT_FALSE( ReadsDenormalAsZero() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "DenormalFlushScope: Inside the scope a denormal flushes to zero and the state is restored afterwards" )
    try
    {
        ASSERT_TRUE( ProducesDenormal() );
        {
            CORE::CDenormalFlushScope scope;
            ASSERT_TRUE( CORE::CDenormalFlushScope::IsActive() == supported );
            if ( supported )
            {
                ASSERT_FALSE( ProducesDenormal() );
                ASSERT_TRUE( 0.0f == HalfSmallestNormal() );
                ASSERT_TRUE( ReadsDenormalAsZero() );
            }
            else
            {
                // a no-op scope changes nothing
                ASSERT_TRUE( ProducesDenormal() );
            }
        }
        ASSERT_FALSE( CORE::CDenormalFlushScope::IsActive() );
        ASSERT_TRUE( ProducesDenormal() );
        ASSERT_FALSE( ReadsDenormalAsZero() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "DenormalFlushScope: Normal numbers are not affected by the scope" )
    try
    {
        CORE::CDenormalFlushScope scope;
        volatile Float32 a = FLT_MIN;
        volatile Float32 b = 3.0f;
        volatile Float32 c = a * b;
        ASSERT_TRUE( c == FLT_MIN * 3.0f );
        volatile Float32 d = 1.0e-30f;
        volatile Float32 e = d * 2.0f;
        ASSERT_TRUE( e > 1.9e-30f && e < 2.1e-30f );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "DenormalFlushScope: Scopes nest and each puts back what it found" )
    try
    {
        {
            CORE::CDenormalFlushScope outer;
            {
                CORE::CDenormalFlushScope inner;
                ASSERT_TRUE( CORE::CDenormalFlushScope::IsActive() == supported );
            }
            // the inner scope found flushing on, so it must leave it on
            ASSERT_TRUE( CORE::CDenormalFlushScope::IsActive() == supported );
            if ( supported )
                ASSERT_FALSE( ProducesDenormal() );
        }
        ASSERT_FALSE( CORE::CDenormalFlushScope::IsActive() );
        ASSERT_TRUE( ProducesDenormal() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "DenormalFlushScope: The state is restored when an exception unwinds the scope" )
    try
    {
        bool caught = false;
        try
        {
            CORE::CDenormalFlushScope scope;
            throw 42;
        }
        catch ( int )
        {
            caught = true;
        }
        ASSERT_TRUE( caught );
        ASSERT_FALSE( CORE::CDenormalFlushScope::IsActive() );
        ASSERT_TRUE( ProducesDenormal() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "DenormalFlushScope: The setting is per thread, a thread that already runs is not affected" )
    try
    {
        // the thread is started before the scope opens (a thread started inside a scope inherits the setting of its creator)
        SThreadResult result;
        MT::SThreadData* td = MT::ThreadDataReserve();
        ASSERT_TRUE( GUCEF_NULL != td );
        if ( GUCEF_NULL != td )
        {
            ASSERT_TRUE( 0 != MT::ThreadCreate( td, &ThreadMain, &result ) );
            {
                CORE::CDenormalFlushScope scope;
                result.go.Store( true, MT::MEMORDER_RELEASE );
                MT::ThreadWait( td, 60000 );
                ASSERT_TRUE( CORE::CDenormalFlushScope::IsActive() == supported );
            }
            MT::ThreadDataCleanup( td );
        }
        ASSERT_FALSE( result.activeInThread );
        ASSERT_TRUE( result.producedInThread );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
