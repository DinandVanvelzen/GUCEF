/*
 *  inputdriverEMSCRIPTEN: GUCEF input driver for web browsers: mouse, wheel, keys and text of a page
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

#ifndef GUCEF_INPUTDRIVEREMSCRIPTEN_PLUGINAPI_H
#define GUCEF_INPUTDRIVEREMSCRIPTEN_PLUGINAPI_H

#ifndef GUCEF_CORE_ESTRUCTS_H
#include "EStructs.h"
#define GUCEF_CORE_ESTRUCTS_H
#endif /* GUCEF_CORE_ESTRUCTS_H ? */

#ifndef GUCEF_INPUTDRIVEREMSCRIPTEN_MACROS_H
#include "inputdriverEMSCRIPTEN_macros.h"
#define GUCEF_INPUTDRIVEREMSCRIPTEN_MACROS_H
#endif /* GUCEF_INPUTDRIVEREMSCRIPTEN_MACROS_H ? */

/*
 *  The plugin is always linked into the application (there is no loading of modules on the web), so its entry points
 *  have unique names, inputdriverEMSCRIPTEN_GUCEFPlugin_Load and so on, which the application calls itself.
 */
#define INPUTDRIVEREMSCRIPTEN_ENTRY( func ) inputdriverEMSCRIPTEN_##func

#ifdef __cplusplus
namespace GUCEF {
namespace INPUTDRIVEREMSCRIPTEN {
extern "C" {
#endif /* __cplusplus ? */

/** Registers the input driver "EMSCRIPTEN" with the input controller */
INPUTDRIVEREMSCRIPTEN_PUBLIC_C CORE::Int32 GUCEF_PLUGIN_CALLSPEC_PREFIX
INPUTDRIVEREMSCRIPTEN_ENTRY( GUCEFPlugin_Load )( CORE::UInt32 argc, const char** argv ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

INPUTDRIVEREMSCRIPTEN_PUBLIC_C void GUCEF_PLUGIN_CALLSPEC_PREFIX
INPUTDRIVEREMSCRIPTEN_ENTRY( GUCEFPlugin_Unload )( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

INPUTDRIVEREMSCRIPTEN_PUBLIC_C void GUCEF_PLUGIN_CALLSPEC_PREFIX
INPUTDRIVEREMSCRIPTEN_ENTRY( GUCEFPlugin_GetVersion )( CORE::TVersion* versionInfo ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

INPUTDRIVEREMSCRIPTEN_PUBLIC_C const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
INPUTDRIVEREMSCRIPTEN_ENTRY( GUCEFPlugin_GetCopyright )( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

INPUTDRIVEREMSCRIPTEN_PUBLIC_C const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
INPUTDRIVEREMSCRIPTEN_ENTRY( GUCEFPlugin_GetDescription )( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

#ifdef __cplusplus
}; /* extern "C" */
}; /* namespace INPUTDRIVEREMSCRIPTEN */
}; /* namespace GUCEF */
#endif /* __cplusplus ? */

#endif /* GUCEF_INPUTDRIVEREMSCRIPTEN_PLUGINAPI_H ? */
