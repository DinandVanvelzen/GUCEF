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

#ifndef GUCEF_CORE_CPULSEGENERATOR_H
#include "gucefCORE_CPulseGenerator.h"
#define GUCEF_CORE_CPULSEGENERATOR_H
#endif /* GUCEF_CORE_CPULSEGENERATOR_H ? */

#ifndef GUCEF_CORE_CBUSYWAITPULSEGENERATORDRIVER_H
#include "gucefCORE_CBusyWaitPulseGeneratorDriver.h"
#define GUCEF_CORE_CBUSYWAITPULSEGENERATORDRIVER_H
#endif /* GUCEF_CORE_CBUSYWAITPULSEGENERATORDRIVER_H ? */

#ifndef GUCEF_CORE_CIPULSEGENERATORDRIVER_H
#include "gucefCORE_CIPulseGeneratorDriver.h"
#define GUCEF_CORE_CIPULSEGENERATORDRIVER_H
#endif /* GUCEF_CORE_CIPULSEGENERATORDRIVER_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

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

#include "TestBusyWaitPulseGeneratorDriver.h"

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

/** Counts the pulses of a pulse generator */
class CBusyWaitPulseCounter : public CObserver
{
    public:

    CBusyWaitPulseCounter( void ) : CObserver(), m_pulses( 0 ) {}
    virtual ~CBusyWaitPulseCounter() { UnsubscribeAllFromObserver(); }

    void Watch( PulseGeneratorPtr pulseGenerator )
    {
        CTEventHandlerFunctor< CBusyWaitPulseCounter > callback( this, &CBusyWaitPulseCounter::OnPulse );
        SubscribeTo( pulseGenerator.GetPointerAlways(), CPulseGenerator::PulseEvent, callback );
    }

    UInt32 GetPulses( void ) const { return m_pulses.Load(); }

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        static const CString typeName = "CBusyWaitPulseCounter";
        return typeName;
    }

    private:

    void OnPulse( CNotifier* notifier, const CEvent& eventId, CICloneable* eventData ) { ++m_pulses; }

    MT::CTAtomic< UInt32 > m_pulses;
};

/*-------------------------------------------------------------------------*/

/** The thread of the Run() loop of the driver: it ends when the loop does */
class CBusyWaitRunThread : public MT::CActiveObject
{
    public:

    CBusyWaitRunThread( CBusyWaitPulseGeneratorDriver& driver, const PulseGeneratorPtr& pulseGenerator )
        : MT::CActiveObject()
        , m_driver( driver )
        , m_pulseGenerator( pulseGenerator )
        , m_runEnded( 0 )
    {}
    virtual ~CBusyWaitRunThread() {}

    bool HasRunEnded( void ) const { return 0 != m_runEnded.Load(); }

    virtual const MT::CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NULL; }

    protected:

    virtual bool OnThreadStart( void* taskdata ) GUCEF_VIRTUAL_OVERRIDE { return true; }

    virtual bool OnThreadCycle( void* taskdata ) GUCEF_VIRTUAL_OVERRIDE
    {
        m_driver.Run( *m_pulseGenerator, 1, 5 );
        m_runEnded.Store( 1 );
        return true;
    }

    private:

    CBusyWaitPulseGeneratorDriver& m_driver;
    PulseGeneratorPtr m_pulseGenerator;
    MT::CTAtomic< UInt32 > m_runEnded;
};

/*-------------------------------------------------------------------------*/

/** Asks the pulse generator, from a thread of its own, for every kind of pulse there is and for the stop of them */
class CBusyWaitRequesterThread : public MT::CActiveObject
{
    public:

    CBusyWaitRequesterThread( const PulseGeneratorPtr& pulseGenerator, UInt32 rounds )
        : MT::CActiveObject()
        , m_pulseGenerator( pulseGenerator )
        , m_rounds( rounds )
        , m_done( 0 )
        , m_driverThreadIds( 0 )
    {}
    virtual ~CBusyWaitRequesterThread() {}

    UInt32 GetRoundsDone( void ) const { return m_done.Load(); }

    virtual const MT::CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NULL; }

    protected:

    virtual bool OnThreadStart( void* taskdata ) GUCEF_VIRTUAL_OVERRIDE { return true; }

    virtual bool OnThreadCycle( void* taskdata ) GUCEF_VIRTUAL_OVERRIDE
    {
        for ( UInt32 i=0; i<50; ++i )
        {
            m_pulseGenerator->RequestPeriodicPulses( this, 1 + ( i % 3 ) );
            m_pulseGenerator->RequestPulsesPerImmediatePulseRequest( 1 + ( i % 2 ) );
            m_pulseGenerator->RequestImmediatePulse();
            if ( 0 != m_pulseGenerator->GetPulseDriverThreadId() )
                ++m_driverThreadIds;
            m_pulseGenerator->RequestStopOfPeriodicUpdates( this );
        }
        return ++m_done >= m_rounds;
    }

    private:

    PulseGeneratorPtr m_pulseGenerator;
    UInt32 m_rounds;
    MT::CTAtomic< UInt32 > m_done;
    UInt32 m_driverThreadIds;
};

/*-------------------------------------------------------------------------*/

bool
WaitForBusyWaitRounds( const CBusyWaitRequesterThread& thread, UInt32 rounds, UInt32 timeoutMs )
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
WaitForBusyWaitPulses( const CBusyWaitPulseCounter& counter, UInt32 atLeast, UInt32 timeoutMs )
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

/*-------------------------------------------------------------------------*/

bool
WaitForRunEnd( const CBusyWaitRunThread& thread, UInt32 timeoutMs )
{
    UInt32 waitedMs = 0;
    while ( !thread.HasRunEnded() )
    {
        if ( waitedMs >= timeoutMs )
            return false;
        MT::ThreadDelay( 5 );
        waitedMs += 5;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

/** Remembers the thread of every pulse of a pulse generator */
class CPulseThreadRecorder : public CObserver
{
    public:

    CPulseThreadRecorder( void ) : CObserver(), m_pulses( 0 ), m_lastPulseThread( 0 ), m_firstPulseTicks( 0 ), m_driverToStop( GUCEF_NULL ) {}
    virtual ~CPulseThreadRecorder() { UnsubscribeAllFromObserver(); }

    void Watch( PulseGeneratorPtr pulseGenerator, CBusyWaitPulseGeneratorDriver* driverToStopAtFirstPulse )
    {
        m_driverToStop = driverToStopAtFirstPulse;
        CTEventHandlerFunctor< CPulseThreadRecorder > callback( this, &CPulseThreadRecorder::OnPulse );
        SubscribeTo( pulseGenerator.GetPointerAlways(), CPulseGenerator::PulseEvent, callback );
    }

    void SetDriverToStopAtFirstPulse( CBusyWaitPulseGeneratorDriver* driver ) { m_driverToStop = driver; }

    UInt32 GetPulses( void ) const { return m_pulses.Load(); }
    UInt32 GetLastPulseThread( void ) const { return m_lastPulseThread.Load(); }
    UInt64 GetFirstPulseTicks( void ) const { return m_firstPulseTicks.Load(); }

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        static const CString typeName = "CPulseThreadRecorder";
        return typeName;
    }

    private:

    void OnPulse( CNotifier* notifier, const CEvent& eventId, CICloneable* eventData )
    {
        if ( 0 == m_pulses.Load() )
            m_firstPulseTicks.Store( MT::PrecisionTickCount() );
        m_lastPulseThread.Store( MT::GetCurrentTaskID() );
        ++m_pulses;
        if ( GUCEF_NULL != m_driverToStop )
            m_driverToStop->RequestStopOfPeriodicUpdates( *static_cast< CPulseGenerator* >( notifier ) );
    }

    MT::CTAtomic< UInt32 > m_pulses;
    MT::CTAtomic< UInt32 > m_lastPulseThread;
    MT::CTAtomic< UInt64 > m_firstPulseTicks;
    CBusyWaitPulseGeneratorDriver* m_driverToStop;
};

/*-------------------------------------------------------------------------*/

/** Makes one RequestImmediatePulseThroughDriver() on a thread of its own */
class CThroughDriverRequesterThread : public MT::CActiveObject
{
    public:

    explicit CThroughDriverRequesterThread( const PulseGeneratorPtr& pulseGenerator )
        : MT::CActiveObject(), m_pulseGenerator( pulseGenerator ), m_thread( 0 ), m_taken( 0 ), m_done( 0 ) {}
    virtual ~CThroughDriverRequesterThread() {}

    bool IsDone( void ) const { return 0 != m_done.Load(); }
    bool WasTaken( void ) const { return 0 != m_taken.Load(); }
    UInt32 GetRequestThread( void ) const { return m_thread.Load(); }

    virtual const MT::CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NULL; }

    protected:

    virtual bool OnThreadStart( void* taskdata ) GUCEF_VIRTUAL_OVERRIDE { return true; }

    virtual bool OnThreadCycle( void* taskdata ) GUCEF_VIRTUAL_OVERRIDE
    {
        m_thread.Store( MT::GetCurrentTaskID() );
        m_taken.Store( m_pulseGenerator->RequestImmediatePulseThroughDriver() ? 1 : 0 );
        m_done.Store( 1 );
        return true;
    }

    private:

    PulseGeneratorPtr m_pulseGenerator;
    MT::CTAtomic< UInt32 > m_thread;
    MT::CTAtomic< UInt32 > m_taken;
    MT::CTAtomic< UInt32 > m_done;
};

/*-------------------------------------------------------------------------*/

bool
WaitUntilDone( const CThroughDriverRequesterThread& thread, UInt32 timeoutMs )
{
    for ( UInt32 waitedMs=0; !thread.IsDone() && waitedMs<timeoutMs; ++waitedMs )
        MT::ThreadDelay( 1 );
    return thread.IsDone();
}

/*-------------------------------------------------------------------------*/

/** Takes the data lock of a pulse generator (through a call that takes it) on a thread of its own */
class CDataLockTakerThread : public MT::CActiveObject
{
    public:

    explicit CDataLockTakerThread( CPulseGenerator& pulseGenerator ) : MT::CActiveObject(), m_pulseGenerator( pulseGenerator ), m_done( 0 ) {}
    virtual ~CDataLockTakerThread() {}

    bool IsDone( void ) const { return 0 != m_done.Load(); }

    virtual const MT::CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NULL; }

    protected:

    virtual bool OnThreadStart( void* taskdata ) GUCEF_VIRTUAL_OVERRIDE { return true; }

    virtual bool OnThreadCycle( void* taskdata ) GUCEF_VIRTUAL_OVERRIDE
    {
        m_pulseGenerator.GetNrOfPeriodicPulseRequestors();
        m_done.Store( 1 );
        return true;
    }

    private:

    CPulseGenerator& m_pulseGenerator;
    MT::CTAtomic< UInt32 > m_done;
};

/*-------------------------------------------------------------------------*/

/**
 *  A driver that takes immediate pulse requests from any thread without pulsing. On request it can look whether another
 *  thread can take the data lock of the pulse generator meanwhile, or hold the request until it is released.
 */
class CInspectingPulseDriver : public CIPulseGeneratorDriver
{
    public:

    CInspectingPulseDriver( void )
        : CIPulseGeneratorDriver(), requests( 0 ), dataLockFreeDuringRequest( 0 ), holdRequests( 0 ), requestsHeld( 0 ), m_dataLockTaker( GUCEF_NULL ) {}
    virtual ~CInspectingPulseDriver() {}

    /** At the next request the taker tries the data lock: it outlives the request, it may have to wait for its end */
    void CheckDataLockAtNextRequest( CDataLockTakerThread* taker ) { m_dataLockTaker = taker; }

    virtual void RequestImmediatePulse( CPulseGenerator& pulseGenerator ) GUCEF_VIRTUAL_OVERRIDE
    {
        ++requests;
        CDataLockTakerThread* taker = m_dataLockTaker;
        m_dataLockTaker = GUCEF_NULL;
        if ( GUCEF_NULL != taker && taker->Activate( GUCEF_NULL, 1, 1 ) )
        {
            for ( UInt32 i=0; i<500 && !taker->IsDone(); ++i )
                MT::ThreadDelay( 1 );
            if ( taker->IsDone() )
                ++dataLockFreeDuringRequest;
        }
        if ( 0 != holdRequests.Load() )
        {
            ++requestsHeld;
            for ( UInt32 i=0; i<5000 && 0 != holdRequests.Load(); ++i )
                MT::ThreadDelay( 1 );
            --requestsHeld;
        }
    }

    virtual void RequestPulsesPerImmediatePulseRequest( CPulseGenerator&, const Int32 ) GUCEF_VIRTUAL_OVERRIDE {}
    virtual void RequestPeriodicPulses( CPulseGenerator&, const UInt32 ) GUCEF_VIRTUAL_OVERRIDE {}
    virtual void RequestPulseInterval( CPulseGenerator&, const UInt32 ) GUCEF_VIRTUAL_OVERRIDE {}
    virtual void RequestStopOfPeriodicUpdates( CPulseGenerator& ) GUCEF_VIRTUAL_OVERRIDE {}
    virtual UInt32 GetPulseDriverThreadId( void ) const GUCEF_VIRTUAL_OVERRIDE { return 0; }

    MT::CTAtomic< UInt32 > requests;
    MT::CTAtomic< UInt32 > dataLockFreeDuringRequest;
    MT::CTAtomic< UInt32 > holdRequests;
    MT::CTAtomic< UInt32 > requestsHeld;

    private:

    CDataLockTakerThread* m_dataLockTaker;
};

/*-------------------------------------------------------------------------*/

/**
 *  A driver that holds a call of GetPulseDriverThreadId() or RequestPeriodicPulses() until it is released, and that
 *  counts the calls it gets after it was destroyed (its memory still holds the mark the destructor left)
 */
class CHoldingPulseDriver : public CIPulseGeneratorDriver
{
    public:

    static const UInt32 AliveMark = 0x600DF00D;
    static MT::CTAtomic< UInt32 > callsAfterDestruction;

    CHoldingPulseDriver( void )
        : CIPulseGeneratorDriver(), holdThreadIdCalls( 0 ), holdPeriodicCalls( 0 ), callsHeld( 0 ), calls( 0 ), m_alive( AliveMark ) {}
    virtual ~CHoldingPulseDriver() { m_alive.Store( 0 ); }

    virtual void RequestImmediatePulse( CPulseGenerator& ) GUCEF_VIRTUAL_OVERRIDE { Called(); }
    virtual void QueueImmediatePulse( CPulseGenerator& ) GUCEF_VIRTUAL_OVERRIDE { Called(); }
    virtual void RequestPulsesPerImmediatePulseRequest( CPulseGenerator&, const Int32 ) GUCEF_VIRTUAL_OVERRIDE { Called(); }
    virtual void RequestPulseInterval( CPulseGenerator&, const UInt32 ) GUCEF_VIRTUAL_OVERRIDE { Called(); }
    virtual void RequestStopOfPeriodicUpdates( CPulseGenerator& ) GUCEF_VIRTUAL_OVERRIDE { Called(); }

    virtual void RequestPeriodicPulses( CPulseGenerator&, const UInt32 ) GUCEF_VIRTUAL_OVERRIDE
    {
        Called();
        Hold( holdPeriodicCalls );
    }

    virtual UInt32 GetPulseDriverThreadId( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        Called();
        Hold( holdThreadIdCalls );
        return 4242;
    }

    mutable MT::CTAtomic< UInt32 > holdThreadIdCalls;
    mutable MT::CTAtomic< UInt32 > holdPeriodicCalls;
    mutable MT::CTAtomic< UInt32 > callsHeld;
    mutable MT::CTAtomic< UInt32 > calls;

    private:

    void Called( void ) const
    {
        if ( AliveMark != m_alive.Load() )
            callsAfterDestruction.FetchAdd( 1 );
        calls.FetchAdd( 1 );
    }

    void Hold( MT::CTAtomic< UInt32 >& hold ) const
    {
        if ( 0 == hold.Load() )
            return;
        callsHeld.FetchAdd( 1 );
        for ( UInt32 i=0; i<5000 && 0 != hold.Load(); ++i )
            MT::ThreadDelay( 1 );
        callsHeld.FetchSub( 1 );
    }

    MT::CTAtomic< UInt32 > m_alive;
};

MT::CTAtomic< UInt32 > CHoldingPulseDriver::callsAfterDestruction( 0 );

/*-------------------------------------------------------------------------*/

/** Asks a pulse generator for its driver's thread, or for periodic pulses, once or in a loop, on a thread of its own */
class CDriverUserThread : public MT::CActiveObject
{
    public:

    enum ECall { CALL_THREAD_ID = 0, CALL_PERIODIC = 1, CALL_THROUGH_DRIVER = 2 };

    CDriverUserThread( const PulseGeneratorPtr& pulseGenerator, ECall call, bool loop )
        : MT::CActiveObject(), m_pulseGenerator( pulseGenerator ), m_call( call ), m_loop( loop ), m_stop( 0 ), m_done( 0 ), m_rounds( 0 ) {}
    virtual ~CDriverUserThread() {}

    bool IsDone( void ) const { return 0 != m_done.Load(); }
    UInt32 GetRounds( void ) const { return m_rounds.Load(); }
    void Stop( void ) { m_stop.Store( 1 ); }

    virtual const MT::CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NULL; }

    protected:

    virtual bool OnThreadStart( void* taskdata ) GUCEF_VIRTUAL_OVERRIDE { return true; }

    virtual bool OnThreadCycle( void* taskdata ) GUCEF_VIRTUAL_OVERRIDE
    {
        do
        {
            switch ( m_call )
            {
                case CALL_THREAD_ID: m_pulseGenerator->GetPulseDriverThreadId(); break;
                case CALL_PERIODIC: m_pulseGenerator->RequestPeriodicPulses( this, 10 ); break;
                default: m_pulseGenerator->RequestImmediatePulseThroughDriver(); break;
            }
            m_rounds.FetchAdd( 1 );
        }
        while ( m_loop && 0 == m_stop.Load() );
        m_done.Store( 1 );
        return true;
    }

    private:

    PulseGeneratorPtr m_pulseGenerator;
    ECall m_call;
    bool m_loop;
    MT::CTAtomic< UInt32 > m_stop;
    MT::CTAtomic< UInt32 > m_done;
    MT::CTAtomic< UInt32 > m_rounds;
};

/*-------------------------------------------------------------------------*/

/** Removes the driver of a pulse generator on a thread of its own */
class CDriverRemoverThread : public MT::CActiveObject
{
    public:

    explicit CDriverRemoverThread( const PulseGeneratorPtr& pulseGenerator ) : MT::CActiveObject(), m_pulseGenerator( pulseGenerator ), m_done( 0 ) {}
    virtual ~CDriverRemoverThread() {}

    bool IsDone( void ) const { return 0 != m_done.Load(); }

    virtual const MT::CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NULL; }

    protected:

    virtual bool OnThreadStart( void* taskdata ) GUCEF_VIRTUAL_OVERRIDE { return true; }

    virtual bool OnThreadCycle( void* taskdata ) GUCEF_VIRTUAL_OVERRIDE
    {
        m_pulseGenerator->SetPulseGeneratorDriver( GUCEF_NULL );
        m_done.Store( 1 );
        return true;
    }

    private:

    PulseGeneratorPtr m_pulseGenerator;
    MT::CTAtomic< UInt32 > m_done;
};

}

/*-------------------------------------------------------------------------*/

void
PerformBusyWaitPulseGeneratorDriverTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "BusyWaitPulseGeneratorDriver" );

    GUCEF_TESTFW_TESTCASE( "Pulse requests from other threads while the busy wait loop pulses: the loop pulses on and the last stop ends it" )
        try
        {
            PulseGeneratorPtr pulseGenerator = CPulseGenerator::CreateSharedObj();
            CBusyWaitPulseGeneratorDriver driver;
            pulseGenerator->SetPulseGeneratorDriver( &driver );

            CBusyWaitPulseCounter counter;
            counter.Watch( pulseGenerator );

            // a requestor for the whole test, so only its stop at the end ends the loop
            int keepAlive = 0;
            pulseGenerator->RequestPeriodicPulses( &keepAlive, 5 );

            CBusyWaitRunThread runThread( driver, pulseGenerator );
            ASSERT_TRUE( runThread.Activate( GUCEF_NULL, 1, 1 ) );
            ASSERT_TRUE( WaitForBusyWaitPulses( counter, 5, 10000 ) );

            // two threads ask for pulses and stop them again, as fast as they can, while the loop pulses
            const UInt32 rounds = 40;
            CBusyWaitRequesterThread first( pulseGenerator, rounds );
            CBusyWaitRequesterThread second( pulseGenerator, rounds );
            ASSERT_TRUE( first.Activate( GUCEF_NULL, 1, 1 ) );
            ASSERT_TRUE( second.Activate( GUCEF_NULL, 1, 1 ) );
            ASSERT_TRUE( WaitForBusyWaitRounds( first, rounds, 120000 ) );
            ASSERT_TRUE( WaitForBusyWaitRounds( second, rounds, 120000 ) );
            first.Deactivate( false, true );
            second.Deactivate( false, true );
            ASSERT_TRUE( 1 == pulseGenerator->GetNrOfPeriodicPulseRequestors() );

            // the loop still pulses
            UInt32 before = counter.GetPulses();
            ASSERT_TRUE( WaitForBusyWaitPulses( counter, before + 5, 10000 ) );
            ASSERT_FALSE( runThread.HasRunEnded() );

            // the stop of the last requestor ends the loop
            pulseGenerator->RequestStopOfPeriodicUpdates( &keepAlive );
            ASSERT_TRUE( 0 == pulseGenerator->GetNrOfPeriodicPulseRequestors() );
            ASSERT_TRUE( WaitForRunEnd( runThread, 10000 ) );
            if ( !runThread.HasRunEnded() )
                driver.RequestStopOfPeriodicUpdates( *pulseGenerator );    // so the thread can be joined after a failure
            runThread.Deactivate( false, true );

            counter.UnsubscribeAllFromObserver();
            pulseGenerator->SetPulseGeneratorDriver( GUCEF_NULL );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "A pulse request through the driver made on another thread never pulses on that thread, the busy wait loop pulses for it on its own" )
        try
        {
            PulseGeneratorPtr pulseGenerator = CPulseGenerator::CreateSharedObj();
            CBusyWaitPulseGeneratorDriver driver;
            pulseGenerator->SetPulseGeneratorDriver( &driver );

            // the loop of the driver does not run, as for the pulse generator of an application before its main loop
            CPulseThreadRecorder recorder;
            recorder.Watch( pulseGenerator, GUCEF_NULL );
            CThroughDriverRequesterThread requester( pulseGenerator );
            ASSERT_TRUE( requester.Activate( GUCEF_NULL, 1, 1 ) );
            ASSERT_TRUE( WaitUntilDone( requester, 5000 ) );
            requester.Deactivate( false, true );
            GUCEF_LOG( LOGLEVEL_NORMAL, "BusyWaitPulseGeneratorDriver: request through the driver from thread " << requester.GetRequestThread()
                       << ": " << recorder.GetPulses() << " pulses meanwhile, the last on thread " << recorder.GetLastPulseThread() );
            ASSERT_TRUE( requester.WasTaken() );
            ASSERT_TRUE( 0 == recorder.GetPulses() );

            // the loop takes the request when it runs: its first pulse comes at once instead of after its pulse window
            recorder.SetDriverToStopAtFirstPulse( &driver );
            const UInt64 runStartTicks = MT::PrecisionTickCount();
            driver.Run( *pulseGenerator, 1000, 1000 );
            ASSERT_TRUE( recorder.GetPulses() >= 1 );
            ASSERT_TRUE( MT::GetCurrentTaskID() == recorder.GetLastPulseThread() );
            const UInt64 firstPulseTicks = recorder.GetFirstPulseTicks();
            const Float64 firstPulseMs = firstPulseTicks >= runStartTicks ? MT::PrecisionTimerTicksToMs( firstPulseTicks - runStartTicks ) : -1.0;
            GUCEF_LOG( LOGLEVEL_NORMAL, "BusyWaitPulseGeneratorDriver: first pulse of the loop after " << firstPulseMs << " ms" );
            ASSERT_TRUE( firstPulseMs >= 0.0 && firstPulseMs < 500.0 );

            recorder.UnsubscribeAllFromObserver();
            pulseGenerator->SetPulseGeneratorDriver( GUCEF_NULL );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "A pulse request through the driver calls the driver without holding the data lock of the pulse generator" )
        try
        {
            PulseGeneratorPtr pulseGenerator = CPulseGenerator::CreateSharedObj();
            CInspectingPulseDriver driver;
            pulseGenerator->SetPulseGeneratorDriver( &driver );

            CDataLockTakerThread taker( *pulseGenerator );
            driver.CheckDataLockAtNextRequest( &taker );
            const UInt32 requestsBefore = driver.requests.Load();
            const UInt32 freeBefore = driver.dataLockFreeDuringRequest.Load();
            ASSERT_TRUE( pulseGenerator->RequestImmediatePulseThroughDriver() );
            ASSERT_TRUE( requestsBefore + 1 == driver.requests.Load() );
            ASSERT_TRUE( freeBefore + 1 == driver.dataLockFreeDuringRequest.Load() );

            // the taker gets the lock once the request returned in any case
            for ( UInt32 i=0; i<5000 && !taker.IsDone(); ++i )
                MT::ThreadDelay( 1 );
            ASSERT_TRUE( taker.IsDone() );
            taker.Deactivate( false, true );
            pulseGenerator->SetPulseGeneratorDriver( GUCEF_NULL );
            ASSERT_FALSE( pulseGenerator->RequestImmediatePulseThroughDriver() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Removing the driver waits for a pulse request through the driver that is still inside the driver" )
        try
        {
            PulseGeneratorPtr pulseGenerator = CPulseGenerator::CreateSharedObj();
            CInspectingPulseDriver driver;
            pulseGenerator->SetPulseGeneratorDriver( &driver );

            driver.holdRequests.Store( 1 );
            CThroughDriverRequesterThread requester( pulseGenerator );
            ASSERT_TRUE( requester.Activate( GUCEF_NULL, 1, 1 ) );
            for ( UInt32 i=0; i<5000 && 0 == driver.requestsHeld.Load(); ++i )
                MT::ThreadDelay( 1 );
            ASSERT_TRUE( 1 == driver.requestsHeld.Load() );

            CDriverRemoverThread remover( pulseGenerator );
            ASSERT_TRUE( remover.Activate( GUCEF_NULL, 1, 1 ) );
            MT::ThreadDelay( 100 );
            ASSERT_FALSE( remover.IsDone() );       // the driver may only go once no request is inside it

            driver.holdRequests.Store( 0 );
            for ( UInt32 i=0; i<5000 && !remover.IsDone(); ++i )
                MT::ThreadDelay( 1 );
            ASSERT_TRUE( remover.IsDone() );
            ASSERT_TRUE( WaitUntilDone( requester, 5000 ) );
            ASSERT_TRUE( requester.WasTaken() );
            remover.Deactivate( false, true );
            requester.Deactivate( false, true );
            ASSERT_FALSE( pulseGenerator->RequestImmediatePulseThroughDriver() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Removing the driver waits for a call of GetPulseDriverThreadId() or RequestPeriodicPulses() that is still inside the driver" )
        try
        {
            for ( UInt32 c=0; c<2; ++c )
            {
                const bool threadIdCall = 0 == c;
                PulseGeneratorPtr pulseGenerator = CPulseGenerator::CreateSharedObj();
                CHoldingPulseDriver driver;
                pulseGenerator->SetPulseGeneratorDriver( &driver );

                if ( threadIdCall )
                    driver.holdThreadIdCalls.Store( 1 );
                else
                    driver.holdPeriodicCalls.Store( 1 );
                CDriverUserThread user( pulseGenerator, threadIdCall ? CDriverUserThread::CALL_THREAD_ID : CDriverUserThread::CALL_PERIODIC, false );
                ASSERT_TRUE( user.Activate( GUCEF_NULL, 1, 1 ) );
                for ( UInt32 i=0; i<5000 && 0 == driver.callsHeld.Load(); ++i )
                    MT::ThreadDelay( 1 );
                ASSERT_TRUE( 1 == driver.callsHeld.Load() );

                CDriverRemoverThread remover( pulseGenerator );
                ASSERT_TRUE( remover.Activate( GUCEF_NULL, 1, 1 ) );
                MT::ThreadDelay( 100 );
                ASSERT_FALSE( remover.IsDone() );       // the driver may only go once no call is inside it

                driver.holdThreadIdCalls.Store( 0 );
                driver.holdPeriodicCalls.Store( 0 );
                for ( UInt32 i=0; i<5000 && ( !remover.IsDone() || !user.IsDone() ); ++i )
                    MT::ThreadDelay( 1 );
                ASSERT_TRUE( remover.IsDone() && user.IsDone() );
                remover.Deactivate( false, true );
                user.Deactivate( false, true );
                ASSERT_TRUE( GUCEF_NULL == pulseGenerator->GetPulseGeneratorDriver() );
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Drivers replaced and destroyed over and over while other threads ask for the driver's thread, for periodic pulses and for pulses through the driver: no call reaches a destroyed driver" )
        try
        {
            CHoldingPulseDriver::callsAfterDestruction.Store( 0 );
            PulseGeneratorPtr pulseGenerator = CPulseGenerator::CreateSharedObj();
            CDriverUserThread threadIdUser( pulseGenerator, CDriverUserThread::CALL_THREAD_ID, true );
            CDriverUserThread periodicUser( pulseGenerator, CDriverUserThread::CALL_PERIODIC, true );
            CDriverUserThread throughDriverUser( pulseGenerator, CDriverUserThread::CALL_THROUGH_DRIVER, true );
            ASSERT_TRUE( threadIdUser.Activate( GUCEF_NULL, 1, 1 ) );
            ASSERT_TRUE( periodicUser.Activate( GUCEF_NULL, 1, 1 ) );
            ASSERT_TRUE( throughDriverUser.Activate( GUCEF_NULL, 1, 1 ) );

            // the drivers are on the heap and go as soon as they are replaced, as the driver of a node task does
            CHoldingPulseDriver* current = GUCEF_NULL;
            UInt32 replacements = 0;
            const UInt64 startTicks = MT::PrecisionTickCount();
            while ( ( MT::PrecisionTickCount() - startTicks ) * 1000 / MT::PrecisionTimerResolution() < 1500 )
            {
                CHoldingPulseDriver* next = GUCEF_NEW CHoldingPulseDriver();
                pulseGenerator->SetPulseGeneratorDriver( next );
                GUCEF_DELETE current;
                current = next;
                ++replacements;
            }
            pulseGenerator->SetPulseGeneratorDriver( GUCEF_NULL );
            GUCEF_DELETE current;
            current = GUCEF_NULL;

            threadIdUser.Stop();
            periodicUser.Stop();
            throughDriverUser.Stop();
            for ( UInt32 i=0; i<5000 && ( !threadIdUser.IsDone() || !periodicUser.IsDone() || !throughDriverUser.IsDone() ); ++i )
                MT::ThreadDelay( 1 );
            ASSERT_TRUE( threadIdUser.IsDone() && periodicUser.IsDone() && throughDriverUser.IsDone() );
            threadIdUser.Deactivate( false, true );
            periodicUser.Deactivate( false, true );
            throughDriverUser.Deactivate( false, true );

            GUCEF_LOG( LOGLEVEL_NORMAL, "Driver replacements: " << replacements << ", thread id calls " << threadIdUser.GetRounds() << ", periodic requests "
                << periodicUser.GetRounds() << ", requests through the driver " << throughDriverUser.GetRounds() << ", calls of destroyed drivers "
                << CHoldingPulseDriver::callsAfterDestruction.Load() );
            ASSERT_TRUE( replacements > 100 );
            ASSERT_TRUE( 0 == CHoldingPulseDriver::callsAfterDestruction.Load() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;
}

/*-------------------------------------------------------------------------*/
