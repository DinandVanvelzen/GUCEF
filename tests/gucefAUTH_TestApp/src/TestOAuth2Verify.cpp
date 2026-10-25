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

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_MT_CACTIVEOBJECT_H
#include "gucefMT_CActiveObject.h"
#define GUCEF_MT_CACTIVEOBJECT_H
#endif /* GUCEF_MT_CACTIVEOBJECT_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestLogCapture.h"
#include "TestOAuth2Fixture.h"
#include "TestOAuth2Verify.h"

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
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static bool
Prerequisites( void )
{GUCEF_TRACE;

    return JsonCodecAvailable() && AUTH::CAuthCrypto::IsRsaAvailable() && AUTH::CAuthCrypto::IsSignatureAvailable();
}

/*-------------------------------------------------------------------------*/

/** The server: the provider as it is configured there, asked to check a proof; it waits for the network on the calling thread */
static bool
VerifyProof( COAuth2Fixture& f, const CORE::CString& providerName, const AUTH::CAuthProof& proof, AUTH::CAuthResult& result )
{GUCEF_TRACE;

    AUTH::CIAuthProvider* provider = f.manager.GetProvider( providerName );
    if ( GUCEF_NULL == provider )
        return false;
    return provider->Verify( proof, result );
}

/*-------------------------------------------------------------------------*/

static AUTH::CAuthProof
IdTokenProof( COAuth2Fixture& f, const CFakeTokenSpec& spec )
{GUCEF_TRACE;

    return AUTH::CAuthProof( AUTH::CAuthProof::KindOidcIdToken, f.fake.MintIdToken( spec, CORE::CString::Empty, CORE::CString::Empty ) );
}

/*-------------------------------------------------------------------------*/

/** A thread of the server that checks proofs, to see that the engine can be asked from several threads */
class CVerifyThread : public MT::CActiveObject
{
    public:

    CVerifyThread( AUTH::CIAuthProvider* provider, const AUTH::CAuthProof& proof, UInt32 rounds )
        : MT::CActiveObject(), m_provider( provider ), m_proof( proof ), m_rounds( rounds ), m_succeeded( 0 ), m_done( false )
    {
    }

    UInt32 GetSucceeded( void ) const { return m_succeeded; }
    bool IsDone( void ) const { return m_done; }

    virtual const MT::CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NULL; }

    protected:

    virtual bool OnThreadStart( void* taskdata ) GUCEF_VIRTUAL_OVERRIDE { return true; }

    virtual bool OnThreadCycle( void* taskdata ) GUCEF_VIRTUAL_OVERRIDE
    {
        for ( UInt32 i=0; i<m_rounds; ++i )
        {
            AUTH::CAuthResult result;
            if ( m_provider->Verify( m_proof, result ) && "google:sub-1000" == result.accountRef.ToString() )
                ++m_succeeded;
        }
        m_done = true;
        return true;
    }

    private:

    AUTH::CIAuthProvider* m_provider;
    AUTH::CAuthProof m_proof;
    UInt32 m_rounds;
    UInt32 m_succeeded;
    volatile bool m_done;
};

/*-------------------------------------------------------------------------*/

static void
PerformIdTokenVerifyTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Test: A server checks the id token of a client: signature, issuer, audience, expiry; the account follows from the claims" )
        GUCEF_TESTFW_SKIP_IF( !Prerequisites(), "The json, rsa or ed25519 codec is not available" );
        try
        {
            COAuth2Fixture f;
            ASSERT_TRUE( f.ready );
            ASSERT_TRUE( f.ConfigureOne( "google", f.LocalSettings() ) );

            AUTH::CAuthResult result;
            CFakeTokenSpec good;
            AUTH::CAuthProof proof = IdTokenProof( f, good );
            ASSERT_TRUE( VerifyProof( f, "google", proof, result ) );
            ASSERT_TRUE( "google:sub-1000" == result.accountRef.ToString() && "google" == result.providerName && "Fake Player" == result.displayName );
            ASSERT_TRUE( result.expiresAtMs > AUTH::GetEpochTimeInMs() + 3000000 && AUTH::CAuthProof::KindOidcIdToken == result.proof.kind && result.proof.data.Reveal() == proof.data.Reveal() );
            ASSERT_TRUE( result.accessToken.IsEmpty() && result.refreshToken.IsEmpty() );
            ASSERT_TRUE( 1 == f.fake.GetStats().jwks );

            // the keys are cached: the second proof does not fetch
            good.subject = "sub-2000";
            ASSERT_TRUE( VerifyProof( f, "google", IdTokenProof( f, good ), result ) && "google:sub-2000" == result.accountRef.ToString() );
            ASSERT_TRUE( 1 == f.fake.GetStats().jwks );

            // every way a token can be wrong. The nonce and the access token belong to the login of the client, a server does not have them
            UInt32 refused = 0, accepted = 0;
            for ( UInt32 i=0; ; ++i )
            {
                CFakeTokenSpec spec;
                CORE::CString name;
                if ( !BadTokenSpec( i, f.fake.GetIssuer(), spec, name ) )
                    break;
                bool expectedGood = 5 == i || 6 == i || 20 == i;
                bool verified = VerifyProof( f, "google", IdTokenProof( f, spec ), result );
                if ( verified != expectedGood )
                {
                    const CORE::CString verdict = verified ? "accepted" : "refused";
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Test: The proof was " << verdict << " but should not have been: " << name );
                }
                ASSERT_TRUE( verified == expectedGood );
                if ( !verified )
                {
                    ++refused;
                    ASSERT_FALSE( result.accountRef.IsValid() );
                    ASSERT_TRUE( result.accessToken.IsEmpty() && result.idToken.IsEmpty() && result.proof.IsEmpty() );
                }
                else
                    ++accepted;
            }
            ASSERT_TRUE( 19 == refused && 3 == accepted );

            // a server that has another client id: the audience is not its own
            AUTH::CAuthProviderSettings other = f.LocalSettings();
            other.clientId = "the-server-client-id";
            ASSERT_TRUE( f.ConfigureOne( "google", other ) );
            ASSERT_FALSE( VerifyProof( f, "google", IdTokenProof( f, good ), result ) );
            CFakeTokenSpec forServer;
            forServer.audiences.push_back( "the-server-client-id" );
            ASSERT_TRUE( VerifyProof( f, "google", IdTokenProof( f, forServer ), result ) );

            // proofs of the wrong kind, empty ones and garbage
            ASSERT_TRUE( f.ConfigureOne( "google", f.LocalSettings() ) );
            ASSERT_FALSE( VerifyProof( f, "google", AUTH::CAuthProof(), result ) );
            ASSERT_FALSE( VerifyProof( f, "google", AUTH::CAuthProof( AUTH::CAuthProof::KindOpenId2Assertion, proof.data.Reveal() ), result ) );
            ASSERT_FALSE( VerifyProof( f, "google", AUTH::CAuthProof( AUTH::CAuthProof::KindOAuth2AccessToken, proof.data.Reveal() ), result ) );
            ASSERT_FALSE( VerifyProof( f, "google", AUTH::CAuthProof( AUTH::CAuthProof::KindPlatformTicket, proof.data.Reveal() ), result ) );
            ASSERT_FALSE( VerifyProof( f, "google", AUTH::CAuthProof( AUTH::CAuthProof::KindOidcIdToken, "not.a.token" ), result ) );
            ASSERT_FALSE( VerifyProof( f, "google", AUTH::CAuthProof( AUTH::CAuthProof::KindOidcIdToken, RepeatedText( 'x', 100000 ) ), result ) );
            ASSERT_FALSE( VerifyProof( f, "google", AUTH::CAuthProof( "made-up-kind", proof.data.Reveal() ), result ) );
            // a provider that cannot be verified by a server
            AUTH::CAuthProviderSettings center;
            center.clientId = "fake-client-id";
            center.parameters[ "region" ] = "eu-west-1";
            center.parameters[ "instanceId" ] = "d-1234567890";
            ASSERT_TRUE( f.ConfigureOne( "aws-identitycenter", center ) );
            ASSERT_FALSE( VerifyProof( f, "aws-identitycenter", proof, result ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A server finds a rotated key with one refresh, not more than one per interval; keys of the discovery document work" )
        GUCEF_TESTFW_SKIP_IF( !Prerequisites(), "The json, rsa or ed25519 codec is not available" );
        try
        {
            COAuth2Fixture f;
            ASSERT_TRUE( f.ready );
            AUTH::COAuth2EngineSettings settings = f.engine->GetSettings();
            settings.jwksMinRefreshIntervalInMs = 1000;
            f.engine->SetSettings( settings );
            ASSERT_TRUE( f.ConfigureOne( "google", f.DiscoverySettings() ) );

            AUTH::CAuthResult result;
            CFakeTokenSpec good;
            ASSERT_TRUE( VerifyProof( f, "google", IdTokenProof( f, good ), result ) );
            ASSERT_TRUE( 1 == f.fake.GetStats().discovery && 1 == f.fake.GetStats().jwks );

            // an unknown key id right after: refused without a fetch; after the interval: one fetch and refused
            CFakeTokenSpec unknown;
            unknown.keyId = "nobody";
            ASSERT_FALSE( VerifyProof( f, "google", IdTokenProof( f, unknown ), result ) );
            ASSERT_TRUE( 1 == f.fake.GetStats().jwks );
            PumpFor( 1100 );
            ASSERT_FALSE( VerifyProof( f, "google", IdTokenProof( f, unknown ), result ) );
            ASSERT_TRUE( 2 == f.fake.GetStats().jwks );
            ASSERT_FALSE( VerifyProof( f, "google", IdTokenProof( f, unknown ), result ) );
            ASSERT_TRUE( 2 == f.fake.GetStats().jwks );

            // the provider rotates
            f.fake.PublishKey( "key-b", &TestKeyB() );
            f.fake.SetSigningKey( "key-b", &TestKeyB() );
            PumpFor( 1100 );
            ASSERT_TRUE( VerifyProof( f, "google", IdTokenProof( f, good ), result ) );
            ASSERT_TRUE( 3 == f.fake.GetStats().jwks );
            // both keys work now
            CFakeTokenSpec oldKey;
            oldKey.keyId = "key-a";
            f.fake.SetSigningKey( "key-a", &TestKeyA() );
            ASSERT_TRUE( VerifyProof( f, "google", IdTokenProof( f, oldKey ), result ) );
            ASSERT_TRUE( 3 == f.fake.GetStats().jwks );

            // the key set cannot be fetched and is not cached: the proof is refused, and the error is not remembered
            AUTH::CAuthProviderSettings broken = f.LocalSettings();
            broken.jwksUri = "http://127.0.0.1:1/jwks";
            ASSERT_TRUE( f.ConfigureOne( "google", broken ) );
            ASSERT_FALSE( VerifyProof( f, "google", IdTokenProof( f, good ), result ) );
            ASSERT_TRUE( f.ConfigureOne( "google", f.LocalSettings() ) );
            ASSERT_TRUE( VerifyProof( f, "google", IdTokenProof( f, good ), result ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A server checks the tenant of a Microsoft proof against its tenant setting" )
        GUCEF_TESTFW_SKIP_IF( !Prerequisites(), "The json, rsa or ed25519 codec is not available" );
        try
        {
            COAuth2Fixture f;
            ASSERT_TRUE( f.ready );
            f.fake.SetIssuerPath( "/{tid}/v2.0" );
            AUTH::CAuthProviderSettings microsoft = f.LocalSettings();
            microsoft.issuer = f.fake.GetBaseUrl() + "/{tid}/v2.0";
            ASSERT_TRUE( f.ConfigureOne( "microsoft", microsoft ) );
            CFakeTokenSpec spec;
            spec.tenantId = "5d8c3d0e-1c41-4d63-b2d8-2f7c5a1e0001";
            spec.objectId = "oid-77";
            AUTH::CAuthResult result;
            ASSERT_TRUE( VerifyProof( f, "microsoft", IdTokenProof( f, spec ), result ) && "microsoft:5d8c3d0e-1c41-4d63-b2d8-2f7c5a1e0001:oid-77" == result.accountRef.ToString() );

            microsoft.parameters[ "tenant" ] = "another-tenant";
            ASSERT_TRUE( f.ConfigureOne( "microsoft", microsoft ) );
            ASSERT_FALSE( VerifyProof( f, "microsoft", IdTokenProof( f, spec ), result ) );
            microsoft.parameters[ "tenant" ] = "5d8c3d0e-1c41-4d63-b2d8-2f7c5a1e0001";
            ASSERT_TRUE( f.ConfigureOne( "microsoft", microsoft ) );
            ASSERT_TRUE( VerifyProof( f, "microsoft", IdTokenProof( f, spec ), result ) );
            // no oid: no account
            spec.objectId = CORE::CString::Empty;
            ASSERT_FALSE( VerifyProof( f, "microsoft", IdTokenProof( f, spec ), result ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Several threads of a server can check proofs at the same time" )
        GUCEF_TESTFW_SKIP_IF( !Prerequisites(), "The json, rsa or ed25519 codec is not available" );
        try
        {
            COAuth2Fixture f;
            ASSERT_TRUE( f.ready );
            ASSERT_TRUE( f.ConfigureOne( "google", f.LocalSettings() ) );
            AUTH::CIAuthProvider* provider = f.manager.GetProvider( "google" );
            ASSERT_TRUE( GUCEF_NULL != provider );
            if ( GUCEF_NULL == provider )
                return;
            AUTH::CAuthProof proof = IdTokenProof( f, CFakeTokenSpec() );

            // the key set is not cached yet: they all ask for it
            CVerifyThread first( provider, proof, 6 ), second( provider, proof, 6 ), third( provider, proof, 6 ), fourth( provider, proof, 6 );
            ASSERT_TRUE( first.Activate( GUCEF_NULL, 1, 1 ) && second.Activate( GUCEF_NULL, 1, 1 ) && third.Activate( GUCEF_NULL, 1, 1 ) && fourth.Activate( GUCEF_NULL, 1, 1 ) );
            for ( UInt32 waited=0; waited<30000 && !( first.IsDone() && second.IsDone() && third.IsDone() && fourth.IsDone() ); ++waited )
                MT::ThreadDelay( 1 );
            ASSERT_TRUE( first.IsDone() && second.IsDone() && third.IsDone() && fourth.IsDone() );
            ASSERT_TRUE( 6 == first.GetSucceeded() && 6 == second.GetSucceeded() && 6 == third.GetSucceeded() && 6 == fourth.GetSucceeded() );
            ASSERT_TRUE( f.fake.GetStats().jwks >= 1 && f.fake.GetStats().jwks <= 4 );
            first.Deactivate( false, true );
            second.Deactivate( false, true );
            third.Deactivate( false, true );
            fourth.Deactivate( false, true );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

/** The settings of a game server: no client id of its own, the client ids of every client of the game as audiences */
static AUTH::CAuthProviderSettings
ServerSettings( const COAuth2Fixture& f )
{GUCEF_TRACE;

    AUTH::CAuthProviderSettings settings = f.LocalSettings();
    settings.clientId = CORE::CString::Empty;
    settings.audiences.push_back( "desktop-client.example.invalid" );
    settings.audiences.push_back( "android-client.example.invalid" );
    return settings;
}

/*-------------------------------------------------------------------------*/

static CFakeTokenSpec
TokenFor( const CORE::CString& audience, const CORE::CString& subject = "sub-1000" )
{GUCEF_TRACE;

    CFakeTokenSpec spec;
    spec.audiences.push_back( audience );
    spec.subject = subject;
    return spec;
}

/*-------------------------------------------------------------------------*/

static void
PerformMultipleAudienceTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Test: A server with the client ids of several clients accepts the id token of any of them and of no other client" )
        GUCEF_TESTFW_SKIP_IF( !Prerequisites(), "The json, rsa or ed25519 codec is not available" );
        try
        {
            COAuth2Fixture f;
            ASSERT_TRUE( f.ready );
            ASSERT_TRUE( f.ConfigureOne( "google", ServerSettings( f ) ) );

            AUTH::CAuthResult result;
            ASSERT_TRUE( VerifyProof( f, "google", IdTokenProof( f, TokenFor( "desktop-client.example.invalid" ) ), result ) );
            ASSERT_TRUE( "google:sub-1000" == result.accountRef.ToString() && "google" == result.providerName );
            ASSERT_TRUE( VerifyProof( f, "google", IdTokenProof( f, TokenFor( "android-client.example.invalid", "sub-2000" ) ), result ) );
            ASSERT_TRUE( "google:sub-2000" == result.accountRef.ToString() );

            // a client that is not one of the game's, also one that looks like it
            ASSERT_FALSE( VerifyProof( f, "google", IdTokenProof( f, TokenFor( "web-client.example.invalid" ) ), result ) );
            ASSERT_FALSE( result.accountRef.IsValid() );
            ASSERT_FALSE( VerifyProof( f, "google", IdTokenProof( f, TokenFor( "android-client.example.invalid.evil" ) ), result ) );
            ASSERT_FALSE( VerifyProof( f, "google", IdTokenProof( f, TokenFor( "ANDROID-CLIENT.example.invalid" ) ), result ) );
            ASSERT_FALSE( VerifyProof( f, "google", IdTokenProof( f, TokenFor( "fake-client-id" ) ), result ) );
            ASSERT_FALSE( VerifyProof( f, "google", IdTokenProof( f, TokenFor( CORE::CString::Empty ) ), result ) );

            // the other checks still hold for every client id
            CFakeTokenSpec expired = TokenFor( "android-client.example.invalid" );
            expired.expiresInSeconds = -3600;
            ASSERT_FALSE( VerifyProof( f, "google", IdTokenProof( f, expired ), result ) );
            CFakeTokenSpec forged = TokenFor( "desktop-client.example.invalid" );
            forged.signWith = "evil";
            ASSERT_FALSE( VerifyProof( f, "google", IdTokenProof( f, forged ), result ) );
            CFakeTokenSpec wrongIssuer = TokenFor( "desktop-client.example.invalid" );
            wrongIssuer.issuer = "https://evil.example";
            ASSERT_FALSE( VerifyProof( f, "google", IdTokenProof( f, wrongIssuer ), result ) );

            // a game server with one client id of its own and further ones: all of them
            AUTH::CAuthProviderSettings withOwn = ServerSettings( f );
            withOwn.clientId = "fake-client-id";
            ASSERT_TRUE( f.ConfigureOne( "google", withOwn ) );
            ASSERT_TRUE( VerifyProof( f, "google", IdTokenProof( f, TokenFor( "fake-client-id" ) ), result ) );
            ASSERT_TRUE( VerifyProof( f, "google", IdTokenProof( f, TokenFor( "desktop-client.example.invalid" ) ), result ) );
            ASSERT_TRUE( VerifyProof( f, "google", IdTokenProof( f, TokenFor( "android-client.example.invalid" ) ), result ) );
            ASSERT_FALSE( VerifyProof( f, "google", IdTokenProof( f, TokenFor( "web-client.example.invalid" ) ), result ) );

            // the list is a setting of the provider of the server: another provider of the same server does not accept the clients of this one
            AUTH::CAuthSettings both;
            both.SetProviderSettings( "google", ServerSettings( f ) );
            AUTH::CAuthProviderSettings epic = f.LocalSettings();
            epic.clientId = "epic-client";
            both.SetProviderSettings( "epic", epic );
            ASSERT_TRUE( f.Configure( both ) );
            ASSERT_TRUE( VerifyProof( f, "google", IdTokenProof( f, TokenFor( "android-client.example.invalid" ) ), result ) );
            ASSERT_FALSE( VerifyProof( f, "epic", IdTokenProof( f, TokenFor( "android-client.example.invalid" ) ), result ) );
            ASSERT_TRUE( VerifyProof( f, "epic", IdTokenProof( f, TokenFor( "epic-client" ) ), result ) && "epic:sub-1000" == result.accountRef.ToString() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A token is accepted when one audience is a client of the server and the authorized party, where there is one or several audiences, is one of its clients" )
        GUCEF_TESTFW_SKIP_IF( !Prerequisites(), "The json, rsa or ed25519 codec is not available" );
        try
        {
            COAuth2Fixture f;
            ASSERT_TRUE( f.ready );
            ASSERT_TRUE( f.ConfigureOne( "google", ServerSettings( f ) ) );
            const CORE::CString desktop = "desktop-client.example.invalid";
            const CORE::CString android = "android-client.example.invalid";
            AUTH::CAuthResult result;

            // the audience names the client the token was issued to and a resource server
            CFakeTokenSpec spec;
            spec.audiences.push_back( "the-resource-server" );
            spec.audiences.push_back( android );
            spec.setAzp = true;
            spec.azp = android;
            ASSERT_TRUE( VerifyProof( f, "google", IdTokenProof( f, spec ), result ) );

            // one audience and the authorized party of the same client
            CFakeTokenSpec single = TokenFor( desktop );
            single.setAzp = true;
            single.azp = desktop;
            ASSERT_TRUE( VerifyProof( f, "google", IdTokenProof( f, single ), result ) );

            // several audiences without an authorized party
            CFakeTokenSpec noAzp;
            noAzp.audiences.push_back( desktop );
            noAzp.audiences.push_back( "the-resource-server" );
            ASSERT_FALSE( VerifyProof( f, "google", IdTokenProof( f, noAzp ), result ) );

            // an authorized party that is not a client of the server, or one that the audience does not name
            CFakeTokenSpec foreignAzp = noAzp;
            foreignAzp.setAzp = true;
            foreignAzp.azp = "the-resource-server";
            ASSERT_FALSE( VerifyProof( f, "google", IdTokenProof( f, foreignAzp ), result ) );
            CFakeTokenSpec singleForeignAzp = TokenFor( desktop );
            singleForeignAzp.setAzp = true;
            singleForeignAzp.azp = "the-resource-server";
            ASSERT_FALSE( VerifyProof( f, "google", IdTokenProof( f, singleForeignAzp ), result ) );
            CFakeTokenSpec emptyAzp = noAzp;
            emptyAzp.setAzp = true;
            emptyAzp.azp = CORE::CString::Empty;
            ASSERT_FALSE( VerifyProof( f, "google", IdTokenProof( f, emptyAzp ), result ) );

            // the audience is the client of the game that a server is, the authorized party the client that asked for the token: both are ours
            CFakeTokenSpec forWebFromAndroid = TokenFor( desktop );
            forWebFromAndroid.setAzp = true;
            forWebFromAndroid.azp = android;
            ASSERT_TRUE( VerifyProof( f, "google", IdTokenProof( f, forWebFromAndroid ), result ) );

            // several audiences that are none of ours, with an authorized party that is ours
            CFakeTokenSpec noneOurs;
            noneOurs.audiences.push_back( "a-server" );
            noneOurs.audiences.push_back( "another-server" );
            noneOurs.setAzp = true;
            noneOurs.azp = desktop;
            ASSERT_FALSE( VerifyProof( f, "google", IdTokenProof( f, noneOurs ), result ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A server without any client id refuses every proof, never accepts all" )
        GUCEF_TESTFW_SKIP_IF( !Prerequisites(), "The json, rsa or ed25519 codec is not available" );
        try
        {
            COAuth2Fixture f;
            ASSERT_TRUE( f.ready );
            AUTH::CAuthResult result;

            // nothing configured
            AUTH::CAuthProviderSettings empty = f.LocalSettings();
            empty.clientId = CORE::CString::Empty;
            ASSERT_TRUE( f.ConfigureOne( "google", empty ) );
            ASSERT_FALSE( VerifyProof( f, "google", IdTokenProof( f, TokenFor( "fake-client-id" ) ), result ) );
            ASSERT_FALSE( VerifyProof( f, "google", IdTokenProof( f, TokenFor( "desktop-client.example.invalid" ) ), result ) );
            ASSERT_FALSE( VerifyProof( f, "google", IdTokenProof( f, TokenFor( CORE::CString::Empty ) ), result ) );
            ASSERT_FALSE( result.accountRef.IsValid() );

            // a list of nothing: empty entries are not client ids
            empty.audiences.push_back( CORE::CString::Empty );
            empty.audiences.push_back( "   " );
            ASSERT_TRUE( f.ConfigureOne( "google", empty ) );
            ASSERT_FALSE( VerifyProof( f, "google", IdTokenProof( f, TokenFor( CORE::CString::Empty ) ), result ) );
            ASSERT_FALSE( VerifyProof( f, "google", IdTokenProof( f, TokenFor( "fake-client-id" ) ), result ) );
            ASSERT_FALSE( VerifyProof( f, "google", IdTokenProof( f, TokenFor( "   " ) ), result ) );

            // the same through the data tree of the configuration, with the list left out and with an empty list
            CORE::CDataNode config( "Auth", GUCEF_DATATYPE_OBJECT );
            CORE::CDataNode* providers = config.AddChild( "Providers", GUCEF_DATATYPE_OBJECT );
            ASSERT_TRUE( GUCEF_NULL != providers );
            CORE::CDataNode* google = providers->AddChild( "google", GUCEF_DATATYPE_OBJECT );
            ASSERT_TRUE( GUCEF_NULL != google );
            AUTH::CAuthProviderSettings local = f.LocalSettings();
            local.clientId = CORE::CString::Empty;
            ASSERT_TRUE( local.SaveConfig( *google ) );
            ASSERT_TRUE( f.manager.LoadConfig( config ) );
            ASSERT_FALSE( VerifyProof( f, "google", IdTokenProof( f, TokenFor( "fake-client-id" ) ), result ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Logging in is unchanged: the client id of the build is the only one a login uses and the only audience a login accepts" )
        GUCEF_TESTFW_SKIP_IF( !Prerequisites(), "The json, rsa or ed25519 codec is not available" );
        try
        {
            COAuth2Fixture f;
            ASSERT_TRUE( f.ready );

            // the client of this build has its id, the others are only for a server
            AUTH::CAuthProviderSettings desktop = f.LocalSettings();
            desktop.audiences.push_back( "android-client.example.invalid" );
            desktop.audiences.push_back( "web-client.example.invalid" );
            ASSERT_TRUE( f.ConfigureOne( "google", desktop ) );
            AUTH::CAuthResult result;
            ASSERT_TRUE( f.Login( "google", AUTH::CAuthOptions(), result ) && "google:sub-1000" == result.accountRef.ToString() );
            ASSERT_TRUE( "fake-client-id" == f.fake.GetLastAuthorizeParameters()[ "client_id" ] );
            ASSERT_TRUE( "fake-client-id" == f.fake.GetLastTokenParameters()[ "client_id" ] );

            // an id token for one of the other clients is not a login of this one
            f.manager.Unlink( result.accountRef );
            f.fake.SetTokenSpec( TokenFor( "android-client.example.invalid" ) );
            ASSERT_TRUE( f.Fails( "google", AUTH::CAuthOptions(), AUTH::AUTHFAIL_INVALID_RESPONSE ) );
            f.fake.SetTokenSpec( CFakeTokenSpec() );

            // the proof of the login, as a server of this game has it
            AUTH::CAuthResult login, verified;
            ASSERT_TRUE( f.Login( "google", AUTH::CAuthOptions(), login ) );
            ASSERT_TRUE( VerifyProof( f, "google", login.proof, verified ) && login.accountRef == verified.accountRef );

            // a client that has only a list is not a client: nothing to log in with
            COAuth2Fixture g;
            ASSERT_TRUE( g.ready );
            ASSERT_TRUE( g.ConfigureOne( "google", ServerSettings( g ) ) );
            AUTH::CIAuthProvider* provider = g.manager.GetProvider( "google" );
            ASSERT_TRUE( GUCEF_NULL != provider );
            if ( GUCEF_NULL == provider )
                return;
            ASSERT_FALSE( provider->IsAvailable() );
            ASSERT_FALSE( g.manager.IsProviderOffered( "google" ) );
            AUTH::CAuthUnavailableProvider unavailable;
            ASSERT_TRUE( g.manager.GetUnavailableProvider( "google", unavailable ) && AUTH::AUTHUNAVAILABLE_NO_CLIENT_ID == unavailable.reason );
            CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            ASSERT_FALSE( g.manager.Login( "google", AUTH::CAuthOptions(), future ) );
            // the server role works all the same
            ASSERT_TRUE( VerifyProof( g, "google", IdTokenProof( g, TokenFor( "android-client.example.invalid" ) ), verified ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A server checks the access tokens of Facebook and Amazon against every client id it has" )
        GUCEF_TESTFW_SKIP_IF( !Prerequisites(), "The json, rsa or ed25519 codec is not available" );
        try
        {
            COAuth2Fixture f;
            ASSERT_TRUE( f.ready );
            const Int64 later = (Int64) ( AUTH::GetEpochTimeInMs() / 1000 ) + 3600;
            AUTH::CAuthResult result;

            // Facebook: the app secret belongs to the app id of the server (clientId); tokens of its other apps are accepted too
            AUTH::CAuthProviderSettings facebook = f.LocalSettings();
            facebook.clientSecret.Set( "the-app-secret" );
            facebook.tokenVerifyEndpoint = f.fake.GetBaseUrl() + "/debug_token";
            facebook.userInfoEndpoint = f.fake.GetBaseUrl() + "/me";
            facebook.audiences.push_back( "second-app" );
            ASSERT_TRUE( f.ConfigureOne( "facebook", facebook ) );
            f.fake.RegisterVerifyToken( "fb-own", "fake-client-id", "fb-user-1", true, later );
            f.fake.RegisterVerifyToken( "fb-second", "second-app", "fb-user-2", true, later );
            f.fake.RegisterVerifyToken( "fb-third", "third-app", "fb-user-3", true, later );
            ASSERT_TRUE( VerifyProof( f, "facebook", AUTH::CAuthProof( AUTH::CAuthProof::KindOAuth2AccessToken, "fb-own" ), result ) && "facebook:fb-user-1" == result.accountRef.ToString() );
            ASSERT_TRUE( VerifyProof( f, "facebook", AUTH::CAuthProof( AUTH::CAuthProof::KindOAuth2AccessToken, "fb-second" ), result ) && "facebook:fb-user-2" == result.accountRef.ToString() );
            ASSERT_FALSE( VerifyProof( f, "facebook", AUTH::CAuthProof( AUTH::CAuthProof::KindOAuth2AccessToken, "fb-third" ), result ) );

            // without the app id there is no app access token to ask with: nothing is asked and nothing accepted
            COAuth2Fixture g;
            ASSERT_TRUE( g.ready );
            AUTH::CAuthProviderSettings noApp = facebook;
            noApp.clientId = CORE::CString::Empty;
            noApp.tokenVerifyEndpoint = g.fake.GetBaseUrl() + "/debug_token";
            noApp.userInfoEndpoint = g.fake.GetBaseUrl() + "/me";
            ASSERT_TRUE( g.ConfigureOne( "facebook", noApp ) );
            g.fake.RegisterVerifyToken( "fb-second", "second-app", "fb-user-2", true, later );
            ASSERT_FALSE( VerifyProof( g, "facebook", AUTH::CAuthProof( AUTH::CAuthProof::KindOAuth2AccessToken, "fb-second" ), result ) );
            ASSERT_TRUE( 0 == g.fake.GetStats().verifyRequests );

            // Amazon: the audience of tokeninfo is any of the client ids, no secret is involved
            AUTH::CAuthProviderSettings amazon = f.LocalSettings();
            amazon.clientId = CORE::CString::Empty;
            amazon.userInfoEndpoint = f.fake.GetBaseUrl() + "/profile";
            amazon.tokenVerifyEndpoint = f.fake.GetBaseUrl() + "/tokeninfo";
            amazon.audiences.push_back( "amzn1.application-oa2-client.desktop" );
            amazon.audiences.push_back( "amzn1.application-oa2-client.android" );
            ASSERT_TRUE( f.ConfigureOne( "amazon", amazon ) );
            f.fake.RegisterVerifyToken( "amz-desktop", "amzn1.application-oa2-client.desktop", "amzn1.account.AAA", true, 0 );
            f.fake.RegisterVerifyToken( "amz-android", "amzn1.application-oa2-client.android", "amzn1.account.BBB", true, 0 );
            f.fake.RegisterVerifyToken( "amz-other", "amzn1.application-oa2-client.other", "amzn1.account.CCC", true, 0 );
            ASSERT_TRUE( VerifyProof( f, "amazon", AUTH::CAuthProof( AUTH::CAuthProof::KindOAuth2AccessToken, "amz-desktop" ), result ) && "amazon:amzn1.account.AAA" == result.accountRef.ToString() );
            ASSERT_TRUE( VerifyProof( f, "amazon", AUTH::CAuthProof( AUTH::CAuthProof::KindOAuth2AccessToken, "amz-android" ), result ) && "amazon:amzn1.account.BBB" == result.accountRef.ToString() );
            ASSERT_FALSE( VerifyProof( f, "amazon", AUTH::CAuthProof( AUTH::CAuthProof::KindOAuth2AccessToken, "amz-other" ), result ) );

            // nothing configured: nothing asked, nothing accepted
            AUTH::CAuthProviderSettings emptyAmazon = amazon;
            emptyAmazon.audiences.clear();
            emptyAmazon.tokenVerifyEndpoint = g.fake.GetBaseUrl() + "/tokeninfo";
            emptyAmazon.userInfoEndpoint = g.fake.GetBaseUrl() + "/profile";
            ASSERT_TRUE( g.ConfigureOne( "amazon", emptyAmazon ) );
            g.fake.RegisterVerifyToken( "amz-desktop", "amzn1.application-oa2-client.desktop", "amzn1.account.AAA", true, 0 );
            ASSERT_FALSE( VerifyProof( g, "amazon", AUTH::CAuthProof( AUTH::CAuthProof::KindOAuth2AccessToken, "amz-desktop" ), result ) );
            ASSERT_TRUE( 0 == g.fake.GetStats().verifyRequests );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Verifying with several client ids puts no token in the log" )
        GUCEF_TESTFW_SKIP_IF( !Prerequisites(), "The json, rsa or ed25519 codec is not available" );
        try
        {
            CLogCapture capture;
            CORE::CString::StringVector secrets;
            {
                COAuth2Fixture f;
                ASSERT_TRUE( f.ready );
                AUTH::CAuthProviderSettings google = ServerSettings( f );
                google.clientSecret.Set( "client-secret-VALUE-0043" );
                ASSERT_TRUE( f.ConfigureOne( "google", google ) );
                secrets.push_back( "client-secret-VALUE-0043" );

                AUTH::CAuthResult result;
                CORE::CString::StringVector audiences;
                audiences.push_back( "desktop-client.example.invalid" );
                audiences.push_back( "android-client.example.invalid" );
                audiences.push_back( "web-client.example.invalid" );
                audiences.push_back( CORE::CString::Empty );
                for ( size_t i=0; i<audiences.size(); ++i )
                {
                    AUTH::CAuthProof proof = IdTokenProof( f, TokenFor( audiences[ i ] ) );
                    secrets.push_back( proof.data.Reveal() );
                    VerifyProof( f, "google", proof, result );
                }
                AUTH::CAuthProviderSettings none = f.LocalSettings();
                none.clientId = CORE::CString::Empty;
                ASSERT_TRUE( f.ConfigureOne( "google", none ) );
                AUTH::CAuthProof refused = IdTokenProof( f, TokenFor( "desktop-client.example.invalid" ) );
                secrets.push_back( refused.data.Reveal() );
                ASSERT_FALSE( VerifyProof( f, "google", refused, result ) );
            }
            CORE::CString log = capture.Text();
            for ( size_t i=0; i<secrets.size(); ++i )
            {
                ASSERT_TRUE( !secrets[ i ].IsNULLOrEmpty() );
                if ( log.HasSubstr( secrets[ i ] ) >= 0 )
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Test: A secret of kind " << (UInt32) i << " is in the log" );
                ASSERT_TRUE( log.HasSubstr( secrets[ i ] ) < 0 );
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static AUTH::CAuthProof
AssertionProof( COAuth2Fixture& f, const CORE::CString& returnTo )
{GUCEF_TRACE;

    AUTH::COpenId2Assertion assertion;
    assertion.Load( f.fake.MakeAssertion( returnTo ) );
    return AUTH::CAuthProof( AUTH::CAuthProof::KindOpenId2Assertion, assertion.ToQueryString() );
}

/*-------------------------------------------------------------------------*/

static void
PerformOpenId2VerifyTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Test: A server checks an OpenID 2.0 assertion at the provider; each assertion once, with a fresh nonce and a loopback return address" )
        GUCEF_TESTFW_SKIP_IF( !Prerequisites(), "The json, rsa or ed25519 codec is not available" );
        try
        {
            COAuth2Fixture f;
            ASSERT_TRUE( f.ready );
            AUTH::CAuthProviderSettings openId2;
            openId2.authorizationEndpoint = f.fake.GetBaseUrl() + "/openid/login";
            ASSERT_TRUE( f.ConfigureOne( "testopenid", openId2 ) );
            const CORE::CString returnTo = "http://127.0.0.1:50123/callback?state=whatever";

            AUTH::CAuthResult result;
            AUTH::CAuthProof proof = AssertionProof( f, returnTo );
            ASSERT_TRUE( VerifyProof( f, "testopenid", proof, result ) );
            ASSERT_TRUE( "testopenid:4200000042" == result.accountRef.ToString() && "testopenid" == result.providerName && AUTH::CAuthProof::KindOpenId2Assertion == result.proof.kind );
            ASSERT_TRUE( 1 == f.fake.GetStats().openIdCheck );

            // the same proof again: seen before, the provider is not asked
            ASSERT_FALSE( VerifyProof( f, "testopenid", proof, result ) );
            ASSERT_TRUE( 1 == f.fake.GetStats().openIdCheck && !result.accountRef.IsValid() );
            // and the provider itself accepts an assertion once
            f.engine->GetVerifyNonceCache().Clear();
            ASSERT_FALSE( VerifyProof( f, "testopenid", proof, result ) );
            ASSERT_TRUE( 2 == f.fake.GetStats().openIdCheck );

            // other proofs: a proof of the wrong kind, a mangled one
            ASSERT_FALSE( VerifyProof( f, "testopenid", AUTH::CAuthProof( AUTH::CAuthProof::KindOidcIdToken, proof.data.Reveal() ), result ) );
            ASSERT_FALSE( VerifyProof( f, "testopenid", AUTH::CAuthProof( AUTH::CAuthProof::KindOpenId2Assertion, "openid.ns=x" ), result ) );
            ASSERT_FALSE( VerifyProof( f, "testopenid", AUTH::CAuthProof( AUTH::CAuthProof::KindOpenId2Assertion, CORE::CString::Empty ), result ) );
            UInt32 checks = f.fake.GetStats().openIdCheck;

            // a different return address: only a loopback address of the application path is acceptable
            ASSERT_FALSE( VerifyProof( f, "testopenid", AssertionProof( f, "http://evil.example/callback" ), result ) );
            ASSERT_FALSE( VerifyProof( f, "testopenid", AssertionProof( f, "https://127.0.0.1:50123/callback" ), result ) );
            ASSERT_FALSE( VerifyProof( f, "testopenid", AssertionProof( f, "http://127.0.0.1:50123/other" ), result ) );
            ASSERT_TRUE( checks == f.fake.GetStats().openIdCheck );
            ASSERT_TRUE( VerifyProof( f, "testopenid", AssertionProof( f, "http://localhost:50123/callback" ), result ) );

            // things the provider signed wrongly, or an old assertion
            CFakeBehavior behavior;
            behavior.openIdNonceAgeInSeconds = 3600;
            f.fake.SetBehavior( behavior );
            checks = f.fake.GetStats().openIdCheck;
            ASSERT_FALSE( VerifyProof( f, "testopenid", AssertionProof( f, returnTo ), result ) );
            behavior = CFakeBehavior();
            behavior.openIdOmitSignedReturnTo = true;
            f.fake.SetBehavior( behavior );
            ASSERT_FALSE( VerifyProof( f, "testopenid", AssertionProof( f, returnTo ), result ) );
            behavior = CFakeBehavior();
            behavior.openIdClaimedIdOverride = f.fake.GetBaseUrl() + "/openid/id/12AB";
            f.fake.SetBehavior( behavior );
            ASSERT_FALSE( VerifyProof( f, "testopenid", AssertionProof( f, returnTo ), result ) );
            behavior = CFakeBehavior();
            behavior.openIdOpEndpointOverride = "https://evil.example/openid/login";
            f.fake.SetBehavior( behavior );
            ASSERT_FALSE( VerifyProof( f, "testopenid", AssertionProof( f, returnTo ), result ) );
            ASSERT_TRUE( checks == f.fake.GetStats().openIdCheck );

            // the provider says no
            behavior = CFakeBehavior();
            behavior.openIdAlwaysInvalid = true;
            f.fake.SetBehavior( behavior );
            ASSERT_FALSE( VerifyProof( f, "testopenid", AssertionProof( f, returnTo ), result ) );
            ASSERT_TRUE( checks + 1 == f.fake.GetStats().openIdCheck );

            // the provider is not reachable: the assertion is not used up
            behavior = CFakeBehavior();
            behavior.openIdOpEndpointOverride = "http://127.0.0.1:1/openid/login";
            f.fake.SetBehavior( behavior );
            AUTH::CAuthProviderSettings unreachable;
            unreachable.authorizationEndpoint = "http://127.0.0.1:1/openid/login";
            ASSERT_TRUE( f.ConfigureOne( "testopenid", unreachable ) );
            UInt32 before = f.engine->GetVerifyNonceCache().GetCount();
            ASSERT_FALSE( VerifyProof( f, "testopenid", AssertionProof( f, returnTo ), result ) );
            ASSERT_TRUE( before == f.engine->GetVerifyNonceCache().GetCount() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A server configured with a relay accepts an OpenID 2.0 assertion made for the relay host only, in the form of the relay, and still asks the provider" )
        GUCEF_TESTFW_SKIP_IF( !Prerequisites(), "The json, rsa or ed25519 codec is not available" );
        try
        {
            COAuth2Fixture f;
            ASSERT_TRUE( f.ready );
            AUTH::CAuthProviderSettings openId2;
            openId2.authorizationEndpoint = f.fake.GetBaseUrl() + "/openid/login";
            openId2.parameters[ "relayUrl" ] = "https://login.relay-test.example/openid-return";
            ASSERT_TRUE( f.ConfigureOne( "testopenid", openId2 ) );
            const CORE::CString state = "AbCdEfGhIjKlMnOpQrStUvWxYz0123456789_-abcde";
            const CORE::CString relayed = "https://login.relay-test.example/openid-return?port=50123&s=" + state;

            AUTH::CAuthResult result;
            ASSERT_TRUE( VerifyProof( f, "testopenid", AssertionProof( f, relayed ), result ) );
            ASSERT_TRUE( "testopenid:4200000042" == result.accountRef.ToString() && 1 == f.fake.GetStats().openIdCheck );

            // anything else is refused before the provider is asked: the loopback form, another host, port or state form, other query
            const CORE::CString refused[] =
            {
                "http://127.0.0.1:50123/callback?state=" + state, "http://127.0.0.1:50123/callback", "https://evil.example/openid-return?port=50123&s=" + state,
                "https://login.relay-test.example.evil.example/openid-return?port=50123&s=" + state, "http://login.relay-test.example/openid-return?port=50123&s=" + state,
                "https://login.relay-test.example/other?port=50123&s=" + state, "https://login.relay-test.example/openid-return", "https://login.relay-test.example/openid-return?port=80&s=" + state,
                "https://login.relay-test.example/openid-return?port=50123&s=short", "https://login.relay-test.example/openid-return?port=50123&s=" + state + "&x=1",
                "https://login.relay-test.example/openid-return?s=" + state + "&port=50123"
            };
            UInt32 checks = f.fake.GetStats().openIdCheck;
            for ( size_t i=0; i<sizeof( refused ) / sizeof( refused[ 0 ] ); ++i )
            {
                if ( VerifyProof( f, "testopenid", AssertionProof( f, refused[ i ] ), result ) )
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Test: The proof was not refused: " << refused[ i ] );
                ASSERT_FALSE( VerifyProof( f, "testopenid", AssertionProof( f, refused[ i ] ), result ) );
            }
            ASSERT_TRUE( checks == f.fake.GetStats().openIdCheck );

            // the provider has the last word
            CFakeBehavior behavior;
            behavior.openIdAlwaysInvalid = true;
            f.fake.SetBehavior( behavior );
            ASSERT_FALSE( VerifyProof( f, "testopenid", AssertionProof( f, relayed ), result ) );
            ASSERT_TRUE( checks + 1 == f.fake.GetStats().openIdCheck );

            // a server without a relay keeps taking the loopback form, and takes nothing else
            f.fake.SetBehavior( CFakeBehavior() );
            AUTH::CAuthProviderSettings plain;
            plain.authorizationEndpoint = f.fake.GetBaseUrl() + "/openid/login";
            ASSERT_TRUE( f.ConfigureOne( "testopenid", plain ) );
            ASSERT_TRUE( VerifyProof( f, "testopenid", AssertionProof( f, "http://127.0.0.1:50123/callback?state=" + state ), result ) );
            ASSERT_FALSE( VerifyProof( f, "testopenid", AssertionProof( f, relayed ), result ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
PerformAccessTokenVerifyTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Test: A server checks a Facebook access token with debug_token: valid, for its app, not expired, and only with the app secret" )
        GUCEF_TESTFW_SKIP_IF( !Prerequisites(), "The json, rsa or ed25519 codec is not available" );
        try
        {
            COAuth2Fixture f;
            ASSERT_TRUE( f.ready );
            AUTH::CAuthProviderSettings facebook = f.LocalSettings();
            facebook.clientSecret.Set( "the-app-secret" );
            facebook.tokenVerifyEndpoint = f.fake.GetBaseUrl() + "/debug_token";
            facebook.userInfoEndpoint = f.fake.GetBaseUrl() + "/me";
            ASSERT_TRUE( f.ConfigureOne( "facebook", facebook ) );
            const Int64 later = (Int64) ( AUTH::GetEpochTimeInMs() / 1000 ) + 3600;
            const Int64 earlier = (Int64) ( AUTH::GetEpochTimeInMs() / 1000 ) - 3600;
            f.fake.RegisterVerifyToken( "fb-good", "fake-client-id", "fb-user-1", true, later );
            f.fake.RegisterVerifyToken( "fb-forever", "fake-client-id", "fb-user-2", true, 0 );
            f.fake.RegisterVerifyToken( "fb-invalid", "fake-client-id", "fb-user-3", false, later );
            f.fake.RegisterVerifyToken( "fb-expired", "fake-client-id", "fb-user-4", true, earlier );
            f.fake.RegisterVerifyToken( "fb-other-app", "someone-elses-app", "fb-user-5", true, later );

            AUTH::CAuthResult result;
            ASSERT_TRUE( VerifyProof( f, "facebook", AUTH::CAuthProof( AUTH::CAuthProof::KindOAuth2AccessToken, "fb-good" ), result ) );
            ASSERT_TRUE( "facebook:fb-user-1" == result.accountRef.ToString() && AUTH::CAuthProof::KindOAuth2AccessToken == result.proof.kind );
            ASSERT_TRUE( "Fb Player" == result.displayName && "https://pictures.invalid/fb.png" == result.avatarUrl );
            ASSERT_TRUE( VerifyProof( f, "facebook", AUTH::CAuthProof( AUTH::CAuthProof::KindOAuth2AccessToken, "fb-forever" ), result ) && "facebook:fb-user-2" == result.accountRef.ToString() );
            ASSERT_FALSE( VerifyProof( f, "facebook", AUTH::CAuthProof( AUTH::CAuthProof::KindOAuth2AccessToken, "fb-invalid" ), result ) );
            ASSERT_FALSE( VerifyProof( f, "facebook", AUTH::CAuthProof( AUTH::CAuthProof::KindOAuth2AccessToken, "fb-expired" ), result ) );
            ASSERT_FALSE( VerifyProof( f, "facebook", AUTH::CAuthProof( AUTH::CAuthProof::KindOAuth2AccessToken, "fb-other-app" ), result ) );
            ASSERT_FALSE( VerifyProof( f, "facebook", AUTH::CAuthProof( AUTH::CAuthProof::KindOAuth2AccessToken, "fb-unknown" ), result ) );
            ASSERT_FALSE( VerifyProof( f, "facebook", AUTH::CAuthProof( AUTH::CAuthProof::KindOAuth2AccessToken, CORE::CString::Empty ), result ) );
            ASSERT_FALSE( VerifyProof( f, "facebook", AUTH::CAuthProof( AUTH::CAuthProof::KindOidcIdToken, "fb-good" ), result ) );
            ASSERT_TRUE( 6 == f.fake.GetStats().verifyRequests );   // debug_token asked for each token that got that far

            // another app on the server than the one the token is for
            AUTH::CAuthProviderSettings otherApp = facebook;
            otherApp.clientId = "another-app";
            ASSERT_TRUE( f.ConfigureOne( "facebook", otherApp ) );
            ASSERT_FALSE( VerifyProof( f, "facebook", AUTH::CAuthProof( AUTH::CAuthProof::KindOAuth2AccessToken, "fb-good" ), result ) );

            // without the app secret nothing is asked: a token of any app would pass
            COAuth2Fixture g;
            ASSERT_TRUE( g.ready );
            AUTH::CAuthProviderSettings noSecret = g.LocalSettings();
            noSecret.tokenVerifyEndpoint = g.fake.GetBaseUrl() + "/debug_token";
            ASSERT_TRUE( g.ConfigureOne( "facebook", noSecret ) );
            g.fake.RegisterVerifyToken( "fb-good", "fake-client-id", "fb-user-1", true, later );
            ASSERT_FALSE( VerifyProof( g, "facebook", AUTH::CAuthProof( AUTH::CAuthProof::KindOAuth2AccessToken, "fb-good" ), result ) );
            ASSERT_TRUE( 0 == g.fake.GetStats().verifyRequests );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A server checks an Amazon access token: the audience is its own client, the account is that of the profile" )
        GUCEF_TESTFW_SKIP_IF( !Prerequisites(), "The json, rsa or ed25519 codec is not available" );
        try
        {
            COAuth2Fixture f;
            ASSERT_TRUE( f.ready );
            AUTH::CAuthProviderSettings amazon = f.LocalSettings();
            amazon.userInfoEndpoint = f.fake.GetBaseUrl() + "/profile";
            amazon.tokenVerifyEndpoint = f.fake.GetBaseUrl() + "/tokeninfo";
            ASSERT_TRUE( f.ConfigureOne( "amazon", amazon ) );
            f.fake.RegisterVerifyToken( "amz-good", "fake-client-id", "amzn1.account.AAA", true, 0 );
            f.fake.RegisterVerifyToken( "amz-other", "someone-elses-client", "amzn1.account.BBB", true, 0 );
            f.fake.RegisterVerifyToken( "amz-invalid", "fake-client-id", "amzn1.account.CCC", false, 0 );

            AUTH::CAuthResult result;
            ASSERT_TRUE( VerifyProof( f, "amazon", AUTH::CAuthProof( AUTH::CAuthProof::KindOAuth2AccessToken, "amz-good" ), result ) );
            ASSERT_TRUE( "amazon:amzn1.account.AAA" == result.accountRef.ToString() && "Amazon Player" == result.displayName );
            ASSERT_FALSE( VerifyProof( f, "amazon", AUTH::CAuthProof( AUTH::CAuthProof::KindOAuth2AccessToken, "amz-other" ), result ) );
            ASSERT_FALSE( VerifyProof( f, "amazon", AUTH::CAuthProof( AUTH::CAuthProof::KindOAuth2AccessToken, "amz-invalid" ), result ) );
            ASSERT_FALSE( VerifyProof( f, "amazon", AUTH::CAuthProof( AUTH::CAuthProof::KindOAuth2AccessToken, "amz-unknown" ), result ) );
            ASSERT_FALSE( VerifyProof( f, "amazon", AUTH::CAuthProof( AUTH::CAuthProof::KindOidcIdToken, "amz-good" ), result ) );
            ASSERT_FALSE( result.accountRef.IsValid() );

            // the proof of a login that was made through the engine verifies at the same provider
            f.fake.SetClientId( "fake-client-id" );
            CFakeBehavior behavior;
            behavior.issueIdToken = false;
            f.fake.SetBehavior( behavior );
            ASSERT_TRUE( f.Login( "amazon", AUTH::CAuthOptions(), result ) );
            f.fake.RegisterVerifyToken( result.accessToken.Reveal(), "fake-client-id", "sub-1000", true, 0 );
            AUTH::CAuthResult verified;
            ASSERT_TRUE( VerifyProof( f, "amazon", result.proof, verified ) && result.accountRef == verified.accountRef );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The proof of a login through the engine verifies at a server with the same settings, for an id token and for an assertion" )
        GUCEF_TESTFW_SKIP_IF( !Prerequisites(), "The json, rsa or ed25519 codec is not available" );
        try
        {
            COAuth2Fixture f;
            ASSERT_TRUE( f.ready );
            AUTH::CAuthSettings settings;
            settings.SetProviderSettings( "google", f.LocalSettings() );
            AUTH::CAuthProviderSettings openId2;
            openId2.authorizationEndpoint = f.fake.GetBaseUrl() + "/openid/login";
            settings.SetProviderSettings( "testopenid", openId2 );
            ASSERT_TRUE( f.Configure( settings ) );

            AUTH::CAuthResult login, verified;
            ASSERT_TRUE( f.Login( "google", AUTH::CAuthOptions(), login ) );
            ASSERT_TRUE( VerifyProof( f, "google", login.proof, verified ) && login.accountRef == verified.accountRef );

            // the assertion was used up by the provider when the login checked it: a login's proof cannot be presented again
            AUTH::CAuthResult openIdLogin;
            ASSERT_TRUE( f.Login( "testopenid", AUTH::CAuthOptions(), openIdLogin ) );
            ASSERT_FALSE( VerifyProof( f, "testopenid", openIdLogin.proof, verified ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
PerformOAuth2VerifyTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING OAuth2 verify TESTS" );
    GUCEF_TESTFW_SUITE_SCOPE( "OAuth2Verify" );

    PerformIdTokenVerifyTests();
    PerformMultipleAudienceTests();
    PerformOpenId2VerifyTests();
    PerformAccessTokenVerifyTests();
}

/*-------------------------------------------------------------------------*/
