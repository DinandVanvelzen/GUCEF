/*
 *  imgui: Dear ImGui as a module of GUCEF, the hooks through which GUCEF receives its assertions and debug log
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
 *  DV edit: added to Dear ImGui. imconfig.h routes IM_ASSERT() and IMGUI_DEBUG_PRINTF() to GucefImGui_AssertFailed()
 *  and GucefImGui_DebugPrintf() (imgui_gucef.cpp), which call the handlers set here. The module has no link to the
 *  GUCEF libraries: the GUI driver library of Dear ImGui sets the handlers that write to the GUCEF log.
 */

#pragma once

#include "imgui.h"

/** Called for a failed IM_ASSERT() with the expression as text, the source file and the line */
typedef void (*GucefImGui_AssertHandler)(const char* expression, const char* file, int line);

/** Called with a line of the debug log of Dear ImGui, already formatted */
typedef void (*GucefImGui_DebugPrintHandler)(const char* text);

/**
 *  Sets the handler of failed assertions, NULL for none. Without a handler a failed assertion stops a build without
 *  NDEBUG through assert() and is ignored otherwise
 */
IMGUI_API void GucefImGui_SetAssertHandler(GucefImGui_AssertHandler handler);
IMGUI_API GucefImGui_AssertHandler GucefImGui_GetAssertHandler();

/** Sets the handler of the debug log, NULL for none: the text is then dropped */
IMGUI_API void GucefImGui_SetDebugPrintHandler(GucefImGui_DebugPrintHandler handler);
IMGUI_API GucefImGui_DebugPrintHandler GucefImGui_GetDebugPrintHandler();
