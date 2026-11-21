/*
 *  AssetWorkbenchLib: the settings layer of the workbench
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

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETSTOREREGISTRY_H
#include "gucefASSETSTORE_CAssetStoreRegistry.h"
#define GUCEF_ASSETSTORE_CASSETSTOREREGISTRY_H
#endif /* GUCEF_ASSETSTORE_CASSETSTOREREGISTRY_H ? */

#ifndef GUCEF_GUI_CGUIINPUTADAPTER_H
#include "gucefGUI_CGUIInputAdapter.h"
#define GUCEF_GUI_CGUIINPUTADAPTER_H
#endif /* GUCEF_GUI_CGUIINPUTADAPTER_H ? */

#ifndef GUCEF_GUI_CTABCONTROL_H
#include "gucefGUI_CTabControl.h"
#define GUCEF_GUI_CTABCONTROL_H
#endif /* GUCEF_GUI_CTABCONTROL_H ? */

#ifndef GUCEF_GUI_CGUIGLOBAL_H
#include "gucefGUI_CGuiGlobal.h"
#define GUCEF_GUI_CGUIGLOBAL_H
#endif /* GUCEF_GUI_CGUIGLOBAL_H ? */

#ifndef GUCEF_AUTHGUI_CAUTHGUIGLOBAL_H
#include "gucefAUTHGUI_CAuthGuiGlobal.h"
#define GUCEF_AUTHGUI_CAUTHGUIGLOBAL_H
#endif /* GUCEF_AUTHGUI_CAUTHGUIGLOBAL_H ? */

#include "AssetWorkbenchLib_CWorkbenchSettingsLayer.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETWORKBENCH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CEvent CWorkbenchSettingsLayer::SettingsAppliedEvent = "GUCEF::ASSETWORKBENCH::CWorkbenchSettingsLayer::SettingsAppliedEvent";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CWorkbenchSettingsLayer::RegisterEvents( void )
{GUCEF_TRACE;

    SettingsAppliedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CWorkbenchSettingsLayer::CWorkbenchSettingsLayer( CWorkbenchShell& shell, CWorkbenchPreferences& preferences )
    : CORE::CObservingNotifier()
    , m_shell( shell )
    , m_preferences( preferences )
    , m_context( GUCEF_NULL )
    , m_form( GUCEF_NULL )
    , m_videoPage( GUCEF_NULL )
    , m_generalPage( GUCEF_NULL )
    , m_audioPage( GUCEF_NULL )
    , m_inputPage( GUCEF_NULL )
    , m_authPage( GUCEF_NULL )
    , m_sourcesPage( GUCEF_NULL )
    , m_remotePage( GUCEF_NULL )
    , m_console( GUCEF_NULL )
    , m_commands( GUCEF_NULL )
    , m_helpViewer( GUCEF_NULL )
    , m_help()
    , m_tooltips()
    , m_video()
    , m_general()
    , m_audio()
    , m_pluginReport()
    , m_pluginHost( GUCEF_NULL )
{GUCEF_TRACE;

    RegisterEvents();
    // the developer console is a tool of the workbench: on until the owner switches it off
    m_general.SetDeveloperConsole( true );
}

/*-------------------------------------------------------------------------*/

CWorkbenchSettingsLayer::~CWorkbenchSettingsLayer()
{GUCEF_TRACE;

    UnsubscribeAllFromObserver();
    m_shell.SetDeveloperConsole( GUCEF_NULL );
    if ( GUCEF_NULL != m_context )
        m_context->GetTooltipService().SetTextProvider( GUCEF_NULL );
    // the commands go before the console they observe, the pages before the form that holds them
    GUCEF_DELETE m_commands;
    m_commands = GUCEF_NULL;
    GUCEF_DELETE m_console;
    m_console = GUCEF_NULL;
    if ( GUCEF_NULL != m_authPage )
        m_authPage->SetSignIn( GUCEF_NULL );
    if ( GUCEF_NULL != m_sourcesPage )
        m_sourcesPage->SetSignIn( GUCEF_NULL );
    if ( GUCEF_NULL != m_remotePage )
        m_remotePage->SetConnection( GUCEF_NULL );
    GUCEF_DELETE m_form;
    m_form = GUCEF_NULL;
    // the form lets go of its pages; the pages the application made are the application's
    GUCEF_DELETE m_generalPage;
    m_generalPage = GUCEF_NULL;
    GUCEF_DELETE m_inputPage;
    m_inputPage = GUCEF_NULL;
    GUCEF_DELETE m_authPage;
    m_authPage = GUCEF_NULL;
    GUCEF_DELETE m_remotePage;
    m_remotePage = GUCEF_NULL;
    GUCEF_DELETE m_sourcesPage;
    m_sourcesPage = GUCEF_NULL;
    // the help window belongs to the context, which deletes its forms
    m_helpViewer = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchSettingsLayer::Create( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_form )
        return true;
    m_context = m_shell.GetGUIContext();
    if ( GUCEF_NULL == m_context )
        return false;

    // the tooltips and the help of the whole window
    GUI::CTooltipService& tooltips = m_context->GetTooltipService();
    tooltips.SetTextProvider( &m_tooltips );
    tooltips.SetHelpSource( m_help.GetSource() );
    tooltips.SetMoreHint( "F1: more about this" );
    tooltips.SetEnabled( true );

    GUCEF::AUTHGUI::CAuthGuiGlobal::Instance();
    m_form = GUCEF_NEW GUI::CSettingsForm( m_context );
    m_form->SetTitle( "Settings of the Asset Workbench" );
    // the general page is the one of the workbench (the shared options and its own group); the form takes it first, so it stays the first tab
    m_generalPage = GUCEF_NEW CWorkbenchGeneralSettingsForm();
    m_generalPage->SetPreferences( &m_preferences );
    m_shell.SetPreferences( &m_preferences );
    if ( !m_form->AddPage( m_generalPage ) )
    {
        GUCEF_DELETE m_generalPage;
        m_generalPage = GUCEF_NULL;
    }
    m_videoPage = static_cast< GUI::CVideoSettingsForm* >( m_form->AddPage( "Video" ) );
    m_audioPage = static_cast< GUI::CAudioSettingsForm* >( m_form->AddPage( "Audio" ) );
    m_inputPage = GUCEF_NEW GUI::CInputSettingsForm();
    m_authPage = GUCEF_NEW CWorkbenchAuthSettingsForm();
    m_sourcesPage = GUCEF_NEW CWorkbenchSourcesSettingsForm();
    m_remotePage = GUCEF_NEW CWorkbenchRemoteStoreForm();
    if ( GUCEF_NULL == m_generalPage || GUCEF_NULL == m_videoPage || GUCEF_NULL == m_audioPage )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "CWorkbenchSettingsLayer: The standard pages of the settings are not registered (is gucefGUI started?)" );
        GUCEF_DELETE m_inputPage;       m_inputPage = GUCEF_NULL;
        GUCEF_DELETE m_authPage;        m_authPage = GUCEF_NULL;
        GUCEF_DELETE m_remotePage;      m_remotePage = GUCEF_NULL;
        GUCEF_DELETE m_sourcesPage;     m_sourcesPage = GUCEF_NULL;
        return false;
    }

    // the keys of the shell: the actions with their shipped keys; the page edits the map of the shell
    UInt32 count = 0;
    const CWorkbenchShell::TActionInfo* actions = CWorkbenchShell::GetActionTable( count );
    m_inputPage->SetShortcutMap( &m_shell.GetShortcuts() );
    for ( UInt32 i=0; i<count; ++i )
        m_inputPage->AddAction( actions[ i ].actionId, actions[ i ].title, actions[ i ].defaultKey, actions[ i ].defaultModifiers );
    m_sourcesPage->SetPreferences( &m_preferences );
    m_remotePage->SetPreferences( &m_preferences );

    if ( !m_form->AddPage( m_inputPage ) || !m_form->AddPage( m_authPage ) || !m_form->AddPage( m_sourcesPage ) || !m_form->AddPage( m_remotePage ) || !m_form->Build() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "CWorkbenchSettingsLayer: The settings window cannot be made" );
        return false;
    }
    m_form->GetRootWidget()->SetProperty( GUI::EWidgetProperty_HelpTopic, CWorkbenchHelp::TopicSettings );

    TEventCallback formApplied( this, &CWorkbenchSettingsLayer::OnFormSettingsApplied );
    SubscribeTo( m_form, GUI::CSettingsForm::SettingsAppliedEvent, formApplied );
    TEventCallback generalApplied( this, &CWorkbenchSettingsLayer::OnGeneralApplied );
    SubscribeTo( m_generalPage, GUI::CSettingsPageForm::SettingsAppliedEvent, generalApplied );
    TEventCallback clearThumbnails( this, &CWorkbenchSettingsLayer::OnClearAutomaticThumbnails );
    SubscribeTo( m_generalPage, CWorkbenchGeneralSettingsForm::ClearAutomaticThumbnailsRequestedEvent, clearThumbnails );
    TEventCallback pluginsRequested( this, &CWorkbenchSettingsLayer::OnPluginsLoadRequested );
    SubscribeTo( m_sourcesPage, CWorkbenchSourcesSettingsForm::PluginsLoadRequestedEvent, pluginsRequested );
    GUI::CWidget* root = m_form->GetRootWidget();
    if ( GUCEF_NULL != root )
    {
        TEventCallback keyDown( this, &CWorkbenchSettingsLayer::OnFormKeyDown );
        SubscribeTo( root, GUI::CWidget::KeyDownEvent, keyDown );
    }

    // the developer console over the window: it is fed with the keys that reach the GUI, so that a text field keeps its character
    m_console = GUCEF_NEW GUI::CDeveloperConsoleForm( m_context );
    if ( !m_console->Build() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "CWorkbenchSettingsLayer: The developer console cannot be made" );
        return false;
    }
    m_console->SetWatchesInputDevices( false );
    m_console->SetConsoleKey( m_general.GetDeveloperConsoleKey() );
    m_console->SetEnabled( m_general.GetDeveloperConsole() );
    m_generalPage->SetDeveloperConsole( m_console );
    m_shell.SetDeveloperConsole( m_console );
    m_commands = GUCEF_NEW CWorkbenchConsoleCommands( m_shell, *m_console );
    m_console->GetRootWidget()->SetProperty( GUI::EWidgetProperty_HelpTopic, CWorkbenchHelp::TopicConsole );

    // the keys the owner changed, and what the shell does for the actions of the layer
    m_preferences.ApplyShortcuts( m_shell.GetShortcuts() );
    TEventCallback shellAction( this, &CWorkbenchSettingsLayer::OnShellAction );
    SubscribeTo( &m_shell, CWorkbenchShell::ActionRequestedEvent, shellAction );
    SyncConsoleKeyToShortcuts();
    UpdateShortcutHelp();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchSettingsLayer::IsCreated( void ) const
{GUCEF_TRACE;

    return GUCEF_NULL != m_form;
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchSettingsLayer::SetVideoSettings( const GUI::CVideoSettings& settings )
{GUCEF_TRACE;

    m_video = settings;
}

/*-------------------------------------------------------------------------*/

const GUI::CVideoSettings&
CWorkbenchSettingsLayer::GetVideoSettings( void ) const
{GUCEF_TRACE;

    return m_video;
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchSettingsLayer::SetGeneralSettings( const GUI::CGeneralSettings& settings )
{GUCEF_TRACE;

    m_general = settings;
}

/*-------------------------------------------------------------------------*/

const GUI::CGeneralSettings&
CWorkbenchSettingsLayer::GetGeneralSettings( void ) const
{GUCEF_TRACE;

    return m_general;
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchSettingsLayer::SetAudioSettings( const AUDIO::CAudioSettings& settings )
{GUCEF_TRACE;

    m_audio = settings;
}

/*-------------------------------------------------------------------------*/

const AUDIO::CAudioSettings&
CWorkbenchSettingsLayer::GetAudioSettings( void ) const
{GUCEF_TRACE;

    return m_audio;
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchSettingsLayer::SetRenderers( const CORE::CString::StringVector& rendererNames, const CORE::CString& rendererInUse )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_videoPage )
    {
        GUI::CVideoSettingsForm::TStringVector names( rendererNames.begin(), rendererNames.end() );
        m_videoPage->SetAvailableRenderers( names, rendererInUse );
    }
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchSettingsLayer::SetWindow( const CORE::CString& windowManagerBackendName, GUI::TWindowContextPtr window )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_videoPage )
        m_videoPage->SetWindow( windowManagerBackendName, window );
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchSettingsLayer::ApplyGeneralSettings( void )
{GUCEF_TRACE;

    GUI::CGeneralSettingsForm::ApplyGeneralSettings( m_general, m_console );
    SyncConsoleKeyToShortcuts();
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchSettingsLayer::SyncConsoleKeyToShortcuts( void )
{GUCEF_TRACE;

    // the key of the console is a setting of the general settings; the map lists it with the other keys
    if ( INPUT::KEYCODE_UNASSIGNED != m_general.GetDeveloperConsoleKey() )
        m_shell.GetShortcuts().Bind( CWorkbenchShell::ActionConsole, m_general.GetDeveloperConsoleKey(), 0 );
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchSettingsLayer::SyncShortcutsToConsoleKey( void )
{GUCEF_TRACE;

    INPUT::KeyCode key = INPUT::KEYCODE_UNASSIGNED;
    UInt32 modifiers = 0;
    if ( m_shell.GetShortcuts().GetChord( CWorkbenchShell::ActionConsole, key, modifiers ) )
    {
        m_general.SetDeveloperConsoleKey( key );
        if ( GUCEF_NULL != m_console )
            m_console->SetConsoleKey( key );
    }
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchSettingsLayer::PrepareOpen( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_form )
        return false;
    m_videoPage->SetVideoSettings( m_video );
    m_generalPage->SetGeneralSettings( m_general );
    m_audioPage->SetAudioSettings( m_audio );
    BindSources();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchSettingsLayer::Open( const CORE::CString& pageTitle )
{GUCEF_TRACE;

    if ( IsOpen() || !PrepareOpen() )
        return false;
    return pageTitle.IsNULLOrEmpty() ? m_form->Open() : m_form->Open( pageTitle );
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchSettingsLayer::IsOpen( void ) const
{GUCEF_TRACE;

    return GUCEF_NULL != m_form && m_form->IsVisible();
}

/*-------------------------------------------------------------------------*/

GUI::CSettingsForm* CWorkbenchSettingsLayer::GetForm( void ) { return m_form; }
GUI::CVideoSettingsForm* CWorkbenchSettingsLayer::GetVideoPage( void ) { return m_videoPage; }
CWorkbenchGeneralSettingsForm* CWorkbenchSettingsLayer::GetGeneralPage( void ) { return m_generalPage; }
GUI::CAudioSettingsForm* CWorkbenchSettingsLayer::GetAudioPage( void ) { return m_audioPage; }
GUI::CInputSettingsForm* CWorkbenchSettingsLayer::GetInputPage( void ) { return m_inputPage; }
CWorkbenchAuthSettingsForm* CWorkbenchSettingsLayer::GetAuthPage( void ) { return m_authPage; }
CWorkbenchSourcesSettingsForm* CWorkbenchSettingsLayer::GetSourcesPage( void ) { return m_sourcesPage; }
CWorkbenchRemoteStoreForm* CWorkbenchSettingsLayer::GetRemotePage( void ) { return m_remotePage; }
GUI::CDeveloperConsoleForm* CWorkbenchSettingsLayer::GetConsole( void ) { return m_console; }
CWorkbenchConsoleCommands* CWorkbenchSettingsLayer::GetCommands( void ) { return m_commands; }
CWorkbenchHelp& CWorkbenchSettingsLayer::GetHelp( void ) { return m_help; }
CWorkbenchTooltips& CWorkbenchSettingsLayer::GetTooltips( void ) { return m_tooltips; }
GUI::CHelpViewerForm* CWorkbenchSettingsLayer::GetHelpViewer( void ) const { return m_helpViewer; }
const ASSETSTORE::CAssetStorePluginLoadReport& CWorkbenchSettingsLayer::GetPluginReport( void ) const { return m_pluginReport; }

/*-------------------------------------------------------------------------*/

bool
CWorkbenchSettingsLayer::ToggleConsole( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_console )
        return false;
    // the menu entry works also while the console is switched off in the settings: it is the owner asking for it
    if ( !m_console->IsEnabled() )
        m_console->SetEnabled( true );
    return m_console->Toggle();
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchSettingsLayer::OpenHelp( const CORE::CString& topicId )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_context )
        return false;
    if ( GUCEF_NULL == m_helpViewer )
    {
        // made in the context of the shell and handed to it: the context deletes its forms before its widgets
        m_helpViewer = GUCEF_NEW GUI::CHelpViewerForm( m_context );
        m_context->AdoptForm( m_helpViewer );
        m_helpViewer->SetHelpSource( m_help.GetSource() );
        if ( !m_helpViewer->Build( "Help of the Asset Workbench" ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CWorkbenchSettingsLayer: The help window cannot be made" );
            return false;
        }
    }
    UpdateShortcutHelp();
    const CORE::CString topic = m_help.GetSource()->HasTopic( topicId ) ? topicId : CWorkbenchHelp::TopicOverview;
    GUCEF_USER_LOG( CORE::LOGLEVEL_NORMAL, "Help: opened at " << topic );
    return m_helpViewer->Open( topic );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CWorkbenchSettingsLayer::GetHelpTopicForFocus( void ) const
{GUCEF_TRACE;

    if ( IsOpen() )
        return CWorkbenchHelp::TopicSettings;
    return m_help.FindTopicOfWidget( GUCEF_NULL != m_context ? m_context->GetFocus() : GUCEF_NULL );
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchSettingsLayer::OpenHelpForFocus( void )
{GUCEF_TRACE;

    return OpenHelp( GetHelpTopicForFocus() );
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchSettingsLayer::UpdateShortcutHelp( void )
{GUCEF_TRACE;

    UInt32 count = 0;
    const CWorkbenchShell::TActionInfo* actions = CWorkbenchShell::GetActionTable( count );
    CORE::CString::StringVector titles;
    CORE::CString::StringVector chords;
    for ( UInt32 i=0; i<count; ++i )
    {
        const CORE::CString chord = m_shell.GetShortcuts().GetShortcutText( actions[ i ].actionId );
        titles.push_back( actions[ i ].title );
        chords.push_back( chord.IsNULLOrEmpty() ? CORE::CString( "(none)" ) : chord );
    }
    m_help.SetShortcutRows( titles, chords );
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchSettingsLayer::SetPluginHost( ASSETSTORE::CIAssetStorePluginHost* host )
{GUCEF_TRACE;

    m_pluginHost = host;
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchSettingsLayer::LoadPrivatePlugins( void )
{GUCEF_TRACE;

    return LoadPrivatePlugins( m_preferences.privatePluginsDir );
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchSettingsLayer::LoadPrivatePlugins( const CORE::CString& directory )
{GUCEF_TRACE;

    ASSETSTORE::CAssetStorePluginLoader loader( m_pluginHost );
    const bool clean = loader.LoadFromDirectory( directory, m_pluginReport );
    CORE::CString::StringVector lines;
    m_pluginReport.GetLines( lines );
    for ( size_t i=0; i<lines.size(); ++i )
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "CWorkbenchSettingsLayer: " << lines[ i ] );
    if ( GUCEF_NULL != m_sourcesPage )
        m_sourcesPage->SetPluginReport( m_pluginReport );
    // the stores of the plugins that were loaded show in the sources tab
    CAssetSourcesPane* sources = m_shell.GetSources();
    if ( GUCEF_NULL != sources && !m_pluginReport.loaded.empty() )
        sources->RefreshStores();
    return clean;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CWorkbenchSettingsLayer::GetServiceNote( void ) const
{GUCEF_TRACE;

    CAssetSourcesPane* sources = m_shell.GetSources();
    return GUCEF_NULL != sources ? sources->GetServiceStateText() : CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchSettingsLayer::BindSources( void )
{GUCEF_TRACE;

    CWorkbenchSignIn* signIn = m_shell.GetSignIn();
    if ( GUCEF_NULL != m_authPage )
        m_authPage->SetSignIn( signIn );
    if ( GUCEF_NULL != m_sourcesPage )
    {
        m_sourcesPage->SetSignIn( signIn );
        m_sourcesPage->SetServiceNote( GetServiceNote() );
    }
    if ( GUCEF_NULL != m_remotePage )
        m_remotePage->SetConnection( m_shell.GetStoreConnection() );
    CAssetSourcesPane* sources = m_shell.GetSources();
    if ( GUCEF_NULL != sources )
        sources->SetDisabledStores( m_preferences.disabledStores );
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchSettingsLayer::OnShellAction( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    const CORE::TCloneableString* action = static_cast< const CORE::TCloneableString* >( eventData );
    if ( GUCEF_NULL == action )
        return;
    const CORE::CString& id = action->GetData();
    if ( CWorkbenchShell::ActionSettings == id )
    {
        GUCEF_USER_LOG( CORE::LOGLEVEL_NORMAL, "Settings: opened" );
        Open();
    }
    else
    if ( CWorkbenchShell::ActionHelp == id )
        OpenHelpForFocus();
    else
    if ( CWorkbenchShell::ActionConsole == id )
        ToggleConsole();
    else
    if ( CWorkbenchShell::ActionConnectStore == id )
    {
        GUCEF_USER_LOG( CORE::LOGLEVEL_NORMAL, "Settings: opened at the page Remote store" );
        Open( CWorkbenchRemoteStoreForm::PageTitle );
    }
    else
    if ( 0 == id.HasSubstr( "Help:" ) )
        OpenHelp( id.SubstrToIndex( 5, false ) );
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchSettingsLayer::OnFormSettingsApplied( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    // what the pages applied is read back: the video and audio pages keep their settings, the general page applied its own
    m_video = m_videoPage->GetVideoSettings();
    m_general = m_generalPage->GetGeneralSettings();
    m_audio = m_audioPage->GetAudioSettings();

    // the key of the console is one of the keys of the input page
    SyncShortcutsToConsoleKey();
    ApplyGeneralSettings();
    CORE::CString::StringVector leaveOut;
    leaveOut.push_back( CWorkbenchShell::ActionConsole );
    m_preferences.CaptureShortcuts( m_shell.GetShortcuts(), leaveOut );
    BindSources();
    UpdateShortcutHelp();
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "CWorkbenchSettingsLayer: The settings are applied" );
    NotifyObservers( SettingsAppliedEvent );
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchSettingsLayer::OnGeneralApplied( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    // the general page set the key of the console from its own settings: it is the key of the input page that counts
    m_general = m_generalPage->GetGeneralSettings();
    SyncShortcutsToConsoleKey();
    ApplyGeneralSettings();
    // the options of the workbench of the page: the shell is told what the user changed
    if ( m_generalPage->TakeResourceViewStartChanged() )
        m_shell.SetOpenPacksInResourceView( m_preferences.openPacksInResourceView );
    if ( m_generalPage->TakePresentationChanged() )
        m_shell.SetPackPresentationEnabled( m_preferences.openPackPresentation );
    if ( m_generalPage->TakeThumbnailsChanged() )
        m_shell.SetAutomaticThumbnailsEnabled( m_preferences.automaticThumbnails );
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchSettingsLayer::OnClearAutomaticThumbnails( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    m_shell.ClearAutoThumbnails();
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchSettingsLayer::OnPluginsLoadRequested( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    LoadPrivatePlugins( m_sourcesPage->GetPluginsDirFromWidgets() );
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchSettingsLayer::OnFormKeyDown( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    GUI::CKeyEventData* key = CORE::CICloneable::CloneableAs< GUI::CKeyEventData >( eventData );
    if ( GUCEF_NULL == key || key->handled || GUI::EVENTPHASE_BUBBLE == key->phase )
        return;
    // F1 in the settings window: the help of the settings (not while a key is being chosen on the input page)
    if ( INPUT::KEYCODE_F1 == key->keyCode && 0 == key->modifiers && !m_inputPage->IsRebinding() )
    {
        OpenHelp( CWorkbenchHelp::TopicSettings );
        key->handled = true;
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETWORKBENCH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
