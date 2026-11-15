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

#ifndef GUCEF_COMPUTE_CCOMPUTEDEVICEREQUIREMENTS_H
#define GUCEF_COMPUTE_CCOMPUTEDEVICEREQUIREMENTS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_COMPUTE_CCOMPUTEDEVICEINFO_H
#include "gucefCOMPUTE_CComputeDeviceInfo.h"
#define GUCEF_COMPUTE_CCOMPUTEDEVICEINFO_H
#endif /* GUCEF_COMPUTE_CCOMPUTEDEVICEINFO_H ? */

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
 *  What a caller needs from a device: required features, minimum limits, the backends and device types that are
 *  acceptable, and whether the CPU device is. The default value accepts every device, the CPU device included,
 *  so CComputeBackendRegistry::CreateBestDevice() with default requirements always succeeds.
 */
class GUCEF_COMPUTE_PUBLIC_CPP CComputeDeviceRequirements
{
    public:

    CComputeDeviceRequirements( void );

    /** Features every acceptable device has; none by default */
    TComputeFeatures GetRequiredFeatures( void ) const;
    void SetRequiredFeatures( TComputeFeatures features );

    /** The smallest acceptable invocations per group; 0 for any */
    UInt32 GetMinInvocationsPerGroup( void ) const;
    void SetMinInvocationsPerGroup( UInt32 invocations );

    /** The smallest acceptable workgroup memory in bytes; 0 for any */
    UInt32 GetMinWorkgroupMemoryInBytes( void ) const;
    void SetMinWorkgroupMemoryInBytes( UInt32 bytes );

    /** The smallest acceptable number of storage buffers per kernel; 0 for any */
    UInt32 GetMinStorageBuffersPerKernel( void ) const;
    void SetMinStorageBuffersPerKernel( UInt32 count );

    /** The smallest acceptable constants block in bytes; 0 for any */
    UInt32 GetMinConstantsSizeInBytes( void ) const;
    void SetMinConstantsSizeInBytes( UInt32 bytes );

    /** The smallest acceptable maximum buffer size in bytes; 0 for any */
    UInt64 GetMinBufferSizeInBytes( void ) const;
    void SetMinBufferSizeInBytes( UInt64 bytes );

    /** The backends (by name, without regard for case) whose devices are acceptable; empty for every backend */
    const CString::StringSet& GetAllowedBackends( void ) const;
    void SetAllowedBackends( const CString::StringSet& backendNames );
    void AllowBackend( const CString& backendName );

    /** A mask with bit ( 1 << TComputeDeviceType ) set for every acceptable device type; 0 for every type */
    UInt32 GetAllowedDeviceTypes( void ) const;
    void SetAllowedDeviceTypes( UInt32 deviceTypeMask );
    void AllowDeviceType( TComputeDeviceType deviceType );

    /** Whether the CPU device is acceptable; true by default */
    bool IsCpuDeviceAcceptable( void ) const;
    void SetCpuDeviceAcceptable( bool acceptable );

    /** True when the device meets every requirement */
    bool IsMetBy( const CComputeDeviceInfo& device ) const;

    private:

    TComputeFeatures m_requiredFeatures;
    UInt32 m_minInvocationsPerGroup;
    UInt32 m_minWorkgroupMemory;
    UInt32 m_minStorageBuffers;
    UInt32 m_minConstantsSize;
    UInt64 m_minBufferSize;
    CString::StringSet m_allowedBackends;
    UInt32 m_allowedDeviceTypes;
    bool m_cpuDeviceAcceptable;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace COMPUTE */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMPUTE_CCOMPUTEDEVICEREQUIREMENTS_H ? */
