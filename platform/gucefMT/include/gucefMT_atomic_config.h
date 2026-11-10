/*
 *  gucefMT: GUCEF module providing multithreading solutions
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

#ifndef GUCEF_MT_ATOMIC_CONFIG_H
#define GUCEF_MT_ATOMIC_CONFIG_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CONFIG_H
#include "gucef_config.h"           /* include the GUCEF common config */
#define GUCEF_CONFIG_H
#endif /* GUCEF_CONFIG_H ? */

#ifndef GUCEF_MACROS_H
#include "gucef_macros.h"           /* include the GUCEF common macros */
#define GUCEF_MACROS_H
#endif /* GUCEF_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      BACKEND DETECTION                                                  //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *  The atomic operations of gucefMT (see gucefMT_CTAtomic.h) are implemented by interchangeable
 *  backends so that every compiler and C++ version that GUCEF supports gets the best operations
 *  that it has. The backends, in order of preference for the default:
 *
 *    STD          C++11 <atomic>.                                            GUCEF_MT_ATOMIC_HAVE_STD
 *    GCC_ATOMIC   __atomic_* builtins (GCC 4.7+, Clang 3.1+), usable in C++98/03 mode.  GUCEF_MT_ATOMIC_HAVE_GCC_ATOMIC
 *    MSVC         _Interlocked* intrinsics (Visual Studio 2005+).             GUCEF_MT_ATOMIC_HAVE_MSVC
 *    GCC_SYNC     legacy __sync_* builtins (GCC 4.1+), full barriers only.   GUCEF_MT_ATOMIC_HAVE_GCC_SYNC
 *    LOCK         striped mutexes, always available, not lock free.          always
 *
 *  Defining GUCEF_MT_ATOMIC_NO_STD, GUCEF_MT_ATOMIC_NO_GCC_ATOMIC, GUCEF_MT_ATOMIC_NO_MSVC or
 *  GUCEF_MT_ATOMIC_NO_GCC_SYNC before including removes a backend from consideration.
 *  Backends that are available can all be used side by side by naming their tag explicitly
 *  (CTAtomic< T, SAtomicBackendGccSync >), which is how the tests cover every backend of a platform.
 */

#undef GUCEF_MT_ATOMIC_HAVE_STD
#undef GUCEF_MT_ATOMIC_HAVE_GCC_ATOMIC
#undef GUCEF_MT_ATOMIC_HAVE_GCC_SYNC
#undef GUCEF_MT_ATOMIC_HAVE_MSVC

/* C++11 <atomic>: needs a compiler and standard library new enough to carry a usable one */
#if defined( __cplusplus ) && !defined( GUCEF_MT_ATOMIC_NO_STD )
  #if ( __cplusplus >= 201103L ) || ( defined( _MSVC_LANG ) && ( _MSVC_LANG >= 201103L ) ) || ( defined( _MSC_VER ) && ( _MSC_VER >= 1700 ) )
    #if defined( __GNUC__ ) && !defined( __clang__ ) && ( ( __GNUC__ < 4 ) || ( ( __GNUC__ == 4 ) && ( __GNUC_MINOR__ < 8 ) ) )
      /* GCC before 4.8 has an incomplete <atomic> */
    #else
      #define GUCEF_MT_ATOMIC_HAVE_STD 1
      #if defined( __has_include )
        #if !__has_include( <atomic> )
          #undef GUCEF_MT_ATOMIC_HAVE_STD
        #endif
      #endif
    #endif
  #endif
#endif

/* GCC / Clang __atomic builtins, both define __ATOMIC_SEQ_CST when they have them */
#if defined( __GNUC__ ) && defined( __ATOMIC_SEQ_CST ) && !defined( GUCEF_MT_ATOMIC_NO_GCC_ATOMIC )
  #define GUCEF_MT_ATOMIC_HAVE_GCC_ATOMIC 1
#endif

/* Legacy GCC __sync builtins */
#if defined( __GNUC__ ) && !defined( GUCEF_MT_ATOMIC_NO_GCC_SYNC )
  #if defined( __clang__ ) || ( __GNUC__ > 4 ) || ( ( __GNUC__ == 4 ) && ( __GNUC_MINOR__ >= 1 ) )
    #define GUCEF_MT_ATOMIC_HAVE_GCC_SYNC 1
  #endif
#endif

/* Visual Studio intrinsics */
#if defined( _MSC_VER ) && ( _MSC_VER >= 1400 ) && !defined( GUCEF_MT_ATOMIC_NO_MSVC )
  #define GUCEF_MT_ATOMIC_HAVE_MSVC 1
#endif

/*
 *  Default backend. Standard library first because that is what the compiler vendors keep correct,
 *  then the compiler builtins, then the legacy ones, and the mutex backend if there is nothing.
 */
#undef GUCEF_MT_ATOMIC_DEFAULT_BACKEND_TAG
#undef GUCEF_MT_ATOMIC_DEFAULT_HAS_64BIT
#if defined( GUCEF_MT_ATOMIC_HAVE_STD )
  #define GUCEF_MT_ATOMIC_DEFAULT_BACKEND_TAG SAtomicBackendStd
  #define GUCEF_MT_ATOMIC_DEFAULT_HAS_64BIT 1
#elif defined( GUCEF_MT_ATOMIC_HAVE_GCC_ATOMIC )
  #define GUCEF_MT_ATOMIC_DEFAULT_BACKEND_TAG SAtomicBackendGccAtomic
  #define GUCEF_MT_ATOMIC_DEFAULT_HAS_64BIT 1
#elif defined( GUCEF_MT_ATOMIC_HAVE_MSVC )
  #define GUCEF_MT_ATOMIC_DEFAULT_BACKEND_TAG SAtomicBackendMsvc
  #define GUCEF_MT_ATOMIC_DEFAULT_HAS_64BIT 1
#elif defined( GUCEF_MT_ATOMIC_HAVE_GCC_SYNC )
  #define GUCEF_MT_ATOMIC_DEFAULT_BACKEND_TAG SAtomicBackendGccSync
  /* 8 byte __sync operations only exist where the target has a 64 bit compare and swap */
  #if defined( __GCC_HAVE_SYNC_COMPARE_AND_SWAP_8 ) || defined( __LP64__ ) || defined( _LP64 ) || defined( __x86_64__ ) || defined( __aarch64__ )
    #define GUCEF_MT_ATOMIC_DEFAULT_HAS_64BIT 1
  #endif
#else
  #define GUCEF_MT_ATOMIC_DEFAULT_BACKEND_TAG SAtomicBackendLock
#endif

/*-------------------------------------------------------------------------//
//                                                                         //
//      OTHER PLATFORM FACTS                                               //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Size in bytes of a cache line. Used to keep independently written atomics (for example the read and
 *  write index of a queue) on separate lines. 64 on x86 and most ARM, 128 on Apple silicon.
 */
#ifndef GUCEF_MT_CACHE_LINE_SIZE
  #if defined( __APPLE__ ) && defined( __aarch64__ )
    #define GUCEF_MT_CACHE_LINE_SIZE 128
  #else
    #define GUCEF_MT_CACHE_LINE_SIZE 64
  #endif
#endif

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MT_ATOMIC_CONFIG_H ? */
