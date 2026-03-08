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

#ifndef GUCEF_MEMORY_H
#define GUCEF_MEMORY_H

/*
 *      Build configuration specific macros for the GUCEF platform
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
 *  now we include macros for activating memory checking related redirects
 *  if the facility is turned on per the build config
 */
#ifdef GUCEF_USE_MEMORY_LEAK_CHECKER
    #ifdef GUCEF_USE_PLATFORM_MEMORY_LEAK_CHECKER

    /*
     *  Since we are using the GUCEF platform's build-in memory leak checking
     *  we will include the header here that will redirect all malloc()/new etc calls
     */
    #ifndef GUCEF_DYNNEWON_H 
    #include "gucef_dynnewon.h"
    #define GUCEF_DYNNEWON_H
    #endif /* GUCEF_DYNNEWON_H ? */

    #else

    /*
     *  Since we are NOT using the GUCEF platform's build-in memory leak checking
     *  we will try to use whatever facilities the current OS/compiler/IDE provides
     */

    #if ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN ) && ( GUCEF_COMPILER == GUCEF_COMPILER_MSVC ) )

        #define _CRTDBG_MAP_ALLOC
        #include <stdlib.h>
        #include <crtdbg.h>    

        #ifdef __cplusplus
          #define GUCEF_MSVCCRT_MEMCHECK_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
          #define new GUCEF_MSVCCRT_MEMCHECK_NEW
        #endif

    #endif 

    #ifndef GUCEF_DYNNEWOFF_H 
    #include "gucef_dynnewoff.h"
    #define GUCEF_DYNNEWOFF_H
    #endif /* GUCEF_DYNNEWOFF_H ? */

    #endif /* GUCEF_USE_PLATFORM_MEMORY_LEAK_CHECKER ? */
#else

    #ifndef GUCEF_DYNNEWOFF_H 
    #include "gucef_dynnewoff.h"
    #define GUCEF_DYNNEWOFF_H
    #endif /* GUCEF_DYNNEWOFF_H ? */

#endif /* GUCEF_USE_MEMORY_LEAK_CHECKER ? */

/*-------------------------------------------------------------------------*/

#if defined( GUCEF_USE_CALLSTACK_TRACING ) && !defined( GUCEF_CALLSTACK_TRACING_DISABLED )
    #ifdef GUCEF_USE_PLATFORM_CALLSTACK_TRACING

        #ifndef GUCEF_MEMORYMANAGERLOADER_H
        #include "gucef_MemoryManagerLoader.h"       /* header that includes the header-only functionality to load the external memory manager */
        #define GUCEF_MEMORYMANAGERLOADER_H
        #endif /* GUCEF_MEMORYMANAGERLOADER_H ? */

        #ifdef __cplusplus

            #ifndef GUCEF_SCOPESTACKTRACER_H    
            #include "gucef_ScopeStackTracer.h"
            #define GUCEF_SCOPESTACKTRACER_H
            #endif /* GUCEF_SCOPESTACKTRACER_H ? */

        #endif
        
        /* macros for C code usage of callstack tracking. Please use ScopeStackTracer via the GUCEF_TRACE macro for C++ code */
        #define GUCEF_BEGIN { DRGUP_CallstackScopeBegin( __FILE__, __LINE__ ); }
        #define GUCEF_END { DRGUP_CallstackScopeEnd(); }
        #define GUCEF_END_RET( retval ) { DRGUP_CallstackScopeEnd(); return retval; }

        #if defined( GUCEF_USE_PLATFORM_LOCK_TRACER ) && !defined( GUCEF_PLATFORM_LOCK_TRACER_DISABLED )

            /*
             *  Platform lock tracing is dependent on platform callstack tracing
             */
            #define GUCEF_TRACE_EXCLUSIVE_LOCK_CREATED( lockId ) { DRGUP_ExclusiveLockCreated( lockId ); }
            #define GUCEF_TRACE_EXCLUSIVE_LOCK_OBTAINED( lockId ) { DRGUP_ExclusiveLockObtained( lockId ); }
            #define GUCEF_TRACE_EXCLUSIVE_LOCK_RELEASED( lockId ) { DRGUP_ExclusiveLockReleased( lockId ); }
            #define GUCEF_TRACE_EXCLUSIVE_LOCK_ABANDONED( lockId ) { DRGUP_ExclusiveLockAbandoned( lockId ); }
            #define GUCEF_TRACE_EXCLUSIVE_LOCK_DESTROY( lockId ) { DRGUP_ExclusiveLockDestroy( lockId ); }

        #else

            #define GUCEF_TRACE_EXCLUSIVE_LOCK_CREATED( lockId )
            #define GUCEF_TRACE_EXCLUSIVE_LOCK_OBTAINED( lockId )
            #define GUCEF_TRACE_EXCLUSIVE_LOCK_RELEASED( lockId )
            #define GUCEF_TRACE_EXCLUSIVE_LOCK_ABANDONED( lockId )
            #define GUCEF_TRACE_EXCLUSIVE_LOCK_DESTROY( lockId )
        
        #endif /* GUCEF_USE_PLATFORM_LOCK_TRACER ? */

    #else

        /*
         *  No non-GUCEF platform stack tracing facility available at this time
         */
        #define GUCEF_BEGIN
        #define GUCEF_END
        #define GUCEF_END_RET( retval ) return (retval);
        #define GUCEF_TRACE
        #define GUCEF_TRACE_EXCLUSIVE_LOCK_CREATED( lockId )
        #define GUCEF_TRACE_EXCLUSIVE_LOCK_OBTAINED( lockId )
        #define GUCEF_TRACE_EXCLUSIVE_LOCK_RELEASED( lockId )
        #define GUCEF_TRACE_EXCLUSIVE_LOCK_ABANDONED( lockId )
        #define GUCEF_TRACE_EXCLUSIVE_LOCK_DESTROY( lockId )
    
    #endif /* GUCEF_USE_PLATFORM_CALLSTACK_TRACING ? */

#else /* defined( GUCEF_USE_CALLSTACK_TRACING ) && !defined( GUCEF_CALLSTACK_TRACING_DISABLED ) ? */

  #define GUCEF_BEGIN
  #define GUCEF_END
  #define GUCEF_END_RET( retval ) return (retval);
  #define GUCEF_TRACE
  #define GUCEF_TRACE_EXCLUSIVE_LOCK_CREATED( lockId )
  #define GUCEF_TRACE_EXCLUSIVE_LOCK_OBTAINED( lockId )
  #define GUCEF_TRACE_EXCLUSIVE_LOCK_RELEASED( lockId )
  #define GUCEF_TRACE_EXCLUSIVE_LOCK_ABANDONED( lockId )
  #define GUCEF_TRACE_EXCLUSIVE_LOCK_DESTROY( lockId )

#endif /* defined( GUCEF_USE_CALLSTACK_TRACING ) && !defined( GUCEF_CALLSTACK_TRACING_DISABLED ) ? */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MEMORY_H ? */
