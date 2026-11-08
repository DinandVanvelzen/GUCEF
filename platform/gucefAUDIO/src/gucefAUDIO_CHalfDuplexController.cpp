/*
 *  gucefAUDIO: GUCEF module providing audio playback, mixing and decoding
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

#include <math.h>

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_AUDIO_CAUDIOLEVELMETER_H
#include "gucefAUDIO_CAudioLevelMeter.h"
#define GUCEF_AUDIO_CAUDIOLEVELMETER_H
#endif /* GUCEF_AUDIO_CAUDIOLEVELMETER_H ? */

#ifndef GUCEF_AUDIO_CAUDIOBUS_H
#include "gucefAUDIO_CAudioBus.h"
#define GUCEF_AUDIO_CAUDIOBUS_H
#endif /* GUCEF_AUDIO_CAUDIOBUS_H ? */

#ifndef GUCEF_AUDIO_CAUDIOENGINE_H
#include "gucefAUDIO_CAudioEngine.h"
#define GUCEF_AUDIO_CAUDIOENGINE_H
#endif /* GUCEF_AUDIO_CAUDIOENGINE_H ? */

#ifndef GUCEF_AUDIO_CDUCKEREFFECT_H
#include "gucefAUDIO_CDuckerEffect.h"
#define GUCEF_AUDIO_CDUCKEREFFECT_H
#endif /* GUCEF_AUDIO_CDUCKEREFFECT_H ? */

#include "gucefAUDIO_CHalfDuplexController.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUDIO {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** The time constant of the near-end power */
const Float64 NearTimeInS = 0.010;

/** The coupling counts as measured once this share of its averaging time was seen */
const Float64 MeasuredWeight = 0.25;

/** Powers below this count as silence (-200 dB) */
const Float64 TinyPower = 1.0e-20;

inline Float64
DecibelsToPower( Float64 decibels )
{
    return pow( 10.0, decibels / 10.0 );
}

inline Float32
PowerToDecibels( Float64 power )
{
    return power > TinyPower ? (Float32) ( 10.0 * log10( power ) ) : -200.0f;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CHalfDuplexController::CHalfDuplexController( const CHalfDuplexSettings& settings )
    : m_settings( settings )
    , m_farMeter( GUCEF_NULL )
    , m_farMeterSet( false )
    , m_duckerRef()
    , m_ducker( GUCEF_NULL )
    , m_detector()
    , m_sampleRate( 0 )
    , m_micGain( 1.0 )
    , m_farThreshold( 0.0 )
    , m_breakInFactor( 1.0 )
    , m_initialCoupling( 1.0 )
    , m_state( HALFDUPLEXSTATE_IDLE )
    , m_wasEnabled( settings.GetEnabled() )
    , m_duckKey( false )
    , m_nearPower( 0.0 )
    , m_farEnvelope( 0.0 )
    , m_nearAverage( 0.0 )
    , m_farEnvelopeAverage( 0.0 )
    , m_farMeanAverage( 0.0 )
    , m_couplingWeight( 0.0 )
    , m_breakInTime( 0.0 )
    , m_quietTime( 0.0 )
    , m_cancellerConverged( false )
    , m_cancellerErleDb( 0.0f )
    , m_handedOver( false )
    , m_enabled( settings.GetEnabled() )
    , m_publishedState( (UInt32) HALFDUPLEXSTATE_IDLE )
    , m_transitions( 0 )
    , m_nearVoice( false )
    , m_couplingMeasured( false )
    , m_publishedGain( 1.0f )
    , m_couplingDb( settings.GetInitialCouplingInDecibels() )
    , m_nearDb( -200.0f )
    , m_farDb( -200.0f )
    , m_echoDb( -200.0f )
    , m_publishedHandedOver( false )
    , m_handOvers( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CHalfDuplexController::~CHalfDuplexController()
{GUCEF_TRACE;

    SetDuckKey( false );
}

/*-------------------------------------------------------------------------*/

void
CHalfDuplexController::SetSettings( const CHalfDuplexSettings& settings )
{GUCEF_TRACE;

    m_settings = settings;
    m_enabled.Store( settings.GetEnabled() );
}

/*-------------------------------------------------------------------------*/

const CHalfDuplexSettings&
CHalfDuplexController::GetSettings( void ) const
{GUCEF_TRACE;

    return m_settings;
}

/*-------------------------------------------------------------------------*/

void
CHalfDuplexController::SetFarEndBus( const CAudioBus* bus )
{GUCEF_TRACE;

    m_farMeter = GUCEF_NULL != bus ? &bus->GetMeter() : GUCEF_NULL;
    m_farMeterSet = GUCEF_NULL != bus;
}

/*-------------------------------------------------------------------------*/

void
CHalfDuplexController::SetFarEndMeter( const CAudioLevelMeter* meter )
{GUCEF_TRACE;

    m_farMeter = meter;
    m_farMeterSet = GUCEF_NULL != meter;
}

/*-------------------------------------------------------------------------*/

const CAudioLevelMeter*
CHalfDuplexController::GetFarEndMeter( void ) const
{GUCEF_TRACE;

    return m_farMeter;
}

/*-------------------------------------------------------------------------*/

bool
CHalfDuplexController::SetDucker( const TAudioEffectPtr& ducker )
{GUCEF_TRACE;

    if ( !ducker.IsNULL() && ducker->GetEffectTypeName() != CDuckerEffect::GetStaticEffectTypeName() )
        return false;

    SetDuckKey( false );
    m_duckerRef = ducker;
    // the type name was checked: the effect is a CDuckerEffect
    m_ducker = m_duckerRef.IsNULL() ? GUCEF_NULL : static_cast< CDuckerEffect* >( m_duckerRef.GetPointerAlways() );
    return true;
}

/*-------------------------------------------------------------------------*/

const TAudioEffectPtr&
CHalfDuplexController::GetDucker( void ) const
{GUCEF_TRACE;

    return m_duckerRef;
}

/*-------------------------------------------------------------------------*/

bool
CHalfDuplexController::ResolveReferences( CAudioEngine& engine )
{GUCEF_TRACE;

    if ( m_farMeterSet )
        return true;

    const CString& name = m_settings.GetFarEndBusName();
    const CAudioBus* bus = name.IsNULLOrEmpty() ? engine.GetMasterBus() : engine.FindBus( name );
    if ( GUCEF_NULL == bus )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "HalfDuplexController: The far-end bus \"" << name << "\" does not exist in the engine" );
        return false;
    }
    m_farMeter = &bus->GetMeter();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CHalfDuplexController::Prepare( UInt32 sampleRate )
{GUCEF_TRACE;

    m_detector.SetSettings( m_settings.GetVoiceActivitySettings() );
    if ( !m_detector.Prepare( sampleRate ) )
    {
        m_sampleRate = 0;
        return false;
    }
    m_sampleRate = sampleRate;
    m_micGain = pow( 10.0, (Float64) m_settings.GetMicAttenuationInDecibels() / 20.0 );
    m_farThreshold = pow( 10.0, (Float64) m_settings.GetFarThresholdInDecibels() / 20.0 );
    m_breakInFactor = DecibelsToPower( (Float64) m_settings.GetBreakInMarginInDecibels() );
    m_initialCoupling = DecibelsToPower( (Float64) m_settings.GetInitialCouplingInDecibels() );
    Reset();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CHalfDuplexController::IsPrepared( void ) const
{GUCEF_TRACE;

    return 0 != m_sampleRate;
}

/*-------------------------------------------------------------------------*/

UInt32
CHalfDuplexController::GetSampleRate( void ) const
{GUCEF_TRACE;

    return m_sampleRate;
}

/*-------------------------------------------------------------------------*/

void
CHalfDuplexController::Reset( void )
{GUCEF_TRACE;

    SetDuckKey( false );
    if ( m_detector.IsPrepared() )
        m_detector.Reset();
    m_state = HALFDUPLEXSTATE_IDLE;
    m_wasEnabled = m_enabled.Load();
    m_nearPower = 0.0;
    m_farEnvelope = 0.0;
    m_nearAverage = 0.0;
    m_farEnvelopeAverage = 0.0;
    m_farMeanAverage = 0.0;
    m_couplingWeight = 0.0;
    m_breakInTime = 0.0;
    m_quietTime = 0.0;
    m_cancellerConverged = false;
    m_cancellerErleDb = 0.0f;
    m_handedOver = false;
    m_publishedHandedOver.Store( false );
    m_handOvers.Store( 0 );
    m_publishedState.Store( (UInt32) HALFDUPLEXSTATE_IDLE );
    m_transitions.Store( 0 );
    m_nearVoice.Store( false );
    m_couplingMeasured.Store( false );
    m_publishedGain.Store( 1.0f );
    m_couplingDb.Store( m_settings.GetInitialCouplingInDecibels() );
    m_nearDb.Store( -200.0f );
    m_farDb.Store( -200.0f );
    m_echoDb.Store( -200.0f );
}

/*-------------------------------------------------------------------------*/

void
CHalfDuplexController::SetDuckKey( bool active )
{
    // the ducker's manual key is an atomic: fit for the capture thread
    if ( m_duckKey == active )
        return;
    m_duckKey = active;
    if ( GUCEF_NULL != m_ducker )
        m_ducker->SetManualKey( active );
}

/*-------------------------------------------------------------------------*/

void
CHalfDuplexController::EnterState( THalfDuplexState state )
{
    if ( state == m_state )
        return;
    m_state = state;
    m_breakInTime = 0.0;
    m_quietTime = 0.0;
    SetDuckKey( HALFDUPLEXSTATE_NEAR == state && m_settings.GetDuckFarEnd() );
    m_publishedState.Store( (UInt32) state, MT::MEMORDER_RELAXED );
    m_transitions.FetchAdd( 1, MT::MEMORDER_RELAXED );
}

/*-------------------------------------------------------------------------*/

Float32
CHalfDuplexController::Process( const Float32* mono, UInt32 frameCount, UInt64 firstFrame ) GUCEF_AUDIO_REALTIME_CONTEXT
{
    if ( 0 == m_sampleRate || GUCEF_NULL == mono || 0 == frameCount )
        return (Float32) ( HALFDUPLEXSTATE_FAR == m_state ? m_micGain : 1.0 );

    // switched off: everything as if there were no controller; back on, it starts from IDLE
    const bool enabled = m_enabled.Load( MT::MEMORDER_RELAXED );
    if ( !enabled )
    {
        if ( m_wasEnabled )
        {
            EnterState( HALFDUPLEXSTATE_IDLE );
            m_publishedGain.Store( 1.0f );
            m_handedOver = false;
            m_publishedHandedOver.Store( false, MT::MEMORDER_RELAXED );
        }
        m_wasEnabled = false;
        return 1.0f;
    }
    m_wasEnabled = true;

    const Float64 dt = (Float64) frameCount / (Float64) m_sampleRate;

    // the near end: power of the block, smoothed, and the detector of the near-end voice
    Float64 sum = 0.0;
    for ( UInt32 n=0; n<frameCount; ++n )
        sum += (Float64) mono[ n ] * (Float64) mono[ n ];
    const Float64 blockPower = sum / (Float64) frameCount;
    const Float64 nearCoefficient = 1.0 - exp( -dt / NearTimeInS );
    m_nearPower += nearCoefficient * ( blockPower - m_nearPower );
    const bool nearVoice = m_detector.Process( mono, frameCount, firstFrame );

    // the far end: the peak of its meter's last block gives the activity and, held, the envelope of the echo
    Float64 farPeak = 0.0;
    Float64 farMeanPower = 0.0;
    if ( GUCEF_NULL != m_farMeter )
    {
        const UInt32 channels = m_farMeter->GetChannelCount();
        for ( UInt32 c=0; c<channels; ++c )
        {
            const Float64 peak = (Float64) m_farMeter->GetPeak( c );
            const Float64 rms = (Float64) m_farMeter->GetRms( c );
            if ( peak > farPeak )
                farPeak = peak;
            if ( rms * rms > farMeanPower )
                farMeanPower = rms * rms;
        }
    }
    const Float64 farPower = farPeak * farPeak;
    const Float64 release = exp( -dt / ( (Float64) m_settings.GetFarReleaseInMs() / 1000.0 ) );
    m_farEnvelope = farPower > m_farEnvelope * release ? farPower : m_farEnvelope * release;
    const bool farActive = farPeak > m_farThreshold;

    // the echo the far end makes in the microphone, and whether the near end talks over it
    const Float64 coupling = m_couplingWeight >= MeasuredWeight && m_farEnvelopeAverage > TinyPower ? m_nearAverage / m_farEnvelopeAverage : m_initialCoupling;
    const Float64 echo = m_farEnvelope * coupling;
    const bool nearTalks = nearVoice && m_nearPower > echo * m_breakInFactor;

    switch ( m_state )
    {
        case HALFDUPLEXSTATE_FAR:
        {
            if ( farActive && !nearTalks )
            {
                // only the far end talks: what the microphone hears is its echo (and the room's noise)
                const Float64 a = 1.0 - exp( -dt / ( (Float64) m_settings.GetCouplingTimeInMs() / 1000.0 ) );
                m_nearAverage += a * ( m_nearPower - m_nearAverage );
                m_farEnvelopeAverage += a * ( m_farEnvelope - m_farEnvelopeAverage );
                m_farMeanAverage += a * ( farMeanPower - m_farMeanAverage );
                m_couplingWeight += a * ( 1.0 - m_couplingWeight );
                if ( m_couplingWeight >= MeasuredWeight && m_farMeanAverage > TinyPower )
                {
                    m_couplingDb.Store( PowerToDecibels( m_nearAverage / m_farMeanAverage ), MT::MEMORDER_RELAXED );
                    m_couplingMeasured.Store( true, MT::MEMORDER_RELAXED );
                }
            }
            m_breakInTime = nearTalks ? m_breakInTime + dt : 0.0;
            m_quietTime = farActive ? 0.0 : m_quietTime + dt;
            if ( nearTalks && m_breakInTime * 1000.0 >= (Float64) m_settings.GetBreakInInMs() )
                EnterState( HALFDUPLEXSTATE_NEAR );
            else
            if ( !farActive && m_quietTime * 1000.0 >= (Float64) m_settings.GetHangoverInMs() )
                EnterState( HALFDUPLEXSTATE_IDLE );
            break;
        }
        case HALFDUPLEXSTATE_NEAR:
        {
            m_quietTime = nearTalks ? 0.0 : m_quietTime + dt;
            if ( !nearTalks && m_quietTime * 1000.0 >= (Float64) m_settings.GetHangoverInMs() )
                EnterState( HALFDUPLEXSTATE_IDLE );
            break;
        }
        case HALFDUPLEXSTATE_IDLE:
        default:
        {
            if ( nearTalks )
                EnterState( HALFDUPLEXSTATE_NEAR );
            else
            if ( farActive )
                EnterState( HALFDUPLEXSTATE_FAR );
            break;
        }
    }

    // the hand-over: while the echo canceller holds the echo down the controller stands aside
    const Float32 handOverLevel = m_settings.GetHandOverErleInDecibels();
    const bool handOverAllowed = m_settings.GetEchoCancellerHandOver() && m_cancellerConverged;
    if ( !m_handedOver )
    {
        if ( handOverAllowed && m_cancellerErleDb >= handOverLevel )
        {
            m_handedOver = true;
            m_handOvers.FetchAdd( 1, MT::MEMORDER_RELAXED );
        }
    }
    else
    if ( !handOverAllowed || m_cancellerErleDb < handOverLevel - 6.0f )
    {
        m_handedOver = false;
    }
    SetDuckKey( !m_handedOver && HALFDUPLEXSTATE_NEAR == m_state && m_settings.GetDuckFarEnd() );
    m_publishedHandedOver.Store( m_handedOver, MT::MEMORDER_RELAXED );

    const Float32 gain = (Float32) ( HALFDUPLEXSTATE_FAR == m_state && !m_handedOver ? m_micGain : 1.0 );
    m_publishedGain.Store( gain, MT::MEMORDER_RELAXED );
    m_nearVoice.Store( nearVoice, MT::MEMORDER_RELAXED );
    m_nearDb.Store( PowerToDecibels( m_nearPower ), MT::MEMORDER_RELAXED );
    m_farDb.Store( PowerToDecibels( m_farEnvelope ), MT::MEMORDER_RELAXED );
    m_echoDb.Store( PowerToDecibels( echo ), MT::MEMORDER_RELAXED );
    return gain;
}

/*-------------------------------------------------------------------------*/

void
CHalfDuplexController::SetEchoCancellation( bool converged, Float32 erleInDecibels ) GUCEF_AUDIO_REALTIME_CONTEXT
{
    m_cancellerConverged = converged;
    m_cancellerErleDb = erleInDecibels;
}

/*-------------------------------------------------------------------------*/

bool
CHalfDuplexController::IsHandedOver( void ) const
{GUCEF_TRACE;

    return m_publishedHandedOver.Load( MT::MEMORDER_RELAXED );
}

/*-------------------------------------------------------------------------*/

UInt64
CHalfDuplexController::GetHandOverCount( void ) const
{GUCEF_TRACE;

    return m_handOvers.Load( MT::MEMORDER_RELAXED );
}

/*-------------------------------------------------------------------------*/

void
CHalfDuplexController::SetEnabled( bool enabled )
{GUCEF_TRACE;

    m_enabled.Store( enabled );
}

/*-------------------------------------------------------------------------*/

bool
CHalfDuplexController::IsEnabled( void ) const
{GUCEF_TRACE;

    return m_enabled.Load();
}

/*-------------------------------------------------------------------------*/

THalfDuplexState
CHalfDuplexController::GetState( void ) const
{GUCEF_TRACE;

    return (THalfDuplexState) m_publishedState.Load( MT::MEMORDER_RELAXED );
}

/*-------------------------------------------------------------------------*/

Float32
CHalfDuplexController::GetMicGain( void ) const
{GUCEF_TRACE;

    return m_publishedGain.Load();
}

/*-------------------------------------------------------------------------*/

Float32
CHalfDuplexController::GetCouplingInDecibels( void ) const
{GUCEF_TRACE;

    return m_couplingDb.Load();
}

/*-------------------------------------------------------------------------*/

bool
CHalfDuplexController::HasMeasuredCoupling( void ) const
{GUCEF_TRACE;

    return m_couplingMeasured.Load( MT::MEMORDER_RELAXED );
}

/*-------------------------------------------------------------------------*/

Float32
CHalfDuplexController::GetNearLevelInDecibels( void ) const
{GUCEF_TRACE;

    return m_nearDb.Load();
}

/*-------------------------------------------------------------------------*/

Float32
CHalfDuplexController::GetFarLevelInDecibels( void ) const
{GUCEF_TRACE;

    return m_farDb.Load();
}

/*-------------------------------------------------------------------------*/

Float32
CHalfDuplexController::GetEchoEstimateInDecibels( void ) const
{GUCEF_TRACE;

    return m_echoDb.Load();
}

/*-------------------------------------------------------------------------*/

bool
CHalfDuplexController::IsNearVoiceActive( void ) const
{GUCEF_TRACE;

    return m_nearVoice.Load( MT::MEMORDER_RELAXED );
}

/*-------------------------------------------------------------------------*/

UInt64
CHalfDuplexController::GetTransitionCount( void ) const
{GUCEF_TRACE;

    return m_transitions.Load( MT::MEMORDER_RELAXED );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
