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

#ifndef GUCEF_COMPUTE_ETYPES_H
#define GUCEF_COMPUTE_ETYPES_H

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

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COMPUTE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

typedef CORE::UInt8 UInt8;
typedef CORE::Int8 Int8;
typedef CORE::UInt16 UInt16;
typedef CORE::Int16 Int16;
typedef CORE::UInt32 UInt32;
typedef CORE::Int32 Int32;
typedef CORE::UInt64 UInt64;
typedef CORE::Int64 Int64;
typedef CORE::Float32 Float32;
typedef CORE::Float64 Float64;

/*-------------------------------------------------------------------------*/

/** Result of a compute operation. The values are stable, new results are only ever appended */
enum EComputeResult
{
    COMPUTERESULT_SUCCESS = 0          ,
    COMPUTERESULT_OUT_OF_DEVICE_MEMORY ,
    COMPUTERESULT_OUT_OF_HOST_MEMORY   ,
    COMPUTERESULT_DEVICE_LOST          ,
    COMPUTERESULT_UNSUPPORTED          ,
    COMPUTERESULT_INVALID_ARGUMENT     ,
    COMPUTERESULT_TIMEOUT              ,
    COMPUTERESULT_COMPILE_FAILED       ,
    COMPUTERESULT_KERNEL_NOT_FOUND     ,
    COMPUTERESULT_BACKEND_ERROR
};
typedef enum EComputeResult TComputeResult;

/** The kind of device that a backend offers */
enum EComputeDeviceType
{
    DEVICETYPE_UNKNOWN = 0 ,
    DEVICETYPE_CPU         ,
    DEVICETYPE_INTEGRATED_GPU ,
    DEVICETYPE_DISCRETE_GPU   ,
    DEVICETYPE_ACCELERATOR
};
typedef enum EComputeDeviceType TComputeDeviceType;

/** Where the memory of a buffer lives */
enum EComputeMemoryLocation
{
    MEMORYLOCATION_DEVICE = 0 ,     /**< fastest for kernels, not mappable */
    MEMORYLOCATION_UPLOAD     ,     /**< host visible, write-combined, for staging uploads */
    MEMORYLOCATION_READBACK   ,     /**< host visible and cached, for staging readback */
    MEMORYLOCATION_UNIFIED          /**< used by kernels and the CPU without copies, only where the device reports it */
};
typedef enum EComputeMemoryLocation TComputeMemoryLocation;

/** What a buffer is used for, a bit mask of the BUFFERUSAGE_ values */
enum EComputeBufferUsage
{
    BUFFERUSAGE_STORAGE   = 0x01 ,
    BUFFERUSAGE_CONSTANTS = 0x02 ,
    BUFFERUSAGE_COPY_SRC  = 0x04 ,
    BUFFERUSAGE_COPY_DST  = 0x08
};
typedef UInt32 TComputeBufferUsage;

/** How a kernel accesses a bound buffer */
enum EComputeAccess
{
    COMPUTEACCESS_READ = 0 ,
    COMPUTEACCESS_WRITE    ,
    COMPUTEACCESS_READ_WRITE
};
typedef enum EComputeAccess TComputeAccess;

/** How the host maps a buffer */
enum EComputeMapAccess
{
    MAPACCESS_READ = 0 ,
    MAPACCESS_WRITE    ,
    MAPACCESS_READ_WRITE
};
typedef enum EComputeMapAccess TComputeMapAccess;

/** The form in which a kernel is supplied to a backend */
enum EComputeArtefactFormat
{
    ARTEFACTFORMAT_UNKNOWN = 0 ,
    ARTEFACTFORMAT_CPU         ,
    ARTEFACTFORMAT_SPIRV       ,
    ARTEFACTFORMAT_OPENCL_C    ,
    ARTEFACTFORMAT_PTX         ,
    ARTEFACTFORMAT_WGSL        ,
    ARTEFACTFORMAT_METAL_SL
};
typedef enum EComputeArtefactFormat TComputeArtefactFormat;

/** How reproducible the results of a kernel are */
enum EComputeDeterminism
{
    DETERMINISM_EXACT_INTEGER = 0 ,
    DETERMINISM_FLOAT_STRICT      ,
    DETERMINISM_FLOAT_RELAXED
};
typedef enum EComputeDeterminism TComputeDeterminism;

/** The type of the elements of a buffer or a constant */
enum EComputeElementType
{
    ELEMENTTYPE_UNKNOWN = 0 ,
    ELEMENTTYPE_INT8        ,
    ELEMENTTYPE_UINT8       ,
    ELEMENTTYPE_INT16       ,
    ELEMENTTYPE_UINT16      ,
    ELEMENTTYPE_INT32       ,
    ELEMENTTYPE_UINT32      ,
    ELEMENTTYPE_INT64       ,
    ELEMENTTYPE_UINT64      ,
    ELEMENTTYPE_FLOAT16     ,
    ELEMENTTYPE_FLOAT32     ,
    ELEMENTTYPE_FLOAT64
};
typedef enum EComputeElementType TComputeElementType;

/** Where the work of a synchronous call may run */
enum EComputeExecutionPolicy
{
    EXECUTIONPOLICY_DEFAULT = 0 ,           /**< fork-join with the calling thread taking part */
    EXECUTIONPOLICY_CALLING_THREAD_ONLY     /**< every chunk runs on the calling thread */
};
typedef enum EComputeExecutionPolicy TComputeExecutionPolicy;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace COMPUTE */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* __cplusplus ? */

#endif /* GUCEF_COMPUTE_ETYPES_H ? */
