/*
 *  AssetWorkbenchLib_TestApp: tests of the AssetWorkbenchLib module
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

#include <string.h>
#include <Ogre.h>

#include "gucef_test_framework.h"
#include "gucefCORE_CTEventHandlerFunctor.h"
#include "gucefVFS.h"
#include "dvcppstringutils.h"
#include "dvcppfileutils.h"
#include "CFileAccess.h"

#include "gucefAUTH_CAuthGlobal.h"
#include "gucefGUI_CTabControl.h"
#include "gucefGUI_CListBox.h"
#include "gucefGUI_CEditbox.h"
#include "gucefGUI_CCombobox.h"
#include "gucefGUI_CTextbox.h"
#include "gucefGUI_CLayoutLint.h"
#include "gucefASSETSTORE_CAssetStoreGlobal.h"
#include "gucefASSETSTORE_CIAssetStoreProbe.h"
#include "gucefASSETSTORE_CAssetStorePluginLoader.h"

#include "AssetWorkbenchLib.h"

#include "TestAssetWorkbenchSupport.h"
#include "TestAssetWorkbenchAssets.h"
#include "TestFakeSourceStore.h"
#include "TestAssetWorkbenchSettings.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;
using namespace GUCEF::ASSETWORKBENCH;

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** A probe of the tests: what the checks say is what the page shows */
class CTestProbe : public ASSETSTORE::CIAssetStoreProbe
{
    public:

    CTestProbe( void ) : runCount( 0 ) {}

    virtual CORE::CString GetDescription( void ) const GUCEF_VIRTUAL_OVERRIDE { return "checks the shape of the answers of the fake store"; }

    virtual bool Run( ASSETSTORE::CAssetStoreProbeResult& result ) GUCEF_VIRTUAL_OVERRIDE
    {
        ++runCount;
        result.AddCheck( "sign in", "the session is accepted", true );
        result.AddCheck( "library listing", "an array of items with an id and a title", true );
        result.AddCheck( "item details", "the field files is missing", false );
        return true;
    }

    UInt32 runCount;
};

/** The fake store of the sources tests, with a probe */
class CProbeSourceProvider : public SOURCESTEST::CFakeSourceProvider
{
    public:

    virtual ASSETSTORE::CIAssetStoreProbe* GetProbe( void ) GUCEF_VIRTUAL_OVERRIDE { return &probe; }

    CTestProbe probe;
};

typedef CORE::CTFactory< ASSETSTORE::CIAssetStoreProvider, CProbeSourceProvider, MT::CMutex > TProbeSourceFactory;

/** The workbench with its settings layer in a window of the tests; the sources, the credentials and a store with a probe on request */
class CSettingsFixture
{
    public:

    GUI::TGuiContextPtr contextPtr;
    GUI::CGUIContext* context;
    GUIDRIVEROGRE::COgreWindowContext* window;
    CAssetMounter mounter;
    CWorkbenchShell* shell;
    CWorkbenchPreferences preferences;
    CWorkbenchSettingsLayer* layer;
    CWorkbenchCredentials* credentials;
    TProbeSourceFactory storeFactory;
    ASSETSTORE::TAssetStoreProviderPtr storePtr;
    CProbeSourceProvider* store;
    CAssetSourcesSettings sourcesSettings;
    CORE::CString dir;
    bool sourcesOpened;
    bool ready;

    CSettingsFixture( const CORE::CString& name, bool withSources, bool withStore, bool withCredentials = true )
        : contextPtr(), context( GUCEF_NULL ), window( GUCEF_NULL ), mounter(), shell( GUCEF_NULL ), preferences(), layer( GUCEF_NULL ), credentials( GUCEF_NULL )
        , storeFactory(), storePtr(), store( GUCEF_NULL ), sourcesSettings(), dir(), sourcesOpened( false ), ready( false )
    {
        if ( GetMainTestWindow().IsNULL() )
            return;
        window = AsOgreWindow( GetMainTestWindow() );
        dir = MakeCleanTestDir( name );
        if ( withStore )
        {
            storePtr = storeFactory.Create();
            store = static_cast< CProbeSourceProvider* >( storePtr.GetPointerAlways() );
            store->SetSignedIn( false );
            ASSETSTORE::CAssetStoreGlobal::Instance()->GetRegistry().AddProvider( storePtr );
        }
        contextPtr = CreateWorkbenchContext();
        context = contextPtr.IsNULL() ? GUCEF_NULL : contextPtr->AsGUIContext();
        if ( GUCEF_NULL == context )
            return;
        shell = GUCEF_NEW CWorkbenchShell( mounter );
        if ( !shell->Initialize( *context, GUCEF_NULL, MakeCleanTestDir( name + "_cache" ) ) )
            return;
        if ( withCredentials )
        {
            credentials = GUCEF_NEW CWorkbenchCredentials();
            credentials->Open( dir + "/auth" );
            credentials->GetKeyringStore().SetKeyringParameters( 64, 1, 1 );
            credentials->Install();
            shell->SetCredentials( credentials );
            shell->SetAuthManager( &AUTH::CAuthGlobal::Instance()->GetManager() );
        }
        layer = GUCEF_NEW CWorkbenchSettingsLayer( *shell, preferences );
        if ( !layer->Create() )
            return;
        if ( withSources )
        {
            const CORE::CString storeDir = dir + "/store";
            CORE::CreateDirs( storeDir );
            const CORE::CString root = "avstore/" + name;
            sourcesSettings.primaryStore.locations[ 0 ].vfsRoot = root;
            sourcesSettings.primaryStore.locations[ 0 ].fileSystemPath = storeDir;
            sourcesSettings.primaryStore.database.database = root + "/catalogue";
            sourcesSettings.primaryStore.analysisConcurrency = 2;
            sourcesOpened = shell->InitializeSources( sourcesSettings );
            layer->BindSources();
        }
        ready = true;
    }

    ~CSettingsFixture()
    {
        GUCEF_DELETE layer;
        if ( GUCEF_NULL != shell )
        {
            shell->Shutdown();
            GUCEF_DELETE shell;
        }
        if ( GUCEF_NULL != store )
            ASSETSTORE::CAssetStoreGlobal::Instance()->GetRegistry().RemoveProvider( SOURCESTEST::CFakeSourceProvider::ProviderIdText );
        storePtr.Unlink();
        if ( GUCEF_NULL != credentials )
        {
            credentials->Shutdown();
            GUCEF_DELETE credentials;
        }
        contextPtr.Unlink();
        mounter.UnmountAll();
    }

    IMAGE::TPixelMapPtr Frames( UInt32 count )
    {
        IMAGE::TPixelMapPtr pixels;
        for ( UInt32 i=0; i<count; ++i )
        {
            shell->Update( 5 );
            window->RenderFrame( &pixels );
        }
        return pixels;
    }

    void Key( INPUT::KeyCode key, UInt32 modifiers = 0 )
    {
        context->InjectKey( key, true, modifiers );
        context->InjectKey( key, false, modifiers );
    }

    /** The key as the window delivers the key left of 1: the key code of the key and then its character */
    void TypeBackquote( void )
    {
        context->InjectKey( INPUT::KEYCODE_GRAVE, true, 0 );
        context->InjectText( "`" );
        context->InjectKey( INPUT::KEYCODE_GRAVE, false, 0 );
    }

    /** A click in the middle of a widget */
    void Click( GUI::CWidget* widget )
    {
        context->FlushChanges();
        const GUI::CRect rect = widget->GetRect();
        context->InjectMouseMove( rect.left + rect.width / 2, rect.top + rect.height / 2 );
        context->InjectMouseButton( GUI::MOUSEBUTTON_LEFT, true );
        context->InjectMouseButton( GUI::MOUSEBUTTON_LEFT, false );
    }
};

/** The first widget below the widget (or the widget itself) that can get the focus; NULL for none */
GUI::CWidget*
FindFocusable( GUI::CWidget* widget )
{
    if ( GUCEF_NULL == widget )
        return GUCEF_NULL;
    if ( widget->IsFocusable() )
        return widget;
    const GUI::CWidget::TWidgetVector& children = widget->GetChildren();
    for ( size_t i=0; i<children.size(); ++i )
    {
        GUI::CWidget* found = FindFocusable( children[ i ] );
        if ( GUCEF_NULL != found )
            return found;
    }
    return GUCEF_NULL;
}

/** The text a widget shows */
CORE::CString
TextOf( GUI::CWidget* widget )
{
    return GUCEF_NULL != widget ? widget->GetProperty( GUI::EWidgetProperty_Text ) : CORE::CString( "no widget" );
}

/** The output of the console */
CORE::CString
ConsoleText( CWorkbenchSettingsLayer& layer )
{
    GUI::CTextbox* output = layer.GetConsole()->GetWidgetAs< GUI::CTextbox >( "Output" );
    return GUCEF_NULL != output ? output->GetTextAsString() : CORE::CString::Empty;
}

/** Counts the events of a notifier */
class CCounter : public CORE::CObserver
{
    public:

    UInt32 count;

    CCounter( CORE::CNotifier* notifier, const CORE::CEvent& id ) : CORE::CObserver(), count( 0 ), m_id( id ) { SubscribeTo( notifier, id ); }
    virtual ~CCounter() { UnsubscribeAllFromObserver(); }

    virtual void OnNotify( CORE::CNotifier* notifier, const CORE::CEvent& id, CORE::CICloneable* eventData ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( id != m_id )
            return;
        ++count;
    }

    private:

    CORE::CEvent m_id;
};

/** The titles of the tabs of a settings form */
CORE::CString
PageTabs( GUI::CSettingsForm* form )
{
    return form->GetWidget( "Pages" )->GetProperty( "Tabs" );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      FUNCTIONS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static void
TestSettingsWindow( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Settings: Ctrl+, and the menu open the settings window with the pages General, Video, Audio, Input, Authentication, Sources and Remote store; Cancel and Escape close it" )
    try
    {
        if ( GetMainTestWindow().IsNULL() )
            GUCEF_TESTFW_SKIP( GetNoWindowReason().C_String() );
        CSettingsFixture fixture( "settings_open", true, false );
        ASSERT_TRUE( fixture.ready && fixture.sourcesOpened );
        CWorkbenchShell& shell = *fixture.shell;
        CWorkbenchSettingsLayer& layer = *fixture.layer;
        fixture.Frames( 3 );

        // the menu bar has the entry, and the keys of the settings and of the help are in the shortcut map with the other keys
        ASSERT_TRUE( GUCEF_NULL != shell.GetMenuBar() && TextOf( shell.GetMenuBar() ).IsNULLOrEmpty() );
        ASSERT_TRUE( shell.GetMenuBar()->GetProperty( GUI::EWidgetProperty_Items ).HasSubstr( "File/Settings..." ) >= 0 );
        ASSERT_TRUE( "File/Settings..." == shell.GetMenuPathOfAction( CWorkbenchShell::ActionSettings ) );
        ASSERT_TRUE( "Ctrl+," == shell.GetShortcuts().GetShortcutText( CWorkbenchShell::ActionSettings ) && "F1" == shell.GetShortcuts().GetShortcutText( CWorkbenchShell::ActionHelp ) );
        ASSERT_FALSE( layer.IsOpen() );

        // the menu bar has its row in a window of 800 pixels high or more, and gives it to the views in a lower one
        fixture.context->SetLogicalResolution( 1600, 900 );
        fixture.Frames( 3 );
        ASSERT_TRUE( shell.GetMenuBar()->IsEffectivelyVisible() && shell.GetMenuBar()->GetRect().height >= 24 );
        ASSERT_TRUE( shell.GetMainSplitter()->GetRect().top == shell.GetMenuBar()->GetRect().top + shell.GetMenuBar()->GetRect().height );
        DumpPicture( fixture.Frames( 2 ), "f9_menu_bar_900" );
        fixture.context->SetLogicalResolution( 1000, 700 );
        fixture.Frames( 3 );
        ASSERT_FALSE( shell.GetMenuBar()->IsEffectivelyVisible() );
        ASSERT_TRUE( 0 == shell.GetMainSplitter()->GetRect().top );
        fixture.context->SetLogicalResolution( 0, 0 );
        fixture.Frames( 3 );

        fixture.Key( INPUT::KEYCODE_COMMA, INPUT::KEYMOD_CTRL );
        fixture.Frames( 2 );
        ASSERT_TRUE( layer.IsOpen() );
        ASSERT_TRUE( "General\nVideo\nAudio\nInput\nAuthentication\nSources\nRemote store" == PageTabs( layer.GetForm() ) );
        ASSERT_TRUE( 7 == layer.GetForm()->GetPages().size() && GUCEF_NULL != layer.GetVideoPage() && GUCEF_NULL != layer.GetAudioPage() && GUCEF_NULL != layer.GetInputPage() );
        DumpPicture( fixture.Frames( 2 ), "f9_settings_general" );
        fixture.Key( INPUT::KEYCODE_ESCAPE );
        fixture.Frames( 2 );
        ASSERT_FALSE( layer.IsOpen() );

        // the menu item does the same as the key
        ASSERT_TRUE( shell.ExecuteAction( CWorkbenchShell::ActionSettings ) );
        ASSERT_TRUE( layer.IsOpen() );
        ASSERT_FALSE( layer.Open() );
        layer.GetForm()->Cancel();
        ASSERT_FALSE( layer.IsOpen() );

        // the page by its title
        ASSERT_TRUE( layer.Open( "Sources" ) );
        ASSERT_TRUE( "Sources" == layer.GetForm()->GetSelectedPageTitle() );
        layer.GetForm()->Cancel();
        ASSERT_TRUE( layer.Open( "Input" ) && "Input" == layer.GetForm()->GetSelectedPageTitle() );
        DumpPicture( fixture.Frames( 2 ), "f9_settings_input" );
        layer.GetForm()->Cancel();
        ASSERT_FALSE( shell.ExecuteAction( "NoSuchAction" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Settings: the settings of the pages are applied and handed to the application, and the preferences are kept in the section AssetWorkbench and read back" )
    try
    {
        if ( GetMainTestWindow().IsNULL() )
            GUCEF_TESTFW_SKIP( GetNoWindowReason().C_String() );
        CSettingsFixture fixture( "settings_apply", false, false );
        ASSERT_TRUE( fixture.ready );
        CWorkbenchSettingsLayer& layer = *fixture.layer;
        GUI::CGeneralSettings general;
        general.SetConsoleLogWindow( false );
        general.SetDeveloperConsole( true );
        layer.SetGeneralSettings( general );
        GUI::CVideoSettings video;
        video.SetResolution( 1280, 720, 32 );
        layer.SetVideoSettings( video );
        CORE::CString::StringVector renderers;
        renderers.push_back( "OpenGL" );
        renderers.push_back( "Vulkan" );
        layer.SetRenderers( renderers, "OpenGL" );
        CCounter applied( &layer, CWorkbenchSettingsLayer::SettingsAppliedEvent );

        ASSERT_TRUE( layer.Open( "Video" ) );
        fixture.Frames( 2 );
        // the renderer choice and the resolution are the ones of the shared video page
        GUI::CWidget* renderer = layer.GetVideoPage()->GetWidget( "Renderer" );
        ASSERT_TRUE( GUCEF_NULL != renderer && GUCEF_NULL != layer.GetVideoPage()->GetWidget( "Resolution" ) );
        GUI::CCombobox* rendererChoice = GUI::CWidget::As< GUI::CCombobox >( renderer );
        ASSERT_TRUE( GUCEF_NULL != rendererChoice );
        GUI::CCombobox::TStringVector choices;
        ASSERT_TRUE( rendererChoice->GetListItems( choices ) );
        bool vulkanOffered = false;
        for ( size_t i=0; i<choices.size(); ++i )
            vulkanOffered = vulkanOffered || choices[ i ] == "Vulkan";
        ASSERT_TRUE( vulkanOffered );
        DumpPicture( fixture.Frames( 2 ), "f9_settings_video" );
        ASSERT_TRUE( layer.GetForm()->SelectPage( "General" ) );
        GUI::CWidget* consoleCheck = layer.GetGeneralPage()->GetWidget( "DeveloperConsole" );
        ASSERT_TRUE( GUCEF_NULL != consoleCheck && consoleCheck->GetBoolProperty( "Checked" ) );
        fixture.Click( consoleCheck );
        ASSERT_FALSE( consoleCheck->GetBoolProperty( "Checked" ) );
        fixture.Click( layer.GetForm()->GetWidget( "Ok" ) );
        fixture.Frames( 2 );
        ASSERT_FALSE( layer.IsOpen() );
        ASSERT_TRUE( 1 == applied.count );
        ASSERT_FALSE( layer.GetGeneralSettings().GetDeveloperConsole() );
        ASSERT_FALSE( layer.GetConsole()->IsEnabled() );
        // the console that is off does not open on its key
        fixture.TypeBackquote();
        ASSERT_FALSE( layer.GetConsole()->IsOpen() );

        // the preferences: written to the section and read back, also what is not set
        CWorkbenchPreferences preferences;
        preferences.privatePluginsDir = "/some/private/folder";
        preferences.SetStoreEnabled( "otherstore", false );
        preferences.SetStoreEnabled( "fakesrc", false );
        preferences.SetStoreEnabled( "fakesrc", true );
        preferences.shortcuts.push_back( "FoldLeftSide=F8" );
        preferences.settingsVersion = CWorkbenchPreferences::CurrentSettingsVersion;
        CORE::CDataNode section( "AssetWorkbench" );
        preferences.Save( section );
        CWorkbenchPreferences back;
        back.Load( section );
        ASSERT_TRUE( "/some/private/folder" == back.privatePluginsDir && !back.IsStoreEnabled( "otherstore" ) && back.IsStoreEnabled( "fakesrc" ) );
        ASSERT_TRUE( 1 == back.shortcuts.size() && "FoldLeftSide=F8" == back.shortcuts[ 0 ] && CWorkbenchPreferences::CurrentSettingsVersion == back.settingsVersion );
        CWorkbenchPreferences empty;
        empty.Load( CORE::CDataNode( "AssetWorkbench" ) );
        ASSERT_TRUE( empty.privatePluginsDir.IsNULLOrEmpty() && empty.disabledStores.empty() && empty.IsStoreEnabled( "otherstore" ) && 0 == empty.settingsVersion );
        // the time a scene of a pack may take to convert: 600 seconds unless set, kept when set, and never zero (a scene has no size cap)
        ASSERT_TRUE( 600 == CWorkbenchPreferences::DefaultMaxSceneSeconds && 600 == empty.maxSceneSeconds && 600 == back.maxSceneSeconds );
        preferences.maxSceneSeconds = 1800;
        CORE::CDataNode longer( "AssetWorkbench" );
        preferences.Save( longer );
        CWorkbenchPreferences longerBack;
        longerBack.Load( longer );
        ASSERT_TRUE( 1800 == longerBack.maxSceneSeconds && GUCEF_NULL != longer.GetAttribute( "MaxSceneSeconds" ) && GUCEF_NULL == longer.GetAttribute( "MaxSceneMegabytes" ) );
        longer.SetAttribute( "MaxSceneSeconds", (UInt32) 0 );
        longerBack.Load( longer );
        ASSERT_TRUE( 600 == longerBack.maxSceneSeconds );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Settings: the application keeps the preferences in its settings, and settings of before the console switch it on once" )
    try
    {
        char program[] = "AssetWorkbench";
        char* plain[] = { program };
        CWorkbenchApp app( 1, plain );

        // an old settings file: the console was off because nobody could ever switch it on
        CORE::CDataNode old( "Config" );
        CORE::CDataNode* general = old.Structure( "GeneralSettings", '/' );
        general->SetAttribute( "developerConsole", false );
        ASSERT_TRUE( app.LoadConfig( old ) );
        ASSERT_TRUE( app.GetGeneralSettings().GetDeveloperConsole() && CWorkbenchPreferences::CurrentSettingsVersion == app.GetPreferences().settingsVersion );

        // a new one: the owner's choice stands
        CORE::CDataNode config( "Config" );
        CORE::CDataNode* section = config.Structure( "AssetWorkbench", '/' );
        section->SetAttribute( "SettingsVersion", CWorkbenchPreferences::CurrentSettingsVersion );
        section->SetAttribute( "PrivatePluginsDir", "/private/plugins" );
        CORE::CString::StringVector disabled;
        disabled.push_back( "otherstore" );
        section->SetValuesOfChildByName( "DisabledStores", disabled );
        CORE::CString::StringVector keys;
        keys.push_back( "PreviewLayout=F8" );
        section->SetValuesOfChildByName( "Shortcuts", keys );
        CORE::CDataNode* generalAgain = config.Structure( "GeneralSettings", '/' );
        generalAgain->SetAttribute( "developerConsole", false );
        CWorkbenchApp second( 1, plain );
        ASSERT_TRUE( second.LoadConfig( config ) );
        ASSERT_FALSE( second.GetGeneralSettings().GetDeveloperConsole() );
        ASSERT_TRUE( "/private/plugins" == second.GetPreferences().privatePluginsDir && !second.GetPreferences().IsStoreEnabled( "otherstore" ) && 1 == second.GetPreferences().shortcuts.size() );
        CORE::CDataNode saved( "Config" );
        ASSERT_TRUE( second.SaveConfig( saved ) );
        const CORE::CDataNode* savedSection = saved.FindChild( "AssetWorkbench" );
        ASSERT_TRUE( GUCEF_NULL != savedSection );
        CWorkbenchPreferences savedPreferences;
        savedPreferences.Load( *savedSection );
        ASSERT_TRUE( "/private/plugins" == savedPreferences.privatePluginsDir && !savedPreferences.IsStoreEnabled( "otherstore" ) && "PreviewLayout=F8" == savedPreferences.shortcuts[ 0 ] );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
TestSettingsInput( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Input settings: the page lists the keys of the workbench, a key of another action is refused, and the keys that are applied work and are kept in the preferences" )
    try
    {
        if ( GetMainTestWindow().IsNULL() )
            GUCEF_TESTFW_SKIP( GetNoWindowReason().C_String() );
        CWorkbenchPreferences kept;
        {
        CSettingsFixture fixture( "settings_input", false, false );
        ASSERT_TRUE( fixture.ready );
        CWorkbenchShell& shell = *fixture.shell;
        CWorkbenchSettingsLayer& layer = *fixture.layer;
        GUI::CInputSettingsForm* input = layer.GetInputPage();
        ASSERT_TRUE( layer.Open( "Input" ) );

        // F9, F10 and F11 of the layouts, F1 of the help, Ctrl+O and Ctrl+Shift+O of the dialogs, Ctrl+, of the settings and the key of the console
        ASSERT_TRUE( 8 == input->GetActionCount() );
        CORE::CString lines;
        for ( UInt32 i=0; i<input->GetActionCount(); ++i )
            lines += input->GetActionLine( i ) + "\n";
        ASSERT_TRUE( lines.HasSubstr( "    F9\n" ) >= 0 && lines.HasSubstr( "    F10\n" ) >= 0 && lines.HasSubstr( "    F11\n" ) >= 0 && lines.HasSubstr( "    F1\n" ) >= 0 );
        ASSERT_TRUE( lines.HasSubstr( "    Ctrl+O\n" ) >= 0 && lines.HasSubstr( "    Ctrl+Shift+O\n" ) >= 0 && lines.HasSubstr( "    Ctrl+,\n" ) >= 0 && lines.HasSubstr( "    Grave\n" ) >= 0 );

        // F10 belongs to the inspector: the fold of the resources cannot have it
        ASSERT_TRUE( input->SelectAction( CWorkbenchShell::ActionFoldLeft ) && input->BeginRebind() );
        fixture.Key( INPUT::KEYCODE_F10 );
        ASSERT_TRUE( input->GetStatusText().HasSubstr( "is the key of Fold or unfold the inspector already" ) >= 0 );
        INPUT::KeyCode key = INPUT::KEYCODE_UNASSIGNED;
        UInt32 modifiers = 0;
        ASSERT_TRUE( input->GetPendingChord( CWorkbenchShell::ActionFoldLeft, key, modifiers ) && INPUT::KEYCODE_F9 == key );

        // F8 is free
        ASSERT_TRUE( input->BeginRebind() );
        fixture.Key( INPUT::KEYCODE_F8 );
        ASSERT_TRUE( input->GetPendingChord( CWorkbenchShell::ActionFoldLeft, key, modifiers ) && INPUT::KEYCODE_F8 == key );
        // a change that is not applied is not in effect
        ASSERT_TRUE( "F9" == shell.GetShortcuts().GetShortcutText( CWorkbenchShell::ActionFoldLeft ) );
        fixture.Click( layer.GetForm()->GetWidget( "Ok" ) );
        fixture.Frames( 2 );
        ASSERT_FALSE( layer.IsOpen() );
        ASSERT_TRUE( "F8" == shell.GetShortcuts().GetShortcutText( CWorkbenchShell::ActionFoldLeft ) && "F10" == shell.GetShortcuts().GetShortcutText( CWorkbenchShell::ActionFoldRight ) );
        ASSERT_FALSE( shell.GetLeftPanel()->IsFolded() );
        fixture.Key( INPUT::KEYCODE_F9 );
        fixture.Frames( 2 );
        ASSERT_FALSE( shell.GetLeftPanel()->IsFolded() );
        fixture.Key( INPUT::KEYCODE_F8 );
        fixture.Frames( 2 );
        ASSERT_TRUE( shell.GetLeftPanel()->IsFolded() );
        fixture.Key( INPUT::KEYCODE_F8 );
        fixture.Frames( 2 );
        ASSERT_FALSE( shell.GetLeftPanel()->IsFolded() );

        // kept in the preferences, as the console key (a setting of the general page) is not
        bool foundF8 = false;
        for ( size_t i=0; i<fixture.preferences.shortcuts.size(); ++i )
        {
            foundF8 = foundF8 || "FoldLeftSide=F8" == fixture.preferences.shortcuts[ i ];
            ASSERT_TRUE( fixture.preferences.shortcuts[ i ].HasSubstr( "DeveloperConsole" ) < 0 );
        }
        ASSERT_TRUE( foundF8 );
        kept = fixture.preferences;
        }

        // the next start: another shell with the same preferences has F8 as soon as its layer is made
        CSettingsFixture second( "settings_input_again", false, false );
        ASSERT_TRUE( second.ready );
        ASSERT_TRUE( "F9" == second.shell->GetShortcuts().GetShortcutText( CWorkbenchShell::ActionFoldLeft ) );
        second.preferences = kept;
        GUCEF_DELETE second.layer;
        second.layer = GUCEF_NEW CWorkbenchSettingsLayer( *second.shell, second.preferences );
        ASSERT_TRUE( second.layer->Create() );
        ASSERT_TRUE( "F8" == second.shell->GetShortcuts().GetShortcutText( CWorkbenchShell::ActionFoldLeft ) && "F10" == second.shell->GetShortcuts().GetShortcutText( CWorkbenchShell::ActionFoldRight ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}


/*-------------------------------------------------------------------------*/

static void
TestConsoleAndCommands( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Console: the key left of 1 opens the developer console (the key code of the platform and its character, once), not while typing in a text field, and the menu does too" )
    try
    {
        if ( GetMainTestWindow().IsNULL() )
            GUCEF_TESTFW_SKIP( GetNoWindowReason().C_String() );
        CSettingsFixture fixture( "console_key", false, false );
        ASSERT_TRUE( fixture.ready );
        CWorkbenchSettingsLayer& layer = *fixture.layer;
        GUI::CDeveloperConsoleForm* console = layer.GetConsole();
        ASSERT_TRUE( GUCEF_NULL != console && console->IsEnabled() && INPUT::KEYCODE_GRAVE == console->GetConsoleKey() && !console->WatchesInputDevices() );
        console->SetSlideTimeInMs( 0 );
        ASSERT_TRUE( fixture.shell->GetDeveloperConsole() == console );
        fixture.Frames( 2 );

        // the key code of the physical key, then its character: it opens, and the character is not typed in the command line
        fixture.TypeBackquote();
        ASSERT_TRUE( console->IsOpen() );
        GUI::CEditbox* commandLine = console->GetWidgetAs< GUI::CEditbox >( "CommandLine" );
        ASSERT_TRUE( GUCEF_NULL != commandLine && commandLine->GetText().IsNULLOrEmpty() );
        DumpPicture( fixture.Frames( 3 ), "f9_console_open" );
        fixture.TypeBackquote();
        ASSERT_FALSE( console->IsOpen() );

        // a keyboard layout that has the character on another key: the character alone does it
        fixture.context->InjectText( "`" );
        ASSERT_TRUE( console->IsOpen() );
        fixture.context->InjectText( "`" );
        ASSERT_FALSE( console->IsOpen() );
        // with the shift held the key is the tilde
        fixture.context->InjectKey( INPUT::KEYCODE_GRAVE, true, INPUT::KEYMOD_SHIFT );
        fixture.context->InjectText( "~" );
        ASSERT_TRUE( console->IsOpen() );
        fixture.Key( INPUT::KEYCODE_ESCAPE );
        console->Close();
        ASSERT_FALSE( console->IsOpen() );

        // typing in a text field of the workbench: a character of the text, not the console
        // the first text field of the window
        GUI::CEditbox* path = GUCEF_NULL;
        GUI::CWidget::TWidgetVector stack;
        stack.push_back( fixture.shell->GetMenuBar()->GetParentWidget() );
        while ( !stack.empty() && GUCEF_NULL == path )
        {
            GUI::CWidget* current = stack.back();
            stack.pop_back();
            if ( "Editbox" == current->GetType() && current->IsEffectivelyVisible() )
                path = GUI::CWidget::As< GUI::CEditbox >( current );
            for ( size_t i=0; i<current->GetChildren().size(); ++i )
                stack.push_back( current->GetChildren()[ i ] );
        }
        ASSERT_TRUE( GUCEF_NULL != path && path->SetFocus() );
        fixture.TypeBackquote();
        ASSERT_FALSE( console->IsOpen() );
        ASSERT_TRUE( path->GetText().HasSubstr( "`" ) >= 0 );
        fixture.context->SetFocus( GUCEF_NULL );

        // the menu entry and the action
        ASSERT_TRUE( fixture.shell->ExecuteAction( CWorkbenchShell::ActionConsole ) && console->IsOpen() );
        ASSERT_TRUE( "View/Developer console" == fixture.shell->GetMenuPathOfAction( CWorkbenchShell::ActionConsole ) );
        ASSERT_TRUE( fixture.shell->ExecuteAction( CWorkbenchShell::ActionConsole ) && !console->IsOpen() );
        console->Close();
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Console: the commands help, open, layout, layout lint, convert and jobs run and answer in the output; an unknown command is left to others" )
    try
    {
        if ( GetMainTestWindow().IsNULL() )
            GUCEF_TESTFW_SKIP( GetNoWindowReason().C_String() );
        CSettingsFixture fixture( "console_commands", true, false );
        ASSERT_TRUE( fixture.ready && fixture.sourcesOpened );
        CWorkbenchSettingsLayer& layer = *fixture.layer;
        layer.GetConsole()->SetSlideTimeInMs( 0 );
        CWorkbenchConsoleCommands* commands = layer.GetCommands();
        GUI::CDeveloperConsoleForm* console = layer.GetConsole();
        ASSERT_TRUE( GUCEF_NULL != commands && GUCEF_NULL != console && 22 == commands->GetCommandCount() );
        ASSERT_TRUE( "help" == commands->GetCommandName( 0 ) && "open <path>" == commands->GetCommandUsage( 1 ) );
        fixture.TypeBackquote();
        ASSERT_TRUE( console->IsOpen() );

        // typed in the command line, Enter
        GUI::CEditbox* commandLine = console->GetWidgetAs< GUI::CEditbox >( "CommandLine" );
        fixture.context->InjectText( "help" );
        fixture.Key( INPUT::KEYCODE_RETURN );
        const CORE::CString helpText = ConsoleText( layer );
        ASSERT_TRUE( helpText.HasSubstr( "> help" ) >= 0 && helpText.HasSubstr( "open <path>" ) >= 0 && helpText.HasSubstr( "layout <Browse" ) >= 0 && helpText.HasSubstr( "gui.lint" ) >= 0 );
        ASSERT_TRUE( commandLine->GetText().IsNULLOrEmpty() );
        DumpPicture( fixture.Frames( 3 ), "f9_console_help" );

        // the jobs, the redo and the bulk conversion
        ASSERT_TRUE( commands->Run( "jobs finished" ) && ConsoleText( layer ).HasSubstr( "No jobs" ) >= 0 );
        ASSERT_TRUE( commands->Run( "redo reanalyse" ) && ConsoleText( layer ).HasSubstr( "There is no pack to do that for" ) >= 0 );
        ASSERT_TRUE( commands->Run( "redo nonsense" ) && ConsoleText( layer ).HasSubstr( "redo <reindex" ) >= 0 );

        // the reprocessing of a pack of the store: no pack, a pack the store does not have, a wrong word or option
        ASSERT_TRUE( commands->Run( "regen nonsense" ) && ConsoleText( layer ).HasSubstr( "Usage: regen <intermediate" ) >= 0 );
        ASSERT_TRUE( commands->Run( "regen intermediate" ) && ConsoleText( layer ).HasSubstr( "There is no pack to do that for" ) >= 0 );
        ASSERT_TRUE( commands->Run( "regen target pkNoSuchPack --failed" ) && ConsoleText( layer ).HasSubstr( "Regenerate target of pkNoSuchPack was refused" ) >= 0 );
        ASSERT_TRUE( commands->Run( "reprocess plan pkNoSuchPack" ) && ConsoleText( layer ).HasSubstr( "The plan was refused" ) >= 0 );
        ASSERT_TRUE( commands->Run( "redownload pkNoSuchPack --nonsense" ) && ConsoleText( layer ).HasSubstr( "the option --nonsense is not known" ) >= 0 );
        ASSERT_TRUE( commands->Run( "reprocess rollback pkNoSuchPack" ) && ConsoleText( layer ).HasSubstr( "Roll back regeneration of pkNoSuchPack was refused" ) >= 0 );
        ASSERT_TRUE( commands->Run( "bulk plan" ) && commands->Run( "jobs open" ) );

        // the source of what is shown: nothing is shown here, and the setting is told and switched
        ASSERT_TRUE( commands->Run( "source" ) && ConsoleText( layer ).HasSubstr( "Prefer cooked output is on" ) >= 0 );
        ASSERT_TRUE( commands->Run( "source quicklook" ) && ConsoleText( layer ).HasSubstr( "Prefer cooked output is off" ) >= 0 );
        ASSERT_TRUE( commands->Run( "source cooked" ) && ConsoleText( layer ).HasSubstr( "Prefer cooked output is on" ) >= 0 );
        ASSERT_TRUE( commands->Run( "ogre" ) && ConsoleText( layer ).HasSubstr( "Convert for Ogre was not started" ) >= 0 );

        // layout
        ASSERT_TRUE( commands->Run( "layout Preview" ) );
        fixture.Frames( 3 );
        ASSERT_TRUE( fixture.shell->GetLeftPanel()->IsFolded() && fixture.shell->GetRightPanel()->IsFolded() );
        ASSERT_TRUE( commands->Run( "layout Browse" ) );
        fixture.Frames( 3 );
        ASSERT_FALSE( fixture.shell->GetLeftPanel()->IsFolded() );
        ASSERT_TRUE( commands->Run( "layout Nonsense" ) && ConsoleText( layer ).HasSubstr( "There is no layout Nonsense" ) >= 0 );
        ASSERT_TRUE( commands->Run( "layout lint report" ) );
        ASSERT_TRUE( ConsoleText( layer ).HasSubstr( "> gui.lint report" ) >= 0 && ConsoleText( layer ).HasSubstr( "Layer " ) >= 0 );

        // open: a file of the disk, a path that is nowhere
        const CORE::CString picture = fixture.dir + "/checker.png";
        ASSERT_TRUE( WriteTestPng( picture, 8, 8 ) );
        ASSERT_TRUE( commands->Run( "open \"" + picture + "\"" ) );
        fixture.Frames( 2 );
        ASSERT_TRUE( ConsoleText( layer ).HasSubstr( "Opened " + picture ) >= 0 && ASSETKIND_IMAGE == fixture.shell->GetShownKind() );
        ASSERT_TRUE( commands->Run( "open /nowhere/at/all.gltf" ) && ConsoleText( layer ).HasSubstr( "Cannot open /nowhere/at/all.gltf" ) >= 0 );
        ASSERT_TRUE( commands->Run( "open" ) && ConsoleText( layer ).HasSubstr( "Usage: open <path>" ) >= 0 );

        // jobs and convert without a store of conversion
        ASSERT_TRUE( commands->Run( "jobs" ) );
        ASSERT_TRUE( ConsoleText( layer ).HasSubstr( "No jobs" ) >= 0 || ConsoleText( layer ).HasSubstr( "download" ) >= 0 || ConsoleText( layer ).HasSubstr( "ingest" ) >= 0 );
        ASSERT_TRUE( commands->Run( "convert pending" ) );
        ASSERT_TRUE( ConsoleText( layer ).HasSubstr( "no converter" ) >= 0 || ConsoleText( layer ).HasSubstr( "Nothing to convert" ) >= 0 || ConsoleText( layer ).HasSubstr( "queued" ) >= 0 );

        // not a command of the workbench: the line stays in the output for others
        ASSERT_FALSE( commands->Run( "gui.lint off" ) || commands->Run( "frobnicate" ) || commands->Run( "" ) );
        ASSERT_TRUE( commands->Run( "HELP" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}


/*-------------------------------------------------------------------------*/

static void
TestHelpAndTooltips( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Help: the catalogue of the workbench is complete and has no broken link, and the document of the shortcuts lists the keys that are in effect" )
    try
    {
        CWorkbenchHelp help;
        CORE::CString::StringVector problems;
        ASSERT_TRUE( help.Validate( problems ) );
        ASSERT_TRUE( problems.empty() );
        ASSERT_TRUE( 7 == help.GetCatalogue().GetTopicCount() );
        const CORE::CString topics[] = { CWorkbenchHelp::TopicOverview, CWorkbenchHelp::TopicOpening, CWorkbenchHelp::TopicViews, CWorkbenchHelp::TopicImporting,
                                         CWorkbenchHelp::TopicSettings, CWorkbenchHelp::TopicConsole, CWorkbenchHelp::TopicShortcuts };
        for ( UInt32 i=0; i<7; ++i )
        {
            GUI::CHelpDocument document;
            ASSERT_TRUE( help.GetSource()->GetTopic( topics[ i ], document ) && !document.brief.IsNULLOrEmpty() && !document.blocks.empty() );
            CORE::CString brief, more;
            ASSERT_TRUE( help.GetSource()->GetBrief( topics[ i ], brief, more ) && !brief.IsNULLOrEmpty() );
        }
        GUI::CHelpDocument views;
        ASSERT_TRUE( help.GetSource()->GetTopic( CWorkbenchHelp::TopicViews, views ) );
        const CORE::CString viewsText = views.ToPlainText( help.GetSource().GetPointer() );
        ASSERT_TRUE( viewsText.HasSubstr( "Four views" ) >= 0 && viewsText.HasSubstr( "F9 folds the resources" ) >= 0 && viewsText.HasSubstr( "Preview" ) >= 0 );

        CORE::CString::StringVector actions, chords;
        actions.push_back( "Fold or unfold the resources" );
        chords.push_back( "F9" );
        actions.push_back( "Open the settings" );
        chords.push_back( "Ctrl+," );
        help.SetShortcutRows( actions, chords );
        help.SetShortcutRows( actions, chords );
        GUI::CHelpDocument shortcuts;
        ASSERT_TRUE( help.GetSource()->GetTopic( CWorkbenchHelp::TopicShortcuts, shortcuts ) );
        const CORE::CString shortcutText = shortcuts.ToPlainText( help.GetSource().GetPointer() );
        ASSERT_TRUE( shortcutText.HasSubstr( "Fold or unfold the resources | F9" ) >= 0 && shortcutText.HasSubstr( "Open the settings | Ctrl+," ) >= 0 );
        // the rows of the first call are not there twice
        ASSERT_TRUE( shortcutText.HasSubstr( "Open the settings", shortcutText.HasSubstr( "Open the settings" ) + 5 ) < 0 );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Help: F1 opens the help window at the document of the area the focus is in: opening, views, importing, the settings in the settings window, the overview elsewhere" )
    try
    {
        if ( GetMainTestWindow().IsNULL() )
            GUCEF_TESTFW_SKIP( GetNoWindowReason().C_String() );
        CSettingsFixture fixture( "help_focus", true, false );
        ASSERT_TRUE( fixture.ready && fixture.sourcesOpened );
        CWorkbenchShell& shell = *fixture.shell;
        CWorkbenchSettingsLayer& layer = *fixture.layer;
        fixture.Frames( 3 );
        ASSERT_TRUE( GUCEF_NULL == layer.GetHelpViewer() );

        // the areas of the window point at their documents
        CWorkbenchHelp& help = layer.GetHelp();
        ASSERT_TRUE( CWorkbenchHelp::TopicOpening == help.FindTopicOfWidget( shell.GetOpenStrip()->GetRootWidget() ) );
        ASSERT_TRUE( CWorkbenchHelp::TopicOpening == help.FindTopicOfWidget( shell.GetBrowser()->GetRootWidget() ) );
        ASSERT_TRUE( CWorkbenchHelp::TopicViews == help.FindTopicOfWidget( shell.GetViewport()->GetRootWidget() ) );
        ASSERT_TRUE( CWorkbenchHelp::TopicViews == help.FindTopicOfWidget( shell.GetInspector()->GetRootWidget() ) );
        ASSERT_TRUE( CWorkbenchHelp::TopicImporting == help.FindTopicOfWidget( shell.GetTransfers()->GetRootWidget() ) );
        ASSERT_TRUE( CWorkbenchHelp::TopicImporting == help.FindTopicOfWidget( shell.GetSources()->GetControl( "Stores" ) ) );
        ASSERT_TRUE( CWorkbenchHelp::TopicOverview == help.FindTopicOfWidget( GUCEF_NULL ) );
        ASSERT_TRUE( CWorkbenchHelp::TopicOverview == help.FindTopicOfWidget( shell.GetMenuBar() ) );

        // F1 with the focus in the content browser
        GUI::CWidget* inBrowser = FindFocusable( shell.GetBrowser()->GetRootWidget() );
        ASSERT_TRUE( GUCEF_NULL != inBrowser && inBrowser->SetFocus() );
        fixture.Key( INPUT::KEYCODE_F1 );
        fixture.Frames( 2 );
        ASSERT_TRUE( GUCEF_NULL != layer.GetHelpViewer() && layer.GetHelpViewer()->IsOpen() );
        ASSERT_TRUE( CWorkbenchHelp::TopicOpening == layer.GetHelpViewer()->GetCurrentTopic() && layer.GetHelpViewer()->IsCurrentTopicFound() );
        ASSERT_TRUE( layer.GetHelpViewer()->GetDocumentText().HasSubstr( "Open folder" ) >= 0 );
        DumpPicture( fixture.Frames( 3 ), "f9_help_open" );
        layer.GetHelpViewer()->Close();
        ASSERT_FALSE( layer.GetHelpViewer()->IsOpen() );

        // in the 3D view area, in the sources
        GUI::CWidget* inViews = FindFocusable( shell.GetViewport()->GetRootWidget() );
        ASSERT_TRUE( GUCEF_NULL != inViews && inViews->SetFocus() );
        fixture.Key( INPUT::KEYCODE_F1 );
        ASSERT_TRUE( CWorkbenchHelp::TopicViews == layer.GetHelpViewer()->GetCurrentTopic() && layer.GetHelpViewer()->GetDocumentText().HasSubstr( "Three quarter" ) >= 0 );
        layer.GetHelpViewer()->Close();
        shell.ShowLeftTab( 2 );
        fixture.Frames( 2 );
        GUI::CWidget* inSources = FindFocusable( shell.GetSources()->GetControl( "Stores" ) );
        ASSERT_TRUE( GUCEF_NULL != inSources && inSources->SetFocus() );
        fixture.Key( INPUT::KEYCODE_F1 );
        ASSERT_TRUE( CWorkbenchHelp::TopicImporting == layer.GetHelpViewer()->GetCurrentTopic() );
        layer.GetHelpViewer()->Close();

        // no focus: the overview; the menu entry for the shortcuts: that document, with the keys that are in effect
        fixture.context->SetFocus( GUCEF_NULL );
        ASSERT_TRUE( shell.ExecuteAction( CWorkbenchShell::ActionHelp ) );
        ASSERT_TRUE( CWorkbenchHelp::TopicOverview == layer.GetHelpViewer()->GetCurrentTopic() );
        layer.GetHelpViewer()->Close();
        ASSERT_TRUE( shell.ExecuteAction( "Help:workbench.shortcuts" ) );
        ASSERT_TRUE( CWorkbenchHelp::TopicShortcuts == layer.GetHelpViewer()->GetCurrentTopic() );
        ASSERT_TRUE( layer.GetHelpViewer()->GetDocumentText().HasSubstr( "Fold or unfold the resources | F9" ) >= 0 && layer.GetHelpViewer()->GetDocumentText().HasSubstr( "Open the settings | Ctrl+," ) >= 0 );
        layer.GetHelpViewer()->Close();
        ASSERT_TRUE( layer.OpenHelp( "no.such.topic" ) && CWorkbenchHelp::TopicOverview == layer.GetHelpViewer()->GetCurrentTopic() );
        layer.GetHelpViewer()->Close();

        // in the settings window: the settings, with F1 in the window
        ASSERT_TRUE( layer.Open( "Sources" ) );
        fixture.Frames( 2 );
        ASSERT_TRUE( CWorkbenchHelp::TopicSettings == layer.GetHelpTopicForFocus() );
        fixture.Key( INPUT::KEYCODE_F1 );
        ASSERT_TRUE( layer.GetHelpViewer()->IsOpen() && CWorkbenchHelp::TopicSettings == layer.GetHelpViewer()->GetCurrentTopic() );
        layer.GetHelpViewer()->Close();
        layer.GetForm()->Cancel();
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Tooltips: every control of the workbench has a text from the table of the tooltips, the tabs one each, and the text reaches the tooltip service" )
    try
    {
        if ( GetMainTestWindow().IsNULL() )
            GUCEF_TESTFW_SKIP( GetNoWindowReason().C_String() );
        CSettingsFixture fixture( "tooltips_all", true, true );
        ASSERT_TRUE( fixture.ready && fixture.sourcesOpened );
        CWorkbenchShell& shell = *fixture.shell;
        CWorkbenchSettingsLayer& layer = *fixture.layer;
        CWorkbenchTooltips& tooltips = layer.GetTooltips();
        ASSERT_TRUE( tooltips.GetEntryCount() > 40 );
        ASSERT_TRUE( &tooltips == fixture.context->GetTooltipService().GetTextProvider() );

        // the forms that open on request are made and opened, so that their controls are there too
        shell.ShowOpenFolderDialog();
        shell.GetFolderDialog()->Cancel();
        shell.ShowOpenArchiveDialog();
        shell.GetArchiveDialog()->Cancel();
        shell.ShowOpenFileDialog();
        shell.GetFileDialog()->Cancel();
        layer.OpenHelp( CWorkbenchHelp::TopicOverview );
        layer.GetHelpViewer()->Close();
        ASSERT_TRUE( layer.Open() );
        fixture.Frames( 2 );
        layer.GetForm()->Cancel();

        CORE::CString::StringVector missing;
        GUCEF::vector< GUI::CWidget* > roots;
        roots.push_back( shell.GetMenuBar()->GetParentWidget() );
        roots.push_back( layer.GetForm()->GetRootWidget() );
        roots.push_back( layer.GetConsole()->GetRootWidget() );
        roots.push_back( layer.GetHelpViewer()->GetRootWidget() );
        roots.push_back( shell.GetFolderDialog()->GetRootWidget() );
        roots.push_back( shell.GetArchiveDialog()->GetRootWidget() );
        roots.push_back( shell.GetFileDialog()->GetRootWidget() );
        roots.push_back( shell.GetSignIn()->GetUnlockForm()->GetRootWidget() );
        roots.push_back( shell.GetSignIn()->GetCodeEntryForm()->GetRootWidget() );
        roots.push_back( shell.GetSignIn()->GetSettingsForm()->GetRootWidget() );
        for ( size_t i=0; i<roots.size(); ++i )
        {
            ASSERT_TRUE( GUCEF_NULL != roots[ i ] );
            tooltips.FindMissing( *roots[ i ], missing );
        }
        for ( size_t i=0; i<missing.size(); ++i )
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Tooltips: no text for " << missing[ i ] );
        ASSERT_TRUE( missing.empty() );

        // the texts reach the service: a button, a check box of the settings, a tab, the menu bar
        GUI::CTooltipService& service = fixture.context->GetTooltipService();
        ASSERT_TRUE( service.GetTooltipText( *layer.GetForm()->GetWidget( "Ok" ), 2, 2 ).HasSubstr( "Applies" ) >= 0 );
        ASSERT_TRUE( !service.GetTooltipText( *layer.GetGeneralPage()->GetWidget( "DeveloperConsole" ), 2, 2 ).IsNULLOrEmpty() );
        GUI::CWidget* leftTabs = shell.GetMenuBar()->GetParentWidget()->FindWidget( "LeftTabs" );
        ASSERT_TRUE( GUCEF_NULL != leftTabs );
        GUI::CTabControl* tabs = GUI::CWidget::As< GUI::CTabControl >( leftTabs );
        ASSERT_TRUE( GUCEF_NULL != tabs && 3 == tabs->GetTabCount() );
        CORE::CString tabTexts;
        for ( UInt32 t=0; t<3; ++t )
        {
            fixture.context->FlushChanges();
            const GUI::CRect rect = tabs->GetTabRect( t );
            const CORE::CString text = service.GetTooltipText( *tabs, rect.left + rect.width / 2, rect.top + rect.height / 2 );
            ASSERT_TRUE( !text.IsNULLOrEmpty() );
            ASSERT_TRUE( tabTexts.HasSubstr( text ) < 0 );
            tabTexts += text + "\n";
        }
        ASSERT_TRUE( !service.GetTooltipText( *shell.GetMenuBar(), 3, 3 ).IsNULLOrEmpty() );
        // the help gives the brief of the area to a widget that has no text of its own
        GUI::CWidget* unnamed = fixture.context->CreateWidget( "Panel", "NoTextHere", shell.GetBrowser()->GetRootWidget() );
        ASSERT_TRUE( service.GetTooltipText( *unnamed, 1, 1 ).HasSubstr( "content" ) >= 0 || !service.GetTooltipText( *unnamed, 1, 1 ).IsNULLOrEmpty() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}


/*-------------------------------------------------------------------------*/

static void
TestSettingsLayoutLint( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Settings: the pages Input, Authentication and Sources have no layout lint findings of the severities warning and error, at 1280 x 720 and at 1000 x 700, with a store and without" )
    try
    {
        if ( GetMainTestWindow().IsNULL() )
            GUCEF_TESTFW_SKIP( GetNoWindowReason().C_String() );
        for ( UInt32 withStore=0; withStore<2; ++withStore )
        {
            CSettingsFixture fixture( "settings_lint", true, 0 != withStore );
            ASSERT_TRUE( fixture.ready && fixture.sourcesOpened );
            CWorkbenchSettingsLayer& layer = *fixture.layer;
            const char* titles[] = { "Input", "Authentication", "Sources" };
            const Int32 widths[] = { 0, 1000 };
            const Int32 heights[] = { 0, 700 };
            for ( UInt32 size=0; size<2; ++size )
            {
                fixture.context->SetLogicalResolution( widths[ size ], heights[ size ] );
                for ( UInt32 t=0; t<3; ++t )
                {
                    ASSERT_TRUE( layer.Open( titles[ t ] ) );
                    fixture.Frames( 3 );
                    GUI::CWidget* pageRoot = 0 == t ? layer.GetInputPage()->GetRootWidget() : ( 1 == t ? layer.GetAuthPage()->GetRootWidget() : layer.GetSourcesPage()->GetRootWidget() );
                    ASSERT_TRUE( GUCEF_NULL != pageRoot );
                    GUI::CLayoutLintOptions options;
                    options.root = pageRoot;
                    GUI::TLayoutLintFindingVector findings;
                    GUI::CLayoutLint lint;
                    lint.Run( *fixture.context, options, findings );
                    const UInt32 problems = GUI::CLayoutLint::CountAtLeast( findings, GUI::LAYOUTLINTSEVERITY_WARNING );
                    if ( 0 != problems )
                        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Settings lint " << titles[ t ] << " at " << widths[ size ] << "x" << heights[ size ] << ": " << GUI::CLayoutLint::Report( findings, 20 ) );
                    ASSERT_TRUE( 0 == problems );
                    layer.GetForm()->Cancel();
                }
                fixture.context->SetLogicalResolution( 0, 0 );
            }
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
TestSourcesSettings( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Sources: with no store plugin installed the settings page and the sources tab say what to do; the folder of the plugins is empty by default" )
    try
    {
        if ( GetMainTestWindow().IsNULL() )
            GUCEF_TESTFW_SKIP( GetNoWindowReason().C_String() );
        CSettingsFixture fixture( "sources_empty", true, false );
        ASSERT_TRUE( fixture.ready && fixture.sourcesOpened );
        CWorkbenchShell& shell = *fixture.shell;
        CWorkbenchSettingsLayer& layer = *fixture.layer;
        ASSERT_TRUE( fixture.preferences.privatePluginsDir.IsNULLOrEmpty() );
        fixture.Frames( 3 );

        // the tab: the line above the stores tells the person what is missing
        CAssetSourcesPane* sources = shell.GetSources();
        ASSERT_TRUE( 0 == sources->GetOnlineStoreCount() && 1 == sources->GetStoreCount() );
        ASSERT_TRUE( "No online store is installed: see Settings, Sources" == sources->GetOnlineHintText() );
        shell.ShowLeftTab( 2 );
        ASSERT_TRUE( TextOf( sources->GetControl( "OnlineHint" ) ).HasSubstr( "No online store is installed" ) >= 0 && sources->GetControl( "OnlineHint" )->IsEffectivelyVisible() );
        DumpPicture( fixture.Frames( 3 ), "f9_sources_tab_empty" );

        // the page: what to do, in order
        ASSERT_TRUE( layer.Open( "Sources" ) );
        fixture.Frames( 2 );
        CWorkbenchSourcesSettingsForm* page = layer.GetSourcesPage();
        ASSERT_TRUE( 0 == page->GetStoreCount() && page->GetEmptyStateText() == CWorkbenchSourcesSettingsForm::GetNoStoreText() );
        ASSERT_TRUE( page->GetEmptyStateText().HasSubstr( "build them" ) >= 0 && page->GetEmptyStateText().HasSubstr( "Load now" ) >= 0 );
        ASSERT_TRUE( TextOf( page->GetWidget( "EmptyState" ) ) == page->GetEmptyStateText() && page->GetWidget( "EmptyState" )->IsEffectivelyVisible() );
        ASSERT_TRUE( TextOf( page->GetWidget( "PluginsDir" ) ).IsNULLOrEmpty() );
        DumpPicture( fixture.Frames( 3 ), "f9_sources_settings_empty" );

        // the folder is typed and applied: it is in the preferences
        GUI::CEditbox* folder = page->GetWidgetAs< GUI::CEditbox >( "PluginsDir" );
        ASSERT_TRUE( GUCEF_NULL != folder );
        folder->SetText( fixture.dir + "/private" );
        ASSERT_TRUE( page->ApplySettings() );
        ASSERT_TRUE( fixture.dir + "/private" == fixture.preferences.privatePluginsDir );
        layer.GetForm()->Cancel();
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Sources: a service in another process shows its connection state in the line above the stores and in the page; the local service shows none" )
    try
    {
        if ( GetMainTestWindow().IsNULL() )
            GUCEF_TESTFW_SKIP( GetNoWindowReason().C_String() );
        CSettingsFixture fixture( "sources_remote", false, false );
        ASSERT_TRUE( fixture.ready );
        fixture.sourcesSettings.serviceMode = "remote";
        fixture.sourcesSettings.primaryStore.locations[ 0 ].vfsRoot = "avstore/sources_remote";
        fixture.shell->InitializeSources( fixture.sourcesSettings );
        fixture.layer->BindSources();
        fixture.Frames( 2 );
        CAssetSourcesPane* sources = fixture.shell->GetSources();
        ASSERT_TRUE( GUCEF_NULL != sources );
        // the service of this mode is either not in this build or not reachable: the line says so and does not say that no store is installed
        const CORE::CString hint = sources->GetOnlineHintText();
        ASSERT_TRUE( !hint.IsNULLOrEmpty() && hint.HasSubstr( "No online store is installed" ) < 0 );
        ASSERT_TRUE( hint.HasSubstr( "remote" ) >= 0 || hint.HasSubstr( "Remote" ) >= 0 );
        ASSERT_TRUE( TextOf( sources->GetControl( "OnlineHint" ) ) == hint );
        ASSERT_TRUE( fixture.layer->Open( "Sources" ) );
        fixture.Frames( 2 );
        const CORE::CString note = TextOf( fixture.layer->GetSourcesPage()->GetWidget( "EmptyState" ) );
        ASSERT_TRUE( note.HasSubstr( "emote" ) >= 0 || note.HasSubstr( "No online store is installed" ) >= 0 );
        fixture.layer->GetForm()->Cancel();
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Sources: the page lists the stores of the registry, switches one off in the preferences and the tab, and runs the connection test of a store that has one" )
    try
    {
        if ( GetMainTestWindow().IsNULL() )
            GUCEF_TESTFW_SKIP( GetNoWindowReason().C_String() );
        CSettingsFixture fixture( "sources_store", true, true );
        ASSERT_TRUE( fixture.ready && fixture.sourcesOpened );
        CWorkbenchShell& shell = *fixture.shell;
        CWorkbenchSettingsLayer& layer = *fixture.layer;
        fixture.Frames( 3 );
        CAssetSourcesPane* sources = shell.GetSources();
        ASSERT_TRUE( 1 == sources->GetOnlineStoreCount() && sources->GetOnlineHintText().IsNULLOrEmpty() );
        ASSERT_FALSE( sources->GetControl( "OnlineHint" )->IsEffectivelyVisible() );

        ASSERT_TRUE( layer.Open( "Sources" ) );
        fixture.Frames( 2 );
        CWorkbenchSourcesSettingsForm* page = layer.GetSourcesPage();
        ASSERT_TRUE( 1 == page->GetStoreCount() && "fakesrc" == page->GetStoreId( 0 ) && "fakesrc" == page->GetSelectedStoreId() );
        ASSERT_TRUE( page->GetStoreLine( 0 ).HasSubstr( "not signed in" ) >= 0 );
        ASSERT_TRUE( page->GetEmptyStateText().IsNULLOrEmpty() && !page->GetWidget( "EmptyState" )->IsEffectivelyVisible() );
        ASSERT_TRUE( page->GetStoreInfoText().HasSubstr( "an account of fake-auth" ) >= 0 && page->GetStoreInfoText().HasSubstr( "Keyring: not used" ) >= 0 );
        ASSERT_TRUE( page->GetStoreStatusText() == "Not signed in." );
        ASSERT_TRUE( page->GetWidget( "StoreEnabled" )->GetBoolProperty( "Checked" ) && page->GetWidget( "TestConnection" )->GetBoolProperty( "Enabled" ) );
        DumpPicture( fixture.Frames( 3 ), "f9_sources_settings_store" );

        // the connection test: a task on the pool, then the checks and no more
        fixture.Click( page->GetWidget( "TestConnection" ) );
        ASSERT_TRUE( page->IsProbeRunning() || !page->GetProbeText().IsNULLOrEmpty() );
        for ( UInt32 i=0; i<300 && page->IsProbeRunning(); ++i )
        {
            CORE::CCoreGlobal::Instance()->GetPulseGenerator()->ForceDirectPulse();
            fixture.Frames( 1 );
            MT::PrecisionDelay( 5 );
        }
        ASSERT_FALSE( page->IsProbeRunning() );
        ASSERT_TRUE( 1 == fixture.store->probe.runCount );
        ASSERT_TRUE( page->GetProbeText() == "ok: sign in: the session is accepted\nok: library listing: an array of items with an id and a title\nfailed: item details: the field files is missing" );
        ASSERT_TRUE( page->GetStoreStatusText() == "Connection test: 2 of 3 checks ok." );
        ASSERT_TRUE( TextOf( page->GetWidget( "ProbeResult" ) ) == page->GetProbeText() );
        DumpPicture( fixture.Frames( 3 ), "f9_sources_settings_probe" );

        // the store is switched off: the tab does not list it, Cancel puts it back
        fixture.Click( page->GetWidget( "StoreEnabled" ) );
        ASSERT_FALSE( page->IsStoreEnabled( "fakesrc" ) );
        ASSERT_TRUE( page->GetStoreLine( 0 ).HasSubstr( "off" ) >= 0 && fixture.preferences.IsStoreEnabled( "fakesrc" ) && 1 == sources->GetOnlineStoreCount() );
        fixture.Click( layer.GetForm()->GetWidget( "Ok" ) );
        fixture.Frames( 2 );
        ASSERT_TRUE( !fixture.preferences.IsStoreEnabled( "fakesrc" ) && 1 == fixture.preferences.disabledStores.size() );
        ASSERT_TRUE( 0 == sources->GetOnlineStoreCount() && sources->GetOnlineHintText().HasSubstr( "switched off" ) >= 0 );
        ASSERT_TRUE( layer.Open( "Sources" ) );
        fixture.Frames( 2 );
        ASSERT_FALSE( layer.GetSourcesPage()->IsStoreEnabled( "fakesrc" ) );
        fixture.Click( layer.GetSourcesPage()->GetWidget( "StoreEnabled" ) );
        ASSERT_TRUE( layer.GetSourcesPage()->IsStoreEnabled( "fakesrc" ) );
        fixture.Click( layer.GetForm()->GetWidget( "Ok" ) );
        fixture.Frames( 2 );
        ASSERT_TRUE( fixture.preferences.IsStoreEnabled( "fakesrc" ) && 1 == sources->GetOnlineStoreCount() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Sources: a store without a connection test says so; the keyring guidance follows the state of the keyring and opens the window of the sign in" )
    try
    {
        if ( GetMainTestWindow().IsNULL() )
            GUCEF_TESTFW_SKIP( GetNoWindowReason().C_String() );
        SOURCESTEST::TFakeSourceFactory factory;
        ASSETSTORE::TAssetStoreProviderPtr plain = factory.Create();
        ASSETSTORE::CAssetStoreGlobal::Instance()->GetRegistry().AddProvider( plain );
        {
            CSettingsFixture fixture( "sources_noprobe", true, false );
            ASSERT_TRUE( fixture.ready && fixture.sourcesOpened );
            CWorkbenchSettingsLayer& layer = *fixture.layer;
            ASSERT_TRUE( layer.Open( "Sources" ) );
            fixture.Frames( 2 );
            CWorkbenchSourcesSettingsForm* page = layer.GetSourcesPage();
            ASSERT_TRUE( 1 == page->GetStoreCount() );
            ASSERT_FALSE( page->GetWidget( "TestConnection" )->GetBoolProperty( "Enabled" ) );
            ASSERT_TRUE( page->GetStoreStatusText().HasSubstr( "no connection test" ) >= 0 );
            ASSERT_FALSE( page->StartProbe() );

            // first run: no keyring yet (or none possible in this build): what to do, with the button that opens the window for it
            CWorkbenchSignIn* signIn = fixture.shell->GetSignIn();
            ASSERT_TRUE( GUCEF_NULL != signIn );
            if ( !fixture.credentials->IsKeyringAvailable() )
            {
                ASSERT_TRUE( page->GetKeyringGuideText().HasSubstr( "cannot keep saved passwords" ) >= 0 );
            }
            else
            {
                ASSERT_TRUE( AUTHGUI::CSignInSettingsState::KEYRING_NOT_SET_UP == signIn->GetKeyringStatus() );
                ASSERT_TRUE( page->GetKeyringGuideText().HasSubstr( "First run: set a code up" ) >= 0 );
                GUI::CWidget* action = page->GetWidget( "KeyringAction" );
                ASSERT_TRUE( action->IsEffectivelyVisible() && "Set up the keyring..." == TextOf( action ) );
                DumpPicture( fixture.Frames( 3 ), "f9_sources_settings_keyring" );
                fixture.Click( action );
                ASSERT_TRUE( signIn->GetUnlockForm()->IsOpen() && AUTHGUI::CKeyringUnlockForm::MODE_SET_UP == signIn->GetUnlockForm()->GetMode() );
                signIn->GetUnlockForm()->Close();

                // a keyring that is set up and locked: unlock; unlocked: nothing to do
                ASSERT_TRUE( AUTH::KEYRING_OK == fixture.credentials->GetKeyringSession().SetUpCode( AUTH::CSecretString( "a good test code 1234" ) ) );
                ASSERT_TRUE( AUTHGUI::CSignInSettingsState::KEYRING_UNLOCKED == signIn->GetKeyringStatus() );
                fixture.Frames( 2 );
                ASSERT_TRUE( page->GetKeyringGuideText() == "The keyring is unlocked." && !action->IsEffectivelyVisible() );
                fixture.credentials->GetKeyringSession().LockNow();
                fixture.Frames( 2 );
                ASSERT_TRUE( AUTHGUI::CSignInSettingsState::KEYRING_LOCKED == signIn->GetKeyringStatus() );
                ASSERT_TRUE( page->GetKeyringGuideText().HasSubstr( "locked" ) >= 0 && action->IsEffectivelyVisible() && "Unlock the keyring..." == TextOf( action ) );
                fixture.Click( action );
                ASSERT_TRUE( signIn->GetUnlockForm()->IsOpen() && AUTHGUI::CKeyringUnlockForm::MODE_UNLOCK == signIn->GetUnlockForm()->GetMode() );
                signIn->GetUnlockForm()->Close();

                // the page Authentication has the same state and the same actions
                ASSERT_TRUE( layer.GetForm()->SelectPage( "Authentication" ) );
                fixture.Frames( 2 );
                CWorkbenchAuthSettingsForm* auth = layer.GetAuthPage();
                ASSERT_TRUE( auth->GetKeyringStateText().HasSubstr( "locked" ) >= 0 && "Unlock the keyring..." == auth->GetKeyringActionText() );
                DumpPicture( fixture.Frames( 3 ), "f9_settings_authentication" );
                ASSERT_TRUE( signIn->OpenKeyring() );
                signIn->GetUnlockForm()->Close();
                ASSERT_FALSE( signIn->OpenKeyringChangeCode() );
            }
            layer.GetForm()->Cancel();
            // forget the code of the test
            fixture.credentials->GetKeyringSession().LockNow();
        }
        ASSETSTORE::CAssetStoreGlobal::Instance()->GetRegistry().RemoveProvider( SOURCESTEST::CFakeSourceProvider::ProviderIdText );
        plain.Unlink();

        // the page before the sources are made: nothing to open yet
        CSettingsFixture early( "sources_early", false, false );
        ASSERT_TRUE( early.ready );
        ASSERT_TRUE( early.layer->Open( "Sources" ) );
        ASSERT_TRUE( early.layer->GetSourcesPage()->GetKeyringGuideText().HasSubstr( "once the sources" ) >= 0 );
        early.layer->GetForm()->Cancel();
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Sources: the private store plugins of a folder are loaded at the start and on Load now, and what happened is on the page" )
    try
    {
        if ( GetMainTestWindow().IsNULL() )
            GUCEF_TESTFW_SKIP( GetNoWindowReason().C_String() );
        CSettingsFixture fixture( "sources_plugins", true, false );
        ASSERT_TRUE( fixture.ready && fixture.sourcesOpened );
        CWorkbenchSettingsLayer& layer = *fixture.layer;

        // nothing set: nothing to do, nothing wrong
        ASSERT_FALSE( layer.LoadPrivatePlugins() );
        ASSERT_TRUE( !layer.GetPluginReport().directoryGiven && layer.GetPluginReport().loaded.empty() );

        // a folder that is not there
        fixture.preferences.privatePluginsDir = fixture.dir + "/not-there";
        ASSERT_FALSE( layer.LoadPrivatePlugins() );
        ASSERT_TRUE( layer.GetPluginReport().directoryGiven && !layer.GetPluginReport().directoryFound );

        // an empty folder, a folder with a library that is broken
        const CORE::CString plugins = fixture.dir + "/plugins";
        CORE::CreateDirs( plugins );
        fixture.preferences.privatePluginsDir = plugins;
        ASSERT_TRUE( layer.LoadPrivatePlugins() );
        ASSERT_TRUE( layer.GetPluginReport().directoryFound && 0 == layer.GetPluginReport().libraryCount );
        {
            CORE::CFileAccess file( plugins + "/libassetstoreBROKEN.so", "wb" );
            ASSERT_TRUE( file.IsValid() );
            const char text[] = "not a library";
            ASSERT_TRUE( 1 == file.Write( text, sizeof( text ), 1 ) );
        }
        ASSERT_FALSE( layer.LoadPrivatePlugins() );
        ASSERT_TRUE( 1 == layer.GetPluginReport().refused.size() && layer.GetPluginReport().loaded.empty() );

        // Load now on the page uses the folder that is typed, applied or not
        ASSERT_TRUE( layer.Open( "Sources" ) );
        fixture.Frames( 2 );
        CWorkbenchSourcesSettingsForm* page = layer.GetSourcesPage();
        page->GetWidgetAs< GUI::CEditbox >( "PluginsDir" )->SetText( fixture.dir + "/typed-and-missing" );
        fixture.Click( page->GetWidget( "LoadPlugins" ) );
        ASSERT_TRUE( layer.GetPluginReport().directoryGiven && !layer.GetPluginReport().directoryFound && layer.GetPluginReport().directory == fixture.dir + "/typed-and-missing" );
        ASSERT_TRUE( page->GetPluginsStatusText().HasSubstr( "does not exist" ) >= 0 );
        ASSERT_TRUE( TextOf( page->GetWidget( "PluginsStatus" ) ) == page->GetPluginsStatusText() );
        // the typed folder is not a preference until it is applied
        ASSERT_TRUE( fixture.preferences.privatePluginsDir == plugins );
        layer.GetForm()->Cancel();
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
PerformAssetWorkbenchSettingsTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "AssetWorkbenchLib.Settings" );

    TestSettingsWindow();
    TestSettingsInput();
    TestConsoleAndCommands();
    TestHelpAndTooltips();
    TestSettingsLayoutLint();
    TestSourcesSettings();
}
