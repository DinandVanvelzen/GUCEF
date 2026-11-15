/*
 *  AssetViewerLib: views and controllers of the asset viewer, on Ogre
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

#ifndef ASSETVIEWERLIB_MACROS_H
#define ASSETVIEWERLIB_MACROS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MACROS_H
#include "gucef_macros.h"
#define GUCEF_MACROS_H
#endif /* GUCEF_MACROS_H ? */

#ifndef ASSETVIEWERLIB_CONFIG_H
#include "AssetViewerLib_config.h"
#define ASSETVIEWERLIB_CONFIG_H
#endif /* ASSETVIEWERLIB_CONFIG_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      GENERAL MACROS                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

#undef ASSETVIEWERLIB_EXPORTSPEC
#ifdef ASSETVIEWERLIB_BUILD_MODULE
  #define ASSETVIEWERLIB_EXPORTSPEC GUCEF_EXPORT
#else
  #define ASSETVIEWERLIB_EXPORTSPEC GUCEF_IMPORT
#endif /* ASSETVIEWERLIB_BUILD_MODULE ? */

#undef ASSETVIEWERLIB_PUBLIC_CPP
#ifdef ASSETVIEWERLIB_PUBLIC_CPP_CODE
  #define ASSETVIEWERLIB_PUBLIC_CPP ASSETVIEWERLIB_EXPORTSPEC
#else
  #define ASSETVIEWERLIB_PUBLIC_CPP GUCEF_HIDDEN
#endif /* ASSETVIEWERLIB_PUBLIC_CPP_CODE ? */

#define ASSETVIEWERLIB_PRIVATE_CPP GUCEF_HIDDEN

#undef ASSETVIEWERLIB_PUBLIC_C
#ifdef ASSETVIEWERLIB_PUBLIC_C_CODE
  #define ASSETVIEWERLIB_PUBLIC_C ASSETVIEWERLIB_EXPORTSPEC
#else
  #define ASSETVIEWERLIB_PUBLIC_C GUCEF_HIDDEN
#endif /* ASSETVIEWERLIB_PUBLIC_C_CODE ? */

#define ASSETVIEWERLIB_PRIVATE_C GUCEF_HIDDEN

/*-------------------------------------------------------------------------*/

#endif /* ASSETVIEWERLIB_MACROS_H ? */
