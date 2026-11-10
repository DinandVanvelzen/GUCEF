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

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CTASKMANAGER_H
#include "gucefCORE_CTaskManager.h"
#define GUCEF_CORE_CTASKMANAGER_H
#endif /* GUCEF_CORE_CTASKMANAGER_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_CORE_CPLUGINCONTROL_H
#include "CPluginControl.h"
#define GUCEF_CORE_CPLUGINCONTROL_H
#endif /* GUCEF_CORE_CPLUGINCONTROL_H ? */

#include "TestAudioAlsa.h"

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

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )

namespace {

/** Plays silence and counts what it was asked for */
class CSilenceCallback : public AUDIO::IAudioDeviceCallback
{
    public:

    CSilenceCallback( void ) : frames( 0 ), lost( 0 ) {}
    virtual void OnRenderAudio( void*, UInt32 frameCount ) GUCEF_VIRTUAL_OVERRIDE { frames.FetchAdd( frameCount ); }
    virtual void OnCaptureAudio( const void*, UInt32 ) GUCEF_VIRTUAL_OVERRIDE {}
    virtual void OnDeviceLost( void ) GUCEF_VIRTUAL_OVERRIDE { lost.FetchAdd( 1 ); }

    MT::CTAtomic< UInt32 > frames;
    MT::CTAtomic< UInt32 > lost;
};

bool
LoadAlsaPlugin( void )
{
    #ifdef GUCEF_DEBUG_MODE
    const CORE::CString first = "$MODULEDIR$/audiodriverALSA_d";
    const CORE::CString second = "$MODULEDIR$/audiodriverALSA";
    #else
    const CORE::CString first = "$MODULEDIR$/audiodriverALSA";
    const CORE::CString second = "$MODULEDIR$/audiodriverALSA_d";
    #endif

    AUDIO::CAudioDriverRegistry::TAudioDriverPtr driver;
    if ( AUDIO::CAudioGlobal::Instance()->GetAudioDriverRegistry().TryFindDriver( "alsa", driver ) )
        return true;
    CORE::CPluginControl& pluginControl = CORE::CCoreGlobal::Instance()->GetPluginControl();
    return pluginControl.AddPluginFromDir( first, "test", true ) || pluginControl.AddPluginFromDir( second, "test", true );
}

/** Plays a source to its end through the engine, true when it ended */
bool
PlayToEnd( AUDIO::CAudioEngine& engine, const AUDIO::IAudioSource::TAudioSourcePtr& source )
{
    const UInt64 frames = source->GetLengthInFrames();
    const UInt32 rate = source->GetFormat().GetSampleRate();
    AUDIO::TAudioVoicePtr voice = engine.CreateVoice( source );
    if ( voice.IsNULL() )
        return false;
    CORE::CFutureResult started = engine.Start( voice );
    if ( started.HasNoFuture() )
        return false;
    CORE::CFutureResult ended = engine.GetEndedResult( voice );
    return engine.WaitFor( ended, (UInt32) ( frames * 1000 / rate ) + 3000 );
}

}

#endif

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformAudioAlsaTests( void )
{GUCEF_TRACE;

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING ALSA driver plugin TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "gucefAUDIO.ALSA" );

    const bool pluginLoaded = LoadAlsaPlugin();
    AUDIO::CAudioDriverRegistry::TAudioDriverPtr driver;
    AUDIO::CAudioGlobal::Instance()->GetAudioDriverRegistry().TryFindDriver( "alsa", driver );

    GUCEF_TESTFW_TESTCASE( "ALSA: The plugin registers the alsa driver, which lists the default playback device" )
        if ( !pluginLoaded )
            GUCEF_TESTFW_SKIP( "the ALSA plugin did not load, libasound is probably not on this machine" );
        try
        {
            ASSERT_FALSE( driver.IsNULL() );
            ASSERT_TRUE( "alsa" == driver->GetName() && driver->GetPriority() > 0 );

            AUDIO::TAudioDeviceInfoVector devices;
            ASSERT_TRUE( driver->EnumerateDevices( AUDIO::AUDIODIRECTION_PLAYBACK, devices ) );
            bool hasDefault = false;
            for ( size_t i=0; i<devices.size(); ++i )
                hasDefault = hasDefault || ( devices[ i ].IsDefault() && "default" == devices[ i ].GetId() );
            ASSERT_TRUE( hasDefault );
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ALSA lists " << devices.size() << " playback devices" );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Silence, so it is safe to run anywhere. Machines without a usable sound device skip it
    AUDIO::TAudioDevicePtr device;
    CSilenceCallback silence;
    AUDIO::CAudioDeviceParams params;
    params.SetBufferSizeInFrames( 512 );
    const bool opened = !driver.IsNULL() && driver->OpenDevice( params, &silence, device );

    GUCEF_TESTFW_TESTCASE( "ALSA: The default device opens with a negotiated format, plays silence and stops" )
        if ( !opened )
            GUCEF_TESTFW_SKIP( "no ALSA playback device could be opened" );
        try
        {
            ASSERT_TRUE( device->GetFormat().IsValid() );
            ASSERT_TRUE( device->GetPeriodSizeInFrames() > 0 && device->GetLatencyInFrames() >= device->GetPeriodSizeInFrames() );
            ASSERT_TRUE( device->Start() );
            MT::PrecisionDelay( 300 );
            ASSERT_TRUE( device->IsStarted() );
            ASSERT_TRUE( device->Stop() );
            ASSERT_FALSE( device->IsStarted() );

            // about 300 ms of audio was asked for, and the device kept up
            const UInt32 asked = silence.frames.Load();
            const UInt32 expected = device->GetFormat().GetSampleRate() * 3 / 10;
            ASSERT_TRUE( asked > expected / 2 && asked < expected * 3 );
            ASSERT_TRUE( 0 == silence.lost.Load() );
            AUDIO::CAudioPushDevice* push = ( device.IsNULL() ? GUCEF_NULL : device->AsPushDevice() );
            ASSERT_TRUE( GUCEF_NULL != push && push->GetTotalFramesPushed() == asked );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
    device.Unlink();

    // The "test my audio" check: not run unless asked for, since it makes sound. Set GUCEF_TEST_AUDIO_LISTEN=1, and
    // GUCEF_TEST_AUDIO_DEVICE to play on another ALSA device than the default ("null" runs it without sound)
    GUCEF_TESTFW_TESTCASE( "ALSA: Listening check, a tone ladder and then each channel in turn" )
        GUCEF_TESTFW_REQUIRE_ENV( listen, "GUCEF_TEST_AUDIO_LISTEN" );
        (void) listen;
        if ( driver.IsNULL() )
            GUCEF_TESTFW_SKIP( "the ALSA driver is not available" );
        try
        {
            AUDIO::CAudioEngineParams engineParams;
            engineParams.SetDriverName( "alsa" );
            engineParams.SetThreadingMode( AUDIO::AUDIOTHREADING_THREADED );
            const CORE::CString deviceId( GUCEF_TESTFW_GETENV( "GUCEF_TEST_AUDIO_DEVICE" ).c_str() );   // empty: the default device
            engineParams.GetDeviceParams().SetDeviceId( deviceId );
            AUDIO::CAudioEngine engine;
            ASSERT_TRUE( engine.Initialize( engineParams ) );

            const AUDIO::CAudioFormat format = engine.GetDeviceFormat();
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Listening check on " << format.GetSampleRate() << " Hz, " << format.GetChannelCount() << " channels" );
            ASSERT_TRUE( PlayToEnd( engine, AUDIO::CAudioTestSignals::MakeToneLadder( format.GetSampleRate(), format.GetChannelCount() ) ) );
            ASSERT_TRUE( PlayToEnd( engine, AUDIO::CAudioTestSignals::MakeChannelCheck( format.GetSampleRate(), format.GetChannelCount() ) ) );
            engine.Shutdown();
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    #endif
}

/*-------------------------------------------------------------------------*/
