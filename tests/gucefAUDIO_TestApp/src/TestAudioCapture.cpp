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

#ifndef GUCEF_MT_CREALTIMESCHEDULINGSCOPE_H
#include "gucefMT_CRealtimeSchedulingScope.h"
#define GUCEF_MT_CREALTIMESCHEDULINGSCOPE_H
#endif /* GUCEF_MT_CREALTIMESCHEDULINGSCOPE_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CPULSEGENERATOR_H
#include "gucefCORE_CPulseGenerator.h"
#define GUCEF_CORE_CPULSEGENERATOR_H
#endif /* GUCEF_CORE_CPULSEGENERATOR_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_COBSERVER_H
#include "CObserver.h"
#define GUCEF_CORE_COBSERVER_H
#endif /* GUCEF_CORE_COBSERVER_H ? */

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_AUDIO_CAUDIOEFFECTREGISTRY_H
#include "gucefAUDIO_CAudioEffectRegistry.h"
#define GUCEF_AUDIO_CAUDIOEFFECTREGISTRY_H
#endif /* GUCEF_AUDIO_CAUDIOEFFECTREGISTRY_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

/* the system headers follow the platform, which the headers above define */
#if ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )
  #include <errno.h>
  #include <pthread.h>
  #include <sched.h>
  #define GUCEF_AUDIO_TEST_POSIX_SCHEDULING
#endif

#include "TestAudioTestEffect.h"
#include "TestAudioCapture.h"

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

const UInt32 PushPeriod = 64;
const UInt32 RampLength = 4096;
const UInt32 MaxBlocks = 4096;

/** The value of frame 'n' of the endless ramp the capture backend records: exact in float */
inline Float32
RampValue( UInt64 n )
{
    return (Float32) ( n % RampLength ) / (Float32) RampLength;
}

/** The scheduling policy of the calling thread, -1 where it cannot be read */
Int32
ReadSchedulingPolicy( void )
{
    #ifdef GUCEF_AUDIO_TEST_POSIX_SCHEDULING
    int policy = 0;
    struct sched_param param;
    param.sched_priority = 0;
    if ( 0 == ::pthread_getschedparam( ::pthread_self(), &policy, &param ) )
        return (Int32) policy;
    #endif
    return -1;
}

/**
 *  A push device whose backend records an endless ramp (stereo float): it can report an overrun or the loss of the
 *  device at a given read, skipping 'overrunGap' frames of the ramp for the overrun as a real device loses them.
 *  Each read takes a millisecond. It notes the thread and scheduling it ran with
 */
class CRampCapturePushDevice : public AUDIO::CAudioPushDevice
{
    public:

    CRampCapturePushDevice( AUDIO::IAudioDeviceCallback* callback, AUDIO::TAudioDirection direction, UInt32 overrunAtPull, UInt32 lostAtPull, UInt32 overrunGap )
        : AUDIO::CAudioPushDevice( AUDIO::CAudioDeviceInfo( "rampcapture", "Ramp capture push device", direction ), callback, AUDIO::CNullAudioDriver::GetDefaultFormat(), PushPeriod )
        , overrunAtPull( overrunAtPull ), lostAtPull( lostAtPull ), overrunGap( overrunGap ), nextFrame( 0 )
        , pulls( 0 ), pushes( 0 ), starts( 0 ), stops( 0 ), pullThreadId( 0 )
        , policyAtThreadStart( -2 ), policyInPull( -2 ), policyAtThreadEnd( -2 ), realtimeActiveInPull( false )
    {}

    virtual ~CRampCapturePushDevice() GUCEF_VIRTUAL_OVERRIDE
    {
        Stop();
    }

    UInt32 overrunAtPull;
    UInt32 lostAtPull;
    UInt32 overrunGap;
    UInt64 nextFrame;                       /**< device thread only */
    MT::CTAtomic< UInt32 > pulls;
    MT::CTAtomic< UInt32 > pushes;
    UInt32 starts;
    UInt32 stops;
    MT::CTAtomic< TThreadId > pullThreadId;
    MT::CTAtomic< Int32 > policyAtThreadStart;
    MT::CTAtomic< Int32 > policyInPull;
    MT::CTAtomic< Int32 > policyAtThreadEnd;
    MT::CTAtomic< bool > realtimeActiveInPull;

    protected:

    virtual bool SupportsCapture( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        return true;
    }

    virtual bool OnDeviceThreadStart( void ) GUCEF_VIRTUAL_OVERRIDE
    {
        policyAtThreadStart.Store( ReadSchedulingPolicy() );
        return true;
    }

    virtual void OnDeviceThreadEnd( void ) GUCEF_VIRTUAL_OVERRIDE
    {
        policyAtThreadEnd.Store( ReadSchedulingPolicy() );
    }

    virtual bool OnPushStart( void ) GUCEF_VIRTUAL_OVERRIDE
    {
        ++starts;
        return true;
    }

    virtual TPushResult PushFrames( const void*, UInt32 ) GUCEF_VIRTUAL_OVERRIDE
    {
        pushes.FetchAdd( 1 );
        MT::PrecisionDelay( 1 );
        return PUSHRESULT_OK;
    }

    virtual TPushResult PullFrames( void* frames, UInt32 frameCount ) GUCEF_VIRTUAL_OVERRIDE
    {
        pullThreadId.Store( MT::GetCurrentThreadID() );
        policyInPull.Store( ReadSchedulingPolicy() );
        realtimeActiveInPull.Store( IsRealtimeSchedulingActive() );

        const UInt32 pull = pulls.FetchAdd( 1 ) + 1;
        MT::PrecisionDelay( 1 );
        if ( pull == lostAtPull )
            return PUSHRESULT_DEVICE_LOST;

        const bool overrun = ( pull == overrunAtPull );
        if ( overrun )
            nextFrame += overrunGap;

        Float32* out = static_cast< Float32* >( frames );
        for ( UInt32 f=0; f<frameCount; ++f )
        {
            const Float32 value = RampValue( nextFrame++ );
            out[ 2*f ] = value;
            out[ 2*f + 1 ] = -value;
        }
        return overrun ? PUSHRESULT_UNDERRUN_RECOVERED : PUSHRESULT_OK;
    }

    virtual void OnPushStop( void ) GUCEF_VIRTUAL_OVERRIDE
    {
        ++stops;
    }
};

/**
 *  Checks what a capture push device delivers: the ramp in order (allowing for the gap after an overrun), the size of
 *  each block and the overrun count the device reports while the callback has the block. Device thread only, read
 *  by the test after the loop ended
 */
class CRampCheckCallback : public AUDIO::IAudioDeviceCallback
{
    public:

    CRampCheckCallback( void )
        : device( GUCEF_NULL ), renderCalls( 0 ), blocks( 0 ), wrongSizes( 0 ), discontinuities( 0 ), firstFrameOfBlock(), overrunsAtBlock(), lost( 0 ), threadId( 0 )
    {
        firstFrameOfBlock.assign( MaxBlocks, 0 );
        overrunsAtBlock.assign( MaxBlocks, 0 );
    }

    virtual void OnRenderAudio( void*, UInt32 ) GUCEF_VIRTUAL_OVERRIDE
    {
        ++renderCalls;
    }

    virtual void OnCaptureAudio( const void* buffer, UInt32 frameCount ) GUCEF_VIRTUAL_OVERRIDE
    {
        threadId = MT::GetCurrentThreadID();
        if ( PushPeriod != frameCount )
            ++wrongSizes;

        // the frame index of the block start, from its first value; the rest of the block must follow it
        const Float32* in = static_cast< const Float32* >( buffer );
        const UInt32 first = (UInt32) ( in[ 0 ] * (Float32) RampLength + 0.5f );
        for ( UInt32 f=0; f<frameCount; ++f )
        {
            const Float32 expected = RampValue( first + f );
            if ( in[ 2*f ] != expected || in[ 2*f + 1 ] != -expected )
                ++discontinuities;
        }

        if ( blocks < MaxBlocks )
        {
            firstFrameOfBlock[ blocks ] = first;
            overrunsAtBlock[ blocks ] = ( GUCEF_NULL != device ) ? device->GetOverrunCount() : 0;
        }
        ++blocks;
    }

    virtual void OnDeviceLost( void ) GUCEF_VIRTUAL_OVERRIDE
    {
        ++lost;
    }

    AUDIO::CAudioPushDevice* device;
    UInt32 renderCalls;
    UInt32 blocks;
    UInt32 wrongSizes;
    UInt32 discontinuities;
    GUCEF::vector< UInt32 > firstFrameOfBlock;
    GUCEF::vector< UInt64 > overrunsAtBlock;
    UInt32 lost;
    TThreadId threadId;
};

bool
WaitForPulls( const CRampCapturePushDevice& device, UInt32 count, UInt32 timeoutInMs )
{
    UInt32 waited = 0;
    while ( device.pulls.Load() < count && waited < timeoutInMs )
    {
        MT::PrecisionDelay( 2 );
        waited += 2;
    }
    return device.pulls.Load() >= count;
}

bool
WaitUntilStopped( const AUDIO::CAudioPushDevice& device, UInt32 timeoutInMs )
{
    UInt32 waited = 0;
    while ( device.IsStarted() && waited < timeoutInMs )
    {
        MT::PrecisionDelay( 2 );
        waited += 2;
    }
    return !device.IsStarted();
}

/*-------------------------------------------------------------------------*/

/** A source that plays the frames of a buffer once; real time safe: a copy per read */
class CBufferSource : public AUDIO::IAudioSource
{
    public:

    explicit CBufferSource( const AUDIO::CAudioBuffer& buffer ) : m_buffer( buffer ), m_position( 0 ), m_available( buffer.GetFrameCount() ) {}

    virtual const AUDIO::CAudioFormat& GetFormat( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_buffer.GetFormat(); }

    virtual UInt32 ReadFrames( void* destination, UInt32 frameCapacity ) GUCEF_VIRTUAL_OVERRIDE
    {
        const UInt32 limit = ( m_available < m_buffer.GetFrameCount() ) ? m_available : m_buffer.GetFrameCount();
        const UInt32 left = ( m_position < limit ) ? limit - (UInt32) m_position : 0;
        const UInt32 count = ( left < frameCapacity ) ? left : frameCapacity;
        if ( count > 0 )
            memcpy( destination, m_buffer.GetDataPtr( (UInt32) m_position ), (size_t) count * m_buffer.GetFormat().GetBytesPerFrame() );
        m_position += count;
        return count;
    }

    virtual bool IsEndOfStream( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_position >= m_buffer.GetFrameCount(); }

    virtual bool IsRealtimeSafe( void ) const GUCEF_VIRTUAL_OVERRIDE { return true; }

    virtual UInt64 GetPositionInFrames( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_position; }

    virtual UInt64 GetLengthInFrames( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_buffer.GetFrameCount(); }

    /** A live source: only the first 'frames' frames are there yet, the rest comes when this is raised */
    void SetAvailableFrames( UInt32 frames ) { m_available = frames; }

    private:

    AUDIO::CAudioBuffer m_buffer;
    UInt64 m_position;
    UInt32 m_available;
};

/** Keeps what a capture device recorded, as floats; nothing is allocated while recording */
class CCaptureRecorder : public AUDIO::IAudioDeviceCallback
{
    public:

    CCaptureRecorder( UInt32 channels, UInt32 capacityInFrames, AUDIO::TSampleType sampleType = AUDIO::SAMPLETYPE_FLOAT32 )
        : samples(), channels( channels ), sampleType( sampleType ), frames( 0 ), calls( 0 ), lost( 0 )
    {
        samples.assign( (size_t) capacityInFrames * channels, 12345.0f );
    }

    virtual void OnRenderAudio( void*, UInt32 ) GUCEF_VIRTUAL_OVERRIDE {}

    virtual void OnCaptureAudio( const void* buffer, UInt32 frameCount ) GUCEF_VIRTUAL_OVERRIDE GUCEF_AUDIO_REALTIME_CONTEXT
    {
        const size_t capacity = samples.size() / channels;
        const size_t count = ( frames + frameCount <= capacity ) ? frameCount : capacity - frames;
        AUDIO::CAudioConverter::ConvertToFloat32( buffer, sampleType, &samples[ frames * channels ], (UInt64) count * channels );
        frames += count;
        ++calls;
    }

    virtual void OnDeviceLost( void ) GUCEF_VIRTUAL_OVERRIDE { ++lost; }

    Float32 At( UInt32 frame, UInt32 channel ) const { return samples[ (size_t) frame * channels + channel ]; }

    GUCEF::vector< Float32 > samples;
    UInt32 channels;
    AUDIO::TSampleType sampleType;
    size_t frames;
    UInt32 calls;
    UInt32 lost;
};

AUDIO::TAudioDevicePtr
OpenNullDevice( AUDIO::TAudioDirection direction, const AUDIO::CAudioFormat& format, UInt32 period, AUDIO::IAudioDeviceCallback* callback )
{
    AUDIO::CNullAudioDriver driver;
    AUDIO::CAudioDeviceParams params;
    params.SetDirection( direction );
    params.SetPreferredFormat( format );
    params.SetBufferSizeInFrames( period );
    AUDIO::TAudioDevicePtr device;
    driver.OpenDevice( params, callback, device );
    return device;
}

/** 'frames' frames whose sample f, channel c is generated by 'value' and stored in the given format */
AUDIO::CAudioBuffer
MakeBuffer( const AUDIO::CAudioFormat& format, UInt32 frames, Float32 ( *value )( UInt32 frame, UInt32 channel ) )
{
    AUDIO::CAudioBuffer buffer( format, frames );
    for ( UInt32 f=0; f<frames; ++f )
        for ( UInt32 c=0; c<format.GetChannelCount(); ++c )
            buffer.SetSampleFromFloat( f, c, value( f, c ) );
    return buffer;
}

Float32 StereoPattern( UInt32 frame, UInt32 channel ) { return ( channel == 0 ? 1.0f : -1.0f ) * (Float32) ( ( frame % 500 ) + 1 ) / 1024.0f; }

Float32 Int16Pattern( UInt32 frame, UInt32 ) { return (Float32) ( (Int32) ( frame % 2000 ) - 1000 ) / 32768.0f; }

Float32 SlowRamp( UInt32 frame, UInt32 ) { return (Float32) frame / 8192.0f; }

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

static void
PerformPushDeviceCaptureTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "AudioCapture: A capture push device reads period by period on its device thread and hands every block to the callback in order" )
    try
    {
        CRampCheckCallback callback;
        CRampCapturePushDevice device( &callback, AUDIO::AUDIODIRECTION_CAPTURE, 0, 0, 0 );
        ASSERT_TRUE( device.Start() );
        ASSERT_TRUE( device.IsStarted() );
        ASSERT_TRUE( WaitForPulls( device, 8, 5000 ) );
        ASSERT_TRUE( device.Stop() );
        ASSERT_FALSE( device.IsStarted() );
        ASSERT_TRUE( 1 == device.starts && 1 == device.stops );

        // every block that was read was delivered, in order, from the device thread; nothing was rendered or pushed
        const UInt32 pulls = device.pulls.Load();
        MT::PrecisionDelay( 20 );
        ASSERT_TRUE( pulls == device.pulls.Load() );
        ASSERT_TRUE( pulls == callback.blocks );
        ASSERT_TRUE( 0 == callback.wrongSizes && 0 == callback.discontinuities );
        for ( UInt32 b=0; b<callback.blocks && b<MaxBlocks; ++b )
            ASSERT_TRUE( ( b * PushPeriod ) % RampLength == callback.firstFrameOfBlock[ b ] );
        ASSERT_TRUE( callback.threadId == device.pullThreadId.Load() );
        ASSERT_TRUE( callback.threadId != MT::GetCurrentThreadID() );
        ASSERT_TRUE( 0 == callback.renderCalls && 0 == device.pushes.Load() );
        ASSERT_TRUE( device.GetTotalFramesPushed() == (UInt64) pulls * PushPeriod );
        ASSERT_TRUE( 0 == device.GetOverrunCount() && 0 == device.GetUnderrunCount() && 0 == callback.lost );

        // and it starts again, the backend going on where it was
        ASSERT_TRUE( device.Start() );
        ASSERT_TRUE( WaitForPulls( device, pulls + 3, 5000 ) );
        ASSERT_TRUE( device.Stop() );
        ASSERT_TRUE( device.pulls.Load() == callback.blocks );
        ASSERT_TRUE( 0 == callback.discontinuities );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioCapture: The device branches on its direction: a capture capable backend still plays as a playback device" )
    try
    {
        CRampCheckCallback callback;
        CRampCapturePushDevice device( &callback, AUDIO::AUDIODIRECTION_PLAYBACK, 0, 0, 0 );
        ASSERT_TRUE( device.Start() );
        UInt32 waited = 0;
        while ( device.pushes.Load() < 5 && waited < 5000 )
        {
            MT::PrecisionDelay( 2 );
            waited += 2;
        }
        ASSERT_TRUE( device.Stop() );
        ASSERT_TRUE( device.pushes.Load() >= 5 );
        ASSERT_TRUE( device.pushes.Load() == callback.renderCalls );
        ASSERT_TRUE( 0 == device.pulls.Load() && 0 == callback.blocks );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioCapture: An overrun is counted before the block after the gap reaches the callback, a lost backend is reported once" )
    try
    {
        CRampCheckCallback callback;
        CRampCapturePushDevice device( &callback, AUDIO::AUDIODIRECTION_CAPTURE, 3, 6, 100 );
        callback.device = &device;
        ASSERT_TRUE( device.Start() );
        ASSERT_TRUE( WaitUntilStopped( device, 5000 ) );

        // reads 1 to 5 were delivered, read 6 lost the device
        ASSERT_TRUE( 1 == callback.lost );
        ASSERT_TRUE( 6 == device.pulls.Load() );
        ASSERT_TRUE( 5 == callback.blocks );
        ASSERT_TRUE( 1 == device.GetOverrunCount() );
        ASSERT_TRUE( 0 == device.GetUnderrunCount() );
        ASSERT_TRUE( device.GetTotalFramesPushed() == 5 * PushPeriod );

        // the third block is the first after the gap: it is the one that sees the count go up, and it starts after the lost frames
        ASSERT_TRUE( 0 == callback.overrunsAtBlock[ 0 ] && 0 == callback.overrunsAtBlock[ 1 ] );
        ASSERT_TRUE( 1 == callback.overrunsAtBlock[ 2 ] && 1 == callback.overrunsAtBlock[ 3 ] && 1 == callback.overrunsAtBlock[ 4 ] );
        ASSERT_TRUE( 2 * PushPeriod + 100 == callback.firstFrameOfBlock[ 2 ] );
        ASSERT_TRUE( 0 == callback.discontinuities && 0 == callback.wrongSizes );

        // the backend is still stopped properly
        ASSERT_TRUE( device.Stop() );
        ASSERT_TRUE( 1 == device.stops );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioCapture: The device option realtimePriority is read and kept, nonsense keeps normal scheduling" )
    try
    {
        CRampCheckCallback callback;
        CRampCapturePushDevice device( &callback, AUDIO::AUDIODIRECTION_CAPTURE, 0, 0, 0 );
        ASSERT_TRUE( 0 == device.GetRealtimePriority() );
        ASSERT_TRUE( AUDIO::CAudioPushDevice::RealtimePriorityOptionName == "realtimePriority" );

        AUDIO::CAudioDeviceParams params;
        device.ApplyDeviceOptions( params );
        ASSERT_TRUE( 0 == device.GetRealtimePriority() );

        params.GetOptions().Set( AUDIO::CAudioPushDevice::RealtimePriorityOptionName, "70" );
        device.ApplyDeviceOptions( params );
        ASSERT_TRUE( 70 == device.GetRealtimePriority() );

        // an option that is not there keeps the value, a negative one asks for normal scheduling
        AUDIO::CAudioDeviceParams other;
        other.GetOptions().Set( "periods", "3" );
        device.ApplyDeviceOptions( other );
        ASSERT_TRUE( 70 == device.GetRealtimePriority() );
        other.GetOptions().Set( AUDIO::CAudioPushDevice::RealtimePriorityOptionName, "-5" );
        device.ApplyDeviceOptions( other );
        ASSERT_TRUE( 0 == device.GetRealtimePriority() );

        device.SetRealtimePriority( 20 );
        ASSERT_TRUE( 20 == device.GetRealtimePriority() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioCapture: With realtimePriority the device thread runs in a real-time scope for the loop only, a refusal is harmless and reported" )
    try
    {
        const AUDIO::TAudioDirection directions[ 2 ] = { AUDIO::AUDIODIRECTION_CAPTURE, AUDIO::AUDIODIRECTION_PLAYBACK };
        for ( UInt32 d=0; d<2; ++d )
        {
            CRampCheckCallback callback;
            CRampCapturePushDevice device( &callback, directions[ d ], 0, 0, 0 );
            AUDIO::CAudioDeviceParams params;
            params.GetOptions().Set( AUDIO::CAudioPushDevice::RealtimePriorityOptionName, "70" );
            device.ApplyDeviceOptions( params );

            ASSERT_TRUE( device.Start() );
            if ( AUDIO::AUDIODIRECTION_CAPTURE == directions[ d ] )
            {
                ASSERT_TRUE( WaitForPulls( device, 4, 5000 ) );
            }
            else
            {
                UInt32 waited = 0;
                while ( device.pushes.Load() < 4 && waited < 5000 )
                {
                    MT::PrecisionDelay( 2 );
                    waited += 2;
                }
            }
            const bool active = device.IsRealtimeSchedulingActive();
            const Int32 error = device.GetRealtimeSchedulingError();
            ASSERT_TRUE( device.Stop() );

            // either way the device ran and the thread went back to its pool as it came
            ASSERT_FALSE( device.IsRealtimeSchedulingActive() );
            ASSERT_TRUE( device.policyAtThreadStart.Load() == device.policyAtThreadEnd.Load() );
            if ( AUDIO::AUDIODIRECTION_CAPTURE == directions[ d ] )
            {
                ASSERT_TRUE( callback.blocks >= 4 && 0 == callback.discontinuities );
                ASSERT_TRUE( active == device.realtimeActiveInPull.Load() );
                #ifdef GUCEF_AUDIO_TEST_POSIX_SCHEDULING
                if ( active )
                    ASSERT_TRUE( SCHED_FIFO == device.policyInPull.Load() );
                else
                    ASSERT_TRUE( device.policyAtThreadStart.Load() == device.policyInPull.Load() );
                #endif
            }
            else
            {
                ASSERT_TRUE( callback.renderCalls >= 4 );
            }

            if ( active )
            {
                ASSERT_TRUE( 0 == error );
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioCapture: the device thread got real-time scheduling, the active case was tested" );
            }
            else
            if ( MT::CRealtimeSchedulingScope::IsSupported() )
            {
                ASSERT_TRUE( 0 != error );
                #ifdef GUCEF_AUDIO_TEST_POSIX_SCHEDULING
                ASSERT_TRUE( EPERM == error );
                #endif
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioCapture: real-time scheduling of the device thread was refused (error " << error << "), the device ran at normal priority" );
            }
        }

        // without the option the device thread keeps normal scheduling and nothing is reported
        CRampCheckCallback callback;
        CRampCapturePushDevice device( &callback, AUDIO::AUDIODIRECTION_CAPTURE, 0, 0, 0 );
        ASSERT_TRUE( device.Start() );
        ASSERT_TRUE( WaitForPulls( device, 3, 5000 ) );
        ASSERT_FALSE( device.IsRealtimeSchedulingActive() );
        ASSERT_TRUE( 0 == device.GetRealtimeSchedulingError() );
        ASSERT_TRUE( device.Stop() );
        ASSERT_FALSE( device.realtimeActiveInPull.Load() );
        ASSERT_TRUE( device.policyAtThreadStart.Load() == device.policyInPull.Load() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
PerformNullDeviceCaptureTests( void )
{GUCEF_TRACE;

    const AUDIO::CAudioFormat stereoFloat( 48000, 2, AUDIO::SAMPLETYPE_FLOAT32 );

    GUCEF_TESTFW_TESTCASE( "AudioCapture: The null capture device records its source bit for bit when the formats match, then silence" )
    try
    {
        CCaptureRecorder recorder( 2, 4096 );
        AUDIO::TAudioDevicePtr device = OpenNullDevice( AUDIO::AUDIODIRECTION_CAPTURE, stereoFloat, 256, &recorder );
        ASSERT_FALSE( device.IsNULL() );
        AUDIO::CNullAudioDevice* nullDevice = device->AsNullDevice();

        const AUDIO::CAudioBuffer pattern = MakeBuffer( stereoFloat, 1000, &StereoPattern );
        AUDIO::TAudioSourcePtr source( GUCEF_NEW CBufferSource( pattern ) );
        ASSERT_TRUE( nullDevice->SetCaptureSource( source ) );
        ASSERT_TRUE( nullDevice->GetCaptureSource() == source );
        ASSERT_TRUE( device->Start() );

        // pieces of the period and a last piece that is shorter
        ASSERT_TRUE( 1536 == nullDevice->Render( 1536 ) );
        ASSERT_TRUE( 100 == nullDevice->Render( 100 ) );
        ASSERT_TRUE( 1636 == recorder.frames );
        ASSERT_TRUE( 7 == recorder.calls );
        bool exact = true;
        for ( UInt32 f=0; f<1000; ++f )
            for ( UInt32 c=0; c<2; ++c )
                exact = exact && ( recorder.At( f, c ) == pattern.GetSampleAsFloat( f, c ) );
        ASSERT_TRUE( exact );
        bool silent = true;
        for ( UInt32 f=1000; f<1636; ++f )
            silent = silent && ( 0.0f == recorder.At( f, 0 ) && 0.0f == recorder.At( f, 1 ) );
        ASSERT_TRUE( silent );
        ASSERT_TRUE( 0 == recorder.lost );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioCapture: The null capture device converts sample type and channels of its source to its own format" )
    try
    {
        CCaptureRecorder recorder( 2, 4096 );
        AUDIO::TAudioDevicePtr device = OpenNullDevice( AUDIO::AUDIODIRECTION_CAPTURE, stereoFloat, 240, &recorder );
        AUDIO::CNullAudioDevice* nullDevice = device->AsNullDevice();

        // 16 bit mono: each sample lands in both channels, scaled by the remix matrix, the integers exact in float
        const AUDIO::CAudioFormat monoInt16( 48000, 1, AUDIO::SAMPLETYPE_INT16 );
        const AUDIO::CAudioBuffer pattern = MakeBuffer( monoInt16, 2500, &Int16Pattern );
        ASSERT_TRUE( nullDevice->SetCaptureSource( AUDIO::TAudioSourcePtr( GUCEF_NEW CBufferSource( pattern ) ) ) );
        GUCEF::vector< Float32 > matrix;
        ASSERT_TRUE( AUDIO::CAudioConverter::BuildChannelMatrix( monoInt16, stereoFloat, matrix ) );
        ASSERT_TRUE( 2 == matrix.size() );

        ASSERT_TRUE( device->Start() );
        ASSERT_TRUE( 3000 == nullDevice->Render( 3000 ) );
        ASSERT_TRUE( 3000 == recorder.frames );
        bool converted = true;
        for ( UInt32 f=0; f<2500; ++f )
        {
            const Float32 input = Int16Pattern( f, 0 );
            converted = converted && ( recorder.At( f, 0 ) == input * matrix[ 0 ] ) && ( recorder.At( f, 1 ) == input * matrix[ 1 ] );
        }
        ASSERT_TRUE( converted );
        bool silent = true;
        for ( UInt32 f=2500; f<3000; ++f )
            silent = silent && ( 0.0f == recorder.At( f, 0 ) && 0.0f == recorder.At( f, 1 ) );
        ASSERT_TRUE( silent );

        // a 16 bit device gets a float source as the same integers
        CCaptureRecorder intRecorder( 1, 512, AUDIO::SAMPLETYPE_INT16 );
        AUDIO::TAudioDevicePtr intDevice = OpenNullDevice( AUDIO::AUDIODIRECTION_CAPTURE, monoInt16, 64, &intRecorder );
        ASSERT_TRUE( intDevice->GetFormat().GetSampleType() == AUDIO::SAMPLETYPE_INT16 );
        const AUDIO::CAudioFormat monoFloat( 48000, 1, AUDIO::SAMPLETYPE_FLOAT32 );
        ASSERT_TRUE( intDevice->AsNullDevice()->SetCaptureSource( AUDIO::TAudioSourcePtr( GUCEF_NEW CBufferSource( MakeBuffer( monoFloat, 500, &Int16Pattern ) ) ) ) );
        ASSERT_TRUE( intDevice->Start() );
        ASSERT_TRUE( 512 == intDevice->AsNullDevice()->Render( 512 ) );
        bool integers = true;
        for ( UInt32 f=0; f<500; ++f )
            integers = integers && ( intRecorder.At( f, 0 ) == Int16Pattern( f, 0 ) );
        ASSERT_TRUE( integers );
        ASSERT_TRUE( 0.0f == intRecorder.At( 500, 0 ) && 0.0f == intRecorder.At( 511, 0 ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioCapture: The null capture device resamples a source of another rate continuously across periods" )
    try
    {
        // a ramp at 24 kHz recorded at 48 kHz: linear interpolation keeps a ramp a ramp, at half the slope per frame
        const AUDIO::CAudioFormat monoFloat24k( 24000, 1, AUDIO::SAMPLETYPE_FLOAT32 );
        const AUDIO::CAudioFormat monoFloat48k( 48000, 1, AUDIO::SAMPLETYPE_FLOAT32 );
        CCaptureRecorder recorder( 1, 8192 );
        AUDIO::TAudioDevicePtr device = OpenNullDevice( AUDIO::AUDIODIRECTION_CAPTURE, monoFloat48k, 256, &recorder );
        AUDIO::CNullAudioDevice* nullDevice = device->AsNullDevice();
        ASSERT_TRUE( nullDevice->SetCaptureSource( AUDIO::TAudioSourcePtr( GUCEF_NEW CBufferSource( MakeBuffer( monoFloat24k, 3000, &SlowRamp ) ) ) ) );
        ASSERT_TRUE( device->Start() );
        ASSERT_TRUE( 7000 == nullDevice->Render( 7000 ) );

        // about twice the source frames carry the ramp, with no step at the period boundaries; silence after
        ASSERT_TRUE( 7000 == recorder.frames );
        UInt32 last = 0;
        for ( UInt32 f=0; f<7000; ++f )
            if ( 0.0f != recorder.At( f, 0 ) )
                last = f;
        ASSERT_TRUE( last >= 5990 && last <= 6002 );
        Float64 worst = 0.0;
        for ( UInt32 f=4; f+4<last; ++f )
        {
            const Float64 step = (Float64) recorder.At( f + 1, 0 ) - (Float64) recorder.At( f, 0 );
            const Float64 error = fabs( step - 0.5 / 8192.0 );
            if ( error > worst )
                worst = error;
        }
        ASSERT_TRUE( worst < 1.0e-5 );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioCapture: A live source that has nothing for a while gives silence and nothing of it is lost; an empty source records silence" )
    try
    {
        CCaptureRecorder recorder( 2, 4096 );
        AUDIO::TAudioDevicePtr device = OpenNullDevice( AUDIO::AUDIODIRECTION_CAPTURE, stereoFloat, 128, &recorder );
        AUDIO::CNullAudioDevice* nullDevice = device->AsNullDevice();
        const AUDIO::CAudioBuffer pattern = MakeBuffer( stereoFloat, 512, &StereoPattern );
        CBufferSource* live = GUCEF_NEW CBufferSource( pattern );
        live->SetAvailableFrames( 200 );
        ASSERT_TRUE( nullDevice->SetCaptureSource( AUDIO::TAudioSourcePtr( live ) ) );
        ASSERT_TRUE( device->Start() );

        // 200 frames are there, then the source has nothing: silence, then the rest arrives and continues where it was
        ASSERT_TRUE( 256 == nullDevice->Render( 256 ) );
        live->SetAvailableFrames( 512 );
        ASSERT_TRUE( 384 == nullDevice->Render( 384 ) );
        bool first = true;
        for ( UInt32 f=0; f<200; ++f )
            first = first && ( recorder.At( f, 0 ) == pattern.GetSampleAsFloat( f, 0 ) );
        ASSERT_TRUE( first );
        bool gap = true;
        for ( UInt32 f=200; f<256; ++f )
            gap = gap && ( 0.0f == recorder.At( f, 0 ) );
        ASSERT_TRUE( gap );
        bool rest = true;
        for ( UInt32 f=200; f<512; ++f )
            rest = rest && ( recorder.At( 56 + f, 0 ) == pattern.GetSampleAsFloat( f, 0 ) );
        ASSERT_TRUE( rest );

        // no source: silence again
        ASSERT_TRUE( nullDevice->SetCaptureSource( AUDIO::TAudioSourcePtr() ) );
        ASSERT_TRUE( nullDevice->GetCaptureSource().IsNULL() );
        ASSERT_TRUE( 128 == nullDevice->Render( 128 ) );
        bool silent = true;
        for ( UInt32 f=640; f<768; ++f )
            silent = silent && ( 0.0f == recorder.At( f, 0 ) && 0.0f == recorder.At( f, 1 ) );
        ASSERT_TRUE( silent );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioCapture: A playback null device takes no capture source" )
    try
    {
        CCaptureRecorder recorder( 2, 16 );
        AUDIO::TAudioDevicePtr device = OpenNullDevice( AUDIO::AUDIODIRECTION_PLAYBACK, stereoFloat, 128, &recorder );
        AUDIO::TAudioSourcePtr source( GUCEF_NEW CBufferSource( MakeBuffer( stereoFloat, 16, &StereoPattern ) ) );
        ASSERT_FALSE( device->AsNullDevice()->SetCaptureSource( source ) );
        ASSERT_TRUE( device->AsNullDevice()->GetCaptureSource().IsNULL() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      ENGINE CAPTURE PATH                                                //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

#if defined( __has_feature )
  #if __has_feature( realtime_sanitizer )
    #define GUCEF_AUDIO_TEST_CAPTURE_RTSAN_BUILD
  #endif
#endif

const Float64 TestPi = 3.14159265358979323846;
const UInt32 TestPushPeriodFrames = 240;

/**
 *  Keeps what the capture path delivers: the samples and the block infos, in memory made beforehand, so nothing is
 *  allocated on the capture thread. The counters can be read while the capture thread runs, the samples and blocks only
 *  once it let go of the consumer
 */
class CRecordingConsumer : public AUDIO::IAudioCaptureConsumer
{
    public:

    struct SBlock
    {
        UInt64 firstFrame;
        UInt64 tickCount;
        UInt32 frames;
        bool discontinuity;
        bool voiceActive;
        Float32 level;
        SBlock( void ) : firstFrame( 0 ), tickCount( 0 ), frames( 0 ), discontinuity( false ), voiceActive( false ), level( 0.0f ) {}
    };

    CRecordingConsumer( UInt32 capacityInFrames, UInt32 maxBlocks = 16384, bool acceptPrepare = true )
        : samples(), frames( 0 ), blocks(), blockCount( 0 ), prepareCount( 0 ), preparedFormat(), preparedMaxBlock( 0 )
        , acceptPrepare( acceptPrepare ), calls( 0 ), gaps( 0 ), oversized( 0 ), threadId( 0 ), m_expectedNext( 0 )
    {
        samples.assign( capacityInFrames, 0.0f );
        blocks.assign( maxBlocks, SBlock() );
    }

    virtual bool PrepareForCapture( const AUDIO::CAudioFormat& processingFormat, UInt32 maxBlockFrames ) GUCEF_VIRTUAL_OVERRIDE
    {
        ++prepareCount;
        preparedFormat = processingFormat;
        preparedMaxBlock = maxBlockFrames;
        return acceptPrepare;
    }

    virtual void OnCapturedAudio( const Float32* mono, UInt32 frameCount, const SCaptureBlockInfo& info ) GUCEF_VIRTUAL_OVERRIDE GUCEF_AUDIO_REALTIME_CONTEXT
    {
        threadId.Store( MT::GetCurrentThreadID() );
        if ( frameCount > preparedMaxBlock )
            oversized.FetchAdd( 1 );
        if ( blockCount > 0 && !info.discontinuity && info.firstFrame != m_expectedNext )
            gaps.FetchAdd( 1 );
        m_expectedNext = info.firstFrame + frameCount;

        const size_t room = samples.size() - frames;
        const size_t count = ( frameCount < room ) ? frameCount : room;
        if ( count > 0 )
            memcpy( &samples[ frames ], mono, count * sizeof( Float32 ) );
        frames += count;

        if ( blockCount < blocks.size() )
        {
            SBlock& block = blocks[ blockCount ];
            block.firstFrame = info.firstFrame;
            block.tickCount = info.tickCount;
            block.frames = frameCount;
            block.discontinuity = info.discontinuity;
            block.voiceActive = info.voiceActive;
            block.level = info.levelInDecibels;
        }
        ++blockCount;
        calls.FetchAdd( 1 );
    }

    GUCEF::vector< Float32 > samples;
    size_t frames;
    GUCEF::vector< SBlock > blocks;
    size_t blockCount;
    UInt32 prepareCount;
    AUDIO::CAudioFormat preparedFormat;
    UInt32 preparedMaxBlock;
    bool acceptPrepare;
    MT::CTAtomic< UInt32 > calls;
    MT::CTAtomic< UInt32 > gaps;          /**< blocks that did not follow the one before without being marked as a discontinuity */
    MT::CTAtomic< UInt32 > oversized;     /**< blocks longer than the size the consumer was prepared for */
    MT::CTAtomic< TThreadId > threadId;

    private:

    UInt64 m_expectedNext;
};

/** Counts one event of a notifier */
class CEventCounter : public CORE::CObserver
{
    public:

    typedef CORE::CTEventHandlerFunctor< CEventCounter > TEventCallback;

    CEventCounter( CORE::CNotifier* notifier, const CORE::CEvent& id )
        : CORE::CObserver(), count( 0 )
    {
        TEventCallback callback( this, &CEventCounter::OnEvent );
        SubscribeTo( notifier, id, callback );
    }

    virtual ~CEventCounter() { UnsubscribeAllFromObserver(); }

    void OnEvent( CORE::CNotifier*, const CORE::CEvent&, CORE::CICloneable* )
    {
        count.FetchAdd( 1 );
    }

    MT::CTAtomic< UInt32 > count;
};

/** A test effect that maps x to gain * x + offset and notes when it ran, so the order of the stages can be seen */
class COrderTestEffect : public CTestAudioEffect
{
    public:

    COrderTestEffect( Float32 gain, Float32 offset ) : CTestAudioEffect(), sequence( 0 ), m_gainFactor( gain ), m_offset( offset ) {}

    static MT::CTAtomic< UInt32 >& Counter( void )
    {
        static MT::CTAtomic< UInt32 > counter( 0 );
        return counter;
    }

    MT::CTAtomic< UInt32 > sequence;      /**< value of the shared counter when this effect last ran */

    protected:

    virtual void OnProcess( Float32* inOut, UInt32 frames ) GUCEF_VIRTUAL_OVERRIDE
    {
        for ( UInt32 f=0; f<frames; ++f )
            inOut[ f ] = inOut[ f ] * m_gainFactor + m_offset;
        sequence.Store( Counter().FetchAdd( 1 ) + 1 );
    }

    private:

    Float32 m_gainFactor;
    Float32 m_offset;
};

/**
 *  A test effect that names a bus (construction option "keyBus") and finds it in the engine it is inserted into, as an
 *  effect keyed on another bus does
 */
class CResolvingTestEffect : public CTestAudioEffect
{
    public:

    static const CORE::CString& GetStaticEffectTypeName( void )
    {
        static const CORE::CString typeName( "resolvingtesteffect" );
        return typeName;
    }

    explicit CResolvingTestEffect( const CORE::CValueList& options = CORE::CValueList() )
        : CTestAudioEffect( options ), keyBus( options.GetValueAlways( "keyBus" ).AsString() ), resolvedBus( GUCEF_NULL ), resolveCalls( 0 )
    {}

    virtual const CORE::CString& GetEffectTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        return GetStaticEffectTypeName();
    }

    virtual void GetConstructionOptions( CORE::CValueList& options ) const GUCEF_VIRTUAL_OVERRIDE
    {
        if ( !keyBus.IsNULLOrEmpty() )
            options.Set( "keyBus", keyBus );
    }

    virtual bool ResolveReferences( AUDIO::CAudioEngine& engine ) GUCEF_VIRTUAL_OVERRIDE
    {
        ++resolveCalls;
        if ( keyBus.IsNULLOrEmpty() )
            return true;
        resolvedBus = engine.FindBus( keyBus );
        return GUCEF_NULL != resolvedBus;
    }

    CORE::CString keyBus;
    AUDIO::CAudioBus* resolvedBus;
    UInt32 resolveCalls;
};

typedef AUDIO::CTAudioEffectFactory< CResolvingTestEffect > TResolvingTestEffectFactory;

/*-------------------------------------------------------------------------*/

/** What the devices of the capture test driver do; set by a test before it initializes an engine on the driver */
struct SCaptureTestDriverConfig
{
    UInt32 overrunAtPull;       /**< the capture device reports an overrun at this read (1 based), 0 for none */
    UInt32 stallInMs;           /**< how long the read of the overrun takes: the time the audio was lost */
    UInt32 lostAtPull;          /**< the capture device is lost at this read, 0 for never */
    bool refuseCapture;
    SCaptureTestDriverConfig( void ) : overrunAtPull( 0 ), stallInMs( 0 ), lostAtPull( 0 ), refuseCapture( false ) {}
};

SCaptureTestDriverConfig&
CaptureTestDriverConfig( void )
{
    static SCaptureTestDriverConfig config;
    return config;
}

/** A capture push device that records a 1 kHz sine at half scale in stereo float, a millisecond per period */
class CSineCapturePushDevice : public AUDIO::CAudioPushDevice
{
    public:

    CSineCapturePushDevice( AUDIO::IAudioDeviceCallback* callback, const SCaptureTestDriverConfig& config )
        : AUDIO::CAudioPushDevice( AUDIO::CAudioDeviceInfo( "sinecapture", "Sine capture push device", AUDIO::AUDIODIRECTION_CAPTURE ), callback, AUDIO::CNullAudioDriver::GetDefaultFormat(), TestPushPeriodFrames )
        , pulls( 0 ), m_config( config ), m_phase( 0.0 )
    {}

    virtual ~CSineCapturePushDevice() GUCEF_VIRTUAL_OVERRIDE
    {
        Stop();
    }

    MT::CTAtomic< UInt32 > pulls;

    protected:

    virtual bool SupportsCapture( void ) const GUCEF_VIRTUAL_OVERRIDE { return true; }
    virtual bool OnPushStart( void ) GUCEF_VIRTUAL_OVERRIDE { return true; }
    virtual void OnPushStop( void ) GUCEF_VIRTUAL_OVERRIDE {}
    virtual TPushResult PushFrames( const void*, UInt32 ) GUCEF_VIRTUAL_OVERRIDE { return PUSHRESULT_OK; }

    virtual TPushResult PullFrames( void* frames, UInt32 frameCount ) GUCEF_VIRTUAL_OVERRIDE
    {
        const UInt32 pull = pulls.FetchAdd( 1 ) + 1;
        MT::PrecisionDelay( 1 );
        if ( pull == m_config.lostAtPull )
            return PUSHRESULT_DEVICE_LOST;
        const bool overrun = ( pull == m_config.overrunAtPull );
        if ( overrun )
            MT::PrecisionDelay( m_config.stallInMs );

        Float32* out = static_cast< Float32* >( frames );
        for ( UInt32 f=0; f<frameCount; ++f )
        {
            const Float32 value = (Float32) ( 0.5 * sin( m_phase ) );
            out[ 2*f ] = value;
            out[ 2*f + 1 ] = value;
            m_phase += 2.0 * TestPi * 1000.0 / 48000.0;
            if ( m_phase > 2.0 * TestPi )
                m_phase -= 2.0 * TestPi;
        }
        return overrun ? PUSHRESULT_UNDERRUN_RECOVERED : PUSHRESULT_OK;
    }

    private:

    SCaptureTestDriverConfig m_config;
    Float64 m_phase;
};

/** A playback push device that takes a period every millisecond */
class CPacedPlaybackPushDevice : public AUDIO::CAudioPushDevice
{
    public:

    explicit CPacedPlaybackPushDevice( AUDIO::IAudioDeviceCallback* callback )
        : AUDIO::CAudioPushDevice( AUDIO::CAudioDeviceInfo( "pacedplayback", "Paced playback push device", AUDIO::AUDIODIRECTION_PLAYBACK ), callback, AUDIO::CNullAudioDriver::GetDefaultFormat(), TestPushPeriodFrames )
        , pushes( 0 )
    {}

    virtual ~CPacedPlaybackPushDevice() GUCEF_VIRTUAL_OVERRIDE
    {
        Stop();
    }

    MT::CTAtomic< UInt32 > pushes;

    protected:

    virtual bool OnPushStart( void ) GUCEF_VIRTUAL_OVERRIDE { return true; }
    virtual void OnPushStop( void ) GUCEF_VIRTUAL_OVERRIDE {}

    virtual TPushResult PushFrames( const void*, UInt32 ) GUCEF_VIRTUAL_OVERRIDE
    {
        pushes.FetchAdd( 1 );
        MT::PrecisionDelay( 1 );
        return PUSHRESULT_OK;
    }
};

/** A driver whose playback and capture devices run on device threads of their own */
class CCaptureTestDriver : public AUDIO::CAudioDriver
{
    public:

    CCaptureTestDriver( void ) : AUDIO::CAudioDriver() {}

    virtual CORE::CString GetName( void ) const GUCEF_VIRTUAL_OVERRIDE { return "capturetest"; }
    virtual Int32 GetPriority( void ) const GUCEF_VIRTUAL_OVERRIDE { return -2000; }

    virtual bool EnumerateDevices( AUDIO::TAudioDirection, AUDIO::TAudioDeviceInfoVector& ) GUCEF_VIRTUAL_OVERRIDE
    {
        return true;
    }

    virtual bool OpenDevice( const AUDIO::CAudioDeviceParams& params, AUDIO::IAudioDeviceCallback* callback, AUDIO::TAudioDevicePtr& device ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( AUDIO::AUDIODIRECTION_CAPTURE == params.GetDirection() )
        {
            if ( CaptureTestDriverConfig().refuseCapture )
                return false;
            device = AUDIO::TAudioDevicePtr( GUCEF_NEW CSineCapturePushDevice( callback, CaptureTestDriverConfig() ) );
        }
        else
            device = AUDIO::TAudioDevicePtr( GUCEF_NEW CPacedPlaybackPushDevice( callback ) );
        return true;
    }
};

/*-------------------------------------------------------------------------*/

/** Engine parameters for the null driver with capture: a stereo float capture device at 'deviceRate' unless told otherwise */
AUDIO::CAudioEngineParams
CaptureParams( UInt32 deviceRate = 48000, UInt32 deviceChannels = 2, UInt32 processingRate = 0, AUDIO::TSampleType deviceType = AUDIO::SAMPLETYPE_FLOAT32 )
{
    AUDIO::CAudioEngineParams params;
    params.SetDriverName( "null" );
    params.SetThreadingMode( AUDIO::AUDIOTHREADING_COOPERATIVE );
    params.SetLimiterMode( AUDIO::AUDIOLIMITERMODE_INSTANT );
    params.GetDeviceParams().SetBufferSizeInFrames( 256 );
    params.GetDeviceParams().GetOptions().Set( "keepOutput", "true" );
    params.SetCaptureEnabled( true );
    params.SetCaptureProcessingRate( processingRate );
    params.GetCaptureDeviceParams().SetPreferredFormat( AUDIO::CAudioFormat( deviceRate, deviceChannels, deviceType ) );
    params.GetCaptureDeviceParams().SetBufferSizeInFrames( 256 );
    return params;
}

/** Parameters for the capture test driver, whose devices run on their own threads */
AUDIO::CAudioEngineParams
PushCaptureParams( AUDIO::TAudioThreadingMode mode )
{
    AUDIO::CAudioEngineParams params;
    params.SetDriverName( "capturetest" );
    params.SetThreadingMode( mode );
    params.SetLimiterMode( AUDIO::AUDIOLIMITERMODE_INSTANT );
    params.SetCaptureEnabled( true );
    return params;
}

AUDIO::CNullAudioDevice*
PlaybackNull( AUDIO::CAudioEngine& engine )
{
    AUDIO::TAudioDevicePtr device = engine.GetDevice();
    return device.IsNULL() ? GUCEF_NULL : device->AsNullDevice();
}

AUDIO::CNullAudioDevice*
CaptureNull( AUDIO::CAudioEngine& engine )
{
    AUDIO::TAudioDevicePtr device = engine.GetCaptureDevice();
    return device.IsNULL() ? GUCEF_NULL : device->AsNullDevice();
}

/** A source of 'frames' frames in 'format': a sine of 'frequency' at amplitude 'left' on channel 0 and 'right' on the others, plus 'offset' */
AUDIO::TAudioSourcePtr
MakeToneSource( const AUDIO::CAudioFormat& format, UInt32 frames, Float64 frequency, Float32 left, Float32 right, Float32 offset = 0.0f )
{
    AUDIO::CAudioBuffer buffer( format, frames );
    const Float64 rate = (Float64) format.GetSampleRate();
    for ( UInt32 f=0; f<frames; ++f )
    {
        const Float32 s = (Float32) sin( 2.0 * TestPi * frequency * (Float64) f / rate );
        for ( UInt32 c=0; c<format.GetChannelCount(); ++c )
            buffer.SetSampleFromFloat( f, c, offset + ( 0 == c ? left : right ) * s );
    }
    return AUDIO::TAudioSourcePtr( GUCEF_NEW CBufferSource( buffer ) );
}

/** Renders the capture device in periods and lets the engine handle what came back */
void
RenderCapture( AUDIO::CAudioEngine& engine, UInt64 frames )
{
    CaptureNull( engine )->Render( frames );
    engine.Update();
}

/** Lets the engine handle what came back and lets its events be delivered: they go to the pulse generator of this thread */
void
UpdateAndDeliver( AUDIO::CAudioEngine& engine )
{
    engine.Update();
    CORE::CCoreGlobal::Instance()->GetPulseGenerator()->ForceDirectPulse();
}

/** Frequency from the positive going zero crossings of 'count' samples from 'start', interpolated between samples */
Float64
MeasureFrequency( const GUCEF::vector< Float32 >& x, size_t start, size_t count, Float64 rate )
{
    Float64 first = -1.0;
    Float64 last = -1.0;
    UInt32 crossings = 0;
    for ( size_t i=start+1; i<start+count && i<x.size(); ++i )
    {
        if ( x[ i - 1 ] < 0.0f && x[ i ] >= 0.0f )
        {
            const Float64 t = (Float64) ( i - 1 ) + (Float64) -x[ i - 1 ] / (Float64) ( x[ i ] - x[ i - 1 ] );
            if ( first < 0.0 )
                first = t;
            last = t;
            ++crossings;
        }
    }
    if ( crossings < 2 )
        return 0.0;
    return (Float64) ( crossings - 1 ) * rate / ( last - first );
}

Float64
RmsInDecibels( const GUCEF::vector< Float32 >& x, size_t start, size_t count )
{
    Float64 sum = 0.0;
    for ( size_t i=start; i<start+count && i<x.size(); ++i )
        sum += (Float64) x[ i ] * (Float64) x[ i ];
    return 10.0 * log10( sum / (Float64) count );
}

/** The status of a future that has ended */
CORE::TTaskStatus
FutureStatus( const CORE::CFutureResult& future )
{
    CORE::CTaskPtr task = future.GetResult( 1000 );
    return task.IsNULL() ? CORE::TTaskStatus::TASKSTATUS_UNDEFINED : task->GetTaskStatus();
}

bool
FailedWith( const CORE::CFutureResult& future, CORE::TTaskStatus status )
{
    return future.HasNoFuture() && FutureStatus( future ) == status;
}

/** Waits for a future that the capture thread of a push device completes */
bool
WaitReady( AUDIO::CAudioEngine& engine, const CORE::CFutureResult& future )
{
    return !future.HasNoFuture() && engine.WaitFor( future, 10000 ) && !future.HasNoFuture();
}

/** Waits until the counter has moved on by at least 'more' */
bool
WaitForCalls( const MT::CTAtomic< UInt32 >& calls, UInt32 more, UInt32 timeoutInMs )
{
    const UInt32 target = calls.Load() + more;
    UInt32 waited = 0;
    while ( calls.Load() < target && waited < timeoutInMs )
    {
        MT::PrecisionDelay( 1 );
        ++waited;
    }
    return calls.Load() >= target;
}

/** Applies a layout and waits for it, for an engine whose audio thread is the null device rendered here */
bool
ApplyLayoutAndWait( AUDIO::CAudioEngine& engine, const AUDIO::CAudioBusLayout& layout )
{
    CORE::CFutureResult applied = engine.ApplyBusLayout( layout );
    if ( applied.HasNoFuture() )
        return false;
    PlaybackNull( engine )->Render( 256 );
    return engine.WaitFor( applied, 2000 ) && !applied.HasNoFuture();
}

}

/*-------------------------------------------------------------------------*/

static void
PerformEngineCaptureSetupTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "AudioCapture: The capture parameters have their defaults and survive a configuration round trip" )
    try
    {
        AUDIO::CAudioEngineParams defaults;
        ASSERT_FALSE( defaults.GetCaptureEnabled() );
        ASSERT_TRUE( 0 == defaults.GetCaptureProcessingRate() );
        ASSERT_TRUE( -1 == defaults.GetCaptureChannel() );
        ASSERT_TRUE( 4 == defaults.GetMaxCaptureConsumers() );
        ASSERT_TRUE( 8 == defaults.GetMaxCaptureEffects() );
        ASSERT_TRUE( 64 == defaults.GetCaptureCommandQueueSize() );
        ASSERT_TRUE( AUDIO::AUDIODIRECTION_CAPTURE == defaults.GetCaptureDeviceParams().GetDirection() );
        ASSERT_TRUE( defaults.GetCaptureDeviceParams().GetDeviceId().IsNULLOrEmpty() );

        AUDIO::CAudioEngineParams params;
        params.SetCaptureEnabled( true );
        params.SetCaptureProcessingRate( 16000 );
        params.SetCaptureChannel( 1 );
        params.SetMaxCaptureConsumers( 3 );
        params.SetMaxCaptureEffects( 5 );
        params.SetCaptureCommandQueueSize( 32 );
        params.GetCaptureDeviceParams().SetDeviceId( "hw:CARD=Mic,DEV=0" );
        params.GetCaptureDeviceParams().SetBufferSizeInFrames( 240 );
        params.GetCaptureDeviceParams().SetPreferredFormat( AUDIO::CAudioFormat( 48000, 2, AUDIO::SAMPLETYPE_INT16 ) );
        params.GetCaptureDeviceParams().GetOptions().Set( "periods", "3" );
        params.GetDeviceParams().SetDeviceId( "speaker" );

        // copies carry everything
        AUDIO::CAudioEngineParams copy( params );
        AUDIO::CAudioEngineParams assigned;
        assigned = params;
        ASSERT_TRUE( copy.GetCaptureEnabled() && assigned.GetCaptureEnabled() );
        ASSERT_TRUE( 3 == copy.GetMaxCaptureConsumers() && 3 == assigned.GetMaxCaptureConsumers() );
        ASSERT_TRUE( copy.GetCaptureDeviceParams().GetDeviceId() == "hw:CARD=Mic,DEV=0" && assigned.GetCaptureDeviceParams().GetDeviceId() == "hw:CARD=Mic,DEV=0" );

        CORE::CDataNode tree;
        ASSERT_TRUE( params.SaveConfig( tree ) );
        const CORE::CDataNode* section = tree.FindChild( AUDIO::CAudioEngineParams::ConfigSectionName );
        ASSERT_TRUE( GUCEF_NULL != section );
        ASSERT_TRUE( GUCEF_NULL != section->FindChild( AUDIO::CAudioEngineParams::CaptureDeviceSectionName ) );
        ASSERT_TRUE( GUCEF_NULL != section->FindChild( AUDIO::CAudioDeviceParams::ConfigSectionName ) );

        AUDIO::CAudioEngineParams loaded;
        ASSERT_TRUE( loaded.LoadConfig( tree ) );
        ASSERT_TRUE( loaded.GetCaptureEnabled() );
        ASSERT_TRUE( 16000 == loaded.GetCaptureProcessingRate() );
        ASSERT_TRUE( 1 == loaded.GetCaptureChannel() );
        ASSERT_TRUE( 3 == loaded.GetMaxCaptureConsumers() );
        ASSERT_TRUE( 5 == loaded.GetMaxCaptureEffects() );
        ASSERT_TRUE( 32 == loaded.GetCaptureCommandQueueSize() );
        ASSERT_TRUE( loaded.GetCaptureDeviceParams().GetDeviceId() == "hw:CARD=Mic,DEV=0" );
        ASSERT_TRUE( 240 == loaded.GetCaptureDeviceParams().GetBufferSizeInFrames() );
        ASSERT_TRUE( loaded.GetCaptureDeviceParams().GetPreferredFormat() == AUDIO::CAudioFormat( 48000, 2, AUDIO::SAMPLETYPE_INT16 ) );
        ASSERT_TRUE( loaded.GetCaptureDeviceParams().GetOptions().GetValueAlways( "periods" ).AsString() == "3" );
        ASSERT_TRUE( AUDIO::AUDIODIRECTION_CAPTURE == loaded.GetCaptureDeviceParams().GetDirection() );
        ASSERT_TRUE( loaded.GetDeviceParams().GetDeviceId() == "speaker" );        // the playback device is its own child

        // a negative channel goes through as well; what a section does not have keeps its value
        params.SetCaptureChannel( -1 );
        CORE::CDataNode second;
        ASSERT_TRUE( params.SaveConfig( second ) );
        ASSERT_TRUE( loaded.LoadConfig( second ) );
        ASSERT_TRUE( -1 == loaded.GetCaptureChannel() );
        CORE::CDataNode sparse;
        sparse.SetName( AUDIO::CAudioEngineParams::ConfigSectionName );
        sparse.SetAttribute( "maxVoices", 7 );
        ASSERT_TRUE( loaded.LoadConfig( sparse ) );
        ASSERT_TRUE( loaded.GetCaptureEnabled() && 16000 == loaded.GetCaptureProcessingRate() && 5 == loaded.GetMaxCaptureEffects() );
        ASSERT_TRUE( loaded.GetCaptureDeviceParams().GetDeviceId() == "hw:CARD=Mic,DEV=0" );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioCapture: Without captureEnabled the engine opens no capture device and refuses every capture call" )
    try
    {
        AUDIO::CAudioEngine engine;
        ASSERT_FALSE( engine.IsCaptureAvailable() );
        AUDIO::CAudioEngineParams params = CaptureParams();
        params.SetCaptureEnabled( false );
        ASSERT_TRUE( engine.Initialize( params ) );
        ASSERT_FALSE( engine.IsCaptureAvailable() );
        ASSERT_TRUE( engine.GetCaptureDevice().IsNULL() );
        ASSERT_FALSE( engine.GetCaptureProcessingFormat().IsValid() );
        ASSERT_TRUE( engine.CreateCaptureEffect( "limiter" ).IsNULL() );

        CRecordingConsumer consumer( 16 );
        ASSERT_TRUE( FailedWith( engine.AddCaptureConsumer( &consumer ), CORE::TTaskStatus::TASKSTATUS_SETUP_FAILED ) );
        ASSERT_TRUE( 0 == consumer.prepareCount );
        AUDIO::TAudioEffectPtr effect( GUCEF_NEW CTestAudioEffect() );
        ASSERT_TRUE( effect->Prepare( AUDIO::CAudioFormat( 48000, 1, AUDIO::SAMPLETYPE_FLOAT32 ), 4096 ) );
        ASSERT_TRUE( FailedWith( engine.InsertCaptureEffect( AUDIO::AUDIOCAPTURESTAGE_PRE_ECHO, 0, effect ), CORE::TTaskStatus::TASKSTATUS_SETUP_FAILED ) );
        ASSERT_TRUE( engine.GetCaptureEffect( AUDIO::AUDIOCAPTURESTAGE_PRE_ECHO, 0 ).IsNULL() );
        engine.SetCaptureEffectEnabled( AUDIO::AUDIOCAPTURESTAGE_PRE_ECHO, 0, false );     // harmless
        ASSERT_TRUE( 0 == engine.GetCaptureOverrunCount() );
        ASSERT_FALSE( engine.IsCaptureVoiceActive() );
        engine.Shutdown();
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioCapture: A capture device that cannot be opened or used leaves capture unavailable and the playback bit-identical to an engine without capture" )
    try
    {
        // the reference: no capture at all
        AUDIO::CAudioEngineParams plain = CaptureParams();
        plain.SetCaptureEnabled( false );
        AUDIO::CAudioEngine reference;
        ASSERT_TRUE( reference.Initialize( plain ) );
        AUDIO::CAudioBuffer pcm( AUDIO::CAudioFormat( 48000, 2, AUDIO::SAMPLETYPE_FLOAT32 ), 4800 );
        for ( UInt32 f=0; f<pcm.GetFrameCount(); ++f )
            for ( UInt32 c=0; c<2; ++c )
                pcm.SetSampleFromFloat( f, c, 0.3f * (Float32) sin( 2.0 * TestPi * ( 440.0 + 220.0 * c ) * f / 48000.0 ) );
        AUDIO::TAudioClipPtr clip( GUCEF_NEW AUDIO::CAudioClip( pcm ) );
        reference.Play( clip );
        PlaybackNull( reference )->Render( 6000 );

        // a device id the driver does not have, a processing rate out of range, a channel the device does not have
        for ( UInt32 variant=0; variant<3; ++variant )
        {
            AUDIO::CAudioEngineParams params = CaptureParams();
            if ( 0 == variant )
                params.GetCaptureDeviceParams().SetDeviceId( "nosuchdevice" );
            else
            if ( 1 == variant )
                params.SetCaptureProcessingRate( 96000 );
            else
                params.SetCaptureChannel( 2 );

            AUDIO::CAudioEngine engine;
            ASSERT_TRUE( engine.Initialize( params ) );
            ASSERT_TRUE( engine.IsInitialized() );
            ASSERT_FALSE( engine.IsCaptureAvailable() );
            ASSERT_TRUE( engine.GetCaptureDevice().IsNULL() );
            CRecordingConsumer consumer( 16 );
            ASSERT_TRUE( FailedWith( engine.AddCaptureConsumer( &consumer ), CORE::TTaskStatus::TASKSTATUS_SETUP_FAILED ) );
            ASSERT_TRUE( engine.CreateCaptureEffect( "limiter" ).IsNULL() );

            engine.Play( clip );
            PlaybackNull( engine )->Render( 6000 );
            const AUDIO::CAudioBuffer& expected = PlaybackNull( reference )->GetRenderedAudio();
            const AUDIO::CAudioBuffer& rendered = PlaybackNull( engine )->GetRenderedAudio();
            ASSERT_TRUE( rendered.GetFrameCount() == expected.GetFrameCount() && 6000 == rendered.GetFrameCount() );
            ASSERT_TRUE( 0 == memcmp( rendered.GetDataPtr(), expected.GetDataPtr(), (size_t) 6000 * expected.GetFormat().GetBytesPerFrame() ) );
            engine.Shutdown();
        }

        // a driver that has no capture device at all
        CaptureTestDriverConfig() = SCaptureTestDriverConfig();
        CaptureTestDriverConfig().refuseCapture = true;
        AUDIO::CAudioEngine engine;
        AUDIO::CAudioEngineParams params = PushCaptureParams( AUDIO::AUDIOTHREADING_COOPERATIVE );
        ASSERT_TRUE( engine.Initialize( params ) );
        ASSERT_FALSE( engine.IsCaptureAvailable() );
        ASSERT_TRUE( engine.GetDevice()->IsStarted() );
        engine.Shutdown();
        CaptureTestDriverConfig() = SCaptureTestDriverConfig();
        reference.Shutdown();
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
PerformEngineCaptureSignalTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "AudioCapture: A tone recorded by the capture device reaches the consumer at the processing rate with its frequency and level, for device rates 16, 44.1 and 48 kHz and processing rates 16 and 48 kHz" )
    try
    {
        const UInt32 deviceRates[ 3 ] = { 16000, 44100, 48000 };
        const UInt32 processingRates[ 2 ] = { 16000, 48000 };
        const Float64 expectedLevel = 20.0 * log10( 0.5 / sqrt( 2.0 ) );
        for ( UInt32 d=0; d<3; ++d )
        {
            for ( UInt32 p=0; p<2; ++p )
            {
                const UInt32 deviceRate = deviceRates[ d ];
                const UInt32 processingRate = processingRates[ p ];
                AUDIO::CAudioEngine engine;
                ASSERT_TRUE( engine.Initialize( CaptureParams( deviceRate, 2, processingRate ) ) );
                ASSERT_TRUE( engine.IsCaptureAvailable() );
                ASSERT_TRUE( engine.GetCaptureProcessingFormat() == AUDIO::CAudioFormat( processingRate, 1, AUDIO::SAMPLETYPE_FLOAT32 ) );
                ASSERT_TRUE( engine.GetCaptureDevice()->GetFormat().GetSampleRate() == deviceRate );

                const UInt32 seconds = 2;
                ASSERT_TRUE( CaptureNull( engine )->SetCaptureSource( MakeToneSource( engine.GetCaptureDevice()->GetFormat(), deviceRate * seconds, 1000.0, 0.5f, 0.5f ) ) );
                CRecordingConsumer consumer( processingRate * seconds + 4096 );
                CORE::CFutureResult added = engine.AddCaptureConsumer( &consumer );
                ASSERT_FALSE( added.HasNoFuture() );
                ASSERT_TRUE( 1 == consumer.prepareCount );
                ASSERT_TRUE( consumer.preparedFormat == engine.GetCaptureProcessingFormat() );
                ASSERT_FALSE( added.IsReady() );                                      // the capture thread has not run yet
                for ( UInt32 n=0; n<seconds * 10; ++n )
                    RenderCapture( engine, deviceRate / 10 );
                ASSERT_TRUE( added.IsReady() && !added.HasNoFuture() );

                // all frames arrive (less what the resampler holds back), in order, in blocks that fit
                ASSERT_TRUE( consumer.frames + 64 >= (size_t) processingRate * seconds );
                ASSERT_TRUE( 0 == consumer.gaps.Load() && 0 == consumer.oversized.Load() );
                ASSERT_TRUE( 0 == consumer.blocks[ 0 ].firstFrame && !consumer.blocks[ 0 ].discontinuity );
                ASSERT_TRUE( 0 != consumer.blocks[ 0 ].tickCount );

                const size_t start = processingRate / 5;
                const Float64 frequency = MeasureFrequency( consumer.samples, start, processingRate, (Float64) processingRate );
                const Float64 level = RmsInDecibels( consumer.samples, start, processingRate );
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioCapture: device " << deviceRate << " Hz, processing " << processingRate << " Hz: frequency " << frequency << " Hz, level " << level << " dB" );
                ASSERT_TRUE( fabs( frequency - 1000.0 ) <= 1.0 );
                ASSERT_TRUE( fabs( level - expectedLevel ) <= 0.05 );
                ASSERT_TRUE( fabs( consumer.blocks[ consumer.blockCount / 2 ].level - expectedLevel ) <= 0.1 );
                ASSERT_TRUE( fabs( engine.GetCaptureMeter().GetPeak( 0 ) - 0.5f ) <= 0.01f );
                ASSERT_FALSE( engine.IsCaptureVoiceActive() );
                engine.Shutdown();
            }
        }
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioCapture: Device periods longer than the block size and upsampling are cut into blocks no longer than the block size, continuously" )
    try
    {
        AUDIO::CAudioEngineParams params = CaptureParams( 16000, 1, 48000 );
        params.SetMaxBlockFrames( 100 );
        AUDIO::CAudioEngine engine;
        ASSERT_TRUE( engine.Initialize( params ) );
        ASSERT_TRUE( CaptureNull( engine )->SetCaptureSource( MakeToneSource( engine.GetCaptureDevice()->GetFormat(), 16000, 1000.0, 0.5f, 0.5f ) ) );
        CRecordingConsumer consumer( 48000 + 4096 );
        engine.AddCaptureConsumer( &consumer );
        ASSERT_TRUE( 1 == consumer.prepareCount && 100 == consumer.preparedMaxBlock );
        for ( UInt32 n=0; n<10; ++n )
            RenderCapture( engine, 1600 );
        ASSERT_TRUE( consumer.frames + 64 >= 48000 );
        ASSERT_TRUE( 0 == consumer.gaps.Load() && 0 == consumer.oversized.Load() );
        ASSERT_TRUE( fabs( MeasureFrequency( consumer.samples, 4800, 38400, 48000.0 ) - 1000.0 ) <= 1.0 );
        ASSERT_TRUE( engine.GetCaptureProcessingFormat().GetSampleRate() == 48000 );
        engine.Shutdown();
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioCapture: The capture path takes the channel it is told, or the mean of all channels, from any sample type" )
    try
    {
        const Int32 channels[ 3 ] = { 0, 1, -1 };
        const Float32 expectedGain[ 3 ] = { 0.6f, 0.2f, 0.4f };
        for ( UInt32 i=0; i<3; ++i )
        {
            AUDIO::CAudioEngineParams params = CaptureParams();
            params.SetCaptureChannel( channels[ i ] );
            AUDIO::CAudioEngine engine;
            ASSERT_TRUE( engine.Initialize( params ) );
            const AUDIO::CAudioFormat deviceFormat = engine.GetCaptureDevice()->GetFormat();
            ASSERT_TRUE( CaptureNull( engine )->SetCaptureSource( MakeToneSource( deviceFormat, 4800, 1000.0, 0.6f, 0.2f ) ) );
            CRecordingConsumer consumer( 4800 );
            engine.AddCaptureConsumer( &consumer );
            RenderCapture( engine, 4800 );
            ASSERT_TRUE( 4800 == consumer.frames );
            Float64 worst = 0.0;
            for ( UInt32 f=0; f<4800; ++f )
            {
                const Float64 expected = expectedGain[ i ] * sin( 2.0 * TestPi * 1000.0 * f / 48000.0 );
                const Float64 error = fabs( consumer.samples[ f ] - expected );
                worst = error > worst ? error : worst;
            }
            ASSERT_TRUE( worst < 1e-6 );
            engine.Shutdown();
        }

        // a 16 bit device: the path converts to float
        AUDIO::CAudioEngine engine;
        ASSERT_TRUE( engine.Initialize( CaptureParams( 48000, 2, 0, AUDIO::SAMPLETYPE_INT16 ) ) );
        ASSERT_TRUE( AUDIO::SAMPLETYPE_INT16 == engine.GetCaptureDevice()->GetFormat().GetSampleType() );
        ASSERT_TRUE( CaptureNull( engine )->SetCaptureSource( MakeToneSource( AUDIO::CAudioFormat( 48000, 2, AUDIO::SAMPLETYPE_FLOAT32 ), 4800, 1000.0, 0.5f, 0.5f ) ) );
        CRecordingConsumer consumer( 4800 );
        engine.AddCaptureConsumer( &consumer );
        RenderCapture( engine, 4800 );
        ASSERT_TRUE( 4800 == consumer.frames );
        Float64 worst = 0.0;
        for ( UInt32 f=0; f<4800; ++f )
        {
            const Float64 error = fabs( consumer.samples[ f ] - 0.5 * sin( 2.0 * TestPi * 1000.0 * f / 48000.0 ) );
            worst = error > worst ? error : worst;
        }
        ASSERT_TRUE( worst <= 1.5 / 32768.0 );
        engine.Shutdown();
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioCapture: Mute ramps the microphone gain over 10 ms without a step larger than the ramp, the meter and the block levels follow" )
    try
    {
        AUDIO::CAudioEngine engine;
        ASSERT_TRUE( engine.Initialize( CaptureParams() ) );
        ASSERT_TRUE( CaptureNull( engine )->SetCaptureSource( MakeToneSource( engine.GetCaptureDevice()->GetFormat(), 48000, 0.0, 0.0f, 0.0f, 0.5f ) ) );
        CRecordingConsumer consumer( 48000 );
        engine.AddCaptureConsumer( &consumer );
        ASSERT_FALSE( engine.IsCaptureMuted() );

        RenderCapture( engine, 4800 );
        bool open = true;
        for ( UInt32 f=0; f<4800; ++f )
            open = open && ( 0.5f == consumer.samples[ f ] );
        ASSERT_TRUE( open );                                                         // gain 1 passes the audio as it is
        ASSERT_TRUE( fabs( consumer.blocks[ 1 ].level - 20.0 * log10( 0.5 ) ) <= 0.001 );
        ASSERT_TRUE( 0.5f == engine.GetCaptureMeter().GetPeak( 0 ) );

        engine.SetCaptureMuted( true );
        ASSERT_TRUE( engine.IsCaptureMuted() );
        RenderCapture( engine, 4800 );
        const Float32 step = 0.5f / 480.0f;
        bool smooth = true;
        bool falling = true;
        for ( UInt32 f=4800; f<9600; ++f )
        {
            const Float32 change = consumer.samples[ f ] - consumer.samples[ f - 1 ];
            smooth = smooth && fabs( change ) <= step * 1.001f;
            falling = falling && change <= 0.0f;
        }
        ASSERT_TRUE( smooth && falling );
        ASSERT_TRUE( consumer.samples[ 4800 ] > 0.49f );                             // the ramp starts with the block after the switch
        ASSERT_TRUE( consumer.samples[ 4800 + 478 ] > 0.0f );                        // and takes 10 ms
        bool silent = true;
        for ( UInt32 f=4800 + 480; f<9600; ++f )
            silent = silent && ( 0.0f == consumer.samples[ f ] );
        ASSERT_TRUE( silent );
        ASSERT_TRUE( 0.0f == engine.GetCaptureMeter().GetPeak( 0 ) );
        ASSERT_TRUE( AUDIO::CAudioLevelMeter::MinimumDecibels == consumer.blocks[ consumer.blockCount - 1 ].level );

        engine.SetCaptureMuted( false );
        RenderCapture( engine, 4800 );
        smooth = true;
        for ( UInt32 f=9600; f<14400; ++f )
            smooth = smooth && fabs( consumer.samples[ f ] - consumer.samples[ f - 1 ] ) <= step * 1.001f;
        ASSERT_TRUE( smooth );
        bool back = true;
        for ( UInt32 f=9600 + 480; f<14400; ++f )
            back = back && ( 0.5f == consumer.samples[ f ] );
        ASSERT_TRUE( back );

        // the voice activity hook passes the audio and reports no voice until a detector is built in
        bool noVoice = true;
        for ( size_t b=0; b<consumer.blockCount; ++b )
            noVoice = noVoice && !consumer.blocks[ b ].voiceActive;
        ASSERT_TRUE( noVoice && !engine.IsCaptureVoiceActive() );

        // the switch survives a restart of the engine
        engine.SetCaptureMuted( true );
        engine.Shutdown();
        ASSERT_TRUE( engine.IsCaptureMuted() );
        ASSERT_TRUE( engine.Initialize( CaptureParams() ) );
        ASSERT_TRUE( CaptureNull( engine )->SetCaptureSource( MakeToneSource( engine.GetCaptureDevice()->GetFormat(), 4800, 0.0, 0.0f, 0.0f, 0.5f ) ) );
        CRecordingConsumer muted( 4800 );
        engine.AddCaptureConsumer( &muted );
        RenderCapture( engine, 4800 );
        bool mutedFromStart = true;
        for ( UInt32 f=0; f<4800; ++f )
            mutedFromStart = mutedFromStart && ( 0.0f == muted.samples[ f ] );
        ASSERT_TRUE( mutedFromStart );
        engine.SetCaptureMuted( false );
        engine.Shutdown();
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
PerformEngineCaptureStructureTests( void )
{GUCEF_TRACE;

    AUDIO::CAudioEffectRegistry& registry = AUDIO::CAudioGlobal::Instance()->GetAudioEffectRegistry();
    registry.RegisterFactory( AUDIO::TAudioEffectFactoryPtr( GUCEF_NEW TTestAudioEffectFactory() ) );
    registry.RegisterFactory( AUDIO::TAudioEffectFactoryPtr( GUCEF_NEW TResolvingTestEffectFactory() ) );

    GUCEF_TESTFW_TESTCASE( "AudioCapture: Capture effects go into the two stages with futures, in stage order, and are released on the owner side only" )
    try
    {
        AUDIO::CAudioEngine engine;
        ASSERT_TRUE( engine.Initialize( CaptureParams() ) );
        ASSERT_TRUE( CaptureNull( engine )->SetCaptureSource( MakeToneSource( engine.GetCaptureDevice()->GetFormat(), 48000, 0.0, 0.0f, 0.0f, 0.0f ) ) );
        CRecordingConsumer consumer( 48000 );
        engine.AddCaptureConsumer( &consumer );
        const AUDIO::CAudioFormat processing = engine.GetCaptureProcessingFormat();

        // pre: x + 0.25, post: 2 x. In order the silence becomes 0.5, the other way round it would be 0.25
        AUDIO::TAudioEffectPtr pre( GUCEF_NEW COrderTestEffect( 1.0f, 0.25f ) );
        AUDIO::TAudioEffectPtr post( GUCEF_NEW COrderTestEffect( 2.0f, 0.0f ) );
        ASSERT_TRUE( pre->Prepare( processing, 4096 ) && post->Prepare( processing, 4096 ) );
        CORE::CFutureResult preIn = engine.InsertCaptureEffect( AUDIO::AUDIOCAPTURESTAGE_PRE_ECHO, 2, pre );
        CORE::CFutureResult postIn = engine.InsertCaptureEffect( AUDIO::AUDIOCAPTURESTAGE_POST_ECHO, 0, post );
        ASSERT_FALSE( preIn.HasNoFuture() || postIn.HasNoFuture() );
        ASSERT_TRUE( engine.GetCaptureEffect( AUDIO::AUDIOCAPTURESTAGE_PRE_ECHO, 2 ) == pre );
        ASSERT_TRUE( engine.GetCaptureEffect( AUDIO::AUDIOCAPTURESTAGE_POST_ECHO, 0 ) == post );
        ASSERT_TRUE( engine.GetCaptureEffect( AUDIO::AUDIOCAPTURESTAGE_PRE_ECHO, 0 ).IsNULL() );

        // what is refused at once
        AUDIO::TAudioEffectPtr other( GUCEF_NEW CTestAudioEffect() );
        ASSERT_TRUE( other->Prepare( engine.GetMixFormat(), 4096 ) );                // the mix format: stereo
        ASSERT_TRUE( FailedWith( engine.InsertCaptureEffect( AUDIO::AUDIOCAPTURESTAGE_POST_ECHO, 1, other ), CORE::TTaskStatus::TASKSTATUS_SETUP_FAILED ) );
        ASSERT_TRUE( other->Prepare( processing, 4096 ) );
        ASSERT_TRUE( FailedWith( engine.InsertCaptureEffect( AUDIO::AUDIOCAPTURESTAGE_PRE_ECHO, 2, other ), CORE::TTaskStatus::TASKSTATUS_SETUP_FAILED ) );    // occupied
        ASSERT_TRUE( FailedWith( engine.InsertCaptureEffect( AUDIO::AUDIOCAPTURESTAGE_PRE_ECHO, 8, other ), CORE::TTaskStatus::TASKSTATUS_SETUP_FAILED ) );    // no such slot
        ASSERT_TRUE( FailedWith( engine.InsertCaptureEffect( AUDIO::AUDIOCAPTURESTAGE_COUNT, 0, other ), CORE::TTaskStatus::TASKSTATUS_SETUP_FAILED ) );      // no such stage
        ASSERT_TRUE( FailedWith( engine.InsertCaptureEffect( AUDIO::AUDIOCAPTURESTAGE_POST_ECHO, 1, pre ), CORE::TTaskStatus::TASKSTATUS_SETUP_FAILED ) );    // in a slot already
        ASSERT_TRUE( FailedWith( engine.InsertCaptureEffect( AUDIO::AUDIOCAPTURESTAGE_POST_ECHO, 1, AUDIO::TAudioEffectPtr() ), CORE::TTaskStatus::TASKSTATUS_SETUP_FAILED ) );
        ASSERT_TRUE( FailedWith( engine.RemoveCaptureEffect( other ), CORE::TTaskStatus::TASKSTATUS_SETUP_FAILED ) );                                         // not in a slot

        // one owner at a time: a capture effect is not a bus effect, a bus effect not a capture effect
        AUDIO::TAudioEffectPtr busEffect = engine.CreateEffect( "testeffect" );
        ASSERT_FALSE( busEffect.IsNULL() );
        CORE::CFutureResult busIn = engine.InsertEffect( engine.GetMasterBus(), 0, busEffect );
        ASSERT_FALSE( busIn.HasNoFuture() );
        ASSERT_TRUE( FailedWith( engine.InsertEffect( engine.GetMasterBus(), 1, pre ), CORE::TTaskStatus::TASKSTATUS_SETUP_FAILED ) );
        AUDIO::TAudioVoicePtr voice = engine.CreateVoice( MakeToneSource( engine.GetMixFormat(), 480, 100.0, 0.1f, 0.1f ) );
        voice->AddEffect( post );
        ASSERT_TRUE( FailedWith( engine.Start( voice ), CORE::TTaskStatus::TASKSTATUS_SETUP_FAILED ) );

        ASSERT_FALSE( preIn.IsReady() );
        RenderCapture( engine, 480 );
        ASSERT_TRUE( preIn.IsReady() && !preIn.HasNoFuture() && postIn.IsReady() && !postIn.HasNoFuture() );
        RenderCapture( engine, 480 );
        ASSERT_TRUE( 0.5f == consumer.samples[ 900 ] );
        ASSERT_TRUE( pre.StaticCast< COrderTestEffect >()->sequence.Load() < post.StaticCast< COrderTestEffect >()->sequence.Load() );

        // the slot switch crossfades to the unprocessed audio and back
        engine.SetCaptureEffectEnabled( AUDIO::AUDIOCAPTURESTAGE_POST_ECHO, 0, false );
        RenderCapture( engine, 2400 );
        ASSERT_TRUE( fabs( consumer.samples[ consumer.frames - 1 ] - 0.25f ) < 1e-6f );
        engine.SetCaptureEffectEnabled( AUDIO::AUDIOCAPTURESTAGE_POST_ECHO, 0, true );
        RenderCapture( engine, 2400 );
        ASSERT_TRUE( fabs( consumer.samples[ consumer.frames - 1 ] - 0.5f ) < 1e-6f );

        // removal: the slot is free at once, the effect lives until the owner side handled the release
        const UInt32 destroyedBefore = CTestAudioEffect::DestroyedCount().Load();
        CORE::CFutureResult preOut = engine.RemoveCaptureEffect( pre );
        ASSERT_FALSE( preOut.HasNoFuture() );
        ASSERT_TRUE( FailedWith( engine.RemoveCaptureEffect( pre ), CORE::TTaskStatus::TASKSTATUS_SETUP_FAILED ) );   // on its way out
        ASSERT_TRUE( engine.GetCaptureEffect( AUDIO::AUDIOCAPTURESTAGE_PRE_ECHO, 2 ).IsNULL() );
        CORE::CFutureResult otherIn = engine.InsertCaptureEffect( AUDIO::AUDIOCAPTURESTAGE_PRE_ECHO, 2, other );
        ASSERT_FALSE( otherIn.HasNoFuture() );
        pre.Unlink();
        CaptureNull( engine )->Render( 480 );                    // the capture thread let go of it, the event waits
        ASSERT_TRUE( destroyedBefore == CTestAudioEffect::DestroyedCount().Load() );
        ASSERT_FALSE( preOut.IsReady() );
        engine.Update();
        ASSERT_TRUE( destroyedBefore + 1 == CTestAudioEffect::DestroyedCount().Load() );
        ASSERT_TRUE( MT::GetCurrentThreadID() == CTestAudioEffect::LastDestructorThread().Load() );
        ASSERT_TRUE( preOut.IsReady() && !preOut.HasNoFuture() && otherIn.IsReady() && !otherIn.HasNoFuture() );

        // a capture effect made by the engine is prepared for the capture format
        AUDIO::TAudioEffectPtr limiter = engine.CreateCaptureEffect( "limiter" );
        ASSERT_FALSE( limiter.IsNULL() );
        ASSERT_TRUE( limiter->IsPreparedFor( processing, 4096 ) );
        ASSERT_TRUE( engine.CreateCaptureEffect( "nosucheffect" ).IsNULL() );
        CORE::CFutureResult limiterIn = engine.InsertCaptureEffect( AUDIO::AUDIOCAPTURESTAGE_POST_ECHO, 7, limiter );
        RenderCapture( engine, 480 );
        ASSERT_TRUE( limiterIn.IsReady() && !limiterIn.HasNoFuture() );
        ASSERT_TRUE( 0 == consumer.gaps.Load() );
        engine.Shutdown();
        ASSERT_TRUE( MT::GetCurrentThreadID() == CTestAudioEffect::LastDestructorThread().Load() || CTestAudioEffect::LastDestructorThread().Load() == 0 );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioCapture: Consumers are added and removed while capture runs: prepared once, a full table, duplicates, refusals and a full queue" )
    try
    {
        AUDIO::CAudioEngineParams params = CaptureParams();
        params.SetMaxCaptureConsumers( 2 );
        params.SetCaptureCommandQueueSize( 2 );
        AUDIO::CAudioEngine engine;
        ASSERT_TRUE( engine.Initialize( params ) );
        ASSERT_TRUE( CaptureNull( engine )->SetCaptureSource( MakeToneSource( engine.GetCaptureDevice()->GetFormat(), 48000, 1000.0, 0.5f, 0.5f ) ) );

        CRecordingConsumer a( 48000 ), b( 48000 ), c( 48000 );
        CRecordingConsumer refusing( 16, 16, false );
        ASSERT_TRUE( FailedWith( engine.AddCaptureConsumer( GUCEF_NULL ), CORE::TTaskStatus::TASKSTATUS_SETUP_FAILED ) );
        ASSERT_TRUE( FailedWith( engine.AddCaptureConsumer( &refusing ), CORE::TTaskStatus::TASKSTATUS_SETUP_FAILED ) );
        ASSERT_TRUE( 1 == refusing.prepareCount );
        CORE::CFutureResult addA = engine.AddCaptureConsumer( &a );
        CORE::CFutureResult addB = engine.AddCaptureConsumer( &b );
        ASSERT_FALSE( addA.HasNoFuture() || addB.HasNoFuture() );
        ASSERT_TRUE( FailedWith( engine.AddCaptureConsumer( &a ), CORE::TTaskStatus::TASKSTATUS_SETUP_FAILED ) );      // registered already
        ASSERT_TRUE( FailedWith( engine.AddCaptureConsumer( &c ), CORE::TTaskStatus::TASKSTATUS_SETUP_FAILED ) );      // the table is full
        ASSERT_TRUE( 1 == a.prepareCount && 0 == c.prepareCount );
        ASSERT_TRUE( FailedWith( engine.RemoveCaptureConsumer( &a ), CORE::TTaskStatus::TASKSTATUS_QUEUEING_FAILED ) ); // two commands on their way fill the queue
        ASSERT_TRUE( FailedWith( engine.RemoveCaptureConsumer( &c ), CORE::TTaskStatus::TASKSTATUS_SETUP_FAILED ) );   // not registered

        RenderCapture( engine, 960 );
        ASSERT_TRUE( addA.IsReady() && !addA.HasNoFuture() && addB.IsReady() && !addB.HasNoFuture() );
        ASSERT_TRUE( 960 == a.frames && 960 == b.frames );

        // the place of a consumer that leaves is free at once; the consumer itself is not, until the capture thread let go
        CORE::CFutureResult removeA = engine.RemoveCaptureConsumer( &a );
        ASSERT_FALSE( removeA.HasNoFuture() );
        ASSERT_TRUE( FailedWith( engine.AddCaptureConsumer( &a ), CORE::TTaskStatus::TASKSTATUS_SETUP_FAILED ) );      // still on its way out
        CORE::CFutureResult addC = engine.AddCaptureConsumer( &c );
        ASSERT_FALSE( addC.HasNoFuture() );
        RenderCapture( engine, 480 );
        ASSERT_TRUE( removeA.IsReady() && !removeA.HasNoFuture() && addC.IsReady() && !addC.HasNoFuture() );
        ASSERT_TRUE( 960 == a.frames && 1440 == b.frames && 480 == c.frames );
        ASSERT_TRUE( c.blocks[ 0 ].firstFrame == 960 );                               // positions are those of the path
        RenderCapture( engine, 480 );
        ASSERT_TRUE( 960 == a.frames && 1920 == b.frames && 960 == c.frames );

        // a consumer that is out can come back, prepared again
        CORE::CFutureResult removeB = engine.RemoveCaptureConsumer( &b );
        CORE::CFutureResult addA2 = engine.AddCaptureConsumer( &a );
        ASSERT_FALSE( removeB.HasNoFuture() || addA2.HasNoFuture() );
        ASSERT_TRUE( 2 == a.prepareCount );
        RenderCapture( engine, 480 );
        ASSERT_TRUE( removeB.IsReady() && addA2.IsReady() );
        ASSERT_TRUE( 1440 == a.frames && 1920 == b.frames && 1440 == c.frames );
        ASSERT_TRUE( 0 == a.oversized.Load() && 0 == b.gaps.Load() && 0 == c.gaps.Load() );

        // the same with the capture device stopped: Update() carries the commands out
        ASSERT_TRUE( engine.StopDevice() );
        CORE::CFutureResult removeC = engine.RemoveCaptureConsumer( &c );
        ASSERT_TRUE( engine.WaitFor( removeC, 1000 ) && !removeC.HasNoFuture() );
        ASSERT_TRUE( engine.StartDevice() );
        RenderCapture( engine, 480 );
        ASSERT_TRUE( 1440 == c.frames && 1920 == a.frames );
        engine.Shutdown();
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioCapture: InsertEffect() and InsertCaptureEffect() let the effect resolve what it refers to first, bus layouts as well" )
    try
    {
        AUDIO::CAudioEngine engine;
        ASSERT_TRUE( engine.Initialize( CaptureParams() ) );
        AUDIO::CAudioBus* music = engine.CreateBus( "music" );
        ASSERT_TRUE( GUCEF_NULL != music );

        CORE::CValueList keyedOnMusic;
        keyedOnMusic.Set( "keyBus", "music" );
        CORE::CValueList keyedOnNothing;
        keyedOnNothing.Set( "keyBus", "nosuchbus" );

        // a bus slot
        AUDIO::TAudioEffectPtr keyed = engine.CreateEffect( "resolvingtesteffect", keyedOnMusic );
        ASSERT_FALSE( keyed.IsNULL() );
        CORE::CFutureResult in = engine.InsertEffect( engine.GetMasterBus(), 0, keyed );
        ASSERT_FALSE( in.HasNoFuture() );
        ASSERT_TRUE( 1 == keyed.StaticCast< CResolvingTestEffect >()->resolveCalls );
        ASSERT_TRUE( music == keyed.StaticCast< CResolvingTestEffect >()->resolvedBus );
        AUDIO::TAudioEffectPtr unresolved = engine.CreateEffect( "resolvingtesteffect", keyedOnNothing );
        ASSERT_TRUE( FailedWith( engine.InsertEffect( engine.GetMasterBus(), 1, unresolved ), CORE::TTaskStatus::TASKSTATUS_SETUP_FAILED ) );
        ASSERT_TRUE( 1 == unresolved.StaticCast< CResolvingTestEffect >()->resolveCalls );
        ASSERT_TRUE( engine.GetBusEffect( engine.GetMasterBus(), 1 ).IsNULL() );
        AUDIO::TAudioEffectPtr plain = engine.CreateEffect( "resolvingtesteffect" );
        ASSERT_FALSE( engine.InsertEffect( music, 0, plain ).HasNoFuture() );           // nothing to resolve: true by default

        // a capture slot
        AUDIO::TAudioEffectPtr captureKeyed = engine.CreateCaptureEffect( "resolvingtesteffect", keyedOnMusic );
        ASSERT_FALSE( engine.InsertCaptureEffect( AUDIO::AUDIOCAPTURESTAGE_POST_ECHO, 0, captureKeyed ).HasNoFuture() );
        ASSERT_TRUE( music == captureKeyed.StaticCast< CResolvingTestEffect >()->resolvedBus );
        AUDIO::TAudioEffectPtr captureUnresolved = engine.CreateCaptureEffect( "resolvingtesteffect", keyedOnNothing );
        ASSERT_TRUE( FailedWith( engine.InsertCaptureEffect( AUDIO::AUDIOCAPTURESTAGE_POST_ECHO, 1, captureUnresolved ), CORE::TTaskStatus::TASKSTATUS_SETUP_FAILED ) );
        ASSERT_TRUE( engine.GetCaptureEffect( AUDIO::AUDIOCAPTURESTAGE_POST_ECHO, 1 ).IsNULL() );
        PlaybackNull( engine )->Render( 256 );
        RenderCapture( engine, 256 );

        // a layout that makes the key bus and an effect keyed on it: the bus comes first, so the effect finds it
        AUDIO::CAudioBusLayout layout;
        layout.AddBus( "key" );
        AUDIO::CAudioBusLayout::SBus& ducked = layout.AddBus( "ducked" );
        CORE::CValueList keyedOnKey;
        keyedOnKey.Set( "keyBus", "key" );
        AUDIO::TAudioEffectPtr described = AUDIO::CAudioGlobal::Instance()->GetAudioEffectRegistry().CreateEffect( "resolvingtesteffect", keyedOnKey );
        ASSERT_TRUE( described->SaveConfig( ducked.AddEffect( 0, "resolvingtesteffect" ).config ) );
        ASSERT_TRUE( ApplyLayoutAndWait( engine, layout ) );
        AUDIO::CAudioBus* key = engine.FindBus( "key" );
        AUDIO::TAudioEffectPtr placed = engine.GetBusEffect( engine.FindBus( "ducked" ), 0 );
        ASSERT_TRUE( GUCEF_NULL != key && !placed.IsNULL() );
        ASSERT_TRUE( key == placed.StaticCast< CResolvingTestEffect >()->resolvedBus );

        // the layout captured from the engine keeps the option, and applies to a fresh engine
        AUDIO::CAudioBusLayout captured;
        ASSERT_TRUE( engine.GetBusLayout( captured ) );
        AUDIO::CAudioEngine fresh;
        ASSERT_TRUE( fresh.Initialize( CaptureParams() ) );
        ASSERT_TRUE( ApplyLayoutAndWait( fresh, captured ) );
        AUDIO::TAudioEffectPtr rebuilt = fresh.GetBusEffect( fresh.FindBus( "ducked" ), 0 );
        ASSERT_FALSE( rebuilt.IsNULL() );
        ASSERT_TRUE( rebuilt.StaticCast< CResolvingTestEffect >()->keyBus == "key" );
        ASSERT_TRUE( fresh.FindBus( "key" ) == rebuilt.StaticCast< CResolvingTestEffect >()->resolvedBus );
        fresh.Shutdown();

        // a layout with an effect keyed on a bus that is nowhere: its insert fails, so does the layout
        AUDIO::CAudioBusLayout broken;
        AUDIO::CAudioBusLayout::SBus& brokenBus = broken.AddBus( "broken" );
        AUDIO::TAudioEffectPtr brokenEffect = AUDIO::CAudioGlobal::Instance()->GetAudioEffectRegistry().CreateEffect( "resolvingtesteffect", keyedOnNothing );
        ASSERT_TRUE( brokenEffect->SaveConfig( brokenBus.AddEffect( 0, "resolvingtesteffect" ).config ) );
        CORE::CFutureResult brokenApplied = engine.ApplyBusLayout( broken );
        ASSERT_TRUE( FailedWith( brokenApplied, CORE::TTaskStatus::TASKSTATUS_STARTUP_FAILED ) );
        ASSERT_TRUE( engine.GetBusEffect( engine.FindBus( "broken" ), 0 ).IsNULL() );
        engine.Shutdown();
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioCapture: Capture effects are not part of bus layouts: a layout neither lists them nor takes them out" )
    try
    {
        AUDIO::CAudioEngine engine;
        ASSERT_TRUE( engine.Initialize( CaptureParams() ) );
        AUDIO::TAudioEffectPtr captureEffect = engine.CreateCaptureEffect( "testeffect" );
        ASSERT_FALSE( engine.InsertCaptureEffect( AUDIO::AUDIOCAPTURESTAGE_PRE_ECHO, 0, captureEffect ).HasNoFuture() );
        AUDIO::TAudioEffectPtr busEffect = engine.CreateEffect( "limiter" );
        ASSERT_FALSE( engine.InsertEffect( engine.GetMasterBus(), 0, busEffect ).HasNoFuture() );
        PlaybackNull( engine )->Render( 256 );
        RenderCapture( engine, 256 );

        AUDIO::CAudioBusLayout layout;
        ASSERT_TRUE( engine.GetBusLayout( layout ) );
        size_t effects = 0;
        bool testEffectListed = false;
        for ( size_t b=0; b<layout.GetBuses().size(); ++b )
        {
            effects += layout.GetBuses()[ b ].effects.size();
            for ( size_t e=0; e<layout.GetBuses()[ b ].effects.size(); ++e )
                testEffectListed = testEffectListed || layout.GetBuses()[ b ].effects[ e ].typeName == "testeffect";
        }
        ASSERT_TRUE( 1 == effects && !testEffectListed );

        // a layout that empties the chain of the master bus leaves the capture stages as they are
        AUDIO::CAudioBusLayout empty;
        empty.AddBus( "master" );
        ASSERT_TRUE( ApplyLayoutAndWait( engine, empty ) );
        ASSERT_TRUE( engine.GetBusEffect( engine.GetMasterBus(), 0 ).IsNULL() );
        ASSERT_TRUE( engine.GetCaptureEffect( AUDIO::AUDIOCAPTURESTAGE_PRE_ECHO, 0 ) == captureEffect );
        engine.Shutdown();
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioCapture: A lost capture device raises CaptureDeviceLostEvent once, playback goes on and removals still complete" )
    try
    {
        AUDIO::CAudioEngine engine;
        ASSERT_TRUE( engine.Initialize( CaptureParams() ) );
        CEventCounter captureLost( &engine, AUDIO::CAudioEngine::CaptureDeviceLostEvent );
        CEventCounter playbackLost( &engine, AUDIO::CAudioEngine::DeviceLostEvent );
        ASSERT_TRUE( CaptureNull( engine )->SetCaptureSource( MakeToneSource( engine.GetCaptureDevice()->GetFormat(), 48000, 1000.0, 0.5f, 0.5f ) ) );
        CRecordingConsumer consumer( 48000 );
        AUDIO::TAudioEffectPtr effect = engine.CreateCaptureEffect( "testeffect" );
        engine.AddCaptureConsumer( &consumer );
        engine.InsertCaptureEffect( AUDIO::AUDIOCAPTURESTAGE_PRE_ECHO, 0, effect );
        RenderCapture( engine, 480 );
        ASSERT_TRUE( 480 == consumer.frames );

        CaptureNull( engine )->SimulateDeviceLoss();
        UpdateAndDeliver( engine );
        ASSERT_TRUE( 1 == captureLost.count.Load() && 0 == playbackLost.count.Load() );
        UpdateAndDeliver( engine );
        ASSERT_TRUE( 1 == captureLost.count.Load() );

        // the commands are carried out on the owner side now
        CORE::CFutureResult removed = engine.RemoveCaptureConsumer( &consumer );
        CORE::CFutureResult effectOut = engine.RemoveCaptureEffect( effect );
        ASSERT_TRUE( engine.WaitFor( removed, 1000 ) && !removed.HasNoFuture() );
        ASSERT_TRUE( engine.WaitFor( effectOut, 1000 ) && !effectOut.HasNoFuture() );

        // the engine plays on
        AUDIO::CAudioBuffer pcm( engine.GetMixFormat(), 480 );
        pcm.FillSilence();
        pcm.SetSampleFromFloat( 10, 0, 0.25f );
        AUDIO::TAudioClipPtr clip( GUCEF_NEW AUDIO::CAudioClip( pcm ) );
        CORE::CFutureResult started = engine.Play( clip );
        PlaybackNull( engine )->Render( 512 );
        UpdateAndDeliver( engine );
        ASSERT_TRUE( started.IsReady() && !started.HasNoFuture() );
        ASSERT_TRUE( PlaybackNull( engine )->GetRenderedAudio().GetSampleAsFloat( 10, 0 ) != 0.0f );
        ASSERT_TRUE( 0 == playbackLost.count.Load() );
        engine.Shutdown();

        // the same for a push device, lost on its own thread
        CaptureTestDriverConfig() = SCaptureTestDriverConfig();
        CaptureTestDriverConfig().lostAtPull = 20;
        AUDIO::CAudioEngine pushEngine;
        CEventCounter pushLost( &pushEngine, AUDIO::CAudioEngine::CaptureDeviceLostEvent );
        ASSERT_TRUE( pushEngine.Initialize( PushCaptureParams( AUDIO::AUDIOTHREADING_COOPERATIVE ) ) );
        CRecordingConsumer pushConsumer( 48000 );
        ASSERT_TRUE( WaitReady( pushEngine, pushEngine.AddCaptureConsumer( &pushConsumer ) ) );
        UInt32 waited = 0;
        while ( 0 == pushLost.count.Load() && waited < 10000 )
        {
            UpdateAndDeliver( pushEngine );
            MT::PrecisionDelay( 2 );
            waited += 2;
        }
        ASSERT_TRUE( 1 == pushLost.count.Load() );
        ASSERT_TRUE( WaitReady( pushEngine, pushEngine.RemoveCaptureConsumer( &pushConsumer ) ) );
        const UInt32 callsAfterLoss = pushConsumer.calls.Load();
        MT::PrecisionDelay( 20 );
        ASSERT_TRUE( callsAfterLoss == pushConsumer.calls.Load() );
        ASSERT_TRUE( pushEngine.GetDevice()->IsStarted() );
        pushEngine.Shutdown();
        CaptureTestDriverConfig() = SCaptureTestDriverConfig();
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    registry.TryUnregisterFactory( "resolvingtesteffect" );
    registry.TryUnregisterFactory( "testeffect" );
}

/*-------------------------------------------------------------------------*/

static void
PerformEngineCaptureThreadTests( void )
{GUCEF_TRACE;

    AUDIO::CAudioEffectRegistry& registry = AUDIO::CAudioGlobal::Instance()->GetAudioEffectRegistry();
    registry.RegisterFactory( AUDIO::TAudioEffectFactoryPtr( GUCEF_NEW TTestAudioEffectFactory() ) );

    GUCEF_TESTFW_TESTCASE( "AudioCapture: An overrun of the capture device marks the next block as a discontinuity and moves the frame position on by the lost time" )
    try
    {
        CaptureTestDriverConfig() = SCaptureTestDriverConfig();
        CaptureTestDriverConfig().overrunAtPull = 20;
        CaptureTestDriverConfig().stallInMs = 40;
        AUDIO::CAudioEngine engine;
        ASSERT_TRUE( engine.Initialize( PushCaptureParams( AUDIO::AUDIOTHREADING_COOPERATIVE ) ) );
        ASSERT_TRUE( engine.IsCaptureAvailable() );
        CRecordingConsumer consumer( 48000 * 4 );
        ASSERT_TRUE( WaitReady( engine, engine.AddCaptureConsumer( &consumer ) ) );
        ASSERT_TRUE( WaitForCalls( consumer.calls, 40, 10000 ) );
        ASSERT_TRUE( engine.StopDevice() );                         // the consumer is not called any more: its records can be read

        ASSERT_TRUE( 1 == engine.GetCaptureOverrunCount() );
        size_t marked = 0;
        size_t markedAt = 0;
        for ( size_t b=0; b<consumer.blockCount && b<consumer.blocks.size(); ++b )
        {
            if ( consumer.blocks[ b ].discontinuity )
            {
                ++marked;
                markedAt = b;
            }
        }
        ASSERT_TRUE( 1 == marked && markedAt > 0 );
        const CRecordingConsumer::SBlock& before = consumer.blocks[ markedAt - 1 ];
        const CRecordingConsumer::SBlock& after = consumer.blocks[ markedAt ];
        const UInt64 jump = after.firstFrame - ( before.firstFrame + before.frames );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioCapture: the overrun of 40 ms moved the frame position on by " << jump << " frames" );
        ASSERT_TRUE( jump >= 40 * 48 - TestPushPeriodFrames );
        ASSERT_TRUE( jump < 48000 );
        ASSERT_TRUE( after.tickCount > before.tickCount );
        ASSERT_TRUE( 0 == consumer.gaps.Load() );                    // every other block follows the one before
        engine.Shutdown();
        CaptureTestDriverConfig() = SCaptureTestDriverConfig();
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioCapture: Threaded engine with playback and capture on their own device threads: consumers and effects come and go while both run" )
    try
    {
        CaptureTestDriverConfig() = SCaptureTestDriverConfig();
        AUDIO::CAudioEngine engine;
        ASSERT_TRUE( engine.Initialize( PushCaptureParams( AUDIO::AUDIOTHREADING_THREADED ) ) );
        ASSERT_TRUE( engine.IsThreaded() && engine.IsCaptureAvailable() );
        AUDIO::CAudioBuffer pcm( engine.GetMixFormat(), 48000 );
        pcm.FillSilence();
        AUDIO::TAudioClipPtr clip( GUCEF_NEW AUDIO::CAudioClip( pcm ) );
        AUDIO::TAudioVoicePtr voice;
        engine.Play( clip, GUCEF_NULL, &voice );

        CRecordingConsumer steady( 16 );
        ASSERT_TRUE( WaitReady( engine, engine.AddCaptureConsumer( &steady ) ) );
        bool removedStayQuiet = true;
        bool allReady = true;
        for ( UInt32 round=0; round<20; ++round )
        {
            CRecordingConsumer passing( 16 );
            AUDIO::TAudioEffectPtr effect = engine.CreateCaptureEffect( "testeffect" );
            allReady = allReady && WaitReady( engine, engine.AddCaptureConsumer( &passing ) );
            allReady = allReady && WaitReady( engine, engine.InsertCaptureEffect( (AUDIO::TAudioCaptureStage) ( round % 2 ), round % 8, effect ) );
            engine.SetCaptureMuted( 0 == round % 3 );
            engine.SetCaptureEffectEnabled( (AUDIO::TAudioCaptureStage) ( round % 2 ), round % 8, 0 != round % 4 );
            allReady = allReady && WaitForCalls( passing.calls, 2, 10000 );
            const Float32 peak = engine.GetCaptureMeter().GetPeak( 0 );
            allReady = allReady && peak >= 0.0f;
            allReady = allReady && WaitReady( engine, engine.RemoveCaptureEffect( effect ) );
            allReady = allReady && WaitReady( engine, engine.RemoveCaptureConsumer( &passing ) );
            effect.Unlink();

            // once the removal is ready the consumer is never called again
            const UInt32 callsAtRemoval = passing.calls.Load();
            WaitForCalls( steady.calls, 3, 10000 );
            removedStayQuiet = removedStayQuiet && ( callsAtRemoval == passing.calls.Load() );
            removedStayQuiet = removedStayQuiet && ( passing.threadId.Load() != MT::GetCurrentThreadID() );
        }
        ASSERT_TRUE( allReady );
        ASSERT_TRUE( removedStayQuiet );
        ASSERT_TRUE( 0 == steady.gaps.Load() && 0 == steady.oversized.Load() );
        ASSERT_TRUE( steady.threadId.Load() != CTestAudioEffect::LastDestructorThread().Load() );    // effects are not released on the capture thread
        engine.SetCaptureMuted( false );

        ASSERT_TRUE( WaitReady( engine, engine.RemoveCaptureConsumer( &steady ) ) );
        engine.Stop( voice );
        engine.Shutdown();
        ASSERT_TRUE( 0 == engine.GetDroppedEventCount() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioCapture: Shutdown stops the capture thread first: no consumer call after it returns, pending futures end, effects are released on the calling thread" )
    try
    {
        for ( UInt32 mode=0; mode<2; ++mode )
        {
            CaptureTestDriverConfig() = SCaptureTestDriverConfig();
            AUDIO::CAudioEngine engine;
            ASSERT_TRUE( engine.Initialize( PushCaptureParams( 0 == mode ? AUDIO::AUDIOTHREADING_THREADED : AUDIO::AUDIOTHREADING_COOPERATIVE ) ) );
            CRecordingConsumer* consumer = GUCEF_NEW CRecordingConsumer( 16 );
            CRecordingConsumer* late = GUCEF_NEW CRecordingConsumer( 16 );
            AUDIO::TAudioEffectPtr effect = engine.CreateCaptureEffect( "testeffect" );
            ASSERT_TRUE( WaitReady( engine, engine.AddCaptureConsumer( consumer ) ) );
            ASSERT_TRUE( WaitReady( engine, engine.InsertCaptureEffect( AUDIO::AUDIOCAPTURESTAGE_POST_ECHO, 3, effect ) ) );
            ASSERT_TRUE( WaitForCalls( consumer->calls, 5, 10000 ) );

            const UInt32 destroyedBefore = CTestAudioEffect::DestroyedCount().Load();
            effect.Unlink();
            CORE::CFutureResult lateAdd = engine.AddCaptureConsumer( late );
            engine.Shutdown();
            ASSERT_TRUE( lateAdd.IsReady() );
            ASSERT_TRUE( destroyedBefore + 1 == CTestAudioEffect::DestroyedCount().Load() );
            ASSERT_TRUE( MT::GetCurrentThreadID() == CTestAudioEffect::LastDestructorThread().Load() );
            ASSERT_FALSE( engine.IsCaptureAvailable() );

            const UInt32 calls = consumer->calls.Load();
            const UInt32 lateCalls = late->calls.Load();
            GUCEF_DELETE consumer;                                  // the engine holds no pointer to it any more
            GUCEF_DELETE late;
            MT::PrecisionDelay( 10 );
            ASSERT_TRUE( calls > 0 );
            (void) lateCalls;

            // the engine starts again with its capture path afresh
            ASSERT_TRUE( engine.Initialize( PushCaptureParams( AUDIO::AUDIOTHREADING_COOPERATIVE ) ) );
            ASSERT_TRUE( engine.IsCaptureAvailable() );
            ASSERT_TRUE( 0 == engine.GetCaptureOverrunCount() );
            ASSERT_TRUE( engine.GetCaptureEffect( AUDIO::AUDIOCAPTURESTAGE_POST_ECHO, 3 ).IsNULL() );
            engine.Shutdown();
        }
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    // The capture callback is marked as a real time context in a RealtimeSanitizer build: any allocation, lock or blocking
    // call made on the capture thread aborts the run. Covered: conversion from 16 bit, downmix, resampling, both stages with
    // effects, the switch crossfade, the mute ramp, the meter, consumers, and the commands of every kind
    GUCEF_TESTFW_TESTCASE( "AudioCapture: The capture path does nothing a real time thread must not do (RealtimeSanitizer build)" )
    #ifndef GUCEF_AUDIO_TEST_CAPTURE_RTSAN_BUILD
    GUCEF_TESTFW_SKIP( "not a RealtimeSanitizer build (-fsanitize=realtime)" );
    #endif
    GUCEF_TESTFW_SKIP_IF( GUCEF_TESTFW_GETENV( "GUCEF_TEST_AUDIO_RTSAN_CLEAN" ) == "0", "switched off with GUCEF_TEST_AUDIO_RTSAN_CLEAN=0" );
    try
    {
        AUDIO::CAudioEngine engine;
        ASSERT_TRUE( engine.Initialize( CaptureParams( 48000, 2, 16000, AUDIO::SAMPLETYPE_INT16 ) ) );
        ASSERT_TRUE( CaptureNull( engine )->SetCaptureSource( MakeToneSource( AUDIO::CAudioFormat( 48000, 2, AUDIO::SAMPLETYPE_FLOAT32 ), 48000, 1000.0, 0.5f, 0.3f ) ) );
        CRecordingConsumer consumer( 16000 );
        AUDIO::TAudioEffectPtr filter = engine.CreateCaptureEffect( "filter" );
        AUDIO::TAudioEffectPtr test = engine.CreateCaptureEffect( "testeffect" );
        AUDIO::TAudioEffectPtr compressor = engine.CreateCaptureEffect( "compressor" );
        AUDIO::TAudioEffectPtr limiter = engine.CreateCaptureEffect( "limiter" );
        ASSERT_FALSE( filter.IsNULL() || test.IsNULL() || compressor.IsNULL() || limiter.IsNULL() );
        CORE::CFutureResult added = engine.AddCaptureConsumer( &consumer );
        engine.InsertCaptureEffect( AUDIO::AUDIOCAPTURESTAGE_PRE_ECHO, 0, filter );
        engine.InsertCaptureEffect( AUDIO::AUDIOCAPTURESTAGE_PRE_ECHO, 1, test );
        engine.InsertCaptureEffect( AUDIO::AUDIOCAPTURESTAGE_POST_ECHO, 0, compressor );
        engine.InsertCaptureEffect( AUDIO::AUDIOCAPTURESTAGE_POST_ECHO, 1, limiter );

        // under the sanitizer the first unsafe call aborts the run here
        RenderCapture( engine, 4800 );
        ASSERT_TRUE( added.IsReady() );
        test->SetParameter( CTestAudioEffect::PARAMETER_CUTOFF, 1000.0f );
        engine.SetCaptureEffectEnabled( AUDIO::AUDIOCAPTURESTAGE_PRE_ECHO, 1, false );
        engine.SetCaptureMuted( true );
        RenderCapture( engine, 4800 );
        engine.SetCaptureEffectEnabled( AUDIO::AUDIOCAPTURESTAGE_PRE_ECHO, 1, true );
        engine.SetCaptureMuted( false );
        RenderCapture( engine, 4800 );
        CORE::CFutureResult out = engine.RemoveCaptureEffect( compressor );
        CORE::CFutureResult removed = engine.RemoveCaptureConsumer( &consumer );
        compressor.Unlink();
        RenderCapture( engine, 4800 );
        ASSERT_TRUE( out.IsReady() && removed.IsReady() );
        ASSERT_TRUE( consumer.frames > 0 );
        engine.Shutdown();

        // a capture push device: the same on a device thread of its own
        CaptureTestDriverConfig() = SCaptureTestDriverConfig();
        CaptureTestDriverConfig().overrunAtPull = 5;
        CaptureTestDriverConfig().stallInMs = 5;
        AUDIO::CAudioEngine pushEngine;
        ASSERT_TRUE( pushEngine.Initialize( PushCaptureParams( AUDIO::AUDIOTHREADING_COOPERATIVE ) ) );
        CRecordingConsumer pushConsumer( 16 );
        AUDIO::TAudioEffectPtr pushEffect = pushEngine.CreateCaptureEffect( "limiter" );
        ASSERT_TRUE( WaitReady( pushEngine, pushEngine.AddCaptureConsumer( &pushConsumer ) ) );
        ASSERT_TRUE( WaitReady( pushEngine, pushEngine.InsertCaptureEffect( AUDIO::AUDIOCAPTURESTAGE_POST_ECHO, 0, pushEffect ) ) );
        ASSERT_TRUE( WaitForCalls( pushConsumer.calls, 20, 10000 ) );
        ASSERT_TRUE( WaitReady( pushEngine, pushEngine.RemoveCaptureEffect( pushEffect ) ) );
        pushEngine.Shutdown();
        CaptureTestDriverConfig() = SCaptureTestDriverConfig();
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    registry.TryUnregisterFactory( "testeffect" );
}

/*-------------------------------------------------------------------------*/

static void
PerformEngineCaptureTests( void )
{GUCEF_TRACE;

    AUDIO::CAudioDriverRegistry& drivers = AUDIO::CAudioGlobal::Instance()->GetAudioDriverRegistry();
    drivers.TryRegister( "capturetest", AUDIO::TAudioDriverPtr( GUCEF_NEW CCaptureTestDriver() ) );

    PerformEngineCaptureSetupTests();
    PerformEngineCaptureSignalTests();
    PerformEngineCaptureStructureTests();
    PerformEngineCaptureThreadTests();

    drivers.Unregister( "capturetest" );
}

/*-------------------------------------------------------------------------*/

void
PerformAudioCaptureTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING AudioCapture TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "gucefAUDIO.Capture" );

    PerformPushDeviceCaptureTests();
    PerformNullDeviceCaptureTests();
    PerformEngineCaptureTests();
}

/*-------------------------------------------------------------------------*/
