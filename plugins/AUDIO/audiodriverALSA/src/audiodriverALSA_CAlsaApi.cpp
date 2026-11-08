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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_DVCPPOSWRAP_H
#include "DVCPPOSWRAP.h"
#define GUCEF_CORE_DVCPPOSWRAP_H
#endif /* GUCEF_CORE_DVCPPOSWRAP_H ? */

#ifndef GUCEF_CORE_DVOSWRAP_H
#include "DVOSWRAP.h"
#define GUCEF_CORE_DVOSWRAP_H
#endif /* GUCEF_CORE_DVOSWRAP_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_AUDIO_CPCMSTREAMSOURCE_H
#include "gucefAUDIO_CPcmStreamSource.h"
#define GUCEF_AUDIO_CPCMSTREAMSOURCE_H
#endif /* GUCEF_AUDIO_CPCMSTREAMSOURCE_H ? */

#include "audiodriverALSA_CAlsaApi.h"

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
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const int CAlsaApi::StreamPlayback;
const int CAlsaApi::StreamCapture;
const int CAlsaApi::AccessRwInterleaved;

/*-------------------------------------------------------------------------*/

CAlsaApi::CAlsaApi( void )
    : pcmOpen( GUCEF_NULL )
    , pcmClose( GUCEF_NULL )
    , hwParamsMalloc( GUCEF_NULL )
    , hwParamsFree( GUCEF_NULL )
    , hwParamsAny( GUCEF_NULL )
    , hwParamsSetAccess( GUCEF_NULL )
    , hwParamsTestFormat( GUCEF_NULL )
    , hwParamsSetFormat( GUCEF_NULL )
    , hwParamsSetChannelsNear( GUCEF_NULL )
    , hwParamsSetRateNear( GUCEF_NULL )
    , hwParamsSetPeriodSizeNear( GUCEF_NULL )
    , hwParamsSetBufferSizeNear( GUCEF_NULL )
    , hwParams( GUCEF_NULL )
    , hwParamsGetPeriodSize( GUCEF_NULL )
    , hwParamsGetBufferSize( GUCEF_NULL )
    , pcmPrepare( GUCEF_NULL )
    , pcmDrop( GUCEF_NULL )
    , pcmWritei( GUCEF_NULL )
    , pcmReadi( GUCEF_NULL )
    , pcmStart( GUCEF_NULL )
    , pcmDelay( GUCEF_NULL )
    , pcmAvailUpdate( GUCEF_NULL )
    , swParamsMalloc( GUCEF_NULL )
    , swParamsFree( GUCEF_NULL )
    , swParamsCurrent( GUCEF_NULL )
    , swParamsSetStartThreshold( GUCEF_NULL )
    , swParamsSetAvailMin( GUCEF_NULL )
    , swParams( GUCEF_NULL )
    , pcmLink( GUCEF_NULL )
    , pcmUnlink( GUCEF_NULL )
    , pcmRecover( GUCEF_NULL )
    , strerror( GUCEF_NULL )
    , deviceNameHint( GUCEF_NULL )
    , deviceNameGetHint( GUCEF_NULL )
    , deviceNameFreeHint( GUCEF_NULL )
    , m_module( GUCEF_NULL )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAlsaApi::~CAlsaApi()
{GUCEF_TRACE;

    Unload();
}

/*-------------------------------------------------------------------------*/

void*
CAlsaApi::GetFunction( const char* name, bool& allFound, bool required ) const
{GUCEF_TRACE;

    CORE::TAnyPointer address = CORE::GetFunctionAddress( m_module, name, 0 );
    if ( GUCEF_NULL == address.funcPtr )
    {
        if ( required )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "AlsaApi: libasound has no function " << name );
            allFound = false;
        }
        else
        {
            GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "AlsaApi: libasound has no function " << name << ", the feature that needs it is unavailable" );
        }
    }
    return reinterpret_cast< void* >( address.funcPtr );
}

/*-------------------------------------------------------------------------*/

bool
CAlsaApi::Load( void )
{GUCEF_TRACE;

    if ( IsLoaded() )
        return true;

    m_module = CORE::LoadModuleDynamicly( CORE::CString( "libasound.so.2" ) );
    if ( GUCEF_NULL == m_module )
    {
        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "AlsaApi: libasound.so.2 is not available, ALSA cannot be used" );
        return false;
    }

    bool allFound = true;
    pcmOpen = (TSndPcmOpen) GetFunction( "snd_pcm_open", allFound );
    pcmClose = (TSndPcmClose) GetFunction( "snd_pcm_close", allFound );
    hwParamsMalloc = (TSndPcmHwParamsMalloc) GetFunction( "snd_pcm_hw_params_malloc", allFound );
    hwParamsFree = (TSndPcmHwParamsFree) GetFunction( "snd_pcm_hw_params_free", allFound );
    hwParamsAny = (TSndPcmHwParamsAny) GetFunction( "snd_pcm_hw_params_any", allFound );
    hwParamsSetAccess = (TSndPcmHwParamsSetAccess) GetFunction( "snd_pcm_hw_params_set_access", allFound );
    hwParamsTestFormat = (TSndPcmHwParamsTestFormat) GetFunction( "snd_pcm_hw_params_test_format", allFound );
    hwParamsSetFormat = (TSndPcmHwParamsSetFormat) GetFunction( "snd_pcm_hw_params_set_format", allFound );
    hwParamsSetChannelsNear = (TSndPcmHwParamsSetChannelsNear) GetFunction( "snd_pcm_hw_params_set_channels_near", allFound );
    hwParamsSetRateNear = (TSndPcmHwParamsSetRateNear) GetFunction( "snd_pcm_hw_params_set_rate_near", allFound );
    hwParamsSetPeriodSizeNear = (TSndPcmHwParamsSetPeriodSizeNear) GetFunction( "snd_pcm_hw_params_set_period_size_near", allFound );
    hwParamsSetBufferSizeNear = (TSndPcmHwParamsSetBufferSizeNear) GetFunction( "snd_pcm_hw_params_set_buffer_size_near", allFound );
    hwParams = (TSndPcmHwParams) GetFunction( "snd_pcm_hw_params", allFound );
    hwParamsGetPeriodSize = (TSndPcmHwParamsGetPeriodSize) GetFunction( "snd_pcm_hw_params_get_period_size", allFound );
    hwParamsGetBufferSize = (TSndPcmHwParamsGetBufferSize) GetFunction( "snd_pcm_hw_params_get_buffer_size", allFound );
    pcmPrepare = (TSndPcmPrepare) GetFunction( "snd_pcm_prepare", allFound );
    pcmDrop = (TSndPcmDrop) GetFunction( "snd_pcm_drop", allFound );
    pcmWritei = (TSndPcmWritei) GetFunction( "snd_pcm_writei", allFound );
    pcmRecover = (TSndPcmRecover) GetFunction( "snd_pcm_recover", allFound );

    // optional: when missing the feature is unavailable, playback still works
    bool optionalFound = true;
    pcmReadi = (TSndPcmReadi) GetFunction( "snd_pcm_readi", optionalFound, false );
    pcmStart = (TSndPcmStart) GetFunction( "snd_pcm_start", optionalFound, false );
    pcmDelay = (TSndPcmDelay) GetFunction( "snd_pcm_delay", optionalFound, false );
    pcmAvailUpdate = (TSndPcmAvailUpdate) GetFunction( "snd_pcm_avail_update", optionalFound, false );
    swParamsMalloc = (TSndPcmSwParamsMalloc) GetFunction( "snd_pcm_sw_params_malloc", optionalFound, false );
    swParamsFree = (TSndPcmSwParamsFree) GetFunction( "snd_pcm_sw_params_free", optionalFound, false );
    swParamsCurrent = (TSndPcmSwParamsCurrent) GetFunction( "snd_pcm_sw_params_current", optionalFound, false );
    swParamsSetStartThreshold = (TSndPcmSwParamsSetStartThreshold) GetFunction( "snd_pcm_sw_params_set_start_threshold", optionalFound, false );
    swParamsSetAvailMin = (TSndPcmSwParamsSetAvailMin) GetFunction( "snd_pcm_sw_params_set_avail_min", optionalFound, false );
    swParams = (TSndPcmSwParams) GetFunction( "snd_pcm_sw_params", optionalFound, false );
    pcmLink = (TSndPcmLink) GetFunction( "snd_pcm_link", optionalFound, false );
    pcmUnlink = (TSndPcmUnlink) GetFunction( "snd_pcm_unlink", optionalFound, false );
    strerror = (TSndStrerror) GetFunction( "snd_strerror", allFound );
    deviceNameHint = (TSndDeviceNameHint) GetFunction( "snd_device_name_hint", allFound );
    deviceNameGetHint = (TSndDeviceNameGetHint) GetFunction( "snd_device_name_get_hint", allFound );
    deviceNameFreeHint = (TSndDeviceNameFreeHint) GetFunction( "snd_device_name_free_hint", allFound );

    if ( !allFound )
    {
        Unload();
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
CAlsaApi::Unload( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_module )
    {
        CORE::UnloadModuleDynamicly( m_module );
        m_module = GUCEF_NULL;
    }
}

/*-------------------------------------------------------------------------*/

bool
CAlsaApi::IsLoaded( void ) const
{GUCEF_TRACE;

    return GUCEF_NULL != m_module;
}


bool
CAlsaApi::HasCaptureSupport( void ) const
{GUCEF_TRACE;

    return GUCEF_NULL != pcmReadi && GUCEF_NULL != pcmStart;
}


bool
CAlsaApi::HasSoftwareParams( void ) const
{GUCEF_TRACE;

    return GUCEF_NULL != swParamsMalloc && GUCEF_NULL != swParamsFree && GUCEF_NULL != swParamsCurrent &&
           GUCEF_NULL != swParamsSetStartThreshold && GUCEF_NULL != swParamsSetAvailMin && GUCEF_NULL != swParams;
}


bool
CAlsaApi::HasDelayQuery( void ) const
{GUCEF_TRACE;

    return GUCEF_NULL != pcmDelay;
}

/*-------------------------------------------------------------------------*/

int
CAlsaApi::ToAlsaFormat( AUDIO::TSampleType sampleType )
{GUCEF_TRACE;

    // SND_PCM_FORMAT_*: the big endian format is always the little endian one plus one
    const int bigEndian = AUDIO::CPcmStreamSource::IsHostBigEndian() ? 1 : 0;
    switch ( sampleType )
    {
        case AUDIO::SAMPLETYPE_UINT8:   return 1;                   /**< SND_PCM_FORMAT_U8 */
        case AUDIO::SAMPLETYPE_INT16:   return 2 + bigEndian;       /**< SND_PCM_FORMAT_S16_LE / _BE */
        case AUDIO::SAMPLETYPE_INT24:   return 32 + bigEndian;      /**< SND_PCM_FORMAT_S24_3LE / _3BE */
        case AUDIO::SAMPLETYPE_INT32:   return 10 + bigEndian;      /**< SND_PCM_FORMAT_S32_LE / _BE */
        case AUDIO::SAMPLETYPE_FLOAT32: return 14 + bigEndian;      /**< SND_PCM_FORMAT_FLOAT_LE / _BE */
        case AUDIO::SAMPLETYPE_FLOAT64: return 16 + bigEndian;      /**< SND_PCM_FORMAT_FLOAT64_LE / _BE */
        default:                        return -1;
    }
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAlsaApi::ErrorText( int error ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL == strerror )
        return CORE::ToString( error );
    return CORE::CString( strerror( error ) ) + " (" + CORE::ToString( error ) + ")";
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ALSA */
}; /* namespace AUDIODRIVER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
