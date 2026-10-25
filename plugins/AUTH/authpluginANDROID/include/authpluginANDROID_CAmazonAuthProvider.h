/*
 *  authpluginANDROID: gucefAUTH plugin for logging in on Android devices
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

#ifndef GUCEF_AUTHPLUGIN_ANDROID_CAMAZONAUTHPROVIDER_H
#define GUCEF_AUTHPLUGIN_ANDROID_CAMAZONAUTHPROVIDER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTFACTORY_H
#include "CTFactory.h"
#define GUCEF_CORE_CTFACTORY_H
#endif /* GUCEF_CORE_CTFACTORY_H ? */

#ifndef GUCEF_AUTHPLUGIN_ANDROID_CANDROIDAUTHPROVIDERBASE_H
#include "authpluginANDROID_CAndroidAuthProviderBase.h"
#define GUCEF_AUTHPLUGIN_ANDROID_CANDROIDAUTHPROVIDERBASE_H
#endif /* GUCEF_AUTHPLUGIN_ANDROID_CANDROIDAUTHPROVIDERBASE_H ? */

#ifndef GUCEF_AUTHPLUGIN_ANDROID_CANDROIDAUTHSETTINGS_H
#include "authpluginANDROID_CAndroidAuthSettings.h"
#define GUCEF_AUTHPLUGIN_ANDROID_CANDROIDAUTHSETTINGS_H
#endif /* GUCEF_AUTHPLUGIN_ANDROID_CANDROIDAUTHSETTINGS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUTHPLUGIN {
namespace ANDROIDAUTH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Logs in with Login with Amazon on Fire OS devices: provider "amazon", account references "amazon:<user_id>", the same
 *  account that the OAuth based "amazon" descriptor gives (the user_id of the Login with Amazon profile).
 *  Capabilities: AUTHCAP_SILENT and AUTHCAP_SERVER_VERIFIABLE. The Login with Amazon SDK identifies the app by its API
 *  key (assets/api_key.txt of the app), so the client needs no setting; the server that verifies needs the client id of
 *  the security profile (clientId).
 *
 *  Client. The Java side (GucefAuthBridge) authorizes the scopes of the setting amazonScopes with AuthorizationManager;
 *  a silent login only takes a token that was authorized before (AuthorizationManager.getToken) and the user id of
 *  User.fetch. The result has the account, the name, the access token and as the proof the access token (kind
 *  platform-ticket).
 *
 *  Server. Verify() asks the token info endpoint of Amazon about the access token and requires that the audience is the
 *  client id of the settings (a token that was issued to another app is refused), that the token has not run out and that
 *  there is a user id. The access token travels in the query of the request, over https only, and is not logged.
 *
 *  UNVERIFIED AGAINST THE REAL SERVICES: the Java side could not be compiled here and nothing was tried against Amazon.
 *  The names of the members of the answer of the token info endpoint, in particular that "exp" is the time left in
 *  seconds, are to be confirmed against the documentation of Login with Amazon. The tests run on a fake table of
 *  functions and a fake token info endpoint.
 */
class GUCEF_AUTHPLUGIN_ANDROID_EXPORT_CPP CAmazonAuthProvider : public CAndroidAuthProviderBase
{
    public:

    static const CORE::CString ProviderName;        /**< "amazon" */

    CAmazonAuthProvider( void );

    virtual ~CAmazonAuthProvider();

    virtual const CORE::CString& GetProviderName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual UInt32 GetCapabilities( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsAvailable( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool TrySilentLogin( CORE::CFutureResult& future ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool BeginLogin( const AUTH::CAuthOptions& options, CORE::CFutureResult& future ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Logout( const AUTH::CAccountRef& account, const AUTH::CSecretString& refreshToken ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Verify( const AUTH::CAuthProof& proof, AUTH::CAuthResult& result ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool SaveConfig( CORE::CDataNode& config ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool LoadConfig( const CORE::CDataNode& config ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** A copy of the settings in use */
    CAndroidAuthSettings GetSettings( void ) const;

    protected:

    virtual bool CanStart( bool interactive, CORE::CString& reason ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool StartNative( UInt32 requestId, bool interactive, const CORE::CString& nonce ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void HandleResult( SPending& pending, const SAndroidAuthResult& result ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    CAmazonAuthProvider( const CAmazonAuthProvider& src );
    CAmazonAuthProvider& operator=( const CAmazonAuthProvider& src );

    private:

    CAndroidAuthSettings m_settings;
};

/*-------------------------------------------------------------------------*/

typedef CORE::CTFactory< AUTH::CIAuthProvider, CAmazonAuthProvider, MT::CMutex > TAmazonAuthProviderFactory;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ANDROIDAUTH */
}; /* namespace AUTHPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUTHPLUGIN_ANDROID_CAMAZONAUTHPROVIDER_H ? */
