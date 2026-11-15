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

#ifndef GUCEF_COMPUTE_CCOMPUTEKERNELDECL_H
#define GUCEF_COMPUTE_CCOMPUTEKERNELDECL_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_COMPUTE_CCOMPUTEKERNELSIGNATURE_H
#include "gucefCOMPUTE_CComputeKernelSignature.h"
#define GUCEF_COMPUTE_CCOMPUTEKERNELSIGNATURE_H
#endif /* GUCEF_COMPUTE_CCOMPUTEKERNELSIGNATURE_H ? */

#ifndef GUCEF_COMPUTE_CCOMPUTETOLERANCE_H
#include "gucefCOMPUTE_CComputeTolerance.h"
#define GUCEF_COMPUTE_CCOMPUTETOLERANCE_H
#endif /* GUCEF_COMPUTE_CCOMPUTETOLERANCE_H ? */

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
 *  The declaration of a kernel: its id ("<MODULE>.<Name>.<Kind>", for example "MATH.TransformPoints3.F32"),
 *  its interface version, its signature, how deterministic it is with the tolerance against the CPU reference,
 *  its preferred grain and a short description for logs. Registered in CComputeKernelRegistry by the module or plugin
 *  that owns the kernel, together with one artefact per backend (the CPU artefact is mandatory: it is the reference).
 *
 *  The grain is the number of workgroups the CPU backend processes per chunk of a parallel dispatch; 0 lets the CPU
 *  backend choose about 64 KiB of output per chunk. Chunk boundaries never depend on the number of workers.
 */
class GUCEF_COMPUTE_PUBLIC_CPP CComputeKernelDecl
{
    public:

    CComputeKernelDecl( void );

    CComputeKernelDecl( const CString& kernelId, UInt32 interfaceVersion );

    const CString& GetKernelId( void ) const;
    void SetKernelId( const CString& kernelId );

    /** Artefacts, also overrides from the VFS, must declare this version to be used */
    UInt32 GetInterfaceVersion( void ) const;
    void SetInterfaceVersion( UInt32 interfaceVersion );

    const CComputeKernelSignature& GetSignature( void ) const;
    CComputeKernelSignature& GetSignature( void );
    void SetSignature( const CComputeKernelSignature& signature );

    TComputeDeterminism GetDeterminism( void ) const;
    void SetDeterminism( TComputeDeterminism determinism );

    const CComputeTolerance& GetTolerance( void ) const;
    void SetTolerance( const CComputeTolerance& tolerance );

    /** The features the kernel needs: those of its signature */
    TComputeFeatures GetRequiredFeatures( void ) const;

    UInt32 GetPreferredGrain( void ) const;
    void SetPreferredGrain( UInt32 groupsPerChunk );

    const CString& GetDescription( void ) const;
    void SetDescription( const CString& description );

    /** True for an id of three non empty parts separated by dots, a signature whose workgroup has at least one invocation */
    bool IsValid( void ) const;

    private:

    CString m_kernelId;
    UInt32 m_interfaceVersion;
    CComputeKernelSignature m_signature;
    TComputeDeterminism m_determinism;
    CComputeTolerance m_tolerance;
    UInt32 m_preferredGrain;
    CString m_description;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace COMPUTE */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMPUTE_CCOMPUTEKERNELDECL_H ? */
