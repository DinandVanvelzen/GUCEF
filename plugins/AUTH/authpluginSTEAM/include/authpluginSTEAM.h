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

#ifndef GUCEF_AUTHPLUGIN_STEAM_H
#define GUCEF_AUTHPLUGIN_STEAM_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_ESTRUCTS_H
#include "EStructs.h"
#define GUCEF_CORE_ESTRUCTS_H
#endif /* GUCEF_CORE_ESTRUCTS_H ? */

#ifndef GUCEF_AUTH_CIAUTHPROVIDER_H
#include "gucefAUTH_CIAuthProvider.h"
#define GUCEF_AUTH_CIAUTHPROVIDER_H
#endif /* GUCEF_AUTH_CIAUTHPROVIDER_H ? */

#ifndef GUCEF_AUTHPLUGIN_STEAM_MACROS_H
#include "authpluginSTEAM_macros.h"
#define GUCEF_AUTHPLUGIN_STEAM_MACROS_H
#endif /* GUCEF_AUTHPLUGIN_STEAM_MACROS_H ? */

#ifndef GUCEF_AUTHPLUGIN_STEAM_CSTEAMOPENID2PROFILE_H
#include "authpluginSTEAM_CSteamOpenId2Profile.h"
#define GUCEF_AUTHPLUGIN_STEAM_CSTEAMOPENID2PROFILE_H
#endif /* GUCEF_AUTHPLUGIN_STEAM_CSTEAMOPENID2PROFILE_H ? */

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
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Registers the factory of the "steam" provider in the factory. What GUCEFPlugin_Load() does with the factory of
 *  CAuthGlobal; separate so that a test can use a factory of its own. False when the factory has a provider of that
 *  name already.
 */
GUCEF_AUTHPLUGIN_STEAM_EXPORT_CPP bool
RegisterSteamAuthProviders( AUTH::CAuthProviderFactory& factory );

/** Takes the "steam" provider out of the factory again */
GUCEF_AUTHPLUGIN_STEAM_EXPORT_CPP void
UnregisterSteamAuthProviders( AUTH::CAuthProviderFactory& factory );

/**
 *  The OpenID 2.0 profile of Steam that the plugin registered with CAuthGlobal::GetOpenId2Profiles() when it loaded;
 *  GUCEF_NULL while the plugin is not loaded. A test gives it a made up Steam installation to look at (SetHostView())
 */
GUCEF_AUTHPLUGIN_STEAM_EXPORT_CPP CSteamOpenId2Profile*
GetSteamOpenId2Profile( void );

/*-------------------------------------------------------------------------*/

/*
 *      Prevent C++ name mangling
 */
#ifdef __cplusplus
extern "C" {
#endif

/*-------------------------------------------------------------------------*/

/**
 *  Registers the Steam backend with gucefAUTH: the descriptor of the provider "steam" (unless the application has described it
 *  already in its configuration), the OpenID 2.0 profile of Steam and the provider. GUCEFPlugin_Unload() takes all of it away again.
 *  The plugin loads whether or not Steam is there: the login in the browser is there, and the login with the Steam client says
 *  it is not available.
 */
GUCEF_AUTHPLUGIN_STEAM_EXPORT_C CORE::Int32 GUCEF_PLUGIN_CALLSPEC_PREFIX
GUCEFPlugin_Load( CORE::UInt32 argc, const char** argv ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

GUCEF_AUTHPLUGIN_STEAM_EXPORT_C void GUCEF_PLUGIN_CALLSPEC_PREFIX
GUCEFPlugin_Unload( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

GUCEF_AUTHPLUGIN_STEAM_EXPORT_C void GUCEF_PLUGIN_CALLSPEC_PREFIX
GUCEFPlugin_GetVersion( CORE::TVersion* versionInfo ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

GUCEF_AUTHPLUGIN_STEAM_EXPORT_C const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
GUCEFPlugin_GetCopyright( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

GUCEF_AUTHPLUGIN_STEAM_EXPORT_C const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
GUCEFPlugin_GetDescription( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

GUCEF_AUTHPLUGIN_STEAM_EXPORT_C const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
GUCEFPlugin_GetLinkBackModuleDependencies( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

/*-------------------------------------------------------------------------*/

#ifdef __cplusplus
   }
#endif /* __cplusplus */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace STEAM */
}; /* namespace AUTHPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUTHPLUGIN_STEAM_H ? */
