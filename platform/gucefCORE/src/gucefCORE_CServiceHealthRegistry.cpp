/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
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

#include <algorithm>

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_CDATETIME_H
#include "gucefCORE_CDateTime.h"
#define GUCEF_CORE_CDATETIME_H
#endif /* GUCEF_CORE_CDATETIME_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_GUCEF_ESSENTIALS_H
#include "gucef_essentials.h"
#define GUCEF_CORE_GUCEF_ESSENTIALS_H
#endif /* GUCEF_CORE_GUCEF_ESSENTIALS_H ? */

#include "gucefCORE_CServiceHealthRegistry.h"

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

const UInt32 CServiceHealthRegistry::DefaultCheckIntervalInMs = 1000;
const UInt32 CServiceHealthRegistry::DefaultStalenessInMs = 10000;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static UInt64
ElapsedMsSince( UInt64 ticks )
{GUCEF_TRACE;

    const Float64 ticksPerMs = MT::PrecisionTimerResolution() / 1000.0;
    const UInt64 now = MT::PrecisionTickCount();
    if ( now <= ticks || ticksPerMs <= 0.0 )
        return 0;
    return (UInt64) ( ( now - ticks ) / ticksPerMs );
}

/*-------------------------------------------------------------------------*/

static UInt64
NowUtcMs( void )
{GUCEF_TRACE;

    return CDateTime::NowUTCDateTime().ToUnixEpochBasedTicksInMillisecs();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CServiceHealthRegistry::CServiceHealthRegistry( void )
    : CObserver()
    , m_dataLock()
    , m_evaluationLock()
    , m_pulseGenerator()
    , m_listening( false )
    , m_checks()
    , m_outcomes()
    , m_started( false )
    , m_info()
    , m_checkIntervalInMs( DefaultCheckIntervalInMs )
    , m_stalenessInMs( DefaultStalenessInMs )
    , m_evaluated( false )
    , m_lastEvaluationTicks( 0 )
    , m_lastEvaluationUtcMs( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CServiceHealthRegistry::CServiceHealthRegistry( PulseGeneratorPtr pulseGenerator )
    : CObserver()
    , m_dataLock()
    , m_evaluationLock()
    , m_pulseGenerator( pulseGenerator )
    , m_listening( false )
    , m_checks()
    , m_outcomes()
    , m_started( false )
    , m_info()
    , m_checkIntervalInMs( DefaultCheckIntervalInMs )
    , m_stalenessInMs( DefaultStalenessInMs )
    , m_evaluated( false )
    , m_lastEvaluationTicks( 0 )
    , m_lastEvaluationUtcMs( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CServiceHealthRegistry::~CServiceHealthRegistry()
{GUCEF_TRACE;

    UnsubscribeAllFromObserver();
}

/*-------------------------------------------------------------------------*/

void
CServiceHealthRegistry::ListenToPulses( void )
{GUCEF_TRACE;

    // Never under the data lock: a pulse handler of the application may call the registry while the pulse
    // generator holds its notification lock
    PulseGeneratorPtr pulseGenerator;
    {
        MT::CScopeMutex lock( m_dataLock );
        if ( m_listening || m_pulseGenerator.IsNULL() )
            return;
        m_listening = true;
        pulseGenerator = m_pulseGenerator;
    }
    TEventCallback callback( this, &CServiceHealthRegistry::OnPulse );
    SubscribeTo( pulseGenerator.GetPointerAlways(), CPulseGenerator::PulseEvent, callback );
}

/*-------------------------------------------------------------------------*/

void
CServiceHealthRegistry::SetPulseGenerator( PulseGeneratorPtr pulseGenerator )
{GUCEF_TRACE;

    PulseGeneratorPtr previous;
    bool wasListening = false;
    {
        MT::CScopeMutex lock( m_dataLock );
        if ( m_pulseGenerator == pulseGenerator )
            return;
        previous = m_pulseGenerator;
        wasListening = m_listening;
        m_listening = false;
        m_pulseGenerator = pulseGenerator;
    }

    // Not under the data lock: the thread of the previous pulse generator may be in OnPulse(), waiting for it
    if ( wasListening && !previous.IsNULL() )
        UnsubscribeFrom( *previous.GetPointerAlways() );
    if ( wasListening )
        ListenToPulses();
}

/*-------------------------------------------------------------------------*/

PulseGeneratorPtr
CServiceHealthRegistry::GetPulseGenerator( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_dataLock );
    return m_pulseGenerator;
}

/*-------------------------------------------------------------------------*/

bool
CServiceHealthRegistry::RegisterReadinessCheck( CIServiceHealthCheck* check )
{GUCEF_TRACE;

    if ( GUCEF_NULL == check )
        return false;

    MT::CScopeMutex lock( m_dataLock );
    for ( size_t i=0; i<m_checks.size(); ++i )
        if ( m_checks[ i ] == check )
            return false;

    // a new check counts as failing until it was evaluated
    SCheckOutcome outcome;
    outcome.name = check->GetName();
    outcome.state = HEALTH_FAIL;
    outcome.detail = "not evaluated yet";
    outcome.sinceUtcMs = NowUtcMs();
    m_checks.push_back( check );
    m_outcomes.push_back( outcome );
    lock.EarlyUnlock();
    ListenToPulses();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CServiceHealthRegistry::UnregisterReadinessCheck( CIServiceHealthCheck* check )
{GUCEF_TRACE;

    // an evaluation in progress on the pulse thread may be calling the check: wait for it
    MT::CScopeMutex evaluationLock( m_evaluationLock );
    MT::CScopeMutex lock( m_dataLock );
    for ( size_t i=0; i<m_checks.size(); ++i )
    {
        if ( m_checks[ i ] == check )
        {
            m_checks.erase( m_checks.begin() + i );
            m_outcomes.erase( m_outcomes.begin() + i );
            return true;
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

UInt32
CServiceHealthRegistry::GetReadinessCheckCount( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_dataLock );
    return (UInt32) m_checks.size();
}

/*-------------------------------------------------------------------------*/

void
CServiceHealthRegistry::SetStarted( bool started )
{GUCEF_TRACE;

    {
        MT::CScopeMutex lock( m_dataLock );
        m_started = started;
    }
    ListenToPulses();
}

/*-------------------------------------------------------------------------*/

bool
CServiceHealthRegistry::IsStarted( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_dataLock );
    return m_started;
}

/*-------------------------------------------------------------------------*/

void
CServiceHealthRegistry::SetInfo( const CValueList& info )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_dataLock );
    m_info = info;
}

/*-------------------------------------------------------------------------*/

CValueList
CServiceHealthRegistry::GetInfo( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_dataLock );
    return m_info;
}

/*-------------------------------------------------------------------------*/

void
CServiceHealthRegistry::SetCheckIntervalInMs( UInt32 checkIntervalInMs )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_dataLock );
    m_checkIntervalInMs = checkIntervalInMs;
}

/*-------------------------------------------------------------------------*/

UInt32
CServiceHealthRegistry::GetCheckIntervalInMs( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_dataLock );
    return m_checkIntervalInMs;
}

/*-------------------------------------------------------------------------*/

void
CServiceHealthRegistry::SetStalenessInMs( UInt32 stalenessInMs )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_dataLock );
    m_stalenessInMs = stalenessInMs;
}

/*-------------------------------------------------------------------------*/

UInt32
CServiceHealthRegistry::GetStalenessInMs( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_dataLock );
    return m_stalenessInMs;
}

/*-------------------------------------------------------------------------*/

void
CServiceHealthRegistry::EvaluateChecks( void )
{GUCEF_TRACE;

    ListenToPulses();
    MT::CScopeMutex evaluationLock( m_evaluationLock );

    // the checks run without the data lock, so the probes keep reading the cache meanwhile
    TCheckVector checks;
    {
        MT::CScopeMutex lock( m_dataLock );
        checks = m_checks;
    }

    GUCEF::vector< TServiceHealthState > states( checks.size(), HEALTH_FAIL );
    CString::StringVector details( checks.size() );
    GUCEF::vector< bool > evaluated( checks.size(), false );
    for ( size_t i=0; i<checks.size(); ++i )
    {
        // Other threads wait for the evaluation lock to change the list, but a check may unregister checks from within
        // its Check() on this thread (the locks are recursive). A check that is no longer registered may be gone: it is
        // not called, and every outcome is matched to its check by pointer, not by its place in the list
        {
            MT::CScopeMutex lock( m_dataLock );
            if ( m_checks.end() == std::find( m_checks.begin(), m_checks.end(), checks[ i ] ) )
                continue;
        }
        evaluated[ i ] = true;
        try
        {
            states[ i ] = checks[ i ]->Check( details[ i ] );
            if ( HEALTH_PASS != states[ i ] && HEALTH_WARN != states[ i ] )
                states[ i ] = HEALTH_FAIL;
        }
        catch ( ... )
        {
            states[ i ] = HEALTH_FAIL;
            details[ i ] = "the check threw an exception";
        }
    }

    const UInt64 nowUtcMs = NowUtcMs();
    CString::StringVector changes;
    MT::CScopeMutex lock( m_dataLock );
    for ( size_t i=0; i<checks.size(); ++i )
    {
        if ( !evaluated[ i ] )
            continue;
        TCheckVector::iterator registered = std::find( m_checks.begin(), m_checks.end(), checks[ i ] );
        if ( m_checks.end() == registered )
            continue;   // unregistered by a check that ran after it
        SCheckOutcome& outcome = m_outcomes[ registered - m_checks.begin() ];
        if ( outcome.state != states[ i ] || 0 == outcome.sinceUtcMs )
        {
            if ( outcome.state != states[ i ] && HEALTH_PASS != states[ i ] )
            {
                // logged once the data lock is released: the log manager's locks are not taken under it
                changes.push_back( "ServiceHealthRegistry: Check \"" + outcome.name + "\" went from " +
                    ServiceHealthStateToString( outcome.state ) + " to " + ServiceHealthStateToString( states[ i ] ) );
            }
            outcome.state = states[ i ];
            outcome.sinceUtcMs = nowUtcMs;
        }
        outcome.detail = details[ i ];
    }
    m_evaluated = true;
    m_lastEvaluationTicks = MT::PrecisionTickCount();
    m_lastEvaluationUtcMs = nowUtcMs;
    lock.EarlyUnlock();

    for ( size_t i=0; i<changes.size(); ++i )
        GUCEF_WARNING_LOG( LOGLEVEL_NORMAL, changes[ i ] );
}

/*-------------------------------------------------------------------------*/

void
CServiceHealthRegistry::OnPulse( CNotifier* notifier    ,
                                 const CEvent& eventId  ,
                                 CICloneable* eventData )
{GUCEF_TRACE;

    bool due = false;
    {
        MT::CScopeMutex lock( m_dataLock );
        due = !m_evaluated || ElapsedMsSince( m_lastEvaluationTicks ) >= m_checkIntervalInMs;
    }
    if ( due )
        EvaluateChecks();
}

/*-------------------------------------------------------------------------*/

bool
CServiceHealthRegistry::IsStaleWhileLocked( void ) const
{GUCEF_TRACE;

    if ( !m_evaluated )
        return true;
    if ( 0 == m_stalenessInMs )
        return false;
    return ElapsedMsSince( m_lastEvaluationTicks ) > m_stalenessInMs;
}

/*-------------------------------------------------------------------------*/

void
CServiceHealthRegistry::FillCommonWhileLocked( SHealthReport& report ) const
{GUCEF_TRACE;

    report.checks.clear();
    report.isStale = IsStaleWhileLocked();
    report.isStarted = m_started;
    report.evaluatedAtUtcMs = m_evaluated ? m_lastEvaluationUtcMs : 0;
}

/*-------------------------------------------------------------------------*/

void
CServiceHealthRegistry::GetLiveness( SHealthReport& report ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_dataLock );
    FillCommonWhileLocked( report );
    report.state = report.isStale ? HEALTH_FAIL : HEALTH_PASS;
}

/*-------------------------------------------------------------------------*/

void
CServiceHealthRegistry::GetReadiness( SHealthReport& report ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_dataLock );
    FillCommonWhileLocked( report );
    report.checks = m_outcomes;

    TServiceHealthState worst = HEALTH_PASS;
    for ( size_t i=0; i<m_outcomes.size(); ++i )
        if ( m_outcomes[ i ].state > worst )
            worst = m_outcomes[ i ].state;
    if ( report.isStale || !m_started )
        worst = HEALTH_FAIL;
    report.state = worst;
}

/*-------------------------------------------------------------------------*/

void
CServiceHealthRegistry::GetStartup( SHealthReport& report ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_dataLock );
    FillCommonWhileLocked( report );
    report.state = m_started ? HEALTH_PASS : HEALTH_FAIL;
}

/*-------------------------------------------------------------------------*/

const CString&
CServiceHealthRegistry::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString classTypeName = "GUCEF::CORE::CServiceHealthRegistry";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
