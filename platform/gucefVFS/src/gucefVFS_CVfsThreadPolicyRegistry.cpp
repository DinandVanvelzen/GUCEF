/*
 *  gucefVFS: GUCEF module implementing a Virtual File System
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

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#include "gucefVFS_CVfsThreadPolicyRegistry.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace VFS {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CVfsThreadPolicyRegistry::CVfsThreadPolicyRegistry( void )
    : m_lock()
    , m_policies()
    , m_registeredCount( 0 )
    , m_lockAcquisitions( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CVfsThreadPolicyRegistry::~CVfsThreadPolicyRegistry()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CVfsThreadPolicyRegistry::SetThreadPolicy( TThreadId threadId, const CVfsThreadPolicy& policy )
{GUCEF_TRACE;

    m_lockAcquisitions.FetchAdd( 1 );
    MT::CScopeMutex lock( m_lock );

    TPolicyMap::iterator i = m_policies.find( threadId );
    if ( i != m_policies.end() )
    {
        i->second = policy;
        return true;
    }
    if ( m_policies.size() >= MaxThreads )
        return false;

    m_policies[ threadId ] = policy;
    m_registeredCount.Store( (UInt32) m_policies.size() );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CVfsThreadPolicyRegistry::ClearThreadPolicy( TThreadId threadId )
{GUCEF_TRACE;

    m_lockAcquisitions.FetchAdd( 1 );
    MT::CScopeMutex lock( m_lock );

    if ( 0 != m_policies.erase( threadId ) )
        m_registeredCount.Store( (UInt32) m_policies.size() );
}

/*-------------------------------------------------------------------------*/

bool
CVfsThreadPolicyRegistry::GetThreadPolicy( TThreadId threadId, CVfsThreadPolicy& policy ) const
{GUCEF_TRACE;

    if ( 0 == m_registeredCount.Load() )
        return false;

    m_lockAcquisitions.FetchAdd( 1 );
    MT::CScopeMutex lock( m_lock );

    TPolicyMap::const_iterator i = m_policies.find( threadId );
    if ( i == m_policies.end() )
        return false;
    policy = i->second;
    return true;
}

/*-------------------------------------------------------------------------*/

CVfsThreadPolicy
CVfsThreadPolicyRegistry::GetCurrent( void ) const
{GUCEF_TRACE;

    // no thread has a rule: the default, without taking the lock
    if ( 0 == m_registeredCount.Load() )
        return CVfsThreadPolicy();

    CVfsThreadPolicy policy;
    GetThreadPolicy( MT::GetCurrentThreadID(), policy );
    return policy;
}

/*-------------------------------------------------------------------------*/

UInt32
CVfsThreadPolicyRegistry::GetRegisteredThreadCount( void ) const
{GUCEF_TRACE;

    return m_registeredCount.Load();
}

/*-------------------------------------------------------------------------*/

UInt64
CVfsThreadPolicyRegistry::GetLockAcquisitionCount( void ) const
{GUCEF_TRACE;

    return m_lockAcquisitions.Load();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace VFS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
