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


#ifndef GUCEF_GUIDRIVERWAYLAND_MACROS_H
#define GUCEF_GUIDRIVERWAYLAND_MACROS_H

#ifndef GUCEF_GUI_MACROS_H
#include "gucefGUI_macros.h"
#define GUCEF_GUI_MACROS_H
#endif /* GUCEF_GUI_MACROS_H ? */

#ifndef GUCEF_GUIDRIVERWAYLAND_CONFIG_H
#include "guidriverWAYLAND_config.h"
#define GUCEF_GUIDRIVERWAYLAND_CONFIG_H
#endif /* GUCEF_GUIDRIVERWAYLAND_CONFIG_H ? */

#undef GUIDRIVERWAYLAND_EXPORTSPEC
#ifdef GUIDRIVERWAYLAND_BUILD_MODULE
  #define GUIDRIVERWAYLAND_EXPORTSPEC GUCEF_EXPORT
#else
  #define GUIDRIVERWAYLAND_EXPORTSPEC GUCEF_IMPORT
#endif /* GUIDRIVERWAYLAND_BUILD_MODULE ? */

#undef GUIDRIVERWAYLAND_PUBLIC_CPP
#ifdef GUIDRIVERWAYLAND_PUBLIC_CPP_CODE
  #define GUIDRIVERWAYLAND_PUBLIC_CPP GUIDRIVERWAYLAND_EXPORTSPEC
#else
  #define GUIDRIVERWAYLAND_PUBLIC_CPP GUCEF_HIDDEN
#endif /* GUIDRIVERWAYLAND_PUBLIC_CPP_CODE ? */

#undef GUIDRIVERWAYLAND_PUBLIC_C
#ifdef GUIDRIVERWAYLAND_PUBLIC_C_CODE
  #define GUIDRIVERWAYLAND_PUBLIC_C GUIDRIVERWAYLAND_EXPORTSPEC
#else
  #define GUIDRIVERWAYLAND_PUBLIC_C GUCEF_HIDDEN
#endif /* GUIDRIVERWAYLAND_PUBLIC_C_CODE ? */

#endif /* GUCEF_GUIDRIVERWAYLAND_MACROS_H ? */
