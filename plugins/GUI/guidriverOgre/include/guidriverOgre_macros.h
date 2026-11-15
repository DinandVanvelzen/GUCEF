/*
 *  guidriverOgre: gucefGUI window backend that gives Ogre render windows on the platform windows
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

#ifndef GUIDRIVEROGRE_MACROS_H
#define GUIDRIVEROGRE_MACROS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GUI_MACROS_H
#include "gucefGUI_macros.h"
#define GUCEF_GUI_MACROS_H
#endif /* GUCEF_GUI_MACROS_H ? */

#ifndef GUIDRIVEROGRE_CONFIG_H
#include "guidriverOgre_config.h"
#define GUIDRIVEROGRE_CONFIG_H
#endif /* GUIDRIVEROGRE_CONFIG_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      GENERAL MACROS                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

#undef GUIDRIVEROGRE_EXPORTSPEC
#ifdef GUIDRIVEROGRE_BUILD_MODULE
  #define GUIDRIVEROGRE_EXPORTSPEC GUCEF_EXPORT
#else
  #define GUIDRIVEROGRE_EXPORTSPEC GUCEF_IMPORT
#endif /* GUIDRIVEROGRE_BUILD_MODULE ? */

#undef GUIDRIVEROGRE_PUBLIC_CPP
#ifdef GUIDRIVEROGRE_PUBLIC_CPP_CODE
  #define GUIDRIVEROGRE_PUBLIC_CPP GUIDRIVEROGRE_EXPORTSPEC
#else
  #define GUIDRIVEROGRE_PUBLIC_CPP GUCEF_HIDDEN
#endif /* GUIDRIVEROGRE_PUBLIC_CPP_CODE ? */

#define GUIDRIVEROGRE_PRIVATE_CPP GUCEF_HIDDEN

#undef GUIDRIVEROGRE_PUBLIC_C
#ifdef GUIDRIVEROGRE_PUBLIC_C_CODE
  #define GUIDRIVEROGRE_PUBLIC_C GUIDRIVEROGRE_EXPORTSPEC
#else
  #define GUIDRIVEROGRE_PUBLIC_C GUCEF_HIDDEN
#endif /* GUIDRIVEROGRE_PUBLIC_C_CODE ? */

#define GUIDRIVEROGRE_PRIVATE_C GUCEF_HIDDEN

/*-------------------------------------------------------------------------*/

#endif /* GUIDRIVEROGRE_MACROS_H ? */
