/*
 *  authpluginEOS: gucefAUTH plugin for logging in with Epic Online Services
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

#ifndef GUCEF_AUTHPLUGIN_EOS_CEOSAUTHPROVIDER_H
#define GUCEF_AUTHPLUGIN_EOS_CEOSAUTHPROVIDER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_COBSERVER_H
#include "CObserver.h"
#define GUCEF_CORE_COBSERVER_H
#endif /* GUCEF_CORE_COBSERVER_H ? */

#ifndef GUCEF_CORE_CTFACTORY_H
#include "CTFactory.h"
#define GUCEF_CORE_CTFACTORY_H
#endif /* GUCEF_CORE_CTFACTORY_H ? */

#ifndef GUCEF_AUTH_CIAUTHPROVIDER_H
#include "gucefAUTH_CIAuthProvider.h"
#define GUCEF_AUTH_CIAUTHPROVIDER_H
#endif /* GUCEF_AUTH_CIAUTHPROVIDER_H ? */

#ifndef GUCEF_PLUGINGLUE_EOS_CEOSGLUE_H
#include "pluginglueEOS_CEosGlue.h"
#define GUCEF_PLUGINGLUE_EOS_CEOSGLUE_H
#endif /* GUCEF_PLUGINGLUE_EOS_CEOSGLUE_H ? */

#ifndef GUCEF_AUTHPLUGIN_EOS_EOSSHIM_H
#include "authpluginEOS_EosShim.h"
#define GUCEF_AUTHPLUGIN_EOS_EOSSHIM_H
#endif /* GUCEF_AUTHPLUGIN_EOS_EOSSHIM_H ? */

#ifndef GUCEF_AUTHPLUGIN_EOS_CEOSSDK_H
#include "authpluginEOS_CEosSdk.h"
#define GUCEF_AUTHPLUGIN_EOS_CEOSSDK_H
#endif /* GUCEF_AUTHPLUGIN_EOS_CEOSSDK_H ? */

#ifndef GUCEF_AUTHPLUGIN_EOS_CEOSAUTHSETTINGS_H
#include "authpluginEOS_CEosAuthSettings.h"
#define GUCEF_AUTHPLUGIN_EOS_CEOSAUTHSETTINGS_H
#endif /* GUCEF_AUTHPLUGIN_EOS_CEOSAUTHSETTINGS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUTHPLUGIN {
namespace EOS {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Logs in with Epic Online Services: provider "epic", account references "epic:<EpicAccountId>", the same account
 *  that the OAuth based "epic" descriptor gives (the sub claim of the Epic id token is the Epic account id).
 *  Capabilities: AUTHCAP_SILENT (the persistent auth token of an earlier login), AUTHCAP_BROWSER (the account portal,
 *  which the SDK opens itself), AUTHCAP_DEVICE_CODE, AUTHCAP_SERVER_VERIFIABLE and AUTHCAP_REQUIRES_CLIENT_ID.
 *
 *  Login. EOS_Auth_Login ends in the Epic account id; the id token of that account (EOS_Auth_CopyIdToken) is the proof
 *  of the login, kind oidc-id-token. Then EOS_Connect_Login with that id token gives the Product User Id (PUID) and
 *  creates the product user when the account has none (settings useConnect and createConnectUser). The PUID is scoped to
 *  the product: it is NOT the subject of the account reference and is only available through GetProductUserId().
 *  The SDK delivers the end of every step as a callback inside EOS_Platform_Tick. The platform is not the provider's
 *  own: it is the one of the glue (pluginglueEOS), shared with the lobby and P2P plugins. The glue ticks it from a
 *  timer on the pulse generator of the application, at the pace the provider asks for while a login is under way,
 *  never from a thread of its own. The provider tells the glue who is logged in (the Epic account and the Product User
 *  Id) when a login ends, which is how the other EOS plugins find the user.
 *
 *  Server. Verify() hands the id token to the OIDC engine of gucefAUTH (CAuthGlobal::GetFlowEngine()) with the
 *  built in "epic" descriptor: signature against the JWKS of Epic, issuer, audience (the client id), expiry. Without
 *  an engine the provider reports that it cannot verify.
 *
 *  Without the EOS SDK in the build the provider is a stub: it registers, reports that it is not available and
 *  never starts a login. See CEosSdk for how the SDK is added to a build.
 *
 *  UNVERIFIED AGAINST THE REAL SERVICE AND SDK: only tested with a fake SDK. An EOS login that is under way (account
 *  portal) cannot be cancelled in the SDK: a cancelled or timed out login ends for the requester at once, a late
 *  completion of the SDK is dropped.
 */
class GUCEF_AUTHPLUGIN_EOS_EXPORT_CPP CEosAuthProvider : public AUTH::CIAuthProvider
{
    public:

    static const CORE::CString ProviderName;        /**< "epic" */

    /** A provider on the SDK that is compiled into the build, if any */
    CEosAuthProvider( void );

    /** A provider on the given table of functions, for tests and for applications that bind the SDK themselves */
    explicit CEosAuthProvider( const TEosShim& shim );

    virtual ~CEosAuthProvider();

    /**
     *  Makes the provider use the given table of functions from now on, instead of the SDK that is compiled into the build:
     *  for an application that binds the SDK itself, and for a provider that a factory made. Before the first login
     */
    void SetShim( const TEosShim& shim );

    virtual const CORE::CString& GetProviderName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual UInt32 GetCapabilities( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsAvailable( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** The platform of the glue this provider holds, NULL until the first login made it */
    PLUGINGLUE::EOS::CEosPlatform* GetPlatform( void ) const;

    virtual bool TrySilentLogin( CORE::CFutureResult& future ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool BeginLogin( const AUTH::CAuthOptions& options, CORE::CFutureResult& future ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Logout( const AUTH::CAccountRef& account, const AUTH::CSecretString& refreshToken ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Verify( const AUTH::CAuthProof& proof, AUTH::CAuthResult& result ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool SaveConfig( CORE::CDataNode& config ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool LoadConfig( const CORE::CDataNode& config ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** A copy of the settings in use */
    CEosAuthSettings GetSettings( void ) const;

    /**
     *  The Product User Id of a product user that is logged in already, for example the one that a login with another
     *  provider through EOS Connect made. When the next Epic login finds that the account has no product user yet, the
     *  account is linked to this one (EOS_Connect_LinkAccount) instead of a new product user being created. Empty goes
     *  back to creating. Used for one login at a time: it is cleared when the login that used it ends
     */
    void SetLinkTargetProductUserId( const CORE::CString& productUserId );

    /** The Product User Id of EOS Connect of an account that logged in, empty when there is none. The PUID is different for every product */
    CORE::CString GetProductUserId( const AUTH::CAccountRef& account ) const;

    /** Whether Verify() can do its work: there is an OIDC engine that supports the "epic" descriptor with these settings */
    bool IsVerifySupported( void ) const;

    /** The number of logins under way */
    UInt32 GetPendingLoginCount( void ) const;

    private:

    CEosAuthProvider( const CEosAuthProvider& src );
    CEosAuthProvider& operator=( const CEosAuthProvider& src );

    class CPump : public CORE::CObserver
    {
        public:
        explicit CPump( CEosAuthProvider& owner );
        virtual ~CPump();
        virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;
        protected:
        virtual void OnNotify( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData ) GUCEF_VIRTUAL_OVERRIDE;
        private:
        CEosAuthProvider& m_owner;
    };
    friend class CPump;

    enum ELoginStage
    {
        STAGE_AUTH = 0,
        STAGE_CONNECT,
        STAGE_CREATE_USER,
        STAGE_LINK_ACCOUNT
    };

    struct SLogin
    {
        UInt32 requestId;
        CORE::CFutureResultSource source;
        ELoginStage stage;
        UInt64 deadlineInMs;
        CORE::CString epicAccountId;
        CORE::CString linkTarget;
        AUTH::CSecretString idToken;
    };
    typedef GUCEF::vector< SLogin > TLoginVector;
    typedef GUCEF::map< AUTH::CAccountRef, CORE::CString > TProductUserIdMap;

    bool Start( EEosLoginType type, bool interactive, UInt32 timeoutInMs, CORE::CFutureResult& future );
    bool EnsurePlatform( CORE::CString& reason );
    bool IsSdkAvailable( void ) const;
    void ReleasePlatform( void );
    void OnTimer( void );
    void UpdateTickDemand( void );
    void HandleEvent( const SEosEvent& event );
    void EndLogin( SLogin& login, const CORE::CString& productUserId );
    static AUTH::TAuthFailureReason ToFailureReason( EEosOutcome outcome );

    private:

    mutable MT::CMutex m_lock;
    CEosAuthSettings m_settings;
    CEosSdk m_sdk;
    TEosShim m_shim;
    PLUGINGLUE::EOS::CEosPlatform* m_platform;
    UInt32 m_tickDemand;
    UInt64 m_lastRunInMs;
    CORE::CString m_loginStateAccount;
    TLoginVector m_logins;
    TProductUserIdMap m_productUserIds;
    UInt32 m_nextRequestId;
    UInt32 m_idleTicksLeft;
    CORE::CString m_linkTarget;
    CPump m_pump;
};

/*-------------------------------------------------------------------------*/

typedef CORE::CTFactory< AUTH::CIAuthProvider, CEosAuthProvider, MT::CMutex > TEosAuthProviderFactory;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace EOS */
}; /* namespace AUTHPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUTHPLUGIN_EOS_CEOSAUTHPROVIDER_H ? */
