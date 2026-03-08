/*
 *  gucefMT: GUCEF module providing multithreading solutions
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

#ifndef GUCEF_LOCKTRACE_H
#define GUCEF_LOCKTRACE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <stdlib.h>     /* size_t */

#ifndef GUCEF_MACROS_H
#include "gucef_macros.h"
#define GUCEF__MACROS_H
#endif /* GUCEF_MACROS_H ? */

#ifndef GUCEF_MLF_CONFIG_H
#include "gucefMLF_config.h"
#define GUCEF_MLF_CONFIG_H
#endif /* GUCEF_MLF_CONFIG_H ? */

#ifndef GUCEF_MLF_ETYPES_H
#include "gucefMLF_ETypes.h"
#define GUCEF_MLF_ETYPES_H
#endif /* GUCEF_MLF_ETYPES_H ? */

#ifndef GUCEF_MLF_MACROS_H
#include "gucefMLF_macros.h"      /* module build configuration */
#define GUCEF_MLF_MACROS_H
#endif /* GUCEF_MLF_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifdef __cplusplus
namespace GUCEF {
namespace MLF {
#endif /* __cplusplus ? */

#ifdef __cplusplus
   extern "C" {
#endif   /* __cplusplus */

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Should be called right after an exclusive lock is created but not yet locked
 *  or in any way used yet across threads. Its the constructor essentially.
 */
GUCEF_MLF_PUBLIC_C void
MEMMAN_ExclusiveLockCreated( void* lockId );

/*-------------------------------------------------------------------------*/

/**
 *  Should be called right after an exclusive lock is obtained
 */
GUCEF_MLF_PUBLIC_C void
MEMMAN_ExclusiveLockObtained( void* lockId );

/*-------------------------------------------------------------------------*/

/**
 *  Should be called right before an exclusive lock is released
 */
GUCEF_MLF_PUBLIC_C void
MEMMAN_ExclusiveLockReleased( void* lockId );

/*-------------------------------------------------------------------------*/

/**
 *  Should be called when obtaining an abandoned lock
 */
GUCEF_MLF_PUBLIC_C void
MEMMAN_ExclusiveLockAbandoned( void* lockId );

/*-------------------------------------------------------------------------*/

/**
 *  Should be called right before an exclusive lock is destroyed.
 *  Its the destructor essentially.
 */
GUCEF_MLF_PUBLIC_C void
MEMMAN_ExclusiveLockDestroy( void* lockId );

/*-------------------------------------------------------------------------*/

/**
 *  MEMMAN_LockProtectsRange():
 *  Declare that lockId is intended to protect the memory range [address, address+size).
 *  At deallocation time, if the allocation overlaps the range and the lock is not
 *  currently held, a warning is logged to the memory report.
 *  This is contract validation, not race detection.
 */
GUCEF_MLF_PUBLIC_C void
MEMMAN_LockProtectsRange( void* lockId, const void* address, size_t size );

/*-------------------------------------------------------------------------*/

/**
 *  MEMMAN_LockUnprotectsRange():
 *  Remove any range association previously declared for lockId.
 */
GUCEF_MLF_PUBLIC_C void
MEMMAN_LockUnprotectsRange( void* lockId );

/*-------------------------------------------------------------------------*/

/**
 *  Initializes the lock tracing machinery
 */
GUCEF_MLF_PUBLIC_C void
MEMMAN_LockTraceInit( void );

/*-------------------------------------------------------------------------*/

/**
 *  Cleans up the lock tracing machinery
 */
GUCEF_MLF_PUBLIC_C void
MEMMAN_LockTraceShutdown( void );

/*--------------------------------------------------------------------------*/

#ifdef __cplusplus
   }
#endif /* __cplusplus */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifdef __cplusplus
}; /* namespace MLF */
}; /* namespace GUCEF */
#endif /* __cplusplus ? */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_LOCKTRACE_H ? */
