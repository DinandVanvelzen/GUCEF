/*
 *  tablecodecpluginXLSX: GUCEF table codec plugin for Microsoft Excel (.xlsx) workbooks
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

#ifndef TABLECODECPLUGIN_XLSX_MACROS_H
#define TABLECODECPLUGIN_XLSX_MACROS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_ETYPES_H
#include "gucefCORE_ETypes.h"
#define GUCEF_CORE_ETYPES_H
#endif /* GUCEF_CORE_ETYPES_H ? */

#ifndef GUCEF_CORE_MACROS_H
#include "gucefCORE_macros.h"
#define GUCEF_CORE_MACROS_H
#endif /* GUCEF_CORE_MACROS_H ? */

#ifndef TABLECODECPLUGIN_XLSX_CONFIG_H
#include "tablecodecpluginXLSX_config.h"
#define TABLECODECPLUGIN_XLSX_CONFIG_H
#endif /* TABLECODECPLUGIN_XLSX_CONFIG_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      GENERAL MACROS                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *      When compiling the plugin TABLECODECPLUGIN_XLSX_BUILD_MODULE is defined.
 */
#undef TABLECODECPLUGIN_XLSX_EXPORTSPEC
#ifdef TABLECODECPLUGIN_XLSX_BUILD_MODULE
  #define TABLECODECPLUGIN_XLSX_EXPORTSPEC GUCEF_EXPORT
#else
  #define TABLECODECPLUGIN_XLSX_EXPORTSPEC GUCEF_IMPORT
#endif /* TABLECODECPLUGIN_XLSX_BUILD_MODULE ? */

/*-------------------------------------------------------------------------*/

#undef TABLECODECPLUGIN_XLSX_PUBLIC_CPP
#ifdef TABLECODECPLUGIN_XLSX_PUBLIC_CPP_CODE
  #define TABLECODECPLUGIN_XLSX_PUBLIC_CPP TABLECODECPLUGIN_XLSX_EXPORTSPEC
#else
  #define TABLECODECPLUGIN_XLSX_PUBLIC_CPP GUCEF_HIDDEN
#endif /* TABLECODECPLUGIN_XLSX_PUBLIC_CPP_CODE */

#undef TABLECODECPLUGIN_XLSX_PUBLIC_C
#ifdef TABLECODECPLUGIN_XLSX_PUBLIC_C_CODE
  #define TABLECODECPLUGIN_XLSX_PUBLIC_C TABLECODECPLUGIN_XLSX_EXPORTSPEC
#else
  #define TABLECODECPLUGIN_XLSX_PUBLIC_C GUCEF_HIDDEN
#endif /* TABLECODECPLUGIN_XLSX_PUBLIC_C_CODE */

#define TABLECODECPLUGIN_XLSX_PRIVATE_CPP GUCEF_HIDDEN
#define TABLECODECPLUGIN_XLSX_PRIVATE_C GUCEF_HIDDEN

/*-------------------------------------------------------------------------*/

#endif /* TABLECODECPLUGIN_XLSX_MACROS_H ? */
