/*
 *  gucefIMAGE: GUCEF module providing image utilities
 *
 *  Copyright (C) 1998 - 2024.  Dinand Vanvelzen
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

#ifndef GUCEF_IMAGE_MACROS_H
#define GUCEF_IMAGE_MACROS_H

/*
 *      Build configuration specific macros.
 *      Also includes the generic macros which are build config independant.
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_BASICMACROS_H
#include "macros.h"             /* generic often used macros */
#define GUCEF_CORE_BASICMACROS_H
#endif /* GUCEF_CORE_BASICMACROS_H ? */

#ifndef GUCEF_IMAGE_ETYPES_H
#include "gucefIMAGE_ETypes.h"      /* simple types */
#define GUCEF_IMAGE_ETYPES_H
#endif /* GUCEF_IMAGE_ETYPES_H ? */

#ifndef GUCEF_IMAGE_CONFIG_H
#include "gucefIMAGE_config.h"      /* Module build configuration */
#define GUCEF_IMAGE_CONFIG_H
#endif /* GUCEF_IMAGE_CONFIG_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      GENERAL MACROS                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *      Macros for dynamic linking or static linking. Use the switches in the
 *      config file to control the export type.
 */
#undef GUCEF_IMAGE_EXPORTSPEC
#ifdef GUCEFIMAGE_BUILD_MODULE
  #define GUCEF_IMAGE_EXPORTSPEC GUCEF_EXPORT
#else
  #define GUCEF_IMAGE_EXPORTSPEC GUCEF_IMPORT
#endif /* BUILD_DLL */

#undef GUCEF_IMAGE_EXPORT_CPP
#ifdef GUCEF_IMAGE_EXPORT_CPP_CODE
  #define GUCEF_IMAGE_EXPORT_CPP GUCEF_IMAGE_EXPORTSPEC
#else
  #define GUCEF_IMAGE_EXPORT_CPP
#endif /* GUCEF_IMAGE_EXPORT_CPP_CODE */

#undef GUCEF_IMAGE_EXPORT_C
#ifdef GUCEF_IMAGE_EXPORT_C_CODE
  #define GUCEF_IMAGE_EXPORT_C GUCEF_IMAGE_EXPORTSPEC
#else
  #define GUCEF_IMAGE_EXPORT_C
#endif /* GUCEF_IMAGE_EXPORT_C_CODE */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_IMAGE_MACROS_H ? */




