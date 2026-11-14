/*
 *  gucefGEOMETRY: GUCEF module providing geometry: shapes, spaces and spatial queries
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

#ifndef GUCEF_GEOMETRY_MACROS_H
#define GUCEF_GEOMETRY_MACROS_H

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

#ifndef GUCEF_GEOMETRY_ETYPES_H
#include "gucefGEOMETRY_ETypes.h"      /* simple types */
#define GUCEF_GEOMETRY_ETYPES_H
#endif /* GUCEF_GEOMETRY_ETYPES_H ? */

#ifndef GUCEF_GEOMETRY_CONFIG_H
#include "gucefGEOMETRY_config.h"      /* Module build configuration */
#define GUCEF_GEOMETRY_CONFIG_H
#endif /* GUCEF_GEOMETRY_CONFIG_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      GENERAL MACROS                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *      Auto detection of debug mode
 */
#ifndef GUCEF_GEOMETRY_DEBUG_MODE
  #if defined( _DEBUG ) || !defined( NDEBUG )
    #define GUCEF_GEOMETRY_DEBUG_MODE
  #endif /* compiler DEBUG switches */
#endif /* GUCEF_GEOMETRY_DEBUG_MODE ? */

/*-------------------------------------------------------------------------*/

#ifdef GUCEFGEOMETRY_BUILD_MODULE
  #define GUCEF_GEOMETRY_BUILD_MODULE
#endif /* GUCEFGEOMETRY_BUILD_MODULE ? */

/*
 *      When compiling the gucefGEOMETRY library you should define GUCEF_GEOMETRY_BUILD_MODULE
 *      before including this macro file.
 */
#undef GUCEF_GEOMETRY_EXPORTSPEC
#ifdef GUCEF_GEOMETRY_BUILD_MODULE
  #define GUCEF_GEOMETRY_EXPORTSPEC GUCEF_EXPORT
#else
  #define GUCEF_GEOMETRY_EXPORTSPEC GUCEF_IMPORT
#endif /* GUCEF_GEOMETRY_BUILD_MODULE ? */

/*-------------------------------------------------------------------------*/

#undef GUCEF_GEOMETRY_PUBLIC_CPP
#ifdef GUCEF_GEOMETRY_PUBLIC_CPP_CODE
  #define GUCEF_GEOMETRY_PUBLIC_CPP GUCEF_GEOMETRY_EXPORTSPEC
#else
  #define GUCEF_GEOMETRY_PUBLIC_CPP GUCEF_HIDDEN
#endif /* GUCEF_GEOMETRY_PUBLIC_CPP_CODE */

#undef GUCEF_GEOMETRY_PUBLIC_C
#ifdef GUCEF_GEOMETRY_PUBLIC_C_CODE
  #define GUCEF_GEOMETRY_PUBLIC_C GUCEF_GEOMETRY_EXPORTSPEC
#else
  #define GUCEF_GEOMETRY_PUBLIC_C GUCEF_HIDDEN
#endif /* GUCEF_GEOMETRY_PUBLIC_C_CODE */

#undef GUCEF_GEOMETRY_PRIVATE_CPP
#define GUCEF_GEOMETRY_PRIVATE_CPP GUCEF_HIDDEN
#undef GUCEF_GEOMETRY_PRIVATE_C
#define GUCEF_GEOMETRY_PRIVATE_C GUCEF_HIDDEN

/*-------------------------------------------------------------------------*/

/*
 *      Marks a function as running in a real time context (no allocation, no locks, no blocking calls).
 *      Placed after the declarator of the function definition. Only has an effect in a build with the Clang
 *      RealtimeSanitizer (-fsanitize=realtime), which then reports every unsafe call made while the function runs.
 *      Other compilers and builds ignore it.
 */
#undef GUCEF_GEOMETRY_REALTIME_CONTEXT
#if defined( __clang__ ) && defined( __has_feature )
  #if __has_feature( realtime_sanitizer )
    #define GUCEF_GEOMETRY_REALTIME_CONTEXT __attribute__(( nonblocking ))
  #endif
#endif
#ifndef GUCEF_GEOMETRY_REALTIME_CONTEXT
  #define GUCEF_GEOMETRY_REALTIME_CONTEXT
#endif /* GUCEF_GEOMETRY_REALTIME_CONTEXT ? */

/*-------------------------------------------------------------------------*/

/*
 *      Inlining that is forced where the compiler supports it
 */
#undef GUCEF_GEOMETRY_FORCE_INLINE
#if defined( _MSC_VER )
  #define GUCEF_GEOMETRY_FORCE_INLINE __forceinline
#elif defined( __GNUC__ ) || defined( __clang__ )
  #define GUCEF_GEOMETRY_FORCE_INLINE __attribute__(( always_inline )) inline
#else
  #define GUCEF_GEOMETRY_FORCE_INLINE inline
#endif

/*-------------------------------------------------------------------------*/

/*
 *      Compile time assertion. The name must be a valid identifier that is unique in its scope: it is what the
 *      compiler reports when the condition is false. The C++98 form is a typedef of an array with a negative
 *      size, a language standard that has static_assert uses that and carries the name in the message.
 */
#undef GUCEF_GEOMETRY_STATIC_ASSERT
#if defined( __cplusplus ) && ( __cplusplus >= 201103L )
  #define GUCEF_GEOMETRY_STATIC_ASSERT( condition, name )  static_assert( ( condition ), #name )
#else
  #define GUCEF_GEOMETRY_STATIC_ASSERT( condition, name )  typedef char name[ ( condition ) ? 1 : -1 ]
#endif

/*-------------------------------------------------------------------------*/

/*
 *      Assertion that only exists in debug mode
 */
#undef GUCEF_GEOMETRY_ASSERT
#ifdef GUCEF_GEOMETRY_DEBUG_MODE
  #define GUCEF_GEOMETRY_ASSERT( condition )  GUCEF_ASSERT( condition )
#else
  #define GUCEF_GEOMETRY_ASSERT( condition )
#endif /* GUCEF_GEOMETRY_DEBUG_MODE ? */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GEOMETRY_MACROS_H ? */
