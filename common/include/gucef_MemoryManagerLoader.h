/*
 *  gucef common header: provides header based platform wide facilities
 *
 *  Copyright (C) 1998 - 2020.  Dinand Vanvelzen
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

/*
 *  Umbrella header: includes the four MemoryManagerLoader sub-headers in order.
 *  External API is unchanged — client code that was previously including this
 *  file directly continues to work without modification.
 *
 *  Sub-header responsibilities:
 *    gucef_MemoryManagerLoader_types.h  — MM_* constants + all TFP_ typedefs
 *    gucef_MemoryManagerLoader_vars.h   — static fp_* and g_* global variables
 *    gucef_MemoryManagerLoader_load.h   — MEMMAN_LazyLoad/Unload + Windows/Linux impl
 *    gucef_MemoryManagerLoader_inline.h — all inline shim functions (malloc, free, OLE, etc.)
 */

#ifndef GUCEF_MEMORYMANAGERLOADER_H
#define GUCEF_MEMORYMANAGERLOADER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CONFIG_H
#include "gucef_config.h"
#define GUCEF_CONFIG_H
#endif /* GUCEF_CONFIG_H ? */

#ifndef GUCEF_TYPES_H
#include "gucef_types.h"
#define GUCEF_TYPES_H
#endif /* GUCEF_TYPES_H ? */

#ifndef GUCEF_BASICHELPERS_H
#include "gucef_basichelpers.h"
#define GUCEF_BASICHELPERS_H
#endif /* GUCEF_BASICHELPERS_H ? */

#if defined( GUCEF_USE_MEMORY_LEAK_CHECKER ) && defined( GUCEF_USE_PLATFORM_MEMORY_LEAK_CHECKER )

#ifndef GUCEF_DYNNEWOFF_H
#include "gucef_dynnewoff.h"      /* prevent circular macro definitions */
#define GUCEF_DYNNEWOFF_H
#endif /* GUCEF_DYNNEWOFF_H ? */

#endif /* defined( GUCEF_USE_MEMORY_LEAK_CHECKER ) && defined( GUCEF_USE_PLATFORM_MEMORY_LEAK_CHECKER ) ? */

#ifndef GUCEF_PLATFORM_H
#include "gucef_platform.h"
#define GUCEF_PLATFORM_H
#endif /* GUCEF_PLATFORM_H ? */

#ifndef GUCEF_CALLCONV_H
#include "gucef_callconv.h"
#define GUCEF_CALLCONV_H
#endif /* GUCEF_CALLCONV_H ? */

#ifndef GUCEF_SHAREDMODULE_H
#include "gucef_sharedmodule.h"
#define GUCEF_SHAREDMODULE_H
#endif /* GUCEF_SHAREDMODULE_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      SUB-HEADERS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MEMORYMANAGERLOADER_TYPES_H
#include "gucef_MemoryManagerLoader_types.h"
#define GUCEF_MEMORYMANAGERLOADER_TYPES_H
#endif /* GUCEF_MEMORYMANAGERLOADER_TYPES_H ? */

#ifndef GUCEF_MEMORYMANAGERLOADER_VARS_H
#include "gucef_MemoryManagerLoader_vars.h"
#define GUCEF_MEMORYMANAGERLOADER_VARS_H
#endif /* GUCEF_MEMORYMANAGERLOADER_VARS_H ? */

#ifndef GUCEF_MEMORYMANAGERLOADER_LOAD_H
#include "gucef_MemoryManagerLoader_load.h"
#define GUCEF_MEMORYMANAGERLOADER_LOAD_H
#endif /* GUCEF_MEMORYMANAGERLOADER_LOAD_H ? */

#ifndef GUCEF_MEMORYMANAGERLOADER_INLINE_H
#include "gucef_MemoryManagerLoader_inline.h"
#define GUCEF_MEMORYMANAGERLOADER_INLINE_H
#endif /* GUCEF_MEMORYMANAGERLOADER_INLINE_H ? */

/*--------------------------------------------------------------------------*/

#endif /* GUCEF_MEMORYMANAGERLOADER_H ? */
