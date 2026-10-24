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

#ifndef GUCEF_GAMEGUI_CLOBBYFORM_H
#define GUCEF_GAMEGUI_CLOBBYFORM_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GUI_CFORMEX_H
#include "gucefGUI_CFormEx.h"
#define GUCEF_GUI_CFORMEX_H
#endif /* GUCEF_GUI_CFORMEX_H ? */

#ifndef GUCEF_GUI_CPOPUPMENU_H
#include "gucefGUI_CPopupMenu.h"
#define GUCEF_GUI_CPOPUPMENU_H
#endif /* GUCEF_GUI_CPOPUPMENU_H ? */

#ifndef GUCEF_MULTIPLAY_CILOBBY_H
#include "gucefMULTIPLAY_CILobby.h"
#define GUCEF_MULTIPLAY_CILOBBY_H
#endif /* GUCEF_MULTIPLAY_CILOBBY_H ? */

#ifndef GUCEF_MULTIPLAY_CLAUNCHDESCRIPTION_H
#include "gucefMULTIPLAY_CLaunchDescription.h"
#define GUCEF_MULTIPLAY_CLAUNCHDESCRIPTION_H
#endif /* GUCEF_MULTIPLAY_CLAUNCHDESCRIPTION_H ? */

#ifndef GUCEF_MULTIPLAY_CICHATSERVICE_H
#include "gucefMULTIPLAY_CIChatService.h"
#define GUCEF_MULTIPLAY_CICHATSERVICE_H
#endif /* GUCEF_MULTIPLAY_CICHATSERVICE_H ? */

#ifndef GUCEF_GAMEGUI_CLOBBYSETTINGSPANELFORM_H
#include "gucefGAMEGUI_CLobbySettingsPanelForm.h"
#define GUCEF_GAMEGUI_CLOBBYSETTINGSPANELFORM_H
#endif /* GUCEF_GAMEGUI_CLOBBYSETTINGSPANELFORM_H ? */

#ifndef GUCEF_GAMEGUI_CCHATFORM_H
#include "gucefGAMEGUI_CChatForm.h"
#define GUCEF_GAMEGUI_CCHATFORM_H
#endif /* GUCEF_GAMEGUI_CCHATFORM_H ? */

#ifndef GUCEF_GAMEGUI_CVOTEPROMPTFORM_H
#include "gucefGAMEGUI_CVotePromptForm.h"
#define GUCEF_GAMEGUI_CVOTEPROMPTFORM_H
#endif /* GUCEF_GAMEGUI_CVOTEPROMPTFORM_H ? */

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
 *  The screen of a lobby a player is in, bound to a CILobby: the slots (slot, team, player, faction, colour, ready,
 *  latency) in a table, the spectators, the faction and colour of the local player, the game settings (a
 *  CLobbySettingsPanelForm), the check boxes Ready and Spectate, the buttons Start, Force start, Vote start and Leave,
 *  the chat of the lobby (a CChatForm for the channel the lobby names) and a line that tells how the start goes.
 *
 *  What the player can do follows the role and the phase of the lobby:
 *      Start and Force start   the boss, while the lobby is open
 *      Vote start              a player that is not the boss, while the lobby is open and no vote runs
 *      Ready                   a player, while the lobby is open or counting down (becoming unready stops the countdown)
 *      Spectate                while the lobby is open
 *      Faction and Colour      a player, while the lobby is open
 *      Leave                   always
 *  A double click on a free slot takes it. The right mouse button on another player's slot reports the player
 *  (ReportPlayer(), with the reason of SetReportReason()). The boss has more in that menu while the lobby is open: for
 *  a slot open, close or give it an AI, for a player kick, kick and ban, or make the boss. A vote that asks the player
 *  is shown by a CVotePromptForm.
 *
 *  A refusal of a request shows as the status line, with the reason the lobby gave; the countdown shows above the
 *  table. The launch description
 *  that starts the game is passed on with LaunchEvent; the end of the lobby (the player left, the lobby closed) with
 *  LeftEvent. The form never lets go of the lobby by itself: the application takes it away with SetLobby().
 *  The layout is built in code and made for BuildDarkTheme().
 */
class GUCEF_GAMEGUI_EXPORT_CPP CLobbyForm : public GUI::CFormEx
{
    public:

    static const CORE::CEvent LaunchEvent;      /**< MULTIPLAY::CLaunchDescription: the game starts, for the local member */
    static const CORE::CEvent LeftEvent;        /**< TLeftEventData: why the lobby is over for the player, empty when the player left */

    typedef CORE::TCloneableString TLeftEventData;

    static void RegisterEvents( void );

    /** The columns of the table of slots, in order */
    enum EColumn
    {
        COLUMN_SLOT = 0 ,
        COLUMN_TEAM     ,
        COLUMN_PLAYER   ,
        COLUMN_FACTION  ,
        COLUMN_COLOUR   ,
        COLUMN_READY    ,
        COLUMN_LATENCY  ,
        COLUMN_COUNT
    };
    typedef enum EColumn TColumn;

    CLobbyForm( void );

    CLobbyForm( GUI::CIGUIContext* context );

    virtual ~CLobbyForm();

    /** Makes the window, hidden */
    bool Build( void );

    /** Makes the window when needed and shows it */
    bool Open( void );

    /** The lobby the form shows and acts on; none to show nothing */
    void SetLobby( MULTIPLAY::CILobbyPtr lobby );

    const MULTIPLAY::CILobbyPtr& GetLobby( void ) const;

    /** The chat service for the chat of the lobby, not owned */
    void SetChatService( MULTIPLAY::CIChatService* chatService );

    MULTIPLAY::CIChatService* GetChatService( void ) const;

    /** The type of AI the menu of the boss gives a slot */
    void SetAiType( const CORE::CString& aiType );

    const CORE::CString& GetAiType( void ) const;

    /** The reason the menu reports a player with, "Reported from the lobby" by default */
    void SetReportReason( const CORE::CString& reason );

    const CORE::CString& GetReportReason( void ) const;

    /** Reports a member to the people who run the lobby service; the status line tells how it went */
    bool ReportPlayer( const CORE::CString& memberId, const CORE::CString& reason );

    /** The embedded forms, NULL before the form is built */
    CChatForm* GetChatForm( void ) const;
    CLobbySettingsPanelForm* GetSettingsPanel( void ) const;
    CVotePromptForm* GetVotePrompt( void ) const;

    /** Shows the lobby again: the table, the controls and what the buttons can do. Done whenever the lobby changes. */
    void Refresh( void );

    /** The start of the game, as the Start button asks for it: the boss starts, another player starts a vote */
    bool RequestStart( void );

    bool ForceStart( void );

    /** Leaves the lobby and notifies LeftEvent */
    bool Leave( void );

    /** The text of the line that tells how the start goes */
    CORE::CString GetCountdownText( void );

    /** The text of the line that tells what was refused, or the lobby that closed */
    CORE::CString GetStatusText( void );

    /** The text of the line that tells about the vote that runs, empty when there is none */
    CORE::CString GetVoteText( void );

    /** The seconds left of a countdown, -1 when none runs */
    Int32 GetCountdown( void ) const;

    /** The row of the table a member is in, -1 when the member has no slot */
    Int32 FindRowOfMember( const CORE::CString& memberId ) const;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual void OnPreLayoutLoad( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void OnPostLayoutLoad( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void OnNotify( CORE::CNotifier* notifier                 ,
                           const CORE::CEvent& eventId               ,
                           CORE::CICloneable* eventData = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    void BuildLayout( CORE::CDataNode& layout );
    void ApplyLobbyToChildren( void );
    void ShowSlots( void );
    void ShowSpectators( void );
    void ShowControls( void );
    void ShowTexts( void );
    void SetLabelText( const char* widgetName, const CORE::CString& text );
    void SetEnabled( const char* widgetName, bool enabled );
    void OpenSlotMenu( Int32 x, Int32 y, Int32 row );
    void OnSlotMenuItem( const CORE::CString& item );
    void OnSlotActivated( Int32 row );

    CLobbyForm( const CLobbyForm& src );
    CLobbyForm& operator=( const CLobbyForm& src );

    private:

    MULTIPLAY::CILobbyPtr m_lobby;
    MULTIPLAY::CIChatService* m_chatService;
    CChatForm* m_chat;
    CLobbySettingsPanelForm* m_settingsPanel;
    CVotePromptForm* m_votePrompt;
    GUI::CPopupMenu* m_slotMenu;
    UInt32 m_menuSlot;
    CORE::CString m_menuMember;
    CORE::CString m_aiType;
    CORE::CString m_reportReason;
    Int32 m_countdown;
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

#endif /* GUCEF_GAMEGUI_CLOBBYFORM_H ? */
