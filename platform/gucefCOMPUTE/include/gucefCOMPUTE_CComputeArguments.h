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

#ifndef GUCEF_COMPUTE_CCOMPUTEARGUMENTS_H
#define GUCEF_COMPUTE_CCOMPUTEARGUMENTS_H

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
 *  The arguments of one dispatch: the buffers per binding slot (with an offset and a size in bytes) and the
 *  constants block. Copied into the command list at CComputeCommandList::Dispatch(), so the object can be changed
 *  or reused right after. No allocation: the bindings and the constants are fixed size members.
 *
 *  The first 8 bytes of the constants block (base item and item count) are written by the backend for every part of
 *  a dispatch; what the caller puts there is overwritten.
 */
class GUCEF_COMPUTE_PUBLIC_CPP CComputeArguments
{
    public:

    struct SBufferBinding
    {
        TComputeBufferPtr buffer;
        UInt64 offset;
        UInt64 size;            /**< 0 for the rest of the buffer from the offset */
    };

    CComputeArguments( void );

    CComputeArguments( const CComputeArguments& src );

    ~CComputeArguments();

    CComputeArguments& operator=( const CComputeArguments& src );

    /** Binds a buffer to a slot (0 to 7). False for a slot of 8 or above or a null buffer */
    bool SetBuffer( UInt32 slot, const TComputeBufferPtr& buffer, UInt64 offset = 0, UInt64 size = 0 );

    /** Removes the buffer of a slot */
    void ClearBuffer( UInt32 slot );

    /** The binding of a slot; null buffer when the slot is not bound */
    const SBufferBinding& GetBinding( UInt32 slot ) const;

    /** A mask with bit 'slot' set for every bound slot */
    UInt32 GetBoundSlotMask( void ) const;

    /**
     *  Copies the constants block, at most GUCEF_COMPUTE_MAX_CONSTANTS_SIZE (128) bytes including the 8 byte header.
     *  False for a larger block or a null pointer with a size
     */
    bool SetConstants( const void* data, UInt32 sizeInBytes );
    const void* GetConstants( void ) const;
    UInt32 GetConstantsSizeInBytes( void ) const;

    /** Unbinds every buffer and empties the constants */
    void Clear( void );

    private:

    SBufferBinding m_bindings[ GUCEF_COMPUTE_MAX_BINDINGS ];
    UInt32 m_boundMask;
    UInt8 m_constants[ GUCEF_COMPUTE_MAX_CONSTANTS_SIZE ];
    UInt32 m_constantsSize;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace COMPUTE */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMPUTE_CCOMPUTEARGUMENTS_H ? */
