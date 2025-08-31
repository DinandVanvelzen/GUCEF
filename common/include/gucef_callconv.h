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

#ifndef GUCEF_CALLCONV_H
#define GUCEF_CALLCONV_H

/*
 *      calling convention macros for the GUCEF platform
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
#include "gucef_platform.h"      /* GUCEF platform compilation targets */
#define GUCEF_PLATFORM_H
#endif /* GUCEF_PLATFORM_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *      Change calling convention notation depending on the compiler used.
 */
#if defined ( __BORLANDC__ ) || defined ( _MSC_VER )
  #define GUCEF_CALLSPEC_C __cdecl
#elif defined( __EMSCRIPTEN__ ) || defined( __wasm__ )
    /* Emscripten/WebAssembly does not support cdecl */
    #define GUCEF_CALLSPEC_C
#else
  #define GUCEF_CALLSPEC_C cdecl
#endif
#if defined ( __BORLANDC__ ) || defined ( _MSC_VER )
  #define GUCEF_CALLSPEC_STD __stdcall
#elif defined( __EMSCRIPTEN__ ) || defined( __wasm__ )
    /* Emscripten/WebAssembly does not support stdcall */
    #define GUCEF_CALLSPEC_STD
#else
  #define GUCEF_CALLSPEC_STD stdcall
#endif
#if defined ( __BORLANDC__ ) || defined ( _MSC_VER )
  #define GUCEF_CALLSPEC_PASCAL __pascal
#elif defined( __EMSCRIPTEN__ ) || defined( __wasm__ )
    /* Emscripten/WebAssembly does not support pascal */
    #define GUCEF_CALLSPEC_PASCAL
#else
  #define GUCEF_CALLSPEC_PASCAL pascal
#endif
#if defined ( __BORLANDC__ ) || defined ( _MSC_VER )
  #define GUCEF_CALLSPEC_FASTCALL __fastcall
#elif defined( __EMSCRIPTEN__ ) || defined( __wasm__ )
    /* Emscripten/WebAssembly does not support fastcall */
    #define GUCEF_CALLSPEC_FASTCALL
#else
  #define GUCEF_CALLSPEC_FASTCALL fastcall
#endif
#if defined ( __BORLANDC__ ) || defined ( _MSC_VER )
  #define GUCEF_CALLSPEC_VECTORCALL __vectorcall
#elif defined( __EMSCRIPTEN__ ) || defined( __wasm__ )
    /* Emscripten/WebAssembly does not support vectorcall */
    #define GUCEF_CALLSPEC_VECTORCALL
#else
  #define GUCEF_CALLSPEC_VECTORCALL vectorcall
#endif

/*
 *  Macro that switches to the desired calling convention
 *  for GUCEF modules.
 *     Default GUCEF_CALLSPEC_C for 32 bit and below
 *     Default GUCEF_CALLSPEC_FASTCALL for 64 bit but compilers don't like
 *     it if you specify that since it gives the illusion of choice for x64
 */
#undef GUCEF_CALLSPEC_TYPE
#ifdef GUCEF_USE_CALLSPEC_C
  #define GUCEF_CALLSPEC_TYPE GUCEF_CALLSPEC_C
#else
  #ifdef GUCEF_USE_CALLSPEC_STD
    #define GUCEF_CALLSPEC_TYPE GUCEF_CALLSPEC_STD
  #else
    #ifdef GUCEF_USE_CALLSPEC_PASCAL
      #define GUCEF_CALLSPEC_TYPE GUCEF_CALLSPEC_PASCAL
    #else
       #ifdef GUCEF_USE_CALLSPEC_FASTCALL
         #define GUCEF_CALLSPEC_TYPE GUCEF_CALLSPEC_FASTCALL
       #else
         #if ( GUCEF_BITNESS == GUCEF_BITNESS_32 )
           #define GUCEF_CALLSPEC_TYPE GUCEF_CALLSPEC_C
         #elif ( GUCEF_BITNESS == GUCEF_BITNESS_64 )
           #define GUCEF_CALLSPEC_TYPE
         #else
           #define GUCEF_CALLSPEC_TYPE GUCEF_CALLSPEC_C
         #endif
       #endif
     #endif
  #endif
#endif

/*
 *  Macro that switches to the desired calling convention
 *  for GUCEF plugin modules.
 *     Default GUCEF_CALLSPEC_STD for 32 bit and below
 *     Default GUCEF_CALLSPEC_FASTCALL for 64 bit but compilers don't like
 *     it if you specify that since it gives the illusion of choice for x64
 */
#undef GUCEF_PLUGIN_CALLSPEC_TYPE
#ifdef GUCEF_USE_PLUGIN_CALLSPEC_C
  #define GUCEF_PLUGIN_CALLSPEC_TYPE GUCEF_CALLSPEC_C
#else
  #ifdef GUCEF_USE_PLUGIN_CALLSPEC_STD
    #define GUCEF_PLUGIN_CALLSPEC_TYPE GUCEF_CALLSPEC_STD
  #else
    #ifdef GUCEF_USE_PLUGIN_CALLSPEC_PASCAL
      #define GUCEF_PLUGIN_CALLSPEC_TYPE GUCEF_CALLSPEC_PASCAL
    #else
       #ifdef GUCEF_USE_PLUGIN_CALLSPEC_FAST
         #define GUCEF_PLUGIN_CALLSPEC_TYPE GUCEF_CALLSPEC_FAST
       #else
         #if ( GUCEF_BITNESS == GUCEF_BITNESS_32 )
           #define GUCEF_PLUGIN_CALLSPEC_TYPE GUCEF_CALLSPEC_STD
         #elif ( GUCEF_BITNESS == GUCEF_BITNESS_64 )
           #define GUCEF_PLUGIN_CALLSPEC_TYPE
         #else
           #define GUCEF_PLUGIN_CALLSPEC_TYPE GUCEF_CALLSPEC_STD
         #endif
       #endif
     #endif
  #endif
#endif

/*
 *      Macro that switches between a calling convention prefix and
 *      postfix notation for the calling convention macro.
 */
#undef GUCEF_CALLSPEC_PREFIX
#undef GUCEF_CALLSPEC_SUFFIX
#undef GUCEF_PLUGIN_CALLSPEC_PREFIX
#undef GUCEF_PLUGIN_CALLSPEC_SUFFIX
#if defined ( __BORLANDC__ ) || defined ( _MSC_VER )
  #define GUCEF_CALLSPEC_PREFIX GUCEF_CALLSPEC_TYPE
  #define GUCEF_CALLSPEC_SUFFIX
  #define GUCEF_PLUGIN_CALLSPEC_PREFIX GUCEF_PLUGIN_CALLSPEC_TYPE
  #define GUCEF_PLUGIN_CALLSPEC_SUFFIX
#elif ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCIPTEN )
  #define GUCEF_CALLSPEC_PREFIX
  #define GUCEF_CALLSPEC_SUFFIX
  #define GUCEF_PLUGIN_CALLSPEC_PREFIX
  #define GUCEF_PLUGIN_CALLSPEC_SUFFIX
#else
  #define GUCEF_CALLSPEC_PREFIX __attribute__((GUCEF_CALLSPEC_TYPE))
  #define GUCEF_CALLSPEC_SUFFIX
  #define GUCEF_PLUGIN_CALLSPEC_PREFIX __attribute__((GUCEF_PLUGIN_CALLSPEC_TYPE))
  #define GUCEF_PLUGIN_CALLSPEC_SUFFIX
#endif


/*
 *      Macro that switches between a calling convention prefix and
 *      postfix notation for the calling convention macro for the std
 *      calling convention
 */
#undef GUCEF_CALLSPEC_STD_PREFIX
#undef GUCEF_CALLSPEC_STD_SUFFIX
#undef GUCEF_PLUGIN_CALLSPEC_STD_PREFIX
#undef GUCEF_PLUGIN_CALLSPEC_STD_SUFFIX
#if defined ( __BORLANDC__ ) || defined ( _MSC_VER )
  #define GUCEF_CALLSPEC_STD_PREFIX GUCEF_CALLSPEC_STD
  #define GUCEF_CALLSPEC_STD_SUFFIX
  #define GUCEF_PLUGIN_CALLSPEC_STD_PREFIX GUCEF_CALLSPEC_STD
  #define GUCEF_PLUGIN_CALLSPEC_STD_SUFFIX
#elif ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID )
  #define GUCEF_CALLSPEC_STD_PREFIX
  #define GUCEF_CALLSPEC_STD_SUFFIX
  #define GUCEF_PLUGIN_CALLSPEC_STD_PREFIX
  #define GUCEF_PLUGIN_CALLSPEC_STD_SUFFIX
#else
  #define GUCEF_CALLSPEC_STD_PREFIX __attribute__((GUCEF_CALLSPEC_STD))
  #define GUCEF_CALLSPEC_STD_SUFFIX
  #define GUCEF_PLUGIN_CALLSPEC_STD_PREFIX __attribute__((GUCEF_CALLSPEC_STD))
  #define GUCEF_PLUGIN_CALLSPEC_STD_SUFFIX
#endif

/*
 *      Macro that switches between a calling convention prefix and
 *      postfix notation for the calling convention macro for the c
 *      calling convention
 */
#undef GUCEF_CALLSPEC_C_PREFIX
#undef GUCEF_CALLSPEC_C_SUFFIX
#undef GUCEF_PLUGIN_CALLSPEC_C_PREFIX
#undef GUCEF_PLUGIN_CALLSPEC_C_SUFFIX
#if defined ( __BORLANDC__ ) || defined ( _MSC_VER )
  #define GUCEF_CALLSPEC_C_PREFIX GUCEF_CALLSPEC_C
  #define GUCEF_CALLSPEC_C_SUFFIX
  #define GUCEF_PLUGIN_CALLSPEC_C_PREFIX GUCEF_CALLSPEC_C
  #define GUCEF_PLUGIN_CALLSPEC_C_SUFFIX
#elif ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID )
  #define GUCEF_CALLSPEC_C_PREFIX
  #define GUCEF_CALLSPEC_C_SUFFIX
  #define GUCEF_PLUGIN_CALLSPEC_C_PREFIX
  #define GUCEF_PLUGIN_CALLSPEC_C_SUFFIX
#else
  #define GUCEF_CALLSPEC_C_PREFIX __attribute__((GUCEF_CALLSPEC_C))
  #define GUCEF_CALLSPEC_C_SUFFIX
  #define GUCEF_PLUGIN_CALLSPEC_C_PREFIX __attribute__((GUCEF_CALLSPEC_C))
  #define GUCEF_PLUGIN_CALLSPEC_C_SUFFIX
#endif

/*
 *      Macro that switches between a calling convention prefix and
 *      postfix notation for the calling convention macro for the default
 *      calling convention
 */
#undef GUCEF_CALLSPEC_PREFIX
#undef GUCEF_CALLSPEC_SUFFIX
#undef GUCEF_PLUGIN_CALLSPEC_PREFIX
#undef GUCEF_PLUGIN_CALLSPEC_SUFFIX
#if ( GUCEF_BITNESS != GUCEF_BITNESS_64 )
  #if defined ( __BORLANDC__ ) || defined ( _MSC_VER )
    #define GUCEF_CALLSPEC_PREFIX GUCEF_CALLSPEC_TYPE
    #define GUCEF_CALLSPEC_SUFFIX
    #define GUCEF_PLUGIN_CALLSPEC_PREFIX GUCEF_CALLSPEC_TYPE
    #define GUCEF_PLUGIN_CALLSPEC_SUFFIX
  #elif ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID )
    #define GUCEF_CALLSPEC_PREFIX
    #define GUCEF_CALLSPEC_SUFFIX
    #define GUCEF_PLUGIN_CALLSPEC_PREFIX
    #define GUCEF_PLUGIN_CALLSPEC_SUFFIX
  #else
    #define GUCEF_CALLSPEC_PREFIX __attribute__((GUCEF_CALLSPEC_TYPE))
    #define GUCEF_CALLSPEC_SUFFIX
    #define GUCEF_PLUGIN_CALLSPEC_PREFIX __attribute__((GUCEF_PLUGIN_CALLSPEC_TYPE))
    #define GUCEF_PLUGIN_CALLSPEC_SUFFIX
  #endif
#else
  #define GUCEF_CALLSPEC_PREFIX
  #define GUCEF_CALLSPEC_SUFFIX
  #define GUCEF_PLUGIN_CALLSPEC_PREFIX
  #define GUCEF_PLUGIN_CALLSPEC_SUFFIX
#endif

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CALLCONV_H ? */
