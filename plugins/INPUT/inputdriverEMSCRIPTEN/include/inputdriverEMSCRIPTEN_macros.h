/*
 *  inputdriverEMSCRIPTEN: GUCEF input driver for web browsers: mouse, wheel, keys and text of a page
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

#ifndef GUCEF_INPUTDRIVEREMSCRIPTEN_MACROS_H
#define GUCEF_INPUTDRIVEREMSCRIPTEN_MACROS_H

#ifndef GUCEF_INPUT_MACROS_H
#include "gucefINPUT_macros.h"
#define GUCEF_INPUT_MACROS_H
#endif /* GUCEF_INPUT_MACROS_H ? */

#ifndef GUCEF_INPUTDRIVEREMSCRIPTEN_CONFIG_H
#include "inputdriverEMSCRIPTEN_config.h"
#define GUCEF_INPUTDRIVEREMSCRIPTEN_CONFIG_H
#endif /* GUCEF_INPUTDRIVEREMSCRIPTEN_CONFIG_H ? */

#undef INPUTDRIVEREMSCRIPTEN_EXPORTSPEC
#ifdef INPUTDRIVEREMSCRIPTEN_BUILD_MODULE
  #define INPUTDRIVEREMSCRIPTEN_EXPORTSPEC GUCEF_EXPORT
#else
  #define INPUTDRIVEREMSCRIPTEN_EXPORTSPEC GUCEF_IMPORT
#endif /* INPUTDRIVEREMSCRIPTEN_BUILD_MODULE ? */

#undef INPUTDRIVEREMSCRIPTEN_PUBLIC_CPP
#ifdef INPUTDRIVEREMSCRIPTEN_PUBLIC_CPP_CODE
  #define INPUTDRIVEREMSCRIPTEN_PUBLIC_CPP INPUTDRIVEREMSCRIPTEN_EXPORTSPEC
#else
  #define INPUTDRIVEREMSCRIPTEN_PUBLIC_CPP GUCEF_HIDDEN
#endif /* INPUTDRIVEREMSCRIPTEN_PUBLIC_CPP_CODE ? */

#undef INPUTDRIVEREMSCRIPTEN_PUBLIC_C
#ifdef INPUTDRIVEREMSCRIPTEN_PUBLIC_C_CODE
  #define INPUTDRIVEREMSCRIPTEN_PUBLIC_C INPUTDRIVEREMSCRIPTEN_EXPORTSPEC
#else
  #define INPUTDRIVEREMSCRIPTEN_PUBLIC_C GUCEF_HIDDEN
#endif /* INPUTDRIVEREMSCRIPTEN_PUBLIC_C_CODE ? */

#endif /* GUCEF_INPUTDRIVEREMSCRIPTEN_MACROS_H ? */
