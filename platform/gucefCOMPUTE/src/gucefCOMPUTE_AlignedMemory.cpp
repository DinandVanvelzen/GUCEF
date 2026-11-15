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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <new>

#ifndef GUCEF_DYNNEWOFF_H
#include "gucef_dynnewoff.h"
#define GUCEF_DYNNEWOFF_H
#endif /* GUCEF_DYNNEWOFF_H ? */

#include "gucefCOMPUTE_AlignedMemory.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COMPUTE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void*
AlignedAllocate( UInt64 sizeInBytes, UInt32 alignment )
{GUCEF_TRACE;

    if ( 0 == sizeInBytes || 0 == alignment || 0 != ( alignment & ( alignment - 1 ) ) )
        return GUCEF_NULL;
    if ( alignment < sizeof( void* ) )
        alignment = sizeof( void* );

    // the original pointer is kept in the slot just before the aligned block
    const UInt64 total = sizeInBytes + alignment + sizeof( void* );
    if ( total < sizeInBytes || total > static_cast< UInt64 >( static_cast< size_t >( -1 ) ) )
        return GUCEF_NULL;
    // through operator new, so allocation counters and replaced allocators of the application see it
    void* raw = ::operator new( static_cast< size_t >( total ), std::nothrow );
    if ( GUCEF_NULL == raw )
        return GUCEF_NULL;

    const size_t start = reinterpret_cast< size_t >( raw ) + sizeof( void* );
    const size_t aligned = ( start + alignment - 1 ) & ~( static_cast< size_t >( alignment ) - 1 );
    void** slot = reinterpret_cast< void** >( aligned - sizeof( void* ) );
    *slot = raw;
    return reinterpret_cast< void* >( aligned );
}

/*-------------------------------------------------------------------------*/

void
AlignedFree( void* memory )
{GUCEF_TRACE;

    if ( GUCEF_NULL == memory )
        return;
    void** slot = reinterpret_cast< void** >( reinterpret_cast< size_t >( memory ) - sizeof( void* ) );
    ::operator delete( *slot );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace COMPUTE */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
