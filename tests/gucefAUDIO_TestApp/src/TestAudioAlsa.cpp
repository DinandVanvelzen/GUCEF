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

#if defined( __linux__ )
#include <dlfcn.h>
#include <errno.h>
#include <stdlib.h>
#endif

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

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )
// the table of libasound functions of the driver, for the fake the tests put in its place (the plugin itself is not linked)
#include "../../../plugins/AUDIO/audiodriverALSA/include/audiodriverALSA_CAlsaApi.h"
#endif

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


/** Records what a capture device delivers: blocks, sizes, the overrun count seen with each block, and the continuity of
 *  a counting signal (16 bit mono, every sample one more than the one before) */
class CCaptureProbe : public AUDIO::IAudioDeviceCallback
{
    public:

    CCaptureProbe( void )
        : blocks( 0 ), frames( 0 ), badBlockSizes( 0 ), jumps( 0 ), lost( 0 ), firstSample( -1 ), blockOfFirstJump( 0 ), blockOfFirstOverrun( 0 )
        , expectedBlock( 0 ), device( GUCEF_NULL ), haveLast( false ), last( 0 ), seenOverruns( 0 ), distinct( false ), firstByte( 0 ), haveFirstByte( false ) {}

    virtual void OnRenderAudio( void*, UInt32 ) GUCEF_VIRTUAL_OVERRIDE {}

    virtual void OnCaptureAudio( const void* buffer, UInt32 frameCount ) GUCEF_VIRTUAL_OVERRIDE
    {
        const UInt32 block = blocks.FetchAdd( 1 ) + 1;
        frames.FetchAdd( frameCount );
        if ( 0 != expectedBlock && frameCount != expectedBlock )
            badBlockSizes.FetchAdd( 1 );

        if ( GUCEF_NULL != device )
        {
            const UInt64 overruns = device->GetOverrunCount();
            if ( overruns != seenOverruns )
            {
                seenOverruns = overruns;
                if ( 0 == blockOfFirstOverrun.Load() )
                    blockOfFirstOverrun.Store( block );
            }
        }

        const UInt8* bytes = static_cast< const UInt8* >( buffer );
        const UInt16* samples = static_cast< const UInt16* >( buffer );
        for ( UInt32 i=0; i<frameCount; ++i )
        {
            if ( !haveFirstByte )
            {
                firstByte = bytes[ 0 ];
                haveFirstByte = true;
            }
            distinct = distinct || bytes[ i ] != firstByte;
            if ( haveLast && samples[ i ] != (UInt16) ( last + 1 ) )
            {
                jumps.FetchAdd( 1 );
                if ( 0 == blockOfFirstJump.Load() )
                    blockOfFirstJump.Store( block );
            }
            if ( !haveLast )
                firstSample.Store( samples[ i ] );
            last = samples[ i ];
            haveLast = true;
        }
    }

    virtual void OnDeviceLost( void ) GUCEF_VIRTUAL_OVERRIDE { lost.FetchAdd( 1 ); }

    MT::CTAtomic< UInt32 > blocks;
    MT::CTAtomic< UInt64 > frames;
    MT::CTAtomic< UInt32 > badBlockSizes;
    MT::CTAtomic< UInt32 > jumps;
    MT::CTAtomic< UInt32 > lost;
    MT::CTAtomic< Int32 > firstSample;
    MT::CTAtomic< UInt32 > blockOfFirstJump;
    MT::CTAtomic< UInt32 > blockOfFirstOverrun;
    UInt32 expectedBlock;                               // set before Start()
    AUDIO::CAudioPushDevice* device;                    // set before Start()

    // device thread only
    bool haveLast;
    UInt16 last;
    UInt64 seenOverruns;
    bool distinct;
    UInt8 firstByte;
    bool haveFirstByte;
};

/** Renders a counting signal (16 bit mono), the same as CCaptureProbe checks */
class CRampRenderer : public AUDIO::IAudioDeviceCallback
{
    public:

    CRampRenderer( void ) : next( 0 ), lost( 0 ) {}
    virtual void OnRenderAudio( void* buffer, UInt32 frameCount ) GUCEF_VIRTUAL_OVERRIDE
    {
        UInt16* samples = static_cast< UInt16* >( buffer );
        for ( UInt32 i=0; i<frameCount; ++i )
            samples[ i ] = next++;
    }
    virtual void OnCaptureAudio( const void*, UInt32 ) GUCEF_VIRTUAL_OVERRIDE {}
    virtual void OnDeviceLost( void ) GUCEF_VIRTUAL_OVERRIDE { lost.FetchAdd( 1 ); }

    UInt16 next;
    MT::CTAtomic< UInt32 > lost;
};

/**
 *  A libasound that is not there: the functions of the table of the driver, faked. The tests reach the driver
 *  through the entry points that the plugin exports for this. Capture makes a counting signal at the pace of a
 *  48 kHz stream (or takes what playback wrote when 'loopback' is on), playback discards at that pace.
 *  Errors are scripted: the read or write after 'ErrorAfter' successful calls fails once. One instance, as the
 *  functions of the table have no context.
 */
struct SFakeAlsa
{
    SFakeAlsa( void ) { Reset(); }

    void Reset( void )
    {
        MT::CScopeMutex scope( lock );
        openCalls = 0; failOpen = false; lastStream = -1; lastName.Clear();
        memset( closes, 0, sizeof( closes ) ); memset( prepares, 0, sizeof( prepares ) ); memset( starts, 0, sizeof( starts ) );
        memset( drops, 0, sizeof( drops ) ); memset( recoverCalls, 0, sizeof( recoverCalls ) ); memset( lastRecoverError, 0, sizeof( lastRecoverError ) );
        memset( availMin, 0, sizeof( availMin ) ); memset( startThreshold, 0, sizeof( startThreshold ) ); memset( swParamsCalls, 0, sizeof( swParamsCalls ) );
        memset( reads, 0, sizeof( reads ) ); memset( writes, 0, sizeof( writes ) ); memset( framesWritten, 0, sizeof( framesWritten ) );
        forcedPeriod = 0; requestedPeriod = 0; requestedBuffer = 0; grantedPeriod = 0; grantedBuffer = 0;
        failSwParams = false; failStart = false; failRecover = false; failDelay = false; delay = 0;
        readErrorArmed = false; readError = 0; readErrorAfter = 0; readErrorLostFrames = 0;
        writeErrorArmed = false; writeError = 0; writeErrorAfter = 0;
        abortReads = false; loopback = false; chunk = 100; counter = 0; ringStart = 0; ringUsed = 0;
        hintCalls = 0;
    }

    static UInt32 Index( const AUDIODRIVER::ALSA::snd_pcm_t* pcm ) { return pcm == reinterpret_cast< const AUDIODRIVER::ALSA::snd_pcm_t* >( &instance.handles[ 1 ] ) ? 1 : 0; }

    template< typename T > T Read( const T& value ) const { MT::CScopeMutex scope( lock ); return value; }

    MT::CMutex lock;
    char handles[ 2 ];                                  // [0] playback, [1] capture
    char token;
    UInt32 openCalls; bool failOpen; int lastStream; CORE::CString lastName;
    UInt32 closes[ 2 ], prepares[ 2 ], starts[ 2 ], drops[ 2 ], recoverCalls[ 2 ];
    int lastRecoverError[ 2 ];
    UInt32 availMin[ 2 ], startThreshold[ 2 ], swParamsCalls[ 2 ];
    UInt32 reads[ 2 ], writes[ 2 ];
    UInt64 framesWritten[ 2 ];
    UInt32 forcedPeriod, requestedPeriod, requestedBuffer, grantedPeriod, grantedBuffer;
    bool failSwParams, failStart, failRecover, failDelay;
    Int32 delay;
    bool readErrorArmed; int readError; UInt32 readErrorAfter; UInt16 readErrorLostFrames;
    bool writeErrorArmed; int writeError; UInt32 writeErrorAfter;
    bool abortReads, loopback;
    UInt32 chunk;                                       // frames per read or write the fake hands out at most
    UInt16 counter;                                     // the counting signal
    enum { RingBytes = 1 << 20 };
    UInt8 ring[ RingBytes ];
    UInt32 ringStart, ringUsed;
    UInt32 hintCalls;

    static SFakeAlsa instance;
};

SFakeAlsa SFakeAlsa::instance;

int FakeOpen( AUDIODRIVER::ALSA::snd_pcm_t** pcm, const char* name, int stream, int )
{
    SFakeAlsa& f = SFakeAlsa::instance;
    MT::CScopeMutex scope( f.lock );
    ++f.openCalls; f.lastStream = stream; f.lastName = name;
    if ( f.failOpen )
        return -ENOENT;
    *pcm = reinterpret_cast< AUDIODRIVER::ALSA::snd_pcm_t* >( &f.handles[ stream ? 1 : 0 ] );
    return 0;
}

int FakeClose( AUDIODRIVER::ALSA::snd_pcm_t* pcm )
{
    SFakeAlsa& f = SFakeAlsa::instance;
    MT::CScopeMutex scope( f.lock );
    ++f.closes[ SFakeAlsa::Index( pcm ) ];
    return 0;
}

int FakeHwMalloc( AUDIODRIVER::ALSA::snd_pcm_hw_params_t** p ) { *p = reinterpret_cast< AUDIODRIVER::ALSA::snd_pcm_hw_params_t* >( &SFakeAlsa::instance.token ); return 0; }
void FakeHwFree( AUDIODRIVER::ALSA::snd_pcm_hw_params_t* ) {}
int FakeHwAny( AUDIODRIVER::ALSA::snd_pcm_t*, AUDIODRIVER::ALSA::snd_pcm_hw_params_t* ) { return 0; }
int FakeHwSetAccess( AUDIODRIVER::ALSA::snd_pcm_t*, AUDIODRIVER::ALSA::snd_pcm_hw_params_t*, int access ) { return 3 == access ? 0 : -EINVAL; }
// SND_PCM_FORMAT_S16_LE, or _BE on a big endian machine: the one format the fake has
const int FakeFormatS16 = 2 + ( AUDIO::CPcmStreamSource::IsHostBigEndian() ? 1 : 0 );

int FakeHwTestFormat( AUDIODRIVER::ALSA::snd_pcm_t*, AUDIODRIVER::ALSA::snd_pcm_hw_params_t*, int format ) { return FakeFormatS16 == format ? 0 : -EINVAL; }
int FakeHwSetFormat( AUDIODRIVER::ALSA::snd_pcm_t* pcm, AUDIODRIVER::ALSA::snd_pcm_hw_params_t* p, int format ) { return FakeHwTestFormat( pcm, p, format ); }
int FakeHwSetChannels( AUDIODRIVER::ALSA::snd_pcm_t*, AUDIODRIVER::ALSA::snd_pcm_hw_params_t*, unsigned int* ) { return 0; }
int FakeHwSetRate( AUDIODRIVER::ALSA::snd_pcm_t*, AUDIODRIVER::ALSA::snd_pcm_hw_params_t*, unsigned int*, int* ) { return 0; }

int FakeHwSetPeriod( AUDIODRIVER::ALSA::snd_pcm_t*, AUDIODRIVER::ALSA::snd_pcm_hw_params_t*, AUDIODRIVER::ALSA::snd_pcm_uframes_t* frames, int* )
{
    SFakeAlsa& f = SFakeAlsa::instance;
    MT::CScopeMutex scope( f.lock );
    f.requestedPeriod = (UInt32) *frames;
    if ( 0 != f.forcedPeriod )
        *frames = f.forcedPeriod;
    f.grantedPeriod = (UInt32) *frames;
    return 0;
}

int FakeHwSetBuffer( AUDIODRIVER::ALSA::snd_pcm_t*, AUDIODRIVER::ALSA::snd_pcm_hw_params_t*, AUDIODRIVER::ALSA::snd_pcm_uframes_t* frames )
{
    SFakeAlsa& f = SFakeAlsa::instance;
    MT::CScopeMutex scope( f.lock );
    f.requestedBuffer = f.grantedBuffer = (UInt32) *frames;
    return 0;
}

int FakeHwParams( AUDIODRIVER::ALSA::snd_pcm_t*, AUDIODRIVER::ALSA::snd_pcm_hw_params_t* ) { return 0; }

int FakeHwGetPeriod( const AUDIODRIVER::ALSA::snd_pcm_hw_params_t*, AUDIODRIVER::ALSA::snd_pcm_uframes_t* frames, int* )
{
    *frames = SFakeAlsa::instance.Read( SFakeAlsa::instance.grantedPeriod );
    return 0;
}

int FakeHwGetBuffer( const AUDIODRIVER::ALSA::snd_pcm_hw_params_t*, AUDIODRIVER::ALSA::snd_pcm_uframes_t* frames )
{
    *frames = SFakeAlsa::instance.Read( SFakeAlsa::instance.grantedBuffer );
    return 0;
}

int FakeSwMalloc( AUDIODRIVER::ALSA::snd_pcm_sw_params_t** p ) { *p = reinterpret_cast< AUDIODRIVER::ALSA::snd_pcm_sw_params_t* >( &SFakeAlsa::instance.token ); return 0; }
void FakeSwFree( AUDIODRIVER::ALSA::snd_pcm_sw_params_t* ) {}
int FakeSwCurrent( AUDIODRIVER::ALSA::snd_pcm_t*, AUDIODRIVER::ALSA::snd_pcm_sw_params_t* ) { return 0; }

int FakeSwStart( AUDIODRIVER::ALSA::snd_pcm_t* pcm, AUDIODRIVER::ALSA::snd_pcm_sw_params_t*, AUDIODRIVER::ALSA::snd_pcm_uframes_t value )
{
    SFakeAlsa& f = SFakeAlsa::instance;
    MT::CScopeMutex scope( f.lock );
    f.startThreshold[ SFakeAlsa::Index( pcm ) ] = (UInt32) value;
    return 0;
}

int FakeSwAvailMin( AUDIODRIVER::ALSA::snd_pcm_t* pcm, AUDIODRIVER::ALSA::snd_pcm_sw_params_t*, AUDIODRIVER::ALSA::snd_pcm_uframes_t value )
{
    SFakeAlsa& f = SFakeAlsa::instance;
    MT::CScopeMutex scope( f.lock );
    f.availMin[ SFakeAlsa::Index( pcm ) ] = (UInt32) value;
    return 0;
}

int FakeSwParams( AUDIODRIVER::ALSA::snd_pcm_t* pcm, AUDIODRIVER::ALSA::snd_pcm_sw_params_t* )
{
    SFakeAlsa& f = SFakeAlsa::instance;
    MT::CScopeMutex scope( f.lock );
    ++f.swParamsCalls[ SFakeAlsa::Index( pcm ) ];
    return f.failSwParams ? -EINVAL : 0;
}

int FakePrepare( AUDIODRIVER::ALSA::snd_pcm_t* pcm )
{
    SFakeAlsa& f = SFakeAlsa::instance;
    MT::CScopeMutex scope( f.lock );
    ++f.prepares[ SFakeAlsa::Index( pcm ) ];
    return 0;
}

int FakeStart( AUDIODRIVER::ALSA::snd_pcm_t* pcm )
{
    SFakeAlsa& f = SFakeAlsa::instance;
    MT::CScopeMutex scope( f.lock );
    ++f.starts[ SFakeAlsa::Index( pcm ) ];
    return f.failStart ? -EIO : 0;
}

int FakeDrop( AUDIODRIVER::ALSA::snd_pcm_t* pcm )
{
    SFakeAlsa& f = SFakeAlsa::instance;
    MT::CScopeMutex scope( f.lock );
    ++f.drops[ SFakeAlsa::Index( pcm ) ];
    return 0;
}

int FakeDelay( AUDIODRIVER::ALSA::snd_pcm_t*, AUDIODRIVER::ALSA::snd_pcm_sframes_t* delay )
{
    SFakeAlsa& f = SFakeAlsa::instance;
    MT::CScopeMutex scope( f.lock );
    if ( f.failDelay )
        return -EIO;
    *delay = f.delay;
    return 0;
}

int FakeRecover( AUDIODRIVER::ALSA::snd_pcm_t* pcm, int error, int )
{
    SFakeAlsa& f = SFakeAlsa::instance;
    MT::CScopeMutex scope( f.lock );
    const UInt32 index = SFakeAlsa::Index( pcm );
    ++f.recoverCalls[ index ];
    f.lastRecoverError[ index ] = error;
    // as libasound: what it has no recovery for is given back
    if ( f.failRecover || ( -EPIPE != error && -ESTRPIPE != error && -EINTR != error ) )
        return error;
    return 0;
}

const char* FakeStrerror( int ) { return "fake error"; }

/** Takes a pace of 48 kHz: the time that many frames take, in whole milliseconds, at least one */
void FakePace( UInt32 frames )
{
    MT::ThreadDelay( frames / 48 > 0 ? frames / 48 : 1 );
}

AUDIODRIVER::ALSA::snd_pcm_sframes_t FakeWritei( AUDIODRIVER::ALSA::snd_pcm_t* pcm, const void* buffer, AUDIODRIVER::ALSA::snd_pcm_uframes_t frames )
{
    SFakeAlsa& f = SFakeAlsa::instance;
    UInt32 count = 0;
    {
        MT::CScopeMutex scope( f.lock );
        const UInt32 index = SFakeAlsa::Index( pcm );
        if ( f.writeErrorArmed && f.writes[ index ] == f.writeErrorAfter )
        {
            f.writeErrorArmed = false;
            return f.writeError;
        }
        ++f.writes[ index ];
        count = (UInt32) frames < f.chunk ? (UInt32) frames : f.chunk;
        f.framesWritten[ index ] += count;
        if ( f.loopback )
        {
            const UInt8* bytes = static_cast< const UInt8* >( buffer );
            for ( UInt32 i=0; i<count*2 && f.ringUsed < (UInt32) SFakeAlsa::RingBytes; ++i, ++bytes, ++f.ringUsed )
                f.ring[ ( f.ringStart + f.ringUsed ) % SFakeAlsa::RingBytes ] = *bytes;
        }
    }
    FakePace( count );
    return count;
}

AUDIODRIVER::ALSA::snd_pcm_sframes_t FakeReadi( AUDIODRIVER::ALSA::snd_pcm_t* pcm, void* buffer, AUDIODRIVER::ALSA::snd_pcm_uframes_t frames )
{
    SFakeAlsa& f = SFakeAlsa::instance;
    while ( true )
    {
        UInt32 count = 0;
        {
            MT::CScopeMutex scope( f.lock );
            const UInt32 index = SFakeAlsa::Index( pcm );
            if ( f.abortReads )
                return -ENODEV;
            if ( f.readErrorArmed && f.reads[ index ] == f.readErrorAfter )
            {
                f.readErrorArmed = false;
                f.counter = (UInt16) ( f.counter + f.readErrorLostFrames );       // what was recorded meanwhile is lost
                return f.readError;
            }

            count = (UInt32) frames < f.chunk ? (UInt32) frames : f.chunk;
            if ( f.loopback )
            {
                const UInt32 available = f.ringUsed / 2;
                count = count < available ? count : available;
                UInt8* bytes = static_cast< UInt8* >( buffer );
                for ( UInt32 i=0; i<count*2; ++i, ++f.ringStart, --f.ringUsed )
                    *bytes++ = f.ring[ f.ringStart % SFakeAlsa::RingBytes ];
                f.ringStart %= SFakeAlsa::RingBytes;
            }
            else
            {
                UInt16* samples = static_cast< UInt16* >( buffer );
                for ( UInt32 i=0; i<count; ++i )
                    samples[ i ] = f.counter++;
            }
            if ( count > 0 )
                ++f.reads[ index ];
        }

        if ( count > 0 )
        {
            if ( !f.loopback )
                FakePace( count );
            return count;
        }
        MT::ThreadDelay( 1 );       // loopback: nothing written yet
    }
}

struct SFakeHint { const char* name; const char* description; const char* ioid; };
const SFakeHint FakeHints[] = {
    { "hw:Out", "Output only\nsecond line", "Output" },
    { "hw:In", "Input only", "Input" },
    { "dev:Both", "Both ways", GUCEF_NULL },
    { "null", "Discard", "Input" },
    { "hw:Out2", "Another output", "Output" } };
const UInt32 FakeHintCount = sizeof( FakeHints ) / sizeof( FakeHints[ 0 ] );

int FakeNameHint( int, const char*, void*** hints )
{
    ++SFakeAlsa::instance.hintCalls;
    void** list = static_cast< void** >( ::malloc( ( FakeHintCount + 1 ) * sizeof( void* ) ) );
    for ( UInt32 i=0; i<FakeHintCount; ++i )
        list[ i ] = const_cast< SFakeHint* >( &FakeHints[ i ] );
    list[ FakeHintCount ] = GUCEF_NULL;
    *hints = list;
    return 0;
}

char* FakeNameGetHint( const void* hint, const char* id )
{
    const SFakeHint* h = static_cast< const SFakeHint* >( hint );
    const char* value = ( 0 == strcmp( id, "NAME" ) ? h->name : ( 0 == strcmp( id, "DESC" ) ? h->description : h->ioid ) );
    return GUCEF_NULL == value ? GUCEF_NULL : ::strdup( value );
}

int FakeNameFreeHint( void** hints ) { ::free( hints ); return 0; }

/** The entry points the plugin exports for tests */
typedef void* ( *TNewTestApi )( void );
typedef void ( *TDeleteTestApi )( void* );
typedef void* ( *TNewTestDriver )( void* );
typedef Int32 ( *TGetCurrentDelay )( void*, Int32* );
typedef void ( *TComputeSoftwareParams )( Int32, UInt32, UInt32, UInt32*, UInt32* );

struct SAlsaTestAccess
{
    SAlsaTestAccess( void ) : module( GUCEF_NULL ), newApi( GUCEF_NULL ), deleteApi( GUCEF_NULL ), newDriver( GUCEF_NULL ), getDelay( GUCEF_NULL ), computeSw( GUCEF_NULL )
    {
        // the plugin is loaded already (by LoadAlsaPlugin()), this only finds it
        const char* names[ 2 ] = { "libaudiodriverALSA_d.so", "libaudiodriverALSA.so" };
        for ( int i=0; i<2 && GUCEF_NULL == module; ++i )
            module = ::dlopen( names[ i ], RTLD_NOW | RTLD_NOLOAD );
        if ( GUCEF_NULL != module )
        {
            newApi = (TNewTestApi) ::dlsym( module, "GUCEFPlugin_ALSA_NewTestApi" );
            deleteApi = (TDeleteTestApi) ::dlsym( module, "GUCEFPlugin_ALSA_DeleteTestApi" );
            newDriver = (TNewTestDriver) ::dlsym( module, "GUCEFPlugin_ALSA_NewTestDriver" );
            getDelay = (TGetCurrentDelay) ::dlsym( module, "GUCEFPlugin_ALSA_GetCurrentDelay" );
            computeSw = (TComputeSoftwareParams) ::dlsym( module, "GUCEFPlugin_ALSA_ComputeSoftwareParams" );
        }
    }
    ~SAlsaTestAccess() { if ( GUCEF_NULL != module ) ::dlclose( module ); }
    bool IsAvailable( void ) const { return GUCEF_NULL != newApi && GUCEF_NULL != deleteApi && GUCEF_NULL != newDriver && GUCEF_NULL != getDelay && GUCEF_NULL != computeSw; }

    void* module;
    TNewTestApi newApi;
    TDeleteTestApi deleteApi;
    TNewTestDriver newDriver;
    TGetCurrentDelay getDelay;
    TComputeSoftwareParams computeSw;
};

/** A driver of the plugin on the fake libasound. The devices opened with it must be released before it goes out of scope */
class CFakeAlsaScope
{
    public:

    /** 'withOptional' false leaves out the functions the driver can do without: capture, software parameters, delay */
    CFakeAlsaScope( const SAlsaTestAccess& access, bool withCapture = true, bool withSoftwareParams = true, bool withDelay = true )
        : m_access( access ), m_api( static_cast< AUDIODRIVER::ALSA::CAlsaApi* >( access.newApi() ) )
    {
        SFakeAlsa::instance.Reset();
        AUDIODRIVER::ALSA::CAlsaApi& a = *m_api;
        a.pcmOpen = FakeOpen; a.pcmClose = FakeClose; a.hwParamsMalloc = FakeHwMalloc; a.hwParamsFree = FakeHwFree; a.hwParamsAny = FakeHwAny;
        a.hwParamsSetAccess = FakeHwSetAccess; a.hwParamsTestFormat = FakeHwTestFormat; a.hwParamsSetFormat = FakeHwSetFormat;
        a.hwParamsSetChannelsNear = FakeHwSetChannels; a.hwParamsSetRateNear = FakeHwSetRate; a.hwParamsSetPeriodSizeNear = FakeHwSetPeriod;
        a.hwParamsSetBufferSizeNear = FakeHwSetBuffer; a.hwParams = FakeHwParams; a.hwParamsGetPeriodSize = FakeHwGetPeriod; a.hwParamsGetBufferSize = FakeHwGetBuffer;
        a.pcmPrepare = FakePrepare; a.pcmDrop = FakeDrop; a.pcmWritei = FakeWritei; a.pcmRecover = FakeRecover; a.strerror = FakeStrerror;
        a.deviceNameHint = FakeNameHint; a.deviceNameGetHint = FakeNameGetHint; a.deviceNameFreeHint = FakeNameFreeHint;
        if ( withCapture )
        {
            a.pcmReadi = FakeReadi;
            a.pcmStart = FakeStart;
        }
        if ( withSoftwareParams )
        {
            a.swParamsMalloc = FakeSwMalloc; a.swParamsFree = FakeSwFree; a.swParamsCurrent = FakeSwCurrent;
            a.swParamsSetStartThreshold = FakeSwStart; a.swParamsSetAvailMin = FakeSwAvailMin; a.swParams = FakeSwParams;
        }
        if ( withDelay )
            a.pcmDelay = FakeDelay;
        driver = AUDIO::TAudioDriverPtr( static_cast< AUDIO::CAudioDriver* >( access.newDriver( m_api ) ) );
    }

    ~CFakeAlsaScope()
    {
        driver.Unlink();
        m_access.deleteApi( m_api );
    }

    AUDIO::TAudioDriverPtr driver;

    private:

    const SAlsaTestAccess& m_access;
    AUDIODRIVER::ALSA::CAlsaApi* m_api;
};

/** Parameters for a device on the fake: 16 bit mono at 48 kHz, 240 frame periods */
AUDIO::CAudioDeviceParams
MakeFakeParams( AUDIO::TAudioDirection direction, UInt32 periods = 4 )
{
    AUDIO::CAudioDeviceParams params;
    params.SetDirection( direction );
    params.SetDeviceId( AUDIO::AUDIODIRECTION_CAPTURE == direction ? "fake_in" : "fake_out" );
    params.SetPreferredFormat( AUDIO::CAudioFormat( 48000, 1, AUDIO::SAMPLETYPE_INT16 ) );
    params.SetBufferSizeInFrames( 240 );
    params.GetOptions().Set( "periods", CORE::ToString( periods ) );
    return params;
}

/** Waits for a condition on the fake (which the device thread changes), true when it came */
template< typename TCondition >
bool
WaitUntil( TCondition condition, UInt32 timeoutInMs )
{
    UInt32 waited = 0;
    while ( !condition() && waited < timeoutInMs )
    {
        MT::ThreadDelay( 2 );
        waited += 2;
    }
    return condition();
}

struct SBlocksAtLeast
{
    SBlocksAtLeast( const CCaptureProbe& p, UInt32 n ) : probe( p ), count( n ) {}
    bool operator()( void ) const { return probe.blocks.Load() >= count; }
    const CCaptureProbe& probe;
    UInt32 count;
};

struct SDeviceStopped
{
    explicit SDeviceStopped( const AUDIO::CAudioDevice& d ) : device( d ) {}
    bool operator()( void ) const { return !device.IsStarted(); }
    const AUDIO::CAudioDevice& device;
};

struct SFramesAtLeast
{
    SFramesAtLeast( const CSilenceCallback& c, UInt32 n ) : callback( c ), count( n ) {}
    bool operator()( void ) const { return callback.frames.Load() >= count; }
    const CSilenceCallback& callback;
    UInt32 count;
};

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

    // Capture. The default run only lists devices and records from the ALSA "null" device, which makes silence
    // and no sound; a real device is used when GUCEF_TEST_AUDIO_CAPTURE_DEVICE names one
    GUCEF_TESTFW_TESTCASE( "ALSA: Capture devices are listed with the capture direction" )
        if ( driver.IsNULL() )
            GUCEF_TESTFW_SKIP( "the ALSA driver is not available" );
        AUDIO::TAudioDeviceInfoVector captureDevices;
        const bool listed = driver->EnumerateDevices( AUDIO::AUDIODIRECTION_CAPTURE, captureDevices );
        if ( listed && captureDevices.empty() )
            GUCEF_TESTFW_SKIP( "this machine lists no ALSA capture device" );
        try
        {
            ASSERT_TRUE( listed );
            for ( size_t i=0; i<captureDevices.size(); ++i )
            {
                ASSERT_TRUE( AUDIO::AUDIODIRECTION_CAPTURE == captureDevices[ i ].GetDirection() );
                ASSERT_FALSE( captureDevices[ i ].GetId().IsNULLOrEmpty() );
                ASSERT_TRUE( "null" != captureDevices[ i ].GetId() );
            }
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ALSA lists " << captureDevices.size() << " capture devices" );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "ALSA: A capture device opens with a negotiated format, delivers whole periods and stops" )
        if ( driver.IsNULL() )
            GUCEF_TESTFW_SKIP( "the ALSA driver is not available" );
        AUDIO::CAudioDeviceParams captureParams;
        captureParams.SetDirection( AUDIO::AUDIODIRECTION_CAPTURE );
        const std::string captureName = GUCEF_TESTFW_GETENV( "GUCEF_TEST_AUDIO_CAPTURE_DEVICE" );
        captureParams.SetDeviceId( captureName.empty() ? CORE::CString( "null" ) : CORE::CString( captureName.c_str() ) );
        captureParams.SetBufferSizeInFrames( 240 );
        CCaptureProbe probe;
        AUDIO::TAudioDevicePtr captureDevice;
        if ( !driver->OpenDevice( captureParams, &probe, captureDevice ) )
            GUCEF_TESTFW_SKIP( "no ALSA capture device could be opened" );
        try
        {
            AUDIO::CAudioPushDevice* push = captureDevice->AsPushDevice();
            ASSERT_TRUE( GUCEF_NULL != push );
            ASSERT_TRUE( AUDIO::AUDIODIRECTION_CAPTURE == captureDevice->GetDirection() );
            ASSERT_TRUE( captureDevice->GetFormat().IsValid() );
            const UInt32 period = captureDevice->GetPeriodSizeInFrames();
            ASSERT_TRUE( period > 0 && period == captureDevice->GetLatencyInFrames() );
            probe.expectedBlock = period;
            probe.device = push;

            ASSERT_TRUE( captureDevice->Start() );
            ASSERT_TRUE( WaitUntil( SBlocksAtLeast( probe, 5 ), 5000 ) );
            ASSERT_TRUE( captureDevice->IsStarted() );
            ASSERT_TRUE( captureDevice->Stop() );
            ASSERT_FALSE( captureDevice->IsStarted() );

            ASSERT_TRUE( 0 == probe.lost.Load() && 0 == probe.badBlockSizes.Load() );
            ASSERT_TRUE( probe.frames.Load() == (UInt64) probe.blocks.Load() * period );
            ASSERT_TRUE( push->GetTotalFramesPushed() == probe.frames.Load() );
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ALSA captured " << probe.blocks.Load() << " blocks of " << period << " frames from \"" << captureParams.GetDeviceId() << "\", overruns " << push->GetOverrunCount() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Opt-in, a real input: 2 s at 240-frame periods. Set GUCEF_TEST_AUDIO_CAPTURE_DEVICE (a microphone or a line input)
    GUCEF_TESTFW_TESTCASE( "ALSA: Recording check, 2 s from GUCEF_TEST_AUDIO_CAPTURE_DEVICE at 240-frame periods" )
        GUCEF_TESTFW_REQUIRE_ENV( deviceName, "GUCEF_TEST_AUDIO_CAPTURE_DEVICE" );
        if ( driver.IsNULL() )
            GUCEF_TESTFW_SKIP( "the ALSA driver is not available" );
        AUDIO::CAudioDeviceParams recordParams;
        recordParams.SetDirection( AUDIO::AUDIODIRECTION_CAPTURE );
        recordParams.SetDeviceId( CORE::CString( deviceName.c_str() ) );
        recordParams.SetBufferSizeInFrames( 240 );
        CCaptureProbe probe;
        AUDIO::TAudioDevicePtr recordDevice;
        if ( !driver->OpenDevice( recordParams, &probe, recordDevice ) )
            GUCEF_TESTFW_SKIP( "the capture device could not be opened" );
        try
        {
            AUDIO::CAudioPushDevice* push = recordDevice->AsPushDevice();
            ASSERT_TRUE( GUCEF_NULL != push );
            const AUDIO::CAudioFormat format = recordDevice->GetFormat();
            ASSERT_TRUE( format.IsValid() );
            probe.expectedBlock = recordDevice->GetPeriodSizeInFrames();
            probe.device = push;
            ASSERT_TRUE( recordDevice->Start() );
            MT::PrecisionDelay( 2000 );
            ASSERT_TRUE( recordDevice->Stop() );

            const UInt64 expected = (UInt64) format.GetSampleRate() * 2;
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Recorded " << probe.frames.Load() << " frames of " << expected << " expected at " << format.GetSampleRate() << " Hz, " <<
                format.GetChannelCount() << " channels, period " << probe.expectedBlock << ", overruns " << push->GetOverrunCount() );
            ASSERT_TRUE( 0 == probe.lost.Load() && 0 == probe.badBlockSizes.Load() );
            ASSERT_TRUE( probe.frames.Load() > expected * 8 / 10 && probe.frames.Load() < expected * 12 / 10 );
            if ( "null" != deviceName )
            {
                // a live input has noise at least; digital silence means it records nothing
                const bool varied = probe.distinct;
                ASSERT_TRUE( varied );
            }
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // The rest runs the driver on a fake libasound, through the entry points the plugin exports for it
    SAlsaTestAccess access;
    const bool fakeAvailable = pluginLoaded && access.IsAvailable();

    GUCEF_TESTFW_TESTCASE( "ALSA: Software parameters, avail_min is a period, playback starts one period short of full and capture at 1" )
        if ( !fakeAvailable )
            GUCEF_TESTFW_SKIP( "the plugin has no test access" );
        try
        {
            struct SCase { Int32 capture; UInt32 period; UInt32 buffer; UInt32 availMin; UInt32 start; };
            const SCase cases[] = {
                { 0,  240,  960, 240,  720 }, { 0, 1024, 4096, 1024, 3072 }, { 0, 240, 480, 240, 240 }, { 0, 240, 240, 240, 1 },
                { 0,    0,    0,   1,    1 }, { 1,  240,  960, 240,    1 }, { 1, 1024, 4096, 1024,   1 }, { 1,   0,   0,   1, 1 } };
            for ( size_t i=0; i<sizeof( cases ) / sizeof( cases[ 0 ] ); ++i )
            {
                UInt32 availMin = 99999, start = 99999;
                access.computeSw( cases[ i ].capture, cases[ i ].period, cases[ i ].buffer, &availMin, &start );
                ASSERT_TRUE( cases[ i ].availMin == availMin && cases[ i ].start == start );
            }
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "ALSA: Fake libasound, devices are listed per direction and the null device never" )
        if ( !fakeAvailable )
            GUCEF_TESTFW_SKIP( "the plugin has no test access" );
        try
        {
            CFakeAlsaScope fake( access );
            AUDIO::TAudioDeviceInfoVector playback, capture;
            ASSERT_TRUE( fake.driver->EnumerateDevices( AUDIO::AUDIODIRECTION_PLAYBACK, playback ) );
            ASSERT_TRUE( fake.driver->EnumerateDevices( AUDIO::AUDIODIRECTION_CAPTURE, capture ) );
            ASSERT_TRUE( 3 == playback.size() && 2 == capture.size() );
            ASSERT_TRUE( "hw:Out" == playback[ 0 ].GetId() && "dev:Both" == playback[ 1 ].GetId() && "hw:Out2" == playback[ 2 ].GetId() );
            ASSERT_TRUE( "hw:In" == capture[ 0 ].GetId() && "dev:Both" == capture[ 1 ].GetId() );
            ASSERT_TRUE( AUDIO::AUDIODIRECTION_PLAYBACK == playback[ 1 ].GetDirection() && AUDIO::AUDIODIRECTION_CAPTURE == capture[ 1 ].GetDirection() );
            ASSERT_TRUE( "Output only second line" == playback[ 0 ].GetName() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "ALSA: Fake libasound, the playback device gets its software parameters and the generic options" )
        if ( !fakeAvailable )
            GUCEF_TESTFW_SKIP( "the plugin has no test access" );
        try
        {
            SFakeAlsa& f = SFakeAlsa::instance;
            CFakeAlsaScope fake( access );
            CSilenceCallback callback;
            AUDIO::CAudioDeviceParams params = MakeFakeParams( AUDIO::AUDIODIRECTION_PLAYBACK );
            params.GetOptions().Set( AUDIO::CAudioPushDevice::RealtimePriorityOptionName, "7" );
            {
                AUDIO::TAudioDevicePtr device;
                ASSERT_TRUE( fake.driver->OpenDevice( params, &callback, device ) );
                ASSERT_TRUE( 0 == f.Read( f.lastStream ) && "fake_out" == f.Read( f.lastName ) );
                ASSERT_TRUE( 240 == f.Read( f.requestedPeriod ) && 960 == f.Read( f.requestedBuffer ) );
                ASSERT_TRUE( 240 == f.Read( f.availMin[ 0 ] ) && 720 == f.Read( f.startThreshold[ 0 ] ) && 1 == f.Read( f.swParamsCalls[ 0 ] ) );
                ASSERT_TRUE( AUDIO::AUDIODIRECTION_PLAYBACK == device->GetDirection() );
                ASSERT_TRUE( 240 == device->GetPeriodSizeInFrames() && 960 == device->GetLatencyInFrames() );
                ASSERT_TRUE( 48000 == device->GetFormat().GetSampleRate() && 1 == device->GetFormat().GetChannelCount() && AUDIO::SAMPLETYPE_INT16 == device->GetFormat().GetSampleType() );
                AUDIO::CAudioPushDevice* push = device->AsPushDevice();
                ASSERT_TRUE( GUCEF_NULL != push && 7 == push->GetRealtimePriority() );

                ASSERT_TRUE( device->Start() );
                ASSERT_TRUE( WaitUntil( SFramesAtLeast( callback, 2400 ), 5000 ) );
                ASSERT_TRUE( device->Stop() );
                ASSERT_TRUE( 1 == f.Read( f.prepares[ 0 ] ) && 0 == f.Read( f.starts[ 0 ] ) && 1 == f.Read( f.drops[ 0 ] ) );
                ASSERT_TRUE( 0 == callback.lost.Load() );
            }
            ASSERT_TRUE( 1 == f.Read( f.closes[ 0 ] ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "ALSA: Fake libasound, the capture device negotiates, sets avail_min and a start threshold of 1 and is started" )
        if ( !fakeAvailable )
            GUCEF_TESTFW_SKIP( "the plugin has no test access" );
        try
        {
            SFakeAlsa& f = SFakeAlsa::instance;
            CFakeAlsaScope fake( access );
            f.forcedPeriod = 256;               // the device does not do 240 and says so
            CCaptureProbe probe;
            AUDIO::CAudioDeviceParams params = MakeFakeParams( AUDIO::AUDIODIRECTION_CAPTURE, 3 );
            params.GetOptions().Set( AUDIO::CAudioPushDevice::RealtimePriorityOptionName, "5" );
            {
                AUDIO::TAudioDevicePtr device;
                ASSERT_TRUE( fake.driver->OpenDevice( params, &probe, device ) );
                ASSERT_TRUE( 1 == f.Read( f.lastStream ) && "fake_in" == f.Read( f.lastName ) );
                ASSERT_TRUE( 240 == f.Read( f.requestedPeriod ) && 768 == f.Read( f.requestedBuffer ) );     // periods of the period the device granted
                ASSERT_TRUE( 256 == f.Read( f.availMin[ 1 ] ) && 1 == f.Read( f.startThreshold[ 1 ] ) && 1 == f.Read( f.swParamsCalls[ 1 ] ) );
                ASSERT_TRUE( AUDIO::AUDIODIRECTION_CAPTURE == device->GetDirection() );
                ASSERT_TRUE( 256 == device->GetPeriodSizeInFrames() && 256 == device->GetLatencyInFrames() );
                AUDIO::CAudioPushDevice* push = device->AsPushDevice();
                ASSERT_TRUE( GUCEF_NULL != push && 5 == push->GetRealtimePriority() );
                ASSERT_TRUE( 0 == f.Read( f.prepares[ 1 ] ) && 0 == f.Read( f.starts[ 1 ] ) );

                probe.device = push;
                ASSERT_TRUE( device->Start() );
                ASSERT_TRUE( 1 == f.Read( f.prepares[ 1 ] ) && 1 == f.Read( f.starts[ 1 ] ) );     // started explicitly, before the first read
                ASSERT_TRUE( device->Stop() );
                ASSERT_TRUE( 1 == f.Read( f.drops[ 1 ] ) );

                // a start that ALSA refuses fails Start() and leaves no thread
                f.failStart = true;
                ASSERT_FALSE( device->Start() );
                ASSERT_FALSE( device->IsStarted() );
            }
            ASSERT_TRUE( 1 == f.Read( f.closes[ 1 ] ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "ALSA: Fake libasound, what libasound does not have turns the feature off and not the driver" )
        if ( !fakeAvailable )
            GUCEF_TESTFW_SKIP( "the plugin has no test access" );
        try
        {
            SFakeAlsa& f = SFakeAlsa::instance;
            CSilenceCallback silence;
            CCaptureProbe probe;
            {
                // no snd_pcm_readi: no capture devices, no capture open, playback unharmed
                CFakeAlsaScope fake( access, false );
                AUDIO::TAudioDeviceInfoVector capture, playback;
                ASSERT_TRUE( fake.driver->EnumerateDevices( AUDIO::AUDIODIRECTION_CAPTURE, capture ) && capture.empty() );
                ASSERT_TRUE( fake.driver->EnumerateDevices( AUDIO::AUDIODIRECTION_PLAYBACK, playback ) && 3 == playback.size() );
                AUDIO::TAudioDevicePtr device;
                ASSERT_FALSE( fake.driver->OpenDevice( MakeFakeParams( AUDIO::AUDIODIRECTION_CAPTURE ), &probe, device ) );
                ASSERT_TRUE( device.IsNULL() && 0 == f.Read( f.openCalls ) );
                ASSERT_TRUE( fake.driver->OpenDevice( MakeFakeParams( AUDIO::AUDIODIRECTION_PLAYBACK ), &silence, device ) );
            }
            {
                // no software parameter functions: both directions open with the defaults of ALSA
                CFakeAlsaScope fake( access, true, false );
                AUDIO::TAudioDevicePtr playback, capture;
                ASSERT_TRUE( fake.driver->OpenDevice( MakeFakeParams( AUDIO::AUDIODIRECTION_PLAYBACK ), &silence, playback ) );
                ASSERT_TRUE( fake.driver->OpenDevice( MakeFakeParams( AUDIO::AUDIODIRECTION_CAPTURE ), &probe, capture ) );
                ASSERT_TRUE( 0 == f.Read( f.swParamsCalls[ 0 ] ) && 0 == f.Read( f.swParamsCalls[ 1 ] ) );
            }
            {
                // ALSA refuses the software parameters: the same
                CFakeAlsaScope fake( access );
                SFakeAlsa::instance.failSwParams = true;
                AUDIO::TAudioDevicePtr playback, capture;
                ASSERT_TRUE( fake.driver->OpenDevice( MakeFakeParams( AUDIO::AUDIODIRECTION_PLAYBACK ), &silence, playback ) );
                ASSERT_TRUE( fake.driver->OpenDevice( MakeFakeParams( AUDIO::AUDIODIRECTION_CAPTURE ), &probe, capture ) );
                ASSERT_TRUE( 1 == f.Read( f.swParamsCalls[ 0 ] ) && 1 == f.Read( f.swParamsCalls[ 1 ] ) );
            }
            {
                // a device that does not open, and a direction that is none
                CFakeAlsaScope fake( access );
                SFakeAlsa::instance.failOpen = true;
                AUDIO::TAudioDevicePtr device;
                ASSERT_FALSE( fake.driver->OpenDevice( MakeFakeParams( AUDIO::AUDIODIRECTION_CAPTURE ), &probe, device ) );
                ASSERT_FALSE( fake.driver->OpenDevice( MakeFakeParams( AUDIO::AUDIODIRECTION_CAPTURE ), GUCEF_NULL, device ) );
                ASSERT_TRUE( device.IsNULL() );
            }
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "ALSA: Fake libasound, GetCurrentDelayInFrames asks snd_pcm_delay of either direction" )
        if ( !fakeAvailable )
            GUCEF_TESTFW_SKIP( "the plugin has no test access" );
        try
        {
            SFakeAlsa& f = SFakeAlsa::instance;
            CSilenceCallback silence;
            CCaptureProbe probe;
            {
                CFakeAlsaScope fake( access );
                AUDIO::TAudioDevicePtr playback, capture;
                ASSERT_TRUE( fake.driver->OpenDevice( MakeFakeParams( AUDIO::AUDIODIRECTION_PLAYBACK ), &silence, playback ) );
                ASSERT_TRUE( fake.driver->OpenDevice( MakeFakeParams( AUDIO::AUDIODIRECTION_CAPTURE ), &probe, capture ) );
                Int32 delay = -1;
                f.delay = 311;
                ASSERT_TRUE( 1 == access.getDelay( playback.GetPointer(), &delay ) && 311 == delay );
                delay = -1;
                ASSERT_TRUE( 1 == access.getDelay( capture.GetPointer(), &delay ) && 311 == delay );
                f.failDelay = true;
                delay = -1;
                ASSERT_TRUE( 0 == access.getDelay( capture.GetPointer(), &delay ) && -1 == delay );
            }
            {
                CFakeAlsaScope fake( access, true, true, false );
                AUDIO::TAudioDevicePtr playback;
                ASSERT_TRUE( fake.driver->OpenDevice( MakeFakeParams( AUDIO::AUDIODIRECTION_PLAYBACK ), &silence, playback ) );
                Int32 delay = -1;
                ASSERT_TRUE( 0 == access.getDelay( playback.GetPointer(), &delay ) && -1 == delay );
            }
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "ALSA: Fake libasound, partial reads are assembled into whole periods in order" )
        if ( !fakeAvailable )
            GUCEF_TESTFW_SKIP( "the plugin has no test access" );
        try
        {
            SFakeAlsa& f = SFakeAlsa::instance;
            CFakeAlsaScope fake( access );
            CCaptureProbe probe;
            {
                AUDIO::TAudioDevicePtr device;
                ASSERT_TRUE( fake.driver->OpenDevice( MakeFakeParams( AUDIO::AUDIODIRECTION_CAPTURE ), &probe, device ) );
                AUDIO::CAudioPushDevice* push = device->AsPushDevice();
                ASSERT_TRUE( GUCEF_NULL != push );
                probe.device = push;
                probe.expectedBlock = 240;
                ASSERT_TRUE( device->Start() );
                ASSERT_TRUE( WaitUntil( SBlocksAtLeast( probe, 20 ), 5000 ) );
                ASSERT_TRUE( device->Stop() );

                ASSERT_TRUE( probe.blocks.Load() >= 20 && 0 == probe.badBlockSizes.Load() );
                ASSERT_TRUE( 0 == probe.jumps.Load() && 0 == probe.firstSample.Load() );
                ASSERT_TRUE( probe.frames.Load() == (UInt64) probe.blocks.Load() * 240 && push->GetTotalFramesPushed() == probe.frames.Load() );
                ASSERT_TRUE( 0 == push->GetOverrunCount() && 0 == f.Read( f.recoverCalls[ 1 ] ) && 0 == probe.lost.Load() );
                ASSERT_TRUE( f.Read( f.reads[ 1 ] ) >= probe.blocks.Load() * 2 );        // 100 frames at most per read
            }
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "ALSA: Fake libasound, an overrun is recovered, restarted and reported with the block that holds the gap" )
        if ( !fakeAvailable )
            GUCEF_TESTFW_SKIP( "the plugin has no test access" );
        try
        {
            SFakeAlsa& f = SFakeAlsa::instance;
            CSilenceCallback silence;

            // -EPIPE and -ESTRPIPE lose audio: 1000 frames are skipped in the fake, which the probe sees as one jump
            const int lossy[ 2 ] = { -EPIPE, -ESTRPIPE };
            for ( int i=0; i<2; ++i )
            {
                CFakeAlsaScope fake( access );
                CCaptureProbe probe;
                AUDIO::TAudioDevicePtr device;
                ASSERT_TRUE( fake.driver->OpenDevice( MakeFakeParams( AUDIO::AUDIODIRECTION_CAPTURE ), &probe, device ) );
                AUDIO::CAudioPushDevice* push = device->AsPushDevice();
                ASSERT_TRUE( GUCEF_NULL != push );
                probe.device = push;
                probe.expectedBlock = 240;
                f.readError = lossy[ i ]; f.readErrorAfter = 10; f.readErrorLostFrames = 1000; f.readErrorArmed = true;
                ASSERT_TRUE( device->Start() );
                ASSERT_TRUE( WaitUntil( SBlocksAtLeast( probe, 12 ), 5000 ) );
                ASSERT_TRUE( device->Stop() );

                ASSERT_TRUE( 1 == push->GetOverrunCount() );
                ASSERT_TRUE( 1 == probe.jumps.Load() );
                ASSERT_TRUE( 0 != probe.blockOfFirstJump.Load() && probe.blockOfFirstJump.Load() == probe.blockOfFirstOverrun.Load() );
                ASSERT_TRUE( 1 == f.Read( f.recoverCalls[ 1 ] ) && lossy[ i ] == f.Read( f.lastRecoverError[ 1 ] ) );
                ASSERT_TRUE( 2 == f.Read( f.starts[ 1 ] ) );                  // at Start() and after the recovery
                ASSERT_TRUE( 0 == probe.lost.Load() && 0 == probe.badBlockSizes.Load() );
            }

            // an interruption loses nothing
            {
                CFakeAlsaScope fake( access );
                CCaptureProbe probe;
                AUDIO::TAudioDevicePtr device;
                ASSERT_TRUE( fake.driver->OpenDevice( MakeFakeParams( AUDIO::AUDIODIRECTION_CAPTURE ), &probe, device ) );
                AUDIO::CAudioPushDevice* push = device->AsPushDevice();
                ASSERT_TRUE( GUCEF_NULL != push );
                probe.device = push;
                f.readError = -EINTR; f.readErrorAfter = 10; f.readErrorLostFrames = 0; f.readErrorArmed = true;
                ASSERT_TRUE( device->Start() );
                ASSERT_TRUE( WaitUntil( SBlocksAtLeast( probe, 12 ), 5000 ) );
                ASSERT_TRUE( device->Stop() );
                ASSERT_TRUE( 0 == push->GetOverrunCount() && 0 == probe.jumps.Load() && 1 == f.Read( f.recoverCalls[ 1 ] ) && 1 == f.Read( f.starts[ 1 ] ) );
            }

            // playback ran dry: counted, recovered, the period is written after
            const int dry[ 2 ] = { -EPIPE, -EINTR };
            for ( int i=0; i<2; ++i )
            {
                CFakeAlsaScope fake( access );
                CSilenceCallback callback;
                AUDIO::TAudioDevicePtr device;
                ASSERT_TRUE( fake.driver->OpenDevice( MakeFakeParams( AUDIO::AUDIODIRECTION_PLAYBACK ), &callback, device ) );
                AUDIO::CAudioPushDevice* push = device->AsPushDevice();
                ASSERT_TRUE( GUCEF_NULL != push );
                f.writeError = dry[ i ]; f.writeErrorAfter = 10; f.writeErrorArmed = true;
                ASSERT_TRUE( device->Start() );
                ASSERT_TRUE( WaitUntil( SFramesAtLeast( callback, 4800 ), 5000 ) );
                ASSERT_TRUE( device->Stop() );
                ASSERT_TRUE( ( 0 == i ? 1 : 0 ) == push->GetUnderrunCount() );
                ASSERT_TRUE( 1 == f.Read( f.recoverCalls[ 0 ] ) && dry[ i ] == f.Read( f.lastRecoverError[ 0 ] ) && 0 == callback.lost.Load() );
            }
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "ALSA: Fake libasound, a device that cannot recover is lost once and the loop ends" )
        if ( !fakeAvailable )
            GUCEF_TESTFW_SKIP( "the plugin has no test access" );
        try
        {
            SFakeAlsa& f = SFakeAlsa::instance;
            const int errors[ 2 ] = { -ENODEV, -EPIPE };
            for ( int i=0; i<2; ++i )
            {
                CFakeAlsaScope fake( access );
                CCaptureProbe probe;
                AUDIO::TAudioDevicePtr device;
                ASSERT_TRUE( fake.driver->OpenDevice( MakeFakeParams( AUDIO::AUDIODIRECTION_CAPTURE ), &probe, device ) );
                AUDIO::CAudioPushDevice* push = device->AsPushDevice();
                ASSERT_TRUE( GUCEF_NULL != push );
                probe.device = push;
                f.failRecover = ( 1 == i );             // -ENODEV is never recoverable, -EPIPE here
                f.readError = errors[ i ]; f.readErrorAfter = 10; f.readErrorLostFrames = 0; f.readErrorArmed = true;
                ASSERT_TRUE( device->Start() );
                ASSERT_TRUE( WaitUntil( SDeviceStopped( *device ), 5000 ) );
                ASSERT_TRUE( 1 == probe.lost.Load() && 0 == push->GetOverrunCount() );
                ASSERT_TRUE( device->Stop() );
                ASSERT_TRUE( ( 0 == i ? 0 : 1 ) == f.Read( f.recoverCalls[ 1 ] ) );     // what libasound cannot recover from is not even tried
            }
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "ALSA: Fake libasound, playback to capture is bit exact through two devices of the driver" )
        if ( !fakeAvailable )
            GUCEF_TESTFW_SKIP( "the plugin has no test access" );
        try
        {
            SFakeAlsa& f = SFakeAlsa::instance;
            CFakeAlsaScope fake( access );
            f.loopback = true;
            CRampRenderer renderer;
            CCaptureProbe probe;
            {
                AUDIO::TAudioDevicePtr playback, capture;
                ASSERT_TRUE( fake.driver->OpenDevice( MakeFakeParams( AUDIO::AUDIODIRECTION_PLAYBACK, 3 ), &renderer, playback ) );
                ASSERT_TRUE( fake.driver->OpenDevice( MakeFakeParams( AUDIO::AUDIODIRECTION_CAPTURE, 3 ), &probe, capture ) );
                AUDIO::CAudioPushDevice* capturePush = capture->AsPushDevice();
                ASSERT_TRUE( GUCEF_NULL != capturePush );
                probe.device = capturePush;
                probe.expectedBlock = 240;

                ASSERT_TRUE( capture->Start() );
                ASSERT_TRUE( playback->Start() );
                ASSERT_TRUE( WaitUntil( SBlocksAtLeast( probe, 40 ), 10000 ) );
                ASSERT_TRUE( capture->Stop() );
                ASSERT_TRUE( playback->Stop() );

                // every sample that was rendered came back, in order, in whole periods
                ASSERT_TRUE( probe.blocks.Load() >= 40 && 0 == probe.badBlockSizes.Load() );
                ASSERT_TRUE( 0 == probe.jumps.Load() && 0 == probe.firstSample.Load() );
                ASSERT_TRUE( 0 == probe.lost.Load() && 0 == renderer.lost.Load() && 0 == capturePush->GetOverrunCount() );
                ASSERT_TRUE( 240 == f.Read( f.availMin[ 0 ] ) && 480 == f.Read( f.startThreshold[ 0 ] ) );        // playback: buffer 720 less one period
                ASSERT_TRUE( 240 == f.Read( f.availMin[ 1 ] ) && 1 == f.Read( f.startThreshold[ 1 ] ) );
            }
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    #endif
}

/*-------------------------------------------------------------------------*/
