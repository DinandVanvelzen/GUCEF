/*
 *  gucefVIDEO: GUCEF module providing video decoding and playback
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

#ifndef GUCEF_VIDEO_MACROS_H
#define GUCEF_VIDEO_MACROS_H

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

#ifndef GUCEF_VIDEO_ETYPES_H
#include "gucefVIDEO_ETypes.h"      /* simple types */
#define GUCEF_VIDEO_ETYPES_H
#endif /* GUCEF_VIDEO_ETYPES_H ? */

#ifndef GUCEF_VIDEO_CONFIG_H
#include "gucefVIDEO_config.h"      /* Module build configuration */
#define GUCEF_VIDEO_CONFIG_H
#endif /* GUCEF_VIDEO_CONFIG_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      GENERAL MACROS                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *      Auto detection of debug mode
 */
#ifndef GUCEF_VIDEO_DEBUG_MODE
  #if defined( _DEBUG ) || !defined( NDEBUG )
    #define GUCEF_VIDEO_DEBUG_MODE
  #endif /* compiler DEBUG switches */
#endif /* GUCEF_VIDEO_DEBUG_MODE ? */

/*-------------------------------------------------------------------------*/

#ifdef GUCEFVIDEO_BUILD_MODULE
  #define BUILD_GUCEF_VIDEO_DLL
  #define GUCEF_VIDEO_BUILD_MODULE
#endif /* GUCEFVIDEO_BUILD_MODULE ? */

/*
 *      When compiling the gucefVIDEO library you should define GUCEFVIDEO_BUILD_MODULE
 *      before including this macro file.
 */
#undef GUCEF_VIDEO_EXPORTSPEC
#ifdef GUCEF_VIDEO_BUILD_MODULE
  #define GUCEF_VIDEO_EXPORTSPEC GUCEF_EXPORT
#else
  #define GUCEF_VIDEO_EXPORTSPEC GUCEF_IMPORT
#endif

/*-------------------------------------------------------------------------*/

#undef GUCEF_VIDEO_PLUGIN_EXPORTSPEC
#ifdef BUILD_GUCEF_VIDEO_PLUGIN_DLL
  #define GUCEF_VIDEO_PLUGIN_EXPORTSPEC GUCEF_EXPORT
#else
  #define GUCEF_VIDEO_PLUGIN_EXPORTSPEC GUCEF_IMPORT
#endif

/*-------------------------------------------------------------------------*/

#undef GUCEF_VIDEO_PUBLIC_CPP
#ifdef GUCEF_VIDEO_PUBLIC_CPP_CODE
  #define GUCEF_VIDEO_PUBLIC_CPP GUCEF_VIDEO_EXPORTSPEC
#else
  #define GUCEF_VIDEO_PUBLIC_CPP GUCEF_HIDDEN
#endif /* GUCEF_VIDEO_PUBLIC_CPP_CODE */

#undef GUCEF_VIDEO_PUBLIC_C
#ifdef GUCEF_VIDEO_PUBLIC_C_CODE
  #define GUCEF_VIDEO_PUBLIC_C GUCEF_VIDEO_EXPORTSPEC
#else
  #define GUCEF_VIDEO_PUBLIC_C GUCEF_HIDDEN
#endif /* GUCEF_VIDEO_PUBLIC_C_CODE */

#define GUCEF_VIDEO_PRIVATE_CPP GUCEF_HIDDEN
#define GUCEF_VIDEO_PRIVATE_C GUCEF_HIDDEN

/*-------------------------------------------------------------------------*/

#undef GUCEF_VIDEO_PLUGIN_PUBLIC_CPP
#ifdef GUCEF_VIDEO_PLUGIN_PUBLIC_CPP_CODE
  #define GUCEF_VIDEO_PLUGIN_PUBLIC_CPP GUCEF_VIDEO_PLUGIN_EXPORTSPEC
#else
  #define GUCEF_VIDEO_PLUGIN_PUBLIC_CPP GUCEF_HIDDEN
#endif /* GUCEF_VIDEO_PLUGIN_PUBLIC_CPP_CODE */

#undef GUCEF_VIDEO_PLUGIN_PUBLIC_C
#ifdef GUCEF_VIDEO_PLUGIN_PUBLIC_C_CODE
  #define GUCEF_VIDEO_PLUGIN_PUBLIC_C GUCEF_VIDEO_PLUGIN_EXPORTSPEC
#else
  #define GUCEF_VIDEO_PLUGIN_PUBLIC_C GUCEF_HIDDEN
#endif /* GUCEF_VIDEO_PLUGIN_PUBLIC_C_CODE */

#define GUCEF_VIDEO_PLUGIN_PRIVATE_CPP GUCEF_HIDDEN
#define GUCEF_VIDEO_PLUGIN_PRIVATE_C GUCEF_HIDDEN

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_VIDEO_MACROS_H ? */
