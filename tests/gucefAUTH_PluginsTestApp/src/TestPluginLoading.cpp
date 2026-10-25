/*
 *  gucefAUTH_PluginsTestApp: Application for testing the gucefAUTH plugins
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

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_CCONFIGSTORE_H
#include "CConfigStore.h"
#define GUCEF_CORE_CCONFIGSTORE_H
#endif /* GUCEF_CORE_CCONFIGSTORE_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_BROWSERLAUNCHER_H
#include "gucefCORE_BrowserLauncher.h"
#define GUCEF_CORE_BROWSERLAUNCHER_H
#endif /* GUCEF_CORE_BROWSERLAUNCHER_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_AUTHPLUGIN_STEAM_H
#include "authpluginSTEAM.h"
#define GUCEF_AUTHPLUGIN_STEAM_H
#endif /* GUCEF_AUTHPLUGIN_STEAM_H ? */

#ifndef GUCEF_AUTHPLUGIN_STEAM_CSTEAMAUTHPROVIDER_H
#include "authpluginSTEAM_CSteamAuthProvider.h"
#define GUCEF_AUTHPLUGIN_STEAM_CSTEAMAUTHPROVIDER_H
#endif /* GUCEF_AUTHPLUGIN_STEAM_CSTEAMAUTHPROVIDER_H ? */

#ifndef GUCEF_AUTHPLUGIN_EOS_CEOSAUTHPROVIDER_H
#include "authpluginEOS_CEosAuthProvider.h"
#define GUCEF_AUTHPLUGIN_EOS_CEOSAUTHPROVIDER_H
#endif /* GUCEF_AUTHPLUGIN_EOS_CEOSAUTHPROVIDER_H ? */

#ifndef GUCEF_AUTHPLUGIN_ANDROID_CGOOGLEAUTHPROVIDER_H
#include "authpluginANDROID_CGoogleAuthProvider.h"
#define GUCEF_AUTHPLUGIN_ANDROID_CGOOGLEAUTHPROVIDER_H
#endif /* GUCEF_AUTHPLUGIN_ANDROID_CGOOGLEAUTHPROVIDER_H ? */

#ifndef GUCEF_AUTHPLUGIN_ANDROID_CAMAZONAUTHPROVIDER_H
#include "authpluginANDROID_CAmazonAuthProvider.h"
#define GUCEF_AUTHPLUGIN_ANDROID_CAMAZONAUTHPROVIDER_H
#endif /* GUCEF_AUTHPLUGIN_ANDROID_CAMAZONAUTHPROVIDER_H ? */

#include <string.h>

#include "TestPluginsCommon.h"
#include "TestFakeSteam.h"
#include "TestFakeSteamHost.h"
#include "TestFakeEos.h"
#include "TestFakeAndroid.h"
#include "TestFakeEngine.h"
#include "TestPluginLoading.h"

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

static const CORE::CString&
ClassOf( const char* name )
{
    static CORE::CString className;
    AUTH::CIAuthProviderPtr made = AUTH::CAuthGlobal::Instance()->GetProviderFactory().Create( name );
    className = made.IsNULL() ? CORE::CString::Empty : made->GetClassTypeName();
    return className;
}

/*-------------------------------------------------------------------------*/

static CORE::CString
ManagedClassOf( const char* name )
{
    AUTH::CIAuthProvider* provider = AUTH::CAuthGlobal::Instance()->GetManager().GetProvider( name );
    return GUCEF_NULL == provider ? CORE::CString::Empty : provider->GetClassTypeName();
}

/*-------------------------------------------------------------------------*/

/** The protocol of the descriptor in the registry of the process */
static AUTH::TAuthProtocol
DescriptorProtocolOf( const CORE::CString& name )
{
    AUTH::CAuthProviderDescriptor descriptor;
    AUTH::CAuthGlobal::Instance()->GetDescriptorRegistry().GetDescriptor( name, descriptor );
    return descriptor.protocol;
}

/*-------------------------------------------------------------------------*/

/** A JWT that says it is not signed: the algorithm none, which must never be taken */
static CORE::CString
MakeUnsignedJwt( void )
{
    CORE::CString header;
    CORE::CString payload;
    const char* headerJson = "{\"alg\":\"none\",\"typ\":\"JWT\"}";
    const char* payloadJson = "{\"iss\":\"https://api.epicgames.dev/epic/oauth/v1\",\"sub\":\"0123456789abcdef0123456789abcdef\",\"aud\":\"xyz-client-id\",\"exp\":4102444800}";
    CORE::Base64UrlEncode( headerJson, (UInt32) strlen( headerJson ), header, false );
    CORE::Base64UrlEncode( payloadJson, (UInt32) strlen( payloadJson ), payload, false );
    return header + "." + payload + ".";
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformPluginLoadingTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING plugin loading TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "PluginLoading" );

    GUCEF_TESTFW_TESTCASE( "Test: The Steam plugin loads as a plugin, registers its descriptor, profile and provider with the process and takes all of it away again when it unloads; without it there is no Steam anywhere" )
        try
        {
            AUTH::CAuthGlobal* global = AUTH::CAuthGlobal::Instance();
            AUTH::CAuthProviderFactory& factory = global->GetProviderFactory();

            // a build without the plugin has no Steam: no provider, no descriptor, no profile, and the offer to log in with it is not there
            ASSERT_FALSE( factory.IsConstructible( "steam" ) );
            ASSERT_TRUE( GUCEF_NULL == global->GetManager().GetProvider( "steam" ) );
            ASSERT_FALSE( global->GetDescriptorRegistry().HasDescriptor( "steam" ) );
            ASSERT_TRUE( GUCEF_NULL == global->GetOpenId2Profiles().Find( "steam" ) );
            ASSERT_TRUE( GUCEF_NULL == AUTHPLUGIN::STEAM::GetSteamOpenId2Profile() );
            CORE::CString::StringVector names;
            global->GetDescriptorRegistry().GetNames( names );
            for ( size_t i=0; i<names.size(); ++i )
                ASSERT_TRUE( AUTH::AUTHPROTOCOL_OPENID2 != DescriptorProtocolOf( names[ i ] ) );
            CORE::CString::StringVector offeredBefore;
            global->GetManager().GetOfferedProviders( offeredBefore );
            ASSERT_FALSE( AUTH::ContainsString( offeredBefore, "steam" ) );

            ASSERT_TRUE( LoadTestPlugin( "authpluginSTEAM", "authSteamTest" ) );
            ASSERT_TRUE( factory.IsConstructible( "steam" ) );
            ASSERT_TRUE( ClassOf( "steam" ) == "GUCEF::AUTHPLUGIN::STEAM::CSteamAuthProvider" );
            // the manager has made the provider
            ASSERT_TRUE( ManagedClassOf( "steam" ) == "GUCEF::AUTHPLUGIN::STEAM::CSteamAuthProvider" );
            // the plugin brought the description of the provider and the profile of its login
            AUTH::CAuthProviderDescriptor descriptor;
            ASSERT_TRUE( global->GetDescriptorRegistry().GetDescriptor( "steam", descriptor ) );
            ASSERT_TRUE( AUTH::AUTHPROTOCOL_OPENID2 == descriptor.protocol && "steam" == descriptor.accountProviderId && "Steam" == descriptor.displayName );
            ASSERT_TRUE( "Steam" == AUTH::GetProviderDisplayName( "steam" ) && "Steam" == AUTH::GetAccountProviderDisplayName( "steam" ) );
            ASSERT_TRUE( GUCEF_NULL != global->GetOpenId2Profiles().Find( "steam" ) && global->GetOpenId2Profiles().Find( "steam" ) == AUTHPLUGIN::STEAM::GetSteamOpenId2Profile() );

            // the login in the browser needs nothing of this machine: Steam is offered, the login with the client is not available
            AUTH::CIAuthProvider* provider = global->GetManager().GetProvider( "steam" );
            ASSERT_TRUE( provider->IsAvailable() );
            ASSERT_TRUE( global->GetManager().IsProviderOffered( "steam" ) );
            ASSERT_FALSE( static_cast< AUTHPLUGIN::STEAM::CSteamAuthProvider* >( provider )->IsClientAvailable() );
            CORE::CString::StringVector offered;
            global->GetManager().GetOfferedProviders( offered );
            ASSERT_TRUE( AUTH::ContainsString( offered, "steam" ) );
            // the old name of the browser login is gone as a provider, and is a former name in the description
            ASSERT_FALSE( factory.IsConstructible( "steam-openid" ) );
            ASSERT_TRUE( 1 == descriptor.formerNames.size() && "steam-openid" == descriptor.formerNames[ 0 ] );

            ASSERT_TRUE( CORE::CCoreGlobal::Instance()->GetPluginControl().UnloadPluginGroup( "authSteamTest" ) );
            ASSERT_FALSE( factory.IsConstructible( "steam" ) );
            ASSERT_TRUE( GUCEF_NULL == global->GetManager().GetProvider( "steam" ) );
            ASSERT_FALSE( global->GetDescriptorRegistry().HasDescriptor( "steam" ) );
            ASSERT_TRUE( GUCEF_NULL == global->GetOpenId2Profiles().Find( "steam" ) && GUCEF_NULL == AUTHPLUGIN::STEAM::GetSteamOpenId2Profile() );
            global->GetManager().GetOfferedProviders( offered );
            ASSERT_FALSE( AUTH::ContainsString( offered, "steam" ) );

            // and it loads again
            ASSERT_TRUE( LoadTestPlugin( "authpluginSTEAM", "authSteamTest" ) );
            ASSERT_TRUE( GUCEF_NULL != global->GetManager().GetProvider( "steam" ) && global->GetManager().IsProviderOffered( "steam" ) );
            ASSERT_TRUE( CORE::CCoreGlobal::Instance()->GetPluginControl().UnloadPluginGroup( "authSteamTest" ) );
            ASSERT_TRUE( GUCEF_NULL == global->GetManager().GetProvider( "steam" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: With the Steam plugin loaded and no Steam runtime library or app id, Steam is offered as the login in the browser, no silent login is possible and nothing else changes" )
        try
        {
            AUTH::CAuthManager& manager = AUTH::CAuthGlobal::Instance()->GetManager();
            manager.RefreshProviders();
            ASSERT_TRUE( GUCEF_NULL == manager.GetProvider( "steam" ) );
            CORE::CString::StringVector before;
            manager.GetOfferedProviders( before );
            AUTH::TAuthUnavailableProviderVector unavailableBefore;
            manager.GetUnavailableProviders( unavailableBefore );

            // a runtime library that is not there, as on a machine without the Steamworks SDK, and no app id or ticket identity in the settings
            AUTH::CAuthSettings settings = manager.GetSettings();
            AUTH::CAuthProviderSettings steamSettings;
            steamSettings.parameters[ "runtimeLibrary" ] = "libsteam_api_does_not_exist_zz.so";
            settings.SetProviderSettings( "steam", steamSettings );
            manager.SetSettings( settings );

            ASSERT_TRUE( LoadTestPlugin( "authpluginSTEAM", "authSteamSafeTest" ) );
            ASSERT_TRUE( GUCEF_NULL != manager.GetProvider( "steam" ) );
            // the machine of the test has no Steam, whatever the machine of the developer has
            CFakeHostView noSteam;
            AUTHPLUGIN::STEAM::GetSteamOpenId2Profile()->SetHostView( &noSteam );

            // the provider offers the login in the browser, and says that the one with the client is not available
            ASSERT_TRUE( manager.GetProvider( "steam" )->IsAvailable() );
            ASSERT_TRUE( manager.IsProviderOffered( "steam" ) );
            AUTHPLUGIN::STEAM::CSteamAuthProvider* steam = static_cast< AUTHPLUGIN::STEAM::CSteamAuthProvider* >( manager.GetProvider( "steam" ) );
            ASSERT_FALSE( steam->IsClientAvailable() );
            ASSERT_TRUE( manager.GetProvider( "steam" )->GetClassTypeName() == "GUCEF::AUTHPLUGIN::STEAM::CSteamAuthProvider" );

            CORE::CString::StringVector after;
            manager.GetOfferedProviders( after );
            ASSERT_TRUE( after.size() == before.size() + 1 && AUTH::ContainsString( after, "steam" ) );

            // a provider that is offered is not listed as one that is not set up
            AUTH::TAuthUnavailableProviderVector unavailableAfter;
            manager.GetUnavailableProviders( unavailableAfter );
            ASSERT_TRUE( unavailableAfter.size() == unavailableBefore.size() );
            for ( size_t i=0; i<unavailableAfter.size(); ++i )
                ASSERT_TRUE( "steam" != unavailableAfter[ i ].providerName );

            // no silent login is possible, so a start up login with nothing stored does nothing
            AUTH::CAuthSettings startSettings = manager.GetSettings();
            ASSERT_TRUE( startSettings.autoLogin );
            AUTH::CIAuthProvider* plugin = manager.GetProvider( "steam" );
            CORE::CFutureResult silent( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            ASSERT_FALSE( plugin->TrySilentLogin( silent ) );
            CORE::CString hintName;
            bool hintRunning = false;
            ASSERT_FALSE( plugin->GetDetectedAccountHint( hintName, hintRunning ) );

            AUTHPLUGIN::STEAM::GetSteamOpenId2Profile()->SetHostView( GUCEF_NULL );
            ASSERT_TRUE( CORE::CCoreGlobal::Instance()->GetPluginControl().UnloadPluginGroup( "authSteamSafeTest" ) );
            ASSERT_TRUE( GUCEF_NULL == manager.GetProvider( "steam" ) );
            settings.providers.erase( "steam" );
            manager.SetSettings( settings );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The EOS plugin without the SDK loads and leaves the built in Epic login where it is" )
        try
        {
            AUTH::CAuthProviderFactory& factory = AUTH::CAuthGlobal::Instance()->GetProviderFactory();
            ASSERT_TRUE( factory.IsConstructible( "epic" ) );
            ASSERT_TRUE( ClassOf( "epic" ) == "GUCEF::AUTH::CDescriptorAuthProvider" );

            ASSERT_TRUE( LoadTestPlugin( "authpluginEOS", "authEosTest" ) );
            ASSERT_TRUE( ClassOf( "epic" ) == "GUCEF::AUTH::CDescriptorAuthProvider" );
            ASSERT_TRUE( ManagedClassOf( "epic" ) == "GUCEF::AUTH::CDescriptorAuthProvider" );

            ASSERT_TRUE( CORE::CCoreGlobal::Instance()->GetPluginControl().UnloadPluginGroup( "authEosTest" ) );
            ASSERT_TRUE( factory.IsConstructible( "epic" ) );
            ASSERT_TRUE( ClassOf( "epic" ) == "GUCEF::AUTH::CDescriptorAuthProvider" );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The Android plugin without the Java side loads and leaves the built in providers; the redirects go to the engine" )
        try
        {
            AUTH::CAuthProviderFactory& factory = AUTH::CAuthGlobal::Instance()->GetProviderFactory();
            AUTHPLUGIN::ANDROIDAUTH::CAndroidAuthBridge* bridge = AUTHPLUGIN::ANDROIDAUTH::CAndroidAuthBridge::Instance();
            CORE::CIBrowserLauncherHandler* launcher = CORE::GetBrowserLauncherHandler();

            ASSERT_FALSE( bridge->HasShim() );
            ASSERT_TRUE( LoadTestPlugin( "authpluginANDROID", "authAndroidTest" ) );
            ASSERT_TRUE( ClassOf( "google" ) == "GUCEF::AUTH::CDescriptorAuthProvider" );
            ASSERT_TRUE( ClassOf( "amazon" ) == "GUCEF::AUTH::CDescriptorAuthProvider" );
            ASSERT_TRUE( ManagedClassOf( "google" ) == "GUCEF::AUTH::CDescriptorAuthProvider" );
            // nothing to open Custom Tabs with: the browser launcher is not touched
            ASSERT_TRUE( CORE::GetBrowserLauncherHandler() == launcher );

            // a redirect that Android hands over goes to the engine of the process
            CFakeOidcEngine engine;
            {
                CScopedEngine scoped( &engine );
                ASSERT_TRUE( bridge->DeliverRedirect( "com.example.game:/oauth2redirect?code=abc&state=def" ) );
                ASSERT_TRUE( 1 == engine.redirectCount && engine.lastRedirectUrl == "com.example.game:/oauth2redirect?code=abc&state=def" );
            }

            ASSERT_TRUE( CORE::CCoreGlobal::Instance()->GetPluginControl().UnloadPluginGroup( "authAndroidTest" ) );
            ASSERT_TRUE( factory.IsConstructible( "google" ) && factory.IsConstructible( "amazon" ) );
            // and after the unload nothing takes redirects
            ASSERT_FALSE( bridge->DeliverRedirect( "com.example.game:/oauth2redirect?code=abc&state=def" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The Android plugin with a Java side that has Play services only replaces google, installs the Custom Tabs launcher and gives everything back" )
        try
        {
            AUTH::CAuthProviderFactory& factory = AUTH::CAuthGlobal::Instance()->GetProviderFactory();
            AUTHPLUGIN::ANDROIDAUTH::CAndroidAuthBridge* bridge = AUTHPLUGIN::ANDROIDAUTH::CAndroidAuthBridge::Instance();
            CORE::CIBrowserLauncherHandler* launcher = CORE::GetBrowserLauncherHandler();

            CFakeAndroid fake;
            fake.amazon = false;
            {
                CScopedAndroid scope( fake );
                ASSERT_TRUE( LoadTestPlugin( "authpluginANDROID", "authAndroidTest2" ) );

                ASSERT_TRUE( ClassOf( "google" ) == "GUCEF::AUTHPLUGIN::ANDROIDAUTH::CGoogleAuthProvider" );
                ASSERT_TRUE( ClassOf( "amazon" ) == "GUCEF::AUTH::CDescriptorAuthProvider" );
                // the provider that the manager holds is the native one
                ASSERT_TRUE( ManagedClassOf( "google" ) == "GUCEF::AUTHPLUGIN::ANDROIDAUTH::CGoogleAuthProvider" );
                ASSERT_TRUE( ManagedClassOf( "amazon" ) == "GUCEF::AUTH::CDescriptorAuthProvider" );

                // the browser of the engine is a Custom Tab
                ASSERT_TRUE( CORE::GetBrowserLauncherHandler() != launcher );
                ASSERT_TRUE( CORE::OpenUrlInSystemBrowser( "https://accounts.example.invalid/authorize?state=1" ) );
                ASSERT_TRUE( 1 == fake.openedUrls.size() );

                ASSERT_TRUE( CORE::CCoreGlobal::Instance()->GetPluginControl().UnloadPluginGroup( "authAndroidTest2" ) );
                ASSERT_TRUE( ClassOf( "google" ) == "GUCEF::AUTH::CDescriptorAuthProvider" );
                ASSERT_TRUE( ManagedClassOf( "google" ) == "GUCEF::AUTH::CDescriptorAuthProvider" );
                ASSERT_TRUE( factory.IsConstructible( "amazon" ) );
                ASSERT_TRUE( CORE::GetBrowserLauncherHandler() == launcher );
                ASSERT_FALSE( bridge->DeliverRedirect( "x:/y" ) );
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The manager logs in with the Steam provider at start up, as the silent provider that it is" )
        try
        {
            CFakeSteam steam;
            AUTH::CAuthProviderFactory noFactories;
            AUTH::CMemoryCredentialStore credentials;
            AUTH::CAuthManager manager;
            manager.SetProviderFactory( &noFactories );
            manager.SetCredentialStore( &credentials );

            AUTH::CAuthSettings settings;
            settings.autoLogin = true;
            AUTH::CAuthProviderSettings steamSettings;
            steamSettings.parameters[ "ticketIdentity" ] = "unit-test-service";
            steamSettings.parameters[ "callbackIntervalInMs" ] = "5";
            settings.SetProviderSettings( "steam", steamSettings );
            manager.SetSettings( settings );

            AUTHPLUGIN::STEAM::TSteamAuthProviderFactory steamFactory;
            AUTH::CIAuthProviderPtr provider = steamFactory.Create();
            static_cast< AUTHPLUGIN::STEAM::CSteamAuthProvider* >( provider.GetPointerAlways() )->SetShim( steam.MakeShim() );
            ASSERT_TRUE( manager.AddProvider( provider ) );
            ASSERT_TRUE( manager.IsProviderOffered( "steam" ) );

            CProviderRecorder none;
            CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            ASSERT_TRUE( manager.AutoLogin( future ) );
            ASSERT_TRUE( PumpUntilReady( future, 5000 ) );

            GUCEF::vector< AUTH::CAuthResult > sessions;
            manager.GetSessionAccounts( sessions );
            ASSERT_TRUE( 1 == sessions.size() );
            ASSERT_TRUE( sessions[ 0 ].accountRef.ToString() == "steam:76561198000000001" );
            ASSERT_TRUE( sessions[ 0 ].proof.kind == AUTH::CAuthProof::KindPlatformTicket );
            ASSERT_TRUE( manager.GetSettings().lastUsedProviders.size() == 1 && manager.GetSettings().lastUsedProviders[ 0 ] == "steam" );

            // Logging out at the manager cancels the ticket in Steam
            ASSERT_TRUE( manager.Logout( sessions[ 0 ].accountRef ) );
            ASSERT_TRUE( steam.WasCancelled( 100 ) );
            manager.RemoveProvider( "steam" );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The Steam plugin loaded twice is loaded both times with one provider, and a provider of its name that is not its own stays when it cannot load" )
        try
        {
            AUTH::CAuthProviderFactory& factory = AUTH::CAuthGlobal::Instance()->GetProviderFactory();
            CORE::CPluginControl& pluginControl = CORE::CCoreGlobal::Instance()->GetPluginControl();
            ASSERT_FALSE( factory.IsConstructible( "steam" ) );

            ASSERT_TRUE( LoadTestPlugin( "authpluginSTEAM", "authSteamTwiceA" ) );
            ASSERT_TRUE( LoadTestPlugin( "authpluginSTEAM", "authSteamTwiceB" ) );
            ASSERT_TRUE( ClassOf( "steam" ) == "GUCEF::AUTHPLUGIN::STEAM::CSteamAuthProvider" );
            ASSERT_TRUE( pluginControl.UnloadPluginGroup( "authSteamTwiceA" ) );
            ASSERT_TRUE( pluginControl.UnloadPluginGroup( "authSteamTwiceB" ) );
            ASSERT_FALSE( factory.IsConstructible( "steam" ) );
            pluginControl.ClearMetaDataFromGroup( "authSteamTwiceA" );
            pluginControl.ClearMetaDataFromGroup( "authSteamTwiceB" );

            // somebody else has the name: the plugin does not load, and what it does when it gives up leaves the provider of the other alone
            AUTHPLUGIN::STEAM::TSteamAuthProviderFactory foreign;
            factory.RegisterConcreteFactory( "steam", &foreign );
            ASSERT_FALSE( LoadTestPlugin( "authpluginSTEAM", "authSteamForeignTest" ) );
            ASSERT_TRUE( factory.IsConstructible( "steam" ) );
            ASSERT_TRUE( ClassOf( "steam" ) == "GUCEF::AUTHPLUGIN::STEAM::CSteamAuthProvider" );
            factory.UnregisterConcreteFactory( "steam" );
            ASSERT_FALSE( factory.IsConstructible( "steam" ) );
            pluginControl.UnloadPluginGroup( "authSteamForeignTest" );
            pluginControl.ClearMetaDataFromGroup( "authSteamForeignTest" );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The Steam plugin in the bootstrap config of an application, as the game ships it, loads and the config of the application loads with it" )
        try
        {
            AUTH::CAuthProviderFactory& factory = AUTH::CAuthGlobal::Instance()->GetProviderFactory();
            ASSERT_FALSE( factory.IsConstructible( "steam" ) );

            // The groups of the tests before this one are known to the plugin control still, with their plugins not loaded:
            // loading the config of the application would load them
            CORE::CPluginControl& pluginControl = CORE::CCoreGlobal::Instance()->GetPluginControl();
            pluginControl.ClearMetaDataFromGroup( "authSteamTest" );
            pluginControl.ClearMetaDataFromGroup( "authSteamSafeTest" );
            pluginControl.ClearMetaDataFromGroup( "authEosTest" );
            pluginControl.ClearMetaDataFromGroup( "authAndroidTest" );
            pluginControl.ClearMetaDataFromGroup( "authAndroidTest2" );

            CORE::CConfigStore& configStore = CORE::CCoreGlobal::Instance()->GetConfigStore();
            const CORE::CString previousBootstrap = configStore.GetBootstrapConfigFile();
            const CORE::CString::StringVector previousConfigFiles = configStore.GetConfigFiles();

            // The block is the one of the bootstrap config of the game: the plugin may fail, the game works without it
            const CORE::CString path = CORE::CombinePath( CORE::RelativePath( "$TEMPDIR$" ), "gucefAUTH_PluginsTestApp_Bootstrap.ini" );
            const CORE::CString ini = "[Bootstrap\\PluginGroup]\n"
                                      "\"GroupName\"=\"authSteamBootstrapTest\"\n"
                                      "\"LoadImmediately\"=\"true\"\n"
                                      "\n"
                                      "[Bootstrap\\PluginGroup\\PluginMetaData]\n"
                                      "\"Description\"=\"gucefAUTH plugin for logging in with the Steam client (Steamworks SDK); without the SDK or the Steam client the login of the browser is used\"\n"
                                      "\"Path\"=\"$MODULEDIR$\"\n"
                                      "\"Filename\"=\"authpluginSTEAM_d\"\n"
                                      "\"AltFilename\"=\"authpluginSTEAM\"\n"
                                      "\"Type\"=\"GucefGenericPlugin\"\n"
                                      "\"LoadFailAllowed\"=\"true\"\n";
            ASSERT_TRUE( CORE::WriteStringAsTextFile( path, ini ) );
            configStore.SetBootstrapConfigFile( path );
            configStore.SetConfigFiles( CORE::CString::StringVector() );

            // the way the application starts: the config store loads the bootstrap config, with the plugin, and then the config
            const bool loaded = configStore.LoadConfig();
            const bool registered = factory.IsConstructible( "steam" );
            AUTH::CIAuthProvider* provider = AUTH::CAuthGlobal::Instance()->GetManager().GetProvider( "steam" );
            const bool providerMade = GUCEF_NULL != provider;
            const bool providerAvailable = providerMade && provider->IsAvailable();

            const bool unloaded = CORE::CCoreGlobal::Instance()->GetPluginControl().UnloadPluginGroup( "authSteamBootstrapTest" );
            CORE::CCoreGlobal::Instance()->GetPluginControl().ClearMetaDataFromGroup( "authSteamBootstrapTest" );
            configStore.SetBootstrapConfigFile( previousBootstrap );
            configStore.SetConfigFiles( previousConfigFiles );
            CORE::DeleteFile( path );

            ASSERT_TRUE( loaded );
            ASSERT_TRUE( registered );
            // no Steam client or app id here: the provider is the login of the browser
            ASSERT_TRUE( providerMade );
            ASSERT_TRUE( providerAvailable );
            ASSERT_TRUE( unloaded );
            ASSERT_FALSE( factory.IsConstructible( "steam" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: With the engine of the process, tokens that are not signed or not tokens are refused without anything being asked of the network" )
        try
        {
            // The process has its own OIDC engine: the one that does the work for the descriptor providers
            ASSERT_TRUE( GUCEF_NULL != AUTH::CAuthGlobal::Instance()->GetFlowEngine() );

            CFakeEos eos;
            AUTHPLUGIN::EOS::CEosAuthProvider epic( eos.MakeShim() );
            CORE::CDataNode epicNode( "epic", GUCEF_DATATYPE_OBJECT );
            epicNode.SetAttribute( "clientId", "xyz-client-id" );
            epicNode.SetAttribute( "clientSecret", "secret" );
            ASSERT_TRUE( epic.LoadConfig( epicNode ) );
            ASSERT_TRUE( epic.IsVerifySupported() );
            AUTH::CAuthResult result;
            ASSERT_FALSE( epic.Verify( AUTH::CAuthProof( AUTH::CAuthProof::KindOidcIdToken, MakeUnsignedJwt() ), result ) );
            ASSERT_FALSE( epic.Verify( AUTH::CAuthProof( AUTH::CAuthProof::KindOidcIdToken, "not.a.jwt" ), result ) );
            ASSERT_FALSE( epic.Verify( AUTH::CAuthProof( AUTH::CAuthProof::KindOidcIdToken, "garbage" ), result ) );

            AUTHPLUGIN::ANDROIDAUTH::CGoogleAuthProvider google;
            CORE::CDataNode googleNode( "google", GUCEF_DATATYPE_OBJECT );
            googleNode.SetAttribute( "clientId", "web-client-id.apps.example.invalid" );
            ASSERT_TRUE( google.LoadConfig( googleNode ) );
            ASSERT_TRUE( google.IsVerifySupported() );
            ASSERT_FALSE( google.Verify( AUTH::CAuthProof( AUTH::CAuthProof::KindOidcIdToken, MakeUnsignedJwt() ), result ) );
            ASSERT_FALSE( google.Verify( AUTH::CAuthProof( AUTH::CAuthProof::KindOidcIdToken, "garbage" ), result ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
