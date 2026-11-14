/*
 *  gucefMATH: GUCEF module providing math: scalar, vector, matrix and numeric classes
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

#ifndef GUCEF_MATH_MACROS_H
#define GUCEF_MATH_MACROS_H

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

#ifndef GUCEF_MATH_ETYPES_H
#include "gucefMATH_ETypes.h"      /* simple types */
#define GUCEF_MATH_ETYPES_H
#endif /* GUCEF_MATH_ETYPES_H ? */

#ifndef GUCEF_MATH_CONFIG_H
#include "gucefMATH_config.h"      /* Module build configuration */
#define GUCEF_MATH_CONFIG_H
#endif /* GUCEF_MATH_CONFIG_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      GENERAL MACROS                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *      Auto detection of debug mode
 */
#ifndef GUCEF_MATH_DEBUG_MODE
  #if defined( _DEBUG ) || !defined( NDEBUG )
    #define GUCEF_MATH_DEBUG_MODE
  #endif /* compiler DEBUG switches */
#endif /* GUCEF_MATH_DEBUG_MODE ? */

/*-------------------------------------------------------------------------*/

#ifdef GUCEFMATH_BUILD_MODULE
  #define GUCEF_MATH_BUILD_MODULE
#endif /* GUCEFMATH_BUILD_MODULE ? */

/*
 *      When compiling the gucefMATH library you should define GUCEF_MATH_BUILD_MODULE
 *      before including this macro file.
 */
#undef GUCEF_MATH_EXPORTSPEC
#ifdef GUCEF_MATH_BUILD_MODULE
  #define GUCEF_MATH_EXPORTSPEC GUCEF_EXPORT
#else
  #define GUCEF_MATH_EXPORTSPEC GUCEF_IMPORT
#endif /* GUCEF_MATH_BUILD_MODULE ? */

/*-------------------------------------------------------------------------*/

#undef GUCEF_MATH_PUBLIC_CPP
#ifdef GUCEF_MATH_PUBLIC_CPP_CODE
  #define GUCEF_MATH_PUBLIC_CPP GUCEF_MATH_EXPORTSPEC
#else
  #define GUCEF_MATH_PUBLIC_CPP GUCEF_HIDDEN
#endif /* GUCEF_MATH_PUBLIC_CPP_CODE */

#undef GUCEF_MATH_PUBLIC_C
#ifdef GUCEF_MATH_PUBLIC_C_CODE
  #define GUCEF_MATH_PUBLIC_C GUCEF_MATH_EXPORTSPEC
#else
  #define GUCEF_MATH_PUBLIC_C GUCEF_HIDDEN
#endif /* GUCEF_MATH_PUBLIC_C_CODE */

#undef GUCEF_MATH_PRIVATE_CPP
#define GUCEF_MATH_PRIVATE_CPP GUCEF_HIDDEN
#undef GUCEF_MATH_PRIVATE_C
#define GUCEF_MATH_PRIVATE_C GUCEF_HIDDEN

/*-------------------------------------------------------------------------*/

/*
 *      Marks a function as running in a real time context (no allocation, no locks, no blocking calls).
 *      Placed after the declarator of the function definition. Only has an effect in a build with the Clang
 *      RealtimeSanitizer (-fsanitize=realtime), which then reports every unsafe call made while the function runs.
 *      Other compilers and builds ignore it.
 */
#undef GUCEF_MATH_REALTIME_CONTEXT
#if defined( __clang__ ) && defined( __has_feature )
  #if __has_feature( realtime_sanitizer )
    #define GUCEF_MATH_REALTIME_CONTEXT __attribute__(( nonblocking ))
  #endif
#endif
#ifndef GUCEF_MATH_REALTIME_CONTEXT
  #define GUCEF_MATH_REALTIME_CONTEXT
#endif /* GUCEF_MATH_REALTIME_CONTEXT ? */

/*-------------------------------------------------------------------------*/

/*
 *      Inlining that is forced where the compiler supports it
 */
#undef GUCEF_MATH_FORCE_INLINE
#if defined( _MSC_VER )
  #define GUCEF_MATH_FORCE_INLINE __forceinline
#elif defined( __GNUC__ ) || defined( __clang__ )
  #define GUCEF_MATH_FORCE_INLINE __attribute__(( always_inline )) inline
#else
  #define GUCEF_MATH_FORCE_INLINE inline
#endif

/*-------------------------------------------------------------------------*/

/*
 *      Compile time assertion. The name must be a valid identifier that is unique in its scope: it is what the
 *      compiler reports when the condition is false. The C++98 form is a typedef of an array with a negative
 *      size, a language standard that has static_assert uses that and carries the name in the message.
 */
#undef GUCEF_MATH_STATIC_ASSERT
#if defined( __cplusplus ) && ( __cplusplus >= 201103L )
  #define GUCEF_MATH_STATIC_ASSERT( condition, name )  static_assert( ( condition ), #name )
#elif defined( __GNUC__ ) || defined( __clang__ )
  /* the attribute keeps -Wunused-local-typedefs quiet for the C++98 form inside functions */
  #define GUCEF_MATH_STATIC_ASSERT( condition, name )  typedef char name[ ( condition ) ? 1 : -1 ] __attribute__(( unused ))
#else
  #define GUCEF_MATH_STATIC_ASSERT( condition, name )  typedef char name[ ( condition ) ? 1 : -1 ]
#endif

/*-------------------------------------------------------------------------*/

/*
 *      Assertion that only exists in debug mode
 */
#undef GUCEF_MATH_ASSERT
#ifdef GUCEF_MATH_DEBUG_MODE
  #define GUCEF_MATH_ASSERT( condition )  GUCEF_ASSERT( condition )
#else
  #define GUCEF_MATH_ASSERT( condition )
#endif /* GUCEF_MATH_DEBUG_MODE ? */

/*-------------------------------------------------------------------------*/

/*
 *      Exported instantiations of the predefined numeric kinds
 *
 *      gucefMATH explicitly instantiates its class templates for the predefined kinds (Float32, Float64, the integer
 *      types, the fixed-point formats and the fractions) inside the module, which is compiled with floating-point
 *      contraction disabled (-ffp-contract=off, /fp:precise). Users of those kinds link to the exported code instead of
 *      instantiating their own copy, so precision-sensitive functions give identical results in every build.
 *
 *      GUCEF_MATH_PRECISE_INLINE marks the out-of-class definitions of precision-sensitive member functions (dot
 *      products, lengths, normalisation, compensated sums, matrix and quaternion products). By default it is empty: the
 *      function is not inline, so for a predefined kind only the module's copy runs. Defining GUCEF_MATH_INLINE_ALL in a
 *      build makes these functions inline and drops the extern template declarations, trading reproducible floating-point
 *      results for inlining. Kinds that are not predefined are always instantiated by their user.
 *
 *      GUCEF_MATH_EXTERN_TEMPLATE_CLASS( declaration ) declares an exported instantiation, for example
 *      GUCEF_MATH_EXTERN_TEMPLATE_CLASS( CTVector3D< Float32 > ). The declaration must not contain a comma (use the
 *      typedef of a multi-parameter type). The matching definition in a module source file is
 *      GUCEF_MATH_INSTANTIATE_TEMPLATE_CLASS( declaration ). extern template is C++11; GCC, Clang and Microsoft Visual C++
 *      accept it as an extension in older language modes, other compilers then instantiate in the user's code.
 */
#undef GUCEF_MATH_PRECISE_INLINE
#ifdef GUCEF_MATH_INLINE_ALL
  #define GUCEF_MATH_PRECISE_INLINE inline
#else
  #define GUCEF_MATH_PRECISE_INLINE
#endif /* GUCEF_MATH_INLINE_ALL ? */

/*
 *      Symbol export of class template instantiations differs per toolchain:
 *      - Microsoft Windows (dllexport / dllimport): the specification goes on the explicit instantiation only. It must not
 *        go on the class template, since a dllimport template could not be instantiated by its user for other kinds.
 *      - ELF and Mach-O (GCC, Clang): a visibility attribute on an explicit instantiation of an already defined class is
 *        ignored, so the class template itself carries default visibility through GUCEF_MATH_TEMPLATE_PUBLIC
 *        ( template < typename T > class GUCEF_MATH_TEMPLATE_PUBLIC CTVector3D ). That is harmless for kinds a user
 *        instantiates and makes the module's instantiations visible despite -fvisibility=hidden.
 */
#undef GUCEF_MATH_TEMPLATE_PUBLIC
#undef GUCEF_MATH_INSTANTIATION_SPEC
#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
  #define GUCEF_MATH_TEMPLATE_PUBLIC
  #define GUCEF_MATH_INSTANTIATION_SPEC  GUCEF_MATH_PUBLIC_CPP
#elif defined( __GNUC__ ) || defined( __clang__ )
  #define GUCEF_MATH_TEMPLATE_PUBLIC  __attribute__(( visibility( "default" ) ))
  #define GUCEF_MATH_INSTANTIATION_SPEC
#else
  #define GUCEF_MATH_TEMPLATE_PUBLIC
  #define GUCEF_MATH_INSTANTIATION_SPEC
#endif

#undef GUCEF_MATH_EXTERN_TEMPLATE_CLASS
#if defined( GUCEF_MATH_INLINE_ALL )
  #define GUCEF_MATH_EXTERN_TEMPLATE_CLASS( declaration )
#elif ( defined( __cplusplus ) && ( __cplusplus >= 201103L ) ) || defined( _MSC_VER )
  #define GUCEF_MATH_EXTERN_TEMPLATE_CLASS( declaration )  extern template class GUCEF_MATH_INSTANTIATION_SPEC declaration;
#elif defined( __GNUC__ ) || defined( __clang__ )
  #define GUCEF_MATH_EXTERN_TEMPLATE_CLASS( declaration )  __extension__ extern template class GUCEF_MATH_INSTANTIATION_SPEC declaration;
#else
  #define GUCEF_MATH_EXTERN_TEMPLATE_CLASS( declaration )
#endif

/*
 *      For declarations that contain commas (function templates, multi-parameter types) use the prefix form inside
 *      #ifdef GUCEF_MATH_EXTERN_TEMPLATES_ENABLED:
 *          GUCEF_MATH_EXTERN_TEMPLATE Float32 Lerp< Float32 >( Float32, Float32, Float32 );
 *      and GUCEF_MATH_INSTANTIATE_TEMPLATE for the definition in the module source.
 */
#undef GUCEF_MATH_EXTERN_TEMPLATES_ENABLED
#undef GUCEF_MATH_EXTERN_TEMPLATE
#if !defined( GUCEF_MATH_INLINE_ALL ) && ( ( defined( __cplusplus ) && ( __cplusplus >= 201103L ) ) || defined( _MSC_VER ) )
  #define GUCEF_MATH_EXTERN_TEMPLATES_ENABLED
  #define GUCEF_MATH_EXTERN_TEMPLATE  extern template GUCEF_MATH_PUBLIC_CPP
#elif !defined( GUCEF_MATH_INLINE_ALL ) && ( defined( __GNUC__ ) || defined( __clang__ ) )
  #define GUCEF_MATH_EXTERN_TEMPLATES_ENABLED
  #define GUCEF_MATH_EXTERN_TEMPLATE  __extension__ extern template GUCEF_MATH_PUBLIC_CPP
#endif

#undef GUCEF_MATH_INSTANTIATE_TEMPLATE
#define GUCEF_MATH_INSTANTIATE_TEMPLATE  template GUCEF_MATH_PUBLIC_CPP

#undef GUCEF_MATH_INSTANTIATE_TEMPLATE_CLASS
#define GUCEF_MATH_INSTANTIATE_TEMPLATE_CLASS( declaration )  template class GUCEF_MATH_INSTANTIATION_SPEC declaration;

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MATH_MACROS_H ? */
