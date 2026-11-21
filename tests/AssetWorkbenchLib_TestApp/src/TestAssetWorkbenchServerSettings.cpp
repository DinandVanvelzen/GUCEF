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

#ifndef GUCEF_TESTFW_H
#include "gucef_test_framework.h"
#define GUCEF_TESTFW_H
#endif /* GUCEF_TESTFW_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_AUTH_CAUTHGLOBAL_H
#include "gucefAUTH_CAuthGlobal.h"
#define GUCEF_AUTH_CAUTHGLOBAL_H
#endif /* GUCEF_AUTH_CAUTHGLOBAL_H ? */

#ifndef GUCEF_ASSETSTORE_CLOCALASSETSTORESERVICE_H
#include "gucefASSETSTORE_CLocalAssetStoreService.h"
#define GUCEF_ASSETSTORE_CLOCALASSETSTORESERVICE_H
#endif /* GUCEF_ASSETSTORE_CLOCALASSETSTORESERVICE_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETSTOREGLOBAL_H
#include "gucefASSETSTORE_CAssetStoreGlobal.h"
#define GUCEF_ASSETSTORE_CASSETSTOREGLOBAL_H
#endif /* GUCEF_ASSETSTORE_CASSETSTOREGLOBAL_H ? */

#include "AssetWorkbenchLib.h"

#include "TestAssetWorkbenchSupport.h"
#include "TestFakeSourceStore.h"
#include "TestWorkbenchRemoteWorld.h"
#include "TestAssetWorkbenchServerSettings.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;
using namespace GUCEF::ASSETWORKBENCH;

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** The workbench with its settings layer (the console), the credentials of the pairing and the store of this computer */
class CServerSettingsFixture
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
    CAssetSourcesSettings sourcesSettings;
    CORE::CString dir;
    bool ready;

    explicit CServerSettingsFixture( const CORE::CString& name )
        : contextPtr(), context( GUCEF_NULL ), window( GUCEF_NULL ), mounter(), shell( GUCEF_NULL ), preferences(), layer( GUCEF_NULL ), credentials( GUCEF_NULL ),
          sourcesSettings(), dir(), ready( false )
    {
        if ( GetMainTestWindow().IsNULL() )
            return;
        window = AsOgreWindow( GetMainTestWindow() );
        dir = MakeCleanTestDir( name );
        contextPtr = CreateWorkbenchContext();
        context = contextPtr.IsNULL() ? GUCEF_NULL : contextPtr->AsGUIContext();
        if ( GUCEF_NULL == context )
            return;
        shell = GUCEF_NEW CWorkbenchShell( mounter );
        if ( !shell->Initialize( *context, GUCEF_NULL, MakeCleanTestDir( name + "_cache" ) ) )
            return;
        credentials = GUCEF_NEW CWorkbenchCredentials();
        credentials->Open( dir + "/auth" );
        credentials->GetKeyringStore().SetKeyringParameters( 64, 1, 1 );
        credentials->Install();
        shell->SetCredentials( credentials );
        shell->SetAuthManager( &AUTH::CAuthGlobal::Instance()->GetManager() );
        layer = GUCEF_NEW CWorkbenchSettingsLayer( *shell, preferences );
        if ( !layer->Create() )
            return;
        const CORE::CString storeDir = dir + "/store";
        CORE::CreateDirs( storeDir );
        const CORE::CString root = "avstore/" + name;
        sourcesSettings.primaryStore.locations[ 0 ].vfsRoot = root;
        sourcesSettings.primaryStore.locations[ 0 ].fileSystemPath = storeDir;
        sourcesSettings.primaryStore.database.database = root + "/catalogue";
        sourcesSettings.primaryStore.analysisConcurrency = 2;
        ready = shell->InitializeSources( sourcesSettings );
        layer->BindSources();
    }

    ~CServerSettingsFixture()
    {
        GUCEF_DELETE layer;
        if ( GUCEF_NULL != shell )
        {
            shell->Shutdown();
            GUCEF_DELETE shell;
        }
        if ( GUCEF_NULL != credentials )
        {
            credentials->Shutdown();
            GUCEF_DELETE credentials;
        }
        contextPtr.Unlink();
        mounter.UnmountAll();
    }

    void Frames( UInt32 count )
    {
        for ( UInt32 i=0; i<count; ++i )
        {
            CORE::CCoreGlobal::Instance()->GetPulseGenerator()->ForceDirectPulse();
            shell->Update( 5 );
            window->RenderFrame( GUCEF_NULL );
            MT::PrecisionDelay( 2 );
        }
    }

    template < typename TCondition >
    bool WaitFor( TCondition condition, UInt32 maxFrames = 4000 )
    {
        for ( UInt32 i=0; i<maxFrames; ++i )
        {
            if ( condition() )
                return true;
            Frames( 1 );
            MT::PrecisionDelay( 4 );
        }
        return condition();
    }

    CWorkbenchStoreConnection& Connection( void ) { return *shell->GetStoreConnection(); }

    /** Pairs with a window of the host that grants the roles and waits until the workbench works on the remote service */
    bool PairAndConnect( WBREMOTETEST::CRemoteWorld& world, const CORE::CString::StringSet& roles )
    {
        DRNUTILS::CPairingWindowRequest request;
        request.deviceKind = "viewer";
        request.roles = roles;
        CORE::CString windowId;
        CORE::CString code;
        UInt64 expires = 0;
        CORE::CString error;
        if ( !world.host->OpenPairingWindow( request, DRNUTILS::CRemoteServiceHost::LocalAdmin, windowId, code, expires, error ) )
            return false;
        Connection().SetSettings( world.MakeConnectionSettings() );
        AUTH::CSecretString secret( code );
        if ( !Connection().Pair( secret ) )
            return false;
        return WaitFor( [&]() { return CWorkbenchStoreConnection::STATE_CONNECTED == Connection().GetState() && shell->GetSources()->IsUsingRemoteService() &&
                                       shell->IsServerSettingsAvailable(); } );
    }
};

/** The output of the console */
CORE::CString
ConsoleText( CWorkbenchSettingsLayer& layer )
{
    GUI::CTextbox* output = layer.GetConsole()->GetWidgetAs< GUI::CTextbox >( "Output" );
    return GUCEF_NULL != output ? output->GetTextAsString() : CORE::CString::Empty;
}

bool
HasKey( const CServerSettingsForm& form, const char* key )
{
    const CORE::CString::StringVector keys = form.GetKeys();
    for ( size_t k=0; k<keys.size(); ++k )
        if ( keys[ k ] == key )
            return true;
    return false;
}

ASSETSTORE::CLocalAssetStoreService&
ServerOf( WBREMOTETEST::CRemoteWorld& world )
{
    return *static_cast< ASSETSTORE::CLocalAssetStoreService* >( world.serverService.GetPointerAlways() );
}

/*-------------------------------------------------------------------------*/

void
TestServerSettingsPage( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "ServerSettings: connected as an owner device the menu has Server/Settings...; the page shows the settings of the server with their source and apply mode and the sync profile of a store; an edit out of range is refused by its editor, Apply sends one and the server applies it at once, a change by another client is shown, Reset to site value; disconnected the item and the page go" )
    try
    {
        if ( GetMainTestWindow().IsNULL() )
            GUCEF_TESTFW_SKIP( GetNoWindowReason().C_String() );
        // a store of the server for the sync profile on the page
        SOURCESTEST::TFakeSourceFactory factory;
        ASSETSTORE::TAssetStoreProviderPtr provider = factory.Create();
        ASSETSTORE::CAssetStoreGlobal::Instance()->GetRegistry().AddProvider( provider );
        WBREMOTETEST::CRemoteWorld world( "srvsettings" );
        world.serverConfig.reprocess.maxConcurrentJobs = 2;
        world.serverConfig.stateDirectory = world.directory + "/service_state";
        CORE::CString error;
        ASSERT_TRUE( world.StartServer( error ) );
        CServerSettingsFixture fixture( "wbclient_srvsettings" );
        ASSERT_TRUE( fixture.ready );
        CWorkbenchShell& shell = *fixture.shell;

        // not connected: no item, no page
        ASSERT_TRUE( !shell.IsServerSettingsAvailable() && !shell.ExecuteAction( "ServerSettings" ) && GUCEF_NULL == shell.GetServerSettingsController() );
        ASSERT_TRUE( shell.GetMenuBar()->GetProperty( GUI::EWidgetProperty_Items ).HasSubstr( "Server/Settings..." ) < 0 );
        ASSERT_TRUE( "Server/Settings..." == shell.GetMenuPathOfAction( "ServerSettings" ) );

        CORE::CString::StringSet ownerRoles;
        ownerRoles.insert( "owner" );
        ASSERT_TRUE( fixture.PairAndConnect( world, ownerRoles ) );
        ASSERT_TRUE( shell.GetMenuBar()->GetProperty( GUI::EWidgetProperty_Items ).HasSubstr( "Server/Settings..." ) >= 0 );
        ASSERT_TRUE( shell.ExecuteAction( "ServerSettings" ) );
        CServerSettingsForm* form = shell.GetServerSettingsForm();
        ASSERT_TRUE( GUCEF_NULL != form && form->IsOpen() );
        ASSERT_TRUE( fixture.WaitFor( [&]() { return HasKey( *form, "reprocess.maxConcurrentJobs" ) && HasKey( *form, "sync:fakesrc:maxItemsPerDay" ); } ) );
        ASSERT_TRUE( "2" == form->GetValueText( "reprocess.maxConcurrentJobs" ) && "site" == form->GetSourceText( "reprocess.maxConcurrentJobs" ) );
        ASSERT_TRUE( "default" == form->GetSourceText( "reprocess.maxQueuedJobs" ) && "restart" == form->GetApplyModeText( "resumeIngestsOnStart" ) && "live" == form->GetApplyModeText( "downloads.maxAttempts" ) );
        ASSERT_TRUE( HasKey( *form, "signInPromptSeconds" ) && !HasKey( *form, "allowSettingsChanges" ) && !HasKey( *form, "stateDirectory" ) );
        ASSERT_TRUE( form->IsEditingAllowed() && form->IsKeyEditable( "reprocess.maxConcurrentJobs" ) && form->IsKeyEditable( "sync:fakesrc:maxItemsPerDay" ) );
        ASSERT_TRUE( form->GetNoticeText().HasSubstr( "Read only" ) < 0 );

        // the editor of the row refuses a value out of the range of the setting; Apply sends a value that fits
        ASSERT_TRUE( form->SelectKey( "reprocess.maxConcurrentJobs" ) && "reprocess.maxConcurrentJobs" == form->GetSelectedKey() );
        ASSERT_TRUE( form->GetDetailsText().HasSubstr( "0 to 256" ) >= 0 && form->GetDetailsText().HasSubstr( "applies at once" ) >= 0 );
        CORE::CString problem;
        ASSERT_FALSE( form->EditValue( "reprocess.maxConcurrentJobs", "300", &problem ) );
        ASSERT_TRUE( !problem.IsNULLOrEmpty() );
        ASSERT_TRUE( form->EditValue( "reprocess.maxConcurrentJobs", "4", &problem ) && "4" == form->GetValueText( "reprocess.maxConcurrentJobs" ) );
        ASSERT_TRUE( 2 == ServerOf( world ).GetReprocessSettings().maxConcurrentJobs );
        ASSERT_TRUE( form->ApplySelected() );
        ASSERT_TRUE( fixture.WaitFor( [&]() { return 4 == ServerOf( world ).GetReprocessSettings().maxConcurrentJobs && "overridden" == form->GetSourceText( "reprocess.maxConcurrentJobs" ); } ) );
        ASSERT_TRUE( fixture.WaitFor( [&]() { return form->GetOutcomeText().HasSubstr( "applied at once" ) >= 0; } ) );
        // the list shows the description the service answered after the change (at its next refresh)
        ASSERT_TRUE( fixture.WaitFor( [&]() { return "4" == form->GetValueText( "reprocess.maxConcurrentJobs" ); } ) );

        // a member of the sync profile of the store goes through Sync/SetProfile
        ASSERT_TRUE( form->SelectKey( "sync:fakesrc:maxItemsPerDay" ) && form->EditValue( "sync:fakesrc:maxItemsPerDay", "25" ) && form->ApplySelected() );
        ASSETSTORE::CAssetLibrarySyncProfile profile;
        ASSERT_TRUE( fixture.WaitFor( [&]() { return ASSETSTORE::ASSETSTORE_STATUS_OK == ServerOf( world ).GetSyncProfile( "fakesrc", profile ) && 25 == profile.maxItemsPerDay; } ) );
        ASSERT_TRUE( fixture.WaitFor( [&]() { return "25" == form->GetValueText( "sync:fakesrc:maxItemsPerDay" ); } ) );

        // a change by another client (here the site administrator on the server) is shown
        ASSETSTORE::CAssetSettingsRequest other;
        other.caller = "admin";
        other.values[ "downloadStagingKeepHours" ] = "30";
        ASSETSTORE::CAssetSettingsChangeResult changed;
        ASSERT_TRUE( ASSETSTORE::ASSETSTORE_STATUS_OK == ServerOf( world ).SetSettings( CORE::CString::Empty, other, changed ) );
        ASSERT_TRUE( fixture.WaitFor( [&]() { return "30" == form->GetValueText( "downloadStagingKeepHours" ) && "overridden" == form->GetSourceText( "downloadStagingKeepHours" ); } ) );

        // back to the site value
        ASSERT_TRUE( form->SelectKey( "reprocess.maxConcurrentJobs" ) && form->ResetSelected() );
        ASSERT_TRUE( fixture.WaitFor( [&]() { return 2 == ServerOf( world ).GetReprocessSettings().maxConcurrentJobs && "site" == form->GetSourceText( "reprocess.maxConcurrentJobs" ); } ) );
        fixture.Frames( 3 );
        form->Close();

        // disconnected: the store of this computer, no item, no page
        fixture.Connection().Disconnect();
        ASSERT_TRUE( fixture.WaitFor( [&]() { return !shell.IsServerSettingsAvailable(); } ) );
        ASSERT_TRUE( shell.GetMenuBar()->GetProperty( GUI::EWidgetProperty_Items ).HasSubstr( "Server/Settings..." ) < 0 && !shell.ExecuteAction( "ServerSettings" ) );
        ASSETSTORE::CAssetStoreGlobal::Instance()->GetRegistry().RemoveProvider( SOURCESTEST::CFakeSourceProvider::ProviderIdText );
    }
    catch ( ... )
    {
        ASSETSTORE::CAssetStoreGlobal::Instance()->GetRegistry().RemoveProvider( SOURCESTEST::CFakeSourceProvider::ProviderIdText );
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
TestServerSettingsReadOnlyAndConsole( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "ServerSettings: a device without the owner role sees the public settings read only with the reason, a server that does not allow changes makes the page read only for the owner too; the console commands server settings, server set and server reset, and their refusals in words" )
    try
    {
        if ( GetMainTestWindow().IsNULL() )
            GUCEF_TESTFW_SKIP( GetNoWindowReason().C_String() );
        WBREMOTETEST::CRemoteWorld world( "srvsettings_ro" );
        world.bindingSettings.roles.insert( "viewer" );
        world.serverConfig.stateDirectory = world.directory + "/service_state";
        CORE::CString error;
        ASSERT_TRUE( world.StartServer( error ) );

        {
            // a viewer device: the public settings, read only
            CServerSettingsFixture fixture( "wbclient_srvsettings_viewer" );
            ASSERT_TRUE( fixture.ready );
            CORE::CString::StringSet viewerRoles;
            viewerRoles.insert( "viewer" );
            ASSERT_TRUE( fixture.PairAndConnect( world, viewerRoles ) );
            ASSERT_TRUE( fixture.shell->ShowServerSettings() );
            CServerSettingsForm* form = fixture.shell->GetServerSettingsForm();
            ASSERT_TRUE( fixture.WaitFor( [&]() { return HasKey( *form, "reprocess.maxConcurrentJobs" ); } ) );
            ASSERT_TRUE( !form->IsEditingAllowed() && !form->IsKeyEditable( "reprocess.maxConcurrentJobs" ) && !HasKey( *form, "signInPromptSeconds" ) );
            ASSERT_TRUE( form->GetNoticeText().HasSubstr( "Read only" ) == 0 && form->GetNoticeText().HasSubstr( "owner role" ) >= 0 );
            ASSERT_FALSE( form->EditValue( "reprocess.maxConcurrentJobs", "3" ) );
            ASSERT_TRUE( form->SelectKey( "reprocess.maxConcurrentJobs" ) && !form->ApplySelected() && !form->ResetSelected() );
            // the console says the refusal of the server in words
            CWorkbenchConsoleCommands* commands = fixture.layer->GetCommands();
            ASSERT_TRUE( commands->Run( "server set reprocess.maxConcurrentJobs 3" ) );
            ASSERT_TRUE( fixture.WaitFor( [&]() { return ConsoleText( *fixture.layer ).HasSubstr( "owner role" ) >= 0; } ) );
            ASSERT_TRUE( 0 == ServerOf( world ).GetReprocessSettings().maxConcurrentJobs );
            form->Close();
            fixture.Connection().Disconnect();
        }

        // the owner device, the console commands
        CServerSettingsFixture fixture( "wbclient_srvsettings_owner" );
        ASSERT_TRUE( fixture.ready );
        CORE::CString::StringSet ownerRoles;
        ownerRoles.insert( "owner" );
        ASSERT_TRUE( fixture.PairAndConnect( world, ownerRoles ) );
        CWorkbenchSettingsLayer& layer = *fixture.layer;
        CWorkbenchConsoleCommands* commands = layer.GetCommands();
        ASSERT_TRUE( commands->Run( "server settings" ) );
        ASSERT_TRUE( fixture.WaitFor( [&]() { return ConsoleText( layer ).HasSubstr( "reprocess.maxConcurrentJobs = 0" ) >= 0; } ) );
        ASSERT_TRUE( ConsoleText( layer ).HasSubstr( "resumeIngestsOnStart = true" ) >= 0 && ConsoleText( layer ).HasSubstr( "restart" ) >= 0 );
        ASSERT_TRUE( commands->Run( "server set reprocess.maxConcurrentJobs 3" ) );
        ASSERT_TRUE( fixture.WaitFor( [&]() { return 3 == ServerOf( world ).GetReprocessSettings().maxConcurrentJobs && ConsoleText( layer ).HasSubstr( "reprocess.maxConcurrentJobs is 3, applied at once" ) >= 0; } ) );
        ASSERT_TRUE( commands->Run( "server set reprocess.maxConcurrentJobs lots" ) );
        ASSERT_TRUE( fixture.WaitFor( [&]() { return ConsoleText( layer ).HasSubstr( "Nothing was changed" ) >= 0; } ) );
        ASSERT_TRUE( commands->Run( "server set ingestResumeAttempts 6" ) );
        ASSERT_TRUE( fixture.WaitFor( [&]() { return ConsoleText( layer ).HasSubstr( "ingestResumeAttempts is 6, applies after a restart of the service" ) >= 0; } ) );
        ASSERT_TRUE( commands->Run( "server reset reprocess.maxConcurrentJobs" ) );
        ASSERT_TRUE( fixture.WaitFor( [&]() { return 0 == ServerOf( world ).GetReprocessSettings().maxConcurrentJobs; } ) );
        ASSERT_TRUE( commands->Run( "server nonsense" ) && ConsoleText( layer ).HasSubstr( "Usage: server [settings" ) >= 0 );
        ASSERT_TRUE( commands->Run( "server settings open" ) && fixture.shell->GetServerSettingsForm()->IsOpen() );
        fixture.shell->GetServerSettingsForm()->Close();
        fixture.Connection().Disconnect();
        ASSERT_TRUE( fixture.WaitFor( [&]() { return !fixture.shell->IsServerSettingsAvailable(); } ) );

        // a server that does not let clients change settings: read only for a device with the owner role too, with the reason
        world.StopServer();
        world.serverConfig.allowSettingsChanges = false;
        ASSERT_TRUE( world.StartServer( error ) );
        ASSERT_TRUE( fixture.Connection().Connect() );
        ASSERT_TRUE( fixture.WaitFor( [&]() { return fixture.shell->IsServerSettingsAvailable(); } ) );
        ASSERT_TRUE( fixture.shell->ShowServerSettings() );
        CServerSettingsForm* form = fixture.shell->GetServerSettingsForm();
        ASSERT_TRUE( fixture.WaitFor( [&]() { return HasKey( *form, "reprocess.maxConcurrentJobs" ); } ) );
        ASSERT_TRUE( !form->IsEditingAllowed() && !form->IsKeyEditable( "reprocess.maxConcurrentJobs" ) && HasKey( *form, "signInPromptSeconds" ) );
        ASSERT_TRUE( form->GetNoticeText().HasSubstr( "switched off" ) >= 0 );
        ASSERT_TRUE( commands->Run( "server set reprocess.maxConcurrentJobs 5" ) );
        ASSERT_TRUE( fixture.WaitFor( [&]() { return ConsoleText( layer ).HasSubstr( "settings changes by clients are switched off on this server" ) >= 0; } ) );
        ASSERT_TRUE( 0 == ServerOf( world ).GetReprocessSettings().maxConcurrentJobs );
        form->Close();
        fixture.Connection().Disconnect();
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformAssetWorkbenchServerSettingsTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "AssetWorkbenchLib.ServerSettings" );
    TestServerSettingsPage();
    TestServerSettingsReadOnlyAndConsole();
}

/*-------------------------------------------------------------------------*/
