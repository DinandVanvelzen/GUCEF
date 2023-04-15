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

#ifndef GUCEF_MT_COBJECTSCOPEREADONLYLOCK_H
#define GUCEF_MT_COBJECTSCOPEREADONLYLOCK_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_TIMEOUT_EXCEPTION_H
#include "gucef_timeout_exception.h"
#define GUCEF_TIMEOUT_EXCEPTION_H
#endif /* GUCEF_TIMEOUT_EXCEPTION_H ? */

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
 *  Leverages the CILockable interface to Lock() and Unlock() the scope of an
 *  entire object while still having the same stack unroll etc advantages of
 *  the CScopeMutex class
 *  Typical usage would be to pass in the 'this' pointer to lock the member function
 *  scope for a object who's class is derived from CILockable
 */
class GUCEF_MT_PUBLIC_CPP CObjectScopeReadOnlyLock
{
    public:

    /**
     *  Read-only Locks if a lockable object is provided
     *  Otherwise ( lockableObject == GUCEF_NULL ) this object will act as a no-op
     *  This allows you to use the same code in a thread-safe or non-threadsafe manner depending on context
     *  Only taking the locking penalty when needed based on said context
     * 
     *  throws timeout_exception if unable to attain the lock before the specified timeout 
     */
    CObjectScopeReadOnlyLock( const CILockable* lockableObject, UInt32 lockWaitTimeoutInMs = GUCEF_MT_DEFAULT_LOCK_TIMEOUT_IN_MS );

    /**
     *  Read-only Locks the lockable object and retaints the lock for the lifespan of this object by default
     *  You can unlock early via the EarlyUnlock() member function
     * 
     *  throws timeout_exception if unable to attain the lock before the specified timeout 
     */
    CObjectScopeReadOnlyLock( const CILockable& lockableObject, UInt32 lockWaitTimeoutInMs = GUCEF_MT_DEFAULT_LOCK_TIMEOUT_IN_MS );

    /**
     *  Read-only Locks if a lockable object is provided
     *  Otherwise ( lockableObject == GUCEF_NULL ) this object will act as a no-op
     *  This allows you to use the same code in a thread-safe or non-threadsafe manner depending on context
     *  Only taking the locking penalty when needed based on said context
     * 
     *  If the lock operation fails for whatever reason it is the caller's responsibility to handle that scenario via
     *  the lockStatus output parameter
     */
    CObjectScopeReadOnlyLock( const CILockable* lockableObject, UInt32 lockWaitTimeoutInMs, TLockStatus& lockStatus );

    /**
     *  Read-only Locks the lockable object and retaints the lock for the lifespan of this object by default
     *  You can unlock early via the EarlyUnlock() member function
     * 
     *  If the lock operation fails for whatever reason it is the caller's responsibility to handle that scenario via
     *  the lockStatus output parameter
     */
    CObjectScopeReadOnlyLock( const CILockable& lockableObject, UInt32 lockWaitTimeoutInMs, TLockStatus& lockStatus );

    ~CObjectScopeReadOnlyLock();

    /**
     *  Returns whether the current lock state based on the Lock() and Unlock()
     *  operations on the lockable object
     */
    bool IsLocked( void ) const;

    /**
     *  Allows you to unlock before the scope lock triggers destruction of the CObjectScopeLock object
     *  Useful for more complex code flows where in most code paths you want to retain the lock except for
     *  a small subset of code path(s)
     */
    bool EarlyReaderUnlock( void );

    private:
    const CILockable* m_lockableObject;
    bool m_isLocked;

    CObjectScopeReadOnlyLock( const CObjectScopeReadOnlyLock& src );              /* Copying doesnt make sense */
    CObjectScopeReadOnlyLock& operator=( const CObjectScopeReadOnlyLock& src );   /* Copying doesnt make sense */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MT */
}; /* namespace GUCEF */

/*--------------------------------------------------------------------------*/

#endif /* GUCEF_MT_COBJECTSCOPEREADONLYLOCK_H ? */
