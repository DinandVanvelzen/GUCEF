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

#ifndef GUCEF_BASICHELPERS_H
#define GUCEF_BASICHELPERS_H

/*
 *      Basic generic helper macros for the GUCEF platform
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

#include <assert.h>

/*-------------------------------------------------------------------------//
//                                                                         //
//      GENERAL MACROS                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *  Dummy define that can be used as a dummy param in macros
 */
#undef GUCEF_VOIDDEF
#define GUCEF_VOIDDEF

/*-------------------------------------------------------------------------*/

/*
 *  GUCEF wide debug flag
 */
#if !defined( NDEBUG ) && !( defined( _DEBUG ) || defined( __DEBUG ) || defined( __DEBUG__ ) )
  #define _DEBUG
#endif
#if defined( _DEBUG ) || defined( __DEBUG ) || defined( __DEBUG__ )
  #define GUCEF_DEBUG_MODE
#else
  #undef GUCEF_DEBUG_MODE
#endif

#if defined( GUCEF_DEBUG_MODE ) && defined( GUCEF_DEBUG_VERBOSE )
  #define GUCEF_DEBUG_MODE_VERBOSE
#else
  #undef GUCEF_DEBUG_MODE_VERBOSE
#endif

/*-------------------------------------------------------------------------*/

/*
 *      Bit operations.
 *      Use the following macro's to set/get a bit in an integer.
 */
#undef GUCEF_GETBITX
#define GUCEF_GETBITX( var, x ) ( var & ( 1<<x ) )
#undef GUCEF_SETBITXON
#define GUCEF_SETBITXON( var, x ) ( var |= 1<<x )
#undef GUCEF_SETBITXOFF
#define GUCEF_SETBITXOFF( var, x ) ( var &= ~( 1<<x ) )

/*-------------------------------------------------------------------------*/

/*
 *  Macro for checking type sizes compile time
 */
#define GUCEF_STATIC_SIZE_EQ_CHECK( var, refType ) ( { char const CompileTimeTypeSizeEqualsCheck[ sizeof( var ) == sizeof( refType ) ? 1 : -1 ]; } )
#define GUCEF_STATIC_SIZE_SM_CHECK( var, refType ) ( { char const CompileTimeTypeSizeSmallerCheck[ sizeof( var ) < sizeof( refType ) ? 1 : -1 ]; } )
#define GUCEF_STATIC_SIZE_SE_CHECK( var, refType ) ( { char const CompileTimeTypeSizeSmallerEqualsCheck[ sizeof( var ) <= sizeof( refType ) ? 1 : -1 ]; } )
#define GUCEF_STATIC_SIZE_SE_LCHECK( var, checkSize ) ( { char const CompileTimeTypeSizeSmallerEqualsCheck[ sizeof( var ) <= checkSize ? 1 : -1 ]; } )
#define GUCEF_STATIC_SIZE_GR_CHECK( var, refType ) ( { char const CompileTimeTypeSizeGreaterCheck[ sizeof( var ) > sizeof( refType ) ? 1 : -1 ]; } )
#define GUCEF_STATIC_SIZE_GE_CHECK( var, refType ) ( { char const CompileTimeTypeSizeGreaterEqualsCheck[ sizeof( var ) >= sizeof( refType ) ? 1 : -1 ]; } )

/*-------------------------------------------------------------------------*/

#define GUCEF_ASSERT( condition )   assert( condition );
#define GUCEF_ASSERT_ALWAYS    assert( 0 );
#define GUCEF_UNREACHABLE      GUCEF_ASSERT_ALWAYS;

/*-------------------------------------------------------------------------*/

/* GCC/G++ does not define __TIMESTAMP__ */

#ifndef __TIMESTAMP__
#define __TIMESTAMP__ __DATE__ " " __TIME__
#endif

/*-------------------------------------------------------------------------*/

#define _GUCEF_CONCAT( a, b ) a##_##b
#define GUCEF_CONCAT( a, b ) _GUCEF_CONCAT( a, b )

/*-------------------------------------------------------------------------*/

#undef NULL
#if defined(__cplusplus)
  #ifndef __CPP09NULLPTRSUPPORTED
    #if GUCEF_COMPILER == GUCEF_COMPILER_GNUC
      #define NULL __null
    #else
      #define NULL (0)
    #endif
  #else
    #define NULL (nullptr)
  #endif
#else
  #define NULL ((void*)0)
#endif

#define GUCEF_NULL NULL

/*-------------------------------------------------------------------------*/

/* Portable deprecation message macro */
#ifndef GUCEF_DEPRECATED
  #if defined(_MSC_VER)
    #define GUCEF_DEPRECATED(msg) __declspec(deprecated(msg))
  #elif defined(__GNUC__) || defined(__clang__)
    #define GUCEF_DEPRECATED(msg) __attribute__((deprecated(msg)))
  #else
    #define GUCEF_DEPRECATED(msg)
  #endif
#endif

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_BASICHELPERS_H ? */
