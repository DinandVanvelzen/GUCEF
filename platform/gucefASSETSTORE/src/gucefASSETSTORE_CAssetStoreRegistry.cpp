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

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */
#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */
#ifndef GUCEF_CORE_CLONEABLES_H
#include "cloneables.h"
#define GUCEF_CORE_CLONEABLES_H
#endif /* GUCEF_CORE_CLONEABLES_H ? */
#include "gucefASSETSTORE_CAssetStoreRegistry.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CEvent CAssetStoreRegistry::ProviderAddedEvent = "GUCEF::ASSETSTORE::CAssetStoreRegistry::ProviderAddedEvent";
const CORE::CEvent CAssetStoreRegistry::ProviderRemovedEvent = "GUCEF::ASSETSTORE::CAssetStoreRegistry::ProviderRemovedEvent";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CAssetStoreRegistry::RegisterEvents( void )
{GUCEF_TRACE;

    ProviderAddedEvent.Initialize();
    ProviderRemovedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CAssetStoreRegistry::CAssetStoreRegistry( CAssetStoreProviderFactory& factory )
    : CORE::CTSGNotifier()
    , m_factory( factory )
    , m_providers()
    , m_lock()
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CAssetStoreRegistry::~CAssetStoreRegistry()
{GUCEF_TRACE;

    SignalUpcomingDestruction();
    RemoveAllProviders();
}

/*-------------------------------------------------------------------------*/

UInt32
CAssetStoreRegistry::RefreshProviders( void )
{GUCEF_TRACE;

    CAssetStoreProviderFactory::TKeySet keys;
    m_factory.ObtainKeySet( keys );
    UInt32 made = 0;
    CAssetStoreProviderFactory::TKeySet::const_iterator i = keys.begin();
    while ( i != keys.end() )
    {
        if ( !HasProvider( (*i).Lowercase() ) )
        {
            TAssetStoreProviderPtr provider = m_factory.Create( (*i) );
            if ( !provider.IsNULL() && AddProvider( provider ) )
                ++made;
        }
        ++i;
    }
    return made;
}

/*-------------------------------------------------------------------------*/

bool
CAssetStoreRegistry::AddProvider( const TAssetStoreProviderPtr& provider )
{GUCEF_TRACE;

    if ( provider.IsNULL() )
        return false;
    const CORE::CString providerId = provider->GetProviderId().Lowercase();
    if ( providerId.IsNULLOrEmpty() )
        return false;
    {
        MT::CScopeMutex lock( m_lock );
        if ( m_providers.end() != m_providers.find( providerId ) )
            return false;
        m_providers[ providerId ] = provider;
    }
    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "AssetStoreRegistry: Added the store \"" << providerId << "\"" );
    CORE::TCloneableString eventData( providerId );
    NotifyObserversFromThread( ProviderAddedEvent, &eventData );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAssetStoreRegistry::RemoveProvider( const CORE::CString& providerId )
{GUCEF_TRACE;

    const CORE::CString id = providerId.Lowercase();
    TAssetStoreProviderPtr provider;
    {
        MT::CScopeMutex lock( m_lock );
        TProviderMap::iterator i = m_providers.find( id );
        if ( i == m_providers.end() )
            return false;
        provider = (*i).second;
        m_providers.erase( i );
    }
    if ( !provider.IsNULL() )
        provider->WaitForAsyncOperations();
    provider.Unlink();
    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "AssetStoreRegistry: Removed the store \"" << id << "\"" );
    CORE::TCloneableString eventData( id );
    NotifyObserversFromThread( ProviderRemovedEvent, &eventData );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CAssetStoreRegistry::RemoveAllProviders( void )
{GUCEF_TRACE;

    CORE::CString::StringVector ids;
    GetProviderIds( ids );
    for ( size_t i=0; i<ids.size(); ++i )
        RemoveProvider( ids[ i ] );
}

/*-------------------------------------------------------------------------*/

bool
CAssetStoreRegistry::TryGetProvider( const CORE::CString& providerId, TAssetStoreProviderPtr& provider ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    TProviderMap::const_iterator i = m_providers.find( providerId.Lowercase() );
    if ( i == m_providers.end() )
        return false;
    provider = (*i).second;
    return !provider.IsNULL();
}

/*-------------------------------------------------------------------------*/

bool
CAssetStoreRegistry::HasProvider( const CORE::CString& providerId ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_providers.end() != m_providers.find( providerId.Lowercase() );
}

/*-------------------------------------------------------------------------*/

void
CAssetStoreRegistry::GetProviderIds( CORE::CString::StringVector& providerIds ) const
{GUCEF_TRACE;

    providerIds.clear();
    MT::CScopeMutex lock( m_lock );
    TProviderMap::const_iterator i = m_providers.begin();
    while ( i != m_providers.end() )
    {
        providerIds.push_back( (*i).first );
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CAssetStoreRegistry::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::ASSETSTORE::CAssetStoreRegistry";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
