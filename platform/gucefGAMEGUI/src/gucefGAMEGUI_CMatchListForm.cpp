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

#ifndef GUCEF_CORE_CINOTIFIER_H
#include "gucefCORE_CINotifier.h"
#define GUCEF_CORE_CINOTIFIER_H
#endif /* GUCEF_CORE_CINOTIFIER_H ? */

#ifndef GUCEF_CORE_CLONEABLES_H
#include "cloneables.h"
#define GUCEF_CORE_CLONEABLES_H
#endif /* GUCEF_CORE_CLONEABLES_H ? */

#ifndef GUCEF_CORE_CFUTURERESULT_H
#include "gucefCORE_CFutureResult.h"
#define GUCEF_CORE_CFUTURERESULT_H
#endif /* GUCEF_CORE_CFUTURERESULT_H ? */

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

#ifndef GUCEF_GUI_CSCROLLPANEL_H
#include "gucefGUI_CScrollPanel.h"
#define GUCEF_GUI_CSCROLLPANEL_H
#endif /* GUCEF_GUI_CSCROLLPANEL_H ? */

#ifndef GUCEF_MULTIPLAY_CMATCHID_H
#include "gucefMULTIPLAY_CMatchId.h"
#define GUCEF_MULTIPLAY_CMATCHID_H
#endif /* GUCEF_MULTIPLAY_CMATCHID_H ? */

#ifndef GUCEF_GAMEGUI_UTILS_H
#include "gucefGAMEGUI_Utils.h"
#define GUCEF_GAMEGUI_UTILS_H
#endif /* GUCEF_GAMEGUI_UTILS_H ? */

#include "gucefGAMEGUI_CMatchListForm.h"

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

const CORE::CEvent CMatchListForm::OpenRequestedEvent = "GUCEF::GAMEGUI::CMatchListForm::OpenRequestedEvent";
const CORE::CEvent CMatchListForm::NewGameRequestedEvent = "GUCEF::GAMEGUI::CMatchListForm::NewGameRequestedEvent";
const CORE::CEvent CMatchListForm::InvitationsRequestedEvent = "GUCEF::GAMEGUI::CMatchListForm::InvitationsRequestedEvent";
const CORE::CEvent CMatchListForm::ExtraRequestedEvent = "GUCEF::GAMEGUI::CMatchListForm::ExtraRequestedEvent";
const CORE::CEvent CMatchListForm::MatchActionEvent = "GUCEF::GAMEGUI::CMatchListForm::MatchActionEvent";

const char* const CMatchListForm::ItemOpen = "Open";
const char* const CMatchListForm::ItemMute = "Mute";
const char* const CMatchListForm::ItemUnmute = "Unmute";
const char* const CMatchListForm::ItemRename = "Rename";
const char* const CMatchListForm::ItemArchive = "Archive";
const char* const CMatchListForm::ItemUnarchive = "Unarchive";
const char* const CMatchListForm::ItemRemove = "Remove";
const char* const CMatchListForm::ItemNudge = "Nudge";
const char* const CMatchListForm::ItemResign = "Resign";
const char* const CMatchListForm::ItemRematch = "Rematch";

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

const Int32 ListMargin = 12;
const Int32 FooterHeight = 112;
const Int32 HeaderHeight = 36;
const Int32 RowGap = 2;
const Int32 DefaultRowHeight = 60;
const Int32 TextLeft = 78;
const Int32 TitleRightReserve = 238;
const Int32 SubtitleRightReserve = 306;
const Int32 MaxProblemLength = 256;
const UInt32 TimesIntervalMs = 1000;
const UInt32 HeaderSlotFlag = 0x80000000;

const char* const RowColour = "30,33,41,255";
const char* const SelectedRowColour = "52,92,160,255";
const char* const NormalTextColour = "226,229,236,255";
const char* const DimTextColour = "150,156,170,255";
const char* const OverdueColour = "255,110,110,255";
const char* const PinnedColour = "232,190,90,255";
const char* const ProblemFill = "190,40,40,255";
const char* const AccentFill = "74,136,232,255";

const char* const GroupNames[ 4 ] = { "Your Turn", "Their Turn", "Invitations", "Finished" };

/** "scale,pixels scale,pixels" for a position or a size */
CORE::CString
UDimText( const char* x, Int32 xPixels, const char* y, Int32 yPixels )
{
    return CORE::CString( x ) + "," + CORE::Int32ToString( xPixels ) + " " + y + "," + CORE::Int32ToString( yPixels );
}

/** The text with control characters (a line break in a name that came from somewhere else) as blanks; control characters are single bytes in UTF-8 */
CORE::CString
CleanText( const CORE::CString& text )
{
    const char* source = text.C_String();
    bool hasControl = false;
    for ( const char* p=source; 0 != *p && !hasControl; ++p )
        hasControl = (unsigned char) *p < 0x20 || 0x7F == (unsigned char) *p;
    if ( !hasControl )
        return text;
    GUCEF::vector< char > bytes( source, source + strlen( source ) + 1 );
    for ( size_t i=0; i+1<bytes.size(); ++i )
        if ( (unsigned char) bytes[ i ] < 0x20 || 0x7F == (unsigned char) bytes[ i ] )
            bytes[ i ] = ' ';
    return CORE::CString( &bytes[ 0 ] );
}

bool
IsActiveGroup( MULTIPLAY::TMatchGroup group )
{
    return MULTIPLAY::MATCHGROUP_YOUR_TURN == group || MULTIPLAY::MATCHGROUP_THEIR_TURN == group;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CMatchActionEventData::CMatchActionEventData( void )
    : CORE::CICloneable()
    , matchId()
    , action( MATCHACTION_MUTE )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMatchActionEventData::CMatchActionEventData( const CORE::CString& id, TMatchAction matchAction )
    : CORE::CICloneable()
    , matchId( id )
    , action( matchAction )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMatchActionEventData::CMatchActionEventData( const CMatchActionEventData& src )
    : CORE::CICloneable( src )
    , matchId( src.matchId )
    , action( src.action )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMatchActionEventData::~CMatchActionEventData()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMatchActionEventData&
CMatchActionEventData::operator=( const CMatchActionEventData& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        matchId = src.matchId;
        action = src.action;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CMatchActionEventData::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CMatchActionEventData( *this );
}

/*-------------------------------------------------------------------------*/

const char*
CMatchActionEventData::GetCloneableTypeName( void ) const
{GUCEF_TRACE;

    return CloneableTypeName();
}

/*-------------------------------------------------------------------------*/

const char*
CMatchActionEventData::CloneableTypeName( void )
{GUCEF_TRACE;

    return "GUCEF::GAMEGUI::CMatchActionEventData";
}

/*-------------------------------------------------------------------------*/

CMatchListForm::SRowInfo::SRowInfo( void )
    : matchId()
    , group( MULTIPLAY::MATCHGROUP_YOUR_TURN )
    , title()
    , subtitle()
    , timeLeft()
    , unseen( false )
    , muted( false )
    , problem( false )
    , pinned( false )
    , isHeader( false )
    , headerText()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMatchListForm::SRow::SRow( void )
    : info()
    , archived( false )
    , active( false )
    , deadline( 0 )
    , ordinal( 0 )
    , slot( -1 )
    , top( 0 )
    , height( 0 )
    , hasProblemText( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMatchListForm::SMatchSlot::SMatchSlot( void )
    : row( GUCEF_NULL ), dot( GUCEF_NULL ), portrait( GUCEF_NULL ), title( GUCEF_NULL ), subtitle( GUCEF_NULL )
    , timeLeft( GUCEF_NULL ), pinned( GUCEF_NULL ), muted( GUCEF_NULL ), problem( GUCEF_NULL ), menu( GUCEF_NULL )
    , matchId(), titleSource(), titleShown(), titleWidth( -1 ), subtitleSource(), subtitleShown(), subtitleWidth( -1 )
    , timeShown(), portraitShown(), tooltipShown()
    , top( -1 ), height( -1 ), shown( false ), selected( false ), unseen( false ), pinnedShown( false ), mutedShown( false )
    , problemShown( false ), archivedShown( false ), overdueShown( false ), used( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMatchListForm::SHeaderSlot::SHeaderSlot( void )
    : row( GUCEF_NULL )
    , text( GUCEF_NULL )
    , textShown()
    , top( -1 )
    , shown( false )
    , used( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CMatchListForm::RegisterEvents( void )
{GUCEF_TRACE;

    OpenRequestedEvent.Initialize();
    NewGameRequestedEvent.Initialize();
    InvitationsRequestedEvent.Initialize();
    ExtraRequestedEvent.Initialize();
    MatchActionEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CMatchListForm::CMatchListForm( void )
    : GUI::CFormEx()
    , m_book( GUCEF_NULL )
    , m_clock( GUCEF_NULL )
    , m_rows()
    , m_matchSlots()
    , m_headerSlots()
    , m_slotOfRowWidget()
    , m_problems()
    , m_portraits()
    , m_selectedMatchId()
    , m_renameMatchId()
    , m_menuMatchId()
    , m_titleFont()
    , m_detailShown()
    , m_menu( GUCEF_NULL )
    , m_list( GUCEF_NULL )
    , m_detail( GUCEF_NULL )
    , m_emptyText( GUCEF_NULL )
    , m_spacer( GUCEF_NULL )
    , m_openButton( GUCEF_NULL )
    , m_invitationsButton( GUCEF_NULL )
    , m_extraButton( GUCEF_NULL )
    , m_extraText()
    , m_detailsProvider()
    , m_detailsText()
    , m_renamePanel( GUCEF_NULL )
    , m_renameEdit( GUCEF_NULL )
    , m_showArchivedBox( GUCEF_NULL )
    , m_rowHeight( DefaultRowHeight )
    , m_titleHeight( 24 )
    , m_subtitleHeight( 22 )
    , m_fittedWidth( -1 )
    , m_shownScroll( -1 )
    , m_shownViewport( -1 )
    , m_sinceTimesMs( 0 )
    , m_showArchived( false )
    , m_open( false )
    , m_updating( false )
    , m_renaming( false )
{GUCEF_TRACE;

    for ( UInt32 i=0; i<5; ++i )
        m_stripWidgets[ i ] = GUCEF_NULL;
    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CMatchListForm::CMatchListForm( GUI::CIGUIContext* context )
    : GUI::CFormEx( context )
    , m_book( GUCEF_NULL )
    , m_clock( GUCEF_NULL )
    , m_rows()
    , m_matchSlots()
    , m_headerSlots()
    , m_slotOfRowWidget()
    , m_problems()
    , m_portraits()
    , m_selectedMatchId()
    , m_renameMatchId()
    , m_menuMatchId()
    , m_titleFont()
    , m_detailShown()
    , m_menu( GUCEF_NULL )
    , m_list( GUCEF_NULL )
    , m_detail( GUCEF_NULL )
    , m_emptyText( GUCEF_NULL )
    , m_spacer( GUCEF_NULL )
    , m_openButton( GUCEF_NULL )
    , m_invitationsButton( GUCEF_NULL )
    , m_extraButton( GUCEF_NULL )
    , m_extraText()
    , m_detailsProvider()
    , m_detailsText()
    , m_renamePanel( GUCEF_NULL )
    , m_renameEdit( GUCEF_NULL )
    , m_showArchivedBox( GUCEF_NULL )
    , m_rowHeight( DefaultRowHeight )
    , m_titleHeight( 24 )
    , m_subtitleHeight( 22 )
    , m_fittedWidth( -1 )
    , m_shownScroll( -1 )
    , m_shownViewport( -1 )
    , m_sinceTimesMs( 0 )
    , m_showArchived( false )
    , m_open( false )
    , m_updating( false )
    , m_renaming( false )
{GUCEF_TRACE;

    for ( UInt32 i=0; i<5; ++i )
        m_stripWidgets[ i ] = GUCEF_NULL;
    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CMatchListForm::~CMatchListForm()
{GUCEF_TRACE;

    UnsubscribeAllFromObserver();
    GUI::CGUIContext* context = GetGUIContext();
    if ( GUCEF_NULL != m_menu && GUCEF_NULL != context )
        context->DestroyWidget( m_menu );
    m_menu = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

void
CMatchListForm::BuildLayout( CORE::CDataNode& layout )
{GUCEF_TRACE;

    const Int32 listTop = WindowContentTop;

    // nine tenths of the screen but no wider than a list needs, from below a strip an application keeps along the top
    CORE::CDataNode* window = GUI::CFormLayout::AddWidgetNode( layout, "Window", "MatchListForm", "0.5,0 0,56", "0.9,0 1,-72" );
    GUI::CFormLayout::SetWidgetNodeProperty( *window, GUI::EWidgetProperty_Title, "My Games" );
    GUI::CFormLayout::SetWidgetNodeProperty( *window, GUI::EWidgetProperty_HAlign, "Center" );
    GUI::CFormLayout::SetWidgetNodeProperty( *window, GUI::EWidgetProperty_MinSize, "0,700 0,420" );
    GUI::CFormLayout::SetWidgetNodeProperty( *window, GUI::EWidgetProperty_MaxSize, "0,960 0,0" );
    GUI::CFormLayout::SetWidgetNodeProperty( *window, GUI::EWidgetProperty_Clip, "true" );

    // the rows scroll
    CORE::CDataNode* list = GUI::CFormLayout::AddWidgetNode( *window, "ScrollPanel", "List", UDimText( "0", ListMargin, "0", listTop ), UDimText( "1", -2 * ListMargin, "1", -( listTop + FooterHeight + 8 ) ) );
    GUI::CFormLayout::SetWidgetNodeProperty( *list, GUI::EWidgetProperty_BackgroundColour, "20,22,28,255" );
    GUI::CFormLayout::SetWidgetNodeProperty( *list, GUI::EWidgetProperty_ScrollStep, CORE::Int32ToString( DefaultRowHeight + RowGap ) );
    GUI::CFormLayout::SetWidgetNodeProperty( *list, GUI::EWidgetProperty_BottomPadding, "0" );
    CORE::CDataNode* spacer = GUI::CFormLayout::AddWidgetNode( *list, "Panel", "Spacer", "0,0 0,0", "1,0 0,1" );
    GUI::CFormLayout::SetWidgetNodeProperty( *spacer, GUI::EWidgetProperty_BackgroundColour, "0,0,0,0" );
    GUI::CFormLayout::SetWidgetNodeProperty( *spacer, GUI::EWidgetProperty_BorderWidth, "0" );
    GUI::CFormLayout::SetWidgetNodeProperty( *spacer, GUI::EWidgetProperty_InputTransparent, "true" );
    GUI::CFormLayout::SetWidgetNodeProperty( *spacer, GUI::EWidgetProperty_Visible, "false" );
    CORE::CDataNode* empty = GUI::CFormLayout::AddWidgetNode( *list, "Label", "EmptyText", "0,16 0,16", "1,-32 0,44" );
    GUI::CFormLayout::SetWidgetNodeProperty( *empty, GUI::EWidgetProperty_Text, "No games yet. Start a new game to begin." );
    GUI::CFormLayout::SetWidgetNodeProperty( *empty, GUI::EWidgetProperty_WordWrap, "true" );
    GUI::CFormLayout::SetWidgetNodeProperty( *empty, GUI::EWidgetProperty_TextColour, DimTextColour );
    GUI::CFormLayout::SetWidgetNodeProperty( *empty, GUI::EWidgetProperty_InputTransparent, "true" );

    // the strip along the bottom edge: the detail of the selected game and the buttons, or the panel for the name of a game
    CORE::CDataNode* footer = GUI::CFormLayout::AddWidgetNode( *window, "Panel", "ButtonRow", UDimText( "0", 1, "1", -( FooterHeight + 1 ) ), UDimText( "1", -2, "0", FooterHeight ) );
    GUI::CFormLayout::SetWidgetNodeProperty( *footer, GUI::EWidgetProperty_BackgroundColour, "18,20,26,255" );
    CORE::CDataNode* detail = GUI::CFormLayout::AddWidgetNode( *footer, "Label", "Detail", "0,16 0,8", "1,-216 0,44" );
    GUI::CFormLayout::SetWidgetNodeProperty( *detail, GUI::EWidgetProperty_VTextAlign, "Center" );
    GUI::CFormLayout::SetWidgetNodeProperty( *detail, GUI::EWidgetProperty_TextColour, "170,175,190,255" );
    GUI::CFormLayout::SetWidgetNodeProperty( *detail, GUI::EWidgetProperty_InputTransparent, "true" );
    CORE::CDataNode* archived = GUI::CFormLayout::AddWidgetNode( *footer, "Checkbox", "ShowArchived", "1,-192 0,8", "0,176 0,44" );
    GUI::CFormLayout::SetWidgetNodeProperty( *archived, GUI::EWidgetProperty_Text, "Show archived" );
    CORE::CDataNode* open = GUI::CFormLayout::AddWidgetNode( *footer, "Button", "Open", "0,16 0,60", "0,120 0,44" );
    GUI::CFormLayout::SetWidgetNodeProperty( *open, GUI::EWidgetProperty_Text, "Open" );
    GUI::CFormLayout::SetWidgetNodeProperty( *open, GUI::EWidgetProperty_StyleClass, "Primary" );
    CORE::CDataNode* newGame = GUI::CFormLayout::AddWidgetNode( *footer, "Button", "NewGame", "0,144 0,60", "0,150 0,44" );
    GUI::CFormLayout::SetWidgetNodeProperty( *newGame, GUI::EWidgetProperty_Text, "New Game" );
    CORE::CDataNode* invitations = GUI::CFormLayout::AddWidgetNode( *footer, "Button", "Invitations", "0,302 0,60", "0,220 0,44" );
    GUI::CFormLayout::SetWidgetNodeProperty( *invitations, GUI::EWidgetProperty_Text, "Invitations" );
    CORE::CDataNode* extra = GUI::CFormLayout::AddWidgetNode( *footer, "Button", "Extra", "1,-216 0,60", "0,200 0,44" );
    GUI::CFormLayout::SetWidgetNodeProperty( *extra, GUI::EWidgetProperty_Text, "" );
    GUI::CFormLayout::SetWidgetNodeProperty( *extra, GUI::EWidgetProperty_Visible, "false" );

    CORE::CDataNode* rename = GUI::CFormLayout::AddWidgetNode( *footer, "Panel", "RenamePanel", "0,0 0,0", "1,0 1,0" );
    GUI::CFormLayout::SetWidgetNodeProperty( *rename, GUI::EWidgetProperty_BackgroundColour, "18,20,26,255" );
    GUI::CFormLayout::SetWidgetNodeProperty( *rename, GUI::EWidgetProperty_Visible, "false" );
    CORE::CDataNode* renameLabel = GUI::CFormLayout::AddWidgetNode( *rename, "Label", "RenameLabel", "0,16 0,8", "1,-32 0,44" );
    GUI::CFormLayout::SetWidgetNodeProperty( *renameLabel, GUI::EWidgetProperty_Text, "Name of the game (only you see it)" );
    GUI::CFormLayout::SetWidgetNodeProperty( *renameLabel, GUI::EWidgetProperty_VTextAlign, "Center" );
    CORE::CDataNode* renameEdit = GUI::CFormLayout::AddWidgetNode( *rename, "Editbox", "RenameEdit", "0,16 0,60", "1,-248 0,44" );
    GUI::CFormLayout::SetWidgetNodeProperty( *renameEdit, GUI::EWidgetProperty_MaxLength, "64" );
    CORE::CDataNode* renameOk = GUI::CFormLayout::AddWidgetNode( *rename, "Button", "RenameOk", "1,-224 0,60", "0,100 0,44" );
    GUI::CFormLayout::SetWidgetNodeProperty( *renameOk, GUI::EWidgetProperty_Text, "OK" );
    GUI::CFormLayout::SetWidgetNodeProperty( *renameOk, GUI::EWidgetProperty_StyleClass, "Primary" );
    CORE::CDataNode* renameCancel = GUI::CFormLayout::AddWidgetNode( *rename, "Button", "RenameCancel", "1,-116 0,60", "0,100 0,44" );
    GUI::CFormLayout::SetWidgetNodeProperty( *renameCancel, GUI::EWidgetProperty_Text, "Cancel" );
}

/*-------------------------------------------------------------------------*/

bool
CMatchListForm::Build( void )
{GUCEF_TRACE;

    CORE::CDataNode layout;
    GUI::CFormLayout::InitLayout( layout );
    BuildLayout( layout );
    if ( !LoadLayout( layout ) )
        return false;
    Hide();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMatchListForm::Open( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == GetRootWidget() && !Build() )
        return false;
    m_open = true;
    m_sinceTimesMs = 0;
    if ( !Show() )
        return false;

    // the width of the list is what the names are cut to: it is there once the window has been laid out
    GUI::CGUIContext* context = GetGUIContext();
    if ( GUCEF_NULL != context )
        context->FlushChanges();
    UpdateRows();
    return true;
}

/*-------------------------------------------------------------------------*/

void
CMatchListForm::Close( void )
{GUCEF_TRACE;

    m_open = false;
    CancelRename();
    CloseMenu();
    Hide();
}

/*-------------------------------------------------------------------------*/

bool
CMatchListForm::IsOpen( void ) const
{GUCEF_TRACE;

    return m_open;
}

/*-------------------------------------------------------------------------*/

void
CMatchListForm::OnPostLayoutLoad( void )
{GUCEF_TRACE;

    GUI::CFormEx::OnPostLayoutLoad();

    // the widgets of a layout that was there before are gone
    m_matchSlots.clear();
    m_headerSlots.clear();
    m_slotOfRowWidget.clear();
    m_fittedWidth = -1;

    m_list = GetWidget( "List" );
    m_detail = GetWidget( "Detail" );
    m_emptyText = GetWidget( "EmptyText" );
    m_spacer = GetWidget( "Spacer" );
    m_shownScroll = -1;
    m_shownViewport = -1;
    m_openButton = GetWidget( "Open" );
    m_invitationsButton = GetWidget( "Invitations" );
    m_extraButton = GetWidget( "Extra" );
    if ( GUCEF_NULL != m_extraButton && !m_extraText.IsNULLOrEmpty() )
    {
        m_extraButton->SetProperty( GUI::EWidgetProperty_Text, m_extraText );
        m_extraButton->SetVisibility( true );
    }
    m_renamePanel = GetWidget( "RenamePanel" );
    m_renameEdit = GetWidget( "RenameEdit" );
    m_showArchivedBox = GetWidget( "ShowArchived" );
    m_stripWidgets[ 0 ] = m_detail;
    m_stripWidgets[ 1 ] = m_showArchivedBox;
    m_stripWidgets[ 2 ] = m_openButton;
    m_stripWidgets[ 3 ] = GetWidget( "NewGame" );
    m_stripWidgets[ 4 ] = m_invitationsButton;

    GUI::CGUIContext* context = GetGUIContext();
    if ( GUCEF_NULL != context )
    {
        TEventCallback callback( this, &CMatchListForm::OnFrameStarted );
        SubscribeTo( context, GUI::CGUIContext::FrameStartedEvent, callback );
    }

    GUI::CWidget* root = GetRootWidget();
    if ( GUCEF_NULL != root )
    {
        TEventCallback callback( this, &CMatchListForm::OnKeyDown );
        SubscribeTo( root, GUI::CWidget::KeyDownEvent, callback );
    }
    if ( GUCEF_NULL != m_list )
    {
        TEventCallback clickCallback( this, &CMatchListForm::OnListClicked );
        SubscribeTo( m_list, GUI::CWidget::MouseClickEvent, clickCallback );
        TEventCallback doubleCallback( this, &CMatchListForm::OnListDoubleClicked );
        SubscribeTo( m_list, GUI::CWidget::MouseDoubleClickEvent, doubleCallback );
        TEventCallback activatedCallback( this, &CMatchListForm::OnListActivated );
        SubscribeTo( m_list, GUI::CWidget::ActivatedEvent, activatedCallback );
    }
    if ( GUCEF_NULL != m_openButton )
    {
        TEventCallback callback( this, &CMatchListForm::OnOpenActivated );
        SubscribeTo( m_openButton, GUI::CWidget::ActivatedEvent, callback );
    }
    GUI::CWidget* button = GetWidget( "NewGame" );
    if ( GUCEF_NULL != button )
    {
        TEventCallback callback( this, &CMatchListForm::OnNewGameActivated );
        SubscribeTo( button, GUI::CWidget::ActivatedEvent, callback );
    }
    if ( GUCEF_NULL != m_invitationsButton )
    {
        TEventCallback callback( this, &CMatchListForm::OnInvitationsActivated );
        SubscribeTo( m_invitationsButton, GUI::CWidget::ActivatedEvent, callback );
    }
    if ( GUCEF_NULL != m_extraButton )
    {
        TEventCallback callback( this, &CMatchListForm::OnExtraActivated );
        SubscribeTo( m_extraButton, GUI::CWidget::ActivatedEvent, callback );
    }
    button = GetWidget( "RenameOk" );
    if ( GUCEF_NULL != button )
    {
        TEventCallback callback( this, &CMatchListForm::OnRenameOkActivated );
        SubscribeTo( button, GUI::CWidget::ActivatedEvent, callback );
    }
    button = GetWidget( "RenameCancel" );
    if ( GUCEF_NULL != button )
    {
        TEventCallback callback( this, &CMatchListForm::OnRenameCancelActivated );
        SubscribeTo( button, GUI::CWidget::ActivatedEvent, callback );
    }
    if ( GUCEF_NULL != m_showArchivedBox )
    {
        TEventCallback callback( this, &CMatchListForm::OnShowArchivedChanged );
        SubscribeTo( m_showArchivedBox, GUI::CWidget::ValueChangedEvent, callback );
        m_showArchivedBox->SetBoolProperty( GUI::EWidgetProperty_Checked, m_showArchived );
    }
    UpdateRows();
}

/*-------------------------------------------------------------------------*/

void
CMatchListForm::SetBook( MULTIPLAY::CMatchBook* book )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_book )
        UnsubscribeFrom( m_book );
    m_book = book;
    if ( GUCEF_NULL != m_book )
    {
        TEventCallback changedCallback( this, &CMatchListForm::OnBookChanged );
        SubscribeTo( m_book, MULTIPLAY::CMatchBook::MatchBookChangedEvent, changedCallback );
        TEventCallback turnCallback( this, &CMatchListForm::OnYourTurn );
        SubscribeTo( m_book, MULTIPLAY::CMatchBook::YourTurnEvent, turnCallback );
    }
    UpdateRows();
}

/*-------------------------------------------------------------------------*/

MULTIPLAY::CMatchBook*
CMatchListForm::GetBook( void ) const
{GUCEF_TRACE;

    return m_book;
}

/*-------------------------------------------------------------------------*/

void
CMatchListForm::SetClock( const MULTIPLAY::CIMatchClock* clock )
{GUCEF_TRACE;

    m_clock = clock;
    UpdateRows();
}

/*-------------------------------------------------------------------------*/

Int64
CMatchListForm::GetNowMs( void ) const
{GUCEF_TRACE;

    return GUCEF_NULL != m_clock ? m_clock->GetNowMs() : MULTIPLAY::CMatchSystemClock::Instance().GetNowMs();
}

/*-------------------------------------------------------------------------*/

void
CMatchListForm::SetShowArchived( bool showArchived )
{GUCEF_TRACE;

    m_showArchived = showArchived;
    if ( GUCEF_NULL != m_showArchivedBox )
        m_showArchivedBox->SetBoolProperty( GUI::EWidgetProperty_Checked, showArchived );
    UpdateRows();
}

/*-------------------------------------------------------------------------*/

bool
CMatchListForm::GetShowArchived( void ) const
{GUCEF_TRACE;

    return m_showArchived;
}

/*-------------------------------------------------------------------------*/

void
CMatchListForm::SetTitleFont( const CORE::CString& fontName )
{GUCEF_TRACE;

    m_titleFont = fontName;
    UpdateRows();
}

/*-------------------------------------------------------------------------*/

void
CMatchListForm::SetMatchProblem( const CORE::CString& matchId, const CORE::CString& problemText )
{GUCEF_TRACE;

    if ( problemText.IsNULLOrEmpty() )
        m_problems.erase( matchId );
    else
        m_problems[ matchId ] = CleanText( problemText.Length() > (UInt32) MaxProblemLength ? problemText.SubstrToIndex( (UInt32) MaxProblemLength ) : problemText );
    UpdateRows();
}

/*-------------------------------------------------------------------------*/

void
CMatchListForm::SetMatchPortrait( const CORE::CString& matchId, const CORE::CString& imageName )
{GUCEF_TRACE;

    if ( imageName.IsNULLOrEmpty() )
        m_portraits.erase( matchId );
    else
        m_portraits[ matchId ] = imageName;
    UpdateRows();
}

/*-------------------------------------------------------------------------*/

CORE::CString
CMatchListForm::MakeSubtitle( const MULTIPLAY::CMatchEntry& entry ) const
{GUCEF_TRACE;

    CORE::CString names;
    if ( entry.hasInfo )
    {
        CORE::CString::StringSet::const_iterator i = entry.preview.currentUserIds.begin();
        while ( i != entry.preview.currentUserIds.end() )
        {
            const MULTIPLAY::CMatchParticipant* participant = entry.info.FindParticipant( *i );
            if ( GUCEF_NULL != participant && !participant->displayName.IsNULLOrEmpty() )
            {
                if ( !names.IsNULLOrEmpty() )
                    names += ", ";
                names += CleanText( participant->displayName );
            }
            ++i;
        }
    }
    if ( !names.IsNULLOrEmpty() )
        return "Turn " + CORE::UInt32ToString( entry.preview.turn ) + " - " + names;
    if ( !entry.preview.summary.IsNULLOrEmpty() )
        return CleanText( entry.preview.summary );
    return "Turn " + CORE::UInt32ToString( entry.preview.turn );
}

/*-------------------------------------------------------------------------*/

void
CMatchListForm::MakeMatchRow( const MULTIPLAY::CMatchEntry& entry, MULTIPLAY::TMatchGroup group, SRow& row ) const
{GUCEF_TRACE;

    row.info.isHeader = false;
    row.info.matchId = entry.preview.matchId;
    row.info.group = group;
    const CORE::CString& displayTitle = entry.GetDisplayTitle();
    row.info.title = displayTitle.IsNULLOrEmpty() ? CORE::CString( "Game " + MULTIPLAY::CMatchId::Short( entry.preview.matchId ) ) : CleanText( displayTitle );
    row.info.subtitle = MakeSubtitle( entry );
    row.info.unseen = entry.unseen;
    row.info.muted = entry.muted;
    row.info.pinned = entry.pinned;
    row.archived = entry.archived;
    TTextMap::const_iterator problem = m_problems.find( entry.preview.matchId );
    row.hasProblemText = problem != m_problems.end();
    row.info.problem = row.hasProblemText;
    row.active = IsActiveGroup( group );
    row.deadline = entry.preview.deadline;
    if ( row.active && row.deadline > 0 )
        row.info.timeLeft = FormatTimeLeft( row.deadline - GetNowMs() );
}

/*-------------------------------------------------------------------------*/

void
CMatchListForm::BuildModel( TRowVector& rows ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_book )
        return;

    static const MULTIPLAY::TMatchGroup order[ 4 ] = { MULTIPLAY::MATCHGROUP_YOUR_TURN, MULTIPLAY::MATCHGROUP_THEIR_TURN,
                                                       MULTIPLAY::MATCHGROUP_FORMING_OR_INVITED, MULTIPLAY::MATCHGROUP_FINISHED };
    for ( UInt32 g=0; g<4; ++g )
    {
        MULTIPLAY::TMatchEntryVector entries;
        m_book->GetGroup( order[ g ], entries, m_showArchived );
        if ( entries.empty() )
            continue;

        SRow header;
        header.info.isHeader = true;
        header.info.group = order[ g ];
        header.info.headerText = CORE::CString( GroupNames[ g ] ) + " (" + CORE::UInt32ToString( (UInt32) entries.size() ) + ")";
        rows.push_back( header );
        for ( size_t e=0; e<entries.size(); ++e )
        {
            SRow row;
            MakeMatchRow( entries[ e ], order[ g ], row );
            rows.push_back( row );
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CMatchListForm::UpdateRows( void )
{GUCEF_TRACE;

    if ( m_updating )
        return;
    m_updating = true;

    TRowVector rows;
    BuildModel( rows );
    m_rows.swap( rows );

    // what is selected or being renamed and is gone, is deselected or cancelled
    if ( !m_selectedMatchId.IsNULLOrEmpty() && FindMatchRow( m_selectedMatchId ) < 0 )
        m_selectedMatchId.Clear();
    if ( m_renaming && FindMatchRow( m_renameMatchId ) < 0 )
        CancelRename();

    ShowRows();
    m_updating = false;
}

/*-------------------------------------------------------------------------*/

void
CMatchListForm::ComputeRowMetrics( void )
{GUCEF_TRACE;

    GUI::CGUIContext* context = GetGUIContext();
    if ( GUCEF_NULL == context || GUCEF_NULL == m_detail )
        return;
    const CORE::CString themeFont = m_detail->GetProperty( GUI::EWidgetProperty_Font );
    const bool ownTitleFont = !m_titleFont.IsNULLOrEmpty() && !context->GetBitmapFont( m_titleFont ).IsNULL();
    m_titleHeight = context->GetLineHeight( ownTitleFont ? m_titleFont : themeFont ) + 4;
    m_subtitleHeight = context->GetLineHeight( themeFont ) + 2;
    m_rowHeight = m_titleHeight + m_subtitleHeight + 14;
    if ( m_rowHeight < DefaultRowHeight )
        m_rowHeight = DefaultRowHeight;
}

/*-------------------------------------------------------------------------*/

bool
CMatchListForm::EnsureMatchSlots( UInt32 count )
{GUCEF_TRACE;

    GUI::CGUIContext* context = GetGUIContext();
    if ( GUCEF_NULL == context || GUCEF_NULL == m_list )
        return false;

    while ( m_matchSlots.size() < count )
    {
        const UInt32 k = (UInt32) m_matchSlots.size();
        const CORE::CString suffix = CORE::UInt32ToString( k );
        SMatchSlot slot;
        slot.row = context->CreateWidget( "Panel", "Row" + suffix, m_list );
        if ( GUCEF_NULL == slot.row )
            return false;
        slot.row->SetVisibility( false );
        slot.row->SetBoolProperty( GUI::EWidgetProperty_Clip, true );
        slot.row->SetProperty( GUI::EWidgetProperty_BackgroundColour, RowColour );
        slot.row->SetProperty( GUI::EWidgetProperty_BorderWidth, "0" );

        slot.dot = context->CreateWidget( "Label", "Dot" + suffix, slot.row );
        slot.portrait = context->CreateWidget( "ImageFrame", "Portrait" + suffix, slot.row );
        slot.title = context->CreateWidget( "Label", "Title" + suffix, slot.row );
        slot.subtitle = context->CreateWidget( "Label", "Subtitle" + suffix, slot.row );
        slot.timeLeft = context->CreateWidget( "Label", "TimeLeft" + suffix, slot.row );
        slot.pinned = context->CreateWidget( "Label", "Pinned" + suffix, slot.row );
        slot.muted = context->CreateWidget( "Label", "Muted" + suffix, slot.row );
        slot.problem = context->CreateWidget( "Label", "Problem" + suffix, slot.row );
        slot.menu = context->CreateWidget( "Button", "Menu" + suffix, slot.row );
        if ( GUCEF_NULL == slot.dot || GUCEF_NULL == slot.portrait || GUCEF_NULL == slot.title || GUCEF_NULL == slot.subtitle ||
             GUCEF_NULL == slot.timeLeft || GUCEF_NULL == slot.pinned || GUCEF_NULL == slot.muted || GUCEF_NULL == slot.problem || GUCEF_NULL == slot.menu )
            return false;

        // the labels let the pointer through to the row, so a click anywhere on a row is a click on the row
        GUI::CWidget* labels[ 8 ] = { slot.dot, slot.portrait, slot.title, slot.subtitle, slot.timeLeft, slot.pinned, slot.muted, slot.problem };
        for ( UInt32 i=0; i<8; ++i )
            labels[ i ]->SetBoolProperty( GUI::EWidgetProperty_InputTransparent, true );

        slot.dot->SetProperty( GUI::EWidgetProperty_BackgroundColour, AccentFill );
        slot.portrait->SetProperty( GUI::EWidgetProperty_ScaleMode, "Fit" );
        slot.timeLeft->SetProperty( GUI::EWidgetProperty_HTextAlign, "Right" );
        slot.timeLeft->SetProperty( GUI::EWidgetProperty_TextColour, NormalTextColour );
        slot.subtitle->SetProperty( GUI::EWidgetProperty_TextColour, DimTextColour );
        slot.pinned->SetProperty( GUI::EWidgetProperty_Text, "Pinned" );
        slot.pinned->SetProperty( GUI::EWidgetProperty_TextColour, PinnedColour );
        slot.pinned->SetProperty( GUI::EWidgetProperty_HTextAlign, "Center" );
        slot.muted->SetProperty( GUI::EWidgetProperty_Text, "Muted" );
        slot.muted->SetProperty( GUI::EWidgetProperty_TextColour, DimTextColour );
        slot.muted->SetProperty( GUI::EWidgetProperty_HTextAlign, "Center" );
        slot.problem->SetProperty( GUI::EWidgetProperty_Text, "!" );
        slot.problem->SetProperty( GUI::EWidgetProperty_BackgroundColour, ProblemFill );
        slot.problem->SetProperty( GUI::EWidgetProperty_HTextAlign, "Center" );
        slot.menu->SetProperty( GUI::EWidgetProperty_Text, "..." );
        slot.menu->SetProperty( GUI::EWidgetProperty_Tooltip, "More" );
        slot.dot->SetVisibility( false );
        slot.portrait->SetVisibility( false );
        slot.pinned->SetVisibility( false );
        slot.muted->SetVisibility( false );
        slot.problem->SetVisibility( false );

        m_slotOfRowWidget[ slot.row ] = k;
        m_matchSlots.push_back( slot );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMatchListForm::EnsureHeaderSlots( UInt32 count )
{GUCEF_TRACE;

    GUI::CGUIContext* context = GetGUIContext();
    if ( GUCEF_NULL == context || GUCEF_NULL == m_list )
        return false;

    while ( m_headerSlots.size() < count )
    {
        const UInt32 k = (UInt32) m_headerSlots.size();
        const CORE::CString suffix = CORE::UInt32ToString( k );
        SHeaderSlot slot;
        slot.row = context->CreateWidget( "Panel", "Header" + suffix, m_list );
        if ( GUCEF_NULL == slot.row )
            return false;
        slot.row->SetVisibility( false );
        slot.row->SetProperty( GUI::EWidgetProperty_BackgroundColour, "24,27,34,255" );
        slot.row->SetProperty( GUI::EWidgetProperty_BorderWidth, "0" );
        slot.row->SetProperty( GUI::EWidgetProperty_Size, UDimText( "1", 0, "0", HeaderHeight ) );
        slot.text = context->CreateWidget( "Label", "HeaderText" + suffix, slot.row );
        if ( GUCEF_NULL == slot.text )
            return false;
        slot.text->SetProperty( GUI::EWidgetProperty_Position, "0,12 0,0" );
        slot.text->SetProperty( GUI::EWidgetProperty_Size, UDimText( "1", -24, "1", 0 ) );
        slot.text->SetProperty( GUI::EWidgetProperty_VTextAlign, "Center" );
        slot.text->SetProperty( GUI::EWidgetProperty_TextColour, "170,175,190,255" );
        slot.text->SetBoolProperty( GUI::EWidgetProperty_InputTransparent, true );
        m_slotOfRowWidget[ slot.row ] = k | HeaderSlotFlag;
        m_headerSlots.push_back( slot );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

Int32
CMatchListForm::GetListWidth( void ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_list && m_list->GetRect().width > 0 )
    {
        const GUI::CScrollPanel* panel = m_list->AsScrollPanel();
        return GUCEF_NULL != panel ? panel->GetViewportRect().width : m_list->GetRect().width;
    }

    // not laid out yet: what the layout of the window will make of the screen
    const GUI::CGUIContext* context = const_cast< CMatchListForm* >( this )->GetGUIContext();
    Int32 screenWidth = GUCEF_NULL != context ? (Int32) context->GetLogicalWidth() : 0;
    if ( screenWidth <= 0 )
        screenWidth = 800;
    Int32 windowWidth = screenWidth * 9 / 10;
    if ( windowWidth > 960 )
        windowWidth = 960;
    if ( windowWidth < 700 )
        windowWidth = 700;
    return windowWidth - 2 * ListMargin - 16;
}

/*-------------------------------------------------------------------------*/

void
CMatchListForm::SetLabelText( GUI::CWidget* label, const CORE::CString& text )
{GUCEF_TRACE;

    if ( GUCEF_NULL != label && label->GetPropertyRef( GUI::EWidgetProperty_Text ) != text )
        label->SetProperty( GUI::EWidgetProperty_Text, text );
}

/*-------------------------------------------------------------------------*/

void
CMatchListForm::SetFlagWidget( GUI::CWidget* widget, bool shown )
{GUCEF_TRACE;

    if ( GUCEF_NULL != widget && widget->IsVisible() != shown )
        widget->SetVisibility( shown );
}

/*-------------------------------------------------------------------------*/

void
CMatchListForm::FitMatchTexts( SMatchSlot& slot, const SRow& row, bool force )
{GUCEF_TRACE;

    GUI::CGUIContext* context = GetGUIContext();
    if ( GUCEF_NULL == context )
        return;
    const Int32 listWidth = GetListWidth();

    Int32 width = listWidth - TitleRightReserve;
    if ( width < 0 )
        width = 0;
    if ( force || slot.titleWidth != width || slot.titleSource != row.info.title )
    {
        slot.titleWidth = width;
        slot.titleSource = row.info.title;
        slot.titleShown = EllipsizeText( *context, slot.title->GetProperty( GUI::EWidgetProperty_Font ), row.info.title, width );
        SetLabelText( slot.title, slot.titleShown );
    }

    width = listWidth - SubtitleRightReserve;
    if ( width < 0 )
        width = 0;
    if ( force || slot.subtitleWidth != width || slot.subtitleSource != row.info.subtitle )
    {
        slot.subtitleWidth = width;
        slot.subtitleSource = row.info.subtitle;
        slot.subtitleShown = EllipsizeText( *context, slot.subtitle->GetProperty( GUI::EWidgetProperty_Font ), row.info.subtitle, width );
        SetLabelText( slot.subtitle, slot.subtitleShown );
    }
}

/*-------------------------------------------------------------------------*/

void
CMatchListForm::ShowMatchTimeLeft( SMatchSlot& slot, const SRow& row )
{GUCEF_TRACE;

    if ( slot.timeShown != row.info.timeLeft )
    {
        slot.timeShown = row.info.timeLeft;
        SetLabelText( slot.timeLeft, slot.timeShown );
    }
    const bool overdue = "Overdue" == row.info.timeLeft;
    if ( overdue != slot.overdueShown )
    {
        slot.overdueShown = overdue;
        slot.timeLeft->SetProperty( GUI::EWidgetProperty_TextColour, overdue ? OverdueColour : NormalTextColour );
    }
}

/*-------------------------------------------------------------------------*/

void
CMatchListForm::ShowMatchRow( const SRow& row )
{GUCEF_TRACE;

    GUI::CGUIContext* context = GetGUIContext();
    SMatchSlot& slot = m_matchSlots[ (size_t) row.slot ];
    if ( !slot.shown )
    {
        slot.shown = true;
        slot.row->SetVisibility( true );
    }
    slot.matchId = row.info.matchId;

    if ( slot.top != row.top )
    {
        slot.top = row.top;
        slot.row->SetProperty( GUI::EWidgetProperty_Position, UDimText( "0", 0, "0", row.top ) );
    }
    if ( slot.height != row.height )
    {
        // the places of the parts of the row, from the left and from the right edge
        slot.height = row.height;
        const Int32 subtitleTop = 6 + m_titleHeight + 2;
        const CORE::CString centred = CORE::Int32ToString( ( row.height - 44 ) / 2 );
        slot.row->SetProperty( GUI::EWidgetProperty_Size, UDimText( "1", 0, "0", row.height ) );
        slot.dot->SetProperty( GUI::EWidgetProperty_Position, "0,8 0," + CORE::Int32ToString( ( row.height - 10 ) / 2 ) );
        slot.dot->SetProperty( GUI::EWidgetProperty_Size, "0,10 0,10" );
        slot.portrait->SetProperty( GUI::EWidgetProperty_Position, "0,24 0," + centred );
        slot.portrait->SetProperty( GUI::EWidgetProperty_Size, "0,44 0,44" );
        slot.title->SetProperty( GUI::EWidgetProperty_Position, UDimText( "0", TextLeft, "0", 6 ) );
        slot.title->SetProperty( GUI::EWidgetProperty_Size, UDimText( "1", -TitleRightReserve, "0", m_titleHeight ) );
        slot.subtitle->SetProperty( GUI::EWidgetProperty_Position, UDimText( "0", TextLeft, "0", subtitleTop ) );
        slot.subtitle->SetProperty( GUI::EWidgetProperty_Size, UDimText( "1", -SubtitleRightReserve, "0", m_subtitleHeight ) );
        slot.timeLeft->SetProperty( GUI::EWidgetProperty_Position, UDimText( "1", -152, "0", 6 ) );
        slot.timeLeft->SetProperty( GUI::EWidgetProperty_Size, UDimText( "0", 92, "0", m_titleHeight ) );
        slot.pinned->SetProperty( GUI::EWidgetProperty_Position, UDimText( "1", -220, "0", subtitleTop ) );
        slot.pinned->SetProperty( GUI::EWidgetProperty_Size, UDimText( "0", 64, "0", m_subtitleHeight ) );
        slot.muted->SetProperty( GUI::EWidgetProperty_Position, UDimText( "1", -152, "0", subtitleTop ) );
        slot.muted->SetProperty( GUI::EWidgetProperty_Size, UDimText( "0", 60, "0", m_subtitleHeight ) );
        slot.problem->SetProperty( GUI::EWidgetProperty_Position, UDimText( "1", -88, "0", subtitleTop ) );
        slot.problem->SetProperty( GUI::EWidgetProperty_Size, UDimText( "0", 28, "0", m_subtitleHeight ) );
        slot.menu->SetProperty( GUI::EWidgetProperty_Position, "1,-52 0," + centred );
        slot.menu->SetProperty( GUI::EWidgetProperty_Size, "0,44 0,44" );
        slot.titleWidth = -1;
        slot.subtitleWidth = -1;
    }

    // the font of the name, when the application has one
    const CORE::CString themeFont = m_detail->GetProperty( GUI::EWidgetProperty_Font );
    const bool ownTitleFont = !m_titleFont.IsNULLOrEmpty() && !context->GetBitmapFont( m_titleFont ).IsNULL();
    const CORE::CString wantedFont = ownTitleFont ? m_titleFont : themeFont;
    if ( slot.title->GetPropertyRef( GUI::EWidgetProperty_Font ) != wantedFont )
    {
        slot.title->SetProperty( GUI::EWidgetProperty_Font, wantedFont );
        slot.timeLeft->SetProperty( GUI::EWidgetProperty_Font, wantedFont );
        slot.titleWidth = -1;
    }

    FitMatchTexts( slot, row, false );
    ShowMatchTimeLeft( slot, row );

    // the marks
    if ( slot.unseen != row.info.unseen )
    {
        slot.unseen = row.info.unseen;
        SetFlagWidget( slot.dot, slot.unseen );
    }
    if ( slot.pinnedShown != row.info.pinned )
    {
        slot.pinnedShown = row.info.pinned;
        SetFlagWidget( slot.pinned, slot.pinnedShown );
    }
    if ( slot.mutedShown != row.info.muted )
    {
        slot.mutedShown = row.info.muted;
        SetFlagWidget( slot.muted, slot.mutedShown );
    }
    if ( slot.problemShown != row.info.problem )
    {
        slot.problemShown = row.info.problem;
        SetFlagWidget( slot.problem, slot.problemShown );
    }
    if ( slot.archivedShown != row.archived )
    {
        slot.archivedShown = row.archived;
        slot.title->SetProperty( GUI::EWidgetProperty_TextColour, row.archived ? DimTextColour : NormalTextColour );
    }

    // the problem is the tooltip of the row
    CORE::CString tooltip;
    if ( row.hasProblemText )
    {
        TTextMap::const_iterator problem = m_problems.find( row.info.matchId );
        if ( problem != m_problems.end() )
            tooltip = (*problem).second;
    }
    if ( slot.tooltipShown != tooltip )
    {
        slot.tooltipShown = tooltip;
        slot.row->SetProperty( GUI::EWidgetProperty_Tooltip, tooltip );
    }

    // the portrait: an image the context knows
    CORE::CString image;
    TTextMap::const_iterator portrait = m_portraits.find( row.info.matchId );
    if ( portrait != m_portraits.end() && context->HasImage( (*portrait).second ) )
        image = (*portrait).second;
    if ( slot.portraitShown != image )
    {
        slot.portraitShown = image;
        slot.portrait->SetProperty( GUI::EWidgetProperty_Image, image );
        SetFlagWidget( slot.portrait, !image.IsNULLOrEmpty() );
    }
}

/*-------------------------------------------------------------------------*/

void
CMatchListForm::ShowHeaderRow( const SRow& row )
{GUCEF_TRACE;

    SHeaderSlot& slot = m_headerSlots[ (size_t) row.slot ];
    if ( !slot.shown )
    {
        slot.shown = true;
        slot.row->SetVisibility( true );
    }
    if ( slot.top != row.top )
    {
        slot.top = row.top;
        slot.row->SetProperty( GUI::EWidgetProperty_Position, UDimText( "0", 0, "0", row.top ) );
    }
    if ( slot.textShown != row.info.headerText )
    {
        slot.textShown = row.info.headerText;
        SetLabelText( slot.text, slot.textShown );
    }
}

/*-------------------------------------------------------------------------*/

void
CMatchListForm::ShowRows( void )
{GUCEF_TRACE;

    // the model is there without the window; the widgets follow when the window is
    if ( GUCEF_NULL == m_list || GUCEF_NULL == GetGUIContext() )
        return;

    ShowWindow();
    ShowEmptyText();
    ShowButtons();
    ShowDetail();
}

/*-------------------------------------------------------------------------*/

void
CMatchListForm::ShowWindow( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_list || GUCEF_NULL == GetGUIContext() )
        return;

    ComputeRowMetrics();

    // where every row is in the list; the list is as high as the rows, a spacer at its end makes it so
    Int32 top = 0;
    Int32 matchOrdinal = 0;
    Int32 headerOrdinal = 0;
    for ( size_t i=0; i<m_rows.size(); ++i )
    {
        SRow& row = m_rows[ i ];
        row.top = top;
        row.slot = -1;
        row.height = row.info.isHeader ? HeaderHeight : m_rowHeight;
        row.ordinal = row.info.isHeader ? headerOrdinal++ : matchOrdinal++;
        top += row.height + RowGap;
    }
    const Int32 contentHeight = m_rows.empty() ? 0 : m_rows[ m_rows.size() - 1 ].top + m_rows[ m_rows.size() - 1 ].height;

    // only the rows in view and a little around them have widgets: many matches cost no more than a screenful
    const GUI::CScrollPanel* panel = m_list->AsScrollPanel();
    const Int32 scroll = GUCEF_NULL != panel ? panel->GetScrollPosition() : 0;
    Int32 viewport = GUCEF_NULL != panel ? panel->GetViewportHeight() : 0;
    if ( viewport <= 0 )
        viewport = 600;
    const Int32 margin = 2 * ( m_rowHeight + RowGap );
    const Int32 from = scroll - margin;
    const Int32 to = scroll + viewport + margin;
    size_t first = m_rows.size();
    size_t last = 0;
    UInt32 matchesInWindow = 0;
    UInt32 headersInWindow = 0;
    for ( size_t i=0; i<m_rows.size(); ++i )
    {
        const SRow& row = m_rows[ i ];
        if ( row.top > to )
            break;
        if ( row.top + row.height < from )
            continue;
        if ( first > i )
            first = i;
        last = i;
        if ( row.info.isHeader )
            ++headersInWindow;
        else
            ++matchesInWindow;
    }

    const UInt32 matchPool = matchesInWindow > (UInt32) m_matchSlots.size() ? matchesInWindow : (UInt32) m_matchSlots.size();
    const UInt32 headerPool = headersInWindow > (UInt32) m_headerSlots.size() ? headersInWindow : (UInt32) m_headerSlots.size();
    if ( !EnsureMatchSlots( matchPool ) || !EnsureHeaderSlots( headerPool ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CMatchListForm: the widgets for the rows could not be made" );
        return;
    }

    for ( size_t k=0; k<m_matchSlots.size(); ++k )
        m_matchSlots[ k ].used = false;
    for ( size_t k=0; k<m_headerSlots.size(); ++k )
        m_headerSlots[ k ].used = false;

    // a row takes the slot of its place among the rows of its kind, so that scrolling by a row changes one slot and not all of them
    for ( size_t i=first; i<=last && i<m_rows.size(); ++i )
    {
        SRow& row = m_rows[ i ];
        if ( row.info.isHeader )
        {
            row.slot = row.ordinal % (Int32) m_headerSlots.size();
            m_headerSlots[ (size_t) row.slot ].used = true;
            ShowHeaderRow( row );
        }
        else
        {
            row.slot = row.ordinal % (Int32) m_matchSlots.size();
            m_matchSlots[ (size_t) row.slot ].used = true;
            ShowMatchRow( row );
        }
    }

    // the slots that are not needed
    for ( size_t k=0; k<m_matchSlots.size(); ++k )
    {
        if ( !m_matchSlots[ k ].used && m_matchSlots[ k ].shown )
        {
            m_matchSlots[ k ].shown = false;
            m_matchSlots[ k ].matchId.Clear();
            m_matchSlots[ k ].row->SetVisibility( false );
        }
    }
    for ( size_t k=0; k<m_headerSlots.size(); ++k )
    {
        if ( !m_headerSlots[ k ].used && m_headerSlots[ k ].shown )
        {
            m_headerSlots[ k ].shown = false;
            m_headerSlots[ k ].row->SetVisibility( false );
        }
    }

    if ( GUCEF_NULL != m_spacer )
    {
        SetFlagWidget( m_spacer, contentHeight > 0 );
        if ( contentHeight > 0 )
            m_spacer->SetProperty( GUI::EWidgetProperty_Position, UDimText( "0", 0, "0", contentHeight - 1 ) );
    }

    m_fittedWidth = GetListWidth();
    m_shownScroll = scroll;
    m_shownViewport = viewport;
    ShowSelection();
}

/*-------------------------------------------------------------------------*/

void
CMatchListForm::ShowEmptyText( void )
{GUCEF_TRACE;

    SetFlagWidget( m_emptyText, m_rows.empty() );
}

/*-------------------------------------------------------------------------*/

void
CMatchListForm::ShowSelection( void )
{GUCEF_TRACE;

    for ( size_t i=0; i<m_rows.size(); ++i )
    {
        const SRow& row = m_rows[ i ];
        if ( row.info.isHeader || row.slot < 0 || (size_t) row.slot >= m_matchSlots.size() )
            continue;
        SMatchSlot& slot = m_matchSlots[ (size_t) row.slot ];
        const bool selected = !m_selectedMatchId.IsNULLOrEmpty() && row.info.matchId == m_selectedMatchId;
        if ( selected != slot.selected )
        {
            slot.selected = selected;
            slot.row->SetProperty( GUI::EWidgetProperty_BackgroundColour, selected ? SelectedRowColour : RowColour );
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CMatchListForm::ShowButtons( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_openButton )
    {
        const bool canOpen = !m_selectedMatchId.IsNULLOrEmpty();
        if ( m_openButton->IsEnabled() != canOpen )
            m_openButton->SetEnabled( canOpen );
    }
    if ( GUCEF_NULL != m_invitationsButton )
    {
        UInt32 invitations = 0;
        for ( size_t i=0; i<m_rows.size(); ++i )
            if ( !m_rows[ i ].info.isHeader && MULTIPLAY::MATCHGROUP_FORMING_OR_INVITED == m_rows[ i ].info.group )
                ++invitations;
        SetLabelText( m_invitationsButton, 0 == invitations ? CORE::CString( "Invitations" ) : "Invitations (" + CORE::UInt32ToString( invitations ) + ")" );
    }
}

/*-------------------------------------------------------------------------*/

void
CMatchListForm::ShowDetail( void )
{GUCEF_TRACE;

    GUI::CGUIContext* context = GetGUIContext();
    if ( GUCEF_NULL == m_detail || GUCEF_NULL == context )
        return;

    // the problem of the selected game, else its whole name
    CORE::CString detail;
    bool problem = false;
    const Int32 index = m_selectedMatchId.IsNULLOrEmpty() ? -1 : FindMatchRow( m_selectedMatchId );
    if ( index >= 0 )
    {
        TTextMap::const_iterator found = m_problems.find( m_selectedMatchId );
        if ( found != m_problems.end() )
        {
            detail = (*found).second;
            problem = true;
        }
        else
            detail = m_rows[ (size_t) index ].info.title;
    }
    Int32 width = m_detail->GetRect().width;
    if ( width <= 0 )
        width = GetListWidth() - 200;
    SetLabelText( m_detail, EllipsizeText( *context, m_detail->GetProperty( GUI::EWidgetProperty_Font ), detail, width ) );
    m_detail->SetProperty( GUI::EWidgetProperty_TextColour, problem ? OverdueColour : "170,175,190,255" );
}

/*-------------------------------------------------------------------------*/

void
CMatchListForm::ShowTimes( void )
{GUCEF_TRACE;

    const Int64 now = GetNowMs();
    for ( size_t i=0; i<m_rows.size(); ++i )
    {
        SRow& row = m_rows[ i ];
        if ( row.info.isHeader || !row.active || row.deadline <= 0 )
            continue;
        const CORE::CString timeLeft = FormatTimeLeft( row.deadline - now );
        if ( timeLeft != row.info.timeLeft )
        {
            row.info.timeLeft = timeLeft;
            if ( row.slot >= 0 && (size_t) row.slot < m_matchSlots.size() && m_matchSlots[ (size_t) row.slot ].shown )
                ShowMatchTimeLeft( m_matchSlots[ (size_t) row.slot ], row );
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CMatchListForm::FitTexts( bool force )
{GUCEF_TRACE;

    for ( size_t i=0; i<m_rows.size(); ++i )
    {
        const SRow& row = m_rows[ i ];
        if ( !row.info.isHeader && row.slot >= 0 && (size_t) row.slot < m_matchSlots.size() && m_matchSlots[ (size_t) row.slot ].shown )
            FitMatchTexts( m_matchSlots[ (size_t) row.slot ], row, force );
    }
    ShowDetail();
}

/*-------------------------------------------------------------------------*/

void
CMatchListForm::Update( UInt32 deltaInMs )
{GUCEF_TRACE;

    if ( !m_open || GUCEF_NULL == m_list )
        return;

    // the list scrolled: the rows in view are the ones that have widgets
    const GUI::CScrollPanel* panel = m_list->AsScrollPanel();
    if ( GUCEF_NULL != panel && ( panel->GetScrollPosition() != m_shownScroll || panel->GetViewportHeight() != m_shownViewport ) )
        ShowWindow();

    // the window changed size: the names are cut to the new width
    const Int32 width = GetListWidth();
    if ( width != m_fittedWidth )
    {
        m_fittedWidth = width;
        FitTexts( false );
    }

    m_sinceTimesMs += deltaInMs;
    if ( m_sinceTimesMs >= TimesIntervalMs )
    {
        m_sinceTimesMs = 0;
        ShowTimes();
    }
}

/*-------------------------------------------------------------------------*/

UInt32
CMatchListForm::GetRowCount( void ) const
{GUCEF_TRACE;

    return (UInt32) m_rows.size();
}

/*-------------------------------------------------------------------------*/

bool
CMatchListForm::GetRow( UInt32 index, SRowInfo& row ) const
{GUCEF_TRACE;

    if ( index >= m_rows.size() )
        return false;
    row = m_rows[ index ].info;
    return true;
}

/*-------------------------------------------------------------------------*/

Int32
CMatchListForm::FindMatchRow( const CORE::CString& matchId ) const
{GUCEF_TRACE;

    if ( matchId.IsNULLOrEmpty() )
        return -1;
    for ( size_t i=0; i<m_rows.size(); ++i )
        if ( !m_rows[ i ].info.isHeader && m_rows[ i ].info.matchId == matchId )
            return (Int32) i;
    return -1;
}

/*-------------------------------------------------------------------------*/

Int32
CMatchListForm::FindRow( const CORE::CString& matchId ) const
{GUCEF_TRACE;

    return FindMatchRow( matchId );
}

/*-------------------------------------------------------------------------*/

Int32
CMatchListForm::FindRowOfWidget( GUI::CWidget* widget ) const
{GUCEF_TRACE;

    // the row panel is the child of the list
    while ( GUCEF_NULL != widget && widget->GetParentWidget() != m_list )
        widget = widget->GetParentWidget();
    if ( GUCEF_NULL == widget )
        return -1;
    TWidgetSlotMap::const_iterator found = m_slotOfRowWidget.find( widget );
    if ( found == m_slotOfRowWidget.end() )
        return -1;
    const bool header = 0 != ( (*found).second & HeaderSlotFlag );
    const Int32 slot = (Int32) ( (*found).second & ~HeaderSlotFlag );
    for ( size_t i=0; i<m_rows.size(); ++i )
        if ( m_rows[ i ].info.isHeader == header && m_rows[ i ].slot == slot )
            return (Int32) i;
    return -1;
}

/*-------------------------------------------------------------------------*/

void
CMatchListForm::ScrollRowIntoView( Int32 index )
{GUCEF_TRACE;

    GUI::CScrollPanel* panel = GUCEF_NULL != m_list ? m_list->AsScrollPanel() : GUCEF_NULL;
    GUI::CGUIContext* context = GetGUIContext();
    if ( GUCEF_NULL == panel || GUCEF_NULL == context || index < 0 || (size_t) index >= m_rows.size() )
        return;

    // the height of the list and of its view are what the last layout measured
    context->FlushChanges();

    // the first game of a group is shown with the header of its group
    Int32 top = m_rows[ (size_t) index ].top;
    const Int32 bottom = top + m_rows[ (size_t) index ].height;
    if ( index > 0 && m_rows[ (size_t) index - 1 ].info.isHeader )
        top = m_rows[ (size_t) index - 1 ].top;

    const Int32 position = panel->GetScrollPosition();
    const Int32 viewport = panel->GetViewportHeight();
    if ( top < position )
        panel->SetScrollPosition( top );
    else
    if ( bottom > position + viewport )
        panel->SetScrollPosition( bottom - viewport );
    ShowWindow();
}

/*-------------------------------------------------------------------------*/

bool
CMatchListForm::SelectIndex( Int32 index )
{GUCEF_TRACE;

    if ( index < 0 || (size_t) index >= m_rows.size() || m_rows[ (size_t) index ].info.isHeader )
        return false;
    m_selectedMatchId = m_rows[ (size_t) index ].info.matchId;
    ShowSelection();
    ShowButtons();
    ShowDetail();
    ScrollRowIntoView( index );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMatchListForm::SelectRow( const CORE::CString& matchId )
{GUCEF_TRACE;

    return SelectIndex( FindMatchRow( matchId ) );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CMatchListForm::GetSelectedMatchId( void ) const
{GUCEF_TRACE;

    return m_selectedMatchId;
}

/*-------------------------------------------------------------------------*/

bool
CMatchListForm::OpenSelected( void )
{GUCEF_TRACE;

    if ( FindMatchRow( m_selectedMatchId ) < 0 )
        return false;
    TOpenRequestedEventData data( m_selectedMatchId );
    NotifyObservers( OpenRequestedEvent, &data );
    return true;
}

/*-------------------------------------------------------------------------*/

GUI::CWidget*
CMatchListForm::GetRowWidget( const CORE::CString& matchId, const CORE::CString& part )
{GUCEF_TRACE;

    const Int32 index = FindMatchRow( matchId );
    if ( index < 0 )
        return GUCEF_NULL;
    const Int32 k = m_rows[ (size_t) index ].slot;
    if ( k < 0 || (size_t) k >= m_matchSlots.size() )
        return GUCEF_NULL;
    const SMatchSlot& slot = m_matchSlots[ (size_t) k ];
    if ( "Row" == part ) return slot.row;
    if ( "Dot" == part ) return slot.dot;
    if ( "Portrait" == part ) return slot.portrait;
    if ( "Title" == part ) return slot.title;
    if ( "Subtitle" == part ) return slot.subtitle;
    if ( "TimeLeft" == part ) return slot.timeLeft;
    if ( "Pinned" == part ) return slot.pinned;
    if ( "Muted" == part ) return slot.muted;
    if ( "Problem" == part ) return slot.problem;
    if ( "Menu" == part ) return slot.menu;
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

UInt32
CMatchListForm::GetVisibleRowCount( void ) const
{GUCEF_TRACE;

    const GUI::CScrollPanel* panel = GUCEF_NULL != m_list ? m_list->AsScrollPanel() : GUCEF_NULL;
    const Int32 viewport = GUCEF_NULL != panel ? panel->GetViewportHeight() : 0;
    const Int32 count = viewport / ( m_rowHeight + RowGap );
    return count < 1 ? 1 : (UInt32) count;
}

/*-------------------------------------------------------------------------*/

void
CMatchListForm::MoveSelection( Int32 direction, UInt32 amount )
{GUCEF_TRACE;

    if ( m_rows.empty() )
        return;
    const Int32 index = m_selectedMatchId.IsNULLOrEmpty() ? -1 : FindMatchRow( m_selectedMatchId );
    const Int32 count = (Int32) m_rows.size();

    // from nothing the first game is the one to go to; a step goes to the next game in the direction, headers are not stopped at
    Int32 target = index;
    if ( index < 0 )
    {
        target = direction < 0 ? count : -1;
        if ( 0 == amount )
            return;
    }
    for ( UInt32 step=0; step<amount; ++step )
    {
        Int32 next = target + direction;
        while ( next >= 0 && next < count && m_rows[ (size_t) next ].info.isHeader )
            next += direction;
        if ( next < 0 || next >= count )
            break;
        target = next;
    }
    if ( target >= 0 && target < count && target != index )
        SelectIndex( target );
}

/*-------------------------------------------------------------------------*/

void
CMatchListForm::SelectEdge( bool first )
{GUCEF_TRACE;

    const Int32 count = (Int32) m_rows.size();
    for ( Int32 i=0; i<count; ++i )
    {
        const Int32 index = first ? i : count - 1 - i;
        if ( !m_rows[ (size_t) index ].info.isHeader )
        {
            SelectIndex( index );
            return;
        }
    }
}

/*-------------------------------------------------------------------------*/

bool
CMatchListForm::GetNudgeTarget( const MULTIPLAY::CMatchEntry& entry, CORE::CString& userId ) const
{GUCEF_TRACE;

    const CORE::CString& local = GUCEF_NULL != m_book ? m_book->GetLocalUserId() : CORE::CString::Empty;
    CORE::CString::StringSet::const_iterator i = entry.preview.currentUserIds.begin();
    while ( i != entry.preview.currentUserIds.end() )
    {
        if ( *i != local )
        {
            userId = *i;
            return true;
        }
        ++i;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CMatchListForm::GetRowMenuItems( const CORE::CString& matchId, GUCEF::vector< CORE::CString >& items ) const
{GUCEF_TRACE;

    items.clear();
    MULTIPLAY::CMatchEntry entry;
    if ( GUCEF_NULL == m_book || FindMatchRow( matchId ) < 0 || !m_book->GetEntry( matchId, entry ) )
        return false;

    const MULTIPLAY::TMatchGroup group = m_book->GetGroupOf( entry );
    const bool finished = MULTIPLAY::MATCHGROUP_FINISHED == group;
    const bool active = IsActiveGroup( group ) && MULTIPLAY::MATCHSTATUS_ACTIVE == entry.preview.status;
    MULTIPLAY::CIMatchProviderPtr provider = m_book->GetProviderOfMatch( matchId );
    const bool canNudge = !provider.IsNULL() && provider->HasCapability( MULTIPLAY::MATCHCAPABILITY_NUDGE );
    const bool canResign = !provider.IsNULL() && provider->HasCapability( MULTIPLAY::MATCHCAPABILITY_RESIGN );
    const bool canRematch = !provider.IsNULL() && provider->HasCapability( MULTIPLAY::MATCHCAPABILITY_REMATCH );
    CORE::CString nudgeTarget;

    items.push_back( ItemOpen );
    if ( !m_detailsText.IsNULLOrEmpty() && entry.providerName == m_detailsProvider )
        items.push_back( m_detailsText );
    items.push_back( entry.muted ? ItemUnmute : ItemMute );
    if ( canNudge && MULTIPLAY::MATCHGROUP_THEIR_TURN == group && GetNudgeTarget( entry, nudgeTarget ) )
        items.push_back( ItemNudge );
    items.push_back( ItemRename );
    if ( canResign && active )
        items.push_back( ItemResign );
    if ( canRematch && finished )
        items.push_back( ItemRematch );
    if ( finished )
        items.push_back( ItemRemove );
    else
        items.push_back( entry.archived ? ItemUnarchive : ItemArchive );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CMatchListForm::Notify( const CORE::CString& matchId, TMatchAction action )
{GUCEF_TRACE;

    TMatchActionEventData data( matchId, action );
    NotifyObservers( MatchActionEvent, &data );
}

/*-------------------------------------------------------------------------*/

bool
CMatchListForm::PerformRowAction( const CORE::CString& matchId, const CORE::CString& item )
{GUCEF_TRACE;

    GUCEF::vector< CORE::CString > items;
    if ( !GetRowMenuItems( matchId, items ) )
        return false;
    bool known = false;
    for ( size_t i=0; i<items.size() && !known; ++i )
        known = items[ i ] == item;
    if ( !known )
        return false;

    MULTIPLAY::CMatchEntry entry;
    if ( !m_book->GetEntry( matchId, entry ) )
        return false;

    if ( item == ItemOpen )
    {
        SelectRow( matchId );
        return OpenSelected();
    }
    if ( item == ItemMute || item == ItemUnmute )
    {
        const bool mute = item == ItemMute;
        if ( !m_book->SetMuted( matchId, mute ) )
            return false;
        Notify( matchId, mute ? MATCHACTION_MUTE : MATCHACTION_UNMUTE );
        return true;
    }
    if ( item == ItemRename )
        return BeginRename( matchId );
    if ( item == ItemArchive || item == ItemUnarchive )
    {
        const bool archive = item == ItemArchive;
        if ( !m_book->SetArchived( matchId, archive ) )
            return false;
        Notify( matchId, archive ? MATCHACTION_ARCHIVE : MATCHACTION_UNARCHIVE );
        return true;
    }
    if ( item == ItemRemove )
    {
        if ( !m_book->RemoveMatch( matchId ) )
            return false;
        Notify( matchId, MATCHACTION_REMOVE );
        return true;
    }

    if ( !m_detailsText.IsNULLOrEmpty() && item == m_detailsText )
    {
        Notify( matchId, MATCHACTION_DETAILS );
        return true;
    }

    // what the provider of the match does; the answer comes through the book, the future is not waited for
    MULTIPLAY::CIMatchProviderPtr provider = m_book->GetProviderOfMatch( matchId );
    if ( provider.IsNULL() )
        return false;
    CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
    if ( item == ItemNudge )
    {
        CORE::CString userId;
        if ( !GetNudgeTarget( entry, userId ) || !provider->Nudge( matchId, userId, future ) )
            return false;
        Notify( matchId, MATCHACTION_NUDGE );
        return true;
    }
    if ( item == ItemResign )
    {
        if ( !provider->Resign( matchId, future ) )
            return false;
        Notify( matchId, MATCHACTION_RESIGN );
        return true;
    }
    if ( item == ItemRematch )
    {
        if ( !provider->Rematch( matchId, future ) )
            return false;
        Notify( matchId, MATCHACTION_REMATCH );
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

void
CMatchListForm::CloseMenu( void )
{GUCEF_TRACE;

    GUI::CGUIContext* context = GetGUIContext();
    if ( GUCEF_NULL != context && GUCEF_NULL != m_menu && context->GetPopup() == m_menu )
        context->ClosePopup();
}

/*-------------------------------------------------------------------------*/

void
CMatchListForm::ShowMenuAt( const CORE::CString& matchId, Int32 x, Int32 y )
{GUCEF_TRACE;

    GUI::CGUIContext* context = GetGUIContext();
    GUCEF::vector< CORE::CString > names;
    if ( GUCEF_NULL == context || !GetRowMenuItems( matchId, names ) || names.empty() )
        return;

    if ( GUCEF_NULL == m_menu )
    {
        m_menu = static_cast< GUI::CPopupMenu* >( context->CreateWidget( "PopupMenu", "MatchListForm#Menu", GUCEF_NULL ) );
        if ( GUCEF_NULL == m_menu )
            return;
        m_menu->SetProperty( GUI::EWidgetProperty_RowHeight, CORE::Int32ToString( MinTouchHeight ) );
        TEventCallback callback( this, &CMatchListForm::OnMenuItemActivated );
        SubscribeTo( m_menu, GUI::CWidget::ItemActivatedEvent, callback );
    }
    GUI::CPopupMenu::TStringVector items;
    for ( size_t i=0; i<names.size(); ++i )
        items.push_back( names[ i ] );
    m_menuMatchId = matchId;
    m_menu->SetItems( items );
    m_menu->ShowAt( x, y );
}

/*-------------------------------------------------------------------------*/

bool
CMatchListForm::OpenRowMenu( const CORE::CString& matchId, Int32 x, Int32 y )
{GUCEF_TRACE;

    GUCEF::vector< CORE::CString > items;
    if ( !GetRowMenuItems( matchId, items ) || items.empty() )
        return false;
    ShowMenuAt( matchId, x, y );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMatchListForm::BeginRename( const CORE::CString& matchId )
{GUCEF_TRACE;

    MULTIPLAY::CMatchEntry entry;
    if ( GUCEF_NULL == m_book || FindMatchRow( matchId ) < 0 || !m_book->GetEntry( matchId, entry ) || GUCEF_NULL == m_renamePanel )
        return false;

    m_renaming = true;
    m_renameMatchId = matchId;
    m_renameEdit->SetProperty( GUI::EWidgetProperty_Text, entry.localTitle.IsNULLOrEmpty() ? entry.GetDisplayTitle() : entry.localTitle );
    for ( UInt32 i=0; i<5; ++i )
        SetFlagWidget( m_stripWidgets[ i ], false );
    SetFlagWidget( m_renamePanel, true );
    GUI::CGUIContext* context = GetGUIContext();
    if ( GUCEF_NULL != context )
        context->SetFocus( m_renameEdit );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMatchListForm::IsRenaming( void ) const
{GUCEF_TRACE;

    return m_renaming;
}

/*-------------------------------------------------------------------------*/

void
CMatchListForm::CancelRename( void )
{GUCEF_TRACE;

    if ( !m_renaming )
        return;
    m_renaming = false;
    m_renameMatchId.Clear();
    SetFlagWidget( m_renamePanel, false );
    for ( UInt32 i=0; i<5; ++i )
        SetFlagWidget( m_stripWidgets[ i ], true );
}

/*-------------------------------------------------------------------------*/

bool
CMatchListForm::CommitRename( void )
{GUCEF_TRACE;

    if ( !m_renaming || GUCEF_NULL == m_book )
        return false;
    const CORE::CString matchId = m_renameMatchId;
    const bool renamed = m_book->SetLocalTitle( matchId, m_renameEdit->GetProperty( GUI::EWidgetProperty_Text ) );
    CancelRename();
    if ( renamed )
        Notify( matchId, MATCHACTION_RENAME );
    return renamed;
}

/*-------------------------------------------------------------------------*/

void
CMatchListForm::OnBookChanged( CORE::CNotifier* notifier    ,
                               const CORE::CEvent& eventId  ,
                               CORE::CICloneable* eventData )
{GUCEF_TRACE;

    UpdateRows();
}

/*-------------------------------------------------------------------------*/

void
CMatchListForm::OnYourTurn( CORE::CNotifier* notifier    ,
                            const CORE::CEvent& eventId  ,
                            CORE::CICloneable* eventData )
{GUCEF_TRACE;

    UpdateRows();
}

/*-------------------------------------------------------------------------*/

void
CMatchListForm::OnFrameStarted( CORE::CNotifier* notifier    ,
                                const CORE::CEvent& eventId  ,
                                CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( GUCEF_NULL != eventData )
        Update( static_cast< CORE::TCloneableUInt32* >( eventData )->GetData() );
}

/*-------------------------------------------------------------------------*/

void
CMatchListForm::OnListClicked( CORE::CNotifier* notifier    ,
                               const CORE::CEvent& eventId  ,
                               CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( GUCEF_NULL == eventData )
        return;
    GUI::CPointerEventData* pointer = static_cast< GUI::CPointerEventData* >( eventData );
    if ( GUI::EVENTPHASE_TUNNEL == pointer->phase )
        return;
    const Int32 index = FindRowOfWidget( pointer->source );
    if ( index < 0 || m_rows[ (size_t) index ].info.isHeader )
        return;
    SelectIndex( index );
    if ( GUI::MOUSEBUTTON_RIGHT == pointer->button )
        ShowMenuAt( m_rows[ (size_t) index ].info.matchId, pointer->x, pointer->y );
}

/*-------------------------------------------------------------------------*/

void
CMatchListForm::OnListDoubleClicked( CORE::CNotifier* notifier    ,
                                     const CORE::CEvent& eventId  ,
                                     CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( GUCEF_NULL == eventData )
        return;
    GUI::CPointerEventData* pointer = static_cast< GUI::CPointerEventData* >( eventData );
    if ( GUI::EVENTPHASE_TUNNEL == pointer->phase || GUI::MOUSEBUTTON_LEFT != pointer->button || GUCEF_NULL == pointer->source )
        return;

    // the button of the row menu is a button, not the row
    if ( pointer->source->GetType() == "Button" )
        return;
    const Int32 index = FindRowOfWidget( pointer->source );
    if ( index >= 0 && !m_rows[ (size_t) index ].info.isHeader && SelectIndex( index ) )
        OpenSelected();
}

/*-------------------------------------------------------------------------*/

void
CMatchListForm::OnListActivated( CORE::CNotifier* notifier    ,
                                 const CORE::CEvent& eventId  ,
                                 CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( GUCEF_NULL == eventData )
        return;
    GUI::CRoutedEventData* routed = static_cast< GUI::CRoutedEventData* >( eventData );
    if ( GUI::EVENTPHASE_TUNNEL == routed->phase || GUCEF_NULL == routed->source )
        return;

    // the "..." button of a row
    const Int32 index = FindRowOfWidget( routed->source );
    if ( index < 0 || m_rows[ (size_t) index ].info.isHeader || routed->source->GetType() != "Button" )
        return;
    const GUI::CRect& rect = routed->source->GetRect();
    ShowMenuAt( m_rows[ (size_t) index ].info.matchId, rect.left, rect.Bottom() );
}

/*-------------------------------------------------------------------------*/

void
CMatchListForm::OnKeyDown( CORE::CNotifier* notifier    ,
                           const CORE::CEvent& eventId  ,
                           CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( GUCEF_NULL == eventData )
        return;
    GUI::CKeyEventData* key = static_cast< GUI::CKeyEventData* >( eventData );
    if ( GUI::EVENTPHASE_TUNNEL == key->phase || key->handled )
        return;
    const bool enter = INPUT::KEYCODE_RETURN == key->keyCode || INPUT::KEYCODE_NUMPADENTER == key->keyCode;

    // the panel for the name of a game has Enter and Escape
    if ( m_renaming )
    {
        if ( enter )
        {
            key->handled = true;
            CommitRename();
        }
        else
        if ( INPUT::KEYCODE_ESCAPE == key->keyCode )
        {
            key->handled = true;
            CancelRename();
        }
        return;
    }

    // the keys of the list are for the list, the window itself and the buttons of the rows; the other buttons and fields keep theirs
    GUI::CWidget* source = key->source;
    GUI::CWidget* root = GetRootWidget();
    bool inList = source == root;
    for ( GUI::CWidget* w=source; GUCEF_NULL != w && !inList; w=w->GetParentWidget() )
        inList = w == m_list;
    if ( !inList )
        return;
    const bool onButton = GUCEF_NULL != source && source->GetType() == "Button";

    switch ( key->keyCode )
    {
        case INPUT::KEYCODE_UP:     MoveSelection( -1, 1 ); break;
        case INPUT::KEYCODE_DOWN:   MoveSelection( 1, 1 ); break;
        case INPUT::KEYCODE_PGUP:   MoveSelection( -1, GetVisibleRowCount() > 1 ? GetVisibleRowCount() - 1 : 1 ); break;
        case INPUT::KEYCODE_PGDOWN: MoveSelection( 1, GetVisibleRowCount() > 1 ? GetVisibleRowCount() - 1 : 1 ); break;
        case INPUT::KEYCODE_HOME:   SelectEdge( true ); break;
        case INPUT::KEYCODE_END:    SelectEdge( false ); break;
        case INPUT::KEYCODE_RETURN:
        case INPUT::KEYCODE_NUMPADENTER:
        {
            if ( onButton || !OpenSelected() )
                return;
            break;
        }
        default: return;
    }
    key->handled = true;
}

/*-------------------------------------------------------------------------*/

void
CMatchListForm::OnOpenActivated( CORE::CNotifier* notifier    ,
                                 const CORE::CEvent& eventId  ,
                                 CORE::CICloneable* eventData )
{GUCEF_TRACE;

    OpenSelected();
}

/*-------------------------------------------------------------------------*/

void
CMatchListForm::OnNewGameActivated( CORE::CNotifier* notifier    ,
                                    const CORE::CEvent& eventId  ,
                                    CORE::CICloneable* eventData )
{GUCEF_TRACE;

    NotifyObservers( NewGameRequestedEvent );
}

/*-------------------------------------------------------------------------*/

void
CMatchListForm::SetDetailsAction( const CORE::CString& providerName, const CORE::CString& text )
{GUCEF_TRACE;

    m_detailsProvider = providerName;
    m_detailsText = text;
}

/*-------------------------------------------------------------------------*/

void
CMatchListForm::SetExtraButton( const CORE::CString& text )
{GUCEF_TRACE;

    m_extraText = text;
    if ( GUCEF_NULL != m_extraButton )
    {
        m_extraButton->SetProperty( GUI::EWidgetProperty_Text, text );
        m_extraButton->SetVisibility( !text.IsNULLOrEmpty() );
    }
}

/*-------------------------------------------------------------------------*/

void
CMatchListForm::OnExtraActivated( CORE::CNotifier* notifier    ,
                                  const CORE::CEvent& eventId  ,
                                  CORE::CICloneable* eventData )
{GUCEF_TRACE;

    NotifyObservers( ExtraRequestedEvent );
}

/*-------------------------------------------------------------------------*/

void
CMatchListForm::OnInvitationsActivated( CORE::CNotifier* notifier    ,
                                        const CORE::CEvent& eventId  ,
                                        CORE::CICloneable* eventData )
{GUCEF_TRACE;

    NotifyObservers( InvitationsRequestedEvent );
}

/*-------------------------------------------------------------------------*/

void
CMatchListForm::OnShowArchivedChanged( CORE::CNotifier* notifier    ,
                                       const CORE::CEvent& eventId  ,
                                       CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_showArchivedBox )
        return;
    const bool showArchived = m_showArchivedBox->GetBoolProperty( GUI::EWidgetProperty_Checked );
    if ( showArchived != m_showArchived )
    {
        m_showArchived = showArchived;
        UpdateRows();
    }
}

/*-------------------------------------------------------------------------*/

void
CMatchListForm::OnRenameOkActivated( CORE::CNotifier* notifier    ,
                                     const CORE::CEvent& eventId  ,
                                     CORE::CICloneable* eventData )
{GUCEF_TRACE;

    CommitRename();
}

/*-------------------------------------------------------------------------*/

void
CMatchListForm::OnRenameCancelActivated( CORE::CNotifier* notifier    ,
                                         const CORE::CEvent& eventId  ,
                                         CORE::CICloneable* eventData )
{GUCEF_TRACE;

    CancelRename();
}

/*-------------------------------------------------------------------------*/

void
CMatchListForm::OnMenuItemActivated( CORE::CNotifier* notifier    ,
                                     const CORE::CEvent& eventId  ,
                                     CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( GUCEF_NULL == eventData )
        return;
    GUI::CItemEventData* item = static_cast< GUI::CItemEventData* >( eventData );
    CloseMenu();
    PerformRowAction( m_menuMatchId, item->item );
}

/*-------------------------------------------------------------------------*/

void
CMatchListForm::OnNotify( CORE::CNotifier* notifier    ,
                          const CORE::CEvent& eventId  ,
                          CORE::CICloneable* eventData )
{GUCEF_TRACE;

    // the standard events do not reach the handlers of the subscriptions
    if ( GUCEF_NULL != m_book && notifier == m_book && CORE::CINotifier::DestructionEvent == eventId )
    {
        m_book = GUCEF_NULL;
        UpdateRows();
    }
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CMatchListForm::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::GAMEGUI::CMatchListForm";
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
