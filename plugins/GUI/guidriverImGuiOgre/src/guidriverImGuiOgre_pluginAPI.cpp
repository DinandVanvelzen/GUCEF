/*
 *  guidriverImGuiOgre: gucefGUI driver ImGuiOgre: Dear ImGui drawn through Ogre over the 3D scene of an Ogre window
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

#ifndef GUCEF_GUI_CGUIMANAGER_H
#include "gucefGUI_CGUIManager.h"
#define GUCEF_GUI_CGUIMANAGER_H
#endif /* GUCEF_GUI_CGUIMANAGER_H ? */

#ifndef GUCEF_GUI_CGUIGLOBAL_H
#include "gucefGUI_CGuiGlobal.h"
#define GUCEF_GUI_CGUIGLOBAL_H
#endif /* GUCEF_GUI_CGUIGLOBAL_H ? */

#include "guidriverImGuiOgre_CImGuiOgreDriver.h"
#include "guidriverImGuiOgre_pluginAPI.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUIDRIVERIMGUIOGRE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

static CImGuiOgreDriver* g_guiDriver = GUCEF_NULL;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

CORE::Int32 GUCEF_PLUGIN_CALLSPEC_PREFIX
GUIDRIVERIMGUIOGRE_ENTRY( GUCEFPlugin_Load )( CORE::UInt32 argc, const char** argv ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    if ( GUCEF_NULL == g_guiDriver )
        g_guiDriver = GUCEF_NEW CImGuiOgreDriver();
    GUI::CGuiGlobal::Instance()->GetGuiManager().RegisterGUIDriver( CImGuiOgreDriver::RegistrationName, g_guiDriver );
    return 1;
}

/*--------------------------------------------------------------------------*/

void GUCEF_PLUGIN_CALLSPEC_PREFIX
GUIDRIVERIMGUIOGRE_ENTRY( GUCEFPlugin_Unload )( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    GUI::CGuiGlobal::Instance()->GetGuiManager().UnregisterGUIDriverByName( CImGuiOgreDriver::RegistrationName );
    GUCEF_DELETE g_guiDriver;
    g_guiDriver = GUCEF_NULL;
}

/*--------------------------------------------------------------------------*/

void GUCEF_PLUGIN_CALLSPEC_PREFIX
GUIDRIVERIMGUIOGRE_ENTRY( GUCEFPlugin_GetVersion )( CORE::TVersion* versionInfo ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    versionInfo->major = 0;
    versionInfo->minor = 1;
    versionInfo->patch = 0;
    versionInfo->release = 0;
}

/*--------------------------------------------------------------------------*/

const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
GUIDRIVERIMGUIOGRE_ENTRY( GUCEFPlugin_GetCopyright )( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    return "Copyright (C) Dinand Vanvelzen, Apache License 2.0";
}

/*--------------------------------------------------------------------------*/

const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
GUIDRIVERIMGUIOGRE_ENTRY( GUCEFPlugin_GetDescription )( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    return "GUI driver ImGuiOgre: Dear ImGui drawn through Ogre over the 3D scene of an Ogre window";
}

/*--------------------------------------------------------------------------*/

const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
GUIDRIVERIMGUIOGRE_ENTRY( GUCEFPlugin_GetLinkBackModuleDependencies )( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    return "gucefCORE,gucefGUI,guidriverImGui,guidriverOgre";
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUIDRIVERIMGUIOGRE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
