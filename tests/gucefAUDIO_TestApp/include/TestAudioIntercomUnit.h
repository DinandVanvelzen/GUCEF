/*
 *  gucefAUDIO_TestApp: GUCEF test application for the AUDIO module
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

#ifndef GUCEF_AUDIO_TESTAPP_TESTAUDIOINTERCOMUNIT_H
#define GUCEF_AUDIO_TESTAPP_TESTAUDIOINTERCOMUNIT_H

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

#include "TestAudioVoiceStreamSupport.h"
#include "TestAudioSimulatedVoiceLink.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  A unit of the intercom in a test: an engine on the null driver with a playback and a capture device, the
 *  microphone fed from a test signal (plus, when asked, the unit's own speaker output through a simulated acoustic
 *  path: delay and attenuation), a sender as capture consumer and receivers on buses. Several units and the
 *  simulated links between them run in one simulated clock (CTestIntercomScenario, in TestAudioIntercomScenario.h).
 *  Test code: allocates freely; the parts the audio threads call (the feed source) do not.
 */
namespace VoiceLinkTest {

using namespace GUCEF;

/*-------------------------------------------------------------------------*/

/** What a unit's microphone picks up from the room: sample n at the unit's rate */
class CTestSignal
{
    public:

    virtual ~CTestSignal() {}

    virtual Float32 Sample( UInt64 n ) const = 0;
};

/*-------------------------------------------------------------------------*/

/** The exact 16 bit PCM test signal of VoiceStreamTest (two tones and an offset) with an optional click every 'clickPeriod' samples */
class CPcmClickSignal : public CTestSignal
{
    public:

    /**
     *  The click: 96 samples (2 ms at 48 kHz) of fixed pseudo random noise, smoothed (a moving average of 6, so it
     *  passes resamplers and codecs with little change) under a Hann window, with a peak of 0.45, exact in 16 bit PCM
     */
    static const UInt32 ClickLength = 96;

    CPcmClickSignal( UInt32 sampleRate, UInt32 clickPeriod, UInt32 clickOffset, bool withTone = true )
        : m_rate( sampleRate ), m_clickPeriod( clickPeriod ), m_clickOffset( clickOffset ), m_withTone( withTone )
    {
        AudioEffectTest::CXorShift random( 0x5EEDC11C );
        Float64 noise[ ClickLength + 5 ];
        for ( UInt32 i=0; i<ClickLength + 5; ++i )
            noise[ i ] = random.NextUnit() * 2.0 - 1.0;
        Float64 shaped[ ClickLength ];
        Float64 peak = 0.0;
        for ( UInt32 i=0; i<ClickLength; ++i )
        {
            Float64 sum = 0.0;
            for ( UInt32 j=0; j<6; ++j )
                sum += noise[ i + j ];
            const Float64 window = 0.5 - 0.5 * cos( 2.0 * 3.14159265358979323846 * ( (Float64) i + 0.5 ) / (Float64) ClickLength );
            shaped[ i ] = window * sum;
            if ( fabs( shaped[ i ] ) > peak )
                peak = fabs( shaped[ i ] );
        }
        for ( UInt32 i=0; i<ClickLength; ++i )
            m_click[ i ] = (Float32) floor( shaped[ i ] / peak * 0.45 * 32768.0 + 0.5 ) / 32768.0f;
    }

    virtual Float32 Sample( UInt64 n ) const GUCEF_VIRTUAL_OVERRIDE
    {
        // the tone pauses 5 ms around each click, so the click stands alone where it is searched for
        const bool nearClick = IsNearClick( n );
        Float32 value = ( m_withTone && !nearClick ) ? VoiceStreamTest::PcmSample( n, m_rate ) : 0.0f;
        const Int64 offset = ClickOffset( n );
        if ( offset >= 0 )
            value += m_click[ offset ];
        return value;
    }

    /** The position in the click of sample n, -1 outside a click */
    Int64 ClickOffset( UInt64 n ) const
    {
        if ( 0 == m_clickPeriod || n < m_clickOffset )
            return -1;
        const UInt64 within = ( n - m_clickOffset ) % m_clickPeriod;
        return within < ClickLength ? (Int64) within : -1;
    }

    /** Sample n lies within 5 ms of a click (where the tone pauses) */
    bool IsNearClick( UInt64 n ) const
    {
        if ( 0 == m_clickPeriod )
            return false;
        const UInt64 guard = m_rate / 200;
        const UInt64 shifted = n + guard;
        if ( shifted < m_clickOffset )
            return false;
        return ( shifted - m_clickOffset ) % m_clickPeriod < ClickLength + 2 * guard;
    }

    UInt32 GetClickPeriod( void ) const { return m_clickPeriod; }
    UInt32 GetClickOffset( void ) const { return m_clickOffset; }
    const Float32* GetClick( void ) const { return m_click; }

    private:

    UInt32 m_rate;
    UInt32 m_clickPeriod;
    UInt32 m_clickOffset;
    bool m_withTone;
    Float32 m_click[ ClickLength ];
};

/*-------------------------------------------------------------------------*/

/** A recorded signal (speech-like, a tone) scaled by 'gain', silence after its end */
class CBufferSignal : public CTestSignal
{
    public:

    explicit CBufferSignal( const GUCEF::vector< Float32 >& samples, Float32 gain = 1.0f ) : m_samples( samples ), m_gain( gain ) {}

    virtual Float32 Sample( UInt64 n ) const GUCEF_VIRTUAL_OVERRIDE
    {
        return n < m_samples.size() ? m_gain * m_samples[ (size_t) n ] : 0.0f;
    }

    void SetGain( Float32 gain ) { m_gain = gain; }

    private:

    GUCEF::vector< Float32 > m_samples;
    Float32 m_gain;
};

/*-------------------------------------------------------------------------*/

/** The sum of two signals (a talker and the noise of the room, two talkers) */
class CSumSignal : public CTestSignal
{
    public:

    CSumSignal( const CTestSignal* a, const CTestSignal* b ) : m_a( a ), m_b( b ) {}

    virtual Float32 Sample( UInt64 n ) const GUCEF_VIRTUAL_OVERRIDE
    {
        return ( GUCEF_NULL != m_a ? m_a->Sample( n ) : 0.0f ) + ( GUCEF_NULL != m_b ? m_b->Sample( n ) : 0.0f );
    }

    private:

    const CTestSignal* m_a;
    const CTestSignal* m_b;
};

/*-------------------------------------------------------------------------*/

/**
 *  A live capture source that the unit fills before every capture render: mono Float32 in a preallocated ring, so the
 *  capture device can read it on its (real time) thread without allocating
 */
class CTestFeedSource : public AUDIO::IAudioSource
{
    public:

    enum { Capacity = 1 << 16 };

    explicit CTestFeedSource( UInt32 sampleRate ) : m_format( sampleRate, 1, AUDIO::SAMPLETYPE_FLOAT32 ), m_ring( Capacity, 0.0f ), m_read( 0 ), m_write( 0 ) {}

    virtual const AUDIO::CAudioFormat& GetFormat( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_format; }

    virtual UInt32 ReadFrames( void* destination, UInt32 frameCapacity ) GUCEF_VIRTUAL_OVERRIDE
    {
        Float32* out = static_cast< Float32* >( destination );
        UInt32 count = 0;
        while ( count < frameCapacity && m_read != m_write )
        {
            out[ count++ ] = m_ring[ (size_t) ( m_read % Capacity ) ];
            ++m_read;
        }
        return count;
    }

    virtual bool IsEndOfStream( void ) const GUCEF_VIRTUAL_OVERRIDE { return false; }
    virtual bool IsRealtimeSafe( void ) const GUCEF_VIRTUAL_OVERRIDE { return true; }
    virtual UInt64 GetPositionInFrames( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_read; }

    /** The owner side, between renders: false when the ring is full */
    bool Push( Float32 sample )
    {
        if ( m_write - m_read >= (UInt64) Capacity )
            return false;
        m_ring[ (size_t) ( m_write % Capacity ) ] = sample;
        ++m_write;
        return true;
    }

    private:

    AUDIO::CAudioFormat m_format;
    GUCEF::vector< Float32 > m_ring;
    UInt64 m_read;
    UInt64 m_write;
};

/*-------------------------------------------------------------------------*/

/** The clock of a unit: simulated, and capture tick counts are real ticks that do not compare with it */
class CUnitClock : public VoiceStreamTest::CSimulatedVoiceClock
{
    public:

    CUnitClock( void ) : VoiceStreamTest::CSimulatedVoiceClock( 1000000 ) {}

    virtual bool CaptureTickToTimeInUs( UInt64, UInt64& ) const GUCEF_VIRTUAL_OVERRIDE { return false; }
};

/*-------------------------------------------------------------------------*/

/** How a unit is made */
struct SUnitConfig
{
    UInt32 sampleRate;              /**< of the devices, the mix and (unless processingRate is set) the capture processing */
    UInt32 processingRate;          /**< of the capture processing, 0: the sample rate */
    UInt32 period;                  /**< device period in frames (240: 5 ms at 48 kHz, the low latency profile) */
    bool capture;
    Float64 driftPpm;               /**< the unit's crystal runs this much faster than the simulated time */
    Float64 echoGain;               /**< acoustic path from the unit's speaker to its microphone, linear (0: none) */
    UInt32 echoDelayFrames;         /**< of that path, beyond the playback period */
    bool keepOutput;                /**< keep everything the speaker played (else only the last seconds, for the echo path) */
    AUDIO::TAudioThreadingMode threading;

    SUnitConfig( void )
        : sampleRate( 48000 ), processingRate( 0 ), period( 240 ), capture( true ), driftPpm( 0.0 ), echoGain( 0.0 ), echoDelayFrames( 0 ), keepOutput( true )
        , threading( AUDIO::AUDIOTHREADING_COOPERATIVE )
    {}
};

/*-------------------------------------------------------------------------*/

/** A frame the sender of a unit made, with the simulated time at which it was complete */
struct SSentFrame
{
    AUDIO::CVoiceFrame frame;
    Float64 sendTimeInUs;
};
typedef GUCEF::vector< SSentFrame > TSentFrames;

/*-------------------------------------------------------------------------*/

class CTestIntercomUnit
{
    public:

    /** The speaker output kept for the echo path: about 2.7 s at 48 kHz */
    enum { OutputRingFrames = 1 << 17 };

    explicit CTestIntercomUnit( const SUnitConfig& config = SUnitConfig() )
        : m_config( config ), clock(), engine( GUCEF_NULL ), sender( GUCEF_NULL ), receivers(), output(), m_feed( GUCEF_NULL ), m_feedPtr()
        , m_playback( GUCEF_NULL ), m_captureDevice( GUCEF_NULL ), m_signal( GUCEF_NULL ), m_capturePosition( 0 ), m_outputPosition( 0 )
        , m_captureCarry( 0.0 ), m_playCarry( 0.0 ), m_senderBase( 0 ), m_ring( OutputRingFrames, 0.0f ), m_capturedLast( 0 )
    {}

    ~CTestIntercomUnit()
    {
        Shutdown();
    }

    const SUnitConfig& GetConfig( void ) const { return m_config; }

    static AUDIO::CAudioEngineParams MakeEngineParams( const SUnitConfig& config )
    {
        AUDIO::CAudioEngineParams params;
        params.SetDriverName( "null" );
        params.SetThreadingMode( config.threading );
        params.SetLimiterMode( AUDIO::AUDIOLIMITERMODE_OFF );
        params.GetDeviceParams().SetPreferredFormat( VoiceStreamTest::MonoFormat( config.sampleRate ) );
        params.GetDeviceParams().SetBufferSizeInFrames( config.period );
        params.GetDeviceParams().GetOptions().Set( "keepOutput", "true" );
        params.SetCaptureEnabled( config.capture );
        params.SetCaptureProcessingRate( 0 != config.processingRate ? config.processingRate : config.sampleRate );
        params.GetCaptureDeviceParams().SetPreferredFormat( VoiceStreamTest::MonoFormat( config.sampleRate ) );
        params.GetCaptureDeviceParams().SetBufferSizeInFrames( config.period );
        return params;
    }

    bool Init( void )
    {
        engine = GUCEF_NEW AUDIO::CAudioEngine();
        if ( !engine->Initialize( MakeEngineParams( m_config ) ) )
            return false;
        m_playback = engine->GetDevice()->AsNullDevice();
        if ( GUCEF_NULL == m_playback )
            return false;
        if ( m_config.capture )
        {
            if ( !engine->IsCaptureAvailable() )
                return false;
            m_captureDevice = engine->GetCaptureDevice()->AsNullDevice();
            m_feed = GUCEF_NEW CTestFeedSource( m_config.sampleRate );
            m_feedPtr = AUDIO::TAudioSourcePtr( m_feed );
            if ( GUCEF_NULL == m_captureDevice || !m_captureDevice->SetCaptureSource( m_feedPtr ) )
                return false;
        }
        return true;
    }

    void Shutdown( void )
    {
        for ( size_t i=0; i<receivers.size(); ++i )
            GUCEF_DELETE receivers[ i ];
        receivers.clear();
        if ( GUCEF_NULL != engine )
        {
            if ( GUCEF_NULL != sender && engine->IsCaptureAvailable() )
            {
                engine->RemoveCaptureConsumer( sender );
                if ( GUCEF_NULL != m_captureDevice )
                    m_captureDevice->Render( 1 );
                engine->Update();
            }
            engine->Shutdown();
        }
        GUCEF_DELETE engine;
        engine = GUCEF_NULL;
        GUCEF_DELETE sender;
        sender = GUCEF_NULL;
        m_playback = GUCEF_NULL;
        m_captureDevice = GUCEF_NULL;
    }

    /** The near-end sound of the room (the unit does not own it) */
    void SetSignal( const CTestSignal* signal ) { m_signal = signal; }

    void SetEcho( Float64 gain, UInt32 delayFrames ) { m_config.echoGain = gain; m_config.echoDelayFrames = delayFrames; }

    /**
     *  Makes the unit's sender and adds it as capture consumer; a sender there is replaced (its stream ends). The
     *  configuration must set the first timestamp: the send times are worked out from the timestamps
     */
    bool AttachSender( const AUDIO::CVoiceSenderConfig& senderConfig )
    {
        DetachSender();
        sender = GUCEF_NEW AUDIO::CVoiceSender( senderConfig );
        const CORE::CFutureResult added = engine->AddCaptureConsumer( sender );
        if ( added.HasNoFuture() )
            return false;
        // the command reaches the capture thread with the next device period
        m_senderBase = m_capturePosition;
        return true;
    }

    void DetachSender( void )
    {
        if ( GUCEF_NULL == sender )
            return;
        engine->RemoveCaptureConsumer( sender );
        // the removal is carried out at the start of the next capture callback: render one frame of the signal for it
        CaptureFrames( 1 );
        engine->Update();
        GUCEF_DELETE sender;
        sender = GUCEF_NULL;
    }

    /** A receiver on 'bus' (the master bus when null), prepared and started, in the unit's clock */
    AUDIO::CVoiceReceiver* AddReceiver( const AUDIO::CVoiceReceiverConfig& receiverConfig, AUDIO::CAudioBus* bus = GUCEF_NULL )
    {
        AUDIO::CVoiceReceiver* receiver = GUCEF_NEW AUDIO::CVoiceReceiver( *engine, receiverConfig );
        receiver->SetClock( clock );
        if ( !receiver->Prepare() )
        {
            GUCEF_DELETE receiver;
            return GUCEF_NULL;
        }
        const CORE::CFutureResult started = receiver->Start( bus );
        engine->Update();
        if ( started.HasNoFuture() )
        {
            GUCEF_DELETE receiver;
            return GUCEF_NULL;
        }
        receivers.push_back( receiver );
        return receiver;
    }

    /** The unit's clock as it is at simulated time 'timeInUs' (the clock starts at 1 s) */
    Float64 LocalScale( void ) const { return 1.0 + m_config.driftPpm * 1.0e-6; }
    Float64 LocalOffsetUs( void ) const { return 1000000.0; }
    Float64 ToLocalUs( Float64 timeInUs ) const { return LocalOffsetUs() + LocalScale() * timeInUs; }

    void SetTime( Float64 timeInUs ) { clock.SetTimeInUs( (UInt64) ( ToLocalUs( timeInUs ) + 0.5 ) ); }

    /** Frames of the device for a span of 'frames' periods of simulated time, with the drift of the unit's crystal */
    UInt32 FramesFor( UInt32 frames, Float64& carry ) const
    {
        carry += (Float64) frames * LocalScale();
        const UInt32 whole = (UInt32) floor( carry );
        carry -= (Float64) whole;
        return whole;
    }

    /** The microphone during one step: the signal and the echo, rendered through the capture device */
    void Capture( UInt32 periodFrames )
    {
        if ( GUCEF_NULL == m_captureDevice )
            return;
        CaptureFrames( FramesFor( periodFrames, m_captureCarry ) );
    }

    void CaptureFrames( UInt32 frames )
    {
        if ( GUCEF_NULL == m_captureDevice )
            return;
        for ( UInt32 i=0; i<frames; ++i )
        {
            const UInt64 n = m_capturePosition + i;
            Float32 value = GUCEF_NULL != m_signal ? m_signal->Sample( n ) : 0.0f;
            if ( m_config.echoGain != 0.0 )
            {
                // the speaker plays output m at local time ( m + period ) / rate; the microphone hears it the echo delay later
                const Int64 m = (Int64) n - (Int64) m_config.period - (Int64) m_config.echoDelayFrames;
                if ( m >= 0 && (UInt64) m < m_outputPosition && m_outputPosition - (UInt64) m <= (UInt64) OutputRingFrames )
                    value += (Float32) ( m_config.echoGain * (Float64) m_ring[ (size_t) ( (UInt64) m % OutputRingFrames ) ] );
            }
            m_feed->Push( value );
        }
        m_captureDevice->Render( frames );
        m_capturePosition += frames;
        m_capturedLast = frames;
    }

    /** Takes the frames the sender made, with the simulated time at which each was complete */
    void PopFrames( TSentFrames& frames )
    {
        if ( GUCEF_NULL == sender )
            return;
        const AUDIO::CVoiceCodecConfig& codec = sender->GetCodecConfig();
        const UInt32 frameSize = codec.GetFrameSizeInFrames();
        UInt32 timestampRate = codec.GetSampleRate();
        if ( codec.GetCodec() == "opus" )
            timestampRate = 48000;     // RFC 7587
        SSentFrame sent;
        while ( sender->PopFrame( sent.frame ) )
        {
            const UInt32 units = sent.frame.GetTimestamp() - sender->GetConfig().GetFirstTimestamp();
            const Float64 position = (Float64) m_senderBase + (Float64) units * (Float64) codec.GetSampleRate() / (Float64) timestampRate + (Float64) frameSize;
            sent.sendTimeInUs = position * 1000000.0 / ( (Float64) m_config.sampleRate * LocalScale() );
            frames.push_back( sent );
        }
    }

    /** The speaker during one step: the mix rendered through the playback device */
    UInt32 Play( UInt32 periodFrames )
    {
        const UInt32 frames = FramesFor( periodFrames, m_playCarry );
        m_playback->Render( frames );
        const AUDIO::CAudioBuffer& rendered = m_playback->GetRenderedAudio();
        const Float32* data = static_cast< const Float32* >( rendered.GetDataPtr( 0 ) );
        const UInt32 count = (UInt32) rendered.GetFrameCount();
        for ( UInt32 i=0; i<count; ++i )
            m_ring[ (size_t) ( ( m_outputPosition + i ) % OutputRingFrames ) ] = data[ i ];
        if ( m_config.keepOutput )
            output.insert( output.end(), data, data + count );
        m_outputPosition += count;
        m_playback->ClearRenderedAudio();
        return count;
    }

    /** Output sample m, while it is in the ring */
    Float32 OutputAt( UInt64 m ) const
    {
        if ( m >= m_outputPosition || m_outputPosition - m > (UInt64) OutputRingFrames )
            return 0.0f;
        return m_ring[ (size_t) ( m % OutputRingFrames ) ];
    }

    UInt64 GetCapturePosition( void ) const { return m_capturePosition; }
    UInt64 GetOutputPosition( void ) const { return m_outputPosition; }
    AUDIO::CNullAudioDevice* GetPlaybackDevice( void ) { return m_playback; }
    AUDIO::CNullAudioDevice* GetCaptureNullDevice( void ) { return m_captureDevice; }

    private:

    CTestIntercomUnit( const CTestIntercomUnit& );
    CTestIntercomUnit& operator=( const CTestIntercomUnit& );

    SUnitConfig m_config;

    public:

    CUnitClock clock;
    AUDIO::CAudioEngine* engine;
    AUDIO::CVoiceSender* sender;
    GUCEF::vector< AUDIO::CVoiceReceiver* > receivers;
    GUCEF::vector< Float32 > output;            /**< what the speaker played, when kept */

    private:

    CTestFeedSource* m_feed;
    AUDIO::TAudioSourcePtr m_feedPtr;
    AUDIO::CNullAudioDevice* m_playback;
    AUDIO::CNullAudioDevice* m_captureDevice;
    const CTestSignal* m_signal;
    UInt64 m_capturePosition;
    UInt64 m_outputPosition;
    Float64 m_captureCarry;
    Float64 m_playCarry;
    UInt64 m_senderBase;
    GUCEF::vector< Float32 > m_ring;
    UInt32 m_capturedLast;
};

/*-------------------------------------------------------------------------*/

}; /* namespace VoiceLinkTest */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIO_TESTAPP_TESTAUDIOINTERCOMUNIT_H ? */
