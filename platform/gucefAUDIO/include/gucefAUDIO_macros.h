/*
 *  gucefAUDIO: GUCEF module providing audio playback, mixing and decoding
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

#ifndef GUCEF_AUDIO_MACROS_H
#define GUCEF_AUDIO_MACROS_H

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

#ifndef GUCEF_AUDIO_ETYPES_H
#include "gucefAUDIO_ETypes.h"      /* simple types */
#define GUCEF_AUDIO_ETYPES_H
#endif /* GUCEF_AUDIO_ETYPES_H ? */

#ifndef GUCEF_AUDIO_CONFIG_H
#include "gucefAUDIO_config.h"      /* Module build configuration */
#define GUCEF_AUDIO_CONFIG_H
#endif /* GUCEF_AUDIO_CONFIG_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      GENERAL MACROS                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *      Auto detection of debug mode
 */
#ifndef GUCEF_AUDIO_DEBUG_MODE
  #if defined( _DEBUG ) || !defined( NDEBUG )
    #define GUCEF_AUDIO_DEBUG_MODE
  #endif /* compiler DEBUG switches */
#endif /* GUCEF_AUDIO_DEBUG_MODE ? */

/*-------------------------------------------------------------------------*/

#ifdef GUCEFAUDIO_BUILD_MODULE
  #define BUILD_GUCEF_AUDIO_DLL
  #define GUCEF_AUDIO_BUILD_MODULE
#endif /* GUCEFAUDIO_BUILD_MODULE ? */

/*
 *      When compiling the gucefAUDIO library you should define GUCEFAUDIO_BUILD_MODULE
 *      before including this macro file.
 */
#undef GUCEF_AUDIO_EXPORTSPEC
#ifdef GUCEF_AUDIO_BUILD_MODULE
  #define GUCEF_AUDIO_EXPORTSPEC GUCEF_EXPORT
#else
  #define GUCEF_AUDIO_EXPORTSPEC GUCEF_IMPORT
#endif

/*-------------------------------------------------------------------------*/

#undef GUCEF_AUDIO_PLUGIN_EXPORTSPEC
#ifdef BUILD_GUCEF_AUDIO_PLUGIN_DLL
  #define GUCEF_AUDIO_PLUGIN_EXPORTSPEC GUCEF_EXPORT
#else
  #define GUCEF_AUDIO_PLUGIN_EXPORTSPEC GUCEF_IMPORT
#endif

/*-------------------------------------------------------------------------*/

#undef GUCEF_AUDIO_PUBLIC_CPP
#ifdef GUCEF_AUDIO_PUBLIC_CPP_CODE
  #define GUCEF_AUDIO_PUBLIC_CPP GUCEF_AUDIO_EXPORTSPEC
#else
  #define GUCEF_AUDIO_PUBLIC_CPP GUCEF_HIDDEN
#endif /* GUCEF_AUDIO_PUBLIC_CPP_CODE */

#undef GUCEF_AUDIO_PUBLIC_C
#ifdef GUCEF_AUDIO_PUBLIC_C_CODE
  #define GUCEF_AUDIO_PUBLIC_C GUCEF_AUDIO_EXPORTSPEC
#else
  #define GUCEF_AUDIO_PUBLIC_C GUCEF_HIDDEN
#endif /* GUCEF_AUDIO_PUBLIC_C_CODE */

#define GUCEF_AUDIO_PRIVATE_CPP GUCEF_HIDDEN
#define GUCEF_AUDIO_PRIVATE_C GUCEF_HIDDEN

/*-------------------------------------------------------------------------*/

/*
 *      Marks a function as running in a real time context (audio thread: no allocation, no locks, no blocking calls).
 *      Placed after the declarator of the function definition. Only has an effect in a build with the Clang
 *      RealtimeSanitizer (-fsanitize=realtime), which then reports every unsafe call made while the function runs.
 *      Other compilers and builds ignore it.
 */
#undef GUCEF_AUDIO_REALTIME_CONTEXT
#if defined( __clang__ ) && defined( __has_feature )
  #if __has_feature( realtime_sanitizer )
    #define GUCEF_AUDIO_REALTIME_CONTEXT __attribute__(( nonblocking ))
  #endif
#endif
#ifndef GUCEF_AUDIO_REALTIME_CONTEXT
  #define GUCEF_AUDIO_REALTIME_CONTEXT
#endif /* GUCEF_AUDIO_REALTIME_CONTEXT ? */

/*-------------------------------------------------------------------------*/

#undef GUCEF_AUDIO_PLUGIN_PUBLIC_CPP
#ifdef GUCEF_AUDIO_PLUGIN_PUBLIC_CPP_CODE
  #define GUCEF_AUDIO_PLUGIN_PUBLIC_CPP GUCEF_AUDIO_PLUGIN_EXPORTSPEC
#else
  #define GUCEF_AUDIO_PLUGIN_PUBLIC_CPP GUCEF_HIDDEN
#endif /* GUCEF_AUDIO_PLUGIN_PUBLIC_CPP_CODE */

#undef GUCEF_AUDIO_PLUGIN_PUBLIC_C
#ifdef GUCEF_AUDIO_PLUGIN_PUBLIC_C_CODE
  #define GUCEF_AUDIO_PLUGIN_PUBLIC_C GUCEF_AUDIO_PLUGIN_EXPORTSPEC
#else
  #define GUCEF_AUDIO_PLUGIN_PUBLIC_C GUCEF_HIDDEN
#endif /* GUCEF_AUDIO_PLUGIN_PUBLIC_C_CODE */

#define GUCEF_AUDIO_PLUGIN_PRIVATE_CPP GUCEF_HIDDEN
#define GUCEF_AUDIO_PLUGIN_PRIVATE_C GUCEF_HIDDEN

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIO_MACROS_H ? */
