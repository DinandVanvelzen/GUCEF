/*
 *  audiocodecpluginOGG: gucefAUDIO codec plugin for Ogg Vorbis and Ogg Opus audio
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
 *  Ogg Vorbis through vorbisfile (decoding) and libvorbisenc (encoding), both of libvorbis.
 *
 *  vorbisfile seeks to the sample and knows the exact length from the last page of every link. Chained streams
 *  (several Vorbis streams one after the other) play as one as long as the later links have the format of the
 *  first, the stream ends before the first link that has not.
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <vorbis/codec.h>
#include <vorbis/vorbisenc.h>
#include <vorbis/vorbisfile.h>

#ifndef GUCEF_CORE_C_LOGGING_H
#include "gucefCORE_c_logging.h"
#define GUCEF_CORE_C_LOGGING_H
#endif /* GUCEF_CORE_C_LOGGING_H ? */

#include "audiocodecpluginOGG_internal.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#define VORBIS_MAX_CHANNELS         8
#define VORBIS_MAX_HOLES            1000        /* consecutive gaps in the data, then the stream is taken to be broken */
#define VORBIS_ENCODE_CHUNK_FRAMES  1024
#define VORBIS_DEFAULT_SERIAL       0x47554345L

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

typedef struct SOggVorbisDecoder
{
    TOggDecoder base;
    TOggInput* io;
    OggVorbis_File vf;
    int isOpen;
    const UInt8* order;         /* channel of Vorbis order for every channel in WAVE order */
} TOggVorbisDecoder;

/*-------------------------------------------------------------------------//
//                                                                         //
//      DECODER                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

static size_t
OnRead( void* destination, size_t size, size_t count, void* user )
{
    TOggInput* io = (TOggInput*) user;
    if ( 0 == size || 0 == count )
        return 0;
    return OggInputRead( io, destination, size * count ) / size;
}

/*-------------------------------------------------------------------------*/

static int
OnSeek( void* user, ogg_int64_t offset, int whence )
{
    return OggInputSeek( (TOggInput*) user, offset, whence );
}

/*-------------------------------------------------------------------------*/

static long
OnTell( void* user )
{
    return (long) OggInputTell( (TOggInput*) user );
}

/*-------------------------------------------------------------------------*/

static UInt32
VorbisRead( TOggDecoder* decoder, float* destination, UInt32 frames )
{
    TOggVorbisDecoder* self = (TOggVorbisDecoder*) decoder;
    UInt32 done = 0;
    UInt32 holes = 0;
    UInt32 channels = decoder->channels;

    while ( done < frames )
    {
        float** planes = NULL;
        int link = 0;
        UInt32 want = frames - done;
        long got;
        UInt32 f, c;

        if ( want > 4096 )
            want = 4096;
        got = ov_read_float( &self->vf, &planes, (int) want, &link );
        if ( OV_HOLE == got )
        {
            if ( ++holes > VORBIS_MAX_HOLES )
                break;
            continue;
        }
        if ( got <= 0 || NULL == planes )
            break;
        holes = 0;

        if ( (UInt32) got > want )
            got = (long) want;
        for ( f=0; f<(UInt32) got; ++f )
            for ( c=0; c<channels; ++c )
                destination[ (size_t) ( done + f ) * channels + c ] = planes[ self->order[ c ] ][ f ];
        done += (UInt32) got;
    }
    return done;
}

/*-------------------------------------------------------------------------*/

static UInt32
VorbisSeek( TOggDecoder* decoder, UInt64 frame )
{
    TOggVorbisDecoder* self = (TOggVorbisDecoder*) decoder;
    return 0 == ov_pcm_seek( &self->vf, (ogg_int64_t) frame ) ? 1 : 0;
}

/*-------------------------------------------------------------------------*/

static void
VorbisTags( TOggDecoder* decoder, TVariantMapApi* metaData )
{
    TOggVorbisDecoder* self = (TOggVorbisDecoder*) decoder;
    vorbis_comment* comment = ov_comment( &self->vf, 0 );
    if ( NULL != comment )
        OggAddComments( metaData, comment->user_comments, comment->comment_lengths, comment->comments );
}

/*-------------------------------------------------------------------------*/

static void
VorbisClose( TOggDecoder* decoder )
{
    TOggVorbisDecoder* self = (TOggVorbisDecoder*) decoder;
    if ( self->isOpen )
        ov_clear( &self->vf );
    free( self );
}

/*-------------------------------------------------------------------------*/

TOggDecoder*
OggVorbisOpen( const TGucefCoreCApi* libApi, TOggInput* io )
{
    TOggVorbisDecoder* self;
    ov_callbacks callbacks;
    vorbis_info* first;
    long links, i, usable;
    UInt64 length = 0;

    self = (TOggVorbisDecoder*) calloc( 1, sizeof( TOggVorbisDecoder ) );
    if ( NULL == self )
        return NULL;
    self->io = io;

    callbacks.read_func = OnRead;
    callbacks.seek_func = OnSeek;
    callbacks.close_func = NULL;
    callbacks.tell_func = OnTell;
    if ( 0 != ov_open_callbacks( io, &self->vf, NULL, 0, callbacks ) )
    {
        free( self );
        return NULL;
    }
    self->isOpen = 1;

    first = ov_info( &self->vf, 0 );
    if ( NULL == first || first->channels < 1 || first->channels > VORBIS_MAX_CHANNELS || first->rate < 1 || !ov_seekable( &self->vf ) )
    {
        GUCEF_C_ERROR_LOG( (*libApi), GUCEF_LOGLEVEL_NORMAL, "audiocodecpluginOGG: the Vorbis stream has a channel count or sample rate that is not supported, or can not seek" );
        VorbisClose( &self->base );
        return NULL;
    }

    /* the links that have the format of the first make the stream, in total as long as the pages of the links say */
    links = ov_streams( &self->vf );
    usable = 0;
    for ( i=0; i<links; ++i )
    {
        vorbis_info* info = ov_info( &self->vf, (int) i );
        ogg_int64_t total;
        if ( NULL == info || info->channels != first->channels || info->rate != first->rate )
        {
            GUCEF_C_LOG( (*libApi), GUCEF_LOGLEVEL_NORMAL, "audiocodecpluginOGG: a link of the chained Vorbis stream has another format than the first, the stream ends before it" );
            break;
        }
        total = ov_pcm_total( &self->vf, (int) i );
        if ( total < 0 )
            break;
        length += (UInt64) total;
        ++usable;
    }
    if ( 0 == usable )
    {
        VorbisClose( &self->base );
        return NULL;
    }

    /*
     *  After opening a chained stream vorbisfile can be left at the start of a later link, when the first link has
     *  its audio in a single page. Going to the first frame puts it where the audio starts
     */
    if ( links > 1 && 0 != ov_pcm_seek( &self->vf, 0 ) )
    {
        VorbisClose( &self->base );
        return NULL;
    }

    self->order = OggWaveToVorbisOrder( (UInt32) first->channels );
    self->base.read = VorbisRead;
    self->base.seek = VorbisSeek;
    self->base.tags = VorbisTags;
    self->base.close = VorbisClose;
    self->base.channels = (UInt32) first->channels;
    self->base.sampleRate = (UInt32) first->rate;
    self->base.channelLayout = OggWaveLayout( (UInt32) first->channels );
    self->base.length = length;
    return &self->base;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      ENCODER                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

typedef struct SOggVorbisEncoder
{
    vorbis_info vi;
    vorbis_comment vc;
    vorbis_dsp_state vd;
    vorbis_block vb;
    ogg_stream_state os;
    TIOAccess* output;
    int isFailed;
} TOggVorbisEncoder;

/*-------------------------------------------------------------------------*/

static void
WritePage( TOggVorbisEncoder* enc, const ogg_page* page )
{
    if ( !OggWriteBytes( enc->output, page->header, (size_t) page->header_len ) || !OggWriteBytes( enc->output, page->body, (size_t) page->body_len ) )
        enc->isFailed = 1;
}

/*-------------------------------------------------------------------------*/

/** Takes the finished blocks out of the encoder and the finished pages out of the stream */
static void
Drain( TOggVorbisEncoder* enc )
{
    ogg_packet packet;
    ogg_page page;

    while ( 1 == vorbis_analysis_blockout( &enc->vd, &enc->vb ) )
    {
        vorbis_analysis( &enc->vb, NULL );
        vorbis_bitrate_addblock( &enc->vb );
        while ( vorbis_bitrate_flushpacket( &enc->vd, &packet ) )
        {
            ogg_stream_packetin( &enc->os, &packet );
            while ( ogg_stream_pageout( &enc->os, &page ) )
                WritePage( enc, &page );
        }
    }
}

/*-------------------------------------------------------------------------*/

UInt32
OggVorbisEncode( const TGucefCoreCApi* libApi, const TAudioFormatInfo* format, const void* frames, UInt64 frameCount, TIOAccess* output, const TVariantMapApi* options )
{
    TOggVorbisEncoder enc;
    const UInt8* data = (const UInt8*) frames;
    UInt32 channels = format->channelCount;
    const UInt8* order;
    UInt32 bytesPerSample = OggBytesPerSample( format->sampleType );
    double quality, bitrate;
    long serial;
    int ret;
    UInt64 position;
    const char* lines;
    TOggTagLine line;
    ogg_packet header, headerComment, headerCode;
    ogg_page page;

    if ( NULL == output || NULL == output->write || 0 == bytesPerSample || channels < 1 || channels > VORBIS_MAX_CHANNELS ||
         format->sampleRate < 1 || ( NULL == frames && 0 != frameCount ) )
    {
        GUCEF_C_ERROR_LOG( (*libApi), GUCEF_LOGLEVEL_NORMAL, "audiocodecpluginOGG: the audio can not be written as Ogg Vorbis (1 to 8 channels)" );
        return 0;
    }

    quality = OggGetNumberOption( options, "quality", 0.5 );
    if ( quality < -0.1 )
        quality = -0.1;
    if ( quality > 1.0 )
        quality = 1.0;
    bitrate = OggGetNumberOption( options, "bitrate", 0.0 );
    serial = (long) OggGetNumberOption( options, "serial", (double) VORBIS_DEFAULT_SERIAL );
    order = OggWaveToVorbisOrder( channels );

    memset( &enc, 0, sizeof( enc ) );
    enc.output = output;
    vorbis_info_init( &enc.vi );
    if ( bitrate > 0.0 )
        ret = vorbis_encode_init( &enc.vi, (long) channels, (long) format->sampleRate, -1, (long) bitrate, -1 );
    else
        ret = vorbis_encode_init_vbr( &enc.vi, (long) channels, (long) format->sampleRate, (float) quality );
    if ( 0 != ret )
    {
        GUCEF_C_ERROR_LOG( (*libApi), GUCEF_LOGLEVEL_NORMAL, "audiocodecpluginOGG: the Vorbis encoder does not take this sample rate, channel count or quality" );
        vorbis_info_clear( &enc.vi );
        return 0;
    }

    vorbis_comment_init( &enc.vc );
    lines = OggGetTextOption( options, "tags" );
    while ( NULL != ( lines = OggNextTagLine( lines, &line ) ) )
    {
        char* name = (char*) malloc( line.nameLength + 1 );
        char* value = (char*) malloc( line.valueLength + 1 );
        if ( NULL != name && NULL != value )
        {
            memcpy( name, line.name, line.nameLength );
            name[ line.nameLength ] = '\0';
            memcpy( value, line.value, line.valueLength );
            value[ line.valueLength ] = '\0';
            vorbis_comment_add_tag( &enc.vc, name, value );
        }
        free( name );
        free( value );
    }

    vorbis_analysis_init( &enc.vd, &enc.vi );
    vorbis_block_init( &enc.vd, &enc.vb );
    ogg_stream_init( &enc.os, serial );

    /* the three header packets, each page that comes out of them is complete before the audio starts */
    vorbis_analysis_headerout( &enc.vd, &enc.vc, &header, &headerComment, &headerCode );
    ogg_stream_packetin( &enc.os, &header );
    ogg_stream_packetin( &enc.os, &headerComment );
    ogg_stream_packetin( &enc.os, &headerCode );
    while ( ogg_stream_flush( &enc.os, &page ) )
        WritePage( &enc, &page );

    for ( position=0; position<frameCount && !enc.isFailed; position += VORBIS_ENCODE_CHUNK_FRAMES )
    {
        UInt32 n = ( frameCount - position < VORBIS_ENCODE_CHUNK_FRAMES ) ? (UInt32) ( frameCount - position ) : VORBIS_ENCODE_CHUNK_FRAMES;
        float** buffer = vorbis_analysis_buffer( &enc.vd, (int) n );
        UInt32 i, c;
        for ( c=0; c<channels; ++c )
            for ( i=0; i<n; ++i )
                buffer[ order[ c ] ][ i ] = OggGetSampleAsFloat( data, format->sampleType, ( position + i ) * channels + c );
        vorbis_analysis_wrote( &enc.vd, (int) n );
        Drain( &enc );
    }

    /* the end of the stream, which also tells the decoder how many samples of the last block count */
    vorbis_analysis_wrote( &enc.vd, 0 );
    Drain( &enc );
    while ( ogg_stream_flush( &enc.os, &page ) )
        WritePage( &enc, &page );

    ogg_stream_clear( &enc.os );
    vorbis_block_clear( &enc.vb );
    vorbis_dsp_clear( &enc.vd );
    vorbis_comment_clear( &enc.vc );
    vorbis_info_clear( &enc.vi );
    return enc.isFailed ? 0 : 1;
}

/*-------------------------------------------------------------------------*/
