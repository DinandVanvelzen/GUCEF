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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_MULTIPLAY_CMULTIPLAYGLOBAL_H
#include "gucefMULTIPLAY_CMultiplayGlobal.h"
#define GUCEF_MULTIPLAY_CMULTIPLAYGLOBAL_H
#endif /* GUCEF_MULTIPLAY_CMULTIPLAYGLOBAL_H ? */

#include "gucefMULTIPLAY_CGameBrowser.h"

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

const CORE::CEvent CGameBrowser::BrowserListChangedEvent = "GUCEF::MULTIPLAY::CGameBrowser::BrowserListChangedEvent";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CBrowserListChange::CBrowserListChange( void )
    : CORE::CICloneable()
    , change( CHANGE_ADDED )
    , listingId()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CBrowserListChange::CBrowserListChange( TChange theChange, const CORE::CString& id )
    : CORE::CICloneable()
    , change( theChange )
    , listingId( id )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CBrowserListChange::~CBrowserListChange()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CBrowserListChange::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CBrowserListChange( *this );
}

/*-------------------------------------------------------------------------*/

void
CGameBrowser::RegisterEvents( void )
{GUCEF_TRACE;

    BrowserListChangedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CGameBrowser::CGameBrowser( void )
    : CORE::CObservingNotifier()
    , CORE::CIConfigurable()
    , m_providers()
    , m_filter()
    , m_pingService()
    , m_pingEnabled( true )
    , m_active( false )
    , m_merged()
    , m_visibleIndex()
{GUCEF_TRACE;

    RegisterEvents();
    SubscribeTo( &m_pingService );
}

/*-------------------------------------------------------------------------*/

CGameBrowser::~CGameBrowser()
{GUCEF_TRACE;

    UnsubscribeAllFromObserver();
    for ( size_t i=0; i<m_providers.size(); ++i )
        m_providers[ i ]->Stop();
    m_pingService.RemoveAllTargets();
}

/*-------------------------------------------------------------------------*/

bool
CGameBrowser::AddProvider( CIGameListingProviderPtr provider )
{GUCEF_TRACE;

    if ( provider.IsNULL() )
        return false;
    for ( size_t i=0; i<m_providers.size(); ++i )
        if ( m_providers[ i ] == provider )
            return true;

    m_providers.push_back( provider );
    SubscribeTo( provider.GetPointerAlways() );
    if ( m_active )
    {
        if ( !provider->Start( m_filter ) )
            return false;
        Rebuild();
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CGameBrowser::AddProvider( const CORE::CString& providerType, const CORE::CDataNode* config )
{GUCEF_TRACE;

    CIGameListingProviderPtr provider = CMultiplayGlobal::Instance()->GetListingProviderFactory().Create( providerType );
    if ( provider.IsNULL() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "GameBrowser: There is no listing provider of type \"" << providerType << "\"" );
        return false;
    }
    if ( GUCEF_NULL != config && !provider->LoadConfig( *config ) )
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "GameBrowser: The config of the listing provider \"" << providerType << "\" has problems" );
    return AddProvider( provider );
}

/*-------------------------------------------------------------------------*/

const CGameBrowser::TProviderVector&
CGameBrowser::GetProviders( void ) const
{GUCEF_TRACE;

    return m_providers;
}

/*-------------------------------------------------------------------------*/

bool
CGameBrowser::Start( void )
{GUCEF_TRACE;

    bool anyStarted = false;
    for ( size_t i=0; i<m_providers.size(); ++i )
    {
        if ( m_providers[ i ]->Start( m_filter ) )
            anyStarted = true;
        else
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "GameBrowser: The listing provider " << m_providers[ i ]->GetProviderName() << " did not start" );
    }
    m_active = true;
    Rebuild();
    return anyStarted;
}

/*-------------------------------------------------------------------------*/

void
CGameBrowser::Stop( void )
{GUCEF_TRACE;

    m_active = false;
    for ( size_t i=0; i<m_providers.size(); ++i )
        m_providers[ i ]->Stop();
    m_pingService.RemoveAllTargets();
    Rebuild();
}

/*-------------------------------------------------------------------------*/

bool
CGameBrowser::Refresh( void )
{GUCEF_TRACE;

    bool anyRefreshed = false;
    for ( size_t i=0; i<m_providers.size(); ++i )
        anyRefreshed = m_providers[ i ]->Refresh() || anyRefreshed;
    return anyRefreshed;
}

/*-------------------------------------------------------------------------*/

bool
CGameBrowser::IsActive( void ) const
{GUCEF_TRACE;

    return m_active;
}

/*-------------------------------------------------------------------------*/

void
CGameBrowser::SetFilter( const CGameListingFilter& filter )
{GUCEF_TRACE;

    m_filter = filter;
    if ( m_active )
    {
        // The providers kept only what passed the old filter
        for ( size_t i=0; i<m_providers.size(); ++i )
            m_providers[ i ]->Start( m_filter );
    }
    Rebuild();
}

/*-------------------------------------------------------------------------*/

const CGameListingFilter&
CGameBrowser::GetFilter( void ) const
{GUCEF_TRACE;

    return m_filter;
}

/*-------------------------------------------------------------------------*/

void
CGameBrowser::GetListings( TGameListingEntryVector& listings ) const
{GUCEF_TRACE;

    listings.reserve( listings.size() + m_visibleIndex.size() );
    for ( size_t i=0; i<m_merged.size(); ++i )
        if ( m_merged[ i ].visible )
            listings.push_back( m_merged[ i ].entry );
}

/*-------------------------------------------------------------------------*/

bool
CGameBrowser::GetListing( const CORE::CString& listingId, CGameListingEntry& listing ) const
{GUCEF_TRACE;

    TIndexMap::const_iterator i = m_visibleIndex.find( listingId );
    if ( i == m_visibleIndex.end() )
        return false;
    listing = m_merged[ (*i).second ].entry;
    return true;
}

/*-------------------------------------------------------------------------*/

UInt32
CGameBrowser::GetListingCount( void ) const
{GUCEF_TRACE;

    return (UInt32) m_visibleIndex.size();
}

/*-------------------------------------------------------------------------*/

void
CGameBrowser::SetPingEnabled( bool pingEnabled )
{GUCEF_TRACE;

    m_pingEnabled = pingEnabled;
    if ( !pingEnabled )
        m_pingService.RemoveAllTargets();
    Rebuild();
}

/*-------------------------------------------------------------------------*/

CPingService&
CGameBrowser::GetPingService( void )
{GUCEF_TRACE;

    return m_pingService;
}

/*-------------------------------------------------------------------------*/

void
CGameBrowser::Rebuild( void )
{GUCEF_TRACE;

    // Merge: the same id, else a shared endpoint, is the same listing
    TMergedVector merged;
    TIndexMap byId;
    TIndexMap byEndpoint;
    if ( m_active )
    {
        for ( size_t p=0; p<m_providers.size(); ++p )
        {
            TGameListingEntryVector listings;
            m_providers[ p ]->GetListings( listings );
            for ( size_t l=0; l<listings.size(); ++l )
            {
                const CGameListingEntry& listing = listings[ l ];
                size_t index = merged.size();
                TIndexMap::iterator found = byId.find( listing.listingId );
                if ( found != byId.end() )
                    index = (*found).second;
                for ( size_t e=0; e<listing.endpoints.size() && index == merged.size(); ++e )
                {
                    found = byEndpoint.find( listing.endpoints[ e ] );
                    if ( found != byEndpoint.end() )
                        index = (*found).second;
                }
                if ( index == merged.size() )
                {
                    TMergedListing newListing;
                    newListing.entry = listing;
                    newListing.visible = false;
                    if ( !listing.endpoints.empty() )
                        newListing.pingTarget = listing.endpoints[ 0 ];
                    merged.push_back( newListing );
                }
                if ( byId.find( listing.listingId ) == byId.end() )
                    byId[ listing.listingId ] = index;
                for ( size_t e=0; e<listing.endpoints.size(); ++e )
                    if ( byEndpoint.find( listing.endpoints[ e ] ) == byEndpoint.end() )
                        byEndpoint[ listing.endpoints[ e ] ] = index;
            }
        }
    }

    // Pings and the filter
    CORE::CString::StringSet targetsInUse;
    for ( size_t i=0; i<merged.size(); ++i )
    {
        TMergedListing& listing = merged[ i ];
        if ( m_pingEnabled && !listing.pingTarget.IsNULLOrEmpty() )
        {
            if ( m_pingService.AddTarget( listing.pingTarget ) )
            {
                targetsInUse.insert( listing.pingTarget );
                listing.entry.pingMs = m_pingService.GetRttMs( listing.pingTarget );
            }
        }
        listing.visible = m_filter.Matches( listing.entry );
    }
    CORE::CString::StringVector targets;
    m_pingService.GetTargets( targets );
    for ( size_t i=0; i<targets.size(); ++i )
        if ( targetsInUse.find( targets[ i ] ) == targetsInUse.end() )
            m_pingService.RemoveTarget( targets[ i ] );

    // What changed for the observers
    TIndexMap visibleIndex;
    TChangeVector changes;
    for ( size_t i=0; i<merged.size(); ++i )
    {
        if ( !merged[ i ].visible )
            continue;
        const CGameListingEntry& listing = merged[ i ].entry;
        if ( visibleIndex.find( listing.listingId ) != visibleIndex.end() )
            continue;
        visibleIndex[ listing.listingId ] = i;

        TIndexMap::iterator old = m_visibleIndex.find( listing.listingId );
        if ( old == m_visibleIndex.end() )
        {
            changes.push_back( CBrowserListChange( CBrowserListChange::CHANGE_ADDED, listing.listingId ) );
        }
        else
        {
            const CGameListingEntry& oldListing = m_merged[ (*old).second ].entry;
            if ( !listing.HasSameContent( oldListing ) || listing.pingMs != oldListing.pingMs )
                changes.push_back( CBrowserListChange( CBrowserListChange::CHANGE_UPDATED, listing.listingId ) );
        }
    }
    TIndexMap::iterator old = m_visibleIndex.begin();
    while ( old != m_visibleIndex.end() )
    {
        if ( visibleIndex.find( (*old).first ) == visibleIndex.end() )
            changes.push_back( CBrowserListChange( CBrowserListChange::CHANGE_REMOVED, (*old).first ) );
        ++old;
    }

    m_merged.swap( merged );
    m_visibleIndex.swap( visibleIndex );
    NotifyChanges( changes );
}

/*-------------------------------------------------------------------------*/

void
CGameBrowser::ApplyPing( const CORE::CString& target, Int32 rttMs )
{GUCEF_TRACE;

    TChangeVector changes;
    for ( size_t i=0; i<m_merged.size(); ++i )
    {
        TMergedListing& listing = m_merged[ i ];
        if ( listing.pingTarget != target || listing.entry.pingMs == rttMs )
            continue;

        listing.entry.pingMs = rttMs;
        bool wasVisible = listing.visible;
        listing.visible = m_filter.Matches( listing.entry );
        const CORE::CString& listingId = listing.entry.listingId;
        if ( listing.visible && wasVisible )
        {
            changes.push_back( CBrowserListChange( CBrowserListChange::CHANGE_UPDATED, listingId ) );
        }
        else
        if ( listing.visible )
        {
            if ( m_visibleIndex.find( listingId ) == m_visibleIndex.end() )
            {
                m_visibleIndex[ listingId ] = i;
                changes.push_back( CBrowserListChange( CBrowserListChange::CHANGE_ADDED, listingId ) );
            }
        }
        else
        if ( wasVisible )
        {
            TIndexMap::iterator found = m_visibleIndex.find( listingId );
            if ( found != m_visibleIndex.end() && (*found).second == i )
            {
                m_visibleIndex.erase( found );
                changes.push_back( CBrowserListChange( CBrowserListChange::CHANGE_REMOVED, listingId ) );
            }
        }
    }
    NotifyChanges( changes );
}

/*-------------------------------------------------------------------------*/

bool
CGameBrowser::NotifyChanges( const TChangeVector& changes )
{GUCEF_TRACE;

    for ( size_t i=0; i<changes.size(); ++i )
    {
        CBrowserListChange change( changes[ i ] );
        if ( !NotifyObservers( BrowserListChangedEvent, &change ) )
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
CGameBrowser::OnNotify( CORE::CNotifier* notifier                 ,
                        const CORE::CEvent& eventId               ,
                        CORE::CICloneable* eventData /* = GUCEF_NULL */ )
{GUCEF_TRACE;

    if ( notifier == &m_pingService )
    {
        if ( CPingService::PingUpdatedEvent == eventId && GUCEF_NULL != eventData )
        {
            const CPingUpdate* update = static_cast< const CPingUpdate* >( eventData );
            ApplyPing( update->target, update->rttMs );
        }
        return;
    }

    if ( CIGameListingProvider::ListingAddedEvent == eventId   ||
         CIGameListingProvider::ListingUpdatedEvent == eventId ||
         CIGameListingProvider::ListingRemovedEvent == eventId  )
    {
        if ( m_active )
            Rebuild();
    }
    else
    if ( CIGameListingProvider::ProviderErrorEvent == eventId )
    {
        const CIGameListingProvider::TProviderErrorEventData* error = static_cast< const CIGameListingProvider::TProviderErrorEventData* >( eventData );
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "GameBrowser: A listing provider reports: " << ( GUCEF_NULL != error ? error->GetData() : CORE::CString::Empty ) );
    }
}

/*-------------------------------------------------------------------------*/

bool
CGameBrowser::SaveConfig( CORE::CDataNode& config ) const
{GUCEF_TRACE;

    config.SetAttribute( "ping", m_pingEnabled );
    for ( size_t i=0; i<m_providers.size(); ++i )
    {
        CORE::CDataNode* providerNode = config.AddChild( "ListingProvider", GUCEF_DATATYPE_OBJECT );
        providerNode->SetAttribute( "type", m_providers[ i ]->GetProviderName() );
        m_providers[ i ]->SaveConfig( *providerNode );
    }
    m_filter.SaveConfig( *config.AddChild( "Filter", GUCEF_DATATYPE_OBJECT ) );
    m_pingService.SaveConfig( *config.AddChild( "PingService", GUCEF_DATATYPE_OBJECT ) );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CGameBrowser::LoadConfig( const CORE::CDataNode& config )
{GUCEF_TRACE;

    m_pingEnabled = config.GetAttributeValueOrChildValueByName( "ping" ).AsBool( m_pingEnabled, true );

    const CORE::CDataNode* filterNode = config.FindChild( "Filter" );
    if ( GUCEF_NULL != filterNode )
        m_filter.LoadConfig( *filterNode );

    const CORE::CDataNode* pingNode = config.FindChild( "PingService" );
    if ( GUCEF_NULL != pingNode )
        m_pingService.LoadConfig( *pingNode );

    bool allMade = true;
    CORE::CDataNode::const_iterator i = config.ConstBegin();
    while ( i != config.ConstEnd() )
    {
        const CORE::CDataNode& child = *(*i);
        ++i;
        if ( "ListingProvider" != child.GetName() )
            continue;
        CORE::CString type = child.GetAttributeValueOrChildValueByName( "type" ).AsString( CORE::CString::Empty, true );
        allMade = AddProvider( type, &child ) && allMade;
    }
    return allMade;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CGameBrowser::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::MULTIPLAY::CGameBrowser";
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
