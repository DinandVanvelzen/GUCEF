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

#ifndef GUCEF_AUTHPLUGIN_STEAM_CSTEAMOPENID2PROFILE_H
#define GUCEF_AUTHPLUGIN_STEAM_CSTEAMOPENID2PROFILE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_AUTH_IOPENID2PROVIDERPROFILE_H
#include "gucefAUTH_IOpenId2ProviderProfile.h"
#define GUCEF_AUTH_IOPENID2PROVIDERPROFILE_H
#endif /* GUCEF_AUTH_IOPENID2PROVIDERPROFILE_H ? */

#ifndef GUCEF_AUTH_CAUTHPROVIDERDESCRIPTOR_H
#include "gucefAUTH_CAuthProviderDescriptor.h"
#define GUCEF_AUTH_CAUTHPROVIDERDESCRIPTOR_H
#endif /* GUCEF_AUTH_CAUTHPROVIDERDESCRIPTOR_H ? */

#ifndef GUCEF_AUTHPLUGIN_STEAM_CSTEAMLOCALDETECTOR_H
#include "authpluginSTEAM_CSteamLocalDetector.h"
#define GUCEF_AUTHPLUGIN_STEAM_CSTEAMLOCALDETECTOR_H
#endif /* GUCEF_AUTHPLUGIN_STEAM_CSTEAMLOCALDETECTOR_H ? */

#ifndef GUCEF_AUTHPLUGIN_STEAM_MACROS_H
#include "authpluginSTEAM_macros.h"
#define GUCEF_AUTHPLUGIN_STEAM_MACROS_H
#endif /* GUCEF_AUTHPLUGIN_STEAM_MACROS_H ? */

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
 *  What the module knows of Steam as an OpenID 2.0 provider: the descriptor of the provider "steam" (the endpoint, the
 *  display name, the account namespace "steam") and the profile with the parts of the login that are Steam's own.
 *  The plugin registers both when it loads (CAuthGlobal::GetDescriptorRegistry() and GetOpenId2Profiles()).
 *
 *  Identity. Steam answers with the claimed id https://steamcommunity.com/openid/id/<SteamID64>, which is also the
 *  identity; the subject of the account reference "steam:<SteamID64>" is the number: digits only, no leading zero and
 *  within 64 bits, on the origin of the endpoint. Display name and avatar need the Steam Web API with a publisher key, so
 *  only a server has them and the assertion has no name.
 *
 *  Realm. Steam shows the player the host of the realm as the site that asks for the sign in; the profile allows a relay
 *  (the parameter relayUrl, see COpenId2Relay), which puts a page of the developer's own domain there instead of the
 *  loopback address of the application.
 *
 *  Local hint. A Steam client on the machine is looked for with CSteamLocalDetector (the files of the client, its process):
 *  the name of the user that used it last, or that is logged in, and whether it runs. The result is kept for a few seconds.
 */
class GUCEF_AUTHPLUGIN_STEAM_EXPORT_CPP CSteamOpenId2Profile : public AUTH::IOpenId2ProviderProfile
{
    public:

    static const CORE::CString ProviderName;            /**< "steam": the name of the descriptor and of the provider */
    static const CORE::CString AccountNamespace;        /**< "steam": the provider part of the account references, "steam:<SteamID64>" */
    static const CORE::CString Endpoint;                /**< https://steamcommunity.com/openid/login */

    CSteamOpenId2Profile( void );

    virtual ~CSteamOpenId2Profile();

    /** The descriptor of the Steam provider: OpenID 2.0 in the browser, no client id and no secret */
    static AUTH::CAuthProviderDescriptor MakeDescriptor( void );

    /** The SteamID64 in <origin of the endpoint>/openid/id/<SteamID64>; false for anything else */
    static bool ExtractSteamId( const CORE::CString& endpoint, const CORE::CString& claimedId, CORE::CString& steamId );

    /** The view of the machine the detection reads, instead of the real one; not owned, GUCEF_NULL for the real machine. A test gives it a made up Steam installation */
    void SetHostView( const ISteamHostView* view );

    virtual const CORE::CString& GetProviderName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool ExtractSubject( const CORE::CString& endpoint ,
                                 const CORE::CString& claimedId ,
                                 const CORE::CString& identity  ,
                                 CORE::CString& subject         ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool GetDetectedAccountHint( CORE::CString& displayName, bool& isRunning ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual void ForgetDetectedAccountHint( void ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    /** How long a detection of the Steam client is kept: long enough for the screens that ask in a row, short enough to see the client start */
    static const UInt64 HintLifeTimeInMs = 3000;

    mutable MT::CMutex m_lock;
    CSteamLocalDetector m_detector;
    mutable CSteamLocalInfo m_info;
    mutable UInt64 m_infoTimeInMs;
    mutable bool m_infoValid;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace STEAM */
}; /* namespace AUTHPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUTHPLUGIN_STEAM_CSTEAMOPENID2PROFILE_H ? */
