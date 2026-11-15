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

#include <stdio.h>

#include "gucefCOMPUTE_CComputeDeviceInfo.h"

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

CComputeDeviceInfo::CComputeDeviceInfo( void )
    : m_backendName()
    , m_deviceName()
    , m_vendorId( 0 )
    , m_deviceId( 0 )
    , m_deviceIndex( 0 )
    , m_driverVersion()
    , m_deviceType( DEVICETYPE_UNKNOWN )
    , m_capabilities()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

const CString&
CComputeDeviceInfo::GetBackendName( void ) const
{GUCEF_TRACE;

    return m_backendName;
}

/*-------------------------------------------------------------------------*/

void
CComputeDeviceInfo::SetBackendName( const CString& backendName )
{GUCEF_TRACE;

    m_backendName = backendName;
}

/*-------------------------------------------------------------------------*/

const CString&
CComputeDeviceInfo::GetDeviceName( void ) const
{GUCEF_TRACE;

    return m_deviceName;
}

/*-------------------------------------------------------------------------*/

void
CComputeDeviceInfo::SetDeviceName( const CString& deviceName )
{GUCEF_TRACE;

    m_deviceName = deviceName;
}

/*-------------------------------------------------------------------------*/

UInt32
CComputeDeviceInfo::GetVendorId( void ) const
{GUCEF_TRACE;

    return m_vendorId;
}

/*-------------------------------------------------------------------------*/

void
CComputeDeviceInfo::SetVendorId( UInt32 vendorId )
{GUCEF_TRACE;

    m_vendorId = vendorId;
}

/*-------------------------------------------------------------------------*/

UInt32
CComputeDeviceInfo::GetDeviceId( void ) const
{GUCEF_TRACE;

    return m_deviceId;
}

/*-------------------------------------------------------------------------*/

void
CComputeDeviceInfo::SetDeviceId( UInt32 deviceId )
{GUCEF_TRACE;

    m_deviceId = deviceId;
}

/*-------------------------------------------------------------------------*/

UInt32
CComputeDeviceInfo::GetDeviceIndex( void ) const
{GUCEF_TRACE;

    return m_deviceIndex;
}

/*-------------------------------------------------------------------------*/

void
CComputeDeviceInfo::SetDeviceIndex( UInt32 deviceIndex )
{GUCEF_TRACE;

    m_deviceIndex = deviceIndex;
}

/*-------------------------------------------------------------------------*/

const CString&
CComputeDeviceInfo::GetDriverVersion( void ) const
{GUCEF_TRACE;

    return m_driverVersion;
}

/*-------------------------------------------------------------------------*/

void
CComputeDeviceInfo::SetDriverVersion( const CString& driverVersion )
{GUCEF_TRACE;

    m_driverVersion = driverVersion;
}

/*-------------------------------------------------------------------------*/

TComputeDeviceType
CComputeDeviceInfo::GetDeviceType( void ) const
{GUCEF_TRACE;

    return m_deviceType;
}

/*-------------------------------------------------------------------------*/

void
CComputeDeviceInfo::SetDeviceType( TComputeDeviceType deviceType )
{GUCEF_TRACE;

    m_deviceType = deviceType;
}

/*-------------------------------------------------------------------------*/

const CComputeDeviceCapabilities&
CComputeDeviceInfo::GetCapabilities( void ) const
{GUCEF_TRACE;

    return m_capabilities;
}

/*-------------------------------------------------------------------------*/

CComputeDeviceCapabilities&
CComputeDeviceInfo::GetCapabilities( void )
{GUCEF_TRACE;

    return m_capabilities;
}

/*-------------------------------------------------------------------------*/

void
CComputeDeviceInfo::SetCapabilities( const CComputeDeviceCapabilities& capabilities )
{GUCEF_TRACE;

    m_capabilities = capabilities;
}

/*-------------------------------------------------------------------------*/

CString
CComputeDeviceInfo::GetIdentifier( void ) const
{GUCEF_TRACE;

    char ids[ 64 ];
    sprintf( ids, ":%x:%x:%u", (unsigned int) m_vendorId, (unsigned int) m_deviceId, (unsigned int) m_deviceIndex );
    return m_backendName + ids;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace COMPUTE */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
