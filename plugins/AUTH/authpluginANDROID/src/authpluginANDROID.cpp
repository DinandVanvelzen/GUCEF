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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_AUTH_CAUTHGLOBAL_H
#include "gucefAUTH_CAuthGlobal.h"
#define GUCEF_AUTH_CAUTHGLOBAL_H
#endif /* GUCEF_AUTH_CAUTHGLOBAL_H ? */

#ifndef GUCEF_AUTH_CAUTHMANAGER_H
#include "gucefAUTH_CAuthManager.h"
#define GUCEF_AUTH_CAUTHMANAGER_H
#endif /* GUCEF_AUTH_CAUTHMANAGER_H ? */

#ifndef GUCEF_AUTHPLUGIN_ANDROID_CGOOGLEAUTHPROVIDER_H
#include "authpluginANDROID_CGoogleAuthProvider.h"
#define GUCEF_AUTHPLUGIN_ANDROID_CGOOGLEAUTHPROVIDER_H
#endif /* GUCEF_AUTHPLUGIN_ANDROID_CGOOGLEAUTHPROVIDER_H ? */

#ifndef GUCEF_AUTHPLUGIN_ANDROID_CAMAZONAUTHPROVIDER_H
#include "authpluginANDROID_CAmazonAuthProvider.h"
#define GUCEF_AUTHPLUGIN_ANDROID_CAMAZONAUTHPROVIDER_H
#endif /* GUCEF_AUTHPLUGIN_ANDROID_CAMAZONAUTHPROVIDER_H ? */

#ifndef GUCEF_AUTHPLUGIN_ANDROID_JNIGLUE_H
#include "authpluginANDROID_JniGlue.h"
#define GUCEF_AUTHPLUGIN_ANDROID_JNIGLUE_H
#endif /* GUCEF_AUTHPLUGIN_ANDROID_JNIGLUE_H ? */

#include "authpluginANDROID.h"

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
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

static TGoogleAuthProviderFactory* g_googleAuthProviderFactory = GUCEF_NULL;
static TAmazonAuthProviderFactory* g_amazonAuthProviderFactory = GUCEF_NULL;

// What the plugin did to the process when it loaded, so that unloading undoes exactly that
static UInt32 g_registeredFlags = 0;
static UInt32 g_replacedFlags = 0;
static CEngineRedirectReceiver* g_redirectReceiver = GUCEF_NULL;
static CAndroidRedirectReceiver* g_previousRedirectReceiver = GUCEF_NULL;
static bool g_launcherInstalled = false;
static bool g_loaded = false;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static bool
RegisterOne( AUTH::CAuthProviderFactory& factory, const CORE::CString& name, CORE::CTFactoryBase< AUTH::CIAuthProvider, MT::CMutex >* concreteFactory, bool replace )
{GUCEF_TRACE;

    if ( factory.IsConstructible( name ) )
    {
        if ( !replace )
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Auth plugin ANDROID: a provider named \"" << name << "\" exists, it stays: the platform of the plugin's provider is not there" );
            return false;
        }
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Auth plugin ANDROID: the provider named \"" << name << "\" is replaced by the native one" );
    }
    factory.RegisterConcreteFactory( name, concreteFactory );
    return true;
}

/*-------------------------------------------------------------------------*/

UInt32
RegisterAndroidAuthProviders( AUTH::CAuthProviderFactory& factory, UInt32 replaceExisting )
{GUCEF_TRACE;

    UInt32 registered = ANDROIDPROVIDER_NONE;

    if ( GUCEF_NULL == g_googleAuthProviderFactory )
        g_googleAuthProviderFactory = GUCEF_NEW TGoogleAuthProviderFactory();
    if ( GUCEF_NULL == g_amazonAuthProviderFactory )
        g_amazonAuthProviderFactory = GUCEF_NEW TAmazonAuthProviderFactory();

    if ( RegisterOne( factory, CGoogleAuthProvider::ProviderName, g_googleAuthProviderFactory, 0 != ( replaceExisting & ANDROIDPROVIDER_GOOGLE ) ) )
        registered |= ANDROIDPROVIDER_GOOGLE;
    if ( RegisterOne( factory, CAmazonAuthProvider::ProviderName, g_amazonAuthProviderFactory, 0 != ( replaceExisting & ANDROIDPROVIDER_AMAZON ) ) )
        registered |= ANDROIDPROVIDER_AMAZON;
    return registered;
}

/*-------------------------------------------------------------------------*/

void
UnregisterAndroidAuthProviders( AUTH::CAuthProviderFactory& factory, UInt32 registered, UInt32 restoreBuiltIn )
{GUCEF_TRACE;

    AUTH::CAuthGlobal* global = AUTH::CAuthGlobal::Instance();
    bool isGlobalFactory = &factory == &global->GetProviderFactory();

    if ( 0 != ( registered & ANDROIDPROVIDER_GOOGLE ) )
    {
        factory.UnregisterConcreteFactory( CORE::CString( "google" ) );
        if ( isGlobalFactory && 0 != ( restoreBuiltIn & ANDROIDPROVIDER_GOOGLE ) )
            global->RegisterDescriptorProvider( CORE::CString( "google" ) );
    }
    if ( 0 != ( registered & ANDROIDPROVIDER_AMAZON ) )
    {
        factory.UnregisterConcreteFactory( CORE::CString( "amazon" ) );
        if ( isGlobalFactory && 0 != ( restoreBuiltIn & ANDROIDPROVIDER_AMAZON ) )
            global->RegisterDescriptorProvider( CORE::CString( "amazon" ) );
    }
}

/*-------------------------------------------------------------------------*/

CORE::Int32 GUCEF_PLUGIN_CALLSPEC_PREFIX
GUCEFPlugin_Load( CORE::UInt32 argc, const char** argv ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Load called on auth plugin ANDROID" );

    if ( g_loaded )
        return 1;

    AUTH::CAuthGlobal* global = AUTH::CAuthGlobal::Instance();
    CAndroidAuthBridge* bridge = CAndroidAuthBridge::Instance();
    TAndroidShim shim = bridge->GetShim();

    // A native provider takes the place of the built in one only where its platform is there
    UInt32 replace = ANDROIDPROVIDER_NONE;
    if ( GUCEF_NULL != shim.googleSignIn && GUCEF_NULL != shim.isGooglePlayAvailable && shim.isGooglePlayAvailable( shim.context ) )
        replace |= ANDROIDPROVIDER_GOOGLE;
    if ( GUCEF_NULL != shim.amazonSignIn && GUCEF_NULL != shim.isAmazonAvailable && shim.isAmazonAvailable( shim.context ) )
        replace |= ANDROIDPROVIDER_AMAZON;

    UInt32 taken = ANDROIDPROVIDER_NONE;
    if ( global->GetProviderFactory().IsConstructible( CGoogleAuthProvider::ProviderName ) )
        taken |= ANDROIDPROVIDER_GOOGLE;
    if ( global->GetProviderFactory().IsConstructible( CAmazonAuthProvider::ProviderName ) )
        taken |= ANDROIDPROVIDER_AMAZON;

    g_registeredFlags = RegisterAndroidAuthProviders( global->GetProviderFactory(), replace );
    g_replacedFlags = g_registeredFlags & taken;

    // The manager holds providers of the built in kind already, if it was refreshed: they make way
    if ( 0 != ( g_replacedFlags & ANDROIDPROVIDER_GOOGLE ) )
        global->GetManager().RemoveProvider( CGoogleAuthProvider::ProviderName );
    if ( 0 != ( g_replacedFlags & ANDROIDPROVIDER_AMAZON ) )
        global->GetManager().RemoveProvider( CAmazonAuthProvider::ProviderName );
    global->GetManager().RefreshProviders();

    // Redirects of the browser flows go to the engine, and the browser of the engine is Custom Tabs
    g_redirectReceiver = GUCEF_NEW CEngineRedirectReceiver();
    g_previousRedirectReceiver = bridge->SetRedirectReceiver( g_redirectReceiver );
    g_launcherInstalled = GUCEF_NULL != shim.openCustomTab && bridge->InstallBrowserLauncher();

    g_loaded = true;
    return 1;
}

/*-------------------------------------------------------------------------*/

void GUCEF_PLUGIN_CALLSPEC_PREFIX
GUCEFPlugin_Unload( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Unload called on auth plugin ANDROID" );

    if ( !g_loaded )
        return;

    AUTH::CAuthGlobal* global = AUTH::CAuthGlobal::Instance();
    CAndroidAuthBridge* bridge = CAndroidAuthBridge::Instance();

    if ( g_launcherInstalled )
        bridge->RemoveBrowserLauncher();
    g_launcherInstalled = false;

    bridge->SetRedirectReceiver( g_previousRedirectReceiver );
    g_previousRedirectReceiver = GUCEF_NULL;
    GUCEF_DELETE g_redirectReceiver;
    g_redirectReceiver = GUCEF_NULL;

    // The providers are code of this plugin: they go before the plugin does. Their names are new strings: the plugin is unloaded when gucefAUTH goes away,
    // which can be at the end of the process, after the static strings of this library are gone
    if ( 0 != ( g_registeredFlags & ANDROIDPROVIDER_GOOGLE ) )
        global->GetManager().RemoveProvider( CORE::CString( "google" ) );
    if ( 0 != ( g_registeredFlags & ANDROIDPROVIDER_AMAZON ) )
        global->GetManager().RemoveProvider( CORE::CString( "amazon" ) );
    UnregisterAndroidAuthProviders( global->GetProviderFactory(), g_registeredFlags, g_replacedFlags );
    if ( 0 != g_replacedFlags )
        global->GetManager().RefreshProviders();
    g_registeredFlags = 0;
    g_replacedFlags = 0;

    GUCEF_DELETE g_googleAuthProviderFactory;
    g_googleAuthProviderFactory = GUCEF_NULL;
    GUCEF_DELETE g_amazonAuthProviderFactory;
    g_amazonAuthProviderFactory = GUCEF_NULL;

    // The Java side is not to call into code that is gone
    ReleaseJavaSide();
    g_loaded = false;
}

/*-------------------------------------------------------------------------*/

void GUCEF_PLUGIN_CALLSPEC_PREFIX
GUCEFPlugin_GetVersion( CORE::TVersion* versionInfo ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    if ( GUCEF_NULL != versionInfo )
    {
        versionInfo->major = 0;
        versionInfo->minor = 1;
        versionInfo->patch = 0;
        versionInfo->release = 0;
    }
}

/*-------------------------------------------------------------------------*/

const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
GUCEFPlugin_GetCopyright( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    return "Copyright (C) Dinand Vanvelzen, Apache License 2.0";
}

/*-------------------------------------------------------------------------*/

const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
GUCEFPlugin_GetDescription( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    return "gucefAUTH plugin for Android: Google sign in (Play Games Services, Credential Manager), Login with Amazon, Custom Tabs and the app link redirect for the browser flows";
}

/*-------------------------------------------------------------------------*/

const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
GUCEFPlugin_GetLinkBackModuleDependencies( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    return "gucefMT,gucefCORE,gucefCOMCORE,gucefWEB,gucefAUTH";
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ANDROIDAUTH */
}; /* namespace AUTHPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
