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

#include <string.h>

#ifndef GUCEFAUDIO_H
#include "gucefAUDIO.h"
#define GUCEFAUDIO_H
#endif /* GUCEFAUDIO_H ? */

#ifndef GUCEF_AUDIO_CAUDIOENGINE_H
#include "gucefAUDIO_CAudioEngine.h"
#define GUCEF_AUDIO_CAUDIOENGINE_H
#endif /* GUCEF_AUDIO_CAUDIOENGINE_H ? */

#ifndef GUCEF_AUDIO_CAUDIOSTREAM_H
#include "gucefAUDIO_CAudioStream.h"
#define GUCEF_AUDIO_CAUDIOSTREAM_H
#endif /* GUCEF_AUDIO_CAUDIOSTREAM_H ? */

#ifndef GUCEF_AUDIO_CTONESEQUENCESOURCE_H
#include "gucefAUDIO_CToneSequenceSource.h"
#define GUCEF_AUDIO_CTONESEQUENCESOURCE_H
#endif /* GUCEF_AUDIO_CTONESEQUENCESOURCE_H ? */

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CLOGMANAGER_H
#include "CLogManager.h"
#define GUCEF_CORE_CLOGMANAGER_H
#endif /* GUCEF_CORE_CLOGMANAGER_H ? */

#ifndef GUCEF_CORE_CILOGGER_H
#include "CILogger.h"
#define GUCEF_CORE_CILOGGER_H
#endif /* GUCEF_CORE_CILOGGER_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestAudioTestEffect.h"
#include "TestAudioRealtime.h"

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

#if defined( __has_feature )
  #if __has_feature( realtime_sanitizer )
    #define GUCEF_AUDIO_TEST_RTSAN_BUILD
  #endif
#endif

/** Source that says nothing about real time safety: a source of the kind a codec plugin makes */
class CPlainSource : public AUDIO::IAudioSource
{
    public:

    CPlainSource( void ) : m_format( 48000, 1, AUDIO::SAMPLETYPE_FLOAT32 ), m_position( 0 ) {}

    virtual const AUDIO::CAudioFormat& GetFormat( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_format; }

    virtual UInt32 ReadFrames( void* destination, UInt32 frameCapacity ) GUCEF_VIRTUAL_OVERRIDE
    {
        memset( destination, 0, (size_t) frameCapacity * sizeof( Float32 ) );
        m_position += frameCapacity;
        return frameCapacity;
    }

    virtual bool IsEndOfStream( void ) const GUCEF_VIRTUAL_OVERRIDE { return false; }
    virtual UInt64 GetPositionInFrames( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_position; }

    private:

    AUDIO::CAudioFormat m_format;
    UInt64 m_position;
};

/** Keeps the text of every log message that is logged while it is attached to the log manager */
class CLogCapture : public CORE::CILogger
{
    public:

    CLogCapture( void ) : CORE::CILogger(), m_text(), m_lock()
    {
        CORE::CCoreGlobal::Instance()->GetLogManager().AddLogger( this );
    }

    virtual ~CLogCapture()
    {
        CORE::CCoreGlobal::Instance()->GetLogManager().RemoveLogger( this );
    }

    /** logs that are still on their way to the loggers are delivered first */
    CORE::CString GetText( void )
    {
        CORE::CCoreGlobal::Instance()->GetLogManager().FlushLogs();
        MT::CScopeMutex lock( m_lock );
        return m_text;
    }

    virtual void Log( const CORE::TLogMsgType logMsgType, const Int32 logLevel, const CORE::CString& logMessage, const UInt32 threadId, const CORE::CTimestamp& timestamp ) GUCEF_VIRTUAL_OVERRIDE
    {
        MT::CScopeMutex lock( m_lock );
        m_text += logMessage;
        m_text += "\n";
    }

    virtual void Log( const CORE::TLogMsgType logMsgType, const Int32 logLevel, const CORE::CVariantStream& logMessage, const UInt32 threadId, const CORE::CTimestamp& timestamp ) GUCEF_VIRTUAL_OVERRIDE
    {
        Log( logMsgType, logLevel, logMessage.ToString(), threadId, timestamp );
    }

    virtual void LogWithoutFormatting( const CORE::TLogMsgType logMsgType, const Int32 logLevel, const CORE::CString& logMessage, const UInt32 threadId, const CORE::CTimestamp& timestamp ) GUCEF_VIRTUAL_OVERRIDE
    {
        Log( logMsgType, logLevel, logMessage, threadId, timestamp );
    }

    virtual void LogWithoutFormatting( const CORE::TLogMsgType logMsgType, const Int32 logLevel, const CORE::CVariantStream& logMessage, const UInt32 threadId, const CORE::CTimestamp& timestamp ) GUCEF_VIRTUAL_OVERRIDE
    {
        Log( logMsgType, logLevel, logMessage.ToString(), threadId, timestamp );
    }

    virtual void FlushLog( void ) GUCEF_VIRTUAL_OVERRIDE {}

    virtual void SetMinimalLogLevel( const Int32 logLevel ) GUCEF_VIRTUAL_OVERRIDE {}

    virtual Int32 GetMinimalLogLevel( void ) const GUCEF_VIRTUAL_OVERRIDE { return CORE::LOGLEVEL_EVERYTHING; }

    private:

    CORE::CString m_text;
    MT::CMutex m_lock;
};

AUDIO::CAudioEngineParams
NullParams( void )
{
    AUDIO::CAudioEngineParams params;
    params.SetDriverName( "null" );
    params.SetThreadingMode( AUDIO::AUDIOTHREADING_COOPERATIVE );
    params.GetDeviceParams().SetBufferSizeInFrames( 256 );
    params.GetDeviceParams().GetOptions().Set( "keepOutput", "true" );
    return params;
}

AUDIO::TAudioClipPtr
MakeClip( UInt32 frames )
{
    AUDIO::CAudioBuffer audio( AUDIO::CAudioFormat( 48000, 2, AUDIO::SAMPLETYPE_FLOAT32 ), frames );
    for ( UInt32 f=0; f<frames; ++f )
        for ( UInt32 c=0; c<2; ++c )
            audio.SetSampleFromFloat( f, c, 0.25f );
    return AUDIO::TAudioClipPtr( GUCEF_NEW AUDIO::CAudioClip( audio ) );
}

const char* const WarningText = "not real time safe";

bool
Contains( const CORE::CString& text, const char* needle )
{
    return text.HasSubstr( needle ) >= 0;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

static void
PerformSourceDeclarationTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "AudioRealtime: IsRealtimeSafe is false by default and true for clip, stream and tone sequence sources" )
    try
    {
        CPlainSource plain;
        ASSERT_FALSE( plain.IsRealtimeSafe() );

        AUDIO::TAudioClipPtr clip = MakeClip( 1000 );
        AUDIO::TAudioSourcePtr clipSource = clip->CreateSource();
        ASSERT_TRUE( clipSource->IsRealtimeSafe() );

        // a stream, with nothing that reads ahead on a task: it is the stream that matters, not what it wraps
        AUDIO::CAudioStream wrapped( AUDIO::TAudioSourcePtr( GUCEF_NEW CPlainSource ), 100, 1, false );
        ASSERT_TRUE( wrapped.IsRealtimeSafe() );

        AUDIO::TAudioSourcePtr tone = AUDIO::CAudioTestSignals::MakeTone( 48000, 2, 440.0, 100 );
        ASSERT_TRUE( tone->IsRealtimeSafe() );
        AUDIO::TAudioSourcePtr noise = AUDIO::CAudioTestSignals::MakeNoise( 48000, 2, 100 );
        ASSERT_TRUE( noise->IsRealtimeSafe() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
PerformWarningTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "AudioRealtime: CreateVoice warns on the owner thread for a source that is not real time safe, and only then" )
    try
    {
        AUDIO::CAudioEngine engine;
        ASSERT_TRUE( engine.Initialize( NullParams() ) );

        {
            CLogCapture capture;
            AUDIO::TAudioVoicePtr safe = engine.CreateVoice( MakeClip( 1000 ) );
            ASSERT_FALSE( safe.IsNULL() );
            AUDIO::TAudioVoicePtr tone = engine.CreateVoice( AUDIO::CAudioTestSignals::MakeTone( 48000, 2, 440.0, 100 ) );
            ASSERT_FALSE( tone.IsNULL() );
            AUDIO::TAudioVoicePtr stream = engine.CreateVoice( AUDIO::TAudioSourcePtr( GUCEF_NEW AUDIO::CAudioStream( AUDIO::TAudioSourcePtr( GUCEF_NEW CPlainSource ), 100, 1, false ) ) );
            ASSERT_FALSE( stream.IsNULL() );
            ASSERT_FALSE( Contains( capture.GetText(), WarningText ) );
        }

        {
            CLogCapture capture;
            AUDIO::TAudioVoicePtr plain = engine.CreateVoice( AUDIO::TAudioSourcePtr( GUCEF_NEW CPlainSource ) );
            ASSERT_FALSE( plain.IsNULL() );     // the warning does not change what happens
            ASSERT_TRUE( Contains( capture.GetText(), WarningText ) );
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
PerformRenderPathTests( void )
{GUCEF_TRACE;

    // The render callback is marked as a real time context in a RealtimeSanitizer build: any allocation, lock or blocking call made
    // while rendering aborts the run. The render covers the commands of every kind (bus, voice, effect, send), effects in bus slots,
    // sends, meters and the release of an effect, which the audio thread only reports. GUCEF_TEST_AUDIO_RTSAN_CLEAN=0 skips it
    GUCEF_TESTFW_TESTCASE( "AudioRealtime: The render path does nothing a real time thread must not do (RealtimeSanitizer build)" )
    #ifndef GUCEF_AUDIO_TEST_RTSAN_BUILD
    GUCEF_TESTFW_SKIP( "not a RealtimeSanitizer build (-fsanitize=realtime)" );
    #endif
    GUCEF_TESTFW_SKIP_IF( GUCEF_TESTFW_GETENV( "GUCEF_TEST_AUDIO_RTSAN_CLEAN" ) == "0", "switched off with GUCEF_TEST_AUDIO_RTSAN_CLEAN=0" );
    try
    {
        AUDIO::CAudioEngine engine;
        ASSERT_TRUE( engine.Initialize( NullParams() ) );

        // a bus created while running reaches the audio thread as an attach command with a bus order, a clip as a start command,
        // an effect as an insert command, a send as a send command with a new bus order
        AUDIO::CAudioBus* bus = engine.CreateBus( "effects" );
        AUDIO::CAudioBus* aux = engine.CreateBus( "aux" );
        ASSERT_TRUE( GUCEF_NULL != bus && GUCEF_NULL != aux );
        AUDIO::TAudioEffectPtr effect( GUCEF_NEW CTestAudioEffect() );
        ASSERT_TRUE( effect->Prepare( engine.GetMixFormat(), 4096 ) );
        CORE::CFutureResult inserted = engine.InsertEffect( bus, 0, effect );
        CORE::CFutureResult sent = engine.AddSend( bus, aux, 0.5f, false );
        AUDIO::TAudioVoicePtr voice;
        engine.Play( MakeClip( 4800 ), bus, &voice );

        // under the sanitizer the first unsafe call aborts the run here
        engine.GetDevice()->AsNullDevice()->Render( 1024 );
        engine.Update();
        ASSERT_TRUE( inserted.IsReady() && sent.IsReady() );

        // parameters change, a slot is switched off and on (crossfade, reset), the effect is taken out again
        effect->SetParameter( CTestAudioEffect::PARAMETER_CUTOFF, 1000.0f );
        bus->SetEffectEnabled( 0, false );
        engine.GetDevice()->AsNullDevice()->Render( 1024 );
        bus->SetEffectEnabled( 0, true );
        engine.GetDevice()->AsNullDevice()->Render( 1024 );
        CORE::CFutureResult removed = engine.RemoveEffect( bus, effect );
        effect.Unlink();
        engine.GetDevice()->AsNullDevice()->Render( 1024 );
        engine.Update();
        ASSERT_TRUE( removed.IsReady() );
        ASSERT_TRUE( aux->GetMeter().GetPeak( 0 ) >= 0.0f );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    // The master limiter stage in each mode, loud enough to limit, and a change of the mode with a device reopen: the reopen
    // itself runs on this thread (owner side, it may allocate), the renders before and after it are checked
    GUCEF_TESTFW_TESTCASE( "AudioRealtime: The master limiter renders in every mode and after a mode change by reopen without anything a real time thread must not do" )
    #ifndef GUCEF_AUDIO_TEST_RTSAN_BUILD
    GUCEF_TESTFW_SKIP( "not a RealtimeSanitizer build (-fsanitize=realtime)" );
    #endif
    GUCEF_TESTFW_SKIP_IF( GUCEF_TESTFW_GETENV( "GUCEF_TEST_AUDIO_RTSAN_CLEAN" ) == "0", "switched off with GUCEF_TEST_AUDIO_RTSAN_CLEAN=0" );
    try
    {
        const AUDIO::TAudioLimiterMode modes[ 3 ] = { AUDIO::AUDIOLIMITERMODE_LOOKAHEAD, AUDIO::AUDIOLIMITERMODE_INSTANT, AUDIO::AUDIOLIMITERMODE_OFF };
        for ( UInt32 m=0; m<3; ++m )
        {
            AUDIO::CAudioEngine engine;
            AUDIO::CAudioEngineParams params = NullParams();
            params.SetLimiterMode( modes[ m ] );
            ASSERT_TRUE( engine.Initialize( params ) );
            for ( UInt32 v=0; v<6; ++v )
                engine.Play( MakeClip( 9600 ) );                                     // six times a quarter: over full level
            engine.GetDevice()->AsNullDevice()->Render( 2048 );
            engine.Update();

            // to the next mode, with the lookahead changed as well
            AUDIO::CAudioEngineParams next = engine.GetParams();
            next.SetLimiterMode( modes[ ( m + 1 ) % 3 ] );
            next.SetLimiterLookaheadInMs( 3.0f );
            ASSERT_TRUE( engine.ReopenDevice( next ) );
            engine.GetDevice()->AsNullDevice()->Render( 2048 );
            engine.Update();
            ASSERT_TRUE( modes[ ( m + 1 ) % 3 ] == engine.GetParams().GetLimiterMode() );
            engine.Shutdown();
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
PerformAudioRealtimeTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING AudioRealtime TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "gucefAUDIO.Realtime" );

    PerformSourceDeclarationTests();
    PerformWarningTests();
    PerformRenderPathTests();
}

/*-------------------------------------------------------------------------*/
