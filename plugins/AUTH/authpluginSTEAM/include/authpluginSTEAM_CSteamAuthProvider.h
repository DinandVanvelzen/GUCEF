/*
 *  authpluginSTEAM: gucefAUTH plugin for logging in with the Steam client
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

#ifndef GUCEF_AUTHPLUGIN_STEAM_CSTEAMAUTHPROVIDER_H
#define GUCEF_AUTHPLUGIN_STEAM_CSTEAMAUTHPROVIDER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_CTIMER_H
#include "CTimer.h"
#define GUCEF_CORE_CTIMER_H
#endif /* GUCEF_CORE_CTIMER_H ? */

#ifndef GUCEF_CORE_COBSERVER_H
#include "CObserver.h"
#define GUCEF_CORE_COBSERVER_H
#endif /* GUCEF_CORE_COBSERVER_H ? */

#ifndef GUCEF_CORE_CTFACTORY_H
#include "CTFactory.h"
#define GUCEF_CORE_CTFACTORY_H
#endif /* GUCEF_CORE_CTFACTORY_H ? */

#ifndef GUCEF_AUTH_CDESCRIPTORAUTHPROVIDER_H
#include "gucefAUTH_CDescriptorAuthProvider.h"
#define GUCEF_AUTH_CDESCRIPTORAUTHPROVIDER_H
#endif /* GUCEF_AUTH_CDESCRIPTORAUTHPROVIDER_H ? */

#ifndef GUCEF_AUTHPLUGIN_STEAM_STEAMSHIM_H
#include "authpluginSTEAM_SteamShim.h"
#define GUCEF_AUTHPLUGIN_STEAM_STEAMSHIM_H
#endif /* GUCEF_AUTHPLUGIN_STEAM_STEAMSHIM_H ? */

#ifndef GUCEF_AUTHPLUGIN_STEAM_CSTEAMRUNTIME_H
#include "authpluginSTEAM_CSteamRuntime.h"
#define GUCEF_AUTHPLUGIN_STEAM_CSTEAMRUNTIME_H
#endif /* GUCEF_AUTHPLUGIN_STEAM_CSTEAMRUNTIME_H ? */

#ifndef GUCEF_AUTHPLUGIN_STEAM_CSTEAMAUTHSETTINGS_H
#include "authpluginSTEAM_CSteamAuthSettings.h"
#define GUCEF_AUTHPLUGIN_STEAM_CSTEAMAUTHSETTINGS_H
#endif /* GUCEF_AUTHPLUGIN_STEAM_CSTEAMAUTHSETTINGS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUTHPLUGIN {
namespace STEAM {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The Steam backend as one login provider: the provider "steam", account references "steam:<SteamID64>". It is the
 *  Steam plugin's own provider and the only place that knows how Steam logs in; the module (gucefAUTH) has no Steam in it.
 *  Capabilities: AUTHCAP_BROWSER | AUTHCAP_SILENT | AUTHCAP_SERVER_VERIFIABLE.
 *
 *  What it is. A CDescriptorAuthProvider for the descriptor "steam" (OpenID 2.0 in the browser, see CSteamOpenId2Profile), which
 *  gives the login in the browser and the check of its proof by the engine of the module, plus the login through the Steam
 *  client that runs next to the application, which this class adds. An offer to log in with Steam is one offer: BeginLogin()
 *  uses the Steam client when it can be used, and the browser when it cannot.
 *
 *  Client login (the Steamworks SDK). The login asks Steam for a ticket (GetAuthTicketForWebApi, or GetAuthSessionTicket
 *  for runtimes without it), without the user doing anything, and ends with the account, the persona name and a proof of kind
 *  platform-ticket whose data is the ticket as hex. Steam delivers the web api ticket by a callback; the plugin dispatches the
 *  callbacks of Steam from a timer on the pulse generator of the application, only while a ticket is wanted, never from a thread
 *  of its own and never blocking. The tickets that were issued are cancelled again at Logout() and when the provider goes away.
 *
 *  Server. Verify() takes both kinds of proof of this provider. A ticket (platform-ticket) goes to the Steam Web API
 *  (ISteamUserAuth/AuthenticateUserTicket) with the publisher key of the settings (webApiKey, a secret that is never logged by
 *  this plugin and never leaves in anything but the request) and requires result OK and a steamid. It runs on the calling thread
 *  and waits for the network, servers call it from a worker. An assertion of the browser login (openid2-assertion) is checked by
 *  the engine of the module, against the endpoint of Steam, with the profile of this plugin. A server that has to accept Steam
 *  logins loads this plugin: without it there is no provider "steam" and a proof of it is refused like one of any unknown provider.
 *  NOTE: the Web API documents the key as a query parameter. gucefWEB writes the url of a request that failed to reach the
 *  server to the log at the debug level (CHttpClientRequestRunner), so in a debug build a failed verification request can
 *  show the key in the log. The setting webApiKeyPlacement "header" sends the key in the x-webapi-key header instead, which
 *  keeps it out of the url (that the Steam Web API accepts the header for this method is to be confirmed with Steam).
 *  With verifyMode "peer" the ticket of another player is checked on this client with BeginAuthSession/EndAuthSession.
 *
 *  Settings. The section of the provider (Auth/Providers/steam) has the keys of CAuthProviderSettings, and in its Parameters the keys
 *  of CSteamAuthSettings (appId, ticketIdentity, webApiKey, ...) and the parameter relayUrl of the browser login. The section was
 *  called steam-openid for the browser login: the descriptor lists that as its former name, and settings kept under it are read as
 *  those of "steam".
 *
 *  The client login is not available when the Steam runtime library cannot be loaded or the Steam client is not running;
 *  the provider is available nevertheless when the browser login is.
 *
 *  UNVERIFIED AGAINST REAL STEAM. The Steamworks SDK and a Steam client are not available where this was written. The
 *  behaviour against Steam is covered by tests with a fake Steam only; see CSteamRuntime for what is to be confirmed.
 */
class GUCEF_AUTHPLUGIN_STEAM_EXPORT_CPP CSteamAuthProvider : public AUTH::CDescriptorAuthProvider
{
    public:

    static const CORE::CString ProviderName;        /**< "steam" */

    /** A provider that uses the Steam runtime library of the machine, loaded when first needed */
    CSteamAuthProvider( void );

    /** A provider that uses the given table of functions, for a test with a fake Steam or an application with its own binding */
    explicit CSteamAuthProvider( const TSteamShim& shim );

    virtual ~CSteamAuthProvider();

    /**
     *  Makes the provider use the given table of functions from now on, instead of the Steam runtime library it would load
     *  itself: for an application that binds Steam itself, and for a provider that a factory made. Before the first login
     */
    void SetShim( const TSteamShim& shim );

    virtual const CORE::CString& GetProviderName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual UInt32 GetCapabilities( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** Available when the browser login is, or the Steam client can be used */
    virtual bool IsAvailable( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  Whether the login with the Steam client can be made: the runtime library is there and starts, the settings allow a
     *  ticket to be asked for. IsAvailable() is this or the login in the browser being possible.
     */
    bool IsClientAvailable( void ) const;

    /** The login without the user doing anything, with the Steam client. False when the client cannot be used */
    virtual bool TrySilentLogin( CORE::CFutureResult& future ) GUCEF_VIRTUAL_OVERRIDE;

    /** With the Steam client when it can be used (no browser step), else in the browser */
    virtual bool BeginLogin( const AUTH::CAuthOptions& options, CORE::CFutureResult& future ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Logout( const AUTH::CAccountRef& account, const AUTH::CSecretString& refreshToken ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Verify( const AUTH::CAuthProof& proof, AUTH::CAuthResult& result ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool SaveConfig( CORE::CDataNode& config ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool LoadConfig( const CORE::CDataNode& config ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** A copy of the Steam settings in use: the Parameters of the section of the provider that are of the Steam client login and the Web API */
    CSteamAuthSettings GetSteamSettings( void ) const;

    /**
     *  callbackMode "external": the application dispatches the callbacks of Steam and hands the
     *  GetTicketForWebApiResponse_t it receives to the provider here. Call it from the thread of the pulse generator
     */
    void OnTicketResponse( const SSteamTicketResponse& response );

    /**
     *  The server role for a client that is not a player of a game server but a peer: checks the ticket the peer
     *  presented against the Steam account it claims to be, with BeginAuthSession/EndAuthSession. The result holds the
     *  claimed account when the ticket is valid for it. Only for a result of the Steam client running here.
     */
    bool VerifyPeerTicket( const AUTH::CAccountRef& claimed, const AUTH::CAuthProof& proof, AUTH::CAuthResult& result );

    /**
     *  Reads the answer of ISteamUserAuth/AuthenticateUserTicket: true with the SteamID64 when it says result OK and has a steamid,
     *  false with a reason otherwise. The reason never holds anything but text of the answer's error description.
     */
    static bool ParseAuthenticateUserTicketResponse( const CORE::CString& body, CORE::CString& steamId, CORE::CString& reason );

    /** The number of tickets that are issued and not yet cancelled (logged in, not logged out) */
    UInt32 GetIssuedTicketCount( void ) const;

    /** The number of tickets that are still being waited for */
    UInt32 GetPendingTicketCount( void ) const;

    private:

    CSteamAuthProvider( const CSteamAuthProvider& src );
    CSteamAuthProvider& operator=( const CSteamAuthProvider& src );

    class CPump : public CORE::CObserver
    {
        public:
        explicit CPump( CSteamAuthProvider& owner );
        virtual ~CPump();
        virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;
        protected:
        virtual void OnNotify( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData ) GUCEF_VIRTUAL_OVERRIDE;
        private:
        CSteamAuthProvider& m_owner;
    };
    friend class CPump;

    struct SPending
    {
        UInt32 ticketHandle;
        CORE::CFutureResultSource source;
        UInt64 deadlineInMs;
        AUTH::CAccountRef account;
        CORE::CString displayName;
    };
    typedef GUCEF::vector< SPending > TPendingVector;
    typedef GUCEF::map< AUTH::CAccountRef, UInt32 > TIssuedMap;

    bool Start( bool interactive, CORE::CFutureResult& future );
    bool CanUseClient( void ) const;
    bool EnsureReady( CORE::CString& reason ) const;
    bool PrepareTicketRequest( bool& useWebApi, CORE::CString& reason ) const;
    void OnTimer( void );
    void UpdateTimer( void );
    void HandleResponse( const SSteamTicketResponse& response );
    void EndLogin( SPending& pending, const UInt8* ticket, UInt32 ticketSize );
    void RememberIssued( const AUTH::CAccountRef& account, UInt32 ticketHandle );
    bool VerifyWithWebApi( const AUTH::CAuthProof& proof, AUTH::CAuthResult& result );

    private:

    mutable MT::CMutex m_lock;
    CSteamAuthSettings m_steamSettings;
    mutable CSteamRuntime m_runtime;
    mutable TSteamShim m_shim;
    bool m_usesRuntime;
    mutable bool m_ready;
    mutable UInt64 m_lastFailureInMs;
    mutable bool m_hasFailed;
    mutable CORE::CString m_lastFailureReason;
    mutable bool m_weInitialized;
    TPendingVector m_pending;
    TIssuedMap m_issued;
    CORE::CTimer m_timer;
    CPump m_pump;
};

/*-------------------------------------------------------------------------*/

typedef CORE::CTFactory< AUTH::CIAuthProvider, CSteamAuthProvider, MT::CMutex > TSteamAuthProviderFactory;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace STEAM */
}; /* namespace AUTHPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUTHPLUGIN_STEAM_CSTEAMAUTHPROVIDER_H ? */
