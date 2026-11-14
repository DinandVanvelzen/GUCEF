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

#ifndef GUCEF_COMPUTE_MACROS_H
#define GUCEF_COMPUTE_MACROS_H

/*
 *      Build configuration specific macros.
 *      Also includes the generic macros which are build config independant.
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MACROS_H
#include "gucef_macros.h"                /* generic often used macros */
#define GUCEF_MACROS_H
#endif /* GUCEF_MACROS_H ? */

#ifndef GUCEF_COMPUTE_ETYPES_H
#include "gucefCOMPUTE_ETypes.h"      /* simple types */
#define GUCEF_COMPUTE_ETYPES_H
#endif /* GUCEF_COMPUTE_ETYPES_H ? */

#ifndef GUCEF_COMPUTE_CONFIG_H
#include "gucefCOMPUTE_config.h"      /* Module build configuration */
#define GUCEF_COMPUTE_CONFIG_H
#endif /* GUCEF_COMPUTE_CONFIG_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      GENERAL MACROS                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *      Auto detection of debug mode
 */
#ifndef GUCEF_COMPUTE_DEBUG_MODE
  #if defined( _DEBUG ) || !defined( NDEBUG )
    #define GUCEF_COMPUTE_DEBUG_MODE
  #endif /* compiler DEBUG switches */
#endif /* GUCEF_COMPUTE_DEBUG_MODE ? */

/*-------------------------------------------------------------------------*/

#ifdef GUCEFCOMPUTE_BUILD_MODULE
  #define GUCEF_COMPUTE_BUILD_MODULE
#endif /* GUCEFCOMPUTE_BUILD_MODULE ? */

/*
 *      When compiling the gucefCOMPUTE library you should define GUCEF_COMPUTE_BUILD_MODULE
 *      before including this macro file.
 */
#undef GUCEF_COMPUTE_EXPORTSPEC
#ifdef GUCEF_COMPUTE_BUILD_MODULE
  #define GUCEF_COMPUTE_EXPORTSPEC GUCEF_EXPORT
#else
  #define GUCEF_COMPUTE_EXPORTSPEC GUCEF_IMPORT
#endif /* GUCEF_COMPUTE_BUILD_MODULE ? */

/*-------------------------------------------------------------------------*/

#undef GUCEF_COMPUTE_PUBLIC_CPP
#ifdef GUCEF_COMPUTE_PUBLIC_CPP_CODE
  #define GUCEF_COMPUTE_PUBLIC_CPP GUCEF_COMPUTE_EXPORTSPEC
#else
  #define GUCEF_COMPUTE_PUBLIC_CPP GUCEF_HIDDEN
#endif /* GUCEF_COMPUTE_PUBLIC_CPP_CODE */

#undef GUCEF_COMPUTE_PUBLIC_C
#ifdef GUCEF_COMPUTE_PUBLIC_C_CODE
  #define GUCEF_COMPUTE_PUBLIC_C GUCEF_COMPUTE_EXPORTSPEC
#else
  #define GUCEF_COMPUTE_PUBLIC_C GUCEF_HIDDEN
#endif /* GUCEF_COMPUTE_PUBLIC_C_CODE */

#undef GUCEF_COMPUTE_PRIVATE_CPP
#define GUCEF_COMPUTE_PRIVATE_CPP GUCEF_HIDDEN
#undef GUCEF_COMPUTE_PRIVATE_C
#define GUCEF_COMPUTE_PRIVATE_C GUCEF_HIDDEN

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMPUTE_MACROS_H ? */
