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

#ifndef GUCEF_AUTHPLUGIN_STEAM_CSTEAMAUTHPROVIDER_H
#include "authpluginSTEAM_CSteamAuthProvider.h"
#define GUCEF_AUTHPLUGIN_STEAM_CSTEAMAUTHPROVIDER_H
#endif /* GUCEF_AUTHPLUGIN_STEAM_CSTEAMAUTHPROVIDER_H ? */

#include "authpluginSTEAM.h"

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
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

static TSteamAuthProviderFactory* g_steamAuthProviderFactory = GUCEF_NULL;
static CSteamOpenId2Profile* g_steamOpenId2Profile = GUCEF_NULL;
static bool g_registeredInGlobal = false;     /**< this module registered its backend: only then there is something to take away */
static bool g_registeredDescriptor = false;   /**< the descriptor is ours: an application that described the provider itself keeps its own */

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The name of the provider as a new string. The plugin is unloaded when the module gucefAUTH goes away, which can be at the end of the process,
 *  after the static strings of this library were destroyed: what it takes away at that time is looked up with a name that does not depend on them.
 */
static CORE::CString
MakeProviderName( void )
{GUCEF_TRACE;

    return CORE::CString( "steam" );
}

/*-------------------------------------------------------------------------*/

bool
RegisterSteamAuthProviders( AUTH::CAuthProviderFactory& factory )
{GUCEF_TRACE;

    if ( factory.IsConstructible( CSteamAuthProvider::ProviderName ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Auth plugin STEAM: a provider named \"steam\" is registered already" );
        return false;
    }
    if ( GUCEF_NULL == g_steamAuthProviderFactory )
        g_steamAuthProviderFactory = GUCEF_NEW TSteamAuthProviderFactory();

    factory.RegisterConcreteFactory( CSteamAuthProvider::ProviderName, g_steamAuthProviderFactory );
    return true;
}

/*-------------------------------------------------------------------------*/

void
UnregisterSteamAuthProviders( AUTH::CAuthProviderFactory& factory )
{GUCEF_TRACE;

    factory.UnregisterConcreteFactory( MakeProviderName() );
}

/*-------------------------------------------------------------------------*/

CSteamOpenId2Profile*
GetSteamOpenId2Profile( void )
{GUCEF_TRACE;

    return g_steamOpenId2Profile;
}

/*-------------------------------------------------------------------------*/

CORE::Int32 GUCEF_PLUGIN_CALLSPEC_PREFIX
GUCEFPlugin_Load( CORE::UInt32 argc, const char** argv ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Load called on auth plugin STEAM" );

    // The module is in the process already and has registered its backend: one registration serves
    if ( g_registeredInGlobal )
        return 1;

    AUTH::CAuthGlobal* global = AUTH::CAuthGlobal::Instance();

    // What the module needs to know of Steam: the description of the provider and the profile of its OpenID 2.0 login
    g_registeredDescriptor = false;
    if ( !global->GetDescriptorRegistry().HasDescriptor( CSteamAuthProvider::ProviderName ) )
        g_registeredDescriptor = global->GetDescriptorRegistry().RegisterDescriptor( CSteamOpenId2Profile::MakeDescriptor() );

    g_steamOpenId2Profile = GUCEF_NEW CSteamOpenId2Profile();
    if ( !global->GetOpenId2Profiles().Register( g_steamOpenId2Profile ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Auth plugin STEAM: a profile for the provider \"steam\" is registered already" );
        GUCEF_DELETE g_steamOpenId2Profile;
        g_steamOpenId2Profile = GUCEF_NULL;
        if ( g_registeredDescriptor )
            global->GetDescriptorRegistry().UnregisterDescriptor( CSteamAuthProvider::ProviderName );
        g_registeredDescriptor = false;
        return 0;
    }

    if ( !RegisterSteamAuthProviders( global->GetProviderFactory() ) )
    {
        global->GetOpenId2Profiles().Unregister( CSteamOpenId2Profile::ProviderName );
        GUCEF_DELETE g_steamOpenId2Profile;
        g_steamOpenId2Profile = GUCEF_NULL;
        if ( g_registeredDescriptor )
            global->GetDescriptorRegistry().UnregisterDescriptor( CSteamAuthProvider::ProviderName );
        g_registeredDescriptor = false;
        return 0;
    }
    g_registeredInGlobal = true;

    // The manager makes the provider now, so that a configuration that was loaded before the plugin applies to it
    global->GetManager().RefreshProviders();
    return 1;
}

/*-------------------------------------------------------------------------*/

void GUCEF_PLUGIN_CALLSPEC_PREFIX
GUCEFPlugin_Unload( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Unload called on auth plugin STEAM" );

    // Also called for a Load that failed, because a provider of that name was there already: what is not ours stays
    if ( !g_registeredInGlobal )
        return;
    g_registeredInGlobal = false;

    AUTH::CAuthGlobal* global = AUTH::CAuthGlobal::Instance();

    // The provider is code of this plugin: it goes before the plugin does, and what it needs goes after it
    global->GetManager().RemoveProvider( MakeProviderName() );
    UnregisterSteamAuthProviders( global->GetProviderFactory() );
    global->GetOpenId2Profiles().Unregister( MakeProviderName() );
    if ( g_registeredDescriptor )
        global->GetDescriptorRegistry().UnregisterDescriptor( MakeProviderName() );
    g_registeredDescriptor = false;

    GUCEF_DELETE g_steamOpenId2Profile;
    g_steamOpenId2Profile = GUCEF_NULL;
    GUCEF_DELETE g_steamAuthProviderFactory;
    g_steamAuthProviderFactory = GUCEF_NULL;
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

    return "gucefAUTH plugin for logging in with Steam: the OpenID 2.0 login in the browser, the login with the Steam client (a Steam ticket), verification of both";
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

}; /* namespace STEAM */
}; /* namespace AUTHPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
