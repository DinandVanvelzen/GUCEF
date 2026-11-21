/*
 *  AssetWorkbenchLib: views and controllers of the asset workbench, on Ogre
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

#ifndef ASSETWORKBENCHLIB_MACROS_H
#define ASSETWORKBENCHLIB_MACROS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MACROS_H
#include "gucef_macros.h"
#define GUCEF_MACROS_H
#endif /* GUCEF_MACROS_H ? */

#ifndef ASSETWORKBENCHLIB_CONFIG_H
#include "AssetWorkbenchLib_config.h"
#define ASSETWORKBENCHLIB_CONFIG_H
#endif /* ASSETWORKBENCHLIB_CONFIG_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      GENERAL MACROS                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

#undef ASSETWORKBENCHLIB_EXPORTSPEC
#ifdef ASSETWORKBENCHLIB_BUILD_MODULE
  #define ASSETWORKBENCHLIB_EXPORTSPEC GUCEF_EXPORT
#else
  #define ASSETWORKBENCHLIB_EXPORTSPEC GUCEF_IMPORT
#endif /* ASSETWORKBENCHLIB_BUILD_MODULE ? */

#undef ASSETWORKBENCHLIB_PUBLIC_CPP
#ifdef ASSETWORKBENCHLIB_PUBLIC_CPP_CODE
  #define ASSETWORKBENCHLIB_PUBLIC_CPP ASSETWORKBENCHLIB_EXPORTSPEC
#else
  #define ASSETWORKBENCHLIB_PUBLIC_CPP GUCEF_HIDDEN
#endif /* ASSETWORKBENCHLIB_PUBLIC_CPP_CODE ? */

#define ASSETWORKBENCHLIB_PRIVATE_CPP GUCEF_HIDDEN

#undef ASSETWORKBENCHLIB_PUBLIC_C
#ifdef ASSETWORKBENCHLIB_PUBLIC_C_CODE
  #define ASSETWORKBENCHLIB_PUBLIC_C ASSETWORKBENCHLIB_EXPORTSPEC
#else
  #define ASSETWORKBENCHLIB_PUBLIC_C GUCEF_HIDDEN
#endif /* ASSETWORKBENCHLIB_PUBLIC_C_CODE ? */

#define ASSETWORKBENCHLIB_PRIVATE_C GUCEF_HIDDEN

/*-------------------------------------------------------------------------*/

#endif /* ASSETWORKBENCHLIB_MACROS_H ? */
