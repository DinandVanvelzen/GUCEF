/*
 *  guidriverEmscripten: GUCEF window driver for web browsers: a canvas with WebGL 2
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

#ifndef GUCEF_GUIDRIVEREMSCRIPTEN_MACROS_H
#define GUCEF_GUIDRIVEREMSCRIPTEN_MACROS_H

#ifndef GUCEF_GUI_MACROS_H
#include "gucefGUI_macros.h"
#define GUCEF_GUI_MACROS_H
#endif /* GUCEF_GUI_MACROS_H ? */

#ifndef GUCEF_GUIDRIVEREMSCRIPTEN_CONFIG_H
#include "guidriverEmscripten_config.h"
#define GUCEF_GUIDRIVEREMSCRIPTEN_CONFIG_H
#endif /* GUCEF_GUIDRIVEREMSCRIPTEN_CONFIG_H ? */

#undef GUIDRIVEREMSCRIPTEN_EXPORTSPEC
#ifdef GUIDRIVEREMSCRIPTEN_BUILD_MODULE
  #define GUIDRIVEREMSCRIPTEN_EXPORTSPEC GUCEF_EXPORT
#else
  #define GUIDRIVEREMSCRIPTEN_EXPORTSPEC GUCEF_IMPORT
#endif /* GUIDRIVEREMSCRIPTEN_BUILD_MODULE ? */

#undef GUIDRIVEREMSCRIPTEN_PUBLIC_CPP
#ifdef GUIDRIVEREMSCRIPTEN_PUBLIC_CPP_CODE
  #define GUIDRIVEREMSCRIPTEN_PUBLIC_CPP GUIDRIVEREMSCRIPTEN_EXPORTSPEC
#else
  #define GUIDRIVEREMSCRIPTEN_PUBLIC_CPP GUCEF_HIDDEN
#endif /* GUIDRIVEREMSCRIPTEN_PUBLIC_CPP_CODE ? */

#undef GUIDRIVEREMSCRIPTEN_PUBLIC_C
#ifdef GUIDRIVEREMSCRIPTEN_PUBLIC_C_CODE
  #define GUIDRIVEREMSCRIPTEN_PUBLIC_C GUIDRIVEREMSCRIPTEN_EXPORTSPEC
#else
  #define GUIDRIVEREMSCRIPTEN_PUBLIC_C GUCEF_HIDDEN
#endif /* GUIDRIVEREMSCRIPTEN_PUBLIC_C_CODE ? */

#endif /* GUCEF_GUIDRIVEREMSCRIPTEN_MACROS_H ? */
