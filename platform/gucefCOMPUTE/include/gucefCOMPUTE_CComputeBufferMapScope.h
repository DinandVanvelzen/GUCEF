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

#ifndef GUCEF_COMPUTE_CCOMPUTEBUFFERMAPSCOPE_H
#define GUCEF_COMPUTE_CCOMPUTEBUFFERMAPSCOPE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_COMPUTE_CCOMPUTEBUFFER_H
#include "gucefCOMPUTE_CComputeBuffer.h"
#define GUCEF_COMPUTE_CCOMPUTEBUFFER_H
#endif /* GUCEF_COMPUTE_CCOMPUTEBUFFER_H ? */

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
 *  Maps a buffer for the lifetime of the scope and unmaps it at the end (the RAII scope of Map() / Unmap()).
 *  When mapping fails GetData() is null and GetResult() tells why; nothing is unmapped then.
 */
class GUCEF_COMPUTE_PUBLIC_CPP CComputeBufferMapScope
{
    public:

    CComputeBufferMapScope( const TComputeBufferPtr& buffer, TComputeMapAccess access );

    ~CComputeBufferMapScope();

    bool IsMapped( void ) const;

    TComputeResult GetResult( void ) const;

    void* GetData( void ) const;

    template < typename T >
    T* GetDataAs( void ) const { return static_cast< T* >( m_data ); }

    /** Unmaps before the end of the scope. False when nothing was mapped */
    bool EarlyUnmap( void );

    private:

    CComputeBufferMapScope( const CComputeBufferMapScope& src );            /**< not implemented */
    CComputeBufferMapScope& operator=( const CComputeBufferMapScope& src ); /**< not implemented */

    private:

    TComputeBufferPtr m_buffer;
    void* m_data;
    TComputeResult m_result;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace COMPUTE */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMPUTE_CCOMPUTEBUFFERMAPSCOPE_H ? */
