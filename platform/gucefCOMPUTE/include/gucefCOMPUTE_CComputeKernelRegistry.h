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

#ifndef GUCEF_COMPUTE_CCOMPUTEKERNELREGISTRY_H
#define GUCEF_COMPUTE_CCOMPUTEKERNELREGISTRY_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

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

class CComputeGlobal;

/**
 *  The kernels that devices can create: declarations by kernel id, and per declaration one artefact per backend.
 *  Modules contribute kernels through bridge plugins (computebridge<MODULE>), which register on load and call
 *  UnregisterModuleKernels() with their owner pointer before they unload. One CPU artefact per kernel is mandatory:
 *  it is the reference. Reach it through CComputeGlobal::GetKernelRegistry(). Registration, removal and lookups are
 *  thread-safe. Devices hold references to the declaration and artefact they built a kernel from, so removing a
 *  kernel while a device uses it is safe; new CreateKernel() calls then fail with COMPUTERESULT_KERNEL_NOT_FOUND.
 *
 *  Override from the VFS: when an override root is set (CComputeSettings::SetKernelOverrideRoot()), the files
 *  "<root>/<kernelId>/<backend>.ini" and "<root>/<kernelId>/<backend>.<ext>" (the extension of the format, see
 *  ComputeArtefactFormatToFileExtension()) replace the embedded artefact of that backend, or add one where none was
 *  embedded, when the interface version that the ini declares equals the interface version of the declaration.
 *  The ini holds the section [ComputeKernelArtefact] with the keys interfaceVersion, format (as
 *  ComputeArtefactFormatToString()), entryPoint, and optionally sourceHash and compiler. An override that is used,
 *  and one that is refused, is logged with the hashes of both artefacts. The CPU backend has no overrides: its
 *  artefacts are code. Overrides are looked up once per kernel and backend and kept until the root changes or the
 *  kernel is registered or unregistered again.
 */
class GUCEF_COMPUTE_PUBLIC_CPP CComputeKernelRegistry
{
    public:

    /** The name of the section of an override ini */
    static const CString OverrideSectionName;

    /**
     *  Registers a declaration for the owner (any pointer that identifies the registering module, usually the address
     *  of an object of the plugin). False for an invalid declaration or an id that is registered already.
     */
    bool RegisterKernel( const CComputeKernelDecl& decl, const void* owner );

    /**
     *  Registers the artefact for its backend; it replaces an earlier artefact for the same backend.
     *  False when the kernel is not registered or the artefact is invalid.
     */
    bool RegisterArtefact( const CString& kernelId, const CComputeKernelArtefact& artefact );

    /** Unregisters the kernels of the owner, returns how many */
    UInt32 UnregisterModuleKernels( const void* owner );

    bool TryGetKernel( const CString& kernelId, TComputeKernelDeclPtr& decl ) const;

    bool IsKernelRegistered( const CString& kernelId ) const;

    /** The artefact of the kernel for the backend (lower case), an override from the VFS where there is one */
    bool GetArtefact( const CString& kernelId, const CString& backendName, TComputeKernelArtefactPtr& artefact ) const;

    /** Whether there is an artefact for the backend, an override included */
    bool HasArtefact( const CString& kernelId, const CString& backendName ) const;

    UInt32 GetKernelCount( void ) const;

    void GetKernelIds( CString::StringVector& kernelIds ) const;

    /**
     *  The stale check of the kernel pack tool: false when the source hash recorded in the artefact differs from the hash
     *  of the current source. True when they are equal, and when either is empty (nothing to compare)
     */
    bool IsArtefactSourceHashCurrent( const CString& kernelId, const CString& backendName, const CString& currentSourceHash ) const;

    /** The VFS path of the override root; empty switches overrides off */
    void SetOverrideRoot( const CString& vfsRoot );

    CString GetOverrideRoot( void ) const;

    /** Forgets the overrides looked up so far, so changed files are read again */
    void ClearOverrideCache( void );

    private:

    friend class CComputeGlobal;

    CComputeKernelRegistry( void );

    ~CComputeKernelRegistry();

    bool LoadOverride( const CString& root, const CComputeKernelDecl& decl, const CString& backendName,
                       const TComputeKernelArtefactPtr& embedded, TComputeKernelArtefactPtr& loaded ) const;

    CComputeKernelRegistry( const CComputeKernelRegistry& src );            /**< not implemented */
    CComputeKernelRegistry& operator=( const CComputeKernelRegistry& src ); /**< not implemented */

    private:

    typedef GUCEF::map< CString, TComputeKernelArtefactPtr > TArtefactMap;

    struct SKernelEntry
    {
        TComputeKernelDeclPtr decl;
        const void* owner;
        TArtefactMap artefacts;
    };

    typedef GUCEF::map< CString, SKernelEntry > TKernelMap;

    struct SOverrideCacheEntry
    {
        bool hasOverride;
        TComputeKernelArtefactPtr artefact;
    };

    typedef GUCEF::map< CString, SOverrideCacheEntry > TOverrideCache;

    MT::CMutex m_lock;
    TKernelMap m_kernels;
    CString m_overrideRoot;
    mutable TOverrideCache m_overrideCache;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace COMPUTE */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMPUTE_CCOMPUTEKERNELREGISTRY_H ? */
