/*
 *  gucefASSETSTORE: GUCEF module for online asset stores and the primary asset store
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

#ifndef GUCEF_ASSETSTORE_CWEBSIGNINCREDENTIALS_H
#include "gucefASSETSTORE_CWebSignInCredentials.h"
#define GUCEF_ASSETSTORE_CWEBSIGNINCREDENTIALS_H
#endif /* GUCEF_ASSETSTORE_CWEBSIGNINCREDENTIALS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CWebSignInCredentials::CredentialProviderName = "assetstore-web-credentials";

/*-------------------------------------------------------------------------*/

CWebSignInCredentials::CWebSignInCredentials( void )
    : m_slots()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CWebSignInCredentials::CWebSignInCredentials( const CWebSignInCredentials& src )
    : m_slots( src.m_slots )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CWebSignInCredentials::~CWebSignInCredentials()
{GUCEF_TRACE;

    Clear();
}

/*-------------------------------------------------------------------------*/

CWebSignInCredentials&
CWebSignInCredentials::operator=( const CWebSignInCredentials& src )
{GUCEF_TRACE;

    if ( this != &src )
    {
        Clear();
        m_slots = src.m_slots;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

void
CWebSignInCredentials::Set( const CORE::CString& slot, const AUTH::CSecretString& value )
{GUCEF_TRACE;

    m_slots[ slot ] = value;
}

/*-------------------------------------------------------------------------*/

bool
CWebSignInCredentials::Get( const CORE::CString& slot, AUTH::CSecretString& value ) const
{GUCEF_TRACE;

    TSlotMap::const_iterator i = m_slots.find( slot );
    if ( i == m_slots.end() || (*i).second.IsEmpty() )
        return false;
    value = (*i).second;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CWebSignInCredentials::Has( const CORE::CString& slot ) const
{GUCEF_TRACE;

    TSlotMap::const_iterator i = m_slots.find( slot );
    return i != m_slots.end() && !(*i).second.IsEmpty();
}

/*-------------------------------------------------------------------------*/

void
CWebSignInCredentials::Remove( const CORE::CString& slot )
{GUCEF_TRACE;

    TSlotMap::iterator i = m_slots.find( slot );
    if ( i != m_slots.end() )
    {
        (*i).second.Clear();
        m_slots.erase( i );
    }
}

/*-------------------------------------------------------------------------*/

void
CWebSignInCredentials::Clear( void )
{GUCEF_TRACE;

    for ( TSlotMap::iterator i = m_slots.begin(); i != m_slots.end(); ++i )
        (*i).second.Clear();
    m_slots.clear();
}

/*-------------------------------------------------------------------------*/

UInt32
CWebSignInCredentials::GetCount( void ) const
{GUCEF_TRACE;

    return (UInt32) m_slots.size();
}

/*-------------------------------------------------------------------------*/

bool
CWebSignInCredentials::HasAll( const CORE::CString::StringVector& slots ) const
{GUCEF_TRACE;

    for ( size_t i=0; i<slots.size(); ++i )
        if ( !Has( slots[ i ] ) )
            return false;
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CWebSignInCredentials::AccountKeyOf( const CORE::CString& storeId, const CORE::CString& slot )
{GUCEF_TRACE;

    return storeId + "/" + slot;
}

/*-------------------------------------------------------------------------*/

bool
CWebSignInCredentials::SaveTo( AUTH::ICredentialStore& store, const CORE::CString& storeId ) const
{GUCEF_TRACE;

    bool success = true;
    for ( TSlotMap::const_iterator i = m_slots.begin(); i != m_slots.end(); ++i )
        success = store.Store( CredentialProviderName, AccountKeyOf( storeId, (*i).first ), (*i).second ) && success;
    return success;
}

/*-------------------------------------------------------------------------*/

bool
CWebSignInCredentials::LoadFrom( AUTH::ICredentialStore& store, const CORE::CString& storeId, const CORE::CString::StringVector& slots )
{GUCEF_TRACE;

    Clear();
    for ( size_t i=0; i<slots.size(); ++i )
    {
        AUTH::CSecretString value;
        if ( store.Load( CredentialProviderName, AccountKeyOf( storeId, slots[ i ] ), value ) && !value.IsEmpty() )
            m_slots[ slots[ i ] ] = value;
        value.Clear();
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CWebSignInCredentials::RemoveFrom( AUTH::ICredentialStore& store, const CORE::CString& storeId, const CORE::CString::StringVector& slots )
{GUCEF_TRACE;

    bool success = true;
    for ( size_t i=0; i<slots.size(); ++i )
        success = store.Remove( CredentialProviderName, AccountKeyOf( storeId, slots[ i ] ) ) && success;
    return success;
}

/*-------------------------------------------------------------------------*/

bool
CWebSignInCredentials::HasSaved( AUTH::ICredentialStore& store, const CORE::CString& storeId, const CORE::CString::StringVector& slots )
{GUCEF_TRACE;

    if ( slots.empty() )
        return false;
    for ( size_t i=0; i<slots.size(); ++i )
    {
        AUTH::CSecretString value;
        const bool has = store.Load( CredentialProviderName, AccountKeyOf( storeId, slots[ i ] ), value ) && !value.IsEmpty();
        value.Clear();
        if ( !has )
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
