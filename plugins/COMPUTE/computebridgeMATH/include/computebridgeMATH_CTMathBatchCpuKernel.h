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

#ifndef GUCEF_COMPUTEBRIDGEMATH_CTMATHBATCHCPUKERNEL_H
#define GUCEF_COMPUTEBRIDGEMATH_CTMATHBATCHCPUKERNEL_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MATH_CMATHGLOBAL_H
#include "gucefMATH_CMathGlobal.h"
#define GUCEF_MATH_CMATHGLOBAL_H
#endif /* GUCEF_MATH_CMATHGLOBAL_H ? */

#ifndef GUCEF_COMPUTE_CCPUCOMPUTEKERNEL_H
#include "gucefCOMPUTE_CCpuComputeKernel.h"
#define GUCEF_COMPUTE_CCPUCOMPUTEKERNEL_H
#endif /* GUCEF_COMPUTE_CCPUCOMPUTEKERNEL_H ? */

#ifndef GUCEF_COMPUTEBRIDGEMATH_CMATHBRIDGEGLOBAL_H
#include "computebridgeMATH_CMathBridgeGlobal.h"
#define GUCEF_COMPUTEBRIDGEMATH_CMATHBRIDGEGLOBAL_H
#endif /* GUCEF_COMPUTEBRIDGEMATH_CMATHBRIDGEGLOBAL_H ? */

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
 *  The CPU implementation of a bridged kernel, the artefact format ARTEFACTFORMAT_CPU: the correctness reference of
 *  every GPU artefact of the kernel. A workgroup is GroupSize items; a group range is run as one MATH batch call over
 *  the items of the range (the best single thread provider, SIMD where there is one), so it is bit-identical with the
 *  scalar provider. The kernel has no state: ExecuteGroups() is called from several threads at once.
 */
template < typename TKernelTraits >
class CTMathBatchCpuKernel : public COMPUTE::CCpuComputeKernel
{
    public:

    /** Items per workgroup of every bridged kernel */
    static const UInt32 GroupSize = 256;

    /** Counted in SMathBridgeDiagnostics::liveKernels: the plugin stays mapped while objects of its code exist (computebridgeMATH.cpp) */
    CTMathBatchCpuKernel( void ) : COMPUTE::CCpuComputeKernel()
    {
        CMathBridgeGlobal::Instance()->GetDiagnostics().liveKernels.FetchAdd( 1 );
    }

    virtual ~CTMathBatchCpuKernel() GUCEF_VIRTUAL_OVERRIDE
    {
        CMathBridgeGlobal::Instance()->GetDiagnostics().liveKernels.FetchSub( 1 );
    }

    virtual void ExecuteGroups( const COMPUTE::CCpuDispatchContext& context, UInt32 firstGroup, UInt32 groupCount ) const GUCEF_VIRTUAL_OVERRIDE
    {
        MATH::CMathGlobal* mathGlobal = MATH::CMathGlobal::Instance();
        if ( GUCEF_NULL == mathGlobal )
            return;

        // the items of the groups, bounded by the item count of the dispatch and by what the bound buffers hold
        UInt64 capacity = TKernelTraits::GetItemCapacity( context );
        if ( capacity > context.GetItemCount() )
            capacity = context.GetItemCount();
        const UInt64 begin = static_cast< UInt64 >( firstGroup ) * GroupSize;
        UInt64 end = begin + static_cast< UInt64 >( groupCount ) * GroupSize;
        if ( end > capacity )
            end = capacity;
        if ( begin >= end )
            return;

        TKernelTraits::RunDevice( context, static_cast< UInt32 >( begin ), static_cast< UInt32 >( end - begin ), mathGlobal->GetBatchDispatcher() );
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

#endif /* GUCEF_COMPUTEBRIDGEMATH_CTMATHBATCHCPUKERNEL_H ? */
