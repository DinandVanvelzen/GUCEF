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

#ifndef GUCEF_AUTHPLUGIN_EOS_CEOSAUTHPROVIDER_H
#include "authpluginEOS_CEosAuthProvider.h"
#define GUCEF_AUTHPLUGIN_EOS_CEOSAUTHPROVIDER_H
#endif /* GUCEF_AUTHPLUGIN_EOS_CEOSAUTHPROVIDER_H ? */

#include "authpluginEOS.h"

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
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

static TEosAuthProviderFactory* g_eosAuthProviderFactory = GUCEF_NULL;

// Whether the plugin put its provider in the place of the built in one, so that unloading gives the place back
static bool g_replacedBuiltIn = false;
static bool g_registeredInGlobal = false;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

bool
RegisterEosAuthProviders( AUTH::CAuthProviderFactory& factory, bool replaceExisting )
{GUCEF_TRACE;

    if ( factory.IsConstructible( CEosAuthProvider::ProviderName ) )
    {
        if ( !replaceExisting )
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Auth plugin EOS: a provider named \"epic\" exists, it stays: the SDK is not part of this build" );
            return false;
        }
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Auth plugin EOS: the provider named \"epic\" is replaced by the one on the EOS SDK" );
    }

    if ( GUCEF_NULL == g_eosAuthProviderFactory )
        g_eosAuthProviderFactory = GUCEF_NEW TEosAuthProviderFactory();

    factory.RegisterConcreteFactory( CEosAuthProvider::ProviderName, g_eosAuthProviderFactory );
    return true;
}

/*-------------------------------------------------------------------------*/

void
UnregisterEosAuthProviders( AUTH::CAuthProviderFactory& factory, bool restoreBuiltIn )
{GUCEF_TRACE;

    factory.UnregisterConcreteFactory( CORE::CString( "epic" ) );

    AUTH::CAuthGlobal* global = AUTH::CAuthGlobal::Instance();
    if ( restoreBuiltIn && &factory == &global->GetProviderFactory() )
        global->RegisterDescriptorProvider( CORE::CString( "epic" ) );
}

/*-------------------------------------------------------------------------*/

CORE::Int32 GUCEF_PLUGIN_CALLSPEC_PREFIX
GUCEFPlugin_Load( CORE::UInt32 argc, const char** argv ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Load called on auth plugin EOS" );

    AUTH::CAuthGlobal* global = AUTH::CAuthGlobal::Instance();
    bool replace = CEosSdk::IsCompiledIn();
    bool nameTaken = global->GetProviderFactory().IsConstructible( CEosAuthProvider::ProviderName );

    // The manager holds a provider of the built in kind already, if it was refreshed: it makes way
    if ( !RegisterEosAuthProviders( global->GetProviderFactory(), replace ) )
        return 1;   // loaded, the name stays with the built in provider

    g_registeredInGlobal = true;
    g_replacedBuiltIn = nameTaken;
    if ( nameTaken )
        global->GetManager().RemoveProvider( CEosAuthProvider::ProviderName );
    global->GetManager().RefreshProviders();
    return 1;
}

/*-------------------------------------------------------------------------*/

void GUCEF_PLUGIN_CALLSPEC_PREFIX
GUCEFPlugin_Unload( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Unload called on auth plugin EOS" );

    // The names are looked up as new strings: the plugin is unloaded when gucefAUTH goes away, which can be at the end of the process, after the static strings of this library are gone
    if ( g_registeredInGlobal )
    {
        AUTH::CAuthGlobal* global = AUTH::CAuthGlobal::Instance();

        // The provider is code of this plugin: it goes before the plugin does
        global->GetManager().RemoveProvider( CORE::CString( "epic" ) );
        UnregisterEosAuthProviders( global->GetProviderFactory(), g_replacedBuiltIn );
        if ( g_replacedBuiltIn )
            global->GetManager().RefreshProviders();
    }
    g_registeredInGlobal = false;
    g_replacedBuiltIn = false;

    GUCEF_DELETE g_eosAuthProviderFactory;
    g_eosAuthProviderFactory = GUCEF_NULL;
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

    return "gucefAUTH plugin for logging in with Epic Online Services: persistent auth, account portal and device code logins, verification of id tokens through the OIDC engine";
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

}; /* namespace EOS */
}; /* namespace AUTHPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
