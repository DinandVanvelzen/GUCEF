/*
 *  gucefMT: GUCEF module providing multithreading solutions
 *
 *  Copyright (C) 1998 - 2026.  Dinand Vanvelzen
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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_MT_ATOMIC_BACKENDS_H
#include "gucefMT_atomic_backends.h"
#define GUCEF_MT_ATOMIC_BACKENDS_H
#endif /* GUCEF_MT_ATOMIC_BACKENDS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MT {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#define GUCEF_MT_ATOMIC_LOCKPOOL_SIZE 64

namespace {

/** The locks live here, in the module, so that every user of the lock backend shares the same ones even across shared libraries */
CMutex&
GetPoolMutex( const volatile void* address )
{
    // No GUCEF_TRACE in this file: these are the lowest level primitives and tracing must be free to use them

    static CMutex pool[ GUCEF_MT_ATOMIC_LOCKPOOL_SIZE ];

    // Atomics are at least naturally aligned, so the lowest bits carry no information
    const size_t key = reinterpret_cast< size_t >( address ) >> 3;
    return pool[ ( key ^ ( key >> 6 ) ) % GUCEF_MT_ATOMIC_LOCKPOOL_SIZE ];
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CAtomicLockPool::Lock( const volatile void* address )
{

    // An atomic operation has no way to report a failure, so keep trying until we own the lock
    CMutex& mutex = GetPoolMutex( address );
    for ( ;; )
    {
        const TLockStatus status = mutex.Lock( GUCEF_MT_ULTRA_LONG_LOCK_TIMEOUT );
        if ( LOCKSTATUS_OPERATION_SUCCESS == status || LOCKSTATUS_ABANDONED == status )
            return;
    }
}

/*-------------------------------------------------------------------------*/

void
CAtomicLockPool::Unlock( const volatile void* address )
{

    GetPoolMutex( address ).Unlock();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MT */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
