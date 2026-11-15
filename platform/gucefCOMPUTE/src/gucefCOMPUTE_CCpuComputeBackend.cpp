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

#ifndef GUCEF_COMPUTE_CCPUCOMPUTEDEVICE_H
#include "gucefCOMPUTE_CCpuComputeDevice.h"
#define GUCEF_COMPUTE_CCPUCOMPUTEDEVICE_H
#endif /* GUCEF_COMPUTE_CCPUCOMPUTEDEVICE_H ? */

#include "gucefCOMPUTE_CCpuComputeBackend.h"

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

const CString CCpuComputeBackend::BackendName = "cpu";
const Int32 CCpuComputeBackend::BackendPriority = -1000;

/*-------------------------------------------------------------------------*/

CCpuComputeBackend::CCpuComputeBackend( void )
    : CComputeBackend()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CCpuComputeBackend::~CCpuComputeBackend()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CString
CCpuComputeBackend::GetName( void ) const
{GUCEF_TRACE;

    return BackendName;
}

/*-------------------------------------------------------------------------*/

CString
CCpuComputeBackend::GetDescription( void ) const
{GUCEF_TRACE;

    return "Built-in CPU multi-core backend";
}

/*-------------------------------------------------------------------------*/

Int32
CCpuComputeBackend::GetPriority( void ) const
{GUCEF_TRACE;

    return BackendPriority;
}

/*-------------------------------------------------------------------------*/

bool
CCpuComputeBackend::Initialize( void )
{GUCEF_TRACE;

    return true;
}

/*-------------------------------------------------------------------------*/

void
CCpuComputeBackend::Shutdown( void )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CCpuComputeBackend::EnumerateDevices( TComputeDeviceInfoVector& devices )
{GUCEF_TRACE;

    devices.clear();
    devices.push_back( CCpuComputeDevice::GetHostDeviceInfo( false ) );
    return true;
}

/*-------------------------------------------------------------------------*/

TComputeResult
CCpuComputeBackend::CreateDevice( const CComputeDeviceParams& params, TComputeDevicePtr& device )
{GUCEF_TRACE;

    const bool nativeLimits = params.GetBackendOptions().GetValueAlways( "reportNativeLimits" ).AsBool( false, true );
    CComputeDeviceInfo info = CCpuComputeDevice::GetHostDeviceInfo( nativeLimits );
    if ( !params.GetDeviceIdentifier().IsNULLOrEmpty() )
    {
        if ( !params.GetDeviceIdentifier().Equals( info.GetIdentifier(), false ) )
            return COMPUTERESULT_INVALID_ARGUMENT;
    }
    else
    if ( 0 != params.GetDeviceIndex() )
    {
        return COMPUTERESULT_INVALID_ARGUMENT;
    }
    device = CCpuComputeDevice::Create( info, params );
    return device.IsNULL() ? COMPUTERESULT_OUT_OF_HOST_MEMORY : COMPUTERESULT_SUCCESS;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace COMPUTE */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
