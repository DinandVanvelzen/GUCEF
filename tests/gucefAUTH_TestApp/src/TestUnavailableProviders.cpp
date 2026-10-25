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

#include <string.h>

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CDSTORECODECREGISTRY_H
#include "CDStoreCodecRegistry.h"
#define GUCEF_CORE_CDSTORECODECREGISTRY_H
#endif /* GUCEF_CORE_CDSTORECODECREGISTRY_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

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
#include "TestSigner.h"
#include "TestRecorder.h"
#include "TestFakeProvider.h"
#include "TestOpenId2Backend.h"
#include "TestUnavailableProviders.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

typedef CORE::CTFactory< AUTH::CIAuthProvider, CFakeAuthProvider, MT::CMutex > TFakeProviderFactory;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static bool
Prerequisites( void )
{GUCEF_TRACE;

    return JsonCodecAvailable() && AUTH::CAuthCrypto::IsSignatureAvailable() && AUTH::CAuthCrypto::IsRsaAvailable();
}

/*-------------------------------------------------------------------------*/

static const AUTH::CAuthUnavailableProvider*
Find( const AUTH::TAuthUnavailableProviderVector& providers, const CORE::CString& name )
{
    for ( size_t i=0; i<providers.size(); ++i )
        if ( providers[ i ].providerName == name )
            return &providers[ i ];
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

/** The text of a data tree as a JSON file says it, for looking for what must not be in it */
static CORE::CString
DumpTree( const CORE::CDataNode& node, const CORE::CString& name )
{GUCEF_TRACE;

    CORE::CString text;
    CORE::CDStoreCodecRegistry::TDStoreCodecPtr codec;
    if ( !CORE::CCoreGlobal::Instance()->GetDStoreCodecRegistry().TryLookup( "json", codec, false ) || codec.IsNULL() )
        return text;
    const CORE::CString dir = MakeTestDirectory( "hint_dump" );
    CORE::CreateDirs( dir );
    const CORE::CString file = CORE::CombinePath( dir, name + ".json" );
    if ( codec->StoreDataTree( &node, file ) )
        CORE::LoadTextFileAsString( file, text, false );
    return text;
}

/*-------------------------------------------------------------------------*/

/** A manager of the providers of the process: the built in descriptors and the OpenID 2.0 backend of the tests, which detects an installed client of its provider whose user is Alice P */
class CBuiltInManager
{
    public:

    AUTH::CAuthManager manager;

    CBuiltInManager( void )
        : manager()
    {
        TestOpenId2Profile().Reset();
        TestOpenId2Profile().hintInstalled = true;
        TestOpenId2Profile().hintRunning = false;
        TestOpenId2Profile().hintName = "Alice P";
        manager.RefreshProviders();
    }

    ~CBuiltInManager()
    {
        TestOpenId2Profile().Reset();
    }

    AUTH::CDescriptorAuthProvider* OpenId2( void )
    {
        return static_cast< AUTH::CDescriptorAuthProvider* >( manager.GetProvider( "testopenid" ) );
    }
};

/*-------------------------------------------------------------------------*/

void
PerformUnavailableProviderTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING UnavailableProviders TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "UnavailableProviders" );

    // Before the HTTP client for https is there, which this group loads when it has seen what its absence does
    GUCEF_TESTFW_TESTCASE( "Test: Without an HTTP client for https the logins that need it are not offered and say why, and requests to https addresses fail with a class and a sentence" )
        GUCEF_TESTFW_SKIP_IF( !Prerequisites(), "The json, rsa or ed25519 codec is not available" );
        GUCEF_TESTFW_SKIP_IF( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN, "WinHTTP is part of the build on Windows, https needs no plugin there" );
        GUCEF_TESTFW_SKIP_IF( WEB::CWebGlobal::Instance()->GetHttpClientBackendFactory().IsConstructible( "curl" ), "The plugin for https is loaded already" );
        try
        {
            // a system without a TLS library of its own, and without the plugin for https
            CScopedNoTlsProviders noTls;
            ASSERT_FALSE( AUTH::COAuth2Http::IsHttpsSupported() );

            CBuiltInManager b;
            ASSERT_TRUE( GUCEF_NULL != b.OpenId2() );
            ASSERT_FALSE( b.OpenId2()->IsAvailable() );
            ASSERT_FALSE( b.manager.IsProviderOffered( "testopenid" ) );
            CORE::CString::StringVector offered;
            b.manager.GetOfferedProviders( offered );
            ASSERT_TRUE( offered.empty() );

            // the sentence for the Account page, with the reason that says a plugin is missing
            AUTH::TAuthUnavailableProviderVector unavailable;
            b.manager.GetUnavailableProviders( unavailable );
            const AUTH::CAuthUnavailableProvider* entry = Find( unavailable, "testopenid" );
            ASSERT_TRUE( GUCEF_NULL != entry );
            if ( GUCEF_NULL != entry )
            {
                ASSERT_TRUE( AUTH::AUTHUNAVAILABLE_MISSING_PLUGIN == entry->reason );
                ASSERT_TRUE( "No HTTPS support: no TLS provider is registered on this platform" == entry->message );
                ASSERT_TRUE( entry->configKey.IsNULLOrEmpty() );
            }

            // no browser is opened for a login that cannot be checked afterwards
            CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            ASSERT_FALSE( b.manager.Login( "testopenid", AUTH::CAuthOptions(), future ) );

            // a request to a https address fails at once, with the class and a sentence that holds the host and nothing else of the address
            AUTH::COAuth2EngineSettings settings;
            AUTH::COAuth2Http http( settings );
            AUTH::CAuthHttpRequest request;
            ASSERT_TRUE( request.Get( "https://openid.example.invalid/openid/login?openid.sig=SECRET", settings ) );
            AUTH::CAuthHttpResponse response;
            ASSERT_FALSE( http.Execute( request, response ) );
            ASSERT_TRUE( !response.transportOk && AUTH::AUTHHTTP_NO_HTTPS == response.failure );
            AUTH::TAuthFailureReason reason = AUTH::AUTHFAIL_UNKNOWN;
            CORE::CString message;
            ASSERT_TRUE( response.DescribeFailure( reason, message ) );
            ASSERT_TRUE( AUTH::AUTHFAIL_NOT_AVAILABLE == reason && "No HTTPS support: no TLS provider is registered on this platform" == message );
            CORE::CString logged = response.DescribeForLog();
            ASSERT_TRUE( logged.HasSubstr( "no-https", true ) >= 0 && logged.HasSubstr( "host=openid.example.invalid", true ) >= 0 );
            // the address is there as the log has it: the path, and the name of the query with its value hidden
            ASSERT_TRUE( logged.HasSubstr( "url=https://openid.example.invalid/openid/login?openid.sig=", true ) >= 0 );
            ASSERT_TRUE( logged.HasSubstr( "SECRET", true ) < 0 );

            // nothing is lost for http: a closed port is a connect failure
            ASSERT_TRUE( request.Get( "http://127.0.0.1:1/token?code=SECRET", settings ) );
            ASSERT_FALSE( http.Execute( request, response ) );
            ASSERT_TRUE( AUTH::AUTHHTTP_CONNECT == response.failure );
            ASSERT_TRUE( response.DescribeFailure( reason, message ) );
            ASSERT_TRUE( AUTH::AUTHFAIL_NETWORK == reason && "No connection to 127.0.0.1 could be made" == message );
            ASSERT_TRUE( response.DescribeForLog().HasSubstr( "SECRET", true ) < 0 );

            // the plugin makes the difference: the login is offered and https gets as far as the host
            ASSERT_TRUE( HttpsBackendAvailable() );
            ASSERT_TRUE( AUTH::COAuth2Http::IsHttpsSupported() );
            ASSERT_TRUE( b.OpenId2()->IsAvailable() );
            ASSERT_TRUE( b.manager.IsProviderOffered( "testopenid" ) );
            b.manager.GetUnavailableProviders( unavailable );
            ASSERT_TRUE( GUCEF_NULL == Find( unavailable, "testopenid" ) );
            ASSERT_TRUE( request.Get( "https://127.0.0.1:1/token", settings ) );
            ASSERT_FALSE( http.Execute( request, response ) );
            ASSERT_TRUE( AUTH::AUTHHTTP_CONNECT == response.failure && "curl" == response.backendName );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // the rest of the group, and the groups after it, work with the HTTP client for https which the shipped game has
    HttpsBackendAvailable();

    GUCEF_TESTFW_TESTCASE( "Test: Without any configuration the OpenID 2.0 login of the backend is offered and the providers that need a client id are listed with the key to fill in" )
        GUCEF_TESTFW_SKIP_IF( !Prerequisites(), "The json, rsa or ed25519 codec is not available" );
        try
        {
            CBuiltInManager b;
            ASSERT_TRUE( GUCEF_NULL != b.OpenId2() );

            CORE::CString::StringVector offered;
            b.manager.GetOfferedProviders( offered );
            ASSERT_TRUE( 1 == offered.size() && "testopenid" == offered[ 0 ] );

            AUTH::TAuthUnavailableProviderVector unavailable;
            b.manager.GetUnavailableProviders( unavailable );
            const char* needClientId[] = { "google", "microsoft", "facebook", "amazon", "aws-cognito", "aws-identitycenter", "epic" };
            ASSERT_TRUE( sizeof( needClientId ) / sizeof( needClientId[ 0 ] ) == unavailable.size() );
            for ( UInt32 i=0; i<sizeof( needClientId ) / sizeof( needClientId[ 0 ] ); ++i )
            {
                const AUTH::CAuthUnavailableProvider* entry = Find( unavailable, needClientId[ i ] );
                ASSERT_TRUE( GUCEF_NULL != entry );
                if ( GUCEF_NULL == entry )
                    continue;
                ASSERT_TRUE( AUTH::AUTHUNAVAILABLE_NO_CLIENT_ID == entry->reason );
                ASSERT_TRUE( "Not set up in this build: needs a client id" == entry->message );
                ASSERT_TRUE( CORE::CString( "Auth/Providers/" ) + needClientId[ i ] + "/clientId" == entry->configKey );
                ASSERT_TRUE( entry->GetTooltipText() == "Not set up in this build: needs a client id. Fill in " + entry->configKey + " in the configuration" );
                ASSERT_FALSE( b.manager.IsProviderOffered( needClientId[ i ] ) );
            }
            // the offered one is not listed as unavailable
            ASSERT_TRUE( GUCEF_NULL == Find( unavailable, "testopenid" ) );
            ASSERT_TRUE( 0 == strcmp( "no-client-id", AUTH::CAuthUnavailableProvider::GetReasonName( AUTH::AUTHUNAVAILABLE_NO_CLIENT_ID ) ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A provider with its client id is offered and no longer listed; one that needs more says what" )
        GUCEF_TESTFW_SKIP_IF( !Prerequisites(), "The json, rsa or ed25519 codec is not available" );
        try
        {
            CBuiltInManager b;
            AUTH::CAuthSettings settings = b.manager.GetSettings();
            AUTH::CAuthProviderSettings google;
            google.clientId = "123-abc.apps.googleusercontent.com";
            settings.SetProviderSettings( "google", google );
            AUTH::CAuthProviderSettings cognito;
            cognito.clientId = "cognito-app-client";
            cognito.parameters[ "region" ] = "eu-west-1";
            settings.SetProviderSettings( "aws-cognito", cognito );
            b.manager.SetSettings( settings );

            ASSERT_TRUE( b.manager.IsProviderOffered( "google" ) );
            AUTH::TAuthUnavailableProviderVector unavailable;
            b.manager.GetUnavailableProviders( unavailable );
            ASSERT_TRUE( GUCEF_NULL == Find( unavailable, "google" ) );

            // a client id, and a pool and its domain are still missing
            const AUTH::CAuthUnavailableProvider* entry = Find( unavailable, "aws-cognito" );
            ASSERT_TRUE( GUCEF_NULL != entry && AUTH::AUTHUNAVAILABLE_MISSING_SETTING == entry->reason );
            if ( GUCEF_NULL != entry )
            {
                ASSERT_TRUE( "Not set up in this build: needs poolId, poolDomain" == entry->message );
                ASSERT_TRUE( "Auth/Providers/aws-cognito/Parameters/poolId" == entry->configKey );
            }
            ASSERT_TRUE( GUCEF_NULL != Find( unavailable, "microsoft" ) && AUTH::AUTHUNAVAILABLE_NO_CLIENT_ID == Find( unavailable, "microsoft" )->reason );

            // the pool is filled in: offered
            cognito.parameters[ "poolId" ] = "eu-west-1_AbCdEfGhI";
            cognito.parameters[ "poolDomain" ] = "example.auth.eu-west-1.amazoncognito.com";
            settings.SetProviderSettings( "aws-cognito", cognito );
            b.manager.SetSettings( settings );
            ASSERT_TRUE( b.manager.IsProviderOffered( "aws-cognito" ) );
            b.manager.GetUnavailableProviders( unavailable );
            ASSERT_TRUE( GUCEF_NULL == Find( unavailable, "aws-cognito" ) );

            // one provider on its own
            AUTH::CAuthUnavailableProvider one;
            ASSERT_TRUE( b.manager.GetUnavailableProvider( "epic", one ) && "epic" == one.providerName );
            ASSERT_FALSE( b.manager.GetUnavailableProvider( "google", one ) );
            ASSERT_FALSE( b.manager.GetUnavailableProvider( "nobody", one ) );
            ASSERT_TRUE( one.providerName.IsNULLOrEmpty() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A provider that is not enabled is not listed, and the setting showUnavailableProviders switches the list off" )
        GUCEF_TESTFW_SKIP_IF( !Prerequisites(), "The json, rsa or ed25519 codec is not available" );
        try
        {
            CBuiltInManager b;
            AUTH::CAuthSettings settings = b.manager.GetSettings();
            ASSERT_TRUE( settings.showUnavailableProviders );

            settings.enabledProviders.push_back( "testopenid" );
            settings.enabledProviders.push_back( "google" );
            b.manager.SetSettings( settings );
            AUTH::TAuthUnavailableProviderVector unavailable;
            b.manager.GetUnavailableProviders( unavailable );
            ASSERT_TRUE( 1 == unavailable.size() && "google" == unavailable[ 0 ].providerName );

            settings.enabledProviders.clear();
            settings.showUnavailableProviders = false;
            b.manager.SetSettings( settings );
            b.manager.GetUnavailableProviders( unavailable );
            ASSERT_TRUE( unavailable.empty() );
            CORE::CString::StringVector offered;
            b.manager.GetOfferedProviders( offered );
            ASSERT_TRUE( 1 == offered.size() );

            // the setting is in the configuration: written, read, and true when it is not there
            CORE::CDataNode node( "Auth" );
            ASSERT_TRUE( settings.SaveConfig( node ) );
            ASSERT_FALSE( node.GetAttributeValueOrChildValueByName( "showUnavailableProviders" ).AsBool( true, true ) );
            AUTH::CAuthSettings loaded;
            ASSERT_TRUE( loaded.LoadConfig( node ) && !loaded.showUnavailableProviders );
            CORE::CDataNode without( "Auth" );
            AUTH::CAuthSettings defaults;
            defaults.showUnavailableProviders = false;
            ASSERT_TRUE( defaults.LoadConfig( without ) && !defaults.showUnavailableProviders );     // not there: what it was
            AUTH::CAuthSettings fresh;
            ASSERT_TRUE( fresh.showUnavailableProviders );
            AUTH::CAuthSettings copy( settings );
            ASSERT_FALSE( copy.showUnavailableProviders );
            copy = fresh;
            ASSERT_TRUE( copy.showUnavailableProviders );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A provider that explains itself is listed, one that does not is not, a plugin that is not loaded is listed by its descriptor" )
        try
        {
            // the factory goes after the manager: the providers are given back to it
            TFakeProviderFactory factory;
            AUTH::CAuthManager manager;
            TFakeProviderFactory::TProductPtr explained = factory.Create();
            TFakeProviderFactory::TProductPtr silent = factory.Create();
            TFakeProviderFactory::TProductPtr offered = factory.Create();
            CFakeAuthProvider* explainer = static_cast< CFakeAuthProvider* >( explained.GetPointerAlways() );
            CFakeAuthProvider* quiet = static_cast< CFakeAuthProvider* >( silent.GetPointerAlways() );
            CFakeAuthProvider* fine = static_cast< CFakeAuthProvider* >( offered.GetPointerAlways() );
            explainer->name = "explainer";
            explainer->available = false;
            explainer->explainsUnavailability = true;
            explainer->unavailability = AUTH::CAuthUnavailableProvider( "wrong", AUTH::AUTHUNAVAILABLE_NOT_AVAILABLE, "Not available: the runtime is missing", "Auth/Providers/explainer/Parameters/runtime" );
            quiet->name = "quiet";
            quiet->available = false;
            fine->name = "fine";
            ASSERT_TRUE( manager.AddProvider( explained ) && manager.AddProvider( silent ) && manager.AddProvider( offered ) );

            AUTH::TAuthUnavailableProviderVector unavailable;
            manager.GetUnavailableProviders( unavailable );
            // the plugin descriptors of the registry come with the built in ones: only the made up one and the explainer are of interest
            ASSERT_TRUE( GUCEF_NULL != Find( unavailable, "explainer" ) && GUCEF_NULL == Find( unavailable, "quiet" ) && GUCEF_NULL == Find( unavailable, "fine" ) );
            const AUTH::CAuthUnavailableProvider* entry = Find( unavailable, "explainer" );
            if ( GUCEF_NULL != entry )
            {
                // the manager names it, whatever the provider put in
                ASSERT_TRUE( "explainer" == entry->providerName && "Not available: the runtime is missing" == entry->message && "Auth/Providers/explainer/Parameters/runtime" == entry->configKey );
                ASSERT_TRUE( 0 == strcmp( "not-available", AUTH::CAuthUnavailableProvider::GetReasonName( entry->reason ) ) );
            }

            // an available provider explains nothing
            explainer->available = true;
            manager.GetUnavailableProviders( unavailable );
            ASSERT_TRUE( GUCEF_NULL == Find( unavailable, "explainer" ) );

            // a descriptor that only a plugin can make
            AUTH::CAuthProviderRegistry& registry = AUTH::CAuthGlobal::Instance()->GetDescriptorRegistry();
            AUTH::CAuthProviderDescriptor descriptor;
            descriptor.name = "pluginonly";
            descriptor.displayName = "Plugin Only";
            descriptor.accountProviderId = "pluginonly";
            descriptor.protocol = AUTH::AUTHPROTOCOL_PLATFORM;
            ASSERT_TRUE( registry.RegisterDescriptor( descriptor ) );
            manager.GetUnavailableProviders( unavailable );
            entry = Find( unavailable, "pluginonly" );
            ASSERT_TRUE( GUCEF_NULL != entry && AUTH::AUTHUNAVAILABLE_MISSING_PLUGIN == entry->reason );
            if ( GUCEF_NULL != entry )
                ASSERT_TRUE( "Not available in this build: the plugin for this login is not loaded" == entry->message && entry->configKey.IsNULLOrEmpty() && entry->GetTooltipText() == entry->message );

            // and not when the plugin made its provider, or when the settings leave it out
            AUTH::CAuthSettings settings = manager.GetSettings();
            settings.enabledProviders.push_back( "fine" );
            manager.SetSettings( settings );
            manager.GetUnavailableProviders( unavailable );
            ASSERT_TRUE( unavailable.empty() );
            registry.UnregisterDescriptor( "pluginonly" );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The hint of an OpenID 2.0 provider is what the profile of its backend detects on the machine; the other providers detect nothing" )
        GUCEF_TESTFW_SKIP_IF( !Prerequisites(), "The json, rsa or ed25519 codec is not available" );
        try
        {
            CBuiltInManager b;
            AUTH::CIAuthProvider* provider = b.manager.GetProvider( "testopenid" );
            ASSERT_TRUE( GUCEF_NULL != provider );
            CORE::CString name = "stale";
            bool running = true;
            CTestOpenId2Profile& profile = TestOpenId2Profile();

            // installed, not running
            ASSERT_TRUE( provider->GetDetectedAccountHint( name, running ) && "Alice P" == name && !running );
            ASSERT_TRUE( 1 == profile.hintLookups );

            // running
            profile.hintRunning = true;
            ASSERT_TRUE( provider->GetDetectedAccountHint( name, running ) && "Alice P" == name && running );

            // installed, and nobody is known
            profile.hintName.Clear();
            ASSERT_TRUE( provider->GetDetectedAccountHint( name, running ) && name.IsNULLOrEmpty() && running );

            // forgetting what was found is the profile's to do
            ASSERT_TRUE( 0 == profile.forgets );
            b.OpenId2()->ForgetDetectedAccountHint();
            ASSERT_TRUE( 1 == profile.forgets );

            // not installed
            profile.hintInstalled = false;
            name = "stale";
            running = true;
            ASSERT_FALSE( provider->GetDetectedAccountHint( name, running ) );
            ASSERT_TRUE( name.IsNULLOrEmpty() && !running );

            // a provider that is a descriptor of another protocol detects nothing, and asks nobody
            profile.hintInstalled = true;
            profile.hintName = "Alice P";
            const UInt32 lookups = profile.hintLookups;
            AUTH::CIAuthProvider* google = b.manager.GetProvider( "google" );
            ASSERT_TRUE( GUCEF_NULL != google && !google->GetDetectedAccountHint( name, running ) && name.IsNULLOrEmpty() && !running );
            ASSERT_TRUE( lookups == profile.hintLookups );

            // an OpenID 2.0 provider without a profile detects nothing either
            AUTH::CAuthProviderDescriptor descriptor = MakeTestOpenId2Descriptor();
            descriptor.name = "noprofile";
            descriptor.accountProviderId = "noprofile";
            AUTH::CDescriptorAuthProvider orphan( descriptor );
            ASSERT_FALSE( orphan.GetDetectedAccountHint( name, running ) );
            orphan.ForgetDetectedAccountHint();
            ASSERT_TRUE( 1 == profile.forgets );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: What the machine says about the account is a hint: no account, no link, no event, not in the configuration" )
        GUCEF_TESTFW_SKIP_IF( !Prerequisites(), "The json, rsa or ed25519 codec is not available" );
        try
        {
            CBuiltInManager b;
            CTestSigner signer( "profile" );
            AUTH::CMemoryCredentialStore store;
            const CORE::CString dir = MakeTestDirectory( "hint_links" );
            AUTH::CLocalIdentityLinkStore links( CORE::CombinePath( dir, "links.json" ) );
            links.SetSigner( &signer );
            b.manager.SetCredentialStore( &store );
            b.manager.SetLinkStore( &links );
            b.manager.SetSigner( &signer );
            CAuthRecorder recorder;
            recorder.SubscribeTo( &b.manager );

            CORE::CString name;
            bool running = false;
            TestOpenId2Profile().hintName = "NotMe-Persona";
            ASSERT_TRUE( b.manager.GetProvider( "testopenid" )->GetDetectedAccountHint( name, running ) && "NotMe-Persona" == name );
            PumpFor( 50 );

            ASSERT_TRUE( AUTH::AUTHLOGINSTATE_GUEST == b.manager.GetLoginState() );
            AUTH::TLinkRecordVector linked;
            b.manager.GetLinkedAccounts( linked );
            ASSERT_TRUE( linked.empty() );
            GUCEF::vector< AUTH::CAuthResult > sessions;
            b.manager.GetSessionAccounts( sessions );
            ASSERT_TRUE( sessions.empty() );
            ASSERT_TRUE( 0 == recorder.Count( CAuthRecorder::KIND_SUCCEEDED ) && 0 == recorder.Count( CAuthRecorder::KIND_FAILED ) && 0 == recorder.Count( CAuthRecorder::KIND_ACTION ) );

            // not in what the provider saves, not in what the manager saves
            CORE::CDataNode providerNode( "Provider" );
            ASSERT_TRUE( b.manager.GetProvider( "testopenid" )->SaveConfig( providerNode ) );
            CORE::CDataNode managerNode( "Auth" );
            ASSERT_TRUE( b.manager.SaveConfig( managerNode ) );
            const CORE::CString dump = DumpTree( providerNode, "provider" );
            const CORE::CString managerDump = DumpTree( managerNode, "manager" );
            ASSERT_TRUE( !dump.IsNULLOrEmpty() && !managerDump.IsNULLOrEmpty() );
            ASSERT_TRUE( dump.HasSubstr( "NotMe-Persona" ) < 0 && managerDump.HasSubstr( "NotMe-Persona" ) < 0 );
            ASSERT_TRUE( dump.HasSubstr( "Alice" ) < 0 && managerDump.HasSubstr( "Alice" ) < 0 );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
