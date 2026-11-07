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

#ifndef GUCEF_GAMEGUI_CMATCHLISTFORM_H
#define GUCEF_GAMEGUI_CMATCHLISTFORM_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

#ifndef GUCEF_CORE_CICLONEABLE_H
#include "CICloneable.h"
#define GUCEF_CORE_CICLONEABLE_H
#endif /* GUCEF_CORE_CICLONEABLE_H ? */

#ifndef GUCEF_GUI_CFORMEX_H
#include "gucefGUI_CFormEx.h"
#define GUCEF_GUI_CFORMEX_H
#endif /* GUCEF_GUI_CFORMEX_H ? */

#ifndef GUCEF_GUI_CPOPUPMENU_H
#include "gucefGUI_CPopupMenu.h"
#define GUCEF_GUI_CPOPUPMENU_H
#endif /* GUCEF_GUI_CPOPUPMENU_H ? */

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
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

/** What the player did to a match in the row menu of CMatchListForm */
enum EMatchAction
{
    MATCHACTION_MUTE      = 0 ,
    MATCHACTION_UNMUTE    = 1 ,
    MATCHACTION_RENAME    = 2 ,
    MATCHACTION_ARCHIVE   = 3 ,
    MATCHACTION_UNARCHIVE = 4 ,
    MATCHACTION_REMOVE    = 5 ,
    MATCHACTION_NUDGE     = 6 ,
    MATCHACTION_RESIGN    = 7 ,
    MATCHACTION_REMATCH   = 8 ,
    MATCHACTION_DETAILS   = 9           /**< the action of the application, see CMatchListForm::SetDetailsAction() */
};
typedef enum EMatchAction TMatchAction;

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The data of CMatchListForm::MatchActionEvent */
class GUCEF_GAMEGUI_EXPORT_CPP CMatchActionEventData : public CORE::CICloneable
{
    public:

    CORE::CString matchId;
    TMatchAction action;

    CMatchActionEventData( void );

    CMatchActionEventData( const CORE::CString& id, TMatchAction matchAction );

    CMatchActionEventData( const CMatchActionEventData& src );

    virtual ~CMatchActionEventData();

    CMatchActionEventData& operator=( const CMatchActionEventData& src );

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const char* GetCloneableTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    static const char* CloneableTypeName( void );
};

/*-------------------------------------------------------------------------*/

/**
 *  "My Games": the window with the matches of a CMatchBook, grouped as the book groups them.
 *
 *  Groups: Your Turn, Their Turn, Invitations and Finished, in this order, each with a header row and its count, a group
 *  without matches has none. Archived matches are left out unless SetShowArchived( true ). A row has the name of the game
 *  (the display title of the entry, "Game <short id>" when it has none), a small portrait slot, the subtitle "Turn <n> -
 *  <name of the player on turn>" (the summary of the preview when the name is not known, else "Turn <n>"), the time left to
 *  the deadline of an active match (FormatTimeLeft(), nothing without a deadline) and the marks unseen (a dot), pinned, muted
 *  and problem. Text that does not fit its box is cut with "...". Rows are at least 56 pixels high, buttons and fields at
 *  least 44.
 *
 *  Choosing: a click selects a row, a double click or Enter opens it, so do the Open button and OpenSelected(). Opening only
 *  notifies OpenRequestedEvent with the match id: the application opens the match in the book. Up, Down, Page Up, Page Down,
 *  Home and End move the selection and keep it in view; the wheel and the scroll bar scroll. New Game and Invitations notify
 *  NewGameRequestedEvent and InvitationsRequestedEvent.
 *
 *  Row menu: the "..." button of a row and a right click open it. Its items are Open, Mute or Unmute, Nudge, Rename, Resign,
 *  Rematch and Archive (Unarchive for an archived match) or Remove (a finished match). What does not apply is not in the menu:
 *  Nudge needs the capability MATCHCAPABILITY_NUDGE of the provider of the match, a match in Their Turn and a player to nudge,
 *  Resign MATCHCAPABILITY_RESIGN and an active match, Rematch MATCHCAPABILITY_REMATCH and a finished match. The form does the
 *  action (through the book, or the provider of the match) and then notifies MatchActionEvent. Rename shows an inline panel
 *  in the strip at the bottom with a field, OK and Cancel; OK gives the match the name with CMatchBook::SetLocalTitle().
 *
 *  What the book does not know is pushed: a problem text of a match (SetMatchProblem) and the name of a portrait image
 *  (SetMatchPortrait), an image the application registered in the context with CGUIContext::RegisterImage(). A portrait
 *  name the context does not know leaves the slot empty.
 *
 *  Time passes with the frames of the context: the time left is worked out against the clock (SetClock(), the system clock by
 *  default) about once a second and only the labels whose text changed are touched.
 *
 *  The layout is built in code and made for BuildDarkTheme().
 */
class GUCEF_GAMEGUI_EXPORT_CPP CMatchListForm : public GUI::CFormEx
{
    public:

    static const CORE::CEvent OpenRequestedEvent;           /**< CORE::TCloneableString: the match id */
    static const CORE::CEvent NewGameRequestedEvent;        /**< no data */
    static const CORE::CEvent InvitationsRequestedEvent;    /**< no data */
    static const CORE::CEvent ExtraRequestedEvent;          /**< no data: the button the application gave a text with SetExtraButton() was activated */
    static const CORE::CEvent MatchActionEvent;             /**< CMatchActionEventData: after the form did the action */

    typedef CORE::TCloneableString TOpenRequestedEventData;
    typedef CMatchActionEventData TMatchActionEventData;

    static void RegisterEvents( void );

    /** The measures of the rows, in pixels */
    static const Int32 MinRowHeight = 56;
    static const Int32 MinTouchHeight = 44;

    /** The menu items, as the menu and GetRowMenuItems() have them */
    static const char* const ItemOpen;
    static const char* const ItemMute;
    static const char* const ItemUnmute;
    static const char* const ItemRename;
    static const char* const ItemArchive;
    static const char* const ItemUnarchive;
    static const char* const ItemRemove;
    static const char* const ItemNudge;
    static const char* const ItemResign;
    static const char* const ItemRematch;

    /** A row of the list as it is shown: a group header or a match */
    struct SRowInfo
    {
        CORE::CString matchId;                  /**< empty for a header */
        MULTIPLAY::TMatchGroup group;
        CORE::CString title;                    /**< the whole name of the game, the label shows it cut to fit */
        CORE::CString subtitle;
        CORE::CString timeLeft;
        bool unseen;
        bool muted;
        bool problem;
        bool pinned;
        bool isHeader;
        CORE::CString headerText;               /**< for a header: the group name and the count, "Your Turn (2)" */

        SRowInfo( void );
    };
    typedef struct SRowInfo TRowInfo;

    CMatchListForm( void );

    CMatchListForm( GUI::CIGUIContext* context );

    virtual ~CMatchListForm();

    /** Makes the window, hidden */
    bool Build( void );

    /** Makes the window when needed, shows it and shows the matches of the book */
    bool Open( void );

    void Close( void );

    bool IsOpen( void ) const;

    /** The book whose matches are shown, not owned; none to show nothing */
    void SetBook( MULTIPLAY::CMatchBook* book );

    MULTIPLAY::CMatchBook* GetBook( void ) const;

    /** The clock the time left is worked out with, which has to outlive the form; NULL for the clock of the system */
    void SetClock( const MULTIPLAY::CIMatchClock* clock );

    /** Whether archived matches are listed too (off by default) */
    void SetShowArchived( bool showArchived );

    /** A button of the application at the right of the buttons (for what only the application knows, such as adding a game from a folder); an empty text hides it. ExtraRequestedEvent when it is activated */
    void SetExtraButton( const CORE::CString& text );

    /**
     *  An action of the application in the menu of the games of one provider, after Open (for what only the application knows about such a game, such as the
     *  table of a game by mail). An empty text takes it away. MatchActionEvent with MATCHACTION_DETAILS when it is chosen
     */
    void SetDetailsAction( const CORE::CString& providerName, const CORE::CString& text );

    bool GetShowArchived( void ) const;

    /** The name of a font of the context for the name of the game, larger than the one of the theme; empty for the font of the theme */
    void SetTitleFont( const CORE::CString& fontName );

    /** The text of a problem with a match (a package that was refused ...): shown with a mark, as the detail of the row and its tooltip; empty clears it */
    void SetMatchProblem( const CORE::CString& matchId, const CORE::CString& problemText );

    /** The name of an image of the context for the small portrait slot of the row; empty clears it */
    void SetMatchPortrait( const CORE::CString& matchId, const CORE::CString& imageName );

    /** Shows the matches of the book again; done whenever the book says something changed */
    void UpdateRows( void );

    /** Lets time pass, done on every frame of the context with the time of the frame: once a second the time left is shown again */
    void Update( UInt32 deltaInMs );

    /*
     *  The rows
     */

    UInt32 GetRowCount( void ) const;

    /** The row at an index (headers included), top to bottom; false when there is none */
    bool GetRow( UInt32 index, SRowInfo& row ) const;

    /** The index of the row of a match, -1 when the match is not listed */
    Int32 FindRow( const CORE::CString& matchId ) const;

    /** Selects a match, and scrolls to it; false when it is not listed */
    bool SelectRow( const CORE::CString& matchId );

    /** The selected match, empty when there is none */
    const CORE::CString& GetSelectedMatchId( void ) const;

    /** Notifies OpenRequestedEvent for the selected match; false when there is none */
    bool OpenSelected( void );

    /**
     *  A widget of the row of a match: "Row", "Dot", "Portrait", "Title", "Subtitle", "TimeLeft", "Pinned", "Muted", "Problem" or "Menu";
     *  NULL when the match is not listed or its row is not in view: only the rows in view (and a little around them) have widgets
     */
    GUI::CWidget* GetRowWidget( const CORE::CString& matchId, const CORE::CString& part );

    /*
     *  Actions
     */

    /** The items of the row menu of a match, in order; false when the match is not listed */
    bool GetRowMenuItems( const CORE::CString& matchId, GUCEF::vector< CORE::CString >& items ) const;

    /** Opens the row menu of a match at a position; false when the match has none */
    bool OpenRowMenu( const CORE::CString& matchId, Int32 x, Int32 y );

    /** Does an item of the row menu, as choosing it in the menu does; false when it is not an item of the match */
    bool PerformRowAction( const CORE::CString& matchId, const CORE::CString& item );

    /** Shows the panel for the name of the game; false when the match is not listed */
    bool BeginRename( const CORE::CString& matchId );

    bool IsRenaming( void ) const;

    /** What OK does: gives the match the name of the field; false when no rename is going on */
    bool CommitRename( void );

    void CancelRename( void );

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual void OnPostLayoutLoad( void ) GUCEF_VIRTUAL_OVERRIDE;

    /** Only the standard events (destruction of the book) arrive here, the others have their own handler */
    virtual void OnNotify( CORE::CNotifier* notifier                 ,
                           const CORE::CEvent& eventId               ,
                           CORE::CICloneable* eventData = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    typedef CORE::CTEventHandlerFunctor< CMatchListForm > TEventCallback;

    /** A row of the model: what is shown, and what the widgets need to know about it */
    struct SRow
    {
        SRowInfo info;
        bool archived;
        bool active;                    /**< the time left is shown */
        Int64 deadline;
        Int32 ordinal;                  /**< the place among the rows of its kind */
        Int32 slot;                     /**< the index in the slots of its kind, -1 while the row is not in view */
        Int32 top;                      /**< in the list, without scrolling */
        Int32 height;
        bool hasProblemText;

        SRow( void );
    };
    typedef GUCEF::vector< SRow > TRowVector;

    /** The widgets of a match row and what they show now, so that only what changed is touched */
    struct SMatchSlot
    {
        GUI::CWidget* row;
        GUI::CWidget* dot;
        GUI::CWidget* portrait;
        GUI::CWidget* title;
        GUI::CWidget* subtitle;
        GUI::CWidget* timeLeft;
        GUI::CWidget* pinned;
        GUI::CWidget* muted;
        GUI::CWidget* problem;
        GUI::CWidget* menu;
        CORE::CString matchId;
        CORE::CString titleSource;
        CORE::CString titleShown;
        Int32 titleWidth;
        CORE::CString subtitleSource;
        CORE::CString subtitleShown;
        Int32 subtitleWidth;
        CORE::CString timeShown;
        CORE::CString portraitShown;
        CORE::CString tooltipShown;
        Int32 top;
        Int32 height;
        bool shown;
        bool selected;
        bool unseen;
        bool pinnedShown;
        bool mutedShown;
        bool problemShown;
        bool archivedShown;
        bool overdueShown;
        bool used;

        SMatchSlot( void );
    };
    typedef GUCEF::vector< SMatchSlot > TMatchSlotVector;

    struct SHeaderSlot
    {
        GUI::CWidget* row;
        GUI::CWidget* text;
        CORE::CString textShown;
        Int32 top;
        bool shown;
        bool used;

        SHeaderSlot( void );
    };
    typedef GUCEF::vector< SHeaderSlot > THeaderSlotVector;

    typedef GUCEF::map< GUI::CWidget*, UInt32 > TWidgetSlotMap;
    typedef GUCEF::map< CORE::CString, CORE::CString > TTextMap;

    void BuildLayout( CORE::CDataNode& layout );
    bool EnsureMatchSlots( UInt32 count );
    bool EnsureHeaderSlots( UInt32 count );
    void ComputeRowMetrics( void );
    void BuildModel( TRowVector& rows ) const;
    void MakeMatchRow( const MULTIPLAY::CMatchEntry& entry, MULTIPLAY::TMatchGroup group, SRow& row ) const;
    CORE::CString MakeSubtitle( const MULTIPLAY::CMatchEntry& entry ) const;
    void ShowRows( void );
    void ShowWindow( void );
    void ShowMatchRow( const SRow& row );
    void ShowHeaderRow( const SRow& row );
    void FitTexts( bool force );
    void FitMatchTexts( SMatchSlot& slot, const SRow& row, bool force );
    void ShowTimes( void );
    void ShowSelection( void );
    void ShowButtons( void );
    void ShowDetail( void );
    void ShowEmptyText( void );
    void ShowMatchTimeLeft( SMatchSlot& slot, const SRow& row );
    Int32 GetListWidth( void ) const;
    void SetLabelText( GUI::CWidget* label, const CORE::CString& text );
    void SetFlagWidget( GUI::CWidget* widget, bool shown );
    Int32 GetSlotOfRow( const SRow& row ) const;
    UInt32 GetVisibleRowCount( void ) const;
    Int32 FindMatchRow( const CORE::CString& matchId ) const;
    Int32 FindRowOfWidget( GUI::CWidget* widget ) const;
    bool SelectIndex( Int32 index );
    void MoveSelection( Int32 direction, UInt32 amount );
    void SelectEdge( bool first );
    void ScrollRowIntoView( Int32 index );
    Int64 GetNowMs( void ) const;
    bool GetNudgeTarget( const MULTIPLAY::CMatchEntry& entry, CORE::CString& userId ) const;
    void ShowMenuAt( const CORE::CString& matchId, Int32 x, Int32 y );
    void CloseMenu( void );
    void Notify( const CORE::CString& matchId, TMatchAction action );

    void OnBookChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnYourTurn( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnFrameStarted( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnListClicked( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnListDoubleClicked( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnListActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnKeyDown( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnOpenActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnNewGameActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnInvitationsActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnExtraActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnShowArchivedChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnRenameOkActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnRenameCancelActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnMenuItemActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );

    CMatchListForm( const CMatchListForm& src );
    CMatchListForm& operator=( const CMatchListForm& src );

    private:

    MULTIPLAY::CMatchBook* m_book;
    const MULTIPLAY::CIMatchClock* m_clock;
    TRowVector m_rows;
    TMatchSlotVector m_matchSlots;
    THeaderSlotVector m_headerSlots;
    TWidgetSlotMap m_slotOfRowWidget;       /**< the row panel of a match slot to its index */
    TTextMap m_problems;
    TTextMap m_portraits;
    CORE::CString m_selectedMatchId;
    CORE::CString m_renameMatchId;
    CORE::CString m_menuMatchId;
    CORE::CString m_titleFont;
    CORE::CString m_detailShown;
    GUI::CPopupMenu* m_menu;
    GUI::CWidget* m_list;
    GUI::CWidget* m_detail;
    GUI::CWidget* m_emptyText;
    GUI::CWidget* m_spacer;
    GUI::CWidget* m_openButton;
    GUI::CWidget* m_invitationsButton;
    GUI::CWidget* m_extraButton;
    CORE::CString m_extraText;
    CORE::CString m_detailsProvider;
    CORE::CString m_detailsText;
    GUI::CWidget* m_renamePanel;
    GUI::CWidget* m_renameEdit;
    GUI::CWidget* m_showArchivedBox;
    GUI::CWidget* m_stripWidgets[ 5 ];      /**< what the rename panel takes the place of: detail, check box and the three buttons */
    Int32 m_rowHeight;
    Int32 m_titleHeight;
    Int32 m_subtitleHeight;
    Int32 m_fittedWidth;
    Int32 m_shownScroll;
    Int32 m_shownViewport;
    UInt32 m_sinceTimesMs;
    bool m_showArchived;
    bool m_open;
    bool m_updating;
    bool m_renaming;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GAMEGUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GAMEGUI_CMATCHLISTFORM_H ? */
