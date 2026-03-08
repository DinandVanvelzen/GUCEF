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

#ifndef GUCEF_DRGUP_LOCKTRACE_H
#define GUCEF_DRGUP_LOCKTRACE_H

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

#ifndef GUCEF_DRGUP_CONFIG_H
#include "gucefDRGUP_config.h"
#define GUCEF_DRGUP_CONFIG_H
#endif /* GUCEF_DRGUP_CONFIG_H ? */

#ifndef GUCEF_DRGUP_ETYPES_H
#include "gucefDRGUP_ETypes.h"
#define GUCEF_DRGUP_ETYPES_H
#endif /* GUCEF_DRGUP_ETYPES_H ? */

#ifndef GUCEF_DRGUP_MACROS_H
#include "gucefDRGUP_macros.h"      /* module build configuration */
#define GUCEF_DRGUP_MACROS_H
#endif /* GUCEF_DRGUP_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifdef __cplusplus
namespace GUCEF {
namespace DRGUP {
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
GUCEF_DRGUP_PUBLIC_C void
DRGUP_ExclusiveLockCreated( void* lockId );

/*-------------------------------------------------------------------------*/

/**
 *  Should be called right after an exclusive lock is obtained
 */
GUCEF_DRGUP_PUBLIC_C void
DRGUP_ExclusiveLockObtained( void* lockId );

/*-------------------------------------------------------------------------*/

/**
 *  Should be called right before an exclusive lock is released
 */
GUCEF_DRGUP_PUBLIC_C void
DRGUP_ExclusiveLockReleased( void* lockId );

/*-------------------------------------------------------------------------*/

/**
 *  Should be called when obtaining an abandoned lock
 */
GUCEF_DRGUP_PUBLIC_C void
DRGUP_ExclusiveLockAbandoned( void* lockId );

/*-------------------------------------------------------------------------*/

/**
 *  Should be called right before an exclusive lock is destroyed.
 *  Its the destructor essentially.
 */
GUCEF_DRGUP_PUBLIC_C void
DRGUP_ExclusiveLockDestroy( void* lockId );

/*-------------------------------------------------------------------------*/

/**
 *  DRGUP_LockProtectsRange():
 *  Declare that lockId is intended to protect the memory range [address, address+size).
 *  At deallocation time, if the allocation overlaps the range and the lock is not
 *  currently held, a warning is logged to the memory report.
 *  This is contract validation, not race detection.
 */
GUCEF_DRGUP_PUBLIC_C void
DRGUP_LockProtectsRange( void* lockId, const void* address, size_t size );

/*-------------------------------------------------------------------------*/

/**
 *  DRGUP_LockUnprotectsRange():
 *  Remove any range association previously declared for lockId.
 */
GUCEF_DRGUP_PUBLIC_C void
DRGUP_LockUnprotectsRange( void* lockId );

/*-------------------------------------------------------------------------*/

/**
 *  Initializes the lock tracing machinery
 */
GUCEF_DRGUP_PUBLIC_C void
DRGUP_LockTraceInit( void );

/*-------------------------------------------------------------------------*/

/**
 *  Cleans up the lock tracing machinery
 */
GUCEF_DRGUP_PUBLIC_C void
DRGUP_LockTraceShutdown( void );

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
}; /* namespace DRGUP */
}; /* namespace GUCEF */
#endif /* __cplusplus ? */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRGUP_LOCKTRACE_H ? */
