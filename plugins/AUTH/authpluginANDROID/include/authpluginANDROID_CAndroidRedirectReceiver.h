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

#ifndef GUCEF_AUTHPLUGIN_ANDROID_CANDROIDREDIRECTRECEIVER_H
#define GUCEF_AUTHPLUGIN_ANDROID_CANDROIDREDIRECTRECEIVER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

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

/**
 *  Takes the address that Android handed to the app at the end of a login in a browser: the app link or custom scheme
 *  redirect, for example "com.example.game:/oauth2redirect?code=...&state=...". The Java side
 *  (AuthRedirectActivity) passes the address to the JNI entry onAuthRedirect(), which hands it to the receiver that is
 *  registered with CAndroidAuthBridge::SetRedirectReceiver(), on the thread of the Android UI. The receiver must not
 *  block: it hands the address to the login that waits for it and returns. The address holds a code and a state, it is
 *  not logged.
 */
class GUCEF_AUTHPLUGIN_ANDROID_EXPORT_CPP CAndroidRedirectReceiver
{
    public:

    CAndroidRedirectReceiver( void );

    virtual ~CAndroidRedirectReceiver();

    /** @return true when a login took the address */
    virtual bool OnAuthRedirect( const CORE::CString& redirectUrl ) = 0;

    private:

    CAndroidRedirectReceiver( const CAndroidRedirectReceiver& src );
    CAndroidRedirectReceiver& operator=( const CAndroidRedirectReceiver& src );
};

/*-------------------------------------------------------------------------*/

/**
 *  The receiver that is wired to the OAuth/OIDC engine of gucefAUTH: the address goes to
 *  AUTH::IAuthFlowEngine::SubmitRedirectUrl() of the engine that CAuthGlobal::GetFlowEngine() gives. The plugin
 *  registers one when it loads. A login of the engine with redirectMode "custom-scheme" (or "manual") for which the
 *  Custom Tab was opened by the browser launcher of this plugin ends with the address.
 */
class GUCEF_AUTHPLUGIN_ANDROID_EXPORT_CPP CEngineRedirectReceiver : public CAndroidRedirectReceiver
{
    public:

    CEngineRedirectReceiver( void );

    virtual ~CEngineRedirectReceiver();

    virtual bool OnAuthRedirect( const CORE::CString& redirectUrl ) GUCEF_VIRTUAL_OVERRIDE;
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

#endif /* GUCEF_AUTHPLUGIN_ANDROID_CANDROIDREDIRECTRECEIVER_H ? */
