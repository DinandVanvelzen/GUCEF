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

/**
 *  A named set of device and engine parameters that trade robustness for latency, see
 *  CAudioEngineParams::ApplyLatencyProfile(). The values are stable, they are stored by name in the audio settings of
 *  the user: new profiles are only ever appended.
 */
enum EAudioLatencyProfile
{
    AUDIOLATENCYPROFILE_DEFAULT = 0 ,   /**< what the engine has always done: the driver chooses the device period and the number of periods, normal thread priority, lookahead limiter */
    AUDIOLATENCYPROFILE_LOW             /**< devices of 5 ms periods, 3 of them, real-time thread priority and the instant limiter: for voice and intercom */
};
typedef enum EAudioLatencyProfile TAudioLatencyProfile;

/*-------------------------------------------------------------------------*/

/** Direction of an audio stream or device */
enum EAudioDirection
{
    AUDIODIRECTION_PLAYBACK = 0 ,   /**< audio goes out to a speaker */
    AUDIODIRECTION_CAPTURE          /**< audio comes in from a microphone */
};
typedef enum EAudioDirection TAudioDirection;

/*-------------------------------------------------------------------------*/

/** A stage of effects in the capture path of the engine, in processing order around the place of the echo canceller */
enum EAudioCaptureStage
{
    AUDIOCAPTURESTAGE_PRE_ECHO = 0 ,    /**< before echo handling: effects that the echo canceller should see, a high pass */
    AUDIOCAPTURESTAGE_POST_ECHO ,       /**< after echo handling: noise suppression, gate, gain control, limiter */

    AUDIOCAPTURESTAGE_COUNT             /**< the number of stages, not a stage */
};
typedef enum EAudioCaptureStage TAudioCaptureStage;

/*-------------------------------------------------------------------------*/

/**
 *  The application profile of a voice codec. The values are stable, they are stored by name in the configuration:
 *  new profiles are only ever appended.
 */
enum EVoiceCodecProfile
{
    VOICECODECPROFILE_LOWDELAY = 0 ,    /**< shortest delay, for wired networks and good Wi-Fi */
    VOICECODECPROFILE_ROBUST            /**< tolerates packet loss (in-band forward error correction), for lossy Wi-Fi */
};
typedef enum EVoiceCodecProfile TVoiceCodecProfile;

/*-------------------------------------------------------------------------*/

/**
 *  The playout profile of a voice receiver: the margins and the measuring window of its playout control. The values
 *  are stable, new profiles are only ever appended.
 */
enum EVoicePlayoutProfile
{
    VOICEPLAYOUTPROFILE_WIRELESS = 0 ,  /**< margin 20 ms, at least 6 ms, at most 200 ms, window 4 s, held beyond the 1 % quantile of the slack: Wi-Fi */
    VOICEPLAYOUTPROFILE_WIRED           /**< margin 4 ms, at least 2 ms, at most 60 ms, window 2 s: wired Ethernet */
};
typedef enum EVoicePlayoutProfile TVoicePlayoutProfile;

/*-------------------------------------------------------------------------*/

/** The state of the playout of a voice stream */
enum EVoicePlayoutState
{
    VOICEPLAYOUTSTATE_IDLE = 0 ,        /**< nothing to play: the output is silence */
    VOICEPLAYOUTSTATE_PREBUFFER ,       /**< frames of a talkspurt are there, playout waits for its margin */
    VOICEPLAYOUTSTATE_PLAYING           /**< frames are being decoded and played */
};
typedef enum EVoicePlayoutState TVoicePlayoutState;

/*-------------------------------------------------------------------------*/

/**
 *  When a voice sender transmits. The values are stable, they are stored by name in the configuration: new modes are
 *  only ever appended.
 */
enum EVoiceTransmitMode
{
    VOICETRANSMITMODE_CONTINUOUS = 0 ,      /**< every frame is sent (calls: the receiver's drift control always has data) */
    VOICETRANSMITMODE_VOICE_ACTIVATED ,     /**< frames are sent while the voice activity detector hears a voice, plus a hangover */
    VOICETRANSMITMODE_PUSH_TO_TALK          /**< frames are sent while the talk key is down, plus the frame in progress at its release */
};
typedef enum EVoiceTransmitMode TVoiceTransmitMode;

/*-------------------------------------------------------------------------*/

/** The state of a half-duplex controller (CHalfDuplexController): who may be heard */
enum EHalfDuplexState
{
    HALFDUPLEXSTATE_IDLE = 0 ,          /**< nobody talks: the microphone is open, the speaker as it is */
    HALFDUPLEXSTATE_FAR ,               /**< the far end talks: the microphone is attenuated */
    HALFDUPLEXSTATE_NEAR                /**< the near end talks: the microphone is open, the far end is ducked */
};
typedef enum EHalfDuplexState THalfDuplexState;

/*-------------------------------------------------------------------------*/

/** The state of an acoustic echo canceller (CAcousticEchoCanceller) */
enum EEchoCancellerState
{
    ECHOCANCELLERSTATE_OFF = 0 ,        /**< switched off or not prepared: the microphone passes unchanged */
    ECHOCANCELLERSTATE_WAITING ,        /**< no reference aligned yet (nothing played since it started): the microphone passes */
    ECHOCANCELLERSTATE_IDLE ,           /**< the far end is quiet over the whole filter: nothing is computed */
    ECHOCANCELLERSTATE_CONVERGING ,     /**< the far end plays and the filter adapts, not converged */
    ECHOCANCELLERSTATE_CONVERGED        /**< the filter removes the echo by at least the converged enhancement of the settings */
};
typedef enum EEchoCancellerState TEchoCancellerState;

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
