/*
 *  gucefCOMCORE: GUCEF module providing basic communication facilities
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

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "gucefCOMCORE_CRpcServiceSet.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COMCORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CRpcServiceSet::SEntry::SEntry( void )
    : contract()
    , service( GUCEF_NULL )
    , impl( GUCEF_NULL )
    , mode( RPC_EXECUTION_CALLING_THREAD )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CRpcServiceSet::CRpcServiceSet( void )
    : CORE::CTSharedObjCreator< CRpcServiceSet, MT::CMutex >( this )
    , m_lock()
    , m_entries()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CRpcServiceSet::~CRpcServiceSet()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CRpcServiceSetPtr
CRpcServiceSet::Create( void )
{GUCEF_TRACE;

    CRpcServiceSet* set = GUCEF_NEW CRpcServiceSet();
    return set->CreateSharedPtr();
}

/*-------------------------------------------------------------------------*/

bool
CRpcServiceSet::RegisterService( const CRpcContractPtr& contract, const CORE::CString& serviceName,
                                 CIRpcServiceImpl* impl, TRpcExecutionMode mode )
{GUCEF_TRACE;

    if ( contract.IsNULL() || GUCEF_NULL == impl )
        return false;
    const CRpcServiceDescriptor* service = contract->FindService( serviceName );
    if ( GUCEF_NULL == service )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "RpcServiceSet: " << contract->GetDisplayName() << " has no service \"" << serviceName << "\"" );
        return false;
    }
    CIRpcServiceImpl* replaced = GUCEF_NULL;
    const CRpcServiceDescriptor* replacedService = GUCEF_NULL;
    {
        MT::CScopeMutex lock( m_lock );
        bool found = false;
        for ( UInt32 i=0; i<m_entries.size(); ++i )
        {
            if ( m_entries[ i ].contract->GetHash() == contract->GetHash() && m_entries[ i ].service == service )
            {
                if ( m_entries[ i ].impl != impl )
                {
                    replaced = m_entries[ i ].impl;
                    replacedService = m_entries[ i ].service;
                }
                m_entries[ i ].impl = impl;
                m_entries[ i ].mode = mode;
                found = true;
                break;
            }
        }
        if ( !found )
        {
            SEntry entry;
            entry.contract = contract;
            entry.service = service;
            entry.impl = impl;
            entry.mode = mode;
            m_entries.push_back( entry );
        }
    }

    // the implementation it replaces may go when this returns, as after UnregisterService()
    if ( GUCEF_NULL != replacedService )
        WaitForHandlersOf( replaced, replacedService );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CRpcServiceSet::UnregisterService( const CRpcContractPtr& contract, const CORE::CString& serviceName )
{GUCEF_TRACE;

    if ( contract.IsNULL() )
        return false;
    const CRpcServiceDescriptor* service = contract->FindService( serviceName );
    CIRpcServiceImpl* impl = GUCEF_NULL;
    const CRpcServiceDescriptor* removedService = GUCEF_NULL;
    {
        MT::CScopeMutex lock( m_lock );
        for ( UInt32 i=0; i<m_entries.size(); ++i )
        {
            if ( m_entries[ i ].contract->GetHash() == contract->GetHash() && m_entries[ i ].service == service )
            {
                impl = m_entries[ i ].impl;
                removedService = m_entries[ i ].service;
                m_entries.erase( m_entries.begin() + i );
                break;
            }
        }
    }
    if ( GUCEF_NULL == removedService )
        return false;

    // no handler starts any more (BeginHandler() no longer finds the entry); the ones under way finish first, so the
    // owner may destroy the implementation when this returns
    WaitForHandlersOf( impl, removedService );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CRpcServiceSet::BeginHandler( const CRpcContractHash& hash, const CRpcMethodDescriptor& method, CIRpcServiceImpl* impl )
{GUCEF_TRACE;

    const CRpcServiceDescriptor* service = method.GetService();
    if ( GUCEF_NULL == service || GUCEF_NULL == impl )
        return false;
    const UInt32 serviceNumber = service->GetNumber();
    MT::CScopeMutex lock( m_lock );
    for ( UInt32 i=0; i<m_entries.size(); ++i )
    {
        // the same match as Find(), which chose the implementation when the call arrived
        const SEntry& entry = m_entries[ i ];
        if ( entry.service->GetNumber() == serviceNumber && entry.contract->GetHash() == hash )
        {
            if ( entry.impl != impl )
                return false;
            SRunningHandler running;
            running.impl = impl;
            running.service = entry.service;
            running.threadId = MT::GetCurrentThreadID();
            m_running.push_back( running );
            return true;
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

void
CRpcServiceSet::EndHandler( CIRpcServiceImpl* impl )
{GUCEF_TRACE;

    const TThreadId self = MT::GetCurrentThreadID();
    MT::CScopeMutex lock( m_lock );
    for ( UInt32 i=(UInt32)m_running.size(); i>0; --i )
    {
        // the latest of this thread: a handler may run another one of the same implementation inside it
        if ( m_running[ i-1 ].impl == impl && m_running[ i-1 ].threadId == self )
        {
            m_running.erase( m_running.begin() + ( i-1 ) );
            return;
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CRpcServiceSet::WaitForHandlersOf( CIRpcServiceImpl* impl, const CRpcServiceDescriptor* service )
{GUCEF_TRACE;

    // a handler that unregisters its own service (or one of its thread further up the stack) is not waited for: it
    // cannot finish while this thread waits
    const TThreadId self = MT::GetCurrentThreadID();
    const UInt64 start = MT::PrecisionTickCount();
    const UInt64 ticksPerMs = MT::PrecisionTimerResolution() / 1000;
    UInt64 nextWarningMs = 5000;
    MT::CScopeMutex lock( m_lock );
    while ( true )
    {
        bool running = false;
        for ( UInt32 i=0; i<m_running.size(); ++i )
        {
            if ( m_running[ i ].impl == impl && m_running[ i ].service == service && m_running[ i ].threadId != self )
            {
                running = true;
                break;
            }
        }
        if ( !running )
            return;
        lock.EarlyUnlock();
        const UInt64 waitedMs = ( MT::PrecisionTickCount() - start ) / ( ticksPerMs > 0 ? ticksPerMs : 1 );
        if ( waitedMs >= nextWarningMs )
        {
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_IMPORTANT, "RpcServiceSet: the service " << service->GetFullName() << " is unregistered and waits for its handlers under way ("
                               << waitedMs << " ms so far)" );
            nextWarningMs += 5000;
        }
        MT::PrecisionDelay( 1 );
        lock.ReLock();
    }
}

/*-------------------------------------------------------------------------*/

bool
CRpcServiceSet::Find( const CRpcContractHash& hash, UInt32 serviceNumber, SEntry& entry ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    for ( UInt32 i=0; i<m_entries.size(); ++i )
    {
        if ( m_entries[ i ].service->GetNumber() == serviceNumber && m_entries[ i ].contract->GetHash() == hash )
        {
            entry = m_entries[ i ];
            return true;
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

void
CRpcServiceSet::GetExportedContracts( TRpcContractPtrVector& contracts ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    contracts.clear();
    for ( UInt32 i=0; i<m_entries.size(); ++i )
    {
        bool known = false;
        for ( UInt32 n=0; n<contracts.size() && !known; ++n )
            known = contracts[ n ]->GetHash() == m_entries[ i ].contract->GetHash();
        if ( !known )
            contracts.push_back( m_entries[ i ].contract );
    }
}

/*-------------------------------------------------------------------------*/

CRpcContractPtr
CRpcServiceSet::FindExported( const CRpcContractHash& hash ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    for ( UInt32 i=0; i<m_entries.size(); ++i )
        if ( m_entries[ i ].contract->GetHash() == hash )
            return m_entries[ i ].contract;
    return CRpcContractPtr();
}

/*-------------------------------------------------------------------------*/

CRpcContractPtr
CRpcServiceSet::FindExported( const CORE::CString& name, UInt32 major ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    CRpcContractPtr best;
    for ( UInt32 i=0; i<m_entries.size(); ++i )
    {
        const CRpcContractPtr& c = m_entries[ i ].contract;
        if ( c->GetName() == name && c->GetMajor() == major && ( best.IsNULL() || c->GetMinor() > best->GetMinor() ) )
            best = c;
    }
    return best;
}

/*-------------------------------------------------------------------------*/

bool
CRpcServiceSet::ExportsName( const CORE::CString& name ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    for ( UInt32 i=0; i<m_entries.size(); ++i )
        if ( m_entries[ i ].contract->GetName() == name )
            return true;
    return false;
}

/*-------------------------------------------------------------------------*/

UInt32
CRpcServiceSet::GetServiceCount( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return (UInt32) m_entries.size();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
