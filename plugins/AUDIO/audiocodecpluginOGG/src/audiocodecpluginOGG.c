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
 *  Ogg Vorbis and Ogg Opus through the gucefAUDIO codec plugin C interface. The codec types "ogg", "oga" and
 *  "opus" are served by the same code: an Ogg file can hold either codec, so the first packet of the file decides
 *  and not the name. Ogg FLAC is recognised and reported as not supported.
 *
 *  Decoding is done by vorbisfile and opusfile, which read and seek through the TIOAccess of the stream, and
 *  gives 32 bit float frames in the channel order of a WAVE file (Vorbis and Opus order their surround channels
 *  differently). Seeking is sample exact for both, so loops are gapless: the audio at a seek target is exactly the
 *  audio there, and for Vorbis the same samples as reading on. Opus is lossy in a way that depends on what it decoded
 *  before: after a seek (not to the start of the stream, where it is exact) the decoder needs about half a second to
 *  have the state it would have had reading on, and the first part of that is a little off. Tags are the Vorbis
 *  comments of the first link, with lower case names, which includes the loop points (LOOPSTART and so on) that
 *  game music has.
 *
 *  Encoding is optional and meant for tests and export: Vorbis through libvorbisenc and Opus through libopus.
 *  The options are text: "quality" (Vorbis, -0.1 to 1), "bitrate" (bits per second, Vorbis and Opus), "serial"
 *  (the number of the Ogg stream), "tags" (lines of NAME=value separated by line feeds) and "codec" ("vorbis" or
 *  "opus", for the types "ogg" and "oga" which write Vorbis by default; "opus" writes Opus). Opus takes 48000 Hz audio.
 *
 *  The Opus part is only built when the Opus libraries are, GUCEF_AUDIOCODECPLUGIN_OGG_HAVE_OPUS.
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

#include "audiocodecpluginOGG.h"
#include "audiocodecpluginOGG_internal.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#define OGG_KIND_UNKNOWN    0
#define OGG_KIND_VORBIS     1
#define OGG_KIND_OPUS       2
#define OGG_KIND_FLAC       3
#define OGG_KIND_OTHER      4       /* Ogg, with something in it that is not audio of ours (video, speech, ...) */

#define OGG_CODEC_TYPE_OGG  "ogg"
#define OGG_CODEC_TYPE_OGA  "oga"
#define OGG_CODEC_TYPE_OPUS "opus"

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

typedef struct SOggPlugin
{
    TGucefCoreCApi libApi;
} TOggPlugin;

/** What a codec type is for: the encoder it uses by default */
typedef struct SOggCodecInfo
{
    const char* type;
    int encodesOpus;
} TOggCodecInfo;

typedef struct SOggSource
{
    TOggInput io;
    TOggDecoder* decoder;
    UInt32 channels;
    UInt64 length;
    UInt64 position;
    int isAtEnd;                /* sought to the very end, nothing to read until the next seek */
} TOggSource;

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

static const TOggCodecInfo g_codecs[] = {
    { OGG_CODEC_TYPE_OGG, 0 },
    { OGG_CODEC_TYPE_OGA, 0 },
    { OGG_CODEC_TYPE_OPUS, 1 } };

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** Tells what the first packet of the Ogg stream at the current position of the input is. Moves the input */
static int
DetectKind( TIOAccess* input )
{
    UInt8 head[ 27 + 255 + 8 ];
    UInt32 got, segments, offset;

    got = input->read( input, head, 1, sizeof( head ) );
    if ( got < 27 + 1 + 8 || 0 != memcmp( head, "OggS", 4 ) || 0 != head[ 4 ] )
        return OGG_KIND_UNKNOWN;
    if ( !( head[ 5 ] & 0x02 ) )
        return OGG_KIND_UNKNOWN;        /* not the first page of a stream */

    segments = head[ 26 ];
    offset = 27 + segments;
    if ( offset + 8 > got )
        return OGG_KIND_UNKNOWN;
    if ( 0 == memcmp( head + offset, "\x01vorbis", 7 ) )
        return OGG_KIND_VORBIS;
    if ( 0 == memcmp( head + offset, "OpusHead", 8 ) )
        return OGG_KIND_OPUS;
    if ( 0 == memcmp( head + offset, "\x7f" "FLAC", 5 ) )
        return OGG_KIND_FLAC;
    return OGG_KIND_OTHER;
}

/*-------------------------------------------------------------------------*/

static void
CloseSource( TOggSource* source )
{
    if ( NULL == source )
        return;
    if ( NULL != source->decoder )
        source->decoder->close( source->decoder );
    free( source );
}

/*-------------------------------------------------------------------------*/

/** Opens a stream on the input at its current position. NULL when it is not Ogg audio that can be decoded */
static TOggSource*
OpenSource( const TOggPlugin* plugin, TIOAccess* input, TAudioFormatInfo* format, UInt64* lengthInFrames )
{
    TOggSource* source;
    UInt64 end;
    int kind;

    if ( NULL == input || NULL == input->read || NULL == input->tell || NULL == input->setpos )
        return NULL;

    source = (TOggSource*) calloc( 1, sizeof( TOggSource ) );
    if ( NULL == source )
        return NULL;
    source->io.input = input;
    source->io.start = input->tell( input );
    end = NULL != input->size ? input->size( input ) : 0;
    source->io.size = end > source->io.start ? end - source->io.start : 0;

    kind = DetectKind( input );
    if ( 0 != input->setpos( input, source->io.start ) )
    {
        free( source );
        return NULL;
    }

    switch ( kind )
    {
        case OGG_KIND_VORBIS:
        {
            source->decoder = OggVorbisOpen( &plugin->libApi, &source->io );
            break;
        }
        case OGG_KIND_OPUS:
        {
            #ifdef GUCEF_AUDIOCODECPLUGIN_OGG_HAVE_OPUS
            source->decoder = OggOpusOpen( &plugin->libApi, &source->io );
            #else
            GUCEF_C_ERROR_LOG( plugin->libApi, GUCEF_LOGLEVEL_NORMAL, "audiocodecpluginOGG: this stream is Ogg Opus, which this build of the plugin does not have" );
            #endif
            break;
        }
        case OGG_KIND_FLAC:
        {
            GUCEF_C_ERROR_LOG( plugin->libApi, GUCEF_LOGLEVEL_NORMAL, "audiocodecpluginOGG: this stream is Ogg FLAC, which is not supported" );
            break;
        }
        default: break;
    }

    if ( NULL == source->decoder )
    {
        free( source );
        return NULL;
    }

    source->channels = source->decoder->channels;
    source->length = source->decoder->length;
    format->sampleRate = source->decoder->sampleRate;
    format->channelCount = source->channels;
    format->sampleType = GUCEF_AUDIOCODEC_SAMPLETYPE_FLOAT32;
    format->channelLayout = source->decoder->channelLayout;
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
    TOggPlugin* plugin = (TOggPlugin*) calloc( 1, sizeof( TOggPlugin ) );
    if ( NULL == plugin )
        return 0;
    if ( NULL != libApi )
        plugin->libApi = *libApi;
    *plugdata = plugin;

    GUCEF_C_LOG( plugin->libApi, GUCEF_LOGLEVEL_BELOW_NORMAL, "audiocodecpluginOGG: initialized" );
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
    return "gucefAUDIO codec plugin for Ogg Vorbis and Ogg Opus audio";
}

/*-------------------------------------------------------------------------*/

const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
MODULE_API_FUNCTION( AUDIOCODECPLUG_Copyright )( const void* plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    return "Copyright (C) Dinand Vanvelzen. Licensed under the Apache License, Version 2.0. Includes libogg, libvorbis, libopus and opusfile by Xiph.Org Foundation (BSD-3-Clause)";
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
    return (UInt32) ( sizeof( g_codecs ) / sizeof( g_codecs[ 0 ] ) );
}

/*-------------------------------------------------------------------------*/

const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
MODULE_API_FUNCTION( AUDIOCODECPLUG_Type )( const void* plugdata, UInt32 index ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    if ( index >= sizeof( g_codecs ) / sizeof( g_codecs[ 0 ] ) )
        return NULL;
    return g_codecs[ index ].type;
}

/*-------------------------------------------------------------------------*/

UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
MODULE_API_FUNCTION( AUDIOCODECPLUG_Create_Codec )( void** plugdata, const char* codecType, void** codecdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    UInt32 i;
    *codecdata = NULL;
    if ( NULL == codecType )
        return 0;

    /* the codec data is a pointer into the constant table, there is nothing to free */
    for ( i=0; i<sizeof( g_codecs ) / sizeof( g_codecs[ 0 ] ); ++i )
    {
        if ( 0 == strcmp( codecType, g_codecs[ i ].type ) )
        {
            *codecdata = (void*) &g_codecs[ i ];
            return 1;
        }
    }
    return 0;
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
    TOggSource* source = OpenSource( (const TOggPlugin*) *plugdata, input, format, lengthInFrames );
    if ( NULL == source )
        return 0;
    *sourcedata = source;
    return 1;
}

/*-------------------------------------------------------------------------*/

UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
MODULE_API_FUNCTION( AUDIOCODECPLUG_Src_Read_Frames )( void** plugdata, void** codecdata, void** sourcedata, void* destination, UInt32 frameCapacity, UInt32* framesRead ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    TOggSource* source = (TOggSource*) *sourcedata;
    UInt32 done = 0;
    UInt64 left;

    *framesRead = 0;
    if ( NULL == source )
        return 0;
    if ( source->isAtEnd )
        return 1;

    /* never past the links that have the format of the first */
    left = source->length > source->position ? source->length - source->position : 0;
    if ( left < frameCapacity )
        frameCapacity = (UInt32) left;

    while ( done < frameCapacity )
    {
        UInt32 got = source->decoder->read( source->decoder, ( (float*) destination ) + (size_t) done * source->channels, frameCapacity - done );
        if ( 0 == got )
            break;
        done += got;
    }

    source->position += done;
    *framesRead = done;
    return 1;
}

/*-------------------------------------------------------------------------*/

UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
MODULE_API_FUNCTION( AUDIOCODECPLUG_Src_Seek )( void** plugdata, void** codecdata, void** sourcedata, UInt64 frame ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    TOggSource* source = (TOggSource*) *sourcedata;
    if ( NULL == source || frame > source->length )
        return 0;

    if ( frame == source->length )
    {
        /* the end is a position too, there is just nothing to decode from there */
        source->isAtEnd = 1;
        source->position = frame;
        return 1;
    }
    if ( !source->decoder->seek( source->decoder, frame ) )
        return 0;
    source->isAtEnd = 0;
    source->position = frame;
    return 1;
}

/*-------------------------------------------------------------------------*/

void GUCEF_PLUGIN_CALLSPEC_PREFIX
MODULE_API_FUNCTION( AUDIOCODECPLUG_Src_Close )( void** plugdata, void** codecdata, void** sourcedata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    CloseSource( (TOggSource*) *sourcedata );
    *sourcedata = NULL;
}

/*-------------------------------------------------------------------------*/

UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
MODULE_API_FUNCTION( AUDIOCODECPLUG_Src_Meta_Data )( void** plugdata, void** codecdata, void** sourcedata, TVariantMapApi* metaData ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    const TOggSource* source = (const TOggSource*) *sourcedata;
    if ( NULL == source )
        return 0;
    source->decoder->tags( source->decoder, metaData );
    return 1;
}

/*-------------------------------------------------------------------------*/

UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
MODULE_API_FUNCTION( AUDIOCODECPLUG_Decode_Meta_Data )( void** plugdata, void** codecdata, TIOAccess* input, TVariantMapApi* metaData ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    TAudioFormatInfo format;
    UInt64 length;
    TOggSource* source = OpenSource( (const TOggPlugin*) *plugdata, input, &format, &length );
    if ( NULL == source )
        return 0;
    source->decoder->tags( source->decoder, metaData );
    CloseSource( source );
    return 1;
}

/*-------------------------------------------------------------------------*/

UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
MODULE_API_FUNCTION( AUDIOCODECPLUG_Encode )( void** plugdata, void** codecdata, const TAudioFormatInfo* format, const void* frames, UInt64 frameCount, TIOAccess* output, const TVariantMapApi* options ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    const TOggPlugin* plugin = (const TOggPlugin*) *plugdata;
    const TOggCodecInfo* info = (const TOggCodecInfo*) *codecdata;
    const char* codec = OggGetTextOption( options, "codec" );
    int useOpus = NULL != info && info->encodesOpus;

    if ( NULL != codec )
        useOpus = 0 == strcmp( codec, "opus" );

    if ( useOpus )
    {
        #ifdef GUCEF_AUDIOCODECPLUGIN_OGG_HAVE_OPUS
        return OggOpusEncode( &plugin->libApi, format, frames, frameCount, output, options );
        #else
        GUCEF_C_ERROR_LOG( plugin->libApi, GUCEF_LOGLEVEL_NORMAL, "audiocodecpluginOGG: this build of the plugin has no Opus encoder" );
        return 0;
        #endif
    }
    return OggVorbisEncode( &plugin->libApi, format, frames, frameCount, output, options );
}

/*-------------------------------------------------------------------------*/

UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
MODULE_API_FUNCTION( AUDIOCODECPLUG_Threading_Model )( const void* plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    /* vorbisfile and opusfile keep everything of a stream in the stream, and the plugin has no state that changes after Init */
    return GUCEF_AUDIOCODEC_THREADING_PER_SOURCE;
}

/*-------------------------------------------------------------------------*/

UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
MODULE_API_FUNCTION( AUDIOCODECPLUG_Probe )( void** plugdata, void** codecdata, TIOAccess* input, UInt32* confidence ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    *confidence = 0;
    if ( NULL == input || NULL == input->read )
        return 0;

    switch ( DetectKind( input ) )
    {
        case OGG_KIND_VORBIS: *confidence = 100; break;
        case OGG_KIND_OPUS:
        {
            #ifdef GUCEF_AUDIOCODECPLUGIN_OGG_HAVE_OPUS
            *confidence = 100;
            #else
            *confidence = 10;           /* it is Ogg Opus, which this build can not decode */
            #endif
            break;
        }
        case OGG_KIND_FLAC: *confidence = 10; break;
        case OGG_KIND_OTHER: *confidence = 5; break;
        default: break;
    }
    return 1;
}

/*-------------------------------------------------------------------------*/
