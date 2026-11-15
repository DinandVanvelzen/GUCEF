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

#ifndef GUCEF_COMPUTE_CCOMPUTEBUFFERDESC_H
#define GUCEF_COMPUTE_CCOMPUTEBUFFERDESC_H

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
 *  The description of a buffer: its size in bytes, a usage mask (BUFFERUSAGE_ bits) and where its memory lives.
 *  Buffers are typeless byte ranges. MEMORYLOCATION_UNIFIED is only valid on devices that report unified memory or
 *  mappable device local memory.
 */
class GUCEF_COMPUTE_PUBLIC_CPP CComputeBufferDesc
{
    public:

    /** An empty storage buffer in device memory */
    CComputeBufferDesc( void );

    CComputeBufferDesc( UInt64 sizeInBytes, TComputeBufferUsage usage, TComputeMemoryLocation location );

    UInt64 GetSizeInBytes( void ) const;
    void SetSizeInBytes( UInt64 sizeInBytes );

    TComputeBufferUsage GetUsage( void ) const;
    void SetUsage( TComputeBufferUsage usage );
    /** True when every usage bit of the mask is set */
    bool HasUsage( TComputeBufferUsage usage ) const;

    TComputeMemoryLocation GetMemoryLocation( void ) const;
    void SetMemoryLocation( TComputeMemoryLocation location );

    /** True for the locations the host can map: upload, readback and unified */
    bool IsHostVisible( void ) const;

    private:

    UInt64 m_sizeInBytes;
    TComputeBufferUsage m_usage;
    TComputeMemoryLocation m_location;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace COMPUTE */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMPUTE_CCOMPUTEBUFFERDESC_H ? */
