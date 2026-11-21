/*
 *  gucefASSETSTORE_TestApp: tests of the gucefASSETSTORE module
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

#ifndef TESTASSETSTOREREMOTEWORLD_H
#define TESTASSETSTOREREMOTEWORLD_H

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

#ifndef GUCEF_AUTH_CMEMORYCREDENTIALSTORE_H
#include "gucefAUTH_CMemoryCredentialStore.h"
#define GUCEF_AUTH_CMEMORYCREDENTIALSTORE_H
#endif /* GUCEF_AUTH_CMEMORYCREDENTIALSTORE_H ? */

#ifndef GUCEF_DRNUTILS_CREMOTESERVICEHOST_H
#include "gucefDRNUTILS_CRemoteServiceHost.h"
#define GUCEF_DRNUTILS_CREMOTESERVICEHOST_H
#endif /* GUCEF_DRNUTILS_CREMOTESERVICEHOST_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETSTOREGLOBAL_H
#include "gucefASSETSTORE_CAssetStoreGlobal.h"
#define GUCEF_ASSETSTORE_CASSETSTOREGLOBAL_H
#endif /* GUCEF_ASSETSTORE_CASSETSTOREGLOBAL_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETSTORESERVICEHOST_H
#include "gucefASSETSTORE_CAssetStoreServiceHost.h"
#define GUCEF_ASSETSTORE_CASSETSTORESERVICEHOST_H
#endif /* GUCEF_ASSETSTORE_CASSETSTORESERVICEHOST_H ? */

#ifndef GUCEF_ASSETSTORE_CREMOTEASSETSTORESERVICE_H
#include "gucefASSETSTORE_CRemoteAssetStoreService.h"
#define GUCEF_ASSETSTORE_CREMOTEASSETSTORESERVICE_H
#endif /* GUCEF_ASSETSTORE_CREMOTEASSETSTORESERVICE_H ? */

#ifndef GUCEF__TEST_UNIQUE_H
#include "gucef_test_unique.h"
#define GUCEF__TEST_UNIQUE_H
#endif /* GUCEF__TEST_UNIQUE_H ? */

#ifndef TESTASSETSTORESUPPORT_H
#include "TestAssetStoreSupport.h"
#define TESTASSETSTORESUPPORT_H
#endif /* TESTASSETSTORESUPPORT_H ? */

#ifndef TESTASSETSTORESERVICESUITE_H
#include "TestAssetStoreServiceSuite.h"
#define TESTASSETSTORESERVICESUITE_H
#endif /* TESTASSETSTORESERVICESUITE_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace ASSETSTORETEST {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The asset store as a remote service in the test process (gucefASSETSTORE_RemoteService_Design.md 10.2): an in-process
 *  service of the server side (CLocalAssetStoreService) behind a DRNUTILS::CRemoteServiceHost with a
 *  CAssetStoreServiceHost on the loopback interface, devices paired through pairing windows of the host, and remote
 *  clients (CRemoteAssetStoreService) made by the factory of the module with the credentials of a memory store. The
 *  host and the clients pulse on the global pulse generator, which the test thread drives (run in a network namespace
 *  with the loopback up: unshare -rn). Times are short for the tests; the keys are fixed so a restarted host is the same
 *  service. Nothing here is logged of the pairing codes.
 */
class CRemoteAssetStoreWorld
{
    public:

    GUCEF::CORE::PulseGeneratorPtr pulse;
    GUCEF::TThreadId previousDriver;
    GUCEF::CORE::CString directory;
    GUCEF::DRNUTILS::CRemoteServiceHostSettings hostSettings;
    GUCEF::ASSETSTORE::CAssetStoreServiceHostSettings bindingSettings;
    GUCEF::ASSETSTORE::CAssetStoreServiceConfig serverConfig;
    GUCEF::ASSETSTORE::TAssetStoreServicePtr serverService;
    GUCEF::DRNUTILS::CRemoteServiceHost* host;
    GUCEF::ASSETSTORE::CAssetStoreServiceHost* binding;
    GUCEF::AUTH::CMemoryCredentialStore clientStore;
    GUCEF::CORE::CString::StringSet pairedSlots;
    GUCEF::CORE::CString lastCode;          /**< the code of the last pairing, for the check that no log holds it (tests only) */

    explicit CRemoteAssetStoreWorld( const GUCEF::CORE::CString& name )
        : pulse(), previousDriver( 0 ), directory(), hostSettings(), bindingSettings(), serverConfig(), serverService(), host( GUCEF_NULL ),
          binding( GUCEF_NULL ), clientStore(), pairedSlots(), lastCode()
    {
        using namespace GUCEF;
        pulse = CORE::CCoreGlobal::Instance()->GetPulseGenerator();
        previousDriver = pulse->GetPulseDriverThreadId();
        pulse->SetPulseDriverThreadId( MT::GetCurrentThreadID() );
        directory = TestDirectory( "remote_" + name );
        CORE::DeleteDir( directory, true );
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
        hostSettings.lanDiscovery = false;
        hostSettings.adminRoles.clear();
        hostSettings.adminRoles.insert( "owner" );
        hostSettings.staticPrivateKeyHex = "6a1b2c3d4e5f60718293a4b5c6d7e8f90112233445566778899aabbccddeeff1";
        hostSettings.issuerSeedHex = "8766554433221100ffeeddccbbaa99887766554433221100ffeeddccbbaa9989";

        bindingSettings.stagingDirectory = CORE::CombinePath( directory, "staging" );
        bindingSettings.stagingVfsRoot = "astest/remote_" + name + "_staging";
        bindingSettings.commandJournalDirectory = CORE::CombinePath( directory, "commands" );
        bindingSettings.jobProgressIntervalMs = 100;
        bindingSettings.learningRoles.insert( "mlexport" );        // a role that exports raw data into a training pipeline; the default has none
    }

    ~CRemoteAssetStoreWorld()
    {
        StopServer();
        PumpFor( 20 );
        pulse->SetPulseDriverThreadId( previousDriver );
    }

    /** The service of the server side on the configuration, opened, and the host serving it */
    bool StartServer( const GUCEF::ASSETSTORE::CAssetStoreServiceConfig& config, GUCEF::CORE::CString& error )
    {
        using namespace GUCEF;
        serverConfig = config;
        serverConfig.implementationId = ASSETSTORE::CAssetStoreServiceConfig::LocalImplementationId;
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

    /** The host and its binding to the service (again, after StopHost(): the same keys, port, state and journal) */
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
            PumpFor( 20 );
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

    /** Pairs a device with the owner role through a window of the host; its credentials go into the client store under the slot */
    bool Pair( const GUCEF::CORE::CString& slot, GUCEF::CORE::CString& error, const GUCEF::CORE::CString::StringSet* extraRoles = GUCEF_NULL, bool grantOwner = true )
    {
        using namespace GUCEF;
        DRNUTILS::CPairingWindowRequest window;
        window.deviceKind = "viewer";
        if ( grantOwner )
            window.roles.insert( "owner" );
        if ( GUCEF_NULL != extraRoles )
            window.roles.insert( extraRoles->begin(), extraRoles->end() );
        CORE::CString windowId;
        CORE::CString code;
        UInt64 expires = 0;
        if ( !host->IsPairingWindowOpen() && !host->OpenPairingWindow( window, DRNUTILS::CRemoteServiceHost::LocalAdmin, windowId, code, expires, error ) )
            return false;
        lastCode = code;
        AUTH::CSecretString secret( code );
        code.Clear();
        const bool paired = ASSETSTORE::CRemoteAssetStoreService::PairDevice( "tcp://127.0.0.1:" + CORE::ToString( host->GetPairingPort() ), secret,
                                                                              hostSettings.serviceName, hostSettings.drnSessionName, "test device " + slot,
                                                                              "viewer", clientStore, slot, error );
        if ( paired )
            pairedSlots.insert( slot );
        return paired;
    }

    /** The configuration of a client of the device of the slot */
    GUCEF::ASSETSTORE::CAssetStoreServiceConfig ClientConfig( const GUCEF::CORE::CString& slot ) const
    {
        using namespace GUCEF;
        ASSETSTORE::CAssetStoreServiceConfig config;
        config.implementationId = ASSETSTORE::CAssetStoreServiceConfig::RemoteImplementationId;
        config.credentialStore = const_cast< AUTH::CMemoryCredentialStore* >( &clientStore );
        config.implementationSettings.SetName( "service" );
        config.implementationSettings.SetNodeType( GUCEF_DATATYPE_OBJECT );
        config.implementationSettings.SetAttribute( "endpoint", "tcp://127.0.0.1:" + CORE::ToString( hostSettings.sessionPort ) );
        config.implementationSettings.SetAttribute( "credentialsSlot", slot );
        config.implementationSettings.SetAttribute( "connectTimeoutMs", (UInt32) 10000 );
        config.implementationSettings.SetAttribute( "linkTimeoutMs", (UInt32) 1500 );
        config.implementationSettings.SetAttribute( "echoIntervalMs", (UInt32) 250 );
        config.implementationSettings.SetAttribute( "reconnectBaseMs", (UInt32) 200 );
        config.implementationSettings.SetAttribute( "reconnectMaxMs", (UInt32) 2000 );
        config.implementationSettings.SetAttribute( "lanDiscovery", false );
        return config;
    }

    /** A remote client of a device of its own (paired when the slot was not), not opened yet */
    GUCEF::ASSETSTORE::TAssetStoreServicePtr MakeClient( const GUCEF::CORE::CString& slot, GUCEF::CORE::CString& error, const GUCEF::CORE::CString::StringSet* extraRoles = GUCEF_NULL, bool grantOwner = true )
    {
        using namespace GUCEF;
        if ( pairedSlots.find( slot ) == pairedSlots.end() && !Pair( slot, error, extraRoles, grantOwner ) )
            return ASSETSTORE::TAssetStoreServicePtr();
        return ASSETSTORE::CAssetStoreGlobal::Instance()->CreateService( ClientConfig( slot ), error );
    }
};

/*-------------------------------------------------------------------------*/

/** The equivalence suite against a remote client of the world: the suite's configuration is that of the server side */
class CRemoteServiceSuiteTarget : public CAssetStoreServiceSuiteTarget
{
    public:

    GUCEF::CORE::CString label;
    GUCEF::COMCORE::CDatabaseClientSettings database;
    GUCEF::AUTH::ICredentialStore* credentials;
    CRemoteAssetStoreWorld world;

    explicit CRemoteServiceSuiteTarget( const GUCEF::CORE::CString& name )
        : CAssetStoreServiceSuiteTarget(), label( name ), database(), credentials( GUCEF_NULL ), world( name ) {}

    virtual GUCEF::CORE::CString GetLabel( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        return label;
    }

    virtual void ConfigureDatabase( GUCEF::ASSETSTORE::CAssetStoreServiceConfig& config ) GUCEF_VIRTUAL_OVERRIDE
    {
        config.primaryStore.database = database;
        config.credentialStore = credentials;
        if ( GUCEF_NULL != credentials )
        {
            config.primaryStore.credentialProvider = "gucefASSETSTORE.test";
            config.primaryStore.credentialAccount = "catalogue";
        }
    }

    virtual GUCEF::ASSETSTORE::TAssetStoreServicePtr MakeService( const GUCEF::ASSETSTORE::CAssetStoreServiceConfig& config ) GUCEF_VIRTUAL_OVERRIDE
    {
        GUCEF::CORE::CString error;
        if ( !world.StartServer( config, error ) )
        {
            GUCEF_ERROR_LOG( GUCEF::CORE::LOGLEVEL_IMPORTANT, "RemoteServiceSuiteTarget: the server side did not start: " << error );
            return GUCEF::ASSETSTORE::TAssetStoreServicePtr();
        }
        GUCEF::ASSETSTORE::TAssetStoreServicePtr client = world.MakeClient( "suite", error );
        if ( client.IsNULL() )
            GUCEF_ERROR_LOG( GUCEF::CORE::LOGLEVEL_IMPORTANT, "RemoteServiceSuiteTarget: no client: " << error );
        return client;
    }

    /** A client of a device that has the learning role of the host besides the owner role: a machine learning consumer */
    virtual GUCEF::ASSETSTORE::TAssetStoreServicePtr MakeLearningClient( void ) GUCEF_VIRTUAL_OVERRIDE
    {
        GUCEF::CORE::CString::StringSet roles;
        roles.insert( "mlexport" );
        GUCEF::CORE::CString error;
        return world.MakeClient( "learning", error, &roles );
    }

    virtual void Pump( GUCEF::UInt32 durationInMs ) GUCEF_VIRTUAL_OVERRIDE
    {
        PumpFor( durationInMs );
    }
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORETEST */

/*-------------------------------------------------------------------------*/

#endif /* TESTASSETSTOREREMOTEWORLD_H ? */
