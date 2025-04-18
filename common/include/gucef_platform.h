/*
 *  GUCEF platform wide macros/defines for OS/Compiler specifics
 *  Copyright (C) 2002 - 2011.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#ifndef GUCEF_PLATFORM_H
#define GUCEF_PLATFORM_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CONFIG_H
#include "gucef_config.h"
#define GUCEF_CONFIG_H
#endif /* GUCEF_CONFIG_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *  Platform defines
 */
#define GUCEF_PLATFORM_MSWIN            1
#define GUCEF_PLATFORM_LINUX            2
#define GUCEF_PLATFORM_APPLE_MACOS      3
#define GUCEF_PLATFORM_ANDROID          4
#define GUCEF_PLATFORM_APPLE_IPHONEOS   5
#define GUCEF_PLATFORM_APPLE_IPADOS     6
#define GUCEF_PLATFORM_APPLE_TVOS       7
#define GUCEF_PLATFORM_APPLE_VISIONOS   8
#define GUCEF_PLATFORM_ARDUINO          9

/*
 *  Legacy platform define remaps
 *  These are deprecated
 */
#define GUCEF_PLATFORM_MACOS            GUCEF_PLATFORM_APPLE_MACOS
#define GUCEF_PLATFORM_IPHONEOS         GUCEF_PLATFORM_APPLE_IPHONEOS

/*
 *  Compiler defines
 */
#define GUCEF_COMPILER_MSVC     1
#define GUCEF_COMPILER_GNUC     2
#define GUCEF_COMPILER_BORL     3
#define GUCEF_COMPILER_CLANG    4

#define GUCEF_BYTEORDER_UNKNOWN_ENDIAN  0
#define GUCEF_BYTEORDER_LITTLE_ENDIAN   1
#define GUCEF_BYTEORDER_BIG_ENDIAN      2

/*
 *  CPU bitness defines
 */
#define GUCEF_CPU_BITNESS_8     4
#define GUCEF_CPU_BITNESS_16    3
#define GUCEF_CPU_BITNESS_32    1
#define GUCEF_CPU_BITNESS_64    2

/*
 *  Legacy cpu bitness define remaps
 *  These are deprecated
 */
#define GUCEF_ARCHITECTURE_32       GUCEF_CPU_BITNESS_32
#define GUCEF_ARCHITECTURE_64       GUCEF_CPU_BITNESS_64

/*
 *  CPU Architecture defines
 */
#define GUCEF_CPU_ARCHITECTURE_ARM          1
#define GUCEF_CPU_ARCHITECTURE_ARM64        2
#define GUCEF_CPU_ARCHITECTURE_X86          3
#define GUCEF_CPU_ARCHITECTURE_AMD64        4
#define GUCEF_CPU_ARCHITECTURE_ATMEGA328P   5
#define GUCEF_CPU_ARCHITECTURE_ATMEGA2560   6
#define GUCEF_CPU_ARCHITECTURE_ATMEGA4809   7


/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#if defined( __GNUG__ ) && !defined( GCC_VERSION ) /* GNU C++ compiler */
    #define GCC_VERSION (__GNUC__ * 10000 \
                         + __GNUC_MINOR__ * 100 \
                         + __GNUC_PATCHLEVEL__)
#endif

#if defined( __clang__ ) && !defined( CLANG_VERSION ) /* Clang C++ compiler */
    #define CLANG_VERSION (__clang_major__ * 10000 \
                         + __clang_minor__ * 100 \
                         + __clang_patchlevel__)
#endif

/* Finds the compiler type and version.
*/
#if defined( _MSC_VER )
#   define GUCEF_COMPILER GUCEF_COMPILER_MSVC
#   define GUCEF_COMP_VER _MSC_VER
#   ifdef _M_ARM
#       define GUCEF_CPU_ARCHITECTURE GUCEF_CPU_ARCHITECTURE_ARM
#   elif _M_ARM64
#       define GUCEF_CPU_ARCHITECTURE GUCEF_CPU_ARCHITECTURE_ARM64
#   elif _M_AMD64
#       define GUCEF_CPU_ARCHITECTURE GUCEF_CPU_ARCHITECTURE_AMD64
#   elif _M_IX86
#       define GUCEF_CPU_ARCHITECTURE GUCEF_CPU_ARCHITECTURE_X86
#   endif
#elif defined( __clang__ )
#   define GUCEF_COMPILER GUCEF_COMPILER_CLANG
#   define GUCEF_COMP_VER CLANG_VERSION
#   if defined( __i386__ ) || defined( __i486__ ) || defined( __i586__ ) || defined( __i686__ )
#       define GUCEF_CPU_ARCHITECTURE GUCEF_CPU_ARCHITECTURE_X86
#   elif defined( __amd64__ ) || defined( __amd64 ) || defined( __x86_64__ ) || defined( __x86_64 )
#       define GUCEF_CPU_ARCHITECTURE GUCEF_CPU_ARCHITECTURE_AMD64
#   elif defined( __arm__ ) || defined( __thumb__ )
#       define GUCEF_CPU_ARCHITECTURE GUCEF_CPU_ARCHITECTURE_ARM
#   elif defined( __aarch64__ )
#       define GUCEF_CPU_ARCHITECTURE GUCEF_CPU_ARCHITECTURE_ARM64
#   endif
#elif defined( __GNUC__ )
#   define GUCEF_COMPILER GUCEF_COMPILER_GNUC
#   define GUCEF_COMP_VER (((__GNUC__)*100) + \
        (__GNUC_MINOR__*10) + \
        __GNUC_PATCHLEVEL__)
#   if defined( __i386__ ) || defined( __i486__ ) || defined( __i586__ ) || defined( __i686__ )
#       define GUCEF_CPU_ARCHITECTURE GUCEF_CPU_ARCHITECTURE_X86
#   elif defined( __amd64__ ) || defined( __amd64 ) || defined( __x86_64__ ) || defined( __x86_64 )
#       define GUCEF_CPU_ARCHITECTURE GUCEF_CPU_ARCHITECTURE_AMD64
#   elif defined( __arm__ ) || defined( __thumb__ )
#       define GUCEF_CPU_ARCHITECTURE GUCEF_CPU_ARCHITECTURE_ARM
#   elif defined( __aarch64__ )
#       define GUCEF_CPU_ARCHITECTURE GUCEF_CPU_ARCHITECTURE_ARM64
#   endif
#elif defined( __BORLANDC__ )
#   define GUCEF_COMPILER GUCEF_COMPILER_BORL
#   define GUCEF_COMP_VER __BCPLUSPLUS__
#   define __FUNCTION__ __FUNC__
#else
#   pragma error "No known compiler. Abort! Abort!"

#endif

/*-------------------------------------------------------------------------*/


/*
 *  O/S Platform detection
 */
#if defined( __WIN32__ ) || defined( _WIN32 )
#   define GUCEF_PLATFORM GUCEF_PLATFORM_MSWIN
#   define GUCEF_MSWIN_BUILD
#elif defined( __APPLE_CC__)
#   define GUCEF_PLATFORM GUCEF_PLATFORM_MACOS
#elif defined( __ANDROID__ ) || defined( ANDROID )
#   define GUCEF_PLATFORM GUCEF_PLATFORM_ANDROID
#elif defined( unix )  || defined( __unix ) || defined( __unix__ ) || defined( linux ) || defined( __linux ) || defined( __linux__ )
#   define GUCEF_PLATFORM GUCEF_PLATFORM_LINUX
#   define GUCEF_LINUX_BUILD
#else
#   define GUCEF_PLATFORM GUCEF_PLATFORM_LINUX
#   define GUCEF_LINUX_BUILD
#endif

/*-------------------------------------------------------------------------*/

/*
 *  Define bitness value for comparison style macro
 */
#define GUCEF_BITNESS_UNKNOWN 0
#define GUCEF_BITNESS_32      1
#define GUCEF_BITNESS_64      2

/*
 *  Compile time bitness target detection
 */
#if !( defined(GUCEF_32BIT) || defined(GUCEF_64BIT) )
    #if ( ( GUCEF_CPU_ARCHITECTURE == GUCEF_CPU_ARCHITECTURE_AMD64 ) || ( GUCEF_CPU_ARCHITECTURE == GUCEF_CPU_ARCHITECTURE_ARM64 ) )
        #define GUCEF_64BIT GUCEF_BITNESS_64
    #elif ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
        #ifdef _WIN64
            #define GUCEF_64BIT GUCEF_BITNESS_64
        #endif
    #elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )
        #ifdef __LP64__
            #define GUCEF_64BIT GUCEF_BITNESS_64
        #endif
    #endif
#endif

/*
 *  Default to 32 bit if detection fails
 */
#if !( defined(GUCEF_32BIT) || defined(GUCEF_64BIT) )
    #define GUCEF_32BIT GUCEF_BITNESS_32
#endif

#ifdef GUCEF_32BIT
    #define GUCEF_BITNESS GUCEF_BITNESS_32
#elif GUCEF_64BIT
    #define GUCEF_BITNESS GUCEF_BITNESS_64
#else
    #define GUCEF_BITNESS GUCEF_BITNESS_UNKNOWN
#endif

/*-------------------------------------------------------------------------*/

/*
 *  Compile time Little or Big Endian byte order detection
 */
#ifndef GUCEF_BYTEORDER_ENDIAN_COMPILE_TIME
    // Detect with GCC 4.6's macro.
#   if defined( __BYTE_ORDER__ )
#       if (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
#           define GUCEF_BYTEORDER_ENDIAN_COMPILE_TIME GUCEF_BYTEORDER_LITTLE_ENDIAN
#       elif (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
#           define GUCEF_BYTEORDER_ENDIAN_COMPILE_TIME GUCEF_BYTEORDER_BIG_ENDIAN
#       else
#           error "Unknown machine byteorder endianness detected. User needs to define GUCEF_BYTEORDER_ENDIAN_COMPILE_TIME."
#       endif
    // Detect with GLIBC's endian.h.
#   elif defined(__GLIBC__)
#       include <endian.h>
#       if (__BYTE_ORDER == __LITTLE_ENDIAN)
#           define GUCEF_BYTEORDER_ENDIAN_COMPILE_TIME GUCEF_BYTEORDER_LITTLE_ENDIAN
#       elif (__BYTE_ORDER == __BIG_ENDIAN)
#           define GUCEF_BYTEORDER_ENDIAN_COMPILE_TIME GUCEF_BYTEORDER_BIG_ENDIAN
#       else
#           error "Unknown machine byteorder endianness detected. User needs to define GUCEF_BYTEORDER_ENDIAN_COMPILE_TIME."
#       endif
    // Detect with _LITTLE_ENDIAN and _BIG_ENDIAN macro.
#   elif defined(_LITTLE_ENDIAN) && !defined(_BIG_ENDIAN)
#       define GUCEF_BYTEORDER_ENDIAN_COMPILE_TIME GUCEF_BYTEORDER_LITTLE_ENDIAN
#   elif defined(_BIG_ENDIAN) && !defined(_LITTLE_ENDIAN)
#       define GUCEF_BYTEORDER_ENDIAN_COMPILE_TIME GUCEF_BYTEORDER_BIG_ENDIAN
    // Detect with architecture macros.
#   elif defined(__sparc) || defined(__sparc__) || defined(_POWER) || defined(__powerpc__) || defined(__ppc__) || defined(__hpux) || defined(__hppa) || defined(_MIPSEB) || defined(_POWER) || defined(__s390__)
#       define GUCEF_BYTEORDER_ENDIAN_COMPILE_TIME GUCEF_BYTEORDER_BIG_ENDIAN
#   elif defined(__i386__) || defined(__alpha__) || defined(__ia64) || defined(__ia64__) || defined(_M_IX86) || defined(_M_IA64) || defined(_M_ALPHA) || defined(__amd64) || defined(__amd64__) || defined(_M_AMD64) || defined(__x86_64) || defined(__x86_64__) || defined(_M_X64) || defined(__bfin__)
#       define GUCEF_BYTEORDER_ENDIAN_COMPILE_TIME GUCEF_BYTEORDER_LITTLE_ENDIAN
#   elif defined(_MSC_VER) && (defined(_M_ARM) || defined(_M_ARM64))
#       define GUCEF_BYTEORDER_ENDIAN_COMPILE_TIME GUCEF_BYTEORDER_LITTLE_ENDIAN
#   else
#       error "Unknown machine byteorder endianness detected. User needs to define GUCEF_BYTEORDER_ENDIAN_COMPILE_TIME."
#   endif
#endif

#ifndef GUCEF_PLATFORM_BYTEORDER_ENDIAN
    #define GUCEF_PLATFORM_BYTEORDER_ENDIAN     GUCEF_BYTEORDER_ENDIAN_COMPILE_TIME
#endif

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_PLATFORM_H ? */
