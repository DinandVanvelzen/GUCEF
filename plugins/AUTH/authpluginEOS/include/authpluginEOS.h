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

#ifndef GUCEF_AUTHPLUGIN_EOS_H
#define GUCEF_AUTHPLUGIN_EOS_H

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

#ifndef GUCEF_AUTHPLUGIN_EOS_MACROS_H
#include "authpluginEOS_macros.h"
#define GUCEF_AUTHPLUGIN_EOS_MACROS_H
#endif /* GUCEF_AUTHPLUGIN_EOS_MACROS_H ? */

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
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Registers the factory of the "epic" provider in the factory. What GUCEFPlugin_Load() does with the factory of
 *  CAuthGlobal; separate so that a test can use a factory of its own.
 *
 *  The name "epic" is also the name of the built in OAuth/OIDC provider (a descriptor). The provider of this plugin
 *  only takes its place when replaceExisting is true, which GUCEFPlugin_Load() passes when the EOS SDK is part of the
 *  build: a build without the SDK has a provider that is never available, and it must not take away the Epic login in
 *  the browser. With replaceExisting false and the name taken the factory is left as it was and false is returned.
 *
 *  @return true when the factory of the plugin is registered
 */
GUCEF_AUTHPLUGIN_EOS_EXPORT_CPP bool
RegisterEosAuthProviders( AUTH::CAuthProviderFactory& factory, bool replaceExisting );

/**
 *  Takes the "epic" provider of this plugin out of the factory again. With restoreBuiltIn, and when the factory is the
 *  one of CAuthGlobal, the built in descriptor provider comes back under the name.
 */
GUCEF_AUTHPLUGIN_EOS_EXPORT_CPP void
UnregisterEosAuthProviders( AUTH::CAuthProviderFactory& factory, bool restoreBuiltIn );

/*-------------------------------------------------------------------------*/

/*
 *      Prevent C++ name mangling
 */
#ifdef __cplusplus
extern "C" {
#endif

/*-------------------------------------------------------------------------*/

/** Registers the "epic" provider with gucefAUTH. The plugin loads whether or not the SDK is in the build: without it the provider is not available and does not replace the built in Epic login in the browser */
GUCEF_AUTHPLUGIN_EOS_EXPORT_C CORE::Int32 GUCEF_PLUGIN_CALLSPEC_PREFIX
GUCEFPlugin_Load( CORE::UInt32 argc, const char** argv ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

GUCEF_AUTHPLUGIN_EOS_EXPORT_C void GUCEF_PLUGIN_CALLSPEC_PREFIX
GUCEFPlugin_Unload( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

GUCEF_AUTHPLUGIN_EOS_EXPORT_C void GUCEF_PLUGIN_CALLSPEC_PREFIX
GUCEFPlugin_GetVersion( CORE::TVersion* versionInfo ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

GUCEF_AUTHPLUGIN_EOS_EXPORT_C const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
GUCEFPlugin_GetCopyright( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

GUCEF_AUTHPLUGIN_EOS_EXPORT_C const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
GUCEFPlugin_GetDescription( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

GUCEF_AUTHPLUGIN_EOS_EXPORT_C const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
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

}; /* namespace EOS */
}; /* namespace AUTHPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUTHPLUGIN_EOS_H ? */
