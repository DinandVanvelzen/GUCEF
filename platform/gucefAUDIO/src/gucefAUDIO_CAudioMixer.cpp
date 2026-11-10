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

#include <string.h>

#include "gucefAUDIO_CAudioMixer.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUDIO {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CAudioMixer::CAudioMixer( const CAudioFormat& mixFormat  ,
                          UInt32 maxBlockFrames          ,
                          UInt32 maxVoices               ,
                          UInt32 maxBuses                ,
                          UInt32 maxEffectsPerBus        ,
                          UInt32 maxSendsPerBus          )
    : m_mixFormat( mixFormat )
    , m_maxBlock( maxBlockFrames > 0 ? maxBlockFrames : 1 )
    , m_maxVoices( maxVoices )
    , m_maxBuses( maxBuses > 0 ? maxBuses : 1 )
    , m_maxEffectsPerBus( maxEffectsPerBus )
    , m_maxSendsPerBus( maxSendsPerBus )
    , m_ownedBuses()
    , m_buses()
    , m_order( GUCEF_NULL )
    , m_orderCount( 0 )
    , m_voices()
    , m_listener( GUCEF_NULL )
    , m_panLaw( AUDIOPANLAW_CONSTANT_POWER )
    , m_totalFrames( 0 )
    , m_clipped( 0 )
    , m_limiterEnabled( false )
    , m_limiterOnMode( AUDIOLIMITERMODE_LOOKAHEAD )
    , m_limiterCeilingInDecibels( -0.3f )
    , m_limiterLookaheadInMs( 1.5f )
    , m_limiterReleaseInMs( 50.0f )
    , m_limiter()
{GUCEF_TRACE;

    // The mix format is always float
    m_mixFormat.SetSampleType( SAMPLETYPE_FLOAT32 );

    // The master limiter stage: off until asked for, ready for every mode
    ApplyLimiterSettings();
    m_limiter.Prepare( m_mixFormat, m_maxBlock );

    // Everything that may grow while mixing is given its room now
    m_ownedBuses.reserve( m_maxBuses );
    m_buses.reserve( m_maxBuses );
    m_voices.reserve( m_maxVoices );

    TAudioBusPtr master( GUCEF_NEW CAudioBus( "master", GUCEF_NULL ) );
    PrepareBusFor( master.GetPointerAlways() );
    m_ownedBuses.push_back( master );
    m_buses.push_back( master.GetPointerAlways() );
}

/*-------------------------------------------------------------------------*/

CAudioMixer::~CAudioMixer()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CAudioMixer::ApplyLimiterSettings( void )
{GUCEF_TRACE;

    // published together, so the mixing thread never sees a mode with the ceiling of another
    const TAudioLimiterMode mode = m_limiterEnabled ? m_limiterOnMode : AUDIOLIMITERMODE_OFF;
    const UInt32 ids[ 5 ] = { CLimiterEffect::PARAMETER_MODE, CLimiterEffect::PARAMETER_CEILING, CLimiterEffect::PARAMETER_LOOKAHEAD,
                              CLimiterEffect::PARAMETER_RELEASE, CLimiterEffect::PARAMETER_INPUT_GAIN };
    const Float32 values[ 5 ] = { (Float32) mode,
                                  AUDIOLIMITERMODE_INSTANT == mode ? CLimiterEffect::GetMixerCeilingInDecibels() : m_limiterCeilingInDecibels,
                                  m_limiterLookaheadInMs, m_limiterReleaseInMs, 0.0f };
    m_limiter.SetParameters( ids, values, 5 );
}

/*-------------------------------------------------------------------------*/

const CAudioFormat&
CAudioMixer::GetMixFormat( void ) const
{GUCEF_TRACE;

    return m_mixFormat;
}

/*-------------------------------------------------------------------------*/

UInt32
CAudioMixer::GetMaxBlockFrames( void ) const
{GUCEF_TRACE;

    return m_maxBlock;
}

/*-------------------------------------------------------------------------*/

UInt32
CAudioMixer::GetMaxBuses( void ) const
{GUCEF_TRACE;

    return m_maxBuses;
}

/*-------------------------------------------------------------------------*/

UInt32
CAudioMixer::GetMaxEffectsPerBus( void ) const
{GUCEF_TRACE;

    return m_maxEffectsPerBus;
}

/*-------------------------------------------------------------------------*/

UInt32
CAudioMixer::GetMaxSendsPerBus( void ) const
{GUCEF_TRACE;

    return m_maxSendsPerBus;
}

/*-------------------------------------------------------------------------*/

void
CAudioMixer::PrepareBusFor( CAudioBus* bus )
{GUCEF_TRACE;

    const UInt32 channels = m_mixFormat.GetChannelCount();
    bus->m_buffer.assign( (size_t) m_maxBlock * channels, 0.0f );
    bus->m_effects.Prepare( m_maxEffectsPerBus, m_mixFormat, m_maxBlock );
    bus->PrepareSends( m_maxSendsPerBus );
    bus->m_meter.Prepare( channels, m_mixFormat.GetSampleRate() );
    bus->m_gainKnown = false;
    for ( UInt32 s=0; s<bus->m_sendCapacity; ++s )
        bus->m_sends[ s ].levelKnown = false;
}

/*-------------------------------------------------------------------------*/

bool
CAudioMixer::SetMixFormat( const CAudioFormat& mixFormat )
{GUCEF_TRACE;

    if ( !mixFormat.IsValid() )
        return false;

    m_mixFormat = mixFormat;
    m_mixFormat.SetSampleType( SAMPLETYPE_FLOAT32 );
    m_limiter.Prepare( m_mixFormat, m_maxBlock );

    for ( size_t b=0; b<m_buses.size(); ++b )
        PrepareBusFor( m_buses[ b ] );

    bool allVoices = true;
    for ( size_t v=0; v<m_voices.size(); ++v )
        allVoices = m_voices[ v ]->Reprepare( m_mixFormat, m_maxBlock ) && allVoices;
    return allVoices;
}

/*-------------------------------------------------------------------------*/

CAudioBus*
CAudioMixer::GetMasterBus( void )
{GUCEF_TRACE;

    return m_buses[ 0 ];
}

/*-------------------------------------------------------------------------*/

CAudioBus*
CAudioMixer::CreateBus( const CString& name, CAudioBus* parent )
{GUCEF_TRACE;

    if ( GUCEF_NULL == parent )
        parent = GetMasterBus();

    TAudioBusPtr bus( GUCEF_NEW CAudioBus( name, parent ) );
    if ( !PrepareBus( bus.GetPointerAlways() ) || !AttachBus( bus.GetPointerAlways() ) )
        return GUCEF_NULL;
    m_ownedBuses.push_back( bus );
    return bus.GetPointerAlways();
}

/*-------------------------------------------------------------------------*/

bool
CAudioMixer::PrepareBus( CAudioBus* bus )
{GUCEF_TRACE;

    if ( GUCEF_NULL == bus )
        return false;
    PrepareBusFor( bus );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAudioMixer::AttachBus( CAudioBus* bus )
{
    if ( GUCEF_NULL == bus || m_buses.size() >= m_maxBuses )
        return false;
    if ( bus->m_buffer.size() != (size_t) m_maxBlock * m_mixFormat.GetChannelCount() )
        return false;

    // a parent has to be one of ours, the order in the list is what makes mixing children before parents work
    bool known = false;
    for ( size_t i=0; i<m_buses.size(); ++i )
        if ( m_buses[ i ] == bus->m_parent )
            known = true;
    if ( !known )
        return false;

    m_buses.push_back( bus );
    return true;
}

/*-------------------------------------------------------------------------*/

CAudioBus*
CAudioMixer::FindBus( const CString& name )
{GUCEF_TRACE;

    for ( size_t i=0; i<m_buses.size(); ++i )
        if ( m_buses[ i ]->GetName() == name )
            return m_buses[ i ];
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

UInt32
CAudioMixer::GetBusCount( void ) const
{GUCEF_TRACE;

    return (UInt32) m_buses.size();
}

/*-------------------------------------------------------------------------*/

void
CAudioMixer::SetBusOrder( CAudioBus* const* order, UInt32 count )
{
    m_order = order;
    m_orderCount = GUCEF_NULL != order ? count : 0;
}

/*-------------------------------------------------------------------------*/

bool
CAudioMixer::SetBusEffect( CAudioBus* bus, UInt32 slot, CAudioEffect* effect )
{
    if ( GUCEF_NULL == bus )
        return false;
    return bus->m_effects.SetSlotEffect( slot, effect );
}

/*-------------------------------------------------------------------------*/

CAudioEffect*
CAudioMixer::GetBusEffect( CAudioBus* bus, UInt32 slot ) const
{
    if ( GUCEF_NULL == bus )
        return GUCEF_NULL;
    return bus->m_effects.GetSlotEffect( slot );
}

/*-------------------------------------------------------------------------*/

bool
CAudioMixer::SetBusSend( CAudioBus* bus, UInt32 sendIndex, CAudioBus* target, bool preFader )
{
    if ( GUCEF_NULL == bus || sendIndex >= bus->m_sendCapacity || target == bus )
        return false;

    CAudioBus::SSend& send = bus->m_sends[ sendIndex ];
    send.target = target;
    send.preFader = preFader;
    send.levelKnown = false;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAudioMixer::PrepareVoice( CAudioVoice* voice )
{GUCEF_TRACE;

    if ( GUCEF_NULL == voice )
        return false;
    if ( GUCEF_NULL == voice->GetBus() )
        voice->SetBus( GetMasterBus() );
    return voice->Prepare( m_mixFormat, m_maxBlock );
}

/*-------------------------------------------------------------------------*/

bool
CAudioMixer::ReprepareVoice( CAudioVoice* voice )
{GUCEF_TRACE;

    if ( GUCEF_NULL == voice )
        return false;
    return voice->Reprepare( m_mixFormat, m_maxBlock );
}

/*-------------------------------------------------------------------------*/

bool
CAudioMixer::AddVoice( CAudioVoice* voice )
{
    if ( GUCEF_NULL == voice || !voice->m_prepared || m_voices.size() >= m_maxVoices )
        return false;
    if ( GUCEF_NULL == voice->m_bus )
        voice->m_bus = m_buses[ 0 ];

    voice->m_state.Store( AUDIOVOICESTATE_PLAYING, MT::MEMORDER_RELEASE );
    m_voices.push_back( voice );
    return true;
}

/*-------------------------------------------------------------------------*/

UInt32
CAudioMixer::GetVoiceCount( void ) const
{GUCEF_TRACE;

    return (UInt32) m_voices.size();
}

/*-------------------------------------------------------------------------*/

void
CAudioMixer::SetListener( IAudioMixerListener* listener )
{GUCEF_TRACE;

    m_listener = listener;
}

/*-------------------------------------------------------------------------*/

void
CAudioMixer::SetPanLaw( TAudioPanLaw panLaw )
{GUCEF_TRACE;

    m_panLaw = panLaw;
}

/*-------------------------------------------------------------------------*/

UInt64
CAudioMixer::GetTotalFramesMixed( void ) const
{GUCEF_TRACE;

    return m_totalFrames;
}

/*-------------------------------------------------------------------------*/

void
CAudioMixer::SetLimiterMode( TAudioLimiterMode mode )
{GUCEF_TRACE;

    if ( AUDIOLIMITERMODE_INSTANT == mode || AUDIOLIMITERMODE_LOOKAHEAD == mode )
    {
        m_limiterOnMode = mode;
        m_limiterEnabled = true;
    }
    else
        m_limiterEnabled = false;
    ApplyLimiterSettings();
}

/*-------------------------------------------------------------------------*/

TAudioLimiterMode
CAudioMixer::GetLimiterMode( void ) const
{GUCEF_TRACE;

    return m_limiterEnabled ? m_limiterOnMode : AUDIOLIMITERMODE_OFF;
}

/*-------------------------------------------------------------------------*/

void
CAudioMixer::SetLimiterEnabled( bool enabled )
{GUCEF_TRACE;

    m_limiterEnabled = enabled;
    ApplyLimiterSettings();
}

/*-------------------------------------------------------------------------*/

bool
CAudioMixer::IsLimiterEnabled( void ) const
{GUCEF_TRACE;

    return m_limiterEnabled;
}

/*-------------------------------------------------------------------------*/

void
CAudioMixer::SetLimiterCeilingInDecibels( Float32 decibels )
{GUCEF_TRACE;

    m_limiterCeilingInDecibels = decibels;
    ApplyLimiterSettings();
}

/*-------------------------------------------------------------------------*/

Float32
CAudioMixer::GetLimiterCeilingInDecibels( void ) const
{GUCEF_TRACE;

    return m_limiterCeilingInDecibels;
}

/*-------------------------------------------------------------------------*/

void
CAudioMixer::SetLimiterLookaheadInMs( Float32 milliseconds )
{GUCEF_TRACE;

    m_limiterLookaheadInMs = milliseconds;
    ApplyLimiterSettings();
}

/*-------------------------------------------------------------------------*/

Float32
CAudioMixer::GetLimiterLookaheadInMs( void ) const
{GUCEF_TRACE;

    return m_limiterLookaheadInMs;
}

/*-------------------------------------------------------------------------*/

void
CAudioMixer::SetLimiterReleaseInMs( Float32 milliseconds )
{GUCEF_TRACE;

    m_limiterReleaseInMs = milliseconds;
    ApplyLimiterSettings();
}

/*-------------------------------------------------------------------------*/

Float32
CAudioMixer::GetLimiterReleaseInMs( void ) const
{GUCEF_TRACE;

    return m_limiterReleaseInMs;
}

/*-------------------------------------------------------------------------*/

UInt32
CAudioMixer::GetLatencyInFrames( void ) const
{GUCEF_TRACE;

    return m_limiter.GetLatencyInFrames();
}

/*-------------------------------------------------------------------------*/

UInt64
CAudioMixer::GetLimitedFrameCount( void ) const
{GUCEF_TRACE;

    return m_limiter.GetLimitedFrameCount();
}

/*-------------------------------------------------------------------------*/

UInt64
CAudioMixer::GetClippedSampleCount( void ) const
{GUCEF_TRACE;

    return m_clipped;
}

/*-------------------------------------------------------------------------*/

void
CAudioMixer::Mix( Float32* output, UInt32 frames )
{
    const UInt32 channels = m_mixFormat.GetChannelCount();
    while ( frames > 0 )
    {
        const UInt32 block = ( frames < m_maxBlock ) ? frames : m_maxBlock;
        MixBlock( output, block );
        output += (size_t) block * channels;
        frames -= block;
    }
}

/*-------------------------------------------------------------------------*/

void
CAudioMixer::ApplySends( CAudioBus* bus, bool preFader, UInt32 frames, UInt32 channels )
{
    const Float32* data = &bus->m_buffer[ 0 ];
    for ( UInt32 s=0; s<bus->m_sendCapacity; ++s )
    {
        CAudioBus::SSend& send = bus->m_sends[ s ];
        if ( GUCEF_NULL == send.target || send.preFader != preFader )
            continue;

        const Float32 target = send.level.Load();
        const Float32 from = send.levelKnown ? send.lastLevel : target;
        send.lastLevel = target;
        send.levelKnown = true;

        Float32* into = &send.target->m_buffer[ 0 ];
        if ( from == target )
        {
            if ( 0.0f == target )
                continue;
            const size_t samples = (size_t) frames * channels;
            for ( size_t i=0; i<samples; ++i )
                into[ i ] += data[ i ] * target;
        }
        else
        {
            const Float32 step = ( target - from ) / (Float32) frames;
            for ( UInt32 f=0; f<frames; ++f )
            {
                const Float32 g = from + step * (Float32) ( f + 1 );
                for ( UInt32 c=0; c<channels; ++c )
                    into[ (size_t) f * channels + c ] += data[ (size_t) f * channels + c ] * g;
            }
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CAudioMixer::ProcessBus( CAudioBus* bus, UInt32 frames )
{
    const UInt32 channels = m_mixFormat.GetChannelCount();
    const size_t samples = (size_t) frames * channels;
    Float32* data = &bus->m_buffer[ 0 ];

    // inserts, then what is sent before the fader
    bus->m_effects.Process( data, frames );
    ApplySends( bus, true, frames, channels );

    // the fader: volume and mute, a change spread over the block
    const Float32 target = bus->m_muted.Load( MT::MEMORDER_RELAXED ) ? 0.0f : bus->m_volume.Load();
    const Float32 from = bus->m_gainKnown ? bus->m_lastGain : target;
    bus->m_lastGain = target;
    bus->m_gainKnown = true;

    const Float32 step = ( target - from ) / (Float32) frames;
    if ( from == target )
    {
        if ( 1.0f != target )
            for ( size_t i=0; i<samples; ++i )
                data[ i ] *= target;
    }
    else
    {
        for ( UInt32 f=0; f<frames; ++f )
        {
            const Float32 g = from + step * (Float32) ( f + 1 );
            for ( UInt32 c=0; c<channels; ++c )
                data[ (size_t) f * channels + c ] *= g;
        }
    }

    // what the bus delivers is what its meter shows, and what is sent after the fader
    bus->m_meter.Update( data, frames );
    ApplySends( bus, false, frames, channels );

    if ( GUCEF_NULL != bus->m_parent )
    {
        Float32* parent = &bus->m_parent->m_buffer[ 0 ];
        for ( size_t i=0; i<samples; ++i )
            parent[ i ] += data[ i ];
    }
}

/*-------------------------------------------------------------------------*/

void
CAudioMixer::MixBlock( Float32* output, UInt32 frames )
{
    const UInt32 channels = m_mixFormat.GetChannelCount();
    const size_t samples = (size_t) frames * channels;

    // 1. every bus starts the block empty
    if ( GUCEF_NULL != m_order )
    {
        for ( UInt32 b=0; b<m_orderCount; ++b )
            memset( &m_order[ b ]->m_buffer[ 0 ], 0, samples * sizeof( Float32 ) );
    }
    else
    {
        for ( size_t b=0; b<m_buses.size(); ++b )
            memset( &m_buses[ b ]->m_buffer[ 0 ], 0, samples * sizeof( Float32 ) );
    }

    // 2. the voices add themselves to the bus they play into, and to the aux buses of their sends. All voices are mixed
    //    before any bus is processed, so a voice can send to any bus and its sends add no edge to the bus order.
    //    Voices that are over are taken out and reported
    size_t v = 0;
    while ( v < m_voices.size() )
    {
        CAudioVoice* voice = m_voices[ v ];
        CAudioBus* bus = voice->m_bus;
        voice->RenderBlock( &bus->m_buffer[ 0 ], frames, m_panLaw );

        if ( AUDIOVOICESTATE_ENDED == voice->m_state.Load( MT::MEMORDER_ACQUIRE ) )
        {
            m_voices.erase( m_voices.begin() + v );
            if ( GUCEF_NULL != m_listener )
                m_listener->OnVoiceEnded( voice, (TAudioVoiceEndReason) voice->m_endReason.Load( MT::MEMORDER_ACQUIRE ) );
        }
        else
            ++v;
    }

    // 3. buses pass what they hold on to their parent and their send targets, every bus after the buses that feed it.
    //    The master bus comes last, it is the output
    if ( GUCEF_NULL != m_order )
    {
        for ( UInt32 b=0; b<m_orderCount; ++b )
            ProcessBus( m_order[ b ], frames );
    }
    else
    {
        for ( size_t b=m_buses.size(); b>0; --b )
            ProcessBus( m_buses[ b - 1 ], frames );
    }

    // 4. the master bus is what comes out: the limiter stage turns it down where it would go over full level (off, it
    //    passes the audio unchanged), then the clamp makes sure nothing outside of -1.0 .. 1.0 and no NaN leaves the
    //    mixer, as a safety net that counts what it had to cut off
    Float32* master = &m_buses[ 0 ]->m_buffer[ 0 ];
    m_limiter.Process( master, frames );
    for ( size_t i=0; i<samples; ++i )
    {
        Float32 sample = master[ i ];
        if ( sample > 1.0f )
        {
            sample = 1.0f;
            ++m_clipped;
        }
        else if ( sample < -1.0f )
        {
            sample = -1.0f;
            ++m_clipped;
        }
        else if ( sample != sample )
            sample = 0.0f;
        output[ i ] = sample;
    }
    m_totalFrames += frames;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
