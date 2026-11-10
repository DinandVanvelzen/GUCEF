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

#ifdef GUCEF_LINUX_BUILD
#include <unistd.h>
#endif /* GUCEF_LINUX_BUILD ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_MT_CTATOMIC_H
#include "gucefMT_CTAtomic.h"
#define GUCEF_MT_CTATOMIC_H
#endif /* GUCEF_MT_CTATOMIC_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CGUCEFAPPLICATION_H
#include "CGUCEFApplication.h"
#define GUCEF_CORE_CGUCEFAPPLICATION_H
#endif /* GUCEF_CORE_CGUCEFAPPLICATION_H ? */

#ifndef GUCEF_CORE_CTASKMANAGER_H
#include "gucefCORE_CTaskManager.h"
#define GUCEF_CORE_CTASKMANAGER_H
#endif /* GUCEF_CORE_CTASKMANAGER_H ? */

#ifndef GUCEF_CORE_CTASKCONSUMER_H
#include "gucefCORE_CTaskConsumer.h"
#define GUCEF_CORE_CTASKCONSUMER_H
#endif /* GUCEF_CORE_CTASKCONSUMER_H ? */

#ifndef GUCEF_CORE_CPROCESS_H
#include "gucefCORE_CProcess.h"
#define GUCEF_CORE_CPROCESS_H
#endif /* GUCEF_CORE_CPROCESS_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_TEST_CHILD_PROCESS_H
#include "gucef_test_child_process.h"
#define GUCEF_TEST_CHILD_PROCESS_H
#endif /* GUCEF_TEST_CHILD_PROCESS_H ? */

#include "TestAppStopWithTasks.h"

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
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

/*-------------------------------------------------------------------------*/

/** How long a stop of the application with one such task may take: a task thread is given far longer to stop */
static const UInt32 AppStopWithTaskMaxStopTimeInMs = 10000;

/*-------------------------------------------------------------------------*/

namespace {

/** Calls into the application all through every cycle until it is asked to stop */
class CAppUsingConsumer : public CORE::CTaskConsumer
{
    public:

    CAppUsingConsumer( void ) : CORE::CTaskConsumer(), cycles( 0 ), ended( false ) {}

    virtual CORE::CString GetType( void ) const GUCEF_VIRTUAL_OVERRIDE { return "TestAppStopWithTasks::CAppUsingConsumer"; }

    /** a cycle is a stretch of work that uses the application throughout, as a task busy with a job does */
    virtual bool OnTaskCycle( CORE::CTaskPtr ) GUCEF_VIRTUAL_OVERRIDE
    {
        for ( UInt32 i=0; i<50; ++i )
        {
            CORE::CCoreGlobal::Instance()->GetApplication().GetApplicationDir();
            MT::ThreadDelay( 1 );
        }
        cycles.FetchAdd( 1 );
        return false;
    }

    virtual void OnTaskEnded( CORE::CTaskPtr task, bool wasForced ) GUCEF_VIRTUAL_OVERRIDE
    {
        ended.Store( true );
        CORE::CTaskConsumer::OnTaskEnded( task, wasForced );
    }

    MT::CTAtomic< UInt32 > cycles;
    MT::CTAtomic< bool > ended;
};

typedef CORE::CTSharedPtr< CAppUsingConsumer, MT::CMutex > TAppUsingConsumerPtr;

}

/*-------------------------------------------------------------------------*/

int
RunAppStopWithAppUsingTask( void )
{GUCEF_TRACE;

    CORE::ThreadPoolPtr pool = CORE::CCoreGlobal::Instance()->GetTaskManager().GetOrCreateThreadPool( "AppStopWithTasks" );
    TAppUsingConsumerPtr consumer( GUCEF_NEW CAppUsingConsumer() );
    if ( pool->StartTaskWithConsumer( consumer ).HasNoFuture() )
        return 2;

    const UInt64 ticksPerMs = MT::PrecisionTimerResolution() / 1000;
    UInt64 startTicks = MT::PrecisionTickCount();
    while ( 0 == consumer->cycles.Load() )
    {
        if ( ( MT::PrecisionTickCount() - startTicks ) > 5000 * ticksPerMs )
            return 3;
        MT::ThreadDelay( 1 );
    }

    startTicks = MT::PrecisionTickCount();
    const bool stopped = CORE::CCoreGlobal::Instance()->GetApplication().Stop( true );
    const UInt64 stopTimeInMs = ( MT::PrecisionTickCount() - startTicks ) / ticksPerMs;

    GUCEF_LOG( CORE::LOGLEVEL_IMPORTANT, "AppStopWithTasks: the application stopped in " << stopTimeInMs << " ms, stop result "
        << stopped << ", task ended " << consumer->ended.Load() );

    if ( !consumer->ended.Load() )
        return 4;
    if ( stopTimeInMs > AppStopWithTaskMaxStopTimeInMs )
        return 5;
    return stopped ? 0 : 6;
}

/*-------------------------------------------------------------------------*/

void
PerformAppStopWithTasksTests( void )
{
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING application stop with tasks TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "AppStopWithTasks" );

    #ifdef GUCEF_LINUX_BUILD

    // The stop is that of the whole application, so it runs in a child process of its own
    GUCEF_TESTFW_TESTCASE( "An application whose task calls into the application on every cycle stops within seconds and exits" )
        try
        {
            char exePath[ 4096 ];
            ssize_t exePathLength = readlink( "/proc/self/exe", exePath, sizeof( exePath ) - 1 );
            ASSERT_TRUE( exePathLength > 0 );
            if ( exePathLength > 0 )
            {
                exePath[ exePathLength ] = '\0';

                CORE::CProcess::TStringVector args;
                args.push_back( GUCEF_CORE_TESTAPP_APP_STOP_WITH_TASK_ARG );
                CORE::CProcess child;
                child.SetIsolateFromParent( true );
                ASSERT_TRUE( child.Launch( exePath, args, CORE::CString(), TESTFW::ChildEnvironmentWithOwnExitCode() ) );

                // a stop that waits on the task for the lock timeout of gucefMT takes over a minute
                Int32 exitCode = -1;
                const bool ended = child.WaitForExit( 180000, exitCode );
                if ( !ended )
                    child.Terminate( false );
                ASSERT_TRUE( ended );
                ASSERT_TRUE( 0 == exitCode );
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    #endif /* GUCEF_LINUX_BUILD ? */
}

/*-------------------------------------------------------------------------*/
