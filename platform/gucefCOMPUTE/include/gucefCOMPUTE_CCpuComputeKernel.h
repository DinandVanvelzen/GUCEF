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

#ifndef GUCEF_COMPUTE_CCPUCOMPUTEKERNEL_H
#define GUCEF_COMPUTE_CCPUCOMPUTEKERNEL_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_COMPUTE_CCOMPUTEKERNEL_H
#include "gucefCOMPUTE_CComputeKernel.h"
#define GUCEF_COMPUTE_CCOMPUTEKERNEL_H
#endif /* GUCEF_COMPUTE_CCOMPUTEKERNEL_H ? */

#ifndef GUCEF_COMPUTE_CCPUDISPATCHCONTEXT_H
#include "gucefCOMPUTE_CCpuDispatchContext.h"
#define GUCEF_COMPUTE_CCPUDISPATCHCONTEXT_H
#endif /* GUCEF_COMPUTE_CCPUDISPATCHCONTEXT_H ? */

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
 *  The base class of CPU kernel implementations, which bridge plugins supply as artefact format ARTEFACTFORMAT_CPU
 *  through a CORE::CTFactory< CCpuComputeKernel, CMyKernel, MT::CMutex > (see CComputeKernelArtefact::ForCpu()).
 *  ExecuteGroups() processes groups [ firstGroup, firstGroup + groupCount ) of a dispatch; a whole workgroup is
 *  processed sequentially, so workgroup barriers need no emulation, and the workgroup memory is the scratch area of
 *  the context. It is called concurrently for disjoint group ranges from several threads: it must not change the
 *  kernel object and must not throw. Implementations call the batch functions of gucefMATH where they exist; they are
 *  the reference every GPU artefact of the kernel is tested against.
 */
class GUCEF_COMPUTE_PUBLIC_CPP CCpuComputeKernel : public CComputeKernel
{
    public:

    CCpuComputeKernel( void );

    virtual ~CCpuComputeKernel() GUCEF_VIRTUAL_OVERRIDE;

    virtual void ExecuteGroups( const CCpuDispatchContext& context, UInt32 firstGroup, UInt32 groupCount ) const = 0;

    private:

    CCpuComputeKernel( const CCpuComputeKernel& src );            /**< not implemented */
    CCpuComputeKernel& operator=( const CCpuComputeKernel& src ); /**< not implemented */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace COMPUTE */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMPUTE_CCPUCOMPUTEKERNEL_H ? */
