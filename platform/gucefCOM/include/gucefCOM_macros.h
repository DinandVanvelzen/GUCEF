/*
 *  gucefCOM: GUCEF module providing communication implementations 
 *  for standardized protocols
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

#ifndef GUCEF_COM_MACROS_H
#define GUCEF_COM_MACROS_H

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
#include "gucef_macros.h"
#define GUCEF_MACROS_H
#endif /* GUCEF_MACROS_H ? */

#ifndef GUCEF_COM_ETYPES_H
#include "gucefCOM_ETypes.h"
#define GUCEF_COM_ETYPES_H
#endif /* GUCEF_COM_ETYPES_H ? */

#ifndef GUCEF_COM_CONFIG_H
#include "gucefCOM_config.h"      /* Module build configuration */
#define GUCEF_COM_CONFIG_H
#endif /* GUCEF_COM_CONFIG_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      GENERAL MACROS                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *      Auto detection of debug mode
 */
#ifndef MANUAL_DEBUG_MODE_DEFINE
  #if defined( _DEBUG )
    #define DEBUG_MODE
  #else
    #undef DEBUG_MODE
  #endif /* compiler DEBUG switches */
#endif /* MANUAL_DEBUG_MODE_DEFINE ? */

/*-------------------------------------------------------------------------*/

/*
 *      Macros for dynamic linking or static linking. Use the switches in the
 *      config file to control the export type.
 */

#undef GUCEF_COM_EXPORT
#ifdef GUCEF_COM_BUILD_MODULE
  #define GUCEF_COM_EXPORT GUCEF_EXPORT
#else
  #define GUCEF_COM_EXPORT GUCEF_IMPORT
#endif

#undef GUCEF_COM_EXPORT_CPP
#ifdef GUCEF_COM_EXPORT_CPP_CODE
  #define GUCEF_COM_EXPORT_CPP GUCEF_COM_EXPORT
#else
  #define GUCEF_COM_EXPORT_CPP
#endif /* GUCEF_COM_EXPORT_CPP_CODE */

#undef GUCEF_COM_EXPORT_C
#ifdef GUCEF_COM_EXPORT_C_CODE
  #define GUCEF_COM_EXPORT_C GUCEF_COM_EXPORT
#else
  #define GUCEF_COM_EXPORT_C
#endif /* GUCEF_COM_EXPORT_C_CODE */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COM_MACROS_H ? */
