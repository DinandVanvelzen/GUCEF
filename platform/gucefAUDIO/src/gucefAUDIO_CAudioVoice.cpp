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
#include <string.h>

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_AUDIO_CAUDIOCONVERTER_H
#include "gucefAUDIO_CAudioConverter.h"
#define GUCEF_AUDIO_CAUDIOCONVERTER_H
#endif /* GUCEF_AUDIO_CAUDIOCONVERTER_H ? */

#include "gucefAUDIO_CAudioVoice.h"

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

/** Frames read from a source in one go: fixed so the memory of a voice does not depend on how much it is asked for at once */
const UInt32 SourceChunkFrames = 2048;

const Float32 MinPlaybackRate = 0.05f;
const Float32 MaxPlaybackRate = 16.0f;

/** A pitch change glides: a one pole on log2 of the factor with this time constant, stepped once per sub-block */
const Float64 PitchGlideInSeconds = 0.020;
const UInt32 PitchGlideSubBlockFrames = 32;

/** A glide has arrived when it is this close to the target (log2 of the factor): about 0.00007 percent of the frequency */
const Float64 PitchArrivedLog2 = 1.0e-6;

const Float64 Log2OfE = 1.4426950408889634;
const Float64 LogOf2 = 0.69314718055994531;

/** An effect that can sound forever has rung out once the voice gave nothing above this level (-100 dBFS) for one second */
const Float32 RingOutSilenceLevel = 1.0e-5f;

Float64
Log2( Float64 value )
{
    return log( value ) * Log2OfE;
}

Int32
IndexOfSpeaker( UInt32 layout, UInt32 speaker )
{
    Int32 index = 0;
    for ( UInt32 bit=0; bit<32; ++bit )
    {
        const UInt32 mask = 1U << bit;
        if ( layout & mask )
        {
            if ( mask == speaker )
                return index;
            ++index;
        }
    }
    return -1;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CAudioVoice::SSend::SSend( void )
    : bus( GUCEF_NULL )
    , level( 1.0f )
    , lastLevel( 1.0f )
    , levelKnown( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAudioVoice::CAudioVoice( const TAudioSourcePtr& source )
    : m_source( source )
    , m_sourceRaw( GUCEF_NULL )
    , m_bus( GUCEF_NULL )
    , m_volume( 1.0f )
    , m_pan( 0.0f )
    , m_playbackRate( 1.0f )
    , m_loopCount( 1 )
    , m_state( AUDIOVOICESTATE_IDLE )
    , m_endReason( AUDIOVOICEEND_NONE )
    , m_position( 0 )
    , m_stopRequested( false )
    , m_pauseRequested( false )
    , m_pitch( 1.0f )
    , m_effectLock()
    , m_effectRefs()
    , m_maxEffects( DefaultMaxEffects )
    , m_sends( GUCEF_NULL )
    , m_maxSends( 0 )
    , m_sourceFormat()
    , m_mixFormat()
    , m_workFormat()
    , m_matrix()
    , m_rawBuffer()
    , m_rawFloatBuffer()
    , m_convertedBuffer()
    , m_blockBuffer()
    , m_effects()
    , m_hasEffects( false )
    , m_hasSends( false )
    , m_tailFrames( 0 )
    , m_tailLeft( 0 )
    , m_silentFrames( 0 )
    , m_silenceToEnd( 0 )
    , m_resampler()
    , m_resampling( false )
    , m_pitchTarget( 1.0f )
    , m_pitchTargetLog2( 0.0 )
    , m_pitchLog2( 0.0 )
    , m_pitchNow( 1.0 )
    , m_pitchCoefficient( 1.0 )
    , m_pitchFramesLeft( 0 )
    , m_maxBlock( 0 )
    , m_pendingInput( 0 )
    , m_playsDone( 0 )
    , m_framesThisPlay( 0 )
    , m_playedPosition( 0.0 )
    , m_lastGains()
    , m_gainsKnown( false )
    , m_prepared( false )
    , m_sourceEnded( false )
    , m_resamplerRatio( 0.0 )
{GUCEF_TRACE;

    m_sourceRaw = m_source.GetPointerAlways();
    m_effects.SetSlotCapacity( m_maxEffects );
    m_sends = GUCEF_NEW SSend[ DefaultMaxSends ];
    m_maxSends = DefaultMaxSends;
}

/*-------------------------------------------------------------------------*/

CAudioVoice::CAudioVoice( const TAudioSourcePtr& source, UInt32 maxEffects, UInt32 maxSends )
    : m_source( source )
    , m_sourceRaw( GUCEF_NULL )
    , m_bus( GUCEF_NULL )
    , m_volume( 1.0f )
    , m_pan( 0.0f )
    , m_playbackRate( 1.0f )
    , m_loopCount( 1 )
    , m_state( AUDIOVOICESTATE_IDLE )
    , m_endReason( AUDIOVOICEEND_NONE )
    , m_position( 0 )
    , m_stopRequested( false )
    , m_pauseRequested( false )
    , m_pitch( 1.0f )
    , m_effectLock()
    , m_effectRefs()
    , m_maxEffects( maxEffects )
    , m_sends( GUCEF_NULL )
    , m_maxSends( 0 )
    , m_sourceFormat()
    , m_mixFormat()
    , m_workFormat()
    , m_matrix()
    , m_rawBuffer()
    , m_rawFloatBuffer()
    , m_convertedBuffer()
    , m_blockBuffer()
    , m_effects()
    , m_hasEffects( false )
    , m_hasSends( false )
    , m_tailFrames( 0 )
    , m_tailLeft( 0 )
    , m_silentFrames( 0 )
    , m_silenceToEnd( 0 )
    , m_resampler()
    , m_resampling( false )
    , m_pitchTarget( 1.0f )
    , m_pitchTargetLog2( 0.0 )
    , m_pitchLog2( 0.0 )
    , m_pitchNow( 1.0 )
    , m_pitchCoefficient( 1.0 )
    , m_pitchFramesLeft( 0 )
    , m_maxBlock( 0 )
    , m_pendingInput( 0 )
    , m_playsDone( 0 )
    , m_framesThisPlay( 0 )
    , m_playedPosition( 0.0 )
    , m_lastGains()
    , m_gainsKnown( false )
    , m_prepared( false )
    , m_sourceEnded( false )
    , m_resamplerRatio( 0.0 )
{GUCEF_TRACE;

    m_sourceRaw = m_source.GetPointerAlways();
    m_effects.SetSlotCapacity( m_maxEffects );
    if ( maxSends > 0 )
    {
        m_sends = GUCEF_NEW SSend[ maxSends ];
        m_maxSends = maxSends;
    }
}

/*-------------------------------------------------------------------------*/

CAudioVoice::~CAudioVoice()
{GUCEF_TRACE;

    GUCEF_DELETE[] m_sends;
    m_sends = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

const TAudioSourcePtr&
CAudioVoice::GetSource( void ) const
{GUCEF_TRACE;

    return m_source;
}

/*-------------------------------------------------------------------------*/

void
CAudioVoice::SetBus( CAudioBus* bus )
{GUCEF_TRACE;

    m_bus = bus;
}

/*-------------------------------------------------------------------------*/

CAudioBus*
CAudioVoice::GetBus( void ) const
{GUCEF_TRACE;

    return m_bus;
}

/*-------------------------------------------------------------------------*/

void
CAudioVoice::SetVolume( Float32 volume )
{GUCEF_TRACE;

    m_volume.Store( volume < 0.0f ? 0.0f : volume );
}

/*-------------------------------------------------------------------------*/

Float32
CAudioVoice::GetVolume( void ) const
{GUCEF_TRACE;

    return m_volume.Load();
}

/*-------------------------------------------------------------------------*/

void
CAudioVoice::SetPan( Float32 pan )
{GUCEF_TRACE;

    m_pan.Store( pan < -1.0f ? -1.0f : ( pan > 1.0f ? 1.0f : pan ) );
}

/*-------------------------------------------------------------------------*/

Float32
CAudioVoice::GetPan( void ) const
{GUCEF_TRACE;

    return m_pan.Load();
}

/*-------------------------------------------------------------------------*/

void
CAudioVoice::SetPlaybackRate( Float32 rate )
{GUCEF_TRACE;

    m_playbackRate.Store( rate < MinPlaybackRate ? MinPlaybackRate : ( rate > MaxPlaybackRate ? MaxPlaybackRate : rate ) );
}

/*-------------------------------------------------------------------------*/

Float32
CAudioVoice::GetPlaybackRate( void ) const
{GUCEF_TRACE;

    return m_playbackRate.Load();
}

/*-------------------------------------------------------------------------*/

void
CAudioVoice::SetPitch( Float32 factor )
{GUCEF_TRACE;

    if ( !( factor >= MinPlaybackRate ) )           // NaN as well
        factor = MinPlaybackRate;
    m_pitch.Store( factor > MaxPlaybackRate ? MaxPlaybackRate : factor );
}

/*-------------------------------------------------------------------------*/

Float32
CAudioVoice::GetPitch( void ) const
{GUCEF_TRACE;

    return m_pitch.Load();
}

/*-------------------------------------------------------------------------*/

UInt32
CAudioVoice::GetMaxEffects( void ) const
{GUCEF_TRACE;

    return m_maxEffects;
}

/*-------------------------------------------------------------------------*/

UInt32
CAudioVoice::GetMaxSends( void ) const
{GUCEF_TRACE;

    return m_maxSends;
}

/*-------------------------------------------------------------------------*/

bool
CAudioVoice::AddEffect( const TAudioEffectPtr& effect )
{GUCEF_TRACE;

    if ( effect.IsNULL() || m_prepared || AUDIOVOICESTATE_IDLE != m_state.Load( MT::MEMORDER_ACQUIRE ) )
        return false;
    MT::CScopeMutex lock( m_effectLock );
    if ( m_effectRefs.size() >= m_maxEffects )
        return false;
    for ( size_t i=0; i<m_effectRefs.size(); ++i )
        if ( m_effectRefs[ i ] == effect )
            return false;
    m_effectRefs.push_back( effect );
    return true;
}

/*-------------------------------------------------------------------------*/

UInt32
CAudioVoice::GetEffectCount( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_effectLock );
    return (UInt32) m_effectRefs.size();
}

/*-------------------------------------------------------------------------*/

TAudioEffectPtr
CAudioVoice::GetEffect( UInt32 index ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_effectLock );
    return index < m_effectRefs.size() ? m_effectRefs[ index ] : TAudioEffectPtr();
}

/*-------------------------------------------------------------------------*/

CAudioEffect*
CAudioVoice::GetEffectPointer( UInt32 index ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_effectLock );
    if ( index >= m_effectRefs.size() )
        return GUCEF_NULL;
    TAudioEffectPtr effect( m_effectRefs[ index ] );
    return effect.GetPointerAlways();
}

/*-------------------------------------------------------------------------*/

void
CAudioVoice::SetEffectEnabled( UInt32 index, bool enabled )
{GUCEF_TRACE;

    m_effects.SetEnabled( index, enabled );
}

/*-------------------------------------------------------------------------*/

bool
CAudioVoice::IsEffectEnabled( UInt32 index ) const
{GUCEF_TRACE;

    return m_effects.IsEnabled( index );
}

/*-------------------------------------------------------------------------*/

void
CAudioVoice::SetEffectMix( UInt32 index, Float32 wet, Float32 dry )
{GUCEF_TRACE;

    m_effects.SetMix( index, wet, dry );
}

/*-------------------------------------------------------------------------*/

Float32
CAudioVoice::GetEffectWet( UInt32 index ) const
{GUCEF_TRACE;

    return m_effects.GetWet( index );
}

/*-------------------------------------------------------------------------*/

Float32
CAudioVoice::GetEffectDry( UInt32 index ) const
{GUCEF_TRACE;

    return m_effects.GetDry( index );
}

/*-------------------------------------------------------------------------*/

bool
CAudioVoice::SetSend( UInt32 index, CAudioBus* bus, Float32 level )
{GUCEF_TRACE;

    if ( index >= m_maxSends || m_prepared || AUDIOVOICESTATE_IDLE != m_state.Load( MT::MEMORDER_ACQUIRE ) )
        return false;
    m_sends[ index ].bus = bus;
    m_sends[ index ].level.Store( level > 0.0f ? level : 0.0f );
    m_sends[ index ].levelKnown = false;
    return true;
}

/*-------------------------------------------------------------------------*/

CAudioBus*
CAudioVoice::GetSendBus( UInt32 index ) const
{GUCEF_TRACE;

    return index < m_maxSends ? m_sends[ index ].bus : GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

void
CAudioVoice::SetSendLevel( UInt32 index, Float32 level )
{GUCEF_TRACE;

    if ( index < m_maxSends )
        m_sends[ index ].level.Store( level > 0.0f ? level : 0.0f );       // NaN goes to 0 as well
}

/*-------------------------------------------------------------------------*/

Float32
CAudioVoice::GetSendLevel( UInt32 index ) const
{GUCEF_TRACE;

    return index < m_maxSends ? m_sends[ index ].level.Load() : 0.0f;
}

/*-------------------------------------------------------------------------*/

void
CAudioVoice::SetLoopCount( UInt32 plays )
{GUCEF_TRACE;

    m_loopCount.Store( plays, MT::MEMORDER_RELAXED );
}

/*-------------------------------------------------------------------------*/

UInt32
CAudioVoice::GetLoopCount( void ) const
{GUCEF_TRACE;

    return m_loopCount.Load( MT::MEMORDER_RELAXED );
}

/*-------------------------------------------------------------------------*/

TAudioVoiceState
CAudioVoice::GetState( void ) const
{GUCEF_TRACE;

    return (TAudioVoiceState) m_state.Load( MT::MEMORDER_ACQUIRE );
}

/*-------------------------------------------------------------------------*/

TAudioVoiceEndReason
CAudioVoice::GetEndReason( void ) const
{GUCEF_TRACE;

    return (TAudioVoiceEndReason) m_endReason.Load( MT::MEMORDER_ACQUIRE );
}

/*-------------------------------------------------------------------------*/

UInt64
CAudioVoice::GetPositionInFrames( void ) const
{GUCEF_TRACE;

    return m_position.Load( MT::MEMORDER_RELAXED );
}

/*-------------------------------------------------------------------------*/

void
CAudioVoice::RequestStop( void )
{GUCEF_TRACE;

    m_stopRequested.Store( true, MT::MEMORDER_RELEASE );
}

/*-------------------------------------------------------------------------*/

void
CAudioVoice::RequestPause( bool paused )
{GUCEF_TRACE;

    m_pauseRequested.Store( paused, MT::MEMORDER_RELEASE );
}

/*-------------------------------------------------------------------------*/

bool
CAudioVoice::IsPauseRequested( void ) const
{GUCEF_TRACE;

    return m_pauseRequested.Load( MT::MEMORDER_ACQUIRE );
}

/*-------------------------------------------------------------------------*/

bool
CAudioVoice::IsStopRequested( void ) const
{GUCEF_TRACE;

    return m_stopRequested.Load( MT::MEMORDER_ACQUIRE );
}

/*-------------------------------------------------------------------------*/

bool
CAudioVoice::Prepare( const CAudioFormat& mixFormat, UInt32 maxBlockFrames )
{GUCEF_TRACE;

    m_prepared = false;
    if ( m_source.IsNULL() || !mixFormat.IsValid() || 0 == maxBlockFrames )
        return false;

    m_sourceFormat = m_source->GetFormat();
    if ( !m_sourceFormat.IsValid() )
        return false;

    // The working channels are those of the mix. A spatial voice (steps 13 to 15 of the design) will work in mono up to its
    // spatializer: the matrix, the converted and block buffers, the resampler and the effects follow m_workFormat
    m_mixFormat = mixFormat;
    m_workFormat = mixFormat;
    if ( !CAudioConverter::BuildChannelMatrix( m_sourceFormat, m_workFormat, m_matrix ) )
        return false;

    m_maxBlock = maxBlockFrames;
    const UInt32 workChannels = m_workFormat.GetChannelCount();
    m_rawBuffer.assign( (size_t) SourceChunkFrames * m_sourceFormat.GetBytesPerFrame(), 0 );
    m_rawFloatBuffer.assign( (size_t) SourceChunkFrames * m_sourceFormat.GetChannelCount(), 0.0f );
    m_convertedBuffer.assign( (size_t) SourceChunkFrames * workChannels, 0.0f );
    m_blockBuffer.assign( (size_t) maxBlockFrames * workChannels, 0.0f );
    m_lastGains.assign( mixFormat.GetChannelCount(), 1.0f );
    m_resampler.Configure( workChannels, (Float64) m_sourceFormat.GetSampleRate(), (Float64) mixFormat.GetSampleRate() );
    m_resampler.Reset();
    m_resampling = false;
    m_resamplerRatio = 0.0;
    m_pendingInput = 0;
    m_playsDone = 0;
    m_framesThisPlay = 0;
    m_playedPosition = 0.0;
    m_sourceEnded = false;
    m_gainsKnown = false;

    // the pitch asked for before the start is there at once, later changes glide
    m_pitchTarget = m_pitch.Load();
    m_pitchTargetLog2 = Log2( (Float64) m_pitchTarget );
    m_pitchLog2 = m_pitchTargetLog2;
    m_pitchNow = (Float64) m_pitchTarget;
    m_pitchCoefficient = 1.0 - exp( -(Float64) PitchGlideSubBlockFrames / ( PitchGlideInSeconds * (Float64) mixFormat.GetSampleRate() ) );
    m_pitchFramesLeft = 0;

    if ( !PrepareEffects( maxBlockFrames ) || !CheckSends( maxBlockFrames ) )
        return false;
    m_prepared = true;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAudioVoice::PrepareEffects( UInt32 maxBlockFrames )
{GUCEF_TRACE;

    m_hasEffects = false;
    m_tailFrames = 0;
    m_tailLeft = 0;
    m_silentFrames = 0;
    m_silenceToEnd = m_mixFormat.GetSampleRate();
    MT::CScopeMutex lock( m_effectLock );
    const UInt32 count = (UInt32) m_effectRefs.size();
    if ( 0 == count )
        return true;

    // Effects work in the channels of the voice, prepared here unless they are already (CAudioEngine::CreateEffect() makes
    // them for the mix format). Preparing clears their state, which is what a device with another format needs as well
    UInt64 ringOut = 0;
    bool unbounded = false;
    for ( UInt32 i=0; i<count; ++i )
    {
        CAudioEffect* effect = m_effectRefs[ i ].GetPointerAlways();
        if ( !effect->IsPreparedFor( m_workFormat, maxBlockFrames ) && !effect->Prepare( m_workFormat, maxBlockFrames ) )
            return false;
        const UInt32 tail = effect->GetTailInFrames();
        if ( GUCEF_AUDIO_TAIL_INFINITE == tail )
            unbounded = true;
        else
            ringOut += (UInt64) effect->GetLatencyInFrames() + tail;
    }
    if ( !m_effects.Prepare( m_maxEffects, m_workFormat, maxBlockFrames ) )        // the slots and their settings stay
        return false;
    for ( UInt32 i=0; i<count; ++i )
        m_effects.SetSlotEffect( i, m_effectRefs[ i ].GetPointerAlways() );

    if ( unbounded || ringOut >= (UInt64) GUCEF_AUDIO_TAIL_INFINITE )
        m_tailFrames = GUCEF_AUDIO_TAIL_INFINITE;
    else
        m_tailFrames = (UInt32) ringOut;
    m_tailLeft = m_tailFrames;
    m_hasEffects = true;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAudioVoice::CheckSends( UInt32 maxBlockFrames )
{GUCEF_TRACE;

    m_hasSends = false;
    const size_t busSamples = (size_t) maxBlockFrames * m_mixFormat.GetChannelCount();
    for ( UInt32 s=0; s<m_maxSends; ++s )
    {
        SSend& send = m_sends[ s ];
        send.levelKnown = false;
        if ( GUCEF_NULL == send.bus )
            continue;
        if ( send.bus->m_buffer.size() < busSamples )
            return false;                               // not a bus of the mixer: it was never prepared for the mix format
        m_hasSends = true;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
CAudioVoice::ReleaseEffects( void )
{GUCEF_TRACE;

    for ( UInt32 i=0; i<m_effects.GetSlotCapacity(); ++i )
        m_effects.SetSlotEffect( i, GUCEF_NULL );
    m_hasEffects = false;

    // the last references may destroy effects: that happens here, without the lock
    GUCEF::vector< TAudioEffectPtr > released;
    {
        MT::CScopeMutex lock( m_effectLock );
        released.swap( m_effectRefs );
    }
    released.clear();
}

/*-------------------------------------------------------------------------*/

bool
CAudioVoice::Reprepare( const CAudioFormat& mixFormat, UInt32 maxBlockFrames )
{GUCEF_TRACE;

    if ( !m_prepared )
        return Prepare( mixFormat, maxBlockFrames );
    if ( !mixFormat.IsValid() || 0 == maxBlockFrames )
        return false;

    // Source audio that was read and converted but not yet mixed goes along to the new speakers
    const CAudioFormat newWorkFormat( mixFormat );
    const UInt32 oldChannels = m_workFormat.GetChannelCount();
    const UInt32 newChannels = newWorkFormat.GetChannelCount();
    GUCEF::vector< Float32 > pending;
    if ( m_pendingInput > 0 )
    {
        pending.assign( (size_t) m_pendingInput * newChannels, 0.0f );
        if ( oldChannels == newChannels && m_workFormat.GetChannelLayout() == newWorkFormat.GetChannelLayout() )
            memcpy( &pending[ 0 ], &m_convertedBuffer[ 0 ], pending.size() * sizeof( Float32 ) );
        else
        {
            GUCEF::vector< Float32 > matrix;
            if ( CAudioConverter::BuildChannelMatrix( m_workFormat, newWorkFormat, matrix ) )
                CAudioConverter::RemixChannels( &m_convertedBuffer[ 0 ], oldChannels, &pending[ 0 ], newChannels, m_pendingInput, matrix );
        }
    }

    if ( !CAudioConverter::BuildChannelMatrix( m_sourceFormat, newWorkFormat, m_matrix ) )
    {
        m_prepared = false;
        return false;
    }

    m_mixFormat = mixFormat;
    m_workFormat = newWorkFormat;
    m_maxBlock = maxBlockFrames;
    m_rawFloatBuffer.assign( (size_t) SourceChunkFrames * m_sourceFormat.GetChannelCount(), 0.0f );
    m_convertedBuffer.assign( (size_t) SourceChunkFrames * newChannels, 0.0f );
    m_blockBuffer.assign( (size_t) maxBlockFrames * newChannels, 0.0f );
    m_lastGains.assign( mixFormat.GetChannelCount(), 1.0f );
    m_gainsKnown = false;
    m_resampler.Configure( newChannels, (Float64) m_sourceFormat.GetSampleRate(), (Float64) mixFormat.GetSampleRate() );
    m_resamplerRatio = 0.0;
    m_pitchCoefficient = 1.0 - exp( -(Float64) PitchGlideSubBlockFrames / ( PitchGlideInSeconds * (Float64) mixFormat.GetSampleRate() ) );
    if ( !pending.empty() )
        memcpy( &m_convertedBuffer[ 0 ], &pending[ 0 ], pending.size() * sizeof( Float32 ) );

    // the effects work in the new format from the start again; a tail that was ringing out is counted afresh
    if ( !PrepareEffects( maxBlockFrames ) || !CheckSends( maxBlockFrames ) )
    {
        m_prepared = false;
        return false;
    }

    // repeats, position, pitch and end of the source stay as they are
    return true;
}

/*-------------------------------------------------------------------------*/

void
CAudioVoice::End( TAudioVoiceEndReason reason )
{
    m_endReason.Store( (UInt32) reason, MT::MEMORDER_RELEASE );
    m_state.Store( AUDIOVOICESTATE_ENDED, MT::MEMORDER_RELEASE );
}

/*-------------------------------------------------------------------------*/

UInt32
CAudioVoice::ReadAndConvert( UInt32 frames, bool sameChannels )
{
    if ( frames > SourceChunkFrames )
        frames = SourceChunkFrames;
    const UInt32 got = m_sourceRaw->ReadFrames( &m_rawBuffer[ 0 ], frames );
    if ( 0 == got )
        return 0;

    const UInt32 sourceChannels = m_sourceFormat.GetChannelCount();
    const UInt32 workChannels = m_workFormat.GetChannelCount();
    if ( sameChannels )
    {
        CAudioConverter::ConvertToFloat32( &m_rawBuffer[ 0 ], m_sourceFormat.GetSampleType(), &m_convertedBuffer[ 0 ], (UInt64) got * sourceChannels );
    }
    else
    {
        CAudioConverter::ConvertToFloat32( &m_rawBuffer[ 0 ], m_sourceFormat.GetSampleType(), &m_rawFloatBuffer[ 0 ], (UInt64) got * sourceChannels );
        CAudioConverter::RemixChannels( &m_rawFloatBuffer[ 0 ], sourceChannels, &m_convertedBuffer[ 0 ], workChannels, got, m_matrix );
    }
    return got;
}

/*-------------------------------------------------------------------------*/

bool
CAudioVoice::HandleSourceEnd( void )
{
    if ( !m_sourceRaw->IsEndOfStream() )
        return false;                       // nothing right now but not over (a live source): the block stays short

    ++m_playsDone;
    const UInt32 plays = m_loopCount.Load( MT::MEMORDER_RELAXED );
    const bool again = ( 0 == plays || m_playsDone < plays ) && m_framesThisPlay > 0 && m_sourceRaw->IsSeekable();
    if ( again && m_sourceRaw->SeekToFrame( 0 ) )
    {
        m_framesThisPlay = 0;
        m_playedPosition = 0.0;
        return true;
    }
    m_sourceEnded = true;
    return false;
}

/*-------------------------------------------------------------------------*/

void
CAudioVoice::ApplyResamplerRate( Float64 playbackRate )
{
    Float64 speed = playbackRate * m_pitchNow;
    if ( speed < (Float64) MinPlaybackRate )
        speed = (Float64) MinPlaybackRate;
    else if ( speed > (Float64) MaxPlaybackRate )
        speed = (Float64) MaxPlaybackRate;

    const Float64 inputRate = (Float64) m_sourceFormat.GetSampleRate() * speed;
    if ( inputRate != m_resamplerRatio )
    {
        m_resampler.SetRates( inputRate, (Float64) m_mixFormat.GetSampleRate() );
        m_resamplerRatio = inputRate;
    }
}

/*-------------------------------------------------------------------------*/

bool
CAudioVoice::StepPitchGlide( Float64 playbackRate )
{
    m_pitchLog2 += ( m_pitchTargetLog2 - m_pitchLog2 ) * m_pitchCoefficient;
    const bool arrived = fabs( m_pitchTargetLog2 - m_pitchLog2 ) < PitchArrivedLog2;
    if ( arrived )
    {
        m_pitchLog2 = m_pitchTargetLog2;
        m_pitchNow = (Float64) m_pitchTarget;           // exactly what was asked for, so a pitch of 1 is no change at all
    }
    else
        m_pitchNow = exp( m_pitchLog2 * LogOf2 );
    ApplyResamplerRate( playbackRate );
    return !arrived;
}

/*-------------------------------------------------------------------------*/

UInt32
CAudioVoice::GatherBlock( UInt32 frames )
{
    const UInt32 workChannels = m_workFormat.GetChannelCount();
    const UInt32 sourceChannels = m_sourceFormat.GetChannelCount();
    const bool sameChannels = ( sourceChannels == workChannels && m_sourceFormat.GetChannelLayout() == m_workFormat.GetChannelLayout() );

    // Speed: how many source frames pass for each frame that is mixed, the playback rate times the pitch. The playback rate
    // applies at once, the pitch glides to where it was asked to be, in steps of a sub-block. (The Doppler ratio of a spatial
    // voice is the third factor, steps 13 to 15 of the design: it glides the same way)
    const Float64 rate = (Float64) m_playbackRate.Load();
    const Float32 pitch = m_pitch.Load();
    if ( pitch != m_pitchTarget )
    {
        m_pitchTarget = pitch;
        m_pitchTargetLog2 = Log2( (Float64) pitch );
    }
    bool gliding = ( m_pitchLog2 != m_pitchTargetLog2 );

    // Once the resampler is in use it stays in use: it holds source audio that was read ahead and a position between two
    // source frames, so going back to reading the source directly would skip audio
    if ( !m_resampling && ( gliding || rate * m_pitchNow != 1.0 || m_sourceFormat.GetSampleRate() != m_mixFormat.GetSampleRate() ) )
        m_resampling = true;
    if ( m_resampling )
        ApplyResamplerRate( rate );

    UInt32 produced = 0;
    Float32* out = &m_blockBuffer[ 0 ];
    UInt32 stalled = 0;

    while ( produced < frames && !m_sourceEnded )
    {
        if ( m_resampling )
        {
            if ( 0 == m_pendingInput )
            {
                m_pendingInput = ReadAndConvert( SourceChunkFrames, sameChannels );
                m_framesThisPlay += m_pendingInput;
            }
            if ( 0 == m_pendingInput )
            {
                if ( !HandleSourceEnd() )
                    break;
                continue;
            }

            // while the pitch glides the rate changes at the start of every sub-block
            UInt32 room = frames - produced;
            if ( gliding )
            {
                if ( 0 == m_pitchFramesLeft )
                {
                    gliding = StepPitchGlide( rate );
                    m_pitchFramesLeft = gliding ? PitchGlideSubBlockFrames : 0;
                }
                if ( gliding && room > m_pitchFramesLeft )
                    room = m_pitchFramesLeft;
            }

            UInt32 consumed = 0, made = 0;
            m_resampler.Process( &m_convertedBuffer[ 0 ], m_pendingInput, consumed, out + (size_t) produced * workChannels, room, made );
            produced += made;
            if ( gliding )
                m_pitchFramesLeft -= made;
            m_playedPosition += (Float64) made * ( m_resamplerRatio / (Float64) m_mixFormat.GetSampleRate() );   // source frames passed
            if ( consumed > 0 )
                m_pendingInput = 0;
            else if ( 0 == made && ++stalled > 2 )
                break;                      // the resampler cannot go on with this, do not spin
        }
        else
        {
            const UInt32 want = ( frames - produced < SourceChunkFrames ) ? ( frames - produced ) : SourceChunkFrames;
            const UInt32 got = ReadAndConvert( want, sameChannels );
            if ( 0 == got )
            {
                if ( !HandleSourceEnd() )
                    break;
                continue;
            }
            m_framesThisPlay += got;
            m_playedPosition += (Float64) got;
            memcpy( out + (size_t) produced * workChannels, &m_convertedBuffer[ 0 ], (size_t) got * workChannels * sizeof( Float32 ) );
            produced += got;
        }
    }
    return produced;
}

/*-------------------------------------------------------------------------*/

bool
CAudioVoice::IsRingOutOver( UInt32 produced, UInt32 frames )
{
    // the frames of this block after the end of the source are ring out
    const UInt32 ringing = frames - produced;
    if ( GUCEF_AUDIO_TAIL_INFINITE != m_tailFrames )
    {
        if ( m_tailLeft <= ringing )
        {
            m_tailLeft = 0;
            return true;
        }
        m_tailLeft -= ringing;
        return false;
    }

    // an effect that can sound forever: over once a second has passed without anything audible
    const size_t samples = (size_t) frames * m_workFormat.GetChannelCount();
    const Float32* block = &m_blockBuffer[ 0 ];
    for ( size_t i=0; i<samples; ++i )
    {
        if ( block[ i ] > RingOutSilenceLevel || block[ i ] < -RingOutSilenceLevel )
        {
            m_silentFrames = 0;
            return false;
        }
    }
    m_silentFrames += ringing;
    return m_silentFrames >= m_silenceToEnd;
}

/*-------------------------------------------------------------------------*/

void
CAudioVoice::DeliverSends( const Float32* block, UInt32 frames, UInt32 channels )
{
    for ( UInt32 s=0; s<m_maxSends; ++s )
    {
        SSend& send = m_sends[ s ];
        if ( GUCEF_NULL == send.bus )
            continue;

        const Float32 target = send.level.Load();
        const Float32 from = send.levelKnown ? send.lastLevel : target;
        send.lastLevel = target;
        send.levelKnown = true;

        Float32* into = &send.bus->m_buffer[ 0 ];
        if ( from == target )
        {
            if ( 0.0f == target )
                continue;
            const size_t samples = (size_t) frames * channels;
            for ( size_t i=0; i<samples; ++i )
                into[ i ] += block[ i ] * target;
        }
        else
        {
            const Float32 step = ( target - from ) / (Float32) frames;
            for ( UInt32 f=0; f<frames; ++f )
            {
                const Float32 g = from + step * (Float32) ( f + 1 );
                for ( UInt32 c=0; c<channels; ++c )
                    into[ (size_t) f * channels + c ] += block[ (size_t) f * channels + c ] * g;
            }
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CAudioVoice::RenderOutput( Float32* busBuffer, UInt32 frames, TAudioPanLaw panLaw )
{
    const UInt32 mixChannels = m_mixFormat.GetChannelCount();

    // The gain of every channel: volume, and for the front speakers the position between them
    const UInt32 gainCount = (UInt32) m_lastGains.size();
    Float32 gains[ 64 ];
    const UInt32 limited = ( gainCount < 64 ) ? gainCount : 64;
    const Float32 volume = m_volume.Load();
    for ( UInt32 c=0; c<limited; ++c )
        gains[ c ] = volume;
    const UInt32 layout = m_mixFormat.GetChannelLayout();
    const Int32 left = IndexOfSpeaker( layout, AUDIOSPEAKER_FRONT_LEFT );
    const Int32 right = IndexOfSpeaker( layout, AUDIOSPEAKER_FRONT_RIGHT );
    const Float32 pan = m_pan.Load();
    if ( left >= 0 && right >= 0 && (UInt32) left < limited && (UInt32) right < limited && 0.0f != pan )
    {
        Float32 gl = 1.0f, gr = 1.0f;
        if ( AUDIOPANLAW_CONSTANT_POWER == panLaw && 1 == m_sourceFormat.GetChannelCount() )
        {
            const Float32 angle = ( pan + 1.0f ) * 0.78539816339f;       // pi / 4, so 0 .. pi / 2 across the range
            gl = 1.41421356f * cosf( angle );
            gr = 1.41421356f * sinf( angle );
        }
        else
        {
            if ( pan > 0.0f ) gl = 1.0f - pan;
            if ( pan < 0.0f ) gr = 1.0f + pan;
        }
        gains[ left ] *= gl;
        gains[ right ] *= gr;
    }

    // Changes in gain are spread over the block, so they do not click
    if ( !m_gainsKnown )
    {
        for ( UInt32 c=0; c<limited; ++c )
            m_lastGains[ c ] = gains[ c ];
        m_gainsKnown = true;
    }
    const Float32 inverseFrames = 1.0f / (Float32) frames;
    if ( !m_hasSends )
    {
        const Float32* block = &m_blockBuffer[ 0 ];
        for ( UInt32 f=0; f<frames; ++f )
        {
            const Float32 t = (Float32) ( f + 1 ) * inverseFrames;
            for ( UInt32 c=0; c<mixChannels; ++c )
            {
                const Float32 from = ( c < limited ) ? m_lastGains[ c ] : volume;
                const Float32 to = ( c < limited ) ? gains[ c ] : volume;
                busBuffer[ (size_t) f * mixChannels + c ] += block[ (size_t) f * mixChannels + c ] * ( from + ( to - from ) * t );
            }
        }
    }
    else
    {
        // The gains go into the block itself, which then goes to the bus and, at their levels, to the sends: what a send
        // takes is the voice after its volume and pan
        Float32* block = &m_blockBuffer[ 0 ];
        for ( UInt32 f=0; f<frames; ++f )
        {
            const Float32 t = (Float32) ( f + 1 ) * inverseFrames;
            for ( UInt32 c=0; c<mixChannels; ++c )
            {
                const Float32 from = ( c < limited ) ? m_lastGains[ c ] : volume;
                const Float32 to = ( c < limited ) ? gains[ c ] : volume;
                block[ (size_t) f * mixChannels + c ] *= ( from + ( to - from ) * t );
            }
        }
        const size_t samples = (size_t) frames * mixChannels;
        for ( size_t i=0; i<samples; ++i )
            busBuffer[ i ] += block[ i ];
        DeliverSends( block, frames, mixChannels );
    }
    for ( UInt32 c=0; c<limited; ++c )
        m_lastGains[ c ] = gains[ c ];
}

/*-------------------------------------------------------------------------*/

void
CAudioVoice::RenderBlock( Float32* busBuffer, UInt32 frames, TAudioPanLaw panLaw )
{
    if ( !m_prepared || frames > m_maxBlock || 0 == frames )
    {
        End( AUDIOVOICEEND_FAILED );
        return;
    }

    if ( m_stopRequested.Load( MT::MEMORDER_ACQUIRE ) )
    {
        End( AUDIOVOICEEND_STOPPED );
        return;
    }

    // Pausing holds everything where it is: nothing is read, nothing is heard
    if ( m_pauseRequested.Load( MT::MEMORDER_ACQUIRE ) )
    {
        m_state.Store( AUDIOVOICESTATE_PAUSED, MT::MEMORDER_RELEASE );
        return;
    }
    if ( AUDIOVOICESTATE_PAUSED == m_state.Load( MT::MEMORDER_RELAXED ) )
        m_state.Store( AUDIOVOICESTATE_PLAYING, MT::MEMORDER_RELEASE );

    // 1. and 2. the source, in the working channels at the mix rate (silence once it is over)
    const UInt32 workChannels = m_workFormat.GetChannelCount();
    const UInt32 produced = GatherBlock( frames );
    if ( produced < frames )
        memset( &m_blockBuffer[ (size_t) produced * workChannels ], 0, (size_t) ( frames - produced ) * workChannels * sizeof( Float32 ) );
    m_position.Store( (UInt64) m_playedPosition, MT::MEMORDER_RELAXED );

    // 3. the voice's own effects. After the end of the source they ring out on silence
    if ( m_hasEffects )
        m_effects.Process( &m_blockBuffer[ 0 ], frames );
    const bool over = m_sourceEnded && ( !m_hasEffects || IsRingOutOver( produced, frames ) );

    // 4. and 5. the output stage, into the bus and the sends. For a voice with an emitter the spatial stage (steps 13 to 15
    //    of the design) takes the place of RenderOutput() here: mono working signal in, the speakers of the mix out
    RenderOutput( busBuffer, frames, panLaw );

    if ( over )
        End( AUDIOVOICEEND_FINISHED );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
