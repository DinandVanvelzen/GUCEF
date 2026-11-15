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

#ifndef GUCEF_COMPUTE_CCOMPUTEKERNEL_H
#define GUCEF_COMPUTE_CCOMPUTEKERNEL_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_COMPUTE_CCOMPUTEKERNELDECL_H
#include "gucefCOMPUTE_CComputeKernelDecl.h"
#define GUCEF_COMPUTE_CCOMPUTEKERNELDECL_H
#endif /* GUCEF_COMPUTE_CCOMPUTEKERNELDECL_H ? */

#ifndef GUCEF_COMPUTE_CCOMPUTEKERNELARTEFACT_H
#include "gucefCOMPUTE_CComputeKernelArtefact.h"
#define GUCEF_COMPUTE_CCOMPUTEKERNELARTEFACT_H
#endif /* GUCEF_COMPUTE_CCOMPUTEKERNELARTEFACT_H ? */

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

class CComputeDevice;

/**
 *  A kernel compiled or loaded for one device, created by CComputeDevice::CreateKernel() from the declaration and
 *  the artefact for its backend in the kernel registry. It keeps its device alive, and the declaration and the
 *  artefact it was made from, also when the registering module unregisters them meanwhile. The device caches it per
 *  kernel id without keeping it alive (a weak reference), so kernels and devices never keep each other.
 *  A kernel created from a factory of a plugin must be released before that plugin unloads. Backends create kernels
 *  with a reference count of their own (TComputeKernelPtr( new CMyKernel ) or a CORE::CTFactory), never through
 *  CTSharedObjCreator: the weak reference of the cache needs a count that outlives the object.
 *  Backends derive from it (the CPU backend through CCpuComputeKernel). Immutable once created and thread-safe.
 */
class GUCEF_COMPUTE_PUBLIC_CPP CComputeKernel
{
    public:

    virtual ~CComputeKernel();

    const CString& GetKernelId( void ) const;

    const CComputeKernelDecl& GetDecl( void ) const;

    const CComputeKernelSignature& GetSignature( void ) const;

    const CComputeKernelArtefact& GetArtefact( void ) const;

    /** The device the kernel was created for, which the kernel keeps alive */
    CComputeDevice* GetDevice( void ) const;

    /** True once the device has bound the kernel to its declaration and artefact */
    bool IsBound( void ) const;

    protected:

    CComputeKernel( void );

    private:

    friend class CComputeDevice;

    void Bind( const TComputeDevicePtr& device, const TComputeKernelDeclPtr& decl, const TComputeKernelArtefactPtr& artefact );

    CComputeKernel( const CComputeKernel& src );            /**< not implemented */
    CComputeKernel& operator=( const CComputeKernel& src ); /**< not implemented */

    private:

    TComputeDevicePtr m_device;
    TComputeKernelDeclPtr m_decl;
    TComputeKernelArtefactPtr m_artefact;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace COMPUTE */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMPUTE_CCOMPUTEKERNEL_H ? */
