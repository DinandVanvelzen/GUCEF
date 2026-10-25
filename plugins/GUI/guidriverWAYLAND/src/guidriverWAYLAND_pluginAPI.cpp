/*
 *  guidriverWAYLAND: GUCEF window driver for Wayland: native windows with OpenGL through EGL
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

#ifndef GUCEF_GUI_CGUIGLOBAL_H
#include "gucefGUI_CGuiGlobal.h"
#define GUCEF_GUI_CGUIGLOBAL_H
#endif /* GUCEF_GUI_CGUIGLOBAL_H ? */

#ifndef GUCEF_GUI_CWINDOWMANAGER_H
#include "gucefGUI_CWindowManager.h"
#define GUCEF_GUI_CWINDOWMANAGER_H
#endif /* GUCEF_GUI_CWINDOWMANAGER_H ? */

#ifndef GUCEF_GUI_CVIDEOOPTIONS_H
#include "gucefGUI_CVideoOptions.h"
#define GUCEF_GUI_CVIDEOOPTIONS_H
#endif /* GUCEF_GUI_CVIDEOOPTIONS_H ? */

#ifndef GUCEF_GUIDRIVERWAYLAND_CWAYLANDDISPLAYMODES_H
#include "guidriverWAYLAND_CWaylandDisplayModes.h"
#define GUCEF_GUIDRIVERWAYLAND_CWAYLANDDISPLAYMODES_H
#endif /* GUCEF_GUIDRIVERWAYLAND_CWAYLANDDISPLAYMODES_H ? */

#ifndef GUCEF_GUIDRIVERWAYLAND_CWAYLANDWINDOWMANAGERIMP_H
#include "guidriverWAYLAND_CWaylandWindowManagerImp.h"
#define GUCEF_GUIDRIVERWAYLAND_CWAYLANDWINDOWMANAGERIMP_H
#endif /* GUCEF_GUIDRIVERWAYLAND_CWAYLANDWINDOWMANAGERIMP_H ? */

#include "guidriverWAYLAND_pluginAPI.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUIDRIVERWAYLAND {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

static TWaylandWindowManagerImpPtr g_windowManager;

/*-------------------------------------------------------------------------*/

CORE::Int32 GUCEF_PLUGIN_CALLSPEC_PREFIX
GUCEFPlugin_Load( CORE::UInt32 argc, const char** argv ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    // nothing is connected or loaded here: the backend is always registered and says with IsAvailable() whether this session can use it
    g_windowManager = TWaylandWindowManagerImpPtr( GUCEF_NEW CWaylandWindowManagerImp() );
    GUI::CGuiGlobal::Instance()->GetWindowManager().RegisterBackend( "Wayland", g_windowManager );

    // the monitors as the compositor sees them, for the video settings; used when the application chose Wayland
    GUI::CVideoOptions::RegisterDisplayModeSource( "Wayland", CWaylandDisplayModes::GetSource() );
    return 1;
}

/*-------------------------------------------------------------------------*/

void GUCEF_PLUGIN_CALLSPEC_PREFIX
GUCEFPlugin_Unload( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    GUI::CVideoOptions::UnregisterDisplayModeSource( "Wayland" );
    GUI::CGuiGlobal::Instance()->GetWindowManager().UnregisterBackend( "Wayland" );
    g_windowManager.Unlink();
}

/*-------------------------------------------------------------------------*/

void GUCEF_PLUGIN_CALLSPEC_PREFIX
GUCEFPlugin_GetVersion( CORE::TVersion* versionInfo ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    versionInfo->major = 0;
    versionInfo->minor = 1;
    versionInfo->patch = 0;
    versionInfo->release = 0;
}

/*-------------------------------------------------------------------------*/

const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
GUCEFPlugin_GetCopyright( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    return "Copyright (C) Dinand Vanvelzen, Apache License 2.0";
}

/*-------------------------------------------------------------------------*/

const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
GUCEFPlugin_GetDescription( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    return "Window management backend for Wayland compositors, drawing with OpenGL through EGL";
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUIDRIVERWAYLAND */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
