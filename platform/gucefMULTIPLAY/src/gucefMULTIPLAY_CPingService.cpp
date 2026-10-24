/*
 *  gucefMULTIPLAY: GUCEF module providing multiplayer RAD functionality
 *
 *  Copyright (C) 1998 - 2020.  Dinand Vanvelzen
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

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_COMCORE_CCOMCOREGLOBAL_H
#include "gucefCOMCORE_CComCoreGlobal.h"
#define GUCEF_COMCORE_CCOMCOREGLOBAL_H
#endif /* GUCEF_COMCORE_CCOMCOREGLOBAL_H ? */

#ifndef GUCEF_MULTIPLAY_DATANODEUTILS_H
#include "gucefMULTIPLAY_DataNodeUtils.h"
#define GUCEF_MULTIPLAY_DATANODEUTILS_H
#endif /* GUCEF_MULTIPLAY_DATANODEUTILS_H ? */

#ifndef GUCEF_MULTIPLAY_CGAMELISTINGENTRY_H
#include "gucefMULTIPLAY_CGameListingEntry.h"
#define GUCEF_MULTIPLAY_CGAMELISTINGENTRY_H
#endif /* GUCEF_MULTIPLAY_CGAMELISTINGENTRY_H ? */

#include "gucefMULTIPLAY_CPingService.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MULTIPLAY {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CEvent CPingService::PingUpdatedEvent = "GUCEF::MULTIPLAY::CPingService::PingUpdatedEvent";

#define GUCEF_MULTIPLAY_PINGSERVICE_TICK_MS     50

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CPingUpdate::CPingUpdate( void )
    : CORE::CICloneable()
    , target()
    , rttMs( -1 )
    , method()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CPingUpdate::CPingUpdate( const CORE::CString& pingTarget, Int32 pingRttMs, const CORE::CString& pingMethod )
    : CORE::CICloneable()
    , target( pingTarget )
    , rttMs( pingRttMs )
    , method( pingMethod )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CPingUpdate::~CPingUpdate()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CPingUpdate::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CPingUpdate( *this );
}

/*-------------------------------------------------------------------------*/

void
CPingService::RegisterEvents( void )
{GUCEF_TRACE;

    PingUpdatedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CPingService::CPingService( void )
    : CORE::CObservingNotifier()
    , CORE::CIConfigurable()
    , m_pulseGenerator( CORE::CCoreGlobal::Instance()->GetPulseGenerator() )
    , m_timer( m_pulseGenerator, GUCEF_MULTIPLAY_PINGSERVICE_TICK_MS )
    , m_targets()
    , m_retiredProbes()
    , m_methods()
    , m_intervalInMs( 2000 )
    , m_timeoutInMs( 1000 )
    , m_smoothing( 0.25f )
    , m_maxFailuresPerMethod( 3 )
    , m_retryAfterInMs( 30000 )
{GUCEF_TRACE;

    Initialize();
}

/*-------------------------------------------------------------------------*/

CPingService::CPingService( const CORE::PulseGeneratorPtr& pulseGenerator )
    : CORE::CObservingNotifier()
    , CORE::CIConfigurable()
    , m_pulseGenerator( pulseGenerator )
    , m_timer( m_pulseGenerator, GUCEF_MULTIPLAY_PINGSERVICE_TICK_MS )
    , m_targets()
    , m_retiredProbes()
    , m_methods()
    , m_intervalInMs( 2000 )
    , m_timeoutInMs( 1000 )
    , m_smoothing( 0.25f )
    , m_maxFailuresPerMethod( 3 )
    , m_retryAfterInMs( 30000 )
{GUCEF_TRACE;

    Initialize();
}

/*-------------------------------------------------------------------------*/

void
CPingService::Initialize( void )
{GUCEF_TRACE;

    RegisterEvents();
    m_methods.push_back( "udp-echo" );
    m_methods.push_back( "tcp-connect" );
    m_methods.push_back( "icmp" );
    SubscribeTo( &m_timer );
}

/*-------------------------------------------------------------------------*/

CPingService::~CPingService()
{GUCEF_TRACE;

    m_timer.SetEnabled( false );
    UnsubscribeAllFromObserver();
    TTargetMap::iterator i = m_targets.begin();
    while ( i != m_targets.end() )
    {
        if ( !(*i).second.probe.IsNULL() )
            (*i).second.probe->Stop();
        ++i;
    }
    m_targets.clear();
    m_retiredProbes.clear();
}

/*-------------------------------------------------------------------------*/

bool
CPingService::ParseTarget( const CORE::CString& target, COMCORE::CHostAddress& address )
{GUCEF_TRACE;

    CORE::CString scheme, host, path;
    UInt16 port = 0;
    if ( 0 <= target.HasSubstr( "://" ) )
    {
        if ( !CGameListingEntry::ParseEndpoint( target, scheme, host, port, path ) )
            return false;
        return address.SetHostnameAndPort( host, port );
    }
    return address.SetHostnameAndPort( target );
}

/*-------------------------------------------------------------------------*/

bool
CPingService::AddTarget( const CORE::CString& target )
{GUCEF_TRACE;

    if ( HasTarget( target ) )
        return true;

    COMCORE::CHostAddress address;
    if ( !ParseTarget( target, address ) )
    {
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "PingService: Cannot ping \"" << target << "\": not a host address" );
        return false;
    }
    return AddTarget( target, address );
}

/*-------------------------------------------------------------------------*/

bool
CPingService::AddTarget( const CORE::CString& name, const COMCORE::CHostAddress& address )
{GUCEF_TRACE;

    TTargetMap::iterator i = m_targets.find( name );
    if ( i != m_targets.end() )
        return true;

    TTarget& target = m_targets[ name ];
    target.name = name;
    target.address = address;
    target.methodIndex = 0;
    target.failures = 0;
    target.rttMs = -1;
    target.switchPending = false;
    target.retryAtInMs = 0;

    // Look for the first method that can run
    target.methodIndex = (UInt32) -1;
    StartNextMethod( target );
    m_timer.SetEnabled( true );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CPingService::RemoveTarget( const CORE::CString& target )
{GUCEF_TRACE;

    TTargetMap::iterator i = m_targets.find( target );
    if ( i == m_targets.end() )
        return false;
    RetireProbe( (*i).second );
    m_targets.erase( i );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CPingService::RemoveAllTargets( void )
{GUCEF_TRACE;

    TTargetMap::iterator i = m_targets.begin();
    while ( i != m_targets.end() )
    {
        RetireProbe( (*i).second );
        ++i;
    }
    m_targets.clear();
}

/*-------------------------------------------------------------------------*/

bool
CPingService::HasTarget( const CORE::CString& target ) const
{GUCEF_TRACE;

    return m_targets.find( target ) != m_targets.end();
}

/*-------------------------------------------------------------------------*/

void
CPingService::GetTargets( CORE::CString::StringVector& targets ) const
{GUCEF_TRACE;

    TTargetMap::const_iterator i = m_targets.begin();
    while ( i != m_targets.end() )
    {
        targets.push_back( (*i).first );
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

Int32
CPingService::GetRttMs( const CORE::CString& target ) const
{GUCEF_TRACE;

    TTargetMap::const_iterator i = m_targets.find( target );
    if ( i == m_targets.end() || (*i).second.rttMs < 0 )
        return -1;
    return (Int32)( (*i).second.rttMs + 0.5 );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CPingService::GetMethod( const CORE::CString& target ) const
{GUCEF_TRACE;

    TTargetMap::const_iterator i = m_targets.find( target );
    if ( i == m_targets.end() || (*i).second.probe.IsNULL() )
        return CORE::CString::Empty;
    return (*i).second.probe->GetMethodName();
}

/*-------------------------------------------------------------------------*/

void
CPingService::SetMethods( const CORE::CString::StringVector& methods )
{GUCEF_TRACE;

    m_methods = methods;
}

/*-------------------------------------------------------------------------*/

const CORE::CString::StringVector&
CPingService::GetMethods( void ) const
{GUCEF_TRACE;

    return m_methods;
}

/*-------------------------------------------------------------------------*/

void
CPingService::SetIntervalInMs( UInt32 intervalInMs )
{GUCEF_TRACE;

    m_intervalInMs = intervalInMs > 0 ? intervalInMs : 1;
}

/*-------------------------------------------------------------------------*/

void
CPingService::SetTimeoutInMs( UInt32 timeoutInMs )
{GUCEF_TRACE;

    m_timeoutInMs = timeoutInMs > 0 ? timeoutInMs : 1;
}

/*-------------------------------------------------------------------------*/

void
CPingService::SetSmoothing( Float32 smoothing )
{GUCEF_TRACE;

    m_smoothing = smoothing <= 0.0f ? 0.01f : ( smoothing > 1.0f ? 1.0f : smoothing );
}

/*-------------------------------------------------------------------------*/

void
CPingService::SetMaxFailuresPerMethod( UInt32 maxFailures )
{GUCEF_TRACE;

    m_maxFailuresPerMethod = maxFailures > 0 ? maxFailures : 1;
}

/*-------------------------------------------------------------------------*/

void
CPingService::SetRetryAfterInMs( UInt32 retryAfterInMs )
{GUCEF_TRACE;

    m_retryAfterInMs = retryAfterInMs;
}

/*-------------------------------------------------------------------------*/

void
CPingService::RetireProbe( TTarget& target )
{GUCEF_TRACE;

    if ( target.probe.IsNULL() )
        return;
    UnsubscribeFrom( target.probe.GetPointerAlways() );
    target.probe->Stop();
    m_retiredProbes.push_back( target.probe );
    target.probe.Unlink();
}

/*-------------------------------------------------------------------------*/

bool
CPingService::StartNextMethod( TTarget& target )
{GUCEF_TRACE;

    RetireProbe( target );
    target.failures = 0;
    target.switchPending = false;

    COMCORE::CRttProbeFactory& factory = COMCORE::CComCoreGlobal::Instance()->GetRttProbeFactory();
    UInt32 methodCount = (UInt32) m_methods.size();
    UInt32 next = (UInt32)( target.methodIndex + 1 );    // wraps from (UInt32)-1 to 0
    for ( ; next < methodCount; ++next )
    {
        COMCORE::CIRttProbePtr probe = factory.Create( m_methods[ next ] );
        if ( probe.IsNULL() || !probe->IsAvailable() )
            continue;

        SubscribeTo( probe.GetPointerAlways() );
        if ( !probe->Start( target.address, 0, m_intervalInMs, m_timeoutInMs ) )
        {
            UnsubscribeFrom( probe.GetPointerAlways() );
            m_retiredProbes.push_back( probe );
            continue;
        }
        target.probe = probe;
        target.methodIndex = next;
        return true;
    }

    // Nothing works now: unknown, and the methods are tried again later
    target.methodIndex = methodCount;
    target.retryAtInMs = GetTimeInMs() + m_retryAfterInMs;
    SetUnknown( target );
    return false;
}

/*-------------------------------------------------------------------------*/

void
CPingService::SetUnknown( TTarget& target )
{GUCEF_TRACE;

    if ( target.rttMs < 0 )
        return;
    target.rttMs = -1;
    CPingUpdate update( target.name, -1, CORE::CString::Empty );
    NotifyObservers( PingUpdatedEvent, &update );
}

/*-------------------------------------------------------------------------*/

CPingService::TTarget*
CPingService::FindTargetOfProbe( CORE::CNotifier* probe )
{GUCEF_TRACE;

    TTargetMap::iterator i = m_targets.begin();
    while ( i != m_targets.end() )
    {
        if ( !(*i).second.probe.IsNULL() && static_cast< CORE::CNotifier* >( (*i).second.probe.GetPointerAlways() ) == probe )
            return &(*i).second;
        ++i;
    }
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

void
CPingService::OnProbeResult( TTarget& target, UInt64 rttInMicroSecs )
{GUCEF_TRACE;

    target.failures = 0;
    Float64 sampleMs = rttInMicroSecs / 1000.0;
    if ( target.rttMs < 0 )
        target.rttMs = sampleMs;
    else
        target.rttMs += m_smoothing * ( sampleMs - target.rttMs );

    CPingUpdate update( target.name, (Int32)( target.rttMs + 0.5 ), target.probe->GetMethodName() );
    NotifyObservers( PingUpdatedEvent, &update );
}

/*-------------------------------------------------------------------------*/

void
CPingService::OnProbeFailure( TTarget& target )
{GUCEF_TRACE;

    ++target.failures;
    if ( target.failures >= m_maxFailuresPerMethod && !target.switchPending )
    {
        // Not from inside the event of the probe: the timer moves on to the next method
        target.switchPending = true;
    }
}

/*-------------------------------------------------------------------------*/

void
CPingService::OnTimer( void )
{GUCEF_TRACE;

    m_retiredProbes.clear();

    UInt64 nowInMs = GetTimeInMs();
    TTargetMap::iterator i = m_targets.begin();
    while ( i != m_targets.end() )
    {
        TTarget& target = (*i).second;
        if ( target.switchPending )
        {
            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "PingService: The method " << target.probe->GetMethodName()
                << " gets no answers from " << target.name << ", trying the next" );
            StartNextMethod( target );
        }
        else
        if ( target.methodIndex >= m_methods.size() && nowInMs >= target.retryAtInMs )
        {
            target.methodIndex = (UInt32) -1;
            StartNextMethod( target );
        }
        ++i;
    }

    if ( m_targets.empty() && m_retiredProbes.empty() )
        m_timer.SetEnabled( false );
}

/*-------------------------------------------------------------------------*/

void
CPingService::OnNotify( CORE::CNotifier* notifier                 ,
                        const CORE::CEvent& eventId               ,
                        CORE::CICloneable* eventData /* = GUCEF_NULL */ )
{GUCEF_TRACE;

    if ( notifier == &m_timer )
    {
        if ( CORE::CTimer::TimerUpdateEvent == eventId )
            OnTimer();
        return;
    }

    TTarget* target = FindTargetOfProbe( notifier );
    if ( GUCEF_NULL == target )
        return;

    if ( COMCORE::CIRttProbe::ProbeResultEvent == eventId )
    {
        const COMCORE::CRttProbeEventData* data = static_cast< const COMCORE::CRttProbeEventData* >( eventData );
        if ( GUCEF_NULL != data )
            OnProbeResult( *target, data->GetRttInMicroSecs() );
    }
    else
    if ( COMCORE::CIRttProbe::ProbeTimeoutEvent == eventId || COMCORE::CIRttProbe::ProbeFailedEvent == eventId )
    {
        OnProbeFailure( *target );
    }
    else
    if ( COMCORE::CIRttProbe::ProbeFinishedEvent == eventId )
    {
        // A probe only finishes by itself when it cannot go on
        target->switchPending = true;
    }
}

/*-------------------------------------------------------------------------*/

bool
CPingService::SaveConfig( CORE::CDataNode& config ) const
{GUCEF_TRACE;

    config.SetAttribute( "methods", CORE::ToString( m_methods ) );
    config.SetAttribute( "intervalInMs", m_intervalInMs );
    config.SetAttribute( "timeoutInMs", m_timeoutInMs );
    config.SetAttribute( "smoothing", CORE::DoubleToString( m_smoothing ) );
    config.SetAttribute( "maxFailuresPerMethod", m_maxFailuresPerMethod );
    config.SetAttribute( "retryAfterInMs", m_retryAfterInMs );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CPingService::LoadConfig( const CORE::CDataNode& config )
{GUCEF_TRACE;

    CORE::CString methods = config.GetAttributeValueOrChildValueByName( "methods" ).AsString( CORE::CString::Empty, true );
    if ( !methods.IsNULLOrEmpty() )
    {
        CORE::CString::StringVector list = methods.ParseElements( ',', false );
        m_methods.clear();
        for ( size_t i=0; i<list.size(); ++i )
        {
            CORE::CString method = list[ i ].Trim( true ).Trim( false );
            if ( !method.IsNULLOrEmpty() )
                m_methods.push_back( method );
        }
    }
    SetIntervalInMs( config.GetAttributeValueOrChildValueByName( "intervalInMs" ).AsUInt32( m_intervalInMs, true ) );
    SetTimeoutInMs( config.GetAttributeValueOrChildValueByName( "timeoutInMs" ).AsUInt32( m_timeoutInMs, true ) );
    SetSmoothing( (Float32) CORE::StringToDouble( config.GetAttributeValueOrChildValueByName( "smoothing" ).AsString( CORE::DoubleToString( m_smoothing ), true ), m_smoothing ) );
    SetMaxFailuresPerMethod( config.GetAttributeValueOrChildValueByName( "maxFailuresPerMethod" ).AsUInt32( m_maxFailuresPerMethod, true ) );
    SetRetryAfterInMs( config.GetAttributeValueOrChildValueByName( "retryAfterInMs" ).AsUInt32( m_retryAfterInMs, true ) );
    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CPingService::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::MULTIPLAY::CPingService";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MULTIPLAY */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
