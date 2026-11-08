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

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_MT_CTATOMIC_H
#include "gucefMT_CTAtomic.h"
#define GUCEF_MT_CTATOMIC_H
#endif /* GUCEF_MT_CTATOMIC_H ? */

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_CORE_CPULSEGENERATOR_H
#include "gucefCORE_CPulseGenerator.h"
#define GUCEF_CORE_CPULSEGENERATOR_H
#endif /* GUCEF_CORE_CPULSEGENERATOR_H ? */

#ifndef GUCEF_CORE_CPUMPEDOBSERVER_H
#include "CPumpedObserver.h"
#define GUCEF_CORE_CPUMPEDOBSERVER_H
#endif /* GUCEF_CORE_CPUMPEDOBSERVER_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestPumpedObserverMail.h"

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

/** How long a step of a case waits for another thread before it counts as failed */
static const UInt32 MailWaitTimeInMs = 5000;

/** How long a thread that has to wait is watched for not waiting */
static const UInt32 MailMustWaitTimeInMs = 200;

/** Mail per producer thread in the order case */
static const UInt32 MailPerProducer = 2000;

/** Producer threads in the order case */
static const UInt32 MailProducers = 3;

/** Copies of CMailValue that exist, the mailbox's included */
static MT::CTAtomic< Int32 > g_liveMailValues( 0 );

/*-------------------------------------------------------------------------*/

static const CORE::CEvent&
MailTestEvent( void )
{
    static const CORE::CEvent event( "GUCEF::CORE::TestPumpedObserverMail::MailTestEvent" );
    event.EnsureInitialized();
    return event;
}

/*-------------------------------------------------------------------------*/

static UInt64
MailElapsedMs( UInt64 startTicks )
{
    return ( MT::PrecisionTickCount() - startTicks ) * 1000 / MT::PrecisionTimerResolution();
}

/*-------------------------------------------------------------------------*/

/** Waits until the flag is set or the time is up; true when it is set */
static bool
MailWaitFor( const MT::CTAtomic< bool >& flag, UInt32 timeoutInMs = MailWaitTimeInMs )
{
    const UInt64 startTicks = MT::PrecisionTickCount();
    while ( !flag.Load( MT::MEMORDER_ACQUIRE ) )
    {
        if ( MailElapsedMs( startTicks ) > timeoutInMs )
            return false;
        MT::ThreadDelay( 1 );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

/** The data of a mail: who sent it and its number, and how many copies of it exist */
class CMailValue : public CORE::CICloneable
{
    public:

    UInt32 producer;
    UInt32 sequence;

    CMailValue( UInt32 p, UInt32 s ) : CORE::CICloneable(), producer( p ), sequence( s ) { g_liveMailValues.FetchAdd( 1 ); }
    CMailValue( const CMailValue& src ) : CORE::CICloneable( src ), producer( src.producer ), sequence( src.sequence ) { g_liveMailValues.FetchAdd( 1 ); }
    virtual ~CMailValue() { g_liveMailValues.FetchSub( 1 ); }

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NEW CMailValue( *this ); }
};

/*-------------------------------------------------------------------------*/

/** A notifier whose events the test raises */
class CMailTestNotifier : public CORE::CNotifier
{
    public:

    CMailTestNotifier( void ) : CORE::CNotifier() {}
    virtual ~CMailTestNotifier() {}

    void Raise( UInt32 producer, UInt32 sequence )
    {
        CMailValue value( producer, sequence );
        NotifyObservers( MailTestEvent(), &value );
    }

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        static const CORE::CString classTypeName = "CMailTestNotifier";
        return classTypeName;
    }
};

/*-------------------------------------------------------------------------*/

/** One mail as it was delivered */
struct SMailRecord
{
    CORE::CNotifier* notifier;
    UInt32 producer;
    UInt32 sequence;
};

typedef GUCEF::vector< SMailRecord > TMailRecordVector;

/*-------------------------------------------------------------------------*/

/** Adds one mail to an observer from a thread of its own once it is told to */
struct SMailLateSender
{
    CORE::CPumpedObserver* observer;
    MT::CTAtomic< bool > go;
    MT::CTAtomic< bool > done;

    SMailLateSender( void ) : observer( GUCEF_NULL ), go( false ), done( false ) {}
};

/*-------------------------------------------------------------------------*/

/**
 *  Keeps every mail it gets, in delivery order. Optionally it holds the delivery of one mail until it is released
 *  (and then reads the mail's data again), or adds mail itself and has another thread add mail while
 *  one mail is delivered
 */
class CMailRecorder : public CORE::CPumpedObserver
{
    public:

    MT::CTAtomic< UInt32 > holdSequence;         /**< the mail whose delivery waits for release, 0 for none */
    MT::CTAtomic< bool > holding;
    MT::CTAtomic< bool > release;
    MT::CTAtomic< bool > heldDataIntact;         /**< after the release the held mail's data was intact */
    MT::CTAtomic< UInt32 > addDuringSequence;    /**< the mail whose delivery adds mail, 0 for none */
    SMailLateSender* lateSender;                 /**< told to add mail during that delivery, waited for */
    MT::CTAtomic< UInt32 > delivered;

    explicit CMailRecorder( CORE::PulseGeneratorPtr pulseGenerator )
        : CORE::CPumpedObserver( pulseGenerator, false )
        , holdSequence( 0 )
        , holding( false )
        , release( false )
        , heldDataIntact( false )
        , addDuringSequence( 0 )
        , lateSender( GUCEF_NULL )
        , delivered( 0 )
        , m_lock()
        , m_records()
    {}

    virtual ~CMailRecorder()
    {
        Shutdown();
    }

    void Mail( CORE::CNotifier* notifier, UInt32 producer, UInt32 sequence )
    {
        CMailValue value( producer, sequence );
        AddEventToMailbox( notifier, MailTestEvent(), &value );
    }

    TMailRecordVector GetRecords( void ) const
    {
        MT::CScopeMutex lock( m_lock );
        return m_records;
    }

    using CORE::CPumpedObserver::SubscribeTo;

    protected:

    virtual void OnPumpedNotify( CORE::CNotifier* notifier         ,
                                 const CORE::CEvent& eventid        ,
                                 CORE::CICloneable* eventdata       ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( MailTestEvent() != eventid || GUCEF_NULL == eventdata )
            return;
        const CMailValue* value = static_cast< const CMailValue* >( eventdata );
        {
            MT::CScopeMutex lock( m_lock );
            SMailRecord record;
            record.notifier = notifier;
            record.producer = value->producer;
            record.sequence = value->sequence;
            m_records.push_back( record );
        }

        if ( 0 != holdSequence.Load() && holdSequence.Load() == value->sequence )
        {
            const UInt32 producer = value->producer;
            const UInt32 sequence = value->sequence;
            holding.Store( true, MT::MEMORDER_RELEASE );
            MailWaitFor( release, 4 * MailWaitTimeInMs );

            // the mail is still there: whoever cleared the mailbox or destroyed the notifier left it to this thread. The
            // notifier is not asked anything: its destruction is under way, in the destructor of CNotifier, which waits
            const bool intact = value->producer == producer && value->sequence == sequence;
            heldDataIntact.Store( intact, MT::MEMORDER_RELEASE );
        }

        if ( 0 != addDuringSequence.Load() && addDuringSequence.Load() == value->sequence )
        {
            Mail( notifier, value->producer, value->sequence + 1 );
            if ( GUCEF_NULL != lateSender )
            {
                lateSender->go.Store( true, MT::MEMORDER_RELEASE );
                MailWaitFor( lateSender->done );
            }
        }
        delivered.FetchAdd( 1 );
    }

    private:

    mutable MT::CMutex m_lock;
    TMailRecordVector m_records;
};

/*-------------------------------------------------------------------------*/

/** A pulse generator that a thread of its own pulses until it is asked to stop */
struct SMailPulser
{
    CORE::PulseGeneratorPtr pulseGenerator;
    MT::CTAtomic< bool > stop;
    MT::SThreadData* thread;

    SMailPulser( void ) : pulseGenerator( CORE::CPulseGenerator::CreateSharedObj() ), stop( false ), thread( GUCEF_NULL ) {}
};

/*-------------------------------------------------------------------------*/

static UInt32 GUCEF_CALLSPEC_PREFIX
MailPulserMain( void* data ) GUCEF_CALLSPEC_SUFFIX
{
    SMailPulser& pulser = *static_cast< SMailPulser* >( data );
    while ( !pulser.stop.Load( MT::MEMORDER_ACQUIRE ) )
    {
        pulser.pulseGenerator->ForceDirectPulse();
        MT::ThreadDelay( 0 );
    }
    return 0;
}

/*-------------------------------------------------------------------------*/

static UInt32 GUCEF_CALLSPEC_PREFIX
MailLateSenderMain( void* data ) GUCEF_CALLSPEC_SUFFIX
{
    SMailLateSender& sender = *static_cast< SMailLateSender* >( data );
    if ( MailWaitFor( sender.go ) )
    {
        CMailValue value( 8, 3 );
        sender.observer->AddEventToMailbox( GUCEF_NULL, MailTestEvent(), &value );
    }
    sender.done.Store( true, MT::MEMORDER_RELEASE );
    return 0;
}

/*-------------------------------------------------------------------------*/

/** Destroys a notifier on a thread of its own */
struct SMailNotifierDestroyer
{
    CMailTestNotifier* notifier;
    MT::CTAtomic< bool > done;

    SMailNotifierDestroyer( void ) : notifier( GUCEF_NULL ), done( false ) {}
};

/*-------------------------------------------------------------------------*/

static UInt32 GUCEF_CALLSPEC_PREFIX
MailNotifierDestroyerMain( void* data ) GUCEF_CALLSPEC_SUFFIX
{
    SMailNotifierDestroyer& destroyer = *static_cast< SMailNotifierDestroyer* >( data );
    GUCEF_DELETE destroyer.notifier;
    destroyer.done.Store( true, MT::MEMORDER_RELEASE );
    return 0;
}

/*-------------------------------------------------------------------------*/

/** Adds numbered mail from a thread of its own */
struct SMailProducer
{
    CMailRecorder* observer;
    CORE::CNotifier* notifier;
    UInt32 producer;

    SMailProducer( void ) : observer( GUCEF_NULL ), notifier( GUCEF_NULL ), producer( 0 ) {}
};

/*-------------------------------------------------------------------------*/

static UInt32 GUCEF_CALLSPEC_PREFIX
MailProducerMain( void* data ) GUCEF_CALLSPEC_SUFFIX
{
    SMailProducer& producer = *static_cast< SMailProducer* >( data );
    for ( UInt32 i=1; i<=MailPerProducer; ++i )
    {
        producer.observer->Mail( producer.notifier, producer.producer, i );
        if ( 0 == i % 64 )
            MT::ThreadDelay( 0 );
    }
    return 0;
}

/*-------------------------------------------------------------------------*/

static MT::SThreadData*
StartMailThread( MT::TThreadFunc threadMain, void* data )
{
    MT::SThreadData* thread = MT::ThreadDataReserve();
    if ( GUCEF_NULL == thread )
        return GUCEF_NULL;
    if ( 0 == MT::ThreadCreate( thread, threadMain, data ) )
    {
        MT::ThreadDataCleanup( thread );
        return GUCEF_NULL;
    }
    return thread;
}

/*-------------------------------------------------------------------------*/

/** Waits for the thread to end, returns whether it did; the thread data of a thread that did not end stays */
static bool
EndMailThread( MT::SThreadData* thread )
{
    if ( GUCEF_NULL == thread )
        return true;
    if ( 1 != MT::ThreadWait( thread, 4 * MailWaitTimeInMs ) )
        return false;
    MT::ThreadDataCleanup( thread );
    return true;
}

/*-------------------------------------------------------------------------*/

static bool
StartMailPulser( SMailPulser& pulser )
{
    pulser.thread = StartMailThread( &MailPulserMain, &pulser );
    return GUCEF_NULL != pulser.thread;
}

/*-------------------------------------------------------------------------*/

static bool
StopMailPulser( SMailPulser& pulser )
{
    pulser.stop.Store( true, MT::MEMORDER_RELEASE );
    const bool ended = EndMailThread( pulser.thread );
    pulser.thread = GUCEF_NULL;
    return ended;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformPumpedObserverMailTests( void )
{
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING pumped observer mail TESTS" );
    GUCEF_TESTFW_SUITE_SCOPE( "PumpedObserverMail" );

    GUCEF_TESTFW_TESTCASE( "DiscardMailOf lets go of the waiting mail of one notifier, its data included, and leaves the mail of others in order" )
        try
        {
            const Int32 liveBefore = g_liveMailValues.Load();
            {
                CORE::PulseGeneratorPtr pulseGenerator = CORE::CPulseGenerator::CreateSharedObj();
                CMailRecorder observer( pulseGenerator );
                CMailTestNotifier discarded;
                CMailTestNotifier kept;
                for ( UInt32 i=1; i<=3; ++i )
                {
                    observer.Mail( &discarded, 1, i );
                    observer.Mail( &kept, 2, i );
                }
                ASSERT_TRUE( liveBefore + 6 == g_liveMailValues.Load() );

                observer.DiscardMailOf( &discarded );
                ASSERT_TRUE( liveBefore + 3 == g_liveMailValues.Load() );

                pulseGenerator->ForceDirectPulse();
                TMailRecordVector records = observer.GetRecords();
                ASSERT_TRUE( 3 == records.size() );
                for ( UInt32 i=0; i<records.size(); ++i )
                    ASSERT_TRUE( &kept == records[ i ].notifier && 2 == records[ i ].producer && i + 1 == records[ i ].sequence );
                ASSERT_TRUE( liveBefore == g_liveMailValues.Load() );

                // nothing of the notifier is left to deliver
                pulseGenerator->ForceDirectPulse();
                ASSERT_TRUE( 3 == observer.GetRecords().size() );
            }
            ASSERT_TRUE( liveBefore == g_liveMailValues.Load() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "A thread that destroys a notifier waits for the delivery of its mail that is under way on the pulse thread" )
        try
        {
            SMailPulser* pulser = GUCEF_NEW SMailPulser();
            CMailRecorder* observer = GUCEF_NEW CMailRecorder( pulser->pulseGenerator );
            SMailNotifierDestroyer* destroyer = GUCEF_NEW SMailNotifierDestroyer();
            destroyer->notifier = GUCEF_NEW CMailTestNotifier();
            observer->SubscribeTo( destroyer->notifier, MailTestEvent() );
            observer->holdSequence.Store( 7 );

            // raised on this thread, which is not the pulse thread: mail
            destroyer->notifier->Raise( 1, 7 );
            ASSERT_TRUE( StartMailPulser( *pulser ) );
            ASSERT_TRUE( MailWaitFor( observer->holding ) );

            MT::SThreadData* destroyerThread = StartMailThread( &MailNotifierDestroyerMain, destroyer );
            ASSERT_TRUE( GUCEF_NULL != destroyerThread );
            MT::ThreadDelay( MailMustWaitTimeInMs );
            ASSERT_FALSE( destroyer->done.Load() );

            observer->release.Store( true, MT::MEMORDER_RELEASE );
            ASSERT_TRUE( MailWaitFor( destroyer->done ) );
            const bool destroyerEnded = EndMailThread( destroyerThread );
            ASSERT_TRUE( destroyerEnded );
            ASSERT_TRUE( observer->heldDataIntact.Load() );
            ASSERT_TRUE( 1 == observer->delivered.Load() );

            const bool pulserEnded = StopMailPulser( *pulser );
            ASSERT_TRUE( pulserEnded );
            if ( destroyerEnded && pulserEnded )
            {
                GUCEF_DELETE destroyer;
                GUCEF_DELETE observer;
                GUCEF_DELETE pulser;
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "ClearMailbox lets go of the waiting mail and leaves the mail being delivered to the pulse thread; new mail is delivered after it" )
        try
        {
            const Int32 liveBefore = g_liveMailValues.Load();
            SMailPulser* pulser = GUCEF_NEW SMailPulser();
            CMailRecorder* observer = GUCEF_NEW CMailRecorder( pulser->pulseGenerator );
            CMailTestNotifier notifier;
            observer->holdSequence.Store( 1 );
            for ( UInt32 i=1; i<=3; ++i )
                observer->Mail( &notifier, 1, i );

            ASSERT_TRUE( StartMailPulser( *pulser ) );
            ASSERT_TRUE( MailWaitFor( observer->holding ) );

            // the pulse thread delivers mail 1 now: 2 and 3 go, 1 stays with that thread
            observer->ClearMailbox( true );
            ASSERT_TRUE( liveBefore + 1 == g_liveMailValues.Load() );
            observer->Mail( &notifier, 1, 4 );

            observer->release.Store( true, MT::MEMORDER_RELEASE );
            const UInt64 startTicks = MT::PrecisionTickCount();
            while ( observer->delivered.Load() < 2 && MailElapsedMs( startTicks ) < MailWaitTimeInMs )
                MT::ThreadDelay( 1 );
            MT::ThreadDelay( 20 );

            const bool pulserEnded = StopMailPulser( *pulser );
            ASSERT_TRUE( pulserEnded );
            ASSERT_TRUE( observer->heldDataIntact.Load() );
            TMailRecordVector records = observer->GetRecords();
            ASSERT_TRUE( 2 == records.size() );
            if ( 2 == records.size() )
                ASSERT_TRUE( 1 == records[ 0 ].sequence && 4 == records[ 1 ].sequence );
            ASSERT_TRUE( liveBefore == g_liveMailValues.Load() );
            if ( pulserEnded )
            {
                GUCEF_DELETE observer;
                GUCEF_DELETE pulser;
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Mail is delivered in the order it arrived, per sender, while other threads add mail during the deliveries" )
        try
        {
            SMailPulser* pulser = GUCEF_NEW SMailPulser();
            CMailRecorder* observer = GUCEF_NEW CMailRecorder( pulser->pulseGenerator );
            CMailTestNotifier notifier;
            ASSERT_TRUE( StartMailPulser( *pulser ) );

            SMailProducer producers[ MailProducers ];
            MT::SThreadData* threads[ MailProducers ];
            for ( UInt32 p=0; p<MailProducers; ++p )
            {
                producers[ p ].observer = observer;
                producers[ p ].notifier = &notifier;
                producers[ p ].producer = p + 1;
                threads[ p ] = StartMailThread( &MailProducerMain, &producers[ p ] );
                ASSERT_TRUE( GUCEF_NULL != threads[ p ] );
            }
            bool producersEnded = true;
            for ( UInt32 p=0; p<MailProducers; ++p )
                producersEnded = EndMailThread( threads[ p ] ) && producersEnded;
            ASSERT_TRUE( producersEnded );

            const UInt32 expected = MailProducers * MailPerProducer;
            const UInt64 startTicks = MT::PrecisionTickCount();
            while ( observer->delivered.Load() < expected && MailElapsedMs( startTicks ) < 6 * MailWaitTimeInMs )
                MT::ThreadDelay( 1 );
            const bool pulserEnded = StopMailPulser( *pulser );
            ASSERT_TRUE( pulserEnded );

            TMailRecordVector records = observer->GetRecords();
            ASSERT_TRUE( expected == records.size() );
            UInt32 next[ MailProducers ];
            for ( UInt32 p=0; p<MailProducers; ++p )
                next[ p ] = 1;
            bool inOrder = true;
            for ( UInt32 i=0; i<records.size(); ++i )
            {
                const UInt32 p = records[ i ].producer - 1;
                if ( p >= MailProducers || records[ i ].sequence != next[ p ] )
                {
                    inOrder = false;
                    break;
                }
                ++next[ p ];
            }
            ASSERT_TRUE( inOrder );
            if ( pulserEnded && producersEnded )
            {
                GUCEF_DELETE observer;
                GUCEF_DELETE pulser;
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Mail that arrives during a pulse, from the delivery itself or from another thread, is delivered at the next pulse" )
        try
        {
            CORE::PulseGeneratorPtr pulseGenerator = CORE::CPulseGenerator::CreateSharedObj();
            CMailRecorder observer( pulseGenerator );
            SMailLateSender* lateSender = GUCEF_NEW SMailLateSender();
            lateSender->observer = &observer;
            observer.lateSender = lateSender;
            observer.addDuringSequence.Store( 1 );
            MT::SThreadData* senderThread = StartMailThread( &MailLateSenderMain, lateSender );
            ASSERT_TRUE( GUCEF_NULL != senderThread );

            // mail 1 adds mail 2 from within its delivery and has the other thread add mail 3 meanwhile
            observer.Mail( GUCEF_NULL, 9, 1 );
            pulseGenerator->ForceDirectPulse();
            TMailRecordVector records = observer.GetRecords();
            ASSERT_TRUE( 1 == records.size() && 1 == records[ 0 ].sequence );
            ASSERT_TRUE( lateSender->done.Load() );

            pulseGenerator->ForceDirectPulse();
            records = observer.GetRecords();
            ASSERT_TRUE( 3 == records.size() );
            if ( 3 == records.size() )
                ASSERT_TRUE( 2 == records[ 1 ].sequence && 3 == records[ 2 ].sequence );

            const bool senderEnded = EndMailThread( senderThread );
            ASSERT_TRUE( senderEnded );
            observer.lateSender = GUCEF_NULL;
            if ( senderEnded )
                GUCEF_DELETE lateSender;
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
