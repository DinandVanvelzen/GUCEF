/*
 *  gucefGAMEGUI: GUCEF module providing prebaked GUI forms for rapid game development
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

#ifndef GUCEF_GAMEGUI_CSERVERBROWSERFORM_H
#define GUCEF_GAMEGUI_CSERVERBROWSERFORM_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

#ifndef GUCEF_GUI_CFORMEX_H
#include "gucefGUI_CFormEx.h"
#define GUCEF_GUI_CFORMEX_H
#endif /* GUCEF_GUI_CFORMEX_H ? */

#ifndef GUCEF_MULTIPLAY_CGAMEBROWSER_H
#include "gucefMULTIPLAY_CGameBrowser.h"
#define GUCEF_MULTIPLAY_CGAMEBROWSER_H
#endif /* GUCEF_MULTIPLAY_CGAMEBROWSER_H ? */

#ifndef GUCEF_MULTIPLAY_CGAMELISTINGENTRY_H
#include "gucefMULTIPLAY_CGameListingEntry.h"
#define GUCEF_MULTIPLAY_CGAMELISTINGENTRY_H
#endif /* GUCEF_MULTIPLAY_CGAMELISTINGENTRY_H ? */

#ifndef GUCEF_MULTIPLAY_CGAMELISTINGFILTER_H
#include "gucefMULTIPLAY_CGameListingFilter.h"
#define GUCEF_MULTIPLAY_CGAMELISTINGFILTER_H
#endif /* GUCEF_MULTIPLAY_CGAMELISTINGFILTER_H ? */

#ifndef GUCEF_GAMEGUI_MACROS_H
#include "gucefGAMEGUI_macros.h"
#define GUCEF_GAMEGUI_MACROS_H
#endif /* GUCEF_GAMEGUI_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GAMEGUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The list of games and lobbies a player picks from, bound to a CGameBrowser.
 *
 *  A table with the columns name, kind (lobby or game in progress), players, map, mode, ping, lock and source;
 *  clicking a column title sorts by it, again the other way round. Above the table the filter bar: text, kind, hide
 *  full, hide passworded and a ping limit (0 for none). The bar filters what the browser lists; the browser's own
 *  filter is what the listing providers look for. Below it the buttons Join, Spectate, Refresh, Create and Direct connect.
 *
 *  Join is for a lobby that has room, Spectate for any listing; both need a row. Double clicking a row (or Enter)
 *  joins, or spectates a game in progress. Join and Spectate notify JoinRequestedEvent and SpectateRequestedEvent with
 *  the listing (a CGameListingEntry) of the row; Create and Direct connect notify CreateRequestedEvent and
 *  DirectConnectRequestedEvent. What happens next is up to the application: a lobby form, a form that creates a lobby,
 *  one that connects directly.
 *
 *  The layout is built in code and made for BuildDarkTheme().
 */
class GUCEF_GAMEGUI_EXPORT_CPP CServerBrowserForm : public GUI::CFormEx
{
    public:

    static const CORE::CEvent JoinRequestedEvent;           /**< MULTIPLAY::CGameListingEntry */
    static const CORE::CEvent SpectateRequestedEvent;       /**< MULTIPLAY::CGameListingEntry */
    static const CORE::CEvent CreateRequestedEvent;         /**< no data */
    static const CORE::CEvent DirectConnectRequestedEvent;  /**< no data */

    static void RegisterEvents( void );

    /** The columns of the table, in order */
    enum EColumn
    {
        COLUMN_NAME = 0 ,
        COLUMN_KIND     ,
        COLUMN_PLAYERS  ,
        COLUMN_MAP      ,
        COLUMN_MODE     ,
        COLUMN_PING     ,
        COLUMN_LOCK     ,
        COLUMN_SOURCE   ,
        COLUMN_COUNT
    };
    typedef enum EColumn TColumn;

    typedef MULTIPLAY::CGameListingList::TGameListingEntryVector TGameListingEntryVector;

    CServerBrowserForm( void );

    CServerBrowserForm( GUI::CIGUIContext* context );

    virtual ~CServerBrowserForm();

    /** Makes the window, hidden */
    bool Build( void );

    /** Makes the window when needed, shows it and shows the listings of the browser */
    bool Open( void );

    /** The browser whose listings are shown, not owned; none to show nothing */
    void SetBrowser( MULTIPLAY::CGameBrowser* browser );

    MULTIPLAY::CGameBrowser* GetBrowser( void ) const;

    /** The filter of the bar, also put in the controls of the bar */
    void SetFilter( const MULTIPLAY::CGameListingFilter& filter );

    const MULTIPLAY::CGameListingFilter& GetFilter( void ) const;

    /** Shows the listings of the browser again, in the order of the sort; done whenever the browser says its list changed */
    void UpdateRows( void );

    /** The listings as the rows show them, top to bottom */
    const TGameListingEntryVector& GetListedEntries( void ) const;

    /** The listing of the selected row */
    bool GetSelectedListing( MULTIPLAY::CGameListingEntry& listing ) const;

    bool SelectListing( const CORE::CString& listingId );

    void SetSort( TColumn column, bool ascending );

    Int32 GetSortColumn( void ) const;

    bool IsSortAscending( void ) const;

    /** Whether the selected listing can be joined: a lobby with room */
    bool CanJoin( void ) const;

    bool CanSpectate( void ) const;

    /** Notifies JoinRequestedEvent for the selected listing; false when it cannot be joined */
    bool Join( void );

    bool Spectate( void );

    /** Asks the browser to look again */
    bool RefreshListings( void );

    void RequestCreate( void );

    void RequestDirectConnect( void );

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual void OnPostLayoutLoad( void ) GUCEF_VIRTUAL_OVERRIDE;

    /** Only the standard events (destruction of the browser) arrive here, the others have their own handler */
    virtual void OnNotify( CORE::CNotifier* notifier                 ,
                           const CORE::CEvent& eventId               ,
                           CORE::CICloneable* eventData = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    typedef CORE::CTEventHandlerFunctor< CServerBrowserForm > TEventCallback;

    void BuildLayout( CORE::CDataNode& layout );
    void ReadFilterFromControls( void );
    void ShowFilterInControls( void );
    void UpdateButtons( void );
    void SortListed( void );
    bool IsBefore( const MULTIPLAY::CGameListingEntry& a, const MULTIPLAY::CGameListingEntry& b ) const;
    static CORE::CString KindText( const MULTIPLAY::CGameListingEntry& listing );

    void OnBrowserListChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnJoinActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnSpectateActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnCreateActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnDirectConnectActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnRefreshActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnFilterChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnSelectionChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnColumnClicked( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnListingActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );

    CServerBrowserForm( const CServerBrowserForm& src );
    CServerBrowserForm& operator=( const CServerBrowserForm& src );

    private:

    MULTIPLAY::CGameBrowser* m_browser;
    MULTIPLAY::CGameListingFilter m_filter;
    TGameListingEntryVector m_listed;
    Int32 m_sortColumn;
    bool m_sortAscending;
    bool m_updating;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GAMEGUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GAMEGUI_CSERVERBROWSERFORM_H ? */
