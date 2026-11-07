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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_GUI_CROUTEDEVENTDATA_H
#include "gucefGUI_CRoutedEventData.h"
#define GUCEF_GUI_CROUTEDEVENTDATA_H
#endif /* GUCEF_GUI_CROUTEDEVENTDATA_H ? */

#ifndef GUCEF_GUI_CFORMLAYOUT_H
#include "gucefGUI_CFormLayout.h"
#define GUCEF_GUI_CFORMLAYOUT_H
#endif /* GUCEF_GUI_CFORMLAYOUT_H ? */

#ifndef GUCEF_GUI_CEDITBOX_H
#include "gucefGUI_CEditbox.h"
#define GUCEF_GUI_CEDITBOX_H
#endif /* GUCEF_GUI_CEDITBOX_H ? */

#ifndef GUCEF_GUI_CCOMBOBOX_H
#include "gucefGUI_CCombobox.h"
#define GUCEF_GUI_CCOMBOBOX_H
#endif /* GUCEF_GUI_CCOMBOBOX_H ? */

#ifndef GUCEF_GUI_CLISTBOX_H
#include "gucefGUI_CListBox.h"
#define GUCEF_GUI_CLISTBOX_H
#endif /* GUCEF_GUI_CLISTBOX_H ? */

#ifndef GUCEF_GUI_CTEXTBOX_H
#include "gucefGUI_CTextbox.h"
#define GUCEF_GUI_CTEXTBOX_H
#endif /* GUCEF_GUI_CTEXTBOX_H ? */

#ifndef GUCEF_GUI_CGUICONTEXT_H
#include "gucefGUI_CGUIContext.h"
#define GUCEF_GUI_CGUICONTEXT_H
#endif /* GUCEF_GUI_CGUICONTEXT_H ? */

#ifndef GUCEF_GAMEGUI_UTILS_H
#include "gucefGAMEGUI_Utils.h"
#define GUCEF_GAMEGUI_UTILS_H
#endif /* GUCEF_GAMEGUI_UTILS_H ? */
#include "gucefGAMEGUI_CMatchTableForm.h"

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

const CORE::CEvent CMatchTableForm::ProposeRequestedEvent = "GUCEF::GAMEGUI::CMatchTableForm::ProposeRequestedEvent";
const CORE::CEvent CMatchTableForm::VoteYesRequestedEvent = "GUCEF::GAMEGUI::CMatchTableForm::VoteYesRequestedEvent";
const CORE::CEvent CMatchTableForm::VoteNoRequestedEvent = "GUCEF::GAMEGUI::CMatchTableForm::VoteNoRequestedEvent";
const CORE::CEvent CMatchTableForm::ApplyRequestedEvent = "GUCEF::GAMEGUI::CMatchTableForm::ApplyRequestedEvent";
const CORE::CEvent CMatchTableForm::ResendRequestedEvent = "GUCEF::GAMEGUI::CMatchTableForm::ResendRequestedEvent";
const CORE::CEvent CMatchTableForm::AskMissingRequestedEvent = "GUCEF::GAMEGUI::CMatchTableForm::AskMissingRequestedEvent";
const CORE::CEvent CMatchTableForm::ExportFileRequestedEvent = "GUCEF::GAMEGUI::CMatchTableForm::ExportFileRequestedEvent";
const CORE::CEvent CMatchTableForm::ExportTextRequestedEvent = "GUCEF::GAMEGUI::CMatchTableForm::ExportTextRequestedEvent";
const CORE::CEvent CMatchTableForm::ImportFileRequestedEvent = "GUCEF::GAMEGUI::CMatchTableForm::ImportFileRequestedEvent";
const CORE::CEvent CMatchTableForm::ImportTextRequestedEvent = "GUCEF::GAMEGUI::CMatchTableForm::ImportTextRequestedEvent";
const CORE::CEvent CMatchTableForm::CloseRequestedEvent = "GUCEF::GAMEGUI::CMatchTableForm::CloseRequestedEvent";

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

const Int32 ButtonHeight = 36;
const Int32 EditHeight = 32;
const UInt32 MaxLines = 64;
const UInt32 MaxPastedBytes = 700000;

CORE::CString
Y( Int32 y )
{
    return "0," + CORE::Int32ToString( y );
}

CORE::CDataNode*
AddLabel( CORE::CDataNode& parent, const char* name, const char* text, Int32 y, Int32 height, const char* x = "0,20", const char* width = "1,-40" )
{
    CORE::CDataNode* node = GUI::CFormLayout::AddWidgetNode( parent, "Label", name, CORE::CString( x ) + " " + Y( y ), CORE::CString( width ) + " " + Y( height ) );
    GUI::CFormLayout::SetWidgetNodeProperty( *node, GUI::EWidgetProperty_Text, text );
    return node;
}

CORE::CDataNode*
AddButton( CORE::CDataNode& parent, const char* name, const char* text, const char* x, Int32 y, const char* width )
{
    CORE::CDataNode* node = GUI::CFormLayout::AddWidgetNode( parent, "Button", name, CORE::CString( x ) + " " + Y( y ), CORE::CString( width ) + " " + Y( ButtonHeight ) );
    GUI::CFormLayout::SetWidgetNodeProperty( *node, GUI::EWidgetProperty_Text, text );
    return node;
}

CORE::CString
Join( const CORE::CString::StringVector& lines )
{
    CORE::CString text;
    for ( size_t i=0; i<lines.size() && i<MaxLines; ++i )
    {
        if ( i > 0 )
            text += "\n";
        text += lines[ i ];
    }
    return text;
}

}; /* anonymous namespace */

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CMatchTableAction::CMatchTableAction( void ) : CORE::CICloneable(), text() {GUCEF_TRACE;}
CMatchTableAction::CMatchTableAction( const CORE::CString& actionText ) : CORE::CICloneable(), text( actionText ) {GUCEF_TRACE;}
CMatchTableAction::CMatchTableAction( const CMatchTableAction& src ) : CORE::CICloneable( src ), text( src.text ) {GUCEF_TRACE;}
CMatchTableAction::~CMatchTableAction() {GUCEF_TRACE;}

CMatchTableAction&
CMatchTableAction::operator=( const CMatchTableAction& src )
{GUCEF_TRACE;

    if ( &src != this )
        text = src.text;
    return *this;
}

CORE::CICloneable*
CMatchTableAction::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CMatchTableAction( *this );
}

const char*
CMatchTableAction::GetCloneableTypeName( void ) const
{GUCEF_TRACE;

    return CloneableTypeName();
}

const char*
CMatchTableAction::CloneableTypeName( void )
{GUCEF_TRACE;

    return "GUCEF::GAMEGUI::CMatchTableAction";
}

/*-------------------------------------------------------------------------*/

void
CMatchTableForm::RegisterEvents( void )
{GUCEF_TRACE;

    ProposeRequestedEvent.Initialize();
    VoteYesRequestedEvent.Initialize();
    VoteNoRequestedEvent.Initialize();
    ApplyRequestedEvent.Initialize();
    ResendRequestedEvent.Initialize();
    AskMissingRequestedEvent.Initialize();
    ExportFileRequestedEvent.Initialize();
    ExportTextRequestedEvent.Initialize();
    ImportFileRequestedEvent.Initialize();
    ImportTextRequestedEvent.Initialize();
    CloseRequestedEvent.Initialize();
}

CMatchTableForm::CMatchTableForm( void )
    : GUI::CFormEx()
    , m_targetIds()
    , m_proposalIds()
    , m_open( false )
{GUCEF_TRACE;

    RegisterEvents();
}

CMatchTableForm::CMatchTableForm( GUI::CIGUIContext* context )
    : GUI::CFormEx( context )
    , m_targetIds()
    , m_proposalIds()
    , m_open( false )
{GUCEF_TRACE;

    RegisterEvents();
}

CMatchTableForm::~CMatchTableForm()
{GUCEF_TRACE;

    UnsubscribeAllFromObserver();
}

/*-------------------------------------------------------------------------*/

void
CMatchTableForm::BuildLayout( CORE::CDataNode& layout )
{GUCEF_TRACE;

    CORE::CDataNode* window = GUI::CFormLayout::AddWidgetNode( layout, "Window", "MatchTableForm", "0.5,0 0.5,0", "0.92,0 0.92,0" );
    GUI::CFormLayout::SetWidgetNodeProperty( *window, GUI::EWidgetProperty_Title, "Game by e-mail" );
    GUI::CFormLayout::SetWidgetNodeProperty( *window, GUI::EWidgetProperty_HAlign, "Center" );
    GUI::CFormLayout::SetWidgetNodeProperty( *window, GUI::EWidgetProperty_VAlign, "Center" );
    GUI::CFormLayout::SetWidgetNodeProperty( *window, GUI::EWidgetProperty_MinSize, "0,720 0,760" );
    GUI::CFormLayout::SetWidgetNodeProperty( *window, GUI::EWidgetProperty_MaxSize, "0,980 0,960" );
    GUI::CFormLayout::SetWidgetNodeProperty( *window, GUI::EWidgetProperty_Clip, "true" );

    Int32 y = 44;
    CORE::CDataNode* title = AddLabel( *window, "Title", "", y, 24 );
    GUI::CFormLayout::SetWidgetNodeProperty( *title, GUI::EWidgetProperty_TextColour, "255,255,255,255" );
    y += 28;
    // the players on the left, the settings of the game on the right: whoever is asked to join sees what the game is
    CORE::CDataNode* tableLabel = AddLabel( *window, "TableLabel", "Players", y, 22, "0,20", "0.5,-25" );
    (void) tableLabel;
    CORE::CDataNode* summaryLabel = AddLabel( *window, "SummaryLabel", "The game", y, 22, "0.5,5", "0.5,-25" );
    (void) summaryLabel;
    y += 24;
    CORE::CDataNode* table = GUI::CFormLayout::AddWidgetNode( *window, "Textbox", "Table", "0,20 " + Y( y ), "0.5,-25 " + Y( 200 ) );
    GUI::CFormLayout::SetWidgetNodeProperty( *table, GUI::EWidgetProperty_BackgroundColour, "20,22,28,255" );
    CORE::CDataNode* summary = GUI::CFormLayout::AddWidgetNode( *window, "Textbox", "Summary", "0.5,5 " + Y( y ), "0.5,-25 " + Y( 200 ) );
    GUI::CFormLayout::SetWidgetNodeProperty( *summary, GUI::EWidgetProperty_BackgroundColour, "20,22,28,255" );
    y += 204;
    CORE::CDataNode* statement = AddLabel( *window, "Statement", "", y, 78 );
    GUI::CFormLayout::SetWidgetNodeProperty( *statement, GUI::EWidgetProperty_WordWrap, "true" );
    GUI::CFormLayout::SetWidgetNodeProperty( *statement, GUI::EWidgetProperty_TextColour, "240,205,120,255" );
    y += 82;

    // a game: the turn on its way and the table's decisions
    const Int32 gameTop = y;
    AddLabel( *window, "Delivery", "", y, 24, "0,20", "0.6,-30" );
    AddButton( *window, "Resend", "Send again", "0.6,0", y - 4, "0.2,-6" );
    AddButton( *window, "AskMissing", "Ask for missing turn", "0.8,0", y - 4, "0.2,-20" );
    y += ButtonHeight + 4;
    AddLabel( *window, "DecisionsLabel", "Decisions: hand a player who does not answer to the computer", y, 22 );
    y += 24;
    GUI::CFormLayout::AddWidgetNode( *window, "Combobox", "Target", "0,20 " + Y( y ), "0.6,-30 " + Y( EditHeight ) );
    AddButton( *window, "Propose", "Propose", "0.6,0", y - 2, "0.4,-20" );
    y += EditHeight + 6;
    GUI::CFormLayout::AddWidgetNode( *window, "Listbox", "Proposals", "0,20 " + Y( y ), "1,-40 " + Y( 84 ) );
    y += 88;
    AddButton( *window, "VoteYes", "Vote yes", "0,20", y, "0.2,-10" );
    AddButton( *window, "VoteNo", "Vote no", "0.2,10", y, "0.2,-10" );
    AddButton( *window, "Apply", "Carry out the decision", "0.4,20", y, "0.6,-40" );
    y += ButtonHeight + 8;
    AddLabel( *window, "FilesLabel", "Without mail: put the newest turn in a file or in text, or take a turn from a file or from text", y, 22 );
    y += 24;
    GUI::CFormLayout::AddWidgetNode( *window, "Editbox", "Path", "0,20 " + Y( y ), "0.55,-30 " + Y( EditHeight ) );
    AddButton( *window, "ExportFile", "Export file", "0.55,0", y - 2, "0.15,-4" );
    AddButton( *window, "ImportFile", "Import file", "0.7,0", y - 2, "0.15,-4" );
    AddButton( *window, "ExportText", "Show as text", "0.85,0", y - 2, "0.15,-20" );
    y += EditHeight + 6;
    CORE::CDataNode* paste = GUI::CFormLayout::AddWidgetNode( *window, "Editbox", "ImportTextField", "0,20 " + Y( y ), "0.7,-30 " + Y( EditHeight ) );
    GUI::CFormLayout::SetWidgetNodeProperty( *paste, GUI::EWidgetProperty_MaxLength, CORE::UInt32ToString( MaxPastedBytes ) );
    AddButton( *window, "ImportText", "Import text", "0.7,0", y - 2, "0.3,-20" );
    y += EditHeight + 6;
    CORE::CDataNode* output = GUI::CFormLayout::AddWidgetNode( *window, "Textbox", "Output", "0,20 " + Y( y ), "1,-40 " + Y( 70 ) );
    GUI::CFormLayout::SetWidgetNodeProperty( *output, GUI::EWidgetProperty_BackgroundColour, "20,22,28,255" );
    y += 74;
    (void) gameTop;

    CORE::CDataNode* footer = GUI::CFormLayout::AddWidgetNode( *window, "Panel", "ButtonRow", "0,1 1,-52", "1,-2 0,50" );
    GUI::CFormLayout::SetWidgetNodeProperty( *footer, GUI::EWidgetProperty_BackgroundColour, "18,20,26,255" );
    CORE::CDataNode* message = AddLabel( *footer, "Message", "", 12, 26, "0,20", "1,-160" );
    GUI::CFormLayout::SetWidgetNodeProperty( *message, GUI::EWidgetProperty_TextColour, "170,225,170,255" );
    CORE::CDataNode* close = GUI::CFormLayout::AddWidgetNode( *footer, "Button", "Close", "1,-128 0,6", "0,112 0,38" );
    GUI::CFormLayout::SetWidgetNodeProperty( *close, GUI::EWidgetProperty_Text, "Close" );
}

bool
CMatchTableForm::Build( void )
{GUCEF_TRACE;

    CORE::CDataNode layout;
    GUI::CFormLayout::InitLayout( layout );
    BuildLayout( layout );
    if ( !LoadLayout( layout ) )
        return false;
    GUI::CTextbox* table = GetWidgetAs< GUI::CTextbox >( "Table" );
    if ( GUCEF_NULL != table )
        table->SetBoolProperty( GUI::EWidgetProperty_WordWrap, false );
    ShowParts();
    Hide();
    m_open = false;
    return true;
}

void
CMatchTableForm::Subscribe( const char* widgetName, TEventCallback& callback )
{GUCEF_TRACE;

    GUI::CWidget* widget = GetWidget( widgetName );
    if ( GUCEF_NULL != widget )
        SubscribeTo( widget, GUI::CWidget::ActivatedEvent, callback );
}

void
CMatchTableForm::OnPostLayoutLoad( void )
{GUCEF_TRACE;

    GUI::CFormEx::OnPostLayoutLoad();

    TEventCallback propose( this, &CMatchTableForm::OnProposeActivated );
    Subscribe( "Propose", propose );
    TEventCallback yes( this, &CMatchTableForm::OnVoteYesActivated );
    Subscribe( "VoteYes", yes );
    TEventCallback no( this, &CMatchTableForm::OnVoteNoActivated );
    Subscribe( "VoteNo", no );
    TEventCallback apply( this, &CMatchTableForm::OnApplyActivated );
    Subscribe( "Apply", apply );
    TEventCallback resend( this, &CMatchTableForm::OnResendActivated );
    Subscribe( "Resend", resend );
    TEventCallback ask( this, &CMatchTableForm::OnAskMissingActivated );
    Subscribe( "AskMissing", ask );
    TEventCallback exportFile( this, &CMatchTableForm::OnExportFileActivated );
    Subscribe( "ExportFile", exportFile );
    TEventCallback exportText( this, &CMatchTableForm::OnExportTextActivated );
    Subscribe( "ExportText", exportText );
    TEventCallback importFile( this, &CMatchTableForm::OnImportFileActivated );
    Subscribe( "ImportFile", importFile );
    TEventCallback importText( this, &CMatchTableForm::OnImportTextActivated );
    Subscribe( "ImportText", importText );
    TEventCallback close( this, &CMatchTableForm::OnCloseActivated );
    Subscribe( "Close", close );

    GUI::CWidget* root = GetRootWidget();
    if ( GUCEF_NULL != root )
    {
        TEventCallback callback( this, &CMatchTableForm::OnNavigation );
        SubscribeTo( root, GUI::CWidget::NavigationEvent, callback );
    }
    SetApplyEnabled( false );
}

/*-------------------------------------------------------------------------*/

bool
CMatchTableForm::Open( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == GetRootWidget() && !Build() )
        return false;
    m_open = Show();
    if ( m_open && GUCEF_NULL != GetRootWidget() )
        GetRootWidget()->BringToFront();
    return m_open;
}

void
CMatchTableForm::Close( void )
{GUCEF_TRACE;

    m_open = false;
    Hide();
}

bool
CMatchTableForm::IsOpen( void ) const
{GUCEF_TRACE;

    return m_open;
}

/*-------------------------------------------------------------------------*/

void
CMatchTableForm::ShowParts( void )
{GUCEF_TRACE;

}

void
CMatchTableForm::SetTable( const CORE::CString& title, const CORE::CString::StringVector& lines )
{GUCEF_TRACE;

    GUI::CWidget* widget = GetWidget( "Title" );
    if ( GUCEF_NULL != widget )
        widget->SetProperty( GUI::EWidgetProperty_Text, title );
    widget = GetWidget( "Table" );
    if ( GUCEF_NULL != widget )
        widget->SetProperty( GUI::EWidgetProperty_Text, Join( lines ) );
}

void
CMatchTableForm::SetSummary( const CORE::CString::StringVector& lines )
{GUCEF_TRACE;

    GUI::CWidget* widget = GetWidget( "Summary" );
    if ( GUCEF_NULL != widget )
        widget->SetProperty( GUI::EWidgetProperty_Text, Join( lines ) );
}

CORE::CString
CMatchTableForm::GetSummary( void )
{GUCEF_TRACE;

    GUI::CWidget* widget = GetWidget( "Summary" );
    return GUCEF_NULL != widget ? widget->GetProperty( GUI::EWidgetProperty_Text ) : CORE::CString::Empty;
}

void
CMatchTableForm::SetStatement( const CORE::CString& text )
{GUCEF_TRACE;

    GUI::CWidget* widget = GetWidget( "Statement" );
    if ( GUCEF_NULL != widget )
        widget->SetProperty( GUI::EWidgetProperty_Text, text );
}

void
CMatchTableForm::SetDelivery( const CORE::CString& text )
{GUCEF_TRACE;

    GUI::CWidget* widget = GetWidget( "Delivery" );
    if ( GUCEF_NULL != widget )
        widget->SetProperty( GUI::EWidgetProperty_Text, text );
}

CORE::CString
CMatchTableForm::GetDelivery( void )
{GUCEF_TRACE;

    GUI::CWidget* widget = GetWidget( "Delivery" );
    return GUCEF_NULL != widget ? widget->GetProperty( GUI::EWidgetProperty_Text ) : CORE::CString::Empty;
}

void
CMatchTableForm::SetTargets( const CORE::CString::StringVector& ids, const CORE::CString::StringVector& names )
{GUCEF_TRACE;

    m_targetIds = ids;
    GUI::CCombobox* target = GetWidgetAs< GUI::CCombobox >( "Target" );
    if ( GUCEF_NULL == target )
        return;
    target->SetListItems( names );
    target->SetSelectedIndex( names.empty() ? -1 : 0 );
}

void
CMatchTableForm::SetProposals( const CORE::CString::StringVector& ids, const CORE::CString::StringVector& lines )
{GUCEF_TRACE;

    GUI::CListbox* list = GetWidgetAs< GUI::CListbox >( "Proposals" );
    if ( GUCEF_NULL == list )
        return;
    const Int32 selected = list->GetSelectedIndex();
    m_proposalIds = ids;
    list->SetItems( lines );
    list->SetSelectedIndex( selected >= 0 && selected < (Int32) lines.size() ? selected : -1 );
}

void
CMatchTableForm::SetApplyEnabled( bool enabled )
{GUCEF_TRACE;

    GUI::CWidget* apply = GetWidget( "Apply" );
    if ( GUCEF_NULL != apply )
        apply->SetBoolProperty( GUI::EWidgetProperty_Enabled, enabled );
}

void
CMatchTableForm::SetMessage( const CORE::CString& text, bool problem )
{GUCEF_TRACE;

    GUI::CWidget* widget = GetWidget( "Message" );
    if ( GUCEF_NULL == widget )
        return;
    widget->SetProperty( GUI::EWidgetProperty_Text, text );
    widget->SetProperty( GUI::EWidgetProperty_TextColour, problem ? "255,110,110,255" : "170,225,170,255" );
}

CORE::CString
CMatchTableForm::GetMessage( void )
{GUCEF_TRACE;

    GUI::CWidget* widget = GetWidget( "Message" );
    return GUCEF_NULL != widget ? widget->GetProperty( GUI::EWidgetProperty_Text ) : CORE::CString::Empty;
}

void
CMatchTableForm::SetOutput( const CORE::CString& text )
{GUCEF_TRACE;

    GUI::CWidget* widget = GetWidget( "Output" );
    if ( GUCEF_NULL != widget )
        widget->SetProperty( GUI::EWidgetProperty_Text, text );
}

CORE::CString
CMatchTableForm::GetOutput( void )
{GUCEF_TRACE;

    GUI::CWidget* widget = GetWidget( "Output" );
    return GUCEF_NULL != widget ? widget->GetProperty( GUI::EWidgetProperty_Text ) : CORE::CString::Empty;
}

void
CMatchTableForm::SetPath( const CORE::CString& path )
{GUCEF_TRACE;

    GUI::CWidget* widget = GetWidget( "Path" );
    if ( GUCEF_NULL != widget )
        widget->SetProperty( GUI::EWidgetProperty_Text, path );
}

CORE::CString
CMatchTableForm::GetPath( void )
{GUCEF_TRACE;

    GUI::CWidget* widget = GetWidget( "Path" );
    return GUCEF_NULL != widget ? TrimText( widget->GetProperty( GUI::EWidgetProperty_Text ) ) : CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

void
CMatchTableForm::NotifyAction( const CORE::CEvent& eventId, const CORE::CString& text )
{GUCEF_TRACE;

    CMatchTableAction action( text );
    NotifyObservers( eventId, &action );
}

void CMatchTableForm::RequestApply( void ) { NotifyObservers( ApplyRequestedEvent ); }
void CMatchTableForm::RequestResend( void ) { NotifyObservers( ResendRequestedEvent ); }
void CMatchTableForm::RequestAskMissing( void ) { NotifyObservers( AskMissingRequestedEvent ); }
void CMatchTableForm::RequestExportText( void ) { NotifyObservers( ExportTextRequestedEvent ); }

void
CMatchTableForm::RequestClose( void )
{GUCEF_TRACE;

    Close();
    NotifyObservers( CloseRequestedEvent );
}

void
CMatchTableForm::RequestPropose( void )
{GUCEF_TRACE;

    GUI::CCombobox* target = GetWidgetAs< GUI::CCombobox >( "Target" );
    const Int32 index = GUCEF_NULL != target ? target->GetSelectedIndex() : -1;
    if ( index >= 0 && index < (Int32) m_targetIds.size() )
        NotifyAction( ProposeRequestedEvent, m_targetIds[ index ] );
}

void
CMatchTableForm::RequestVote( bool yes )
{GUCEF_TRACE;

    GUI::CListbox* list = GetWidgetAs< GUI::CListbox >( "Proposals" );
    const Int32 index = GUCEF_NULL != list ? list->GetSelectedIndex() : -1;
    if ( index >= 0 && index < (Int32) m_proposalIds.size() )
        NotifyAction( yes ? VoteYesRequestedEvent : VoteNoRequestedEvent, m_proposalIds[ index ] );
}

void
CMatchTableForm::RequestExportFile( void )
{GUCEF_TRACE;

    NotifyAction( ExportFileRequestedEvent, GetPath() );
}

void
CMatchTableForm::RequestImportFile( void )
{GUCEF_TRACE;

    NotifyAction( ImportFileRequestedEvent, GetPath() );
}

void
CMatchTableForm::RequestImportText( void )
{GUCEF_TRACE;

    GUI::CWidget* field = GetWidget( "ImportTextField" );
    NotifyAction( ImportTextRequestedEvent, GUCEF_NULL != field ? field->GetProperty( GUI::EWidgetProperty_Text ) : CORE::CString::Empty );
}

void CMatchTableForm::OnProposeActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData ) {GUCEF_TRACE; RequestPropose(); }
void CMatchTableForm::OnVoteYesActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData ) {GUCEF_TRACE; RequestVote( true ); }
void CMatchTableForm::OnVoteNoActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData ) {GUCEF_TRACE; RequestVote( false ); }
void CMatchTableForm::OnApplyActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData ) {GUCEF_TRACE; RequestApply(); }
void CMatchTableForm::OnResendActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData ) {GUCEF_TRACE; RequestResend(); }
void CMatchTableForm::OnAskMissingActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData ) {GUCEF_TRACE; RequestAskMissing(); }
void CMatchTableForm::OnExportFileActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData ) {GUCEF_TRACE; RequestExportFile(); }
void CMatchTableForm::OnExportTextActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData ) {GUCEF_TRACE; RequestExportText(); }
void CMatchTableForm::OnImportFileActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData ) {GUCEF_TRACE; RequestImportFile(); }
void CMatchTableForm::OnImportTextActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData ) {GUCEF_TRACE; RequestImportText(); }
void CMatchTableForm::OnCloseActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData ) {GUCEF_TRACE; RequestClose(); }

void
CMatchTableForm::OnNavigation( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( GUCEF_NULL == eventData )
        return;
    GUI::CNavigationEventData* navigation = static_cast< GUI::CNavigationEventData* >( eventData );
    if ( GUI::NAVIGATION_CANCEL == navigation->direction && GUI::EVENTPHASE_TUNNEL != navigation->phase && m_open )
    {
        navigation->handled = true;
        RequestClose();
    }
}

const CORE::CString&
CMatchTableForm::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::GAMEGUI::CMatchTableForm";
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
