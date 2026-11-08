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

#ifndef GUCEF_AUDIO_TESTAPP_TESTAUDIOVOICESTREAMSUPPORT_H
#define GUCEF_AUDIO_TESTAPP_TESTAUDIOVOICESTREAMSUPPORT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <math.h>
#include <string.h>

#ifndef GUCEF_AUDIO_CVOICESENDER_H
#include "gucefAUDIO_CVoiceSender.h"
#define GUCEF_AUDIO_CVOICESENDER_H
#endif /* GUCEF_AUDIO_CVOICESENDER_H ? */

#ifndef GUCEF_AUDIO_CVOICERECEIVER_H
#include "gucefAUDIO_CVoiceReceiver.h"
#define GUCEF_AUDIO_CVOICERECEIVER_H
#endif /* GUCEF_AUDIO_CVOICERECEIVER_H ? */

#ifndef GUCEF_AUDIO_CRTPVOICEPACKETCODEC_H
#include "gucefAUDIO_CRtpVoicePacketCodec.h"
#define GUCEF_AUDIO_CRTPVOICEPACKETCODEC_H
#endif /* GUCEF_AUDIO_CRTPVOICEPACKETCODEC_H ? */

#include "TestAudioVoiceCodecSupport.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  What the tests of the voice sender and receiver share: a simulated clock, an exactly representable PCM test signal,
 *  feeding a sender as the capture path would and collecting its frames. Header only, inline.
 */
namespace VoiceStreamTest {

using namespace GUCEF;

typedef AudioEffectTest::TSamples TSamples;
typedef GUCEF::vector< AUDIO::CVoiceFrame > TFrames;

/*-------------------------------------------------------------------------*/

/**
 *  A clock that only moves when the test moves it. Capture tick counts are taken to be microseconds of the same clock
 *  (the tests hand the sender simulated microseconds as tick counts), so one-way delays come out in simulated time
 */
class CSimulatedVoiceClock : public AUDIO::CVoiceClock
{
    public:

    explicit CSimulatedVoiceClock( UInt64 startInUs = 1000000 ) : AUDIO::CVoiceClock(), m_now( startInUs ) {}

    virtual UInt64 GetTimeInUs( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_now.Load( MT::MEMORDER_ACQUIRE ); }

    virtual bool CaptureTickToTimeInUs( UInt64 tickCount, UInt64& timeInUs ) const GUCEF_VIRTUAL_OVERRIDE
    {
        timeInUs = tickCount;
        return true;
    }

    void SetTimeInUs( UInt64 timeInUs ) { m_now.Store( timeInUs, MT::MEMORDER_RELEASE ); }
    void Advance( UInt64 microseconds ) { m_now.FetchAdd( microseconds ); }

    private:

    MT::CTAtomic< UInt64 > m_now;
};

/*-------------------------------------------------------------------------*/

/** A sample of a test signal that 16 bit PCM carries exactly: two tones and an offset, never silent for a frame */
inline Float32
PcmSample( UInt64 n, UInt32 sampleRate )
{
    const Float64 t = (Float64) n / (Float64) sampleRate;
    const Float64 v = 9000.0 * sin( 2.0 * AudioEffectTest::Pi * 211.0 * t ) + 5000.0 * sin( 2.0 * AudioEffectTest::Pi * 1033.0 * t ) + 3000.0;
    return (Float32) floor( v + 0.5 ) / 32768.0f;
}

inline TSamples
MakePcmSignal( UInt32 frames, UInt32 sampleRate, UInt64 firstSample = 0 )
{
    TSamples signal( frames, 0.0f );
    for ( UInt32 n=0; n<frames; ++n )
        signal[ n ] = PcmSample( firstSample + n, sampleRate );
    return signal;
}

/*-------------------------------------------------------------------------*/

inline AUDIO::CVoiceCodecConfig
PcmConfig( UInt32 sampleRate, UInt32 frameDurationInUs = 10000 )
{
    return VoiceCodecTest::MakeConfig( "pcm", sampleRate, frameDurationInUs );
}

inline AUDIO::CAudioFormat
MonoFormat( UInt32 sampleRate )
{
    return AUDIO::CAudioFormat( sampleRate, 1, AUDIO::SAMPLETYPE_FLOAT32 );
}

/*-------------------------------------------------------------------------*/

/**
 *  Hands 'count' samples of 'signal' from 'from' on to the sender as the capture path would: blocks of 'blockFrames'
 *  (or, when 'randomBlocks', of 1 to 'blockFrames' frames from 'random'), positions from 'position' on, the tick count
 *  of a block is its position in microseconds plus 'tickOffsetInUs', 'voiceActive' for every block
 */
inline void
FeedSender( AUDIO::CVoiceSender& sender, const TSamples& signal, size_t from, size_t count, UInt64& position, UInt32 sampleRate,
            UInt32 blockFrames, bool voiceActive, AudioEffectTest::CXorShift* random = GUCEF_NULL, bool discontinuity = false,
            UInt64 tickOffsetInUs = 0 )
{
    size_t done = 0;
    bool first = true;
    while ( done < count )
    {
        UInt32 block = blockFrames;
        if ( GUCEF_NULL != random )
            block = 1 + random->Next() % blockFrames;
        if ( block > count - done )
            block = (UInt32) ( count - done );
        AUDIO::IAudioCaptureConsumer::SCaptureBlockInfo info;
        info.firstFrame = position;
        info.tickCount = position * 1000000ULL / sampleRate + tickOffsetInUs;
        info.discontinuity = first && discontinuity;
        info.voiceActive = voiceActive;
        sender.OnCapturedAudio( &signal[ from + done ], block, info );
        position += block;
        done += block;
        first = false;
    }
}

/** Takes every frame out of the sender's queue */
inline UInt32
PopAll( AUDIO::CVoiceSender& sender, TFrames& frames )
{
    UInt32 count = 0;
    AUDIO::CVoiceFrame frame;
    while ( sender.PopFrame( frame ) )
    {
        frames.push_back( frame );
        ++count;
    }
    return count;
}

/** Decodes a PCM payload (16 bit big endian) into floats, appended to 'out' */
inline void
AppendPcmPayload( const AUDIO::CVoiceFrame& frame, TSamples& out )
{
    const UInt8* p = frame.GetPayload();
    for ( UInt32 i=0; i+1<frame.GetPayloadSize(); i+=2 )
    {
        const Int16 v = (Int16) (UInt16) ( ( (UInt16) p[ i ] << 8 ) | p[ i + 1 ] );
        out.push_back( (Float32) v / 32768.0f );
    }
}

/*-------------------------------------------------------------------------*/

}; /* namespace VoiceStreamTest */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIO_TESTAPP_TESTAUDIOVOICESTREAMSUPPORT_H ? */
