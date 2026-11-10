/*
 *  gucefCORE_TestApp: GUCEF test application for the CORE module
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
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

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CTASKMANAGER_H
#include "gucefCORE_CTaskManager.h"
#define GUCEF_CORE_CTASKMANAGER_H
#endif /* GUCEF_CORE_CTASKMANAGER_H ? */

#ifndef GUCEF_CORE_CTASKCONSUMER_H
#include "gucefCORE_CTaskConsumer.h"
#define GUCEF_CORE_CTASKCONSUMER_H
#endif /* GUCEF_CORE_CTASKCONSUMER_H ? */

#ifndef GUCEF_CORE_CFUTURERESULTSOURCE_H
#include "gucefCORE_CFutureResultSource.h"
#define GUCEF_CORE_CFUTURERESULTSOURCE_H
#endif /* GUCEF_CORE_CFUTURERESULTSOURCE_H ? */

#ifndef GUCEF_CORE_CPUMPEDOBSERVER_H
#include "CPumpedObserver.h"
#define GUCEF_CORE_CPUMPEDOBSERVER_H
#endif /* GUCEF_CORE_CPUMPEDOBSERVER_H ? */

#ifndef GUCEF_CORE_CPULSEGENERATOR_H
#include "gucefCORE_CPulseGenerator.h"
#define GUCEF_CORE_CPULSEGENERATOR_H
#endif /* GUCEF_CORE_CPULSEGENERATOR_H ? */

#ifndef GUCEF_CORE_CTFACTORY_H
#include "CTFactory.h"
#define GUCEF_CORE_CTFACTORY_H
#endif /* GUCEF_CORE_CTFACTORY_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestAllocationCounter.h"
#include "TestTaskMemoryRetention.h"

using namespace GUCEF;

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
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** Pumped observer which counts the mail it is given, apart from the pulses */
class CRetentionMailProbe : public CORE::CPumpedObserver
{
    public:

    static const CORE::CEvent ProbeEvent;

    CRetentionMailProbe( void )
        : CORE::CPumpedObserver( false )
        , m_pulseGeneratorNotifier( GUCEF_NULL )
        , m_linkEventsDelivered( 0 )
        , m_probeEventsDelivered( 0 )
    {GUCEF_TRACE;
    }

    virtual ~CRetentionMailProbe() GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        Shutdown();
    }

    const CORE::CNotifier* m_pulseGeneratorNotifier;
    UInt32 m_linkEventsDelivered;
    UInt32 m_probeEventsDelivered;

    protected:

    virtual void OnPumpedNotify( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        if ( ( CORE::CNotifier::SubscribeEvent == eventId || CORE::CNotifier::UnsubscribeEvent == eventId ) && notifier == m_pulseGeneratorNotifier )
            ++m_linkEventsDelivered;
        else
        if ( ProbeEvent == eventId )
            ++m_probeEventsDelivered;
    }
};

const CORE::CEvent CRetentionMailProbe::ProbeEvent = "GUCEF::CORE::TestTaskMemoryRetention::ProbeEvent";

/*-------------------------------------------------------------------------*/

/** A notifier which the probe observes, to show that mail which is meant for it still arrives */
class CRetentionProbeNotifier : public CORE::CNotifier
{
    public:

    CRetentionProbeNotifier( void ) : CORE::CNotifier() {}
    virtual ~CRetentionProbeNotifier() GUCEF_VIRTUAL_OVERRIDE {}

    void Send( void ) { NotifyObservers( CRetentionMailProbe::ProbeEvent ); }

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        static const CORE::CString typeName = "GUCEF::CORE::TestTaskMemoryRetention::CRetentionProbeNotifier";
        return typeName;
    }
};

/*-------------------------------------------------------------------------*/

/** Task data with a future, as the asynchronous operations of the platform use */
class CRetentionTaskData : public CORE::CICloneable
{
    public:

    CORE::CFutureResultSource result;

    CRetentionTaskData( void ) : CORE::CICloneable(), result() {}
    CRetentionTaskData( const CRetentionTaskData& src ) : CORE::CICloneable( src ), result( src.result ) {}
    virtual ~CRetentionTaskData() GUCEF_VIRTUAL_OVERRIDE {}

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NEW CRetentionTaskData( *this ); }
};

/*-------------------------------------------------------------------------*/

/** A task which is done in its first cycle */
class CRetentionQuickTask : public CORE::CTaskConsumer
{
    public:

    static const CORE::CString TaskType;

    CRetentionQuickTask( void ) : CORE::CTaskConsumer() {}
    virtual ~CRetentionQuickTask() GUCEF_VIRTUAL_OVERRIDE {}

    virtual CORE::CString GetType( void ) const GUCEF_VIRTUAL_OVERRIDE { return TaskType; }
    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE { return TaskType; }

    virtual bool OnTaskCycle( CORE::CTaskPtr task ) GUCEF_VIRTUAL_OVERRIDE
    {
        CRetentionTaskData* data = static_cast< CRetentionTaskData* >( task->GetTaskData() );
        if ( GUCEF_NULL != data )
            data->result.Complete();
        return true;
    }

    virtual void OnTaskEnded( CORE::CTaskPtr task, bool wasForced ) GUCEF_VIRTUAL_OVERRIDE
    {
        CORE::CTaskConsumer::OnTaskEnded( task, wasForced );
    }
};

const CORE::CString CRetentionQuickTask::TaskType = "TaskMemoryRetentionQuickTask";

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** Runs one task through the queue of the pool and waits for its future, as an asynchronous operation does */
static bool
RunQueuedTask( CORE::ThreadPoolPtr& pool )
{GUCEF_TRACE;

    CRetentionTaskData data;
    CORE::CFutureResult future = data.result.GetFuture();
    CORE::CFutureResult queued = pool->QueueTask( CRetentionQuickTask::TaskType, &data, GUCEF_NULL, false );
    if ( queued.HasNoFuture() )
        return false;
    future.Await( 10000 );
    if ( !future.IsReady() )
        return false;
    // the task itself is done once its future is
    return !queued.GetResult( 10000 ).IsNULL();
}

/*-------------------------------------------------------------------------*/

/** Delivers the mail which waits for the main thread, as the main loop of an application does */
static void
PumpMainThread( void )
{GUCEF_TRACE;

    CORE::CCoreGlobal::Instance()->GetPulseGenerator()->ForceDirectPulse();
}

/*-------------------------------------------------------------------------*/

void
PerformTaskMemoryRetentionTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING task memory retention TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "TaskMemoryRetention" );

    GUCEF_TESTFW_TESTCASE( "Test: A pumped observer which changes its pulse generator does not mail itself the events of that link" )
        try
        {
            CORE::PulseGeneratorPtr pulseGenerator = CORE::CPulseGenerator::CreateSharedObj();
            CRetentionProbeNotifier notifier;
            CRetentionMailProbe probe;
            probe.m_pulseGeneratorNotifier = pulseGenerator.GetPointerAlways();
            probe.SubscribeTo( &notifier, CRetentionMailProbe::ProbeEvent );

            // a task consumer moves to the pulse generator of the thread that runs a task and away from it after
            for ( UInt32 i=0; i<100; ++i )
            {
                probe.SetPulseGenerator( pulseGenerator );
                probe.SetPulseGenerator( CORE::PulseGeneratorPtr() );
            }
            notifier.Send();
            probe.SetPulseGenerator( pulseGenerator );
            pulseGenerator->ForceDirectPulse();

            // the mail meant for the observer arrives, the bookkeeping of its own link to a pulse generator is not mail
            ASSERT_TRUE( 1 == probe.m_probeEventsDelivered );
            ASSERT_TRUE( 0 == probe.m_linkEventsDelivered );

            // the observer still gets pulses and mail after all that
            notifier.Send();
            pulseGenerator->ForceDirectPulse();
            ASSERT_TRUE( 2 == probe.m_probeEventsDelivered );

            probe.SetPulseGenerator( CORE::PulseGeneratorPtr() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    typedef CORE::CTFactory< CORE::CTaskConsumer, CRetentionQuickTask, MT::CMutex > TQuickTaskFactory;
    TQuickTaskFactory quickTaskFactory;
    CORE::CTaskManager& taskManager = CORE::CCoreGlobal::Instance()->GetTaskManager();
    taskManager.RegisterTaskConsumerFactory( CRetentionQuickTask::TaskType, &quickTaskFactory );

    GUCEF_TESTFW_TESTCASE( "Test: Thousands of tasks queued on a thread pool and awaited leave no memory behind" )
        GUCEF_TESTFW_SKIP_IF( !CORETEST::IsLiveHeapMeasurementAvailable(), "the live heap cannot be measured in this build" );
        try
        {
            CORE::ThreadPoolPtr pool = taskManager.GetOrCreateThreadPool( "TaskMemoryRetentionPool" );
            ASSERT_FALSE( pool.IsNULL() );
            if ( !pool.IsNULL() )
            {
                // warm up: the free lists of task objects and consumers, the worker thread, the buffers of the log
                bool allRan = true;
                for ( UInt32 i=0; i<300; ++i )
                {
                    allRan = RunQueuedTask( pool ) && allRan;
                    PumpMainThread();
                }
                MT::ThreadDelay( 100 );
                PumpMainThread();

                const UInt32 taskCount = 3000;
                const UInt64 heapBefore = CORETEST::GetLiveHeapBytes();
                for ( UInt32 i=0; i<taskCount; ++i )
                {
                    allRan = RunQueuedTask( pool ) && allRan;
                    PumpMainThread();
                }
                MT::ThreadDelay( 100 );
                PumpMainThread();
                const UInt64 heapAfter = CORETEST::GetLiveHeapBytes();

                const Int64 growth = (Int64) heapAfter - (Int64) heapBefore;
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "TaskMemoryRetention: the heap changed by " << growth << " bytes over " << taskCount << " tasks" );
                ASSERT_TRUE( allRan );

                // a leak of a single small allocation per task is 3000 times that, the bound leaves room for noise
                ASSERT_TRUE( growth < (Int64) ( 16 * taskCount ) );
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    taskManager.UnregisterTaskConsumerFactory( CRetentionQuickTask::TaskType );

    CORE::CLogStreamScope::FlushLogs();
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ALL task memory retention TESTS COMPLETED" );
}

/*-------------------------------------------------------------------------*/
