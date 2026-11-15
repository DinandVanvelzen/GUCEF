/*
 *  computebridgeMATH: GUCEF compute bridge plugin of gucefMATH: multi-core and accelerator providers for the batch kernels
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

#ifndef GUCEF_COMPUTEBRIDGEMATH_CTMATHACCELERATORPROVIDER_H
#define GUCEF_COMPUTEBRIDGEMATH_CTMATHACCELERATORPROVIDER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_COMPUTEBRIDGEMATH_CMATHACCELERATOR_H
#include "computebridgeMATH_CMathAccelerator.h"
#define GUCEF_COMPUTEBRIDGEMATH_CMATHACCELERATOR_H
#endif /* GUCEF_COMPUTEBRIDGEMATH_CMATHACCELERATOR_H ? */

#ifndef GUCEF_COMPUTEBRIDGEMATH_MATHKERNELTRAITS_H
#include "computebridgeMATH_MathKernelTraits.h"
#define GUCEF_COMPUTEBRIDGEMATH_MATHKERNELTRAITS_H
#endif /* GUCEF_COMPUTEBRIDGEMATH_MATHKERNELTRAITS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COMPUTEBRIDGEMATH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The ACCELERATOR provider of a kernel (tier ACCELERATOR of the gucefMATH dispatch table): runs the call on the best
 *  device through CComputeHostBatch::Run on the calling thread (upload, dispatch, readback, wait; the synchronous form of
 *  COMPUTE design 3.8). The arrays go into device buffers of their own, so a call in place gives the same result as a
 *  call with separate arrays (MD-7).
 *
 *  Calls with strides other than the packed element size are not supported by the device kernels: the provider
 *  declines them and the dispatcher continues with the next tier, which is why it is registered with
 *  MATHPROVIDER_MAY_FAIL. So does a device that is lost or refuses the work. It writes nothing to the output when it
 *  declines: the outputs are only read back after a successful run.
 */
template < typename TKernelTraits >
class CTMathAcceleratorProvider
{
    public:

    typedef typename TKernelTraits::TArgs TArgs;

    /** The function registered in the dispatch table; 'context' is the SMathBridgeProviderContext of the kernel */
    static MATH::TMathStatus Provide( void* context, const TArgs& args )
    {
        SMathBridgeProviderContext* providerContext = static_cast< SMathBridgeProviderContext* >( context );
        SMathBridgeDiagnostics& diagnostics = *providerContext->diagnostics;
        const UInt32 kernelIndex = static_cast< UInt32 >( providerContext->kernel );

        diagnostics.acceleratorCalls[ kernelIndex ].FetchAdd( 1, MT::MEMORDER_RELAXED );

        CMathAccelerator::TStatePtr state;
        if ( GUCEF_NULL == providerContext->accelerator || !providerContext->accelerator->GetState( state ) )
            return Fail( diagnostics, kernelIndex );

        COMPUTE::CComputeHostBatch::SHostBinding bindings[ GUCEF_COMPUTE_MAX_BINDINGS ];
        const UInt32 bindingCount = TKernelTraits::MakeBindings( args, bindings );
        const COMPUTE::TComputeKernelPtr& kernel = state->kernels[ kernelIndex ];
        if ( 0 == bindingCount || kernel.IsNULL() )
            return Fail( diagnostics, kernelIndex );

        UInt64 constants[ GUCEF_COMPUTE_MAX_CONSTANTS_SIZE / 8 ];
        memset( constants, 0, sizeof( constants ) );
        TKernelTraits::PackConstants( args, constants );

        const COMPUTE::TComputeResult result = state->batch->Run( kernel, bindings, bindingCount, constants, TKernelTraits::GetConstantsSize(),
                                                                  COMPUTE::CComputeDispatchSize::ForItems( TKernelTraits::GetCount( args ) ) );
        // a device that was lost is noticed by its event, which removes the providers: removing them here, inside a call,
        // would wait for this very call (UnregisterProviders waits for the calls that could reach a removed provider)
        if ( COMPUTE::COMPUTERESULT_SUCCESS != result )
            return Fail( diagnostics, kernelIndex );
        return MATH::MATHSTATUS_OK;
    }

    private:

    static MATH::TMathStatus Fail( SMathBridgeDiagnostics& diagnostics, UInt32 kernelIndex )
    {
        diagnostics.acceleratorFailures[ kernelIndex ].FetchAdd( 1, MT::MEMORDER_RELAXED );
        return MATH::MATHSTATUS_INVALID_OPERAND;
    }
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace COMPUTEBRIDGEMATH */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMPUTEBRIDGEMATH_CTMATHACCELERATORPROVIDER_H ? */
