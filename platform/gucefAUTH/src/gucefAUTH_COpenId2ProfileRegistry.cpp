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

#include "gucefAUTH_COpenId2ProfileRegistry.h"

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

COpenId2ProfileRegistry::COpenId2ProfileRegistry( void )
    : m_lock()
    , m_profiles()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

COpenId2ProfileRegistry::~COpenId2ProfileRegistry()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
COpenId2ProfileRegistry::Register( IOpenId2ProviderProfile* profile )
{GUCEF_TRACE;

    if ( GUCEF_NULL == profile || profile->GetProviderName().IsNULLOrEmpty() )
        return false;

    MT::CScopeMutex lock( m_lock );
    if ( m_profiles.find( profile->GetProviderName() ) != m_profiles.end() )
        return false;
    m_profiles[ profile->GetProviderName() ] = profile;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
COpenId2ProfileRegistry::Unregister( const CORE::CString& providerName )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return 0 != m_profiles.erase( providerName );
}

/*-------------------------------------------------------------------------*/

IOpenId2ProviderProfile*
COpenId2ProfileRegistry::Find( const CORE::CString& providerName ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    TProfileMap::const_iterator i = m_profiles.find( providerName );
    return i == m_profiles.end() ? GUCEF_NULL : (*i).second;
}

/*-------------------------------------------------------------------------*/

UInt32
COpenId2ProfileRegistry::GetCount( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return (UInt32) m_profiles.size();
}

/*-------------------------------------------------------------------------*/

void
COpenId2ProfileRegistry::Clear( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_profiles.clear();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUTH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
