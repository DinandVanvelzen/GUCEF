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

#ifndef GUCEF_COMPUTE_CCOMPUTEKERNELSIGNATURE_H
#define GUCEF_COMPUTE_CCOMPUTEKERNELSIGNATURE_H

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

/**
 *  The interface of a kernel: its buffer bindings (slot, access, element type), the size of its constants block,
 *  its fixed workgroup size, the workgroup memory it uses and the device features it needs.
 *
 *  Binding model (common to every backend): at most GUCEF_COMPUTE_MAX_BINDINGS (8) storage buffers in slots 0 to 7
 *  and one constants block of at most GUCEF_COMPUTE_MAX_CONSTANTS_SIZE (128) bytes whose first 8 bytes are the base
 *  item and the item count (two UInt32), which every backend writes so it can split a dispatch.
 */
class GUCEF_COMPUTE_PUBLIC_CPP CComputeKernelSignature
{
    public:

    struct SBinding
    {
        UInt32 slot;
        TComputeAccess access;
        TComputeElementType elementType;      /**< for validation and for the size of an item; ELEMENTTYPE_UNKNOWN is untyped */

        SBinding( void ) : slot( 0 ), access( COMPUTEACCESS_READ ), elementType( ELEMENTTYPE_UNKNOWN ) {}
    };

    /** No bindings, only the 8 byte constants header, a workgroup of 64 x 1 x 1, no workgroup memory, no features */
    CComputeKernelSignature( void );

    /** Adds a binding. False for a slot of 8 or above, a slot that is already bound, or a ninth binding */
    bool AddBinding( UInt32 slot, TComputeAccess access, TComputeElementType elementType );
    UInt32 GetBindingCount( void ) const;
    /** The binding at position 'index' in the order they were added; false for an index past the end */
    bool GetBinding( UInt32 index, SBinding& binding ) const;
    /** The binding of a slot; false when the slot is not bound */
    bool FindBinding( UInt32 slot, SBinding& binding ) const;
    /** A mask with bit 'slot' set for every bound slot */
    UInt32 GetBoundSlotMask( void ) const;

    /** The size of the constants block in bytes including the 8 byte header; from 8 to 128. False outside that range */
    bool SetConstantsSizeInBytes( UInt32 bytes );
    UInt32 GetConstantsSizeInBytes( void ) const;

    /** The fixed workgroup size per axis, each at least 1. False for a 0 */
    bool SetWorkgroupSize( UInt32 x, UInt32 y, UInt32 z );
    UInt32 GetWorkgroupSize( UInt32 axis ) const;
    /** x * y * z */
    UInt32 GetInvocationsPerGroup( void ) const;

    void SetWorkgroupMemoryInBytes( UInt32 bytes );
    UInt32 GetWorkgroupMemoryInBytes( void ) const;

    void SetRequiredFeatures( TComputeFeatures features );
    TComputeFeatures GetRequiredFeatures( void ) const;

    /** The largest element size of the writable bindings, 4 when none is typed: the output bytes of one item */
    UInt32 GetOutputBytesPerItem( void ) const;

    private:

    SBinding m_bindings[ GUCEF_COMPUTE_MAX_BINDINGS ];
    UInt32 m_bindingCount;
    UInt32 m_constantsSize;
    UInt32 m_workgroupSize[ 3 ];
    UInt32 m_workgroupMemory;
    TComputeFeatures m_requiredFeatures;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace COMPUTE */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMPUTE_CCOMPUTEKERNELSIGNATURE_H ? */
