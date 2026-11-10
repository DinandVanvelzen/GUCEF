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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN )
#include <emscripten.h>
#ifndef GUCEF_AUDIODRIVER_WEBAUDIO_H
#include "audiodriverWEBAUDIO.h"
#define GUCEF_AUDIODRIVER_WEBAUDIO_H
#endif /* GUCEF_AUDIODRIVER_WEBAUDIO_H ? */
#endif

#include "TestAudioWebAudio.h"

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

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN )

namespace {

/** Renders a quiet sine and counts what the browser asked for */
class CToneCallback : public AUDIO::IAudioDeviceCallback
{
    public:

    CToneCallback( UInt32 channels ) : frames( 0 ), lost( 0 ), m_channels( channels ), m_phase( 0.0 ) {}

    virtual void OnRenderAudio( void* buffer, UInt32 frameCount ) GUCEF_VIRTUAL_OVERRIDE
    {
        Float32* out = static_cast< Float32* >( buffer );
        for ( UInt32 f=0; f<frameCount; ++f )
        {
            const Float32 sample = 0.1f * (Float32) sin( m_phase );
            m_phase += 2.0 * 3.14159265358979 * 440.0 / 48000.0;
            for ( UInt32 c=0; c<m_channels; ++c )
                out[ f * m_channels + c ] = sample;
        }
        frames.FetchAdd( frameCount );
    }
    virtual void OnCaptureAudio( const void*, UInt32 ) GUCEF_VIRTUAL_OVERRIDE {}
    virtual void OnDeviceLost( void ) GUCEF_VIRTUAL_OVERRIDE { lost.FetchAdd( 1 ); }

    MT::CTAtomic< UInt32 > frames;
    MT::CTAtomic< UInt32 > lost;

    private:

    UInt32 m_channels;
    Float64 m_phase;
};

}

#endif

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformAudioWebAudioTests( void )
{GUCEF_TRACE;

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN )

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING Web Audio driver plugin TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "gucefAUDIO.WebAudio" );

    // On the web the plugin is linked in, the application calls its entry point itself
    const bool loaded = 0 != AUDIODRIVER::WEBAUDIO::audiodriverWEBAUDIO_GUCEFPlugin_Load( 0, GUCEF_NULL );
    AUDIO::CAudioDriverRegistry::TAudioDriverPtr driver;
    AUDIO::CAudioGlobal::Instance()->GetAudioDriverRegistry().TryFindDriver( "webaudio", driver );

    GUCEF_TESTFW_TESTCASE( "WebAudio: The plugin registers the webaudio driver with one default output" )
        if ( !loaded )
            GUCEF_TESTFW_SKIP( "this browser (or node) has no Web Audio API" );
        try
        {
            ASSERT_FALSE( driver.IsNULL() );
            ASSERT_TRUE( "webaudio" == driver->GetName() );
            AUDIO::TAudioDeviceInfoVector devices;
            ASSERT_TRUE( driver->EnumerateDevices( AUDIO::AUDIODIRECTION_PLAYBACK, devices ) );
            ASSERT_TRUE( 1 == devices.size() && devices[ 0 ].IsDefault() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "WebAudio: The browser pulls audio from the device callback while it runs, and not after it stopped" )
        if ( driver.IsNULL() )
            GUCEF_TESTFW_SKIP( "the webaudio driver is not available" );
        try
        {
            AUDIO::CAudioDeviceParams params;
            params.SetPreferredFormat( AUDIO::CAudioFormat( 48000, 2, AUDIO::SAMPLETYPE_FLOAT32 ) );
            CToneCallback tone( 2 );
            AUDIO::TAudioDevicePtr device;
            ASSERT_TRUE( driver->OpenDevice( params, &tone, device ) );
            ASSERT_TRUE( device->GetFormat().IsValid() && AUDIO::SAMPLETYPE_FLOAT32 == device->GetFormat().GetSampleType() );
            ASSERT_TRUE( 2 == device->GetFormat().GetChannelCount() );
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Web Audio context runs at " << device->GetFormat().GetSampleRate() << " Hz" );

            ASSERT_TRUE( device->Start() );
            MT::PrecisionDelay( 1000 );
            const UInt32 asked = tone.frames.Load();
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Web Audio asked for " << asked << " frames in a second" );
            ASSERT_TRUE( asked >= device->GetFormat().GetSampleRate() / 2 );

            ASSERT_TRUE( device->Stop() );
            ASSERT_FALSE( device->IsStarted() );
            const UInt32 atStop = tone.frames.Load();
            MT::PrecisionDelay( 300 );
            ASSERT_TRUE( atStop == tone.frames.Load() );
            ASSERT_TRUE( 0 == tone.lost.Load() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "WebAudio: The engine plays a tone ladder to its end through the browser" )
        if ( driver.IsNULL() )
            GUCEF_TESTFW_SKIP( "the webaudio driver is not available" );
        try
        {
            AUDIO::CAudioEngineParams engineParams;
            engineParams.SetDriverName( "webaudio" );
            engineParams.SetThreadingMode( AUDIO::AUDIOTHREADING_THREADED );
            AUDIO::CAudioEngine engine;
            ASSERT_TRUE( engine.Initialize( engineParams ) );
            const AUDIO::CAudioFormat format = engine.GetDeviceFormat();
            AUDIO::IAudioSource::TAudioSourcePtr ladder = AUDIO::CAudioTestSignals::MakeToneLadder( format.GetSampleRate(), format.GetChannelCount(), 100 );
            const UInt64 frames = ladder->GetLengthInFrames();
            AUDIO::TAudioVoicePtr voice = engine.CreateVoice( ladder );
            ASSERT_FALSE( voice.IsNULL() );
            ASSERT_FALSE( engine.Start( voice ).HasNoFuture() );
            CORE::CFutureResult ended = engine.GetEndedResult( voice );
            ASSERT_TRUE( engine.WaitFor( ended, (UInt32) ( frames * 1000 / format.GetSampleRate() ) + 3000 ) );
            engine.Shutdown();
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    AUDIODRIVER::WEBAUDIO::audiodriverWEBAUDIO_GUCEFPlugin_Unload();

    #endif
}

/*-------------------------------------------------------------------------*/
