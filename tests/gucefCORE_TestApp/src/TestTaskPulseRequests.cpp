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

#ifndef GUCEF_CORE_CPULSEGENERATOR_H
#include "gucefCORE_CPulseGenerator.h"
#define GUCEF_CORE_CPULSEGENERATOR_H
#endif /* GUCEF_CORE_CPULSEGENERATOR_H ? */

#ifndef GUCEF_CORE_COBSERVER_H
#include "CObserver.h"
#define GUCEF_CORE_COBSERVER_H
#endif /* GUCEF_CORE_COBSERVER_H ? */

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

#ifndef GUCEF_MT_CACTIVEOBJECT_H
#include "gucefMT_CActiveObject.h"
#define GUCEF_MT_CACTIVEOBJECT_H
#endif /* GUCEF_MT_CACTIVEOBJECT_H ? */

#ifndef GUCEF_MT_CTATOMIC_H
#include "gucefMT_CTAtomic.h"
#define GUCEF_MT_CTATOMIC_H
#endif /* GUCEF_MT_CTATOMIC_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestTaskPulseRequests.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

using namespace GUCEF;
using namespace GUCEF::CORE;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

const char* g_poolName = "gucefCORE_TestApp.TaskPulseRequests";

/** A persistent dedicated task: its work is in the pulses of its delegator's pulse generator */
class CPulseRequestTestTask : public CTaskConsumer
{
    public:

    CPulseRequestTestTask( void ) : CTaskConsumer() {}
    virtual ~CPulseRequestTestTask() {}

    virtual CString GetType( void ) const GUCEF_VIRTUAL_OVERRIDE { return "GUCEF::CORE::TestTaskPulseRequests::Task"; }
    virtual bool OnTaskCycle( CTaskPtr task ) GUCEF_VIRTUAL_OVERRIDE { return false; }
    virtual void OnTaskEnded( CTaskPtr task, bool wasForced ) GUCEF_VIRTUAL_OVERRIDE { CTaskConsumer::OnTaskEnded( task, wasForced ); }

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        static const CString typeName = "CPulseRequestTestTask";
        return typeName;
    }
};

typedef CTSharedPtr< CPulseRequestTestTask, MT::CMutex > CPulseRequestTestTaskPtr;

/*-------------------------------------------------------------------------*/

/** Counts the pulses of a pulse generator */
class CPulseCounter : public CObserver
{
    public:

    CPulseCounter( void ) : CObserver(), m_pulses( 0 ) {}
    virtual ~CPulseCounter() { UnsubscribeAllFromObserver(); }

    void Watch( PulseGeneratorPtr pulseGenerator )
    {
        CTEventHandlerFunctor< CPulseCounter > callback( this, &CPulseCounter::OnPulse );
        SubscribeTo( pulseGenerator.GetPointerAlways(), CPulseGenerator::PulseEvent, callback );
    }

    UInt32 GetPulses( void ) const { return m_pulses.Load(); }

    private:

    void OnPulse( CNotifier* notifier, const CEvent& eventId, CICloneable* eventData ) { ++m_pulses; }

    MT::CTAtomic< UInt32 > m_pulses;
};

/*-------------------------------------------------------------------------*/

/** Asks a pulse generator, from a thread of its own, for every kind of pulse there is and for the stop of them */
class CPulseRequesterThread : public MT::CActiveObject
{
    public:

    CPulseRequesterThread( const PulseGeneratorPtr& pulseGenerator, UInt32 rounds )
        : MT::CActiveObject()
        , m_pulseGenerator( pulseGenerator )
        , m_rounds( rounds )
        , m_done( 0 )
    {}
    virtual ~CPulseRequesterThread() {}

    UInt32 GetRoundsDone( void ) const { return m_done.Load(); }

    virtual const MT::CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NULL; }

    protected:

    virtual bool OnThreadStart( void* taskdata ) GUCEF_VIRTUAL_OVERRIDE { return true; }

    virtual bool OnThreadCycle( void* taskdata ) GUCEF_VIRTUAL_OVERRIDE
    {
        for ( UInt32 i=0; i<50; ++i )
        {
            m_pulseGenerator->RequestPeriodicPulses( this, 1 + ( i % 3 ) );
            m_pulseGenerator->RequestPulseInterval( 1 );
            m_pulseGenerator->RequestPulsesPerImmediatePulseRequest( 2 );
            m_pulseGenerator->RequestImmediatePulse();
            m_pulseGenerator->RequestStopOfPeriodicUpdates( this );
        }
        return ++m_done >= m_rounds;
    }

    private:

    PulseGeneratorPtr m_pulseGenerator;
    UInt32 m_rounds;
    MT::CTAtomic< UInt32 > m_done;
};

/*-------------------------------------------------------------------------*/

bool
WaitForRounds( const CPulseRequesterThread& thread, UInt32 rounds, UInt32 timeoutMs )
{
    UInt32 waitedMs = 0;
    while ( thread.GetRoundsDone() < rounds )
    {
        if ( waitedMs >= timeoutMs )
            return false;
        MT::ThreadDelay( 5 );
        waitedMs += 5;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
WaitForPulses( const CPulseCounter& counter, UInt32 atLeast, UInt32 timeoutMs )
{
    UInt32 waitedMs = 0;
    while ( counter.GetPulses() < atLeast )
    {
        if ( waitedMs >= timeoutMs )
            return false;
        MT::ThreadDelay( 5 );
        waitedMs += 5;
    }
    return true;
}

}

/*-------------------------------------------------------------------------*/

void
PerformTaskPulseRequestTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "TaskPulseRequests" );

    GUCEF_TESTFW_TESTCASE( "Pulse requests from other threads while the delegator thread pulses: the pulses follow the last request" )
        try
        {
            ThreadPoolPtr pool = CCoreGlobal::Instance()->GetTaskManager().GetOrCreateThreadPool( g_poolName );
            ASSERT_TRUE( !pool.IsNULL() );
            if ( !pool.IsNULL() )
            {
                CPulseRequestTestTaskPtr task( GUCEF_NEW CPulseRequestTestTask() );
                ASSERT_TRUE( pool->SetupSingularTask( task ).HasAFuture() );
                ASSERT_TRUE( pool->StartTaskWithConsumer( task ).HasAFuture() );

                // the consumer takes the delegator's pulse generator once it is linked to its thread
                PulseGeneratorPtr global = CCoreGlobal::Instance()->GetPulseGenerator();
                PulseGeneratorPtr pulseGenerator;
                for ( UInt32 i=0; i<600 && pulseGenerator.IsNULL(); ++i )
                {
                    if ( !task->GetTaskDelegator().IsNULL() && task->GetPulseGenerator() != global )
                        pulseGenerator = task->GetPulseGenerator();
                    else
                        MT::ThreadDelay( 5 );
                }
                ASSERT_TRUE( !pulseGenerator.IsNULL() );
                if ( !pulseGenerator.IsNULL() )
                {
                    CPulseCounter counter;
                    counter.Watch( pulseGenerator );
                    UInt32 requestorsBefore = pulseGenerator->GetNrOfPeriodicPulseRequestors();

                    // two threads ask for pulses and stop them again, as fast as they can, while the delegator pulses
                    const UInt32 rounds = 100;
                    CPulseRequesterThread first( pulseGenerator, rounds );
                    CPulseRequesterThread second( pulseGenerator, rounds );
                    ASSERT_TRUE( first.Activate( GUCEF_NULL, 1, 1 ) );
                    ASSERT_TRUE( second.Activate( GUCEF_NULL, 1, 1 ) );
                    ASSERT_TRUE( WaitForRounds( first, rounds, 60000 ) );
                    ASSERT_TRUE( WaitForRounds( second, rounds, 60000 ) );
                    first.Deactivate( false, true );
                    second.Deactivate( false, true );

                    // every request was taken back again
                    ASSERT_TRUE( requestorsBefore == pulseGenerator->GetNrOfPeriodicPulseRequestors() );

                    // the last word: periodic pulses, which keep coming
                    UInt32 before = counter.GetPulses();
                    pulseGenerator->RequestPeriodicPulses( &counter, 1 );
                    ASSERT_TRUE( WaitForPulses( counter, before + 20, 5000 ) );

                    // and their stop: with no other requestor, after it settles no more than the odd pulse of a cycle in
                    // progress (the task consumer itself may ask for pulses of its own, then they go on)
                    pulseGenerator->RequestStopOfPeriodicUpdates( &counter );
                    if ( 0 == pulseGenerator->GetNrOfPeriodicPulseRequestors() )
                    {
                        MT::ThreadDelay( 100 );
                        UInt32 settled = counter.GetPulses();
                        MT::ThreadDelay( 200 );
                        ASSERT_TRUE( counter.GetPulses() <= settled + 1 );
                    }

                    counter.UnsubscribeAllFromObserver();
                }
                task->RequestTaskToStop( true );
            }
            CCoreGlobal::Instance()->GetTaskManager().UnregisterThreadPool( g_poolName );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;
}

/*-------------------------------------------------------------------------*/
