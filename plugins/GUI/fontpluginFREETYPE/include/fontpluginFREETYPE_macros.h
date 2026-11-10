/*
 *  fontpluginFREETYPE: GUCEF GUI plugin that rasterizes TrueType and OpenType fonts with FreeType
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

#ifndef GUCEF_FONTPLUGINFREETYPE_MACROS_H
#define GUCEF_FONTPLUGINFREETYPE_MACROS_H

#ifndef GUCEF_GUI_MACROS_H
#include "gucefGUI_macros.h"
#define GUCEF_GUI_MACROS_H
#endif /* GUCEF_GUI_MACROS_H ? */

#ifndef GUCEF_FONTPLUGINFREETYPE_CONFIG_H
#include "fontpluginFREETYPE_config.h"
#define GUCEF_FONTPLUGINFREETYPE_CONFIG_H
#endif /* GUCEF_FONTPLUGINFREETYPE_CONFIG_H ? */

#undef FONTPLUGINFREETYPE_EXPORTSPEC
#ifdef FONTPLUGINFREETYPE_BUILD_MODULE
  #define FONTPLUGINFREETYPE_EXPORTSPEC GUCEF_EXPORT
#else
  #define FONTPLUGINFREETYPE_EXPORTSPEC GUCEF_IMPORT
#endif /* FONTPLUGINFREETYPE_BUILD_MODULE ? */

#undef FONTPLUGINFREETYPE_PUBLIC_CPP
#ifdef FONTPLUGINFREETYPE_PUBLIC_CPP_CODE
  #define FONTPLUGINFREETYPE_PUBLIC_CPP FONTPLUGINFREETYPE_EXPORTSPEC
#else
  #define FONTPLUGINFREETYPE_PUBLIC_CPP GUCEF_HIDDEN
#endif /* FONTPLUGINFREETYPE_PUBLIC_CPP_CODE ? */

#undef FONTPLUGINFREETYPE_PUBLIC_C
#ifdef FONTPLUGINFREETYPE_PUBLIC_C_CODE
  #define FONTPLUGINFREETYPE_PUBLIC_C FONTPLUGINFREETYPE_EXPORTSPEC
#else
  #define FONTPLUGINFREETYPE_PUBLIC_C GUCEF_HIDDEN
#endif /* FONTPLUGINFREETYPE_PUBLIC_C_CODE ? */

#endif /* GUCEF_FONTPLUGINFREETYPE_MACROS_H ? */
