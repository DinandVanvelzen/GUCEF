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

#ifndef GUCEF_SCOPESTACKTRACER_H
#define GUCEF_SCOPESTACKTRACER_H

#ifdef __cplusplus

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

#if defined( GUCEF_USE_CALLSTACK_TRACING ) && defined( GUCEF_USE_PLATFORM_CALLSTACK_TRACING ) && !defined( GUCEF_CALLSTACK_TRACING_DISABLED )

#ifndef GUCEF_MEMORYMANAGERLOADER_H
#include "gucef_MemoryManagerLoader.h"       /* header that includes the header-only functionality to load the external memory manager */
#define GUCEF_MEMORYMANAGERLOADER_H
#endif /* GUCEF_MEMORYMANAGERLOADER_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Very simply class that calls the GUCEF callstack functions.
 *  This allows us to use creation and destruction behavior of a C++ object
 *  to ensure that both the begin and end callstack functions are called for
 *  the scope where the tracer object is created.
 *
 *  Note that it is recommended to use the GUCEF_TRACE macro instead of 
 *  using the class directly
 */
class GUCEF_HIDDEN ScopeStackTracer
{
    public:

    ScopeStackTracer( const char* file ,
                      const int line   )
    {
        MEMMAN_CallstackScopeBegin( file, line );
    }

    ~ScopeStackTracer() 
    {
        MEMMAN_CallstackScopeEnd();
    }

    private:

    ScopeStackTracer( void ) {}                                                   /**< not applicable */
    ScopeStackTracer( const ScopeStackTracer& src ) {}                            /**< not applicable */
    ScopeStackTracer& operator=( const ScopeStackTracer& src ) { return *this; }  /**< not applicable */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUCEF */

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *      Macro that creates a tracer object and fills in the file and line
 *      precompiler directives. This should save you some typing.
 */

  #define GUCEF_TRACE ::GUCEF::ScopeStackTracer GUCEFScopeStackTracerObject( __FILE__, __LINE__ );

#else  /* defined( GUCEF_USE_CALLSTACK_TRACING ) && defined( GUCEF_USE_PLATFORM_CALLSTACK_TRACING ) && !defined( GUCEF_CALLSTACK_TRACING_DISABLED ) ? */

  #define GUCEF_TRACE

#endif /* defined( GUCEF_USE_CALLSTACK_TRACING ) && defined( GUCEF_USE_PLATFORM_CALLSTACK_TRACING ) && !defined( GUCEF_CALLSTACK_TRACING_DISABLED ) ? */

/*-------------------------------------------------------------------------*/

#else /* __cplusplus ? */

/* for headers included in C code just define an empty macro */
#define GUCEF_TRACE

#endif

#endif /* GUCEF_SCOPESTACKTRACER_H ? */

