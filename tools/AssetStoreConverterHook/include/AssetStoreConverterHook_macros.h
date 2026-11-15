/*
 *  AssetStoreConverterHook: the converter pipeline as the conversion step of the primary asset store
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

#ifndef ASSETSTORECONVERTERHOOK_MACROS_H
#define ASSETSTORECONVERTERHOOK_MACROS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MACROS_H
#include "gucef_macros.h"
#define GUCEF_MACROS_H
#endif /* GUCEF_MACROS_H ? */

#ifndef ASSETSTORECONVERTERHOOK_CONFIG_H
#include "AssetStoreConverterHook_config.h"
#define ASSETSTORECONVERTERHOOK_CONFIG_H
#endif /* ASSETSTORECONVERTERHOOK_CONFIG_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      GENERAL MACROS                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

#undef ASSETSTORECONVERTERHOOK_EXPORTSPEC
#ifdef ASSETSTORECONVERTERHOOK_BUILD_MODULE
  #define ASSETSTORECONVERTERHOOK_EXPORTSPEC GUCEF_EXPORT
#else
  #define ASSETSTORECONVERTERHOOK_EXPORTSPEC GUCEF_IMPORT
#endif /* ASSETSTORECONVERTERHOOK_BUILD_MODULE ? */

#undef ASSETSTORECONVERTERHOOK_PUBLIC_CPP
#ifdef ASSETSTORECONVERTERHOOK_PUBLIC_CPP_CODE
  #define ASSETSTORECONVERTERHOOK_PUBLIC_CPP ASSETSTORECONVERTERHOOK_EXPORTSPEC
#else
  #define ASSETSTORECONVERTERHOOK_PUBLIC_CPP GUCEF_HIDDEN
#endif /* ASSETSTORECONVERTERHOOK_PUBLIC_CPP_CODE ? */

#define ASSETSTORECONVERTERHOOK_PRIVATE_CPP GUCEF_HIDDEN

#undef ASSETSTORECONVERTERHOOK_PUBLIC_C
#ifdef ASSETSTORECONVERTERHOOK_PUBLIC_C_CODE
  #define ASSETSTORECONVERTERHOOK_PUBLIC_C ASSETSTORECONVERTERHOOK_EXPORTSPEC
#else
  #define ASSETSTORECONVERTERHOOK_PUBLIC_C GUCEF_HIDDEN
#endif /* ASSETSTORECONVERTERHOOK_PUBLIC_C_CODE ? */

#define ASSETSTORECONVERTERHOOK_PRIVATE_C GUCEF_HIDDEN

/*-------------------------------------------------------------------------*/

#endif /* ASSETSTORECONVERTERHOOK_MACROS_H ? */
