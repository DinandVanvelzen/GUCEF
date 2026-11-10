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

#ifndef GUCEF_GUIDRIVEREGL_MACROS_H
#define GUCEF_GUIDRIVEREGL_MACROS_H

#ifndef GUCEF_GUI_MACROS_H
#include "gucefGUI_macros.h"
#define GUCEF_GUI_MACROS_H
#endif /* GUCEF_GUI_MACROS_H ? */

#ifndef GUCEF_GUIDRIVEREGL_CONFIG_H
#include "guidriverEGL_config.h"
#define GUCEF_GUIDRIVEREGL_CONFIG_H
#endif /* GUCEF_GUIDRIVEREGL_CONFIG_H ? */

#undef GUIDRIVEREGL_EXPORTSPEC
#ifdef GUIDRIVEREGL_BUILD_MODULE
  #define GUIDRIVEREGL_EXPORTSPEC GUCEF_EXPORT
#else
  #define GUIDRIVEREGL_EXPORTSPEC GUCEF_IMPORT
#endif /* GUIDRIVEREGL_BUILD_MODULE ? */

#undef GUIDRIVEREGL_PUBLIC_CPP
#ifdef GUIDRIVEREGL_PUBLIC_CPP_CODE
  #define GUIDRIVEREGL_PUBLIC_CPP GUIDRIVEREGL_EXPORTSPEC
#else
  #define GUIDRIVEREGL_PUBLIC_CPP GUCEF_HIDDEN
#endif /* GUIDRIVEREGL_PUBLIC_CPP_CODE ? */

#undef GUIDRIVEREGL_PUBLIC_C
#ifdef GUIDRIVEREGL_PUBLIC_C_CODE
  #define GUIDRIVEREGL_PUBLIC_C GUIDRIVEREGL_EXPORTSPEC
#else
  #define GUIDRIVEREGL_PUBLIC_C GUCEF_HIDDEN
#endif /* GUIDRIVEREGL_PUBLIC_C_CODE ? */

#endif /* GUCEF_GUIDRIVEREGL_MACROS_H ? */
