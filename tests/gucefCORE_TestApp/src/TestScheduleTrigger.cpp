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

#ifndef GUCEF_CORE_CSCHEDULETRIGGER_H
#include "gucefCORE_CScheduleTrigger.h"

#ifndef GUCEF_CORE_CPULSEGENERATOR_H
#include "gucefCORE_CPulseGenerator.h"
#define GUCEF_CORE_CPULSEGENERATOR_H
#endif /* GUCEF_CORE_CPULSEGENERATOR_H ? */
#define GUCEF_CORE_CSCHEDULETRIGGER_H
#endif /* GUCEF_CORE_CSCHEDULETRIGGER_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestScheduleTrigger.h"

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
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

class CScheduleTriggerListener : public CORE::CObservingNotifier
{
    public:

    UInt32 triggeredCount;
    UInt32 endedCount;
    CORE::CDateTime lastScheduledTime;
    CORE::CDateTime lastFiredTime;
    UInt64 lastOccurrenceCount;
    CORE::CScheduleTrigger* disableOnTrigger;

    CScheduleTriggerListener( CORE::CScheduleTrigger& trigger )
        : CORE::CObservingNotifier()
        , triggeredCount( 0 )
        , endedCount( 0 )
        , lastScheduledTime()
        , lastFiredTime()
        , lastOccurrenceCount( 0 )
        , disableOnTrigger( GUCEF_NULL )
    {
        TEventCallback triggeredCallback( this, &CScheduleTriggerListener::OnTriggered );
        SubscribeTo( &trigger, CORE::CScheduleTrigger::ScheduleTriggeredEvent, triggeredCallback );
        TEventCallback endedCallback( this, &CScheduleTriggerListener::OnEnded );
        SubscribeTo( &trigger, CORE::CScheduleTrigger::ScheduleEndedEvent, endedCallback );
    }

    private:

    typedef CORE::CTEventHandlerFunctor< CScheduleTriggerListener > TEventCallback;

    void OnTriggered( CORE::CNotifier* notifier   ,
                      const CORE::CEvent& eventId ,
                      CORE::CICloneable* data     )
    {
        ++triggeredCount;
        CORE::CScheduleTrigger::TScheduleTriggeredEventData* eventData =
            static_cast< CORE::CScheduleTrigger::TScheduleTriggeredEventData* >( data );
        lastScheduledTime = eventData->GetData().scheduledTime;
        lastFiredTime = eventData->GetData().firedTime;
        lastOccurrenceCount = eventData->GetData().occurrenceCount;
        if ( GUCEF_NULL != disableOnTrigger )
            disableOnTrigger->SetEnabled( false );
    }

    void OnEnded( CORE::CNotifier* notifier   ,
                  const CORE::CEvent& eventId ,
                  CORE::CICloneable* data     )
    {
        ++endedCount;
    }
};

/*-------------------------------------------------------------------------*/

/** Schedules the trigger for destruction from within the first event it receives, which a handler is allowed to do */
class CTriggerDeletingListener : public CORE::CObservingNotifier
{
    public:

    CORE::CScheduleTrigger* trigger;
    UInt32 eventCount;

    CTriggerDeletingListener( CORE::CScheduleTrigger* triggerToDelete ,
                              const CORE::CEvent& eventToDeleteOn     )
        : CORE::CObservingNotifier()
        , trigger( triggerToDelete )
        , eventCount( 0 )
    {
        TEventCallback callback( this, &CTriggerDeletingListener::OnEvent );
        SubscribeTo( triggerToDelete, eventToDeleteOn, callback );
    }

    private:

    typedef CORE::CTEventHandlerFunctor< CTriggerDeletingListener > TEventCallback;

    void OnEvent( CORE::CNotifier* notifier   ,
                  const CORE::CEvent& eventId ,
                  CORE::CICloneable* data     )
    {
        ++eventCount;
        trigger->ScheduleForDestruction();
        trigger = GUCEF_NULL;
    }
};

/*-------------------------------------------------------------------------*/

// all synthetic clock times are far enough in the future to be after the real time the trigger starts at
static CORE::CDateTime
FutureTime( UInt8 hour, UInt8 minute, UInt8 second )
{
    return CORE::CDateTime( 2040, 1, 1, hour, minute, second, 0, 0 );
}

/*-------------------------------------------------------------------------*/

void
PerformScheduleTriggerTests( void )
{
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING ScheduleTrigger TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "CScheduleTrigger" );

    GUCEF_TESTFW_TESTCASE( "Test: A trigger without a valid schedule cannot be enabled" )
        try
        {
            CORE::CScheduleTrigger trigger;
            ASSERT_FALSE( trigger.SetEnabled( true ) );
            ASSERT_FALSE( trigger.GetEnabled() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Enabling waits for the next occurrence after now and disabling stops it" )
        try
        {
            CORE::CScheduleTrigger trigger;
            trigger.SetSchedule( CORE::CSchedule( CORE::CCronSchedule( "* * * * *" ) ) );
            CScheduleTriggerListener listener( trigger );

            ASSERT_TRUE( trigger.SetEnabled( true ) );
            ASSERT_TRUE( trigger.GetEnabled() );
            CORE::CDateTime next;
            ASSERT_TRUE( trigger.GetNextOccurrence( next ) );
            Int64 untilNext = CORE::CDateTime::NowUTCDateTime().GetTimeDifferenceInMillisecondsTowards( next );
            ASSERT_TRUE( untilNext > -2000 && untilNext <= 61000 );
            ASSERT_TRUE( 0 == listener.triggeredCount );

            ASSERT_TRUE( trigger.SetEnabled( false ) );
            ASSERT_FALSE( trigger.GetNextOccurrence( next ) );

            // a disabled trigger ignores the clock
            trigger.Evaluate( FutureTime( 0, 30, 0 ) );
            ASSERT_TRUE( 0 == listener.triggeredCount );
            ASSERT_TRUE( 0 == trigger.GetOccurrenceCount() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Occurrences fire once each and missed occurrences are coalesced" )
        try
        {
            CORE::CScheduleTrigger trigger;
            trigger.SetSchedule( CORE::CSchedule( CORE::CCronSchedule( "* * * * *" ) ) );
            CScheduleTriggerListener listener( trigger );
            ASSERT_TRUE( trigger.SetEnabled( true ) );

            // the first occurrence after the real start time has long passed, it fires once
            trigger.Evaluate( FutureTime( 0, 0, 30 ) );
            ASSERT_TRUE( 1 == listener.triggeredCount );
            ASSERT_TRUE( 1 == listener.lastOccurrenceCount );
            ASSERT_TRUE( FutureTime( 0, 0, 30 ) == listener.lastFiredTime );

            // the next one is the minute after the clock time
            CORE::CDateTime next;
            ASSERT_TRUE( trigger.GetNextOccurrence( next ) );
            ASSERT_TRUE( FutureTime( 0, 1, 0 ) == next );

            // not due yet
            trigger.Evaluate( FutureTime( 0, 0, 59 ) );
            ASSERT_TRUE( 1 == listener.triggeredCount );

            // exactly due
            trigger.Evaluate( FutureTime( 0, 1, 0 ) );
            ASSERT_TRUE( 2 == listener.triggeredCount );
            ASSERT_TRUE( FutureTime( 0, 1, 0 ) == listener.lastScheduledTime );
            ASSERT_TRUE( 2 == listener.lastOccurrenceCount );

            // evaluating again at the same time must not fire again
            trigger.Evaluate( FutureTime( 0, 1, 0 ) );
            trigger.Evaluate( FutureTime( 0, 1, 30 ) );
            ASSERT_TRUE( 2 == listener.triggeredCount );

            // the clock jumped over several occurrences: one event, for the first missed one
            trigger.Evaluate( FutureTime( 0, 5, 30 ) );
            ASSERT_TRUE( 3 == listener.triggeredCount );
            ASSERT_TRUE( FutureTime( 0, 2, 0 ) == listener.lastScheduledTime );
            ASSERT_TRUE( FutureTime( 0, 5, 30 ) == listener.lastFiredTime );
            ASSERT_TRUE( trigger.GetNextOccurrence( next ) );
            ASSERT_TRUE( FutureTime( 0, 6, 0 ) == next );
            ASSERT_TRUE( 3 == trigger.GetOccurrenceCount() );
            ASSERT_TRUE( 0 == listener.endedCount );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A handler can disable the trigger from within the event" )
        try
        {
            CORE::CScheduleTrigger trigger;
            trigger.SetSchedule( CORE::CSchedule( CORE::CCronSchedule( "* * * * *" ) ) );
            CScheduleTriggerListener listener( trigger );
            listener.disableOnTrigger = &trigger;
            ASSERT_TRUE( trigger.SetEnabled( true ) );

            trigger.Evaluate( FutureTime( 1, 0, 30 ) );
            ASSERT_TRUE( 1 == listener.triggeredCount );
            ASSERT_FALSE( trigger.GetEnabled() );
            trigger.Evaluate( FutureTime( 1, 10, 0 ) );
            ASSERT_TRUE( 1 == listener.triggeredCount );
            ASSERT_TRUE( 0 == listener.endedCount );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The end of the schedule is reported once" )
        try
        {
            // only active within the first minutes of the year 2040
            CORE::CDateTimeRange range( FutureTime( 0, 0, 0 ), FutureTime( 0, 3, 30 ) );
            CORE::CScheduleTrigger trigger;
            trigger.SetSchedule( CORE::CSchedule( CORE::CCronSchedule( "* * * * *" ), range ) );
            CScheduleTriggerListener listener( trigger );
            ASSERT_TRUE( trigger.SetEnabled( true ) );

            // the first evaluation fires the pending occurrence, the one after the clock time
            // would be at 00:04:00 which is past the end of the window so the schedule ends
            trigger.Evaluate( FutureTime( 0, 3, 0 ) );
            ASSERT_TRUE( 1 == listener.triggeredCount );
            ASSERT_TRUE( 1 == listener.endedCount );

            trigger.Evaluate( FutureTime( 0, 10, 0 ) );
            trigger.Evaluate( FutureTime( 0, 20, 0 ) );
            ASSERT_TRUE( 1 == listener.triggeredCount );
            ASSERT_TRUE( 1 == listener.endedCount );
            CORE::CDateTime next;
            ASSERT_FALSE( trigger.GetNextOccurrence( next ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Replacing the schedule of an enabled trigger continues with the new schedule" )
        try
        {
            CORE::CScheduleTrigger trigger;
            trigger.SetSchedule( CORE::CSchedule( CORE::CCronSchedule( "* * * * *" ) ) );
            CScheduleTriggerListener listener( trigger );
            ASSERT_TRUE( trigger.SetEnabled( true ) );

            // an hourly schedule: the next occurrence is at the top of an hour
            trigger.SetSchedule( CORE::CSchedule( CORE::CCronSchedule( "0 * * * *" ) ) );
            CORE::CDateTime next;
            ASSERT_TRUE( trigger.GetNextOccurrence( next ) );
            ASSERT_TRUE( 0 == next.GetMinutes() );
            ASSERT_TRUE( 0 == listener.triggeredCount );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The poll interval has a floor of one millisecond" )
        try
        {
            CORE::CScheduleTrigger trigger;
            ASSERT_TRUE( 30000 == trigger.GetMaxPollIntervalInMilliSecs() );
            trigger.SetMaxPollIntervalInMilliSecs( 0 );
            ASSERT_TRUE( 1 == trigger.GetMaxPollIntervalInMilliSecs() );
            trigger.SetMaxPollIntervalInMilliSecs( 500 );
            ASSERT_TRUE( 500 == trigger.GetMaxPollIntervalInMilliSecs() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // The pulse generator requirement follows the interval the trigger wants, also while the timer is already enabled
    GUCEF_TESTFW_TESTCASE( "Test: The pulse request follows the interval when enabling, re-enabling, changing the poll interval and when the occurrence is near" )
        try
        {
            CORE::PulseGeneratorPtr pulseGenerator = CORE::CPulseGenerator::CreateSharedObj();
            CORE::CScheduleTrigger trigger( pulseGenerator );
            CScheduleTriggerListener listener( trigger );

            // the first occurrence is far away, so the interval is the poll interval
            CORE::CDateTimeRange range( FutureTime( 0, 0, 0 ), FutureTime( 0, 10, 0 ) );
            trigger.SetSchedule( CORE::CSchedule( CORE::CCronSchedule( "* * * * *" ), range ) );
            trigger.SetMaxPollIntervalInMilliSecs( 7777 );
            ASSERT_TRUE( trigger.SetEnabled( true ) );
            ASSERT_TRUE( 1 == pulseGenerator->GetNrOfPeriodicPulseRequestors() );
            ASSERT_TRUE( 7777 == pulseGenerator->GetRequiredPulseDeltaInMilliSecs() );

            // re-enabling with another poll interval
            ASSERT_TRUE( trigger.SetEnabled( false ) );
            ASSERT_TRUE( 0 == pulseGenerator->GetNrOfPeriodicPulseRequestors() );
            trigger.SetMaxPollIntervalInMilliSecs( 5000 );
            ASSERT_TRUE( trigger.SetEnabled( true ) );
            ASSERT_TRUE( 5000 == pulseGenerator->GetRequiredPulseDeltaInMilliSecs() );

            // lowering the poll interval of an enabled trigger
            trigger.SetMaxPollIntervalInMilliSecs( 3000 );
            ASSERT_TRUE( 3000 == pulseGenerator->GetRequiredPulseDeltaInMilliSecs() );

            // an occurrence that is near arms a shorter interval than the poll interval: 00:01:00 is 1000 ms after 00:00:59
            trigger.Evaluate( FutureTime( 0, 0, 0 ) );
            ASSERT_TRUE( 1 == listener.triggeredCount );
            trigger.SetMaxPollIntervalInMilliSecs( 30000 );
            ASSERT_TRUE( 30000 == pulseGenerator->GetRequiredPulseDeltaInMilliSecs() );
            trigger.Evaluate( FutureTime( 0, 0, 59 ) );
            ASSERT_TRUE( 1 == listener.triggeredCount );
            ASSERT_TRUE( 1000 == pulseGenerator->GetRequiredPulseDeltaInMilliSecs() );

            // and a later evaluation goes back to the poll interval
            trigger.Evaluate( FutureTime( 0, 1, 0 ) );
            ASSERT_TRUE( 2 == listener.triggeredCount );
            trigger.Evaluate( FutureTime( 0, 1, 1 ) );
            ASSERT_TRUE( 30000 == pulseGenerator->GetRequiredPulseDeltaInMilliSecs() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: After the end of the schedule nothing asks for pulses and the pulse requirement is not left at one millisecond" )
        try
        {
            CORE::PulseGeneratorPtr pulseGenerator = CORE::CPulseGenerator::CreateSharedObj();
            CORE::CScheduleTrigger trigger( pulseGenerator );
            CScheduleTriggerListener listener( trigger );
            trigger.SetMaxPollIntervalInMilliSecs( 7777 );

            // a window long past: no occurrence is left, the trigger evaluates right away to report the end
            CORE::CDateTimeRange range( CORE::CDateTime( 2000, 1, 1, 0, 0, 0, 0, 0 ), CORE::CDateTime( 2000, 1, 2, 0, 0, 0, 0, 0 ) );
            trigger.SetSchedule( CORE::CSchedule( CORE::CCronSchedule( "* * * * *" ), range ) );
            ASSERT_TRUE( trigger.SetEnabled( true ) );
            ASSERT_TRUE( 1 == pulseGenerator->GetRequiredPulseDeltaInMilliSecs() );
            trigger.Evaluate( CORE::CDateTime::NowUTCDateTime() );
            ASSERT_TRUE( 1 == listener.endedCount );
            ASSERT_TRUE( 0 == pulseGenerator->GetNrOfPeriodicPulseRequestors() );
            ASSERT_TRUE( 7777 == pulseGenerator->GetRequiredPulseDeltaInMilliSecs() );

            // the same after a regular end of the window and after disabling
            CORE::CDateTimeRange shortRange( FutureTime( 0, 0, 0 ), FutureTime( 0, 3, 30 ) );
            trigger.SetSchedule( CORE::CSchedule( CORE::CCronSchedule( "* * * * *" ), shortRange ) );
            ASSERT_TRUE( trigger.SetEnabled( false ) || true );
            ASSERT_TRUE( trigger.SetEnabled( true ) );
            ASSERT_TRUE( 1 == pulseGenerator->GetNrOfPeriodicPulseRequestors() );
            trigger.Evaluate( FutureTime( 0, 3, 0 ) );
            ASSERT_TRUE( 2 == listener.endedCount );
            ASSERT_TRUE( 0 == pulseGenerator->GetNrOfPeriodicPulseRequestors() );
            ASSERT_TRUE( 7777 == pulseGenerator->GetRequiredPulseDeltaInMilliSecs() );

            // restarting after the end arms the timer again, and the end of the previous run does not stay armed
            trigger.SetSchedule( CORE::CSchedule( CORE::CCronSchedule( "* * * * *" ), range ) );
            ASSERT_TRUE( trigger.SetEnabled( false ) || true );
            trigger.SetSchedule( CORE::CSchedule( CORE::CCronSchedule( "* * * * *" ), CORE::CDateTimeRange( FutureTime( 0, 0, 0 ), FutureTime( 0, 10, 0 ) ) ) );
            ASSERT_TRUE( trigger.SetEnabled( true ) );
            ASSERT_TRUE( 7777 == pulseGenerator->GetRequiredPulseDeltaInMilliSecs() );
            ASSERT_TRUE( trigger.SetEnabled( false ) );
            ASSERT_TRUE( 0 == pulseGenerator->GetNrOfPeriodicPulseRequestors() );
            ASSERT_TRUE( 7777 == pulseGenerator->GetRequiredPulseDeltaInMilliSecs() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // A handler may delete the trigger: nothing of the trigger may be touched after the notification that did it. Run under ASan.
    GUCEF_TESTFW_TESTCASE( "Test: A handler can destroy the trigger from within the triggered event and from within the ended event" )
        try
        {
            {
                CORE::CScheduleTrigger* trigger = new CORE::CScheduleTrigger( CORE::CPulseGenerator::CreateSharedObj() );
                trigger->SetSchedule( CORE::CSchedule( CORE::CCronSchedule( "* * * * *" ) ) );
                CTriggerDeletingListener listener( trigger, CORE::CScheduleTrigger::ScheduleTriggeredEvent );
                ASSERT_TRUE( trigger->SetEnabled( true ) );
                CORE::CDateTime next;
                ASSERT_TRUE( trigger->GetNextOccurrence( next ) );
                trigger->Evaluate( next );
                ASSERT_TRUE( 1 == listener.eventCount );
                ASSERT_TRUE( GUCEF_NULL == listener.trigger );
            }
            {
                CORE::CScheduleTrigger* trigger = new CORE::CScheduleTrigger( CORE::CPulseGenerator::CreateSharedObj() );
                trigger->SetSchedule( CORE::CSchedule( CORE::CCronSchedule( "* * * * *" ), CORE::CDateTimeRange( FutureTime( 0, 0, 0 ), FutureTime( 0, 3, 30 ) ) ) );
                CTriggerDeletingListener listener( trigger, CORE::CScheduleTrigger::ScheduleEndedEvent );
                ASSERT_TRUE( trigger->SetEnabled( true ) );
                trigger->Evaluate( FutureTime( 0, 3, 0 ) );
                ASSERT_TRUE( 1 == listener.eventCount );
                ASSERT_TRUE( GUCEF_NULL == listener.trigger );
            }
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    CORE::CLogStreamScope::FlushLogs();
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ALL ScheduleTrigger TESTS COMPLETED" );
}

/*-------------------------------------------------------------------------*/
