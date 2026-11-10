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

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CDSTORECODECREGISTRY_H
#include "CDStoreCodecRegistry.h"
#define GUCEF_CORE_CDSTORECODECREGISTRY_H
#endif /* GUCEF_CORE_CDSTORECODECREGISTRY_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#include "CDynamicBufferAccess.h"
#define GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#endif /* GUCEF_CORE_CDYNAMICBUFFERACCESS_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_AUTH_H
#include "gucefAUTH.h"
#define GUCEF_AUTH_H
#endif /* GUCEF_AUTH_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestPump.h"
#include "TestSettings.h"

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

static AUTH::CAuthSettings
MakeFilledSettings( void )
{GUCEF_TRACE;

    AUTH::CAuthSettings settings;
    settings.autoLogin = false;
    settings.appName = "ReduxTest";
    settings.credentialDirectory = "$CURWORKDIR$/authsettings/cred";
    settings.linkStorePath = "$CURWORKDIR$/authsettings/links.json";
    settings.autoLoginStepTimeoutInMs = 4321;
    settings.loginDeadlineSlackInMs = 777;
    settings.enabledProviders.push_back( "google" );
    settings.enabledProviders.push_back( "testopenid" );
    settings.lastUsedProviders.push_back( "testopenid" );
    settings.lastUsedProviders.push_back( "google" );

    AUTH::CAuthProviderSettings google;
    google.clientId = "1234-abcd.apps.example.invalid";
    google.clientSecret.Set( "not-confidential-but-kept" );
    google.scopes.push_back( "openid" );
    google.scopes.push_back( "email" );
    google.audiences.push_back( "desktop.example.invalid" );
    google.audiences.push_back( "android.example.invalid" );
    google.authorizationEndpoint = "https://auth.example.invalid/authorize";
    google.tokenEndpoint = "https://auth.example.invalid/token";
    google.redirectMode = "loopback";
    google.loopbackPort = 49152;
    google.loopbackPath = "/oauth/cb";
    google.hasIdScope = true;
    google.idScope = AUTH::AUTHIDSCOPE_PER_ORGANISATION;
    settings.SetProviderSettings( "google", google );

    AUTH::CAuthProviderSettings cognito;
    cognito.clientId = "cognito-client";
    cognito.redirectMode = "custom-scheme";
    cognito.redirectUri = "myapp://callback";
    cognito.parameters[ "region" ] = "eu-west-1";
    cognito.parameters[ "poolId" ] = "eu-west-1_AbCdEfGhI";
    cognito.parameters[ "poolDomain" ] = "myapp.auth.eu-west-1.amazoncognito.com";
    settings.SetProviderSettings( "aws-cognito", cognito );
    return settings;
}

/*-------------------------------------------------------------------------*/

static bool
SameSettings( const AUTH::CAuthSettings& a, const AUTH::CAuthSettings& b )
{GUCEF_TRACE;

    if ( a.autoLogin != b.autoLogin || a.appName != b.appName || a.credentialDirectory != b.credentialDirectory ||
         a.linkStorePath != b.linkStorePath || a.autoLoginStepTimeoutInMs != b.autoLoginStepTimeoutInMs || a.loginDeadlineSlackInMs != b.loginDeadlineSlackInMs ||
         a.enabledProviders.size() != b.enabledProviders.size() || a.lastUsedProviders.size() != b.lastUsedProviders.size() ||
         a.providers.size() != b.providers.size() )
        return false;
    for ( size_t i=0; i<a.enabledProviders.size(); ++i )
        if ( a.enabledProviders[ i ] != b.enabledProviders[ i ] )
            return false;
    for ( size_t i=0; i<a.lastUsedProviders.size(); ++i )
        if ( a.lastUsedProviders[ i ] != b.lastUsedProviders[ i ] )
            return false;
    AUTH::CAuthSettings::TProviderSettingsMap::const_iterator i = a.providers.begin();
    while ( i != a.providers.end() )
    {
        AUTH::CAuthSettings::TProviderSettingsMap::const_iterator j = b.providers.find( (*i).first );
        if ( j == b.providers.end() )
            return false;
        const AUTH::CAuthProviderSettings& x = (*i).second;
        const AUTH::CAuthProviderSettings& y = (*j).second;
        if ( x.clientId != y.clientId || !( x.clientSecret == y.clientSecret ) || x.scopes.size() != y.scopes.size() || x.audiences != y.audiences ||
             x.authorizationEndpoint != y.authorizationEndpoint || x.tokenEndpoint != y.tokenEndpoint ||
             x.redirectMode != y.redirectMode || x.loopbackPort != y.loopbackPort || x.loopbackPath != y.loopbackPath ||
             x.redirectUri != y.redirectUri || x.hasIdScope != y.hasIdScope || ( x.hasIdScope && x.idScope != y.idScope ) ||
             x.parameters != y.parameters )
            return false;
        ++i;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

/** Reads json text into a data tree with the json codec */
static bool
ParseJson( const CORE::CString& text, CORE::CDataNode& tree )
{GUCEF_TRACE;

    CORE::CDStoreCodecRegistry::TDStoreCodecPtr codec;
    if ( !CORE::CCoreGlobal::Instance()->GetDStoreCodecRegistry().TryGetJsonCodec( codec ) )
        return false;
    CORE::CDynamicBuffer buffer;
    buffer.CopyFrom( AUTH::GetByteLength( text ), text.C_String() );
    CORE::CDynamicBufferAccess access( buffer );
    return codec->BuildDataTree( &tree, &access );
}

/*-------------------------------------------------------------------------*/

void
PerformSettingsTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING auth settings TESTS" );
    GUCEF_TESTFW_SUITE_SCOPE( "AuthSettings" );

    GUCEF_TESTFW_TESTCASE( "Test: Defaults of the settings" )
        try
        {
            AUTH::CAuthSettings settings;
            ASSERT_TRUE( settings.autoLogin && "GUCEF" == settings.appName );
            ASSERT_TRUE( AUTH::CAuthSettings::DefaultAutoLoginStepTimeoutInMs == settings.autoLoginStepTimeoutInMs );
            ASSERT_TRUE( AUTH::CAuthSettings::DefaultLoginDeadlineSlackInMs == settings.loginDeadlineSlackInMs );
            ASSERT_TRUE( settings.enabledProviders.empty() && settings.lastUsedProviders.empty() && settings.providers.empty() );
            // everything is enabled when no list is given
            ASSERT_TRUE( settings.IsProviderEnabled( "google" ) && settings.IsProviderEnabled( "anything" ) );

            CORE::CString credentialDirectory = settings.GetCredentialDirectory();
            ASSERT_TRUE( credentialDirectory.HasSubstr( "GUCEF" ) >= 0 && credentialDirectory.HasSubstr( "Auth" ) >= 0 );
            CORE::CString linkPath = settings.GetLinkStorePath();
            ASSERT_TRUE( linkPath.HasSubstr( "links.json" ) >= 0 && linkPath.HasSubstr( "Auth" ) >= 0 );
            // the link file is next to the credentials by default
            ASSERT_TRUE( CORE::StripFilename( linkPath ) == credentialDirectory );

            AUTH::CAuthProviderSettings providerSettings;
            ASSERT_TRUE( !providerSettings.HasClientId() && "loopback" == providerSettings.redirectMode && 0 == providerSettings.loopbackPort );
            ASSERT_TRUE( "/callback" == providerSettings.loopbackPath && !providerSettings.hasIdScope );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The settings round trip through a data tree" )
        try
        {
            AUTH::CAuthSettings settings = MakeFilledSettings();
            CORE::CDataNode node( "Auth", GUCEF_DATATYPE_OBJECT );
            ASSERT_TRUE( settings.SaveConfig( node ) );

            AUTH::CAuthSettings loaded;
            ASSERT_TRUE( loaded.LoadConfig( node ) );
            ASSERT_TRUE( SameSettings( settings, loaded ) );
            AUTH::CAuthProviderSettings cognito;
            ASSERT_TRUE( loaded.GetProviderSettings( "aws-cognito", cognito ) );
            ASSERT_TRUE( "eu-west-1_AbCdEfGhI" == cognito.GetParameter( "poolId" ) && cognito.GetParameter( "tenant" ).IsNULLOrEmpty() );
            AUTH::CAuthProviderSettings google;
            ASSERT_TRUE( loaded.GetProviderSettings( "google", google ) && google.hasIdScope && AUTH::AUTHIDSCOPE_PER_ORGANISATION == google.idScope );
            ASSERT_FALSE( loaded.GetProviderSettings( "nobody", google ) );

            // the section can also be found as a child of the application config
            CORE::CDataNode appConfig( "App", GUCEF_DATATYPE_OBJECT );
            CORE::CDataNode* section = appConfig.AddChild( "Auth", GUCEF_DATATYPE_OBJECT );
            ASSERT_TRUE( settings.SaveConfig( *section ) );
            AUTH::CAuthSettings fromApp;
            ASSERT_TRUE( fromApp.LoadConfig( appConfig ) );
            ASSERT_TRUE( SameSettings( settings, fromApp ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The settings round trip through a json file" )
        GUCEF_TESTFW_SKIP_IF( !JsonCodecAvailable(), "The json codec plugin is not available" );
        try
        {
            AUTH::CAuthSettings settings = MakeFilledSettings();
            CORE::CDataNode node( "Auth", GUCEF_DATATYPE_OBJECT );
            ASSERT_TRUE( settings.SaveConfig( node ) );

            CORE::CDStoreCodecRegistry::TDStoreCodecPtr codec;
            ASSERT_TRUE( CORE::CCoreGlobal::Instance()->GetDStoreCodecRegistry().TryGetJsonCodec( codec ) );
            CORE::CDynamicBuffer buffer;
            CORE::CDynamicBufferAccess access( buffer );
            ASSERT_TRUE( codec->StoreDataTree( &node, &access ) );
            ASSERT_TRUE( buffer.GetDataSize() > 0 );

            CORE::CDynamicBufferAccess readAccess( buffer );
            CORE::CDataNode parsed;
            ASSERT_TRUE( codec->BuildDataTree( &parsed, &readAccess ) );
            AUTH::CAuthSettings loaded;
            ASSERT_TRUE( loaded.LoadConfig( parsed ) );
            ASSERT_TRUE( SameSettings( settings, loaded ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A provider is offered when it is enabled and has a client id where it needs one" )
        try
        {
            AUTH::CAuthSettings settings;
            AUTH::CAuthProviderSettings withId;
            withId.clientId = "client";
            settings.SetProviderSettings( "google", withId );
            settings.SetProviderSettings( "microsoft", AUTH::CAuthProviderSettings() );   // an empty client id

            ASSERT_TRUE( settings.IsProviderOffered( "google", true ) );
            ASSERT_FALSE( settings.IsProviderOffered( "microsoft", true ) );
            ASSERT_FALSE( settings.IsProviderOffered( "epic", true ) );
            // a provider that needs no client id is offered with or without settings
            ASSERT_TRUE( settings.IsProviderOffered( "testopenid", false ) && settings.IsProviderOffered( "microsoft", false ) );

            settings.enabledProviders.push_back( "microsoft" );
            ASSERT_FALSE( settings.IsProviderOffered( "google", true ) );
            ASSERT_FALSE( settings.IsProviderOffered( "microsoft", true ) );
            ASSERT_TRUE( settings.IsProviderOffered( "microsoft", false ) && !settings.IsProviderOffered( "testopenid", false ) );
            settings.enabledProviders.push_back( "google" );
            ASSERT_TRUE( settings.IsProviderOffered( "google", true ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The client ids a server accepts are the client id and the list, without empty entries, spaces and duplicates" )
        try
        {
            AUTH::CAuthProviderSettings settings;
            CORE::CString::StringVector ids;
            ASSERT_TRUE( settings.audiences.empty() && !settings.HasAcceptedClientIds() );
            settings.GetAcceptedClientIds( ids );
            ASSERT_TRUE( ids.empty() );

            settings.audiences.push_back( CORE::CString::Empty );
            settings.audiences.push_back( "  " );
            ASSERT_FALSE( settings.HasAcceptedClientIds() );
            settings.GetAcceptedClientIds( ids );
            ASSERT_TRUE( ids.empty() );

            settings.audiences.push_back( " desktop " );
            settings.audiences.push_back( "android" );
            settings.audiences.push_back( "desktop" );
            ASSERT_TRUE( settings.HasAcceptedClientIds() && !settings.HasClientId() );
            settings.GetAcceptedClientIds( ids );
            ASSERT_TRUE( 2 == ids.size() && "desktop" == ids[ 0 ] && "android" == ids[ 1 ] );

            // the client id of the build comes first, once
            settings.clientId = "own";
            settings.GetAcceptedClientIds( ids );
            ASSERT_TRUE( 3 == ids.size() && "own" == ids[ 0 ] && "desktop" == ids[ 1 ] && "android" == ids[ 2 ] );
            settings.audiences.push_back( "own" );
            settings.GetAcceptedClientIds( ids );
            ASSERT_TRUE( 3 == ids.size() );

            // the client id alone, as before
            AUTH::CAuthProviderSettings single;
            single.clientId = "only";
            single.GetAcceptedClientIds( ids );
            ASSERT_TRUE( single.HasAcceptedClientIds() && 1 == ids.size() && "only" == ids[ 0 ] );

            // copies keep the list
            AUTH::CAuthProviderSettings copy( settings );
            AUTH::CAuthProviderSettings assigned;
            assigned = settings;
            ASSERT_TRUE( copy.audiences == settings.audiences && assigned.audiences == settings.audiences );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The list of client ids is an array, a single value or a comma list in a config, and an array again when saved" )
        GUCEF_TESTFW_SKIP_IF( !JsonCodecAvailable(), "The json codec plugin is not available" );
        try
        {
            // an array
            CORE::CDataNode array;
            ASSERT_TRUE( ParseJson( "{\"Providers\":{\"google\":{\"audiences\":[\"a.example.invalid\",\"b.example.invalid\"]}}}", array ) );
            AUTH::CAuthSettings fromArray;
            ASSERT_TRUE( fromArray.LoadConfig( array ) );
            AUTH::CAuthProviderSettings google;
            ASSERT_TRUE( fromArray.GetProviderSettings( "google", google ) );
            ASSERT_TRUE( 2 == google.audiences.size() && "a.example.invalid" == google.audiences[ 0 ] && "b.example.invalid" == google.audiences[ 1 ] );
            ASSERT_TRUE( google.clientId.IsNULLOrEmpty() );

            // a single value
            CORE::CDataNode scalar;
            ASSERT_TRUE( ParseJson( "{\"Providers\":{\"google\":{\"audiences\":\"a.example.invalid\"}}}", scalar ) );
            AUTH::CAuthSettings fromScalar;
            ASSERT_TRUE( fromScalar.LoadConfig( scalar ) );
            ASSERT_TRUE( fromScalar.GetProviderSettings( "google", google ) );
            ASSERT_TRUE( 1 == google.audiences.size() && "a.example.invalid" == google.audiences[ 0 ] );

            // a comma list, with spaces and an empty element
            CORE::CDataNode comma;
            ASSERT_TRUE( ParseJson( "{\"Providers\":{\"google\":{\"audiences\":\"a.example.invalid, b.example.invalid,,c.example.invalid \"}}}", comma ) );
            AUTH::CAuthSettings fromComma;
            ASSERT_TRUE( fromComma.LoadConfig( comma ) );
            ASSERT_TRUE( fromComma.GetProviderSettings( "google", google ) );
            ASSERT_TRUE( 3 == google.audiences.size() && "a.example.invalid" == google.audiences[ 0 ] && "b.example.invalid" == google.audiences[ 1 ] && "c.example.invalid" == google.audiences[ 2 ] );

            // an array with spaces and empty values, an empty array and an empty text
            CORE::CDataNode dirty;
            ASSERT_TRUE( ParseJson( "{\"Providers\":{\"google\":{\"audiences\":[\" a.example.invalid \",\"\",\"  \"]}}}", dirty ) );
            AUTH::CAuthSettings fromDirty;
            ASSERT_TRUE( fromDirty.LoadConfig( dirty ) && fromDirty.GetProviderSettings( "google", google ) );
            ASSERT_TRUE( 1 == google.audiences.size() && "a.example.invalid" == google.audiences[ 0 ] );
            CORE::CDataNode emptyArray;
            ASSERT_TRUE( ParseJson( "{\"Providers\":{\"google\":{\"audiences\":[]}}}", emptyArray ) );
            AUTH::CAuthSettings fromEmptyArray;
            ASSERT_TRUE( fromEmptyArray.LoadConfig( emptyArray ) && fromEmptyArray.GetProviderSettings( "google", google ) );
            ASSERT_TRUE( google.audiences.empty() && !google.HasAcceptedClientIds() );
            CORE::CDataNode emptyText;
            ASSERT_TRUE( ParseJson( "{\"Providers\":{\"google\":{\"audiences\":\"\"}}}", emptyText ) );
            AUTH::CAuthSettings fromEmptyText;
            ASSERT_TRUE( fromEmptyText.LoadConfig( emptyText ) && fromEmptyText.GetProviderSettings( "google", google ) );
            ASSERT_TRUE( google.audiences.empty() );

            // as a game server writes it: no client id, the list
            CORE::CDataNode server;
            ASSERT_TRUE( ParseJson( "{\"Auth\":{\"Providers\":{\"google\":{\"audiences\":[\"desktop.example.invalid\",\"android.example.invalid\"]}}}}", server ) );
            AUTH::CAuthSettings fromServer;
            ASSERT_TRUE( fromServer.LoadConfig( server ) && fromServer.GetProviderSettings( "google", google ) );
            ASSERT_TRUE( !google.HasClientId() && google.HasAcceptedClientIds() && 2 == google.audiences.size() );

            // saved as an array, which loads back as it was, in a tree and as json text
            CORE::CDataNode saved( "Auth", GUCEF_DATATYPE_OBJECT );
            ASSERT_TRUE( fromServer.SaveConfig( saved ) );
            const CORE::CDataNode* providers = saved.FindChild( "Providers" );
            ASSERT_TRUE( GUCEF_NULL != providers );
            const CORE::CDataNode* googleNode = GUCEF_NULL != providers ? providers->FindChild( "google" ) : GUCEF_NULL;
            ASSERT_TRUE( GUCEF_NULL != googleNode );
            const CORE::CDataNode* list = GUCEF_NULL != googleNode ? googleNode->FindChild( "audiences" ) : GUCEF_NULL;
            ASSERT_TRUE( GUCEF_NULL != list && GUCEF_DATATYPE_ARRAY == list->GetNodeType() && 2 == list->GetChildrenValues().size() );
            AUTH::CAuthSettings again;
            ASSERT_TRUE( again.LoadConfig( saved ) && again.GetProviderSettings( "google", google ) );
            ASSERT_TRUE( 2 == google.audiences.size() && "desktop.example.invalid" == google.audiences[ 0 ] && "android.example.invalid" == google.audiences[ 1 ] );

            // loading a config of a provider that does not mention the list keeps it; an empty list clears it
            AUTH::CAuthProviderSettings kept;
            kept.audiences.push_back( "desktop.example.invalid" );
            kept.audiences.push_back( "android.example.invalid" );
            CORE::CDataNode silent;
            ASSERT_TRUE( ParseJson( "{\"clientId\":\"own.example.invalid\"}", silent ) );
            ASSERT_TRUE( kept.LoadConfig( silent ) );
            ASSERT_TRUE( 2 == kept.audiences.size() && "own.example.invalid" == kept.clientId );
            CORE::CDataNode cleared;
            ASSERT_TRUE( ParseJson( "{\"audiences\":[]}", cleared ) );
            ASSERT_TRUE( kept.LoadConfig( cleared ) );
            ASSERT_TRUE( kept.audiences.empty() && "own.example.invalid" == kept.clientId );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A list of client ids does not make a provider offered: a client logs in with its own client id" )
        try
        {
            AUTH::CAuthSettings settings;
            AUTH::CAuthProviderSettings server;
            server.audiences.push_back( "desktop.example.invalid" );
            server.audiences.push_back( "android.example.invalid" );
            settings.SetProviderSettings( "google", server );
            ASSERT_FALSE( settings.IsProviderOffered( "google", true ) );
            ASSERT_TRUE( settings.IsProviderOffered( "google", false ) );
            server.clientId = "desktop.example.invalid";
            settings.SetProviderSettings( "google", server );
            ASSERT_TRUE( settings.IsProviderOffered( "google", true ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The last used providers are in order of use and limited" )
        try
        {
            AUTH::CAuthSettings settings;
            ASSERT_TRUE( settings.TouchLastUsed( "google" ) );
            ASSERT_FALSE( settings.TouchLastUsed( "google" ) );
            ASSERT_TRUE( settings.TouchLastUsed( "testopenid" ) );
            ASSERT_TRUE( settings.TouchLastUsed( "epic" ) );
            ASSERT_TRUE( 3 == settings.lastUsedProviders.size() && "epic" == settings.lastUsedProviders[ 0 ] && "testopenid" == settings.lastUsedProviders[ 1 ] && "google" == settings.lastUsedProviders[ 2 ] );
            ASSERT_TRUE( settings.TouchLastUsed( "google" ) );
            ASSERT_TRUE( 3 == settings.lastUsedProviders.size() && "google" == settings.lastUsedProviders[ 0 ] && "epic" == settings.lastUsedProviders[ 1 ] && "testopenid" == settings.lastUsedProviders[ 2 ] );
            ASSERT_FALSE( settings.TouchLastUsed( CORE::CString::Empty ) );

            for ( UInt32 i=0; i<40; ++i )
                settings.TouchLastUsed( "provider" + CORE::UInt32ToString( i ) );
            ASSERT_TRUE( AUTH::CAuthSettings::MaxLastUsedProviders == settings.lastUsedProviders.size() );
            ASSERT_TRUE( "provider39" == settings.lastUsedProviders[ 0 ] );

            // a longer saved list is cut when loaded
            CORE::CDataNode node( "Auth", GUCEF_DATATYPE_OBJECT );
            CORE::CDataNode* list = node.AddChild( "lastUsedProviders", GUCEF_DATATYPE_ARRAY );
            for ( UInt32 i=0; i<30; ++i )
                list->AddValueAsChild( "p" + CORE::UInt32ToString( i ) );
            AUTH::CAuthSettings loaded;
            ASSERT_TRUE( loaded.LoadConfig( node ) && AUTH::CAuthSettings::MaxLastUsedProviders == loaded.lastUsedProviders.size() && "p0" == loaded.lastUsedProviders[ 0 ] );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Login hints: remembered per provider, forgotten all at once, nothing kept for what is not a name or while the setting is off" )
        try
        {
            AUTH::CAuthSettings settings;
            ASSERT_TRUE( settings.rememberLoginHints && !settings.HasLoginHints() );
            CORE::CString hint;
            ASSERT_FALSE( settings.GetLoginHint( "google", hint ) );

            ASSERT_TRUE( settings.SetLoginHint( "google", "me@example.invalid" ) );
            ASSERT_FALSE( settings.SetLoginHint( "google", "me@example.invalid" ) );
            ASSERT_TRUE( settings.SetLoginHint( "microsoft", "upn@example.invalid" ) );
            ASSERT_TRUE( settings.SetLoginHint( "google", "new@example.invalid" ) );
            ASSERT_TRUE( settings.GetLoginHint( "google", hint ) && "new@example.invalid" == hint );
            ASSERT_TRUE( settings.GetLoginHint( "microsoft", hint ) && "upn@example.invalid" == hint && settings.HasLoginHints() );

            // nothing that cannot be a name, no provider without a name
            ASSERT_FALSE( settings.SetLoginHint( "google", CORE::CString::Empty ) );
            ASSERT_FALSE( settings.SetLoginHint( "google", "two\nlines" ) );
            ASSERT_FALSE( settings.SetLoginHint( CORE::CString::Empty, "me@example.invalid" ) );
            ASSERT_TRUE( settings.GetLoginHint( "google", hint ) && "new@example.invalid" == hint );

            // forgetting
            ASSERT_TRUE( settings.ClearLoginHints() && !settings.HasLoginHints() );
            ASSERT_FALSE( settings.ClearLoginHints() );
            ASSERT_FALSE( settings.GetLoginHint( "google", hint ) );

            // the setting off: nothing is kept, what is there is not used or saved
            settings.SetLoginHint( "google", "me@example.invalid" );
            settings.rememberLoginHints = false;
            ASSERT_FALSE( settings.GetLoginHint( "google", hint ) );
            ASSERT_FALSE( settings.HasLoginHints() );
            ASSERT_FALSE( settings.SetLoginHint( "microsoft", "upn@example.invalid" ) );
            CORE::CDataNode node( "Auth", GUCEF_DATATYPE_OBJECT );
            ASSERT_TRUE( settings.SaveConfig( node ) );
            ASSERT_TRUE( GUCEF_NULL == node.FindChild( "LoginHints" ) );
            ASSERT_FALSE( node.GetAttributeValueOrChildValueByName( "rememberLoginHints" ).AsBool( true, true ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Login hints round trip through the data tree and a json file, with plus, at sign and unicode; a file written with the setting off or by hand with nonsense gives none" )
        GUCEF_TESTFW_SKIP_IF( !JsonCodecAvailable(), "The json codec plugin is not available" );
        try
        {
            AUTH::CAuthSettings settings;
            settings.SetLoginHint( "google", "j\xC3\xBCrgen+tag@example.invalid" );
            settings.SetLoginHint( "microsoft", "\xE5\xBC\xA0\xE4\xBC\x9F@example.invalid" );
            CORE::CDataNode node( "Auth", GUCEF_DATATYPE_OBJECT );
            ASSERT_TRUE( settings.SaveConfig( node ) );
            ASSERT_TRUE( GUCEF_NULL != node.FindChild( "LoginHints" ) );

            AUTH::CAuthSettings fromTree;
            ASSERT_TRUE( fromTree.LoadConfig( node ) && fromTree.loginHints == settings.loginHints && fromTree.rememberLoginHints );

            CORE::CDStoreCodecRegistry::TDStoreCodecPtr codec;
            ASSERT_TRUE( CORE::CCoreGlobal::Instance()->GetDStoreCodecRegistry().TryGetJsonCodec( codec ) );
            CORE::CDynamicBuffer buffer;
            CORE::CDynamicBufferAccess access( buffer );
            ASSERT_TRUE( codec->StoreDataTree( &node, &access ) );
            CORE::CDynamicBufferAccess readAccess( buffer );
            CORE::CDataNode parsed;
            ASSERT_TRUE( codec->BuildDataTree( &parsed, &readAccess ) );
            AUTH::CAuthSettings fromFile;
            ASSERT_TRUE( fromFile.LoadConfig( parsed ) && fromFile.loginHints == settings.loginHints );

            // a hand written file: values that are not names are not taken, with the setting off the whole child is dropped
            CORE::CDataNode handWritten( "Auth", GUCEF_DATATYPE_OBJECT );
            CORE::CDataNode* hints = handWritten.AddChild( "LoginHints", GUCEF_DATATYPE_OBJECT );
            hints->SetAttribute( "google", CORE::CString( "fine@example.invalid" ) );
            hints->SetAttribute( "epic", CORE::CString( "two\nlines" ) );
            hints->SetAttribute( "amazon", CORE::CString::Empty );
            AUTH::CAuthSettings loaded;
            ASSERT_TRUE( loaded.LoadConfig( handWritten ) && 1 == loaded.loginHints.size() && "fine@example.invalid" == loaded.loginHints[ "google" ] );
            handWritten.SetAttribute( "rememberLoginHints", false );
            AUTH::CAuthSettings off;
            off.loginHints[ "google" ] = "old@example.invalid";
            ASSERT_TRUE( off.LoadConfig( handWritten ) && !off.rememberLoginHints && off.loginHints.empty() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The login timeout is ten minutes, kept between a minute and half an hour, and saved and loaded" )
        try
        {
            AUTH::CAuthSettings settings;
            ASSERT_TRUE( 600 == settings.loginTimeoutInSeconds && 600000 == AUTH::CAuthOptions::DefaultTimeoutInMs );
            ASSERT_TRUE( 60 == AUTH::CAuthSettings::ClampLoginTimeoutInSeconds( 0 ) && 60 == AUTH::CAuthSettings::ClampLoginTimeoutInSeconds( 59 ) && 60 == AUTH::CAuthSettings::ClampLoginTimeoutInSeconds( 60 ) );
            ASSERT_TRUE( 900 == AUTH::CAuthSettings::ClampLoginTimeoutInSeconds( 900 ) && 1800 == AUTH::CAuthSettings::ClampLoginTimeoutInSeconds( 1800 ) && 1800 == AUTH::CAuthSettings::ClampLoginTimeoutInSeconds( 1801 ) && 1800 == AUTH::CAuthSettings::ClampLoginTimeoutInSeconds( 4000000000U ) );

            settings.loginTimeoutInSeconds = 900;
            CORE::CDataNode node( "Auth", GUCEF_DATATYPE_OBJECT );
            ASSERT_TRUE( settings.SaveConfig( node ) );
            AUTH::CAuthSettings loaded;
            ASSERT_TRUE( loaded.LoadConfig( node ) && 900 == loaded.loginTimeoutInSeconds );

            // a file with nonsense is brought into the range, a file that does not say keeps what there is
            node.SetAttribute( "loginTimeoutInSeconds", (UInt32) 5 );
            ASSERT_TRUE( loaded.LoadConfig( node ) && 60 == loaded.loginTimeoutInSeconds );
            node.SetAttribute( "loginTimeoutInSeconds", (UInt32) 99999 );
            ASSERT_TRUE( loaded.LoadConfig( node ) && 1800 == loaded.loginTimeoutInSeconds );
            CORE::CDataNode empty( "Auth", GUCEF_DATATYPE_OBJECT );
            ASSERT_TRUE( loaded.LoadConfig( empty ) && 1800 == loaded.loginTimeoutInSeconds );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Loading a config keeps the values it does not mention" )
        try
        {
            AUTH::CAuthSettings settings = MakeFilledSettings();
            CORE::CDataNode node( "Auth", GUCEF_DATATYPE_OBJECT );
            node.SetAttribute( "autoLogin", true );
            ASSERT_TRUE( settings.LoadConfig( node ) );
            ASSERT_TRUE( settings.autoLogin );
            ASSERT_TRUE( "ReduxTest" == settings.appName && 4321 == settings.autoLoginStepTimeoutInMs );
            ASSERT_TRUE( 2 == settings.enabledProviders.size() && 2 == settings.lastUsedProviders.size() && 2 == settings.providers.size() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The settings kept under an earlier name of a provider are renamed: the provider settings, the lists and the login hint; what the new name has stays" )
        try
        {
            AUTH::CAuthSettings settings;
            AUTH::CAuthProviderSettings old;
            old.parameters[ "relayUrl" ] = "https://login.example.com/return";
            old.parameters[ "shared" ] = "from the old one";
            settings.SetProviderSettings( "before", old );
            settings.enabledProviders.push_back( "google" );
            settings.enabledProviders.push_back( "before" );
            settings.lastUsedProviders.push_back( "before" );
            settings.lastUsedProviders.push_back( "google" );
            settings.loginHints[ "before" ] = "person@example.com";

            // nothing under that name: nothing to do
            ASSERT_FALSE( settings.RenameProvider( "nothere", "after" ) );
            ASSERT_FALSE( settings.RenameProvider( "before", "before" ) );
            ASSERT_FALSE( settings.RenameProvider( "", "after" ) || settings.RenameProvider( "before", "" ) );

            ASSERT_TRUE( settings.RenameProvider( "before", "after" ) );
            AUTH::CAuthProviderSettings found;
            ASSERT_FALSE( settings.GetProviderSettings( "before", found ) );
            ASSERT_TRUE( settings.GetProviderSettings( "after", found ) && "https://login.example.com/return" == found.GetParameter( "relayUrl" ) );
            ASSERT_TRUE( 2 == settings.enabledProviders.size() && "google" == settings.enabledProviders[ 0 ] && "after" == settings.enabledProviders[ 1 ] );
            ASSERT_TRUE( 2 == settings.lastUsedProviders.size() && "after" == settings.lastUsedProviders[ 0 ] && "google" == settings.lastUsedProviders[ 1 ] );
            ASSERT_TRUE( 1 == settings.loginHints.size() && "person@example.com" == settings.loginHints[ "after" ] );
            ASSERT_FALSE( settings.RenameProvider( "before", "after" ) );

            // the new name has settings of its own already: they win, the parameters it lacks come over, the lists have it once
            AUTH::CAuthProviderSettings own;
            own.parameters[ "shared" ] = "from the new one";
            own.clientId = "own-client";
            AUTH::CAuthSettings both;
            both.SetProviderSettings( "before", old );
            both.SetProviderSettings( "after", own );
            both.enabledProviders.push_back( "before" );
            both.enabledProviders.push_back( "after" );
            both.lastUsedProviders.push_back( "after" );
            both.lastUsedProviders.push_back( "before" );
            both.loginHints[ "before" ] = "old@example.com";
            both.loginHints[ "after" ] = "new@example.com";
            ASSERT_TRUE( both.RenameProvider( "before", "after" ) );
            ASSERT_TRUE( 1 == both.providers.size() && both.GetProviderSettings( "after", found ) );
            ASSERT_TRUE( "own-client" == found.clientId && "from the new one" == found.GetParameter( "shared" ) && "https://login.example.com/return" == found.GetParameter( "relayUrl" ) );
            ASSERT_TRUE( 1 == both.enabledProviders.size() && "after" == both.enabledProviders[ 0 ] );
            ASSERT_TRUE( 1 == both.lastUsedProviders.size() && "after" == both.lastUsedProviders[ 0 ] );
            ASSERT_TRUE( 1 == both.loginHints.size() && "new@example.com" == both.loginHints[ "after" ] );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Relay: the parameter relayUrl of a provider is kept when acceptable; an address that is not refuses the settings of that provider and changes nothing" )
        try
        {
            AUTH::CAuthProviderSettings settings;
            settings.clientId = "kept";
            CORE::CDataNode node( "testopenid", GUCEF_DATATYPE_OBJECT );
            CORE::CDataNode* parameters = node.AddChild( "Parameters", GUCEF_DATATYPE_OBJECT );
            parameters->SetAttribute( "relayUrl", "https://login.vanvelzensoftware.com/openid-return" );
            ASSERT_TRUE( settings.LoadConfig( node ) );
            ASSERT_TRUE( "https://login.vanvelzensoftware.com/openid-return" == settings.GetParameter( "relayUrl" ) );

            // saved and loaded again
            CORE::CDataNode saved( "testopenid", GUCEF_DATATYPE_OBJECT );
            ASSERT_TRUE( settings.SaveConfig( saved ) );
            AUTH::CAuthProviderSettings again;
            ASSERT_TRUE( again.LoadConfig( saved ) && "https://login.vanvelzensoftware.com/openid-return" == again.GetParameter( "relayUrl" ) );

            // empty: no relay
            parameters->SetAttribute( "relayUrl", "" );
            ASSERT_TRUE( settings.LoadConfig( node ) && settings.GetParameter( "relayUrl" ).IsNULLOrEmpty() );
            ASSERT_TRUE( settings.LoadConfig( CORE::CDataNode( "testopenid", GUCEF_DATATYPE_OBJECT ) ) );

            // not acceptable: refused, and what was there stays
            settings.parameters[ "relayUrl" ] = "https://login.vanvelzensoftware.com/openid-return";
            const char* bad[] =
            {
                "http://login.vanvelzensoftware.com/openid-return", "https://user@login.vanvelzensoftware.com/", "https://login.vanvelzensoftware.com:8443/", "https://login.vanvelzensoftware.com/../x",
                "https://login.vanvelzensoftware.com/x?y=1", "https://login.vanvelzensoftware.com/x#y", "https://login.vanvelzensoftware.com/\xC3\xA9", "https://192.168.0.1/x", "login.vanvelzensoftware.com", "https://login.vanvelzensoftware.com/a b"
            };
            for ( size_t i=0; i<sizeof( bad ) / sizeof( bad[ 0 ] ); ++i )
            {
                parameters->SetAttribute( "relayUrl", bad[ i ] );
                node.SetAttribute( "clientId", "changed" );
                ASSERT_FALSE( settings.LoadConfig( node ) );
                ASSERT_TRUE( "kept" == settings.clientId && "https://login.vanvelzensoftware.com/openid-return" == settings.GetParameter( "relayUrl" ) );
            }

            // no way to relax it in a config: another port and http on the loopback address are for the tests of the engine only
            parameters->SetAttribute( "relayUrl", "http://127.0.0.1:50000/openid-return" );
            ASSERT_FALSE( settings.LoadConfig( node ) );
            parameters->SetAttribute( "relayUrl", "https://login.vanvelzensoftware.com/other" );
            node.SetAttribute( "clientId", "changed" );
            ASSERT_TRUE( settings.LoadConfig( node ) && "changed" == settings.clientId && "https://login.vanvelzensoftware.com/other" == settings.GetParameter( "relayUrl" ) );

            // in the settings of the application the entry of that provider is left out, the others stay
            AUTH::CAuthSettings all;
            CORE::CDataNode auth( "Auth", GUCEF_DATATYPE_OBJECT );
            CORE::CDataNode* providers = auth.AddChild( "Providers", GUCEF_DATATYPE_OBJECT );
            CORE::CDataNode* openId2 = providers->AddChild( "testopenid", GUCEF_DATATYPE_OBJECT );
            openId2->AddChild( "Parameters", GUCEF_DATATYPE_OBJECT )->SetAttribute( "relayUrl", "https://login.vanvelzensoftware.com:8443/openid-return" );
            providers->AddChild( "google", GUCEF_DATATYPE_OBJECT )->SetAttribute( "clientId", "g" );
            ASSERT_TRUE( all.LoadConfig( auth ) );
            AUTH::CAuthProviderSettings found;
            ASSERT_FALSE( all.GetProviderSettings( "testopenid", found ) );
            ASSERT_TRUE( all.GetProviderSettings( "google", found ) && "g" == found.clientId );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Relay: the section of a json config with an empty relayUrl loads as no relay, with one as the relay" )
        GUCEF_TESTFW_SKIP_IF( !JsonCodecAvailable(), "The json codec plugin is not available" );
        try
        {
            CORE::CDStoreCodecRegistry::TDStoreCodecPtr codec;
            ASSERT_TRUE( CORE::CCoreGlobal::Instance()->GetDStoreCodecRegistry().TryGetJsonCodec( codec ) );
            const char* texts[] =
            {
                "{ \"Providers\": { \"testopenid\": { \"Parameters\": { \"relayUrl\": \"\" } }, \"google\": { \"clientId\": \"\", \"audiences\": [] } } }",
                "{ \"Providers\": { \"testopenid\": { \"Parameters\": { \"relayUrl\": \"https://login.vanvelzensoftware.com/openid-return\" } } } }"
            };
            for ( UInt32 i=0; i<2; ++i )
            {
                CORE::CDynamicBuffer buffer;
                buffer.CopyFrom( (UInt32) strlen( texts[ i ] ), texts[ i ] );
                CORE::CDynamicBufferAccess access( buffer );
                CORE::CDataNode parsed;
                ASSERT_TRUE( codec->BuildDataTree( &parsed, &access ) );
                AUTH::CAuthSettings loaded;
                ASSERT_TRUE( loaded.LoadConfig( parsed ) );
                AUTH::CAuthProviderSettings openId2;
                ASSERT_TRUE( loaded.GetProviderSettings( "testopenid", openId2 ) );
                ASSERT_TRUE( 0 == i ? openId2.GetParameter( "relayUrl" ).IsNULLOrEmpty() : "https://login.vanvelzensoftware.com/openid-return" == openId2.GetParameter( "relayUrl" ) );
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
