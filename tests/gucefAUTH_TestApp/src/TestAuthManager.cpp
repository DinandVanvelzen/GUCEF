/*
 *  gucefAUTH_TestApp: Application for testing gucefAUTH
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
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

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_AUTH_H
#include "gucefAUTH.h"
#define GUCEF_AUTH_H
#endif /* GUCEF_AUTH_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestPump.h"
#include "TestFiles.h"
#include "TestLogCapture.h"
#include "TestSigner.h"
#include "TestRecorder.h"
#include "TestFlowEngine.h"
#include "TestFakeProvider.h"
#include "TestAuthManager.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

/** Records the failure and abandons the test case, for what the rest of it needs */
#define REQUIRE_TRUE(t) do { if ( !(t) ) { GUCEF_TESTFW_ASSERT_TRUE( t ); throw CORE::CString( "required: " #t ); } } while ( 0 )

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

typedef CORE::CTFactory< AUTH::CIAuthProvider, CFakeAuthProvider, MT::CMutex > TFakeProviderFactory;

static TFakeProviderFactory g_fakeFactory;

/** Makes fake providers that have a name */
class CNamedFakeFactory : public TFakeProviderFactory
{
    public:

    explicit CNamedFakeFactory( const CORE::CString& providerName ) : TFakeProviderFactory(), m_name( providerName ) {}

    virtual ~CNamedFakeFactory() {}

    virtual TProductPtr Create( void ) GUCEF_VIRTUAL_OVERRIDE
    {
        TProductPtr product = TFakeProviderFactory::Create();
        static_cast< CFakeAuthProvider* >( product.GetPointerAlways() )->name = m_name;
        return product;
    }

    private:

    CNamedFakeFactory( const CNamedFakeFactory& src );
    CNamedFakeFactory& operator=( const CNamedFakeFactory& src );

    CORE::CString m_name;
};

/*-------------------------------------------------------------------------*/

/** A manager with stores in a directory of its own, a profile key and a recorder of its events */
class CFixture
{
    public:

    CORE::CString dir;
    CTestSigner signer;
    AUTH::CFileCredentialStore fileStore;
    AUTH::CMemoryCredentialStore memoryStore;
    AUTH::CLocalIdentityLinkStore linkStore;
    AUTH::CAuthManager manager;
    CAuthRecorder recorder;
    GUCEF::vector< CORE::CString > callLog;

    explicit CFixture( const CORE::CString& name, bool withSigner = true )
        : dir( MakeTestDirectory( name ) )
        , signer( "profile" )
        , fileStore( CORE::CombinePath( dir, "cred" ) )
        , memoryStore()
        , linkStore( CORE::CombinePath( dir, "links.json" ) )
        , manager()
        , recorder()
        , callLog()
    {
        linkStore.SetSigner( &signer );
        manager.SetCredentialStore( Store() );
        manager.SetLinkStore( &linkStore );
        if ( withSigner )
            manager.SetSigner( &signer );
        recorder.SubscribeTo( &manager );
    }

    AUTH::ICredentialStore* Store( void )
    {
        if ( AUTH::CFileCredentialStore::IsAvailable() )
            return &fileStore;
        return &memoryStore;
    }

    CFakeAuthProvider* AddFake( const CORE::CString& name, const CORE::CString& subject = "player-1", const CORE::CString& ns = "fake" )
    {
        TFakeProviderFactory::TProductPtr product = g_fakeFactory.Create();
        CFakeAuthProvider* fake = static_cast< CFakeAuthProvider* >( product.GetPointerAlways() );
        fake->name = name;
        fake->subject = subject;
        fake->accountNamespace = ns;
        fake->callLog = &callLog;
        if ( !manager.AddProvider( product ) )
            return GUCEF_NULL;
        return fake;
    }

    UInt32 LinkCount( void )
    {
        AUTH::TLinkRecordVector links;
        linkStore.GetLinks( signer.GetKeyId(), links );
        return (UInt32) links.size();
    }

    private:

    CFixture( const CFixture& src );
    CFixture& operator=( const CFixture& src );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** Pulses until the future has ended; true if it did in time */
static bool
Await( const CORE::CFutureResult& future, UInt32 timeoutInMs )
{GUCEF_TRACE;

    for ( UInt32 waited=0; waited<timeoutInMs && !future.IsReady(); ++waited )
        PumpFor( 1 );
    return future.IsReady();
}

/*-------------------------------------------------------------------------*/

static CORE::TTaskStatus
StatusOf( const CORE::CFutureResult& future )
{GUCEF_TRACE;

    CORE::CTaskPtr task = future.GetResult();
    if ( task.IsNULL() )
        return CORE::TTaskStatus::TASKSTATUS_UNDEFINED;
    return task->GetTaskStatus();
}

/*-------------------------------------------------------------------------*/

static bool
Prerequisites( void )
{GUCEF_TRACE;

    return JsonCodecAvailable() && AUTH::CAuthCrypto::IsSignatureAvailable();
}

/*-------------------------------------------------------------------------*/

/** The index of the first record of the kind, or -1 */
static int
IndexOf( const CAuthRecorder& recorder, CAuthRecorder::EKind kind )
{GUCEF_TRACE;

    for ( size_t i=0; i<recorder.records.size(); ++i )
        if ( recorder.records[ i ].kind == kind )
            return (int) i;
    return -1;
}

/*-------------------------------------------------------------------------*/

void
PerformAuthManagerTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING auth manager TESTS" );
    GUCEF_TESTFW_SUITE_SCOPE( "AuthManager" );

    RegisterFakeLoginTask();
    const UInt32 mainThreadId = MT::GetCurrentTaskID();

    GUCEF_TESTFW_TESTCASE( "Test: Only available, enabled and configured providers are offered, the last used first" )
        GUCEF_TESTFW_SKIP_IF( !Prerequisites(), "The json or ed25519 codec is not available" );
        try
        {
            CFixture f( "mgr_offered" );
            CFakeAuthProvider* alpha = f.AddFake( "alpha" );
            CFakeAuthProvider* beta = f.AddFake( "beta" );
            CFakeAuthProvider* gamma = f.AddFake( "gamma" );
            ASSERT_TRUE( GUCEF_NULL != alpha && GUCEF_NULL != beta && GUCEF_NULL != gamma );
            beta->available = false;
            gamma->capabilities |= AUTH::AUTHCAP_REQUIRES_CLIENT_ID;

            CORE::CString::StringVector names;
            f.manager.GetProviderNames( names );
            ASSERT_TRUE( 3 == names.size() );
            CORE::CString::StringVector offered;
            f.manager.GetOfferedProviders( offered );
            ASSERT_TRUE( 1 == offered.size() && "alpha" == offered[ 0 ] );
            ASSERT_TRUE( f.manager.IsProviderOffered( "alpha" ) && !f.manager.IsProviderOffered( "beta" ) && !f.manager.IsProviderOffered( "gamma" ) && !f.manager.IsProviderOffered( "nobody" ) );

            // a client id in the settings offers the provider that needs one, and reaches the provider
            AUTH::CAuthSettings settings = f.manager.GetSettings();
            AUTH::CAuthProviderSettings gammaSettings;
            gammaSettings.clientId = "gamma-client-id";
            settings.SetProviderSettings( "gamma", gammaSettings );
            f.manager.SetSettings( settings );
            ASSERT_TRUE( f.manager.IsProviderOffered( "gamma" ) && "gamma-client-id" == gamma->configuredClientId );

            // enabled providers narrow it down
            settings.enabledProviders.push_back( "alpha" );
            settings.enabledProviders.push_back( "gamma" );
            f.manager.SetSettings( settings );
            f.manager.GetOfferedProviders( offered );
            ASSERT_TRUE( 2 == offered.size() && "alpha" == offered[ 0 ] && "gamma" == offered[ 1 ] );

            // the last used one is first
            settings.lastUsedProviders.push_back( "gamma" );
            f.manager.SetSettings( settings );
            f.manager.GetOfferedProviders( offered );
            ASSERT_TRUE( 2 == offered.size() && "gamma" == offered[ 0 ] && "alpha" == offered[ 1 ] );
            settings.enabledProviders.clear();
            beta->available = true;
            f.manager.SetSettings( settings );
            f.manager.GetOfferedProviders( offered );
            ASSERT_TRUE( 3 == offered.size() && "gamma" == offered[ 0 ] && "alpha" == offered[ 1 ] && "beta" == offered[ 2 ] );

            // providers are added once by name and removed
            TFakeProviderFactory::TProductPtr duplicate = g_fakeFactory.Create();
            static_cast< CFakeAuthProvider* >( duplicate.GetPointerAlways() )->name = "alpha";
            ASSERT_FALSE( f.manager.AddProvider( duplicate ) );
            ASSERT_FALSE( f.manager.AddProvider( AUTH::CIAuthProviderPtr() ) );
            TFakeProviderFactory::TProductPtr nameless = g_fakeFactory.Create();
            static_cast< CFakeAuthProvider* >( nameless.GetPointerAlways() )->name = CORE::CString::Empty;
            ASSERT_FALSE( f.manager.AddProvider( nameless ) );
            ASSERT_TRUE( f.manager.GetProvider( "alpha" ) == alpha );
            ASSERT_TRUE( f.manager.RemoveProvider( "alpha" ) && !f.manager.RemoveProvider( "alpha" ) && GUCEF_NULL == f.manager.GetProvider( "alpha" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The providers of a factory are made and configured by the manager" )
        GUCEF_TESTFW_SKIP_IF( !Prerequisites(), "The json or ed25519 codec is not available" );
        try
        {
            CNamedFakeFactory firstFactory( "first" );
            CNamedFakeFactory secondFactory( "second" );
            AUTH::CAuthProviderFactory factory;
            factory.RegisterConcreteFactory( "first", &firstFactory );
            factory.RegisterConcreteFactory( "second", &secondFactory );

            CFixture f( "mgr_factory" );
            f.manager.SetProviderFactory( &factory );

            CORE::CDataNode config( "Auth", GUCEF_DATATYPE_OBJECT );
            AUTH::CAuthSettings settings;
            AUTH::CAuthProviderSettings firstSettings;
            firstSettings.clientId = "first-client";
            settings.SetProviderSettings( "first", firstSettings );
            settings.lastUsedProviders.push_back( "second" );
            settings.SaveConfig( config );
            ASSERT_TRUE( f.manager.LoadConfig( config ) );

            CORE::CString::StringVector names;
            f.manager.GetProviderNames( names );
            ASSERT_TRUE( 2 == names.size() );
            CFakeAuthProvider* first = static_cast< CFakeAuthProvider* >( f.manager.GetProvider( "first" ) );
            ASSERT_TRUE( GUCEF_NULL != first && "first-client" == first->configuredClientId );
            ASSERT_TRUE( 1 == f.manager.GetSettings().lastUsedProviders.size() && "second" == f.manager.GetSettings().lastUsedProviders[ 0 ] );
            ASSERT_TRUE( 0 == f.manager.RefreshProviders() );

            CORE::CDataNode saved( "Auth", GUCEF_DATATYPE_OBJECT );
            ASSERT_TRUE( f.manager.SaveConfig( saved ) );
            AUTH::CAuthSettings back;
            ASSERT_TRUE( back.LoadConfig( saved ) && "second" == back.lastUsedProviders[ 0 ] && back.providers.size() == 1 );

            f.manager.RemoveProvider( "first" );
            f.manager.RemoveProvider( "second" );
            factory.UnregisterAllConcreteFactories();
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The name of the account of a login that worked is remembered for the next one, and only for a provider that takes a login hint; an explicit hint wins" )
        GUCEF_TESTFW_SKIP_IF( !Prerequisites(), "The json or ed25519 codec is not available" );
        try
        {
            // two providers of the registry of the process: one takes a login hint, one does not
            AUTH::CAuthProviderDescriptor hintful;
            hintful.name = "hintful";
            hintful.displayName = "Hintful";
            hintful.accountProviderId = "hintful";
            hintful.protocol = AUTH::AUTHPROTOCOL_PLATFORM;
            hintful.supportsLoginHint = true;
            AUTH::CAuthProviderDescriptor hintless = hintful;
            hintless.name = "hintless";
            hintless.displayName = "Hintless";
            hintless.accountProviderId = "hintless";
            hintless.supportsLoginHint = false;
            AUTH::CAuthProviderRegistry& registry = AUTH::CAuthGlobal::Instance()->GetDescriptorRegistry();
            ASSERT_TRUE( registry.RegisterDescriptor( hintful ) && registry.RegisterDescriptor( hintless ) );

            {
                CLogCapture capture;
                CFixture f( "mgr_hint" );
                CFakeAuthProvider* one = f.AddFake( "hintful", "player-1", "hintful" );
                CFakeAuthProvider* two = f.AddFake( "hintless", "player-2", "hintless" );
                REQUIRE_TRUE( GUCEF_NULL != one && GUCEF_NULL != two );
                one->resultLoginHint = "j\xC3\xBCrgen+x@example.invalid";
                two->resultLoginHint = "other@example.invalid";

                // nothing to prefill at the first login
                AUTH::CAuthOptions options;
                CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
                ASSERT_FALSE( f.manager.HasLoginHints() );
                ASSERT_TRUE( f.manager.Login( "hintful", options, future ) && Await( future, 3000 ) );
                PumpFor( 20 );
                ASSERT_TRUE( one->lastOptions.loginHint.IsNULLOrEmpty() );

                // the name is kept, the application is told to save the settings
                CORE::CString remembered;
                ASSERT_TRUE( f.manager.GetSettings().GetLoginHint( "hintful", remembered ) && one->resultLoginHint == remembered && f.manager.HasLoginHints() );
                ASSERT_TRUE( f.recorder.Count( CAuthRecorder::KIND_SETTINGS ) >= 1 );

                // the next login gets it
                ASSERT_TRUE( f.manager.Login( "hintful", options, future ) && Await( future, 3000 ) );
                PumpFor( 20 );
                ASSERT_TRUE( one->resultLoginHint == one->lastOptions.loginHint );

                // an explicit hint is not replaced
                options.loginHint = "explicit@example.invalid";
                ASSERT_TRUE( f.manager.Login( "hintful", options, future ) && Await( future, 3000 ) );
                PumpFor( 20 );
                ASSERT_TRUE( "explicit@example.invalid" == one->lastOptions.loginHint );
                options.loginHint.Clear();

                // a provider that takes none is neither handed a name nor does it keep one for the next time it might
                ASSERT_TRUE( f.manager.Login( "hintless", options, future ) && Await( future, 3000 ) );
                PumpFor( 20 );
                ASSERT_TRUE( f.manager.Login( "hintless", options, future ) && Await( future, 3000 ) );
                PumpFor( 20 );
                ASSERT_TRUE( two->lastOptions.loginHint.IsNULLOrEmpty() );

                // forgotten: the settings are changed (and saved by the application), the next login starts empty
                const UInt32 settingsEvents = f.recorder.Count( CAuthRecorder::KIND_SETTINGS );
                ASSERT_TRUE( f.manager.ClearLoginHints() && !f.manager.HasLoginHints() );
                ASSERT_TRUE( f.recorder.Count( CAuthRecorder::KIND_SETTINGS ) == settingsEvents + 1 );
                ASSERT_FALSE( f.manager.ClearLoginHints() );
                ASSERT_TRUE( f.manager.Login( "hintful", options, future ) && Await( future, 3000 ) );
                PumpFor( 20 );
                ASSERT_TRUE( one->lastOptions.loginHint.IsNULLOrEmpty() );

                // the setting off: not used, not kept
                AUTH::CAuthSettings settings = f.manager.GetSettings();
                settings.rememberLoginHints = false;
                f.manager.SetSettings( settings );
                ASSERT_FALSE( f.manager.HasLoginHints() );
                ASSERT_TRUE( f.manager.Login( "hintful", options, future ) && Await( future, 3000 ) );
                PumpFor( 20 );
                ASSERT_TRUE( one->lastOptions.loginHint.IsNULLOrEmpty() && f.manager.GetSettings().loginHints.empty() );

                // never in a log
                ASSERT_FALSE( capture.Contains( "example.invalid" ) );
            }
            registry.UnregisterDescriptor( "hintful" );
            registry.UnregisterDescriptor( "hintless" );
        }
        catch ( ... )
        {
            AUTH::CAuthGlobal::Instance()->GetDescriptorRegistry().UnregisterDescriptor( "hintful" );
            AUTH::CAuthGlobal::Instance()->GetDescriptorRegistry().UnregisterDescriptor( "hintless" );
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The name of an account is not remembered for a login that failed or was cancelled; it survives saving and loading the settings" )
        GUCEF_TESTFW_SKIP_IF( !Prerequisites(), "The json or ed25519 codec is not available" );
        try
        {
            AUTH::CAuthProviderDescriptor hintful;
            hintful.name = "hintful2";
            hintful.displayName = "Hintful";
            hintful.accountProviderId = "hintful2";
            hintful.protocol = AUTH::AUTHPROTOCOL_PLATFORM;
            hintful.supportsLoginHint = true;
            AUTH::CAuthGlobal::Instance()->GetDescriptorRegistry().RegisterDescriptor( hintful );
            {
                CFixture f( "mgr_hint2" );
                CFakeAuthProvider* one = f.AddFake( "hintful2", "player-1", "hintful2" );
                REQUIRE_TRUE( GUCEF_NULL != one );
                one->resultLoginHint = "me@example.invalid";

                AUTH::CAuthOptions options;
                CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );

                // fails
                one->loginBehaviour = FAKE_FAIL;
                one->failReason = AUTH::AUTHFAIL_NETWORK;
                ASSERT_TRUE( f.manager.Login( "hintful2", options, future ) && Await( future, 3000 ) );
                PumpFor( 20 );
                ASSERT_FALSE( f.manager.HasLoginHints() );

                // cancelled
                one->loginBehaviour = FAKE_HANG;
                ASSERT_TRUE( f.manager.Login( "hintful2", options, future ) );
                PumpFor( 20 );
                ASSERT_TRUE( f.manager.CancelLogin( "hintful2" ) && Await( future, 6000 ) );
                PumpFor( 20 );
                ASSERT_FALSE( f.manager.HasLoginHints() );

                // works: remembered, and a settings round trip keeps it
                one->loginBehaviour = FAKE_COMPLETE;
                ASSERT_TRUE( f.manager.Login( "hintful2", options, future ) && Await( future, 3000 ) );
                PumpFor( 20 );
                ASSERT_TRUE( f.manager.HasLoginHints() );
                CORE::CDataNode node( "Auth", GUCEF_DATATYPE_OBJECT );
                ASSERT_TRUE( f.manager.GetSettings().SaveConfig( node ) );
                AUTH::CAuthSettings loaded;
                ASSERT_TRUE( loaded.LoadConfig( node ) );
                CORE::CString hint;
                ASSERT_TRUE( loaded.GetLoginHint( "hintful2", hint ) && "me@example.invalid" == hint );
            }
            AUTH::CAuthGlobal::Instance()->GetDescriptorRegistry().UnregisterDescriptor( "hintful2" );
        }
        catch ( ... )
        {
            AUTH::CAuthGlobal::Instance()->GetDescriptorRegistry().UnregisterDescriptor( "hintful2" );
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A login that says the default time gets the time of the settings, a device code gets what the provider says up to half an hour, an explicit time stays" )
        GUCEF_TESTFW_SKIP_IF( !Prerequisites(), "The json or ed25519 codec is not available" );
        try
        {
            CFixture f( "mgr_timeout" );
            CFakeAuthProvider* alpha = f.AddFake( "alpha" );
            REQUIRE_TRUE( GUCEF_NULL != alpha );
            CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );

            // ten minutes unless told otherwise
            ASSERT_TRUE( f.manager.Login( "alpha", AUTH::CAuthOptions(), future ) && Await( future, 3000 ) );
            PumpFor( 20 );
            ASSERT_TRUE( 600000 == alpha->lastOptions.timeoutInMs );

            // the setting, kept in its range
            AUTH::CAuthSettings settings = f.manager.GetSettings();
            settings.loginTimeoutInSeconds = 900;
            f.manager.SetSettings( settings );
            ASSERT_TRUE( f.manager.Login( "alpha", AUTH::CAuthOptions(), future ) && Await( future, 3000 ) );
            PumpFor( 20 );
            ASSERT_TRUE( 900000 == alpha->lastOptions.timeoutInMs );
            settings.loginTimeoutInSeconds = 5;
            f.manager.SetSettings( settings );
            ASSERT_TRUE( f.manager.Login( "alpha", AUTH::CAuthOptions(), future ) && Await( future, 3000 ) );
            PumpFor( 20 );
            ASSERT_TRUE( 60000 == alpha->lastOptions.timeoutInMs );

            // the caller who wants another time says it
            AUTH::CAuthOptions explicitOptions;
            explicitOptions.timeoutInMs = 120000;
            ASSERT_TRUE( f.manager.Login( "alpha", explicitOptions, future ) && Await( future, 3000 ) );
            PumpFor( 20 );
            ASSERT_TRUE( 120000 == alpha->lastOptions.timeoutInMs );

            // a device code lives as long as the provider says: half an hour is the ceiling of what is waited for
            AUTH::CAuthOptions deviceOptions;
            deviceOptions.preferDeviceCode = true;
            ASSERT_TRUE( f.manager.Login( "alpha", deviceOptions, future ) && Await( future, 3000 ) );
            PumpFor( 20 );
            ASSERT_TRUE( 1800000 == alpha->lastOptions.timeoutInMs );

            // the manager ends a login of a provider that hangs after the time and its margin, not before
            struct SGuard { ~SGuard() { AUTH::SetMonotonicClockOffsetInMs( 0 ); } } guard;
            settings.loginTimeoutInSeconds = 600;
            f.manager.SetSettings( settings );
            alpha->loginBehaviour = FAKE_IGNORE_CANCEL;
            ASSERT_TRUE( f.manager.Login( "alpha", AUTH::CAuthOptions(), future ) );
            AUTH::SetMonotonicClockOffsetInMs( 540000 );
            PumpFor( 400 );
            ASSERT_TRUE( f.manager.IsLoginPending( "alpha" ) && !future.IsReady() );
            AUTH::SetMonotonicClockOffsetInMs( 606000 );
            ASSERT_TRUE( Await( future, 3000 ) );
            PumpFor( 20 );
            ASSERT_FALSE( f.manager.IsLoginPending( "alpha" ) );
            ASSERT_TRUE( AUTH::AUTHFAIL_TIMEOUT == f.recorder.Last( CAuthRecorder::KIND_FAILED )->failure.reason );
            AUTH::SetMonotonicClockOffsetInMs( 0 );
        }
        catch ( ... )
        {
            AUTH::SetMonotonicClockOffsetInMs( 0 );
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: An interactive login is stored, linked to the profile and announced" )
        GUCEF_TESTFW_SKIP_IF( !Prerequisites(), "The json or ed25519 codec is not available" );
        try
        {
            CLogCapture capture;
            CFixture f( "mgr_login" );
            CFakeAuthProvider* alpha = f.AddFake( "alpha" );
            alpha->announceAction = true;
            ASSERT_TRUE( AUTH::AUTHLOGINSTATE_GUEST == f.manager.GetLoginState() );
            ASSERT_TRUE( f.signer.GetKeyId() == f.manager.GetProfileKeyId() );

            AUTH::CAuthOptions options;
            options.loginHint = "alice";
            options.forceInteractive = true;
            options.preferDeviceCode = true;
            CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            ASSERT_TRUE( f.manager.Login( "alpha", options, future ) );
            ASSERT_TRUE( f.manager.IsLoginPending( "alpha" ) && !future.IsReady() );
            ASSERT_TRUE( Await( future, 3000 ) );
            PumpFor( 20 );
            ASSERT_FALSE( f.manager.IsLoginPending( "alpha" ) );

            AUTH::CAuthResult result;
            ASSERT_TRUE( AUTH::CAuthResult::TryGetFromFuture( future, result ) );
            ASSERT_TRUE( "fake:player-1" == result.accountRef.ToString() && "alpha" == result.providerName && "Fake Player" == result.displayName );
            ASSERT_TRUE( CFakeAuthProvider::SecretAccessToken == result.accessToken.Reveal() );

            // what the provider was asked
            ASSERT_TRUE( 1 == alpha->beginCount && "alice" == alpha->lastOptions.loginHint && alpha->lastOptions.forceInteractive && alpha->lastOptions.preferDeviceCode );

            // the events: the action for the user, the login, the link and the new state
            ASSERT_TRUE( 1 == f.recorder.Count( CAuthRecorder::KIND_ACTION ) && 1 == f.recorder.Count( CAuthRecorder::KIND_SUCCEEDED ) );
            ASSERT_TRUE( IndexOf( f.recorder, CAuthRecorder::KIND_ACTION ) < IndexOf( f.recorder, CAuthRecorder::KIND_SUCCEEDED ) );
            ASSERT_TRUE( "alpha" == f.recorder.Last( CAuthRecorder::KIND_ACTION )->action.providerName && AUTH::AUTHACTION_OPEN_URL == f.recorder.Last( CAuthRecorder::KIND_ACTION )->action.kind );
            ASSERT_TRUE( 1 == f.recorder.Count( CAuthRecorder::KIND_LINKED ) && "fake:player-1" == f.recorder.Last( CAuthRecorder::KIND_LINKED )->account.accountRef.ToString() );
            ASSERT_TRUE( 1 == f.recorder.Count( CAuthRecorder::KIND_STATE ) );
            ASSERT_TRUE( AUTH::AUTHLOGINSTATE_LINKED == f.recorder.Last( CAuthRecorder::KIND_STATE )->state.state && 1 == f.recorder.Last( CAuthRecorder::KIND_STATE )->state.linkedAccountCount );
            ASSERT_TRUE( 1 == f.recorder.Count( CAuthRecorder::KIND_SETTINGS ) );
            ASSERT_TRUE( 0 == f.recorder.Count( CAuthRecorder::KIND_FAILED ) );
            ASSERT_TRUE( mainThreadId == f.recorder.Last( CAuthRecorder::KIND_SUCCEEDED )->threadId );

            // the state: linked, with a link record that verifies
            ASSERT_TRUE( AUTH::AUTHLOGINSTATE_LINKED == f.manager.GetLoginState() );
            AUTH::TLinkRecordVector links;
            REQUIRE_TRUE( f.manager.GetLinkedAccounts( links ) && 1 == links.size() );
            ASSERT_TRUE( links[ 0 ].VerifySignature() && links[ 0 ].profileKeyId == f.signer.GetKeyId() && links[ 0 ].proofDigest == result.proof.GetDigestHex() );
            CORE::CString profile;
            ASSERT_TRUE( f.linkStore.Resolve( result.accountRef, profile ) && f.signer.GetKeyId() == profile );

            // the session has the tokens and the proof, the refresh token is kept for the next start
            AUTH::CAuthResult session;
            ASSERT_TRUE( f.manager.GetSession( result.accountRef, session ) && session.refreshToken == result.refreshToken && !session.proof.IsEmpty() );
            GUCEF::vector< AUTH::CAuthResult > sessions;
            f.manager.GetSessionAccounts( sessions );
            ASSERT_TRUE( 1 == sessions.size() );
            AUTH::CSecretString stored;
            ASSERT_TRUE( f.Store()->Load( "alpha", "fake:player-1", stored ) && stored == result.refreshToken );

            // and the provider that was used is the first to try next time
            ASSERT_TRUE( 1 == f.manager.GetSettings().lastUsedProviders.size() && "alpha" == f.manager.GetSettings().lastUsedProviders[ 0 ] );

            // not a trace of a token in the log
            ASSERT_FALSE( capture.Contains( "SECRET-" ) );
            ASSERT_TRUE( capture.Contains( "fake:player-1" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A failed login leaves nothing behind and says why" )
        GUCEF_TESTFW_SKIP_IF( !Prerequisites(), "The json or ed25519 codec is not available" );
        try
        {
            CFixture f( "mgr_fail" );
            CFakeAuthProvider* alpha = f.AddFake( "alpha" );
            alpha->loginBehaviour = FAKE_FAIL;
            alpha->failReason = AUTH::AUTHFAIL_DENIED;
            CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            ASSERT_TRUE( f.manager.Login( "alpha", AUTH::CAuthOptions(), future ) );
            ASSERT_TRUE( Await( future, 3000 ) );
            PumpFor( 20 );
            ASSERT_TRUE( future.HasNoFuture() && CORE::TTaskStatus::TASKSTATUS_STARTUP_FAILED == StatusOf( future ) );
            AUTH::CAuthResult none;
            ASSERT_FALSE( AUTH::CAuthResult::TryGetFromFuture( future, none ) );
            ASSERT_TRUE( 1 == f.recorder.Count( CAuthRecorder::KIND_FAILED ) && 0 == f.recorder.Count( CAuthRecorder::KIND_SUCCEEDED ) );
            ASSERT_TRUE( AUTH::AUTHFAIL_DENIED == f.recorder.Last( CAuthRecorder::KIND_FAILED )->failure.reason && "alpha" == f.recorder.Last( CAuthRecorder::KIND_FAILED )->failure.providerName );
            ASSERT_TRUE( AUTH::AUTHLOGINSTATE_GUEST == f.manager.GetLoginState() && 0 == f.LinkCount() );
            ASSERT_TRUE( 0 == f.recorder.Count( CAuthRecorder::KIND_STATE ) && 0 == f.recorder.Count( CAuthRecorder::KIND_SETTINGS ) );
            CORE::CString::StringVector accounts;
            ASSERT_TRUE( f.Store()->ListAccounts( "alpha", accounts ) && accounts.empty() );
            ASSERT_TRUE( f.manager.GetSettings().lastUsedProviders.empty() );
            GUCEF::vector< AUTH::CAuthResult > sessions;
            f.manager.GetSessionAccounts( sessions );
            ASSERT_TRUE( sessions.empty() && !f.manager.IsLoginPending( "alpha" ) );

            // trying again works
            alpha->loginBehaviour = FAKE_COMPLETE;
            ASSERT_TRUE( f.manager.Login( "alpha", AUTH::CAuthOptions(), future ) && Await( future, 3000 ) );
            PumpFor( 20 );
            ASSERT_TRUE( AUTH::AUTHLOGINSTATE_LINKED == f.manager.GetLoginState() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A login that cannot start is refused with the failure in the future" )
        GUCEF_TESTFW_SKIP_IF( !Prerequisites(), "The json or ed25519 codec is not available" );
        try
        {
            CFixture f( "mgr_refuse" );
            CFakeAuthProvider* refusing = f.AddFake( "refusing" );
            refusing->loginBehaviour = FAKE_REFUSE;
            CFakeAuthProvider* away = f.AddFake( "away" );
            away->available = false;
            CFakeAuthProvider* hanging = f.AddFake( "hanging" );
            hanging->loginBehaviour = FAKE_HANG;

            CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            ASSERT_FALSE( f.manager.Login( "refusing", AUTH::CAuthOptions(), future ) );
            ASSERT_TRUE( future.IsReady() && future.HasNoFuture() );
            ASSERT_FALSE( f.manager.IsLoginPending( "refusing" ) );
            ASSERT_TRUE( 1 == f.recorder.Count( CAuthRecorder::KIND_FAILED ) && AUTH::AUTHFAIL_NOT_AVAILABLE == f.recorder.Last( CAuthRecorder::KIND_FAILED )->failure.reason );

            ASSERT_FALSE( f.manager.Login( "away", AUTH::CAuthOptions(), future ) );
            ASSERT_TRUE( 0 == away->beginCount && future.IsReady() && future.HasNoFuture() );
            ASSERT_FALSE( f.manager.Login( "nobody", AUTH::CAuthOptions(), future ) );
            ASSERT_TRUE( 3 == f.recorder.Count( CAuthRecorder::KIND_FAILED ) );

            // one login at a time per provider
            ASSERT_TRUE( f.manager.Login( "hanging", AUTH::CAuthOptions(), future ) );
            CORE::CFutureResult second( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            ASSERT_FALSE( f.manager.Login( "hanging", AUTH::CAuthOptions(), second ) );
            ASSERT_TRUE( second.IsReady() && 1 == hanging->beginCount );
            ASSERT_TRUE( AUTH::AUTHFAIL_IN_PROGRESS == f.recorder.Last( CAuthRecorder::KIND_FAILED )->failure.reason );
            ASSERT_TRUE( f.manager.CancelLogin( "hanging" ) && Await( future, 3000 ) );
            ASSERT_FALSE( f.manager.CancelLogin( "hanging" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A login that completes on a pool thread is handled on the thread of the pulse generator" )
        GUCEF_TESTFW_SKIP_IF( !Prerequisites(), "The json or ed25519 codec is not available" );
        try
        {
            CFixture f( "mgr_slow" );
            CFakeAuthProvider* alpha = f.AddFake( "alpha" );
            alpha->loginBehaviour = FAKE_SLOW;
            alpha->delayMs = 150;
            CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            ASSERT_TRUE( f.manager.Login( "alpha", AUTH::CAuthOptions(), future ) );
            PumpFor( 40 );
            ASSERT_TRUE( !future.IsReady() && f.manager.IsLoginPending( "alpha" ) && 0 == f.recorder.Count( CAuthRecorder::KIND_SUCCEEDED ) );
            ASSERT_TRUE( AUTH::AUTHLOGINSTATE_GUEST == f.manager.GetLoginState() );
            ASSERT_TRUE( Await( future, 5000 ) );
            PumpFor( 20 );
            ASSERT_TRUE( 1 == f.recorder.Count( CAuthRecorder::KIND_SUCCEEDED ) && mainThreadId == f.recorder.Last( CAuthRecorder::KIND_SUCCEEDED )->threadId );
            ASSERT_TRUE( 1 == f.recorder.Count( CAuthRecorder::KIND_LINKED ) && AUTH::AUTHLOGINSTATE_LINKED == f.manager.GetLoginState() );
            AUTH::CAuthResult result;
            ASSERT_TRUE( AUTH::CAuthResult::TryGetFromFuture( future, result ) && "fake:player-1" == result.accountRef.ToString() );

            // and a failure from a pool thread
            alpha->loginBehaviour = FAKE_SLOW_FAIL;
            alpha->failReason = AUTH::AUTHFAIL_NETWORK;
            ASSERT_TRUE( f.manager.Login( "alpha", AUTH::CAuthOptions(), future ) && Await( future, 5000 ) );
            PumpFor( 20 );
            ASSERT_TRUE( 1 == f.recorder.Count( CAuthRecorder::KIND_FAILED ) && AUTH::AUTHFAIL_NETWORK == f.recorder.Last( CAuthRecorder::KIND_FAILED )->failure.reason );
            ASSERT_TRUE( mainThreadId == f.recorder.Last( CAuthRecorder::KIND_FAILED )->threadId );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A login can be cancelled, runs out of time, or is ended by the manager when the provider does not react" )
        GUCEF_TESTFW_SKIP_IF( !Prerequisites(), "The json or ed25519 codec is not available" );
        try
        {
            CFixture f( "mgr_cancel" );
            CFakeAuthProvider* hanging = f.AddFake( "hanging" );
            hanging->loginBehaviour = FAKE_HANG;
            CFakeAuthProvider* deaf = f.AddFake( "deaf" );
            deaf->loginBehaviour = FAKE_IGNORE_CANCEL;

            // cancelled by the requester
            CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            ASSERT_TRUE( f.manager.Login( "hanging", AUTH::CAuthOptions(), future ) );
            PumpFor( 30 );
            ASSERT_TRUE( !future.IsReady() && f.manager.CancelLogin( "hanging" ) );
            ASSERT_TRUE( Await( future, 3000 ) );
            PumpFor( 20 );
            ASSERT_TRUE( CORE::TTaskStatus::TASKSTATUS_CANCELLED == StatusOf( future ) );
            ASSERT_TRUE( 1 == f.recorder.Count( CAuthRecorder::KIND_FAILED ) && AUTH::AUTHFAIL_CANCELLED == f.recorder.Last( CAuthRecorder::KIND_FAILED )->failure.reason );
            ASSERT_FALSE( f.manager.IsLoginPending( "hanging" ) );
            ASSERT_TRUE( AUTH::AUTHLOGINSTATE_GUEST == f.manager.GetLoginState() );

            // out of time: the manager asks the provider to stop and the outcome is a timeout
            AUTH::CAuthSettings settings = f.manager.GetSettings();
            settings.loginDeadlineSlackInMs = 0;
            f.manager.SetSettings( settings );
            AUTH::CAuthOptions shortOptions;
            shortOptions.timeoutInMs = 300;
            ASSERT_TRUE( f.manager.Login( "hanging", shortOptions, future ) );
            ASSERT_TRUE( Await( future, 4000 ) );
            PumpFor( 20 );
            ASSERT_TRUE( 2 == f.recorder.Count( CAuthRecorder::KIND_FAILED ) && AUTH::AUTHFAIL_TIMEOUT == f.recorder.Last( CAuthRecorder::KIND_FAILED )->failure.reason );
            ASSERT_FALSE( f.manager.IsLoginPending( "hanging" ) );

            // a provider that does not look at the cancellation: the manager ends the login itself after a grace time
            ASSERT_TRUE( f.manager.Login( "deaf", AUTH::CAuthOptions(), future ) );
            PumpFor( 30 );
            ASSERT_TRUE( f.manager.CancelLogin( "deaf" ) );
            PumpFor( 200 );
            ASSERT_TRUE( !future.IsReady() );
            ASSERT_TRUE( Await( future, 5000 ) );
            PumpFor( 20 );
            ASSERT_TRUE( CORE::TTaskStatus::TASKSTATUS_CANCELLED == StatusOf( future ) );
            ASSERT_TRUE( 3 == f.recorder.Count( CAuthRecorder::KIND_FAILED ) && AUTH::AUTHFAIL_CANCELLED == f.recorder.Last( CAuthRecorder::KIND_FAILED )->failure.reason );
            ASSERT_FALSE( f.manager.IsLoginPending( "deaf" ) );
            ASSERT_TRUE( 0 == f.LinkCount() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A provider that ends its future without an event is still handled" )
        GUCEF_TESTFW_SKIP_IF( !Prerequisites(), "The json or ed25519 codec is not available" );
        try
        {
            CFixture f( "mgr_noevent" );
            CFakeAuthProvider* quiet = f.AddFake( "quiet" );
            quiet->loginBehaviour = FAKE_NO_EVENT;
            CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            ASSERT_TRUE( f.manager.Login( "quiet", AUTH::CAuthOptions(), future ) );
            PumpFor( 50 );
            ASSERT_TRUE( future.IsReady() == false && f.manager.IsLoginPending( "quiet" ) );
            ASSERT_TRUE( Await( future, 5000 ) );
            PumpFor( 20 );
            ASSERT_TRUE( 1 == f.recorder.Count( CAuthRecorder::KIND_SUCCEEDED ) && AUTH::AUTHLOGINSTATE_LINKED == f.manager.GetLoginState() );
            AUTH::CSecretString stored;
            ASSERT_TRUE( f.Store()->Load( "quiet", "fake:player-1", stored ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Two providers log in at the same time and both accounts are linked to the one profile" )
        GUCEF_TESTFW_SKIP_IF( !Prerequisites(), "The json or ed25519 codec is not available" );
        try
        {
            CFixture f( "mgr_concurrent" );
            CFakeAuthProvider* ann = f.AddFake( "ann-provider", "ann", "fakea" );
            CFakeAuthProvider* bob = f.AddFake( "bob-provider", "bob", "fakeb" );
            ann->loginBehaviour = FAKE_SLOW;
            ann->delayMs = 300;
            bob->loginBehaviour = FAKE_COMPLETE;

            CORE::CFutureResult annFuture( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            CORE::CFutureResult bobFuture( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            ASSERT_TRUE( f.manager.Login( "ann-provider", AUTH::CAuthOptions(), annFuture ) );
            ASSERT_TRUE( f.manager.Login( "bob-provider", AUTH::CAuthOptions(), bobFuture ) );
            ASSERT_TRUE( f.manager.IsLoginPending( "ann-provider" ) && f.manager.IsLoginPending( "bob-provider" ) );

            ASSERT_TRUE( Await( bobFuture, 5000 ) );
            PumpFor( 20 );
            ASSERT_TRUE( f.manager.IsLoginPending( "ann-provider" ) && 1 == f.LinkCount() && AUTH::AUTHLOGINSTATE_LINKED == f.manager.GetLoginState() );
            ASSERT_TRUE( Await( annFuture, 5000 ) );
            PumpFor( 20 );

            ASSERT_TRUE( 2 == f.LinkCount() );
            AUTH::TLinkRecordVector links;
            REQUIRE_TRUE( f.manager.GetLinkedAccounts( links ) && 2 == links.size() );
            ASSERT_TRUE( links[ 0 ].profileKeyId == links[ 1 ].profileKeyId && links[ 0 ].profileKeyId == f.signer.GetKeyId() );
            ASSERT_TRUE( links[ 0 ].accountRef != links[ 1 ].accountRef );
            ASSERT_TRUE( "fakeb:bob" == links[ 0 ].accountRef.ToString() && "fakea:ann" == links[ 1 ].accountRef.ToString() );
            ASSERT_TRUE( links[ 0 ].VerifySignature() && links[ 1 ].VerifySignature() );

            // the state changed once, to linked
            ASSERT_TRUE( 1 == f.recorder.Count( CAuthRecorder::KIND_STATE ) && 2 == f.recorder.Count( CAuthRecorder::KIND_LINKED ) && 2 == f.recorder.Count( CAuthRecorder::KIND_SUCCEEDED ) );
            GUCEF::vector< AUTH::CAuthResult > sessions;
            f.manager.GetSessionAccounts( sessions );
            ASSERT_TRUE( 2 == sessions.size() );
            // the last one to log in is the first to try at the next start
            ASSERT_TRUE( 2 == f.manager.GetSettings().lastUsedProviders.size() && "ann-provider" == f.manager.GetSettings().lastUsedProviders[ 0 ] && "bob-provider" == f.manager.GetSettings().lastUsedProviders[ 1 ] );
            AUTH::CSecretString stored;
            ASSERT_TRUE( f.Store()->Load( "ann-provider", "fakea:ann", stored ) && f.Store()->Load( "bob-provider", "fakeb:bob", stored ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Logging out keeps the link, unlinking removes it with the login" )
        GUCEF_TESTFW_SKIP_IF( !Prerequisites(), "The json or ed25519 codec is not available" );
        try
        {
            CFixture f( "mgr_unlink" );
            CFakeAuthProvider* a = f.AddFake( "pa", "a1", "fake" );
            CFakeAuthProvider* b = f.AddFake( "pb", "b1", "fake" );
            CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            ASSERT_TRUE( f.manager.Login( "pa", AUTH::CAuthOptions(), future ) && Await( future, 3000 ) );
            ASSERT_TRUE( f.manager.Login( "pb", AUTH::CAuthOptions(), future ) && Await( future, 3000 ) );
            PumpFor( 20 );
            ASSERT_TRUE( 2 == f.LinkCount() && 1 == f.recorder.Count( CAuthRecorder::KIND_STATE ) );

            AUTH::CAccountRef accountA( "fake", "a1" );
            AUTH::CAccountRef accountB( "fake", "b1" );
            AUTH::CAuthResult sessionA;
            ASSERT_TRUE( f.manager.GetSession( accountA, sessionA ) );

            // logout: tokens are gone, the provider is told, the link stays
            ASSERT_TRUE( f.manager.Logout( accountA ) );
            ASSERT_TRUE( 1 == a->logoutCount && accountA == a->lastLogoutAccount && sessionA.refreshToken.Reveal() == a->lastLogoutToken );
            AUTH::CAuthResult gone;
            ASSERT_FALSE( f.manager.GetSession( accountA, gone ) );
            AUTH::CSecretString stored;
            ASSERT_FALSE( f.Store()->Load( "pa", "fake:a1", stored ) );
            ASSERT_TRUE( 2 == f.LinkCount() && AUTH::AUTHLOGINSTATE_LINKED == f.manager.GetLoginState() );
            ASSERT_TRUE( 1 == f.recorder.Count( CAuthRecorder::KIND_LOGGEDOUT ) && "pa" == f.recorder.Last( CAuthRecorder::KIND_LOGGEDOUT )->account.providerName );
            ASSERT_FALSE( f.manager.Logout( accountA ) );
            ASSERT_TRUE( 1 == a->logoutCount );
            ASSERT_FALSE( f.manager.Logout( AUTH::CAccountRef() ) );

            // unlink: the link record, the stored login and the session
            ASSERT_TRUE( f.manager.Unlink( accountB ) );
            CORE::CString profile;
            ASSERT_FALSE( f.linkStore.Resolve( accountB, profile ) );
            ASSERT_TRUE( 1 == b->logoutCount );
            ASSERT_FALSE( f.Store()->Load( "pb", "fake:b1", stored ) );
            ASSERT_FALSE( f.manager.GetSession( accountB, gone ) );
            ASSERT_TRUE( 1 == f.recorder.Count( CAuthRecorder::KIND_UNLINKED ) && accountB == f.recorder.Last( CAuthRecorder::KIND_UNLINKED )->account.accountRef );
            ASSERT_TRUE( 1 == f.LinkCount() && AUTH::AUTHLOGINSTATE_LINKED == f.manager.GetLoginState() && 1 == f.recorder.Count( CAuthRecorder::KIND_STATE ) );

            // the last one: back to a guest
            ASSERT_TRUE( f.manager.Unlink( accountA ) );
            ASSERT_TRUE( 0 == f.LinkCount() && AUTH::AUTHLOGINSTATE_GUEST == f.manager.GetLoginState() );
            ASSERT_TRUE( 2 == f.recorder.Count( CAuthRecorder::KIND_STATE ) && AUTH::AUTHLOGINSTATE_GUEST == f.recorder.Last( CAuthRecorder::KIND_STATE )->state.state );
            ASSERT_TRUE( 2 == f.recorder.Count( CAuthRecorder::KIND_UNLINKED ) );

            // an account that is not linked: nothing to do, no event
            ASSERT_TRUE( f.manager.Unlink( AUTH::CAccountRef( "fake", "never-linked" ) ) );
            ASSERT_TRUE( 2 == f.recorder.Count( CAuthRecorder::KIND_UNLINKED ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The startup policy tries silent providers, then stored logins, in the order of last use" )
        GUCEF_TESTFW_SKIP_IF( !Prerequisites(), "The json or ed25519 codec is not available" );
        try
        {
            CFixture f( "mgr_auto_order" );
            CFakeAuthProvider* p1 = f.AddFake( "p1", "one" );
            CFakeAuthProvider* p2 = f.AddFake( "p2", "two" );
            CFakeAuthProvider* p3 = f.AddFake( "p3", "three" );
            CFakeAuthProvider* p4 = f.AddFake( "p4", "four" );
            p1->capabilities |= AUTH::AUTHCAP_SILENT;
            p2->capabilities |= AUTH::AUTHCAP_SILENT;
            p2->silentBehaviour = FAKE_FAIL;
            p2->failReason = AUTH::AUTHFAIL_NOT_AVAILABLE;

            ASSERT_TRUE( f.Store()->Store( "p1", "fake:one", AUTH::CSecretString( "tok1" ) ) );
            ASSERT_TRUE( f.Store()->Store( "p3", "fake:three", AUTH::CSecretString( "tok3" ) ) );
            ASSERT_TRUE( f.Store()->Store( "p4", "fake:four", AUTH::CSecretString( "tok4" ) ) );
            AUTH::CAuthSettings settings = f.manager.GetSettings();
            settings.lastUsedProviders.push_back( "p3" );
            settings.lastUsedProviders.push_back( "p2" );
            settings.lastUsedProviders.push_back( "p1" );
            f.manager.SetSettings( settings );

            CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            ASSERT_TRUE( f.manager.AutoLogin( future ) );
            ASSERT_TRUE( Await( future, 5000 ) );
            PumpFor( 20 );
            ASSERT_FALSE( f.manager.IsAutoLoginRunning() );

            // silent ones first in the order of last use (p2, p1); the stored login of p3 after; p1 is not refreshed, the platform had it; p4 was not used last
            ASSERT_TRUE( 3 == f.callLog.size() );
            if ( 3 == f.callLog.size() )
                ASSERT_TRUE( "silent:p2" == f.callLog[ 0 ] && "silent:p1" == f.callLog[ 1 ] && "refresh:p3" == f.callLog[ 2 ] );
            ASSERT_TRUE( "tok3" == p3->lastRefreshTokenSeen && 0 == p4->refreshCount && 0 == p1->refreshCount );
            ASSERT_TRUE( 0 == p1->beginCount && 0 == p2->beginCount && 0 == p3->beginCount );

            ASSERT_TRUE( 2 == f.recorder.Count( CAuthRecorder::KIND_SUCCEEDED ) && 1 == f.recorder.Count( CAuthRecorder::KIND_FAILED ) );
            ASSERT_TRUE( AUTH::AUTHFAIL_NOT_AVAILABLE == f.recorder.Last( CAuthRecorder::KIND_FAILED )->failure.reason );
            ASSERT_TRUE( 1 == f.recorder.Count( CAuthRecorder::KIND_AUTOFINISHED ) );
            ASSERT_TRUE( AUTH::AUTHLOGINSTATE_LINKED == f.recorder.Last( CAuthRecorder::KIND_AUTOFINISHED )->state.state && 2 == f.recorder.Last( CAuthRecorder::KIND_AUTOFINISHED )->state.linkedAccountCount );
            GUCEF::vector< AUTH::CAuthResult > sessions;
            f.manager.GetSessionAccounts( sessions );
            ASSERT_TRUE( 2 == sessions.size() && 2 == f.LinkCount() );
            // the last one that logged in is the first next time
            REQUIRE_TRUE( f.manager.GetSettings().lastUsedProviders.size() >= 2 );
            ASSERT_TRUE( "p3" == f.manager.GetSettings().lastUsedProviders[ 0 ] && "p1" == f.manager.GetSettings().lastUsedProviders[ 1 ] );
            // and the rotated refresh token of the refresh replaced the old one
            AUTH::CSecretString rotated;
            ASSERT_TRUE( f.Store()->Load( "p3", "fake:three", rotated ) && "tok3" != rotated.Reveal() && rotated.Reveal().HasSubstr( CFakeAuthProvider::SecretRefreshTokenPrefix ) >= 0 );
            // AutoLogin while nothing runs, again: it runs again
            f.callLog.clear();
            ASSERT_TRUE( f.manager.AutoLogin( future ) && Await( future, 5000 ) );
            ASSERT_TRUE( 3 == f.callLog.size() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A stored login the provider refuses is dropped, one lost to the network is kept" )
        GUCEF_TESTFW_SKIP_IF( !Prerequisites(), "The json or ed25519 codec is not available" );
        try
        {
            CFixture f( "mgr_auto_grant" );
            CFakeAuthProvider* revoked = f.AddFake( "revoked", "r1" );
            CFakeAuthProvider* offline = f.AddFake( "offline", "o1" );
            revoked->refreshBehaviour = FAKE_FAIL;
            revoked->failReason = AUTH::AUTHFAIL_INVALID_GRANT;
            offline->refreshBehaviour = FAKE_FAIL;
            offline->failReason = AUTH::AUTHFAIL_NETWORK;
            f.Store()->Store( "revoked", "fake:r1", AUTH::CSecretString( "old" ) );
            f.Store()->Store( "offline", "fake:o1", AUTH::CSecretString( "old" ) );
            AUTH::CAuthSettings settings = f.manager.GetSettings();
            settings.lastUsedProviders.push_back( "revoked" );
            settings.lastUsedProviders.push_back( "offline" );
            f.manager.SetSettings( settings );

            CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            ASSERT_TRUE( f.manager.AutoLogin( future ) && Await( future, 5000 ) );
            PumpFor( 20 );
            ASSERT_TRUE( 2 == f.recorder.Count( CAuthRecorder::KIND_FAILED ) && 0 == f.recorder.Count( CAuthRecorder::KIND_SUCCEEDED ) );
            AUTH::CSecretString stored;
            ASSERT_FALSE( f.Store()->Load( "revoked", "fake:r1", stored ) );
            ASSERT_TRUE( f.Store()->Load( "offline", "fake:o1", stored ) );
            // nobody logged in: still a guest
            ASSERT_TRUE( AUTH::AUTHLOGINSTATE_GUEST == f.manager.GetLoginState() && AUTH::AUTHLOGINSTATE_GUEST == f.recorder.Last( CAuthRecorder::KIND_AUTOFINISHED )->state.state );

            ASSERT_TRUE( f.manager.AutoLogin( future ) && Await( future, 5000 ) );
            ASSERT_TRUE( 1 == revoked->refreshCount && 2 == offline->refreshCount );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The startup policy can be off, can time a step out and can be cancelled" )
        GUCEF_TESTFW_SKIP_IF( !Prerequisites(), "The json or ed25519 codec is not available" );
        try
        {
            // off
            {
                CFixture f( "mgr_auto_off" );
                CFakeAuthProvider* p = f.AddFake( "p", "one" );
                p->capabilities |= AUTH::AUTHCAP_SILENT;
                AUTH::CAuthSettings settings = f.manager.GetSettings();
                settings.autoLogin = false;
                f.manager.SetSettings( settings );
                CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
                ASSERT_TRUE( f.manager.AutoLogin( future ) && future.IsReady() && !f.manager.IsAutoLoginRunning() );
                PumpFor( 20 );
                ASSERT_TRUE( 0 == p->silentCount && 1 == f.recorder.Count( CAuthRecorder::KIND_AUTOFINISHED ) && 0 == f.recorder.Count( CAuthRecorder::KIND_SUCCEEDED ) );
            }

            // nothing to try: a guest, and it says so
            {
                CFixture f( "mgr_auto_nothing" );
                f.AddFake( "p", "one" );
                CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
                ASSERT_TRUE( f.manager.AutoLogin( future ) && Await( future, 3000 ) );
                PumpFor( 20 );
                ASSERT_TRUE( 1 == f.recorder.Count( CAuthRecorder::KIND_AUTOFINISHED ) && AUTH::AUTHLOGINSTATE_GUEST == f.recorder.Last( CAuthRecorder::KIND_AUTOFINISHED )->state.state );
                ASSERT_TRUE( f.callLog.empty() );
            }

            // a step that hangs is given up after its time and the next one is tried
            {
                CFixture f( "mgr_auto_timeout" );
                CFakeAuthProvider* slow = f.AddFake( "slow", "s1" );
                CFakeAuthProvider* quick = f.AddFake( "quick", "q1" );
                slow->refreshBehaviour = FAKE_HANG;
                f.Store()->Store( "slow", "fake:s1", AUTH::CSecretString( "tok" ) );
                f.Store()->Store( "quick", "fake:q1", AUTH::CSecretString( "tok" ) );
                AUTH::CAuthSettings settings = f.manager.GetSettings();
                settings.autoLoginStepTimeoutInMs = 300;
                settings.lastUsedProviders.push_back( "slow" );
                settings.lastUsedProviders.push_back( "quick" );
                f.manager.SetSettings( settings );
                CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
                ASSERT_TRUE( f.manager.AutoLogin( future ) );
                // running a second time returns the same sequence
                CORE::CFutureResult again( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
                ASSERT_TRUE( f.manager.AutoLogin( again ) && f.manager.IsAutoLoginRunning() && again.GetTaskId() == future.GetTaskId() );
                ASSERT_TRUE( Await( future, 6000 ) );
                PumpFor( 20 );
                ASSERT_TRUE( 1 == slow->refreshCount && 1 == quick->refreshCount );
                ASSERT_TRUE( 1 == f.recorder.Count( CAuthRecorder::KIND_FAILED ) && AUTH::AUTHFAIL_TIMEOUT == f.recorder.Last( CAuthRecorder::KIND_FAILED )->failure.reason );
                ASSERT_TRUE( 1 == f.recorder.Count( CAuthRecorder::KIND_SUCCEEDED ) && 1 == f.recorder.Count( CAuthRecorder::KIND_AUTOFINISHED ) );
                // the hanging one lost no stored login: it only took too long
                AUTH::CSecretString stored;
                ASSERT_TRUE( f.Store()->Load( "slow", "fake:s1", stored ) );
            }

            // cancelled in the middle
            {
                CFixture f( "mgr_auto_cancel" );
                CFakeAuthProvider* first = f.AddFake( "first", "f1" );
                CFakeAuthProvider* second = f.AddFake( "second", "s1" );
                first->refreshBehaviour = FAKE_HANG;
                f.Store()->Store( "first", "fake:f1", AUTH::CSecretString( "tok" ) );
                f.Store()->Store( "second", "fake:s1", AUTH::CSecretString( "tok" ) );
                AUTH::CAuthSettings settings = f.manager.GetSettings();
                settings.lastUsedProviders.push_back( "first" );
                settings.lastUsedProviders.push_back( "second" );
                f.manager.SetSettings( settings );
                CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
                ASSERT_TRUE( f.manager.AutoLogin( future ) );
                PumpFor( 40 );
                ASSERT_TRUE( 1 == first->refreshCount && f.manager.IsAutoLoginRunning() );
                f.manager.CancelAutoLogin();
                ASSERT_TRUE( Await( future, 5000 ) );
                PumpFor( 20 );
                ASSERT_TRUE( !f.manager.IsAutoLoginRunning() && 0 == second->refreshCount && 1 == f.recorder.Count( CAuthRecorder::KIND_AUTOFINISHED ) );
            }

            // a provider the user is logging in with is left alone
            {
                CFixture f( "mgr_auto_busy" );
                CFakeAuthProvider* busy = f.AddFake( "busy", "b1" );
                busy->capabilities |= AUTH::AUTHCAP_SILENT;
                busy->loginBehaviour = FAKE_HANG;
                CORE::CFutureResult loginFuture( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
                ASSERT_TRUE( f.manager.Login( "busy", AUTH::CAuthOptions(), loginFuture ) );
                CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
                ASSERT_TRUE( f.manager.AutoLogin( future ) && Await( future, 3000 ) );
                ASSERT_TRUE( 0 == busy->silentCount && f.manager.IsLoginPending( "busy" ) );
                f.manager.CancelLogin( "busy" );
                ASSERT_TRUE( Await( loginFuture, 3000 ) );
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The links, the stored logins and the settings survive a restart" )
        GUCEF_TESTFW_SKIP_IF( !Prerequisites(), "The json or ed25519 codec is not available" );
        GUCEF_TESTFW_SKIP_IF( !AUTH::CFileCredentialStore::IsAvailable(), "The xchacha20-poly1305 codec is not available" );
        try
        {
            CORE::CDataNode savedSettings( "Auth", GUCEF_DATATYPE_OBJECT );
            CTestSigner profile( "profile" );
            CORE::CString dir = MakeTestDirectory( "mgr_restart" );
            CORE::CString credDir = CORE::CombinePath( dir, "cred" );
            CORE::CString linkPath = CORE::CombinePath( dir, "links.json" );
            CORE::CString firstToken;
            {
                AUTH::CFileCredentialStore credentials( credDir );
                AUTH::CLocalIdentityLinkStore links( linkPath );
                links.SetSigner( &profile );
                AUTH::CAuthManager manager;
                manager.SetCredentialStore( &credentials );
                manager.SetLinkStore( &links );
                manager.SetSigner( &profile );
                TFakeProviderFactory::TProductPtr product = g_fakeFactory.Create();
                CFakeAuthProvider* alpha = static_cast< CFakeAuthProvider* >( product.GetPointerAlways() );
                alpha->name = "alpha";
                ASSERT_TRUE( manager.AddProvider( product ) );
                CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
                ASSERT_TRUE( manager.Login( "alpha", AUTH::CAuthOptions(), future ) && Await( future, 3000 ) );
                PumpFor( 20 );
                AUTH::CAuthResult session;
                ASSERT_TRUE( manager.GetSession( AUTH::CAccountRef( "fake", "player-1" ), session ) );
                firstToken = session.refreshToken.Reveal();
                ASSERT_TRUE( manager.SaveConfig( savedSettings ) );
            }

            // another run: new objects over the same files
            AUTH::CFileCredentialStore credentials( credDir );
            AUTH::CLocalIdentityLinkStore links( linkPath );
            links.SetSigner( &profile );
            AUTH::CAuthManager manager;
            manager.SetCredentialStore( &credentials );
            manager.SetLinkStore( &links );
            manager.SetSigner( &profile );
            AUTH::CAuthProviderFactory noFactories;
            manager.SetProviderFactory( &noFactories );
            CAuthRecorder recorder;
            recorder.SubscribeTo( &manager );
            TFakeProviderFactory::TProductPtr product = g_fakeFactory.Create();
            CFakeAuthProvider* alpha = static_cast< CFakeAuthProvider* >( product.GetPointerAlways() );
            alpha->name = "alpha";
            ASSERT_TRUE( manager.AddProvider( product ) );
            ASSERT_TRUE( manager.LoadConfig( savedSettings ) );

            // linked before anyone logged in, and the order of last use is back
            ASSERT_TRUE( AUTH::AUTHLOGINSTATE_LINKED == manager.GetLoginState() );
            AUTH::TLinkRecordVector linked;
            ASSERT_TRUE( manager.GetLinkedAccounts( linked ) && 1 == linked.size() && "fake:player-1" == linked[ 0 ].accountRef.ToString() && linked[ 0 ].VerifySignature() );
            ASSERT_TRUE( 1 == manager.GetSettings().lastUsedProviders.size() && "alpha" == manager.GetSettings().lastUsedProviders[ 0 ] );
            GUCEF::vector< AUTH::CAuthResult > sessions;
            manager.GetSessionAccounts( sessions );
            ASSERT_TRUE( sessions.empty() );

            // the startup policy brings the login back from the stored refresh token, without the user
            CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            ASSERT_TRUE( manager.AutoLogin( future ) && Await( future, 5000 ) );
            PumpFor( 20 );
            ASSERT_TRUE( 1 == alpha->refreshCount && firstToken == alpha->lastRefreshTokenSeen && 0 == alpha->beginCount );
            manager.GetSessionAccounts( sessions );
            ASSERT_TRUE( 1 == sessions.size() && "fake:player-1" == sessions[ 0 ].accountRef.ToString() );
            ASSERT_TRUE( 1 == recorder.Count( CAuthRecorder::KIND_SUCCEEDED ) && 1 == recorder.Count( CAuthRecorder::KIND_AUTOFINISHED ) );
            // the account was linked already: not linked again, the state did not change
            ASSERT_TRUE( 0 == recorder.Count( CAuthRecorder::KIND_LINKED ) && 0 == recorder.Count( CAuthRecorder::KIND_STATE ) );
            // and the token is the new one
            AUTH::CSecretString stored;
            ASSERT_TRUE( credentials.Load( "alpha", "fake:player-1", stored ) && firstToken != stored.Reveal() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: An account of another profile is not linked, and a merge moves the accounts" )
        GUCEF_TESTFW_SKIP_IF( !Prerequisites(), "The json or ed25519 codec is not available" );
        try
        {
            CFixture f( "mgr_conflict" );
            CTestSigner otherProfile( "other" );
            AUTH::CMemoryCredentialStore otherCredentials;
            AUTH::CAuthManager other;
            other.SetCredentialStore( &otherCredentials );
            other.SetLinkStore( &f.linkStore );
            other.SetSigner( &otherProfile );
            CAuthRecorder otherRecorder;
            otherRecorder.SubscribeTo( &other );

            CFakeAuthProvider* mine = f.AddFake( "alpha" );
            TFakeProviderFactory::TProductPtr product = g_fakeFactory.Create();
            CFakeAuthProvider* theirs = static_cast< CFakeAuthProvider* >( product.GetPointerAlways() );
            theirs->name = "alpha";
            ASSERT_TRUE( other.AddProvider( product ) );

            CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            ASSERT_TRUE( f.manager.Login( "alpha", AUTH::CAuthOptions(), future ) && Await( future, 3000 ) );
            PumpFor( 20 );
            ASSERT_TRUE( 1 == f.LinkCount() && mine->beginCount == 1 );

            // the same account at another profile: the login worked at the provider and is refused here
            ASSERT_TRUE( other.Login( "alpha", AUTH::CAuthOptions(), future ) );
            ASSERT_TRUE( Await( future, 3000 ) );
            PumpFor( 20 );
            ASSERT_TRUE( future.HasNoFuture() && 1 == otherRecorder.Count( CAuthRecorder::KIND_FAILED ) );
            ASSERT_TRUE( AUTH::AUTHFAIL_ACCOUNT_LINKED_ELSEWHERE == otherRecorder.Last( CAuthRecorder::KIND_FAILED )->failure.reason );
            GUCEF::vector< AUTH::CAuthResult > sessions;
            other.GetSessionAccounts( sessions );
            AUTH::CSecretString stored;
            ASSERT_TRUE( sessions.empty() && !otherCredentials.Load( "alpha", "fake:player-1", stored ) );
            ASSERT_TRUE( AUTH::AUTHLOGINSTATE_GUEST == other.GetLoginState() );
            CORE::CString profile;
            ASSERT_TRUE( f.linkStore.Resolve( AUTH::CAccountRef( "fake", "player-1" ), profile ) && f.signer.GetKeyId() == profile );
            ASSERT_TRUE( 0 == otherRecorder.Count( CAuthRecorder::KIND_SUCCEEDED ) && 0 == otherRecorder.Count( CAuthRecorder::KIND_LINKED ) );

            // merging the accounts of the first profile into the second one
            UInt32 merged = 0;
            ASSERT_TRUE( AUTH::LINKRESULT_OK == other.MergeProfile( f.signer.GetKeyId(), &merged ) && 1 == merged );
            ASSERT_TRUE( f.linkStore.Resolve( AUTH::CAccountRef( "fake", "player-1" ), profile ) && otherProfile.GetKeyId() == profile );
            ASSERT_TRUE( AUTH::AUTHLOGINSTATE_LINKED == other.GetLoginState() && 1 == otherRecorder.Count( CAuthRecorder::KIND_STATE ) );
            ASSERT_TRUE( AUTH::AUTHLOGINSTATE_GUEST == f.manager.GetLoginState() );
            ASSERT_TRUE( AUTH::LINKRESULT_INVALID_RECORD == f.manager.MergeProfile( f.signer.GetKeyId() ) );

            // now the second profile logs in with it
            ASSERT_TRUE( other.Login( "alpha", AUTH::CAuthOptions(), future ) && Await( future, 3000 ) );
            PumpFor( 20 );
            ASSERT_TRUE( 1 == otherRecorder.Count( CAuthRecorder::KIND_SUCCEEDED ) );
            other.GetSessionAccounts( sessions );
            ASSERT_TRUE( 1 == sessions.size() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Without a signer or when asked not to, a login is not linked; Link() always links" )
        GUCEF_TESTFW_SKIP_IF( !Prerequisites(), "The json or ed25519 codec is not available" );
        try
        {
            {
                CFixture f( "mgr_nosigner", false );
                CFakeAuthProvider* alpha = f.AddFake( "alpha" );
                ASSERT_TRUE( f.manager.GetProfileKeyId().IsNULLOrEmpty() );
                CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
                ASSERT_TRUE( f.manager.Login( "alpha", AUTH::CAuthOptions(), future ) && Await( future, 3000 ) );
                PumpFor( 20 );
                ASSERT_TRUE( 1 == f.recorder.Count( CAuthRecorder::KIND_SUCCEEDED ) && 0 == f.recorder.Count( CAuthRecorder::KIND_LINKED ) );
                ASSERT_TRUE( AUTH::AUTHLOGINSTATE_GUEST == f.manager.GetLoginState() );
                AUTH::TLinkRecordVector links;
                ASSERT_FALSE( f.manager.GetLinkedAccounts( links ) );
                ASSERT_TRUE( 0 == f.signer.GetSignCount() );
                AUTH::CAuthResult session;
                ASSERT_TRUE( f.manager.GetSession( AUTH::CAccountRef( "fake", "player-1" ), session ) );
                AUTH::CSecretString stored;
                ASSERT_TRUE( f.Store()->Load( "alpha", "fake:player-1", stored ) );
                ASSERT_FALSE( f.manager.Unlink( AUTH::CAccountRef( "fake", "player-1" ) ) );
                ASSERT_TRUE( 1 == alpha->beginCount );
            }

            {
                CFixture f( "mgr_nolink" );
                f.AddFake( "alpha" );
                AUTH::CAuthOptions options;
                options.linkToProfile = false;
                CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
                ASSERT_TRUE( f.manager.Login( "alpha", options, future ) && Await( future, 3000 ) );
                PumpFor( 20 );
                ASSERT_TRUE( 1 == f.recorder.Count( CAuthRecorder::KIND_SUCCEEDED ) && 0 == f.LinkCount() && AUTH::AUTHLOGINSTATE_GUEST == f.manager.GetLoginState() );
                ASSERT_TRUE( 0 == f.signer.GetSignCount() );

                ASSERT_TRUE( f.manager.Link( "alpha", options, future ) && Await( future, 3000 ) );
                PumpFor( 20 );
                ASSERT_TRUE( 1 == f.LinkCount() && AUTH::AUTHLOGINSTATE_LINKED == f.manager.GetLoginState() && 1 == f.recorder.Count( CAuthRecorder::KIND_LINKED ) );
                ASSERT_TRUE( 1 == f.signer.GetSignCount() );

                // logging in again with the account that is linked does not make another record
                ASSERT_TRUE( f.manager.Login( "alpha", AUTH::CAuthOptions(), future ) && Await( future, 3000 ) );
                PumpFor( 20 );
                ASSERT_TRUE( 1 == f.LinkCount() && 1 == f.signer.GetSignCount() && 1 == f.recorder.Count( CAuthRecorder::KIND_LINKED ) );
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Events of logins the manager did not start are ignored, a removed provider ends its pending login" )
        GUCEF_TESTFW_SKIP_IF( !Prerequisites(), "The json or ed25519 codec is not available" );
        try
        {
            CFixture f( "mgr_stray" );
            CFakeAuthProvider* alpha = f.AddFake( "alpha" );
            CORE::CFutureResult direct( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            ASSERT_TRUE( alpha->BeginLogin( AUTH::CAuthOptions(), direct ) );
            PumpFor( 60 );
            ASSERT_TRUE( 0 == f.recorder.Count( CAuthRecorder::KIND_SUCCEEDED ) && AUTH::AUTHLOGINSTATE_GUEST == f.manager.GetLoginState() );
            GUCEF::vector< AUTH::CAuthResult > sessions;
            f.manager.GetSessionAccounts( sessions );
            ASSERT_TRUE( sessions.empty() && 0 == f.LinkCount() );

            CFakeAuthProvider* hanging = f.AddFake( "hanging", "h1" );
            hanging->loginBehaviour = FAKE_HANG;
            CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            ASSERT_TRUE( f.manager.Login( "hanging", AUTH::CAuthOptions(), future ) );
            PumpFor( 30 );
            ASSERT_TRUE( f.manager.RemoveProvider( "hanging" ) );
            ASSERT_TRUE( future.IsReady() && CORE::TTaskStatus::TASKSTATUS_CANCELLED == StatusOf( future ) );
            ASSERT_FALSE( f.manager.IsLoginPending( "hanging" ) );
            PumpFor( 100 );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The manager of the process logs in with a descriptor driven provider through an engine" )
        GUCEF_TESTFW_SKIP_IF( !Prerequisites(), "The json or ed25519 codec is not available" );
        try
        {
            AUTH::CAuthGlobal* global = AUTH::CAuthGlobal::Instance();
            AUTH::CAuthManager& manager = global->GetManager();
            CTestFlowEngine engine;
            global->SetFlowEngine( &engine );

            CTestSigner profile( "profile" );
            AUTH::CMemoryCredentialStore credentials;
            AUTH::CLocalIdentityLinkStore links;
            links.SetSigner( &profile );
            manager.SetCredentialStore( &credentials );
            manager.SetLinkStore( &links );
            manager.SetSigner( &profile );
            CAuthRecorder recorder;
            recorder.SubscribeTo( &manager );

            // the application config: a client id for google, none for microsoft
            AUTH::CAuthSettings settings;
            AUTH::CAuthProviderSettings google;
            google.clientId = "google-client-id";
            settings.SetProviderSettings( "google", google );
            settings.SetProviderSettings( "microsoft", AUTH::CAuthProviderSettings() );
            CORE::CDataNode config( "Auth", GUCEF_DATATYPE_OBJECT );
            settings.SaveConfig( config );
            ASSERT_TRUE( manager.LoadConfig( config ) );

            CORE::CString::StringVector offered;
            manager.GetOfferedProviders( offered );
            ASSERT_TRUE( AUTH::ContainsString( offered, "google" ) && AUTH::ContainsString( offered, "testopenid" ) );
            ASSERT_FALSE( AUTH::ContainsString( offered, "microsoft" ) );
            ASSERT_FALSE( AUTH::ContainsString( offered, "epic" ) );

            CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            ASSERT_TRUE( manager.Login( "google", AUTH::CAuthOptions(), future ) );
            ASSERT_TRUE( Await( future, 3000 ) );
            PumpFor( 20 );
            ASSERT_TRUE( "google-client-id" == engine.lastClientIdSeen );
            AUTH::CAuthResult result;
            ASSERT_TRUE( AUTH::CAuthResult::TryGetFromFuture( future, result ) && "google:112233" == result.accountRef.ToString() && "google" == result.providerName );
            ASSERT_TRUE( 1 == recorder.Count( CAuthRecorder::KIND_ACTION ) && AUTH::AUTHACTION_SHOW_DEVICE_CODE == recorder.Last( CAuthRecorder::KIND_ACTION )->action.kind );
            ASSERT_TRUE( AUTH::AUTHLOGINSTATE_LINKED == manager.GetLoginState() );
            AUTH::CSecretString stored;
            ASSERT_TRUE( credentials.Load( "google", "google:112233", stored ) && "engine-refresh-token" == stored.Reveal() );

            // microsoft is not offered without a client id
            ASSERT_FALSE( manager.Login( "microsoft", AUTH::CAuthOptions(), future ) );

            // put the process manager back as it was
            manager.Unlink( result.accountRef );
            manager.SetSettings( AUTH::CAuthSettings() );
            CORE::CString::StringVector names;
            manager.GetProviderNames( names );
            for ( size_t i=0; i<names.size(); ++i )
                manager.RemoveProvider( names[ i ] );
            manager.SetCredentialStore( GUCEF_NULL );
            manager.SetLinkStore( GUCEF_NULL );
            manager.SetSigner( GUCEF_NULL );
            global->RegisterBuiltInEngine();
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    UnregisterFakeLoginTask();
}

/*-------------------------------------------------------------------------*/
