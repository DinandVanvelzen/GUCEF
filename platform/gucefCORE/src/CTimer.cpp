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

#include <assert.h>

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"  /* needed for the precision timer */
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_CPULSEGENERATOR_H
#include "gucefCORE_CPulseGenerator.h"
#define GUCEF_CORE_CPULSEGENERATOR_H
#endif /* GUCEF_CORE_CPULSEGENERATOR_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CGUCEFAPPLICATION_H
#include "CGUCEFApplication.h"
#define GUCEF_CORE_CGUCEFAPPLICATION_H
#endif /* GUCEF_CORE_CGUCEFAPPLICATION_H ? */

#include "CTimer.h"

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

const CEvent CTimer::TimerStartedEvent = "GUCEF::CORE::CTimer::TimerStartedEvent";
const CEvent CTimer::TimerUpdateEvent = "GUCEF::CORE::CTimer::TimerUpdateEvent";
const CEvent CTimer::TimerStoppedEvent = "GUCEF::CORE::CTimer::TimerStoppedEvent";
const CEvent CTimer::TimerIntervalChangedEvent = "GUCEF::CORE::CTimer::TimerIntervalChangedEvent";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CTimer::CTimer( const UInt32 updateDeltaInMilliSecs /* = 25 */ )
    : CObservingNotifier()
    , m_timerFreq( MT::PrecisionTimerResolution() / 1000.0 )
    , m_lastTimerCycle( 0 )
    , m_tickCount( 0 )
    , m_updateDeltaInMilliSecs( updateDeltaInMilliSecs )
    , m_activationTickCount( 0 )
    , m_enabled( false )
    , m_immediateTriggerRequested( false )
    , m_pulseGenerator( CCoreGlobal::Instance()->GetPulseGenerator() )
    , m_opaqueUserData( GUCEF_NULL )
{GUCEF_TRACE;

    RegisterEvents();

    if ( 0 == m_updateDeltaInMilliSecs )
    {
        m_updateDeltaInMilliSecs = 1;
    }

    RegisterPulseGeneratorEventHandlers();
}

/*-------------------------------------------------------------------------*/

CTimer::CTimer( PulseGeneratorPtr pulseGenerator               ,
                const UInt32 updateDeltaInMilliSecs /* = 25 */ )
    : CObservingNotifier()
    , m_timerFreq( MT::PrecisionTimerResolution() / 1000.0 )
    , m_lastTimerCycle( 0 )
    , m_tickCount( 0 )
    , m_updateDeltaInMilliSecs( updateDeltaInMilliSecs )
    , m_activationTickCount( 0 )
    , m_enabled( false )
    , m_immediateTriggerRequested( false )
    , m_pulseGenerator( pulseGenerator )
    , m_opaqueUserData( GUCEF_NULL )
{GUCEF_TRACE;

    RegisterEvents();

    if ( 0 == m_updateDeltaInMilliSecs )
    {
        m_updateDeltaInMilliSecs = 1;
    }

    RegisterPulseGeneratorEventHandlers();
}

/*-------------------------------------------------------------------------*/

CTimer::CTimer( const CTimer& src )
    : CObservingNotifier( src )
    , m_timerFreq( MT::PrecisionTimerResolution() / 1000.0 )
    , m_lastTimerCycle( src.m_lastTimerCycle )
    , m_tickCount( 0 )
    , m_updateDeltaInMilliSecs( src.m_updateDeltaInMilliSecs )
    , m_activationTickCount( 0 )
    , m_enabled( false )
    , m_immediateTriggerRequested( false )
    , m_pulseGenerator( src.m_pulseGenerator )
    , m_opaqueUserData( GUCEF_NULL )
{GUCEF_TRACE;

    RegisterEvents();

    if ( 0 == m_updateDeltaInMilliSecs )
    {
        m_updateDeltaInMilliSecs = 1;
    }

    RegisterPulseGeneratorEventHandlers();
}

/*-------------------------------------------------------------------------*/

CTimer::~CTimer()
{GUCEF_TRACE;

    SetEnabled( false );
    SignalUpcomingObserverDestruction();

    if ( !m_pulseGenerator.IsNULL() )
    {
        // Make sure this object is not still registered as requiring periodic updates
        m_pulseGenerator->RequestStopOfPeriodicUpdates( this );
        m_pulseGenerator.Unlink();
    }
}

/*-------------------------------------------------------------------------*/

CTimer&
CTimer::operator=( const CTimer& src )
{GUCEF_TRACE;

    if ( this != &src )
    {
        m_updateDeltaInMilliSecs = src.m_updateDeltaInMilliSecs;
        m_lastTimerCycle = src.m_lastTimerCycle;
        m_enabled = src.m_enabled;
        m_tickCount = src.m_tickCount;
        m_opaqueUserData = GUCEF_NULL;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

void
CTimer::RegisterEvents( void )
{GUCEF_TRACE;

    TimerStartedEvent.Initialize();
    TimerUpdateEvent.Initialize();
    TimerStoppedEvent.Initialize();
    TimerIntervalChangedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

void
CTimer::RegisterPulseGeneratorEventHandlers( void )
{GUCEF_TRACE;

    if ( !m_pulseGenerator.IsNULL() )
    {
        TEventCallback callback( this, &CTimer::OnPulse );
        SubscribeTo( m_pulseGenerator.GetPointerAlways() ,
                     CPulseGenerator::PulseEvent         ,
                     callback                            );
    }
}

/*-------------------------------------------------------------------------*/

void
CTimer::SetInterval( const UInt32 updateDeltaInMilliSecs )
{GUCEF_TRACE;

    const bool changed = m_updateDeltaInMilliSecs != updateDeltaInMilliSecs;
    m_updateDeltaInMilliSecs = updateDeltaInMilliSecs;

    // An enabled timer has told the pulse generator what pulse rate it requires, that has to follow the new interval
    if ( changed && m_enabled && !m_pulseGenerator.IsNULL() )
    {
        m_pulseGenerator->RequestPeriodicPulses( this, 0 == m_updateDeltaInMilliSecs ? 1 : m_updateDeltaInMilliSecs );
    }

    TimerIntervalChangedEventData eData( updateDeltaInMilliSecs );
    NotifyObservers( TimerIntervalChangedEvent, &eData );
}

/*-------------------------------------------------------------------------*/

UInt32
CTimer::GetInterval( void ) const
{GUCEF_TRACE;

    return m_updateDeltaInMilliSecs;
}

/*-------------------------------------------------------------------------*/

bool
CTimer::SetEnabled( const bool enabled )
{GUCEF_TRACE;

    if ( m_pulseGenerator.IsNULL() )
    {
        // presumably a pulse generator will be set later via SetPulseGenerator()
        // we still store the enabled state so that we can become active as soon as the pulse
        // generator is hooked up
        m_enabled = enabled;
        return false;
    }

    if ( m_enabled != enabled )
    {
        m_enabled = enabled;

        if ( enabled )
        {
            m_lastTimerCycle = m_activationTickCount = MT::PrecisionTickCount();

            m_timerFreq = MT::PrecisionTimerResolution() / 1000.0;
            assert( m_timerFreq != 0 );
            m_tickCount = 0;

            // Make sure we get an update and tell the pump about our update requirement
            m_pulseGenerator->RequestPeriodicPulses( this, m_updateDeltaInMilliSecs );
            NotifyObservers( TimerStartedEvent );

        }
        else
        {
            // we don't need periodic updates anymore
            m_pulseGenerator->RequestStopOfPeriodicUpdates( this );

            NotifyObservers( TimerStoppedEvent );
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CTimer::GetEnabled( void ) const
{GUCEF_TRACE;

    return m_enabled;
}

/*-------------------------------------------------------------------------*/

void
CTimer::SetOpaqueUserData( void* opaqueUserData )
{GUCEF_TRACE;

    m_opaqueUserData = opaqueUserData;
}

/*-------------------------------------------------------------------------*/

void*
CTimer::GetOpaqueUserData( void ) const
{GUCEF_TRACE;

    return m_opaqueUserData;
}

/*-------------------------------------------------------------------------*/

void
CTimer::OnPulse( CNotifier* notifier                 ,
                 const CEvent& eventid               ,
                 CICloneable* eventdata /* = NULL */ )
{GUCEF_TRACE;

    if ( m_enabled )
    {
        UInt64 newTickCount = MT::PrecisionTickCount();
        UInt64 deltaTicks = ( newTickCount - m_lastTimerCycle );
		Float64 deltaMilliSecs = deltaTicks / m_timerFreq;
        m_tickCount += deltaTicks;

        if ( m_immediateTriggerRequested || deltaMilliSecs >= m_updateDeltaInMilliSecs )
        {
            m_immediateTriggerRequested = false;
            m_lastTimerCycle = newTickCount;

            TimerUpdateEventData cuData;
            TTimerUpdateData& updateData = cuData.GetData();
            updateData.tickCount = m_tickCount;
            updateData.updateDeltaInMilliSecs = deltaMilliSecs;
            updateData.opaqueUserData = m_opaqueUserData;
            NotifyObservers( TimerUpdateEvent, &cuData );
        }
    }
}

/*-------------------------------------------------------------------------*/

UInt64
CTimer::GetRunTicks( void ) const
{GUCEF_TRACE;

    if ( m_enabled )
    {
        return MT::PrecisionTickCount() - m_activationTickCount;
    }
    else
    {
        return m_tickCount;
    }
}

/*-------------------------------------------------------------------------*/

Float64
CTimer::GetRunTimeInMilliSecs( void ) const
{GUCEF_TRACE;

    return GetRunTicks() / m_timerFreq;
}

/*-------------------------------------------------------------------------*/

UInt64
CTimer::GetTickCount( void ) const
{GUCEF_TRACE;

    return m_tickCount;
}

/*-------------------------------------------------------------------------*/

Float64
CTimer::GetTimeInMilliSecs( void ) const
{GUCEF_TRACE;

    return m_tickCount / m_timerFreq;
}

/*-------------------------------------------------------------------------*/

void
CTimer::Reset( void )
{GUCEF_TRACE;

    m_lastTimerCycle = MT::PrecisionTickCount();
    m_tickCount = 0;
}

/*-------------------------------------------------------------------------*/

 void
 CTimer::TriggerNow( void )
 {GUCEF_TRACE;

    UInt64 newTickCount = MT::PrecisionTickCount();
    UInt64 deltaTicks = ( newTickCount - m_lastTimerCycle );
	Float64 deltaMilliSecs = deltaTicks / m_timerFreq;

    TimerUpdateEventData cuData;
    TTimerUpdateData& updateData = cuData.GetData();
    updateData.tickCount = m_tickCount;
    updateData.updateDeltaInMilliSecs = deltaMilliSecs;
    if ( !NotifyObservers( TimerUpdateEvent, &cuData ) ) return;

    Reset();
 }

 /*-------------------------------------------------------------------------*/

 void
 CTimer::RequestImmediateTrigger( void )
 {GUCEF_TRACE;

    if (  !m_pulseGenerator.IsNULL() )
    {
        m_immediateTriggerRequested = true;
        m_pulseGenerator->RequestImmediatePulse();
    }
 }

/*-------------------------------------------------------------------------*/

Float64
CTimer::GetApproxMaxTimerResolutionInMilliSecs( void )
{GUCEF_TRACE;

    return MT::PrecisionTimerResolution() / 1000.0;
}

/*-------------------------------------------------------------------------*/

const CString&
CTimer::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static CString typeName = "GUCEF::CORE::CTimer";
    return typeName;
}

/*-------------------------------------------------------------------------*/

MT::TLockStatus
CTimer::Lock( UInt32 lockWaitTimeoutInMs ) const
{GUCEF_TRACE;

    return MT::LOCKSTATUS_NOT_APPLICABLE;
}

/*-------------------------------------------------------------------------*/

MT::TLockStatus
CTimer::Unlock( void ) const
{GUCEF_TRACE;

    return MT::LOCKSTATUS_NOT_APPLICABLE;
}

/*-------------------------------------------------------------------------*/

void
CTimer::SetPulseGenerator( PulseGeneratorPtr newPulseGenerator )
{GUCEF_TRACE;

    if ( m_pulseGenerator != newPulseGenerator )
    {
        bool wasEnabled = m_enabled;
        UnsubscribeFrom( m_pulseGenerator.GetPointerAlways() );

        SetEnabled( false );
        Reset();

        m_pulseGenerator = newPulseGenerator;
        RegisterPulseGeneratorEventHandlers();

        if ( wasEnabled )
            SetEnabled( wasEnabled );
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*--------------------------------------------------------------------------*/
