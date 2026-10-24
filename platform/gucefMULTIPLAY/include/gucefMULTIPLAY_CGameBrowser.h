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

#ifndef GUCEF_MULTIPLAY_CGAMEBROWSER_H
#define GUCEF_MULTIPLAY_CGAMEBROWSER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MULTIPLAY_CIGAMELISTINGPROVIDER_H
#include "gucefMULTIPLAY_CIGameListingProvider.h"
#define GUCEF_MULTIPLAY_CIGAMELISTINGPROVIDER_H
#endif /* GUCEF_MULTIPLAY_CIGAMELISTINGPROVIDER_H ? */

#ifndef GUCEF_MULTIPLAY_CPINGSERVICE_H
#include "gucefMULTIPLAY_CPingService.h"
#define GUCEF_MULTIPLAY_CPINGSERVICE_H
#endif /* GUCEF_MULTIPLAY_CPINGSERVICE_H ? */

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

/** The data of BrowserListChangedEvent */
class GUCEF_MULTIPLAY_EXPORT_CPP CBrowserListChange : public CORE::CICloneable
{
    public:

    enum EChange
    {
        CHANGE_ADDED   = 0 ,
        CHANGE_UPDATED = 1 ,
        CHANGE_REMOVED = 2
    };
    typedef enum EChange TChange;

    TChange change;
    CORE::CString listingId;

    CBrowserListChange( void );

    CBrowserListChange( TChange change, const CORE::CString& listingId );

    virtual ~CBrowserListChange();

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------*/

/**
 *  The list a game shows to pick a game or lobby from: the listings of all its providers in one list.
 *
 *  Listings are merged when they have the same id or share an endpoint; the provider added first gives the
 *  content of a merged listing. The filter goes to the providers and the browser applies it too, the ping limit
 *  included: the browser pings the first endpoint of every listing with its CPingService. A listing whose ping is
 *  not known (yet) passes the ping limit.
 *
 *  Config: "ListingProvider" children (attribute "type", the rest is the config of the provider), a "Filter"
 *  child, a "PingService" child, ping (default true)
 */
class GUCEF_MULTIPLAY_EXPORT_CPP CGameBrowser : public CORE::CObservingNotifier ,
                                                public CORE::CIConfigurable
{
    public:

    static const CORE::CEvent BrowserListChangedEvent;  /**< CBrowserListChange */

    typedef CGameListingList::TGameListingEntryVector   TGameListingEntryVector;
    typedef GUCEF::vector< CIGameListingProviderPtr >   TProviderVector;

    static void RegisterEvents( void );

    CGameBrowser( void );

    virtual ~CGameBrowser();

    /** Adds a provider, started at once when the browser runs */
    bool AddProvider( CIGameListingProviderPtr provider );

    /** Makes a provider with the listing provider factory and the given config, then adds it */
    bool AddProvider( const CORE::CString& providerType, const CORE::CDataNode* config );

    const TProviderVector& GetProviders( void ) const;

    bool Start( void );

    void Stop( void );

    bool Refresh( void );

    bool IsActive( void ) const;

    /** Applies the filter, the providers start again with it */
    void SetFilter( const CGameListingFilter& filter );

    const CGameListingFilter& GetFilter( void ) const;

    /** The listings that pass the filter, with their pings */
    void GetListings( TGameListingEntryVector& listings ) const;

    bool GetListing( const CORE::CString& listingId, CGameListingEntry& listing ) const;

    UInt32 GetListingCount( void ) const;

    void SetPingEnabled( bool pingEnabled );

    CPingService& GetPingService( void );

    virtual bool SaveConfig( CORE::CDataNode& config ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool LoadConfig( const CORE::CDataNode& config ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual void OnNotify( CORE::CNotifier* notifier                 ,
                           const CORE::CEvent& eventId               ,
                           CORE::CICloneable* eventData = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    struct SMergedListing
    {
        CGameListingEntry entry;
        CORE::CString pingTarget;       /**< empty when the listing has no endpoint */
        bool visible;
    };
    typedef struct SMergedListing TMergedListing;
    typedef GUCEF::vector< TMergedListing > TMergedVector;
    typedef GUCEF::map< CORE::CString, size_t > TIndexMap;
    typedef GUCEF::vector< CBrowserListChange > TChangeVector;

    CGameBrowser( const CGameBrowser& src );
    CGameBrowser& operator=( const CGameBrowser& src );

    void Rebuild( void );
    void ApplyPing( const CORE::CString& target, Int32 rttMs );
    bool NotifyChanges( const TChangeVector& changes );

    private:

    TProviderVector m_providers;
    CGameListingFilter m_filter;
    CPingService m_pingService;
    bool m_pingEnabled;
    bool m_active;
    TMergedVector m_merged;
    TIndexMap m_visibleIndex;           /**< the visible listings by id, into m_merged */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MULTIPLAY */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MULTIPLAY_CGAMEBROWSER_H ? */
