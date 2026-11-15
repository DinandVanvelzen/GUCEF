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

#ifndef GUCEF_COMPUTE_CCOMPUTEDEVICEINFO_H
#define GUCEF_COMPUTE_CCOMPUTEDEVICEINFO_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_COMPUTE_CCOMPUTEDEVICECAPABILITIES_H
#include "gucefCOMPUTE_CComputeDeviceCapabilities.h"
#define GUCEF_COMPUTE_CCOMPUTEDEVICECAPABILITIES_H
#endif /* GUCEF_COMPUTE_CCOMPUTEDEVICECAPABILITIES_H ? */

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
 *  The identity of a device as its backend enumerates it, with its capabilities. Plain data.
 *  GetIdentifier() is stable between runs on the same machine, so settings can name a preferred device with it:
 *  "<backend>:<vendor id>:<device id>:<index>", vendor and device id in hexadecimal.
 */
class GUCEF_COMPUTE_PUBLIC_CPP CComputeDeviceInfo
{
    public:

    CComputeDeviceInfo( void );

    /** The name of the backend that offers the device ("cpu", "vulkan", ...) */
    const CString& GetBackendName( void ) const;
    void SetBackendName( const CString& backendName );

    /** The name of the device for people */
    const CString& GetDeviceName( void ) const;
    void SetDeviceName( const CString& deviceName );

    /** The PCI vendor id, or a backend specific id, 0 when there is none */
    UInt32 GetVendorId( void ) const;
    void SetVendorId( UInt32 vendorId );

    /** The PCI device id, or a backend specific id, 0 when there is none */
    UInt32 GetDeviceId( void ) const;
    void SetDeviceId( UInt32 deviceId );

    /** The position of the device in the enumeration of its backend */
    UInt32 GetDeviceIndex( void ) const;
    void SetDeviceIndex( UInt32 deviceIndex );

    /** The driver version as the driver reports it, for logs */
    const CString& GetDriverVersion( void ) const;
    void SetDriverVersion( const CString& driverVersion );

    TComputeDeviceType GetDeviceType( void ) const;
    void SetDeviceType( TComputeDeviceType deviceType );

    const CComputeDeviceCapabilities& GetCapabilities( void ) const;
    CComputeDeviceCapabilities& GetCapabilities( void );
    void SetCapabilities( const CComputeDeviceCapabilities& capabilities );

    /** "<backend>:<vendor id>:<device id>:<index>" with the ids in hexadecimal */
    CString GetIdentifier( void ) const;

    private:

    CString m_backendName;
    CString m_deviceName;
    UInt32 m_vendorId;
    UInt32 m_deviceId;
    UInt32 m_deviceIndex;
    CString m_driverVersion;
    TComputeDeviceType m_deviceType;
    CComputeDeviceCapabilities m_capabilities;
};

/*-------------------------------------------------------------------------*/

typedef GUCEF::vector< CComputeDeviceInfo > TComputeDeviceInfoVector;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace COMPUTE */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMPUTE_CCOMPUTEDEVICEINFO_H ? */
