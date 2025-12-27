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
 *  This header contains build switches relative to the entire GUCEF platform
 */

#ifndef GUCEF_CONFIG_H
#define GUCEF_CONFIG_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      BUILD DEFINES                                                      //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *      Target platform selection switches
 *      By default autodetects
 */
/* #define GUCEF_LINUX_BUILD */
/* #define GUCEF_MSWIN_BUILD */

/*-------------------------------------------------------------------------*/

/*
 *      Switches for forcing the calling convention used
 *      If you don't specify one of these a default will be selected for the target platform
 */
/* #define GUCEF_USE_CALLSPEC_C */
/* #define GUCEF_USE_CALLSPEC_STD */
/* #define GUCEF_USE_CALLSPEC_PASCAL */
/* #define GUCEF_USE_CALLSPEC_FAST */

/* #define GUCEF_USE_PLUGIN_CALLSPEC_C */
/* #define GUCEF_USE_PLUGIN_CALLSPEC_STD */
/* #define GUCEF_USE_PLUGIN_CALLSPEC_PASCAL */
/* #define GUCEF_USE_PLUGIN_CALLSPEC_FAST */

/*-------------------------------------------------------------------------*/

/*
 *      Switches for manually setting the bit target
 *      Default is 32bit, may be able to autodetect
 */
/* #define GUCEF_32BIT */
/* #define GUCEF_64BIT */

/*-------------------------------------------------------------------------*/

/*
 *      Debug mode
 *      Autodetects from the _DEBUG define
 */
/* #define GUCEF_DEBUG_MODE */
/* #define GUCEF_DEBUG_VERBOSE */

/*-------------------------------------------------------------------------*/

/*
 *  Switches to enable/disable callstack tracking
 */
/* #define GUCEF_USE_CALLSTACK_TRACING */
/* #define GUCEF_USE_PLATFORM_CALLSTACK_TRACING */

/*-------------------------------------------------------------------------*/

/*
 *  Switches to enable/disable lock tracing
 */
/* #define GUCEF_USE_PLATFORM_LOCK_TRACER */

/*-------------------------------------------------------------------------*/

/*
 *  Switches to enable/disable platform provided memory checking and leak detection
 */
/* #define GUCEF_USE_MEMORY_LEAK_CHECKER */
/* #define GUCEF_USE_PLATFORM_MEMORY_LEAK_CHECKER */
/* #define GUCEF_MEMCHECK_OLEAPI */

/*-------------------------------------------------------------------------*/

/*
 *  Options for the platform allocator to use
 */
#define GUCEF_ALLOCATOR_TYPE_STANDARD_STL               1
#define GUCEF_ALLOCATOR_TYPE_PLATFORM_DIAGNOSTIC        2

#ifndef GUCEF_ALLOCATOR_TYPE
  #define GUCEF_ALLOCATOR_TYPE GUCEF_ALLOCATOR_TYPE_STANDARD_STL
#endif 

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CONFIG_H */
