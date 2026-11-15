/*
 *  gucefDRNUTILS: GUCEF module providing remote view and control of job schedulers and the task manager
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

#ifndef GUCEF_DRNUTILS_MACROS_H
#define GUCEF_DRNUTILS_MACROS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_MACROS_H
#include "gucefCORE_macros.h"
#define GUCEF_CORE_MACROS_H
#endif /* GUCEF_CORE_MACROS_H ? */

#ifndef GUCEF_DRNUTILS_CONFIG_H
#include "gucefDRNUTILS_config.h"
#define GUCEF_DRNUTILS_CONFIG_H
#endif /* GUCEF_DRNUTILS_CONFIG_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *      When compiling the gucefDRNUTILS library GUCEF_DRNUTILS_BUILD_MODULE is defined (by the build files).
 *      Macros for dynamic linking, controlled by the switches in the config file.
 */
#undef GUCEF_DRNUTILS_EXPORTSPEC
#if defined( GUCEF_DRNUTILS_BUILD_MODULE )
  #define GUCEF_DRNUTILS_EXPORTSPEC GUCEF_EXPORT
#elif defined( GUCEF_DRNUTILS_EMBED_CODE )
  #define GUCEF_DRNUTILS_EXPORTSPEC GUCEF_HIDDEN
#else
  #define GUCEF_DRNUTILS_EXPORTSPEC GUCEF_IMPORT
#endif /* GUCEF_DRNUTILS_BUILD_MODULE ? */

#undef GUCEF_DRNUTILS_PUBLIC_CPP
#ifdef GUCEF_DRNUTILS_PUBLIC_CPP_CODE
  #define GUCEF_DRNUTILS_PUBLIC_CPP GUCEF_DRNUTILS_EXPORTSPEC
#else
  #define GUCEF_DRNUTILS_PUBLIC_CPP GUCEF_HIDDEN
#endif /* GUCEF_DRNUTILS_PUBLIC_CPP_CODE ? */

#undef GUCEF_DRNUTILS_PUBLIC_C
#ifdef GUCEF_DRNUTILS_PUBLIC_C_CODE
  #define GUCEF_DRNUTILS_PUBLIC_C GUCEF_DRNUTILS_EXPORTSPEC
#else
  #define GUCEF_DRNUTILS_PUBLIC_C GUCEF_HIDDEN
#endif /* GUCEF_DRNUTILS_PUBLIC_C_CODE ? */

#define GUCEF_DRNUTILS_PRIVATE_CPP GUCEF_HIDDEN
#define GUCEF_DRNUTILS_PRIVATE_C GUCEF_HIDDEN

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRNUTILS_MACROS_H ? */
