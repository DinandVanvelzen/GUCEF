/*
 *  guidriverRmlUIOgre: gucefGUI driver drawing RmlUi through Ogre, over the 3D scene of an Ogre window
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

#ifndef GUIDRIVERRMLUIOGRE_MACROS_H
#define GUIDRIVERRMLUIOGRE_MACROS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GUI_MACROS_H
#include "gucefGUI_macros.h"
#define GUCEF_GUI_MACROS_H
#endif /* GUCEF_GUI_MACROS_H ? */

#ifndef GUIDRIVERRMLUIOGRE_CONFIG_H
#include "guidriverRmlUIOgre_config.h"
#define GUIDRIVERRMLUIOGRE_CONFIG_H
#endif /* GUIDRIVERRMLUIOGRE_CONFIG_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      GENERAL MACROS                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

#undef GUIDRIVERRMLUIOGRE_EXPORTSPEC
#ifdef GUIDRIVERRMLUIOGRE_BUILD_MODULE
  #define GUIDRIVERRMLUIOGRE_EXPORTSPEC GUCEF_EXPORT
#else
  #define GUIDRIVERRMLUIOGRE_EXPORTSPEC GUCEF_IMPORT
#endif /* GUIDRIVERRMLUIOGRE_BUILD_MODULE ? */

#undef GUIDRIVERRMLUIOGRE_PUBLIC_CPP
#ifdef GUIDRIVERRMLUIOGRE_PUBLIC_CPP_CODE
  #define GUIDRIVERRMLUIOGRE_PUBLIC_CPP GUIDRIVERRMLUIOGRE_EXPORTSPEC
#else
  #define GUIDRIVERRMLUIOGRE_PUBLIC_CPP GUCEF_HIDDEN
#endif /* GUIDRIVERRMLUIOGRE_PUBLIC_CPP_CODE ? */

#define GUIDRIVERRMLUIOGRE_PRIVATE_CPP GUCEF_HIDDEN

#undef GUIDRIVERRMLUIOGRE_PUBLIC_C
#ifdef GUIDRIVERRMLUIOGRE_PUBLIC_C_CODE
  #define GUIDRIVERRMLUIOGRE_PUBLIC_C GUIDRIVERRMLUIOGRE_EXPORTSPEC
#else
  #define GUIDRIVERRMLUIOGRE_PUBLIC_C GUCEF_HIDDEN
#endif /* GUIDRIVERRMLUIOGRE_PUBLIC_C_CODE ? */

#define GUIDRIVERRMLUIOGRE_PRIVATE_C GUCEF_HIDDEN

/*-------------------------------------------------------------------------*/

#endif /* GUIDRIVERRMLUIOGRE_MACROS_H ? */
