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

#ifndef GUCEF_GUI_CCOMBOBOX_H
#include "gucefGUI_CCombobox.h"
#define GUCEF_GUI_CCOMBOBOX_H
#endif /* GUCEF_GUI_CCOMBOBOX_H ? */

#ifndef GUCEF_GUI_CCHECKBOX_H
#include "gucefGUI_CCheckbox.h"
#define GUCEF_GUI_CCHECKBOX_H
#endif /* GUCEF_GUI_CCHECKBOX_H ? */

#ifndef GUCEF_GUI_CGUICONTEXT_H
#include "gucefGUI_CGUIContext.h"
#define GUCEF_GUI_CGUICONTEXT_H
#endif /* GUCEF_GUI_CGUICONTEXT_H ? */

#ifndef ASSETWORKBENCHLIB_CWORKBENCHJOBFILTER_H
#include "AssetWorkbenchLib_CWorkbenchJobFilter.h"
#define ASSETWORKBENCHLIB_CWORKBENCHJOBFILTER_H
#endif /* ASSETWORKBENCHLIB_CWORKBENCHJOBFILTER_H ? */

#ifndef ASSETWORKBENCHLIB_CBULKCONVERSIONFORM_H
#include "AssetWorkbenchLib_CBulkConversionForm.h"
#define ASSETWORKBENCHLIB_CBULKCONVERSIONFORM_H
#endif /* ASSETWORKBENCHLIB_CBULKCONVERSIONFORM_H ? */

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

const CORE::CEvent CBulkConversionForm::ClosedEvent = "GUCEF::ASSETWORKBENCH::CBulkConversionForm::ClosedEvent";
const CORE::CEvent CBulkConversionForm::TemplatesChangedEvent = "GUCEF::ASSETWORKBENCH::CBulkConversionForm::TemplatesChangedEvent";

namespace {

const char* const g_scopeTitles[] = { "Selection", "Folder", "Pack", "Group", "Saved search" };
const char* const g_planColumns = "Asset\nAction\nReason\nEstimated work";

const CORE::CString&
ScopeKindOfIndex( Int32 index )
{
    switch ( index )
    {
        case 0:  return CBulkConversionSettings::ScopeSelection;
        case 1:  return CBulkConversionSettings::ScopeFolder;
        case 3:  return CBulkConversionSettings::ScopeGroup;
        case 4:  return CBulkConversionSettings::ScopeSavedSearch;
        default: return CBulkConversionSettings::ScopePack;
    }
}

Int32
IndexOfScopeKind( const CORE::CString& kind )
{
    for ( Int32 i=0; i<5; ++i )
        if ( ScopeKindOfIndex( i ) == kind )
            return i;
    return 2;
}

}

/*-------------------------------------------------------------------------*/

void
CBulkConversionForm::RegisterEvents( void )
{GUCEF_TRACE;

    ClosedEvent.Initialize();
    TemplatesChangedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CBulkConversionForm::CBulkConversionForm( void )
    : GUI::CFormEx()
    , m_controller( GUCEF_NULL )
    , m_model( GUCEF_NULL )
    , m_preferences( GUCEF_NULL )
    , m_assets()
    , m_groupId()
    , m_planRows()
    , m_planText()
    , m_jobText()
    , m_notice()
    , m_waitedInMs( 0 )
    , m_open( false )
    , m_updatingWidgets( false )
    , m_dirty( true )
    , m_frameSubscribed( false )
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CBulkConversionForm::CBulkConversionForm( GUI::CIGUIContext* context )
    : GUI::CFormEx( context )
    , m_controller( GUCEF_NULL )
    , m_model( GUCEF_NULL )
    , m_preferences( GUCEF_NULL )
    , m_assets()
    , m_groupId()
    , m_planRows()
    , m_planText()
    , m_jobText()
    , m_notice()
    , m_waitedInMs( 0 )
    , m_open( false )
    , m_updatingWidgets( false )
    , m_dirty( true )
    , m_frameSubscribed( false )
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CBulkConversionForm::~CBulkConversionForm()
{GUCEF_TRACE;

    UnsubscribeAllFromObserver();
}

/*-------------------------------------------------------------------------*/

void
CBulkConversionForm::BuildLayout( CORE::CDataNode& layout )
{GUCEF_TRACE;

    CORE::CDataNode* window = GUI::CFormLayout::AddWidgetNode( layout, "Window", "BulkConversionForm", "0.04,0 0.04,0", "0.92,0 0.92,0" );
    GUI::CFormLayout::SetWidgetNodeProperty( *window, GUI::EWidgetProperty_Title, "Bulk conversion" );
    GUI::CFormLayout::SetWidgetNodeProperty( *window, GUI::EWidgetProperty_TitleHeight, "28" );
    CORE::CDataNode* c = GUI::CFormLayout::AddWidgetNode( *window, "Panel", "Content", "0,0 0,28", "1,0 1,-28" );

    struct SLabel { const char* name; const char* text; const char* position; const char* size; };
    const SLabel labels[] =
    {
        { "ScopeLabel", "Scope", "0,8 0,10", "0,50 0,24" }, { "PackLabel", "Pack", "0,212 0,10", "0,44 0,24" },
        { "DetailLabel", "Folder", "0,470 0,10", "0,70 0,24" }, { "ProfileLabel", "Profile", "0,462 0,46", "0,56 0,24" },
        { "FilterLabel", "Filter", "0,530 0,82", "0,44 0,24" }, { "TemplateLabel", "Template", "0,8 0,118", "0,80 0,24" }
    };
    for ( UInt32 i=0; i<6; ++i )
    {
        CORE::CDataNode* label = GUI::CFormLayout::AddWidgetNode( *c, "Label", labels[ i ].name, labels[ i ].position, labels[ i ].size );
        GUI::CFormLayout::SetWidgetNodeProperty( *label, GUI::EWidgetProperty_Text, labels[ i ].text );
    }
    GUI::CFormLayout::AddWidgetNode( *c, "Combobox", "ScopeKind", "0,60 0,6", "0,140 0,28" );
    GUI::CFormLayout::AddWidgetNode( *c, "Editbox", "Pack", "0,260 0,6", "0,200 0,28" );
    GUI::CFormLayout::AddWidgetNode( *c, "Editbox", "ScopeDetail", "0,544 0,6", "1,-554 0,28" );
    GUI::CFormLayout::AddWidgetNode( *c, "Editbox", "CookProfile", "0,520 0,42", "1,-530 0,28" );
    GUI::CFormLayout::AddWidgetNode( *c, "Editbox", "AssetFilter", "0,576 0,78", "1,-586 0,28" );

    struct SCheck { const char* name; const char* text; const char* position; const char* size; bool checked; };
    const SCheck checks[] =
    {
        { "Intermediate", "Intermediate glTF + sidecar", "0,8 0,42", "0,250 0,28", true }, { "OgreRuntime", "Ogre runtime files", "0,262 0,42", "0,190 0,28", true },
        { "Force", "Force reimport", "0,8 0,78", "0,150 0,28", false }, { "ValidateOnly", "Validate only", "0,162 0,78", "0,140 0,28", false },
        { "CookOnly", "Cook only", "0,306 0,78", "0,110 0,28", false }, { "Analyse", "Analyse", "0,420 0,78", "0,100 0,28", false }
    };
    for ( UInt32 i=0; i<6; ++i )
    {
        CORE::CDataNode* box = GUI::CFormLayout::AddWidgetNode( *c, "Checkbox", checks[ i ].name, checks[ i ].position, checks[ i ].size );
        GUI::CFormLayout::SetWidgetNodeProperty( *box, GUI::EWidgetProperty_Text, checks[ i ].text );
        GUI::CFormLayout::SetWidgetNodeProperty( *box, GUI::EWidgetProperty_Checked, checks[ i ].checked ? "true" : "false" );
    }

    GUI::CFormLayout::AddWidgetNode( *c, "Combobox", "Templates", "0,92 0,114", "0,160 0,28" );
    GUI::CFormLayout::AddWidgetNode( *c, "Editbox", "TemplateName", "0,260 0,114", "0,150 0,28" );
    struct SButton { const char* name; const char* text; const char* position; const char* size; };
    const SButton buttons[] =
    {
        { "SaveTemplate", "Save template", "0,418 0,114", "0,110 0,28" }, { "LoadTemplate", "Load", "0,532 0,114", "0,80 0,28" },
        { "DeleteTemplate", "Delete", "0,616 0,114", "0,90 0,28" },
        { "Plan", "Plan", "0,8 0,152", "0,90 0,28" }, { "Start", "Start", "0,102 0,152", "0,90 0,28" },
        { "CancelJob", "Cancel job", "0,196 0,152", "0,100 0,28" }, { "ResumeJob", "Resume", "0,300 0,152", "0,90 0,28" },
        { "RetryFailed", "Retry failed", "0,394 0,152", "0,110 0,28" }, { "Close", "Close", "1,-102 0,152", "0,94 0,28" }
    };
    for ( UInt32 i=0; i<9; ++i )
    {
        CORE::CDataNode* button = GUI::CFormLayout::AddWidgetNode( *c, "Button", buttons[ i ].name, buttons[ i ].position, buttons[ i ].size );
        GUI::CFormLayout::SetWidgetNodeProperty( *button, GUI::EWidgetProperty_Text, buttons[ i ].text );
    }

    CORE::CDataNode* grid = GUI::CFormLayout::AddWidgetNode( *c, "GridView", "PlanItems", "0,0 0,190", "1,0 1,-292" );
    GUI::CFormLayout::SetWidgetNodeProperty( *grid, GUI::EWidgetProperty_FitColumns, "true" );
    GUI::CFormLayout::SetWidgetNodeProperty( *grid, GUI::EWidgetProperty_TextOverflow, "Ellipsis" );
    GUI::CFormLayout::AddWidgetNode( *c, "Label", "PlanSummary", "0,8 1,-98", "1,-16 0,28" );
    GUI::CFormLayout::AddWidgetNode( *c, "Label", "JobStatus", "0,8 1,-66", "1,-16 0,28" );
    GUI::CFormLayout::AddWidgetNode( *c, "Label", "Notice", "0,8 1,-34", "1,-16 0,28" );
}

/*-------------------------------------------------------------------------*/

bool
CBulkConversionForm::Build( void )
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
    return true;
}

/*-------------------------------------------------------------------------*/

void
CBulkConversionForm::OnPostLayoutLoad( void )
{GUCEF_TRACE;

    GUI::CFormEx::OnPostLayoutLoad();
    struct SButton { const char* name; void ( CBulkConversionForm::*handler )( CORE::CNotifier*, const CORE::CEvent&, CORE::CICloneable* ); };
    const SButton buttons[] =
    {
        { "Plan", &CBulkConversionForm::OnPlanActivated }, { "Start", &CBulkConversionForm::OnStartActivated },
        { "CancelJob", &CBulkConversionForm::OnCancelActivated }, { "ResumeJob", &CBulkConversionForm::OnResumeActivated },
        { "RetryFailed", &CBulkConversionForm::OnRetryActivated }, { "SaveTemplate", &CBulkConversionForm::OnSaveActivated },
        { "LoadTemplate", &CBulkConversionForm::OnLoadActivated }, { "DeleteTemplate", &CBulkConversionForm::OnDeleteActivated },
        { "Close", &CBulkConversionForm::OnCloseActivated }
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
    GUI::CWidget* kind = GetWidget( "ScopeKind" );
    if ( GUCEF_NULL != kind )
    {
        TEventCallback callback( this, &CBulkConversionForm::OnSettingChanged );
        SubscribeTo( kind, GUI::CWidget::ValueChangedEvent, callback );
    }
    GUI::CGUIContext* context = GetGUIContext();
    if ( GUCEF_NULL != context && !m_frameSubscribed )
    {
        m_frameSubscribed = true;
        TEventCallback callback( this, &CBulkConversionForm::OnFrameStarted );
        SubscribeTo( context, GUI::CGUIContext::FrameStartedEvent, callback );
    }
    GUI::CGridView* grid = GetWidgetAs< GUI::CGridView >( "PlanItems" );
    if ( GUCEF_NULL != grid )
    {
        grid->SetProperty( GUI::EWidgetProperty_Columns, g_planColumns );
        GUCEF::vector< Int32 > widths;
        widths.push_back( 4 );
        widths.push_back( 1 );
        widths.push_back( 3 );
        widths.push_back( 3 );
        grid->SetIntListProperty( GUI::EWidgetProperty_ColumnWidths, widths );
    }
    ShowChoices();
    ShowTemplates();
    ShowPlan();
    ShowJob();
}

/*-------------------------------------------------------------------------*/

void
CBulkConversionForm::ShowChoices( void )
{GUCEF_TRACE;

    GUI::CCombobox* kind = GetWidgetAs< GUI::CCombobox >( "ScopeKind" );
    if ( GUCEF_NULL == kind )
        return;
    m_updatingWidgets = true;
    GUI::CCombobox::TStringVector items;
    for ( UInt32 i=0; i<5; ++i )
        items.push_back( g_scopeTitles[ i ] );
    kind->SetListItems( items );
    kind->SetSelectedIndex( 2 );
    m_updatingWidgets = false;
}

/*-------------------------------------------------------------------------*/

void
CBulkConversionForm::ShowTemplates( void )
{GUCEF_TRACE;

    GUI::CCombobox* templates = GetWidgetAs< GUI::CCombobox >( "Templates" );
    if ( GUCEF_NULL == templates )
        return;
    m_updatingWidgets = true;
    GUI::CCombobox::TStringVector items;
    items.push_back( "Templates" );
    CORE::CString::StringVector names;
    if ( GUCEF_NULL != m_preferences )
        m_preferences->GetBulkTemplateNames( names );
    for ( size_t i=0; i<names.size(); ++i )
        items.push_back( names[ i ] );
    templates->SetListItems( items );
    templates->SetSelectedIndex( 0 );
    m_updatingWidgets = false;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CBulkConversionForm::GetText( const char* widgetName ) const
{GUCEF_TRACE;

    const GUI::CWidget* widget = const_cast< CBulkConversionForm* >( this )->GetWidget( widgetName );
    return GUCEF_NULL == widget ? CORE::CString::Empty : widget->GetProperty( GUI::EWidgetProperty_Text );
}

/*-------------------------------------------------------------------------*/

bool
CBulkConversionForm::GetChecked( const char* widgetName ) const
{GUCEF_TRACE;

    const GUI::CCheckbox* box = GUI::CWidget::As< GUI::CCheckbox >( const_cast< CBulkConversionForm* >( this )->GetWidget( widgetName ) );
    return GUCEF_NULL != box && box->GetCheckedState();
}

/*-------------------------------------------------------------------------*/

void
CBulkConversionForm::SetText( const char* widgetName, const CORE::CString& text )
{GUCEF_TRACE;

    GUI::CWidget* widget = GetWidget( widgetName );
    if ( GUCEF_NULL != widget )
        widget->SetProperty( GUI::EWidgetProperty_Text, text );
}

/*-------------------------------------------------------------------------*/

void
CBulkConversionForm::SetChecked( const char* widgetName, bool checked )
{GUCEF_TRACE;

    GUI::CCheckbox* box = GUI::CWidget::As< GUI::CCheckbox >( GetWidget( widgetName ) );
    if ( GUCEF_NULL != box )
        box->SetCheckedState( checked );
}

/*-------------------------------------------------------------------------*/

void
CBulkConversionForm::SetSettings( const CBulkConversionSettings& settings )
{GUCEF_TRACE;

    m_assets = settings.assets;
    m_groupId = settings.groupId;
    m_updatingWidgets = true;
    GUI::CCombobox* kind = GetWidgetAs< GUI::CCombobox >( "ScopeKind" );
    if ( GUCEF_NULL != kind )
        kind->SetSelectedIndex( IndexOfScopeKind( settings.scopeKind ) );
    SetText( "Pack", settings.packId );
    const bool listScope = CBulkConversionSettings::ScopeSelection == settings.scopeKind || CBulkConversionSettings::ScopeGroup == settings.scopeKind;
    SetText( "ScopeDetail", CBulkConversionSettings::ScopeFolder == settings.scopeKind ? settings.folder :
                            ( CBulkConversionSettings::ScopeSavedSearch == settings.scopeKind ? settings.savedSearchId :
                            ( listScope ? CORE::UInt32ToString( (UInt32) settings.assets.size() ) + " assets" : CORE::CString::Empty ) ) );
    SetText( "DetailLabel", CBulkConversionSettings::ScopeFolder == settings.scopeKind ? "Folder" : ( CBulkConversionSettings::ScopeSavedSearch == settings.scopeKind ? "Search" : "Assets" ) );
    SetText( "CookProfile", settings.cookProfile );
    SetText( "AssetFilter", settings.assetFilter );
    SetText( "TemplateName", settings.name );
    SetChecked( "Intermediate", settings.intermediate );
    SetChecked( "OgreRuntime", settings.ogreRuntime );
    SetChecked( "Force", settings.force );
    SetChecked( "ValidateOnly", settings.validateOnly );
    SetChecked( "CookOnly", settings.cookOnly );
    SetChecked( "Analyse", settings.analyse );
    m_updatingWidgets = false;
    ShowButtons();
}

/*-------------------------------------------------------------------------*/

CBulkConversionSettings
CBulkConversionForm::GetSettings( void ) const
{GUCEF_TRACE;

    CBulkConversionSettings settings;
    const GUI::CCombobox* kind = GUI::CWidget::As< GUI::CCombobox >( const_cast< CBulkConversionForm* >( this )->GetWidget( "ScopeKind" ) );
    settings.scopeKind = ScopeKindOfIndex( GUCEF_NULL != kind ? kind->GetSelectedIndex() : 2 );
    settings.packId = GetText( "Pack" ).Trim( true ).Trim( false );
    const CORE::CString detail = GetText( "ScopeDetail" ).Trim( true ).Trim( false );
    if ( CBulkConversionSettings::ScopeFolder == settings.scopeKind )
        settings.folder = detail;
    else
    if ( CBulkConversionSettings::ScopeSavedSearch == settings.scopeKind )
        settings.savedSearchId = detail;
    settings.assets = m_assets;
    settings.groupId = m_groupId;
    settings.cookProfile = GetText( "CookProfile" ).Trim( true ).Trim( false );
    settings.assetFilter = GetText( "AssetFilter" ).Trim( true ).Trim( false );
    settings.name = GetText( "TemplateName" ).Trim( true ).Trim( false );
    settings.intermediate = GetChecked( "Intermediate" );
    settings.ogreRuntime = GetChecked( "OgreRuntime" );
    settings.force = GetChecked( "Force" );
    settings.validateOnly = GetChecked( "ValidateOnly" );
    settings.cookOnly = GetChecked( "CookOnly" );
    settings.analyse = GetChecked( "Analyse" );
    return settings;
}

/*-------------------------------------------------------------------------*/

bool
CBulkConversionForm::Open( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == GetRootWidget() && !Build() )
        return false;
    if ( !Show() )
        return false;
    m_open = true;
    ShowTemplates();
    ShowButtons();
    return true;
}

/*-------------------------------------------------------------------------*/

void
CBulkConversionForm::Close( void )
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
CBulkConversionForm::IsOpen( void ) const
{GUCEF_TRACE;

    return m_open;
}

/*-------------------------------------------------------------------------*/

void
CBulkConversionForm::SetController( CBulkConversionController* controller )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_controller )
    {
        UnsubscribeFrom( m_controller, CBulkConversionController::PlanReadyEvent );
        UnsubscribeFrom( m_controller, CBulkConversionController::NotificationEvent );
    }
    m_controller = controller;
    if ( GUCEF_NULL != m_controller )
    {
        TEventCallback planCallback( this, &CBulkConversionForm::OnPlanReady );
        SubscribeTo( m_controller, CBulkConversionController::PlanReadyEvent, planCallback );
        TEventCallback noticeCallback( this, &CBulkConversionForm::OnNotification );
        SubscribeTo( m_controller, CBulkConversionController::NotificationEvent, noticeCallback );
    }
}

/*-------------------------------------------------------------------------*/

void
CBulkConversionForm::SetModel( CAssetStoreTransferModel* model )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_model )
        UnsubscribeFrom( m_model, GUI::CITransferQueueModel::EntriesChangedEvent );
    m_model = model;
    if ( GUCEF_NULL != m_model )
    {
        TEventCallback callback( this, &CBulkConversionForm::OnModelChanged );
        SubscribeTo( m_model, GUI::CITransferQueueModel::EntriesChangedEvent, callback );
    }
    m_dirty = true;
}

/*-------------------------------------------------------------------------*/

void
CBulkConversionForm::SetPreferences( CWorkbenchPreferences* preferences )
{GUCEF_TRACE;

    m_preferences = preferences;
    ShowTemplates();
}

/*-------------------------------------------------------------------------*/

bool
CBulkConversionForm::PlanNow( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_controller )
        return false;
    m_notice.Clear();
    m_planRows.clear();
    const bool asked = m_controller->RequestPlan( GetSettings() );
    if ( !asked )
        m_planText = m_controller->GetProblem();
    else
        m_planText = m_controller->IsPlanning() ? CORE::CString( "Planning..." ) : m_planText;
    if ( asked && !m_controller->IsPlanning() )
    {
        ShowPlan();
        ShowButtons();
    }
    else
        ShowPlan();
    return asked;
}

/*-------------------------------------------------------------------------*/

bool
CBulkConversionForm::StartNow( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_controller )
        return false;
    const UInt32 started = m_controller->Start();
    m_notice = started > 0 ? CORE::UInt32ToString( started ) + " job(s) started: " + m_controller->GetPlannedSettings().Describe() : m_controller->GetProblem();
    ShowJob();
    ShowButtons();
    return started > 0;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CBulkConversionForm::GetCurrentJobId( void ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_controller || m_controller->GetJobIds().empty() )
        return CORE::CString::Empty;
    return m_controller->GetJobIds().back();
}

/*-------------------------------------------------------------------------*/

bool
CBulkConversionForm::CancelJob( void )
{GUCEF_TRACE;

    const CORE::CString jobId = GetCurrentJobId();
    return !jobId.IsNULLOrEmpty() && m_controller->CancelJob( jobId );
}

/*-------------------------------------------------------------------------*/

bool
CBulkConversionForm::ResumeJob( void )
{GUCEF_TRACE;

    const CORE::CString jobId = GetCurrentJobId();
    return !jobId.IsNULLOrEmpty() && m_controller->ResumeJob( jobId );
}

/*-------------------------------------------------------------------------*/

bool
CBulkConversionForm::RetryFailed( void )
{GUCEF_TRACE;

    const CORE::CString jobId = GetCurrentJobId();
    return !jobId.IsNULLOrEmpty() && m_controller->RetryFailedOfJob( jobId );
}

/*-------------------------------------------------------------------------*/

bool
CBulkConversionForm::SaveTemplate( const CORE::CString& name )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_preferences || name.IsNULLOrEmpty() )
        return false;
    CBulkConversionSettings settings = GetSettings();
    settings.name = name;
    if ( !m_preferences->SetBulkTemplate( settings ) )
        return false;
    ShowTemplates();
    NotifyObservers( TemplatesChangedEvent );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CBulkConversionForm::LoadTemplate( const CORE::CString& name )
{GUCEF_TRACE;

    CBulkConversionSettings settings;
    if ( GUCEF_NULL == m_preferences || !m_preferences->GetBulkTemplate( name, settings ) )
        return false;
    SetSettings( settings );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CBulkConversionForm::DeleteTemplate( const CORE::CString& name )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_preferences || !m_preferences->RemoveBulkTemplate( name ) )
        return false;
    ShowTemplates();
    NotifyObservers( TemplatesChangedEvent );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CBulkConversionForm::ShowPlan( void )
{GUCEF_TRACE;

    m_planRows.clear();
    if ( GUCEF_NULL != m_controller && m_controller->GetPlan().valid )
    {
        const CBulkConversionPlan& plan = m_controller->GetPlan();
        for ( size_t i=0; i<plan.items.size(); ++i )
        {
            GUCEF::vector< CORE::CString > row;
            row.push_back( plan.items[ i ].assetPath );
            row.push_back( plan.items[ i ].action );
            row.push_back( plan.items[ i ].reason );
            row.push_back( CBulkConversionPlan::DescribeWork( plan.items[ i ] ) );
            m_planRows.push_back( row );
        }
        m_planText = plan.ToSummaryText();
        if ( plan.totalItems > plan.items.size() )
            m_planText += " (the first " + CORE::UInt32ToString( (UInt32) plan.items.size() ) + " of " + CORE::UInt64ToString( plan.totalItems ) + " assets are listed)";
        if ( !plan.restrictions.empty() )
            m_planText += "; licence: " + plan.restrictions[ 0 ];
    }
    else
    if ( GUCEF_NULL != m_controller && !m_controller->IsPlanning() && !m_controller->GetPlan().conversionAllowed )
        m_planText = m_controller->GetPlan().ToSummaryText();
    GUI::CGridView* grid = GetWidgetAs< GUI::CGridView >( "PlanItems" );
    if ( GUCEF_NULL != grid )
    {
        GUI::CGridView::TRowVector rows;
        for ( size_t r=0; r<m_planRows.size(); ++r )
        {
            GUI::CGridView::TStringVector cells;
            for ( size_t c=0; c<m_planRows[ r ].size(); ++c )
                cells.push_back( m_planRows[ r ][ c ].ReplaceChar( '\n', ' ' ).ReplaceChar( '\t', ' ' ) );
            rows.push_back( cells );
        }
        grid->SetRows( rows );
    }
    SetText( "PlanSummary", m_planText );
}

/*-------------------------------------------------------------------------*/

void
CBulkConversionForm::ShowJob( void )
{GUCEF_TRACE;

    m_dirty = false;
    m_waitedInMs = 0;
    m_jobText.Clear();
    const CORE::CString jobId = GetCurrentJobId();
    if ( !jobId.IsNULLOrEmpty() )
    {
        ASSETSTORE::CAssetJobEntry job;
        if ( GUCEF_NULL != m_model && m_model->GetJob( jobId, job ) )
        {
            m_jobText = "Job " + job.jobId + ": " + CWorkbenchJobFilter::StateTextOf( job );
            if ( job.itemsTotal > 0 )
                m_jobText += ", " + CORE::UInt32ToString( job.itemsDone ) + " of " + CORE::UInt32ToString( job.itemsTotal ) + " assets";
            if ( job.IsEnded() && !job.resultText.IsNULLOrEmpty() )
                m_jobText += ": " + job.resultText;
            else
            if ( !job.IsEnded() && !job.message.IsNULLOrEmpty() )
                m_jobText += " (" + job.message + ")";
        }
        else
            m_jobText = "Job " + jobId;
    }
    SetText( "JobStatus", m_jobText );
    SetText( "Notice", m_notice );
    ShowButtons();
}

/*-------------------------------------------------------------------------*/

bool
CBulkConversionForm::IsStartEnabled( void ) const
{GUCEF_TRACE;

    return GUCEF_NULL != m_controller && m_controller->CanStart();
}

/*-------------------------------------------------------------------------*/

void
CBulkConversionForm::ShowButtons( void )
{GUCEF_TRACE;

    GUI::CWidget* start = GetWidget( "Start" );
    if ( GUCEF_NULL != start )
        start->SetBoolProperty( GUI::EWidgetProperty_Enabled, IsStartEnabled() );
    const CORE::CString jobId = GetCurrentJobId();
    ASSETSTORE::CAssetJobEntry job;
    const bool known = !jobId.IsNULLOrEmpty() && GUCEF_NULL != m_model && m_model->GetJob( jobId, job );
    GUI::CWidget* cancel = GetWidget( "CancelJob" );
    if ( GUCEF_NULL != cancel )
        cancel->SetBoolProperty( GUI::EWidgetProperty_Enabled, known && job.IsActionAvailable( ASSETSTORE::ASSETJOB_ACTION_CANCEL ) );
    GUI::CWidget* resume = GetWidget( "ResumeJob" );
    if ( GUCEF_NULL != resume )
        resume->SetBoolProperty( GUI::EWidgetProperty_Enabled, known && job.IsActionAvailable( ASSETSTORE::ASSETJOB_ACTION_RESUME ) );
    GUI::CWidget* retry = GetWidget( "RetryFailed" );
    if ( GUCEF_NULL != retry )
        retry->SetBoolProperty( GUI::EWidgetProperty_Enabled, known && job.IsActionAvailable( ASSETSTORE::ASSETJOB_ACTION_RETRY ) );
}

/*-------------------------------------------------------------------------*/

bool
CBulkConversionForm::Update( UInt32 deltaInMs )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_controller )
        m_controller->Update();
    m_waitedInMs += deltaInMs;
    if ( m_open && ( m_dirty || m_waitedInMs >= (UInt32) RefreshIntervalInMs ) && m_waitedInMs >= (UInt32) RefreshIntervalInMs )
    {
        ShowJob();
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

CORE::CString CBulkConversionForm::GetPlanSummaryText( void ) const
{GUCEF_TRACE;

    return m_planText;
}
CORE::CString CBulkConversionForm::GetJobStatusText( void ) const
{GUCEF_TRACE;

    return m_jobText;
}
CORE::CString CBulkConversionForm::GetNoticeText( void ) const
{GUCEF_TRACE;

    return m_notice;
}
UInt32 CBulkConversionForm::GetPlanRowCount( void ) const
{GUCEF_TRACE;

    return (UInt32) m_planRows.size();
}

/*-------------------------------------------------------------------------*/

CORE::CString
CBulkConversionForm::GetPlanCellText( UInt32 row, UInt32 column ) const
{GUCEF_TRACE;

    return row < (UInt32) m_planRows.size() && column < (UInt32) m_planRows[ row ].size() ? m_planRows[ row ][ column ] : CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

void
CBulkConversionForm::OnFrameStarted( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( GUCEF_NULL != eventData )
        Update( static_cast< CORE::TCloneableUInt32* >( eventData )->GetData() );
}

/*-------------------------------------------------------------------------*/

void
CBulkConversionForm::OnPlanReady( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    ShowPlan();
    if ( GUCEF_NULL != m_controller && !m_controller->GetProblem().IsNULLOrEmpty() && !m_controller->GetPlan().valid )
        SetText( "PlanSummary", m_controller->GetProblem() );
    ShowButtons();
}

/*-------------------------------------------------------------------------*/

void
CBulkConversionForm::OnNotification( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    const CORE::TCloneableString* text = static_cast< const CORE::TCloneableString* >( eventData );
    if ( GUCEF_NULL != text )
        m_notice = text->GetData();
    m_dirty = true;
    ShowJob();
}

/*-------------------------------------------------------------------------*/

void
CBulkConversionForm::OnModelChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    m_dirty = true;
}

/*-------------------------------------------------------------------------*/

void
CBulkConversionForm::OnSettingChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( m_updatingWidgets )
        return;
    const CORE::CString kind = GetSettings().scopeKind;
    SetText( "DetailLabel", CBulkConversionSettings::ScopeFolder == kind ? "Folder" : ( CBulkConversionSettings::ScopeSavedSearch == kind ? "Search" : "Assets" ) );
}

/*-------------------------------------------------------------------------*/

void
CBulkConversionForm::OnPlanActivated( CORE::CNotifier*, const CORE::CEvent&, CORE::CICloneable* )
{GUCEF_TRACE;

    PlanNow();
}

/*-------------------------------------------------------------------------*/

void CBulkConversionForm::OnStartActivated( CORE::CNotifier*, const CORE::CEvent&, CORE::CICloneable* )
{GUCEF_TRACE;

    StartNow();
}
void CBulkConversionForm::OnCancelActivated( CORE::CNotifier*, const CORE::CEvent&, CORE::CICloneable* )
{GUCEF_TRACE;

    CancelJob();
}
void CBulkConversionForm::OnResumeActivated( CORE::CNotifier*, const CORE::CEvent&, CORE::CICloneable* )
{GUCEF_TRACE;

    ResumeJob();
}
void CBulkConversionForm::OnRetryActivated( CORE::CNotifier*, const CORE::CEvent&, CORE::CICloneable* )
{GUCEF_TRACE;

    RetryFailed();
}
void CBulkConversionForm::OnCloseActivated( CORE::CNotifier*, const CORE::CEvent&, CORE::CICloneable* )
{GUCEF_TRACE;

    Close();
}

/*-------------------------------------------------------------------------*/

void
CBulkConversionForm::OnSaveActivated( CORE::CNotifier*, const CORE::CEvent&, CORE::CICloneable* )
{GUCEF_TRACE;

    SaveTemplate( GetText( "TemplateName" ).Trim( true ).Trim( false ) );
}

/*-------------------------------------------------------------------------*/

void
CBulkConversionForm::OnLoadActivated( CORE::CNotifier*, const CORE::CEvent&, CORE::CICloneable* )
{GUCEF_TRACE;

    const GUI::CCombobox* templates = GetWidgetAs< GUI::CCombobox >( "Templates" );
    CORE::CString::StringVector names;
    if ( GUCEF_NULL != m_preferences )
        m_preferences->GetBulkTemplateNames( names );
    const Int32 index = GUCEF_NULL != templates ? templates->GetSelectedIndex() : 0;
    if ( index > 0 && index <= (Int32) names.size() )
        LoadTemplate( names[ index - 1 ] );
}

/*-------------------------------------------------------------------------*/

void
CBulkConversionForm::OnDeleteActivated( CORE::CNotifier*, const CORE::CEvent&, CORE::CICloneable* )
{GUCEF_TRACE;

    const GUI::CCombobox* templates = GetWidgetAs< GUI::CCombobox >( "Templates" );
    CORE::CString::StringVector names;
    if ( GUCEF_NULL != m_preferences )
        m_preferences->GetBulkTemplateNames( names );
    const Int32 index = GUCEF_NULL != templates ? templates->GetSelectedIndex() : 0;
    if ( index > 0 && index <= (Int32) names.size() )
        DeleteTemplate( names[ index - 1 ] );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CBulkConversionForm::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::ASSETWORKBENCH::CBulkConversionForm";
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
