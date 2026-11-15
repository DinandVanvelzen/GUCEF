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
 *  DV edit: added to Dear ImGui, see imgui_gucef.h
 */

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>

#include "imgui_gucef.h"

#ifndef IMGUI_DISABLE

namespace {

GucefImGui_AssertHandler g_assertHandler = NULL;
GucefImGui_DebugPrintHandler g_debugPrintHandler = NULL;

}

void
GucefImGui_SetAssertHandler(GucefImGui_AssertHandler handler)
{
    g_assertHandler = handler;
}

GucefImGui_AssertHandler
GucefImGui_GetAssertHandler()
{
    return g_assertHandler;
}

void
GucefImGui_SetDebugPrintHandler(GucefImGui_DebugPrintHandler handler)
{
    g_debugPrintHandler = handler;
}

GucefImGui_DebugPrintHandler
GucefImGui_GetDebugPrintHandler()
{
    return g_debugPrintHandler;
}

void
GucefImGui_AssertFailed(const char* expression, const char* file, int line)
{
    GucefImGui_AssertHandler handler = g_assertHandler;
    if (NULL != handler)
    {
        handler(expression, file, line);
        return;
    }
    // no handler: what IM_ASSERT() does without this configuration
    assert(!"Dear ImGui assertion failed");
    (void) expression;
    (void) file;
    (void) line;
}

void
GucefImGui_DebugPrintf(const char* format, ...)
{
    GucefImGui_DebugPrintHandler handler = g_debugPrintHandler;
    if (NULL == handler || NULL == format)
        return;

    char buffer[1024];
    va_list args;
    va_start(args, format);
    int length = vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    if (length < 0)
        return;
    if ((size_t) length < sizeof(buffer))
    {
        handler(buffer);
        return;
    }

    // longer than the buffer on the stack
    char* text = (char*) IM_ALLOC((size_t) length + 1);
    va_start(args, format);
    vsnprintf(text, (size_t) length + 1, format, args);
    va_end(args);
    handler(text);
    IM_FREE(text);
}

#endif // IMGUI_DISABLE
