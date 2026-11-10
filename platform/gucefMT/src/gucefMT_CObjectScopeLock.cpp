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

#include <assert.h>

#include "gucefMT_CObjectScopeLock.h"

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

/**
 *  Whether a lock was actually obtained and thus needs to be released later.
 *  LOCKSTATUS_NOT_APPLICABLE counts as success for the caller but there is nothing to unlock:
 *  unlocking anyway would release a lock this scope never took, for example when the lockable object
 *  only got an underlying lock during the scope (as happens when a shared pointer is assigned)
 */
static bool
WasLockObtained( TLockStatus lockStatus )
{
    return LOCKSTATUS_OPERATION_SUCCESS == lockStatus || LOCKSTATUS_ABANDONED == lockStatus;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CObjectScopeLock::CObjectScopeLock( const CILockable* lockableObject, UInt32 lockWaitTimeoutInMs )
    : m_lockableObject( lockableObject )
    , m_isLocked( false )
    , m_unlockNeeded( false )
{GUCEF_TRACE;

    if ( GUCEF_NULL != lockableObject )    
    {
        TLockStatus lockStatus = m_lockableObject->Lock( lockWaitTimeoutInMs );
        m_isLocked = LockStatusToLockSuccessStatusBool( lockStatus );
        m_unlockNeeded = WasLockObtained( lockStatus );
        if ( lockStatus == LOCKSTATUS_WAIT_TIMEOUT )
            throw timeout_exception();
    }
}

/*--------------------------------------------------------------------------*/

CObjectScopeLock::CObjectScopeLock( const CILockable& lockableObject, UInt32 lockWaitTimeoutInMs )
    : m_lockableObject( &lockableObject )
    , m_isLocked( false )
    , m_unlockNeeded( false )
{GUCEF_TRACE;

    assert( 0 != m_lockableObject );        
    TLockStatus lockStatus = m_lockableObject->Lock( lockWaitTimeoutInMs );
    m_isLocked = LockStatusToLockSuccessStatusBool( lockStatus );
    m_unlockNeeded = WasLockObtained( lockStatus );
    if ( lockStatus == LOCKSTATUS_WAIT_TIMEOUT )
        throw timeout_exception();
}

/*--------------------------------------------------------------------------*/

CObjectScopeLock::CObjectScopeLock( const CILockable* lockableObject, UInt32 lockWaitTimeoutInMs, TLockStatus& lockStatus )
    : m_lockableObject( lockableObject )
    , m_isLocked( false )
    , m_unlockNeeded( false )
{GUCEF_TRACE;

    if ( GUCEF_NULL != lockableObject )    
    {
        lockStatus = m_lockableObject->Lock( lockWaitTimeoutInMs );
        m_isLocked = LockStatusToLockSuccessStatusBool( lockStatus );
        m_unlockNeeded = WasLockObtained( lockStatus );
    }
}

/*--------------------------------------------------------------------------*/

CObjectScopeLock::CObjectScopeLock( const CILockable& lockableObject, UInt32 lockWaitTimeoutInMs, TLockStatus& lockStatus )
    : m_lockableObject( &lockableObject )
    , m_isLocked( false )
    , m_unlockNeeded( false )
{GUCEF_TRACE;

    assert( 0 != m_lockableObject );        
    lockStatus = m_lockableObject->Lock( lockWaitTimeoutInMs );
    m_isLocked = LockStatusToLockSuccessStatusBool( lockStatus );
    m_unlockNeeded = WasLockObtained( lockStatus );
}

/*--------------------------------------------------------------------------*/

CObjectScopeLock::~CObjectScopeLock()
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_lockableObject && m_unlockNeeded )
    {
        m_isLocked = !LockStatusToLockSuccessStatusBool( m_lockableObject->Unlock() );
        m_unlockNeeded = m_isLocked;
    }
}

/*--------------------------------------------------------------------------*/

bool 
CObjectScopeLock::IsLocked( void ) const
{GUCEF_TRACE;

    return m_isLocked;
}

/*--------------------------------------------------------------------------*/

bool 
CObjectScopeLock::EarlyUnlock( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_lockableObject && m_unlockNeeded )
    {
        m_isLocked = !LockStatusToLockSuccessStatusBool( m_lockableObject->Unlock() );
        m_unlockNeeded = m_isLocked;
        return !m_isLocked;
    }
    if ( m_isLocked )
    {
        // LOCKSTATUS_NOT_APPLICABLE: there is no actual lock to release
        m_isLocked = false;
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MT */
}; /* namespace GUCEF */

/*--------------------------------------------------------------------------*/
