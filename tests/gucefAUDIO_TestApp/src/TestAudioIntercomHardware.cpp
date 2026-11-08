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
#include <stdlib.h>

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

#ifndef GUCEF_CORE_CPLUGINCONTROL_H
#include "CPluginControl.h"
#define GUCEF_CORE_CPLUGINCONTROL_H
#endif /* GUCEF_CORE_CPLUGINCONTROL_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_MT_CREALTIMESCHEDULINGSCOPE_H
#include "gucefMT_CRealtimeSchedulingScope.h"
#define GUCEF_MT_CREALTIMESCHEDULINGSCOPE_H
#endif /* GUCEF_MT_CREALTIMESCHEDULINGSCOPE_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestAudioVoiceCodecSupport.h"
#include "TestAudioVoiceStreamSupport.h"
#include "TestAudioIntercomUnit.h"
#include "TestAudioSimulatedVoiceLink.h"
#include "TestAudioIntercomHardware.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

/** The enabling variable and the item list: skips the test case unless this item was asked for */
#define SKIP_UNLESS_ITEM( item ) \
    GUCEF_TESTFW_SKIP_IF( !IsIntercomHardwareEnabled(), "GUCEF_TEST_AUDIO_INTERCOM is not 1" ); \
    GUCEF_TESTFW_SKIP_IF( !IsItemSelected( item ), "the item is not in GUCEF_TEST_AUDIO_INTERCOM_ITEMS" )

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )

namespace {

typedef GUCEF::vector< Float32 > TSamples;
typedef GUCEF::vector< Float64 > TTimes;

/*-------------------------------------------------------------------------*/

bool
IsIntercomHardwareEnabled( void )
{
    return GUCEF_TESTFW_GETENV( "GUCEF_TEST_AUDIO_INTERCOM" ) == "1";
}

/** The items variable: a comma separated list of 1 to 4, or "all" */
bool
IsItemSelected( UInt32 item )
{
    const CORE::CString items( GUCEF_TESTFW_GETENV( "GUCEF_TEST_AUDIO_INTERCOM_ITEMS" ).c_str() );
    if ( items.IsNULLOrEmpty() )
        return false;
    if ( "all" == items )
        return true;
    // the items are single digits: any digit of the list that matches selects the item
    const char digit = (char) ( '0' + item );
    return GUCEF_NULL != strchr( items.C_String(), digit );
}

UInt32
EnvNumber( const char* name, UInt32 defaultValue )
{
    const CORE::CString text( GUCEF_TESTFW_GETENV( name ).c_str() );
    return text.IsNULLOrEmpty() ? defaultValue : CORE::StringToUInt32( text );
}

Float64
EnvDecibels( const char* name, Float64 defaultValue )
{
    const CORE::CString text( GUCEF_TESTFW_GETENV( name ).c_str() );
    return text.IsNULLOrEmpty() ? defaultValue : CORE::StringToDouble( text );
}

/** The value with one decimal, as text (the log would show six decimals for a number) */
CORE::CString
Round( Float64 value )
{
    const bool negative = value < 0.0;
    const Int64 scaled = (Int64) floor( fabs( value ) * 10.0 + 0.5 );
    return CORE::CString( negative && scaled != 0 ? "-" : "" ) + CORE::Int64ToString( scaled / 10 ) + "." + CORE::Int64ToString( scaled % 10 );
}

Float64
ToDecibels( Float64 linear )
{
    return linear > 1.0e-9 ? 20.0 * log10( linear ) : -180.0;
}

Float64
ToLinear( Float64 decibels )
{
    return pow( 10.0, decibels / 20.0 );
}

/** Milliseconds of the tick counter of MT::PrecisionTickCount() as a point in time */
Float64
TickToMs( UInt64 tick )
{
    return MT::PrecisionTimerTicksToMs( tick );
}

/*-------------------------------------------------------------------------*/

bool
LoadAlsaDriver( AUDIO::CAudioDriverRegistry::TAudioDriverPtr& driver )
{
    if ( AUDIO::CAudioGlobal::Instance()->GetAudioDriverRegistry().TryFindDriver( "alsa", driver ) )
        return true;
    #ifdef GUCEF_DEBUG_MODE
    const CORE::CString first = "$MODULEDIR$/audiodriverALSA_d";
    const CORE::CString second = "$MODULEDIR$/audiodriverALSA";
    #else
    const CORE::CString first = "$MODULEDIR$/audiodriverALSA";
    const CORE::CString second = "$MODULEDIR$/audiodriverALSA_d";
    #endif
    CORE::CPluginControl& pluginControl = CORE::CCoreGlobal::Instance()->GetPluginControl();
    return ( pluginControl.AddPluginFromDir( first, "intercomhardware", true ) || pluginControl.AddPluginFromDir( second, "intercomhardware", true ) ) &&
           AUDIO::CAudioGlobal::Instance()->GetAudioDriverRegistry().TryFindDriver( "alsa", driver );
}

/*-------------------------------------------------------------------------*/

/** Statistics of the intervals between the calls of a device thread, from a histogram (no allocation on the device thread) */
class CIntervalRecorder
{
    public:

    enum { BinCount = 400 };            // bins of 0.05 ms for the deviation from the nominal interval, the last one holds the rest

    CIntervalRecorder( void ) : m_nominalMs( 0.0 ), m_lastTick( 0 ), m_count( 0 ), m_sumMs( 0.0 ), m_maxDeviationMs( 0.0 ) { Reset( 0.0 ); }

    void Reset( Float64 nominalMs )
    {
        m_nominalMs = nominalMs;
        m_lastTick = 0;
        m_count = 0;
        m_sumMs = 0.0;
        m_maxDeviationMs = 0.0;
        memset( m_bins, 0, sizeof( m_bins ) );
    }

    /** The device thread, at the start of each callback */
    void Record( UInt64 tick )
    {
        if ( 0 != m_lastTick )
        {
            const Float64 interval = TickToMs( tick - m_lastTick );
            const Float64 deviation = fabs( interval - m_nominalMs );
            UInt32 bin = (UInt32) ( deviation / 0.05 );
            if ( bin >= BinCount )
                bin = BinCount - 1;
            ++m_bins[ bin ];
            ++m_count;
            m_sumMs += deviation;
            if ( deviation > m_maxDeviationMs )
                m_maxDeviationMs = deviation;
        }
        m_lastTick = tick;
    }

    UInt64 GetCount( void ) const { return m_count; }
    Float64 GetMeanDeviationMs( void ) const { return m_count > 0 ? m_sumMs / (Float64) m_count : 0.0; }
    Float64 GetMaxDeviationMs( void ) const { return m_maxDeviationMs; }

    /** The deviation below which the given share (0 to 1) of the callbacks came, to the resolution of a bin */
    Float64 GetPercentileMs( Float64 share ) const
    {
        const UInt64 wanted = (UInt64) ceil( share * (Float64) m_count );
        UInt64 seen = 0;
        for ( UInt32 b=0; b<BinCount; ++b )
        {
            seen += m_bins[ b ];
            if ( seen >= wanted )
                return 0.05 * ( b + 1 );
        }
        return m_maxDeviationMs;
    }

    private:

    Float64 m_nominalMs;
    UInt64 m_lastTick;
    UInt64 m_count;
    Float64 m_sumMs;
    Float64 m_maxDeviationMs;
    UInt32 m_bins[ BinCount ];
};

/*-------------------------------------------------------------------------*/

/** Plays silence and discards the capture, noting when each callback came (item 1) */
class CTimingCallback : public AUDIO::IAudioDeviceCallback
{
    public:

    CTimingCallback( void ) : bytesPerFrame( 0 ), frames( 0 ), lost( 0 ), timing() {}

    virtual void OnRenderAudio( void* buffer, UInt32 frameCount ) GUCEF_VIRTUAL_OVERRIDE
    {
        timing.Record( MT::PrecisionTickCount() );
        memset( buffer, 0, (size_t) frameCount * bytesPerFrame );
        frames.FetchAdd( frameCount );
    }

    virtual void OnCaptureAudio( const void*, UInt32 frameCount ) GUCEF_VIRTUAL_OVERRIDE
    {
        timing.Record( MT::PrecisionTickCount() );
        frames.FetchAdd( frameCount );
    }

    virtual void OnDeviceLost( void ) GUCEF_VIRTUAL_OVERRIDE { lost.FetchAdd( 1 ); }

    UInt32 bytesPerFrame;
    MT::CTAtomic< UInt64 > frames;
    MT::CTAtomic< UInt32 > lost;
    CIntervalRecorder timing;
};

/*-------------------------------------------------------------------------*/

struct SLoadThread
{
    MT::CTAtomic< UInt32 > stop;
    MT::CTAtomic< UInt64 > spins;
    SLoadThread( void ) : stop( 0 ), spins( 0 ) {}
};

UInt32 GUCEF_CALLSPEC_PREFIX
LoadMain( void* data ) GUCEF_CALLSPEC_SUFFIX
{
    SLoadThread* load = static_cast< SLoadThread* >( data );
    volatile Float64 value = 1.0;
    UInt64 spins = 0;
    while ( 0 == load->stop.Load() )
    {
        for ( UInt32 i=0; i<10000; ++i )
            value = value * 1.0000001 + 0.5;
        ++spins;
    }
    load->spins.Store( spins );
    return 0;
}

/** The CPU load generator: busy threads for the length of a run */
class CLoadGenerator
{
    public:

    explicit CLoadGenerator( UInt32 threadCount ) : m_count( threadCount ), m_threads( threadCount, (MT::SThreadData*) GUCEF_NULL ), m_loads( threadCount )
    {
        for ( UInt32 i=0; i<m_count; ++i )
        {
            m_threads[ i ] = MT::ThreadDataReserve();
            if ( 0 == MT::ThreadCreate( m_threads[ i ], &LoadMain, &m_loads[ i ] ) )
            {
                MT::ThreadDataCleanup( m_threads[ i ] );
                m_threads[ i ] = GUCEF_NULL;
            }
        }
    }

    ~CLoadGenerator()
    {
        for ( UInt32 i=0; i<m_count; ++i )
            m_loads[ i ].stop.Store( 1 );
        for ( UInt32 i=0; i<m_count; ++i )
            if ( GUCEF_NULL != m_threads[ i ] )
            {
                MT::ThreadWait( m_threads[ i ], 60000 );
                MT::ThreadDataCleanup( m_threads[ i ] );
            }
    }

    private:

    UInt32 m_count;
    GUCEF::vector< MT::SThreadData* > m_threads;
    GUCEF::vector< SLoadThread > m_loads;
};

/*-------------------------------------------------------------------------*/

/** What one run of item 1 found */
struct SDeviceRunResult
{
    bool opened;
    UInt32 playPeriod, playLatency, capPeriod, capLatency, sampleRate;
    UInt64 playUnderruns, capOverruns, playFrames, capFrames;
    UInt32 playLost, capLost;
    bool playRealtime, capRealtime;
    Int32 playRealtimeError, capRealtimeError;
    SDeviceRunResult( void )
        : opened( false ), playPeriod( 0 ), playLatency( 0 ), capPeriod( 0 ), capLatency( 0 ), sampleRate( 0 ), playUnderruns( 0 ), capOverruns( 0 )
        , playFrames( 0 ), capFrames( 0 ), playLost( 0 ), capLost( 0 ), playRealtime( false ), capRealtime( false ), playRealtimeError( 0 ), capRealtimeError( 0 )
    {}
};

void
FillDeviceParams( AUDIO::CAudioDeviceParams& params, const CORE::CString& deviceId, AUDIO::TAudioDirection direction, UInt32 period, UInt32 periods, UInt32 realtimePriority )
{
    params.SetDeviceId( deviceId );
    params.SetDirection( direction );
    params.SetBufferSizeInFrames( period );
    params.GetOptions().Delete( AUDIO::CAudioEngineParams::PeriodsOptionName );
    params.GetOptions().Set( AUDIO::CAudioEngineParams::PeriodsOptionName, CORE::UInt32ToString( periods ) );
    params.GetOptions().Delete( AUDIO::CAudioPushDevice::RealtimePriorityOptionName );
    params.GetOptions().Set( AUDIO::CAudioPushDevice::RealtimePriorityOptionName, CORE::UInt32ToString( realtimePriority ) );
}

/** Opens a playback and a capture device at the given period and number of periods and runs both for 'seconds' */
bool
RunDevicePair( AUDIO::CAudioDriverRegistry::TAudioDriverPtr& driver, const CORE::CString& playId, const CORE::CString& captureId,
               UInt32 periods, UInt32 realtimePriority, UInt32 seconds, SDeviceRunResult& result, CIntervalRecorder& playTiming, CIntervalRecorder& capTiming )
{
    const UInt32 period = 240;
    AUDIO::CAudioDeviceParams playParams, capParams;
    FillDeviceParams( playParams, playId, AUDIO::AUDIODIRECTION_PLAYBACK, period, periods, realtimePriority );
    FillDeviceParams( capParams, captureId, AUDIO::AUDIODIRECTION_CAPTURE, period, periods, realtimePriority );

    CTimingCallback playCallback, capCallback;
    AUDIO::TAudioDevicePtr playDevice, capDevice;
    if ( !driver->OpenDevice( playParams, &playCallback, playDevice ) || !driver->OpenDevice( capParams, &capCallback, capDevice ) )
        return false;
    AUDIO::CAudioPushDevice* playPush = playDevice->AsPushDevice();
    AUDIO::CAudioPushDevice* capPush = capDevice->AsPushDevice();
    if ( GUCEF_NULL == playPush || GUCEF_NULL == capPush )
        return false;
    result.sampleRate = playDevice->GetFormat().GetSampleRate();
    playCallback.bytesPerFrame = playDevice->GetFormat().GetBytesPerFrame();
    const Float64 playNominal = 1000.0 * playDevice->GetPeriodSizeInFrames() / playDevice->GetFormat().GetSampleRate();
    const Float64 capNominal = 1000.0 * capDevice->GetPeriodSizeInFrames() / capDevice->GetFormat().GetSampleRate();
    playCallback.timing.Reset( playNominal );
    capCallback.timing.Reset( capNominal );

    if ( !playDevice->Start() || !capDevice->Start() )
        return false;
    const UInt64 start = MT::PrecisionTickCount();
    UInt32 lastReport = 0;
    while ( TickToMs( MT::PrecisionTickCount() - start ) < 1000.0 * seconds )
    {
        MT::PrecisionDelay( 250 );
        const UInt32 elapsed = (UInt32) ( TickToMs( MT::PrecisionTickCount() - start ) / 1000.0 );
        if ( elapsed >= lastReport + 60 )
        {
            lastReport = elapsed;
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "IntercomHardware: item 1, " << elapsed << " s of " << seconds << " s, playback underruns " << playPush->GetUnderrunCount() << ", capture overruns " << capPush->GetOverrunCount() );
        }
    }
    capDevice->Stop();
    playDevice->Stop();

    result.opened = true;
    result.playPeriod = playDevice->GetPeriodSizeInFrames();
    result.playLatency = playDevice->GetLatencyInFrames();
    result.capPeriod = capDevice->GetPeriodSizeInFrames();
    result.capLatency = capDevice->GetLatencyInFrames();
    result.playUnderruns = playPush->GetUnderrunCount();
    result.capOverruns = capPush->GetOverrunCount();
    result.playFrames = playCallback.frames.Load();
    result.capFrames = capCallback.frames.Load();
    result.playLost = playCallback.lost.Load();
    result.capLost = capCallback.lost.Load();
    result.playRealtime = playPush->IsRealtimeSchedulingActive();
    result.capRealtime = capPush->IsRealtimeSchedulingActive();
    result.playRealtimeError = playPush->GetRealtimeSchedulingError();
    result.capRealtimeError = capPush->GetRealtimeSchedulingError();
    playTiming = playCallback.timing;
    capTiming = capCallback.timing;
    return true;
}

void
ReportTiming( const char* which, const CIntervalRecorder& timing )
{
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "IntercomHardware:   " << which << " thread: " << timing.GetCount() << " callbacks, deviation of the interval from the period: mean " << Round( timing.GetMeanDeviationMs() ) <<
                                      " ms, 99th percentile " << Round( timing.GetPercentileMs( 0.99 ) ) << " ms, 99.9th percentile " << Round( timing.GetPercentileMs( 0.999 ) ) << " ms, largest " << Round( timing.GetMaxDeviationMs() ) << " ms" );
}

/*-------------------------------------------------------------------------*/

/**
 *  The stimulus of the engine tests: silence, clicks (every 500 ms, the click of CPcmClickSignal scaled to a peak level)
 *  or noise. Clicks note when the block that holds them was asked for and where in it they start. Audio thread.
 */
class CStimulusSource : public AUDIO::IAudioSource
{
    public:

    enum EMode { MODE_SILENCE, MODE_CLICKS, MODE_NOISE };
    enum { MaxClicks = 1024 };

    CStimulusSource( UInt32 sampleRate, EMode mode, Float64 levelDb )
        : m_format( sampleRate, 1, AUDIO::SAMPLETYPE_FLOAT32 ), m_mode( mode ), m_signal( sampleRate, sampleRate / 2, sampleRate / 10, false ), m_gain( (Float32) ( ToLinear( levelDb ) / 0.45 ) )
        , m_noiseGain( (Float32) ToLinear( levelDb ) ), m_state( 0x2545F491 ), m_low( 0.0f ), m_position( 0 ), m_clickCount( 0 ), m_sumSquares( 0.0 ), m_noiseFrames( 0 )
    {
        memset( m_clickTicks, 0, sizeof( m_clickTicks ) );
        memset( m_clickOffsets, 0, sizeof( m_clickOffsets ) );
    }

    virtual const AUDIO::CAudioFormat& GetFormat( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_format; }

    virtual UInt32 ReadFrames( void* destination, UInt32 frameCapacity ) GUCEF_VIRTUAL_OVERRIDE
    {
        Float32* out = static_cast< Float32* >( destination );
        const UInt64 tick = MT::PrecisionTickCount();
        for ( UInt32 i=0; i<frameCapacity; ++i )
        {
            const UInt64 n = m_position + i;
            Float32 value = 0.0f;
            if ( MODE_CLICKS == m_mode )
            {
                value = m_gain * m_signal.Sample( n );
                if ( 0 == m_signal.ClickOffset( n ) )
                {
                    const UInt32 slot = m_clickCount.Load();
                    if ( slot < MaxClicks )
                    {
                        m_clickTicks[ slot ] = tick;
                        m_clickOffsets[ slot ] = i;
                        m_clickCount.Store( slot + 1 );
                    }
                }
            }
            else if ( MODE_NOISE == m_mode )
            {
                m_state ^= m_state << 13;
                m_state ^= m_state >> 17;
                m_state ^= m_state << 5;
                const Float32 white = (Float32) ( m_state & 0xFFFF ) / 32768.0f - 1.0f;
                m_low += 0.5f * ( white - m_low );
                value = m_noiseGain * 3.0f * m_low;               // the filtered noise has an RMS of a third: the level is the RMS
                m_sumSquares += (Float64) value * value;
                ++m_noiseFrames;
            }
            out[ i ] = value;
        }
        m_position += frameCapacity;
        return frameCapacity;
    }

    virtual bool IsEndOfStream( void ) const GUCEF_VIRTUAL_OVERRIDE { return false; }
    virtual bool IsRealtimeSafe( void ) const GUCEF_VIRTUAL_OVERRIDE { return true; }
    virtual UInt64 GetPositionInFrames( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_position; }

    /** After the run: the clicks noted, with the moment (ms of the tick counter) each one was handed to the device */
    UInt32 GetClickCount( void ) const { return m_clickCount.Load(); }
    Float64 GetClickTimeMs( UInt32 index ) const { return TickToMs( m_clickTicks[ index ] ) + 1000.0 * m_clickOffsets[ index ] / m_format.GetSampleRate(); }
    Float64 GetNoiseRms( void ) const { return m_noiseFrames > 0 ? sqrt( m_sumSquares / (Float64) m_noiseFrames ) : 0.0; }
    const VoiceLinkTest::CPcmClickSignal& GetSignal( void ) const { return m_signal; }

    private:

    AUDIO::CAudioFormat m_format;
    EMode m_mode;
    VoiceLinkTest::CPcmClickSignal m_signal;
    Float32 m_gain;
    Float32 m_noiseGain;
    UInt32 m_state;
    Float32 m_low;
    UInt64 m_position;
    MT::CTAtomic< UInt32 > m_clickCount;
    UInt64 m_clickTicks[ MaxClicks ];
    UInt32 m_clickOffsets[ MaxClicks ];
    Float64 m_sumSquares;
    UInt64 m_noiseFrames;
};

/*-------------------------------------------------------------------------*/

/** Keeps the processed capture audio in memory, with the time each block arrived (no allocation on the capture thread) */
class CCaptureRecorder : public AUDIO::IAudioCaptureConsumer
{
    public:

    struct SBlock
    {
        UInt64 start;           /**< index of its first sample in the recording */
        UInt32 count;
        UInt64 tick;
    };

    explicit CCaptureRecorder( UInt32 seconds, UInt32 sampleRate )
        : m_samples( (size_t) ( seconds + 4 ) * sampleRate, 0.0f ), m_blocks( (size_t) ( seconds + 4 ) * 4000 ), m_sampleCount( 0 ), m_blockCount( 0 ), m_rate( sampleRate ), m_discontinuities( 0 )
    {}

    virtual bool PrepareForCapture( const AUDIO::CAudioFormat& processingFormat, UInt32 ) GUCEF_VIRTUAL_OVERRIDE
    {
        return processingFormat.GetSampleRate() == m_rate;
    }

    virtual void OnCapturedAudio( const Float32* mono, UInt32 frameCount, const SCaptureBlockInfo& info ) GUCEF_VIRTUAL_OVERRIDE
    {
        const UInt64 used = m_sampleCount.Load();
        const UInt32 blocks = m_blockCount.Load();
        if ( used + frameCount > m_samples.size() || blocks >= m_blocks.size() )
            return;
        memcpy( &m_samples[ (size_t) used ], mono, (size_t) frameCount * sizeof( Float32 ) );
        m_blocks[ blocks ].start = used;
        m_blocks[ blocks ].count = frameCount;
        m_blocks[ blocks ].tick = info.tickCount;
        if ( info.discontinuity )
            m_discontinuities.FetchAdd( 1 );
        m_blockCount.Store( blocks + 1 );
        m_sampleCount.Store( used + frameCount );
    }

    UInt64 GetSampleCount( void ) const { return m_sampleCount.Load(); }
    UInt32 GetBlockCount( void ) const { return m_blockCount.Load(); }
    UInt32 GetDiscontinuityCount( void ) const { return m_discontinuities.Load(); }
    const TSamples& GetSamples( void ) const { return m_samples; }
    const GUCEF::vector< SBlock >& GetBlocks( void ) const { return m_blocks; }

    /** The moment, in ms of the tick counter, at which sample 'index' of the recording was captured: the block arrives at its end */
    Float64 TimeOfSampleMs( UInt64 index ) const
    {
        const UInt32 blocks = m_blockCount.Load();
        for ( UInt32 b=0; b<blocks; ++b )
            if ( index >= m_blocks[ b ].start && index < m_blocks[ b ].start + m_blocks[ b ].count )
                return TickToMs( m_blocks[ b ].tick ) - 1000.0 * (Float64) ( m_blocks[ b ].start + m_blocks[ b ].count - index ) / m_rate;
        return 0.0;
    }

    /** RMS of a range of the recording, linear */
    Float64 RmsOfRange( UInt64 from, UInt64 to ) const
    {
        const UInt64 end = to < m_sampleCount.Load() ? to : m_sampleCount.Load();
        if ( from >= end )
            return 0.0;
        Float64 sum = 0.0;
        for ( UInt64 i=from; i<end; ++i )
            sum += (Float64) m_samples[ (size_t) i ] * m_samples[ (size_t) i ];
        return sqrt( sum / (Float64) ( end - from ) );
    }

    private:

    TSamples m_samples;
    GUCEF::vector< SBlock > m_blocks;
    MT::CTAtomic< UInt64 > m_sampleCount;
    MT::CTAtomic< UInt32 > m_blockCount;
    UInt32 m_rate;
    MT::CTAtomic< UInt32 > m_discontinuities;
};

/*-------------------------------------------------------------------------*/

/** A click found in a recording: the time it was captured (ms of the tick counter) and how well it matched the click */
struct SDetection
{
    Float64 timeMs;
    Float64 score;
};

/**
 *  Cross-correlates the recording with the click and returns the peaks above 'threshold' of the normalised correlation
 *  (the correlation divided by the energies of the click and of the stretch under it, so the level does not matter)
 *  that are at least 'separationSamples' apart. Items 2 and 3
 */
void
FindClicks( const CCaptureRecorder& recorder, const Float32* click, UInt32 clickLength, Float64 threshold, UInt32 separationSamples, GUCEF::vector< SDetection >& found )
{
    const TSamples& x = recorder.GetSamples();
    const UInt64 total = recorder.GetSampleCount();
    if ( total < clickLength )
        return;
    Float64 clickEnergy = 0.0;
    for ( UInt32 i=0; i<clickLength; ++i )
        clickEnergy += (Float64) click[ i ] * click[ i ];

    const UInt64 positions = total - clickLength + 1;
    TSamples score( (size_t) positions, 0.0f );
    Float64 window = 0.0;
    for ( UInt32 i=0; i<clickLength; ++i )
        window += (Float64) x[ i ] * x[ i ];
    for ( UInt64 p=0; p<positions; ++p )
    {
        if ( p > 0 )
        {
            const Float64 leaving = x[ (size_t) ( p - 1 ) ];
            const Float64 entering = x[ (size_t) ( p + clickLength - 1 ) ];
            window += entering * entering - leaving * leaving;
            if ( window < 0.0 )
                window = 0.0;
        }
        if ( window < 1.0e-9 )
            continue;
        Float64 sum = 0.0;
        for ( UInt32 i=0; i<clickLength; ++i )
            sum += (Float64) x[ (size_t) ( p + i ) ] * click[ i ];
        score[ (size_t) p ] = (Float32) ( sum / sqrt( window * clickEnergy ) );
    }

    // strongest first within each stretch of 'separationSamples'
    UInt64 p = 0;
    while ( p < positions )
    {
        if ( score[ (size_t) p ] < threshold )
        {
            ++p;
            continue;
        }
        UInt64 best = p;
        const UInt64 end = p + separationSamples < positions ? p + separationSamples : positions;
        for ( UInt64 q=p; q<end; ++q )
            if ( score[ (size_t) q ] > score[ (size_t) best ] )
                best = q;
        SDetection detection;
        detection.timeMs = recorder.TimeOfSampleMs( best );
        detection.score = score[ (size_t) best ];
        found.push_back( detection );
        p = best + separationSamples;
    }
}

struct SSummary
{
    UInt32 count;
    Float64 mean, minimum, maximum, deviation;
    SSummary( void ) : count( 0 ), mean( 0.0 ), minimum( 0.0 ), maximum( 0.0 ), deviation( 0.0 ) {}
};

SSummary
Summarize( const TTimes& values )
{
    SSummary summary;
    summary.count = (UInt32) values.size();
    if ( values.empty() )
        return summary;
    summary.minimum = summary.maximum = values[ 0 ];
    Float64 sum = 0.0;
    for ( size_t i=0; i<values.size(); ++i )
    {
        sum += values[ i ];
        if ( values[ i ] < summary.minimum )
            summary.minimum = values[ i ];
        if ( values[ i ] > summary.maximum )
            summary.maximum = values[ i ];
    }
    summary.mean = sum / (Float64) values.size();
    Float64 squares = 0.0;
    for ( size_t i=0; i<values.size(); ++i )
        squares += ( values[ i ] - summary.mean ) * ( values[ i ] - summary.mean );
    summary.deviation = sqrt( squares / (Float64) values.size() );
    return summary;
}

/** The first detection after each click was played, within 400 ms: the round trip times in ms */
void
MatchRoundTrips( const CStimulusSource& source, const GUCEF::vector< SDetection >& found, TTimes& roundTrips, TTimes& firstDetectionTimes )
{
    for ( UInt32 c=0; c<source.GetClickCount(); ++c )
    {
        const Float64 played = source.GetClickTimeMs( c );
        for ( size_t d=0; d<found.size(); ++d )
            if ( found[ d ].timeMs > played && found[ d ].timeMs - played < 400.0 )
            {
                roundTrips.push_back( found[ d ].timeMs - played );
                firstDetectionTimes.push_back( found[ d ].timeMs );
                break;
            }
    }
}

/*-------------------------------------------------------------------------*/

/** An engine on the ALSA devices with the low latency profile, capture enabled; the processing rate is the mix rate */
bool
InitHardwareEngine( AUDIO::CAudioEngine& engine, const CORE::CString& playId, const CORE::CString& captureId, UInt32 realtimePriority )
{
    AUDIO::CAudioEngineParams params;
    params.SetDriverName( "alsa" );
    params.SetThreadingMode( AUDIO::AUDIOTHREADING_THREADED );
    params.ApplyLatencyProfile( AUDIO::AUDIOLATENCYPROFILE_LOW, 48000 );
    params.GetDeviceParams().SetDeviceId( playId );
    params.GetCaptureDeviceParams().SetDeviceId( captureId );
    params.GetDeviceParams().GetOptions().Delete( AUDIO::CAudioPushDevice::RealtimePriorityOptionName );
    params.GetDeviceParams().GetOptions().Set( AUDIO::CAudioPushDevice::RealtimePriorityOptionName, CORE::UInt32ToString( realtimePriority ) );
    params.GetCaptureDeviceParams().GetOptions().Delete( AUDIO::CAudioPushDevice::RealtimePriorityOptionName );
    params.GetCaptureDeviceParams().GetOptions().Set( AUDIO::CAudioPushDevice::RealtimePriorityOptionName, CORE::UInt32ToString( realtimePriority ) );
    params.SetCaptureEnabled( true );
    if ( !engine.Initialize( params ) || !engine.IsCaptureAvailable() )
        return false;
    return engine.GetDeviceFormat().GetSampleRate() == engine.GetCaptureProcessingFormat().GetSampleRate();
}

void
ReportEngineDevices( AUDIO::CAudioEngine& engine )
{
    AUDIO::TAudioDevicePtr play = engine.GetDevice();
    AUDIO::TAudioDevicePtr capture = engine.GetCaptureDevice();
    AUDIO::CAudioPushDevice* playPush = play->AsPushDevice();
    AUDIO::CAudioPushDevice* capPush = capture->AsPushDevice();
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "IntercomHardware:   playback " << play->GetFormat().GetSampleRate() << " Hz, " << play->GetFormat().GetChannelCount() << " channels, period " << play->GetPeriodSizeInFrames() <<
                                      " frames, latency " << play->GetLatencyInFrames() << " frames, mix latency " << engine.GetMixLatencyInFrames() << " frames, real-time scheduling " <<
                                      ( GUCEF_NULL != playPush && playPush->IsRealtimeSchedulingActive() ? "active" : "not active" ) );
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "IntercomHardware:   capture period " << capture->GetPeriodSizeInFrames() << " frames, latency " << capture->GetLatencyInFrames() << " frames, real-time scheduling " <<
                                      ( GUCEF_NULL != capPush && capPush->IsRealtimeSchedulingActive() ? "active" : "not active" ) );
}

/** The latency the engine reports from the click handed to the playback device until the block of the capture device is delivered, in ms */
Float64
ReportedLatencyMs( AUDIO::CAudioEngine& engine )
{
    const UInt32 rate = engine.GetDeviceFormat().GetSampleRate();
    const UInt32 frames = engine.GetDevice()->GetLatencyInFrames() + engine.GetMixLatencyInFrames() + engine.GetCaptureDevice()->GetLatencyInFrames();
    return 1000.0 * frames / rate;
}

bool
WaitSeconds( UInt32 seconds )
{
    MT::PrecisionDelay( seconds * 1000 );
    return true;
}

/** Picks the devices from the environment; false (with the reason in the log) when one is missing */
bool
GetDeviceNames( CORE::CString& playId, CORE::CString& captureId, bool needCapture = true )
{
    playId = CORE::CString( GUCEF_TESTFW_GETENV( "GUCEF_TEST_AUDIO_DEVICE" ).c_str() );
    captureId = CORE::CString( GUCEF_TESTFW_GETENV( "GUCEF_TEST_AUDIO_CAPTURE_DEVICE" ).c_str() );
    return !playId.IsNULLOrEmpty() && ( !needCapture || !captureId.IsNULLOrEmpty() );
}

}

#endif /* GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformAudioIntercomHardwareTests( void )
{GUCEF_TRACE;

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING intercom hardware TESTS (opt in, GUCEF_TEST_AUDIO_INTERCOM=1)" );

    GUCEF_TESTFW_SUITE_SCOPE( "gucefAUDIO.IntercomHardware" );

    // item 1 ---------------------------------------------------------------------------------------------------------------------------------

    GUCEF_TESTFW_TESTCASE( "IntercomHardware: Item 1, capture and playback at 240-frame periods with 2 and 3 periods, with and without real-time scheduling (opt in)" )
        SKIP_UNLESS_ITEM( 1 );
        CORE::CString playId, captureId;
        GUCEF_TESTFW_SKIP_IF( !GetDeviceNames( playId, captureId ), "GUCEF_TEST_AUDIO_DEVICE and GUCEF_TEST_AUDIO_CAPTURE_DEVICE must name the devices" );
        AUDIO::CAudioDriverRegistry::TAudioDriverPtr driver;
        GUCEF_TESTFW_SKIP_IF( !LoadAlsaDriver( driver ), "the ALSA driver is not available" );
        try
        {
            const Float64 minutes = EnvDecibels( "GUCEF_TEST_AUDIO_INTERCOM_MINUTES", 1.0 );     // fractions are allowed
            const UInt32 loadThreads = EnvNumber( "GUCEF_TEST_AUDIO_INTERCOM_LOAD", 0 );
            const UInt32 priority = EnvNumber( "GUCEF_TEST_AUDIO_INTERCOM_RT_PRIORITY", MT::CRealtimeSchedulingScope::DefaultPriority );
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "INTERCOM HARDWARE TEST: item 1 opens the playback device \"" << playId << "\" (plays silence only) and the capture device \"" << captureId <<
                                              "\" (the microphone is opened and its audio is discarded) in four runs of " << Round( minutes ) << " min each (2 and 3 periods, without and with real-time priority " << priority <<
                                              "), with " << loadThreads << " CPU load threads. About " << Round( 4 * minutes ) << " min in total." );
            CLoadGenerator load( loadThreads );
            for ( UInt32 run=0; run<4; ++run )
            {
                const UInt32 periods = ( run & 1 ) ? 3 : 2;
                const UInt32 realtimePriority = ( run & 2 ) ? priority : 0;
                SDeviceRunResult result;
                CIntervalRecorder playTiming, capTiming;
                ASSERT_TRUE( RunDevicePair( driver, playId, captureId, periods, realtimePriority, (UInt32) ( minutes * 60.0 + 0.5 ), result, playTiming, capTiming ) );
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "IntercomHardware: item 1, " << periods << " periods requested, real-time priority requested " << realtimePriority << ": " <<
                                                  "device rate " << result.sampleRate << " Hz, playback period " << result.playPeriod << " frames and latency " << result.playLatency << " frames, capture period " << result.capPeriod <<
                                                  " frames and latency " << result.capLatency << " frames; playback underruns " << result.playUnderruns << ", capture overruns " << result.capOverruns <<
                                                  ", devices lost " << result.playLost + result.capLost << "; frames played " << result.playFrames << ", captured " << result.capFrames );
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "IntercomHardware:   real-time scheduling: playback thread " << ( result.playRealtime ? "granted" : "not granted" ) << " (error " << result.playRealtimeError << "), capture thread " <<
                                                  ( result.capRealtime ? "granted" : "not granted" ) << " (error " << result.capRealtimeError << ")" );
                ReportTiming( "playback", playTiming );
                ReportTiming( "capture", capTiming );
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // item 2 ---------------------------------------------------------------------------------------------------------------------------------

    GUCEF_TESTFW_TESTCASE( "IntercomHardware: Item 2, round trip latency of a click from the playback to the capture by cross-correlation (opt in)" )
        SKIP_UNLESS_ITEM( 2 );
        CORE::CString playId, captureId;
        GUCEF_TESTFW_SKIP_IF( !GetDeviceNames( playId, captureId ), "GUCEF_TEST_AUDIO_DEVICE and GUCEF_TEST_AUDIO_CAPTURE_DEVICE must name the devices" );
        AUDIO::CAudioDriverRegistry::TAudioDriverPtr driver;
        GUCEF_TESTFW_SKIP_IF( !LoadAlsaDriver( driver ), "the ALSA driver is not available" );
        try
        {
            const UInt32 seconds = EnvNumber( "GUCEF_TEST_AUDIO_INTERCOM_SECONDS", 10 );
            const Float64 levelDb = EnvDecibels( "GUCEF_TEST_AUDIO_INTERCOM_LEVEL_DB", -20.0 );
            const UInt32 priority = EnvNumber( "GUCEF_TEST_AUDIO_INTERCOM_RT_PRIORITY", MT::CRealtimeSchedulingScope::DefaultPriority );
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "INTERCOM HARDWARE TEST: item 2 plays a short click (2 ms, peak " << Round( levelDb ) << " dBFS) twice a second for " << seconds << " s on \"" << playId <<
                                              "\" and records \"" << captureId << "\" (the microphone is opened). The line out must be connected to the line in by a cable, or the speaker must face the microphone." );
            AUDIO::CAudioEngine engine;
            ASSERT_TRUE( InitHardwareEngine( engine, playId, captureId, priority ) );
            const UInt32 rate = engine.GetDeviceFormat().GetSampleRate();
            ReportEngineDevices( engine );

            CCaptureRecorder recorder( seconds, rate );
            CStimulusSource* clicks = GUCEF_NEW CStimulusSource( rate, CStimulusSource::MODE_CLICKS, levelDb );
            AUDIO::TAudioSourcePtr clickPtr( clicks );
            ASSERT_FALSE( engine.AddCaptureConsumer( &recorder ).HasNoFuture() );
            AUDIO::TAudioVoicePtr voice = engine.CreateVoice( clickPtr );
            ASSERT_FALSE( voice.IsNULL() );
            ASSERT_FALSE( engine.Start( voice ).HasNoFuture() );
            WaitSeconds( seconds );
            CORE::CFutureResult removed = engine.RemoveCaptureConsumer( &recorder );
            engine.WaitFor( removed, 2000 );
            const UInt64 overruns = engine.GetCaptureOverrunCount();
            const Float64 reported = ReportedLatencyMs( engine );
            engine.Shutdown();

            GUCEF::vector< SDetection > found;
            FindClicks( recorder, clicks->GetSignal().GetClick(), VoiceLinkTest::CPcmClickSignal::ClickLength, 0.6, rate / 100, found );
            TTimes roundTrips, detectionTimes;
            MatchRoundTrips( *clicks, found, roundTrips, detectionTimes );
            const SSummary summary = Summarize( roundTrips );
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "IntercomHardware: item 2, " << clicks->GetClickCount() << " clicks played, " << found.size() << " found in the recording, " << summary.count << " matched; capture overruns " << overruns <<
                                              ", blocks with a discontinuity " << recorder.GetDiscontinuityCount() );
            if ( 0 == summary.count )
            {
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "IntercomHardware: item 2, no click came back: check the cable, the input selection and GUCEF_TEST_AUDIO_INTERCOM_LEVEL_DB (a null device records silence)" );
            }
            else
            {
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "IntercomHardware: item 2, round trip from the click handed to the playback device to its capture: mean " << Round( summary.mean ) << " ms, smallest " << Round( summary.minimum ) <<
                                                  " ms, largest " << Round( summary.maximum ) << " ms, deviation " << Round( summary.deviation ) << " ms; the device latencies add up to " << Round( reported ) <<
                                                  " ms (playback latency, mix latency, capture block); the rest, " << Round( summary.mean - reported ) << " ms, is the loop outside the engine (converters, acoustic path)" );
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // item 3 ---------------------------------------------------------------------------------------------------------------------------------

    GUCEF_TESTFW_TESTCASE( "IntercomHardware: Item 3, local loopback intercom: capture, sender, simulated link, receiver and playback; listening stages and, with a cable, the end to end latency (opt in)" )
        SKIP_UNLESS_ITEM( 3 );
        CORE::CString playId, captureId;
        GUCEF_TESTFW_SKIP_IF( !GetDeviceNames( playId, captureId ), "GUCEF_TEST_AUDIO_DEVICE and GUCEF_TEST_AUDIO_CAPTURE_DEVICE must name the devices" );
        AUDIO::CAudioDriverRegistry::TAudioDriverPtr driver;
        GUCEF_TESTFW_SKIP_IF( !LoadAlsaDriver( driver ), "the ALSA driver is not available" );
        const bool opus = VoiceCodecTest::LoadVoiceCodecPlugin( "audiovoicecodecOPUS", "audiointercomhardwareopus", "opus" );
        try
        {
            const UInt32 seconds = EnvNumber( "GUCEF_TEST_AUDIO_INTERCOM_SECONDS", 10 );
            const Float64 levelDb = EnvDecibels( "GUCEF_TEST_AUDIO_INTERCOM_LEVEL_DB", -20.0 );
            const UInt32 priority = EnvNumber( "GUCEF_TEST_AUDIO_INTERCOM_RT_PRIORITY", MT::CRealtimeSchedulingScope::DefaultPriority );
            const bool cable = GUCEF_TESTFW_GETENV( "GUCEF_TEST_AUDIO_INTERCOM_CABLE" ) == "1";

            AUDIO::CVoiceCodecConfig opusLowDelay = VoiceCodecTest::MakeConfig( "opus", 48000, 10000 );
            opusLowDelay.SetProfileDefaults( AUDIO::VOICECODECPROFILE_LOWDELAY );
            AUDIO::CVoiceCodecConfig opusRobust = VoiceCodecTest::MakeConfig( "opus", 48000, 10000 );
            opusRobust.SetProfileDefaults( AUDIO::VOICECODECPROFILE_ROBUST );
            AUDIO::CVoiceCodecConfig codecs[ 3 ] = { VoiceStreamTest::PcmConfig( 48000, 10000 ), opusLowDelay, opusRobust };
            const char* names[ 3 ] = { "pcm", "opus low delay", "opus robust" };

            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "INTERCOM HARDWARE TEST: item 3 carries the microphone of \"" << captureId << "\" through the sender, a simulated wired link and the receiver to the playback \"" << playId <<
                                              "\" for " << seconds << " s per codec (pcm" << ( opus ? ", opus low delay, opus robust" : "" ) << "). WEAR HEADPHONES and keep the microphone away from them: an open speaker and microphone howl. Speak into the microphone during each stage." );
            if ( cable )
            {
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "INTERCOM HARDWARE TEST: item 3 then plays clicks (peak " << Round( levelDb ) << " dBFS) on the line out for " << seconds << " s per codec to measure the end to end latency through the cable to the line in. The loop gain stays below one by the receiver volume." );
            }
            else
            {
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "IntercomHardware: item 3, GUCEF_TEST_AUDIO_INTERCOM_CABLE is not 1: the end to end latency stage with clicks is not run" );
            }

            // stage 1: the listening stages; stage 2: the latency with clicks, the cable closes the loop
            for ( UInt32 stage=0; stage<( cable ? 2 : 1 ); ++stage )
            {
                for ( UInt32 c=0; c<( opus ? 3U : 1U ); ++c )
                {
                    AUDIO::CAudioEngine engine;
                    ASSERT_TRUE( InitHardwareEngine( engine, playId, captureId, priority ) );
                    const UInt32 rate = engine.GetDeviceFormat().GetSampleRate();

                    AUDIO::CVoiceSenderConfig senderConfig;
                    senderConfig.SetCodecConfig( codecs[ c ] );
                    senderConfig.SetStreamId( 0x0A0A0A0A );
                    senderConfig.SetFirstSequenceNumber( 100 );
                    senderConfig.SetFirstTimestamp( 0 );
                    senderConfig.SetTransmitMode( AUDIO::VOICETRANSMITMODE_CONTINUOUS );
                    senderConfig.SetOutboundQueueFrames( 64 );
                    AUDIO::CVoiceSender sender( senderConfig );
                    AUDIO::CVoiceReceiverConfig receiverConfig;
                    receiverConfig.SetCodecConfig( codecs[ c ] );
                    receiverConfig.SetPlayoutProfile( AUDIO::VOICEPLAYOUTPROFILE_WIRED );
                    receiverConfig.SetInboundQueueFrames( 64 );
                    AUDIO::CVoiceReceiver receiver( engine, receiverConfig );
                    receiver.SetVolume( 0 == stage ? 1.0f : 0.5f );
                    ASSERT_TRUE( receiver.Prepare() );

                    CCaptureRecorder recorder( seconds, rate );
                    CStimulusSource* clicks = GUCEF_NEW CStimulusSource( rate, 0 == stage ? CStimulusSource::MODE_SILENCE : CStimulusSource::MODE_CLICKS, levelDb );
                    AUDIO::TAudioSourcePtr clickPtr( clicks );
                    ASSERT_FALSE( engine.AddCaptureConsumer( &recorder ).HasNoFuture() );
                    CORE::CFutureResult senderAdded = engine.AddCaptureConsumer( &sender );
                    ASSERT_FALSE( senderAdded.HasNoFuture() );
                    ASSERT_FALSE( receiver.Start().HasNoFuture() );
                    AUDIO::TAudioVoicePtr clickVoice = engine.CreateVoice( clickPtr );
                    ASSERT_FALSE( clickVoice.IsNULL() );
                    if ( 1 == stage )
                        ASSERT_FALSE( engine.Start( clickVoice ).HasNoFuture() );
                    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "IntercomHardware: item 3, " << ( 0 == stage ? "listening stage, speak now" : "latency stage, clicks" ) << ", codec " << names[ c ] << ", " << seconds << " s" );
                    ReportEngineDevices( engine );

                    VoiceLinkTest::CTestSimulatedVoiceLink link( VoiceLinkTest::SLinkModel::Wired() );
                    link.SetRedPayloadType( receiverConfig.GetRedPayloadType() );
                    AUDIO::CVoiceFrame frame;
                    const UInt64 start = MT::PrecisionTickCount();
                    while ( TickToMs( MT::PrecisionTickCount() - start ) < 1000.0 * seconds )
                    {
                        const Float64 nowUs = (Float64) AUDIO::CVoiceClock::GetSystemTimeInUs();
                        while ( sender.PopFrame( frame ) )
                            link.Send( frame, nowUs );
                        link.Deliver( nowUs, receiver );
                        MT::PrecisionDelay( 1 );
                    }
                    AUDIO::CVoiceReceiverStats receiverStats;
                    receiver.GetStats( receiverStats );
                    AUDIO::CVoiceSenderStats senderStats;
                    sender.GetStats( senderStats );
                    CORE::CFutureResult senderRemoved = engine.RemoveCaptureConsumer( &sender );
                    CORE::CFutureResult recorderRemoved = engine.RemoveCaptureConsumer( &recorder );
                    engine.WaitFor( senderRemoved, 2000 );
                    engine.WaitFor( recorderRemoved, 2000 );
                    receiver.Stop();
                    const UInt64 overruns = engine.GetCaptureOverrunCount();
                    AUDIO::CAudioPushDevice* playPush = engine.GetDevice()->AsPushDevice();
                    const UInt64 underruns = GUCEF_NULL != playPush ? playPush->GetUnderrunCount() : 0;
                    engine.Shutdown();

                    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "IntercomHardware: item 3, " << names[ c ] << ": frames sent " << senderStats.framesSent << ", received " << receiverStats.framesReceived << ", decoded " << receiverStats.framesDecoded <<
                                                      ", concealed " << receiverStats.framesConcealed << ", late " << receiverStats.lateFrames << ", underruns of the playout " << receiverStats.underruns << "; playout margin " << Round( receiverStats.marginInMs ) <<
                                                      " ms, buffered " << Round( receiverStats.levelInMs ) << " ms, clock ratio " << Round( receiverStats.ratioInPpm ) << " ppm; capture overruns " << overruns << ", playback underruns " << underruns <<
                                                      ", mean encode " << Round( senderStats.meanEncodeTimeInUs ) << " us" );

                    if ( 1 == stage )
                    {
                        // direct clicks come back after the round trip R; the click the intercom made of them comes back one end to end delay later
                        GUCEF::vector< SDetection > found;
                        FindClicks( recorder, clicks->GetSignal().GetClick(), VoiceLinkTest::CPcmClickSignal::ClickLength, 0.35, rate / 200, found );
                        TTimes roundTrips, firstTimes, delays;
                        MatchRoundTrips( *clicks, found, roundTrips, firstTimes );
                        const SSummary direct = Summarize( roundTrips );
                        for ( size_t k=0; k<firstTimes.size(); ++k )
                            for ( size_t d=0; d<found.size(); ++d )
                                if ( found[ d ].timeMs > firstTimes[ k ] + 3.0 && found[ d ].timeMs - firstTimes[ k ] < 400.0 )
                                {
                                    delays.push_back( found[ d ].timeMs - firstTimes[ k ] - direct.mean );
                                    break;
                                }
                        const SSummary endToEnd = Summarize( delays );
                        if ( 0 == direct.count )
                        {
                            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "IntercomHardware: item 3, " << names[ c ] << ": no click came back through the cable, no latency to report" );
                        }
                        else if ( 0 == endToEnd.count )
                        {
                            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "IntercomHardware: item 3, " << names[ c ] << ": " << direct.count << " direct clicks came back (round trip " << Round( direct.mean ) << " ms) but not their intercom copies: raise GUCEF_TEST_AUDIO_INTERCOM_LEVEL_DB" );
                        }
                        else
                        {
                            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "IntercomHardware: item 3, " << names[ c ] << ": round trip of the cable " << Round( direct.mean ) << " ms; end to end delay of the intercom from capture to the playback device " <<
                                                              Round( endToEnd.mean ) << " ms (smallest " << Round( endToEnd.minimum ) << ", largest " << Round( endToEnd.maximum ) << ", deviation " << Round( endToEnd.deviation ) << ", " << endToEnd.count << " clicks)" );
                        }
                    }
                }
            }
            if ( opus )
                CORE::CCoreGlobal::Instance()->GetPluginControl().UnloadPluginGroup( "audiointercomhardwareopus" );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // item 4 ---------------------------------------------------------------------------------------------------------------------------------

    GUCEF_TESTFW_TESTCASE( "IntercomHardware: Item 4, acoustic coupling of the speaker to the microphone and half-duplex switching (opt in)" )
        SKIP_UNLESS_ITEM( 4 );
        CORE::CString playId, captureId;
        GUCEF_TESTFW_SKIP_IF( !GetDeviceNames( playId, captureId ), "GUCEF_TEST_AUDIO_DEVICE and GUCEF_TEST_AUDIO_CAPTURE_DEVICE must name the devices" );
        AUDIO::CAudioDriverRegistry::TAudioDriverPtr driver;
        GUCEF_TESTFW_SKIP_IF( !LoadAlsaDriver( driver ), "the ALSA driver is not available" );
        try
        {
            const UInt32 seconds = EnvNumber( "GUCEF_TEST_AUDIO_INTERCOM_SECONDS", 10 );
            const Float64 levelDb = EnvDecibels( "GUCEF_TEST_AUDIO_INTERCOM_LEVEL_DB", -20.0 );
            const UInt32 priority = EnvNumber( "GUCEF_TEST_AUDIO_INTERCOM_RT_PRIORITY", MT::CRealtimeSchedulingScope::DefaultPriority );
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "INTERCOM HARDWARE TEST: item 4 on the unit's own speaker \"" << playId << "\" and microphone \"" << captureId << "\" (the microphone is opened): 3 s of silence, then 4 s of noise at " <<
                                              Round( levelDb ) << " dBFS RMS from the speaker (the coupling), then " << seconds << " s of the same noise during which you should speak into the microphone now and then, to see the half-duplex controller break in. Keep the room quiet during the first stages." );
            AUDIO::CAudioEngine engine;
            ASSERT_TRUE( InitHardwareEngine( engine, playId, captureId, priority ) );
            const UInt32 rate = engine.GetDeviceFormat().GetSampleRate();
            ReportEngineDevices( engine );

            AUDIO::CHalfDuplexController controller;
            CCaptureRecorder recorder( 3 + 4 + seconds, rate );
            CStimulusSource* noise = GUCEF_NEW CStimulusSource( rate, CStimulusSource::MODE_NOISE, levelDb );
            AUDIO::TAudioSourcePtr noisePtr( noise );
            ASSERT_FALSE( engine.AddCaptureConsumer( &recorder ).HasNoFuture() );
            CORE::CFutureResult controllerSet = engine.SetCaptureHalfDuplexController( &controller );
            ASSERT_FALSE( controllerSet.HasNoFuture() );
            ASSERT_TRUE( engine.WaitFor( controllerSet, 2000 ) );

            WaitSeconds( 3 );
            const UInt64 silenceEnd = recorder.GetSampleCount();
            AUDIO::TAudioVoicePtr voice = engine.CreateVoice( noisePtr );
            ASSERT_FALSE( voice.IsNULL() );
            ASSERT_FALSE( engine.Start( voice ).HasNoFuture() );
            WaitSeconds( 4 );
            const UInt64 couplingEnd = recorder.GetSampleCount();
            const Float32 controllerCoupling = controller.GetCouplingInDecibels();
            const Float64 floorDb = ToDecibels( recorder.RmsOfRange( rate / 2, silenceEnd ) );
            const Float64 heardDb = ToDecibels( recorder.RmsOfRange( silenceEnd + 2 * (UInt64) rate, couplingEnd ) );
            const Float64 playedDb = ToDecibels( noise->GetNoiseRms() );
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "IntercomHardware: item 4, capture noise floor " << Round( floorDb ) << " dBFS RMS, noise played " << Round( playedDb ) << " dBFS RMS, heard on the microphone " << Round( heardDb ) <<
                                              " dBFS RMS: coupling " << Round( heardDb - playedDb ) << " dB; the controller's own estimate " << Round( controllerCoupling ) << " dB, its state " << (UInt32) controller.GetState() <<
                                              " (0 idle, 1 far end, 2 near end)" );

            // the break-in stage: the state of the controller is logged when it changes
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "INTERCOM HARDWARE TEST: item 4, speak into the microphone now (" << seconds << " s)" );
            const UInt64 stageStart = MT::PrecisionTickCount();
            AUDIO::THalfDuplexState lastState = controller.GetState();
            Float64 inState[ 3 ] = { 0.0, 0.0, 0.0 };
            Float64 lastChangeMs = 0.0;
            while ( TickToMs( MT::PrecisionTickCount() - stageStart ) < 1000.0 * seconds )
            {
                MT::PrecisionDelay( 10 );
                const AUDIO::THalfDuplexState state = controller.GetState();
                if ( state != lastState )
                {
                    const Float64 nowMs = TickToMs( MT::PrecisionTickCount() - stageStart );
                    inState[ (UInt32) lastState ] += nowMs - lastChangeMs;
                    lastChangeMs = nowMs;
                    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "IntercomHardware: item 4, " << Round( nowMs ) << " ms: state " << (UInt32) lastState << " to " << (UInt32) state << ", near level " << Round( controller.GetNearLevelInDecibels() ) <<
                                                      " dB, echo estimate " << Round( controller.GetEchoEstimateInDecibels() ) << " dB, coupling " << Round( controller.GetCouplingInDecibels() ) << " dB, microphone gain " << Round( ToDecibels( controller.GetMicGain() ) ) << " dB" );
                    lastState = state;
                }
            }
            inState[ (UInt32) lastState ] += TickToMs( MT::PrecisionTickCount() - stageStart ) - lastChangeMs;
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "IntercomHardware: item 4, time in idle " << Round( inState[ 0 ] ) << " ms, far end " << Round( inState[ 1 ] ) << " ms, near end " << Round( inState[ 2 ] ) <<
                                              " ms, transitions " << controller.GetTransitionCount() << ", capture overruns " << engine.GetCaptureOverrunCount() );

            CORE::CFutureResult removed = engine.RemoveCaptureConsumer( &recorder );
            engine.WaitFor( removed, 2000 );
            engine.SetCaptureHalfDuplexController( GUCEF_NULL );
            engine.Shutdown();
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // item 5 ---------------------------------------------------------------------------------------------------------------------------------

    GUCEF_TESTFW_TESTCASE( "IntercomHardware: Item 5, the echo canceller on the unit's own speaker and microphone: convergence, ERLE, bulk delay, drift and double talk (opt in)" )
        SKIP_UNLESS_ITEM( 5 );
        CORE::CString playId, captureId;
        GUCEF_TESTFW_SKIP_IF( !GetDeviceNames( playId, captureId ), "GUCEF_TEST_AUDIO_DEVICE and GUCEF_TEST_AUDIO_CAPTURE_DEVICE must name the devices" );
        AUDIO::CAudioDriverRegistry::TAudioDriverPtr driver;
        GUCEF_TESTFW_SKIP_IF( !LoadAlsaDriver( driver ), "the ALSA driver is not available" );
        try
        {
            const UInt32 seconds = EnvNumber( "GUCEF_TEST_AUDIO_INTERCOM_SECONDS", 10 );
            const Float64 levelDb = EnvDecibels( "GUCEF_TEST_AUDIO_INTERCOM_LEVEL_DB", -20.0 );
            const UInt32 priority = EnvNumber( "GUCEF_TEST_AUDIO_INTERCOM_RT_PRIORITY", MT::CRealtimeSchedulingScope::DefaultPriority );
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "INTERCOM HARDWARE TEST: item 5 on the unit's own speaker \"" << playId << "\" and microphone \"" << captureId << "\" (the microphone is opened), processing at 16 kHz: 3 s of " <<
                                              "silence, then " << seconds << " s of noise at " << Round( levelDb ) << " dBFS RMS from the speaker with the echo canceller learning, 4 s more with the canceller switched off (for the comparison), then " <<
                                              seconds << " s of the noise during which you should speak into the microphone (double talk). Keep the room quiet during the first stages." );
            AUDIO::CAudioEngineParams params;
            params.SetDriverName( "alsa" );
            params.SetThreadingMode( AUDIO::AUDIOTHREADING_THREADED );
            params.ApplyLatencyProfile( AUDIO::AUDIOLATENCYPROFILE_LOW, 48000 );
            params.GetDeviceParams().SetDeviceId( playId );
            params.GetCaptureDeviceParams().SetDeviceId( captureId );
            params.GetDeviceParams().GetOptions().Delete( AUDIO::CAudioPushDevice::RealtimePriorityOptionName );
            params.GetDeviceParams().GetOptions().Set( AUDIO::CAudioPushDevice::RealtimePriorityOptionName, CORE::UInt32ToString( priority ) );
            params.GetCaptureDeviceParams().GetOptions().Delete( AUDIO::CAudioPushDevice::RealtimePriorityOptionName );
            params.GetCaptureDeviceParams().GetOptions().Set( AUDIO::CAudioPushDevice::RealtimePriorityOptionName, CORE::UInt32ToString( priority ) );
            params.SetCaptureEnabled( true );
            params.SetCaptureProcessingRate( 16000 );
            AUDIO::CAudioEngine engine;
            ASSERT_TRUE( engine.Initialize( params ) && engine.IsCaptureAvailable() );
            const UInt32 rate = engine.GetDeviceFormat().GetSampleRate();
            ReportEngineDevices( engine );

            AUDIO::CAcousticEchoCanceller canceller;
            CCaptureRecorder recorder( 3 + 2 * seconds + 4, 16000 );
            CStimulusSource* noise = GUCEF_NEW CStimulusSource( rate, CStimulusSource::MODE_NOISE, levelDb );
            AUDIO::TAudioSourcePtr noisePtr( noise );
            ASSERT_FALSE( engine.AddCaptureConsumer( &recorder ).HasNoFuture() );
            CORE::CFutureResult set = engine.SetEchoCanceller( &canceller );
            ASSERT_FALSE( set.HasNoFuture() );
            ASSERT_TRUE( engine.WaitFor( set, 2000 ) );

            WaitSeconds( 3 );
            const UInt64 silenceEnd = recorder.GetSampleCount();
            AUDIO::TAudioVoicePtr voice = engine.CreateVoice( noisePtr );
            ASSERT_FALSE( voice.IsNULL() );
            ASSERT_FALSE( engine.Start( voice ).HasNoFuture() );
            for ( UInt32 s=0; s<seconds; ++s )
            {
                WaitSeconds( 1 );
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "IntercomHardware: item 5, " << s + 1 << " s: state " << (UInt32) canceller.GetState() << " (2 idle, 3 converging, 4 converged), ERLE estimate " <<
                                                  Round( canceller.GetErleInDecibels() ) << " dB, residual suppressor " << Round( canceller.GetResidualSuppressionInDecibels() ) << " dB, bulk delay " <<
                                                  canceller.GetBulkDelayInFrames() / 16 << " ms (estimate " << canceller.GetEstimatedDelayInFrames() / 16 << " ms), reference level " <<
                                                  canceller.GetReferenceLevelInFrames() << " frames, drift " << Round( canceller.GetDriftInPpm() ) << " ppm, underruns " << canceller.GetReferenceUnderrunCount() <<
                                                  ", alignments " << canceller.GetResyncCount() );
            }
            const UInt64 learnedEnd = recorder.GetSampleCount();
            canceller.SetEnabled( false );
            WaitSeconds( 4 );
            const UInt64 offEnd = recorder.GetSampleCount();
            canceller.SetEnabled( true );
            const Float64 floorDb = ToDecibels( recorder.RmsOfRange( 8000, silenceEnd ) );
            const Float64 withDb = ToDecibels( recorder.RmsOfRange( learnedEnd - 3 * 16000, learnedEnd ) );
            const Float64 withoutDb = ToDecibels( recorder.RmsOfRange( learnedEnd + 16000, offEnd ) );
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "IntercomHardware: item 5, microphone noise floor " << Round( floorDb ) << " dBFS, the echo of the noise " << Round( withoutDb ) << " dBFS without the canceller and " <<
                                              Round( withDb ) << " dBFS with it: " << Round( withoutDb - withDb ) << " dB suppressed in all (limited by the noise floor); FFTs " << canceller.GetTransformCount() <<
                                              ", capture overruns " << engine.GetCaptureOverrunCount() );

            // double talk: the state of the canceller is logged when it changes
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "INTERCOM HARDWARE TEST: item 5, speak into the microphone now (" << seconds << " s)" );
            const UInt64 stageStart = MT::PrecisionTickCount();
            bool lastDoubleTalk = false;
            UInt32 changes = 0;
            while ( TickToMs( MT::PrecisionTickCount() - stageStart ) < 1000.0 * seconds )
            {
                MT::PrecisionDelay( 10 );
                const bool doubleTalk = canceller.IsDoubleTalk();
                if ( doubleTalk != lastDoubleTalk )
                {
                    ++changes;
                    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "IntercomHardware: item 5, " << Round( TickToMs( MT::PrecisionTickCount() - stageStart ) ) << " ms: double talk " << ( doubleTalk ? "begins" : "ends" ) <<
                                                      ", ERLE estimate " << Round( canceller.GetErleInDecibels() ) << " dB, state " << (UInt32) canceller.GetState() );
                    lastDoubleTalk = doubleTalk;
                }
            }
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "IntercomHardware: item 5, double talk changes " << changes << ", divergences " << canceller.GetDivergenceCount() << ", ERLE estimate at the end " <<
                                              Round( canceller.GetErleInDecibels() ) << " dB, converged " << canceller.IsConverged() );

            CORE::CFutureResult removed = engine.RemoveCaptureConsumer( &recorder );
            engine.WaitFor( removed, 2000 );
            engine.WaitFor( engine.SetEchoCanceller( GUCEF_NULL ), 2000 );
            engine.Shutdown();
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    #else

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Intercom hardware tests are only built for Linux (ALSA)" );

    #endif /* GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ? */
}

/*-------------------------------------------------------------------------*/
