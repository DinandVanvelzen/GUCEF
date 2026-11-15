/*
 *  computebridgeMATH: GUCEF compute bridge plugin of gucefMATH: multi-core and accelerator providers for the batch kernels
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

#ifndef GUCEF_COMPUTEBRIDGEMATH_H
#define GUCEF_COMPUTEBRIDGEMATH_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_ESTRUCTS_H
#include "EStructs.h"
#define GUCEF_CORE_ESTRUCTS_H
#endif /* GUCEF_CORE_ESTRUCTS_H ? */

#ifndef GUCEF_COMPUTEBRIDGEMATH_MACROS_H
#include "computebridgeMATH_macros.h"
#define GUCEF_COMPUTEBRIDGEMATH_MACROS_H
#endif /* GUCEF_COMPUTEBRIDGEMATH_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COMPUTEBRIDGEMATH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *  On the web there is no loading of modules: the plugin is linked into the application and its entry points get
 *  unique names (computebridgeMATH_GUCEFPlugin_Load ...), which the application calls itself
 */
#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN )
  #define COMPUTEBRIDGEMATH_ENTRY( func ) computebridgeMATH_##func
#else
  #define COMPUTEBRIDGEMATH_ENTRY( func ) func
#endif

/*
 *      Prevent C++ name mangling
 */
#ifdef __cplusplus
extern "C" {
#endif

/*-------------------------------------------------------------------------*/

/**
 *  Registers the compute kernels of the gucefMATH batch kernels with gucefCOMPUTE and the MULTICORE and ACCELERATOR
 *  providers with the batch dispatch table of gucefMATH (see CMathBridgeGlobal). Returns 0 when gucefCOMPUTE or
 *  gucefMATH is not available or a kernel could not be registered.
 */
COMPUTEBRIDGEMATH_PUBLIC_C CORE::Int32 GUCEF_PLUGIN_CALLSPEC_PREFIX
COMPUTEBRIDGEMATH_ENTRY( GUCEFPlugin_Load )( CORE::UInt32 argc, const char** argv ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

/** Removes the providers and the kernels again; calls that are in flight finish first */
COMPUTEBRIDGEMATH_PUBLIC_C void GUCEF_PLUGIN_CALLSPEC_PREFIX
COMPUTEBRIDGEMATH_ENTRY( GUCEFPlugin_Unload )( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

COMPUTEBRIDGEMATH_PUBLIC_C void GUCEF_PLUGIN_CALLSPEC_PREFIX
COMPUTEBRIDGEMATH_ENTRY( GUCEFPlugin_GetVersion )( CORE::TVersion* versionInfo ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

COMPUTEBRIDGEMATH_PUBLIC_C const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
COMPUTEBRIDGEMATH_ENTRY( GUCEFPlugin_GetCopyright )( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

COMPUTEBRIDGEMATH_PUBLIC_C const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
COMPUTEBRIDGEMATH_ENTRY( GUCEFPlugin_GetDescription )( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

COMPUTEBRIDGEMATH_PUBLIC_C const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
COMPUTEBRIDGEMATH_ENTRY( GUCEFPlugin_GetLinkBackModuleDependencies )( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

/*-------------------------------------------------------------------------*/

#ifdef __cplusplus
   }
#endif /* __cplusplus */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMPUTEBRIDGEMATH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMPUTEBRIDGEMATH_H ? */
