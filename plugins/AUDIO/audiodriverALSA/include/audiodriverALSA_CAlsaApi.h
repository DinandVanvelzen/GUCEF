/*
 *  audiodriverALSA: gucefAUDIO driver plugin for ALSA on Linux
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

#ifndef GUCEF_AUDIODRIVER_ALSA_CALSAAPI_H
#define GUCEF_AUDIODRIVER_ALSA_CALSAAPI_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_AUDIO_ETYPES_H
#include "gucefAUDIO_ETypes.h"
#define GUCEF_AUDIO_ETYPES_H
#endif /* GUCEF_AUDIO_ETYPES_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUDIODRIVER {
namespace ALSA {

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *  The parts of the ALSA library interface that are used here. libasound keeps this C interface stable, so the
 *  declarations are made here instead of taken from the ALSA development headers, which builds need not have.
 */
extern "C" {

typedef struct _snd_pcm snd_pcm_t;
typedef struct _snd_pcm_hw_params snd_pcm_hw_params_t;
typedef struct _snd_pcm_sw_params snd_pcm_sw_params_t;
typedef unsigned long snd_pcm_uframes_t;
typedef long snd_pcm_sframes_t;

typedef int ( *TSndPcmOpen )( snd_pcm_t** pcm, const char* name, int stream, int mode );
typedef int ( *TSndPcmClose )( snd_pcm_t* pcm );
typedef int ( *TSndPcmHwParamsMalloc )( snd_pcm_hw_params_t** params );
typedef void ( *TSndPcmHwParamsFree )( snd_pcm_hw_params_t* params );
typedef int ( *TSndPcmHwParamsAny )( snd_pcm_t* pcm, snd_pcm_hw_params_t* params );
typedef int ( *TSndPcmHwParamsSetAccess )( snd_pcm_t* pcm, snd_pcm_hw_params_t* params, int access );
typedef int ( *TSndPcmHwParamsTestFormat )( snd_pcm_t* pcm, snd_pcm_hw_params_t* params, int format );
typedef int ( *TSndPcmHwParamsSetFormat )( snd_pcm_t* pcm, snd_pcm_hw_params_t* params, int format );
typedef int ( *TSndPcmHwParamsSetChannelsNear )( snd_pcm_t* pcm, snd_pcm_hw_params_t* params, unsigned int* channels );
typedef int ( *TSndPcmHwParamsSetRateNear )( snd_pcm_t* pcm, snd_pcm_hw_params_t* params, unsigned int* rate, int* dir );
typedef int ( *TSndPcmHwParamsSetPeriodSizeNear )( snd_pcm_t* pcm, snd_pcm_hw_params_t* params, snd_pcm_uframes_t* frames, int* dir );
typedef int ( *TSndPcmHwParamsSetBufferSizeNear )( snd_pcm_t* pcm, snd_pcm_hw_params_t* params, snd_pcm_uframes_t* frames );
typedef int ( *TSndPcmHwParams )( snd_pcm_t* pcm, snd_pcm_hw_params_t* params );
typedef int ( *TSndPcmHwParamsGetPeriodSize )( const snd_pcm_hw_params_t* params, snd_pcm_uframes_t* frames, int* dir );
typedef int ( *TSndPcmHwParamsGetBufferSize )( const snd_pcm_hw_params_t* params, snd_pcm_uframes_t* frames );
typedef int ( *TSndPcmPrepare )( snd_pcm_t* pcm );
typedef int ( *TSndPcmDrop )( snd_pcm_t* pcm );
typedef snd_pcm_sframes_t ( *TSndPcmWritei )( snd_pcm_t* pcm, const void* buffer, snd_pcm_uframes_t frames );
typedef snd_pcm_sframes_t ( *TSndPcmReadi )( snd_pcm_t* pcm, void* buffer, snd_pcm_uframes_t frames );
typedef int ( *TSndPcmStart )( snd_pcm_t* pcm );
typedef int ( *TSndPcmDelay )( snd_pcm_t* pcm, snd_pcm_sframes_t* delay );
typedef snd_pcm_sframes_t ( *TSndPcmAvailUpdate )( snd_pcm_t* pcm );
typedef int ( *TSndPcmSwParamsMalloc )( snd_pcm_sw_params_t** params );
typedef void ( *TSndPcmSwParamsFree )( snd_pcm_sw_params_t* params );
typedef int ( *TSndPcmSwParamsCurrent )( snd_pcm_t* pcm, snd_pcm_sw_params_t* params );
typedef int ( *TSndPcmSwParamsSetStartThreshold )( snd_pcm_t* pcm, snd_pcm_sw_params_t* params, snd_pcm_uframes_t value );
typedef int ( *TSndPcmSwParamsSetAvailMin )( snd_pcm_t* pcm, snd_pcm_sw_params_t* params, snd_pcm_uframes_t value );
typedef int ( *TSndPcmSwParams )( snd_pcm_t* pcm, snd_pcm_sw_params_t* params );
typedef int ( *TSndPcmLink )( snd_pcm_t* pcm1, snd_pcm_t* pcm2 );
typedef int ( *TSndPcmUnlink )( snd_pcm_t* pcm );
typedef int ( *TSndPcmRecover )( snd_pcm_t* pcm, int error, int silent );
typedef const char* ( *TSndStrerror )( int error );
typedef int ( *TSndDeviceNameHint )( int card, const char* iface, void*** hints );
typedef char* ( *TSndDeviceNameGetHint )( const void* hint, const char* id );
typedef int ( *TSndDeviceNameFreeHint )( void** hints );

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The ALSA library, loaded at run time. All entries are valid once Load() succeeded.
 */
class CAlsaApi
{
    public:

    // values of the ALSA enumerations that are used, as the ALSA interface defines them
    static const int StreamPlayback = 0;            /**< SND_PCM_STREAM_PLAYBACK */
    static const int StreamCapture = 1;             /**< SND_PCM_STREAM_CAPTURE */
    static const int AccessRwInterleaved = 3;       /**< SND_PCM_ACCESS_RW_INTERLEAVED */

    CAlsaApi( void );
    ~CAlsaApi();

    /**
     *  Loads libasound. False when it is not on the system or misses something that is needed for playback. Functions
     *  that only capture, the software parameters, the delay query and linking need are optional: when libasound
     *  lacks one, the feature is unavailable (see the Has...() methods) and the driver still loads
     */
    bool Load( void );
    void Unload( void );
    bool IsLoaded( void ) const;

    /** True when libasound has what recording needs: snd_pcm_readi() and snd_pcm_start() */
    bool HasCaptureSupport( void ) const;

    /** True when libasound has everything to set the software parameters (avail_min, start threshold) */
    bool HasSoftwareParams( void ) const;

    /** True when libasound has snd_pcm_delay() */
    bool HasDelayQuery( void ) const;

    /** The ALSA sample format for a sample type in the byte order of this machine, -1 when ALSA has none */
    static int ToAlsaFormat( AUDIO::TSampleType sampleType );

    /** Text of an ALSA error code */
    CORE::CString ErrorText( int error ) const;

    TSndPcmOpen pcmOpen;
    TSndPcmClose pcmClose;
    TSndPcmHwParamsMalloc hwParamsMalloc;
    TSndPcmHwParamsFree hwParamsFree;
    TSndPcmHwParamsAny hwParamsAny;
    TSndPcmHwParamsSetAccess hwParamsSetAccess;
    TSndPcmHwParamsTestFormat hwParamsTestFormat;
    TSndPcmHwParamsSetFormat hwParamsSetFormat;
    TSndPcmHwParamsSetChannelsNear hwParamsSetChannelsNear;
    TSndPcmHwParamsSetRateNear hwParamsSetRateNear;
    TSndPcmHwParamsSetPeriodSizeNear hwParamsSetPeriodSizeNear;
    TSndPcmHwParamsSetBufferSizeNear hwParamsSetBufferSizeNear;
    TSndPcmHwParams hwParams;
    TSndPcmHwParamsGetPeriodSize hwParamsGetPeriodSize;
    TSndPcmHwParamsGetBufferSize hwParamsGetBufferSize;
    TSndPcmPrepare pcmPrepare;
    TSndPcmDrop pcmDrop;
    TSndPcmWritei pcmWritei;
    TSndPcmReadi pcmReadi;
    TSndPcmStart pcmStart;
    TSndPcmDelay pcmDelay;
    TSndPcmAvailUpdate pcmAvailUpdate;
    TSndPcmSwParamsMalloc swParamsMalloc;
    TSndPcmSwParamsFree swParamsFree;
    TSndPcmSwParamsCurrent swParamsCurrent;
    TSndPcmSwParamsSetStartThreshold swParamsSetStartThreshold;
    TSndPcmSwParamsSetAvailMin swParamsSetAvailMin;
    TSndPcmSwParams swParams;
    TSndPcmLink pcmLink;
    TSndPcmUnlink pcmUnlink;
    TSndPcmRecover pcmRecover;
    TSndStrerror strerror;
    TSndDeviceNameHint deviceNameHint;
    TSndDeviceNameGetHint deviceNameGetHint;
    TSndDeviceNameFreeHint deviceNameFreeHint;

    private:

    void* GetFunction( const char* name, bool& allFound, bool required = true ) const;

    CAlsaApi( const CAlsaApi& src );            /**< not implemented */
    CAlsaApi& operator=( const CAlsaApi& src ); /**< not implemented */

    private:

    void* m_module;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ALSA */
}; /* namespace AUDIODRIVER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIODRIVER_ALSA_CALSAAPI_H ? */
