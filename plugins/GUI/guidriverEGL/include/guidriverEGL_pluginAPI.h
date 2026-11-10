/*
 *  guidriverEGL: GUCEF window driver giving offscreen OpenGL windows through EGL, for tests, tools and headless runs
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

#ifndef GUCEF_GUIDRIVEREGL_PLUGINAPI_H
#define GUCEF_GUIDRIVEREGL_PLUGINAPI_H

#ifndef GUCEF_CORE_ESTRUCTS_H
#include "EStructs.h"
#define GUCEF_CORE_ESTRUCTS_H
#endif /* GUCEF_CORE_ESTRUCTS_H ? */

#ifndef GUCEF_GUIDRIVEREGL_MACROS_H
#include "guidriverEGL_macros.h"
#define GUCEF_GUIDRIVEREGL_MACROS_H
#endif /* GUCEF_GUIDRIVEREGL_MACROS_H ? */

#ifdef __cplusplus
namespace GUCEF {
namespace GUIDRIVEREGL {
extern "C" {
#endif /* __cplusplus ? */

GUIDRIVEREGL_PUBLIC_C CORE::Int32 GUCEF_PLUGIN_CALLSPEC_PREFIX
GUCEFPlugin_Load( CORE::UInt32 argc, const char** argv ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

GUIDRIVEREGL_PUBLIC_C void GUCEF_PLUGIN_CALLSPEC_PREFIX
GUCEFPlugin_Unload( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

GUIDRIVEREGL_PUBLIC_C void GUCEF_PLUGIN_CALLSPEC_PREFIX
GUCEFPlugin_GetVersion( CORE::TVersion* versionInfo ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

GUIDRIVEREGL_PUBLIC_C const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
GUCEFPlugin_GetCopyright( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

GUIDRIVEREGL_PUBLIC_C const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
GUCEFPlugin_GetDescription( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

#ifdef __cplusplus
}; /* extern "C" */
}; /* namespace GUIDRIVEREGL */
}; /* namespace GUCEF */
#endif /* __cplusplus ? */

#endif /* GUCEF_GUIDRIVEREGL_PLUGINAPI_H ? */
