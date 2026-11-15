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

#ifndef GUCEF_COMPUTE_CCOMPUTEDEVICEPARAMS_H
#define GUCEF_COMPUTE_CCOMPUTEDEVICEPARAMS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CPULSEGENERATOR_H
#include "gucefCORE_CPulseGenerator.h"
#define GUCEF_CORE_CPULSEGENERATOR_H
#endif /* GUCEF_CORE_CPULSEGENERATOR_H ? */

#ifndef GUCEF_CORE_CVALUELIST_H
#include "CValueList.h"
#define GUCEF_CORE_CVALUELIST_H
#endif /* GUCEF_CORE_CVALUELIST_H ? */

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
 *  How a device is created: which device of the backend, how many queues, validation, the default execution policy
 *  of its queues, the pulse generator on whose thread its events reach observers, and options for the backend.
 *
 *  The device is chosen by identifier (CComputeDeviceInfo::GetIdentifier()) when one is set, else by index in the
 *  enumeration of the backend (0 by default). Backend options are name / value pairs the backend documents; the CPU
 *  backend knows "reportNativeLimits" (true: report the limits of the host instead of the portable ones).
 */
class GUCEF_COMPUTE_PUBLIC_CPP CComputeDeviceParams
{
    public:

    CComputeDeviceParams( void );

    /** The device by identifier; empty to choose by index */
    const CString& GetDeviceIdentifier( void ) const;
    void SetDeviceIdentifier( const CString& identifier );

    /** The device by its index in the enumeration of the backend, used when no identifier is set */
    UInt32 GetDeviceIndex( void ) const;
    void SetDeviceIndex( UInt32 index );

    /** The number of queues wanted, at least 1. The device creates as many as it can, up to this number */
    UInt32 GetQueueCount( void ) const;
    void SetQueueCount( UInt32 count );

    /** Checks of signatures and mapped buffers, and the validation layers of backends that have them */
    bool GetValidation( void ) const;
    void SetValidation( bool validation );

    /**
     *  Cooperative completion: the queues get no completion task, their pending work completes in
     *  CComputeGlobal::Update() (or in a wait on the calling thread where the backend can). For a browser main thread,
     *  and to test the cooperative path elsewhere. Off by default
     */
    bool GetCooperativeCompletion( void ) const;
    void SetCooperativeCompletion( bool cooperative );

    /** The execution policy the queues of the device start with */
    TComputeExecutionPolicy GetExecutionPolicy( void ) const;
    void SetExecutionPolicy( TComputeExecutionPolicy policy );

    /** The pulse generator on whose thread DeviceLostEvent reaches observers; null (the default) for the global one */
    const CORE::PulseGeneratorPtr& GetEventPulseGenerator( void ) const;
    void SetEventPulseGenerator( const CORE::PulseGeneratorPtr& pulseGenerator );

    const CORE::CValueList& GetBackendOptions( void ) const;
    CORE::CValueList& GetBackendOptions( void );

    private:

    CString m_deviceIdentifier;
    UInt32 m_deviceIndex;
    UInt32 m_queueCount;
    bool m_validation;
    bool m_cooperativeCompletion;
    TComputeExecutionPolicy m_executionPolicy;
    CORE::PulseGeneratorPtr m_eventPulseGenerator;
    CORE::CValueList m_backendOptions;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace COMPUTE */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMPUTE_CCOMPUTEDEVICEPARAMS_H ? */
