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

#ifndef GUCEF_AUTH_H
#include "gucefAUTH.h"
#define GUCEF_AUTH_H
#endif /* GUCEF_AUTH_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestPump.h"
#include "TestRecorder.h"
#include "TestFlowEngine.h"
#include "TestOpenId2Backend.h"
#include "TestDescriptors.h"

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
HasFlag( UInt32 flags, UInt32 flag )
{GUCEF_TRACE;

    return 0 != ( flags & flag );
}

/*-------------------------------------------------------------------------*/

static bool
ContainsName( const CORE::CString::StringVector& names, const CORE::CString& name )
{GUCEF_TRACE;

    return AUTH::ContainsString( names, name );
}

/*-------------------------------------------------------------------------*/

static void
CollectTemplateNames( const AUTH::CAuthProviderDescriptor& d, CORE::CString::StringVector& names )
{GUCEF_TRACE;

    const CORE::CString* fields[] = { &d.discoveryUrl, &d.issuer, &d.authorizationEndpoint, &d.tokenEndpoint, &d.deviceAuthorizationEndpoint,
                                      &d.userInfoEndpoint, &d.jwksUri, &d.revocationEndpoint };
    for ( size_t i=0; i<sizeof( fields ) / sizeof( fields[ 0 ] ); ++i )
    {
        CORE::CString::StringVector found;
        AUTH::CAuthProviderDescriptor::GetTemplateNames( *fields[ i ], found );
        for ( size_t n=0; n<found.size(); ++n )
            names.push_back( found[ n ] );
    }
}

/*-------------------------------------------------------------------------*/

/** The profile of a second backend of the tests, which a test registers and takes away again */
class COtherOpenId2Profile : public CTestOpenId2Profile
{
    public:

    virtual const CORE::CString& GetProviderName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        static const CORE::CString name = "otheropenid";
        return name;
    }
};

/*-------------------------------------------------------------------------*/

void
PerformDescriptorTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING provider descriptor TESTS" );
    GUCEF_TESTFW_SUITE_SCOPE( "Descriptors" );

    GUCEF_TESTFW_TESTCASE( "Test: The built in descriptors are all there and well formed" )
        try
        {
            AUTH::CAuthProviderRegistry registry;
            registry.RegisterBuiltIns();
            ASSERT_TRUE( 7 == registry.GetCount() );
            CORE::CString::StringVector names;
            registry.GetNames( names );
            const char* expected[] = { "google", "microsoft", "facebook", "amazon", "aws-cognito", "aws-identitycenter", "epic" };
            for ( size_t i=0; i<sizeof( expected ) / sizeof( expected[ 0 ] ); ++i )
            {
                ASSERT_TRUE( ContainsName( names, expected[ i ] ) );
                AUTH::CAuthProviderDescriptor d;
                ASSERT_TRUE( registry.GetDescriptor( expected[ i ], d ) );
                ASSERT_TRUE( d.IsWellFormed() && d.name == expected[ i ] && !d.displayName.IsNULLOrEmpty() && !d.notes.IsNULLOrEmpty() );
                ASSERT_TRUE( AUTH::CAccountRef::IsValidProvider( d.accountProviderId ) );
                ASSERT_TRUE( HasFlag( d.capabilities, AUTH::AUTHCAP_BROWSER ) );
            }
            ASSERT_TRUE( registry.HasDescriptor( AUTH::CAuthProviderRegistry::GoogleName ) && !registry.HasDescriptor( "nobody" ) );
            AUTH::CAuthProviderDescriptor none;
            ASSERT_FALSE( registry.GetDescriptor( "nobody", none ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The id namespaces, scopes and capabilities of the built in descriptors" )
        try
        {
            AUTH::CAuthProviderRegistry registry;
            registry.RegisterBuiltIns();
            struct SExpect { const char* name; const char* ns; AUTH::TAuthIdScope scope; AUTH::TAuthProtocol protocol; bool needsClientId; bool device; bool refresh; bool serverVerifiable; };
            const SExpect table[] =
            {
                { "google",             "google",    AUTH::AUTHIDSCOPE_GLOBAL,           AUTH::AUTHPROTOCOL_OIDC,    true,  true,  true,  true  },
                { "microsoft",          "microsoft", AUTH::AUTHIDSCOPE_GLOBAL,           AUTH::AUTHPROTOCOL_OIDC,    true,  true,  true,  true  },
                { "facebook",           "facebook",  AUTH::AUTHIDSCOPE_PER_APP,          AUTH::AUTHPROTOCOL_OAUTH2,  true,  true,  false, true  },
                { "amazon",             "amazon",    AUTH::AUTHIDSCOPE_PER_APP,          AUTH::AUTHPROTOCOL_OAUTH2,  true,  false, true,  true  },
                { "aws-cognito",        "aws",       AUTH::AUTHIDSCOPE_PER_ORGANISATION, AUTH::AUTHPROTOCOL_OIDC,    true,  false, true,  true  },
                { "aws-identitycenter", "aws",       AUTH::AUTHIDSCOPE_PER_ORGANISATION, AUTH::AUTHPROTOCOL_OAUTH2,  true,  true,  true,  false },
                { "epic",               "epic",      AUTH::AUTHIDSCOPE_GLOBAL,           AUTH::AUTHPROTOCOL_OIDC,    true,  false, true,  true  }
            };
            for ( size_t i=0; i<sizeof( table ) / sizeof( table[ 0 ] ); ++i )
            {
                AUTH::CAuthProviderDescriptor d;
                ASSERT_TRUE( registry.GetDescriptor( table[ i ].name, d ) );
                ASSERT_TRUE( d.accountProviderId == table[ i ].ns && d.idScope == table[ i ].scope && d.protocol == table[ i ].protocol );
                ASSERT_TRUE( HasFlag( d.capabilities, AUTH::AUTHCAP_REQUIRES_CLIENT_ID ) == table[ i ].needsClientId );
                ASSERT_TRUE( HasFlag( d.capabilities, AUTH::AUTHCAP_DEVICE_CODE ) == table[ i ].device );
                ASSERT_TRUE( HasFlag( d.capabilities, AUTH::AUTHCAP_REFRESHABLE ) == table[ i ].refresh );
                ASSERT_TRUE( HasFlag( d.capabilities, AUTH::AUTHCAP_SERVER_VERIFIABLE ) == table[ i ].serverVerifiable );
                // a descriptor never says silent: platform sessions are plugins
                ASSERT_FALSE( HasFlag( d.capabilities, AUTH::AUTHCAP_SILENT ) );
            }

            // the things the application supplies are named
            AUTH::CAuthProviderDescriptor cognito;
            registry.GetDescriptor( "aws-cognito", cognito );
            ASSERT_TRUE( ContainsName( cognito.requiredParameters, "region" ) && ContainsName( cognito.requiredParameters, "poolId" ) && ContainsName( cognito.requiredParameters, "poolDomain" ) );
            AUTH::CAuthProviderDescriptor center;
            registry.GetDescriptor( "aws-identitycenter", center );
            ASSERT_TRUE( ContainsName( center.requiredParameters, "region" ) && ContainsName( center.requiredParameters, "instanceId" ) && center.jsonRequestBodies );
            AUTH::CAuthProviderDescriptor microsoft;
            registry.GetDescriptor( "microsoft", microsoft );
            ASSERT_TRUE( "common" == microsoft.defaultParameters[ "tenant" ] && microsoft.requiredParameters.empty() );
            // the built in descriptors are for the generic protocols: an OpenID 2.0 provider comes with the plugin of its backend
            for ( size_t i=0; i<sizeof( table ) / sizeof( table[ 0 ] ); ++i )
                ASSERT_FALSE( AUTH::AUTHPROTOCOL_OPENID2 == table[ i ].protocol );
            AUTH::CAuthProviderDescriptor openId2 = MakeTestOpenId2Descriptor();
            ASSERT_TRUE( openId2.IsWellFormed() && openId2.requiredParameters.empty() && !openId2.usesPkce && AUTH::AUTHPROTOCOL_OPENID2 == openId2.protocol );
            ASSERT_FALSE( HasFlag( openId2.capabilities, AUTH::AUTHCAP_REQUIRES_CLIENT_ID ) );
            AUTH::CAuthProviderDescriptor google;
            registry.GetDescriptor( "google", google );
            ASSERT_TRUE( "https://accounts.google.com" == google.issuer && google.usesPkce && google.clientSecretNeeded );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Every placeholder in the endpoints of a descriptor has a source" )
        try
        {
            AUTH::CAuthProviderRegistry registry;
            registry.RegisterBuiltIns();
            CORE::CString::StringVector names;
            registry.GetNames( names );
            for ( size_t i=0; i<names.size(); ++i )
            {
                AUTH::CAuthProviderDescriptor d;
                registry.GetDescriptor( names[ i ], d );
                CORE::CString::StringVector placeholders;
                CollectTemplateNames( d, placeholders );
                for ( size_t p=0; p<placeholders.size(); ++p )
                {
                    // supplied by the application, defaulted, or taken from the token being validated
                    bool known = ContainsName( d.requiredParameters, placeholders[ p ] ) || d.defaultParameters.find( placeholders[ p ] ) != d.defaultParameters.end() || "tid" == placeholders[ p ];
                    ASSERT_TRUE( known );
                }
                // with the required parameters filled in, the endpoints expand
                CORE::CString::StringMap values;
                for ( size_t r=0; r<d.requiredParameters.size(); ++r )
                    values[ d.requiredParameters[ r ] ] = "value" + CORE::UInt32ToString( (UInt32) r );
                values[ "tid" ] = "tenant-1";
                CORE::CString expanded;
                const CORE::CString* fields[] = { &d.discoveryUrl, &d.issuer, &d.authorizationEndpoint, &d.tokenEndpoint, &d.deviceAuthorizationEndpoint, &d.userInfoEndpoint, &d.jwksUri, &d.revocationEndpoint };
                for ( size_t f=0; f<sizeof( fields ) / sizeof( fields[ 0 ] ); ++f )
                {
                    ASSERT_TRUE( AUTH::CAuthProviderDescriptor::ExpandTemplate( *fields[ f ], values, &d.defaultParameters, false, expanded ) );
                    ASSERT_TRUE( expanded.HasChar( '{' ) < 0 && expanded.HasChar( '}' ) < 0 );
                }
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The subject of the account reference follows the claim mapping" )
        try
        {
            AUTH::CAuthProviderRegistry registry;
            registry.RegisterBuiltIns();
            CORE::CString::StringMap noParameters;
            AUTH::CAccountRef ref;
            AUTH::CAuthProviderDescriptor d;

            // Google: the sub claim
            registry.GetDescriptor( "google", d );
            CORE::CString::StringMap google;
            google[ "sub" ] = "112233445566778899";
            ASSERT_TRUE( d.MakeAccountRef( google, noParameters, ref ) && "google:112233445566778899" == ref.ToString() );

            // Microsoft: tenant id and object id
            registry.GetDescriptor( "microsoft", d );
            CORE::CString::StringMap microsoft;
            microsoft[ "tid" ] = "9188040d-6c67-4c5b-b112-36a304b66dad";
            microsoft[ "oid" ] = "00000000-0000-0000-0000-0001a2b3c4d5";
            microsoft[ "sub" ] = "pairwise-per-app-must-not-be-used";
            ASSERT_TRUE( d.MakeAccountRef( microsoft, noParameters, ref ) );
            ASSERT_TRUE( "microsoft:9188040d-6c67-4c5b-b112-36a304b66dad:00000000-0000-0000-0000-0001a2b3c4d5" == ref.ToString() );
            AUTH::CAccountRef parsed;
            ASSERT_TRUE( AUTH::CAccountRef::TryParse( ref.ToString(), parsed ) && parsed == ref );
            microsoft.erase( "oid" );
            ASSERT_FALSE( d.MakeAccountRef( microsoft, noParameters, ref ) && ref.IsValid() );

            // Cognito: the pool as a parameter, the sub as a claim
            registry.GetDescriptor( "aws-cognito", d );
            CORE::CString::StringMap cognitoClaims;
            cognitoClaims[ "sub" ] = "2f6c9d1e-5a3b-4c7d-8e9f-0a1b2c3d4e5f";
            CORE::CString::StringMap cognitoParameters;
            cognitoParameters[ "poolId" ] = "eu-west-1_AbCdEfGhI";
            ASSERT_TRUE( d.MakeAccountRef( cognitoClaims, cognitoParameters, ref ) );
            ASSERT_TRUE( "aws:eu-west-1_AbCdEfGhI:2f6c9d1e-5a3b-4c7d-8e9f-0a1b2c3d4e5f" == ref.ToString() );
            ASSERT_FALSE( d.MakeAccountRef( cognitoClaims, noParameters, ref ) );

            // IAM Identity Center: the identity store and the user
            registry.GetDescriptor( "aws-identitycenter", d );
            CORE::CString::StringMap centerClaims;
            centerClaims[ "userId" ] = "94482408-30a1-7035-4f1e-aaaaaaaaaaaa";
            CORE::CString::StringMap centerParameters;
            centerParameters[ "instanceId" ] = "d-9067012345";
            ASSERT_TRUE( d.MakeAccountRef( centerClaims, centerParameters, ref ) && "aws:d-9067012345:94482408-30a1-7035-4f1e-aaaaaaaaaaaa" == ref.ToString() );

            // the others take one claim
            registry.GetDescriptor( "facebook", d );
            CORE::CString::StringMap facebook;
            facebook[ "id" ] = "10158123456789012";
            ASSERT_TRUE( d.MakeAccountRef( facebook, noParameters, ref ) && "facebook:10158123456789012" == ref.ToString() );
            registry.GetDescriptor( "amazon", d );
            CORE::CString::StringMap amazon;
            amazon[ "user_id" ] = "amzn1.account.AF3XYZ7EXAMPLE";
            ASSERT_TRUE( d.MakeAccountRef( amazon, noParameters, ref ) && "amazon:amzn1.account.AF3XYZ7EXAMPLE" == ref.ToString() );
            registry.GetDescriptor( "epic", d );
            CORE::CString::StringMap epic;
            epic[ "sub" ] = "0123456789abcdef0123456789abcdef";
            ASSERT_TRUE( d.MakeAccountRef( epic, noParameters, ref ) && "epic:0123456789abcdef0123456789abcdef" == ref.ToString() );
            d = MakeTestOpenId2Descriptor();
            CORE::CString::StringMap openId2;
            openId2[ "testid" ] = "4200000042";
            ASSERT_TRUE( d.MakeAccountRef( openId2, noParameters, ref ) && "testopenid:4200000042" == ref.ToString() );
            ASSERT_FALSE( d.MakeAccountRef( noParameters, noParameters, ref ) );

            // a subject with characters that are not allowed is encoded, not refused or cut
            registry.GetDescriptor( "google", d );
            google[ "sub" ] = "weird sub/1";
            ASSERT_TRUE( d.MakeAccountRef( google, noParameters, ref ) && "google:weird%20sub%2F1" == ref.ToString() );
            google[ "sub" ] = "";
            ASSERT_FALSE( d.MakeAccountRef( google, noParameters, ref ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Templates expand, refuse what is missing and report their names" )
        try
        {
            CORE::CString::StringMap primary;
            primary[ "a" ] = "one";
            CORE::CString::StringMap fallback;
            fallback[ "b" ] = "two";
            fallback[ "a" ] = "shadowed";
            CORE::CString out;
            ASSERT_TRUE( AUTH::CAuthProviderDescriptor::ExpandTemplate( "x-{a}-{b}-y", primary, &fallback, false, out ) && "x-one-two-y" == out );
            ASSERT_TRUE( AUTH::CAuthProviderDescriptor::ExpandTemplate( "no placeholders", primary, GUCEF_NULL, false, out ) && "no placeholders" == out );
            ASSERT_FALSE( AUTH::CAuthProviderDescriptor::ExpandTemplate( "{c}", primary, &fallback, false, out ) );
            ASSERT_TRUE( out.IsNULLOrEmpty() );
            ASSERT_FALSE( AUTH::CAuthProviderDescriptor::ExpandTemplate( "{a", primary, &fallback, false, out ) );
            ASSERT_FALSE( AUTH::CAuthProviderDescriptor::ExpandTemplate( "a}", primary, &fallback, false, out ) );
            ASSERT_FALSE( AUTH::CAuthProviderDescriptor::ExpandTemplate( "{}", primary, &fallback, false, out ) );
            primary[ "e" ] = "";
            ASSERT_FALSE( AUTH::CAuthProviderDescriptor::ExpandTemplate( "{e}", primary, &fallback, false, out ) );
            primary[ "s" ] = "a b";
            ASSERT_TRUE( AUTH::CAuthProviderDescriptor::ExpandTemplate( "{s}:{a}", primary, GUCEF_NULL, true, out ) && "a%20b:one" == out );

            CORE::CString::StringVector found;
            AUTH::CAuthProviderDescriptor::GetTemplateNames( "https://{poolDomain}/x/{region}/{poolDomain}", found );
            ASSERT_TRUE( 3 == found.size() && "poolDomain" == found[ 0 ] && "region" == found[ 1 ] && "poolDomain" == found[ 2 ] );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Descriptors round trip through a data tree and the registry takes them from a config" )
        try
        {
            AUTH::CAuthProviderRegistry registry;
            registry.RegisterBuiltIns();
            CORE::CDataNode config( "ProviderDescriptors", GUCEF_DATATYPE_OBJECT );
            ASSERT_TRUE( registry.SaveConfig( config ) );
            UInt32 directChildren = 0;
            CORE::CDataNode::const_iterator child = config.ConstBegin();
            while ( child != config.ConstEnd() )
            {
                ++directChildren;
                ++child;
            }
            ASSERT_TRUE( 7 == directChildren );

            AUTH::CAuthProviderRegistry loaded;
            ASSERT_TRUE( loaded.LoadConfig( config ) && 7 == loaded.GetCount() );
            CORE::CString::StringVector names;
            registry.GetNames( names );
            for ( size_t i=0; i<names.size(); ++i )
            {
                AUTH::CAuthProviderDescriptor a;
                AUTH::CAuthProviderDescriptor b;
                ASSERT_TRUE( registry.GetDescriptor( names[ i ], a ) && loaded.GetDescriptor( names[ i ], b ) );
                ASSERT_TRUE( a.name == b.name && a.displayName == b.displayName && a.accountProviderId == b.accountProviderId && a.protocol == b.protocol );
                ASSERT_TRUE( a.idScope == b.idScope && a.capabilities == b.capabilities && a.discoveryUrl == b.discoveryUrl && a.issuer == b.issuer );
                ASSERT_TRUE( a.authorizationEndpoint == b.authorizationEndpoint && a.tokenEndpoint == b.tokenEndpoint && a.deviceAuthorizationEndpoint == b.deviceAuthorizationEndpoint );
                ASSERT_TRUE( a.userInfoEndpoint == b.userInfoEndpoint && a.jwksUri == b.jwksUri && a.revocationEndpoint == b.revocationEndpoint );
                ASSERT_TRUE( a.defaultScopes.size() == b.defaultScopes.size() && a.requiredParameters.size() == b.requiredParameters.size() && a.defaultParameters == b.defaultParameters );
                ASSERT_TRUE( a.subjectClaim == b.subjectClaim && a.subjectTemplate == b.subjectTemplate && a.displayNameClaim == b.displayNameClaim && a.avatarClaim == b.avatarClaim );
                ASSERT_TRUE( a.publicClient == b.publicClient && a.usesPkce == b.usesPkce && a.clientSecretNeeded == b.clientSecretNeeded && a.jsonRequestBodies == b.jsonRequestBodies && a.notes == b.notes );
            }

            // a provider of an application: all the standard needs is a record
            CORE::CDataNode custom( "ProviderDescriptors", GUCEF_DATATYPE_OBJECT );
            CORE::CDataNode* mine = custom.AddChild( "keycloak", GUCEF_DATATYPE_OBJECT );
            mine->SetAttribute( "displayName", CORE::CString( "Our Keycloak" ) );
            mine->SetAttribute( "accountProviderId", CORE::CString( "ourkeycloak" ) );
            mine->SetAttribute( "protocol", CORE::CString( "oidc" ) );
            mine->SetAttribute( "discoveryUrl", CORE::CString( "https://sso.example.invalid/realms/game/.well-known/openid-configuration" ) );
            mine->SetAttribute( "idScope", CORE::CString( "organisation" ) );
            CORE::CDataNode* caps = mine->AddChild( "capabilities", GUCEF_DATATYPE_ARRAY );
            caps->AddValueAsChild( "browser" );
            caps->AddValueAsChild( "refreshable" );
            caps->AddValueAsChild( "requires-client-id" );
            // and a change to a built in one
            CORE::CDataNode* google = custom.AddChild( "google", GUCEF_DATATYPE_OBJECT );
            google->SetAttribute( "displayName", CORE::CString( "Google Sign-In" ) );
            google->SetAttribute( "accountProviderId", CORE::CString( "google" ) );
            google->SetAttribute( "discoveryUrl", CORE::CString( "https://accounts.google.com/.well-known/openid-configuration" ) );
            // and one that is no use
            CORE::CDataNode* broken = custom.AddChild( "broken", GUCEF_DATATYPE_OBJECT );
            broken->SetAttribute( "displayName", CORE::CString( "Nothing to go on" ) );

            ASSERT_FALSE( loaded.LoadConfig( custom ) );    // the broken one is reported
            ASSERT_TRUE( 8 == loaded.GetCount() && loaded.HasDescriptor( "keycloak" ) && !loaded.HasDescriptor( "broken" ) );
            AUTH::CAuthProviderDescriptor keycloak;
            ASSERT_TRUE( loaded.GetDescriptor( "keycloak", keycloak ) );
            ASSERT_TRUE( "ourkeycloak" == keycloak.accountProviderId && AUTH::AUTHIDSCOPE_PER_ORGANISATION == keycloak.idScope && HasFlag( keycloak.capabilities, AUTH::AUTHCAP_REFRESHABLE ) && !HasFlag( keycloak.capabilities, AUTH::AUTHCAP_DEVICE_CODE ) );
            AUTH::CAuthProviderDescriptor changed;
            ASSERT_TRUE( loaded.GetDescriptor( "google", changed ) && "Google Sign-In" == changed.displayName );

            ASSERT_TRUE( loaded.UnregisterDescriptor( "keycloak" ) && !loaded.UnregisterDescriptor( "keycloak" ) && 7 == loaded.GetCount() );
            AUTH::CAuthProviderDescriptor refused;
            ASSERT_FALSE( loaded.RegisterDescriptor( refused ) );
            loaded.Clear();
            ASSERT_TRUE( 0 == loaded.GetCount() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Login hints are data of the descriptor: only Google, Microsoft and Cognito take them, the record round trips and defaults to none" )
        try
        {
            AUTH::CAuthProviderRegistry registry;
            registry.RegisterBuiltIns();
            CORE::CString::StringVector names;
            registry.GetNames( names );
            ASSERT_TRUE( 7 == names.size() );
            for ( size_t i=0; i<names.size(); ++i )
            {
                AUTH::CAuthProviderDescriptor d;
                ASSERT_TRUE( registry.GetDescriptor( names[ i ], d ) );
                const bool expected = "google" == names[ i ] || "microsoft" == names[ i ] || "aws-cognito" == names[ i ];
                ASSERT_TRUE( expected == d.supportsLoginHint );
                ASSERT_TRUE( expected == !d.loginHintClaims.empty() );
                ASSERT_TRUE( "login_hint" == d.loginHintParameter );
            }
            AUTH::CAuthProviderDescriptor google, microsoft, openId2 = MakeTestOpenId2Descriptor();
            ASSERT_TRUE( registry.GetDescriptor( "google", google ) && registry.GetDescriptor( "microsoft", microsoft ) );
            ASSERT_TRUE( "email" == google.loginHintClaims[ 0 ] && AUTH::ContainsString( google.defaultScopes, "email" ) );
            ASSERT_TRUE( "preferred_username" == microsoft.loginHintClaims[ 0 ] );
            ASSERT_FALSE( openId2.supportsLoginHint );

            // a record that says nothing about it takes none
            AUTH::CAuthProviderDescriptor empty;
            ASSERT_FALSE( empty.supportsLoginHint );

            // it round trips through a data tree, with the name of the parameter
            AUTH::CAuthProviderDescriptor custom = google;
            custom.loginHintParameter = "username";
            custom.loginHintClaims.clear();
            custom.loginHintClaims.push_back( "upn" );
            CORE::CDataNode node( "custom", GUCEF_DATATYPE_OBJECT );
            ASSERT_TRUE( custom.Serialize( node, CORE::CDataNodeSerializableSettings() ) );
            AUTH::CAuthProviderDescriptor back;
            ASSERT_TRUE( back.Deserialize( node, CORE::CDataNodeSerializableSettings() ) );
            ASSERT_TRUE( back.supportsLoginHint && "username" == back.loginHintParameter && 1 == back.loginHintClaims.size() && "upn" == back.loginHintClaims[ 0 ] );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The global registry has the built in providers, unavailable without an engine" )
        try
        {
            AUTH::CAuthGlobal* global = AUTH::CAuthGlobal::Instance();
            ASSERT_TRUE( GUCEF_NULL != global && 8 <= global->GetDescriptorRegistry().GetCount() );

            // The module registers its engine; the providers are looked at without one
            ASSERT_TRUE( GUCEF_NULL != global->GetOAuth2Engine() && global->GetFlowEngine() == global->GetOAuth2Engine() );
            global->SetFlowEngine( GUCEF_NULL );
            ASSERT_TRUE( GUCEF_NULL == global->GetFlowEngine() );

            const char* expected[] = { "google", "microsoft", "facebook", "amazon", "aws-cognito", "aws-identitycenter", "epic", "testopenid" };
            for ( size_t i=0; i<sizeof( expected ) / sizeof( expected[ 0 ] ); ++i )
            {
                ASSERT_TRUE( global->GetProviderFactory().IsConstructible( expected[ i ] ) );
                AUTH::CIAuthProviderPtr provider = global->GetProviderFactory().Create( expected[ i ] );
                ASSERT_FALSE( provider.IsNULL() );
                if ( provider.IsNULL() )
                    continue;
                ASSERT_TRUE( provider->GetProviderName() == expected[ i ] );
                ASSERT_TRUE( 0 != provider->GetCapabilities() );
                ASSERT_FALSE( provider->IsAvailable() );
                CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
                ASSERT_FALSE( provider->BeginLogin( AUTH::CAuthOptions(), future ) );
                ASSERT_TRUE( future.HasNoFuture() );
                ASSERT_FALSE( provider->TrySilentLogin( future ) );
                ASSERT_FALSE( provider->Refresh( AUTH::CAccountRef( "google", "1" ), AUTH::CSecretString( "r" ), future ) );
                AUTH::CAuthResult result;
                ASSERT_FALSE( provider->Verify( AUTH::CAuthProof( "oidc-id-token", "a.b.c" ), result ) );
            }

            // the manager of the process sees them as providers, none of them offered
            AUTH::CAuthManager& manager = global->GetManager();
            manager.RefreshProviders();
            CORE::CString::StringVector providerNames;
            manager.GetProviderNames( providerNames );
            ASSERT_TRUE( providerNames.size() >= 8 );
            CORE::CString::StringVector offered;
            manager.GetOfferedProviders( offered );
            ASSERT_TRUE( offered.empty() );

            global->RegisterBuiltInEngine();
            ASSERT_TRUE( global->GetFlowEngine() == global->GetOAuth2Engine() );
        }
        catch ( ... )
        {
            AUTH::CAuthGlobal::Instance()->RegisterBuiltInEngine();
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A descriptor driven provider works through the engine it is given" )
        try
        {
            AUTH::CAuthProviderRegistry registry;
            registry.RegisterBuiltIns();
            AUTH::CAuthProviderDescriptor google;
            AUTH::CAuthProviderDescriptor openId2 = MakeTestOpenId2Descriptor();
            AUTH::CAuthProviderDescriptor microsoft;
            registry.GetDescriptor( "google", google );
            registry.GetDescriptor( "microsoft", microsoft );

            CTestFlowEngine engine;
            AUTH::CDescriptorAuthProvider provider( google );
            provider.SetFlowEngine( &engine );
            CAuthRecorder recorder;
            recorder.SubscribeTo( &provider );

            // no client id, no provider
            ASSERT_FALSE( provider.IsAvailable() );
            CORE::CDataNode config( "Provider", GUCEF_DATATYPE_OBJECT );
            config.SetAttribute( "clientId", CORE::CString( "google-client" ) );
            ASSERT_TRUE( provider.LoadConfig( config ) && provider.IsAvailable() );
            ASSERT_TRUE( "google-client" == provider.GetSettings().clientId );
            CORE::CDataNode saved( "Provider", GUCEF_DATATYPE_OBJECT );
            ASSERT_TRUE( provider.SaveConfig( saved ) && "google-client" == saved.GetAttributeValueOrChildValueByName( "clientId" ).AsString() );

            // an engine that does not support what is configured
            engine.supported = false;
            ASSERT_FALSE( provider.IsAvailable() );
            CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            ASSERT_FALSE( provider.BeginLogin( AUTH::CAuthOptions(), future ) && engine.interactiveCount > 0 );
            ASSERT_TRUE( 0 == engine.interactiveCount );
            engine.supported = true;

            // the login: the engine reports through the provider, the events follow with the next pulse
            AUTH::CAuthOptions options;
            options.scopes.push_back( "email" );
            ASSERT_TRUE( provider.BeginLogin( options, future ) );
            ASSERT_TRUE( 1 == engine.interactiveCount && "google-client" == engine.lastClientIdSeen && "email" == engine.lastScopeSeen );
            AUTH::CAuthResult result;
            ASSERT_TRUE( AUTH::CAuthResult::TryGetFromFuture( future, result ) );
            ASSERT_TRUE( "google:112233" == result.accountRef.ToString() && "google" == result.providerName && "Engine Player" == result.displayName );
            ASSERT_TRUE( PumpUntilRecorded( recorder, CAuthRecorder::KIND_SUCCEEDED, 1, 2000 ) );
            ASSERT_TRUE( 1 == recorder.Count( CAuthRecorder::KIND_SUCCEEDED ) && 1 == recorder.Count( CAuthRecorder::KIND_ACTION ) );
            const CAuthRecorder::SRecord* action = recorder.Last( CAuthRecorder::KIND_ACTION );
            ASSERT_TRUE( GUCEF_NULL != action && AUTH::AUTHACTION_SHOW_DEVICE_CODE == action->action.kind && "ABCD-1234" == action->action.userCode && "google" == action->action.providerName );

            // a failure of the engine ends as a failure with its reason
            engine.fail = true;
            engine.failWith = AUTH::AUTHFAIL_TIMEOUT;
            ASSERT_TRUE( provider.BeginLogin( options, future ) );
            ASSERT_TRUE( future.IsReady() && future.HasNoFuture() );
            ASSERT_TRUE( PumpUntilRecorded( recorder, CAuthRecorder::KIND_FAILED, 1, 2000 ) );
            ASSERT_TRUE( AUTH::AUTHFAIL_TIMEOUT == recorder.Last( CAuthRecorder::KIND_FAILED )->failure.reason && "google" == recorder.Last( CAuthRecorder::KIND_FAILED )->failure.providerName );
            engine.fail = false;

            // refresh, logout and verify go to the engine when the provider can
            ASSERT_TRUE( provider.Refresh( AUTH::CAccountRef( "google", "112233" ), AUTH::CSecretString( "the-refresh-token" ), future ) );
            ASSERT_TRUE( 1 == engine.refreshCount && "the-refresh-token" == engine.lastRefreshToken );
            ASSERT_FALSE( provider.Refresh( AUTH::CAccountRef( "google", "112233" ), AUTH::CSecretString(), future ) );
            ASSERT_TRUE( provider.Logout( AUTH::CAccountRef( "google", "112233" ), AUTH::CSecretString( "the-refresh-token" ) ) );
            ASSERT_TRUE( 1 == engine.revokeCount );
            AUTH::CAuthResult verified;
            ASSERT_TRUE( provider.Verify( AUTH::CAuthProof( "oidc-id-token", "a.b.c" ), verified ) );
            ASSERT_TRUE( 1 == engine.verifyCount && "google:verified-subject" == verified.accountRef.ToString() && "google" == verified.providerName );
            ASSERT_FALSE( provider.Verify( AUTH::CAuthProof(), verified ) );

            // a provider without refresh tokens (OpenID 2.0) does not refresh, one without a client id need is available with the engine alone
            AUTH::CDescriptorAuthProvider openId2Provider( openId2 );
            openId2Provider.SetFlowEngine( &engine );
            ASSERT_TRUE( openId2Provider.IsAvailable() );
            ASSERT_FALSE( openId2Provider.Refresh( AUTH::CAccountRef( "testopenid", "1" ), AUTH::CSecretString( "r" ), future ) );
            ASSERT_TRUE( 1 == engine.refreshCount );
            ASSERT_TRUE( openId2Provider.Logout( AUTH::CAccountRef( "testopenid", "1" ), AUTH::CSecretString( "r" ) ) && 1 == engine.revokeCount );

            // the account of Microsoft is made from two claims by the descriptor
            AUTH::CDescriptorAuthProvider microsoftProvider( microsoft );
            microsoftProvider.SetFlowEngine( &engine );
            CORE::CDataNode msConfig( "Provider", GUCEF_DATATYPE_OBJECT );
            msConfig.SetAttribute( "clientId", CORE::CString( "ms-client" ) );
            ASSERT_TRUE( microsoftProvider.LoadConfig( msConfig ) && microsoftProvider.IsAvailable() );
            ASSERT_TRUE( microsoftProvider.BeginLogin( options, future ) );
            AUTH::CAuthResult msResult;
            ASSERT_TRUE( AUTH::CAuthResult::TryGetFromFuture( future, msResult ) && "microsoft:tenant-1:object-9" == msResult.accountRef.ToString() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A backend registers its descriptor, its profile and its provider when it loads and takes them away when it unloads; a former name of the provider keeps the settings of the application working" )
        try
        {
            AUTH::CAuthGlobal* global = AUTH::CAuthGlobal::Instance();
            AUTH::CAuthProviderFactory& factory = global->GetProviderFactory();
            AUTH::CAuthProviderRegistry& registry = global->GetDescriptorRegistry();
            AUTH::COpenId2ProfileRegistry& profiles = global->GetOpenId2Profiles();
            COtherOpenId2Profile profile;

            // before the plugin loads there is nothing of the backend anywhere
            ASSERT_FALSE( factory.IsConstructible( "otheropenid" ) || registry.HasDescriptor( "otheropenid" ) || GUCEF_NULL != profiles.Find( "otheropenid" ) );
            ASSERT_TRUE( "Otheropenid" == AUTH::GetProviderDisplayName( "otheropenid" ) );

            // the application has settings that were made when the provider had another name
            AUTH::CAuthManager manager;
            AUTH::CAuthSettings settings;
            AUTH::CAuthProviderSettings relayed;
            relayed.parameters[ "relayUrl" ] = "https://login.example.com/openid-return";
            relayed.parameters[ "other" ] = "kept";
            settings.SetProviderSettings( "oldother", relayed );
            settings.enabledProviders.push_back( "oldother" );
            settings.enabledProviders.push_back( "google" );
            settings.lastUsedProviders.push_back( "oldother" );
            manager.SetSettings( settings );
            AUTH::CAuthProviderSettings found;
            ASSERT_TRUE( manager.GetSettings().GetProviderSettings( "oldother", found ) );      // nobody knows it by another name yet

            // the plugin loads
            AUTH::CAuthProviderDescriptor descriptor = MakeTestOpenId2Descriptor();
            descriptor.name = "otheropenid";
            descriptor.accountProviderId = "otheropenid";
            descriptor.displayName = "Other OpenId";
            descriptor.formerNames.push_back( "oldother" );
            ASSERT_TRUE( registry.RegisterDescriptor( descriptor ) );
            ASSERT_TRUE( profiles.Register( &profile ) );
            ASSERT_TRUE( global->RegisterDescriptorProvider( "otheropenid" ) );
            ASSERT_FALSE( global->RegisterDescriptorProvider( "otheropenid" ) );     // once
            ASSERT_TRUE( factory.IsConstructible( "otheropenid" ) );
            ASSERT_TRUE( manager.RefreshProviders() >= 1 );
            AUTH::CIAuthProvider* provider = manager.GetProvider( "otheropenid" );
            ASSERT_TRUE( GUCEF_NULL != provider && "otheropenid" == provider->GetProviderName() );
            ASSERT_TRUE( "Other OpenId" == AUTH::GetProviderDisplayName( "otheropenid" ) && "Other OpenId" == AUTH::GetAccountProviderDisplayName( "otheropenid" ) );
            ASSERT_TRUE( 0 != ( provider->GetCapabilities() & AUTH::AUTHCAP_BROWSER ) );

            // the settings are the settings of the provider under its name now, and nothing is left under the old one
            ASSERT_TRUE( manager.GetSettings().GetProviderSettings( "otheropenid", found ) );
            ASSERT_TRUE( "https://login.example.com/openid-return" == found.GetParameter( "relayUrl" ) && "kept" == found.GetParameter( "other" ) );
            ASSERT_FALSE( manager.GetSettings().GetProviderSettings( "oldother", found ) );
            ASSERT_TRUE( 2 == manager.GetSettings().enabledProviders.size() && "otheropenid" == manager.GetSettings().enabledProviders[ 0 ] && "google" == manager.GetSettings().enabledProviders[ 1 ] );
            ASSERT_TRUE( 1 == manager.GetSettings().lastUsedProviders.size() && "otheropenid" == manager.GetSettings().lastUsedProviders[ 0 ] );

            // the former names are part of what the descriptor says about itself
            CORE::CDataNode node( "descriptor", GUCEF_DATATYPE_OBJECT );
            ASSERT_TRUE( descriptor.Serialize( node, CORE::CDataNodeSerializableSettings() ) );
            AUTH::CAuthProviderDescriptor loaded;
            ASSERT_TRUE( loaded.Deserialize( node, CORE::CDataNodeSerializableSettings() ) && 1 == loaded.formerNames.size() && "oldother" == loaded.formerNames[ 0 ] );

            // the plugin unloads: the provider goes first, then what it was made from
            ASSERT_TRUE( manager.RemoveProvider( "otheropenid" ) );
            ASSERT_TRUE( global->UnregisterDescriptorProvider( "otheropenid" ) );
            ASSERT_FALSE( global->UnregisterDescriptorProvider( "otheropenid" ) );
            ASSERT_TRUE( profiles.Unregister( "otheropenid" ) );
            ASSERT_TRUE( registry.UnregisterDescriptor( "otheropenid" ) );
            ASSERT_FALSE( factory.IsConstructible( "otheropenid" ) || registry.HasDescriptor( "otheropenid" ) || GUCEF_NULL != profiles.Find( "otheropenid" ) );
            ASSERT_TRUE( GUCEF_NULL == manager.GetProvider( "otheropenid" ) && 0 == manager.RefreshProviders() );
            ASSERT_TRUE( "Otheropenid" == AUTH::GetProviderDisplayName( "otheropenid" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
