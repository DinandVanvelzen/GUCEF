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
 *      These are some functions which represent my implementation of
 *      the reader/writers problem and subsequently it's solution.
 *      Reader tasks can work parallel while writer tasks will be given
 *      exclusive access. This can allow some applications to work much faster
 *      then with a simple mutex lock implementation since readers can operate
 *      paralell.
 *
 *      You might wonder when something is a reader and when is it a writer task
 *      Well,.. if you use a thread that uses data but does not change the data
 *      at any time then you have a reader task. If the data gets changed at any
 *      time by the task then you have a writer task.
 */

#ifndef GUCEF_MT_DVRWLOCK_H
#define GUCEF_MT_DVRWLOCK_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_ETYPES_H
#include "gucefMT_ETypes.h"             /* simple types used */
#define GUCEF_MT_ETYPES_H
#endif /* GUCEF_MT_ETYPES_H ? */

#ifndef GUCEF_MT_MACROS_H
#include "gucefMT_macros.h"     /* often used gucef macros */
#define GUCEF_MT_MACROS_H
#endif /* GUCEF_MT_MACROS_H ? */

#ifndef GUCEF_MT_LOCKSTATUS_H
#include "gucefMT_LockStatus.h"
#define GUCEF_MT_LOCKSTATUS_H
#endif /* GUCEF_MT_LOCKSTATUS_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"      /* TThreadId */
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *	Storage used for the readers/writers lock.
 */
struct SRWLock;
typedef struct SRWLock TRWLock;

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

#define GUCEF_RWLOCK_OPERATION_FAILED       GUCEF_LOCKSTATUS_OPERATION_FAILED
#define GUCEF_RWLOCK_OPERATION_SUCCESS      GUCEF_LOCKSTATUS_OPERATION_SUCCESS
#define GUCEF_RWLOCK_WAIT_TIMEOUT           GUCEF_LOCKSTATUS_WAIT_TIMEOUT
#define GUCEF_RWLOCK_ABANDONED              GUCEF_LOCKSTATUS_ABANDONED

#define GUCEF_RWLOCK_INFINITE_TIMEOUT       GUCEF_MT_INFINITE_LOCK_TIMEOUT

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
 *      Function that creates a readers/writers lock data storage struct
 *      writer_overrules is a boolean. When non zero writers will be given
 *      a higher priority then readers. If 0 then readers will have priority
 *      over writers.
 */
GUCEF_MT_PUBLIC_C TRWLock*
rwl_create( UInt32 writer_overrules );

/*-------------------------------------------------------------------------*/

/**
 *      Cleanup a readers/writers lock.
 *      This function should only be called when all threads using the lock have
 *      finished using it. There are some safeguards but they are not fullproof.
 */
GUCEF_MT_PUBLIC_C void
rwl_destroy( TRWLock *rwlock );

/*-------------------------------------------------------------------------*/

/**
 *      Returns a boolean value. 1 if writers have priority and 0 if readers
 *      have priority.
 */
GUCEF_MT_PUBLIC_C UInt32
rwl_writer_overrules( const TRWLock *rwlock );

/*-------------------------------------------------------------------------*/

/**
 *  Returns the current amount of active writers. 
 *  Note that this function is meant for output to humans in an informational capacity as it is a snapshot-in-time
 *  It has no use in relation with the other rwl_ functions.
 */
GUCEF_MT_PUBLIC_C UInt32
rwl_active_writers( const TRWLock *rwlock );

/*-------------------------------------------------------------------------*/

/**
 *  Returns the current reentrancy depth of the currently active writer.
 *  If there is no active writer 0 is always returned
 *  Note that this function is meant for output to humans in an informational capacity as it is a snapshot-in-time
 *  It has no use in relation with the other rwl_ functions.
 */
GUCEF_MT_PUBLIC_C UInt32
rwl_active_writer_reentrancy_depth( const TRWLock *rwlock );

/*-------------------------------------------------------------------------*/

/**
 *  Returns the current amount of queued writers. 
 *  Note that this function is meant for output to humans in an informational capacity as it is a snapshot-in-time
 *  It has no use in relation with the other rwl_ functions.
 */
GUCEF_MT_PUBLIC_C UInt32
rwl_queued_writers( const TRWLock *rwlock );

/*-------------------------------------------------------------------------*/

/**
 *  Returns the current amount of active readers. 
 *  Note that this function is meant for output to humans in an informational capacity as it is a snapshot-in-time
 *  It has no use in relation with the other rwl_ functions.
 */
GUCEF_MT_PUBLIC_C UInt32
rwl_active_readers( const TRWLock *rwlock );

/*-------------------------------------------------------------------------*/

/**
 *  Returns the current amount of queued readers. 
 *  Note that this function is meant for output to humans in an informational capacity as it is a snapshot-in-time
 *  It has no use in relation with the other rwl_ functions.
 */
GUCEF_MT_PUBLIC_C UInt32
rwl_queued_readers( const TRWLock *rwlock );

/*-------------------------------------------------------------------------*/

/**
 *  Returns the current reentrancy depth of the currently active reader.
 *  If there is no active reader 0 is always returned
 *  Note that this function is meant for output to humans in an informational capacity as it is a snapshot-in-time
 *  It has no use in relation with the other rwl_ functions.
 */
GUCEF_MT_PUBLIC_C UInt32
rwl_active_reader_reentrancy_depth( const TRWLock *rwlock, TThreadId threadId );

/*-------------------------------------------------------------------------*/

/**
 *  Returns the operating system's id (GetCurrentThreadSystemID()) of the active reader with the given identity
 *  (GetCurrentThreadID()), 0 if that thread does not hold a read lock. For display, see GUCEF_SYSTEM_THREAD_ID_IS_THREAD_ID:
 *  where it is 1 this is the identity itself; where gucefMT is built with GUCEF_MT_RECORD_SYSTEM_THREAD_ID set to 0
 *  it is 0. A snapshot in time like rwl_active_reader_reentrancy_depth().
 */
GUCEF_MT_PUBLIC_C TSystemThreadId
rwl_active_reader_system_thread_id( const TRWLock *rwlock, TThreadId threadId );

/*-------------------------------------------------------------------------*/

/**
 *  Returns the identity (GetCurrentThreadID()) of the thread that holds the write lock or, once released, held it
 *  last; 0 if no thread ever held it. A snapshot in time for diagnostics.
 */
GUCEF_MT_PUBLIC_C TThreadId
rwl_last_writer_thread_id( const TRWLock *rwlock );

/*-------------------------------------------------------------------------*/

/**
 *  Returns the operating system's id of the thread rwl_last_writer_thread_id() reports, for display; see
 *  rwl_active_reader_system_thread_id() for the values where the ids are the same or not recorded.
 */
GUCEF_MT_PUBLIC_C TSystemThreadId
rwl_last_writer_system_thread_id( const TRWLock *rwlock );

/*--------------------------------------------------------------------------*/

/**
 *      Call this function when a reader task needs access to the data
 *      protected by the given rwlock. Fails if destroy has been called for
 *      the given rwlock. If it fails you should no longer access the given
 *      rwlock.
 */
GUCEF_MT_PUBLIC_C UInt32
rwl_reader_start( TRWLock *rwlock, UInt32 lockWaitTimeoutInMs );

/*--------------------------------------------------------------------------*/

/**
 *  Call this function when a reader task finished using data that is
 *  protected by the given rwlock.
 *  A return value of 1 indicates success and 0 failure.
 */
GUCEF_MT_PUBLIC_C UInt32
rwl_reader_stop( TRWLock *rwlock );

/*--------------------------------------------------------------------------*/

/**
 *  Call this function when a reader task now needs write access to the data
 *  protected by the given rwlock. You should have already aquired the read lock
 *  using rwl_reader_start before using this.
 */
GUCEF_MT_PUBLIC_C UInt32
rwl_reader_transition_to_writer( TRWLock *rwlock, UInt32 lockWaitTimeoutInMs );

/*--------------------------------------------------------------------------*/

/**
 *  Call this function when a writer task no longer needs write access to the data
 *  protected by the given rwlock but would like to retain read-only access. 
 *  You should have already aquired the write lock from the calling thread before using this.
 */
GUCEF_MT_PUBLIC_C UInt32
rwl_writer_transition_to_reader( TRWLock *rwlock, UInt32 lockWaitTimeoutInMs );

/*--------------------------------------------------------------------------*/

/**
 *      Call this function when a writer task needs access to the data
 *      protected by the given rwlock. Fails if destroy has been called for
 *      the given rwlock. If it fails you should no longer access the given
 *      rwlock. A return value of 1 indicates success and 0 failure.
 */
GUCEF_MT_PUBLIC_C UInt32
rwl_writer_start( TRWLock *rwlock, UInt32 lockWaitTimeoutInMs );

/*--------------------------------------------------------------------------*/

/**
 *  Call this function when a writer task finished using data that is
 *  protected by the given rwlock.
 *  A return value of 1 indicates success and 0 failure.
 */
GUCEF_MT_PUBLIC_C UInt32
rwl_writer_stop( TRWLock *rwlock );

/*--------------------------------------------------------------------------*/

/**
 *  If you are killing threads you might leave the read-write lock with an incorrect
 *  administration of which threads are using the lock. If the killed thread was capable
 *  of using the lock, use this function to signal that a cleanup sanity check should be
 *  performed, checking for dangling references to said thread.
 */
GUCEF_MT_PUBLIC_C void
rwl_signal_thread_killed( TRWLock *rwlock, TThreadId killedThreadId );

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
}; /* namespace MT */
}; /* namespace GUCEF */
#endif /* __cplusplus ? */

/*--------------------------------------------------------------------------*/

#endif /* GUCEF_MT_DVRWLOCK_H ? */
