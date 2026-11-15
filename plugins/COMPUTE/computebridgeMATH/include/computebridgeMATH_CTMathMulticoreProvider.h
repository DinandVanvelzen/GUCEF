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

#ifndef GUCEF_COMPUTEBRIDGEMATH_CTMATHMULTICOREPROVIDER_H
#define GUCEF_COMPUTEBRIDGEMATH_CTMATHMULTICOREPROVIDER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MATH_CMATHGLOBAL_H
#include "gucefMATH_CMathGlobal.h"
#define GUCEF_MATH_CMATHGLOBAL_H
#endif /* GUCEF_MATH_CMATHGLOBAL_H ? */

#ifndef GUCEF_COMPUTE_CCOMPUTEGLOBAL_H
#include "gucefCOMPUTE_CComputeGlobal.h"
#define GUCEF_COMPUTE_CCOMPUTEGLOBAL_H
#endif /* GUCEF_COMPUTE_CCOMPUTEGLOBAL_H ? */

#ifndef GUCEF_COMPUTEBRIDGEMATH_MATHKERNELTRAITS_H
#include "computebridgeMATH_MathKernelTraits.h"
#define GUCEF_COMPUTEBRIDGEMATH_MATHKERNELTRAITS_H
#endif /* GUCEF_COMPUTEBRIDGEMATH_MATHKERNELTRAITS_H ? */

#ifndef GUCEF_COMPUTEBRIDGEMATH_SMATHBRIDGEKERNELINFO_H
#include "computebridgeMATH_SMathBridgeKernelInfo.h"
#define GUCEF_COMPUTEBRIDGEMATH_SMATHBRIDGEKERNELINFO_H
#endif /* GUCEF_COMPUTEBRIDGEMATH_SMATHBRIDGEKERNELINFO_H ? */

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
 *  The MULTICORE provider of a kernel (gucefMATH batch dispatch table, tier MULTICORE): cuts the argument struct of
 *  the call into chunks of 'grain' items by adjusting pointers and count, and runs the chunks through the CPU job
 *  system of gucefCOMPUTE (CCpuParallelFor::Run) directly over the host pointers: nothing is copied. The calling
 *  thread takes part in the work (the synchronous form of data-parallel work, COMPUTE design 4.3), so the call
 *  returns with the complete result.
 *
 *  Every chunk is a nested MATH batch call to the tiers below MULTICORE with MATHBATCH_DETERMINISTIC: the best bit-exact
 *  single thread provider. The chunk boundaries depend only on the count and the grain, and every lower provider is
 *  bit-identical with the scalar one, so the result is bit-identical with the scalar provider for any number of
 *  workers. The provider is registered with MATHPROVIDER_BITEXACT and MATHPROVIDER_MAY_FAIL.
 *
 *  A call that fails (the job system is gone, a forced failure of a test, an in place conversion)
 *  returns before any output was written, so the dispatcher can continue with the next tier on the same arguments.
 */
template < typename TKernelTraits >
class CTMathMulticoreProvider
{
    public:

    typedef typename TKernelTraits::TArgs TArgs;

    /** The function registered in the dispatch table; 'context' is the SMathBridgeProviderContext of the kernel */
    static MATH::TMathStatus Provide( void* context, const TArgs& args )
    {
        SMathBridgeProviderContext* providerContext = static_cast< SMathBridgeProviderContext* >( context );
        SMathBridgeDiagnostics& diagnostics = *providerContext->diagnostics;
        const UInt32 kernelIndex = static_cast< UInt32 >( providerContext->kernel );

        diagnostics.multiCoreCalls[ kernelIndex ].FetchAdd( 1, MT::MEMORDER_RELAXED );
        if ( diagnostics.TakeForcedMultiCoreFailure() || !TKernelTraits::CanSlice( args ) )
            return Fail( diagnostics, kernelIndex, MATH::MATHSTATUS_INVALID_OPERAND );

        COMPUTE::CComputeGlobal* compute = COMPUTE::CComputeGlobal::InstanceIfCreated();
        MATH::CMathGlobal* mathGlobal = MATH::CMathGlobal::Instance();
        if ( GUCEF_NULL == compute || GUCEF_NULL == mathGlobal )
            return Fail( diagnostics, kernelIndex, MATH::MATHSTATUS_INVALID_OPERAND );

        UInt32 grain = diagnostics.grainOverrideItems.Load( MT::MEMORDER_RELAXED );
        if ( 0 == grain )
            grain = providerContext->info->grainItems;

        const UInt32 count = TKernelTraits::GetCount( args );
        const UInt64 chunkCount = ( static_cast< UInt64 >( count ) + grain - 1 ) / grain;
        CChunkBody body( mathGlobal->GetBatchDispatcher(), args, count, grain );
        const COMPUTE::TComputeResult result = compute->GetCpuParallelFor().Run( body, static_cast< UInt32 >( chunkCount ) );
        if ( COMPUTE::COMPUTERESULT_SUCCESS != result || body.HasFailed() )
            return Fail( diagnostics, kernelIndex, COMPUTE::COMPUTERESULT_OUT_OF_HOST_MEMORY == result ? MATH::MATHSTATUS_OUT_OF_MEMORY : MATH::MATHSTATUS_INVALID_OPERAND );
        return MATH::MATHSTATUS_OK;
    }

    private:

    /** One chunk: a nested batch call over the items of the chunk */
    class CChunkBody : public COMPUTE::CCpuParallelFor::CBody
    {
        public:

        CChunkBody( const MATH::CMathBatchDispatcher& dispatcher, const TArgs& args, UInt32 count, UInt32 grain )
            : COMPUTE::CCpuParallelFor::CBody() ,
              m_dispatcher( dispatcher )        ,
              m_args( args )                    ,
              m_count( count )                  ,
              m_grain( grain )                  ,
              m_failed( 0 )
        {
        }

        virtual void ExecuteChunk( UInt32 chunkIndex, void* /* scratch */, UInt32 /* scratchSizeInBytes */ ) GUCEF_VIRTUAL_OVERRIDE
        {
            const UInt64 first = static_cast< UInt64 >( chunkIndex ) * m_grain;
            const UInt64 remaining = m_count - first;
            const UInt32 count = static_cast< UInt32 >( remaining < m_grain ? remaining : m_grain );

            TArgs sliced;
            TKernelTraits::Slice( m_args, static_cast< UInt32 >( first ), count, sliced );
            const MATH::TMathStatus status = m_dispatcher.Execute( TKernelTraits::Kernel(), sliced, count, MATHBRIDGE_NESTED_FLAGS,
                                                                   TKernelTraits::GetInput( sliced ), TKernelTraits::GetOutput( sliced ) );
            if ( MATH::MATHSTATUS_OK != status )
                m_failed.Store( 1, MT::MEMORDER_RELEASE );
        }

        bool HasFailed( void ) const { return 0 != m_failed.Load( MT::MEMORDER_ACQUIRE ); }

        private:

        const MATH::CMathBatchDispatcher& m_dispatcher;
        const TArgs& m_args;
        UInt32 m_count;
        UInt32 m_grain;
        MT::CTAtomic< UInt32 > m_failed;
    };

    static MATH::TMathStatus Fail( SMathBridgeDiagnostics& diagnostics, UInt32 kernelIndex, MATH::TMathStatus status )
    {
        diagnostics.multiCoreFailures[ kernelIndex ].FetchAdd( 1, MT::MEMORDER_RELAXED );
        return status;
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

#endif /* GUCEF_COMPUTEBRIDGEMATH_CTMATHMULTICOREPROVIDER_H ? */
