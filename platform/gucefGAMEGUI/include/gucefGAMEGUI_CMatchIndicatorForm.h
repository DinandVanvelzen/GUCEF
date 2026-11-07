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

#ifndef GUCEF_GAMEGUI_CMATCHINDICATORFORM_H
#define GUCEF_GAMEGUI_CMATCHINDICATORFORM_H

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

#ifndef GUCEF_MULTIPLAY_CMATCHBOOK_H
#include "gucefMULTIPLAY_CMatchBook.h"
#define GUCEF_MULTIPLAY_CMATCHBOOK_H
#endif /* GUCEF_MULTIPLAY_CMATCHBOOK_H ? */

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
 *  The games indicator: a small badge in a corner of the screen with the number of games that wait for the player ("Your
 *  turn: 3"), bound to a CMatchBook. It is hidden while the count is 0 (SetHideWhenEmpty( false ) keeps it), and a hidden
 *  badge takes no room and no input.
 *
 *  A click on the badge opens a compact list under it (above it at the bottom of the screen) with the games that wait,
 *  the one that has waited the longest first (the oldest yourTurnSince, as CMatchBook::GetNextYourTurn() chooses), at
 *  most SetMaxListed() of them, each with its name and the time left to its deadline. Clicking a game notifies
 *  OpenRequestedEvent with the match id and closes the list; the last entry, "More games ..." when not every game is
 *  listed and "My Games" when it is, notifies MyGamesRequestedEvent and closes the list. Another click on the badge closes it.
 *
 *  The window is not modal and has no title bar. It is as large as what it shows (the badge, or the badge and the list) and
 *  stays at the corner it is anchored to, so in the corner nothing else is covered. Everything is at least 44 pixels high.
 *
 *  The layout is built in code and made for BuildDarkTheme().
 */
class GUCEF_GAMEGUI_EXPORT_CPP CMatchIndicatorForm : public GUI::CFormEx
{
    public:

    static const CORE::CEvent OpenRequestedEvent;           /**< CORE::TCloneableString: the match id */
    static const CORE::CEvent MyGamesRequestedEvent;        /**< no data */
    static const CORE::CEvent BadgeRequestedEvent;          /**< no data: the badge was clicked while the application has a list of its own for the games that wait (SetBadgeOpensList( false )) */

    typedef CORE::TCloneableString TOpenRequestedEventData;

    static void RegisterEvents( void );

    /** The corner of the screen the badge sits in */
    enum EAnchor
    {
        ANCHOR_TOP_LEFT = 0 ,
        ANCHOR_TOP_RIGHT    ,
        ANCHOR_BOTTOM_LEFT  ,
        ANCHOR_BOTTOM_RIGHT
    };
    typedef enum EAnchor TAnchor;

    static const UInt32 DefaultMaxListed = 8;
    static const UInt32 LargestMaxListed = 20;

    /** The measures of the badge and the list in pixels */
    static const Int32 BadgeWidth = 176;
    static const Int32 BadgeHeight = 44;
    static const Int32 ListWidth = 360;
    static const Int32 EntryHeight = 44;

    CMatchIndicatorForm( void );

    CMatchIndicatorForm( GUI::CIGUIContext* context );

    virtual ~CMatchIndicatorForm();

    /** Makes the window, hidden */
    bool Build( void );

    /** Makes the window when needed and shows it when there are games that wait (or it is kept without) */
    bool Open( void );

    void Close( void );

    /** Whether the indicator is open; it is not seen while the count is 0 and it hides when empty */
    bool IsOpen( void ) const;

    /** Whether the badge is on the screen: open, and games wait or the badge is kept without */
    bool IsBadgeShown( void ) const;

    /** The book whose games are counted, not owned; none to count nothing */
    void SetBook( MULTIPLAY::CMatchBook* book );

    MULTIPLAY::CMatchBook* GetBook( void ) const;

    /** The clock the time left is worked out with, which has to outlive the form; NULL for the clock of the system */
    void SetClock( const MULTIPLAY::CIMatchClock* clock );

    /** The corner and the distance from the edges of the screen; the default is the top right with 12 pixels */
    void SetAnchor( TAnchor corner, Int32 offsetX, Int32 offsetY );

    TAnchor GetAnchor( void ) const;

    /**
     *  What a click on the badge does. By default it opens the popup with the games that wait. An application with a list of its own for them (the browser of the games by
     *  mail, filtered on the player's turn) turns that off: the click then notifies BadgeRequestedEvent and opens nothing
     */
    void SetBadgeOpensList( bool opensList );

    bool GetBadgeOpensList( void ) const;

    /** Whether the badge is hidden while no game waits (on by default) */
    void SetHideWhenEmpty( bool hideWhenEmpty );

    bool GetHideWhenEmpty( void ) const;

    /** The most games the list has, from 1 to LargestMaxListed */
    void SetMaxListed( UInt32 maxListed );

    UInt32 GetMaxListed( void ) const;

    /** The games that wait for the player */
    UInt32 GetYourTurnCount( void ) const;

    bool IsPopupOpen( void ) const;

    /** Opens the list, or closes it when it is open; false (and nothing opens) while the badge is not shown. The result is whether the list is open now */
    bool TogglePopup( void );

    void ClosePopup( void );

    /** The matches the list has, or would have, in order */
    void GetListedMatchIds( GUCEF::vector< CORE::CString >& matchIds ) const;

    /** Shows the count and the list again; done whenever the book says something changed */
    void Refresh( void );

    /** Lets time pass, done on every frame of the context with the time of the frame: once a second the time left in the list is shown again */
    void Update( UInt32 deltaInMs );

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual void OnPostLayoutLoad( void ) GUCEF_VIRTUAL_OVERRIDE;

    /** Only the standard events (destruction of the book) arrive here, the others have their own handler */
    virtual void OnNotify( CORE::CNotifier* notifier                 ,
                           const CORE::CEvent& eventId               ,
                           CORE::CICloneable* eventData = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    typedef CORE::CTEventHandlerFunctor< CMatchIndicatorForm > TEventCallback;

    struct SEntry
    {
        GUI::CWidget* button;
        GUI::CWidget* time;
        CORE::CString matchId;
        CORE::CString source;
        CORE::CString textShown;
        CORE::CString timeShown;
        Int64 deadline;
        bool shown;
        bool overdueShown;

        SEntry( void );
    };
    typedef GUCEF::vector< SEntry > TEntryVector;
    typedef GUCEF::map< GUI::CWidget*, UInt32 > TWidgetEntryMap;

    void BuildLayout( CORE::CDataNode& layout );
    bool EnsureEntries( UInt32 count );
    void Place( void );
    void ShowBadge( void );
    void ShowPopup( void );
    void ShowTimes( void );
    void GetOrdered( MULTIPLAY::TMatchEntryVector& entries ) const;
    Int64 GetNowMs( void ) const;

    void OnBookChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnYourTurn( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnFrameStarted( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnBadgeActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnEntryActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnMyGamesActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );

    CMatchIndicatorForm( const CMatchIndicatorForm& src );
    CMatchIndicatorForm& operator=( const CMatchIndicatorForm& src );

    private:

    MULTIPLAY::CMatchBook* m_book;
    const MULTIPLAY::CIMatchClock* m_clock;
    TEntryVector m_entries;
    TWidgetEntryMap m_entryOfButton;
    GUI::CWidget* m_badge;
    GUI::CWidget* m_popup;
    GUI::CWidget* m_myGames;
    TAnchor m_anchor;
    Int32 m_offsetX;
    Int32 m_offsetY;
    UInt32 m_maxListed;
    UInt32 m_count;
    UInt32 m_listedShown;
    UInt32 m_sinceTimesMs;
    bool m_badgeOpensList;
    bool m_hideWhenEmpty;
    bool m_open;
    bool m_popupOpen;
    bool m_badgeShownNow;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GAMEGUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GAMEGUI_CMATCHINDICATORFORM_H ? */
