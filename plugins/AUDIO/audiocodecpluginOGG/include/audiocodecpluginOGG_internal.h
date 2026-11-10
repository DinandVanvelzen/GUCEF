/*
 *  audiocodecpluginOGG: gucefAUDIO codec plugin for OGG audio
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

#ifndef GUCEF_AUDIOCODECPLUGIN_OGG_INTERNAL_H
#define GUCEF_AUDIOCODECPLUGIN_OGG_INTERNAL_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_AUDIO_C_AUDIOCODEC_PLUGIN_API_H
#include "gucefAUDIO_c_audiocodec_plugin_api.h"     /* the interface this plugin implements */
#define GUCEF_AUDIO_C_AUDIOCODEC_PLUGIN_API_H
#endif /* GUCEF_AUDIO_C_AUDIOCODEC_PLUGIN_API_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The input of one stream as the Ogg libraries see it: positions count from where the Ogg data starts */
struct SOggInput
{
    TIOAccess* input;
    UInt64 start;
    UInt64 size;                /* bytes from 'start' to the end, 0 when the input does not tell */
};
typedef struct SOggInput TOggInput;

/** One line of NAME=value out of the "tags" option of the encoders */
struct SOggTagLine
{
    const char* name;
    size_t nameLength;
    const char* value;
    size_t valueLength;
};
typedef struct SOggTagLine TOggTagLine;

struct SOggDecoder;

/**
 *  A decoder of one codec on top of an Ogg stream. It makes 32 bit float frames in the order of a WAVE file.
 *  The plugin keeps the position and the length, a decoder is only asked for the audio.
 */
struct SOggDecoder
{
    /** Decodes up to 'frames' frames into 'destination', returns how many, 0 at the end of the stream (or when it can not go on) */
    UInt32 ( *read )( struct SOggDecoder* decoder, float* destination, UInt32 frames );
    /** Moves to a frame, which is inside the stream. Returns 1 when it worked */
    UInt32 ( *seek )( struct SOggDecoder* decoder, UInt64 frame );
    /** Adds the tags of the stream to the map */
    void ( *tags )( struct SOggDecoder* decoder, TVariantMapApi* metaData );
    void ( *close )( struct SOggDecoder* decoder );
    UInt32 channels;
    UInt32 sampleRate;
    UInt32 channelLayout;
    UInt64 length;
};
typedef struct SOggDecoder TOggDecoder;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifdef __cplusplus
extern "C" {
#endif

/* The input, in audiocodecpluginOGG_common.c. Return values as the C library functions they stand in for */
size_t OggInputRead( TOggInput* io, void* destination, size_t bytes );
int OggInputSeek( TOggInput* io, Int64 offset, int whence );
Int64 OggInputTell( TOggInput* io );

/* Options and tags */
const char* OggGetTextOption( const TVariantMapApi* map, const char* key );
double OggGetNumberOption( const TVariantMapApi* map, const char* key, double defaultValue );
void OggSetText( TVariantMapApi* map, const char* key, const char* value, UInt32 valueSize );
void OggAddComments( TVariantMapApi* map, char** comments, const int* lengths, int count );
const char* OggNextTagLine( const char* lines, TOggTagLine* line );

/* Channels: the Vorbis order of 1 to 8 channels is not the WAVE order */
const UInt8* OggWaveToVorbisOrder( UInt32 channels );
UInt32 OggWaveLayout( UInt32 channels );

/** The sample at 'index' of interleaved input of any sample type as a float */
float OggGetSampleAsFloat( const UInt8* data, UInt32 sampleType, UInt64 index );
UInt32 OggBytesPerSample( UInt32 sampleType );
int OggWriteBytes( TIOAccess* output, const void* data, size_t size );

/* Vorbis, in audiocodecpluginOGG_vorbis.c */
TOggDecoder* OggVorbisOpen( const TGucefCoreCApi* libApi, TOggInput* io );
UInt32 OggVorbisEncode( const TGucefCoreCApi* libApi, const TAudioFormatInfo* format, const void* frames, UInt64 frameCount, TIOAccess* output, const TVariantMapApi* options );

#ifdef GUCEF_AUDIOCODECPLUGIN_OGG_HAVE_OPUS
/* Opus, in audiocodecpluginOGG_opus.c */
TOggDecoder* OggOpusOpen( const TGucefCoreCApi* libApi, TOggInput* io );
UInt32 OggOpusEncode( const TGucefCoreCApi* libApi, const TAudioFormatInfo* format, const void* frames, UInt64 frameCount, TIOAccess* output, const TVariantMapApi* options );
#endif /* GUCEF_AUDIOCODECPLUGIN_OGG_HAVE_OPUS ? */

#ifdef __cplusplus
   }
#endif /* __cplusplus */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIOCODECPLUGIN_OGG_INTERNAL_H ? */
