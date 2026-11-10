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
    MT::CScopeMutex lock( m_lock );
    for ( UInt32 i=0; i<m_entries.size(); ++i )
    {
        if ( m_entries[ i ].contract->GetHash() == contract->GetHash() && m_entries[ i ].service == service )
        {
            m_entries[ i ].impl = impl;
            m_entries[ i ].mode = mode;
            return true;
        }
    }
    SEntry entry;
    entry.contract = contract;
    entry.service = service;
    entry.impl = impl;
    entry.mode = mode;
    m_entries.push_back( entry );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CRpcServiceSet::UnregisterService( const CRpcContractPtr& contract, const CORE::CString& serviceName )
{GUCEF_TRACE;

    if ( contract.IsNULL() )
        return false;
    const CRpcServiceDescriptor* service = contract->FindService( serviceName );
    MT::CScopeMutex lock( m_lock );
    for ( UInt32 i=0; i<m_entries.size(); ++i )
    {
        if ( m_entries[ i ].contract->GetHash() == contract->GetHash() && m_entries[ i ].service == service )
        {
            m_entries.erase( m_entries.begin() + i );
            return true;
        }
    }
    return false;
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
