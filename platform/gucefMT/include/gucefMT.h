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

/*
 *      This is the main include header for the gucefMT library.
 *      It is part of a framework called GUCEF.
 *      All classes in this library have to do with threading and
 *      threadsafety.
 */

#ifndef GUCEF_MT_GUCEFMT_H
#define GUCEF_MT_GUCEFMT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CONFIG_H
#include "gucef_config.h"           /* GUCEF platform wide config */
#define GUCEF_CONFIG_H
#endif /* GUCEF_CONFIG_H ? */

#ifndef GUCEF_MACROS_H
#include "gucef_macros.h"          /* GUCEF platform wide macros */
#define GUCEF_MACROS_H
#endif /* GUCEF_MACROS_H ? */

/*
 *      Library build configuration
 */
#ifndef GUCEF_MT_GUCEFMT_CONFIG_H
#include "gucefMT_config.h"
#define GUCEF_MT_GUCEFMT_CONFIG_H
#endif /* GUCEF_MT_GUCEFMT_CONFIG_H ? */

#ifndef GUCEF_MT_BASICMACROS_H
#include "gucefMT_basicMacros.h"
#define GUCEF_MT_BASICMACROS_H
#endif /* GUCEF_MT_BASICMACROS_H ? */

/*
 *      Library specific macros and generic macros
 */
#ifndef GUCEF_MT_GUCEFMT_MACROS_H
#include "gucefMT_macros.h"
#define GUCEF_MT_GUCEFMT_MACROS_H
#endif /* GUCEF_MT_GUCEFMT_MACROS_H ? */

/*
 *      Simple types
 */
#ifndef GUCEF_MT_ETYPES_H
#include "gucefMT_ETypes.h"
#define GUCEF_MT_ETYPES_H
#endif /* GUCEF_MT_ETYPES_H ? */

#ifndef GUCEF_MT_LOCKSTATUS_H
#include "gucefMT_LockStatus.h"
#define GUCEF_MT_LOCKSTATUS_H
#endif /* GUCEF_MT_LOCKSTATUS_H ? */

/*
 *      C implementation of a readers writers lock.
 */
#ifndef GUCEF_MT_DVRWLOCK_H
#include "gucefMT_DVRWLOCK.h"
#define GUCEF_MT_DVRWLOCK_H
#endif /* GUCEF_MT_DVRWLOCK_H ? */

/**
 *      Wrapper functions for threading related O/S functionality
 */
#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

/*
 *      C implementation of a cross-platform mutex wrapper
 */
#ifndef GUCEF_MT_MUTEX_H
#include "gucefMT_mutex.h"
#define GUCEF_MT_MUTEX_H
#endif /* GUCEF_MT_MUTEX_H ? */

/*---------------------------------------------------------------------------/

        C++ includes

/---------------------------------------------------------------------------*/
#ifdef __cplusplus

#ifndef GUCEF_MT_CILOCKABLE_H
#include "gucefMT_CILockable.h"
#define GUCEF_MT_CILOCKABLE_H
#endif /* GUCEF_MT_CILOCKABLE_H ? */

#ifndef GUCEF_MT_COBJECTSCOPELOCK_H
#include "gucefMT_CObjectScopeLock.h"
#define GUCEF_MT_COBJECTSCOPELOCK_H
#endif /* GUCEF_MT_COBJECTSCOPELOCK_H ? */

/*
 *      class that implements a recursive mutex
 *      supports mswindows and linux
 */
#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

/*
 *      implementation of a thread mailbox.
 *      basicly just a tread-safe stack for event data
 */
#ifndef GUCEF_MT_CTMAILBOX_H
#include "gucefMT_CTMailBox.h"
#define GUCEF_MT_CTMAILBOX_H
#endif /* GUCEF_MT_CTMAILBOX_H ? */

/*
 *      Wrapper class for my C implementation of a readers/writers lock
 */
#ifndef GUCEF_MT_CREADWRITELOCK_H
#include "gucefMT_CReadWriteLock.h"
#define GUCEF_MT_CREADWRITELOCK_H
#endif /* GUCEF_MT_CREADWRITELOCK_H ? */

/*
 *      Counting semaphore.
 *      The usage of this class allows you to forgo having to type the same
 *      tests and allocation/de-allocation code all the time.
 */
#ifndef GUCEF_MT_CSEMAPHORE_H
#include "gucefMT_CSemaphore.h"
#define GUCEF_MT_CSEMAPHORE_H
#endif /* GUCEF_MT_CSEMAPHORE_H ? */

/*
 *      Base class for an active object.
 *      Independant activity is implemented using threads.
 */
#ifndef GUCEF_MT_CACTIVEOBJECT_H
#include "gucefMT_CActiveObject.h"
#define GUCEF_MT_CACTIVEOBJECT_H
#endif /* GUCEF_MT_CACTIVEOBJECT_H ? */

#endif /* __cplusplus ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *      Prevent C++ name mangeling
 */
#ifdef __cplusplus
   extern "C" {
#endif   /* __cplusplus */

/*-------------------------------------------------------------------------*/

GUCEF_MT_PUBLIC_C const char* GUCEF_MT_ModuleCompileTimeStamp( void );

/*-------------------------------------------------------------------------*/

#ifdef __cplusplus
   }
#endif /* __cplusplus */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MT_GUCEFMT_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      Info & Changes                                                     //
//                                                                         //
//-------------------------------------------------------------------------//

- 11-03-2004 :
       - Made the initial version of the library header. The classes here
         where originally part of one large GUCEF codebase without segmentation.

-----------------------------------------------------------------------------*/



