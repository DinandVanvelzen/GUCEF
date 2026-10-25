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

#include <math.h>

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

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

#ifndef GUCEF_GUI_CSPINNER_H
#include "gucefGUI_CSpinner.h"
#define GUCEF_GUI_CSPINNER_H
#endif /* GUCEF_GUI_CSPINNER_H ? */

#ifndef GUCEF_GUI_CGUICONTEXT_H
#include "gucefGUI_CGUIContext.h"
#define GUCEF_GUI_CGUICONTEXT_H
#endif /* GUCEF_GUI_CGUICONTEXT_H ? */

#ifndef GUCEF_GUI_CGUIGLOBAL_H
#include "gucefGUI_CGuiGlobal.h"
#define GUCEF_GUI_CGUIGLOBAL_H
#endif /* GUCEF_GUI_CGUIGLOBAL_H ? */

#ifndef GUCEF_GUI_CWIDGETTYPEREGISTRY_H
#include "gucefGUI_CWidgetTypeRegistry.h"
#define GUCEF_GUI_CWIDGETTYPEREGISTRY_H
#endif /* GUCEF_GUI_CWIDGETTYPEREGISTRY_H ? */

#ifndef GUCEF_GAMEGUI_UTILS_H
#include "gucefGAMEGUI_Utils.h"
#define GUCEF_GAMEGUI_UTILS_H
#endif /* GUCEF_GAMEGUI_UTILS_H ? */

#include "gucefGAMEGUI_CLobbySettingsPanelForm.h"

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

const CORE::CEvent CLobbySettingsPanelForm::SettingChangedEvent = "GUCEF::GAMEGUI::CLobbySettingsPanelForm::SettingChangedEvent";
const CORE::CString CLobbySettingsPanelForm::ProposeSuffix = " (propose)";

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

const Int32 RowHeight = 32;
const Int32 ControlHeight = 32;
const Int32 UnitWidth = 40;
const Int32 GroupPadding = 10;
const Int32 CaptionHeight = 22;
const Int32 ScrollBarWidth = 16;

/** The groups go in columns of at least this width, up to this many */
const Int32 MinColumnWidth = 420;
const Int32 MaxColumns = 3;

/** A group box with its rows, or a setting that is in none, for the columns */
struct SBlock
{
    UInt32 first;
    UInt32 count;
    bool grouped;
    Int32 height;
};

/** The digits after the point a spinner needs to show multiples of the step */
UInt32
CountDecimals( Float64 step )
{
    if ( step <= 0.0 )
        return 1;
    UInt32 decimals = 0;
    Float64 scaled = step;
    while ( decimals < 4 && fabs( scaled - floor( scaled + 0.5 ) ) > 1e-6 )
    {
        scaled *= 10.0;
        ++decimals;
    }
    return decimals;
}

const CORE::CString&
LabelOf( const MULTIPLAY::CLobbySettingDef& def )
{
    return def.label.IsNULLOrEmpty() ? def.key : def.label;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CSettingChangedEventData::CSettingChangedEventData( void )
    : CORE::CICloneable()
    , key()
    , value()
    , proposed( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CSettingChangedEventData::CSettingChangedEventData( const CORE::CString& key_, const CORE::CString& value_, bool proposed_ )
    : CORE::CICloneable()
    , key( key_ )
    , value( value_ )
    , proposed( proposed_ )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CSettingChangedEventData::~CSettingChangedEventData()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CSettingChangedEventData::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CSettingChangedEventData( *this );
}

/*-------------------------------------------------------------------------*/

void
CLobbySettingsPanelForm::RegisterEvents( void )
{GUCEF_TRACE;

    SettingChangedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CLobbySettingsPanelForm::CLobbySettingsPanelForm( void )
    : GUI::CFormEx()
    , m_placeholder( GUCEF_NULL )
    , m_schema()
    , m_values()
    , m_lobby()
    , m_controls()
    , m_hostMakesRoom( false )
    , m_columns( 1 )
    , m_wantedColumns( 1 )
    , m_fittedWidth( -1 )
    , m_subscribedToFrames( false )
    , m_updating( false )
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CLobbySettingsPanelForm::~CLobbySettingsPanelForm()
{GUCEF_TRACE;

    UnsubscribeAllFromObserver();
}

/*-------------------------------------------------------------------------*/

void
CLobbySettingsPanelForm::AddControlNodes( CORE::CDataNode& parent, UInt32 index )
{GUCEF_TRACE;

    const TControl& control = m_controls[ index ];
    const MULTIPLAY::CLobbySettingDef& def = control.def;

    CORE::CDataNode* row = GUI::CFormLayout::AddWidgetNode( parent, "Panel", "Row" + CORE::UInt32ToString( index ), "0,0 0,0", "1,0 0," + CORE::Int32ToString( RowHeight ) );

    if ( MULTIPLAY::CLobbySettingDef::TYPE_BOOL == def.type )
    {
        CORE::CDataNode* box = GUI::CFormLayout::AddWidgetNode( *row, "Checkbox", control.widgetName, "0,0 0,0", "1,0 1,0" );
        GUI::CFormLayout::SetWidgetNodeProperty( *box, GUI::EWidgetProperty_Text, LabelOf( def ) );
        return;
    }

    CORE::CDataNode* label = GUI::CFormLayout::AddWidgetNode( *row, "Label", control.labelName, "0,0 0,0", "0.45,-8 1,0" );
    GUI::CFormLayout::SetWidgetNodeProperty( *label, GUI::EWidgetProperty_Text, LabelOf( def ) );
    GUI::CFormLayout::SetWidgetNodeProperty( *label, GUI::EWidgetProperty_VTextAlign, "Center" );

    // a unit is shown after the value, in the room the control leaves for it; the control keeps its height in a taller row
    const bool hasUnit = !control.unitName.IsNULLOrEmpty();
    const CORE::CString controlSize = ( hasUnit ? "0.55," + CORE::Int32ToString( -UnitWidth ) : CORE::CString( "0.55,0" ) ) + " 0," + CORE::Int32ToString( ControlHeight );
    const CORE::CString controlPosition = hasUnit ? "1," + CORE::Int32ToString( -UnitWidth ) + " 0.5,0" : CORE::CString( "1,0 0.5,0" );

    CORE::CDataNode* controlNode = GUCEF_NULL;
    switch ( def.type )
    {
        case MULTIPLAY::CLobbySettingDef::TYPE_INT:
        case MULTIPLAY::CLobbySettingDef::TYPE_FLOAT:
        {
            CORE::CDataNode* spinner = GUI::CFormLayout::AddWidgetNode( *row, "Spinner", control.widgetName, controlPosition, controlSize );
            bool isInt = MULTIPLAY::CLobbySettingDef::TYPE_INT == def.type;
            Float64 step = def.step > 0.0 ? def.step : ( isInt ? 1.0 : 0.1 );
            GUI::CFormLayout::SetWidgetNodeProperty( *spinner, GUI::EWidgetProperty_Min, CORE::DoubleToString( def.hasRange ? def.minValue : -1000000.0 ) );
            GUI::CFormLayout::SetWidgetNodeProperty( *spinner, GUI::EWidgetProperty_Max, CORE::DoubleToString( def.hasRange ? def.maxValue : 1000000.0 ) );
            GUI::CFormLayout::SetWidgetNodeProperty( *spinner, GUI::EWidgetProperty_Step, CORE::DoubleToString( step ) );
            GUI::CFormLayout::SetWidgetNodeProperty( *spinner, GUI::EWidgetProperty_Decimals, CORE::UInt32ToString( isInt ? 0 : CountDecimals( step ) ) );
            controlNode = spinner;
            break;
        }
        case MULTIPLAY::CLobbySettingDef::TYPE_ENUM:
        {
            CORE::CDataNode* combo = GUI::CFormLayout::AddWidgetNode( *row, "Combobox", control.widgetName, controlPosition, controlSize );
            GUI::CFormLayout::SetWidgetNodeProperty( *combo, GUI::EWidgetProperty_Items, JoinLines( def.options ) );
            controlNode = combo;
            break;
        }
        default:
        {
            controlNode = GUI::CFormLayout::AddWidgetNode( *row, "Editbox", control.widgetName, controlPosition, controlSize );
            break;
        }
    }
    if ( GUCEF_NULL != controlNode )
    {
        // against the right edge of the row (or of the room left for the unit), so rounding cannot push it out
        GUI::CFormLayout::SetWidgetNodeProperty( *controlNode, GUI::EWidgetProperty_HAlign, "Right" );
        GUI::CFormLayout::SetWidgetNodeProperty( *controlNode, GUI::EWidgetProperty_VAlign, "Center" );
    }

    if ( hasUnit )
    {
        CORE::CDataNode* unit = GUI::CFormLayout::AddWidgetNode( *row, "Label", control.unitName, "1," + CORE::Int32ToString( -UnitWidth + 4 ) + " 0,0", "0," + CORE::Int32ToString( UnitWidth - 4 ) + " 1,0" );
        GUI::CFormLayout::SetWidgetNodeProperty( *unit, GUI::EWidgetProperty_Text, def.unit );
        GUI::CFormLayout::SetWidgetNodeProperty( *unit, GUI::EWidgetProperty_VTextAlign, "Center" );
    }
}

/*-------------------------------------------------------------------------*/

void
CLobbySettingsPanelForm::FitRows( void )
{GUCEF_TRACE;

    GUI::CGUIContext* context = GetGUIContext();
    GUI::CWidget* root = GetRootWidget();
    if ( GUCEF_NULL == context || GUCEF_NULL == root )
        return;

    // a label that does not fit next to its control is broken into lines and its row is as high as they need
    context->FlushChanges();
    for ( UInt32 i=0; i<(UInt32)m_controls.size(); ++i )
    {
        if ( m_controls[ i ].labelName.IsNULLOrEmpty() )
            continue;
        GUI::CWidget* label = GetWidget( m_controls[ i ].labelName );
        GUI::CWidget* row = GUCEF_NULL != label ? label->GetParentWidget() : GUCEF_NULL;
        if ( GUCEF_NULL == row || label->GetRect().width <= 0 )
            continue;

        CORE::CString font = label->GetProperty( GUI::EWidgetProperty_Font );
        CORE::CString text = label->GetProperty( GUI::EWidgetProperty_Text );
        Int32 width = 0, height = 0;
        Int32 rowHeight = RowHeight;
        bool wrap = false;
        if ( context->MeasureText( font, text, 0, width, height ) && width > label->GetRect().width )
        {
            context->MeasureText( font, text, label->GetRect().width, width, height );
            wrap = true;
            if ( height + 8 > rowHeight )
                rowHeight = height + 8;
        }
        label->SetBoolProperty( GUI::EWidgetProperty_WordWrap, wrap );
        row->SetProperty( GUI::EWidgetProperty_Size, "1,0 0," + CORE::Int32ToString( rowHeight ) );
    }
    m_fittedWidth = root->GetRect().width;
}

/*-------------------------------------------------------------------------*/

void
CLobbySettingsPanelForm::AddBlockNodes( CORE::CDataNode& parent, UInt32 first, UInt32 count, bool grouped, UInt32& groupCount )
{GUCEF_TRACE;

    if ( !grouped )
    {
        AddControlNodes( parent, first );
        return;
    }
    CORE::CDataNode* groupNode = GUI::CFormLayout::AddWidgetNode( parent, "GroupBox", "Group" + CORE::UInt32ToString( groupCount++ ), "0,0 0,0", "1,0 0,0" );
    GUI::CFormLayout::SetWidgetNodeProperty( *groupNode, GUI::EWidgetProperty_Caption, m_controls[ first ].def.group );
    for ( UInt32 i=first; i<first+count; ++i )
        AddControlNodes( *groupNode, i );
}

/*-------------------------------------------------------------------------*/

void
CLobbySettingsPanelForm::BuildLayout( CORE::CDataNode& layout )
{GUCEF_TRACE;

    // the groups below each other, as high as their rows need: in a scroll panel of its own, unless the host of the form scrolls or
    // makes room for all of it
    CORE::CDataNode* parent = &layout;
    if ( !m_hostMakesRoom )
    {
        parent = GUI::CFormLayout::AddWidgetNode( layout, "ScrollPanel", "LobbySettingsPanel", "0,0 0,0", "1,0 1,0" );
        GUI::CFormLayout::SetWidgetNodeProperty( *parent, GUI::EWidgetProperty_BottomPadding, "4" );
    }

    // consecutive settings of one group share a group box: the blocks are those boxes and the settings that are in none
    typedef GUCEF::vector< SBlock > TBlockVector;
    TBlockVector blocks;
    for ( UInt32 i=0; i<(UInt32)m_controls.size(); ++i )
    {
        const CORE::CString& group = m_controls[ i ].def.group;
        if ( group.IsNULLOrEmpty() )
        {
            SBlock block = { i, 1, false, RowHeight };
            blocks.push_back( block );
        }
        else
        if ( !blocks.empty() && blocks.back().grouped && m_controls[ blocks.back().first ].def.group == group )
        {
            ++blocks.back().count;
        }
        else
        {
            SBlock block = { i, 1, true, 0 };
            blocks.push_back( block );
        }
    }
    Int32 total = 0;
    for ( UInt32 b=0; b<(UInt32)blocks.size(); ++b )
    {
        if ( blocks[ b ].grouped )
            blocks[ b ].height = 2 * GroupPadding + CaptionHeight + (Int32) blocks[ b ].count * ( RowHeight + 4 );
        total += blocks[ b ].height + 6;
    }

    UInt32 groupCount = 0;
    if ( m_columns <= 1 )
    {
        CORE::CDataNode* list = GUI::CFormLayout::AddWidgetNode( *parent, "VBox", "Settings", "0,0 0,0", "1,0 0,0" );
        GUI::CFormLayout::SetWidgetNodeProperty( *list, GUI::EWidgetProperty_AutoHeight, "true" );
        GUI::CFormLayout::SetWidgetNodeProperty( *list, GUI::EWidgetProperty_Padding, "4" );
        GUI::CFormLayout::SetWidgetNodeProperty( *list, GUI::EWidgetProperty_Spacing, "6" );
        for ( UInt32 b=0; b<(UInt32)blocks.size(); ++b )
            AddBlockNodes( *list, blocks[ b ].first, blocks[ b ].count, blocks[ b ].grouped, groupCount );
        return;
    }

    // several columns side by side, filled in turn with about as much height each
    CORE::CDataNode* list = GUI::CFormLayout::AddWidgetNode( *parent, "HBox", "Settings", "0,0 0,0", "1,0 0,0" );
    GUI::CFormLayout::SetWidgetNodeProperty( *list, GUI::EWidgetProperty_AutoHeight, "true" );
    GUI::CFormLayout::SetWidgetNodeProperty( *list, GUI::EWidgetProperty_Padding, "4" );
    GUI::CFormLayout::SetWidgetNodeProperty( *list, GUI::EWidgetProperty_Spacing, "12" );
    CORE::CString columnWidth = CORE::CString( CORE::DoubleToString( 1.0 / (Float64) m_columns ) ) + ",-12 0,0";
    GUCEF::vector< CORE::CDataNode* > columns;
    for ( UInt32 c=0; c<m_columns; ++c )
    {
        CORE::CDataNode* column = GUI::CFormLayout::AddWidgetNode( *list, "VBox", "Column" + CORE::UInt32ToString( c ), "0,0 0,0", columnWidth );
        GUI::CFormLayout::SetWidgetNodeProperty( *column, GUI::EWidgetProperty_AutoHeight, "true" );
        GUI::CFormLayout::SetWidgetNodeProperty( *column, GUI::EWidgetProperty_Spacing, "6" );
        columns.push_back( column );
    }
    Int32 filled = 0;
    UInt32 column = 0;
    for ( UInt32 b=0; b<(UInt32)blocks.size(); ++b )
    {
        // the next column when this block would sit mostly beyond the share of the column
        if ( column + 1 < m_columns && filled + ( blocks[ b ].height + 6 ) / 2 > total * (Int32) ( column + 1 ) / (Int32) m_columns )
            ++column;
        AddBlockNodes( *columns[ column ], blocks[ b ].first, blocks[ b ].count, blocks[ b ].grouped, groupCount );
        filled += blocks[ b ].height + 6;
    }
}

/*-------------------------------------------------------------------------*/

UInt32
CLobbySettingsPanelForm::GetColumnsForWidth( Int32 width ) const
{GUCEF_TRACE;

    Int32 columns = width / MinColumnWidth;
    return columns < 1 ? 1 : ( columns > MaxColumns ? (UInt32) MaxColumns : (UInt32) columns );
}

/*-------------------------------------------------------------------------*/

void
CLobbySettingsPanelForm::UpdateColumns( void )
{GUCEF_TRACE;

    GUI::CGUIContext* context = GetGUIContext();
    if ( GUCEF_NULL == context || GUCEF_NULL == m_placeholder )
        return;
    context->FlushChanges();
    Int32 width = m_placeholder->GetRect().width;
    if ( width <= 0 )
        return;
    // the room of the groups is the width less the scroll bar of the panel they scroll in
    if ( GUI::CGuiGlobal::Instance()->GetWidgetTypeRegistry().IsA( m_placeholder->GetType(), "ScrollPanel" ) )
        width -= m_placeholder->GetIntProperty( GUI::EWidgetProperty_ScrollBarWidth );
    else
    if ( !m_hostMakesRoom )
        width -= ScrollBarWidth;
    m_wantedColumns = GetColumnsForWidth( width );
}

/*-------------------------------------------------------------------------*/

bool
CLobbySettingsPanelForm::Rebuild( void )
{GUCEF_TRACE;

    // one control per setting, named by place: the keys of settings can be anything
    m_controls.clear();
    for ( UInt32 i=0; i<(UInt32)m_schema.settings.size(); ++i )
    {
        TControl control;
        control.def = m_schema.settings[ i ];
        control.widgetName = "Setting" + CORE::UInt32ToString( i );
        if ( MULTIPLAY::CLobbySettingDef::TYPE_BOOL != control.def.type )
        {
            control.labelName = "Label" + CORE::UInt32ToString( i );
            if ( !control.def.unit.IsNULLOrEmpty() )
                control.unitName = "Unit" + CORE::UInt32ToString( i );
        }
        m_controls.push_back( control );
    }

    if ( GUCEF_NULL == m_placeholder )
        return true;

    // as many columns as the room has: a wide window gets the groups side by side
    UpdateColumns();
    m_columns = m_wantedColumns;

    CORE::CDataNode layout;
    GUI::CFormLayout::InitLayout( layout );
    BuildLayout( layout );
    return LoadLayout( layout );
}

bool
CLobbySettingsPanelForm::Build( GUI::CWidget* placeholder )
{GUCEF_TRACE;

    if ( GUCEF_NULL == placeholder )
        return false;
    SetContext( placeholder->GetContext() );
    SetParent( placeholder );
    m_placeholder = placeholder;
    m_hostMakesRoom = GUI::CGuiGlobal::Instance()->GetWidgetTypeRegistry().IsA( placeholder->GetType(), "ScrollPanel" ) || placeholder->GetBoolProperty( GUI::EWidgetProperty_AutoHeight );
    return Rebuild();
}

/*-------------------------------------------------------------------------*/

void
CLobbySettingsPanelForm::OnPostLayoutLoad( void )
{GUCEF_TRACE;

    GUI::CFormEx::OnPostLayoutLoad();

    // one handler for the widget of every setting, which of them it is follows from the widget
    TEventCallback controlCallback( this, &CLobbySettingsPanelForm::OnControlChanged );
    for ( UInt32 i=0; i<(UInt32)m_controls.size(); ++i )
    {
        GUI::CWidget* widget = GetWidget( m_controls[ i ].widgetName );
        if ( GUCEF_NULL == widget )
            continue;
        if ( MULTIPLAY::CLobbySettingDef::TYPE_STRING == m_controls[ i ].def.type )
        {
            SubscribeTo( widget, GUI::CEditbox::TextEnteredEvent, controlCallback );
            SubscribeTo( widget, GUI::CWidget::KeyboardFocusLostEvent, controlCallback );
        }
        else
            SubscribeTo( widget, GUI::CWidget::ValueChangedEvent, controlCallback );
    }
    FitRows();
    Refresh();

    // the rows follow the width of the panel
    GUI::CGUIContext* context = GetGUIContext();
    if ( GUCEF_NULL != context && !m_subscribedToFrames )
    {
        TEventCallback callback( this, &CLobbySettingsPanelForm::OnFrameStarted );
        SubscribeTo( context, GUI::CGUIContext::FrameStartedEvent, callback );
        m_subscribedToFrames = true;
    }
}

/*-------------------------------------------------------------------------*/

void
CLobbySettingsPanelForm::SetSchema( const MULTIPLAY::CLobbySettingsSchema& schema )
{GUCEF_TRACE;

    m_schema = schema;
    m_values.clear();
    Rebuild();
}

/*-------------------------------------------------------------------------*/

void
CLobbySettingsPanelForm::SetValues( const CORE::CValueList& values )
{GUCEF_TRACE;

    for ( UInt32 i=0; i<(UInt32)m_controls.size(); ++i )
    {
        const CORE::CString& key = m_controls[ i ].def.key;
        if ( values.HasKey( key ) )
            m_values[ key ] = values.GetValueAlways( key ).AsString();
    }
    Refresh();
}

/*-------------------------------------------------------------------------*/

void
CLobbySettingsPanelForm::GetValues( CORE::CValueList& values )
{GUCEF_TRACE;

    values.SetAllowMultipleValues( false );
    for ( UInt32 i=0; i<(UInt32)m_controls.size(); ++i )
        values.Set( m_controls[ i ].def.key, GetValue( m_controls[ i ].def.key ) );
}

/*-------------------------------------------------------------------------*/

void
CLobbySettingsPanelForm::SetLobby( MULTIPLAY::CILobbyPtr lobby )
{GUCEF_TRACE;

    if ( !m_lobby.IsNULL() )
        UnsubscribeFrom( m_lobby.GetPointerAlways() );

    m_lobby = lobby;
    if ( !m_lobby.IsNULL() )
    {
        TEventCallback callback( this, &CLobbySettingsPanelForm::OnLobbyStateChanged );
        SubscribeTo( m_lobby.GetPointerAlways(), MULTIPLAY::CILobby::LobbyStateChangedEvent, callback );
        m_schema = m_lobby->GetSchema();
        m_values.clear();
    }
    Rebuild();
}

/*-------------------------------------------------------------------------*/

const MULTIPLAY::CILobbyPtr&
CLobbySettingsPanelForm::GetLobby( void ) const
{GUCEF_TRACE;

    return m_lobby;
}

/*-------------------------------------------------------------------------*/

const MULTIPLAY::CLobbySettingsSchema&
CLobbySettingsPanelForm::GetSchema( void ) const
{GUCEF_TRACE;

    return m_schema;
}

/*-------------------------------------------------------------------------*/

UInt32
CLobbySettingsPanelForm::GetSettingCount( void ) const
{GUCEF_TRACE;

    return (UInt32) m_controls.size();
}

/*-------------------------------------------------------------------------*/

Int32
CLobbySettingsPanelForm::FindControlIndex( const CORE::CString& key ) const
{GUCEF_TRACE;

    for ( UInt32 i=0; i<(UInt32)m_controls.size(); ++i )
        if ( m_controls[ i ].def.key == key )
            return (Int32) i;
    return -1;
}

/*-------------------------------------------------------------------------*/

Int32
CLobbySettingsPanelForm::FindControlIndexOfWidget( CORE::CNotifier* widget )
{GUCEF_TRACE;

    for ( UInt32 i=0; i<(UInt32)m_controls.size(); ++i )
        if ( widget == GetWidget( m_controls[ i ].widgetName ) )
            return (Int32) i;
    return -1;
}

/*-------------------------------------------------------------------------*/

GUI::CWidget*
CLobbySettingsPanelForm::GetControl( const CORE::CString& key )
{GUCEF_TRACE;

    Int32 index = FindControlIndex( key );
    return index < 0 ? GUCEF_NULL : GetWidget( m_controls[ index ].widgetName );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CLobbySettingsPanelForm::GetLabelText( const CORE::CString& key )
{GUCEF_TRACE;

    Int32 index = FindControlIndex( key );
    if ( index < 0 )
        return CORE::CString::Empty;
    const TControl& control = m_controls[ index ];
    GUI::CWidget* label = GetWidget( control.labelName.IsNULLOrEmpty() ? control.widgetName : control.labelName );
    return GUCEF_NULL != label ? label->GetProperty( GUI::EWidgetProperty_Text ) : CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CLobbySettingsPanelForm::GetUnitText( const CORE::CString& key )
{GUCEF_TRACE;

    Int32 index = FindControlIndex( key );
    if ( index < 0 || m_controls[ index ].unitName.IsNULLOrEmpty() )
        return CORE::CString::Empty;
    GUI::CWidget* unit = GetWidget( m_controls[ index ].unitName );
    return GUCEF_NULL != unit ? unit->GetProperty( GUI::EWidgetProperty_Text ) : CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CLobbySettingsPanelForm::GetValueKey( const TControl& control ) const
{GUCEF_TRACE;

    // a value that belongs to the member is kept under the id of the member
    if ( !m_lobby.IsNULL() && MULTIPLAY::CLobbySettingDef::CHANGEPOLICY_OWNER == control.def.changePolicy )
        return m_lobby->GetLocalMemberId() + "/" + control.def.key;
    return control.def.key;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CLobbySettingsPanelForm::GetKnownValue( const TControl& control ) const
{GUCEF_TRACE;

    if ( !m_lobby.IsNULL() )
    {
        const MULTIPLAY::CLobbyState& state = m_lobby->GetState();
        CORE::CString valueKey = GetValueKey( control );
        if ( state.settings.HasKey( valueKey ) )
            return state.GetSetting( valueKey );
        return control.def.defaultValue;
    }
    TValueMap::const_iterator i = m_values.find( control.def.key );
    if ( i != m_values.end() )
        return (*i).second;
    return control.def.defaultValue;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CLobbySettingsPanelForm::ReadControl( const TControl& control )
{GUCEF_TRACE;

    GUI::CWidget* widget = GetWidget( control.widgetName );
    if ( GUCEF_NULL == widget )
        return GetKnownValue( control );

    switch ( control.def.type )
    {
        case MULTIPLAY::CLobbySettingDef::TYPE_BOOL:
        {
            return widget->GetBoolProperty( GUI::EWidgetProperty_Checked ) ? "true" : "false";
        }
        case MULTIPLAY::CLobbySettingDef::TYPE_INT:
        case MULTIPLAY::CLobbySettingDef::TYPE_FLOAT:
        {
            GUI::CSpinner* spinner = WidgetAs< GUI::CSpinner >( widget, "Spinner" );
            return GUCEF_NULL != spinner ? spinner->GetValueText() : widget->GetProperty( GUI::EWidgetProperty_Value );
        }
        case MULTIPLAY::CLobbySettingDef::TYPE_ENUM:
        {
            GUI::CCombobox* combo = WidgetAs< GUI::CCombobox >( widget, "Combobox" );
            if ( GUCEF_NULL == combo )
                return CORE::CString::Empty;
            Int32 selected = combo->GetSelectedIndex();
            if ( selected >= 0 && selected < (Int32) control.def.options.size() )
                return control.def.options[ selected ];
            return combo->GetText();
        }
        default:
        {
            return widget->GetProperty( GUI::EWidgetProperty_Text );
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CLobbySettingsPanelForm::ShowControl( const TControl& control, const CORE::CString& value )
{GUCEF_TRACE;

    GUI::CWidget* widget = GetWidget( control.widgetName );
    if ( GUCEF_NULL == widget )
        return;

    switch ( control.def.type )
    {
        case MULTIPLAY::CLobbySettingDef::TYPE_BOOL:
        {
            widget->SetProperty( GUI::EWidgetProperty_Checked, CORE::StringToBool( value, false ) ? "true" : "false" );
            break;
        }
        case MULTIPLAY::CLobbySettingDef::TYPE_INT:
        case MULTIPLAY::CLobbySettingDef::TYPE_FLOAT:
        {
            widget->SetProperty( GUI::EWidgetProperty_Value, value );
            break;
        }
        case MULTIPLAY::CLobbySettingDef::TYPE_ENUM:
        {
            GUI::CCombobox* combo = WidgetAs< GUI::CCombobox >( widget, "Combobox" );
            if ( GUCEF_NULL == combo )
                break;
            Int32 found = -1;
            for ( UInt32 i=0; i<(UInt32)control.def.options.size(); ++i )
                if ( control.def.options[ i ] == value )
                    found = (Int32) i;
            if ( found >= 0 )
                combo->SetSelectedIndex( found );
            else
            {
                combo->SetSelectedIndex( -1 );
                combo->SetText( value );
            }
            break;
        }
        default:
        {
            widget->SetProperty( GUI::EWidgetProperty_Text, value );
            break;
        }
    }
}

/*-------------------------------------------------------------------------*/

CORE::CString
CLobbySettingsPanelForm::GetValue( const CORE::CString& key )
{GUCEF_TRACE;

    Int32 index = FindControlIndex( key );
    if ( index < 0 )
        return CORE::CString::Empty;
    return ReadControl( m_controls[ index ] );
}

/*-------------------------------------------------------------------------*/

bool
CLobbySettingsPanelForm::IsProposal( const TControl& control ) const
{GUCEF_TRACE;

    return !m_lobby.IsNULL() &&
           MULTIPLAY::CLobbySettingDef::CHANGEPOLICY_VOTE == control.def.changePolicy &&
           !m_lobby->IsBoss();
}

/*-------------------------------------------------------------------------*/

bool
CLobbySettingsPanelForm::CanChange( const TControl& control ) const
{GUCEF_TRACE;

    if ( m_lobby.IsNULL() )
        return true;

    const MULTIPLAY::CLobbyState& state = m_lobby->GetState();
    if ( MULTIPLAY::LOBBYPHASE_OPEN != state.phase )
        return false;
    const MULTIPLAY::CLobbyMember* member = state.FindMember( m_lobby->GetLocalMemberId() );
    if ( GUCEF_NULL == member || member->IsSpectator() )
        return false;
    if ( MULTIPLAY::CLobbySettingDef::CHANGEPOLICY_BOSS == control.def.changePolicy )
        return m_lobby->IsBoss();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CLobbySettingsPanelForm::IsSettingEditable( const CORE::CString& key )
{GUCEF_TRACE;

    Int32 index = FindControlIndex( key );
    if ( index < 0 )
        return false;
    GUI::CWidget* widget = GetWidget( m_controls[ index ].widgetName );
    return GUCEF_NULL != widget && widget->GetBoolProperty( GUI::EWidgetProperty_Enabled );
}

/*-------------------------------------------------------------------------*/

bool
CLobbySettingsPanelForm::IsGateOpen( const TControl& control ) const
{GUCEF_TRACE;

    if ( !control.def.HasEnabledBy() )
        return true;

    // a gate on a setting the schema does not have is no gate
    Int32 index = FindControlIndex( control.def.enabledByKey );
    if ( index < 0 )
        return true;
    const TControl& controller = m_controls[ index ];
    return control.def.IsEnabledByValue( controller.def, GetKnownValue( controller ) );
}

/*-------------------------------------------------------------------------*/

bool
CLobbySettingsPanelForm::IsGateOpen( const CORE::CString& key )
{GUCEF_TRACE;

    Int32 index = FindControlIndex( key );
    return index < 0 || IsGateOpen( m_controls[ index ] );
}

/*-------------------------------------------------------------------------*/

bool
CLobbySettingsPanelForm::IsProposalOnly( const CORE::CString& key )
{GUCEF_TRACE;

    Int32 index = FindControlIndex( key );
    return index >= 0 && IsProposal( m_controls[ index ] );
}

/*-------------------------------------------------------------------------*/

void
CLobbySettingsPanelForm::Refresh( void )
{GUCEF_TRACE;

    if ( m_updating )
        return;
    m_updating = true;
    bool labelsChanged = false;

    for ( UInt32 i=0; i<(UInt32)m_controls.size(); ++i )
    {
        const TControl& control = m_controls[ i ];
        GUI::CWidget* widget = GetWidget( control.widgetName );
        if ( GUCEF_NULL == widget )
            continue;

        // text being typed is not taken away from the player
        bool typing = MULTIPLAY::CLobbySettingDef::TYPE_STRING == control.def.type && widget->HasFocus();
        if ( !typing )
            ShowControl( control, GetKnownValue( control ) );

        // a setting that depends on another is editable while that one has the value, and keeps its value when it is not
        bool canChange = CanChange( control );
        bool gateOpen = IsGateOpen( control );
        bool proposal = canChange && IsProposal( control );
        widget->SetEnabled( canChange && gateOpen );

        CORE::CString label = LabelOf( control.def );
        if ( proposal )
            label += ProposeSuffix;
        GUI::CWidget* labelWidget = GetWidget( control.labelName.IsNULLOrEmpty() ? control.widgetName : control.labelName );
        if ( GUCEF_NULL != labelWidget && labelWidget->GetProperty( GUI::EWidgetProperty_Text ) != label )
        {
            labelWidget->SetProperty( GUI::EWidgetProperty_Text, label );
            labelsChanged = true;
        }

        CORE::CString tip;
        if ( proposal )
            tip = "A change is a proposal that the players vote on";
        else
        if ( !canChange && !m_lobby.IsNULL() && MULTIPLAY::CLobbySettingDef::CHANGEPOLICY_BOSS == control.def.changePolicy )
            tip = "Only the boss changes this";
        else
        if ( !gateOpen )
        {
            Int32 controllerIndex = FindControlIndex( control.def.enabledByKey );
            tip = "Available when " + LabelOf( m_controls[ controllerIndex ].def ) + " is " + control.def.enabledByValue;
        }
        widget->SetProperty( GUI::EWidgetProperty_Tooltip, tip );
    }

    m_updating = false;

    // a label that says "(propose)" now may need another line
    if ( labelsChanged )
        FitRows();
}

/*-------------------------------------------------------------------------*/

void
CLobbySettingsPanelForm::Commit( UInt32 index )
{GUCEF_TRACE;

    if ( index >= (UInt32) m_controls.size() )
        return;
    const TControl& control = m_controls[ index ];

    CORE::CString value = ReadControl( control );
    CORE::CString known = GetKnownValue( control );
    if ( control.def.IsSameValue( value, known ) )
        return;

    // a setting that is gated off keeps its value, whoever asks
    if ( !IsGateOpen( control ) )
    {
        Refresh();
        return;
    }

    if ( m_lobby.IsNULL() )
    {
        m_values[ control.def.key ] = value;
        Refresh();          // other settings can depend on this one
        CSettingChangedEventData eventData( control.def.key, value, false );
        NotifyObservers( SettingChangedEvent, &eventData );
        return;
    }

    if ( !CanChange( control ) )
    {
        Refresh();
        return;
    }

    bool proposal = IsProposal( control );
    CSettingChangedEventData eventData( control.def.key, value, proposal );
    m_lobby->SetSetting( GetValueKey( control ), value );
    if ( proposal )
        Refresh();          // a proposal changes nothing until the players agree
    NotifyObservers( SettingChangedEvent, &eventData );
}

/*-------------------------------------------------------------------------*/

void
CLobbySettingsPanelForm::OnFrameStarted( CORE::CNotifier* notifier    ,
                                         const CORE::CEvent& eventId  ,
                                         CORE::CICloneable* eventData )
{GUCEF_TRACE;

    GUI::CGUIContext* context = GetGUIContext();
    GUI::CWidget* root = GetRootWidget();
    if ( GUCEF_NULL == context || GUCEF_NULL == root )
        return;

    context->FlushChanges();
    if ( root->GetRect().width != m_fittedWidth && root->GetRect().width > 0 )
    {
        // a wider or narrower panel has more or fewer columns of groups, what the player set goes along
        UpdateColumns();
        if ( m_wantedColumns != m_columns )
        {
            if ( m_lobby.IsNULL() )
                for ( UInt32 i=0; i<(UInt32)m_controls.size(); ++i )
                    m_values[ m_controls[ i ].def.key ] = ReadControl( m_controls[ i ] );
            Rebuild();
            return;
        }
        FitRows();
    }
}

/*-------------------------------------------------------------------------*/

void
CLobbySettingsPanelForm::OnLobbyStateChanged( CORE::CNotifier* notifier    ,
                                              const CORE::CEvent& eventId  ,
                                              CORE::CICloneable* eventData )
{GUCEF_TRACE;

    Refresh();
}

/*-------------------------------------------------------------------------*/

void
CLobbySettingsPanelForm::OnControlChanged( CORE::CNotifier* notifier    ,
                                           const CORE::CEvent& eventId  ,
                                           CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( m_updating )
        return;
    Int32 index = FindControlIndexOfWidget( notifier );
    if ( index >= 0 )
        Commit( (UInt32) index );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CLobbySettingsPanelForm::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::GAMEGUI::CLobbySettingsPanelForm";
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
