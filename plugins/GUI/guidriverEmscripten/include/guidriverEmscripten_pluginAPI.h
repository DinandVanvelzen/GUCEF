/*
 *  guidriverEmscripten: GUCEF window driver for web browsers: a canvas with WebGL 2
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

#ifndef GUCEF_GUIDRIVEREMSCRIPTEN_PLUGINAPI_H
#define GUCEF_GUIDRIVEREMSCRIPTEN_PLUGINAPI_H

#ifndef GUCEF_CORE_ESTRUCTS_H
#include "EStructs.h"
#define GUCEF_CORE_ESTRUCTS_H
#endif /* GUCEF_CORE_ESTRUCTS_H ? */

#ifndef GUCEF_GUIDRIVEREMSCRIPTEN_MACROS_H
#include "guidriverEmscripten_macros.h"
#define GUCEF_GUIDRIVEREMSCRIPTEN_MACROS_H
#endif /* GUCEF_GUIDRIVEREMSCRIPTEN_MACROS_H ? */

/*
 *  The plugin is always linked into the application (there is no loading of modules on the web), so its entry points
 *  have unique names, guidriverEmscripten_GUCEFPlugin_Load and so on, which the application calls itself.
 */
#define GUIDRIVEREMSCRIPTEN_ENTRY( func ) guidriverEmscripten_##func

#ifdef __cplusplus
namespace GUCEF {
namespace GUIDRIVEREMSCRIPTEN {
extern "C" {
#endif /* __cplusplus ? */

/** Registers the window manager backend "Emscripten" */
GUIDRIVEREMSCRIPTEN_PUBLIC_C CORE::Int32 GUCEF_PLUGIN_CALLSPEC_PREFIX
GUIDRIVEREMSCRIPTEN_ENTRY( GUCEFPlugin_Load )( CORE::UInt32 argc, const char** argv ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

GUIDRIVEREMSCRIPTEN_PUBLIC_C void GUCEF_PLUGIN_CALLSPEC_PREFIX
GUIDRIVEREMSCRIPTEN_ENTRY( GUCEFPlugin_Unload )( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

GUIDRIVEREMSCRIPTEN_PUBLIC_C void GUCEF_PLUGIN_CALLSPEC_PREFIX
GUIDRIVEREMSCRIPTEN_ENTRY( GUCEFPlugin_GetVersion )( CORE::TVersion* versionInfo ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

GUIDRIVEREMSCRIPTEN_PUBLIC_C const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
GUIDRIVEREMSCRIPTEN_ENTRY( GUCEFPlugin_GetCopyright )( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

GUIDRIVEREMSCRIPTEN_PUBLIC_C const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
GUIDRIVEREMSCRIPTEN_ENTRY( GUCEFPlugin_GetDescription )( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

#ifdef __cplusplus
}; /* extern "C" */
}; /* namespace GUIDRIVEREMSCRIPTEN */
}; /* namespace GUCEF */
#endif /* __cplusplus ? */

#endif /* GUCEF_GUIDRIVEREMSCRIPTEN_PLUGINAPI_H ? */
