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

#ifndef GUCEF_AUDIO_ETYPES_H
#define GUCEF_AUDIO_ETYPES_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_ETYPES_H
#include "gucefCORE_ETypes.h"
#define GUCEF_CORE_ETYPES_H
#endif /* GUCEF_CORE_ETYPES_H ? */

#ifdef __cplusplus
#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */
#endif /* __cplusplus ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifdef __cplusplus
namespace GUCEF {
namespace AUDIO {
#endif /* __cplusplus ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifdef __cplusplus
typedef CORE::CString CString;
#endif /* __cplusplus ? */

/**
 *  How one sample of one channel is stored. These are the types that audio hardware and file formats use.
 *  The numeric values are stable, they are used in plugin interfaces: new types are only ever appended.
 */
enum ESampleType
{
    SAMPLETYPE_UNKNOWN = 0 ,
    SAMPLETYPE_UINT8       ,    /**< 8 bit unsigned, silence is 128 */
    SAMPLETYPE_INT16       ,    /**< 16 bit signed, native endian */
    SAMPLETYPE_INT24       ,    /**< 24 bit signed, packed in 3 bytes, native endian */
    SAMPLETYPE_INT32       ,    /**< 32 bit signed, native endian */
    SAMPLETYPE_FLOAT32     ,    /**< 32 bit float, nominal range -1.0 to 1.0 */
    SAMPLETYPE_FLOAT64          /**< 64 bit float, nominal range -1.0 to 1.0 */
};
typedef enum ESampleType TSampleType;

/*-------------------------------------------------------------------------*/

/**
 *  Positions of speakers, as a bit mask that describes which channels a stream has. Interleaved channels are in the
 *  order of the lowest bit to the highest. The values are those of the common WAVE channel mask so they map to
 *  most file formats and drivers directly.
 */
enum ESpeakerPosition
{
    AUDIOSPEAKER_FRONT_LEFT            = 0x00001 ,
    AUDIOSPEAKER_FRONT_RIGHT           = 0x00002 ,
    AUDIOSPEAKER_FRONT_CENTER          = 0x00004 ,
    AUDIOSPEAKER_LOW_FREQUENCY         = 0x00008 ,
    AUDIOSPEAKER_BACK_LEFT             = 0x00010 ,
    AUDIOSPEAKER_BACK_RIGHT            = 0x00020 ,
    AUDIOSPEAKER_FRONT_LEFT_OF_CENTER  = 0x00040 ,
    AUDIOSPEAKER_FRONT_RIGHT_OF_CENTER = 0x00080 ,
    AUDIOSPEAKER_BACK_CENTER           = 0x00100 ,
    AUDIOSPEAKER_SIDE_LEFT             = 0x00200 ,
    AUDIOSPEAKER_SIDE_RIGHT            = 0x00400 ,
    AUDIOSPEAKER_TOP_CENTER            = 0x00800 ,
    AUDIOSPEAKER_TOP_FRONT_LEFT        = 0x01000 ,
    AUDIOSPEAKER_TOP_FRONT_CENTER      = 0x02000 ,
    AUDIOSPEAKER_TOP_FRONT_RIGHT       = 0x04000 ,
    AUDIOSPEAKER_TOP_BACK_LEFT         = 0x08000 ,
    AUDIOSPEAKER_TOP_BACK_CENTER       = 0x10000 ,
    AUDIOSPEAKER_TOP_BACK_RIGHT        = 0x20000
};

/*-------------------------------------------------------------------------*/

/**
 *  How the master stage of the engine keeps the output from going over full level. The values are stable, they are
 *  stored by name in the audio settings of the user: new modes are only ever appended.
 */
enum EAudioLimiterMode
{
    AUDIOLIMITERMODE_OFF = 0 ,      /**< no limiting: what goes over full level clips in the device */
    AUDIOLIMITERMODE_INSTANT ,      /**< turns the output down where it would go over full level, with no delay */
    AUDIOLIMITERMODE_LOOKAHEAD      /**< looks ahead in the signal so it can turn down smoothly, at the cost of latency */
};
typedef enum EAudioLimiterMode TAudioLimiterMode;

/*-------------------------------------------------------------------------*/

/** Direction of an audio stream or device */
enum EAudioDirection
{
    AUDIODIRECTION_PLAYBACK = 0 ,   /**< audio goes out to a speaker */
    AUDIODIRECTION_CAPTURE          /**< audio comes in from a microphone */
};
typedef enum EAudioDirection TAudioDirection;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifdef __cplusplus
}; /* namespace AUDIO */
}; /* namespace GUCEF */
#endif /* __cplusplus ? */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIO_ETYPES_H ? */
