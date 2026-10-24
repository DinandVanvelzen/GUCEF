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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CLONEABLES_H
#include "cloneables.h"
#define GUCEF_CORE_CLONEABLES_H
#endif /* GUCEF_CORE_CLONEABLES_H ? */

#ifndef GUCEF_GUI_CROUTEDEVENTDATA_H
#include "gucefGUI_CRoutedEventData.h"
#define GUCEF_GUI_CROUTEDEVENTDATA_H
#endif /* GUCEF_GUI_CROUTEDEVENTDATA_H ? */

#ifndef GUCEF_GUI_CFORMLAYOUT_H
#include "gucefGUI_CFormLayout.h"
#define GUCEF_GUI_CFORMLAYOUT_H
#endif /* GUCEF_GUI_CFORMLAYOUT_H ? */

#ifndef GUCEF_GUI_CGUICONTEXT_H
#include "gucefGUI_CGUIContext.h"
#define GUCEF_GUI_CGUICONTEXT_H
#endif /* GUCEF_GUI_CGUICONTEXT_H ? */

#ifndef GUCEF_GAMEGUI_UTILS_H
#include "gucefGAMEGUI_Utils.h"
#define GUCEF_GAMEGUI_UTILS_H
#endif /* GUCEF_GAMEGUI_UTILS_H ? */

#include "gucefGAMEGUI_CVotePromptForm.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GAMEGUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CEvent CVotePromptForm::VoteCastEvent = "GUCEF::GAMEGUI::CVotePromptForm::VoteCastEvent";
const CORE::CEvent CVotePromptForm::ClosedEvent = "GUCEF::GAMEGUI::CVotePromptForm::ClosedEvent";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CVoteCastEventData::CVoteCastEventData( void )
    : CORE::CICloneable()
    , voteId()
    , yes( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CVoteCastEventData::CVoteCastEventData( const CORE::CString& voteId_, bool yes_ )
    : CORE::CICloneable()
    , voteId( voteId_ )
    , yes( yes_ )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CVoteCastEventData::~CVoteCastEventData()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CVoteCastEventData::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CVoteCastEventData( *this );
}

/*-------------------------------------------------------------------------*/

void
CVotePromptForm::RegisterEvents( void )
{GUCEF_TRACE;

    VoteCastEvent.Initialize();
    ClosedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CVotePromptForm::CVotePromptForm( void )
    : GUI::CFormEx()
    , m_lobby()
    , m_vote()
    , m_proposerName()
    , m_open( false )
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CVotePromptForm::CVotePromptForm( GUI::CIGUIContext* context )
    : GUI::CFormEx( context )
    , m_lobby()
    , m_vote()
    , m_proposerName()
    , m_open( false )
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CVotePromptForm::~CVotePromptForm()
{GUCEF_TRACE;

    UnsubscribeAllFromObserver();
}

/*-------------------------------------------------------------------------*/

void
CVotePromptForm::BuildLayout( CORE::CDataNode& layout )
{GUCEF_TRACE;

    CORE::CDataNode* window = GUI::CFormLayout::AddWidgetNode( layout, "Window", "VotePromptForm", "0.5,-260 0.5,-140", "0,520 0,280" );
    GUI::CFormLayout::SetWidgetNodeProperty( *window, "Title", "Vote" );

    CORE::CDataNode* what = GUI::CFormLayout::AddWidgetNode( *window, "Label", "What", "0,20 0,56", "1,-40 0,72" );
    GUI::CFormLayout::SetWidgetNodeProperty( *what, "WordWrap", "true" );
    GUI::CFormLayout::AddWidgetNode( *window, "Label", "Tally", "0,20 0,136", "1,-40 0,24" );
    GUI::CFormLayout::AddWidgetNode( *window, "Label", "TimeLeft", "0,20 0,164", "1,-40 0,24" );

    CORE::CDataNode* yes = GUI::CFormLayout::AddWidgetNode( *window, "Button", "Yes", "1,-252 1,-56", "0,112 0,38" );
    GUI::CFormLayout::SetWidgetNodeProperty( *yes, "Text", "Yes" );
    GUI::CFormLayout::SetWidgetNodeProperty( *yes, "StyleClass", "Primary" );
    CORE::CDataNode* no = GUI::CFormLayout::AddWidgetNode( *window, "Button", "No", "1,-128 1,-56", "0,112 0,38" );
    GUI::CFormLayout::SetWidgetNodeProperty( *no, "Text", "No" );
}

/*-------------------------------------------------------------------------*/

bool
CVotePromptForm::Build( void )
{GUCEF_TRACE;

    CORE::CDataNode layout;
    GUI::CFormLayout::InitLayout( layout );
    BuildLayout( layout );
    if ( !LoadLayout( layout ) )
        return false;
    Hide();
    m_open = false;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CVotePromptForm::OnPostLayoutLoad( void )
{GUCEF_TRACE;

    GUI::CFormEx::OnPostLayoutLoad();

    const char* buttons[] = { "Yes", "No" };
    for ( UInt32 i=0; i<2; ++i )
    {
        GUI::CWidget* button = GetWidget( buttons[ i ] );
        if ( GUCEF_NULL != button )
            SubscribeTo( button, GUI::CWidget::ActivatedEvent );
    }
    GUI::CWidget* root = GetRootWidget();
    if ( GUCEF_NULL != root )
        SubscribeTo( root, GUI::CWidget::NavigationEvent );
    GUI::CGUIContext* context = GetGUIContext();
    if ( GUCEF_NULL != context )
        SubscribeTo( context, GUI::CGUIContext::FrameStartedEvent );
}

/*-------------------------------------------------------------------------*/

void
CVotePromptForm::SetLabelText( const char* widgetName, const CORE::CString& text )
{GUCEF_TRACE;

    GUI::CWidget* label = GetWidget( widgetName );
    if ( GUCEF_NULL != label )
        label->SetProperty( "Text", text );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CVotePromptForm::DescribeVote( const MULTIPLAY::CLobbyVote& vote             ,
                               const CORE::CString& proposerName             ,
                               const MULTIPLAY::CLobbySettingsSchema* schema )
{GUCEF_TRACE;

    if ( "setting" == vote.what )
    {
        CORE::CString name = vote.key;
        if ( GUCEF_NULL != schema )
        {
            const MULTIPLAY::CLobbySettingDef* def = schema->FindSetting( vote.key );
            if ( GUCEF_NULL != def && !def->label.IsNULLOrEmpty() )
                name = def->label;
        }
        return proposerName + " proposes to change " + name + " to " + vote.value;
    }
    if ( "start" == vote.what )
        return proposerName + " proposes to start the game";
    return proposerName + " proposes: " + vote.what;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CVotePromptForm::DescribeTally( const MULTIPLAY::CLobbyVote& vote )
{GUCEF_TRACE;

    return "Yes " + CORE::UInt32ToString( vote.yes ) + "  No " + CORE::UInt32ToString( vote.no ) + "  (" + CORE::UInt32ToString( vote.needed ) + " needed)";
}

/*-------------------------------------------------------------------------*/

void
CVotePromptForm::ShowVote( void )
{GUCEF_TRACE;

    const MULTIPLAY::CLobbySettingsSchema* schema = m_lobby.IsNULL() ? GUCEF_NULL : &m_lobby->GetSchema();
    SetLabelText( "What", DescribeVote( m_vote, m_proposerName, schema ) );
    SetLabelText( "Tally", DescribeTally( m_vote ) );
    Update();
}

/*-------------------------------------------------------------------------*/

void
CVotePromptForm::Update( void )
{GUCEF_TRACE;

    if ( !m_open )
        return;
    Int32 seconds = GetSecondsUntil( m_vote.deadline );
    SetLabelText( "TimeLeft", seconds < 0 ? CORE::CString::Empty : "Time left: " + CORE::Int32ToString( seconds ) + " s" );
}

/*-------------------------------------------------------------------------*/

bool
CVotePromptForm::Open( const MULTIPLAY::CLobbyVote& vote, const CORE::CString& proposerName )
{GUCEF_TRACE;

    if ( GUCEF_NULL == GetRootWidget() && !Build() )
        return false;

    m_vote = vote;
    m_proposerName = proposerName;
    m_open = true;
    ShowVote();
    return ShowModal();
}

/*-------------------------------------------------------------------------*/

bool
CVotePromptForm::Cast( bool yes )
{GUCEF_TRACE;

    if ( !m_open )
        return false;

    CVoteCastEventData eventData( m_vote.id, yes );
    MULTIPLAY::CILobbyPtr lobby = m_lobby;
    Close();
    NotifyObservers( VoteCastEvent, &eventData );
    if ( !lobby.IsNULL() )
        lobby->Vote( eventData.voteId, yes );
    NotifyObservers( ClosedEvent );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CVotePromptForm::Close( void )
{GUCEF_TRACE;

    m_open = false;
    Hide();
}

/*-------------------------------------------------------------------------*/

bool
CVotePromptForm::IsOpen( void ) const
{GUCEF_TRACE;

    return m_open;
}

/*-------------------------------------------------------------------------*/

const MULTIPLAY::CLobbyVote&
CVotePromptForm::GetVote( void ) const
{GUCEF_TRACE;

    return m_vote;
}

/*-------------------------------------------------------------------------*/

bool
CVotePromptForm::CanVoteOn( const MULTIPLAY::CLobbyVote& vote ) const
{GUCEF_TRACE;

    if ( m_lobby.IsNULL() || !vote.IsActive() )
        return false;
    const MULTIPLAY::CLobbyState& state = m_lobby->GetState();
    const CORE::CString& localId = m_lobby->GetLocalMemberId();
    const MULTIPLAY::CLobbyMember* member = state.FindMember( localId );
    if ( GUCEF_NULL == member || member->IsSpectator() )
        return false;
    if ( vote.proposer == localId )
        return false;
    return vote.voters.find( localId ) == vote.voters.end();
}

/*-------------------------------------------------------------------------*/

void
CVotePromptForm::OpenForLobbyVote( const MULTIPLAY::CLobbyVote& vote )
{GUCEF_TRACE;

    if ( !CanVoteOn( vote ) )
        return;
    Open( vote, GetMemberName( m_lobby->GetState(), vote.proposer ) );
}

/*-------------------------------------------------------------------------*/

void
CVotePromptForm::SetLobby( MULTIPLAY::CILobbyPtr lobby )
{GUCEF_TRACE;

    if ( !m_lobby.IsNULL() )
        UnsubscribeFrom( m_lobby.GetPointerAlways() );
    if ( m_open )
        Close();

    m_lobby = lobby;
    if ( m_lobby.IsNULL() )
        return;

    SubscribeTo( m_lobby.GetPointerAlways() );
    OpenForLobbyVote( m_lobby->GetState().activeVote );
}

/*-------------------------------------------------------------------------*/

const MULTIPLAY::CILobbyPtr&
CVotePromptForm::GetLobby( void ) const
{GUCEF_TRACE;

    return m_lobby;
}

/*-------------------------------------------------------------------------*/

void
CVotePromptForm::OnNotify( CORE::CNotifier* notifier    ,
                           const CORE::CEvent& eventId  ,
                           CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( GUI::CWidget::ActivatedEvent == eventId )
    {
        if ( notifier == GetWidget( "Yes" ) )
            Cast( true );
        else
        if ( notifier == GetWidget( "No" ) )
            Cast( false );
        return;
    }
    if ( GUI::CGUIContext::FrameStartedEvent == eventId )
    {
        Update();
        return;
    }
    if ( GUI::CWidget::NavigationEvent == eventId && GUCEF_NULL != eventData )
    {
        GUI::CNavigationEventData* navigation = static_cast< GUI::CNavigationEventData* >( eventData );
        if ( GUI::NAVIGATION_CANCEL == navigation->direction && GUI::EVENTPHASE_TUNNEL != navigation->phase && m_open )
        {
            navigation->handled = true;
            Cast( false );
        }
        return;
    }
    if ( !m_lobby.IsNULL() && notifier == m_lobby.GetPointerAlways() )
    {
        if ( MULTIPLAY::CILobby::VoteStartedEvent == eventId && GUCEF_NULL != eventData )
        {
            const MULTIPLAY::CLobbyVoteEventData* started = static_cast< const MULTIPLAY::CLobbyVoteEventData* >( eventData );
            OpenForLobbyVote( started->vote );
            return;
        }
        if ( MULTIPLAY::CILobby::VoteEndedEvent == eventId && GUCEF_NULL != eventData )
        {
            const MULTIPLAY::CLobbyVoteEventData* ended = static_cast< const MULTIPLAY::CLobbyVoteEventData* >( eventData );
            if ( m_open && ended->vote.id == m_vote.id )
            {
                Close();
                NotifyObservers( ClosedEvent );
            }
            return;
        }
        if ( MULTIPLAY::CILobby::LobbyStateChangedEvent == eventId && m_open )
        {
            // the tally moves on, and the vote is gone or answered by another route
            const MULTIPLAY::CLobbyVote& current = m_lobby->GetState().activeVote;
            if ( !current.IsActive() || current.id != m_vote.id || current.voters.find( m_lobby->GetLocalMemberId() ) != current.voters.end() )
            {
                Close();
                NotifyObservers( ClosedEvent );
                return;
            }
            m_vote = current;
            ShowVote();
        }
        return;
    }
    GUI::CFormEx::OnNotify( notifier, eventId, eventData );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CVotePromptForm::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::GAMEGUI::CVotePromptForm";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GAMEGUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
