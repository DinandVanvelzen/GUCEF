/*
 *  guidriverMyGUIOpenGL: GUI backend using MyGUI with OpenGL, OpenGL 3 or OpenGL ES 3 / WebGL 2
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

#ifndef GUCEF_guidriverMyGUIOpenGL_MACROS_H
#define GUCEF_guidriverMyGUIOpenGL_MACROS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_guidriverMyGUIOpenGL_ETYPES_H
#include "guidriverMyGUIOpenGL_ETypes.h"
#define GUCEF_guidriverMyGUIOpenGL_ETYPES_H
#endif /* GUCEF_guidriverMyGUIOpenGL_ETYPES_H ? */

#ifndef GUCEF_guidriverMyGUIOpenGL_CONFIG_H
#include "guidriverMyGUIOpenGL_config.h"        /* Module build configuration */
#define GUCEF_guidriverMyGUIOpenGL_CONFIG_H
#endif /* GUCEF_guidriverMyGUIOpenGL_CONFIG_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      GENERAL MACROS                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *      Macros for dynamic linking or static linking. Use the switches in the
 *      config file to control the export type.
 */
#undef GUCEF_guidriverMyGUIOpenGL_EXPORTSPEC
#if defined( GUIDRIVERMYGUIOPENGL_BUILD_MODULE ) || defined( GUCEF_guidriverMyGUIOpenGL_BUILD_MODULE )
  #define GUCEF_guidriverMyGUIOpenGL_EXPORTSPEC GUCEF_EXPORT
#else
  #define GUCEF_guidriverMyGUIOpenGL_EXPORTSPEC GUCEF_IMPORT
#endif /* GUIDRIVERMYGUIOPENGL_BUILD_MODULE ? */

#undef GUCEF_guidriverMyGUIOpenGL_PUBLIC_CPP
#ifdef GUCEF_guidriverMyGUIOpenGL_PUBLIC_CPP_CODE
  #define GUCEF_guidriverMyGUIOpenGL_PUBLIC_CPP GUCEF_guidriverMyGUIOpenGL_EXPORTSPEC
#else
  #define GUCEF_guidriverMyGUIOpenGL_PUBLIC_CPP GUCEF_HIDDEN
#endif /* GUCEF_guidriverMyGUIOpenGL_PUBLIC_CPP_CODE ? */

#undef GUCEF_guidriverMyGUIOpenGL_PRIVATE_CPP
#define GUCEF_guidriverMyGUIOpenGL_PRIVATE_CPP GUCEF_HIDDEN

#undef GUCEF_guidriverMyGUIOpenGL_PUBLIC_C
#ifdef GUCEF_guidriverMyGUIOpenGL_PUBLIC_C_CODE
  #define GUCEF_guidriverMyGUIOpenGL_PUBLIC_C GUCEF_guidriverMyGUIOpenGL_EXPORTSPEC
#else
  #define GUCEF_guidriverMyGUIOpenGL_PUBLIC_C GUCEF_HIDDEN
#endif /* GUCEF_guidriverMyGUIOpenGL_PUBLIC_C_CODE ? */

#undef GUCEF_guidriverMyGUIOpenGL_PRIVATE_C
#define GUCEF_guidriverMyGUIOpenGL_PRIVATE_C GUCEF_HIDDEN

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_guidriverMyGUIOpenGL_MACROS_H ? */
