/*
 *  gucefAUTH: GUCEF module providing player and service authentication through external identity providers
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

#include "gucefAUTH_CMemoryCredentialStore.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUTH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CMemoryCredentialStore::CMemoryCredentialStore( void )
    : ICredentialStore()
    , m_lock()
    , m_slots()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMemoryCredentialStore::~CMemoryCredentialStore()
{GUCEF_TRACE;

    Wipe();
}

/*-------------------------------------------------------------------------*/

bool
CMemoryCredentialStore::Store( const CORE::CString& providerName ,
                               const CORE::CString& accountKey   ,
                               const CSecretString& secret       )
{GUCEF_TRACE;

    if ( providerName.IsNULLOrEmpty() || accountKey.IsNULLOrEmpty() )
        return false;
    MT::CScopeMutex lock( m_lock );
    for ( size_t i=0; i<m_slots.size(); ++i )
    {
        if ( m_slots[ i ].providerName == providerName && m_slots[ i ].accountKey == accountKey )
        {
            m_slots.erase( m_slots.begin() + i );
            break;
        }
    }
    SSlot slot;
    slot.providerName = providerName;
    slot.accountKey = accountKey;
    slot.secret = secret;
    m_slots.push_back( slot );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMemoryCredentialStore::Load( const CORE::CString& providerName ,
                              const CORE::CString& accountKey   ,
                              CSecretString& secret             )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    for ( size_t i=0; i<m_slots.size(); ++i )
    {
        if ( m_slots[ i ].providerName == providerName && m_slots[ i ].accountKey == accountKey )
        {
            secret = m_slots[ i ].secret;
            return true;
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CMemoryCredentialStore::Remove( const CORE::CString& providerName ,
                                const CORE::CString& accountKey   )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    for ( size_t i=0; i<m_slots.size(); ++i )
    {
        if ( m_slots[ i ].providerName == providerName && m_slots[ i ].accountKey == accountKey )
        {
            m_slots.erase( m_slots.begin() + i );
            break;
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMemoryCredentialStore::ListAccounts( const CORE::CString& providerName        ,
                                      CORE::CString::StringVector& accountKeys )
{GUCEF_TRACE;

    accountKeys.clear();
    MT::CScopeMutex lock( m_lock );
    // the vector is in order of storing, the slot stored last first
    for ( size_t i=m_slots.size(); i>0; --i )
        if ( m_slots[ i-1 ].providerName == providerName )
            accountKeys.push_back( m_slots[ i-1 ].accountKey );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMemoryCredentialStore::RemoveProvider( const CORE::CString& providerName )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    TSlotVector kept;
    for ( size_t i=0; i<m_slots.size(); ++i )
        if ( m_slots[ i ].providerName != providerName )
            kept.push_back( m_slots[ i ] );
    m_slots = kept;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMemoryCredentialStore::Wipe( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_slots.clear();
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUTH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
