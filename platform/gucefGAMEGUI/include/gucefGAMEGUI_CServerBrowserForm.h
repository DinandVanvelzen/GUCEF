/*
 *  gucefGAMEGUI: GUCEF module providing prebaked GUI forms for rapid game development
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
 *  What a list of games shows and offers, for a source of listings that has no ping, no passwords or no connection to make: which columns
 *  and controls are there, and the words of the titles and buttons. The defaults are the list of a game on a network.
 *
 *  A listing can also say more about its own row than the columns do (CGameListingEntry::ExtraStatusText: the words of the status column instead
 *  of "Lobby" or "In progress", ExtraDetailText: the cell of the detail column, ExtraJoinText: the text of the button for this row,
 *  ExtraCanJoin and ExtraCanSpectate: whether the row can be joined or watched at all, "true" or "false").
 *
 *  A source whose games are played over time (a game by mail) can tell the list more. Every listing has a group (ExtraGroup) and may need the player now
 *  (ExtraAttention "true": it is the player's turn) and may say when that turn ends (ExtraDeadline). With 'viewLabels' and 'viewKeys' (the key of a view is
 *  the group it shows, empty for all) the control of the filter that lists the kinds lists the groups instead; 'quickFilterKey' is the group of a box in
 *  the filter bar that shows only that group with one click, 'attentionText' the words for the rows that need the player in the status line ("100 games,
 *  12 your turn"), and with 'defaultOrderByAttention' the list starts in the order that puts those rows first, then the nearest end of the turn, then the
 *  name; a click on the title of a column is the player's own order. The rows that need the player are drawn to stand out.
 */
class GUCEF_GAMEGUI_EXPORT_CPP CServerBrowserCapabilities
{
    public:

    bool showPing;                          /**< the ping column and the ping limit of the filter bar */
    bool showLock;                          /**< the lock column and the filter for passworded games */
    bool showSource;                        /**< the column with the source (the provider that found the listing) */
    bool showFullFilter;                    /**< the filter that hides full lobbies */
    bool showDirectConnect;                 /**< the button to connect to an address */
    bool showSettings;                      /**< the button for the settings of the source of the listings */
    bool showDetail;                        /**< the detail column, from the extra text of the listings */
    bool showNote;                          /**< the line of text below the filter bar, for what the source of the listings has to say (SetNote()) */

    CORE::CString title;                    /**< the title of the window */
    CORE::CString kindColumnTitle;
    CORE::CString mapColumnTitle;
    CORE::CString modeColumnTitle;
    CORE::CString sourceColumnTitle;
    CORE::CString detailColumnTitle;
    CORE::CString joinText;
    CORE::CString spectateText;
    CORE::CString createText;
    CORE::CString settingsText;
    GUCEF::vector< CORE::CString > viewLabels;  /**< the options of the control of the filter that picks a group, in order; none: the options are the kinds (all, lobbies, in progress) */
    GUCEF::vector< CORE::CString > viewKeys;    /**< the group of every option of viewLabels ("" is all) */
    CORE::CString quickFilterKey;           /**< the group of the box that shows only the rows of that group, empty for no such box */
    CORE::CString quickFilterText;          /**< the words of the box ("Your turn"), followed by the number of rows */
    CORE::CString attentionText;            /**< what the status line calls the rows that need the player ("your turn"), empty for no count */
    bool defaultOrderByAttention;           /**< the order the list starts with, until the player picks a column: the rows that need the player first, then the nearest end of the turn, then the name */
    GUCEF::vector< Int32 > columnWidths;    /**< the widths of the columns in pixels, by column (CServerBrowserForm::TColumn); a column with none or 0 has the usual width. The widths are weights: the table is fitted to the window */

    CServerBrowserCapabilities( void );
};

/*-------------------------------------------------------------------------*/

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
    static const CORE::CEvent SettingsRequestedEvent;       /**< no data: the settings button, when the capabilities have it */
    static const CORE::CEvent FilterChangedEvent;           /**< CORE::TCloneableString: the group the player picked with the box or the control of the filter (empty for all), so that an application can keep it */

    typedef CORE::TCloneableString TFilterChangedEventData;

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
        COLUMN_DETAIL   ,
        COLUMN_COUNT
    };
    typedef enum EColumn TColumn;

    typedef MULTIPLAY::CGameListingList::TGameListingEntryVector TGameListingEntryVector;

    CServerBrowserForm( void );

    CServerBrowserForm( GUI::CIGUIContext* context );

    virtual ~CServerBrowserForm();

    /** What the list shows and offers; the window is made again when it is there already (what the player typed in the filter stays) */
    void SetCapabilities( const CServerBrowserCapabilities& capabilities );
    const CServerBrowserCapabilities& GetCapabilities( void ) const;

    /** What the source of the listings has to say, in the line below the filter bar when the capabilities have it; 'problem' makes it stand out */
    void SetNote( const CORE::CString& text, bool problem );
    CORE::CString GetNoteText( void );

    /** Whether a column is in the table, and the columns that are, in order */
    bool IsColumnShown( TColumn column ) const;
    void GetShownColumns( GUCEF::vector< Int32 >& columns ) const;

    /** The width of every shown column, in the order of GetShownColumns(), as the table has them (before it is fitted to the window) */
    void GetColumnWidths( GUCEF::vector< Int32 >& widths ) const;

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

    /** The group the list is filtered on (empty for all) */
    CORE::CString GetFilterGroup( void ) const;

    /** Asks the browser to look again */
    bool RefreshListings( void );

    void RequestCreate( void );

    void RequestDirectConnect( void );
    void RequestSettings( void );

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual void OnPostLayoutLoad( void ) GUCEF_VIRTUAL_OVERRIDE;

    /** Only the standard events (destruction of the browser) arrive here, the others have their own handler */
    virtual void OnNotify( CORE::CNotifier* notifier                 ,
                           const CORE::CEvent& eventId               ,
                           CORE::CICloneable* eventData = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    typedef CORE::CTEventHandlerFunctor< CServerBrowserForm > TEventCallback;

    void ShowQuickFilter( void );
    void OnQuickFilterChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );

    void BuildLayout( CORE::CDataNode& layout );
    void ReadFilterFromControls( void );
    void ShowFilterInControls( void );
    void UpdateButtons( void );
    void SortListed( void );
    bool IsBefore( const MULTIPLAY::CGameListingEntry& a, const MULTIPLAY::CGameListingEntry& b ) const;
    static CORE::CString KindText( const MULTIPLAY::CGameListingEntry& listing );
    static CORE::CString ExtraText( const MULTIPLAY::CGameListingEntry& listing, const char* key );
    static bool ExtraFlag( const MULTIPLAY::CGameListingEntry& listing, const char* key, bool defaultValue );
    CORE::CString TitleOf( TColumn column ) const;
    void ReadShownColumns( void );
    void ReloadLayout( void );

    void OnBrowserListChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnJoinActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnSpectateActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnCreateActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnDirectConnectActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnSettingsActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnRefreshActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnFilterChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnSelectionChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnColumnClicked( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnListingActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );

    CServerBrowserForm( const CServerBrowserForm& src );
    CServerBrowserForm& operator=( const CServerBrowserForm& src );

    private:

    MULTIPLAY::CGameBrowser* m_browser;
    CServerBrowserCapabilities m_capabilities;
    CORE::CString m_note;
    bool m_noteIsProblem;
    GUCEF::vector< Int32 > m_shownColumns;
    MULTIPLAY::CGameListingFilter m_filter;
    TGameListingEntryVector m_listed;
    Int32 m_sortColumn;
    bool m_sortAscending;
    bool m_playerSorted;
    UInt32 m_attentionTotal;
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
