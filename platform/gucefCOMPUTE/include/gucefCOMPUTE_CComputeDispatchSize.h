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

#ifndef GUCEF_COMPUTE_CCOMPUTEDISPATCHSIZE_H
#define GUCEF_COMPUTE_CCOMPUTEDISPATCHSIZE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

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

class CComputeKernelSignature;

/**
 *  How much work one dispatch does: a number of items, from which the groups follow with the workgroup size of the
 *  kernel (the last group is partly empty, kernels bounds-check against the item count), or explicit group counts in
 *  one to three dimensions. Counts are UInt32; callers split larger batches.
 */
class GUCEF_COMPUTE_PUBLIC_CPP CComputeDispatchSize
{
    public:

    /** No work */
    CComputeDispatchSize( void );

    /** 'itemCount' items in one dimension */
    static CComputeDispatchSize ForItems( UInt32 itemCount );

    /** Explicit group counts per axis */
    static CComputeDispatchSize ForGroups( UInt32 x, UInt32 y = 1, UInt32 z = 1 );

    /** True for a size given in items */
    bool IsItems( void ) const;

    /** The item count of an item size; for a group size the invocations of all groups for the given signature */
    UInt32 GetItemCount( const CComputeKernelSignature& signature ) const;

    /** The groups per axis for a kernel with the given signature. For an item size: ceil( items / invocations per group ), 1, 1 */
    UInt32 GetGroupCount( UInt32 axis, const CComputeKernelSignature& signature ) const;

    /** The groups of all axes together; 0 for no work */
    UInt64 GetTotalGroupCount( const CComputeKernelSignature& signature ) const;

    private:

    bool m_isItems;
    UInt32 m_items;
    UInt32 m_groups[ 3 ];
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace COMPUTE */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMPUTE_CCOMPUTEDISPATCHSIZE_H ? */
