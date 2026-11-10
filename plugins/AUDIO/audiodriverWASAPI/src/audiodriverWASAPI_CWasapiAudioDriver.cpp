/*
 *  audiodriverWASAPI: gucefAUDIO driver plugin for WASAPI on Windows
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

#include <stdio.h>
#include <string.h>

#include <windows.h>
#include <mmreg.h>
#include <objbase.h>
#include <mmdeviceapi.h>
#include <audioclient.h>

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CUTF16STRING_H
#include "gucefCORE_CUtf16String.h"
#define GUCEF_CORE_CUTF16STRING_H
#endif /* GUCEF_CORE_CUTF16STRING_H ? */

#include "audiodriverWASAPI_CWasapiAudioDriver.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUDIODRIVER {
namespace WASAPI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

// Defined here so no extra libraries or INITGUID tricks are needed for them
static const GUID SubtypePcm = { 0x00000001, 0x0000, 0x0010, { 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71 } };          // KSDATAFORMAT_SUBTYPE_PCM
static const GUID SubtypeIeeeFloat = { 0x00000003, 0x0000, 0x0010, { 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71 } };    // KSDATAFORMAT_SUBTYPE_IEEE_FLOAT
static const PROPERTYKEY FriendlyNameKey = { { 0xA45C254E, 0xDF1C, 0x4EFD, { 0x80, 0x20, 0x67, 0xD1, 0x46, 0xA8, 0x50, 0xE0 } }, 14 };    // PKEY_Device_FriendlyName

static const CORE::UInt32 DefaultBufferMs = 40;
static const DWORD EventWaitMs = 200;
static const CORE::UInt32 MaxEventTimeouts = 10;     // waits in a row without room before the endpoint counts as lost

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** Joins the multithreaded apartment for a scope; a thread that is in a single threaded one already can use COM as well */
class CComScope
{
    public:

    CComScope( void ) : m_result( ::CoInitializeEx( NULL, COINIT_MULTITHREADED ) ) {}
    ~CComScope() { if ( SUCCEEDED( m_result ) ) ::CoUninitialize(); }
    bool IsUsable( void ) const { return SUCCEEDED( m_result ) || RPC_E_CHANGED_MODE == m_result; }

    private:

    HRESULT m_result;
};

/*-------------------------------------------------------------------------*/

template < class T >
void
SafeRelease( T*& object )
{
    if ( NULL != object )
    {
        object->Release();
        object = NULL;
    }
}

/*-------------------------------------------------------------------------*/

CORE::CString
HResultText( HRESULT result )
{
    char text[ 32 ];
    snprintf( text, sizeof( text ), "HRESULT 0x%08lX", (unsigned long) result );
    return text;
}

/*-------------------------------------------------------------------------*/

/** Takes a string COM allocated */
CORE::CString
TakeComString( LPWSTR text )
{
    if ( NULL == text )
        return CORE::CString::Empty;
    CORE::CString result = CORE::ToString( (const wchar_t*) text );
    ::CoTaskMemFree( text );
    return result;
}

/*-------------------------------------------------------------------------*/

CORE::CString
GetEndpointId( IMMDevice* endpoint )
{
    LPWSTR id = NULL;
    if ( FAILED( endpoint->GetId( &id ) ) )
        return CORE::CString::Empty;
    return TakeComString( id );
}

/*-------------------------------------------------------------------------*/

CORE::CString
GetEndpointName( IMMDevice* endpoint )
{
    CORE::CString name;
    IPropertyStore* properties = NULL;
    if ( SUCCEEDED( endpoint->OpenPropertyStore( STGM_READ, &properties ) ) )
    {
        PROPVARIANT value;
        ::PropVariantInit( &value );
        if ( SUCCEEDED( properties->GetValue( FriendlyNameKey, &value ) ) && VT_LPWSTR == value.vt )
            name = CORE::ToString( (const wchar_t*) value.pwszVal );
        ::PropVariantClear( &value );
        SafeRelease( properties );
    }
    return name;
}

/*-------------------------------------------------------------------------*/

IMMDeviceEnumerator*
CreateEnumerator( void )
{
    IMMDeviceEnumerator* enumerator = NULL;
    HRESULT result = ::CoCreateInstance( __uuidof( MMDeviceEnumerator ), NULL, CLSCTX_ALL, __uuidof( IMMDeviceEnumerator ), (void**) &enumerator );
    if ( FAILED( result ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "WasapiAudioDriver: No device enumerator: " << HResultText( result ) );
        return NULL;
    }
    return enumerator;
}

/*-------------------------------------------------------------------------*/

/** The wave format WASAPI is asked for. Sample types it has no form for are asked as float, the engine converts */
AUDIO::CAudioFormat
ToWaveFormat( const AUDIO::CAudioFormat& format, WAVEFORMATEXTENSIBLE& wave )
{
    AUDIO::TSampleType sampleType = format.GetSampleType();
    if ( AUDIO::SAMPLETYPE_INT16 != sampleType && AUDIO::SAMPLETYPE_INT24 != sampleType && AUDIO::SAMPLETYPE_INT32 != sampleType )
        sampleType = AUDIO::SAMPLETYPE_FLOAT32;
    const AUDIO::CAudioFormat result( format.GetSampleRate(), format.GetChannelCount(), sampleType, format.GetChannelLayout() );

    memset( &wave, 0, sizeof( wave ) );
    wave.Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
    wave.Format.nChannels = (WORD) result.GetChannelCount();
    wave.Format.nSamplesPerSec = (DWORD) result.GetSampleRate();
    wave.Format.wBitsPerSample = (WORD) ( result.GetBytesPerSample() * 8 );
    wave.Format.nBlockAlign = (WORD) result.GetBytesPerFrame();
    wave.Format.nAvgBytesPerSec = wave.Format.nSamplesPerSec * wave.Format.nBlockAlign;
    wave.Format.cbSize = sizeof( WAVEFORMATEXTENSIBLE ) - sizeof( WAVEFORMATEX );
    wave.Samples.wValidBitsPerSample = wave.Format.wBitsPerSample;
    wave.dwChannelMask = (DWORD) result.GetChannelLayout();     // the speaker bits of gucefAUDIO are those of WAVE
    wave.SubFormat = AUDIO::SAMPLETYPE_FLOAT32 == sampleType ? SubtypeIeeeFloat : SubtypePcm;
    return result;
}

/*-------------------------------------------------------------------------*/

/** The gucefAUDIO format of a wave format WASAPI gave, invalid when it has none */
AUDIO::CAudioFormat
FromWaveFormat( const WAVEFORMATEX& wave )
{
    bool isFloat = WAVE_FORMAT_IEEE_FLOAT == wave.wFormatTag;
    UInt32 layout = 0;
    if ( WAVE_FORMAT_EXTENSIBLE == wave.wFormatTag && wave.cbSize >= sizeof( WAVEFORMATEXTENSIBLE ) - sizeof( WAVEFORMATEX ) )
    {
        const WAVEFORMATEXTENSIBLE& extensible = reinterpret_cast< const WAVEFORMATEXTENSIBLE& >( wave );
        isFloat = 0 == memcmp( &extensible.SubFormat, &SubtypeIeeeFloat, sizeof( GUID ) );
        layout = extensible.dwChannelMask;
    }

    AUDIO::TSampleType sampleType = AUDIO::SAMPLETYPE_UNKNOWN;
    switch ( wave.wBitsPerSample )
    {
        case 16: sampleType = isFloat ? AUDIO::SAMPLETYPE_UNKNOWN : AUDIO::SAMPLETYPE_INT16; break;
        case 24: sampleType = isFloat ? AUDIO::SAMPLETYPE_UNKNOWN : AUDIO::SAMPLETYPE_INT24; break;
        case 32: sampleType = isFloat ? AUDIO::SAMPLETYPE_FLOAT32 : AUDIO::SAMPLETYPE_INT32; break;
        case 64: sampleType = isFloat ? AUDIO::SAMPLETYPE_FLOAT64 : AUDIO::SAMPLETYPE_UNKNOWN; break;
        default: break;
    }
    if ( AUDIO::SAMPLETYPE_UNKNOWN == sampleType )
        return AUDIO::CAudioFormat();
    return AUDIO::CAudioFormat( wave.nSamplesPerSec, wave.nChannels, sampleType, layout );
}

/*-------------------------------------------------------------------------*/

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CWasapiAudioDevice::CWasapiAudioDevice( const AUDIO::CAudioDeviceInfo& info   ,
                                        AUDIO::IAudioDeviceCallback* callback ,
                                        const AUDIO::CAudioFormat& format     ,
                                        CORE::UInt32 periodSizeInFrames       ,
                                        CORE::UInt32 bufferSizeInFrames       ,
                                        IAudioClient* client                  ,
                                        IAudioRenderClient* renderClient      ,
                                        void* bufferEvent                     )
    : AUDIO::CAudioPushDevice( info, callback, format, periodSizeInFrames )
    , m_client( client )
    , m_renderClient( renderClient )
    , m_bufferEvent( bufferEvent )
    , m_bufferSizeInFrames( bufferSizeInFrames )
    , m_bytesPerFrame( format.GetBytesPerFrame() )
    , m_comInitializedOnDeviceThread( false )
    , m_hasWritten( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CWasapiAudioDevice::~CWasapiAudioDevice()
{GUCEF_TRACE;

    // stopped here and not in the base class: the hooks of this class are gone by the time that destructor runs
    Stop();
    SafeRelease( m_renderClient );
    SafeRelease( m_client );
    if ( NULL != m_bufferEvent )
    {
        ::CloseHandle( (HANDLE) m_bufferEvent );
        m_bufferEvent = NULL;
    }
}

/*-------------------------------------------------------------------------*/

CORE::UInt32
CWasapiAudioDevice::GetLatencyInFrames( void ) const
{GUCEF_TRACE;

    return m_bufferSizeInFrames;
}

/*-------------------------------------------------------------------------*/

bool
CWasapiAudioDevice::OnPushStart( void )
{GUCEF_TRACE;

    m_hasWritten = false;
    const HRESULT result = m_client->Start();
    if ( FAILED( result ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "WasapiAudioDevice: Could not start \"" << GetDeviceInfo().GetName() << "\": " << HResultText( result ) );
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CWasapiAudioDevice::OnDeviceThreadStart( void )
{GUCEF_TRACE;

    const HRESULT result = ::CoInitializeEx( NULL, COINIT_MULTITHREADED );
    m_comInitializedOnDeviceThread = SUCCEEDED( result );
    return SUCCEEDED( result ) || RPC_E_CHANGED_MODE == result;
}

/*-------------------------------------------------------------------------*/

AUDIO::CAudioPushDevice::TPushResult
CWasapiAudioDevice::PushFrames( const void* frames, CORE::UInt32 frameCount )
{
    // no trace macro: this runs once per period on the device thread

    const CORE::UInt8* data = static_cast< const CORE::UInt8* >( frames );
    CORE::UInt32 remaining = frameCount;
    CORE::UInt32 timeouts = 0;
    bool underrun = false;
    while ( remaining > 0 )
    {
        UINT32 padding = 0;
        HRESULT result = m_client->GetCurrentPadding( &padding );
        if ( FAILED( result ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "WasapiAudioDevice: \"" << GetDeviceInfo().GetName() << "\" is lost: " << HResultText( result ) );
            return PUSHRESULT_DEVICE_LOST;
        }

        // nothing left to play while we come with more: it ran dry in between, which was heard as a gap
        if ( m_hasWritten && 0 == padding )
            underrun = true;

        const CORE::UInt32 room = m_bufferSizeInFrames > padding ? m_bufferSizeInFrames - padding : 0;
        if ( 0 == room )
        {
            if ( WAIT_TIMEOUT == ::WaitForSingleObject( (HANDLE) m_bufferEvent, EventWaitMs ) && ++timeouts >= MaxEventTimeouts )
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "WasapiAudioDevice: \"" << GetDeviceInfo().GetName() << "\" stopped taking audio" );
                return PUSHRESULT_DEVICE_LOST;
            }
            continue;
        }
        timeouts = 0;

        const CORE::UInt32 count = room < remaining ? room : remaining;
        BYTE* destination = NULL;
        result = m_renderClient->GetBuffer( count, &destination );
        if ( FAILED( result ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "WasapiAudioDevice: \"" << GetDeviceInfo().GetName() << "\" is lost: " << HResultText( result ) );
            return PUSHRESULT_DEVICE_LOST;
        }
        memcpy( destination, data, (size_t) count * m_bytesPerFrame );
        m_renderClient->ReleaseBuffer( count, 0 );

        data += (size_t) count * m_bytesPerFrame;
        remaining -= count;
        m_hasWritten = true;
    }
    return underrun ? PUSHRESULT_UNDERRUN_RECOVERED : PUSHRESULT_OK;
}

/*-------------------------------------------------------------------------*/

void
CWasapiAudioDevice::InterruptPush( void )
{GUCEF_TRACE;

    // wakes a wait for room, the device thread then sees the stop request
    ::SetEvent( (HANDLE) m_bufferEvent );
}

/*-------------------------------------------------------------------------*/

void
CWasapiAudioDevice::OnDeviceThreadEnd( void )
{GUCEF_TRACE;

    if ( m_comInitializedOnDeviceThread )
    {
        ::CoUninitialize();
        m_comInitializedOnDeviceThread = false;
    }
}

/*-------------------------------------------------------------------------*/

void
CWasapiAudioDevice::OnPushStop( void )
{GUCEF_TRACE;

    m_client->Stop();
    m_client->Reset();
}

/*-------------------------------------------------------------------------*/

CWasapiAudioDriver::CWasapiAudioDriver( void )
    : AUDIO::CAudioDriver()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CWasapiAudioDriver::~CWasapiAudioDriver()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CWasapiAudioDriver::IsAvailable( void ) const
{GUCEF_TRACE;

    CComScope com;
    if ( !com.IsUsable() )
        return false;
    IMMDeviceEnumerator* enumerator = CreateEnumerator();
    const bool available = NULL != enumerator;
    SafeRelease( enumerator );
    return available;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CWasapiAudioDriver::GetName( void ) const
{GUCEF_TRACE;

    return "wasapi";
}

/*-------------------------------------------------------------------------*/

CORE::CString
CWasapiAudioDriver::GetDescription( void ) const
{GUCEF_TRACE;

    return "WASAPI, the audio interface of Windows Vista and newer, in shared mode";
}

/*-------------------------------------------------------------------------*/

CORE::Int32
CWasapiAudioDriver::GetPriority( void ) const
{GUCEF_TRACE;

    return 10;
}

/*-------------------------------------------------------------------------*/

bool
CWasapiAudioDriver::EnumerateDevices( AUDIO::TAudioDirection direction, AUDIO::TAudioDeviceInfoVector& devices )
{GUCEF_TRACE;

    if ( AUDIO::AUDIODIRECTION_PLAYBACK != direction )
        return true;                // playback only for now

    CComScope com;
    if ( !com.IsUsable() )
        return false;
    IMMDeviceEnumerator* enumerator = CreateEnumerator();
    if ( NULL == enumerator )
        return false;

    CORE::CString defaultId;
    IMMDevice* defaultEndpoint = NULL;
    if ( SUCCEEDED( enumerator->GetDefaultAudioEndpoint( eRender, eConsole, &defaultEndpoint ) ) )
    {
        defaultId = GetEndpointId( defaultEndpoint );
        SafeRelease( defaultEndpoint );
    }

    IMMDeviceCollection* endpoints = NULL;
    if ( SUCCEEDED( enumerator->EnumAudioEndpoints( eRender, DEVICE_STATE_ACTIVE, &endpoints ) ) )
    {
        UINT count = 0;
        endpoints->GetCount( &count );
        for ( UINT i=0; i<count; ++i )
        {
            IMMDevice* endpoint = NULL;
            if ( FAILED( endpoints->Item( i, &endpoint ) ) )
                continue;

            const CORE::CString id = GetEndpointId( endpoint );
            const CORE::CString name = GetEndpointName( endpoint );
            AUDIO::CAudioDeviceInfo info( id, name.IsNULLOrEmpty() ? id : name, AUDIO::AUDIODIRECTION_PLAYBACK );
            info.SetIsDefault( !id.IsNULLOrEmpty() && id == defaultId );
            devices.push_back( info );
            SafeRelease( endpoint );
        }
        SafeRelease( endpoints );
    }
    SafeRelease( enumerator );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CWasapiAudioDriver::OpenDevice( const AUDIO::CAudioDeviceParams& params ,
                                AUDIO::IAudioDeviceCallback* callback   ,
                                AUDIO::TAudioDevicePtr& device          )
{GUCEF_TRACE;

    if ( AUDIO::AUDIODIRECTION_PLAYBACK != params.GetDirection() || GUCEF_NULL == callback )
        return false;
    if ( AUDIO::AUDIOSHAREMODE_EXCLUSIVE == params.GetShareMode() )
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "WasapiAudioDriver: Exclusive mode is not offered yet, the endpoint is opened shared" );

    // The objects made here live on in the device. WASAPI objects can be used from any thread of the process, and the
    // device thread keeps itself in the multithreaded apartment while it runs
    CComScope com;
    if ( !com.IsUsable() )
        return false;
    IMMDeviceEnumerator* enumerator = CreateEnumerator();
    if ( NULL == enumerator )
        return false;

    const bool useDefault = params.GetDeviceId().IsNULLOrEmpty() || "default" == params.GetDeviceId();
    IMMDevice* endpoint = NULL;
    HRESULT result = S_OK;
    if ( useDefault )
    {
        result = enumerator->GetDefaultAudioEndpoint( eRender, eConsole, &endpoint );
    }
    else
    {
        const CORE::CUtf16String wideId( params.GetDeviceId() );
        result = enumerator->GetDevice( reinterpret_cast< LPCWSTR >( wideId.C_String() ), &endpoint );
    }
    SafeRelease( enumerator );
    if ( FAILED( result ) || NULL == endpoint )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "WasapiAudioDriver: No endpoint \"" << ( useDefault ? CORE::CString( "default" ) : params.GetDeviceId() ) << "\": " << HResultText( result ) );
        return false;
    }

    const CORE::CString endpointId = GetEndpointId( endpoint );
    const CORE::CString endpointName = GetEndpointName( endpoint );

    IAudioClient* client = NULL;
    result = endpoint->Activate( __uuidof( IAudioClient ), CLSCTX_ALL, NULL, (void**) &client );
    SafeRelease( endpoint );
    if ( FAILED( result ) || NULL == client )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "WasapiAudioDriver: Could not activate \"" << endpointName << "\": " << HResultText( result ) );
        return false;
    }

    // What the endpoint mixes in fills in what was not asked for
    WAVEFORMATEX* mixWave = NULL;
    AUDIO::CAudioFormat mixFormat;
    if ( SUCCEEDED( client->GetMixFormat( &mixWave ) ) && NULL != mixWave )
    {
        mixFormat = FromWaveFormat( *mixWave );
        ::CoTaskMemFree( mixWave );
    }
    AUDIO::CAudioFormat wanted = params.GetPreferredFormat();
    if ( !wanted.IsValid() )
        wanted = mixFormat.IsValid() ? mixFormat : AUDIO::CAudioFormat( 48000, 2, AUDIO::SAMPLETYPE_FLOAT32 );

    const CORE::UInt32 bufferMs = params.GetOptions().GetValueAlways( "bufferMs" ).AsUInt32( DefaultBufferMs );
    const REFERENCE_TIME bufferDuration = (REFERENCE_TIME) bufferMs * 10000;
    const DWORD flags = AUDCLNT_STREAMFLAGS_EVENTCALLBACK | AUDCLNT_STREAMFLAGS_AUTOCONVERTPCM | AUDCLNT_STREAMFLAGS_SRC_DEFAULT_QUALITY;

    WAVEFORMATEXTENSIBLE wave;
    AUDIO::CAudioFormat format = ToWaveFormat( wanted, wave );
    result = client->Initialize( AUDCLNT_SHAREMODE_SHARED, flags, bufferDuration, 0, &wave.Format, NULL );
    if ( FAILED( result ) && mixFormat.IsValid() && !( mixFormat == format ) )
    {
        // Windows before 7 SP1 has no conversion: then the mix format is the only one that works
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "WasapiAudioDriver: \"" << endpointName << "\" refused " << format.ToString() << " (" << HResultText( result ) << "), using its mix format" );
        format = ToWaveFormat( mixFormat, wave );
        result = client->Initialize( AUDCLNT_SHAREMODE_SHARED, AUDCLNT_STREAMFLAGS_EVENTCALLBACK, bufferDuration, 0, &wave.Format, NULL );
    }
    if ( FAILED( result ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "WasapiAudioDriver: Could not initialize \"" << endpointName << "\": " << HResultText( result ) );
        SafeRelease( client );
        return false;
    }

    UINT32 bufferSize = 0;
    client->GetBufferSize( &bufferSize );
    REFERENCE_TIME defaultPeriod = 0, minimumPeriod = 0;
    client->GetDevicePeriod( &defaultPeriod, &minimumPeriod );
    CORE::UInt32 periodSize = (CORE::UInt32) ( (CORE::UInt64) format.GetSampleRate() * (CORE::UInt64) defaultPeriod / 10000000 );
    if ( periodSize < 64 )
        periodSize = 64;
    if ( bufferSize >= 128 && periodSize > bufferSize / 2 )
        periodSize = bufferSize / 2;

    HANDLE bufferEvent = ::CreateEventW( NULL, FALSE, FALSE, NULL );
    IAudioRenderClient* renderClient = NULL;
    if ( NULL == bufferEvent || FAILED( client->SetEventHandle( bufferEvent ) ) ||
         FAILED( client->GetService( __uuidof( IAudioRenderClient ), (void**) &renderClient ) ) || NULL == renderClient )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "WasapiAudioDriver: Could not set up buffering for \"" << endpointName << "\"" );
        if ( NULL != bufferEvent )
            ::CloseHandle( bufferEvent );
        SafeRelease( client );
        return false;
    }

    AUDIO::CAudioDeviceInfo info( endpointId, endpointName.IsNULLOrEmpty() ? endpointId : endpointName, AUDIO::AUDIODIRECTION_PLAYBACK );
    info.SetIsDefault( useDefault );
    info.SetPreferredFormat( format );

    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "WasapiAudioDriver: Opened \"" << info.GetName() << "\" at " << format.GetSampleRate() << " Hz, " <<
        format.GetChannelCount() << " channels, sample type " << (CORE::UInt32) format.GetSampleType() << ", period " << periodSize << " frames, buffer " << bufferSize << " frames" );

    device = AUDIO::TAudioDevicePtr( GUCEF_NEW CWasapiAudioDevice( info, callback, format, periodSize, bufferSize, client, renderClient, bufferEvent ) );
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WASAPI */
}; /* namespace AUDIODRIVER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
