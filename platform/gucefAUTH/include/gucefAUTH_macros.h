/*
 *  gucefAUTH: GUCEF module providing player and service authentication through external identity providers
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

#ifndef GUCEF_AUTH_MACROS_H
#define GUCEF_AUTH_MACROS_H

/*
 *      Build configuration specific macros.
 *      Also includes the generic macros which are build config independant.
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_MACROS_H
#include "gucefCORE_macros.h"
#define GUCEF_CORE_MACROS_H
#endif /* GUCEF_CORE_MACROS_H ? */

#ifndef GUCEF_AUTH_TYPES_H
#include "gucefAUTH_Types.h"   /* simple types */
#define GUCEF_AUTH_TYPES_H
#endif /* GUCEF_AUTH_TYPES_H ? */

#ifndef GUCEF_AUTH_CONFIG_H
#include "gucefAUTH_config.h"  /* Module build configuration */
#define GUCEF_AUTH_CONFIG_H
#endif /* GUCEF_AUTH_CONFIG_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      GENERAL MACROS                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *      Macros for dynamic linking. Use the switches in the
 *      config file to control the export type.
 */

#ifdef GUCEFAUTH_BUILD_MODULE
  #define GUCEF_AUTH_BUILD_MODULE
#endif /* GUCEFAUTH_BUILD_MODULE ? */

#undef GUCEF_AUTH_EXPORT
#ifdef GUCEF_AUTH_BUILD_MODULE
  #define GUCEF_AUTH_EXPORT GUCEF_EXPORT
#else
  #define GUCEF_AUTH_EXPORT GUCEF_IMPORT
#endif /* GUCEF_AUTH_BUILD_MODULE ? */

/*
 *  Are we exporting C++ code ?
 */
#undef GUCEF_AUTH_EXPORT_CPP
#ifdef GUCEF_AUTH_EXPORT_CPP_CODE
  #define GUCEF_AUTH_EXPORT_CPP GUCEF_AUTH_EXPORT
#else
  #define GUCEF_AUTH_EXPORT_CPP
#endif /* GUCEF_AUTH_EXPORT_CPP_CODE ? */

/*
 *  Are we exporting C code ?
 */
#undef GUCEF_AUTH_EXPORT_C
#ifdef GUCEF_AUTH_EXPORT_C_CODE
  #define GUCEF_AUTH_EXPORT_C GUCEF_AUTH_EXPORT
#else
  #define GUCEF_AUTH_EXPORT_C
#endif /* GUCEF_AUTH_EXPORT_C_CODE ? */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUTH_MACROS_H ? */
