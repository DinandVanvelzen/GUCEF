/*
 *  guidriverNull: GUCEF GUI driver that keeps the widgets but draws nothing, for tests and tools
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

#ifndef GUCEF_GUIDRIVERNULL_MACROS_H
#define GUCEF_GUIDRIVERNULL_MACROS_H

#ifndef GUCEF_GUI_MACROS_H
#include "gucefGUI_macros.h"
#define GUCEF_GUI_MACROS_H
#endif /* GUCEF_GUI_MACROS_H ? */

#ifndef GUCEF_GUIDRIVERNULL_CONFIG_H
#include "guidriverNull_config.h"
#define GUCEF_GUIDRIVERNULL_CONFIG_H
#endif /* GUCEF_GUIDRIVERNULL_CONFIG_H ? */

#undef GUIDRIVERNULL_EXPORTSPEC
#ifdef GUIDRIVERNULL_BUILD_MODULE
  #define GUIDRIVERNULL_EXPORTSPEC GUCEF_EXPORT
#else
  #define GUIDRIVERNULL_EXPORTSPEC GUCEF_IMPORT
#endif /* GUIDRIVERNULL_BUILD_MODULE ? */

#undef GUIDRIVERNULL_PUBLIC_CPP
#ifdef GUIDRIVERNULL_PUBLIC_CPP_CODE
  #define GUIDRIVERNULL_PUBLIC_CPP GUIDRIVERNULL_EXPORTSPEC
#else
  #define GUIDRIVERNULL_PUBLIC_CPP GUCEF_HIDDEN
#endif /* GUIDRIVERNULL_PUBLIC_CPP_CODE ? */

#undef GUIDRIVERNULL_PUBLIC_C
#ifdef GUIDRIVERNULL_PUBLIC_C_CODE
  #define GUIDRIVERNULL_PUBLIC_C GUIDRIVERNULL_EXPORTSPEC
#else
  #define GUIDRIVERNULL_PUBLIC_C GUCEF_HIDDEN
#endif /* GUIDRIVERNULL_PUBLIC_C_CODE ? */

#endif /* GUCEF_GUIDRIVERNULL_MACROS_H ? */
