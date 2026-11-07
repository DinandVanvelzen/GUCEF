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

#ifndef GUCEF_MULTIPLAY_CIGAMELISTINGPROVIDER_H
#define GUCEF_MULTIPLAY_CIGAMELISTINGPROVIDER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_COBSERVINGNOTIFIER_H
#include "CObservingNotifier.h"
#define GUCEF_CORE_COBSERVINGNOTIFIER_H
#endif /* GUCEF_CORE_COBSERVINGNOTIFIER_H ? */

#ifndef GUCEF_CORE_CICONFIGURABLE_H
#include "gucefCORE_CIConfigurable.h"
#define GUCEF_CORE_CICONFIGURABLE_H
#endif /* GUCEF_CORE_CICONFIGURABLE_H ? */

#ifndef GUCEF_CORE_CLONEABLES_H
#include "cloneables.h"
#define GUCEF_CORE_CLONEABLES_H
#endif /* GUCEF_CORE_CLONEABLES_H ? */

#ifndef GUCEF_CORE_CTABSTRACTFACTORY_H
#include "CTAbstractFactory.h"
#define GUCEF_CORE_CTABSTRACTFACTORY_H
#endif /* GUCEF_CORE_CTABSTRACTFACTORY_H ? */

#ifndef GUCEF_CORE_CTFACTORY_H
#include "CTFactory.h"
#define GUCEF_CORE_CTFACTORY_H
#endif /* GUCEF_CORE_CTFACTORY_H ? */

#ifndef GUCEF_MULTIPLAY_CGAMELISTINGFILTER_H
#include "gucefMULTIPLAY_CGameListingFilter.h"
#define GUCEF_MULTIPLAY_CGAMELISTINGFILTER_H
#endif /* GUCEF_MULTIPLAY_CGAMELISTINGFILTER_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MULTIPLAY {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  A source of game and lobby listings: the LAN, a lobby service, a list on a web server.
 *
 *  The interface keeps the listings a provider found, so an implementation only has to report what it hears with
 *  UpsertListing() and RemoveListing(). Listings that do not pass the filter given to Start() are not kept,
 *  the ping limit of the filter is left to the browser that measures the pings.
 */
class GUCEF_MULTIPLAY_EXPORT_CPP CIGameListingProvider : public CORE::CObservingNotifier ,
                                                         public CORE::CIConfigurable
{
    public:

    static const CORE::CEvent ListingAddedEvent;        /**< TListingEventData: the listing */
    static const CORE::CEvent ListingUpdatedEvent;      /**< TListingEventData: the listing as it is now */
    static const CORE::CEvent ListingRemovedEvent;      /**< TListingEventData: the listing as it was */
    static const CORE::CEvent ProviderErrorEvent;       /**< TProviderErrorEventData: a description of the problem */
    static const CORE::CEvent ListingsBatchEndedEvent;  /**< no data: a batch of changes (BeginBatch()) ended and something in it changed */

    typedef CGameListingEntry                               TListingEventData;
    typedef CORE::TCloneableString                          TProviderErrorEventData;
    typedef CGameListingList::TGameListingEntryVector       TGameListingEntryVector;

    static void RegisterEvents( void );

    CIGameListingProvider( void );

    virtual ~CIGameListingProvider();

    /** Starts finding listings that pass the filter */
    virtual bool Start( const CGameListingFilter& filter ) = 0;

    /** Stops and forgets the listings */
    virtual void Stop( void ) = 0;

    /** Asks the source again now */
    virtual bool Refresh( void ) = 0;

    virtual bool IsActive( void ) const = 0;

    /** The name the provider is registered with in the factory, for example "lan" */
    virtual const CORE::CString& GetProviderName( void ) const = 0;

    /** Adds the listings currently known to the list */
    virtual void GetListings( TGameListingEntryVector& listings ) const;

    bool GetListing( const CORE::CString& listingId, CGameListingEntry& listing ) const;

    UInt32 GetListingCount( void ) const;

    const CGameListingFilter& GetFilter( void ) const;

    /**
     *  Whether the provider is in the middle of a batch of changes: the events of the listings that arrive now are part of a set that ends with
     *  ListingsBatchEndedEvent, and a browser that has to look at the whole list for every change waits for the end
     */
    bool IsBatching( void ) const;

    protected:

    /** Changes that come together (the rows of a source that was looked at again) are one batch: the events of the listings are sent as before, then ListingsBatchEndedEvent once, when something changed */
    void BeginBatch( void );
    bool EndBatch( void );

    void SetFilter( const CGameListingFilter& filter );

    /**
     *  Records a listing that was found: new ones are reported as added, changed ones as updated
     *
     *  @return false if this object was destroyed by an observer of the events
     */
    bool UpsertListing( const CGameListingEntry& listing );

    /** @return false if this object was destroyed by an observer of the events */
    bool RemoveListing( const CORE::CString& listingId );

    /** Removes the listings whose id is not in the given set */
    bool RemoveListingsExcept( const CORE::CString::StringSet& listingIdsToKeep );

    bool RemoveAllListings( void );

    bool ReportProviderError( const CORE::CString& description );

    private:

    typedef GUCEF::map< CORE::CString, CGameListingEntry > TListingMap;

    CIGameListingProvider( const CIGameListingProvider& src );
    CIGameListingProvider& operator=( const CIGameListingProvider& src );

    private:

    CGameListingFilter m_filter;
    TListingMap m_listings;
    mutable MT::CMutex m_dataLock;
    UInt32 m_batchDepth;
    bool m_batchChanged;
};

/*-------------------------------------------------------------------------*/

typedef CORE::CTAbstractFactory< CORE::CString, CIGameListingProvider, MT::CMutex > CGameListingProviderFactory;
typedef CGameListingProviderFactory::TProductPtr                                     CIGameListingProviderPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MULTIPLAY */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MULTIPLAY_CIGAMELISTINGPROVIDER_H ? */
