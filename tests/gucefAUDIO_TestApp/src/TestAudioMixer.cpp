/*
 *  gucefAUDIO_TestApp: GUCEF test application for the AUDIO module
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <math.h>
#include <string.h>

#ifndef GUCEFAUDIO_H
#include "gucefAUDIO.h"
#define GUCEFAUDIO_H
#endif /* GUCEFAUDIO_H ? */

#ifndef GUCEF_AUDIO_CAUDIOMIXER_H
#include "gucefAUDIO_CAudioMixer.h"
#define GUCEF_AUDIO_CAUDIOMIXER_H
#endif /* GUCEF_AUDIO_CAUDIOMIXER_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestAudioMixer.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

const AUDIO::CAudioFormat MixFormat( 48000, 2, AUDIO::SAMPLETYPE_FLOAT32 );

bool
Near( Float32 a, Float32 b, Float32 tolerance = 1e-5f )
{
    return fabsf( a - b ) <= tolerance;
}

/** A clip whose frames all have the same value, or run from 'first' to 'last' as a ramp */
AUDIO::TAudioClipPtr
MakeClip( UInt32 rate, UInt32 channels, UInt32 frames, Float32 first, Float32 last )
{
    AUDIO::CAudioBuffer audio( AUDIO::CAudioFormat( rate, channels, AUDIO::SAMPLETYPE_FLOAT32 ), frames );
    for ( UInt32 f=0; f<frames; ++f )
        for ( UInt32 c=0; c<channels; ++c )
            audio.SetSampleFromFloat( f, c, frames > 1 ? first + ( last - first ) * (Float32) f / (Float32) ( frames - 1 ) : first );
    return AUDIO::TAudioClipPtr( GUCEF_NEW AUDIO::CAudioClip( audio ) );
}

AUDIO::TAudioClipPtr
MakeConstant( UInt32 rate, UInt32 channels, UInt32 frames, Float32 value )
{
    return MakeClip( rate, channels, frames, value, value );
}

/** Voice that is ready to be mixed */
AUDIO::TAudioVoicePtr
MakeVoice( AUDIO::CAudioMixer& mixer, const AUDIO::TAudioClipPtr& clip, AUDIO::CAudioBus* bus = GUCEF_NULL )
{
    AUDIO::TAudioVoicePtr voice( GUCEF_NEW AUDIO::CAudioVoice( clip->CreateSource() ) );
    if ( GUCEF_NULL != bus )
        voice->SetBus( bus );
    if ( !mixer.PrepareVoice( voice.GetPointerAlways() ) )
        return AUDIO::TAudioVoicePtr();
    return voice;
}

/** What the mixer produced for a number of frames, as stereo floats */
GUCEF::vector< Float32 >
MixFrames( AUDIO::CAudioMixer& mixer, UInt32 frames )
{
    GUCEF::vector< Float32 > out( (size_t) frames * mixer.GetMixFormat().GetChannelCount(), 123.0f );
    mixer.Mix( &out[ 0 ], frames );
    return out;
}

class CEndedRecorder : public AUDIO::IAudioMixerListener
{
    public:

    virtual void OnVoiceEnded( AUDIO::CAudioVoice* voice, AUDIO::TAudioVoiceEndReason reason ) GUCEF_VIRTUAL_OVERRIDE
    {
        voices.push_back( voice );
        reasons.push_back( reason );
    }

    GUCEF::vector< AUDIO::CAudioVoice* > voices;
    GUCEF::vector< AUDIO::TAudioVoiceEndReason > reasons;
};

/** A source that has nothing right now but is not over: a live feed that is behind */
class CStarvedSource : public AUDIO::IAudioSource
{
    public:

    CStarvedSource( void ) : m_format( 48000, 1, AUDIO::SAMPLETYPE_FLOAT32 ), available( 0 ), value( 0.25f ) {}
    virtual const AUDIO::CAudioFormat& GetFormat( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_format; }
    virtual UInt32 ReadFrames( void* destination, UInt32 frameCapacity ) GUCEF_VIRTUAL_OVERRIDE
    {
        const UInt32 n = ( available < frameCapacity ) ? available : frameCapacity;
        Float32* out = static_cast< Float32* >( destination );
        for ( UInt32 i=0; i<n; ++i )
            out[ i ] = value;
        available -= n;
        return n;
    }
    virtual bool IsEndOfStream( void ) const GUCEF_VIRTUAL_OVERRIDE { return false; }
    virtual UInt64 GetPositionInFrames( void ) const GUCEF_VIRTUAL_OVERRIDE { return 0; }
    AUDIO::CAudioFormat m_format;
    UInt32 available;
    Float32 value;
};

/** A source with broken audio in it */
class CNotANumberSource : public AUDIO::IAudioSource
{
    public:

    CNotANumberSource( void ) : m_format( 48000, 1, AUDIO::SAMPLETYPE_FLOAT32 ), m_left( 200 ) {}
    virtual const AUDIO::CAudioFormat& GetFormat( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_format; }
    virtual UInt32 ReadFrames( void* destination, UInt32 frameCapacity ) GUCEF_VIRTUAL_OVERRIDE
    {
        const UInt32 n = ( m_left < frameCapacity ) ? m_left : frameCapacity;
        Float32* out = static_cast< Float32* >( destination );
        const Float32 zero = 0.0f;
        for ( UInt32 i=0; i<n; ++i )
            out[ i ] = zero / zero;
        m_left -= n;
        return n;
    }
    virtual bool IsEndOfStream( void ) const GUCEF_VIRTUAL_OVERRIDE { return 0 == m_left; }
    virtual UInt64 GetPositionInFrames( void ) const GUCEF_VIRTUAL_OVERRIDE { return 200 - m_left; }
    AUDIO::CAudioFormat m_format;
    UInt32 m_left;
};

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

static void
PerformBasicMixingTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "AudioMixer: Without voices the output is silence, a voice plays at its own level and ends" )
    try
    {
        AUDIO::CAudioMixer mixer( MixFormat, 256, 8, 4 );
        ASSERT_TRUE( MixFormat == mixer.GetMixFormat() );
        GUCEF::vector< Float32 > silent = MixFrames( mixer, 300 );
        for ( size_t i=0; i<silent.size(); ++i )
            ASSERT_TRUE( 0.0f == silent[ i ] );

        CEndedRecorder recorder;
        mixer.SetListener( &recorder );
        AUDIO::TAudioVoicePtr voice = MakeVoice( mixer, MakeConstant( 48000, 1, 500, 0.4f ) );
        ASSERT_FALSE( voice.IsNULL() );
        ASSERT_TRUE( AUDIO::AUDIOVOICESTATE_IDLE == voice->GetState() );
        ASSERT_TRUE( mixer.AddVoice( voice.GetPointerAlways() ) );
        ASSERT_TRUE( AUDIO::AUDIOVOICESTATE_PLAYING == voice->GetState() && 1 == mixer.GetVoiceCount() );

        GUCEF::vector< Float32 > out = MixFrames( mixer, 300 );
        ASSERT_TRUE( Near( 0.4f, out[ 0 ] ) && Near( 0.4f, out[ 1 ] ) && Near( 0.4f, out[ 299 * 2 ] ) && Near( 0.4f, out[ 299 * 2 + 1 ] ) );
        ASSERT_TRUE( 300 == voice->GetPositionInFrames() );
        ASSERT_TRUE( recorder.voices.empty() );

        out = MixFrames( mixer, 300 );
        ASSERT_TRUE( Near( 0.4f, out[ 199 * 2 ] ) && 0.0f == out[ 200 * 2 ] && 0.0f == out[ 299 * 2 + 1 ] );    // 200 frames were left, then silence
        ASSERT_TRUE( AUDIO::AUDIOVOICESTATE_ENDED == voice->GetState() && AUDIO::AUDIOVOICEEND_FINISHED == voice->GetEndReason() );
        ASSERT_TRUE( 0 == mixer.GetVoiceCount() );
        ASSERT_TRUE( 1 == recorder.voices.size() && recorder.voices[ 0 ] == voice.GetPointerAlways() && AUDIO::AUDIOVOICEEND_FINISHED == recorder.reasons[ 0 ] );
        ASSERT_TRUE( 600 == mixer.GetTotalFramesMixed() || 900 == mixer.GetTotalFramesMixed() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioMixer: Voices add up, loud mixes are limited and counted, not wrapped" )
    try
    {
        AUDIO::CAudioMixer mixer( MixFormat, 256, 8, 4 );
        AUDIO::TAudioVoicePtr a = MakeVoice( mixer, MakeConstant( 48000, 1, 1000, 0.3f ) );
        AUDIO::TAudioVoicePtr b = MakeVoice( mixer, MakeConstant( 48000, 1, 1000, 0.2f ) );
        ASSERT_TRUE( mixer.AddVoice( a.GetPointerAlways() ) && mixer.AddVoice( b.GetPointerAlways() ) );
        GUCEF::vector< Float32 > out = MixFrames( mixer, 100 );
        ASSERT_TRUE( Near( 0.5f, out[ 0 ] ) && Near( 0.5f, out[ 199 ] ) );
        ASSERT_TRUE( 0 == mixer.GetClippedSampleCount() );

        AUDIO::TAudioVoicePtr loud1 = MakeVoice( mixer, MakeConstant( 48000, 1, 1000, 0.8f ) );
        AUDIO::TAudioVoicePtr loud2 = MakeVoice( mixer, MakeConstant( 48000, 1, 1000, 0.8f ) );
        ASSERT_TRUE( mixer.AddVoice( loud1.GetPointerAlways() ) && mixer.AddVoice( loud2.GetPointerAlways() ) );
        out = MixFrames( mixer, 50 );
        ASSERT_TRUE( 1.0f == out[ 0 ] && 1.0f == out[ 99 ] );
        ASSERT_TRUE( mixer.GetClippedSampleCount() >= 100 );

        // negative side too
        AUDIO::CAudioMixer other( MixFormat, 256, 8, 4 );
        AUDIO::TAudioVoicePtr n1 = MakeVoice( other, MakeConstant( 48000, 1, 100, -0.9f ) );
        AUDIO::TAudioVoicePtr n2 = MakeVoice( other, MakeConstant( 48000, 1, 100, -0.9f ) );
        ASSERT_TRUE( other.AddVoice( n1.GetPointerAlways() ) && other.AddVoice( n2.GetPointerAlways() ) );
        out = MixFrames( other, 10 );
        ASSERT_TRUE( -1.0f == out[ 0 ] );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioMixer: The limiter turns a loud moment down instead of cutting it off, and comes back gradually" )
    try
    {
        AUDIO::CAudioMixer mixer( MixFormat, 256, 8, 4 );
        ASSERT_FALSE( mixer.IsLimiterEnabled() );
        mixer.SetLimiterMode( AUDIO::AUDIOLIMITERMODE_INSTANT );
        ASSERT_TRUE( mixer.IsLimiterEnabled() );

        // music at half level, and a short loud effect on top of it
        AUDIO::TAudioVoicePtr music = MakeVoice( mixer, MakeConstant( 48000, 1, 48000, 0.5f ) );
        AUDIO::TAudioVoicePtr effect = MakeVoice( mixer, MakeConstant( 48000, 1, 480, 0.9f ) );
        ASSERT_TRUE( mixer.AddVoice( music.GetPointerAlways() ) && mixer.AddVoice( effect.GetPointerAlways() ) );

        GUCEF::vector< Float32 > out = MixFrames( mixer, 480 );
        Float32 peak = 0.0f;
        for ( size_t i=0; i<out.size(); ++i )
            peak = out[ i ] > peak ? out[ i ] : peak;
        ASSERT_TRUE( peak <= 0.98f + 1e-5f );                                   // never over full level ...
        ASSERT_TRUE( Near( 0.98f, out[ 0 ], 1e-4f ) && Near( 0.98f, out[ 959 ], 1e-4f ) );   // ... but no quieter than needed
        ASSERT_TRUE( 0 == mixer.GetClippedSampleCount() );
        ASSERT_TRUE( 480 == mixer.GetLimitedFrameCount() );

        // the effect is over: the music comes back up over tens of milliseconds, without a jump
        out = MixFrames( mixer, 14400 );
        ASSERT_TRUE( out[ 0 ] < 0.5f && out[ 0 ] > 0.68f * 0.5f );
        for ( size_t f=1; f<14400; ++f )
            ASSERT_TRUE( out[ f * 2 ] >= out[ ( f - 1 ) * 2 ] - 1e-6f );
        ASSERT_TRUE( Near( 0.5f, out[ 14399 * 2 ], 1e-3f ) );
        ASSERT_TRUE( 0 == mixer.GetClippedSampleCount() );

        // the engine has it on unless asked otherwise
        AUDIO::CAudioEngineParams params;
        ASSERT_TRUE( params.GetLimiterEnabled() );
        params.SetLimiterEnabled( false );
        AUDIO::CAudioEngineParams copy( params );
        ASSERT_FALSE( copy.GetLimiterEnabled() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioMixer: Broken audio (not a number) is silent, not poisoning the mix" )
    try
    {
        AUDIO::CAudioMixer mixer( MixFormat, 256, 8, 4 );
        AUDIO::TAudioVoicePtr broken( GUCEF_NEW AUDIO::CAudioVoice( AUDIO::TAudioSourcePtr( GUCEF_NEW CNotANumberSource() ) ) );
        AUDIO::TAudioVoicePtr fine = MakeVoice( mixer, MakeConstant( 48000, 1, 100, 0.25f ) );
        ASSERT_TRUE( mixer.PrepareVoice( broken.GetPointerAlways() ) && mixer.AddVoice( broken.GetPointerAlways() ) && mixer.AddVoice( fine.GetPointerAlways() ) );
        GUCEF::vector< Float32 > out = MixFrames( mixer, 100 );
        for ( size_t i=0; i<out.size(); ++i )
            ASSERT_TRUE( out[ i ] == out[ i ] );                    // never NaN
        ASSERT_TRUE( 0.0f == out[ 0 ] || Near( 0.25f, out[ 0 ] ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
PerformLevelTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "AudioMixer: Voice, bus and master volume multiply, mute silences, changes fade instead of clicking" )
    try
    {
        AUDIO::CAudioMixer mixer( MixFormat, 256, 8, 4 );
        AUDIO::CAudioBus* effects = mixer.CreateBus( "effects" );
        ASSERT_TRUE( GUCEF_NULL != effects && effects->GetParent() == mixer.GetMasterBus() );
        AUDIO::TAudioVoicePtr voice = MakeVoice( mixer, MakeConstant( 48000, 1, 100000, 1.0f ), effects );
        voice->SetVolume( 0.5f );
        effects->SetVolume( 0.5f );
        mixer.GetMasterBus()->SetVolume( 0.5f );
        ASSERT_TRUE( mixer.AddVoice( voice.GetPointerAlways() ) );
        GUCEF::vector< Float32 > out = MixFrames( mixer, 256 );
        ASSERT_TRUE( Near( 0.125f, out[ 0 ] ) && Near( 0.125f, out[ 511 ] ) );

        // an abrupt change in volume is a ramp over the next block
        voice->SetVolume( 1.0f );
        out = MixFrames( mixer, 256 );
        ASSERT_TRUE( out[ 0 ] > 0.125f && out[ 0 ] < 0.126f + 0.005f );                  // starts close to the old level
        ASSERT_TRUE( Near( 0.25f, out[ 255 * 2 ], 1e-4f ) );                               // ends at the new one
        for ( UInt32 f=1; f<256; ++f )
            ASSERT_TRUE( out[ f * 2 ] >= out[ ( f - 1 ) * 2 ] );                           // monotonic, no jump
        out = MixFrames( mixer, 256 );
        ASSERT_TRUE( Near( 0.25f, out[ 0 ] ) && Near( 0.25f, out[ 400 ] ) );

        effects->SetMuted( true );
        ASSERT_TRUE( effects->IsMuted() && 0.0f == effects->GetEffectiveVolume() );
        out = MixFrames( mixer, 256 );
        ASSERT_TRUE( 0.0f == out[ 255 * 2 ] );
        ASSERT_TRUE( out[ 0 ] > 0.0f );                                                      // the mute faded in too
        effects->SetMuted( false );
        out = MixFrames( mixer, 256 );
        ASSERT_TRUE( Near( 0.25f, out[ 255 * 2 ], 1e-4f ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioMixer: Bus tree: children feed parents, buses are found by name, limits and foreign parents are refused" )
    try
    {
        AUDIO::CAudioMixer mixer( MixFormat, 256, 8, 4 );
        ASSERT_TRUE( 1 == mixer.GetBusCount() && mixer.GetMasterBus()->GetName() == "master" && GUCEF_NULL == mixer.GetMasterBus()->GetParent() );
        AUDIO::CAudioBus* music = mixer.CreateBus( "music" );
        AUDIO::CAudioBus* effects = mixer.CreateBus( "effects" );
        AUDIO::CAudioBus* ui = mixer.CreateBus( "ui", effects );
        ASSERT_TRUE( GUCEF_NULL != music && GUCEF_NULL != effects && GUCEF_NULL != ui && ui->GetParent() == effects );
        ASSERT_TRUE( mixer.FindBus( "ui" ) == ui && mixer.FindBus( "music" ) == music && GUCEF_NULL == mixer.FindBus( "nope" ) );
        ASSERT_TRUE( GUCEF_NULL == mixer.CreateBus( "toomany" ) );                           // 4 buses is the limit
        AUDIO::CAudioMixer foreign( MixFormat, 256, 8, 4 );
        ASSERT_TRUE( GUCEF_NULL == mixer.CreateBus( "stranger", foreign.GetMasterBus() ) );

        AUDIO::TAudioVoicePtr song = MakeVoice( mixer, MakeConstant( 48000, 1, 100000, 0.2f ), music );
        AUDIO::TAudioVoicePtr click = MakeVoice( mixer, MakeConstant( 48000, 1, 100000, 0.1f ), ui );
        ASSERT_TRUE( mixer.AddVoice( song.GetPointerAlways() ) && mixer.AddVoice( click.GetPointerAlways() ) );
        GUCEF::vector< Float32 > out = MixFrames( mixer, 128 );
        ASSERT_TRUE( Near( 0.3f, out[ 0 ] ) );

        effects->SetVolume( 0.0f );                                                          // silences the ui voice through its parent
        MixFrames( mixer, 256 );
        out = MixFrames( mixer, 128 );
        ASSERT_TRUE( Near( 0.2f, out[ 0 ] ) );
        music->SetMuted( true );
        MixFrames( mixer, 256 );
        out = MixFrames( mixer, 128 );
        ASSERT_TRUE( 0.0f == out[ 0 ] );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioBus: Decibels and linear volume convert both ways" )
    try
    {
        ASSERT_TRUE( Near( 1.0f, AUDIO::CAudioBus::DecibelsToLinear( 0.0f ) ) );
        ASSERT_TRUE( Near( 0.5f, AUDIO::CAudioBus::DecibelsToLinear( -6.0206f ), 1e-4f ) );
        ASSERT_TRUE( Near( 2.0f, AUDIO::CAudioBus::DecibelsToLinear( 6.0206f ), 1e-3f ) );
        ASSERT_TRUE( Near( -20.0f, AUDIO::CAudioBus::LinearToDecibels( 0.1f ), 1e-3f ) );
        ASSERT_TRUE( AUDIO::CAudioBus::LinearToDecibels( 0.0f ) < -100.0f );

        AUDIO::CAudioBus bus( "test", GUCEF_NULL );
        bus.SetVolumeInDecibels( -12.0f );
        ASSERT_TRUE( Near( -12.0f, bus.GetVolumeInDecibels(), 1e-3f ) && Near( 0.2512f, bus.GetVolume(), 1e-3f ) );
        bus.SetVolume( -5.0f );
        ASSERT_TRUE( 0.0f == bus.GetVolume() );                                              // no negative volume
        ASSERT_TRUE( Near( 1.0f, AUDIO::CAudioBus( "x", GUCEF_NULL ).GetVolume() ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
PerformPanAndChannelTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "AudioMixer: Constant power panning of a mono sound, and the balance law" )
    try
    {
        for ( UInt32 law=0; law<2; ++law )
        {
            AUDIO::CAudioMixer mixer( MixFormat, 256, 8, 4 );
            mixer.SetPanLaw( 0 == law ? AUDIO::AUDIOPANLAW_CONSTANT_POWER : AUDIO::AUDIOPANLAW_BALANCE );
            AUDIO::TAudioVoicePtr voice = MakeVoice( mixer, MakeConstant( 48000, 1, 100000, 0.5f ) );
            ASSERT_TRUE( mixer.AddVoice( voice.GetPointerAlways() ) );
            const Float32 pans[ 5 ] = { 0.0f, -1.0f, 1.0f, -0.5f, 0.5f };
            for ( UInt32 p=0; p<5; ++p )
            {
                voice->SetPan( pans[ p ] );
                MixFrames( mixer, 256 );                                                    // let the change fade in
                const GUCEF::vector< Float32 > out = MixFrames( mixer, 64 );
                const Float32 left = out[ 20 * 2 ], right = out[ 20 * 2 + 1 ];
                if ( 0 == law )
                {
                    ASSERT_TRUE( Near( left * left + right * right, 0.5f, 1e-3f ) );        // the power stays the same wherever the sound is
                    if ( 0 == p ) ASSERT_TRUE( Near( 0.5f, left, 1e-4f ) && Near( 0.5f, right, 1e-4f ) );
                    if ( 1 == p ) ASSERT_TRUE( Near( 0.7071f, left, 1e-3f ) && Near( 0.0f, right, 1e-4f ) );
                    if ( 2 == p ) ASSERT_TRUE( Near( 0.0f, left, 1e-4f ) && Near( 0.7071f, right, 1e-3f ) );
                }
                else
                {
                    if ( 1 == p ) ASSERT_TRUE( Near( 0.5f, left, 1e-4f ) && Near( 0.0f, right, 1e-4f ) );
                    if ( 4 == p ) ASSERT_TRUE( Near( 0.25f, left, 1e-4f ) && Near( 0.5f, right, 1e-4f ) );
                    if ( 3 == p ) ASSERT_TRUE( Near( 0.5f, left, 1e-4f ) && Near( 0.25f, right, 1e-4f ) );
                }
            }
            voice->SetPan( 7.0f );
            ASSERT_TRUE( 1.0f == voice->GetPan() );                                          // limited to the range
        }

        // a stereo sound is balanced, never boosted
        AUDIO::CAudioMixer mixer( MixFormat, 256, 8, 4 );
        AUDIO::CAudioBuffer stereo( AUDIO::CAudioFormat( 48000, 2, AUDIO::SAMPLETYPE_FLOAT32 ), 100000 );
        for ( UInt32 f=0; f<100000; ++f )
        {
            stereo.SetSampleFromFloat( f, 0, 0.2f );
            stereo.SetSampleFromFloat( f, 1, 0.6f );
        }
        AUDIO::TAudioClipPtr clip( GUCEF_NEW AUDIO::CAudioClip( stereo ) );
        AUDIO::TAudioVoicePtr voice = MakeVoice( mixer, clip );
        voice->SetPan( -0.5f );
        ASSERT_TRUE( mixer.AddVoice( voice.GetPointerAlways() ) );
        const GUCEF::vector< Float32 > out = MixFrames( mixer, 64 );
        ASSERT_TRUE( Near( 0.2f, out[ 0 ], 1e-4f ) && Near( 0.3f, out[ 1 ], 1e-4f ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioMixer: Sources are remixed to the speakers of the output" )
    try
    {
        // stereo source to a mono output
        {
            AUDIO::CAudioMixer mono( AUDIO::CAudioFormat( 48000, 1, AUDIO::SAMPLETYPE_FLOAT32 ), 256, 4, 2 );
            AUDIO::CAudioBuffer stereo( AUDIO::CAudioFormat( 48000, 2, AUDIO::SAMPLETYPE_FLOAT32 ), 1000 );
            for ( UInt32 f=0; f<1000; ++f ) { stereo.SetSampleFromFloat( f, 0, 0.2f ); stereo.SetSampleFromFloat( f, 1, 0.6f ); }
            AUDIO::TAudioClipPtr clip( GUCEF_NEW AUDIO::CAudioClip( stereo ) );
            AUDIO::TAudioVoicePtr voice = MakeVoice( mono, clip );
            ASSERT_TRUE( mono.AddVoice( voice.GetPointerAlways() ) );
            GUCEF::vector< Float32 > out( 100 );
            mono.Mix( &out[ 0 ], 100 );
            ASSERT_TRUE( Near( 0.4f, out[ 0 ] ) && Near( 0.4f, out[ 99 ] ) );
        }

        // stereo source to a 5.1 output: the fronts get the sound, the rest stays silent
        {
            AUDIO::CAudioMixer surround( AUDIO::CAudioFormat( 48000, 6, AUDIO::SAMPLETYPE_FLOAT32 ), 256, 4, 2 );
            AUDIO::CAudioBuffer stereo( AUDIO::CAudioFormat( 48000, 2, AUDIO::SAMPLETYPE_FLOAT32 ), 1000 );
            for ( UInt32 f=0; f<1000; ++f ) { stereo.SetSampleFromFloat( f, 0, 0.2f ); stereo.SetSampleFromFloat( f, 1, 0.6f ); }
            AUDIO::TAudioClipPtr clip( GUCEF_NEW AUDIO::CAudioClip( stereo ) );
            AUDIO::TAudioVoicePtr voice = MakeVoice( surround, clip );
            ASSERT_TRUE( surround.AddVoice( voice.GetPointerAlways() ) );
            GUCEF::vector< Float32 > out( 100 * 6 );
            surround.Mix( &out[ 0 ], 100 );
            ASSERT_TRUE( Near( 0.2f, out[ 50 * 6 + 0 ] ) && Near( 0.6f, out[ 50 * 6 + 1 ] ) );
            for ( UInt32 c=2; c<6; ++c )
                ASSERT_TRUE( 0.0f == out[ 50 * 6 + c ] );
        }

        // mono source to the centre of a 5.1 output, then 5.1 source to a stereo output
        {
            AUDIO::CAudioMixer surround( AUDIO::CAudioFormat( 48000, 6, AUDIO::SAMPLETYPE_FLOAT32 ), 256, 4, 2 );
            AUDIO::TAudioVoicePtr voice = MakeVoice( surround, MakeConstant( 48000, 1, 1000, 0.5f ) );
            ASSERT_TRUE( surround.AddVoice( voice.GetPointerAlways() ) );
            GUCEF::vector< Float32 > out( 100 * 6 );
            surround.Mix( &out[ 0 ], 100 );
            ASSERT_TRUE( 0.0f == out[ 0 ] && 0.0f == out[ 1 ] && Near( 0.5f, out[ 2 ] ) );

            AUDIO::CAudioMixer stereoOut( MixFormat, 256, 4, 2 );
            AUDIO::TAudioVoicePtr film = MakeVoice( stereoOut, MakeConstant( 48000, 6, 1000, 0.5f ) );
            ASSERT_TRUE( GUCEF_NULL != film.GetPointerAlways() );
            ASSERT_TRUE( stereoOut.AddVoice( film.GetPointerAlways() ) );
            GUCEF::vector< Float32 > down( 100 * 2 );
            stereoOut.Mix( &down[ 0 ], 100 );
            ASSERT_TRUE( Near( 0.5f - 0.5f * 0.0f, down[ 0 ], 0.2f ) );                    // some mix of the channels, never over full level
            ASSERT_TRUE( down[ 0 ] <= 0.5f + 1e-5f && down[ 0 ] > 0.0f );
        }
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
PerformSpeedAndTimeTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "AudioMixer: A source of another sample rate is played at the right speed" )
    try
    {
        AUDIO::CAudioMixer mixer( MixFormat, 256, 8, 4 );
        CEndedRecorder recorder;
        mixer.SetListener( &recorder );
        AUDIO::TAudioVoicePtr voice = MakeVoice( mixer, MakeConstant( 24000, 1, 1000, 0.5f ) );
        ASSERT_TRUE( mixer.AddVoice( voice.GetPointerAlways() ) );
        const GUCEF::vector< Float32 > out = MixFrames( mixer, 2200 );
        UInt32 audible = 0;
        for ( UInt32 f=0; f<2200; ++f )
            if ( out[ f * 2 ] != 0.0f )
                ++audible;
        ASSERT_TRUE( audible >= 1995 && audible <= 2000 );                                   // 1000 frames at half the rate are 2000 frames
        for ( UInt32 f=10; f<audible - 10; f += 37 )
            ASSERT_TRUE( Near( 0.5f, out[ f * 2 ] ) );
        ASSERT_TRUE( 1 == recorder.voices.size() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioMixer: Playback rate speeds a sound up and slows it down, also while it plays" )
    try
    {
        AUDIO::CAudioMixer mixer( MixFormat, 256, 8, 4 );
        AUDIO::TAudioVoicePtr voice = MakeVoice( mixer, MakeClip( 48000, 1, 10000, 0.0f, 1.0f ) );
        voice->SetPlaybackRate( 2.0f );
        ASSERT_TRUE( Near( 2.0f, voice->GetPlaybackRate() ) );
        ASSERT_TRUE( mixer.AddVoice( voice.GetPointerAlways() ) );
        GUCEF::vector< Float32 > out = MixFrames( mixer, 1000 );
        for ( UInt32 f=0; f<1000; f += 111 )
            ASSERT_TRUE( Near( 2.0f * (Float32) f / 9999.0f, out[ f * 2 ], 1e-4f ) );       // the ramp is crossed twice as fast
        ASSERT_TRUE( Near( 2000.0f, (Float32) voice->GetPositionInFrames(), 5.0f ) );

        voice->SetPlaybackRate( 0.5f );
        MixFrames( mixer, 100 );
        const UInt64 before = voice->GetPositionInFrames();
        MixFrames( mixer, 1000 );
        ASSERT_TRUE( Near( 500.0f, (Float32) ( voice->GetPositionInFrames() - before ), 5.0f ) );   // half speed: 500 source frames for 1000 mixed

        voice->SetPlaybackRate( 1000.0f );
        ASSERT_TRUE( Near( 16.0f, voice->GetPlaybackRate() ) );                              // limited to what is sensible
        voice->SetPlaybackRate( 0.0f );
        ASSERT_TRUE( Near( 0.05f, voice->GetPlaybackRate() ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioMixer: Looping plays the sound the asked number of times, 0 goes on until stopped" )
    try
    {
        AUDIO::CAudioMixer mixer( MixFormat, 256, 8, 4 );
        CEndedRecorder recorder;
        mixer.SetListener( &recorder );
        AUDIO::TAudioVoicePtr thrice = MakeVoice( mixer, MakeConstant( 48000, 1, 100, 0.5f ) );
        thrice->SetLoopCount( 3 );
        ASSERT_TRUE( 3 == thrice->GetLoopCount() );
        ASSERT_TRUE( mixer.AddVoice( thrice.GetPointerAlways() ) );
        GUCEF::vector< Float32 > out = MixFrames( mixer, 500 );
        UInt32 audible = 0;
        for ( UInt32 f=0; f<500; ++f )
            if ( out[ f * 2 ] != 0.0f )
                ++audible;
        ASSERT_TRUE( 300 == audible );
        ASSERT_TRUE( AUDIO::AUDIOVOICEEND_FINISHED == thrice->GetEndReason() && 1 == recorder.voices.size() );

        AUDIO::TAudioVoicePtr forever = MakeVoice( mixer, MakeConstant( 48000, 1, 100, 0.5f ) );
        forever->SetLoopCount( 0 );
        ASSERT_TRUE( mixer.AddVoice( forever.GetPointerAlways() ) );
        out = MixFrames( mixer, 5000 );
        ASSERT_TRUE( Near( 0.5f, out[ 4999 * 2 ] ) && AUDIO::AUDIOVOICESTATE_PLAYING == forever->GetState() );
        forever->RequestStop();
        ASSERT_TRUE( forever->IsStopRequested() );
        out = MixFrames( mixer, 100 );
        ASSERT_TRUE( 0.0f == out[ 0 ] );
        ASSERT_TRUE( AUDIO::AUDIOVOICESTATE_ENDED == forever->GetState() && AUDIO::AUDIOVOICEEND_STOPPED == forever->GetEndReason() );
        ASSERT_TRUE( 2 == recorder.voices.size() && AUDIO::AUDIOVOICEEND_STOPPED == recorder.reasons[ 1 ] );

        // an empty sound does not loop forever, it just ends
        AUDIO::TAudioVoicePtr empty = MakeVoice( mixer, MakeConstant( 48000, 1, 0, 0.5f ) );
        empty->SetLoopCount( 0 );
        ASSERT_TRUE( mixer.AddVoice( empty.GetPointerAlways() ) );
        MixFrames( mixer, 100 );
        ASSERT_TRUE( AUDIO::AUDIOVOICESTATE_ENDED == empty->GetState() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioMixer: Pausing holds the sound and the position, going on continues from there" )
    try
    {
        AUDIO::CAudioMixer mixer( MixFormat, 256, 8, 4 );
        AUDIO::TAudioVoicePtr voice = MakeVoice( mixer, MakeClip( 48000, 1, 10000, 0.0f, 1.0f ) );
        ASSERT_TRUE( mixer.AddVoice( voice.GetPointerAlways() ) );
        MixFrames( mixer, 500 );
        ASSERT_TRUE( 500 == voice->GetPositionInFrames() );

        voice->RequestPause( true );
        ASSERT_TRUE( voice->IsPauseRequested() );
        GUCEF::vector< Float32 > out = MixFrames( mixer, 500 );
        ASSERT_TRUE( 0.0f == out[ 0 ] && 0.0f == out[ 999 ] );
        ASSERT_TRUE( AUDIO::AUDIOVOICESTATE_PAUSED == voice->GetState() && 500 == voice->GetPositionInFrames() );
        ASSERT_TRUE( 1 == mixer.GetVoiceCount() );

        voice->RequestPause( false );
        out = MixFrames( mixer, 100 );
        ASSERT_TRUE( AUDIO::AUDIOVOICESTATE_PLAYING == voice->GetState() && 600 == voice->GetPositionInFrames() );
        ASSERT_TRUE( Near( 500.0f / 9999.0f, out[ 0 ], 1e-4f ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioMixer: Mixing in pieces of any size gives the same audio as mixing all at once" )
    try
    {
        const UInt32 total = 5000;
        GUCEF::vector< Float32 > reference;
        const UInt32 pieces[ 6 ] = { 5000, 1, 7, 64, 255, 1234 };
        for ( UInt32 p=0; p<6; ++p )
        {
            AUDIO::CAudioMixer mixer( MixFormat, 128, 8, 4 );
            AUDIO::CAudioBus* effects = mixer.CreateBus( "effects" );
            effects->SetVolume( 0.7f );
            AUDIO::TAudioVoicePtr a = MakeVoice( mixer, MakeClip( 22050, 1, 3000, -0.5f, 0.9f ), effects );   // other rate
            AUDIO::TAudioVoicePtr b = MakeVoice( mixer, MakeClip( 48000, 2, 900, 0.1f, 0.4f ) );
            a->SetPan( -0.3f );
            a->SetVolume( 0.8f );
            a->SetPlaybackRate( 1.25f );
            b->SetLoopCount( 4 );
            ASSERT_TRUE( mixer.AddVoice( a.GetPointerAlways() ) && mixer.AddVoice( b.GetPointerAlways() ) );

            GUCEF::vector< Float32 > out( (size_t) total * 2 );
            UInt32 done = 0;
            while ( done < total )
            {
                const UInt32 n = ( total - done < pieces[ p ] ) ? ( total - done ) : pieces[ p ];
                mixer.Mix( &out[ (size_t) done * 2 ], n );
                done += n;
            }
            if ( 0 == p )
                reference = out;
            else
            {
                bool same = true;
                for ( size_t i=0; same && i<out.size(); ++i )
                    same = Near( out[ i ], reference[ i ], 2e-5f );
                ASSERT_TRUE( same );
            }
        }
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
PerformLimitTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "AudioMixer: Limits and bad input are refused, not mixed" )
    try
    {
        AUDIO::CAudioMixer mixer( MixFormat, 256, 2, 4 );
        ASSERT_FALSE( mixer.AddVoice( GUCEF_NULL ) );
        ASSERT_FALSE( mixer.PrepareVoice( GUCEF_NULL ) );

        AUDIO::TAudioVoicePtr unprepared( GUCEF_NEW AUDIO::CAudioVoice( MakeConstant( 48000, 1, 10, 0.1f )->CreateSource() ) );
        ASSERT_FALSE( mixer.AddVoice( unprepared.GetPointerAlways() ) );                    // not prepared

        AUDIO::TAudioVoicePtr noSource( GUCEF_NEW AUDIO::CAudioVoice( AUDIO::TAudioSourcePtr() ) );
        ASSERT_FALSE( mixer.PrepareVoice( noSource.GetPointerAlways() ) );

        AUDIO::TAudioVoicePtr noFormat( GUCEF_NEW AUDIO::CAudioVoice( AUDIO::CAudioClip().CreateSource() ) );
        ASSERT_FALSE( mixer.PrepareVoice( noFormat.GetPointerAlways() ) );                  // a clip without a format

        AUDIO::TAudioVoicePtr a = MakeVoice( mixer, MakeConstant( 48000, 1, 100000, 0.1f ) );
        AUDIO::TAudioVoicePtr b = MakeVoice( mixer, MakeConstant( 48000, 1, 100000, 0.1f ) );
        AUDIO::TAudioVoicePtr c = MakeVoice( mixer, MakeConstant( 48000, 1, 100000, 0.1f ) );
        ASSERT_TRUE( mixer.AddVoice( a.GetPointerAlways() ) && mixer.AddVoice( b.GetPointerAlways() ) );
        ASSERT_FALSE( mixer.AddVoice( c.GetPointerAlways() ) );                             // the maximum is two voices
        ASSERT_TRUE( 2 == mixer.GetVoiceCount() );
        a->RequestStop();
        MixFrames( mixer, 10 );
        ASSERT_TRUE( 1 == mixer.GetVoiceCount() );
        ASSERT_TRUE( mixer.AddVoice( c.GetPointerAlways() ) );                              // room again
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioMixer: A source that has nothing right now gives silence and plays on when it has audio again" )
    try
    {
        AUDIO::CAudioMixer mixer( MixFormat, 256, 8, 4 );
        CStarvedSource* starved = GUCEF_NEW CStarvedSource();
        AUDIO::TAudioVoicePtr voice( GUCEF_NEW AUDIO::CAudioVoice( AUDIO::TAudioSourcePtr( starved ) ) );
        ASSERT_TRUE( mixer.PrepareVoice( voice.GetPointerAlways() ) && mixer.AddVoice( voice.GetPointerAlways() ) );
        GUCEF::vector< Float32 > out = MixFrames( mixer, 100 );
        ASSERT_TRUE( 0.0f == out[ 0 ] && AUDIO::AUDIOVOICESTATE_PLAYING == voice->GetState() );     // it waits, it does not end
        starved->available = 50;
        out = MixFrames( mixer, 100 );
        ASSERT_TRUE( Near( 0.25f, out[ 49 * 2 ] ) && 0.0f == out[ 50 * 2 ] );
        ASSERT_TRUE( AUDIO::AUDIOVOICESTATE_PLAYING == voice->GetState() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioMixer: The mix is float whatever format was asked, voices keep working at many channel counts" )
    try
    {
        AUDIO::CAudioMixer mixer( AUDIO::CAudioFormat( 44100, 2, AUDIO::SAMPLETYPE_INT16 ), 128, 4, 2 );
        ASSERT_TRUE( AUDIO::SAMPLETYPE_FLOAT32 == mixer.GetMixFormat().GetSampleType() );

        // eight channel output with a mono source
        AUDIO::CAudioMixer wide( AUDIO::CAudioFormat( 48000, 8, AUDIO::SAMPLETYPE_FLOAT32 ), 128, 4, 2 );
        AUDIO::TAudioVoicePtr voice = MakeVoice( wide, MakeConstant( 48000, 1, 1000, 0.5f ) );
        voice->SetPan( 0.5f );
        ASSERT_TRUE( wide.AddVoice( voice.GetPointerAlways() ) );
        GUCEF::vector< Float32 > out( 100 * 8 );
        wide.Mix( &out[ 0 ], 100 );
        ASSERT_TRUE( Near( 0.5f, out[ 50 * 8 + 2 ] ) );                                       // the centre channel, not touched by the pan
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

namespace {

/** Reproducible noise in -1 .. 1 */
Float32
NextNoise( UInt32& state )
{
    state = state * 1664525UL + 1013904223UL;
    return (Float32) ( (Float64) ( state >> 8 ) / 8388608.0 - 1.0 );
}

/** A stereo clip of noise at 'amplitude', with an occasional full burst so the peaks are sharp */
AUDIO::TAudioClipPtr
MakeLoudNoiseClip( UInt32 frames, Float32 amplitude, UInt32 seed )
{
    AUDIO::CAudioBuffer audio( MixFormat, frames );
    UInt32 state = seed;
    for ( UInt32 f=0; f<frames; ++f )
    {
        const Float32 burst = ( f % 997 ) < 3 ? 1.0f : 0.6f;
        for ( UInt32 c=0; c<2; ++c )
            audio.SetSampleFromFloat( f, c, amplitude * burst * NextNoise( state ) );
    }
    return AUDIO::TAudioClipPtr( GUCEF_NEW AUDIO::CAudioClip( audio ) );
}

/** A mono clip that is silent but for one frame */
AUDIO::TAudioClipPtr
MakeImpulse( UInt32 rate, UInt32 frames, UInt32 at, Float32 value )
{
    AUDIO::CAudioBuffer audio( AUDIO::CAudioFormat( rate, 1, AUDIO::SAMPLETYPE_FLOAT32 ), frames );
    for ( UInt32 f=0; f<frames; ++f )
        audio.SetSampleFromFloat( f, 0, f == at ? value : 0.0f );
    return AUDIO::TAudioClipPtr( GUCEF_NEW AUDIO::CAudioClip( audio ) );
}

/** The frame where a signal first is not silent, 'frames' when it never is */
UInt32
FirstSoundFrame( const GUCEF::vector< Float32 >& out, UInt32 channels )
{
    const UInt32 frames = (UInt32) ( out.size() / channels );
    for ( UInt32 f=0; f<frames; ++f )
        for ( UInt32 c=0; c<channels; ++c )
            if ( 0.0f != out[ (size_t) f * channels + c ] )
                return f;
    return frames;
}

Float32
PeakOf( const GUCEF::vector< Float32 >& out )
{
    Float32 peak = 0.0f;
    for ( size_t i=0; i<out.size(); ++i )
        peak = fabsf( out[ i ] ) > peak ? fabsf( out[ i ] ) : peak;
    return peak;
}

}

/*-------------------------------------------------------------------------*/

static void
PerformMasterLimiterTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "AudioMixer: The master limiter is off by default; SetLimiterEnabled switches between off and the mode last chosen" )
    try
    {
        AUDIO::CAudioMixer mixer( MixFormat, 256, 8, 4 );
        ASSERT_TRUE( !mixer.IsLimiterEnabled() && AUDIO::AUDIOLIMITERMODE_OFF == mixer.GetLimiterMode() && 0 == mixer.GetLatencyInFrames() );
        ASSERT_TRUE( Near( -0.3f, mixer.GetLimiterCeilingInDecibels() ) && Near( 1.5f, mixer.GetLimiterLookaheadInMs() ) && Near( 50.0f, mixer.GetLimiterReleaseInMs() ) );

        // on without a mode chosen is lookahead, as in the engine
        mixer.SetLimiterEnabled( true );
        ASSERT_TRUE( mixer.IsLimiterEnabled() && AUDIO::AUDIOLIMITERMODE_LOOKAHEAD == mixer.GetLimiterMode() && 72 == mixer.GetLatencyInFrames() );

        // instant is remembered over off
        mixer.SetLimiterMode( AUDIO::AUDIOLIMITERMODE_INSTANT );
        ASSERT_TRUE( mixer.IsLimiterEnabled() && AUDIO::AUDIOLIMITERMODE_INSTANT == mixer.GetLimiterMode() && 0 == mixer.GetLatencyInFrames() );
        mixer.SetLimiterEnabled( false );
        ASSERT_TRUE( !mixer.IsLimiterEnabled() && AUDIO::AUDIOLIMITERMODE_OFF == mixer.GetLimiterMode() );
        mixer.SetLimiterEnabled( true );
        ASSERT_TRUE( AUDIO::AUDIOLIMITERMODE_INSTANT == mixer.GetLimiterMode() );
        mixer.SetLimiterMode( AUDIO::AUDIOLIMITERMODE_OFF );
        ASSERT_TRUE( !mixer.IsLimiterEnabled() && AUDIO::AUDIOLIMITERMODE_OFF == mixer.GetLimiterMode() );
        mixer.SetLimiterEnabled( true );
        ASSERT_TRUE( AUDIO::AUDIOLIMITERMODE_INSTANT == mixer.GetLimiterMode() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioMixer: With the limiter off the master passes unchanged but for the clamp, which counts what it cut off" )
    try
    {
        AUDIO::CAudioMixer mixer( MixFormat, 256, 8, 4 );
        mixer.SetLimiterMode( AUDIO::AUDIOLIMITERMODE_OFF );
        const UInt32 frames = 3000;
        AUDIO::CAudioBuffer audio( MixFormat, frames );
        for ( UInt32 f=0; f<frames; ++f )
        {
            audio.SetSampleFromFloat( f, 0, -1.6f + 3.2f * (Float32) f / (Float32) ( frames - 1 ) );
            audio.SetSampleFromFloat( f, 1, 0.37f * (Float32) sin( (Float64) f * 0.05 ) );
        }
        AUDIO::TAudioVoicePtr voice = MakeVoice( mixer, AUDIO::TAudioClipPtr( GUCEF_NEW AUDIO::CAudioClip( audio ) ) );
        ASSERT_TRUE( mixer.AddVoice( voice.GetPointerAlways() ) );
        GUCEF::vector< Float32 > out( (size_t) frames * 2, 123.0f );
        const UInt32 pieces[ 4 ] = { 1, 37, 256, 999 };
        UInt32 done = 0;
        for ( UInt32 p=0; done<frames; ++p )
        {
            UInt32 piece = pieces[ p % 4 ];
            piece = piece < frames - done ? piece : frames - done;
            mixer.Mix( &out[ (size_t) done * 2 ], piece );
            done += piece;
        }

        UInt64 over = 0;
        for ( UInt32 f=0; f<frames; ++f )
        {
            for ( UInt32 c=0; c<2; ++c )
            {
                const Float32 in = audio.GetSampleAsFloat( f, c );
                const Float32 expected = in > 1.0f ? 1.0f : ( in < -1.0f ? -1.0f : in );
                if ( in > 1.0f || in < -1.0f )
                    ++over;
                ASSERT_TRUE( expected == out[ (size_t) f * 2 + c ] );       // bit for bit, no delay
            }
        }
        ASSERT_TRUE( over > 1000 && over == mixer.GetClippedSampleCount() );
        ASSERT_TRUE( 0 == mixer.GetLimitedFrameCount() && 0 == mixer.GetLatencyInFrames() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioMixer: In lookahead mode a loud mix never goes over the ceiling and the clamp has nothing to cut off" )
    try
    {
        const Float32 ceilings[ 2 ] = { -0.3f, -6.0f };
        for ( UInt32 run=0; run<2; ++run )
        {
            AUDIO::CAudioMixer mixer( MixFormat, 256, 8, 4 );
            mixer.SetLimiterMode( AUDIO::AUDIOLIMITERMODE_LOOKAHEAD );
            if ( 1 == run )
                mixer.SetLimiterCeilingInDecibels( ceilings[ run ] );
            AUDIO::CAudioBus* music = mixer.CreateBus( "music" );
            music->SetVolume( 1.5f );

            // four noisy voices: together up to about six times full level
            AUDIO::TAudioVoicePtr voices[ 4 ];
            for ( UInt32 v=0; v<4; ++v )
            {
                voices[ v ] = MakeVoice( mixer, MakeLoudNoiseClip( 24000, 0.9f, 7 + v ), v < 2 ? music : GUCEF_NULL );
                ASSERT_TRUE( mixer.AddVoice( voices[ v ].GetPointerAlways() ) );
            }

            GUCEF::vector< Float32 > out( (size_t) 24000 * 2, 123.0f );
            const UInt32 pieces[ 5 ] = { 100, 1, 256, 73, 512 };
            UInt32 done = 0;
            for ( UInt32 p=0; done<24000; ++p )
            {
                UInt32 piece = pieces[ p % 5 ];
                piece = piece < 24000 - done ? piece : 24000 - done;
                mixer.Mix( &out[ (size_t) done * 2 ], piece );
                done += piece;
            }

            const Float64 ceiling = pow( 10.0, (Float64) ceilings[ run ] / 20.0 );
            const Float32 peak = PeakOf( out );
            ASSERT_TRUE( (Float64) peak <= ceiling );
            ASSERT_TRUE( (Float64) peak > 0.9 * ceiling );                      // turned down to the ceiling, not below it
            ASSERT_TRUE( 0 == mixer.GetClippedSampleCount() );
            ASSERT_TRUE( mixer.GetLimitedFrameCount() > 10000 );
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioMixer: lookahead master limiter at " << ceilings[ run ] << " dB, peak " << peak << ", " << mixer.GetLimitedFrameCount() << " frames limited" );
        }
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioMixer: The latency the mixer reports is the lookahead, and the mix comes out exactly that much later" )
    try
    {
        AUDIO::CAudioMixer mixer( MixFormat, 256, 8, 4 );
        mixer.SetLimiterMode( AUDIO::AUDIOLIMITERMODE_LOOKAHEAD );
        ASSERT_TRUE( 72 == mixer.GetLatencyInFrames() );                       // 1.5 ms at 48 kHz

        AUDIO::TAudioVoicePtr voice = MakeVoice( mixer, MakeImpulse( 48000, 1000, 50, 0.5f ) );
        ASSERT_TRUE( mixer.AddVoice( voice.GetPointerAlways() ) );
        GUCEF::vector< Float32 > out = MixFrames( mixer, 1000 );
        ASSERT_TRUE( 50 + 72 == FirstSoundFrame( out, 2 ) );
        ASSERT_TRUE( 0.5f == out[ ( 50 + 72 ) * 2 ] && 0.5f == out[ ( 50 + 72 ) * 2 + 1 ] && 0.0f == out[ ( 50 + 73 ) * 2 ] );   // unchanged, only later
        ASSERT_TRUE( 0 == mixer.GetLimitedFrameCount() );

        // another lookahead, and another rate
        mixer.SetLimiterLookaheadInMs( 3.0f );
        ASSERT_TRUE( 144 == mixer.GetLatencyInFrames() );
        AUDIO::TAudioVoicePtr second = MakeVoice( mixer, MakeImpulse( 48000, 1000, 50, 0.5f ) );
        ASSERT_TRUE( mixer.AddVoice( second.GetPointerAlways() ) );
        out = MixFrames( mixer, 1000 );
        ASSERT_TRUE( 50 + 144 == FirstSoundFrame( out, 2 ) );

        ASSERT_TRUE( mixer.SetMixFormat( AUDIO::CAudioFormat( 96000, 2, AUDIO::SAMPLETYPE_FLOAT32 ) ) );
        ASSERT_TRUE( 288 == mixer.GetLatencyInFrames() );
        AUDIO::TAudioVoicePtr third = MakeVoice( mixer, MakeImpulse( 96000, 1000, 50, 0.5f ) );
        ASSERT_TRUE( mixer.AddVoice( third.GetPointerAlways() ) );
        out = MixFrames( mixer, 1000 );
        ASSERT_TRUE( 50 + 288 == FirstSoundFrame( out, 2 ) );

        // no latency in the other modes
        mixer.SetLimiterMode( AUDIO::AUDIOLIMITERMODE_INSTANT );
        ASSERT_TRUE( 0 == mixer.GetLatencyInFrames() );
        mixer.SetLimiterMode( AUDIO::AUDIOLIMITERMODE_OFF );
        ASSERT_TRUE( 0 == mixer.GetLatencyInFrames() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioMixer: Instant mode keeps its ceiling of 0.98 whatever ceiling is set for lookahead mode, also after a format change" )
    try
    {
        AUDIO::CAudioMixer mixer( MixFormat, 256, 8, 4 );
        mixer.SetLimiterCeilingInDecibels( -6.0f );
        mixer.SetLimiterMode( AUDIO::AUDIOLIMITERMODE_INSTANT );
        ASSERT_TRUE( Near( -6.0f, mixer.GetLimiterCeilingInDecibels() ) );
        AUDIO::TAudioVoicePtr a = MakeVoice( mixer, MakeConstant( 48000, 1, 4000, 0.8f ) );
        AUDIO::TAudioVoicePtr b = MakeVoice( mixer, MakeConstant( 48000, 1, 4000, 0.8f ) );
        ASSERT_TRUE( mixer.AddVoice( a.GetPointerAlways() ) && mixer.AddVoice( b.GetPointerAlways() ) );
        GUCEF::vector< Float32 > out = MixFrames( mixer, 100 );
        ASSERT_TRUE( Near( 0.98f, out[ 0 ], 1e-6f ) && Near( 0.98f, out[ 199 ], 1e-6f ) );

        ASSERT_TRUE( mixer.SetMixFormat( AUDIO::CAudioFormat( 44100, 2, AUDIO::SAMPLETYPE_FLOAT32 ) ) );
        out = MixFrames( mixer, 100 );
        ASSERT_TRUE( Near( 0.98f, out[ 0 ], 1e-6f ) && Near( 0.98f, out[ 199 ], 1e-6f ) );
        ASSERT_TRUE( 0 == mixer.GetClippedSampleCount() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
PerformAudioMixerTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING AudioMixer TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "gucefAUDIO.Mixer" );

    PerformBasicMixingTests();
    PerformLevelTests();
    PerformPanAndChannelTests();
    PerformSpeedAndTimeTests();
    PerformLimitTests();
    PerformMasterLimiterTests();
}

/*-------------------------------------------------------------------------*/
