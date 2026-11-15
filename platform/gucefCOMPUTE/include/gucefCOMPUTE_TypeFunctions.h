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

#ifndef GUCEF_COMPUTE_TYPEFUNCTIONS_H
#define GUCEF_COMPUTE_TYPEFUNCTIONS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_COMPUTE_MACROS_H
#include "gucefCOMPUTE_macros.h"
#define GUCEF_COMPUTE_MACROS_H
#endif /* GUCEF_COMPUTE_MACROS_H ? */

#ifndef GUCEF_CORE_CFUTURERESULT_H
#include "gucefCORE_CFutureResult.h"
#define GUCEF_CORE_CFUTURERESULT_H
#endif /* GUCEF_CORE_CFUTURERESULT_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COMPUTE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The name of a result, "COMPUTERESULT_SUCCESS" and so on; "COMPUTERESULT_UNKNOWN" for a value that is not a result */
GUCEF_COMPUTE_PUBLIC_CPP const char*
ComputeResultToString( TComputeResult result );

/** The reverse of ComputeResultToString(). False, and 'result' as it was, for text that is not a result name */
GUCEF_COMPUTE_PUBLIC_CPP bool
StringToComputeResult( const CString& text, TComputeResult& result );

/**
 *  The result that an asynchronous compute operation ended with. A future that finished is COMPUTERESULT_SUCCESS, a
 *  future that failed carries the result name at the start of its extra info (see ComputeResultToFutureInfo()), a
 *  future that is still pending gives COMPUTERESULT_TIMEOUT and any other end state COMPUTERESULT_BACKEND_ERROR.
 *  The text after the result name, the error text of the backend, is given back in 'errorText' when asked.
 */
GUCEF_COMPUTE_PUBLIC_CPP TComputeResult
ComputeResultFromFuture( const CORE::CFutureResult& future, CString* errorText = GUCEF_NULL );

/** The extra info with which a failed compute future is ended: the result name, ": " and the error text when there is one */
GUCEF_COMPUTE_PUBLIC_CPP CString
ComputeResultToFutureInfo( TComputeResult result, const CString& errorText );

/** "unknown", "cpu", "integrated", "discrete", "accelerator" or "virtual" */
GUCEF_COMPUTE_PUBLIC_CPP const char*
ComputeDeviceTypeToString( TComputeDeviceType deviceType );

/** "unknown", "cpu", "spirv", "ptx-fatbin", "msl", "metallib", "wgsl" or "opencl-c" */
GUCEF_COMPUTE_PUBLIC_CPP const char*
ComputeArtefactFormatToString( TComputeArtefactFormat format );

/** The reverse of ComputeArtefactFormatToString() without regard for case; ARTEFACTFORMAT_UNKNOWN for other text */
GUCEF_COMPUTE_PUBLIC_CPP TComputeArtefactFormat
StringToComputeArtefactFormat( const CString& text );

/** The file extension of an artefact of the format in a kernel override root: "spv", "fatbin", "msl", "metallib", "wgsl", "cl"; empty for the CPU and unknown formats */
GUCEF_COMPUTE_PUBLIC_CPP const char*
ComputeArtefactFormatToFileExtension( TComputeArtefactFormat format );

/** "scalar", "simd", "multicore", "accelerator", "none" for PROVIDERTIER_NONE */
GUCEF_COMPUTE_PUBLIC_CPP const char*
ComputeProviderTierToString( TComputeProviderTier tier );

/** The reverse of ComputeProviderTierToString() without regard for case. False, and 'tier' as it was, for other text */
GUCEF_COMPUTE_PUBLIC_CPP bool
StringToComputeProviderTier( const CString& text, TComputeProviderTier& tier );

/** The size of one element of the type in bytes; 0 for ELEMENTTYPE_UNKNOWN */
GUCEF_COMPUTE_PUBLIC_CPP UInt32
ComputeElementTypeSize( TComputeElementType elementType );

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace COMPUTE */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMPUTE_TYPEFUNCTIONS_H ? */
