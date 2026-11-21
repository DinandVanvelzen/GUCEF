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

#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include <Ogre.h>
#include <OgreMeshSerializer.h>

#ifndef GUCEF_TESTFW_H
#include "gucef_test_framework.h"
#define GUCEF_TESTFW_H
#endif /* GUCEF_TESTFW_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_AUTH_CMEMORYCREDENTIALSTORE_H
#include "gucefAUTH_CMemoryCredentialStore.h"
#define GUCEF_AUTH_CMEMORYCREDENTIALSTORE_H
#endif /* GUCEF_AUTH_CMEMORYCREDENTIALSTORE_H ? */

#ifndef GUCEF_AUTH_CAUTHGLOBAL_H
#include "gucefAUTH_CAuthGlobal.h"
#define GUCEF_AUTH_CAUTHGLOBAL_H
#endif /* GUCEF_AUTH_CAUTHGLOBAL_H ? */

#ifndef GUCEF_DRNUTILS_CDEVICEPAIRINGCLIENT_H
#include "gucefDRNUTILS_CDevicePairingClient.h"
#define GUCEF_DRNUTILS_CDEVICEPAIRINGCLIENT_H
#endif /* GUCEF_DRNUTILS_CDEVICEPAIRINGCLIENT_H ? */

#ifndef ASSETWORKBENCHLIB_CSTORECONNECTIONTASKCONSUMER_H
#include "AssetWorkbenchLib_CStoreConnectionTaskConsumer.h"
#define ASSETWORKBENCHLIB_CSTORECONNECTIONTASKCONSUMER_H
#endif /* ASSETWORKBENCHLIB_CSTORECONNECTIONTASKCONSUMER_H ? */

#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */

#ifndef GUCEF_ASSETSTORE_CREMOTEASSETFILESYSTEM_H
#include "gucefASSETSTORE_CRemoteAssetFileSystem.h"
#define GUCEF_ASSETSTORE_CREMOTEASSETFILESYSTEM_H
#endif /* GUCEF_ASSETSTORE_CREMOTEASSETFILESYSTEM_H ? */

#ifndef GUCEF_ASSETSTORE_CPACKSTORAGELAYOUT_H
#include "gucefASSETSTORE_CPackStorageLayout.h"
#define GUCEF_ASSETSTORE_CPACKSTORAGELAYOUT_H
#endif /* GUCEF_ASSETSTORE_CPACKSTORAGELAYOUT_H ? */

#ifndef GUCEF_ASSETSTORE_CLOCALASSETSTORESERVICE_H
#include "gucefASSETSTORE_CLocalAssetStoreService.h"
#define GUCEF_ASSETSTORE_CLOCALASSETSTORESERVICE_H
#endif /* GUCEF_ASSETSTORE_CLOCALASSETSTORESERVICE_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETPACKINDEXLOADER_H
#include "gucefASSETSTORE_CAssetPackIndexLoader.h"
#define GUCEF_ASSETSTORE_CASSETPACKINDEXLOADER_H
#endif /* GUCEF_ASSETSTORE_CASSETPACKINDEXLOADER_H ? */

#ifndef GUCEF_GUI_CLAYOUTLINT_H
#include "gucefGUI_CLayoutLint.h"
#define GUCEF_GUI_CLAYOUTLINT_H
#endif /* GUCEF_GUI_CLAYOUTLINT_H ? */

#include "AssetWorkbenchLib.h"
#include "AssetWorkbenchLib_CWorkbenchMaterials.h"
#include "AssetConverterLib.h"
#include "AssetConverterLib_CAssetConverterGlobal.h"
#include "AssetConverterLib_CAssetCacheEntry.h"
#include "AssetConverterLib_CConversionReport.h"
#include "AssetConverterLib_CContentHash.h"

#include "TestAssetWorkbenchSupport.h"
#include "TestWorkbenchRemoteWorld.h"
#include "TestAssetWorkbenchRemoteStore.h"

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

/** The code of a pairing that no server knows: valid characters, wrong value */
const char* const WrongCode = "AAAAAAAAAAAAAAAA";

/** Keeps the states of the connection in the order they came, and counts the other events */
class CConnectionRecorder : public CORE::CObserver
{
    public:

    GUCEF::vector< Int32 > states;
    UInt32 ready;
    UInt32 lost;
    UInt32 paired;
    UInt32 servicesChanged;
    bool serviceValidWhenLost;

    explicit CConnectionRecorder( CWorkbenchStoreConnection& connection )
        : CORE::CObserver(), states(), ready( 0 ), lost( 0 ), paired( 0 ), servicesChanged( 0 ), serviceValidWhenLost( false ), m_connection( connection )
    {
        SubscribeTo( &connection );
    }

    virtual ~CConnectionRecorder() { UnsubscribeAllFromObserver(); }

    virtual void OnNotify( CORE::CNotifier* notifier, const CORE::CEvent& id, CORE::CICloneable* eventData ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( CWorkbenchStoreConnection::StateChangedEvent == id )
        {
            const CORE::TCloneableInt32* state = static_cast< const CORE::TCloneableInt32* >( eventData );
            if ( GUCEF_NULL != state && ( states.empty() || states.back() != state->GetData() ) )
                states.push_back( state->GetData() );
        }
        else
        if ( CWorkbenchStoreConnection::ServiceReadyEvent == id )
            ++ready;
        else
        if ( CWorkbenchStoreConnection::ServiceLostEvent == id )
        {
            ++lost;
            serviceValidWhenLost = !m_connection.GetService().IsNULL();
        }
        else
        if ( CWorkbenchStoreConnection::PairedEvent == id )
            ++paired;
        else
        if ( CWorkbenchStoreConnection::ServicesChangedEvent == id )
            ++servicesChanged;
    }

    bool Saw( CWorkbenchStoreConnection::TState state ) const
    {
        for ( size_t i=0; i<states.size(); ++i )
            if ( states[ i ] == (Int32) state )
                return true;
        return false;
    }

    private:

    CWorkbenchStoreConnection& m_connection;
};

/** Counts how often the sources page changed its service */
class CServiceChangedCounter : public CORE::CObserver
{
    public:

    UInt32 count;

    explicit CServiceChangedCounter( CAssetSourcesPane& pane ) : CORE::CObserver(), count( 0 ) { SubscribeTo( &pane, CAssetSourcesPane::ServiceChangedEvent ); }

    virtual ~CServiceChangedCounter() { UnsubscribeAllFromObserver(); }

    virtual void OnNotify( CORE::CNotifier* notifier, const CORE::CEvent& id, CORE::CICloneable* eventData ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( CAssetSourcesPane::ServiceChangedEvent == id )
            ++count;
    }
};

/** Pumps the pulse generator (the host, the connection and the events live on it) until the condition is true, at most the time given */
template < typename TCondition >
bool
PumpUntilTrue( WBREMOTETEST::CRemoteWorld& world, TCondition condition, UInt32 maxMs = 20000 )
{
    const UInt64 resolution = MT::PrecisionTimerResolution();
    const UInt64 start = MT::PrecisionTickCount();
    while ( ( MT::PrecisionTickCount() - start ) * 1000 / resolution < maxMs )
    {
        world.pulse->ForceDirectPulse();
        if ( condition() )
            return true;
        MT::PrecisionDelay( 3 );
    }
    return condition();
}

/** The workbench with its settings layer, its credential stores and its primary store (of this process), as the application makes it, and a window */
class CRemoteFixture
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
    bool sourcesOpened;
    bool ready;

    CRemoteFixture( const CORE::CString& name )
        : contextPtr(), context( GUCEF_NULL ), window( GUCEF_NULL ), mounter(), shell( GUCEF_NULL ), preferences(), layer( GUCEF_NULL ), credentials( GUCEF_NULL ),
          sourcesSettings(), dir(), sourcesOpened( false ), ready( false )
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
        // the credential stores of the workbench, in a folder of the test: the pairing is kept in the key file store of the sessions
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
        sourcesOpened = shell->InitializeSources( sourcesSettings );
        layer->BindSources();
        ready = true;
    }

    ~CRemoteFixture()
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

    IMAGE::TPixelMapPtr Frames( UInt32 count )
    {
        IMAGE::TPixelMapPtr pixels;
        for ( UInt32 i=0; i<count; ++i )
        {
            CORE::CCoreGlobal::Instance()->GetPulseGenerator()->ForceDirectPulse();
            shell->Update( 5 );
            window->RenderFrame( &pixels );
            MT::PrecisionDelay( 2 );
        }
        return pixels;
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
    CAssetSourcesPane& Pane( void ) { return *shell->GetSources(); }

    /** Pairs with the code through the connection and waits for it to end (connected, or the reason) */
    bool PairWithCode( WBREMOTETEST::CRemoteWorld& world, const CORE::CString& code )
    {
        AUTH::CSecretString secret( code );
        if ( !Connection().Pair( secret ) )
            return false;
        return WaitFor( [&]() { return !Connection().IsBusy() && CWorkbenchStoreConnection::STATE_CONNECTING != Connection().GetState() && CWorkbenchStoreConnection::STATE_PAIRING != Connection().GetState(); } );
    }
};

/** The output of the console */
CORE::CString
ConsoleText( CWorkbenchSettingsLayer& layer )
{
    GUI::CTextbox* output = layer.GetConsole()->GetWidgetAs< GUI::CTextbox >( "Output" );
    return GUCEF_NULL != output ? output->GetTextAsString() : CORE::CString::Empty;
}

CORE::CString
TextOf( GUI::CWidget* widget )
{
    return GUCEF_NULL != widget ? widget->GetProperty( GUI::EWidgetProperty_Text ) : CORE::CString( "no widget" );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

static void
TestRemoteStorePreferences( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "RemoteStore: the settings of the remote store default to worker.lan:47400, pairing port 47401 and the look on the network, survive a settings tree, and addresses are cleaned" )
    try
    {
        CWorkbenchPreferences preferences;
        ASSERT_TRUE( "worker.lan:47400" == preferences.remoteEndpoint && 47401 == preferences.remotePairingPort && preferences.remoteLanDiscovery && !preferences.remoteConnectAtStart );
        preferences.remoteEndpoint = "10.0.0.7:47400";
        preferences.remotePairingPort = 47555;
        preferences.remoteLanDiscovery = false;
        preferences.remoteConnectAtStart = true;
        CORE::CDataNode section( "AssetWorkbench" );
        preferences.Save( section );
        CWorkbenchPreferences loaded;
        loaded.Load( section );
        ASSERT_TRUE( "10.0.0.7:47400" == loaded.remoteEndpoint && 47555 == loaded.remotePairingPort && !loaded.remoteLanDiscovery && loaded.remoteConnectAtStart );
        // settings of before the remote store keep the defaults
        CWorkbenchPreferences old;
        old.Load( CORE::CDataNode( "AssetWorkbench" ) );
        ASSERT_TRUE( "worker.lan:47400" == old.remoteEndpoint && old.remoteLanDiscovery );
        // no secret is among the settings
        ASSERT_TRUE( GUCEF_NULL == section.FindChild( "PairingCode" ) && !section.IsAttribute( "PairingCode" ) );

        ASSERT_TRUE( "worker.lan:47400" == CWorkbenchStoreConnection::NormaliseEndpoint( "  tcp://worker.lan:47400/ " ) );
        ASSERT_TRUE( "worker.lan" == CWorkbenchStoreConnection::GetHostOf( "tcp://worker.lan:47400" ) && "worker.lan" == CWorkbenchStoreConnection::GetHostOf( "worker.lan" ) );

        // the words of a refused pairing say what to do
        ASSERT_TRUE( CStoreConnectionTaskConsumer::DescribePairingFailure( DRNUTILS::CDevicePairingClient::PAIRINGFAILURE_WRONG_CODE, CORE::CString::Empty ).HasSubstr( "not accepted" ) >= 0 );
        ASSERT_TRUE( CStoreConnectionTaskConsumer::DescribePairingFailure( DRNUTILS::CDevicePairingClient::PAIRINGFAILURE_UNREACHABLE, CORE::CString::Empty ).HasSubstr( "Open pairing on the server" ) >= 0 );
        ASSERT_TRUE( CStoreConnectionTaskConsumer::DescribePairingFailure( DRNUTILS::CDevicePairingClient::PAIRINGFAILURE_SERVER_PROOF, CORE::CString::Empty ).HasSubstr( "not trusted" ) >= 0 );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
TestRemoteStoreDiscovery( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "RemoteStore: a store service that announces itself is found on the network (here the loopback interface) and offered, never trusted; choosing it makes its address the address and its pairing port the pairing port" )
    try
    {
        WBREMOTETEST::CRemoteWorld world( "discover", true );
        CORE::CString error;
        ASSERT_TRUE( world.StartServer( error ) );
        AUTH::CMemoryCredentialStore store;
        CWorkbenchStoreConnection connection;
        ASSERT_TRUE( connection.Initialize( &store ) );
        CConnectionRecorder recorder( connection );
        connection.SetSettings( world.MakeConnectionSettings( true ) );
        // the endpoint of the settings stays the primary path: it is cleared here so that the found service is the only address
        connection.SetEndpoint( CORE::CString::Empty );
        ASSERT_TRUE( CWorkbenchStoreConnection::STATE_DISCONNECTED == connection.GetState() );

        ASSERT_TRUE( connection.Discover() && connection.IsBusy() );
        ASSERT_TRUE( CWorkbenchStoreConnection::STATE_DISCOVERING == connection.GetState() && connection.GetStateText().HasSubstr( "Looking" ) >= 0 );
        ASSERT_TRUE( PumpUntilTrue( world, [&]() { return !connection.IsBusy(); }, 15000 ) );
        ASSERT_TRUE( CWorkbenchStoreConnection::STATE_DISCONNECTED == connection.GetState() && 1 == recorder.servicesChanged );
        const CWorkbenchStoreConnection::TDiscoveredVector& found = connection.GetDiscoveredServices();
        ASSERT_TRUE( 1 == found.size() );
        if ( 1 == found.size() )
        {
            ASSERT_TRUE( "Workbench test store" == found[ 0 ].displayName );
            ASSERT_TRUE( world.GetEndpoint() == found[ 0 ].endpoint && "127.0.0.1" == found[ 0 ].host );
            ASSERT_TRUE( world.hostSettings.pairingPort == found[ 0 ].pairingPort );
            ASSERT_FALSE( found[ 0 ].keyFingerprint.IsNULLOrEmpty() );
            ASSERT_TRUE( CWorkbenchStoreConnection::DescribeService( found[ 0 ] ).HasSubstr( "Workbench test store" ) >= 0 );
            ASSERT_TRUE( connection.SelectService( 0 ) && world.GetEndpoint() == connection.GetEndpoint() );
            ASSERT_TRUE( "tcp://127.0.0.1:" + CORE::ToString( world.hostSettings.pairingPort ) == connection.GetPairingEndpoint() );
        }
        ASSERT_FALSE( connection.SelectService( 5 ) );
        // being found does not pair or connect anything
        ASSERT_FALSE( connection.IsPaired() || connection.IsConnected() );

        // the look can be switched off
        CWorkbenchStoreConnection::SSettings off = world.MakeConnectionSettings( false );
        connection.SetSettings( off );
        ASSERT_FALSE( connection.Discover() );
        connection.Shutdown();
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
TestRemoteStoreConnection( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "RemoteStore: a wrong code is refused in plain words, the right code pairs and connects, the service describes itself, a dropped link goes offline and comes back by itself, the search goes through the service, disconnect and connect again with the kept pairing" )
    try
    {
        WBREMOTETEST::CRemoteWorld world( "connect" );
        CORE::CString error;
        ASSERT_TRUE( world.StartServer( error ) && world.AddPack( "Sword Pack" ) );
        AUTH::CMemoryCredentialStore store;
        CWorkbenchStoreConnection connection;
        ASSERT_TRUE( connection.Initialize( &store ) );
        CConnectionRecorder recorder( connection );
        CORE::CString code;
        ASSERT_TRUE( world.OpenPairing( code, error ) );
        connection.SetSettings( world.MakeConnectionSettings() );

        // not paired yet: connecting says what to do, and nothing is tried
        ASSERT_FALSE( connection.IsPaired() );
        ASSERT_FALSE( connection.Connect() );
        ASSERT_TRUE( connection.GetErrorText().HasSubstr( "not paired" ) >= 0 && connection.GetErrorText().HasSubstr( "pairing" ) >= 0 );
        ASSERT_TRUE( CWorkbenchStoreConnection::STATE_DISCONNECTED == connection.GetState() );

        // a wrong code
        AUTH::CSecretString wrong( ( CORE::CString( WrongCode ) ) );
        ASSERT_TRUE( connection.Pair( wrong ) );
        ASSERT_TRUE( CWorkbenchStoreConnection::STATE_PAIRING == connection.GetState() && connection.IsBusy() );
        ASSERT_TRUE( PumpUntilTrue( world, [&]() { return !connection.IsBusy() && CWorkbenchStoreConnection::STATE_PAIRING != connection.GetState(); } ) );
        ASSERT_TRUE( CWorkbenchStoreConnection::STATE_DISCONNECTED == connection.GetState() && !connection.IsPaired() && recorder.Saw( CWorkbenchStoreConnection::STATE_PAIRING ) );
        ASSERT_TRUE( connection.GetErrorText().HasSubstr( "not accepted" ) >= 0 && connection.GetErrorText().HasSubstr( code ) < 0 );
        ASSERT_TRUE( 0 == recorder.paired && 0 == recorder.ready );

        // no code at all
        AUTH::CSecretString none;
        ASSERT_FALSE( connection.Pair( none ) );
        ASSERT_TRUE( connection.GetErrorText().HasSubstr( "code" ) >= 0 );

        // the right code: paired, and connected after it
        AUTH::CSecretString secret( code );
        ASSERT_TRUE( connection.Pair( secret ) );
        ASSERT_TRUE( PumpUntilTrue( world, [&]() { return CWorkbenchStoreConnection::STATE_CONNECTED == connection.GetState(); } ) );
        ASSERT_TRUE( connection.IsPaired() && connection.IsConnected() && connection.GetErrorText().IsNULLOrEmpty() );
        ASSERT_TRUE( 1 == recorder.paired && 1 == recorder.ready && recorder.Saw( CWorkbenchStoreConnection::STATE_CONNECTING ) );
        ASSERT_TRUE( "gucef.assetstore.test" == connection.GetServerName() && !connection.GetServerVersion().IsNULLOrEmpty() );
        ASSERT_TRUE( connection.GetServiceInfo().HasCapability( "catalogue" ) && connection.GetServiceInfo().roles.count( "owner" ) > 0 );
        ASSERT_TRUE( connection.GetServerSummary().HasSubstr( "gucef.assetstore.test" ) >= 0 && connection.GetServerSummary().HasSubstr( "catalogue" ) >= 0 );
        ASSERT_TRUE( connection.GetStateText().HasSubstr( "Connected to 127.0.0.1" ) >= 0 && connection.GetStoreLabel().HasSubstr( "gucef.assetstore.test" ) >= 0 );
        // the code is nowhere in what the connection tells
        ASSERT_TRUE( connection.GetStateText().HasSubstr( code ) < 0 && connection.GetServerSummary().HasSubstr( code ) < 0 && connection.GetStoreLabel().HasSubstr( code ) < 0 );

        // the search goes to the service of the server: its pack is found
        ASSETSTORE::TAssetStoreServicePtr service = connection.GetService();
        ASSERT_TRUE( !service.IsNULL() && !service->IsInProcess() && "remote" == service->GetType() );
        ASSETSTORE::CAssetSearchQuery query;
        query.text = "sword";
        ASSETSTORE::CAssetSearchResultPage page;
        ASSERT_TRUE( ASSETSTORE::ASSETSTORE_STATUS_OK == service->Search( query, page ) && 2 == page.total );

        // the link drops: offline while it is down, then connected again by itself, with the same service
        DRNUTILS::CRemoteServiceConnection* link = static_cast< ASSETSTORE::CRemoteAssetStoreService* >( service.GetPointerAlways() )->GetConnection();
        ASSERT_TRUE( GUCEF_NULL != link );
        link->DropLink();
        ASSERT_TRUE( PumpUntilTrue( world, [&]() { return recorder.Saw( CWorkbenchStoreConnection::STATE_OFFLINE ); }, 8000 ) );
        ASSERT_TRUE( PumpUntilTrue( world, [&]() { return CWorkbenchStoreConnection::STATE_CONNECTED == connection.GetState(); }, 15000 ) );
        ASSERT_TRUE( service.GetPointerAlways() == connection.GetService().GetPointerAlways() && 1 == recorder.ready );
        ASSERT_TRUE( link->GetReconnectAttemptCount() >= 1 );
        page = ASSETSTORE::CAssetSearchResultPage();
        ASSERT_TRUE( ASSETSTORE::ASSETSTORE_STATUS_OK == service->Search( query, page ) && 2 == page.total );
        service.Unlink();

        // disconnect: the application is told while the service is still there, then it is gone; the pairing is kept
        connection.Disconnect();
        ASSERT_TRUE( 1 == recorder.lost && recorder.serviceValidWhenLost );
        ASSERT_TRUE( CWorkbenchStoreConnection::STATE_DISCONNECTED == connection.GetState() && connection.GetService().IsNULL() && connection.IsPaired() );

        // connect again with what was kept: no code
        ASSERT_TRUE( connection.Connect() );
        ASSERT_TRUE( PumpUntilTrue( world, [&]() { return CWorkbenchStoreConnection::STATE_CONNECTED == connection.GetState(); } ) );
        ASSERT_TRUE( 2 == recorder.ready );

        // another connection object with the same store is paired already
        connection.Disconnect();
        CWorkbenchStoreConnection second;
        ASSERT_TRUE( second.Initialize( &store ) && second.IsPaired() );
        second.Shutdown();

        // forgetting the pairing: connect says so again
        ASSERT_TRUE( connection.ForgetPairing() && !connection.IsPaired() );
        ASSERT_FALSE( connection.Connect() );
        connection.Shutdown();
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
TestRemoteStoreUnreachable( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "RemoteStore: a paired computer whose service is not there is told so in plain words and tries again with a backoff until the service is back or the person disconnects" )
    try
    {
        WBREMOTETEST::CRemoteWorld world( "unreachable" );
        CORE::CString error;
        ASSERT_TRUE( world.StartServer( error ) );
        AUTH::CMemoryCredentialStore store;
        CWorkbenchStoreConnection connection;
        ASSERT_TRUE( connection.Initialize( &store ) );
        CConnectionRecorder recorder( connection );
        CORE::CString code;
        ASSERT_TRUE( world.OpenPairing( code, error ) );
        connection.SetSettings( world.MakeConnectionSettings() );
        AUTH::CSecretString secret( code );
        ASSERT_TRUE( connection.Pair( secret, false ) );
        ASSERT_TRUE( PumpUntilTrue( world, [&]() { return !connection.IsBusy() && connection.IsPaired(); } ) );
        ASSERT_TRUE( CWorkbenchStoreConnection::STATE_DISCONNECTED == connection.GetState() && 0 == recorder.ready );

        // the service goes away (its host stops)
        world.StopHost();
        CWorkbenchStoreConnection::SSettings settings = connection.GetSettings();
        settings.connectTimeoutMs = 2000;
        connection.SetSettings( settings );
        ASSERT_TRUE( connection.Connect() );
        ASSERT_TRUE( PumpUntilTrue( world, [&]() { return connection.GetErrorText().HasSubstr( "cannot be reached" ) >= 0; }, 15000 ) );
        ASSERT_TRUE( CWorkbenchStoreConnection::STATE_CONNECTING == connection.GetState() && connection.GetStateText().HasSubstr( "Trying again" ) >= 0 );
        ASSERT_TRUE( connection.GetStateText().HasSubstr( "Cannot reach" ) >= 0 );

        // the service comes back: the next attempt connects
        ASSERT_TRUE( world.StartHost( error ) );
        ASSERT_TRUE( PumpUntilTrue( world, [&]() { return CWorkbenchStoreConnection::STATE_CONNECTED == connection.GetState(); }, 30000 ) );
        ASSERT_TRUE( 1 == recorder.ready && connection.GetErrorText().IsNULLOrEmpty() );

        // a person who disconnects ends the attempts
        connection.Disconnect();
        world.StopHost();
        ASSERT_TRUE( connection.Connect() );
        ASSERT_TRUE( PumpUntilTrue( world, [&]() { return connection.GetStateText().HasSubstr( "Trying again" ) >= 0; }, 15000 ) );
        connection.Disconnect();
        ASSERT_TRUE( CWorkbenchStoreConnection::STATE_DISCONNECTED == connection.GetState() );
        world.Pump( 3000 );
        ASSERT_TRUE( CWorkbenchStoreConnection::STATE_DISCONNECTED == connection.GetState() && 1 == recorder.ready );
        connection.Shutdown();
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
TestRemoteStoreWorkbench( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "RemoteStore: connected, the Sources tab, the search, the details, the transfers and the bulk conversion work on the remote service and the tab says which store it is; a drop keeps the view and the service; disconnect falls back to the store of this computer" )
    try
    {
        if ( GetMainTestWindow().IsNULL() )
            GUCEF_TESTFW_SKIP( GetNoWindowReason().C_String() );
        WBREMOTETEST::CRemoteWorld world( "workbench" );
        CORE::CString error;
        ASSERT_TRUE( world.StartServer( error ) && world.AddPack( "Sword Pack" ) );
        CRemoteFixture fixture( "wbclient_wb" );
        ASSERT_TRUE( fixture.ready && fixture.sourcesOpened );
        CAssetSourcesPane& pane = fixture.Pane();
        CWorkbenchStoreConnection& connection = fixture.Connection();
        CConnectionRecorder recorder( connection );
        CServiceChangedCounter changed( pane );

        // before: the store of this computer, nothing to tell
        ASSETSTORE::CIAssetStoreService* local = pane.GetService();
        ASSERT_TRUE( GUCEF_NULL != local && local->IsInProcess() && !pane.IsUsingRemoteService() && pane.GetServiceLabel().IsNULLOrEmpty() );
        GUI::CAssetSearchForm* form = pane.GetSearchForm();
        form->SetText( "sword" );
        ASSERT_TRUE( fixture.WaitFor( [&]() { return !form->IsSearching(); } ) );
        ASSERT_TRUE( 0 == form->GetPage().total );
        ASSERT_TRUE( connection.GetSettings().endpoint.IsNULLOrEmpty() );

        // the pairing is kept in the key file store of the workbench credentials
        CORE::CString code;
        ASSERT_TRUE( world.OpenPairing( code, error ) );
        connection.SetSettings( world.MakeConnectionSettings() );
        ASSERT_TRUE( fixture.PairWithCode( world, code ) );
        ASSERT_TRUE( CWorkbenchStoreConnection::STATE_CONNECTED == connection.GetState() && connection.IsPaired() );
        ASSERT_TRUE( 1 == recorder.ready );

        // the page works on the remote service now
        ASSERT_TRUE( pane.IsUsingRemoteService() && pane.GetService() == connection.GetService().GetPointerAlways() && 1 == changed.count );
        ASSERT_TRUE( pane.GetServiceLabel().HasSubstr( world.GetEndpoint() ) >= 0 );
        fixture.Frames( 3 );
        ASSERT_TRUE( pane.GetServiceStateText().HasSubstr( "Remote store " + world.GetEndpoint() ) == 0 && pane.GetServiceStateText().HasSubstr( "connected" ) >= 0 );
        GUI::CWidget* hint = pane.GetControl( "OnlineHint" );
        ASSERT_TRUE( GUCEF_NULL != hint && hint->GetBoolProperty( "Visible" ) && TextOf( hint ).HasSubstr( world.GetEndpoint() ) >= 0 );
        ASSERT_TRUE( pane.GetStoreCount() >= 1 && "Primary store" == pane.GetStoreLine( 0 ) );

        // the search is the search of the server: its pack, which this computer does not have
        form->SetText( "sword" );
        form->Search();
        ASSERT_TRUE( fixture.WaitFor( [&]() { return !form->IsSearching() && 2 == form->GetPage().total; } ) );
        CORE::CString modelKey;
        for ( UInt32 i=0; i<form->GetShownRowCount(); ++i )
            if ( "sword_model" == form->GetShownRow( i ).title )
                modelKey = form->GetShownRow( i ).key;
        ASSERT_TRUE( !modelKey.IsNULLOrEmpty() );

        // the details: the asset and its pack, asked of the service without the interface waiting
        ASSERT_TRUE( form->SelectResult( modelKey ) );
        const CORE::CDataNode& data = fixture.shell->GetInspector()->GetData();
        ASSERT_TRUE( fixture.WaitFor( [&]() { return "sword_model" == data.GetAttributeValue( "Name" ).AsString( CORE::CString::Empty, true ); } ) );
        const CORE::CDataNode* packNode = data.FindChild( "Pack" );
        ASSERT_TRUE( GUCEF_NULL != packNode && "Sword Pack" == packNode->GetAttributeValue( "Title" ).AsString( CORE::CString::Empty, true ) );

        // the transfers and the jobs are the jobs of the service; the bulk conversion is a controller of it
        ASSERT_TRUE( GUCEF_NULL != pane.GetTransferModel() && GUCEF_NULL != fixture.shell->GetJobsForm() );
        ASSERT_TRUE( !pane.GetSelectedPackId().IsNULLOrEmpty() );
        ASSERT_TRUE( GUCEF_NULL != fixture.shell->GetBulkConversionForm() );

        // adding a pack of this computer to a remote store is not possible yet and says so, instead of ingesting into the wrong store
        ASSERT_FALSE( pane.AddDiskPack( world.WritePack( "Other" ) ) );
        ASSERT_TRUE( pane.GetStartedIngestCount() == 0 );

        // the link drops: the service stays in use, the view stays, the search works again when it is back
        DRNUTILS::CRemoteServiceConnection* link = static_cast< ASSETSTORE::CRemoteAssetStoreService* >( pane.GetService() )->GetConnection();
        ASSERT_TRUE( GUCEF_NULL != link );
        link->DropLink();
        ASSERT_TRUE( fixture.WaitFor( [&]() { return recorder.Saw( CWorkbenchStoreConnection::STATE_OFFLINE ); } ) );
        ASSERT_TRUE( pane.IsUsingRemoteService() && 2 == form->GetPage().total );
        ASSERT_TRUE( fixture.WaitFor( [&]() { return CWorkbenchStoreConnection::STATE_CONNECTED == connection.GetState(); }, 6000 ) );
        ASSERT_TRUE( pane.IsUsingRemoteService() && 1 == recorder.ready && 1 == changed.count );
        form->SetText( "sword model" );
        form->Search();
        ASSERT_TRUE( fixture.WaitFor( [&]() { return !form->IsSearching() && form->GetPage().total >= 1; } ) );

        // disconnect: the store of this computer again, with its own (empty) catalogue
        connection.Disconnect();
        ASSERT_TRUE( !pane.IsUsingRemoteService() && pane.GetService() == local && 2 == changed.count && pane.GetServiceLabel().IsNULLOrEmpty() );
        form->SetText( "sword" );
        form->Search();
        ASSERT_TRUE( fixture.WaitFor( [&]() { return !form->IsSearching(); } ) );
        ASSERT_TRUE( 0 == form->GetPage().total && pane.GetServiceStateText().IsNULLOrEmpty() );
        fixture.Frames( 3 );

        // and the remote store again, with what was kept: no code
        ASSERT_TRUE( connection.Connect() );
        ASSERT_TRUE( fixture.WaitFor( [&]() { return CWorkbenchStoreConnection::STATE_CONNECTED == connection.GetState() && pane.IsUsingRemoteService(); } ) );
        form->SetText( "sword" );
        form->Search();
        ASSERT_TRUE( fixture.WaitFor( [&]() { return !form->IsSearching() && 2 == form->GetPage().total; } ) );
        IMAGE::TPixelMapPtr pixels = fixture.Frames( 3 );
        DumpPicture( pixels, "remote_sources_" + CORE::CString( GetTestRenderer() ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
TestRemoteStorePage( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "RemoteStore: the settings page Remote store and the menu entry File, Connect to store: the address of the settings, the wrong code in words, the field emptied, pairing and connecting, the service shown, Disconnect, Apply keeps the address, and no layout lint findings" )
    try
    {
        if ( GetMainTestWindow().IsNULL() )
            GUCEF_TESTFW_SKIP( GetNoWindowReason().C_String() );
        WBREMOTETEST::CRemoteWorld world( "page" );
        CORE::CString error;
        ASSERT_TRUE( world.StartServer( error ) );
        CRemoteFixture fixture( "wbclient_page" );
        ASSERT_TRUE( fixture.ready && fixture.sourcesOpened );
        CWorkbenchShell& shell = *fixture.shell;
        CWorkbenchSettingsLayer& layer = *fixture.layer;
        CWorkbenchRemoteStoreForm* page = layer.GetRemotePage();
        ASSERT_TRUE( GUCEF_NULL != page );
        CWorkbenchStoreConnection& connection = fixture.Connection();

        // the menu has the entry and it opens the settings at the page
        ASSERT_TRUE( shell.GetMenuBar()->GetProperty( GUI::EWidgetProperty_Items ).HasSubstr( "File/Connect to store..." ) >= 0 );
        ASSERT_TRUE( "File/Connect to store..." == shell.GetMenuPathOfAction( CWorkbenchShell::ActionConnectStore ) );
        ASSERT_TRUE( shell.ExecuteAction( CWorkbenchShell::ActionConnectStore ) && layer.IsOpen() );
        fixture.Frames( 3 );
        ASSERT_TRUE( layer.GetForm()->GetWidget( "Pages" )->GetProperty( "Tabs" ).HasSubstr( "Remote store" ) >= 0 );
        ASSERT_TRUE( page->GetRootWidget()->GetBoolProperty( "Visible" ) );

        // the address of the settings is the default one, the look on the network is on, and nothing is connected
        ASSERT_TRUE( "worker.lan:47400" == page->GetEndpointText() && page->IsLanDiscoveryChecked() );
        ASSERT_TRUE( page->GetStateText().HasSubstr( "Not connected" ) >= 0 && page->GetErrorText().IsNULLOrEmpty() && page->GetServerInfoText().HasSubstr( "Not connected" ) >= 0 );

        // a service that is not there: the connect says that this computer is not paired
        ASSERT_TRUE( page->SetEndpointText( world.GetEndpoint() ) );
        ASSERT_FALSE( page->RequestConnect() );
        ASSERT_TRUE( page->GetErrorText().HasSubstr( "not paired" ) >= 0 );

        // a wrong code: refused in words, the field is empty afterwards
        CORE::CString code;
        ASSERT_TRUE( world.OpenPairing( code, error ) );
        CWorkbenchStoreConnection::SSettings settings = world.MakeConnectionSettings();
        connection.SetSettings( settings );
        ASSERT_TRUE( page->SetPairingCode( WrongCode ) && page->HasPairingCode() );
        ASSERT_TRUE( page->RequestPair() && !page->HasPairingCode() );
        ASSERT_TRUE( page->GetStateText().HasSubstr( "Pairing" ) >= 0 );
        ASSERT_TRUE( fixture.WaitFor( [&]() { return page->GetErrorText().HasSubstr( "not accepted" ) >= 0; } ) );
        ASSERT_TRUE( page->GetStateText().HasSubstr( "Not connected" ) >= 0 && TextOf( page->GetWidget( "StateText" ) ).HasSubstr( code ) < 0 );

        // the right code: paired and connected, the service shown
        ASSERT_TRUE( page->SetPairingCode( code ) && page->RequestPair() && !page->HasPairingCode() );
        ASSERT_TRUE( fixture.WaitFor( [&]() { return CWorkbenchStoreConnection::STATE_CONNECTED == connection.GetState(); } ) );
        fixture.Frames( 3 );
        ASSERT_TRUE( page->GetStateText().HasSubstr( "Connected to" ) >= 0 && page->GetErrorText().IsNULLOrEmpty() );
        ASSERT_TRUE( page->GetServerInfoText().HasSubstr( "gucef.assetstore.test" ) >= 0 && page->GetServerInfoText().HasSubstr( "catalogue" ) >= 0 );
        ASSERT_TRUE( page->GetWidget( "Disconnect" )->GetBoolProperty( "Enabled" ) && !page->GetWidget( "Connect" )->GetBoolProperty( "Enabled" ) );
        ASSERT_TRUE( fixture.Pane().IsUsingRemoteService() );
        // the code is in no widget of the page
        const CORE::CString allText = TextOf( page->GetWidget( "StateText" ) ) + TextOf( page->GetWidget( "ErrorText" ) ) + TextOf( page->GetWidget( "ServerInfo" ) ) + TextOf( page->GetWidget( "Endpoint" ) ) + TextOf( page->GetWidget( "PairingCode" ) ) + TextOf( page->GetWidget( "PairHint" ) );
        ASSERT_TRUE( allText.HasSubstr( code ) < 0 );

        // Apply keeps the address and the switch in the preferences
        ASSERT_TRUE( page->SetEndpointText( "tcp://127.0.0.1:" + CORE::ToString( world.hostSettings.sessionPort ) + "/" ) );
        ASSERT_TRUE( layer.GetForm()->Apply() );
        ASSERT_TRUE( world.GetEndpoint() == fixture.preferences.remoteEndpoint && fixture.preferences.remoteLanDiscovery );
        ASSERT_TRUE( world.GetEndpoint() == connection.GetEndpoint() );

        // Disconnect
        ASSERT_TRUE( page->RequestDisconnect() );
        fixture.Frames( 3 );
        ASSERT_TRUE( page->GetStateText().HasSubstr( "Not connected" ) >= 0 && !fixture.Pane().IsUsingRemoteService() );
        ASSERT_TRUE( page->GetWidget( "Connect" )->GetBoolProperty( "Enabled" ) && !page->GetWidget( "Disconnect" )->GetBoolProperty( "Enabled" ) );
        ASSERT_TRUE( page->RequestConnect() );
        ASSERT_TRUE( fixture.WaitFor( [&]() { return CWorkbenchStoreConnection::STATE_CONNECTED == connection.GetState(); } ) );
        layer.GetForm()->Cancel();

        // the page has no layout lint findings of the severity warning and above, at the sizes of the other pages
        const Int32 widths[] = { 0, 1000 };
        const Int32 heights[] = { 0, 700 };
        for ( UInt32 size=0; size<2; ++size )
        {
            fixture.context->SetLogicalResolution( widths[ size ], heights[ size ] );
            ASSERT_TRUE( layer.Open( "Remote store" ) );
            fixture.Frames( 3 );
            GUI::CLayoutLintOptions options;
            options.root = page->GetRootWidget();
            GUI::TLayoutLintFindingVector findings;
            GUI::CLayoutLint lint;
            lint.Run( *fixture.context, options, findings );
            const UInt32 problems = GUI::CLayoutLint::CountAtLeast( findings, GUI::LAYOUTLINTSEVERITY_WARNING );
            if ( 0 != problems )
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Remote store lint at " << widths[ size ] << "x" << heights[ size ] << ": " << GUI::CLayoutLint::Report( findings, 20 ) );
            ASSERT_TRUE( 0 == problems );
            if ( 0 == size )
                DumpPicture( fixture.Frames( 3 ), "remote_page_" + CORE::CString( GetTestRenderer() ) );
            layer.GetForm()->Cancel();
            fixture.context->SetLogicalResolution( 0, 0 );
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
TestRemoteStoreConsole( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "RemoteStore: the console commands remote, connect, pair and disconnect, and the command line options --store-endpoint and --store-pair-code" )
    try
    {
        // the command line
        {
            char program[] = "AssetWorkbench";
            char endpoint[] = "--store-endpoint";
            char endpointValue[] = "10.1.2.3:47400";
            char pairOption[] = "--store-pair-code";
            char pairValue[] = "ABCD-EFGH-JKMN-PQRS";
            char* argv[] = { program, endpoint, endpointValue, pairOption, pairValue };
            CWorkbenchApp app( 5, argv );
            ASSERT_TRUE( app.ParseCommandLine() );
            ASSERT_TRUE( "10.1.2.3:47400" == app.GetStoreEndpoint() && app.HasStorePairCode() );
            // the code does not stay in the argument list
            ASSERT_TRUE( CORE::CString( pairValue ).HasSubstr( "ABCD" ) < 0 );
            char missing[] = "--store-endpoint";
            char* wrong[] = { program, missing };
            CWorkbenchApp bad( 2, wrong );
            ASSERT_FALSE( bad.ParseCommandLine() );
        }

        if ( GetMainTestWindow().IsNULL() )
            GUCEF_TESTFW_SKIP( GetNoWindowReason().C_String() );
        WBREMOTETEST::CRemoteWorld world( "console" );
        CORE::CString error;
        ASSERT_TRUE( world.StartServer( error ) );
        CRemoteFixture fixture( "wbclient_console" );
        ASSERT_TRUE( fixture.ready && fixture.sourcesOpened );
        CWorkbenchSettingsLayer& layer = *fixture.layer;
        CWorkbenchConsoleCommands* commands = layer.GetCommands();
        CWorkbenchStoreConnection& connection = fixture.Connection();
        ASSERT_TRUE( GUCEF_NULL != commands );
        bool hasCommand[ 4 ] = { false, false, false, false };
        for ( UInt32 i=0; i<commands->GetCommandCount(); ++i )
        {
            const CORE::CString name = commands->GetCommandName( i );
            hasCommand[ 0 ] = hasCommand[ 0 ] || "connect" == name;
            hasCommand[ 1 ] = hasCommand[ 1 ] || "pair" == name;
            hasCommand[ 2 ] = hasCommand[ 2 ] || "disconnect" == name;
            hasCommand[ 3 ] = hasCommand[ 3 ] || "remote" == name;
        }
        ASSERT_TRUE( hasCommand[ 0 ] && hasCommand[ 1 ] && hasCommand[ 2 ] && hasCommand[ 3 ] );

        CORE::CString code;
        ASSERT_TRUE( world.OpenPairing( code, error ) );
        connection.SetSettings( world.MakeConnectionSettings() );

        ASSERT_TRUE( commands->Run( "remote" ) );
        ASSERT_TRUE( ConsoleText( layer ).HasSubstr( "State: disconnected" ) >= 0 && ConsoleText( layer ).HasSubstr( "not paired" ) >= 0 );
        // connect without pairing says what to do
        ASSERT_TRUE( commands->Run( "connect" ) );
        ASSERT_TRUE( ConsoleText( layer ).HasSubstr( "not paired" ) >= 0 && ConsoleText( layer ).HasSubstr( "Pair" ) >= 0 );
        ASSERT_TRUE( commands->Run( "pair" ) && ConsoleText( layer ).HasSubstr( "Usage: pair <code>" ) >= 0 );
        // a wrong code
        ASSERT_TRUE( commands->Run( CORE::CString( "pair " ) + WrongCode ) );
        ASSERT_TRUE( fixture.WaitFor( [&]() { return !connection.IsBusy() && CWorkbenchStoreConnection::STATE_PAIRING != connection.GetState(); } ) );
        ASSERT_TRUE( commands->Run( "remote" ) && ConsoleText( layer ).HasSubstr( "Problem: The pairing code was not accepted" ) >= 0 );
        // the right code
        ASSERT_TRUE( commands->Run( "pair " + code ) );
        ASSERT_TRUE( fixture.WaitFor( [&]() { return CWorkbenchStoreConnection::STATE_CONNECTED == connection.GetState(); } ) );
        ASSERT_TRUE( commands->Run( "remote" ) );
        ASSERT_TRUE( ConsoleText( layer ).HasSubstr( "State: connected" ) >= 0 && ConsoleText( layer ).HasSubstr( "Service: gucef.assetstore.test" ) >= 0 && ConsoleText( layer ).HasSubstr( "this computer is paired" ) >= 0 );
        ASSERT_TRUE( fixture.Pane().IsUsingRemoteService() );
        // disconnect, then connect to an address
        ASSERT_TRUE( commands->Run( "disconnect" ) && !fixture.Pane().IsUsingRemoteService() && ConsoleText( layer ).HasSubstr( "Disconnected" ) >= 0 );
        ASSERT_TRUE( commands->Run( "connect " + world.GetEndpoint() ) );
        ASSERT_TRUE( fixture.WaitFor( [&]() { return CWorkbenchStoreConnection::STATE_CONNECTED == connection.GetState(); } ) );
        ASSERT_TRUE( world.GetEndpoint() == connection.GetEndpoint() );
        ASSERT_TRUE( commands->Run( "disconnect" ) );
        // the help lists them
        ASSERT_TRUE( commands->Run( "help" ) && ConsoleText( layer ).HasSubstr( "connect [host:port]" ) >= 0 && ConsoleText( layer ).HasSubstr( "pair <code>" ) >= 0 );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}


/*-------------------------------------------------------------------------*/

namespace {

/** Paired and connected to a world, with the mounts of the remote files below a prefix of their own (the service is in this process too) */
bool
ConnectFixture( CRemoteFixture& fixture, WBREMOTETEST::CRemoteWorld& world )
{
    fixture.Pane().SetRemoteMountPrefix( "client" );
    fixture.Pane().SetRemoteCacheFolder( fixture.dir + "/remote_cache" );
    CORE::CString code;
    CORE::CString error;
    if ( !world.OpenPairing( code, error ) )
        return false;
    fixture.Connection().SetSettings( world.MakeConnectionSettings() );
    return fixture.PairWithCode( world, code ) && CWorkbenchStoreConnection::STATE_CONNECTED == fixture.Connection().GetState() && fixture.Pane().IsUsingRemoteService() &&
           GUCEF_NULL != fixture.Pane().GetRemoteFiles();
}

/** The keys of the rows of a search: the model and the picture of the sword pack */
bool
FindSwordKeys( CRemoteFixture& fixture, CORE::CString& modelKey, CORE::CString& textureKey )
{
    GUI::CAssetSearchForm* form = fixture.Pane().GetSearchForm();
    form->SetText( "sword" );
    form->Search();
    if ( !fixture.WaitFor( [&]() { return !form->IsSearching() && 2 == form->GetPage().total; } ) )
        return false;
    for ( UInt32 i=0; i<form->GetShownRowCount(); ++i )
    {
        if ( "sword_model" == form->GetShownRow( i ).title )
            modelKey = form->GetShownRow( i ).key;
        if ( "sword_d" == form->GetShownRow( i ).title )
            textureKey = form->GetShownRow( i ).key;
    }
    return !modelKey.IsNULLOrEmpty() && !textureKey.IsNULLOrEmpty();
}

/** Keeps the text that an event of a pane carries */
class CPathRecorder : public CORE::CObserver
{
    public:

    UInt32 count;
    CORE::CString last;

    CPathRecorder( CORE::CNotifier* notifier, const CORE::CEvent& eventId ) : CORE::CObserver(), count( 0 ), last()
    {
        SubscribeTo( notifier, eventId );
    }

    virtual ~CPathRecorder() { UnsubscribeAllFromObserver(); }

    virtual void OnNotify( CORE::CNotifier* notifier, const CORE::CEvent& id, CORE::CICloneable* eventData ) GUCEF_VIRTUAL_OVERRIDE
    {
        const CORE::TCloneableString* text = static_cast< const CORE::TCloneableString* >( eventData );
        if ( GUCEF_NULL != text )
        {
            ++count;
            last = text->GetData();
        }
    }
};

/** A file through the VFS in pieces; false when it cannot be opened */
bool
ReadThroughVfs( const CORE::CString& path, CORE::CDynamicBuffer& content )
{
    VFS::TBasicVfsResourcePtr file = VFS::CVfsGlobal::Instance()->GetVfs().GetFile( path, "rb", false );
    if ( file.IsNULL() )
        return false;
    CORE::IOAccessPtr access = file->GetAccess();
    if ( access.IsNULL() )
        return false;
    content.Clear( false );
    UInt8 piece[ 16384 ];
    while ( true )
    {
        const UInt32 read = access->Read( piece, 1, sizeof( piece ) );
        if ( 0 == read )
            break;
        content.Append( piece, read );
    }
    return content.GetDataSize() > 0;
}

/** A quad of four vertices exported as a cooked mesh file of the disk (as the cooked output tests do) */
bool
WriteCookedQuadFile( CWorkbenchStage& stage, const CORE::CString& diskPath )
{
    Ogre::SceneManager* sceneManager = stage.GetSceneManager();
    Ogre::ManualObject* quad = sceneManager->createManualObject( "RemoteCookedQuadMaker" );
    quad->begin( CWorkbenchMaterials::VertexColoursMaterialName.STL_String(), Ogre::RenderOperation::OT_TRIANGLE_LIST );
    const float xs[ 4 ] = { -1.0f, 1.0f, 1.0f, -1.0f };
    const float ys[ 4 ] = { -1.0f, -1.0f, 1.0f, 1.0f };
    for ( UInt32 i=0; i<4; ++i )
    {
        quad->position( xs[ i ], ys[ i ], 0.0f );
        quad->colour( 0.9f, 0.2f, 0.1f, 1.0f );
    }
    quad->quad( 0, 1, 2, 3 );
    quad->end();
    Ogre::MeshPtr mesh = quad->convertToMesh( "RemoteCookedQuadForTheFile", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME );
    sceneManager->destroyManualObject( quad );
    CORE::CreatePathDirectories( diskPath );
    bool written = false;
    try
    {
        FILE* handle = fopen( diskPath.C_String(), "wb" );
        if ( GUCEF_NULL != handle )
        {
            Ogre::DataStreamPtr stream( OGRE_NEW Ogre::FileHandleDataStream( diskPath.STL_String(), handle, Ogre::DataStream::WRITE ) );
            Ogre::MeshSerializer serializer;
            serializer.exportMesh( mesh.get(), stream );
            stream->close();
        }
        written = CORE::FileExists( diskPath );
    }
    catch ( const Ogre::Exception& e )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "TestRemoteFiles: The cooked mesh cannot be written: " << e.getFullDescription() );
    }
    Ogre::MeshManager::getSingleton().remove( mesh );
    return written;
}

}

/*-------------------------------------------------------------------------*/

static void
TestRemoteFilesOpen( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "RemoteFiles: a result of the remote store opens in the 3D view and the texture view from the files of the service: the Fetching state, the block cache on this computer, no read of the interface thread waits, the files are not on the disk of this computer" )
    try
    {
        if ( GetMainTestWindow().IsNULL() )
            GUCEF_TESTFW_SKIP( GetNoWindowReason().C_String() );
        WBREMOTETEST::CRemoteWorld world( "wbfiles" );
        CORE::CString error;
        ASSERT_TRUE( world.StartServer( error ) && world.AddPack( "Sword Pack" ) );
        CRemoteFixture fixture( "wbfiles_wb" );
        ASSERT_TRUE( fixture.ready && fixture.sourcesOpened );
        CAssetSourcesPane& pane = fixture.Pane();
        ASSERT_TRUE( ConnectFixture( fixture, world ) );
        ASSETSTORE::CRemoteAssetFileSystem* files = pane.GetRemoteFiles();
        ASSERT_TRUE( GUCEF_NULL != files && files->IsMounted( "avstore/remote_wbfiles" ) && GUCEF_NULL != pane.GetFetchJobs() );
        // the mounter adopted the location: the paths of the catalogue open (here below the prefix of the test)
        CAssetMounter::SMount mount;
        CORE::CString relative;
        ASSERT_TRUE( fixture.mounter.FindMount( "client/avstore/remote_wbfiles/original/x", mount, relative ) && mount.remote );

        CORE::CString modelKey, textureKey;
        ASSERT_TRUE( FindSwordKeys( fixture, modelKey, textureKey ) );
        GUI::CAssetSearchForm* form = pane.GetSearchForm();
        ASSERT_TRUE( form->SelectResult( modelKey ) );

        // opening: the page says that it fetches, then the views open the file of the cache
        CPathRecorder opened( &pane, CAssetSourcesPane::AssetOpenRequestedEvent );
        ASSERT_TRUE( form->OpenResult( modelKey ) );
        ASSERT_TRUE( pane.IsRemoteOpenBusy() && pane.GetRemoteOpenText().HasSubstr( "Fetching sword_model" ) == 0 );
        ASSERT_TRUE( pane.GetStoreStatusText().HasSubstr( "Fetching sword_model" ) == 0 );
        ASSERT_TRUE( fixture.WaitFor( [&]() { return 1 == opened.count; } ) );
        ASSERT_TRUE( opened.last.HasSubstr( "client/avstore/remote_wbfiles/original/" ) == 0 && opened.last.HasSubstr( "sword_model.gltf" ) > 0 && !pane.IsRemoteOpenBusy() && pane.GetRemoteOpenText().IsNULLOrEmpty() );
        ASSERT_TRUE( fixture.WaitFor( [&]() { return ASSETKIND_MODEL == fixture.shell->GetShownKind(); } ) );
        ASSERT_TRUE( CAssetMounter::NormalizePath( opened.last ) == fixture.shell->GetShownPath() );

        // what the cache holds: the file set of the asset; the disk of this computer has none of the files of the store
        ASSETSTORE::CAssetBlockCache::SStatistics statistics;
        files->GetCache().GetStatistics( statistics );
        ASSERT_TRUE( statistics.usedBytes > 0 && statistics.entries >= 1 );
        ASSERT_TRUE( !CORE::DirExists( fixture.dir + "/store/original" ) );
        ASSETSTORE::CRemoteAssetFileSystem::SStatistics fileStatistics;
        files->GetStatistics( fileStatistics );
        ASSERT_TRUE( fileStatistics.bytesFetched > 0 && fileStatistics.fetchCalls >= 1 );

        // a picture of the pack opens in the texture view
        ASSERT_TRUE( form->SelectResult( textureKey ) && form->OpenResult( textureKey ) );
        ASSERT_TRUE( fixture.WaitFor( [&]() { return ASSETKIND_IMAGE == fixture.shell->GetShownKind(); } ) );
        ASSERT_TRUE( 2 == pane.GetRemoteOpenCount() );
        IMAGE::TPixelMapPtr pixels = fixture.Frames( 3 );
        DumpPicture( pixels, "remote_files_open_" + CORE::CString( GetTestRenderer() ) );

        // a pack: its folder is listed in the background and then shown in the content browser
        CPathRecorder folders( &pane, CAssetSourcesPane::FolderOpenRequestedEvent );
        CORE::CString packKey;
        for ( UInt32 i=0; i<form->GetShownRowCount(); ++i )
            if ( 0 == form->GetShownRow( i ).key.HasSubstr( "pack:" ) )
                packKey = form->GetShownRow( i ).key;
        form->SetText( "sword" );
        if ( !packKey.IsNULLOrEmpty() )
        {
            ASSERT_TRUE( form->OpenResult( packKey ) );
            ASSERT_TRUE( fixture.WaitFor( [&]() { return 1 == folders.count; } ) );
            ASSERT_TRUE( folders.last.HasSubstr( "client/avstore/remote_wbfiles/original/" ) == 0 );
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
TestRemoteFilesReferenced( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "RemoteFiles: the picture that a model names is fetched with it although the catalogue does not relate it to the model, and the resource group of the pack finds it" )
    try
    {
        if ( GetMainTestWindow().IsNULL() )
            GUCEF_TESTFW_SKIP( GetNoWindowReason().C_String() );
        WBREMOTETEST::CRemoteWorld world( "wbreferenced" );
        CORE::CString error;
        ASSERT_TRUE( world.StartServer( error ) && world.AddPack( "Sword Pack" ) );
        CRemoteFixture fixture( "wbreferenced_wb" );
        ASSERT_TRUE( fixture.ready && fixture.sourcesOpened );
        CAssetSourcesPane& pane = fixture.Pane();

        // a picture that is in the folders of the pack on the server and that the model names (the model of the test names "checker.png"), but that no asset of the catalogue holds
        ASSETSTORE::CAssetSearchQuery query;
        query.text = "sword";
        ASSETSTORE::CAssetSearchResultPage page;
        ASSERT_TRUE( ASSETSTORE::ASSETSTORE_STATUS_OK == world.serverService->Search( query, page ) );
        CORE::CString sourceVfs;
        for ( size_t i=0; i<page.hits.size(); ++i )
            if ( "sword_model" == page.hits[ i ].name )
                sourceVfs = page.hits[ i ].vfsPath;
        ASSERT_TRUE( !sourceVfs.IsNULLOrEmpty() );
        const Int32 at = sourceVfs.HasSubstr( "/original/", false );
        ASSERT_TRUE( at > 0 );
        const CORE::CString packFolder = sourceVfs.CutChars( (UInt32) at + 10, true ).SubstrToChar( '/', true );
        ASSERT_TRUE( WriteTestPng( world.directory + "/store/original/" + packFolder + "/Textures/checker.png", 8, 8 ) );
        const CORE::CString pictureService = "avstore/remote_wbreferenced/original/" + packFolder + "/Textures/checker.png";

        ASSERT_TRUE( ConnectFixture( fixture, world ) );
        CORE::CString modelKey, textureKey;
        ASSERT_TRUE( FindSwordKeys( fixture, modelKey, textureKey ) );
        GUI::CAssetSearchForm* form = pane.GetSearchForm();
        ASSERT_TRUE( form->SelectResult( modelKey ) );
        ASSETSTORE::CRemoteAssetFileSystem* files = pane.GetRemoteFiles();
        ASSERT_TRUE( GUCEF_NULL != files );
        CPathRecorder opened( &pane, CAssetSourcesPane::AssetOpenRequestedEvent );
        ASSERT_TRUE( form->OpenResult( modelKey ) );
        ASSERT_TRUE( fixture.WaitFor( [&]() { return 1 == opened.count; } ) );

        // the picture is in the cache of the files, and the group of the pack has it as a resource
        ASSETSTORE::CRemoteAssetFileSystem::SFileInfo info;
        ASSERT_TRUE( files->GetFileInfo( pictureService, info ) && info.exists && info.size > 0 );
        ASSERT_TRUE( files->IsFileCached( pictureService, info.size ) );
        CORE::CString relative;
        const CORE::CString group = fixture.mounter.GetResourceGroup( "client/avstore/remote_wbreferenced/original/" + packFolder + "/sword_model.gltf", relative );
        ASSERT_TRUE( !group.IsNULLOrEmpty() && Ogre::ResourceGroupManager::getSingleton().resourceExists( group.STL_String(), "checker.png" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
TestRemoteFilesCooked( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "RemoteFiles: the cooked Ogre output of a model of the remote store (the layout of version 2: original, intermediate and gucef-ogre3d per pack) is found through the VFS of the remote files, fetched with the model and shown instead of the quick look" )
    try
    {
        if ( GetMainTestWindow().IsNULL() )
            GUCEF_TESTFW_SKIP( GetNoWindowReason().C_String() );
        WBREMOTETEST::CRemoteWorld world( "wbcooked" );
        CORE::CString error;
        ASSERT_TRUE( world.StartServer( error ) && world.AddPack( "Sword Pack" ) );
        CRemoteFixture fixture( "wbcooked_wb" );
        ASSERT_TRUE( fixture.ready && fixture.sourcesOpened );
        CAssetSourcesPane& pane = fixture.Pane();

        // the cooked output on the server: the cooked mesh in the cooked area of the pack and the cache entry of the converter in its intermediate area
        CORE::CString modelId;
        ASSETSTORE::CAssetSearchQuery query;
        query.text = "sword";
        ASSETSTORE::CAssetSearchResultPage page;
        ASSERT_TRUE( ASSETSTORE::ASSETSTORE_STATUS_OK == world.serverService->Search( query, page ) );
        CORE::CString sourceVfs;
        for ( size_t i=0; i<page.hits.size(); ++i )
            if ( "sword_model" == page.hits[ i ].name )
                sourceVfs = page.hits[ i ].vfsPath;
        ASSERT_TRUE( !sourceVfs.IsNULLOrEmpty() );
        const CORE::CString root = "avstore/remote_wbcooked";
        const Int32 at = sourceVfs.HasSubstr( "/original/", false );
        ASSERT_TRUE( at > 0 );
        const CORE::CString packFolder = sourceVfs.CutChars( (UInt32) at + 10, true ).SubstrToChar( '/', true );
        const CORE::CString cookedVfs = root + "/gucef-ogre3d/" + packFolder + "/desktop/" + packFolder + "/sword_model.mesh";
        ASSERT_TRUE( WriteCookedQuadFile( fixture.shell->GetStage(), world.directory + "/store/gucef-ogre3d/" + packFolder + "/desktop/" + packFolder + "/sword_model.mesh" ) );
        ASSETCONVERTER::CAssetCacheEntry entry;
        entry.assetPath = "sword_model.gltf";
        entry.sourceHash = ASSETCONVERTER::CContentHash::HashFile( sourceVfs );
        entry.settingsHash = "settings";
        entry.plugins[ "converter:AssetConverterLib" ] = ASSETCONVERTER::CAssetConverterGlobal::GetConverterVersion();
        ASSETCONVERTER::CConversionReportFile intermediate;
        intermediate.path = root + "/intermediate/" + packFolder + "/sword_model.glb";
        entry.files.push_back( intermediate );
        ASSETCONVERTER::CConversionReportFile cooked;
        cooked.path = cookedVfs;
        cooked.profile = "desktop";
        entry.files.push_back( cooked );
        entry.ComputeKey();
        ASSERT_TRUE( entry.Save( ASSETCONVERTER::CAssetCacheEntry::GetEntryPath( root + "/intermediate/" + packFolder, "sword_model.gltf" ) ) );

        // connected: opening the result fetches the model and its cooked files (found through the VFS of the remote files) and the views show the cooked output
        ASSERT_TRUE( ConnectFixture( fixture, world ) );
        CORE::CString modelKey, textureKey;
        ASSERT_TRUE( FindSwordKeys( fixture, modelKey, textureKey ) );
        GUI::CAssetSearchForm* form = pane.GetSearchForm();
        ASSERT_TRUE( form->SelectResult( modelKey ) );
        CPathRecorder opened( &pane, CAssetSourcesPane::AssetOpenRequestedEvent );
        ASSERT_TRUE( form->OpenResult( modelKey ) );
        ASSERT_TRUE( fixture.WaitFor( [&]() { return 1 == opened.count; } ) );
        ASSERT_TRUE( fixture.WaitFor( [&]() { return ASSETKIND_MODEL == fixture.shell->GetShownKind() && !fixture.shell->IsOpening(); } ) );
        ASSERT_TRUE( CORE::CString( "cooked Ogre v1 output (gucef-ogre3d)" ) == fixture.shell->GetShownSourceDescription() );
        ASSERT_TRUE( 4 == fixture.shell->GetModelView().GetStatistics().vertexCount );
        IMAGE::TPixelMapPtr pixels = fixture.Frames( 3 );
        DumpPicture( pixels, "remote_files_cooked_" + CORE::CString( GetTestRenderer() ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/

static void
TestRemoteFilesDownload( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "RemoteFiles: Download to folder and Keep offline: a job in the jobs list and the transfer queue with progress, the files digest checked in the folder, the console command download, and what is kept offline opens from the cache while the service is gone" )
    try
    {
        if ( GetMainTestWindow().IsNULL() )
            GUCEF_TESTFW_SKIP( GetNoWindowReason().C_String() );
        WBREMOTETEST::CRemoteWorld world( "wbdownload" );
        CORE::CString error;
        ASSERT_TRUE( world.StartServer( error ) && world.AddPack( "Sword Pack" ) );
        CRemoteFixture fixture( "wbdownload_wb" );
        ASSERT_TRUE( fixture.ready && fixture.sourcesOpened );
        CAssetSourcesPane& pane = fixture.Pane();
        ASSERT_TRUE( ConnectFixture( fixture, world ) );
        CORE::CString modelKey, textureKey;
        ASSERT_TRUE( FindSwordKeys( fixture, modelKey, textureKey ) );
        GUI::CAssetSearchForm* form = pane.GetSearchForm();
        ASSERT_TRUE( form->SelectResult( modelKey ) && !pane.GetSelectedPackId().IsNULLOrEmpty() );

        // the button of the page and the box above it
        GUI::CWidget* download = pane.GetControl( "Download" );
        GUI::CWidget* keep = pane.GetControl( "KeepOffline" );
        ASSERT_TRUE( GUCEF_NULL != download && download->GetBoolProperty( "Visible" ) && GUCEF_NULL != keep && keep->GetBoolProperty( "Visible" ) );
        GUI::CWidget* addDisk = pane.GetControl( "AddDisk" );
        ASSERT_TRUE( GUCEF_NULL != addDisk && !addDisk->GetBoolProperty( "Visible" ) );

        // the download of the selected result (an asset with its file set) through the page
        const CORE::CString target = fixture.dir + "/downloaded";
        CORE::DeleteDir( target, true );
        CORE::CString message;
        ASSERT_FALSE( pane.DownloadSelected( CORE::CString::Empty, message ) || message.IsNULLOrEmpty() );
        ASSERT_TRUE( pane.DownloadSelected( target, message ) );
        ASSERT_TRUE( 1 == pane.GetFetchJobs()->GetActiveCount() );
        ASSERT_TRUE( fixture.WaitFor( [&]() { return 0 == pane.GetFetchJobs()->GetActiveCount(); } ) );
        ASSERT_TRUE( ASSETSTORE::ASSETSTORE_STATUS_OK == pane.GetFetchJobs()->GetLastEnded().status && "download" == pane.GetFetchJobs()->GetLastEnded().kind );
        // it is a job of the workbench: in the jobs list, finished, with its words
        ASSETSTORE::TAssetJobEntryVector jobs;
        fixture.shell->GetJobRegistry().GetJobs( jobs );
        bool listed = false;
        for ( size_t i=0; i<jobs.size(); ++i )
            if ( "download" == jobs[ i ].kind && ASSETSTORE::ASSETJOB_STATE_DONE == jobs[ i ].state )
                listed = true;
        ASSERT_TRUE( listed || pane.GetFetchJobs()->GetLastEnded().bytes < 1024 * 1024 );
        CORE::CDynamicBuffer downloaded;
        ASSERT_TRUE( CORE::FileExists( target + "/sword_model.gltf" ) && downloaded.LoadContentFromFile( target + "/sword_model.gltf" ) && downloaded.GetDataSize() > 100 );

        // the whole pack through the console: the same as the button
        const CORE::CString packId = pane.GetSelectedPackId();
        const CORE::CString packTarget = fixture.dir + "/downloaded_pack";
        ASSETSTORE::CAssetPackRecord pack0;
        ASSERT_TRUE( ASSETSTORE::ASSETSTORE_STATUS_OK == pane.GetService()->GetPack( packId, pack0 ) );
        CORE::DeleteDir( packTarget, true );
        ASSERT_TRUE( pane.DownloadPack( packId, packTarget, message ) );
        ASSERT_TRUE( fixture.WaitFor( [&]() { return 0 == pane.GetFetchJobs()->GetActiveCount(); } ) );
        ASSERT_TRUE( ASSETSTORE::ASSETSTORE_STATUS_OK == pane.GetFetchJobs()->GetLastEnded().status );
        ASSERT_TRUE( CORE::DirExists( packTarget ) && CORE::FileExists( packTarget + "/" + ASSETSTORE::CPackStorageLayout( pack0.rootPath ).GetPackFolder() + "/sword_model.gltf" ) );

        // keep offline of the selected result (an asset: its file set with its companion files), then of its whole pack, then the service goes:
        // the files of the pack are read from the cache, anything else says why not
        ASSERT_TRUE( pane.KeepSelectedOffline( true, message ) );
        ASSERT_TRUE( fixture.WaitFor( [&]() { return 0 == pane.GetFetchJobs()->GetActiveCount(); } ) );
        ASSERT_TRUE( ASSETSTORE::ASSETSTORE_STATUS_OK == pane.GetFetchJobs()->GetLastEnded().status && "offline" == pane.GetFetchJobs()->GetLastEnded().kind );
        ASSETSTORE::CAssetBlockCache::SStatistics statistics;
        pane.GetRemoteFiles()->GetCache().GetStatistics( statistics );
        ASSERT_TRUE( statistics.pinnedEntries >= 1 && statistics.pinnedBytes > 0 );
        ASSERT_TRUE( pane.KeepPackOffline( packId, true, message ) );
        ASSERT_TRUE( fixture.WaitFor( [&]() { return 0 == pane.GetFetchJobs()->GetActiveCount(); } ) );
        ASSERT_TRUE( ASSETSTORE::ASSETSTORE_STATUS_OK == pane.GetFetchJobs()->GetLastEnded().status && "offline" == pane.GetFetchJobs()->GetLastEnded().kind );
        pane.GetRemoteFiles()->GetCache().GetStatistics( statistics );
        ASSERT_TRUE( statistics.pinnedEntries >= 2 && statistics.pinnedBytes > 0 );
        ASSETSTORE::CAssetPackRecord pack;
        ASSERT_TRUE( ASSETSTORE::ASSETSTORE_STATUS_OK == pane.GetService()->GetPack( packId, pack ) );
        const CORE::CString original = "client/avstore/remote_wbdownload/original/" + ASSETSTORE::CPackStorageLayout( pack.rootPath ).GetPackFolder() + "/sword_model.gltf";
        world.StopServer();
        fixture.Frames( 10 );
        CORE::CDynamicBuffer offline;
        ASSERT_TRUE( ReadThroughVfs( original, offline ) && offline.GetDataSize() == downloaded.GetDataSize() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
TestRemoteFilesCompanions( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "RemoteFiles: a single asset is downloaded and kept offline with its companion files from the relations of the catalogue (its texture); for an asset without relations the files its model names are found by name" )
    try
    {
        if ( GetMainTestWindow().IsNULL() )
            GUCEF_TESTFW_SKIP( GetNoWindowReason().C_String() );
        WBREMOTETEST::CRemoteWorld world( "wbcompanion" );
        CORE::CString error;
        ASSERT_TRUE( world.StartServer( error ) && world.AddPack( "Sword Pack" ) );
        ASSETSTORE::CAssetSearchQuery query;
        query.text = "sword";
        ASSETSTORE::CAssetSearchResultPage page;
        ASSERT_TRUE( ASSETSTORE::ASSETSTORE_STATUS_OK == world.serverService->Search( query, page ) );
        CORE::CString modelId, packId, sourceVfs;
        for ( size_t i=0; i<page.hits.size(); ++i )
            if ( "sword_model" == page.hits[ i ].name )
            {
                modelId = page.hits[ i ].assetId;
                packId = page.hits[ i ].packId;
                sourceVfs = page.hits[ i ].vfsPath;
            }
        ASSERT_TRUE( !modelId.IsNULLOrEmpty() && !packId.IsNULLOrEmpty() );
        const Int32 at = sourceVfs.HasSubstr( "/original/", false );
        ASSERT_TRUE( at > 0 );
        const CORE::CString packFolder = sourceVfs.CutChars( (UInt32) at + 10, true ).SubstrToChar( '/', true );
        // a picture that the model names (checker.png) but that no relation leads to: the fallback finds it, the relations do not need it
        ASSERT_TRUE( WriteTestPng( world.directory + "/store/original/" + packFolder + "/Textures/checker.png", 8, 8 ) );

        CRemoteFixture fixture( "wbcompanion_wb" );
        ASSERT_TRUE( fixture.ready && fixture.sourcesOpened );
        CAssetSourcesPane& pane = fixture.Pane();
        ASSERT_TRUE( ConnectFixture( fixture, world ) );

        // without relations: the file set has the model only, the download finds the picture the model names
        ASSETSTORE::CAssetFileSet files;
        ASSERT_TRUE( ASSETSTORE::ASSETSTORE_STATUS_OK == pane.GetService()->GetAssetFileSet( modelId, files ) && 0 == files.relationCount && !files.companionsTruncated );
        CORE::CString message;
        const CORE::CString plain = fixture.dir + "/companion_plain";
        CORE::DeleteDir( plain, true );
        ASSERT_TRUE( pane.DownloadAsset( modelId, plain, message ) );
        ASSERT_TRUE( fixture.WaitFor( [&]() { return 0 == pane.GetFetchJobs()->GetActiveCount(); } ) );
        ASSERT_TRUE( ASSETSTORE::ASSETSTORE_STATUS_OK == pane.GetFetchJobs()->GetLastEnded().status );
        ASSERT_TRUE( CORE::FileExists( plain + "/sword_model.gltf" ) && CORE::FileExists( plain + "/Textures/checker.png" ) );
        ASSERT_FALSE( CORE::FileExists( plain + "/Textures/sword_d.png" ) );

        // the relation of the model to its texture, as the loader of a pack index writes it into the catalogue of the server
        ASSETSTORE::CLocalAssetStoreService* local = static_cast< ASSETSTORE::CLocalAssetStoreService* >( world.serverService.GetPointerAlways() );
        ASSERT_TRUE( GUCEF_NULL != local && GUCEF_NULL != local->GetPrimaryStore() );
        ASSETSTORE::CAssetCatalogue::TRow relation;
        relation[ "from_asset" ] = CORE::CVariant( modelId );
        relation[ "kind" ] = CORE::CVariant( CORE::CString( "usesTexture" ) );
        relation[ "to_asset" ] = CORE::CVariant( ASSETSTORE::CAssetPackIndexLoader::MakeResourceId( packId, "Textures/sword_d.png" ) );
        relation[ "pack_id" ] = CORE::CVariant( packId );
        ASSETSTORE::CAssetCatalogue::TRowVector rows;
        rows.push_back( relation );
        ASSERT_TRUE( ASSETSTORE::ASSETSTORE_STATUS_OK == local->GetPrimaryStore()->GetCatalogue().Write( "relations", rows ) );

        // with the relation: the file set of the service holds the texture, the download takes it and the name search is not asked
        ASSERT_TRUE( ASSETSTORE::ASSETSTORE_STATUS_OK == pane.GetService()->GetAssetFileSet( modelId, files ) && 1 == files.relationCount );
        bool listed = false;
        for ( size_t f=0; f<files.files.size(); ++f )
            listed = listed || ( "referenced" == files.files[ f ].role && files.files[ f ].path.EndsWith( "/Textures/sword_d.png" ) && !files.files[ f ].digest.IsNULLOrEmpty() );
        ASSERT_TRUE( listed );
        const CORE::CString related = fixture.dir + "/companion_related";
        CORE::DeleteDir( related, true );
        ASSERT_TRUE( pane.DownloadAsset( modelId, related, message ) );
        ASSERT_TRUE( fixture.WaitFor( [&]() { return 0 == pane.GetFetchJobs()->GetActiveCount(); } ) );
        ASSERT_TRUE( ASSETSTORE::ASSETSTORE_STATUS_OK == pane.GetFetchJobs()->GetLastEnded().status );
        ASSERT_TRUE( CORE::FileExists( related + "/sword_model.gltf" ) && CORE::FileExists( related + "/Textures/sword_d.png" ) );
        ASSERT_FALSE( CORE::FileExists( related + "/Textures/checker.png" ) );
        CORE::CDynamicBuffer texture;
        ASSERT_TRUE( texture.LoadContentFromFile( related + "/Textures/sword_d.png" ) && texture.GetDataSize() > 0 );

        // keep offline of the selected asset: its file set with the texture is pinned in the cache
        CORE::CString modelKey, textureKey;
        ASSERT_TRUE( FindSwordKeys( fixture, modelKey, textureKey ) );
        ASSERT_TRUE( pane.GetSearchForm()->SelectResult( modelKey ) );
        ASSERT_TRUE( pane.KeepSelectedOffline( true, message ) );
        ASSERT_TRUE( fixture.WaitFor( [&]() { return 0 == pane.GetFetchJobs()->GetActiveCount(); } ) );
        ASSERT_TRUE( ASSETSTORE::ASSETSTORE_STATUS_OK == pane.GetFetchJobs()->GetLastEnded().status && "offline" == pane.GetFetchJobs()->GetLastEnded().kind );
        const CORE::CString textureService = "avstore/remote_wbcompanion/original/" + packFolder + "/Textures/sword_d.png";
        ASSETSTORE::CRemoteAssetFileSystem::SFileInfo info;
        ASSERT_TRUE( pane.GetRemoteFiles()->GetFileInfo( textureService, info ) && info.exists && pane.GetRemoteFiles()->IsFileCached( textureService, info.size ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
TestRemoteFilesThumbnails( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "RemoteFiles: the pictures of the results come from the service for the visible rows only, in one batch, and are kept on this computer; a render of the workbench is put on the service" )
    try
    {
        if ( GetMainTestWindow().IsNULL() )
            GUCEF_TESTFW_SKIP( GetNoWindowReason().C_String() );
        WBREMOTETEST::CRemoteWorld world( "wbthumbs" );
        CORE::CString error;
        ASSERT_TRUE( world.StartServer( error ) && world.AddPack( "Sword Pack" ) );
        CRemoteFixture fixture( "wbthumbs_wb" );
        ASSERT_TRUE( fixture.ready && fixture.sourcesOpened );
        CAssetSourcesPane& pane = fixture.Pane();
        ASSERT_TRUE( ConnectFixture( fixture, world ) );
        GUI::CAssetSearchForm* form = pane.GetSearchForm();
        ASSERT_TRUE( GUCEF_NULL != pane.GetRemoteThumbnails() );

        // the service stores the picture of one of the two assets (a picture of the texture)
        form->SetText( "sword" );
        form->Search();
        ASSERT_TRUE( fixture.WaitFor( [&]() { return !form->IsSearching() && 2 == form->GetPage().total; } ) );
        ASSETSTORE::CAssetSearchQuery query;
        query.text = "sword";
        ASSETSTORE::CAssetSearchResultPage page;
        ASSERT_TRUE( ASSETSTORE::ASSETSTORE_STATUS_OK == world.serverService->Search( query, page ) && 2 == page.hits.size() );
        CORE::CString textureId;
        for ( size_t i=0; i<page.hits.size(); ++i )
            if ( "sword_d" == page.hits[ i ].name )
                textureId = page.hits[ i ].assetId;
        ASSERT_TRUE( !textureId.IsNULLOrEmpty() );
        CORE::CDynamicBuffer png;
        {
            CORE::CString text;
            const CORE::CString file = fixture.dir + "/picture.png";
            WriteTestPng( file, 32, 32 );
            ASSERT_TRUE( png.LoadContentFromFile( file ) );
        }
        ASSETSTORE::CAssetThumbnail thumbnail;
        thumbnail.assetId = textureId;
        thumbnail.kind = "thumbnail";
        thumbnail.size = 128;
        thumbnail.key.size = 128;
        thumbnail.key.rendererId = "test";
        thumbnail.encoded = png;
        ASSETSTORE::CAssetServiceResult put;
        ASSERT_TRUE( ASSETSTORE::ASSETSTORE_STATUS_OK == world.serverService->PutThumbnail( "wb-put-1", thumbnail, put ) );

        // the result list in the thumbnail view asks for the rows that are shown: one call for the two, the texture has a picture, the model none
        CAssetCapture capture( AsOgreWindow( GetMainTestWindow() ) );
        CThumbnailProvider ownProvider( fixture.mounter, capture );
        CThumbnailProvider* provider = &ownProvider;
        provider->SetCacheFolder( fixture.dir + "/thumbs" );
        provider->SetRemoteSource( pane.GetRemoteThumbnails() );
        form->SetThumbnailProvider( provider );
        form->SetThumbnailSize( 128 );
        form->SetThumbnailView( true );
        CRemoteThumbnailSource* source = pane.GetRemoteThumbnails();
        const bool settled = fixture.WaitFor( [&]() { provider->Update( 8 ); return 1 == source->GetFetchedCount() && 1 == source->GetMissingCount(); } );
        if ( !settled )
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "RemoteThumbnails: requested " << source->GetRequestedCount() << ", fetched " << source->GetFetchedCount() << ", missing " << source->GetMissingCount()
                             << ", batches " << source->GetBatchCount() << ", pending " << source->GetPendingCount() << ", provider queued " << provider->GetQueuedCount() << ", rows " << form->GetShownRowCount()
                             << ", thumbnail view " << ( form->IsThumbnailView() ? 1 : 0 ) );
        ASSERT_TRUE( settled );
        ASSERT_TRUE( source->GetBatchCount() >= 1 && source->GetRequestedCount() >= 2 && source->GetRequestedCount() <= form->GetShownRowCount() );
        ASSERT_TRUE( CORE::DirExists( source->GetCacheFolder() ) );
        ASSERT_TRUE( provider->GetRemoteHitCount() >= 1 );
        // what was fetched is not asked for again
        const UInt32 batches = source->GetBatchCount();
        form->SetThumbnailView( false );
        form->SetThumbnailView( true );
        for ( UInt32 i=0; i<20; ++i )
        {
            provider->Update( 8 );
            fixture.Frames( 1 );
        }
        ASSERT_TRUE( source->GetBatchCount() <= batches + 1 && 1 == source->GetFetchedCount() );
        IMAGE::TPixelMapPtr pixels = fixture.Frames( 3 );
        DumpPicture( pixels, "remote_files_thumbnails_" + CORE::CString( GetTestRenderer() ) );
        // this computer is an owner device: it puts pictures; as another role it would not call the service at all
        ASSERT_TRUE( source->GetMayUpload() );
        source->NoteAsset( "wbthumbs/upload.png", textureId );
        const UInt32 uploads = source->GetUploadCount();
        source->SetMayUpload( false );
        ASSERT_TRUE( !source->Upload( "wbthumbs/upload.png", 32, png ) && uploads == source->GetUploadCount() );
        source->SetMayUpload( true );
        ASSERT_TRUE( source->Upload( "wbthumbs/upload.png", 32, png ) && uploads + 1 == source->GetUploadCount() );
        form->SetThumbnailProvider( GUCEF_NULL );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------//
//                                                                         //
//      FUNCTIONS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformAssetWorkbenchRemoteStoreTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "AssetWorkbenchLib.RemoteStore" );

    TestRemoteStorePreferences();
    TestRemoteStoreDiscovery();
    TestRemoteStoreConnection();
    TestRemoteStoreUnreachable();
    TestRemoteStoreWorkbench();
    TestRemoteStorePage();
    TestRemoteStoreConsole();
    TestRemoteFilesOpen();
    TestRemoteFilesCooked();
    TestRemoteFilesReferenced();
    TestRemoteFilesDownload();
    TestRemoteFilesCompanions();
    TestRemoteFilesThumbnails();
}

void
PerformAssetWorkbenchRemoteThumbnailTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "AssetWorkbenchLib.RemoteStore" );

    TestRemoteFilesThumbnails();
}

/*-------------------------------------------------------------------------*/
