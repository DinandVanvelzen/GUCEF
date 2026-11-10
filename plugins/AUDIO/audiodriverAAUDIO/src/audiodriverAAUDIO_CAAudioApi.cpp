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

#include "audiodriverAAUDIO_CAAudioApi.h"

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
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::Int32 CAAudioApi::Ok;
const CORE::Int32 CAAudioApi::ErrorDisconnected;
const CORE::Int32 CAAudioApi::ErrorTimeout;
const CORE::Int32 CAAudioApi::Unspecified;
const CORE::Int32 CAAudioApi::DirectionOutput;
const CORE::Int32 CAAudioApi::FormatPcmI16;
const CORE::Int32 CAAudioApi::FormatPcmFloat;
const CORE::Int32 CAAudioApi::FormatPcmI24Packed;
const CORE::Int32 CAAudioApi::FormatPcmI32;
const CORE::Int32 CAAudioApi::SharingModeExclusive;
const CORE::Int32 CAAudioApi::SharingModeShared;
const CORE::Int32 CAAudioApi::PerformanceModeNone;
const CORE::Int32 CAAudioApi::PerformanceModeLowLatency;
const CORE::Int32 CAAudioApi::UsageGame;
const CORE::Int32 CAAudioApi::ContentTypeMusic;

/*-------------------------------------------------------------------------*/

CAAudioApi::CAAudioApi( void )
    : createStreamBuilder( GUCEF_NULL )
    , builderSetDeviceId( GUCEF_NULL )
    , builderSetDirection( GUCEF_NULL )
    , builderSetSharingMode( GUCEF_NULL )
    , builderSetSampleRate( GUCEF_NULL )
    , builderSetChannelCount( GUCEF_NULL )
    , builderSetFormat( GUCEF_NULL )
    , builderSetBufferCapacityInFrames( GUCEF_NULL )
    , builderSetPerformanceMode( GUCEF_NULL )
    , builderSetUsage( GUCEF_NULL )
    , builderSetContentType( GUCEF_NULL )
    , builderOpenStream( GUCEF_NULL )
    , builderDelete( GUCEF_NULL )
    , streamRequestStart( GUCEF_NULL )
    , streamRequestStop( GUCEF_NULL )
    , streamClose( GUCEF_NULL )
    , streamWrite( GUCEF_NULL )
    , streamGetSampleRate( GUCEF_NULL )
    , streamGetChannelCount( GUCEF_NULL )
    , streamGetFormat( GUCEF_NULL )
    , streamGetFramesPerBurst( GUCEF_NULL )
    , streamGetBufferCapacityInFrames( GUCEF_NULL )
    , streamGetBufferSizeInFrames( GUCEF_NULL )
    , streamSetBufferSizeInFrames( GUCEF_NULL )
    , streamGetXRunCount( GUCEF_NULL )
    , convertResultToText( GUCEF_NULL )
    , m_module( GUCEF_NULL )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAAudioApi::~CAAudioApi()
{GUCEF_TRACE;

    Unload();
}

/*-------------------------------------------------------------------------*/

void*
CAAudioApi::GetFunction( const char* name, bool& allFound, bool optional ) const
{GUCEF_TRACE;

    CORE::TAnyPointer address = CORE::GetFunctionAddress( m_module, name, 0 );
    if ( GUCEF_NULL == address.funcPtr && !optional )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "AAudioApi: libaaudio has no function " << name );
        allFound = false;
    }
    return reinterpret_cast< void* >( address.funcPtr );
}

/*-------------------------------------------------------------------------*/

bool
CAAudioApi::Load( void )
{GUCEF_TRACE;

    if ( IsLoaded() )
        return true;

    m_module = CORE::LoadModuleDynamicly( CORE::CString( "libaaudio.so" ) );
    if ( GUCEF_NULL == m_module )
    {
        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "AAudioApi: libaaudio.so is not available (Android 8 or newer is needed), AAudio cannot be used" );
        return false;
    }

    bool allFound = true;
    createStreamBuilder = (TAAudioCreateStreamBuilder) GetFunction( "AAudio_createStreamBuilder", allFound );
    builderSetDeviceId = (TAAudioBuilderSetInt) GetFunction( "AAudioStreamBuilder_setDeviceId", allFound );
    builderSetDirection = (TAAudioBuilderSetInt) GetFunction( "AAudioStreamBuilder_setDirection", allFound );
    builderSetSharingMode = (TAAudioBuilderSetInt) GetFunction( "AAudioStreamBuilder_setSharingMode", allFound );
    builderSetSampleRate = (TAAudioBuilderSetInt) GetFunction( "AAudioStreamBuilder_setSampleRate", allFound );
    builderSetChannelCount = (TAAudioBuilderSetInt) GetFunction( "AAudioStreamBuilder_setChannelCount", allFound );
    builderSetFormat = (TAAudioBuilderSetInt) GetFunction( "AAudioStreamBuilder_setFormat", allFound );
    builderSetBufferCapacityInFrames = (TAAudioBuilderSetInt) GetFunction( "AAudioStreamBuilder_setBufferCapacityInFrames", allFound );
    builderSetPerformanceMode = (TAAudioBuilderSetInt) GetFunction( "AAudioStreamBuilder_setPerformanceMode", allFound );
    builderSetUsage = (TAAudioBuilderSetInt) GetFunction( "AAudioStreamBuilder_setUsage", allFound, true );
    builderSetContentType = (TAAudioBuilderSetInt) GetFunction( "AAudioStreamBuilder_setContentType", allFound, true );
    builderOpenStream = (TAAudioBuilderOpenStream) GetFunction( "AAudioStreamBuilder_openStream", allFound );
    builderDelete = (TAAudioBuilderDelete) GetFunction( "AAudioStreamBuilder_delete", allFound );
    streamRequestStart = (TAAudioStreamAction) GetFunction( "AAudioStream_requestStart", allFound );
    streamRequestStop = (TAAudioStreamAction) GetFunction( "AAudioStream_requestStop", allFound );
    streamClose = (TAAudioStreamAction) GetFunction( "AAudioStream_close", allFound );
    streamWrite = (TAAudioStreamWrite) GetFunction( "AAudioStream_write", allFound );
    streamGetSampleRate = (TAAudioStreamGetInt) GetFunction( "AAudioStream_getSampleRate", allFound );
    streamGetChannelCount = (TAAudioStreamGetInt) GetFunction( "AAudioStream_getChannelCount", allFound );
    streamGetFormat = (TAAudioStreamGetInt) GetFunction( "AAudioStream_getFormat", allFound );
    streamGetFramesPerBurst = (TAAudioStreamGetInt) GetFunction( "AAudioStream_getFramesPerBurst", allFound );
    streamGetBufferCapacityInFrames = (TAAudioStreamGetInt) GetFunction( "AAudioStream_getBufferCapacityInFrames", allFound );
    streamGetBufferSizeInFrames = (TAAudioStreamGetInt) GetFunction( "AAudioStream_getBufferSizeInFrames", allFound );
    streamSetBufferSizeInFrames = (TAAudioStreamSetBufferSize) GetFunction( "AAudioStream_setBufferSizeInFrames", allFound );
    streamGetXRunCount = (TAAudioStreamGetInt) GetFunction( "AAudioStream_getXRunCount", allFound );
    convertResultToText = (TAAudioConvertResultToText) GetFunction( "AAudio_convertResultToText", allFound );

    if ( !allFound )
    {
        Unload();
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
CAAudioApi::Unload( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_module )
    {
        CORE::UnloadModuleDynamicly( m_module );
        m_module = GUCEF_NULL;
    }
}

/*-------------------------------------------------------------------------*/

bool
CAAudioApi::IsLoaded( void ) const
{GUCEF_TRACE;

    return GUCEF_NULL != m_module;
}

/*-------------------------------------------------------------------------*/

CORE::Int32
CAAudioApi::ToAAudioFormat( AUDIO::TSampleType sampleType )
{GUCEF_TRACE;

    // AAudio samples are in the byte order of the device
    switch ( sampleType )
    {
        case AUDIO::SAMPLETYPE_INT16:   return FormatPcmI16;
        case AUDIO::SAMPLETYPE_INT24:   return FormatPcmI24Packed;
        case AUDIO::SAMPLETYPE_INT32:   return FormatPcmI32;
        case AUDIO::SAMPLETYPE_FLOAT32: return FormatPcmFloat;
        default:                        return Unspecified;
    }
}

/*-------------------------------------------------------------------------*/

AUDIO::TSampleType
CAAudioApi::FromAAudioFormat( CORE::Int32 format )
{GUCEF_TRACE;

    switch ( format )
    {
        case FormatPcmI16:          return AUDIO::SAMPLETYPE_INT16;
        case FormatPcmI24Packed:    return AUDIO::SAMPLETYPE_INT24;
        case FormatPcmI32:          return AUDIO::SAMPLETYPE_INT32;
        case FormatPcmFloat:        return AUDIO::SAMPLETYPE_FLOAT32;
        default:                    return AUDIO::SAMPLETYPE_UNKNOWN;
    }
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAAudioApi::ResultText( aaudio_result_t result ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL == convertResultToText )
        return CORE::ToString( result );
    return CORE::CString( convertResultToText( result ) ) + " (" + CORE::ToString( result ) + ")";
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AAUDIO */
}; /* namespace AUDIODRIVER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
