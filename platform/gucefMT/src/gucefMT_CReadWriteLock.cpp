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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_DVRWLOCK_H
#include "gucefMT_DVRWLOCK.h"           /* my C implementation of a RW lock */
#define GUCEF_MT_DVRWLOCK_H
#endif /* GUCEF_MT_DVRWLOCK_H ? */

#include "gucefMT_CReadWriteLock.h"     /* definition of the class implemented here */

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

CReadWriteLock::CReadWriteLock( bool writers_overrule )
    : _rwlock( GUCEF_NULL )
{GUCEF_TRACE;

    _rwlock = rwl_create( writers_overrule ? 1 : 0 );
}

/*--------------------------------------------------------------------------*/

CReadWriteLock::CReadWriteLock( const CReadWriteLock& src )
    : _rwlock( GUCEF_NULL )
{GUCEF_TRACE;

    _rwlock = rwl_create( rwl_writer_overrules( src._rwlock ) );
}

/*--------------------------------------------------------------------------*/

CReadWriteLock::CReadWriteLock( CReadWriteLock&& src ) GUCEF_NOEXCEPT      
    : _rwlock( src._rwlock )
{GUCEF_TRACE;

    src._rwlock = GUCEF_NULL;
}

/*--------------------------------------------------------------------------*/

CReadWriteLock::~CReadWriteLock()
{GUCEF_TRACE;

    rwl_destroy( _rwlock );
    _rwlock = GUCEF_NULL;
}

/*--------------------------------------------------------------------------*/

CReadWriteLock::TRWLockStates
CReadWriteLock::RwLockIntStateToEnum( UInt32 intState )
{GUCEF_TRACE;

    switch ( intState )
    {
        case GUCEF_RWLOCK_OPERATION_FAILED:     return ERWLockStates::RWLOCK_OPERATION_FAILED;
        case GUCEF_RWLOCK_OPERATION_SUCCESS:    return ERWLockStates::RWLOCK_OPERATION_SUCCESS;        
        case GUCEF_RWLOCK_WAIT_TIMEOUT:         return ERWLockStates::RWLOCK_WAIT_TIMEOUT;
        case GUCEF_RWLOCK_ABANDONED:            return ERWLockStates::RWLOCK_ABANDONED;
        default: 
            return ERWLockStates::RWLOCK_OPERATION_FAILED;
    }
}

/*--------------------------------------------------------------------------*/

bool 
CReadWriteLock::RwLockStateToLockOpBool( TRWLockStates state )
{GUCEF_TRACE;

    switch ( state )
    {
        case ERWLockStates::RWLOCK_OPERATION_SUCCESS:
        case ERWLockStates::RWLOCK_ABANDONED:
        {
            return true;
        }
        default:
        {
            return false;
        }
    }
}

/*--------------------------------------------------------------------------*/

TLockStatus 
CReadWriteLock::RwLockStateToLockStatus( TRWLockStates state )
{GUCEF_TRACE;

    switch ( state )
    {
        case ERWLockStates::RWLOCK_OPERATION_SUCCESS: 
            return TLockStatus::LOCKSTATUS_OPERATION_SUCCESS;
        case ERWLockStates::RWLOCK_ABANDONED:
            return TLockStatus::LOCKSTATUS_ABANDONED;
        case ERWLockStates::RWLOCK_OPERATION_FAILED: 
            return TLockStatus::LOCKSTATUS_OPERATION_FAILED;
        case ERWLockStates::RWLOCK_WAIT_TIMEOUT: 
            return TLockStatus::LOCKSTATUS_WAIT_TIMEOUT;
            
        default:
        {
            return TLockStatus::LOCKSTATUS_OPERATION_FAILED;
        }
    }
}

/*--------------------------------------------------------------------------*/

CReadWriteLock::TRWLockStates
CReadWriteLock::WriterStart( UInt32 lockWaitTimeoutInMs ) const
{GUCEF_TRACE;

    return RwLockIntStateToEnum( rwl_writer_start( _rwlock, lockWaitTimeoutInMs ) );
}

/*--------------------------------------------------------------------------*/

CReadWriteLock::TRWLockStates
CReadWriteLock::WriterStop( void ) const
{GUCEF_TRACE;

    return RwLockIntStateToEnum( rwl_writer_stop( _rwlock ) );
}

/*--------------------------------------------------------------------------*/

UInt32
CReadWriteLock::ActiveWriterCount( void ) const
{GUCEF_TRACE;

    return rwl_active_writers( _rwlock );
}

/*--------------------------------------------------------------------------*/

UInt32 
CReadWriteLock::ActiveWriterReentrancyDepth( void ) const
{GUCEF_TRACE;

    return rwl_active_writer_reentrancy_depth( _rwlock );
}

/*--------------------------------------------------------------------------*/

UInt32 
CReadWriteLock::ActiveReaderReentrancyDepth( UInt32 threadId ) const
{GUCEF_TRACE;

    return rwl_active_reader_reentrancy_depth( _rwlock, threadId );
}

/*--------------------------------------------------------------------------*/

UInt32
CReadWriteLock::QueuedWriterCount( void ) const
{GUCEF_TRACE;

    return rwl_queued_writers( _rwlock );
}

/*--------------------------------------------------------------------------*/

CReadWriteLock::TRWLockStates
CReadWriteLock::ReaderStart( UInt32 lockWaitTimeoutInMs ) const
{GUCEF_TRACE;

    return RwLockIntStateToEnum( rwl_reader_start( _rwlock, lockWaitTimeoutInMs ) );
}

/*--------------------------------------------------------------------------*/

CReadWriteLock::TRWLockStates
CReadWriteLock::ReaderStop( void ) const
{GUCEF_TRACE;

    return RwLockIntStateToEnum( rwl_reader_stop( _rwlock ) );
}

/*--------------------------------------------------------------------------*/

CReadWriteLock::TRWLockStates 
CReadWriteLock::TransitionReaderToWriter( UInt32 lockWaitTimeoutInMs ) const
{GUCEF_TRACE;

    return RwLockIntStateToEnum( rwl_reader_transition_to_writer( _rwlock, lockWaitTimeoutInMs ) );
}

/*--------------------------------------------------------------------------*/

CReadWriteLock::TRWLockStates
CReadWriteLock::TransitionWriterToReader( UInt32 lockWaitTimeoutInMs ) const
{GUCEF_TRACE;

    return RwLockIntStateToEnum( rwl_writer_transition_to_reader( _rwlock, lockWaitTimeoutInMs ) );
}

/*--------------------------------------------------------------------------*/

UInt32
CReadWriteLock::ActiveReaderCount( void ) const
{GUCEF_TRACE;

    return rwl_active_readers( _rwlock );
}

/*--------------------------------------------------------------------------*/

UInt32
CReadWriteLock::QueuedReaderCount( void ) const
{GUCEF_TRACE;

    return rwl_queued_readers( _rwlock );
}

/*--------------------------------------------------------------------------*/

bool
CReadWriteLock::DoWritersOverrule( void ) const
{GUCEF_TRACE;

    return rwl_writer_overrules( _rwlock ) != 0;
}

/*--------------------------------------------------------------------------*/

const CILockable* 
CReadWriteLock::AsLockable( void ) const
{GUCEF_TRACE;

    return this;
}

/*--------------------------------------------------------------------------*/

TLockStatus 
CReadWriteLock::Lock( UInt32 lockWaitTimeoutInMs ) const
{GUCEF_TRACE;

    return RwLockStateToLockStatus( WriterStart( lockWaitTimeoutInMs ) );
}

/*--------------------------------------------------------------------------*/

TLockStatus
CReadWriteLock::Unlock( void ) const
{GUCEF_TRACE;

    return RwLockStateToLockStatus( WriterStop() );
}

/*--------------------------------------------------------------------------*/

TLockStatus 
CReadWriteLock::ReadOnlyLock( UInt32 lockWaitTimeoutInMs ) const
{GUCEF_TRACE;

    return RwLockStateToLockStatus( ReaderStart( lockWaitTimeoutInMs ) );
}                                                                                  

/*--------------------------------------------------------------------------*/

TLockStatus
CReadWriteLock::ReadOnlyUnlock( void ) const
{GUCEF_TRACE;

    return RwLockStateToLockStatus( ReaderStop() );
}

/*------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MT */
}; /* namespace GUCEF */

/*--------------------------------------------------------------------------*/
