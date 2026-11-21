/*
 *  AssetWorkbenchLib: views and controllers of the asset workbench
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
#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */
#ifndef GUCEF_CORE_CLONEABLES_H
#include "cloneables.h"
#define GUCEF_CORE_CLONEABLES_H
#endif /* GUCEF_CORE_CLONEABLES_H ? */
#ifndef GUCEF_GUI_CFORMLAYOUT_H
#include "gucefGUI_CFormLayout.h"
#define GUCEF_GUI_CFORMLAYOUT_H
#endif /* GUCEF_GUI_CFORMLAYOUT_H ? */
#ifndef GUCEF_GUI_CGRIDVIEW_H
#include "gucefGUI_CGridView.h"
#define GUCEF_GUI_CGRIDVIEW_H
#endif /* GUCEF_GUI_CGRIDVIEW_H ? */
#ifndef GUCEF_GUI_CGUICONTEXT_H
#include "gucefGUI_CGUIContext.h"
#define GUCEF_GUI_CGUICONTEXT_H
#endif /* GUCEF_GUI_CGUICONTEXT_H ? */
#include "AssetWorkbenchLib_CServerSettingsForm.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETWORKBENCH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CEvent CServerSettingsForm::ClosedEvent = "GUCEF::ASSETWORKBENCH::CServerSettingsForm::ClosedEvent";

namespace {

/** The prefix of the keys of the rows of the sync profiles: "sync:<store id>:<member>" */
const char* const SyncKeyPrefix = "sync:";

}

/*-------------------------------------------------------------------------*/

void
CServerSettingsForm::RegisterEvents( void )
{GUCEF_TRACE;

    ClosedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CServerSettingsForm::SRow::SRow( void )
    : key()
    , path()
    , storeId()
    , member()
    , editable( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CServerSettingsForm::CServerSettingsForm( void )
    : GUI::CFormEx()
    , m_controller( GUCEF_NULL )
    , m_list( GUCEF_NULL )
    , m_rows()
    , m_edits()
    , m_notice()
    , m_outcome()
    , m_details()
    , m_waitedInMs( 0 )
    , m_open( false )
    , m_dirty( true )
    , m_frameSubscribed( false )
    , m_rebuilding( false )
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CServerSettingsForm::CServerSettingsForm( GUI::CIGUIContext* context )
    : GUI::CFormEx( context )
    , m_controller( GUCEF_NULL )
    , m_list( GUCEF_NULL )
    , m_rows()
    , m_edits()
    , m_notice()
    , m_outcome()
    , m_details()
    , m_waitedInMs( 0 )
    , m_open( false )
    , m_dirty( true )
    , m_frameSubscribed( false )
    , m_rebuilding( false )
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CServerSettingsForm::~CServerSettingsForm()
{GUCEF_TRACE;

    UnsubscribeAllFromObserver();
    GUCEF_DELETE m_list;
    m_list = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

void
CServerSettingsForm::BuildLayout( CORE::CDataNode& layout )
{GUCEF_TRACE;

    CORE::CDataNode* window = GUI::CFormLayout::AddWidgetNode( layout, "Window", "ServerSettingsForm", "0.08,0 0.04,0", "0.84,0 0.92,0" );
    GUI::CFormLayout::SetWidgetNodeProperty( *window, GUI::EWidgetProperty_Title, "Server settings" );
    GUI::CFormLayout::SetWidgetNodeProperty( *window, GUI::EWidgetProperty_TitleHeight, "28" );
    CORE::CDataNode* c = GUI::CFormLayout::AddWidgetNode( *window, "Panel", "Content", "0,0 0,28", "1,0 1,-28" );

    // whether this device may change the settings, then the settings, then the selected one and the outcome of the last change
    GUI::CFormLayout::AddWidgetNode( *c, "Label", "Notice", "0,8 0,4", "1,-16 0,28" );
    GUI::CFormLayout::AddWidgetNode( *c, "Panel", "Properties", "0,0 0,36", "1,0 1,-150" );
    GUI::CFormLayout::AddWidgetNode( *c, "Label", "Details", "0,8 1,-110", "1,-16 0,28" );
    struct SButton { const char* name; const char* text; const char* position; const char* size; };
    const SButton buttons[] =
    {
        { "Apply", "Apply", "0,8 1,-76", "0,90 0,30" }, { "ResetToSite", "Reset to site value", "0,104 1,-76", "0,170 0,30" },
        { "Refresh", "Refresh", "0,280 1,-76", "0,90 0,30" }, { "Close", "Close", "1,-102 1,-76", "0,94 0,30" }
    };
    for ( UInt32 i=0; i<(UInt32) ( sizeof( buttons ) / sizeof( buttons[ 0 ] ) ); ++i )
    {
        CORE::CDataNode* button = GUI::CFormLayout::AddWidgetNode( *c, "Button", buttons[ i ].name, buttons[ i ].position, buttons[ i ].size );
        GUI::CFormLayout::SetWidgetNodeProperty( *button, GUI::EWidgetProperty_Text, buttons[ i ].text );
    }
    GUI::CFormLayout::AddWidgetNode( *c, "Label", "Outcome", "0,8 1,-38", "1,-16 0,28" );
}

/*-------------------------------------------------------------------------*/

bool
CServerSettingsForm::Build( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == GetContext() )
        return false;
    CORE::CDataNode layout;
    GUI::CFormLayout::InitLayout( layout );
    BuildLayout( layout );
    if ( !LoadLayout( layout ) )
        return false;
    Hide();
    m_open = false;
    return GUCEF_NULL != m_list;
}

/*-------------------------------------------------------------------------*/

void
CServerSettingsForm::OnPostLayoutLoad( void )
{GUCEF_TRACE;

    GUI::CFormEx::OnPostLayoutLoad();
    struct SButton { const char* name; void ( CServerSettingsForm::*handler )( CORE::CNotifier*, const CORE::CEvent&, CORE::CICloneable* ); };
    const SButton buttons[] =
    {
        { "Apply", &CServerSettingsForm::OnApplyActivated }, { "ResetToSite", &CServerSettingsForm::OnResetActivated },
        { "Refresh", &CServerSettingsForm::OnRefreshActivated }, { "Close", &CServerSettingsForm::OnCloseActivated }
    };
    for ( UInt32 i=0; i<(UInt32) ( sizeof( buttons ) / sizeof( buttons[ 0 ] ) ); ++i )
    {
        GUI::CWidget* button = GetWidget( buttons[ i ].name );
        if ( GUCEF_NULL != button )
        {
            TEventCallback callback( this, buttons[ i ].handler );
            SubscribeTo( button, GUI::CWidget::ActivatedEvent, callback );
        }
    }

    // the property list in its place; its own buttons are not used: a row is sent with Apply of this form
    GUI::CWidget* host = GetWidget( "Properties" );
    if ( GUCEF_NULL != host && GUCEF_NULL == m_list )
    {
        m_list = GUCEF_NEW GUI::CPropertyInspectorForm( GetContext() );
        if ( !m_list->Build( host ) )
        {
            GUCEF_DELETE m_list;
            m_list = GUCEF_NULL;
        }
        else
        {
            const char* const unused[] = { "Apply", "Revert" };
            for ( UInt32 u=0; u<2; ++u )
            {
                GUI::CWidget* widget = m_list->GetWidget( unused[ u ] );
                if ( GUCEF_NULL != widget )
                    widget->SetBoolProperty( GUI::EWidgetProperty_Visible, false );
            }
            TEventCallback changed( this, &CServerSettingsForm::OnPropertyChanged );
            SubscribeTo( m_list, GUI::CPropertyInspectorForm::PropertyChangedEvent, changed );
            GUI::CGridView* grid = m_list->GetGrid();
            if ( GUCEF_NULL != grid )
            {
                TEventCallback selection( this, &CServerSettingsForm::OnGridSelectionChanged );
                SubscribeTo( grid, GUI::CGridView::SelectionChangedEvent, selection );
                SubscribeTo( grid, GUI::CGridView::CellCursorChangedEvent, selection );
            }
        }
    }
    GUI::CGUIContext* context = GetGUIContext();
    if ( GUCEF_NULL != context && !m_frameSubscribed )
    {
        m_frameSubscribed = true;
        TEventCallback callback( this, &CServerSettingsForm::OnFrameStarted );
        SubscribeTo( context, GUI::CGUIContext::FrameStartedEvent, callback );
    }
    Rebuild();
}

/*-------------------------------------------------------------------------*/

bool
CServerSettingsForm::Open( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == GetRootWidget() && !Build() )
        return false;
    if ( !Show() )
        return false;
    m_open = true;
    RefreshNow();
    return true;
}

/*-------------------------------------------------------------------------*/

void
CServerSettingsForm::Close( void )
{GUCEF_TRACE;

    const bool wasOpen = m_open;
    m_open = false;
    if ( GUCEF_NULL != GetRootWidget() )
        Hide();
    if ( wasOpen )
        NotifyObservers( ClosedEvent );
}

/*-------------------------------------------------------------------------*/

bool
CServerSettingsForm::IsOpen( void ) const
{GUCEF_TRACE;

    return m_open;
}

/*-------------------------------------------------------------------------*/

void
CServerSettingsForm::SetController( CServerSettingsController* controller )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_controller )
        UnsubscribeFrom( m_controller );
    m_controller = controller;
    m_edits.clear();
    m_outcome.Clear();
    if ( GUCEF_NULL != m_controller )
    {
        TEventCallback read( this, &CServerSettingsForm::OnControllerRead );
        SubscribeTo( m_controller, CServerSettingsController::SettingsReadEvent, read );
        TEventCallback outcome( this, &CServerSettingsForm::OnControllerOutcome );
        SubscribeTo( m_controller, CServerSettingsController::OutcomeEvent, outcome );
    }
    m_dirty = true;
    Rebuild();
}

/*-------------------------------------------------------------------------*/

GUI::CPropertyRowDescriptor
CServerSettingsForm::MakeRowDescriptor( const ASSETSTORE::CAssetSettingDescriptor& descriptor )
{GUCEF_TRACE;

    typedef ASSETSTORE::CAssetSettingDescriptor D;
    GUI::CPropertyRowDescriptor row;
    if ( D::TypeBool == descriptor.type )
        row = GUI::CPropertyRowDescriptor::Boolean();
    else
    if ( D::TypeEnum == descriptor.type )
        row = GUI::CPropertyRowDescriptor::Choice( descriptor.allowedValues );
    else
    if ( D::TypeUInt == descriptor.type || D::TypeInt == descriptor.type || D::TypeFloat == descriptor.type )
    {
        const Int32 decimals = D::TypeFloat == descriptor.type ? -1 : 0;
        if ( !descriptor.minimum.IsNULLOrEmpty() && !descriptor.maximum.IsNULLOrEmpty() )
            row = GUI::CPropertyRowDescriptor::NumberInRange( CORE::StringToDouble( descriptor.minimum ), CORE::StringToDouble( descriptor.maximum ), decimals );
        else
            row = GUI::CPropertyRowDescriptor::Number( decimals );
    }
    else
        row = GUI::CPropertyRowDescriptor::Text();
    row.label = descriptor.key;
    row.tooltip = descriptor.description + " (" + descriptor.GetRangeText() + ", default " + descriptor.defaultValue + "; " +
                  ( descriptor.AppliesLive() ? "applies at once" : "applies after a restart of the service" ) + ")";
    return row;
}

/*-------------------------------------------------------------------------*/

void
CServerSettingsForm::Rebuild( void )
{GUCEF_TRACE;

    m_dirty = false;
    m_waitedInMs = 0;
    m_rows.clear();
    if ( GUCEF_NULL == m_list )
        return;
    const CORE::CString selectedKey = GetSelectedKey();
    CORE::CDataNode data( "settings", GUCEF_DATATYPE_OBJECT );
    GUI::CPropertyInspectorForm::TDescriptorMap descriptors;
    if ( GUCEF_NULL != m_controller && m_controller->HasDescription() )
    {
        const ASSETSTORE::CAssetSettingsDescription& description = m_controller->GetDescription();
        for ( size_t e=0; e<description.entries.size(); ++e )
        {
            const ASSETSTORE::CAssetSettingState& state = description.entries[ e ];
            const CORE::CString group = state.descriptor.group.IsNULLOrEmpty() ? CORE::CString( "Settings" ) : state.descriptor.group;
            CORE::CDataNode* groupNode = data.FindChild( group );
            if ( GUCEF_NULL == groupNode )
                groupNode = data.AddChild( group, GUCEF_DATATYPE_OBJECT );
            groupNode->SetAttribute( state.descriptor.key, state.value );
            SRow row;
            row.key = state.descriptor.key;
            row.path = group + "@" + state.descriptor.key;
            row.editable = description.canChange;
            GUI::CPropertyRowDescriptor rowDescriptor = MakeRowDescriptor( state.descriptor );
            if ( !row.editable )
                rowDescriptor = rowDescriptor.AsReadOnly();
            descriptors[ row.path ] = rowDescriptor;
            m_rows.push_back( row );
        }

        // the sync profiles of the stores, through the operations of the library sync: their plain members
        const CServerSettingsController::TStoreProfileVector& profiles = m_controller->GetProfiles();
        for ( size_t s=0; s<profiles.size(); ++s )
        {
            if ( !profiles[ s ].known )
                continue;
            const CORE::CString group = "Sync profile " + profiles[ s ].storeId;
            CORE::CDataNode* groupNode = data.AddChild( group, GUCEF_DATATYPE_OBJECT );
            CORE::CDataNode members( "profile", GUCEF_DATATYPE_OBJECT );
            CORE::CDataNodeSerializableSettings serializerSettings;
            profiles[ s ].profile.Serialize( members, serializerSettings );
            for ( CORE::CDataNode::TAttributeMap::const_iterator a = members.AttributeBegin(); a != members.AttributeEnd(); ++a )
            {
                const CORE::CString member = (*a).first;
                const CORE::CVariant& value = (*a).second;
                groupNode->SetAttribute( member, value.AsString( CORE::CString::Empty, true ) );
                SRow row;
                row.key = CORE::CString( SyncKeyPrefix ) + profiles[ s ].storeId + ":" + member;
                row.path = group + "@" + member;
                row.storeId = profiles[ s ].storeId;
                row.member = member;
                row.editable = description.callerMayChange;
                GUI::CPropertyRowDescriptor rowDescriptor;
                if ( value.IsBoolean() )
                    rowDescriptor = GUI::CPropertyRowDescriptor::Boolean();
                else
                if ( value.IsInteger() )
                    rowDescriptor = GUI::CPropertyRowDescriptor::NumberInRange( 0, 1.0e15, 0 );
                else
                if ( value.IsFloat() )
                    rowDescriptor = GUI::CPropertyRowDescriptor::NumberInRange( 0, 1.0e15, -1 );
                else
                    rowDescriptor = GUI::CPropertyRowDescriptor::Text();
                rowDescriptor.tooltip = "A member of the sync profile of the store " + profiles[ s ].displayName + "; applies to the next sync";
                if ( !row.editable )
                    rowDescriptor = rowDescriptor.AsReadOnly();
                descriptors[ row.path ] = rowDescriptor;
                m_rows.push_back( row );
            }
        }
    }
    m_rebuilding = true;
    m_list->SetDescriptors( descriptors );
    m_list->SetDataCopy( data );
    // the edits that were not sent stay
    CORE::CString::StringMap kept;
    for ( CORE::CString::StringMap::const_iterator e = m_edits.begin(); e != m_edits.end(); ++e )
    {
        const SRow* row = FindRow( (*e).first );
        if ( GUCEF_NULL != row && row->editable && m_list->SetValue( row->path, (*e).second ) )
            kept[ (*e).first ] = (*e).second;
    }
    m_edits = kept;
    if ( !selectedKey.IsNULLOrEmpty() )
        SelectKey( selectedKey );
    m_rebuilding = false;
    ShowTexts();
    ShowButtons();
}

/*-------------------------------------------------------------------------*/

const CServerSettingsForm::SRow*
CServerSettingsForm::FindRow( const CORE::CString& key ) const
{GUCEF_TRACE;

    for ( size_t r=0; r<m_rows.size(); ++r )
        if ( m_rows[ r ].key == key )
            return &m_rows[ r ];
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

const CServerSettingsForm::SRow*
CServerSettingsForm::FindRowByPath( const CORE::CString& path ) const
{GUCEF_TRACE;

    for ( size_t r=0; r<m_rows.size(); ++r )
        if ( m_rows[ r ].path == path )
            return &m_rows[ r ];
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

const ASSETSTORE::CAssetSettingState*
CServerSettingsForm::FindState( const CORE::CString& key ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_controller || !m_controller->HasDescription() )
        return GUCEF_NULL;
    return m_controller->GetDescription().Find( key );
}

/*-------------------------------------------------------------------------*/

bool
CServerSettingsForm::SelectKey( const CORE::CString& key )
{GUCEF_TRACE;

    const SRow* row = FindRow( key );
    if ( GUCEF_NULL == row || GUCEF_NULL == m_list || !m_list->SelectPath( row->path ) )
        return false;
    ShowTexts();
    ShowButtons();
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CServerSettingsForm::GetSelectedKey( void ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_list )
        return CORE::CString::Empty;
    const SRow* row = FindRowByPath( m_list->GetSelectedPath() );
    return GUCEF_NULL != row ? row->key : CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

bool
CServerSettingsForm::EditValue( const CORE::CString& key, const CORE::CString& text, CORE::CString* problem )
{GUCEF_TRACE;

    const SRow* row = FindRow( key );
    if ( GUCEF_NULL == row || GUCEF_NULL == m_list )
    {
        if ( GUCEF_NULL != problem )
            *problem = "There is no setting " + key;
        return false;
    }
    return m_list->SetValue( row->path, text, problem );
}

/*-------------------------------------------------------------------------*/

bool
CServerSettingsForm::ApplySelected( void )
{GUCEF_TRACE;

    const CORE::CString key = GetSelectedKey();
    const SRow* row = FindRow( key );
    if ( GUCEF_NULL == m_controller || GUCEF_NULL == row || !row->editable )
        return false;
    CORE::CString::StringMap::iterator edit = m_edits.find( key );
    if ( edit == m_edits.end() )
    {
        m_outcome = "The value of " + key + " was not changed";
        ShowTexts();
        return false;
    }
    const CORE::CString value = (*edit).second;
    m_edits.erase( edit );
    const bool asked = row->storeId.IsNULLOrEmpty() ? m_controller->SetValue( key, value ) : m_controller->SetProfileMember( row->storeId, row->member, value );
    m_outcome = asked ? "Sent " + key + " = " + value + " to the service" : m_controller->GetProblem();
    ShowTexts();
    ShowButtons();
    return asked;
}

/*-------------------------------------------------------------------------*/

bool
CServerSettingsForm::ResetSelected( void )
{GUCEF_TRACE;

    const CORE::CString key = GetSelectedKey();
    const SRow* row = FindRow( key );
    if ( GUCEF_NULL == m_controller || GUCEF_NULL == row || !row->editable || !row->storeId.IsNULLOrEmpty() )
        return false;
    m_edits.erase( key );
    const bool asked = m_controller->ResetValue( key );
    m_outcome = asked ? "Asked the service to put " + key + " back to its site value" : m_controller->GetProblem();
    ShowTexts();
    ShowButtons();
    return asked;
}

/*-------------------------------------------------------------------------*/

bool
CServerSettingsForm::RefreshNow( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_controller )
    {
        m_notice = "The workbench is not connected to a remote asset store service";
        ShowTexts();
        return false;
    }
    return m_controller->Refresh();
}

/*-------------------------------------------------------------------------*/

bool
CServerSettingsForm::Update( UInt32 deltaInMs )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_controller )
        m_controller->Update();
    m_waitedInMs += deltaInMs;
    if ( m_open && m_dirty && m_waitedInMs >= (UInt32) RefreshIntervalInMs )
    {
        Rebuild();
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

void
CServerSettingsForm::ShowTexts( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_controller )
        m_notice = "The workbench is not connected to a remote asset store service";
    else
    if ( !m_controller->HasDescription() )
        m_notice = m_controller->GetProblem().IsNULLOrEmpty() ? CORE::CString( "Reading the settings of the service..." ) : m_controller->GetProblem();
    else
    {
        const ASSETSTORE::CAssetSettingsDescription& description = m_controller->GetDescription();
        if ( description.canChange )
            m_notice = "Changes are sent to the service and applied there at once, or after a restart where the details say so" +
                       CORE::CString( description.persistent ? CORE::CString::Empty : CORE::CString( "; the service keeps them until it stops" ) );
        else
            m_notice = "Read only: " + description.changeRefusal;
    }

    m_details.Clear();
    const CORE::CString key = GetSelectedKey();
    const SRow* row = FindRow( key );
    const ASSETSTORE::CAssetSettingState* state = FindState( key );
    if ( GUCEF_NULL != state )
    {
        const ASSETSTORE::CAssetSettingDescriptor& d = state->descriptor;
        m_details = d.key + ": " + state->value + ( d.unit.IsNULLOrEmpty() ? CORE::CString::Empty : " " + d.unit ) + " (" + state->source +
                    "; site " + state->siteValue + ", default " + d.defaultValue + "; " + d.GetRangeText() + "; " +
                    ( d.AppliesLive() ? "applies at once" : "applies after a restart" ) + ")" +
                    ( state->hasPendingValue ? "; " + state->pendingValue + " after the next restart" : CORE::CString::Empty );
    }
    else
    if ( GUCEF_NULL != row && !row->storeId.IsNULLOrEmpty() )
        m_details = row->member + " of the sync profile of " + row->storeId + "; applies to the next sync of the store";
    if ( GUCEF_NULL != row && m_edits.end() != m_edits.find( key ) )
        m_details += "; edited, not sent: Apply sends it";
    SetText( "Notice", m_notice );
    SetText( "Details", m_details );
    SetText( "Outcome", m_outcome );
}

/*-------------------------------------------------------------------------*/

void
CServerSettingsForm::ShowButtons( void )
{GUCEF_TRACE;

    const CORE::CString key = GetSelectedKey();
    const SRow* row = FindRow( key );
    const ASSETSTORE::CAssetSettingState* state = FindState( key );
    struct SEnabled { const char* name; bool enabled; };
    const SEnabled buttons[] =
    {
        { "Apply", GUCEF_NULL != row && row->editable && m_edits.end() != m_edits.find( key ) },
        { "ResetToSite", GUCEF_NULL != row && row->editable && GUCEF_NULL != state && ASSETSTORE::CAssetSettingState::SourceOverridden == state->source },
        { "Refresh", GUCEF_NULL != m_controller }
    };
    for ( UInt32 i=0; i<(UInt32) ( sizeof( buttons ) / sizeof( buttons[ 0 ] ) ); ++i )
    {
        GUI::CWidget* button = GetWidget( buttons[ i ].name );
        if ( GUCEF_NULL != button )
            button->SetBoolProperty( GUI::EWidgetProperty_Enabled, buttons[ i ].enabled );
    }
}

/*-------------------------------------------------------------------------*/

void
CServerSettingsForm::SetText( const char* widgetName, const CORE::CString& text )
{GUCEF_TRACE;

    GUI::CWidget* widget = GetWidget( widgetName );
    if ( GUCEF_NULL != widget )
        widget->SetProperty( GUI::EWidgetProperty_Text, text );
}

/*-------------------------------------------------------------------------*/

CORE::CString::StringVector
CServerSettingsForm::GetKeys( void ) const
{GUCEF_TRACE;

    CORE::CString::StringVector keys;
    for ( size_t r=0; r<m_rows.size(); ++r )
        keys.push_back( m_rows[ r ].key );
    return keys;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CServerSettingsForm::GetValueText( const CORE::CString& key ) const
{GUCEF_TRACE;

    const SRow* row = FindRow( key );
    return GUCEF_NULL != row && GUCEF_NULL != m_list ? m_list->GetValue( row->path ) : CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CServerSettingsForm::GetSourceText( const CORE::CString& key ) const
{GUCEF_TRACE;

    const ASSETSTORE::CAssetSettingState* state = FindState( key );
    return GUCEF_NULL != state ? state->source : CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CServerSettingsForm::GetApplyModeText( const CORE::CString& key ) const
{GUCEF_TRACE;

    const ASSETSTORE::CAssetSettingState* state = FindState( key );
    return GUCEF_NULL != state ? state->descriptor.applyMode : CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

bool
CServerSettingsForm::IsKeyEditable( const CORE::CString& key ) const
{GUCEF_TRACE;

    const SRow* row = FindRow( key );
    return GUCEF_NULL != row && row->editable;
}

/*-------------------------------------------------------------------------*/

bool
CServerSettingsForm::IsEditingAllowed( void ) const
{GUCEF_TRACE;

    return GUCEF_NULL != m_controller && m_controller->HasDescription() && m_controller->GetDescription().canChange;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CServerSettingsForm::GetNoticeText( void ) const
{GUCEF_TRACE;

    return m_notice;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CServerSettingsForm::GetDetailsText( void ) const
{GUCEF_TRACE;

    return m_details;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CServerSettingsForm::GetOutcomeText( void ) const
{GUCEF_TRACE;

    return m_outcome;
}

/*-------------------------------------------------------------------------*/

GUI::CPropertyInspectorForm*
CServerSettingsForm::GetPropertyList( void )
{GUCEF_TRACE;

    return m_list;
}

/*-------------------------------------------------------------------------*/

void
CServerSettingsForm::OnFrameStarted( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( GUCEF_NULL != eventData )
        Update( static_cast< CORE::TCloneableUInt32* >( eventData )->GetData() );
}

/*-------------------------------------------------------------------------*/

void
CServerSettingsForm::OnControllerRead( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    // shown at the next refresh
    m_dirty = true;
}

/*-------------------------------------------------------------------------*/

void
CServerSettingsForm::OnControllerOutcome( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    const CORE::TCloneableString* text = static_cast< const CORE::TCloneableString* >( eventData );
    if ( GUCEF_NULL != text )
        m_outcome = text->GetData();
    ShowTexts();
    m_dirty = true;
}

/*-------------------------------------------------------------------------*/

void
CServerSettingsForm::OnPropertyChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( m_rebuilding || GUCEF_NULL == eventData )
        return;
    const GUI::CPropertyInspectorForm::TPropertyChangedEventData* change = static_cast< const GUI::CPropertyInspectorForm::TPropertyChangedEventData* >( eventData );
    const SRow* row = FindRowByPath( change->GetData().path );
    if ( GUCEF_NULL == row )
        return;
    m_edits[ row->key ] = change->GetData().newValue;
    ShowTexts();
    ShowButtons();
}

/*-------------------------------------------------------------------------*/

void
CServerSettingsForm::OnGridSelectionChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( m_rebuilding )
        return;
    ShowTexts();
    ShowButtons();
}

/*-------------------------------------------------------------------------*/

void CServerSettingsForm::OnApplyActivated( CORE::CNotifier*, const CORE::CEvent&, CORE::CICloneable* )
{GUCEF_TRACE;

    ApplySelected();
}
void CServerSettingsForm::OnResetActivated( CORE::CNotifier*, const CORE::CEvent&, CORE::CICloneable* )
{GUCEF_TRACE;

    ResetSelected();
}
void CServerSettingsForm::OnRefreshActivated( CORE::CNotifier*, const CORE::CEvent&, CORE::CICloneable* )
{GUCEF_TRACE;

    RefreshNow();
}
void CServerSettingsForm::OnCloseActivated( CORE::CNotifier*, const CORE::CEvent&, CORE::CICloneable* )
{GUCEF_TRACE;

    Close();
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CServerSettingsForm::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::ASSETWORKBENCH::CServerSettingsForm";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETWORKBENCH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
