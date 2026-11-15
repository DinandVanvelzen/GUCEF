/*
 *  guidriverImGui: the gucefGUI driver library of Dear ImGui, shared by its renderer drivers
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

#include "guidriverImGui_pluginAPI.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUIDRIVERIMGUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

CORE::Int32 GUCEF_PLUGIN_CALLSPEC_PREFIX
GUIDRIVERIMGUI_ENTRY( GUCEFPlugin_Load )( CORE::UInt32 argc, const char** argv ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    // a library of the renderer drivers: what it offers is used by linking to it, nothing is registered
    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "guidriverImGui: Loaded, the Dear ImGui GUI drivers are registered by the renderer plugins" );
    return 1;
}

/*--------------------------------------------------------------------------*/

void GUCEF_PLUGIN_CALLSPEC_PREFIX
GUIDRIVERIMGUI_ENTRY( GUCEFPlugin_Unload )( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "guidriverImGui: Unloaded" );
}

/*--------------------------------------------------------------------------*/

void GUCEF_PLUGIN_CALLSPEC_PREFIX
GUIDRIVERIMGUI_ENTRY( GUCEFPlugin_GetVersion )( CORE::TVersion* versionInfo ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    versionInfo->major = 0;
    versionInfo->minor = 1;
    versionInfo->patch = 0;
    versionInfo->release = 0;
}

/*--------------------------------------------------------------------------*/

const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
GUIDRIVERIMGUI_ENTRY( GUCEFPlugin_GetCopyright )( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    return "Copyright (C) Dinand Vanvelzen, Apache License 2.0";
}

/*--------------------------------------------------------------------------*/

const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
GUIDRIVERIMGUI_ENTRY( GUCEFPlugin_GetDescription )( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    return "The gucefGUI driver library of Dear ImGui, shared by its renderer drivers";
}

/*--------------------------------------------------------------------------*/

const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
GUIDRIVERIMGUI_ENTRY( GUCEFPlugin_GetLinkBackModuleDependencies )( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    return "gucefCORE,gucefGUI";
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUIDRIVERIMGUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
