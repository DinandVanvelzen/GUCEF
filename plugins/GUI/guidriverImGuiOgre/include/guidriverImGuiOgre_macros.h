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

#ifndef GUIDRIVERIMGUIOGRE_MACROS_H
#define GUIDRIVERIMGUIOGRE_MACROS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GUI_MACROS_H
#include "gucefGUI_macros.h"
#define GUCEF_GUI_MACROS_H
#endif /* GUCEF_GUI_MACROS_H ? */

#ifndef GUIDRIVERIMGUIOGRE_CONFIG_H
#include "guidriverImGuiOgre_config.h"
#define GUIDRIVERIMGUIOGRE_CONFIG_H
#endif /* GUIDRIVERIMGUIOGRE_CONFIG_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      GENERAL MACROS                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

#undef GUIDRIVERIMGUIOGRE_EXPORTSPEC
#ifdef GUIDRIVERIMGUIOGRE_BUILD_MODULE
  #define GUIDRIVERIMGUIOGRE_EXPORTSPEC GUCEF_EXPORT
#else
  #define GUIDRIVERIMGUIOGRE_EXPORTSPEC GUCEF_IMPORT
#endif /* GUIDRIVERIMGUIOGRE_BUILD_MODULE ? */

#undef GUIDRIVERIMGUIOGRE_PUBLIC_CPP
#ifdef GUIDRIVERIMGUIOGRE_PUBLIC_CPP_CODE
  #define GUIDRIVERIMGUIOGRE_PUBLIC_CPP GUIDRIVERIMGUIOGRE_EXPORTSPEC
#else
  #define GUIDRIVERIMGUIOGRE_PUBLIC_CPP GUCEF_HIDDEN
#endif /* GUIDRIVERIMGUIOGRE_PUBLIC_CPP_CODE ? */

#define GUIDRIVERIMGUIOGRE_PRIVATE_CPP GUCEF_HIDDEN

#undef GUIDRIVERIMGUIOGRE_PUBLIC_C
#ifdef GUIDRIVERIMGUIOGRE_PUBLIC_C_CODE
  #define GUIDRIVERIMGUIOGRE_PUBLIC_C GUIDRIVERIMGUIOGRE_EXPORTSPEC
#else
  #define GUIDRIVERIMGUIOGRE_PUBLIC_C GUCEF_HIDDEN
#endif /* GUIDRIVERIMGUIOGRE_PUBLIC_C_CODE ? */

#define GUIDRIVERIMGUIOGRE_PRIVATE_C GUCEF_HIDDEN

/*-------------------------------------------------------------------------*/

#endif /* GUIDRIVERIMGUIOGRE_MACROS_H ? */
