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

#include "gucefCOMPUTE_CComputeDeviceParams.h"

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

CComputeDeviceParams::CComputeDeviceParams( void )
    : m_deviceIdentifier()
    , m_deviceIndex( 0 )
    , m_queueCount( 1 )
    , m_validation( false )
    , m_cooperativeCompletion( false )
    , m_executionPolicy( EXECUTIONPOLICY_DEFAULT )
    , m_eventPulseGenerator()
    , m_backendOptions()
{GUCEF_TRACE;

    #ifdef GUCEF_COMPUTE_DEBUG_MODE
    m_validation = true;
    #endif
}

/*-------------------------------------------------------------------------*/

const CString&
CComputeDeviceParams::GetDeviceIdentifier( void ) const
{GUCEF_TRACE;

    return m_deviceIdentifier;
}

/*-------------------------------------------------------------------------*/

void
CComputeDeviceParams::SetDeviceIdentifier( const CString& identifier )
{GUCEF_TRACE;

    m_deviceIdentifier = identifier;
}

/*-------------------------------------------------------------------------*/

UInt32
CComputeDeviceParams::GetDeviceIndex( void ) const
{GUCEF_TRACE;

    return m_deviceIndex;
}

/*-------------------------------------------------------------------------*/

void
CComputeDeviceParams::SetDeviceIndex( UInt32 index )
{GUCEF_TRACE;

    m_deviceIndex = index;
}

/*-------------------------------------------------------------------------*/

UInt32
CComputeDeviceParams::GetQueueCount( void ) const
{GUCEF_TRACE;

    return m_queueCount;
}

/*-------------------------------------------------------------------------*/

void
CComputeDeviceParams::SetQueueCount( UInt32 count )
{GUCEF_TRACE;

    m_queueCount = count > 0 ? count : 1;
}

/*-------------------------------------------------------------------------*/

bool
CComputeDeviceParams::GetValidation( void ) const
{GUCEF_TRACE;

    return m_validation;
}

/*-------------------------------------------------------------------------*/

void
CComputeDeviceParams::SetValidation( bool validation )
{GUCEF_TRACE;

    m_validation = validation;
}

/*-------------------------------------------------------------------------*/

bool
CComputeDeviceParams::GetCooperativeCompletion( void ) const
{GUCEF_TRACE;

    return m_cooperativeCompletion;
}

/*-------------------------------------------------------------------------*/

void
CComputeDeviceParams::SetCooperativeCompletion( bool cooperative )
{GUCEF_TRACE;

    m_cooperativeCompletion = cooperative;
}

/*-------------------------------------------------------------------------*/

TComputeExecutionPolicy
CComputeDeviceParams::GetExecutionPolicy( void ) const
{GUCEF_TRACE;

    return m_executionPolicy;
}

/*-------------------------------------------------------------------------*/

void
CComputeDeviceParams::SetExecutionPolicy( TComputeExecutionPolicy policy )
{GUCEF_TRACE;

    m_executionPolicy = policy;
}

/*-------------------------------------------------------------------------*/

const CORE::PulseGeneratorPtr&
CComputeDeviceParams::GetEventPulseGenerator( void ) const
{GUCEF_TRACE;

    return m_eventPulseGenerator;
}

/*-------------------------------------------------------------------------*/

void
CComputeDeviceParams::SetEventPulseGenerator( const CORE::PulseGeneratorPtr& pulseGenerator )
{GUCEF_TRACE;

    m_eventPulseGenerator = pulseGenerator;
}

/*-------------------------------------------------------------------------*/

const CORE::CValueList&
CComputeDeviceParams::GetBackendOptions( void ) const
{GUCEF_TRACE;

    return m_backendOptions;
}

/*-------------------------------------------------------------------------*/

CORE::CValueList&
CComputeDeviceParams::GetBackendOptions( void )
{GUCEF_TRACE;

    return m_backendOptions;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace COMPUTE */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
