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

#ifndef GUCEF_MT_MUTEX_H
#define GUCEF_MT_MUTEX_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_ETYPES_H
#include "gucefMT_ETypes.h"
#define GUCEF_MT_ETYPES_H
#endif /* GUCEF_MT_ETYPES_H ? */

#ifndef GUCEFMT_MACROS_H
#include "gucefMT_macros.h"     /* often used gucef macros */
#define GUCEFMT_MACROS_H
#endif /* GUCEFMT_MACROS_H ? */

#ifndef GUCEF_MT_LOCKSTATUS_H
#include "gucefMT_LockStatus.h"
#define GUCEF_MT_LOCKSTATUS_H
#endif /* GUCEF_MT_LOCKSTATUS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifdef __cplusplus
namespace GUCEF {
namespace MT {
#endif /* __cplusplus ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

struct SMutex;

#define GUCEF_MUTEX_OPERATION_FAILED        GUCEF_LOCKSTATUS_OPERATION_FAILED
#define GUCEF_MUTEX_OPERATION_SUCCESS       GUCEF_LOCKSTATUS_OPERATION_SUCCESS
#define GUCEF_MUTEX_WAIT_TIMEOUT            GUCEF_LOCKSTATUS_WAIT_TIMEOUT
#define GUCEF_MUTEX_ABANDONED               GUCEF_LOCKSTATUS_ABANDONED

#define GUCEF_MUTEX_INFINITE_TIMEOUT        GUCEF_MT_INFINITE_LOCK_TIMEOUT

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

/**
 *      Creates the initial unlocked mutex.
 */
GUCEF_MT_PUBLIC_C struct SMutex* 
MutexCreate( void );

/*-------------------------------------------------------------------------*/

/**
 *      Deallocates/destroys a mutex created with MutexCreate()
 */
GUCEF_MT_PUBLIC_C void 
MutexDestroy( struct SMutex* mutex );

/*-------------------------------------------------------------------------*/

/**
 *  Lock the mutex. If the mutex is already locked the calling
 *  process will have to wait for the mutex to allow a lock.
 *
 *  @return one of the GUCEF_MUTEX_* values
 */
GUCEF_MT_PUBLIC_C UInt32 
MutexLock( struct SMutex* mutex, UInt32 timeoutInMs );

/*-------------------------------------------------------------------------*/

/**
 *  Returns wheter or not the mutex is currently locked.
 *  0 is not locked.
 *  non-zero is locked.
 *  The value if non-zero and thus locked is the thread ID of the thread holding the lock
 *
 *  Note that this is subject to race conditions and it only intended as a snapshot-in-time
 *  Its for human consumption and test code under controlled cisrumstances only
 */
GUCEF_MT_PUBLIC_C UInt32 
MutexLocked( struct SMutex* mutex );

/*-------------------------------------------------------------------------*/

/**
 *  Returns the reentrancy count on the lock by the current thread holding the lock.
 *
 *  Note that this is subject to race conditions and it only intended as a snapshot-in-time
 *  The value should only be considered accurate when queried by the thread holding the lock
 */
GUCEF_MT_PUBLIC_C Int32 
MutexReentrancy( struct SMutex* mutex );

/*-------------------------------------------------------------------------*/

/**
 *  Returns the identity (GetCurrentThreadID()) of the thread holding the lock, 0 when no thread holds it.
 *  A snapshot in time for diagnostics and test code under controlled circumstances, like MutexLocked().
 */
GUCEF_MT_PUBLIC_C TThreadId 
MutexLockedByThread( struct SMutex* mutex );

/*-------------------------------------------------------------------------*/

/**
 *  Returns the operating system's id (GetCurrentThreadSystemID()) of the thread holding the lock, 0 when no thread
 *  holds it, for display: it matches the debugger's thread list. Where GUCEF_SYSTEM_THREAD_ID_IS_THREAD_ID is 1 this
 *  is the identity; where gucefMT is built with GUCEF_MT_RECORD_SYSTEM_THREAD_ID set to 0 it is always 0.
 *  Never use it to decide which thread holds the lock, use MutexLockedByThread() for that.
 */
GUCEF_MT_PUBLIC_C TSystemThreadId 
MutexLockedBySystemThread( struct SMutex* mutex );

/*-------------------------------------------------------------------------*/

/**
 *  Returns sizeof( struct SMutex ), the size of the mutex's private data, for tests that check which members exist
 */
GUCEF_MT_PUBLIC_C UInt32 
MutexStructSize( void );

/*-------------------------------------------------------------------------*/

/**
 *      Unlocks the mutex after a call to Lock_Mutex(). Other processes
 *      will have the ability to get a mutex lock after this call.
 *      The boolean return value indicates wheter the unlock failed or succeeded.
 */
GUCEF_MT_PUBLIC_C UInt32 
MutexUnlock( struct SMutex* mutex );

/*--------------------------------------------------------------------------*/

#ifdef __cplusplus
   };
#endif /* __cplusplus */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifdef __cplusplus
}; /* namespace MT */
}; /* namespace GUCEF */
#endif /* __cplusplus ? */

/*--------------------------------------------------------------------------*/

#endif /* GUCEF_MT_MUTEX_H ? */
