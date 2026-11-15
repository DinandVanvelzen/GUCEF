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

#ifndef GUCEF_COMPUTE_CCOMPUTEDEVICECAPABILITIES_H
#define GUCEF_COMPUTE_CCOMPUTEDEVICECAPABILITIES_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_COMPUTE_MACROS_H
#include "gucefCOMPUTE_macros.h"
#define GUCEF_COMPUTE_MACROS_H
#endif /* GUCEF_COMPUTE_MACROS_H ? */

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

/**
 *  What a device can do: optional features (COMPUTEFEATURE_ bits) and limits. Plain data. Backends fill it in when
 *  they enumerate their devices; everybody else only reads it.
 *
 *  The default constructed value has no features and the portable limits of every backend (PortableLimits()):
 *  256 invocations per group, groups of at most 256 x 256 x 64, 16 KiB of workgroup memory, 65535 groups per axis,
 *  8 storage buffers and 128 bytes of constants per kernel, buffers of 256 MiB and bindings of 128 MiB.
 *  A limit of 0 for the memory budget means unknown.
 */
class GUCEF_COMPUTE_PUBLIC_CPP CComputeDeviceCapabilities
{
    public:

    CComputeDeviceCapabilities( void );

    /** The limits that kernels which must run on every backend stay within (section 2.7 of the design), no features */
    static CComputeDeviceCapabilities PortableLimits( void );

    /** True when every feature in the mask is supported; true for an empty mask */
    bool HasFeatures( TComputeFeatures features ) const;
    TComputeFeatures GetFeatures( void ) const;
    void SetFeatures( TComputeFeatures features );
    /** Adds or removes the features of the mask */
    void SetFeature( TComputeFeatures features, bool supported );

    UInt32 GetMaxInvocationsPerGroup( void ) const;
    void SetMaxInvocationsPerGroup( UInt32 invocations );

    /** The largest group size per axis (0, 1, 2); 0 for an axis above 2 */
    UInt32 GetMaxGroupSize( UInt32 axis ) const;
    void SetMaxGroupSize( UInt32 x, UInt32 y, UInt32 z );

    UInt32 GetMaxWorkgroupMemoryInBytes( void ) const;
    void SetMaxWorkgroupMemoryInBytes( UInt32 bytes );

    /** The most groups of one dispatch per axis (0, 1, 2); 0 for an axis above 2. Backends split dispatches that need more */
    UInt32 GetMaxGroupsPerDispatch( UInt32 axis ) const;
    void SetMaxGroupsPerDispatch( UInt32 x, UInt32 y, UInt32 z );

    UInt32 GetMaxStorageBuffersPerKernel( void ) const;
    void SetMaxStorageBuffersPerKernel( UInt32 count );

    UInt32 GetMaxConstantsSizeInBytes( void ) const;
    void SetMaxConstantsSizeInBytes( UInt32 bytes );

    UInt64 GetMaxBufferSizeInBytes( void ) const;
    void SetMaxBufferSizeInBytes( UInt64 bytes );

    UInt64 GetMaxBindingSizeInBytes( void ) const;
    void SetMaxBindingSizeInBytes( UInt64 bytes );

    /** The smallest and largest subgroup size; both 0 without COMPUTEFEATURE_SUBGROUPS */
    UInt32 GetMinSubgroupSize( void ) const;
    UInt32 GetMaxSubgroupSize( void ) const;
    void SetSubgroupSizeRange( UInt32 minSize, UInt32 maxSize );

    /** The number of queues the device offers, at least 1 */
    UInt32 GetQueueCount( void ) const;
    void SetQueueCount( UInt32 count );

    /** The memory the device makes available in bytes, 0 when unknown */
    UInt64 GetMemoryBudgetInBytes( void ) const;
    void SetMemoryBudgetInBytes( UInt64 bytes );

    private:

    TComputeFeatures m_features;
    UInt32 m_maxInvocationsPerGroup;
    UInt32 m_maxGroupSize[ 3 ];
    UInt32 m_maxWorkgroupMemory;
    UInt32 m_maxGroupsPerDispatch[ 3 ];
    UInt32 m_maxStorageBuffers;
    UInt32 m_maxConstantsSize;
    UInt64 m_maxBufferSize;
    UInt64 m_maxBindingSize;
    UInt32 m_minSubgroupSize;
    UInt32 m_maxSubgroupSize;
    UInt32 m_queueCount;
    UInt64 m_memoryBudget;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace COMPUTE */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMPUTE_CCOMPUTEDEVICECAPABILITIES_H ? */
