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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <math.h>
#include <string.h>

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CPLUGINCONTROL_H
#include "CPluginControl.h"
#define GUCEF_CORE_CPLUGINCONTROL_H
#endif /* GUCEF_CORE_CPLUGINCONTROL_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_MT_CACTIVEOBJECT_H
#include "gucefMT_CActiveObject.h"
#define GUCEF_MT_CACTIVEOBJECT_H
#endif /* GUCEF_MT_CACTIVEOBJECT_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_MT_CTATOMIC_H
#include "gucefMT_CTAtomic.h"
#define GUCEF_MT_CTATOMIC_H
#endif /* GUCEF_MT_CTATOMIC_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_AUDIO_CVOICEFRAME_H
#include "gucefAUDIO_CVoiceFrame.h"
#define GUCEF_AUDIO_CVOICEFRAME_H
#endif /* GUCEF_AUDIO_CVOICEFRAME_H ? */

#include "TestAudioVoiceCodecSupport.h"
#include "TestAudioVoiceCodecOpus.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

using namespace GUCEF;
using namespace VoiceCodecTest;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

const char* PluginGroup = "audiovoicecodecopus";

typedef GUCEF::vector< UInt8 > TBytes;
typedef GUCEF::vector< TBytes > TPackets;

/*-------------------------------------------------------------------------*/

AUDIO::CVoiceCodecRegistry&
Registry( void )
{
    return AUDIO::CAudioGlobal::Instance()->GetVoiceCodecRegistry();
}

/*-------------------------------------------------------------------------*/

bool
IsOpusRegistered( void )
{
    AUDIO::CVoiceCodecRegistry::TRegisteredFactoryPtr factory;
    return Registry().TryFindFactory( "opus", factory );
}

/*-------------------------------------------------------------------------*/

bool
LoadOpusPlugin( void )
{
    return LoadVoiceCodecPlugin( "audiovoicecodecOPUS", PluginGroup, "opus" );
}

/*-------------------------------------------------------------------------*/

AUDIO::CVoiceCodecConfig
MakeOpusConfig( AUDIO::TVoiceCodecProfile profile, UInt32 sampleRate, UInt32 frameDurationInUs, UInt32 bitrate )
{
    AUDIO::CVoiceCodecConfig config = MakeConfig( "opus", sampleRate, frameDurationInUs );
    config.SetProfile( profile );
    config.SetBitrate( bitrate );
    if ( AUDIO::VOICECODECPROFILE_ROBUST == profile )
    {
        config.SetInbandFec( true );
        config.SetExpectedLossPercentage( 10 );
    }
    return config;
}

/*-------------------------------------------------------------------------*/

bool
MakeOpus( const AUDIO::CVoiceCodecConfig& config, AUDIO::TVoiceEncoderPtr& encoder, AUDIO::TVoiceDecoderPtr& decoder )
{
    return Registry().CreateEncoder( config, encoder ) && Registry().CreateDecoder( config, decoder ) && !encoder.IsNULL() && !decoder.IsNULL();
}

/*-------------------------------------------------------------------------*/

/** Encodes whole frames of the signal. False when a frame fails to encode */
bool
EncodeAll( AUDIO::CVoiceEncoder& encoder, const TSamples& input, TPackets& packets )
{
    const UInt32 frameSize = encoder.GetFrameSizeInFrames();
    const size_t frames = input.size() / frameSize;
    packets.clear();
    UInt8 payload[ AUDIO::CVoiceFrame::MaxPayloadSize ];
    for ( size_t f=0; f<frames; ++f )
    {
        const Int32 size = encoder.Encode( &input[ f * frameSize ], payload, sizeof( payload ) );
        if ( size <= 0 || (UInt32) size > encoder.GetMaxPayloadSize() )
            return false;
        packets.push_back( TBytes( payload, payload + size ) );
    }
    return true;
}

/** Decodes the packets in order; a packet in 'lost' is concealed. Returns the output */
TSamples
DecodeAll( AUDIO::CVoiceDecoder& decoder, UInt32 frameSize, const TPackets& packets, const GUCEF::vector< bool >& lost, bool useFec )
{
    TSamples output( packets.size() * frameSize, 0.0f );
    for ( size_t f=0; f<packets.size(); ++f )
    {
        Float32* out = &output[ f * frameSize ];
        if ( lost[ f ] )
        {
            // the way a receiver works: a lost frame is made up when the next one is there, from that one when it can
            if ( useFec && f + 1 < packets.size() && !lost[ f + 1 ] )
                decoder.DecodeFec( &packets[ f + 1 ][ 0 ], (UInt32) packets[ f + 1 ].size(), out, frameSize );
            else
                decoder.Conceal( out, frameSize );
        }
        else
        if ( (Int32) frameSize != decoder.Decode( &packets[ f ][ 0 ], (UInt32) packets[ f ].size(), out, frameSize ) )
            return TSamples();
    }
    return output;
}

/*-------------------------------------------------------------------------*/

#ifdef GUCEF_AUDIO_VOICETEST_RTSAN_BUILD
const bool IsRtsanBuild = true;
#else
const bool IsRtsanBuild = false;
#endif

/*-------------------------------------------------------------------------*/

/** Sets the bit rate over and over from another thread while the test thread encodes */
class CBitrateThread : public MT::CActiveObject
{
    public:

    explicit CBitrateThread( AUDIO::CVoiceEncoder& encoder ) : MT::CActiveObject(), m_encoder( encoder ), done( 0 ) {}

    virtual bool OnThreadStart( void* ) GUCEF_VIRTUAL_OVERRIDE { return true; }

    virtual bool OnThreadCycle( void* ) GUCEF_VIRTUAL_OVERRIDE
    {
        for ( UInt32 i=0; i<3000; ++i )
        {
            m_encoder.SetBitrate( 16000 + ( i % 5 ) * 8000 );
            m_encoder.SetExpectedLossPercentage( i % 40 );
            if ( 0 == ( i & 15 ) )
                MT::ThreadDelay( 0 );
        }
        done.Store( 1 );
        return true;
    }

    virtual const MT::CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NULL; }

    AUDIO::CVoiceEncoder& m_encoder;
    MT::CTAtomic< UInt32 > done;
};

/*-------------------------------------------------------------------------*/

void
PerformSupportTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "AudioVoiceCodecOpus: The plugin registers the voice codec opus and supports the rates and frame sizes of the two profiles" )
    {
        GUCEF_TESTFW_SKIP_IF( !LoadOpusPlugin(), "the Opus voice codec plugin is not available on this platform" );
        try
        {
            AUDIO::CVoiceCodecRegistry::TRegisteredFactoryPtr opus;
            ASSERT_TRUE( Registry().TryFindFactory( "OPUS", opus ) && opus->GetCodecTypeName() == "opus" );

            const UInt32 rates[] = { 8000, 12000, 16000, 24000, 48000 };
            for ( UInt32 r=0; r<5; ++r )
            {
                for ( UInt32 d=0; d<4; ++d )
                {
                    const UInt32 durations[] = { 2500, 5000, 10000, 20000 };
                    ASSERT_TRUE( opus->IsSupported( MakeOpusConfig( AUDIO::VOICECODECPROFILE_LOWDELAY, rates[ r ], durations[ d ], 48000 ) ) );
                    ASSERT_TRUE( opus->IsSupported( MakeOpusConfig( AUDIO::VOICECODECPROFILE_ROBUST, rates[ r ], durations[ d ], 32000 ) ) == ( 3 == d ) );
                }
            }
            ASSERT_FALSE( opus->IsSupported( MakeOpusConfig( AUDIO::VOICECODECPROFILE_LOWDELAY, 44100, 10000, 48000 ) ) );
            ASSERT_FALSE( opus->IsSupported( MakeOpusConfig( AUDIO::VOICECODECPROFILE_LOWDELAY, 32000, 10000, 48000 ) ) );
            ASSERT_FALSE( opus->IsSupported( MakeOpusConfig( AUDIO::VOICECODECPROFILE_LOWDELAY, 48000, 15000, 48000 ) ) );
            ASSERT_FALSE( opus->IsSupported( MakeOpusConfig( AUDIO::VOICECODECPROFILE_LOWDELAY, 48000, 40000, 48000 ) ) );
            AUDIO::CVoiceCodecConfig stereo = MakeOpusConfig( AUDIO::VOICECODECPROFILE_LOWDELAY, 48000, 10000, 48000 );
            stereo.SetChannels( 2 );
            ASSERT_FALSE( opus->IsSupported( stereo ) );

            // the profile defaults of the configuration are supported
            AUDIO::CVoiceCodecConfig defaults;
            ASSERT_TRUE( opus->IsSupported( defaults ) && 111 == defaults.GetPayloadType() );
            defaults.SetProfileDefaults( AUDIO::VOICECODECPROFILE_ROBUST );
            ASSERT_TRUE( opus->IsSupported( defaults ) && 20000 == defaults.GetFrameDurationInUs() && 32000 == defaults.GetBitrate() && defaults.GetInbandFec() && 10 == defaults.GetExpectedLossPercentage() );
            defaults.SetProfileDefaults( AUDIO::VOICECODECPROFILE_LOWDELAY );
            ASSERT_TRUE( opus->IsSupported( defaults ) && 10000 == defaults.GetFrameDurationInUs() && 48000 == defaults.GetBitrate() && !defaults.GetInbandFec() && 0 == defaults.GetExpectedLossPercentage() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioVoiceCodecOpus: The encoder reports a timestamp rate of 48 kHz, the delay of libopus per profile and a payload limit that fits a voice frame" )
    {
        GUCEF_TESTFW_SKIP_IF( !LoadOpusPlugin(), "the Opus voice codec plugin is not available on this platform" );
        try
        {
            const UInt32 rates[] = { 8000, 16000, 24000, 48000 };
            for ( UInt32 r=0; r<4; ++r )
            {
                AUDIO::TVoiceEncoderPtr encoder;
                AUDIO::TVoiceDecoderPtr decoder;
                AUDIO::CVoiceCodecConfig low = MakeOpusConfig( AUDIO::VOICECODECPROFILE_LOWDELAY, rates[ r ], 10000, 48000 );
                ASSERT_TRUE( MakeOpus( low, encoder, decoder ) );
                ASSERT_TRUE( 48000 == encoder->GetTimestampRate() && rates[ r ] == decoder->GetOutputRate() );
                ASSERT_TRUE( rates[ r ] / 100 == encoder->GetFrameSizeInFrames() );
                // frame plus 2.5 ms: the lookahead is the 2.5 ms
                ASSERT_TRUE( encoder->GetLookaheadInFrames() == rates[ r ] / 400 );
                ASSERT_TRUE( encoder->GetMaxPayloadSize() <= AUDIO::CVoiceFrame::MaxPayloadSize && encoder->GetMaxPayloadSize() >= 1275 );
                ASSERT_TRUE( decoder->SupportsFec() );

                AUDIO::CVoiceCodecConfig robust = MakeOpusConfig( AUDIO::VOICECODECPROFILE_ROBUST, rates[ r ], 20000, 32000 );
                AUDIO::TVoiceEncoderPtr robustEncoder;
                AUDIO::TVoiceDecoderPtr robustDecoder;
                ASSERT_TRUE( MakeOpus( robust, robustEncoder, robustDecoder ) );
                // 6.5 ms: 2.5 ms of CELT lookahead and 4 ms of delay compensation
                ASSERT_TRUE( robustEncoder->GetLookaheadInFrames() == rates[ r ] * 13 / 2000 );
                ASSERT_TRUE( robustEncoder->GetConfig() == robust );
            }
        }
        catch( ... )
        {
            ERRORHERE;
        }
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
PerformRoundTripTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "AudioVoiceCodecOpus: Round trips of a speech-like signal reach 15 dB SNR at 48 kbit/s at the delay the encoder reports (low delay profile), and the robust profile keeps the signal in time, at all frame sizes" )
    {
        GUCEF_TESTFW_SKIP_IF( !LoadOpusPlugin(), "the Opus voice codec plugin is not available on this platform" );
        try
        {
            const TSamples input = MakeSpeechLike( 48000 * 3, 48000, 0.4f );
            struct { AUDIO::TVoiceCodecProfile profile; UInt32 duration; } variants[] =
            {
                { AUDIO::VOICECODECPROFILE_LOWDELAY, 2500 }, { AUDIO::VOICECODECPROFILE_LOWDELAY, 5000 },
                { AUDIO::VOICECODECPROFILE_LOWDELAY, 10000 }, { AUDIO::VOICECODECPROFILE_LOWDELAY, 20000 },
                { AUDIO::VOICECODECPROFILE_ROBUST, 20000 }
            };
            for ( UInt32 v=0; v<5; ++v )
            {
                AUDIO::TVoiceEncoderPtr encoder;
                AUDIO::TVoiceDecoderPtr decoder;
                const AUDIO::CVoiceCodecConfig config = MakeOpusConfig( variants[ v ].profile, 48000, variants[ v ].duration, 48000 );
                ASSERT_TRUE( MakeOpus( config, encoder, decoder ) );

                TPackets packets;
                ASSERT_TRUE( EncodeAll( *encoder, input, packets ) );
                const UInt32 frameSize = encoder->GetFrameSizeInFrames();
                GUCEF::vector< bool > lost( packets.size(), false );
                const TSamples output = DecodeAll( *decoder, frameSize, packets, lost, false );
                ASSERT_TRUE( output.size() == packets.size() * frameSize && AllFinite( output ) );

                // after the start-up of the codec: the best delay and the SNR at it
                Float64 snr = 0.0;
                const size_t from = 48000 / 2, to = 48000 * 2;
                const size_t delay = BestDelay( input, output, 700, from, to, snr );
                UInt64 bytes = 0;
                for ( size_t p=0; p<packets.size(); ++p )
                    bytes += packets[ p ].size();
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioVoiceCodecOpus: " << AUDIO::CVoiceCodecConfig::ProfileToString( variants[ v ].profile ) << ", " << variants[ v ].duration << " us frames: SNR " << snr <<
                                                  " dB at a delay of " << delay << " samples (reported lookahead " << encoder->GetLookaheadInFrames() << "), " << ( bytes * 8 * 48000 / ( packets.size() * frameSize ) ) << " bit/s" );
                if ( AUDIO::VOICECODECPROFILE_LOWDELAY == variants[ v ].profile )
                {
                    // CELT keeps the waveform: SNR of the design and the delay is exactly the reported lookahead
                    ASSERT_TRUE( snr >= 15.0 );
                    ASSERT_TRUE( delay == encoder->GetLookaheadInFrames() );
                }
                else
                {
                    // SILK shapes the noise perceptually and does not keep the waveform: the signal is there, in time (within 8 samples of the reported delay)
                    ASSERT_TRUE( snr >= 3.0 );
                    ASSERT_TRUE( delay + 8 >= encoder->GetLookaheadInFrames() && delay <= encoder->GetLookaheadInFrames() + 8 );
                }
            }
        }
        catch( ... )
        {
            ERRORHERE;
        }
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioVoiceCodecOpus: A 1 kHz tone keeps its level within 0.5 dB through both profiles and at every coding rate" )
    {
        GUCEF_TESTFW_SKIP_IF( !LoadOpusPlugin(), "the Opus voice codec plugin is not available on this platform" );
        try
        {
            const UInt32 rates[] = { 8000, 16000, 48000 };
            for ( UInt32 r=0; r<3; ++r )
            {
                for ( UInt32 p=0; p<2; ++p )
                {
                    const AUDIO::TVoiceCodecProfile profile = 0 == p ? AUDIO::VOICECODECPROFILE_LOWDELAY : AUDIO::VOICECODECPROFILE_ROBUST;
                    AUDIO::TVoiceEncoderPtr encoder;
                    AUDIO::TVoiceDecoderPtr decoder;
                    ASSERT_TRUE( MakeOpus( MakeOpusConfig( profile, rates[ r ], 20000, 48000 ), encoder, decoder ) );
                    const TSamples input = AudioEffectTest::MakeSine( rates[ r ] * 2, 1, 1000.0, 0.3f, (Float64) rates[ r ] );
                    TPackets packets;
                    ASSERT_TRUE( EncodeAll( *encoder, input, packets ) );
                    GUCEF::vector< bool > lost( packets.size(), false );
                    const TSamples output = DecodeAll( *decoder, encoder->GetFrameSizeInFrames(), packets, lost, false );
                    const Float64 inLevel = Rms( input, rates[ r ] / 2, rates[ r ] * 3 / 2 );
                    const Float64 outLevel = Rms( output, rates[ r ] / 2, rates[ r ] * 3 / 2 );
                    ASSERT_TRUE( fabs( 20.0 * log10( outLevel / inLevel ) ) < 0.5 );
                }
            }
        }
        catch( ... )
        {
            ERRORHERE;
        }
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioVoiceCodecOpus: Frames at every supported coding rate decode to the frame size, and a reset starts the stream again" )
    {
        GUCEF_TESTFW_SKIP_IF( !LoadOpusPlugin(), "the Opus voice codec plugin is not available on this platform" );
        try
        {
            const UInt32 rates[] = { 8000, 12000, 16000, 24000, 48000 };
            for ( UInt32 r=0; r<5; ++r )
            {
                AUDIO::TVoiceEncoderPtr encoder;
                AUDIO::TVoiceDecoderPtr decoder;
                ASSERT_TRUE( MakeOpus( MakeOpusConfig( AUDIO::VOICECODECPROFILE_LOWDELAY, rates[ r ], 10000, 32000 ), encoder, decoder ) );
                const TSamples input = MakeSpeechLike( rates[ r ], rates[ r ], 0.4f );
                TPackets packets;
                ASSERT_TRUE( EncodeAll( *encoder, input, packets ) );
                ASSERT_TRUE( packets.size() == 100 );
                GUCEF::vector< bool > lost( packets.size(), false );
                const TSamples first = DecodeAll( *decoder, rates[ r ] / 100, packets, lost, false );
                ASSERT_TRUE( first.size() == input.size() );

                // after a reset of both the stream is the same stream again
                encoder->Reset();
                decoder->Reset();
                TPackets again;
                ASSERT_TRUE( EncodeAll( *encoder, input, again ) );
                ASSERT_TRUE( again.size() == packets.size() );
                bool same = true;
                for ( size_t p=0; p<packets.size(); ++p )
                    same = same && packets[ p ] == again[ p ];
                ASSERT_TRUE( same );
                const TSamples second = DecodeAll( *decoder, rates[ r ] / 100, again, lost, false );
                ASSERT_TRUE( second.size() == first.size() && 0 == memcmp( &first[ 0 ], &second[ 0 ], first.size() * sizeof( Float32 ) ) );
            }
        }
        catch( ... )
        {
            ERRORHERE;
        }
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
PerformLossTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "AudioVoiceCodecOpus: Concealment gives finite audio that fades out over a long loss" )
    {
        GUCEF_TESTFW_SKIP_IF( !LoadOpusPlugin(), "the Opus voice codec plugin is not available on this platform" );
        try
        {
            for ( UInt32 p=0; p<2; ++p )
            {
                const AUDIO::TVoiceCodecProfile profile = 0 == p ? AUDIO::VOICECODECPROFILE_LOWDELAY : AUDIO::VOICECODECPROFILE_ROBUST;
                AUDIO::TVoiceEncoderPtr encoder;
                AUDIO::TVoiceDecoderPtr decoder;
                ASSERT_TRUE( MakeOpus( MakeOpusConfig( profile, 48000, 20000, 48000 ), encoder, decoder ) );
                const TSamples input = MakeHarmonic( 48000, 140.0, 48000, 0.4f );
                TPackets packets;
                ASSERT_TRUE( EncodeAll( *encoder, input, packets ) );
                GUCEF::vector< bool > lost( packets.size(), false );
                // the last 20 frames (400 ms) are lost
                for ( size_t f=packets.size()-20; f<packets.size(); ++f )
                    lost[ f ] = true;
                const TSamples output = DecodeAll( *decoder, 960, packets, lost, false );
                ASSERT_TRUE( output.size() == packets.size() * 960 && AllFinite( output ) );
                const size_t start = ( packets.size() - 20 ) * 960;
                const Float64 before = Rms( output, start - 4800, start );
                const Float64 early = Rms( output, start, start + 960 );
                const Float64 late = Rms( output, output.size() - 960 * 4, output.size() );
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioVoiceCodecOpus: concealment " << AUDIO::CVoiceCodecConfig::ProfileToString( profile ) << ": level before " << before << ", first lost frame " << early << ", last four lost frames " << late );
                ASSERT_TRUE( before > 0.05 && early > 0.2 * before );
                ASSERT_TRUE( late < early * 0.5 );
                ASSERT_TRUE( PeakOfRange( output, 0, output.size() ) < 1.0f );
            }
        }
        catch( ... )
        {
            ERRORHERE;
        }
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioVoiceCodecOpus: With every fifth packet lost the robust profile recovers the frames from the next packet better than concealment does" )
    {
        GUCEF_TESTFW_SKIP_IF( !LoadOpusPlugin(), "the Opus voice codec plugin is not available on this platform" );
        try
        {
            const TSamples input = MakeSpeechLike( 48000 * 6, 48000, 0.4f );
            AUDIO::CVoiceCodecConfig config = MakeOpusConfig( AUDIO::VOICECODECPROFILE_ROBUST, 48000, 20000, 32000 );
            config.SetExpectedLossPercentage( 20 );

            AUDIO::TVoiceEncoderPtr encoder;
            AUDIO::TVoiceDecoderPtr fecDecoder;
            AUDIO::TVoiceDecoderPtr concealDecoder;
            ASSERT_TRUE( MakeOpus( config, encoder, fecDecoder ) );
            ASSERT_TRUE( Registry().CreateDecoder( config, concealDecoder ) );
            ASSERT_TRUE( concealDecoder->SupportsFec() );

            TPackets packets;
            ASSERT_TRUE( EncodeAll( *encoder, input, packets ) );
            GUCEF::vector< bool > lost( packets.size(), false );
            for ( size_t f=4; f<packets.size()-1; f+=5 )
                lost[ f ] = true;

            const TSamples withFec = DecodeAll( *fecDecoder, 960, packets, lost, true );
            const TSamples withConceal = DecodeAll( *concealDecoder, 960, packets, lost, false );
            GUCEF::vector< bool > none( packets.size(), false );
            AUDIO::TVoiceDecoderPtr cleanDecoder;
            ASSERT_TRUE( Registry().CreateDecoder( config, cleanDecoder ) );
            const TSamples clean = DecodeAll( *cleanDecoder, 960, packets, none, false );
            ASSERT_TRUE( withFec.size() == clean.size() && withConceal.size() == clean.size() );

            // what the lost frames come to, against the decoded stream without loss (the delay of the codec is in both)
            Float64 errorFec = 0.0, errorConceal = 0.0, energy = 1.0e-30;
            for ( size_t f=10; f<packets.size()-1; ++f )
            {
                if ( !lost[ f ] )
                    continue;
                for ( size_t n=f*960; n<(f+1)*960; ++n )
                {
                    const Float64 c = clean[ n ];
                    errorFec += ( withFec[ n ] - c ) * ( withFec[ n ] - c );
                    errorConceal += ( withConceal[ n ] - c ) * ( withConceal[ n ] - c );
                    energy += c * c;
                }
            }
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioVoiceCodecOpus: lost frames against the lossless decode: error with FEC " << 10.0 * log10( errorFec / energy + 1.0e-30 ) <<
                                              " dB, with concealment " << 10.0 * log10( errorConceal / energy + 1.0e-30 ) << " dB" );
            ASSERT_TRUE( errorFec < errorConceal );
            ASSERT_TRUE( errorFec < 0.8 * errorConceal );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
PerformSettingsTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "AudioVoiceCodecOpus: A bit rate published to the encoder shows in the payload sizes from the next frame on, and survives a reset" )
    {
        GUCEF_TESTFW_SKIP_IF( !LoadOpusPlugin(), "the Opus voice codec plugin is not available on this platform" );
        try
        {
            AUDIO::TVoiceEncoderPtr encoder;
            AUDIO::TVoiceDecoderPtr decoder;
            ASSERT_TRUE( MakeOpus( MakeOpusConfig( AUDIO::VOICECODECPROFILE_LOWDELAY, 48000, 10000, 48000 ), encoder, decoder ) );
            const TSamples input = MakeSpeechLike( 48000 * 4, 48000, 0.4f, 777 );

            const UInt32 bitrates[] = { 16000, 64000, 24000 };
            UInt64 average[ 3 ] = { 0, 0, 0 };
            for ( UInt32 b=0; b<3; ++b )
            {
                encoder->SetBitrate( bitrates[ b ] );
                UInt64 bytes = 0;
                UInt32 frames = 0;
                UInt8 payload[ 1500 ];
                for ( UInt32 f=0; f<100; ++f )
                {
                    const Int32 size = encoder->Encode( &input[ ( (size_t) b * 100 + f ) * 480 ], payload, sizeof( payload ) );
                    ASSERT_TRUE( size > 0 );
                    if ( f >= 10 )          // the encoder moves to a new rate within a few frames
                    {
                        bytes += (UInt32) size;
                        ++frames;
                    }
                }
                average[ b ] = bytes * 8 * 100 / frames;       // bit/s at 100 frames per second
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioVoiceCodecOpus: target " << bitrates[ b ] << " bit/s gives " << average[ b ] << " bit/s" );
            }
            ASSERT_TRUE( average[ 0 ] < average[ 1 ] && average[ 2 ] < average[ 1 ] && average[ 0 ] < average[ 2 ] );
            ASSERT_TRUE( average[ 1 ] > 2 * average[ 0 ] );
            ASSERT_TRUE( average[ 0 ] < 24000 );

            // after a reset the rate in force stays
            encoder->SetBitrate( 16000 );
            encoder->Reset();
            UInt8 payload[ 1500 ];
            UInt64 bytes = 0;
            for ( UInt32 f=0; f<100; ++f )
            {
                const Int32 size = encoder->Encode( &input[ (size_t) f * 480 ], payload, sizeof( payload ) );
                ASSERT_TRUE( size > 0 );
                if ( f >= 10 )
                    bytes += (UInt32) size;
            }
            ASSERT_TRUE( bytes * 8 * 100 / 90 < 24000 );

            // out of range values are limited, not refused
            encoder->SetBitrate( 0 );
            encoder->SetBitrate( 4000000 );
            encoder->SetExpectedLossPercentage( 400 );
            ASSERT_TRUE( encoder->Encode( &input[ 0 ], payload, sizeof( payload ) ) > 0 );
            ASSERT_TRUE( encoder->Encode( &input[ 480 ], payload, 1 ) < 0 );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioVoiceCodecOpus: The expected loss reaches the robust encoder and its packets still decode" )
    {
        GUCEF_TESTFW_SKIP_IF( !LoadOpusPlugin(), "the Opus voice codec plugin is not available on this platform" );
        try
        {
            AUDIO::TVoiceEncoderPtr encoder;
            AUDIO::TVoiceDecoderPtr decoder;
            AUDIO::CVoiceCodecConfig config = MakeOpusConfig( AUDIO::VOICECODECPROFILE_ROBUST, 48000, 20000, 32000 );
            config.SetExpectedLossPercentage( 0 );
            ASSERT_TRUE( MakeOpus( config, encoder, decoder ) );
            const TSamples input = MakeSpeechLike( 48000 * 4, 48000, 0.4f, 31 );

            UInt64 bytesAt[ 2 ] = { 0, 0 };
            UInt8 payload[ 1500 ];
            TSamples out( 960, 0.0f );
            for ( UInt32 round=0; round<2; ++round )
            {
                encoder->SetExpectedLossPercentage( 0 == round ? 0 : 25 );
                for ( UInt32 f=0; f<100; ++f )
                {
                    const Int32 size = encoder->Encode( &input[ ( (size_t) round * 100 + f ) * 960 ], payload, sizeof( payload ) );
                    ASSERT_TRUE( size > 0 );
                    ASSERT_TRUE( 960 == decoder->Decode( payload, (UInt32) size, &out[ 0 ], 960 ) );
                    if ( f >= 20 )
                        bytesAt[ round ] += (UInt32) size;
                }
            }
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioVoiceCodecOpus: robust profile packet bytes over 80 frames: expected loss 0 % " << bytesAt[ 0 ] << ", 25 % " << bytesAt[ 1 ] );
            ASSERT_TRUE( bytesAt[ 0 ] > 0 && bytesAt[ 1 ] > 0 );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioVoiceCodecOpus: Settings published from another thread while encoding are safe (ThreadSanitizer watches)" )
    {
        GUCEF_TESTFW_SKIP_IF( !LoadOpusPlugin(), "the Opus voice codec plugin is not available on this platform" );
        try
        {
            AUDIO::TVoiceEncoderPtr encoder;
            AUDIO::TVoiceDecoderPtr decoder;
            ASSERT_TRUE( MakeOpus( MakeOpusConfig( AUDIO::VOICECODECPROFILE_ROBUST, 48000, 20000, 32000 ), encoder, decoder ) );
            const TSamples input = MakeSpeechLike( 48000 * 4, 48000, 0.4f, 5 );

            CBitrateThread* thread = GUCEF_NEW CBitrateThread( *encoder );
            ASSERT_TRUE( thread->Activate() );
            UInt8 payload[ 1500 ];
            TSamples out( 960, 0.0f );
            bool isOk = true;
            for ( UInt32 f=0; f<200; ++f )
            {
                const Int32 size = encoder->Encode( &input[ (size_t) f * 960 ], payload, sizeof( payload ) );
                isOk = isOk && size > 0 && 960 == decoder->Decode( payload, (UInt32) size, &out[ 0 ], 960 );
                MT::ThreadDelay( 0 );
            }
            for ( UInt32 i=0; i<10000 && 0 == thread->done.Load(); ++i )
                MT::ThreadDelay( 1 );
            ASSERT_TRUE( 0 != thread->done.Load() );
            thread->Deactivate( false, true );
            GUCEF_DELETE thread;
            ASSERT_TRUE( isOk );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
PerformHostileInputTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "AudioVoiceCodecOpus: Payloads that are no Opus frame, or do not fit, are refused or decoded to a bounded frame (ASan and UBSan watch)" )
    {
        GUCEF_TESTFW_SKIP_IF( !LoadOpusPlugin(), "the Opus voice codec plugin is not available on this platform" );
        try
        {
            AUDIO::TVoiceEncoderPtr encoder;
            AUDIO::TVoiceDecoderPtr decoder;
            ASSERT_TRUE( MakeOpus( MakeOpusConfig( AUDIO::VOICECODECPROFILE_LOWDELAY, 48000, 10000, 48000 ), encoder, decoder ) );
            TSamples out( 480, 0.0f );
            UInt8 payload[ 1500 ];
            memset( payload, 0, sizeof( payload ) );
            ASSERT_TRUE( decoder->Decode( payload, 0, &out[ 0 ], 480 ) < 0 );
            ASSERT_TRUE( decoder->DecodeFec( payload, 0, &out[ 0 ], 480 ) < 0 );

            // a real frame into a buffer that is too small
            const TSamples input = MakeHarmonic( 480, 150.0, 48000, 0.4f );
            const Int32 size = encoder->Encode( &input[ 0 ], payload, sizeof( payload ) );
            ASSERT_TRUE( size > 0 );
            ASSERT_TRUE( decoder->Decode( payload, (UInt32) size, &out[ 0 ], 240 ) < 0 );
            ASSERT_TRUE( 480 == decoder->Decode( payload, (UInt32) size, &out[ 0 ], 480 ) );
            // concealment must be a multiple of 2.5 ms
            ASSERT_TRUE( decoder->Conceal( &out[ 0 ], 100 ) < 0 );
            ASSERT_TRUE( 480 == decoder->Conceal( &out[ 0 ], 480 ) );

            // random payloads of random lengths and shapes
            AudioEffectTest::CXorShift random( 424242 );
            TSamples big( 5760 + 480, 0.0f );
            for ( UInt32 round=0; round<3000; ++round )
            {
                const UInt32 length = 1 + random.Next() % 1400;
                for ( UInt32 i=0; i<length; ++i )
                    payload[ i ] = (UInt8) random.Next();
                const Int32 got = decoder->Decode( payload, length, &big[ 0 ], 5760 );
                ASSERT_TRUE( got <= 5760 );
                ASSERT_TRUE( AllFinite( big ) );
            }

            // corrupted real frames
            UInt8 original[ 1500 ];
            const Int32 originalSize = encoder->Encode( &input[ 0 ], original, sizeof( original ) );
            for ( UInt32 round=0; round<2000; ++round )
            {
                memcpy( payload, original, (size_t) originalSize );
                payload[ random.Next() % (UInt32) originalSize ] = (UInt8) random.Next();
                const Int32 got = decoder->Decode( payload, (UInt32) originalSize, &big[ 0 ], 5760 );
                ASSERT_TRUE( got <= 5760 );
            }
        }
        catch( ... )
        {
            ERRORHERE;
        }
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
PerformRealtimeTests( void )
{GUCEF_TRACE;

    // libopus is built with USE_ALLOCA: its scratch memory is on the stack, so the codec calls allocate nothing
    GUCEF_TESTFW_TESTCASE( "AudioVoiceCodecOpus: Encode, Decode, Conceal, DecodeFec and Reset do nothing a real time thread must not do (RealtimeSanitizer build)" )
    {
        if ( !IsRtsanBuild )
        {
            GUCEF_TESTFW_SKIP( "not a RealtimeSanitizer build (-fsanitize=realtime)" );
        }
        GUCEF_TESTFW_SKIP_IF( GUCEF_TESTFW_GETENV( "GUCEF_TEST_AUDIO_RTSAN_CLEAN" ) == "0", "switched off with GUCEF_TEST_AUDIO_RTSAN_CLEAN=0" );
        GUCEF_TESTFW_SKIP_IF( !LoadOpusPlugin(), "the Opus voice codec plugin is not available on this platform" );
        try
        {
            const TSamples input = MakeSpeechLike( 48000 * 2, 48000, 0.4f );
            for ( UInt32 p=0; p<2; ++p )
            {
                const AUDIO::TVoiceCodecProfile profile = 0 == p ? AUDIO::VOICECODECPROFILE_LOWDELAY : AUDIO::VOICECODECPROFILE_ROBUST;
                const UInt32 duration = 0 == p ? 10000 : 20000;
                AUDIO::TVoiceEncoderPtr encoder;
                AUDIO::TVoiceDecoderPtr decoder;
                ASSERT_TRUE( MakeOpus( MakeOpusConfig( profile, 48000, duration, 32000 ), encoder, decoder ) );
                const UInt32 frameSize = encoder->GetFrameSizeInFrames();

                UInt8 payload[ 1500 ];
                UInt8 previous[ 1500 ];
                UInt32 previousSize = 0;
                TSamples out( frameSize, 0.0f );
                for ( UInt32 f=0; f<80; ++f )
                {
                    if ( 20 == f )
                    {
                        encoder->SetBitrate( 24000 );               // published from this thread, applied inside Encode()
                        encoder->SetExpectedLossPercentage( 15 );
                    }
                    const Int32 size = EncodeRealtime( *encoder, &input[ (size_t) f * frameSize ], payload, sizeof( payload ) );
                    ASSERT_TRUE( size > 0 );
                    if ( 30 == f || 31 == f )
                    {
                        ASSERT_TRUE( (Int32) frameSize == ConcealRealtime( *decoder, &out[ 0 ], frameSize ) );
                    }
                    else
                    if ( 50 == f && previousSize > 0 )
                    {
                        ASSERT_TRUE( (Int32) frameSize == DecodeFecRealtime( *decoder, payload, (UInt32) size, &out[ 0 ], frameSize ) );
                        ASSERT_TRUE( (Int32) frameSize == DecodeRealtime( *decoder, payload, (UInt32) size, &out[ 0 ], frameSize ) );
                    }
                    else
                    {
                        ASSERT_TRUE( (Int32) frameSize == DecodeRealtime( *decoder, payload, (UInt32) size, &out[ 0 ], frameSize ) );
                    }
                    memcpy( previous, payload, (size_t) size );
                    previousSize = (UInt32) size;
                }
                ResetRealtime( *encoder, *decoder );
                const Int32 size = EncodeRealtime( *encoder, &input[ 0 ], payload, sizeof( payload ) );
                ASSERT_TRUE( size > 0 && (Int32) frameSize == DecodeRealtime( *decoder, payload, (UInt32) size, &out[ 0 ], frameSize ) );
            }
        }
        catch( ... )
        {
            ERRORHERE;
        }
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
PerformPluginLifetimeTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "AudioVoiceCodecOpus: The codec stays registered while an encoder or decoder lives and goes with the plugin when none do" )
    {
        GUCEF_TESTFW_SKIP_IF( !LoadOpusPlugin(), "the Opus voice codec plugin is not available on this platform" );
        try
        {
            AUDIO::TVoiceEncoderPtr encoder;
            AUDIO::TVoiceDecoderPtr decoder;
            ASSERT_TRUE( MakeOpus( MakeOpusConfig( AUDIO::VOICECODECPROFILE_LOWDELAY, 48000, 10000, 48000 ), encoder, decoder ) );

            AUDIO::CVoiceCodecRegistry::TRegisteredFactoryPtr factory;
            ASSERT_TRUE( Registry().TryFindFactory( "opus", factory ) && 2 == factory->GetLiveInstanceCount() );
            ASSERT_FALSE( Registry().TryUnregisterFactory( "opus" ) );
            ASSERT_TRUE( IsOpusRegistered() );
            encoder.Unlink();
            ASSERT_FALSE( Registry().TryUnregisterFactory( "opus" ) );
            decoder.Unlink();
            ASSERT_TRUE( 0 == factory->GetLiveInstanceCount() );
            factory.Unlink();

            // now the plugin can go
            ASSERT_TRUE( CORE::CCoreGlobal::Instance()->GetPluginControl().UnloadPluginGroup( PluginGroup ) );
            ASSERT_FALSE( IsOpusRegistered() );

            // and come back
            ASSERT_TRUE( LoadOpusPlugin() );
            AUDIO::TVoiceEncoderPtr again;
            ASSERT_TRUE( Registry().CreateEncoder( MakeOpusConfig( AUDIO::VOICECODECPROFILE_LOWDELAY, 16000, 5000, 24000 ), again ) );
            again.Unlink();
            ASSERT_TRUE( CORE::CCoreGlobal::Instance()->GetPluginControl().UnloadPluginGroup( PluginGroup ) );
            ASSERT_FALSE( IsOpusRegistered() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    }
    GUCEF_TESTFW_TESTCASE_END
}

}

/*-------------------------------------------------------------------------*/

void
PerformAudioVoiceCodecOpusTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING AudioVoiceCodecOpus TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "gucefAUDIO.VoiceCodecOpus" );

    PerformSupportTests();
    PerformRoundTripTests();
    PerformLossTests();
    PerformSettingsTests();
    PerformHostileInputTests();
    PerformRealtimeTests();
    PerformPluginLifetimeTests();
}

/*-------------------------------------------------------------------------*/
