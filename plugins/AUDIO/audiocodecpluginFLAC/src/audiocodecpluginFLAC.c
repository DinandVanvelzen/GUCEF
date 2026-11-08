/*
 *  audiocodecpluginFLAC: gucefAUDIO codec plugin for FLAC audio
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
 *  FLAC through gucefAUDIO codec plugin C interface. Decoding is done by dr_flac (dependencies/dr_libs), which
 *  reads and seeks through the TIOAccess of the stream. Writing is done by a small encoder of our own, see
 *  audiocodecpluginFLAC_encoder.c.
 *
 *  Frames come out as 16 bit samples for streams of up to 16 bits per sample and as 32 bit samples, left
 *  justified, for deeper ones, so lossless content stays exact. FLAC has the channel order of a WAVE file, the
 *  channel mask is read from the WAVEFORMATEXTENSIBLE_CHANNEL_MASK tag when the file has it.
 *
 *  A stream uses nothing but its own state and its own input, so streams decode at the same time
 *  (AUDIOCODECPLUG_Threading_Model). Tags are the VORBIS_COMMENT of the file with lower case names.
 *
 *  This plugin decodes native FLAC only. FLAC in an Ogg container (.oga) is left to the Ogg plugin, which
 *  recognises it and says it is not supported.
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

#include "audiocodecpluginFLAC.h"
#include "audiocodecpluginFLAC_internal.h"

/* The decoder is part of this plugin: this is the one source file of the module that holds the implementation of dr_flac */
#define DR_FLAC_IMPLEMENTATION
#define DR_FLAC_NO_STDIO
#define DR_FLAC_NO_OGG
#include "dr_flac.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#define FLAC_CODEC_TYPE             "flac"
#define FLAC_MAX_COMMENT_BYTES      ( 4U * 1024U * 1024U )      /* a larger VORBIS_COMMENT block is ignored */
#define FLAC_MAX_TAG_VALUE_BYTES    ( 64U * 1024U )             /* a longer tag value is cut off */
#define FLAC_MAX_TAG_NAME_BYTES     256U

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

typedef struct SFlacPlugin
{
    TGucefCoreCApi libApi;
} TFlacPlugin;

/** The input of one stream, as dr_flac sees it: positions count from where the FLAC data starts */
typedef struct SFlacInput
{
    TIOAccess* input;
    UInt64 start;
    UInt64 size;                /* bytes from 'start' to the end, 0 when the input does not tell */
} TFlacInput;

typedef struct SFlacSource
{
    TFlacInput io;
    drflac* flac;
    UInt32 channels;
    UInt32 sampleType;
    UInt32 bytesPerSample;
    UInt32 channelLayout;
    UInt64 length;              /* GUCEF_AUDIOCODEC_LENGTH_UNKNOWN when the file does not tell */
    UInt64 position;
    int isAtEnd;                /* sought to the very end, nothing to read until the next seek */
    UInt8* comments;            /* the comments of the VORBIS_COMMENT block, without vendor string and count */
    UInt32 commentsSize;
    UInt32 commentCount;
} TFlacSource;

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

static const char* g_codecTypes[] = { FLAC_CODEC_TYPE };

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static UInt32
ReadLE32( const UInt8* bytes )
{
    return (UInt32) bytes[ 0 ] | ( (UInt32) bytes[ 1 ] << 8 ) | ( (UInt32) bytes[ 2 ] << 16 ) | ( (UInt32) bytes[ 3 ] << 24 );
}

/*-------------------------------------------------------------------------*/

static size_t
InputRead( void* user, void* out, size_t bytesToRead )
{
    TFlacInput* io = (TFlacInput*) user;
    size_t done = 0;

    while ( done < bytesToRead )
    {
        size_t left = bytesToRead - done;
        UInt32 chunk = left > 0x40000000UL ? 0x40000000UL : (UInt32) left;
        UInt32 got = io->input->read( io->input, ( (UInt8*) out ) + done, 1, chunk );
        done += got;
        if ( got < chunk )
            break;
    }
    return done;
}

/*-------------------------------------------------------------------------*/

static drflac_bool32
InputSeek( void* user, int offset, drflac_seek_origin origin )
{
    TFlacInput* io = (TFlacInput*) user;
    Int64 target;

    switch ( origin )
    {
        case DRFLAC_SEEK_SET: target = offset; break;
        case DRFLAC_SEEK_CUR: target = (Int64) ( io->input->tell( io->input ) - io->start ) + offset; break;
        case DRFLAC_SEEK_END:
        {
            if ( 0 == io->size )
                return DRFLAC_FALSE;
            target = (Int64) io->size + offset;
            break;
        }
        default: return DRFLAC_FALSE;
    }

    if ( target < 0 || ( 0 != io->size && (UInt64) target > io->size ) )
        return DRFLAC_FALSE;
    return 0 == io->input->setpos( io->input, io->start + (UInt64) target ) ? DRFLAC_TRUE : DRFLAC_FALSE;
}

/*-------------------------------------------------------------------------*/

static drflac_bool32
InputTell( void* user, drflac_int64* cursor )
{
    TFlacInput* io = (TFlacInput*) user;
    UInt64 position = io->input->tell( io->input );
    if ( position < io->start )
        return DRFLAC_FALSE;
    *cursor = (drflac_int64) ( position - io->start );
    return DRFLAC_TRUE;
}

/*-------------------------------------------------------------------------*/

/** Puts text under a name in a map of the host, which copies both */
static void
SetTextValue( TVariantMapApi* map, const char* key, const char* value, UInt32 valueSize )
{
    TVariantData keyData;
    TVariantData valueData;

    if ( NULL == map || NULL == map->insert )
        return;

    memset( &keyData, 0, sizeof( keyData ) );
    keyData.containedType = GUCEF_DATATYPE_ASCII_STRING;
    keyData.union_data.heap_data.union_data.const_char_heap_data = key;
    keyData.union_data.heap_data.heap_data_size = (UInt32) strlen( key );
    keyData.union_data.heap_data.heap_data_is_linked = 1;

    memset( &valueData, 0, sizeof( valueData ) );
    valueData.containedType = GUCEF_DATATYPE_UTF8_STRING;
    valueData.union_data.heap_data.union_data.const_char_heap_data = value;
    valueData.union_data.heap_data.heap_data_size = valueSize;
    valueData.union_data.heap_data.heap_data_is_linked = 1;

    map->insert( map->privateData, &keyData, &valueData, 0 );
}

/*-------------------------------------------------------------------------*/

/**
 *  Puts the comments of a VORBIS_COMMENT block into a map: 'count' times a 32 bit little endian length and then
 *  "NAME=value", all within 'size' bytes. Names are lower cased. What does not fit the block is not read.
 */
static void
AddComments( TVariantMapApi* map, const UInt8* data, UInt32 size, UInt32 count )
{
    char name[ FLAC_MAX_TAG_NAME_BYTES + 1 ];
    char* value;
    UInt32 offset = 0;
    UInt32 i;

    value = (char*) malloc( FLAC_MAX_TAG_VALUE_BYTES + 1 );
    if ( NULL == value )
        return;

    for ( i=0; i<count; ++i )
    {
        UInt32 length, nameLength, valueLength, n;

        if ( size - offset < 4 )
            break;
        length = ReadLE32( data + offset );
        offset += 4;
        if ( length > size - offset )
            break;

        /* the name is up to the first '=' and made of printable ASCII */
        nameLength = 0;
        while ( nameLength < length && '=' != (char) data[ offset + nameLength ] )
            ++nameLength;
        if ( nameLength == length || 0 == nameLength || nameLength > FLAC_MAX_TAG_NAME_BYTES )
        {
            offset += length;
            continue;
        }
        for ( n=0; n<nameLength; ++n )
        {
            char c = (char) data[ offset + n ];
            if ( c < 0x20 || c > 0x7D )
                break;
            name[ n ] = ( c >= 'A' && c <= 'Z' ) ? (char) ( c + ( 'a' - 'A' ) ) : c;
        }
        if ( n == nameLength )
        {
            name[ nameLength ] = '\0';
            valueLength = length - nameLength - 1;
            if ( valueLength > FLAC_MAX_TAG_VALUE_BYTES )
                valueLength = FLAC_MAX_TAG_VALUE_BYTES;
            memcpy( value, data + offset + nameLength + 1, valueLength );
            value[ valueLength ] = '\0';
            /* a value is text: it ends at a NUL if it holds one */
            SetTextValue( map, name, value, (UInt32) strlen( value ) );
        }
        offset += length;
    }
    free( value );
}

/*-------------------------------------------------------------------------*/

/** The value of a hexadecimal tag, 0 when the file has no such tag. Used for the channel mask */
static UInt32
FindHexComment( const UInt8* data, UInt32 size, UInt32 count, const char* name )
{
    UInt32 nameLength = (UInt32) strlen( name );
    UInt32 offset = 0;
    UInt32 i;

    for ( i=0; i<count; ++i )
    {
        UInt32 length, n;
        int isMatch;

        if ( size - offset < 4 )
            break;
        length = ReadLE32( data + offset );
        offset += 4;
        if ( length > size - offset )
            break;

        isMatch = length > nameLength && '=' == (char) data[ offset + nameLength ];
        for ( n=0; isMatch && n<nameLength; ++n )
        {
            char c = (char) data[ offset + n ];
            if ( c >= 'a' && c <= 'z' )
                c = (char) ( c - ( 'a' - 'A' ) );
            isMatch = c == name[ n ];
        }
        if ( isMatch )
        {
            UInt32 value = 0;
            UInt32 digits = 0;
            n = nameLength + 1;
            if ( length - n > 2 && '0' == (char) data[ offset + n ] && ( 'x' == (char) data[ offset + n + 1 ] || 'X' == (char) data[ offset + n + 1 ] ) )
                n += 2;
            for ( ; n<length && digits < 8; ++n, ++digits )
            {
                char c = (char) data[ offset + n ];
                if ( c >= '0' && c <= '9' )
                    value = ( value << 4 ) | (UInt32) ( c - '0' );
                else if ( c >= 'a' && c <= 'f' )
                    value = ( value << 4 ) | (UInt32) ( c - 'a' + 10 );
                else if ( c >= 'A' && c <= 'F' )
                    value = ( value << 4 ) | (UInt32) ( c - 'A' + 10 );
                else
                    break;
            }
            return value;
        }
        offset += length;
    }
    return 0;
}

/*-------------------------------------------------------------------------*/

static UInt32
CountBits( UInt32 value )
{
    UInt32 count = 0;
    while ( 0 != value )
    {
        count += value & 1U;
        value >>= 1;
    }
    return count;
}

/*-------------------------------------------------------------------------*/

/** Called by dr_flac for every metadata block while opening, the comments are kept for AUDIOCODECPLUG_Src_Meta_Data */
static void
OnMetaBlock( void* user, drflac_metadata* meta )
{
    TFlacSource* source = (TFlacSource*) user;
    UInt32 headSize;
    UInt32 size;

    if ( DRFLAC_METADATA_BLOCK_TYPE_VORBIS_COMMENT != meta->type || NULL != source->comments )
        return;

    /* the block is the vendor string, the number of comments and the comments */
    headSize = 8U + meta->data.vorbis_comment.vendorLength;
    if ( NULL == meta->data.vorbis_comment.pComments || meta->rawDataSize <= headSize )
        return;
    size = meta->rawDataSize - headSize;
    if ( size > FLAC_MAX_COMMENT_BYTES )
        return;

    source->comments = (UInt8*) malloc( size );
    if ( NULL == source->comments )
        return;
    memcpy( source->comments, meta->data.vorbis_comment.pComments, size );
    source->commentsSize = size;
    source->commentCount = meta->data.vorbis_comment.commentCount;
}

/*-------------------------------------------------------------------------*/

static void
CloseSource( TFlacSource* source )
{
    if ( NULL == source )
        return;
    if ( NULL != source->flac )
        drflac_close( source->flac );
    free( source->comments );
    free( source );
}

/*-------------------------------------------------------------------------*/

/** Opens a stream on the input at its current position. NULL when it is not FLAC that can be decoded */
static TFlacSource*
OpenSource( const TFlacPlugin* plugin, TIOAccess* input, TAudioFormatInfo* format, UInt64* lengthInFrames )
{
    TFlacSource* source;
    UInt64 end;

    if ( NULL == input || NULL == input->read || NULL == input->tell || NULL == input->setpos )
        return NULL;

    source = (TFlacSource*) calloc( 1, sizeof( TFlacSource ) );
    if ( NULL == source )
        return NULL;

    source->io.input = input;
    source->io.start = input->tell( input );
    end = NULL != input->size ? input->size( input ) : 0;
    source->io.size = end > source->io.start ? end - source->io.start : 0;

    source->flac = drflac_open_with_metadata( InputRead, InputSeek, InputTell, OnMetaBlock, &source->io, NULL );
    if ( NULL == source->flac )
    {
        CloseSource( source );
        return NULL;
    }

    if ( source->flac->channels < 1 || source->flac->channels > 8 || 0 == source->flac->sampleRate || source->flac->bitsPerSample < 4 )
    {
        GUCEF_C_ERROR_LOG( plugin->libApi, GUCEF_LOGLEVEL_NORMAL, "audiocodecpluginFLAC: the stream has a channel count, sample rate or sample size that is not supported" );
        CloseSource( source );
        return NULL;
    }

    source->channels = source->flac->channels;
    if ( source->flac->bitsPerSample <= 16 )
    {
        source->sampleType = GUCEF_AUDIOCODEC_SAMPLETYPE_INT16;
        source->bytesPerSample = 2;
    }
    else
    {
        source->sampleType = GUCEF_AUDIOCODEC_SAMPLETYPE_INT32;
        source->bytesPerSample = 4;
    }
    source->length = 0 == source->flac->totalPCMFrameCount ? GUCEF_AUDIOCODEC_LENGTH_UNKNOWN : source->flac->totalPCMFrameCount;

    /* a channel mask only counts when it names as many speakers as there are channels */
    if ( NULL != source->comments )
    {
        UInt32 mask = FindHexComment( source->comments, source->commentsSize, source->commentCount, "WAVEFORMATEXTENSIBLE_CHANNEL_MASK" );
        if ( CountBits( mask ) == source->channels )
            source->channelLayout = mask;
    }

    format->sampleRate = source->flac->sampleRate;
    format->channelCount = source->channels;
    format->sampleType = source->sampleType;
    format->channelLayout = source->channelLayout;
    *lengthInFrames = source->length;
    return source;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      PLUGIN API                                                         //
//                                                                         //
//-------------------------------------------------------------------------*/

UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
MODULE_API_FUNCTION( AUDIOCODECPLUG_Init )( void** plugdata, TGucefCoreCApi* libApi ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    TFlacPlugin* plugin = (TFlacPlugin*) calloc( 1, sizeof( TFlacPlugin ) );
    if ( NULL == plugin )
        return 0;
    if ( NULL != libApi )
        plugin->libApi = *libApi;
    *plugdata = plugin;

    GUCEF_C_LOG( plugin->libApi, GUCEF_LOGLEVEL_BELOW_NORMAL, "audiocodecpluginFLAC: initialized" );
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
    return "gucefAUDIO codec plugin for FLAC audio";
}

/*-------------------------------------------------------------------------*/

const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
MODULE_API_FUNCTION( AUDIOCODECPLUG_Copyright )( const void* plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    return "Copyright (C) Dinand Vanvelzen. Licensed under the Apache License, Version 2.0. Includes dr_flac by David Reid (public domain or MIT-0)";
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
    return NULL != codecType && 0 == strcmp( codecType, FLAC_CODEC_TYPE ) ? 1 : 0;
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
    TFlacSource* source = OpenSource( (const TFlacPlugin*) *plugdata, input, format, lengthInFrames );
    if ( NULL == source )
        return 0;
    *sourcedata = source;
    return 1;
}

/*-------------------------------------------------------------------------*/

UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
MODULE_API_FUNCTION( AUDIOCODECPLUG_Src_Read_Frames )( void** plugdata, void** codecdata, void** sourcedata, void* destination, UInt32 frameCapacity, UInt32* framesRead ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    TFlacSource* source = (TFlacSource*) *sourcedata;
    UInt8* out = (UInt8*) destination;
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

    /* dr_flac gives fewer frames than asked only at the end of the stream, or when it can not go on */
    while ( done < frameCapacity )
    {
        UInt64 got;
        if ( GUCEF_AUDIOCODEC_SAMPLETYPE_INT16 == source->sampleType )
            got = drflac_read_pcm_frames_s16( source->flac, frameCapacity - done, (drflac_int16*) ( out + (size_t) done * source->channels * 2 ) );
        else
            got = drflac_read_pcm_frames_s32( source->flac, frameCapacity - done, (drflac_int32*) ( out + (size_t) done * source->channels * 4 ) );
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
    TFlacSource* source = (TFlacSource*) *sourcedata;
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

    if ( !drflac_seek_to_pcm_frame( source->flac, frame ) )
        return 0;
    source->isAtEnd = 0;
    source->position = frame;
    return 1;
}

/*-------------------------------------------------------------------------*/

void GUCEF_PLUGIN_CALLSPEC_PREFIX
MODULE_API_FUNCTION( AUDIOCODECPLUG_Src_Close )( void** plugdata, void** codecdata, void** sourcedata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    CloseSource( (TFlacSource*) *sourcedata );
    *sourcedata = NULL;
}

/*-------------------------------------------------------------------------*/

UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
MODULE_API_FUNCTION( AUDIOCODECPLUG_Src_Meta_Data )( void** plugdata, void** codecdata, void** sourcedata, TVariantMapApi* metaData ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    const TFlacSource* source = (const TFlacSource*) *sourcedata;
    if ( NULL == source || NULL == source->comments )
        return 0;
    AddComments( metaData, source->comments, source->commentsSize, source->commentCount );
    return 1;
}

/*-------------------------------------------------------------------------*/

UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
MODULE_API_FUNCTION( AUDIOCODECPLUG_Decode_Meta_Data )( void** plugdata, void** codecdata, TIOAccess* input, TVariantMapApi* metaData ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    TAudioFormatInfo format;
    UInt64 length;
    TFlacSource* source = OpenSource( (const TFlacPlugin*) *plugdata, input, &format, &length );
    if ( NULL == source )
        return 0;
    if ( NULL != source->comments )
        AddComments( metaData, source->comments, source->commentsSize, source->commentCount );
    CloseSource( source );
    return 1;
}

/*-------------------------------------------------------------------------*/

UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
MODULE_API_FUNCTION( AUDIOCODECPLUG_Encode )( void** plugdata, void** codecdata, const TAudioFormatInfo* format, const void* frames, UInt64 frameCount, TIOAccess* output, const TVariantMapApi* options ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    const TFlacPlugin* plugin = (const TFlacPlugin*) *plugdata;
    return FlacEncode( &plugin->libApi, format, frames, frameCount, output, options );
}

/*-------------------------------------------------------------------------*/

UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
MODULE_API_FUNCTION( AUDIOCODECPLUG_Threading_Model )( const void* plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    /* dr_flac keeps everything of a stream in the stream, and the plugin has no state that changes after Init */
    return GUCEF_AUDIOCODEC_THREADING_PER_SOURCE;
}

/*-------------------------------------------------------------------------*/

UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
MODULE_API_FUNCTION( AUDIOCODECPLUG_Probe )( void** plugdata, void** codecdata, TIOAccess* input, UInt32* confidence ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    UInt8 head[ 10 ];
    UInt64 start;

    *confidence = 0;
    if ( NULL == input || NULL == input->read || NULL == input->tell || NULL == input->setpos )
        return 0;

    start = input->tell( input );
    if ( 4 != input->read( input, head, 1, 4 ) )
        return 1;
    if ( 0 == memcmp( head, "fLaC", 4 ) )
    {
        *confidence = 100;
        return 1;
    }

    /* a file may start with an ID3v2 tag before the FLAC data, dr_flac skips it */
    if ( 0 == memcmp( head, "ID3", 3 ) )
    {
        UInt32 tagSize;
        if ( 6 != input->read( input, head + 4, 1, 6 ) )
            return 1;
        tagSize = ( (UInt32) ( head[ 6 ] & 0x7F ) << 21 ) | ( (UInt32) ( head[ 7 ] & 0x7F ) << 14 ) | ( (UInt32) ( head[ 8 ] & 0x7F ) << 7 ) | (UInt32) ( head[ 9 ] & 0x7F );
        if ( 0 == input->setpos( input, start + 10 + tagSize ) && 4 == input->read( input, head, 1, 4 ) && 0 == memcmp( head, "fLaC", 4 ) )
            *confidence = 90;
    }
    return 1;
}

/*-------------------------------------------------------------------------*/
