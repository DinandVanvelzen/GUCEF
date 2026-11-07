/*
 *  gucefMULTIPLAY: GUCEF module providing multiplayer RAD functionality
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

#ifndef GUCEF_CORE_CINOTIFIER_H
#include "gucefCORE_CINotifier.h"
#define GUCEF_CORE_CINOTIFIER_H
#endif /* GUCEF_CORE_CINOTIFIER_H ? */

#include "gucefMULTIPLAY_CAsyncLobbyListingProvider.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MULTIPLAY {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CAsyncLobbyListingProvider::CAsyncLobbyListingProvider( CIAsyncLobbyBackend& backend, const CORE::CString& providerName )
    : CIGameListingProvider()
    , m_backend( &backend )
    , m_providerName( providerName )
    , m_active( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAsyncLobbyListingProvider::~CAsyncLobbyListingProvider()
{GUCEF_TRACE;

    UnsubscribeAllFromObserver();
}

/*-------------------------------------------------------------------------*/

void
CAsyncLobbyListingProvider::Synchronize( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_backend )
        return;
    CIAsyncLobbyBackend::TGameListingEntryVector rows;
    m_backend->GetListings( rows );
    CORE::CString::StringSet keep;

    // the rows that come together are one batch: a browser that lists them looks at its whole list once, not once for every row
    BeginBatch();
    for ( size_t i=0; i<rows.size(); ++i )
    {
        if ( !GetFilter().Matches( rows[ i ] ) )
            continue;
        keep.insert( rows[ i ].listingId );
        if ( !UpsertListing( rows[ i ] ) )
            return;
    }
    RemoveListingsExcept( keep );
    EndBatch();
}

/*-------------------------------------------------------------------------*/

bool
CAsyncLobbyListingProvider::Start( const CGameListingFilter& filter )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_backend )
        return false;
    SetFilter( filter );
    if ( !m_active )
    {
        TEventCallback callback( this, &CAsyncLobbyListingProvider::OnBackendListingsChanged );
        SubscribeTo( m_backend, CIAsyncLobbyBackend::ListingsChangedEvent, callback );
        m_active = true;
    }
    Synchronize();
    return true;
}

/*-------------------------------------------------------------------------*/

void
CAsyncLobbyListingProvider::Stop( void )
{GUCEF_TRACE;

    if ( !m_active )
        return;
    m_active = false;
    if ( GUCEF_NULL != m_backend )
        UnsubscribeFrom( m_backend );
    RemoveAllListings();
}

/*-------------------------------------------------------------------------*/

bool
CAsyncLobbyListingProvider::Refresh( void )
{GUCEF_TRACE;

    if ( !m_active || GUCEF_NULL == m_backend )
        return false;
    const bool asked = m_backend->Refresh();
    Synchronize();
    return asked;
}

/*-------------------------------------------------------------------------*/

bool
CAsyncLobbyListingProvider::IsActive( void ) const
{GUCEF_TRACE;

    return m_active;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CAsyncLobbyListingProvider::GetProviderName( void ) const
{GUCEF_TRACE;

    return m_providerName;
}

/*-------------------------------------------------------------------------*/

void
CAsyncLobbyListingProvider::OnBackendListingsChanged( CORE::CNotifier* notifier    ,
                                                      const CORE::CEvent& eventId  ,
                                                      CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( m_active )
        Synchronize();
}

/*-------------------------------------------------------------------------*/

void
CAsyncLobbyListingProvider::OnNotify( CORE::CNotifier* notifier    ,
                                      const CORE::CEvent& eventId  ,
                                      CORE::CICloneable* eventData )
{GUCEF_TRACE;

    // the standard events do not reach the handlers of the subscriptions
    if ( GUCEF_NULL != m_backend && notifier == m_backend && CORE::CINotifier::DestructionEvent == eventId )
    {
        m_backend = GUCEF_NULL;
        m_active = false;
        RemoveAllListings();
    }
    CIGameListingProvider::OnNotify( notifier, eventId, eventData );
}

/*-------------------------------------------------------------------------*/

bool
CAsyncLobbyListingProvider::SaveConfig( CORE::CDataNode& /* config */ ) const
{GUCEF_TRACE;

    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAsyncLobbyListingProvider::LoadConfig( const CORE::CDataNode& /* config */ )
{GUCEF_TRACE;

    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CAsyncLobbyListingProvider::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::MULTIPLAY::CAsyncLobbyListingProvider";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MULTIPLAY */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
