/*
 *  gucefAUDIO: GUCEF module providing audio playback, mixing and decoding
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

#ifndef GUCEF_AUDIO_C_AUDIOCODEC_PLUGIN_API_H
#define GUCEF_AUDIO_C_AUDIOCODEC_PLUGIN_API_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_MACROS_H
#include "gucefCORE_macros.h"
#define GUCEF_CORE_MACROS_H
#endif /* GUCEF_CORE_MACROS_H ? */

#ifndef GUCEF_CORE_ESTRUCTS_H
#include "EStructs.h"                   /* TVersion */
#define GUCEF_CORE_ESTRUCTS_H
#endif /* GUCEF_CORE_ESTRUCTS_H ? */

#ifndef GUCEF_CORE_C_API_H
#include "gucefCORE_c_api.h"            /* TGucefCoreCApi, logging for plugins */
#define GUCEF_CORE_C_API_H
#endif /* GUCEF_CORE_C_API_H ? */

#ifndef GUCEF_CORE_C_IOACCESS_H
#include "gucefCORE_c_ioaccess.h"       /* TIOAccess */
#define GUCEF_CORE_C_IOACCESS_H
#endif /* GUCEF_CORE_C_IOACCESS_H ? */

#ifndef GUCEF_CORE_C_VARIANTMAP_H
#include "gucefCORE_c_variantmap.h"     /* TVariantMapApi */
#define GUCEF_CORE_C_VARIANTMAP_H
#endif /* GUCEF_CORE_C_VARIANTMAP_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifdef __cplusplus
namespace GUCEF {
namespace AUDIO {

using CORE::TIOAccess;
using CORE::TVariantMapApi;
using CORE::TVersion;
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

/* Values of TAudioFormatInfo::sampleType, the same as ESampleType in gucefAUDIO_ETypes.h. They never change */
#define GUCEF_AUDIOCODEC_SAMPLETYPE_UNKNOWN     0
#define GUCEF_AUDIOCODEC_SAMPLETYPE_UINT8       1   /* 8 bit unsigned, silence is 128 */
#define GUCEF_AUDIOCODEC_SAMPLETYPE_INT16       2   /* 16 bit signed, native endian */
#define GUCEF_AUDIOCODEC_SAMPLETYPE_INT24       3   /* 24 bit signed, packed in 3 bytes, native endian */
#define GUCEF_AUDIOCODEC_SAMPLETYPE_INT32       4   /* 32 bit signed, native endian */
#define GUCEF_AUDIOCODEC_SAMPLETYPE_FLOAT32     5   /* 32 bit float, -1.0 to 1.0 */
#define GUCEF_AUDIOCODEC_SAMPLETYPE_FLOAT64     6   /* 64 bit float, -1.0 to 1.0 */

/* Values returned by AUDIOCODECPLUG_Threading_Model, see there */
#define GUCEF_AUDIOCODEC_THREADING_PLUGIN_LOCK  0   /* the host makes one call at a time into the plugin, the default */
#define GUCEF_AUDIOCODEC_THREADING_PER_SOURCE   1   /* calls for different open streams may run at the same time */

/* Length of a stream that does not tell how long it is */
#define GUCEF_AUDIOCODEC_LENGTH_UNKNOWN         ( ~( (UInt64) 0 ) )   /* the same as GUCEF_UINT64MAX, written so C compilers take it without a suffix */

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The format of decoded frames: interleaved samples, one per channel per frame.
 *  Published once and never changed, a later need gets a new struct and new functions.
 */
struct SAudioFormatInfo
{
    UInt32 sampleRate;      /**< frames per second */
    UInt32 channelCount;
    UInt32 sampleType;      /**< GUCEF_AUDIOCODEC_SAMPLETYPE_* */
    UInt32 channelLayout;   /**< speaker mask as in the WAVE channel mask, 0 = the usual layout for the channel count */
};
typedef struct SAudioFormatInfo TAudioFormatInfo;

/*-------------------------------------------------------------------------*/

/*
 *  The functions an audio codec plugin exports. The host looks them up by name: the type name without the
 *  leading 'TAUDIOCODECPLUGFPTR' and with 'AUDIOCODECPLUG' in front, so 'AUDIOCODECPLUG_Init' and so on.
 *  Functions marked optional may be left out, the host then does without.
 *
 *  Status codes: 1 is success, 0 is failure.
 *  State comes in levels: 'plugdata' per plugin, 'codecdata' per codec type the plugin offers and 'sourcedata'
 *  per open stream. The plugin allocates and frees each of them, the host only passes them back.
 *  Strings the plugin hands out are its own and must stay valid until Shutdown. Text that belongs to a stream
 *  (tags) goes into the host maps through TVariantMapApi, where the host copies it.
 *  The host calls into one plugin from one thread at a time, so a plugin needs no locks of its own, unless it
 *  declares otherwise with AUDIOCODECPLUG_Threading_Model.
 */

/* plugin life cycle and identity */
typedef UInt32 ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TAUDIOCODECPLUGFPTR_Init )               ( void** plugdata, TGucefCoreCApi* libApi ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;
typedef void ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TAUDIOCODECPLUGFPTR_Shutdown )             ( void** plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;
typedef const char* ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TAUDIOCODECPLUGFPTR_Name )          ( const void* plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;
typedef const char* ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TAUDIOCODECPLUGFPTR_Copyright )     ( const void* plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;
typedef const TVersion* ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TAUDIOCODECPLUGFPTR_Version )   ( const void* plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;
typedef UInt32 ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TAUDIOCODECPLUGFPTR_Type_Count )         ( const void* plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;
typedef const char* ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TAUDIOCODECPLUGFPTR_Type )          ( const void* plugdata, UInt32 index ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

/* per codec type */
typedef UInt32 ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TAUDIOCODECPLUGFPTR_Create_Codec )       ( void** plugdata, const char* codecType, void** codecdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;
typedef void ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TAUDIOCODECPLUGFPTR_Destroy_Codec )        ( void** plugdata, void** codecdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

/*
 *  Decoding, streaming. Src_Open reads what it needs from 'input' and fills in the format of the frames it will
 *  make and the length (GUCEF_AUDIOCODEC_LENGTH_UNKNOWN when it cannot tell). 'options' may be NULL.
 *  The input stays valid until Src_Close. Src_Read_Frames returns 1 with 0 frames read at the end.
 *  Src_Seek is optional: absent, or returning 0, means the stream cannot seek (there). When it returns 1 the
 *  next Src_Read_Frames starts exactly at the requested frame.
 */
typedef UInt32 ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TAUDIOCODECPLUGFPTR_Src_Open )           ( void** plugdata, void** codecdata, void** sourcedata, TIOAccess* input, const TVariantMapApi* options, TAudioFormatInfo* format, UInt64* lengthInFrames ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;
typedef UInt32 ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TAUDIOCODECPLUGFPTR_Src_Read_Frames )    ( void** plugdata, void** codecdata, void** sourcedata, void* destination, UInt32 frameCapacity, UInt32* framesRead ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;
typedef UInt32 ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TAUDIOCODECPLUGFPTR_Src_Seek )           ( void** plugdata, void** codecdata, void** sourcedata, UInt64 frame ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;
typedef void ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TAUDIOCODECPLUGFPTR_Src_Close )            ( void** plugdata, void** codecdata, void** sourcedata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

/*
 *  Optional. Decode_Meta_Data puts what the data says about itself into 'metaData' (tags such as "title"); the
 *  host adds the format and length itself, so a plugin without tags leaves this out.
 *  Encode writes the frames in the format of the codec; absent means the codec only decodes.
 */
typedef UInt32 ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TAUDIOCODECPLUGFPTR_Decode_Meta_Data )   ( void** plugdata, void** codecdata, TIOAccess* input, TVariantMapApi* metaData ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;
typedef UInt32 ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TAUDIOCODECPLUGFPTR_Encode )             ( void** plugdata, void** codecdata, const TAudioFormatInfo* format, const void* frames, UInt64 frameCount, TIOAccess* output, const TVariantMapApi* options ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

/*
 *  Optional additions. A plugin that leaves them out keeps working as before.
 *
 *  Threading_Model tells how the plugin may be called. It is asked once, after Init, and the answer holds until
 *  Shutdown. GUCEF_AUDIOCODEC_THREADING_PLUGIN_LOCK (also what a plugin without this function gets): one call at
 *  a time into the plugin, from whatever thread. GUCEF_AUDIOCODEC_THREADING_PER_SOURCE: calls that use different
 *  'sourcedata' (Src_Read_Frames, Src_Seek, Src_Meta_Data) may run at the same time on different threads, so
 *  several streams of one plugin decode in parallel; calls for one stream stay one at a time. Everything else
 *  (Init, Shutdown, Create_Codec, Destroy_Codec, Src_Open, Src_Close, Decode_Meta_Data, Encode, Probe) is still
 *  one at a time, but may run at the same time as the stream calls. A plugin declaring this must not change
 *  'plugdata' or 'codecdata' in a stream call without a lock of its own (or an atomic) and must not use
 *  global state that is not protected the same way. Other values are read as GUCEF_AUDIOCODEC_THREADING_PLUGIN_LOCK.
 *
 *  Src_Meta_Data puts what the open stream says about itself into 'metaData' (tags such as "title", loop points
 *  such as "loopstart"), text keys and text values, like Decode_Meta_Data does for a whole input. It may be
 *  called at any time between Src_Open and Src_Close and must not move the stream.
 *
 *  Probe tells whether 'input' looks like data of the codec type 'codecdata' was made for, by reading from the
 *  current position, and sets 'confidence' from 0 (no) to 100 (certain); the host restores the input position
 *  afterwards. Returns 0 when it cannot tell. Meant to be cheap: it reads a header, not the whole input.
 */
typedef UInt32 ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TAUDIOCODECPLUGFPTR_Threading_Model )    ( const void* plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;
typedef UInt32 ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TAUDIOCODECPLUGFPTR_Src_Meta_Data )      ( void** plugdata, void** codecdata, void** sourcedata, TVariantMapApi* metaData ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;
typedef UInt32 ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TAUDIOCODECPLUGFPTR_Probe )              ( void** plugdata, void** codecdata, TIOAccess* input, UInt32* confidence ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

/*-------------------------------------------------------------------------*/

/**
 *  All the functions of an audio codec plugin, as the host found them. Optional ones are NULL when absent.
 *  The counterpart of TGucefCoreCDStorePluginApi.
 */
struct SGucefAudioCodecPluginApi
{
    TAUDIOCODECPLUGFPTR_Init Init;
    TAUDIOCODECPLUGFPTR_Shutdown Shutdown;
    TAUDIOCODECPLUGFPTR_Name Name;
    TAUDIOCODECPLUGFPTR_Copyright Copyright;
    TAUDIOCODECPLUGFPTR_Version Version;
    TAUDIOCODECPLUGFPTR_Type_Count Type_Count;
    TAUDIOCODECPLUGFPTR_Type Type;
    TAUDIOCODECPLUGFPTR_Create_Codec Create_Codec;
    TAUDIOCODECPLUGFPTR_Destroy_Codec Destroy_Codec;
    TAUDIOCODECPLUGFPTR_Src_Open Src_Open;
    TAUDIOCODECPLUGFPTR_Src_Read_Frames Src_Read_Frames;
    TAUDIOCODECPLUGFPTR_Src_Seek Src_Seek;                      /**< optional */
    TAUDIOCODECPLUGFPTR_Src_Close Src_Close;
    TAUDIOCODECPLUGFPTR_Decode_Meta_Data Decode_Meta_Data;      /**< optional */
    TAUDIOCODECPLUGFPTR_Encode Encode;                          /**< optional */
    TAUDIOCODECPLUGFPTR_Threading_Model Threading_Model;        /**< optional */
    TAUDIOCODECPLUGFPTR_Src_Meta_Data Src_Meta_Data;            /**< optional */
    TAUDIOCODECPLUGFPTR_Probe Probe;                            /**< optional */
};
typedef struct SGucefAudioCodecPluginApi TGucefAudioCodecPluginApi;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifdef __cplusplus
   }
#endif /* __cplusplus */

#ifdef __cplusplus
}; /* namespace AUDIO */
}; /* namespace GUCEF */
#endif /* __cplusplus ? */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIO_C_AUDIOCODEC_PLUGIN_API_H ? */
