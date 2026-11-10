/*
 *  audiodriverWEBAUDIO: gucefAUDIO driver plugin for the Web Audio API in browsers (Emscripten)
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

#ifndef GUCEF_AUDIODRIVER_WEBAUDIO_H
#define GUCEF_AUDIODRIVER_WEBAUDIO_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_ESTRUCTS_H
#include "EStructs.h"
#define GUCEF_CORE_ESTRUCTS_H
#endif /* GUCEF_CORE_ESTRUCTS_H ? */

#ifndef GUCEF_AUDIODRIVER_WEBAUDIO_MACROS_H
#include "audiodriverWEBAUDIO_macros.h"
#define GUCEF_AUDIODRIVER_WEBAUDIO_MACROS_H
#endif /* GUCEF_AUDIODRIVER_WEBAUDIO_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *  The plugin is always linked into the application (there is no loading of modules on the web), so its entry points
 *  have unique names, audiodriverWEBAUDIO_GUCEFPlugin_Load and so on, which the application calls itself.
 */
#define GUCEF_AUDIODRIVER_WEBAUDIO_API( func ) audiodriverWEBAUDIO_##func

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUDIODRIVER {
namespace WEBAUDIO {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *      Prevent C++ name mangling
 */
#ifdef __cplusplus
extern "C" {
#endif

/*-------------------------------------------------------------------------*/

/** Registers the "webaudio" driver with gucefAUDIO. Fails when the browser has no Web Audio API */
GUCEF_AUDIODRIVER_WEBAUDIO_EXPORT_C CORE::Int32 GUCEF_PLUGIN_CALLSPEC_PREFIX
GUCEF_AUDIODRIVER_WEBAUDIO_API( GUCEFPlugin_Load )( CORE::UInt32 argc, const char** argv ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

GUCEF_AUDIODRIVER_WEBAUDIO_EXPORT_C void GUCEF_PLUGIN_CALLSPEC_PREFIX
GUCEF_AUDIODRIVER_WEBAUDIO_API( GUCEFPlugin_Unload )( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

GUCEF_AUDIODRIVER_WEBAUDIO_EXPORT_C void GUCEF_PLUGIN_CALLSPEC_PREFIX
GUCEF_AUDIODRIVER_WEBAUDIO_API( GUCEFPlugin_GetVersion )( CORE::TVersion* versionInfo ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

GUCEF_AUDIODRIVER_WEBAUDIO_EXPORT_C const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
GUCEF_AUDIODRIVER_WEBAUDIO_API( GUCEFPlugin_GetCopyright )( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

GUCEF_AUDIODRIVER_WEBAUDIO_EXPORT_C const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
GUCEF_AUDIODRIVER_WEBAUDIO_API( GUCEFPlugin_GetDescription )( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

GUCEF_AUDIODRIVER_WEBAUDIO_EXPORT_C const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
GUCEF_AUDIODRIVER_WEBAUDIO_API( GUCEFPlugin_GetLinkBackModuleDependencies )( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

/*-------------------------------------------------------------------------*/

#ifdef __cplusplus
   }
#endif /* __cplusplus */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEBAUDIO */
}; /* namespace AUDIODRIVER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIODRIVER_WEBAUDIO_H ? */
