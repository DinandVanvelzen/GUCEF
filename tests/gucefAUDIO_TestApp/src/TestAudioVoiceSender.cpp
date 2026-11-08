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

#ifndef GUCEFAUDIO_H
#include "gucefAUDIO.h"
#define GUCEFAUDIO_H
#endif /* GUCEFAUDIO_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_MT_CACTIVEOBJECT_H
#include "gucefMT_CActiveObject.h"
#define GUCEF_MT_CACTIVEOBJECT_H
#endif /* GUCEF_MT_CACTIVEOBJECT_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestAudioVoiceStreamSupport.h"
#include "TestAudioVoiceSender.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

using namespace GUCEF;
using namespace VoiceStreamTest;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

#ifdef GUCEF_AUDIO_VOICETEST_RTSAN_BUILD
const bool IsRtsanBuild = true;
#else
const bool IsRtsanBuild = false;
#endif

const UInt16 FirstSequenceNumber = 1000;
const UInt32 FirstTimestamp = 5000;
const UInt32 StreamId = 0x12345678;

AUDIO::CVoiceSenderConfig
SenderConfig( const AUDIO::CVoiceCodecConfig& codecConfig, AUDIO::TVoiceTransmitMode mode )
{
    AUDIO::CVoiceSenderConfig config;
    config.SetCodecConfig( codecConfig );
    config.SetTransmitMode( mode );
    config.SetStreamId( StreamId );
    config.SetFirstSequenceNumber( FirstSequenceNumber );
    config.SetFirstTimestamp( FirstTimestamp );
    config.SetOutboundQueueFrames( 256 );
    return config;
}

/*-------------------------------------------------------------------------*/

bool
LoadOpus( void )
{
    return VoiceCodecTest::LoadVoiceCodecPlugin( "audiovoicecodecOPUS", "audiovoicecodecopus", "opus" );
}

/*-------------------------------------------------------------------------*/

/**
 *  A codec for the tests of the sender: a payload of 8 bytes holding the frame number and the first sample; frames
 *  listed as errors give -1, frames listed as empty give 0. It notes the bit rate and expected loss it was given
 */
class CScriptedVoiceEncoder : public AUDIO::CVoiceEncoder
{
    public:

    explicit CScriptedVoiceEncoder( const AUDIO::CVoiceCodecConfig& config )
        : AUDIO::CVoiceEncoder(), m_config( config ), m_frame( 0 ), bitrate( 0 ), loss( 0 ), errorEvery( 0 ), emptyEvery( 0 ) {}

    virtual const AUDIO::CVoiceCodecConfig& GetConfig( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_config; }
    virtual UInt32 GetMaxPayloadSize( void ) const GUCEF_VIRTUAL_OVERRIDE { return 8; }
    virtual UInt32 GetLookaheadInFrames( void ) const GUCEF_VIRTUAL_OVERRIDE { return 0; }
    virtual UInt32 GetTimestampRate( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_config.GetSampleRate(); }

    virtual Int32 Encode( const Float32* frame, UInt8* payload, UInt32 capacity ) GUCEF_VIRTUAL_OVERRIDE
    {
        const UInt32 index = m_frame++;
        if ( 0 != errorEvery && errorEvery - 1 == index % errorEvery )
            return -1;
        if ( 0 != emptyEvery && emptyEvery - 1 == index % emptyEvery )
            return 0;
        if ( capacity < 8 )
            return -1;
        memcpy( payload, &index, 4 );
        memcpy( payload + 4, frame, 4 );
        return 8;
    }

    virtual void SetBitrate( UInt32 bitsPerSecond ) GUCEF_VIRTUAL_OVERRIDE { bitrate.Store( bitsPerSecond ); }
    virtual void SetExpectedLossPercentage( UInt32 percentage ) GUCEF_VIRTUAL_OVERRIDE { loss.Store( percentage ); }
    virtual void Reset( void ) GUCEF_VIRTUAL_OVERRIDE { m_frame = 0; }

    private:

    AUDIO::CVoiceCodecConfig m_config;
    UInt32 m_frame;

    public:

    MT::CTAtomic< UInt32 > bitrate;
    MT::CTAtomic< UInt32 > loss;
    UInt32 errorEvery;
    UInt32 emptyEvery;
};

/** Makes CScriptedVoiceEncoder for the codec "sendertest"; the last one made is kept for the test to look at */
class CScriptedVoiceCodecFactory : public AUDIO::CVoiceCodecFactory
{
    public:

    CScriptedVoiceCodecFactory( void ) : AUDIO::CVoiceCodecFactory(), last( GUCEF_NULL ), errorEvery( 0 ), emptyEvery( 0 ) {}

    virtual const CORE::CString& GetCodecTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        static const CORE::CString name = "sendertest";
        return name;
    }

    virtual bool IsSupported( const AUDIO::CVoiceCodecConfig& config ) const GUCEF_VIRTUAL_OVERRIDE
    {
        return 1 == config.GetChannels() && config.GetFrameSizeInFrames() > 0;
    }

    CScriptedVoiceEncoder* last;
    UInt32 errorEvery;
    UInt32 emptyEvery;

    protected:

    virtual AUDIO::CVoiceEncoder* OnCreateEncoder( const AUDIO::CVoiceCodecConfig& config ) GUCEF_VIRTUAL_OVERRIDE
    {
        last = GUCEF_NEW CScriptedVoiceEncoder( config );
        last->errorEvery = errorEvery;
        last->emptyEvery = emptyEvery;
        return last;
    }

    virtual AUDIO::CVoiceDecoder* OnCreateDecoder( const AUDIO::CVoiceCodecConfig& ) GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NULL; }
};

/*-------------------------------------------------------------------------*/

/** Checks the frames of one talkspurt: consecutive sequence numbers and timestamps, the marker on the first only */
bool
IsTalkspurt( const TFrames& frames, size_t from, size_t count, UInt32 timestampStep )
{
    for ( size_t i=from; i<from+count; ++i )
    {
        if ( i >= frames.size() )
            return false;
        if ( frames[ i ].GetMarker() != ( i == from ) )
            return false;
        if ( i > from && ( (UInt16) ( frames[ i ].GetSequenceNumber() - frames[ i - 1 ].GetSequenceNumber() ) != 1 ||
                           frames[ i ].GetTimestamp() - frames[ i - 1 ].GetTimestamp() != timestampStep ) )
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

/** Feeds blocks from its own thread, as the capture thread of the engine would */
class CCaptureThread : public MT::CActiveObject
{
    public:

    CCaptureThread( AUDIO::CVoiceSender& sender, const TSamples& signal, UInt32 blocks )
        : MT::CActiveObject(), m_sender( sender ), m_signal( signal ), m_blocks( blocks ), done( 0 ) {}

    virtual bool OnThreadStart( void* ) GUCEF_VIRTUAL_OVERRIDE { return true; }

    virtual bool OnThreadCycle( void* ) GUCEF_VIRTUAL_OVERRIDE
    {
        UInt64 position = 0;
        AudioEffectTest::CXorShift random( 77 );
        for ( UInt32 b=0; b<m_blocks; ++b )
        {
            const size_t from = (size_t) ( position % ( m_signal.size() - 512 ) );
            const bool overrun = 0 == b % 500 && b > 0;
            if ( overrun )
                position += 1735;
            FeedSender( m_sender, m_signal, from, 160, position, 16000, 160, 0 != ( ( b / 40 ) & 1 ), GUCEF_NULL, overrun );
            if ( 0 == ( random.Next() & 3 ) )
                MT::PrecisionDelay( 1 );
        }
        done.Store( 1 );
        return true;
    }

    virtual const MT::CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NULL; }

    private:

    AUDIO::CVoiceSender& m_sender;
    const TSamples& m_signal;
    UInt32 m_blocks;

    public:

    MT::CTAtomic< UInt32 > done;
};

/** Polls PopFrame() as the network task would, and checks the order of what it gets */
class CNetworkThread : public MT::CActiveObject
{
    public:

    explicit CNetworkThread( AUDIO::CVoiceSender& sender )
        : MT::CActiveObject(), m_sender( sender ), stop( 0 ), popped( 0 ), outOfOrder( 0 ), seqGaps( 0 ) {}

    virtual bool OnThreadStart( void* ) GUCEF_VIRTUAL_OVERRIDE { return true; }

    virtual bool OnThreadCycle( void* ) GUCEF_VIRTUAL_OVERRIDE
    {
        AUDIO::CVoiceFrame frame;
        bool hasLast = false;
        UInt16 lastSeq = 0;
        UInt8 packet[ AUDIO::CRtpVoicePacketCodec::MaxPacketSize ];
        AUDIO::CRtpVoicePacketCodec codec;
        for ( ;; )
        {
            const bool stopping = 0 != stop.Load();
            bool any = false;
            while ( m_sender.PopFrame( frame ) )
            {
                any = true;
                if ( hasLast )
                {
                    const UInt16 step = (UInt16) ( frame.GetSequenceNumber() - lastSeq );
                    if ( 0 == step || step > 0x8000 )
                        outOfOrder.FetchAdd( 1 );
                    else
                        seqGaps.FetchAdd( step - 1 );
                }
                hasLast = true;
                lastSeq = frame.GetSequenceNumber();
                const Int32 size = codec.Write( frame, packet, sizeof( packet ) );
                if ( size <= 0 || size > (Int32) AUDIO::CVoiceSender::MaxPacketSize )
                    outOfOrder.FetchAdd( 1 );
                popped.FetchAdd( 1 );
            }
            if ( stopping && !any )
                break;
            MT::PrecisionDelay( 1 );
        }
        return true;
    }

    virtual const MT::CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NULL; }

    private:

    AUDIO::CVoiceSender& m_sender;

    public:

    MT::CTAtomic< UInt32 > stop;
    MT::CTAtomic< UInt32 > popped;
    MT::CTAtomic< UInt32 > outOfOrder;
    MT::CTAtomic< UInt32 > seqGaps;
};

/*-------------------------------------------------------------------------*/

AUDIO::CAudioEngineParams
CaptureEngineParams( UInt32 rate )
{
    AUDIO::CAudioEngineParams params;
    params.SetDriverName( "null" );
    params.SetThreadingMode( AUDIO::AUDIOTHREADING_COOPERATIVE );
    params.SetLimiterMode( AUDIO::AUDIOLIMITERMODE_INSTANT );
    params.GetDeviceParams().SetBufferSizeInFrames( 256 );
    params.SetCaptureEnabled( true );
    params.SetCaptureProcessingRate( rate );
    params.GetCaptureDeviceParams().SetPreferredFormat( MonoFormat( rate ) );
    params.GetCaptureDeviceParams().SetBufferSizeInFrames( rate / 100 );
    return params;
}

/** A source that plays the samples of a buffer once and then has nothing */
class CSamplesSource : public AUDIO::IAudioSource
{
    public:

    CSamplesSource( const TSamples& samples, UInt32 rate ) : m_samples( samples ), m_format( MonoFormat( rate ) ), m_position( 0 ) {}

    virtual const AUDIO::CAudioFormat& GetFormat( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_format; }

    virtual UInt32 ReadFrames( void* destination, UInt32 frameCapacity ) GUCEF_VIRTUAL_OVERRIDE
    {
        const size_t left = m_position < m_samples.size() ? m_samples.size() - m_position : 0;
        const UInt32 count = (UInt32) ( left < frameCapacity ? left : frameCapacity );
        if ( count > 0 )
            memcpy( destination, &m_samples[ m_position ], count * sizeof( Float32 ) );
        m_position += count;
        return count;
    }

    virtual bool IsEndOfStream( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_position >= m_samples.size(); }
    virtual bool IsRealtimeSafe( void ) const GUCEF_VIRTUAL_OVERRIDE { return true; }
    virtual UInt64 GetPositionInFrames( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_position; }

    private:

    TSamples m_samples;
    AUDIO::CAudioFormat m_format;
    size_t m_position;
};

/*-------------------------------------------------------------------------*/

void
PerformConfigTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "AudioVoiceSender: The configuration has the defaults of the design, limits its values and survives a save and load" )
    try
    {
        AUDIO::CVoiceSenderConfig config;
        ASSERT_TRUE( AUDIO::VOICETRANSMITMODE_CONTINUOUS == config.GetTransmitMode() );
        ASSERT_TRUE( 300 == config.GetVadHangoverInMs() && 2 == config.GetPreRollFrames() && 0 == config.GetRedundancy() );
        ASSERT_TRUE( 113 == config.GetRedPayloadType() && 16 == config.GetOutboundQueueFrames() && 0 == config.GetStreamId() );
        ASSERT_FALSE( config.HasFirstSequenceNumber() || config.HasFirstTimestamp() );
        ASSERT_TRUE( config.GetCodecConfig() == AUDIO::CVoiceCodecConfig() );

        config.SetPreRollFrames( 20 );
        config.SetRedundancy( 5 );
        config.SetOutboundQueueFrames( 1 );
        config.SetVadHangoverInMs( 99999 );
        config.SetRedPayloadType( 200 );
        ASSERT_TRUE( 8 == config.GetPreRollFrames() && 1 == config.GetRedundancy() && 2 == config.GetOutboundQueueFrames() );
        ASSERT_TRUE( 5000 == config.GetVadHangoverInMs() && 113 == config.GetRedPayloadType() );
        config.SetOutboundQueueFrames( 100000 );
        ASSERT_TRUE( 4096 == config.GetOutboundQueueFrames() );

        AUDIO::TVoiceTransmitMode mode = AUDIO::VOICETRANSMITMODE_CONTINUOUS;
        ASSERT_TRUE( AUDIO::CVoiceSenderConfig::StringToTransmitMode( "PushToTalk", mode ) && AUDIO::VOICETRANSMITMODE_PUSH_TO_TALK == mode );
        ASSERT_TRUE( AUDIO::CVoiceSenderConfig::StringToTransmitMode( "voiceactivated", mode ) && AUDIO::VOICETRANSMITMODE_VOICE_ACTIVATED == mode );
        ASSERT_FALSE( AUDIO::CVoiceSenderConfig::StringToTransmitMode( "always", mode ) );
        ASSERT_TRUE( AUDIO::VOICETRANSMITMODE_VOICE_ACTIVATED == mode );
        ASSERT_TRUE( CORE::CString( "pushToTalk" ) == AUDIO::CVoiceSenderConfig::TransmitModeToString( AUDIO::VOICETRANSMITMODE_PUSH_TO_TALK ) );

        AUDIO::CVoiceSenderConfig saved = SenderConfig( PcmConfig( 16000, 20000 ), AUDIO::VOICETRANSMITMODE_VOICE_ACTIVATED );
        saved.SetVadHangoverInMs( 450 );
        saved.SetPreRollFrames( 3 );
        saved.SetRedundancy( 1 );
        saved.SetRedPayloadType( 99 );
        saved.SetOutboundQueueFrames( 24 );
        saved.GetCodecConfig().SetPayloadType( 101 );
        CORE::CDataNode tree;
        ASSERT_TRUE( saved.SaveConfig( tree ) );
        ASSERT_TRUE( GUCEF_NULL != tree.FindChild( "VoiceSender" ) && GUCEF_NULL != tree.FindChild( "VoiceSender" )->FindChild( "VoiceCodec" ) );
        AUDIO::CVoiceSenderConfig loaded;
        ASSERT_TRUE( loaded.LoadConfig( tree ) );
        ASSERT_TRUE( loaded == saved );
        ASSERT_TRUE( FirstSequenceNumber == loaded.GetFirstSequenceNumber() && FirstTimestamp == loaded.GetFirstTimestamp() );

        // the section itself, unknown names, a configuration without the optional starts
        AUDIO::CVoiceSenderConfig fromSection;
        ASSERT_TRUE( fromSection.LoadConfig( *tree.FindChild( "VoiceSender" ) ) && fromSection == saved );
        tree.FindChild( "VoiceSender" )->SetAttribute( "transmitMode", "sometimes" );
        ASSERT_TRUE( fromSection.LoadConfig( tree ) && AUDIO::VOICETRANSMITMODE_VOICE_ACTIVATED == fromSection.GetTransmitMode() );
        AUDIO::CVoiceSenderConfig plain;
        CORE::CDataNode plainTree;
        ASSERT_TRUE( plain.SaveConfig( plainTree ) );
        AUDIO::CVoiceSenderConfig fresh;
        ASSERT_TRUE( fresh.LoadConfig( plainTree ) && !fresh.HasFirstSequenceNumber() && !fresh.HasFirstTimestamp() );
        ASSERT_TRUE( loaded.LoadConfig( plainTree ) && loaded.HasFirstSequenceNumber() );         // what the section does not have stays
        CORE::CDataNode empty;
        ASSERT_FALSE( loaded.LoadConfig( empty ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
PerformFrameTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "AudioVoiceSender: Continuous mode sends every frame with consecutive sequence numbers and timestamps, one marker, and the payload of the audio" )
    try
    {
        const UInt32 rates[ 3 ] = { 16000, 48000, 8000 };
        const UInt32 durations[ 3 ] = { 10000, 5000, 20000 };
        for ( int c=0; c<3; ++c )
        {
            const UInt32 rate = rates[ c ];
            const UInt32 frameSize = rate * durations[ c ] / 1000000;
            AUDIO::CVoiceSender sender( SenderConfig( PcmConfig( rate, durations[ c ] ), AUDIO::VOICETRANSMITMODE_CONTINUOUS ) );
            ASSERT_FALSE( sender.PrepareForCapture( MonoFormat( rate + 1 ), 1024 ) );
            ASSERT_FALSE( sender.PrepareForCapture( AUDIO::CAudioFormat( rate, 2, AUDIO::SAMPLETYPE_FLOAT32 ), 1024 ) );
            ASSERT_TRUE( sender.PrepareForCapture( MonoFormat( rate ), 1024 ) );
            ASSERT_TRUE( StreamId == sender.GetStreamId() );

            const UInt32 frames = 50;
            const TSamples signal = MakePcmSignal( frames * frameSize + frameSize / 2, rate );
            UInt64 position = 123456;
            AudioEffectTest::CXorShift random( 11 + c );
            FeedSender( sender, signal, 0, signal.size(), position, rate, 400, false, &random );
            ASSERT_TRUE( frames == sender.GetPendingFrameCount() );

            TFrames sent;
            ASSERT_TRUE( frames == PopAll( sender, sent ) );
            ASSERT_TRUE( IsTalkspurt( sent, 0, frames, frameSize ) );
            TSamples decoded;
            for ( UInt32 k=0; k<frames; ++k )
            {
                const AUDIO::CVoiceFrame& frame = sent[ k ];
                ASSERT_TRUE( StreamId == frame.GetStreamId() && 112 == frame.GetPayloadType() );
                ASSERT_TRUE( (UInt16) ( FirstSequenceNumber + k ) == frame.GetSequenceNumber() );
                ASSERT_TRUE( FirstTimestamp + k * frameSize == frame.GetTimestamp() );
                ASSERT_TRUE( 2 * frameSize == frame.GetPayloadSize() && !frame.HasRedundantBlock() );
                // the capture time is that of the block in which the frame began
                const UInt64 firstSampleInUs = ( 123456ULL + (UInt64) k * frameSize ) * 1000000ULL / rate;
                ASSERT_TRUE( frame.GetCaptureTickCount() <= firstSampleInUs && frame.GetCaptureTickCount() + 400ULL * 1000000ULL / rate + 1 >= firstSampleInUs );
                AppendPcmPayload( frame, decoded );
            }
            ASSERT_TRUE( 0 == memcmp( &decoded[ 0 ], &signal[ 0 ], (size_t) frames * frameSize * sizeof( Float32 ) ) );

            AUDIO::CVoiceSenderStats stats;
            sender.GetStats( stats );
            ASSERT_TRUE( frames == stats.framesCaptured && frames == stats.framesEncoded && frames == stats.framesSent && 0 == stats.framesDropped );
            ASSERT_TRUE( (UInt64) frames * 2 * frameSize == stats.octetsSent && 1 == stats.talkspurts && StreamId == stats.streamId );
            ASSERT_TRUE( stats.hasLastFrame && FirstTimestamp + ( frames - 1 ) * frameSize == stats.lastTimestamp );
            ASSERT_TRUE( (UInt16) ( FirstSequenceNumber + frames - 1 ) == stats.lastSequenceNumber && sent.back().GetCaptureTickCount() == stats.lastCaptureTickCount );
            ASSERT_TRUE( stats.transmitting && sender.IsTransmitting() && stats.maxEncodeTimeInUs >= stats.meanEncodeTimeInUs );
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioVoiceSender: pcm " << rate << " Hz, " << durations[ c ] << " us frames: " << frames << " frames, encode mean " << stats.meanEncodeTimeInUs << " us, worst " << stats.maxEncodeTimeInUs << " us" );
        }
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioVoiceSender: Sequence numbers and timestamps wrap; without configured values the identity of the stream is random" )
    try
    {
        AUDIO::CVoiceSenderConfig config = SenderConfig( PcmConfig( 16000 ), AUDIO::VOICETRANSMITMODE_CONTINUOUS );
        config.SetFirstSequenceNumber( 65530 );
        config.SetFirstTimestamp( 0xFFFFFFFFU - 478 );
        config.SetOutboundQueueFrames( 64 );
        AUDIO::CVoiceSender sender( config );
        ASSERT_TRUE( sender.PrepareForCapture( MonoFormat( 16000 ), 1024 ) );
        const TSamples signal = MakePcmSignal( 20 * 160, 16000 );
        UInt64 position = 0;
        FeedSender( sender, signal, 0, signal.size(), position, 16000, 160, false );
        TFrames sent;
        ASSERT_TRUE( 20 == PopAll( sender, sent ) );
        ASSERT_TRUE( IsTalkspurt( sent, 0, 20, 160 ) );
        ASSERT_TRUE( 65535 == sent[ 5 ].GetSequenceNumber() && 0 == sent[ 6 ].GetSequenceNumber() && 13 == sent[ 19 ].GetSequenceNumber() );
        ASSERT_TRUE( 0xFFFFFFFFU - 158 == sent[ 2 ].GetTimestamp() && 1 == sent[ 3 ].GetTimestamp() && 161 == sent[ 4 ].GetTimestamp() );

        // random identity: different senders differ, the stream id is never 0
        AUDIO::CVoiceSenderConfig open;
        open.SetCodecConfig( PcmConfig( 16000 ) );
        AUDIO::CVoiceSender a( open ), b( open );
        ASSERT_TRUE( 0 != a.GetStreamId() && 0 != b.GetStreamId() && a.GetStreamId() != b.GetStreamId() );
        ASSERT_TRUE( a.PrepareForCapture( MonoFormat( 16000 ), 1024 ) && b.PrepareForCapture( MonoFormat( 16000 ), 1024 ) );
        UInt64 pa = 0, pb = 0;
        FeedSender( a, signal, 0, 160, pa, 16000, 160, false );
        FeedSender( b, signal, 0, 160, pb, 16000, 160, false );
        AUDIO::CVoiceFrame fa, fb;
        ASSERT_TRUE( a.PopFrame( fa ) && b.PopFrame( fb ) );
        ASSERT_TRUE( fa.GetStreamId() == a.GetStreamId() && ( fa.GetSequenceNumber() != fb.GetSequenceNumber() || fa.GetTimestamp() != fb.GetTimestamp() ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioVoiceSender: An overrun loses the frame in progress, the timestamps jump by the lost audio and the next frame has the marker" )
    try
    {
        AUDIO::CVoiceSender sender( SenderConfig( PcmConfig( 16000 ), AUDIO::VOICETRANSMITMODE_CONTINUOUS ) );
        ASSERT_TRUE( sender.PrepareForCapture( MonoFormat( 16000 ), 1024 ) );
        const TSamples signal = MakePcmSignal( 16000, 16000 );
        UInt64 position = 1000;
        FeedSender( sender, signal, 0, 400, position, 16000, 400, false );         // two frames and half of a third
        position += 1735;                                                            // the overrun
        FeedSender( sender, signal, 400, 480, position, 16000, 480, false, GUCEF_NULL, true );
        // a jump without the flag is a break as well
        position += 77;
        FeedSender( sender, signal, 880, 320, position, 16000, 160, false );

        TFrames sent;
        ASSERT_TRUE( 2 + 3 + 2 == PopAll( sender, sent ) );
        ASSERT_TRUE( IsTalkspurt( sent, 0, 2, 160 ) && IsTalkspurt( sent, 2, 3, 160 ) && IsTalkspurt( sent, 5, 2, 160 ) );
        ASSERT_TRUE( FirstTimestamp + 400 + 1735 == sent[ 2 ].GetTimestamp() );                    // the timestamp is the position of the audio
        ASSERT_TRUE( FirstTimestamp + 400 + 1735 + 480 + 77 == sent[ 5 ].GetTimestamp() );
        ASSERT_TRUE( (UInt16) ( FirstSequenceNumber + 2 ) == sent[ 2 ].GetSequenceNumber() );        // nothing was sent for the lost half frame
        TSamples decoded;
        AppendPcmPayload( sent[ 2 ], decoded );
        ASSERT_TRUE( 0 == memcmp( &decoded[ 0 ], &signal[ 400 ], 160 * sizeof( Float32 ) ) );

        AUDIO::CVoiceSenderStats stats;
        sender.GetStats( stats );
        ASSERT_TRUE( 2 == stats.discontinuities && 7 == stats.framesCaptured && 3 == stats.talkspurts );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioVoiceSender: A full outbound queue drops the newest frame and counts it; its sequence number is used" )
    try
    {
        AUDIO::CVoiceSenderConfig config = SenderConfig( PcmConfig( 16000 ), AUDIO::VOICETRANSMITMODE_CONTINUOUS );
        config.SetOutboundQueueFrames( 5 );     // a queue of 8 inside: the configured 5 is the limit
        AUDIO::CVoiceSender sender( config );
        ASSERT_TRUE( sender.PrepareForCapture( MonoFormat( 16000 ), 1024 ) );
        const TSamples signal = MakePcmSignal( 16000, 16000 );
        UInt64 position = 0;
        FeedSender( sender, signal, 0, 10 * 160, position, 16000, 160, false );
        ASSERT_TRUE( 5 == sender.GetPendingFrameCount() );
        TFrames sent;
        ASSERT_TRUE( 5 == PopAll( sender, sent ) );
        ASSERT_TRUE( IsTalkspurt( sent, 0, 5, 160 ) );
        FeedSender( sender, signal, 1600, 160, position, 16000, 160, false );
        AUDIO::CVoiceFrame frame;
        ASSERT_TRUE( sender.PopFrame( frame ) && (UInt16) ( FirstSequenceNumber + 10 ) == frame.GetSequenceNumber() && !frame.GetMarker() );
        ASSERT_FALSE( sender.PopFrame( frame ) );

        AUDIO::CVoiceSenderStats stats;
        sender.GetStats( stats );
        ASSERT_TRUE( 6 == stats.framesSent && 5 == stats.framesDropped && 11 == stats.framesEncoded && 6 * 320 == stats.octetsSent );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioVoiceSender: Redundancy attaches the previous frame within the packet size limit; packets made of the frames read back" )
    try
    {
        struct SCase { UInt32 rate; UInt32 duration; bool fits; };
        const SCase cases[ 3 ] = { { 16000, 10000, true }, { 16000, 20000, true }, { 48000, 10000, false } };
        for ( int c=0; c<3; ++c )
        {
            AUDIO::CVoiceSenderConfig config = SenderConfig( PcmConfig( cases[ c ].rate, cases[ c ].duration ), AUDIO::VOICETRANSMITMODE_CONTINUOUS );
            config.SetRedundancy( 1 );
            AUDIO::CVoiceSender sender( config );
            ASSERT_TRUE( sender.PrepareForCapture( MonoFormat( cases[ c ].rate ), 1024 ) );
            const UInt32 frameSize = cases[ c ].rate * cases[ c ].duration / 1000000;
            const TSamples signal = MakePcmSignal( 10 * frameSize, cases[ c ].rate );
            UInt64 position = 0;
            FeedSender( sender, signal, 0, signal.size(), position, cases[ c ].rate, 333, false );
            TFrames sent;
            ASSERT_TRUE( 10 == PopAll( sender, sent ) );

            AUDIO::CRtpVoicePacketCodec codec;
            codec.SetRedPayloadType( config.GetRedPayloadType() );
            UInt8 packet[ AUDIO::CRtpVoicePacketCodec::MaxPacketSize ];
            for ( size_t k=0; k<sent.size(); ++k )
            {
                const bool expected = cases[ c ].fits && k > 0;
                ASSERT_TRUE( expected == sent[ k ].HasRedundantBlock() );
                if ( expected )
                {
                    ASSERT_TRUE( frameSize == sent[ k ].GetRedundantTimestampOffset() && 112 == sent[ k ].GetRedundantPayloadType() );
                    ASSERT_TRUE( sent[ k - 1 ].GetPayloadSize() == sent[ k ].GetRedundantSize() );
                    ASSERT_TRUE( 0 == memcmp( sent[ k - 1 ].GetPayload(), sent[ k ].GetRedundantPayload(), sent[ k ].GetRedundantSize() ) );
                }
                const Int32 size = codec.Write( sent[ k ], packet, sizeof( packet ) );
                ASSERT_TRUE( size > 0 && size <= (Int32) AUDIO::CVoiceSender::MaxPacketSize );
                AUDIO::CVoiceFrame back;
                ASSERT_TRUE( codec.Read( packet, (UInt32) size, back ) );
                ASSERT_TRUE( back.GetSequenceNumber() == sent[ k ].GetSequenceNumber() && back.GetTimestamp() == sent[ k ].GetTimestamp() );
                ASSERT_TRUE( back.GetPayloadSize() == sent[ k ].GetPayloadSize() && back.HasRedundantBlock() == sent[ k ].HasRedundantBlock() );
            }
            AUDIO::CVoiceSenderStats stats;
            sender.GetStats( stats );
            const UInt64 bytes = 2ULL * frameSize;
            if ( cases[ c ].fits )
                ASSERT_TRUE( 9 == stats.redundantBlocksSent && 0 == stats.redundancyOmitted && 10 * bytes + 9 * ( 5 + bytes ) == stats.octetsSent );
            else
                ASSERT_TRUE( 0 == stats.redundantBlocksSent && 9 == stats.redundancyOmitted && 10 * bytes == stats.octetsSent );
        }
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
PerformGatingTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "AudioVoiceSender: Push to talk sends while the key is down and the frame in progress at its release; a tap sends one frame" )
    try
    {
        AUDIO::CVoiceSender sender( SenderConfig( PcmConfig( 16000 ), AUDIO::VOICETRANSMITMODE_PUSH_TO_TALK ) );
        ASSERT_TRUE( sender.PrepareForCapture( MonoFormat( 16000 ), 1024 ) );
        const TSamples signal = MakePcmSignal( 16000, 16000 );
        UInt64 position = 0;
        FeedSender( sender, signal, 0, 1600, position, 16000, 80, false );                  // frames 0 to 9: key up
        ASSERT_TRUE( 0 == sender.GetPendingFrameCount() && !sender.IsTransmitting() );
        sender.SetTransmitting( true );
        ASSERT_TRUE( sender.IsTalkKeyDown() );
        FeedSender( sender, signal, 1600, 800, position, 16000, 80, false );                // frames 10 to 14
        FeedSender( sender, signal, 2400, 80, position, 16000, 80, false );                 // half of frame 15
        ASSERT_TRUE( sender.IsTransmitting() );
        sender.SetTransmitting( false );
        FeedSender( sender, signal, 2480, 80 + 800, position, 16000, 80, false );           // the rest of 15, then 16 to 20
        ASSERT_FALSE( sender.IsTransmitting() );

        // a tap within frame 21
        FeedSender( sender, signal, 3360, 40, position, 16000, 40, false );
        sender.SetTransmitting( true );
        sender.SetTransmitting( false );
        sender.SetTransmitting( false );                                                    // a second release changes nothing
        FeedSender( sender, signal, 3400, 120 + 480, position, 16000, 40, false );          // the rest of 21, then 22 to 24

        TFrames sent;
        ASSERT_TRUE( 7 == PopAll( sender, sent ) );
        ASSERT_TRUE( IsTalkspurt( sent, 0, 6, 160 ) && IsTalkspurt( sent, 6, 1, 160 ) );
        ASSERT_TRUE( FirstTimestamp + 10 * 160 == sent[ 0 ].GetTimestamp() && FirstTimestamp + 15 * 160 == sent[ 5 ].GetTimestamp() );
        ASSERT_TRUE( FirstTimestamp + 21 * 160 == sent[ 6 ].GetTimestamp() && (UInt16) ( FirstSequenceNumber + 6 ) == sent[ 6 ].GetSequenceNumber() );

        // the mode changes from any thread at the next frame: continuous sends again, push to talk with the key up stops
        sender.SetTransmitMode( AUDIO::VOICETRANSMITMODE_CONTINUOUS );
        ASSERT_TRUE( AUDIO::VOICETRANSMITMODE_CONTINUOUS == sender.GetTransmitMode() );
        FeedSender( sender, signal, 4000, 320, position, 16000, 160, false );
        sender.SetTransmitMode( AUDIO::VOICETRANSMITMODE_PUSH_TO_TALK );
        FeedSender( sender, signal, 4320, 320, position, 16000, 160, false );
        sent.clear();
        ASSERT_TRUE( 2 == PopAll( sender, sent ) && sent[ 0 ].GetMarker() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioVoiceSender: Voice activated mode sends the pre-roll before the onset, holds the hangover, and mute sends nothing" )
    try
    {
        const UInt32 preRolls[ 3 ] = { 2, 0, 8 };
        for ( int c=0; c<3; ++c )
        {
            AUDIO::CVoiceSenderConfig config = SenderConfig( PcmConfig( 16000 ), AUDIO::VOICETRANSMITMODE_VOICE_ACTIVATED );
            config.SetPreRollFrames( preRolls[ c ] );
            config.SetOutboundQueueFrames( 128 );
            AUDIO::CVoiceSender sender( config );
            ASSERT_TRUE( sender.PrepareForCapture( MonoFormat( 16000 ), 1024 ) );
            const TSamples signal = MakePcmSignal( 160 * 200, 16000 );
            UInt64 position = 0;
            FeedSender( sender, signal, 0, 10 * 160, position, 16000, 160, false );         // frames 0 to 9: no voice
            ASSERT_TRUE( 0 == sender.GetPendingFrameCount() );
            FeedSender( sender, signal, 1600, 5 * 160, position, 16000, 160, true );        // 10 to 14: voice
            FeedSender( sender, signal, 2400, 40 * 160, position, 16000, 160, false );      // 15 to 54: 30 of hangover (300 ms)

            TFrames sent;
            const UInt32 preRoll = preRolls[ c ];
            ASSERT_TRUE( preRoll + 5 + 30 == PopAll( sender, sent ) );
            ASSERT_TRUE( IsTalkspurt( sent, 0, sent.size(), 160 ) );
            ASSERT_TRUE( FirstTimestamp + ( 10 - preRoll ) * 160 == sent[ 0 ].GetTimestamp() );
            TSamples decoded;
            for ( size_t k=0; k<sent.size(); ++k )
                AppendPcmPayload( sent[ k ], decoded );
            ASSERT_TRUE( 0 == memcmp( &decoded[ 0 ], &signal[ ( 10 - preRoll ) * 160 ], decoded.size() * sizeof( Float32 ) ) );   // the pre-roll is real audio
            AUDIO::CVoiceSenderStats stats;
            sender.GetStats( stats );
            ASSERT_TRUE( preRoll == stats.preRollFramesSent && 55 == stats.framesEncoded && 1 == stats.talkspurts );

            // the next onset: the pre-roll again, a new talkspurt with a timestamp gap
            FeedSender( sender, signal, 8800, 3 * 160, position, 16000, 160, true );        // 55 to 57
            sent.clear();
            ASSERT_TRUE( preRoll + 3 == PopAll( sender, sent ) );
            ASSERT_TRUE( IsTalkspurt( sent, 0, sent.size(), 160 ) && FirstTimestamp + ( 55 - preRoll ) * 160 == sent[ 0 ].GetTimestamp() );

            // muted: nothing, also not while the voice is there; unmuted it starts a talkspurt without a pre-roll of the muted time
            sender.SetMuted( true );
            ASSERT_TRUE( sender.IsMuted() );
            FeedSender( sender, signal, 9280, 5 * 160, position, 16000, 160, true );        // 58 to 62
            ASSERT_TRUE( 0 == sender.GetPendingFrameCount() && !sender.IsTransmitting() );
            sender.SetMuted( false );
            FeedSender( sender, signal, 10080, 2 * 160, position, 16000, 160, true );       // 63 and 64
            sent.clear();
            ASSERT_TRUE( 2 == PopAll( sender, sent ) && IsTalkspurt( sent, 0, 2, 160 ) && FirstTimestamp + 63 * 160 == sent[ 0 ].GetTimestamp() );
        }

        // a hangover of 0: sending stops with the voice
        AUDIO::CVoiceSenderConfig config = SenderConfig( PcmConfig( 16000 ), AUDIO::VOICETRANSMITMODE_VOICE_ACTIVATED );
        config.SetVadHangoverInMs( 0 );
        config.SetPreRollFrames( 0 );
        AUDIO::CVoiceSender sender( config );
        ASSERT_TRUE( sender.PrepareForCapture( MonoFormat( 16000 ), 1024 ) );
        const TSamples signal = MakePcmSignal( 1600, 16000 );
        UInt64 position = 0;
        FeedSender( sender, signal, 0, 320, position, 16000, 160, true );
        FeedSender( sender, signal, 320, 640, position, 16000, 160, false );
        ASSERT_TRUE( 2 == sender.GetPendingFrameCount() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
PerformCodecTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "AudioVoiceSender: Encoder errors and empty frames are counted and leave gaps; bit rate and expected loss reach the encoder from another thread" )
    try
    {
        CScriptedVoiceCodecFactory* factory = GUCEF_NEW CScriptedVoiceCodecFactory();
        factory->errorEvery = 5;
        factory->emptyEvery = 7;
        AUDIO::TVoiceCodecFactoryPtr factoryPtr( factory );
        AUDIO::CVoiceCodecRegistry& registry = AUDIO::CAudioGlobal::Instance()->GetVoiceCodecRegistry();
        ASSERT_TRUE( registry.RegisterFactory( factoryPtr ) );
        {
            AUDIO::CVoiceCodecConfig codecConfig = VoiceCodecTest::MakeConfig( "sendertest", 16000, 10000 );
            codecConfig.SetPayloadType( 100 );
            AUDIO::CVoiceSenderConfig config = SenderConfig( codecConfig, AUDIO::VOICETRANSMITMODE_CONTINUOUS );
            config.SetOutboundQueueFrames( 64 );
            AUDIO::CVoiceSender sender( config );
            ASSERT_TRUE( sender.PrepareForCapture( MonoFormat( 16000 ), 1024 ) );
            ASSERT_TRUE( GUCEF_NULL != factory->last );
            CScriptedVoiceEncoder* encoder = factory->last;
            const TSamples signal = MakePcmSignal( 35 * 160, 16000 );
            UInt64 position = 0;
            FeedSender( sender, signal, 0, signal.size(), position, 16000, 160, false );
            TFrames sent;
            PopAll( sender, sent );

            // frames 4, 9, 14 ... 34 fail, 6, 13, 20 and 27 are empty: 7 + 4 not sent, each followed by a new talkspurt
            AUDIO::CVoiceSenderStats stats;
            sender.GetStats( stats );
            ASSERT_TRUE( 7 == stats.encodeErrors && 4 == stats.framesEmpty && 24 == stats.framesEncoded && 24 == sent.size() );
            UInt32 markers = 0;
            for ( size_t k=0; k<sent.size(); ++k )
            {
                UInt32 index = 0;
                memcpy( &index, sent[ k ].GetPayload(), 4 );
                ASSERT_TRUE( FirstTimestamp + index * 160 == sent[ k ].GetTimestamp() && 100 == sent[ k ].GetPayloadType() );
                const bool afterGap = 0 == index || 4 == ( index - 1 ) % 5 || 6 == ( index - 1 ) % 7;
                ASSERT_TRUE( afterGap == sent[ k ].GetMarker() );
                markers += sent[ k ].GetMarker() ? 1 : 0;
            }
            ASSERT_TRUE( markers == stats.talkspurts );

            // bit rate and loss from another thread: applied at the next frame
            sender.SetBitrate( 24000 );
            sender.SetExpectedLossPercentage( 15 );
            ASSERT_TRUE( 0 == encoder->bitrate.Load() );
            FeedSender( sender, signal, 0, 160, position, 16000, 160, false );
            ASSERT_TRUE( 24000 == encoder->bitrate.Load() && 15 == encoder->loss.Load() );
            sender.SetBitrate( 32000 );
            FeedSender( sender, signal, 0, 160, position, 16000, 160, false );
            ASSERT_TRUE( 32000 == encoder->bitrate.Load() );
        }
        ASSERT_TRUE( registry.TryUnregisterFactory( "sendertest" ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioVoiceSender: A codec change is taken at the next frame: a new talkspurt with the new payload type, the timestamps go on from the audio position" )
    try
    {
        AUDIO::CVoiceSenderConfig config = SenderConfig( PcmConfig( 16000 ), AUDIO::VOICETRANSMITMODE_CONTINUOUS );
        config.SetOutboundQueueFrames( 64 );
        AUDIO::CVoiceSender sender( config );
        ASSERT_TRUE( sender.PrepareForCapture( MonoFormat( 16000 ), 1024 ) );
        const TSamples signal = MakePcmSignal( 16000, 16000 );
        UInt64 position = 0;
        FeedSender( sender, signal, 0, 400, position, 16000, 100, false );                  // frames 0, 1 and half of 2

        AUDIO::CVoiceCodecConfig longer = PcmConfig( 16000, 20000 );
        longer.SetPayloadType( 100 );
        ASSERT_TRUE( sender.SetCodecConfig( longer ) );
        ASSERT_TRUE( sender.IsCodecChangePending() );
        ASSERT_FALSE( sender.SetCodecConfig( PcmConfig( 16000, 5000 ) ) );                  // the first change is not taken yet
        ASSERT_FALSE( sender.SetCodecConfig( PcmConfig( 48000 ) ) );
        FeedSender( sender, signal, 400, 80 + 3 * 320, position, 16000, 100, false );       // the rest of 2, then three of 20 ms
        ASSERT_FALSE( sender.IsCodecChangePending() );
        ASSERT_TRUE( sender.GetCodecConfig() == longer );

        TFrames sent;
        ASSERT_TRUE( 3 + 3 == PopAll( sender, sent ) );
        ASSERT_TRUE( IsTalkspurt( sent, 0, 3, 160 ) && IsTalkspurt( sent, 3, 3, 320 ) );
        ASSERT_TRUE( FirstTimestamp + 480 == sent[ 3 ].GetTimestamp() && 100 == sent[ 3 ].GetPayloadType() && 640 == sent[ 3 ].GetPayloadSize() );
        ASSERT_TRUE( 112 == sent[ 2 ].GetPayloadType() && (UInt16) ( FirstSequenceNumber + 3 ) == sent[ 3 ].GetSequenceNumber() );
        TSamples decoded;
        for ( size_t k=0; k<sent.size(); ++k )
            AppendPcmPayload( sent[ k ], decoded );
        ASSERT_TRUE( 1440 == decoded.size() && 0 == memcmp( &decoded[ 0 ], &signal[ 0 ], decoded.size() * sizeof( Float32 ) ) );
        AUDIO::CVoiceSenderStats stats;
        sender.GetStats( stats );
        ASSERT_TRUE( 1 == stats.codecSwitches && 2 == stats.talkspurts );

        // to Opus at 16 kHz: its RTP clock is 48 kHz, the timestamps go on from the position in the new rate
        if ( LoadOpus() )
        {
            AUDIO::CVoiceCodecConfig opus = VoiceCodecTest::MakeConfig( "opus", 16000, 10000 );
            ASSERT_TRUE( sender.SetCodecConfig( opus ) );
            FeedSender( sender, signal, 1440, 5 * 160, position, 16000, 160, false );
            sent.clear();
            ASSERT_TRUE( 5 == PopAll( sender, sent ) );
            ASSERT_TRUE( IsTalkspurt( sent, 0, 5, 480 ) && 111 == sent[ 0 ].GetPayloadType() );
            ASSERT_TRUE( FirstTimestamp + 1440 == sent[ 0 ].GetTimestamp() );
        }
        else
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioVoiceSender: the Opus plugin is not available, the change to Opus is not tested" );
        }

        // not prepared yet: the change is the configuration to prepare
        AUDIO::CVoiceSender fresh( config );
        ASSERT_TRUE( fresh.SetCodecConfig( longer ) && !fresh.IsCodecChangePending() && fresh.GetCodecConfig() == longer );
        ASSERT_TRUE( fresh.PrepareForCapture( MonoFormat( 16000 ), 1024 ) );
        UInt64 freshPosition = 0;
        FeedSender( fresh, signal, 0, 320, freshPosition, 16000, 160, false );
        AUDIO::CVoiceFrame frame;
        ASSERT_TRUE( fresh.PopFrame( frame ) && 100 == frame.GetPayloadType() && 640 == frame.GetPayloadSize() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
PerformEngineTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "AudioVoiceSender: As a capture consumer of the engine it sends what the capture path processed, and stops once removed" )
    try
    {
        const TSamples signal = MakePcmSignal( 16000, 16000 );
        AUDIO::CAudioEngine engine;
        ASSERT_TRUE( engine.Initialize( CaptureEngineParams( 16000 ) ) && engine.IsCaptureAvailable() );
        AUDIO::CNullAudioDevice* device = engine.GetCaptureDevice()->AsNullDevice();
        ASSERT_TRUE( GUCEF_NULL != device && device->SetCaptureSource( AUDIO::TAudioSourcePtr( GUCEF_NEW CSamplesSource( signal, 16000 ) ) ) );

        AUDIO::CVoiceSenderConfig config = SenderConfig( PcmConfig( 16000 ), AUDIO::VOICETRANSMITMODE_CONTINUOUS );
        config.SetOutboundQueueFrames( 256 );
        AUDIO::CVoiceSender sender( config );
        CORE::CFutureResult added = engine.AddCaptureConsumer( &sender );
        ASSERT_FALSE( added.HasNoFuture() );
        TFrames sent;
        for ( int p=0; p<100; ++p )
        {
            device->Render( 160 );
            engine.Update();
            PopAll( sender, sent );
        }
        CORE::CFutureResult removed = engine.RemoveCaptureConsumer( &sender );
        device->Render( 160 );
        engine.Update();
        ASSERT_TRUE( added.IsReady() && removed.IsReady() );
        PopAll( sender, sent );
        const size_t count = sent.size();
        device->Render( 1600 );
        engine.Update();
        ASSERT_FALSE( sender.PopFrame( sent[ 0 ] ) );

        ASSERT_TRUE( count >= 99 && IsTalkspurt( sent, 0, count, 160 ) );
        TSamples decoded;
        for ( size_t k=0; k<count; ++k )
            AppendPcmPayload( sent[ k ], decoded );
        ASSERT_TRUE( 0 == memcmp( &decoded[ 0 ], &signal[ 0 ], decoded.size() * sizeof( Float32 ) ) );      // equal rates: the path passes the audio bit for bit
        engine.Shutdown();
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioVoiceSender: Voice activated on the engine's detector: the pre-roll carries the first 10 ms of a burst, the sending stops after the hangovers" )
    try
    {
        // 1 s of faint noise, 1 s of a tone at -20 dBFS, 2 s of faint noise
        TSamples signal = AudioEffectTest::MakeNoise( 64000, 1, 5, 0.0003f );
        const size_t onset = 16000 + 37;
        for ( size_t n=onset; n<onset+16000; ++n )
            signal[ n ] += 0.1f * (Float32) sin( 2.0 * AudioEffectTest::Pi * 700.0 * (Float64) ( n - onset ) / 16000.0 );

        AUDIO::CAudioEngine engine;
        ASSERT_TRUE( engine.Initialize( CaptureEngineParams( 16000 ) ) && engine.IsCaptureAvailable() );
        AUDIO::CNullAudioDevice* device = engine.GetCaptureDevice()->AsNullDevice();
        ASSERT_TRUE( device->SetCaptureSource( AUDIO::TAudioSourcePtr( GUCEF_NEW CSamplesSource( signal, 16000 ) ) ) );
        AUDIO::CVoiceSenderConfig config = SenderConfig( PcmConfig( 16000 ), AUDIO::VOICETRANSMITMODE_VOICE_ACTIVATED );
        config.SetOutboundQueueFrames( 512 );
        AUDIO::CVoiceSender sender( config );
        CORE::CFutureResult added = engine.AddCaptureConsumer( &sender );
        TFrames sent;
        UInt32 lastActivePeriod = 0;
        for ( UInt32 p=0; p<400; ++p )
        {
            device->Render( 160 );
            engine.Update();
            if ( engine.IsCaptureVoiceActive() )
                lastActivePeriod = p;
            PopAll( sender, sent );
        }
        ASSERT_TRUE( added.IsReady() && !sent.empty() );
        ASSERT_TRUE( IsTalkspurt( sent, 0, sent.size(), 160 ) );

        // the sent audio begins at least a frame before the burst and holds its first 10 ms exactly
        const UInt64 firstSample = sent[ 0 ].GetTimestamp() - FirstTimestamp;
        ASSERT_TRUE( firstSample <= onset );
        TSamples decoded;
        for ( size_t k=0; k<sent.size(); ++k )
            AppendPcmPayload( sent[ k ], decoded );
        for ( size_t n=onset; n<onset+160; ++n )
            ASSERT_TRUE( fabsf( decoded[ n - firstSample ] - signal[ n ] ) <= 1.0f / 32768.0f );
        // the end: the detector's hangover (200 ms) and the sender's (300 ms) after the burst, no more
        const UInt64 endSample = firstSample + decoded.size();
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioVoiceSender: voice activated on the engine: burst " << onset << " to " << onset + 16000 << ", sent " << firstSample << " to " << endSample << ", detector active until period " << lastActivePeriod );
        ASSERT_TRUE( endSample >= onset + 16000 + 4800 && endSample <= onset + 16000 + 9600 );
        engine.RemoveCaptureConsumer( &sender );
        device->Render( 160 );
        engine.Update();
        engine.Shutdown();
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
PerformThreadTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "AudioVoiceSender: A capture thread, a polling network thread and controls from the owner work together (TSan)" )
    try
    {
        AUDIO::CVoiceSenderConfig config = SenderConfig( PcmConfig( 16000 ), AUDIO::VOICETRANSMITMODE_CONTINUOUS );
        config.SetRedundancy( 1 );
        config.SetOutboundQueueFrames( 8 );
        AUDIO::CVoiceSender sender( config );
        ASSERT_TRUE( sender.PrepareForCapture( MonoFormat( 16000 ), 1024 ) );
        const TSamples signal = MakePcmSignal( 16000, 16000 );

        CCaptureThread* capture = GUCEF_NEW CCaptureThread( sender, signal, 3000 );
        CNetworkThread* network = GUCEF_NEW CNetworkThread( sender );
        ASSERT_TRUE( network->Activate() && capture->Activate() );
        UInt32 rounds = 0;
        UInt32 switches = 0;
        AUDIO::CVoiceSenderStats stats;
        while ( 0 == capture->done.Load() && rounds < 100000 )
        {
            ++rounds;
            switch ( rounds % 6 )
            {
                case 0: sender.SetTransmitMode( AUDIO::VOICETRANSMITMODE_VOICE_ACTIVATED ); break;
                case 1: sender.SetTransmitting( 0 != ( rounds & 8 ) ); break;
                case 2: sender.SetTransmitMode( AUDIO::VOICETRANSMITMODE_PUSH_TO_TALK ); break;
                case 3: sender.SetMuted( 0 == ( rounds % 60 ) ); break;
                case 4: sender.SetBitrate( 16000 + rounds % 3 * 8000 ); break;
                default: sender.SetTransmitMode( AUDIO::VOICETRANSMITMODE_CONTINUOUS ); break;
            }
            if ( 0 == rounds % 50 && !sender.IsCodecChangePending() )
            {
                AUDIO::CVoiceCodecConfig codecConfig = PcmConfig( 16000, 0 == ( switches & 1 ) ? 20000 : 10000 );
                if ( sender.SetCodecConfig( codecConfig ) )
                    ++switches;
            }
            sender.GetStats( stats );
            sender.IsTransmitting();
            MT::PrecisionDelay( 1 );
        }
        for ( UInt32 i=0; i<10000 && 0 == capture->done.Load(); ++i )
            MT::ThreadDelay( 1 );
        network->stop.Store( 1 );
        network->WaitForThreadToFinish( 10000 );
        capture->WaitForThreadToFinish( 10000 );
        ASSERT_TRUE( 1 == capture->done.Load() );
        sender.GetStats( stats );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioVoiceSender: threads: " << stats.framesCaptured << " frames captured, " << stats.framesSent << " sent, " << stats.framesDropped << " dropped, " <<
                   network->popped.Load() << " popped, " << stats.codecSwitches << " codec changes of " << switches << ", " << stats.discontinuities << " discontinuities" );
        ASSERT_TRUE( network->popped.Load() == stats.framesSent );
        ASSERT_TRUE( 0 == network->outOfOrder.Load() );
        ASSERT_TRUE( stats.framesSent > 100 && stats.discontinuities >= 5 );
        ASSERT_TRUE( stats.codecSwitches + ( sender.IsCodecChangePending() ? 1 : 0 ) == switches );
        GUCEF_DELETE capture;
        GUCEF_DELETE network;
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
PerformRealtimeTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "AudioVoiceSender: The capture side allocates nothing and takes no lock, PCM and Opus in every mode (RealtimeSanitizer build)" )
    if ( !IsRtsanBuild )
    {
        GUCEF_TESTFW_SKIP( "not a RealtimeSanitizer build (-fsanitize=realtime)" );
    }
    GUCEF_TESTFW_SKIP_IF( GUCEF_TESTFW_GETENV( "GUCEF_TEST_AUDIO_RTSAN_CLEAN" ) == "0", "switched off with GUCEF_TEST_AUDIO_RTSAN_CLEAN=0" );
    try
    {
        // OnCapturedAudio() is a real time context: every call below is checked
        const bool opus = LoadOpus();
        for ( int c=0; c<( opus ? 3 : 1 ); ++c )
        {
            AUDIO::CVoiceCodecConfig codecConfig = PcmConfig( 16000 );
            if ( 1 == c )
                codecConfig = VoiceCodecTest::MakeConfig( "opus", 16000, 10000 );
            if ( 2 == c )
            {
                codecConfig = VoiceCodecTest::MakeConfig( "opus", 16000, 20000 );
                codecConfig.SetProfileDefaults( AUDIO::VOICECODECPROFILE_ROBUST );
                codecConfig.SetSampleRate( 16000 );
            }
            AUDIO::CVoiceSenderConfig config = SenderConfig( codecConfig, AUDIO::VOICETRANSMITMODE_CONTINUOUS );
            config.SetRedundancy( 1 );
            config.SetOutboundQueueFrames( 4 );
            AUDIO::CVoiceSender sender( config );
            ASSERT_TRUE( sender.PrepareForCapture( MonoFormat( 16000 ), 1024 ) );
            const TSamples signal = VoiceCodecTest::MakeSpeechLike( 32000, 16000, 0.5f );
            UInt64 position = 0;
            AudioEffectTest::CXorShift random( 3 );
            for ( int round=0; round<12; ++round )
            {
                sender.SetTransmitMode( (AUDIO::TVoiceTransmitMode) ( round % 3 ) );
                sender.SetTransmitting( 0 != ( round & 1 ) );
                sender.SetMuted( 7 == round );
                sender.SetBitrate( 16000 + round * 1000 );
                sender.SetExpectedLossPercentage( round );
                if ( 5 == round )
                    ASSERT_TRUE( sender.SetCodecConfig( PcmConfig( 16000, 20000 ) ) );
                FeedSender( sender, signal, 0, 2000, position, 16000, 333, 0 != ( round & 2 ), &random, 9 == round );
                TFrames sent;
                PopAll( sender, sent );
            }
            AUDIO::CVoiceSenderStats stats;
            sender.GetStats( stats );
            ASSERT_TRUE( stats.framesSent > 0 && stats.framesDropped > 0 && 1 == stats.codecSwitches );
        }
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

}

/*-------------------------------------------------------------------------*/

void
PerformAudioVoiceSenderTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING AudioVoiceSender TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "gucefAUDIO.VoiceSender" );

    PerformConfigTests();
    PerformFrameTests();
    PerformGatingTests();
    PerformCodecTests();
    PerformEngineTests();
    PerformThreadTests();
    PerformRealtimeTests();
}

/*-------------------------------------------------------------------------*/
