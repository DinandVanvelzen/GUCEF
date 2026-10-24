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

#ifndef GUCEF_GAMEGUI_CVOTEPROMPTFORM_H
#define GUCEF_GAMEGUI_CVOTEPROMPTFORM_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GUI_CFORMEX_H
#include "gucefGUI_CFormEx.h"
#define GUCEF_GUI_CFORMEX_H
#endif /* GUCEF_GUI_CFORMEX_H ? */

#ifndef GUCEF_MULTIPLAY_CILOBBY_H
#include "gucefMULTIPLAY_CILobby.h"
#define GUCEF_MULTIPLAY_CILOBBY_H
#endif /* GUCEF_MULTIPLAY_CILOBBY_H ? */

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

/** The data of CVotePromptForm::VoteCastEvent */
class GUCEF_GAMEGUI_EXPORT_CPP CVoteCastEventData : public CORE::CICloneable
{
    public:

    CORE::CString voteId;
    bool yes;

    CVoteCastEventData( void );

    CVoteCastEventData( const CORE::CString& voteId, bool yes );

    virtual ~CVoteCastEventData();

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------*/

/**
 *  A modal window that asks the player to vote: what is proposed by whom, the tally, the time left, and the
 *  buttons Yes and No (Escape is No). A vote is cast with VoteCastEvent (CVoteCastEventData).
 *
 *  Bound to a lobby (SetLobby()) the form opens by itself when a vote starts that the local member can still vote on
 *  (not the proposer, not a spectator), keeps the tally current, casts with CILobby::Vote(), and closes when the vote
 *  ends or the member voted. Without a lobby, Open() shows a vote the application describes.
 *  The layout is built in code and made for BuildDarkTheme().
 */
class GUCEF_GAMEGUI_EXPORT_CPP CVotePromptForm : public GUI::CFormEx
{
    public:

    static const CORE::CEvent VoteCastEvent;        /**< CVoteCastEventData */
    static const CORE::CEvent ClosedEvent;          /**< no data: the prompt is gone, after a vote or because the vote ended */

    static void RegisterEvents( void );

    CVotePromptForm( void );

    CVotePromptForm( GUI::CIGUIContext* context );

    virtual ~CVotePromptForm();

    /** Makes the window, hidden */
    bool Build( void );

    /** Watches the lobby for votes, none to stop; a vote that runs already is opened when the member can vote on it */
    void SetLobby( MULTIPLAY::CILobbyPtr lobby );

    const MULTIPLAY::CILobbyPtr& GetLobby( void ) const;

    /** Shows a vote, modal */
    bool Open( const MULTIPLAY::CLobbyVote& vote, const CORE::CString& proposerName );

    /** Casts the vote on the prompt and closes it; false when no prompt is open */
    bool Cast( bool yes );

    void Close( void );

    bool IsOpen( void ) const;

    const MULTIPLAY::CLobbyVote& GetVote( void ) const;

    /** Shows the time left until the deadline of the vote, done on every frame of the context */
    void Update( void );

    /** The question as the prompt shows it. The schema, when given, names the setting. */
    static CORE::CString DescribeVote( const MULTIPLAY::CLobbyVote& vote             ,
                                       const CORE::CString& proposerName             ,
                                       const MULTIPLAY::CLobbySettingsSchema* schema );

    /** "Yes 1  No 0  (2 needed)" */
    static CORE::CString DescribeTally( const MULTIPLAY::CLobbyVote& vote );

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual void OnPostLayoutLoad( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void OnNotify( CORE::CNotifier* notifier                 ,
                           const CORE::CEvent& eventId               ,
                           CORE::CICloneable* eventData = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    void BuildLayout( CORE::CDataNode& layout );
    void ShowVote( void );
    bool CanVoteOn( const MULTIPLAY::CLobbyVote& vote ) const;
    void OpenForLobbyVote( const MULTIPLAY::CLobbyVote& vote );
    void SetLabelText( const char* widgetName, const CORE::CString& text );

    CVotePromptForm( const CVotePromptForm& src );
    CVotePromptForm& operator=( const CVotePromptForm& src );

    private:

    MULTIPLAY::CILobbyPtr m_lobby;
    MULTIPLAY::CLobbyVote m_vote;
    CORE::CString m_proposerName;
    bool m_open;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GAMEGUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GAMEGUI_CVOTEPROMPTFORM_H ? */
