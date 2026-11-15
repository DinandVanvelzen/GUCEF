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
#elif defined( __GNUC__ ) || defined( __clang__ )
  /* the attribute keeps -Wunused-local-typedefs quiet for the C++98 form inside functions */
  #define GUCEF_GEOMETRY_STATIC_ASSERT( condition, name )  typedef char name[ ( condition ) ? 1 : -1 ] __attribute__(( unused ))
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

/*
 *      Exported instantiations of the predefined numeric kinds
 *
 *      gucefGEOMETRY explicitly instantiates its shape class templates for the predefined kinds (Float32, Float64, Int32
 *      for the integer ranges) inside the module, which is compiled with floating-point contraction disabled
 *      (-ffp-contract=off, /fp:precise). Users of those kinds link to the exported code instead of instantiating their own
 *      copy, so precision-sensitive functions give identical results in every build. The semantics of these macros are
 *      those of their GUCEF_MATH_* counterparts (see gucefMATH_macros.h).
 *
 *      GUCEF_GEOMETRY_PRECISE_INLINE marks the out-of-class definitions of precision-sensitive member functions. By
 *      default it is empty: the function is not inline, so for a predefined kind only the module's copy runs.
 *      Defining GUCEF_GEOMETRY_INLINE_ALL in a build makes these functions inline and drops the extern template
 *      declarations. GUCEF_GEOMETRY_INLINE_ALL is implied by GUCEF_MATH_INLINE_ALL, so one define switches both modules;
 *      defining only GUCEF_GEOMETRY_INLINE_ALL leaves gucefMATH untouched.
 *
 *      GUCEF_GEOMETRY_EXTERN_TEMPLATE_CLASS( declaration ) declares an exported instantiation, for example
 *      GUCEF_GEOMETRY_EXTERN_TEMPLATE_CLASS( CTSphere< Float32 > ). The declaration must not contain a comma. The matching
 *      definition in a module source file is GUCEF_GEOMETRY_INSTANTIATE_TEMPLATE_CLASS( declaration ).
 */
#if defined( GUCEF_MATH_INLINE_ALL ) && !defined( GUCEF_GEOMETRY_INLINE_ALL )
  #define GUCEF_GEOMETRY_INLINE_ALL
#endif

#undef GUCEF_GEOMETRY_PRECISE_INLINE
#ifdef GUCEF_GEOMETRY_INLINE_ALL
  #define GUCEF_GEOMETRY_PRECISE_INLINE inline
#else
  #define GUCEF_GEOMETRY_PRECISE_INLINE
#endif /* GUCEF_GEOMETRY_INLINE_ALL ? */

/*
 *      Symbol export of class template instantiations: see GUCEF_MATH_TEMPLATE_PUBLIC in gucefMATH_macros.h.
 */
#undef GUCEF_GEOMETRY_TEMPLATE_PUBLIC
#undef GUCEF_GEOMETRY_INSTANTIATION_SPEC
#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
  #define GUCEF_GEOMETRY_TEMPLATE_PUBLIC
  #define GUCEF_GEOMETRY_INSTANTIATION_SPEC  GUCEF_GEOMETRY_PUBLIC_CPP
#elif defined( __GNUC__ ) || defined( __clang__ )
  #define GUCEF_GEOMETRY_TEMPLATE_PUBLIC  __attribute__(( visibility( "default" ) ))
  #define GUCEF_GEOMETRY_INSTANTIATION_SPEC
#else
  #define GUCEF_GEOMETRY_TEMPLATE_PUBLIC
  #define GUCEF_GEOMETRY_INSTANTIATION_SPEC
#endif

#undef GUCEF_GEOMETRY_EXTERN_TEMPLATE_CLASS
#if defined( GUCEF_GEOMETRY_INLINE_ALL )
  #define GUCEF_GEOMETRY_EXTERN_TEMPLATE_CLASS( declaration )
#elif ( defined( __cplusplus ) && ( __cplusplus >= 201103L ) ) || defined( _MSC_VER )
  #define GUCEF_GEOMETRY_EXTERN_TEMPLATE_CLASS( declaration )  extern template class GUCEF_GEOMETRY_INSTANTIATION_SPEC declaration;
#elif defined( __GNUC__ ) || defined( __clang__ )
  #define GUCEF_GEOMETRY_EXTERN_TEMPLATE_CLASS( declaration )  __extension__ extern template class GUCEF_GEOMETRY_INSTANTIATION_SPEC declaration;
#else
  #define GUCEF_GEOMETRY_EXTERN_TEMPLATE_CLASS( declaration )
#endif

/*
 *      Prefix form for declarations that contain commas (member-wise instantiation), inside
 *      #ifdef GUCEF_GEOMETRY_EXTERN_TEMPLATES_ENABLED, with GUCEF_GEOMETRY_INSTANTIATE_TEMPLATE for the definition.
 */
#undef GUCEF_GEOMETRY_EXTERN_TEMPLATES_ENABLED
#undef GUCEF_GEOMETRY_EXTERN_TEMPLATE
#if !defined( GUCEF_GEOMETRY_INLINE_ALL ) && ( ( defined( __cplusplus ) && ( __cplusplus >= 201103L ) ) || defined( _MSC_VER ) )
  #define GUCEF_GEOMETRY_EXTERN_TEMPLATES_ENABLED
  #define GUCEF_GEOMETRY_EXTERN_TEMPLATE  extern template GUCEF_GEOMETRY_PUBLIC_CPP
#elif !defined( GUCEF_GEOMETRY_INLINE_ALL ) && ( defined( __GNUC__ ) || defined( __clang__ ) )
  #define GUCEF_GEOMETRY_EXTERN_TEMPLATES_ENABLED
  #define GUCEF_GEOMETRY_EXTERN_TEMPLATE  __extension__ extern template GUCEF_GEOMETRY_PUBLIC_CPP
#endif

#undef GUCEF_GEOMETRY_INSTANTIATE_TEMPLATE
#define GUCEF_GEOMETRY_INSTANTIATE_TEMPLATE  template GUCEF_GEOMETRY_PUBLIC_CPP

#undef GUCEF_GEOMETRY_INSTANTIATE_TEMPLATE_CLASS
#define GUCEF_GEOMETRY_INSTANTIATE_TEMPLATE_CLASS( declaration )  template class GUCEF_GEOMETRY_INSTANTIATION_SPEC declaration;

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GEOMETRY_MACROS_H ? */
