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

#ifndef GUCEF_COMPUTE_CCOMPUTEKERNELARTEFACT_H
#define GUCEF_COMPUTE_CCOMPUTEKERNELARTEFACT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTFACTORYBASE_H
#include "CTFactoryBase.h"
#define GUCEF_CORE_CTFACTORYBASE_H
#endif /* GUCEF_CORE_CTFACTORYBASE_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

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

class CCpuComputeKernel;

/** The factory of the CPU implementation of a kernel: a CORE::CTFactory< CCpuComputeKernel, CMyKernel, MT::CMutex > of the registering module */
typedef CORE::CTFactoryBase< CCpuComputeKernel, MT::CMutex > TCpuComputeKernelFactory;

/*-------------------------------------------------------------------------*/

/**
 *  The form of a kernel for one backend: the backend name, the format, the entry point, and either the data (SPIR-V,
 *  a fatbin, source text) or, for ARTEFACTFORMAT_CPU, the factory of the CPU implementation. Also the hash of the
 *  source it was produced from and the identification of the compiler, from the kernel pack tool.
 *
 *  Embedded data is static memory of the registering module and is not copied ('SetStaticData()'), the factory
 *  belongs to the registering module too: both must stay valid until the module has unregistered its kernels and
 *  the devices that created the kernel have released it. Data loaded at run time (an override from the VFS) is owned
 *  by the artefact ('SetOwnedData()').
 */
class GUCEF_COMPUTE_PUBLIC_CPP CComputeKernelArtefact
{
    public:

    CComputeKernelArtefact( void );

    CComputeKernelArtefact( const CComputeKernelArtefact& src );

    CComputeKernelArtefact& operator=( const CComputeKernelArtefact& src );

    /** A CPU artefact of the given factory */
    static CComputeKernelArtefact ForCpu( TCpuComputeKernelFactory* factory );

    /** The backend name the artefact is for ("cpu", "vulkan", ...), stored in lower case */
    const CString& GetBackendName( void ) const;
    void SetBackendName( const CString& backendName );

    TComputeArtefactFormat GetFormat( void ) const;
    void SetFormat( TComputeArtefactFormat format );

    /** The name of the function in the artefact to run; empty for the CPU format */
    const CString& GetEntryPoint( void ) const;
    void SetEntryPoint( const CString& entryPoint );

    /** Data in static memory of the registering module, not copied */
    void SetStaticData( const void* data, UInt64 sizeInBytes );
    /** Data owned by the artefact, copied */
    void SetOwnedData( const CORE::CDynamicBuffer& data );
    const void* GetData( void ) const;
    UInt64 GetDataSizeInBytes( void ) const;
    /** True for data owned by the artefact (an override) */
    bool HasOwnedData( void ) const;

    TCpuComputeKernelFactory* GetCpuFactory( void ) const;
    void SetCpuFactory( TCpuComputeKernelFactory* factory );

    /** The interface version of the declaration the artefact was produced for; 0 accepts any */
    UInt32 GetInterfaceVersion( void ) const;
    void SetInterfaceVersion( UInt32 interfaceVersion );

    const CString& GetSourceHash( void ) const;
    void SetSourceHash( const CString& sourceHash );

    const CString& GetCompilerId( void ) const;
    void SetCompilerId( const CString& compilerId );

    /** True when a CPU artefact has a factory, or any other artefact has a format, an entry point and data */
    bool IsValid( void ) const;

    private:

    CString m_backendName;
    TComputeArtefactFormat m_format;
    CString m_entryPoint;
    const void* m_staticData;
    UInt64 m_staticDataSize;
    CORE::CDynamicBuffer m_ownedData;
    bool m_hasOwnedData;
    TCpuComputeKernelFactory* m_cpuFactory;
    UInt32 m_interfaceVersion;
    CString m_sourceHash;
    CString m_compilerId;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace COMPUTE */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMPUTE_CCOMPUTEKERNELARTEFACT_H ? */
