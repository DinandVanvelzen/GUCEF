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

#ifndef GUCEF_AUTHPLUGIN_ANDROID_CANDROIDAUTHUTILS_H
#define GUCEF_AUTHPLUGIN_ANDROID_CANDROIDAUTHUTILS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_AUTH_CAUTHRESULT_H
#include "gucefAUTH_CAuthResult.h"
#define GUCEF_AUTH_CAUTHRESULT_H
#endif /* GUCEF_AUTH_CAUTHRESULT_H ? */

#ifndef GUCEF_AUTH_CAUTHSETTINGS_H
#include "gucefAUTH_CAuthSettings.h"
#define GUCEF_AUTH_CAUTHSETTINGS_H
#endif /* GUCEF_AUTH_CAUTHSETTINGS_H ? */

#ifndef GUCEF_AUTHPLUGIN_ANDROID_MACROS_H
#include "authpluginANDROID_macros.h"
#define GUCEF_AUTHPLUGIN_ANDROID_MACROS_H
#endif /* GUCEF_AUTHPLUGIN_ANDROID_MACROS_H ? */

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

/** The helpers that the Android providers need to read tokens and to talk to the token services of the servers they verify for */
class GUCEF_AUTHPLUGIN_ANDROID_EXPORT_CPP CAndroidAuthUtils
{
    public:

    /** The longest answer of a token service that is read, bytes */
    static const UInt32 MaxResponseSize = 65536;

    /** The longest id token that is looked at, bytes */
    static const UInt32 MaxJwtSize = 16384;

    /** Whether the url may be used for a request that carries a secret: https, or http when the settings allow it (tests) */
    static bool IsAcceptableUrl( const CORE::CString& url, bool allowInsecureHttp );

    /**
     *  One request on the calling thread. No redirects are followed (a secret in the request is not sent anywhere the
     *  settings did not name), answers over MaxResponseSize are refused. Nothing of the request or the answer is logged.
     *  @return true when an answer came, whatever its status
     */
    static bool Execute( const CORE::CString& method      ,
                         const CORE::CString& url         ,
                         const CORE::CString& contentType ,
                         const CORE::CString& body        ,
                         UInt32 timeoutInMs               ,
                         Int32& statusCode                ,
                         CORE::CString& responseBody      );

    /** Parses json text with the json codec of the process; the document is the root node. False for anything else than json */
    static bool ParseJson( const CORE::CString& text, CORE::CDataNode& root );

    /** The claims of a compact JWT, read WITHOUT verifying anything: only for what a client does with its own token. False when it is not a JWT */
    static bool DecodeJwtClaims( const CORE::CString& jwt, CORE::CDataNode& claims );

    /** The string member (attribute) of the node, empty when there is none */
    static CORE::CString GetString( const CORE::CDataNode& node, const char* name );

    /** A random value for a nonce: 16 random bytes as base64url. Empty when the system has no random source */
    static CORE::CString MakeNonce( void );

    /**
     *  Hands the proof to the OIDC engine of gucefAUTH with the built in descriptor of the given name. False when there is no
     *  engine, no such descriptor, the engine does not support it with these settings, or the proof does not verify.
     */
    static bool VerifyWithEngine( const CORE::CString& descriptorName         ,
                                  const AUTH::CAuthProviderSettings& settings ,
                                  const AUTH::CAuthProof& proof               ,
                                  AUTH::CAuthResult& result                   );

    private:

    CAndroidAuthUtils( void );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ANDROIDAUTH */
}; /* namespace AUTHPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUTHPLUGIN_ANDROID_CANDROIDAUTHUTILS_H ? */
