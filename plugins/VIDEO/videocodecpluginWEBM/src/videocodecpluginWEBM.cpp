/*
 *  videocodecpluginWEBM: gucefVIDEO codec plugin for WebM video (VP8 / VP9 with Opus)
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

/*
 *  WebM files (Matroska with VP8 / VP9 video and Opus audio) through the gucefVIDEO codec plugin C interface.
 *  The container is read and written with libwebm, the video with libvpx and the sound with libopus.
 *
 *  Decoding: the blocks of all clusters are listed when the file is opened, in file order. A frame is decoded
 *  by going through the blocks up to the next block of the video track: the sound blocks on the way are decoded
 *  and handed to the host, the video block gives the picture (VP9 alt-ref frames that are not shown are passed
 *  over). After the last picture the rest of the sound comes with it. Seeking starts at the key frame at or
 *  before the frame and decodes forward.
 *
 *  Encoding: pictures come as RGBA and become I420 for the VP9 encoder, sound comes as 48 kHz float (the host
 *  resamples) and is encoded in frames of 20 ms. The packets of all tracks are written in the order of their
 *  time stamps; Snk_Close( finish ) drains the encoders, writes the rest and lets the muxer write the cues and
 *  the duration.
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <stdlib.h>
#include <string.h>
#include <math.h>

#if defined( _WIN32 )
  #ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
  #endif
  #ifndef NOMINMAX
    #define NOMINMAX
  #endif
  #include <windows.h>
#else
  #include <unistd.h>
#endif

#ifndef GUCEF_SFINAE_H
#include "gucef_SFINAE.h"
#define GUCEF_SFINAE_H
#endif /* GUCEF_SFINAE_H ? */

#ifndef GUCEF_STL_H
#include "gucef_stl.h"
#define GUCEF_STL_H
#endif /* GUCEF_STL_H ? */

#ifndef GUCEF_CORE_C_LOGGING_H
#include "gucefCORE_c_logging.h"
#define GUCEF_CORE_C_LOGGING_H
#endif /* GUCEF_CORE_C_LOGGING_H ? */

#include "vpx/vpx_decoder.h"
#include "vpx/vpx_encoder.h"
#include "vpx/vp8dx.h"
#include "vpx/vp8cx.h"
#include "opus.h"
#include "mkvparser/mkvparser.h"
#include "mkvmuxer/mkvmuxer.h"

#include "videocodecpluginWEBM.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace VIDEO {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

const char* const g_codecTypes[] = { "webm", "mkv" };

const UInt32 OpusSampleRate = 48000;
const UInt32 OpusFrameSize = 960;               // 20 ms at 48 kHz
const UInt32 OpusMaxFrameSize = 5760;           // 120 ms, the longest packet
const UInt64 NanoSecondsPerSecond = 1000000000ULL;

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

struct SWebmPlugin
{
    TGucefCoreCApi libApi;
};

/*-------------------------------------------------------------------------*/

/** libwebm reads through the input of the host */
class CIOAccessReader : public mkvparser::IMkvReader
{
    public:

    explicit CIOAccessReader( TIOAccess* input ) : m_input( input ) {}

    virtual int Read( long long pos, long len, unsigned char* buf )
    {
        if ( len < 0 || pos < 0 )
            return -1;
        if ( 0 == len )
            return 0;
        if ( 0 != m_input->setpos( m_input, (UInt64) pos ) )
            return -1;
        return m_input->read( m_input, buf, 1, (UInt32) len ) == (UInt32) len ? 0 : -1;
    }

    virtual int Length( long long* total, long long* available )
    {
        const UInt64 size = m_input->size( m_input );
        if ( NULL != total )
            *total = (long long) size;
        if ( NULL != available )
            *available = (long long) size;
        return 0;
    }

    private:

    TIOAccess* m_input;
};

/*-------------------------------------------------------------------------*/

/** libwebm writes through the output of the host */
class CIOAccessWriter : public mkvmuxer::IMkvWriter
{
    public:

    explicit CIOAccessWriter( TIOAccess* output ) : m_output( output ) {}

    virtual mkvmuxer::int32 Write( const void* buf, mkvmuxer::uint32 len )
    {
        if ( 0 == len )
            return 0;
        return m_output->write( m_output, buf, 1, len ) == len ? 0 : -1;
    }

    virtual mkvmuxer::int64 Position( void ) const
    {
        return (mkvmuxer::int64) m_output->tell( m_output );
    }

    virtual mkvmuxer::int32 Position( mkvmuxer::int64 position )
    {
        return 0 == m_output->setpos( m_output, (UInt64) position ) ? 0 : -1;
    }

    virtual bool Seekable( void ) const
    {
        return NULL != m_output->setpos;
    }

    virtual void ElementStartNotify( mkvmuxer::uint64, mkvmuxer::int64 )
    {
    }

    private:

    TIOAccess* m_output;
};

/*-------------------------------------------------------------------------*/

/** A block of the file, in file order */
struct SBlockRef
{
    const mkvparser::Cluster* cluster;
    const mkvparser::BlockEntry* entry;
    long long trackNumber;
};

/*-------------------------------------------------------------------------*/

/** An Opus track the source decodes */
struct SAudioTrack
{
    long long trackNumber;
    UInt32 channels;
    OpusDecoder* decoder;
    UInt32 preSkipLeft;                 /**< samples at the start that are not part of the sound */
    UInt32 preSkip;
};

/*-------------------------------------------------------------------------*/

struct SWebmSource
{
    SWebmPlugin* plugin;
    CIOAccessReader* reader;
    mkvparser::Segment* segment;
    long long videoTrackNumber;
    UInt32 width;
    UInt32 height;
    vpx_codec_ctx_t decoder;
    bool decoderOpen;
    GUCEF::vector< SBlockRef > blocks;
    GUCEF::vector< size_t > videoBlocks;            /**< index into blocks of each picture */
    GUCEF::vector< SAudioTrack > audioTracks;
    size_t nextBlock;
    UInt64 position;                                /**< the picture the next decode gives */
    GUCEF::vector< UInt8 > packet;
    GUCEF::vector< Float32 > pcm;
};

/*-------------------------------------------------------------------------*/

/** A packet waiting to be written, so all tracks go into the file in time order */
struct SPacket
{
    UInt64 trackNumber;
    UInt64 timestampNs;
    bool isKey;
    GUCEF::vector< UInt8 > data;
};

/*-------------------------------------------------------------------------*/

struct SSinkAudioTrack
{
    UInt64 trackNumber;
    UInt32 channels;
    OpusEncoder* encoder;
    UInt32 preSkip;
    UInt64 samplesEncoded;                          /**< per channel, including the pre-skip */
    GUCEF::vector< Float32 > pending;               /**< samples not yet a full frame */
    GUCEF::vector< SPacket* > queue;
};

/*-------------------------------------------------------------------------*/

struct SWebmSink
{
    SWebmPlugin* plugin;
    CIOAccessWriter* writer;
    mkvmuxer::Segment* segment;
    UInt64 videoTrackNumber;
    UInt32 width;
    UInt32 height;
    UInt32 frameRateNumerator;
    UInt32 frameRateDenominator;
    vpx_codec_ctx_t encoder;
    bool encoderOpen;
    vpx_image_t* image;
    unsigned long deadline;
    UInt64 framesIn;
    GUCEF::vector< SPacket* > videoQueue;
    GUCEF::vector< SSinkAudioTrack > audioTracks;
    GUCEF::vector< UInt8 > packet;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** A string from a variant map, NULL when the key is absent or not text */
const char*
GetTextOption( const TVariantMapApi* map, const char* key )
{
    CORE::TVariantData keyData;
    CORE::TVariantData valueData;

    if ( NULL == map || NULL == map->at_key )
        return NULL;

    memset( &keyData, 0, sizeof( keyData ) );
    keyData.containedType = GUCEF_DATATYPE_ASCII_STRING;
    keyData.union_data.heap_data.union_data.const_char_heap_data = key;
    keyData.union_data.heap_data.heap_data_size = (UInt32) strlen( key );
    keyData.union_data.heap_data.heap_data_is_linked = 1;

    memset( &valueData, 0, sizeof( valueData ) );
    map->at_key( map->privateData, &keyData, &valueData );
    if ( GUCEF_DATATYPE_ASCII_STRING != valueData.containedType && GUCEF_DATATYPE_UTF8_STRING != valueData.containedType )
        return NULL;
    return valueData.union_data.heap_data.union_data.const_char_heap_data;
}

/*-------------------------------------------------------------------------*/

Int32
GetIntOption( const TVariantMapApi* map, const char* key, Int32 defaultValue, Int32 minValue, Int32 maxValue )
{
    const char* text = GetTextOption( map, key );
    if ( NULL == text || '\0' == *text )
        return defaultValue;
    long value = strtol( text, NULL, 10 );
    if ( value < minValue ) value = minValue;
    if ( value > maxValue ) value = maxValue;
    return (Int32) value;
}

/*-------------------------------------------------------------------------*/

/** The logical processors of this machine, the default number of threads of the encoder */
Int32
LogicalProcessorCount( void )
{
    #if defined( _WIN32 )
    SYSTEM_INFO info;
    GetSystemInfo( &info );
    const Int32 count = (Int32) info.dwNumberOfProcessors;
    #else
    const Int32 count = (Int32) sysconf( _SC_NPROCESSORS_ONLN );
    #endif
    return count < 1 ? 1 : ( count > 64 ? 64 : count );
}

/*-------------------------------------------------------------------------*/

/** The tile columns of VP9 as a power of 2: as many as the width allows (at least 256 pixels each), for the threads */
Int32
TileColumnsLog2( UInt32 width, Int32 threads )
{
    Int32 log2 = 0;
    while ( log2 < 6 && ( width >> ( log2 + 1 ) ) >= 256 && ( 1 << ( log2 + 1 ) ) <= threads )
        ++log2;
    return log2;
}

/*-------------------------------------------------------------------------*/

UInt64
GreatestCommonDivisor( UInt64 a, UInt64 b )
{
    while ( 0 != b )
    {
        const UInt64 t = a % b;
        a = b;
        b = t;
    }
    return a;
}

/*-------------------------------------------------------------------------*/

UInt8
ClampToByte( Int32 value )
{
    return (UInt8) ( value < 0 ? 0 : ( value > 255 ? 255 : value ) );
}

/*-------------------------------------------------------------------------*/

/** A decoded picture (any of the 8 bit planar formats libvpx gives) to RGBA */
void
ImageToRgba( const vpx_image_t* image, UInt8* pixels, UInt32 pitch )
{
    // the coefficients of the colour space, studio range unless the stream says full range (x 256)
    const bool fullRange = VPX_CR_FULL_RANGE == image->range;
    const bool bt709 = VPX_CS_BT_709 == image->cs;
    const Int32 yScale = fullRange ? 256 : 298;
    const Int32 yOffset = fullRange ? 0 : 16;
    const Int32 rv = fullRange ? 359 : ( bt709 ? 459 : 409 );
    const Int32 gu = fullRange ? 88 : ( bt709 ? 55 : 100 );
    const Int32 gv = fullRange ? 183 : ( bt709 ? 136 : 208 );
    const Int32 bu = fullRange ? 454 : ( bt709 ? 541 : 516 );

    for ( UInt32 y=0; y<image->d_h; ++y )
    {
        const UInt8* yRow = image->planes[ VPX_PLANE_Y ] + (size_t) y * image->stride[ VPX_PLANE_Y ];
        const UInt8* uRow = image->planes[ VPX_PLANE_U ] + (size_t) ( y >> image->y_chroma_shift ) * image->stride[ VPX_PLANE_U ];
        const UInt8* vRow = image->planes[ VPX_PLANE_V ] + (size_t) ( y >> image->y_chroma_shift ) * image->stride[ VPX_PLANE_V ];
        UInt8* out = pixels + (size_t) y * pitch;
        for ( UInt32 x=0; x<image->d_w; ++x, out+=4 )
        {
            const Int32 c = yScale * ( (Int32) yRow[ x ] - yOffset );
            const Int32 d = (Int32) uRow[ x >> image->x_chroma_shift ] - 128;
            const Int32 e = (Int32) vRow[ x >> image->x_chroma_shift ] - 128;
            out[ 0 ] = ClampToByte( ( c + rv * e + 128 ) >> 8 );
            out[ 1 ] = ClampToByte( ( c - gu * d - gv * e + 128 ) >> 8 );
            out[ 2 ] = ClampToByte( ( c + bu * d + 128 ) >> 8 );
            out[ 3 ] = 255;
        }
    }
}

/*-------------------------------------------------------------------------*/

/** An RGBA picture to the I420 image of the encoder, BT.601 studio range */
void
RgbaToImage( const UInt8* pixels, UInt32 pitch, UInt32 width, UInt32 height, vpx_image_t* image )
{
    for ( UInt32 y=0; y<height; ++y )
    {
        const UInt8* in = pixels + (size_t) y * pitch;
        UInt8* yRow = image->planes[ VPX_PLANE_Y ] + (size_t) y * image->stride[ VPX_PLANE_Y ];
        for ( UInt32 x=0; x<width; ++x, in+=4 )
            yRow[ x ] = (UInt8) ( ( ( 66 * in[ 0 ] + 129 * in[ 1 ] + 25 * in[ 2 ] + 128 ) >> 8 ) + 16 );
    }
    const UInt32 chromaWidth = ( width + 1 ) / 2, chromaHeight = ( height + 1 ) / 2;
    for ( UInt32 cy=0; cy<chromaHeight; ++cy )
    {
        UInt8* uRow = image->planes[ VPX_PLANE_U ] + (size_t) cy * image->stride[ VPX_PLANE_U ];
        UInt8* vRow = image->planes[ VPX_PLANE_V ] + (size_t) cy * image->stride[ VPX_PLANE_V ];
        for ( UInt32 cx=0; cx<chromaWidth; ++cx )
        {
            // the average of the (up to) 2 x 2 pixels the chroma sample covers
            Int32 r = 0, g = 0, b = 0, n = 0;
            for ( UInt32 dy=0; dy<2; ++dy )
                for ( UInt32 dx=0; dx<2; ++dx )
                {
                    const UInt32 px = cx * 2 + dx, py = cy * 2 + dy;
                    if ( px >= width || py >= height )
                        continue;
                    const UInt8* p = pixels + (size_t) py * pitch + (size_t) px * 4;
                    r += p[ 0 ];
                    g += p[ 1 ];
                    b += p[ 2 ];
                    ++n;
                }
            r /= n;
            g /= n;
            b /= n;
            uRow[ cx ] = ClampToByte( ( ( -38 * r - 74 * g + 112 * b + 128 ) >> 8 ) + 128 );
            vRow[ cx ] = ClampToByte( ( ( 112 * r - 94 * g - 18 * b + 128 ) >> 8 ) + 128 );
        }
    }
}

/*-------------------------------------------------------------------------*/

UInt16
ReadLE16( const UInt8* p )
{
    return (UInt16) ( p[ 0 ] | ( p[ 1 ] << 8 ) );
}

/*-------------------------------------------------------------------------*/

void
WriteLE16( UInt8* p, UInt32 value )
{
    p[ 0 ] = (UInt8) ( value & 0xFF );
    p[ 1 ] = (UInt8) ( ( value >> 8 ) & 0xFF );
}

/*-------------------------------------------------------------------------*/

void
WriteLE32( UInt8* p, UInt32 value )
{
    WriteLE16( p, value & 0xFFFF );
    WriteLE16( p + 2, value >> 16 );
}

/*-------------------------------------------------------------------------*/

void
CloseSource( SWebmSource* source )
{
    if ( NULL == source )
        return;
    if ( source->decoderOpen )
        vpx_codec_destroy( &source->decoder );
    for ( size_t t=0; t<source->audioTracks.size(); ++t )
        if ( NULL != source->audioTracks[ t ].decoder )
            opus_decoder_destroy( source->audioTracks[ t ].decoder );
    delete source->segment;
    delete source->reader;
    delete source;
}

/*-------------------------------------------------------------------------*/

/** Starts the video decoder again: after it the next picture has to be a key frame */
bool
ResetDecoders( SWebmSource* source, const char* codecId )
{
    if ( source->decoderOpen )
        vpx_codec_destroy( &source->decoder );
    source->decoderOpen = false;

    vpx_codec_dec_cfg_t config;
    memset( &config, 0, sizeof( config ) );
    config.threads = 4;
    config.w = source->width;
    config.h = source->height;
    vpx_codec_iface_t* iface = 0 == strcmp( codecId, "V_VP8" ) ? vpx_codec_vp8_dx() : vpx_codec_vp9_dx();
    if ( VPX_CODEC_OK != vpx_codec_dec_init( &source->decoder, iface, &config, 0 ) )
        return false;
    source->decoderOpen = true;

    for ( size_t t=0; t<source->audioTracks.size(); ++t )
    {
        SAudioTrack& track = source->audioTracks[ t ];
        opus_decoder_ctl( track.decoder, OPUS_RESET_STATE );
        track.preSkipLeft = track.preSkip;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

/** Reads a frame of a block into the packet buffer */
bool
ReadFrame( SWebmSource* source, const mkvparser::Block::Frame& frame )
{
    if ( frame.len <= 0 )
        return false;
    source->packet.resize( (size_t) frame.len );
    return 0 == frame.Read( source->reader, &source->packet[ 0 ] );
}

/*-------------------------------------------------------------------------*/

/** Decodes the sound of an audio block and gives it to the host (NULL output drops it) */
bool
DecodeAudioBlock( SWebmSource* source, const SBlockRef& ref, const TVideoAudioOutput* audio )
{
    UInt32 trackIndex = 0;
    while ( trackIndex < source->audioTracks.size() && source->audioTracks[ trackIndex ].trackNumber != ref.trackNumber )
        ++trackIndex;
    if ( trackIndex >= source->audioTracks.size() )
        return true;     // a track that is left out

    SAudioTrack& track = source->audioTracks[ trackIndex ];
    const mkvparser::Block* block = ref.entry->GetBlock();
    for ( int f=0; f<block->GetFrameCount(); ++f )
    {
        if ( !ReadFrame( source, block->GetFrame( f ) ) )
            return false;
        source->pcm.resize( (size_t) OpusMaxFrameSize * track.channels );
        int samples = opus_decode_float( track.decoder, &source->packet[ 0 ], (opus_int32) source->packet.size(), &source->pcm[ 0 ], (int) OpusMaxFrameSize, 0 );
        if ( samples < 0 )
        {
            GUCEF_C_ERROR_LOG( source->plugin->libApi, GUCEF_LOGLEVEL_NORMAL, "videocodecpluginWEBM: an Opus packet cannot be decoded" );
            return false;
        }

        // the last packet can say how much of its end is padding
        if ( f + 1 == block->GetFrameCount() && block->GetDiscardPadding() > 0 )
        {
            const long long discard = (long long) ( (double) block->GetDiscardPadding() * OpusSampleRate / NanoSecondsPerSecond + 0.5 );
            samples = discard >= samples ? 0 : samples - (int) discard;
        }
        UInt32 skip = track.preSkipLeft < (UInt32) samples ? track.preSkipLeft : (UInt32) samples;
        track.preSkipLeft -= skip;
        if ( NULL != audio && NULL != audio->onAudio && (UInt32) samples > skip )
            audio->onAudio( audio->userData, trackIndex, &source->pcm[ (size_t) skip * track.channels ], (UInt32) samples - skip );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

/**
 *  Decodes up to and including the next shown picture: true with 'gotPicture' when there was one. The sound on
 *  the way goes to 'audio'; after the last picture the rest of the sound comes with it
 */
bool
DecodeNext( SWebmSource* source, TVideoPictureBuffer* picture, const TVideoAudioOutput* audio, bool& gotPicture )
{
    gotPicture = false;
    while ( source->nextBlock < source->blocks.size() && !gotPicture )
    {
        const SBlockRef& ref = source->blocks[ source->nextBlock++ ];
        if ( ref.trackNumber != source->videoTrackNumber )
        {
            if ( !DecodeAudioBlock( source, ref, audio ) )
                return false;
            continue;
        }

        const mkvparser::Block* block = ref.entry->GetBlock();
        for ( int f=0; f<block->GetFrameCount(); ++f )
        {
            if ( !ReadFrame( source, block->GetFrame( f ) ) )
                return false;
            if ( VPX_CODEC_OK != vpx_codec_decode( &source->decoder, &source->packet[ 0 ], (unsigned int) source->packet.size(), NULL, 0 ) )
            {
                GUCEF_C_ERROR_LOG( source->plugin->libApi, GUCEF_LOGLEVEL_NORMAL, "videocodecpluginWEBM: a picture cannot be decoded" );
                return false;
            }
            vpx_codec_iter_t iter = NULL;
            const vpx_image_t* image = NULL;
            const vpx_image_t* shown = NULL;
            while ( NULL != ( image = vpx_codec_get_frame( &source->decoder, &iter ) ) )
                shown = image;
            if ( NULL == shown )
                continue;
            if ( shown->d_w != source->width || shown->d_h != source->height || ( VPX_IMG_FMT_I420 != shown->fmt && VPX_IMG_FMT_I422 != shown->fmt && VPX_IMG_FMT_I444 != shown->fmt && VPX_IMG_FMT_I440 != shown->fmt ) )
            {
                GUCEF_C_ERROR_LOG( source->plugin->libApi, GUCEF_LOGLEVEL_NORMAL, "videocodecpluginWEBM: a picture changes size or has a pixel format that is not supported" );
                return false;
            }
            if ( NULL != picture && NULL != picture->pixels )
                ImageToRgba( shown, static_cast< UInt8* >( picture->pixels ), picture->pitch );
            gotPicture = true;
        }
    }

    // the sound after the last picture comes with it
    if ( gotPicture && source->position + 1 >= source->videoBlocks.size() )
    {
        while ( source->nextBlock < source->blocks.size() )
        {
            const SBlockRef& ref = source->blocks[ source->nextBlock++ ];
            if ( ref.trackNumber != source->videoTrackNumber && !DecodeAudioBlock( source, ref, audio ) )
                return false;
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
DeletePackets( GUCEF::vector< SPacket* >& queue )
{
    for ( size_t i=0; i<queue.size(); ++i )
        delete queue[ i ];
    queue.clear();
}

/*-------------------------------------------------------------------------*/

void
CloseSink( SWebmSink* sink )
{
    if ( NULL == sink )
        return;
    if ( sink->encoderOpen )
        vpx_codec_destroy( &sink->encoder );
    if ( NULL != sink->image )
        vpx_img_free( sink->image );
    DeletePackets( sink->videoQueue );
    for ( size_t t=0; t<sink->audioTracks.size(); ++t )
    {
        if ( NULL != sink->audioTracks[ t ].encoder )
            opus_encoder_destroy( sink->audioTracks[ t ].encoder );
        DeletePackets( sink->audioTracks[ t ].queue );
    }
    delete sink->segment;
    delete sink->writer;
    delete sink;
}

/*-------------------------------------------------------------------------*/

/**
 *  Writes the queued packets in time order. A packet is written once every track that can still get packets has
 *  one at least as late queued, so nothing comes later with an earlier time; 'all' writes everything
 */
bool
WriteQueued( SWebmSink* sink, bool all )
{
    while ( true )
    {
        GUCEF::vector< SPacket* >* earliest = NULL;
        bool waiting = false;

        GUCEF::vector< GUCEF::vector< SPacket* >* > queues;
        queues.push_back( &sink->videoQueue );
        for ( size_t t=0; t<sink->audioTracks.size(); ++t )
            queues.push_back( &sink->audioTracks[ t ].queue );

        for ( size_t q=0; q<queues.size(); ++q )
        {
            if ( queues[ q ]->empty() )
            {
                waiting = true;
                continue;
            }
            if ( NULL == earliest || queues[ q ]->front()->timestampNs < earliest->front()->timestampNs )
                earliest = queues[ q ];
        }
        if ( NULL == earliest || ( waiting && !all ) )
            return true;

        SPacket* packet = earliest->front();
        earliest->erase( earliest->begin() );
        const bool written = sink->segment->AddFrame( packet->data.empty() ? NULL : &packet->data[ 0 ], packet->data.size(), packet->trackNumber, packet->timestampNs, packet->isKey );
        delete packet;
        if ( !written )
        {
            GUCEF_C_ERROR_LOG( sink->plugin->libApi, GUCEF_LOGLEVEL_NORMAL, "videocodecpluginWEBM: a packet cannot be written" );
            return false;
        }
    }
}

/*-------------------------------------------------------------------------*/

/** Takes the packets the video encoder has ready into the queue */
bool
CollectVideoPackets( SWebmSink* sink )
{
    vpx_codec_iter_t iter = NULL;
    const vpx_codec_cx_pkt_t* packet = NULL;
    while ( NULL != ( packet = vpx_codec_get_cx_data( &sink->encoder, &iter ) ) )
    {
        if ( VPX_CODEC_CX_FRAME_PKT != packet->kind )
            continue;
        SPacket* queued = new SPacket();
        queued->trackNumber = sink->videoTrackNumber;
        queued->timestampNs = (UInt64) packet->data.frame.pts * NanoSecondsPerSecond * sink->frameRateDenominator / sink->frameRateNumerator;
        queued->isKey = 0 != ( packet->data.frame.flags & VPX_FRAME_IS_KEY );
        const UInt8* data = static_cast< const UInt8* >( packet->data.frame.buf );
        queued->data.assign( data, data + packet->data.frame.sz );
        sink->videoQueue.push_back( queued );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

/** Encodes the whole frames of sound a track has; 'flush' pads the rest with silence to a last frame */
bool
EncodeAudio( SWebmSink* sink, SSinkAudioTrack& track, bool flush )
{
    const size_t frameSamples = (size_t) OpusFrameSize * track.channels;
    if ( flush && !track.pending.empty() )
    {
        const size_t rest = track.pending.size() % frameSamples;
        if ( 0 != rest )
            track.pending.insert( track.pending.end(), frameSamples - rest, 0.0f );
    }
    size_t used = 0;
    while ( track.pending.size() - used >= frameSamples )
    {
        sink->packet.resize( 4000 );
        const opus_int32 bytes = opus_encode_float( track.encoder, &track.pending[ used ], (int) OpusFrameSize, &sink->packet[ 0 ], (opus_int32) sink->packet.size() );
        if ( bytes < 0 )
        {
            GUCEF_C_ERROR_LOG( sink->plugin->libApi, GUCEF_LOGLEVEL_NORMAL, "videocodecpluginWEBM: sound cannot be encoded with Opus" );
            return false;
        }
        used += frameSamples;

        // the time of a packet is that of its first sample after the pre-skip
        SPacket* queued = new SPacket();
        queued->trackNumber = track.trackNumber;
        queued->timestampNs = track.samplesEncoded > track.preSkip ? ( track.samplesEncoded - track.preSkip ) * NanoSecondsPerSecond / OpusSampleRate : 0;
        queued->isKey = true;
        queued->data.assign( &sink->packet[ 0 ], &sink->packet[ 0 ] + bytes );
        track.queue.push_back( queued );
        track.samplesEncoded += OpusFrameSize;
    }
    track.pending.erase( track.pending.begin(), track.pending.begin() + used );
    return true;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      PLUGIN API                                                         //
//                                                                         //
//-------------------------------------------------------------------------*/

UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
MODULE_API_FUNCTION( VIDEOCODECPLUG_Init )( void** plugdata, TGucefCoreCApi* libApi ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    SWebmPlugin* plugin = new SWebmPlugin();
    memset( &plugin->libApi, 0, sizeof( plugin->libApi ) );
    if ( NULL != libApi )
        plugin->libApi = *libApi;
    *plugdata = plugin;

    GUCEF_C_LOG( plugin->libApi, GUCEF_LOGLEVEL_BELOW_NORMAL, "videocodecpluginWEBM: initialized" );
    return 1;
}

/*-------------------------------------------------------------------------*/

void GUCEF_PLUGIN_CALLSPEC_PREFIX
MODULE_API_FUNCTION( VIDEOCODECPLUG_Shutdown )( void** plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    delete static_cast< SWebmPlugin* >( *plugdata );
    *plugdata = NULL;
}

/*-------------------------------------------------------------------------*/

const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
MODULE_API_FUNCTION( VIDEOCODECPLUG_Name )( const void* plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    return "gucefVIDEO codec plugin for WebM video (VP8 / VP9 with Opus)";
}

/*-------------------------------------------------------------------------*/

const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
MODULE_API_FUNCTION( VIDEOCODECPLUG_Copyright )( const void* plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    return "Copyright (C) Dinand Vanvelzen. Licensed under the Apache License, Version 2.0. Uses libvpx, libopus and libwebm (BSD licenses)";
}

/*-------------------------------------------------------------------------*/

const TVersion* GUCEF_PLUGIN_CALLSPEC_PREFIX
MODULE_API_FUNCTION( VIDEOCODECPLUG_Version )( const void* plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    static TVersion version;
    version.major = 0;
    version.minor = 1;
    version.patch = 0;
    version.release = 0;
    return &version;
}

/*-------------------------------------------------------------------------*/

UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
MODULE_API_FUNCTION( VIDEOCODECPLUG_Type_Count )( const void* plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    return (UInt32) ( sizeof( g_codecTypes ) / sizeof( g_codecTypes[ 0 ] ) );
}

/*-------------------------------------------------------------------------*/

const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
MODULE_API_FUNCTION( VIDEOCODECPLUG_Type )( const void* plugdata, UInt32 index ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    if ( index >= sizeof( g_codecTypes ) / sizeof( g_codecTypes[ 0 ] ) )
        return NULL;
    return g_codecTypes[ index ];
}

/*-------------------------------------------------------------------------*/

UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
MODULE_API_FUNCTION( VIDEOCODECPLUG_Create_Codec )( void** plugdata, const char* codecType, void** codecdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    // one container, no per codec state
    *codecdata = NULL;
    if ( NULL == codecType )
        return 0;
    for ( size_t i=0; i<sizeof( g_codecTypes ) / sizeof( g_codecTypes[ 0 ] ); ++i )
        if ( 0 == strcmp( codecType, g_codecTypes[ i ] ) )
            return 1;
    return 0;
}

/*-------------------------------------------------------------------------*/

void GUCEF_PLUGIN_CALLSPEC_PREFIX
MODULE_API_FUNCTION( VIDEOCODECPLUG_Destroy_Codec )( void** plugdata, void** codecdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    *codecdata = NULL;
}

/*-------------------------------------------------------------------------*/

UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
MODULE_API_FUNCTION( VIDEOCODECPLUG_Src_Open )( void** plugdata, void** codecdata, void** sourcedata, TIOAccess* input, const TVariantMapApi* options, TVideoFormatInfo* format, UInt64* frameCount ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    SWebmPlugin* plugin = static_cast< SWebmPlugin* >( *plugdata );
    *sourcedata = NULL;
    if ( NULL == input || NULL == input->read || NULL == input->setpos || NULL == input->size || NULL == format )
        return 0;

    SWebmSource* source = new SWebmSource();
    source->plugin = plugin;
    source->reader = new CIOAccessReader( input );
    source->segment = NULL;
    source->videoTrackNumber = -1;
    source->width = 0;
    source->height = 0;
    source->decoderOpen = false;
    source->nextBlock = 0;
    source->position = 0;

    // the header and the segment with all its clusters
    long long pos = 0;
    mkvparser::EBMLHeader header;
    if ( header.Parse( source->reader, pos ) < 0 || NULL == header.m_docType ||
         ( 0 != strcmp( header.m_docType, "webm" ) && 0 != strcmp( header.m_docType, "matroska" ) ) ||
         0 != mkvparser::Segment::CreateInstance( source->reader, pos, source->segment ) || NULL == source->segment ||
         source->segment->Load() < 0 )
    {
        CloseSource( source );
        return 0;
    }

    // the first VP8 / VP9 track is the picture, Opus tracks are the sound
    const mkvparser::Tracks* tracks = source->segment->GetTracks();
    const char* videoCodec = NULL;
    double frameRate = 0.0;
    UInt64 defaultDuration = 0;
    for ( unsigned long i=0; NULL != tracks && i<tracks->GetTracksCount(); ++i )
    {
        const mkvparser::Track* track = tracks->GetTrackByIndex( i );
        if ( NULL == track || NULL == track->GetCodecId() )
            continue;
        if ( mkvparser::Track::kVideo == track->GetType() )
        {
            if ( -1 != source->videoTrackNumber )
                continue;
            if ( 0 != strcmp( track->GetCodecId(), "V_VP8" ) && 0 != strcmp( track->GetCodecId(), "V_VP9" ) )
            {
                GUCEF_C_LOG( plugin->libApi, GUCEF_LOGLEVEL_NORMAL, "videocodecpluginWEBM: a video track with a codec other than VP8 / VP9 is left out" );
                continue;
            }
            const mkvparser::VideoTrack* video = static_cast< const mkvparser::VideoTrack* >( track );
            source->videoTrackNumber = track->GetNumber();
            source->width = (UInt32) video->GetWidth();
            source->height = (UInt32) video->GetHeight();
            frameRate = video->GetFrameRate();
            defaultDuration = track->GetDefaultDuration();
            videoCodec = track->GetCodecId();
        }
        else
        if ( mkvparser::Track::kAudio == track->GetType() )
        {
            size_t privateSize = 0;
            const unsigned char* privateData = track->GetCodecPrivate( privateSize );
            if ( 0 != strcmp( track->GetCodecId(), "A_OPUS" ) || NULL == privateData || privateSize < 19 || 0 != memcmp( privateData, "OpusHead", 8 ) )
            {
                GUCEF_C_LOG( plugin->libApi, GUCEF_LOGLEVEL_NORMAL, "videocodecpluginWEBM: an audio track with a codec other than Opus is left out" );
                continue;
            }
            const UInt32 channels = privateData[ 9 ];
            if ( 0 != privateData[ 18 ] || channels < 1 || channels > 2 )
            {
                GUCEF_C_LOG( plugin->libApi, GUCEF_LOGLEVEL_NORMAL, "videocodecpluginWEBM: an Opus track with more than two channels is left out" );
                continue;
            }
            SAudioTrack audioTrack;
            audioTrack.trackNumber = track->GetNumber();
            audioTrack.channels = channels;
            audioTrack.preSkip = ReadLE16( privateData + 10 );
            audioTrack.preSkipLeft = audioTrack.preSkip;
            int error = 0;
            audioTrack.decoder = opus_decoder_create( (opus_int32) OpusSampleRate, (int) channels, &error );
            if ( NULL == audioTrack.decoder || OPUS_OK != error )
            {
                CloseSource( source );
                return 0;
            }
            source->audioTracks.push_back( audioTrack );
        }
    }
    if ( -1 == source->videoTrackNumber || 0 == source->width || 0 == source->height || !ResetDecoders( source, videoCodec ) )
    {
        GUCEF_C_ERROR_LOG( plugin->libApi, GUCEF_LOGLEVEL_NORMAL, "videocodecpluginWEBM: the file has no VP8 / VP9 video that can be decoded" );
        CloseSource( source );
        return 0;
    }

    // all blocks in file order, and where the pictures are
    long long firstTime = -1, lastTime = -1;
    const mkvparser::Cluster* cluster = source->segment->GetFirst();
    while ( NULL != cluster && !cluster->EOS() )
    {
        const mkvparser::BlockEntry* entry = NULL;
        if ( cluster->GetFirst( entry ) < 0 )
            break;
        while ( NULL != entry && !entry->EOS() )
        {
            const mkvparser::Block* block = entry->GetBlock();
            if ( NULL != block )
            {
                SBlockRef ref;
                ref.cluster = cluster;
                ref.entry = entry;
                ref.trackNumber = block->GetTrackNumber();
                if ( ref.trackNumber == source->videoTrackNumber )
                {
                    for ( int f=0; f<block->GetFrameCount(); ++f )
                        source->videoBlocks.push_back( source->blocks.size() );
                    const long long time = block->GetTime( cluster );
                    if ( firstTime < 0 ) firstTime = time;
                    lastTime = time;
                }
                source->blocks.push_back( ref );
            }
            if ( cluster->GetNext( entry, entry ) < 0 )
                break;
        }
        cluster = source->segment->GetNext( cluster );
    }
    if ( source->videoBlocks.empty() )
    {
        GUCEF_C_ERROR_LOG( plugin->libApi, GUCEF_LOGLEVEL_NORMAL, "videocodecpluginWEBM: the video track has no pictures" );
        CloseSource( source );
        return 0;
    }

    // the frame rate: the default duration of a picture, else the stated rate, else from the times of the pictures
    UInt64 numerator = 0, denominator = 1;
    if ( defaultDuration > 0 )
    {
        numerator = NanoSecondsPerSecond;
        denominator = defaultDuration;
    }
    else
    if ( frameRate > 0.0 )
    {
        numerator = (UInt64) ( frameRate * 1000.0 + 0.5 );
        denominator = 1000;
    }
    else
    if ( source->videoBlocks.size() > 1 && lastTime > firstTime )
    {
        numerator = (UInt64) ( source->videoBlocks.size() - 1 ) * NanoSecondsPerSecond;
        denominator = (UInt64) ( lastTime - firstTime );
    }
    else
    {
        numerator = 25;
    }
    const UInt64 divisor = GreatestCommonDivisor( numerator, denominator );
    numerator /= divisor;
    denominator /= divisor;
    while ( numerator > 0xFFFFFFFFULL || denominator > 0xFFFFFFFFULL )
    {
        numerator = ( numerator + 1 ) / 2;
        denominator = ( denominator + 1 ) / 2;
    }

    format->width = source->width;
    format->height = source->height;
    format->pixelFormat = GUCEF_VIDEOCODEC_PIXELFORMAT_RGBA32;
    format->frameRateNumerator = (UInt32) numerator;
    format->frameRateDenominator = (UInt32) ( 0 == denominator ? 1 : denominator );
    format->audioTrackCount = (UInt32) source->audioTracks.size();
    if ( NULL != frameCount )
        *frameCount = (UInt64) source->videoBlocks.size();
    *sourcedata = source;
    return 1;
}

/*-------------------------------------------------------------------------*/

UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
MODULE_API_FUNCTION( VIDEOCODECPLUG_Src_Get_Audio_Track_Format )( void** plugdata, void** codecdata, void** sourcedata, UInt32 track, TAudioFormatInfo* format ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    SWebmSource* source = static_cast< SWebmSource* >( *sourcedata );
    if ( NULL == source || NULL == format || track >= source->audioTracks.size() )
        return 0;

    format->sampleRate = OpusSampleRate;
    format->channelCount = source->audioTracks[ track ].channels;
    format->sampleType = GUCEF_AUDIOCODEC_SAMPLETYPE_FLOAT32;
    format->channelLayout = 0;
    return 1;
}

/*-------------------------------------------------------------------------*/

UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
MODULE_API_FUNCTION( VIDEOCODECPLUG_Src_Decode_Frame )( void** plugdata, void** codecdata, void** sourcedata, TVideoPictureBuffer* picture, const TVideoAudioOutput* audio, UInt32* endOfStream ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    SWebmSource* source = static_cast< SWebmSource* >( *sourcedata );
    if ( NULL == source || NULL == endOfStream )
        return 0;

    *endOfStream = 0;
    if ( source->position >= source->videoBlocks.size() )
    {
        *endOfStream = 1;
        return 1;
    }
    bool gotPicture = false;
    if ( !DecodeNext( source, picture, audio, gotPicture ) )
        return 0;
    if ( !gotPicture )
    {
        // fewer pictures shown than blocks: the stream ends here
        source->position = source->videoBlocks.size();
        *endOfStream = 1;
        return 1;
    }
    ++source->position;
    return 1;
}

/*-------------------------------------------------------------------------*/

UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
MODULE_API_FUNCTION( VIDEOCODECPLUG_Src_Seek_Frame )( void** plugdata, void** codecdata, void** sourcedata, UInt64 frame ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    SWebmSource* source = static_cast< SWebmSource* >( *sourcedata );
    if ( NULL == source || frame > source->videoBlocks.size() )
        return 0;

    // from the key frame at or before the frame (the first picture is always one)
    size_t keyFrame = (size_t) ( frame < source->videoBlocks.size() ? frame : source->videoBlocks.size() - 1 );
    while ( keyFrame > 0 && !source->blocks[ source->videoBlocks[ keyFrame ] ].entry->GetBlock()->IsKey() )
        --keyFrame;
    const mkvparser::Track* track = source->segment->GetTracks()->GetTrackByNumber( source->videoTrackNumber );
    if ( NULL == track || !ResetDecoders( source, track->GetCodecId() ) )
        return 0;

    // the sound from the key frame on goes with the pictures; what lies before it is not played
    source->nextBlock = source->videoBlocks[ keyFrame ];
    while ( source->nextBlock > 0 && source->blocks[ source->nextBlock - 1 ].trackNumber != source->videoTrackNumber )
        --source->nextBlock;
    source->position = keyFrame;
    for ( size_t t=0; t<source->audioTracks.size(); ++t )
        source->audioTracks[ t ].preSkipLeft = 0 == keyFrame ? source->audioTracks[ t ].preSkip : 0;

    while ( source->position < frame )
    {
        bool gotPicture = false;
        if ( !DecodeNext( source, NULL, NULL, gotPicture ) || !gotPicture )
            return 0;
        ++source->position;
    }
    return 1;
}

/*-------------------------------------------------------------------------*/

void GUCEF_PLUGIN_CALLSPEC_PREFIX
MODULE_API_FUNCTION( VIDEOCODECPLUG_Src_Close )( void** plugdata, void** codecdata, void** sourcedata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    CloseSource( static_cast< SWebmSource* >( *sourcedata ) );
    *sourcedata = NULL;
}

/*-------------------------------------------------------------------------*/

UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
MODULE_API_FUNCTION( VIDEOCODECPLUG_Snk_Open )( void** plugdata, void** codecdata, void** sinkdata, TIOAccess* output, const TVariantMapApi* options, TVideoFormatInfo* format, UInt32 audioTrackCount, TAudioFormatInfo* audioTrackFormats ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    SWebmPlugin* plugin = static_cast< SWebmPlugin* >( *plugdata );
    *sinkdata = NULL;
    if ( NULL == output || NULL == output->write || NULL == output->tell || NULL == output->setpos || NULL == format ||
         0 == format->width || 0 == format->height || 0 == format->frameRateNumerator || 0 == format->frameRateDenominator ||
         ( audioTrackCount > 0 && NULL == audioTrackFormats ) )
        return 0;

    const Int32 quality = GetIntOption( options, "quality", 75, 0, 100 );
    const Int32 speed = GetIntOption( options, "speed", 5, 0, 10 );
    const Int32 threads = GetIntOption( options, "threads", LogicalProcessorCount(), 1, 64 );
    const double frameRate = (double) format->frameRateNumerator / (double) format->frameRateDenominator;
    const Int32 keyFrameInterval = GetIntOption( options, "keyFrameInterval", (Int32) ( frameRate * 5.0 + 0.5 ), 1, 100000 );
    const Int32 audioBitrate = GetIntOption( options, "audioBitrate", 96000, 6000, 510000 );

    SWebmSink* sink = new SWebmSink();
    sink->plugin = plugin;
    sink->writer = new CIOAccessWriter( output );
    sink->segment = new mkvmuxer::Segment();
    sink->width = format->width;
    sink->height = format->height;
    sink->frameRateNumerator = format->frameRateNumerator;
    sink->frameRateDenominator = format->frameRateDenominator;
    sink->encoderOpen = false;
    sink->image = NULL;
    sink->framesIn = 0;
    sink->deadline = speed >= 6 ? VPX_DL_REALTIME : VPX_DL_GOOD_QUALITY;

    // VP9 at a constant quality: 0 .. 100 becomes the quantizer level 63 .. 0
    vpx_codec_iface_t* iface = vpx_codec_vp9_cx();
    vpx_codec_enc_cfg_t config;
    if ( VPX_CODEC_OK != vpx_codec_enc_config_default( iface, &config, 0 ) )
    {
        CloseSink( sink );
        return 0;
    }
    config.g_w = format->width;
    config.g_h = format->height;
    config.g_timebase.num = (int) format->frameRateDenominator;
    config.g_timebase.den = (int) format->frameRateNumerator;
    config.g_threads = (unsigned int) threads;
    config.rc_end_usage = VPX_Q;
    config.rc_min_quantizer = 0;
    config.rc_max_quantizer = 63;
    config.kf_mode = VPX_KF_AUTO;
    config.kf_max_dist = (unsigned int) keyFrameInterval;
    config.g_lag_in_frames = speed >= 6 ? 0 : 16;
    if ( VPX_CODEC_OK != vpx_codec_enc_init( &sink->encoder, iface, &config, 0 ) )
    {
        GUCEF_C_ERROR_LOG( plugin->libApi, GUCEF_LOGLEVEL_NORMAL, "videocodecpluginWEBM: the VP9 encoder cannot be set up" );
        CloseSink( sink );
        return 0;
    }
    sink->encoderOpen = true;
    vpx_codec_control( &sink->encoder, VP8E_SET_CQ_LEVEL, (unsigned int) ( 63 - quality * 63 / 100 ) );
    vpx_codec_control( &sink->encoder, VP8E_SET_CPUUSED, speed > 9 ? 9 : speed );
    vpx_codec_control( &sink->encoder, VP9E_SET_ROW_MT, 1 );
    vpx_codec_control( &sink->encoder, VP9E_SET_TILE_COLUMNS, TileColumnsLog2( format->width, threads ) );
    vpx_codec_control( &sink->encoder, VP8E_SET_ENABLEAUTOALTREF, speed >= 6 ? 0 : 1 );
    vpx_codec_control( &sink->encoder, VP9E_SET_COLOR_SPACE, VPX_CS_BT_601 );
    sink->image = vpx_img_alloc( NULL, VPX_IMG_FMT_I420, format->width, format->height, 16 );
    if ( NULL == sink->image )
    {
        CloseSink( sink );
        return 0;
    }

    // the container: a video track and an Opus track per audio track, cues on the video for seeking
    if ( !sink->segment->Init( sink->writer ) )
    {
        CloseSink( sink );
        return 0;
    }
    sink->segment->set_mode( mkvmuxer::Segment::kFile );
    sink->segment->OutputCues( true );
    sink->segment->GetSegmentInfo()->set_writing_app( "GUCEF videocodecpluginWEBM" );
    sink->videoTrackNumber = sink->segment->AddVideoTrack( (int) format->width, (int) format->height, 0 );
    mkvmuxer::VideoTrack* videoTrack = static_cast< mkvmuxer::VideoTrack* >( sink->segment->GetTrackByNumber( sink->videoTrackNumber ) );
    if ( NULL == videoTrack || !sink->segment->CuesTrack( sink->videoTrackNumber ) )
    {
        CloseSink( sink );
        return 0;
    }
    videoTrack->set_codec_id( mkvmuxer::Tracks::kVp9CodecId );
    videoTrack->set_frame_rate( frameRate );
    videoTrack->set_default_duration( (UInt64) format->frameRateDenominator * NanoSecondsPerSecond / format->frameRateNumerator );

    for ( UInt32 t=0; t<audioTrackCount; ++t )
    {
        SSinkAudioTrack track;
        track.channels = audioTrackFormats[ t ].channelCount > 1 ? 2 : 1;
        track.samplesEncoded = 0;
        int error = 0;
        track.encoder = opus_encoder_create( (opus_int32) OpusSampleRate, (int) track.channels, OPUS_APPLICATION_AUDIO, &error );
        if ( NULL == track.encoder || OPUS_OK != error )
        {
            CloseSink( sink );
            return 0;
        }
        opus_encoder_ctl( track.encoder, OPUS_SET_BITRATE( audioBitrate ) );
        opus_int32 lookahead = 0;
        opus_encoder_ctl( track.encoder, OPUS_GET_LOOKAHEAD( &lookahead ) );
        track.preSkip = (UInt32) lookahead;
        sink->audioTracks.push_back( track );

        UInt8 opusHead[ 19 ];
        memcpy( opusHead, "OpusHead", 8 );
        opusHead[ 8 ] = 1;
        opusHead[ 9 ] = (UInt8) track.channels;
        WriteLE16( opusHead + 10, track.preSkip );
        WriteLE32( opusHead + 12, audioTrackFormats[ t ].sampleRate );
        WriteLE16( opusHead + 16, 0 );
        opusHead[ 18 ] = 0;
        sink->audioTracks.back().trackNumber = sink->segment->AddAudioTrack( (int) OpusSampleRate, (int) track.channels, 0 );
        mkvmuxer::AudioTrack* audioTrack = static_cast< mkvmuxer::AudioTrack* >( sink->segment->GetTrackByNumber( sink->audioTracks.back().trackNumber ) );
        if ( NULL == audioTrack || !audioTrack->SetCodecPrivate( opusHead, sizeof( opusHead ) ) )
        {
            CloseSink( sink );
            return 0;
        }
        audioTrack->set_codec_id( mkvmuxer::Tracks::kOpusCodecId );
        audioTrack->set_codec_delay( (UInt64) track.preSkip * NanoSecondsPerSecond / OpusSampleRate );
        audioTrack->set_seek_pre_roll( 80000000ULL );

        // what the encoder takes: 48 kHz float, one or two channels
        audioTrackFormats[ t ].sampleRate = OpusSampleRate;
        audioTrackFormats[ t ].channelCount = track.channels;
        audioTrackFormats[ t ].sampleType = GUCEF_AUDIOCODEC_SAMPLETYPE_FLOAT32;
        audioTrackFormats[ t ].channelLayout = 0;
    }

    format->pixelFormat = GUCEF_VIDEOCODEC_PIXELFORMAT_RGBA32;
    *sinkdata = sink;
    return 1;
}

/*-------------------------------------------------------------------------*/

UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
MODULE_API_FUNCTION( VIDEOCODECPLUG_Snk_Write_Frame )( void** plugdata, void** codecdata, void** sinkdata, const TVideoPictureBuffer* picture ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    SWebmSink* sink = static_cast< SWebmSink* >( *sinkdata );
    if ( NULL == sink || NULL == picture || NULL == picture->pixels )
        return 0;

    RgbaToImage( static_cast< const UInt8* >( picture->pixels ), picture->pitch, sink->width, sink->height, sink->image );
    if ( VPX_CODEC_OK != vpx_codec_encode( &sink->encoder, sink->image, (vpx_codec_pts_t) sink->framesIn, 1, 0, sink->deadline ) )
    {
        GUCEF_C_ERROR_LOG( sink->plugin->libApi, GUCEF_LOGLEVEL_NORMAL, "videocodecpluginWEBM: a picture cannot be encoded" );
        return 0;
    }
    ++sink->framesIn;
    return CollectVideoPackets( sink ) && WriteQueued( sink, false ) ? 1 : 0;
}

/*-------------------------------------------------------------------------*/

UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
MODULE_API_FUNCTION( VIDEOCODECPLUG_Snk_Write_Audio )( void** plugdata, void** codecdata, void** sinkdata, UInt32 track, const void* frames, UInt32 frameCount ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    SWebmSink* sink = static_cast< SWebmSink* >( *sinkdata );
    if ( NULL == sink || track >= sink->audioTracks.size() )
        return 0;
    if ( NULL == frames || 0 == frameCount )
        return 1;

    SSinkAudioTrack& audioTrack = sink->audioTracks[ track ];
    const Float32* samples = static_cast< const Float32* >( frames );
    audioTrack.pending.insert( audioTrack.pending.end(), samples, samples + (size_t) frameCount * audioTrack.channels );
    return EncodeAudio( sink, audioTrack, false ) && WriteQueued( sink, false ) ? 1 : 0;
}

/*-------------------------------------------------------------------------*/

UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
MODULE_API_FUNCTION( VIDEOCODECPLUG_Snk_Close )( void** plugdata, void** codecdata, void** sinkdata, UInt32 finish ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    SWebmSink* sink = static_cast< SWebmSink* >( *sinkdata );
    if ( NULL == sink )
        return 0;

    bool success = true;
    if ( 0 != finish )
    {
        // what the encoders still hold, then everything in time order, then the cues and the duration
        while ( success )
        {
            if ( VPX_CODEC_OK != vpx_codec_encode( &sink->encoder, NULL, 0, 1, 0, sink->deadline ) )
            {
                success = false;
                break;
            }
            const size_t before = sink->videoQueue.size();
            CollectVideoPackets( sink );
            if ( sink->videoQueue.size() == before )
                break;
        }
        for ( size_t t=0; success && t<sink->audioTracks.size(); ++t )
            success = EncodeAudio( sink, sink->audioTracks[ t ], true );
        success = success && WriteQueued( sink, true ) && sink->segment->Finalize();
        if ( !success )
            GUCEF_C_ERROR_LOG( sink->plugin->libApi, GUCEF_LOGLEVEL_NORMAL, "videocodecpluginWEBM: the file cannot be completed" );
    }
    CloseSink( sink );
    *sinkdata = NULL;
    return success ? 1 : 0;
}

/*-------------------------------------------------------------------------*/

}; /* namespace VIDEO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
