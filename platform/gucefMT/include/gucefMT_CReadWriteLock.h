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

#ifndef GUCEF_MT_CREADWRITELOCK_H
#define GUCEF_MT_CREADWRITELOCK_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_ETYPES_H
#include "gucefMT_ETypes.h"           /* simple types used */
#define GUCEF_MT_ETYPES_H
#endif /* GUCEF_MT_ETYPES_H ? */

#ifndef GUCEF_MT_MACROS_H
#include "gucefMT_macros.h"     /* often used gucef macros */
#define GUCEF_MT_MACROS_H
#endif /* GUCEF_MT_MACROS_H ? */

#ifndef GUCEF_MT_CILOCKABLE_H
#include "gucefMT_CILockable.h"
#define GUCEF_MT_CILOCKABLE_H
#endif /* GUCEF_MT_CILOCKABLE_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *      Forward decalaration of the storage structure for the actual readers
 *      writers lock implementation.
 */
struct SRWLock;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MT {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *      Wrapper class for my C implementation of a readers/writers lock
 */
class GUCEF_MT_PUBLIC_CPP CReadWriteLock : public CILockable
{
    public:

    enum ERWLockStates
    {
        RWLOCK_OPERATION_FAILED     = GUCEF_LOCKSTATUS_OPERATION_FAILED,
        RWLOCK_OPERATION_SUCCESS    = GUCEF_LOCKSTATUS_OPERATION_SUCCESS,
        RWLOCK_WAIT_TIMEOUT         = GUCEF_LOCKSTATUS_WAIT_TIMEOUT,
        RWLOCK_ABANDONED            = GUCEF_LOCKSTATUS_ABANDONED
    };
    typedef enum ERWLockStates TRWLockStates;

    /**
     *      Creates the readers/writers lock with the given priority setting
     *      @param writers_overrule Wheter writers or readers have priority.
     */
    CReadWriteLock( bool writers_overrule );

    /**
     *      Creates a new independent reader writer lock but copies the settings from the existing one
     */
    CReadWriteLock( const CReadWriteLock& src );

    #ifdef GUCEF_RVALUE_REFERENCES_SUPPORTED
    CReadWriteLock( CReadWriteLock&& src ) GUCEF_NOEXCEPT;
    #endif

    /**
     *      Cleans up the lock.
     *      The destructor will block untill all readers and writers are
     *      finished !!!.
     */
    virtual ~CReadWriteLock();

    /**
     *      Use when a writer task needs access to the data protected by
     *      the given lock. Blocks untill access is granted.
     *      Returns wheter the operation was successfull.
     *      In case of failure it returns imediatly and the return value
     *      will be false. In such a case the lock should no longer be used.
     */
    TRWLockStates WriterStart( UInt32 lockWaitTimeoutInMs = GUCEF_MT_DEFAULT_LOCK_TIMEOUT_IN_MS ) const;

    /**
     *      Use when a writer task finished using data/code that is
     *      protected by this lock.
     */
    TRWLockStates WriterStop( void ) const;

    /**
     *  Returns the current number of active writers. Note that this function
     *  is meant for output to humans. Race conditions ect. make it
     *  useless for any control code.
     *
     *  Note that bugs aside this should logically always return 0 or 1
     */
    UInt32 ActiveWriterCount( void ) const;

    /**
     *  Returns the rentrancy depth of the currently active writer, if any. 
     *  Note that this function is meant for output to humans. Race conditions ect. make it
     *  useless for any control code.    
     */
    UInt32 ActiveWriterReentrancyDepth( void ) const;

    /**
     *  Returns the rentrancy depth of the currently active reader with the given thread id, if any. 
     *  Note that this function is meant for output to humans. Race conditions ect. make it
     *  useless for any control code.    
     */
    UInt32 ActiveReaderReentrancyDepth( UInt32 threadId ) const;

    /**
     *      Returns the current number of writers. Note that this function
     *      is meant for output to humans. Race conditions ect. make it
     *      useless for any control code.
     */
    UInt32 QueuedWriterCount( void ) const;

    /**
     *      Use when a reader task needs access to the data protected by
     *      the given lock. Blocks untill access is granted (if a writer is
     *      currently busy). Returns wheter the operation was successfull.
     *      In case of failure it returns imediatly and the return value
     *      will be false. In such a case the lock should no longer be used.
     */
    TRWLockStates ReaderStart( UInt32 lockWaitTimeoutInMs = GUCEF_MT_DEFAULT_LOCK_TIMEOUT_IN_MS ) const;

    /**
     *      Use when a reader task finished using data/code that is
     *      protected by this lock.
     */
    TRWLockStates ReaderStop( void ) const;

    TRWLockStates TransitionReaderToWriter( UInt32 lockWaitTimeoutInMs = GUCEF_MT_DEFAULT_LOCK_TIMEOUT_IN_MS ) const;

    TRWLockStates TransitionWriterToReader( UInt32 lockWaitTimeoutInMs = GUCEF_MT_DEFAULT_LOCK_TIMEOUT_IN_MS ) const;

    /**
     *      Returns the current number of active readers. Note that this function
     *      is meant for output to humans. Race conditions ect. make it
     *      useless for any control code.
     */
    UInt32 ActiveReaderCount( void ) const;

    /**
     *      Returns the current number of queued readers. Note that this function
     *      is meant for output to humans. Race conditions ect. make it
     *      useless for any control code.
     */
    UInt32 QueuedReaderCount( void ) const;

    /**
     *      Returns wheter readers or writers have prioritity.
     *      True in case the writers have priority and false if the readers
     *      have priority.
     */
    bool DoWritersOverrule( void ) const;

    virtual const CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE;

    static TRWLockStates RwLockIntStateToEnum( UInt32 intState );

    static bool RwLockStateToLockOpBool( TRWLockStates state );

    static TLockStatus RwLockStateToLockStatus( TRWLockStates state );

    /**
     *  Should be implemented by the derived classes such that all interactions with the class
     *  member functions and variables are protected against multiple threads accessing them
     *  Typical implementation would be to have this call Lock() on a mutex member in a derived class
     */
    virtual TLockStatus Lock( UInt32 lockWaitTimeoutInMs = GUCEF_MT_DEFAULT_LOCK_TIMEOUT_IN_MS ) const GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  Counterpart to the Lock() member function. This releases the lock obtained using Lock() 
     *  Typical implementation would be to have this call Unlock() on a mutex member in a derived class
     */
    virtual TLockStatus Unlock( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  Should be implemented by the derived classes such that all interactions with the class
     *  member functions and variables are protected against multiple threads accessing them
     *  Typical implementation would be to have this call Lock() as a reader on a reader-writer lock in a derived class
     */
    virtual TLockStatus ReadOnlyLock( UInt32 lockWaitTimeoutInMs = GUCEF_MT_DEFAULT_LOCK_TIMEOUT_IN_MS ) const GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  Counterpart to the ReadOnlyLock() member function. This releases the lock obtained using ReadOnlyLock() 
     *  Typical implementation would be to have this call Unlock() as a reader on a reader-writer lock in a derived class
     */
    virtual TLockStatus ReadOnlyUnlock( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:
    mutable struct SRWLock* _rwlock;          /* encapsulated rwlock struct */

    CReadWriteLock( void );                   /* can't use this: need data */
    CReadWriteLock& operator=( const CReadWriteLock& );  /* copy's arent allowed */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MT */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MT_CREADWRITELOCK_H ? */
