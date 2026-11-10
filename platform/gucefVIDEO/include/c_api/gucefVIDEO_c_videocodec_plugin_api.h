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

#ifndef GUCEF_VIDEO_C_VIDEOCODEC_PLUGIN_API_H
#define GUCEF_VIDEO_C_VIDEOCODEC_PLUGIN_API_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_AUDIO_C_AUDIOCODEC_PLUGIN_API_H
#include "gucefAUDIO_c_audiocodec_plugin_api.h"     /* TAudioFormatInfo, and through it TIOAccess, TVariantMapApi, TGucefCoreCApi, TVersion */
#define GUCEF_AUDIO_C_AUDIOCODEC_PLUGIN_API_H
#endif /* GUCEF_AUDIO_C_AUDIOCODEC_PLUGIN_API_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifdef __cplusplus
namespace GUCEF {
namespace VIDEO {

using CORE::TIOAccess;
using CORE::TVariantMapApi;
using CORE::TVersion;
using AUDIO::TAudioFormatInfo;
#endif /* __cplusplus ? */

/*
 *      Prevent C++ name mangling
 */
#ifdef __cplusplus
extern "C" {
#endif

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/* Values of TVideoFormatInfo::pixelFormat. They never change, new ones are only appended */
#define GUCEF_VIDEOCODEC_PIXELFORMAT_UNKNOWN    0
#define GUCEF_VIDEOCODEC_PIXELFORMAT_PALETTE8   1   /* one byte per pixel, an index into a palette of 256 RGB entries */
#define GUCEF_VIDEOCODEC_PIXELFORMAT_RGB24      2   /* bytes R, G, B */
#define GUCEF_VIDEOCODEC_PIXELFORMAT_RGBA32     3   /* bytes R, G, B, A */
#define GUCEF_VIDEOCODEC_PIXELFORMAT_BGRA32     4   /* bytes B, G, R, A */

/* Frame count of a video that does not tell how long it is */
#define GUCEF_VIDEOCODEC_FRAMECOUNT_UNKNOWN     ( ~( (UInt64) 0 ) )   /* the same as GUCEF_UINT64MAX, written so C compilers take it without a suffix */

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  What a video is. Published once and never changed, a later need gets a new struct and new functions.
 */
struct SVideoFormatInfo
{
    UInt32 width;                   /**< pixels */
    UInt32 height;                  /**< lines, as the picture is shown (doubled or interlaced lines already counted) */
    UInt32 pixelFormat;             /**< GUCEF_VIDEOCODEC_PIXELFORMAT_* */
    UInt32 frameRateNumerator;      /**< frames per second as a fraction: numerator / denominator */
    UInt32 frameRateDenominator;
    UInt32 audioTrackCount;         /**< audio tracks are numbered 0 to audioTrackCount-1 */
};
typedef struct SVideoFormatInfo TVideoFormatInfo;

/*-------------------------------------------------------------------------*/

/**
 *  The picture of a frame. The host owns the memory and keeps the previous picture in it between calls, so a
 *  plugin for a format with delta frames only writes what changed. Never changed after publication.
 */
struct SVideoPictureBuffer
{
    void* pixels;                   /**< 'height' lines of 'pitch' bytes */
    UInt32 pitch;                   /**< bytes from one line to the next */
    UInt8* palette;                 /**< PALETTE8: 256 entries of 3 bytes R, G, B; NULL for other pixel formats */
    UInt32 paletteChanged;          /**< set by the plugin: 1 when the palette changed with this frame */
};
typedef struct SVideoPictureBuffer TVideoPictureBuffer;

/*-------------------------------------------------------------------------*/

typedef void ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TVIDEOCODECPLUGFPTR_OnAudio )( void* userData, UInt32 track, const void* frames, UInt32 frameCount ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

/**
 *  Where the sound of a frame goes. The plugin calls onAudio during Src_Decode_Frame, any number of times, with
 *  interleaved frames in the format of the track (Src_Get_Audio_Track_Format). The host copies them.
 */
struct SVideoAudioOutput
{
    TVIDEOCODECPLUGFPTR_OnAudio onAudio;
    void* userData;
};
typedef struct SVideoAudioOutput TVideoAudioOutput;

/*-------------------------------------------------------------------------*/

/*
 *  The functions a video codec plugin exports. The host looks them up by name: the type name without the
 *  leading 'TVIDEOCODECPLUGFPTR' and with 'VIDEOCODECPLUG' in front, so 'VIDEOCODECPLUG_Init' and so on.
 *  Functions marked optional may be left out, the host then does without.
 *
 *  Status codes: 1 is success, 0 is failure.
 *  State comes in levels: 'plugdata' per plugin, 'codecdata' per codec type the plugin offers and 'sourcedata'
 *  per open video. The plugin allocates and frees each of them, the host only passes them back.
 *  Strings the plugin hands out are its own and must stay valid until Shutdown. Text that belongs to a video
 *  (tags) goes into the host maps through TVariantMapApi, where the host copies it.
 *  The host calls into one plugin from one thread at a time, so a plugin needs no locks of its own.
 */

/* plugin life cycle and identity */
typedef UInt32 ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TVIDEOCODECPLUGFPTR_Init )               ( void** plugdata, TGucefCoreCApi* libApi ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;
typedef void ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TVIDEOCODECPLUGFPTR_Shutdown )             ( void** plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;
typedef const char* ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TVIDEOCODECPLUGFPTR_Name )          ( const void* plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;
typedef const char* ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TVIDEOCODECPLUGFPTR_Copyright )     ( const void* plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;
typedef const TVersion* ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TVIDEOCODECPLUGFPTR_Version )   ( const void* plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;
typedef UInt32 ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TVIDEOCODECPLUGFPTR_Type_Count )         ( const void* plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;
typedef const char* ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TVIDEOCODECPLUGFPTR_Type )          ( const void* plugdata, UInt32 index ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

/* per codec type */
typedef UInt32 ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TVIDEOCODECPLUGFPTR_Create_Codec )       ( void** plugdata, const char* codecType, void** codecdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;
typedef void ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TVIDEOCODECPLUGFPTR_Destroy_Codec )        ( void** plugdata, void** codecdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

/*
 *  Decoding. Src_Open reads what it needs from 'input' and fills in the format and the frame count
 *  (GUCEF_VIDEOCODEC_FRAMECOUNT_UNKNOWN when it cannot tell). 'options' may be NULL. The input stays valid until
 *  Src_Close. Src_Get_Audio_Track_Format gives the format in which the sound of a track is handed out.
 *  Src_Decode_Frame decodes the next frame: the picture into 'picture', the sound that belongs to it to 'audio'
 *  ('audio' may be NULL: the sound is then dropped). At the end it returns 1 with *endOfStream set to 1 and does
 *  not touch the picture. 0 is an error, after which the host closes the source.
 *  Src_Seek_Frame is optional: absent, or returning 0, means the video cannot seek (there). After a successful
 *  seek the next Src_Decode_Frame gives that frame, completely drawn.
 */
typedef UInt32 ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TVIDEOCODECPLUGFPTR_Src_Open )                   ( void** plugdata, void** codecdata, void** sourcedata, TIOAccess* input, const TVariantMapApi* options, TVideoFormatInfo* format, UInt64* frameCount ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;
typedef UInt32 ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TVIDEOCODECPLUGFPTR_Src_Get_Audio_Track_Format ) ( void** plugdata, void** codecdata, void** sourcedata, UInt32 track, TAudioFormatInfo* format ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;
typedef UInt32 ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TVIDEOCODECPLUGFPTR_Src_Decode_Frame )           ( void** plugdata, void** codecdata, void** sourcedata, TVideoPictureBuffer* picture, const TVideoAudioOutput* audio, UInt32* endOfStream ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;
typedef UInt32 ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TVIDEOCODECPLUGFPTR_Src_Seek_Frame )             ( void** plugdata, void** codecdata, void** sourcedata, UInt64 frame ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;
typedef void ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TVIDEOCODECPLUGFPTR_Src_Close )                    ( void** plugdata, void** codecdata, void** sourcedata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

/*
 *  Optional. Decode_Meta_Data puts what the data says about itself into 'metaData' (tags such as "title"); the
 *  host adds the format and length itself, so a plugin without tags leaves this out.
 */
typedef UInt32 ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TVIDEOCODECPLUGFPTR_Decode_Meta_Data )   ( void** plugdata, void** codecdata, TIOAccess* input, TVariantMapApi* metaData ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

/*-------------------------------------------------------------------------*/

/*
 *  Encoding, all optional: a plugin without them only decodes.
 *
 *  Snk_Open: 'format' has the wanted width, height and frame rate; the plugin writes the pixel format it takes
 *  into format->pixelFormat. 'audioTrackFormats' has 'audioTrackCount' entries with the format of the sound the
 *  host has; the plugin writes over each the format it takes, the host converts to it. 'output' is writable
 *  and seekable. Returns 0 on failure.
 *  Snk_Write_Frame: the next picture, in the pixel format the plugin asked for (palette is NULL). 0 on failure.
 *  Snk_Write_Audio: interleaved frames of a track in the format the plugin asked for. 0 on failure.
 *  Snk_Close: with 'finish' 1 completes the output first and returns 0 when that failed; with 0 only frees.
 */
typedef UInt32 ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TVIDEOCODECPLUGFPTR_Snk_Open )          ( void** plugdata, void** codecdata, void** sinkdata, TIOAccess* output, const TVariantMapApi* options, TVideoFormatInfo* format, UInt32 audioTrackCount, TAudioFormatInfo* audioTrackFormats ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;
typedef UInt32 ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TVIDEOCODECPLUGFPTR_Snk_Write_Frame )   ( void** plugdata, void** codecdata, void** sinkdata, const TVideoPictureBuffer* picture ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;
typedef UInt32 ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TVIDEOCODECPLUGFPTR_Snk_Write_Audio )   ( void** plugdata, void** codecdata, void** sinkdata, UInt32 track, const void* frames, UInt32 frameCount ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;
typedef UInt32 ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TVIDEOCODECPLUGFPTR_Snk_Close )         ( void** plugdata, void** codecdata, void** sinkdata, UInt32 finish ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

/*-------------------------------------------------------------------------*/

/**
 *  All the functions of a video codec plugin, as the host found them. Optional ones are NULL when absent.
 */
struct SGucefVideoCodecPluginApi
{
    TVIDEOCODECPLUGFPTR_Init Init;
    TVIDEOCODECPLUGFPTR_Shutdown Shutdown;
    TVIDEOCODECPLUGFPTR_Name Name;
    TVIDEOCODECPLUGFPTR_Copyright Copyright;
    TVIDEOCODECPLUGFPTR_Version Version;
    TVIDEOCODECPLUGFPTR_Type_Count Type_Count;
    TVIDEOCODECPLUGFPTR_Type Type;
    TVIDEOCODECPLUGFPTR_Create_Codec Create_Codec;
    TVIDEOCODECPLUGFPTR_Destroy_Codec Destroy_Codec;
    TVIDEOCODECPLUGFPTR_Src_Open Src_Open;
    TVIDEOCODECPLUGFPTR_Src_Get_Audio_Track_Format Src_Get_Audio_Track_Format;
    TVIDEOCODECPLUGFPTR_Src_Decode_Frame Src_Decode_Frame;
    TVIDEOCODECPLUGFPTR_Src_Seek_Frame Src_Seek_Frame;              /**< optional */
    TVIDEOCODECPLUGFPTR_Src_Close Src_Close;
    TVIDEOCODECPLUGFPTR_Decode_Meta_Data Decode_Meta_Data;          /**< optional */
    TVIDEOCODECPLUGFPTR_Snk_Open Snk_Open;                          /**< optional, with the other Snk_ functions */
    TVIDEOCODECPLUGFPTR_Snk_Write_Frame Snk_Write_Frame;
    TVIDEOCODECPLUGFPTR_Snk_Write_Audio Snk_Write_Audio;
    TVIDEOCODECPLUGFPTR_Snk_Close Snk_Close;
};
typedef struct SGucefVideoCodecPluginApi TGucefVideoCodecPluginApi;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifdef __cplusplus
   }
#endif /* __cplusplus */

#ifdef __cplusplus
}; /* namespace VIDEO */
}; /* namespace GUCEF */
#endif /* __cplusplus ? */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_VIDEO_C_VIDEOCODEC_PLUGIN_API_H ? */
