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

#ifndef GUCEF_AUTH_TESTAPP_TESTOAUTH2FIXTURE_H
#define GUCEF_AUTH_TESTAPP_TESTOAUTH2FIXTURE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_AUTH_H
#include "gucefAUTH.h"
#define GUCEF_AUTH_H
#endif /* GUCEF_AUTH_H ? */

#include "TestPump.h"
#include "TestText.h"
#include "TestHttp.h"
#include "TestSigner.h"
#include "TestRecorder.h"
#include "TestRsaKeys.h"
#include "TestFakeOidc.h"
#include "TestBrowser.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  A provider that lives in the test, a browser that does not exist, the engine of the module with settings for the test, and a
 *  manager of its own that uses the descriptor providers of the module. Everything is put back when it goes away.
 */
class COAuth2Fixture
{
    public:

    CFakeOidcProvider fake;
    CTestBrowser browser;
    GUCEF::AUTH::CMemoryCredentialStore credentials;
    CTestSigner signer;
    GUCEF::AUTH::CLocalIdentityLinkStore links;
    GUCEF::AUTH::CAuthManager manager;
    CAuthRecorder recorder;
    GUCEF::AUTH::COAuth2Engine* engine;
    GUCEF::AUTH::COAuth2EngineSettings savedSettings;
    bool ready;

    COAuth2Fixture( void )
        : fake()
        , browser()
        , credentials()
        , signer( "profile" )
        , links()
        , manager()
        , recorder()
        , engine( GUCEF::AUTH::CAuthGlobal::Instance()->GetOAuth2Engine() )
        , savedSettings( engine->GetSettings() )
        , ready( false )
    {
        fake.SetClientId( "fake-client-id" );
        fake.PublishKey( "key-a", &TestKeyA() );
        fake.SetSigningKey( "key-a", &TestKeyA() );
        // a key that is in no key set, to sign what must not be accepted
        fake.PublishKey( "evil", &TestKeyC() );
        fake.UnpublishKey( "evil" );
        ready = fake.Start() && browser.Start();
        browser.Install();

        GUCEF::AUTH::COAuth2EngineSettings settings = savedSettings;
        settings.allowInsecureHttp = true;
        settings.deviceIntervalUnitInMs = 30;
        settings.jwksMinRefreshIntervalInMs = 250;
        settings.httpTimeoutInMs = 8000;
        engine->SetSettings( settings );
        engine->ClearCaches();

        links.SetSigner( &signer );
        manager.SetCredentialStore( &credentials );
        manager.SetLinkStore( &links );
        manager.SetSigner( &signer );
        recorder.SubscribeTo( &manager );
    }

    ~COAuth2Fixture()
    {
        engine->CancelAll();
        PumpFor( 30 );
        browser.Uninstall();
        browser.Stop();
        fake.Stop();
        engine->SetSettings( savedSettings );
        engine->ClearCaches();
    }

    /** Settings of a provider whose addresses are all those of the fake */
    GUCEF::AUTH::CAuthProviderSettings LocalSettings( void ) const
    {
        GUCEF::AUTH::CAuthProviderSettings settings;
        GUCEF::CORE::CString base = fake.GetBaseUrl();
        settings.clientId = "fake-client-id";
        settings.issuer = fake.GetIssuer();
        settings.authorizationEndpoint = base + "/authorize";
        settings.tokenEndpoint = base + "/token";
        settings.deviceAuthorizationEndpoint = base + "/device";
        settings.userInfoEndpoint = base + "/userinfo";
        settings.jwksUri = base + "/jwks";
        settings.revocationEndpoint = base + "/revoke";
        return settings;
    }

    /** Settings of a provider that finds its addresses in the discovery document of the fake */
    GUCEF::AUTH::CAuthProviderSettings DiscoverySettings( void ) const
    {
        GUCEF::AUTH::CAuthProviderSettings settings;
        settings.clientId = "fake-client-id";
        settings.issuer = fake.GetIssuer();
        settings.discoveryUrl = fake.GetBaseUrl() + "/.well-known/openid-configuration";
        return settings;
    }

    /** Gives the manager the settings: the providers of the module are made for the names in them */
    bool Configure( const GUCEF::AUTH::CAuthSettings& settings )
    {
        GUCEF::CORE::CDataNode config( "Auth", GUCEF_DATATYPE_OBJECT );
        settings.SaveConfig( config );
        return manager.LoadConfig( config );
    }

    bool ConfigureOne( const GUCEF::CORE::CString& providerName, const GUCEF::AUTH::CAuthProviderSettings& providerSettings )
    {
        GUCEF::AUTH::CAuthSettings settings;
        settings.SetProviderSettings( providerName, providerSettings );
        return Configure( settings );
    }

    /** Starts a login and waits for the manager to be done with it; true when it ended well */
    bool Login( const GUCEF::CORE::CString& providerName, const GUCEF::AUTH::CAuthOptions& options, GUCEF::AUTH::CAuthResult& result, GUCEF::UInt32 timeoutInMs = 20000 )
    {
        GUCEF::CORE::CFutureResult future( GUCEF::CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
        if ( !manager.Login( providerName, options, future ) )
            return false;
        for ( GUCEF::UInt32 waited=0; waited<timeoutInMs && !future.IsReady(); ++waited )
            PumpFor( 1 );
        if ( !future.IsReady() )
            return false;
        PumpFor( 20 );
        return GUCEF::AUTH::CAuthResult::TryGetFromFuture( future, result );
    }

    /** The reason of the last failure the manager reported */
    GUCEF::AUTH::TAuthFailureReason LastFailure( void ) const
    {
        const CAuthRecorder::SRecord* record = recorder.Last( CAuthRecorder::KIND_FAILED );
        return GUCEF_NULL != record ? record->failure.reason : GUCEF::AUTH::AUTHFAIL_UNKNOWN;
    }

    GUCEF::UInt32 FailureCount( void ) const
    {
        return recorder.Count( CAuthRecorder::KIND_FAILED );
    }

    /** Whether the login failed with the reason; the failure is waited for */
    bool Fails( const GUCEF::CORE::CString& providerName, const GUCEF::AUTH::CAuthOptions& options, GUCEF::AUTH::TAuthFailureReason reason, GUCEF::UInt32 timeoutInMs = 20000 )
    {
        GUCEF::UInt32 before = FailureCount();
        GUCEF::CORE::CFutureResult future( GUCEF::CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
        if ( !manager.Login( providerName, options, future ) )
            return false;
        for ( GUCEF::UInt32 waited=0; waited<timeoutInMs && !future.IsReady(); ++waited )
            PumpFor( 1 );
        if ( !future.IsReady() )
            return false;
        PumpUntilRecorded( recorder, CAuthRecorder::KIND_FAILED, before + 1, 2000 );
        GUCEF::AUTH::CAuthResult result;
        return !GUCEF::AUTH::CAuthResult::TryGetFromFuture( future, result ) && FailureCount() > before && reason == LastFailure();
    }

    /** The queries of the last address that was opened */
    GUCEF::CORE::CString::StringMap LastUrlParameters( void ) const
    {
        GUCEF::CORE::CString scheme, host, path, query;
        GUCEF::UInt16 port = 0;
        GUCEF::CORE::CString::StringMap parameters;
        bool duplicate = false;
        GUCEF::AUTH::CAuthUrl::Split( browser.GetLastUrl(), scheme, host, port, path, query );
        GUCEF::AUTH::CAuthUrl::ParseQuery( query, parameters, duplicate );
        return parameters;
    }

    private:

    COAuth2Fixture( const COAuth2Fixture& src );
    COAuth2Fixture& operator=( const COAuth2Fixture& src );
};

/*-------------------------------------------------------------------------*/

/** Pulses until the condition flag, set by the caller's loop, holds; here for a count that a fake reaches */
inline bool
PumpUntilStat( const CFakeOidcProvider& fake, GUCEF::UInt32 CFakeStats::* counter, GUCEF::UInt32 target, GUCEF::UInt32 timeoutInMs )
{
    for ( GUCEF::UInt32 waited=0; waited<timeoutInMs && fake.GetStats().*counter < target; ++waited )
        PumpFor( 1 );
    return fake.GetStats().*counter >= target;
}

/*-------------------------------------------------------------------------*/

/** The ways an id token can be wrong; false when there are no more. The fake's issuer is given for the ones that build on it */
inline bool
BadTokenSpec( GUCEF::UInt32 index, const GUCEF::CORE::CString& issuer, CFakeTokenSpec& spec, GUCEF::CORE::CString& name )
{GUCEF_TRACE;

    switch ( index )
    {
        case 0 :  name = "wrong issuer"; spec.issuer = "https://evil.example"; return true;
        case 1 :  name = "issuer with a slash"; spec.issuer = issuer + "/"; return true;
        case 2 :  name = "wrong audience"; spec.audiences.push_back( "other-client" ); return true;
        case 3 :  name = "several audiences, no authorized party"; spec.audiences.push_back( "fake-client-id" ); spec.audiences.push_back( "other-client" ); return true;
        case 4 :  name = "several audiences, wrong authorized party"; spec.audiences.push_back( "fake-client-id" ); spec.audiences.push_back( "other-client" ); spec.setAzp = true; spec.azp = "other-client"; return true;
        case 5 :  name = "wrong nonce"; spec.nonceOverride = "forged-nonce"; return true;
        case 6 :  name = "no nonce"; spec.includeNonce = false; return true;
        case 7 :  name = "expired"; spec.expiresInSeconds = -3600; return true;
        case 8 :  name = "not yet valid"; spec.setNotBefore = true; spec.notBeforeInSeconds = 3600; return true;
        case 9 :  name = "issued in the future"; spec.issuedAtInSeconds = 3600; return true;
        case 10 : name = "bad signature"; spec.tamperSignature = true; return true;
        case 11 : name = "signed with another key under the id of the good one"; spec.signWith = "evil"; return true;
        case 12 : name = "alg none"; spec.algorithm = "none"; return true;
        case 13 : name = "alg HS256"; spec.algorithm = "HS256"; return true;
        case 14 : name = "alg ES256"; spec.algorithm = "ES256"; return true;
        case 15 : name = "alg in lower case"; spec.algorithm = "rs256"; return true;
        case 16 : name = "no key id"; spec.omitKeyId = true; return true;
        case 17 : name = "crit header"; spec.addCrit = true; return true;
        case 18 : name = "alg member twice"; spec.duplicateAlg = true; return true;
        case 19 : name = "oversized token"; spec.extraPayloadMembers = "\"pad\":\"" + RepeatedText( 'p', 20000 ) + "\""; return true;
        case 20 : name = "wrong at_hash"; spec.atHashOverride = "AAAAAAAAAAAAAAAAAAAAAA"; return true;
        case 21 : name = "no subject"; spec.subject = GUCEF::CORE::CString::Empty; return true;
        default : return false;
    }
}


/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUTH_TESTAPP_TESTOAUTH2FIXTURE_H ? */
