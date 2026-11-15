/*
 *  guidriverImGuiOpenGL: gucefGUI driver ImGui: Dear ImGui drawn with OpenGL
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

#ifndef GUIDRIVERIMGUIGL_MACROS_H
#define GUIDRIVERIMGUIGL_MACROS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GUI_MACROS_H
#include "gucefGUI_macros.h"
#define GUCEF_GUI_MACROS_H
#endif /* GUCEF_GUI_MACROS_H ? */

#ifndef GUIDRIVERIMGUIGL_CONFIG_H
#include "guidriverImGuiOpenGL_config.h"
#define GUIDRIVERIMGUIGL_CONFIG_H
#endif /* GUIDRIVERIMGUIGL_CONFIG_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      GENERAL MACROS                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

#undef GUIDRIVERIMGUIGL_EXPORTSPEC
#ifdef GUIDRIVERIMGUIGL_BUILD_MODULE
  #define GUIDRIVERIMGUIGL_EXPORTSPEC GUCEF_EXPORT
#else
  #define GUIDRIVERIMGUIGL_EXPORTSPEC GUCEF_IMPORT
#endif /* GUIDRIVERIMGUIGL_BUILD_MODULE ? */

#undef GUIDRIVERIMGUIGL_PUBLIC_CPP
#ifdef GUIDRIVERIMGUIGL_PUBLIC_CPP_CODE
  #define GUIDRIVERIMGUIGL_PUBLIC_CPP GUIDRIVERIMGUIGL_EXPORTSPEC
#else
  #define GUIDRIVERIMGUIGL_PUBLIC_CPP GUCEF_HIDDEN
#endif /* GUIDRIVERIMGUIGL_PUBLIC_CPP_CODE ? */

#define GUIDRIVERIMGUIGL_PRIVATE_CPP GUCEF_HIDDEN

#undef GUIDRIVERIMGUIGL_PUBLIC_C
#ifdef GUIDRIVERIMGUIGL_PUBLIC_C_CODE
  #define GUIDRIVERIMGUIGL_PUBLIC_C GUIDRIVERIMGUIGL_EXPORTSPEC
#else
  #define GUIDRIVERIMGUIGL_PUBLIC_C GUCEF_HIDDEN
#endif /* GUIDRIVERIMGUIGL_PUBLIC_C_CODE ? */

#define GUIDRIVERIMGUIGL_PRIVATE_C GUCEF_HIDDEN

/*-------------------------------------------------------------------------*/

#endif /* GUIDRIVERIMGUIGL_MACROS_H ? */
