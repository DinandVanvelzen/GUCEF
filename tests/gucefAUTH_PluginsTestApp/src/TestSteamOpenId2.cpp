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

#include <string.h>

#ifndef GUCEF_MT_CACTIVEOBJECT_H
#include "gucefMT_CActiveObject.h"
#define GUCEF_MT_CACTIVEOBJECT_H
#endif /* GUCEF_MT_CACTIVEOBJECT_H ? */

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_BROWSERLAUNCHER_H
#include "gucefCORE_BrowserLauncher.h"
#define GUCEF_CORE_BROWSERLAUNCHER_H
#endif /* GUCEF_CORE_BROWSERLAUNCHER_H ? */

#ifndef GUCEF_CORE_CDATETIME_H
#include "gucefCORE_CDateTime.h"
#define GUCEF_CORE_CDATETIME_H
#endif /* GUCEF_CORE_CDATETIME_H ? */

#ifndef GUCEF_CORE_CPLUGINCONTROL_H
#include "CPluginControl.h"
#define GUCEF_CORE_CPLUGINCONTROL_H
#endif /* GUCEF_CORE_CPLUGINCONTROL_H ? */

#ifndef GUCEF_WEB_H
#include "gucefWEB.h"
#define GUCEF_WEB_H
#endif /* GUCEF_WEB_H ? */

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

#include "TestPluginsCommon.h"
#include "TestFakeHttp.h"
#include "TestFakeSteamHost.h"
#include "TestSteamOpenId2.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;
using namespace GUCEF::AUTHPLUGIN::STEAM;

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The browser of the test: the handler behind CORE::OpenUrlInSystemBrowser(), so no window ever opens. It does what a person
 *  does who signs in with Steam: it takes the address of the login, and sends the browser to the return address of it with the
 *  positive assertion that Steam would have given (the provider that checks it is the fake web server of the test).
 */
class CSteamBrowser : public CORE::CIBrowserLauncherHandler ,
                      public MT::CActiveObject
{
    public:

    CSteamBrowser( void )
        : CORE::CIBrowserLauncherHandler(), MT::CActiveObject(), m_lock(), m_queue(), m_urls(), m_statuses(), m_running( 0 ),
          m_previous( GUCEF_NULL ), m_installed( false ), m_started( false ), m_client(), m_steamId( "76561198000000042" ), m_origin(), m_claimedId()
    {}

    virtual ~CSteamBrowser()
    {
        Uninstall();
        Stop();
    }

    bool Start( void )
    {
        if ( !m_started )
            m_started = Activate( GUCEF_NULL, 1, 1 );
        return m_started;
    }

    void Stop( void )
    {
        if ( m_started )
        {
            m_started = false;
            Deactivate( false, true );
        }
    }

    void Install( void )
    {
        if ( !m_installed )
        {
            m_previous = CORE::SetBrowserLauncherHandler( this );
            m_installed = true;
        }
    }

    void Uninstall( void )
    {
        if ( m_installed )
        {
            CORE::SetBrowserLauncherHandler( m_previous );
            m_previous = GUCEF_NULL;
            m_installed = false;
        }
    }

    /** The account the fake Steam says the person is, and the origin of the endpoint that gives the claimed id (the fake web server) */
    void SetIdentity( const CORE::CString& steamId, const CORE::CString& origin ) { MT::CScopeMutex lock( m_lock ); m_steamId = steamId; m_origin = origin; }

    /** What the claimed id is when it is not <origin>/openid/id/<id> */
    void SetClaimedId( const CORE::CString& claimedId ) { MT::CScopeMutex lock( m_lock ); m_claimedId = claimedId; }

    virtual bool OpenUrl( const CORE::CString& url ) GUCEF_VIRTUAL_OVERRIDE
    {
        MT::CScopeMutex lock( m_lock );
        m_urls.push_back( url );
        m_queue.push_back( url );
        ++m_running;
        return true;
    }

    UInt32 GetOpenCount( void ) const { MT::CScopeMutex lock( m_lock ); return (UInt32) m_urls.size(); }
    CORE::CString GetLastUrl( void ) const { MT::CScopeMutex lock( m_lock ); return m_urls.empty() ? CORE::CString::Empty : m_urls.back(); }
    vector< int > GetStatuses( void ) const { MT::CScopeMutex lock( m_lock ); return m_statuses; }
    bool IsIdle( void ) const { MT::CScopeMutex lock( m_lock ); return 0 == m_running; }

    virtual const MT::CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NULL; }

    protected:

    virtual bool OnThreadStart( void* taskdata ) GUCEF_VIRTUAL_OVERRIDE { return true; }

    virtual bool OnThreadCycle( void* taskdata ) GUCEF_VIRTUAL_OVERRIDE
    {
        CORE::CString url;
        CORE::CString steamId;
        CORE::CString origin;
        CORE::CString claimedOverride;
        {
            MT::CScopeMutex lock( m_lock );
            if ( m_queue.empty() )
                return false;
            url = m_queue.front();
            m_queue.erase( m_queue.begin() );
            steamId = m_steamId;
            origin = m_origin;
            claimedOverride = m_claimedId;
        }
        const int status = GoToReturnAddress( url, steamId, origin, claimedOverride );
        {
            MT::CScopeMutex lock( m_lock );
            m_statuses.push_back( status );
            --m_running;
        }
        return false;
    }

    private:

    int GoToReturnAddress( const CORE::CString& url, const CORE::CString& steamId, const CORE::CString& origin, const CORE::CString& claimedOverride )
    {
        CORE::CString scheme, host, path, query;
        UInt16 port = 0;
        CORE::CString::StringMap asked;
        bool duplicate = false;
        if ( !AUTH::CAuthUrl::Split( url, scheme, host, port, path, query ) || !AUTH::CAuthUrl::ParseQuery( query, asked, duplicate ) )
            return 0;
        const CORE::CString returnTo = asked[ "openid.return_to" ];
        if ( returnTo.IsNULLOrEmpty() )
            return 0;

        // what Steam answers with: the claimed id is the one of the account, on the origin of the endpoint
        const CORE::CString claimedId = claimedOverride.IsNULLOrEmpty() ? origin + "/openid/id/" + steamId : claimedOverride;
        CORE::CString nonce = CORE::CDateTime::NowUTCDateTime().ToIso8601DateTimeString( true, false );
        if ( !nonce.EndsWith( "Z" ) )
            nonce += "Z";
        nonce += "steam-test-nonce";
        CORE::CString::StringMap fields;
        fields[ "openid.ns" ] = AUTH::COpenId2Assertion::Namespace;
        fields[ "openid.mode" ] = "id_res";
        fields[ "openid.op_endpoint" ] = origin + "/openid/login";
        fields[ "openid.claimed_id" ] = claimedId;
        fields[ "openid.identity" ] = claimedId;
        fields[ "openid.return_to" ] = returnTo;
        fields[ "openid.response_nonce" ] = nonce;
        fields[ "openid.assoc_handle" ] = "1234567890";
        fields[ "openid.signed" ] = "signed,op_endpoint,claimed_id,identity,return_to,response_nonce,assoc_handle";
        fields[ "openid.sig" ] = "c2lnbmF0dXJl";
        const CORE::CString target = returnTo + ( returnTo.HasChar( '?' ) >= 0 ? "&" : "?" ) + AUTH::CAuthUrl::EncodeForm( fields );

        WEB::CHttpRequestData request;
        WEB::CHttpClientRequestOptions options;
        options.followRedirects = false;
        options.totalTimeoutInMs = 10000;
        request.requestType = "GET";
        if ( !request.SetUrl( target ) )
            return 0;
        WEB::CHttpResponseData response;
        WEB::CHttpClientTransferInfo info;
        if ( !m_client.Execute( request, options, response, &info ) )
            return 0;
        return response.statusCode;
    }

    CSteamBrowser( const CSteamBrowser& src );                /**< not implemented */
    CSteamBrowser& operator=( const CSteamBrowser& src );     /**< not implemented */

    private:

    mutable MT::CMutex m_lock;
    CORE::CString::StringVector m_queue;
    CORE::CString::StringVector m_urls;
    vector< int > m_statuses;
    UInt32 m_running;
    CORE::CIBrowserLauncherHandler* m_previous;
    bool m_installed;
    bool m_started;
    WEB::CHTTPClient m_client;
    CORE::CString m_steamId;
    CORE::CString m_origin;
    CORE::CString m_claimedId;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The plugin is in the process for as long as this lives, the way the application has it loaded; the home folder of the machine is a made up one without Steam */
class CLoadedSteamPlugin
{
    public:

    bool loaded;
    CFakeHostView machine;

    CLoadedSteamPlugin( void )
        : loaded( LoadTestPlugin( "authpluginSTEAM", "authSteamOpenId2Test" ) ), machine()
    {
        if ( loaded && GUCEF_NULL != GetSteamOpenId2Profile() )
            GetSteamOpenId2Profile()->SetHostView( &machine );
    }

    ~CLoadedSteamPlugin()
    {
        if ( loaded && GUCEF_NULL != GetSteamOpenId2Profile() )
            GetSteamOpenId2Profile()->SetHostView( GUCEF_NULL );
        CORE::CCoreGlobal::Instance()->GetPluginControl().UnloadPluginGroup( "authSteamOpenId2Test" );
        CORE::CCoreGlobal::Instance()->GetPluginControl().ClearMetaDataFromGroup( "authSteamOpenId2Test" );
    }

    /** A Steam client of Linux with one user that has used it: the loginusers.vdf of the client */
    void InstallClient( const CORE::CString& persona, const CORE::CString& account, bool running )
    {
        machine.AddFile( "/home/test/.steam/steam/config/loginusers.vdf", "\"users\" { \"76561197972611406\" { \"AccountName\" \"" + account + "\" \"PersonaName\" \"" + persona + "\" \"MostRecent\" \"1\" \"Timestamp\" \"1700000000\" } }" );
        machine.AddDir( "/home/test/.steam/steam" );
        machine.processes.clear();
        machine.files.erase( "/home/test/.steam/steam.pid" );
        if ( running )
        {
            machine.AddFile( "/home/test/.steam/steam.pid", "4242" );
            machine.processes[ "4242" ] = "steam";
        }
    }

    private:

    CLoadedSteamPlugin( const CLoadedSteamPlugin& src );
    CLoadedSteamPlugin& operator=( const CLoadedSteamPlugin& src );
};

/*-------------------------------------------------------------------------*/

static bool
Contains( const CORE::CString& text, const CORE::CString& part )
{
    return text.HasSubstr( part, true ) >= 0;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformSteamOpenId2Tests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING Steam OpenID 2.0 TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "SteamOpenId2" );

    GUCEF_TESTFW_TESTCASE( "Test: The claimed id of Steam has to be the endpoint's origin, /openid/id/ and a number of at most 64 bits, and the identity the same" )
        try
        {
            const CORE::CString endpoint = "https://steamcommunity.com/openid/login";
            CORE::CString id;
            ASSERT_TRUE( CSteamOpenId2Profile::ExtractSteamId( endpoint, "https://steamcommunity.com/openid/id/76561198000000042", id ) && "76561198000000042" == id );
            ASSERT_TRUE( CSteamOpenId2Profile::ExtractSteamId( endpoint, "https://steamcommunity.com/openid/id/18446744073709551615", id ) && "18446744073709551615" == id );
            const char* refused[] =
            {
                "https://steamcommunity.com/openid/id/18446744073709551616", "https://steamcommunity.com/openid/id/99999999999999999999", "https://steamcommunity.com/openid/id/",
                "https://steamcommunity.com/openid/id/abc", "https://steamcommunity.com/openid/id/12ab", "https://steamcommunity.com/openid/id/0123", "https://steamcommunity.com/openid/id/-1",
                "https://steamcommunity.com/openid/id/123/evil", "https://steamcommunity.com/openid/id/123?x=1", "https://steamcommunity.com/openid/id/123 ", "http://steamcommunity.com/openid/id/123",
                "https://steamcommunity.com.evil.example/openid/id/123", "https://evil.example/openid/id/123", "https://steamcommunity.com/openid/ID/123", "https://steamcommunity.com/openid/id/123456789012345678901",
                "https://user@steamcommunity.com/openid/id/123", "", "76561198000000042"
            };
            for ( size_t i=0; i<sizeof( refused ) / sizeof( refused[ 0 ] ); ++i )
            {
                ASSERT_FALSE( CSteamOpenId2Profile::ExtractSteamId( endpoint, refused[ i ], id ) );
                ASSERT_TRUE( id.IsNULLOrEmpty() );
            }
            // the origin is the one of the endpoint that was configured
            ASSERT_TRUE( CSteamOpenId2Profile::ExtractSteamId( "http://127.0.0.1:4000/openid/login", "http://127.0.0.1:4000/openid/id/5", id ) && "5" == id );
            ASSERT_FALSE( CSteamOpenId2Profile::ExtractSteamId( "http://127.0.0.1:4000/openid/login", "http://127.0.0.1:4001/openid/id/5", id ) );

            // the profile: the subject of the account is the number, and the identity is the claimed id
            CSteamOpenId2Profile profile;
            CORE::CString subject;
            const CORE::CString claimed = "https://steamcommunity.com/openid/id/76561198000000042";
            ASSERT_TRUE( profile.ExtractSubject( endpoint, claimed, claimed, subject ) && "76561198000000042" == subject );
            ASSERT_FALSE( profile.ExtractSubject( endpoint, claimed, "https://steamcommunity.com/openid/id/76561198000000043", subject ) );
            ASSERT_TRUE( subject.IsNULLOrEmpty() );
            ASSERT_FALSE( profile.ExtractSubject( endpoint, claimed, CORE::CString::Empty, subject ) );
            ASSERT_TRUE( "steam" == profile.GetProviderName() && profile.IsRelayAllowed() );
            AUTH::COpenId2Assertion assertion;
            CORE::CString name = "stale";
            ASSERT_FALSE( profile.GetDisplayName( assertion, name ) );
            ASSERT_TRUE( name.IsNULLOrEmpty() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The descriptor of Steam is what the plugin says Steam is: OpenID 2.0, the account namespace steam, no client id, and the earlier name of the provider" )
        try
        {
            AUTH::CAuthProviderDescriptor d = CSteamOpenId2Profile::MakeDescriptor();
            ASSERT_TRUE( d.IsWellFormed() && "steam" == d.name && "Steam" == d.displayName && "steam" == d.accountProviderId );
            ASSERT_TRUE( AUTH::AUTHPROTOCOL_OPENID2 == d.protocol && AUTH::AUTHIDSCOPE_GLOBAL == d.idScope );
            ASSERT_TRUE( "https://steamcommunity.com/openid/login" == d.authorizationEndpoint && d.requiredParameters.empty() && !d.usesPkce && d.publicClient );
            ASSERT_TRUE( 0 != ( d.capabilities & AUTH::AUTHCAP_BROWSER ) && 0 != ( d.capabilities & AUTH::AUTHCAP_SERVER_VERIFIABLE ) );
            ASSERT_TRUE( 0 == ( d.capabilities & AUTH::AUTHCAP_REQUIRES_CLIENT_ID ) && 0 == ( d.capabilities & AUTH::AUTHCAP_REFRESHABLE ) );
            ASSERT_TRUE( 1 == d.formerNames.size() && "steam-openid" == d.formerNames[ 0 ] );
            // the subject of the account reference is the steamid claim
            CORE::CString::StringMap claims;
            claims[ "steamid" ] = "76561198000000000";
            AUTH::CAccountRef ref;
            ASSERT_TRUE( d.MakeAccountRef( claims, CORE::CString::StringMap(), ref ) && "steam:76561198000000000" == ref.ToString() );
            ASSERT_FALSE( d.MakeAccountRef( CORE::CString::StringMap(), CORE::CString::StringMap(), ref ) );
            // it does not take a login hint
            ASSERT_FALSE( d.supportsLoginHint );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The profile of Steam says that Steam is installed and who used it, and keeps that for a moment; the provider asks it" )
        try
        {
            CLoadedSteamPlugin plugin;
            ASSERT_TRUE( plugin.loaded && GUCEF_NULL != GetSteamOpenId2Profile() );
            AUTH::CIAuthProvider* provider = AUTH::CAuthGlobal::Instance()->GetManager().GetProvider( "steam" );
            ASSERT_TRUE( GUCEF_NULL != provider );
            CORE::CString name = "stale";
            bool running = true;

            // no Steam on the machine
            ASSERT_FALSE( provider->GetDetectedAccountHint( name, running ) );
            ASSERT_TRUE( name.IsNULLOrEmpty() && !running );

            // installed, not running; the answer of a moment ago is what is given until it is forgotten
            plugin.InstallClient( "Alice P", "alice", false );
            GetSteamOpenId2Profile()->ForgetDetectedAccountHint();
            ASSERT_TRUE( provider->GetDetectedAccountHint( name, running ) && "Alice P" == name && !running );
            plugin.InstallClient( "Alice P", "alice", true );
            ASSERT_TRUE( provider->GetDetectedAccountHint( name, running ) && "Alice P" == name && !running );
            static_cast< AUTH::CDescriptorAuthProvider* >( provider )->ForgetDetectedAccountHint();
            ASSERT_TRUE( provider->GetDetectedAccountHint( name, running ) && "Alice P" == name && running );

            // the name of the account when the persona is empty
            plugin.InstallClient( "", "alice", true );
            GetSteamOpenId2Profile()->ForgetDetectedAccountHint();
            ASSERT_TRUE( provider->GetDetectedAccountHint( name, running ) && "alice" == name );

            // not installed any more
            plugin.machine.files.clear();
            plugin.machine.dirs.clear();
            plugin.machine.processes.clear();
            GetSteamOpenId2Profile()->ForgetDetectedAccountHint();
            name = "stale";
            running = true;
            ASSERT_FALSE( provider->GetDetectedAccountHint( name, running ) );
            ASSERT_TRUE( name.IsNULLOrEmpty() && !running );

            // after the life time of a detection it is looked at again
            plugin.InstallClient( "Bob Q", "bob", false );
            ASSERT_FALSE( provider->GetDetectedAccountHint( name, running ) );
            MT::ThreadDelay( 3200 );
            ASSERT_TRUE( provider->GetDetectedAccountHint( name, running ) && "Bob Q" == name );

            // a hint is no account: nothing is logged in and nothing is kept
            AUTH::CAuthManager& manager = AUTH::CAuthGlobal::Instance()->GetManager();
            ASSERT_TRUE( AUTH::AUTHLOGINSTATE_GUEST == manager.GetLoginState() );
            CORE::CDataNode providerNode( "Provider" );
            ASSERT_TRUE( provider->SaveConfig( providerNode ) );
            CORE::CDataNode managerNode( "Auth" );
            ASSERT_TRUE( manager.SaveConfig( managerNode ) );
            ASSERT_TRUE( providerNode.FindChild( "Parameters" ) != GUCEF_NULL );
            for ( CORE::CDataNode::TAttributeMap::const_iterator a = providerNode.FindChild( "Parameters" )->AttributeBegin(); a != providerNode.FindChild( "Parameters" )->AttributeEnd(); ++a )
                ASSERT_FALSE( Contains( (*a).second.AsString(), "Bob" ) || Contains( (*a).second.AsString(), "bob" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Settings kept under steam-openid, the name of the browser login before it became part of the provider steam, are the settings of steam, relay included" )
        try
        {
            AUTH::CAuthManager& manager = AUTH::CAuthGlobal::Instance()->GetManager();
            AUTH::CAuthSettings saved = manager.GetSettings();

            // a configuration that was written for the old name, and loaded before the plugin
            AUTH::CAuthSettings settings = saved;
            AUTH::CAuthProviderSettings old;
            old.parameters[ "relayUrl" ] = "https://login.example.com/steam-return";
            settings.SetProviderSettings( "steam-openid", old );
            AUTH::CAuthProviderSettings sdk;
            sdk.parameters[ "appId" ] = "480";
            sdk.parameters[ "runtimeLibrary" ] = "libsteam_api_does_not_exist_zz.so";
            settings.SetProviderSettings( "steam", sdk );
            settings.enabledProviders.push_back( "steam-openid" );
            manager.SetSettings( settings );
            AUTH::CAuthProviderSettings found;
            ASSERT_TRUE( manager.GetSettings().GetProviderSettings( "steam-openid", found ) );

            {
                CLoadedSteamPlugin plugin;
                ASSERT_TRUE( plugin.loaded );
                // the plugin knows the old name: the settings are the ones of steam now, and the settings of both are there
                ASSERT_FALSE( manager.GetSettings().GetProviderSettings( "steam-openid", found ) );
                ASSERT_TRUE( manager.GetSettings().GetProviderSettings( "steam", found ) );
                ASSERT_TRUE( "https://login.example.com/steam-return" == found.GetParameter( "relayUrl" ) && "480" == found.GetParameter( "appId" ) );
                ASSERT_TRUE( manager.GetSettings().enabledProviders.size() == saved.enabledProviders.size() + 1 && AUTH::ContainsString( manager.GetSettings().enabledProviders, "steam" ) );
                // the provider was configured with them
                CSteamAuthProvider* steam = static_cast< CSteamAuthProvider* >( manager.GetProvider( "steam" ) );
                ASSERT_TRUE( GUCEF_NULL != steam );
                ASSERT_TRUE( "480" == steam->GetSteamSettings().appId );
                ASSERT_TRUE( "https://login.example.com/steam-return" == steam->GetSettings().GetParameter( "relayUrl" ) );
                ASSERT_TRUE( manager.IsProviderOffered( "steam" ) );

                // what the manager writes uses the name steam
                CORE::CDataNode node( "Auth" );
                ASSERT_TRUE( manager.GetSettings().SaveConfig( node ) );
                const CORE::CDataNode* providers = node.FindChild( "Providers" );
                ASSERT_TRUE( GUCEF_NULL != providers && GUCEF_NULL != providers->FindChild( "steam" ) && GUCEF_NULL == providers->FindChild( "steam-openid" ) );
            }
            manager.SetSettings( saved );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The login in the browser through the plugin: the Steam provider asks the browser, takes the assertion at the loopback address, has it checked at the provider, and gives steam:<id> with the proof" )
        GUCEF_TESTFW_SKIP_IF( !JsonCodecAvailable(), "The json codec is not available" );
        try
        {
            CFakeHttpServer provider;
            ASSERT_TRUE( provider.Start() );
            provider.SetResponse( 200, "ns:http://specs.openid.net/auth/2.0\nis_valid:true\n", "text/plain" );
            CSteamBrowser browser;
            ASSERT_TRUE( browser.Start() );
            browser.Install();
            browser.SetIdentity( "76561198000000042", provider.GetBaseUrl() );

            CLoadedSteamPlugin plugin;
            ASSERT_TRUE( plugin.loaded );
            AUTH::CAuthManager& manager = AUTH::CAuthGlobal::Instance()->GetManager();
            AUTH::COAuth2Engine* engine = AUTH::CAuthGlobal::Instance()->GetOAuth2Engine();
            AUTH::COAuth2EngineSettings savedEngine = engine->GetSettings();
            AUTH::COAuth2EngineSettings engineSettings = savedEngine;
            engineSettings.allowInsecureHttp = true;
            engine->SetSettings( engineSettings );
            engine->ClearCaches();
            AUTH::CAuthSettings savedSettings = manager.GetSettings();

            // the endpoint is the fake one; no Steam client is on this machine
            AUTH::CAuthSettings settings = savedSettings;
            AUTH::CAuthProviderSettings steamSettings;
            steamSettings.authorizationEndpoint = provider.GetBaseUrl() + "/openid/login";
            steamSettings.parameters[ "runtimeLibrary" ] = "libsteam_api_does_not_exist_zz.so";
            settings.SetProviderSettings( "steam", steamSettings );
            manager.SetSettings( settings );
            ASSERT_TRUE( manager.IsProviderOffered( "steam" ) );
            CSteamAuthProvider* steam = static_cast< CSteamAuthProvider* >( manager.GetProvider( "steam" ) );
            ASSERT_TRUE( GUCEF_NULL != steam && !steam->IsClientAvailable() );

            CProviderRecorder recorder;
            recorder.SubscribeTo( steam );
            CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            ASSERT_TRUE( manager.Login( "steam", AUTH::CAuthOptions(), future ) );
            ASSERT_TRUE( PumpUntilReady( future, 15000 ) );
            AUTH::CAuthResult result;
            ASSERT_TRUE( AUTH::CAuthResult::TryGetFromFuture( future, result ) );
            ASSERT_TRUE( "steam:76561198000000042" == result.accountRef.ToString() && "steam" == result.providerName );
            ASSERT_TRUE( AUTH::CAuthProof::KindOpenId2Assertion == result.proof.kind && !result.proof.data.IsEmpty() );

            // the address the browser was given: checkid_setup at the endpoint, with the loopback address as realm and return address
            ASSERT_TRUE( 1 == browser.GetOpenCount() );
            CORE::CString scheme, host, path, query;
            UInt16 port = 0;
            CORE::CString::StringMap asked;
            bool duplicate = false;
            ASSERT_TRUE( AUTH::CAuthUrl::Split( browser.GetLastUrl(), scheme, host, port, path, query ) && AUTH::CAuthUrl::ParseQuery( query, asked, duplicate ) );
            ASSERT_TRUE( "/openid/login" == path && "checkid_setup" == asked[ "openid.mode" ] && asked[ "openid.return_to" ].StartsWith( "http://127.0.0.1:" ) && asked[ "openid.return_to" ].StartsWith( asked[ "openid.realm" ] ) );

            // the provider was asked to check the assertion, once, as a form that has check_authentication in it
            ASSERT_TRUE( 1 == provider.GetRequestCount() && Contains( provider.GetLastRequestLine(), "POST /openid/login" ) );
            ASSERT_TRUE( Contains( provider.GetLastRequestBody(), "openid.mode=check_authentication" ) );

            // the server that has the same plugin checks the proof: the same assertion is refused as a replay at the provider's side of the fake only when asked again
            provider.SetResponse( 200, "ns:http://specs.openid.net/auth/2.0\nis_valid:true\n", "text/plain" );
            AUTH::CAuthResult verified;
            ASSERT_TRUE( steam->Verify( result.proof, verified ) );
            ASSERT_TRUE( "steam:76561198000000042" == verified.accountRef.ToString() && "steam" == verified.providerName );
            ASSERT_TRUE( 2 == provider.GetRequestCount() );
            // a proof of a kind that the provider does not take, and an assertion the provider says is not valid
            ASSERT_FALSE( steam->Verify( AUTH::CAuthProof( AUTH::CAuthProof::KindOidcIdToken, result.proof.data.Reveal() ), verified ) );
            provider.SetResponse( 200, "ns:http://specs.openid.net/auth/2.0\nis_valid:false\n", "text/plain" );
            AUTH::COpenId2Assertion again;
            ASSERT_TRUE( again.LoadFromQueryString( result.proof.data.Reveal() ) );
            CORE::CString::StringMap fields = again.fields;
            fields[ "openid.response_nonce" ] = CORE::CDateTime::NowUTCDateTime().ToIso8601DateTimeString( true, false ) + ( CORE::CDateTime::NowUTCDateTime().ToIso8601DateTimeString( true, false ).EndsWith( "Z" ) ? "" : "Z" ) + "second-nonce";
            again.Load( fields );
            ASSERT_FALSE( steam->Verify( AUTH::CAuthProof( AUTH::CAuthProof::KindOpenId2Assertion, again.ToQueryString() ), verified ) );

            // the account of another place is refused: not the origin of the endpoint
            browser.SetIdentity( "76561198000000042", provider.GetBaseUrl() );
            browser.SetClaimedId( "https://steamcommunity.com/openid/id/76561198000000042" );
            provider.SetResponse( 200, "ns:http://specs.openid.net/auth/2.0\nis_valid:true\n", "text/plain" );
            manager.Unlink( result.accountRef );
            CORE::CFutureResult refusedFuture( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            const UInt32 requestsBefore = provider.GetRequestCount();
            ASSERT_TRUE( manager.Login( "steam", AUTH::CAuthOptions(), refusedFuture ) );
            ASSERT_TRUE( PumpUntilReady( refusedFuture, 15000 ) );
            AUTH::CAuthResult none;
            ASSERT_FALSE( AUTH::CAuthResult::TryGetFromFuture( refusedFuture, none ) );
            ASSERT_TRUE( requestsBefore == provider.GetRequestCount() );        // refused before the provider was asked

            browser.Uninstall();
            engine->SetSettings( savedEngine );
            engine->ClearCaches();
            manager.SetSettings( savedSettings );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
