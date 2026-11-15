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

#ifndef GUCEF_COMPUTEBRIDGEMATH_CMATHKERNELDECLS_H
#define GUCEF_COMPUTEBRIDGEMATH_CMATHKERNELDECLS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_COMPUTEBRIDGEMATH_SMATHBRIDGEKERNELINFO_H
#include "computebridgeMATH_SMathBridgeKernelInfo.h"
#define GUCEF_COMPUTEBRIDGEMATH_SMATHBRIDGEKERNELINFO_H
#endif /* GUCEF_COMPUTEBRIDGEMATH_SMATHBRIDGEKERNELINFO_H ? */

#ifndef GUCEF_COMPUTEBRIDGEMATH_MACROS_H
#include "computebridgeMATH_macros.h"
#define GUCEF_COMPUTEBRIDGEMATH_MACROS_H
#endif /* GUCEF_COMPUTEBRIDGEMATH_MACROS_H ? */

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
 *  The table of the MATH batch kernels the bridge serves and the construction of their declarations for gucefCOMPUTE.
 *  A kernel is bridged when it has a row here; the kernel ids (the strings of section 5.1 of the COMPUTE design) come
 *  from the MATH dispatcher, so they cannot differ from the ones the dispatch table uses. Owned by CMathBridgeGlobal
 *  and plain data after construction.
 */
class COMPUTEBRIDGEMATH_PUBLIC_CPP CMathKernelDecls
{
    public:

    /** The interface version of the declarations: artefacts and overrides of a kernel must declare it */
    static const UInt32 InterfaceVersion = 1;

    /** The number of bridged kernels */
    enum { KernelRowCount = 18 };

    CMathKernelDecls( void );

    ~CMathKernelDecls();

    /** The number of bridged kernels */
    UInt32 GetKernelCount( void ) const;

    /** The row at 'index' of GetKernelCount() */
    const SMathBridgeKernelInfo& GetKernelInfo( UInt32 index ) const;

    /** The row of a kernel; null when the bridge does not serve it */
    const SMathBridgeKernelInfo* FindKernelInfo( MATH::TMathBatchKernel kernel ) const;

    /**
     *  The declaration of a kernel: 'kernelId' is its string id, the signature, determinism and tolerance, the grain
     *  (groups per chunk of the CPU backend) and the description come from the row
     */
    static void MakeDecl( const SMathBridgeKernelInfo& info, const CString& kernelId, COMPUTE::CComputeKernelDecl& decl );

    private:

    CMathKernelDecls( const CMathKernelDecls& src );                /**< not implemented */
    CMathKernelDecls& operator=( const CMathKernelDecls& src );     /**< not implemented */

    private:

    SMathBridgeKernelInfo m_rows[ KernelRowCount ];
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace COMPUTEBRIDGEMATH */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMPUTEBRIDGEMATH_CMATHKERNELDECLS_H ? */
