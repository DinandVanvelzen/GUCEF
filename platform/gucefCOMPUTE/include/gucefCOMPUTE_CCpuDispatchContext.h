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

#ifndef GUCEF_COMPUTE_CCPUDISPATCHCONTEXT_H
#define GUCEF_COMPUTE_CCPUDISPATCHCONTEXT_H

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
 *  What a CPU kernel sees while it executes groups of a dispatch: the constants block (base item and item count in
 *  its first 8 bytes), the memory of every bound buffer range, the group counts and workgroup size, and the scratch
 *  area of the thread (the workgroup memory, at least the size the kernel declares, 64 byte aligned).
 *  One context per thread and chunk; the dispatch data is shared and read only.
 */
class GUCEF_COMPUTE_PUBLIC_CPP CCpuDispatchContext
{
    public:

    /** A bound buffer range */
    struct SBinding
    {
        UInt8* data;
        UInt64 sizeInBytes;
        TComputeAccess access;
    };

    /** The data of one dispatch, filled by the CPU queue */
    struct SDispatchData
    {
        SBinding bindings[ GUCEF_COMPUTE_MAX_BINDINGS ];    /**< indexed by slot; data null for an unbound slot */
        UInt64 constants[ GUCEF_COMPUTE_MAX_CONSTANTS_SIZE / 8 ];
        UInt32 constantsSize;
        UInt32 groupCount[ 3 ];
        UInt32 workgroupSize[ 3 ];
        UInt32 totalGroups;

        SDispatchData( void );
    };

    CCpuDispatchContext( const SDispatchData& data, void* scratch, UInt32 scratchSizeInBytes );

    const void* GetConstants( void ) const;

    UInt32 GetConstantsSizeInBytes( void ) const;

    /** The constants as a struct of the kernel; the struct starts with the two UInt32 of the header */
    template < typename T >
    const T& GetConstantsAs( void ) const { return *reinterpret_cast< const T* >( m_data->constants ); }

    /** The first item of the dispatch (always 0 on the CPU backend, which never splits) */
    UInt32 GetBaseItem( void ) const;

    /** The items of the dispatch; kernels bounds-check against it */
    UInt32 GetItemCount( void ) const;

    /** The memory of the range bound to the slot; null for an unbound slot */
    void* GetBufferData( UInt32 slot ) const;

    UInt64 GetBufferSizeInBytes( UInt32 slot ) const;

    template < typename T >
    T* GetBufferAs( UInt32 slot ) const { return static_cast< T* >( GetBufferData( slot ) ); }

    /** The elements of type T that fit the range bound to the slot */
    template < typename T >
    UInt64 GetBufferElementCount( UInt32 slot ) const { return GetBufferSizeInBytes( slot ) / sizeof( T ); }

    UInt32 GetGroupCount( UInt32 axis ) const;

    UInt32 GetTotalGroupCount( void ) const;

    UInt32 GetWorkgroupSize( UInt32 axis ) const;

    UInt32 GetInvocationsPerGroup( void ) const;

    void* GetScratch( void ) const;

    UInt32 GetScratchSizeInBytes( void ) const;

    private:

    const SDispatchData* m_data;
    void* m_scratch;
    UInt32 m_scratchSize;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace COMPUTE */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMPUTE_CCPUDISPATCHCONTEXT_H ? */
