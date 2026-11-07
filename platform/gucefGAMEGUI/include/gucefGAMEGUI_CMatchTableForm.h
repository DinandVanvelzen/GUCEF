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

#ifndef GUCEF_GAMEGUI_CMATCHTABLEFORM_H
#define GUCEF_GAMEGUI_CMATCHTABLEFORM_H

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

#ifndef GUCEF_GAMEGUI_MACROS_H
#include "gucefGAMEGUI_macros.h"
#define GUCEF_GAMEGUI_MACROS_H
#endif /* GUCEF_GAMEGUI_MACROS_H ? */

#ifndef GUCEF_GAMEGUI_CMATCHTABLEACTION_H
#include "gucefGAMEGUI_CMatchTableAction.h"
#define GUCEF_GAMEGUI_CMATCHTABLEACTION_H
#endif /* GUCEF_GAMEGUI_CMATCHTABLEACTION_H ? */

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
 *  The table of a game by mail: who plays, with their mail addresses and where they stand, said plainly to be visible to all players; where the
 *  last turn of the player is on its way; and what the player can do: send the turn again, ask the
 *  table for a turn that did not come, put the newest turn in a file or in text and take a turn from a file or from pasted text, propose to hand
 *  a player who does not answer to the computer, vote on the proposals, and carry out a decision that passed. The form does none of it:
 *  it tells the application what the player asked for. A non-modal window that moves.
 *
 *  The invitations and the games that wait for their players are the lobbies of the mailbox (CLobbyForm), not this table.
 */
class GUCEF_GAMEGUI_EXPORT_CPP CMatchTableForm : public GUI::CFormEx
{
    public:

    static const CORE::CEvent ProposeRequestedEvent;        /**< CMatchTableAction: the id of the player to hand to the computer */
    static const CORE::CEvent VoteYesRequestedEvent;        /**< CMatchTableAction: the id of the proposal */
    static const CORE::CEvent VoteNoRequestedEvent;         /**< CMatchTableAction: the id of the proposal */
    static const CORE::CEvent ApplyRequestedEvent;          /**< no data: carry out the decision that passed for the player who is up */
    static const CORE::CEvent ResendRequestedEvent;         /**< no data: send the turn of the player again */
    static const CORE::CEvent AskMissingRequestedEvent;     /**< no data: ask the table for the turn that did not come */
    static const CORE::CEvent ExportFileRequestedEvent;     /**< CMatchTableAction: the path of the file */
    static const CORE::CEvent ExportTextRequestedEvent;     /**< no data */
    static const CORE::CEvent ImportFileRequestedEvent;     /**< CMatchTableAction: the path of the file */
    static const CORE::CEvent ImportTextRequestedEvent;     /**< CMatchTableAction: the pasted text */
    static const CORE::CEvent CloseRequestedEvent;          /**< no data: the form is closed */

    static void RegisterEvents( void );

    CMatchTableForm( void );

    CMatchTableForm( GUI::CIGUIContext* context );

    virtual ~CMatchTableForm();

    bool Build( void );
    bool Open( void );
    void Close( void );
    bool IsOpen( void ) const;

    /** The name of the game and the lines of the table, one per player, as the application writes them ("Alice (you, made the game), alice@mail.test, playing") */
    void SetTable( const CORE::CString& title, const CORE::CString::StringVector& lines );

    /** The settings of the game in plain words, one line each, shown next to the table: what the player agrees to by accepting an invitation */
    void SetSummary( const CORE::CString::StringVector& lines );
    CORE::CString GetSummary( void );

    /** The statement about the mail addresses, shown with the table */
    void SetStatement( const CORE::CString& text );


    /** Where the turn of the player is on its way */
    void SetDelivery( const CORE::CString& text );
    CORE::CString GetDelivery( void );

    /** The players a proposal can be about: their ids and names */
    void SetTargets( const CORE::CString::StringVector& ids, const CORE::CString::StringVector& names );

    /** The proposals: their ids and the lines that say how each stands */
    void SetProposals( const CORE::CString::StringVector& ids, const CORE::CString::StringVector& lines );

    /** Whether a decision that passed can be carried out now */
    void SetApplyEnabled( bool enabled );

    /** What the last request led to; 'problem' draws it in red */
    void SetMessage( const CORE::CString& text, bool problem );
    CORE::CString GetMessage( void );

    /** Text the player can read and keep: the turn as text */
    void SetOutput( const CORE::CString& text );
    CORE::CString GetOutput( void );

    /** The path the last export or import named */
    void SetPath( const CORE::CString& path );
    CORE::CString GetPath( void );

    /** As the buttons do */
    void RequestPropose( void );
    void RequestVote( bool yes );
    void RequestApply( void );
    void RequestResend( void );
    void RequestAskMissing( void );
    void RequestExportFile( void );
    void RequestExportText( void );
    void RequestImportFile( void );
    void RequestImportText( void );
    void RequestClose( void );

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual void OnPostLayoutLoad( void ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    typedef CORE::CTEventHandlerFunctor< CMatchTableForm > TEventCallback;

    CMatchTableForm( const CMatchTableForm& src );
    CMatchTableForm& operator=( const CMatchTableForm& src );

    void BuildLayout( CORE::CDataNode& layout );
    void ShowParts( void );
    void Subscribe( const char* widgetName, TEventCallback& callback );
    void NotifyAction( const CORE::CEvent& eventId, const CORE::CString& text );
    void OnProposeActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnVoteYesActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnVoteNoActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnApplyActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnResendActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnAskMissingActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnExportFileActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnExportTextActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnImportFileActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnImportTextActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnCloseActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnNavigation( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );

    CORE::CString::StringVector m_targetIds;
    CORE::CString::StringVector m_proposalIds;
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

#endif /* GUCEF_GAMEGUI_CMATCHTABLEFORM_H ? */
