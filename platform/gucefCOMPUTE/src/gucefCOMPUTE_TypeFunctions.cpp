/*
 *  gucefCOMPUTE: GUCEF module providing compute: devices, buffers and kernels over pluggable backends
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

#ifndef GUCEF_CORE_CTASK_H
#include "gucefCORE_CTask.h"
#define GUCEF_CORE_CTASK_H
#endif /* GUCEF_CORE_CTASK_H ? */

#include "gucefCOMPUTE_TypeFunctions.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COMPUTE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

const char* const g_resultNames[] =
{
    "COMPUTERESULT_SUCCESS"              ,
    "COMPUTERESULT_OUT_OF_DEVICE_MEMORY" ,
    "COMPUTERESULT_OUT_OF_HOST_MEMORY"   ,
    "COMPUTERESULT_DEVICE_LOST"          ,
    "COMPUTERESULT_UNSUPPORTED"          ,
    "COMPUTERESULT_INVALID_ARGUMENT"     ,
    "COMPUTERESULT_TIMEOUT"              ,
    "COMPUTERESULT_COMPILE_FAILED"       ,
    "COMPUTERESULT_KERNEL_NOT_FOUND"     ,
    "COMPUTERESULT_BACKEND_ERROR"
};
const UInt32 g_resultNameCount = sizeof( g_resultNames ) / sizeof( g_resultNames[ 0 ] );

const char* const g_artefactFormatNames[] = { "unknown", "cpu", "spirv", "ptx-fatbin", "msl", "metallib", "wgsl", "opencl-c" };
const char* const g_artefactFormatExtensions[] = { "", "", "spv", "fatbin", "msl", "metallib", "wgsl", "cl" };
const UInt32 g_artefactFormatCount = sizeof( g_artefactFormatNames ) / sizeof( g_artefactFormatNames[ 0 ] );

const char* const g_tierNames[] = { "scalar", "simd", "multicore", "accelerator" };

}

/*-------------------------------------------------------------------------*/

const char*
ComputeResultToString( TComputeResult result )
{GUCEF_TRACE;

    const UInt32 index = static_cast< UInt32 >( result );
    if ( index < g_resultNameCount )
        return g_resultNames[ index ];
    return "COMPUTERESULT_UNKNOWN";
}

/*-------------------------------------------------------------------------*/

bool
StringToComputeResult( const CString& text, TComputeResult& result )
{GUCEF_TRACE;

    for ( UInt32 i=0; i<g_resultNameCount; ++i )
    {
        if ( text == g_resultNames[ i ] )
        {
            result = static_cast< TComputeResult >( i );
            return true;
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

CString
ComputeResultToFutureInfo( TComputeResult result, const CString& errorText )
{GUCEF_TRACE;

    CString info( ComputeResultToString( result ) );
    if ( !errorText.IsNULLOrEmpty() )
        info += ": " + errorText;
    return info;
}

/*-------------------------------------------------------------------------*/

TComputeResult
ComputeResultFromFuture( const CORE::CFutureResult& future, CString* errorText )
{GUCEF_TRACE;

    if ( GUCEF_NULL != errorText )
        errorText->Clear();
    if ( !future.IsReady() )
        return COMPUTERESULT_TIMEOUT;

    CORE::CTaskPtr task = future.GetResult( 0 );
    if ( task.IsNULL() )
        return COMPUTERESULT_BACKEND_ERROR;

    const CORE::TTaskStatus status = task->GetTaskStatus();
    if ( CORE::TASKSTATUS_FINISHED == status )
        return COMPUTERESULT_SUCCESS;

    const CString info = task->GetTaskStatusExtraInfo();
    const CString name = info.SubstrToChar( ':', true );
    TComputeResult result = COMPUTERESULT_BACKEND_ERROR;
    if ( !StringToComputeResult( name.Trim( true ).Trim( false ), result ) )
    {
        if ( GUCEF_NULL != errorText )
            *errorText = info;
        return COMPUTERESULT_BACKEND_ERROR;
    }
    if ( GUCEF_NULL != errorText && info.Length() > name.Length() + 1 )
        *errorText = info.CutChars( name.Length() + 1, true, 0 ).Trim( true );
    return result;
}

/*-------------------------------------------------------------------------*/

const char*
ComputeDeviceTypeToString( TComputeDeviceType deviceType )
{GUCEF_TRACE;

    switch ( deviceType )
    {
        case DEVICETYPE_CPU: return "cpu";
        case DEVICETYPE_INTEGRATED_GPU: return "integrated";
        case DEVICETYPE_DISCRETE_GPU: return "discrete";
        case DEVICETYPE_ACCELERATOR: return "accelerator";
        case DEVICETYPE_VIRTUAL_GPU: return "virtual";
        case DEVICETYPE_UNKNOWN:
        default: return "unknown";
    }
}

/*-------------------------------------------------------------------------*/

const char*
ComputeArtefactFormatToString( TComputeArtefactFormat format )
{GUCEF_TRACE;

    const UInt32 index = static_cast< UInt32 >( format );
    if ( index < g_artefactFormatCount )
        return g_artefactFormatNames[ index ];
    return g_artefactFormatNames[ 0 ];
}

/*-------------------------------------------------------------------------*/

TComputeArtefactFormat
StringToComputeArtefactFormat( const CString& text )
{GUCEF_TRACE;

    const CString lowered = text.Lowercase();
    for ( UInt32 i=0; i<g_artefactFormatCount; ++i )
    {
        if ( lowered == g_artefactFormatNames[ i ] )
            return static_cast< TComputeArtefactFormat >( i );
    }
    return ARTEFACTFORMAT_UNKNOWN;
}

/*-------------------------------------------------------------------------*/

const char*
ComputeArtefactFormatToFileExtension( TComputeArtefactFormat format )
{GUCEF_TRACE;

    const UInt32 index = static_cast< UInt32 >( format );
    if ( index < g_artefactFormatCount )
        return g_artefactFormatExtensions[ index ];
    return "";
}

/*-------------------------------------------------------------------------*/

const char*
ComputeProviderTierToString( TComputeProviderTier tier )
{GUCEF_TRACE;

    const UInt32 index = static_cast< UInt32 >( tier );
    if ( index < static_cast< UInt32 >( PROVIDERTIER_COUNT ) )
        return g_tierNames[ index ];
    return "none";
}

/*-------------------------------------------------------------------------*/

bool
StringToComputeProviderTier( const CString& text, TComputeProviderTier& tier )
{GUCEF_TRACE;

    const CString lowered = text.Lowercase();
    if ( lowered == "none" )
    {
        tier = PROVIDERTIER_NONE;
        return true;
    }
    for ( UInt32 i=0; i<static_cast< UInt32 >( PROVIDERTIER_COUNT ); ++i )
    {
        if ( lowered == g_tierNames[ i ] )
        {
            tier = static_cast< TComputeProviderTier >( i );
            return true;
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

UInt32
ComputeElementTypeSize( TComputeElementType elementType )
{GUCEF_TRACE;

    switch ( elementType )
    {
        case ELEMENTTYPE_INT8:
        case ELEMENTTYPE_UINT8: return 1;
        case ELEMENTTYPE_INT16:
        case ELEMENTTYPE_UINT16:
        case ELEMENTTYPE_FLOAT16: return 2;
        case ELEMENTTYPE_INT32:
        case ELEMENTTYPE_UINT32:
        case ELEMENTTYPE_FLOAT32: return 4;
        case ELEMENTTYPE_INT64:
        case ELEMENTTYPE_UINT64:
        case ELEMENTTYPE_FLOAT64: return 8;
        case ELEMENTTYPE_UNKNOWN:
        default: return 0;
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace COMPUTE */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
