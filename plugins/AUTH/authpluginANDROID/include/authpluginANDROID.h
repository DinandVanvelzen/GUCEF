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

#ifndef GUCEF_AUTHPLUGIN_ANDROID_H
#define GUCEF_AUTHPLUGIN_ANDROID_H

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
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The providers of the plugin, as flags */
enum EAndroidProviderFlags
{
    ANDROIDPROVIDER_NONE = 0,
    ANDROIDPROVIDER_GOOGLE = 1,
    ANDROIDPROVIDER_AMAZON = 2
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Registers the factories of the "google" and "amazon" providers in the factory. What GUCEFPlugin_Load() does with the
 *  factory of CAuthGlobal; separate so that a test can use a factory of its own.
 *
 *  The names "google" and "amazon" are also the names of the built in OAuth/OIDC providers (descriptors). A provider of
 *  this plugin only takes the place of the built in one when its flag is in replaceExisting, which GUCEFPlugin_Load()
 *  sets for a provider whose platform is there (Play services, Login with Amazon): where it is not, the browser
 *  login of the built in provider is all there is and stays. With the flag not set and the name taken the factory is
 *  left as it was for that name.
 *
 *  @return the flags of the providers that are registered
 */
GUCEF_AUTHPLUGIN_ANDROID_EXPORT_CPP UInt32
RegisterAndroidAuthProviders( AUTH::CAuthProviderFactory& factory, UInt32 replaceExisting );

/**
 *  Takes the providers in registered out of the factory again. With the flags in restoreBuiltIn, and when the factory is
 *  the one of CAuthGlobal, the built in descriptor provider of that name comes back.
 */
GUCEF_AUTHPLUGIN_ANDROID_EXPORT_CPP void
UnregisterAndroidAuthProviders( AUTH::CAuthProviderFactory& factory, UInt32 registered, UInt32 restoreBuiltIn );

/*-------------------------------------------------------------------------*/

/*
 *      Prevent C++ name mangling
 */
#ifdef __cplusplus
extern "C" {
#endif

/*-------------------------------------------------------------------------*/

/**
 *  Registers the "google" and "amazon" providers with gucefAUTH, installs the Custom Tabs browser launcher and the
 *  receiver that hands redirects to the OAuth/OIDC engine. Load the plugin after the Java side (GucefAuthBridge) has
 *  initialized. The plugin loads whether or not the Java side is there: the providers are then not available and do not
 *  take the place of the built in ones.
 */
GUCEF_AUTHPLUGIN_ANDROID_EXPORT_C CORE::Int32 GUCEF_PLUGIN_CALLSPEC_PREFIX
GUCEFPlugin_Load( CORE::UInt32 argc, const char** argv ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

GUCEF_AUTHPLUGIN_ANDROID_EXPORT_C void GUCEF_PLUGIN_CALLSPEC_PREFIX
GUCEFPlugin_Unload( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

GUCEF_AUTHPLUGIN_ANDROID_EXPORT_C void GUCEF_PLUGIN_CALLSPEC_PREFIX
GUCEFPlugin_GetVersion( CORE::TVersion* versionInfo ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

GUCEF_AUTHPLUGIN_ANDROID_EXPORT_C const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
GUCEFPlugin_GetCopyright( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

GUCEF_AUTHPLUGIN_ANDROID_EXPORT_C const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
GUCEFPlugin_GetDescription( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

GUCEF_AUTHPLUGIN_ANDROID_EXPORT_C const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
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

}; /* namespace ANDROIDAUTH */
}; /* namespace AUTHPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUTHPLUGIN_ANDROID_H ? */
