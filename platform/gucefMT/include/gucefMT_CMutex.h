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

#ifndef GUCEF_MT_CMUTEX_H
#define GUCEF_MT_CMUTEX_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_MACROS_H
#include "gucefMT_macros.h"
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

struct SMutexData;

/*-------------------------------------------------------------------------*/

/**
 *      O/S Mutex wrapper
 */
class GUCEF_MT_PUBLIC_CPP CMutex : public CILockable
{
    public:

    /**
     *      Lock the mutex. If the mutex is already locked the calling
     *      process will have to wait for the mutex to allow a lock.
     *      The return value indicates whether the lock failed or succeeded.
     */
    virtual TLockStatus Lock( UInt32 lockWaitTimeoutInMs = GUCEF_MT_DEFAULT_LOCK_TIMEOUT_IN_MS ) const GUCEF_VIRTUAL_OVERRIDE;

    /**
     *      Unlocks the mutex after a call to Lock(). Other processes
     *      will have the ability to get a mutex lock after this call.
     *      The return value indicates whether the unlock failed or succeeded.
     */
    virtual TLockStatus Unlock( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsLocked( void ) const;

    /**
     *  This member function is only of limited use and should not be relied upon
     *  Its mainly provided for debugging purposes
     */
    virtual UInt32 GetThreadIdOfThreadLastHoldingTheLock( void ) const;

    CMutex( void );

    #ifdef GUCEF_RVALUE_REFERENCES_SUPPORTED
    CMutex( CMutex&& src ) GUCEF_NOEXCEPT;
    #endif

    virtual ~CMutex();

    private:
    friend class CCondition;

    struct SMutexData* _mutexdata;

    CMutex( const CMutex& src );      /* Copying doesnt make sense */
    CMutex& operator=( const CMutex& src );   /* Copying doesnt make sense */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MT */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define GUCEF_MUTEX( varName )       ( GUCEF::MT::CMutex varName; )
#define GUCEF_LOCKMUTEX( varName )   ( varName.Lock(); )
#define GUCEF_UNLOCKMUTEX( varName ) ( varName.Unlock(); )

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MT_CMUTEX_H ? */
