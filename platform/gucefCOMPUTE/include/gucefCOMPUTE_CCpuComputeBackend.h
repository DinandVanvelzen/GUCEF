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

#ifndef GUCEF_COMPUTE_CCPUCOMPUTEBACKEND_H
#define GUCEF_COMPUTE_CCPUCOMPUTEBACKEND_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_COMPUTE_CCOMPUTEBACKEND_H
#include "gucefCOMPUTE_CComputeBackend.h"
#define GUCEF_COMPUTE_CCOMPUTEBACKEND_H
#endif /* GUCEF_COMPUTE_CCOMPUTEBACKEND_H ? */

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
 *  The built-in CPU multi-core backend ("cpu"), present on every platform and always registered: the correctness
 *  reference of every kernel and the multi-core provider of batch dispatch chains. It offers one device,
 *  CCpuComputeDevice, whose dispatches run through CCpuParallelFor. Its priority is the lowest, so every GPU device
 *  that meets the requirements wins over it.
 */
class GUCEF_COMPUTE_PUBLIC_CPP CCpuComputeBackend : public CComputeBackend
{
    public:

    static const CString BackendName;       /**< "cpu" */
    static const Int32 BackendPriority;     /**< -1000 */

    CCpuComputeBackend( void );

    virtual ~CCpuComputeBackend() GUCEF_VIRTUAL_OVERRIDE;

    virtual CString GetName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual CString GetDescription( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual Int32 GetPriority( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Initialize( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void Shutdown( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool EnumerateDevices( TComputeDeviceInfoVector& devices ) GUCEF_VIRTUAL_OVERRIDE;

    /** The one device; any identifier other than its own, or an index above 0, is COMPUTERESULT_INVALID_ARGUMENT */
    virtual TComputeResult CreateDevice( const CComputeDeviceParams& params, TComputeDevicePtr& device ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    CCpuComputeBackend( const CCpuComputeBackend& src );            /**< not implemented */
    CCpuComputeBackend& operator=( const CCpuComputeBackend& src ); /**< not implemented */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace COMPUTE */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMPUTE_CCPUCOMPUTEBACKEND_H ? */
