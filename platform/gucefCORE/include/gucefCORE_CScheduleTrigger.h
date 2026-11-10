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

#ifndef GUCEF_CORE_CSCHEDULETRIGGER_H
#define GUCEF_CORE_CSCHEDULETRIGGER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_COBSERVINGNOTIFIER_H
#include "CObservingNotifier.h"
#define GUCEF_CORE_COBSERVINGNOTIFIER_H
#endif /* GUCEF_CORE_COBSERVINGNOTIFIER_H ? */

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

#ifndef GUCEF_CORE_CTCLONEABLEOBJ_H
#include "CTCloneableObj.h"
#define GUCEF_CORE_CTCLONEABLEOBJ_H
#endif /* GUCEF_CORE_CTCLONEABLEOBJ_H ? */

#ifndef GUCEF_CORE_CPULSEGENERATOR_H
#include "gucefCORE_CPulseGenerator.h"
#define GUCEF_CORE_CPULSEGENERATOR_H
#endif /* GUCEF_CORE_CPULSEGENERATOR_H ? */

#ifndef GUCEF_CORE_CTIMER_H
#include "CTimer.h"
#define GUCEF_CORE_CTIMER_H
#endif /* GUCEF_CORE_CTIMER_H ? */

#ifndef GUCEF_CORE_CSCHEDULE_H
#include "gucefCORE_CSchedule.h"
#define GUCEF_CORE_CSCHEDULE_H
#endif /* GUCEF_CORE_CSCHEDULE_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Executes a CSchedule: fires ScheduleTriggeredEvent each time an occurrence of the
 *  schedule is reached, on the thread of the pulse generator the trigger is driven by.
 *
 *  All schedule times are UTC, as for CSchedule and CCronSchedule.
 *
 *  Occurrences that were missed, for example because the process was suspended or the
 *  system clock was moved forward, are coalesced: the trigger fires once for the first
 *  missed occurrence and continues with the first occurrence after the current time.
 *  Occurrences are never fired twice.
 *
 *  The trigger re-evaluates at least every MaxPollIntervalInMilliSecs so that changes of the
 *  system clock are noticed. In between it sleeps on a timer sized to the next occurrence.
 */
class GUCEF_CORE_PUBLIC_CPP CScheduleTrigger : public CObservingNotifier
{
    public:

    static const CEvent ScheduleTriggeredEvent;     /**< event data is TScheduleTriggeredEventData */
    static const CEvent ScheduleEndedEvent;         /**< no event data, fired once when the schedule has no further occurrences */

    struct SScheduleTriggeredData
    {
        CDateTime scheduledTime;        /**< the occurrence of the schedule that this event is for */
        CDateTime firedTime;            /**< the time at which the trigger noticed the occurrence */
        UInt64 occurrenceCount;         /**< number of times the trigger has fired, including this one */
    };
    typedef struct SScheduleTriggeredData TScheduleTriggeredData;
    typedef CTCloneableObj< TScheduleTriggeredData > TScheduleTriggeredEventData;

    static void RegisterEvents( void );

    public:

    CScheduleTrigger( void );

    CScheduleTrigger( PulseGeneratorPtr pulseGenerator );

    virtual ~CScheduleTrigger();

    /**
     *  Sets the schedule to execute. If the trigger is enabled it
     *  continues with the first occurrence of the new schedule after now.
     */
    void SetSchedule( const CSchedule& schedule );

    const CSchedule& GetSchedule( void ) const;

    /**
     *  Starts or stops the trigger. Starting begins at the first occurrence after now.
     *  Returns false if the trigger could not be started because the schedule is not valid.
     */
    bool SetEnabled( bool enabled );

    bool GetEnabled( void ) const;

    /**
     *  The longest time the trigger sleeps without re-evaluating the schedule against the clock.
     *  Defaults to 30 seconds.
     */
    void SetMaxPollIntervalInMilliSecs( UInt32 maxPollIntervalInMilliSecs );

    UInt32 GetMaxPollIntervalInMilliSecs( void ) const;

    UInt64 GetOccurrenceCount( void ) const;

    /**
     *  The next occurrence the trigger is waiting for, if any
     */
    bool GetNextOccurrence( CDateTime& next ) const;

    /**
     *  Evaluates the schedule against the given time, normally this is driven by the internal timer
     *  with the current UTC time. It is public so the schedule logic can be exercised with a given clock.
     */
    void Evaluate( const CDateTime& nowUtc );

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    typedef CTEventHandlerFunctor< CScheduleTrigger > TEventCallback;

    CScheduleTrigger( const CScheduleTrigger& src );
    CScheduleTrigger& operator=( const CScheduleTrigger& src );

    void OnTimerUpdate( CNotifier* notifier    ,
                        const CEvent& eventId  ,
                        CICloneable* eventData );

    void RegisterTimerEventHandlers( void );

    void FindNextOccurrence( const CDateTime& after );

    void ArmTimer( const CDateTime& nowUtc );

    void ReleaseTimer( void );

    private:

    CTimer m_timer;
    CSchedule m_schedule;
    CDateTime m_nextOccurrence;
    bool m_hasNextOccurrence;
    bool m_enabled;
    bool m_endedNotified;
    UInt32 m_maxPollIntervalInMilliSecs;
    UInt64 m_occurrenceCount;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CSCHEDULETRIGGER_H ? */
