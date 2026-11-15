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

/*
 *      The main include header of guidriverImGui: Dear ImGui as gucefGUI contexts, shared by the renderer drivers of
 *      Dear ImGui (guidriverImGuiOpenGL, guidriverImGuiOgre). It needs C++11, as Dear ImGui does.
 */

#ifndef GUIDRIVERIMGUI_H
#define GUIDRIVERIMGUI_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUIDRIVERIMGUI_ETYPES_H
#include "guidriverImGui_ETypes.h"
#define GUIDRIVERIMGUI_ETYPES_H
#endif /* GUIDRIVERIMGUI_ETYPES_H ? */

#ifndef GUIDRIVERIMGUI_MACROS_H
#include "guidriverImGui_macros.h"
#define GUIDRIVERIMGUI_MACROS_H
#endif /* GUIDRIVERIMGUI_MACROS_H ? */

#ifndef GUIDRIVERIMGUI_CONVERSIONS_H
#include "guidriverImGui_Conversions.h"
#define GUIDRIVERIMGUI_CONVERSIONS_H
#endif /* GUIDRIVERIMGUI_CONVERSIONS_H ? */

#ifndef GUIDRIVERIMGUI_CIMGUIGLOBAL_H
#include "guidriverImGui_CImGuiGlobal.h"
#define GUIDRIVERIMGUI_CIMGUIGLOBAL_H
#endif /* GUIDRIVERIMGUI_CIMGUIGLOBAL_H ? */

#ifndef GUIDRIVERIMGUI_CIMGUICONTEXTSCOPE_H
#include "guidriverImGui_CImGuiContextScope.h"
#define GUIDRIVERIMGUI_CIMGUICONTEXTSCOPE_H
#endif /* GUIDRIVERIMGUI_CIMGUICONTEXTSCOPE_H ? */

#ifndef GUIDRIVERIMGUI_CIMGUIINPUTMAPPER_H
#include "guidriverImGui_CImGuiInputMapper.h"
#define GUIDRIVERIMGUI_CIMGUIINPUTMAPPER_H
#endif /* GUIDRIVERIMGUI_CIMGUIINPUTMAPPER_H ? */

#ifndef GUIDRIVERIMGUI_CIMGUIFONTLOADER_H
#include "guidriverImGui_CImGuiFontLoader.h"
#define GUIDRIVERIMGUI_CIMGUIFONTLOADER_H
#endif /* GUIDRIVERIMGUI_CIMGUIFONTLOADER_H ? */

#ifndef GUIDRIVERIMGUI_CIMGUITEXTURECACHE_H
#include "guidriverImGui_CImGuiTextureCache.h"
#define GUIDRIVERIMGUI_CIMGUITEXTURECACHE_H
#endif /* GUIDRIVERIMGUI_CIMGUITEXTURECACHE_H ? */

#ifndef GUIDRIVERIMGUI_CIMGUISOFTWARERENDERER_H
#include "guidriverImGui_CImGuiSoftwareRenderer.h"
#define GUIDRIVERIMGUI_CIMGUISOFTWARERENDERER_H
#endif /* GUIDRIVERIMGUI_CIMGUISOFTWARERENDERER_H ? */

#ifndef GUIDRIVERIMGUI_CIMGUIWIDGETHANDLER_H
#include "guidriverImGui_CImGuiWidgetHandler.h"
#define GUIDRIVERIMGUI_CIMGUIWIDGETHANDLER_H
#endif /* GUIDRIVERIMGUI_CIMGUIWIDGETHANDLER_H ? */

#ifndef GUIDRIVERIMGUI_CIMGUIDRAWLISTPAINTER_H
#include "guidriverImGui_CImGuiDrawListPainter.h"
#define GUIDRIVERIMGUI_CIMGUIDRAWLISTPAINTER_H
#endif /* GUIDRIVERIMGUI_CIMGUIDRAWLISTPAINTER_H ? */

#ifndef GUIDRIVERIMGUI_CIMGUICONTEXT_H
#include "guidriverImGui_CImGuiContext.h"
#define GUIDRIVERIMGUI_CIMGUICONTEXT_H
#endif /* GUIDRIVERIMGUI_CIMGUICONTEXT_H ? */

#ifndef GUIDRIVERIMGUI_CIMGUIFRAMEBUILDER_H
#include "guidriverImGui_CImGuiFrameBuilder.h"
#define GUIDRIVERIMGUI_CIMGUIFRAMEBUILDER_H
#endif /* GUIDRIVERIMGUI_CIMGUIFRAMEBUILDER_H ? */

#ifndef GUIDRIVERIMGUI_CIMGUIDRIVER_H
#include "guidriverImGui_CImGuiDriver.h"
#define GUIDRIVERIMGUI_CIMGUIDRIVER_H
#endif /* GUIDRIVERIMGUI_CIMGUIDRIVER_H ? */

#ifndef GUIDRIVERIMGUI_PLUGINAPI_H
#include "guidriverImGui_pluginAPI.h"
#define GUIDRIVERIMGUI_PLUGINAPI_H
#endif /* GUIDRIVERIMGUI_PLUGINAPI_H ? */

/*-------------------------------------------------------------------------*/

#endif /* GUIDRIVERIMGUI_H ? */
