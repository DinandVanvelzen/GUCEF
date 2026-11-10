/*
 *  audiodriverAAUDIO: gucefAUDIO driver plugin for AAudio on Android
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

#ifndef GUCEF_AUDIODRIVER_AAUDIO_CAAUDIOAPI_H
#define GUCEF_AUDIODRIVER_AAUDIO_CAAUDIOAPI_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_AUDIO_ETYPES_H
#include "gucefAUDIO_ETypes.h"
#define GUCEF_AUDIO_ETYPES_H
#endif /* GUCEF_AUDIO_ETYPES_H ? */

#ifndef GUCEF_AUDIODRIVER_AAUDIO_MACROS_H
#include "audiodriverAAUDIO_macros.h"
#define GUCEF_AUDIODRIVER_AAUDIO_MACROS_H
#endif /* GUCEF_AUDIODRIVER_AAUDIO_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUDIODRIVER {
namespace AAUDIO {

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *  The parts of the AAudio interface that are used here. AAudio is a stable C interface of the platform; the
 *  declarations are made here so the plugin binds to libaaudio at run time and builds for any minimum API level.
 */
extern "C" {

typedef struct AAudioStreamStruct AAudioStream;
typedef struct AAudioStreamBuilderStruct AAudioStreamBuilder;
typedef CORE::Int32 aaudio_result_t;

typedef aaudio_result_t ( *TAAudioCreateStreamBuilder )( AAudioStreamBuilder** builder );
typedef void ( *TAAudioBuilderSetInt )( AAudioStreamBuilder* builder, CORE::Int32 value );
typedef aaudio_result_t ( *TAAudioBuilderOpenStream )( AAudioStreamBuilder* builder, AAudioStream** stream );
typedef aaudio_result_t ( *TAAudioBuilderDelete )( AAudioStreamBuilder* builder );
typedef aaudio_result_t ( *TAAudioStreamAction )( AAudioStream* stream );
typedef CORE::Int32 ( *TAAudioStreamGetInt )( AAudioStream* stream );
typedef aaudio_result_t ( *TAAudioStreamSetBufferSize )( AAudioStream* stream, CORE::Int32 frames );
typedef aaudio_result_t ( *TAAudioStreamWrite )( AAudioStream* stream, const void* buffer, CORE::Int32 frames, CORE::Int64 timeoutNanoseconds );
typedef const char* ( *TAAudioConvertResultToText )( aaudio_result_t result );

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  libaaudio, loaded at run time. All entries are valid once Load() succeeded.
 */
class CAAudioApi
{
    public:

    // values of the AAudio enumerations that are used, as the AAudio interface defines them
    static const CORE::Int32 Ok = 0;                                /**< AAUDIO_OK */
    static const CORE::Int32 ErrorDisconnected = -899;              /**< AAUDIO_ERROR_DISCONNECTED */
    static const CORE::Int32 ErrorTimeout = -885;                   /**< AAUDIO_ERROR_TIMEOUT */
    static const CORE::Int32 Unspecified = 0;                       /**< AAUDIO_UNSPECIFIED */
    static const CORE::Int32 DirectionOutput = 0;                   /**< AAUDIO_DIRECTION_OUTPUT */
    static const CORE::Int32 FormatPcmI16 = 1;                      /**< AAUDIO_FORMAT_PCM_I16 */
    static const CORE::Int32 FormatPcmFloat = 2;                    /**< AAUDIO_FORMAT_PCM_FLOAT */
    static const CORE::Int32 FormatPcmI24Packed = 3;                /**< AAUDIO_FORMAT_PCM_I24_PACKED, Android 12 */
    static const CORE::Int32 FormatPcmI32 = 4;                      /**< AAUDIO_FORMAT_PCM_I32, Android 12 */
    static const CORE::Int32 SharingModeExclusive = 0;              /**< AAUDIO_SHARING_MODE_EXCLUSIVE */
    static const CORE::Int32 SharingModeShared = 1;                 /**< AAUDIO_SHARING_MODE_SHARED */
    static const CORE::Int32 PerformanceModeNone = 10;              /**< AAUDIO_PERFORMANCE_MODE_NONE */
    static const CORE::Int32 PerformanceModeLowLatency = 12;        /**< AAUDIO_PERFORMANCE_MODE_LOW_LATENCY */
    static const CORE::Int32 UsageGame = 14;                        /**< AAUDIO_USAGE_GAME, Android 9 */
    static const CORE::Int32 ContentTypeMusic = 2;                  /**< AAUDIO_CONTENT_TYPE_MUSIC, Android 9 */

    CAAudioApi( void );

    ~CAAudioApi();

    /** Loads libaaudio. False when it is not on the device or misses something that is needed */
    bool Load( void );

    void Unload( void );

    bool IsLoaded( void ) const;

    /** The AAudio format for a sample type, Unspecified when AAudio has none */
    static CORE::Int32 ToAAudioFormat( AUDIO::TSampleType sampleType );

    /** The sample type of an AAudio format, SAMPLETYPE_UNKNOWN when there is none */
    static AUDIO::TSampleType FromAAudioFormat( CORE::Int32 format );

    /** Text of an AAudio result code */
    CORE::CString ResultText( aaudio_result_t result ) const;

    TAAudioCreateStreamBuilder createStreamBuilder;
    TAAudioBuilderSetInt builderSetDeviceId;
    TAAudioBuilderSetInt builderSetDirection;
    TAAudioBuilderSetInt builderSetSharingMode;
    TAAudioBuilderSetInt builderSetSampleRate;
    TAAudioBuilderSetInt builderSetChannelCount;
    TAAudioBuilderSetInt builderSetFormat;
    TAAudioBuilderSetInt builderSetBufferCapacityInFrames;
    TAAudioBuilderSetInt builderSetPerformanceMode;
    TAAudioBuilderSetInt builderSetUsage;                   /**< optional, Android 9, NULL before */
    TAAudioBuilderSetInt builderSetContentType;             /**< optional, Android 9, NULL before */
    TAAudioBuilderOpenStream builderOpenStream;
    TAAudioBuilderDelete builderDelete;
    TAAudioStreamAction streamRequestStart;
    TAAudioStreamAction streamRequestStop;
    TAAudioStreamAction streamClose;
    TAAudioStreamWrite streamWrite;
    TAAudioStreamGetInt streamGetSampleRate;
    TAAudioStreamGetInt streamGetChannelCount;
    TAAudioStreamGetInt streamGetFormat;
    TAAudioStreamGetInt streamGetFramesPerBurst;
    TAAudioStreamGetInt streamGetBufferCapacityInFrames;
    TAAudioStreamGetInt streamGetBufferSizeInFrames;
    TAAudioStreamSetBufferSize streamSetBufferSizeInFrames;
    TAAudioStreamGetInt streamGetXRunCount;
    TAAudioConvertResultToText convertResultToText;

    private:

    void* GetFunction( const char* name, bool& allFound, bool optional = false ) const;

    CAAudioApi( const CAAudioApi& src );            /**< not implemented */
    CAAudioApi& operator=( const CAAudioApi& src ); /**< not implemented */

    private:

    void* m_module;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AAUDIO */
}; /* namespace AUDIODRIVER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIODRIVER_AAUDIO_CAAUDIOAPI_H ? */
