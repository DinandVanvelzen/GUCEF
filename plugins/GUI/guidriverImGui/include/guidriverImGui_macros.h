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


#ifndef GUIDRIVERIMGUI_MACROS_H
#define GUIDRIVERIMGUI_MACROS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GUI_MACROS_H
#include "gucefGUI_macros.h"
#define GUCEF_GUI_MACROS_H
#endif /* GUCEF_GUI_MACROS_H ? */

#ifndef GUIDRIVERIMGUI_CONFIG_H
#include "guidriverImGui_config.h"
#define GUIDRIVERIMGUI_CONFIG_H
#endif /* GUIDRIVERIMGUI_CONFIG_H ? */

#ifndef GUIDRIVERIMGUI_ETYPES_H
#include "guidriverImGui_ETypes.h"
#define GUIDRIVERIMGUI_ETYPES_H
#endif /* GUIDRIVERIMGUI_ETYPES_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      GENERAL MACROS                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

#undef GUIDRIVERIMGUI_EXPORTSPEC
#ifdef GUIDRIVERIMGUI_BUILD_MODULE
  #define GUIDRIVERIMGUI_EXPORTSPEC GUCEF_EXPORT
#else
  #define GUIDRIVERIMGUI_EXPORTSPEC GUCEF_IMPORT
#endif /* GUIDRIVERIMGUI_BUILD_MODULE ? */

#undef GUIDRIVERIMGUI_PUBLIC_CPP
#ifdef GUIDRIVERIMGUI_PUBLIC_CPP_CODE
  #define GUIDRIVERIMGUI_PUBLIC_CPP GUIDRIVERIMGUI_EXPORTSPEC
#else
  #define GUIDRIVERIMGUI_PUBLIC_CPP GUCEF_HIDDEN
#endif /* GUIDRIVERIMGUI_PUBLIC_CPP_CODE ? */

#define GUIDRIVERIMGUI_PRIVATE_CPP GUCEF_HIDDEN

#undef GUIDRIVERIMGUI_PUBLIC_C
#ifdef GUIDRIVERIMGUI_PUBLIC_C_CODE
  #define GUIDRIVERIMGUI_PUBLIC_C GUIDRIVERIMGUI_EXPORTSPEC
#else
  #define GUIDRIVERIMGUI_PUBLIC_C GUCEF_HIDDEN
#endif /* GUIDRIVERIMGUI_PUBLIC_C_CODE ? */

#define GUIDRIVERIMGUI_PRIVATE_C GUCEF_HIDDEN

/*-------------------------------------------------------------------------*/

#endif /* GUIDRIVERIMGUI_MACROS_H ? */
