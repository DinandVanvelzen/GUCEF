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

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CTASKMANAGER_H
#include "gucefCORE_CTaskManager.h"
#define GUCEF_CORE_CTASKMANAGER_H
#endif /* GUCEF_CORE_CTASKMANAGER_H ? */

#include "TestAudioDevices.h"

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
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** Plays a rising ramp of float samples and notes every call it gets */
class CRampCallback : public AUDIO::IAudioDeviceCallback
{
    public:

    CRampCallback( void ) : renderCalls(), captureCalls(), lost( 0 ), next( 0.0f ), channels( 2 ), capturedPeak( 0.0f ) {}

    virtual void OnRenderAudio( void* buffer, UInt32 frameCount ) GUCEF_VIRTUAL_OVERRIDE
    {
        renderCalls.push_back( frameCount );
        Float32* out = static_cast< Float32* >( buffer );
        for ( UInt32 f=0; f<frameCount; ++f )
        {
            for ( UInt32 c=0; c<channels; ++c )
                out[ f * channels + c ] = next;
            next += 0.001f;
        }
    }

    virtual void OnCaptureAudio( const void* buffer, UInt32 frameCount ) GUCEF_VIRTUAL_OVERRIDE
    {
        captureCalls.push_back( frameCount );
        const Float32* in = static_cast< const Float32* >( buffer );
        for ( UInt32 i=0; i<frameCount * channels; ++i )
            if ( fabsf( in[ i ] ) > capturedPeak )
                capturedPeak = fabsf( in[ i ] );
    }

    virtual void OnDeviceLost( void ) GUCEF_VIRTUAL_OVERRIDE { ++lost; }

    GUCEF::vector< UInt32 > renderCalls;
    GUCEF::vector< UInt32 > captureCalls;
    UInt32 lost;
    Float32 next;
    UInt32 channels;
    Float32 capturedPeak;
};

/**
 *  A push device whose backend is a script: it can refuse to start, report an underrun or the loss of the device at a
 *  given write, and it notes the thread its writes run on. Each write takes a millisecond, as a blocking backend would
 */
class CScriptedPushDevice : public AUDIO::CAudioPushDevice
{
    public:

    CScriptedPushDevice( AUDIO::IAudioDeviceCallback* callback, bool canStart, UInt32 underrunAtPush, UInt32 lostAtPush, AUDIO::TAudioDirection direction = AUDIO::AUDIODIRECTION_PLAYBACK )
        : AUDIO::CAudioPushDevice( AUDIO::CAudioDeviceInfo( "scripted", "Scripted push device", direction ), callback, AUDIO::CNullAudioDriver::GetDefaultFormat(), 64 )
        , canStart( canStart ), underrunAtPush( underrunAtPush ), lostAtPush( lostAtPush ), pushes( 0 ), framesPushed( 0 ), pushThreadId( 0 ), starts( 0 ), stops( 0 )
        , threadStartFails( false ), threadStarts( 0 ), threadEnds( 0 ), threadStartThreadId( 0 ), threadEndThreadId( 0 )
    {}

    virtual ~CScriptedPushDevice() GUCEF_VIRTUAL_OVERRIDE
    {
        Stop();
    }

    bool canStart;
    UInt32 underrunAtPush;
    UInt32 lostAtPush;
    MT::CTAtomic< UInt32 > pushes;
    MT::CTAtomic< UInt32 > framesPushed;
    MT::CTAtomic< UInt32 > pushThreadId;
    UInt32 starts;
    UInt32 stops;
    bool threadStartFails;
    MT::CTAtomic< UInt32 > threadStarts;
    MT::CTAtomic< UInt32 > threadEnds;
    MT::CTAtomic< UInt32 > threadStartThreadId;
    MT::CTAtomic< UInt32 > threadEndThreadId;

    protected:

    virtual bool OnDeviceThreadStart( void ) GUCEF_VIRTUAL_OVERRIDE
    {
        threadStartThreadId.Store( MT::GetCurrentTaskID() );
        threadStarts.FetchAdd( 1 );
        return !threadStartFails;
    }

    virtual void OnDeviceThreadEnd( void ) GUCEF_VIRTUAL_OVERRIDE
    {
        threadEndThreadId.Store( MT::GetCurrentTaskID() );
        threadEnds.FetchAdd( 1 );
    }

    virtual bool OnPushStart( void ) GUCEF_VIRTUAL_OVERRIDE
    {
        ++starts;
        return canStart;
    }

    virtual TPushResult PushFrames( const void*, UInt32 frameCount ) GUCEF_VIRTUAL_OVERRIDE
    {
        pushThreadId.Store( MT::GetCurrentTaskID() );
        const UInt32 push = pushes.FetchAdd( 1 ) + 1;
        MT::PrecisionDelay( 1 );
        if ( push == lostAtPush )
            return PUSHRESULT_DEVICE_LOST;
        framesPushed.FetchAdd( frameCount );
        return push == underrunAtPush ? PUSHRESULT_UNDERRUN_RECOVERED : PUSHRESULT_OK;
    }

    virtual void OnPushStop( void ) GUCEF_VIRTUAL_OVERRIDE
    {
        ++stops;
    }
};

bool
WaitForPushes( const CScriptedPushDevice& device, UInt32 count, UInt32 timeoutInMs )
{
    UInt32 waited = 0;
    while ( device.pushes.Load() < count && waited < timeoutInMs )
    {
        MT::PrecisionDelay( 2 );
        waited += 2;
    }
    return device.pushes.Load() >= count;
}

/** A driver for the registry tests: refuses or accepts everything, at the priority it was made with */
class CFakeDriver : public AUDIO::CAudioDriver
{
    public:

    CFakeDriver( const CORE::CString& name, Int32 priority, bool accepts ) : CAudioDriver(), m_name( name ), m_priority( priority ), m_accepts( accepts ), openCount( 0 ) {}

    virtual CORE::CString GetName( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_name; }
    virtual Int32 GetPriority( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_priority; }

    virtual bool EnumerateDevices( AUDIO::TAudioDirection direction, AUDIO::TAudioDeviceInfoVector& devices ) GUCEF_VIRTUAL_OVERRIDE
    {
        AUDIO::CAudioDeviceInfo first( "a", "first", direction ), second( "b", "second", direction );
        second.SetIsDefault( true );
        devices.push_back( first );
        devices.push_back( second );
        return true;
    }

    virtual bool OpenDevice( const AUDIO::CAudioDeviceParams& params, AUDIO::IAudioDeviceCallback* callback, AUDIO::TAudioDevicePtr& device ) GUCEF_VIRTUAL_OVERRIDE
    {
        ++openCount;
        if ( !m_accepts )
            return false;
        AUDIO::CAudioDeviceInfo info( "fake", "fake device", params.GetDirection() );
        device = AUDIO::TAudioDevicePtr( GUCEF_NEW AUDIO::CNullAudioDevice( info, callback, AUDIO::CNullAudioDriver::GetDefaultFormat(), 256, false ) );
        return true;
    }

    CORE::CString m_name;
    Int32 m_priority;
    bool m_accepts;
    UInt32 openCount;
};

AUDIO::CNullAudioDevice*
AsNull( const AUDIO::TAudioDevicePtr& device )
{
    AUDIO::TAudioDevicePtr copy( device );
    return ( copy.IsNULL() ? GUCEF_NULL : copy->AsNullDevice() );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

static void
PerformInfoTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "AudioDevice: Info and parameters hold what they are given and copy independently" )
    try
    {
        AUDIO::CAudioDeviceInfo info( "hw:0", "Speakers", AUDIO::AUDIODIRECTION_CAPTURE );
        info.SetIsDefault( true );
        info.SetPreferredFormat( AUDIO::CAudioFormat( 44100, 2, AUDIO::SAMPLETYPE_INT16 ) );
        info.SetMaxChannelCount( 8 );
        info.GetProperties().Set( "bus", "usb" );
        AUDIO::CAudioDeviceInfo copy( info );
        info.SetName( "changed" );
        info.GetProperties().Set( "bus", "pci" );
        ASSERT_TRUE( copy.GetId() == "hw:0" && copy.GetName() == "Speakers" && copy.IsDefault() && 8 == copy.GetMaxChannelCount() );
        ASSERT_TRUE( AUDIO::AUDIODIRECTION_CAPTURE == copy.GetDirection() );
        ASSERT_TRUE( copy.GetPreferredFormat() == AUDIO::CAudioFormat( 44100, 2, AUDIO::SAMPLETYPE_INT16 ) );
        ASSERT_TRUE( copy.GetProperties().GetValueAlways( "bus" ).AsString() == "usb" );

        AUDIO::CAudioDeviceParams params;
        ASSERT_TRUE( params.GetDeviceId().IsNULLOrEmpty() && AUDIO::AUDIODIRECTION_PLAYBACK == params.GetDirection() );
        ASSERT_TRUE( !params.GetPreferredFormat().IsValid() && 0 == params.GetBufferSizeInFrames() && AUDIO::AUDIOSHAREMODE_SHARED == params.GetShareMode() );
        params.SetDeviceId( "x" );
        params.SetBufferSizeInFrames( 128 );
        params.SetShareMode( AUDIO::AUDIOSHAREMODE_EXCLUSIVE );
        params.GetOptions().Set( "latency", "low" );
        AUDIO::CAudioDeviceParams other;
        other = params;
        ASSERT_TRUE( other.GetDeviceId() == "x" && 128 == other.GetBufferSizeInFrames() && AUDIO::AUDIOSHAREMODE_EXCLUSIVE == other.GetShareMode() );
        ASSERT_TRUE( other.GetOptions().GetValueAlways( "latency" ).AsString() == "low" );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
PerformNullDriverTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "NullDriver: Is registered, lists one default device, opens with defaults or the asked format" )
    try
    {
        AUDIO::CAudioDriverRegistry& registry = AUDIO::CAudioGlobal::Instance()->GetAudioDriverRegistry();
        AUDIO::CAudioDriverRegistry::TAudioDriverPtr driver;
        ASSERT_TRUE( registry.TryFindDriver( "NULL", driver ) );
        ASSERT_TRUE( driver->GetName() == "null" && driver->GetPriority() < 0 );

        AUDIO::TAudioDeviceInfoVector devices;
        ASSERT_TRUE( driver->EnumerateDevices( AUDIO::AUDIODIRECTION_PLAYBACK, devices ) && 1 == devices.size() );
        ASSERT_TRUE( devices[ 0 ].IsDefault() && devices[ 0 ].GetDirection() == AUDIO::AUDIODIRECTION_PLAYBACK );
        AUDIO::CAudioDeviceInfo defaultDevice;
        ASSERT_TRUE( driver->GetDefaultDevice( AUDIO::AUDIODIRECTION_CAPTURE, defaultDevice ) && defaultDevice.GetId() == "null" && defaultDevice.GetDirection() == AUDIO::AUDIODIRECTION_CAPTURE );

        CRampCallback callback;
        AUDIO::TAudioDevicePtr device;
        ASSERT_TRUE( driver->OpenDevice( AUDIO::CAudioDeviceParams(), &callback, device ) );
        ASSERT_TRUE( device->GetFormat() == AUDIO::CAudioFormat( 48000, 2, AUDIO::SAMPLETYPE_FLOAT32 ) );
        ASSERT_FALSE( device->IsStarted() );

        AUDIO::CAudioDeviceParams wanted;
        wanted.SetPreferredFormat( AUDIO::CAudioFormat( 22050, 1, AUDIO::SAMPLETYPE_INT16 ) );
        wanted.SetBufferSizeInFrames( 100 );
        AUDIO::TAudioDevicePtr small;
        ASSERT_TRUE( driver->OpenDevice( wanted, &callback, small ) );
        ASSERT_TRUE( small->GetFormat() == AUDIO::CAudioFormat( 22050, 1, AUDIO::SAMPLETYPE_INT16 ) );
        ASSERT_TRUE( 100 == small->GetPeriodSizeInFrames() );

        // only some parts of the format asked for: the rest is the default
        AUDIO::CAudioDeviceParams partial;
        partial.SetPreferredFormat( AUDIO::CAudioFormat( 96000, 0, AUDIO::SAMPLETYPE_UNKNOWN ) );
        AUDIO::TAudioDevicePtr rateOnly;
        ASSERT_TRUE( driver->OpenDevice( partial, &callback, rateOnly ) );
        ASSERT_TRUE( rateOnly->GetFormat() == AUDIO::CAudioFormat( 96000, 2, AUDIO::SAMPLETYPE_FLOAT32 ) );

        AUDIO::CAudioDeviceParams wrong;
        wrong.SetDeviceId( "hw:9" );
        AUDIO::TAudioDevicePtr none;
        ASSERT_FALSE( driver->OpenDevice( wrong, &callback, none ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "NullDriver: Render calls the callback in device sized pieces, only while started" )
    try
    {
        CRampCallback callback;
        AUDIO::CAudioDeviceParams params;
        params.SetBufferSizeInFrames( 512 );
        params.GetOptions().Set( "keepOutput", "true" );
        AUDIO::TAudioDevicePtr device;
        CORE::CString driverName;
        ASSERT_TRUE( AUDIO::CAudioGlobal::Instance()->GetAudioDriverRegistry().OpenDevice( "null", params, &callback, device, driverName ) );
        ASSERT_TRUE( driverName == "null" );
        AUDIO::CNullAudioDevice* null = AsNull( device );
        ASSERT_TRUE( GUCEF_NULL != null );

        ASSERT_TRUE( 0 == null->Render( 1000 ) );                       // not started: nothing happens
        ASSERT_TRUE( callback.renderCalls.empty() );

        ASSERT_TRUE( device->Start() && device->IsStarted() );
        ASSERT_TRUE( 1000 == null->Render( 1000 ) );
        ASSERT_TRUE( 2 == callback.renderCalls.size() && 512 == callback.renderCalls[ 0 ] && 488 == callback.renderCalls[ 1 ] );
        ASSERT_TRUE( 1000 == null->GetRenderedAudio().GetFrameCount() );
        ASSERT_TRUE( 1000 == null->GetTotalFramesRendered() );
        ASSERT_TRUE( fabsf( null->GetRenderedAudio().GetSampleAsFloat( 700, 1 ) - 0.7f ) < 1e-4f );     // the ramp is what came out, in order

        ASSERT_TRUE( device->Stop() && !device->IsStarted() );
        ASSERT_TRUE( 0 == null->Render( 100 ) );
        ASSERT_TRUE( 1000 == null->GetRenderedAudio().GetFrameCount() );

        null->ClearRenderedAudio();
        ASSERT_TRUE( null->GetRenderedAudio().IsEmpty() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "NullDriver: A capture device records silence, and device loss is reported" )
    try
    {
        CRampCallback callback;
        AUDIO::CAudioDeviceParams params;
        params.SetDirection( AUDIO::AUDIODIRECTION_CAPTURE );
        params.SetBufferSizeInFrames( 256 );
        AUDIO::TAudioDevicePtr device;
        CORE::CString driverName;
        ASSERT_TRUE( AUDIO::CAudioGlobal::Instance()->GetAudioDriverRegistry().OpenDevice( "", params, &callback, device, driverName ) );
        ASSERT_TRUE( AUDIO::AUDIODIRECTION_CAPTURE == device->GetDirection() );
        ASSERT_TRUE( device->Start() );
        ASSERT_TRUE( 600 == AsNull( device )->Render( 600 ) );
        ASSERT_TRUE( callback.renderCalls.empty() );
        ASSERT_TRUE( 3 == callback.captureCalls.size() && 256 == callback.captureCalls[ 0 ] && 88 == callback.captureCalls[ 2 ] );
        ASSERT_TRUE( 0.0f == callback.capturedPeak );

        AsNull( device )->SimulateDeviceLoss();
        ASSERT_TRUE( 1 == callback.lost );
        ASSERT_FALSE( device->IsStarted() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
PerformRegistryTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "AudioDriverRegistry: The best driver that manages to open the device is used, or the one named" )
    try
    {
        AUDIO::CAudioDriverRegistry& registry = AUDIO::CAudioGlobal::Instance()->GetAudioDriverRegistry();
        CFakeDriver* refusing = GUCEF_NEW CFakeDriver( "refuser", 50, false );
        CFakeDriver* good = GUCEF_NEW CFakeDriver( "goodone", 20, true );
        CFakeDriver* better = GUCEF_NEW CFakeDriver( "betterone", 30, true );
        ASSERT_TRUE( registry.TryRegister( "refuser", AUDIO::TAudioDriverPtr( refusing ) ) );
        ASSERT_TRUE( registry.TryRegister( "goodone", AUDIO::TAudioDriverPtr( good ) ) );
        ASSERT_FALSE( registry.TryRegister( "goodone", AUDIO::TAudioDriverPtr( GUCEF_NEW CFakeDriver( "dup", 1, true ) ) ) );

        GUCEF::vector< AUDIO::CAudioDriverRegistry::TAudioDriverPtr > ordered;
        registry.GetDriversByPriority( ordered );
        ASSERT_TRUE( 3 == ordered.size() );
        ASSERT_TRUE( ordered[ 0 ]->GetName() == "refuser" && ordered[ 1 ]->GetName() == "goodone" && ordered[ 2 ]->GetName() == "null" );

        CRampCallback callback;
        AUDIO::TAudioDevicePtr device;
        CORE::CString used;
        ASSERT_TRUE( registry.OpenDevice( "", AUDIO::CAudioDeviceParams(), &callback, device, used ) );
        ASSERT_TRUE( used == "goodone" );                                   // the refuser was tried first and gave way
        ASSERT_TRUE( 1 == refusing->openCount && 1 == good->openCount );

        ASSERT_TRUE( registry.TryRegister( "betterone", AUDIO::TAudioDriverPtr( better ) ) );
        ASSERT_TRUE( registry.OpenDevice( "", AUDIO::CAudioDeviceParams(), &callback, device, used ) );
        ASSERT_TRUE( used == "betterone" );                              // the refuser (50) is still first but gives way, the better one (30) beats the good one (20)
        registry.GetDriversByPriority( ordered );
        ASSERT_TRUE( ordered[ 1 ]->GetName() == "betterone" && ordered[ 2 ]->GetName() == "goodone" );

        ASSERT_TRUE( registry.OpenDevice( "GoodOne", AUDIO::CAudioDeviceParams(), &callback, device, used ) && used == "goodone" );      // by name, any case
        ASSERT_FALSE( registry.OpenDevice( "refuser", AUDIO::CAudioDeviceParams(), &callback, device, used ) );                           // named and refusing: no fallback
        ASSERT_TRUE( used.IsNULLOrEmpty() );
        ASSERT_FALSE( registry.OpenDevice( "nosuchdriver", AUDIO::CAudioDeviceParams(), &callback, device, used ) );

        AUDIO::CAudioDeviceInfo defaultDevice;
        ASSERT_TRUE( good->GetDefaultDevice( AUDIO::AUDIODIRECTION_PLAYBACK, defaultDevice ) && defaultDevice.GetId() == "b" );     // the one marked default, not the first

        registry.Unregister( "refuser" );
        registry.Unregister( "goodone" );
        registry.Unregister( "betterone" );
        registry.GetDriversByPriority( ordered );
        ASSERT_TRUE( 1 == ordered.size() && ordered[ 0 ]->GetName() == "null" );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
PerformPushDeviceTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "AudioPushDevice: Renders and pushes period by period on a thread of its pool until stopped" )
    try
    {
        CRampCallback callback;
        CScriptedPushDevice device( &callback, true, 0, 0 );
        ASSERT_TRUE( device.Start() );
        ASSERT_TRUE( device.IsStarted() );
        ASSERT_TRUE( WaitForPushes( device, 5, 5000 ) );

        // the thread belongs to the pool the device was made for, so the task manager knows it
        CORE::ThreadPoolPtr pool = CORE::CCoreGlobal::Instance()->GetTaskManager().GetThreadPool( AUDIO::CAudioPushDevice::DefaultThreadPoolName );
        ASSERT_FALSE( pool.IsNULL() );
        ASSERT_TRUE( !pool.IsNULL() && pool->GetActiveNrOfDedicatedThreads() >= 1 );
        ASSERT_TRUE( device.pushThreadId.Load() != MT::GetCurrentTaskID() );

        ASSERT_TRUE( device.Stop() );
        ASSERT_FALSE( device.IsStarted() );
        ASSERT_TRUE( 1 == device.starts && 1 == device.stops );

        // nothing happens after Stop() returned, and every period that was rendered was pushed
        const UInt32 pushes = device.pushes.Load();
        MT::PrecisionDelay( 20 );
        ASSERT_TRUE( pushes == device.pushes.Load() );
        ASSERT_TRUE( pushes == callback.renderCalls.size() );
        for ( size_t i=0; i<callback.renderCalls.size(); ++i )
            ASSERT_TRUE( 64 == callback.renderCalls[ i ] );
        ASSERT_TRUE( device.GetTotalFramesPushed() == (UInt64) pushes * 64 );
        ASSERT_TRUE( 0 == device.GetUnderrunCount() && 0 == callback.lost );

        // and it starts again
        ASSERT_TRUE( device.Start() );
        ASSERT_TRUE( WaitForPushes( device, pushes + 3, 5000 ) );
        ASSERT_TRUE( device.Stop() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioPushDevice: A backend that cannot be readied makes Start fail without a thread, capture is refused" )
    try
    {
        CRampCallback callback;
        CScriptedPushDevice device( &callback, false, 0, 0 );
        ASSERT_FALSE( device.Start() );
        ASSERT_FALSE( device.IsStarted() );
        MT::PrecisionDelay( 20 );
        ASSERT_TRUE( 0 == device.pushes.Load() && callback.renderCalls.empty() );

        CScriptedPushDevice capture( &callback, true, 0, 0, AUDIO::AUDIODIRECTION_CAPTURE );
        ASSERT_FALSE( capture.Start() );
        ASSERT_TRUE( 0 == capture.starts );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioPushDevice: Underruns are counted and a lost backend is reported once and ends the device thread" )
    try
    {
        CRampCallback callback;
        CScriptedPushDevice device( &callback, true, 2, 4 );
        ASSERT_TRUE( device.Start() );

        UInt32 waited = 0;
        while ( device.IsStarted() && waited < 5000 )
        {
            MT::PrecisionDelay( 2 );
            waited += 2;
        }
        ASSERT_FALSE( device.IsStarted() );
        ASSERT_TRUE( 1 == callback.lost );
        ASSERT_TRUE( 4 == device.pushes.Load() );
        ASSERT_TRUE( 1 == device.GetUnderrunCount() );
        ASSERT_TRUE( device.GetTotalFramesPushed() == 3 * 64 );

        // the backend is still stopped properly
        ASSERT_TRUE( device.Stop() );
        ASSERT_TRUE( 1 == device.stops );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioPushDevice: Per thread set up of the backend runs on the device thread, around all pushes, once per start" )
    try
    {
        CRampCallback callback;
        CScriptedPushDevice device( &callback, true, 0, 0 );
        ASSERT_TRUE( device.Start() );
        ASSERT_TRUE( WaitForPushes( device, 3, 5000 ) );
        ASSERT_TRUE( 1 == device.threadStarts.Load() && 0 == device.threadEnds.Load() );
        ASSERT_TRUE( device.threadStartThreadId.Load() == device.pushThreadId.Load() );
        ASSERT_TRUE( device.threadStartThreadId.Load() != MT::GetCurrentTaskID() );
        ASSERT_TRUE( device.Stop() );
        ASSERT_TRUE( 1 == device.threadEnds.Load() );
        ASSERT_TRUE( device.threadEndThreadId.Load() == device.pushThreadId.Load() );

        ASSERT_TRUE( device.Start() );
        ASSERT_TRUE( WaitForPushes( device, device.pushes.Load() + 2, 5000 ) );
        ASSERT_TRUE( device.Stop() );
        ASSERT_TRUE( 2 == device.threadStarts.Load() && 2 == device.threadEnds.Load() );

        // a thread that cannot be set up is a lost device: no audio is pushed and the backend still stops
        CScriptedPushDevice refusing( &callback, true, 0, 0 );
        refusing.threadStartFails = true;
        const UInt32 lostBefore = callback.lost;
        ASSERT_TRUE( refusing.Start() );
        UInt32 waited = 0;
        while ( refusing.IsStarted() && waited < 5000 )
        {
            MT::PrecisionDelay( 2 );
            waited += 2;
        }
        ASSERT_FALSE( refusing.IsStarted() );
        ASSERT_TRUE( lostBefore + 1 == callback.lost );
        ASSERT_TRUE( 0 == refusing.pushes.Load() );
        ASSERT_TRUE( 0 == refusing.threadEnds.Load() );
        ASSERT_TRUE( refusing.Stop() );
        ASSERT_TRUE( 1 == refusing.stops );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
PerformAudioDeviceTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING AudioDevice TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "gucefAUDIO.Devices" );

    PerformInfoTests();
    PerformNullDriverTests();
    PerformRegistryTests();
    PerformPushDeviceTests();
}

/*-------------------------------------------------------------------------*/
