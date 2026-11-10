/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "gucefCORE_CScheduleTrigger.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CEvent CScheduleTrigger::ScheduleTriggeredEvent = "GUCEF::CORE::CScheduleTrigger::ScheduleTriggeredEvent";
const CEvent CScheduleTrigger::ScheduleEndedEvent = "GUCEF::CORE::CScheduleTrigger::ScheduleEndedEvent";

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#define GUCEF_CORE_SCHEDULETRIGGER_DEFAULT_MAX_POLL_INTERVAL_IN_MS     30000

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CScheduleTrigger::CScheduleTrigger( void )
    : CObservingNotifier()
    , m_timer( 1000 )
    , m_schedule()
    , m_nextOccurrence()
    , m_hasNextOccurrence( false )
    , m_enabled( false )
    , m_endedNotified( false )
    , m_maxPollIntervalInMilliSecs( GUCEF_CORE_SCHEDULETRIGGER_DEFAULT_MAX_POLL_INTERVAL_IN_MS )
    , m_occurrenceCount( 0 )
{GUCEF_TRACE;

    RegisterEvents();
    RegisterTimerEventHandlers();
}

/*-------------------------------------------------------------------------*/

CScheduleTrigger::CScheduleTrigger( PulseGeneratorPtr pulseGenerator )
    : CObservingNotifier()
    , m_timer( pulseGenerator, 1000 )
    , m_schedule()
    , m_nextOccurrence()
    , m_hasNextOccurrence( false )
    , m_enabled( false )
    , m_endedNotified( false )
    , m_maxPollIntervalInMilliSecs( GUCEF_CORE_SCHEDULETRIGGER_DEFAULT_MAX_POLL_INTERVAL_IN_MS )
    , m_occurrenceCount( 0 )
{GUCEF_TRACE;

    RegisterEvents();
    RegisterTimerEventHandlers();
}

/*-------------------------------------------------------------------------*/

CScheduleTrigger::~CScheduleTrigger()
{GUCEF_TRACE;

    m_timer.SetEnabled( false );
}

/*-------------------------------------------------------------------------*/

void
CScheduleTrigger::RegisterEvents( void )
{GUCEF_TRACE;

    ScheduleTriggeredEvent.Initialize();
    ScheduleEndedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

void
CScheduleTrigger::RegisterTimerEventHandlers( void )
{GUCEF_TRACE;

    TEventCallback callback( this, &CScheduleTrigger::OnTimerUpdate );
    SubscribeTo( &m_timer                  ,
                 CTimer::TimerUpdateEvent  ,
                 callback                  );
}

/*-------------------------------------------------------------------------*/

const CString&
CScheduleTrigger::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static CString typeName = "GUCEF::CORE::CScheduleTrigger";
    return typeName;
}

/*-------------------------------------------------------------------------*/

void
CScheduleTrigger::SetSchedule( const CSchedule& schedule )
{GUCEF_TRACE;

    m_schedule = schedule;
    if ( m_enabled )
    {
        CDateTime now = CDateTime::NowUTCDateTime();
        m_endedNotified = false;
        FindNextOccurrence( now );
        ArmTimer( now );
        m_timer.SetEnabled( true );
    }
}

/*-------------------------------------------------------------------------*/

const CSchedule&
CScheduleTrigger::GetSchedule( void ) const
{GUCEF_TRACE;

    return m_schedule;
}

/*-------------------------------------------------------------------------*/

bool
CScheduleTrigger::SetEnabled( bool enabled )
{GUCEF_TRACE;

    if ( enabled == m_enabled )
        return true;

    if ( enabled )
    {
        if ( !m_schedule.IsValid() )
            return false;

        CDateTime now = CDateTime::NowUTCDateTime();
        m_enabled = true;
        m_endedNotified = false;
        FindNextOccurrence( now );
        ArmTimer( now );
        m_timer.SetEnabled( true );
    }
    else
    {
        m_enabled = false;
        m_hasNextOccurrence = false;
        ReleaseTimer();
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CScheduleTrigger::GetEnabled( void ) const
{GUCEF_TRACE;

    return m_enabled;
}

/*-------------------------------------------------------------------------*/

void
CScheduleTrigger::SetMaxPollIntervalInMilliSecs( UInt32 maxPollIntervalInMilliSecs )
{GUCEF_TRACE;

    m_maxPollIntervalInMilliSecs = maxPollIntervalInMilliSecs == 0 ? 1 : maxPollIntervalInMilliSecs;
    if ( m_enabled )
        ArmTimer( CDateTime::NowUTCDateTime() );
}

/*-------------------------------------------------------------------------*/

UInt32
CScheduleTrigger::GetMaxPollIntervalInMilliSecs( void ) const
{GUCEF_TRACE;

    return m_maxPollIntervalInMilliSecs;
}

/*-------------------------------------------------------------------------*/

UInt64
CScheduleTrigger::GetOccurrenceCount( void ) const
{GUCEF_TRACE;

    return m_occurrenceCount;
}

/*-------------------------------------------------------------------------*/

bool
CScheduleTrigger::GetNextOccurrence( CDateTime& next ) const
{GUCEF_TRACE;

    if ( m_enabled && m_hasNextOccurrence )
    {
        next = m_nextOccurrence;
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

void
CScheduleTrigger::FindNextOccurrence( const CDateTime& after )
{GUCEF_TRACE;

    m_hasNextOccurrence = m_schedule.GetNextOccurrence( after, m_nextOccurrence );
}

/*-------------------------------------------------------------------------*/

void
CScheduleTrigger::ReleaseTimer( void )
{GUCEF_TRACE;

    // The pulse generator keeps the last requirement it was given when nothing asks for pulses anymore:
    // do not leave that at the one millisecond of an evaluation that was due right away
    m_timer.SetInterval( m_maxPollIntervalInMilliSecs );
    m_timer.SetEnabled( false );
}

/*-------------------------------------------------------------------------*/

void
CScheduleTrigger::ArmTimer( const CDateTime& nowUtc )
{GUCEF_TRACE;

    UInt32 interval = m_maxPollIntervalInMilliSecs;
    if ( m_hasNextOccurrence )
    {
        Int64 remaining = nowUtc.GetTimeDifferenceInMillisecondsTowards( m_nextOccurrence );
        if ( remaining < 1 )
            interval = 1;
        else
        if ( remaining < (Int64) interval )
            interval = (UInt32) remaining;
    }
    else
    {
        // nothing left to wait for, evaluate right away so the end gets reported
        interval = 1;
    }
    m_timer.SetInterval( interval );
}

/*-------------------------------------------------------------------------*/

void
CScheduleTrigger::Evaluate( const CDateTime& nowUtc )
{GUCEF_TRACE;

    if ( !m_enabled )
        return;

    if ( m_hasNextOccurrence && nowUtc >= m_nextOccurrence )
    {
        TScheduleTriggeredEventData eventData;
        eventData.GetData().scheduledTime = m_nextOccurrence;
        eventData.GetData().firedTime = nowUtc;
        eventData.GetData().occurrenceCount = ++m_occurrenceCount;

        // continue after now, any occurrences missed in the meantime are coalesced into this one
        FindNextOccurrence( nowUtc );

        // handlers are allowed to change the schedule or to disable the trigger
        if ( !NotifyObservers( ScheduleTriggeredEvent, &eventData ) )
            return;                     // a handler deleted this object
        if ( !m_enabled )
            return;
    }

    if ( !m_hasNextOccurrence )
    {
        if ( !m_endedNotified )
        {
            m_endedNotified = true;
            ReleaseTimer();
            NotifyObservers( ScheduleEndedEvent );      // nothing of this object is used after this
        }
        return;
    }

    ArmTimer( nowUtc );
}

/*-------------------------------------------------------------------------*/

void
CScheduleTrigger::OnTimerUpdate( CNotifier* notifier    ,
                                 const CEvent& eventId  ,
                                 CICloneable* eventData )
{GUCEF_TRACE;

    Evaluate( CDateTime::NowUTCDateTime() );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
