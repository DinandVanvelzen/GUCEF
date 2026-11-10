/*
 *  audiodriverWEBAUDIO: gucefAUDIO driver plugin for the Web Audio API in browsers (Emscripten)
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

#include <stdint.h>

#include <emscripten.h>
#include <emscripten/em_js.h>
#ifdef __EMSCRIPTEN_PTHREADS__
#include <emscripten/threading.h>
#endif

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "audiodriverWEBAUDIO_CWebAudioDriver.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      PAGE SIDE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *  The browser side: an audio context with a script processor node per device, kept in a table on the module by id.
 *  The node asks the device for interleaved floats in wasm memory and spreads them over the channels of the output.
 */

extern "C" {

/** Called by the page for every buffer it plays */
EMSCRIPTEN_KEEPALIVE const float*
audiodriverWEBAUDIO_Render( void* device, int frameCount )
{
    if ( GUCEF_NULL == device || frameCount <= 0 )
        return GUCEF_NULL;
    return static_cast< GUCEF::AUDIODRIVER::WEBAUDIO::CWebAudioDevice* >( device )->Render( (GUCEF::CORE::UInt32) frameCount );
}

EM_JS( int, audiodriverWEBAUDIO_JsIsAvailable, ( void ), {
    return ( typeof AudioContext !== 'undefined' || typeof webkitAudioContext !== 'undefined' ) ? 1 : 0;
} );

EM_JS( int, audiodriverWEBAUDIO_JsOpen, ( int sampleRate, int channels, int bufferFrames ), {
    var Context = ( typeof AudioContext !== 'undefined' ) ? AudioContext : ( ( typeof webkitAudioContext !== 'undefined' ) ? webkitAudioContext : null );
    if ( !Context )
        return 0;
    var context = null;
    try { context = sampleRate > 0 ? new Context( { sampleRate: sampleRate } ) : new Context(); }
    catch ( e ) { try { context = new Context(); } catch ( e2 ) { return 0; } }

    var node = context.createScriptProcessor( bufferFrames, 0, channels );
    var state = { context: context, node: node, device: 0, channels: channels, running: false, resume: null };
    node.onaudioprocess = function( event ) {
        var output = event.outputBuffer;
        var frames = output.length;
        var outputChannels = output.numberOfChannels;
        var pointer = ( state.running && state.device ) ? _audiodriverWEBAUDIO_Render( state.device, frames ) : 0;
        var c = 0;
        if ( !pointer ) {
            for ( c = 0; c < outputChannels; ++c )
                output.getChannelData( c ).fill( 0 );
            return;
        }
        var heap = HEAPF32;
        var base = pointer >> 2;
        for ( c = 0; c < outputChannels; ++c ) {
            var data = output.getChannelData( c );
            if ( c >= state.channels ) { data.fill( 0 ); continue; }
            for ( var i = 0; i < frames; ++i )
                data[ i ] = heap[ base + i * state.channels + c ];
        }
    };

    // a context starts suspended until the user does something on the page
    state.resume = function() {
        if ( state.running && state.context.state === 'suspended' )
            state.context.resume();
    };
    if ( typeof document !== 'undefined' ) {
        [ 'pointerdown', 'keydown', 'touchend' ].forEach( function( type ) {
            document.addEventListener( type, state.resume, true );
        } );
    }

    var table = Module[ 'gucefWebAudio' ] || ( Module[ 'gucefWebAudio' ] = { next: 1, devices: {} } );
    var id = table.next++;
    table.devices[ id ] = state;
    return id;
} );

EM_JS( int, audiodriverWEBAUDIO_JsBind, ( int id, int device ), {
    var table = Module[ 'gucefWebAudio' ];
    var state = table ? table.devices[ id ] : null;
    if ( !state )
        return 0;
    state.device = device;
    return 1;
} );

EM_JS( int, audiodriverWEBAUDIO_JsSampleRate, ( int id ), {
    var table = Module[ 'gucefWebAudio' ];
    var state = table ? table.devices[ id ] : null;
    return state ? ( state.context.sampleRate | 0 ) : 0;
} );

EM_JS( int, audiodriverWEBAUDIO_JsStart, ( int id ), {
    var table = Module[ 'gucefWebAudio' ];
    var state = table ? table.devices[ id ] : null;
    if ( !state )
        return 0;
    if ( !state.running ) {
        state.running = true;
        state.node.connect( state.context.destination );
    }
    state.resume();
    return 1;
} );

EM_JS( int, audiodriverWEBAUDIO_JsStop, ( int id ), {
    var table = Module[ 'gucefWebAudio' ];
    var state = table ? table.devices[ id ] : null;
    if ( !state )
        return 0;
    if ( state.running ) {
        state.running = false;
        state.node.disconnect();
        if ( state.context.state === 'running' )
            state.context.suspend();
    }
    return 1;
} );

EM_JS( int, audiodriverWEBAUDIO_JsClose, ( int id ), {
    var table = Module[ 'gucefWebAudio' ];
    var state = table ? table.devices[ id ] : null;
    if ( !state )
        return 0;
    state.running = false;
    state.node.onaudioprocess = null;
    try { state.node.disconnect(); } catch ( e ) {}
    if ( typeof document !== 'undefined' ) {
        [ 'pointerdown', 'keydown', 'touchend' ].forEach( function( type ) {
            document.removeEventListener( type, state.resume, true );
        } );
    }
    state.context.close();
    delete table.devices[ id ];
    return 1;
} );

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUDIODRIVER {
namespace WEBAUDIO {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

enum EPageCall
{
    PAGECALL_IS_AVAILABLE = 0 ,
    PAGECALL_OPEN             ,
    PAGECALL_SAMPLE_RATE      ,
    PAGECALL_START            ,
    PAGECALL_STOP             ,
    PAGECALL_CLOSE            ,
    PAGECALL_BIND
};

int
PageCallOnThisThread( int call, int a, int b, int c, int d )
{
    switch ( call )
    {
        case PAGECALL_IS_AVAILABLE: return audiodriverWEBAUDIO_JsIsAvailable();
        case PAGECALL_OPEN:         return audiodriverWEBAUDIO_JsOpen( a, b, c );
        case PAGECALL_SAMPLE_RATE:  return audiodriverWEBAUDIO_JsSampleRate( a );
        case PAGECALL_START:        return audiodriverWEBAUDIO_JsStart( a );
        case PAGECALL_STOP:         return audiodriverWEBAUDIO_JsStop( a );
        case PAGECALL_CLOSE:        return audiodriverWEBAUDIO_JsClose( a );
        case PAGECALL_BIND:         return audiodriverWEBAUDIO_JsBind( a, b );
        default:                    return 0;
    }
}

/** The page can only be used from the main browser thread: calls from workers are run there, and waited for */
int
PageCall( int call, int a = 0, int b = 0, int c = 0, int d = 0 )
{
    #ifdef __EMSCRIPTEN_PTHREADS__
    if ( !emscripten_is_main_runtime_thread() )
        return emscripten_sync_run_in_main_runtime_thread( EM_FUNC_SIG_IIIIII, PageCallOnThisThread, call, a, b, c, d );
    #endif
    return PageCallOnThisThread( call, a, b, c, d );
}

/*-------------------------------------------------------------------------*/

bool
IsPowerOfTwo( CORE::UInt32 value )
{
    return 0 != value && 0 == ( value & ( value - 1 ) );
}

/*-------------------------------------------------------------------------*/

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static const CORE::UInt32 DefaultBufferFrames = 1024;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CWebAudioDevice::CWebAudioDevice( const AUDIO::CAudioDeviceInfo& info   ,
                                  AUDIO::IAudioDeviceCallback* callback ,
                                  const AUDIO::CAudioFormat& format     ,
                                  CORE::UInt32 periodSizeInFrames       ,
                                  CORE::Int32 contextId                 )
    : AUDIO::CAudioDevice( info, callback )
    , m_format( format )
    , m_periodSize( periodSizeInFrames )
    , m_contextId( contextId )
    , m_renderBuffer( format, periodSizeInFrames )
    , m_started( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CWebAudioDevice::~CWebAudioDevice()
{GUCEF_TRACE;

    Stop();
    PageCall( PAGECALL_CLOSE, m_contextId );
}

/*-------------------------------------------------------------------------*/

const AUDIO::CAudioFormat&
CWebAudioDevice::GetFormat( void ) const
{GUCEF_TRACE;

    return m_format;
}

/*-------------------------------------------------------------------------*/

bool
CWebAudioDevice::Start( void )
{GUCEF_TRACE;

    if ( m_started.Load() )
        return true;
    m_started.Store( true );
    if ( 0 == PageCall( PAGECALL_START, m_contextId ) )
    {
        m_started.Store( false );
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CWebAudioDevice::Stop( void )
{GUCEF_TRACE;

    // the page calls Render() on the main thread and this runs there too (or waits for it), so once the page has
    // let go of the node no render is under way
    m_started.Store( false );
    PageCall( PAGECALL_STOP, m_contextId );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CWebAudioDevice::IsStarted( void ) const
{GUCEF_TRACE;

    return m_started.Load();
}

/*-------------------------------------------------------------------------*/

CORE::UInt32
CWebAudioDevice::GetPeriodSizeInFrames( void ) const
{GUCEF_TRACE;

    return m_periodSize;
}

/*-------------------------------------------------------------------------*/

CORE::UInt32
CWebAudioDevice::GetLatencyInFrames( void ) const
{GUCEF_TRACE;

    return 2 * m_periodSize;
}

/*-------------------------------------------------------------------------*/

const float*
CWebAudioDevice::Render( CORE::UInt32 frameCount )
{
    // no trace macro: this runs for every buffer the page plays

    if ( !m_started.Load() )
        return GUCEF_NULL;
    if ( frameCount > m_renderBuffer.GetFrameCount() && !m_renderBuffer.Resize( frameCount ) )
        return GUCEF_NULL;

    // whatever the callback does not write is heard as it is, so the buffer starts as silence
    m_renderBuffer.FillSilence( 0, frameCount );
    GetCallback()->OnRenderAudio( m_renderBuffer.GetDataPtr(), frameCount );
    return static_cast< const float* >( m_renderBuffer.GetDataPtr() );
}

/*-------------------------------------------------------------------------*/

CWebAudioDriver::CWebAudioDriver( void )
    : AUDIO::CAudioDriver()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CWebAudioDriver::~CWebAudioDriver()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CWebAudioDriver::IsAvailable( void )
{GUCEF_TRACE;

    return 0 != PageCall( PAGECALL_IS_AVAILABLE );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CWebAudioDriver::GetName( void ) const
{GUCEF_TRACE;

    return "webaudio";
}

/*-------------------------------------------------------------------------*/

CORE::CString
CWebAudioDriver::GetDescription( void ) const
{GUCEF_TRACE;

    return "The Web Audio API of the browser";
}

/*-------------------------------------------------------------------------*/

CORE::Int32
CWebAudioDriver::GetPriority( void ) const
{GUCEF_TRACE;

    return 10;
}

/*-------------------------------------------------------------------------*/

bool
CWebAudioDriver::EnumerateDevices( AUDIO::TAudioDirection direction, AUDIO::TAudioDeviceInfoVector& devices )
{GUCEF_TRACE;

    if ( AUDIO::AUDIODIRECTION_PLAYBACK != direction )
        return true;                // playback only for now

    AUDIO::CAudioDeviceInfo info( "default", "Browser audio output", AUDIO::AUDIODIRECTION_PLAYBACK );
    info.SetIsDefault( true );
    devices.push_back( info );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CWebAudioDriver::OpenDevice( const AUDIO::CAudioDeviceParams& params ,
                             AUDIO::IAudioDeviceCallback* callback   ,
                             AUDIO::TAudioDevicePtr& device          )
{GUCEF_TRACE;

    if ( AUDIO::AUDIODIRECTION_PLAYBACK != params.GetDirection() || GUCEF_NULL == callback )
        return false;

    const AUDIO::CAudioFormat& preferred = params.GetPreferredFormat();
    const CORE::UInt32 channels = preferred.IsValid() ? preferred.GetChannelCount() : 2;
    CORE::UInt32 bufferFrames = params.GetOptions().GetValueAlways( "bufferFrames" ).AsUInt32( DefaultBufferFrames );
    if ( !IsPowerOfTwo( bufferFrames ) || bufferFrames < 256 || bufferFrames > 16384 )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "WebAudioDriver: bufferFrames must be a power of 2 from 256 to 16384, using " << DefaultBufferFrames );
        bufferFrames = DefaultBufferFrames;
    }

    // The context decides the rate, so it is made first; the device made for it is then bound to it
    const int contextId = PageCall( PAGECALL_OPEN, preferred.IsValid() ? (int) preferred.GetSampleRate() : 0, (int) channels, (int) bufferFrames );
    if ( 0 == contextId )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "WebAudioDriver: The browser would not make an audio context" );
        return false;
    }

    const CORE::UInt32 sampleRate = (CORE::UInt32) PageCall( PAGECALL_SAMPLE_RATE, contextId );
    const AUDIO::CAudioFormat format( sampleRate, channels, AUDIO::SAMPLETYPE_FLOAT32 );
    AUDIO::CAudioDeviceInfo info( "default", "Browser audio output", AUDIO::AUDIODIRECTION_PLAYBACK );
    info.SetIsDefault( true );
    info.SetPreferredFormat( format );
    CWebAudioDevice* webDevice = GUCEF_NEW CWebAudioDevice( info, callback, format, bufferFrames, contextId );
    PageCall( PAGECALL_BIND, contextId, (int) (intptr_t) webDevice );

    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "WebAudioDriver: Opened the browser output at " << format.GetSampleRate() << " Hz, " <<
        format.GetChannelCount() << " channels, buffer " << bufferFrames << " frames" );

    device = AUDIO::TAudioDevicePtr( webDevice );
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEBAUDIO */
}; /* namespace AUDIODRIVER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
