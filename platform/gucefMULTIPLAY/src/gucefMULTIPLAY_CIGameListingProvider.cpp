/*
 *  gucefMULTIPLAY: GUCEF module providing multiplayer RAD functionality
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

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "gucefMULTIPLAY_CIGameListingProvider.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MULTIPLAY {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CEvent CIGameListingProvider::ListingAddedEvent = "GUCEF::MULTIPLAY::CIGameListingProvider::ListingAddedEvent";
const CORE::CEvent CIGameListingProvider::ListingUpdatedEvent = "GUCEF::MULTIPLAY::CIGameListingProvider::ListingUpdatedEvent";
const CORE::CEvent CIGameListingProvider::ListingRemovedEvent = "GUCEF::MULTIPLAY::CIGameListingProvider::ListingRemovedEvent";
const CORE::CEvent CIGameListingProvider::ProviderErrorEvent = "GUCEF::MULTIPLAY::CIGameListingProvider::ProviderErrorEvent";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CIGameListingProvider::RegisterEvents( void )
{GUCEF_TRACE;

    ListingAddedEvent.Initialize();
    ListingUpdatedEvent.Initialize();
    ListingRemovedEvent.Initialize();
    ProviderErrorEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CIGameListingProvider::CIGameListingProvider( void )
    : CORE::CObservingNotifier()
    , CORE::CIConfigurable()
    , m_filter()
    , m_listings()
    , m_dataLock()
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CIGameListingProvider::~CIGameListingProvider()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CIGameListingProvider::SetFilter( const CGameListingFilter& filter )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_dataLock );
    m_filter = filter;
}

/*-------------------------------------------------------------------------*/

const CGameListingFilter&
CIGameListingProvider::GetFilter( void ) const
{GUCEF_TRACE;

    return m_filter;
}

/*-------------------------------------------------------------------------*/

void
CIGameListingProvider::GetListings( TGameListingEntryVector& listings ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_dataLock );
    listings.reserve( listings.size() + m_listings.size() );
    TListingMap::const_iterator i = m_listings.begin();
    while ( i != m_listings.end() )
    {
        listings.push_back( (*i).second );
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

bool
CIGameListingProvider::GetListing( const CORE::CString& listingId, CGameListingEntry& listing ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_dataLock );
    TListingMap::const_iterator i = m_listings.find( listingId );
    if ( i == m_listings.end() )
        return false;
    listing = (*i).second;
    return true;
}

/*-------------------------------------------------------------------------*/

UInt32
CIGameListingProvider::GetListingCount( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_dataLock );
    return (UInt32) m_listings.size();
}

/*-------------------------------------------------------------------------*/

bool
CIGameListingProvider::UpsertListing( const CGameListingEntry& listing )
{GUCEF_TRACE;

    if ( listing.listingId.IsNULLOrEmpty() )
        return true;

    // The ping limit is for whoever measures the pings, the rest of the filter applies here
    CGameListingFilter filter;
    {
        MT::CScopeMutex lock( m_dataLock );
        filter = m_filter;
    }
    filter.maxPingMs = -1;
    if ( !filter.Matches( listing ) )
        return RemoveListing( listing.listingId );

    const CORE::CEvent* eventToSend = GUCEF_NULL;
    {
        MT::CScopeMutex lock( m_dataLock );
        TListingMap::iterator i = m_listings.find( listing.listingId );
        if ( i == m_listings.end() )
        {
            m_listings[ listing.listingId ] = listing;
            eventToSend = &ListingAddedEvent;
        }
        else
        {
            bool changed = !(*i).second.HasSameContent( listing );
            (*i).second = listing;
            if ( changed )
                eventToSend = &ListingUpdatedEvent;
        }
    }

    if ( GUCEF_NULL != eventToSend )
    {
        TListingEventData eData( listing );
        return NotifyObservers( *eventToSend, &eData );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CIGameListingProvider::RemoveListing( const CORE::CString& listingId )
{GUCEF_TRACE;

    CGameListingEntry removed;
    {
        MT::CScopeMutex lock( m_dataLock );
        TListingMap::iterator i = m_listings.find( listingId );
        if ( i == m_listings.end() )
            return true;
        removed = (*i).second;
        m_listings.erase( i );
    }
    TListingEventData eData( removed );
    return NotifyObservers( ListingRemovedEvent, &eData );
}

/*-------------------------------------------------------------------------*/

bool
CIGameListingProvider::RemoveListingsExcept( const CORE::CString::StringSet& listingIdsToKeep )
{GUCEF_TRACE;

    CORE::CString::StringVector toRemove;
    {
        MT::CScopeMutex lock( m_dataLock );
        TListingMap::iterator i = m_listings.begin();
        while ( i != m_listings.end() )
        {
            if ( listingIdsToKeep.find( (*i).first ) == listingIdsToKeep.end() )
                toRemove.push_back( (*i).first );
            ++i;
        }
    }
    for ( size_t n=0; n<toRemove.size(); ++n )
    {
        if ( !RemoveListing( toRemove[ n ] ) )
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CIGameListingProvider::RemoveAllListings( void )
{GUCEF_TRACE;

    return RemoveListingsExcept( CORE::CString::StringSet() );
}

/*-------------------------------------------------------------------------*/

bool
CIGameListingProvider::ReportProviderError( const CORE::CString& description )
{GUCEF_TRACE;

    GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "GameListingProvider(" << GetProviderName() << "): " << description );
    TProviderErrorEventData eData( description );
    return NotifyObservers( ProviderErrorEvent, &eData );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MULTIPLAY */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
