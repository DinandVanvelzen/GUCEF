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
 *  Ogg Opus (RFC 7845) through opusfile for decoding and libopus plus a small writer on libogg for encoding.
 *
 *  Opus always decodes at 48 kHz. opusfile applies the pre-skip and the output gain of the header and seeks to
 *  the sample, with the decoder warmed up on the audio before it. Mapping families 0 (mono, stereo) and 1 (up to
 *  8 channels in Vorbis order) are supported. Chained streams play as one as long as the later links have the
 *  channel count of the first, the stream ends before the first link that has not.
 */

/* The whole file is for builds that have the Opus libraries */
#ifdef GUCEF_AUDIOCODECPLUGIN_OGG_HAVE_OPUS

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <stdlib.h>
#include <string.h>

#include <ogg/ogg.h>
#include <opus.h>
#include <opus_multistream.h>
#include <opusfile.h>

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

#define OPUS_MAX_CHANNELS           8
#define OPUS_MAX_HOLES              1000
#define OPUS_RATE                   48000
#define OPUS_FRAME_SIZE             960             /* 20 ms at 48 kHz */
#define OPUS_MAX_PACKET_BYTES       32768
#define OPUS_DEFAULT_SERIAL         0x47554345L
#define OPUS_VENDOR                 "GUCEF audiocodecpluginOGG"

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

typedef struct SOggOpusDecoder
{
    TOggDecoder base;
    TOggInput* io;
    OggOpusFile* file;
    const UInt8* order;         /* channel of Vorbis order for every channel in WAVE order, family 1 only */
    int isVorbisOrder;
    float* scratch;             /* one frame while the channels are put in the other order */
} TOggOpusDecoder;

/*-------------------------------------------------------------------------//
//                                                                         //
//      DECODER                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

static int
OnRead( void* user, unsigned char* destination, int bytes )
{
    TOggInput* io = (TOggInput*) user;
    if ( bytes <= 0 )
        return 0;
    return (int) OggInputRead( io, destination, (size_t) bytes );
}

/*-------------------------------------------------------------------------*/

static int
OnSeek( void* user, opus_int64 offset, int whence )
{
    return OggInputSeek( (TOggInput*) user, offset, whence );
}

/*-------------------------------------------------------------------------*/

static opus_int64
OnTell( void* user )
{
    return OggInputTell( (TOggInput*) user );
}

/*-------------------------------------------------------------------------*/

static UInt32
OpusRead( TOggDecoder* decoder, float* destination, UInt32 frames )
{
    TOggOpusDecoder* self = (TOggOpusDecoder*) decoder;
    UInt32 done = 0;
    UInt32 holes = 0;
    UInt32 channels = decoder->channels;

    while ( done < frames )
    {
        UInt32 want = frames - done;
        float* target = destination + (size_t) done * channels;
        int got;

        if ( want > 5760 )
            want = 5760;
        got = op_read_float( self->file, target, (int) ( want * channels ), NULL );
        if ( OP_HOLE == got )
        {
            if ( ++holes > OPUS_MAX_HOLES )
                break;
            continue;
        }
        if ( got <= 0 )
            break;
        holes = 0;

        if ( (UInt32) got > want )
            got = (int) want;
        if ( self->isVorbisOrder )
        {
            UInt32 f, c;
            for ( f=0; f<(UInt32) got; ++f )
            {
                float* frame = target + (size_t) f * channels;
                for ( c=0; c<channels; ++c )
                    self->scratch[ c ] = frame[ self->order[ c ] ];
                for ( c=0; c<channels; ++c )
                    frame[ c ] = self->scratch[ c ];
            }
        }
        done += (UInt32) got;
    }
    return done;
}

/*-------------------------------------------------------------------------*/

static UInt32
OpusSeek( TOggDecoder* decoder, UInt64 frame )
{
    TOggOpusDecoder* self = (TOggOpusDecoder*) decoder;
    return 0 == op_pcm_seek( self->file, (ogg_int64_t) frame ) ? 1 : 0;
}

/*-------------------------------------------------------------------------*/

static void
OpusTagsOfStream( TOggDecoder* decoder, TVariantMapApi* metaData )
{
    TOggOpusDecoder* self = (TOggOpusDecoder*) decoder;
    const OpusTags* tags = op_tags( self->file, 0 );
    if ( NULL != tags )
        OggAddComments( metaData, tags->user_comments, tags->comment_lengths, tags->comments );
}

/*-------------------------------------------------------------------------*/

static void
OpusClose( TOggDecoder* decoder )
{
    TOggOpusDecoder* self = (TOggOpusDecoder*) decoder;
    if ( NULL != self->file )
        op_free( self->file );
    free( self->scratch );
    free( self );
}

/*-------------------------------------------------------------------------*/

TOggDecoder*
OggOpusOpen( const TGucefCoreCApi* libApi, TOggInput* io )
{
    TOggOpusDecoder* self;
    OpusFileCallbacks callbacks;
    const OpusHead* head;
    int error = 0;
    int links, i, channels, usable;
    UInt64 length = 0;

    self = (TOggOpusDecoder*) calloc( 1, sizeof( TOggOpusDecoder ) );
    if ( NULL == self )
        return NULL;
    self->io = io;

    callbacks.read = OnRead;
    callbacks.seek = OnSeek;
    callbacks.tell = OnTell;
    callbacks.close = NULL;
    self->file = op_open_callbacks( io, &callbacks, NULL, 0, &error );
    if ( NULL == self->file )
    {
        free( self );
        return NULL;
    }

    head = op_head( self->file, 0 );
    channels = op_channel_count( self->file, 0 );
    if ( NULL == head || channels < 1 || channels > OPUS_MAX_CHANNELS || ( 0 != head->mapping_family && 1 != head->mapping_family ) ||
         ( 0 == head->mapping_family && channels > 2 ) || !op_seekable( self->file ) )
    {
        GUCEF_C_ERROR_LOG( (*libApi), GUCEF_LOGLEVEL_NORMAL, "audiocodecpluginOGG: the Opus stream has a channel mapping that is not supported (only the families 0 and 1 with up to 8 channels), or can not seek" );
        OpusClose( &self->base );
        return NULL;
    }

    links = op_link_count( self->file );
    usable = 0;
    for ( i=0; i<links; ++i )
    {
        ogg_int64_t total;
        if ( op_channel_count( self->file, i ) != channels )
        {
            GUCEF_C_LOG( (*libApi), GUCEF_LOGLEVEL_NORMAL, "audiocodecpluginOGG: a link of the chained Opus stream has another channel count than the first, the stream ends before it" );
            break;
        }
        total = op_pcm_total( self->file, i );
        if ( total < 0 )
            break;
        length += (UInt64) total;
        ++usable;
    }
    if ( 0 == usable )
    {
        OpusClose( &self->base );
        return NULL;
    }

    self->isVorbisOrder = 1 == head->mapping_family && channels > 2;
    self->order = OggWaveToVorbisOrder( (UInt32) channels );
    if ( self->isVorbisOrder )
    {
        self->scratch = (float*) malloc( sizeof( float ) * OPUS_MAX_CHANNELS );
        if ( NULL == self->scratch )
        {
            OpusClose( &self->base );
            return NULL;
        }
    }

    self->base.read = OpusRead;
    self->base.seek = OpusSeek;
    self->base.tags = OpusTagsOfStream;
    self->base.close = OpusClose;
    self->base.channels = (UInt32) channels;
    self->base.sampleRate = OPUS_RATE;
    self->base.channelLayout = OggWaveLayout( (UInt32) channels );
    self->base.length = length;
    return &self->base;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      ENCODER                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

static void
PutLE16( UInt8* out, UInt32 value )
{
    out[ 0 ] = (UInt8) ( value & 0xFF );
    out[ 1 ] = (UInt8) ( ( value >> 8 ) & 0xFF );
}

/*-------------------------------------------------------------------------*/

static void
PutLE32( UInt8* out, UInt32 value )
{
    out[ 0 ] = (UInt8) ( value & 0xFF );
    out[ 1 ] = (UInt8) ( ( value >> 8 ) & 0xFF );
    out[ 2 ] = (UInt8) ( ( value >> 16 ) & 0xFF );
    out[ 3 ] = (UInt8) ( ( value >> 24 ) & 0xFF );
}

/*-------------------------------------------------------------------------*/

static int
WritePage( TIOAccess* output, const ogg_page* page )
{
    return OggWriteBytes( output, page->header, (size_t) page->header_len ) && OggWriteBytes( output, page->body, (size_t) page->body_len );
}

/*-------------------------------------------------------------------------*/

/** Writes the pages the stream has ready. With 'flush' also the page that is not full yet */
static int
WritePages( ogg_stream_state* os, TIOAccess* output, int flush )
{
    ogg_page page;
    for ( ;; )
    {
        int got = flush ? ogg_stream_flush( os, &page ) : ogg_stream_pageout( os, &page );
        if ( 0 == got )
            return 1;
        if ( !WritePage( output, &page ) )
            return 0;
    }
}

/*-------------------------------------------------------------------------*/

UInt32
OggOpusEncode( const TGucefCoreCApi* libApi, const TAudioFormatInfo* format, const void* frames, UInt64 frameCount, TIOAccess* output, const TVariantMapApi* options )
{
    const UInt8* data = (const UInt8*) frames;
    UInt32 channels = format->channelCount;
    UInt32 bytesPerSample = OggBytesPerSample( format->sampleType );
    const UInt8* order = OggWaveToVorbisOrder( channels );
    OpusEncoder* single = NULL;
    OpusMSEncoder* multi = NULL;
    unsigned char mapping[ OPUS_MAX_CHANNELS ];
    int streams = 1, coupled = channels > 1 ? 1 : 0;
    int error = 0, lookahead = 0;
    opus_int32 bitrate;
    float* pcm = NULL;
    unsigned char* packet = NULL;
    ogg_stream_state os;
    int isStreamOpen = 0;
    ogg_packet op;
    UInt8 headBuffer[ 19 + 2 + OPUS_MAX_CHANNELS ];
    UInt32 headSize;
    UInt8* tagsBuffer = NULL;
    size_t tagsSize, tagsCapacity;
    const char* lines;
    TOggTagLine line;
    UInt32 tagCount = 0;
    UInt64 packetCount, totalPacketsOut, p;
    UInt32 preSkip;
    UInt32 c, i;
    UInt32 result = 0;
    long serial;

    if ( NULL == output || NULL == output->write || 0 == bytesPerSample || channels < 1 || channels > OPUS_MAX_CHANNELS ||
         OPUS_RATE != format->sampleRate || ( NULL == frames && 0 != frameCount ) )
    {
        GUCEF_C_ERROR_LOG( (*libApi), GUCEF_LOGLEVEL_NORMAL, "audiocodecpluginOGG: the audio can not be written as Ogg Opus (48000 Hz, 1 to 8 channels)" );
        return 0;
    }

    serial = (long) OggGetNumberOption( options, "serial", (double) OPUS_DEFAULT_SERIAL );
    bitrate = (opus_int32) OggGetNumberOption( options, "bitrate", 64000.0 * (double) channels );

    /* mono and stereo as a plain stream (family 0), more channels as a surround layout (family 1, Vorbis order) */
    if ( channels <= 2 )
    {
        single = opus_encoder_create( OPUS_RATE, (int) channels, OPUS_APPLICATION_AUDIO, &error );
        if ( NULL == single || OPUS_OK != error )
            goto cleanup;
        opus_encoder_ctl( single, OPUS_SET_BITRATE( bitrate ) );
        opus_encoder_ctl( single, OPUS_GET_LOOKAHEAD( &lookahead ) );
    }
    else
    {
        multi = opus_multistream_surround_encoder_create( OPUS_RATE, (int) channels, 1, &streams, &coupled, mapping, OPUS_APPLICATION_AUDIO, &error );
        if ( NULL == multi || OPUS_OK != error )
            goto cleanup;
        opus_multistream_encoder_ctl( multi, OPUS_SET_BITRATE( bitrate ) );
        opus_multistream_encoder_ctl( multi, OPUS_GET_LOOKAHEAD( &lookahead ) );
    }
    preSkip = lookahead > 0 ? (UInt32) lookahead : 0;

    pcm = (float*) malloc( sizeof( float ) * OPUS_FRAME_SIZE * channels );
    packet = (unsigned char*) malloc( OPUS_MAX_PACKET_BYTES );
    if ( NULL == pcm || NULL == packet )
        goto cleanup;
    ogg_stream_init( &os, serial );
    isStreamOpen = 1;

    /* OpusHead */
    memcpy( headBuffer, "OpusHead", 8 );
    headBuffer[ 8 ] = 1;
    headBuffer[ 9 ] = (UInt8) channels;
    PutLE16( headBuffer + 10, preSkip );
    PutLE32( headBuffer + 12, OPUS_RATE );
    PutLE16( headBuffer + 16, 0 );
    headBuffer[ 18 ] = channels <= 2 ? 0 : 1;
    headSize = 19;
    if ( channels > 2 )
    {
        headBuffer[ headSize++ ] = (UInt8) streams;
        headBuffer[ headSize++ ] = (UInt8) coupled;
        for ( c=0; c<channels; ++c )
            headBuffer[ headSize++ ] = mapping[ c ];
    }
    memset( &op, 0, sizeof( op ) );
    op.packet = headBuffer;
    op.bytes = (long) headSize;
    op.b_o_s = 1;
    op.e_o_s = 0;
    op.granulepos = 0;
    op.packetno = 0;
    ogg_stream_packetin( &os, &op );
    if ( !WritePages( &os, output, 1 ) )
        goto cleanup;

    /* OpusTags: the vendor and the comments */
    tagsCapacity = 8 + 4 + strlen( OPUS_VENDOR ) + 4;
    lines = OggGetTextOption( options, "tags" );
    {
        const char* scan = lines;
        while ( NULL != ( scan = OggNextTagLine( scan, &line ) ) )
            tagsCapacity += 4 + line.nameLength + 1 + line.valueLength;
    }
    tagsBuffer = (UInt8*) malloc( tagsCapacity );
    if ( NULL == tagsBuffer )
        goto cleanup;
    memcpy( tagsBuffer, "OpusTags", 8 );
    tagsSize = 8;
    PutLE32( tagsBuffer + tagsSize, (UInt32) strlen( OPUS_VENDOR ) );
    tagsSize += 4;
    memcpy( tagsBuffer + tagsSize, OPUS_VENDOR, strlen( OPUS_VENDOR ) );
    tagsSize += strlen( OPUS_VENDOR );
    tagsSize += 4;      /* the number of comments, filled in below */
    while ( NULL != ( lines = OggNextTagLine( lines, &line ) ) )
    {
        PutLE32( tagsBuffer + tagsSize, (UInt32) ( line.nameLength + 1 + line.valueLength ) );
        tagsSize += 4;
        memcpy( tagsBuffer + tagsSize, line.name, line.nameLength );
        tagsSize += line.nameLength;
        tagsBuffer[ tagsSize++ ] = '=';
        memcpy( tagsBuffer + tagsSize, line.value, line.valueLength );
        tagsSize += line.valueLength;
        ++tagCount;
    }
    PutLE32( tagsBuffer + 8 + 4 + strlen( OPUS_VENDOR ), tagCount );
    op.packet = tagsBuffer;
    op.bytes = (long) tagsSize;
    op.b_o_s = 0;
    op.e_o_s = 0;
    op.granulepos = 0;
    op.packetno = 1;
    ogg_stream_packetin( &os, &op );
    if ( !WritePages( &os, output, 1 ) )
        goto cleanup;

    /*
     *  The audio: the decoder drops 'preSkip' frames at the start, so the encoded frames have to reach as far as
     *  the audio plus those. The last packet says how far the audio really goes (end trimming)
     */
    packetCount = ( frameCount + preSkip + OPUS_FRAME_SIZE - 1 ) / OPUS_FRAME_SIZE;
    totalPacketsOut = packetCount;
    for ( p=0; p<packetCount; ++p )
    {
        UInt64 start = p * OPUS_FRAME_SIZE;
        int bytes;

        for ( i=0; i<OPUS_FRAME_SIZE; ++i )
        {
            UInt64 frame = start + i;
            for ( c=0; c<channels; ++c )
                pcm[ (size_t) i * channels + order[ c ] ] = frame < frameCount ? OggGetSampleAsFloat( data, format->sampleType, frame * channels + c ) : 0.0f;
        }

        if ( NULL != single )
            bytes = opus_encode_float( single, pcm, OPUS_FRAME_SIZE, packet, OPUS_MAX_PACKET_BYTES );
        else
            bytes = opus_multistream_encode_float( multi, pcm, OPUS_FRAME_SIZE, packet, OPUS_MAX_PACKET_BYTES );
        if ( bytes < 0 )
        {
            GUCEF_C_ERROR_LOG( (*libApi), GUCEF_LOGLEVEL_NORMAL, "audiocodecpluginOGG: the Opus encoder failed" );
            goto cleanup;
        }

        memset( &op, 0, sizeof( op ) );
        op.packet = packet;
        op.bytes = bytes;
        op.b_o_s = 0;
        op.e_o_s = ( p + 1 == totalPacketsOut ) ? 1 : 0;
        op.granulepos = op.e_o_s ? (ogg_int64_t) ( preSkip + frameCount ) : (ogg_int64_t) ( ( p + 1 ) * OPUS_FRAME_SIZE );
        op.packetno = (ogg_int64_t) ( p + 2 );
        ogg_stream_packetin( &os, &op );
        if ( !WritePages( &os, output, 0 ) )
            goto cleanup;
    }
    if ( !WritePages( &os, output, 1 ) )
        goto cleanup;
    result = 1;

cleanup:
    if ( isStreamOpen )
        ogg_stream_clear( &os );
    if ( NULL != single )
        opus_encoder_destroy( single );
    if ( NULL != multi )
        opus_multistream_encoder_destroy( multi );
    free( tagsBuffer );
    free( packet );
    free( pcm );
    if ( 0 == result )
    {
        GUCEF_C_ERROR_LOG( (*libApi), GUCEF_LOGLEVEL_NORMAL, "audiocodecpluginOGG: writing Ogg Opus failed" );
    }
    return result;
}

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIOCODECPLUGIN_OGG_HAVE_OPUS ? */

/*-------------------------------------------------------------------------*/
