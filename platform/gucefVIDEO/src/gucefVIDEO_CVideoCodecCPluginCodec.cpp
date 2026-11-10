/*
 *  gucefVIDEO: GUCEF module providing video decoding and playback
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

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_CORE_CTVARIANTMAPADAPTER_CPP2C_H
#include "gucefCORE_CTVariantMapAdapter_Cpp2C.h"
#define GUCEF_CORE_CTVARIANTMAPADAPTER_CPP2C_H
#endif /* GUCEF_CORE_CTVARIANTMAPADAPTER_CPP2C_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_AUDIO_CAUDIOCONVERTER_H
#include "gucefAUDIO_CAudioConverter.h"
#define GUCEF_AUDIO_CAUDIOCONVERTER_H
#endif /* GUCEF_AUDIO_CAUDIOCONVERTER_H ? */

#ifndef GUCEF_AUDIO_CLINEARRESAMPLER_H
#include "gucefAUDIO_CLinearResampler.h"
#define GUCEF_AUDIO_CLINEARRESAMPLER_H
#endif /* GUCEF_AUDIO_CLINEARRESAMPLER_H ? */

#include "gucefVIDEO_CVideoCodecCPluginCodec.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace VIDEO {

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

typedef CORE::CTVariantMapAdapterCpp2C< CString, CString > TStringMapAdapter;
typedef TStringMapAdapter::TLinkedMapType TStringMap;

/*-------------------------------------------------------------------------*/

/**
 *  The codec state a plugin keeps per codec type ('codecdata'). Shared by the codec and the sources it opened,
 *  so it lives until the last of them is gone, or until the plugin is unlinked.
 */
class CVideoCodecCPluginInstance : public CIVideoCodecCPluginClient
{
    public:

    typedef CVideoCodecCPlugin::TVideoCodecCPluginPtr TVideoCodecCPluginPtr;

    CVideoCodecCPluginInstance( const TVideoCodecCPluginPtr& plugin )
        : CIVideoCodecCPluginClient()
        , m_plugin( plugin )
        , m_codecdata( GUCEF_NULL )
        , m_isCreated( false )
    {GUCEF_TRACE;
    }

    virtual ~CVideoCodecCPluginInstance() GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        MT::CScopeMutex lock( m_plugin->GetCallLock() );
        m_plugin->UnregisterClient( this );
        const TGucefVideoCodecPluginApi* api = m_plugin->GetLinkedApi();
        if ( m_isCreated && GUCEF_NULL != api )
            api->Destroy_Codec( m_plugin->GetPlugData(), &m_codecdata );
        m_isCreated = false;
    }

    /** Call with the call lock held */
    bool Create( const CString& codecType )
    {GUCEF_TRACE;

        const TGucefVideoCodecPluginApi* api = m_plugin->GetLinkedApi();
        if ( GUCEF_NULL == api )
            return false;

        if ( 0 == api->Create_Codec( m_plugin->GetPlugData(), codecType.C_String(), &m_codecdata ) )
            return false;

        m_isCreated = true;
        m_plugin->RegisterClient( this, false );
        return true;
    }

    /** True while the plugin can be called for this instance. Call with the call lock held */
    bool IsUsable( void ) const
    {GUCEF_TRACE;

        return m_isCreated && GUCEF_NULL != m_plugin->GetLinkedApi();
    }

    void** GetCodecData( void )
    {GUCEF_TRACE;

        return &m_codecdata;
    }

    virtual void OnPluginUnlink( const TGucefVideoCodecPluginApi& api, void** plugdata ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        if ( m_isCreated )
            api.Destroy_Codec( plugdata, &m_codecdata );
        m_isCreated = false;
        m_codecdata = GUCEF_NULL;
    }

    CVideoCodecCPlugin& Plugin( void )
    {GUCEF_TRACE;

        return *m_plugin;
    }

    private:

    TVideoCodecCPluginPtr m_plugin;
    void* m_codecdata;
    bool m_isCreated;
};

/*-------------------------------------------------------------------------*/

namespace {

/*-------------------------------------------------------------------------*/

/** Options as the plugin reads them: text keys and text values */
void
OptionsToStringMap( const CORE::CValueList& options, TStringMap& map )
{GUCEF_TRACE;

    UInt32 keyCount = options.GetKeyCount();
    for ( UInt32 i=0; i<keyCount; ++i )
        map[ options.GetKey( i ).AsString() ] = options.GetValue( i ).AsString();
}

/*-------------------------------------------------------------------------*/

IMAGE::TPixelStorageFormat
PixelFormatFromInfo( UInt32 pixelFormat )
{GUCEF_TRACE;

    switch ( pixelFormat )
    {
        case GUCEF_VIDEOCODEC_PIXELFORMAT_PALETTE8: return IMAGE::PSF_PALETTE_INDICES;
        case GUCEF_VIDEOCODEC_PIXELFORMAT_RGB24: return IMAGE::PSF_RGB;
        case GUCEF_VIDEOCODEC_PIXELFORMAT_RGBA32: return IMAGE::PSF_RGBA;
        case GUCEF_VIDEOCODEC_PIXELFORMAT_BGRA32: return IMAGE::PSF_BGRA;
        default: return IMAGE::PSF_UNKNOWN;
    }
}

/*-------------------------------------------------------------------------*/

bool
AudioFormatFromInfo( const TAudioFormatInfo& info, AUDIO::CAudioFormat& format )
{GUCEF_TRACE;

    if ( info.sampleType < (UInt32) AUDIO::SAMPLETYPE_UINT8 || info.sampleType > (UInt32) AUDIO::SAMPLETYPE_FLOAT64 )
        return false;

    format = AUDIO::CAudioFormat( info.sampleRate, info.channelCount, (AUDIO::TSampleType) info.sampleType, info.channelLayout );
    return format.IsValid();
}

/*-------------------------------------------------------------------------*/

void GUCEF_PLUGIN_CALLSPEC_PREFIX
OnPluginAudio( void* userData, UInt32 track, const void* frames, UInt32 frameCount ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    IVideoAudioSink* sink = static_cast< IVideoAudioSink* >( userData );
    if ( GUCEF_NULL != sink && GUCEF_NULL != frames && frameCount > 0 )
        sink->OnVideoAudio( track, frames, frameCount );
}

/*-------------------------------------------------------------------------*/

/** A video the plugin decodes ('sourcedata') */
class CVideoCodecCPluginSource : public IVideoSource ,
                                 public CIVideoCodecCPluginClient
{
    public:

    typedef CVideoCodecCPluginCodec::TCodecInstancePtr TCodecInstancePtr;
    typedef CIVideoCodec::TIOAccessPtr TIOAccessPtr;
    typedef GUCEF::vector< AUDIO::CAudioFormat > TAudioFormatVector;

    CVideoCodecCPluginSource( const TCodecInstancePtr& instance, CORE::CIOAccess* input, const TIOAccessPtr& owner )
        : IVideoSource()
        , CIVideoCodecCPluginClient()
        , m_instance( instance )
        , m_input( input )
        , m_inputOwner( owner )
        , m_sourcedata( GUCEF_NULL )
        , m_isOpen( false )
        , m_isEndOfStream( false )
        , m_hasFailed( false )
        , m_canSeek( false )
        , m_format()
        , m_audioFormats()
        , m_position( 0 )
    {GUCEF_TRACE;
    }

    virtual ~CVideoCodecCPluginSource() GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        CVideoCodecCPlugin& plugin = m_instance->Plugin();
        MT::CScopeMutex lock( plugin.GetCallLock() );
        plugin.UnregisterClient( this );
        if ( m_isOpen && m_instance->IsUsable() )
            plugin.GetLinkedApi()->Src_Close( plugin.GetPlugData(), m_instance->GetCodecData(), &m_sourcedata );
        m_isOpen = false;
    }

    bool Open( const CORE::CValueList& options )
    {GUCEF_TRACE;

        CVideoCodecCPlugin& plugin = m_instance->Plugin();
        MT::CScopeMutex lock( plugin.GetCallLock() );
        if ( !m_instance->IsUsable() )
            return false;

        TStringMap optionMap;
        OptionsToStringMap( options, optionMap );
        TStringMapAdapter optionAdapter;
        optionAdapter.SetMapAccessPtr( (const TStringMap*) &optionMap );

        TVideoFormatInfo info;
        memset( &info, 0, sizeof( info ) );
        UInt64 frameCount = GUCEF_VIDEOCODEC_FRAMECOUNT_UNKNOWN;
        const TGucefVideoCodecPluginApi* api = plugin.GetLinkedApi();
        if ( 0 == api->Src_Open( plugin.GetPlugData(), m_instance->GetCodecData(), &m_sourcedata, m_input->CStyleAccess(), optionAdapter.GetCStyleAccess(), &info, &frameCount ) )
            return false;

        m_isOpen = true;
        m_format = CVideoFormat( info.width, info.height, PixelFormatFromInfo( info.pixelFormat ), info.frameRateNumerator, info.frameRateDenominator,
                                 GUCEF_VIDEOCODEC_FRAMECOUNT_UNKNOWN == frameCount ? GUCEF_VIDEO_FRAMECOUNT_UNKNOWN : frameCount );
        bool valid = m_format.IsValid();
        for ( UInt32 track=0; valid && track<info.audioTrackCount; ++track )
        {
            TAudioFormatInfo audioInfo;
            memset( &audioInfo, 0, sizeof( audioInfo ) );
            AUDIO::CAudioFormat audioFormat;
            valid = 0 != api->Src_Get_Audio_Track_Format( plugin.GetPlugData(), m_instance->GetCodecData(), &m_sourcedata, track, &audioInfo ) &&
                    AudioFormatFromInfo( audioInfo, audioFormat );
            m_audioFormats.push_back( audioFormat );
        }
        if ( !valid )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "VideoCodecCPluginSource: The plugin \"" << plugin.GetName() << "\" opened a video with an invalid format" );
            api->Src_Close( plugin.GetPlugData(), m_instance->GetCodecData(), &m_sourcedata );
            m_isOpen = false;
            return false;
        }

        m_canSeek = GUCEF_NULL != api->Src_Seek_Frame;
        plugin.RegisterClient( this, true );
        return true;
    }

    virtual const CVideoFormat& GetFormat( void ) const GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        return m_format;
    }

    virtual UInt32 GetAudioTrackCount( void ) const GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        return (UInt32) m_audioFormats.size();
    }

    virtual bool GetAudioTrackFormat( UInt32 track, AUDIO::CAudioFormat& format ) const GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        if ( track >= m_audioFormats.size() )
            return false;
        format = m_audioFormats[ track ];
        return true;
    }

    virtual bool DecodeNextFrame( CVideoFrame& frame, IVideoAudioSink* audioSink ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        if ( m_isEndOfStream || !frame.Prepare( m_format ) )
            return false;

        CVideoCodecCPlugin& plugin = m_instance->Plugin();
        MT::CScopeMutex lock( plugin.GetCallLock() );
        if ( !m_isOpen || !m_instance->IsUsable() )
        {
            m_isEndOfStream = true;
            return false;
        }

        TVideoPictureBuffer picture;
        picture.pixels = frame.GetPicture().GetDataPtr();
        picture.pitch = frame.GetPitch();
        picture.palette = m_format.IsPalettized() ? frame.GetPalette() : GUCEF_NULL;
        picture.paletteChanged = 0;

        TVideoAudioOutput audio;
        audio.onAudio = &OnPluginAudio;
        audio.userData = audioSink;

        UInt32 endOfStream = 0;
        if ( 0 == plugin.GetLinkedApi()->Src_Decode_Frame( plugin.GetPlugData(), m_instance->GetCodecData(), &m_sourcedata, &picture, GUCEF_NULL != audioSink ? &audio : GUCEF_NULL, &endOfStream ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "VideoCodecCPluginSource: The plugin \"" << plugin.GetName() << "\" failed to decode frame " << m_position << ", ending the video" );
            m_isEndOfStream = true;
            m_hasFailed = true;
            return false;
        }
        if ( 0 != endOfStream )
        {
            m_isEndOfStream = true;
            return false;
        }

        frame.SetFrameIndex( m_position );
        frame.SetTimeInMicroSeconds( m_format.GetFrameTimeInMicroSeconds( m_position ) );
        frame.SetPaletteChanged( 0 != picture.paletteChanged );
        ++m_position;
        return true;
    }

    virtual bool IsEndOfStream( void ) const GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        return m_isEndOfStream;
    }

    virtual bool HasFailed( void ) const GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        return m_hasFailed;
    }

    virtual UInt64 GetPositionInFrames( void ) const GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        return m_position;
    }

    virtual bool IsSeekable( void ) const GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        return m_canSeek;
    }

    virtual bool SeekToFrame( UInt64 frame ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        if ( !m_canSeek || ( GUCEF_VIDEO_FRAMECOUNT_UNKNOWN != m_format.GetFrameCount() && frame > m_format.GetFrameCount() ) )
            return false;

        CVideoCodecCPlugin& plugin = m_instance->Plugin();
        MT::CScopeMutex lock( plugin.GetCallLock() );
        if ( !m_isOpen || !m_instance->IsUsable() )
            return false;

        if ( 0 == plugin.GetLinkedApi()->Src_Seek_Frame( plugin.GetPlugData(), m_instance->GetCodecData(), &m_sourcedata, frame ) )
            return false;

        m_position = frame;
        m_hasFailed = false;
        m_isEndOfStream = GUCEF_VIDEO_FRAMECOUNT_UNKNOWN != m_format.GetFrameCount() && frame >= m_format.GetFrameCount();
        return true;
    }

    virtual void OnPluginUnlink( const TGucefVideoCodecPluginApi& api, void** plugdata ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        if ( m_isOpen )
            api.Src_Close( plugdata, m_instance->GetCodecData(), &m_sourcedata );
        m_isOpen = false;
        m_isEndOfStream = true;
    }

    private:

    TCodecInstancePtr m_instance;
    CORE::CIOAccess* m_input;
    TIOAccessPtr m_inputOwner;
    void* m_sourcedata;
    bool m_isOpen;
    bool m_isEndOfStream;
    bool m_hasFailed;
    bool m_canSeek;
    CVideoFormat m_format;
    TAudioFormatVector m_audioFormats;
    UInt64 m_position;
};

/*-------------------------------------------------------------------------*/

UInt32
PixelFormatToInfo( IMAGE::TPixelStorageFormat pixelFormat )
{GUCEF_TRACE;

    switch ( pixelFormat )
    {
        case IMAGE::PSF_PALETTE_INDICES: return GUCEF_VIDEOCODEC_PIXELFORMAT_PALETTE8;
        case IMAGE::PSF_RGB: return GUCEF_VIDEOCODEC_PIXELFORMAT_RGB24;
        case IMAGE::PSF_RGBA: return GUCEF_VIDEOCODEC_PIXELFORMAT_RGBA32;
        case IMAGE::PSF_BGRA: return GUCEF_VIDEOCODEC_PIXELFORMAT_BGRA32;
        default: return GUCEF_VIDEOCODEC_PIXELFORMAT_UNKNOWN;
    }
}

/*-------------------------------------------------------------------------*/

void
AudioFormatToInfo( const AUDIO::CAudioFormat& format, TAudioFormatInfo& info )
{GUCEF_TRACE;

    memset( &info, 0, sizeof( info ) );
    info.sampleRate = format.GetSampleRate();
    info.channelCount = format.GetChannelCount();
    info.sampleType = (UInt32) format.GetSampleType();
    info.channelLayout = format.GetChannelLayout();
}

/*-------------------------------------------------------------------------*/

/**
 *  The sound of a track on its way to the plugin: from the format the sink was opened with to the one the plugin
 *  takes, as a stream, so the resampler keeps its place from one piece to the next
 */
class CSinkAudioTrack
{
    public:

    AUDIO::CAudioFormat input;
    AUDIO::CAudioFormat output;
    GUCEF::vector< Float32 > matrix;            /**< the channel mix when the channels differ, else empty */
    AUDIO::CLinearResampler resampler;
    bool resample;
    GUCEF::vector< Float32 > pending;           /**< input frames (in the output channels) the resampler did not take yet */

    CSinkAudioTrack( void ) : input(), output(), matrix(), resampler(), resample( false ), pending() {}

    bool Setup( const AUDIO::CAudioFormat& in, const AUDIO::CAudioFormat& out )
    {GUCEF_TRACE;

        input = in;
        output = out;
        matrix.clear();
        if ( in.GetChannelCount() != out.GetChannelCount() || in.GetChannelLayout() != out.GetChannelLayout() )
        {
            if ( !AUDIO::CAudioConverter::BuildChannelMatrix( in, out, matrix ) )
                return false;
        }
        resample = in.GetSampleRate() != out.GetSampleRate();
        if ( resample && !resampler.Configure( out.GetChannelCount(), (Float64) in.GetSampleRate(), (Float64) out.GetSampleRate() ) )
            return false;
        return true;
    }

    /** The sound converted to the output format, as far as the resampler gives it now */
    void Convert( const void* frames, UInt32 frameCount, GUCEF::vector< UInt8 >& converted, UInt32& convertedFrames )
    {GUCEF_TRACE;

        const UInt32 inChannels = input.GetChannelCount(), outChannels = output.GetChannelCount();
        GUCEF::vector< Float32 > work( (size_t) frameCount * inChannels );
        if ( frameCount > 0 )
            AUDIO::CAudioConverter::ConvertToFloat32( frames, input.GetSampleType(), &work[ 0 ], (UInt64) frameCount * inChannels );
        if ( !matrix.empty() )
        {
            GUCEF::vector< Float32 > remixed( (size_t) frameCount * outChannels );
            if ( frameCount > 0 )
                AUDIO::CAudioConverter::RemixChannels( &work[ 0 ], inChannels, &remixed[ 0 ], outChannels, frameCount, matrix );
            work.swap( remixed );
        }
        Output( work, converted, convertedFrames );
    }

    /** What the resampler still holds, pushed out with silence after the last sound */
    void Flush( GUCEF::vector< UInt8 >& converted, UInt32& convertedFrames )
    {GUCEF_TRACE;

        convertedFrames = 0;
        if ( !resample )
            return;
        GUCEF::vector< Float32 > silence( (size_t) ( resampler.GetLatencyInFrames() + 1 ) * output.GetChannelCount(), 0.0f );
        Output( silence, converted, convertedFrames );
    }

    private:

    void Output( GUCEF::vector< Float32 >& work, GUCEF::vector< UInt8 >& converted, UInt32& convertedFrames )
    {GUCEF_TRACE;

        const UInt32 outChannels = output.GetChannelCount();
        if ( resample )
        {
            pending.insert( pending.end(), work.begin(), work.end() );
            const UInt32 pendingFrames = (UInt32) ( pending.size() / outChannels );
            const Float64 ratio = (Float64) output.GetSampleRate() / (Float64) input.GetSampleRate();
            GUCEF::vector< Float32 > resampled( (size_t) ( pendingFrames * ratio + 16 ) * outChannels );
            UInt32 consumed = 0, produced = 0;
            if ( pendingFrames > 0 )
                resampler.Process( &pending[ 0 ], pendingFrames, consumed, &resampled[ 0 ], (UInt32) ( resampled.size() / outChannels ), produced );
            pending.erase( pending.begin(), pending.begin() + (size_t) consumed * outChannels );
            resampled.resize( (size_t) produced * outChannels );
            work.swap( resampled );
        }
        convertedFrames = (UInt32) ( work.size() / outChannels );
        converted.resize( (size_t) convertedFrames * output.GetBytesPerFrame() );
        if ( convertedFrames > 0 )
            AUDIO::CAudioConverter::ConvertFromFloat32( &work[ 0 ], &converted[ 0 ], output.GetSampleType(), (UInt64) convertedFrames * outChannels );
    }
};

/*-------------------------------------------------------------------------*/

/** A video the plugin encodes ('sinkdata') */
class CVideoCodecCPluginSink : public IVideoSink ,
                               public CIVideoCodecCPluginClient
{
    public:

    typedef CVideoCodecCPluginCodec::TCodecInstancePtr TCodecInstancePtr;
    typedef CIVideoCodec::TIOAccessPtr TIOAccessPtr;
    typedef CIVideoCodec::TAudioFormatVector TAudioFormatVector;

    CVideoCodecCPluginSink( const TCodecInstancePtr& instance, CORE::CIOAccess* output, const TIOAccessPtr& owner )
        : IVideoSink()
        , CIVideoCodecCPluginClient()
        , m_instance( instance )
        , m_output( output )
        , m_outputOwner( owner )
        , m_sinkdata( GUCEF_NULL )
        , m_isOpen( false )
        , m_hasFailed( false )
        , m_isFinished( false )
        , m_format()
        , m_pluginPixelFormat( GUCEF_VIDEOCODEC_PIXELFORMAT_UNKNOWN )
        , m_audioFormats()
        , m_audioTracks()
        , m_picture()
        , m_audio()
        , m_framesWritten( 0 )
    {GUCEF_TRACE;
    }

    virtual ~CVideoCodecCPluginSink() GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        CVideoCodecCPlugin& plugin = m_instance->Plugin();
        MT::CScopeMutex lock( plugin.GetCallLock() );
        plugin.UnregisterClient( this );
        if ( m_isOpen && m_instance->IsUsable() )
            plugin.GetLinkedApi()->Snk_Close( plugin.GetPlugData(), m_instance->GetCodecData(), &m_sinkdata, 0 );
        m_isOpen = false;
        for ( size_t t=0; t<m_audioTracks.size(); ++t )
            GUCEF_DELETE m_audioTracks[ t ];
    }

    bool Open( const CVideoFormat& format, const TAudioFormatVector& audioTrackFormats, const CORE::CValueList& options )
    {GUCEF_TRACE;

        CVideoCodecCPlugin& plugin = m_instance->Plugin();
        MT::CScopeMutex lock( plugin.GetCallLock() );
        const TGucefVideoCodecPluginApi* api = plugin.GetLinkedApi();
        if ( !m_instance->IsUsable() || GUCEF_NULL == api->Snk_Open || 0 == format.GetWidth() || 0 == format.GetHeight() ||
             0 == format.GetFrameRateNumerator() || 0 == format.GetFrameRateDenominator() )
            return false;

        TStringMap optionMap;
        OptionsToStringMap( options, optionMap );
        TStringMapAdapter optionAdapter;
        optionAdapter.SetMapAccessPtr( (const TStringMap*) &optionMap );

        TVideoFormatInfo info;
        memset( &info, 0, sizeof( info ) );
        info.width = format.GetWidth();
        info.height = format.GetHeight();
        info.pixelFormat = GUCEF_VIDEOCODEC_PIXELFORMAT_RGBA32;
        info.frameRateNumerator = format.GetFrameRateNumerator();
        info.frameRateDenominator = format.GetFrameRateDenominator();
        info.audioTrackCount = (UInt32) audioTrackFormats.size();

        GUCEF::vector< TAudioFormatInfo > audioInfo( audioTrackFormats.size() );
        for ( size_t t=0; t<audioTrackFormats.size(); ++t )
        {
            if ( !audioTrackFormats[ t ].IsValid() )
                return false;
            AudioFormatToInfo( audioTrackFormats[ t ], audioInfo[ t ] );
        }

        if ( 0 == api->Snk_Open( plugin.GetPlugData(), m_instance->GetCodecData(), &m_sinkdata, m_output->CStyleAccess(), optionAdapter.GetCStyleAccess(),
                                 &info, (UInt32) audioInfo.size(), audioInfo.empty() ? GUCEF_NULL : &audioInfo[ 0 ] ) )
            return false;
        m_isOpen = true;

        // what the plugin takes: pictures of 32 bits the frames are converted to, sound the host converts to
        bool valid = GUCEF_VIDEOCODEC_PIXELFORMAT_RGBA32 == info.pixelFormat || GUCEF_VIDEOCODEC_PIXELFORMAT_BGRA32 == info.pixelFormat;
        m_pluginPixelFormat = info.pixelFormat;
        m_format = CVideoFormat( format.GetWidth(), format.GetHeight(), PixelFormatFromInfo( info.pixelFormat ), format.GetFrameRateNumerator(), format.GetFrameRateDenominator(), format.GetFrameCount() );
        for ( size_t t=0; valid && t<audioTrackFormats.size(); ++t )
        {
            AUDIO::CAudioFormat pluginFormat;
            CSinkAudioTrack* track = GUCEF_NEW CSinkAudioTrack();
            m_audioTracks.push_back( track );
            valid = AudioFormatFromInfo( audioInfo[ t ], pluginFormat ) && track->Setup( audioTrackFormats[ t ], pluginFormat );
        }
        if ( !valid )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "VideoCodecCPluginSink: The plugin \"" << plugin.GetName() << "\" asked for a picture or sound format the host cannot give" );
            api->Snk_Close( plugin.GetPlugData(), m_instance->GetCodecData(), &m_sinkdata, 0 );
            m_isOpen = false;
            return false;
        }
        m_audioFormats = audioTrackFormats;
        plugin.RegisterClient( this, true );
        return true;
    }

    virtual const CVideoFormat& GetFormat( void ) const GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        return m_format;
    }

    virtual UInt32 GetAudioTrackCount( void ) const GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        return (UInt32) m_audioFormats.size();
    }

    virtual bool GetAudioTrackFormat( UInt32 track, AUDIO::CAudioFormat& format ) const GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        if ( track >= m_audioFormats.size() )
            return false;
        format = m_audioFormats[ track ];
        return true;
    }

    virtual bool WriteFrame( const CVideoFrame& frame ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        if ( m_hasFailed || m_isFinished || !frame.HasPicture() || frame.GetWidth() != m_format.GetWidth() || frame.GetHeight() != m_format.GetHeight() )
            return Fail( "a picture that does not fit the sink" );

        const IMAGE::TPixelStorageFormat wanted = m_format.GetPixelFormat();
        TVideoPictureBuffer picture;
        memset( &picture, 0, sizeof( picture ) );
        if ( frame.GetPixelFormat() == wanted )
        {
            picture.pixels = const_cast< void* >( frame.GetPicture().GetDataPtr() );
            picture.pitch = frame.GetPitch();
        }
        else
        {
            m_picture.resize( (size_t) m_format.GetWidth() * m_format.GetHeight() * 4 );
            if ( !frame.ConvertTo32Bit( wanted, &m_picture[ 0 ], m_format.GetWidth() * 4 ) )
                return Fail( "a picture that cannot be converted" );
            picture.pixels = &m_picture[ 0 ];
            picture.pitch = m_format.GetWidth() * 4;
        }

        CVideoCodecCPlugin& plugin = m_instance->Plugin();
        MT::CScopeMutex lock( plugin.GetCallLock() );
        if ( !m_isOpen || !m_instance->IsUsable() )
            return Fail( "the plugin is gone" );
        if ( 0 == plugin.GetLinkedApi()->Snk_Write_Frame( plugin.GetPlugData(), m_instance->GetCodecData(), &m_sinkdata, &picture ) )
            return Fail( "the plugin failed to encode a picture" );
        ++m_framesWritten;
        return true;
    }

    virtual bool WriteAudio( UInt32 track, const void* frames, UInt32 frameCount ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        if ( m_hasFailed || m_isFinished || track >= m_audioTracks.size() )
            return Fail( "sound for a track the sink does not have" );
        if ( GUCEF_NULL == frames || 0 == frameCount )
            return true;

        UInt32 convertedFrames = 0;
        m_audioTracks[ track ]->Convert( frames, frameCount, m_audio, convertedFrames );
        return SendAudio( track, convertedFrames );
    }

    virtual bool Finish( void ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        if ( m_hasFailed || m_isFinished )
            return false;

        for ( UInt32 track=0; track<(UInt32)m_audioTracks.size(); ++track )
        {
            UInt32 convertedFrames = 0;
            m_audioTracks[ track ]->Flush( m_audio, convertedFrames );
            if ( !SendAudio( track, convertedFrames ) )
                return false;
        }

        CVideoCodecCPlugin& plugin = m_instance->Plugin();
        MT::CScopeMutex lock( plugin.GetCallLock() );
        if ( !m_isOpen || !m_instance->IsUsable() )
            return Fail( "the plugin is gone" );
        m_isFinished = true;
        m_isOpen = false;
        if ( 0 == plugin.GetLinkedApi()->Snk_Close( plugin.GetPlugData(), m_instance->GetCodecData(), &m_sinkdata, 1 ) )
            return Fail( "the plugin failed to complete the output" );
        return true;
    }

    virtual bool HasFailed( void ) const GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        return m_hasFailed;
    }

    virtual UInt64 GetFramesWritten( void ) const GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        return m_framesWritten;
    }

    virtual void OnPluginUnlink( const TGucefVideoCodecPluginApi& api, void** plugdata ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        if ( m_isOpen )
            api.Snk_Close( plugdata, m_instance->GetCodecData(), &m_sinkdata, 0 );
        m_isOpen = false;
        m_hasFailed = true;
    }

    private:

    bool SendAudio( UInt32 track, UInt32 convertedFrames )
    {GUCEF_TRACE;

        if ( 0 == convertedFrames )
            return true;
        CVideoCodecCPlugin& plugin = m_instance->Plugin();
        MT::CScopeMutex lock( plugin.GetCallLock() );
        if ( !m_isOpen || !m_instance->IsUsable() )
            return Fail( "the plugin is gone" );
        if ( 0 == plugin.GetLinkedApi()->Snk_Write_Audio( plugin.GetPlugData(), m_instance->GetCodecData(), &m_sinkdata, track, &m_audio[ 0 ], convertedFrames ) )
            return Fail( "the plugin failed to encode sound" );
        return true;
    }

    bool Fail( const char* what )
    {GUCEF_TRACE;

        if ( !m_hasFailed )
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "VideoCodecCPluginSink: Encoding stopped: " << what << ", after " << m_framesWritten << " pictures" );
        m_hasFailed = true;
        return false;
    }

    TCodecInstancePtr m_instance;
    CORE::CIOAccess* m_output;
    TIOAccessPtr m_outputOwner;
    void* m_sinkdata;
    bool m_isOpen;
    bool m_hasFailed;
    bool m_isFinished;
    CVideoFormat m_format;
    UInt32 m_pluginPixelFormat;
    TAudioFormatVector m_audioFormats;
    GUCEF::vector< CSinkAudioTrack* > m_audioTracks;
    GUCEF::vector< UInt8 > m_picture;
    GUCEF::vector< UInt8 > m_audio;
    UInt64 m_framesWritten;
};

/*-------------------------------------------------------------------------*/

}; /* anonymous namespace */

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

bool
CVideoCodecCPluginCodec::Create( const TVideoCodecCPluginPtr& plugin, const CString& codecType, CORE::CTBasicSharedPtr< CIVideoCodec, MT::CMutex >& codec )
{GUCEF_TRACE;

    if ( plugin.IsNULL() )
        return false;

    TCodecInstancePtr instance( GUCEF_NEW CVideoCodecCPluginInstance( plugin ) );
    {
        MT::CScopeMutex lock( plugin->GetCallLock() );
        if ( !instance->Create( codecType ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "VideoCodecCPluginCodec: The plugin \"" << plugin->GetName() << "\" could not create a codec of type \"" << codecType << "\"" );
            return false;
        }
    }

    codec = CORE::CTSharedPtr< CIVideoCodec, MT::CMutex >( GUCEF_NEW CVideoCodecCPluginCodec( plugin, codecType, instance ) );
    return true;
}

/*-------------------------------------------------------------------------*/

CVideoCodecCPluginCodec::CVideoCodecCPluginCodec( const TVideoCodecCPluginPtr& plugin, const CString& codecType, const TCodecInstancePtr& instance )
    : CIVideoCodec()
    , m_plugin( plugin )
    , m_codecType( codecType )
    , m_instance( instance )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CVideoCodecCPluginCodec::CVideoCodecCPluginCodec( const CVideoCodecCPluginCodec& src )
    : CIVideoCodec( src )
    , m_plugin( src.m_plugin )
    , m_codecType( src.m_codecType )
    , m_instance( src.m_instance )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CVideoCodecCPluginCodec::~CVideoCodecCPluginCodec()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CVideoCodecCPluginCodec::OpenSourceImpl( CORE::CIOAccess* input, const TIOAccessPtr& owner, const CORE::CValueList& options, TVideoSourcePtr& source )
{GUCEF_TRACE;

    if ( GUCEF_NULL == input )
        return false;

    CVideoCodecCPluginSource* pluginSource = GUCEF_NEW CVideoCodecCPluginSource( m_instance, input, owner );
    if ( !pluginSource->Open( options ) )
    {
        GUCEF_DELETE pluginSource;
        return false;
    }
    source = TVideoSourcePtr( pluginSource );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CVideoCodecCPluginCodec::CanEncode( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_plugin->GetCallLock() );
    const TGucefVideoCodecPluginApi* api = m_plugin->GetLinkedApi();
    return GUCEF_NULL != api && GUCEF_NULL != api->Snk_Open && m_instance->IsUsable();
}

/*-------------------------------------------------------------------------*/

bool
CVideoCodecCPluginCodec::OpenSinkImpl( CORE::CIOAccess* output, const TIOAccessPtr& owner, const CVideoFormat& format, const TAudioFormatVector& audioTrackFormats, const CORE::CValueList& options, TVideoSinkPtr& sink )
{GUCEF_TRACE;

    if ( GUCEF_NULL == output )
        return false;

    CVideoCodecCPluginSink* pluginSink = GUCEF_NEW CVideoCodecCPluginSink( m_instance, output, owner );
    if ( !pluginSink->Open( format, audioTrackFormats, options ) )
    {
        GUCEF_DELETE pluginSink;
        return false;
    }
    sink = TVideoSinkPtr( pluginSink );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CVideoCodecCPluginCodec::DecodeMetaData( CORE::CIOAccess& encodedInput, CORE::CValueList& metaData )
{GUCEF_TRACE;

    UInt64 startOffset = encodedInput.Tell();

    // The format, the length and the tracks are what opening the video tells
    {
        CVideoCodecCPluginSource source( m_instance, &encodedInput, TIOAccessPtr() );
        if ( !source.Open( CORE::CValueList() ) )
            return false;

        const CVideoFormat& format = source.GetFormat();
        metaData.Set( "width", CORE::UInt32ToString( format.GetWidth() ) );
        metaData.Set( "height", CORE::UInt32ToString( format.GetHeight() ) );
        metaData.Set( "pixelFormat", format.IsPalettized() ? CString( "palette8" ) : ( IMAGE::PSF_RGB == format.GetPixelFormat() ? CString( "rgb24" ) :
                                     ( IMAGE::PSF_RGBA == format.GetPixelFormat() ? CString( "rgba32" ) : CString( "bgra32" ) ) ) );
        metaData.Set( "frameRate", CORE::DoubleToString( format.GetFrameRate() ) );
        if ( GUCEF_VIDEO_FRAMECOUNT_UNKNOWN != format.GetFrameCount() )
        {
            metaData.Set( "frames", CORE::UInt64ToString( format.GetFrameCount() ) );
            metaData.Set( "durationInMs", CORE::UInt64ToString( format.GetDurationInMs() ) );
        }
        metaData.Set( "audioTracks", CORE::UInt32ToString( source.GetAudioTrackCount() ) );
        for ( UInt32 track=0; track<source.GetAudioTrackCount(); ++track )
        {
            AUDIO::CAudioFormat audioFormat;
            source.GetAudioTrackFormat( track, audioFormat );
            const CString prefix = "audioTrack" + CORE::UInt32ToString( track ) + ".";
            metaData.Set( prefix + "sampleRate", CORE::UInt32ToString( audioFormat.GetSampleRate() ) );
            metaData.Set( prefix + "channels", CORE::UInt32ToString( audioFormat.GetChannelCount() ) );
            metaData.Set( prefix + "sampleType", CString( AUDIO::CAudioFormat::SampleTypeToString( audioFormat.GetSampleType() ) ) );
        }
    }

    // Tags, if the plugin reads them
    MT::CScopeMutex lock( m_plugin->GetCallLock() );
    const TGucefVideoCodecPluginApi* api = m_plugin->GetLinkedApi();
    if ( GUCEF_NULL != api && GUCEF_NULL != api->Decode_Meta_Data && m_instance->IsUsable() )
    {
        if ( 0 != encodedInput.Setpos( startOffset ) )
            return true;

        TStringMap tags;
        TStringMapAdapter tagAdapter;
        tagAdapter.SetMapAccessPtr( &tags );
        if ( 0 != api->Decode_Meta_Data( m_plugin->GetPlugData(), m_instance->GetCodecData(), encodedInput.CStyleAccess(), tagAdapter.GetCStyleAccess() ) )
        {
            for ( TStringMap::const_iterator i = tags.begin(); i != tags.end(); ++i )
                metaData.Set( (*i).first, (*i).second );
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

CString
CVideoCodecCPluginCodec::GetType( void ) const
{GUCEF_TRACE;

    return m_codecType;
}

/*-------------------------------------------------------------------------*/

const CString&
CVideoCodecCPluginCodec::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString typeName = "GUCEF::VIDEO::CVideoCodecCPluginCodec";
    return typeName;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CVideoCodecCPluginCodec::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CVideoCodecCPluginCodec( *this );
}

/*-------------------------------------------------------------------------*/

const CVideoCodecCPluginCodec::TVideoCodecCPluginPtr&
CVideoCodecCPluginCodec::GetPlugin( void ) const
{GUCEF_TRACE;

    return m_plugin;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace VIDEO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
