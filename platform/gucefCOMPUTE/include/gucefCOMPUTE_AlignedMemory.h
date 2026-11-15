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

#ifndef GUCEF_COMPUTE_ALIGNEDMEMORY_H
#define GUCEF_COMPUTE_ALIGNEDMEMORY_H

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
 *  Allocates 'sizeInBytes' bytes aligned to 'alignment' (a power of two, at least the size of a pointer).
 *  Null on failure or for a size of 0. Free with AlignedFree(). Used for buffers of the CPU backend and the
 *  scratch memory of CPU workers, which honour GUCEF_CPU_REQUIRES_ALIGNED_ACCESS and the alignment of SIMD.
 */
GUCEF_COMPUTE_PUBLIC_CPP void*
AlignedAllocate( UInt64 sizeInBytes, UInt32 alignment );

/** Frees memory of AlignedAllocate(); null is ignored */
GUCEF_COMPUTE_PUBLIC_CPP void
AlignedFree( void* memory );

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace COMPUTE */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMPUTE_ALIGNEDMEMORY_H ? */
