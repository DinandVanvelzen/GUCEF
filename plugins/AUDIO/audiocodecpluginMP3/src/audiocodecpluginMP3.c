/*
 *  audiocodecpluginMP3: gucefAUDIO codec plugin for MP3 audio
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
 *  MP3 (MPEG 1, 2 and 2.5 audio layer III) through the gucefAUDIO codec plugin C interface. Decoding is done by
 *  dr_mp3 (dependencies/dr_libs), which reads and seeks through the TIOAccess of the stream. Decoding only: GUCEF
 *  has no MP3 encoder.
 *
 *  Frames come out as 32 bit float samples. The length is known when the file starts with a Xing or Info frame
 *  that holds the frame count (and the LAME tag then tells the encoder delay and padding, which are trimmed, so
 *  loops are gapless). Without it the length is unknown, unless the option "mp3.seekTable" is "onOpen": the file is
 *  then scanned once when it is opened, which gives the exact length and a seek table, so seeks into long music do
 *  not decode from the start. Seeking is sample exact either way.
 *
 *  The ID3v2 tag at the start is read here and not by dr_mp3, so that the size an attacker puts in it can not make
 *  the decoder allocate that much. Tags are the text frames of ID3v2 and the fields of ID3v1 with lower case names.
 *
 *  A stream uses nothing but its own state and its own input, so streams decode at the same time
 *  (AUDIOCODECPLUG_Threading_Model).
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <stdlib.h>
#include <string.h>

#ifndef GUCEF_CORE_C_LOGGING_H
#include "gucefCORE_c_logging.h"
#define GUCEF_CORE_C_LOGGING_H
#endif /* GUCEF_CORE_C_LOGGING_H ? */

#include "audiocodecpluginMP3.h"
#include "audiocodecpluginMP3_internal.h"

/* The decoder is part of this plugin: this is the one source file of the module that holds the implementation of dr_mp3 */
#define DR_MP3_IMPLEMENTATION
#define DR_MP3_NO_STDIO
#define DR_MP3_FLOAT_OUTPUT
#include "dr_mp3.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#define MP3_CODEC_TYPE              "mp3"
#define MP3_MAX_ID3V2_BYTES         ( 1024U * 1024U )   /* a larger tag is skipped without reading its text */
#define MP3_MAX_ID3V2_TAGS          4U                  /* tags in a row at the start of a file that are skipped */
#define MP3_SEEK_POINTS             512U                /* seek table size with "mp3.seekTable" = "onOpen" */
#define MP3_MAX_PCM_FRAMES_PER_BYTE 48U                 /* 24 byte frames of 1152 samples are the densest there are */

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

typedef struct SMp3Plugin
{
    TGucefCoreCApi libApi;
} TMp3Plugin;

typedef struct SMp3Source
{
    TIOAccess* input;
    UInt64 start;               /* where the MP3 data starts, after the ID3v2 tag */
    UInt64 size;                /* bytes from 'start' to the end, 0 when the input does not tell */
    drmp3 mp3;
    int isMp3Open;
    UInt32 channels;
    UInt64 length;              /* GUCEF_AUDIOCODEC_LENGTH_UNKNOWN when not known */
    UInt64 position;
    int isAtEnd;                /* sought to the very end, nothing to read until the next seek */
    drmp3_seek_point* seekPoints;
    UInt8* id3v2;
    UInt32 id3v2Size;
    UInt8 id3v1[ 128 ];
    int hasId3v1;
} TMp3Source;

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

static const char* g_codecTypes[] = { MP3_CODEC_TYPE };

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static size_t
OnRead( void* user, void* out, size_t bytesToRead )
{
    TMp3Source* source = (TMp3Source*) user;
    size_t done = 0;

    while ( done < bytesToRead )
    {
        size_t left = bytesToRead - done;
        UInt32 chunk = left > 0x40000000UL ? 0x40000000UL : (UInt32) left;
        UInt32 got = source->input->read( source->input, ( (UInt8*) out ) + done, 1, chunk );
        done += got;
        if ( got < chunk )
            break;
    }
    return done;
}

/*-------------------------------------------------------------------------*/

static drmp3_bool32
OnSeek( void* user, int offset, drmp3_seek_origin origin )
{
    TMp3Source* source = (TMp3Source*) user;
    Int64 target;

    switch ( origin )
    {
        case DRMP3_SEEK_SET: target = offset; break;
        case DRMP3_SEEK_CUR: target = (Int64) ( source->input->tell( source->input ) - source->start ) + offset; break;
        case DRMP3_SEEK_END:
        {
            if ( 0 == source->size )
                return DRMP3_FALSE;
            target = (Int64) source->size + offset;
            break;
        }
        default: return DRMP3_FALSE;
    }

    if ( target < 0 || ( 0 != source->size && (UInt64) target > source->size ) )
        return DRMP3_FALSE;
    return 0 == source->input->setpos( source->input, source->start + (UInt64) target ) ? DRMP3_TRUE : DRMP3_FALSE;
}

/*-------------------------------------------------------------------------*/

static drmp3_bool32
OnTell( void* user, drmp3_int64* cursor )
{
    TMp3Source* source = (TMp3Source*) user;
    UInt64 position = source->input->tell( source->input );
    if ( position < source->start )
        return DRMP3_FALSE;
    *cursor = (drmp3_int64) ( position - source->start );
    return DRMP3_TRUE;
}

/*-------------------------------------------------------------------------*/

/** dr_mp3 reports tags as raw data while it opens. ID3v2 is not left to it, so this is the ID3v1 tag */
static void
OnMeta( void* user, const drmp3_metadata* meta )
{
    TMp3Source* source = (TMp3Source*) user;
    if ( DRMP3_METADATA_TYPE_ID3V1 == meta->type && 128 == meta->rawDataSize && !source->hasId3v1 )
    {
        memcpy( source->id3v1, meta->pRawData, 128 );
        source->hasId3v1 = 1;
    }
}

/*-------------------------------------------------------------------------*/

static const char*
GetTextOption( const TVariantMapApi* map, const char* key )
{
    TVariantData keyData;
    TVariantData valueData;

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

static void
CloseSource( TMp3Source* source )
{
    if ( NULL == source )
        return;
    if ( source->isMp3Open )
        drmp3_uninit( &source->mp3 );
    free( source->seekPoints );
    free( source->id3v2 );
    free( source );
}

/*-------------------------------------------------------------------------*/

/**
 *  Reads the ID3v2 tags at the current position of the input, keeps the first one for AUDIOCODECPLUG_Src_Meta_Data
 *  and moves the start of the stream to after them
 */
static void
SkipId3v2Tags( TMp3Source* source )
{
    UInt32 count;

    for ( count=0; count<MP3_MAX_ID3V2_TAGS; ++count )
    {
        UInt8 header[ 10 ];
        UInt32 tagSize, total;
        UInt64 here = source->input->tell( source->input );

        if ( 10 != source->input->read( source->input, header, 1, 10 ) || 0 != memcmp( header, "ID3", 3 ) ||
             0xFF == header[ 3 ] || 0xFF == header[ 4 ] || ( header[ 6 ] | header[ 7 ] | header[ 8 ] | header[ 9 ] ) & 0x80 )
        {
            source->input->setpos( source->input, here );
            return;
        }

        tagSize = ( (UInt32) header[ 6 ] << 21 ) | ( (UInt32) header[ 7 ] << 14 ) | ( (UInt32) header[ 8 ] << 7 ) | (UInt32) header[ 9 ];
        total = 10U + tagSize + ( ( header[ 5 ] & 0x10 ) ? 10U : 0U );

        if ( NULL == source->id3v2 && total <= MP3_MAX_ID3V2_BYTES )
        {
            UInt8* tag = (UInt8*) malloc( total );
            if ( NULL != tag )
            {
                memcpy( tag, header, 10 );
                if ( total - 10 == source->input->read( source->input, tag + 10, 1, total - 10 ) )
                {
                    source->id3v2 = tag;
                    source->id3v2Size = total;
                }
                else
                    free( tag );
            }
        }
        source->start = here + total;
        if ( 0 != source->input->setpos( source->input, source->start ) )
            return;
    }
}

/*-------------------------------------------------------------------------*/

/** Opens a stream on the input at its current position. NULL when it is not MP3 that can be decoded */
static TMp3Source*
OpenSource( const TMp3Plugin* plugin, TIOAccess* input, const TVariantMapApi* options, TAudioFormatInfo* format, UInt64* lengthInFrames )
{
    TMp3Source* source;
    UInt64 end;
    const char* seekTableOption;

    if ( NULL == input || NULL == input->read || NULL == input->tell || NULL == input->setpos )
        return NULL;

    source = (TMp3Source*) calloc( 1, sizeof( TMp3Source ) );
    if ( NULL == source )
        return NULL;
    source->input = input;
    source->start = input->tell( input );

    SkipId3v2Tags( source );
    end = NULL != input->size ? input->size( input ) : 0;
    source->size = end > source->start ? end - source->start : 0;

    if ( !drmp3_init( &source->mp3, OnRead, OnSeek, OnTell, OnMeta, source, NULL ) )
    {
        free( source->id3v2 );
        free( source );
        return NULL;
    }
    source->isMp3Open = 1;

    if ( source->mp3.channels < 1 || source->mp3.channels > 2 || 0 == source->mp3.sampleRate )
    {
        GUCEF_C_ERROR_LOG( plugin->libApi, GUCEF_LOGLEVEL_NORMAL, "audiocodecpluginMP3: the stream has a channel count or sample rate that is not supported" );
        CloseSource( source );
        return NULL;
    }
    source->channels = source->mp3.channels;

    /* the length from the Xing or Info frame, when it says something that the size of the file allows */
    source->length = GUCEF_AUDIOCODEC_LENGTH_UNKNOWN;
    if ( DRMP3_UINT64_MAX != source->mp3.totalPCMFrameCount )
    {
        UInt64 total = source->mp3.totalPCMFrameCount;
        UInt64 trim = (UInt64) source->mp3.delayInPCMFrames + source->mp3.paddingInPCMFrames;
        if ( 0 != source->size && total > (UInt64) source->size * MP3_MAX_PCM_FRAMES_PER_BYTE + 2304 )
        {
            GUCEF_C_LOG( plugin->libApi, GUCEF_LOGLEVEL_NORMAL, "audiocodecpluginMP3: the frame count in the Xing frame is more than the file can hold, ignoring it" );
        }
        else
            source->length = total > trim ? total - trim : 0;
    }

    /* an exact length and a seek table, for a price of reading the file once */
    seekTableOption = GetTextOption( options, "mp3.seekTable" );
    if ( NULL != seekTableOption && 0 == strcmp( seekTableOption, "onOpen" ) && 0 != source->size )
    {
        drmp3_uint64 mp3Frames = 0;
        drmp3_uint64 pcmFrames = 0;
        if ( drmp3_get_mp3_and_pcm_frame_count( &source->mp3, &mp3Frames, &pcmFrames ) && mp3Frames > 0 )
        {
            drmp3_uint32 pointCount = MP3_SEEK_POINTS;
            source->seekPoints = (drmp3_seek_point*) malloc( sizeof( drmp3_seek_point ) * pointCount );
            if ( NULL != source->seekPoints )
            {
                if ( drmp3_calculate_seek_points( &source->mp3, &pointCount, source->seekPoints ) )
                    drmp3_bind_seek_table( &source->mp3, pointCount, source->seekPoints );
                else
                {
                    free( source->seekPoints );
                    source->seekPoints = NULL;
                }
            }
            if ( GUCEF_AUDIOCODEC_LENGTH_UNKNOWN == source->length )
                source->length = pcmFrames > source->mp3.delayInPCMFrames ? pcmFrames - source->mp3.delayInPCMFrames : 0;
        }
        else
        {
            GUCEF_C_ERROR_LOG( plugin->libApi, GUCEF_LOGLEVEL_NORMAL, "audiocodecpluginMP3: could not scan the stream for a seek table" );
            CloseSource( source );
            return NULL;
        }
    }

    format->sampleRate = source->mp3.sampleRate;
    format->channelCount = source->channels;
    format->sampleType = GUCEF_AUDIOCODEC_SAMPLETYPE_FLOAT32;
    format->channelLayout = 0;
    *lengthInFrames = source->length;
    return source;
}

/*-------------------------------------------------------------------------*/

static UInt32
AddTags( const TMp3Source* source, TVariantMapApi* metaData )
{
    UInt32 added = 0;
    if ( NULL != source->id3v2 )
        added += Mp3AddId3v2Tags( metaData, source->id3v2, source->id3v2Size );
    if ( source->hasId3v1 )
        added += Mp3AddId3v1Tags( metaData, source->id3v1 );
    return added;
}

/*-------------------------------------------------------------------------*/

/** The bit rate, sample rate and frame size of an MPEG audio frame header, 0 when it is not a layer III one */
static UInt32
FrameSize( const UInt8* h )
{
    static const UInt16 bitrates[ 2 ][ 15 ] = {
        { 0, 8, 16, 24, 32, 40, 48, 56, 64, 80, 96, 112, 128, 144, 160 },                       /* MPEG 2 and 2.5 */
        { 0, 32, 40, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320 } };                 /* MPEG 1 */
    static const UInt16 rates[ 3 ] = { 44100, 48000, 32000 };
    UInt32 version = ( h[ 1 ] >> 3 ) & 3;
    UInt32 layer = ( h[ 1 ] >> 1 ) & 3;
    UInt32 bitrateIndex = h[ 2 ] >> 4;
    UInt32 rateIndex = ( h[ 2 ] >> 2 ) & 3;
    UInt32 bitrate, rate, padding;

    if ( 0xFF != h[ 0 ] || 0xE0 != ( h[ 1 ] & 0xE0 ) || 1 == version || 1 != layer || 0 == bitrateIndex || 15 == bitrateIndex || 3 == rateIndex )
        return 0;

    bitrate = (UInt32) bitrates[ 3 == version ? 1 : 0 ][ bitrateIndex ] * 1000UL;
    rate = rates[ rateIndex ];
    if ( 2 == version )
        rate /= 2;
    else if ( 0 == version )
        rate /= 4;
    padding = ( h[ 2 ] >> 1 ) & 1;
    return ( ( 3 == version ? 144UL : 72UL ) * bitrate ) / rate + padding;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      PLUGIN API                                                         //
//                                                                         //
//-------------------------------------------------------------------------*/

UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
MODULE_API_FUNCTION( AUDIOCODECPLUG_Init )( void** plugdata, TGucefCoreCApi* libApi ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    TMp3Plugin* plugin = (TMp3Plugin*) calloc( 1, sizeof( TMp3Plugin ) );
    if ( NULL == plugin )
        return 0;
    if ( NULL != libApi )
        plugin->libApi = *libApi;
    *plugdata = plugin;

    GUCEF_C_LOG( plugin->libApi, GUCEF_LOGLEVEL_BELOW_NORMAL, "audiocodecpluginMP3: initialized" );
    return 1;
}

/*-------------------------------------------------------------------------*/

void GUCEF_PLUGIN_CALLSPEC_PREFIX
MODULE_API_FUNCTION( AUDIOCODECPLUG_Shutdown )( void** plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    free( *plugdata );
    *plugdata = NULL;
}

/*-------------------------------------------------------------------------*/

const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
MODULE_API_FUNCTION( AUDIOCODECPLUG_Name )( const void* plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    return "gucefAUDIO codec plugin for MP3 audio";
}

/*-------------------------------------------------------------------------*/

const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
MODULE_API_FUNCTION( AUDIOCODECPLUG_Copyright )( const void* plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    return "Copyright (C) Dinand Vanvelzen. Licensed under the Apache License, Version 2.0. Includes dr_mp3 by David Reid (public domain or MIT-0), based on minimp3 by lieff (CC0)";
}

/*-------------------------------------------------------------------------*/

const TVersion* GUCEF_PLUGIN_CALLSPEC_PREFIX
MODULE_API_FUNCTION( AUDIOCODECPLUG_Version )( const void* plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
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
MODULE_API_FUNCTION( AUDIOCODECPLUG_Type_Count )( const void* plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    return (UInt32) ( sizeof( g_codecTypes ) / sizeof( g_codecTypes[ 0 ] ) );
}

/*-------------------------------------------------------------------------*/

const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
MODULE_API_FUNCTION( AUDIOCODECPLUG_Type )( const void* plugdata, UInt32 index ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    if ( index >= sizeof( g_codecTypes ) / sizeof( g_codecTypes[ 0 ] ) )
        return NULL;
    return g_codecTypes[ index ];
}

/*-------------------------------------------------------------------------*/

UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
MODULE_API_FUNCTION( AUDIOCODECPLUG_Create_Codec )( void** plugdata, const char* codecType, void** codecdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    /* There is one type and no per codec state */
    *codecdata = NULL;
    return NULL != codecType && 0 == strcmp( codecType, MP3_CODEC_TYPE ) ? 1 : 0;
}

/*-------------------------------------------------------------------------*/

void GUCEF_PLUGIN_CALLSPEC_PREFIX
MODULE_API_FUNCTION( AUDIOCODECPLUG_Destroy_Codec )( void** plugdata, void** codecdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    *codecdata = NULL;
}

/*-------------------------------------------------------------------------*/

UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
MODULE_API_FUNCTION( AUDIOCODECPLUG_Src_Open )( void** plugdata, void** codecdata, void** sourcedata, TIOAccess* input, const TVariantMapApi* options, TAudioFormatInfo* format, UInt64* lengthInFrames ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    TMp3Source* source = OpenSource( (const TMp3Plugin*) *plugdata, input, options, format, lengthInFrames );
    if ( NULL == source )
        return 0;
    *sourcedata = source;
    return 1;
}

/*-------------------------------------------------------------------------*/

UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
MODULE_API_FUNCTION( AUDIOCODECPLUG_Src_Read_Frames )( void** plugdata, void** codecdata, void** sourcedata, void* destination, UInt32 frameCapacity, UInt32* framesRead ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    TMp3Source* source = (TMp3Source*) *sourcedata;
    float* out = (float*) destination;
    UInt32 done = 0;

    *framesRead = 0;
    if ( NULL == source )
        return 0;
    if ( source->isAtEnd )
        return 1;

    if ( GUCEF_AUDIOCODEC_LENGTH_UNKNOWN != source->length )
    {
        UInt64 left = source->length > source->position ? source->length - source->position : 0;
        if ( left < frameCapacity )
            frameCapacity = (UInt32) left;
    }

    while ( done < frameCapacity )
    {
        drmp3_uint64 got = drmp3_read_pcm_frames_f32( &source->mp3, frameCapacity - done, out + (size_t) done * source->channels );
        if ( 0 == got )
            break;
        done += (UInt32) got;
    }

    source->position += done;
    *framesRead = done;
    return 1;
}

/*-------------------------------------------------------------------------*/

UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
MODULE_API_FUNCTION( AUDIOCODECPLUG_Src_Seek )( void** plugdata, void** codecdata, void** sourcedata, UInt64 frame ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    TMp3Source* source = (TMp3Source*) *sourcedata;
    if ( NULL == source )
        return 0;

    if ( GUCEF_AUDIOCODEC_LENGTH_UNKNOWN != source->length )
    {
        if ( frame > source->length )
            return 0;
        if ( frame == source->length )
        {
            /* the end is a position too, there is just nothing to decode from there */
            source->isAtEnd = 1;
            source->position = frame;
            return 1;
        }
    }

    if ( !drmp3_seek_to_pcm_frame( &source->mp3, frame ) )
        return 0;
    source->isAtEnd = 0;
    source->position = frame;
    return 1;
}

/*-------------------------------------------------------------------------*/

void GUCEF_PLUGIN_CALLSPEC_PREFIX
MODULE_API_FUNCTION( AUDIOCODECPLUG_Src_Close )( void** plugdata, void** codecdata, void** sourcedata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    CloseSource( (TMp3Source*) *sourcedata );
    *sourcedata = NULL;
}

/*-------------------------------------------------------------------------*/

UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
MODULE_API_FUNCTION( AUDIOCODECPLUG_Src_Meta_Data )( void** plugdata, void** codecdata, void** sourcedata, TVariantMapApi* metaData ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    const TMp3Source* source = (const TMp3Source*) *sourcedata;
    if ( NULL == source || ( NULL == source->id3v2 && !source->hasId3v1 ) )
        return 0;
    return 0 != AddTags( source, metaData ) ? 1 : 0;
}

/*-------------------------------------------------------------------------*/

UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
MODULE_API_FUNCTION( AUDIOCODECPLUG_Decode_Meta_Data )( void** plugdata, void** codecdata, TIOAccess* input, TVariantMapApi* metaData ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    TAudioFormatInfo format;
    UInt64 length;
    TMp3Source* source = OpenSource( (const TMp3Plugin*) *plugdata, input, NULL, &format, &length );
    if ( NULL == source )
        return 0;
    AddTags( source, metaData );
    CloseSource( source );
    return 1;
}

/*-------------------------------------------------------------------------*/

UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
MODULE_API_FUNCTION( AUDIOCODECPLUG_Threading_Model )( const void* plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    /* dr_mp3 keeps everything of a stream in the stream, and the plugin has no state that changes after Init */
    return GUCEF_AUDIOCODEC_THREADING_PER_SOURCE;
}

/*-------------------------------------------------------------------------*/

UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
MODULE_API_FUNCTION( AUDIOCODECPLUG_Probe )( void** plugdata, void** codecdata, TIOAccess* input, UInt32* confidence ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    UInt8 buffer[ 4096 + 4 ];
    UInt64 start;
    UInt32 got, i, skip = 0;
    int hasTag = 0;

    *confidence = 0;
    if ( NULL == input || NULL == input->read || NULL == input->tell || NULL == input->setpos )
        return 0;

    start = input->tell( input );
    got = input->read( input, buffer, 1, 10 );
    if ( got < 4 )
        return 1;

    if ( 10 == got && 0 == memcmp( buffer, "ID3", 3 ) )
    {
        skip = 10U + ( ( (UInt32) ( buffer[ 6 ] & 0x7F ) << 21 ) | ( (UInt32) ( buffer[ 7 ] & 0x7F ) << 14 ) | ( (UInt32) ( buffer[ 8 ] & 0x7F ) << 7 ) | (UInt32) ( buffer[ 9 ] & 0x7F ) );
        hasTag = 1;
    }
    if ( 0 != input->setpos( input, start + skip ) )
        return 1;

    /* the first frame header is expected where the tag ends, or soon after in a file with junk in front */
    got = input->read( input, buffer, 1, sizeof( buffer ) );
    for ( i=0; i+4<=got && i<( hasTag ? 4U : 2048U ); ++i )
    {
        UInt32 size = FrameSize( buffer + i );
        if ( 0 == size )
            continue;
        if ( i + size + 4 <= got && 0 != FrameSize( buffer + i + size ) )
            *confidence = 0 == i ? 95 : 85;                 /* two frames in a row */
        else if ( i + size + 4 > got )
            *confidence = 0 == i ? 70 : 50;                 /* a lone frame, the file ends or the next is out of reach */
        else
            continue;
        return 1;
    }
    if ( hasTag )
        *confidence = 20;                                   /* a tag and nothing that looks like audio after it */
    return 1;
}

/*-------------------------------------------------------------------------*/
