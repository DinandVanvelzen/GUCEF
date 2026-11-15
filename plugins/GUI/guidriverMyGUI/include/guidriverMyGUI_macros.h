/*
 *  guidriverMyGUI: GUI backend using MyGUI
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

#ifndef GUCEF_guidriverMyGUI_MACROS_H
#define GUCEF_guidriverMyGUI_MACROS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_guidriverMyGUI_ETYPES_H
#include "guidriverMyGUI_ETypes.h"
#define GUCEF_guidriverMyGUI_ETYPES_H
#endif /* GUCEF_guidriverMyGUI_ETYPES_H ? */

#ifndef GUCEF_guidriverMyGUI_CONFIG_H
#include "guidriverMyGUI_config.h"        /* Module build configuration */
#define GUCEF_guidriverMyGUI_CONFIG_H
#endif /* GUCEF_guidriverMyGUI_CONFIG_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      GENERAL MACROS                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *      Macros for dynamic linking or static linking. Use the switches in the
 *      config file to control the export type.
 */
#undef GUCEF_EXPORTSPEC
#if defined( GUIDRIVERMYGUI_BUILD_MODULE ) || defined( GUCEF_guidriverMyGUI_BUILD_MODULE )
  #define GUCEF_EXPORTSPEC GUCEF_EXPORT
#else
  #define GUCEF_EXPORTSPEC GUCEF_IMPORT
#endif /* GUIDRIVERMYGUI_BUILD_MODULE ? */

#undef GUCEF_guidriverMyGUI_PUBLIC_CPP
#ifdef GUCEF_guidriverMyGUI_PUBLIC_CPP_CODE
  #define GUCEF_guidriverMyGUI_PUBLIC_CPP GUCEF_EXPORTSPEC
#else
  #define GUCEF_guidriverMyGUI_PUBLIC_CPP GUCEF_HIDDEN
#endif /* GUCEF_guidriverMyGUI_PUBLIC_CPP_CODE ? */

#undef GUCEF_guidriverMyGUI_PRIVATE_CPP
#define GUCEF_guidriverMyGUI_PRIVATE_CPP GUCEF_HIDDEN

#undef GUCEF_guidriverMyGUI_PUBLIC_C
#ifdef GUCEF_guidriverMyGUI_PUBLIC_C_CODE
  #define GUCEF_guidriverMyGUI_PUBLIC_C GUCEF_EXPORTSPEC
#else
  #define GUCEF_guidriverMyGUI_PUBLIC_C GUCEF_HIDDEN
#endif /* GUCEF_guidriverMyGUI_PUBLIC_C_CODE ? */

#undef GUCEF_guidriverMyGUI_PRIVATE_C
#define GUCEF_guidriverMyGUI_PRIVATE_C GUCEF_HIDDEN

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_guidriverMyGUI_MACROS_H ? */
