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

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_CORE_CTSHAREDPTR_H
#include "CTSharedPtr.h"
#define GUCEF_CORE_CTSHAREDPTR_H
#endif /* GUCEF_CORE_CTSHAREDPTR_H ? */

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
typedef CORE::CString CString;

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

/** The kind of device that a backend offers. The values are stable, new kinds are only ever appended */
enum EComputeDeviceType
{
    DEVICETYPE_UNKNOWN = 0    ,
    DEVICETYPE_CPU            ,
    DEVICETYPE_INTEGRATED_GPU ,
    DEVICETYPE_DISCRETE_GPU   ,
    DEVICETYPE_ACCELERATOR    ,
    DEVICETYPE_VIRTUAL_GPU          /**< a GPU of a virtual machine or a software implementation of a GPU API */
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
    ARTEFACTFORMAT_UNKNOWN = 0    ,
    ARTEFACTFORMAT_CPU            ,     /**< a CCpuComputeKernel factory, code in the module or plugin that registers it */
    ARTEFACTFORMAT_SPIRV          ,     /**< SPIR-V module, shader flavour (Vulkan) */
    ARTEFACTFORMAT_PTX_FATBIN     ,     /**< PTX and cubins packed as a fatbin (CUDA) */
    ARTEFACTFORMAT_MSL_SOURCE     ,     /**< Metal shading language source */
    ARTEFACTFORMAT_METALLIB       ,     /**< precompiled Metal library */
    ARTEFACTFORMAT_WGSL_SOURCE    ,     /**< WebGPU shading language source */
    ARTEFACTFORMAT_OPENCL_C_SOURCE      /**< OpenCL C source */
};
typedef enum EComputeArtefactFormat TComputeArtefactFormat;

/** How reproducible the results of a kernel are */
enum EComputeDeterminism
{
    DETERMINISM_EXACT_INTEGER = 0 ,     /**< integer operations of defined semantics only, results equal on every backend */
    DETERMINISM_FLOAT_STRICT      ,     /**< compiled without fast math, bounded by a ULP tolerance */
    DETERMINISM_FLOAT_RELAXED           /**< fast math allowed, bounded by absolute and relative tolerances */
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

/** Optional device features, a bit mask of the COMPUTEFEATURE_ values. Never assumed, always asked for */
enum EComputeFeature
{
    COMPUTEFEATURE_FLOAT16                      = 0x0001 ,
    COMPUTEFEATURE_FLOAT64                      = 0x0002 ,
    COMPUTEFEATURE_INT64                        = 0x0004 ,
    COMPUTEFEATURE_INT64_ATOMICS                = 0x0008 ,
    COMPUTEFEATURE_FLOAT_ATOMICS                = 0x0010 ,
    COMPUTEFEATURE_SUBGROUPS                    = 0x0020 ,
    COMPUTEFEATURE_RAY_QUERY                    = 0x0040 ,
    COMPUTEFEATURE_UNIFIED_MEMORY               = 0x0080 ,
    COMPUTEFEATURE_MAPPABLE_DEVICE_LOCAL_MEMORY = 0x0100 ,
    COMPUTEFEATURE_TIMESTAMP_QUERIES            = 0x0200 ,
    COMPUTEFEATURE_FAST_FLOAT64                 = 0x0400 ,     /**< f64 at a rate of at least 1/4 of f32 */
    COMPUTEFEATURE_STRICT_FLOAT                 = 0x0800       /**< fast math can be disabled, denormals are preserved */
};
typedef UInt32 TComputeFeatures;

/**
 *  The tiers of a batch dispatch chain into which bridge plugins register providers. The values equal the tiers of
 *  the gucefMATH batch dispatch table, so a bridge maps them one to one; gucefCOMPUTE itself never depends on gucefMATH.
 */
enum EComputeProviderTier
{
    PROVIDERTIER_SCALAR      = 0 ,
    PROVIDERTIER_SIMD        = 1 ,
    PROVIDERTIER_MULTICORE   = 2 ,
    PROVIDERTIER_ACCELERATOR = 3 ,

    PROVIDERTIER_COUNT       = 4 ,
    PROVIDERTIER_NONE        = 0xFF     /**< no tier: nothing forced */
};
typedef enum EComputeProviderTier TComputeProviderTier;

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The most storage buffers one kernel can bind on every backend (WebGPU default limit) */
#define GUCEF_COMPUTE_MAX_BINDINGS                  8

/** The largest constants block of a kernel on every backend (Vulkan minimum for push constants) */
#define GUCEF_COMPUTE_MAX_CONSTANTS_SIZE            128

/** The first 8 bytes of every constants block hold the base item and the item count, both UInt32 */
#define GUCEF_COMPUTE_CONSTANTS_HEADER_SIZE         8

/** A wait that never times out */
#define GUCEF_COMPUTE_INFINITE_TIMEOUT              0xFFFFFFFF

/** The alignment of the memory of buffers of the CPU backend, enough for every SIMD instruction set in use */
#define GUCEF_COMPUTE_CPU_BUFFER_ALIGNMENT          64

/*-------------------------------------------------------------------------//
//                                                                         //
//      SMART POINTERS                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

class CComputeBackend;
class CComputeDevice;
class CComputeBuffer;
class CComputeKernel;
class CComputeQueue;
class CComputeKernelDecl;
class CComputeKernelArtefact;
class CCpuComputeKernel;

typedef CORE::CTSharedPtr< CComputeBackend, MT::CMutex >          TComputeBackendPtr;
typedef CORE::CTSharedPtr< CComputeDevice, MT::CMutex >           TComputeDevicePtr;
typedef CORE::CTSharedPtr< CComputeBuffer, MT::CMutex >           TComputeBufferPtr;
typedef CORE::CTSharedPtr< CComputeKernel, MT::CMutex >           TComputeKernelPtr;
typedef CORE::CTSharedPtr< CComputeQueue, MT::CMutex >            TComputeQueuePtr;
typedef CORE::CTSharedPtr< CComputeKernelDecl, MT::CMutex >       TComputeKernelDeclPtr;
typedef CORE::CTSharedPtr< CComputeKernelArtefact, MT::CMutex >   TComputeKernelArtefactPtr;

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
