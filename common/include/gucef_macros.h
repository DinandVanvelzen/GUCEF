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

#ifndef GUCEF_MACROS_H
#define GUCEF_MACROS_H

/*
 *      Build configuration specific macros for the GUCEF platform
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CONFIG_H
#include "gucef_config.h"        /* GUCEF configuration */
#define GUCEF_CONFIG_H
#endif /* GUCEF_CONFIG_H ? */

#ifndef GUCEF_PLATFORM_H
#include "gucef_platform.h"      /* GUCEF platform build configuration */
#define GUCEF_PLATFORM_H
#endif /* GUCEF_PLATFORM_H ? */

#ifndef GUCEF_TYPES_H
#include "gucef_types.h"         /* GUCEF platform type identifiers */
#define GUCEF_TYPES_H
#endif /* GUCEF_TYPES_H ? */

#ifndef GUCEF_ENDIAN_H
#include "gucef_endian.h"         /* GUCEF platform endian utils */
#define GUCEF_ENDIAN_H
#endif /* GUCEF_ENDIAN_H ? */

#ifndef GUCEF_BASICHELPERS_H
#include "gucef_basichelpers.h"  /* GUCEF platform convenience basic helper macros */
#define GUCEF_BASICHELPERS_H
#endif /* GUCEF_BASICHELPERS_H ? */

#ifndef GUCEF_CALLCONV_H
#include "gucef_callconv.h"      /* GUCEF platform calling convention macros */
#define GUCEF_CALLCONV_H
#endif /* GUCEF_CALLCONV_H ? */

#ifndef GUCEF_IO_H
#include "gucef_io.h"            /* GUCEF platform macros/constants related to I/O */
#define GUCEF_IO_H
#endif /* GUCEF_IO_H ? */

#ifndef GUCEF_CPP_H
#include "gucef_cpp.h"           /* GUCEF platform macros/constants related to C++ feature support */
#define GUCEF_CPP_H
#endif /* GUCEF_CPP_H ? */

#ifndef GUCEF_SFINAE_H
#include "gucef_SFINAE.h"        /* GUCEF platform C++ template SFINAE utilities, C++98 compatible */
#define GUCEF_SFINAE_H
#endif /* GUCEF_SFINAE_H ? */

#ifndef GUCEF_MEMORY_H
#include "gucef_memory.h"        /* GUCEF platform macros related to memory management */
#define GUCEF_MEMORY_H
#endif /* GUCEF_MEMORY_H ? */

#ifndef GUCEF_ALLOCATOR_H
#include "gucef_allocator.h"     /* GUCEF platform C++ allocator which incorporates memory management macros */
#define GUCEF_ALLOCATOR_H
#endif /* GUCEF_ALLOCATOR_H ? */

#ifndef GUCEF_STL_H
#include "gucef_stl.h"          /* GUCEF platform wrappers of stl containers which by default use GUP's custom allocator */
#define GUCEF_STL_H
#endif /* GUCEF_STL_H ? */

#ifndef GUCEF_SHAREDMODULE_H
#include "gucef_sharedmodule.h"  /* GUCEF platform macros related to the use of shared modules */
#define GUCEF_SHAREDMODULE_H
#endif /* GUCEF_SHAREDMODULE_H ? */

#ifndef GUCEF_SINGLETON_H
#include "gucef_singleton.h"     /* GUCEF platform legacy singleton macros */
#define GUCEF_SINGLETON_H
#endif /* GUCEF_SINGLETON_H ? */

#ifndef GUCEF_OSMAIN_H
#include "gucef_osmain.h"        /* GUCEF platform convenience macros for application entry point management */
#define GUCEF_OSMAIN_H
#endif /* GUCEF_OSMAIN_H ? */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MACROS_H ? */
