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

#ifndef TESTWORKBENCHREMOTEWORLD_H
#define TESTWORKBENCHREMOTEWORLD_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_DRNUTILS_CREMOTESERVICEHOST_H
#include "gucefDRNUTILS_CRemoteServiceHost.h"
#define GUCEF_DRNUTILS_CREMOTESERVICEHOST_H
#endif /* GUCEF_DRNUTILS_CREMOTESERVICEHOST_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETSTOREGLOBAL_H
#include "gucefASSETSTORE_CAssetStoreGlobal.h"
#define GUCEF_ASSETSTORE_CASSETSTOREGLOBAL_H
#endif /* GUCEF_ASSETSTORE_CASSETSTOREGLOBAL_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETSTORESAFENAMES_H
#include "gucefASSETSTORE_CAssetStoreSafeNames.h"
#define GUCEF_ASSETSTORE_CASSETSTORESAFENAMES_H
#endif /* GUCEF_ASSETSTORE_CASSETSTORESAFENAMES_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETSTORESERVICEHOST_H
#include "gucefASSETSTORE_CAssetStoreServiceHost.h"
#define GUCEF_ASSETSTORE_CASSETSTORESERVICEHOST_H
#endif /* GUCEF_ASSETSTORE_CASSETSTORESERVICEHOST_H ? */

#ifndef GUCEF__TEST_UNIQUE_H
#include "gucef_test_unique.h"
#define GUCEF__TEST_UNIQUE_H
#endif /* GUCEF__TEST_UNIQUE_H ? */

#include "AssetWorkbenchLib.h"

#include "TestAssetWorkbenchSupport.h"
#include "TestAssetWorkbenchAssets.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace WBREMOTETEST {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  A real asset store service host on the loopback interface of the test process, for the tests of the connection of the workbench to a remote store
 *  (the same construction as the world of the gucefASSETSTORE tests, which this module's tests do not depend on): a service of the server side
 *  (CLocalAssetStoreService on the table database of the VFS in a folder of the test) behind a DRNUTILS::CRemoteServiceHost with a
 *  CAssetStoreServiceHost, pairing windows of the host, and a catalogue with a pack that the workbench side does not have. The host pulses on
 *  the global pulse generator, which the test thread drives. The keys are fixed so that a restarted host is the same service.
 */
class CRemoteWorld
{
    public:

    GUCEF::CORE::PulseGeneratorPtr pulse;
    GUCEF::TThreadId previousDriver;
    GUCEF::CORE::CString name;
    GUCEF::CORE::CString directory;
    GUCEF::DRNUTILS::CRemoteServiceHostSettings hostSettings;
    GUCEF::ASSETSTORE::CAssetStoreServiceHostSettings bindingSettings;
    GUCEF::ASSETSTORE::CAssetStoreServiceConfig serverConfig;
    GUCEF::ASSETSTORE::TAssetStoreServicePtr serverService;
    GUCEF::DRNUTILS::CRemoteServiceHost* host;
    GUCEF::ASSETSTORE::CAssetStoreServiceHost* binding;
    GUCEF::ASSETWORKBENCH::CAssetMounter mounter;
    GUCEF::UInt16 hostDiscoveryPort;
    GUCEF::UInt16 finderPort;
    GUCEF::UInt32 packCount;
    GUCEF::CORE::CString lastCode;          /**< the code of the last pairing window, to see that nothing keeps it (tests only) */

    CRemoteWorld( const GUCEF::CORE::CString& worldName, bool lanDiscovery = false )
        : pulse(), previousDriver( 0 ), name( worldName ), directory(), hostSettings(), bindingSettings(), serverConfig(), serverService(), host( GUCEF_NULL ),
          binding( GUCEF_NULL ), mounter(), hostDiscoveryPort( 0 ), finderPort( 0 ), packCount( 0 ), lastCode()
    {
        using namespace GUCEF;
        pulse = CORE::CCoreGlobal::Instance()->GetPulseGenerator();
        previousDriver = pulse->GetPulseDriverThreadId();
        pulse->SetPulseDriverThreadId( MT::GetCurrentThreadID() );
        directory = MakeCleanTestDir( "remote_" + name );
        CORE::CreateDirs( directory );

        hostSettings.serviceName = "gucef.assetstore.test";
        hostSettings.drnSessionName = "assetstore";
        hostSettings.sessionPort = TESTFW::FindFreeTcpAndUdpPort( 0 );
        hostSettings.bindAddress = "127.0.0.1";
        hostSettings.linkTimeoutMs = 1500;
        hostSettings.echoIntervalMs = 250;
        hostSettings.tickIntervalMs = 20;
        hostSettings.drainSeconds = 1;
        hostSettings.expectedBackSeconds = 1;
        hostSettings.lanDiscovery = lanDiscovery;
        hostSettings.adminRoles.clear();
        hostSettings.adminRoles.insert( "owner" );
        hostSettings.staticPrivateKeyHex = "6a1b2c3d4e5f60718293a4b5c6d7e8f90112233445566778899aabbccddeeff1";
        hostSettings.issuerSeedHex = "8766554433221100ffeeddccbbaa99887766554433221100ffeeddccbbaa9989";
        if ( lanDiscovery )
        {
            // announced on the loopback interface only, on ports of this test: nothing of the real network hears it, and it hears nothing of it
            hostDiscoveryPort = TESTFW::FindFreeUdpPort( 47871 );
            finderPort = TESTFW::FindFreeUdpPort( 47872 );
            hostSettings.pairingPort = TESTFW::FindFreeTcpAndUdpPort( 0 );
            hostSettings.lanDiscoveryAdvertiser.serviceType = "_gucef-assetstore._tcp";
            hostSettings.lanDiscoveryAdvertiser.displayName = "Workbench test store";
            AddLoopbackClient( hostSettings.lanDiscoveryAdvertiser.clientConfig, hostDiscoveryPort, finderPort );
        }

        bindingSettings.stagingDirectory = CORE::CombinePath( directory, "staging" );
        bindingSettings.stagingVfsRoot = "astest/wbremote_" + name + "_staging";
        bindingSettings.commandJournalDirectory = CORE::CombinePath( directory, "commands" );
        bindingSettings.jobProgressIntervalMs = 100;
    }

    ~CRemoteWorld()
    {
        StopServer();
        Pump( 20 );
        mounter.UnmountAll();
        pulse->SetPulseDriverThreadId( previousDriver );
    }

    /** A "DiscoveryClient" node of the broadcast type that only talks unicast on the loopback interface */
    static void AddLoopbackClient( GUCEF::CORE::CDataNode& clientConfig, GUCEF::UInt16 port, GUCEF::UInt16 peerPort )
    {
        using namespace GUCEF;
        CORE::CDataNode* client = clientConfig.AddChild( "DiscoveryClient", GUCEF_DATATYPE_OBJECT );
        client->SetAttribute( "type", "broadcast" );
        client->SetAttribute( "port", CORE::ToString( port ) );
        client->SetAttribute( "useLimitedBroadcast", "false" );
        client->SetAttribute( "useDirectedBroadcasts", "false" );
        if ( 0 != peerPort )
            client->SetAttribute( "extraTargets", "127.0.0.1:" + CORE::ToString( peerPort ) );
    }

    void Pump( GUCEF::UInt32 durationInMs )
    {
        using namespace GUCEF;
        const UInt64 start = MT::PrecisionTickCount();
        const UInt64 resolution = MT::PrecisionTimerResolution();
        while ( ( MT::PrecisionTickCount() - start ) * 1000 / resolution < durationInMs )
        {
            pulse->ForceDirectPulse();
            MT::PrecisionDelay( 2 );
        }
    }

    /** "127.0.0.1:port" of the session listener */
    GUCEF::CORE::CString GetEndpoint( void ) const
    {
        return "127.0.0.1:" + GUCEF::CORE::ToString( hostSettings.sessionPort );
    }

    /** The service of the server side, opened, and the host serving it */
    bool StartServer( GUCEF::CORE::CString& error )
    {
        using namespace GUCEF;
        const CORE::CString storeDir = directory + "/store";
        CORE::CreateDirs( storeDir );
        const CORE::CString root = "avstore/remote_" + name;
        serverConfig.implementationId = ASSETSTORE::CAssetStoreServiceConfig::LocalImplementationId;
        serverConfig.primaryStore.locations[ 0 ].vfsRoot = root;
        serverConfig.primaryStore.locations[ 0 ].fileSystemPath = storeDir;
        serverConfig.primaryStore.database.database = root + "/catalogue";
        serverConfig.primaryStore.analysisConcurrency = 2;
        serverService = ASSETSTORE::CAssetStoreGlobal::Instance()->CreateService( serverConfig, error );
        if ( serverService.IsNULL() )
            return false;
        if ( ASSETSTORE::ASSETSTORE_STATUS_OK != serverService->Open() )
        {
            error = "the service of the server side did not open: " + serverService->GetStatusMessage();
            return false;
        }
        return StartHost( error );
    }

    bool StartHost( GUCEF::CORE::CString& error )
    {
        using namespace GUCEF;
        host = GUCEF_NEW DRNUTILS::CRemoteServiceHost( pulse, hostSettings );
        host->SetStateDirectory( CORE::CombinePath( directory, "state" ) );
        if ( !host->Start( error ) )
            return false;
        binding = GUCEF_NEW ASSETSTORE::CAssetStoreServiceHost( *host, serverService, bindingSettings );
        UInt8 key[ 32 ];
        memset( key, 0x5a, sizeof( key ) );
        binding->SetJournalKey( key, sizeof( key ) );
        return binding->Attach( error );
    }

    void StopHost( void )
    {
        if ( GUCEF_NULL != binding )
        {
            binding->Detach();
            GUCEF_DELETE binding;
            binding = GUCEF_NULL;
        }
        if ( GUCEF_NULL != host )
        {
            host->Stop();
            Pump( 20 );
            GUCEF_DELETE host;
            host = GUCEF_NULL;
        }
    }

    void StopServer( void )
    {
        StopHost();
        if ( !serverService.IsNULL() )
        {
            serverService->Close();
            serverService.Unlink();
        }
    }

    /** A pack on the disk of the server that its catalogue ingests: a model and a picture */
    GUCEF::CORE::CString WritePack( const GUCEF::CORE::CString& packName )
    {
        using namespace GUCEF;
        const CORE::CString pack = directory + "/packs/" + packName;
        CORE::CreateDirs( pack + "/Textures" );
        WriteTestFile( pack + "/pack.json", "{ \"title\": \"" + packName + "\", \"author\": \"Forge Works\", \"tags\": [ \"sword\", \"medieval\" ], "
                                            "\"licence\": { \"name\": \"CC BY 4.0\", \"restrictions\": [ \"noMachineLearning\" ] } }" );
        WriteTestFile( pack + "/sword_model.gltf", BuildTestGltf() );
        WriteTestPng( pack + "/Textures/sword_d.png", 16, 8 );
        return pack;
    }

    /** Adds a pack to the catalogue of the server and waits until its two assets are found by a search of the server side service */
    bool AddPack( const GUCEF::CORE::CString& packName )
    {
        using namespace GUCEF;
        const CORE::CString native = WritePack( packName );
        const CORE::CString contentPath = "disk/" + CORE::UInt32ToString( ++packCount ) + "_" + ASSETSTORE::CAssetStoreSafeNames::MakeSafePart( packName );
        if ( !mounter.Mount( contentPath, native ) )
            return false;
        ASSETSTORE::CAssetIngestRequest request;
        request.sourceId = ASSETSTORE::CPrimaryAssetStoreSettings::DiskSourceId;
        request.contentPath = contentPath;
        request.keep = ASSETSTORE::AssetKeepPolicyToString( ASSETSTORE::ASSETKEEP_BOTH );
        request.locationId = serverConfig.primaryStore.defaultLocationId;
        serverService->IngestAsync( CORE::CString::Empty, request );
        for ( UInt32 i=0; i<2000; ++i )
        {
            Pump( 10 );
            ASSETSTORE::CAssetSearchQuery query;
            query.text = "sword";
            ASSETSTORE::CAssetSearchResultPage page;
            if ( ASSETSTORE::ASSETSTORE_STATUS_OK == serverService->Search( query, page ) && page.total >= 2 )
                return true;
        }
        return false;
    }

    /** Opens a pairing window that grants the owner role; the one-time code */
    bool OpenPairing( GUCEF::CORE::CString& code, GUCEF::CORE::CString& error )
    {
        using namespace GUCEF;
        DRNUTILS::CPairingWindowRequest window;
        window.deviceKind = "viewer";
        window.roles.insert( "owner" );
        CORE::CString windowId;
        UInt64 expires = 0;
        if ( !host->OpenPairingWindow( window, DRNUTILS::CRemoteServiceHost::LocalAdmin, windowId, code, expires, error ) )
            return false;
        lastCode = code;
        return true;
    }

    /** The settings of the connection of the workbench to this world; the pairing port is that of the window that is open */
    GUCEF::ASSETWORKBENCH::CWorkbenchStoreConnection::SSettings MakeConnectionSettings( bool withLanDiscovery = false ) const
    {
        using namespace GUCEF;
        ASSETWORKBENCH::CWorkbenchStoreConnection::SSettings settings;
        settings.endpoint = GetEndpoint();
        settings.pairingPort = GUCEF_NULL != host && host->GetPairingPort() > 0 ? host->GetPairingPort() : 47401;
        settings.lanDiscovery = withLanDiscovery;
        settings.serviceName = hostSettings.serviceName;
        settings.sessionName = hostSettings.drnSessionName;
        settings.connectTimeoutMs = 8000;
        settings.retryBaseMs = 200;
        settings.retryMaxMs = 1000;
        settings.serviceSettings.SetNodeType( GUCEF_DATATYPE_OBJECT );
        settings.serviceSettings.SetAttribute( "linkTimeoutMs", (UInt32) 1500 );
        settings.serviceSettings.SetAttribute( "echoIntervalMs", (UInt32) 250 );
        settings.serviceSettings.SetAttribute( "reconnectBaseMs", (UInt32) 200 );
        settings.serviceSettings.SetAttribute( "reconnectMaxMs", (UInt32) 2000 );
        if ( withLanDiscovery )
        {
            settings.finder.findTimeoutMs = 3000;
            settings.finder.findMinCandidates = 0;
            AddLoopbackClient( settings.finder.clientConfig, finderPort, hostDiscoveryPort );
        }
        return settings;
    }
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WBREMOTETEST */

/*-------------------------------------------------------------------------*/

#endif /* TESTWORKBENCHREMOTEWORLD_H ? */
